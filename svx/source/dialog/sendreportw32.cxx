/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sendreportw32.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:38:18 $
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
#include "precompiled_svx.hxx"
#define UNICODE
#define _UNICODE

#include "docrecovery.hxx"
#include <tools/svwin.h>

#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <stdio.h>

#include <systools/win32/uwinapi.h>


//***************************************************************************

static LONG RegReadValue( HKEY hBaseKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, LPVOID lpData, DWORD cbData )
{
    HKEY    hKey = NULL;
    LONG    lResult;

    lResult = RegOpenKeyEx( hBaseKey, lpSubKey, 0, KEY_QUERY_VALUE, &hKey );

    if ( ERROR_SUCCESS == lResult )
    {
        lResult = RegQueryValueEx( hKey, lpValueName, NULL, NULL, (LPBYTE)lpData, &cbData );
        RegCloseKey( hKey );
    }

    return lResult;
}

//***************************************************************************

static LONG RegWriteValue( HKEY hBaseKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, DWORD dwType, LPCVOID lpData, DWORD cbData )
{
    HKEY    hKey = NULL;
    LONG    lResult;

    lResult = RegCreateKeyEx( hBaseKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL );

    if ( ERROR_SUCCESS == lResult )
    {
        lResult = RegSetValueEx( hKey, lpValueName, NULL, dwType, (CONST BYTE *)lpData, cbData );
        RegCloseKey( hKey );
    }

    return lResult;
}

//***************************************************************************

namespace svx{
    namespace DocRecovery{

        bool ErrorRepSendDialog::ReadParams()
        {
            _TCHAR  szBuffer[2048];

            if ( ERROR_SUCCESS == RegReadValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
                TEXT("HTTPProxyServer"),
                szBuffer,
                sizeof(szBuffer) ) )
                maParams.maHTTPProxyServer = (sal_Unicode *)szBuffer;

            DWORD   dwProxyPort;
            if ( ERROR_SUCCESS == RegReadValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
                TEXT("HTTPProxyPort"),
                &dwProxyPort,
                sizeof(dwProxyPort) ) )
            {
                _stprintf( szBuffer, _T("%d"), dwProxyPort );
                maParams.maHTTPProxyPort = (sal_Unicode *)szBuffer;
            }

            if ( ERROR_SUCCESS == RegReadValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
                TEXT("ReturnAddress"),
                szBuffer,
                sizeof(szBuffer) ) )
                maEMailAddrED.SetText( (sal_Unicode *)szBuffer );

            DWORD   fAllowContact = FALSE;
            RegReadValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
                TEXT("AllowContact"),
                &fAllowContact,
                sizeof(fAllowContact) );
            maContactCB.Check( fAllowContact );

            DWORD   uInternetConnection = 0;
            RegReadValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
                TEXT("HTTPConnection"),
                &uInternetConnection,
                sizeof(uInternetConnection) );
            maParams.miHTTPConnectionType = uInternetConnection;

            return true;
        }

        bool ErrorRepSendDialog::SaveParams()
        {
            const _TCHAR    *lpHTTPProxyServer = maParams.maHTTPProxyServer.GetBuffer();
            RegWriteValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
                TEXT("HTTPProxyServer"), REG_SZ,
                lpHTTPProxyServer,
                sizeof(TCHAR) * (_tcslen(lpHTTPProxyServer) + 1) );

            _TCHAR* endptr = NULL;
            DWORD dwProxyPort = _tcstoul( maParams.maHTTPProxyPort.GetBuffer(), &endptr, 10 );

            RegWriteValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
                TEXT("HTTPProxyPort"), REG_DWORD,
                &dwProxyPort,
                sizeof(DWORD) );

            DWORD   fAllowContact = IsContactAllowed();
            RegWriteValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
                TEXT("AllowContact"), REG_DWORD,
                &fAllowContact,
                sizeof(DWORD) );


            DWORD uInternetConnection = maParams.miHTTPConnectionType;

            RegWriteValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
                TEXT("HTTPConnection"), REG_DWORD,
                &uInternetConnection,
                sizeof(DWORD) );

            const _TCHAR    *lpEmail = GetEMailAddress().GetBuffer();
            RegWriteValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\OpenOffice.org\\CrashReport"),
                TEXT("ReturnAddress"), REG_SZ,
                lpEmail,
                sizeof(TCHAR) * (_tcslen(lpEmail) + 1) );

            return true;
        }

        bool ErrorRepSendDialog::SendReport()
        {
            TCHAR   szTempPath[MAX_PATH];
            TCHAR   szFileName[MAX_PATH];

            GetTempPath( elementsof(szTempPath), szTempPath );
            GetTempFileName( szTempPath, TEXT("DSC"), 0, szFileName );

            FILE *fp = _tfopen( szFileName, _T("wb") );

            if ( fp )
            {
                ByteString  strUTF8( GetUsing(), RTL_TEXTENCODING_UTF8 );

                fwrite( strUTF8.GetBuffer(), 1, strUTF8.Len(), fp );
                fclose( fp );
            }

            SetEnvironmentVariable( TEXT("ERRORREPORT_SUBJECT"), GetDocType().GetBuffer() );
            SetEnvironmentVariable( TEXT("ERRORREPORT_BODYFILE"), szFileName );

            BOOL    fSuccess = FALSE;
            _TCHAR  szBuffer[1024];
            TCHAR   szPath[MAX_PATH];
            LPTSTR  lpFilePart;
            PROCESS_INFORMATION ProcessInfo;
            STARTUPINFO StartupInfo;

            if ( SearchPath( NULL, TEXT("crashrep.exe"), NULL, MAX_PATH, szPath, &lpFilePart ) )
            {
                ZeroMemory( &StartupInfo, sizeof(StartupInfo) );
                StartupInfo.cb = sizeof(StartupInfo.cb);

                sntprintf( szBuffer, elementsof(szBuffer),
                    _T("%s -noui -load -send"),
                    szPath );

                if (
                    CreateProcess(
                        NULL,
                        szBuffer,
                        NULL,
                        NULL,
                        FALSE,
                        0,
                        NULL, NULL, &StartupInfo, &ProcessInfo )
                    )
                {
                    DWORD   dwExitCode;

                    WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
                    if ( GetExitCodeProcess( ProcessInfo.hProcess, &dwExitCode ) && 0 == dwExitCode )
                        return true;

                }
            }

            DeleteFile( szFileName );


            return false;
        }


    }   // namespace DocRecovery
}   // namespace svx
