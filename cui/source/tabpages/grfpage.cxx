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

#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/sfxsids.hrc>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/brushitem.hxx>
#include <grfpage.hxx>
#include <svx/grfcrop.hxx>
#include <rtl/ustring.hxx>
#include <cuires.hrc>
#include <tools/fract.hxx>
#include <svx/dialogs.hrc>
#include <vcl/builderfactory.hxx>
#include <vcl/settings.hxx>

#define CM_1_TO_TWIP        567
#define TWIP_TO_INCH        1440


static inline long lcl_GetValue( MetricField& rMetric, FieldUnit eUnit )
{
    return static_cast<long>(rMetric.Denormalize( rMetric.GetValue( eUnit )));
}

/*--------------------------------------------------------------------
    description: crop graphic
 --------------------------------------------------------------------*/

SvxGrfCropPage::SvxGrfCropPage ( vcl::Window *pParent, const SfxItemSet &rSet )
    : SfxTabPage(pParent, "CropPage", "cui/ui/croppage.ui", &rSet)
    , pLastCropField(nullptr)
    , nOldWidth(0)
    , nOldHeight(0)
    , bReset(false)
    , bInitialized(false)
    , bSetOrigSize(false)
{
    get(m_pCropFrame, "cropframe");
    get(m_pScaleFrame, "scaleframe");
    get(m_pSizeFrame, "sizeframe");
    get(m_pOrigSizeGrid, "origsizegrid");
    get(m_pZoomConstRB, "keepscale");
    get(m_pSizeConstRB, "keepsize");
    get(m_pOrigSizeFT, "origsizeft");
    get(m_pOrigSizePB, "origsize");
    get(m_pLeftMF, "left");
    get(m_pRightMF, "right");
    get(m_pTopMF, "top");
    get(m_pBottomMF, "bottom");
    get(m_pWidthZoomMF, "widthzoom");
    get(m_pHeightZoomMF, "heightzoom");
    get(m_pWidthMF, "width");
    get(m_pHeightMF, "height");
    get(m_pExampleWN, "preview");

    SetExchangeSupport();

    // set the correct metric
    const FieldUnit eMetric = GetModuleFieldUnit( rSet );

    SetFieldUnit( *m_pWidthMF, eMetric );
    SetFieldUnit( *m_pHeightMF, eMetric );
    SetFieldUnit( *m_pLeftMF, eMetric );
    SetFieldUnit( *m_pRightMF, eMetric );
    SetFieldUnit( *m_pTopMF , eMetric );
    SetFieldUnit( *m_pBottomMF, eMetric );

    Link<Edit&,void> aLk = LINK(this, SvxGrfCropPage, SizeHdl);
    m_pWidthMF->SetModifyHdl( aLk );
    m_pHeightMF->SetModifyHdl( aLk );

    aLk = LINK(this, SvxGrfCropPage, ZoomHdl);
    m_pWidthZoomMF->SetModifyHdl( aLk );
    m_pHeightZoomMF->SetModifyHdl( aLk );

    Link<SpinField&,void> aLk3 = LINK(this, SvxGrfCropPage, CropHdl);
    m_pLeftMF->SetDownHdl( aLk3 );
    m_pRightMF->SetDownHdl( aLk3 );
    m_pTopMF->SetDownHdl( aLk3 );
    m_pBottomMF->SetDownHdl( aLk3 );
    m_pLeftMF->SetUpHdl( aLk3 );
    m_pRightMF->SetUpHdl( aLk3 );
    m_pTopMF->SetUpHdl( aLk3 );
    m_pBottomMF->SetUpHdl( aLk3 );

    aLk = LINK(this, SvxGrfCropPage, CropModifyHdl);
    m_pLeftMF->SetModifyHdl( aLk );
    m_pRightMF->SetModifyHdl( aLk );
    m_pTopMF->SetModifyHdl( aLk );
    m_pBottomMF->SetModifyHdl( aLk );

    Link<Control&,void> aLk2 = LINK(this, SvxGrfCropPage, CropLoseFocusHdl);
    m_pLeftMF->SetLoseFocusHdl( aLk2 );
    m_pRightMF->SetLoseFocusHdl( aLk2 );
    m_pTopMF->SetLoseFocusHdl( aLk2 );
    m_pBottomMF->SetLoseFocusHdl( aLk2 );

    m_pOrigSizePB->SetClickHdl( LINK(this, SvxGrfCropPage, OrigSizeHdl) );

    aTimer.SetTimeoutHdl(LINK(this, SvxGrfCropPage, Timeout));
    aTimer.SetTimeout( 1500 );
}

