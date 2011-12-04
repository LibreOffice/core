/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// QuickStart.cpp : Defines the entry point for the application.
//

#include "StdAfx.h"
#include "resource.h"
#include <systools/win32/uwinapi.h>
#include <stdio.h>

// message used to communicate with soffice
#define LISTENER_WINDOWCLASS    "SO Listener Class"
#define KILLTRAY_MESSAGE        "SO KillTray"

bool SofficeRuns()
{
    // check for soffice by searching the communication window
    return ( FindWindowEx( NULL, NULL, LISTENER_WINDOWCLASS, NULL ) == NULL ) ? false : true;
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
            HWND    hwndTray = FindWindow( LISTENER_WINDOWCLASS, NULL );

            if ( hwndTray )
            {
                UINT    uMsgKillTray = RegisterWindowMessage( KILLTRAY_MESSAGE );
                SendMessage( hwndTray, uMsgKillTray, 0, 0 );
            }

            return 0;
        }
    }

    launchSoffice();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
