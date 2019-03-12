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

#include <stdio.h>
#include "StdAfx2.h"
#include "resource.h"
#include <initguid.h>

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include <so_activex.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-tokens"
    // "#endif !_MIDL_USE_GUIDDEF_" in midl-generated code
#endif
#include <so_activex_i.c>
#if defined __clang__
#pragma clang diagnostic pop
#endif

#include "SOActiveX.h"

#include <comphelper\documentconstants.hxx>
#include <exception>

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_SOActiveX, CSOActiveX)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif
END_OBJECT_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

#define X64_LIB_NAME L"so_activex_x64.dll"
#define X32_LIB_NAME L"so_activex.dll"

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
const bool bX64 = true;
#define REG_DELETE_KEY_A( key, aPath, nKeyAccess ) RegDeleteKeyExA( key, aPath, nKeyAccess, 0 )
#else
const bool bX64 = false;
#define REG_DELETE_KEY_A( key, aPath, nKeyAccess ) RegDeleteKeyA( key, aPath )
#endif

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

namespace
{
// Wraps an updatable Win32 error; keeps first incoming error.
// Ctor defines if upd will throw std::exception on error or
// return false.
class Status
{
public:
    explicit Status(bool bTrow)
        : m_bThrow(bTrow)
    {
    }
    // used to check success of an operation, and update the status if it's still ERROR_SUCCESS
    bool upd(LSTATUS nNewStatus)
    {
        if (m_nStatus == ERROR_SUCCESS)
            m_nStatus = nNewStatus;
        if (m_bThrow && nNewStatus != ERROR_SUCCESS)
            throw std::exception();
        return nNewStatus == ERROR_SUCCESS;
    };
    LSTATUS get() { return m_nStatus; }
    operator bool() { return m_nStatus == ERROR_SUCCESS; }

private:
    LSTATUS m_nStatus = ERROR_SUCCESS;
    const bool m_bThrow;
};

class HRegKey
{
public:
    ~HRegKey()
    {
        if (m_hkey)
            RegCloseKey(m_hkey);
    }
    PHKEY operator&() { return &m_hkey; }
    operator HKEY() { return m_hkey; }

private:
    HKEY m_hkey = nullptr;
};
}

// for now database component and chart are always installed
#define SUPPORTED_EXT_NUM 30
const char* const aFileExt[] = { ".vor",
                           ".sds", ".sda", ".sdd", ".sdp", ".sdc", ".sdw", ".smf",
                           ".stw", ".stc", ".sti", ".std",
                           ".sxw", ".sxc", ".sxi", ".sxd", ".sxg", ".sxm",
                           ".ott", ".otg", ".otp", ".ots", ".otf",
                           ".odt", ".oth", ".odm", ".odg", ".odp", ".ods", ".odf"};
