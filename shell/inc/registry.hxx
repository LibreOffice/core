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

#ifndef INCLUDED_SHELL_INC_INTERNAL_REGISTRY_HXX
#define INCLUDED_SHELL_INC_INTERNAL_REGISTRY_HXX


// registry helper functions


#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <objbase.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <string>
#include <filepath.hxx>

/** Sets a value of the specified registry key, an empty ValueName sets the default
    value
    Returns true on success
*/
bool SetRegistryKey(HKEY RootKey, const Filepath_char_t* KeyName, const Filepath_char_t* ValueName, const Filepath_char_t* Value);

/** Deletes the specified registry key and all of its subkeys
    Returns true on success
*/
bool DeleteRegistryKey(HKEY RootKey, const Filepath_char_t* KeyName);

/** May be used to determine if the specified registry key has subkeys
    The function returns true on success else if an error occurs false
*/
bool HasSubkeysRegistryKey(HKEY RootKey, const Filepath_char_t* KeyName, bool& bResult);

/** Converts a GUID to its string representation
*/
Filepath_t ClsidToString(const CLSID& clsid);

/** Get the content of a specific key.
*/
bool QueryRegistryKey(HKEY RootKey, const Filepath_char_t* KeyName, const Filepath_char_t* ValueName, Filepath_char_t *pszData, DWORD dwBufLen);


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
