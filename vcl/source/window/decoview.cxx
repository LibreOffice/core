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

#include <vcl/settings.hxx>
#include <vcl/outdev.hxx>
#include <vcl/decoview.hxx>
#include <vcl/window.hxx>
#include <vcl/ctrl.hxx>

// =======================================================================

#define BUTTON_DRAW_FLATTEST    (BUTTON_DRAW_FLAT |             \
                                 BUTTON_DRAW_PRESSED |          \
                                 BUTTON_DRAW_CHECKED |          \
                                 BUTTON_DRAW_HIGHLIGHT)

// =======================================================================

namespace {

long AdjustRectToSquare( Rectangle &rRect )
{
    const long nWidth = rRect.GetWidth();
    const long nHeight = rRect.GetHeight();
    long nSide = Min( nWidth, nHeight );

    if ( nSide && !(nSide & 1) )
    {
        // we prefer an odd size
        --nSide;
    }

    // Make the rectangle a square
    rRect.SetSize( Size( nSide, nSide ) );

    // and place it at the center of the original rectangle
    rRect.Move( (nWidth-nSide)/2, (nHeight-nSide)/2 );

    return nSide;
}

void ImplDrawSymbol( OutputDevice* pDev, Rectangle nRect, const SymbolType eType  )
{
    const long nSide = AdjustRectToSquare( nRect );

    if ( !nSide ) return;
    if ( nSide==1 )
    {
        pDev->DrawPixel( Point( nRect.Left(), nRect.Top() ) );
        return;
    }

    // Precalculate some values
    const long n2 = nSide/2;
    const long n4 = (n2+1)/2;
    const long n8 = (n4+1)/2;
    const Point aCenter = nRect.Center();

    switch ( eType )
    {
        case SYMBOL_ARROW_UP:
            pDev->DrawPixel( Point( aCenter.X(), nRect.Top() ) );
            for ( long i=1; i <= n2; ++i )
            {
                ++nRect.Top();
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Top() ),
                                Point( aCenter.X()+i, nRect.Top() ) );
            }
            pDev->DrawRect( Rectangle( aCenter.X()-n8, nRect.Top()+1,
                                       aCenter.X()+n8, nRect.Bottom() ) );
            break;

