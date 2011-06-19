/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "macros.h"
#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#endif
#include <psapi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

IMPLEMENT_THUNK( psapi, WINDOWS, DWORD, WINAPI, GetModuleFileNameExW, (HANDLE hProcess, HMODULE hModule, LPWSTR lpFileNameW, DWORD nSize ) )
{
    AUTO_STR( lpFileName, 2 * nSize );

    if ( GetModuleFileNameExA( hProcess, hModule, lpFileNameA, 2 * nSize ) )
        return (DWORD) STR2WSTR( lpFileName, nSize );
    else
        return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
