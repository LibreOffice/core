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

#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include "swfwriter.hxx"
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/graphictools.hxx>

#include <zlib.h>

#include <vcl/salbtype.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <memory>

using namespace ::swf;
using namespace ::std;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;

static MapMode aTWIPSMode( MAP_TWIP );
static MapMode a100thmmMode( MAP_100TH_MM );


Point Writer::map( const Point& rPoint ) const
{
    const MapMode& aSourceMapMode = mpVDev->GetMapMode();

    Point retPoint = mpVDev->LogicToLogic( rPoint,  &aSourceMapMode, &aTWIPSMode );

    // AS: Produces a 'possible loss of data' warning that we can't fix without
    //  hurting code readability.
    retPoint.X() = (long)( retPoint.X() * mnDocXScale );
    retPoint.Y() = (long)( retPoint.Y() * mnDocYScale );

    return retPoint;
}


Size Writer::map( const Size& rSize ) const
{
    const MapMode& aSourceMapMode = mpVDev->GetMapMode();

    Size retSize = mpVDev->LogicToLogic( rSize,  &aSourceMapMode, &aTWIPSMode );

    // AS: Produces a 'possible loss of data' warning that we can't fix without
    //  hurting code readability.
    retSize.Width() = (long)( retSize.Width() * mnDocXScale );
    retSize.Height() = (long)( retSize.Height() * mnDocYScale );

    return retSize;
}


void Writer::map( tools::PolyPolygon& rPolyPolygon ) const
{
    const sal_uInt16 nPolyCount = rPolyPolygon.Count();
    if( nPolyCount )
    {
        sal_uInt16 nPoly, nPoint, nPointCount;
        for( nPoly = 0; nPoly < nPolyCount; nPoly++ )
        {
            tools::Polygon& rPoly = rPolyPolygon[nPoly];
            nPointCount = rPoly.GetSize();

            for( nPoint = 0; nPoint < nPointCount; nPoint++ )
            {
                rPoly[nPoint] = map( rPoly[nPoint] );
            }
        }
    }
}


sal_Int32 Writer::mapRelative( sal_Int32 n100thMM ) const
{
    MapMode aSourceMapMode( mpVDev->GetMapMode() );
    aSourceMapMode.SetOrigin( Point() );

    sal_Int32 nTwips = mpVDev->LogicToLogic( Point( n100thMM, n100thMM ),  &aSourceMapMode, &aTWIPSMode ).X();
    return nTwips;
}


void Writer::Impl_addPolygon( BitStream& rBits, const tools::Polygon& rPoly, bool bFilled )
{
    Point aLastPoint( rPoly[0] );

    Impl_addShapeRecordChange( rBits, _Int16(aLastPoint.X()),_Int16(aLastPoint.Y()), bFilled );

    sal_uInt16 i = 0, nSize = rPoly.GetSize();

    double d = 16.0f;

    // points
    while( ( i + 1 ) < nSize )
    {
        if( ( i + 3 ) < nSize )
        {
            PolyFlags P1( rPoly.GetFlags( i ) );
            PolyFlags P4( rPoly.GetFlags( i + 3 ) );

            if( ( POLY_NORMAL == P1 || POLY_SMOOTH == P1 || POLY_SYMMTR == P1 ) &&
                ( POLY_CONTROL == rPoly.GetFlags( i + 1 ) ) &&
                ( POLY_CONTROL == rPoly.GetFlags( i + 2 ) ) &&
                ( POLY_NORMAL == P4 || POLY_SMOOTH == P4 || POLY_SYMMTR == P4 ) )
            {
                Impl_quadBezierApprox( rBits, aLastPoint, d*d,
                                      rPoly.GetPoint( i ).X(),   rPoly.GetPoint( i ).Y(),
                                      rPoly.GetPoint( i+1 ).X(), rPoly.GetPoint( i+1 ).Y(),
                                      rPoly.GetPoint( i+2 ).X(), rPoly.GetPoint( i+2 ).Y(),
                                      rPoly.GetPoint( i+3 ).X(), rPoly.GetPoint( i+3 ).Y() );
                i += 3;
                continue;
            }
        }

        ++i;

        const Point aPolyPoint( rPoly[ i ] );
        if( aPolyPoint != aLastPoint )
        {
            Impl_addStraightEdgeRecord( rBits, _Int16(aPolyPoint.X() - aLastPoint.X()),_Int16(aPolyPoint.Y() - aLastPoint.Y()));
            aLastPoint = aPolyPoint;
        }
    }

    if( bFilled && (rPoly[0] != rPoly[nSize-1]))
    {
        const Point aPolyPoint( rPoly[ 0 ] );
        if( aPolyPoint != aLastPoint )
        {
            Impl_addStraightEdgeRecord( rBits, _Int16(aPolyPoint.X() - aLastPoint.X()),_Int16(aPolyPoint.Y() - aLastPoint.Y()));
        }
    }
}


/** Exports a style change record with a move to (x,y) and depending on bFilled a line style 1 or fill style 1
*/
void Writer::Impl_addShapeRecordChange( BitStream& rBits, sal_Int16 dx, sal_Int16 dy, bool bFilled )
{
    rBits.writeUB( 0, 1 );          // TypeFlag
    rBits.writeUB( 0, 1 );          // StateNewStyles
    rBits.writeUB( sal_uInt32(!bFilled), 1 ); // StateLineStyle
    rBits.writeUB( 0, 1 );          // StateFillStyle0
    rBits.writeUB( bFilled ? 1 : 0, 1 );        // StateFillStyle1
    rBits.writeUB( 1, 1 );          // StateMoveTo

    sal_uInt16 nMoveBits = max( getMaxBitsSigned( dx ), getMaxBitsSigned( dy ) );

    rBits.writeUB( nMoveBits, 5 );  // Number of bits per value
                                    // TODO: Optimize horizontal and vertical lines
    rBits.writeSB( dx, nMoveBits ); // DeltaX
    rBits.writeSB( dy, nMoveBits ); // DeltaY

    rBits.writeUB( 1, 1 );          // set FillStyle1 or LineStyle to 1
}


/** Exports a straight edge record
*/
void Writer::Impl_addStraightEdgeRecord( BitStream& rBits, sal_Int16 dx, sal_Int16 dy )
{
    rBits.writeUB( 1, 1 );          // TypeFlag
    rBits.writeUB( 1, 1 );          // StraightFlag

    sal_uInt16 nBits = max( getMaxBitsSigned( dx ), getMaxBitsSigned( dy ) );

    rBits.writeUB( nBits - 2, 4 );  // Number of bits per value

    if( (dx != 0) && (dy != 0) )
    {
        rBits.writeUB( 1, 1 );          // GeneralLineFlag
        rBits.writeSB( dx, nBits );     // DeltaX
        rBits.writeSB( dy, nBits );     // DeltaY
    }
    else
    {
        rBits.writeUB( 0, 1 );
        rBits.writeUB( sal_uInt32( dx == 0 ), 1 );
        if( dx == 0 )
        {
            rBits.writeSB( dy, nBits );     // DeltaY
        }
        else
        {
            rBits.writeSB( dx, nBits );     // DeltaX
        }
    }
}


/** Exports a curved edge record
*/
void Writer::Impl_addCurvedEdgeRecord( BitStream& rBits, sal_Int16 control_dx, sal_Int16 control_dy, sal_Int16 anchor_dx, sal_Int16 anchor_dy )
{
    rBits.writeUB( 1, 1 );          // TypeFlag
    rBits.writeUB( 0, 1 );          // CurvedFlag

    sal_uInt8 nBits = static_cast<sal_uInt8>(
        max( getMaxBitsSigned( control_dx ),
            max( getMaxBitsSigned( control_dy ),
                max( getMaxBitsSigned( anchor_dx ),
                    max( getMaxBitsSigned( anchor_dy ), (sal_uInt16)3 ) ) ) ) );

    rBits.writeUB( nBits - 2, 4 );  // Number of bits per value

    rBits.writeSB( control_dx, nBits );     // DeltaX
    rBits.writeSB( control_dy, nBits );     // DeltaY
    rBits.writeSB( anchor_dx, nBits );      // DeltaX
    rBits.writeSB( anchor_dy, nBits );      // DeltaY
}


/** Exports a end shape record
*/
void Writer::Impl_addEndShapeRecord( BitStream& rBits )
{
    rBits.writeUB( 0, 6 );
}


void Writer::Impl_writePolygon( const tools::Polygon& rPoly, bool bFilled )
{
    tools::PolyPolygon aPolyPoly( rPoly );
    Impl_writePolyPolygon( aPolyPoly, bFilled );
}


void Writer::Impl_writePolygon( const tools::Polygon& rPoly, bool bFilled, const Color& rFillColor, const Color& rLineColor )
{
    tools::PolyPolygon aPolyPoly( rPoly );
    Impl_writePolyPolygon( aPolyPoly, bFilled, rFillColor, rLineColor );
}


void Writer::Impl_writePolyPolygon( const tools::PolyPolygon& rPolyPoly, bool bFilled, sal_uInt8 nTransparence /* = 0 */ )
{
    Color aLineColor( mpVDev->GetLineColor() );
    if( 0 == aLineColor.GetTransparency() )
        aLineColor.SetTransparency( nTransparence );
    Color aFillColor( mpVDev->GetFillColor() );
    if( 0 == aFillColor.GetTransparency() )
        aFillColor.SetTransparency( nTransparence );
    Impl_writePolyPolygon(rPolyPoly, bFilled, aFillColor, aLineColor );
}


