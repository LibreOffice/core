/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Shobjidl.h>
#include <string>
#include <vcl/weld.hxx>
#include <Lmcons.h>
#include <Lm.h>
#include <strings.hrc>
#include <svdata.hxx>
#include <versionhelpers.h>

#include <vcl/fileextcheck.hxx>

namespace vcl::fileextcheck
{
inline bool IsWindowsBuildOrLater(uint32_t aBuild)
{
    static uint32_t minBuild = 0;
    static uint32_t maxBuild = UINT32_MAX;

    if (minBuild >= aBuild)
    {
        return true;
    }

    if (aBuild >= maxBuild)
    {
        return false;
    }

    OSVERSIONINFOEXW info;
    ZeroMemory(&info, sizeof(OSVERSIONINFOEXW));
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    info.dwBuildNumber = aBuild;

    DWORDLONG conditionMask = 0;
    VER_SET_CONDITION(conditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

    if (VerifyVersionInfoW(&info, VER_BUILDNUMBER, conditionMask))
    {
        minBuild = aBuild;
        return true;
    }

    maxBuild = aBuild;
    return false;
}

static bool IsWindowsLogonConnected()
{
    WCHAR userName[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (!GetUserNameW(userName, &size))
    {
        return false;
    }

    LPUSER_INFO_24 info;
    if (NetUserGetInfo(nullptr, userName, 24, (LPBYTE*)&info) != NERR_Success)
    {
        return false;
    }
    bool connected = info->usri24_internet_identity;
    NetApiBufferFree(info);

    return connected;
}

static bool SettingsAppBelievesConnected()
{
    const wchar_t* keyPath = L"SOFTWARE\\Microsoft\\Windows\\Shell\\Associations";
    const wchar_t* valueName = L"IsConnectedAtLogon";

    uint32_t value = 0;
    DWORD size = sizeof(uint32_t);
    LSTATUS ls
        = RegGetValueW(HKEY_CURRENT_USER, keyPath, valueName, RRF_RT_ANY, nullptr, &value, &size);
    if (ls != ERROR_SUCCESS)
    {
        return false;
    }

    return !!value;
}

void LaunchModernSettingsDialogDefaultApps()
{
    if (!IsWindowsBuildOrLater(14965) && !IsWindowsLogonConnected()
        && SettingsAppBelievesConnected())
    {
        // TODO: Use the classic Control Panel to work around a bug of older
        // builds of Windows 10.
        return; // TODO: LaunchControlPanelDefaultPrograms();
    }
#if (NTDDI_VERSION >= NTDDI_WIN8)
    IApplicationActivationManager* pActivator;
    HRESULT hr = ::CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_INPROC,
                                    IID_IApplicationActivationManager, (void**)&pActivator);

    if (SUCCEEDED(hr))
    {
        DWORD pid;
        hr = pActivator->ActivateApplication(L"windows.immersivecontrolpanel_cw5n1h2txyewy"
                                             L"!microsoft.windows.immersivecontrolpanel",
                                             L"page=SettingsPageAppsDefaults", AO_NONE, &pid);
        if (SUCCEEDED(hr))
        {
            // Do not check error because we could at least open
            // the "Default apps" setting.
            pActivator->ActivateApplication(L"windows.immersivecontrolpanel_cw5n1h2txyewy"
                                            L"!microsoft.windows.immersivecontrolpanel",
                                            L"page=SettingsPageAppsDefaults"
                                            L"&target=SettingsPageAppsDefaultsDefaultAppsListView",
                                            AO_NONE, &pid);
        }
        pActivator->Release();
    }
#endif
}

static HRESULT IsPathDefaultForClass(IApplicationAssociationRegistration* pAAR, LPCWSTR aClassName,
                                     LPCWSTR progID)
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

void CheckFileExtRegistration()
{
    if (!IsWindows10OrGreater())
        return;

    if (!IsDefaultAppInstalledInReg())
        return;

    CoInitializeEx((LPVOID)NULL, COINIT_APARTMENTTHREADED);

    IApplicationAssociationRegistration* pAAR;
    HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration, nullptr, CLSCTX_INPROC,
                                  IID_IApplicationAssociationRegistration, (LPVOID*)&pAAR);
    if (FAILED(hr))
    {
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
        OUString aMsg(VclResId(STR_FILEEXT_NONDEFAULT_ASK));
        aMsg = aMsg.replaceFirst("$1", aNonDefaults);

        std::unique_ptr<weld::MessageDialog> xFileExtAskBox(Application::CreateMessageDialog(
            nullptr, VclMessageType::Question, VclButtonsType::NONE, aMsg));
        xFileExtAskBox->add_button(VclResId(STR_ANSWER_OK), RET_YES);
        xFileExtAskBox->add_button(VclResId(STR_ANSWER_CANCEL), RET_CANCEL);
        int ret = xFileExtAskBox->run();

        if (ret == RET_YES)
            LaunchModernSettingsDialogDefaultApps();
    }

    if (pAAR)
        pAAR->Release();
    CoFreeUnusedLibraries();
    CoUninitialize();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
