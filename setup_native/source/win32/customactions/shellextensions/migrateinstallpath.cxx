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

#include "shlxtmsi.hxx"
#include <algorithm>
#include <systools/win32/uwinapi.h>

extern "C" UINT __stdcall MigrateInstallPath( MSIHANDLE handle )
{
    WCHAR   szValue[8192];
    DWORD   nValueSize = sizeof(szValue); // yes, it is the number of bytes
    HKEY    hKey;
    std::wstring   sInstDir;

    std::wstring   sManufacturer = GetMsiPropertyW( handle, L"Manufacturer" );
    std::wstring   sDefinedName = GetMsiPropertyW( handle, L"DEFINEDPRODUCT" );
    std::wstring   sUpdateVersion = GetMsiPropertyW( handle, L"DEFINEDVERSION" );
    std::wstring   sUpgradeCode = GetMsiPropertyW( handle, L"UpgradeCode" );

    std::wstring   sProductKey = L"Software\\" + sManufacturer + L"\\" + sDefinedName +
                                        L"\\" + sUpdateVersion + L"\\" + sUpgradeCode;

    if ( ERROR_SUCCESS == RegOpenKeyW( HKEY_CURRENT_USER,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueExW( hKey, L"INSTALLLOCATION", nullptr, nullptr, reinterpret_cast<LPBYTE>(szValue), &nValueSize ) )
        {
            szValue[std::min(SAL_N_ELEMENTS(szValue) - 1, nValueSize / sizeof(*szValue))] = 0;
            sInstDir = szValue;
            MsiSetPropertyW(handle, L"INSTALLLOCATION", sInstDir.c_str());
            // MessageBoxW( NULL, sInstDir.c_str(), L"Found in HKEY_CURRENT_USER", MB_OK );
        }

        RegCloseKey( hKey );
    }
    else if ( ERROR_SUCCESS == RegOpenKeyW( HKEY_LOCAL_MACHINE,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueExW( hKey, L"INSTALLLOCATION", nullptr, nullptr, reinterpret_cast<LPBYTE>(szValue), &nValueSize ) )
        {
            szValue[std::min(SAL_N_ELEMENTS(szValue) - 1, nValueSize / sizeof(*szValue))] = 0;
            sInstDir = szValue;
            MsiSetPropertyW(handle, L"INSTALLLOCATION", sInstDir.c_str());
            // MessageBoxW( NULL, sInstDir.c_str(), L"Found in HKEY_LOCAL_MACHINE", MB_OK );
        }

        RegCloseKey( hKey );
    }

    return ERROR_SUCCESS;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
