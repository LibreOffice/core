/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "vcl/event.hxx"
#include "vcl/decoview.hxx"
#include "vcl/scrbar.hxx"
#include "vcl/timer.hxx"

#include "svdata.hxx"

#include "rtl/string.hxx"
#include "tools/rc.h"

using ::rtl::OUString;


/*  #i77549#
    HACK: for scrollbars in case of thumb rect, page up and page down rect we
    abuse the HitTestNativeControl interface. All theming engines but aqua
    are actually able to draw the thumb according to our internal representation.
    However aqua draws a little outside. The canonical way would be to enhance the
    HitTestNativeControl passing a ScrollbarValue additionally so all necessary
    information is available in the call.
    .
    However since there is only this one small exception we will deviate a little and
    instead pass the respective rect as control region to allow for a small correction.

    So all places using HitTestNativeControl on PART_THUMB_HORZ, PART_THUMB_VERT,
    PART_TRACK_HORZ_LEFT, PART_TRACK_HORZ_RIGHT, PART_TRACK_VERT_UPPER, PART_TRACK_VERT_LOWER
    do not use the control rectangle as region but the actuall part rectangle, making
    only small deviations feasible.
*/

#include "thumbpos.hxx"

// =======================================================================

#define SCRBAR_DRAW_BTN1            ((sal_uInt16)0x0001)
#define SCRBAR_DRAW_BTN2            ((sal_uInt16)0x0002)
#define SCRBAR_DRAW_PAGE1           ((sal_uInt16)0x0004)
#define SCRBAR_DRAW_PAGE2           ((sal_uInt16)0x0008)
#define SCRBAR_DRAW_THUMB           ((sal_uInt16)0x0010)
#define SCRBAR_DRAW_BACKGROUND      ((sal_uInt16)0x0020)
#define SCRBAR_DRAW_ALL             (SCRBAR_DRAW_BTN1 | SCRBAR_DRAW_BTN2 |  \
                                     SCRBAR_DRAW_PAGE1 | SCRBAR_DRAW_PAGE2 |\
                                     SCRBAR_DRAW_THUMB | SCRBAR_DRAW_BACKGROUND )

#define SCRBAR_STATE_BTN1_DOWN      ((sal_uInt16)0x0001)
#define SCRBAR_STATE_BTN1_DISABLE   ((sal_uInt16)0x0002)
#define SCRBAR_STATE_BTN2_DOWN      ((sal_uInt16)0x0004)
#define SCRBAR_STATE_BTN2_DISABLE   ((sal_uInt16)0x0008)
#define SCRBAR_STATE_PAGE1_DOWN     ((sal_uInt16)0x0010)
#define SCRBAR_STATE_PAGE2_DOWN     ((sal_uInt16)0x0020)
#define SCRBAR_STATE_THUMB_DOWN     ((sal_uInt16)0x0040)

#define SCRBAR_VIEW_STYLE           (WB_3DLOOK | WB_HORZ | WB_VERT)

struct ImplScrollBarData
{
    AutoTimer       maTimer;            // Timer
    sal_Bool            mbHide;
    Rectangle       maTrackRect; // TODO: move to ScrollBar class when binary incompatibility of ScrollBar class is no longer problematic
};

// =======================================================================

void ScrollBar::ImplInit( Window* pParent, WinBits nStyle )
{
    mpData              = NULL;
    mnThumbPixRange     = 0;
    mnThumbPixPos       = 0;
    mnThumbPixSize      = 0;
    mnMinRange          = 0;
    mnMaxRange          = 100;
    mnThumbPos          = 0;
    mnVisibleSize       = 0;
    mnLineSize          = 1;
    mnPageSize          = 1;
    mnDelta             = 0;
    mnDragDraw          = 0;
    mnStateFlags        = 0;
    meScrollType        = SCROLL_DONTKNOW;
    meDDScrollType      = SCROLL_DONTKNOW;
    mbCalcSize          = sal_True;
    mbFullDrag          = 0;

    if( !mpData )  // TODO: remove when maTrackRect is no longer in mpData
    {
        mpData = new ImplScrollBarData;
        mpData->maTimer.SetTimeoutHdl( LINK( this, ScrollBar, ImplAutoTimerHdl ) );
        mpData->mbHide = sal_False;
    }

    ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );

    long nScrollSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    SetSizePixel( Size( nScrollSize, nScrollSize ) );
    SetBackground();
}

// -----------------------------------------------------------------------

void ScrollBar::ImplInitStyle( WinBits nStyle )
{
    if ( nStyle & WB_DRAG )
        mbFullDrag = sal_True;
    else
        mbFullDrag = (GetSettings().GetStyleSettings().GetDragFullOptions() & DRAGFULL_OPTION_SCROLL) != 0;
}

// -----------------------------------------------------------------------

ScrollBar::ScrollBar( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_SCROLLBAR )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ScrollBar::ScrollBar( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_SCROLLBAR )
{
    rResId.SetRT( RSC_SCROLLBAR );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

ScrollBar::~ScrollBar()
{
    if( mpData )
        delete mpData;
}

// -----------------------------------------------------------------------

void ScrollBar::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    sal_Int16 nMin          = ReadShortRes();
    sal_Int16 nMax          = ReadShortRes();
    sal_Int16 nThumbPos     = ReadShortRes();
    sal_Int16 nPage         = ReadShortRes();
    sal_Int16 nStep         = ReadShortRes();
    sal_Int16 nVisibleSize  = ReadShortRes();

    SetRange( Range( nMin, nMax ) );
    SetLineSize( nStep );
    SetPageSize( nPage );
    SetVisibleSize( nVisibleSize );
    SetThumbPos( nThumbPos );
}

// -----------------------------------------------------------------------

