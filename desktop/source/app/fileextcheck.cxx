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
#include <dp_shared.hxx>
#include "fileextcheck.hxx"

namespace desktop::fileextcheck
{
inline bool IsWindowsVersionOrLater(uint32_t aVersion)
{
    static uint32_t minVersion = 0;
    static uint32_t maxVersion = UINT32_MAX;

    if (minVersion >= aVersion)
    {
        return true;
    }

    if (aVersion >= maxVersion)
    {
        return false;
    }

    OSVERSIONINFOEXW info;
    ZeroMemory(&info, sizeof(OSVERSIONINFOEXW));
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    info.dwMajorVersion = aVersion >> 24;
    info.dwMinorVersion = (aVersion >> 16) & 0xFF;
    info.wServicePackMajor = (aVersion >> 8) & 0xFF;
    info.wServicePackMinor = aVersion & 0xFF;

    DWORDLONG conditionMask = 0;
    VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(conditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    VER_SET_CONDITION(conditionMask, VER_SERVICEPACKMINOR, VER_GREATER_EQUAL);

    if (VerifyVersionInfoW(&info,
                           VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR
                               | VER_SERVICEPACKMINOR,
                           conditionMask))
    {
        minVersion = aVersion;
        return true;
    }

    maxVersion = aVersion;
    return false;
}

bool IsWin10OrLater() { return IsWindowsVersionOrLater(0x0a000000ul); }

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

template <typename T, size_t N> constexpr size_t ArrayLength(T (&aArr)[N]) { return N; }

static bool IsWindowsLogonConnected()
{
    WCHAR userName[UNLEN + 1];
    DWORD size = ArrayLength(userName);
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

void CheckFileExtRegistration()
{
    if (!IsWin10OrLater())
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
        { L".odp", L"soffice.StarImpressDocument.6" },
        { L".odt", L"soffice.StarWriterDocument.6" },
        { L".ods", L"soffice.StarCalcDocument.6" },
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
        OUString aMsg(DpResId(STR_FILEEXT_NONDEFAULT_ASK));
        aMsg = aMsg.replaceFirst("$1", aNonDefaults);

        std::unique_ptr<weld::MessageDialog> xFileExtAskBox(Application::CreateMessageDialog(
            nullptr, VclMessageType::Question, VclButtonsType::NONE, aMsg));
        xFileExtAskBox->add_button(DpResId(STR_ANSWER_OK), RET_YES);
        xFileExtAskBox->add_button(DpResId(STR_ANSWER_CANCEL), RET_CANCEL);
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
