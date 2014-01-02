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


#include <tools/rc.h>
#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/slider.hxx>
#include <vcl/settings.hxx>

#include "thumbpos.hxx"

// =======================================================================

#define SLIDER_DRAW_THUMB           ((sal_uInt16)0x0001)
#define SLIDER_DRAW_CHANNEL1        ((sal_uInt16)0x0002)
#define SLIDER_DRAW_CHANNEL2        ((sal_uInt16)0x0004)
#define SLIDER_DRAW_CHANNEL         (SLIDER_DRAW_CHANNEL1 | SLIDER_DRAW_CHANNEL2)
#define SLIDER_DRAW_ALL             (SLIDER_DRAW_THUMB | SLIDER_DRAW_CHANNEL)

#define SLIDER_STATE_CHANNEL1_DOWN  ((sal_uInt16)0x0001)
#define SLIDER_STATE_CHANNEL2_DOWN  ((sal_uInt16)0x0002)
#define SLIDER_STATE_THUMB_DOWN     ((sal_uInt16)0x0004)

#define SLIDER_THUMB_SIZE           9
#define SLIDER_THUMB_HALFSIZE       4
#define SLIDER_CHANNEL_OFFSET       0
#define SLIDER_CHANNEL_SIZE         4
#define SLIDER_CHANNEL_HALFSIZE     2

#define SLIDER_HEIGHT               16

#define SLIDER_VIEW_STYLE           (WB_3DLOOK | WB_HORZ | WB_VERT)

// =======================================================================

void Slider::ImplInit( Window* pParent, WinBits nStyle )
{
    mnThumbPixOffset    = 0;
    mnThumbPixRange     = 0;
    mnThumbPixPos       = 0;    // between mnThumbPixOffset and mnThumbPixOffset+mnThumbPixRange
    mnChannelPixOffset  = 0;
    mnChannelPixRange   = 0;
    mnChannelPixTop     = 0;
    mnChannelPixBottom  = 0;

    mnMinRange          = 0;
    mnMaxRange          = 100;
    mnThumbPos          = 0;
    mnLineSize          = 1;
    mnPageSize          = 1;
    mnDelta             = 0;
    mnDragDraw          = 0;
    mnStateFlags        = 0;
    meScrollType        = SCROLL_DONTKNOW;
    mbCalcSize          = sal_True;
    mbFullDrag          = sal_True;

    Control::ImplInit( pParent, nStyle, NULL );

    ImplInitSettings();
    SetSizePixel( CalcWindowSizePixel() );
}

// -----------------------------------------------------------------------

Slider::Slider( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_SLIDER )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