        case SYMBOL_ARROW_DOWN:
            pDev->DrawPixel( Point( aCenter.X(), nRect.Bottom() ) );
            for ( long i=1; i <= n2; ++i )
            {
                --nRect.Bottom();
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Bottom() ),
                                Point( aCenter.X()+i, nRect.Bottom() ) );
            }
            pDev->DrawRect( Rectangle( aCenter.X()-n8, nRect.Top(),
                                       aCenter.X()+n8, nRect.Bottom()-1 ) );
            break;

        case SYMBOL_ARROW_LEFT:
            pDev->DrawPixel( Point( nRect.Left(), aCenter.Y() ) );
            for ( long i=1; i <= n2; ++i )
            {
                ++nRect.Left();
                pDev->DrawLine( Point( nRect.Left(), aCenter.Y()-i ),
                                Point( nRect.Left(), aCenter.Y()+i ) );
            }
            pDev->DrawRect( Rectangle( nRect.Left()+1, aCenter.Y()-n8,
                                       nRect.Right(), aCenter.Y()+n8 ) );
            break;

        case SYMBOL_ARROW_RIGHT:
            pDev->DrawPixel( Point( nRect.Right(), aCenter.Y() ) );
            for ( long i=1; i <= n2; ++i )
            {
                --nRect.Right();
                pDev->DrawLine( Point( nRect.Right(), aCenter.Y()-i ),
                                Point( nRect.Right(), aCenter.Y()+i ) );
            }
            pDev->DrawRect( Rectangle( nRect.Left(), aCenter.Y()-n8,
                                       nRect.Right()-1, aCenter.Y()+n8 ) );
            break;


        case SYMBOL_SPIN_UP:
            nRect.Top() += n4;
            pDev->DrawPixel( Point( aCenter.X(), nRect.Top() ) );
            for ( long i=1; i <= n2; ++i )
            {
                ++nRect.Top();
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Top() ),
                                Point( aCenter.X()+i, nRect.Top() ) );
            }
            break;

        case SYMBOL_SPIN_DOWN:
            nRect.Bottom() -= n4;
            pDev->DrawPixel( Point( aCenter.X(), nRect.Bottom() ) );
            for ( long i=1; i <= n2; ++i )
            {
                --nRect.Bottom();
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Bottom() ),
                                Point( aCenter.X()+i, nRect.Bottom() ) );
            }
            break;

        case SYMBOL_SPIN_LEFT:
        case SYMBOL_FIRST:
        case SYMBOL_PREV:
        case SYMBOL_REVERSEPLAY:
            nRect.Left() += n4;
            if ( eType==SYMBOL_FIRST )
            {
                pDev->DrawLine( Point( nRect.Left(), nRect.Top() ),
                                Point( nRect.Left(), nRect.Bottom() ) );
                ++nRect.Left();
            }
            pDev->DrawPixel( Point( nRect.Left(), aCenter.Y() ) );
            for ( long i=1; i <= n2; ++i )
            {
                ++nRect.Left();
                pDev->DrawLine( Point( nRect.Left(), aCenter.Y()-i ),
                                Point( nRect.Left(), aCenter.Y()+i ) );
            }
            break;

        case SYMBOL_SPIN_RIGHT:
        case SYMBOL_LAST:
        case SYMBOL_NEXT:
        case SYMBOL_PLAY:
            nRect.Right() -= n4;
            if ( eType==SYMBOL_LAST )
            {
                pDev->DrawLine( Point( nRect.Right(), nRect.Top() ),
                                Point( nRect.Right(), nRect.Bottom() ) );
                --nRect.Right();
            }
            pDev->DrawPixel( Point( nRect.Right(), aCenter.Y() ) );
            for ( long i=1; i <= n2; ++i )
            {
                --nRect.Right();
                pDev->DrawLine( Point( nRect.Right(), aCenter.Y()-i ),
                                Point( nRect.Right(), aCenter.Y()+i ) );
            }
            break;

        case SYMBOL_PAGEUP:
            pDev->DrawPixel( Point( aCenter.X(), nRect.Top() ) );
            pDev->DrawPixel( Point( aCenter.X(), nRect.Top()+n2 ) );
            for ( long i=1; i < n2; ++i )
            {
                ++nRect.Top();
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Top() ),
                                Point( aCenter.X()+i, nRect.Top() ) );
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Top()+n2 ),
                                Point( aCenter.X()+i, nRect.Top()+n2 ) );
            }
            break;

        case SYMBOL_PAGEDOWN:
            pDev->DrawPixel( Point( aCenter.X(), nRect.Bottom() ) );
            pDev->DrawPixel( Point( aCenter.X(), nRect.Bottom()-n2 ) );
            for ( long i=1; i < n2; ++i )
            {
                --nRect.Bottom();
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Bottom() ),
                                Point( aCenter.X()+i, nRect.Bottom() ) );
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Bottom()-n2 ),
                                Point( aCenter.X()+i, nRect.Bottom()-n2 ) );
            }
            break;

        case SYMBOL_RADIOCHECKMARK:
        case SYMBOL_RECORD:
            {
                // Midpoint circle algorithm
                long x = 0;
                long y = n2;
                long p = 1 - n2;
                // Draw central line
                pDev->DrawLine( Point( aCenter.X(), aCenter.Y()-y ),
                                Point( aCenter.X(), aCenter.Y()+y ) );
                while ( x<y )
                {
                    if ( p>=0 )
                    {
                        // Draw vertical lines close to sides
                        pDev->DrawLine( Point( aCenter.X()+y, aCenter.Y()-x ),
                                        Point( aCenter.X()+y, aCenter.Y()+x ) );
                        pDev->DrawLine( Point( aCenter.X()-y, aCenter.Y()-x ),
                                        Point( aCenter.X()-y, aCenter.Y()+x ) );
                        --y;
                        p -= 2*y;
                    }
                    ++x;
                    p += 2*x+1;
                    // Draw vertical lines close to center
                    pDev->DrawLine( Point( aCenter.X()-x, aCenter.Y()-y ),
                                    Point( aCenter.X()-x, aCenter.Y()+y ) );
                    pDev->DrawLine( Point( aCenter.X()+x, aCenter.Y()-y ),
                                    Point( aCenter.X()+x, aCenter.Y()+y ) );
                }
            }
            break;

        case SYMBOL_STOP:
            pDev->DrawRect( nRect );
            break;

        case SYMBOL_PAUSE:
            pDev->DrawRect( Rectangle ( nRect.Left(), nRect.Top(),
                                        aCenter.X()-n8, nRect.Bottom() ) );
            pDev->DrawRect( Rectangle ( aCenter.X()+n8, nRect.Top(),
                                        nRect.Right(), nRect.Bottom() ) );
            break;

        case SYMBOL_WINDSTART:
            pDev->DrawLine( Point( nRect.Left(), aCenter.Y()-n2+1 ),
                            Point( nRect.Left(), aCenter.Y()+n2-1 ) );
            ++nRect.Left();
            // Intentional fall-through
        case SYMBOL_WINDBACKWARD:
            pDev->DrawPixel( Point( nRect.Left(), aCenter.Y() ) );
            pDev->DrawPixel( Point( nRect.Left()+n2, aCenter.Y() ) );
            for ( long i=1; i < n2; ++i )
            {
                ++nRect.Left();
                pDev->DrawLine( Point( nRect.Left(), aCenter.Y()-i ),
                                Point( nRect.Left(), aCenter.Y()+i ) );
                pDev->DrawLine( Point( nRect.Left()+n2, aCenter.Y()-i ),
                                Point( nRect.Left()+n2, aCenter.Y()+i ) );
            }
            break;

        case SYMBOL_WINDEND:
            pDev->DrawLine( Point( nRect.Right(), aCenter.Y()-n2+1 ),
                            Point( nRect.Right(), aCenter.Y()+n2-1 ) );
            --nRect.Right();
            // Intentional fall-through
        case SYMBOL_WINDFORWARD:
            pDev->DrawPixel( Point( nRect.Right(), aCenter.Y() ) );
            pDev->DrawPixel( Point( nRect.Right()-n2, aCenter.Y() ) );
            for ( long i=1; i < n2; ++i )
            {
                --nRect.Right();
                pDev->DrawLine( Point( nRect.Right(), aCenter.Y()-i ),
                                Point( nRect.Right(), aCenter.Y()+i ) );
                pDev->DrawLine( Point( nRect.Right()-n2, aCenter.Y()-i ),
                                Point( nRect.Right()-n2, aCenter.Y()+i ) );
            }
            break;

        case SYMBOL_CLOSE:
            pDev->DrawLine( Point( nRect.Left(), nRect.Top() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Bottom() ),
                            Point( nRect.Right(), nRect.Top() ) );
            for ( long i=1; i<n8; ++i )
            {
                pDev->DrawLine( Point( nRect.Left()+i, nRect.Top() ),
                                Point( nRect.Right(), nRect.Bottom()-i ) );
                pDev->DrawLine( Point( nRect.Left(), nRect.Top()+i ),
                                Point( nRect.Right()-i, nRect.Bottom() ) );
                pDev->DrawLine( Point( nRect.Left()+i, nRect.Bottom() ),
                                Point( nRect.Right(), nRect.Top()+i ) );
                pDev->DrawLine( Point( nRect.Left(), nRect.Bottom()-i ),
                                Point( nRect.Right()-i, nRect.Top() ) );
            }
            break;

        case SYMBOL_ROLLDOWN:
            pDev->DrawLine( Point( nRect.Left(), nRect.Top() ),
                            Point( nRect.Left(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Right(), nRect.Top() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Bottom() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            // Intentional fall-through
        case SYMBOL_ROLLUP:
            pDev->DrawRect( Rectangle( nRect.Left(), nRect.Top(),
                                       nRect.Right(), nRect.Top()+n8 ) );
            break;

        case SYMBOL_CHECKMARK:
            {
                long n3 = nSide/3;
                nRect.Top() -= n3/2;
                nRect.Bottom() -= n3/2;
                // #106953# never mirror checkmarks
                if ( pDev->ImplHasMirroredGraphics() && pDev->IsRTLEnabled() )
                {
                    // Draw a mirrored checkmark so that it looks "normal" in a
                    // mirrored graphics device (double mirroring!)
                    pDev->DrawLine( Point( nRect.Right(), nRect.Bottom()-n3 ),
                                    Point( nRect.Right()-n3, nRect.Bottom() ) );
                    pDev->DrawLine( Point( nRect.Right()-n3, nRect.Bottom() ),
                                    Point( nRect.Left(), nRect.Top()+n3 ) );
                    ++nRect.Top();
                    ++nRect.Bottom();
                    pDev->DrawLine( Point( nRect.Right(), nRect.Bottom()-n3 ),
                                    Point( nRect.Right()-n3, nRect.Bottom() ) );
                    pDev->DrawLine( Point( nRect.Right()-n3, nRect.Bottom() ),
                                    Point( nRect.Left(), nRect.Top()+n3 ) );
                }
                else
                {
                    pDev->DrawLine( Point( nRect.Left(), nRect.Bottom()-n3 ),
                                    Point( nRect.Left()+n3, nRect.Bottom() ) );
                    pDev->DrawLine( Point( nRect.Left()+n3, nRect.Bottom() ),
                                    Point( nRect.Right(), nRect.Top()+n3 ) );
                    ++nRect.Top();
                    ++nRect.Bottom();
                    pDev->DrawLine( Point( nRect.Left(), nRect.Bottom()-n3 ),
                                    Point( nRect.Left()+n3, nRect.Bottom() ) );
                    pDev->DrawLine( Point( nRect.Left()+n3, nRect.Bottom() ),
                                    Point( nRect.Right(), nRect.Top()+n3 ) );
                }
            }
            break;

        case SYMBOL_SPIN_UPDOWN:
            pDev->DrawPixel( Point( aCenter.X(), nRect.Top() ) );
            pDev->DrawPixel( Point( aCenter.X(), nRect.Bottom() ) );
            for ( long i=1; i < n2; ++i )
            {
                ++nRect.Top();
                --nRect.Bottom();
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Top() ),
                                Point( aCenter.X()+i, nRect.Top() ) );
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Bottom() ),
                                Point( aCenter.X()+i, nRect.Bottom() ) );
            }
            break;

        case SYMBOL_FLOAT:
            nRect.Right() -= n4;
            nRect.Top() += n4+1;
            pDev->DrawRect( Rectangle( nRect.Left(), nRect.Top(),
                                       nRect.Right(), nRect.Top()+n8 ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Top()+n8 ),
                            Point( nRect.Left(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Bottom() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Right(), nRect.Top()+n8 ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            nRect.Right() += n4;
            nRect.Top() -= n4+1;
            nRect.Left() += n4;
            nRect.Bottom() -= n4+1;
            pDev->DrawRect( Rectangle( nRect.Left(), nRect.Top(),
                                       nRect.Right(), nRect.Top()+n8 ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Top()+n8 ),
                            Point( nRect.Left(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Bottom() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Right(), nRect.Top()+n8 ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            break;

        case SYMBOL_DOCK:
            pDev->DrawLine( Point( nRect.Left(), nRect.Top() ),
                            Point( nRect.Right(), nRect.Top() ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Top() ),
                            Point( nRect.Left(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Bottom() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Right(), nRect.Top() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            break;

        case SYMBOL_HIDE:
            pDev->DrawRect( Rectangle( nRect.Left()+n8, nRect.Bottom()-n8,
                                       nRect.Right()-n8, nRect.Bottom() ) );
            break;

        case SYMBOL_PLUS:
            pDev->DrawRect( Rectangle( nRect.Left(), aCenter.Y()-n8,
                                       nRect.Right(), aCenter.Y()+n8 ) );
            pDev->DrawRect( Rectangle( aCenter.X()-n8, nRect.Top(),
                                       aCenter.X()+n8, nRect.Bottom() ) );
            break;
    }
}


void ImplDrawDPILineRect( OutputDevice *const pDev, Rectangle& rRect,
                          const Color *const pColor, const bool bRound = false )
{
    long nLineWidth = pDev->ImplGetDPIX()/300;
    long nLineHeight = pDev->ImplGetDPIY()/300;
    if ( !nLineWidth )
        nLineWidth = 1;
    if ( !nLineHeight )
        nLineHeight = 1;

    if ( pColor )
    {
        if ( (nLineWidth == 1) && (nLineHeight == 1) )
        {
            pDev->SetLineColor( *pColor );
            if( bRound )
            {
                pDev->DrawLine( Point( rRect.Left()+1, rRect.Top()), Point( rRect.Right()-1, rRect.Top()) );
                pDev->DrawLine( Point( rRect.Left()+1, rRect.Bottom()), Point( rRect.Right()-1, rRect.Bottom()) );
                pDev->DrawLine( Point( rRect.Left(), rRect.Top()+1), Point( rRect.Left(), rRect.Bottom()-1) );
                pDev->DrawLine( Point( rRect.Right(), rRect.Top()+1), Point( rRect.Right(), rRect.Bottom()-1) );
            }
            else
            {
                pDev->SetFillColor();
                pDev->DrawRect( rRect );
            }
        }
        else
        {
            const long nWidth = rRect.GetWidth();
            const long nHeight = rRect.GetHeight();
            pDev->SetLineColor();
            pDev->SetFillColor( *pColor );
            pDev->DrawRect( Rectangle( rRect.TopLeft(), Size( nWidth, nLineHeight ) ) );
            pDev->DrawRect( Rectangle( rRect.TopLeft(), Size( nLineWidth, nHeight ) ) );
            pDev->DrawRect( Rectangle( Point( rRect.Left(), rRect.Bottom()-nLineHeight ),
                                       Size( nWidth, nLineHeight ) ) );
            pDev->DrawRect( Rectangle( Point( rRect.Right()-nLineWidth, rRect.Top() ),
                                       Size( nLineWidth, nHeight ) ) );
        }
    }

    rRect.Left()   += nLineWidth;
    rRect.Top()    += nLineHeight;
    rRect.Right()  -= nLineWidth;
    rRect.Bottom() -= nLineHeight;
}


void ImplDraw2ColorFrame( OutputDevice *const pDev, Rectangle& rRect,
                          const Color& rLeftTopColor, const Color& rRightBottomColor )
{
    pDev->SetLineColor( rLeftTopColor );
    pDev->DrawLine( rRect.TopLeft(), rRect.BottomLeft() );
    pDev->DrawLine( rRect.TopLeft(), rRect.TopRight() );
    pDev->SetLineColor( rRightBottomColor );
    pDev->DrawLine( rRect.BottomLeft(), rRect.BottomRight() );
    pDev->DrawLine( rRect.TopRight(), rRect.BottomRight() );

    // reduce drawing area
    ++rRect.Left();
    ++rRect.Top();
    --rRect.Right();
    --rRect.Bottom();
}


void ImplDrawButton( OutputDevice *const pDev, Rectangle aFillRect,
                     const sal_uInt16 nStyle )
{
    const StyleSettings& rStyleSettings = pDev->GetSettings().GetStyleSettings();

    if ( (nStyle & BUTTON_DRAW_MONO) ||
         (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
    {
        const Color aBlackColor( COL_BLACK );

        if ( nStyle & BUTTON_DRAW_DEFAULT )
        {
            // default selection shows a wider border
            ImplDrawDPILineRect( pDev, aFillRect, &aBlackColor );
        }

        ImplDrawDPILineRect( pDev, aFillRect, &aBlackColor );

        Size aBrdSize( 1, 1 );
        if ( pDev->GetOutDevType() == OUTDEV_PRINTER )
        {
            aBrdSize = pDev->LogicToPixel( Size( 20, 20 ), MapMode(MAP_100TH_MM) );
            if ( !aBrdSize.Width() )
                aBrdSize.Width() = 1;
            if ( !aBrdSize.Height() )
                aBrdSize.Height() = 1;
        }

        pDev->SetLineColor();
        pDev->SetFillColor( aBlackColor );
        const Rectangle aOrigFillRect(aFillRect);
        if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
        {
            // shrink fill rect
            aFillRect.Left() += aBrdSize.Width();
            aFillRect.Top()  += aBrdSize.Height();
            // draw top and left borders (aOrigFillRect-aFillRect)
            pDev->DrawRect( Rectangle( aOrigFillRect.Left(), aOrigFillRect.Top(),
                                       aOrigFillRect.Right(), aFillRect.Top()-1 ) );
            pDev->DrawRect( Rectangle( aOrigFillRect.Left(), aOrigFillRect.Top(),
                                       aFillRect.Left()-1, aOrigFillRect.Bottom() ) );
        }
        else
        {
            // shrink fill rect
            aFillRect.Right()  -= aBrdSize.Width();
            aFillRect.Bottom() -= aBrdSize.Height();
            // draw bottom and right borders (aOrigFillRect-aFillRect)
            pDev->DrawRect( Rectangle( aOrigFillRect.Left(), aFillRect.Bottom()+1,
                                       aOrigFillRect.Right(), aOrigFillRect.Bottom() ) );
            pDev->DrawRect( Rectangle( aFillRect.Right()+1, aOrigFillRect.Top(),
                                       aOrigFillRect.Right(), aOrigFillRect.Bottom() ) );
        }

        if ( !(nStyle & BUTTON_DRAW_NOFILL) )
        {
            // Hack: Auf Druckern wollen wir im MonoChrom-Modus trotzdem
            // erstmal graue Buttons haben
            if ( pDev->GetOutDevType() == OUTDEV_PRINTER )
                pDev->SetFillColor( Color( COL_LIGHTGRAY ) );
            else
                pDev->SetFillColor( Color( COL_WHITE ) );
            pDev->DrawRect( aFillRect );
        }
    }
    else
    {
        if ( nStyle & BUTTON_DRAW_DEFAULT )
        {
            const Color aDefBtnColor = rStyleSettings.GetDarkShadowColor();
            ImplDrawDPILineRect( pDev, aFillRect, &aDefBtnColor );
        }

        if ( nStyle & BUTTON_DRAW_NOLEFTLIGHTBORDER )
        {
            pDev->SetLineColor( rStyleSettings.GetLightBorderColor() );
            pDev->DrawLine( Point( aFillRect.Left(), aFillRect.Top() ),
                            Point( aFillRect.Left(), aFillRect.Bottom() ) );
            ++aFillRect.Left();
        }

        Color aColor1;
        Color aColor2;
        if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
        {
            aColor1 = rStyleSettings.GetDarkShadowColor();
            aColor2 = rStyleSettings.GetLightColor();
        }
        else
        {
            if ( nStyle & BUTTON_DRAW_NOLIGHTBORDER )
                aColor1 = rStyleSettings.GetLightBorderColor();
            else
                aColor1 = rStyleSettings.GetLightColor();
            if ( (nStyle & BUTTON_DRAW_FLATTEST) == BUTTON_DRAW_FLAT )
                aColor2 = rStyleSettings.GetShadowColor();
            else
                aColor2 = rStyleSettings.GetDarkShadowColor();
        }

        ImplDraw2ColorFrame( pDev, aFillRect, aColor1, aColor2 );

        if ( !((nStyle & BUTTON_DRAW_FLATTEST) == BUTTON_DRAW_FLAT) )
        {
            if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
            {
                aColor1 = rStyleSettings.GetShadowColor();
                aColor2 = rStyleSettings.GetLightBorderColor();
            }
            else
            {
                if ( nStyle & BUTTON_DRAW_NOLIGHTBORDER )
                    aColor1 = rStyleSettings.GetLightColor();
                else
                    aColor1 = rStyleSettings.GetLightBorderColor();
                aColor2 = rStyleSettings.GetShadowColor();
            }
            ImplDraw2ColorFrame( pDev, aFillRect, aColor1, aColor2 );
        }

        if ( !(nStyle & BUTTON_DRAW_NOFILL) )
        {
            pDev->SetLineColor();
            if ( nStyle & (BUTTON_DRAW_CHECKED | BUTTON_DRAW_DONTKNOW) )
                pDev->SetFillColor( rStyleSettings.GetCheckedColor() );
            else
                pDev->SetFillColor( rStyleSettings.GetFaceColor() );
            pDev->DrawRect( aFillRect );
        }
    }
}

void ImplDrawFrame( OutputDevice *const pDev, Rectangle& rRect,
                    const StyleSettings& rStyleSettings, sal_uInt16 nStyle )
{
    Window *const pWin = (pDev->GetOutDevType()==OUTDEV_WINDOW) ? (Window*) pDev : NULL;

    const bool bMenuStyle = nStyle & FRAME_DRAW_MENU;

    // UseFlatBorders disables 3D style for all frames except menus
    // menus may use different border colors (eg on XP)
    // normal frames will be drawn using the shadow color
    // whereas window frame borders will use black
    bool bFlatBorders = !bMenuStyle && rStyleSettings.GetUseFlatBorders();

    // no flat borders for standard VCL controls (ie formcontrols that keep their classic look)
    // will not affect frame windows (like dropdowns)
    if( bFlatBorders && pWin && pWin->GetType() == WINDOW_BORDERWINDOW && (pWin != pWin->ImplGetFrameWindow()) )
    {
        // check for formcontrol, i.e., a control without NWF enabled
        Control *const pControl = dynamic_cast< Control* >( pWin->GetWindow( WINDOW_CLIENT ) );
        if( !pControl || !pControl->IsNativeWidgetEnabled() )
            bFlatBorders = false;
    }

    const bool bNoDraw = nStyle & FRAME_DRAW_NODRAW;

    if ( (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) ||
         (pDev->GetOutDevType() == OUTDEV_PRINTER) ||
         bFlatBorders )
        nStyle |= FRAME_DRAW_MONO;

    if( (nStyle & FRAME_DRAW_STYLE) != FRAME_DRAW_NWF &&
        pWin && pWin->IsNativeControlSupported(CTRL_FRAME, PART_BORDER) )
    {
        ImplControlValue aControlValue( nStyle |
                                        (pWin->GetType()==WINDOW_BORDERWINDOW ?
                                        FRAME_DRAW_BORDERWINDOWBORDER : 0) );
        Rectangle aBound, aContent;
        Rectangle aNatRgn( rRect );
        if( pWin->GetNativeControlRegion(CTRL_FRAME, PART_BORDER,
            aNatRgn, 0, aControlValue, rtl::OUString(), aBound, aContent) )
        {
            // if bNoDraw is true then don't call the drawing routine
            // but just update the target rectangle
            if( bNoDraw ||
                pWin->DrawNativeControl( CTRL_FRAME, PART_BORDER, aContent, CTRL_STATE_ENABLED,
                                         aControlValue, rtl::OUString()) )
            {
                rRect = aContent;
                return;
            }
        }
    }

    if ( nStyle & FRAME_DRAW_MONO )
    {
        // no round corners for window frame borders
        const bool bRound = bFlatBorders && !(nStyle & FRAME_DRAW_WINDOWBORDER);

        if ( bNoDraw )
        {
            ImplDrawDPILineRect( pDev, rRect, NULL, bRound );
        }
        else
        {
            Color aColor = bRound ? rStyleSettings.GetShadowColor()
                                  : pDev->GetSettings().GetStyleSettings().GetMonoColor();
            // when the MonoColor wasn't set, check face color
            if (
                (bRound && aColor.IsDark()) ||
                (
                  (aColor == Color(COL_BLACK)) &&
                  pDev->GetSettings().GetStyleSettings().GetFaceColor().IsDark()
                )
               )
            {
                aColor = Color( COL_WHITE );
            }
            ImplDrawDPILineRect( pDev, rRect, &aColor, bRound );
        }
    }
    else
    {
        if ( bNoDraw )
        {
            switch ( nStyle & FRAME_DRAW_STYLE )
            {
                case FRAME_DRAW_IN:
                case FRAME_DRAW_OUT:
                    ++rRect.Left();
                    ++rRect.Top();
                    --rRect.Right();
                    --rRect.Bottom();
                    break;

                case FRAME_DRAW_GROUP:
                case FRAME_DRAW_DOUBLEIN:
                case FRAME_DRAW_DOUBLEOUT:
                    rRect.Left()   += 2;
                    rRect.Top()    += 2;
                    rRect.Right()  -= 2;
                    rRect.Bottom() -= 2;
                    break;

                case FRAME_DRAW_NWF:
                    // enough space for the native rendering
                    rRect.Left() += 4;
                    rRect.Top() += 4;
                    rRect.Right() -= 4;
                    rRect.Bottom() -= 4;
                    break;
            }
        }
        else
        {
            switch ( nStyle & FRAME_DRAW_STYLE )
            {
                case FRAME_DRAW_GROUP:
                    pDev->SetFillColor();
                    pDev->SetLineColor( rStyleSettings.GetLightColor() );
                    pDev->DrawRect( Rectangle( rRect.Left()+1, rRect.Top()+1,
                                            rRect.Right(), rRect.Bottom() ) );
                    pDev->SetLineColor( rStyleSettings.GetShadowColor() );
                    pDev->DrawRect( Rectangle( rRect.Left(), rRect.Top(),
                                            rRect.Right()-1, rRect.Bottom()-1 ) );

                    // adjust target rectangle
                    rRect.Left()   += 2;
                    rRect.Top()    += 2;
                    rRect.Right()  -= 2;
                    rRect.Bottom() -= 2;
                    break;

                case FRAME_DRAW_IN:
                    ImplDraw2ColorFrame( pDev, rRect,
                                         rStyleSettings.GetShadowColor(),
                                         rStyleSettings.GetLightColor() );
                    break;

                case FRAME_DRAW_OUT:
                    ImplDraw2ColorFrame( pDev, rRect,
                                         rStyleSettings.GetLightColor(),
                                         rStyleSettings.GetShadowColor() );
                    break;

                case FRAME_DRAW_DOUBLEIN:
                    if( bFlatBorders )
                    {
                        // no 3d effect
                        ImplDraw2ColorFrame( pDev, rRect,
                                             rStyleSettings.GetShadowColor(),
                                             rStyleSettings.GetShadowColor() );
                        ImplDraw2ColorFrame( pDev, rRect,
                                             rStyleSettings.GetFaceColor(),
                                             rStyleSettings.GetFaceColor() );
                    }
                    else
                    {
                        ImplDraw2ColorFrame( pDev, rRect,
                                             rStyleSettings.GetShadowColor(),
                                             rStyleSettings.GetLightColor() );
                        ImplDraw2ColorFrame( pDev, rRect,
                                             rStyleSettings.GetDarkShadowColor(),
                                             rStyleSettings.GetLightBorderColor() );
                    }
                    break;

                case FRAME_DRAW_DOUBLEOUT:
                    if( bMenuStyle )
                    {
                        ImplDraw2ColorFrame( pDev, rRect,
                                             rStyleSettings.GetMenuBorderColor(),
                                             rStyleSettings.GetDarkShadowColor() );
                        if ( !rStyleSettings.GetUseFlatMenues() )
                        {
                            ImplDraw2ColorFrame( pDev, rRect,
                                                 rStyleSettings.GetLightColor(),
                                                 rStyleSettings.GetShadowColor() );
                        }
                    }
                    else
                    {
                        ImplDraw2ColorFrame( pDev, rRect,
                                             bFlatBorders ? // no 3d effect
                                             rStyleSettings.GetDarkShadowColor() :
                                             rStyleSettings.GetLightBorderColor(),
                                             rStyleSettings.GetDarkShadowColor() );
                        ImplDraw2ColorFrame( pDev, rRect,
                                             rStyleSettings.GetLightColor(),
                                             rStyleSettings.GetShadowColor() );
                    }
                    break;

                case FRAME_DRAW_NWF:
                    // no rendering, just enough space for the native rendering
                    rRect.Left() += 4;
                    rRect.Top() += 4;
                    rRect.Right() -= 4;
                    rRect.Bottom() -= 4;
                    break;
            }
        }
    }
}

}


// -----------------------------------------------------------------------

void DecorationView::DrawSymbol( const Rectangle& rRect, SymbolType eType,
                                 const Color& rColor, sal_uInt16 nStyle )
{
    const StyleSettings&    rStyleSettings  = mpOutDev->GetSettings().GetStyleSettings();
    const Rectangle         aRect           = mpOutDev->LogicToPixel( rRect );
    const Color             aOldLineColor   = mpOutDev->GetLineColor();
    const Color             aOldFillColor   = mpOutDev->GetFillColor();
    const bool              bOldMapMode     = mpOutDev->IsMapModeEnabled();
    Color                   nColor(rColor);
    mpOutDev->EnableMapMode( sal_False );

    if ( (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) ||
         (mpOutDev->GetOutDevType() == OUTDEV_PRINTER) )
        nStyle |= BUTTON_DRAW_MONO;

    if ( nStyle & SYMBOL_DRAW_MONO )
    {
        // Monochrome: set color to black if enabled, to gray if disabled
        nColor = Color( ( nStyle & SYMBOL_DRAW_DISABLE ) ? COL_GRAY : COL_BLACK );
    }
    else
    {
        if ( nStyle & SYMBOL_DRAW_DISABLE )
        {
            // Draw shifted and brighter symbol for embossed look
            mpOutDev->SetLineColor( rStyleSettings.GetLightColor() );
            mpOutDev->SetFillColor( rStyleSettings.GetLightColor() );
            ImplDrawSymbol( mpOutDev, aRect + Point(1, 1) , eType );
            nColor = rStyleSettings.GetShadowColor();
        }
    }

    // Set selected color and draw the symbol
    mpOutDev->SetLineColor( nColor );
    mpOutDev->SetFillColor( nColor );
    ImplDrawSymbol( mpOutDev, aRect, eType );

    // Restore previous settings
    mpOutDev->SetLineColor( aOldLineColor );
    mpOutDev->SetFillColor( aOldFillColor );
    mpOutDev->EnableMapMode( bOldMapMode );
}

// =======================================================================

void DecorationView::DrawFrame( const Rectangle& rRect,
                                const Color& rLeftTopColor,
                                const Color& rRightBottomColor )
{
    Rectangle   aRect         = mpOutDev->LogicToPixel( rRect );
    const Color aOldLineColor = mpOutDev->GetLineColor();
    const bool  bOldMapMode   = mpOutDev->IsMapModeEnabled();
    mpOutDev->EnableMapMode( false );
    ImplDraw2ColorFrame( mpOutDev, aRect, rLeftTopColor, rRightBottomColor );
    mpOutDev->SetLineColor( aOldLineColor );
    mpOutDev->EnableMapMode( bOldMapMode );
}

// =======================================================================

void DecorationView::DrawHighlightFrame( const Rectangle& rRect,
                                         sal_uInt16 nStyle )
{
    const StyleSettings& rStyleSettings = mpOutDev->GetSettings().GetStyleSettings();
    Color aLightColor = rStyleSettings.GetLightColor();
    Color aShadowColor = rStyleSettings.GetShadowColor();

    if ( (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) ||
         (mpOutDev->GetOutDevType() == OUTDEV_PRINTER) )
    {
        aLightColor = Color( COL_BLACK );
        aShadowColor = Color( COL_BLACK );
    }
    else if ( nStyle & FRAME_HIGHLIGHT_TESTBACKGROUND )
    {
        Wallpaper aBackground = mpOutDev->GetBackground();
        if ( aBackground.IsBitmap() || aBackground.IsGradient() )
        {
            aLightColor = rStyleSettings.GetFaceColor();
            aShadowColor = Color( COL_BLACK );
        }
        else
        {
            Color aBackColor = aBackground.GetColor();
            if ( (aLightColor.GetColorError( aBackColor ) < 32) ||
                 (aShadowColor.GetColorError( aBackColor ) < 32) )
            {
                aLightColor = Color( COL_WHITE );
                aShadowColor = Color( COL_BLACK );

                if ( aLightColor.GetColorError( aBackColor ) < 32 )
                    aLightColor.DecreaseLuminance( 64 );
                if ( aShadowColor.GetColorError( aBackColor ) < 32 )
                    aShadowColor.IncreaseLuminance( 64 );
            }
        }
    }

    if ( (nStyle & FRAME_HIGHLIGHT_STYLE) == FRAME_HIGHLIGHT_IN )
    {
        Color aTempColor = aLightColor;
        aLightColor = aShadowColor;
        aShadowColor = aTempColor;
    }

    DrawFrame( rRect, aLightColor, aShadowColor );
}

// =======================================================================

Rectangle DecorationView::DrawFrame( const Rectangle& rRect, sal_uInt16 nStyle )
{
    Rectangle   aRect = rRect;
    sal_Bool        bOldMap = mpOutDev->IsMapModeEnabled();
    if ( bOldMap )
    {
        aRect = mpOutDev->LogicToPixel( aRect );
        mpOutDev->EnableMapMode( sal_False );
    }

    if ( !rRect.IsEmpty() )
    {
        if ( nStyle & FRAME_DRAW_NODRAW )
             ImplDrawFrame( mpOutDev, aRect, mpOutDev->GetSettings().GetStyleSettings(), nStyle );
        else
        {
             Color maOldLineColor  = mpOutDev->GetLineColor();
             Color maOldFillColor  = mpOutDev->GetFillColor();
             ImplDrawFrame( mpOutDev, aRect, mpOutDev->GetSettings().GetStyleSettings(), nStyle );
             mpOutDev->SetLineColor( maOldLineColor );
             mpOutDev->SetFillColor( maOldFillColor );
        }
    }

    if ( bOldMap )
    {
        mpOutDev->EnableMapMode( bOldMap );
        aRect = mpOutDev->PixelToLogic( aRect );
    }

    return aRect;
}

// =======================================================================

Rectangle DecorationView::DrawButton( const Rectangle& rRect, sal_uInt16 nStyle )
{
    if ( rRect.IsEmpty() )
    {
        return rRect;
    }

    Rectangle aRect = rRect;
    const bool bOldMap = mpOutDev->IsMapModeEnabled();

    if ( bOldMap )
    {
        aRect = mpOutDev->LogicToPixel( aRect );
        mpOutDev->EnableMapMode( false );
    }

    const Color maOldLineColor = mpOutDev->GetLineColor();
    const Color maOldFillColor = mpOutDev->GetFillColor();
    ImplDrawButton( mpOutDev, aRect, nStyle );
    mpOutDev->SetLineColor( maOldLineColor );
    mpOutDev->SetFillColor( maOldFillColor );

    // Ein Border freilassen, der jedoch bei Default-Darstellung
    // mitbenutzt wird
    ++aRect.Left();
    ++aRect.Top();
    --aRect.Right();
    --aRect.Bottom();

    if ( nStyle & BUTTON_DRAW_NOLIGHTBORDER )
    {
        ++aRect.Left();
        ++aRect.Top();
    }
    else if ( nStyle & BUTTON_DRAW_NOLEFTLIGHTBORDER )
    {
        ++aRect.Left();
    }

    if ( nStyle & BUTTON_DRAW_PRESSED )
    {
        if ( (aRect.GetHeight() > 10) && (aRect.GetWidth() > 10) )
        {
            aRect.Left()   += 4;
            aRect.Top()    += 4;
            aRect.Right()  -= 1;
            aRect.Bottom() -= 1;
        }
        else
        {
            aRect.Left()   += 3;
            aRect.Top()    += 3;
            aRect.Right()  -= 2;
            aRect.Bottom() -= 2;
        }
    }
    else if ( nStyle & BUTTON_DRAW_CHECKED )
    {
        aRect.Left()   += 3;
        aRect.Top()    += 3;
        aRect.Right()  -= 2;
        aRect.Bottom() -= 2;
    }
    else
    {
        aRect.Left()   += 2;
        aRect.Top()    += 2;
        aRect.Right()  -= 3;
        aRect.Bottom() -= 3;
    }

    if ( bOldMap )
    {
        mpOutDev->EnableMapMode( bOldMap );
        aRect = mpOutDev->PixelToLogic( aRect );
    }

    return aRect;
}

// -----------------------------------------------------------------------

void DecorationView::DrawSeparator( const Point& rStart, const Point& rStop, bool bVertical )
{
    Point aStart( rStart ), aStop( rStop );
    const StyleSettings& rStyleSettings = mpOutDev->GetSettings().GetStyleSettings();
    Window *const pWin = (mpOutDev->GetOutDevType()==OUTDEV_WINDOW) ? (Window*) mpOutDev: NULL;
    if(pWin)
    {
        ControlPart nPart = ( bVertical ? PART_SEPARATOR_VERT : PART_SEPARATOR_HORZ );
        bool nativeSupported = pWin->IsNativeControlSupported( CTRL_FIXEDLINE, nPart );
        ImplControlValue    aValue;
        ControlState        nState = 0;
        Rectangle aRect(rStart,rStop);
        if(nativeSupported && pWin->DrawNativeControl(CTRL_FIXEDLINE,nPart,aRect,nState,aValue,rtl::OUString()))
            return;
    }

    mpOutDev->Push( PUSH_LINECOLOR );
    if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
        mpOutDev->SetLineColor( Color( COL_BLACK ) );
    else
        mpOutDev->SetLineColor( rStyleSettings.GetShadowColor() );

    mpOutDev->DrawLine( aStart, aStop );
    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
    {
        mpOutDev->SetLineColor( rStyleSettings.GetLightColor() );
        if( bVertical )
        {
            aStart.X()++;
            aStop.X()++;
        }
        else
        {
            aStart.Y()++;
            aStop.Y()++;
        }
        mpOutDev->DrawLine( aStart, aStop );
    }
    mpOutDev->Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
