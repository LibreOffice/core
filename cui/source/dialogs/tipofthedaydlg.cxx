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

#include <sal/config.h>
#include <tipofthedaydlg.hxx>
#include <tipoftheday.hrc>

#include <vcl/graphicfilter.hxx>
#include <vcl/help.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertysequence.hxx>
#include <dialmgr.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/configmgr.hxx>

TipOfTheDayDialog::TipOfTheDayDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/tipofthedaydialog.ui", "TipOfTheDayDialog")
    , m_pImage(m_xBuilder->weld_image("imImage"))
    , m_pText(m_xBuilder->weld_label("lbText"))
    , m_pShowTip(m_xBuilder->weld_check_button("cbShowTip"))
    , m_pNext(m_xBuilder->weld_button("btnNext"))
    , m_pLink(m_xBuilder->weld_link_button("btnLink"))
{
    m_pShowTip->set_active(officecfg::Office::Common::Misc::ShowTipOfTheDay::get());
    m_pNext->connect_clicked(LINK(this, TipOfTheDayDialog, OnNextClick));

    nNumberOfTips = SAL_N_ELEMENTS(TIPOFTHEDAY_STRINGARRAY);
    nCurrentTip = officecfg::Office::Common::Misc::LastTipOfTheDayID::get();

    const auto t0 = std::chrono::system_clock::now().time_since_epoch();
    nDay = std::chrono::duration_cast<std::chrono::hours>(t0).count() / 24; //days since 1970-01-01
    if (nDay > officecfg::Office::Common::Misc::LastTipOfTheDayShown::get())
        nCurrentTip++;

    UpdateTip();
}

TipOfTheDayDialog::~TipOfTheDayDialog()
{
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::LastTipOfTheDayShown::set(nDay, xChanges);
    officecfg::Office::Common::Misc::LastTipOfTheDayID::set(nCurrentTip, xChanges);
    officecfg::Office::Common::Misc::ShowTipOfTheDay::set(m_pShowTip->get_active(), xChanges);
    xChanges->commit();
}

static bool file_exists(const OUString& fileName)
{
    ::osl::File aFile(fileName);
    return aFile.open(osl_File_OpenFlag_Read) == osl::FileBase::E_None;
}

void TipOfTheDayDialog::UpdateTip()
{
    if ((nCurrentTip + 1 > nNumberOfTips) || (nCurrentTip < 0))
        nCurrentTip = 0;
    m_xDialog->set_title(CuiResId(STR_TITLE) + ": " + OUString::number(nCurrentTip + 1) + "/"
                         + OUString::number(nNumberOfTips));

    // text
    OUString aText = CuiResId(std::get<0>(TIPOFTHEDAY_STRINGARRAY[nCurrentTip]));
//replace MOD1 & MOD2 shortcuts depending on platform
#ifdef MACOSX
    const OUString aMOD1 = CuiResId(STR_CMD);
    const OUString aMOD2 = CuiResId(STR_Option);
#else
    const OUString aMOD1 = CuiResId(STR_CTRL);
    const OUString aMOD2 = CuiResId(STR_Alt);
#endif
    sal_Int32 aPos;
    aPos = aText.indexOf("%MOD1");
    while (aPos != -1)
    {
        aText = aText.replaceAt(aPos, 5, aMOD1);
        aPos = aText.indexOf("%MOD1");
    }
    aPos = aText.indexOf("%MOD2");
    while (aPos != -1)
    {
        aText = aText.replaceAt(aPos, 5, aMOD2);
        aPos = aText.indexOf("%MOD2");
    }
    m_pText->set_label(aText);

    // hyperlink
    aLink = std::get<1>(TIPOFTHEDAY_STRINGARRAY[nCurrentTip]);
    if (aLink.isEmpty())
    {
        m_pLink->set_visible(false);
    }
    else if (aLink.startsWith(".uno:"))
    {
        m_pLink->set_uri(CuiResId(STR_UNO_EXECUTE).replaceFirst("%COMMAND", aLink));
        m_pLink->set_label(CuiResId(STR_UNO_LINK));
        m_pLink->set_visible(true);
        m_pLink->connect_activate_link(LINK(this, TipOfTheDayDialog, OnLinkClick));
    }
    else if (aLink.startsWith("http"))
    {
        // Links may have some %PRODUCTVERSION which need to be expanded
        aText = Translate::ExpandVariables(aLink);
        aPos = aText.indexOf("%LANGUAGENAME");
        if (aPos != -1)
        {
            OUString aLang = LanguageTag(utl::ConfigManager::getUILocale()).getLanguage();
            if (aLang == "en" || aLang == "pt" || aLang == "zh") //en-US/GB, pt-BR, zh-CH/TW
                aLang = LanguageTag(utl::ConfigManager::getUILocale()).getBcp47();
            aText = aText.replaceAt(aPos, 13, aLang);
        }
        m_pLink->set_uri(aText);

        m_pLink->set_label(CuiResId(STR_MORE_LINK));
        m_pLink->set_visible(true);
        m_pLink->connect_activate_link(Link<weld::LinkButton&, bool>());
    }
    else
    {
        m_pLink->set_uri("");
        m_pLink->set_label(CuiResId(STR_HELP_LINK));
        m_pLink->set_visible(true);
        //converts aLink into the proper offline/online hyperlink
        m_pLink->connect_activate_link(LINK(this, TipOfTheDayDialog, OnLinkClick));
    }
    // image
    OUString aURL("$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/tipoftheday/");
    rtl::Bootstrap::expandMacros(aURL);
    OUString aImage = std::get<2>(TIPOFTHEDAY_STRINGARRAY[nCurrentTip]);
    // use default image if none is available with the number
    if (aImage.isEmpty() || !file_exists(aURL + aImage))
        aImage = "tipoftheday.png";
    // draw image
    Graphic aGraphic;
    if (GraphicFilter::LoadGraphic(aURL + aImage, OUString(), aGraphic) == ERRCODE_NONE)
    {
        ScopedVclPtr<VirtualDevice> m_pVirDev = m_pImage->create_virtual_device();
        m_pVirDev->SetOutputSizePixel(aGraphic.GetSizePixel());
        m_pVirDev->DrawBitmapEx(Point(0, 0), aGraphic.GetBitmapEx());
        m_pImage->set_image(m_pVirDev.get());
        m_pVirDev.disposeAndClear();
    }
}

IMPL_LINK_NOARG(TipOfTheDayDialog, OnLinkClick, weld::LinkButton&, bool)
{
    if (aLink.startsWith("http"))
    {
        Application::GetHelp()->Start(aLink, static_cast<weld::Widget*>(nullptr));
    }
    else if (aLink.startsWith(".uno:"))
    {
        comphelper::dispatchCommand(aLink, {});
        TipOfTheDayDialog::response(RET_OK);
    }
    return true;
}

IMPL_LINK_NOARG(TipOfTheDayDialog, OnNextClick, weld::Button&, void)
{
    nCurrentTip++; //zeroed at updatetip when out of range
    UpdateTip();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
