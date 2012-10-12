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

#include <tools/shl.hxx>
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/sfxsids.hrc>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/brshitem.hxx>
#include <grfpage.hxx>
#include <svx/grfcrop.hxx>
#include <grfpage.hrc>
#include <cuires.hrc>
#include <svx/dialogs.hrc> // for RID_SVXPAGE_GRFCROP

#define CM_1_TO_TWIP        567
#define TWIP_TO_INCH        1440


static inline long lcl_GetValue( MetricField& rMetric, FieldUnit eUnit )
{
    return static_cast<long>(rMetric.Denormalize( rMetric.GetValue( eUnit )));
}

/*--------------------------------------------------------------------
    description: crop graphic
 --------------------------------------------------------------------*/

SvxGrfCropPage::SvxGrfCropPage ( Window *pParent, const SfxItemSet &rSet )
    : SfxTabPage( pParent,  CUI_RES( RID_SVXPAGE_GRFCROP ), rSet ),
    aCropFL(        this, CUI_RES( FL_CROP    )),
    aZoomConstRB(   this, CUI_RES( RB_ZOOMCONST)),
    aSizeConstRB(   this, CUI_RES( RB_SIZECONST)),
    aLeftFT(        this, CUI_RES( FT_LEFT    )),
    aLeftMF(        this, CUI_RES( MF_LEFT    )),
    aRightFT(       this, CUI_RES( FT_RIGHT   )),
    aRightMF(       this, CUI_RES( MF_RIGHT   )),
    aTopFT(         this, CUI_RES( FT_TOP     )),
    aTopMF(         this, CUI_RES( MF_TOP     )),
    aBottomFT(      this, CUI_RES( FT_BOTTOM  )),
    aBottomMF(      this, CUI_RES( MF_BOTTOM  )),
    aZoomFL(        this, CUI_RES( FL_ZOOM    )),
    aWidthZoomFT(   this, CUI_RES( FT_WIDTHZOOM )),
    aWidthZoomMF(   this, CUI_RES( MF_WIDTHZOOM )),
    aHeightZoomFT(  this, CUI_RES( FT_HEIGHTZOOM)),
    aHeightZoomMF(  this, CUI_RES( MF_HEIGHTZOOM)),
    aSizeFL(        this, CUI_RES( FL_SIZE    )),
    aWidthFT(       this, CUI_RES( FT_WIDTH   )),
    aWidthMF(       this, CUI_RES( MF_WIDTH   )),
    aHeightFT(      this, CUI_RES( FT_HEIGHT  )),
    aHeightMF(      this, CUI_RES( MF_HEIGHT  )),
    aOrigSizeFT(    this, CUI_RES(FT_ORIG_SIZE)),
    aOrigSizePB(    this, CUI_RES( PB_ORGSIZE )),
    aExampleWN(     this, CUI_RES( WN_BSP     )),
    pLastCropField(0),
    bInitialized(sal_False),
    bSetOrigSize(sal_False)
{
    FreeResource();

    SetExchangeSupport();

    // set the correct metric
    const FieldUnit eMetric = GetModuleFieldUnit( rSet );

    SetFieldUnit( aWidthMF, eMetric );
    SetFieldUnit( aHeightMF, eMetric );
    SetFieldUnit( aLeftMF, eMetric );
    SetFieldUnit( aRightMF, eMetric );
    SetFieldUnit( aTopMF , eMetric );
    SetFieldUnit( aBottomMF, eMetric );

    Link aLk = LINK(this, SvxGrfCropPage, SizeHdl);
    aWidthMF.SetModifyHdl( aLk );
    aHeightMF.SetModifyHdl( aLk );

    aLk = LINK(this, SvxGrfCropPage, ZoomHdl);
    aWidthZoomMF.SetModifyHdl( aLk );
    aHeightZoomMF.SetModifyHdl( aLk );

    aLk = LINK(this, SvxGrfCropPage, CropHdl);
    aLeftMF.SetDownHdl( aLk );
    aRightMF.SetDownHdl( aLk );
    aTopMF.SetDownHdl( aLk );
    aBottomMF.SetDownHdl( aLk );
    aLeftMF.SetUpHdl( aLk );
    aRightMF.SetUpHdl( aLk );
    aTopMF.SetUpHdl( aLk );
    aBottomMF.SetUpHdl( aLk );

    aLk = LINK(this, SvxGrfCropPage, CropModifyHdl);
    aLeftMF.SetModifyHdl( aLk );
    aRightMF.SetModifyHdl( aLk );
    aTopMF.SetModifyHdl( aLk );
    aBottomMF.SetModifyHdl( aLk );

    aLk = LINK(this, SvxGrfCropPage, CropLoseFocusHdl);
    aLeftMF.SetLoseFocusHdl( aLk );
    aRightMF.SetLoseFocusHdl( aLk );
    aTopMF.SetLoseFocusHdl( aLk );
    aBottomMF.SetLoseFocusHdl( aLk );

    aLk = LINK(this, SvxGrfCropPage, OrigSizeHdl);
    aOrigSizePB.SetClickHdl( aLk );

    aTimer.SetTimeoutHdl(LINK(this, SvxGrfCropPage, Timeout));
    aTimer.SetTimeout( 1500 );

    aOrigSizePB.SetAccessibleRelationLabeledBy( &aOrigSizeFT );
}