void ScrollBar::ImplUpdateRects( sal_Bool bUpdate )
{
    sal_uInt16      nOldStateFlags  = mnStateFlags;
    Rectangle   aOldPage1Rect = maPage1Rect;
    Rectangle   aOldPage2Rect = maPage2Rect;
    Rectangle   aOldThumbRect = maThumbRect;

    mnStateFlags  &= ~SCRBAR_STATE_BTN1_DISABLE;
    mnStateFlags  &= ~SCRBAR_STATE_BTN2_DISABLE;

    Rectangle& maTrackRect = mpData->maTrackRect; // TODO: remove when maTrackRect is no longer in mpData
    if ( mnThumbPixRange )
    {
        if ( GetStyle() & WB_HORZ )
        {
            maThumbRect.Left()      = maTrackRect.Left()+mnThumbPixPos;
            maThumbRect.Right()     = maThumbRect.Left()+mnThumbPixSize-1;
            if ( !mnThumbPixPos )
                maPage1Rect.Right()     = RECT_EMPTY;
            else
                maPage1Rect.Right()     = maThumbRect.Left()-1;
            if ( mnThumbPixPos >= (mnThumbPixRange-mnThumbPixSize) )
                maPage2Rect.Right()     = RECT_EMPTY;
            else
            {
                maPage2Rect.Left()      = maThumbRect.Right()+1;
                maPage2Rect.Right()     = maTrackRect.Right();
            }
        }
        else
        {
            maThumbRect.Top()       = maTrackRect.Top()+mnThumbPixPos;
            maThumbRect.Bottom()    = maThumbRect.Top()+mnThumbPixSize-1;
            if ( !mnThumbPixPos )
                maPage1Rect.Bottom()    = RECT_EMPTY;
            else
                maPage1Rect.Bottom()    = maThumbRect.Top()-1;
            if ( mnThumbPixPos >= (mnThumbPixRange-mnThumbPixSize) )
                maPage2Rect.Bottom()    = RECT_EMPTY;
            else
            {
                maPage2Rect.Top()       = maThumbRect.Bottom()+1;
                maPage2Rect.Bottom()    = maTrackRect.Bottom();
            }
        }
    }
    else
    {
        if ( GetStyle() & WB_HORZ )
        {
            const long nSpace = maTrackRect.Right() - maTrackRect.Left();
            if ( nSpace > 0 )
            {
                maPage1Rect.Left()   = maTrackRect.Left();
                maPage1Rect.Right()  = maTrackRect.Left() + (nSpace/2);
                maPage2Rect.Left()   = maPage1Rect.Right() + 1;
                maPage2Rect.Right()  = maTrackRect.Right();
            }
        }
        else
        {
            const long nSpace = maTrackRect.Bottom() - maTrackRect.Top();
            if ( nSpace > 0 )
            {
                maPage1Rect.Top()    = maTrackRect.Top();
                maPage1Rect.Bottom() = maTrackRect.Top() + (nSpace/2);
                maPage2Rect.Top()    = maPage1Rect.Bottom() + 1;
                maPage2Rect.Bottom() = maTrackRect.Bottom();
            }
        }
    }

    if( !IsNativeControlSupported(CTRL_SCROLLBAR, PART_ENTIRE_CONTROL) )
    {
        // disable scrollbar buttons only in VCL's own 'theme'
        // as it is uncommon on other platforms
        if ( mnThumbPos == mnMinRange )
            mnStateFlags |= SCRBAR_STATE_BTN1_DISABLE;
        if ( mnThumbPos >= (mnMaxRange-mnVisibleSize) )
            mnStateFlags |= SCRBAR_STATE_BTN2_DISABLE;
    }

    if ( bUpdate )
    {
        sal_uInt16 nDraw = 0;
        if ( (nOldStateFlags & SCRBAR_STATE_BTN1_DISABLE) !=
             (mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) )
            nDraw |= SCRBAR_DRAW_BTN1;
        if ( (nOldStateFlags & SCRBAR_STATE_BTN2_DISABLE) !=
             (mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) )
            nDraw |= SCRBAR_DRAW_BTN2;
        if ( aOldPage1Rect != maPage1Rect )
            nDraw |= SCRBAR_DRAW_PAGE1;
        if ( aOldPage2Rect != maPage2Rect )
            nDraw |= SCRBAR_DRAW_PAGE2;
        if ( aOldThumbRect != maThumbRect )
            nDraw |= SCRBAR_DRAW_THUMB;
        ImplDraw( nDraw, this );
    }
}

// -----------------------------------------------------------------------

long ScrollBar::ImplCalcThumbPos( long nPixPos )
{
    // Position berechnen
    long nCalcThumbPos;
    nCalcThumbPos = ImplMulDiv( nPixPos, mnMaxRange-mnVisibleSize-mnMinRange,
                                mnThumbPixRange-mnThumbPixSize );
    nCalcThumbPos += mnMinRange;
    return nCalcThumbPos;
}

// -----------------------------------------------------------------------

long ScrollBar::ImplCalcThumbPosPix( long nPos )
{
    long nCalcThumbPos;

    // Position berechnen
    nCalcThumbPos = ImplMulDiv( nPos-mnMinRange, mnThumbPixRange-mnThumbPixSize,
                                mnMaxRange-mnVisibleSize-mnMinRange );

    // Am Anfang und Ende des ScrollBars versuchen wir die Anzeige korrekt
    // anzuzeigen
    if ( !nCalcThumbPos && (mnThumbPos > mnMinRange) )
        nCalcThumbPos = 1;
    if ( nCalcThumbPos &&
         ((nCalcThumbPos+mnThumbPixSize) >= mnThumbPixRange) &&
         (mnThumbPos < (mnMaxRange-mnVisibleSize)) )
        nCalcThumbPos--;

    return nCalcThumbPos;
}

// -----------------------------------------------------------------------