SvxGrfCropPage::~SvxGrfCropPage()
{
    disposeOnce();
}

void SvxGrfCropPage::dispose()
{
    aTimer.Stop();
    m_pCropFrame.clear();
    m_pZoomConstRB.clear();
    m_pSizeConstRB.clear();
    m_pLeftMF.clear();
    m_pRightMF.clear();
    m_pTopMF.clear();
    m_pBottomMF.clear();
    m_pScaleFrame.clear();
    m_pWidthZoomMF.clear();
    m_pHeightZoomMF.clear();
    m_pSizeFrame.clear();
    m_pWidthMF.clear();
    m_pHeightMF.clear();
    m_pOrigSizeGrid.clear();
    m_pOrigSizeFT.clear();
    m_pOrigSizePB.clear();
    m_pExampleWN.clear();
    pLastCropField.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxGrfCropPage::Create(vcl::Window *pParent, const SfxItemSet *rSet)
{
    return VclPtr<SvxGrfCropPage>::Create( pParent, *rSet );
}

void SvxGrfCropPage::Reset( const SfxItemSet *rSet )
{
    const SfxPoolItem* pItem;
    const SfxItemPool& rPool = *rSet->GetPool();

    if(SfxItemState::SET == rSet->GetItemState( rPool.GetWhich(
                                    SID_ATTR_GRAF_KEEP_ZOOM ), true, &pItem ))
    {
        if( static_cast<const SfxBoolItem*>(pItem)->GetValue() )
            m_pZoomConstRB->Check();
        else
            m_pSizeConstRB->Check();
        m_pZoomConstRB->SaveValue();
    }

    sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_CROP );
    if( SfxItemState::SET == rSet->GetItemState( nW, true, &pItem))
    {
        FieldUnit eUnit = MapToFieldUnit( rSet->GetPool()->GetMetric( nW ));

        const SvxGrfCrop* pCrop = static_cast<const SvxGrfCrop*>(pItem);

        m_pExampleWN->SetLeft(     pCrop->GetLeft());
        m_pExampleWN->SetRight(    pCrop->GetRight());
        m_pExampleWN->SetTop(      pCrop->GetTop());
        m_pExampleWN->SetBottom(   pCrop->GetBottom());

        m_pLeftMF->SetValue( m_pLeftMF->Normalize( pCrop->GetLeft()), eUnit );
        m_pRightMF->SetValue( m_pRightMF->Normalize( pCrop->GetRight()), eUnit );
        m_pTopMF->SetValue( m_pTopMF->Normalize( pCrop->GetTop()), eUnit );
        m_pBottomMF->SetValue( m_pBottomMF->Normalize( pCrop->GetBottom()), eUnit );
    }
    else
    {
        m_pLeftMF->SetValue( 0 );
        m_pRightMF->SetValue( 0 );
        m_pTopMF->SetValue( 0 );
        m_pBottomMF->SetValue( 0 );
    }

    nW = rPool.GetWhich( SID_ATTR_PAGE_SIZE );
    if ( SfxItemState::SET == rSet->GetItemState( nW, false, &pItem ) )
    {
        // orientation and size from the PageItem
        FieldUnit eUnit = MapToFieldUnit( rSet->GetPool()->GetMetric( nW ));

        aPageSize = static_cast<const SvxSizeItem*>(pItem)->GetSize();

        sal_Int64 nTmp = m_pHeightMF->Normalize(aPageSize.Height());
        m_pHeightMF->SetMax( nTmp, eUnit );
        nTmp = m_pWidthMF->Normalize(aPageSize.Width());
        m_pWidthMF->SetMax( nTmp, eUnit );
        nTmp = m_pWidthMF->Normalize( 23 );
        m_pHeightMF->SetMin( nTmp, eUnit );
        m_pWidthMF->SetMin( nTmp, eUnit );
    }
    else
    {
        aPageSize = OutputDevice::LogicToLogic(
                        Size( CM_1_TO_TWIP,  CM_1_TO_TWIP ),
                        MapMode( MAP_TWIP ),
                        MapMode( (MapUnit)rSet->GetPool()->GetMetric( nW ) ) );
    }

    bool bFound = false;
    if( SfxItemState::SET == rSet->GetItemState( SID_ATTR_GRAF_GRAPHIC, false, &pItem ) )
    {
        OUString referer;
        SfxStringItem const * it = static_cast<SfxStringItem const *>(
            rSet->GetItem(SID_REFERER));
        if (it != nullptr) {
            referer = it->GetValue();
        }
        const Graphic* pGrf = static_cast<const SvxBrushItem*>(pItem)->GetGraphic(referer);
        if( pGrf )
        {
            aOrigSize = GetGrfOrigSize( *pGrf );
            if (pGrf->GetType() == GraphicType::Bitmap && aOrigSize.Width() && aOrigSize.Height())
            {
                Bitmap aBitmap = pGrf->GetBitmap();
                aOrigPixelSize = aBitmap.GetSizePixel();
            }

            if( aOrigSize.Width() && aOrigSize.Height() )
            {
                CalcMinMaxBorder();
                m_pExampleWN->SetGraphic( *pGrf );
                m_pExampleWN->SetFrameSize( aOrigSize );

                bFound = true;
                if( !static_cast<const SvxBrushItem*>(pItem)->GetGraphicLink().isEmpty() )
                    aGraphicName = static_cast<const SvxBrushItem*>(pItem)->GetGraphicLink();
            }
        }
    }

    GraphicHasChanged( bFound );
    bReset = true;
    ActivatePage( *rSet );
    bReset = false;
}

