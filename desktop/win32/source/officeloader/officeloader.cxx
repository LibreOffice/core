//*************************************************************************
//
//   $RCSfile: officeloader.cxx,v $
//
//   $Revision: 1.2 $
//
//   last change: $Author: rt $ $Date: 2004-11-26 14:49:57 $
//
//   The Contents of this file are made available subject to the terms of
//   either of the following licenses
//
//          - GNU Lesser General Public License Version 2.1
//          - Sun Industry Standards Source License Version 1.1
//
//   Sun Microsystems Inc., October, 2000
//
//   GNU Lesser General Public License Version 2.1
//   =============================================
//   Copyright 2000 by Sun Microsystems, Inc.
//   901 San Antonio Road, Palo Alto, CA 94303, USA
//
//   This library is free software; you can redistribute it and/or
//   modify it under the terms of the GNU Lesser General Public
//   License version 2.1, as published by the Free Software Foundation.
//
//   This library is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   Lesser General Public License for more details.
//
//   You should have received a copy of the GNU Lesser General Public
//   License along with this library; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//   MA  02111-1307  USA
//
//
//   Sun Industry Standards Source License Version 1.1
//   =================================================
//   The contents of this file are subject to the Sun Industry Standards
//   Source License Version 1.1 (the "License"); You may not use this file
//   except in compliance with the License. You may obtain a copy of the
//   License at http://www.openoffice.org/license.html.
//
//   Software provided under this License is provided on an "AS IS" basis,
//   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
//   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
//   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
//   See the License for the specific provisions governing your rights and
//   obligations concerning the Software.
//
//   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
//
//   Copyright: 2000 by Sun Microsystems, Inc.
//
//   All Rights Reserved.
//
//   Contributor(s): _______________________________________
//
//
//
//*************************************************************************
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
            WaitForSingleObject( aProcessInfo.hProcess, INFINITE );
            dwExitCode = 0;
            GetExitCodeProcess( aProcessInfo.hProcess, &dwExitCode );

            CloseHandle( aProcessInfo.hProcess );
            CloseHandle( aProcessInfo.hThread );
        }
    } while ( fSuccess && ::desktop::ExitHelper::E_CRASH_WITH_RESTART == dwExitCode );

    return fSuccess ? dwExitCode : -1;
}
