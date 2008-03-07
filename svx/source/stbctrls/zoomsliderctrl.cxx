/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoomsliderctrl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:19:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#ifndef _ZOOMSLIDER_STBCONTRL_HXX
#include <svx/zoomsliderctrl.hxx>
#endif
#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SVX_ZOOMSLIDERITEM_HXX
#include <svx/zoomslideritem.hxx>
#endif
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#include <set>

// -----------------------------------------------------------------------

SFX_IMPL_STATUSBAR_CONTROL( SvxZoomSliderControl, SvxZoomSliderItem );

// -----------------------------------------------------------------------

struct SvxZoomSliderControl::SvxZoomSliderControl_Impl
{
    USHORT                   mnCurrentZoom;
    USHORT                   mnMinZoom;
    USHORT                   mnMaxZoom;
    USHORT                   mnSliderCenter;
    std::vector< long >      maSnappingPointOffsets;
    std::vector< USHORT >    maSnappingPointZooms;
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

const long nButtonWidth   = 10;
const long nButtonHeight  = 10;
const long nIncDecWidth   = 11;
const long nIncDecHeight  = 11;
const long nSliderHeight  = 2;
const long nSnappingHeight = 4;
const long nSliderXOffset = 20;
const long nSnappingEpsilon = 5; // snapping epsilon in pixels
const long nSnappingPointsMinDist = nSnappingEpsilon; // minimum distance of two adjacent snapping points

// -----------------------------------------------------------------------

// nOffset referes to the origin of the control:
// + ----------- -
USHORT SvxZoomSliderControl::Offset2Zoom( long nOffset ) const
{
    const long nControlWidth = getControlRect().GetWidth();
    USHORT nRet = 0;

    if ( nOffset < nSliderXOffset )
        return mpImpl->mnMinZoom;;

    if ( nOffset > nControlWidth - nSliderXOffset )
        return mpImpl->mnMaxZoom;

    // check for snapping points:
    USHORT nCount = 0;
    std::vector< long >::iterator aSnappingPointIter;
    for ( aSnappingPointIter = mpImpl->maSnappingPointOffsets.begin();
          aSnappingPointIter != mpImpl->maSnappingPointOffsets.end();
          ++aSnappingPointIter )
    {
        const long nCurrent = *aSnappingPointIter;
        if ( Abs(nCurrent - nOffset) < nSnappingEpsilon )
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
            nRet = mpImpl->mnMinZoom + USHORT( nOffsetToSliderLeft * nZoomPerSliderPixel / 1000 );
        }
        else
        {
            // second half of slider
            const long nSecondHalfRange = mpImpl->mnMaxZoom - mpImpl->mnSliderCenter;
            const long nHalfSliderWidth = nControlWidth/2 - nSliderXOffset;
            const long nZoomPerSliderPixel = 1000 * nSecondHalfRange / nHalfSliderWidth;
            const long nOffsetToSliderCenter = nOffset - nControlWidth/2;
            nRet = mpImpl->mnSliderCenter + USHORT( nOffsetToSliderCenter * nZoomPerSliderPixel / 1000 );
        }
    }

    if ( nRet < mpImpl->mnMinZoom )
        nRet = mpImpl->mnMinZoom;
    else if ( nRet > mpImpl->mnMaxZoom )
        nRet = mpImpl->mnMaxZoom;

    return nRet;
}

// returns the offset to the left control border
long SvxZoomSliderControl::Zoom2Offset( USHORT nCurrentZoom ) const
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

SvxZoomSliderControl::SvxZoomSliderControl( USHORT _nSlotId,  USHORT _nId, StatusBar& _rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, _rStb ),
    mpImpl( new SvxZoomSliderControl_Impl )
{
    const sal_Bool bIsDark = GetStatusBar().GetBackground().GetColor().IsDark();
    mpImpl->maSliderButton   = Image( SVX_RES( bIsDark ? RID_SVXBMP_SLIDERBUTTON_HC : RID_SVXBMP_SLIDERBUTTON ) );
    mpImpl->maIncreaseButton = Image( SVX_RES( bIsDark ? RID_SVXBMP_SLIDERINCREASE_HC : RID_SVXBMP_SLIDERINCREASE ) );
    mpImpl->maDecreaseButton = Image( SVX_RES( bIsDark ? RID_SVXBMP_SLIDERDECREASE_HC : RID_SVXBMP_SLIDERDECREASE ) );
}

