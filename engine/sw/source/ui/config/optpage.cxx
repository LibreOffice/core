/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <utility>

#include <optpage.hxx>
#include <doc.hxx>
#include <hintids.hxx>
#include <cmdid.h>
#include <fmtcol.hxx>
#include <charatr.hxx>
#include <swtypes.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentSettingAccess.hxx>

#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <cfgitems.hxx>
#include <poolfmt.hxx>
#include <uiitems.hxx>
#include <printdata.hxx>
#include <modcfg.hxx>
#include <crstate.hxx>
#include <viewopt.hxx>
#include <globals.hrc>
#include <strings.hrc>
#include <optload.hrc>

#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/svxenum.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Writer.hxx>
#include <officecfg/Office/WriterWeb.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/macros.h>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/ctloptions.hxx>
#include <svl/eitem.hxx>
#include <svl/cjkoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/unitconv.hxx>
#include <sfx2/htmlmode.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <optload.hxx>

using namespace ::com::sun::star;


// TabPage Printer additional settings
SwAddPrinterTabPage::SwAddPrinterTabPage(weld::Container* pPage, weld::DialogController* pController,
    const SfxItemSet& rCoreSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/printoptionspage.ui"_ustr, u"PrintOptionsPage"_ustr, &rCoreSet)
    , m_sNone(SwResId(SW_STR_NONE))
    , m_bAttrModified(false)
    , m_bPreview(false)
    , m_bHTMLMode(false)
    , m_xGrfCB(m_xBuilder->weld_check_button(u"graphics"_ustr))
    , m_xGrfImg(m_xBuilder->weld_widget(u"lockgraphics"_ustr))
    , m_xCtrlFieldCB(m_xBuilder->weld_check_button(u"formcontrols"_ustr))
    , m_xCtrlFieldImg(m_xBuilder->weld_widget(u"lockformcontrols"_ustr))
    , m_xBackgroundCB(m_xBuilder->weld_check_button(u"background"_ustr))
    , m_xBackgroundImg(m_xBuilder->weld_widget(u"lockbackground"_ustr))
    , m_xBlackFontCB(m_xBuilder->weld_check_button(u"inblack"_ustr))
    , m_xBlackFontImg(m_xBuilder->weld_widget(u"lockinblack"_ustr))
    , m_xPrintHiddenTextCB(m_xBuilder->weld_check_button(u"hiddentext"_ustr))
    , m_xPrintHiddenTextImg(m_xBuilder->weld_widget(u"lockhiddentext"_ustr))
    , m_xPrintTextPlaceholderCB(m_xBuilder->weld_check_button(u"textplaceholder"_ustr))
    , m_xPrintTextPlaceholderImg(m_xBuilder->weld_widget(u"locktextplaceholder"_ustr))
    , m_xPagesFrame(m_xBuilder->weld_widget(u"pagesframe"_ustr))
    , m_xLeftPageCB(m_xBuilder->weld_check_button(u"leftpages"_ustr))
    , m_xLeftPageImg(m_xBuilder->weld_widget(u"lockleftpages"_ustr))
    , m_xRightPageCB(m_xBuilder->weld_check_button(u"rightpages"_ustr))
    , m_xRightPageImg(m_xBuilder->weld_widget(u"lockrightpages"_ustr))
    , m_xProspectCB(m_xBuilder->weld_check_button(u"brochure"_ustr))
    , m_xProspectImg(m_xBuilder->weld_widget(u"lockbrochure"_ustr))
    , m_xProspectCB_RTL(m_xBuilder->weld_check_button(u"rtl"_ustr))
    , m_xProspectImg_RTL(m_xBuilder->weld_widget(u"lockrtl"_ustr))
    , m_xCommentsFrame(m_xBuilder->weld_widget(u"commentsframe"_ustr))
    , m_xNoRB(m_xBuilder->weld_radio_button(u"none"_ustr))
    , m_xOnlyRB(m_xBuilder->weld_radio_button(u"only"_ustr))
    , m_xEndRB(m_xBuilder->weld_radio_button(u"end"_ustr))
    , m_xEndPageRB(m_xBuilder->weld_radio_button(u"endpage"_ustr))
    , m_xInMarginsRB(m_xBuilder->weld_radio_button(u"inmargins"_ustr))
    , m_xMarginsImg(m_xBuilder->weld_widget(u"lockcomments"_ustr))
    , m_xPrintEmptyPagesCB(m_xBuilder->weld_check_button(u"blankpages"_ustr))
    , m_xPrintEmptyPagesImg(m_xBuilder->weld_widget(u"lockblankpages"_ustr))
    , m_xPaperFromSetupCB(m_xBuilder->weld_check_button(u"papertray"_ustr))
    , m_xPaperFromSetupImg(m_xBuilder->weld_widget(u"lockpapertray"_ustr))
    , m_xFaxLB(m_xBuilder->weld_combo_box(u"fax"_ustr))
    , m_xFaxImg(m_xBuilder->weld_widget(u"lockfax"_ustr))
{
    Link<weld::Toggleable&,void> aLk = LINK( this, SwAddPrinterTabPage, AutoClickHdl);
    m_xGrfCB->connect_toggled( aLk );
    m_xRightPageCB->connect_toggled( aLk );
    m_xLeftPageCB->connect_toggled( aLk );
    m_xCtrlFieldCB->connect_toggled( aLk );
    m_xBackgroundCB->connect_toggled( aLk );
    m_xBlackFontCB->connect_toggled( aLk );
    m_xPrintHiddenTextCB->connect_toggled( aLk );
    m_xPrintTextPlaceholderCB->connect_toggled( aLk );
    m_xProspectCB->connect_toggled( aLk );
    m_xProspectCB_RTL->connect_toggled( aLk );
    m_xPaperFromSetupCB->connect_toggled( aLk );
    m_xPrintEmptyPagesCB->connect_toggled( aLk );
    m_xEndPageRB->connect_toggled( aLk );
    m_xInMarginsRB->connect_toggled( aLk );
    m_xEndRB->connect_toggled( aLk );
    m_xOnlyRB->connect_toggled( aLk );
    m_xNoRB->connect_toggled( aLk );
    m_xFaxLB->connect_changed( LINK( this, SwAddPrinterTabPage, SelectHdl ) );

    const SfxUInt16Item* pItem = rCoreSet.GetItemIfSet(SID_HTML_MODE, false );
    if(pItem && pItem->GetValue() & HTMLMODE_ON)
    {
        m_bHTMLMode = true;
        m_xLeftPageCB->hide();
        m_xRightPageCB->hide();
        m_xPrintHiddenTextCB->hide();
        m_xPrintTextPlaceholderCB->hide();
        m_xPrintEmptyPagesCB->hide();
    }
    m_xProspectCB_RTL->set_sensitive(false);
    SvtCTLOptions aCTLOptions;
    m_xProspectCB_RTL->set_visible(SvtCTLOptions::IsCTLFontEnabled());
}

SwAddPrinterTabPage::~SwAddPrinterTabPage()
{
}

void SwAddPrinterTabPage::SetPreview(bool bPrev)
{
    m_bPreview = bPrev;
    m_xCommentsFrame->set_sensitive(!m_bPreview);
    m_xPagesFrame->set_sensitive(!m_bPreview);
}

std::unique_ptr<SfxTabPage> SwAddPrinterTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                                const SfxItemSet* rAttrSet )
{
    return std::make_unique<SwAddPrinterTabPage>(pPage, pController, *rAttrSet);
}

