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

#include <about.hxx>

#include <osl/diagnose.h> //OSL_ENSURE
#include <osl/process.h> //osl_getProcessLocale
#include <rtl/character.hxx> //rtl::isAsciiHexDigit
#include <sal/log.hxx> //SAL_WARN
#include <vcl/settings.hxx> //GetSettings
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx> //Application::
#include <vcl/virdev.hxx> //VirtualDevice

#include <i18nlangtag/languagetag.hxx>
#include <svtools/langhelp.hxx>
#include <unotools/bootstrap.hxx> //utl::Bootstrap::getBuildVersion
#include <unotools/configmgr.hxx> //ConfigManager::
#include <config_buildid.h> //EXTRA_BUILDID
#include <dialmgr.hxx> //CuiResId
#include <sfx2/app.hxx> //SfxApplication::loadBrandSvg
#include <strings.hrc>

#include <config_feature_opencl.h>
#if HAVE_FEATURE_OPENCL
#include <opencl/openclwrapper.hxx>
#endif
#include <officecfg/Office/Calc.hxx>
#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star::uno;

AboutDialog::AboutDialog(weld::Window *pParent)
    : GenericDialogController(pParent, "cui/ui/aboutdialog.ui", "AboutDialog"),
      m_pCreditsButton(m_xBuilder->weld_link_button("btnCredits")),
      m_pWebsiteButton(m_xBuilder->weld_link_button("btnWebsite")),
      m_pReleaseNotesButton(m_xBuilder->weld_link_button("btnReleaseNotes")),
      m_pCloseButton(m_xBuilder->weld_button("btnClose")),
      m_pBrandImage(m_xBuilder->weld_image("imBrand")),
      m_pAboutImage(m_xBuilder->weld_image("imAbout")),
      m_pVersionLabel(m_xBuilder->weld_label("lbVersion")),
      m_pCopyrightLabel(m_xBuilder->weld_label("lbCopyright")) {
  m_pVersionLabel->set_label(GetVersionString());
  m_pCopyrightLabel->set_label(GetCopyrightString());

  //Images
  const Size nWidth(m_pVersionLabel->get_preferred_size());
  BitmapEx aBackgroundBitmap;

  if (SfxApplication::loadBrandSvg(Application::GetSettings()
                                           .GetStyleSettings()
                                           .GetDialogColor()
                                           .IsDark()
                                       ? "shell/logo_inverted"
                                       : "shell/logo",
                                   aBackgroundBitmap,
                                   nWidth.getWidth() * 0.8)) {
    ScopedVclPtr<VirtualDevice> m_pVirDev =
        m_pBrandImage->create_virtual_device();
    m_pVirDev->SetOutputSizePixel(aBackgroundBitmap.GetSizePixel());
    m_pVirDev->DrawBitmapEx(Point(0, 0), aBackgroundBitmap);
    m_pBrandImage->set_image(m_pVirDev.get());
    m_pVirDev.disposeAndClear();
  }
  if (SfxApplication::loadBrandSvg("shell/about", aBackgroundBitmap,
                                   nWidth.getWidth())) {
    ScopedVclPtr<VirtualDevice> m_pVirDev =
        m_pAboutImage->create_virtual_device();
    m_pVirDev->SetOutputSizePixel(aBackgroundBitmap.GetSizePixel());
    m_pVirDev->DrawBitmapEx(Point(0, 0), aBackgroundBitmap);
    m_pAboutImage->set_image(m_pVirDev.get());
    m_pVirDev.disposeAndClear();
  }

  //Links
  m_pCreditsButton->set_uri(CuiResId(RID_SVXSTR_ABOUT_CREDITS_URL));

  OUString sURL(officecfg::Office::Common::Help::StartCenter::InfoURL::get());
  localizeWebserviceURI(sURL);
  m_pWebsiteButton->set_uri(sURL);

  sURL = officecfg::Office::Common::Menus::ReleaseNotesURL::get() +
         "?LOvers=" + utl::ConfigManager::getProductVersion() + "&LOlocale=" +
         LanguageTag(utl::ConfigManager::getUILocale()).getLanguage();
  m_pReleaseNotesButton->set_uri(sURL);

  //Handler
  m_pCloseButton->grab_focus();
}

AboutDialog::~AboutDialog() {}

