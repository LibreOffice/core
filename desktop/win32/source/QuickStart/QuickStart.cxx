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

// QuickStart.cpp : Defines the entry point for the application.

#include <sal/config.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include "resource.h"
#include <systools/win32/uwinapi.h>
#include <systools/win32/qswin32.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

static bool SofficeRuns()
{
    // check for soffice by searching the communication window
    return FindWindowExW( nullptr, nullptr, QUICKSTART_CLASSNAME, nullptr ) != nullptr;
}

static bool launchSoffice( )
{
    if ( !SofficeRuns() )
    {
        wchar_t filename[_MAX_PATH + 1];

        filename[_MAX_PATH] = 0;
        GetModuleFileNameW( nullptr, filename, _MAX_PATH ); // soffice resides in the same dir
        wchar_t *p = wcsrchr( filename, L'\\' );
        if ( !p )
            return false;

        wcsncpy( p+1, L"soffice.exe", _MAX_PATH - (p+1 - filename) );

        wchar_t imagename[_MAX_PATH + 1];

        imagename[_MAX_PATH] = 0;
        _snwprintf(imagename, _MAX_PATH, L"\"%s\" --quickstart", filename );

        STARTUPINFOW aStartupInfo{ .cb = sizeof(aStartupInfo), .wShowWindow = SW_SHOW };
        PROCESS_INFORMATION aProcessInfo;
        bool bSuccess = CreateProcessW(filename, imagename, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &aStartupInfo, &aProcessInfo);
        if ( !bSuccess )
            return false;

        CloseHandle(aProcessInfo.hProcess);
        CloseHandle(aProcessInfo.hThread);
        return true;
    }
    else
        return true;
}

int APIENTRY wWinMain(HINSTANCE /*hInstance*/,
                      HINSTANCE /*hPrevInstance*/,
                      LPWSTR    /*lpCmdLine*/,
                      int       /*nCmdShow*/)
{
    // Look for --killtray argument

    for ( int i = 1; i < __argc; i++ )
    {
        if ( 0 == wcscmp( __wargv[i], L"--killtray" ) )
        {
            HWND hwndTray = FindWindowW( QUICKSTART_CLASSNAME, nullptr );

            if ( hwndTray )
            {
                UINT uMsgKillTray = RegisterWindowMessageW( SHUTDOWN_QUICKSTART_MESSAGE );
                SendMessageW( hwndTray, uMsgKillTray, 0, 0 );
            }

            return 0;
        }
    }

    launchSoffice();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
