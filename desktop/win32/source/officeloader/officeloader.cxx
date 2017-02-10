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

#include <cstddef>
#include <cwchar>

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <tchar.h>

#include <string.h>
#include <stdlib.h>
#include <systools/win32/uwinapi.h>

#include <desktop/exithelper.h>
#include <rtl/string.h>
#include <sal/macros.h>

#include "../loader.hxx"

#include <config_version.h>

static LPTSTR   *GetCommandArgs( int *pArgc )
{
#ifdef UNICODE
    return CommandLineToArgvW( GetCommandLineW(), pArgc );
#else
    *pArgc = __argc;
    return __argv;
#endif
}

int WINAPI _tWinMain( HINSTANCE, HINSTANCE, LPTSTR, int )
{
    TCHAR               szTargetFileName[MAX_PATH] = TEXT("");
    TCHAR               szIniDirectory[MAX_PATH];
    STARTUPINFO         aStartupInfo;

    desktop_win32::getPaths(szTargetFileName, szIniDirectory);

    ZeroMemory( &aStartupInfo, sizeof(aStartupInfo) );
    aStartupInfo.cb = sizeof(aStartupInfo);

    // Create process with same command line, environment and stdio handles which
    // are directed to the created pipes
    GetStartupInfo(&aStartupInfo);

    // If this process hasn't its stdio handles set, then check if its parent
    // has a console (i.e. this process is launched from command line), and if so,
    // attach to it. It will enable child process to retrieve this console if it needs
    // to output to console
    if ((aStartupInfo.dwFlags & STARTF_USESTDHANDLES) == 0)
        AttachConsole(ATTACH_PARENT_PROCESS);

    DWORD   dwExitCode = (DWORD)-1;

    BOOL    fSuccess = FALSE;
    LPTSTR  lpCommandLine = nullptr;
    int argc = 0;
    LPTSTR * argv = nullptr;
    bool bFirst = true;
    WCHAR cwd[MAX_PATH];
    DWORD cwdLen = GetCurrentDirectoryW(MAX_PATH, cwd);
    if (cwdLen >= MAX_PATH) {
        cwdLen = 0;
    }

    do
    {
        if ( bFirst ) {
            argv = GetCommandArgs(&argc);
            std::size_t n = wcslen(argv[0]) + 2;
            for (int i = 1; i < argc; ++i) {
                n += wcslen(argv[i]) + 3;
            }
            n += MY_LENGTH(L" \"-env:OOO_CWD=2") + 4 * cwdLen +
                MY_LENGTH(L"\"") + 1;
                // 4 * cwdLen: each char preceded by backslash, each trailing
                // backslash doubled
            lpCommandLine = new WCHAR[n];
        }
        WCHAR * p = desktop_win32::commandLineAppend(
            lpCommandLine, MY_STRING(L"\""));
        p = desktop_win32::commandLineAppend(p, argv[0]);
        for (int i = 1; i < argc; ++i) {
            if (bFirst || EXITHELPER_NORMAL_RESTART == dwExitCode || wcsncmp(argv[i], MY_STRING(L"-env:")) == 0) {
                p = desktop_win32::commandLineAppend(p, MY_STRING(L"\" \""));
                p = desktop_win32::commandLineAppend(p, argv[i]);
            }
        }

        p = desktop_win32::commandLineAppend(
            p, MY_STRING(L"\" \"-env:OOO_CWD="));
        if (cwdLen == 0) {
            p = desktop_win32::commandLineAppend(p, MY_STRING(L"0"));
        } else {
            p = desktop_win32::commandLineAppend(p, MY_STRING(L"2"));
            p = desktop_win32::commandLineAppendEncoded(p, cwd);
        }
        desktop_win32::commandLineAppend(p, MY_STRING(L"\""));
        bFirst = false;

        TCHAR   szParentProcessId[64]; // This is more than large enough for a 128 bit decimal value
        BOOL    bHeadlessMode( FALSE );

        {
            // Check command line arguments for "--headless" parameter. We only
            // set the environment variable "ATTACHED_PARENT_PROCESSID" for the headless
            // mode as self-destruction of the soffice.bin process can lead to
            // certain side-effects (log-off can result in data-loss, ".lock" is not deleted.
            // See 138244 for more information.
            int     argc2;
            LPTSTR  *argv2 = GetCommandArgs( &argc2 );

            if ( argc2 > 1 )
            {
                int n;

                for ( n = 1; n < argc2; n++ )
                {
                    if ( 0 == _tcsnicmp( argv2[n], _T("-headless"), 9 ) ||
                         0 == _tcsnicmp( argv2[n], _T("--headless"), 10 ) )
                    {
                        bHeadlessMode = TRUE;
                    }
                }
            }
        }

        if ( _ltot( (long)GetCurrentProcessId(),szParentProcessId, 10 ) && bHeadlessMode )
            SetEnvironmentVariable( TEXT("ATTACHED_PARENT_PROCESSID"), szParentProcessId );

        PROCESS_INFORMATION aProcessInfo;

        fSuccess = CreateProcess(
            szTargetFileName,
            lpCommandLine,
            nullptr,
            nullptr,
            TRUE,
            0,
            nullptr,
            szIniDirectory,
            &aStartupInfo,
            &aProcessInfo );

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
    } while ( fSuccess
              && ( EXITHELPER_CRASH_WITH_RESTART == dwExitCode || EXITHELPER_NORMAL_RESTART == dwExitCode ));
    delete[] lpCommandLine;

    return fSuccess ? dwExitCode : -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
