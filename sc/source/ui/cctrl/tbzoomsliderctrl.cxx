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
#include <tbzoomsliderctrl.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>
#include <svx/zoomslideritem.hxx>
#include <iterator>
#include <set>
#include <bitmaps.hlst>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

// class ScZoomSliderControl ---------------------------------------

SFX_IMPL_TOOLBOX_CONTROL( ScZoomSliderControl, SvxZoomSliderItem );

ScZoomSliderControl::ScZoomSliderControl(
    sal_uInt16     nSlotId,
    ToolBoxItemId  nId,
    ToolBox&   rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.Invalidate();
}

ScZoomSliderControl::~ScZoomSliderControl()
{

}

void ScZoomSliderControl::StateChanged( sal_uInt16 /*nSID*/, SfxItemState eState,
                                       const SfxPoolItem* pState )
{
    ToolBoxItemId           nId  = GetId();
    ToolBox&                rTbx = GetToolBox();
    ScZoomSliderWnd*        pBox = static_cast<ScZoomSliderWnd*>(rTbx.GetItemWindow( nId ));
    OSL_ENSURE( pBox ,"Control not found!" );

    if ( SfxItemState::DEFAULT != eState || pState->IsVoidItem() )
    {
        SvxZoomSliderItem aZoomSliderItem( 100 );
        pBox->Disable();
        pBox->UpdateFromItem( &aZoomSliderItem );
    }
    else
    {
        pBox->Enable();
        OSL_ENSURE( dynamic_cast<const SvxZoomSliderItem*>( pState) !=  nullptr, "invalid item type" );
        const SvxZoomSliderItem* pZoomSliderItem = dynamic_cast< const SvxZoomSliderItem* >( pState );

        OSL_ENSURE( pZoomSliderItem, "Sc::ScZoomSliderControl::StateChanged(), wrong item type!" );
        if( pZoomSliderItem )
            pBox->UpdateFromItem( pZoomSliderItem );
    }
}

VclPtr<InterimItemWindow> ScZoomSliderControl::CreateItemWindow( vcl::Window *pParent )
{
    // #i98000# Don't try to get a value via SfxViewFrame::Current here.
    // The view's value is always notified via StateChanged later.
    VclPtrInstance<ScZoomSliderWnd> xSlider( pParent,
        css::uno::Reference< css::frame::XDispatchProvider >( m_xFrame->getController(),
        css::uno::UNO_QUERY ), 100 );
    return xSlider;
}

struct ScZoomSlider::ScZoomSliderWnd_Impl
{
    sal_uInt16                   mnCurrentZoom;
    sal_uInt16                   mnMinZoom;
    sal_uInt16                   mnMaxZoom;
    std::vector< tools::Long >      maSnappingPointOffsets;
    std::vector< sal_uInt16 >    maSnappingPointZooms;
    Image                    maSliderButton;
    Image                    maIncreaseButton;
    Image                    maDecreaseButton;
    bool                     mbOmitPaint;
    VclPtr<vcl::Window>      mxParentWindow;

    explicit ScZoomSliderWnd_Impl( sal_uInt16 nCurrentZoom, vcl::Window* parentWindow ) :
        mnCurrentZoom( nCurrentZoom ),
        mnMinZoom( 10 ),
        mnMaxZoom( 400 ),
        maSnappingPointOffsets(),
        maSnappingPointZooms(),
        maSliderButton(),
        maIncreaseButton(),
        maDecreaseButton(),
        mbOmitPaint( false ),
        mxParentWindow(parentWindow)
        {
        }
};

constexpr sal_uInt16 gnSliderCenter(100);

const tools::Long nButtonWidth     = 10;
const tools::Long nButtonHeight    = 10;
const tools::Long nIncDecWidth     = 11;
const tools::Long nIncDecHeight    = 11;
const tools::Long nSliderHeight    = 2;
const tools::Long nSliderWidth     = 4;
const tools::Long nSnappingHeight  = 4;
const tools::Long nSliderXOffset   = 20;
const tools::Long nSnappingEpsilon = 5; // snapping epsilon in pixels
const tools::Long nSnappingPointsMinDist = nSnappingEpsilon; // minimum distance of two adjacent snapping points

