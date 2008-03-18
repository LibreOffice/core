/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: genericloader.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 13:53:29 $
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
#define UNICODE
#define _UNICODE

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <shellapi.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <tchar.h>

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <systools/win32/uwinapi.h>

#include "../extendloaderenvironment.hxx"

//---------------------------------------------------------------------------

static int GenericMain()
{
    TCHAR               szTargetFileName[MAX_PATH];
    TCHAR               szIniDirectory[MAX_PATH];
    STARTUPINFO         aStartupInfo;

    desktop_win32::extendLoaderEnvironment(szTargetFileName, szIniDirectory);

    ZeroMemory( &aStartupInfo, sizeof(aStartupInfo) );
    aStartupInfo.cb = sizeof(aStartupInfo);

    GetStartupInfo( &aStartupInfo );

    DWORD   dwExitCode = (DWORD)-1;

    PROCESS_INFORMATION aProcessInfo;

    LPTSTR cl1 = GetCommandLine();
    LPTSTR cl2 = new TCHAR[
        _tcslen(cl1) + MY_LENGTH(_T(" \"-env:INIFILEPATH=")) +
        _tcslen(szIniDirectory) + MY_LENGTH(_T("redirect.ini\""))];
    _tcscpy(cl2, cl1);
    _tcscat(cl2, _T(" \"-env:INIFILEPATH="));
    _tcscat(cl2, szIniDirectory);
    _tcscat(cl2, _T("redirect.ini\""));

    BOOL fSuccess = CreateProcess(
        szTargetFileName,
        cl2,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &aStartupInfo,
        &aProcessInfo );

    delete[] cl2;

    if ( fSuccess )
    {
        DWORD   dwWaitResult;

        do
        {
            // On Windows XP it seems as the desktop calls WaitForInputIdle after "OpenWidth" so we have to do so
            // as if we where processing any messages

            dwWaitResult = MsgWaitForMultipleObjects( 1, &aProcessInfo.hProcess, FALSE, INFINITE, QS_ALLEVENTS );

            if (  WAIT_OBJECT_0 + 1 == dwWaitResult )
            {
                MSG msg;

                PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
            }
        } while ( WAIT_OBJECT_0 + 1 == dwWaitResult );

        dwExitCode = 0;
        GetExitCodeProcess( aProcessInfo.hProcess, &dwExitCode );

        CloseHandle( aProcessInfo.hProcess );
        CloseHandle( aProcessInfo.hThread );
    }

    return dwExitCode;
}

//---------------------------------------------------------------------------

#ifdef __MINGW32__
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
#else
int WINAPI _tWinMain( HINSTANCE, HINSTANCE, LPTSTR, int )
#endif
{
    return GenericMain();
}

//---------------------------------------------------------------------------

#ifdef __MINGW32__
int __cdecl main()
#else
int __cdecl _tmain()
#endif
{
    return GenericMain();
}

