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

// WindowsRegistry.h: Schnittstelle für die Klasse WindowsRegistry.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WINDOWSREGISTRY_HXX_
#define _WINDOWSREGISTRY_HXX_

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "Registry.hxx"

/** Basically a factory class
*/
class WindowsRegistry
{
public:
    WindowsRegistry();

    RegistryKey GetClassesRootKey(bool Writeable = true) const;

    RegistryKey GetCurrentUserKey(bool Writeable = true) const;

    RegistryKey GetLocalMachineKey(bool Writeable = true) const;

    RegistryKey GetUserKey(bool Writeable = true) const;

private:
    RegistryKey GetRegistryKey(HKEY RootKey, bool Writeable) const;

private:
    bool m_IsWinNT;
};

#endif
