/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <malloc.h>
#include "internal/registry.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <objbase.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif




const int CLSID_STRING_SIZE = 39;



bool SetRegistryKey(HKEY RootKey, const char* KeyName, const char* ValueName, const char* Value)
{
    HKEY hSubKey;

    
    char dummy[] = "";
    int rc = RegCreateKeyExA(
        RootKey, const_cast<char*>(KeyName), 0, dummy, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hSubKey, 0);

    if (ERROR_SUCCESS == rc)
    {
        rc = RegSetValueExA(
            hSubKey, ValueName, 0, REG_SZ, reinterpret_cast<const BYTE*>(Value),
            static_cast<DWORD>(strlen(Value) + 1));

        RegCloseKey(hSubKey);
    }

    return (ERROR_SUCCESS == rc);
}



bool DeleteRegistryKey(HKEY RootKey, const char* KeyName)
{
    HKEY hKey;

    int rc = RegOpenKeyExA(
        RootKey,
        KeyName,
        0,
        KEY_READ | DELETE,
        &hKey);

    if ( rc == ERROR_FILE_NOT_FOUND )
        return true;

    if (ERROR_SUCCESS == rc)
    {
        char* SubKey;
        DWORD nMaxSubKeyLen;

        rc = RegQueryInfoKeyA(
            hKey, 0, 0, 0, 0,
            &nMaxSubKeyLen,
            0, 0, 0, 0, 0, 0);

        nMaxSubKeyLen++; 

        SubKey = reinterpret_cast<char*>(
            _alloca(nMaxSubKeyLen*sizeof(char)));

        while (ERROR_SUCCESS == rc)
        {
            DWORD nLen = nMaxSubKeyLen;

            rc = RegEnumKeyExA(
                hKey,
                0,       
                SubKey,
                &nLen,
                0, 0, 0, 0);

            if (ERROR_NO_MORE_ITEMS == rc)
            {
                rc = RegDeleteKeyA(RootKey, KeyName);
                break;
            }
            else if (rc == ERROR_SUCCESS)
            {
                DeleteRegistryKey(hKey, SubKey);
            }

        } 

        RegCloseKey(hKey);

    } 

    return (ERROR_SUCCESS == rc);
}

/** May be used to determine if the specified registry key has subkeys
    The function returns true on success else if an error occurs false
*/
bool HasSubkeysRegistryKey(HKEY RootKey, const char* KeyName, /* out */ bool& bResult)
{
    HKEY hKey;

    LONG rc = RegOpenKeyExA(RootKey, KeyName, 0, KEY_READ, &hKey);

    if (ERROR_SUCCESS == rc)
    {
        DWORD nSubKeys = 0;

        rc = RegQueryInfoKeyA(hKey, 0, 0, 0, &nSubKeys, 0, 0, 0, 0, 0, 0, 0);

        bResult = (nSubKeys > 0);
    }

    return (ERROR_SUCCESS == rc);
}


std::string ClsidToString(const CLSID& clsid)
{
    
    LPOLESTR wszCLSID = NULL;
    StringFromCLSID(clsid, &wszCLSID);

    char buff[39];
    
    wcstombs(buff, wszCLSID, sizeof(buff));

    
    CoTaskMemFree(wszCLSID) ;

    return std::string(buff);
}



bool QueryRegistryKey(HKEY RootKey, const char* KeyName, const char* ValueName, char *pszData, DWORD dwBufLen)
{
    HKEY hKey;

    int rc = RegOpenKeyExA(
        RootKey,
        KeyName,
        0,
        KEY_READ,
        &hKey);

    if (ERROR_SUCCESS == rc)
    {
        rc = RegQueryValueExA(
            hKey, ValueName, NULL, NULL, (LPBYTE)pszData,&dwBufLen);

        RegCloseKey(hKey);
    }

    return (ERROR_SUCCESS == rc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
