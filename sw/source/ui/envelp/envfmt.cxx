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

#include <hintids.hxx>

#include <editeng/paperinf.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brshitem.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/menu.hxx>

#include <cmdid.h>
#include <frmatr.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <basesh.hxx>
#include <drpcps.hxx>
#include <envfmt.hxx>
#include <fmtcol.hxx>
#include "swuipardlg.hxx"
#include <pattern.hxx>
#include <poolfmt.hxx>
#include <uiborder.hxx>
#include <uitool.hxx>

#include <envfmt.hrc>

#include <vector>
#include <algorithm>

#include "swabstdlg.hxx"
#include "chrdlg.hrc"

namespace {
    /// Converts a ranges array to a list containing one entry for each
    /// element covered by the ranges.
    /// @param aRanges An array containing zero or more range specifications and
    ///                terminated by one or more zero entries. A range
    ///                specification is two consecutive entries that specify
    ///                the start and end points of the range.
    /// @returns A vector containing one element for each item covered by the
    ///          ranges. This is not gauranteed to be sorted and may contain
    ///          duplicates if the original ranges contained overlaps.
    static std::vector<sal_uInt16> lcl_convertRangesToList(const sal_uInt16 aRanges[]) {
        std::vector<sal_uInt16> aVec;
        int i = 0;
        while (aRanges[i])
        {
            for (sal_uInt16 n = aRanges[i]; n <= aRanges[i+1]; ++n)
            {
                aVec.push_back(n);
            }
            i += 2;
        }
        return aVec;
    }

    /// Converts a list of elements to a ranges array.
    /// @param rElements Vector of the initial elements, this need not be sorted,
    ///                  and may contain duplicate items. The vector is sorted
    ///                  on exit from this function but may still contain duplicates.
    /// @returns An array containing zero or more range specifications and
    ///          terminated by one or more zero entries. A range specification
    ///          is two consecutive entries that specify the start and end
    ///          points of the range. This list will be sorted and will not
    ///          contain any overlapping ranges.
    static sal_uInt16* lcl_convertListToRanges(std::vector<sal_uInt16> &rElements) {
        std::sort(rElements.begin(), rElements.end());
        std::vector<sal_uInt16> aRanges;
        size_t i;
        for (i = 0; i < rElements.size(); ++i)
        {
            //Push the start of the this range.
            aRanges.push_back(rElements[i]);
            //Seek to the end of this range.
            while (i + 1 < rElements.size() && rElements[i+1] - rElements[i] <= 1)
            {
                ++i;
            }
            //Push the end of this range (may be the same as the start).
            aRanges.push_back( rElements[i] );
        }

        // Convert the vector to an array with terminating zero
        sal_uInt16 *pNewRanges = new sal_uInt16[aRanges.size() + 1];
        for (i = 0; i < aRanges.size(); ++i)
        {
            pNewRanges[i] = aRanges[i];
        }
        pNewRanges[i] = 0;
        return pNewRanges;
    }

}

namespace swui
{
    SwAbstractDialogFactory * GetFactory();
}

static PopupMenu *pMenu;
static long lUserW = 5669; // 10 cm
static long lUserH = 5669; // 10 cm

