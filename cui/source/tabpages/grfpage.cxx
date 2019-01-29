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

#include <memory>
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
#include <tools/fract.hxx>
#include <svx/dialogs.hrc>
#include <strings.hrc>
#include <vcl/builderfactory.hxx>
#include <vcl/field.hxx>
#include <vcl/settings.hxx>

#define CM_1_TO_TWIP        567
#define TWIP_TO_INCH        1440


static int lcl_GetValue(const weld::MetricSpinButton& rMetric, FieldUnit eUnit)
{
    return rMetric.denormalize(rMetric.get_value(eUnit));
}

/*--------------------------------------------------------------------
    description: crop graphic
 --------------------------------------------------------------------*/

SvxGrfCropPage::SvxGrfCropPage(TabPageParent pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "cui/ui/croppage.ui", "CropPage", &rSet)
    , nOldWidth(0)
    , nOldHeight(0)
    , bSetOrigSize(false)
    , m_xCropFrame(m_xBuilder->weld_widget("cropframe"))
    , m_xZoomConstRB(m_xBuilder->weld_radio_button("keepscale"))
    , m_xSizeConstRB(m_xBuilder->weld_radio_button("keepsize"))
    , m_xLeftMF(m_xBuilder->weld_metric_spin_button("left", FieldUnit::CM))
    , m_xRightMF(m_xBuilder->weld_metric_spin_button("right", FieldUnit::CM))
    , m_xTopMF(m_xBuilder->weld_metric_spin_button("top", FieldUnit::CM))
    , m_xBottomMF(m_xBuilder->weld_metric_spin_button("bottom", FieldUnit::CM))
    , m_xScaleFrame(m_xBuilder->weld_widget("scaleframe"))
    , m_xWidthZoomMF(m_xBuilder->weld_metric_spin_button("widthzoom", FieldUnit::PERCENT))
    , m_xHeightZoomMF(m_xBuilder->weld_metric_spin_button("heightzoom", FieldUnit::PERCENT))
    , m_xSizeFrame(m_xBuilder->weld_widget("sizeframe"))
    , m_xWidthMF(m_xBuilder->weld_metric_spin_button("width", FieldUnit::CM))
    , m_xHeightMF(m_xBuilder->weld_metric_spin_button("height", FieldUnit::CM))
    , m_xOrigSizeGrid(m_xBuilder->weld_widget("origsizegrid"))
    , m_xOrigSizeFT(m_xBuilder->weld_label("origsizeft"))
    , m_xOrigSizePB(m_xBuilder->weld_button("origsize"))
    , m_xExampleWN(new weld::CustomWeld(*m_xBuilder, "preview", m_aExampleWN))
{
    SetExchangeSupport();

    // set the correct metric
    const FieldUnit eMetric = GetModuleFieldUnit( rSet );

    SetFieldUnit( *m_xWidthMF, eMetric );
    SetFieldUnit( *m_xHeightMF, eMetric );
    SetFieldUnit( *m_xLeftMF, eMetric );
    SetFieldUnit( *m_xRightMF, eMetric );
    SetFieldUnit( *m_xTopMF , eMetric );
    SetFieldUnit( *m_xBottomMF, eMetric );

    Link<weld::MetricSpinButton&,void> aLk = LINK(this, SvxGrfCropPage, SizeHdl);
    m_xWidthMF->connect_value_changed( aLk );
    m_xHeightMF->connect_value_changed( aLk );

    aLk = LINK(this, SvxGrfCropPage, ZoomHdl);
    m_xWidthZoomMF->connect_value_changed( aLk );
    m_xHeightZoomMF->connect_value_changed( aLk );

    aLk = LINK(this, SvxGrfCropPage, CropModifyHdl);
    m_xLeftMF->connect_value_changed( aLk );
    m_xRightMF->connect_value_changed( aLk );
    m_xTopMF->connect_value_changed( aLk );
    m_xBottomMF->connect_value_changed( aLk );

    m_xOrigSizePB->connect_clicked(LINK(this, SvxGrfCropPage, OrigSizeHdl));
}

SvxGrfCropPage::~SvxGrfCropPage()
{
    disposeOnce();
}

