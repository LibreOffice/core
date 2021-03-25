/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WIN8 // needed for IApplicationActivationManager
#include <o3tl/char16_t2wchar_t.hxx>
#include <Shobjidl.h>
#include <string>
#include <strings.hrc>
#include <versionhelpers.h>
#include <vcl/abstdlg.hxx>
#include <systools/win32/comtools.hxx>

#include <vcl/fileregistration.hxx>

namespace vcl::fileregistration
{
static void LaunchModernSettingsDialogDefaultApps()
{
    auto pIf = sal::systools::COMReference<IApplicationActivationManager>().CoCreateInstance(
        CLSID_ApplicationActivationManager, nullptr, CLSCTX_INPROC_SERVER);

    DWORD pid;
    HRESULT hr = pIf->ActivateApplication(L"windows.immersivecontrolpanel_cw5n1h2txyewy"
                                          L"!microsoft.windows.immersivecontrolpanel",
                                          L"page=SettingsPageAppsDefaults", AO_NONE, &pid);
    if (SUCCEEDED(hr))
    {
        // Do not check error because we could at least open
        // the "Default apps" setting.
        pIf->ActivateApplication(L"windows.immersivecontrolpanel_cw5n1h2txyewy"
                                 L"!microsoft.windows.immersivecontrolpanel",
                                 L"page=SettingsPageAppsDefaults"
                                 L"&target=SettingsPageAppsDefaultsDefaultAppsListView",
                                 AO_NONE, &pid);
    }
}

static HRESULT
IsPathDefaultForClass(sal::systools::COMReference<IApplicationAssociationRegistration>& pAAR,
                      LPCWSTR aClassName, LPCWSTR progID)
{
    // Make sure the Prog ID matches what we have
    LPWSTR registeredApp;
    HRESULT hr
        = pAAR->QueryCurrentDefault(aClassName, AT_FILEEXTENSION, AL_EFFECTIVE, &registeredApp);
    if (FAILED(hr))
    {
        return hr;
    }

    if (!wcsnicmp(registeredApp, progID, wcslen(progID)))
        hr = S_OK;
    else
        hr = S_FALSE;

    CoTaskMemFree(registeredApp);

    return hr;
}

#define MAX_VALUE_NAME 16383
static bool IsDefaultAppInstalledInReg()
{
    const wchar_t* keyPath = L"SOFTWARE\\LibreOffice\\UNO\\InstallPath";

    HKEY hTestKey;
    bool bFound = false;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ, &hTestKey) == ERROR_SUCCESS)
    {
        DWORD cValues; // number of values for key
        DWORD cbMaxValueData; // longest value data

        if (RegQueryInfoKey(hTestKey, // key handle
                            NULL, // buffer for class name
                            NULL, // size of class string
                            NULL, // reserved
                            NULL, // number of subkeys
                            NULL, // longest subkey size
                            NULL, // longest class string
                            &cValues, // number of values for this key
                            NULL, // longest value name
                            &cbMaxValueData, // longest value data
                            NULL, // security descriptor
                            NULL) // last write time
            == ERROR_SUCCESS)
        {
            DWORD retCode = ERROR_SUCCESS;
            void* pData = malloc(cbMaxValueData);
            if (pData != nullptr)
            {
                for (DWORD i = 0; i < cValues; i++)
                {
                    WCHAR achValue[MAX_VALUE_NAME];
                    DWORD cchValue = MAX_VALUE_NAME;
                    achValue[0] = L'\0';
                    DWORD cbData = cbMaxValueData;
                    DWORD type = REG_NONE;

                    retCode = RegEnumValueW(hTestKey, i, achValue, &cchValue, NULL, &type,
                                            (LPBYTE)pData, &cbData);
                    if (retCode == ERROR_SUCCESS && type == REG_SZ)
                    {
                        std::wstring val(achValue);
                        if (val.find(L"LibreOffice") != std::wstring::npos)
                        {
                            wchar_t* regPath = (wchar_t*)pData;
                            regPath[cbData] = L'\0';
                            WCHAR szProcPath[MAX_PATH];

                            if (GetModuleFileNameW(NULL, szProcPath, MAX_PATH))
                            {
                                std::wstring procPath(szProcPath);
                                if (procPath.find(regPath) != std::wstring::npos)
                                    bFound = true;
                            }
                            break;
                        }
                    }
                }
                free(pData);
            }
        }
    }

    RegCloseKey(hTestKey);

    return bFound;
}

void LaunchRegistrationUI()
{
    const bool bUninit = SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));

    try
    {
        if (IsWindows8OrGreater())
        {
            LaunchModernSettingsDialogDefaultApps();
        }
        else
        {
            auto pIf = sal::systools::COMReference<IApplicationAssociationRegistrationUI>()
                           .CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI, nullptr,
                                             CLSCTX_INPROC_SERVER);

            // LaunchAdvancedAssociationUI only works for applications registered under
            // Software\RegisteredApplications. See scp2/source/ooo/registryitem_ooo.scp
            const OUString expanded = Translate::ExpandVariables("%PRODUCTNAME %PRODUCTVERSION");
            pIf->LaunchAdvancedAssociationUI(o3tl::toW(expanded.getStr()));
        }
    }
    catch (...)
    {
        // Just ignore any error here: this is not something we need to make sure to succeed
    }

    if (bUninit)
        CoUninitialize();
}

void CheckFileExtRegistration()
{
    if (!officecfg::Office::Common::Misc::PerformFileExtCheck::get())
        return;

    if (!IsDefaultAppInstalledInReg())
        return;

    const bool bUninit = SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
    sal::systools::COMReference<IApplicationAssociationRegistration> pAAR = nullptr;
    try
    {
        pAAR = sal::systools::COMReference<IApplicationAssociationRegistration>().CoCreateInstance(
            CLSID_ApplicationAssociationRegistration, nullptr, CLSCTX_INPROC_SERVER);
    }
    catch (...)
    {
        // Just return on any error here: this is not something we need to make sure to succeed
        return;
    }

    std::map<std::wstring, std::wstring> formats = {
        { L".odp", L"LibreOffice.ImpressDocument.1" },
        { L".odt", L"LibreOffice.WriterDocument.1" },
        { L".ods", L"LibreOffice.CalcDocument.1" },
    };
    std::wstring nonDefaultExts;
    bool isNotDefault = false;

    for (std::map<std::wstring, std::wstring>::iterator it = formats.begin(); it != formats.end();
         it++)
    {
        if (IsPathDefaultForClass(pAAR, it->first.c_str(), it->second.c_str()) == S_FALSE)
        {
            isNotDefault = true;
            nonDefaultExts += it->first;
            nonDefaultExts += L"\n";
        }
    }

    if (isNotDefault)
    {
        OUString aNonDefaults(reinterpret_cast<const sal_Unicode*>(nonDefaultExts.c_str()));
        OUString aMsg(VclResId(STR_FILEEXT_NONDEFAULT_ASK_MSG));
        aMsg = aMsg.replaceFirst("$1", aNonDefaults);

        VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
        ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateFileExtCheckDialog(
            nullptr, VclResId(STR_FILEEXT_NONDEFAULT_ASK_TITLE), aMsg));
        pDlg->Execute();
    }

    if (bUninit)
        CoUninitialize();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