SwEnvFmtPage::SwEnvFmtPage(Window* pParent, const SfxItemSet& rSet) :

    SfxTabPage(pParent, SW_RES(TP_ENV_FMT), rSet),

    aAddrFL             (this, SW_RES( FL_ADDRESSEE )),
    aAddrPosInfo        (this, SW_RES( TXT_ADDR_POS )),
    aAddrLeftText       (this, SW_RES( TXT_ADDR_LEFT )),
    aAddrLeftField      (this, SW_RES( FLD_ADDR_LEFT )),
    aAddrTopText        (this, SW_RES( TXT_ADDR_TOP )),
    aAddrTopField       (this, SW_RES( FLD_ADDR_TOP )),
    aAddrFormatInfo     (this, SW_RES( TXT_ADDR_FORMAT )),
    aAddrEditButton     (this, SW_RES( BTN_ADDR_EDIT )),
    aSendFL             (this, SW_RES( FL_SENDER )),
    aSendPosInfo        (this, SW_RES( TXT_SEND_POS )),
    aSendLeftText       (this, SW_RES( TXT_SEND_LEFT )),
    aSendLeftField      (this, SW_RES( FLD_SEND_LEFT )),
    aSendTopText        (this, SW_RES( TXT_SEND_TOP )),
    aSendTopField       (this, SW_RES( FLD_SEND_TOP )),
    aSendFormatInfo     (this, SW_RES( TXT_SEND_FORMAT )),
    aSendEditButton     (this, SW_RES( BTN_SEND_EDIT )),
    aSizeFL             (this, SW_RES( FL_SIZE )),
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

    // Metrics
    FieldUnit aMetric = ::GetDfltMetric(sal_False);
    SetMetric(aAddrLeftField,   aMetric);
    SetMetric(aAddrTopField,    aMetric);
    SetMetric(aSendLeftField,   aMetric);
    SetMetric(aSendTopField,    aMetric);
    SetMetric(aSizeWidthField,  aMetric);
    SetMetric(aSizeHeightField, aMetric);

    // Hook in Menues
    ::pMenu = new PopupMenu(SW_RES(MNU_EDIT));
    aAddrEditButton.SetPopupMenu(::pMenu);
    aSendEditButton.SetPopupMenu(::pMenu);

    // Install handlers
    Link aLk = LINK(this, SwEnvFmtPage, ModifyHdl);
    aAddrLeftField  .SetUpHdl( aLk );
    aAddrTopField   .SetUpHdl( aLk );
    aSendLeftField  .SetUpHdl( aLk );
    aSendTopField   .SetUpHdl( aLk );
    aSizeWidthField .SetUpHdl( aLk );
    aSizeHeightField.SetUpHdl( aLk );

    aAddrLeftField  .SetDownHdl( aLk );
    aAddrTopField   .SetDownHdl( aLk );
    aSendLeftField  .SetDownHdl( aLk );
    aSendTopField   .SetDownHdl( aLk );
    aSizeWidthField .SetDownHdl( aLk );
    aSizeHeightField.SetDownHdl( aLk );

    aAddrLeftField  .SetLoseFocusHdl( aLk );
    aAddrTopField   .SetLoseFocusHdl( aLk );
    aSendLeftField  .SetLoseFocusHdl( aLk );
    aSendTopField   .SetLoseFocusHdl( aLk );
    aSizeWidthField .SetLoseFocusHdl( aLk );
    aSizeHeightField.SetLoseFocusHdl( aLk );

    aLk = LINK(this, SwEnvFmtPage, EditHdl );
    aAddrEditButton.SetSelectHdl( aLk );
    aSendEditButton.SetSelectHdl( aLk );

    aPreview.SetBorderStyle( WINDOW_BORDER_MONO );

    aSizeFormatBox     .SetSelectHdl(LINK(this, SwEnvFmtPage, FormatHdl));

    // aSizeFormatBox
    for (sal_uInt16 i = PAPER_A3; i <= PAPER_KAI32BIG; i++)
    {
        if (i != PAPER_USER)
        {
            String aPaperName = SvxPaperInfo::GetName((Paper) i),
                   aEntryName;

            sal_uInt16 nPos   = 0;
            sal_Bool   bFound = sal_False;
            while (nPos < aSizeFormatBox.GetEntryCount() && !bFound)
            {
                aEntryName = aSizeFormatBox.GetEntry(i);
                if (aEntryName < aPaperName)
                    nPos++;
                else
                    bFound = sal_True;
            }
            aSizeFormatBox.InsertEntry(aPaperName, nPos);
            aIDs.insert( aIDs.begin() + nPos, (sal_uInt16) i);
        }
    }
    aSizeFormatBox.InsertEntry(SvxPaperInfo::GetName(PAPER_USER));
    aIDs.push_back( (sal_uInt16) PAPER_USER );

}

SwEnvFmtPage::~SwEnvFmtPage()
{
    aAddrEditButton.SetPopupMenu(0);
    aSendEditButton.SetPopupMenu(0);
    delete ::pMenu;
}