sal_uInt16 ScZoomSlider::Offset2Zoom( tools::Long nOffset ) const
{
    Size aSliderWindowSize = GetOutputSizePixel();
    const tools::Long nControlWidth = aSliderWindowSize.Width();
    sal_uInt16 nRet = 0;

    if( nOffset < nSliderXOffset )
        return mpImpl->mnMinZoom;
    if( nOffset > nControlWidth - nSliderXOffset )
        return mpImpl->mnMaxZoom;

    // check for snapping points:
    auto aSnappingPointIter = std::find_if(mpImpl->maSnappingPointOffsets.begin(), mpImpl->maSnappingPointOffsets.end(),
        [nOffset](const tools::Long nCurrent) { return std::abs(nCurrent - nOffset) < nSnappingEpsilon; });
    if (aSnappingPointIter != mpImpl->maSnappingPointOffsets.end())
    {
        nOffset = *aSnappingPointIter;
        auto nCount = static_cast<sal_uInt16>(std::distance(mpImpl->maSnappingPointOffsets.begin(), aSnappingPointIter));
        nRet = mpImpl->maSnappingPointZooms[ nCount ];
    }

    if( 0 == nRet )
    {
        if( nOffset < nControlWidth / 2 )
        {
            // first half of slider
            const tools::Long nFirstHalfRange      = gnSliderCenter - mpImpl->mnMinZoom;
            const tools::Long nHalfSliderWidth     = nControlWidth/2 - nSliderXOffset;
            const tools::Long nZoomPerSliderPixel  = (1000 * nFirstHalfRange) / nHalfSliderWidth;
            const tools::Long nOffsetToSliderLeft  = nOffset - nSliderXOffset;
            nRet = mpImpl->mnMinZoom + sal_uInt16( nOffsetToSliderLeft * nZoomPerSliderPixel / 1000 );
        }
        else
        {
            // second half of slider
            const tools::Long nSecondHalfRange         = mpImpl->mnMaxZoom - gnSliderCenter;
            const tools::Long nHalfSliderWidth         = nControlWidth/2 - nSliderXOffset;
            const tools::Long nZoomPerSliderPixel      = 1000 * nSecondHalfRange / nHalfSliderWidth;
            const tools::Long nOffsetToSliderCenter    = nOffset - nControlWidth/2;
            nRet = gnSliderCenter + sal_uInt16( nOffsetToSliderCenter * nZoomPerSliderPixel / 1000 );
        }
    }

    if( nRet < mpImpl->mnMinZoom )
        return mpImpl->mnMinZoom;

    else if( nRet > mpImpl->mnMaxZoom )
        return mpImpl->mnMaxZoom;

    return nRet;
}

tools::Long ScZoomSlider::Zoom2Offset( sal_uInt16 nCurrentZoom ) const
{
    Size aSliderWindowSize = GetOutputSizePixel();
    const tools::Long nControlWidth = aSliderWindowSize.Width();
    tools::Long  nRect = nSliderXOffset;

    const tools::Long nHalfSliderWidth = nControlWidth/2 - nSliderXOffset;
    if( nCurrentZoom <= gnSliderCenter )
    {
        nCurrentZoom = nCurrentZoom - mpImpl->mnMinZoom;
        const tools::Long nFirstHalfRange = gnSliderCenter - mpImpl->mnMinZoom;
        const tools::Long nSliderPixelPerZoomPercent = 1000 * nHalfSliderWidth  / nFirstHalfRange;
        const tools::Long nOffset = (nSliderPixelPerZoomPercent * nCurrentZoom) / 1000;
        nRect += nOffset;
    }
    else
    {
        nCurrentZoom = nCurrentZoom - gnSliderCenter;
        const tools::Long nSecondHalfRange = mpImpl->mnMaxZoom - gnSliderCenter;
        const tools::Long nSliderPixelPerZoomPercent = 1000 * nHalfSliderWidth  / nSecondHalfRange;
        const tools::Long nOffset = (nSliderPixelPerZoomPercent * nCurrentZoom) / 1000;
        nRect += nHalfSliderWidth + nOffset;
    }
    return nRect;
}

