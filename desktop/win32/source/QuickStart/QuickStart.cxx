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


#include "StdAfx.h"
#include "resource.h"
#include <systools/win32/uwinapi.h>
#include <systools/win32/qswin32.h>
#include <stdio.h>

bool SofficeRuns()
{
    // check for soffice by searching the communication window
    return ( FindWindowEx( NULL, NULL, QUICKSTART_CLASSNAME, NULL ) == NULL ) ? false : true;
}

bool launchSoffice( )
{
    if ( !SofficeRuns() )
    {
        char filename[_MAX_PATH + 1];

        filename[_MAX_PATH] = 0;
        GetModuleFileName( NULL, filename, _MAX_PATH ); // soffice resides in the same dir
        char *p = strrchr( filename, '\\' );
        if ( !p )
            return false;

        strncpy( p+1, "soffice.exe", _MAX_PATH - (p+1 - filename) );

        char imagename[_MAX_PATH + 1];

        imagename[_MAX_PATH] = 0;
        _snprintf(imagename, _MAX_PATH, "\"%s\" --quickstart", filename );

        UINT ret = WinExec( imagename, SW_SHOW );
        if ( ret < 32 )
            return false;

        return true;
    }
    else
        return true;
}

int APIENTRY WinMain(HINSTANCE /*hInstance*/,
                     HINSTANCE /*hPrevInstance*/,
                     LPSTR     /*lpCmdLine*/,
                     int       /*nCmdShow*/)
{
    // Look for --killtray argument

    for ( int i = 1; i < __argc; i++ )
    {
        if ( 0 == strcmp( __argv[i], "--killtray" ) )
        {
            HWND hwndTray = FindWindowA( QUICKSTART_CLASSNAME, NULL );

            if ( hwndTray )
            {
                UINT uMsgKillTray = RegisterWindowMessage( SHUTDOWN_QUICKSTART_MESSAGE );
                SendMessage( hwndTray, uMsgKillTray, 0, 0 );
            }

            return 0;
        }
    }

    launchSoffice();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
