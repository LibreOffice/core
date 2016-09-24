/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_AVMEDIA_SOURCE_VLC_WRAPPER_SYMBOLLOADER_HXX
#define INCLUDED_AVMEDIA_SOURCE_VLC_WRAPPER_SYMBOLLOADER_HXX
#if defined(_WIN32)
# include <windows.h>
# include <winreg.h>
#endif
#include <osl/module.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#define SYM_MAP(a) { #a, reinterpret_cast<SymbolFunc *>(&a) }

namespace avmedia
{
namespace vlc
{
namespace wrapper
{
typedef void (*SymbolFunc) (void);

struct ApiMap
{
    const char *symName;
    SymbolFunc *refValue;
};

namespace
{
#if defined( LINUX )
    const char LibName[] = "libvlc.so.5";
#elif defined( MACOSX )
    const char LibName[] = "/Applications/VLC.app/Contents/MacOS/lib/libvlc.dylib";
#elif defined( WNT )
    const char LibName[] = "libvlc.dll";

    OUString GetVLCPath()
    {
        HKEY hKey;
        wchar_t arCurrent[MAX_PATH];
        DWORD dwType, dwCurrentSize = sizeof( arCurrent );

        //TODO: This one will work only with LibreOffice 32-bit + VLC 32-bit on Win x86_64.
        const LONG errorCore = ::RegOpenKeyExW( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\VideoLAN\\VLC", 0, KEY_READ | KEY_WOW64_64KEY, &hKey );
        if ( errorCore == ERROR_SUCCESS )
        {
            if ( ::RegQueryValueExW( hKey, L"InstallDir", NULL, &dwType, (LPBYTE) arCurrent, &dwCurrentSize ) == ERROR_SUCCESS &&
                 dwType == REG_SZ )
            {
                ::RegCloseKey( hKey );
                dwCurrentSize -= 2;
                dwCurrentSize /= 2;

                return OUString( arCurrent, dwCurrentSize ) + OUString::createFromAscii("\\");
            }

            ::RegCloseKey( hKey );
        }

        return OUString();
    }
#endif

    template<size_t N>
    bool tryLink( oslModule &aModule, const ApiMap ( &pMap )[N] )
    {
        for (size_t i = 0; i < N; ++i)
        {
            SymbolFunc aMethod = reinterpret_cast<SymbolFunc>(osl_getFunctionSymbol
                ( aModule, OUString::createFromAscii( pMap[ i ].symName ).pData ));
            if ( !aMethod )
            {
                SAL_WARN("avmedia", "Cannot load method " << pMap[ i ].symName);
                *pMap[ i ].refValue = nullptr;
                return false;
            }
            else
                *pMap[ i ].refValue = aMethod;
        }

        return true;
    }
}

    template<size_t N>
    bool InitApiMap( const ApiMap ( &pMap )[N]  )
    {
#if defined( LINUX ) || defined( MACOSX )
        OUString const fullPath(LibName);
#elif defined( WNT )
        OUString const fullPath(GetVLCPath() + LibName);
#endif
        SAL_INFO("avmedia", fullPath);

        oslModule aModule = osl_loadModule( fullPath.pData,
                                            SAL_LOADMODULE_DEFAULT );


        if( aModule == nullptr)
        {
            SAL_WARN("avmedia", "Cannot load libvlc");
            return false;
        }

        if (tryLink( aModule, pMap ))
        {
            return true;
        }

        SAL_WARN("avmedia", "Cannot load libvlc");
        osl_unloadModule( aModule );

        return false;
    }
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
