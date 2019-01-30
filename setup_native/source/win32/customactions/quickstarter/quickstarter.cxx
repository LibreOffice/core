/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "quickstarter.hxx"

#include <psapi.h>

#include <malloc.h>

std::wstring GetOfficeInstallationPathW(MSIHANDLE handle)
{
    std::wstring progpath;
    DWORD sz = 0;
    PWSTR dummy = const_cast<PWSTR>(L"");

    if (MsiGetPropertyW(handle, L"INSTALLLOCATION", dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++; // space for the final '\0'
        DWORD nbytes = sz * sizeof(WCHAR);
        PWSTR buff = static_cast<PWSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetPropertyW(handle, L"INSTALLLOCATION", buff, &sz);
        progpath = buff;
    }
    return progpath;
}

std::wstring GetOfficeProductNameW(MSIHANDLE handle)
{
    std::wstring productname;
    DWORD sz = 0;
    PWSTR dummy = const_cast<PWSTR>(L"");

    if (MsiGetPropertyW(handle, L"ProductName", dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++; // space for the final '\0'
        DWORD nbytes = sz * sizeof(WCHAR);
        PWSTR buff = static_cast<PWSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetPropertyW(handle, L"ProductName", buff, &sz);
        productname = buff;
    }
    return productname;
}

std::wstring GetQuickstarterLinkNameW(MSIHANDLE handle)
{
    std::wstring quickstarterlinkname;
    DWORD sz = 0;
    PWSTR dummy = const_cast<PWSTR>(L"");

    if (MsiGetPropertyW(handle, L"Quickstarterlinkname", dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++; // space for the final '\0'
        DWORD nbytes = sz * sizeof(WCHAR);
        PWSTR buff = static_cast<PWSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetPropertyW(handle, L"Quickstarterlinkname", buff, &sz);
        quickstarterlinkname = buff;
    }
    else if (MsiGetPropertyW(handle, L"ProductName", dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++; // space for the final '\0'
        DWORD nbytes = sz * sizeof(WCHAR);
        PWSTR buff = static_cast<PWSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetPropertyW(handle, L"ProductName", buff, &sz);
        quickstarterlinkname = buff;
    }
    return quickstarterlinkname;
}

static bool IsValidHandle( HANDLE handle )
{
    return nullptr != handle && INVALID_HANDLE_VALUE != handle;
}

static DWORD WINAPI GetModuleFileNameExW_( HANDLE hProcess, HMODULE hModule, PWSTR lpFileName, DWORD nSize )
{
    typedef DWORD (WINAPI *FN_PROC)( HANDLE hProcess, HMODULE hModule, LPWSTR lpFileName, DWORD nSize );

    static FN_PROC  lpProc = nullptr;

    if ( !lpProc )
    {
        HMODULE hLibrary = LoadLibraryW(L"PSAPI.DLL");

        if ( hLibrary )
            lpProc = reinterpret_cast< FN_PROC >(GetProcAddress( hLibrary, "GetModuleFileNameExW" ));
    }

    if ( lpProc )
        return lpProc( hProcess, hModule, lpFileName, nSize );

    return 0;

}

std::wstring GetProcessImagePathW( DWORD dwProcessId )
{
    std::wstring sImagePath;

    HANDLE  hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId );

    if ( IsValidHandle( hProcess ) )
    {
        WCHAR szPathBuffer[MAX_PATH] = L"";

        if ( GetModuleFileNameExW_( hProcess, nullptr, szPathBuffer, sizeof(szPathBuffer)/sizeof(szPathBuffer[0]) ) )
            sImagePath = szPathBuffer;

        CloseHandle( hProcess );
    }

    return sImagePath;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