OUString AboutDialog::GetBuildId() {
  OUString sDefault;
  OUString sBuildId(utl::Bootstrap::getBuildVersion(sDefault));
  if (!sBuildId.isEmpty())
    return sBuildId;

  sBuildId = utl::Bootstrap::getBuildIdData(sDefault);

  if (!sBuildId.isEmpty()) {
    return sBuildId.getToken(0, '-');
  }

  OSL_ENSURE(!sBuildId.isEmpty(), "No BUILDID in bootstrap file");
  return sBuildId;
}

OUString AboutDialog::GetLocaleString() {
  OUString aLocaleStr;
  rtl_Locale *pLocale;

  osl_getProcessLocale(&pLocale);

  if (pLocale && pLocale->Language) {
    if (pLocale->Country && rtl_uString_getLength(pLocale->Country) > 0)
      aLocaleStr = OUString::unacquired(&pLocale->Language) + "_" +
                   OUString::unacquired(&pLocale->Country);
    else
      aLocaleStr = OUString(pLocale->Language);
    if (pLocale->Variant && rtl_uString_getLength(pLocale->Variant) > 0)
      aLocaleStr += OUString(pLocale->Variant);
  }

  return aLocaleStr;
}

bool AboutDialog::IsStringValidGitHash(const OUString &hash) {
  for (int i = 0; i < hash.getLength(); i++) {
    if (!rtl::isAsciiHexDigit(hash[i])) {
      return false;
    }
  }

  return true;
}

OUString AboutDialog::GetVersionString() {
  OUString sVersion = CuiResId(RID_SVXSTR_ABOUT_VERSION);

#ifdef _WIN64
  sVersion += " (x64)";
#elif defined(_WIN32)
  sVersion += " (x86)";
#endif

  OUString sBuildId = GetBuildId();

  OUString aLocaleStr = Application::GetSettings().GetLanguageTag().getBcp47() +
                        " (" + GetLocaleString() + ")";
  OUString aUILocaleStr =
      Application::GetSettings().GetUILanguageTag().getBcp47();

  if (!sBuildId.trim().isEmpty()) {
    sVersion += "\n";
    OUString sBuildStr = CuiResId(RID_SVXSTR_ABOUT_BUILDID);
    if (sBuildStr.indexOf("$BUILDID") == -1) {
      SAL_WARN("cui.dialogs", "translated Build Id string in translations "
                              "doesn't contain $BUILDID placeholder");
      sBuildStr += " $BUILDID";
    }
    sVersion += sBuildStr.replaceAll("$BUILDID", sBuildId);
  }

  sVersion += "\n" + Application::GetHWOSConfInfo();

  bool const extra = EXTRA_BUILDID[0] != '\0';
  // extracted from the 'if' to avoid Clang -Wunreachable-code
  if (extra) {
    sVersion += "\n" EXTRA_BUILDID;
  }

  OUString sLocaleStr(CuiResId(RID_SVXSTR_ABOUT_LOCALE));
  if (sLocaleStr.indexOf("$LOCALE") == -1) {
    SAL_WARN("cui.dialogs", "translated locale string in translations doesn't "
                            "contain $LOCALE placeholder");
    sLocaleStr += " $LOCALE";
  }
  sVersion += "\n" + sLocaleStr.replaceAll("$LOCALE", aLocaleStr);

  OUString sUILocaleStr(CuiResId(RID_SVXSTR_ABOUT_UILOCALE));
  if (sUILocaleStr.indexOf("$LOCALE") == -1) {
    SAL_WARN("cui.dialogs", "translated uilocale string in translations "
                            "doesn't contain $LOCALE placeholder");
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

  static const bool bThreadingProhibited =
      std::getenv("SC_NO_THREADED_CALCULATION");
  bool bThreadedCalc = officecfg::Office::Calc::Formula::Calculation::
      UseThreadedCalculationForFormulaGroups::get();

  if (!bThreadingProhibited && !bOpenCL && bThreadedCalc) {
    if (!aCalcMode.endsWith(" "))
      aCalcMode += " ";
    aCalcMode += "threaded";
  }

  sVersion += "\n" + aCalcMode;

  return sVersion;
}

OUString AboutDialog::GetCopyrightString() {
  OUString sVendorTextStr(CuiResId(RID_SVXSTR_ABOUT_VENDOR));
  OUString aCopyrightString =
      sVendorTextStr + "\n" + CuiResId(RID_SVXSTR_ABOUT_COPYRIGHT) + "\n";

  if (utl::ConfigManager::getProductName() == "LibreOffice")
    aCopyrightString += CuiResId(RID_SVXSTR_ABOUT_BASED_ON);
  else
    aCopyrightString += CuiResId(RID_SVXSTR_ABOUT_DERIVED);

  return aCopyrightString;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
