/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

// so_activex.cpp : Implementation of DLL Exports.

// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL,
//      run nmake -f so_activexps.mk in the project directory.

#include "stdio.h"
#include "stdafx2.h"
#include "resource.h"
#include <initguid.h>
#include "so_activex.h"

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-tokens"
    // "#endif !_MIDL_USE_GUIDDEF_" in midl-generated code
#endif
#include "so_activex_i.c"
#if defined __clang__
#pragma clang diagnostic pop
#endif

#include "SOActiveX.h"

#include <comphelper\documentconstants.hxx>

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_SOActiveX, CSOActiveX)
END_OBJECT_MAP()


#define X64_LIB_NAME "so_activex_x64.dll"
#define X32_LIB_NAME "so_activex.dll"

// to provide windows xp as build systems for mingw we need to define KEY_WOW64_64KEY
// in mingw 3.13 KEY_WOW64_64KEY isn't available < Win2003 systems.
// Also defined in setup_native\source\win32\customactions\reg64\reg64.cxx,source\win32\customactions\shellextensions\shellextensions.cxx and
// extensions\source\activex\main\so_activex.cpp
#ifndef KEY_WOW64_64KEY
    #define KEY_WOW64_64KEY (0x0100)
#endif
#ifndef KEY_WOW64_32KEY
    #define KEY_WOW64_32KEY (0x0200)
#endif

const REGSAM n64KeyAccess = KEY_ALL_ACCESS | KEY_WOW64_64KEY;
const REGSAM n32KeyAccess = KEY_ALL_ACCESS;

#ifdef _AMD64_
const BOOL bX64 = TRUE;
#define REG_DELETE_KEY_A( key, aPath, nKeyAccess ) RegDeleteKeyExA( key, aPath, nKeyAccess, 0 )
#else
const BOOL bX64 = FALSE;
#define REG_DELETE_KEY_A( key, aPath, nKeyAccess ) RegDeleteKeyA( key, aPath )
#endif

// MinGW doesn't know anything about RegDeleteKeyExA if WINVER < 0x0502.
extern "C" {
WINADVAPI LONG WINAPI RegDeleteKeyExA(HKEY,LPCSTR,REGSAM,DWORD);
}


// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_SO_ACTIVEXLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}


// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow()
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry

// for now database component and chart are always installed
#define SUPPORTED_EXT_NUM 30
const char* aFileExt[] = { ".vor",
                           ".sds", ".sda", ".sdd", ".sdp", ".sdc", ".sdw", ".smf",
                           ".stw", ".stc", ".sti", ".std",
                           ".sxw", ".sxc", ".sxi", ".sxd", ".sxg", ".sxm",
                           ".ott", ".otg", ".otp", ".ots", ".otf",
                           ".odt", ".oth", ".odm", ".odg", ".odp", ".ods", ".odf"};
const char* aMimeType[] = {
                          "application/vnd.stardivision.writer",

                          "application/vnd.stardivision.chart",
                          "application/vnd.stardivision.draw",
                          "application/vnd.stardivision.impress",
                          "application/vnd.stardivision.impress-packed",
                          "application/vnd.stardivision.calc",
                          "application/vnd.stardivision.writer",
                          "application/vnd.stardivision.math",

                          MIMETYPE_VND_SUN_XML_WRITER_TEMPLATE_ASCII,
                          MIMETYPE_VND_SUN_XML_CALC_TEMPLATE_ASCII,
                          MIMETYPE_VND_SUN_XML_IMPRESS_TEMPLATE_ASCII,
                          MIMETYPE_VND_SUN_XML_DRAW_TEMPLATE_ASCII,

                          MIMETYPE_VND_SUN_XML_WRITER_ASCII,
                          MIMETYPE_VND_SUN_XML_CALC_ASCII,
                          MIMETYPE_VND_SUN_XML_IMPRESS_ASCII,
                          MIMETYPE_VND_SUN_XML_DRAW_ASCII,
                          MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII,
                          MIMETYPE_VND_SUN_XML_MATH_ASCII,

                          MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII,
                          MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_TEMPLATE_ASCII,
                          MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII,
                          MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII,
                          MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII,
                          MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII,

                          MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII,
                          MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII,
                          MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII,
                          MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII,
                          MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII,
                          MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII,
                          MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII };

