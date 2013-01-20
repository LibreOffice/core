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

using namespace std;

extern "C" UINT __stdcall MigrateInstallPath( MSIHANDLE handle )
{
    TCHAR   szValue[8192];
    DWORD   nValueSize = sizeof(szValue);
    HKEY    hKey;
    std::_tstring   sInstDir;

    std::_tstring   sManufacturer = GetMsiPropValue( handle, TEXT("Manufacturer") );
    std::_tstring   sDefinedName = GetMsiPropValue( handle, TEXT("DEFINEDPRODUCT") );
    std::_tstring   sUpdateVersion = GetMsiPropValue( handle, TEXT("DEFINEDVERSION") );
    std::_tstring   sUpgradeCode = GetMsiPropValue( handle, TEXT("UpgradeCode") );

    std::_tstring   sProductKey = "Software\\" + sManufacturer + "\\" + sDefinedName +
                                        "\\" + sUpdateVersion + "\\" + sUpgradeCode;

    if ( ERROR_SUCCESS == RegOpenKey( HKEY_CURRENT_USER,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("INSTALLLOCATION"), NULL, NULL, (LPBYTE)szValue, &nValueSize ) )
        {
            sInstDir = szValue;
            MsiSetProperty(handle, TEXT("INSTALLLOCATION"), sInstDir.c_str());
            // MessageBox( NULL, sInstDir.c_str(), "Found in HKEY_CURRENT_USER", MB_OK );
        }

        RegCloseKey( hKey );
    }
    else if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("INSTALLLOCATION"), NULL, NULL, (LPBYTE)szValue, &nValueSize ) )
        {
            sInstDir = szValue;
            MsiSetProperty(handle, TEXT("INSTALLLOCATION"), sInstDir.c_str());
            // MessageBox( NULL, sInstDir.c_str(), "Found in HKEY_LOCAL_MACHINE", MB_OK );
        }

        RegCloseKey( hKey );
    }

    return ERROR_SUCCESS;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
