/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: reg4allmsi.cxx,v $
 *
 * $Revision: 1.3 $
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

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <string>
#include <strsafe.h>

//----------------------------------------------------------
static const CHAR* g_Extensions[] =
{
    ".doc",     // Microsoft Word Text [0]
    ".dot",     // Microsoft Word Template
    ".rtf",     // rtf text
    ".docx",    // Office Word 2007 XML document
    ".docm",    // Office Word 2007 XML macro-enabled document
    ".dotx",    // Office Word 2007 XML template
    ".dotm",    // Office Word 2007 XML macro-enabled template
    ".xlw",     // Microsoft Excel
    ".xls",     // Microsoft Excel
    ".xlt",     // Microsoft Excel Template
    ".xlsx",    // Office Excel 2007 XML workbook
    ".xlsm",    // Office Excel 2007 XML macro-enabled workbook
    ".xltx",    // Office Excel 2007 XML template
    ".xltm",    // Office Excel 2007 XML macro-enabled template
    ".xlsb",    // Office Excel 2007 binary workbook (BIFF12)
    ".ppt",     // Microsoft Powerpoint
    ".pps",     // Microsoft Powerpoint
    ".pot",     // Microsoft Powerpoint Template
    ".pptx",    // Office PowerPoint 2007 XML presentation
    ".pptm",    // Office PowerPoint 2007 macro-enabled XML presentation
    ".potx",    // Office PowerPoint 2007 XML template
    ".potm",    // Office PowerPoint 2007 macro-enabled XML template
    ".ppsx",    // Office PowerPoint 2007 XML show
    0
};

//    ".xlam",    // Office Excel 2007 XML macro-enabled add-in
//    ".ppam",    // Office PowerPoint 2007 macro-enabled XML add-in
//    ".ppsm",    // Office PowerPoint 2007 macro-enabled XML show

