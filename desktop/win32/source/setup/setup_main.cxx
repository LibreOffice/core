/*************************************************************************
 *
 *  $RCSfile: setup_main.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: svesik $ $Date: 2004-04-20 12:40:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <windows.h>
#include <new>

#include "setup_main.hxx"

//--------------------------------------------------------------------------

void __cdecl newhandler()
{
    throw std::bad_alloc();
    return;
}

//--------------------------------------------------------------------------

SetupApp::SetupApp()
{
    m_uiRet         = ERROR_SUCCESS;

    // Get OS version
    OSVERSIONINFO sInfoOS;

    ZeroMemory( &sInfoOS, sizeof(OSVERSIONINFO) );
    sInfoOS.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

    GetVersionEx( &sInfoOS );

    m_nOSVersion    = sInfoOS.dwMajorVersion;
    m_bIsWin9x      = ( VER_PLATFORM_WIN32_NT != sInfoOS.dwPlatformId );
}

//--------------------------------------------------------------------------

SetupApp::~SetupApp()
{
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

extern "C" int __stdcall WinMain( HINSTANCE hInst, HINSTANCE hPrevInst,
                                  LPSTR lpszCmdLine, int nCmdShow )
{
    // Get OS version
    OSVERSIONINFO sInfoOS;

    ZeroMemory( &sInfoOS, sizeof(OSVERSIONINFO) );
    sInfoOS.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

    GetVersionEx( &sInfoOS );

    boolean bIsWin9x = ( VER_PLATFORM_WIN32_NT != sInfoOS.dwPlatformId );

    SetupApp *pSetup;

    if ( bIsWin9x )
        pSetup = Create_SetupAppA();
    else
        pSetup = Create_SetupAppW();

    try
    {
        if ( ! pSetup->Initialize( hInst ) )
            throw pSetup->GetError();

        if ( pSetup->AlreadyRunning() )
            throw (UINT) ERROR_INSTALL_ALREADY_RUNNING;

        if ( ! pSetup->ReadProfile() )
            throw pSetup->GetError();

        if ( ! pSetup->CheckVersion() )
            throw pSetup->GetError();

        long nLanguage;

        if ( ! pSetup->ChooseLanguage( nLanguage ) )
            throw pSetup->GetError();

        if ( ! pSetup->Install( nLanguage ) )
            throw pSetup->GetError();
    }
    catch ( std::bad_alloc )
    {
        pSetup->DisplayError( ERROR_OUTOFMEMORY );
    }
    catch ( UINT nErr )
    {
        pSetup->DisplayError( nErr );
    }

    int nRet = pSetup->GetError();

    delete pSetup;

    return nRet;
}
