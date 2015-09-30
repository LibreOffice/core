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
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <docfnote.hxx>
#include <impfnote.hxx>
#include <ftninfo.hxx>
#include <fmtcol.hxx>
#include <pagedesc.hxx>
#include <charfmt.hxx>
#include <docstyle.hxx>
#include <wdocsh.hxx>
#include <uitool.hxx>
#include <poolfmt.hxx>
#include <swstyle.h>
#include <helpid.h>
#include <misc.hrc>
#include <frmui.hrc>
#include <SwStyleNameMapper.hxx>
#include <memory>

SwFootNoteOptionDlg::SwFootNoteOptionDlg(vcl::Window *pParent, SwWrtShell &rS)
    : SfxTabDialog(pParent, "FootEndnoteDialog", "modules/swriter/ui/footendnotedialog.ui")
    , rSh( rS )
{
    RemoveResetButton();

    aOldOkHdl = GetOKButton().GetClickHdl();
    GetOKButton().SetClickHdl( LINK( this, SwFootNoteOptionDlg, OkHdl ) );

    m_nFootNoteId = AddTabPage( "footnotes", SwFootNoteOptionPage::Create, 0 );
    m_nEndNoteId = AddTabPage( "endnotes",  SwEndNoteOptionPage::Create, 0 );
}

void SwFootNoteOptionDlg::PageCreated( sal_uInt16 /*nId*/, SfxTabPage &rPage )
{
    static_cast<SwEndNoteOptionPage&>(rPage).SetShell( rSh );
}

IMPL_LINK_TYPED( SwFootNoteOptionDlg, OkHdl, Button *, pBtn, void )
{
    SfxItemSet aDummySet(rSh.GetAttrPool(), 1, 1 );
    SfxTabPage *pPage = GetTabPage( m_nFootNoteId );
    if ( pPage )
        pPage->FillItemSet( &aDummySet );
    pPage = GetTabPage( m_nEndNoteId );
    if ( pPage )
        pPage->FillItemSet( &aDummySet );
    aOldOkHdl.Call( pBtn );
}

SwEndNoteOptionPage::SwEndNoteOptionPage(vcl::Window *pParent, bool bEN,
    const SfxItemSet &rSet)
    : SfxTabPage(pParent,
        bEN ? OString("EndnotePage") : OString("FootnotePage"),
        bEN ? OUString("modules/swriter/ui/endnotepage.ui") : OUString("modules/swriter/ui/footnotepage.ui"),
        &rSet)
    , m_pNumCountBox(NULL)
    , m_pPosFT(NULL)
    , m_pPosPageBox(NULL)
    , m_pPosChapterBox(NULL)
    , m_pContEdit(NULL)
    , m_pContFromEdit(NULL)
    , pSh(0)
    , bPosDoc(false)
    , bEndNote(bEN)
{
    get(m_pNumViewBox, "numberinglb");
    get(m_pOffsetLbl, "offset");
    get(m_pOffsetField, "offsetnf");
    get(m_pPrefixED, "prefix");
    get(m_pSuffixED, "suffix");

    if (!bEndNote)
    {
        get(m_pNumCountBox, "countinglb");
        m_pNumCountBox->SetSelectHdl(LINK(this, SwEndNoteOptionPage, NumCountHdl));
        aNumDoc = m_pNumCountBox->GetEntry(FTNNUM_DOC);
        aNumPage = m_pNumCountBox->GetEntry(FTNNUM_PAGE);
        aNumChapter = m_pNumCountBox->GetEntry(FTNNUM_CHAPTER);
        get(m_pPosPageBox, "pospagecb");
        m_pPosPageBox->SetClickHdl(LINK(this, SwEndNoteOptionPage, PosPageHdl));
        get(m_pPosChapterBox, "posdoccb");
        m_pPosChapterBox->SetClickHdl(LINK(this, SwEndNoteOptionPage, PosChapterHdl));
        get(m_pPosFT, "pos");
        m_pPosPageBox->SetAccessibleRelationMemberOf(m_pPosFT);
        m_pPosChapterBox->SetAccessibleRelationMemberOf(m_pPosFT);
        get(m_pContEdit, "conted");
        get(m_pContFromEdit, "contfromed");
    }

    get(m_pStylesContainer, "allstyles");
    get(m_pParaTemplBox, "parastylelb");
    get(m_pPageTemplLbl, "pagestyleft");
    get(m_pPageTemplBox, "pagestylelb");
    get(m_pFootnoteCharAnchorTemplBox, "charanchorstylelb");
    get(m_pFootnoteCharTextTemplBox, "charstylelb");
}

