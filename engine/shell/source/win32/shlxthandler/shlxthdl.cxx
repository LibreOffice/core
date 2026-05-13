/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <config.hxx>
#include <global.hxx>
#include <shlxthdl.hxx>
#include "classfactory.hxx"
#include <registry.hxx>
#include <fileextensions.hxx>
#include <utilities.hxx>

#include <string>
#include <shlobj.h>

#include <olectl.h> // declarations of DllRegisterServer/DllUnregisterServer

#include <systools/win32/extended_max_path.hxx>

// Module global

LONG g_DllRefCnt = 0;
HINSTANCE g_hModule = nullptr;

namespace /* private */
{
    const wchar_t* const GUID_PLACEHOLDER       = L"{GUID}";
    const wchar_t* const EXTENSION_PLACEHOLDER  = L"{EXT}";

    const wchar_t* const CLSID_ENTRY                         = L"CLSID\\{GUID}\\InProcServer32";
    const wchar_t* const SHELLEX_IID_ENTRY                   = L"{EXT}\\shellex\\{GUID}";
    const wchar_t* const SHELLEX_ENTRY                       = L"{EXT}\\shellex";

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
        return DeleteRegistryTree(HKEY_CLASSES_ROOT, tmp.c_str()) ? S_OK : E_FAIL;
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

            DeleteRegistryTree(HKEY_CLASSES_ROOT, tmp.c_str());

            // if there are no further subkey below .ext\\shellex
            // delete the whole subkey
            tmp = SHELLEX_ENTRY;
            SubstitutePlaceholder(tmp, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionU);

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
        LSTATUS rc = RegDeleteKeyValueW(HKEY_LOCAL_MACHINE, SHELL_EXTENSION_APPROVED_KEY_NAME,
                                        ClsidToString(Clsid).c_str());
        return rc == ERROR_SUCCESS ? S_OK : E_FAIL;
    }

} // namespace /* private */


// COM exports


STDAPI DllRegisterServer()
{
    WCHAR ModuleFileName[EXTENDED_MAX_PATH];

    GetModuleFileNameW(
        GetCurrentModuleHandle(),
        ModuleFileName,
        std::size(ModuleFileName));

    HRESULT hr = S_OK;

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

    if (FAILED(UnregisterThumbviewerHandler()))
        hr = E_FAIL;

    UnapproveShellExtension(CLSID_THUMBVIEWER_HANDLER);

    // notify the Shell that something has changed
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    return hr;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    *ppv = nullptr;

    if (rclsid != CLSID_THUMBVIEWER_HANDLER)
        return CLASS_E_CLASSNOTAVAILABLE;

    if ((riid != IID_IUnknown) && (riid != IID_IClassFactory))
        return E_NOINTERFACE;

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
