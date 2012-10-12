/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <docfnote.hrc>
#include <frmui.hrc>
#include <SwStyleNameMapper.hxx>

SwFootNoteOptionDlg::SwFootNoteOptionDlg( Window *pParent, SwWrtShell &rS ) :
    SfxTabDialog( pParent, SW_RES(DLG_DOC_FOOTNOTE) ),
    rSh( rS )
{
    FreeResource();
    RemoveResetButton();

    aOldOkHdl = GetOKButton().GetClickHdl();
    GetOKButton().SetClickHdl( LINK( this, SwFootNoteOptionDlg, OkHdl ) );

    AddTabPage( TP_FOOTNOTEOPTION, SwFootNoteOptionPage::Create, 0 );
    AddTabPage( TP_ENDNOTEOPTION,  SwEndNoteOptionPage::Create, 0 );
}

void SwFootNoteOptionDlg::PageCreated( sal_uInt16 /*nId*/, SfxTabPage &rPage )
{
    ((SwEndNoteOptionPage&)rPage).SetShell( rSh );
}

SwFootNoteOptionDlg::~SwFootNoteOptionDlg()
{
}

IMPL_LINK( SwFootNoteOptionDlg, OkHdl, Button *, pBtn )
{
    SfxItemSet aDummySet(rSh.GetAttrPool(), 1, 1 );
    SfxTabPage *pPage = GetTabPage( TP_FOOTNOTEOPTION );
    if ( pPage )
        pPage->FillItemSet( aDummySet );
    pPage = GetTabPage( TP_ENDNOTEOPTION  );
    if ( pPage )
        pPage->FillItemSet( aDummySet );
    aOldOkHdl.Call( pBtn );
    return 0;
}


//----------------------------------------------------------------------


SwEndNoteOptionPage::SwEndNoteOptionPage( Window *pParent, sal_Bool bEN,
                                          const SfxItemSet &rSet ) :
    SfxTabPage( pParent, SW_RES(bEN ? TP_ENDNOTEOPTION : TP_FOOTNOTEOPTION), rSet ),
    aNumFL         (this, SW_RES( FL_NUM        )),

    aNumTypeFT      (this, SW_RES( FT_NUMTYPE    )),
    aNumViewBox     (this, SW_RES( LB_NUMVIEW   ), INSERT_NUM_EXTENDED_TYPES),
    aOffsetLbl      (this, SW_RES( FT_OFFSET    )),
    aOffsetFld      (this, SW_RES( FLD_OFFSET   )),
    aNumCountFT     (this, SW_RES( FT_NUMCOUNT   )),
    aNumCountBox    (this, SW_RES( LB_NUMCOUNT  )),
    aPrefixFT       (this, SW_RES( FT_PREFIX    )),
    aPrefixED       (this, SW_RES( ED_PREFIX    )),
    aSuffixFT       (this, SW_RES( FT_SUFFIX    )),
    aSuffixED       (this, SW_RES( ED_SUFFIX    )),
    aPosFT          (this, SW_RES( FT_POS    )),
    aPosPageBox     (this, SW_RES( RB_POS_PAGE   )),
    aPosChapterBox  (this, SW_RES( RB_POS_CHAPTER)),

    aTemplFL       (this, SW_RES( FL_TEMPL      )),
    aParaTemplLbl   (this, SW_RES( FT_PARA_TEMPL)),
    aParaTemplBox   (this, SW_RES( LB_PARA_TEMPL)),
    aPageTemplLbl   (this, SW_RES( FT_PAGE_TEMPL)),
    aPageTemplBox   (this, SW_RES( LB_PAGE_TEMPL)),

    aCharTemplFL(          this, SW_RES(FL_CHAR_TEMPL)),
    aFtnCharAnchorTemplLbl( this, SW_RES( FT_ANCHR_CHARFMT)),
    aFtnCharAnchorTemplBox( this, SW_RES( LB_ANCHR_CHARFMT)),
    aFtnCharTextTemplLbl(   this, SW_RES( FT_TEXT_CHARFMT)),
    aFtnCharTextTemplBox(   this, SW_RES( LB_TEXT_CHARFMT)),

    aContFL        (this, SW_RES( FL_CONT       )),
    aContLbl        (this, SW_RES( FT_CONT      )),
    aContEdit       (this, SW_RES( ED_CONT      )),
    aContFromLbl    (this, SW_RES( FT_CONT_FROM )),
    aContFromEdit   (this, SW_RES( ED_CONT_FROM )),

    aNumDoc(aNumCountBox.GetEntry(FTNNUM_DOC)),
    aNumPage(aNumCountBox.GetEntry(FTNNUM_PAGE)),
    aNumChapter(aNumCountBox.GetEntry(FTNNUM_CHAPTER)),
    pSh( 0 ),
    bPosDoc(sal_False),
    bEndNote( bEN )
{
    FreeResource();

    aPosPageBox.SetClickHdl(LINK(this, SwEndNoteOptionPage, PosPageHdl));
    aPosChapterBox.SetClickHdl(LINK(this, SwEndNoteOptionPage, PosChapterHdl));
    aNumCountBox.SetSelectHdl(LINK(this, SwEndNoteOptionPage, NumCountHdl));

    aPosPageBox.SetAccessibleRelationMemberOf(&aPosFT);
    aPosChapterBox.SetAccessibleRelationMemberOf(&aPosFT);
}

