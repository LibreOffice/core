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

#include <DashedLine.hxx>

#include <basegfx/color/bcolortools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <svx/sdr/contact/objectcontacttools.hxx>
#include <vcl/svapp.hxx>

SwDashedLine::SwDashedLine( Window* pParent, const basegfx::BColor& rColor ) :
    FixedLine( pParent, WB_DIALOGCONTROL | WB_HORZ ),
    m_aColor( rColor )
{
}

SwDashedLine::~SwDashedLine( )
{
}

void SwDashedLine::Paint( const Rectangle& )
{
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    drawinglayer::processor2d::BaseProcessor2D * pProcessor =
        sdr::contact::createBaseProcessor2DFromOutputDevice(
                    *this, aNewViewInfos );

    // Compute the start and end points
    const Rectangle aRect( Rectangle( Point( 0, 0 ), PixelToLogic( GetSizePixel() ) ) );
    double nHalfWidth = double( aRect.Top() + aRect.Bottom() ) / 2.0;

    basegfx::B2DPoint aStart( double( aRect.Left() ), nHalfWidth );
    basegfx::B2DPoint aEnd( double( aRect.Right() ), nHalfWidth );

    basegfx::B2DPolygon aPolygon;
    aPolygon.append( aStart );
    aPolygon.append( aEnd );

    drawinglayer::primitive2d::Primitive2DSequence aSeq( 1 );

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    std::vector< double > aStrokePattern;
    basegfx::BColor aColor = m_aColor;
    if ( rSettings.GetHighContrastMode( ) )
    {
        // Only a solid line in high contrast mode
        aColor = rSettings.GetDialogTextColor().getBColor();
    }
    else
    {
        // Get a color for the contrast
        basegfx::BColor aHslLine = basegfx::tools::rgb2hsl( aColor );
        double nLuminance = aHslLine.getZ();
        nLuminance += ( 1.0 - nLuminance ) * 0.75;
        if ( aHslLine.getZ() > 0.7 )
            nLuminance = aHslLine.getZ() * 0.7;
        aHslLine.setZ( nLuminance );
        const basegfx::BColor aOtherColor = basegfx::tools::hsl2rgb( aHslLine );

        // Compute the plain line
        drawinglayer::primitive2d::PolygonHairlinePrimitive2D * pPlainLine =
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    aPolygon, aOtherColor );

        aSeq[0] = drawinglayer::primitive2d::Primitive2DReference( pPlainLine );
        // Dashed line in twips
        aStrokePattern.push_back( 3 );
        aStrokePattern.push_back( 3 );

        aSeq.realloc( 2 );
    }

    // Compute the dashed line primitive
    drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D * pLine =
            new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D (
                basegfx::B2DPolyPolygon( aPolygon ),
                drawinglayer::attribute::LineAttribute( m_aColor ),
                drawinglayer::attribute::StrokeAttribute( aStrokePattern ) );

    aSeq[ aSeq.getLength() - 1 ] = drawinglayer::primitive2d::Primitive2DReference( pLine );

    pProcessor->process( aSeq );
    delete pProcessor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