void Writer::Impl_writePolyPolygon( const tools::PolyPolygon& rPolyPoly, bool bFilled, const Color& rFillColor, const Color& rLineColor )
{
    tools::PolyPolygon aPolyPoly( rPolyPoly );

    if( aPolyPoly.Count() )
    {
        map( aPolyPoly );

        if( mpClipPolyPolygon )
            rPolyPoly.GetIntersection( *mpClipPolyPolygon, aPolyPoly );

        sal_uInt16 nID;
        if( bFilled )
        {
            Color aFillColor( rFillColor );
            if( 0 != mnGlobalTransparency )
                aFillColor.SetTransparency( mnGlobalTransparency );

            FillStyle aStyle( aFillColor );
            nID = defineShape( aPolyPoly, aStyle );
        }
        else
        {
            Color aLineColor( rLineColor );
            if( 0 != mnGlobalTransparency )
                aLineColor.SetTransparency( mnGlobalTransparency );

            nID = defineShape( aPolyPoly, 1, aLineColor );
        }
        maShapeIds.push_back( nID );
    }
}


/** A gradient is a transition from one color to another, rendered inside a given polypolygon */
void Writer::Impl_writeGradientEx( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
    if( rPolyPoly.Count() )
    {
        tools::PolyPolygon aPolyPolygon( rPolyPoly );
        map( aPolyPolygon );

        if( (rGradient.GetStyle() == GradientStyle_LINEAR && rGradient.GetAngle() == 900) || (rGradient.GetStyle() == GradientStyle_RADIAL)  )
        {
            const Rectangle aBoundRect( aPolyPolygon.GetBoundRect() );

            FillStyle aFillStyle( aBoundRect, rGradient );

            sal_uInt16 nShapeId = defineShape( aPolyPolygon, aFillStyle );
            maShapeIds.push_back( nShapeId );
        }
        else
        {
            setClipping( &aPolyPolygon );

            // render the gradient filling to simple polygons
            {
                GDIMetaFile aTmpMtf;
                mpVDev->AddGradientActions( aPolyPolygon.GetBoundRect(), rGradient, aTmpMtf );
                Impl_writeActions( aTmpMtf );
            }

            setClipping( nullptr );
        }
    }
}


void Writer::setClipping( const tools::PolyPolygon* pClipPolyPolygon )
{
    mpClipPolyPolygon = pClipPolyPolygon;
}


// AS: Just comparing fonts straight up is too literal.  There are some
//  differences in font that actually require different glyphs to be defined,
//  and some that don't.  This function is meant to capture all the differences
//  that we care about.
bool compare_fonts_for_me(const vcl::Font& rFont1, const vcl::Font& rFont2)
{
    return rFont1.GetFamilyName() == rFont2.GetFamilyName() &&
            rFont1.GetWeight() == rFont2.GetWeight() &&
            rFont1.GetItalic() == rFont2.GetItalic() &&
            rFont1.IsOutline() == rFont2.IsOutline() &&
            rFont1.IsShadow() == rFont2.IsShadow() &&
            rFont1.GetRelief() == rFont2.GetRelief();
}


FlashFont& Writer::Impl_getFont( const vcl::Font& rFont )
{
    FontMap::iterator aIter( maFonts.begin() );
    const FontMap::iterator aEnd( maFonts.end() );

    for(; aIter != aEnd; ++aIter)
    {
        const vcl::Font tempFont = (*aIter)->getFont();
        if( compare_fonts_for_me(tempFont, rFont) )
        {
            return **aIter;
        }
    }

    FlashFont* pFont = new FlashFont( rFont, createID() );
    maFonts.push_back( pFont );
    return *pFont;
}


void Writer::Impl_writeText( const Point& rPos, const OUString& rText, const long* pDXArray, long nWidth )
{
    const FontMetric aMetric( mpVDev->GetFontMetric() );

    bool bTextSpecial = aMetric.IsShadow() || aMetric.IsOutline() || (aMetric.GetRelief() != RELIEF_NONE);

    if( !bTextSpecial )
    {
        Impl_writeText( rPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
    }
    else
    {
        if( aMetric.GetRelief() != RELIEF_NONE )
        {
            Color aReliefColor( COL_LIGHTGRAY );
            Color aTextColor( mpVDev->GetTextColor() );

            if ( aTextColor.GetColor() == COL_BLACK )
                aTextColor = Color( COL_WHITE );

            if ( aTextColor.GetColor() == COL_WHITE )
                aReliefColor = Color( COL_BLACK );


            Point aPos( rPos );
            Point aOffset( 6,6 );

            if ( aMetric.GetRelief() == RELIEF_ENGRAVED )
            {
                aPos -= aOffset;
            }
            else
            {
                aPos += aOffset;
            }

            Impl_writeText( aPos, rText, pDXArray, nWidth, aReliefColor );
            Impl_writeText( rPos, rText, pDXArray, nWidth, aTextColor );
        }
        else
        {
            if( aMetric.IsShadow() )
            {
                long nOff = 1 + ((aMetric.GetLineHeight()-24)/24);
                if ( aMetric.IsOutline() )
                    nOff += 6;

                Color aTextColor( mpVDev->GetTextColor() );
                Color aShadowColor( COL_BLACK );

                if ( (aTextColor.GetColor() == COL_BLACK) || (aTextColor.GetLuminance() < 8) )
                    aShadowColor = Color( COL_LIGHTGRAY );

                Point aPos( rPos );
                aPos += Point( nOff, nOff );
                Impl_writeText( aPos, rText, pDXArray, nWidth, aShadowColor );

                if( !aMetric.IsOutline() )
                {
                    Impl_writeText( rPos, rText, pDXArray, nWidth, aTextColor );
                }
            }

            if( aMetric.IsOutline() )
            {
                Point aPos = rPos + Point( -6, -6 );
                Impl_writeText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point(+6,+6);
                Impl_writeText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point(-6,+0);
                Impl_writeText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point(-6,+6);
                Impl_writeText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point(+0,+6);
                Impl_writeText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point(+0,-6);
                Impl_writeText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point(+6,-1);
                Impl_writeText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point(+6,+0);
                Impl_writeText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );

                Impl_writeText( rPos, rText, pDXArray, nWidth, Color( COL_WHITE ) );
            }
        }
    }
}