SvxGrfCropPage::~SvxGrfCropPage()
{
    aTimer.Stop();
}

SfxTabPage* SvxGrfCropPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SvxGrfCropPage( pParent, rSet );
}

void SvxGrfCropPage::Reset( const SfxItemSet &rSet )
{
    const SfxPoolItem* pItem;
    const SfxItemPool& rPool = *rSet.GetPool();

    if(SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich(
                                    SID_ATTR_GRAF_KEEP_ZOOM ), sal_True, &pItem ))
    {
        if( ((const SfxBoolItem*)pItem)->GetValue() )
            aZoomConstRB.Check();
        else
            aSizeConstRB.Check();
        aZoomConstRB.SaveValue();
    }

    sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_CROP );
    if( SFX_ITEM_SET == rSet.GetItemState( nW, sal_True, &pItem))
    {
        FieldUnit eUnit = MapToFieldUnit( rSet.GetPool()->GetMetric( nW ));

        SvxGrfCrop* pCrop =  (SvxGrfCrop*)pItem;

        aExampleWN.SetLeft(     pCrop->GetLeft());
        aExampleWN.SetRight(    pCrop->GetRight());
        aExampleWN.SetTop(      pCrop->GetTop());
        aExampleWN.SetBottom(   pCrop->GetBottom());

        aLeftMF.SetValue( aLeftMF.Normalize( pCrop->GetLeft()), eUnit );
        aRightMF.SetValue( aRightMF.Normalize( pCrop->GetRight()), eUnit );
        aTopMF.SetValue( aTopMF.Normalize( pCrop->GetTop()), eUnit );
        aBottomMF.SetValue( aBottomMF.Normalize( pCrop->GetBottom()), eUnit );
    }
    else
    {
        aLeftMF.SetValue( 0 );
        aRightMF.SetValue( 0 );
        aTopMF.SetValue( 0 );
        aBottomMF.SetValue( 0 );
    }

    nW = rPool.GetWhich( SID_ATTR_PAGE_SIZE );
    if ( SFX_ITEM_SET == rSet.GetItemState( nW, sal_False, &pItem ) )
    {
        // orientation and size from the PageItem
        FieldUnit eUnit = MapToFieldUnit( rSet.GetPool()->GetMetric( nW ));

        aPageSize = ((const SvxSizeItem*)pItem)->GetSize();

        sal_Int64 nTmp = aHeightMF.Normalize(aPageSize.Height());
        aHeightMF.SetMax( nTmp, eUnit );
        nTmp = aWidthMF.Normalize(aPageSize.Width());
        aWidthMF.SetMax( nTmp, eUnit );
        nTmp = aWidthMF.Normalize( 23 );
        aHeightMF.SetMin( nTmp, eUnit );
        aWidthMF.SetMin( nTmp, eUnit );
    }
    else
    {
        aPageSize = OutputDevice::LogicToLogic(
                        Size( CM_1_TO_TWIP,  CM_1_TO_TWIP ),
                        MapMode( MAP_TWIP ),
                        MapMode( (MapUnit)rSet.GetPool()->GetMetric( nW ) ) );
    }

    sal_Bool bFound = sal_False;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, sal_False, &pItem ) )
    {
        const Graphic* pGrf = ((SvxBrushItem*)pItem)->GetGraphic();
        if( pGrf ) {
            aOrigSize = GetGrfOrigSize( *pGrf );
            if (pGrf->GetType() == GRAPHIC_BITMAP && aOrigSize.Width() && aOrigSize.Height()) {
                Bitmap aBitmap = pGrf->GetBitmap();
                aOrigPixelSize = aBitmap.GetSizePixel();
            }
        }

        if( aOrigSize.Width() && aOrigSize.Height() )
        {
            CalcMinMaxBorder();
            aExampleWN.SetGraphic( *pGrf );
            aExampleWN.SetFrameSize( aOrigSize );

            bFound = sal_True;
            if( ((SvxBrushItem*)pItem)->GetGraphicLink() )
                aGraphicName = *((SvxBrushItem*)pItem)->GetGraphicLink();
        }
    }

    GraphicHasChanged( bFound );
    bReset = sal_True;
    ActivatePage( rSet );
    bReset = sal_False;
}

