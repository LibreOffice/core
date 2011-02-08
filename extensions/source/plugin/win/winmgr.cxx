/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "vcl/svapp.hxx"
#include "tools/fsys.hxx"
#include "tools/urlobj.hxx"
#include "osl/mutex.hxx"

#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"

#include "plugin/impl.hxx"

#pragma warning (push,1)
#pragma warning (disable:4005)

    #include "prewin.h"

    #include <windows.h>
    #include <string.h>
    #include <tchar.h>
    #include <winreg.h>
    #include <winbase.h>
    #include <objbase.h>

    #include "postwin.h"

#pragma warning (pop)

#include <list>
#include <map>
#include <algorithm>


using namespace rtl;
using namespace std;
using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::plugin;

typedef map< OString, OUString, less< OString > > PluginLocationMap;


#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>

static void logPlugin( OUString const & path_ )
{
    static FILE * s_file = 0;
    if (! s_file)
        s_file = fopen( "d:\\plugins.log", "a+" );
    OString path( OUStringToOString( path_, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( s_file, "%s\n", path.getStr() );
}
#endif

//__________________________________________________________________________________________________
static void addPluginsFromPath( const TCHAR * pPluginsPath, PluginLocationMap & rPlugins )
{
    // append dll name pattern we are looking for
    TCHAR arPluginsPath[MAX_PATH];
    arPluginsPath[0] = 0;

    if (::rtl_str_indexOfStr( pPluginsPath, "%programfiles%" ) == 0)
    {
        const char * p = ::getenv( "ProgramFiles" );
        if (p)
        {
            ::lstrcpy( arPluginsPath, p );
            pPluginsPath += 14;
        }
    }
    ::lstrcat( arPluginsPath, pPluginsPath );
    ::lstrcat( arPluginsPath, _T("\\") );

    TCHAR arPluginsPattern[MAX_PATH];
    ::lstrcpy( arPluginsPattern, arPluginsPath );
    ::lstrcat( arPluginsPattern, _T("NP*.DLL") );

    WIN32_FIND_DATA aFindData;
    HANDLE hFind = ::FindFirstFile( arPluginsPattern, &aFindData );

    while (hFind != INVALID_HANDLE_VALUE)
    {
        OString aName( aFindData.cFileName );
        aName.toAsciiLowerCase();

        // no netscape default plugin anymore...
        // and no double plugin dlls
        if ( !aName.equals( "npnul32.dll" ) &&
             ! aName.equals( "npnrvp.dll" ) &&
             rPlugins.find( aName ) == rPlugins.end())
        {
            TCHAR arComplete[MAX_PATH];
            ::lstrcpy( arComplete, arPluginsPath );
            ::lstrcat( arComplete, aFindData.cFileName );

            OUString path( OStringToOUString( arComplete, RTL_TEXTENCODING_MS_1252 ) );
            rPlugins[ aName ] = path;
#if OSL_DEBUG_LEVEL > 1
            logPlugin( path );
#endif
        }

        if (! ::FindNextFile( hFind, &aFindData ))
            break;
    }

    if (hFind != INVALID_HANDLE_VALUE)
        ::FindClose( hFind );
}
//__________________________________________________________________________________________________
static void addPluginsFromPath( const OUString & rPath, PluginLocationMap & rPlugins )
{
    TCHAR arPluginsPath[MAX_PATH];
    DWORD dwPluginsPathSize = sizeof(arPluginsPath);
    arPluginsPath[dwPluginsPathSize-1] = 0;

    OString aStr( OUStringToOString( rPath, RTL_TEXTENCODING_MS_1252 ) );
    ::strncpy( arPluginsPath, aStr.getStr(), dwPluginsPathSize );

    addPluginsFromPath( arPluginsPath, rPlugins );
}


//__________________________________________________________________________________________________
static void add_IE_Plugins( PluginLocationMap & rPlugins )
{
    HKEY hKey;
    TCHAR arCurrent[MAX_PATH];
    DWORD dwType, dwCurrentSize = sizeof(arCurrent);

    if (::RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\IE4\\SETUP"),
                        0, KEY_READ, &hKey ) == ERROR_SUCCESS)
    {
        if (::RegQueryValueEx( hKey, _T("Path"), NULL, &dwType,
                               (LPBYTE)arCurrent, &dwCurrentSize ) == ERROR_SUCCESS &&
            (dwType == REG_SZ || dwType == REG_EXPAND_SZ))
        {
            // add \\Plugins
            ::lstrcat( arCurrent, _T("\\Plugins") );

            addPluginsFromPath( arCurrent, rPlugins );
        }
        ::RegCloseKey( hKey );
    }
}

//--------------------------------------------------------------------------------------------------
static void add_NS_keys( HKEY hKey, PluginLocationMap & rPlugins )
{
    TCHAR value[MAX_PATH];
    DWORD dwType, size = sizeof(value);

    // 4.7
    size = sizeof(value);
    if (::RegQueryValueEx(
        hKey, _T("Plugins Directory"), NULL, &dwType,
        (LPBYTE)value, &size ) == ERROR_SUCCESS &&
        (dwType == REG_SZ || dwType == REG_EXPAND_SZ))
    {
        addPluginsFromPath( value, rPlugins );
    }
    // 6
    size = sizeof(value);
    if (::RegQueryValueEx(
        hKey, _T("Install Directory"), NULL, &dwType,
        (LPBYTE)value, &size ) == ERROR_SUCCESS &&
        (dwType == REG_SZ || dwType == REG_EXPAND_SZ))
    {
        int n = size / sizeof (TCHAR);
        if ('\\' != value[ n -2 ])
        {
            value[ n -1 ] = '\\';
            value[ n ] = 0;
        }
        addPluginsFromPath( ::lstrcat( value, _T("Plugins") ), rPlugins );
    }
    size = sizeof(value);
    if (::RegQueryValueEx(
        hKey, _T("Plugins"), NULL, &dwType,
        (LPBYTE)value, &size ) == ERROR_SUCCESS &&
        (dwType == REG_SZ || dwType == REG_EXPAND_SZ))
    {
        addPluginsFromPath( value, rPlugins );
    }
}
//--------------------------------------------------------------------------------------------------
static void add_NS_lookupRecursive( HKEY hKey, PluginLocationMap & rPlugins )
{
    add_NS_keys( hKey, rPlugins );

    TCHAR keyName[MAX_PATH];
    DWORD dwIndex = 0, size = sizeof (keyName);

    while (::RegEnumKeyEx( hKey, dwIndex, keyName, &size, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS)
    {
        size = sizeof (keyName);
        HKEY hSubKey;
        if (::RegOpenKeyEx( hKey, keyName, 0, KEY_READ, &hSubKey ) == ERROR_SUCCESS)
        {
            add_NS_lookupRecursive( hSubKey, rPlugins );
            ::RegCloseKey( hSubKey );
        }
        ++dwIndex;
    }
}
//__________________________________________________________________________________________________
static void add_MozPlugin( HKEY hKey, PluginLocationMap & rPlugins )
{
    TCHAR value[MAX_PATH];
    DWORD dwType, size = sizeof(value);

    size = sizeof(value);
    if (::RegQueryValueEx(
        hKey, _T("Path"), NULL, &dwType,
        (LPBYTE)value, &size ) == ERROR_SUCCESS &&
        (dwType == REG_SZ || dwType == REG_EXPAND_SZ))
    {
        OUString aUPath( OStringToOUString( value, RTL_TEXTENCODING_MS_1252 ) );
        INetURLObject aURL( aUPath );
        OString aName( OUStringToOString( aURL.GetName().toAsciiLowerCase(), RTL_TEXTENCODING_MS_1252 ) );

        // no netscape default plugin anymore...
        // and no double plugin dlls
        if ( !aName.equals( "npnul32.dll" ) &&
             ! aName.equals( "npnrvp.dll" ) &&
             rPlugins.find( aName ) == rPlugins.end())
        {
            rPlugins[ aName ] = aUPath;
#if OSL_DEBUG_LEVEL > 1
            logPlugin( aUPath );
#endif
        }
    }
}
static void add_MozillaPlugin( HKEY hKey, PluginLocationMap & rPlugins )
{
    TCHAR keyName[MAX_PATH];
    DWORD dwIndex = 0, size = sizeof (keyName);

    while (::RegEnumKeyEx( hKey, dwIndex, keyName, &size, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS)
    {
        size = sizeof (keyName);
        HKEY hSubKey;
        if (::RegOpenKeyEx( hKey, keyName, 0, KEY_READ, &hSubKey ) == ERROR_SUCCESS)
        {
            add_MozPlugin( hSubKey, rPlugins );
            ::RegCloseKey( hSubKey );
        }
        ++dwIndex;
    }
}
//__________________________________________________________________________________________________
static void add_NS_Plugins( PluginLocationMap & rPlugins )
{
    HKEY hKey;
    // Netscape
    if (::RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, _T("Software\\Netscape"),
        0, KEY_READ, &hKey ) == ERROR_SUCCESS)
    {
        add_NS_lookupRecursive( hKey, rPlugins );
        ::RegCloseKey( hKey );
    }
    // Mozilla
    if (::RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, _T("Software\\Mozilla"),
        0, KEY_READ, &hKey ) == ERROR_SUCCESS)
    {
        add_NS_lookupRecursive( hKey, rPlugins );
        ::RegCloseKey( hKey );
    }
    // Mozilla - plugins
    if (::RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, _T("Software\\MozillaPlugins"),
        0, KEY_READ, &hKey ) == ERROR_SUCCESS)
    {
        add_MozillaPlugin( hKey, rPlugins );
        ::RegCloseKey( hKey );
    }
}

