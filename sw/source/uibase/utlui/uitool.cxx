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

#include <hintids.hxx>

#include <osl/diagnose.h>
#include <tools/datetime.hxx>
#include <vcl/svapp.hxx>
#include <unotools/collatorwrapper.hxx>
#include <svl/urihelper.hxx>
#include <svl/stritem.hxx>
#include <unotools/syslocale.hxx>
#include <sfx2/app.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <editeng/pmdlitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/brushitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svl/style.hxx>
#include <vcl/lstbox.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <fmtornt.hxx>
#include <tabcol.hxx>
#include <edtwin.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtpdsc.hxx>
#include <wview.hxx>
#include <uiitems.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <uitool.hxx>
#include <frmatr.hxx>
#include <paratr.hxx>
#include <fmtcol.hxx>
#include <poolfmt.hxx>
#include "usrpref.hxx"

#include <cmdid.h>
#include <globals.hrc>
#include <utlui.hrc>
#include <doc.hxx>
#include <docary.hxx>
#include <charfmt.hxx>
#include <SwStyleNameMapper.hxx>
// 50 cm 28350

#define MAXHEIGHT 28350
#define MAXWIDTH  28350

using namespace ::com::sun::star;

// General list of string pointer

// Switch metric

void SetMetric(MetricFormatter& rCtrl, FieldUnit eUnit)
{
    SwTwips nMin = static_cast< SwTwips >(rCtrl.GetMin(FUNIT_TWIP));
    SwTwips nMax = static_cast< SwTwips >(rCtrl.GetMax(FUNIT_TWIP));

    rCtrl.SetUnit(eUnit);

    rCtrl.SetMin(nMin, FUNIT_TWIP);
    rCtrl.SetMax(nMax, FUNIT_TWIP);
}

// Set boxinfo attribute

void PrepareBoxInfo(SfxItemSet& rSet, const SwWrtShell& rSh)
{
    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
    const SfxPoolItem *pBoxInfo;
    if ( SfxItemState::SET == rSet.GetItemState( SID_ATTR_BORDER_INNER,
                                        true, &pBoxInfo))
        aBoxInfo = *(SvxBoxInfoItem*)pBoxInfo;

        // Table variant: If more than one table cells are selected
    rSh.GetCrsr();                  //So that GetCrsrCnt() returns the right thing
    aBoxInfo.SetTable          (rSh.IsTableMode() && rSh.GetCrsrCnt() > 1);
        // Always show the distance field
    aBoxInfo.SetDist           (true);
        // Set minimal size in tables and paragraphs
    aBoxInfo.SetMinDist        (rSh.IsTableMode() || rSh.GetSelectionType() & (nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL));
        // Set always the default distance
    aBoxInfo.SetDefDist        (MIN_BORDER_DIST);
        // Single lines can have only in tables DontCare-Status
    aBoxInfo.SetValid(VALID_DISABLE, !rSh.IsTableMode());

    rSet.Put(aBoxInfo);
}

void ConvertAttrCharToGen(SfxItemSet& rSet, const sal_uInt8 nMode)
{
    // Background
    {
        const SfxPoolItem *pTmpBrush;
        if( SfxItemState::SET == rSet.GetItemState( RES_CHRATR_BACKGROUND, true, &pTmpBrush ) )
        {
            SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
            aTmpBrush.SetWhich( RES_BACKGROUND );
            rSet.Put( aTmpBrush );
        }
        else
            rSet.ClearItem(RES_BACKGROUND);
    }

    if( nMode == CONV_ATTR_STD )
    {
        // Border
        const SfxPoolItem *pTmpItem;
        if( SfxItemState::SET == rSet.GetItemState( RES_CHRATR_BOX, true, &pTmpItem ) )
        {
            SvxBoxItem aTmpBox( *((SvxBoxItem*)pTmpItem) );
            aTmpBox.SetWhich( RES_BOX );
            rSet.Put( aTmpBox );
        }
        else
            rSet.ClearItem(RES_BOX);

        // Border shadow
        if( SfxItemState::SET == rSet.GetItemState( RES_CHRATR_SHADOW, false, &pTmpItem ) )
        {
            SvxShadowItem aTmpShadow( *((SvxShadowItem*)pTmpItem) );
            aTmpShadow.SetWhich( RES_SHADOW );
            rSet.Put( aTmpShadow );
        }
        else
            rSet.ClearItem( RES_SHADOW );
    }
}

