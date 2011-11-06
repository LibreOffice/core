/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
