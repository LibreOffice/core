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


#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <malloc.h>
#include <registry.hxx>

#include <objbase.h>

bool SetRegistryKey(HKEY RootKey, const Filepath_char_t* KeyName, const Filepath_char_t* ValueName, const Filepath_char_t* Value)
{
    int rc = RegSetKeyValueW(RootKey, KeyName, ValueName, REG_SZ, Value,
                             (wcslen(Value) + 1) * sizeof(*Value));
    return (ERROR_SUCCESS == rc);
}

bool DeleteRegistryTree(HKEY RootKey, const Filepath_char_t* KeyName)
{
    return (ERROR_SUCCESS == RegDeleteTreeW(RootKey, KeyName));
}

bool DeleteRegistryKey(HKEY RootKey, const Filepath_char_t* KeyName)
{
    return (ERROR_SUCCESS == RegDeleteKeyW(RootKey, KeyName));
}

// Convert a CLSID to a char string.
Filepath_t ClsidToString(const CLSID& clsid)
{
    wchar_t wszCLSID[std::size(L"{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}")];
    if (StringFromGUID2(clsid, wszCLSID, std::size(wszCLSID)))
        return std::wstring(wszCLSID);
    return {};
}


bool QueryRegistryKey(HKEY RootKey, const Filepath_char_t* KeyName, const Filepath_char_t* ValueName, Filepath_char_t *pszData, DWORD dwBufLen)
{
    DWORD dwBytes = dwBufLen * sizeof(*pszData);
    LSTATUS rc = RegGetValueW(RootKey, KeyName, ValueName, RRF_RT_REG_SZ, nullptr,
                              pszData, &dwBytes);
    return (ERROR_SUCCESS == rc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
