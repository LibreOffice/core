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

namespace {

tools::Long AdjustRectToSquare( tools::Rectangle &rRect )
{
    const tools::Long nWidth = rRect.GetWidth();
    const tools::Long nHeight = rRect.GetHeight();
    tools::Long nSide = std::min( nWidth, nHeight );

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
    const tools::Long nSide = AdjustRectToSquare( nRect );

    if ( !nSide ) return;
    if ( nSide==1 )
    {
        pDev->DrawPixel( Point( nRect.Left(), nRect.Top() ) );
        return;
    }

    // Precalculate some values
    const tools::Long n2 = nSide/2;
    const tools::Long n4 = (n2+1)/2;
    const tools::Long n8 = (n4+1)/2;
    const tools::Long n16 = (n8+1)/2;
    const Point aCenter = nRect.Center();

    switch ( eType )
    {
        case SymbolType::ARROW_UP:
        {
            tools::Polygon arrow(7);
            arrow.SetPoint( Point( aCenter.X(), nRect.Top()), 0 );
            arrow.SetPoint( Point( aCenter.X() - n2, nRect.Top() + n2 ), 1 );
            arrow.SetPoint( Point( aCenter.X() - n8, nRect.Top() + n2 ), 2 );
            arrow.SetPoint( Point( aCenter.X() - n8, nRect.Bottom()), 3 );
            arrow.SetPoint( Point( aCenter.X() + n8, nRect.Bottom()), 4 );
            arrow.SetPoint( Point( aCenter.X() + n8, nRect.Top() + n2 ), 5 );
            arrow.SetPoint( Point( aCenter.X() + n2, nRect.Top() + n2 ), 6 );
            pDev->Push(vcl::PushFlags::LINECOLOR);
            pDev->SetLineColor();
            pDev->DrawPolygon( arrow );
            pDev->Pop();
            break;
        }

        case SymbolType::ARROW_DOWN:
        {
            tools::Polygon arrow(7);
            arrow.SetPoint( Point( aCenter.X(), nRect.Bottom()), 0 );
            arrow.SetPoint( Point( aCenter.X() - n2, nRect.Bottom() - n2 ), 1 );
            arrow.SetPoint( Point( aCenter.X() - n8, nRect.Bottom() - n2 ), 2 );
            arrow.SetPoint( Point( aCenter.X() - n8, nRect.Top()), 3 );
            arrow.SetPoint( Point( aCenter.X() + n8, nRect.Top()), 4 );
            arrow.SetPoint( Point( aCenter.X() + n8, nRect.Bottom() - n2 ), 5 );
            arrow.SetPoint( Point( aCenter.X() + n2, nRect.Bottom() - n2 ), 6 );
            pDev->Push(vcl::PushFlags::LINECOLOR);
            pDev->SetLineColor();
            pDev->DrawPolygon( arrow );
            pDev->Pop();
            break;
        }

        case SymbolType::ARROW_LEFT:
        {
            tools::Polygon arrow(7);
            arrow.SetPoint( Point( nRect.Left(), aCenter.Y()), 0 );
            arrow.SetPoint( Point( nRect.Left() + n2, aCenter.Y() - n2 ), 1 );
            arrow.SetPoint( Point( nRect.Left() + n2, aCenter.Y() - n8 ), 2 );
            arrow.SetPoint( Point( nRect.Right(), aCenter.Y() - n8 ), 3 );
            arrow.SetPoint( Point( nRect.Right(), aCenter.Y() + n8 ), 4 );
            arrow.SetPoint( Point( nRect.Left() + n2, aCenter.Y() + n8 ), 5 );
            arrow.SetPoint( Point( nRect.Left() + n2, aCenter.Y() + n2 ), 6 );
            pDev->Push(vcl::PushFlags::LINECOLOR);
            pDev->SetLineColor();
            pDev->DrawPolygon( arrow );
            pDev->Pop();
            break;
        }

        case SymbolType::ARROW_RIGHT:
        {
            tools::Polygon arrow(7);
            arrow.SetPoint( Point( nRect.Right(), aCenter.Y()), 0 );
            arrow.SetPoint( Point( nRect.Right() - n2, aCenter.Y() - n2 ), 1 );
            arrow.SetPoint( Point( nRect.Right() - n2, aCenter.Y() - n8 ), 2 );
            arrow.SetPoint( Point( nRect.Left(), aCenter.Y() - n8 ), 3 );
            arrow.SetPoint( Point( nRect.Left(), aCenter.Y() + n8 ), 4 );
            arrow.SetPoint( Point( nRect.Right() - n2, aCenter.Y() + n8 ), 5 );
            arrow.SetPoint( Point( nRect.Right() - n2, aCenter.Y() + n2 ), 6 );
            pDev->Push(vcl::PushFlags::LINECOLOR);
            pDev->SetLineColor();
            pDev->DrawPolygon( arrow );
            pDev->Pop();
            break;
        }

        case SymbolType::SPIN_UP:
        {
            tools::Polygon triangle( 3 );
            triangle.SetPoint( Point( aCenter.X(), nRect.Top() + n4 ), 0 );
            triangle.SetPoint( Point( aCenter.X() - n2, nRect.Top() + n4 + n2 ), 1 );
            triangle.SetPoint( Point( aCenter.X() + n2, nRect.Top() + n4 + n2 ), 2 );
            pDev->Push(vcl::PushFlags::LINECOLOR);
            pDev->SetLineColor();
            pDev->DrawPolygon( triangle );
            pDev->Pop();
            break;
        }

        case SymbolType::SPIN_DOWN:
        {
            tools::Polygon triangle( 3 );
            triangle.SetPoint( Point( aCenter.X(), nRect.Bottom() - n4 ), 0 );
            triangle.SetPoint( Point( aCenter.X() - n2, nRect.Bottom() - n4 - n2 ), 1 );
            triangle.SetPoint( Point( aCenter.X() + n2, nRect.Bottom() - n4 - n2 ), 2 );
            pDev->Push(vcl::PushFlags::LINECOLOR);
            pDev->SetLineColor();
            pDev->DrawPolygon( triangle );
            pDev->Pop();
            break;
        }

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

            pDev->Push(vcl::PushFlags::LINECOLOR);
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

            pDev->Push(vcl::PushFlags::LINECOLOR);
            pDev->SetLineColor();
            pDev->DrawPolygon(aTriangle);
            pDev->Pop();
            break;
        }

        case SymbolType::PAGEUP:
        {
            tools::Polygon triangle( 3 );
            triangle.SetPoint( Point( aCenter.X(), nRect.Top()), 0 );
            triangle.SetPoint( Point( aCenter.X() - n2, nRect.Top() + n2 ), 1 );
            triangle.SetPoint( Point( aCenter.X() + n2, nRect.Top() + n2 ), 2 );
            pDev->Push(vcl::PushFlags::LINECOLOR);
            pDev->SetLineColor();
            pDev->DrawPolygon( triangle );
            triangle.Move( 0, n2 );
            pDev->DrawPolygon( triangle );
            pDev->Pop();
            break;
        }

        case SymbolType::PAGEDOWN:
        {
            tools::Polygon triangle( 3 );
            triangle.SetPoint( Point( aCenter.X(), nRect.Bottom()), 0 );
            triangle.SetPoint( Point( aCenter.X() - n2, nRect.Bottom() - n2 ), 1 );
            triangle.SetPoint( Point( aCenter.X() + n2, nRect.Bottom() - n2 ), 2 );
            pDev->Push(vcl::PushFlags::LINECOLOR);
            pDev->SetLineColor();
            pDev->DrawPolygon( triangle );
            triangle.Move( 0, -n2 );
            pDev->DrawPolygon( triangle );
            pDev->Pop();
            break;
        }

        case SymbolType::RADIOCHECKMARK:
        {
            pDev->DrawEllipse(nRect);
            break;
        }

        case SymbolType::STOP:
            pDev->DrawRect( nRect );
            break;

        case SymbolType::CLOSE:
        {
            const tools::Long diff = std::max<tools::Long>( 0, n8 - 1 );
            tools::Polygon cross( 16 );
            cross.SetPoint( Point( nRect.Left(), nRect.Top()), 0 );
            cross.SetPoint( Point( nRect.Left(), nRect.Top() + diff ), 1 );
            cross.SetPoint( Point( aCenter.X() - diff, aCenter.Y()), 2 );
            cross.SetPoint( Point( nRect.Left(), nRect.Bottom() - diff ), 3 );
            cross.SetPoint( Point( nRect.Left(), nRect.Bottom()), 4 );
            cross.SetPoint( Point( nRect.Left() + diff, nRect.Bottom()), 5 );
            cross.SetPoint( Point( aCenter.X(), aCenter.Y() + diff ), 6 );
            cross.SetPoint( Point( nRect.Right() - diff, nRect.Bottom()), 7 );
            cross.SetPoint( Point( nRect.Right(), nRect.Bottom()), 8 );
            cross.SetPoint( Point( nRect.Right(), nRect.Bottom() - diff ), 9 );
            cross.SetPoint( Point( aCenter.X() + diff, aCenter.Y()), 10 );
            cross.SetPoint( Point( nRect.Right(), nRect.Top() + diff ), 11 );
            cross.SetPoint( Point( nRect.Right(), nRect.Top()), 12 );
            cross.SetPoint( Point( nRect.Right() - diff, nRect.Top()), 13 );
            cross.SetPoint( Point( aCenter.X(), aCenter.Y() - diff ), 14 );
            cross.SetPoint( Point( nRect.Left() + diff, nRect.Top()), 15 );
            pDev->DrawPolygon( cross );
            break;
        }

        case SymbolType::CHECKMARK:
            {
                tools::Long n3 = nSide/3;
                nRect.AdjustTop( -(n3/2) );
                nRect.AdjustBottom( -(n3/2) );
                tools::Polygon checkmark(6);
                // #106953# never mirror checkmarks
                if ( pDev->HasMirroredGraphics() && pDev->IsRTLEnabled() )
                {
                    // Draw a mirrored checkmark so that it looks "normal" in a
                    // mirrored graphics device (double mirroring!)
                    checkmark.SetPoint( Point( nRect.Right(), nRect.Bottom()-n3 ), 0 );
                    checkmark.SetPoint( Point( nRect.Right()-n3, nRect.Bottom()), 1 );
                    checkmark.SetPoint( Point( nRect.Left(), nRect.Top()+n3 ), 2 );
                    checkmark.SetPoint( Point( nRect.Left(), nRect.Top()+n3 + 1 ), 3 );
                    checkmark.SetPoint( Point( nRect.Right()-n3, nRect.Bottom() + 1 ), 4 );
                    checkmark.SetPoint( Point( nRect.Right(), nRect.Bottom()-n3 + 1 ), 5 );
                }
                else
                {
                    checkmark.SetPoint( Point( nRect.Left(), nRect.Bottom()-n3 ), 0 );
                    checkmark.SetPoint( Point( nRect.Left()+n3, nRect.Bottom()), 1 );
                    checkmark.SetPoint( Point( nRect.Right(), nRect.Top()+n3 ), 2 );
                    checkmark.SetPoint( Point( nRect.Right(), nRect.Top()+n3 + 1 ), 3 );
                    checkmark.SetPoint( Point( nRect.Left()+n3, nRect.Bottom() + 1 ), 4 );
                    checkmark.SetPoint( Point( nRect.Left(), nRect.Bottom()-n3 + 1 ), 5 );
                }
                pDev->DrawPolygon( checkmark );
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
    tools::Long nLineWidth = pDev->GetDPIX()/300;
    tools::Long nLineHeight = pDev->GetDPIY()/300;
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
            const tools::Long nWidth = rRect.GetWidth();
            const tools::Long nHeight = rRect.GetHeight();
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

        Size aBrdSize(pDev->GetButtonBorderSize());

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
        pDev->SetFillColor(pDev->GetMonochromeButtonColor());
        pDev->DrawRect( aFillRect );
    }
    else
    {
        const bool bFlat(nStyle & DrawButtonFlags::Flat);
        const bool bDepressed(nStyle & (DrawButtonFlags::Pressed | DrawButtonFlags::Checked));

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

        bool bNoFace = false;
        Color aColor1;
        Color aColor2;
        if (!bFlat)
        {
            if (bDepressed)
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
                aColor2 = rStyleSettings.GetDarkShadowColor();
            }

            ImplDraw2ColorFrame( pDev, aFillRect, aColor1, aColor2 );

            if (bDepressed)
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
        else // flat buttons
        {
            // draw a border if the flat button is highlighted
            if (nStyle & DrawButtonFlags::Highlight)
            {
                aColor1 = rStyleSettings.GetShadowColor();
                ImplDraw2ColorFrame(pDev, aFillRect, aColor1, aColor1);
            }
            // fill in the button if it is pressed in
            bNoFace = !bDepressed;
        }

        pDev->SetLineColor();
        if ( nStyle & (DrawButtonFlags::Checked | DrawButtonFlags::DontKnow) )
            pDev->SetFillColor( rStyleSettings.GetCheckedColor() );
        else if (!bNoFace)
            pDev->SetFillColor( rStyleSettings.GetFaceColor() );
        pDev->DrawRect( aFillRect );
    }
}

void ImplDrawFrame( OutputDevice *const pDev, tools::Rectangle& rRect,
                    const StyleSettings& rStyleSettings, DrawFrameStyle nStyle, DrawFrameFlags nFlags )
{
    vcl::Window * pWin = pDev->GetOwnerWindow();

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
        tools::Long nControlFlags = static_cast<tools::Long>(nStyle);
        nControlFlags |= static_cast<tools::Long>(nFlags);
        nControlFlags |= static_cast<tools::Long>(pWin->GetType() == WindowType::BORDERWINDOW ?
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
                pWin->GetOutDev()->DrawNativeControl( ControlType::Frame, ControlPart::Border, aBound, ControlState::ENABLED,
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
    mpOutDev->Push(vcl::PushFlags::FILLCOLOR | vcl::PushFlags::LINECOLOR | vcl::PushFlags::MAPMODE);
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
    mpOutDev->Pop();
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

void DecorationView::DrawHighlightFrame( const tools::Rectangle& rRect )
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
            if ( (aLightColor.GetColorError( aBackColor ) < 96) ||
                 (aShadowColor.GetColorError( aBackColor ) < 96) )
            {
                aLightColor = COL_WHITE;
                aShadowColor = COL_BLACK;

                if ( aLightColor.GetColorError( aBackColor ) < 96 )
                    aLightColor.DecreaseLuminance( 64 );
                if ( aShadowColor.GetColorError( aBackColor ) < 96 )
                    aShadowColor.IncreaseLuminance( 64 );
            }
        }
    }

    DrawFrame( rRect, aLightColor, aShadowColor );
}

tools::Rectangle DecorationView::DrawFrame( const tools::Rectangle& rRect, DrawFrameStyle nStyle, DrawFrameFlags nFlags )
{
    tools::Rectangle aRect = mpOutDev->LogicToPixel( rRect );
    bool bOldMap = mpOutDev->IsMapModeEnabled();
    mpOutDev->EnableMapMode( false );

    if ( !rRect.IsEmpty() )
    {
        if ( nFlags & DrawFrameFlags::NoDraw )
             ImplDrawFrame( mpOutDev, aRect, mpOutDev->GetSettings().GetStyleSettings(), nStyle, nFlags );
        else
        {
             mpOutDev->Push(vcl::PushFlags::FILLCOLOR | vcl::PushFlags::LINECOLOR);
             ImplDrawFrame( mpOutDev, aRect, mpOutDev->GetSettings().GetStyleSettings(), nStyle, nFlags );
             mpOutDev->Pop();
        }
    }

    mpOutDev->EnableMapMode( bOldMap );
    aRect = mpOutDev->PixelToLogic( aRect );

    return aRect;
}

tools::Rectangle DecorationView::DrawButton( const tools::Rectangle& rRect, DrawButtonFlags nStyle )
{
    if ( rRect.IsEmpty() )
    {
        return rRect;
    }

    tools::Rectangle aRect = mpOutDev->LogicToPixel( rRect );
    const bool bOldMap = mpOutDev->IsMapModeEnabled();
    mpOutDev->EnableMapMode( false );

    mpOutDev->Push(vcl::PushFlags::FILLCOLOR | vcl::PushFlags::LINECOLOR);
    ImplDrawButton( mpOutDev, aRect, nStyle );
    mpOutDev->Pop();

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

    mpOutDev->EnableMapMode( bOldMap );
    aRect = mpOutDev->PixelToLogic( aRect );

    return aRect;
}

void DecorationView::DrawSeparator( const Point& rStart, const Point& rStop, bool bVertical )
{
    Point aStart( rStart ), aStop( rStop );
    const StyleSettings& rStyleSettings = mpOutDev->GetSettings().GetStyleSettings();
    vcl::Window *const pWin = mpOutDev->GetOwnerWindow();
    if(pWin)
    {
        ControlPart nPart = ( bVertical ? ControlPart::SeparatorVert : ControlPart::SeparatorHorz );
        bool nativeSupported = pWin->IsNativeControlSupported( ControlType::Fixedline, nPart );
        ImplControlValue    aValue;
        tools::Rectangle aRect(rStart,rStop);
        if(nativeSupported && pWin->GetOutDev()->DrawNativeControl(ControlType::Fixedline,nPart,aRect,ControlState::NONE,aValue,OUString()))
            return;
    }

    mpOutDev->Push( vcl::PushFlags::LINECOLOR );
    if ( rStyleSettings.GetOptions() & StyleSettingsOptions::Mono )
        mpOutDev->SetLineColor( COL_BLACK );
    else
        mpOutDev->SetLineColor( rStyleSettings.GetSeparatorColor() );

    mpOutDev->DrawLine( aStart, aStop );

    mpOutDev->Pop();
}

void DecorationView::DrawHandle(const tools::Rectangle& rRect)
{
    const StyleSettings& rStyleSettings = mpOutDev->GetSettings().GetStyleSettings();

    Size aOutputSize = rRect.GetSize();

    mpOutDev->SetLineColor(rStyleSettings.GetDarkShadowColor());
    mpOutDev->SetFillColor(rStyleSettings.GetDarkShadowColor());

    const sal_Int32 nNumberOfPoints = 3;

    tools::Long nHalfWidth = aOutputSize.Width() / 2.0f;

    float fDistance = aOutputSize.Height();
    fDistance /= (nNumberOfPoints + 1);

    tools::Long nRadius = aOutputSize.Width();
    nRadius /= (nNumberOfPoints + 2);

    for (tools::Long i = 1; i <= nNumberOfPoints; i++)
    {
        tools::Rectangle aLocation(nHalfWidth - nRadius,
                              round(fDistance * i) - nRadius,
                              nHalfWidth + nRadius,
                              round(fDistance * i) + nRadius);
        mpOutDev->DrawEllipse(aLocation);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