void ScrollBar::ImplCalc( sal_Bool bUpdate )
{
    const Size aSize = GetOutputSizePixel();
    const long nMinThumbSize = GetSettings().GetStyleSettings().GetMinThumbSize();

    Rectangle& maTrackRect = mpData->maTrackRect;  // TODO: remove when maTrackRect is no longer in mpData
    if ( mbCalcSize )
    {
        Size aOldSize = GetOptimalSize(WINDOWSIZE_PREFERRED);

        const Rectangle aControlRegion( Point(0,0), aSize );
        Rectangle aBtn1Region, aBtn2Region, aTrackRegion, aBoundingRegion;

        if ( GetStyle() & WB_HORZ )
        {
            if ( GetNativeControlRegion( CTRL_SCROLLBAR, PART_BUTTON_LEFT,
                        aControlRegion, 0, ImplControlValue(), rtl::OUString(), aBoundingRegion, aBtn1Region ) &&
                 GetNativeControlRegion( CTRL_SCROLLBAR, PART_BUTTON_RIGHT,
                        aControlRegion, 0, ImplControlValue(), rtl::OUString(), aBoundingRegion, aBtn2Region ) )
            {
                maBtn1Rect = aBtn1Region;
                maBtn2Rect = aBtn2Region;
            }
            else
            {
                Size aBtnSize( aSize.Height(), aSize.Height() );
                maBtn2Rect.Top()    = maBtn1Rect.Top();
                maBtn2Rect.Left()   = aSize.Width()-aSize.Height();
                maBtn1Rect.SetSize( aBtnSize );
                maBtn2Rect.SetSize( aBtnSize );
            }

            if ( GetNativeControlRegion( CTRL_SCROLLBAR, PART_TRACK_HORZ_AREA,
                     aControlRegion, 0, ImplControlValue(), rtl::OUString(), aBoundingRegion, aTrackRegion ) )
                maTrackRect = aTrackRegion;
            else
                maTrackRect = Rectangle( maBtn1Rect.TopRight(), maBtn2Rect.BottomLeft() );

            // Check if available space is big enough for thumb ( min thumb size = ScrBar width/height )
            mnThumbPixRange = maTrackRect.Right() - maTrackRect.Left();
            if( mnThumbPixRange > 0 )
            {
                maPage1Rect.Left()      = maTrackRect.Left();
                maPage1Rect.Bottom()    =
                maPage2Rect.Bottom()    =
                maThumbRect.Bottom()    = maTrackRect.Bottom();
            }
            else
            {
                mnThumbPixRange = 0;
                maPage1Rect.SetEmpty();
                maPage2Rect.SetEmpty();
            }
        }
        else
        {
            if ( GetNativeControlRegion( CTRL_SCROLLBAR, PART_BUTTON_UP,
                        aControlRegion, 0, ImplControlValue(), rtl::OUString(), aBoundingRegion, aBtn1Region ) &&
                 GetNativeControlRegion( CTRL_SCROLLBAR, PART_BUTTON_DOWN,
                        aControlRegion, 0, ImplControlValue(), rtl::OUString(), aBoundingRegion, aBtn2Region ) )
            {
                maBtn1Rect = aBtn1Region;
                maBtn2Rect = aBtn2Region;
            }
            else
            {
                const Size aBtnSize( aSize.Width(), aSize.Width() );
                maBtn2Rect.Left()   = maBtn1Rect.Left();
                maBtn2Rect.Top()    = aSize.Height()-aSize.Width();
                maBtn1Rect.SetSize( aBtnSize );
                maBtn2Rect.SetSize( aBtnSize );
            }

            if ( GetNativeControlRegion( CTRL_SCROLLBAR, PART_TRACK_VERT_AREA,
                     aControlRegion, 0, ImplControlValue(), rtl::OUString(), aBoundingRegion, aTrackRegion ) )
                maTrackRect = aTrackRegion;
            else
                maTrackRect = Rectangle( maBtn1Rect.BottomLeft()+Point(0,1), maBtn2Rect.TopRight() );

            // Check if available space is big enough for thumb
            mnThumbPixRange = maTrackRect.Bottom() - maTrackRect.Top();
            if( mnThumbPixRange > 0 )
            {
                maPage1Rect.Top()       = maTrackRect.Top();
                maPage1Rect.Right()     =
                maPage2Rect.Right()     =
                maThumbRect.Right()     = maTrackRect.Right();
            }
            else
            {
                mnThumbPixRange = 0;
                maPage1Rect.SetEmpty();
                maPage2Rect.SetEmpty();
            }
        }

        if ( !mnThumbPixRange )
            maThumbRect.SetEmpty();

        mbCalcSize = sal_False;

        Size aNewSize = GetOptimalSize(WINDOWSIZE_PREFERRED);
        if (aOldSize != aNewSize)
        {
            queue_resize();
        }
    }

    if ( mnThumbPixRange )
    {
        // Werte berechnen
        if ( (mnVisibleSize >= (mnMaxRange-mnMinRange)) ||
             ((mnMaxRange-mnMinRange) <= 0) )
        {
            mnThumbPos      = mnMinRange;
            mnThumbPixPos   = 0;
            mnThumbPixSize  = mnThumbPixRange;
        }
        else
        {
            if ( mnVisibleSize )
                mnThumbPixSize = ImplMulDiv( mnThumbPixRange, mnVisibleSize, mnMaxRange-mnMinRange );
            else
            {
                if ( GetStyle() & WB_HORZ )
                    mnThumbPixSize = maThumbRect.GetWidth();
                else
                    mnThumbPixSize = maThumbRect.GetHeight();
            }
            if ( mnThumbPixSize < nMinThumbSize )
                mnThumbPixSize = nMinThumbSize;
            if ( mnThumbPixSize > mnThumbPixRange )
                mnThumbPixSize = mnThumbPixRange;
            mnThumbPixPos = ImplCalcThumbPosPix( mnThumbPos );
        }
    }

    // Wenn neu ausgegeben werden soll und wir schon ueber eine
    // Aktion einen Paint-Event ausgeloest bekommen haben, dann
    // geben wir nicht direkt aus, sondern invalidieren nur alles
    if ( bUpdate && HasPaintEvent() )
    {
        Invalidate();
        bUpdate = sal_False;
    }
    ImplUpdateRects( bUpdate );
}

// -----------------------------------------------------------------------

void ScrollBar::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Rectangle   aRect( aPos, aSize );

    pDev->Push();
    pDev->SetMapMode();
    if ( !(nFlags & WINDOW_DRAW_MONO) )
    {
        // DecoView uses the FaceColor...
        AllSettings aSettings = pDev->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        if ( IsControlBackground() )
            aStyleSettings.SetFaceColor( GetControlBackground() );
        else
            aStyleSettings.SetFaceColor( GetSettings().GetStyleSettings().GetFaceColor() );

        aSettings.SetStyleSettings( aStyleSettings );
        pDev->SetSettings( aSettings );
    }

    // for printing:
    // -calculate the size of the rects
    // -because this is zero-based add the correct offset
    // -print
    // -force recalculate

    if ( mbCalcSize )
        ImplCalc( sal_False );

    maBtn1Rect+=aPos;
    maBtn2Rect+=aPos;
    maThumbRect+=aPos;
    mpData->maTrackRect+=aPos; // TODO: update when maTrackRect is no longer in mpData
    maPage1Rect+=aPos;
    maPage2Rect+=aPos;

    ImplDraw( SCRBAR_DRAW_ALL, pDev );
    pDev->Pop();

    mbCalcSize = sal_True;
}

// -----------------------------------------------------------------------