sal_Bool SvxGrfCropPage::FillItemSet(SfxItemSet &rSet)
{
    const SfxItemPool& rPool = *rSet.GetPool();
    sal_Bool bModified = sal_False;
    if( aWidthMF.GetSavedValue() != aWidthMF.GetText() ||
        aHeightMF.GetSavedValue() != aHeightMF.GetText() )
    {
        sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_FRMSIZE );
        FieldUnit eUnit = MapToFieldUnit( rSet.GetPool()->GetMetric( nW ));

        SvxSizeItem aSz( nW );

        // size could already have been set from another page
        // #44204#
        const SfxItemSet* pExSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;
        const SfxPoolItem* pItem = 0;
        if( pExSet && SFX_ITEM_SET ==
                pExSet->GetItemState( nW, sal_False, &pItem ) )
            aSz = *(const SvxSizeItem*)pItem;
        else
            aSz = (const SvxSizeItem&)GetItemSet().Get( nW );

        Size aTmpSz( aSz.GetSize() );
        if( aWidthMF.GetText() != aWidthMF.GetSavedValue() )
            aTmpSz.Width() = lcl_GetValue( aWidthMF, eUnit );
        if( aHeightMF.GetText() != aHeightMF.GetSavedValue() )
            aTmpSz.Height() = lcl_GetValue( aHeightMF, eUnit );
        aSz.SetSize( aTmpSz );
        aWidthMF.SaveValue();
        aHeightMF.SaveValue();

        bModified |= 0 != rSet.Put( aSz );

        if( bSetOrigSize )
        {
            bModified |= 0 != rSet.Put( SvxSizeItem( rPool.GetWhich(
                        SID_ATTR_GRAF_FRMSIZE_PERCENT ), Size( 0, 0 )) );
        }
    }
    if( aLeftMF.IsModified() || aRightMF.IsModified() ||
        aTopMF.IsModified()  || aBottomMF.IsModified() )
    {
        sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_CROP );
        FieldUnit eUnit = MapToFieldUnit( rSet.GetPool()->GetMetric( nW ));
        SvxGrfCrop* pNew = (SvxGrfCrop*)rSet.Get( nW ).Clone();

        pNew->SetLeft( lcl_GetValue( aLeftMF, eUnit ) );
        pNew->SetRight( lcl_GetValue( aRightMF, eUnit ) );
        pNew->SetTop( lcl_GetValue( aTopMF, eUnit ) );
        pNew->SetBottom( lcl_GetValue( aBottomMF, eUnit ) );
        bModified |= 0 != rSet.Put( *pNew );
        delete pNew;
    }

    if( aZoomConstRB.GetSavedValue() != aZoomConstRB.IsChecked() )
    {
        bModified |= 0 != rSet.Put( SfxBoolItem( rPool.GetWhich(
                    SID_ATTR_GRAF_KEEP_ZOOM), aZoomConstRB.IsChecked() ) );
    }

    bInitialized = sal_False;

    return bModified;
}

