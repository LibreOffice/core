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

#include "config.hxx"
#include "global.hxx"
#include "shlxthdl.hxx"
#include "classfactory.hxx"
#include "registry.hxx"
#include "fileextensions.hxx"
#include "utilities.hxx"

#include <string>
#include <shlobj.h>


// Module global

long g_DllRefCnt = 0;
HINSTANCE g_hModule = nullptr;

namespace /* private */
{
    const wchar_t* const GUID_PLACEHOLDER       = L"{GUID}";
    const wchar_t* const EXTENSION_PLACEHOLDER  = L"{EXT}";
    const wchar_t* const FORWARDKEY_PLACEHOLDER = L"{FWDKEY}";

    const wchar_t* const CLSID_ENTRY                         = L"CLSID\\{GUID}\\InProcServer32";
    const wchar_t* const SHELLEX_IID_ENTRY                   = L"{EXT}\\shellex\\{GUID}";
    const wchar_t* const SHELLEX_ENTRY                       = L"{EXT}\\shellex";
    const wchar_t* const FORWARD_PROPSHEET_MYPROPSHEET_ENTRY = L"{FWDKEY}\\shellex\\PropertySheetHandlers\\MyPropSheet1";
    const wchar_t* const FORWARD_PROPSHEET_ENTRY             = L"{FWDKEY}\\shellex\\PropertySheetHandlers";
    const wchar_t* const FORWARD_SHELLEX_ENTRY               = L"{FWDKEY}\\shellex";

    const wchar_t* const SHELL_EXTENSION_APPROVED_KEY_NAME   = L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved";


    // "String Placeholder" ->
    // "String Replacement"

    void SubstitutePlaceholder(std::wstring& String, const std::wstring& Placeholder, const std::wstring& Replacement)
    {
        std::wstring::size_type idx = String.find(Placeholder);
        std::wstring::size_type len = Placeholder.length();

        while (std::wstring::npos != idx)
        {
            String.replace(idx, len, Replacement);
            idx = String.find(Placeholder);
        }
    }

    /* Make the registry entry
       HKCR\CLSID\{GUID}
            InProcServer32 = Path\shlxthdl.dll
                ThreadingModel = Apartment
    */
    HRESULT RegisterComComponent(const wchar_t* FilePath, const CLSID& Guid)
    {
        std::wstring ClsidEntry = CLSID_ENTRY;
        SubstitutePlaceholder(ClsidEntry, GUID_PLACEHOLDER, ClsidToString(Guid));

        if (!SetRegistryKey(HKEY_CLASSES_ROOT, ClsidEntry.c_str(), L"", FilePath))
            return E_FAIL;

        if (!SetRegistryKey(HKEY_CLASSES_ROOT, ClsidEntry.c_str(), L"ThreadingModel", L"Apartment"))
            return E_FAIL;

        return S_OK;
    }

