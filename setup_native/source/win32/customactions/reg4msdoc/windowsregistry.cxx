/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// WindowsRegistry.cpp: Implementierung der Klasse WindowsRegistry.
//
//////////////////////////////////////////////////////////////////////

#include "windowsregistry.hxx"
#include "registrywnt.hxx"
#include "registryw9x.hxx"

#ifdef _MSC_VER
#pragma warning(disable : 4350)
#endif

WindowsRegistry::WindowsRegistry()
{
    OSVERSIONINFOA osverinfo;
    ZeroMemory(&osverinfo, sizeof(osverinfo));
    osverinfo.dwOSVersionInfoSize = sizeof(osverinfo);
    GetVersionExA(&osverinfo);

    m_IsWinNT = (osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
}

RegistryKey WindowsRegistry::GetClassesRootKey(bool Writeable) const
{
    return GetRegistryKey(HKEY_CLASSES_ROOT, Writeable);
}

RegistryKey WindowsRegistry::GetCurrentUserKey(bool Writeable) const
{
    return GetRegistryKey(HKEY_CURRENT_USER, Writeable);
}

RegistryKey WindowsRegistry::GetLocalMachineKey(bool Writeable) const
{
    return GetRegistryKey(HKEY_LOCAL_MACHINE, Writeable);
}

RegistryKey WindowsRegistry::GetUserKey(bool Writeable) const
{
    return GetRegistryKey(HKEY_USERS, Writeable);
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