const char* const aMimeType[] = {
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

const char* const aClassID = "{67F2A879-82D5-4A6D-8CC5-FFB3C114B69D}";
const char* const aTypeLib = "{61FA3F13-8061-4796-B055-3697ED28CB38}";

// ISOComWindowPeer interface information
const char* const aInterIDWinPeer = "{BF5D10F3-8A10-4A0B-B150-2B6AA2D7E118}";
const char* const aProxyStubWinPeer = "{00020424-0000-0000-C000-000000000046}";

// ISODispatchInterceptor interface information
const char* const aInterIDDispInt = "{9337694C-B27D-4384-95A4-9D8E0EABC9E5}";
const char* const aProxyStubDispInt = "{00020424-0000-0000-C000-000000000046}";

// ISOActionsApproval interface information
const char* const aInterIDActApprove = "{029E9F1E-2B3F-4297-9160-8197DE7ED54F}";
const char* const aProxyStubActApprove = "{00020424-0000-0000-C000-000000000046}";

// The following prefix is required for HKEY_LOCAL_MACHINE and HKEY_CURRENT_USER ( not for HKEY_CLASSES_ROOT )
const char* const aLocalPrefix = "Software\\Classes\\";

static LSTATUS createKey( HKEY hkey,
                const char* aKeyToCreate,
                REGSAM nKeyAccess,
                const char* aValue = nullptr,
                const char* aChildName = nullptr,
                const char* aChildValue = nullptr )
{
    Status s(false); // no throw
    HRegKey hkey1;
    if (s.upd(RegCreateKeyExA(hkey, aKeyToCreate, 0, nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess,
                              nullptr, &hkey1, nullptr)))
    {
        if (aValue)
            s.upd(RegSetValueExA(hkey1, "", 0, REG_SZ, reinterpret_cast<const BYTE*>(aValue),
                                 sal::static_int_cast<DWORD>(strlen(aValue))));
        if (aChildName)
            s.upd(RegSetValueExA(hkey1, aChildName, 0, REG_SZ,
                                 reinterpret_cast<const BYTE*>(aChildValue),
                                 sal::static_int_cast<DWORD>(strlen(aChildValue))));
    }
    return s.get();
}

static LSTATUS createKey(HKEY hkey,
                const wchar_t* aKeyToCreate,
                REGSAM nKeyAccess,
                const wchar_t* aValue = nullptr,
                const wchar_t* aChildName = nullptr,
                const wchar_t* aChildValue = nullptr )
{
    Status s(false); // no throw
    HRegKey hkey1;
    if (s.upd(RegCreateKeyExW(hkey, aKeyToCreate, 0, nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess,
                              nullptr, &hkey1, nullptr)))
    {
        if (aValue)
            s.upd(RegSetValueExW(hkey1, L"", 0, REG_SZ, reinterpret_cast<const BYTE*>(aValue),
                                 sal::static_int_cast<DWORD>(wcslen(aValue) * sizeof(wchar_t))));
        if (aChildName)
            s.upd(RegSetValueExW(
                hkey1, aChildName, 0, REG_SZ, reinterpret_cast<const BYTE*>(aChildValue),
                sal::static_int_cast<DWORD>(wcslen(aChildValue) * sizeof(wchar_t))));
    }
    return s.get();
}

EXTERN_C __declspec(dllexport) HRESULT STDAPICALLTYPE DllUnregisterServerNative( int nMode, BOOL bForAllUsers, BOOL bFor64Bit );
static HRESULT DllRegisterServerNative_Impl( int nMode, BOOL bForAllUsers, REGSAM nKeyAccess, const wchar_t* pProgramPath, const wchar_t* pLibName )
{
    char        aSubKey[513];
    int         ind;
    const char* aPrefix = aLocalPrefix; // bForAllUsers ? "" : aLocalPrefix;

    // In case SO7 is installed for this user he can have local registry entries that will prevent him from
    // using SO8 ActiveX control. The fix is just to clean up the local entries related to ActiveX control.
    // Unfortunately it can be done only for the user who installs the office.
    if ( bForAllUsers )
        DllUnregisterServerNative( nMode, false, false );

    Status s(true); // throw
    try
    {
        if (pProgramPath && wcslen(pProgramPath) < 1024)
        {
            wchar_t pActiveXPath[1124];
            wchar_t pActiveXPath101[1124];

            swprintf(pActiveXPath, L"%s\\%s", pProgramPath, pLibName);
            swprintf(pActiveXPath101, L"%s\\%s, 101", pProgramPath, pLibName);

            {
                wsprintfA(aSubKey, "%sCLSID\\%s", aPrefix, aClassID);
                HRegKey hkey;
                s.upd(RegCreateKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                      aSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess,
                                      nullptr, &hkey, nullptr));
                s.upd(RegSetValueExA(hkey, "", 0, REG_SZ,
                                     reinterpret_cast<const BYTE*>("SOActiveX Class"), 17));
                s.upd(createKey(hkey, "Control", nKeyAccess));
                s.upd(createKey(hkey, "EnableFullPage", nKeyAccess));
                s.upd(createKey(hkey, L"InprocServer32", nKeyAccess, pActiveXPath,
                                L"ThreadingModel", L"Apartment"));
                s.upd(createKey(hkey, "MiscStatus", nKeyAccess, "0"));
                s.upd(createKey(hkey, "MiscStatus\\1", nKeyAccess, "131473"));
                s.upd(createKey(hkey, "ProgID", nKeyAccess, "so_activex.SOActiveX.1"));
                s.upd(createKey(hkey, "Programmable", nKeyAccess));
                s.upd(createKey(hkey, L"ToolboxBitmap32", nKeyAccess, pActiveXPath101));
                s.upd(createKey(hkey, "TypeLib", nKeyAccess, aTypeLib));
                s.upd(createKey(hkey, "Version", nKeyAccess, "1.0"));
                s.upd(createKey(hkey, "VersionIndependentProgID", nKeyAccess,
                                "so_activex.SOActiveX"));
            }
            {
                HRegKey hkey;
                s.upd(RegCreateKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                      aPrefix, 0, nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess,
                                      nullptr, &hkey, nullptr));
                s.upd(createKey(hkey, "so_activex.SOActiveX", nKeyAccess, "SOActiveX Class"));
                {
                    HRegKey hkey1;
                    s.upd(RegCreateKeyExA(hkey, "so_activex.SOActiveX", 0, nullptr,
                                          REG_OPTION_NON_VOLATILE, nKeyAccess, nullptr, &hkey1,
                                          nullptr));
                    s.upd(createKey(hkey1, "CLSID", nKeyAccess, aClassID));
                    s.upd(createKey(hkey1, "CurVer", nKeyAccess, "so_activex.SOActiveX.1"));
                }
                s.upd(createKey(hkey, "so_activex.SOActiveX.1", nKeyAccess, "SOActiveX Class"));
                {
                    HRegKey hkey1;
                    s.upd(RegCreateKeyExA(hkey, "so_activex.SOActiveX.1", 0, nullptr,
                                          REG_OPTION_NON_VOLATILE, nKeyAccess, nullptr, &hkey1,
                                          nullptr));
                    s.upd(createKey(hkey1, "CLSID", nKeyAccess, aClassID));
                }
                {
                    HRegKey hkey1;
                    s.upd(RegCreateKeyExA(hkey, "TypeLib", 0, nullptr, REG_OPTION_NON_VOLATILE,
                                          nKeyAccess, nullptr, &hkey1, nullptr));
                    {
                        HRegKey hkey2;
                        s.upd(RegCreateKeyExA(hkey1, aTypeLib, 0, nullptr, REG_OPTION_NON_VOLATILE,
                                              nKeyAccess, nullptr, &hkey2, nullptr));
                        s.upd(createKey(hkey2, "1.0", nKeyAccess, "wrap_activex 1.0 Type Library"));
                        {
                            HRegKey hkey3;
                            s.upd(RegCreateKeyExA(hkey2, "1.0", 0, nullptr, REG_OPTION_NON_VOLATILE,
                                                  nKeyAccess, nullptr, &hkey3, nullptr));
                            {
                                HRegKey hkey4;
                                s.upd(RegCreateKeyExA(hkey3, "0", 0, nullptr,
                                                      REG_OPTION_NON_VOLATILE, nKeyAccess, nullptr,
                                                      &hkey4, nullptr));
                                s.upd(createKey(hkey4, L"win32", nKeyAccess, pActiveXPath));
                            }
                            s.upd(createKey(hkey3, "FLAGS", nKeyAccess, "0"));
                            s.upd(createKey(hkey3, L"HELPDIR", nKeyAccess, pProgramPath));
                        }
                    }
                }
                {
                    HRegKey hkey1;
                    s.upd(RegCreateKeyExA(hkey, "Interface", 0, nullptr, REG_OPTION_NON_VOLATILE,
                                          nKeyAccess, nullptr, &hkey1, nullptr));
                    s.upd(createKey(hkey1, aInterIDWinPeer, nKeyAccess, "ISOComWindowPeer"));
                    {
                        HRegKey hkey2;
                        s.upd(RegCreateKeyExA(hkey1, aInterIDWinPeer, 0, nullptr,
                                              REG_OPTION_NON_VOLATILE, nKeyAccess, nullptr, &hkey2,
                                              nullptr));
                        s.upd(createKey(hkey2, "ProxyStubClsid", nKeyAccess, aProxyStubWinPeer));
                        s.upd(createKey(hkey2, "ProxyStubClsid32", nKeyAccess, aProxyStubWinPeer));
                        s.upd(createKey(hkey2, "TypeLib", nKeyAccess, aTypeLib, "Version", "1.0"));
                    }
                    s.upd(createKey(hkey1, aInterIDActApprove, nKeyAccess, "ISOActionsApproval"));
                    {
                        HRegKey hkey2;
                        s.upd(RegCreateKeyExA(hkey1, aInterIDActApprove, 0, nullptr,
                                              REG_OPTION_NON_VOLATILE, nKeyAccess, nullptr, &hkey2,
                                              nullptr));
                        s.upd(createKey(hkey2, "ProxyStubClsid", nKeyAccess, aProxyStubActApprove));
                        s.upd(
                            createKey(hkey2, "ProxyStubClsid32", nKeyAccess, aProxyStubActApprove));
                        s.upd(createKey(hkey2, "TypeLib", nKeyAccess, aTypeLib, "Version", "1.0"));
                    }
                    s.upd(createKey(hkey1, aInterIDDispInt, nKeyAccess, "ISODispatchInterceptor"));
                    {
                        HRegKey hkey2;
                        s.upd(RegCreateKeyExA(hkey1, aInterIDDispInt, 0, nullptr,
                                              REG_OPTION_NON_VOLATILE, nKeyAccess, nullptr, &hkey2,
                                              nullptr));
                        s.upd(createKey(hkey2, "ProxyStubClsid", nKeyAccess, aProxyStubDispInt));
                        s.upd(createKey(hkey2, "ProxyStubClsid32", nKeyAccess, aProxyStubDispInt));
                        s.upd(createKey(hkey2, "TypeLib", nKeyAccess, aTypeLib, "Version", "1.0"));
                    }
                }
            }
        }

        for (ind = 0; ind < SUPPORTED_EXT_NUM; ind++)
        {
            if (nForModes[ind] & nMode)
            {
                wsprintfA(aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix, aMimeType[ind]);
                HRegKey hkey;
                s.upd(RegCreateKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                      aSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess,
                                      nullptr, &hkey, nullptr));
                s.upd(RegSetValueExA(hkey, "CLSID", 0, REG_SZ,
                                     reinterpret_cast<const BYTE*>(aClassID),
                                     sal::static_int_cast<DWORD>(strlen(aClassID))));
            }
        }

        {
            wsprintfA(aSubKey, "%sCLSID\\%s", aPrefix, aClassID);
            HRegKey hkey;
            s.upd(RegOpenKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0,
                                nKeyAccess, &hkey));
            for (ind = 0; ind < SUPPORTED_EXT_NUM; ind++)
            {
                wsprintfA(aSubKey, "EnableFullPage\\%s", aFileExt[ind]);
                HRegKey hkey1;
                s.upd(RegCreateKeyExA(hkey, aSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE,
                                      nKeyAccess, nullptr, &hkey1, nullptr));
            }
        }
    }
    catch (const std::exception&) {}

    return HRESULT_FROM_WIN32(s.get());
}

