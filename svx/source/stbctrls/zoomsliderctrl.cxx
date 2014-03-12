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
#include <svx/zoomslideritem.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#include <basegfx/tools/zoomtools.hxx>

#include <set>

// -----------------------------------------------------------------------

SFX_IMPL_STATUSBAR_CONTROL( SvxZoomSliderControl, SvxZoomSliderItem );

// -----------------------------------------------------------------------

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
    bool                     mbOmitPaint;

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
        mbOmitPaint( false ) {}
};

// -----------------------------------------------------------------------

const long nSliderXOffset = 20;
const long nSnappingEpsilon = 5; // snapping epsilon in pixels
const long nSnappingPointsMinDist = nSnappingEpsilon; // minimum distance of two adjacent snapping points

// -----------------------------------------------------------------------

// nOffset referes to the origin of the control:
// + ----------- -
sal_uInt16 SvxZoomSliderControl::Offset2Zoom( long nOffset ) const
{
    const long nControlWidth = getControlRect().GetWidth();
    sal_uInt16 nRet = 0;

    if ( nOffset < nSliderXOffset )
        return mpImpl->mnMinZoom;

    if ( nOffset > nControlWidth - nSliderXOffset )
        return mpImpl->mnMaxZoom;

    // check for snapping points:
    sal_uInt16 nCount = 0;
    std::vector< long >::iterator aSnappingPointIter;
    for ( aSnappingPointIter = mpImpl->maSnappingPointOffsets.begin();
          aSnappingPointIter != mpImpl->maSnappingPointOffsets.end();
          ++aSnappingPointIter )
    {
        const long nCurrent = *aSnappingPointIter;
        if ( std::abs(nCurrent - nOffset) < nSnappingEpsilon )
        {
            nOffset = nCurrent;
            nRet = mpImpl->maSnappingPointZooms[ nCount ];
            break;
        }
        ++nCount;
    }

    if ( 0 == nRet )
    {
        if ( nOffset < nControlWidth / 2 )
        {
            // first half of slider
            const long nFirstHalfRange = mpImpl->mnSliderCenter - mpImpl->mnMinZoom;
            const long nHalfSliderWidth = nControlWidth/2 - nSliderXOffset;
            const long nZoomPerSliderPixel = (1000 * nFirstHalfRange) / nHalfSliderWidth;
            const long nOffsetToSliderLeft = nOffset - nSliderXOffset;
            nRet = mpImpl->mnMinZoom + sal_uInt16( nOffsetToSliderLeft * nZoomPerSliderPixel / 1000 );
        }
        else
        {
            // second half of slider
            const long nSecondHalfRange = mpImpl->mnMaxZoom - mpImpl->mnSliderCenter;
            const long nHalfSliderWidth = nControlWidth/2 - nSliderXOffset;
            const long nZoomPerSliderPixel = 1000 * nSecondHalfRange / nHalfSliderWidth;
            const long nOffsetToSliderCenter = nOffset - nControlWidth/2;
            nRet = mpImpl->mnSliderCenter + sal_uInt16( nOffsetToSliderCenter * nZoomPerSliderPixel / 1000 );
        }
    }

    if ( nRet < mpImpl->mnMinZoom )
        nRet = mpImpl->mnMinZoom;
    else if ( nRet > mpImpl->mnMaxZoom )
        nRet = mpImpl->mnMaxZoom;

    return nRet;
}

// returns the offset to the left control border
long SvxZoomSliderControl::Zoom2Offset( sal_uInt16 nCurrentZoom ) const
{
    const long nControlWidth = getControlRect().GetWidth();
    long nRet = nSliderXOffset;

    const long nHalfSliderWidth = nControlWidth/2 - nSliderXOffset;

    if ( nCurrentZoom <= mpImpl->mnSliderCenter )
    {
        nCurrentZoom = nCurrentZoom - mpImpl->mnMinZoom;
        const long nFirstHalfRange = mpImpl->mnSliderCenter - mpImpl->mnMinZoom;
        const long nSliderPixelPerZoomPercent = 1000 * nHalfSliderWidth  / nFirstHalfRange;
        const long nOffset = (nSliderPixelPerZoomPercent * nCurrentZoom) / 1000;
        nRet += nOffset;
    }
    else
    {
        nCurrentZoom = nCurrentZoom - mpImpl->mnSliderCenter;
        const long nSecondHalfRange = mpImpl->mnMaxZoom - mpImpl->mnSliderCenter;
        const long nSliderPixelPerZoomPercent = 1000 * nHalfSliderWidth  / nSecondHalfRange;
        const long nOffset = (nSliderPixelPerZoomPercent * nCurrentZoom) / 1000;
        nRet += nHalfSliderWidth + nOffset;
    }

    return nRet;
}