IMPL_LINK_INLINE_START( SwEnvFmtPage, ModifyHdl, Edit *, pEdit )
{
    long lWVal = static_cast< long >(GetFldVal(aSizeWidthField ));
    long lHVal = static_cast< long >(GetFldVal(aSizeHeightField));

    long lWidth  = Max(lWVal, lHVal);
    long lHeight = Min(lWVal, lHVal);

    if (pEdit == &aSizeWidthField || pEdit == &aSizeHeightField)
    {
        Paper ePaper = SvxPaperInfo::GetSvxPaper(
            Size(lHeight, lWidth), MAP_TWIP, sal_True);
        for (sal_uInt16 i = 0; i < (sal_uInt16)aIDs.size(); i++)
            if (aIDs[i] == (sal_uInt16)ePaper)
                aSizeFormatBox.SelectEntryPos(i);

        // remember user size
        if (aIDs[aSizeFormatBox.GetSelectEntryPos()] == (sal_uInt16)PAPER_USER)
        {
            lUserW = lWidth ;
            lUserH = lHeight;
        }

        aSizeFormatBox.GetSelectHdl().Call(&aSizeFormatBox);
    }
    else
    {
        FillItem(GetParentSwEnvDlg()->aEnvItem);
        SetMinMax();
        aPreview.Invalidate();
    }
    return 0;
}
IMPL_LINK_INLINE_END( SwEnvFmtPage, ModifyHdl, Edit *, pEdit )

