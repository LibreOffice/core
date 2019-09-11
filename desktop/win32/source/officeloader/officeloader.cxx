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

#include <cstddef>

#include <systools/win32/uwinapi.h>
#include <stdlib.h>
#include <desktop/exithelper.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "../loader.hxx"

static LPWSTR *GetCommandArgs( int *pArgc )
{
    return CommandLineToArgvW( GetCommandLineW(), pArgc );
}

int WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    WCHAR        szTargetFileName[MAX_PATH] = {};
    WCHAR        szIniDirectory[MAX_PATH];
    STARTUPINFOW aStartupInfo;

    desktop_win32::extendLoaderEnvironment(szTargetFileName, szIniDirectory);

    ZeroMemory( &aStartupInfo, sizeof(aStartupInfo) );
    aStartupInfo.cb = sizeof(aStartupInfo);

    // Create process with same command line, environment and stdio handles which
    // are directed to the created pipes
    GetStartupInfoW(&aStartupInfo);

    // If this process hasn't its stdio handles set, then check if its parent
    // has a console (i.e. this process is launched from command line), and if so,
    // attach to it. It will enable child process to retrieve this console if it needs
    // to output to console
    if ((aStartupInfo.dwFlags & STARTF_USESTDHANDLES) == 0)
        AttachConsole(ATTACH_PARENT_PROCESS);

    DWORD   dwExitCode = DWORD(-1);

    BOOL    fSuccess = FALSE;
    LPWSTR  lpCommandLine = nullptr;
    int argc = 0;
    LPWSTR * argv = nullptr;
    bool bFirst = true;
    WCHAR cwd[MAX_PATH];
    DWORD cwdLen = GetCurrentDirectoryW(MAX_PATH, cwd);
    if (cwdLen >= MAX_PATH) {
        cwdLen = 0;
    }

    // read limit values from bootstrap.ini
    unsigned int nMaxMemoryInMB = 0;
    bool bExcludeChildProcesses = true;

    const WCHAR* szIniFile = L"\\bootstrap.ini";
    const size_t nDirLen = wcslen(szIniDirectory);
    if (wcslen(szIniFile) + nDirLen < MAX_PATH)
    {
        WCHAR szBootstrapIni[MAX_PATH];
        wcscpy(szBootstrapIni, szIniDirectory);
        wcscpy(&szBootstrapIni[nDirLen], szIniFile);

        try
        {
            boost::property_tree::ptree pt;
            std::fstream aFile(szBootstrapIni);
            boost::property_tree::ini_parser::read_ini(aFile, pt);
            nMaxMemoryInMB = pt.get("Win32.LimitMaximumMemoryInMB", nMaxMemoryInMB);
            bExcludeChildProcesses = pt.get("Win32.ExcludeChildProcessesFromLimit", bExcludeChildProcesses);
        }
        catch (...)
        {
            nMaxMemoryInMB = 0;
        }
    }

    // create a Windows JobObject with a memory limit
    HANDLE hJobObject = NULL;
    if (nMaxMemoryInMB > 0)
    {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION aJobLimit;
        aJobLimit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_JOB_MEMORY;
        if (bExcludeChildProcesses)
            aJobLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
        aJobLimit.JobMemoryLimit = nMaxMemoryInMB * 1024 * 1024;
        hJobObject = CreateJobObjectW(NULL, NULL);
        if (hJobObject != NULL)
            SetInformationJobObject(hJobObject, JobObjectExtendedLimitInformation, &aJobLimit,
                                    sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
    }

    do
    {
        if ( bFirst ) {
            argv = GetCommandArgs(&argc);
            std::size_t n = wcslen(argv[0]) + 2;
            for (int i = 1; i < argc; ++i) {
                n += wcslen(argv[i]) + 4; // 2 doublequotes + a space + optional trailing backslash
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
                const size_t arglen = wcslen(argv[i]);
                // tdf#120249: if an argument ends with backslash, we should escape it with another
                // backslash; otherwise, the trailing backslash will be treated as an escapement
                // character for the following doublequote by CommandLineToArgvW in soffice.bin. See
                // https://docs.microsoft.com/en-us/windows/desktop/api/shellapi/nf-shellapi-commandlinetoargvw
                if (arglen && argv[i][arglen-1] == '\\')
                    p = desktop_win32::commandLineAppend(p, MY_STRING(L"\\"));
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

        WCHAR szParentProcessId[64]; // This is more than large enough for a 128 bit decimal value
        BOOL  bHeadlessMode( FALSE );

        {
            // Check command line arguments for "--headless" parameter. We only
            // set the environment variable "ATTACHED_PARENT_PROCESSID" for the headless
            // mode as self-destruction of the soffice.bin process can lead to
            // certain side-effects (log-off can result in data-loss, ".lock" is not deleted.
            // See 138244 for more information.
            int    argc2;
            LPWSTR *argv2 = GetCommandArgs( &argc2 );

            if ( argc2 > 1 )
            {
                int n;

                for ( n = 1; n < argc2; n++ )
                {
                    if ( 0 == wcsnicmp( argv2[n], L"-headless", 9 ) ||
                         0 == wcsnicmp( argv2[n], L"--headless", 10 ) )
                    {
                        bHeadlessMode = TRUE;
                    }
                }
            }

            LocalFree(argv2);
        }

        if ( _ltow( static_cast<long>(GetCurrentProcessId()),szParentProcessId, 10 ) && bHeadlessMode )
            SetEnvironmentVariableW( L"ATTACHED_PARENT_PROCESSID", szParentProcessId );

        PROCESS_INFORMATION aProcessInfo;

        fSuccess = CreateProcessW(
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

            if (hJobObject)
                AssignProcessToJobObject(hJobObject, aProcessInfo.hProcess);

            do
            {
                // On Windows XP it seems as the desktop calls WaitForInputIdle after "OpenWidth" so we have to do so
                // as if we where processing any messages

                dwWaitResult = MsgWaitForMultipleObjects( 1, &aProcessInfo.hProcess, FALSE, INFINITE, QS_ALLEVENTS );

                if (  WAIT_OBJECT_0 + 1 == dwWaitResult )
                {
                    MSG msg;

                    PeekMessageW( &msg, nullptr, 0, 0, PM_REMOVE );
                }
            } while ( WAIT_OBJECT_0 + 1 == dwWaitResult );

            dwExitCode = 0;
            GetExitCodeProcess( aProcessInfo.hProcess, &dwExitCode );

            CloseHandle( aProcessInfo.hProcess );
            CloseHandle( aProcessInfo.hThread );
        }
    } while ( fSuccess
              && ( EXITHELPER_CRASH_WITH_RESTART == dwExitCode || EXITHELPER_NORMAL_RESTART == dwExitCode ));

    if (hJobObject)
        CloseHandle(hJobObject);

    delete[] lpCommandLine;
    LocalFree(argv);

    return fSuccess ? dwExitCode : -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
