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

#include <svl/style.hxx>
#include <osl/diagnose.h>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <docfnote.hxx>
#include "impfnote.hxx"
#include <ftninfo.hxx>
#include <fmtcol.hxx>
#include <pagedesc.hxx>
#include <charfmt.hxx>
#include <docstyle.hxx>
#include <wdocsh.hxx>
#include <uitool.hxx>
#include <poolfmt.hxx>
#include <SwStyleNameMapper.hxx>
#include <memory>

SwFootNoteOptionDlg::SwFootNoteOptionDlg(weld::Window *pParent, SwWrtShell &rS)
    : SfxTabDialogController(pParent, "modules/swriter/ui/footendnotedialog.ui", "FootEndnoteDialog")
    , m_rSh( rS )
{
    RemoveResetButton();

    GetOKButton().connect_clicked(LINK(this, SwFootNoteOptionDlg, OkHdl));

    AddTabPage("footnotes", SwFootNoteOptionPage::Create, nullptr);
    AddTabPage("endnotes",  SwEndNoteOptionPage::Create, nullptr);
}

void SwFootNoteOptionDlg::PageCreated(const OUString& /*rId*/, SfxTabPage &rPage)
{
    static_cast<SwEndNoteOptionPage&>(rPage).SetShell(m_rSh);
}

IMPL_LINK(SwFootNoteOptionDlg, OkHdl, weld::Button&, rBtn, void)
{
    SfxItemSetFixed<1, 1> aDummySet(m_rSh.GetAttrPool());
    SfxTabPage *pPage = GetTabPage(u"footnotes");
    if ( pPage )
        pPage->FillItemSet( &aDummySet );
    pPage = GetTabPage(u"endnotes");
    if ( pPage )
        pPage->FillItemSet( &aDummySet );
    SfxTabDialogController::OkHdl(rBtn);
}

SwEndNoteOptionPage::SwEndNoteOptionPage(weld::Container* pPage, weld::DialogController* pController, bool bEN,
    const SfxItemSet &rSet)
    : SfxTabPage(pPage, pController,
        bEN ? OUString("modules/swriter/ui/endnotepage.ui") : OUString("modules/swriter/ui/footnotepage.ui"),
        bEN ? OUString("EndnotePage") : OUString("FootnotePage"),
        &rSet)
    , m_pSh(nullptr)
    , m_bPosDoc(false)
    , m_bEndNote(bEN)
    , m_xNumViewBox(new SwNumberingTypeListBox(m_xBuilder->weld_combo_box("numberinglb")))
    , m_xOffsetLbl(m_xBuilder->weld_label("offset"))
    , m_xOffsetField(m_xBuilder->weld_spin_button("offsetnf"))
    , m_xNumCountBox(m_xBuilder->weld_combo_box("countinglb"))
    , m_xPrefixED(m_xBuilder->weld_entry("prefix"))
    , m_xSuffixED(m_xBuilder->weld_entry("suffix"))
    , m_xPosPageBox(m_xBuilder->weld_radio_button("pospagecb"))
    , m_xPosChapterBox(m_xBuilder->weld_radio_button("posdoccb"))
    , m_xStylesContainer(m_xBuilder->weld_widget("allstyles"))
    , m_xParaTemplBox(m_xBuilder->weld_combo_box("parastylelb"))
    , m_xPageTemplLbl(m_xBuilder->weld_label("pagestyleft"))
    , m_xPageTemplBox(m_xBuilder->weld_combo_box("pagestylelb"))
    , m_xFootnoteCharAnchorTemplBox(m_xBuilder->weld_combo_box("charanchorstylelb"))
    , m_xFootnoteCharTextTemplBox(m_xBuilder->weld_combo_box("charstylelb"))
    , m_xContEdit(m_xBuilder->weld_entry("conted"))
    , m_xContFromEdit(m_xBuilder->weld_entry("contfromed"))
{
    m_xNumViewBox->Reload(SwInsertNumTypes::Extended);
    if (!m_bEndNote)
    {
        m_xNumCountBox->connect_changed(LINK(this, SwEndNoteOptionPage, NumCountHdl));
        m_aNumDoc = m_xNumCountBox->get_text(FTNNUM_DOC);
        m_aNumPage = m_xNumCountBox->get_text(FTNNUM_PAGE);
        m_aNumChapter = m_xNumCountBox->get_text(FTNNUM_CHAPTER);
        m_xPosPageBox->connect_toggled(LINK(this, SwEndNoteOptionPage, ToggleHdl));
        m_xPosChapterBox->connect_toggled(LINK(this, SwEndNoteOptionPage, ToggleHdl));
    }
    m_xParaTemplBox->make_sorted();
}

