/*************************************************************************
 *
 *  $RCSfile: envfmt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-20 17:01:50 $
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


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#include <tools/pstm.hxx>

#define _SVSTDARR_LONGSSORT
#define _SVSTDARR_LONGS
#include <svtools/svstdarr.hxx>

#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _CHRDLG_HXX
#include <chrdlg.hxx>
#endif
#ifndef _DRPCPS_HXX
#include <drpcps.hxx>
#endif
#ifndef _ENVFMT_HXX
#include <envfmt.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _PARDLG_HXX
#include <pardlg.hxx>
#endif
#ifndef _PATTERN_HXX
#include <pattern.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _UIBORDER_HXX
#include <uiborder.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif

#ifndef _ENVFMT_HRC
#include <envfmt.hrc>
#endif

static PopupMenu *pMenu;
static long lUserW = 5669; // 10 cm
static long lUserH = 5669; // 10 cm

// --------------------------------------------------------------------------


SwEnvFmtPage::SwEnvFmtPage(Window* pParent, const SfxItemSet& rSet) :

    SfxTabPage(pParent, SW_RES(TP_ENV_FMT), rSet),

    aAddrGroup          (this, SW_RES( GRP_ADDRESSEE )),
    aAddrPosInfo        (this, SW_RES( TXT_ADDR_POS )),
    aAddrLeftText       (this, SW_RES( TXT_ADDR_LEFT )),
    aAddrLeftField      (this, SW_RES( FLD_ADDR_LEFT )),
    aAddrTopText        (this, SW_RES( TXT_ADDR_TOP )),
    aAddrTopField       (this, SW_RES( FLD_ADDR_TOP )),
    aAddrFormatInfo     (this, SW_RES( TXT_ADDR_FORMAT )),
    aAddrEditButton     (this, SW_RES( BTN_ADDR_EDIT )),
    aSendGroup          (this, SW_RES( GRP_SENDER )),
    aSendPosInfo        (this, SW_RES( TXT_SEND_POS )),
    aSendLeftText       (this, SW_RES( TXT_SEND_LEFT )),
    aSendLeftField      (this, SW_RES( FLD_SEND_LEFT )),
    aSendTopText        (this, SW_RES( TXT_SEND_TOP )),
    aSendTopField       (this, SW_RES( FLD_SEND_TOP )),
    aSendFormatInfo     (this, SW_RES( TXT_SEND_FORMAT )),
    aSendEditButton     (this, SW_RES( BTN_SEND_EDIT )),
    aSizeGroup          (this, SW_RES( GRP_SIZE )),
    aSizeFormatText     (this, SW_RES( TXT_SIZE_FORMAT )),
    aSizeFormatBox      (this, SW_RES( BOX_SIZE_FORMAT )),
    aSizeWidthText      (this, SW_RES( TXT_SIZE_WIDTH )),
    aSizeWidthField     (this, SW_RES( FLD_SIZE_WIDTH )),
    aSizeHeightText     (this, SW_RES( TXT_SIZE_HEIGHT )),
    aSizeHeightField    (this, SW_RES( FLD_SIZE_HEIGHT )),
    aPreview            (this, SW_RES( WIN_PREVIEW ))

{
    FreeResource();
    SetExchangeSupport();

    // Metriken
    FieldUnit aMetric = ::GetDfltMetric(FALSE);
    SetMetric(aAddrLeftField,   aMetric);
    SetMetric(aAddrTopField,    aMetric);
    SetMetric(aSendLeftField,   aMetric);
    SetMetric(aSendTopField,    aMetric);
    SetMetric(aSizeWidthField,  aMetric);
    SetMetric(aSizeHeightField, aMetric);

    // Menues einhaengen
    ::pMenu = new PopupMenu(SW_RES(MNU_EDIT));
    aAddrEditButton.SetPopupMenu(::pMenu);
    aSendEditButton.SetPopupMenu(::pMenu);

    // Handler installieren
    Link aLk = LINK(this, SwEnvFmtPage, ModifyHdl);
    aAddrLeftField  .SetModifyHdl( aLk );
    aAddrTopField   .SetModifyHdl( aLk );
    aSendLeftField  .SetModifyHdl( aLk );
    aSendTopField   .SetModifyHdl( aLk );
    aSizeWidthField .SetModifyHdl( aLk );
    aSizeHeightField.SetModifyHdl( aLk );

    aLk = LINK(this, SwEnvFmtPage, EditHdl );
    aAddrEditButton.SetSelectHdl( aLk );
    aSendEditButton.SetSelectHdl( aLk );

    aSizeFormatBox     .SetSelectHdl(LINK(this, SwEnvFmtPage, FormatHdl));

    // aSizeFormatBox
    int i;
    for (i = SVX_PAPER_A3; i <= SVX_PAPER_KAI32BIG; i++)
    {
        if (i != SVX_PAPER_USER)
        {
            String aPaperName = SvxPaperInfo::GetName((SvxPaper) i),
                   aEntryName;

            USHORT nPos   = 0;
            BOOL   bFound = FALSE;
            while (nPos < aSizeFormatBox.GetEntryCount() && !bFound)
            {
                aEntryName = aSizeFormatBox.GetEntry(i);
                if (aEntryName < aPaperName)
                    nPos++;
                else
                    bFound = TRUE;
            }
            aSizeFormatBox.InsertEntry(aPaperName, nPos);
            aIDs.Insert((USHORT) i, nPos);
        }
    }
    aSizeFormatBox.InsertEntry(SvxPaperInfo::GetName(SVX_PAPER_USER));
    aIDs.Insert((USHORT) SVX_PAPER_USER, aIDs.Count());

    // Timer einstellen
    aPreviewTimer.SetTimeout(500);
    aPreviewTimer.SetTimeoutHdl(LINK(this, SwEnvFmtPage, PreviewHdl));
}

// --------------------------------------------------------------------------


__EXPORT SwEnvFmtPage::~SwEnvFmtPage()
{
    aAddrEditButton.SetPopupMenu(0);
    aSendEditButton.SetPopupMenu(0);
    delete ::pMenu;
}

// --------------------------------------------------------------------------


IMPL_LINK_INLINE_START( SwEnvFmtPage, ModifyHdl, Edit *, pEdit )
{
    pLastEdit = pEdit;
    aPreviewTimer.Start();
    return 0;
}
IMPL_LINK_INLINE_END( SwEnvFmtPage, ModifyHdl, Edit *, pEdit )

// --------------------------------------------------------------------------


IMPL_LINK( SwEnvFmtPage, EditHdl, MenuButton *, pButton )
{
    SwWrtShell* pSh = GetParent()->pSh;
    ASSERT(pSh, "Shell fehlt");

    // Collection-Ptr ermitteln
    BOOL bSender = pButton != &aAddrEditButton;

    SwTxtFmtColl* pColl = pSh->GetTxtCollFromPool(
        bSender ? RES_POOLCOLL_SENDADRESS : RES_POOLCOLL_JAKETADRESS);
    ASSERT(pColl, "Text-Collection fehlt");

    switch (pButton->GetCurItemId())
    {
        case MID_CHAR:
        {
            SfxItemSet *pSet = GetCollItemSet(pColl, bSender);

            // Damit die Hintergrundfarbe nicht uebergebuegelt wird:
            SfxAllItemSet aTmpSet(*pSet);

            // Das CHRATR_BACKGROUND-Attribut wird fuer den Dialog in
            // ein RES_BACKGROUND verwandelt und wieder zurueck ...
            const SfxPoolItem *pTmpBrush;

            if( SFX_ITEM_SET == aTmpSet.GetItemState( RES_CHRATR_BACKGROUND,
                TRUE, &pTmpBrush ) )
            {
                SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                aTmpBrush.SetWhich( RES_BACKGROUND );
                aTmpSet.Put( aTmpBrush );
            }
            else
                aTmpSet.ClearItem( RES_BACKGROUND );

            SwCharDlg* pDlg = new SwCharDlg(GetParent(), pSh->GetView(), aTmpSet, &pColl->GetName());
            if (pDlg->Execute() == RET_OK)
            {
                SfxItemSet aOutputSet( *pDlg->GetOutputItemSet() );
                if( SFX_ITEM_SET == aOutputSet.GetItemState( RES_BACKGROUND,
                    FALSE, &pTmpBrush ) )
                {
                    SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                    aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
                    pSet->Put( aTmpBrush );
                }
                aOutputSet.ClearItem( RES_BACKGROUND );
                //pColl->SetAttr( aTmpSet );
                pSet->Put(aOutputSet);
            }
            delete pDlg;
        }
        break;

        case MID_PARA:
        {
            SfxItemSet *pSet = GetCollItemSet(pColl, bSender);

            // Damit die Tabulatoren nicht uebergebuegelt werden:
            SfxAllItemSet aTmpSet(*pSet);

            // Tabulatoren, DefaultTabs ins ItemSet Stecken
            const SvxTabStopItem& rDefTabs = (const SvxTabStopItem&)
                pSh->GetView().GetCurShell()->GetPool().GetDefaultItem(RES_PARATR_TABSTOP);

            USHORT nDefDist = ::GetTabDist( rDefTabs );
            SfxUInt16Item aDefDistItem( SID_ATTR_TABSTOP_DEFAULTS, nDefDist );
            aTmpSet.Put( aDefDistItem );

            // Aktueller Tab
            SfxUInt16Item aTabPos( SID_ATTR_TABSTOP_POS, 0 );
            aTmpSet.Put( aTabPos );

            // linker Rand als Offset
            const long nOff = ((SvxLRSpaceItem&)aTmpSet.Get( RES_LR_SPACE )).
                                                                GetTxtLeft();
            SfxInt32Item aOff( SID_ATTR_TABSTOP_OFFSET, nOff );
            aTmpSet.Put( aOff );

            // BoxInfo setzen
            ::PrepareBoxInfo( aTmpSet, *pSh );

            SwParaDlg *pDlg = new SwParaDlg(GetParent(), pSh->GetView(), aTmpSet, DLG_ENVELOP, &pColl->GetName());

            if ( pDlg->Execute() == RET_OK )
            {
                // Defaults evtl umsetzen
                const SfxPoolItem* pItem = 0;
                SfxItemSet* pOutputSet = (SfxItemSet*)pDlg->GetOutputItemSet();
                USHORT nNewDist;
                if( SFX_ITEM_SET == pOutputSet->GetItemState( SID_ATTR_TABSTOP_DEFAULTS,
                    FALSE, &pItem ) &&
                    nDefDist != (nNewDist = ((SfxUInt16Item*)pItem)->GetValue()) )
                {
                    SvxTabStopItem aDefTabs( 0, 0 );
                    MakeDefTabs( nNewDist, aDefTabs );
                    pSh->SetDefault( aDefTabs );
                    pOutputSet->ClearItem( SID_ATTR_TABSTOP_DEFAULTS );
                }
                if( pOutputSet->Count() )
                {
                    //pColl->SetAttr( *pOutputSet );
                    pSet->Put(*pOutputSet);
                }
            }
            delete pDlg;
        }
        break;
    }
    return 0;
}

/*------------------------------------------------------------------------
  Beschreibung: Ein temporaeres Itemset, das bei Abbruch verworfen wird
------------------------------------------------------------------------*/

