/*************************************************************************
 *
 *  $RCSfile: winmgr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <vcl/svapp.hxx>
#include <tools/fsys.hxx>
#include <osl/mutex.hxx>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <plugin/impl.hxx>

#ifdef WNT
#if _MSC_VER >= 1200
#include <tools/prewin.h>
#else
#include <tools/presys.h>
#endif
#endif
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <winreg.h>
#include <winbase.h>
#include <objbase.h>
#ifdef WNT
#if _MSC_VER >= 1200
#include <tools/postwin.h>
#else
#include <tools/postsys.h>
#endif
#endif
#if STLPORT_VERSION >= 321
#include <stl/list>
#include <stl/map>
#include <stl/algorithm>
#else
#include <stl/map.h>
#endif

using namespace rtl;
using namespace std;
using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::plugin;

typedef map< OString, OUString, less< OString > > PluginLocationMap;


//__________________________________________________________________________________________________
static void addPluginsFromPath( const TCHAR * pPluginsPath, PluginLocationMap & rPlugins )
{
    // append dll name pattern we are looking for
    TCHAR arPluginsPath[MAX_PATH];
    arPluginsPath[0] = 0;

#ifdef UNICODE
    if (::rtl_ustr_indexOfStr( pPluginsPath, L"%programfiles%" ) == 0)
    {
        const char * p = ::getenv( "ProgramFiles" );
        if (p)
        {
            OUString aStr( OUString::createFromAscii( p ) );
            ::lstrcpy( arPluginsPath, aStr.getStr() );
            pPluginsPath += 14;
        }
    }
#else
    if (::rtl_str_indexOfStr( pPluginsPath, "%programfiles%" ) == 0)
    {
        const char * p = ::getenv( "ProgramFiles" );
        if (p)
        {
            ::lstrcpy( arPluginsPath, p );
            pPluginsPath += 14;
        }
    }
#endif
    ::lstrcat( arPluginsPath, pPluginsPath );
    ::lstrcat( arPluginsPath, _T("\\") );

    TCHAR arPluginsPattern[MAX_PATH];
    ::lstrcpy( arPluginsPattern, arPluginsPath );
    ::lstrcat( arPluginsPattern, _T("NP*.DLL") );

    WIN32_FIND_DATA aFindData;
    HANDLE hFind = ::FindFirstFile( arPluginsPattern, &aFindData );

    while (hFind != INVALID_HANDLE_VALUE)
    {
#ifdef UNICODE
        OString aStr( OUStringToOString( aFindData.cFileName, RTL_TEXTENCODING_MS_1252 ) );
        OString aName( aStr ); // cAsE?
#else
        OString aName( aFindData.cFileName );
#endif
        aName.toLowerCase();

        // no netscape default plugin anymore...
        // and no double plugin dlls
        if (!aName.equals( "npnul32.dll" ) && rPlugins.find( aName ) == rPlugins.end())
        {
            TCHAR arComplete[MAX_PATH];
            ::lstrcpy( arComplete, arPluginsPath );
            ::lstrcat( arComplete, aFindData.cFileName );

#ifdef UNICODE
            rPlugins[aName] = OUString( arComplete );
#else
            OUString aStr( OStringToOUString( arComplete, RTL_TEXTENCODING_MS_1252 ) );
            rPlugins[aName] = OUString( aStr.getStr() );
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

#ifdef UNICODE
    ::lstrcpy( arPluginsPath, rPath.getStr() );
#else
    OString aStr( OUStringToOString( rPath, RTL_TEXTENCODING_MS_1252 ) );
    ::strcpy( arPluginsPath, aStr.getStr() );
#endif

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

//__________________________________________________________________________________________________
static void add_NS_Plugins( PluginLocationMap & rPlugins )
{
    HKEY hKey1, hKey2;
    TCHAR arCurrent[MAX_PATH];
    DWORD dwType, dwCurrentSize = sizeof(arCurrent);

    if (::RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("Software\\Netscape\\Netscape Navigator"),
                        0, KEY_READ, &hKey1 ) == ERROR_SUCCESS)
    {
        if (::RegQueryValueEx( hKey1, _T("CurrentVersion"), NULL, &dwType,
                               (LPBYTE)arCurrent, &dwCurrentSize ) == ERROR_SUCCESS &&
            (dwType == REG_SZ || dwType == REG_EXPAND_SZ) &&
            ::RegOpenKeyEx( hKey1, ::lstrcat( arCurrent, _T("\\Main") ),
                            0, KEY_READ, &hKey2 ) == ERROR_SUCCESS)
        {
            dwCurrentSize = sizeof(arCurrent);
            if (::RegQueryValueEx( hKey2, _T("Plugins Directory"), NULL, &dwType,
                                   (LPBYTE)arCurrent, &dwCurrentSize ) == ERROR_SUCCESS &&
                (dwType == REG_SZ || dwType == REG_EXPAND_SZ))
            {
                addPluginsFromPath( arCurrent, rPlugins );
            }
            ::RegCloseKey( hKey2 );
        }
        ::RegCloseKey( hKey1 );
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
Sequence< PluginDescription > XPluginManager_Impl::getPluginDescriptions(void) throw()
{
    Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
    static Sequence<PluginDescription > s_aDescriptions( 0 );
    static s_bInit = FALSE;

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

#ifdef UNICODE
            ::lstrcpy( arFileName, aName.getStr() );
#else
            OString aStr( OUStringToOString( aName, RTL_TEXTENCODING_MS_1252 ) );
            ::strcpy( arFileName, aStr.getStr() );
#endif
            dwSize = ::GetFileVersionInfoSize( arFileName, &dwDummy );

            char * pVersionData = NULL;
            if (dwSize && (pVersionData = new char[dwSize]) &&
                ::GetFileVersionInfo( arFileName, 0, dwSize, pVersionData ))
            {
                // optional comment
                OUString aComment;

                TCHAR * pInfo, * pInfo2;
                UINT nSize;
                if (::VerQueryValue( pVersionData, _T("\\StringFileInfo\\040904E4\\ProductName"),
                                     (void**)&pInfo, &nSize ) && pInfo)
                {
#ifdef UNICODE
                    aComment.operator=( pInfo );
#else
                    aComment.operator=( OStringToOUString( OString(pInfo), RTL_TEXTENCODING_MS_1252 ) );
#endif
                }

                // mandatory mime type and file extensions
                if (::VerQueryValue( pVersionData, _T("\\StringFileInfo\\040904E4\\MIMEType"),
                                     (void**)&pInfo, &nSize ) && pInfo &&
                    ::VerQueryValue( pVersionData, _T("\\StringFileInfo\\040904E4\\FileExtents"),
                                     (void**)&pInfo2, &nSize ) && pInfo2)
                {
#ifdef UNICODE
                    OUString aExt( pInfo2 );
                    OUString aMIME( pInfo );
#else
                    OString aStr2( pInfo2 );
                    OString aExt( aStr2 );
                    OString aStr( pInfo );
                    OString aMIME( aStr );
#endif
                    aMIME.trim();

                    USHORT nToken  = aExt.getTokenCount( '|' );
                    USHORT nMToken = aMIME.getTokenCount( '|' );
                    if (nToken != nMToken)
                    {
                        DBG_ERROR( "### mime type count differ to extension count!" );
                        nToken = (nToken > nMToken ? nMToken : nToken);
                    }

                    UINT32 nStart = s_aDescriptions.getLength();
                    s_aDescriptions.realloc( nStart + nToken );
                    PluginDescription* pDescriptions = s_aDescriptions.getArray();
                    // for every MIME Type
                    for ( ; nToken--; )
                    {
                        PluginDescription & rDescr = pDescriptions[nStart+nToken];
                        rDescr.PluginName          = aName;
                        rDescr.Description         = aComment;
                        rDescr.Mimetype            = OStringToOUString( aMIME.getToken( nToken, '|' ), RTL_TEXTENCODING_MS_1252 );

                        OUString aExtToken( OStringToOUString( aExt.getToken( nToken, '|' ), RTL_TEXTENCODING_MS_1252 ) );
                        USHORT nPos = 0, nLen = aExtToken.getLength();
                        OUString aExtensions( OUString::createFromAscii( nLen ? "*." : "*.*" ) );

                        for ( ; nPos < nLen; ++nPos )
                        {
                            sal_Unicode c = aExtToken[nPos];
                            switch (c)
                            {
                            case ',':
                            case ';':
                                aExtensions += OUString::createFromAscii( ";*." );
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
                    }
                }
#if DEBUG
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


