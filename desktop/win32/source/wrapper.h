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
#pragma once
#ifndef __cplusplus
#error Need C++ to compile
#endif

#include "main.h"

#ifndef _WINDOWS_
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#   include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#endif


#ifndef _INC_TCHAR
#   ifdef UNICODE
#       define _UNICODE
#   endif
#   include <tchar.h>
#endif

#ifdef UNICODE
#   define Main MainW
#   define GetArgv( pArgc )         CommandLineToArgvW( GetCommandLine(), pArgc )
#   define PROCESS_CREATIONFLAGS    CREATE_UNICODE_ENVIRONMENT
#else
#   define GetArgv( pArgc )         (*pArgc = __argc, __argv)
#   define PROCESS_CREATIONFLAGS    0
#   define Main MainA
#endif

#define BIN_EXT_STR         TEXT(".bin")
#define PARAM_LIBPATH_STR   TEXT("-libpath=")
#define PARAM_LOCAL_STR     TEXT("-local")
#define PARAM_REMOTE_STR    TEXT("-remote")

#if defined( REMOTE )
#define DEFAULT_LIBPATH     TEXT("remote;")
#elif defined( LOCAL )
#define DEFAULT_LIBPATH     TEXT("local;")
#endif

extern "C" int Main()
{
    // Retrieve startup info

    STARTUPINFO aStartupInfo;

    ZeroMemory( &aStartupInfo, sizeof(aStartupInfo) );
    aStartupInfo.cb = sizeof( aStartupInfo );
    GetStartupInfo( &aStartupInfo );

    // Retrieve command line

    LPTSTR  lpCommandLine = GetCommandLine();

    LPTSTR  *ppArguments = NULL;
    int     nArguments = 0;

    ppArguments = GetArgv( &nArguments );

    // Calculate application name


    TCHAR   szApplicationName[MAX_PATH];
    TCHAR   szDrive[MAX_PATH];
    TCHAR   szDir[MAX_PATH];
    TCHAR   szFileName[MAX_PATH];
    TCHAR   szExt[MAX_PATH];

    GetModuleFileName( NULL, szApplicationName, MAX_PATH );
    _tsplitpath( szApplicationName, szDrive, szDir, szFileName, szExt );
    _tmakepath( szApplicationName, szDrive, szDir, szFileName, BIN_EXT_STR );

    // Retrieve actual environment

    TCHAR   szBuffer[1024];
    TCHAR   szPathValue[1024] = TEXT("");

#ifdef DEFAULT_LIBPATH
    _tmakepath( szPathValue, szDrive, szDir, DEFAULT_LIBPATH, TEXT("") );
#endif

    for ( int argn = 1; argn < nArguments; argn++ )
    {
        if ( 0 == _tcscmp( ppArguments[argn], PARAM_REMOTE_STR ) )
        {
            _tmakepath( szPathValue, szDrive, szDir, TEXT("remote;"), TEXT("") );
            break;
        }
        else if ( 0 == _tcscmp( ppArguments[argn], PARAM_LOCAL_STR ) )
        {
            _tmakepath( szPathValue, szDrive, szDir, TEXT("local;"), TEXT("") );
            break;
        }
        else if ( 0 == _tcsncmp( ppArguments[argn], PARAM_LIBPATH_STR, _tcslen(PARAM_LIBPATH_STR) ) )
        {
            LPTSTR  pFileSpec = NULL;

            GetFullPathName( ppArguments[argn] + _tcslen(PARAM_LIBPATH_STR), sizeof(szPathValue) / sizeof(TCHAR), szPathValue, &pFileSpec );
            _tcscat( szPathValue, TEXT(";") );
            break;
        }
    }

    GetEnvironmentVariable( TEXT("PATH"), szBuffer, sizeof(szBuffer) );
    _tcscat( szPathValue, szBuffer );
    SetEnvironmentVariable( TEXT("PATH"), szPathValue);

    LPVOID  lpEnvironment = GetEnvironmentStrings();


    // Retrieve current directory

    TCHAR               szCurrentDirectory[MAX_PATH];
    GetCurrentDirectory( MAX_PATH, szCurrentDirectory );

    // Set the Flags

    DWORD   dwCreationFlags = PROCESS_CREATIONFLAGS;

    PROCESS_INFORMATION aProcessInfo;

    BOOL    fSuccess = CreateProcess(
        szApplicationName,
        lpCommandLine,
        NULL,
        NULL,
        TRUE,
        dwCreationFlags,
        lpEnvironment,
        szCurrentDirectory,
        &aStartupInfo,
        &aProcessInfo );

    if ( fSuccess )
    {
        DWORD   dwExitCode;

        WaitForSingleObject( aProcessInfo.hProcess, INFINITE );
        fSuccess = GetExitCodeProcess( aProcessInfo.hProcess, &dwExitCode );

        return dwExitCode;
    }

    DWORD   dwError = GetLastError();

    LPVOID lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );

    // Display the string.
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, NULL, MB_OK | MB_ICONERROR );

    // Free the buffer.
    LocalFree( lpMsgBuf );

    return GetLastError();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