const int nForModes[] = { 16,
                           1,  2,  4,  4,  8, 16, 32,
                          16,  8,  4,  2,
                          16,  8,  4,  2, 16, 32,
                          16,  2,  4,  8, 32,
                          16, 16, 16,  2,  4,  8, 32 };

const char* aClassID = "{67F2A879-82D5-4A6D-8CC5-FFB3C114B69D}";
const char* aTypeLib = "{61FA3F13-8061-4796-B055-3697ED28CB38}";

// ISOComWindowPeer interface information
const char* aInterIDWinPeer = "{BF5D10F3-8A10-4A0B-B150-2B6AA2D7E118}";
const char* aProxyStubWinPeer = "{00020424-0000-0000-C000-000000000046}";

// ISODispatchInterceptor interface information
const char* aInterIDDispInt = "{9337694C-B27D-4384-95A4-9D8E0EABC9E5}";
const char* aProxyStubDispInt = "{00020424-0000-0000-C000-000000000046}";

// ISOActionsApproval interface information
const char* aInterIDActApprove = "{029E9F1E-2B3F-4297-9160-8197DE7ED54F}";
const char* aProxyStubActApprove = "{00020424-0000-0000-C000-000000000046}";

// The following prefix is required for HKEY_LOCAL_MACHINE and HKEY_CURRENT_USER ( not for HKEY_CLASSES_ROOT )
const char* aLocalPrefix = "Software\\Classes\\";

BOOL createKey( HKEY hkey,
                const char* aKeyToCreate,
        REGSAM nKeyAccess,
                const char* aValue = NULL,
                const char* aChildName = NULL,
                const char* aChildValue = NULL )
{
    HKEY hkey1;

    return ( ERROR_SUCCESS == RegCreateKeyExA( hkey, aKeyToCreate, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey1 , NULL )
           && ( !aValue || ERROR_SUCCESS == RegSetValueExA( hkey1,
                                                           "",
                                                           0,
                                                           REG_SZ,
                       reinterpret_cast<const BYTE*>(aValue),
                       sal::static_int_cast<DWORD>(strlen(aValue))))
           && ( !aChildName || ERROR_SUCCESS == RegSetValueExA( hkey1,
                                                               aChildName,
                                                               0,
                                                               REG_SZ,
                       reinterpret_cast<const BYTE*>(aChildValue),
                       sal::static_int_cast<DWORD>(strlen(aChildValue))))
           && ERROR_SUCCESS == RegCloseKey( hkey1 ) );

}

