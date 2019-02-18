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
#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gradient.hxx>
#include <vcl/settings.hxx>
#include <svx/zoomslideritem.hxx>
#include <set>
#include <bitmaps.hlst>

// class ScZoomSliderControl ---------------------------------------

SFX_IMPL_TOOLBOX_CONTROL( ScZoomSliderControl, SvxZoomSliderItem );

ScZoomSliderControl::ScZoomSliderControl(
    sal_uInt16     nSlotId,
    sal_uInt16     nId,
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
    sal_uInt16                  nId  = GetId();
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

VclPtr<vcl::Window> ScZoomSliderControl::CreateItemWindow( vcl::Window *pParent )
{
    // #i98000# Don't try to get a value via SfxViewFrame::Current here.
    // The view's value is always notified via StateChanged later.
    VclPtrInstance<ScZoomSliderWnd> pSlider( pParent,
        css::uno::Reference< css::frame::XDispatchProvider >( m_xFrame->getController(),
        css::uno::UNO_QUERY ), 100 );
    return pSlider.get();
}

struct ScZoomSliderWnd::ScZoomSliderWnd_Impl
{
    sal_uInt16                   mnCurrentZoom;
    sal_uInt16                   mnMinZoom;
    sal_uInt16                   mnMaxZoom;
    std::vector< long >      maSnappingPointOffsets;
    std::vector< sal_uInt16 >    maSnappingPointZooms;
    Image                    maSliderButton;
    Image                    maIncreaseButton;
    Image                    maDecreaseButton;
    bool                     mbOmitPaint;

    explicit ScZoomSliderWnd_Impl( sal_uInt16 nCurrentZoom ) :
        mnCurrentZoom( nCurrentZoom ),
        mnMinZoom( 10 ),
        mnMaxZoom( 400 ),
        maSnappingPointOffsets(),
        maSnappingPointZooms(),
        maSliderButton(),
        maIncreaseButton(),
        maDecreaseButton(),
        mbOmitPaint( false )
        {
        }
};

static constexpr sal_uInt16 gnSliderCenter(100);

const long nButtonWidth     = 10;
const long nButtonHeight    = 10;
const long nIncDecWidth     = 11;
const long nIncDecHeight    = 11;
const long nSliderHeight    = 2;
const long nSliderWidth     = 4;
const long nSnappingHeight  = 4;
const long nSliderXOffset   = 20;
const long nSnappingEpsilon = 5; // snapping epsilon in pixels
const long nSnappingPointsMinDist = nSnappingEpsilon; // minimum distance of two adjacent snapping points

sal_uInt16 ScZoomSliderWnd::Offset2Zoom( long nOffset ) const
{
    Size aSliderWindowSize = GetOutputSizePixel();
    const long nControlWidth = aSliderWindowSize.Width();
    sal_uInt16 nRet = 0;

    if( nOffset < nSliderXOffset )
        return mpImpl->mnMinZoom;
    if( nOffset > nControlWidth - nSliderXOffset )
        return mpImpl->mnMaxZoom;

    // check for snapping points:
    auto aSnappingPointIter = std::find_if(mpImpl->maSnappingPointOffsets.begin(), mpImpl->maSnappingPointOffsets.end(),
        [nOffset](const long nCurrent) { return std::abs(nCurrent - nOffset) < nSnappingEpsilon; });
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
            const long nFirstHalfRange      = gnSliderCenter - mpImpl->mnMinZoom;
            const long nHalfSliderWidth     = nControlWidth/2 - nSliderXOffset;
            const long nZoomPerSliderPixel  = (1000 * nFirstHalfRange) / nHalfSliderWidth;
            const long nOffsetToSliderLeft  = nOffset - nSliderXOffset;
            nRet = mpImpl->mnMinZoom + sal_uInt16( nOffsetToSliderLeft * nZoomPerSliderPixel / 1000 );
        }
        else
        {
            // second half of slider
            const long nSecondHalfRange         = mpImpl->mnMaxZoom - gnSliderCenter;
            const long nHalfSliderWidth         = nControlWidth/2 - nSliderXOffset;
            const long nZoomPerSliderPixel      = 1000 * nSecondHalfRange / nHalfSliderWidth;
            const long nOffsetToSliderCenter    = nOffset - nControlWidth/2;
            nRet = gnSliderCenter + sal_uInt16( nOffsetToSliderCenter * nZoomPerSliderPixel / 1000 );
        }
    }

    if( nRet < mpImpl->mnMinZoom )
        return mpImpl->mnMinZoom;

    else if( nRet > mpImpl->mnMaxZoom )
        return mpImpl->mnMaxZoom;

    return nRet;
}