bool    SwAddPrinterTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    if ( m_bAttrModified )
    {
        SwAddPrinterItem aAddPrinterAttr;
        aAddPrinterAttr.m_bPrintGraphic   = m_xGrfCB->get_active();
        aAddPrinterAttr.m_bPrintControl   = m_xCtrlFieldCB->get_active();
        aAddPrinterAttr.m_bPrintPageBackground = m_xBackgroundCB->get_active();
        aAddPrinterAttr.m_bPrintBlackFont = m_xBlackFontCB->get_active();
        aAddPrinterAttr.m_bPrintHiddenText = m_xPrintHiddenTextCB->get_active();
        aAddPrinterAttr.m_bPrintTextPlaceholder = m_xPrintTextPlaceholderCB->get_active();

        aAddPrinterAttr.m_bPrintLeftPages     = m_xLeftPageCB->get_active();
        aAddPrinterAttr.m_bPrintRightPages    = m_xRightPageCB->get_active();
        aAddPrinterAttr.m_bPrintProspect      = m_xProspectCB->get_active();
        aAddPrinterAttr.m_bPrintProspectRTL   = m_xProspectCB_RTL->get_active();
        aAddPrinterAttr.m_bPaperFromSetup     = m_xPaperFromSetupCB->get_active();
        aAddPrinterAttr.m_bPrintEmptyPages    = m_xPrintEmptyPagesCB->get_active();

        if (m_xNoRB->get_active())  aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::NONE;
        if (m_xOnlyRB->get_active()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::Only;
        if (m_xEndRB->get_active()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::EndDoc;
        if (m_xEndPageRB->get_active()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::EndPage;
        if (m_xInMarginsRB->get_active()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::InMargins;

        const OUString sFax = m_xFaxLB->get_active_text();
        aAddPrinterAttr.m_sFaxName = m_sNone == sFax ? OUString() : sFax;
        rCoreSet->Put(aAddPrinterAttr);
    }
    return m_bAttrModified;
}

void    SwAddPrinterTabPage::Reset( const SfxItemSet*  )
{
    const   SfxItemSet&         rSet = GetItemSet();
    bool bReadOnly = false;

    if( const SwAddPrinterItem* pAddPrinterAttr = rSet.GetItemIfSet( FN_PARAM_ADDPRINTER , false ) )
    {
        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Content::Graphic::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Content::Graphic::isReadOnly();
        m_xGrfCB->set_active(pAddPrinterAttr->m_bPrintGraphic);
        m_xGrfCB->set_sensitive(!bReadOnly);
        m_xGrfImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Content::Control::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Content::Control::isReadOnly();
        m_xCtrlFieldCB->set_active(       pAddPrinterAttr->m_bPrintControl);
        m_xCtrlFieldCB->set_sensitive(!bReadOnly);
        m_xCtrlFieldImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Content::Background::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Content::Background::isReadOnly();
        m_xBackgroundCB->set_active(    pAddPrinterAttr->m_bPrintPageBackground);
        m_xBackgroundCB->set_sensitive(!bReadOnly);
        m_xBackgroundImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Content::PrintBlack::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Content::PrintBlack::isReadOnly();
        m_xBlackFontCB->set_active(     pAddPrinterAttr->m_bPrintBlackFont);
        m_xBlackFontCB->set_sensitive(!bReadOnly);
        m_xBlackFontImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Print::Content::PrintHiddenText::isReadOnly();
        m_xPrintHiddenTextCB->set_active( pAddPrinterAttr->m_bPrintHiddenText);
        m_xPrintHiddenTextCB->set_sensitive(!bReadOnly);
        m_xPrintHiddenTextImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Print::Content::PrintPlaceholders::isReadOnly();
        m_xPrintTextPlaceholderCB->set_active(pAddPrinterAttr->m_bPrintTextPlaceholder);
        m_xPrintTextPlaceholderCB->set_sensitive(!bReadOnly);
        m_xPrintTextPlaceholderImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Print::Page::LeftPage::isReadOnly();
        m_xLeftPageCB->set_active(      pAddPrinterAttr->m_bPrintLeftPages);
        m_xLeftPageCB->set_sensitive(!bReadOnly);
        m_xLeftPageImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Print::Page::RightPage::isReadOnly();
        m_xRightPageCB->set_active(     pAddPrinterAttr->m_bPrintRightPages);
        m_xRightPageCB->set_sensitive(!bReadOnly);
        m_xRightPageImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Papertray::FromPrinterSetup::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Papertray::FromPrinterSetup::isReadOnly();
        m_xPaperFromSetupCB->set_active(pAddPrinterAttr->m_bPaperFromSetup);
        m_xPaperFromSetupCB->set_sensitive(!bReadOnly);
        m_xPaperFromSetupImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Print::EmptyPages::isReadOnly();
        m_xPrintEmptyPagesCB->set_active(pAddPrinterAttr->m_bPrintEmptyPages);
        m_xPrintEmptyPagesCB->set_sensitive(!bReadOnly);
        m_xPrintEmptyPagesImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Page::Brochure::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Page::Brochure::isReadOnly();
        m_xProspectCB->set_active(      pAddPrinterAttr->m_bPrintProspect);
        m_xProspectCB->set_sensitive(!bReadOnly);
        m_xProspectImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Page::BrochureRightToLeft::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Page::BrochureRightToLeft::isReadOnly();
        m_xProspectCB_RTL->set_active(      pAddPrinterAttr->m_bPrintProspectRTL);
        m_xProspectCB_RTL->set_sensitive(!bReadOnly);
        m_xProspectImg_RTL->set_visible(bReadOnly);

        m_xNoRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::NONE ) ;
        m_xOnlyRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::Only ) ;
        m_xEndRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::EndDoc ) ;
        m_xEndPageRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::EndPage ) ;
        m_xInMarginsRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::InMargins ) ;

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Content::Note::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Content::Note::isReadOnly();
        m_xNoRB->set_sensitive(!bReadOnly);
        m_xOnlyRB->set_sensitive(!bReadOnly);
        m_xEndRB->set_sensitive(!bReadOnly);
        m_xEndPageRB->set_sensitive(!bReadOnly);
        m_xInMarginsRB->set_sensitive(!bReadOnly);
        m_xMarginsImg->set_visible(bReadOnly);

        auto nFound = m_xFaxLB->find_text(pAddPrinterAttr->m_sFaxName);
        if (nFound != -1)
            m_xFaxLB->set_active(nFound);
        else if (m_xFaxLB->get_count())
            m_xFaxLB->set_active(0);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Output::Fax::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Output::Fax::isReadOnly();
        m_xFaxLB->set_sensitive(!bReadOnly);
        m_xFaxImg->set_visible(bReadOnly);
    }
    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Page::BrochureRightToLeft::isReadOnly() :
        officecfg::Office::WriterWeb::Print::Page::BrochureRightToLeft::isReadOnly();
    if (m_xProspectCB->get_active())
    {
        m_xProspectCB_RTL->set_sensitive(!bReadOnly);
        m_xNoRB->set_sensitive( false );
        m_xOnlyRB->set_sensitive( false );
        m_xEndRB->set_sensitive( false );
        m_xEndPageRB->set_sensitive( false );
    }
    else
        m_xProspectCB_RTL->set_sensitive( false );

    m_xProspectImg_RTL->set_visible(bReadOnly);
}