void ConvertAttrGenToChar(SfxItemSet& rSet, const sal_uInt8 nMode)
{
    // Background
    {
        const SfxPoolItem *pTmpBrush;
        if( SfxItemState::SET == rSet.GetItemState( RES_BACKGROUND, false, &pTmpBrush ) )
        {
            SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
            aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
            rSet.Put( aTmpBrush );
        }
        rSet.ClearItem( RES_BACKGROUND );
    }

    if( nMode == CONV_ATTR_STD )
    {
        // Border
        const SfxPoolItem *pTmpItem;
        if( SfxItemState::SET == rSet.GetItemState( RES_BOX, false, &pTmpItem ) )
        {
            SvxBoxItem aTmpBox( *((SvxBoxItem*)pTmpItem) );
            aTmpBox.SetWhich( RES_CHRATR_BOX );
            rSet.Put( aTmpBox );
        }
        rSet.ClearItem( RES_BOX );

        // Border shadow
        if( SfxItemState::SET == rSet.GetItemState( RES_SHADOW, false, &pTmpItem ) )
        {
            SvxShadowItem aTmpShadow( *((SvxShadowItem*)pTmpItem) );
            aTmpShadow.SetWhich( RES_CHRATR_SHADOW );
            rSet.Put( aTmpShadow );
        }
        rSet.ClearItem( RES_SHADOW );
    }
}

// Fill header footer

void FillHdFt(SwFrmFmt* pFmt, const  SfxItemSet& rSet)
{
    SwAttrSet aSet(pFmt->GetAttrSet());
    aSet.Put(rSet);

    const SvxSizeItem& rSize = (const SvxSizeItem&)rSet.Get(SID_ATTR_PAGE_SIZE);
    const SfxBoolItem& rDynamic = (const SfxBoolItem&)rSet.Get(SID_ATTR_PAGE_DYNAMIC);

    // Convert size
    SwFmtFrmSize aFrmSize(rDynamic.GetValue() ? ATT_MIN_SIZE : ATT_FIX_SIZE,
                            rSize.GetSize().Width(),
                            rSize.GetSize().Height());
    aSet.Put(aFrmSize);
    pFmt->SetFmtAttr(aSet);
}

/// Convert from UseOnPage to SvxPageUsage.
UseOnPage lcl_convertUseToSvx(UseOnPage nUse)
{
    UseOnPage nRet = nsUseOnPage::PD_NONE;
    if ((nUse & nsUseOnPage::PD_LEFT) == nsUseOnPage::PD_LEFT)
        nRet |= SVX_PAGE_LEFT;
    if ((nUse & nsUseOnPage::PD_RIGHT) == nsUseOnPage::PD_RIGHT)
        nRet |= SVX_PAGE_RIGHT;
    if ((nUse & nsUseOnPage::PD_ALL) == nsUseOnPage::PD_ALL)
        nRet |= SVX_PAGE_ALL;
    if ((nUse & nsUseOnPage::PD_MIRROR) == nsUseOnPage::PD_MIRROR)
        nRet |= SVX_PAGE_MIRROR;
    return nRet;
}

/// Convert from SvxPageUsage to UseOnPage.
UseOnPage lcl_convertUseFromSvx(UseOnPage nUse)
{
    UseOnPage nRet = nsUseOnPage::PD_NONE;
    if ((nUse & SVX_PAGE_LEFT) == SVX_PAGE_LEFT)
        nRet |= nsUseOnPage::PD_LEFT;
    if ((nUse & SVX_PAGE_RIGHT) == SVX_PAGE_RIGHT)
        nRet |= nsUseOnPage::PD_RIGHT;
    if ((nUse & SVX_PAGE_ALL) == SVX_PAGE_ALL)
        nRet |= nsUseOnPage::PD_ALL;
    if ((nUse & SVX_PAGE_MIRROR) == SVX_PAGE_MIRROR)
        nRet |= nsUseOnPage::PD_MIRROR;
    return nRet;
}

