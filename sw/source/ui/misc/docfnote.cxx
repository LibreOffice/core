/*************************************************************************
 *
 *  $RCSfile: docfnote.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hjs $ $Date: 2003-09-25 10:50:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _DOCFNOTE_HXX
#include <docfnote.hxx>
#endif
#ifndef _IMPFNOTE_HXX
#include <impfnote.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _DOCSTYLE_HXX //autogen
#include <docstyle.hxx>
#endif
#ifndef _SWWDOCSH_HXX //autogen
#include <wdocsh.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif

#ifndef _SWSTYLE_H
#include <swstyle.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _MISC_HRC
#include <misc.hrc>
#endif
#ifndef _DOCFNOTE_HRC
#include <docfnote.hrc>
#endif
#ifndef _FRMUI_HRC
#include <frmui.hrc>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

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

void SwFootNoteOptionDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
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


SwEndNoteOptionPage::SwEndNoteOptionPage( Window *pParent, BOOL bEN,
                                          const SfxItemSet &rSet ) :
    SfxTabPage( pParent, SW_RES(bEN ? TP_ENDNOTEOPTION : TP_FOOTNOTEOPTION), rSet ),
    aNumTypeFT      (this, ResId( FT_NUMTYPE    )),
    aNumViewBox     (this, ResId( LB_NUMVIEW    ), INSERT_NUM_EXTENDED_TYPES),
    aOffsetLbl      (this, ResId( FT_OFFSET )),
    aOffsetFld      (this, ResId( FLD_OFFSET   )),
    aNumCountFT     (this, ResId( FT_NUMCOUNT   )),
    aNumCountBox    (this, ResId( LB_NUMCOUNT   )),
    aPrefixFT       (this, ResId( FT_PREFIX     )),
    aPrefixED       (this, ResId( ED_PREFIX    )),
    aSuffixFT       (this, ResId( FT_SUFFIX    )),
    aSuffixED       (this, ResId( ED_SUFFIX    )),
    aPosFT          (this, ResId( FT_POS    )),
    aPosPageBox     (this, ResId( RB_POS_PAGE    )),
    aPosChapterBox  (this, ResId( RB_POS_CHAPTER)),
    aNumFL         (this, ResId( FL_NUM        )),

    aParaTemplLbl   (this, ResId( FT_PARA_TEMPL)),
    aParaTemplBox   (this, ResId( LB_PARA_TEMPL)),
    aPageTemplLbl   (this, ResId( FT_PAGE_TEMPL)),
    aPageTemplBox   (this, ResId( LB_PAGE_TEMPL)),
    aTemplFL       (this, ResId( FL_TEMPL      )),

    aFtnCharTextTemplLbl(   this, ResId( FT_TEXT_CHARFMT)),
    aFtnCharTextTemplBox(   this, ResId( LB_TEXT_CHARFMT)),
    aFtnCharAnchorTemplLbl( this, ResId( FT_ANCHR_CHARFMT)),
    aFtnCharAnchorTemplBox( this, ResId( LB_ANCHR_CHARFMT)),
    aCharTemplFL(          this, ResId(FL_CHAR_TEMPL)),

    aContLbl        (this, ResId( FT_CONT       )),
    aContEdit       (this, ResId( ED_CONT       )),
    aContFromLbl    (this, ResId( FT_CONT_FROM )),
    aContFromEdit   (this, ResId( ED_CONT_FROM )),
    aContFL        (this, ResId( FL_CONT       )),

    aNumDoc(aNumCountBox.GetEntry(FTNNUM_DOC)),
    aNumPage(aNumCountBox.GetEntry(FTNNUM_PAGE)),
    aNumChapter(aNumCountBox.GetEntry(FTNNUM_CHAPTER)),
    pSh( 0 ),
    bPosDoc(FALSE),
    bEndNote( bEN )
{
    FreeResource();

    aPosPageBox.SetClickHdl(LINK(this, SwEndNoteOptionPage, PosPageHdl));
    aPosChapterBox.SetClickHdl(LINK(this, SwEndNoteOptionPage, PosChapterHdl));
    aNumCountBox.SetSelectHdl(LINK(this, SwEndNoteOptionPage, NumCountHdl));

}

void SwEndNoteOptionPage::Reset( const SfxItemSet& )
{
    SwEndNoteInfo *pInf = bEndNote ? new SwEndNoteInfo( pSh->GetEndNoteInfo() )
                                   : new SwFtnInfo( pSh->GetFtnInfo() );
    SfxObjectShell * pDocSh = SfxObjectShell::Current();
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
        bPosDoc = TRUE;
    }
    else
    {
        const SwFtnInfo &rInf = pSh->GetFtnInfo();
        // Position (Seite, Kapitel) setzen
        if ( rInf.ePos == FTNPOS_PAGE )
        {
            aPosPageBox.Check();
            aPageTemplLbl.Enable(FALSE);
            aPageTemplBox.Enable(FALSE);
        }
        else // if ( rInf.ePos == FTNPOS_CHAPTER )
        {
            aPosChapterBox.Check();
            aNumCountBox.RemoveEntry(aNumPage);
            aNumCountBox.RemoveEntry(aNumChapter);
            bPosDoc = TRUE;
        }
            // Verweistexte
        aContEdit.SetText(rInf.aQuoVadis);
        aContFromEdit.SetText(rInf.aErgoSum);

            // gesammelt wo
        SelectNumbering(rInf.eNum);
    }

        // Numerierung
        // Art
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

        // Vorlagen - Sonderbereiche
        // Absatz
    SfxStyleSheetBasePool* pStyleSheetPool = pSh->GetView().GetDocShell()->GetStyleSheetPool();
    pStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, SWSTYLEBIT_EXTRA);
    SfxStyleSheetBase *pStyle = pStyleSheetPool->First();
    while(pStyle)
    {
        aParaTemplBox.InsertEntry(pStyle->GetName());
        pStyle = pStyleSheetPool->Next();
    }

    String sStr;
    SwStyleNameMapper::FillUIName( bEndNote ? RES_POOLCOLL_ENDNOTE
                           : RES_POOLCOLL_FOOTNOTE, sStr );
    if(LISTBOX_ENTRY_NOTFOUND == aParaTemplBox.GetEntryPos( sStr ) )
        aParaTemplBox.InsertEntry( sStr );

    SwTxtFmtColl* pColl = pInf->GetFtnTxtColl();
    if( !pColl )
        aParaTemplBox.SelectEntry( sStr );      // Default
    else
    {
        ASSERT(!pColl->IsDefault(), "Defaultvorlage fuer Fussnoten ist falsch.");
        const USHORT nPos = aParaTemplBox.GetEntryPos(pColl->GetName());
        if( LISTBOX_ENTRY_NOTFOUND != nPos )
            aParaTemplBox.SelectEntryPos( nPos );
        else
        {
            aParaTemplBox.InsertEntry(pColl->GetName());
            aParaTemplBox.SelectEntry(pColl->GetName());
        }
    }

        // Seite
    for( USHORT i = RES_POOLPAGE_BEGIN; i <= RES_POOLPAGE_ENDNOTE; ++i )
        aPageTemplBox.InsertEntry(SwStyleNameMapper::GetUIName( i, aEmptyStr ));

    USHORT nCount = pSh->GetPageDescCnt();
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
    return new SwEndNoteOptionPage( pParent, TRUE, rSet );
}

/*------------------------------------------------------------------------
 Beschreibung:  Unterschiedliche Arten der Numerierung; da die Listbox
                unterschiedlich viele Eintraege hat, hier Funktionen
                fuer das Setzen und Erfragen der gemeinten Art
                der Numerierung.
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
#ifdef DBG_UTIL
        default:
            DBG_ERROR("Which numbering type?")
#endif
    }
    aNumCountBox.SelectEntry(sSelect);
    NumCountHdl( &aNumCountBox );
}



int SwEndNoteOptionPage::GetNumbering() const
{
    const USHORT nPos = aNumCountBox.GetSelectEntryPos();
    return (int) bPosDoc? nPos + 1: nPos;
}

/*-----------------09.02.98 11:17-------------------

--------------------------------------------------*/
void SwEndNoteOptionPage::SetShell( SwWrtShell &rShell )
{
    pSh = &rShell;
    // Zeichenvorlagen sammeln
    aFtnCharTextTemplBox.Clear();
    aFtnCharAnchorTemplBox.Clear();
    ::FillCharStyleListBox(aFtnCharTextTemplBox,
                        pSh->GetView().GetDocShell());

    ::FillCharStyleListBox(aFtnCharAnchorTemplBox,
                        pSh->GetView().GetDocShell());
}

