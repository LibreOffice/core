/*************************************************************************
 *
 *  $RCSfile: scrbar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obr $ $Date: 2001-02-14 08:24:01 $
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

#define _SV_SCRBAR_CXX

#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <sound.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <scrbar.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif

#pragma hdrstop

// =======================================================================

static long ImplMulDiv( long nNumber, long nNumerator, long nDenominator )
{
    double n = ((double)nNumber * (double)nNumerator) / (double)nDenominator;
    return (long)n;
}

// =======================================================================

#define SCRBAR_DRAW_BTN1            ((USHORT)0x0001)
#define SCRBAR_DRAW_BTN2            ((USHORT)0x0002)
#define SCRBAR_DRAW_PAGE1           ((USHORT)0x0004)
#define SCRBAR_DRAW_PAGE2           ((USHORT)0x0008)
#define SCRBAR_DRAW_THUMB           ((USHORT)0x0010)
#define SCRBAR_DRAW_ALL             (SCRBAR_DRAW_BTN1 | SCRBAR_DRAW_BTN2 |  \
                                     SCRBAR_DRAW_PAGE1 | SCRBAR_DRAW_PAGE2 |\
                                     SCRBAR_DRAW_THUMB)

#define SCRBAR_STATE_BTN1_DOWN      ((USHORT)0x0001)
#define SCRBAR_STATE_BTN1_DISABLE   ((USHORT)0x0002)
#define SCRBAR_STATE_BTN2_DOWN      ((USHORT)0x0004)
#define SCRBAR_STATE_BTN2_DISABLE   ((USHORT)0x0008)
#define SCRBAR_STATE_PAGE1_DOWN     ((USHORT)0x0010)
#define SCRBAR_STATE_PAGE2_DOWN     ((USHORT)0x0020)
#define SCRBAR_STATE_THUMB_DOWN     ((USHORT)0x0040)

#define SCRBAR_MIN_THUMB            8

#define SCRBAR_VIEW_STYLE           (WB_3DLOOK | WB_HORZ | WB_VERT)

// =======================================================================

void ScrollBar::ImplInit( Window* pParent, WinBits nStyle )
{
    mpDDScrollTimer     = NULL;
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
    mbCalcSize          = TRUE;
    mbFullDrag          = 0;
    mbDDScroll          = FALSE;

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
        mbFullDrag = TRUE;
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
    if ( mpDDScrollTimer )
    {
        delete mpDDScrollTimer;
        mpDDScrollTimer = NULL;
    }
}

// -----------------------------------------------------------------------

void ScrollBar::ImplLoadRes( const ResId& rResId )
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
    SetVisibleSize( nVisibleSize );
    SetThumbPos( nThumbPos );
}

// -----------------------------------------------------------------------

BOOL ScrollBar::ImplUpdateThumbRect( const Rectangle& rOldRect )
{
    Size aThumbRectSize  = rOldRect.GetSize();
/* !!! Wegen ueberlappenden Fenstern ... !!!
    if ( aThumbRectSize == maThumbRect.GetSize() )
    {
        DrawOutDev( maThumbRect.TopLeft(), aThumbRectSize,
                    rOldRect.TopLeft(), aThumbRectSize );
        return TRUE;
    }
    else
*/
        return FALSE;
}

// -----------------------------------------------------------------------