SfxItemSet *SwEnvFmtPage::GetCollItemSet(SwTxtFmtColl* pColl, BOOL bSender)
{
    SfxItemSet *&pSet = bSender ? GetParent()->pSenderSet : GetParent()->pAddresseeSet;

    if (!pSet)
    {
        // Range ermitteln (Ranges beider Itemsets mergen)
        const USHORT *pRanges = pColl->GetAttrSet().GetRanges();

        static USHORT __READONLY_DATA aRanges[] =
        {
            RES_PARATR_BEGIN, RES_PARATR_ADJUST,
            RES_PARATR_TABSTOP, RES_PARATR_END-1,
            RES_LR_SPACE, RES_UL_SPACE,
            RES_BACKGROUND, RES_SHADOW,
            SID_ATTR_TABSTOP_POS, SID_ATTR_TABSTOP_POS,
            SID_ATTR_TABSTOP_DEFAULTS, SID_ATTR_TABSTOP_DEFAULTS,
            SID_ATTR_TABSTOP_OFFSET, SID_ATTR_TABSTOP_OFFSET,
            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
            0, 0
        };

        // BruteForce-Merge, weil MergeRange in SvTools buggy ist:
        USHORT i = 0;
        SvLongsSort aMergedRanges( 0, 10 );

        while (pRanges[i])
        {
            for (USHORT nPos = pRanges[i]; nPos <= pRanges[i+1]; nPos++)
                aMergedRanges.Insert(nPos);
            i += 2;
        }

        i = 0;

        while (aRanges[i])
        {
            for (USHORT nPos = aRanges[i]; nPos <= aRanges[i+1]; nPos++)
                aMergedRanges.Insert(nPos);
            i += 2;
        }

        // Ranges kompaktieren
        SvLongs aCompactedRanges( 0, 10 );

        aCompactedRanges.Insert(aMergedRanges[0], aCompactedRanges.Count());

        for (i = 0; i < aMergedRanges.Count(); i++)
        {
            while (i + 1 < aMergedRanges.Count() &&
                aMergedRanges[i+1] - aMergedRanges[i] == 1)
            {
                i++;
            }
            long nEnd = aMergedRanges[i];
            aCompactedRanges.Insert(nEnd, aCompactedRanges.Count());

            if (i + 1 < aMergedRanges.Count())
            {
                long nStart = aMergedRanges[i+1];
                aCompactedRanges.Insert(nStart, aCompactedRanges.Count());
            }
        }

        // Neue Ranges erzeugen
        USHORT *pNewRanges = new USHORT[aCompactedRanges.Count() + 1];
        for (i = 0; i < aCompactedRanges.Count(); i++)
            pNewRanges[i] = (USHORT)aCompactedRanges[i];

        pNewRanges[i] = 0;

        pSet = new SfxItemSet(GetParent()->pSh->GetView().GetCurShell()->GetPool(),
                                pNewRanges);

        pSet->Put(pColl->GetAttrSet());
        delete pNewRanges;
    }

    return pSet;
}