/*------------------------------------------------------------------------
 Beschreibung:  Handler hinter dem Button fuer Sammeln der Fussnote
                auf der Seite.
                In diesem Fall koennen alle Numerierungsarten verwendet
                werden.
------------------------------------------------------------------------*/


IMPL_LINK( SwEndNoteOptionPage, PosPageHdl, Button *, EMPTYARG )
{
    const SwFtnNum eNum = (const SwFtnNum)GetNumbering();
    bPosDoc = FALSE;
    if(LISTBOX_ENTRY_NOTFOUND == aNumCountBox.GetEntryPos(aNumPage))
    {
        aNumCountBox.InsertEntry(aNumPage, FTNNUM_PAGE);
        aNumCountBox.InsertEntry(aNumChapter, FTNNUM_CHAPTER);
        SelectNumbering(eNum);
    }
    aPageTemplLbl.Enable(FALSE);
    aPageTemplBox.Enable(FALSE);

    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


IMPL_LINK( SwEndNoteOptionPage, NumCountHdl, ListBox*, EMPTYARG )
{
    BOOL bEnable = TRUE;
    if( aNumCountBox.GetEntryCount() - 1 != aNumCountBox.GetSelectEntryPos() )
    {
        bEnable = FALSE;
        aOffsetFld.SetValue(1);
    }
    aOffsetLbl.Enable(bEnable);
    aOffsetFld.Enable(bEnable);
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:  Handler hinter dem Button fuer Sammeln der Fussnote
                am Kapitel oder Dokumentende.
                In diesem Fall kann keine seitenweise Numerierung verwendet
                werden.
------------------------------------------------------------------------*/


IMPL_LINK_INLINE_START( SwEndNoteOptionPage, PosChapterHdl, Button *, EMPTYARG )
{
    if ( !bPosDoc )
        SelectNumbering(FTNNUM_DOC);

    bPosDoc = TRUE;
    aNumCountBox.RemoveEntry(aNumPage);
    aNumCountBox.RemoveEntry(aNumChapter);
    aPageTemplLbl.Enable();
    aPageTemplBox.Enable();
    return 0;
}
IMPL_LINK_INLINE_END( SwEndNoteOptionPage, PosChapterHdl, Button *, EMPTYARG )

SwCharFmt* lcl_GetCharFormat( SwWrtShell* pSh, const String& rCharFmtName )
{
    SwCharFmt* pFmt = 0;
    USHORT nChCount = pSh->GetCharFmtCount();
    for(USHORT i = 0; i< nChCount; i++)
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

BOOL SwEndNoteOptionPage::FillItemSet( SfxItemSet & )
{
    SwEndNoteInfo *pInf = bEndNote ? new SwEndNoteInfo() : new SwFtnInfo();

    pInf->nFtnOffset = aOffsetFld.GetValue() -1;
    pInf->aFmt.SetNumberingType(aNumViewBox.GetSelectedNumberingType() );
    pInf->SetPrefix(aPrefixED.GetText());
    pInf->SetSuffix(aSuffixED.GetText());

    pInf->SetCharFmt( lcl_GetCharFormat( pSh,
                        aFtnCharTextTemplBox.GetSelectEntry() ) );
    pInf->SetAnchorCharFmt( lcl_GetCharFormat( pSh,
                        aFtnCharAnchorTemplBox.GetSelectEntry() ) );

    // Absatzvorlage
    USHORT nPos = aParaTemplBox.GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        const String aFmtName( aParaTemplBox.GetSelectEntry() );
        SwTxtFmtColl *pColl = pSh->GetParaStyle(aFmtName, SwWrtShell::GETSTYLE_CREATEANY);
        ASSERT(pColl, "Absatzvorlage nicht gefunden.");
        pInf->SetFtnTxtColl(*pColl);
    }

    // Seitenvorlage
    pInf->ChgPageDesc( pSh->FindPageDescByName(
                                aPageTemplBox.GetSelectEntry(), TRUE ) );

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
    return TRUE;
}

SwFootNoteOptionPage::SwFootNoteOptionPage( Window *pParent, const SfxItemSet &rSet ) :
    SwEndNoteOptionPage( pParent, FALSE, rSet )
{
}

SwFootNoteOptionPage::~SwFootNoteOptionPage()
{
}

SfxTabPage *SwFootNoteOptionPage::Create(Window *pParent, const SfxItemSet &rSet )
{
    return new SwFootNoteOptionPage( pParent, rSet );
}