void SvxGrfCropPage::dispose()
{
    m_xExampleWN.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxGrfCropPage::Create(TabPageParent pParent, const SfxItemSet *rSet)
{
    return VclPtr<SvxGrfCropPage>::Create(pParent, *rSet);
}

void SvxGrfCropPage::Reset( const SfxItemSet *rSet )
{
    const SfxPoolItem* pItem;
    const SfxItemPool& rPool = *rSet->GetPool();

    if(SfxItemState::SET == rSet->GetItemState( rPool.GetWhich(
                                    SID_ATTR_GRAF_KEEP_ZOOM ), true, &pItem ))
    {
        if( static_cast<const SfxBoolItem*>(pItem)->GetValue() )
            m_xZoomConstRB->set_active(true);
        else
            m_xSizeConstRB->set_active(true);
        m_xZoomConstRB->save_state();
    }

    sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_CROP );
    if( SfxItemState::SET == rSet->GetItemState( nW, true, &pItem))
    {
        FieldUnit eUnit = MapToFieldUnit( rSet->GetPool()->GetMetric( nW ));

        const SvxGrfCrop* pCrop = static_cast<const SvxGrfCrop*>(pItem);

        m_aExampleWN.SetLeft(pCrop->GetLeft());
        m_aExampleWN.SetRight(pCrop->GetRight());
        m_aExampleWN.SetTop(pCrop->GetTop());
        m_aExampleWN.SetBottom(pCrop->GetBottom());

        m_xLeftMF->set_value( m_xLeftMF->normalize( pCrop->GetLeft()), eUnit );
        m_xRightMF->set_value( m_xRightMF->normalize( pCrop->GetRight()), eUnit );
        m_xTopMF->set_value( m_xTopMF->normalize( pCrop->GetTop()), eUnit );
        m_xBottomMF->set_value( m_xBottomMF->normalize( pCrop->GetBottom()), eUnit );
    }
    else
    {
        m_xLeftMF->set_value(0, FieldUnit::NONE);
        m_xRightMF->set_value(0, FieldUnit::NONE);
        m_xTopMF->set_value(0, FieldUnit::NONE);
        m_xBottomMF->set_value(0, FieldUnit::NONE);
    }

    m_xLeftMF->save_value();
    m_xRightMF->save_value();
    m_xTopMF->save_value();
    m_xBottomMF->save_value();

    nW = rPool.GetWhich( SID_ATTR_PAGE_SIZE );
    if ( SfxItemState::SET == rSet->GetItemState( nW, false, &pItem ) )
    {
        // orientation and size from the PageItem
        FieldUnit eUnit = MapToFieldUnit( rSet->GetPool()->GetMetric( nW ));

        aPageSize = static_cast<const SvxSizeItem*>(pItem)->GetSize();

        auto nMin = m_xWidthMF->normalize( 23 );
        auto nMax = m_xHeightMF->normalize(aPageSize.Height());
        m_xHeightMF->set_range(nMin, nMax, eUnit);
        nMax = m_xWidthMF->normalize(aPageSize.Width());
        m_xWidthMF->set_range(nMin, nMax, eUnit);
    }
    else
    {
        aPageSize = OutputDevice::LogicToLogic(
                        Size( CM_1_TO_TWIP,  CM_1_TO_TWIP ),
                        MapMode( MapUnit::MapTwip ),
                        MapMode( rSet->GetPool()->GetMetric( nW ) ) );
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
                aOrigPixelSize = pGrf->GetSizePixel();
            }

            if( aOrigSize.Width() && aOrigSize.Height() )
            {
                CalcMinMaxBorder();
                m_aExampleWN.SetGraphic( *pGrf );
                m_aExampleWN.SetFrameSize( aOrigSize );

                bFound = true;
                if( !static_cast<const SvxBrushItem*>(pItem)->GetGraphicLink().isEmpty() )
                    aGraphicName = static_cast<const SvxBrushItem*>(pItem)->GetGraphicLink();
            }
        }
    }

    GraphicHasChanged( bFound );
    ActivatePage( *rSet );
}