// --------------------------------------------------------------------------


IMPL_LINK( SwEnvFmtPage, FormatHdl, ListBox *, EMPTYARG )
{
    long lWidth;
    long lHeight;
    long lSendFromLeft;
    long lSendFromTop;
    long lAddrFromLeft;
    long lAddrFromTop;

    USHORT nPaper = aIDs[aSizeFormatBox.GetSelectEntryPos()];
    if (nPaper != (USHORT)SVX_PAPER_USER)
    {
        Size aSz = SvxPaperInfo::GetPaperSize((SvxPaper)nPaper);
        lWidth  = Max(aSz.Width(), aSz.Height());
        lHeight = Min(aSz.Width(), aSz.Height());
    }
    else
    {
        lWidth  = lUserW;
        lHeight = lUserH;
    }

    lSendFromLeft = 566;            // 1cm
    lSendFromTop  = 566;            // 1cm
    lAddrFromLeft = lWidth  / 2;
    lAddrFromTop  = lHeight / 2;

    SetFldVal(aAddrLeftField, lAddrFromLeft);
    SetFldVal(aAddrTopField , lAddrFromTop );
    SetFldVal(aSendLeftField, lSendFromLeft);
    SetFldVal(aSendTopField , lSendFromTop );

    SetFldVal(aSizeWidthField , lWidth );
    SetFldVal(aSizeHeightField, lHeight);

    SetMinMax();

    FillItem(GetParent()->aEnvItem);
    aPreview.Invalidate();
    return 0;
}

