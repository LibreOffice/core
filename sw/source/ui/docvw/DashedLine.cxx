/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <DashedLine.hxx>

#include <basegfx/color/bcolortools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

SwDashedLine::SwDashedLine( Window* pParent, Color& ( *pColorFn )() ) :
    FixedLine( pParent, WB_DIALOGCONTROL | WB_HORZ ),
    m_pColorFn( pColorFn )
{
}

SwDashedLine::~SwDashedLine( )
{
}

void SwDashedLine::Paint( const Rectangle& )
{
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    drawinglayer::processor2d::BaseProcessor2D * pProcessor =
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(
                    *this, aNewViewInfos );

    
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
    basegfx::BColor aColor = m_pColorFn().getBColor();
    if ( rSettings.GetHighContrastMode( ) )
    {
        
        aColor = rSettings.GetDialogTextColor().getBColor();
    }
    else
    {
        
        basegfx::BColor aHslLine = basegfx::tools::rgb2hsl( aColor );
        double nLuminance = aHslLine.getZ();
        nLuminance += ( 1.0 - nLuminance ) * 0.75;
        if ( aHslLine.getZ() > 0.7 )
            nLuminance = aHslLine.getZ() * 0.7;
        aHslLine.setZ( nLuminance );
        const basegfx::BColor aOtherColor = basegfx::tools::hsl2rgb( aHslLine );

        
        drawinglayer::primitive2d::PolygonHairlinePrimitive2D * pPlainLine =
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    aPolygon, aOtherColor );

        aSeq[0] = drawinglayer::primitive2d::Primitive2DReference( pPlainLine );
        
        aStrokePattern.push_back( 3 );
        aStrokePattern.push_back( 3 );

        aSeq.realloc( 2 );
    }

    
    drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D * pLine =
            new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D (
                basegfx::B2DPolyPolygon( aPolygon ),
                drawinglayer::attribute::LineAttribute( m_pColorFn().getBColor() ),
                drawinglayer::attribute::StrokeAttribute( aStrokePattern ) );

    aSeq[ aSeq.getLength() - 1 ] = drawinglayer::primitive2d::Primitive2DReference( pLine );

    pProcessor->process( aSeq );
    delete pProcessor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