sal_Bool ScrollBar::ImplDrawNative( sal_uInt16 nDrawFlags )
{
    ScrollbarValue scrValue;

    sal_Bool bNativeOK = IsNativeControlSupported(CTRL_SCROLLBAR, PART_ENTIRE_CONTROL);
    if( bNativeOK )
    {
        sal_Bool bHorz = (GetStyle() & WB_HORZ ? true : false);

        // Draw the entire background if the control supports it
        if( IsNativeControlSupported(CTRL_SCROLLBAR, bHorz ? PART_DRAW_BACKGROUND_HORZ : PART_DRAW_BACKGROUND_VERT) )
        {
            ControlState        nState = ( IsEnabled() ? CTRL_STATE_ENABLED : 0 ) | ( HasFocus() ? CTRL_STATE_FOCUSED : 0 );

            scrValue.mnMin = mnMinRange;
            scrValue.mnMax = mnMaxRange;
            scrValue.mnCur = mnThumbPos;
            scrValue.mnVisibleSize = mnVisibleSize;
            scrValue.maThumbRect = maThumbRect;
            scrValue.maButton1Rect = maBtn1Rect;
            scrValue.maButton2Rect = maBtn2Rect;
            scrValue.mnButton1State = ((mnStateFlags & SCRBAR_STATE_BTN1_DOWN) ? CTRL_STATE_PRESSED : 0) |
                                ((!(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE)) ? CTRL_STATE_ENABLED : 0);
            scrValue.mnButton2State = ((mnStateFlags & SCRBAR_STATE_BTN2_DOWN) ? CTRL_STATE_PRESSED : 0) |
                                ((!(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE)) ? CTRL_STATE_ENABLED : 0);
            scrValue.mnThumbState = nState | ((mnStateFlags & SCRBAR_STATE_THUMB_DOWN) ? CTRL_STATE_PRESSED : 0);
            scrValue.mnPage1State = nState | ((mnStateFlags & SCRBAR_STATE_PAGE1_DOWN) ? CTRL_STATE_PRESSED : 0);
            scrValue.mnPage2State = nState | ((mnStateFlags & SCRBAR_STATE_PAGE2_DOWN) ? CTRL_STATE_PRESSED : 0);

            if( IsMouseOver() )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                if( pRect )
                {
                    if( pRect == &maThumbRect )
                        scrValue.mnThumbState |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maBtn1Rect )
                        scrValue.mnButton1State |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maBtn2Rect )
                        scrValue.mnButton2State |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maPage1Rect )
                        scrValue.mnPage1State |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maPage2Rect )
                        scrValue.mnPage2State |= CTRL_STATE_ROLLOVER;
                }
            }

            Rectangle aCtrlRegion;
            aCtrlRegion.Union( maBtn1Rect );
            aCtrlRegion.Union( maBtn2Rect );
            aCtrlRegion.Union( maPage1Rect );
            aCtrlRegion.Union( maPage2Rect );
            aCtrlRegion.Union( maThumbRect );
            bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, (bHorz ? PART_DRAW_BACKGROUND_HORZ : PART_DRAW_BACKGROUND_VERT),
                            aCtrlRegion, nState, scrValue, rtl::OUString() );
        }
        else
      {
        if ( (nDrawFlags & SCRBAR_DRAW_PAGE1) || (nDrawFlags & SCRBAR_DRAW_PAGE2) )
        {
            sal_uInt32  part1 = bHorz ? PART_TRACK_HORZ_LEFT : PART_TRACK_VERT_UPPER;
            sal_uInt32  part2 = bHorz ? PART_TRACK_HORZ_RIGHT : PART_TRACK_VERT_LOWER;
            Rectangle   aCtrlRegion1( maPage1Rect );
            Rectangle   aCtrlRegion2( maPage2Rect );
            ControlState nState1 = (IsEnabled() ? CTRL_STATE_ENABLED : 0) | (HasFocus() ? CTRL_STATE_FOCUSED : 0);
            ControlState nState2 = nState1;

            nState1 |= ((mnStateFlags & SCRBAR_STATE_PAGE1_DOWN) ? CTRL_STATE_PRESSED : 0);
            nState2 |= ((mnStateFlags & SCRBAR_STATE_PAGE2_DOWN) ? CTRL_STATE_PRESSED : 0);

            if( IsMouseOver() )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                if( pRect )
                {
                    if( pRect == &maPage1Rect )
                        nState1 |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maPage2Rect )
                        nState2 |= CTRL_STATE_ROLLOVER;
                }
            }

            if ( nDrawFlags & SCRBAR_DRAW_PAGE1 )
                bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, part1, aCtrlRegion1, nState1,
                                scrValue, rtl::OUString() );

            if ( nDrawFlags & SCRBAR_DRAW_PAGE2 )
                bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, part2, aCtrlRegion2, nState2,
                                scrValue, rtl::OUString() );
        }
        if ( (nDrawFlags & SCRBAR_DRAW_BTN1) || (nDrawFlags & SCRBAR_DRAW_BTN2) )
        {
            sal_uInt32  part1 = bHorz ? PART_BUTTON_LEFT : PART_BUTTON_UP;
            sal_uInt32  part2 = bHorz ? PART_BUTTON_RIGHT : PART_BUTTON_DOWN;
            Rectangle   aCtrlRegion1( maBtn1Rect );
            Rectangle   aCtrlRegion2( maBtn2Rect );
            ControlState nState1 = HasFocus() ? CTRL_STATE_FOCUSED : 0;
            ControlState nState2 = nState1;

            if ( !Window::IsEnabled() || !IsEnabled() )
                nState1 = (nState2 &= ~CTRL_STATE_ENABLED);
            else
                nState1 = (nState2 |= CTRL_STATE_ENABLED);

            nState1 |= ((mnStateFlags & SCRBAR_STATE_BTN1_DOWN) ? CTRL_STATE_PRESSED : 0);
            nState2 |= ((mnStateFlags & SCRBAR_STATE_BTN2_DOWN) ? CTRL_STATE_PRESSED : 0);

            if(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE)
                nState1 &= ~CTRL_STATE_ENABLED;
            if(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE)
                nState2 &= ~CTRL_STATE_ENABLED;

            if( IsMouseOver() )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                if( pRect )
                {
                    if( pRect == &maBtn1Rect )
                        nState1 |= CTRL_STATE_ROLLOVER;
                    else if( pRect == &maBtn2Rect )
                        nState2 |= CTRL_STATE_ROLLOVER;
                }
            }

            if ( nDrawFlags & SCRBAR_DRAW_BTN1 )
                bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, part1, aCtrlRegion1, nState1,
                                scrValue, rtl::OUString() );

            if ( nDrawFlags & SCRBAR_DRAW_BTN2 )
                bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, part2, aCtrlRegion2, nState2,
                                scrValue, rtl::OUString() );
        }
        if ( (nDrawFlags & SCRBAR_DRAW_THUMB) && !maThumbRect.IsEmpty() )
        {
            ControlState    nState = IsEnabled() ? CTRL_STATE_ENABLED : 0;
            Rectangle       aCtrlRegion( maThumbRect );

            if ( mnStateFlags & SCRBAR_STATE_THUMB_DOWN )
                nState |= CTRL_STATE_PRESSED;

            if ( HasFocus() )
                nState |= CTRL_STATE_FOCUSED;

            if( IsMouseOver() )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                if( pRect )
                {
                    if( pRect == &maThumbRect )
                        nState |= CTRL_STATE_ROLLOVER;
                }
            }

            bNativeOK = DrawNativeControl( CTRL_SCROLLBAR, (bHorz ? PART_THUMB_HORZ : PART_THUMB_VERT),
                    aCtrlRegion, nState, scrValue, rtl::OUString() );
        }
      }
    }
    return bNativeOK;
}