void SvxGrfCropPage::ActivatePage(const SfxItemSet& rSet)
{
#ifdef DBG_UTIL
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
#endif

    bSetOrigSize = sal_False;

    // Size
    Size aSize;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_FRMSIZE, sal_False, &pItem ) )
        aSize = ((const SvxSizeItem*)pItem)->GetSize();

    nOldWidth = aSize.Width();
    nOldHeight = aSize.Height();

    sal_Int64 nWidth = aWidthMF.Normalize(nOldWidth);
    sal_Int64 nHeight = aHeightMF.Normalize(nOldHeight);

    if (nWidth != aWidthMF.GetValue(FUNIT_TWIP))
    {
        if(!bReset)
        {
            // value was changed by wrap-tabpage and has to
            // be set with modify-flag
            aWidthMF.SetUserValue(nWidth, FUNIT_TWIP);
        }
        else
            aWidthMF.SetValue(nWidth, FUNIT_TWIP);
    }
    aWidthMF.SaveValue();

    if (nHeight != aHeightMF.GetValue(FUNIT_TWIP))
    {
        if (!bReset)
        {
            // value was changed by wrap-tabpage and has to
            // be set with modify-flag
            aHeightMF.SetUserValue(nHeight, FUNIT_TWIP);
        }
        else
            aHeightMF.SetValue(nHeight, FUNIT_TWIP);
    }
    aHeightMF.SaveValue();
    bInitialized = sal_True;

    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, sal_False, &pItem ) )
    {
        const SvxBrushItem& rBrush = *(SvxBrushItem*)pItem;
        if( rBrush.GetGraphicLink() &&
            aGraphicName != *rBrush.GetGraphicLink() )
            aGraphicName = *rBrush.GetGraphicLink();

        const Graphic* pGrf = rBrush.GetGraphic();
        if( pGrf )
        {
            aExampleWN.SetGraphic( *pGrf );
            aOrigSize = GetGrfOrigSize( *pGrf );
            if (pGrf->GetType() == GRAPHIC_BITMAP && aOrigSize.Width() > 1 && aOrigSize.Height() > 1) {
                Bitmap aBitmap = pGrf->GetBitmap();
                aOrigPixelSize = aBitmap.GetSizePixel();
            }
            aExampleWN.SetFrameSize(aOrigSize);
            GraphicHasChanged( aOrigSize.Width() && aOrigSize.Height() );
            CalcMinMaxBorder();
        }
        else
            GraphicHasChanged( sal_False );
    }

    CalcZoom();
}

int SvxGrfCropPage::DeactivatePage(SfxItemSet *_pSet)
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return sal_True;
}

/*--------------------------------------------------------------------
    description: scale changed, adjust size
 --------------------------------------------------------------------*/

IMPL_LINK( SvxGrfCropPage, ZoomHdl, MetricField *, pField )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    if( pField == &aWidthZoomMF )
    {
        long nLRBorders = lcl_GetValue(aLeftMF, eUnit)
                         +lcl_GetValue(aRightMF, eUnit);
        aWidthMF.SetValue( aWidthMF.Normalize(
            ((aOrigSize.Width() - nLRBorders) * pField->GetValue())/100L),
            eUnit);
    }
    else
    {
        long nULBorders = lcl_GetValue(aTopMF, eUnit)
                         +lcl_GetValue(aBottomMF, eUnit);
        aHeightMF.SetValue( aHeightMF.Normalize(
            ((aOrigSize.Height() - nULBorders ) * pField->GetValue())/100L) ,
            eUnit );
    }

    return 0;
}