//__________________________________________________________________________________________________
static void add_SO_Plugins( PluginLocationMap & rPlugins )
{
    const Sequence< OUString > & rPaths = PluginManager::getAdditionalSearchPaths();

    const OUString * pPaths = rPaths.getConstArray();
    for ( UINT32 nPos = rPaths.getLength(); nPos--; )
    {
        addPluginsFromPath( pPaths[nPos], rPlugins );
    }
}

//__________________________________________________________________________________________________
Sequence< PluginDescription > XPluginManager_Impl::impl_getPluginDescriptions(void) throw()
{
    Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
    static Sequence<PluginDescription > s_aDescriptions( 0 );
    static bool s_bInit = FALSE;

    if (! s_bInit)
    {
        // collect all distinct plugin dlls
        PluginLocationMap aPlugins;
        add_SO_Plugins( aPlugins );
        add_NS_Plugins( aPlugins );
        add_IE_Plugins( aPlugins );

        // collect mime types of plugin dlls
        for ( PluginLocationMap::iterator iPos( aPlugins.begin() );
              iPos != aPlugins.end();
              ++iPos )
        {
            TCHAR arFileName[MAX_PATH];
            DWORD dwDummy, dwSize;

            // DLL name
            OUString aName( (*iPos).second.getStr() );

            OString aStr( OUStringToOString( aName, RTL_TEXTENCODING_MS_1252 ) );
            ::strcpy( arFileName, aStr.getStr() );
            dwSize = ::GetFileVersionInfoSize( arFileName, &dwDummy );

            if ( !dwSize )
                continue;

            char * pVersionData = new char[dwSize];
            if (pVersionData && ::GetFileVersionInfo( arFileName, 0, dwSize, pVersionData ))
            {
                // optional comment
                OUString aComment;

                TCHAR * pInfo = NULL, * pInfo2 = NULL;
                UINT nSize = 0;
                if (::VerQueryValue( pVersionData, _T("\\StringFileInfo\\040904E4\\ProductName"),
                                     (void**)&pInfo, &nSize ) && pInfo)
                {
                    aComment.operator=( OStringToOUString( OString(pInfo), RTL_TEXTENCODING_MS_1252 ) );
                }

                // mandatory mime type and file extensions
                if (::VerQueryValue( pVersionData, _T("\\StringFileInfo\\040904E4\\MIMEType"),
                                     (void**)&pInfo, &nSize ) && pInfo &&
                    ::VerQueryValue( pVersionData, _T("\\StringFileInfo\\040904E4\\FileExtents"),
                                     (void**)&pInfo2, &nSize ) && pInfo2)
                {
                    OString aStr2( pInfo2 );
                    OString aExt( aStr2 );
                    OString aStr( pInfo );
                    OString aMIME( aStr );
                    aMIME.trim();

                    // count mime tokens
                    USHORT nToken = 0;
                    if (aMIME.getLength())
                    {
                        ++nToken;
                        for ( sal_Int32 n = aMIME.getLength(); n--; )
                        {
                            if (aMIME[ n ] == '|')
                            {
                                ++nToken;
                            }
                        }
                    }
                    sal_Int32 nIndex = 0, nIndex2 = 0;

                    UINT32 nStart = s_aDescriptions.getLength();
                    s_aDescriptions.realloc( nStart + nToken );
                    PluginDescription* pDescriptions = s_aDescriptions.getArray();
                    // for every MIME Type
                    sal_Int32 nTok = 0;
                    while (true)
                    {
                        if (nIndex < 0 || nIndex2 < 0)
                            break;

                        PluginDescription & rDescr = pDescriptions[nStart+nTok];
                        OString aMIMEToken( aMIME.getToken( 0, '|', nIndex ) );
                        OString aExtToken2( aExt.getToken( 0, '|', nIndex2 ) );
                        if( aMIMEToken.getLength() == 0 || aExtToken2.getLength() == 0 )
                            continue;

                        rDescr.Mimetype = OUString(
                            aMIMEToken.getStr(), aMIMEToken.getLength(), RTL_TEXTENCODING_MS_1252 );
                        if (! rDescr.Mimetype.getLength())
                            break;

                        OUString aExtToken( aExtToken2.getStr(), aExtToken2.getLength(), RTL_TEXTENCODING_MS_1252 );
                        rDescr.PluginName = aName;
                        rDescr.Description = aComment;

                        sal_Int32 nPos = 0, nLen = aExtToken.getLength();
                        OUString aExtensions = nLen ? OUString(RTL_CONSTASCII_USTRINGPARAM("*.")) : OUString(RTL_CONSTASCII_USTRINGPARAM("*.*"));

                        for ( ; nPos < nLen; ++nPos )
                        {
                            sal_Unicode c = aExtToken[nPos];
                            switch (c)
                            {
                            case ',':
                            case ';':
                                aExtensions += OUString(RTL_CONSTASCII_USTRINGPARAM(";*."));
                            case ' ':
                                break;
                            case '*':
                                if (nPos < (nLen-1) && aExtToken[ nPos+1 ] == '.')
                                {
                                    ++nPos;
                                    break;
                                }
                            default:
                                aExtensions += OUString( &c, 1 );
                            }
                        }
                        rDescr.Extension = aExtensions;

                        ++nTok;
                    }

                    if (nToken != nTok)
                    {
                        s_aDescriptions.realloc( nTok );
                    }
                }
#if OSL_DEBUG_LEVEL > 1
                else
                    DBG_ERROR( "### cannot get MIME type or extensions!" );
#endif
            }
            if (pVersionData)
                delete[] pVersionData;
        }

        s_bInit = TRUE;
    }
    return s_aDescriptions;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