void SwEndNoteOptionPage::Reset( const SfxItemSet& )
{
    SwEndNoteInfo *pInf = bEndNote ? new SwEndNoteInfo( pSh->GetEndNoteInfo() )
                                   : new SwFtnInfo( pSh->GetFtnInfo() );
    SfxObjectShell * pDocSh = SfxObjectShell::Current();
    sal_uInt16 i;

    if(PTR_CAST(SwWebDocShell, pDocSh))
    {
        aParaTemplLbl   .Hide();
        aParaTemplBox   .Hide();
        aPageTemplLbl   .Hide();
        aPageTemplBox   .Hide();
        aFtnCharTextTemplLbl.Hide();
        aFtnCharTextTemplBox.Hide();
        aFtnCharAnchorTemplLbl.Hide();
        aFtnCharAnchorTemplBox.Hide();
        aCharTemplFL   .Hide();
        aTemplFL       .Hide();
    }
    if ( bEndNote )
    {
        aPosPageBox.Hide();
        aPosChapterBox.Hide();
        aNumCountBox.Hide();
        aContLbl.Hide();
        aContEdit.Hide();
        aContFromLbl.Hide();
        aContFromEdit.Hide();
        aContFL.Hide();
        bPosDoc = sal_True;
    }
    else
    {
        const SwFtnInfo &rInf = pSh->GetFtnInfo();
        // set position (page, chapter)
        if ( rInf.ePos == FTNPOS_PAGE )
        {
            aPosPageBox.Check();
            aPageTemplLbl.Enable(sal_False);
            aPageTemplBox.Enable(sal_False);
        }
        else
        {
            aPosChapterBox.Check();
            aNumCountBox.RemoveEntry(aNumPage);
            aNumCountBox.RemoveEntry(aNumChapter);
            bPosDoc = sal_True;
        }
            // reference tests
        aContEdit.SetText(rInf.aQuoVadis);
        aContFromEdit.SetText(rInf.aErgoSum);

            // collected
        SelectNumbering(rInf.eNum);
    }

        // numbering
        // art
    aNumViewBox.SelectNumberingType( pInf->aFmt.GetNumberingType());
    aOffsetFld.SetValue(pInf->nFtnOffset + 1);
    aPrefixED.SetText(pInf->GetPrefix());
    aSuffixED.SetText(pInf->GetSuffix());

    const SwCharFmt* pCharFmt = pInf->GetCharFmt(
                        *pSh->GetView().GetDocShell()->GetDoc());
    aFtnCharTextTemplBox.SelectEntry(pCharFmt->GetName());
    aFtnCharTextTemplBox.SaveValue();

    pCharFmt = pInf->GetAnchorCharFmt( *pSh->GetDoc() );
    aFtnCharAnchorTemplBox.SelectEntry( pCharFmt->GetName() );
    aFtnCharAnchorTemplBox.SaveValue();

        // styles   special regions
        // paragraph
    SfxStyleSheetBasePool* pStyleSheetPool = pSh->GetView().GetDocShell()->GetStyleSheetPool();
    pStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, SWSTYLEBIT_EXTRA);
    SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
    while(pStyle)
    {
        aParaTemplBox.InsertEntry(pStyle->GetName());
        pStyle = pStyleSheetPool->Next();
    }

    String sStr;
    SwStyleNameMapper::FillUIName( static_cast< sal_uInt16 >(bEndNote ? RES_POOLCOLL_ENDNOTE
                           : RES_POOLCOLL_FOOTNOTE), sStr );
    if(LISTBOX_ENTRY_NOTFOUND == aParaTemplBox.GetEntryPos( sStr ) )
        aParaTemplBox.InsertEntry( sStr );

    SwTxtFmtColl* pColl = pInf->GetFtnTxtColl();
    if( !pColl )
        aParaTemplBox.SelectEntry( sStr );      // Default
    else
    {
        OSL_ENSURE(!pColl->IsDefault(), "default style for footnotes is wrong");
        const sal_uInt16 nPos = aParaTemplBox.GetEntryPos(pColl->GetName());
        if( LISTBOX_ENTRY_NOTFOUND != nPos )
            aParaTemplBox.SelectEntryPos( nPos );
        else
        {
            aParaTemplBox.InsertEntry(pColl->GetName());
            aParaTemplBox.SelectEntry(pColl->GetName());
        }
    }

        // page
    for( i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i )
        aPageTemplBox.InsertEntry(SwStyleNameMapper::GetUIName( i, aEmptyStr ));

    sal_uInt16 nCount = pSh->GetPageDescCnt();
    for(i = 0; i < nCount; ++i)
    {
        const SwPageDesc &rPageDesc = pSh->GetPageDesc(i);
        if(LISTBOX_ENTRY_NOTFOUND == aPageTemplBox.GetEntryPos(rPageDesc.GetName()))
            aPageTemplBox.InsertEntry(rPageDesc.GetName());
    }

    aPageTemplBox.SelectEntry( pInf->GetPageDesc( *pSh->GetDoc() )->GetName());
    delete pInf;
}

