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

#include <svx/zoomsliderctrl.hxx>
#include <vcl/status.hxx>
#include <vcl/menu.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <basegfx/utils/zoomtools.hxx>
#include <bitmaps.hlst>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <set>

SFX_IMPL_STATUSBAR_CONTROL( SvxZoomSliderControl, SvxZoomSliderItem );

struct SvxZoomSliderControl::SvxZoomSliderControl_Impl
{
    sal_uInt16                   mnCurrentZoom;
    sal_uInt16                   mnMinZoom;
    sal_uInt16                   mnMaxZoom;
    sal_uInt16                   mnSliderCenter;
    std::vector< long >      maSnappingPointOffsets;
    std::vector< sal_uInt16 >    maSnappingPointZooms;
    Image                    maSliderButton;
    Image                    maIncreaseButton;
    Image                    maDecreaseButton;
    bool                     mbValuesSet;
    bool                     mbDraggingStarted;

    SvxZoomSliderControl_Impl() :
        mnCurrentZoom( 0 ),
        mnMinZoom( 0 ),
        mnMaxZoom( 0 ),
        mnSliderCenter( 0 ),
        maSnappingPointOffsets(),
        maSnappingPointZooms(),
        maSliderButton(),
        maIncreaseButton(),
        maDecreaseButton(),
        mbValuesSet( false ),
        mbDraggingStarted( false ) {}
};

const long nSliderXOffset = 20;
const long nSnappingEpsilon = 5; // snapping epsilon in pixels
const long nSnappingPointsMinDist = nSnappingEpsilon; // minimum distance of two adjacent snapping points

// nOffset referes to the origin of the control:
// + ----------- -
sal_uInt16 SvxZoomSliderControl::Offset2Zoom( long nOffset ) const
{
    const long nControlWidth = getControlRect().GetWidth();
    sal_uInt16 nRet = 0;

    if ( nOffset < nSliderXOffset )
        return mxImpl->mnMinZoom;

    if ( nOffset > nControlWidth - nSliderXOffset )
        return mxImpl->mnMaxZoom;

    // check for snapping points:
    sal_uInt16 nCount = 0;
    for ( const long nCurrent : mxImpl->maSnappingPointOffsets )
    {
        if ( std::abs(nCurrent - nOffset) < nSnappingEpsilon )
        {
            nOffset = nCurrent;
            nRet = mxImpl->maSnappingPointZooms[ nCount ];
            break;
        }
        ++nCount;
    }

    if ( 0 == nRet )
    {
        if ( nOffset < nControlWidth / 2 )
        {
            // first half of slider
            const long nFirstHalfRange = mxImpl->mnSliderCenter - mxImpl->mnMinZoom;
            const long nHalfSliderWidth = nControlWidth/2 - nSliderXOffset;
            const long nZoomPerSliderPixel = (1000 * nFirstHalfRange) / nHalfSliderWidth;
            const long nOffsetToSliderLeft = nOffset - nSliderXOffset;
            nRet = mxImpl->mnMinZoom + sal_uInt16( nOffsetToSliderLeft * nZoomPerSliderPixel / 1000 );
        }
        else
        {
            // second half of slider
            const long nSecondHalfRange = mxImpl->mnMaxZoom - mxImpl->mnSliderCenter;
            const long nHalfSliderWidth = nControlWidth/2 - nSliderXOffset;
            const long nZoomPerSliderPixel = 1000 * nSecondHalfRange / nHalfSliderWidth;
            const long nOffsetToSliderCenter = nOffset - nControlWidth/2;
            nRet = mxImpl->mnSliderCenter + sal_uInt16( nOffsetToSliderCenter * nZoomPerSliderPixel / 1000 );
        }
    }

    if ( nRet < mxImpl->mnMinZoom )
        nRet = mxImpl->mnMinZoom;
    else if ( nRet > mxImpl->mnMaxZoom )
        nRet = mxImpl->mnMaxZoom;

    return nRet;
}