void Writer::Impl_writeText( const Point& rPos, const OUString& rText, const long* pDXArray, long nWidth, Color aTextColor )
{
    sal_Int32 nLen = rText.getLength();

    if( !nLen )
        return;

    const bool bRTL = bool(mpVDev->GetLayoutMode() & TEXT_LAYOUT_BIDI_RTL);

    sal_Int16 nScriptType = ScriptType::LATIN;
    Reference < XBreakIterator > xBI( Impl_GetBreakIterator() );
    if( xBI.is() )
    {
        const OUString oText( rText );
        nScriptType = xBI->getScriptType( oText, 0 );
    }

    // if the text is either right to left or complex or asian, we
    // ask the output device for a polygon representation.
    // On complex and asian text, each unicode character can have
    // different glyph representation, based on context. Also positioning
    // is not trivial so we let the output device do it for us.
    if( bRTL || (nScriptType != ScriptType::LATIN) )
    {
        // todo: optimize me as this will generate a huge amount of duplicate polygons
        tools::PolyPolygon aPolyPoygon;
        mpVDev->GetTextOutline( aPolyPoygon, rText, 0, 0, (sal_uInt16)nLen, true, nWidth, pDXArray );
        aPolyPoygon.Translate( rPos );
        Impl_writePolyPolygon( aPolyPoygon, true, aTextColor, aTextColor );
    }
    else
    {
        Size    aNormSize;
        std::unique_ptr<long[]> pOwnArray;
        long* pDX;

        // get text sizes
        if( pDXArray )
        {
            aNormSize = Size( mpVDev->GetTextWidth( rText ), 0 );
            pDX = const_cast<long*>(pDXArray);
        }
        else
        {
            pOwnArray.reset(new long[ nLen ]);
            aNormSize = Size( mpVDev->GetTextArray( rText, pOwnArray.get() ), 0 );
            pDX = pOwnArray.get();
        }

        if( nLen > 1 )
        {
            aNormSize.Width() = pDX[ nLen - 2 ] + mpVDev->GetTextWidth( OUString(rText[nLen - 1]) );

            if( nWidth && aNormSize.Width() && ( nWidth != aNormSize.Width() ) )
            {
                const double fFactor = (double) nWidth / aNormSize.Width();

                for( sal_Int32 i = 0; i < ( nLen - 1 ); i++ )
                    pDX[ i ] = FRound( pDX[ i ] * fFactor );
            }
        }

        vcl::Font aOldFont( mpVDev->GetFont() );
        Point               aBaseLinePos( rPos );

        vcl::Font aFont(aOldFont);
        short nOrientation = aFont.GetOrientation();
        aFont.SetOrientation( 0 );
        aFont.SetUnderline(UNDERLINE_NONE);
        aFont.SetStrikeout(STRIKEOUT_NONE);
        mpVDev->SetFont( aFont );

        const FontMetric    aMetric( mpVDev->GetFontMetric() );

        FlashFont&          rFlashFont = Impl_getFont( aFont );

        // always adjust text position to match baseline alignment
        switch( aOldFont.GetAlign() )
        {
            case( ALIGN_TOP ):
                aBaseLinePos.Y() += aMetric.GetAscent();
            break;

            case( ALIGN_BOTTOM ):
                aBaseLinePos.Y() -= aMetric.GetDescent();
            break;

            default:
            break;
        }

        // get mapped text position
        const Point aPt( map( aBaseLinePos ) );

        // write text element

#if 0 // makes the calculated bound rect visible for debugging
{
    tools::Polygon aTmpPoly( aPoly );
    sal_uInt16 nID = FlashGeometryExporter::writePolygonShape( aMovieStream, aTmpPoly, false, Color(COL_MAGENTA), Color(COL_MAGENTA), mpClipPolyPolygon  );
    ImplPlaceObject( nID );
}
#endif

        // CL: This is still a hack until we figure out how to calculate a correct bound rect
        //     for rotated text
        Rectangle textBounds( 0, 0, static_cast<long>(mnDocWidth*mnDocXScale), static_cast<long>(mnDocHeight*mnDocYScale) );
        double scale = 1.0;

        // scale width if we have a stretched text
        if( 0 != aFont.GetSize().Width() )
        {
            vcl::Font aTmpFont( aFont );
            aTmpFont.SetWidth(0);
            mpVDev->SetFont( aTmpFont );

            const FontMetric aMetric2( mpVDev->GetFontMetric() );
            mpVDev->SetFont( aFont );

            const long n1 = aFont.GetSize().Width();
            const long n2 = aMetric2.GetSize().Width();
            scale =  (double)n1 / (double)n2;
        }

        basegfx::B2DHomMatrix m(basegfx::tools::createRotateB2DHomMatrix(static_cast<double>(nOrientation) * F_PI1800));
        m.translate( double(aPt.X() / scale), double(aPt.Y()) );
        m.scale( scale, scale );

        sal_Int16 nHeight = _Int16( map( Size( 0, aFont.GetHeight() ) ).Height() );

        startTag( TAG_DEFINETEXT );

        sal_uInt16 nTextId = createID();

        mpTag->addUI16( nTextId );
        mpTag->addRect( textBounds );
        mpTag->addMatrix( m );

        sal_uInt8 nGlyphBits = 16;
        sal_uInt8 nAdvanceBits = 16;

        mpTag->addUI8( nGlyphBits );
        mpTag->addUI8( nAdvanceBits );

        // text style change record
        mpTag->addUI8( 0x8c );
        mpTag->addUI16( rFlashFont.getID() );
        mpTag->addRGB( aTextColor );
        mpTag->addUI16( _uInt16( nHeight ) );

        DBG_ASSERT( nLen <= 127, "TODO: handle text with more than 127 characters" );

        // Glyph record
        mpTag->addUI8( (sal_uInt8) nLen );

        BitStream aBits;

        sal_Int32 nLastDX = 0;
        sal_Int32 nAdvance;
        for( sal_Int32 i = 0; i < nLen; i++  )
        {
            if( i < (nLen-1) )
            {
                nAdvance = pDX[i] - nLastDX;
                nLastDX = pDX[i];
            }
            else
            {
                nAdvance = 0;
            }

            aBits.writeUB( rFlashFont.getGlyph(rText[i],mpVDev), nGlyphBits );
            aBits.writeSB( _Int16(map( Size( (long)( nAdvance / scale ), 0 ) ).Width() ), nAdvanceBits );
        }

        mpTag->addBits( aBits );
        mpTag->addUI8( 0 );

        endTag();

        maShapeIds.push_back( nTextId );

        // AS: Write strikeout and underline, if necessary.  This code was originally taken from the SVG
        //  export facility, although the positioning had to be tweaked a little.  I can't explain the
        //  numbers, but the flash lines up very well with the original OOo document.  All of this should
        //  probably be converted to polygons as part of the meta file, though, as we don't handle any
        //  fancy lines (like dashes).
        if( ( aOldFont.GetStrikeout() != STRIKEOUT_NONE ) || ( aOldFont.GetUnderline() != UNDERLINE_NONE ) )
        {
            tools::Polygon aPoly( 4 );
            const long  nLineHeight = std::max( (long) FRound( aMetric.GetLineHeight() * 0.05 ), (long) 1 );

            if( aOldFont.GetStrikeout() != STRIKEOUT_NONE )
            {
                aPoly[ 0 ].X() = aBaseLinePos.X();
                aPoly[ 0 ].Y() = aBaseLinePos.Y() - FRound( aMetric.GetAscent() * 0.26 ) - nLineHeight;
                aPoly[ 1 ].X() = aPoly[ 0 ].X() + aNormSize.Width() - 1;
                aPoly[ 1 ].Y() = aPoly[ 0 ].Y();
                aPoly[ 2 ].X() = aPoly[ 1 ].X();
                aPoly[ 2 ].Y() = aPoly[ 1 ].Y() + nLineHeight - 1;
                aPoly[ 3 ].X() = aPoly[ 0 ].X();
                aPoly[ 3 ].Y() = aPoly[ 2 ].Y();

                Impl_writePolygon( aPoly, true, aTextColor, aTextColor );
            }

            // AS: The factor of 1.5 on the nLineHeight is a magic number.  I'm not sure why it works,
            //  but it looks good to me.
            if( aOldFont.GetUnderline() != UNDERLINE_NONE )
            {
                aPoly[ 0 ].X() = aBaseLinePos.X();
                aPoly[ 0 ].Y() = static_cast<long>(aBaseLinePos.Y() + 1.5*nLineHeight);
                aPoly[ 1 ].X() = aPoly[ 0 ].X() + aNormSize.Width() - 1;
                aPoly[ 1 ].Y() = aPoly[ 0 ].Y();
                aPoly[ 2 ].X() = aPoly[ 1 ].X();
                aPoly[ 2 ].Y() = aPoly[ 1 ].Y() + nLineHeight - 1;
                aPoly[ 3 ].X() = aPoly[ 0 ].X();
                aPoly[ 3 ].Y() = aPoly[ 2 ].Y();

                Impl_writePolygon( aPoly, true, aTextColor, aTextColor );
            }
        }

        mpVDev->SetFont( aOldFont );
    }
}

// AS: Because JPEGs require the alpha channel provided separately (JPEG does not
//  natively support alpha channel, but SWF lets you provide it separately), we
//  extract the alpha channel into a separate array here.
void getBitmapData( const BitmapEx& aBmpEx, sal_uInt8*& tgadata, sal_uInt8*& tgaAlphadata, sal_uInt32& nWidth, sal_uInt32& nHeight )
{
    if( !aBmpEx.IsEmpty() )
    {
        Bitmap              aBmp( aBmpEx.GetBitmap() );
        BitmapReadAccess*   pRAcc = aBmp.AcquireReadAccess();

        if( pRAcc )
        {
            AlphaMask   aAlpha;
            nWidth = pRAcc->Width();
            nHeight = pRAcc->Height();
            tgadata = new sal_uInt8[nWidth*nHeight*4];
            tgaAlphadata = new sal_uInt8[nWidth*nHeight];
            sal_uInt8* p = tgadata, *pAlpha = tgaAlphadata;


            if( aBmpEx.IsAlpha() )
                aAlpha = aBmpEx.GetAlpha();
            else if( aBmpEx.IsTransparent() )
                aAlpha = aBmpEx.GetMask();
            else
            {
                sal_uInt8 cAlphaVal = 0;
                aAlpha = AlphaMask( aBmp.GetSizePixel(), &cAlphaVal );
            }

            BitmapReadAccess* pAAcc = aAlpha.AcquireReadAccess();

            if( pAAcc )
            {
                for( sal_uInt32 nY = 0; nY < nHeight; nY++ )
                {
                    for( sal_uInt32 nX = 0; nX < nWidth; nX++ )
                    {
                        const sal_uInt8     nAlpha = pAAcc->GetPixel( nY, nX ).GetIndex();
                        const BitmapColor   aPixelColor( pRAcc->GetColor( nY, nX ) );

                        if( nAlpha == 0xff )
                        {
                            *p++ = 0;
                            *p++ = 0;
                            *p++ = 0;
                            *p++ = 0;
                        }
                        else
                        {
                            *p++ = 0xff-nAlpha;
                            *p++ = aPixelColor.GetRed();
                            *p++ = aPixelColor.GetGreen();
                            *p++ = aPixelColor.GetBlue();
                        }
                        *pAlpha++ = 0xff - nAlpha;
                    }
                }

                aAlpha.ReleaseAccess( pAAcc );
            }

            Bitmap::ReleaseAccess( pRAcc );
        }
    }
}