bool SvxGrfCropPage::FillItemSet(SfxItemSet *rSet)
{
    const SfxItemPool& rPool = *rSet->GetPool();
    bool bModified = false;
    if( m_pWidthMF->IsValueChangedFromSaved() ||
        m_pHeightMF->IsValueChangedFromSaved() )
    {
        sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_FRMSIZE );
        FieldUnit eUnit = MapToFieldUnit( rSet->GetPool()->GetMetric( nW ));

        SvxSizeItem aSz( nW );

        // size could already have been set from another page
        const SfxItemSet* pExSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : nullptr;
        const SfxPoolItem* pItem = nullptr;
        if( pExSet && SfxItemState::SET ==
                pExSet->GetItemState( nW, false, &pItem ) )
            aSz = *static_cast<const SvxSizeItem*>(pItem);
        else
            aSz = static_cast<const SvxSizeItem&>(GetItemSet().Get( nW ));

        Size aTmpSz( aSz.GetSize() );
        if( m_pWidthMF->IsValueChangedFromSaved() )
            aTmpSz.Width() = lcl_GetValue( *m_pWidthMF, eUnit );
        if( m_pHeightMF->IsValueChangedFromSaved() )
            aTmpSz.Height() = lcl_GetValue( *m_pHeightMF, eUnit );
        aSz.SetSize( aTmpSz );
        m_pWidthMF->SaveValue();
        m_pHeightMF->SaveValue();

        bModified |= nullptr != rSet->Put( aSz );

        if( bSetOrigSize )
        {
            bModified |= nullptr != rSet->Put( SvxSizeItem( rPool.GetWhich(
                        SID_ATTR_GRAF_FRMSIZE_PERCENT ), Size( 0, 0 )) );
        }
    }
    if( m_pLeftMF->IsModified() || m_pRightMF->IsModified() ||
        m_pTopMF->IsModified()  || m_pBottomMF->IsModified() )
    {
        sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_CROP );
        FieldUnit eUnit = MapToFieldUnit( rSet->GetPool()->GetMetric( nW ));
        std::unique_ptr<SvxGrfCrop> pNew(static_cast<SvxGrfCrop*>(rSet->Get( nW ).Clone()));

        pNew->SetLeft( lcl_GetValue( *m_pLeftMF, eUnit ) );
        pNew->SetRight( lcl_GetValue( *m_pRightMF, eUnit ) );
        pNew->SetTop( lcl_GetValue( *m_pTopMF, eUnit ) );
        pNew->SetBottom( lcl_GetValue( *m_pBottomMF, eUnit ) );
        bModified |= nullptr != rSet->Put( *pNew );
    }

    if( m_pZoomConstRB->IsValueChangedFromSaved() )
    {
        bModified |= nullptr != rSet->Put( SfxBoolItem( rPool.GetWhich(
                    SID_ATTR_GRAF_KEEP_ZOOM), m_pZoomConstRB->IsChecked() ) );
    }

    bInitialized = false;

    return bModified;
}

