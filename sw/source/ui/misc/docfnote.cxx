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

SwFootNoteOptionDlg::SwFootNoteOptionDlg(Window *pParent, SwWrtShell &rS)
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
    ((SwEndNoteOptionPage&)rPage).SetShell( rSh );
}

IMPL_LINK( SwFootNoteOptionDlg, OkHdl, Button *, pBtn )
{
    SfxItemSet aDummySet(rSh.GetAttrPool(), 1, 1 );
    SfxTabPage *pPage = GetTabPage( m_nFootNoteId );
    if ( pPage )
        pPage->FillItemSet( aDummySet );
    pPage = GetTabPage( m_nEndNoteId );
    if ( pPage )
        pPage->FillItemSet( aDummySet );
    aOldOkHdl.Call( pBtn );
    return 0;
}


//----------------------------------------------------------------------


SwEndNoteOptionPage::SwEndNoteOptionPage(Window *pParent, bool bEN,
    const SfxItemSet &rSet)
    : SfxTabPage(pParent,
        bEN ? OString("EndnotePage") : OString("FootnotePage"),
        bEN ? OUString("modules/swriter/ui/endnotepage.ui") : OUString("modules/swriter/ui/footnotepage.ui"),
        rSet)
    , pSh(0)
    , bPosDoc(false)
    , bEndNote(bEN)
{
    get(m_pNumViewBox, "numberinglb");
    get(m_pOffsetLbl, "offset");
    get(m_pOffsetFld, "offsetnf");
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
    get(m_pFtnCharAnchorTemplBox, "charanchorstylelb");
    get(m_pFtnCharTextTemplBox, "charstylelb");
}