void ScrollBar::ImplDraw( sal_uInt16 nDrawFlags, OutputDevice* pOutDev )
{
    DecorationView          aDecoView( pOutDev );
    Rectangle               aTempRect;
    sal_uInt16                  nStyle;
    const StyleSettings&    rStyleSettings = pOutDev->GetSettings().GetStyleSettings();
    SymbolType              eSymbolType;
    sal_Bool                    bEnabled = IsEnabled();

    // Evt. noch offene Berechnungen nachholen
    if ( mbCalcSize )
        ImplCalc( sal_False );

    Window *pWin = NULL;
    if( pOutDev->GetOutDevType() == OUTDEV_WINDOW )
        pWin = (Window*) pOutDev;

    // Draw the entire control if the native theme engine needs it
    if ( nDrawFlags && pWin && pWin->IsNativeControlSupported(CTRL_SCROLLBAR, PART_DRAW_BACKGROUND_HORZ) )
    {
        ImplDrawNative( SCRBAR_DRAW_BACKGROUND );
        return;
    }

    if( (nDrawFlags & SCRBAR_DRAW_BTN1) && (!pWin || !ImplDrawNative( SCRBAR_DRAW_BTN1 ) ) )
    {
        nStyle = BUTTON_DRAW_NOLIGHTBORDER;
        if ( mnStateFlags & SCRBAR_STATE_BTN1_DOWN )
            nStyle |= BUTTON_DRAW_PRESSED;
        aTempRect = aDecoView.DrawButton( maBtn1Rect, nStyle );
        ImplCalcSymbolRect( aTempRect );
        nStyle = 0;
        if ( (mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) || !bEnabled )
            nStyle |= SYMBOL_DRAW_DISABLE;
        if ( rStyleSettings.GetOptions() & STYLE_OPTION_SCROLLARROW )
        {
            if ( GetStyle() & WB_HORZ )
                eSymbolType = SYMBOL_ARROW_LEFT;
            else
                eSymbolType = SYMBOL_ARROW_UP;
        }
        else
        {
            if ( GetStyle() & WB_HORZ )
                eSymbolType = SYMBOL_SPIN_LEFT;
            else
                eSymbolType = SYMBOL_SPIN_UP;
        }
        aDecoView.DrawSymbol( aTempRect, eSymbolType, rStyleSettings.GetButtonTextColor(), nStyle );
    }

    if ( (nDrawFlags & SCRBAR_DRAW_BTN2) && (!pWin || !ImplDrawNative( SCRBAR_DRAW_BTN2 ) ) )
    {
        nStyle = BUTTON_DRAW_NOLIGHTBORDER;
        if ( mnStateFlags & SCRBAR_STATE_BTN2_DOWN )
            nStyle |= BUTTON_DRAW_PRESSED;
        aTempRect = aDecoView.DrawButton(  maBtn2Rect, nStyle );
        ImplCalcSymbolRect( aTempRect );
        nStyle = 0;
        if ( (mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) || !bEnabled )
            nStyle |= SYMBOL_DRAW_DISABLE;
        if ( rStyleSettings.GetOptions() & STYLE_OPTION_SCROLLARROW )
        {
            if ( GetStyle() & WB_HORZ )
                eSymbolType = SYMBOL_ARROW_RIGHT;
            else
                eSymbolType = SYMBOL_ARROW_DOWN;
        }
        else
        {
            if ( GetStyle() & WB_HORZ )
                eSymbolType = SYMBOL_SPIN_RIGHT;
            else
                eSymbolType = SYMBOL_SPIN_DOWN;
        }
        aDecoView.DrawSymbol( aTempRect, eSymbolType, rStyleSettings.GetButtonTextColor(), nStyle );
    }

    pOutDev->SetLineColor();

    if ( (nDrawFlags & SCRBAR_DRAW_THUMB) && (!pWin || !ImplDrawNative( SCRBAR_DRAW_THUMB ) ) )
    {
        if ( !maThumbRect.IsEmpty() )
        {
            if ( bEnabled )
            {
                nStyle = BUTTON_DRAW_NOLIGHTBORDER;
                aTempRect = aDecoView.DrawButton( maThumbRect, nStyle );
            }
            else
            {
                pOutDev->SetFillColor( rStyleSettings.GetCheckedColor() );
                pOutDev->DrawRect( maThumbRect );
            }
        }
    }

    if ( (nDrawFlags & SCRBAR_DRAW_PAGE1) && (!pWin || !ImplDrawNative( SCRBAR_DRAW_PAGE1 ) ) )
    {
        if ( mnStateFlags & SCRBAR_STATE_PAGE1_DOWN )
            pOutDev->SetFillColor( rStyleSettings.GetShadowColor() );
        else
            pOutDev->SetFillColor( rStyleSettings.GetCheckedColor() );
        pOutDev->DrawRect( maPage1Rect );
    }
    if ( (nDrawFlags & SCRBAR_DRAW_PAGE2) && (!pWin || !ImplDrawNative( SCRBAR_DRAW_PAGE2 ) ) )
    {
        if ( mnStateFlags & SCRBAR_STATE_PAGE2_DOWN )
            pOutDev->SetFillColor( rStyleSettings.GetShadowColor() );
        else
            pOutDev->SetFillColor( rStyleSettings.GetCheckedColor() );
        pOutDev->DrawRect( maPage2Rect );
    }
}

// -----------------------------------------------------------------------

long ScrollBar::ImplScroll( long nNewPos, sal_Bool bCallEndScroll )
{
    long nOldPos = mnThumbPos;
    SetThumbPos( nNewPos );
    long nDelta = mnThumbPos-nOldPos;
    if ( nDelta )
    {
        mnDelta = nDelta;
        Scroll();
        if ( bCallEndScroll )
            EndScroll();
        mnDelta = 0;
    }
    return nDelta;
}

// -----------------------------------------------------------------------

long ScrollBar::ImplDoAction( sal_Bool bCallEndScroll )
{
    long nDelta = 0;

    switch ( meScrollType )
    {
        case SCROLL_LINEUP:
            nDelta = ImplScroll( mnThumbPos-mnLineSize, bCallEndScroll );
            break;

        case SCROLL_LINEDOWN:
            nDelta = ImplScroll( mnThumbPos+mnLineSize, bCallEndScroll );
            break;

        case SCROLL_PAGEUP:
            nDelta = ImplScroll( mnThumbPos-mnPageSize, bCallEndScroll );
            break;

        case SCROLL_PAGEDOWN:
            nDelta = ImplScroll( mnThumbPos+mnPageSize, bCallEndScroll );
            break;
        default:
            ;
    }

    return nDelta;
}

// -----------------------------------------------------------------------

void ScrollBar::ImplDoMouseAction( const Point& rMousePos, sal_Bool bCallAction )
{
    sal_uInt16  nOldStateFlags = mnStateFlags;
    sal_Bool    bAction = sal_False;
    sal_Bool    bHorizontal = ( GetStyle() & WB_HORZ )? sal_True: sal_False;
    sal_Bool    bIsInside = sal_False;

    Point aPoint( 0, 0 );
    Rectangle aControlRegion( aPoint, GetOutputSizePixel() );

    switch ( meScrollType )
    {
        case SCROLL_LINEUP:
            if ( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? (IsRTLEnabled()? PART_BUTTON_RIGHT: PART_BUTTON_LEFT): PART_BUTTON_UP,
                        aControlRegion, rMousePos, bIsInside )?
                    bIsInside:
                    maBtn1Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_BTN1_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_BTN1_DOWN;
            break;

        case SCROLL_LINEDOWN:
            if ( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? (IsRTLEnabled()? PART_BUTTON_LEFT: PART_BUTTON_RIGHT): PART_BUTTON_DOWN,
                        aControlRegion, rMousePos, bIsInside )?
                    bIsInside:
                    maBtn2Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_BTN2_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_BTN2_DOWN;
            break;

        case SCROLL_PAGEUP:
            // HitTestNativeControl, see remark at top of file
            if ( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? PART_TRACK_HORZ_LEFT: PART_TRACK_VERT_UPPER,
                                       maPage1Rect, rMousePos, bIsInside )?
                    bIsInside:
                    maPage1Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_PAGE1_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_PAGE1_DOWN;
            break;

        case SCROLL_PAGEDOWN:
            // HitTestNativeControl, see remark at top of file
            if ( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? PART_TRACK_HORZ_RIGHT: PART_TRACK_VERT_LOWER,
                                       maPage2Rect, rMousePos, bIsInside )?
                    bIsInside:
                    maPage2Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_PAGE2_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_PAGE2_DOWN;
            break;
        default:
            ;
    }

    if ( nOldStateFlags != mnStateFlags )
        ImplDraw( mnDragDraw, this );
    if ( bAction )
        ImplDoAction( sal_False );
}

