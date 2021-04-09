/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if !defined(NTDDI_VERSION) || NTDDI_VERSION < NTDDI_WIN8
#define NTDDI_VERSION NTDDI_WIN8 // needed for IApplicationActivationManager
#endif

#include <sal/config.h>

#include <comphelper/scopeguard.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/fileregistration.hxx>

#include <strings.hrc>
#include <svdata.hxx>

#include <utility>

#include <prewin.h>
#include <Shobjidl.h>
#include <systools/win32/comtools.hxx>
#include <versionhelpers.h>
#include <postwin.h>

#define MAX_LONG_PATH 32767

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

static bool IsDefaultAppInstalledInReg()
{
    const wchar_t* keyPath = L"SOFTWARE\\LibreOffice\\UNO\\InstallPath";

    WCHAR szRegPath[MAX_LONG_PATH];
    DWORD cbData = static_cast<DWORD>(MAX_LONG_PATH * sizeof(WCHAR));
    auto rc = RegGetValueW(HKEY_LOCAL_MACHINE, keyPath, nullptr, RRF_RT_REG_SZ, nullptr,
                           static_cast<PVOID>(szRegPath), &cbData);
    if (rc != ERROR_SUCCESS)
        return false;

    WCHAR szProcPath[MAX_LONG_PATH];
    if (!GetModuleFileNameW(nullptr, szProcPath, MAX_LONG_PATH))
        return false;

    WCHAR szFullProcPath[MAX_LONG_PATH];
    if (!GetFullPathNameW(szProcPath, MAX_LONG_PATH, szFullProcPath, nullptr))
        return false;

    if (!GetLongPathNameW(szFullProcPath, szFullProcPath, MAX_LONG_PATH))
        return false;

    if (!GetLongPathNameW(szRegPath, szRegPath, MAX_LONG_PATH))
        return false;

    if (wcslen(szRegPath) > 0 && wcsstr(szFullProcPath, szRegPath) != nullptr)
        return true;

    return false;
}

void LaunchRegistrationUI()
{
    const bool bUninit = SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
    comphelper::ScopeGuard g([bUninit]() {
        if (bUninit)
            CoUninitialize();
    });

    try
    {
        if (IsWindows10OrGreater())
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
}

void CheckFileExtRegistration(weld::Window* pDialogParent)
{
    if (!officecfg::Office::Common::Misc::PerformFileExtCheck::get())
        return;

    if (!IsDefaultAppInstalledInReg())
        return;

    const bool bUninit = SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
    comphelper::ScopeGuard g([bUninit]() {
        if (bUninit)
            CoUninitialize();
    });
    sal::systools::COMReference<IApplicationAssociationRegistration> pAAR;
    try
    {
        pAAR.CoCreateInstance(CLSID_ApplicationAssociationRegistration, nullptr,
                              CLSCTX_INPROC_SERVER);
    }
    catch (...)
    {
        // Just return on any error here: this is not something we need to make sure to succeed
        return;
    }

    static const std::pair<LPCWSTR, LPCWSTR> formats[] = {
        { L".odp", L"LibreOffice.ImpressDocument.1" },
        { L".odt", L"LibreOffice.WriterDocument.1" },
        { L".ods", L"LibreOffice.CalcDocument.1" },
    };
    OUString aNonDefaults;

    for (const auto & [ szExt, szProgId ] : formats)
    {
        if (IsPathDefaultForClass(pAAR, szExt, szProgId) == S_FALSE)
            aNonDefaults += OUString::Concat(o3tl::toU(szExt)) + "\n";
    }

    if (!aNonDefaults.isEmpty())
    {
        OUString aMsg(VclResId(STR_FILEEXT_NONDEFAULT_ASK_MSG).replaceFirst("$1", aNonDefaults));

        VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
        ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateFileExtCheckDialog(
            pDialogParent, VclResId(STR_FILEEXT_NONDEFAULT_ASK_TITLE), aMsg));
        pDlg->Execute();
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
