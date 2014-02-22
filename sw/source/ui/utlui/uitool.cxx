/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

//
#define MAXHEIGHT 28350
#define MAXWIDTH  28350

using namespace ::com::sun::star;





void SetMetric(MetricFormatter& rCtrl, FieldUnit eUnit)
{
    SwTwips nMin = static_cast< SwTwips >(rCtrl.GetMin(FUNIT_TWIP));
    SwTwips nMax = static_cast< SwTwips >(rCtrl.GetMax(FUNIT_TWIP));

    rCtrl.SetUnit(eUnit);

    rCtrl.SetMin(nMin, FUNIT_TWIP);
    rCtrl.SetMax(nMax, FUNIT_TWIP);
}



void PrepareBoxInfo(SfxItemSet& rSet, const SwWrtShell& rSh)
{
    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
    const SfxPoolItem *pBoxInfo;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BORDER_INNER,
                                        true, &pBoxInfo))
        aBoxInfo = *(SvxBoxInfoItem*)pBoxInfo;

        
    rSh.GetCrsr();                  
    aBoxInfo.SetTable          (rSh.IsTableMode() && rSh.GetCrsrCnt() > 1);
        
    aBoxInfo.SetDist           (true);
        
    aBoxInfo.SetMinDist        (rSh.IsTableMode() || rSh.GetSelectionType() & (nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL));
        
    aBoxInfo.SetDefDist        (MIN_BORDER_DIST);
        
    aBoxInfo.SetValid(VALID_DISABLE, !rSh.IsTableMode());

    rSet.Put(aBoxInfo);
}

void ConvertAttrCharToGen(SfxItemSet& rSet, const sal_uInt8 nMode)
{
    
    {
        const SfxPoolItem *pTmpBrush;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_BACKGROUND, true, &pTmpBrush ) )
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
        
        const SfxPoolItem *pTmpItem;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_BOX, true, &pTmpItem ) )
        {
            SvxBoxItem aTmpBox( *((SvxBoxItem*)pTmpItem) );
            aTmpBox.SetWhich( RES_BOX );
            rSet.Put( aTmpBox );
        }
        else
            rSet.ClearItem(RES_BOX);

        
        if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_SHADOW, false, &pTmpItem ) )
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
    
    {
        const SfxPoolItem *pTmpBrush;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_BACKGROUND, false, &pTmpBrush ) )
        {
            SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
            aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
            rSet.Put( aTmpBrush );
        }
        rSet.ClearItem( RES_BACKGROUND );
    }

    if( nMode == CONV_ATTR_STD )
    {
        
        const SfxPoolItem *pTmpItem;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_BOX, false, &pTmpItem ) )
        {
            SvxBoxItem aTmpBox( *((SvxBoxItem*)pTmpItem) );
            aTmpBox.SetWhich( RES_CHRATR_BOX );
            rSet.Put( aTmpBox );
        }
        rSet.ClearItem( RES_BOX );

        
        if( SFX_ITEM_SET == rSet.GetItemState( RES_SHADOW, false, &pTmpItem ) )
        {
            SvxShadowItem aTmpShadow( *((SvxShadowItem*)pTmpItem) );
            aTmpShadow.SetWhich( RES_CHRATR_SHADOW );
            rSet.Put( aTmpShadow );
        }
        rSet.ClearItem( RES_SHADOW );
    }
}




void FillHdFt(SwFrmFmt* pFmt, const  SfxItemSet& rSet)
{
    SwAttrSet aSet(pFmt->GetAttrSet());
    aSet.Put(rSet);

    const SvxSizeItem& rSize = (const SvxSizeItem&)rSet.Get(SID_ATTR_PAGE_SIZE);
    const SfxBoolItem& rDynamic = (const SfxBoolItem&)rSet.Get(SID_ATTR_PAGE_DYNAMIC);

    
    SwFmtFrmSize aFrmSize(rDynamic.GetValue() ? ATT_MIN_SIZE : ATT_FIX_SIZE,
                            rSize.GetSize().Width(),
                            rSize.GetSize().Height());
    aSet.Put(aFrmSize);
    pFmt->SetFmtAttr(aSet);
}


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