bool SvxGrfCropPage::FillItemSet(SfxItemSet *rSet)
{
    const SfxItemPool& rPool = *rSet->GetPool();
    bool bModified = false;
    if( m_xWidthMF->get_value_changed_from_saved() ||
        m_xHeightMF->get_value_changed_from_saved() )
    {
        sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_FRMSIZE );
        FieldUnit eUnit = MapToFieldUnit( rSet->GetPool()->GetMetric( nW ));

        SvxSizeItem aSz( nW );

        // size could already have been set from another page
        const SfxItemSet* pExSet = GetDialogExampleSet();
        const SfxPoolItem* pItem = nullptr;
        if( pExSet && SfxItemState::SET ==
                pExSet->GetItemState( nW, false, &pItem ) )
            aSz = *static_cast<const SvxSizeItem*>(pItem);
        else
            aSz = static_cast<const SvxSizeItem&>(GetItemSet().Get( nW ));

        Size aTmpSz( aSz.GetSize() );
        if( m_xWidthMF->get_value_changed_from_saved() )
            aTmpSz.setWidth( lcl_GetValue( *m_xWidthMF, eUnit ) );
        if( m_xHeightMF->get_value_changed_from_saved() )
            aTmpSz.setHeight( lcl_GetValue( *m_xHeightMF, eUnit ) );
        aSz.SetSize( aTmpSz );
        m_xWidthMF->save_value();
        m_xHeightMF->save_value();

        bModified |= nullptr != rSet->Put( aSz );

        if( bSetOrigSize )
        {
            bModified |= nullptr != rSet->Put( SvxSizeItem( rPool.GetWhich(
                        SID_ATTR_GRAF_FRMSIZE_PERCENT ), Size( 0, 0 )) );
        }
    }
    if( m_xLeftMF->get_value_changed_from_saved() || m_xRightMF->get_value_changed_from_saved() ||
        m_xTopMF->get_value_changed_from_saved()  || m_xBottomMF->get_value_changed_from_saved() )
    {
        sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_CROP );
        FieldUnit eUnit = MapToFieldUnit( rSet->GetPool()->GetMetric( nW ));
        std::unique_ptr<SvxGrfCrop> pNew(static_cast<SvxGrfCrop*>(rSet->Get( nW ).Clone()));

        pNew->SetLeft( lcl_GetValue( *m_xLeftMF, eUnit ) );
        pNew->SetRight( lcl_GetValue( *m_xRightMF, eUnit ) );
        pNew->SetTop( lcl_GetValue( *m_xTopMF, eUnit ) );
        pNew->SetBottom( lcl_GetValue( *m_xBottomMF, eUnit ) );
        bModified |= nullptr != rSet->Put( *pNew );
    }

    if( m_xZoomConstRB->get_state_changed_from_saved() )
    {
        bModified |= nullptr != rSet->Put( SfxBoolItem( rPool.GetWhich(
                    SID_ATTR_GRAF_KEEP_ZOOM), m_xZoomConstRB->get_active() ) );
    }

    return bModified;
}

void SvxGrfCropPage::ActivatePage(const SfxItemSet& rSet)
{
#ifdef DBG_UTIL
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
#endif

    bSetOrigSize = false;

    // Size
    Size aSize;
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_GRAF_FRMSIZE, false, &pItem ) )
        aSize = static_cast<const SvxSizeItem*>(pItem)->GetSize();

    nOldWidth = aSize.Width();
    nOldHeight = aSize.Height();

    auto nWidth = m_xWidthMF->normalize(nOldWidth);
    auto nHeight = m_xHeightMF->normalize(nOldHeight);

    if (nWidth != m_xWidthMF->get_value(FieldUnit::TWIP))
        m_xWidthMF->set_value(nWidth, FieldUnit::TWIP);
    m_xWidthMF->save_value();

    if (nHeight != m_xHeightMF->get_value(FieldUnit::TWIP))
        m_xHeightMF->set_value(nHeight, FieldUnit::TWIP);
    m_xHeightMF->save_value();

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
            m_aExampleWN.SetGraphic( *pGrf );
            aOrigSize = GetGrfOrigSize( *pGrf );
            if (pGrf->GetType() == GraphicType::Bitmap && aOrigSize.Width() > 1 && aOrigSize.Height() > 1) {
                aOrigPixelSize = pGrf->GetSizePixel();
            }
            m_aExampleWN.SetFrameSize(aOrigSize);
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

