// WindowsRegistry.h: Schnittstelle für die Klasse WindowsRegistry.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WINDOWSREGISTRY_HXX_
#define _WINDOWSREGISTRY_HXX_

#include <windows.h>
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