void Slider::ImplInitSettings()
{
    Window* pParent = GetParent();
    if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
    {
        EnableChildTransparentMode( sal_True );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( sal_True );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( sal_False );
        SetParentClipMode( 0 );
        SetPaintTransparent( sal_False );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

// -----------------------------------------------------------------------

void Slider::ImplUpdateRects( sal_Bool bUpdate )
{
    Rectangle aOldThumbRect = maThumbRect;
    bool bInvalidateAll = false;

    if ( mnThumbPixRange )
    {
        if ( GetStyle() & WB_HORZ )
        {
            maThumbRect.Left()      = mnThumbPixPos-SLIDER_THUMB_HALFSIZE;
            maThumbRect.Right()     = maThumbRect.Left()+SLIDER_THUMB_SIZE-1;
            if ( mnChannelPixOffset < maThumbRect.Left() )
            {
                maChannel1Rect.Left()   = mnChannelPixOffset;
                maChannel1Rect.Right()  = maThumbRect.Left()-1;
                maChannel1Rect.Top()    = mnChannelPixTop;
                maChannel1Rect.Bottom() = mnChannelPixBottom;
            }
            else
                maChannel1Rect.SetEmpty();
            if ( mnChannelPixOffset+mnChannelPixRange-1 > maThumbRect.Right() )
            {
                maChannel2Rect.Left()   = maThumbRect.Right()+1;
                maChannel2Rect.Right()  = mnChannelPixOffset+mnChannelPixRange-1;
                maChannel2Rect.Top()    = mnChannelPixTop;
                maChannel2Rect.Bottom() = mnChannelPixBottom;
            }
            else
                maChannel2Rect.SetEmpty();

            const Rectangle aControlRegion( Rectangle( Point(0,0), Size( SLIDER_THUMB_SIZE, 10 ) ) );
            Rectangle aThumbBounds, aThumbContent;
            if ( GetNativeControlRegion( CTRL_SLIDER, PART_THUMB_HORZ,
                                         aControlRegion, 0, ImplControlValue(), OUString(),
                                         aThumbBounds, aThumbContent ) )
            {
                maThumbRect.Left() = mnThumbPixPos - aThumbBounds.GetWidth()/2;
                maThumbRect.Right() = maThumbRect.Left() + aThumbBounds.GetWidth() - 1;
                bInvalidateAll = true;
            }
        }
        else
        {
            maThumbRect.Top()       = mnThumbPixPos-SLIDER_THUMB_HALFSIZE;
            maThumbRect.Bottom()    = maThumbRect.Top()+SLIDER_THUMB_SIZE-1;
            if ( mnChannelPixOffset < maThumbRect.Top() )
            {
                maChannel1Rect.Top()    = mnChannelPixOffset;
                maChannel1Rect.Bottom() = maThumbRect.Top()-1;
                maChannel1Rect.Left()   = mnChannelPixTop;
                maChannel1Rect.Right()  = mnChannelPixBottom;
            }
            else
                maChannel1Rect.SetEmpty();
            if ( mnChannelPixOffset+mnChannelPixRange-1 > maThumbRect.Bottom() )
            {
                maChannel2Rect.Top()    = maThumbRect.Bottom()+1;
                maChannel2Rect.Bottom() = mnChannelPixOffset+mnChannelPixRange-1;
                maChannel2Rect.Left()   = mnChannelPixTop;
                maChannel2Rect.Right()  = mnChannelPixBottom;
            }
            else
                maChannel2Rect.SetEmpty();

            const Rectangle aControlRegion( Rectangle( Point(0,0), Size( 10, SLIDER_THUMB_SIZE ) ) );
            Rectangle aThumbBounds, aThumbContent;
            if ( GetNativeControlRegion( CTRL_SLIDER, PART_THUMB_VERT,
                                         aControlRegion, 0, ImplControlValue(), OUString(),
                                         aThumbBounds, aThumbContent ) )
            {
                maThumbRect.Top() = mnThumbPixPos - aThumbBounds.GetHeight()/2;
                maThumbRect.Bottom() = maThumbRect.Top() + aThumbBounds.GetHeight() - 1;
                bInvalidateAll = true;
            }
        }
    }
    else
    {
        maChannel1Rect.SetEmpty();
        maChannel2Rect.SetEmpty();
        maThumbRect.SetEmpty();
    }

    if ( bUpdate )
    {
        if ( aOldThumbRect != maThumbRect )
        {
            if( bInvalidateAll )
                Invalidate();
            else
            {
                Region aInvalidRegion( aOldThumbRect );
                aInvalidRegion.Union( maThumbRect );

                if( !IsBackground() && GetParent() )
                {
                    const Point aPos( GetPosPixel() );
                    aInvalidRegion.Move( aPos.X(), aPos.Y() );
                    GetParent()->Invalidate( aInvalidRegion, INVALIDATE_TRANSPARENT | INVALIDATE_UPDATE );
                }
                else
                    Invalidate( aInvalidRegion );
            }
        }
    }
}

// -----------------------------------------------------------------------

long Slider::ImplCalcThumbPos( long nPixPos )
{
    // calculate position
    long nCalcThumbPos;
    nCalcThumbPos = ImplMulDiv( nPixPos-mnThumbPixOffset, mnMaxRange-mnMinRange, mnThumbPixRange-1 );
    nCalcThumbPos += mnMinRange;
    return nCalcThumbPos;
}

// -----------------------------------------------------------------------

long Slider::ImplCalcThumbPosPix( long nPos )
{
    // calculate position
    long nCalcThumbPos;
    nCalcThumbPos = ImplMulDiv( nPos-mnMinRange, mnThumbPixRange-1, mnMaxRange-mnMinRange );
    // at the beginning and end we try to display Slider correctly
    if ( !nCalcThumbPos && (mnThumbPos > mnMinRange) )
        nCalcThumbPos = 1;
    if ( nCalcThumbPos &&
         (nCalcThumbPos == mnThumbPixRange-1) &&
         (mnThumbPos < mnMaxRange) )
        nCalcThumbPos--;
    return nCalcThumbPos+mnThumbPixOffset;
}

// -----------------------------------------------------------------------

void Slider::ImplCalc( sal_Bool bUpdate )
{
    bool bInvalidateAll = false;

    if ( mbCalcSize )
    {
        long nOldChannelPixOffset   = mnChannelPixOffset;
        long nOldChannelPixRange    = mnChannelPixRange;
        long nOldChannelPixTop      = mnChannelPixTop;
        long nOldChannelPixBottom   = mnChannelPixBottom;
        long nCalcWidth;
        long nCalcHeight;

        maChannel1Rect.SetEmpty();
        maChannel2Rect.SetEmpty();
        maThumbRect.SetEmpty();

        Size aSize = GetOutputSizePixel();
        if ( GetStyle() & WB_HORZ )
        {
            nCalcWidth          = aSize.Width();
            nCalcHeight         = aSize.Height();
            maThumbRect.Top()   = 0;
            maThumbRect.Bottom()= aSize.Height()-1;
        }
        else
        {
            nCalcWidth          = aSize.Height();
            nCalcHeight         = aSize.Width();
            maThumbRect.Left()  = 0;
            maThumbRect.Right() = aSize.Width()-1;
        }

        if ( nCalcWidth >= SLIDER_THUMB_SIZE )
        {
            mnThumbPixOffset    = SLIDER_THUMB_HALFSIZE;
            mnThumbPixRange     = nCalcWidth-(SLIDER_THUMB_HALFSIZE*2);
            mnThumbPixPos       = 0;
            mnChannelPixOffset  = SLIDER_CHANNEL_OFFSET;
            mnChannelPixRange   = nCalcWidth-(SLIDER_CHANNEL_OFFSET*2);
            mnChannelPixTop     = (nCalcHeight/2)-SLIDER_CHANNEL_HALFSIZE;
            mnChannelPixBottom  = mnChannelPixTop+SLIDER_CHANNEL_SIZE-1;
        }
        else
        {
            mnThumbPixRange = 0;
            mnChannelPixRange = 0;
        }

        if ( (nOldChannelPixOffset != mnChannelPixOffset) ||
             (nOldChannelPixRange != mnChannelPixRange) ||
             (nOldChannelPixTop != mnChannelPixTop) ||
             (nOldChannelPixBottom != mnChannelPixBottom) )
            bInvalidateAll = true;

        mbCalcSize = sal_False;
    }

    if ( mnThumbPixRange )
        mnThumbPixPos = ImplCalcThumbPosPix( mnThumbPos );

    if ( bUpdate && bInvalidateAll )
    {
        Invalidate();
        bUpdate = sal_False;
    }
    ImplUpdateRects( bUpdate );
}

// -----------------------------------------------------------------------

void Slider::ImplDraw( sal_uInt16 nDrawFlags )
{
    DecorationView          aDecoView( this );
    sal_uInt16                  nStyle;
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    sal_Bool                    bEnabled = IsEnabled();

    // do missing calculations
    if ( mbCalcSize )
        ImplCalc( sal_False );

    ControlPart nPart = (GetStyle() & WB_HORZ) ? PART_TRACK_HORZ_AREA : PART_TRACK_VERT_AREA;
    ControlState   nState = ( IsEnabled() ? CTRL_STATE_ENABLED : 0 ) | ( HasFocus() ? CTRL_STATE_FOCUSED : 0 );
    SliderValue    sldValue;

    sldValue.mnMin       = mnMinRange;
    sldValue.mnMax       = mnMaxRange;
    sldValue.mnCur       = mnThumbPos;
    sldValue.maThumbRect = maThumbRect;

    if( IsMouseOver() )
    {
        if( maThumbRect.IsInside( GetPointerPosPixel() ) )
            sldValue.mnThumbState |= CTRL_STATE_ROLLOVER;
    }

    const Rectangle aCtrlRegion( Point(0,0), GetOutputSizePixel() );
    bool bNativeOK = DrawNativeControl( CTRL_SLIDER, nPart,
                                        aCtrlRegion, nState, sldValue, OUString() );
    if( bNativeOK )
        return;

    if ( (nDrawFlags & SLIDER_DRAW_CHANNEL1) && !maChannel1Rect.IsEmpty() )
    {
        long        nRectSize;
        Rectangle   aRect = maChannel1Rect;
        SetLineColor( rStyleSettings.GetShadowColor() );
        if ( GetStyle() & WB_HORZ )
        {
            DrawLine( aRect.TopLeft(), Point( aRect.Left(), aRect.Bottom()-1 ) );
            DrawLine( aRect.TopLeft(), aRect.TopRight() );
        }
        else
        {
            DrawLine( aRect.TopLeft(), Point( aRect.Right()-1, aRect.Top() ) );
            DrawLine( aRect.TopLeft(), aRect.BottomLeft() );
        }
        SetLineColor( rStyleSettings.GetLightColor() );
        if ( GetStyle() & WB_HORZ )
        {
            DrawLine( aRect.BottomLeft(), aRect.BottomRight() );
            nRectSize = aRect.GetWidth();
        }
        else
        {
            DrawLine( aRect.TopRight(), aRect.BottomRight() );
            nRectSize = aRect.GetHeight();
        }

        if ( nRectSize > 1 )
        {
            aRect.Left()++;
            aRect.Top()++;
            if ( GetStyle() & WB_HORZ )
                aRect.Bottom()--;
            else
                aRect.Right()--;
            SetLineColor();
            if ( mnStateFlags & SLIDER_STATE_CHANNEL1_DOWN )
                SetFillColor( rStyleSettings.GetShadowColor() );
            else
                SetFillColor( rStyleSettings.GetCheckedColor() );
            DrawRect( aRect );
        }
    }

    if ( (nDrawFlags & SLIDER_DRAW_CHANNEL2) && !maChannel2Rect.IsEmpty() )
    {
        long        nRectSize;
        Rectangle   aRect = maChannel2Rect;
        SetLineColor( rStyleSettings.GetLightColor() );
        if ( GetStyle() & WB_HORZ )
        {
            DrawLine( aRect.TopRight(), aRect.BottomRight() );
            DrawLine( aRect.BottomLeft(), aRect.BottomRight() );
            nRectSize = aRect.GetWidth();
        }
        else
        {
            DrawLine( aRect.BottomLeft(), aRect.BottomRight() );
            DrawLine( aRect.TopRight(), aRect.BottomRight() );
            nRectSize = aRect.GetHeight();
        }

        if ( nRectSize > 1 )
        {
            SetLineColor( rStyleSettings.GetShadowColor() );
            if ( GetStyle() & WB_HORZ )
                DrawLine( aRect.TopLeft(), Point( aRect.Right()-1, aRect.Top() ) );
            else
                DrawLine( aRect.TopLeft(), Point( aRect.Left(), aRect.Bottom()-1 ) );

            aRect.Right()--;
            aRect.Bottom()--;
            if ( GetStyle() & WB_HORZ )
                aRect.Top()++;
            else
                aRect.Left()++;
            SetLineColor();
            if ( mnStateFlags & SLIDER_STATE_CHANNEL2_DOWN )
                SetFillColor( rStyleSettings.GetShadowColor() );
            else
                SetFillColor( rStyleSettings.GetCheckedColor() );
            DrawRect( aRect );
        }
    }

    if ( nDrawFlags & SLIDER_DRAW_THUMB )
    {
        if ( !maThumbRect.IsEmpty() )
        {
            if ( bEnabled )
            {
                nStyle = 0;
                if ( mnStateFlags & SLIDER_STATE_THUMB_DOWN )
                    nStyle |= BUTTON_DRAW_PRESSED;
                aDecoView.DrawButton( maThumbRect, nStyle );
            }
            else
            {
                SetLineColor( rStyleSettings.GetShadowColor() );
                SetFillColor( rStyleSettings.GetCheckedColor() );
                DrawRect( maThumbRect );
            }
        }
    }
}

// -----------------------------------------------------------------------

sal_Bool Slider::ImplIsPageUp( const Point& rPos )
{
    Size aSize = GetOutputSizePixel();
    Rectangle aRect = maChannel1Rect;
    if ( GetStyle() & WB_HORZ )
    {
        aRect.Top()     = 0;
        aRect.Bottom()  = aSize.Height()-1;
    }
    else
    {
        aRect.Left()    = 0;
        aRect.Right()   = aSize.Width()-1;
    }
    return aRect.IsInside( rPos );
}

// -----------------------------------------------------------------------

sal_Bool Slider::ImplIsPageDown( const Point& rPos )
{
    Size aSize = GetOutputSizePixel();
    Rectangle aRect = maChannel2Rect;
    if ( GetStyle() & WB_HORZ )
    {
        aRect.Top()     = 0;
        aRect.Bottom()  = aSize.Height()-1;
    }
    else
    {
        aRect.Left()    = 0;
        aRect.Right()   = aSize.Width()-1;
    }
    return aRect.IsInside( rPos );
}

// -----------------------------------------------------------------------

long Slider::ImplSlide( long nNewPos, sal_Bool bCallEndSlide )
{
    long nOldPos = mnThumbPos;
    SetThumbPos( nNewPos );
    long nDelta = mnThumbPos-nOldPos;
    if ( nDelta )
    {
        mnDelta = nDelta;
        Slide();
        if ( bCallEndSlide )
            EndSlide();
        mnDelta = 0;
    }
    return nDelta;
}

// -----------------------------------------------------------------------

long Slider::ImplDoAction( sal_Bool bCallEndSlide )
{
    long nDelta = 0;

    switch ( meScrollType )
    {
        case SCROLL_LINEUP:
            nDelta = ImplSlide( mnThumbPos-mnLineSize, bCallEndSlide );
            break;

        case SCROLL_LINEDOWN:
            nDelta = ImplSlide( mnThumbPos+mnLineSize, bCallEndSlide );
            break;

        case SCROLL_PAGEUP:
            nDelta = ImplSlide( mnThumbPos-mnPageSize, bCallEndSlide );
            break;

        case SCROLL_PAGEDOWN:
            nDelta = ImplSlide( mnThumbPos+mnPageSize, bCallEndSlide );
            break;

        case SCROLL_SET:
            nDelta = ImplSlide( ImplCalcThumbPos( GetPointerPosPixel().X() ), bCallEndSlide );
            break;
        default:
            break;
    }

    return nDelta;
}

// -----------------------------------------------------------------------

void Slider::ImplDoMouseAction( const Point& rMousePos, sal_Bool bCallAction )
{
    sal_uInt16  nOldStateFlags = mnStateFlags;
    sal_Bool    bAction = sal_False;

    switch ( meScrollType )
    {
        case( SCROLL_SET ):
        {
            const bool bUp = ImplIsPageUp( rMousePos ), bDown = ImplIsPageDown( rMousePos );

            if ( bUp || bDown )
            {
                bAction = bCallAction;
                mnStateFlags |= ( bUp ? SLIDER_STATE_CHANNEL1_DOWN : SLIDER_STATE_CHANNEL2_DOWN );
            }
            else
                mnStateFlags &= ~( SLIDER_STATE_CHANNEL1_DOWN | SLIDER_STATE_CHANNEL2_DOWN );
            break;
        }

        case SCROLL_PAGEUP:
            if ( ImplIsPageUp( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SLIDER_STATE_CHANNEL1_DOWN;
            }
            else
                mnStateFlags &= ~SLIDER_STATE_CHANNEL1_DOWN;
            break;

        case SCROLL_PAGEDOWN:
            if ( ImplIsPageDown( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SLIDER_STATE_CHANNEL2_DOWN;
            }
            else
                mnStateFlags &= ~SLIDER_STATE_CHANNEL2_DOWN;
            break;
        default:
            break;
    }

    if ( bAction )
    {
        if ( ImplDoAction( sal_False ) )
        {
            // Update the channel complete
            if ( mnDragDraw & SLIDER_DRAW_CHANNEL )
            {
                Update();
                ImplDraw( mnDragDraw );
            }
        }
    }
    else if ( nOldStateFlags != mnStateFlags )
        ImplDraw( mnDragDraw );
}

// -----------------------------------------------------------------------

long Slider::ImplDoSlide( long nNewPos )
{
    if ( meScrollType != SCROLL_DONTKNOW )
        return 0;

    meScrollType = SCROLL_DRAG;
    long nDelta = ImplSlide( nNewPos, sal_True );
    meScrollType = SCROLL_DONTKNOW;
    return nDelta;
}

// -----------------------------------------------------------------------

long Slider::ImplDoSlideAction( ScrollType eScrollType )
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

void Slider::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        const Point&    rMousePos = rMEvt.GetPosPixel();
        sal_uInt16          nTrackFlags = 0;

        if ( maThumbRect.IsInside( rMousePos ) )
        {
            nTrackFlags     = 0;
            meScrollType    = SCROLL_DRAG;
            mnDragDraw      = SLIDER_DRAW_THUMB;

            // calculate additional values
            Point aCenterPos = maThumbRect.Center();
            if ( GetStyle() & WB_HORZ )
                mnMouseOff = rMousePos.X()-aCenterPos.X();
            else
                mnMouseOff = rMousePos.Y()-aCenterPos.Y();
        }
        else if ( ImplIsPageUp( rMousePos ) )
        {
            if( GetStyle() & WB_SLIDERSET )
                meScrollType = SCROLL_SET;
            else
            {
                nTrackFlags = STARTTRACK_BUTTONREPEAT;
                meScrollType = SCROLL_PAGEUP;
            }

            mnDragDraw = SLIDER_DRAW_CHANNEL;
        }
        else if ( ImplIsPageDown( rMousePos ) )
        {
            if( GetStyle() & WB_SLIDERSET )
                meScrollType = SCROLL_SET;
            else
            {
                nTrackFlags = STARTTRACK_BUTTONREPEAT;
                meScrollType = SCROLL_PAGEDOWN;
            }

            mnDragDraw = SLIDER_DRAW_CHANNEL;
        }

        // Shall we start Tracking?
        if( meScrollType != SCROLL_DONTKNOW )
        {
            // store Start position for cancel and EndScroll delta
            mnStartPos = mnThumbPos;
            ImplDoMouseAction( rMousePos, meScrollType != SCROLL_SET );
            Update();

            if( meScrollType != SCROLL_SET )
                StartTracking( nTrackFlags );
        }
    }
}

// -----------------------------------------------------------------------

void Slider::MouseButtonUp( const MouseEvent& )
{
    if( SCROLL_SET == meScrollType )
    {
        // reset Button and PageRect state
        const sal_uInt16 nOldStateFlags = mnStateFlags;

        mnStateFlags &= ~( SLIDER_STATE_CHANNEL1_DOWN | SLIDER_STATE_CHANNEL2_DOWN | SLIDER_STATE_THUMB_DOWN );

        if ( nOldStateFlags != mnStateFlags )
            ImplDraw( mnDragDraw );

        mnDragDraw = 0;
        ImplDoAction( sal_True );
        meScrollType = SCROLL_DONTKNOW;
    }
}

// -----------------------------------------------------------------------

void Slider::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        // reset Button and PageRect state
        sal_uInt16 nOldStateFlags = mnStateFlags;
        mnStateFlags &= ~(SLIDER_STATE_CHANNEL1_DOWN | SLIDER_STATE_CHANNEL2_DOWN |
                          SLIDER_STATE_THUMB_DOWN);
        if ( nOldStateFlags != mnStateFlags )
            ImplDraw( mnDragDraw );
        mnDragDraw = 0;

        // on cancel, reset the previous Thumb position
        if ( rTEvt.IsTrackingCanceled() )
        {
            long nOldPos = mnThumbPos;
            SetThumbPos( mnStartPos );
            mnDelta = mnThumbPos-nOldPos;
            Slide();
        }

        if ( meScrollType == SCROLL_DRAG )
        {
            // after dragging, recalculate to a rounded Thumb position
            ImplCalc();
            Update();

            if ( !mbFullDrag && (mnStartPos != mnThumbPos) )
            {
                mnDelta = mnThumbPos-mnStartPos;
                Slide();
                mnDelta = 0;
            }
        }

        mnDelta = mnThumbPos-mnStartPos;
        EndSlide();
        mnDelta = 0;
        meScrollType = SCROLL_DONTKNOW;
    }
    else
    {
        const Point rMousePos = rTEvt.GetMouseEvent().GetPosPixel();

        // special handling for dragging
        if ( meScrollType == SCROLL_DRAG )
        {
            long nMovePix;
            Point aCenterPos = maThumbRect.Center();
            if ( GetStyle() & WB_HORZ )
                nMovePix = rMousePos.X()-(aCenterPos.X()+mnMouseOff);
            else
                nMovePix = rMousePos.Y()-(aCenterPos.Y()+mnMouseOff);
            // only if the mouse moves in Scroll direction we have to act
            if ( nMovePix )
            {
                mnThumbPixPos += nMovePix;
                if ( mnThumbPixPos < mnThumbPixOffset )
                    mnThumbPixPos = mnThumbPixOffset;
                if ( mnThumbPixPos > (mnThumbPixOffset+mnThumbPixRange-1) )
                    mnThumbPixPos = mnThumbPixOffset+mnThumbPixRange-1;
                long nOldPos = mnThumbPos;
                mnThumbPos = ImplCalcThumbPos( mnThumbPixPos );
                if ( nOldPos != mnThumbPos )
                {
                    ImplUpdateRects();
                    Update();
                    if ( mbFullDrag && (nOldPos != mnThumbPos) )
                    {
                        mnDelta = mnThumbPos-nOldPos;
                        Slide();
                        mnDelta = 0;
                    }
                }
            }
        }
        else
            ImplDoMouseAction( rMousePos, rTEvt.IsTrackingRepeat() );

        // end tracking if ScrollBar values indicate we are done
        if ( !IsVisible() )
            EndTracking();
    }
}

