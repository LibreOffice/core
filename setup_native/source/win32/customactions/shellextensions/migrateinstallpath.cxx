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
#include <systools/win32/uwinapi.h>

extern "C" __declspec(dllexport) UINT __stdcall MigrateInstallPath(MSIHANDLE handle)
{
    auto RegValue = [](HKEY hRoot, const WCHAR* sKey, const WCHAR* sVal) {
        std::wstring sResult;

        if (HKEY hKey; RegOpenKeyW(hRoot, sKey, &hKey) == ERROR_SUCCESS)
        {
            WCHAR buf[32767]; // max longpath
            DWORD bufsize = sizeof(buf); // yes, it is the number of bytes
            if (RegQueryValueExW(hKey, sVal, nullptr, nullptr, reinterpret_cast<LPBYTE>(buf),
                                 &bufsize)
                == ERROR_SUCCESS)
            {
                buf[std::min<size_t>(SAL_N_ELEMENTS(buf) - 1, bufsize / sizeof(*buf))] = 0;
                sResult = buf;
            }
            RegCloseKey(hKey);
        }

        return sResult;
    };

    std::wstring   sManufacturer = GetMsiPropertyW( handle, L"Manufacturer" );
    std::wstring   sDefinedName = GetMsiPropertyW( handle, L"DEFINEDPRODUCT" );
    std::wstring   sUpdateVersion = GetMsiPropertyW( handle, L"DEFINEDVERSION" );
    std::wstring   sUpgradeCode = GetMsiPropertyW( handle, L"UpgradeCode" );

    std::wstring   sKey = L"Software\\" + sManufacturer + L"\\" + sDefinedName +
                                        L"\\" + sUpdateVersion + L"\\" + sUpgradeCode;

    if (auto sInstDir = RegValue(HKEY_CURRENT_USER, sKey.c_str(), L"INSTALLLOCATION");
        !sInstDir.empty())
    {
        MsiSetPropertyW(handle, L"INSTALLLOCATION", sInstDir.c_str());
        // MessageBoxW( NULL, sInstDir.c_str(), L"Found in HKEY_CURRENT_USER", MB_OK );
    }
    else if (auto sInstDir = RegValue(HKEY_LOCAL_MACHINE, sKey.c_str(), L"INSTALLLOCATION");
             !sInstDir.empty())
    {
        MsiSetPropertyW(handle, L"INSTALLLOCATION", sInstDir.c_str());
        // MessageBoxW( NULL, sInstDir.c_str(), L"Found in HKEY_LOCAL_MACHINE", MB_OK );
    }
    else if (std::wistringstream sOlds{ GetMsiPropertyW(handle, L"OLDPRODUCTS") }; !sOlds.eof())
    {
        std::wstring sOld;
        bool bFound = false;
        while (!bFound && std::getline(sOlds, sOld, L';'))
        {
            if (sOld.empty())
                continue;
            sKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + sOld;
            if (auto sInstDir = RegValue(HKEY_LOCAL_MACHINE, sKey.c_str(), L"InstallLocation");
                !sInstDir.empty())
            {
                MsiSetPropertyW(handle, L"INSTALLLOCATION", sInstDir.c_str());
                // MessageBoxW( NULL, sInstDir.c_str(), L"Found in Uninstall", MB_OK );
                bFound = true;
            }
        }
    }

    return ERROR_SUCCESS;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