void SvxGrfCropPage::ActivatePage(const SfxItemSet& rSet)
{
#ifdef DBG_UTIL
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
#endif

    bSetOrigSize = false;

    // Size
    Size aSize;
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_GRAF_FRMSIZE, false, &pItem ) )
        aSize = static_cast<const SvxSizeItem*>(pItem)->GetSize();

    nOldWidth = aSize.Width();
    nOldHeight = aSize.Height();

    sal_Int64 nWidth = m_pWidthMF->Normalize(nOldWidth);
    sal_Int64 nHeight = m_pHeightMF->Normalize(nOldHeight);

    if (nWidth != m_pWidthMF->GetValue(FUNIT_TWIP))
    {
        if(!bReset)
        {
            // value was changed by wrap-tabpage and has to
            // be set with modify-flag
            m_pWidthMF->SetUserValue(nWidth, FUNIT_TWIP);
        }
        else
            m_pWidthMF->SetValue(nWidth, FUNIT_TWIP);
    }
    m_pWidthMF->SaveValue();

    if (nHeight != m_pHeightMF->GetValue(FUNIT_TWIP))
    {
        if (!bReset)
        {
            // value was changed by wrap-tabpage and has to
            // be set with modify-flag
            m_pHeightMF->SetUserValue(nHeight, FUNIT_TWIP);
        }
        else
            m_pHeightMF->SetValue(nHeight, FUNIT_TWIP);
    }
    m_pHeightMF->SaveValue();
    bInitialized = true;

    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, false, &pItem ) )
    {
        const SvxBrushItem& rBrush = *static_cast<const SvxBrushItem*>(pItem);
        if( !rBrush.GetGraphicLink().isEmpty() &&
            aGraphicName != rBrush.GetGraphicLink() )
            aGraphicName = rBrush.GetGraphicLink();

        OUString referer;
        SfxStringItem const * it = static_cast<SfxStringItem const *>(
            rSet.GetItem(SID_REFERER));
        if (it != nullptr) {
            referer = it->GetValue();
        }
        const Graphic* pGrf = rBrush.GetGraphic(referer);
        if( pGrf )
        {
            m_pExampleWN->SetGraphic( *pGrf );
            aOrigSize = GetGrfOrigSize( *pGrf );
            if (pGrf->GetType() == GraphicType::Bitmap && aOrigSize.Width() > 1 && aOrigSize.Height() > 1) {
                Bitmap aBitmap = pGrf->GetBitmap();
                aOrigPixelSize = aBitmap.GetSizePixel();
            }
            m_pExampleWN->SetFrameSize(aOrigSize);
            GraphicHasChanged( aOrigSize.Width() && aOrigSize.Height() );
            CalcMinMaxBorder();
        }
        else
            GraphicHasChanged( false );
    }

    CalcZoom();
}