// -----------------------------------------------------------------------

void Slider::KeyInput( const KeyEvent& rKEvt )
{
    if ( !rKEvt.GetKeyCode().GetModifier() )
    {
        switch ( rKEvt.GetKeyCode().GetCode() )
        {
            case KEY_HOME:
                ImplDoSlide( GetRangeMin() );
                break;
            case KEY_END:
                ImplDoSlide( GetRangeMax() );
                break;

            case KEY_LEFT:
            case KEY_UP:
                ImplDoSlideAction( SCROLL_LINEUP );
                break;

            case KEY_RIGHT:
            case KEY_DOWN:
                ImplDoSlideAction( SCROLL_LINEDOWN );
                break;

            case KEY_PAGEUP:
                ImplDoSlideAction( SCROLL_PAGEUP );
                break;

            case KEY_PAGEDOWN:
                ImplDoSlideAction( SCROLL_PAGEDOWN );
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

void Slider::Paint( const Rectangle& )
{
    ImplDraw( SLIDER_DRAW_ALL );
}

// -----------------------------------------------------------------------

void Slider::Resize()
{
    Control::Resize();
    mbCalcSize = sal_True;
    if ( IsReallyVisible() )
        ImplCalc( sal_False );
    Invalidate();
}

// -----------------------------------------------------------------------

void Slider::RequestHelp( const HelpEvent& rHEvt )
{
    Control::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void Slider::StateChanged( StateChangedType nType )
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
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            if ( (GetPrevStyle() & SLIDER_VIEW_STYLE) !=
                 (GetStyle() & SLIDER_VIEW_STYLE) )
            {
                mbCalcSize = sal_True;
                ImplCalc( sal_False );
                Invalidate();
            }
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void Slider::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void Slider::Slide()
{
    maSlideHdl.Call( this );
}

// -----------------------------------------------------------------------

void Slider::EndSlide()
{
    maEndSlideHdl.Call( this );
}

// -----------------------------------------------------------------------

void Slider::SetRange( const Range& rRange )
{
    // adjust Range
    Range aRange = rRange;
    aRange.Justify();
    long nNewMinRange = aRange.Min();
    long nNewMaxRange = aRange.Max();

    // reset Range if different
    if ( (mnMinRange != nNewMinRange) ||
         (mnMaxRange != nNewMaxRange) )
    {
        mnMinRange = nNewMinRange;
        mnMaxRange = nNewMaxRange;

        // adjust Thumb
        if ( mnThumbPos > mnMaxRange )
            mnThumbPos = mnMaxRange;
        if ( mnThumbPos < mnMinRange )
            mnThumbPos = mnMinRange;

        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void Slider::SetThumbPos( long nNewThumbPos )
{
    if ( nNewThumbPos < mnMinRange )
        nNewThumbPos = mnMinRange;
    if ( nNewThumbPos > mnMaxRange )
        nNewThumbPos = mnMaxRange;

    if ( mnThumbPos != nNewThumbPos )
    {
        mnThumbPos = nNewThumbPos;
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

Size Slider::CalcWindowSizePixel()
{
    long nWidth = mnMaxRange-mnMinRange+(SLIDER_THUMB_HALFSIZE*2)+1;
    long nHeight = SLIDER_HEIGHT;
    Size aSize;
    if ( GetStyle() & WB_HORZ )
    {
        aSize.Width()   = nWidth;
        aSize.Height()  = nHeight;
    }
    else
    {
        aSize.Height()  = nWidth;
        aSize.Width()   = nHeight;
    }
    return aSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
