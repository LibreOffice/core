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

#include <filesystem>
#include <stdlib.h>
#include <malloc.h>

#include <systools/win32/extended_max_path.hxx>

extern "C" int APIENTRY wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Retrieve startup info

    STARTUPINFOW aStartupInfo{ .cb = sizeof(aStartupInfo) };
    GetStartupInfoW( &aStartupInfo );

    // Retrieve command line

    LPWSTR lpCommandLine = static_cast<LPWSTR>(_alloca( sizeof(WCHAR) * (wcslen(GetCommandLineW()) + wcslen(APPLICATION_SWITCH) + 2) ));

    wcscpy( lpCommandLine, GetCommandLineW() );
    wcscat( lpCommandLine, L" " );
    wcscat( lpCommandLine, APPLICATION_SWITCH );

    // Calculate application name

    WCHAR szThisAppName[EXTENDED_MAX_PATH];
    GetModuleFileNameW(nullptr, szThisAppName, std::size(szThisAppName));
    std::filesystem::path soffice_exe(szThisAppName);
    soffice_exe.replace_filename(L"soffice.exe");

    PROCESS_INFORMATION aProcessInfo;

    bool fSuccess = CreateProcessW(
        soffice_exe.c_str(),
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

    DWORD dwError = GetLastError();

    LPWSTR lpMsgBuf = nullptr;

    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        reinterpret_cast<LPWSTR>(&lpMsgBuf),
        0,
        nullptr
    );

    // Display the string.
    MessageBoxW( nullptr, lpMsgBuf, nullptr, MB_OK | MB_ICONERROR );

    // Free the buffer.
    HeapFree( GetProcessHeap(), 0, lpMsgBuf );

    return dwError;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
