/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shutdown_quickstart.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:34:27 $
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