SwEndNoteOptionPage::~SwEndNoteOptionPage()
{
}

SfxTabPage *SwEndNoteOptionPage::Create( Window *pParent, const SfxItemSet &rSet )
{
    return new SwEndNoteOptionPage( pParent, sal_True, rSet );
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
    aNumCountBox.SelectEntry(sSelect);
    NumCountHdl( &aNumCountBox );
}



int SwEndNoteOptionPage::GetNumbering() const
{
    const sal_uInt16 nPos = aNumCountBox.GetSelectEntryPos();
    return (int) bPosDoc? nPos + 1: nPos;
}

void SwEndNoteOptionPage::SetShell( SwWrtShell &rShell )
{
    pSh = &rShell;
    // collect character templates
    aFtnCharTextTemplBox.Clear();
    aFtnCharAnchorTemplBox.Clear();
    ::FillCharStyleListBox(aFtnCharTextTemplBox,
                        pSh->GetView().GetDocShell());

    ::FillCharStyleListBox(aFtnCharAnchorTemplBox,
                        pSh->GetView().GetDocShell());
}

/*------------------------------------------------------------------------
 Description:  Handler behind the button to collect the footnote at the
               page. In this case all kinds of numbering can be used.
------------------------------------------------------------------------*/


IMPL_LINK_NOARG(SwEndNoteOptionPage, PosPageHdl)
{
    const SwFtnNum eNum = (const SwFtnNum)GetNumbering();
    bPosDoc = sal_False;
    if(LISTBOX_ENTRY_NOTFOUND == aNumCountBox.GetEntryPos(aNumPage))
    {
        aNumCountBox.InsertEntry(aNumPage, FTNNUM_PAGE);
        aNumCountBox.InsertEntry(aNumChapter, FTNNUM_CHAPTER);
        SelectNumbering(eNum);
    }
    aPageTemplLbl.Enable(sal_False);
    aPageTemplBox.Enable(sal_False);

    return 0;
}

IMPL_LINK_NOARG(SwEndNoteOptionPage, NumCountHdl)
{
    sal_Bool bEnable = sal_True;
    if( aNumCountBox.GetEntryCount() - 1 != aNumCountBox.GetSelectEntryPos() )
    {
        bEnable = sal_False;
        aOffsetFld.SetValue(1);
    }
    aOffsetLbl.Enable(bEnable);
    aOffsetFld.Enable(bEnable);
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

    bPosDoc = sal_True;
    aNumCountBox.RemoveEntry(aNumPage);
    aNumCountBox.RemoveEntry(aNumChapter);
    aPageTemplLbl.Enable();
    aPageTemplBox.Enable();
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

    pInf->nFtnOffset = static_cast< sal_uInt16 >(aOffsetFld.GetValue() -1);
    pInf->aFmt.SetNumberingType(aNumViewBox.GetSelectedNumberingType() );
    pInf->SetPrefix(aPrefixED.GetText());
    pInf->SetSuffix(aSuffixED.GetText());

    pInf->SetCharFmt( lcl_GetCharFormat( pSh,
                        aFtnCharTextTemplBox.GetSelectEntry() ) );
    pInf->SetAnchorCharFmt( lcl_GetCharFormat( pSh,
                        aFtnCharAnchorTemplBox.GetSelectEntry() ) );

    // paragraph template
    sal_uInt16 nPos = aParaTemplBox.GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        const String aFmtName( aParaTemplBox.GetSelectEntry() );
        SwTxtFmtColl *pColl = pSh->GetParaStyle(aFmtName, SwWrtShell::GETSTYLE_CREATEANY);
        OSL_ENSURE(pColl, "paragraph style not found");
        pInf->SetFtnTxtColl(*pColl);
    }

    // page template
    pInf->ChgPageDesc( pSh->FindPageDescByName(
                                aPageTemplBox.GetSelectEntry(), sal_True ) );

    if ( bEndNote )
    {
        if ( !(*pInf == pSh->GetEndNoteInfo()) )
            pSh->SetEndNoteInfo( *pInf );
    }
    else
    {
        SwFtnInfo *pI = (SwFtnInfo*)pInf;
        pI->ePos = aPosPageBox.IsChecked() ? FTNPOS_PAGE : FTNPOS_CHAPTER;
        pI->eNum = (SwFtnNum)GetNumbering();
        pI->aQuoVadis = aContEdit.GetText();
        pI->aErgoSum = aContFromEdit.GetText();
        if ( !(*pI == pSh->GetFtnInfo()) )
            pSh->SetFtnInfo( *pI );
    }
    delete pInf;
    return sal_True;
}

SwFootNoteOptionPage::SwFootNoteOptionPage( Window *pParent, const SfxItemSet &rSet ) :
    SwEndNoteOptionPage( pParent, sal_False, rSet )
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