// -----------------------------------------------------------------------

SvxZoomSliderControl::SvxZoomSliderControl( sal_uInt16 _nSlotId,  sal_uInt16 _nId, StatusBar& _rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, _rStb ),
    mpImpl( new SvxZoomSliderControl_Impl )
{
    mpImpl->maSliderButton   = Image( SVX_RES( RID_SVXBMP_SLIDERBUTTON   ) );
    mpImpl->maIncreaseButton = Image( SVX_RES( RID_SVXBMP_SLIDERINCREASE ) );
    mpImpl->maDecreaseButton = Image( SVX_RES( RID_SVXBMP_SLIDERDECREASE ) );

    if ( _rStb.GetDPIScaleFactor() > 1)
    {
        Image arr[3] = {mpImpl->maSliderButton, mpImpl->maIncreaseButton, mpImpl->maDecreaseButton};

        for (int i = 0; i < 3; i++)
        {
            BitmapEx b = arr[i].GetBitmapEx();
            //Use Lanczos scaling for the slider button because it does a better job with circles
            b.Scale(_rStb.GetDPIScaleFactor(), _rStb.GetDPIScaleFactor(), i == 0 ? BMP_SCALE_LANCZOS : BMP_SCALE_FAST);
            arr[i] = Image(b);
        }

        mpImpl->maSliderButton = arr[0];
        mpImpl->maIncreaseButton = arr[1];
        mpImpl->maDecreaseButton = arr[2];
    }
}

// -----------------------------------------------------------------------

SvxZoomSliderControl::~SvxZoomSliderControl()
{
    delete mpImpl;
}

// -----------------------------------------------------------------------

void SvxZoomSliderControl::StateChanged( sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( (SFX_ITEM_AVAILABLE != eState) || pState->ISA( SfxVoidItem ) )
    {
        GetStatusBar().SetItemText( GetId(), "" );
        mpImpl->mbValuesSet   = false;
    }
    else
    {
        OSL_ENSURE( pState->ISA( SvxZoomSliderItem ), "invalid item type: should be a SvxZoomSliderItem" );
        mpImpl->mnCurrentZoom = static_cast<const SvxZoomSliderItem*>( pState )->GetValue();
        mpImpl->mnMinZoom     = static_cast<const SvxZoomSliderItem*>( pState )->GetMinZoom();
        mpImpl->mnMaxZoom     = static_cast<const SvxZoomSliderItem*>( pState )->GetMaxZoom();
        mpImpl->mnSliderCenter= 100;
        mpImpl->mbValuesSet   = true;

        if ( mpImpl->mnSliderCenter == mpImpl->mnMaxZoom )
            mpImpl->mnSliderCenter = mpImpl->mnMinZoom + (sal_uInt16)((mpImpl->mnMaxZoom - mpImpl->mnMinZoom) * 0.5);


        DBG_ASSERT( mpImpl->mnMinZoom <= mpImpl->mnCurrentZoom &&
                    mpImpl->mnMinZoom <  mpImpl->mnSliderCenter &&
                    mpImpl->mnMaxZoom >= mpImpl->mnCurrentZoom &&
                    mpImpl->mnMaxZoom > mpImpl->mnSliderCenter,
                    "Looks like the zoom slider item is corrupted" );

        const com::sun::star::uno::Sequence < sal_Int32 > rSnappingPoints = static_cast<const SvxZoomSliderItem*>( pState )->GetSnappingPoints();
        mpImpl->maSnappingPointOffsets.clear();
        mpImpl->maSnappingPointZooms.clear();

        // get all snapping points:
        std::set< sal_uInt16 > aTmpSnappingPoints;
        for ( sal_uInt16 j = 0; j < rSnappingPoints.getLength(); ++j )
        {
            const sal_Int32 nSnappingPoint = rSnappingPoints[j];
            aTmpSnappingPoints.insert( (sal_uInt16)nSnappingPoint );
        }

        // remove snapping points that are to close to each other:
        std::set< sal_uInt16 >::iterator aSnappingPointIter;
        long nLastOffset = 0;

        for ( aSnappingPointIter = aTmpSnappingPoints.begin(); aSnappingPointIter != aTmpSnappingPoints.end(); ++aSnappingPointIter )
        {
            const sal_uInt16 nCurrent = *aSnappingPointIter;
            const long nCurrentOffset = Zoom2Offset( nCurrent );

            if ( nCurrentOffset - nLastOffset >= nSnappingPointsMinDist )
            {
                mpImpl->maSnappingPointOffsets.push_back( nCurrentOffset );
                mpImpl->maSnappingPointZooms.push_back( nCurrent );
                nLastOffset = nCurrentOffset;
            }
        }
    }

    if ( !mpImpl->mbOmitPaint && GetStatusBar().AreItemsVisible() )
        GetStatusBar().SetItemData( GetId(), 0 );    // force repaint
}

