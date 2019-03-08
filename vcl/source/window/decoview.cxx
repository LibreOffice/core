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

#include <vcl/settings.hxx>
#include <vcl/outdev.hxx>
#include <vcl/decoview.hxx>
#include <vcl/window.hxx>
#include <vcl/ctrl.hxx>

static constexpr auto BUTTON_DRAW_FLATTEST = DrawButtonFlags::Flat |
                                 DrawButtonFlags::Pressed |
                                 DrawButtonFlags::Checked |
                                 DrawButtonFlags::Highlight;

using namespace std;

namespace {

long AdjustRectToSquare( tools::Rectangle &rRect )
{
    const long nWidth = rRect.GetWidth();
    const long nHeight = rRect.GetHeight();
    long nSide = std::min( nWidth, nHeight );

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

void ImplDrawSymbol( OutputDevice* pDev, tools::Rectangle nRect, const SymbolType eType  )
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
    const long n16 = (n8+1)/2;
    const Point aCenter = nRect.Center();

    switch ( eType )
    {
        case SymbolType::ARROW_UP:
            pDev->DrawPixel( Point( aCenter.X(), nRect.Top() ) );
            for ( long i=1; i <= n2; ++i )
            {
                nRect.AdjustTop( 1 );
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Top() ),
                                Point( aCenter.X()+i, nRect.Top() ) );
                pDev->DrawPixel( Point( aCenter.X()-i, nRect.Top() ) );
                pDev->DrawPixel( Point( aCenter.X()+i, nRect.Top() ) );
            }
            pDev->DrawRect( tools::Rectangle( aCenter.X()-n8, nRect.Top()+1,
                                       aCenter.X()+n8, nRect.Bottom() ) );
            break;

