/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 SUSE <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <edtwin.hxx>
#include <HeaderFooterWin.hxx>
#include <viewopt.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/color/bcolortools.hxx>

#define TEXT_PADDING 7
#define BOX_DISTANCE 10

// the WB_MOVABLE flag is used here to avoid the window to appear on all desktops (on linux)
// and the WB_OWNERDRAWDECORATION prevents the system to draw the window decorations.
//
SwHeaderFooterWin::SwHeaderFooterWin( SwEditWin* pEditWin, const rtl::OUString& sLabel, bool bHeader, Point aOffset ) :
    FloatingWindow( pEditWin, WB_SYSTEMWINDOW | WB_NOBORDER | WB_NOSHADOW | WB_MOVEABLE | WB_OWNERDRAWDECORATION  ),
    m_pEditWin( pEditWin ),
    m_sText( sLabel ),
    m_bIsHeader( bHeader )
{
    // Get the font and configure it
    Font aFont = GetSettings().GetStyleSettings().GetToolFont();
    SetZoomedPointFont( aFont );

    // Use pixels for the rest of the drawing
    SetMapMode( MapMode ( MAP_PIXEL ) );

    // Compute the position & size of the window
    Rectangle aTextRect;
    GetTextBoundRect( aTextRect, String( sLabel ) );
    Rectangle aTextPxRect = LogicToPixel( aTextRect );

    Size  aBoxSize ( aTextPxRect.GetWidth() + TEXT_PADDING * 2,
                     aTextPxRect.GetHeight() + TEXT_PADDING  * 2 );

    long nYFooterOff = 0;
    if ( !bHeader )
        nYFooterOff = aBoxSize.Height();

    Size aPosOffset ( pEditWin->GetOutOffXPixel(), pEditWin->GetOutOffYPixel() );
    Point aBoxPos( aPosOffset.Width() + aOffset.X() - aBoxSize.Width() - BOX_DISTANCE,
                   aPosOffset.Height() + aOffset.Y() - nYFooterOff );

    // Set the position & Size of the window
    SetPosSizePixel( aBoxPos, aBoxSize );

    // TODO Add the list_add.png picture
}

void SwHeaderFooterWin::Paint( const Rectangle& rRect )
{
    // Colors
    basegfx::BColor aLineColor = SwViewOption::GetHeaderFooterMarkColor().getBColor();
    basegfx::BColor aHslLine = basegfx::tools::rgb2hsl( aLineColor );
    double nLuminance = aHslLine.getZ() * 2.5;
    if ( nLuminance == 0 )
        nLuminance = 0.5;
    else if ( nLuminance >= 1.0 )
        nLuminance = aHslLine.getZ() * 0.4;
    aHslLine.setZ( nLuminance );
    basegfx::BColor aFillColor = basegfx::tools::hsl2rgb( aHslLine );

    // Draw the background rect
    SetFillColor( Color ( aFillColor ) );
    SetLineColor( Color ( aFillColor ) );
    DrawRect( rRect );

    // Draw the lines around the rect
    SetLineColor( Color( aLineColor ) );
    basegfx::B2DPolygon aPolygon;
    aPolygon.append( basegfx::B2DPoint( rRect.Left(), rRect.Top() ) );
    aPolygon.append( basegfx::B2DPoint( rRect.Left(), rRect.Bottom() ) );
    DrawPolyLine( aPolygon, 1.0 );

    aPolygon.clear();
    aPolygon.append( basegfx::B2DPoint( rRect.Right(), rRect.Top() ) );
    aPolygon.append( basegfx::B2DPoint( rRect.Right(), rRect.Bottom() ) );
    DrawPolyLine( aPolygon, 1.0 );

    long nYLine = rRect.Bottom();
    if ( !m_bIsHeader )
        nYLine = rRect.Top();
    aPolygon.clear();
    aPolygon.append( basegfx::B2DPoint( rRect.Left(), nYLine ) );
    aPolygon.append( basegfx::B2DPoint( rRect.Right(), nYLine ) );
    DrawPolyLine( aPolygon, 1.0 );


    // Draw the text
    SetTextColor( Color( aLineColor ) );
    DrawText( Point( rRect.Left() + TEXT_PADDING, rRect.Top() + TEXT_PADDING ),
           String( m_sText ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
