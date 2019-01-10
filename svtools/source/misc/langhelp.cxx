/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <comphelper/sequence.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/System.hxx>
#include <org/freedesktop/PackageKit/SyncDbusSessionHelper.hpp>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <svtools/langhelp.hxx>
#include <vcl/idle.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/settings.hxx>
#include <vcl/window.hxx>
#include <config_langs.h>
#include <config_vendor.h>

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

static std::unique_ptr<Idle> xLangpackInstaller;

class InstallLangpack : public Idle
{
    std::vector<OUString> const m_aPackages;
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
            css::uno::Reference<XSyncDbusSessionHelper> xSyncDbusSessionHelper(SyncDbusSessionHelper::create(comphelper::getProcessComponentContext()));
            xSyncDbusSessionHelper->InstallPackageNames(comphelper::containerToSequence(m_aPackages), OUString());
        }
        catch (const css::uno::Exception& e)
        {
            SAL_INFO("svl", "trying to install a LibreOffice langpack, caught " << e);
        }
        xLangpackInstaller.reset();
    }
};

OUString getInstalledLocaleForSystemUILanguage(const css::uno::Sequence<OUString>& rLocaleElementNames, bool bRequestInstallIfMissing, const OUString& rPreferredLocale)
{
    OUString wantedLocale(rPreferredLocale);
    if (wantedLocale.isEmpty())
        wantedLocale = officecfg::System::L10N::UILocale::get();

    OUString locale = getInstalledLocaleForLanguage(rLocaleElementNames, wantedLocale);
    if (bRequestInstallIfMissing && locale.isEmpty() && !wantedLocale.isEmpty() && !Application::IsHeadlessModeEnabled() &&
        officecfg::Office::Common::PackageKit::EnableLangpackInstallation::get())
    {
        LanguageTag aWantedTag(wantedLocale);
        if (aWantedTag.getLanguage() != "en")
        {
            // Get the list of langpacks that this build was configured to include
            std::vector<OUString> aPackages;
            OUString const sAvailableLocales(WITH_LANG);
            std::vector<OUString> aAvailable;
            sal_Int32 nIndex = 0;
            do
            {
                aAvailable.emplace_back(sAvailableLocales.getToken(0, ' ', nIndex));
            }
            while (nIndex >= 0);
            // See which one matches the desired ui locale
            OUString install = getInstalledLocaleForLanguage(comphelper::containerToSequence(aAvailable), wantedLocale);
            if (!install.isEmpty() && install != "en-US")
            {
                std::string_view sVendor(OOO_VENDOR);
                if (sVendor == "Red Hat, Inc." || sVendor == "The Fedora Project")
                {
                    // langpack is the typical Fedora/RHEL naming convention
                    LanguageType eType = aWantedTag.getLanguageType();
                    if (MsLangId::isSimplifiedChinese(eType))
                        aPackages.emplace_back("libreoffice-langpack-zh-Hans");
                    else if (MsLangId::isTraditionalChinese(eType))
                        aPackages.emplace_back("libreoffice-langpack-zh-Hant");
                    else
                        aPackages.emplace_back("libreoffice-langpack-" + install);
                }
                //Debian would be: "The Document Foundation/Debian"
                else if (sVendor == "The Document Foundation, Debian and Ubuntu")
                {
                    // l10n is the typical Debian/Ubuntu naming convention
                    aPackages.emplace_back("libreoffice-l10n-" + install);
                }
            }
            if (!aPackages.empty())
            {
                xLangpackInstaller.reset(new InstallLangpack(aPackages));
                xLangpackInstaller->Start();
            }
        }
    }
    if (locale.isEmpty())
        locale = getInstalledLocaleForLanguage(rLocaleElementNames, "en-US");
    if (locale.isEmpty() && rLocaleElementNames.hasElements())
        locale = rLocaleElementNames[0];
    return locale;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