DeactivateRC SvxGrfCropPage::DeactivatePage(SfxItemSet *_pSet)
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

/*--------------------------------------------------------------------
    description: scale changed, adjust size
 --------------------------------------------------------------------*/

IMPL_LINK_TYPED( SvxGrfCropPage, ZoomHdl, Edit&, rField, void )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    if( &rField == m_pWidthZoomMF )
    {
        long nLRBorders = lcl_GetValue(*m_pLeftMF, eUnit)
                         +lcl_GetValue(*m_pRightMF, eUnit);
        m_pWidthMF->SetValue( m_pWidthMF->Normalize(
            ((aOrigSize.Width() - nLRBorders) * static_cast<MetricField&>(rField).GetValue())/100L),
            eUnit);
    }
    else
    {
        long nULBorders = lcl_GetValue(*m_pTopMF, eUnit)
                         +lcl_GetValue(*m_pBottomMF, eUnit);
        m_pHeightMF->SetValue( m_pHeightMF->Normalize(
            ((aOrigSize.Height() - nULBorders ) * static_cast<MetricField&>(rField).GetValue())/100L) ,
            eUnit );
    }
}

/*--------------------------------------------------------------------
    description: change size, adjust scale
 --------------------------------------------------------------------*/

IMPL_LINK_TYPED( SvxGrfCropPage, SizeHdl, Edit&, rField, void )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    Size aSize( lcl_GetValue(*m_pWidthMF, eUnit),
                lcl_GetValue(*m_pHeightMF, eUnit) );

    if(&rField == m_pWidthMF)
    {
        long nWidth = aOrigSize.Width() -
                ( lcl_GetValue(*m_pLeftMF, eUnit) +
                  lcl_GetValue(*m_pRightMF, eUnit) );
        if(!nWidth)
            nWidth++;
        sal_uInt16 nZoom = (sal_uInt16)( aSize.Width() * 100L / nWidth);
        m_pWidthZoomMF->SetValue(nZoom);
    }
    else
    {
        long nHeight = aOrigSize.Height() -
                ( lcl_GetValue(*m_pTopMF, eUnit) +
                  lcl_GetValue(*m_pBottomMF, eUnit));
        if(!nHeight)
            nHeight++;
        sal_uInt16 nZoom = (sal_uInt16)( aSize.Height() * 100L/ nHeight);
        m_pHeightZoomMF->SetValue(nZoom);
    }
}

/*--------------------------------------------------------------------
    description: evaluate border
 --------------------------------------------------------------------*/