void ScrollBar::ImplUpdateRects( BOOL bUpdate )
{
    USHORT      nOldStateFlags  = mnStateFlags;
    Rectangle   aOldPage1Rect = maPage1Rect;
    Rectangle   aOldPage2Rect = maPage2Rect;
    Rectangle   aOldThumbRect = maThumbRect;

    mnStateFlags  &= ~SCRBAR_STATE_BTN1_DISABLE;
    mnStateFlags  &= ~SCRBAR_STATE_BTN2_DISABLE;

    if ( mnThumbPixRange )
    {
        if ( GetStyle() & WB_HORZ )
        {
            maThumbRect.Left()      = maBtn1Rect.Right()+1+mnThumbPixPos;
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
                maPage2Rect.Right()     = maBtn2Rect.Left()-1;
            }
        }
        else
        {
            maThumbRect.Top()       = maBtn1Rect.Bottom()+1+mnThumbPixPos;
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
                maPage2Rect.Bottom()    = maBtn2Rect.Top()-1;
            }
        }
    }

    if ( mnThumbPos == mnMinRange )
        mnStateFlags |= SCRBAR_STATE_BTN1_DISABLE;
    if ( mnThumbPos >= (mnMaxRange-mnVisibleSize) )
        mnStateFlags |= SCRBAR_STATE_BTN2_DISABLE;

    if ( bUpdate )
    {
        USHORT nDraw = 0;
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
        {
            if ( !ImplUpdateThumbRect( aOldThumbRect ) )
                nDraw |= SCRBAR_DRAW_THUMB;
        }
        ImplDraw( nDraw );
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

void ScrollBar::ImplCalc( BOOL bUpdate )
{
    if ( mbCalcSize )
    {
        Size    aSize = GetOutputSizePixel();
        Size    aBtnSize;

        if ( GetStyle() & WB_HORZ )
        {
            if ( aSize.Height()*2 > aSize.Width()-SCRBAR_MIN_THUMB )
            {
                mnThumbPixRange         = 0;
                maBtn1Rect.Bottom()     = aSize.Height()-1;
                maBtn1Rect.Right()      = aSize.Width()/2;
                maBtn2Rect.Bottom()     = maBtn1Rect.Bottom();
                maBtn2Rect.Left()       = maBtn1Rect.Right()+1;
                maBtn2Rect.Right()      = aSize.Width()-1;
            }
            else
            {
                mnThumbPixRange         = aSize.Width()-(aSize.Height()*2);
                aBtnSize                = Size( aSize.Height(), aSize.Height() );
                maBtn2Rect.Left()       = aSize.Width()-aSize.Height();
                maBtn1Rect.SetSize( aBtnSize );
                maBtn2Rect.SetSize( aBtnSize );
                maPage1Rect.Left()      = maBtn1Rect.Right()+1;
                maPage1Rect.Bottom()    = maBtn1Rect.Bottom();
                maPage2Rect.Bottom()    = maBtn1Rect.Bottom();
                maThumbRect.Bottom()    = maBtn1Rect.Bottom();
            }
        }
        else
        {
            if ( aSize.Width()*2 > aSize.Height()-SCRBAR_MIN_THUMB )
            {
                mnThumbPixRange         = 0;
                maBtn1Rect.Right()      = aSize.Width()-1;
                maBtn1Rect.Bottom()     = aSize.Height()/2;
                maBtn2Rect.Right()      = maBtn1Rect.Right();
                maBtn2Rect.Top()        = maBtn1Rect.Bottom()+1;
                maBtn2Rect.Bottom()     = aSize.Height()-1;
            }
            else
            {
                mnThumbPixRange         = aSize.Height()-(aSize.Width()*2);
                aBtnSize                = Size( aSize.Width(), aSize.Width() );
                maBtn2Rect.Top()        = aSize.Height()-aSize.Width();
                maBtn1Rect.SetSize( aBtnSize );
                maBtn2Rect.SetSize( aBtnSize );
                maPage1Rect.Top()       = maBtn1Rect.Bottom()+1;
                maPage1Rect.Right()     = maBtn1Rect.Right();
                maPage2Rect.Right()     = maBtn1Rect.Right();
                maThumbRect.Right()     = maBtn1Rect.Right();
            }
        }

        if ( !mnThumbPixRange )
        {
            maPage1Rect.SetEmpty();
            maPage2Rect.SetEmpty();
            maThumbRect.SetEmpty();
        }

        mbCalcSize = FALSE;
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
                    mnThumbPixSize = maThumbRect.GetHeight();
                else
                    mnThumbPixSize = maThumbRect.GetWidth();
            }
            if ( mnThumbPixSize < SCRBAR_MIN_THUMB )
                mnThumbPixSize = SCRBAR_MIN_THUMB;
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
        bUpdate = FALSE;
    }
    ImplUpdateRects( bUpdate );
}

// -----------------------------------------------------------------------