SwEndNoteOptionPage::~SwEndNoteOptionPage()
{
}

void SwEndNoteOptionPage::Reset( const SfxItemSet* )
{
    std::unique_ptr<SwEndNoteInfo> pInf(m_bEndNote ? new SwEndNoteInfo( m_pSh->GetEndNoteInfo() )
                                          : new SwFootnoteInfo( m_pSh->GetFootnoteInfo() ));
    SfxObjectShell * pDocSh = SfxObjectShell::Current();

    if (dynamic_cast<SwWebDocShell*>( pDocSh) )
        m_xStylesContainer->hide();

    if ( m_bEndNote )
    {
        m_bPosDoc = true;
    }
    else
    {
        const SwFootnoteInfo &rInf = m_pSh->GetFootnoteInfo();
        // set position (page, chapter)
        if ( rInf.m_ePos == FTNPOS_PAGE )
        {
            m_xPosPageBox->set_active(true);
            m_xPageTemplLbl->set_sensitive(false);
            m_xPageTemplBox->set_sensitive(false);
        }
        else
        {
            m_xPosChapterBox->set_active(true);
            m_xNumCountBox->remove_text(m_aNumPage);
            m_xNumCountBox->remove_text(m_aNumChapter);
            m_bPosDoc = true;
        }
            // reference tests
        m_xContEdit->set_text(rInf.m_aQuoVadis);
        m_xContFromEdit->set_text(rInf.m_aErgoSum);

            // collected
        SelectNumbering(rInf.m_eNum);
    }

        // numbering
        // art
    m_xNumViewBox->SelectNumberingType( pInf->m_aFormat.GetNumberingType());
    m_xOffsetField->set_value(pInf->m_nFootnoteOffset + 1);
    m_xPrefixED->set_text(pInf->GetPrefix().replaceAll("\t", "\\t")); // fdo#65666
    m_xSuffixED->set_text(pInf->GetSuffix().replaceAll("\t", "\\t"));

    const SwCharFormat* pCharFormat = pInf->GetCharFormat(
                        *m_pSh->GetView().GetDocShell()->GetDoc());
    m_xFootnoteCharTextTemplBox->set_active_text(pCharFormat->GetName());
    m_xFootnoteCharTextTemplBox->save_value();

    pCharFormat = pInf->GetAnchorCharFormat( *m_pSh->GetDoc() );
    m_xFootnoteCharAnchorTemplBox->set_active_text( pCharFormat->GetName() );
    m_xFootnoteCharAnchorTemplBox->save_value();

        // styles   special regions
        // paragraph
    SfxStyleSheetBasePool* pStyleSheetPool = m_pSh->GetView().GetDocShell()->GetStyleSheetPool();
    SfxStyleSheetBase *pStyle = pStyleSheetPool->First(SfxStyleFamily::Para, SfxStyleSearchBits::SwExtra);
    while(pStyle)
    {
        m_xParaTemplBox->append_text(pStyle->GetName());
        pStyle = pStyleSheetPool->Next();
    }

    OUString sStr;
    SwStyleNameMapper::FillUIName( static_cast< sal_uInt16 >(m_bEndNote ? RES_POOLCOLL_ENDNOTE
                           : RES_POOLCOLL_FOOTNOTE), sStr );
    if (m_xParaTemplBox->find_text(sStr) == -1)
        m_xParaTemplBox->append_text(sStr);

    SwTextFormatColl* pColl = pInf->GetFootnoteTextColl();
    if( !pColl )
        m_xParaTemplBox->set_active_text(sStr);      // Default
    else
    {
        OSL_ENSURE(!pColl->IsDefault(), "default style for footnotes is wrong");
        const int nPos = m_xParaTemplBox->find_text(pColl->GetName());
        if (nPos != -1)
            m_xParaTemplBox->set_active( nPos );
        else
        {
            m_xParaTemplBox->append_text(pColl->GetName());
            m_xParaTemplBox->set_active_text(pColl->GetName());
        }
    }

    // page
    for (sal_uInt16 i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i)
        m_xPageTemplBox->append_text(SwStyleNameMapper::GetUIName(i, OUString()));

    const size_t nCount = m_pSh->GetPageDescCnt();
    for(size_t i = 0; i < nCount; ++i)
    {
        const SwPageDesc &rPageDesc = m_pSh->GetPageDesc(i);
        if (m_xPageTemplBox->find_text(rPageDesc.GetName()) == -1)
            m_xPageTemplBox->append_text(rPageDesc.GetName());
    }
    m_xPageTemplBox->make_sorted();

    m_xPageTemplBox->set_active_text(pInf->GetPageDesc(*m_pSh->GetDoc())->GetName());
}