IMPL_LINK_TYPED( SvxGrfCropPage, CropHdl, SpinField&, rField, void )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    bool bZoom = m_pZoomConstRB->IsChecked();
    if( &rField == m_pLeftMF || &rField == m_pRightMF )
    {
        long nLeft = lcl_GetValue( *m_pLeftMF, eUnit );
        long nRight = lcl_GetValue( *m_pRightMF, eUnit );
        long nWidthZoom = static_cast<long>(m_pWidthZoomMF->GetValue());
        if(bZoom && ( ( ( aOrigSize.Width() - (nLeft + nRight )) * nWidthZoom )
                            / 100 >= aPageSize.Width() ) )
        {
            if(&rField == m_pLeftMF)
            {
                nLeft = aOrigSize.Width() -
                            ( aPageSize.Width() * 100 / nWidthZoom + nRight );
                m_pLeftMF->SetValue( m_pLeftMF->Normalize( nLeft ), eUnit );
            }
            else
            {
                nRight = aOrigSize.Width() -
                            ( aPageSize.Width() * 100 / nWidthZoom + nLeft );
                m_pRightMF->SetValue( m_pRightMF->Normalize( nRight ), eUnit );
            }
        }
        if (AllSettings::GetLayoutRTL())
        {
            m_pExampleWN->SetLeft(nRight);
            m_pExampleWN->SetRight(nLeft);
        }
        else
        {
            m_pExampleWN->SetLeft(nLeft);
            m_pExampleWN->SetRight(nRight);
        }
        if(bZoom)
        {
            // scale stays, recompute width
            ZoomHdl(*m_pWidthZoomMF);
        }
    }
    else
    {
        long nTop = lcl_GetValue( *m_pTopMF, eUnit );
        long nBottom = lcl_GetValue( *m_pBottomMF, eUnit );
        long nHeightZoom = static_cast<long>(m_pHeightZoomMF->GetValue());
        if(bZoom && ( ( ( aOrigSize.Height() - (nTop + nBottom )) * nHeightZoom)
                                            / 100 >= aPageSize.Height()))
        {
            if(&rField == m_pTopMF)
            {
                nTop = aOrigSize.Height() -
                            ( aPageSize.Height() * 100 / nHeightZoom + nBottom);
                m_pTopMF->SetValue( m_pWidthMF->Normalize( nTop ), eUnit );
            }
            else
            {
                nBottom = aOrigSize.Height() -
                            ( aPageSize.Height() * 100 / nHeightZoom + nTop);
                m_pBottomMF->SetValue( m_pWidthMF->Normalize( nBottom ), eUnit );
            }
        }
        m_pExampleWN->SetTop( nTop );
        m_pExampleWN->SetBottom( nBottom );
        if(bZoom)
        {
            // scale stays, recompute height
            ZoomHdl(*m_pHeightZoomMF);
        }
    }
    m_pExampleWN->Invalidate();
    // size and border changed -> recompute scale
    if(!bZoom)
        CalcZoom();
    CalcMinMaxBorder();
}
/*--------------------------------------------------------------------
    description: set original size
 --------------------------------------------------------------------*/

IMPL_LINK_NOARG_TYPED(SvxGrfCropPage, OrigSizeHdl, Button*, void)
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    long nWidth = aOrigSize.Width() -
        lcl_GetValue( *m_pLeftMF, eUnit ) -
        lcl_GetValue( *m_pRightMF, eUnit );
    m_pWidthMF->SetValue( m_pWidthMF->Normalize( nWidth ), eUnit );
    long nHeight = aOrigSize.Height() -
        lcl_GetValue( *m_pTopMF, eUnit ) -
        lcl_GetValue( *m_pBottomMF, eUnit );
    m_pHeightMF->SetValue( m_pHeightMF->Normalize( nHeight ), eUnit );
    m_pWidthZoomMF->SetValue(100);
    m_pHeightZoomMF->SetValue(100);
    bSetOrigSize = true;
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

    long nWidth = lcl_GetValue( *m_pWidthMF, eUnit );
    long nHeight = lcl_GetValue( *m_pHeightMF, eUnit );
    long nLRBorders = lcl_GetValue( *m_pLeftMF, eUnit ) +
                      lcl_GetValue( *m_pRightMF, eUnit );
    long nULBorders = lcl_GetValue( *m_pTopMF, eUnit ) +
                      lcl_GetValue( *m_pBottomMF, eUnit );
    sal_uInt16 nZoom = 0;
    long nDen;
    if( (nDen = aOrigSize.Width() - nLRBorders) > 0)
        nZoom = (sal_uInt16)((( nWidth  * 1000L / nDen )+5)/10);
    m_pWidthZoomMF->SetValue(nZoom);
    if( (nDen = aOrigSize.Height() - nULBorders) > 0)
        nZoom = (sal_uInt16)((( nHeight * 1000L / nDen )+5)/10);
    else
        nZoom = 0;
    m_pHeightZoomMF->SetValue(nZoom);
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
    long nR = lcl_GetValue(*m_pRightMF, eUnit );
    long nMinWidth = (aOrigSize.Width() * 10) /11;
    long nMin = nMinWidth - (nR >= 0 ? nR : 0);
    m_pLeftMF->SetMax( m_pLeftMF->Normalize(nMin), eUnit );

    long nL = lcl_GetValue(*m_pLeftMF, eUnit );
    nMin = nMinWidth - (nL >= 0 ? nL : 0);
    m_pRightMF->SetMax( m_pRightMF->Normalize(nMin), eUnit );

    long nUp  = lcl_GetValue( *m_pTopMF, eUnit );
    long nMinHeight = (aOrigSize.Height() * 10) /11;
    nMin = nMinHeight - (nUp >= 0 ? nUp : 0);
    m_pBottomMF->SetMax( m_pBottomMF->Normalize(nMin), eUnit );

    long nLow = lcl_GetValue(*m_pBottomMF, eUnit );
    nMin = nMinHeight - (nLow >= 0 ? nLow : 0);
    m_pTopMF->SetMax( m_pTopMF->Normalize(nMin), eUnit );
}
/*--------------------------------------------------------------------
    description:   set spinsize to 1/20 of the original size,
                   fill FixedText with the original size
 --------------------------------------------------------------------*/