STDAPI DllUnregisterServerNative( int nMode, BOOL bForAllUsers, BOOL bFor64Bit );
STDAPI DllRegisterServerNative_Impl( int nMode, BOOL bForAllUsers, REGSAM nKeyAccess, const char* pProgramPath, const char* pLibName )
{
    BOOL aResult = FALSE;

    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    HKEY        hkey2 = NULL;
    HKEY        hkey3 = NULL;
    HKEY        hkey4 = NULL;
    char aSubKey[513];
    int         ind;
    const char* aPrefix = aLocalPrefix; // bForAllUsers ? "" : aLocalPrefix;


    // In case SO7 is installed for this user he can have local registry entries that will prevent him from
    // using SO8 ActiveX control. The fix is just to clean up the local entries related to ActiveX control.
    // Unfortunately it can be done only for the user who installs the office.
    if ( bForAllUsers )
        DllUnregisterServerNative( nMode, sal_False, sal_False );

    if ( pProgramPath && strlen( pProgramPath ) < 1024 )
    {
        char pActiveXPath[1124];
        char pActiveXPath101[1124];

        sprintf( pActiveXPath, "%s\\%s", pProgramPath, pLibName );
        sprintf( pActiveXPath101, "%s\\%s, 101", pProgramPath, pLibName );

        {
            wsprintfA( aSubKey, "%sCLSID\\%s", aPrefix, aClassID );
            aResult =
                ( ERROR_SUCCESS == RegCreateKeyExA( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey , NULL )
                    && ERROR_SUCCESS == RegSetValueExA( hkey, "", 0, REG_SZ, (const BYTE*)"SOActiveX Class", 17 )
                    && createKey( hkey, "Control", nKeyAccess )
                    && createKey( hkey, "EnableFullPage", nKeyAccess )
                    && createKey( hkey, "InprocServer32", nKeyAccess, pActiveXPath, "ThreadingModel", "Apartment" )
                    && createKey( hkey, "MiscStatus", nKeyAccess, "0" )
                    && createKey( hkey, "MiscStatus\\1", nKeyAccess, "131473" )
                    && createKey( hkey, "ProgID", nKeyAccess, "so_activex.SOActiveX.1" )
                    && createKey( hkey, "Programmable", nKeyAccess )
                    && createKey( hkey, "ToolboxBitmap32", nKeyAccess, pActiveXPath101 )
                    && createKey( hkey, "TypeLib", nKeyAccess, aTypeLib )
                    && createKey( hkey, "Version", nKeyAccess, "1.0" )
                    && createKey( hkey, "VersionIndependentProgID", nKeyAccess, "so_activex.SOActiveX" )
                && ERROR_SUCCESS == RegCloseKey( hkey )
                && ERROR_SUCCESS == RegCreateKeyExA( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aPrefix, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey , NULL )
                    && createKey( hkey, "so_activex.SOActiveX", nKeyAccess, "SOActiveX Class" )
                    && ERROR_SUCCESS == RegCreateKeyExA( hkey, "so_activex.SOActiveX", 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey1 , NULL )
                        && createKey( hkey1, "CLSID", nKeyAccess, aClassID )
                        && createKey( hkey1, "CurVer", nKeyAccess, "so_activex.SOActiveX.1" )
                    && ERROR_SUCCESS == RegCloseKey( hkey1 )
                    && createKey( hkey, "so_activex.SOActiveX.1", nKeyAccess, "SOActiveX Class" )
                    && ERROR_SUCCESS == RegCreateKeyExA( hkey, "so_activex.SOActiveX.1", 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey1 , NULL )
                        && createKey( hkey1, "CLSID", nKeyAccess, aClassID )
                    && ERROR_SUCCESS == RegCloseKey( hkey1 )
                    && ERROR_SUCCESS == RegCreateKeyExA( hkey, "TypeLib", 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey1 , NULL )
                        && ERROR_SUCCESS == RegCreateKeyExA( hkey1, aTypeLib, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey2 , NULL )
                            && createKey( hkey2, "1.0", nKeyAccess, "wrap_activex 1.0 Type Library" )
                            && ERROR_SUCCESS == RegCreateKeyExA( hkey2, "1.0", 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey3 , NULL )
                                && ERROR_SUCCESS == RegCreateKeyExA( hkey3, "0", 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey4 , NULL )
                                    && createKey( hkey4, "win32", nKeyAccess, pActiveXPath )
                                && ERROR_SUCCESS == RegCloseKey( hkey4 )
                                && createKey( hkey3, "FLAGS", nKeyAccess, "0" )
                                && createKey( hkey3, "HELPDIR", nKeyAccess, pProgramPath )
                            && ERROR_SUCCESS == RegCloseKey( hkey3 )
                        && ERROR_SUCCESS == RegCloseKey( hkey2 )
                    && ERROR_SUCCESS == RegCloseKey( hkey1 )
                    && ERROR_SUCCESS == RegCreateKeyExA( hkey, "Interface", 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey1 , NULL )
                        && createKey( hkey1, aInterIDWinPeer, nKeyAccess, "ISOComWindowPeer" )
                        && ERROR_SUCCESS == RegCreateKeyExA( hkey1, aInterIDWinPeer, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey2 , NULL )
                            && createKey( hkey2, "ProxyStubClsid", nKeyAccess, aProxyStubWinPeer )
                            && createKey( hkey2, "ProxyStubClsid32", nKeyAccess, aProxyStubWinPeer )
                            && createKey( hkey2, "TypeLib", nKeyAccess, aTypeLib, "Version", "1.0" )
                        && ERROR_SUCCESS == RegCloseKey( hkey2 )
                        && createKey( hkey1, aInterIDActApprove, nKeyAccess, "ISOActionsApproval" )
                        && ERROR_SUCCESS == RegCreateKeyExA( hkey1, aInterIDActApprove, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey2 , NULL )
                            && createKey( hkey2, "ProxyStubClsid", nKeyAccess, aProxyStubActApprove )
                            && createKey( hkey2, "ProxyStubClsid32", nKeyAccess, aProxyStubActApprove )
                            && createKey( hkey2, "TypeLib", nKeyAccess, aTypeLib, "Version", "1.0" )
                        && ERROR_SUCCESS == RegCloseKey( hkey2 )
                        && createKey( hkey1, aInterIDDispInt, nKeyAccess, "ISODispatchInterceptor" )
                        && ERROR_SUCCESS == RegCreateKeyExA( hkey1, aInterIDDispInt, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey2 , NULL )
                            && createKey( hkey2, "ProxyStubClsid", nKeyAccess, aProxyStubDispInt )
                            && createKey( hkey2, "ProxyStubClsid32", nKeyAccess, aProxyStubDispInt )
                            && createKey( hkey2, "TypeLib", nKeyAccess, aTypeLib, "Version", "1.0" )
                        && ERROR_SUCCESS == RegCloseKey( hkey2 )
                    && ERROR_SUCCESS == RegCloseKey( hkey1 )
                && ERROR_SUCCESS == RegCloseKey( hkey ) );

            hkey = hkey1 = hkey2 = hkey3 = hkey4 = NULL;
        }
    }

    for( ind = 0; ind < SUPPORTED_EXT_NUM && aResult; ind++ )
    {
        if( nForModes[ind] & nMode )
        {
            wsprintfA( aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix, aMimeType[ind] );
            if ( ERROR_SUCCESS != RegCreateKeyExA( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey, NULL )
              || ERROR_SUCCESS != RegSetValueExA(hkey, "CLSID", 0, REG_SZ,
                    reinterpret_cast<const BYTE *>(aClassID),
                    sal::static_int_cast<DWORD>(strlen(aClassID))) )
            {
                    aResult = FALSE;
            }

            if( hkey )
                RegCloseKey(hkey),hkey= NULL;
        }
    }

    wsprintfA( aSubKey, "%sCLSID\\%s", aPrefix, aClassID );
    if ( aResult && ERROR_SUCCESS == RegOpenKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0, nKeyAccess, &hkey) )
    {
           for( ind = 0; ind < SUPPORTED_EXT_NUM; ind++ )
           {
               wsprintfA( aSubKey, "EnableFullPage\\%s", aFileExt[ind] );
               if ( ERROR_SUCCESS != RegCreateKeyExA( hkey, aSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey1 , NULL ) )
                   aResult = FALSE;

            if ( hkey1 )
                   RegCloseKey(hkey1),hkey1= NULL;
         }
    }
    else
        aResult = FALSE;

    if ( hkey )
           RegCloseKey(hkey),hkey= NULL;

    return aResult;
}