IMPL_LINK( SvxGrfCropPage, ZoomHdl, weld::MetricSpinButton&, rField, void )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    if (&rField == m_xWidthZoomMF.get())
    {
        long nLRBorders = lcl_GetValue(*m_xLeftMF, eUnit)
                         +lcl_GetValue(*m_xRightMF, eUnit);
        m_xWidthMF->set_value( m_xWidthMF->normalize(
            ((aOrigSize.Width() - nLRBorders) * rField.get_value(FieldUnit::NONE))/100),
            eUnit);
    }
    else
    {
        long nULBorders = lcl_GetValue(*m_xTopMF, eUnit)
                         +lcl_GetValue(*m_xBottomMF, eUnit);
        m_xHeightMF->set_value( m_xHeightMF->normalize(
            ((aOrigSize.Height() - nULBorders ) * rField.get_value(FieldUnit::NONE))/100) ,
            eUnit );
    }
}

/*--------------------------------------------------------------------
    description: change size, adjust scale
 --------------------------------------------------------------------*/

IMPL_LINK( SvxGrfCropPage, SizeHdl, weld::MetricSpinButton&, rField, void )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    Size aSize( lcl_GetValue(*m_xWidthMF, eUnit),
                lcl_GetValue(*m_xHeightMF, eUnit) );

    if(&rField == m_xWidthMF.get())
    {
        long nWidth = aOrigSize.Width() -
                ( lcl_GetValue(*m_xLeftMF, eUnit) +
                  lcl_GetValue(*m_xRightMF, eUnit) );
        if(!nWidth)
            nWidth++;
        sal_uInt16 nZoom = static_cast<sal_uInt16>( aSize.Width() * 100 / nWidth);
        m_xWidthZoomMF->set_value(nZoom, FieldUnit::NONE);
    }
    else
    {
        long nHeight = aOrigSize.Height() -
                ( lcl_GetValue(*m_xTopMF, eUnit) +
                  lcl_GetValue(*m_xBottomMF, eUnit));
        if(!nHeight)
            nHeight++;
        sal_uInt16 nZoom = static_cast<sal_uInt16>( aSize.Height() * 100 / nHeight);
        m_xHeightZoomMF->set_value(nZoom, FieldUnit::NONE);
    }
}

/*--------------------------------------------------------------------
    description: evaluate border
 --------------------------------------------------------------------*/