IMPL_LINK_NOARG(SwAddPrinterTabPage, AutoClickHdl, weld::Toggleable&, void)
{
    m_bAttrModified = true;
    bool bIsProspect = m_xProspectCB->get_active();
    if (!bIsProspect)
        m_xProspectCB_RTL->set_active( false );
    m_xProspectCB_RTL->set_sensitive( bIsProspect );
    m_xNoRB->set_sensitive( !bIsProspect );
    m_xOnlyRB->set_sensitive( !bIsProspect );
    m_xEndRB->set_sensitive( !bIsProspect );
    m_xEndPageRB->set_sensitive( !bIsProspect );
    m_xInMarginsRB->set_sensitive( !bIsProspect );
}

void  SwAddPrinterTabPage::SetFax( const std::vector<OUString>& rFaxLst )
{
    m_xFaxLB->append_text(m_sNone);
    for(const auto & i : rFaxLst)
    {
        m_xFaxLB->append_text(i);
    }
    m_xFaxLB->set_active(0);
}

IMPL_LINK_NOARG(SwAddPrinterTabPage, SelectHdl, weld::ComboBox&, void)
{
    m_bAttrModified=true;
}

void SwAddPrinterTabPage::PageCreated( const SfxAllItemSet& aSet)
{
    const SfxBoolItem* pListItem = aSet.GetItem<SfxBoolItem>(SID_FAX_LIST, false);
    const SfxBoolItem* pPreviewItem = aSet.GetItem<SfxBoolItem>(SID_PREVIEWFLAG_TYPE, false);
    if (pPreviewItem)
    {
        SetPreview(pPreviewItem->GetValue());
        Reset(&aSet);
    }
    if (pListItem && pListItem->GetValue())
    {
        std::vector<OUString> aFaxList;
        const std::vector<OUString>& rPrinters = Printer::GetPrinterQueues();
        for (const auto & rPrinter : rPrinters)
            aFaxList.insert(aFaxList.begin(), rPrinter);
        SetFax( aFaxList );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