EXTERN_C __declspec(dllexport) HRESULT STDAPICALLTYPE DllRegisterServerNative( int nMode, BOOL bForAllUsers, BOOL bFor64Bit, const wchar_t* pProgramPath )
{
    HRESULT hr = S_OK;
    if ( bFor64Bit )
        hr = DllRegisterServerNative_Impl( nMode, bForAllUsers, n64KeyAccess, pProgramPath, X64_LIB_NAME );

    if ( SUCCEEDED( hr ) )
        hr = DllRegisterServerNative_Impl( nMode, bForAllUsers, n32KeyAccess, pProgramPath, X32_LIB_NAME );

    return hr;
}


// DllUnregisterServer - Removes entries from the system registry
static HRESULT DeleteKeyTree( HKEY hkey, const char* pPath, REGSAM nKeyAccess )
{
    char pSubKeyName[256];
    // first delete the subkeys
    while (true)
    {
        HRegKey hkey1;
        if (ERROR_SUCCESS != RegOpenKeyExA(hkey, pPath, 0, nKeyAccess, &hkey1)
            || ERROR_SUCCESS != RegEnumKeyA(hkey1, 0, pSubKeyName, 256)
            || ERROR_SUCCESS != DeleteKeyTree(hkey1, pSubKeyName, nKeyAccess))
            break;
    }

    // delete the key itself
    return REG_DELETE_KEY_A( hkey, pPath, nKeyAccess & ( KEY_WOW64_64KEY | KEY_WOW64_32KEY ) );
}

