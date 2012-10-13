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

#include "internal/config.hxx"
#include "internal/global.hxx"
#include "internal/shlxthdl.hxx"
#include "classfactory.hxx"
#include "internal/registry.hxx"
#include "internal/fileextensions.hxx"
#include "internal/utilities.hxx"

#include <tchar.h>
#include <string>
#include <shlobj.h>

//---------------------------
// Module global
//---------------------------
long g_DllRefCnt = 0;
HINSTANCE g_hModule = NULL;

namespace /* private */
{
    const char* GUID_PLACEHOLDER       = "{GUID}";
    const char* EXTENSION_PLACEHOLDER  = "{EXT}";
    const char* FORWARDKEY_PLACEHOLDER = "{FWDKEY}";

    const char* CLSID_ENTRY                         = "CLSID\\{GUID}\\InProcServer32";
    const char* SHELLEX_IID_ENTRY                   = "{EXT}\\shellex\\{GUID}";
    const char* SHELLEX_ENTRY                       = "{EXT}\\shellex";
    const char* PROPSHEET_ENTRY                     = "{EXT}\\CLSID\\{GUID}\\InProcServer32";
    const char* EXTENSION_CLSID                     = "{EXT}\\CLSID";
    const char* EXTENSION_CLSID_GUID                = "{EXT}\\CLSID\\{GUID}";
    const char* FORWARD_PROPSHEET_MYPROPSHEET_ENTRY = "{FWDKEY}\\shellex\\PropertySheetHandlers\\MyPropSheet1";
    const char* FORWARD_PROPSHEET_ENTRY             = "{FWDKEY}\\shellex\\PropertySheetHandlers";
    const char* FORWARD_SHELLEX_ENTRY               = "{FWDKEY}\\shellex";

    const char* SHELL_EXTENSION_APPROVED_KEY_NAME   = "Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved";

    //---------------------------
    // "String Placeholder" ->
    // "String Replacement"
    //---------------------------
    void SubstitutePlaceholder(std::string& String, const std::string& Placeholder, const std::string& Replacement)
    {
        std::string::size_type idx = String.find(Placeholder);
        std::string::size_type len = Placeholder.length();

        while (std::string::npos != idx)
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
    HRESULT RegisterComComponent(const char* FilePath, const CLSID& Guid)
    {
        std::string ClsidEntry = CLSID_ENTRY;
        SubstitutePlaceholder(ClsidEntry, GUID_PLACEHOLDER, ClsidToString(Guid));

        if (!SetRegistryKey(HKEY_CLASSES_ROOT, ClsidEntry.c_str(), "", FilePath))
            return E_FAIL;

        if (!SetRegistryKey(HKEY_CLASSES_ROOT, ClsidEntry.c_str(), "ThreadingModel", "Apartment"))
            return E_FAIL;

        return S_OK;
    }

    HRESULT UnregisterComComponent(const CLSID& Guid)
    {
        std::string tmp = "CLSID\\";
        tmp += ClsidToString(Guid);
        return DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str()) ? S_OK : E_FAIL;
    }

    HRESULT RegisterColumnHandler(const char* ModuleFileName)
    {
        if (FAILED(RegisterComComponent(ModuleFileName, CLSID_COLUMN_HANDLER)))
            return E_FAIL;

        std::string tmp = "Folder\\shellex\\ColumnHandlers\\";
        tmp += ClsidToString(CLSID_COLUMN_HANDLER);

        return SetRegistryKey(
            HKEY_CLASSES_ROOT,
            tmp.c_str(),
            "",
            WStringToString(COLUMN_HANDLER_DESCRIPTIVE_NAME).c_str()) ? S_OK : E_FAIL;
    }

    HRESULT UnregisterColumnHandler()
    {
        std::string tmp = "Folder\\shellex\\ColumnHandlers\\";
        tmp += ClsidToString(CLSID_COLUMN_HANDLER);

        if (!DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str()))
            return E_FAIL;