// PageDesc <-> convert into sets and back

void ItemSetToPageDesc( const SfxItemSet& rSet, SwPageDesc& rPageDesc )
{
    SwFrmFmt& rMaster = rPageDesc.GetMaster();
    int nFirstShare = -1;

    // Transfer all general frame attributes
    rMaster.SetFmtAttr(rSet);

    // PageData
    if(rSet.GetItemState(SID_ATTR_PAGE) == SfxItemState::SET)
    {
        const SvxPageItem& rPageItem = (const SvxPageItem&)rSet.Get(SID_ATTR_PAGE);

        const sal_uInt16 nUse = rPageItem.GetPageUsage();
        if(nUse)
            rPageDesc.SetUseOn( lcl_convertUseFromSvx((UseOnPage) nUse) );
        rPageDesc.SetLandscape(rPageItem.IsLandscape());
        SvxNumberType aNumType;
        aNumType.SetNumberingType( static_cast< sal_Int16 >(rPageItem.GetNumType()) );
        rPageDesc.SetNumType(aNumType);
    }
    // Size
    if(rSet.GetItemState(SID_ATTR_PAGE_SIZE) == SfxItemState::SET)
    {
        const SvxSizeItem& rSizeItem = (const SvxSizeItem&)rSet.Get(SID_ATTR_PAGE_SIZE);
        SwFmtFrmSize aSize(ATT_FIX_SIZE);
        aSize.SetSize(rSizeItem.GetSize());
        rMaster.SetFmtAttr(aSize);
    }
    // Evaluate header attributes
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_PAGE_HEADERSET,
            false, &pItem ) )
    {
        const SfxItemSet& rHeaderSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rHeaderOn = (const SfxBoolItem&)rHeaderSet.Get(SID_ATTR_PAGE_ON);

        if(rHeaderOn.GetValue())
        {
            // Take over values
            if(!rMaster.GetHeader().IsActive())
                rMaster.SetFmtAttr(SwFmtHeader(true));

            // Pick out everything and adapt the header format
            SwFmtHeader aHeaderFmt(rMaster.GetHeader());
            SwFrmFmt *pHeaderFmt = aHeaderFmt.GetHeaderFmt();
            OSL_ENSURE(pHeaderFmt != 0, "no header format");

            ::FillHdFt(pHeaderFmt, rHeaderSet);

            rPageDesc.ChgHeaderShare(((const SfxBoolItem&)
                        rHeaderSet.Get(SID_ATTR_PAGE_SHARED)).GetValue());
            if (nFirstShare < 0)
            {
                rPageDesc.ChgFirstShare(((const SfxBoolItem&)
                            rHeaderSet.Get(SID_ATTR_PAGE_SHARED_FIRST)).GetValue());
                nFirstShare = rPageDesc.IsFirstShared() ? 1 : 0;
            }
        }
        else
        {
            // Disable header
            if(rMaster.GetHeader().IsActive())
            {
                rMaster.SetFmtAttr(SwFmtHeader(false));
                rPageDesc.ChgHeaderShare(false);
            }
        }
    }

    // Evaluate footer attributes
    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_PAGE_FOOTERSET,
            false, &pItem ) )
    {
        const SfxItemSet& rFooterSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rFooterOn = (const SfxBoolItem&)rFooterSet.Get(SID_ATTR_PAGE_ON);

        if(rFooterOn.GetValue())
        {
            // Take over values
            if(!rMaster.GetFooter().IsActive())
                rMaster.SetFmtAttr(SwFmtFooter(true));

            // Pick out everything and adapt the footer format
            SwFmtFooter aFooterFmt(rMaster.GetFooter());
            SwFrmFmt *pFooterFmt = aFooterFmt.GetFooterFmt();
            OSL_ENSURE(pFooterFmt != 0, "no footer format");

            ::FillHdFt(pFooterFmt, rFooterSet);

            rPageDesc.ChgFooterShare(((const SfxBoolItem&)
                        rFooterSet.Get(SID_ATTR_PAGE_SHARED)).GetValue());
            if (nFirstShare < 0)
            {
                rPageDesc.ChgFirstShare(((const SfxBoolItem&)
                            rFooterSet.Get(SID_ATTR_PAGE_SHARED_FIRST)).GetValue());
                nFirstShare = rPageDesc.IsFirstShared() ? 1 : 0;
            }
        }
        else
        {
            // Disable footer
            if(rMaster.GetFooter().IsActive())
            {
                rMaster.SetFmtAttr(SwFmtFooter(false));
                rPageDesc.ChgFooterShare(false);
            }
        }
    }

    // Footnotes

    if( SfxItemState::SET == rSet.GetItemState( FN_PARAM_FTN_INFO,
            false, &pItem ) )
        rPageDesc.SetFtnInfo( ((SwPageFtnInfoItem*)pItem)->GetPageFtnInfo() );

    // Columns

    // Register compliant

    if(SfxItemState::SET == rSet.GetItemState(
                            SID_SWREGISTER_MODE, false, &pItem))
    {
        bool bSet = ((const SfxBoolItem*)pItem)->GetValue();
        if(!bSet)
            rPageDesc.SetRegisterFmtColl(0);
        else if(SfxItemState::SET == rSet.GetItemState(
                                SID_SWREGISTER_COLLECTION, false, &pItem))
        {
            const OUString& rColl = ((const SfxStringItem*)pItem)->GetValue();
            SwDoc& rDoc = *rMaster.GetDoc();
            SwTxtFmtColl* pColl = rDoc.FindTxtFmtCollByName( rColl );
            if( !pColl )
            {
                const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(
                    rColl, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if( USHRT_MAX != nId )
                    pColl = rDoc.getIDocumentStylePoolAccess().GetTxtCollFromPool( nId );
                else
                    pColl = rDoc.MakeTxtFmtColl( rColl,
                                (SwTxtFmtColl*)rDoc.GetDfltTxtFmtColl() );
            }
            if( pColl )
                pColl->SetFmtAttr( SwRegisterItem ( true ));
            rPageDesc.SetRegisterFmtColl( pColl );
        }
    }
}

