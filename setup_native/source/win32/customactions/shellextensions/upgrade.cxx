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

// NB This file still makes use of largely ANSI MSI API calls
#undef UNICODE
#undef _UNICODE

#include "shlxtmsi.hxx"

#include <malloc.h>
#include <assert.h>

using namespace std;

namespace
{
    // The provided GUID must be without surrounding '{}'
    string GetGuidPart(const string& guid, int index)
    {
        assert((guid.length() == 36) && "No GUID or wrong format!");
        assert(((index > -1) && (index < 5)) && "Out of range!");

        if (index == 0) return string(guid.c_str(), 8);
        if (index == 1) return string(guid.c_str() + 9, 4);
        if (index == 2) return string(guid.c_str() + 14, 4);
        if (index == 3) return string(guid.c_str() + 19, 4);
        if (index == 4) return string(guid.c_str() + 24, 12);

        return string();
    }

    void Swap(char* p1, char* p2)
    {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
    }

    string Invert(const string& str)
    {
        char* buff = static_cast<char*>(_alloca(str.length()));
        strncpy(buff, str.c_str(), str.length());

        char* front = buff;
        char* back = buff + str.length() - 1;

        while (front < back)
            Swap(front++, back--);

        return string(buff, str.length());
    }

    // Convert the upgrade code (which is a GUID) according
    // to the way the windows installer does when writing it
    // to the registry
    // The first 8 bytes will be inverted, from the last
    // 8 bytes always the nibbles will be inverted for further
    // details look in the MSDN under compressed registry keys
    string ConvertGuid(const string& guid)
    {
        string convertedGuid;

        string part = GetGuidPart(guid, 0);
        convertedGuid = Invert(part);

        part = GetGuidPart(guid, 1);
        convertedGuid += Invert(part);

        part = GetGuidPart(guid, 2);
        convertedGuid += Invert(part);

        part = GetGuidPart(guid, 3);
        convertedGuid += Invert(string(part.c_str(), 2));
        convertedGuid += Invert(string(part.c_str() + 2, 2));

        part = GetGuidPart(guid, 4);
        int pos = 0;
        for (int i = 0; i < 6; i++)
        {
            convertedGuid += Invert(string(part.c_str() + pos, 2));
            pos += 2;
        }
        return convertedGuid;
    }

    string GetMsiPropertyA(MSIHANDLE handle, const string& sProperty)
    {
        string  result;
        CHAR    szDummy[1] = "";
        DWORD   nChars = 0;

        if (MsiGetPropertyA(handle, sProperty.c_str(), szDummy, &nChars) == ERROR_MORE_DATA)
        {
            DWORD nBytes = ++nChars * sizeof(CHAR);
            LPSTR buffer = static_cast<LPSTR>(_alloca(nBytes));
            ZeroMemory( buffer, nBytes );
            MsiGetPropertyA( handle, sProperty.c_str(), buffer, &nChars );
            result = buffer;
        }
        return  result;
    }

    inline bool IsSetMsiPropertyA(MSIHANDLE handle, const string& sProperty)
    {
        return (GetMsiPropertyA(handle, sProperty).length() > 0);
    }

    inline void UnsetMsiPropertyA(MSIHANDLE handle, const string& sProperty)
    {
        MsiSetPropertyA(handle, sProperty.c_str(), nullptr);
    }

    inline void SetMsiPropertyA(MSIHANDLE handle, const string& sProperty)
    {
        MsiSetPropertyA(handle, sProperty.c_str(), "1");
    }

    bool RegistryKeyHasUpgradeSubKey(
        HKEY hRootKey, const wstring& regKey, const string& upgradeKey)
    {
        HKEY hKey;
        if (RegOpenKeyW(hRootKey, regKey.c_str(), &hKey) == ERROR_SUCCESS)
        {
            DWORD nSubKeys;
            DWORD lLongestSubKey;

            if (RegQueryInfoKeyA(
                hKey, nullptr, nullptr, nullptr, &nSubKeys, &lLongestSubKey, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
            {
                LPSTR buffer = static_cast<LPSTR>(_alloca(lLongestSubKey + 1));

                for (DWORD i = 0; i < nSubKeys; i++)
                {
                    LONG ret = RegEnumKeyA(hKey, i, buffer, lLongestSubKey + 1);
                    if ((ret == ERROR_SUCCESS) && (buffer == upgradeKey))
                        return true;
                }
            }
        }
        return false;
    }
} // namespace

extern "C" UINT __stdcall SetProductInstallMode(MSIHANDLE handle)
{
    string upgradeCode = GetMsiPropertyA(handle, "UpgradeCode");
    upgradeCode = ConvertGuid(string(upgradeCode.c_str() + 1, upgradeCode.length() - 2));

    // MessageBoxA(NULL, upgradeCode.c_str(), "Debug", MB_OK);

    if (RegistryKeyHasUpgradeSubKey(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Installer\\UpgradeCodes",
        upgradeCode) && IsSetMsiPropertyA(handle, "ALLUSERS"))
    {
        UnsetMsiPropertyA(handle, "ALLUSERS");
        // MessageBoxW(NULL, L"ALLUSERS removed", L"DEBUG", MB_OK);
    }
    else if (RegistryKeyHasUpgradeSubKey(
             HKEY_LOCAL_MACHINE,
             L"Software\\Classes\\Installer\\UpgradeCodes",
             upgradeCode) && !IsSetMsiPropertyA(handle, "ALLUSERS"))
    {
        SetMsiPropertyA(handle, "ALLUSERS");
        // MessageBoxW(NULL, L"ALLUSERS set", L"DEBUG", MB_OK);
    }
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
