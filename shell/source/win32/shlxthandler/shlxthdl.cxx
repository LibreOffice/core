/*************************************************************************
 *
 *  $RCSfile: shlxthdl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 11:16:15 $
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

#ifndef _CONFIG_HXX_
#include "config.hxx"
#endif

#ifndef _GLOBAL_HXX_
#include "global.hxx"
#endif

#ifndef _SHLXTHDL_HXX_
#include "shlxthdl.hxx"
#endif

#ifndef _CLASSFACTORY_HXX_
#include "classfactory.hxx"
#endif

#ifndef _REGISTRY_HXX_
#include "registry.hxx"
#endif

#ifndef _FILEEXTENSIONS_HXX_
#include "fileextensions.hxx"
#endif

#ifndef _UTILITIES_HXX_
#include "utilities.hxx"
#endif

#include <tchar.h>
#include <string>
#include <shlobj.h>

//---------------------------
// Module global
//---------------------------

long g_DllRefCnt = 0;

//---------------------------
//
//---------------------------

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

    //----------------------------------------------
    // Make the registry entry
    // HKCR\CLSID\{GUID}
    //      InProcServer32     = Path\shlxthdl.dll
    //          ThreadingModel = Apartment
    //----------------------------------------------

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

    //---------------------------
    //
    //---------------------------

    HRESULT UnregisterComComponent(const CLSID& Guid)
    {
        std::string tmp = "CLSID\\";
        tmp += ClsidToString(Guid);
        return DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str()) ? S_OK : E_FAIL;
    }

    //---------------------------
    //
    //---------------------------

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

    //---------------------------
    //
    //---------------------------

    HRESULT UnregisterColumnHandler()
    {
        std::string tmp = "Folder\\shellex\\ColumnHandlers\\";
        tmp += ClsidToString(CLSID_COLUMN_HANDLER);

        if (!DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str()))
            return E_FAIL;

        return UnregisterComComponent(CLSID_COLUMN_HANDLER);
    }

    //---------------------------
    //
    //---------------------------

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

    //---------------------------
    //
    //---------------------------

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

    //---------------------------
    //
    //---------------------------

    HRESULT RegisterPropSheetHandler(const char* ModuleFileName)
    {
        std::string ExtEntry;
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

    //---------------------------
    //
    //---------------------------

    HRESULT UnregisterPropSheetHandler()
    {
        std::string ExtEntry;
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

    //-----------------------------------
    /** Approve the Shell Extension, it's
        important for Windows NT/2000/XP
        See MSDN: Creating Shell Extension
        Handlers
    */
    HRESULT ApproveShellExtension(CLSID clsid, const std::wstring& Description)
    {
        bool bRet = SetRegistryKey(
            HKEY_LOCAL_MACHINE,
            SHELL_EXTENSION_APPROVED_KEY_NAME,
            ClsidToString(clsid).c_str(),
            WStringToString(Description).c_str());

        return bRet ? S_OK : E_FAIL;
    }

    //------------------------------------
    /** Unapprove the Shell Extension, it's
        important under Windows NT/2000/XP
        See MSDN: Creating Shell Extension
        Handlers
    */
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


//---------------------------
// COM exports
//---------------------------


extern "C" STDAPI DllRegisterServer()
{
    TCHAR ModuleFileName[MAX_PATH];

    GetModuleFileName(
        GetModuleHandle(MODULE_NAME),
        ModuleFileName,
        sizeof(ModuleFileName));

    HRESULT hr = S_OK;

/*

// register column handler
#ifdef UNICODE
    if (FAILED(RegisterColumnHandler(WStringToString(ModuleFileName))))
        hr = E_FAIL;
#else
    if (FAILED(RegisterColumnHandler(ModuleFileName)))
        hr = E_FAIL;
#endif

    ApproveShellExtension(
        CLSID_COLUMN_HANDLER,
        COLUMN_HANDLER_DESCRIPTIVE_NAME);

// register info tip control
#ifdef UNICODE
    if (FAILED(RegisterInfotipHandler(WStringToString(ModuleFileName))))
        hr = E_FAIL;
#else
    if (FAILED(RegisterInfotipHandler(ModuleFileName)))
        hr = E_FAIL;
#endif

    ApproveShellExtension(
        CLSID_INFOTIP_HANDLER,
        INFOTIP_HANDLER_DESCRIPTIVE_NAME);

*/

// register property sheet handler
#ifdef UNICODE
    if (FAILED(RegisterPropSheetHandler(WStringToString(ModuleFileName).c_str())))
        hr = E_FAIL;
#else
    if (FAILED(RegisterPropSheetHandler(ModuleFileName)))
        hr = E_FAIL;
#endif

    ApproveShellExtension(
        CLSID_PROPERTYSHEET_HANDLER,
        PROPSHEET_HANDLER_DESCRIPTIVE_NAME);

    // notify the Shell that something has
    // changed
    SHChangeNotify(SHCNE_ASSOCCHANGED, 0, 0, 0);

    return hr;
}

//---------------------------
//
//---------------------------

extern "C" STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;

/*
    if (FAILED(UnregisterColumnHandler()))
        hr = E_FAIL;

    UnapproveShellExtension(CLSID_COLUMN_HANDLER);

    if (FAILED(UnregisterInfotipHandler()))
        hr = E_FAIL;

    UnapproveShellExtension(CLSID_INFOTIP_HANDLER);

*/

    if (FAILED(UnregisterPropSheetHandler()))
        hr = E_FAIL;

    UnapproveShellExtension(CLSID_PROPERTYSHEET_HANDLER);

    // notify the Shell that something has
    // changed
    SHChangeNotify(SHCNE_ASSOCCHANGED, 0, 0, 0);

    return hr;
}

//---------------------------
//
//---------------------------

extern "C" STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    *ppv = 0;

    if ((rclsid != CLSID_INFOTIP_HANDLER) && (rclsid != CLSID_COLUMN_HANDLER) && (rclsid != CLSID_PROPERTYSHEET_HANDLER))
        return CLASS_E_CLASSNOTAVAILABLE;

    if ((riid != IID_IUnknown) && (riid != IID_IClassFactory))
        return E_NOINTERFACE;

    IUnknown* pUnk = new CClassFactory(rclsid);
    if (0 == pUnk)
        return E_OUTOFMEMORY;

    *ppv = pUnk;

    return S_OK;
}

//---------------------------
//
//---------------------------

extern "C" STDAPI DllCanUnloadNow(void)
{
    if (CClassFactory::IsLocked() || g_DllRefCnt > 0)
        return S_FALSE;

    return S_OK;
}