static HRESULT DllUnregisterServerNative_Impl( int nMode, BOOL bForAllUsers, REGSAM nKeyAccess )
{
    char aSubKey[513];
    const char*    aPrefix = aLocalPrefix; // bForAllUsers ? "" : aLocalPrefix;

    Status s(false); // no throw
    for( int ind = 0; ind < SUPPORTED_EXT_NUM; ind++ )
    {
        if( nForModes[ind] & nMode )
        {
            DWORD nSubKeys = 0, nValues = 0;
            wsprintfA(aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix, aMimeType[ind]);
            Status s1(false); // no throw
            {
                HRegKey hkey;
                if (s1.upd(RegCreateKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                           aSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess,
                                           nullptr, &hkey, nullptr)))
                {
                    s1.upd(RegDeleteValueA(hkey, "CLSID"));
                    s1.upd(RegQueryInfoKeyA(hkey, nullptr, nullptr, nullptr, &nSubKeys, nullptr,
                                            nullptr, &nValues, nullptr, nullptr, nullptr, nullptr));
                }
            }
            if (s1 && !nSubKeys && !nValues)
                DeleteKeyTree(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey,
                              nKeyAccess);
            s.upd(s1.get());

            wsprintfA(aSubKey, "%s%s", aPrefix, aFileExt[ind]);
            Status s2(false); // no throw
            {
                HRegKey hkey;
                if (s2.upd(RegCreateKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                           aSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess,
                                           nullptr, &hkey, nullptr)))
                {
                    s2.upd(RegQueryInfoKeyA(hkey, nullptr, nullptr, nullptr, &nSubKeys, nullptr,
                                            nullptr, &nValues, nullptr, nullptr, nullptr, nullptr));
                }
            }
            if (s2 && !nSubKeys && !nValues)
                DeleteKeyTree(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey,
                              nKeyAccess);
            s.upd(s2.get());
        }
    }

    wsprintfA(aSubKey, "%sCLSID\\%s", aPrefix, aClassID);
    s.upd(DeleteKeyTree(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey,
                        nKeyAccess));

    wsprintfA(aSubKey, "%sso_activex.SOActiveX", aPrefix);
    s.upd(DeleteKeyTree(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey,
                        nKeyAccess));

    wsprintfA(aSubKey, "%sso_activex.SOActiveX.1", aPrefix);
    s.upd(DeleteKeyTree(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey,
                        nKeyAccess));

    wsprintfA(aSubKey, "%s\\TypeLib\\%s", aPrefix, aTypeLib);
    s.upd(DeleteKeyTree(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey,
                        nKeyAccess));

    wsprintfA(aSubKey, "%s\\Interface\\%s", aPrefix, aInterIDWinPeer);
    s.upd(DeleteKeyTree(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey,
                        nKeyAccess));

    wsprintfA(aSubKey, "%s\\Interface\\%s", aPrefix, aInterIDDispInt);
    s.upd(DeleteKeyTree(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey,
                        nKeyAccess));

    wsprintfA(aSubKey, "%s\\Interface\\%s", aPrefix, aInterIDActApprove);
    s.upd(DeleteKeyTree(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey,
                        nKeyAccess));

    return HRESULT_FROM_WIN32(s.get());
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
const char* const aMSFileExt[] = { ".dot", ".doc", ".xlt", ".xls", ".pot", ".ppt", ".pps" };
const char* const aMSMimeType[] = { "application/msword",
                          "application/msword",
                          "application/vnd.ms-excel",
                          "application/vnd.ms-excel",
                          "application/vnd.ms-powerpoint",
                          "application/vnd.ms-powerpoint",
                          "application/vnd.ms-powerpoint" };