        case SymbolType::ARROW_DOWN:
            pDev->DrawPixel( Point( aCenter.X(), nRect.Bottom() ) );
            for ( long i=1; i <= n2; ++i )
            {
                nRect.AdjustBottom( -1 );
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Bottom() ),
                                Point( aCenter.X()+i, nRect.Bottom() ) );
                pDev->DrawPixel( Point( aCenter.X()-i, nRect.Bottom() ) );
                pDev->DrawPixel( Point( aCenter.X()+i, nRect.Bottom() ) );
            }
            pDev->DrawRect( tools::Rectangle( aCenter.X()-n8, nRect.Top(),
                                       aCenter.X()+n8, nRect.Bottom()-1 ) );
            break;

        case SymbolType::ARROW_LEFT:
            pDev->DrawPixel( Point( nRect.Left(), aCenter.Y() ) );
            for ( long i=1; i <= n2; ++i )
            {
                nRect.AdjustLeft( 1 );
                pDev->DrawLine( Point( nRect.Left(), aCenter.Y()-i ),
                                Point( nRect.Left(), aCenter.Y()+i ) );
                pDev->DrawPixel( Point( nRect.Left(), aCenter.Y()-i ) );
                pDev->DrawPixel( Point( nRect.Left(), aCenter.Y()+i ) );
            }
            pDev->DrawRect( tools::Rectangle( nRect.Left()+1, aCenter.Y()-n8,
                                       nRect.Right(), aCenter.Y()+n8 ) );
            break;

        case SymbolType::ARROW_RIGHT:
            pDev->DrawPixel( Point( nRect.Right(), aCenter.Y() ) );
            for ( long i=1; i <= n2; ++i )
            {
                nRect.AdjustRight( -1 );
                pDev->DrawLine( Point( nRect.Right(), aCenter.Y()-i ),
                                Point( nRect.Right(), aCenter.Y()+i ) );
                pDev->DrawPixel( Point( nRect.Right(), aCenter.Y()-i ) );
                pDev->DrawPixel( Point( nRect.Right(), aCenter.Y()+i ) );
            }
            pDev->DrawRect( tools::Rectangle( nRect.Left(), aCenter.Y()-n8,
                                       nRect.Right()-1, aCenter.Y()+n8 ) );
            break;

        case SymbolType::SPIN_UP:
            nRect.AdjustTop(n4 );
            pDev->DrawPixel( Point( aCenter.X(), nRect.Top() ) );
            for ( long i=1; i <= n2; ++i )
            {
                nRect.AdjustTop( 1 );
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Top() ),
                                Point( aCenter.X()+i, nRect.Top() ) );
                pDev->DrawPixel( Point( aCenter.X()-i, nRect.Top() ) );
                pDev->DrawPixel( Point( aCenter.X()+i, nRect.Top() ) );
            }
            break;

        case SymbolType::SPIN_DOWN:
            nRect.AdjustBottom( -n4 );
            pDev->DrawPixel( Point( aCenter.X(), nRect.Bottom() ) );
            for ( long i=1; i <= n2; ++i )
            {
                nRect.AdjustBottom( -1 );
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Bottom() ),
                                Point( aCenter.X()+i, nRect.Bottom() ) );
                pDev->DrawPixel( Point( aCenter.X()-i, nRect.Bottom() ) );
                pDev->DrawPixel( Point( aCenter.X()+i, nRect.Bottom() ) );
            }
            break;

        case SymbolType::SPIN_LEFT:
        case SymbolType::FIRST:
        case SymbolType::PREV:
        {
            nRect.AdjustLeft(n4 );
            if ( eType==SymbolType::FIRST )
            {
                pDev->DrawLine( Point( nRect.Left(), nRect.Top() ),
                                Point( nRect.Left(), nRect.Bottom() ) );
                nRect.AdjustLeft( 1 );
            }

            tools::Polygon aTriangle(3);
            aTriangle.SetPoint(Point(nRect.Left() + n2,  aCenter.Y() - n2), 0);
            aTriangle.SetPoint(Point(nRect.Left(), aCenter.Y()), 1);
            aTriangle.SetPoint(Point(nRect.Left() + n2, aCenter.Y() + n2), 2);

            pDev->Push(PushFlags::LINECOLOR);
            pDev->SetLineColor();
            pDev->DrawPolygon(aTriangle);
            pDev->Pop();

            break;
        }

        case SymbolType::SPIN_RIGHT:
        case SymbolType::LAST:
        case SymbolType::NEXT:
        case SymbolType::PLAY:
        {
            nRect.AdjustRight( -n4 );
            if ( eType==SymbolType::LAST )
            {
                pDev->DrawLine( Point( nRect.Right(), nRect.Top() ),
                                Point( nRect.Right(), nRect.Bottom() ) );
                nRect.AdjustRight( -1 );
            }

            tools::Polygon aTriangle(3);
            aTriangle.SetPoint(Point(nRect.Right() - n2,  aCenter.Y() - n2), 0);
            aTriangle.SetPoint(Point(nRect.Right(), aCenter.Y()), 1);
            aTriangle.SetPoint(Point(nRect.Right() - n2, aCenter.Y() + n2), 2);

            pDev->Push(PushFlags::LINECOLOR);
            pDev->SetLineColor();
            pDev->DrawPolygon(aTriangle);
            pDev->Pop();
            break;
        }

        case SymbolType::PAGEUP:
            pDev->DrawPixel( Point( aCenter.X(), nRect.Top() ) );
            pDev->DrawPixel( Point( aCenter.X(), nRect.Top()+n2 ) );
            for ( long i=1; i < n2; ++i )
            {
                nRect.AdjustTop( 1 );
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Top() ),
                                Point( aCenter.X()+i, nRect.Top() ) );
                pDev->DrawPixel( Point( aCenter.X()-i, nRect.Top() ) );
                pDev->DrawPixel( Point( aCenter.X()+i, nRect.Top() ) );
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Top()+n2 ),
                                Point( aCenter.X()+i, nRect.Top()+n2 ) );
                pDev->DrawPixel( Point( aCenter.X()-i, nRect.Top()+n2 ) );
                pDev->DrawPixel( Point( aCenter.X()+i, nRect.Top()+n2 ) );
            }
            break;

        case SymbolType::PAGEDOWN:
            pDev->DrawPixel( Point( aCenter.X(), nRect.Bottom() ) );
            pDev->DrawPixel( Point( aCenter.X(), nRect.Bottom()-n2 ) );
            for ( long i=1; i < n2; ++i )
            {
                nRect.AdjustBottom( -1 );
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Bottom() ),
                                Point( aCenter.X()+i, nRect.Bottom() ) );
                pDev->DrawPixel( Point( aCenter.X()-i, nRect.Bottom() ) );
                pDev->DrawPixel( Point( aCenter.X()+i, nRect.Bottom() ) );
                pDev->DrawLine( Point( aCenter.X()-i, nRect.Bottom()-n2 ),
                                Point( aCenter.X()+i, nRect.Bottom()-n2 ) );
                pDev->DrawPixel( Point( aCenter.X()-i, nRect.Bottom()-n2 ) );
                pDev->DrawPixel( Point( aCenter.X()+i, nRect.Bottom()-n2 ) );
            }
            break;

        case SymbolType::RADIOCHECKMARK:
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

        case SymbolType::STOP:
            pDev->DrawRect( nRect );
            break;

        case SymbolType::CLOSE:
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

        case SymbolType::ROLLDOWN:
            pDev->DrawLine( Point( nRect.Left(), nRect.Top() ),
                            Point( nRect.Left(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Right(), nRect.Top() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Bottom() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            [[fallthrough]];
        case SymbolType::ROLLUP:
            pDev->DrawRect( tools::Rectangle( nRect.Left(), nRect.Top(),
                                       nRect.Right(), nRect.Top()+n8 ) );
            break;

        case SymbolType::CHECKMARK:
            {
                long n3 = nSide/3;
                nRect.AdjustTop( -(n3/2) );
                nRect.AdjustBottom( -(n3/2) );
                // #106953# never mirror checkmarks
                if ( pDev->HasMirroredGraphics() && pDev->IsRTLEnabled() )
                {
                    // Draw a mirrored checkmark so that it looks "normal" in a
                    // mirrored graphics device (double mirroring!)
                    pDev->DrawLine( Point( nRect.Right(), nRect.Bottom()-n3 ),
                                    Point( nRect.Right()-n3, nRect.Bottom() ) );
                    pDev->DrawLine( Point( nRect.Right()-n3, nRect.Bottom() ),
                                    Point( nRect.Left(), nRect.Top()+n3 ) );
                    nRect.AdjustTop( 1 );
                    nRect.AdjustBottom( 1 );
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
                    nRect.AdjustTop( 1 );
                    nRect.AdjustBottom( 1 );
                    pDev->DrawLine( Point( nRect.Left(), nRect.Bottom()-n3 ),
                                    Point( nRect.Left()+n3, nRect.Bottom() ) );
                    pDev->DrawLine( Point( nRect.Left()+n3, nRect.Bottom() ),
                                    Point( nRect.Right(), nRect.Top()+n3 ) );
                }
            }
            break;

        case SymbolType::FLOAT:
            nRect.AdjustRight( -n4 );
            nRect.AdjustTop(n4+1 );
            pDev->DrawRect( tools::Rectangle( nRect.Left(), nRect.Top(),
                                       nRect.Right(), nRect.Top()+n8 ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Top()+n8 ),
                            Point( nRect.Left(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Bottom() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Right(), nRect.Top()+n8 ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            nRect.AdjustRight(n4 );
            nRect.AdjustTop( -(n4+1) );
            nRect.AdjustLeft(n4 );
            nRect.AdjustBottom( -(n4+1) );
            pDev->DrawRect( tools::Rectangle( nRect.Left(), nRect.Top(),
                                       nRect.Right(), nRect.Top()+n8 ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Top()+n8 ),
                            Point( nRect.Left(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Bottom() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Right(), nRect.Top()+n8 ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            break;

        case SymbolType::DOCK:
            pDev->DrawLine( Point( nRect.Left(), nRect.Top() ),
                            Point( nRect.Right(), nRect.Top() ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Top() ),
                            Point( nRect.Left(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Left(), nRect.Bottom() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            pDev->DrawLine( Point( nRect.Right(), nRect.Top() ),
                            Point( nRect.Right(), nRect.Bottom() ) );
            break;

        case SymbolType::HIDE:
            pDev->DrawRect( tools::Rectangle( nRect.Left()+n8, nRect.Bottom()-n8,
                                       nRect.Right()-n8, nRect.Bottom() ) );
            break;

        case SymbolType::PLUS:
            pDev->DrawRect( tools::Rectangle( nRect.Left(), aCenter.Y()-n16,
                                              nRect.Right(), aCenter.Y()+n16 ) );
            pDev->DrawRect( tools::Rectangle( aCenter.X()-n16, nRect.Top(),
                                              aCenter.X()+n16, nRect.Bottom() ) );
            break;
        case SymbolType::DONTKNOW:
        case SymbolType::IMAGE:
        case SymbolType::HELP: break;
    }
}

void ImplDrawDPILineRect( OutputDevice *const pDev, tools::Rectangle& rRect,
                          const Color *const pColor, const bool bRound = false )
{
    long nLineWidth = pDev->GetDPIX()/300;
    long nLineHeight = pDev->GetDPIY()/300;
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
            pDev->DrawRect( tools::Rectangle( rRect.TopLeft(), Size( nWidth, nLineHeight ) ) );
            pDev->DrawRect( tools::Rectangle( rRect.TopLeft(), Size( nLineWidth, nHeight ) ) );
            pDev->DrawRect( tools::Rectangle( Point( rRect.Left(), rRect.Bottom()-nLineHeight ),
                                       Size( nWidth, nLineHeight ) ) );
            pDev->DrawRect( tools::Rectangle( Point( rRect.Right()-nLineWidth, rRect.Top() ),
                                       Size( nLineWidth, nHeight ) ) );
        }
    }

    rRect.AdjustLeft(nLineWidth );
    rRect.AdjustTop(nLineHeight );
    rRect.AdjustRight( -nLineWidth );
    rRect.AdjustBottom( -nLineHeight );
}

void ImplDraw2ColorFrame( OutputDevice *const pDev, tools::Rectangle& rRect,
                          const Color& rLeftTopColor, const Color& rRightBottomColor )
{
    pDev->SetLineColor( rLeftTopColor );
    pDev->DrawLine( rRect.TopLeft(), rRect.BottomLeft() );
    pDev->DrawLine( rRect.TopLeft(), rRect.TopRight() );
    pDev->SetLineColor( rRightBottomColor );
    pDev->DrawLine( rRect.BottomLeft(), rRect.BottomRight() );
    pDev->DrawLine( rRect.TopRight(), rRect.BottomRight() );

    // reduce drawing area
    rRect.AdjustLeft( 1 );
    rRect.AdjustTop( 1 );
    rRect.AdjustRight( -1 );
    rRect.AdjustBottom( -1 );
}

void ImplDrawButton( OutputDevice *const pDev, tools::Rectangle aFillRect,
                     const DrawButtonFlags nStyle )
{
    const StyleSettings& rStyleSettings = pDev->GetSettings().GetStyleSettings();

    if ( (nStyle & DrawButtonFlags::Mono) ||
         (rStyleSettings.GetOptions() & StyleSettingsOptions::Mono) )
    {
        const Color aBlackColor(COL_BLACK);

        if ( nStyle & DrawButtonFlags::Default )
        {
            // default selection shows a wider border
            ImplDrawDPILineRect( pDev, aFillRect, &aBlackColor );
        }

        ImplDrawDPILineRect( pDev, aFillRect, &aBlackColor );

        Size aBrdSize( 1, 1 );
        if ( pDev->GetOutDevType() == OUTDEV_PRINTER )
        {
            aBrdSize = pDev->LogicToPixel( Size( 20, 20 ), MapMode(MapUnit::Map100thMM) );
            if ( !aBrdSize.Width() )
                aBrdSize.setWidth( 1 );
            if ( !aBrdSize.Height() )
                aBrdSize.setHeight( 1 );
        }

        pDev->SetLineColor();
        pDev->SetFillColor( aBlackColor );
        const tools::Rectangle aOrigFillRect(aFillRect);
        if ( nStyle & (DrawButtonFlags::Pressed | DrawButtonFlags::Checked) )
        {
            // shrink fill rect
            aFillRect.AdjustLeft(aBrdSize.Width() );
            aFillRect.AdjustTop(aBrdSize.Height() );
            // draw top and left borders (aOrigFillRect-aFillRect)
            pDev->DrawRect( tools::Rectangle( aOrigFillRect.Left(), aOrigFillRect.Top(),
                                       aOrigFillRect.Right(), aFillRect.Top()-1 ) );
            pDev->DrawRect( tools::Rectangle( aOrigFillRect.Left(), aOrigFillRect.Top(),
                                       aFillRect.Left()-1, aOrigFillRect.Bottom() ) );
        }
        else
        {
            // shrink fill rect
            aFillRect.AdjustRight( -(aBrdSize.Width()) );
            aFillRect.AdjustBottom( -(aBrdSize.Height()) );
            // draw bottom and right borders (aOrigFillRect-aFillRect)
            pDev->DrawRect( tools::Rectangle( aOrigFillRect.Left(), aFillRect.Bottom()+1,
                                       aOrigFillRect.Right(), aOrigFillRect.Bottom() ) );
            pDev->DrawRect( tools::Rectangle( aFillRect.Right()+1, aOrigFillRect.Top(),
                                       aOrigFillRect.Right(), aOrigFillRect.Bottom() ) );
        }

        // Hack: in monochrome mode on printers we like to have grey buttons
        if ( pDev->GetOutDevType() == OUTDEV_PRINTER )
            pDev->SetFillColor( COL_LIGHTGRAY );
        else
            pDev->SetFillColor( COL_WHITE );
        pDev->DrawRect( aFillRect );
    }
    else
    {
        if ( nStyle & DrawButtonFlags::Default )
        {
            const Color aDefBtnColor = rStyleSettings.GetDarkShadowColor();
            ImplDrawDPILineRect( pDev, aFillRect, &aDefBtnColor );
        }

        if ( nStyle & DrawButtonFlags::NoLeftLightBorder )
        {
            pDev->SetLineColor( rStyleSettings.GetLightBorderColor() );
            pDev->DrawLine( Point( aFillRect.Left(), aFillRect.Top() ),
                            Point( aFillRect.Left(), aFillRect.Bottom() ) );
            aFillRect.AdjustLeft( 1 );
        }

        Color aColor1;
        Color aColor2;
        if ( nStyle & (DrawButtonFlags::Pressed | DrawButtonFlags::Checked) )
        {
            aColor1 = rStyleSettings.GetDarkShadowColor();
            aColor2 = rStyleSettings.GetLightColor();
        }
        else
        {
            if ( nStyle & DrawButtonFlags::NoLightBorder )
                aColor1 = rStyleSettings.GetLightBorderColor();
            else
                aColor1 = rStyleSettings.GetLightColor();
            if ( (nStyle & BUTTON_DRAW_FLATTEST) == DrawButtonFlags::Flat )
                aColor2 = rStyleSettings.GetShadowColor();
            else
                aColor2 = rStyleSettings.GetDarkShadowColor();
        }

        ImplDraw2ColorFrame( pDev, aFillRect, aColor1, aColor2 );

        if ( (nStyle & BUTTON_DRAW_FLATTEST) != DrawButtonFlags::Flat )
        {
            if ( nStyle & (DrawButtonFlags::Pressed | DrawButtonFlags::Checked) )
            {
                aColor1 = rStyleSettings.GetShadowColor();
                aColor2 = rStyleSettings.GetLightBorderColor();
            }
            else
            {
                if ( nStyle & DrawButtonFlags::NoLightBorder )
                    aColor1 = rStyleSettings.GetLightColor();
                else
                    aColor1 = rStyleSettings.GetLightBorderColor();
                aColor2 = rStyleSettings.GetShadowColor();
            }
            ImplDraw2ColorFrame( pDev, aFillRect, aColor1, aColor2 );
        }

        pDev->SetLineColor();
        if ( nStyle & (DrawButtonFlags::Checked | DrawButtonFlags::DontKnow) )
            pDev->SetFillColor( rStyleSettings.GetCheckedColor() );
        else
            pDev->SetFillColor( rStyleSettings.GetFaceColor() );
        pDev->DrawRect( aFillRect );
    }
}

void ImplDrawFrame( OutputDevice *const pDev, tools::Rectangle& rRect,
                    const StyleSettings& rStyleSettings, DrawFrameStyle nStyle, DrawFrameFlags nFlags )
{
    vcl::Window *const pWin = (pDev->GetOutDevType()==OUTDEV_WINDOW) ? static_cast<vcl::Window*>(pDev) : nullptr;

    const bool bMenuStyle(nFlags & DrawFrameFlags::Menu);

    // UseFlatBorders disables 3D style for all frames except menus
    // menus may use different border colors (eg on XP)
    // normal frames will be drawn using the shadow color
    // whereas window frame borders will use black
    bool bFlatBorders = !bMenuStyle && rStyleSettings.GetUseFlatBorders();

    // no flat borders for standard VCL controls (ie formcontrols that keep their classic look)
    // will not affect frame windows (like dropdowns)
    if( bFlatBorders && pWin && pWin->GetType() == WindowType::BORDERWINDOW && (pWin != pWin->ImplGetFrameWindow()) )
    {
        // check for formcontrol, i.e., a control without NWF enabled
        Control *const pControl = dynamic_cast< Control* >( pWin->GetWindow( GetWindowType::Client ) );
        if( !pControl || !pControl->IsNativeWidgetEnabled() )
            bFlatBorders = false;
    }

    const bool bNoDraw(nFlags & DrawFrameFlags::NoDraw);

    if ( (rStyleSettings.GetOptions() & StyleSettingsOptions::Mono) ||
         (pDev->GetOutDevType() == OUTDEV_PRINTER) ||
         bFlatBorders )
        nFlags |= DrawFrameFlags::Mono;

    if( nStyle != DrawFrameStyle::NWF &&
        pWin && pWin->IsNativeControlSupported(ControlType::Frame, ControlPart::Border) )
    {
        long nControlFlags = static_cast<long>(nStyle);
        nControlFlags |= static_cast<long>(nFlags);
        nControlFlags |= static_cast<long>(pWin->GetType() == WindowType::BORDERWINDOW ?
                                           DrawFrameFlags::BorderWindowBorder : DrawFrameFlags::NONE);
        ImplControlValue aControlValue( nControlFlags );

        tools::Rectangle aBound, aContent;
        tools::Rectangle aNatRgn( rRect );
        if( pWin->GetNativeControlRegion(ControlType::Frame, ControlPart::Border,
            aNatRgn, ControlState::NONE, aControlValue, aBound, aContent) )
        {
            // if bNoDraw is true then don't call the drawing routine
            // but just update the target rectangle
            if( bNoDraw ||
                pWin->DrawNativeControl( ControlType::Frame, ControlPart::Border, aContent, ControlState::ENABLED,
                                         aControlValue, OUString()) )
            {
                rRect = aContent;
                return;
            }
        }
    }

    if ( nFlags & DrawFrameFlags::Mono )
    {
        // no round corners for window frame borders
        const bool bRound = bFlatBorders && !(nFlags & DrawFrameFlags::WindowBorder);

        if ( bNoDraw )
        {
            ImplDrawDPILineRect( pDev, rRect, nullptr, bRound );
        }
        else
        {
            Color aColor = bRound ? rStyleSettings.GetShadowColor()
                                  : pDev->GetSettings().GetStyleSettings().GetMonoColor();
            // when the MonoColor wasn't set, check face color
            if (
                (bRound && aColor.IsDark()) ||
                (
                  (aColor == COL_BLACK) &&
                  pDev->GetSettings().GetStyleSettings().GetFaceColor().IsDark()
                )
               )
            {
                aColor = COL_WHITE;
            }
            ImplDrawDPILineRect( pDev, rRect, &aColor, bRound );
        }
    }
    else
    {
        if ( bNoDraw )
        {
            switch ( nStyle )
            {
                case DrawFrameStyle::In:
                case DrawFrameStyle::Out:
                    rRect.AdjustLeft( 1 );
                    rRect.AdjustTop( 1 );
                    rRect.AdjustRight( -1 );
                    rRect.AdjustBottom( -1 );
                    break;

                case DrawFrameStyle::Group:
                case DrawFrameStyle::DoubleIn:
                case DrawFrameStyle::DoubleOut:
                    rRect.AdjustLeft(2 );
                    rRect.AdjustTop(2 );
                    rRect.AdjustRight( -2 );
                    rRect.AdjustBottom( -2 );
                    break;

                case DrawFrameStyle::NWF:
                    // enough space for the native rendering
                    rRect.AdjustLeft(4 );
                    rRect.AdjustTop(4 );
                    rRect.AdjustRight( -4 );
                    rRect.AdjustBottom( -4 );
                    break;
                default: break;
            }
        }
        else
        {
            switch ( nStyle )
            {
                case DrawFrameStyle::Group:
                    pDev->SetFillColor();
                    pDev->SetLineColor( rStyleSettings.GetLightColor() );
                    pDev->DrawRect( tools::Rectangle( rRect.Left()+1, rRect.Top()+1,
                                            rRect.Right(), rRect.Bottom() ) );
                    pDev->SetLineColor( rStyleSettings.GetShadowColor() );
                    pDev->DrawRect( tools::Rectangle( rRect.Left(), rRect.Top(),
                                            rRect.Right()-1, rRect.Bottom()-1 ) );

                    // adjust target rectangle
                    rRect.AdjustLeft(2 );
                    rRect.AdjustTop(2 );
                    rRect.AdjustRight( -2 );
                    rRect.AdjustBottom( -2 );
                    break;

                case DrawFrameStyle::In:
                    ImplDraw2ColorFrame( pDev, rRect,
                                         rStyleSettings.GetShadowColor(),
                                         rStyleSettings.GetLightColor() );
                    break;

                case DrawFrameStyle::Out:
                    ImplDraw2ColorFrame( pDev, rRect,
                                         rStyleSettings.GetLightColor(),
                                         rStyleSettings.GetShadowColor() );
                    break;

                case DrawFrameStyle::DoubleIn:
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

                case DrawFrameStyle::DoubleOut:
                    if( bMenuStyle )
                    {
                        ImplDraw2ColorFrame( pDev, rRect,
                                             rStyleSettings.GetMenuBorderColor(),
                                             rStyleSettings.GetDarkShadowColor() );
                        if ( !rStyleSettings.GetUseFlatMenus() )
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

                case DrawFrameStyle::NWF:
                    // no rendering, just enough space for the native rendering
                    rRect.AdjustLeft(4 );
                    rRect.AdjustTop(4 );
                    rRect.AdjustRight( -4 );
                    rRect.AdjustBottom( -4 );
                    break;
                default: break;
            }
        }
    }
}

} // end anonymous namespace

DecorationView::DecorationView(OutputDevice* pOutDev) :
    mpOutDev(pOutDev)
{}

void DecorationView::DrawSymbol( const tools::Rectangle& rRect, SymbolType eType,
                                 const Color& rColor, DrawSymbolFlags nStyle )
{
    const StyleSettings&    rStyleSettings  = mpOutDev->GetSettings().GetStyleSettings();
    const tools::Rectangle         aRect           = mpOutDev->LogicToPixel( rRect );
    const Color             aOldLineColor   = mpOutDev->GetLineColor();
    const Color             aOldFillColor   = mpOutDev->GetFillColor();
    const bool              bOldMapMode     = mpOutDev->IsMapModeEnabled();
    Color                   nColor(rColor);
    mpOutDev->EnableMapMode( false );

    if ( (rStyleSettings.GetOptions() & StyleSettingsOptions::Mono) ||
         (mpOutDev->GetOutDevType() == OUTDEV_PRINTER) )
        nStyle |= DrawSymbolFlags::Mono;

    if ( nStyle & DrawSymbolFlags::Mono )
    {
        // Monochrome: set color to black if enabled, to gray if disabled
        nColor = ( nStyle & DrawSymbolFlags::Disable ) ? COL_GRAY : COL_BLACK;
    }
    else
    {
        if ( nStyle & DrawSymbolFlags::Disable )
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

void DecorationView::DrawFrame( const tools::Rectangle& rRect,
                                const Color& rLeftTopColor,
                                const Color& rRightBottomColor )
{
    tools::Rectangle   aRect         = mpOutDev->LogicToPixel( rRect );
    const Color aOldLineColor = mpOutDev->GetLineColor();
    const bool  bOldMapMode   = mpOutDev->IsMapModeEnabled();
    mpOutDev->EnableMapMode( false );
    ImplDraw2ColorFrame( mpOutDev, aRect, rLeftTopColor, rRightBottomColor );
    mpOutDev->SetLineColor( aOldLineColor );
    mpOutDev->EnableMapMode( bOldMapMode );
}

void DecorationView::DrawHighlightFrame( const tools::Rectangle& rRect,
                                         DrawHighlightFrameStyle nStyle )
{
    const StyleSettings& rStyleSettings = mpOutDev->GetSettings().GetStyleSettings();
    Color aLightColor = rStyleSettings.GetLightColor();
    Color aShadowColor = rStyleSettings.GetShadowColor();

    if ( (rStyleSettings.GetOptions() & StyleSettingsOptions::Mono) ||
         (mpOutDev->GetOutDevType() == OUTDEV_PRINTER) )
    {
        aLightColor = COL_BLACK;
        aShadowColor = COL_BLACK;
    }
    else
    {
        Wallpaper aBackground = mpOutDev->GetBackground();
        if ( aBackground.IsBitmap() || aBackground.IsGradient() )
        {
            aLightColor = rStyleSettings.GetFaceColor();
            aShadowColor = COL_BLACK;
        }
        else
        {
            Color aBackColor = aBackground.GetColor();
            if ( (aLightColor.GetColorError( aBackColor ) < 32) ||
                 (aShadowColor.GetColorError( aBackColor ) < 32) )
            {
                aLightColor = COL_WHITE;
                aShadowColor = COL_BLACK;

                if ( aLightColor.GetColorError( aBackColor ) < 32 )
                    aLightColor.DecreaseLuminance( 64 );
                if ( aShadowColor.GetColorError( aBackColor ) < 32 )
                    aShadowColor.IncreaseLuminance( 64 );
            }
        }
    }

    if ( nStyle == DrawHighlightFrameStyle::In )
    {
        Color aTempColor = aLightColor;
        aLightColor = aShadowColor;
        aShadowColor = aTempColor;
    }

    DrawFrame( rRect, aLightColor, aShadowColor );
}

tools::Rectangle DecorationView::DrawFrame( const tools::Rectangle& rRect, DrawFrameStyle nStyle, DrawFrameFlags nFlags )
{
    tools::Rectangle   aRect = rRect;
    bool        bOldMap = mpOutDev->IsMapModeEnabled();
    if ( bOldMap )
    {
        aRect = mpOutDev->LogicToPixel( aRect );
        mpOutDev->EnableMapMode( false );
    }

    if ( !rRect.IsEmpty() )
    {
        if ( nFlags & DrawFrameFlags::NoDraw )
             ImplDrawFrame( mpOutDev, aRect, mpOutDev->GetSettings().GetStyleSettings(), nStyle, nFlags );
        else
        {
             Color aOldLineColor  = mpOutDev->GetLineColor();
             Color aOldFillColor  = mpOutDev->GetFillColor();
             ImplDrawFrame( mpOutDev, aRect, mpOutDev->GetSettings().GetStyleSettings(), nStyle, nFlags );
             mpOutDev->SetLineColor( aOldLineColor );
             mpOutDev->SetFillColor( aOldFillColor );
        }
    }

    if ( bOldMap )
    {
        mpOutDev->EnableMapMode( bOldMap );
        aRect = mpOutDev->PixelToLogic( aRect );
    }

    return aRect;
}

tools::Rectangle DecorationView::DrawButton( const tools::Rectangle& rRect, DrawButtonFlags nStyle )
{
    if ( rRect.IsEmpty() )
    {
        return rRect;
    }

    tools::Rectangle aRect = rRect;
    const bool bOldMap = mpOutDev->IsMapModeEnabled();

    if ( bOldMap )
    {
        aRect = mpOutDev->LogicToPixel( aRect );
        mpOutDev->EnableMapMode( false );
    }

    const Color aOldLineColor = mpOutDev->GetLineColor();
    const Color aOldFillColor = mpOutDev->GetFillColor();
    ImplDrawButton( mpOutDev, aRect, nStyle );
    mpOutDev->SetLineColor( aOldLineColor );
    mpOutDev->SetFillColor( aOldFillColor );

    // keep border free, although it is used at default representation
    aRect.AdjustLeft( 1 );
    aRect.AdjustTop( 1 );
    aRect.AdjustRight( -1 );
    aRect.AdjustBottom( -1 );

    if ( nStyle & DrawButtonFlags::NoLightBorder )
    {
        aRect.AdjustLeft( 1 );
        aRect.AdjustTop( 1 );
    }
    else if ( nStyle & DrawButtonFlags::NoLeftLightBorder )
    {
        aRect.AdjustLeft( 1 );
    }

    if ( nStyle & DrawButtonFlags::Pressed )
    {
        if ( (aRect.GetHeight() > 10) && (aRect.GetWidth() > 10) )
        {
            aRect.AdjustLeft(4 );
            aRect.AdjustTop(4 );
            aRect.AdjustRight( -1 );
            aRect.AdjustBottom( -1 );
        }
        else
        {
            aRect.AdjustLeft(3 );
            aRect.AdjustTop(3 );
            aRect.AdjustRight( -2 );
            aRect.AdjustBottom( -2 );
        }
    }
    else if ( nStyle & DrawButtonFlags::Checked )
    {
        aRect.AdjustLeft(3 );
        aRect.AdjustTop(3 );
        aRect.AdjustRight( -2 );
        aRect.AdjustBottom( -2 );
    }
    else
    {
        aRect.AdjustLeft(2 );
        aRect.AdjustTop(2 );
        aRect.AdjustRight( -3 );
        aRect.AdjustBottom( -3 );
    }

    if ( bOldMap )
    {
        mpOutDev->EnableMapMode( bOldMap );
        aRect = mpOutDev->PixelToLogic( aRect );
    }

    return aRect;
}

void DecorationView::DrawSeparator( const Point& rStart, const Point& rStop, bool bVertical )
{
    Point aStart( rStart ), aStop( rStop );
    const StyleSettings& rStyleSettings = mpOutDev->GetSettings().GetStyleSettings();
    vcl::Window *const pWin = (mpOutDev->GetOutDevType()==OUTDEV_WINDOW) ? static_cast<vcl::Window*>(mpOutDev.get()) : nullptr;
    if(pWin)
    {
        ControlPart nPart = ( bVertical ? ControlPart::SeparatorVert : ControlPart::SeparatorHorz );
        bool nativeSupported = pWin->IsNativeControlSupported( ControlType::Fixedline, nPart );
        ImplControlValue    aValue;
        tools::Rectangle aRect(rStart,rStop);
        if(nativeSupported && pWin->DrawNativeControl(ControlType::Fixedline,nPart,aRect,ControlState::NONE,aValue,OUString()))
            return;
    }

    mpOutDev->Push( PushFlags::LINECOLOR );
    if ( rStyleSettings.GetOptions() & StyleSettingsOptions::Mono )
        mpOutDev->SetLineColor( COL_BLACK );
    else
        mpOutDev->SetLineColor( rStyleSettings.GetShadowColor() );

    mpOutDev->DrawLine( aStart, aStop );
    if ( !(rStyleSettings.GetOptions() & StyleSettingsOptions::Mono) )
    {
        mpOutDev->SetLineColor( rStyleSettings.GetLightColor() );
        if( bVertical )
        {
            aStart.AdjustX( 1 );
            aStop.AdjustX( 1 );
        }
        else
        {
            aStart.AdjustY( 1 );
            aStop.AdjustY( 1 );
        }
        mpOutDev->DrawLine( aStart, aStop );
    }
    mpOutDev->Pop();
}

void DecorationView::DrawHandle(const tools::Rectangle& rRect)
{
    const StyleSettings& rStyleSettings = mpOutDev->GetSettings().GetStyleSettings();

    Size aOutputSize = rRect.GetSize();

    mpOutDev->SetLineColor(rStyleSettings.GetDarkShadowColor());
    mpOutDev->SetFillColor(rStyleSettings.GetDarkShadowColor());

    const sal_Int32 nNumberOfPoints = 3;

    long nHalfWidth = aOutputSize.Width() / 2.0f;

    float fDistance = aOutputSize.Height();
    fDistance /= (nNumberOfPoints + 1);

    long nRadius = aOutputSize.Width();
    nRadius /= (nNumberOfPoints + 2);

    for (long i = 1; i <= nNumberOfPoints; i++)
    {
        tools::Rectangle aLocation;
        aLocation = tools::Rectangle(nHalfWidth - nRadius,
                              round(fDistance * i) - nRadius,
                              nHalfWidth + nRadius,
                              round(fDistance * i) + nRadius);
        mpOutDev->DrawEllipse(aLocation);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
