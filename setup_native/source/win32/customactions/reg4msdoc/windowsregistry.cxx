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

// WindowsRegistry.cpp: Implementierung der Klasse WindowsRegistry.
//
//////////////////////////////////////////////////////////////////////

#include "windowsregistry.hxx"
#include "registrywnt.hxx"
#include "registryw9x.hxx"

#ifdef _MSC_VER
#pragma warning(disable : 4350)
#endif

//------------------------------
//
//------------------------------

WindowsRegistry::WindowsRegistry()
{
    OSVERSIONINFOA osverinfo;
    ZeroMemory(&osverinfo, sizeof(osverinfo));
    osverinfo.dwOSVersionInfoSize = sizeof(osverinfo);
    GetVersionExA(&osverinfo);

    m_IsWinNT = (osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
}

//------------------------------
//
//------------------------------

RegistryKey WindowsRegistry::GetClassesRootKey(bool Writeable) const
{
    return GetRegistryKey(HKEY_CLASSES_ROOT, Writeable);
}

//------------------------------
//
//------------------------------

RegistryKey WindowsRegistry::GetCurrentUserKey(bool Writeable) const
{
    return GetRegistryKey(HKEY_CURRENT_USER, Writeable);
}

//------------------------------
//
//------------------------------

RegistryKey WindowsRegistry::GetLocalMachineKey(bool Writeable) const
{
    return GetRegistryKey(HKEY_LOCAL_MACHINE, Writeable);
}

//------------------------------
//
//------------------------------

RegistryKey WindowsRegistry::GetUserKey(bool Writeable) const
{
    return GetRegistryKey(HKEY_USERS, Writeable);
}

//------------------------------
//
//------------------------------

RegistryKey WindowsRegistry::GetRegistryKey(HKEY RootKey, bool Writeable) const
{
    RegistryKey regkey;

    if (m_IsWinNT)
        regkey = RegistryKey(new RegistryKeyImplWinNT(RootKey));
    else
        regkey = RegistryKey(new RegistryKeyImplWin9x(RootKey));

    regkey->Open(Writeable);

    return regkey;
}