long ScZoomSliderWnd::Zoom2Offset( sal_uInt16 nCurrentZoom ) const
{
    Size aSliderWindowSize = GetOutputSizePixel();
    const long nControlWidth = aSliderWindowSize.Width();
    long  nRect = nSliderXOffset;

    const long nHalfSliderWidth = nControlWidth/2 - nSliderXOffset;
    if( nCurrentZoom <= gnSliderCenter )
    {
        nCurrentZoom = nCurrentZoom - mpImpl->mnMinZoom;
        const long nFirstHalfRange = gnSliderCenter - mpImpl->mnMinZoom;
        const long nSliderPixelPerZoomPercent = 1000 * nHalfSliderWidth  / nFirstHalfRange;
        const long nOffset = (nSliderPixelPerZoomPercent * nCurrentZoom) / 1000;
        nRect += nOffset;
    }
    else
    {
        nCurrentZoom = nCurrentZoom - gnSliderCenter;
        const long nSecondHalfRange = mpImpl->mnMaxZoom - gnSliderCenter;
        const long nSliderPixelPerZoomPercent = 1000 * nHalfSliderWidth  / nSecondHalfRange;
        const long nOffset = (nSliderPixelPerZoomPercent * nCurrentZoom) / 1000;
        nRect += nHalfSliderWidth + nOffset;
    }
    return nRect;
}

ScZoomSliderWnd::ScZoomSliderWnd( vcl::Window* pParent,
                const css::uno::Reference< css::frame::XDispatchProvider >& rDispatchProvider,
                sal_uInt16 nCurrentZoom ):
                Window( pParent ),
                mpImpl( new ScZoomSliderWnd_Impl( nCurrentZoom ) ),
                aLogicalSize( 115, 40 ),
                m_xDispatchProvider( rDispatchProvider )
{
    mpImpl->maSliderButton      = Image(StockImage::Yes, RID_SVXBMP_SLIDERBUTTON);
    mpImpl->maIncreaseButton    = Image(StockImage::Yes, RID_SVXBMP_SLIDERINCREASE);
    mpImpl->maDecreaseButton    = Image(StockImage::Yes, RID_SVXBMP_SLIDERDECREASE);
    Size  aSliderSize           = LogicToPixel( aLogicalSize, MapMode( MapUnit::Map10thMM ) );
    SetSizePixel( Size( aSliderSize.Width() * nSliderWidth-1, aSliderSize.Height() + nSliderHeight ) );
}

ScZoomSliderWnd::~ScZoomSliderWnd()
{
    disposeOnce();
}

void ScZoomSliderWnd::dispose()
{
    mpImpl.reset();
    vcl::Window::dispose();
}

void ScZoomSliderWnd::MouseButtonDown( const MouseEvent& rMEvt )
{
    Size aSliderWindowSize = GetOutputSizePixel();

    const Point aPoint = rMEvt.GetPosPixel();

    const long nButtonLeftOffset    = ( nSliderXOffset - nIncDecWidth )/2;
    const long nButtonRightOffset   = ( nSliderXOffset + nIncDecWidth )/2;

    const long nOldZoom = mpImpl->mnCurrentZoom;

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
        return ;

    tools::Rectangle aRect( Point( 0, 0 ), aSliderWindowSize );

    Invalidate(aRect);
    mpImpl->mbOmitPaint = true;

    SvxZoomSliderItem   aZoomSliderItem( mpImpl->mnCurrentZoom );

    css::uno::Any  a;
    aZoomSliderItem.QueryValue( a );

    css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = "ScalingFactor";
    aArgs[0].Value = a;

    SfxToolBoxControl::Dispatch( m_xDispatchProvider, ".uno:ScalingFactor", aArgs );

    mpImpl->mbOmitPaint = false;
}