const int nForMSModes[] = { 1, 1, 2, 2, 4, 4, 4 };

EXTERN_C __declspec(dllexport) HRESULT STDAPICALLTYPE DllUnregisterServerDoc( int nMode, BOOL bForAllUsers, BOOL bFor64Bit );
static HRESULT DllRegisterServerDoc_Impl( int nMode, BOOL bForAllUsers, REGSAM nKeyAccess )
{
    char aSubKey[513];
    int         ind;
    const char*    aPrefix = aLocalPrefix; // bForAllUsers ? "" : aLocalPrefix;

    // In case SO7 is installed for this user he can have local registry entries that will prevent him from
    // using SO8 ActiveX control. The fix is just to clean up the local entries related to ActiveX control.
    // Unfortunately it can be done only for the user who installs the office.
    if ( bForAllUsers )
        DllUnregisterServerDoc( nMode, false, false );

    Status s(true); // throw
    try
    {
        for (ind = 0; ind < SUPPORTED_MSEXT_NUM; ind++)
        {
            if (nForMSModes[ind] & nMode)
            {
                {
                    wsprintfA(aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix,
                              aMSMimeType[ind]);
                    HRegKey hkey;
                    s.upd(RegCreateKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                          aSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess,
                                          nullptr, &hkey, nullptr));
                    s.upd(RegSetValueExA(hkey, "Extension", 0, REG_SZ,
                                         reinterpret_cast<const BYTE*>(aMSFileExt[ind]),
                                         sal::static_int_cast<DWORD>(strlen(aMSFileExt[ind]))));
                    s.upd(RegSetValueExA(hkey, "CLSID", 0, REG_SZ,
                                         reinterpret_cast<const BYTE*>(aClassID),
                                         sal::static_int_cast<DWORD>(strlen(aClassID))));
                }
                {
                    wsprintfA(aSubKey, "%s%s", aPrefix, aMSFileExt[ind]);
                    HRegKey hkey;
                    s.upd(RegCreateKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                          aSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess,
                                          nullptr, &hkey, nullptr));
                    s.upd(RegSetValueExA(hkey, "Content Type", 0, REG_SZ,
                                         reinterpret_cast<const BYTE*>(aMSMimeType[ind]),
                                         sal::static_int_cast<DWORD>(strlen(aMSMimeType[ind]))));
                }
            }
        }

        wsprintfA(aSubKey, "%sCLSID\\%s", aPrefix, aClassID);
        HRegKey hkey;
        s.upd(RegCreateKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, 0,
                              nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess, nullptr, &hkey,
                              nullptr));
        s.upd(createKey(hkey, "EnableFullPage", nKeyAccess));
        for (ind = 0; ind < SUPPORTED_MSEXT_NUM; ind++)
        {
            if (nForMSModes[ind] & nMode)
            {
                wsprintfA(aSubKey, "EnableFullPage\\%s", aMSFileExt[ind]);
                HRegKey hkey1;
                s.upd(RegCreateKeyExA(hkey, aSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE,
                                      nKeyAccess, nullptr, &hkey1, nullptr));
            }
        }
    }
    catch (const std::exception&) {}

    return HRESULT_FROM_WIN32(s.get());
}