void PageDescToItemSet( const SwPageDesc& rPageDesc, SfxItemSet& rSet)
{
    const SwFrmFmt& rMaster = rPageDesc.GetMaster();

    // Page data
    SvxPageItem aPageItem(SID_ATTR_PAGE);
    aPageItem.SetDescName(rPageDesc.GetName());
    aPageItem.SetPageUsage(lcl_convertUseToSvx(rPageDesc.GetUseOn()));
    aPageItem.SetLandscape(rPageDesc.GetLandscape());
    aPageItem.SetNumType((SvxNumType)rPageDesc.GetNumType().GetNumberingType());
    rSet.Put(aPageItem);

    // Size
    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, rMaster.GetFrmSize().GetSize());
    rSet.Put(aSizeItem);

    // Maximum size
    SvxSizeItem aMaxSizeItem(SID_ATTR_PAGE_MAXSIZE, Size(MAXWIDTH, MAXHEIGHT));
    rSet.Put(aMaxSizeItem);

    // Margins, border and the other stuff.
    rSet.Put(rMaster.GetAttrSet());

    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
    const SfxPoolItem *pBoxInfo;
    if ( SfxItemState::SET == rSet.GetItemState( SID_ATTR_BORDER_INNER,
                                            true, &pBoxInfo) )
        aBoxInfo = *(SvxBoxInfoItem*)pBoxInfo;

    aBoxInfo.SetTable( false );
        // Show always the distance field
    aBoxInfo.SetDist( true);
        // Set minimal size in tables and paragraphs
    aBoxInfo.SetMinDist( false );
        // Set always the default distance
    aBoxInfo.SetDefDist( MIN_BORDER_DIST );
        // Single lines can have only in tables DontCare-Status
    aBoxInfo.SetValid( VALID_DISABLE );
    rSet.Put( aBoxInfo );

    SfxStringItem aFollow(SID_ATTR_PAGE_EXT1, OUString());
    if(rPageDesc.GetFollow())
        aFollow.SetValue(rPageDesc.GetFollow()->GetName());
    rSet.Put(aFollow);

    // Header
    if(rMaster.GetHeader().IsActive())
    {
        const SwFmtHeader &rHeaderFmt = rMaster.GetHeader();
        const SwFrmFmt *pHeaderFmt = rHeaderFmt.GetHeaderFmt();
        OSL_ENSURE(pHeaderFmt != 0, "no header format");

        // HeaderInfo, margins, background, border
        SfxItemSet aHeaderSet(*rSet.GetPool(),
            RES_FRMATR_BEGIN,RES_FRMATR_END - 1,            // [82

            //UUUU FillAttribute support
            XATTR_FILL_FIRST, XATTR_FILL_LAST,              // [1014

            SID_ATTR_BORDER_INNER,SID_ATTR_BORDER_INNER,    // [10023
            SID_ATTR_PAGE_SIZE,SID_ATTR_PAGE_SIZE,          // [10051
            SID_ATTR_PAGE_ON,SID_ATTR_PAGE_SHARED,          // [10060
            SID_ATTR_PAGE_SHARED_FIRST,SID_ATTR_PAGE_SHARED_FIRST,
            0, 0);

        //UUUU set correct parent to get the XFILL_NONE FillStyle as needed
        aHeaderSet.SetParent(&rMaster.GetDoc()->GetDfltFrmFmt()->GetAttrSet());

        // Dynamic or fixed height
        SfxBoolItem aOn(SID_ATTR_PAGE_ON, true);
        aHeaderSet.Put(aOn);

        const SwFmtFrmSize &rFrmSize = pHeaderFmt->GetFrmSize();
        const SwFrmSize eSizeType = rFrmSize.GetHeightSizeType();
        SfxBoolItem aDynamic(SID_ATTR_PAGE_DYNAMIC, eSizeType != ATT_FIX_SIZE);
        aHeaderSet.Put(aDynamic);

        // Left equal right
        SfxBoolItem aShared(SID_ATTR_PAGE_SHARED, rPageDesc.IsHeaderShared());
        aHeaderSet.Put(aShared);
        SfxBoolItem aFirstShared(SID_ATTR_PAGE_SHARED_FIRST, rPageDesc.IsFirstShared());
        aHeaderSet.Put(aFirstShared);

        // Size
        SvxSizeItem aSize(SID_ATTR_PAGE_SIZE, Size(rFrmSize.GetSize()));
        aHeaderSet.Put(aSize);

        // Shifting frame attributes
        aHeaderSet.Put(pHeaderFmt->GetAttrSet());
        aHeaderSet.Put( aBoxInfo );

        // Create SetItem
        SvxSetItem aSetItem(SID_ATTR_PAGE_HEADERSET, aHeaderSet);
        rSet.Put(aSetItem);
    }

    // Footer
    if(rMaster.GetFooter().IsActive())
    {
        const SwFmtFooter &rFooterFmt = rMaster.GetFooter();
        const SwFrmFmt *pFooterFmt = rFooterFmt.GetFooterFmt();
        OSL_ENSURE(pFooterFmt != 0, "no footer format");

        // FooterInfo, margins, background, border
        SfxItemSet aFooterSet(*rSet.GetPool(),
            RES_FRMATR_BEGIN,RES_FRMATR_END - 1,            // [82

            //UUUU FillAttribute support
            XATTR_FILL_FIRST, XATTR_FILL_LAST,              // [1014

            SID_ATTR_BORDER_INNER,SID_ATTR_BORDER_INNER,    // [10023
            SID_ATTR_PAGE_SIZE,SID_ATTR_PAGE_SIZE,          // [10051
            SID_ATTR_PAGE_ON,SID_ATTR_PAGE_SHARED,          // [10060
            SID_ATTR_PAGE_SHARED_FIRST,SID_ATTR_PAGE_SHARED_FIRST,
            0, 0);

        //UUUU set correct parent to get the XFILL_NONE FillStyle as needed
        aFooterSet.SetParent(&rMaster.GetDoc()->GetDfltFrmFmt()->GetAttrSet());

        // Dynamic or fixed height
        SfxBoolItem aOn(SID_ATTR_PAGE_ON, true);
        aFooterSet.Put(aOn);

        const SwFmtFrmSize &rFrmSize = pFooterFmt->GetFrmSize();
        const SwFrmSize eSizeType = rFrmSize.GetHeightSizeType();
        SfxBoolItem aDynamic(SID_ATTR_PAGE_DYNAMIC, eSizeType != ATT_FIX_SIZE);
        aFooterSet.Put(aDynamic);

        // Left equal right
        SfxBoolItem aShared(SID_ATTR_PAGE_SHARED, rPageDesc.IsFooterShared());
        aFooterSet.Put(aShared);
        SfxBoolItem aFirstShared(SID_ATTR_PAGE_SHARED_FIRST, rPageDesc.IsFirstShared());
        aFooterSet.Put(aFirstShared);

        // Size
        SvxSizeItem aSize(SID_ATTR_PAGE_SIZE, Size(rFrmSize.GetSize()));
        aFooterSet.Put(aSize);

        // Shifting Frame attributes
        aFooterSet.Put(pFooterFmt->GetAttrSet());
        aFooterSet.Put( aBoxInfo );

        // Create SetItem
        SvxSetItem aSetItem(SID_ATTR_PAGE_FOOTERSET, aFooterSet);
        rSet.Put(aSetItem);
    }

    // Integrate footnotes
    SwPageFtnInfo& rInfo = (SwPageFtnInfo&)rPageDesc.GetFtnInfo();
    SwPageFtnInfoItem aFtnItem(FN_PARAM_FTN_INFO, rInfo);
    rSet.Put(aFtnItem);

    // Register compliant
    const SwTxtFmtColl* pCol = rPageDesc.GetRegisterFmtColl();
    SwRegisterItem aReg(pCol != 0);
    aReg.SetWhich(SID_SWREGISTER_MODE);
    rSet.Put(aReg);
    if(pCol)
        rSet.Put(SfxStringItem(SID_SWREGISTER_COLLECTION, pCol->GetName()));

}