/*--------------------------------------------------------------------
    description: change size, adjust scale
 --------------------------------------------------------------------*/

IMPL_LINK( SvxGrfCropPage, SizeHdl, MetricField *, pField )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    Size aSize( lcl_GetValue(aWidthMF, eUnit),
                lcl_GetValue(aHeightMF, eUnit) );

    if(pField == &aWidthMF)
    {
        long nWidth = aOrigSize.Width() -
                ( lcl_GetValue(aLeftMF, eUnit) +
                  lcl_GetValue(aRightMF, eUnit) );
        if(!nWidth)
            nWidth++;
        sal_uInt16 nZoom = (sal_uInt16)( aSize.Width() * 100L / nWidth);
        aWidthZoomMF.SetValue(nZoom);
    }
    else
    {
        long nHeight = aOrigSize.Height() -
                ( lcl_GetValue(aTopMF, eUnit) +
                  lcl_GetValue(aBottomMF, eUnit));
        if(!nHeight)
            nHeight++;
        sal_uInt16 nZoom = (sal_uInt16)( aSize.Height() * 100L/ nHeight);
        aHeightZoomMF.SetValue(nZoom);
    }

    return 0;
}

/*--------------------------------------------------------------------
    description: evaluate border
 --------------------------------------------------------------------*/

IMPL_LINK( SvxGrfCropPage, CropHdl, const MetricField *, pField )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    sal_Bool bZoom = aZoomConstRB.IsChecked();
    if( pField == &aLeftMF || pField == &aRightMF )
    {
        long nLeft = lcl_GetValue( aLeftMF, eUnit );
        long nRight = lcl_GetValue( aRightMF, eUnit );
        long nWidthZoom = static_cast<long>(aWidthZoomMF.GetValue());
        if(bZoom && ( ( ( aOrigSize.Width() - (nLeft + nRight )) * nWidthZoom )
                            / 100 >= aPageSize.Width() ) )
        {
            if(pField == &aLeftMF)
            {
                nLeft = aOrigSize.Width() -
                            ( aPageSize.Width() * 100 / nWidthZoom + nRight );
                aLeftMF.SetValue( aLeftMF.Normalize( nLeft ), eUnit );
            }
            else
            {
                nRight = aOrigSize.Width() -
                            ( aPageSize.Width() * 100 / nWidthZoom + nLeft );
                aRightMF.SetValue( aRightMF.Normalize( nRight ), eUnit );
            }
        }
        aExampleWN.SetLeft(nLeft);
        aExampleWN.SetRight(nRight);
        if(bZoom)
        {
            // scale stays, recompute width
            ZoomHdl(&aWidthZoomMF);
        }
    }
    else
    {
        long nTop = lcl_GetValue( aTopMF, eUnit );
        long nBottom = lcl_GetValue( aBottomMF, eUnit );
        long nHeightZoom = static_cast<long>(aHeightZoomMF.GetValue());
        if(bZoom && ( ( ( aOrigSize.Height() - (nTop + nBottom )) * nHeightZoom)
                                            / 100 >= aPageSize.Height()))
        {
            if(pField == &aTopMF)
            {
                nTop = aOrigSize.Height() -
                            ( aPageSize.Height() * 100 / nHeightZoom + nBottom);
                aTopMF.SetValue( aWidthMF.Normalize( nTop ), eUnit );
            }
            else
            {
                nBottom = aOrigSize.Height() -
                            ( aPageSize.Height() * 100 / nHeightZoom + nTop);
                aBottomMF.SetValue( aWidthMF.Normalize( nBottom ), eUnit );
            }
        }
        aExampleWN.SetTop( nTop );
        aExampleWN.SetBottom( nBottom );
        if(bZoom)
        {
            // scale stays, recompute height
            ZoomHdl(&aHeightZoomMF);
        }
    }
    aExampleWN.Invalidate();
    // size and border changed -> recompute scale
    if(!bZoom)
        CalcZoom();
    CalcMinMaxBorder();
    return 0;
}
/*--------------------------------------------------------------------
    description: set original size
 --------------------------------------------------------------------*/