STDAPI DllRegisterServerNative( int nMode, BOOL bForAllUsers, BOOL bFor64Bit, const char* pProgramPath )
{
    HRESULT hr = S_OK;
    if ( bFor64Bit )
        hr = DllRegisterServerNative_Impl( nMode, bForAllUsers, n64KeyAccess, pProgramPath, X64_LIB_NAME );

    if ( SUCCEEDED( hr ) )
        hr = DllRegisterServerNative_Impl( nMode, bForAllUsers, n32KeyAccess, pProgramPath, X32_LIB_NAME );

    return hr;
}



// DllUnregisterServer - Removes entries from the system registry
HRESULT DeleteKeyTree( HKEY hkey, const char* pPath, REGSAM nKeyAccess )
{
    HKEY hkey1 = NULL;

    char pSubKeyName[256];
    // first delete the subkeys
    while( ERROR_SUCCESS == RegOpenKeyExA( hkey, pPath, 0, nKeyAccess, &hkey1)
        && ERROR_SUCCESS == RegEnumKeyA( hkey1, 0, pSubKeyName, 256 )
        && ERROR_SUCCESS == DeleteKeyTree( hkey1, pSubKeyName, nKeyAccess ) )
    {
        RegCloseKey( hkey1 ),hkey1= NULL;
    }

    if ( hkey1 )
        RegCloseKey( hkey1 ),hkey1= NULL;

    // delete the key itself
    return REG_DELETE_KEY_A( hkey, pPath, nKeyAccess & ( KEY_WOW64_64KEY | KEY_WOW64_32KEY ) );
}