std::unique_ptr<SfxTabPage> SwEndNoteOptionPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet )
{
    return std::make_unique<SwEndNoteOptionPage>(pPage, pController, true, *rSet);
}

// Different kinds of numbering; because the Listbox has varying numbers of
// entries, here are functions to set and query the intended kind of numbering.
void SwEndNoteOptionPage::SelectNumbering(SwFootnoteNum const eNum)
{
    OUString sSelect;
    switch(eNum)
    {
        case FTNNUM_DOC:
            sSelect = m_aNumDoc;
        break;
        case FTNNUM_PAGE:
            sSelect = m_aNumPage;
        break;
        case FTNNUM_CHAPTER:
            sSelect = m_aNumChapter;
        break;
        default:
            assert(false);
    }
    m_xNumCountBox->set_active_text(sSelect);
    NumCountHdl(*m_xNumCountBox);
}

SwFootnoteNum SwEndNoteOptionPage::GetNumbering() const
{
    const int nPos = m_xNumCountBox->get_active();
    return static_cast<SwFootnoteNum>(m_bPosDoc ? nPos + 2 : nPos);
}

void SwEndNoteOptionPage::SetShell( SwWrtShell &rShell )
{
    m_pSh = &rShell;
    // collect character templates
    m_xFootnoteCharTextTemplBox->clear();
    m_xFootnoteCharAnchorTemplBox->clear();
    ::FillCharStyleListBox(*m_xFootnoteCharTextTemplBox,
                        m_pSh->GetView().GetDocShell(), true);

    ::FillCharStyleListBox(*m_xFootnoteCharAnchorTemplBox,
                        m_pSh->GetView().GetDocShell(), true);
}