IMPL_LINK( SwEnvFmtPage, EditHdl, MenuButton *, pButton )
{
    SwWrtShell* pSh = GetParentSwEnvDlg()->pSh;
    OSL_ENSURE(pSh, "Shell missing");

    // determine collection-ptr
    sal_Bool bSender = pButton != &aAddrEditButton;

    SwTxtFmtColl* pColl = pSh->GetTxtCollFromPool( static_cast< sal_uInt16 >(
        bSender ? RES_POOLCOLL_SENDADRESS : RES_POOLCOLL_JAKETADRESS));
    OSL_ENSURE(pColl, "Text collection missing");

    switch (pButton->GetCurItemId())
    {
        case MID_CHAR:
        {
            SfxItemSet *pCollSet = GetCollItemSet(pColl, bSender);

            // In order for the background color not to get ironed over:
            SfxAllItemSet aTmpSet(*pCollSet);

            // The CHRATR_BACKGROUND attribute gets transformed into a
            // RES_BACKGROUND for the dialog and back again ...
            const SfxPoolItem *pTmpBrush;

            if( SFX_ITEM_SET == aTmpSet.GetItemState( RES_CHRATR_BACKGROUND,
                sal_True, &pTmpBrush ) )
            {
                SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                aTmpBrush.SetWhich( RES_BACKGROUND );
                aTmpSet.Put( aTmpBrush );
            }
            else
                aTmpSet.ClearItem( RES_BACKGROUND );

            SwAbstractDialogFactory* pFact = swui::GetFactory();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            SfxAbstractTabDialog* pDlg = pFact->CreateSwCharDlg( GetParentSwEnvDlg(), pSh->GetView(), aTmpSet, DLG_CHAR ,&pColl->GetName() );
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            if (pDlg->Execute() == RET_OK)
            {
                SfxItemSet aOutputSet( *pDlg->GetOutputItemSet() );
                if( SFX_ITEM_SET == aOutputSet.GetItemState( RES_BACKGROUND,
                    sal_False, &pTmpBrush ) )
                {
                    SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                    aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
                    pCollSet->Put( aTmpBrush );
                }
                aOutputSet.ClearItem( RES_BACKGROUND );
                //pColl->SetAttr( aTmpSet );
                pCollSet->Put(aOutputSet);
            }
            delete pDlg;
        }
        break;

        case MID_PARA:
        {
            SfxItemSet *pCollSet = GetCollItemSet(pColl, bSender);

            // In order for the tabulators not to get ironed over:
            SfxAllItemSet aTmpSet(*pCollSet);

            // Insert tabs, default tabs into ItemSet
            const SvxTabStopItem& rDefTabs = (const SvxTabStopItem&)
                pSh->GetView().GetCurShell()->GetPool().GetDefaultItem(RES_PARATR_TABSTOP);

            sal_uInt16 nDefDist = ::GetTabDist( rDefTabs );
            SfxUInt16Item aDefDistItem( SID_ATTR_TABSTOP_DEFAULTS, nDefDist );
            aTmpSet.Put( aDefDistItem );

            // Current tab
            SfxUInt16Item aTabPos( SID_ATTR_TABSTOP_POS, 0 );
            aTmpSet.Put( aTabPos );

            // left border as offset
            const long nOff = ((SvxLRSpaceItem&)aTmpSet.Get( RES_LR_SPACE )).
                                                                GetTxtLeft();
            SfxInt32Item aOff( SID_ATTR_TABSTOP_OFFSET, nOff );
            aTmpSet.Put( aOff );

            // set BoxInfo
            ::PrepareBoxInfo( aTmpSet, *pSh );

            SwParaDlg *pDlg = new SwParaDlg(GetParentSwEnvDlg(), pSh->GetView(), aTmpSet, DLG_ENVELOP, &pColl->GetName());

            if ( pDlg->Execute() == RET_OK )
            {
                // maybe relocate defaults
                const SfxPoolItem* pItem = 0;
                SfxItemSet* pOutputSet = (SfxItemSet*)pDlg->GetOutputItemSet();
                sal_uInt16 nNewDist;

                if( SFX_ITEM_SET == pOutputSet->GetItemState( SID_ATTR_TABSTOP_DEFAULTS,
                    sal_False, &pItem ) &&
                    nDefDist != (nNewDist = ((SfxUInt16Item*)pItem)->GetValue()) )
                {
                    SvxTabStopItem aDefTabs( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
                    MakeDefTabs( nNewDist, aDefTabs );
                    pSh->SetDefault( aDefTabs );
                    pOutputSet->ClearItem( SID_ATTR_TABSTOP_DEFAULTS );
                }
                if( pOutputSet->Count() )
                {
                    pCollSet->Put(*pOutputSet);
                }
            }
            delete pDlg;
        }
        break;
    }
    return 0;
}

/*------------------------------------------------------------------------
  Description: A temporary Itemset that gets discarded at abort
------------------------------------------------------------------------*/

SfxItemSet *SwEnvFmtPage::GetCollItemSet(SwTxtFmtColl* pColl, sal_Bool bSender)
{
    SfxItemSet *&pAddrSet = bSender ? GetParentSwEnvDlg()->pSenderSet : GetParentSwEnvDlg()->pAddresseeSet;

    if (!pAddrSet)
    {
        // determine range (merge both Itemsets' ranges)
        const sal_uInt16 *pRanges = pColl->GetAttrSet().GetRanges();

        static sal_uInt16 const aRanges[] =
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

        // BruteForce merge because MergeRange in SvTools is buggy:
        std::vector<sal_uInt16> pVec = ::lcl_convertRangesToList(pRanges);
        std::vector<sal_uInt16> aVec = ::lcl_convertRangesToList(aRanges);
        pVec.insert(pVec.end(), aVec.begin(), aVec.end());
        sal_uInt16 *pNewRanges = ::lcl_convertListToRanges(pVec);

        pAddrSet = new SfxItemSet(GetParentSwEnvDlg()->pSh->GetView().GetCurShell()->GetPool(),
                                pNewRanges);
        pAddrSet->Put(pColl->GetAttrSet());
        delete[] pNewRanges;
    }

    return pAddrSet;
}

IMPL_LINK_NOARG(SwEnvFmtPage, FormatHdl)
{
    long lWidth;
    long lHeight;
    long lSendFromLeft;
    long lSendFromTop;
    long lAddrFromLeft;
    long lAddrFromTop;

    sal_uInt16 nPaper = aIDs[aSizeFormatBox.GetSelectEntryPos()];
    if (nPaper != (sal_uInt16)PAPER_USER)
    {
        Size aSz = SvxPaperInfo::GetPaperSize((Paper)nPaper);
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

    FillItem(GetParentSwEnvDlg()->aEnvItem);
    aPreview.Invalidate();
    return 0;
}

void SwEnvFmtPage::SetMinMax()
{
    long lWVal = static_cast< long >(GetFldVal(aSizeWidthField ));
    long lHVal = static_cast< long >(GetFldVal(aSizeHeightField));

    long lWidth  = Max(lWVal, lHVal),
         lHeight = Min(lWVal, lHVal);

    // Min and Max
    aAddrLeftField.SetMin((long) 100 * (GetFldVal(aSendLeftField) + 566), FUNIT_TWIP);
    aAddrLeftField.SetMax((long) 100 * (lWidth  - 2 * 566), FUNIT_TWIP);
    aAddrTopField .SetMin((long) 100 * (GetFldVal(aSendTopField ) + 2 * 566), FUNIT_TWIP);
    aAddrTopField .SetMax((long) 100 * (lHeight - 2 * 566), FUNIT_TWIP);
    aSendLeftField.SetMin((long) 100 * (566), FUNIT_TWIP);
    aSendLeftField.SetMax((long) 100 * (GetFldVal(aAddrLeftField) - 566), FUNIT_TWIP);
    aSendTopField .SetMin((long) 100 * (566), FUNIT_TWIP);
    aSendTopField .SetMax((long) 100 * (GetFldVal(aAddrTopField ) - 2 * 566), FUNIT_TWIP);

    // First and last
    aAddrLeftField.SetFirst(aAddrLeftField.GetMin());
    aAddrLeftField.SetLast (aAddrLeftField.GetMax());
    aAddrTopField .SetFirst(aAddrTopField .GetMin());
    aAddrTopField .SetLast (aAddrTopField .GetMax());
    aSendLeftField.SetFirst(aSendLeftField.GetMin());
    aSendLeftField.SetLast (aSendLeftField.GetMax());
    aSendTopField .SetFirst(aSendTopField .GetMin());
    aSendTopField .SetLast (aSendTopField .GetMax());

    // Reformat fields
    aAddrLeftField  .Reformat();
    aAddrTopField   .Reformat();
    aSendLeftField  .Reformat();
    aSendTopField   .Reformat();
    aSizeWidthField .Reformat();
    aSizeHeightField.Reformat();
}

SfxTabPage* SwEnvFmtPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwEnvFmtPage(pParent, rSet);
}

void SwEnvFmtPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemSet aSet(rSet);
    aSet.Put(GetParentSwEnvDlg()->aEnvItem);
    Reset(aSet);
}

