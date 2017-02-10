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

#define UNICODE
#define _UNICODE

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <shellapi.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <tchar.h>

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <systools/win32/uwinapi.h>

#include <tools/pathutils.hxx>
#include "../loader.hxx"


static int GenericMain()
{
    TCHAR               szTargetFileName[MAX_PATH];
    TCHAR               szIniDirectory[MAX_PATH];
    STARTUPINFO         aStartupInfo;

    desktop_win32::getPaths(szTargetFileName, szIniDirectory);

    ZeroMemory( &aStartupInfo, sizeof(aStartupInfo) );
    aStartupInfo.cb = sizeof(aStartupInfo);

    GetStartupInfo( &aStartupInfo );

    DWORD   dwExitCode = (DWORD)-1;

    PROCESS_INFORMATION aProcessInfo;

    size_t iniDirLen = wcslen(szIniDirectory);
    WCHAR cwd[MAX_PATH];
    DWORD cwdLen = GetCurrentDirectoryW(MAX_PATH, cwd);
    if (cwdLen >= MAX_PATH) {
        cwdLen = 0;
    }
    WCHAR redirect[MAX_PATH];
    DWORD dummy;
    bool hasRedirect =
        tools::buildPath(
            redirect, szIniDirectory, szIniDirectory + iniDirLen,
            MY_STRING(L"redirect.ini")) != nullptr &&
        (GetBinaryType(redirect, &dummy) || // cheaper check for file existence?
         GetLastError() != ERROR_FILE_NOT_FOUND);
    LPTSTR cl1 = GetCommandLine();
    WCHAR * cl2 = new WCHAR[
        wcslen(cl1) +
        (hasRedirect
         ? (MY_LENGTH(L" \"-env:INIFILENAME=vnd.sun.star.pathname:") +
            iniDirLen + MY_LENGTH(L"redirect.ini\""))
         : 0) +
        MY_LENGTH(L" \"-env:OOO_CWD=2") + 4 * cwdLen + MY_LENGTH(L"\"") + 1];
        // 4 * cwdLen: each char preceded by backslash, each trailing backslash
        // doubled
    WCHAR * p = desktop_win32::commandLineAppend(cl2, cl1);
    if (hasRedirect) {
        p = desktop_win32::commandLineAppend(
            p, MY_STRING(L" \"-env:INIFILENAME=vnd.sun.star.pathname:"));
        p = desktop_win32::commandLineAppend(p, szIniDirectory);
        p = desktop_win32::commandLineAppend(p, MY_STRING(L"redirect.ini\""));
    }
    p = desktop_win32::commandLineAppend(p, MY_STRING(L" \"-env:OOO_CWD="));
    if (cwdLen == 0) {
        p = desktop_win32::commandLineAppend(p, MY_STRING(L"0"));
    } else {
        p = desktop_win32::commandLineAppend(p, MY_STRING(L"2"));
        p = desktop_win32::commandLineAppendEncoded(p, cwd);
    }
    desktop_win32::commandLineAppend(p, MY_STRING(L"\""));

    BOOL fSuccess = CreateProcess(
        szTargetFileName,
        cl2,
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        szIniDirectory,
        &aStartupInfo,
        &aProcessInfo );

    delete[] cl2;

    if ( fSuccess )
    {
        DWORD   dwWaitResult;

        do
        {
            // On Windows XP it seems as the desktop calls WaitForInputIdle after "OpenWidth" so we have to do so
            // as if we where processing any messages

            dwWaitResult = MsgWaitForMultipleObjects( 1, &aProcessInfo.hProcess, FALSE, INFINITE, QS_ALLEVENTS );

            if (  WAIT_OBJECT_0 + 1 == dwWaitResult )
            {
                MSG msg;

                PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE );
            }
        } while ( WAIT_OBJECT_0 + 1 == dwWaitResult );

        dwExitCode = 0;
        GetExitCodeProcess( aProcessInfo.hProcess, &dwExitCode );

        CloseHandle( aProcessInfo.hProcess );
        CloseHandle( aProcessInfo.hThread );
    }

    return dwExitCode;
}

int WINAPI _tWinMain( HINSTANCE, HINSTANCE, LPTSTR, int )
{
    return GenericMain();
}

int __cdecl _tmain()
{
    return GenericMain();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