IMPL_LINK(SwEndNoteOptionPage, ToggleHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;

    if (m_xPosPageBox->get_active())
    {
        // Handler behind the button to collect the footnote at the page. In this case
        // all kinds of numbering can be used.

        const SwFootnoteNum eNum = GetNumbering();
        m_bPosDoc = false;
        if (m_xNumCountBox->find_text(m_aNumPage) == -1)
        {
            m_xNumCountBox->insert_text(FTNNUM_PAGE, m_aNumPage);
            m_xNumCountBox->insert_text(FTNNUM_CHAPTER, m_aNumChapter);
            SelectNumbering(eNum);
        }
        m_xPageTemplLbl->set_sensitive(false);
        m_xPageTemplBox->set_sensitive(false);
    }
    else if (m_xPosChapterBox->get_active())
    {
        // Handler behind the button to collect the footnote at the chapter or end of
        // the document. In this case no pagewise numbering can be used.

        if ( !m_bPosDoc )
            SelectNumbering(FTNNUM_DOC);

        m_bPosDoc = true;
        m_xNumCountBox->remove_text(m_aNumPage);
        m_xNumCountBox->remove_text(m_aNumChapter);
        m_xPageTemplLbl->set_sensitive(true);
        m_xPageTemplBox->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(SwEndNoteOptionPage, NumCountHdl, weld::ComboBox&, void)
{
    bool bEnable = true;
    if (m_xNumCountBox->get_count() - 1 != m_xNumCountBox->get_active())
    {
        bEnable = false;
        m_xOffsetField->set_value(1);
    }
    m_xOffsetLbl->set_sensitive(bEnable);
    m_xOffsetField->set_sensitive(bEnable);
}

static SwCharFormat* lcl_GetCharFormat( SwWrtShell* pSh, const OUString& rCharFormatName )
{
    SwCharFormat* pFormat = nullptr;
    const sal_uInt16 nChCount = pSh->GetCharFormatCount();
    for(sal_uInt16 i = 0; i< nChCount; i++)
    {
        SwCharFormat& rChFormat = pSh->GetCharFormat(i);
        if(rChFormat.GetName() == rCharFormatName )
        {
            pFormat = &rChFormat;
            break;
        }
    }
    if(!pFormat)
    {
        SfxStyleSheetBasePool* pPool = pSh->GetView().GetDocShell()->GetStyleSheetPool();
        SfxStyleSheetBase* pBase;
        pBase = pPool->Find(rCharFormatName, SfxStyleFamily::Char);
        if(!pBase)
            pBase = &pPool->Make(rCharFormatName, SfxStyleFamily::Char);
        pFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat();
    }
    return pFormat;
}

bool SwEndNoteOptionPage::FillItemSet( SfxItemSet * )
{
    std::unique_ptr<SwEndNoteInfo> pInf(m_bEndNote ? new SwEndNoteInfo() : new SwFootnoteInfo());

    pInf->m_nFootnoteOffset = m_xOffsetField->get_value() - 1;
    pInf->m_aFormat.SetNumberingType(m_xNumViewBox->GetSelectedNumberingType() );
    pInf->SetPrefix(m_xPrefixED->get_text().replaceAll("\\t", "\t"));
    pInf->SetSuffix(m_xSuffixED->get_text().replaceAll("\\t", "\t"));

    pInf->SetCharFormat( lcl_GetCharFormat( m_pSh,
                        m_xFootnoteCharTextTemplBox->get_active_text() ) );
    pInf->SetAnchorCharFormat( lcl_GetCharFormat( m_pSh,
                        m_xFootnoteCharAnchorTemplBox->get_active_text() ) );

    // paragraph template
    int nPos = m_xParaTemplBox->get_active();
    if (nPos != -1)
    {
        const OUString aFormatName( m_xParaTemplBox->get_active_text() );
        SwTextFormatColl *pColl = m_pSh->GetParaStyle(aFormatName, SwWrtShell::GETSTYLE_CREATEANY);
        assert(pColl && "paragraph style not found");
        pInf->SetFootnoteTextColl(*pColl);
    }

    // page template
    pInf->ChgPageDesc( m_pSh->FindPageDescByName(
                                m_xPageTemplBox->get_active_text(), true ) );

    if ( m_bEndNote )
    {
        if ( !(*pInf == m_pSh->GetEndNoteInfo()) )
            m_pSh->SetEndNoteInfo( *pInf );
    }
    else
    {
        SwFootnoteInfo *pI = static_cast<SwFootnoteInfo*>(pInf.get());
        pI->m_ePos = m_xPosPageBox->get_active() ? FTNPOS_PAGE : FTNPOS_CHAPTER;
        pI->m_eNum = GetNumbering();
        pI->m_aQuoVadis = m_xContEdit->get_text();
        pI->m_aErgoSum = m_xContFromEdit->get_text();
        if ( !(*pI == m_pSh->GetFootnoteInfo()) )
            m_pSh->SetFootnoteInfo( *pI );
    }
    return true;
}

SwFootNoteOptionPage::SwFootNoteOptionPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet)
    : SwEndNoteOptionPage(pPage, pController, false, rSet)
{
}

SwFootNoteOptionPage::~SwFootNoteOptionPage()
{
}

std::unique_ptr<SfxTabPage> SwFootNoteOptionPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet )
{
    return std::make_unique<SwFootNoteOptionPage>(pPage, pController, *rSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