// returns the offset to the left control border
long SvxZoomSliderControl::Zoom2Offset( sal_uInt16 nCurrentZoom ) const
{
    const long nControlWidth = getControlRect().GetWidth();
    long nRet = nSliderXOffset;

    const long nHalfSliderWidth = nControlWidth/2 - nSliderXOffset;

    if ( nCurrentZoom <= mxImpl->mnSliderCenter )
    {
        nCurrentZoom = nCurrentZoom - mxImpl->mnMinZoom;
        const long nFirstHalfRange = mxImpl->mnSliderCenter - mxImpl->mnMinZoom;
        const long nSliderPixelPerZoomPercent = 1000 * nHalfSliderWidth  / nFirstHalfRange;
        const long nOffset = (nSliderPixelPerZoomPercent * nCurrentZoom) / 1000;
        nRet += nOffset;
    }
    else
    {
        nCurrentZoom = nCurrentZoom - mxImpl->mnSliderCenter;
        const long nSecondHalfRange = mxImpl->mnMaxZoom - mxImpl->mnSliderCenter;
        const long nSliderPixelPerZoomPercent = 1000 * nHalfSliderWidth  / nSecondHalfRange;
        const long nOffset = (nSliderPixelPerZoomPercent * nCurrentZoom) / 1000;
        nRet += nHalfSliderWidth + nOffset;
    }

    return nRet;
}

SvxZoomSliderControl::SvxZoomSliderControl( sal_uInt16 _nSlotId,  sal_uInt16 _nId, StatusBar& rStatusBar ) :
    SfxStatusBarControl( _nSlotId, _nId, rStatusBar ),
    mxImpl( new SvxZoomSliderControl_Impl )
{
    mxImpl->maSliderButton   = Image(StockImage::Yes, RID_SVXBMP_SLIDERBUTTON);
    mxImpl->maIncreaseButton = Image(StockImage::Yes, RID_SVXBMP_SLIDERINCREASE);
    mxImpl->maDecreaseButton = Image(StockImage::Yes, RID_SVXBMP_SLIDERDECREASE);
}

SvxZoomSliderControl::~SvxZoomSliderControl()
{
}

void SvxZoomSliderControl::StateChanged( sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( (SfxItemState::DEFAULT != eState) || pState->IsVoidItem() )
    {
        GetStatusBar().SetItemText( GetId(), "" );
        mxImpl->mbValuesSet   = false;
    }
    else
    {
        OSL_ENSURE( dynamic_cast<const SvxZoomSliderItem*>( pState) !=  nullptr, "invalid item type: should be a SvxZoomSliderItem" );
        mxImpl->mnCurrentZoom = static_cast<const SvxZoomSliderItem*>( pState )->GetValue();
        mxImpl->mnMinZoom     = static_cast<const SvxZoomSliderItem*>( pState )->GetMinZoom();
        mxImpl->mnMaxZoom     = static_cast<const SvxZoomSliderItem*>( pState )->GetMaxZoom();
        mxImpl->mnSliderCenter= 100;
        mxImpl->mbValuesSet   = true;

        if ( mxImpl->mnSliderCenter == mxImpl->mnMaxZoom )
            mxImpl->mnSliderCenter = mxImpl->mnMinZoom + static_cast<sal_uInt16>((mxImpl->mnMaxZoom - mxImpl->mnMinZoom) * 0.5);


        DBG_ASSERT( mxImpl->mnMinZoom <= mxImpl->mnCurrentZoom &&
                    mxImpl->mnMinZoom <  mxImpl->mnSliderCenter &&
                    mxImpl->mnMaxZoom >= mxImpl->mnCurrentZoom &&
                    mxImpl->mnMaxZoom > mxImpl->mnSliderCenter,
                    "Looks like the zoom slider item is corrupted" );

        const css::uno::Sequence < sal_Int32 > rSnappingPoints = static_cast<const SvxZoomSliderItem*>( pState )->GetSnappingPoints();
        mxImpl->maSnappingPointOffsets.clear();
        mxImpl->maSnappingPointZooms.clear();

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
                mxImpl->maSnappingPointOffsets.push_back( nCurrentOffset );
                mxImpl->maSnappingPointZooms.push_back( nCurrent );
                nLastOffset = nCurrentOffset;
            }
        }
    }

    forceRepaint();
}