// --------------------------------------------------------------------------


IMPL_LINK( SwEnvFmtPage, PreviewHdl, Timer *, EMPTYARG )
{
    long lWVal = GetFldVal(aSizeWidthField );
    long lHVal = GetFldVal(aSizeHeightField);

    long lWidth  = Max(lWVal, lHVal);
    long lHeight = Min(lWVal, lHVal);

    if (pLastEdit == &aSizeWidthField || pLastEdit == &aSizeHeightField)
    {
        SvxPaper ePaper = SvxPaperInfo::GetPaper(
            Size(lHeight, lWidth), MAP_TWIP, TRUE);
        for (USHORT i = 0; i < aIDs.Count(); i++)
            if (aIDs[i] == (USHORT)ePaper)
                aSizeFormatBox.SelectEntryPos(i);

        // Benutzergroesse merken
        if (aIDs[aSizeFormatBox.GetSelectEntryPos()] == (USHORT)SVX_PAPER_USER)
        {
            lUserW = lWidth ;
            lUserH = lHeight;
        }

        aSizeFormatBox.GetSelectHdl().Call(&aSizeFormatBox);
    }
    else
    {
        FillItem(GetParent()->aEnvItem);
        SetMinMax();
        aPreview.Invalidate();
    }
    return 0;
}