//----------------------------------------------------------
#ifdef DEBUG
inline void OutputDebugStringFormat( LPCSTR pFormat, ... )
{
    CHAR    buffer[1024];
    va_list args;

    va_start( args, pFormat );
    StringCchVPrintfA( buffer, sizeof(buffer), pFormat, args );
    OutputDebugStringA( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCSTR, ... )
{
}
#endif

//----------------------------------------------------------
static bool IsModuleSelectedForInstallation( MSIHANDLE handle, LPCTSTR name )
{
    INSTALLSTATE current_state;
    INSTALLSTATE future_state;
    MsiGetFeatureState(handle, name, &current_state, &future_state);
    return (future_state == INSTALLSTATE_LOCAL);
}

//----------------------------------------------------------
static BOOL CheckExtensionInRegistry( LPCSTR lpSubKey )
{
    BOOL    bRet = false;
    HKEY    hKey = NULL;
    LONG    lResult = RegOpenKeyExA( HKEY_CLASSES_ROOT, lpSubKey, 0, KEY_QUERY_VALUE, &hKey );

    if ( ERROR_SUCCESS == lResult )
    {
        CHAR    szBuffer[1024];
        DWORD   nSize = sizeof( szBuffer );

        lResult = RegQueryValueExA( hKey, "", NULL, NULL, (LPBYTE)szBuffer, &nSize );
        if ( ERROR_SUCCESS == lResult )
        {
            szBuffer[nSize] = '\0';
            OutputDebugStringFormat( "Found value [%s] for key [%s].\n", szBuffer, lpSubKey );

            if ( strncmp( szBuffer, "WordPad.Document.1", 18 ) == 0 )
            {   // We will replace registration for word pad
                bRet = true;
            }
            if ( strncmp( szBuffer, "OpenOffice.org.", 15 ) == 0 )
            {   // We will replace registration for our own types, too
                bRet = true;
            }
            if ( strncmp( szBuffer, "ooostub.", 8 ) == 0 )
            {   // We will replace registration for ooostub, too
                bRet = true;
            }
        }
        else    // no default value found -> return TRUE to register for that key
            bRet = true;

        RegCloseKey( hKey );
    }
    else // no key found -> return TRUE to register for that key
        bRet = true;

    return bRet;
}

//----------------------------------------------------------
static LONG DeleteSubKeyTree( HKEY RootKey, LPCSTR lpKey )
{
    HKEY hKey;
    LONG rc = RegOpenKeyExA( RootKey, lpKey, 0, KEY_READ | DELETE, &hKey );

    if (ERROR_SUCCESS == rc)
    {
        LPCSTR    lpSubKey;
        DWORD     nMaxSubKeyLen;

        rc = RegQueryInfoKeyA( hKey, 0, 0, 0, 0, &nMaxSubKeyLen, 0, 0, 0, 0, 0, 0 );
        nMaxSubKeyLen++; // space for trailing '\0'
        lpSubKey = reinterpret_cast<CHAR*>( _alloca( nMaxSubKeyLen*sizeof(CHAR) ) );

        while (ERROR_SUCCESS == rc)
        {
            DWORD nLen = nMaxSubKeyLen;
            rc = RegEnumKeyExA( hKey, 0, (LPSTR)lpSubKey, &nLen, 0, 0, 0, 0);    // always index zero

            if ( ERROR_NO_MORE_ITEMS == rc )
            {
                rc = RegDeleteKeyA( RootKey, lpKey );
                if ( rc == ERROR_SUCCESS )
                    OutputDebugStringFormat( "deleted key [%s] from registry.\n", lpKey );
                else
                    OutputDebugStringFormat( "RegDeleteKeyA %s returned %ld.\n", lpKey, rc );
                break;
            }
            else if ( rc == ERROR_SUCCESS )
            {
                rc = DeleteSubKeyTree( hKey, lpSubKey );
                if ( ERROR_SUCCESS != rc )
                    OutputDebugStringFormat( "RegDeleteKeyA %s returned %ld.\n", lpSubKey, rc );
            }

        }
        RegCloseKey(hKey);
    }
    else
    {
        OutputDebugStringFormat( "RegOpenKeyExA %s returned %ld.\n", lpKey, rc );
    }

    return rc;
}

//----------------------------------------------------------
static BOOL RemoveExtensionInRegistry( LPCSTR lpSubKey )
{
    CHAR    szBuffer[4096];
    DWORD   nSize = sizeof( szBuffer );
    HKEY    hKey = NULL;
    HKEY    hSubKey = NULL;
    LONG    lResult = RegOpenKeyExA( HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes", 0, KEY_QUERY_VALUE, &hKey );

    if ( ERROR_SUCCESS == lResult )
    {
        lResult = RegOpenKeyExA( hKey, lpSubKey, 0, KEY_QUERY_VALUE, &hSubKey );

        if ( ERROR_SUCCESS == lResult )
        {
            DWORD nSubKeys = 1;
            szBuffer[0] = '\0';

            // we get the value of the default key fist and while we are on querying,
            // we ask for the subkey count, too
            lResult = RegQueryValueExA( hSubKey, "", NULL, NULL, (LPBYTE)szBuffer, &nSize );
            if ( ERROR_SUCCESS == lResult )
                RegQueryInfoKeyA( hSubKey, 0, 0, 0, &nSubKeys, 0, 0, 0, 0, 0, 0, 0 );
            RegCloseKey( hSubKey );

            // we will remove all key with an default value starting with ooostub but
            // we have to be careful about MSO keys
            if ( strncmp( szBuffer, "opendocument.", 13 ) == 0 )
            {
                if ( nSubKeys == 0 )
                {
                    DeleteSubKeyTree( hKey, lpSubKey );
                }
                else
                {
                    lResult = RegOpenKeyExA( hKey, lpSubKey, 0, KEY_SET_VALUE, &hSubKey );
                    if ( ERROR_SUCCESS == lResult )
                        RegDeleteValueA( hSubKey, "" );
                    else
                        OutputDebugStringFormat( "Could not open key %s for deleting: RegOpenKeyEx returned %ld.\n", lpSubKey, lResult );
                }
            }
        }

        RegCloseKey( hKey );
    }

    return ( ERROR_SUCCESS == lResult );
}

//----------------------------------------------------------
bool GetMsiProp( MSIHANDLE handle, LPCSTR name, /*out*/std::string& value )
{
    DWORD sz = 0;
    LPSTR dummy = "";
    if (MsiGetPropertyA(handle, name, dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++;
        DWORD nbytes = sz * sizeof(TCHAR);
        LPSTR buff = reinterpret_cast<LPSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetPropertyA(handle, name, buff, &sz);
        value = buff;
        return true;
    }
    return false;
}

//----------------------------------------------------------
bool IsSetMsiProp( MSIHANDLE handle, LPCSTR name )
{
    std::string val;
    GetMsiProp( handle, name, val );
    return (val == "1");
}

//----------------------------------------------------------
static void registerForExtensions( MSIHANDLE handle, BOOL bRegisterAll )
{ // Check all file extensions
    int nIndex = 0;
    while ( g_Extensions[nIndex] != 0 )
    {
        BOOL bRegister = bRegisterAll || CheckExtensionInRegistry( g_Extensions[nIndex] );
        if ( bRegister )
        {
            CHAR sPropName[256];
            StringCchCopyA( sPropName, 256, "REGISTER_" );
            StringCchCatA( sPropName, 256, (g_Extensions[nIndex])+1 );
            CharUpperBuffA( sPropName+9, 4 );
            MsiSetPropertyA( handle, sPropName, "1" );
            OutputDebugStringFormat( "Set MSI property %s.\n", sPropName );
        }
        ++nIndex;
    }
}

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
extern "C" UINT __stdcall FindRegisteredExtensions( MSIHANDLE handle )
{
    bool bRegisterAll = IsSetMsiProp( handle, "REGISTER_ALL_MSO_TYPES" );

    if ( IsSetMsiProp( handle, "REGISTER_NO_MSO_TYPES" ) )
    {
        OutputDebugStringFormat( "FindRegisteredExtensions: Register none!" );
        return ERROR_SUCCESS;
    }
    else if ( bRegisterAll )
        OutputDebugStringFormat( "FindRegisteredExtensions: Force all on" );
    else
        OutputDebugStringFormat( "FindRegisteredExtensions: " );

    registerForExtensions( handle, bRegisterAll );

    return ERROR_SUCCESS;
}

//----------------------------------------------------------
extern "C" UINT __stdcall DeleteRegisteredExtensions( MSIHANDLE /*handle*/ )
{
    OutputDebugStringFormat( "DeleteRegisteredExtensions\n" );

    // remove all file extensions
    int nIndex = 0;
    while ( g_Extensions[nIndex] != 0 )
    {
        RemoveExtensionInRegistry( g_Extensions[nIndex] );
        ++nIndex;
    }

    return ERROR_SUCCESS;
}
