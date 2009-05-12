/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: registry.hxx,v $
 * $Revision: 1.6 $
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

#ifndef REGISTRY_HXX_INCLUDED
#define REGISTRY_HXX_INCLUDED

//-----------------------------
// registry helper functions
//-----------------------------

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <objbase.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <string>

/** Sets a value of the specified registry key, an empty ValueName sets the default
    value
    Returns true on success
*/
bool SetRegistryKey(HKEY RootKey, const char* KeyName, const char* ValueName, const char* Value);

/** Deletes the specified registry key and all of its subkeys
    Returns true on success
*/
bool DeleteRegistryKey(HKEY RootKey, const char* KeyName);

/** May be used to determine if the specified registry key has subkeys
    The function returns true on success else if an error occures false
*/
bool HasSubkeysRegistryKey(HKEY RootKey, const char* KeyName, bool& bResult);

/** Converts a GUID to its string representation
*/
std::string ClsidToString(const CLSID& clsid);

/** Get the content of a specific key.
*/
bool QueryRegistryKey(HKEY RootKey, const char* KeyName, const char* ValueName, char *pszData, DWORD dwBufLen);


#endif