    HRESULT UnregisterComComponent(const CLSID& Guid)
    {
        std::wstring tmp = L"CLSID\\";
        tmp += ClsidToString(Guid);
        return DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str()) ? S_OK : E_FAIL;
    }

    HRESULT RegisterColumnHandler(const wchar_t* ModuleFileName)
    {
        if (FAILED(RegisterComComponent(ModuleFileName, CLSID_COLUMN_HANDLER)))
            return E_FAIL;

        std::wstring tmp = L"Folder\\shellex\\ColumnHandlers\\";
        tmp += ClsidToString(CLSID_COLUMN_HANDLER);

        return SetRegistryKey(
            HKEY_CLASSES_ROOT,
            tmp.c_str(),
            L"",
            COLUMN_HANDLER_DESCRIPTIVE_NAME) ? S_OK : E_FAIL;
    }

    HRESULT UnregisterColumnHandler()
    {
        std::wstring tmp = L"Folder\\shellex\\ColumnHandlers\\";
        tmp += ClsidToString(CLSID_COLUMN_HANDLER);

        if (!DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str()))
            return E_FAIL;

        return UnregisterComComponent(CLSID_COLUMN_HANDLER);
    }

    HRESULT RegisterInfotipHandler(const wchar_t* ModuleFileName)
    {
        if (FAILED(RegisterComComponent(ModuleFileName, CLSID_INFOTIP_HANDLER)))
            return E_FAIL;

        std::wstring iid = ClsidToString(IID_IQueryInfo);
        std::wstring tmp;

        for(size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            tmp = SHELLEX_IID_ENTRY;
            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionU);
            SubstitutePlaceholder(tmp, GUID_PLACEHOLDER, iid);

            if (!SetRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str(), L"", ClsidToString(CLSID_INFOTIP_HANDLER).c_str()))
                return E_FAIL;
        }
        return S_OK;
    }

    HRESULT UnregisterInfotipHandler()
    {
        std::wstring iid = ClsidToString(IID_IQueryInfo);
        std::wstring tmp;

        for (size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            tmp = SHELLEX_IID_ENTRY;

            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionU);
            SubstitutePlaceholder(tmp, GUID_PLACEHOLDER, iid);

            DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str());

            // if there are no further subkey below .ext\\shellex
            // delete the whole subkey
            tmp = SHELLEX_ENTRY;
            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionU);

            bool HasSubKeys = true;
            if (HasSubkeysRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str(), HasSubKeys) && !HasSubKeys)
                DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str());
        }
        return UnregisterComComponent(CLSID_INFOTIP_HANDLER);
    }

    HRESULT RegisterPropSheetHandler(const wchar_t* ModuleFileName)
    {
        std::wstring FwdKeyEntry;

        if (FAILED(RegisterComComponent(ModuleFileName, CLSID_PROPERTYSHEET_HANDLER)))
            return E_FAIL;

        for (size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            FwdKeyEntry = FORWARD_PROPSHEET_MYPROPSHEET_ENTRY;
            SubstitutePlaceholder(FwdKeyEntry, FORWARDKEY_PLACEHOLDER, OOFileExtensionTable[i].RegistryForwardKey);

            if (!SetRegistryKey(HKEY_CLASSES_ROOT, FwdKeyEntry.c_str(), L"", ClsidToString(CLSID_PROPERTYSHEET_HANDLER).c_str()))
                return E_FAIL;
        }
        return S_OK;
    }

    HRESULT UnregisterPropSheetHandler()
    {
        std::wstring FwdKeyEntry;

        for (size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            FwdKeyEntry = FORWARD_PROPSHEET_MYPROPSHEET_ENTRY;
            SubstitutePlaceholder(FwdKeyEntry, FORWARDKEY_PLACEHOLDER, OOFileExtensionTable[i].RegistryForwardKey);

            DeleteRegistryKey(HKEY_CLASSES_ROOT, FwdKeyEntry.c_str());

            FwdKeyEntry = FORWARD_PROPSHEET_ENTRY;
            SubstitutePlaceholder(FwdKeyEntry, FORWARDKEY_PLACEHOLDER, OOFileExtensionTable[i].RegistryForwardKey);

            bool HasSubKeys = true;
            if (HasSubkeysRegistryKey(HKEY_CLASSES_ROOT, FwdKeyEntry.c_str(), HasSubKeys) && !HasSubKeys)
                DeleteRegistryKey(HKEY_CLASSES_ROOT, FwdKeyEntry.c_str());

            FwdKeyEntry = FORWARD_SHELLEX_ENTRY;
            SubstitutePlaceholder(FwdKeyEntry, FORWARDKEY_PLACEHOLDER, OOFileExtensionTable[i].RegistryForwardKey);

            HasSubKeys = true;
            if (HasSubkeysRegistryKey(HKEY_CLASSES_ROOT, FwdKeyEntry.c_str(), HasSubKeys) && !HasSubKeys)
                DeleteRegistryKey(HKEY_CLASSES_ROOT, FwdKeyEntry.c_str());
        }

        return UnregisterComComponent(CLSID_PROPERTYSHEET_HANDLER);
    }

    HRESULT RegisterThumbviewerHandler(const wchar_t* ModuleFileName)
    {
        if (FAILED(RegisterComComponent(ModuleFileName, CLSID_THUMBVIEWER_HANDLER)))
            return E_FAIL;

        std::wstring iid = ClsidToString(IID_IExtractImage);
        std::wstring tmp;

        for(size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            tmp = SHELLEX_IID_ENTRY;

            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionU);
            SubstitutePlaceholder(tmp, GUID_PLACEHOLDER, iid);

            if (!SetRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str(), L"", ClsidToString(CLSID_THUMBVIEWER_HANDLER).c_str()))
                return E_FAIL;
        }
        return S_OK;
    }

    HRESULT UnregisterThumbviewerHandler()
    {
        std::wstring iid = ClsidToString(IID_IExtractImage);
        std::wstring tmp;

        for (size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            tmp = SHELLEX_IID_ENTRY;

            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionU);
            SubstitutePlaceholder(tmp, GUID_PLACEHOLDER, iid);

            DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str());

            // if there are no further subkey below .ext\\shellex
            // delete the whole subkey
            tmp = SHELLEX_ENTRY;
            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionU);

            bool HasSubKeys = true;
            if (HasSubkeysRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str(), HasSubKeys) && !HasSubKeys)
                DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str());
        }
        return UnregisterComComponent(CLSID_THUMBVIEWER_HANDLER);
    }

    /** Approving/Unapproving the Shell Extension, it's important under Windows
        NT/2000/XP, see MSDN: Creating Shell Extension Handlers */
    HRESULT ApproveShellExtension(const CLSID& clsid, const std::wstring& Description)
    {
        bool bRet = SetRegistryKey(
            HKEY_LOCAL_MACHINE,
            SHELL_EXTENSION_APPROVED_KEY_NAME,
            ClsidToString(clsid).c_str(),
            Description.c_str());

        return bRet ? S_OK : E_FAIL;
    }

    HRESULT UnapproveShellExtension(const CLSID& Clsid)
    {
        HKEY hkey;

        LONG rc = RegOpenKeyW(
            HKEY_LOCAL_MACHINE,
            SHELL_EXTENSION_APPROVED_KEY_NAME,
            &hkey);

        if (ERROR_SUCCESS == rc)
        {
            rc = RegDeleteValueW(
                hkey,
                ClsidToString(Clsid).c_str());

            rc |= RegCloseKey(hkey);
        }

        return rc == ERROR_SUCCESS ? S_OK : E_FAIL;
    }

} // namespace /* private */