void SvxGrfCropPage::GraphicHasChanged( bool bFound )
{
    if( bFound )
    {
        SfxItemPool* pPool = GetItemSet().GetPool();
        DBG_ASSERT( pPool, "Wo ist der Pool" );
        FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ));

        sal_Int64 nSpin = m_pLeftMF->Normalize(aOrigSize.Width()) / 20;
        nSpin = MetricField::ConvertValue( nSpin, aOrigSize.Width(), 0,
                                               eUnit, m_pLeftMF->GetUnit());

        // if the margin is too big, it is set to 1/3 on both pages
        long nR = lcl_GetValue( *m_pRightMF, eUnit );
        long nL = lcl_GetValue( *m_pLeftMF, eUnit );
        if((nL + nR) < - aOrigSize.Width())
        {
            long nVal = aOrigSize.Width() / -3;
            m_pRightMF->SetValue( m_pRightMF->Normalize( nVal ), eUnit );
            m_pLeftMF->SetValue( m_pLeftMF->Normalize( nVal ), eUnit );
            m_pExampleWN->SetLeft(nVal);
            m_pExampleWN->SetRight(nVal);
        }
        long nUp  = lcl_GetValue(*m_pTopMF, eUnit );
        long nLow = lcl_GetValue(*m_pBottomMF, eUnit );
        if((nUp + nLow) < - aOrigSize.Height())
        {
            long nVal = aOrigSize.Height() / -3;
            m_pTopMF->SetValue( m_pTopMF->Normalize( nVal ), eUnit );
            m_pBottomMF->SetValue( m_pBottomMF->Normalize( nVal ), eUnit );
            m_pExampleWN->SetTop(nVal);
            m_pExampleWN->SetBottom(nVal);
        }

        m_pLeftMF->SetSpinSize(nSpin);
        m_pRightMF->SetSpinSize(nSpin);
        nSpin = m_pTopMF->Normalize(aOrigSize.Height()) / 20;
        nSpin = MetricField::ConvertValue( nSpin, aOrigSize.Width(), 0,
                                               eUnit, m_pLeftMF->GetUnit() );
        m_pTopMF->SetSpinSize(nSpin);
        m_pBottomMF->SetSpinSize(nSpin);

        // display original size
        const FieldUnit eMetric = GetModuleFieldUnit( GetItemSet() );

        ScopedVclPtrInstance< MetricField > aFld(this, WB_HIDE);
        SetFieldUnit( *aFld.get(), eMetric );
        aFld->SetDecimalDigits( m_pWidthMF->GetDecimalDigits() );
        aFld->SetMax( LONG_MAX - 1 );

        aFld->SetValue( aFld->Normalize( aOrigSize.Width() ), eUnit );
        OUString sTemp = aFld->GetText();
        aFld->SetValue( aFld->Normalize( aOrigSize.Height() ), eUnit );
        // multiplication sign (U+00D7)
        sTemp += OUString( sal_Unicode (0x00D7) ) + aFld->GetText();

        if ( aOrigPixelSize.Width() && aOrigPixelSize.Height() ) {
             sal_Int32 ax = sal_Int32(floor((float)aOrigPixelSize.Width() /
                        ((float)aOrigSize.Width()/TWIP_TO_INCH)+0.5));
             sal_Int32 ay = sal_Int32(floor((float)aOrigPixelSize.Height() /
                        ((float)aOrigSize.Height()/TWIP_TO_INCH)+0.5));
             sTemp += " ";
             sTemp += CUI_RESSTR( RID_SVXSTR_PPI );
             OUString sPPI = OUString::number(ax);
             if (abs(ax - ay) > 1) {
                sPPI += OUString( sal_Unicode (0x00D7) ) + OUString::number(ay);
             }
             sTemp = sTemp.replaceAll("%1", sPPI);
        }
        m_pOrigSizeFT->SetText( sTemp );
    }

    m_pCropFrame->Enable(bFound);
    m_pScaleFrame->Enable(bFound);
    m_pSizeFrame->Enable(bFound);
    m_pOrigSizeGrid->Enable(bFound);
    m_pZoomConstRB->Enable(bFound);
}

