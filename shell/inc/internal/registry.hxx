/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registry.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:38:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef REGISTRY_HXX_INCLUDED
#define REGISTRY_HXX_INCLUDED

//-----------------------------
// registry helper functions
//-----------------------------

#include <objbase.h>
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