ScZoomSliderWnd::ScZoomSliderWnd( vcl::Window* pParent,
                const css::uno::Reference< css::frame::XDispatchProvider >& rDispatchProvider,
                sal_uInt16 nCurrentZoom ):
                InterimItemWindow(pParent, "modules/scalc/ui/zoombox.ui", "ZoomBox"),
                mxWidget(new ScZoomSlider(rDispatchProvider, nCurrentZoom, pParent)),
                mxWeld(new weld::CustomWeld(*m_xBuilder, "zoom", *mxWidget))
{
    Size aLogicalSize( 115, 40 );
    Size aSliderSize = LogicToPixel(aLogicalSize, MapMode(MapUnit::Map10thMM));
    Size aPreferredSize(aSliderSize.Width() * nSliderWidth-1, aSliderSize.Height() + nSliderHeight);
    mxWidget->GetDrawingArea()->set_size_request(aPreferredSize.Width(), aPreferredSize.Height());
    mxWidget->SetOutputSizePixel(aPreferredSize);
    SetSizePixel(aPreferredSize);
}

ScZoomSliderWnd::~ScZoomSliderWnd()
{
    disposeOnce();
}

void ScZoomSliderWnd::dispose()
{
    mxWeld.reset();
    mxWidget.reset();
    InterimItemWindow::dispose();
}

ScZoomSlider::ScZoomSlider(const css::uno::Reference< css::frame::XDispatchProvider>& rDispatchProvider,
                           sal_uInt16 nCurrentZoom, vcl::Window* parentWindow)
    : mpImpl(new ScZoomSliderWnd_Impl(nCurrentZoom, parentWindow))
    , m_xDispatchProvider(rDispatchProvider)
{
    mpImpl->maSliderButton      = Image(StockImage::Yes, RID_SVXBMP_SLIDERBUTTON);
    mpImpl->maIncreaseButton    = Image(StockImage::Yes, RID_SVXBMP_SLIDERINCREASE);
    mpImpl->maDecreaseButton    = Image(StockImage::Yes, RID_SVXBMP_SLIDERDECREASE);
}

bool ScZoomSlider::MouseButtonDown( const MouseEvent& rMEvt )
{
    Size aSliderWindowSize = GetOutputSizePixel();

    const Point aPoint = rMEvt.GetPosPixel();

    const tools::Long nButtonLeftOffset    = ( nSliderXOffset - nIncDecWidth )/2;
    const tools::Long nButtonRightOffset   = ( nSliderXOffset + nIncDecWidth )/2;

    const tools::Long nOldZoom = mpImpl->mnCurrentZoom;

    // click to - button
    if ( aPoint.X() >= nButtonLeftOffset && aPoint.X() <= nButtonRightOffset )
    {
        mpImpl->mnCurrentZoom = mpImpl->mnCurrentZoom - 5;
    }
    // click to + button
    else if ( aPoint.X() >= aSliderWindowSize.Width() - nSliderXOffset + nButtonLeftOffset &&
              aPoint.X() <= aSliderWindowSize.Width() - nSliderXOffset + nButtonRightOffset )
    {
        mpImpl->mnCurrentZoom = mpImpl->mnCurrentZoom + 5;
    }
    else if( aPoint.X() >= nSliderXOffset && aPoint.X() <= aSliderWindowSize.Width() - nSliderXOffset )
    {
        mpImpl->mnCurrentZoom = Offset2Zoom( aPoint.X() );
    }

    if( mpImpl->mnCurrentZoom < mpImpl->mnMinZoom )
        mpImpl->mnCurrentZoom = mpImpl->mnMinZoom;
    else if( mpImpl->mnCurrentZoom > mpImpl->mnMaxZoom )
        mpImpl->mnCurrentZoom = mpImpl->mnMaxZoom;

    if( nOldZoom == mpImpl->mnCurrentZoom )
        return true;

    // need to invalidate parent since we rely on the toolbox drawing it's fancy gradient background
    mpImpl->mxParentWindow->Invalidate();
    mpImpl->mbOmitPaint = true;

    SvxZoomSliderItem   aZoomSliderItem( mpImpl->mnCurrentZoom );

    css::uno::Any  a;
    aZoomSliderItem.QueryValue( a );

    css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = "ScalingFactor";
    aArgs[0].Value = a;

    SfxToolBoxControl::Dispatch( m_xDispatchProvider, ".uno:ScalingFactor", aArgs );

    mpImpl->mbOmitPaint = false;

    return true;
}