sal_uInt16 Writer::defineBitmap( const BitmapEx &bmpSource, sal_Int32 nJPEGQualityLevel )
{
    BitmapChecksum bmpChecksum = bmpSource.GetChecksum();

    ChecksumCache::iterator it = mBitmapCache.find(bmpChecksum);

    // AS: We already exported this bitmap, so just return its ID.
    if (mBitmapCache.end() != it)
        return it->second;

    sal_uInt16 nBitmapId = createID();
    mBitmapCache[bmpChecksum] = nBitmapId;

    // AS: OK, we have a good image, so now we decide whether or not to JPEG it or
    //  or Lossless compress it.

    // Figure out lossless size
    sal_uInt8 *pImageData, *pAlphaData;
    sal_uInt32 width(0), height(0);

    getBitmapData( bmpSource, pImageData, pAlphaData, width, height );
    sal_uInt32 raw_size = width * height * 4;
    uLongf compressed_size = raw_size + (sal_uInt32)(raw_size/100) + 12;
    std::unique_ptr<sal_uInt8[]> pCompressed(new sal_uInt8[ compressed_size ]);

#ifdef DBG_UTIL
    if(compress2(pCompressed.get(), &compressed_size, pImageData, raw_size, Z_BEST_COMPRESSION) != Z_OK)
    {
        DBG_ASSERT( false, "compress2 failed!" ); ((void)0);
    }
#else
    compress2(pCompressed.get(), &compressed_size, pImageData, raw_size, Z_BEST_COMPRESSION);
#endif

    // AS: SWF files let you provide an Alpha mask for JPEG images, but we have
    //  to ZLIB compress the alpha channel separately.
    uLong alpha_compressed_size = 0;
    std::unique_ptr<sal_uInt8[]> pAlphaCompressed;
    if (bmpSource.IsAlpha() || bmpSource.IsTransparent())
    {
        alpha_compressed_size = uLongf(width * height + (sal_uInt32)(raw_size/100) + 12);
        pAlphaCompressed.reset(new sal_uInt8[ compressed_size ]);

#ifdef DBG_UTIL
        if(compress2(pAlphaCompressed.get(), &alpha_compressed_size, pAlphaData, width * height, Z_BEST_COMPRESSION) != Z_OK)
        {
            DBG_ASSERT( false, "compress2 failed!" ); ((void)0);
        }
#else
        compress2(pAlphaCompressed.get(), &alpha_compressed_size, pAlphaData, width * height, Z_BEST_COMPRESSION);
#endif
    }

    // Figure out JPEG size
    const sal_uInt8* pJpgData = nullptr;
    sal_uInt32 nJpgDataLength = 0xffffffff;

    Graphic aGraphic( bmpSource );
    SvMemoryStream aDstStm( 65535, 65535 );

    GraphicFilter aFilter;

    Sequence< PropertyValue > aFilterData(sal_Int32(nJPEGQualityLevel != -1));
    if( nJPEGQualityLevel != -1 )
    {
        aFilterData[0].Name = "Quality";
        aFilterData[0].Value <<= nJPEGQualityLevel;
    }

    if( aFilter.ExportGraphic( aGraphic, OUString(), aDstStm,
                                aFilter.GetExportFormatNumberForShortName( JPG_SHORTNAME ), &aFilterData ) == ERRCODE_NONE )
    {
        pJpgData = static_cast<const sal_uInt8*>(aDstStm.GetData());
        nJpgDataLength = aDstStm.Seek( STREAM_SEEK_TO_END );
    }

    // AS: Ok, now go ahead and use whichever is smaller.  If JPEG is smaller, then
    //  we have to export as TAG_DEFINEBITSJPEG3 in the case that there is alpha
    //  channel data.
    if ( pJpgData && ( nJpgDataLength + alpha_compressed_size < compressed_size) )
        Impl_writeJPEG(nBitmapId, pJpgData, nJpgDataLength, pAlphaCompressed.get(), alpha_compressed_size );
    else
        Impl_writeBmp( nBitmapId, width, height, pCompressed.get(), compressed_size );

    delete[] pImageData;
    delete[] pAlphaData;

    return nBitmapId;
}


void Writer::Impl_writeImage( const BitmapEx& rBmpEx, const Point& rPt, const Size& rSz, const Point& /* rSrcPt */, const Size& /* rSrcSz */, const Rectangle& rClipRect, bool bNeedToMapClipRect )
{
    if( !!rBmpEx )
    {
        BitmapEx bmpSource( rBmpEx );

        Rectangle originalPixelRect = Rectangle(Point(), bmpSource.GetSizePixel());

        Point srcPt( map(rPt) );
        Size srcSize( map(rSz) );
        Rectangle destRect( srcPt, srcSize );

        // AS: Christian, my scaling factors are different than yours, and work better for me.
        //  However, I can't explain why exactly.  I got some of this by trial and error.
        double XScale = destRect.GetWidth() ? static_cast<double>(originalPixelRect.GetWidth())/destRect.GetWidth() : 1.0;
        double YScale = destRect.GetHeight() ? static_cast<double>(originalPixelRect.GetHeight())/destRect.GetHeight() : 1.0;

        // AS: If rClipRect has a value set, then we need to crop the bmp appropriately.
        //  If a map event already occurred in the metafile, then we do not need to map
        //  the clip rect as it's already been done.
        if (!rClipRect.IsEmpty())
        {
            // AS: Christian, I also don't understand why bNeedToMapClipRect is necessary, but it
            //  works like a charm.  Usually, the map event in the meta file does not cause the
            //  clipping rectangle to get mapped.  However, sometimes there are multiple layers
            //  of mapping which eventually do cause the clipping rect to be mapped.
            Size clipSize( bNeedToMapClipRect ? map(rClipRect.GetSize()) : rClipRect.GetSize() );
            Rectangle clipRect = Rectangle(Point(), clipSize);
            destRect.Intersection( clipRect );

            Rectangle cropRect(destRect);

            // AS: The bmp origin is always 0,0 so we have to adjust before we crop.
            cropRect.Move(-srcPt.X(), -srcPt.Y());
            // AS: Rectangle has no scale function (?!) so I do it manually...
            Rectangle cropPixelRect(static_cast<long>(cropRect.Left()*XScale),
                                    static_cast<long>(cropRect.Top()*YScale),
                                    static_cast<long>(cropRect.Right()*XScale),
                                    static_cast<long>(cropRect.Bottom()*YScale));

            bmpSource.Crop(cropPixelRect);
        }

        if( !!bmpSource )
        {
            // #105949# fix images that are under 16 pixels width or height by
            //          expanding them. Some swf players can't display such small
            //          bitmaps
            const Size& rSizePixel = bmpSource.GetSizePixel();
            if( (rSizePixel.Width() < 16) || (rSizePixel.Height() < 16) )
            {
                const sal_uInt32 nDX = rSizePixel.Width() < 16 ? 16 - rSizePixel.Width() : 0;
                const sal_uInt32 nDY = rSizePixel.Height() < 16 ? 16 - rSizePixel.Height() : 0;
                bmpSource.Expand( nDX, nDY );
            }

            sal_Int32 nJPEGQuality = mnJPEGCompressMode;

            Size szDestPixel = mpVDev->LogicToPixel(srcSize, aTWIPSMode);

            double pixXScale = originalPixelRect.GetWidth() ? static_cast<double>(szDestPixel.Width()) / originalPixelRect.GetWidth() : 1.0;
            double pixYScale = originalPixelRect.GetHeight() ? static_cast<double>(szDestPixel.Height()) / originalPixelRect.GetHeight() : 1.0;

            // AS: If the image has been scaled down, then scale down the quality
            //   that we use for JPEG compression.
            if (pixXScale < 1.0 && pixYScale < 1.0)
            {

                double qualityScale = (pixXScale + pixYScale)/2;

                nJPEGQuality = (sal_Int32)( nJPEGQuality * qualityScale );

                if (nJPEGQuality < 10)
                    nJPEGQuality += 3;
            }

            sal_uInt16 nBitmapId = defineBitmap(bmpSource, nJPEGQuality);

            tools::Polygon aPoly( destRect );

            // AS: Since images are being cropped now, no translation is normally necessary.
            //  However, some things like graphical bullet points still get translated.
            ::basegfx::B2DHomMatrix m; // #i73264#
            m.scale(1.0/XScale, 1.0/YScale );
            if (destRect.Left() || destRect.Top())
                m.translate(destRect.Left(), destRect.Top());

            FillStyle aFillStyle( nBitmapId, true, m );

            sal_uInt16 nShapeId = defineShape( aPoly, aFillStyle );

            maShapeIds.push_back( nShapeId );
        }
    }
}


void Writer::Impl_writeBmp( sal_uInt16 nBitmapId, sal_uInt32 width, sal_uInt32 height, sal_uInt8 *pCompressed, sal_uInt32 compressed_size )
{
    startTag( TAG_DEFINEBITSLOSSLESS2 );

    mpTag->addUI16( nBitmapId );
    mpTag->addUI8( 5 );
    mpTag->addUI16( _uInt16(width) );
    mpTag->addUI16( _uInt16(height) );

    mpTag->Write( pCompressed, compressed_size );

    endTag();
}


