/*************************************************************************
 *
 *  $RCSfile: linenum.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2001-03-02 14:09:26 $
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

#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif

#ifndef _DOCSTYLE_HXX //autogen
#include <docstyle.hxx>
#endif

#include "fldbas.hxx"
#include "lineinfo.hxx"
#include "globals.hrc"
#include "linenum.hrc"
#include "linenum.hxx"
#include "uitool.hxx"

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwLineNumberingDlg::SwLineNumberingDlg(SwView *pVw) :
    SfxSingleTabDialog(&pVw->GetViewFrame()->GetWindow(), 0, 0),
    pSh(pVw->GetWrtShellPtr())
{
    // TabPage erzeugen
    SfxTabPage* pPage = SwLineNumberingPage::Create(this, *(SfxItemSet*)0);
    SetTabPage(pPage);

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

IMPL_LINK( SwLineNumberingDlg, OKHdl, Button *, pBtn )
{
    if (GetOKButton()->IsEnabled())
    {
        SfxTabPage* pPage = GetTabPage();
        if (pPage)
            pPage->FillItemSet(*(SfxItemSet*)0);

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
    aDisplayGB          ( this, SW_RES( GB_DISPLAY )),
    aDivisorFT          ( this, SW_RES( FT_DIVISOR )),
    aDivisorED          ( this, SW_RES( ED_DIVISOR )),
    aDivIntervalFT      ( this, SW_RES( FT_DIV_INTERVAL )),
    aDivIntervalNF      ( this, SW_RES( NF_DIV_INTERVAL )),
    aDivRowsFT          ( this, SW_RES( FT_DIV_ROWS )),
    aDivisorGB          ( this, SW_RES( GB_DIVISOR )),
    aCountEmptyLinesCB  ( this, SW_RES( CB_COUNT_EMPTYLINES )),
    aCountFrameLinesCB  ( this, SW_RES( CB_COUNT_FRAMELINES )),
    aRestartEachPageCB  ( this, SW_RES( CB_RESTART_PAGE )),
    aCountGB            ( this, SW_RES( GB_COUNT ))

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

void __EXPORT SwLineNumberingPage::Reset( const SfxItemSet& rSet )
{
    const SwLineNumberInfo &rInf = pSh->GetLineNumberInfo();

    String sStyleName(rInf.GetCharFmt(*pSh->GetDoc())->GetName());
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
    aDisplayGB.Enable(bEnable);
    aDivisorFT.Enable(bEnable);
    aDivisorED.Enable(bEnable);
    aDivIntervalFT.Enable(bEnable);
    aDivIntervalNF.Enable(bEnable);
    aDivRowsFT.Enable(bEnable);
    aDivisorGB.Enable(bEnable);
    aCountEmptyLinesCB.Enable(bEnable);
    aCountFrameLinesCB.Enable(bEnable);
    aRestartEachPageCB.Enable(bEnable);
    aCountGB.Enable(bEnable);

    return 0;
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

BOOL __EXPORT SwLineNumberingPage::FillItemSet( SfxItemSet& rSet )
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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.2  2001/02/23 12:45:29  os
    Complete use of DefaultNumbering component

    Revision 1.1.1.1  2000/09/18 17:14:45  hr
    initial import

    Revision 1.11  2000/09/18 16:05:57  willem.vandorp
    OpenOffice header added.

    Revision 1.10  2000/08/14 12:50:15  ma
    optional restart at each page for line numbering

    Revision 1.9  2000/04/26 14:55:46  os
    GetName() returns const String&

    Revision 1.8  2000/02/11 14:56:26  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.7  1998/11/17 09:57:52  OS
    #58263# NumType durch SvxExtNumType ersetzt


      Rev 1.6   17 Nov 1998 10:57:52   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.5   11 Aug 1998 14:26:30   HR
   void* kann nicht direkt nach USHORT gecastet werden

      Rev 1.4   10 Aug 1998 16:42:10   JP
   Bug #54796#: neue NumerierungsTypen (WW97 kompatibel)

      Rev 1.3   15 Apr 1998 14:34:08   OS
   CharStyles sortiert

      Rev 1.2   20 Feb 1998 11:23:22   MA
   Restarbeiten Zeilennummerierung

      Rev 1.1   19 Feb 1998 13:51:32   OM
   Zeilennumerierung

      Rev 1.0   18 Feb 1998 18:56:48   OM
   Initial revision.

------------------------------------------------------------------------*/

