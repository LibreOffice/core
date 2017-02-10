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

#include "launcher.hxx"

#include <shellapi.h>

#include <stdlib.h>
#include <malloc.h>

extern "C" int APIENTRY _tWinMain( HINSTANCE, HINSTANCE, LPTSTR, int )
{
    // Retrieve startup info

    STARTUPINFO aStartupInfo;

    ZeroMemory( &aStartupInfo, sizeof(aStartupInfo) );
    aStartupInfo.cb = sizeof( aStartupInfo );
    GetStartupInfo( &aStartupInfo );

    // Retrieve command line

    LPTSTR  lpCommandLine = GetCommandLine();

    {
        lpCommandLine = static_cast<LPTSTR>(_alloca( sizeof(_TCHAR) * (_tcslen(lpCommandLine) + _tcslen(APPLICATION_SWITCH) + 2) ));

        _tcscpy( lpCommandLine, GetCommandLine() );
        _tcscat( lpCommandLine, _T(" ") );
        _tcscat( lpCommandLine, APPLICATION_SWITCH );
    }


    // Calculate application name

    TCHAR   szApplicationName[MAX_PATH];
    TCHAR   szDrive[MAX_PATH];
    TCHAR   szDir[MAX_PATH];
    TCHAR   szFileName[MAX_PATH];
    TCHAR   szExt[MAX_PATH];

    GetModuleFileName( nullptr, szApplicationName, MAX_PATH );
    _tsplitpath( szApplicationName, szDrive, szDir, szFileName, szExt );
    _tmakepath( szApplicationName, szDrive, szDir, _T("soffice"), _T(".exe") );


    PROCESS_INFORMATION aProcessInfo;

    BOOL    fSuccess = CreateProcess(
        szApplicationName,
        lpCommandLine,
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &aStartupInfo,
        &aProcessInfo );

    if ( fSuccess )
    {
        // Wait for soffice process to be terminated to allow other applications
        // to wait for termination of started process

        WaitForSingleObject( aProcessInfo.hProcess, INFINITE );

        CloseHandle( aProcessInfo.hProcess );
        CloseHandle( aProcessInfo.hThread );

        return 0;
    }

    DWORD   dwError = GetLastError();

    LPVOID lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        reinterpret_cast<LPTSTR>(&lpMsgBuf),
        0,
        nullptr
    );

    // Display the string.
    MessageBox( nullptr, static_cast<LPCTSTR>(lpMsgBuf), nullptr, MB_OK | MB_ICONERROR );

    // Free the buffer.
    LocalFree( lpMsgBuf );

    return GetLastError();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
