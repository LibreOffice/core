/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/sequence.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/System.hxx>
#include <org/freedesktop/PackageKit/SyncDbusSessionHelper.hpp>
#include <rtl/ustring.hxx>
#include <svtools/langhelp.hxx>
#include <svtools/restartdialog.hxx>
#include <vcl/idle.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/settings.hxx>
#include <vcl/window.hxx>

void localizeWebserviceURI( OUString& rURI )
{
    OUString aLang = Application::GetSettings().GetUILanguageTag().getLanguage();
    if ( aLang.equalsIgnoreAsciiCase("pt")
         && Application::GetSettings().GetUILanguageTag().getCountry().equalsIgnoreAsciiCase("br") )
    {
        aLang = "pt-br";
    }
    if ( aLang.equalsIgnoreAsciiCase("zh") )
    {
        if ( Application::GetSettings().GetUILanguageTag().getCountry().equalsIgnoreAsciiCase("cn") )
            aLang = "zh-cn";
        if ( Application::GetSettings().GetUILanguageTag().getCountry().equalsIgnoreAsciiCase("tw") )
            aLang = "zh-tw";
    }

    rURI += aLang;
}

OUString getInstalledLocaleForLanguage(css::uno::Sequence<OUString> const & installed, OUString const & locale)
{
    if (locale.isEmpty())
        return OUString();  // do not attempt to resolve anything

    for (sal_Int32 i = 0; i != installed.getLength(); ++i) {
        if (installed[i] == locale) {
            return installed[i];
        }
    }
    std::vector<OUString> fallbacks(LanguageTag(locale).getFallbackStrings(false));
    for (OUString & rf : fallbacks) {
        for (sal_Int32 i = 0; i != installed.getLength(); ++i) {
            if (installed[i] == rf) {
                return installed[i];
            }
        }
    }
    return OUString();
}

std::unique_ptr<Idle> xLangpackInstaller;

class InstallLangpack : public Idle
{
    std::vector<OUString> m_aPackages;
public:
    explicit InstallLangpack(const std::vector<OUString>& rPackages)
        : Idle("install langpack")
        , m_aPackages(rPackages)
    {
        SetPriority(TaskPriority::LOWEST);
    }

    virtual void Invoke() override
    {
        vcl::Window* pTopWindow = Application::GetActiveTopWindow();
        if (!pTopWindow)
            pTopWindow = Application::GetFirstTopLevelWindow();
        if (!pTopWindow)
        {
            Start();
            return;
        }
        try
        {
            using namespace org::freedesktop::PackageKit;
            using namespace svtools;
            css::uno::Reference<XSyncDbusSessionHelper> xSyncDbusSessionHelper(SyncDbusSessionHelper::create(comphelper::getProcessComponentContext()));
            const SystemEnvData* pEnvData = pTopWindow->GetSystemData();
            sal_uInt32 nDbusId = pEnvData ? pEnvData->aWindow : 0;
            xSyncDbusSessionHelper->InstallPackageNames(nDbusId, comphelper::containerToSequence(m_aPackages), OUString());
            // Ill be back (hopefully)!
            SolarMutexGuard aGuard;
            executeRestartDialog(comphelper::getProcessComponentContext(), pTopWindow, RESTART_REASON_LANGUAGE_CHANGE);
        }
        catch (const css::uno::Exception& e)
        {
            SAL_INFO("svl", "trying to install a LibreOffice langpack, caught " << e);
        }
        xLangpackInstaller.reset();
    }
};

OUString getInstalledLocaleForSystemUILanguage(const css::uno::Sequence<OUString>& rLocaleElementNames, bool bRequestInstallIfMissing)
{
    OUString wantedLocale = officecfg::System::L10N::UILocale::get();
    OUString locale = getInstalledLocaleForLanguage(rLocaleElementNames, wantedLocale);
    if (bRequestInstallIfMissing && locale.isEmpty() && !wantedLocale.isEmpty() && !Application::IsHeadlessModeEnabled() &&
        officecfg::Office::Common::PackageKit::EnableLangpackInstallation::get())
    {
        LanguageTag aWantedTag(wantedLocale);
        if (aWantedTag.getLanguage() != "en")
        {
            // langpack is the typical Fedora/RHEL naming convention
            // l10n is the typical Debian/Ubuntu naming convention
            //
            // We try these bases + the exact locale + fallback locale.
            //
            // The theory is that we can request a bunch of packages
            // and it's a success if one of them is available
            std::vector<OUString> aPackages;
            LanguageType eType = aWantedTag.getLanguageType();
            if (MsLangId::isSimplifiedChinese(eType))
            {
                aPackages.emplace_back("libreoffice-langpack-zh-Hans");
                aPackages.emplace_back("libreoffice-l10n-zh-CN");
            }
            else if (MsLangId::isTraditionalChinese(eType))
            {
                aPackages.emplace_back("libreoffice-langpack-zh-Hant");
                aPackages.emplace_back("libreoffice-l10n-zh-TW");
            }
            else
            {
                std::vector<OUString> aFallbacks(aWantedTag.getFallbackStrings(false));
                for (const auto& a : aFallbacks)
                {
                    aPackages.emplace_back("libreoffice-langpack-" + a);
                    aPackages.emplace_back("libreoffice-l10n-" + a);
                }
            }
            xLangpackInstaller.reset(new InstallLangpack(aPackages));
            xLangpackInstaller->Start();
        }
    }
    if (locale.isEmpty())
        locale = getInstalledLocaleForLanguage(rLocaleElementNames, "en-US");
    if (locale.isEmpty() && rLocaleElementNames.hasElements())
        locale = rLocaleElementNames[0];
    return locale;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
