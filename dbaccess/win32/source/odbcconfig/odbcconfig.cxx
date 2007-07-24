/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: odbcconfig.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 12:11:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#pragma once
#ifndef __cplusplus
#error Need C++ to compile
#endif

#define UNICODE
#define _UNICODE
#include <tchar.h>

#pragma warning(push, 1)
#pragma warning(disable:4005)

#include <windows.h>
#include <shellapi.h>
#include <sqlext.h>

#pragma warning(pop)

// the name of the library which contains the SQLManageDataSources function
#define ODBC_UI_LIB_NAME    L"ODBCCP32.DLL"

// the signature of the SQLManageDataSources function
typedef SQLRETURN (SQL_API* TSQLManageDataSource) (SQLHWND hwndParent);

// displays the error text for the last error (GetLastError), and returns this error value
int displayLastError()
{
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

    return dwError;
}

/** registers the window class for our application's main window
*/
BOOL registerWindowClass( HINSTANCE _hAppInstance )
{
    WNDCLASSEX wcx;

    wcx.cbSize = sizeof(wcx);                   // size of structure
    wcx.style = CS_HREDRAW | CS_VREDRAW;        // redraw if size changes
    wcx.lpfnWndProc = DefWindowProc;            // points to window procedure
    wcx.cbClsExtra = 0;                         // no extra class memory
    wcx.cbWndExtra = 0;                         // no extra window memory
    wcx.hInstance = _hAppInstance;              // handle to instance
    wcx.hIcon = NULL;                           // predefined app. icon
    wcx.hCursor = NULL;                         // predefined arrow
    wcx.hbrBackground = NULL;                   // no background brush
    wcx.lpszMenuName =  NULL;                   // name of menu resource
    wcx.lpszClassName = L"ODBCConfigMainClass"; // name of window class
    wcx.hIconSm = NULL;                         // small class icon

    return ( NULL != RegisterClassEx( &wcx ) );
}

/// initializes the application instances
HWND initInstance( HINSTANCE _hAppInstance )
{
    HWND hWindow = CreateWindow(
        L"ODBCConfigMainClass", // name of window class
        L"ODBC Config Wrapper", // title-bar string
        WS_OVERLAPPEDWINDOW,    // top-level window
        CW_USEDEFAULT,          // default horizontal position
        CW_USEDEFAULT,          // default vertical position
        CW_USEDEFAULT,          // default width
        CW_USEDEFAULT,          // default height
        (HWND) NULL,            // no owner window
        (HMENU) NULL,           // use class menu
        _hAppInstance,          // handle to application instance
        (LPVOID) NULL);         // no window-creation data

    // don't show the window, we only need it as parent handle for the
    // SQLManageDataSources function
    return hWindow;
}

// main window function
extern "C" int APIENTRY _tWinMain( HINSTANCE _hAppInstance, HINSTANCE, LPTSTR, int )
{
    if ( !registerWindowClass( _hAppInstance ) )
        return FALSE;

    HWND hAppWindow = initInstance( _hAppInstance );
    if ( !IsWindow( hAppWindow ) )
        return displayLastError();

    HMODULE hModule = LoadLibraryW( ODBC_UI_LIB_NAME );
    if ( hModule == NULL )
        hModule = LoadLibraryExW( ODBC_UI_LIB_NAME, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
    if ( hModule == NULL )
        return displayLastError();

    FARPROC pManageDSProc = GetProcAddress( hModule, "SQLManageDataSources" );
    if ( pManageDSProc == NULL )
        return displayLastError();

    TSQLManageDataSource pManageDS = (TSQLManageDataSource)pManageDSProc;
    if ( !( (*pManageDS)( hAppWindow ) ) )
        return displayLastError();

    FreeLibrary( hModule );

    return 0;
}

