#include "macros.h"
#pragma warning(push,1) // disable warnings within system headers
#include <psapi.h>
#pragma warning(pop)

IMPLEMENT_THUNK( psapi, WINDOWS, DWORD, WINAPI, GetModuleFileNameExW, (HANDLE hProcess, HMODULE hModule, LPWSTR lpFileNameW, DWORD nSize ) )
{
    AUTO_STR( lpFileName, 2 * nSize );

    if ( GetModuleFileNameExA( hProcess, hModule, lpFileNameA, 2 * nSize ) )
        return STR2WSTR( lpFileName, nSize );
    else
        return 0;
}
