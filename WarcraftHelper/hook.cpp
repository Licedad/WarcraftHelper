#include "hook.h"
#include <detours.h>

void Hook(void* pOldFuncAddr, void* pNewFuncAddr)
{
	if (!pOldFuncAddr) {
		return;
	}
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(void*&)pOldFuncAddr, pNewFuncAddr);
	DetourTransactionCommit();
}

void InlineHook(void* pOldFuncAddr, void* pNewFuncAddr, void*& pCallBackFuncAddr)
{
	if (!pOldFuncAddr) {
		return;
	}
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(void*&)pOldFuncAddr, pNewFuncAddr);
	DetourTransactionCommit();
	pCallBackFuncAddr = pOldFuncAddr;
}

void DetachHook(void* pOldFuncAddr, void* pNewFuncAddr)
{
	if (!pOldFuncAddr) {
		return;
	}
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(void*&)pOldFuncAddr, pNewFuncAddr);
	DetourTransactionCommit();
}

void PatchMemory(uintptr_t pAddress, unsigned char* bytes, uint32_t size)
{
	void* address = reinterpret_cast<void*>(pAddress);

	unsigned long Protection;
	VirtualProtect(address, size, PAGE_READWRITE, &Protection);
	memcpy(address, (const void*)bytes, size);
	VirtualProtect(address, size, Protection, &Protection);
}

DWORD War3_Search(void* pPattern, DWORD dwPatternLen, DWORD dwSearchStart, DWORD dwSearchEnd)
{
	DWORD dwStartAddr = dwSearchStart;
	DWORD dwEndAddr = dwSearchEnd - dwPatternLen;

	while (dwStartAddr < dwEndAddr)
	{
		bool found = true;
		for (DWORD i = 0; i < dwPatternLen; i++)
		{
			char code = *(char*)(dwStartAddr + i);
			if (*((char*)pPattern + i) != code)
			{
				found = false;
				break;
			}
		}
		if (found)
			return dwStartAddr;
		dwStartAddr++;
	}
	return 0;
}

DWORD ReadDwordFromReg(LPCTSTR regPath, LPCTSTR keyName)
{
	HKEY hKey = 0;
	long lret = ::RegOpenKeyEx(HKEY_CURRENT_USER, regPath, 0, KEY_ALL_ACCESS, &hKey);
	if (lret == ERROR_SUCCESS)
	{
		DWORD value;
		DWORD dwType = REG_DWORD;
		DWORD dwSize = sizeof(DWORD);

		lret = ::RegQueryValueEx(hKey, keyName, 0, &dwType, (BYTE*)&value, &dwSize);
		if ((lret == ERROR_SUCCESS) && (dwType == REG_DWORD))
		{
			::RegCloseKey(hKey);
			return value;
		}
		::RegCloseKey(hKey);
	}

	return 0;
}

BOOL WriteDwordToReg(LPCTSTR regPath, LPCTSTR keyName, DWORD value)
{
	HKEY hKey = 0;
	long lret = ::RegCreateKey(HKEY_CURRENT_USER, regPath, &hKey);
	if (lret == ERROR_SUCCESS)
	{
		lret = ::RegSetValueEx(hKey, keyName, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
		if (lret == ERROR_SUCCESS)
		{
			::RegCloseKey(hKey);
			return TRUE;
		}
		::RegCloseKey(hKey);
	}

	return FALSE;
}