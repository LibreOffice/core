/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linenum.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:21:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif

#ifndef _WRTSH_HXX //autogen
#include <wrtsh.hxx>
#endif

#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif

#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif

//#ifndef _FLDMGR_HXX //autogen
//#include <fldmgr.hxx>
//#endif


#ifndef _DOCSTYLE_HXX //autogen
#include <docstyle.hxx>
#endif

#include "fldbas.hxx"
#include "lineinfo.hxx"
#include "globals.hrc"
#include "linenum.hrc"
#include "linenum.hxx"
#include "uitool.hxx"

#include <IDocumentStylePoolAccess.hxx>

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwLineNumberingDlg::SwLineNumberingDlg(SwView *pVw) :
    SfxSingleTabDialog(&pVw->GetViewFrame()->GetWindow(), 0, 0),
    pSh(pVw->GetWrtShellPtr())
{
    // TabPage erzeugen
    SetTabPage(SwLineNumberingPage::Create(this, *(SfxItemSet*)0));

    GetOKButton()->SetClickHdl(LINK(this, SwLineNumberingDlg, OKHdl));
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

__EXPORT SwLineNumberingDlg::~SwLineNumberingDlg()
{
}

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwLineNumberingDlg, OKHdl, Button *, EMPTYARG )
{
    if (GetOKButton()->IsEnabled())
    {
        SfxTabPage* pCurPage = GetTabPage();
        if( pCurPage )
            pCurPage->FillItemSet(*(SfxItemSet*)0);

        EndDialog( RET_OK );
    }

    return 0;
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

SwLineNumberingPage::SwLineNumberingPage( Window* pParent,
                                                    const SfxItemSet& rSet )
    : SfxTabPage(pParent, SW_RES(TP_LINENUMBERING), rSet),
    aNumberingOnCB      ( this, SW_RES( CB_NUMBERING_ON )),
    aCharStyleFT        ( this, SW_RES( FT_CHAR_STYLE )),
    aCharStyleLB        ( this, SW_RES( LB_CHAR_STYLE )),
    aFormatFT           ( this, SW_RES( FT_FORMAT )),
    aFormatLB           ( this, SW_RES( LB_FORMAT ), INSERT_NUM_EXTENDED_TYPES),
    aPosFT              ( this, SW_RES( FT_POS )),
    aPosLB              ( this, SW_RES( LB_POS )),
    aOffsetFT           ( this, SW_RES( FT_OFFSET )),
    aOffsetMF           ( this, SW_RES( MF_OFFSET )),
    aNumIntervalFT      ( this, SW_RES( FT_NUM_INVERVAL )),
    aNumIntervalNF      ( this, SW_RES( NF_NUM_INVERVAL )),
    aNumRowsFT          ( this, SW_RES( FT_NUM_ROWS )),
    aDisplayFL          ( this, SW_RES( FL_DISPLAY )),
    aDivisorFT          ( this, SW_RES( FT_DIVISOR )),
    aDivisorED          ( this, SW_RES( ED_DIVISOR )),
    aDivIntervalFT      ( this, SW_RES( FT_DIV_INTERVAL )),
    aDivIntervalNF      ( this, SW_RES( NF_DIV_INTERVAL )),
    aDivRowsFT          ( this, SW_RES( FT_DIV_ROWS )),
    aDivisorFL          ( this, SW_RES( FL_DIVISOR )),
    aCountEmptyLinesCB  ( this, SW_RES( CB_COUNT_EMPTYLINES )),
    aCountFrameLinesCB  ( this, SW_RES( CB_COUNT_FRAMELINES )),
    aRestartEachPageCB  ( this, SW_RES( CB_RESTART_PAGE )),
    aCountFL            ( this, SW_RES( FL_COUNT ))

{
    FreeResource();
    SwLineNumberingDlg *pDlg = (SwLineNumberingDlg *)GetParent();
    pSh = pDlg->GetWrtShell();
    // Zeichenvorlagen
    ::FillCharStyleListBox(aCharStyleLB, pSh->GetView().GetDocShell());
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

__EXPORT SwLineNumberingPage::~SwLineNumberingPage()
{
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

SfxTabPage* __EXPORT SwLineNumberingPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SwLineNumberingPage( pParent, rSet );
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

void __EXPORT SwLineNumberingPage::Reset( const SfxItemSet&  )
{
    const SwLineNumberInfo &rInf = pSh->GetLineNumberInfo();
    IDocumentStylePoolAccess* pIDSPA = pSh->getIDocumentStylePoolAccess();

    String sStyleName(rInf.GetCharFmt( *pIDSPA )->GetName());
    const USHORT nPos = aCharStyleLB.GetEntryPos(sStyleName);

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
        aCharStyleLB.SelectEntryPos(nPos);
    else
    {
        if (sStyleName.Len())
        {
            aCharStyleLB.InsertEntry(sStyleName);
            aCharStyleLB.SelectEntry(sStyleName);
        }
    }

    // Format
//  SwFldMgr aMgr( pSh );
    USHORT nSelFmt = rInf.GetNumType().GetNumberingType();
//  USHORT nCnt = aMgr.GetFormatCount( TYP_SEQFLD, FALSE );

//  for( USHORT i = 0; i < nCnt; i++)
//  {
//      aFormatLB.InsertEntry(aMgr.GetFormatStr( TYP_SEQFLD, i));
//      USHORT nFmtId = aMgr.GetFormatId( TYP_SEQFLD, i );
//      aFormatLB.SetEntryData( i, (void*)nFmtId );
//      if( nFmtId == nSelFmt )
//          aFormatLB.SelectEntryPos( i );
//  }
    aFormatLB.SelectNumberingType(nSelFmt);

//  if ( !aFormatLB.GetSelectEntryCount() )
//      aFormatLB.SelectEntryPos(aFormatLB.GetEntryCount() - 1);

    // Position
    aPosLB.SelectEntryPos((USHORT)rInf.GetPos());

    // Offset
    USHORT nOffset = rInf.GetPosFromLeft();
    if (nOffset == USHRT_MAX)
        nOffset = 0;

    aOffsetMF.SetValue(aOffsetMF.Normalize(nOffset), FUNIT_TWIP);

    // Numerierungsoffset
    aNumIntervalNF.SetValue(rInf.GetCountBy());

    // Teiler
    aDivisorED.SetText(rInf.GetDivider());

    // Teileroffset
    aDivIntervalNF.SetValue(rInf.GetDividerCountBy());

    // Zaehlen
    aCountEmptyLinesCB.Check(rInf.IsCountBlankLines());
    aCountFrameLinesCB.Check(rInf.IsCountInFlys());
    aRestartEachPageCB.Check(rInf.IsRestartEachPage());

    aNumberingOnCB.Check(rInf.IsPaintLineNumbers());

    aNumberingOnCB.SetClickHdl(LINK(this, SwLineNumberingPage, LineOnOffHdl));
    aDivisorED.SetModifyHdl(LINK(this, SwLineNumberingPage, ModifyHdl));
    ModifyHdl();
    LineOnOffHdl();
}

/*--------------------------------------------------------------------
    Beschreibung: Modify
 --------------------------------------------------------------------*/

IMPL_LINK( SwLineNumberingPage, ModifyHdl, Edit *, EMPTYARG )
{
    BOOL bHasValue = aDivisorED.GetText().Len() != 0;

    aDivIntervalFT.Enable(bHasValue);
    aDivIntervalNF.Enable(bHasValue);
    aDivRowsFT.Enable(bHasValue);

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: On/Off
 --------------------------------------------------------------------*/

IMPL_LINK( SwLineNumberingPage, LineOnOffHdl, CheckBox *, EMPTYARG )
{
    BOOL bEnable = aNumberingOnCB.IsChecked();

    aCharStyleFT.Enable(bEnable);
    aCharStyleLB.Enable(bEnable);
    aFormatFT.Enable(bEnable);
    aFormatLB.Enable(bEnable);
    aPosFT.Enable(bEnable);
    aPosLB.Enable(bEnable);
    aOffsetFT.Enable(bEnable);
    aOffsetMF.Enable(bEnable);
    aNumIntervalFT.Enable(bEnable);
    aNumIntervalNF.Enable(bEnable);
    aNumRowsFT.Enable(bEnable);
    aDisplayFL.Enable(bEnable);
    aDivisorFT.Enable(bEnable);
    aDivisorED.Enable(bEnable);
    aDivIntervalFT.Enable(bEnable);
    aDivIntervalNF.Enable(bEnable);
    aDivRowsFT.Enable(bEnable);
    aDivisorFL.Enable(bEnable);
    aCountEmptyLinesCB.Enable(bEnable);
    aCountFrameLinesCB.Enable(bEnable);
    aRestartEachPageCB.Enable(bEnable);
    aCountFL.Enable(bEnable);

    return 0;
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

BOOL __EXPORT SwLineNumberingPage::FillItemSet( SfxItemSet& )
{
    SwLineNumberInfo aInf(pSh->GetLineNumberInfo());

    // Zeichenvorlagen
    String sCharFmtName(aCharStyleLB.GetSelectEntry());
    SwCharFmt *pCharFmt = pSh->FindCharFmtByName(sCharFmtName);

    if (!pCharFmt)
    {
        SfxStyleSheetBasePool* pPool = pSh->GetView().GetDocShell()->GetStyleSheetPool();
        SfxStyleSheetBase* pBase;
        pBase = pPool->Find(sCharFmtName, SFX_STYLE_FAMILY_CHAR);
        if(!pBase)
            pBase = &pPool->Make(sCharFmtName, SFX_STYLE_FAMILY_CHAR);
        pCharFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();
    }

    if (pCharFmt)
        aInf.SetCharFmt(pCharFmt);

    // Format
    SvxNumberType aType;
    aType.SetNumberingType(aFormatLB.GetSelectedNumberingType());
    aInf.SetNumType(aType);

    // Position
    aInf.SetPos((LineNumberPosition)aPosLB.GetSelectEntryPos());

    // Offset
    aInf.SetPosFromLeft((USHORT)aOffsetMF.Denormalize(aOffsetMF.GetValue(FUNIT_TWIP)));

    // Numerierungsoffset
    aInf.SetCountBy((USHORT)aNumIntervalNF.GetValue());

    // Teiler
    aInf.SetDivider(aDivisorED.GetText());

    // Teileroffset
    aInf.SetDividerCountBy((USHORT)aDivIntervalNF.GetValue());

    // Zaehlen
    aInf.SetCountBlankLines(aCountEmptyLinesCB.IsChecked());
    aInf.SetCountInFlys(aCountFrameLinesCB.IsChecked());
    aInf.SetRestartEachPage(aRestartEachPageCB.IsChecked());

    aInf.SetPaintLineNumbers(aNumberingOnCB.IsChecked());

    pSh->SetLineNumberInfo(aInf);

    return FALSE;
}


