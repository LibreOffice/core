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

#include <sfx2/viewfrm.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/help.hxx>

#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <comphelper/dispatchcommand.hxx>
#include <dialmgr.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

//size of preview
const Size ThumbSize(150, 150);

TipOfTheDayDialog::TipOfTheDayDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/tipofthedaydialog.ui", "TipOfTheDayDialog")
    , m_pText(m_xBuilder->weld_label("lbText"))
    , m_pShowTip(m_xBuilder->weld_check_button("cbShowTip"))
    , m_pNext(m_xBuilder->weld_button("btnNext"))
    , m_pLink(m_xBuilder->weld_link_button("btnLink"))
    , m_pPreview(new weld::CustomWeld(*m_xBuilder, "imPreview", m_aPreview))
{
    m_pShowTip->set_active(officecfg::Office::Common::Misc::ShowTipOfTheDay::get());
    m_pNext->connect_clicked(LINK(this, TipOfTheDayDialog, OnNextClick));
    m_nCurrentTip = officecfg::Office::Common::Misc::LastTipOfTheDayID::get();
    m_pPreview->set_size_request(ThumbSize.Width(), ThumbSize.Height());

    const auto t0 = std::chrono::system_clock::now().time_since_epoch();
    m_nDay = std::chrono::duration_cast<std::chrono::hours>(t0).count() / 24;
    if (m_nDay > officecfg::Office::Common::Misc::LastTipOfTheDayShown::get())
        m_nCurrentTip++;

    UpdateTip();
}

TipOfTheDayDialog::~TipOfTheDayDialog()
{
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::LastTipOfTheDayShown::set(m_nDay, xChanges);
    officecfg::Office::Common::Misc::LastTipOfTheDayID::set(m_nCurrentTip, xChanges);
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
    constexpr sal_Int32 nNumberOfTips = SAL_N_ELEMENTS(TIPOFTHEDAY_STRINGARRAY);

    if ((m_nCurrentTip >= nNumberOfTips) || (m_nCurrentTip < 0))
        m_nCurrentTip = 0;

    //title
    m_xDialog->set_title(CuiResId(STR_TITLE)
                             .replaceFirst("%CURRENT", OUString::number(m_nCurrentTip + 1))
                             .replaceFirst("%TOTAL", OUString::number(nNumberOfTips)));

    auto[sTip, sLink, sImage] = TIPOFTHEDAY_STRINGARRAY[m_nCurrentTip];

    // text
//replace MOD1 & MOD2 shortcuts depending on platform
#ifdef MACOSX
    const OUString aMOD1 = CuiResId(STR_CMD);
    const OUString aMOD2 = CuiResId(STR_Option);
#else
    const OUString aMOD1 = CuiResId(STR_CTRL);
    const OUString aMOD2 = CuiResId(STR_Alt);
#endif
    m_pText->set_label(CuiResId(sTip).replaceAll("%MOD1", aMOD1).replaceAll("%MOD2", aMOD2));

    // hyperlink
    if (sLink.isEmpty())
    {
        m_pLink->set_visible(false);
    }
    else if (sLink.startsWith(".uno:"))
    {
        m_pLink->set_visible(false);
        //show the link only if the UNO command is available in the current module
        SfxViewFrame* pViewFrame = SfxViewFrame::Current();
        if (pViewFrame)
        {
            const auto xFrame = pViewFrame->GetFrame().GetFrameInterface();
            const css::uno::Reference<css::frame::XDispatchProvider> xDispatchProvider(
                xFrame, css::uno::UNO_QUERY);
            if (xDispatchProvider.is())
            {
                css::util::URL aCommandURL;
                aCommandURL.Complete = sLink;
                const css::uno::Reference<css::uno::XComponentContext> xContext
                    = comphelper::getProcessComponentContext();
                const css::uno::Reference<css::util::XURLTransformer> xParser
                    = css::util::URLTransformer::create(xContext);
                xParser->parseStrict(aCommandURL);
                const css::uno::Reference<css::frame::XDispatch> xDisp
                    = xDispatchProvider->queryDispatch(aCommandURL, OUString(), 0);
                if (xDisp.is())
                {
                    m_pLink->set_label(CuiResId(STR_UNO_LINK));
                    m_pLink->set_uri(sLink);

                    const OUString aModuleName(
                        vcl::CommandInfoProvider::GetModuleIdentifier(xFrame));
                    const auto aProperties
                        = vcl::CommandInfoProvider::GetCommandProperties(sLink, aModuleName);
                    m_pLink->set_tooltip_text(
                        vcl::CommandInfoProvider::GetTooltipForCommand(sLink, aProperties, xFrame));

                    m_pLink->set_visible(true);
                    m_pLink->connect_activate_link(LINK(this, TipOfTheDayDialog, OnLinkClick));
                }
            }
        }
    }
    else if (sLink.startsWith("http"))
    {
        // Links may have some %PRODUCTVERSION which need to be expanded
        OUString aText = Translate::ExpandVariables(sLink);
        OUString aLang = LanguageTag(utl::ConfigManager::getUILocale()).getLanguage();
        if (aLang == "en" || aLang == "pt" || aLang == "zh") //en-US/GB, pt-BR, zh-CH/TW
            aLang = LanguageTag(utl::ConfigManager::getUILocale()).getBcp47();
        m_pLink->set_uri(aText.replaceFirst("%LANGUAGENAME", aLang));
        m_pLink->set_label(CuiResId(STR_MORE_LINK));
        m_pLink->set_visible(true);
        m_pLink->connect_activate_link(Link<weld::LinkButton&, bool>());
    }
    else
    {
        m_pLink->set_uri(sLink);
        m_pLink->set_label(CuiResId(STR_HELP_LINK));
        m_pLink->set_visible(true);
        m_pLink->connect_activate_link(LINK(this, TipOfTheDayDialog, OnLinkClick));
    }
    // image
    OUString aURL("$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/tipoftheday/");
    rtl::Bootstrap::expandMacros(aURL);
    OUString aImageName = sImage;
    // use default image if none is available with the number
    if (aImageName.isEmpty() || !file_exists(aURL + aImageName))
        aImageName = "tipoftheday.png";
    Graphic aGraphic;
    GraphicFilter::LoadGraphic(aURL + aImageName, OUString(), aGraphic);

    if (!aGraphic.IsAnimated())
    {
        BitmapEx aBmpEx(aGraphic.GetBitmapEx());
        if (aBmpEx.Scale(ThumbSize))
            aGraphic = aBmpEx;
    }
    m_aPreview.SetPreview(aGraphic);
}

IMPL_LINK(TipOfTheDayDialog, OnLinkClick, weld::LinkButton&, rButton, bool)
{
    const OUString sLink = rButton.get_uri();
    if (sLink.startsWith(".uno:"))
    {
        comphelper::dispatchCommand(sLink, {});
        TipOfTheDayDialog::response(RET_OK);
    }
    else
    {
        Application::GetHelp()->Start(sLink, static_cast<weld::Widget*>(nullptr));
    }
    return true;
}

IMPL_LINK_NOARG(TipOfTheDayDialog, OnNextClick, weld::Button&, void)
{
    m_nCurrentTip++; //zeroed at updatetip when out of range
    UpdateTip();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
