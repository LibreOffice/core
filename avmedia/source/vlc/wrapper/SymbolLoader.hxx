/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _SYMBOL_LOADER_HXX
#define _SYMBOL_LOADER_HXX
#if defined( WNT )
# include <windows.h>
# include <winreg.h>
#endif
#include <iostream>
#include <osl/module.h>
#include <rtl/ustring.hxx>

#define SYM_MAP(a) { #a, (SymbolFunc *)&a }

typedef void (*SymbolFunc) (void);

struct ApiMap
{
    const char *symName;
    SymbolFunc *refValue;
};

namespace
{
#if defined( UNX )
    const char LibName[] = "libvlc.so.5";
#elif defined( MACOS )
    const char LibName[] = "/Applications/VLC.app/Contents/MacOS/lib/libvlc.dylib";
#elif defined( WNT )
    const char LibName[] = "libvlc.dll";

    OUString GetVLCPath()
    {
        HKEY hKey;
        TCHAR arCurrent[MAX_PATH];
        DWORD dwType, dwCurrentSize = sizeof( arCurrent );

        if ( ::RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T( "Software\\VideoLAN\\VLC" ),
                             0, KEY_READ, &hKey ) == ERROR_SUCCESS )
        {
            if ( ::RegQueryValueEx( hKey, _T( "InstallDir" ), NULL, &dwType, ( LPBYTE )arCurrent, &dwCurrentSize ) == ERROR_SUCCESS )
            {
                ::RegCloseKey( hKey );
                return OUString( arCurrent, MAX_PATH, rtl_TextEncoding, RTL_TEXTENCODING_UTF8 ) + "/";
            }

            ::RegCloseKey( hKey );
        }
    }
#endif


    template<size_t N>
    bool tryLink( oslModule &aModule, const ApiMap ( &pMap )[N] )
    {
        for (uint i = 0; i < N; ++i)
        {
            SymbolFunc aMethod = ( SymbolFunc )osl_getFunctionSymbol
                ( aModule, OUString::createFromAscii( pMap[ i ].symName ).pData );
            if ( !aMethod )
            {
                std::cerr << "Cannot load method " << pMap[ i ].symName << std::endl;
                *pMap[ i ].refValue = NULL;
                return false;
            }

            *pMap[ i ].refValue = aMethod;
        }

        return true;
    }
}

namespace VLC
{
    template<size_t N>
    bool InitApiMap( const ApiMap ( &pMap )[N]  )
    {
#if defined( UNX ) || defined( MACOS )
        const OUString& fullPath = OUString::createFromAscii(LibName);
#elif defined( WNT )
        const OUString& fullPath = GetVLCPath() + OUString::createFromAscii(LibName);
#endif

        oslModule aModule = osl_loadModule( fullPath.pData,
                                            SAL_LOADMODULE_DEFAULT );

        if( aModule == NULL)
        {
            std::cerr << "Cannot load libvlc" << std::endl;
            return false;
        }

        if (tryLink( aModule, pMap ))
        {
            osl_unloadModule( aModule );
            return true;
        }

        osl_unloadModule( aModule );

        return false;
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