// Set DefaultTabs

void MakeDefTabs(SwTwips nDefDist, SvxTabStopItem& rTabs)
{
    if( rTabs.Count() )
        return;
    {
        SvxTabStop aSwTabStop( nDefDist, SVX_TAB_ADJUST_DEFAULT );
        rTabs.Insert( aSwTabStop );
    }
}

// Distance between two tabs

SwTwips GetTabDist(const SvxTabStopItem& rTabs)
{
    return rTabs.Count() ? rTabs[0].GetTabPos() : 1134; // 1134 = 2 cm
}

// Inquire if in the set is a Sfx-PageDesc combination present and return it.
void SfxToSwPageDescAttr( const SwWrtShell& rShell, SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    SwFmtPageDesc aPgDesc;

    bool bChanged = false;
    // Page number
    if(SfxItemState::SET == rSet.GetItemState(SID_ATTR_PARA_PAGENUM, false, &pItem))
    {
        aPgDesc.SetNumOffset(((SfxUInt16Item*)pItem)->GetValue());
        bChanged = true;
    }
    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_PARA_MODEL, false, &pItem ))
    {
        const OUString& rDescName = ((SvxPageModelItem*)pItem)->GetValue();
        if( !rDescName.isEmpty() )   // No name -> disable PageDesc!
        {
            // Delete only, if PageDesc will be enabled!
            rSet.ClearItem( RES_BREAK );
            SwPageDesc* pDesc = ((SwWrtShell&)rShell).FindPageDescByName(
                                                    rDescName, true );
            if( pDesc )
                aPgDesc.RegisterToPageDesc( *pDesc );
        }
        rSet.ClearItem( SID_ATTR_PARA_MODEL );
        bChanged = true;
    }
    else
    {
        SfxItemSet aCoreSet(rShell.GetView().GetPool(), RES_PAGEDESC, RES_PAGEDESC );
        rShell.GetCurAttr( aCoreSet );
        if(SfxItemState::SET == aCoreSet.GetItemState( RES_PAGEDESC, true, &pItem ) )
        {
            if( ((SwFmtPageDesc*)pItem)->GetPageDesc() )
            {
                aPgDesc.RegisterToPageDesc( *((SwFmtPageDesc*)pItem)->GetPageDesc() );
            }
        }
    }

    if(bChanged)
        rSet.Put( aPgDesc );
}