void Writer::Impl_writeJPEG(sal_uInt16 nBitmapId, const sal_uInt8* pJpgData, sal_uInt32 nJpgDataLength, sal_uInt8 *pAlphaCompressed, sal_uInt32 alpha_compressed_size )
{
    // AS: Go through the actual JPEG bits, separating out the
    //  header fields from the actual image fields.  Fields are
    //  identifed by 0xFFXX where XX is the field type.  Both
    //  the header and the image need start and stop (D8 and D9),
    //  so that's why you see those written to both.  I don't
    //  really know what the rest of these are, I got it to work
    //  kind of by trial and error and by comparing with known
    //  good SWF files.
    sal_uInt8 cType = 0x01;
    const sal_uInt8* pJpgSearch = pJpgData;

    int nLength = 0;

    SvMemoryStream EncodingTableStream;
    SvMemoryStream ImageBitsStream;
    for (;pJpgSearch < pJpgData + nJpgDataLength; pJpgSearch += nLength)
    {

#ifdef DBG_UTIL
        if (0xFF != *pJpgSearch)
        {
            OSL_FAIL( "Expected JPEG marker." ); ((void)0);
        }
#endif

        cType = *(pJpgSearch + 1);

        if (0xD8 == cType || 0xD9 == cType)
        {
            nLength = 2;
        }
        else if (0xDA == cType)
        {
            //AS: This is the actual image data, and runs to the
            // end of the file (as best I know), minus 2 bytes
            // for the closing 0xFFD9.
            nLength = nJpgDataLength - (pJpgSearch - pJpgData) - 2;
        }
        else
        {
            // AS: Lengths are big endian.

            // Beware. pJpgSearch is not necessarily word-aligned,
            // so we access it byte-wise.

            // AS: Add 2 to the length to include the 0xFFXX itself.
            nLength = 2 + (pJpgSearch[2]<<8) + pJpgSearch[3];
        }

        // AS: I'm referring to libjpeg for a list of what these
        //  markers are.  See jdmarker.c for a list.
        // AS: I'm ignoring application specific markers 0xE1...0xEF
        //  and comments 0xFE.  I don't know what
        //  0xF0 or 0xFD are for, and they don't come up.
        //  Additionally, 0xDE and 0xDF aren't clear to me.
        switch(cType)
        {
        case 0xD8:
        case 0xD9:
            EncodingTableStream.Write( pJpgSearch, nLength );
            ImageBitsStream.Write( pJpgSearch, nLength );
            break;

        case 0x01:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xC4:
            EncodingTableStream.Write( pJpgSearch, nLength );
            break;

        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC5:
        case 0xC6:
        case 0xC7:
//      case 0xC8: Apparently reserved for JPEG extensions?
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCD:
        case 0xCE:
        case 0xCF:
        case 0xDA:
        case 0xE0:
            ImageBitsStream.Write( pJpgSearch, nLength );
            break;

        default:
            OSL_FAIL( "JPEG marker I didn't handle!" );

        }
    }

    EncodingTableStream.Seek( STREAM_SEEK_TO_END );
    sal_uInt32 nEncodingTableSize = EncodingTableStream.Tell();
    EncodingTableStream.Seek( STREAM_SEEK_TO_BEGIN );

    ImageBitsStream.Seek( STREAM_SEEK_TO_END );
    sal_uInt32 nImageBitsSize = ImageBitsStream.Tell();
    ImageBitsStream.Seek( STREAM_SEEK_TO_BEGIN );

    // AS: If we need alpha support, use TAG_DEFINEBITSJPEG3.
    if (alpha_compressed_size > 0)
    {
        startTag( TAG_DEFINEBITSJPEG3 );

        mpTag->addUI16( nBitmapId );

        mpTag->addUI32( nEncodingTableSize + nImageBitsSize );

        mpTag->Write(EncodingTableStream.GetData(), nEncodingTableSize);
        mpTag->Write(ImageBitsStream.GetData(), nImageBitsSize);

        mpTag->Write( pAlphaCompressed, alpha_compressed_size );

        endTag();
    }
    else
    {
        startTag( TAG_DEFINEBITSJPEG2 );

        mpTag->addUI16( nBitmapId );

        mpTag->Write(EncodingTableStream.GetData(), nEncodingTableSize);
        mpTag->Write(ImageBitsStream.GetData(), nImageBitsSize);

        endTag();
    }
}


void Writer::Impl_writeLine( const Point& rPt1, const Point& rPt2, const Color* pLineColor )
{
    Color aOldColor( mpVDev->GetLineColor() );
    if( pLineColor )
        mpVDev->SetLineColor( *pLineColor );

    const Point aPtAry[2] = { rPt1, rPt2 };
    tools::Polygon aPoly( 2, aPtAry );
    Impl_writePolyPolygon( aPoly, false );

    mpVDev->SetLineColor( aOldColor );
}


void Writer::Impl_writeRect( const Rectangle& rRect, long nRadX, long nRadY )
{
    if( (rRect.Top() == rRect.Bottom()) || (rRect.Left() == rRect.Right()) )
    {
        Color aColor( mpVDev->GetFillColor() );
        Impl_writeLine( rRect.TopLeft(), rRect.BottomRight(), &aColor );
    }
    else
    {
        tools::Polygon aPoly( rRect, nRadX, nRadY );
        Impl_writePolyPolygon( aPoly, true );
    }
}


void Writer::Impl_writeEllipse( const Point& rCenter, long nRadX, long nRadY )
{
    tools::Polygon aPoly( rCenter, nRadX, nRadY );
    Impl_writePolyPolygon( aPoly, false );
}


/** Writes the stroke defined by SvtGraphicStroke and returns true or it returns
    false if it can't handle this stroke.
*/
bool Writer::Impl_writeStroke( SvtGraphicStroke& rStroke )
{
    tools::Polygon aPolygon;
    rStroke.getPath( aPolygon );
    tools::PolyPolygon aPolyPolygon( aPolygon );

    map( aPolyPolygon );

    // as log as not LINESTYLE2 and DefineShape4 is used (which
    // added support for LineJoin), only round LineJoins are
    // supported. Fallback to MetaActionType::POLYLINE and MetaActionType::LINE
    if(SvtGraphicStroke::joinRound != rStroke.getJoinType())
        return false;

    tools::PolyPolygon aStartArrow;
    rStroke.getStartArrow( aStartArrow );
    if( 0 != aStartArrow.Count() )
        return false;       // todo: Implement line ends

    tools::PolyPolygon aEndArrow;
    rStroke.getEndArrow( aEndArrow );
    if( 0 != aEndArrow.Count() )
        return false;       // todo: Implement line ends

    SvtGraphicStroke::DashArray aDashArray;
    rStroke.getDashArray( aDashArray );
    if( 0 != aDashArray.size() )
        return false;       // todo: implement dashes

    Color aColor( mpVDev->GetLineColor() );

    if( 0.0 != rStroke.getTransparency() )
        aColor.SetTransparency( sal::static_int_cast<sal_uInt8>( MinMax( (long int)( rStroke.getTransparency() * 0xff ), 0, 0xff ) ) );

    sal_uInt16 nShapeId = defineShape( aPolyPolygon, sal::static_int_cast<sal_uInt16>( mapRelative( (sal_Int32)( rStroke.getStrokeWidth() ) ) ), aColor );
    maShapeIds.push_back( nShapeId );
    return true;
}


/** Writes the filling defined by SvtGraphicFill and returns true or it returns
    false if it can't handle this filling.
*/
bool Writer::Impl_writeFilling( SvtGraphicFill& rFilling )
{
    tools::PolyPolygon aPolyPolygon;
    rFilling.getPath( aPolyPolygon );

    Rectangle aOldRect( aPolyPolygon.GetBoundRect() );

    map( aPolyPolygon );

    Rectangle aNewRect( aPolyPolygon.GetBoundRect() );

    switch( rFilling.getFillType() )
    {
    case SvtGraphicFill::fillSolid:
        {
            Color aColor( rFilling.getFillColor() );

            if( 0.0 != rFilling.getTransparency() )
                aColor.SetTransparency( sal::static_int_cast<sal_uInt8>( MinMax( (long int)( rFilling.getTransparency() * 0xff ) , 0, 0xff ) ) );

            FillStyle aFillStyle( aColor );

            sal_uInt16 nShapeId = defineShape( aPolyPolygon, aFillStyle );
            maShapeIds.push_back( nShapeId );
        }
        break;
    case SvtGraphicFill::fillGradient:
        return false;
    case SvtGraphicFill::fillHatch:
        return false;
    case SvtGraphicFill::fillTexture:
        {
            Graphic aGraphic;
            rFilling.getGraphic( aGraphic );

            // CL->AS: Should we also scale down the quality here depending on image scale?
            sal_uInt16 nBitmapId = defineBitmap( aGraphic.GetBitmapEx(), mnJPEGCompressMode );

            ::basegfx::B2DHomMatrix aMatrix; // #i73264#

            SvtGraphicFill::Transform aTransform;

            rFilling.getTransform( aTransform );

            sal_uInt16 a,b;
            for( a = 0; a < 2; a++ )
            {
                for( b = 0; b < 3; b++ )
                {
                    aMatrix.set(a, b, aTransform.matrix[a*3+b]);
                }
            }
            aMatrix.set(2, 0, 0.0);
            aMatrix.set(2, 1, 0.0);
            aMatrix.set(2, 2, 1.0);

            // scale bitmap
            double XScale = aOldRect.GetWidth() ? (double)aNewRect.GetWidth()/aOldRect.GetWidth() : 1.0;
            double YScale = aOldRect.GetHeight() ? (double)aNewRect.GetHeight()/aOldRect.GetHeight() : 1.0;

            aMatrix.scale( XScale, YScale );

            FillStyle aFillStyle( nBitmapId, !rFilling.IsTiling(), aMatrix );

            sal_uInt16 nShapeId = defineShape( aPolyPolygon, aFillStyle );
            maShapeIds.push_back( nShapeId );
        }
        break;
    }
    return true;
}


/* CL: The idea was to export page fields as text fields that get theire
   string from a variable set with actionscript by each page. This didn't
   work out since the formatting is always wrong when text follows the
   page number field since pages greater one may require more space than
   page 1
*/
#if 0
bool Writer::Impl_writePageField( Rectangle& rTextBounds )
{
    startTag( TAG_DEFINEEDITTEXT );

    sal_uInt16 nTextId = createID();

    mpTag->addUI16( nTextId );
    mpTag->addRect( rTextBounds );

    BitStream aBits;
    aBits.writeUB( 1, 1 );                  // HasText
    aBits.writeUB( 0, 1 );                  // WordWrap
    aBits.writeUB( 0, 1 );                  // MultiLine
    aBits.writeUB( 0, 1 );                  // Password
    aBits.writeUB( 1, 1 );                  // HasTextColor
    aBits.writeUB( 0, 1 );                  // HasMaxLength
    aBits.writeUB( 0, 1 );                  // HasFont
    aBits.writeUB( 0, 1 );                  // Reserved
    aBits.writeUB( 0, 1 );                  // AutoSize
    aBits.writeUB( 0, 1 );                  // HasLayout
    aBits.writeUB( 1, 1 );                  // NoSelect
    aBits.writeUB( 1, 1 );                  // Border
    aBits.writeUB( 0, 1 );                  // Reserved
    aBits.writeUB( 0, 1 );                  // HTML
    aBits.writeUB( 0, 1 );                  // UseOutlines
    mpTag->addBits( aBits );

    Color aColor( COL_BLACK );
    mpTag->addRGB( aColor );
    mpTag->addString( "PageNumber" );
    mpTag->addString( "XXX" );

    endTag();

    maShapeIds.push_back( nTextId );

    return true;
}
#endif