void ScrollBar::ImplDraw( USHORT nDrawFlags )
{
    DecorationView          aDecoView( this );
    Rectangle               aTempRect;
    USHORT                  nStyle;
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    SymbolType              eSymbolType;
    BOOL                    bEnabled = IsEnabled();

    // Evt. noch offene Berechnungen nachholen
    if ( mbCalcSize )
        ImplCalc( FALSE );

    if ( nDrawFlags & SCRBAR_DRAW_BTN1 )
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

    if ( nDrawFlags & SCRBAR_DRAW_BTN2 )
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

    SetLineColor();

    if ( nDrawFlags & SCRBAR_DRAW_THUMB )
    {
        if ( !maThumbRect.IsEmpty() )
        {
            if ( bEnabled )
            {
                nStyle = BUTTON_DRAW_NOLIGHTBORDER;
                if ( mnStateFlags & SCRBAR_STATE_THUMB_DOWN )
                    nStyle |= BUTTON_DRAW_PRESSED;
                aTempRect = aDecoView.DrawButton( maThumbRect, nStyle );
                // Im OS2-Look geben wir auch ein Muster auf dem Thumb aus
                if ( rStyleSettings.GetOptions() & STYLE_OPTION_OS2STYLE )
                {
                    if ( GetStyle() & WB_HORZ )
                    {
                        if ( aTempRect.GetWidth() > 6 )
                        {
                            long nX = aTempRect.Center().X();
                            nX -= 6;
                            if ( nX < aTempRect.Left() )
                                nX = aTempRect.Left();
                            for ( int i = 0; i < 6; i++ )
                            {
                                if ( nX > aTempRect.Right()-1 )
                                    break;

                                SetLineColor( rStyleSettings.GetButtonTextColor() );
                                DrawLine( Point( nX, aTempRect.Top()+1 ),
                                          Point( nX, aTempRect.Bottom()-1 ) );
                                nX++;
                                SetLineColor( rStyleSettings.GetLightColor() );
                                DrawLine( Point( nX, aTempRect.Top()+1 ),
                                          Point( nX, aTempRect.Bottom()-1 ) );
                                nX++;
                            }
                        }
                    }
                    else
                    {
                        if ( aTempRect.GetHeight() > 6 )
                        {
                            long nY = aTempRect.Center().Y();
                            nY -= 6;
                            if ( nY < aTempRect.Top() )
                                nY = aTempRect.Top();
                            for ( int i = 0; i < 6; i++ )
                            {
                                if ( nY > aTempRect.Bottom()-1 )
                                    break;

                                SetLineColor( rStyleSettings.GetButtonTextColor() );
                                DrawLine( Point( aTempRect.Left()+1, nY ),
                                          Point( aTempRect.Right()-1, nY ) );
                                nY++;
                                SetLineColor( rStyleSettings.GetLightColor() );
                                DrawLine( Point( aTempRect.Left()+1, nY ),
                                          Point( aTempRect.Right()-1, nY ) );
                                nY++;
                            }
                        }
                    }
                    SetLineColor();
                }
            }
            else
            {
                SetFillColor( rStyleSettings.GetCheckedColor() );
                DrawRect( maThumbRect );
            }
        }
    }

    if ( nDrawFlags & SCRBAR_DRAW_PAGE1 )
    {
        if ( mnStateFlags & SCRBAR_STATE_PAGE1_DOWN )
            SetFillColor( rStyleSettings.GetShadowColor() );
        else
            SetFillColor( rStyleSettings.GetCheckedColor() );
        DrawRect( maPage1Rect );
    }
    if ( nDrawFlags & SCRBAR_DRAW_PAGE2 )
    {
        if ( mnStateFlags & SCRBAR_STATE_PAGE2_DOWN )
            SetFillColor( rStyleSettings.GetShadowColor() );
        else
            SetFillColor( rStyleSettings.GetCheckedColor() );
        DrawRect( maPage2Rect );
    }
}

// -----------------------------------------------------------------------

long ScrollBar::ImplScroll( long nNewPos, BOOL bCallEndScroll )
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

long ScrollBar::ImplDoAction( BOOL bCallEndScroll )
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
    }

    return nDelta;
}