IMPL_LINK_NOARG(SvxGrfCropPage, OrigSizeHdl)
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    long nWidth = aOrigSize.Width() -
        lcl_GetValue( aLeftMF, eUnit ) -
        lcl_GetValue( aRightMF, eUnit );
    aWidthMF.SetValue( aWidthMF.Normalize( nWidth ), eUnit );
    long nHeight = aOrigSize.Height() -
        lcl_GetValue( aTopMF, eUnit ) -
        lcl_GetValue( aBottomMF, eUnit );
    aHeightMF.SetValue( aHeightMF.Normalize( nHeight ), eUnit );
    aWidthZoomMF.SetValue(100);
    aHeightZoomMF.SetValue(100);
    bSetOrigSize = sal_True;
    return 0;
}
/*--------------------------------------------------------------------
    description: compute scale
 --------------------------------------------------------------------*/

void SvxGrfCropPage::CalcZoom()
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    long nWidth = lcl_GetValue( aWidthMF, eUnit );
    long nHeight = lcl_GetValue( aHeightMF, eUnit );
    long nLRBorders = lcl_GetValue( aLeftMF, eUnit ) +
                      lcl_GetValue( aRightMF, eUnit );
    long nULBorders = lcl_GetValue( aTopMF, eUnit ) +
                      lcl_GetValue( aBottomMF, eUnit );
    sal_uInt16 nZoom = 0;
    long nDen;
    if( (nDen = aOrigSize.Width() - nLRBorders) > 0)
        nZoom = (sal_uInt16)((( nWidth  * 1000L / nDen )+5)/10);
    aWidthZoomMF.SetValue(nZoom);
    if( (nDen = aOrigSize.Height() - nULBorders) > 0)
        nZoom = (sal_uInt16)((( nHeight * 1000L / nDen )+5)/10);
    else
        nZoom = 0;
    aHeightZoomMF.SetValue(nZoom);
}

/*--------------------------------------------------------------------
    description: set minimum/maximum values for the margins
 --------------------------------------------------------------------*/

void SvxGrfCropPage::CalcMinMaxBorder()
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );
    long nR = lcl_GetValue(aRightMF, eUnit );
    long nMinWidth = (aOrigSize.Width() * 10) /11;
    long nMin = nMinWidth - (nR >= 0 ? nR : 0);
    aLeftMF.SetMax( aLeftMF.Normalize(nMin), eUnit );

    long nL = lcl_GetValue(aLeftMF, eUnit );
    nMin = nMinWidth - (nL >= 0 ? nL : 0);
    aRightMF.SetMax( aRightMF.Normalize(nMin), eUnit );

    long nUp  = lcl_GetValue( aTopMF, eUnit );
    long nMinHeight = (aOrigSize.Height() * 10) /11;
    nMin = nMinHeight - (nUp >= 0 ? nUp : 0);
    aBottomMF.SetMax( aBottomMF.Normalize(nMin), eUnit );

    long nLow = lcl_GetValue(aBottomMF, eUnit );
    nMin = nMinHeight - (nLow >= 0 ? nLow : 0);
    aTopMF.SetMax( aTopMF.Normalize(nMin), eUnit );
}
/*--------------------------------------------------------------------
    description:   set spinsize to 1/20 of the original size,
                   fill FixedText with the original size
 --------------------------------------------------------------------*/