void ScZoomSliderWnd::MouseMove( const MouseEvent& rMEvt )
{
    Size aSliderWindowSize   = GetOutputSizePixel();
    const long nControlWidth = aSliderWindowSize.Width();
    const short nButtons     = rMEvt.GetButtons();

    // check mouse move with button pressed
    if ( 1 == nButtons )
    {
        const Point aPoint = rMEvt.GetPosPixel();

        if ( aPoint.X() >= nSliderXOffset && aPoint.X() <= nControlWidth - nSliderXOffset )
        {
            mpImpl->mnCurrentZoom = Offset2Zoom( aPoint.X() );

            tools::Rectangle aRect(Point(0, 0), aSliderWindowSize);
            Invalidate(aRect);

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
}

void ScZoomSliderWnd::UpdateFromItem( const SvxZoomSliderItem* pZoomSliderItem )
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
        for ( sal_Int32 j = 0; j < rSnappingPoints.getLength(); ++j )
        {
            const sal_Int32 nSnappingPoint = rSnappingPoints[j];
            aTmpSnappingPoints.insert( static_cast<sal_uInt16>(nSnappingPoint) );
        }

        // remove snapping points that are too close to each other:
        long nLastOffset = 0;

        for ( const sal_uInt16 nCurrent : aTmpSnappingPoints )
        {
            const long nCurrentOffset = Zoom2Offset( nCurrent );

            if ( nCurrentOffset - nLastOffset >= nSnappingPointsMinDist )
            {
                mpImpl->maSnappingPointOffsets.push_back( nCurrentOffset );
                mpImpl->maSnappingPointZooms.push_back( nCurrent );
                nLastOffset = nCurrentOffset;
            }
        }
    }

    Size aSliderWindowSize = GetOutputSizePixel();
    tools::Rectangle aRect(Point(0, 0), aSliderWindowSize);

    if ( !mpImpl->mbOmitPaint )
       Invalidate(aRect);
}

void ScZoomSliderWnd::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    DoPaint(rRenderContext);
}

void ScZoomSliderWnd::DoPaint(vcl::RenderContext& rRenderContext)
{
    if (mpImpl->mbOmitPaint)
        return;

    Size aSliderWindowSize(GetOutputSizePixel());
    tools::Rectangle aRect(Point(0, 0), aSliderWindowSize);

    ScopedVclPtrInstance< VirtualDevice > pVDev(rRenderContext);
    pVDev->SetOutputSizePixel(aSliderWindowSize);

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

    // draw VirtualDevice's background color
    Color aStartColor = rRenderContext.GetSettings().GetStyleSettings().GetFaceColor();
    Color aEndColor   = rRenderContext.GetSettings().GetStyleSettings().GetFaceColor();

    if (aEndColor.IsDark())
        aStartColor = aEndColor;

    Gradient aGradient;
    aGradient.SetAngle(0);
    aGradient.SetStyle(GradientStyle::Linear);

    aGradient.SetStartColor(aStartColor);
    aGradient.SetEndColor(aEndColor);
    pVDev->DrawGradient(aRect, aGradient);

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
    aImagePoint.setX( aRect.TopLeft().X() + aSliderWindowSize.Width() - nIncDecWidth - (nSliderXOffset - nIncDecWidth) / 2 );
    pVDev->DrawImage(aImagePoint, mpImpl->maIncreaseButton);

    rRenderContext.DrawOutDev(Point(0, 0), aSliderWindowSize, Point(0, 0), aSliderWindowSize, *pVDev);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