// --------------------------------------------------------------------------


void SwEnvFmtPage::SetMinMax()
{
    long lWVal = GetFldVal(aSizeWidthField );
    long lHVal = GetFldVal(aSizeHeightField);

    long lWidth  = Max(lWVal, lHVal),
         lHeight = Min(lWVal, lHVal);

    // Min und Max
    aAddrLeftField.SetMin((long) 100 * (GetFldVal(aSendLeftField) + 566), FUNIT_TWIP);
    aAddrLeftField.SetMax((long) 100 * (lWidth  - 2 * 566), FUNIT_TWIP);
    aAddrTopField .SetMin((long) 100 * (GetFldVal(aSendTopField ) + 2 * 566), FUNIT_TWIP);
    aAddrTopField .SetMax((long) 100 * (lHeight - 2 * 566), FUNIT_TWIP);
    aSendLeftField.SetMin((long) 100 * (566), FUNIT_TWIP);
    aSendLeftField.SetMax((long) 100 * (GetFldVal(aAddrLeftField) - 566), FUNIT_TWIP);
    aSendTopField .SetMin((long) 100 * (566), FUNIT_TWIP);
    aSendTopField .SetMax((long) 100 * (GetFldVal(aAddrTopField ) - 2 * 566), FUNIT_TWIP);

    // First und Last
    aAddrLeftField.SetFirst(aAddrLeftField.GetMin());
    aAddrLeftField.SetLast (aAddrLeftField.GetMax());
    aAddrTopField .SetFirst(aAddrTopField .GetMin());
    aAddrTopField .SetLast (aAddrTopField .GetMax());
    aSendLeftField.SetFirst(aSendLeftField.GetMin());
    aSendLeftField.SetLast (aSendLeftField.GetMax());
    aSendTopField .SetFirst(aSendTopField .GetMin());
    aSendTopField .SetLast (aSendTopField .GetMax());

    // Fields neu formatieren
    aAddrLeftField  .Reformat();
    aAddrTopField   .Reformat();
    aSendLeftField  .Reformat();
    aSendTopField   .Reformat();
    aSizeWidthField .Reformat();
    aSizeHeightField.Reformat();
}

// --------------------------------------------------------------------------


SfxTabPage* __EXPORT SwEnvFmtPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwEnvFmtPage(pParent, rSet);
}

// ----------------------------------------------------------------------------


void __EXPORT SwEnvFmtPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    aSet.Put(GetParent()->aEnvItem);
    Reset(aSet);
}

// ----------------------------------------------------------------------------


int __EXPORT SwEnvFmtPage::DeactivatePage(SfxItemSet* pSet)
{
    FillItemSet(*pSet);
    return SfxTabPage::LEAVE_PAGE;
}

// --------------------------------------------------------------------------