void Writer::Impl_handleLineInfoPolyPolygons(const LineInfo& rInfo, const basegfx::B2DPolygon& rLinePolygon)
{
    if(rLinePolygon.count())
    {
        basegfx::B2DPolyPolygon aLinePolyPolygon(rLinePolygon);
        basegfx::B2DPolyPolygon aFillPolyPolygon;

        rInfo.applyToB2DPolyPolygon(aLinePolyPolygon, aFillPolyPolygon);

        if(aLinePolyPolygon.count())
        {
            for(sal_uInt32 a(0); a < aLinePolyPolygon.count(); a++)
            {
                const basegfx::B2DPolygon aCandidate(aLinePolyPolygon.getB2DPolygon(a));
                Impl_writePolygon( tools::Polygon(aCandidate), false );
            }
        }

        if(aFillPolyPolygon.count())
        {
            const Color aOldLineColor(mpVDev->GetLineColor());
            const Color aOldFillColor(mpVDev->GetFillColor());

            mpVDev->SetLineColor();
            mpVDev->SetFillColor(aOldLineColor);

            for(sal_uInt32 a(0); a < aFillPolyPolygon.count(); a++)
            {
                const tools::Polygon aPolygon(aFillPolyPolygon.getB2DPolygon(a));
                Impl_writePolyPolygon(tools::PolyPolygon(tools::Polygon(aPolygon)), true );
            }

            mpVDev->SetLineColor(aOldLineColor);
            mpVDev->SetFillColor(aOldFillColor);
        }
    }
}