void SvxGrfCropPage::GraphicHasChanged( sal_Bool bFound )
{
    if( bFound )
    {
        SfxItemPool* pPool = GetItemSet().GetPool();
        DBG_ASSERT( pPool, "Wo ist der Pool" );
        FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ));

        sal_Int64 nSpin = aLeftMF.Normalize(aOrigSize.Width()) / 20;
        nSpin = MetricField::ConvertValue( nSpin, aOrigSize.Width(), 0,
                                               eUnit, aLeftMF.GetUnit());

        // if the margin is too big, it is set to 1/3 on both pages
        long nR = lcl_GetValue( aRightMF, eUnit );
        long nL = lcl_GetValue( aLeftMF, eUnit );
        if((nL + nR) < - aOrigSize.Width())
        {
            long nVal = aOrigSize.Width() / -3;
            aRightMF.SetValue( aRightMF.Normalize( nVal ), eUnit );
            aLeftMF.SetValue( aLeftMF.Normalize( nVal ), eUnit );
            aExampleWN.SetLeft(nVal);
            aExampleWN.SetRight(nVal);
        }
        long nUp  = lcl_GetValue(aTopMF, eUnit );
        long nLow = lcl_GetValue(aBottomMF, eUnit );
        if((nUp + nLow) < - aOrigSize.Height())
        {
            long nVal = aOrigSize.Height() / -3;
            aTopMF.SetValue( aTopMF.Normalize( nVal ), eUnit );
            aBottomMF.SetValue( aBottomMF.Normalize( nVal ), eUnit );
            aExampleWN.SetTop(nVal);
            aExampleWN.SetBottom(nVal);
        }

        aLeftMF.SetSpinSize(nSpin);
        aRightMF.SetSpinSize(nSpin);
        nSpin = aTopMF.Normalize(aOrigSize.Height()) / 20;
        nSpin = MetricField::ConvertValue( nSpin, aOrigSize.Width(), 0,
                                               eUnit, aLeftMF.GetUnit() );
        aTopMF.SetSpinSize(nSpin);
        aBottomMF.SetSpinSize(nSpin);

        // display original size
        const FieldUnit eMetric = GetModuleFieldUnit( GetItemSet() );

        MetricField aFld(this, WB_HIDE);
        SetFieldUnit( aFld, eMetric );
        aFld.SetDecimalDigits( aWidthMF.GetDecimalDigits() );
        aFld.SetMax( LONG_MAX - 1 );

        aFld.SetValue( aFld.Normalize( aOrigSize.Width() ), eUnit );
        String sTemp = aFld.GetText();
        aFld.SetValue( aFld.Normalize( aOrigSize.Height() ), eUnit );
        // multiplication sign (U+00D7)
        sTemp += UniString("\xc3\x97", RTL_TEXTENCODING_UTF8);
        sTemp += aFld.GetText();

        if ( aOrigPixelSize.Width() && aOrigPixelSize.Height() ) {
             int ax = int(floor((float)aOrigPixelSize.Width() /
                        ((float)aOrigSize.Width()/TWIP_TO_INCH)+0.5));
             int ay = int(floor((float)aOrigPixelSize.Height() /
                        ((float)aOrigSize.Height()/TWIP_TO_INCH)+0.5));
             sTemp += rtl::OUString(" ");
             sTemp += CUI_RESSTR( STR_PPI );
             String sPPI = UniString::CreateFromInt32(ax);
             if (abs(ax - ay) > 1) {
                sPPI += UniString("\xc3\x97", RTL_TEXTENCODING_UTF8);
                sPPI += UniString::CreateFromInt32(ay);
             }
             sTemp.SearchAndReplaceAscii("%1", sPPI);
        }
        aOrigSizeFT.SetText( sTemp );
    }
    aLeftFT         .Enable(bFound);
    aLeftMF         .Enable(bFound);
    aRightFT        .Enable(bFound);
    aRightMF        .Enable(bFound);
    aTopFT          .Enable(bFound);
    aTopMF          .Enable(bFound);
    aBottomFT       .Enable(bFound);
    aBottomMF       .Enable(bFound);
    aSizeConstRB    .Enable(bFound);
    aZoomConstRB    .Enable(bFound);
    aWidthFT        .Enable(bFound);
    aWidthMF        .Enable(bFound);
    aHeightFT       .Enable(bFound);
    aHeightMF       .Enable(bFound);
    aWidthZoomFT    .Enable(bFound);
    aWidthZoomMF    .Enable(bFound);
    aHeightZoomFT   .Enable(bFound);
    aHeightZoomMF   .Enable(bFound);
    aExampleWN      .Enable(bFound);
    aOrigSizePB     .Enable(bFound);
    aOrigSizeFT     .Enable(bFound);
}