SwEndNoteOptionPage::~SwEndNoteOptionPage()
{
    disposeOnce();
}

void SwEndNoteOptionPage::dispose()
{
    m_pNumViewBox.clear();
    m_pOffsetLbl.clear();
    m_pOffsetField.clear();
    m_pNumCountBox.clear();
    m_pPrefixED.clear();
    m_pSuffixED.clear();
    m_pPosFT.clear();
    m_pPosPageBox.clear();
    m_pPosChapterBox.clear();
    m_pStylesContainer.clear();
    m_pParaTemplBox.clear();
    m_pPageTemplLbl.clear();
    m_pPageTemplBox.clear();
    m_pFootnoteCharAnchorTemplBox.clear();
    m_pFootnoteCharTextTemplBox.clear();
    m_pContEdit.clear();
    m_pContFromEdit.clear();
    SfxTabPage::dispose();
}

void SwEndNoteOptionPage::Reset( const SfxItemSet* )
{
    std::unique_ptr<SwEndNoteInfo> pInf(bEndNote ? new SwEndNoteInfo( pSh->GetEndNoteInfo() )
                                          : new SwFootnoteInfo( pSh->GetFootnoteInfo() ));
    SfxObjectShell * pDocSh = SfxObjectShell::Current();

    if (dynamic_cast<SwWebDocShell*>( pDocSh) )
        m_pStylesContainer->Hide();

    if ( bEndNote )
    {
        bPosDoc = true;
    }
    else
    {
        const SwFootnoteInfo &rInf = pSh->GetFootnoteInfo();
        // set position (page, chapter)
        if ( rInf.ePos == FTNPOS_PAGE )
        {
            m_pPosPageBox->Check();
            m_pPageTemplLbl->Enable(false);
            m_pPageTemplBox->Enable(false);
        }
        else
        {
            m_pPosChapterBox->Check();
            m_pNumCountBox->RemoveEntry(aNumPage);
            m_pNumCountBox->RemoveEntry(aNumChapter);
            bPosDoc = true;
        }
            // reference tests
        m_pContEdit->SetText(rInf.aQuoVadis);
        m_pContFromEdit->SetText(rInf.aErgoSum);

            // collected
        SelectNumbering(rInf.eNum);
    }

        // numbering
        // art
    m_pNumViewBox->SelectNumberingType( pInf->aFormat.GetNumberingType());
    m_pOffsetField->SetValue(pInf->nFootnoteOffset + 1);
    m_pPrefixED->SetText(pInf->GetPrefix().replaceAll("\t", "\\t")); // fdo#65666
    m_pSuffixED->SetText(pInf->GetSuffix().replaceAll("\t", "\\t"));

    const SwCharFormat* pCharFormat = pInf->GetCharFormat(
                        *pSh->GetView().GetDocShell()->GetDoc());
    m_pFootnoteCharTextTemplBox->SelectEntry(pCharFormat->GetName());
    m_pFootnoteCharTextTemplBox->SaveValue();

    pCharFormat = pInf->GetAnchorCharFormat( *pSh->GetDoc() );
    m_pFootnoteCharAnchorTemplBox->SelectEntry( pCharFormat->GetName() );
    m_pFootnoteCharAnchorTemplBox->SaveValue();

        // styles   special regions
        // paragraph
    SfxStyleSheetBasePool* pStyleSheetPool = pSh->GetView().GetDocShell()->GetStyleSheetPool();
    pStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, SWSTYLEBIT_EXTRA);
    SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
    while(pStyle)
    {
        m_pParaTemplBox->InsertEntry(pStyle->GetName());
        pStyle = pStyleSheetPool->Next();
    }

    OUString sStr;
    SwStyleNameMapper::FillUIName( static_cast< sal_uInt16 >(bEndNote ? RES_POOLCOLL_ENDNOTE
                           : RES_POOLCOLL_FOOTNOTE), sStr );
    if(LISTBOX_ENTRY_NOTFOUND == m_pParaTemplBox->GetEntryPos( sStr ) )
        m_pParaTemplBox->InsertEntry( sStr );

    SwTextFormatColl* pColl = pInf->GetFootnoteTextColl();
    if( !pColl )
        m_pParaTemplBox->SelectEntry( sStr );      // Default
    else
    {
        OSL_ENSURE(!pColl->IsDefault(), "default style for footnotes is wrong");
        const sal_Int32 nPos = m_pParaTemplBox->GetEntryPos(pColl->GetName());
        if( LISTBOX_ENTRY_NOTFOUND != nPos )
            m_pParaTemplBox->SelectEntryPos( nPos );
        else
        {
            m_pParaTemplBox->InsertEntry(pColl->GetName());
            m_pParaTemplBox->SelectEntry(pColl->GetName());
        }
    }

    // page
    for( sal_uInt16 i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i )
        m_pPageTemplBox->InsertEntry(SwStyleNameMapper::GetUIName( i, OUString() ));

    const size_t nCount = pSh->GetPageDescCnt();
    for(size_t i = 0; i < nCount; ++i)
    {
        const SwPageDesc &rPageDesc = pSh->GetPageDesc(i);
        if(LISTBOX_ENTRY_NOTFOUND == m_pPageTemplBox->GetEntryPos(rPageDesc.GetName()))
            m_pPageTemplBox->InsertEntry(rPageDesc.GetName());
    }

    m_pPageTemplBox->SelectEntry( pInf->GetPageDesc( *pSh->GetDoc() )->GetName());
}

