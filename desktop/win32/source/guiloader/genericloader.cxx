/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: genericloader.cxx,v $
 * $Revision: 1.4 $
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
        _tcslen(szIniDirectory) + MY_LENGTH(_T("redirect.ini\"")) + 1];
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