IMPL_LINK( SvxGrfCropPage, CropModifyHdl, weld::MetricSpinButton&, rField, void )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    bool bZoom = m_xZoomConstRB->get_active();
    if (&rField == m_xLeftMF.get() || &rField == m_xRightMF.get())
    {
        long nLeft = lcl_GetValue( *m_xLeftMF, eUnit );
        long nRight = lcl_GetValue( *m_xRightMF, eUnit );
        long nWidthZoom = static_cast<long>(m_xWidthZoomMF->get_value(FieldUnit::NONE));
        if (bZoom && nWidthZoom != 0 && ( ( ( aOrigSize.Width() - (nLeft + nRight )) * nWidthZoom )
                            / 100 >= aPageSize.Width() ) )
        {
            if (&rField == m_xLeftMF.get())
            {
                nLeft = aOrigSize.Width() -
                            ( aPageSize.Width() * 100 / nWidthZoom + nRight );
                m_xLeftMF->set_value( m_xLeftMF->normalize( nLeft ), eUnit );
            }
            else
            {
                nRight = aOrigSize.Width() -
                            ( aPageSize.Width() * 100 / nWidthZoom + nLeft );
                m_xRightMF->set_value( m_xRightMF->normalize( nRight ), eUnit );
            }
        }
        if (AllSettings::GetLayoutRTL())
        {
            m_aExampleWN.SetLeft(nRight);
            m_aExampleWN.SetRight(nLeft);
        }
        else
        {
            m_aExampleWN.SetLeft(nLeft);
            m_aExampleWN.SetRight(nRight);
        }
        if(bZoom)
        {
            // scale stays, recompute width
            ZoomHdl(*m_xWidthZoomMF);
        }
    }
    else
    {
        long nTop = lcl_GetValue( *m_xTopMF, eUnit );
        long nBottom = lcl_GetValue( *m_xBottomMF, eUnit );
        long nHeightZoom = static_cast<long>(m_xHeightZoomMF->get_value(FieldUnit::NONE));
        if(bZoom && ( ( ( aOrigSize.Height() - (nTop + nBottom )) * nHeightZoom)
                                            / 100 >= aPageSize.Height()))
        {
            assert(nHeightZoom && "div-by-zero");
            if(&rField == m_xTopMF.get())
            {
                nTop = aOrigSize.Height() -
                            ( aPageSize.Height() * 100 / nHeightZoom + nBottom);
                m_xTopMF->set_value( m_xWidthMF->normalize( nTop ), eUnit );
            }
            else
            {
                nBottom = aOrigSize.Height() -
                            ( aPageSize.Height() * 100 / nHeightZoom + nTop);
                m_xBottomMF->set_value( m_xWidthMF->normalize( nBottom ), eUnit );
            }
        }
        m_aExampleWN.SetTop( nTop );
        m_aExampleWN.SetBottom( nBottom );
        if(bZoom)
        {
            // scale stays, recompute height
            ZoomHdl(*m_xHeightZoomMF);
        }
    }
    m_aExampleWN.Invalidate();
    // size and border changed -> recompute scale
    if(!bZoom)
        CalcZoom();
    CalcMinMaxBorder();
}
/*--------------------------------------------------------------------
    description: set original size
 --------------------------------------------------------------------*/

IMPL_LINK_NOARG(SvxGrfCropPage, OrigSizeHdl, weld::Button&, void)
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    long nWidth = aOrigSize.Width() -
        lcl_GetValue( *m_xLeftMF, eUnit ) -
        lcl_GetValue( *m_xRightMF, eUnit );
    m_xWidthMF->set_value( m_xWidthMF->normalize( nWidth ), eUnit );
    long nHeight = aOrigSize.Height() -
        lcl_GetValue( *m_xTopMF, eUnit ) -
        lcl_GetValue( *m_xBottomMF, eUnit );
    m_xHeightMF->set_value( m_xHeightMF->normalize( nHeight ), eUnit );
    m_xWidthZoomMF->set_value(100, FieldUnit::NONE);
    m_xHeightZoomMF->set_value(100, FieldUnit::NONE);
    bSetOrigSize = true;
}
/*--------------------------------------------------------------------
    description: compute scale
 --------------------------------------------------------------------*/

void SvxGrfCropPage::CalcZoom()
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    long nWidth = lcl_GetValue( *m_xWidthMF, eUnit );
    long nHeight = lcl_GetValue( *m_xHeightMF, eUnit );
    long nLRBorders = lcl_GetValue( *m_xLeftMF, eUnit ) +
                      lcl_GetValue( *m_xRightMF, eUnit );
    long nULBorders = lcl_GetValue( *m_xTopMF, eUnit ) +
                      lcl_GetValue( *m_xBottomMF, eUnit );
    sal_uInt16 nZoom = 0;
    long nDen;
    if( (nDen = aOrigSize.Width() - nLRBorders) > 0)
        nZoom = static_cast<sal_uInt16>((( nWidth  * 1000 / nDen )+5)/10);
    m_xWidthZoomMF->set_value(nZoom, FieldUnit::NONE);
    if( (nDen = aOrigSize.Height() - nULBorders) > 0)
        nZoom = static_cast<sal_uInt16>((( nHeight * 1000 / nDen )+5)/10);
    else
        nZoom = 0;
    m_xHeightZoomMF->set_value(nZoom, FieldUnit::NONE);
}

/*--------------------------------------------------------------------
    description: set minimum/maximum values for the margins
 --------------------------------------------------------------------*/