STDAPI DllUnregisterServerNative_Impl( int nMode, BOOL bForAllUsers, REGSAM nKeyAccess )
{
    HKEY        hkey = NULL;
    BOOL        fErr = FALSE;
    char aSubKey[513];
    const char*    aPrefix = aLocalPrefix; // bForAllUsers ? "" : aLocalPrefix;

    for( int ind = 0; ind < SUPPORTED_EXT_NUM; ind++ )
    {
        if( nForModes[ind] & nMode )
        {
            DWORD nSubKeys = 0, nValues = 0;
            wsprintfA( aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix, aMimeType[ind] );
            if ( ERROR_SUCCESS != RegCreateKeyExA( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey, NULL ) )
                fErr = TRUE;
            else
            {
                if ( ERROR_SUCCESS != RegDeleteValue( hkey, "CLSID" ) )
                    fErr = TRUE;

                if ( ERROR_SUCCESS != RegQueryInfoKey(  hkey, NULL, NULL, NULL,
                                                    &nSubKeys, NULL, NULL,
                                                    &nValues, NULL, NULL, NULL, NULL ) )
                {
                    RegCloseKey( hkey ), hkey = NULL;
                    fErr = TRUE;
                }
                else
                {
                    RegCloseKey( hkey ), hkey = NULL;
                    if ( !nSubKeys && !nValues )
                        DeleteKeyTree( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, nKeyAccess );
                }
            }

               wsprintfA( aSubKey, "%s%s", aPrefix, aFileExt[ind] );
               if ( ERROR_SUCCESS != RegCreateKeyExA( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey, NULL ) )
                fErr = TRUE;
            else
            {
                   if ( ERROR_SUCCESS != RegQueryInfoKey(  hkey, NULL, NULL, NULL,
                                                    &nSubKeys, NULL, NULL,
                                                    &nValues, NULL, NULL, NULL, NULL ) )
                {
                    RegCloseKey( hkey ), hkey = NULL;
                    fErr = TRUE;
                }
                else
                {
                    RegCloseKey( hkey ), hkey = NULL;
                    if ( !nSubKeys && !nValues )
                        DeleteKeyTree( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, nKeyAccess );
                }
            }
        }
    }

    wsprintfA( aSubKey, "%sCLSID\\%s", aPrefix, aClassID );
    if( ERROR_SUCCESS != DeleteKeyTree( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, nKeyAccess ) )
           fErr = TRUE;

    wsprintfA( aSubKey, "%sso_activex.SOActiveX", aPrefix );
    if( ERROR_SUCCESS != DeleteKeyTree( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, nKeyAccess ) )
           fErr = TRUE;

    wsprintfA( aSubKey, "%sso_activex.SOActiveX.1", aPrefix );
    if( ERROR_SUCCESS != DeleteKeyTree( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, nKeyAccess ) )
           fErr = TRUE;

    wsprintfA( aSubKey, "%s\\TypeLib\\%s", aPrefix, aTypeLib );
    if( ERROR_SUCCESS != DeleteKeyTree( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, nKeyAccess ) )
           fErr = TRUE;

    wsprintfA( aSubKey, "%s\\Interface\\%s", aPrefix, aInterIDWinPeer );
    if( ERROR_SUCCESS != DeleteKeyTree( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, nKeyAccess ) )
           fErr = TRUE;

    wsprintfA( aSubKey, "%s\\Interface\\%s", aPrefix, aInterIDDispInt );
    if( ERROR_SUCCESS != DeleteKeyTree( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, nKeyAccess ) )
           fErr = TRUE;

    wsprintfA( aSubKey, "%s\\Interface\\%s", aPrefix, aInterIDActApprove );
    if( ERROR_SUCCESS != DeleteKeyTree( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, nKeyAccess ) )
           fErr = TRUE;

    return !fErr;
}

STDAPI DllUnregisterServerNative( int nMode, BOOL bForAllUsers, BOOL bFor64Bit )
{
    HRESULT hr = DllUnregisterServerNative_Impl( nMode, bForAllUsers, n32KeyAccess );
    if ( SUCCEEDED( hr ) && bFor64Bit )
        hr = DllUnregisterServerNative_Impl( nMode, bForAllUsers, n64KeyAccess );

    return hr;
}



// DllRegisterServerDoc - Adds entries to the system registry

#define SUPPORTED_MSEXT_NUM 7
const char* aMSFileExt[] = { ".dot", ".doc", ".xlt", ".xls", ".pot", ".ppt", ".pps" };
const char* aMSMimeType[] = { "application/msword",
                          "application/msword",
                          "application/msexcell",
                          "application/msexcell",
                          "application/mspowerpoint",
                          "application/mspowerpoint",
                          "application/mspowerpoint" };