bool ScZoomSlider::MouseMove( const MouseEvent& rMEvt )
{
    Size aSliderWindowSize   = GetOutputSizePixel();
    const tools::Long nControlWidth = aSliderWindowSize.Width();
    const short nButtons     = rMEvt.GetButtons();

    // check mouse move with button pressed
    if ( 1 == nButtons )
    {
        const Point aPoint = rMEvt.GetPosPixel();

        if ( aPoint.X() >= nSliderXOffset && aPoint.X() <= nControlWidth - nSliderXOffset )
        {
            mpImpl->mnCurrentZoom = Offset2Zoom( aPoint.X() );

            // need to invalidate parent since we rely on the toolbox drawing it's fancy gradient background
            mpImpl->mxParentWindow->Invalidate();

            mpImpl->mbOmitPaint = true; // optimization: paint before executing command,

            // commit state change
            SvxZoomSliderItem aZoomSliderItem( mpImpl->mnCurrentZoom );

            css::uno::Any a;
            aZoomSliderItem.QueryValue( a );

            css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name = "ScalingFactor";
            aArgs[0].Value = a;

            SfxToolBoxControl::Dispatch( m_xDispatchProvider, ".uno:ScalingFactor", aArgs );

            mpImpl->mbOmitPaint = false;
        }
    }

    return false;
}

void ScZoomSliderWnd::UpdateFromItem( const SvxZoomSliderItem* pZoomSliderItem )
{
    mxWidget->UpdateFromItem(pZoomSliderItem);
}

void ScZoomSlider::UpdateFromItem(const SvxZoomSliderItem* pZoomSliderItem)
{
    if( pZoomSliderItem )
    {
        mpImpl->mnCurrentZoom = pZoomSliderItem->GetValue();
        mpImpl->mnMinZoom     = pZoomSliderItem->GetMinZoom();
        mpImpl->mnMaxZoom     = pZoomSliderItem->GetMaxZoom();

        OSL_ENSURE( mpImpl->mnMinZoom <= mpImpl->mnCurrentZoom &&
            mpImpl->mnMinZoom <  gnSliderCenter &&
            mpImpl->mnMaxZoom >= mpImpl->mnCurrentZoom &&
            mpImpl->mnMaxZoom > gnSliderCenter,
            "Looks like the zoom slider item is corrupted" );
        const css::uno::Sequence < sal_Int32 >& rSnappingPoints = pZoomSliderItem->GetSnappingPoints();
        mpImpl->maSnappingPointOffsets.clear();
        mpImpl->maSnappingPointZooms.clear();

        // get all snapping points:
        std::set< sal_uInt16 > aTmpSnappingPoints;
        std::transform(rSnappingPoints.begin(), rSnappingPoints.end(), std::inserter(aTmpSnappingPoints, aTmpSnappingPoints.end()),
            [](const sal_Int32 nSnappingPoint) -> sal_uInt16 { return static_cast<sal_uInt16>(nSnappingPoint); });

        // remove snapping points that are too close to each other:
        tools::Long nLastOffset = 0;

        for ( const sal_uInt16 nCurrent : aTmpSnappingPoints )
        {
            const tools::Long nCurrentOffset = Zoom2Offset( nCurrent );

            if ( nCurrentOffset - nLastOffset >= nSnappingPointsMinDist )
            {
                mpImpl->maSnappingPointOffsets.push_back( nCurrentOffset );
                mpImpl->maSnappingPointZooms.push_back( nCurrent );
                nLastOffset = nCurrentOffset;
            }
        }
    }

    if ( !mpImpl->mbOmitPaint )
        // need to invalidate parent since we rely on the toolbox drawing it's fancy gradient background
        mpImpl->mxParentWindow->Invalidate();
}

