/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef _MSC_VER
#pragma warning(disable:4273)       // inconsistent dll linkage
#endif

#if (defined(_MSC_VER) && (_MSC_VER >= 1900)) || defined __clang__
// older versions use uwinapi.def file for export
__declspec(dllexport)
#endif
DWORD WINAPI GetShortPathNameW(LPCWSTR lpszLongPath,LPWSTR lpszShortPath,DWORD cchBuffer)
{
    typedef DWORD (WINAPI * GetShortPathNameW_t) (LPCWSTR,LPWSTR,DWORD);

    GetShortPathNameW_t p_GetShortPathNameW =
        (GetShortPathNameW_t) GetProcAddress (
            GetModuleHandle ("kernel32.dll"), "GetShortPathNameW");

    if (p_GetShortPathNameW)
        return p_GetShortPathNameW(lpszLongPath,lpszShortPath,cchBuffer);
    else
        return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