const int nForMSModes[] = { 1, 1, 2, 2, 4, 4, 4 };

STDAPI DllUnregisterServerDoc( int nMode, BOOL bForAllUsers, BOOL bFor64Bit );
STDAPI DllRegisterServerDoc_Impl( int nMode, BOOL bForAllUsers, REGSAM nKeyAccess )
{
    BOOL aResult = TRUE;

    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    char aSubKey[513];
    int         ind;
    const char*    aPrefix = aLocalPrefix; // bForAllUsers ? "" : aLocalPrefix;

    // In case SO7 is installed for this user he can have local registry entries that will prevent him from
    // using SO8 ActiveX control. The fix is just to clean up the local entries related to ActiveX control.
    // Unfortunately it can be done only for the user who installs the office.
    if ( bForAllUsers )
        DllUnregisterServerDoc( nMode, sal_False, sal_False );

    for( ind = 0; ind < SUPPORTED_MSEXT_NUM && aResult; ind++ )
    {
        if( nForMSModes[ind] & nMode )
        {
               wsprintfA( aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix, aMSMimeType[ind] );
               if ( ERROR_SUCCESS != RegCreateKeyExA( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey, NULL )
               || ERROR_SUCCESS != RegSetValueExA(hkey, "Extension", 0, REG_SZ,
                        reinterpret_cast<const BYTE *>(aMSFileExt[ind]),
                        sal::static_int_cast<DWORD>(strlen(aMSFileExt[ind])))
               || ERROR_SUCCESS != RegSetValueExA(hkey, "CLSID", 0, REG_SZ,
                        reinterpret_cast<const BYTE *>(aClassID),
                        sal::static_int_cast<DWORD>(strlen(aClassID))))
               {
                       aResult = FALSE;
               }

               if( hkey )
                   RegCloseKey(hkey),hkey= NULL;

               wsprintfA( aSubKey, "%s%s", aPrefix, aMSFileExt[ind] );
               if ( ERROR_SUCCESS != RegCreateKeyExA( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey, NULL )
               || ERROR_SUCCESS != RegSetValueExA(hkey, "Content Type", 0, REG_SZ,
                        reinterpret_cast<const BYTE *>(aMSMimeType[ind]),
                        sal::static_int_cast<DWORD>(strlen(aMSMimeType[ind]))))
               {
                       aResult = FALSE;
               }

               if( hkey )
                   RegCloseKey(hkey),hkey= NULL;
        }
    }

    wsprintfA( aSubKey, "%sCLSID\\%s", aPrefix, aClassID );
    if ( aResult && ERROR_SUCCESS == RegCreateKeyExA( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey , NULL )
      && createKey( hkey, "EnableFullPage", nKeyAccess ) )
    {
           for( ind = 0; ind < SUPPORTED_MSEXT_NUM; ind++ )
           {
            if( nForMSModes[ind] & nMode )
            {
                   wsprintfA( aSubKey, "EnableFullPage\\%s", aMSFileExt[ind] );
                   if ( ERROR_SUCCESS != RegCreateKeyExA( hkey, aSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey1 , NULL ) )
                       aResult = FALSE;

                   if ( hkey1 )
                       RegCloseKey(hkey1),hkey1= NULL;
            }
           }
    }
    else
        aResult = FALSE;

    if ( hkey )
        RegCloseKey(hkey),hkey= NULL;

    return aResult;
}

STDAPI DllRegisterServerDoc( int nMode, BOOL bForAllUsers, BOOL bFor64Bit )
{
    HRESULT hr = S_OK;
    if ( bFor64Bit )
        hr = DllRegisterServerDoc_Impl( nMode, bForAllUsers, n64KeyAccess );

    if ( SUCCEEDED( hr ) )
        hr = DllRegisterServerDoc_Impl( nMode, bForAllUsers, n32KeyAccess );

    return hr;
}



// DllUnregisterServerDoc - Removes entries from the system registry

