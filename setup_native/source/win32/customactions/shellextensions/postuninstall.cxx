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

#define _WIN32_WINDOWS 0x0410
#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <../tools/msiprop.hxx>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>

#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif
#include <tchar.h>
#include <string>

#include <io.h>

static BOOL ExecuteCommand( LPCTSTR lpCommand, BOOL bSync )
{
    BOOL                fSuccess = FALSE;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);

    fSuccess = CreateProcess(
        NULL,
        (LPTSTR)lpCommand,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
        );

    if ( fSuccess )
    {
        if ( bSync )
            WaitForSingleObject( pi.hProcess, INFINITE );

        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
    }

    return fSuccess;
}

extern "C" UINT __stdcall ExecutePostUninstallScript( MSIHANDLE handle )
{
    TCHAR   szValue[8192];
    DWORD   nValueSize = sizeof(szValue);
    HKEY    hKey;
    std::_tstring   sInstDir;

    std::_tstring   sProductKey = GetMsiPropValue( handle, TEXT("FINDPRODUCT") );

    // MessageBox( NULL, sProductKey.c_str(), "Titel", MB_OK );

    if ( ERROR_SUCCESS == RegOpenKey( HKEY_CURRENT_USER,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("INSTALLLOCATION"), NULL, NULL, (LPBYTE)szValue, &nValueSize ) )
        {
            sInstDir = szValue;
        }
        RegCloseKey( hKey );
    }
    else if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("INSTALLLOCATION"), NULL, NULL, (LPBYTE)szValue, &nValueSize ) )
        {
            sInstDir = szValue;
        }
        RegCloseKey( hKey );
    }
    else
        return ERROR_SUCCESS;

    std::_tstring   sInfFile = sInstDir + TEXT("program\\postuninstall.inf");
    std::_tstring   sCommand = _T("RUNDLL32.EXE ");

    // MessageBox( NULL, sInfFile.c_str(), "Titel", MB_OK );

    sCommand += _T("setupapi.dll");

    sCommand += _T(",InstallHinfSection PostUninstall 132 ");
    sCommand += sInfFile;

    if ( 0 == _taccess( sInfFile.c_str(), 2 ) )
        ExecuteCommand( sCommand.c_str(), TRUE );

    DeleteFile( sInfFile.c_str() );

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