int SwEnvFmtPage::DeactivatePage(SfxItemSet* _pSet)
{
    if( _pSet )
        FillItemSet(*_pSet);
    return SfxTabPage::LEAVE_PAGE;
}

void SwEnvFmtPage::FillItem(SwEnvItem& rItem)
{
    rItem.lAddrFromLeft = static_cast< sal_Int32 >(GetFldVal(aAddrLeftField));
    rItem.lAddrFromTop  = static_cast< sal_Int32 >(GetFldVal(aAddrTopField ));
    rItem.lSendFromLeft = static_cast< sal_Int32 >(GetFldVal(aSendLeftField));
    rItem.lSendFromTop  = static_cast< sal_Int32 >(GetFldVal(aSendTopField ));

    sal_uInt16 nPaper = aIDs[aSizeFormatBox.GetSelectEntryPos()];
    if (nPaper == (sal_uInt16)PAPER_USER)
    {
        long lWVal = static_cast< long >(GetFldVal(aSizeWidthField ));
        long lHVal = static_cast< long >(GetFldVal(aSizeHeightField));
        rItem.lWidth  = Max(lWVal, lHVal);
        rItem.lHeight = Min(lWVal, lHVal);
    }
    else
    {
        long lWVal = SvxPaperInfo::GetPaperSize((Paper)nPaper).Width ();
        long lHVal = SvxPaperInfo::GetPaperSize((Paper)nPaper).Height();
        rItem.lWidth  = Max(lWVal, lHVal);
        rItem.lHeight = Min(lWVal, lHVal);
    }
}

sal_Bool SwEnvFmtPage::FillItemSet(SfxItemSet& rSet)
{
    FillItem(GetParentSwEnvDlg()->aEnvItem);
    rSet.Put(GetParentSwEnvDlg()->aEnvItem);
    return sal_True;
}

void SwEnvFmtPage::Reset(const SfxItemSet& rSet)
{
    const SwEnvItem& rItem = (const SwEnvItem&) rSet.Get(FN_ENVELOP);

    Paper ePaper = SvxPaperInfo::GetSvxPaper(
        Size( Min(rItem.lWidth, rItem.lHeight),
        Max(rItem.lWidth, rItem.lHeight)), MAP_TWIP, sal_True);
    for (sal_uInt16 i = 0; i < (sal_uInt16) aIDs.size(); i++)
        if (aIDs[i] == (sal_uInt16)ePaper)
            aSizeFormatBox.SelectEntryPos(i);

    // Metric fields
    SetFldVal(aAddrLeftField, rItem.lAddrFromLeft);
    SetFldVal(aAddrTopField , rItem.lAddrFromTop );
    SetFldVal(aSendLeftField, rItem.lSendFromLeft);
    SetFldVal(aSendTopField , rItem.lSendFromTop );
    SetFldVal(aSizeWidthField  , Max(rItem.lWidth, rItem.lHeight));
    SetFldVal(aSizeHeightField , Min(rItem.lWidth, rItem.lHeight));
    SetMinMax();

    DELETEZ(GetParentSwEnvDlg()->pSenderSet);
    DELETEZ(GetParentSwEnvDlg()->pAddresseeSet);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