// -----------------------------------------------------------------------

void SvxZoomSliderControl::Paint( const UserDrawEvent& rUsrEvt )
{
    if ( !mpImpl->mbValuesSet || mpImpl->mbOmitPaint )
        return;

    const Rectangle     aControlRect = getControlRect();
    OutputDevice*       pDev =  rUsrEvt.GetDevice();
    Rectangle           aRect = rUsrEvt.GetRect();
    Rectangle           aSlider = aRect;

    long nSliderHeight  = 2 * pDev->GetDPIScaleFactor();
    long nSnappingHeight = 4 * pDev->GetDPIScaleFactor();

    aSlider.Top()   += (aControlRect.GetHeight() - nSliderHeight)/2;
    aSlider.Bottom() = aSlider.Top() + nSliderHeight - 1;
    aSlider.Left()  += nSliderXOffset;
    aSlider.Right() -= nSliderXOffset;

    Color               aOldLineColor = pDev->GetLineColor();
    Color               aOldFillColor = pDev->GetFillColor();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    pDev->SetLineColor( rStyleSettings.GetShadowColor() );
    pDev->SetFillColor( rStyleSettings.GetShadowColor() );


    // draw snapping points:
    std::vector< long >::iterator aSnappingPointIter;
    for ( aSnappingPointIter = mpImpl->maSnappingPointOffsets.begin();
          aSnappingPointIter != mpImpl->maSnappingPointOffsets.end();
          ++aSnappingPointIter )
    {
        long nSnapPosX = aRect.Left() + *aSnappingPointIter;

        pDev->DrawRect( Rectangle( nSnapPosX - 1, aSlider.Top() - nSnappingHeight,
                    nSnapPosX, aSlider.Bottom() + nSnappingHeight ) );
    }

    // draw slider
    pDev->DrawRect( aSlider );

    // draw slider button
    Point aImagePoint = aRect.TopLeft();
    aImagePoint.X() += Zoom2Offset( mpImpl->mnCurrentZoom );
    aImagePoint.X() -= mpImpl->maSliderButton.GetSizePixel().Width()/2;
    aImagePoint.Y() += (aControlRect.GetHeight() - mpImpl->maSliderButton.GetSizePixel().Height())/2;
    pDev->DrawImage( aImagePoint, mpImpl->maSliderButton );

    // draw decrease button
    aImagePoint = aRect.TopLeft();
    aImagePoint.X() += (nSliderXOffset - mpImpl->maDecreaseButton.GetSizePixel().Width())/2;
    aImagePoint.Y() += (aControlRect.GetHeight() - mpImpl->maDecreaseButton.GetSizePixel().Height())/2;
    pDev->DrawImage( aImagePoint, mpImpl->maDecreaseButton );

    // draw increase button
    aImagePoint.X() = aRect.TopLeft().X() + aControlRect.GetWidth() - mpImpl->maIncreaseButton.GetSizePixel().Width() - (nSliderXOffset - mpImpl->maIncreaseButton.GetSizePixel().Height())/2;
    pDev->DrawImage( aImagePoint, mpImpl->maIncreaseButton );

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

// -----------------------------------------------------------------------

sal_Bool SvxZoomSliderControl::MouseButtonDown( const MouseEvent & rEvt )
{
    if ( !mpImpl->mbValuesSet )
        return sal_True;

    const Rectangle aControlRect = getControlRect();
    const Point aPoint = rEvt.GetPosPixel();
    const sal_Int32 nXDiff = aPoint.X() - aControlRect.Left();

    long nIncDecWidth   = mpImpl->maIncreaseButton.GetSizePixel().Width();

    const long nButtonLeftOffset = (nSliderXOffset - nIncDecWidth)/2;
    const long nButtonRightOffset = (nSliderXOffset + nIncDecWidth)/2;

    const long nOldZoom = mpImpl->mnCurrentZoom;

    // click to - button
    if ( nXDiff >= nButtonLeftOffset && nXDiff <= nButtonRightOffset )
        mpImpl->mnCurrentZoom = basegfx::zoomtools::zoomOut( static_cast<int>(mpImpl->mnCurrentZoom) );
    // click to + button
    else if ( nXDiff >= aControlRect.GetWidth() - nSliderXOffset + nButtonLeftOffset &&
              nXDiff <= aControlRect.GetWidth() - nSliderXOffset + nButtonRightOffset )
        mpImpl->mnCurrentZoom = basegfx::zoomtools::zoomIn( static_cast<int>(mpImpl->mnCurrentZoom) );
    // click to slider
    else if( nXDiff >= nSliderXOffset && nXDiff <= aControlRect.GetWidth() - nSliderXOffset )
        mpImpl->mnCurrentZoom = Offset2Zoom( nXDiff );

    if ( mpImpl->mnCurrentZoom < mpImpl->mnMinZoom )
        mpImpl->mnCurrentZoom = mpImpl->mnMinZoom;
    else if ( mpImpl->mnCurrentZoom > mpImpl->mnMaxZoom )
        mpImpl->mnCurrentZoom = mpImpl->mnMaxZoom;

    if ( nOldZoom == mpImpl->mnCurrentZoom )
        return sal_True;

    if ( GetStatusBar().AreItemsVisible() )
        GetStatusBar().SetItemData( GetId(), 0 );    // force repaint

    mpImpl->mbOmitPaint = true; // optimization: paint before executing command,
                                // then omit painting which is triggered by the execute function

    SvxZoomSliderItem aZoomSliderItem( mpImpl->mnCurrentZoom );

    ::com::sun::star::uno::Any a;
    aZoomSliderItem.QueryValue( a );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = "ZoomSlider";
    aArgs[0].Value = a;

    execute( aArgs );

    mpImpl->mbOmitPaint = false;

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxZoomSliderControl::MouseMove( const MouseEvent & rEvt )
{
    if ( !mpImpl->mbValuesSet )
        return sal_True;

    const short nButtons = rEvt.GetButtons();

    // check mouse move with button pressed
    if ( 1 == nButtons )
    {
        const Rectangle aControlRect = getControlRect();
        const Point aPoint = rEvt.GetPosPixel();
        const sal_Int32 nXDiff = aPoint.X() - aControlRect.Left();

        if ( nXDiff >= nSliderXOffset && nXDiff <= aControlRect.GetWidth() - nSliderXOffset )
        {
            mpImpl->mnCurrentZoom = Offset2Zoom( nXDiff );

            if ( GetStatusBar().AreItemsVisible() )
                GetStatusBar().SetItemData( GetId(), 0 );    // force repaint

            mpImpl->mbOmitPaint = true; // optimization: paint before executing command,
                                        // then omit painting which is triggered by the execute function

            // commit state change
            SvxZoomSliderItem aZoomSliderItem( mpImpl->mnCurrentZoom );

            ::com::sun::star::uno::Any a;
            aZoomSliderItem.QueryValue( a );

            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name = "ZoomSlider";
            aArgs[0].Value = a;

            execute( aArgs );

            mpImpl->mbOmitPaint = false;
        }
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