void ItemSetToPageDesc( const SfxItemSet& rSet, SwPageDesc& rPageDesc )
{
    SwFrmFmt& rMaster = rPageDesc.GetMaster();
    int nFirstShare = -1;

    
    rMaster.SetFmtAttr(rSet);

    
    if(rSet.GetItemState(SID_ATTR_PAGE) == SFX_ITEM_SET)
    {
        const SvxPageItem& rPageItem = (const SvxPageItem&)rSet.Get(SID_ATTR_PAGE);

        sal_uInt16 nUse = (sal_uInt16)rPageItem.GetPageUsage();
        if(nUse)
            rPageDesc.SetUseOn( lcl_convertUseFromSvx((UseOnPage) nUse) );
        rPageDesc.SetLandscape(rPageItem.IsLandscape());
        SvxNumberType aNumType;
        aNumType.SetNumberingType( static_cast< sal_Int16 >(rPageItem.GetNumType()) );
        rPageDesc.SetNumType(aNumType);
    }
    
    if(rSet.GetItemState(SID_ATTR_PAGE_SIZE) == SFX_ITEM_SET)
    {
        const SvxSizeItem& rSizeItem = (const SvxSizeItem&)rSet.Get(SID_ATTR_PAGE_SIZE);
        SwFmtFrmSize aSize(ATT_FIX_SIZE);
        aSize.SetSize(rSizeItem.GetSize());
        rMaster.SetFmtAttr(aSize);
    }
    
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_PAGE_HEADERSET,
            false, &pItem ) )
    {
        const SfxItemSet& rHeaderSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rHeaderOn = (const SfxBoolItem&)rHeaderSet.Get(SID_ATTR_PAGE_ON);

        if(rHeaderOn.GetValue())
        {
            
            if(!rMaster.GetHeader().IsActive())
                rMaster.SetFmtAttr(SwFmtHeader(sal_True));

            
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
                nFirstShare = rPageDesc.IsFirstShared();
            }
        }
        else
        {
            
            if(rMaster.GetHeader().IsActive())
            {
                rMaster.SetFmtAttr(SwFmtHeader(sal_Bool(sal_False)));
                rPageDesc.ChgHeaderShare(sal_False);
            }
        }
    }

    
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_PAGE_FOOTERSET,
            false, &pItem ) )
    {
        const SfxItemSet& rFooterSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rFooterOn = (const SfxBoolItem&)rFooterSet.Get(SID_ATTR_PAGE_ON);

        if(rFooterOn.GetValue())
        {
            
            if(!rMaster.GetFooter().IsActive())
                rMaster.SetFmtAttr(SwFmtFooter(sal_True));

            
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
                nFirstShare = rPageDesc.IsFirstShared();
            }
        }
        else
        {
            
            if(rMaster.GetFooter().IsActive())
            {
                rMaster.SetFmtAttr(SwFmtFooter(sal_Bool(sal_False)));
                rPageDesc.ChgFooterShare(sal_False);
            }
        }
    }

    
    //
    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_FTN_INFO,
            false, &pItem ) )
        rPageDesc.SetFtnInfo( ((SwPageFtnInfoItem*)pItem)->GetPageFtnInfo() );

    

    

    if(SFX_ITEM_SET == rSet.GetItemState(
                            SID_SWREGISTER_MODE, false, &pItem))
    {
        sal_Bool bSet = ((const SfxBoolItem*)pItem)->GetValue();
        if(!bSet)
            rPageDesc.SetRegisterFmtColl(0);
        else if(SFX_ITEM_SET == rSet.GetItemState(
                                SID_SWREGISTER_COLLECTION, false, &pItem))
        {
            const OUString& rColl = ((const SfxStringItem*)pItem)->GetValue();
            SwDoc& rDoc = *rMaster.GetDoc();
            SwTxtFmtColl* pColl = rDoc.FindTxtFmtCollByName( rColl );
            if( !pColl )
            {
                sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( rColl, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if( USHRT_MAX != nId )
                    pColl = rDoc.GetTxtCollFromPool( nId );
                else
                    pColl = rDoc.MakeTxtFmtColl( rColl,
                                (SwTxtFmtColl*)rDoc.GetDfltTxtFmtColl() );
            }
            if( pColl )
                pColl->SetFmtAttr( SwRegisterItem ( sal_True ));
            rPageDesc.SetRegisterFmtColl( pColl );
        }
    }
}

