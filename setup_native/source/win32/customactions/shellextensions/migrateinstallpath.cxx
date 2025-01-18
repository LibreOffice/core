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
#include <sstream>
#include <systools/win32/extended_max_path.hxx>
#include <systools/win32/uwinapi.h>

extern "C" __declspec(dllexport) UINT __stdcall MigrateInstallPath(MSIHANDLE handle)
{
    std::wstring sInstDir = GetMsiPropertyW(handle, L"INSTALLLOCATION");
    if (!sInstDir.empty())
        return ERROR_SUCCESS; // Don't overwrite explicitly set value

    auto RegValue = [](HKEY hRoot, const WCHAR* sKey, const WCHAR* sVal) {
        std::wstring sResult;
        WCHAR buf[EXTENDED_MAX_PATH]; // max longpath
        DWORD bufsize = sizeof(buf); // yes, it is the number of bytes
        if (RegGetValueW(hRoot, sKey, sVal, RRF_RT_REG_SZ, nullptr, buf, &bufsize) == ERROR_SUCCESS)
            sResult = buf; // RegGetValueW null-terminates strings

        return sResult;
    };

    const std::wstring sManufacturer = GetMsiPropertyW( handle, L"Manufacturer" );
    const std::wstring sDefinedName = GetMsiPropertyW( handle, L"DEFINEDPRODUCT" );
    const std::wstring sUpdateVersion = GetMsiPropertyW( handle, L"DEFINEDVERSION" );
    const std::wstring sUpgradeCode = GetMsiPropertyW( handle, L"UpgradeCode" );
    const std::wstring sBrandPackageVersion = GetMsiPropertyW(handle, L"BRANDPACKAGEVERSION");

    std::wstring   sKey = L"Software\\" + sManufacturer + L"\\" + sDefinedName +
                                        L"\\" + sUpdateVersion + L"\\" + sUpgradeCode;

    sInstDir = RegValue(HKEY_CURRENT_USER, sKey.c_str(), L"INSTALLLOCATION");
    if (sInstDir.empty())
        sInstDir = RegValue(HKEY_LOCAL_MACHINE, sKey.c_str(), L"INSTALLLOCATION");
    // See #i93032# for layers description
    if (sInstDir.empty())
    {
        sKey = L"Software\\LibreOffice\\Layers\\" + sDefinedName + L"\\" + sBrandPackageVersion;
        sInstDir = RegValue(HKEY_CURRENT_USER, sKey.c_str(), L"INSTALLLOCATION");
    }
    if (sInstDir.empty())
    {
        sKey = L"Software\\LibreOffice\\Layers_\\" + sDefinedName + L"\\" + sBrandPackageVersion;
        sInstDir = RegValue(HKEY_CURRENT_USER, sKey.c_str(), L"INSTALLLOCATION");
    }
    if (sInstDir.empty())
    {
        sKey = L"Software\\LibreOffice\\Layers\\" + sDefinedName + L"\\" + sBrandPackageVersion;
        sInstDir = RegValue(HKEY_LOCAL_MACHINE, sKey.c_str(), L"INSTALLLOCATION");
    }
    if (sInstDir.empty())
    {
        sKey = L"Software\\LibreOffice\\Layers_\\" + sDefinedName + L"\\" + sBrandPackageVersion;
        sInstDir = RegValue(HKEY_LOCAL_MACHINE, sKey.c_str(), L"INSTALLLOCATION");
    }
    if (sInstDir.empty())
    {
        std::wistringstream sOlds{ GetMsiPropertyW(handle, L"OLDPRODUCTS") };
        std::wstring sOld;
        while (std::getline(sOlds, sOld, L';'))
        {
            if (sOld.empty())
                continue;
            sKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + sOld;
            sInstDir = RegValue(HKEY_LOCAL_MACHINE, sKey.c_str(), L"InstallLocation");
            if (!sInstDir.empty())
                break;
        }
    }

    if (!sInstDir.empty())
        MsiSetPropertyW(handle, L"INSTALLLOCATION", sInstDir.c_str());

    return ERROR_SUCCESS;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