// -----------------------------------------------------------------------

void ScrollBar::ImplDoMouseAction( const Point& rMousePos, BOOL bCallAction )
{
    USHORT  nOldStateFlags = mnStateFlags;
    BOOL    bAction = FALSE;

    switch ( meScrollType )
    {
        case SCROLL_LINEUP:
            if ( maBtn1Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_BTN1_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_BTN1_DOWN;
            break;

        case SCROLL_LINEDOWN:
            if ( maBtn2Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_BTN2_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_BTN2_DOWN;
            break;

        case SCROLL_PAGEUP:
            if ( maPage1Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_PAGE1_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_PAGE1_DOWN;
            break;

        case SCROLL_PAGEDOWN:
            if ( maPage2Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_PAGE2_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_PAGE2_DOWN;
            break;
    }

    if ( nOldStateFlags != mnStateFlags )
        ImplDraw( mnDragDraw );
    if ( bAction )
        ImplDoAction( FALSE );
}

// -----------------------------------------------------------------------

IMPL_LINK( ScrollBar, ImplTimerHdl, Timer*, pTimer )
{
    pTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonRepeat() );
    DoScroll( meDDScrollType );
    return 0;
}

// -----------------------------------------------------------------------

void ScrollBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        const Point&    rMousePos = rMEvt.GetPosPixel();
        USHORT          nTrackFlags = 0;

        if ( maBtn1Rect.IsInside( rMousePos ) )
        {
            if ( !(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) )
            {
                nTrackFlags     = STARTTRACK_BUTTONREPEAT;
                meScrollType    = SCROLL_LINEUP;
                mnDragDraw      = SCRBAR_DRAW_BTN1;
            }
            else
                Sound::Beep( SOUND_DISABLE, this );
        }
        else if ( maBtn2Rect.IsInside( rMousePos ) )
        {
            if ( !(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) )
            {
                nTrackFlags     = STARTTRACK_BUTTONREPEAT;
                meScrollType    = SCROLL_LINEDOWN;
                mnDragDraw      = SCRBAR_DRAW_BTN2;
            }
            else
                Sound::Beep( SOUND_DISABLE, this );
        }
        else if ( maThumbRect.IsInside( rMousePos ) )
        {
            if ( mnVisibleSize < mnMaxRange-mnMinRange )
            {
                nTrackFlags     = 0;
                meScrollType    = SCROLL_DRAG;
                mnDragDraw      = SCRBAR_DRAW_THUMB;

                // Zusaetzliche Daten berechnen
                if ( GetStyle() & WB_HORZ )
                    mnMouseOff = rMousePos.X()-maThumbRect.Left();
                else
                    mnMouseOff = rMousePos.Y()-maThumbRect.Top();

                // Im OS2-Look geben wir den Thumb gedrueck aus
                if ( GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_OS2STYLE )
                {
                    mnStateFlags |= SCRBAR_STATE_THUMB_DOWN;
                    ImplDraw( mnDragDraw );
                }
            }
            else
                Sound::Beep( SOUND_DISABLE, this );
        }
        else
        {
            nTrackFlags = STARTTRACK_BUTTONREPEAT;

            if ( maPage1Rect.IsInside( rMousePos ) )
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

        // Soll Tracking gestartet werden
        if ( meScrollType != SCROLL_DONTKNOW )
        {
            // Startposition merken fuer Abbruch und EndScroll-Delta
            mnStartPos = mnThumbPos;
            ImplDoMouseAction( rMousePos );
            StartTracking( nTrackFlags );
        }
    }
}

// -----------------------------------------------------------------------

void ScrollBar::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        // Button und PageRect-Status wieder herstellen
        USHORT nOldStateFlags = mnStateFlags;
        mnStateFlags &= ~(SCRBAR_STATE_BTN1_DOWN | SCRBAR_STATE_BTN2_DOWN |
                          SCRBAR_STATE_PAGE1_DOWN | SCRBAR_STATE_PAGE2_DOWN |
                          SCRBAR_STATE_THUMB_DOWN);
        if ( nOldStateFlags != mnStateFlags )
            ImplDraw( mnDragDraw );
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
    }
    else
    {
        const Point rMousePos = rTEvt.GetMouseEvent().GetPosPixel();

        // Dragging wird speziell behandelt
        if ( meScrollType == SCROLL_DRAG )
        {
            long nMovePix;
            if ( GetStyle() & WB_HORZ )
                nMovePix = rMousePos.X()-(maThumbRect.Left()+mnMouseOff);
            else
                nMovePix = rMousePos.Y()-(maThumbRect.Top()+mnMouseOff);
            // Nur wenn sich Maus in die Scrollrichtung bewegt, muessen
            // wir etwas tun
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

void ScrollBar::Paint( const Rectangle& rRect )
{
    ImplDraw( SCRBAR_DRAW_ALL );
}

// -----------------------------------------------------------------------

void ScrollBar::Resize()
{
    mbCalcSize = TRUE;
    if ( IsReallyVisible() )
        ImplCalc( FALSE );
    Invalidate();
}

#ifndef TF_SVDATA

// -----------------------------------------------------------------------

BOOL ScrollBar::QueryDrop( DropEvent& rDEvt )
{
    if ( mbDDScroll )
        DDScroll( rDEvt );
    return Control::QueryDrop( rDEvt );
}

#endif

// -----------------------------------------------------------------------

void ScrollBar::StateChanged( StateChangedType nType )
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
            if ( (GetPrevStyle() & SCRBAR_VIEW_STYLE) !=
                 (GetStyle() & SCRBAR_VIEW_STYLE) )
            {
                mbCalcSize = TRUE;
                ImplCalc( FALSE );
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
        Invalidate();
}

// -----------------------------------------------------------------------

void ScrollBar::Scroll()
{
    maScrollHdl.Call( this );
}

// -----------------------------------------------------------------------

void ScrollBar::EndScroll()
{
    maEndScrollHdl.Call( this );
}

// -----------------------------------------------------------------------

long ScrollBar::DoScroll( long nNewPos )
{
    if ( meScrollType != SCROLL_DONTKNOW )
        return 0;

    meScrollType = SCROLL_DRAG;
    long nDelta = ImplScroll( nNewPos, TRUE );
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
    long nDelta = ImplDoAction( TRUE );
    meScrollType = SCROLL_DONTKNOW;
    return nDelta;
}

// -----------------------------------------------------------------------

#ifndef TF_SVDATA

void ScrollBar::DDScroll( const DropEvent& rDEvt )
{
    if ( rDEvt.IsLeaveWindow() )
        meDDScrollType = SCROLL_DONTKNOW;
    else
    {
        const Point& rMousePos = rDEvt.GetPosPixel();
        if ( maBtn1Rect.IsInside( rMousePos ) )
        {
            if ( !(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) )
                meDDScrollType = SCROLL_LINEUP;
        }
        else if ( maBtn2Rect.IsInside( rMousePos ) )
        {
            if ( !(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) )
                meDDScrollType = SCROLL_LINEDOWN;
        }
        else if ( maThumbRect.IsInside( rMousePos ) )
            meDDScrollType = SCROLL_DONTKNOW;
        else
        {
            if ( maPage1Rect.IsInside( rMousePos ) )
                meDDScrollType = SCROLL_PAGEUP;
            else
                meDDScrollType = SCROLL_PAGEDOWN;
        }
    }

    if ( meDDScrollType == SCROLL_DONTKNOW )
    {
        if ( mpDDScrollTimer )
        {
            delete mpDDScrollTimer;
            mpDDScrollTimer = NULL;
            meDDScrollType = SCROLL_DONTKNOW;
        }
    }
    else
    {
        if ( !mpDDScrollTimer )
        {
            mpDDScrollTimer = new Timer;
            mpDDScrollTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonStartRepeat() );
            mpDDScrollTimer->SetTimeoutHdl( LINK( this, ScrollBar, ImplTimerHdl ) );
            mpDDScrollTimer->Start();
        }
    }
}

#endif

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

ScrollBarBox::ScrollBarBox( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_SCROLLBARBOX )
{
    rResId.SetRT( RSC_SCROLLBAR );
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
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
