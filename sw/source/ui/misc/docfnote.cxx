/*************************************************************************
 *
 *  $RCSfile: docfnote.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2001-03-02 14:08:37 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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
    SfxItemSet *pSet = 0;
    SfxTabPage *pPage = GetTabPage( TP_FOOTNOTEOPTION );
    if ( pPage )
        pPage->FillItemSet( (SfxItemSet&)pSet );
    pPage = GetTabPage( TP_ENDNOTEOPTION  );
    if ( pPage )
        pPage->FillItemSet( (SfxItemSet&)pSet );
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
    aNumGrp         (this, ResId( GB_NUM        )),

    aParaTemplLbl   (this, ResId( FT_PARA_TEMPL)),
    aParaTemplBox   (this, ResId( LB_PARA_TEMPL)),
    aPageTemplLbl   (this, ResId( FT_PAGE_TEMPL)),
    aPageTemplBox   (this, ResId( LB_PAGE_TEMPL)),
    aTemplGrp       (this, ResId( GB_TEMPL      )),

    aFtnCharTextTemplLbl(   this, ResId( FT_TEXT_CHARFMT)),
    aFtnCharTextTemplBox(   this, ResId( LB_TEXT_CHARFMT)),
    aFtnCharAnchorTemplLbl( this, ResId( FT_ANCHR_CHARFMT)),
    aFtnCharAnchorTemplBox( this, ResId( LB_ANCHR_CHARFMT)),
    aCharTemplGrp(          this, ResId(GB_CHAR_TEMPL)),

    aContLbl        (this, ResId( FT_CONT       )),
    aContEdit       (this, ResId( ED_CONT       )),
    aContFromLbl    (this, ResId( FT_CONT_FROM )),
    aContFromEdit   (this, ResId( ED_CONT_FROM )),
    aContGrp        (this, ResId( GB_CONT       )),

    aNumPage(aNumCountBox.GetEntry(FTNNUM_PAGE)),
    pSh( 0 ),
    bPosDoc(FALSE),
    bEndNote( bEN )
{
    FreeResource();

    aPosPageBox.SetClickHdl(LINK(this, SwEndNoteOptionPage, PosPage));
    aPosChapterBox.SetClickHdl(LINK(this, SwEndNoteOptionPage, PosChapter));
    aNumCountBox.SetSelectHdl(LINK(this, SwEndNoteOptionPage, NumCount));

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
        aCharTemplGrp   .Hide();
        aTemplGrp       .Hide();
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
        aContGrp.Hide();
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
    GetDocPoolNm( bEndNote ? RES_POOLCOLL_ENDNOTE
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
        aPageTemplBox.InsertEntry(GetDocPoolNm( i, sStr ));

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
inline void SwEndNoteOptionPage::SelectNumbering(int eNum)
{
    aNumCountBox.SelectEntryPos(bPosDoc? (USHORT)eNum - 1: eNum);
    NumCount( &aNumCountBox );
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


IMPL_LINK( SwEndNoteOptionPage, PosPage, Button *, EMPTYARG )
{
    const SwFtnNum eNum = (const SwFtnNum)GetNumbering();
    bPosDoc = FALSE;
    if(LISTBOX_ENTRY_NOTFOUND == aNumCountBox.GetEntryPos(aNumPage)) {
        aNumCountBox.InsertEntry(aNumPage, FTNNUM_PAGE);
        SelectNumbering(eNum);
    }
    aPageTemplLbl.Enable(FALSE);
    aPageTemplBox.Enable(FALSE);

    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


IMPL_LINK( SwEndNoteOptionPage, NumCount, ListBox*, EMPTYARG )
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


IMPL_LINK_INLINE_START( SwEndNoteOptionPage, PosChapter, Button *, EMPTYARG )
{
    if ( !bPosDoc )
        SelectNumbering(FTNNUM_DOC);

    bPosDoc = TRUE;
    aNumCountBox.RemoveEntry(aNumPage);
    aPageTemplLbl.Enable();
    aPageTemplBox.Enable();
    return 0;
}
IMPL_LINK_INLINE_END( SwEndNoteOptionPage, PosChapter, Button *, EMPTYARG )

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

BOOL SwEndNoteOptionPage::FillItemSet( SfxItemSet &rSet )
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



/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.3  2001/02/23 12:45:29  os
    Complete use of DefaultNumbering component

    Revision 1.2  2001/02/09 08:01:42  os
    TabPage size changed

    Revision 1.1.1.1  2000/09/18 17:14:44  hr
    initial import

    Revision 1.72  2000/09/18 16:05:56  willem.vandorp
    OpenOffice header added.

    Revision 1.71  2000/07/27 21:16:43  jp
    opt: get template names direct from the doc and don't load it from the resource

    Revision 1.70  2000/04/26 14:57:09  os
    GetName() returns const String&

    Revision 1.69  2000/02/22 14:50:14  os
    #72894# footnote option dialog in HTML documents

    Revision 1.68  1999/11/12 13:02:12  jp
    call Set-/GetAnchorCharFormat at SwEndnoteInfo

    Revision 1.67  1999/11/05 12:45:36  os
    character style for footnote anchor

    Revision 1.66  1999/02/05 16:45:26  JP
    Task #61467#/#61014#: neu FindPageDescByName


      Rev 1.65   05 Feb 1999 17:45:26   JP
   Task #61467#/#61014#: neu FindPageDescByName

      Rev 1.64   17 Nov 1998 10:57:52   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.63   06 Nov 1998 16:36:02   OS
   #58450# Fussnoten auch im HTML

      Rev 1.62   06 Aug 1998 21:41:14   JP
   Bug #54796#: neue NumerierungsTypen (WW97 kompatibel)

      Rev 1.61   26 Jun 1998 16:17:42   OM
   #51758# Unterschiedliche HelpIDs fuer alle Controls

      Rev 1.60   15 Apr 1998 14:34:58   OS
   CharStyles sortiert

      Rev 1.59   26 Feb 1998 17:09:28   OS
   FillItemSet: erst aendern, dann vergleichen

      Rev 1.58   24 Feb 1998 12:17:08   OS
   Endnotesymbol #47534#

      Rev 1.57   19 Feb 1998 13:52:18   OM
   Zeichen statt Seitenvorlagen

      Rev 1.56   09 Feb 1998 13:07:28   OS
   Fuss-/Endnoten mit Zeichenvorlage und Pre-/Suffix

      Rev 1.55   03 Feb 1998 12:54:48   OM
   #46624# HelpIDs fuer Fuss- und Endnoten TPs

      Rev 1.54   21 Jan 1998 11:03:56   JP
   FootNodeOptionPage: unoetiges und falsche FreeResource entfernt

      Rev 1.53   15 Dec 1997 10:45:20   MA
   FtnInfo: client fuer Pagedesc; TxtColl fuer Endnote

      Rev 1.52   12 Dec 1997 16:07:14   MA
   poolpage fuer footnote und endnote

      Rev 1.51   12 Dec 1997 10:10:22   MA
   Fuss-/Endnoteneinstellungen

      Rev 1.50   24 Nov 1997 16:47:44   MA
   includes

      Rev 1.49   03 Nov 1997 13:22:42   MA
   precomp entfernt

      Rev 1.48   07 Feb 1997 15:48:18   MA
   fix: ClickHdl wird gerufen, der darf dann aber die Defaults nicht verstellen.

      Rev 1.47   11 Nov 1996 11:05:44   MA
   ResMgr

      Rev 1.46   24 Oct 1996 17:08:28   JP
   Optimierung: Find...ByName

      Rev 1.45   02 Oct 1996 18:28:58   MA
   Umstellung Enable/Disable

      Rev 1.44   28 Aug 1996 14:12:20   OS
   includes

      Rev 1.43   22 Mar 1996 14:06:46   HJS
   umstellung 311

      Rev 1.42   17 Feb 1996 10:25:06   OS
   Absatzvorlagen-Listbox zeigt jetzt alle Vorlagen/Sonderbereiche an

      Rev 1.41   13 Feb 1996 11:37:46   OS
   Offset fuer Fussnoten nur pro Dokument

      Rev 1.40   12 Feb 1996 16:42:18   OS
   neu: Offset fuer Fussnotennumerierung

      Rev 1.39   06 Feb 1996 15:21:06   JP
   Link Umstellung 305

      Rev 1.38   24 Nov 1995 16:58:44   OM
   PCH->PRECOMPILED

      Rev 1.37   08 Nov 1995 13:31:40   JP
   Umstellung zur 301: Change -> Set

      Rev 1.36   12 Sep 1995 17:29:26   OM
   Helpbutton eingefuegt

      Rev 1.35   30 Aug 1995 14:00:46   MA
   fix: sexport'iert

      Rev 1.34   21 Aug 1995 09:33:48   MA
   chg: swstddlg -> svxstandarddialog, Optimierungen

      Rev 1.33   26 Oct 1994 12:17:50   ER
   add: PCH (missed the misc)

      Rev 1.32   17 Oct 1994 16:26:14   PK
   ausgeboxtes wieder reingeboxt

------------------------------------------------------------------------*/