IMPL_LINK_NOARG(SvxGrfCropPage, Timeout)
{
    DBG_ASSERT(pLastCropField,"Timeout ohne Feld?");
    CropHdl(pLastCropField);
    pLastCropField = 0;
    return 0;
}


IMPL_LINK( SvxGrfCropPage, CropLoseFocusHdl, MetricField*, pField )
{
    aTimer.Stop();
    CropHdl(pField);
    pLastCropField = 0;
    return 0;
}


IMPL_LINK( SvxGrfCropPage, CropModifyHdl, MetricField *, pField )
{
    aTimer.Start();
    pLastCropField = pField;
    return 0;
}

Size SvxGrfCropPage::GetGrfOrigSize( const Graphic& rGrf ) const
{
    const MapMode aMapTwip( MAP_TWIP );
    Size aSize( rGrf.GetPrefSize() );
    if( MAP_PIXEL == rGrf.GetPrefMapMode().GetMapUnit() )
        aSize = PixelToLogic( aSize, aMapTwip );
    else
        aSize = OutputDevice::LogicToLogic( aSize,
                                        rGrf.GetPrefMapMode(), aMapTwip );
    return aSize;
}

/*****************************************************************/

SvxGrfCropPage::SvxCropExample::SvxCropExample( Window* pPar,
                                                const ResId& rResId )
    : Window( pPar, rResId ),
    aFrameSize( OutputDevice::LogicToLogic(
                            Size( CM_1_TO_TWIP / 2, CM_1_TO_TWIP / 2 ),
                            MapMode( MAP_TWIP ), GetMapMode() )),
    aTopLeft(0,0), aBottomRight(0,0)
{
    SetBorderStyle( WINDOW_BORDER_MONO );
}

void SvxGrfCropPage::SvxCropExample::Paint( const Rectangle& )
{
    Size aWinSize( PixelToLogic(GetOutputSizePixel() ));
    SetLineColor();
    SetFillColor( GetSettings().GetStyleSettings().GetWindowColor() );
    SetRasterOp( ROP_OVERPAINT );
    DrawRect( Rectangle( Point(), aWinSize ) );

    SetLineColor( Color( COL_WHITE ) );
    Rectangle aRect(Point((aWinSize.Width() - aFrameSize.Width())/2,
                          (aWinSize.Height() - aFrameSize.Height())/2),
                          aFrameSize );
    aGrf.Draw( this,  aRect.TopLeft(), aRect.GetSize() );

    Size aSz( 2, 0 );
    aSz = PixelToLogic( aSz );
    SetFillColor( Color( COL_TRANSPARENT ) );
    SetRasterOp( ROP_INVERT );
    aRect.Left()    += aTopLeft.Y();
    aRect.Top()     += aTopLeft.X();
    aRect.Right()   -= aBottomRight.Y();
    aRect.Bottom()  -= aBottomRight.X();
    DrawRect( aRect );
}

void SvxGrfCropPage::SvxCropExample::SetFrameSize( const Size& rSz )
{
    aFrameSize = rSz;
    if(!aFrameSize.Width())
        aFrameSize.Width() = 1;
    if(!aFrameSize.Height())
        aFrameSize.Height() = 1;
    Size aWinSize( GetOutputSizePixel() );
    Fraction aXScale( aWinSize.Width() * 4, aFrameSize.Width() * 5 );
    Fraction aYScale( aWinSize.Height() * 4, aFrameSize.Height() * 5 );

    if( aYScale < aXScale )
        aXScale = aYScale;

    MapMode aMapMode( GetMapMode() );

    aMapMode.SetScaleX( aXScale );
    aMapMode.SetScaleY( aXScale );

    SetMapMode( aMapMode );
    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