        return UnregisterComComponent(CLSID_COLUMN_HANDLER);
    }

    HRESULT RegisterInfotipHandler(const char* ModuleFileName)
    {
        if (FAILED(RegisterComComponent(ModuleFileName, CLSID_INFOTIP_HANDLER)))
            return E_FAIL;

        std::string iid = ClsidToString(IID_IQueryInfo);
        std::string tmp;

        for(size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            tmp = SHELLEX_IID_ENTRY;
            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionAnsi);
            SubstitutePlaceholder(tmp, GUID_PLACEHOLDER, iid);

            if (!SetRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str(), "", ClsidToString(CLSID_INFOTIP_HANDLER).c_str()))
                return E_FAIL;
        }
        return S_OK;
    }

    HRESULT UnregisterInfotipHandler()
    {
        std::string iid = ClsidToString(IID_IQueryInfo);
        std::string tmp;

        for (size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            tmp = SHELLEX_IID_ENTRY;

            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionAnsi);
            SubstitutePlaceholder(tmp, GUID_PLACEHOLDER, iid);

            DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str());

            // if there are no further subkey below .ext\\shellex
            // delete the whole subkey
            tmp = SHELLEX_ENTRY;
            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionAnsi);

            bool HasSubKeys = true;
            if (HasSubkeysRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str(), HasSubKeys) && !HasSubKeys)
                DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str());
        }
        return UnregisterComComponent(CLSID_INFOTIP_HANDLER);
    }

    HRESULT RegisterPropSheetHandler(const char* ModuleFileName)
    {
        std::string FwdKeyEntry;

        if (FAILED(RegisterComComponent(ModuleFileName, CLSID_PROPERTYSHEET_HANDLER)))
            return E_FAIL;

        for (size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            FwdKeyEntry = FORWARD_PROPSHEET_MYPROPSHEET_ENTRY;
            SubstitutePlaceholder(FwdKeyEntry, FORWARDKEY_PLACEHOLDER, OOFileExtensionTable[i].RegistryForwardKey);

            if (!SetRegistryKey(HKEY_CLASSES_ROOT, FwdKeyEntry.c_str(), "", ClsidToString(CLSID_PROPERTYSHEET_HANDLER).c_str()))
                return E_FAIL;
        }
        return S_OK;
    }

    HRESULT UnregisterPropSheetHandler()
    {
        std::string FwdKeyEntry;

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

    HRESULT RegisterThumbviewerHandler(const char* ModuleFileName)
    {
        if (FAILED(RegisterComComponent(ModuleFileName, CLSID_THUMBVIEWER_HANDLER)))
            return E_FAIL;

        std::string iid = ClsidToString(IID_IExtractImage);
        std::string tmp;

        for(size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            tmp = SHELLEX_IID_ENTRY;

            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionAnsi);
            SubstitutePlaceholder(tmp, GUID_PLACEHOLDER, iid);

            if (!SetRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str(), "", ClsidToString(CLSID_THUMBVIEWER_HANDLER).c_str()))
                return E_FAIL;
        }
        return S_OK;
    }

    HRESULT UnregisterThumbviewerHandler()
    {
        std::string iid = ClsidToString(IID_IExtractImage);
        std::string tmp;

        for (size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            tmp = SHELLEX_IID_ENTRY;

            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionAnsi);
            SubstitutePlaceholder(tmp, GUID_PLACEHOLDER, iid);

            DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str());

            // if there are no further subkey below .ext\\shellex
            // delete the whole subkey
            tmp = SHELLEX_ENTRY;
            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionAnsi);

            bool HasSubKeys = true;
            if (HasSubkeysRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str(), HasSubKeys) && !HasSubKeys)
                DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str());
        }
        return UnregisterComComponent(CLSID_THUMBVIEWER_HANDLER);
    }

    /** Approving/Unapproving the Shell Extension, it's important under Windows
        NT/2000/XP, see MSDN: Creating Shell Extension Handlers */
    HRESULT ApproveShellExtension(CLSID clsid, const std::wstring& Description)
    {
        bool bRet = SetRegistryKey(
            HKEY_LOCAL_MACHINE,
            SHELL_EXTENSION_APPROVED_KEY_NAME,
            ClsidToString(clsid).c_str(),
            WStringToString(Description).c_str());

        return bRet ? S_OK : E_FAIL;
    }

    HRESULT UnapproveShellExtension(CLSID Clsid)
    {
        HKEY hkey;

        LONG rc = RegOpenKeyA(
            HKEY_LOCAL_MACHINE,
            SHELL_EXTENSION_APPROVED_KEY_NAME,
            &hkey);

        if (ERROR_SUCCESS == rc)
        {
            rc = RegDeleteValueA(
                hkey,
                ClsidToString(Clsid).c_str());

            rc = RegCloseKey(hkey);
        }

        return rc == ERROR_SUCCESS ? S_OK : E_FAIL;
    }

} // namespace /* private */


