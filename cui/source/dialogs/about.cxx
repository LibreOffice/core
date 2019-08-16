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

#include <config_features.h>
#include <osl/process.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <vcl/button.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>

#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <com/sun/star/uno/Any.h>
#include <svtools/langhelp.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <svtools/optionsdrawinglayer.hxx>

#include <about.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>
#include <config_buildid.h>
#include <sfx2/app.hxx>

#if HAVE_FEATURE_OPENCL
#include <opencl/openclwrapper.hxx>
#endif
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Calc.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

AboutDialog::AboutDialog(weld::Window* pParent)
    : m_xBuilder(Application::CreateBuilder(pParent, "cui/ui/aboutdialog.ui"))
    , m_xDialog(m_xBuilder->weld_about_dialog("AboutDialog"))
    , m_xContentArea(m_xDialog->weld_content_area())
{
    m_xDialog->add_button(Button::GetStandardText(StandardButtonType::Close), RET_CLOSE);
    m_xDialog->add_button(CuiResId(RID_SVXSTR_ABOUT_CREDITS), 101);
    m_xDialog->add_button(CuiResId(RID_SVXSTR_ABOUT_WEBSITE), 102);
    m_xDialog->add_button(CuiResId(RID_SVXSTR_ABOUT_RELEASE_NOTES), 103);

    m_pCreditsButton = m_xDialog->get_widget_for_response(101);
    m_pCreditsButton->set_secondary(true);
    m_pWebsiteButton = m_xDialog->get_widget_for_response(102);
    m_pWebsiteButton->set_secondary(true);
    m_pReleaseNotesButton = m_xDialog->get_widget_for_response(103);
    m_pReleaseNotesButton->set_secondary(true);
    m_pCloseButton = m_xDialog->get_widget_for_response(RET_CLOSE);

    m_buildIdLinkString = m_xDialog->get_website_label();

    m_xDialog->set_version(GetVersionString());
    m_xDialog->set_copyright(GetCopyrightString());

    SetBuildIdLink();

    SetLogo();

    m_xDialog->connect_size_allocate(LINK(this, AboutDialog, SizeAllocHdl));

    // Connect all handlers
    m_pCreditsButton->connect_clicked( LINK( this, AboutDialog, HandleClick ) );
    m_pWebsiteButton->connect_clicked( LINK( this, AboutDialog, HandleClick ) );
    m_pReleaseNotesButton->connect_clicked( LINK( this, AboutDialog, HandleClick ) );
    m_pCloseButton->grab_focus();
}

AboutDialog::~AboutDialog()
{
}

IMPL_LINK(AboutDialog, HandleClick, weld::Button&, rButton, void)
{
    OUString sURL = "";

    // Find which button was pressed and from this, get the URL to be opened
    if (&rButton == m_pCreditsButton)
        sURL = CuiResId(RID_SVXSTR_ABOUT_CREDITS_URL);
    else if (&rButton == m_pWebsiteButton)
    {
        sURL = officecfg::Office::Common::Help::StartCenter::InfoURL::get();
        localizeWebserviceURI(sURL);
    }
    else if (&rButton == m_pReleaseNotesButton)
    {
        sURL = officecfg::Office::Common::Menus::ReleaseNotesURL::get() +
               "?LOvers=" + utl::ConfigManager::getProductVersion() +
               "&LOlocale=" + LanguageTag(utl::ConfigManager::getUILocale()).getLanguage();
    }

    // If the URL is empty, don't do anything
    if ( sURL.isEmpty() )
        return;
    try
    {
        Reference< css::system::XSystemShellExecute > xSystemShellExecute(
            css::system::SystemShellExecute::create(::comphelper::getProcessComponentContext() ) );
        xSystemShellExecute->execute( sURL, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY );
    }
    catch (const Exception&)
    {
        Any exc( ::cppu::getCaughtException() );
        OUString msg( ::comphelper::anyToString( exc ) );
        const SolarMutexGuard guard;
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(getDialog(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, msg));
        xErrorBox->set_title(m_xDialog->get_title());
        xErrorBox->run();
    }
}

void AboutDialog::SetBuildIdLink()
{
    const OUString buildId = GetBuildId();

    if (IsStringValidGitHash(buildId))
    {
        if (m_buildIdLinkString.indexOf("$GITHASH") == -1)
        {
            SAL_WARN( "cui.dialogs", "translated git hash string in translations doesn't contain $GITHASH placeholder" );
            m_buildIdLinkString += " $GITHASH";
        }

        m_xDialog->set_website_label(m_buildIdLinkString.replaceAll("$GITHASH", buildId));
        m_xDialog->set_website("https://hub.libreoffice.org/git-core/" + buildId);
    }
    else
    {
        m_xDialog->set_website_label(OUString());
        m_xDialog->set_website(OUString());
    }
}

void AboutDialog::SetLogo()
{
    auto nWidth = m_xContentArea->get_preferred_size().Width();

    // fdo#67401 set AntiAliasing for SVG logo
    SvtOptionsDrawinglayer aDrawOpt;
    bool bOldAntiAliasSetting = aDrawOpt.IsAntiAliasing();
    aDrawOpt.SetAntiAliasing(true);

    // load svg logo, specify desired width, scale height isotropically
    SfxApplication::loadBrandSvg("flat_logo", aLogoBitmap, nWidth);

    aDrawOpt.SetAntiAliasing(bOldAntiAliasSetting);

    if (!aLogoBitmap)
        m_xDialog->set_logo(nullptr);
    else
    {
        Graphic aGraphic(aLogoBitmap);
        m_xDialog->set_logo(aGraphic.GetXGraphic());
    }
}