void SwEndNoteOptionPage::Reset( const SfxItemSet& )
{
    SwEndNoteInfo *pInf = bEndNote ? new SwEndNoteInfo( pSh->GetEndNoteInfo() )
                                   : new SwFtnInfo( pSh->GetFtnInfo() );
    SfxObjectShell * pDocSh = SfxObjectShell::Current();
    sal_uInt16 i;

    if (PTR_CAST(SwWebDocShell, pDocSh))
        m_pStylesContainer->Hide();

    if ( bEndNote )
    {
        bPosDoc = true;
    }
    else
    {
        const SwFtnInfo &rInf = pSh->GetFtnInfo();
        // set position (page, chapter)
        if ( rInf.ePos == FTNPOS_PAGE )
        {
            m_pPosPageBox->Check();
            m_pPageTemplLbl->Enable(sal_False);
            m_pPageTemplBox->Enable(sal_False);
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
    m_pNumViewBox->SelectNumberingType( pInf->aFmt.GetNumberingType());
    m_pOffsetFld->SetValue(pInf->nFtnOffset + 1);
    m_pPrefixED->SetText(pInf->GetPrefix().replaceAll("\t", "\\t")); // fdo#65666
    m_pSuffixED->SetText(pInf->GetSuffix().replaceAll("\t", "\\t"));

    const SwCharFmt* pCharFmt = pInf->GetCharFmt(
                        *pSh->GetView().GetDocShell()->GetDoc());
    m_pFtnCharTextTemplBox->SelectEntry(pCharFmt->GetName());
    m_pFtnCharTextTemplBox->SaveValue();

    pCharFmt = pInf->GetAnchorCharFmt( *pSh->GetDoc() );
    m_pFtnCharAnchorTemplBox->SelectEntry( pCharFmt->GetName() );
    m_pFtnCharAnchorTemplBox->SaveValue();

        // styles   special regions
        // paragraph
    SfxStyleSheetBasePool *const pSPool =
        pSh->GetView().GetDocShell()->GetStyleSheetPool();
    SfxStyleSheetIterator iter(pSPool, SFX_STYLE_FAMILY_PARA, SWSTYLEBIT_EXTRA);
    SfxStyleSheetBase *pStyle = iter.First();
    while(pStyle)
    {
        m_pParaTemplBox->InsertEntry(pStyle->GetName());
        pStyle = iter.Next();
    }

    String sStr;
    SwStyleNameMapper::FillUIName( static_cast< sal_uInt16 >(bEndNote ? RES_POOLCOLL_ENDNOTE
                           : RES_POOLCOLL_FOOTNOTE), sStr );
    if(LISTBOX_ENTRY_NOTFOUND == m_pParaTemplBox->GetEntryPos( sStr ) )
        m_pParaTemplBox->InsertEntry( sStr );

    SwTxtFmtColl* pColl = pInf->GetFtnTxtColl();
    if( !pColl )
        m_pParaTemplBox->SelectEntry( sStr );      // Default
    else
    {
        OSL_ENSURE(!pColl->IsDefault(), "default style for footnotes is wrong");
        const sal_uInt16 nPos = m_pParaTemplBox->GetEntryPos(pColl->GetName());
        if( LISTBOX_ENTRY_NOTFOUND != nPos )
            m_pParaTemplBox->SelectEntryPos( nPos );
        else
        {
            m_pParaTemplBox->InsertEntry(pColl->GetName());
            m_pParaTemplBox->SelectEntry(pColl->GetName());
        }
    }

        // page
    for( i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i )
        m_pPageTemplBox->InsertEntry(SwStyleNameMapper::GetUIName( i, aEmptyStr ));

    sal_uInt16 nCount = pSh->GetPageDescCnt();
    for(i = 0; i < nCount; ++i)
    {
        const SwPageDesc &rPageDesc = pSh->GetPageDesc(i);
        if(LISTBOX_ENTRY_NOTFOUND == m_pPageTemplBox->GetEntryPos(rPageDesc.GetName()))
            m_pPageTemplBox->InsertEntry(rPageDesc.GetName());
    }

    m_pPageTemplBox->SelectEntry( pInf->GetPageDesc( *pSh->GetDoc() )->GetName());
    delete pInf;
}

SwEndNoteOptionPage::~SwEndNoteOptionPage()
{
}

SfxTabPage *SwEndNoteOptionPage::Create( Window *pParent, const SfxItemSet &rSet )
{
    return new SwEndNoteOptionPage( pParent, true, rSet );
}

/*------------------------------------------------------------------------
 Description:  Different kinds of numbering; because the Listbox has
               varying numbers of entries, here are functions to
               set and query the intended kind of numbering.
------------------------------------------------------------------------*/
void SwEndNoteOptionPage::SelectNumbering(int eNum)
{
    String sSelect;
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
    const sal_uInt16 nPos = m_pNumCountBox->GetSelectEntryPos();
    return (int) bPosDoc? nPos + 1: nPos;
}

void SwEndNoteOptionPage::SetShell( SwWrtShell &rShell )
{
    pSh = &rShell;
    // collect character templates
    m_pFtnCharTextTemplBox->Clear();
    m_pFtnCharAnchorTemplBox->Clear();
    ::FillCharStyleListBox(*m_pFtnCharTextTemplBox,
                        pSh->GetView().GetDocShell());

    ::FillCharStyleListBox(*m_pFtnCharAnchorTemplBox,
                        pSh->GetView().GetDocShell());
}

/*------------------------------------------------------------------------
 Description:  Handler behind the button to collect the footnote at the
               page. In this case all kinds of numbering can be used.
------------------------------------------------------------------------*/


IMPL_LINK_NOARG(SwEndNoteOptionPage, PosPageHdl)
{
    const SwFtnNum eNum = (const SwFtnNum)GetNumbering();
    bPosDoc = false;
    if(LISTBOX_ENTRY_NOTFOUND == m_pNumCountBox->GetEntryPos(aNumPage))
    {
        m_pNumCountBox->InsertEntry(aNumPage, FTNNUM_PAGE);
        m_pNumCountBox->InsertEntry(aNumChapter, FTNNUM_CHAPTER);
        SelectNumbering(eNum);
    }
    m_pPageTemplLbl->Enable(sal_False);
    m_pPageTemplBox->Enable(sal_False);

    return 0;
}

IMPL_LINK_NOARG(SwEndNoteOptionPage, NumCountHdl)
{
    sal_Bool bEnable = sal_True;
    if( m_pNumCountBox->GetEntryCount() - 1 != m_pNumCountBox->GetSelectEntryPos() )
    {
        bEnable = sal_False;
        m_pOffsetFld->SetValue(1);
    }
    m_pOffsetLbl->Enable(bEnable);
    m_pOffsetFld->Enable(bEnable);
    return 0;
}

/*------------------------------------------------------------------------
 Description:  Handler behind the button to collect the footnote at the
               chapter or end of the document. In this case no pagewise
               numbering can be used.
------------------------------------------------------------------------*/


IMPL_LINK_NOARG_INLINE_START(SwEndNoteOptionPage, PosChapterHdl)
{
    if ( !bPosDoc )
        SelectNumbering(FTNNUM_DOC);

    bPosDoc = true;
    m_pNumCountBox->RemoveEntry(aNumPage);
    m_pNumCountBox->RemoveEntry(aNumChapter);
    m_pPageTemplLbl->Enable();
    m_pPageTemplBox->Enable();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwEndNoteOptionPage, PosChapterHdl)

static SwCharFmt* lcl_GetCharFormat( SwWrtShell* pSh, const String& rCharFmtName )
{
    SwCharFmt* pFmt = 0;
    sal_uInt16 nChCount = pSh->GetCharFmtCount();
    for(sal_uInt16 i = 0; i< nChCount; i++)
    {
        SwCharFmt& rChFmt = pSh->GetCharFmt(i);
        if(rChFmt.GetName() == rCharFmtName )
        {
            pFmt = &rChFmt;
            break;
        }
    }
    if(!pFmt)
    {
        SfxStyleSheetBasePool* pPool = pSh->GetView().GetDocShell()->GetStyleSheetPool();
        SfxStyleSheetBase* pBase;
        pBase = pPool->Find(rCharFmtName, SFX_STYLE_FAMILY_CHAR);
        if(!pBase)
            pBase = &pPool->Make(rCharFmtName, SFX_STYLE_FAMILY_CHAR);
        pFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();
    }
    return pFmt;
}

sal_Bool SwEndNoteOptionPage::FillItemSet( SfxItemSet & )
{
    SwEndNoteInfo *pInf = bEndNote ? new SwEndNoteInfo() : new SwFtnInfo();

    pInf->nFtnOffset = static_cast< sal_uInt16 >(m_pOffsetFld->GetValue() -1);
    pInf->aFmt.SetNumberingType(m_pNumViewBox->GetSelectedNumberingType() );
    pInf->SetPrefix(m_pPrefixED->GetText().replaceAll("\\t", "\t"));
    pInf->SetSuffix(m_pSuffixED->GetText().replaceAll("\\t", "\t"));

    pInf->SetCharFmt( lcl_GetCharFormat( pSh,
                        m_pFtnCharTextTemplBox->GetSelectEntry() ) );
    pInf->SetAnchorCharFmt( lcl_GetCharFormat( pSh,
                        m_pFtnCharAnchorTemplBox->GetSelectEntry() ) );

    // paragraph template
    sal_uInt16 nPos = m_pParaTemplBox->GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        const String aFmtName( m_pParaTemplBox->GetSelectEntry() );
        SwTxtFmtColl *pColl = pSh->GetParaStyle(aFmtName, SwWrtShell::GETSTYLE_CREATEANY);
        OSL_ENSURE(pColl, "paragraph style not found");
        pInf->SetFtnTxtColl(*pColl);
    }

    // page template
    pInf->ChgPageDesc( pSh->FindPageDescByName(
                                m_pPageTemplBox->GetSelectEntry(), sal_True ) );

    if ( bEndNote )
    {
        if ( !(*pInf == pSh->GetEndNoteInfo()) )
            pSh->SetEndNoteInfo( *pInf );
    }
    else
    {
        SwFtnInfo *pI = (SwFtnInfo*)pInf;
        pI->ePos = m_pPosPageBox->IsChecked() ? FTNPOS_PAGE : FTNPOS_CHAPTER;
        pI->eNum = (SwFtnNum)GetNumbering();
        pI->aQuoVadis = m_pContEdit->GetText();
        pI->aErgoSum = m_pContFromEdit->GetText();
        if ( !(*pI == pSh->GetFtnInfo()) )
            pSh->SetFtnInfo( *pI );
    }
    delete pInf;
    return sal_True;
}

SwFootNoteOptionPage::SwFootNoteOptionPage( Window *pParent, const SfxItemSet &rSet ) :
    SwEndNoteOptionPage( pParent, false, rSet )
{
}

SwFootNoteOptionPage::~SwFootNoteOptionPage()
{
}

SfxTabPage *SwFootNoteOptionPage::Create(Window *pParent, const SfxItemSet &rSet )
{
    return new SwFootNoteOptionPage( pParent, rSet );
}







/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
