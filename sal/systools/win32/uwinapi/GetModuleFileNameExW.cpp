#include "macros.h"
#include <psapi.h>

IMPLEMENT_THUNK( psapi, WINDOWS, DWORD, WINAPI, GetModuleFileNameExW, (HANDLE hProcess, HMODULE hModule, LPWSTR lpFileNameW, DWORD nSize ) )
{
    AUTO_STR( lpFileName, 2 * nSize );

    if ( GetModuleFileNameExA( hProcess, hModule, lpFileNameA, 2 * nSize ) )
        return STR2WSTR( lpFileName, nSize );
    else
        return 0;
}