// -----------------------------------------------------------------------

void ScrollBar::ImplDragThumb( const Point& rMousePos )
{
    long nMovePix;
    if ( GetStyle() & WB_HORZ )
        nMovePix = rMousePos.X()-(maThumbRect.Left()+mnMouseOff);
    else
        nMovePix = rMousePos.Y()-(maThumbRect.Top()+mnMouseOff);

    // move thumb if necessary
    if ( nMovePix )
    {
        mnThumbPixPos += nMovePix;
        if ( mnThumbPixPos < 0 )
            mnThumbPixPos = 0;
        if ( mnThumbPixPos > (mnThumbPixRange-mnThumbPixSize) )
            mnThumbPixPos = mnThumbPixRange-mnThumbPixSize;
        long nOldPos = mnThumbPos;
        mnThumbPos = ImplCalcThumbPos( mnThumbPixPos );
        ImplUpdateRects();
        if ( mbFullDrag && (nOldPos != mnThumbPos) )
        {
            mnDelta = mnThumbPos-nOldPos;
            Scroll();
            mnDelta = 0;
        }
    }
}

// -----------------------------------------------------------------------

void ScrollBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() || rMEvt.IsMiddle() )
    {
        const Point&    rMousePos = rMEvt.GetPosPixel();
        sal_uInt16          nTrackFlags = 0;
        sal_Bool            bHorizontal = ( GetStyle() & WB_HORZ )? sal_True: sal_False;
        sal_Bool            bIsInside = sal_False;
        sal_Bool            bDragToMouse = sal_False;

        Point aPoint( 0, 0 );
        Rectangle aControlRegion( aPoint, GetOutputSizePixel() );

        if ( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? (IsRTLEnabled()? PART_BUTTON_RIGHT: PART_BUTTON_LEFT): PART_BUTTON_UP,
                    aControlRegion, rMousePos, bIsInside )?
                bIsInside:
                maBtn1Rect.IsInside( rMousePos ) )
        {
            if ( !(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) )
            {
                nTrackFlags     = STARTTRACK_BUTTONREPEAT;
                meScrollType    = SCROLL_LINEUP;
                mnDragDraw      = SCRBAR_DRAW_BTN1;
            }
        }
        else if ( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? (IsRTLEnabled()? PART_BUTTON_LEFT: PART_BUTTON_RIGHT): PART_BUTTON_DOWN,
                    aControlRegion, rMousePos, bIsInside )?
                bIsInside:
                maBtn2Rect.IsInside( rMousePos ) )
        {
            if ( !(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) )
            {
                nTrackFlags     = STARTTRACK_BUTTONREPEAT;
                meScrollType    = SCROLL_LINEDOWN;
                mnDragDraw      = SCRBAR_DRAW_BTN2;
            }
        }
        else
        {
            bool bThumbHit = HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? PART_THUMB_HORZ : PART_THUMB_VERT,
                                                   maThumbRect, rMousePos, bIsInside )
                             ? bIsInside : maThumbRect.IsInside( rMousePos );
            bool bDragHandling = rMEvt.IsMiddle() || bThumbHit || ImplGetSVData()->maNWFData.mbScrollbarJumpPage;
            if( bDragHandling )
            {
                if( mpData )
                {
                    mpData->mbHide = sal_True;  // disable focus blinking
                    if( HasFocus() )
                        ImplDraw( SCRBAR_DRAW_THUMB, this ); // paint without focus
                }

                if ( mnVisibleSize < mnMaxRange-mnMinRange )
                {
                    nTrackFlags     = 0;
                    meScrollType    = SCROLL_DRAG;
                    mnDragDraw      = SCRBAR_DRAW_THUMB;

                    // calculate mouse offset
                    if( rMEvt.IsMiddle() || (ImplGetSVData()->maNWFData.mbScrollbarJumpPage && !bThumbHit) )
                    {
                        bDragToMouse = sal_True;
                        if ( GetStyle() & WB_HORZ )
                            mnMouseOff = maThumbRect.GetWidth()/2;
                        else
                            mnMouseOff = maThumbRect.GetHeight()/2;
                    }
                    else
                    {
                        if ( GetStyle() & WB_HORZ )
                            mnMouseOff = rMousePos.X()-maThumbRect.Left();
                        else
                            mnMouseOff = rMousePos.Y()-maThumbRect.Top();
                    }

                    mnStateFlags |= SCRBAR_STATE_THUMB_DOWN;
                    ImplDraw( mnDragDraw, this );
                }
            }
            else if( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? PART_TRACK_HORZ_AREA : PART_TRACK_VERT_AREA,
                                           aControlRegion, rMousePos, bIsInside )?
                bIsInside : sal_True )
            {
                nTrackFlags = STARTTRACK_BUTTONREPEAT;

                // HitTestNativeControl, see remark at top of file
                if ( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? PART_TRACK_HORZ_LEFT : PART_TRACK_VERT_UPPER,
                                           maPage1Rect, rMousePos, bIsInside )?
                    bIsInside:
                    maPage1Rect.IsInside( rMousePos ) )
                {
                    meScrollType    = SCROLL_PAGEUP;
                    mnDragDraw      = SCRBAR_DRAW_PAGE1;
                }
                else
                {
                    meScrollType    = SCROLL_PAGEDOWN;
                    mnDragDraw      = SCRBAR_DRAW_PAGE2;
                }
            }
        }

        // Soll Tracking gestartet werden
        if ( meScrollType != SCROLL_DONTKNOW )
        {
            // remember original position in case of abort or EndScroll-Delta
            mnStartPos = mnThumbPos;
            // #92906# Call StartTracking() before ImplDoMouseAction(), otherwise
            // MouseButtonUp() / EndTracking() may be called if somebody is spending
            // a lot of time in the scroll handler
            StartTracking( nTrackFlags );
            ImplDoMouseAction( rMousePos );

            if( bDragToMouse )
                ImplDragThumb( rMousePos );
        }
    }
}

