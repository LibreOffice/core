/*************************************************************************
 *
 *  $RCSfile: slider.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_SLIDER_CXX

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_SLIDER_HXX
#include <slider.hxx>
#endif

#pragma hdrstop

// =======================================================================

static long ImplMulDiv( long nNumber, long nNumerator, long nDenominator )
{
    double n = ((double)nNumber * (double)nNumerator) / (double)nDenominator;
    return (long)n;
}

// =======================================================================

#define SLIDER_DRAW_THUMB           ((USHORT)0x0001)
#define SLIDER_DRAW_CHANNEL1        ((USHORT)0x0002)
#define SLIDER_DRAW_CHANNEL2        ((USHORT)0x0004)
#define SLIDER_DRAW_CHANNEL         (SLIDER_DRAW_CHANNEL1 | SLIDER_DRAW_CHANNEL2)
#define SLIDER_DRAW_ALL             (SLIDER_DRAW_THUMB | SLIDER_DRAW_CHANNEL)

#define SLIDER_STATE_CHANNEL1_DOWN  ((USHORT)0x0001)
#define SLIDER_STATE_CHANNEL2_DOWN  ((USHORT)0x0002)
#define SLIDER_STATE_THUMB_DOWN     ((USHORT)0x0004)

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
    mbCalcSize          = TRUE;
    mbFullDrag          = TRUE;

    ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );

    ImplInitSettings();
    SetSizePixel( CalcWindowSizePixel() );
}

// -----------------------------------------------------------------------

void Slider::ImplInitStyle( WinBits nStyle )
{
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

    INT16 nMin          = ReadShortRes();
    INT16 nMax          = ReadShortRes();
    INT16 nThumbPos     = ReadShortRes();
    INT16 nPage         = ReadShortRes();
    INT16 nStep         = ReadShortRes();
    INT16 nVisibleSize  = ReadShortRes();

    SetRange( Range( nMin, nMax ) );
    SetLineSize( nStep );
    SetPageSize( nPage );
    SetThumbPos( nThumbPos );
}

// -----------------------------------------------------------------------

void Slider::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    Window* pParent = GetParent();
    if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
    {
        EnableChildTransparentMode( TRUE );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( TRUE );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( FALSE );
        SetParentClipMode( 0 );
        SetPaintTransparent( FALSE );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

// -----------------------------------------------------------------------

void Slider::ImplUpdateRects( BOOL bUpdate )
{
    Rectangle aOldThumbRect = maThumbRect;

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
            Region aInvalidRegion( aOldThumbRect );
            aInvalidRegion.Union( maThumbRect );
            Invalidate( aInvalidRegion );
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

void Slider::ImplCalc( BOOL bUpdate )
{
    BOOL bInvalidateAll = FALSE;

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
            bInvalidateAll = TRUE;

        mbCalcSize = FALSE;
    }

    if ( mnThumbPixRange )
        mnThumbPixPos = ImplCalcThumbPosPix( mnThumbPos );

    if ( bUpdate && bInvalidateAll )
    {
        Invalidate();
        bUpdate = FALSE;
    }
    ImplUpdateRects( bUpdate );
}

// -----------------------------------------------------------------------

void Slider::ImplDraw( USHORT nDrawFlags )
{
    DecorationView          aDecoView( this );
    USHORT                  nStyle;
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    BOOL                    bEnabled = IsEnabled();

    // Evt. noch offene Berechnungen nachholen
    if ( mbCalcSize )
        ImplCalc( FALSE );

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

BOOL Slider::ImplIsPageUp( const Point& rPos )
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

BOOL Slider::ImplIsPageDown( const Point& rPos )
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

long Slider::ImplSlide( long nNewPos, BOOL bCallEndSlide )
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

long Slider::ImplDoAction( BOOL bCallEndSlide )
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
    }

    return nDelta;
}

// -----------------------------------------------------------------------

void Slider::ImplDoMouseAction( const Point& rMousePos, BOOL bCallAction )
{
    USHORT  nOldStateFlags = mnStateFlags;
    BOOL    bAction = FALSE;

    switch ( meScrollType )
    {
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
    }

    if ( bAction )
    {
        if ( ImplDoAction( FALSE ) )
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
    long nDelta = ImplSlide( nNewPos, TRUE );
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
    long nDelta = ImplDoAction( TRUE );
    meScrollType = SCROLL_DONTKNOW;
    return nDelta;
}

// -----------------------------------------------------------------------

void Slider::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        const Point&    rMousePos = rMEvt.GetPosPixel();
        USHORT          nTrackFlags = 0;

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

            // Im OS2-Look geben wir den Thumb gedrueckt aus
            if ( GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_OS2STYLE )
            {
                mnStateFlags |= SLIDER_STATE_THUMB_DOWN;
                ImplDraw( SLIDER_DRAW_THUMB );
            }
        }
        else if ( ImplIsPageUp( rMousePos ) )
        {
            nTrackFlags     = STARTTRACK_BUTTONREPEAT;
            meScrollType    = SCROLL_PAGEUP;
            mnDragDraw      = SLIDER_DRAW_CHANNEL;
        }
        else if ( ImplIsPageDown( rMousePos ) )
        {
            nTrackFlags     = STARTTRACK_BUTTONREPEAT;
            meScrollType    = SCROLL_PAGEDOWN;
            mnDragDraw      = SLIDER_DRAW_CHANNEL;
        }

        // Soll Tracking gestartet werden
        if ( meScrollType != SCROLL_DONTKNOW )
        {
            // Startposition merken fuer Abbruch und EndScroll-Delta
            mnStartPos = mnThumbPos;
            ImplDoMouseAction( rMousePos );
            Update();
            StartTracking( nTrackFlags );
        }
    }
}

// -----------------------------------------------------------------------

void Slider::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        // Button und PageRect-Status wieder herstellen
        USHORT nOldStateFlags = mnStateFlags;
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

void Slider::Paint( const Rectangle& rRect )
{
    ImplDraw( SLIDER_DRAW_ALL );
}

// -----------------------------------------------------------------------

void Slider::Resize()
{
    mbCalcSize = TRUE;
    if ( IsReallyVisible() )
        ImplCalc( FALSE );
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
        ImplCalc( FALSE );
    else if ( nType == STATE_CHANGE_DATA )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            ImplCalc( TRUE );
    }
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            ImplCalc( FALSE );
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
            if ( (GetPrevStyle() & SLIDER_VIEW_STYLE) !=
                 (GetStyle() & SLIDER_VIEW_STYLE) )
            {
                mbCalcSize = TRUE;
                ImplCalc( FALSE );
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
