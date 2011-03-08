/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <malloc.h>
#include "internal/dbgmacros.hxx"
#include "internal/registry.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <objbase.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//---------------------------------------
//
//---------------------------------------

// Size of a CLSID as a string
const int CLSID_STRING_SIZE = 39;

//---------------------------------------
//
//---------------------------------------

bool SetRegistryKey(HKEY RootKey, const char* KeyName, const char* ValueName, const char* Value)
{
    HKEY hSubKey;

    // open or create the desired key
    int rc = RegCreateKeyExA(
        RootKey, KeyName, 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hSubKey, 0);

    if (ERROR_SUCCESS == rc)
    {
        rc = RegSetValueExA(
            hSubKey, ValueName, 0, REG_SZ, reinterpret_cast<const BYTE*>(Value), strlen(Value) + 1);

        RegCloseKey(hSubKey);
    }

    return (ERROR_SUCCESS == rc);
}

//---------------------------------------
//
//---------------------------------------

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

        nMaxSubKeyLen++; // space for trailing '\0'

        SubKey = reinterpret_cast<char*>(
            _alloca(nMaxSubKeyLen*sizeof(char)));

        while (ERROR_SUCCESS == rc)
        {
            DWORD nLen = nMaxSubKeyLen;

            rc = RegEnumKeyExA(
                hKey,
                0,       // always index zero
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

        } // while

        RegCloseKey(hKey);

    } // if

    return (ERROR_SUCCESS == rc);
}

/** May be used to determine if the specified registry key has subkeys
    The function returns true on success else if an error occures false
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

// Convert a CLSID to a char string.
std::string ClsidToString(const CLSID& clsid)
{
    // Get CLSID
    LPOLESTR wszCLSID = NULL;
    StringFromCLSID(clsid, &wszCLSID);

    char buff[39];
    // Covert from wide characters to non-wide.
    wcstombs(buff, wszCLSID, sizeof(buff));

    // Free memory.
    CoTaskMemFree(wszCLSID) ;

    return std::string(buff);
}

//---------------------------------------
//
//---------------------------------------

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
