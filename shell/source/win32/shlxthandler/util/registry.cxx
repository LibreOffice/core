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


#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <malloc.h>
#include "registry.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <objbase.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

bool SetRegistryKey(HKEY RootKey, const wchar_t* KeyName, const wchar_t* ValueName, const wchar_t* Value)
{
    HKEY hSubKey;

    // open or create the desired key
    wchar_t dummy[] = L"";
    int rc = RegCreateKeyExW(
        RootKey, KeyName, 0, dummy, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hSubKey, nullptr);

    if (ERROR_SUCCESS == rc)
    {
        rc = RegSetValueExW(
            hSubKey, ValueName, 0, REG_SZ, reinterpret_cast<const BYTE*>(Value),
            static_cast<DWORD>((wcslen(Value) + 1) * sizeof(*Value)));

        RegCloseKey(hSubKey);
    }

    return (ERROR_SUCCESS == rc);
}


bool DeleteRegistryKey(HKEY RootKey, const wchar_t* KeyName)
{
    HKEY hKey;

    int rc = RegOpenKeyExW(
        RootKey,
        KeyName,
        0,
        KEY_READ | DELETE,
        &hKey);

    if ( rc == ERROR_FILE_NOT_FOUND )
        return true;

    if (ERROR_SUCCESS == rc)
    {
        wchar_t* SubKey;
        DWORD nMaxSubKeyLen;

        rc = RegQueryInfoKeyW(
            hKey, nullptr, nullptr, nullptr, nullptr,
            &nMaxSubKeyLen,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

        nMaxSubKeyLen++; // space for trailing '\0'

        SubKey = static_cast<wchar_t*>(
            _alloca(nMaxSubKeyLen*sizeof(wchar_t)));

        while (ERROR_SUCCESS == rc)
        {
            DWORD nLen = nMaxSubKeyLen;

            rc = RegEnumKeyExW(
                hKey,
                0,       // always index zero
                SubKey,
                &nLen,
                nullptr, nullptr, nullptr, nullptr);

            if (ERROR_NO_MORE_ITEMS == rc)
            {
                rc = RegDeleteKeyW(RootKey, KeyName);
                break;
            }
            else if (rc == ERROR_SUCCESS)
            {
                DeleteRegistryKey(hKey, SubKey);
            }

        } // while

        RegCloseKey(hKey);

    } // if

    return (ERROR_SUCCESS == rc);
}

/** May be used to determine if the specified registry key has subkeys
    The function returns true on success else if an error occurs false
*/
bool HasSubkeysRegistryKey(HKEY RootKey, const wchar_t* KeyName, /* out */ bool& bResult)
{
    HKEY hKey;

    LONG rc = RegOpenKeyExW(RootKey, KeyName, 0, KEY_READ, &hKey);

    if (ERROR_SUCCESS == rc)
    {
        DWORD nSubKeys = 0;

        rc = RegQueryInfoKeyW(hKey, nullptr, nullptr, nullptr, &nSubKeys, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

        RegCloseKey(hKey);
        bResult = (nSubKeys > 0);
    }

    return (ERROR_SUCCESS == rc);
}

// Convert a CLSID to a char string.
std::wstring ClsidToString(const CLSID& clsid)
{
    // Get CLSID
    LPOLESTR wszCLSID = nullptr;
    StringFromCLSID(clsid, &wszCLSID);

    std::wstring sResult = wszCLSID;

    // Free memory.
    CoTaskMemFree(wszCLSID) ;

    return sResult;
}


bool QueryRegistryKey(HKEY RootKey, const wchar_t* KeyName, const wchar_t* ValueName, wchar_t *pszData, DWORD dwBufLen)
{
    HKEY hKey;

    int rc = RegOpenKeyExW(
        RootKey,
        KeyName,
        0,
        KEY_READ,
        &hKey);

    if (ERROR_SUCCESS == rc)
    {
        DWORD dwBytes = dwBufLen * sizeof(*pszData);
        rc = RegQueryValueExW(
            hKey, ValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(pszData),&dwBytes);

        RegCloseKey(hKey);
    }

    return (ERROR_SUCCESS == rc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
