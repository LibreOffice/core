/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: officeloader.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:52:32 $
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
#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define _UNICODE
#include <tchar.h>

#include <string.h>
#include <stdlib.h>
#include <systools/win32/uwinapi.h>

#include "../../../source/inc/exithelper.hxx"


//---------------------------------------------------------------------------

int WINAPI _tWinMain( HINSTANCE, HINSTANCE, LPTSTR, int )
{
    TCHAR               szTargetFileName[MAX_PATH] = TEXT("");
    STARTUPINFO         aStartupInfo;

    ZeroMemory( &aStartupInfo, sizeof(aStartupInfo) );
    aStartupInfo.cb = sizeof(aStartupInfo);

    GetStartupInfo( &aStartupInfo );
    // Get image path with same name but with .bin extension

    TCHAR               szModuleFileName[MAX_PATH];

    GetModuleFileName( NULL, szModuleFileName, MAX_PATH );
    _TCHAR  *lpLastDot = _tcsrchr( szModuleFileName, '.' );
    if ( lpLastDot && 0 == _tcsicmp( lpLastDot, _T(".EXE") ) )
    {
        size_t len = lpLastDot - szModuleFileName;
        _tcsncpy( szTargetFileName, szModuleFileName, len );
        _tcsncpy( szTargetFileName + len, _T(".BIN"), sizeof(szTargetFileName)/sizeof(szTargetFileName[0]) - len );
    }

    // Create process with same command line, environment and stdio handles which
    // are directed to the created pipes

    DWORD   dwExitCode = (DWORD)-1;
    BOOL    fSuccess = FALSE;

    do
    {
        PROCESS_INFORMATION aProcessInfo;

        fSuccess = CreateProcess(
            szTargetFileName,
            // When restarting office process do not pass a command line
            fSuccess ? NULL : GetCommandLine(),
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &aStartupInfo,
            &aProcessInfo );

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
    } while ( fSuccess && ::desktop::ExitHelper::E_CRASH_WITH_RESTART == dwExitCode );

    return fSuccess ? dwExitCode : -1;
}
