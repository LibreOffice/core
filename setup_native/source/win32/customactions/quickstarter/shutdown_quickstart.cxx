/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: shutdown_quickstart.cxx,v $
 * $Revision: 1.6 $
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

#include "quickstarter.hxx"
#include <setup_native/qswin32.h>

static BOOL CALLBACK EnumWindowsProc( HWND hWnd, LPARAM lParam )
{
    MSIHANDLE   hMSI = static_cast< MSIHANDLE >( lParam );
    CHAR    szClassName[sizeof(QUICKSTART_CLASSNAMEA) + 1];

    int nCharsCopied = GetClassName( hWnd, szClassName, sizeof( szClassName ) );

    if ( nCharsCopied && !stricmp( QUICKSTART_CLASSNAMEA, szClassName ) )
    {
        DWORD   dwProcessId;

        if ( GetWindowThreadProcessId( hWnd, &dwProcessId ) )
        {
            std::string sImagePath = GetProcessImagePath( dwProcessId );
            std::string sOfficeImageDir = GetOfficeInstallationPath( hMSI ) + "program\\";

            if ( !strnicmp( sImagePath.c_str(), sOfficeImageDir.c_str(), sOfficeImageDir.length() ) )
            {
                UINT    uMsgShutdownQuickstart = RegisterWindowMessageA( SHUTDOWN_QUICKSTART_MESSAGEA );

                if ( uMsgShutdownQuickstart )
                    SendMessageA( hWnd, uMsgShutdownQuickstart, 0, 0 );


                HANDLE  hProcess = OpenProcess( SYNCHRONIZE, FALSE, dwProcessId );

                if ( hProcess )
                {
                    WaitForSingleObject( hProcess, 30000 ); // Wait at most 30 seconds for process to terminate
                    CloseHandle( hProcess );
                }

                return FALSE;
            }

        }
    }

    return TRUE;
}


extern "C" UINT __stdcall ShutDownQuickstarter( MSIHANDLE hMSI )
{
    EnumWindows( EnumWindowsProc, hMSI );

    return ERROR_SUCCESS;
}

