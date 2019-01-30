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

#include <malloc.h>
#include <assert.h>

namespace
{
    // The provided GUID must be without surrounding '{}'
    std::wstring GetGuidPart(const std::wstring& guid, int index)
    {
        assert((guid.length() == 36) && "No GUID or wrong format!");
        assert(((index > -1) && (index < 5)) && "Out of range!");

        if (index == 0) return std::wstring(guid.c_str(), 8);
        if (index == 1) return std::wstring(guid.c_str() + 9, 4);
        if (index == 2) return std::wstring(guid.c_str() + 14, 4);
        if (index == 3) return std::wstring(guid.c_str() + 19, 4);
        if (index == 4) return std::wstring(guid.c_str() + 24, 12);

        return std::wstring();
    }

    void Swap(wchar_t* p1, wchar_t* p2)
    {
        wchar_t tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
    }

    std::wstring Invert(const std::wstring& str)
    {
        wchar_t* buff = static_cast<wchar_t*>(_alloca(str.length()*sizeof(wchar_t)));
        wcsncpy(buff, str.c_str(), str.length());

        wchar_t* front = buff;
        wchar_t* back = buff + str.length() - 1;

        while (front < back)
            Swap(front++, back--);

        return std::wstring(buff, str.length());
    }

    // Convert the upgrade code (which is a GUID) according
    // to the way the windows installer does when writing it
    // to the registry
    // The first 8 bytes will be inverted, from the last
    // 8 bytes always the nibbles will be inverted for further
    // details look in the MSDN under compressed registry keys
    std::wstring ConvertGuid(const std::wstring& guid)
    {
        std::wstring convertedGuid;

        std::wstring part = GetGuidPart(guid, 0);
        convertedGuid = Invert(part);

        part = GetGuidPart(guid, 1);
        convertedGuid += Invert(part);

        part = GetGuidPart(guid, 2);
        convertedGuid += Invert(part);

        part = GetGuidPart(guid, 3);
        convertedGuid += Invert(std::wstring(part.c_str(), 2));
        convertedGuid += Invert(std::wstring(part.c_str() + 2, 2));

        part = GetGuidPart(guid, 4);
        int pos = 0;
        for (int i = 0; i < 6; i++)
        {
            convertedGuid += Invert(std::wstring(part.c_str() + pos, 2));
            pos += 2;
        }
        return convertedGuid;
    }

    bool IsSetMsiPropertyW(MSIHANDLE handle, const std::wstring& sProperty)
    {
        return (GetMsiPropertyW(handle, sProperty).length() > 0);
    }

    void SetMsiPropertyW(MSIHANDLE handle, const std::wstring& sProperty)
    {
        MsiSetPropertyW(handle, sProperty.c_str(), L"1");
    }

    bool RegistryKeyHasUpgradeSubKey(
        HKEY hRootKey, const std::wstring& regKey, const std::wstring& upgradeKey)
    {
        HKEY hKey;
        if (RegOpenKeyW(hRootKey, regKey.c_str(), &hKey) == ERROR_SUCCESS)
        {
            DWORD nSubKeys;
            DWORD lLongestSubKey;

            if (RegQueryInfoKeyW(
                hKey, nullptr, nullptr, nullptr, &nSubKeys, &lLongestSubKey, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
            {
                LPWSTR buffer = static_cast<LPWSTR>(_alloca((lLongestSubKey + 1)*sizeof(WCHAR)));

                for (DWORD i = 0; i < nSubKeys; i++)
                {
                    LONG ret = RegEnumKeyW(hKey, i, buffer, lLongestSubKey + 1);
                    if ((ret == ERROR_SUCCESS) && (buffer == upgradeKey))
                        return true;
                }
            }
        }
        return false;
    }
} // namespace

extern "C" __declspec(dllexport) UINT __stdcall SetProductInstallMode(MSIHANDLE handle)
{
    std::wstring upgradeCode = GetMsiPropertyW(handle, L"UpgradeCode");
    upgradeCode = ConvertGuid(std::wstring(upgradeCode.c_str() + 1, upgradeCode.length() - 2));

    // MessageBoxW(NULL, upgradeCode.c_str(), "Debug", MB_OK);

    if (RegistryKeyHasUpgradeSubKey(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Installer\\UpgradeCodes",
        upgradeCode) && IsSetMsiPropertyW(handle, L"ALLUSERS"))
    {
        UnsetMsiPropertyW(handle, L"ALLUSERS");
        // MessageBoxW(NULL, L"ALLUSERS removed", L"DEBUG", MB_OK);
    }
    else if (RegistryKeyHasUpgradeSubKey(
             HKEY_LOCAL_MACHINE,
             L"Software\\Classes\\Installer\\UpgradeCodes",
             upgradeCode) && !IsSetMsiPropertyW(handle, L"ALLUSERS"))
    {
        SetMsiPropertyW(handle, L"ALLUSERS");
        // MessageBoxW(NULL, L"ALLUSERS set", L"DEBUG", MB_OK);
    }
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