STDAPI DllUnregisterServerDoc_Impl( int nMode, BOOL bForAllUsers, REGSAM nKeyAccess )
{
    HKEY        hkey = NULL;
    BOOL        fErr = FALSE;
    char aSubKey[513];
    const char*    aPrefix = aLocalPrefix; // bForAllUsers ? "" : aLocalPrefix;

      for( int ind = 0; ind < SUPPORTED_MSEXT_NUM; ind++ )
       {
        if( nForMSModes[ind] & nMode )
        {
            DWORD nSubKeys = 0, nValues = 0;

               wsprintfA( aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix, aMSMimeType[ind] );
               if ( ERROR_SUCCESS != RegCreateKeyExA( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey, NULL ) )
                fErr = TRUE;
            else
            {
                   if ( ERROR_SUCCESS != RegDeleteValue( hkey, "Extension" ) )
                    fErr = TRUE;

                   if ( ERROR_SUCCESS != RegDeleteValue( hkey, "CLSID" ) )
                    fErr = TRUE;

                if ( ERROR_SUCCESS != RegQueryInfoKey(  hkey, NULL, NULL, NULL,
                                                        &nSubKeys, NULL, NULL,
                                                        &nValues, NULL, NULL, NULL, NULL ) )
                {
                    RegCloseKey( hkey ), hkey = NULL;
                    fErr = TRUE;
                }
                else
                {
                    RegCloseKey( hkey ), hkey = NULL;
                    if ( !nSubKeys && !nValues )
                        DeleteKeyTree( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, nKeyAccess );
                }
            }

               wsprintfA( aSubKey, "%s%s", aPrefix, aMSFileExt[ind] );
               if ( ERROR_SUCCESS != RegCreateKeyExA( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, nKeyAccess, NULL, &hkey, NULL ) )
                fErr = TRUE;
            else
            {
                   if ( ERROR_SUCCESS != RegDeleteValue( hkey, "Content Type" ) )
                    fErr = TRUE;

                if ( ERROR_SUCCESS != RegQueryInfoKey(  hkey, NULL, NULL, NULL,
                                                        &nSubKeys, NULL, NULL,
                                                        &nValues, NULL, NULL, NULL, NULL ) )
                {
                    RegCloseKey( hkey ), hkey = NULL;
                    fErr = TRUE;
                }
                else
                {
                    RegCloseKey( hkey ), hkey = NULL;
                    if ( !nSubKeys && !nValues )
                        DeleteKeyTree( bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, nKeyAccess );
                }
            }
        }
       }

    return !fErr;
}

STDAPI DllUnregisterServerDoc( int nMode, BOOL bForAllUsers, BOOL bFor64Bit )
{
    HRESULT hr = S_OK;
    if ( bFor64Bit )
        hr = DllUnregisterServerDoc_Impl( nMode, bForAllUsers, n64KeyAccess );

    if ( SUCCEEDED( hr ) )
        hr = DllUnregisterServerDoc_Impl( nMode, bForAllUsers, n32KeyAccess );

    return hr;
}


// DllRegisterServer - regsvr32 entry point

STDAPI DllRegisterServer()
{
    HRESULT aResult = E_FAIL;

    HMODULE aCurModule = GetModuleHandleA( bX64 ? X64_LIB_NAME : X32_LIB_NAME );
    DWORD nLibNameLen = sal::static_int_cast<DWORD>(
            strlen((bX64) ? X64_LIB_NAME : X32_LIB_NAME));

    if( aCurModule )
    {
        char pProgramPath[1024];
        DWORD nLen = GetModuleFileNameA( aCurModule, pProgramPath, 1024 );
        if ( nLen && nLen > nLibNameLen + 1 )
        {
            pProgramPath[ nLen - nLibNameLen - 1 ] = 0;
            aResult = DllRegisterServerNative( 31, TRUE, bX64, pProgramPath );
            if( SUCCEEDED( aResult ) )
                aResult = DllRegisterServerDoc( 31, TRUE, bX64 );
            else
            {
                aResult = DllRegisterServerNative( 31, FALSE, bX64, pProgramPath );
                if( SUCCEEDED( aResult ) )
                    aResult = DllRegisterServerDoc( 31, FALSE, bX64 );
            }
        }
    }

    return aResult;
}


// DllUnregisterServer - regsvr32 entry point

STDAPI DllUnregisterServer()
{
    DllUnregisterServerDoc( 63, FALSE, bX64 );
    DllUnregisterServerNative( 63, FALSE, bX64 );
    DllUnregisterServerDoc( 63, TRUE, bX64 );
    return DllUnregisterServerNative( 63, TRUE, bX64 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
