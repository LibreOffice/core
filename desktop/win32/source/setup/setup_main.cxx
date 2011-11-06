/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    m_bAdministrative = false;
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

        if ( ! pSetup->IsAdminInstall() )
            if ( ! pSetup->GetPatches() )
                throw pSetup->GetError();

        // CheckForUpgrade() has to be called after calling GetPatches()!
        if ( ! pSetup->CheckForUpgrade() )
            throw pSetup->GetError();

        long nLanguage;

        if ( ! pSetup->ChooseLanguage( nLanguage ) )
            throw pSetup->GetError();

        if ( ! pSetup->InstallRuntimes() )
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