void SwEnvFmtPage::FillItem(SwEnvItem& rItem)
{
    rItem.lAddrFromLeft = GetFldVal(aAddrLeftField);
    rItem.lAddrFromTop  = GetFldVal(aAddrTopField );
    rItem.lSendFromLeft = GetFldVal(aSendLeftField);
    rItem.lSendFromTop  = GetFldVal(aSendTopField );

    USHORT nPaper = aIDs[aSizeFormatBox.GetSelectEntryPos()];
    if (nPaper == (USHORT)SVX_PAPER_USER)
    {
        long lWVal = GetFldVal(aSizeWidthField );
        long lHVal = GetFldVal(aSizeHeightField);
        rItem.lWidth  = Max(lWVal, lHVal);
        rItem.lHeight = Min(lWVal, lHVal);
    }
    else
    {
        long lWVal = SvxPaperInfo::GetPaperSize((SvxPaper)nPaper).Width ();
        long lHVal = SvxPaperInfo::GetPaperSize((SvxPaper)nPaper).Height();
        rItem.lWidth  = Max(lWVal, lHVal);
        rItem.lHeight = Min(lWVal, lHVal);
    }
}

// --------------------------------------------------------------------------


BOOL __EXPORT SwEnvFmtPage::FillItemSet(SfxItemSet& rSet)
{
    FillItem(GetParent()->aEnvItem);
    rSet.Put(GetParent()->aEnvItem);
    return TRUE;
}

// --------------------------------------------------------------------------


void __EXPORT SwEnvFmtPage::Reset(const SfxItemSet& rSet)
{
    const SwEnvItem& rItem = (const SwEnvItem&) rSet.Get(FN_ENVELOP);

    SvxPaper ePaper = SvxPaperInfo::GetPaper(
        Size( Min(rItem.lWidth, rItem.lHeight),
        Max(rItem.lWidth, rItem.lHeight)), MAP_TWIP, TRUE);
    for (USHORT i = 0; i < (USHORT) aIDs.Count(); i++)
        if (aIDs[i] == (USHORT)ePaper)
            aSizeFormatBox.SelectEntryPos(i);

    // Die MetricFields
    SetFldVal(aAddrLeftField, rItem.lAddrFromLeft);
    SetFldVal(aAddrTopField , rItem.lAddrFromTop );
    SetFldVal(aSendLeftField, rItem.lSendFromLeft);
    SetFldVal(aSendTopField , rItem.lSendFromTop );
    SetFldVal(aSizeWidthField  , Max(rItem.lWidth, rItem.lHeight));
    SetFldVal(aSizeHeightField , Min(rItem.lWidth, rItem.lHeight));
    SetMinMax();

    DELETEZ(GetParent()->pSenderSet);
    DELETEZ(GetParent()->pAddresseeSet);
}