// -----------------------------------------------------------------------

void ScrollBar::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        // Button und PageRect-Status wieder herstellen
        sal_uInt16 nOldStateFlags = mnStateFlags;
        mnStateFlags &= ~(SCRBAR_STATE_BTN1_DOWN | SCRBAR_STATE_BTN2_DOWN |
                          SCRBAR_STATE_PAGE1_DOWN | SCRBAR_STATE_PAGE2_DOWN |
                          SCRBAR_STATE_THUMB_DOWN);
        if ( nOldStateFlags != mnStateFlags )
            ImplDraw( mnDragDraw, this );
        mnDragDraw = 0;

        // Bei Abbruch, die alte ThumbPosition wieder herstellen
        if ( rTEvt.IsTrackingCanceled() )
        {
            long nOldPos = mnThumbPos;
            SetThumbPos( mnStartPos );
            mnDelta = mnThumbPos-nOldPos;
            Scroll();
        }

        if ( meScrollType == SCROLL_DRAG )
        {
            // Wenn gedragt wurde, berechnen wir den Thumb neu, damit
            // er wieder auf einer gerundeten ThumbPosition steht
            ImplCalc();

            if ( !mbFullDrag && (mnStartPos != mnThumbPos) )
            {
                mnDelta = mnThumbPos-mnStartPos;
                Scroll();
                mnDelta = 0;
            }
        }

        mnDelta = mnThumbPos-mnStartPos;
        EndScroll();
        mnDelta = 0;
        meScrollType = SCROLL_DONTKNOW;

        if( mpData )
            mpData->mbHide = sal_False; // re-enable focus blinking
    }
    else
    {
        const Point rMousePos = rTEvt.GetMouseEvent().GetPosPixel();

        // Dragging wird speziell behandelt
        if ( meScrollType == SCROLL_DRAG )
            ImplDragThumb( rMousePos );
        else
            ImplDoMouseAction( rMousePos, rTEvt.IsTrackingRepeat() );

        // Wenn ScrollBar-Werte so umgesetzt wurden, das es nichts
        // mehr zum Tracking gibt, dann berechen wir hier ab
        if ( !IsVisible() || (mnVisibleSize >= (mnMaxRange-mnMinRange)) )
            EndTracking();
    }
}

// -----------------------------------------------------------------------

void ScrollBar::KeyInput( const KeyEvent& rKEvt )
{
    if ( !rKEvt.GetKeyCode().GetModifier() )
    {
        switch ( rKEvt.GetKeyCode().GetCode() )
        {
            case KEY_HOME:
                DoScroll( 0 );
                break;

            case KEY_END:
                DoScroll( GetRangeMax() );
                break;

            case KEY_LEFT:
            case KEY_UP:
                DoScrollAction( SCROLL_LINEUP );
                break;

            case KEY_RIGHT:
            case KEY_DOWN:
                DoScrollAction( SCROLL_LINEDOWN );
                break;

            case KEY_PAGEUP:
                DoScrollAction( SCROLL_PAGEUP );
                break;

            case KEY_PAGEDOWN:
                DoScrollAction( SCROLL_PAGEDOWN );
                break;

            default:
                Control::KeyInput( rKEvt );
                break;
        }
    }
    else
        Control::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void ScrollBar::Paint( const Rectangle& )
{
    ImplDraw( SCRBAR_DRAW_ALL, this );
}

// -----------------------------------------------------------------------

void ScrollBar::Resize()
{
    Control::Resize();
    mbCalcSize = sal_True;
    if ( IsReallyVisible() )
        ImplCalc( sal_False );
    Invalidate();
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ScrollBar, ImplAutoTimerHdl)
{
    if( mpData && mpData->mbHide )
        return 0;
    ImplInvert();
    return 0;
}

void ScrollBar::ImplInvert()
{
    Rectangle aRect( maThumbRect );
    if( aRect.getWidth() > 4 )
    {
        aRect.Left() += 2;
        aRect.Right() -= 2;
    }
    if( aRect.getHeight() > 4 )
    {
        aRect.Top() += 2;
        aRect.Bottom() -= 2;
    }

    Invert( aRect, 0 );
}

// -----------------------------------------------------------------------

void ScrollBar::GetFocus()
{
    if( !mpData )
    {
        mpData = new ImplScrollBarData;
        mpData->maTimer.SetTimeoutHdl( LINK( this, ScrollBar, ImplAutoTimerHdl ) );
        mpData->mbHide = sal_False;
    }
    ImplInvert();   // react immediately
    mpData->maTimer.SetTimeout( GetSettings().GetStyleSettings().GetCursorBlinkTime() );
    mpData->maTimer.Start();
    Control::GetFocus();
}

// -----------------------------------------------------------------------

void ScrollBar::LoseFocus()
{
    if( mpData )
        mpData->maTimer.Stop();
    ImplDraw( SCRBAR_DRAW_THUMB, this );

    Control::LoseFocus();
}

// -----------------------------------------------------------------------

void ScrollBar::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
        ImplCalc( sal_False );
    else if ( nType == STATE_CHANGE_DATA )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            ImplCalc( sal_True );
    }
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            ImplCalc( sal_False );
            Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_ENABLE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        ImplInitStyle( GetStyle() );
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            if ( (GetPrevStyle() & SCRBAR_VIEW_STYLE) !=
                 (GetStyle() & SCRBAR_VIEW_STYLE) )
            {
                mbCalcSize = sal_True;
                ImplCalc( sal_False );
                Invalidate();
            }
        }
    }
}

// -----------------------------------------------------------------------

void ScrollBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        mbCalcSize = sal_True;
        ImplCalc( sal_False );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

Rectangle* ScrollBar::ImplFindPartRect( const Point& rPt )
{
    sal_Bool    bHorizontal = ( GetStyle() & WB_HORZ )? sal_True: sal_False;
    sal_Bool    bIsInside = sal_False;

    Point aPoint( 0, 0 );
    Rectangle aControlRegion( aPoint, GetOutputSizePixel() );

    if( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? (IsRTLEnabled()? PART_BUTTON_RIGHT: PART_BUTTON_LEFT): PART_BUTTON_UP,
                aControlRegion, rPt, bIsInside )?
            bIsInside:
            maBtn1Rect.IsInside( rPt ) )
        return &maBtn1Rect;
    else if( HitTestNativeControl( CTRL_SCROLLBAR, bHorizontal? (IsRTLEnabled()? PART_BUTTON_LEFT: PART_BUTTON_RIGHT): PART_BUTTON_DOWN,
                aControlRegion, rPt, bIsInside )?
            bIsInside:
            maBtn2Rect.IsInside( rPt ) )
        return &maBtn2Rect;
    // HitTestNativeControl, see remark at top of file
    else if( HitTestNativeControl( CTRL_SCROLLBAR,  bHorizontal ? PART_TRACK_HORZ_LEFT : PART_TRACK_VERT_UPPER,
                maPage1Rect, rPt, bIsInside)?
            bIsInside:
            maPage1Rect.IsInside( rPt ) )
        return &maPage1Rect;
    // HitTestNativeControl, see remark at top of file
    else if( HitTestNativeControl( CTRL_SCROLLBAR,  bHorizontal ? PART_TRACK_HORZ_RIGHT : PART_TRACK_VERT_LOWER,
                maPage2Rect, rPt, bIsInside)?
            bIsInside:
            maPage2Rect.IsInside( rPt ) )
        return &maPage2Rect;
    // HitTestNativeControl, see remark at top of file
    else if( HitTestNativeControl( CTRL_SCROLLBAR,  bHorizontal ? PART_THUMB_HORZ : PART_THUMB_VERT,
                maThumbRect, rPt, bIsInside)?
             bIsInside:
             maThumbRect.IsInside( rPt ) )
        return &maThumbRect;
    else
        return NULL;
}

