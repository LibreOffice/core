/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#define UNICODE
#define _UNICODE

#include "docrecovery.hxx"

#undef WB_LEFT
#undef WB_RIGHT

#include <prewin.h>
#include <postwin.h>
#include <tchar.h>
#include <stdio.h>
#include <systools/win32/uwinapi.h>
#include <sal/macros.h>

// need to undef min and max macros from MS headers here to make
// the std::min and std::max from stl visible again
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

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
        lResult = RegSetValueEx( hKey, lpValueName, 0, dwType, (CONST sal_uInt8 *)lpData, cbData );
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
                TEXT("SOFTWARE\\LibreOffice\\CrashReport"),
                TEXT("HTTPProxyServer"),
                szBuffer,
                sizeof(szBuffer) ) )
                maParams.maHTTPProxyServer = (sal_Unicode *)szBuffer;

            DWORD   dwProxyPort;
            if ( ERROR_SUCCESS == RegReadValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\LibreOffice\\CrashReport"),
                TEXT("HTTPProxyPort"),
                &dwProxyPort,
                sizeof(dwProxyPort) ) )
            {
                _stprintf( szBuffer, _T("%d"), dwProxyPort );
                maParams.maHTTPProxyPort = (sal_Unicode *)szBuffer;
            }

            if ( ERROR_SUCCESS == RegReadValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\LibreOffice\\CrashReport"),
                TEXT("ReturnAddress"),
                szBuffer,
                sizeof(szBuffer) ) )
                maEMailAddrED.SetText(OUString((sal_Unicode *)szBuffer));

            DWORD   fAllowContact = sal_False;
            RegReadValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\LibreOffice\\CrashReport"),
                TEXT("AllowContact"),
                &fAllowContact,
                sizeof(fAllowContact) );
            maContactCB.Check( (sal_Bool)fAllowContact );

            DWORD   uInternetConnection = 0;
            RegReadValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\LibreOffice\\CrashReport"),
                TEXT("HTTPConnection"),
                &uInternetConnection,
                sizeof(uInternetConnection) );
            maParams.miHTTPConnectionType = uInternetConnection;

            return true;
        }

        bool ErrorRepSendDialog::SaveParams()
        {
            const _TCHAR    *lpHTTPProxyServer = reinterpret_cast<LPCTSTR>(maParams.maHTTPProxyServer.getStr());
            RegWriteValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\LibreOffice\\CrashReport"),
                TEXT("HTTPProxyServer"), REG_SZ,
                lpHTTPProxyServer,
                sizeof(TCHAR) * (_tcslen(lpHTTPProxyServer) + 1) );

            _TCHAR* endptr = NULL;
            DWORD dwProxyPort = _tcstoul( reinterpret_cast<LPCTSTR>(maParams.maHTTPProxyPort.getStr()), &endptr, 10 );

            RegWriteValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\LibreOffice\\CrashReport"),
                TEXT("HTTPProxyPort"), REG_DWORD,
                &dwProxyPort,
                sizeof(DWORD) );

            DWORD   fAllowContact = IsContactAllowed();
            RegWriteValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\LibreOffice\\CrashReport"),
                TEXT("AllowContact"), REG_DWORD,
                &fAllowContact,
                sizeof(DWORD) );


            DWORD uInternetConnection = maParams.miHTTPConnectionType;

            RegWriteValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\LibreOffice\\CrashReport"),
                TEXT("HTTPConnection"), REG_DWORD,
                &uInternetConnection,
                sizeof(DWORD) );

            const _TCHAR    *lpEmail = reinterpret_cast<LPCTSTR>(GetEMailAddress().getStr());
            RegWriteValue(
                HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\LibreOffice\\CrashReport"),
                TEXT("ReturnAddress"), REG_SZ,
                lpEmail,
                sizeof(TCHAR) * (_tcslen(lpEmail) + 1) );

            return true;
        }

        bool ErrorRepSendDialog::SendReport()
        {
            TCHAR   szTempPath[MAX_PATH];
            TCHAR   szFileName[MAX_PATH];

            GetTempPath( SAL_N_ELEMENTS(szTempPath), szTempPath );
            GetTempFileName( szTempPath, TEXT("DSC"), 0, szFileName );

            FILE *fp = _tfopen( szFileName, _T("wb") );

            if ( fp )
            {
                OString strUTF8(OUStringToOString(GetUsing(),
                    RTL_TEXTENCODING_UTF8));

                fwrite( strUTF8.getStr(), 1, strUTF8.getLength(), fp );
                fclose( fp );
            }

            SetEnvironmentVariable( TEXT("ERRORREPORT_SUBJECT"), reinterpret_cast<LPCTSTR>(GetDocType().getStr()) );
            SetEnvironmentVariable( TEXT("ERRORREPORT_BODYFILE"), szFileName );

            _TCHAR  szBuffer[1024];
            TCHAR   szPath[MAX_PATH];
            LPTSTR  lpFilePart;
            PROCESS_INFORMATION ProcessInfo;
            STARTUPINFO StartupInfo;

            if ( SearchPath( NULL, TEXT("crashrep.exe"), NULL, MAX_PATH, szPath, &lpFilePart ) )
            {
                ZeroMemory( &StartupInfo, sizeof(StartupInfo) );
                StartupInfo.cb = sizeof(StartupInfo.cb);

                sntprintf( szBuffer, SAL_N_ELEMENTS(szBuffer),
                    _T("%s -noui -load -send"),
                    szPath );

                if (
                    CreateProcess(
                        NULL,
                        szBuffer,
                        NULL,
                        NULL,
                        sal_False,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