VclPtr<SfxTabPage> SwEndNoteOptionPage::Create( vcl::Window *pParent, const SfxItemSet *rSet )
{
    return VclPtr<SwEndNoteOptionPage>::Create( pParent, true, *rSet );
}

// Different kinds of numbering; because the Listbox has varying numbers of
// entries, here are functions to set and query the intended kind of numbering.
void SwEndNoteOptionPage::SelectNumbering(int eNum)
{
    OUString sSelect;
    switch(eNum)
    {
        case FTNNUM_DOC:
            sSelect = aNumDoc;
        break;
        case FTNNUM_PAGE:
            sSelect = aNumPage;
        break;
        case FTNNUM_CHAPTER:
            sSelect = aNumChapter;
        break;
#if OSL_DEBUG_LEVEL > 0
        default:
            OSL_FAIL("Which numbering type?");
#endif
    }
    m_pNumCountBox->SelectEntry(sSelect);
    NumCountHdl(m_pNumCountBox);
}

int SwEndNoteOptionPage::GetNumbering() const
{
    const sal_Int32 nPos = m_pNumCountBox->GetSelectEntryPos();
    return (int) bPosDoc? nPos + 1: nPos;
}

void SwEndNoteOptionPage::SetShell( SwWrtShell &rShell )
{
    pSh = &rShell;
    // collect character templates
    m_pFootnoteCharTextTemplBox->Clear();
    m_pFootnoteCharAnchorTemplBox->Clear();
    ::FillCharStyleListBox(*m_pFootnoteCharTextTemplBox,
                        pSh->GetView().GetDocShell());

    ::FillCharStyleListBox(*m_pFootnoteCharAnchorTemplBox,
                        pSh->GetView().GetDocShell());
}

// Handler behind the button to collect the footnote at the page. In this case
// all kinds of numbering can be used.
IMPL_LINK_NOARG_TYPED(SwEndNoteOptionPage, PosPageHdl, Button*, void)
{
    const SwFootnoteNum eNum = (const SwFootnoteNum)GetNumbering();
    bPosDoc = false;
    if(LISTBOX_ENTRY_NOTFOUND == m_pNumCountBox->GetEntryPos(aNumPage))
    {
        m_pNumCountBox->InsertEntry(aNumPage, FTNNUM_PAGE);
        m_pNumCountBox->InsertEntry(aNumChapter, FTNNUM_CHAPTER);
        SelectNumbering(eNum);
    }
    m_pPageTemplLbl->Enable(false);
    m_pPageTemplBox->Enable(false);
}