void SvxGrfCropPage::CalcMinMaxBorder()
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );
    long nR = lcl_GetValue(*m_xRightMF, eUnit );
    long nMinWidth = (aOrigSize.Width() * 10) /11;
    long nMin = nMinWidth - (nR >= 0 ? nR : 0);
    m_xLeftMF->set_max( m_xLeftMF->normalize(nMin), eUnit );

    long nL = lcl_GetValue(*m_xLeftMF, eUnit );
    nMin = nMinWidth - (nL >= 0 ? nL : 0);
    m_xRightMF->set_max( m_xRightMF->normalize(nMin), eUnit );

    long nUp  = lcl_GetValue( *m_xTopMF, eUnit );
    long nMinHeight = (aOrigSize.Height() * 10) /11;
    nMin = nMinHeight - (nUp >= 0 ? nUp : 0);
    m_xBottomMF->set_max( m_xBottomMF->normalize(nMin), eUnit );

    long nLow = lcl_GetValue(*m_xBottomMF, eUnit );
    nMin = nMinHeight - (nLow >= 0 ? nLow : 0);
    m_xTopMF->set_max( m_xTopMF->normalize(nMin), eUnit );
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
        DBG_ASSERT( pPool, "Where is the pool?" );
        FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ));

        sal_Int64 nSpin = m_xLeftMF->normalize(aOrigSize.Width()) / 20;
        nSpin = MetricField::ConvertValue( nSpin, aOrigSize.Width(), 0,
                                               eUnit, m_xLeftMF->get_unit());

        // if the margin is too big, it is set to 1/3 on both pages
        long nR = lcl_GetValue( *m_xRightMF, eUnit );
        long nL = lcl_GetValue( *m_xLeftMF, eUnit );
        if((nL + nR) < - aOrigSize.Width())
        {
            long nVal = aOrigSize.Width() / -3;
            m_xRightMF->set_value( m_xRightMF->normalize( nVal ), eUnit );
            m_xLeftMF->set_value( m_xLeftMF->normalize( nVal ), eUnit );
            m_aExampleWN.SetLeft(nVal);
            m_aExampleWN.SetRight(nVal);
        }
        long nUp  = lcl_GetValue(*m_xTopMF, eUnit );
        long nLow = lcl_GetValue(*m_xBottomMF, eUnit );
        if((nUp + nLow) < - aOrigSize.Height())
        {
            long nVal = aOrigSize.Height() / -3;
            m_xTopMF->set_value( m_xTopMF->normalize( nVal ), eUnit );
            m_xBottomMF->set_value( m_xBottomMF->normalize( nVal ), eUnit );
            m_aExampleWN.SetTop(nVal);
            m_aExampleWN.SetBottom(nVal);
        }

        m_xLeftMF->set_increments(nSpin, nSpin * 10, FieldUnit::NONE);
        m_xRightMF->set_increments(nSpin, nSpin * 10, FieldUnit::NONE);
        nSpin = m_xTopMF->normalize(aOrigSize.Height()) / 20;
        nSpin = MetricField::ConvertValue( nSpin, aOrigSize.Width(), 0,
                                               eUnit, m_xLeftMF->get_unit() );
        m_xTopMF->set_increments(nSpin, nSpin * 10, FieldUnit::NONE);
        m_xBottomMF->set_increments(nSpin, nSpin * 10, FieldUnit::NONE);

        // display original size
        const FieldUnit eMetric = GetModuleFieldUnit( GetItemSet() );

        ScopedVclPtrInstance< MetricField > aFld(this, WB_HIDE);
        SetFieldUnit( *aFld, eMetric );
        aFld->SetDecimalDigits(m_xWidthMF->get_digits());
        aFld->SetMax( LONG_MAX - 1 );

        aFld->SetValue( aFld->Normalize( aOrigSize.Width() ), eUnit );
        OUString sTemp = aFld->GetText();
        aFld->SetValue( aFld->Normalize( aOrigSize.Height() ), eUnit );
        // multiplication sign (U+00D7)
        sTemp += OUStringLiteral1(0x00D7) + aFld->GetText();

        if ( aOrigPixelSize.Width() && aOrigPixelSize.Height() ) {
             sal_Int32 ax = sal_Int32(floor(static_cast<float>(aOrigPixelSize.Width()) /
                        (static_cast<float>(aOrigSize.Width())/TWIP_TO_INCH)+0.5));
             sal_Int32 ay = sal_Int32(floor(static_cast<float>(aOrigPixelSize.Height()) /
                        (static_cast<float>(aOrigSize.Height())/TWIP_TO_INCH)+0.5));
             sTemp += " ";
             sTemp += CuiResId( RID_SVXSTR_PPI );
             OUString sPPI = OUString::number(ax);
             if (abs(ax - ay) > 1) {
                sPPI += OUStringLiteral1(0x00D7) + OUString::number(ay);
             }
             sTemp = sTemp.replaceAll("%1", sPPI);
        }
        m_xOrigSizeFT->set_label(sTemp);
    }

    m_xCropFrame->set_sensitive(bFound);
    m_xScaleFrame->set_sensitive(bFound);
    m_xSizeFrame->set_sensitive(bFound);
    m_xOrigSizeGrid->set_sensitive(bFound);
    m_xZoomConstRB->set_sensitive(bFound);
}