// Inquire if in the set is a Sfx-PageDesc combination present and return it.
void SwToSfxPageDescAttr( SfxItemSet& rCoreSet )
{
    const SfxPoolItem* pItem = 0;
    OUString aName;
    ::boost::optional<sal_uInt16> oNumOffset;
    bool bPut = true;
    switch( rCoreSet.GetItemState( RES_PAGEDESC, true, &pItem ) )
    {
    case SfxItemState::SET:
        {
            if( ((SwFmtPageDesc*)pItem)->GetPageDesc() )
            {
                aName = ((SwFmtPageDesc*)pItem)->GetPageDesc()->GetName();
                oNumOffset = ((SwFmtPageDesc*)pItem)->GetNumOffset();
            }
            rCoreSet.ClearItem( RES_PAGEDESC );
            // Page number
        }
        break;

    case SfxItemState::DEFAULT:
        break;

    default:
        bPut = false;
    }

    // TODO for now always pass a page number to cui, it can't make a
    // difference between 0 and no page number at the moment.
    {
        SfxUInt16Item aPageNum( SID_ATTR_PARA_PAGENUM, oNumOffset ? oNumOffset.get() : 0 );
        rCoreSet.Put( aPageNum );
    }

    if(bPut)
        rCoreSet.Put( SvxPageModelItem( aName, true, SID_ATTR_PARA_MODEL ) );
}

