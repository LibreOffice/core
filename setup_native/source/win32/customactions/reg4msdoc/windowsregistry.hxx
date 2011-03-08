/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