IMPL_LINK_NOARG(SwEndNoteOptionPage, NumCountHdl)
{
    bool bEnable = true;
    if( m_pNumCountBox->GetEntryCount() - 1 != m_pNumCountBox->GetSelectEntryPos() )
    {
        bEnable = false;
        m_pOffsetField->SetValue(1);
    }
    m_pOffsetLbl->Enable(bEnable);
    m_pOffsetField->Enable(bEnable);
    return 0;
}

// Handler behind the button to collect the footnote at the chapter or end of
// the document. In this case no pagewise numbering can be used.
IMPL_LINK_NOARG_TYPED(SwEndNoteOptionPage, PosChapterHdl, Button*, void)
{
    if ( !bPosDoc )
        SelectNumbering(FTNNUM_DOC);

    bPosDoc = true;
    m_pNumCountBox->RemoveEntry(aNumPage);
    m_pNumCountBox->RemoveEntry(aNumChapter);
    m_pPageTemplLbl->Enable();
    m_pPageTemplBox->Enable();
}

static SwCharFormat* lcl_GetCharFormat( SwWrtShell* pSh, const OUString& rCharFormatName )
{
    SwCharFormat* pFormat = 0;
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
        pBase = pPool->Find(rCharFormatName, SFX_STYLE_FAMILY_CHAR);
        if(!pBase)
            pBase = &pPool->Make(rCharFormatName, SFX_STYLE_FAMILY_CHAR);
        pFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat();
    }
    return pFormat;
}

bool SwEndNoteOptionPage::FillItemSet( SfxItemSet * )
{
    std::unique_ptr<SwEndNoteInfo> pInf(bEndNote ? new SwEndNoteInfo() : new SwFootnoteInfo());

    pInf->nFootnoteOffset = static_cast< sal_uInt16 >(m_pOffsetField->GetValue() -1);
    pInf->aFormat.SetNumberingType(m_pNumViewBox->GetSelectedNumberingType() );
    pInf->SetPrefix(m_pPrefixED->GetText().replaceAll("\\t", "\t"));
    pInf->SetSuffix(m_pSuffixED->GetText().replaceAll("\\t", "\t"));

    pInf->SetCharFormat( lcl_GetCharFormat( pSh,
                        m_pFootnoteCharTextTemplBox->GetSelectEntry() ) );
    pInf->SetAnchorCharFormat( lcl_GetCharFormat( pSh,
                        m_pFootnoteCharAnchorTemplBox->GetSelectEntry() ) );

    // paragraph template
    sal_Int32 nPos = m_pParaTemplBox->GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        const OUString aFormatName( m_pParaTemplBox->GetSelectEntry() );
        SwTextFormatColl *pColl = pSh->GetParaStyle(aFormatName, SwWrtShell::GETSTYLE_CREATEANY);
        OSL_ENSURE(pColl, "paragraph style not found");
        pInf->SetFootnoteTextColl(*pColl);
    }

    // page template
    pInf->ChgPageDesc( pSh->FindPageDescByName(
                                m_pPageTemplBox->GetSelectEntry(), true ) );

    if ( bEndNote )
    {
        if ( !(*pInf == pSh->GetEndNoteInfo()) )
            pSh->SetEndNoteInfo( *pInf );
    }
    else
    {
        SwFootnoteInfo *pI = static_cast<SwFootnoteInfo*>(pInf.get());
        pI->ePos = m_pPosPageBox->IsChecked() ? FTNPOS_PAGE : FTNPOS_CHAPTER;
        pI->eNum = (SwFootnoteNum)GetNumbering();
        pI->aQuoVadis = m_pContEdit->GetText();
        pI->aErgoSum = m_pContFromEdit->GetText();
        if ( !(*pI == pSh->GetFootnoteInfo()) )
            pSh->SetFootnoteInfo( *pI );
    }
    return true;
}

SwFootNoteOptionPage::SwFootNoteOptionPage( vcl::Window *pParent, const SfxItemSet &rSet ) :
    SwEndNoteOptionPage( pParent, false, rSet )
{
}

SwFootNoteOptionPage::~SwFootNoteOptionPage()
{
}

VclPtr<SfxTabPage> SwFootNoteOptionPage::Create(vcl::Window *pParent, const SfxItemSet *rSet )
{
    return VclPtr<SwFootNoteOptionPage>::Create( pParent, *rSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