void Writer::Impl_writeActions( const GDIMetaFile& rMtf )
{
    Rectangle clipRect;
    int bMap = 0;
    for( size_t i = 0, nCount = rMtf.GetActionSize(); i < nCount; i++ )
    {
        const MetaAction*    pAction = rMtf.GetAction( i );
        const MetaActionType nType = pAction->GetType();

        switch( nType )
        {
            case( MetaActionType::PIXEL ):
            {
                const MetaPixelAction* pA = static_cast<const MetaPixelAction*>(pAction);

                Impl_writeLine( pA->GetPoint(), pA->GetPoint(), &pA->GetColor() );
            }
            break;

            case( MetaActionType::POINT ):
            {
                const MetaPointAction* pA = static_cast<const MetaPointAction*>(pAction);

                Impl_writeLine( pA->GetPoint(), pA->GetPoint() );
            }
            break;

            case( MetaActionType::LINE ):
            {
                const MetaLineAction* pA = static_cast<const MetaLineAction*>(pAction);

                if(pA->GetLineInfo().IsDefault())
                {
                    Impl_writeLine( pA->GetStartPoint(), pA->GetEndPoint() );
                }
                else
                {
                    // LineInfo used; handle Dash/Dot and fat lines
                    basegfx::B2DPolygon aPolygon;
                    aPolygon.append(basegfx::B2DPoint(pA->GetStartPoint().X(), pA->GetStartPoint().Y()));
                    aPolygon.append(basegfx::B2DPoint(pA->GetEndPoint().X(), pA->GetEndPoint().Y()));
                    Impl_handleLineInfoPolyPolygons(pA->GetLineInfo(), aPolygon);
                }
            }
            break;

            case( MetaActionType::RECT ):
            {
                Impl_writeRect( static_cast<const MetaRectAction*>(pAction)->GetRect(), 0, 0 );
            }
            break;

            case( MetaActionType::ROUNDRECT ):
            {
                const MetaRoundRectAction* pA = static_cast<const MetaRoundRectAction*>(pAction);

                Impl_writeRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
            }
            break;

            case( MetaActionType::ELLIPSE ):
            {
                const MetaEllipseAction*    pA = static_cast<const MetaEllipseAction*>(pAction);
                const Rectangle&            rRect = pA->GetRect();

                Impl_writeEllipse( rRect.Center(), rRect.GetWidth() >> 1, rRect.GetHeight() >> 1 );
            }
            break;

            case( MetaActionType::ARC ):
            case( MetaActionType::PIE ):
            case( MetaActionType::CHORD ):
            case( MetaActionType::POLYGON ):
            {
                tools::Polygon aPoly;

                switch( nType )
                {
                    case( MetaActionType::ARC ):
                    {
                        const MetaArcAction* pA = static_cast<const MetaArcAction*>(pAction);
                        aPoly = tools::Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_ARC );
                    }
                    break;

                    case( MetaActionType::PIE ):
                    {
                        const MetaPieAction* pA = static_cast<const MetaPieAction*>(pAction);
                        aPoly = tools::Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_PIE );
                    }
                    break;

                    case( MetaActionType::CHORD ):
                    {
                        const MetaChordAction* pA = static_cast<const MetaChordAction*>(pAction);
                        aPoly = tools::Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_CHORD );
                    }
                    break;

                    case( MetaActionType::POLYGON ):
                        aPoly = static_cast<const MetaPolygonAction*>(pAction)->GetPolygon();
                    break;
                    default: break;
                }

                if( aPoly.GetSize() )
                {
                    Impl_writePolygon( aPoly, true );
                }
            }
            break;

            case( MetaActionType::POLYLINE ):
            {
                const MetaPolyLineAction* pA = static_cast<const MetaPolyLineAction*>(pAction);
                const tools::Polygon& rPoly = pA->GetPolygon();

                if( rPoly.GetSize() )
                {
                    if(pA->GetLineInfo().IsDefault())
                    {
                        Impl_writePolygon( rPoly, false );
                    }
                    else
                    {
                        // LineInfo used; handle Dash/Dot and fat lines
                        Impl_handleLineInfoPolyPolygons(pA->GetLineInfo(), rPoly.getB2DPolygon());
                    }
                }
            }
            break;

            case( MetaActionType::POLYPOLYGON ):
            {
                const MetaPolyPolygonAction*    pA = static_cast<const MetaPolyPolygonAction*>(pAction);
                const tools::PolyPolygon&              rPolyPoly = pA->GetPolyPolygon();

                if( rPolyPoly.Count() )
                    Impl_writePolyPolygon( rPolyPoly, true );
            }
            break;

            case( MetaActionType::GRADIENT ):
            {
                const MetaGradientAction*   pA = static_cast<const MetaGradientAction*>(pAction);

                tools::Polygon aPoly( pA->GetRect() );
                Impl_writeGradientEx( aPoly, pA->GetGradient() );
            }
            break;

            case( MetaActionType::GRADIENTEX ):
            {
                const MetaGradientExAction* pA = static_cast<const MetaGradientExAction*>(pAction);
                Impl_writeGradientEx( pA->GetPolyPolygon(), pA->GetGradient() );
            }
            break;

            case MetaActionType::HATCH:
            {
                const MetaHatchAction*  pA = static_cast<const MetaHatchAction*>(pAction);
                GDIMetaFile             aTmpMtf;

                mpVDev->AddHatchActions( pA->GetPolyPolygon(), pA->GetHatch(), aTmpMtf );
                Impl_writeActions( aTmpMtf );
            }
            break;

            case( MetaActionType::Transparent ):
            {
                const MetaTransparentAction*    pA = static_cast<const MetaTransparentAction*>(pAction);
                const tools::PolyPolygon&              rPolyPoly = pA->GetPolyPolygon();

                if( rPolyPoly.Count() )
                {
                    // convert transparence from percent into 0x00 - 0xff
                    sal_uInt8 nTransparence = (sal_uInt8) MinMax( FRound( pA->GetTransparence() * 2.55 ), 0, 255 );
                    Impl_writePolyPolygon( rPolyPoly, true, nTransparence );
                }
            }
            break;

            case( MetaActionType::FLOATTRANSPARENT ):
            {
                const MetaFloatTransparentAction*   pA = static_cast<const MetaFloatTransparentAction*>(pAction);
                GDIMetaFile                         aTmpMtf( pA->GetGDIMetaFile() );
                Point                               aSrcPt( aTmpMtf.GetPrefMapMode().GetOrigin() );
                const Size                          aSrcSize( aTmpMtf.GetPrefSize() );
                const Point                         aDestPt( pA->GetPoint() );
                const Size                          aDestSize( pA->GetSize() );
                const double                        fScaleX = aSrcSize.Width() ? (double) aDestSize.Width() / aSrcSize.Width() : 1.0;
                const double                        fScaleY = aSrcSize.Height() ? (double) aDestSize.Height() / aSrcSize.Height() : 1.0;
                long                                nMoveX, nMoveY;

                if( fScaleX != 1.0 || fScaleY != 1.0 )
                {
                    aTmpMtf.Scale( fScaleX, fScaleY );
                    aSrcPt.X() = FRound( aSrcPt.X() * fScaleX );
                    aSrcPt.Y() = FRound( aSrcPt.Y() * fScaleY );
                }

                nMoveX = aDestPt.X() - aSrcPt.X(), nMoveY = aDestPt.Y() - aSrcPt.Y();

                if( nMoveX || nMoveY )
                    aTmpMtf.Move( nMoveX, nMoveY );

                const Gradient& rGradient = pA->GetGradient();
                sal_uInt32 nLuminance = ((sal_Int32)rGradient.GetStartColor().GetLuminance() + (sal_Int32)rGradient.GetEndColor().GetLuminance() ) >> 1;

                sal_uInt8 nOldGlobalTransparency = mnGlobalTransparency;
                mnGlobalTransparency = (sal_uInt8)MinMax( nLuminance, 0, 0xff );

                mpVDev->Push();
                Impl_writeActions( aTmpMtf );
                mpVDev->Pop();

                mnGlobalTransparency = nOldGlobalTransparency;
            }
            break;

            case( MetaActionType::EPS ):
            {
                const MetaEPSAction*    pA = static_cast<const MetaEPSAction*>(pAction);
                const GDIMetaFile       aGDIMetaFile( pA->GetSubstitute() );
                bool                bFound = false;

                for( size_t j = 0, nC = aGDIMetaFile.GetActionSize(); ( j < nC ) && !bFound; j++ )
                {
                    const MetaAction* pSubstAct = aGDIMetaFile.GetAction( j );

                    if( pSubstAct->GetType() == MetaActionType::BMPSCALE )
                    {
                        bFound = true;
                        const MetaBmpScaleAction* pBmpScaleAction = static_cast<const MetaBmpScaleAction*>(pSubstAct);
                        Impl_writeImage( pBmpScaleAction->GetBitmap(),
                                      pA->GetPoint(), pA->GetSize(),
                                      Point(), pBmpScaleAction->GetBitmap().GetSizePixel(), clipRect, 1 == bMap  );
                    }
                }
            }
            break;

            case( MetaActionType::COMMENT ):
            {
                const MetaCommentAction*    pA = static_cast<const MetaCommentAction*>(pAction);
                const sal_uInt8*                pData = pA->GetData();

                if( pA->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_BEGIN") )
                {
                    const MetaGradientExAction* pGradAction = nullptr;
                    bool                    bDone = false;

                    while( !bDone && ( ++i < nCount ) )
                    {
                        pAction = rMtf.GetAction( i );

                        if( pAction->GetType() == MetaActionType::GRADIENTEX )
                            pGradAction = static_cast<const MetaGradientExAction*>(pAction);
                        else if( ( pAction->GetType() == MetaActionType::COMMENT ) &&
                                 ( static_cast<const MetaCommentAction*>(pAction)->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_END") ) )
                        {
                            bDone = true;
                        }
                    }

                    if( pGradAction )
                        Impl_writeGradientEx( pGradAction->GetPolyPolygon(), pGradAction->GetGradient());
                }
                else if( pA->GetComment().equalsIgnoreAsciiCase("XPATHFILL_SEQ_BEGIN") &&  pData )
                {

                    // this comment encapsulates all high level information for a filling that caused
                    // the meta actions between the "XPATHFILL_SEQ_BEGIN" and "XPATHFILL_SEQ_END" comment.

                    SvtGraphicFill  aFilling;
                    SvMemoryStream  aMemStm( const_cast<sal_uInt8 *>(pData), pA->GetDataSize(), StreamMode::READ );

                    // read the fill info
                    ReadSvtGraphicFill( aMemStm, aFilling );

                    // if impl_writeFilling can handle this high level filling, it returns true and we
                    // skip all meta actions until "XPATHFILL_SEQ_END"
                    if( Impl_writeFilling( aFilling ) )
                    {
                        bool bDone = false;

                        while( !bDone && ( ++i < nCount ) )
                        {
                            pAction = rMtf.GetAction( i );

                            if( ( pAction->GetType() == MetaActionType::COMMENT ) &&
                                     ( static_cast<const MetaCommentAction*>(pAction)->GetComment().equalsIgnoreAsciiCase("XPATHFILL_SEQ_END") ) )
                            {
                                bDone = true;
                            }
                        }
                    }
                }
                else if( pA->GetComment().equalsIgnoreAsciiCase("XPATHSTROKE_SEQ_BEGIN") && pData )
                {

                    // this comment encapsulates all high level information for a filling that caused
                    // the meta actions between the "XPATHFILL_SEQ_BEGIN" and "XPATHFILL_SEQ_END" comment.

                    SvtGraphicStroke aStroke;
                    SvMemoryStream  aMemStm( const_cast<sal_uInt8 *>(pData), pA->GetDataSize(), StreamMode::READ );

                    // read the fill info
                    ReadSvtGraphicStroke( aMemStm, aStroke );

                    // if impl_writeStroke can handle this high level stroke, it returns true and we
                    // skip all meta actions until "XPATHSTROKE_SEQ_END"
                    if( Impl_writeStroke( aStroke ) )
                    {
                        bool bDone = false;

                        while( !bDone && ( ++i < nCount ) )
                        {
                            pAction = rMtf.GetAction( i );

                            if( ( pAction->GetType() == MetaActionType::COMMENT ) &&
                                     ( static_cast<const MetaCommentAction*>(pAction)->GetComment().equalsIgnoreAsciiCase("XPATHSTROKE_SEQ_END") ) )
                            {
                                bDone = true;
                            }
                        }
                    }
                }
            }
            break;

            case( MetaActionType::BMPSCALE ):
            {
                const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pAction);

                Impl_writeImage( pA->GetBitmap(),
                          pA->GetPoint(), pA->GetSize(),
                          Point(), pA->GetBitmap().GetSizePixel(), clipRect, 1 == bMap );
            }
            break;

            case( MetaActionType::BMP ):
            {
                const MetaBmpAction* pA = static_cast<const MetaBmpAction*>(pAction);
                Impl_writeImage( pA->GetBitmap(),
                          pA->GetPoint(), mpVDev->PixelToLogic( pA->GetBitmap().GetSizePixel()),
                          Point(), pA->GetBitmap().GetSizePixel(), clipRect, 1 ==bMap );
            }
            break;

            case( MetaActionType::BMPSCALEPART ):
            {
                const MetaBmpScalePartAction* pA = static_cast<const MetaBmpScalePartAction*>(pAction);
                Impl_writeImage( pA->GetBitmap(),
                          pA->GetDestPoint(), pA->GetDestSize(),
                          pA->GetSrcPoint(), pA->GetSrcSize(), clipRect, 1 == bMap );
            }
            break;

            case( MetaActionType::BMPEX ):
            {
                const MetaBmpExAction*  pA = static_cast<const MetaBmpExAction*>(pAction);
                Impl_writeImage( pA->GetBitmapEx(),
                          pA->GetPoint(), mpVDev->PixelToLogic( pA->GetBitmapEx().GetSizePixel() ),
                          Point(), pA->GetBitmapEx().GetSizePixel(), clipRect, 1 == bMap );
            }
            break;

            case( MetaActionType::BMPEXSCALE ):
            {
                const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pAction);
                Impl_writeImage( pA->GetBitmapEx(),
                          pA->GetPoint(), pA->GetSize(),
                          Point(), pA->GetBitmapEx().GetSizePixel(), clipRect, 1 == bMap );
            }
            break;

            case( MetaActionType::BMPEXSCALEPART ):
            {
                const MetaBmpExScalePartAction* pA = static_cast<const MetaBmpExScalePartAction*>(pAction);
                Impl_writeImage( pA->GetBitmapEx(),
                          pA->GetDestPoint(), pA->GetDestSize(),
                          pA->GetSrcPoint(), pA->GetSrcSize(), clipRect, 1 == bMap );
            }
            break;

            case( MetaActionType::TEXT ):
            {
                const MetaTextAction* pA = static_cast<const MetaTextAction*>(pAction);
                Impl_writeText( pA->GetPoint(),  pA->GetText().copy( pA->GetIndex(), pA->GetLen() ), nullptr, 0);
            }
            break;

            case( MetaActionType::TEXTRECT ):
            {
                const MetaTextRectAction* pA = static_cast<const MetaTextRectAction*>(pAction);
                Impl_writeText( pA->GetRect().TopLeft(), pA->GetText(), nullptr, 0  );
            }
            break;

            case( MetaActionType::TEXTARRAY ):
            {
                const MetaTextArrayAction*  pA = static_cast<const MetaTextArrayAction*>(pAction);
                Impl_writeText( pA->GetPoint(), pA->GetText().copy( pA->GetIndex(), pA->GetLen() ), pA->GetDXArray(), 0 );
            }
            break;

            case( MetaActionType::STRETCHTEXT ):
            {
                const MetaStretchTextAction* pA = static_cast<const MetaStretchTextAction*>(pAction);
                Impl_writeText( pA->GetPoint(), pA->GetText().copy( pA->GetIndex(), pA->GetLen() ), nullptr, pA->GetWidth() );
            }
            break;

            case( MetaActionType::ISECTRECTCLIPREGION ):
            {
                const MetaISectRectClipRegionAction* pA = static_cast<const MetaISectRectClipRegionAction*>(pAction);
                clipRect = pA->GetRect();
                SAL_FALLTHROUGH;
            }
            // fall-through
            case( MetaActionType::CLIPREGION ):
            case( MetaActionType::ISECTREGIONCLIPREGION ):
            case( MetaActionType::MOVECLIPREGION ):
            {
                const_cast<MetaAction*>(pAction)->Execute( mpVDev );
            }
            break;

            case( MetaActionType::MAPMODE ):
            {
                bMap++;
                SAL_FALLTHROUGH;
            }
            // fall-through
            case( MetaActionType::REFPOINT ):
            case( MetaActionType::LINECOLOR ):
            case( MetaActionType::FILLCOLOR ):
            case( MetaActionType::TEXTLINECOLOR ):
            case( MetaActionType::TEXTFILLCOLOR ):
            case( MetaActionType::TEXTCOLOR ):
            case( MetaActionType::TEXTALIGN ):
            case( MetaActionType::FONT ):
            case( MetaActionType::PUSH ):
            case( MetaActionType::POP ):
            case( MetaActionType::LAYOUTMODE ):
            {
                const_cast<MetaAction*>(pAction)->Execute( mpVDev );
            }
            break;

            case( MetaActionType::RASTEROP ):
            case( MetaActionType::MASK ):
            case( MetaActionType::MASKSCALE ):
            case( MetaActionType::MASKSCALEPART ):
            case( MetaActionType::WALLPAPER ):
            case( MetaActionType::TEXTLINE ):
            {
                // !!! >>> we don't want to support these actions
            }
            break;

            default:
            break;
        }
    }
}