void PageDescToItemSet( const SwPageDesc& rPageDesc, SfxItemSet& rSet)
{
    const SwFrmFmt& rMaster = rPageDesc.GetMaster();

    
    SvxPageItem aPageItem(SID_ATTR_PAGE);
    aPageItem.SetDescName(rPageDesc.GetName());
    aPageItem.SetPageUsage(lcl_convertUseToSvx(rPageDesc.GetUseOn()));
    aPageItem.SetLandscape(rPageDesc.GetLandscape());
    aPageItem.SetNumType((SvxNumType)rPageDesc.GetNumType().GetNumberingType());
    rSet.Put(aPageItem);

    
    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, rMaster.GetFrmSize().GetSize());
    rSet.Put(aSizeItem);

    
    SvxSizeItem aMaxSizeItem(SID_ATTR_PAGE_MAXSIZE, Size(MAXWIDTH, MAXHEIGHT));
    rSet.Put(aMaxSizeItem);

    
    rSet.Put(rMaster.GetAttrSet());

    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
    const SfxPoolItem *pBoxInfo;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BORDER_INNER,
                                            true, &pBoxInfo) )
        aBoxInfo = *(SvxBoxInfoItem*)pBoxInfo;

    aBoxInfo.SetTable( false );
        
    aBoxInfo.SetDist( true);
        
    aBoxInfo.SetMinDist( false );
        
    aBoxInfo.SetDefDist( MIN_BORDER_DIST );
        
    aBoxInfo.SetValid( VALID_DISABLE );
    rSet.Put( aBoxInfo );


    SfxStringItem aFollow(SID_ATTR_PAGE_EXT1, OUString());
    if(rPageDesc.GetFollow())
        aFollow.SetValue(rPageDesc.GetFollow()->GetName());
    rSet.Put(aFollow);

    
    if(rMaster.GetHeader().IsActive())
    {
        const SwFmtHeader &rHeaderFmt = rMaster.GetHeader();
        const SwFrmFmt *pHeaderFmt = rHeaderFmt.GetHeaderFmt();
        OSL_ENSURE(pHeaderFmt != 0, "no header format");

        
        SfxItemSet aHeaderSet( *rSet.GetPool(),
                    SID_ATTR_PAGE_ON,       SID_ATTR_PAGE_SHARED,
                    SID_ATTR_PAGE_SHARED_FIRST, SID_ATTR_PAGE_SHARED_FIRST,
                    SID_ATTR_PAGE_SIZE,     SID_ATTR_PAGE_SIZE,
                    SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
                    RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
                    0);

        
        SfxBoolItem aOn(SID_ATTR_PAGE_ON, true);
        aHeaderSet.Put(aOn);

        const SwFmtFrmSize &rFrmSize = pHeaderFmt->GetFrmSize();
        const SwFrmSize eSizeType = rFrmSize.GetHeightSizeType();
        SfxBoolItem aDynamic(SID_ATTR_PAGE_DYNAMIC, eSizeType != ATT_FIX_SIZE);
        aHeaderSet.Put(aDynamic);

        
        SfxBoolItem aShared(SID_ATTR_PAGE_SHARED, rPageDesc.IsHeaderShared());
        aHeaderSet.Put(aShared);
        SfxBoolItem aFirstShared(SID_ATTR_PAGE_SHARED_FIRST, rPageDesc.IsFirstShared());
        aHeaderSet.Put(aFirstShared);

        
        SvxSizeItem aSize(SID_ATTR_PAGE_SIZE, Size(rFrmSize.GetSize()));
        aHeaderSet.Put(aSize);

        
        aHeaderSet.Put(pHeaderFmt->GetAttrSet());
        aHeaderSet.Put( aBoxInfo );

        
        SvxSetItem aSetItem(SID_ATTR_PAGE_HEADERSET, aHeaderSet);
        rSet.Put(aSetItem);
    }

    
    if(rMaster.GetFooter().IsActive())
    {
        const SwFmtFooter &rFooterFmt = rMaster.GetFooter();
        const SwFrmFmt *pFooterFmt = rFooterFmt.GetFooterFmt();
        OSL_ENSURE(pFooterFmt != 0, "no footer format");

        
        SfxItemSet aFooterSet( *rSet.GetPool(),
                    SID_ATTR_PAGE_ON,       SID_ATTR_PAGE_SHARED,
                    SID_ATTR_PAGE_SHARED_FIRST, SID_ATTR_PAGE_SHARED_FIRST,
                    SID_ATTR_PAGE_SIZE,     SID_ATTR_PAGE_SIZE,
                    SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
                    RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
                    0);

        
        SfxBoolItem aOn(SID_ATTR_PAGE_ON, true);
        aFooterSet.Put(aOn);

        const SwFmtFrmSize &rFrmSize = pFooterFmt->GetFrmSize();
        const SwFrmSize eSizeType = rFrmSize.GetHeightSizeType();
        SfxBoolItem aDynamic(SID_ATTR_PAGE_DYNAMIC, eSizeType != ATT_FIX_SIZE);
        aFooterSet.Put(aDynamic);

        
        SfxBoolItem aShared(SID_ATTR_PAGE_SHARED, rPageDesc.IsFooterShared());
        aFooterSet.Put(aShared);
        SfxBoolItem aFirstShared(SID_ATTR_PAGE_SHARED_FIRST, rPageDesc.IsFirstShared());
        aFooterSet.Put(aFirstShared);

        
        SvxSizeItem aSize(SID_ATTR_PAGE_SIZE, Size(rFrmSize.GetSize()));
        aFooterSet.Put(aSize);

        
        aFooterSet.Put(pFooterFmt->GetAttrSet());
        aFooterSet.Put( aBoxInfo );

        
        SvxSetItem aSetItem(SID_ATTR_PAGE_FOOTERSET, aFooterSet);
        rSet.Put(aSetItem);
    }

    
    SwPageFtnInfo& rInfo = (SwPageFtnInfo&)rPageDesc.GetFtnInfo();
    SwPageFtnInfoItem aFtnItem(FN_PARAM_FTN_INFO, rInfo);
    rSet.Put(aFtnItem);

    
    const SwTxtFmtColl* pCol = rPageDesc.GetRegisterFmtColl();
    SwRegisterItem aReg(pCol != 0);
    aReg.SetWhich(SID_SWREGISTER_MODE);
    rSet.Put(aReg);
    if(pCol)
        rSet.Put(SfxStringItem(SID_SWREGISTER_COLLECTION, pCol->GetName()));

}



