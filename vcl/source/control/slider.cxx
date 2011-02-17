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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/slider.hxx>



// =======================================================================

static long ImplMulDiv( long nNumber, long nNumerator, long nDenominator )
{
    double n = ((double)nNumber * (double)nNumerator) / (double)nDenominator;
    return (long)n;
}

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

Slider::Slider( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_SLIDER )
{
    rResId.SetRT( RSC_SCROLLBAR );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void Slider::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    sal_Int16 nMin          = ReadShortRes();
    sal_Int16 nMax          = ReadShortRes();
    sal_Int16 nThumbPos     = ReadShortRes();
    sal_Int16 nPage         = ReadShortRes();
    sal_Int16 nStep         = ReadShortRes();
    /* sal_Int16 nVisibleSize   = */ ReadShortRes();

    SetRange( Range( nMin, nMax ) );
    SetLineSize( nStep );
    SetPageSize( nPage );
    SetThumbPos( nThumbPos );
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
                                         aControlRegion, 0, ImplControlValue(), rtl::OUString(),
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
                                         aControlRegion, 0, ImplControlValue(), rtl::OUString(),
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
    // Position berechnen
    long nCalcThumbPos;
    nCalcThumbPos = ImplMulDiv( nPixPos-mnThumbPixOffset, mnMaxRange-mnMinRange, mnThumbPixRange-1 );
    nCalcThumbPos += mnMinRange;
    return nCalcThumbPos;
}

// -----------------------------------------------------------------------

long Slider::ImplCalcThumbPosPix( long nPos )
{
    // Position berechnen
    long nCalcThumbPos;
    nCalcThumbPos = ImplMulDiv( nPos-mnMinRange, mnThumbPixRange-1, mnMaxRange-mnMinRange );
    // Am Anfang und Ende des Sliders versuchen wir die Anzeige korrekt
    // anzuzeigen
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
    sal_Bool bInvalidateAll = sal_False;

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
            bInvalidateAll = sal_True;

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

    // Evt. noch offene Berechnungen nachholen
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
                                        aCtrlRegion, nState, sldValue, rtl::OUString() );
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

            // Zusaetzliche Daten berechnen
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

        // Soll Tracking gestartet werden
        if( meScrollType != SCROLL_DONTKNOW )
        {
            // Startposition merken fuer Abbruch und EndScroll-Delta
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
        // Button und PageRect-Status wieder herstellen
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
        // Button und PageRect-Status wieder herstellen
        sal_uInt16 nOldStateFlags = mnStateFlags;
        mnStateFlags &= ~(SLIDER_STATE_CHANNEL1_DOWN | SLIDER_STATE_CHANNEL2_DOWN |
                          SLIDER_STATE_THUMB_DOWN);
        if ( nOldStateFlags != mnStateFlags )
            ImplDraw( mnDragDraw );
        mnDragDraw = 0;

        // Bei Abbruch, die alte ThumbPosition wieder herstellen
        if ( rTEvt.IsTrackingCanceled() )
        {
            long nOldPos = mnThumbPos;
            SetThumbPos( mnStartPos );
            mnDelta = mnThumbPos-nOldPos;
            Slide();
        }

        if ( meScrollType == SCROLL_DRAG )
        {
            // Wenn gedragt wurde, berechnen wir den Thumb neu, damit
            // er wieder auf einer gerundeten ThumbPosition steht
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

        // Dragging wird speziell behandelt
        if ( meScrollType == SCROLL_DRAG )
        {
            long nMovePix;
            Point aCenterPos = maThumbRect.Center();
            if ( GetStyle() & WB_HORZ )
                nMovePix = rMousePos.X()-(aCenterPos.X()+mnMouseOff);
            else
                nMovePix = rMousePos.Y()-(aCenterPos.Y()+mnMouseOff);
            // Nur wenn sich Maus in die Scrollrichtung bewegt, muessen
            // wir etwas tun
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

        // Wenn Slider-Werte so umgesetzt wurden, das es nichts
        // mehr zum Tracking gibt, dann berechen wir hier ab
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

void Slider::SetRangeMin( long nNewRange )
{
    SetRange( Range( nNewRange, GetRangeMax() ) );
}

// -----------------------------------------------------------------------

void Slider::SetRangeMax( long nNewRange )
{
    SetRange( Range( GetRangeMin(), nNewRange ) );
}

// -----------------------------------------------------------------------

void Slider::SetRange( const Range& rRange )
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