void SvxZoomSliderControl::Paint( const UserDrawEvent& rUsrEvt )
{
    if ( !mxImpl->mbValuesSet )
        return;

    const tools::Rectangle     aControlRect = getControlRect();
    vcl::RenderContext* pDev = rUsrEvt.GetRenderContext();
    tools::Rectangle           aRect = rUsrEvt.GetRect();
    tools::Rectangle           aSlider = aRect;

    long nSliderHeight  = 1 * pDev->GetDPIScaleFactor();
    long nSnappingHeight = 2 * pDev->GetDPIScaleFactor();

    aSlider.AdjustTop((aControlRect.GetHeight() - nSliderHeight)/2 );
    aSlider.SetBottom( aSlider.Top() + nSliderHeight - 1 );
    aSlider.AdjustLeft(nSliderXOffset );
    aSlider.AdjustRight( -nSliderXOffset );

    Color               aOldLineColor = pDev->GetLineColor();
    Color               aOldFillColor = pDev->GetFillColor();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    pDev->SetLineColor( rStyleSettings.GetDarkShadowColor() );
    pDev->SetFillColor( rStyleSettings.GetDarkShadowColor() );

    // draw slider
    pDev->DrawRect( aSlider );
    // shadow
    pDev->SetLineColor( rStyleSettings.GetShadowColor() );
    pDev->DrawLine(Point(aSlider.Left()+1,aSlider.Bottom()+1), Point(aSlider.Right()+1,aSlider.Bottom()+1));
    pDev->SetLineColor( rStyleSettings.GetDarkShadowColor() );

    // draw snapping points:
    for ( const auto& rSnappingPoint : mxImpl->maSnappingPointOffsets )
    {
        long nSnapPosX = aRect.Left() + rSnappingPoint;

        pDev->DrawRect( tools::Rectangle( nSnapPosX - 1, aSlider.Top() - nSnappingHeight,
                    nSnapPosX, aSlider.Bottom() + nSnappingHeight ) );
    }

    // draw slider button
    Point aImagePoint = aRect.TopLeft();
    aImagePoint.AdjustX(Zoom2Offset( mxImpl->mnCurrentZoom ) );
    aImagePoint.AdjustX( -(mxImpl->maSliderButton.GetSizePixel().Width()/2) );
    aImagePoint.AdjustY((aControlRect.GetHeight() - mxImpl->maSliderButton.GetSizePixel().Height())/2 );
    pDev->DrawImage( aImagePoint, mxImpl->maSliderButton );

    // draw decrease button
    aImagePoint = aRect.TopLeft();
    aImagePoint.AdjustX((nSliderXOffset - mxImpl->maDecreaseButton.GetSizePixel().Width())/2 );
    aImagePoint.AdjustY((aControlRect.GetHeight() - mxImpl->maDecreaseButton.GetSizePixel().Height())/2 );
    pDev->DrawImage( aImagePoint, mxImpl->maDecreaseButton );

    // draw increase button
    aImagePoint.setX( aRect.TopLeft().X() + aControlRect.GetWidth() - mxImpl->maIncreaseButton.GetSizePixel().Width() - (nSliderXOffset - mxImpl->maIncreaseButton.GetSizePixel().Height())/2 );
    pDev->DrawImage( aImagePoint, mxImpl->maIncreaseButton );

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

bool SvxZoomSliderControl::MouseButtonDown( const MouseEvent & rEvt )
{
    if ( !mxImpl->mbValuesSet )
        return true;

    const tools::Rectangle aControlRect = getControlRect();
    const Point aPoint = rEvt.GetPosPixel();
    const sal_Int32 nXDiff = aPoint.X() - aControlRect.Left();

    long nIncDecWidth   = mxImpl->maIncreaseButton.GetSizePixel().Width();

    const long nButtonLeftOffset = (nSliderXOffset - nIncDecWidth)/2;
    const long nButtonRightOffset = (nSliderXOffset + nIncDecWidth)/2;

    const long nOldZoom = mxImpl->mnCurrentZoom;

    // click to - button
    if ( nXDiff >= nButtonLeftOffset && nXDiff <= nButtonRightOffset )
        mxImpl->mnCurrentZoom = basegfx::zoomtools::zoomOut( static_cast<int>(mxImpl->mnCurrentZoom) );
    // click to + button
    else if ( nXDiff >= aControlRect.GetWidth() - nSliderXOffset + nButtonLeftOffset &&
              nXDiff <= aControlRect.GetWidth() - nSliderXOffset + nButtonRightOffset )
        mxImpl->mnCurrentZoom = basegfx::zoomtools::zoomIn( static_cast<int>(mxImpl->mnCurrentZoom) );
    // click to slider
    else if( nXDiff >= nSliderXOffset && nXDiff <= aControlRect.GetWidth() - nSliderXOffset )
    {
        mxImpl->mnCurrentZoom = Offset2Zoom( nXDiff );
        mxImpl->mbDraggingStarted = true;
    }

    if ( mxImpl->mnCurrentZoom < mxImpl->mnMinZoom )
        mxImpl->mnCurrentZoom = mxImpl->mnMinZoom;
    else if ( mxImpl->mnCurrentZoom > mxImpl->mnMaxZoom )
        mxImpl->mnCurrentZoom = mxImpl->mnMaxZoom;

    if ( nOldZoom == mxImpl->mnCurrentZoom )
        return true;

    repaintAndExecute();

    return true;
}

bool SvxZoomSliderControl::MouseButtonUp( const MouseEvent & )
{
    mxImpl->mbDraggingStarted = false;
    return true;
}

bool SvxZoomSliderControl::MouseMove( const MouseEvent & rEvt )
{
    if ( !mxImpl->mbValuesSet )
        return true;

    const short nButtons = rEvt.GetButtons();
    const tools::Rectangle aControlRect = getControlRect();
    const Point aPoint = rEvt.GetPosPixel();
    const sal_Int32 nXDiff = aPoint.X() - aControlRect.Left();

    // check mouse move with button pressed
    if ( 1 == nButtons && mxImpl->mbDraggingStarted )
    {
        if ( nXDiff >= nSliderXOffset && nXDiff <= aControlRect.GetWidth() - nSliderXOffset )
        {
            mxImpl->mnCurrentZoom = Offset2Zoom( nXDiff );

            repaintAndExecute();
        }
    }

    // Tooltips

    long nIncDecWidth = mxImpl->maIncreaseButton.GetSizePixel().Width();

    const long nButtonLeftOffset = (nSliderXOffset - nIncDecWidth)/2;
    const long nButtonRightOffset = (nSliderXOffset + nIncDecWidth)/2;

    // click to - button
    if ( nXDiff >= nButtonLeftOffset && nXDiff <= nButtonRightOffset )
        GetStatusBar().SetQuickHelpText(GetId(), SvxResId(RID_SVXSTR_ZOOM_OUT));
    // click to + button
    else if ( nXDiff >= aControlRect.GetWidth() - nSliderXOffset + nButtonLeftOffset &&
              nXDiff <= aControlRect.GetWidth() - nSliderXOffset + nButtonRightOffset )
        GetStatusBar().SetQuickHelpText(GetId(), SvxResId(RID_SVXSTR_ZOOM_IN));
    else
        // don't hide the slider and its handle with a tooltip during zooming
        GetStatusBar().SetQuickHelpText(GetId(), "");

    return true;
}

void SvxZoomSliderControl::forceRepaint() const
{
    GetStatusBar().SetItemData(GetId(), nullptr);
}

void SvxZoomSliderControl::repaintAndExecute()
{
    forceRepaint();

    // commit state change
    SvxZoomSliderItem aZoomSliderItem(mxImpl->mnCurrentZoom);

    css::uno::Any any;
    aZoomSliderItem.QueryValue(any);

    css::uno::Sequence<css::beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "ZoomSlider";
    aArgs[0].Value = any;

    execute(aArgs);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