IMPL_LINK_NOARG_TYPED(SvxGrfCropPage, Timeout, Timer *, void)
{
    DBG_ASSERT(pLastCropField,"Timeout ohne Feld?");
    CropHdl(*pLastCropField);
    pLastCropField = nullptr;
}


IMPL_LINK_TYPED( SvxGrfCropPage, CropLoseFocusHdl, Control&, rControl, void )
{
    MetricField* pField = static_cast<MetricField*>(&rControl);
    aTimer.Stop();
    CropHdl(*pField);
    pLastCropField = nullptr;
}


IMPL_LINK_TYPED( SvxGrfCropPage, CropModifyHdl, Edit&, rField, void )
{
    aTimer.Start();
    pLastCropField = static_cast<MetricField*>(&rField);
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

SvxCropExample::SvxCropExample( vcl::Window* pPar, WinBits nStyle )
    : Window( pPar, nStyle)
    , aFrameSize( OutputDevice::LogicToLogic(
                            Size( CM_1_TO_TWIP / 2, CM_1_TO_TWIP / 2 ),
                            MapMode( MAP_TWIP ), GetMapMode() ))
    , aTopLeft(0,0)
    , aBottomRight(0,0)
{
    SetBorderStyle( WindowBorderStyle::MONO );
}

Size SvxCropExample::GetOptimalSize() const
{
    return LogicToPixel(Size(78, 78), MAP_APPFONT);
}

VCL_BUILDER_DECL_FACTORY(SvxCropExample)
{
    WinBits nWinStyle = 0;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<SvxCropExample>::Create(pParent, nWinStyle);
}

void SvxCropExample::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    Size aWinSize(rRenderContext.PixelToLogic(GetOutputSizePixel()));
    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor());
    rRenderContext.SetRasterOp(ROP_OVERPAINT);
    rRenderContext.DrawRect(Rectangle(Point(), aWinSize));

    rRenderContext.SetLineColor(Color(COL_WHITE));
    Rectangle aRect(Point((aWinSize.Width() - aFrameSize.Width())/2,
                          (aWinSize.Height() - aFrameSize.Height())/2),
                          aFrameSize);
    aGrf.Draw(&rRenderContext, aRect.TopLeft(), aRect.GetSize());

    Size aSz(2, 0);
    aSz = rRenderContext.PixelToLogic(aSz);
    rRenderContext.SetFillColor(Color(COL_TRANSPARENT));
    rRenderContext.SetRasterOp(ROP_INVERT);
    aRect.Left()    += aTopLeft.Y();
    aRect.Top()     += aTopLeft.X();
    aRect.Right()   -= aBottomRight.Y();
    aRect.Bottom()  -= aBottomRight.X();
    rRenderContext.DrawRect(aRect);
}

void SvxCropExample::Resize()
{
    SetFrameSize(aFrameSize);
}

void SvxCropExample::SetFrameSize( const Size& rSz )
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