Size SvxGrfCropPage::GetGrfOrigSize( const Graphic& rGrf ) const
{
    const MapMode aMapTwip( MapUnit::MapTwip );
    Size aSize( rGrf.GetPrefSize() );
    if( MapUnit::MapPixel == rGrf.GetPrefMapMode().GetMapUnit() )
        aSize = PixelToLogic( aSize, aMapTwip );
    else
        aSize = OutputDevice::LogicToLogic( aSize,
                                        rGrf.GetPrefMapMode(), aMapTwip );
    return aSize;
}

/*****************************************************************/

SvxCropExample::SvxCropExample()
    : m_aTopLeft(0, 0)
    , m_aBottomRight(0, 0)
{
}

void SvxCropExample::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    OutputDevice& rDevice = pDrawingArea->get_ref_device();
    Size aSize(rDevice.LogicToPixel(Size(78, 78), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());

    m_aMapMode = rDevice.GetMapMode();
    m_aFrameSize = OutputDevice::LogicToLogic(
                            Size(CM_1_TO_TWIP / 2, CM_1_TO_TWIP / 2),
                            MapMode(MapUnit::MapTwip), m_aMapMode);
}

void SvxCropExample::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    rRenderContext.Push(PushFlags::MAPMODE | PushFlags::RASTEROP);
    rRenderContext.SetMapMode(m_aMapMode);

    Size aWinSize(rRenderContext.PixelToLogic(GetOutputSizePixel()));
    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor());
    rRenderContext.DrawRect(::tools::Rectangle(Point(), aWinSize));

    rRenderContext.SetLineColor(COL_WHITE);
    ::tools::Rectangle aRect(Point((aWinSize.Width() - m_aFrameSize.Width())/2,
                          (aWinSize.Height() - m_aFrameSize.Height())/2),
                          m_aFrameSize);
    m_aGrf.Draw(&rRenderContext, aRect.TopLeft(), aRect.GetSize());

    rRenderContext.SetFillColor(COL_TRANSPARENT);
    rRenderContext.SetRasterOp(RasterOp::Invert);
    aRect.AdjustLeft(m_aTopLeft.Y() );
    aRect.AdjustTop(m_aTopLeft.X() );
    aRect.AdjustRight(-m_aBottomRight.Y());
    aRect.AdjustBottom(-m_aBottomRight.X());
    rRenderContext.DrawRect(aRect);

    rRenderContext.Pop();
}

void SvxCropExample::Resize()
{
    SetFrameSize(m_aFrameSize);
}

void SvxCropExample::SetFrameSize( const Size& rSz )
{
    m_aFrameSize = rSz;
    if (!m_aFrameSize.Width())
        m_aFrameSize.setWidth( 1 );
    if (!m_aFrameSize.Height())
        m_aFrameSize.setHeight( 1 );
    Size aWinSize( GetOutputSizePixel() );
    Fraction aXScale( aWinSize.Width() * 4, m_aFrameSize.Width() * 5 );
    Fraction aYScale( aWinSize.Height() * 4, m_aFrameSize.Height() * 5 );

    if( aYScale < aXScale )
        aXScale = aYScale;

    m_aMapMode.SetScaleX(aXScale);
    m_aMapMode.SetScaleY(aXScale);

    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