// COM exports


STDAPI DllRegisterServer()
{
    WCHAR ModuleFileName[MAX_PATH];

    GetModuleFileNameW(
        GetModuleHandleW(MODULE_NAME),
        ModuleFileName,
        sizeof(ModuleFileName)/sizeof(ModuleFileName[0]));

    HRESULT hr = S_OK;

    if (SUCCEEDED(RegisterColumnHandler(ModuleFileName)))
        ApproveShellExtension(CLSID_COLUMN_HANDLER, COLUMN_HANDLER_DESCRIPTIVE_NAME);
    else
        hr = E_FAIL;

    if (SUCCEEDED(RegisterInfotipHandler(ModuleFileName)))
        ApproveShellExtension(CLSID_INFOTIP_HANDLER, INFOTIP_HANDLER_DESCRIPTIVE_NAME);
    else
        hr = E_FAIL;

    if (SUCCEEDED(RegisterPropSheetHandler(ModuleFileName)))
        ApproveShellExtension(CLSID_PROPERTYSHEET_HANDLER, PROPSHEET_HANDLER_DESCRIPTIVE_NAME);
    else
        hr = E_FAIL;

    if (SUCCEEDED(RegisterThumbviewerHandler(ModuleFileName)))
        ApproveShellExtension(CLSID_THUMBVIEWER_HANDLER, THUMBVIEWER_HANDLER_DESCRIPTIVE_NAME);
    else
        hr = E_FAIL;

    // notify the Shell that something has changed
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    return hr;
}

STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;

    if (FAILED(UnregisterColumnHandler()))
        hr = E_FAIL;

    UnapproveShellExtension(CLSID_COLUMN_HANDLER);

    if (FAILED(UnregisterInfotipHandler()))
        hr = E_FAIL;

    UnapproveShellExtension(CLSID_INFOTIP_HANDLER);

    if (FAILED(UnregisterPropSheetHandler()))
        hr = E_FAIL;

    UnapproveShellExtension(CLSID_PROPERTYSHEET_HANDLER);

    if (FAILED(UnregisterThumbviewerHandler()))
        hr = E_FAIL;

    UnapproveShellExtension(CLSID_THUMBVIEWER_HANDLER);

    // notify the Shell that something has changed
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    return hr;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    *ppv = nullptr;

    if ((rclsid != CLSID_INFOTIP_HANDLER) &&
        (rclsid != CLSID_COLUMN_HANDLER) &&
        (rclsid != CLSID_PROPERTYSHEET_HANDLER) &&
        (rclsid != CLSID_THUMBVIEWER_HANDLER))
        return CLASS_E_CLASSNOTAVAILABLE;

    if ((riid != IID_IUnknown) && (riid != IID_IClassFactory))
        return E_NOINTERFACE;

    if ( rclsid == CLSID_INFOTIP_HANDLER )
        OutputDebugStringFormatW( L"DllGetClassObject: Create CLSID_INFOTIP_HANDLER\n" );
    else if ( rclsid == CLSID_COLUMN_HANDLER )
        OutputDebugStringFormatW( L"DllGetClassObject: Create CLSID_COLUMN_HANDLER\n" );
    else if ( rclsid == CLSID_PROPERTYSHEET_HANDLER )
        OutputDebugStringFormatW( L"DllGetClassObject: Create CLSID_PROPERTYSHEET_HANDLER\n" );
    else if ( rclsid == CLSID_THUMBVIEWER_HANDLER )
        OutputDebugStringFormatW( L"DllGetClassObject: Create CLSID_THUMBVIEWER_HANDLER\n" );

    IUnknown* pUnk = new CClassFactory(rclsid);
    *ppv = pUnk;
    return S_OK;
}

STDAPI DllCanUnloadNow()
{
    if (CClassFactory::IsLocked() || g_DllRefCnt > 0)
        return S_FALSE;

    return S_OK;
}

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
{
    g_hModule = hInst;
    return TRUE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
