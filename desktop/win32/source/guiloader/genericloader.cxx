/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"
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

#include "tools/pathutils.hxx"
#include "../extendloaderenvironment.hxx"

//---------------------------------------------------------------------------

static int GenericMain()
{
    TCHAR               szTargetFileName[MAX_PATH];
    TCHAR               szIniDirectory[MAX_PATH];
    STARTUPINFO         aStartupInfo;

    desktop_win32::extendLoaderEnvironment(szTargetFileName, szIniDirectory);

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
            MY_STRING(L"redirect.ini")) != NULL &&
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
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
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

                PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
            }
        } while ( WAIT_OBJECT_0 + 1 == dwWaitResult );

        dwExitCode = 0;
        GetExitCodeProcess( aProcessInfo.hProcess, &dwExitCode );

        CloseHandle( aProcessInfo.hProcess );
        CloseHandle( aProcessInfo.hThread );
    }

    return dwExitCode;
}

//---------------------------------------------------------------------------

#ifdef __MINGW32__
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
#else
int WINAPI _tWinMain( HINSTANCE, HINSTANCE, LPTSTR, int )
#endif
{
    return GenericMain();
}

//---------------------------------------------------------------------------

#ifdef __MINGW32__
int __cdecl main()
#else
int __cdecl _tmain()
#endif
{
    return GenericMain();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