void Writer::Impl_addStraightLine( BitStream& rBits, Point& rLastPoint,
                                   const double P2x, const double P2y )
{
    Point aPoint( FRound(P2x), FRound(P2y) );

    Impl_addStraightEdgeRecord( rBits, _Int16(aPoint.X() - rLastPoint.X()),_Int16(aPoint.Y() - rLastPoint.Y()));
    rLastPoint = aPoint;

}


void Writer::Impl_addQuadBezier( BitStream& rBits, Point& rLastPoint,
                                 const double P2x, const double P2y,
                                 const double P3x, const double P3y )
{

    Point aControlPoint( FRound(P2x), FRound(P2y) );
    Point aAnchorPoint( FRound(P3x), FRound(P3y) );

    Impl_addCurvedEdgeRecord( rBits,
                                _Int16(aControlPoint.X() - rLastPoint.X()),_Int16(aControlPoint.Y() - rLastPoint.Y()),
                                _Int16(aAnchorPoint.X() - aControlPoint.X()),_Int16(aAnchorPoint.Y() - aControlPoint.Y()) );
    rLastPoint = aAnchorPoint;
}


/* Approximate given cubic bezier curve by quadratic bezier segments */
void Writer::Impl_quadBezierApprox( BitStream& rBits,
                                   Point& rLastPoint,
                                   const double d2,
                                   const double P1x, const double P1y,
                                   const double P2x, const double P2y,
                                   const double P3x, const double P3y,
                                   const double P4x, const double P4y )
{
    // Check for degenerate case, where the given cubic bezier curve
    // is already quadratic: P4 == 3P3 - 3P2 + P1
    if( P4x == 3.0*P3x - 3.0*P2x + P1x &&
        P4y == 3.0*P3y - 3.0*P2y + P1y )
    {
        Impl_addQuadBezier( rBits, rLastPoint,
                           3.0/2.0*P2x - 1.0/2.0*P1x, 3.0/2.0*P2y - 1.0/2.0*P1y,
                           P4x, P4y);
    }
    else
    {
        // Create quadratic segment for given cubic:
        // Start and end point must coincide, determine quadratic control
        // point in such a way that it lies on the intersection of the
        // tangents at start and end point, resp. Thus, both cubic and
        // quadratic curve segments will match in 0th and 1st derivative
        // at the start and end points

        // Intersection of P2P1 and P4P3
        //           (P2y-P4y)(P3x-P4x)-(P2x-P4x)(P3y-P4y)
        //  lambda = -------------------------------------
        //           (P1x-P2x)(P3y-P4y)-(P1y-P2y)(P3x-P4x)
        //
        // Intersection point IP is now
        // IP = P2 + lambda(P1-P2)
        //
        const double nominator( (P2y-P4y)*(P3x-P4x) - (P2x-P4x)*(P3y-P4y) );
        const double denominator( (P1x-P2x)*(P3y-P4y) - (P1y-P2y)*(P3x-P4x) );
        const double lambda( nominator / denominator );

        const double IPx( P2x + lambda*( P1x - P2x) );
        const double IPy( P2y + lambda*( P1y - P2y) );

        // Introduce some alias names: quadratic start point is P1, end
        // point is P4, control point is IP
        const double QP1x( P1x );
        const double QP1y( P1y );
        const double QP2x( IPx );
        const double QP2y( IPy );
        const double QP3x( P4x );
        const double QP3y( P4y );

        // Adapted bezier flatness test (lecture notes from R. Schaback,
        // Mathematics of Computer-Aided Design, Uni Goettingen, 2000)
        //
        // ||C(t) - Q(t)|| <= max     ||c_j - q_j||
        //                    0<=j<=n
        //
        // In this case, we don't need the distance from the cubic bezier
        // to a straight line, but to a quadratic bezier. The c_j's are
        // the cubic bezier's bernstein coefficients, the q_j's the
        // quadratic bezier's. We have the c_j's given, the q_j's can be
        // calculated from QPi like this (sorry, mixed index notation, we
        // use [1,n], formulas use [0,n-1]):
        //
        // q_0 = QP1 = P1
        // q_1 = 1/3 QP1 + 2/3 QP2
        // q_2 = 2/3 QP2 + 1/3 QP3
        // q_3 = QP3 = P4
        //
        // We can drop case 0 and 3, since there the curves coincide
        // (distance is zero)

        // calculate argument of max for j=1 and j=2
        const double fJ1x( P2x - 1.0/3.0*QP1x - 2.0/3.0*QP2x );
        const double fJ1y( P2y - 1.0/3.0*QP1y - 2.0/3.0*QP2y );
        const double fJ2x( P3x - 2.0/3.0*QP2x - 1.0/3.0*QP3x );
        const double fJ2y( P3y - 2.0/3.0*QP2y - 1.0/3.0*QP3y );

        // stop if distance from cubic curve is guaranteed to be bounded by d
        // Should denominator be 0: then P1P2 and P3P4 are parallel (P1P2^T R[90,P3P4] = 0.0),
        // meaning that either we have a straight line or an inflexion point (see else block below)
        if( 0.0 != denominator &&
            ::std::max( fJ1x*fJ1x + fJ1y*fJ1y,
                        fJ2x*fJ2x + fJ2y*fJ2y) < d2 )
        {
            // requested resolution reached.
            // Add end points to output file.
            // order is preserved, since this is so to say depth first traversal.
            Impl_addQuadBezier( rBits, rLastPoint,
                                QP2x, QP2y,
                                QP3x, QP3y);
        }
        else
        {
            // Maybe subdivide further

            // This is for robustness reasons, since the line intersection
            // method below gets instable if the curve gets closer to a
            // straight line. If the given cubic bezier does not deviate by
            // more than d/4 from a straight line, either:
            //  - take the line (that's what we do here)
            //  - express the line by a quadratic bezier

            // Perform bezier flatness test (lecture notes from R. Schaback,
            // Mathematics of Computer-Aided Design, Uni Goettingen, 2000)
            //
            // ||P(t) - L(t)|| <= max     ||b_j - b_0 - j/n(b_n - b_0)||
            //                    0<=j<=n
            //
            // What is calculated here is an upper bound to the distance from
            // a line through b_0 and b_3 (P1 and P4 in our notation) and the
            // curve. We can drop 0 and n from the running indices, since the
            // argument of max becomes zero for those cases.
            const double fJ1x2( P2x - P1x - 1.0/3.0*(P4x - P1x) );
            const double fJ1y2( P2y - P1y - 1.0/3.0*(P4y - P1y) );
            const double fJ2x2( P3x - P1x - 2.0/3.0*(P4x - P1x) );
            const double fJ2y2( P3y - P1y - 2.0/3.0*(P4y - P1y) );

            // stop if distance from line is guaranteed to be bounded by d/4
            if( ::std::max( fJ1x2*fJ1x2 + fJ1y2*fJ1y2,
                            fJ2x2*fJ2x2 + fJ2y2*fJ2y2) < d2/16.0 )
            {
                // do not subdivide further, add straight line instead
                Impl_addStraightLine( rBits, rLastPoint, P4x, P4y);
            }
            else
            {
                // deCasteljau bezier arc, split at t=0.5
                // Foley/vanDam, p. 508
                const double L1x( P1x ),             L1y( P1y );
                const double L2x( (P1x + P2x)*0.5 ), L2y( (P1y + P2y)*0.5 );
                const double Hx ( (P2x + P3x)*0.5 ), Hy ( (P2y + P3y)*0.5 );
                const double L3x( (L2x + Hx)*0.5 ),  L3y( (L2y + Hy)*0.5 );
                const double R4x( P4x ),             R4y( P4y );
                const double R3x( (P3x + P4x)*0.5 ), R3y( (P3y + P4y)*0.5 );
                const double R2x( (Hx + R3x)*0.5 ),  R2y( (Hy + R3y)*0.5 );
                const double R1x( (L3x + R2x)*0.5 ), R1y( (L3y + R2y)*0.5 );
                const double L4x( R1x ),             L4y( R1y );

                // subdivide further
                Impl_quadBezierApprox(rBits, rLastPoint, d2, L1x, L1y, L2x, L2y, L3x, L3y, L4x, L4y);
                Impl_quadBezierApprox(rBits, rLastPoint, d2, R1x, R1y, R2x, R2y, R3x, R3y, R4x, R4y);
            }
        }
    }
}

Reference < XBreakIterator > Writer::Impl_GetBreakIterator()
{
    if ( !mxBreakIterator.is() )
    {
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        mxBreakIterator = BreakIterator::create(xContext);
    }
    return mxBreakIterator;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