// Determine metric

FieldUnit   GetDfltMetric(bool bWeb)
{
    return SW_MOD()->GetUsrPref(bWeb)->GetMetric();
}

// Determine metric

void    SetDfltMetric( FieldUnit eMetric, bool bWeb )
{
    SW_MOD()->ApplyUserMetric(eMetric, bWeb);
}

sal_Int32 InsertStringSorted(const OUString& rEntry, ListBox& rToFill, sal_Int32 nOffset )
{
    CollatorWrapper& rCaseColl = ::GetAppCaseCollator();
    const sal_Int32 nCount = rToFill.GetEntryCount();
    while (nOffset < nCount)
    {
        if( 0 < rCaseColl.compareString( rToFill.GetEntry(nOffset), rEntry ))
            break;
        ++nOffset;
    }
    return rToFill.InsertEntry(rEntry, nOffset);
}

void FillCharStyleListBox(ListBox& rToFill, SwDocShell* pDocSh, bool bSorted, bool bWithDefault)
{
    const sal_Int32 nOffset = rToFill.GetEntryCount() > 0 ? 1 : 0;
    SfxStyleSheetBasePool* pPool = pDocSh->GetStyleSheetPool();
    pPool->SetSearchMask(SFX_STYLE_FAMILY_CHAR, SFXSTYLEBIT_ALL);
    SwDoc* pDoc = pDocSh->GetDoc();
    const SfxStyleSheetBase* pBase = pPool->First();
    OUString sStandard;
    SwStyleNameMapper::FillUIName( RES_POOLCOLL_STANDARD, sStandard );
    while(pBase)
    {
        if(bWithDefault || pBase->GetName() !=  sStandard)
        {
            const sal_Int32 nPos = bSorted
                ? InsertStringSorted(pBase->GetName(), rToFill, nOffset )
                : rToFill.InsertEntry(pBase->GetName());
            sal_IntPtr nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( pBase->GetName(), nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            rToFill.SetEntryData( nPos, (void*) (nPoolId));
        }
        pBase = pPool->Next();
    }
    // non-pool styles
    const SwCharFmts* pFmts = pDoc->GetCharFmts();
    for(size_t i = 0; i < pFmts->size(); ++i)
    {
        const SwCharFmt* pFmt = (*pFmts)[i];
        if(pFmt->IsDefault())
            continue;
        const OUString& rName = pFmt->GetName();
        if(rToFill.GetEntryPos(rName) == LISTBOX_ENTRY_NOTFOUND)
        {
            const sal_Int32 nPos = bSorted
                ? InsertStringSorted(rName, rToFill, nOffset )
                : rToFill.InsertEntry(rName);
            sal_IntPtr nPoolId = USHRT_MAX;
            rToFill.SetEntryData( nPos, (void*) (nPoolId));
        }
    }
};

SwTwips GetTableWidth( SwFrmFmt* pFmt, SwTabCols& rCols, sal_uInt16 *pPercent,
            SwWrtShell* pSh )
{
    // To get the width is slightly more complicated.
    SwTwips nWidth = 0;
    const sal_Int16 eOri = pFmt->GetHoriOrient().GetHoriOrient();
    switch(eOri)
    {
        case text::HoriOrientation::FULL: nWidth = rCols.GetRight(); break;
        case text::HoriOrientation::LEFT_AND_WIDTH:
        case text::HoriOrientation::LEFT:
        case text::HoriOrientation::RIGHT:
        case text::HoriOrientation::CENTER:
            nWidth = pFmt->GetFrmSize().GetWidth();
        break;
        default:
        {
            if(pSh)
            {
                if ( 0 == pSh->GetFlyFrmFmt() )
                {
                    nWidth = pSh->GetAnyCurRect(RECT_PAGE_PRT).Width();
                }
                else
                {
                    nWidth = pSh->GetAnyCurRect(RECT_FLY_PRT_EMBEDDED).Width();
                }
            }
            else
            {
                OSL_FAIL("where to get the actual width from?");
            }
            const SvxLRSpaceItem& rLRSpace = pFmt->GetLRSpace();
            nWidth -= (rLRSpace.GetRight() + rLRSpace.GetLeft());
        }
    }
    if (pPercent)
        *pPercent = pFmt->GetFrmSize().GetWidthPercent();
    return nWidth;
}

OUString GetAppLangDateTimeString( const DateTime& rDT )
{
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rAppLclData = aSysLocale.GetLocaleData();
    OUString sRet = rAppLclData.getDate( rDT ) + " " + rAppLclData.getTime( rDT, false, false );
    return sRet;
}

// Add a new function which can get and set the current "SID_ATTR_APPLYCHARUNIT" value

bool HasCharUnit( bool bWeb)
{
    return SW_MOD()->GetUsrPref(bWeb)->IsApplyCharUnit();
}

void SetApplyCharUnit(bool bApplyChar, bool bWeb)
{
    SW_MOD()->ApplyUserCharUnit(bApplyChar, bWeb);
}

bool ExecuteMenuCommand( PopupMenu& rMenu, SfxViewFrame& rViewFrame, sal_uInt16 nId )
{
    bool bRet = false;
    const sal_uInt16 nItemCount = rMenu.GetItemCount();
    OUString sCommand;
    for( sal_uInt16 nItem = 0; nItem < nItemCount; ++nItem)
    {
        PopupMenu* pPopup = rMenu.GetPopupMenu( rMenu.GetItemId( nItem ) );
        if(pPopup)
        {
            sCommand = pPopup->GetItemCommand(nId);
            if(!sCommand.isEmpty())
                break;
        }
    }
    if(!sCommand.isEmpty())
    {
        uno::Reference< frame::XFrame >  xFrame = rViewFrame.GetFrame().GetFrameInterface();
        uno::Reference < frame::XDispatchProvider > xProv( xFrame, uno::UNO_QUERY );
        util::URL aURL;
        aURL.Complete = sCommand;
        uno::Reference < util::XURLTransformer > xTrans( util::URLTransformer::create(::comphelper::getProcessComponentContext() ) );
        xTrans->parseStrict( aURL );
        uno::Reference< frame::XDispatch >  xDisp = xProv->queryDispatch( aURL, OUString(), 0 );
        if( xDisp.is() )
        {
            uno::Sequence< beans::PropertyValue > aSeq;
            xDisp->dispatch( aURL, aSeq );
            bRet = true;
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