IMPL_LINK(AboutDialog, SizeAllocHdl, const Size&, rSize, void)
{
    if (rSize.Width() == aBackgroundBitmap.GetSizePixel().Width())
        return;
    // Load background image
    if (!(Application::GetSettings().GetStyleSettings().GetHighContrastMode()))
    {
        SfxApplication::loadBrandSvg("shell/about", aBackgroundBitmap, rSize.Width());
        Graphic aGraphic(aBackgroundBitmap);
        m_xDialog->set_background(aGraphic.GetXGraphic());
    }
}

OUString AboutDialog::GetBuildId()
{
    OUString sDefault;
    OUString sBuildId(utl::Bootstrap::getBuildVersion(sDefault));
    if (!sBuildId.isEmpty())
        return sBuildId;

    sBuildId = utl::Bootstrap::getBuildIdData(sDefault);

    if (!sBuildId.isEmpty())
    {
        return sBuildId.getToken( 0, '-' );
    }

    OSL_ENSURE( !sBuildId.isEmpty(), "No BUILDID in bootstrap file" );
    return sBuildId;
}

OUString AboutDialog::GetLocaleString()
{
    OUString aLocaleStr;
    rtl_Locale * pLocale;

    osl_getProcessLocale( &pLocale );

    if ( pLocale && pLocale->Language )
    {
        if (pLocale->Country && rtl_uString_getLength( pLocale->Country) > 0)
            aLocaleStr = OUString(pLocale->Language) + "_" + OUString(pLocale->Country);
        else
            aLocaleStr = OUString(pLocale->Language);
        if (pLocale->Variant && rtl_uString_getLength( pLocale->Variant) > 0)
            aLocaleStr += OUString(pLocale->Variant);
    }

    return aLocaleStr;
}

bool AboutDialog::IsStringValidGitHash(const OUString& hash)
{
    for (int i = 0; i < hash.getLength(); i++)
    {
        if (!rtl::isAsciiHexDigit(hash[i]))
        {
            return false;
        }
    }

    return true;
}

OUString AboutDialog::GetVersionString()
{
    OUString sVersion = CuiResId(RID_SVXSTR_ABOUT_VERSION);

#ifdef _WIN64
    sVersion += " (x64)";
#elif defined(_WIN32)
    sVersion += " (x86)";
#endif

    OUString sBuildId = GetBuildId();

    OUString aLocaleStr = Application::GetSettings().GetLanguageTag().getBcp47() + " (" + GetLocaleString() + ")";
    OUString aUILocaleStr = Application::GetSettings().GetUILanguageTag().getBcp47();

    if (!sBuildId.trim().isEmpty())
    {
        sVersion += "\n";
        OUString sBuildStr = CuiResId(RID_SVXSTR_ABOUT_BUILDID);
        if (sBuildStr.indexOf("$BUILDID") == -1)
        {
            SAL_WARN( "cui.dialogs", "translated Build Id string in translations doesn't contain $BUILDID placeholder" );
            sBuildStr += " $BUILDID";
        }
        sVersion += sBuildStr.replaceAll("$BUILDID", sBuildId);
    }

    sVersion += "\n" + Application::GetHWOSConfInfo();

    bool const extra = EXTRA_BUILDID[0] != '\0';
        // extracted from the 'if' to avoid Clang -Wunreachable-code
    if (extra)
    {
        sVersion += "\n" EXTRA_BUILDID;
    }

    OUString sLocaleStr(CuiResId(RID_SVXSTR_ABOUT_LOCALE));
    if (sLocaleStr.indexOf("$LOCALE") == -1)
    {
        SAL_WARN( "cui.dialogs", "translated locale string in translations doesn't contain $LOCALE placeholder" );
        sLocaleStr += " $LOCALE";
    }
    sVersion += "\n" + sLocaleStr.replaceAll("$LOCALE", aLocaleStr);

    OUString sUILocaleStr(CuiResId(RID_SVXSTR_ABOUT_UILOCALE));
    if (sUILocaleStr.indexOf("$LOCALE") == -1)
    {
        SAL_WARN( "cui.dialogs", "translated uilocale string in translations doesn't contain $LOCALE placeholder" );
        sUILocaleStr += " $LOCALE";
    }
    sVersion += "; " + sUILocaleStr.replaceAll("$LOCALE", aUILocaleStr);

    OUString aCalcMode = "Calc: "; // Calc calculation mode

#if HAVE_FEATURE_OPENCL
    bool bOpenCL = openclwrapper::GPUEnv::isOpenCLEnabled();
    if (bOpenCL)
        aCalcMode += "CL";
#else
    const bool bOpenCL = false;
#endif

    static const bool bThreadingProhibited = std::getenv("SC_NO_THREADED_CALCULATION");
    bool bThreadedCalc = officecfg::Office::Calc::Formula::Calculation::UseThreadedCalculationForFormulaGroups::get();

    if (!bThreadingProhibited && !bOpenCL && bThreadedCalc)
    {
        if (!aCalcMode.endsWith(" "))
            aCalcMode += " ";
        aCalcMode += "threaded";
    }

    sVersion += "\n" + aCalcMode;

    return sVersion;
}

OUString AboutDialog::GetCopyrightString()
{
    OUString sVendorTextStr(CuiResId(RID_SVXSTR_ABOUT_VENDOR));
    OUString aCopyrightString  = sVendorTextStr + "\n"
                               + CuiResId(RID_SVXSTR_ABOUT_COPYRIGHT) + "\n";

    if (utl::ConfigManager::getProductName() == "LibreOffice")
        aCopyrightString += CuiResId(RID_SVXSTR_ABOUT_BASED_ON);
    else
        aCopyrightString += CuiResId(RID_SVXSTR_ABOUT_DERIVED);

    return aCopyrightString;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