long ScrollBar::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    const MouseEvent* pMouseEvt = NULL;

    if( (rNEvt.GetType() == EVENT_MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != NULL )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // trigger redraw if mouse over state has changed
            if( IsNativeControlSupported(CTRL_SCROLLBAR, PART_ENTIRE_CONTROL) )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                Rectangle* pLastRect = ImplFindPartRect( GetLastPointerPosPixel() );
                if( pRect != pLastRect || pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow() )
                {
                    Region aRgn( GetActiveClipRegion() );
                    Region aClipRegion;

                    if ( pRect )
                        aClipRegion.Union( *pRect );
                    if ( pLastRect )
                        aClipRegion.Union( *pLastRect );

                    // Support for 3-button scroll bars
                    sal_Bool bHas3Buttons = IsNativeControlSupported( CTRL_SCROLLBAR, HAS_THREE_BUTTONS );
                    if ( bHas3Buttons && ( pRect == &maBtn1Rect || pLastRect == &maBtn1Rect ) )
                    {
                        aClipRegion.Union( maBtn2Rect );
                    }

                    SetClipRegion( aClipRegion );
                    Paint( aClipRegion.GetBoundRect() );

                    SetClipRegion( aRgn );
                }
            }
        }
    }

    return nDone ? nDone : Control::PreNotify(rNEvt);
}

// -----------------------------------------------------------------------

void ScrollBar::Scroll()
{
    ImplCallEventListenersAndHandler( VCLEVENT_SCROLLBAR_SCROLL, maScrollHdl, this );
}

// -----------------------------------------------------------------------

void ScrollBar::EndScroll()
{
    ImplCallEventListenersAndHandler( VCLEVENT_SCROLLBAR_ENDSCROLL, maEndScrollHdl, this );
}

// -----------------------------------------------------------------------

long ScrollBar::DoScroll( long nNewPos )
{
    if ( meScrollType != SCROLL_DONTKNOW )
        return 0;

    meScrollType = SCROLL_DRAG;
    long nDelta = ImplScroll( nNewPos, sal_True );
    meScrollType = SCROLL_DONTKNOW;
    return nDelta;
}

// -----------------------------------------------------------------------

long ScrollBar::DoScrollAction( ScrollType eScrollType )
{
    if ( (meScrollType != SCROLL_DONTKNOW) ||
         (eScrollType == SCROLL_DONTKNOW) ||
         (eScrollType == SCROLL_DRAG) )
        return 0;

    meScrollType = eScrollType;
    long nDelta = ImplDoAction( sal_True );
    meScrollType = SCROLL_DONTKNOW;
    return nDelta;
}

// -----------------------------------------------------------------------

void ScrollBar::SetRangeMin( long nNewRange )
{
    SetRange( Range( nNewRange, GetRangeMax() ) );
}

// -----------------------------------------------------------------------

void ScrollBar::SetRangeMax( long nNewRange )
{
    SetRange( Range( GetRangeMin(), nNewRange ) );
}

// -----------------------------------------------------------------------

void ScrollBar::SetRange( const Range& rRange )
{
    // Range einpassen
    Range aRange = rRange;
    aRange.Justify();
    long nNewMinRange = aRange.Min();
    long nNewMaxRange = aRange.Max();

    // Wenn Range sich unterscheidet, dann neuen setzen
    if ( (mnMinRange != nNewMinRange) ||
         (mnMaxRange != nNewMaxRange) )
    {
        mnMinRange = nNewMinRange;
        mnMaxRange = nNewMaxRange;

        // Thumb einpassen
        if ( mnThumbPos > mnMaxRange-mnVisibleSize )
            mnThumbPos = mnMaxRange-mnVisibleSize;
        if ( mnThumbPos < mnMinRange )
            mnThumbPos = mnMinRange;

        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void ScrollBar::SetThumbPos( long nNewThumbPos )
{
    if ( nNewThumbPos > mnMaxRange-mnVisibleSize )
        nNewThumbPos = mnMaxRange-mnVisibleSize;
    if ( nNewThumbPos < mnMinRange )
        nNewThumbPos = mnMinRange;

    if ( mnThumbPos != nNewThumbPos )
    {
        mnThumbPos = nNewThumbPos;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void ScrollBar::SetVisibleSize( long nNewSize )
{
    if ( mnVisibleSize != nNewSize )
    {
        mnVisibleSize = nNewSize;

        // Thumb einpassen
        if ( mnThumbPos > mnMaxRange-mnVisibleSize )
            mnThumbPos = mnMaxRange-mnVisibleSize;
        if ( mnThumbPos < mnMinRange )
            mnThumbPos = mnMinRange;
        StateChanged( STATE_CHANGE_DATA );
    }
}

Size ScrollBar::GetOptimalSize(WindowSizeType) const
{
    Rectangle aCtrlRegion;
    aCtrlRegion.Union(maBtn1Rect);
    aCtrlRegion.Union(maBtn2Rect);
    aCtrlRegion.Union(maPage1Rect);
    aCtrlRegion.Union(maPage2Rect);
    aCtrlRegion.Union(maThumbRect);
    return aCtrlRegion.GetSize();
}

// =======================================================================

void ScrollBarBox::ImplInit( Window* pParent, WinBits nStyle )
{
    Window::ImplInit( pParent, nStyle, NULL );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    long nScrollSize = rStyleSettings.GetScrollBarSize();
    SetSizePixel( Size( nScrollSize, nScrollSize ) );
    ImplInitSettings();
}

// -----------------------------------------------------------------------

ScrollBarBox::ScrollBarBox( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_SCROLLBARBOX )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

void ScrollBarBox::ImplInitSettings()
{
    // Hack, damit man auch DockingWindows ohne Hintergrund bauen kann
    // und noch nicht alles umgestellt ist
    if ( IsBackground() )
    {
        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else
            aColor = GetSettings().GetStyleSettings().GetFaceColor();
        SetBackground( aColor );
    }
}

// -----------------------------------------------------------------------

void ScrollBarBox::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void ScrollBarBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