//---------------------
// COM exports
//---------------------

extern "C" STDAPI DllRegisterServer()
{
    TCHAR ModuleFileName[MAX_PATH];

    GetModuleFileName(
        GetModuleHandle(MODULE_NAME),
        ModuleFileName,
        sizeof(ModuleFileName));

    std::string module_path = WStringToString(ModuleFileName);
    HRESULT hr = S_OK;

    if (SUCCEEDED(RegisterColumnHandler(module_path.c_str())))
        ApproveShellExtension(CLSID_COLUMN_HANDLER, COLUMN_HANDLER_DESCRIPTIVE_NAME);
    else
        hr = E_FAIL;

    if (SUCCEEDED(RegisterInfotipHandler(module_path.c_str())))
        ApproveShellExtension(CLSID_INFOTIP_HANDLER, INFOTIP_HANDLER_DESCRIPTIVE_NAME);
    else
        hr = E_FAIL;

    if (SUCCEEDED(RegisterPropSheetHandler(module_path.c_str())))
        ApproveShellExtension(CLSID_PROPERTYSHEET_HANDLER, PROPSHEET_HANDLER_DESCRIPTIVE_NAME);
    else
        hr = E_FAIL;

    if (SUCCEEDED(RegisterThumbviewerHandler(module_path.c_str())))
        ApproveShellExtension(CLSID_THUMBVIEWER_HANDLER, THUMBVIEWER_HANDLER_DESCRIPTIVE_NAME);
    else
        hr = E_FAIL;

    // notify the Shell that something has changed
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);

    return hr;
}

extern "C" STDAPI DllUnregisterServer()
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
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);

    return hr;
}

extern "C" STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    *ppv = 0;

    if ((rclsid != CLSID_INFOTIP_HANDLER) &&
        (rclsid != CLSID_COLUMN_HANDLER) &&
        (rclsid != CLSID_PROPERTYSHEET_HANDLER) &&
        (rclsid != CLSID_THUMBVIEWER_HANDLER))
        return CLASS_E_CLASSNOTAVAILABLE;

    if ((riid != IID_IUnknown) && (riid != IID_IClassFactory))
        return E_NOINTERFACE;

    if ( rclsid == CLSID_INFOTIP_HANDLER )
        OutputDebugStringFormat( "DllGetClassObject: Create CLSID_INFOTIP_HANDLER\n" );
    else if ( rclsid == CLSID_COLUMN_HANDLER )
        OutputDebugStringFormat( "DllGetClassObject: Create CLSID_COLUMN_HANDLER\n" );
    else if ( rclsid == CLSID_PROPERTYSHEET_HANDLER )
        OutputDebugStringFormat( "DllGetClassObject: Create CLSID_PROPERTYSHEET_HANDLER\n" );
    else if ( rclsid == CLSID_THUMBVIEWER_HANDLER )
        OutputDebugStringFormat( "DllGetClassObject: Create CLSID_THUMBVIEWER_HANDLER\n" );

    IUnknown* pUnk = new CClassFactory(rclsid);
    *ppv = pUnk;
    return S_OK;
}

extern "C" STDAPI DllCanUnloadNow(void)
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