// -----------------------------------------------------------------------

SvxZoomSliderControl::~SvxZoomSliderControl()
{
    delete mpImpl;
}

// -----------------------------------------------------------------------

void SvxZoomSliderControl::StateChanged( USHORT /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState || pState->ISA( SfxVoidItem ) )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        DBG_ASSERT( pState->ISA( SvxZoomSliderItem ), "invalid item type" );
        mpImpl->mnCurrentZoom = static_cast<const SvxZoomSliderItem*>( pState )->GetValue();
        mpImpl->mnMinZoom     = static_cast<const SvxZoomSliderItem*>( pState )->GetMinZoom();
        mpImpl->mnMaxZoom     = static_cast<const SvxZoomSliderItem*>( pState )->GetMaxZoom();
        mpImpl->mnSliderCenter= 100;
        mpImpl->mbValuesSet   = true;

        DBG_ASSERT( mpImpl->mnMinZoom <= mpImpl->mnCurrentZoom &&
                    mpImpl->mnMinZoom <  mpImpl->mnSliderCenter &&
                    mpImpl->mnMaxZoom >= mpImpl->mnCurrentZoom &&
                    mpImpl->mnMaxZoom > mpImpl->mnSliderCenter,
                    "Looks like the zoom slider item is corrupted" )

        const com::sun::star::uno::Sequence < sal_Int32 > rSnappingPoints = static_cast<const SvxZoomSliderItem*>( pState )->GetSnappingPoints();
        mpImpl->maSnappingPointOffsets.clear();
        mpImpl->maSnappingPointZooms.clear();

        // get all snapping points:
        std::set< USHORT > aTmpSnappingPoints;
        for ( USHORT j = 0; j < rSnappingPoints.getLength(); ++j )
        {
            const sal_Int32 nSnappingPoint = rSnappingPoints[j];
            aTmpSnappingPoints.insert( (USHORT)nSnappingPoint );
        }

        // remove snapping points that are to close to each other:
        std::set< USHORT >::iterator aSnappingPointIter;
        long nLastOffset = 0;

        for ( aSnappingPointIter = aTmpSnappingPoints.begin(); aSnappingPointIter != aTmpSnappingPoints.end(); ++aSnappingPointIter )
        {
            const USHORT nCurrent = *aSnappingPointIter;
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

    aSlider.Top()   += (aControlRect.GetHeight() - nSliderHeight)/2 - 1;
    aSlider.Bottom() = aSlider.Top() + nSliderHeight;
    aSlider.Left()  += nSliderXOffset;
    aSlider.Right() -= nSliderXOffset;

    Color               aOldLineColor = pDev->GetLineColor();
    Color               aOldFillColor = pDev->GetFillColor();

    pDev->SetLineColor( Color( COL_GRAY ) );
    pDev->SetFillColor( Color( COL_GRAY ) );

    // draw snapping points:
    std::vector< long >::iterator aSnappingPointIter;
    for ( aSnappingPointIter = mpImpl->maSnappingPointOffsets.begin();
          aSnappingPointIter != mpImpl->maSnappingPointOffsets.end();
          ++aSnappingPointIter )
    {
        Rectangle aSnapping( aRect );
        aSnapping.Bottom()   = aSlider.Top();
        aSnapping.Top() = aSnapping.Bottom() - nSnappingHeight;
        aSnapping.Left() += *aSnappingPointIter;
        aSnapping.Right() = aSnapping.Left();
        pDev->DrawRect( aSnapping );

        aSnapping.Top() += nSnappingHeight + nSliderHeight;
        aSnapping.Bottom() += nSnappingHeight + nSliderHeight;
        pDev->DrawRect( aSnapping );
    }

    // draw slider
    Rectangle aFirstLine( aSlider );
    aFirstLine.Bottom() = aFirstLine.Top();

    Rectangle aSecondLine( aSlider );
    aSecondLine.Top() = aSecondLine.Bottom();

    Rectangle aLeft( aSlider );
    aLeft.Right() = aLeft.Left();

    Rectangle aRight( aSlider );
    aRight.Left() = aRight.Right();

    pDev->SetLineColor( Color ( COL_WHITE ) );
    pDev->SetFillColor( Color ( COL_WHITE ) );
    pDev->DrawRect( aSecondLine );
    pDev->DrawRect( aRight );

    pDev->SetLineColor( Color( COL_GRAY ) );
    pDev->SetFillColor( Color( COL_GRAY ) );
    pDev->DrawRect( aFirstLine );
    pDev->DrawRect( aLeft );

    // draw slider button
    Point aImagePoint = aRect.TopLeft();
    aImagePoint.X() += Zoom2Offset( mpImpl->mnCurrentZoom );
    aImagePoint.X() -= nButtonWidth/2;
    aImagePoint.Y() += (aControlRect.GetHeight() - nButtonHeight)/2;
    pDev->DrawImage( aImagePoint, mpImpl->maSliderButton );

    // draw decrease button
    aImagePoint = aRect.TopLeft();
    aImagePoint.X() += (nSliderXOffset - nIncDecWidth)/2;
    aImagePoint.Y() += (aControlRect.GetHeight() - nIncDecHeight)/2;
    pDev->DrawImage( aImagePoint, mpImpl->maDecreaseButton );

    // draw increase button
    aImagePoint.X() = aRect.TopLeft().X() + aControlRect.GetWidth() - nIncDecWidth - (nSliderXOffset - nIncDecWidth)/2;
    pDev->DrawImage( aImagePoint, mpImpl->maIncreaseButton );

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

// -----------------------------------------------------------------------

BOOL SvxZoomSliderControl::MouseButtonDown( const MouseEvent & rEvt )
{
    if ( !mpImpl->mbValuesSet )
        return TRUE;;

    const Rectangle aControlRect = getControlRect();
    const Point aPoint = rEvt.GetPosPixel();
    const sal_Int32 nXDiff = aPoint.X() - aControlRect.Left();

    const long nButtonLeftOffset = (nSliderXOffset - nIncDecWidth)/2;
    const long nButtonRightOffset = (nSliderXOffset + nIncDecWidth)/2;

    const long nOldZoom = mpImpl->mnCurrentZoom;

    // click to - button
    if ( nXDiff >= nButtonLeftOffset && nXDiff <= nButtonRightOffset )
        mpImpl->mnCurrentZoom = mpImpl->mnCurrentZoom - 5;
    // click to + button
    else if ( nXDiff >= aControlRect.GetWidth() - nSliderXOffset + nButtonLeftOffset &&
              nXDiff <= aControlRect.GetWidth() - nSliderXOffset + nButtonRightOffset )
        mpImpl->mnCurrentZoom = mpImpl->mnCurrentZoom + 5;
    // click to slider
    else if( nXDiff >= nSliderXOffset && nXDiff <= aControlRect.GetWidth() - nSliderXOffset )
        mpImpl->mnCurrentZoom = Offset2Zoom( nXDiff );

    if ( mpImpl->mnCurrentZoom < mpImpl->mnMinZoom )
        mpImpl->mnCurrentZoom = mpImpl->mnMinZoom;
    else if ( mpImpl->mnCurrentZoom > mpImpl->mnMaxZoom )
        mpImpl->mnCurrentZoom = mpImpl->mnMaxZoom;

    if ( nOldZoom == mpImpl->mnCurrentZoom )
        return TRUE;

    if ( GetStatusBar().AreItemsVisible() )
        GetStatusBar().SetItemData( GetId(), 0 );    // force repaint

    mpImpl->mbOmitPaint = true; // optimization: paint before executing command,
                                // then omit painting which is triggered by the execute function

    SvxZoomSliderItem aZoomSliderItem( mpImpl->mnCurrentZoom );

    ::com::sun::star::uno::Any a;
    aZoomSliderItem.QueryValue( a );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ZoomSlider" ));
    aArgs[0].Value = a;

    execute( aArgs );

    mpImpl->mbOmitPaint = false;

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL SvxZoomSliderControl::MouseMove( const MouseEvent & rEvt )
{
    if ( !mpImpl->mbValuesSet )
        return TRUE;;

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
            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ZoomSlider" ));
            aArgs[0].Value = a;

            execute( aArgs );

            mpImpl->mbOmitPaint = false;
        }
    }

    return TRUE;
}