void MakeDefTabs(SwTwips nDefDist, SvxTabStopItem& rTabs)
{
    if( rTabs.Count() )
        return;
    {
        SvxTabStop aSwTabStop( nDefDist, SVX_TAB_ADJUST_DEFAULT );
        rTabs.Insert( aSwTabStop );
    }
}



sal_uInt16 GetTabDist(const SvxTabStopItem& rTabs)
{
    sal_uInt16 nDefDist;
    if( rTabs.Count() )
        nDefDist = (sal_uInt16)( rTabs[0].GetTabPos() );
    else
        nDefDist = 1134;     
    return nDefDist;
}



void SfxToSwPageDescAttr( const SwWrtShell& rShell, SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    SwFmtPageDesc aPgDesc;

    bool bChanged = false;
    
    if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_PARA_PAGENUM, false, &pItem))
    {
        aPgDesc.SetNumOffset(((SfxUInt16Item*)pItem)->GetValue());
        bChanged = true;
    }
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_PARA_MODEL, false, &pItem ))
    {
        const OUString& rDescName = ((SvxPageModelItem*)pItem)->GetValue();
        if( !rDescName.isEmpty() )   
        {
            
            rSet.ClearItem( RES_BREAK );
            SwPageDesc* pDesc = ((SwWrtShell&)rShell).FindPageDescByName(
                                                    rDescName, sal_True );
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
        if(SFX_ITEM_SET == aCoreSet.GetItemState( RES_PAGEDESC, true, &pItem ) )
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


void SwToSfxPageDescAttr( SfxItemSet& rCoreSet )
{
    const SfxPoolItem* pItem = 0;
    OUString aName;
    ::boost::optional<sal_uInt16> oNumOffset;
    bool bPut = true;
    switch( rCoreSet.GetItemState( RES_PAGEDESC, true, &pItem ) )
    {
    case SFX_ITEM_SET:
        {
            if( ((SwFmtPageDesc*)pItem)->GetPageDesc() )
            {
                aName = ((SwFmtPageDesc*)pItem)->GetPageDesc()->GetName();
                oNumOffset = ((SwFmtPageDesc*)pItem)->GetNumOffset();
            }
            rCoreSet.ClearItem( RES_PAGEDESC );
            
        }
        break;

    case SFX_ITEM_AVAILABLE:
        break;

    default:
        bPut = false;
    }

    
    
    {
        SfxUInt16Item aPageNum( SID_ATTR_PARA_PAGENUM, oNumOffset ? oNumOffset.get() : 0 );
        rCoreSet.Put( aPageNum );
    }

    if(bPut)
        rCoreSet.Put( SvxPageModelItem( aName, sal_True, SID_ATTR_PARA_MODEL ) );
}



FieldUnit   GetDfltMetric(sal_Bool bWeb)
{
    return SW_MOD()->GetUsrPref(bWeb)->GetMetric();
}



void    SetDfltMetric( FieldUnit eMetric, sal_Bool bWeb )
{
    SW_MOD()->ApplyUserMetric(eMetric, bWeb);
}

sal_uInt16 InsertStringSorted(const OUString& rEntry, ListBox& rToFill, sal_uInt16 nOffset )
{
    sal_uInt16 i = nOffset;
    CollatorWrapper& rCaseColl = ::GetAppCaseCollator();

    for( ; i < rToFill.GetEntryCount(); i++ )
    {
        if( 0 < rCaseColl.compareString( rToFill.GetEntry(i), rEntry ))
            break;
    }
    return rToFill.InsertEntry(rEntry, i);
}
void FillCharStyleListBox(ListBox& rToFill, SwDocShell* pDocSh, bool bSorted, bool bWithDefault)
{
    sal_Bool bHasOffset = rToFill.GetEntryCount() > 0;
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
            sal_uInt16 nPos;
            if(bSorted)
                nPos = InsertStringSorted(pBase->GetName(), rToFill, bHasOffset );
            else
                nPos = rToFill.InsertEntry(pBase->GetName());
            sal_IntPtr nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( pBase->GetName(), nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            rToFill.SetEntryData( nPos, (void*) (nPoolId));
        }
        pBase = pPool->Next();
    }
    
    const SwCharFmts* pFmts = pDoc->GetCharFmts();
    for(sal_uInt16 i = 0; i < pFmts->size(); i++)
    {
        const SwCharFmt* pFmt = (*pFmts)[i];
        if(pFmt->IsDefault())
            continue;
        const OUString& rName = pFmt->GetName();
        if(rToFill.GetEntryPos(rName) == LISTBOX_ENTRY_NOTFOUND)
        {
            sal_uInt16 nPos;
            if(bSorted)
                nPos = InsertStringSorted(rName, rToFill, bHasOffset );
            else
                nPos = rToFill.InsertEntry(rName);
            sal_IntPtr nPoolId = USHRT_MAX;
            rToFill.SetEntryData( nPos, (void*) (nPoolId));
        }
    }
};

SwTwips GetTableWidth( SwFrmFmt* pFmt, SwTabCols& rCols, sal_uInt16 *pPercent,
            SwWrtShell* pSh )
{
    
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



sal_Bool HasCharUnit( sal_Bool bWeb)
{
    return SW_MOD()->GetUsrPref(bWeb)->IsApplyCharUnit();
}

void SetApplyCharUnit(sal_Bool bApplyChar, sal_Bool bWeb)
{
    SW_MOD()->ApplyUserCharUnit(bApplyChar, bWeb);
}

bool ExecuteMenuCommand( PopupMenu& rMenu, SfxViewFrame& rViewFrame, sal_uInt16 nId )
{
    bool bRet = false;
    sal_uInt16 nItemCount = rMenu.GetItemCount();
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