/*--------------------------------------------------------------------
$Log: not supported by cvs2svn $
Revision 1.2  2001/02/09 09:07:12  tl
TabPages modification

Revision 1.1.1.1  2000/09/18 17:14:35  hr
initial import

Revision 1.78  2000/09/18 16:05:24  willem.vandorp
OpenOffice header added.

Revision 1.77  2000/04/26 14:44:49  os
GetName() returns const String&

Revision 1.76  2000/04/17 05:36:01  os
#75131# paper formats unlimited

Revision 1.75  2000/02/11 14:45:16  hr
#70473# changes for unicode ( patched by automated patchtool )

Revision 1.74  1998/09/08 14:52:42  OS
#56134# Metric fuer Text und HTML getrennt


      Rev 1.73   08 Sep 1998 16:52:42   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.72   29 May 1998 12:01:42   TJ
   include

      Rev 1.71   09 Apr 1998 14:23:48   OM
   #47097# Undo von Vorlagenaenderungen ermoeglichen

      Rev 1.70   05 Feb 1998 16:47:06   OM
   #47086# Fuer HPUX mit USHORT statt mit SvxPaper arbeiten

      Rev 1.69   05 Feb 1998 15:33:26   OM
   HPUX-Umstellung SvxPaper

      Rev 1.68   05 Feb 1998 14:49:04   OS
   Absatzdialog: Numerierung nicht fuer Umschlaege

      Rev 1.67   24 Nov 1997 11:52:12   MA
   includes

      Rev 1.66   12 Sep 1997 10:42:14   OS
   ITEMID_* definiert

      Rev 1.65   02 Sep 1997 13:23:50   OS
   includes

      Rev 1.64   09 Aug 1997 13:13:14   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.63   08 Aug 1997 17:29:38   OM
   Headerfile-Umstellung

      Rev 1.62   07 Aug 1997 15:00:12   OM
   Headerfile-Umstellung

      Rev 1.61   20 Feb 1997 17:51:38   MA
   fix: Deactivate ueber Fill

      Rev 1.60   11 Nov 1996 09:44:18   MA
   ResMgr

      Rev 1.59   01 Oct 1996 13:31:20   AMA
   Fix: RES_CHRATR_BACKGROUND nur bei Aenderung setzen.

      Rev 1.58   17 Sep 1996 16:55:22   AMA
   Chg: Zeichenhintergrund als Zeichenattribut

      Rev 1.57   26 Jul 1996 20:36:38   MA
   includes

      Rev 1.56   06 Feb 1996 15:18:56   JP
   Link Umstellung 305

      Rev 1.55   05 Feb 1996 17:29:02   OM
   Parent-Win an Par/Chardlg uebergeben

      Rev 1.54   28 Nov 1995 21:14:56   JP
   UiSystem-Klasse aufgehoben, in initui/swtype aufgeteilt

      Rev 1.53   27 Nov 1995 19:35:46   OM
   HasExchangeSupport->303a

      Rev 1.52   24 Nov 1995 16:59:44   OM
   PCH->PRECOMPILED

      Rev 1.51   23 Nov 1995 18:01:28   OM
   TabDlg entfernt

      Rev 1.50   22 Nov 1995 18:57:36   MA
   opt/fix: blc-Warnings

      Rev 1.49   13 Nov 1995 13:44:26   MA
   chg: Link-Cast entfernt

      Rev 1.48   08 Nov 1995 13:48:24   OM
   Change->Set

      Rev 1.47   31 Oct 1995 18:38:20   OM
   GetActive... entfernt

      Rev 1.46   24 Aug 1995 09:25:30   MA
   fix: Ranges verfeinert

      Rev 1.45   22 Aug 1995 09:31:38   MA
   svxitems-header entfernt

      Rev 1.44   16 Aug 1995 17:47:58   MA
   Riesenheader dialogs entfernt.

      Rev 1.43   14 Aug 1995 08:39:28   MA
   fix: Absatz-Dlg, einige Items entfernt.

      Rev 1.42   12 Aug 1995 16:29:18   MA
   fix: fehlende Impl notduerftig nachgeholt

      Rev 1.41   02 Aug 1995 17:39:26   OM
   Datenbankumstellung

      Rev 1.40   30 May 1995 17:01:36   ER
   Umstellung SwPaper...  SvxPaper...

      Rev 1.39   24 May 1995 18:16:48   ER
   Segmentierung

      Rev 1.38   05 Apr 1995 09:26:48   JP
   Benutzung vom Link-Makro eingeschraenkt

      Rev 1.37   30 Mar 1995 18:38:44   PK
   geht immer weiter

      Rev 1.36   24 Mar 1995 20:30:04   PK
   geht immer weiter

      Rev 1.35   23 Mar 1995 18:33:18   PK
   geht immer weiter ...

      Rev 1.34   17 Mar 1995 17:10:02   PK
   geht immer weiter

      Rev 1.33   15 Mar 1995 18:37:42   PK
   geht immer weiter

      Rev 1.32   06 Mar 1995 00:08:14   PK
   linkbarer envelp-zustand

      Rev 1.31   04 Mar 1995 22:54:52   PK
   geht immer weiter

      Rev 1.30   01 Mar 1995 11:12:20   MA
   weg: Sw-BoxItem, neu: KeepItem.

      Rev 1.29   21 Feb 1995 15:39:04   PK
   erstmal eingecheckt

      Rev 1.28   20 Feb 1995 19:39:18   PK
   erstma eingecheckt

--------------------------------------------------------------*/

