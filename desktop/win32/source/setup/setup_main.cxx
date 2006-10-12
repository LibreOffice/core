/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: setup_main.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 14:31:43 $
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
#include "precompiled_desktop.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
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
    m_nMinorVersion = sInfoOS.dwMinorVersion;
    m_bIsWin9x      = ( VER_PLATFORM_WIN32_NT != sInfoOS.dwPlatformId );
    m_bNeedReboot   = false;
}

//--------------------------------------------------------------------------

SetupApp::~SetupApp()
{
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

extern "C" int __stdcall WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, int )
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