EXTERN_C __declspec(dllexport) HRESULT STDAPICALLTYPE DllRegisterServerDoc( int nMode, BOOL bForAllUsers, BOOL bFor64Bit )
{
    HRESULT hr = S_OK;
    if ( bFor64Bit )
        hr = DllRegisterServerDoc_Impl( nMode, bForAllUsers, n64KeyAccess );

    if ( SUCCEEDED( hr ) )
        hr = DllRegisterServerDoc_Impl( nMode, bForAllUsers, n32KeyAccess );

    return hr;
}


// DllUnregisterServerDoc - Removes entries from the system registry

static HRESULT DllUnregisterServerDoc_Impl( int nMode, BOOL bForAllUsers, REGSAM nKeyAccess )
{
    char aSubKey[513];
    const char*    aPrefix = aLocalPrefix; // bForAllUsers ? "" : aLocalPrefix;

    Status s(false); // no throw
    for (int ind = 0; ind < SUPPORTED_MSEXT_NUM; ind++)
    {
        if (nForMSModes[ind] & nMode)
        {
            DWORD nSubKeys = 0, nValues = 0;
            Status s1(false); // no throw
            {
                wsprintfA(aSubKey, "%sMIME\\DataBase\\Content Type\\%s", aPrefix, aMSMimeType[ind]);
                HRegKey hkey;
                if (s1.upd(RegCreateKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                           aSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess,
                                           nullptr, &hkey, nullptr)))
                {
                    s.upd(RegDeleteValueA(hkey, "Extension"));
                    s.upd(RegDeleteValueA(hkey, "CLSID"));
                    s1.upd(RegQueryInfoKeyA(hkey, nullptr, nullptr, nullptr, &nSubKeys, nullptr,
                                            nullptr, &nValues, nullptr, nullptr, nullptr, nullptr));
                }
            }
            if (s1 && !nSubKeys && !nValues)
                DeleteKeyTree(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey,
                              nKeyAccess);
            s.upd(s1.get());

            Status s2(false); // no throw
            {
                wsprintfA(aSubKey, "%s%s", aPrefix, aMSFileExt[ind]);
                HRegKey hkey;
                if (s2.upd(RegCreateKeyExA(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                           aSubKey, 0, nullptr, REG_OPTION_NON_VOLATILE, nKeyAccess,
                                           nullptr, &hkey, nullptr)))
                {
                    s.upd(RegDeleteValueA(hkey, "Content Type"));
                    s2.upd(RegQueryInfoKeyA(hkey, nullptr, nullptr, nullptr, &nSubKeys, nullptr,
                                            nullptr, &nValues, nullptr, nullptr, nullptr, nullptr));
                }
            }
            if (s2 && !nSubKeys && !nValues)
                DeleteKeyTree(bForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey,
                              nKeyAccess);
            s.upd(s2.get());
        }
    }

    return HRESULT_FROM_WIN32(s.get());
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

    HMODULE aCurModule = GetModuleHandleW( bX64 ? X64_LIB_NAME : X32_LIB_NAME );
    DWORD nLibNameLen = sal::static_int_cast<DWORD>(
            wcslen(bX64 ? X64_LIB_NAME : X32_LIB_NAME));

    if( aCurModule )
    {
        wchar_t pProgramPath[1024];
        DWORD nLen = GetModuleFileNameW( aCurModule, pProgramPath, 1024 );
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