void ScZoomSlider::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    DoPaint(rRenderContext);
}

void ScZoomSlider::DoPaint(vcl::RenderContext& rRenderContext)
{
    if (mpImpl->mbOmitPaint)
        return;

    Size aSliderWindowSize(GetOutputSizePixel());
    tools::Rectangle aRect(Point(0, 0), aSliderWindowSize);

    ScopedVclPtrInstance< VirtualDevice > pVDev(rRenderContext);
    pVDev->SetOutputSizePixel(aSliderWindowSize);
    pVDev->SetFillColor( COL_TRANSPARENT );
    pVDev->SetLineColor( COL_TRANSPARENT );
    pVDev->DrawRect( aRect );

    tools::Rectangle aSlider = aRect;

    aSlider.AdjustTop((aSliderWindowSize.Height() - nSliderHeight) / 2 - 1 );
    aSlider.SetBottom( aSlider.Top() + nSliderHeight );
    aSlider.AdjustLeft(nSliderXOffset );
    aSlider.AdjustRight( -nSliderXOffset );

    tools::Rectangle aFirstLine(aSlider);
    aFirstLine.SetBottom( aFirstLine.Top() );

    tools::Rectangle aSecondLine(aSlider);
    aSecondLine.SetTop( aSecondLine.Bottom() );

    tools::Rectangle aLeft(aSlider);
    aLeft.SetRight( aLeft.Left() );

    tools::Rectangle aRight(aSlider);
    aRight.SetLeft( aRight.Right() );

    // draw slider
    pVDev->SetLineColor(COL_WHITE);
    pVDev->DrawRect(aSecondLine);
    pVDev->DrawRect(aRight);

    pVDev->SetLineColor(COL_GRAY);
    pVDev->DrawRect(aFirstLine);
    pVDev->DrawRect(aLeft);

    // draw snapping points:
    for (const auto& rSnappingPointOffset : mpImpl->maSnappingPointOffsets)
    {
        pVDev->SetLineColor(COL_GRAY);
        tools::Rectangle aSnapping(aRect);
        aSnapping.SetBottom( aSlider.Top() );
        aSnapping.SetTop( aSnapping.Bottom() - nSnappingHeight );
        aSnapping.AdjustLeft(rSnappingPointOffset );
        aSnapping.SetRight( aSnapping.Left() );
        pVDev->DrawRect(aSnapping);

        aSnapping.AdjustTop(nSnappingHeight + nSliderHeight );
        aSnapping.AdjustBottom(nSnappingHeight + nSliderHeight );
        pVDev->DrawRect(aSnapping);
    }

    // draw slider button
    Point aImagePoint = aRect.TopLeft();
    aImagePoint.AdjustX(Zoom2Offset(mpImpl->mnCurrentZoom) );
    aImagePoint.AdjustX( -(nButtonWidth / 2) );
    aImagePoint.AdjustY( (aSliderWindowSize.Height() - nButtonHeight) / 2 );
    pVDev->DrawImage(aImagePoint, mpImpl->maSliderButton);

    // draw decrease button
    aImagePoint = aRect.TopLeft();
    aImagePoint.AdjustX((nSliderXOffset - nIncDecWidth) / 2 );
    aImagePoint.AdjustY((aSliderWindowSize.Height() - nIncDecHeight) / 2 );
    pVDev->DrawImage(aImagePoint, mpImpl->maDecreaseButton);

    // draw increase button
    aImagePoint.setX( aRect.Left() + aSliderWindowSize.Width() - nIncDecWidth - (nSliderXOffset - nIncDecWidth) / 2 );
    pVDev->DrawImage(aImagePoint, mpImpl->maIncreaseButton);

    rRenderContext.DrawOutDev(Point(0, 0), aSliderWindowSize, Point(0, 0), aSliderWindowSize, *pVDev);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
