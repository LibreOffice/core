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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "svgfontexport.hxx"
#include "svgwriter.hxx"

// -----------
// - statics -
// -----------

static const char   aXMLElemG[] = "g";
static const char   aXMLElemDefs[] = "defs";
static const char   aXMLElemClipPath[] = "clipPath";
static const char   aXMLElemMask[] = "mask";
static const char   aXMLElemPattern[] = "pattern";
static const char   aXMLElemLinearGradient[] = "linearGradient";
static const char   aXMLElemStop[] = "stop";
static const char   aXMLElemLine[] = "line";
static const char   aXMLElemRect[] = "rect";
static const char   aXMLElemEllipse[] = "ellipse";
static const char   aXMLElemPath[] = "path";
static const char   aXMLElemPolygon[] = "polygon";
static const char   aXMLElemPolyLine[] = "polyline";
static const char   aXMLElemText[] = "text";
static const char   aXMLElemTSpan[] = "tspan";
static const char   aXMLElemImage[] = "image";

static const char   aXMLAttrTransform[] = "transform";
static const char   aXMLAttrStyle[] = "style";
static const char   aXMLAttrId[] = "id";
static const char   aXMLAttrD[] = "d";
static const char   aXMLAttrX[] = "x";
static const char   aXMLAttrY[] = "y";
static const char   aXMLAttrX1[] = "x1";
static const char   aXMLAttrY1[] = "y1";
static const char   aXMLAttrX2[] = "x2";
static const char   aXMLAttrY2[] = "y2";
static const char   aXMLAttrCX[] = "cx";
static const char   aXMLAttrCY[] = "cy";
static const char   aXMLAttrRX[] = "rx";
static const char   aXMLAttrRY[] = "ry";
static const char   aXMLAttrWidth[] = "width";
static const char   aXMLAttrHeight[] = "height";
static const char   aXMLAttrPoints[] = "points";
static const char   aXMLAttrPatternUnits[] = "patternUnits";
static const char   aXMLAttrGradientUnits[] = "gradientUnits";
static const char   aXMLAttrOffset[] = "offset";
static const char   aXMLAttrXLinkHRef[] = "xlink:href";

// --------------
// - FastString -
// --------------

FastString::FastString( sal_uInt32 nInitLen, sal_uInt32 nIncrement ) :
    mnBufLen( nInitLen ),
    mnCurLen( 0 ),
    mnBufInc( nIncrement ),
    mpBuffer( new sal_Unicode[ nInitLen * sizeof( sal_Unicode ) ] ),
    mnPartPos( 0 )
{
    DBG_ASSERT( nInitLen, "invalid initial length" );
    DBG_ASSERT( nIncrement, "invalid increment" );
}

// -----------------------------------------------------------------------------

FastString::~FastString()
{
    delete[] mpBuffer;
}

// -----------------------------------------------------------------------------

FastString& FastString::operator+=( const NMSP_RTL::OUString& rStr )
{
    if( rStr.getLength() )
    {
        if( ( mnCurLen + rStr.getLength() ) > mnBufLen )
        {
            const sal_uInt32    nNewBufLen = ( mnBufLen + ( ( ( mnCurLen + rStr.getLength() ) - mnBufLen ) / mnBufInc + 1 ) * mnBufInc );
            sal_Unicode*        pNewBuffer = new sal_Unicode[ nNewBufLen * sizeof( sal_Unicode ) ];

            memcpy( pNewBuffer, mpBuffer, mnBufLen * sizeof( sal_Unicode )  );
            delete[] mpBuffer;
            mpBuffer = pNewBuffer;
            mnBufLen = nNewBufLen;
        }

        memcpy( mpBuffer + mnCurLen, rStr.getStr(), rStr.getLength() * sizeof( sal_Unicode ) );
        mnCurLen += rStr.getLength();

        if( maString.getLength() )
            maString = NMSP_RTL::OUString();
    }

    return *this;
}

// -----------------------------------------------------------------------------

const NMSP_RTL::OUString& FastString::GetString() const
{
    if( !maString.getLength() && mnCurLen )
        ( (FastString*) this )->maString = NMSP_RTL::OUString( mpBuffer, mnCurLen );

    return maString;
}

// ----------------------
// - SVGAttributeWriter -
// ----------------------

SVGAttributeWriter::SVGAttributeWriter( SvXMLExport& rExport, SVGFontExport& rFontExport ) :
    mrExport( rExport ),
    mrFontExport( rFontExport ),
    mpElemFont( NULL ),
    mpElemPaint( NULL )
{
}

// -----------------------------------------------------------------------------

SVGAttributeWriter::~SVGAttributeWriter()
{
    delete mpElemPaint;
    delete mpElemFont;
}

// -----------------------------------------------------------------------------

NMSP_RTL::OUString SVGAttributeWriter::GetFontStyle( const Font& rFont )
{
    FastString aStyle;

    // font family
    aStyle += B2UCONST( "font-family:" );
    aStyle += mrFontExport.GetMappedFontName( rFont.GetName() );

    // font size
    aStyle += B2UCONST( ";" );
    aStyle += B2UCONST( "font-size:" );
    aStyle += SVGActionWriter::GetValueString( rFont.GetHeight() );
    aStyle += B2UCONST( "px" );

    // font style
/*
    if( rFont.GetItalic() != ITALIC_NONE )
    {
        aStyle += B2UCONST( ";" );
        aStyle += B2UCONST( "font-style:" );

        if( rFont.GetItalic() == ITALIC_OBLIQUE )
            aStyle += B2UCONST( "oblique" );
        else
            aStyle += B2UCONST( "italic" );
    }
*/

    // font weight
    sal_Int32 nFontWeight;

    switch( rFont.GetWeight() )
    {
        case WEIGHT_THIN:           nFontWeight = 100; break;
        case WEIGHT_ULTRALIGHT:     nFontWeight = 200; break;
        case WEIGHT_LIGHT:          nFontWeight = 300; break;
        case WEIGHT_SEMILIGHT:      nFontWeight = 400; break;
        case WEIGHT_NORMAL:         nFontWeight = 400; break;
        case WEIGHT_MEDIUM:         nFontWeight = 500; break;
        case WEIGHT_SEMIBOLD:       nFontWeight = 600; break;
        case WEIGHT_BOLD:           nFontWeight = 700; break;
        case WEIGHT_ULTRABOLD:      nFontWeight = 800; break;
        case WEIGHT_BLACK:          nFontWeight = 900; break;
        default:                    nFontWeight = 400; break;
    }

    aStyle += B2UCONST( ";" );
    aStyle += B2UCONST( "font-weight:" );
    aStyle += NMSP_RTL::OUString::valueOf( nFontWeight );

    // !!!
    // font-variant
    // font-stretch
    // font-size-adjust

#ifdef _SVG_USE_NATIVE_TEXTDECORATION

    if( rFont.GetUnderline() != UNDERLINE_NONE || rFont.GetStrikeout() != STRIKEOUT_NONE )
    {
        aStyle += B2UCONST( ";" );
        aStyle += B2UCONST( "text-decoration:" );

        if( rFont.GetUnderline() != UNDERLINE_NONE )
            aStyle += B2UCONST( " underline" );

        if( rFont.GetStrikeout() != STRIKEOUT_NONE )
             aStyle += B2UCONST( " line-through" );
    }

#endif // _SVG_USE_NATIVE_TEXTDECORATION

    return aStyle.GetString();
}

// -----------------------------------------------------------------------------

NMSP_RTL::OUString SVGAttributeWriter::GetColorStyle( const Color& rColor )
{
    FastString aStyle;
    aStyle += B2UCONST( "rgb(" );
    aStyle += NMSP_RTL::OUString::valueOf( (sal_Int32) rColor.GetRed() );
    aStyle += B2UCONST( "," );
    aStyle += NMSP_RTL::OUString::valueOf( (sal_Int32) rColor.GetGreen() );
    aStyle += B2UCONST( "," );
    aStyle += NMSP_RTL::OUString::valueOf( (sal_Int32) rColor.GetBlue() );
    aStyle += B2UCONST( ")" );
    return aStyle.GetString();
}

// -----------------------------------------------------------------------------

NMSP_RTL::OUString SVGAttributeWriter::GetPaintStyle( const Color& rLineColor, const Color& rFillColor, const LineInfo* pLineInfo )
{
    FastString aStyle;

    // line color
    aStyle += B2UCONST( "stroke:" );

    if( rLineColor.GetTransparency() == 255 )
        aStyle += B2UCONST( "none" );
    else
    {
        // line color value in rgb
        aStyle += GetColorStyle( rLineColor );

        // line color opacity in percent if neccessary
        if( rLineColor.GetTransparency() )
        {
            aStyle += B2UCONST( ";" );
            aStyle += B2UCONST( "stroke-opacity:" );
            aStyle += NMSP_RTL::OUString::valueOf( ( 255 - (double) rLineColor.GetTransparency() ) / 255.0 );
        }

        if(pLineInfo)
        {
            // more infos for line needed
            if(pLineInfo->GetWidth() > 1)
            {
                aStyle += B2UCONST( ";" );
                aStyle += B2UCONST( "stroke-width:" );
                aStyle += NMSP_RTL::OUString::valueOf(pLineInfo->GetWidth());
            }

            if(LINE_DASH == pLineInfo->GetStyle())
            {
                aStyle += B2UCONST( ";" );
                aStyle += B2UCONST( "stroke-dasharray:" );
                const long nDashLen(pLineInfo->GetDashLen());
                const long nDotLen(pLineInfo->GetDotLen());
                const long nDistance(pLineInfo->GetDistance());
                bool bIsFirst(true);

                for(sal_uInt16 a(0); a < pLineInfo->GetDashCount(); a++)
                {
                    if(bIsFirst)
                        aStyle += B2UCONST(" "), bIsFirst = false;
                    else
                        aStyle += B2UCONST(",");
                    aStyle += NMSP_RTL::OUString::valueOf(nDashLen);
                    aStyle += B2UCONST(",");
                    aStyle += NMSP_RTL::OUString::valueOf(nDistance);
                }

                for(sal_uInt16 b(0); b < pLineInfo->GetDotCount(); b++)
                {
                    if(bIsFirst)
                        aStyle += B2UCONST(" "), bIsFirst = false;
                    else
                        aStyle += B2UCONST(",");
                    aStyle += NMSP_RTL::OUString::valueOf(nDotLen);
                    aStyle += B2UCONST(",");
                    aStyle += NMSP_RTL::OUString::valueOf(nDistance);
                }
            }

            if(basegfx::B2DLINEJOIN_MITER != pLineInfo->GetLineJoin())
            {
                aStyle += B2UCONST( ";" );
                aStyle += B2UCONST( "stroke-linejoin:" );

                switch(pLineInfo->GetLineJoin())
                {
                    default: // B2DLINEJOIN_NONE, B2DLINEJOIN_MIDDLE, B2DLINEJOIN_MITER
                        aStyle += B2UCONST( "miter" );
                        break;
                    case basegfx::B2DLINEJOIN_ROUND:
                        aStyle += B2UCONST( "round" );
                        break;
                    case basegfx::B2DLINEJOIN_BEVEL:
                        aStyle += B2UCONST( "bevel" );
                        break;
                }
            }
        }
    }

    // fill color
    aStyle += B2UCONST( ";" );
    aStyle += B2UCONST( "fill:" );

    if( rFillColor.GetTransparency() == 255 )
        aStyle += B2UCONST( "none" );
    else
    {
        // fill color value in rgb
        aStyle += GetColorStyle( rFillColor );

        // fill color opacity in percent if neccessary
        if( rFillColor.GetTransparency() )
        {
            aStyle += B2UCONST( ";" );
            aStyle += B2UCONST( "fill-opacity:" );
            aStyle += NMSP_RTL::OUString::valueOf( ( 255 - (double) rFillColor.GetTransparency() ) / 255.0 );
        }
    }

    return aStyle.GetString();
}

// -----------------------------------------------------------------------------

void SVGAttributeWriter::SetFontAttr( const Font& rFont )
{
    if( !mpElemFont || ( rFont != maCurFont ) )
    {
        delete mpElemPaint, mpElemPaint = NULL;
        delete mpElemFont;
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, GetFontStyle( maCurFont = rFont ) );
        mpElemFont = new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );
    }
}

// -----------------------------------------------------------------------------

void SVGAttributeWriter::SetPaintAttr( const Color& rLineColor, const Color& rFillColor, const LineInfo* pLineInfo )
{
    if( !mpElemPaint || ( rLineColor != maCurLineColor ) || ( rFillColor != maCurFillColor ) )
    {
        delete mpElemPaint;
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, GetPaintStyle( maCurLineColor = rLineColor, maCurFillColor = rFillColor, pLineInfo ) );
        mpElemPaint = new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );
    }
}

// -------------------
// - SVGActionWriter -
// -------------------

SVGActionWriter::SVGActionWriter( SvXMLExport& rExport, SVGFontExport& rFontExport ) :
    mrExport( rExport ),
    mrFontExport( rFontExport ),
    mpContext( NULL ),
    mbClipAttrChanged( sal_False ),
    mnCurClipId( 1 ),
    mnCurPatternId( 1 ),
    mnCurGradientId( 1 ),
    mnCurMaskId( 1 )
{
    mpVDev = new VirtualDevice;
    mpVDev->EnableOutput( sal_False );
    maTargetMapMode = MAP_100TH_MM;
}

// -----------------------------------------------------------------------------

SVGActionWriter::~SVGActionWriter()
{
    DBG_ASSERT( !mpContext, "Not all contexts are closed" );
    delete mpVDev;
}

// -----------------------------------------------------------------------------

long SVGActionWriter::ImplMap( sal_Int32 nVal ) const
{
    return ImplMap( Size( nVal, nVal ) ).Width();
}

// -----------------------------------------------------------------------------

Point SVGActionWriter::ImplMap( const Point& rPt ) const
{
    return mpVDev->LogicToLogic( rPt, mpVDev->GetMapMode(), maTargetMapMode );
}

// -----------------------------------------------------------------------------

Size SVGActionWriter::ImplMap( const Size& rSz ) const
{
    return mpVDev->LogicToLogic( rSz, mpVDev->GetMapMode(), maTargetMapMode );
}

// -----------------------------------------------------------------------------

LineInfo SVGActionWriter::ImplMap( const LineInfo& rLineInfo ) const
{
    LineInfo aInfo(rLineInfo);
    long aTemp(0);

    if(aInfo.GetStyle() == LINE_DASH)
    {
        if(aInfo.GetDotCount() && aInfo.GetDotLen())
        {
            aTemp = aInfo.GetDotLen();
            mpVDev->LogicToLogic(&aTemp, 1, &mpVDev->GetMapMode(), &maTargetMapMode);
            aInfo.SetDotLen(Max(aTemp, 1L));
        }
        else
            aInfo.SetDotCount(0);

        if(aInfo.GetDashCount() && aInfo.GetDashLen())
        {
            aTemp = aInfo.GetDashLen();
            mpVDev->LogicToLogic(&aTemp, 1, &mpVDev->GetMapMode(), &maTargetMapMode);
            aInfo.SetDashLen(Max(aTemp, 1L));
        }
        else
            aInfo.SetDashCount(0);

        aTemp = aInfo.GetDistance();
        mpVDev->LogicToLogic(&aTemp, 1, &mpVDev->GetMapMode(), &maTargetMapMode);
        aInfo.SetDistance(aTemp);

        if((!aInfo.GetDashCount() && !aInfo.GetDotCount()) || !aInfo.GetDistance())
            aInfo.SetStyle(LINE_SOLID);
    }

    aTemp = aInfo.GetWidth();
    mpVDev->LogicToLogic(&aTemp, 1, &mpVDev->GetMapMode(), &maTargetMapMode);
    aInfo.SetWidth(aTemp);

    return aInfo;
}

// -----------------------------------------------------------------------------

NMSP_RTL::OUString SVGActionWriter::GetValueString( sal_Int32 nVal )
{
    return ::rtl::OUString::valueOf( nVal );
}

// -----------------------------------------------------------------------------

NMSP_RTL::OUString  SVGActionWriter::GetPathString( const PolyPolygon& rPolyPoly, sal_Bool bLine )
{
    FastString                  aPathData;
    const NMSP_RTL::OUString    aBlank( B2UCONST( " " ) );
    const NMSP_RTL::OUString    aComma( B2UCONST( "," ) );
    Point                       aPolyPoint;

    for( long i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
    {
        const Polygon&  rPoly = rPolyPoly[ (USHORT) i ];
        USHORT          n = 1, nSize = rPoly.GetSize();

        if( nSize > 1 )
        {
            aPathData += B2UCONST( "M " );
            aPathData += GetValueString( ( aPolyPoint = rPoly[ 0 ] ).X() );
            aPathData += aComma;
            aPathData += GetValueString( aPolyPoint.Y() );
            sal_Char nCurrentMode = 0;

            while( n < nSize )
            {
                aPathData += aBlank;
                if ( ( rPoly.GetFlags( n ) == POLY_CONTROL ) && ( ( n + 2 ) < nSize ) )
                {
                    if ( nCurrentMode != 'C' )
                    {
                        nCurrentMode = 'C';
                        aPathData += B2UCONST( "C " );
                    }
                    for ( int j = 0; j < 3; j++ )
                    {
                        if ( j )
                            aPathData += aBlank;
                        aPathData += GetValueString( ( aPolyPoint = rPoly[ n++ ] ).X() );
                        aPathData += aComma;
                        aPathData += GetValueString( aPolyPoint.Y() );
                    }
                }
                else
                {
                    if ( nCurrentMode != 'L' )
                    {
                        nCurrentMode = 'L';
                        aPathData += B2UCONST( "L " );
                    }
                    aPathData += GetValueString( ( aPolyPoint = rPoly[ n++ ] ).X() );
                    aPathData += aComma;
                    aPathData += GetValueString( aPolyPoint.Y() );
                }
            }

            if( !bLine )
                aPathData += B2UCONST( " Z" );

            if( i < ( nCount - 1 ) )
                aPathData += aBlank;
        }
    }

    return aPathData.GetString();
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteLine( const Point& rPt1, const Point& rPt2, const Color* pLineColor,
                                     const NMSP_RTL::OUString* pStyle )
{
    const Point aPt1( ImplMap( rPt1 ) );
    const Point aPt2( ImplMap( rPt2 ) );

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX1, GetValueString( aPt1.X() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY1, GetValueString( aPt1.Y() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX2, GetValueString( aPt2.X() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY2, GetValueString( aPt2.Y() ) );

    // add additional style if requested
    if( pStyle )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, *pStyle );

    if( pLineColor )
    {
        // !!! mrExport.AddAttribute( XML_NAMESPACE_NONE, ... )
        DBG_ERROR( "SVGActionWriter::ImplWriteLine: Line color not implemented" );
    }

    {
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemLine, TRUE, TRUE );
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteRect( const Rectangle& rRect, long nRadX, long nRadY,
                                     const NMSP_RTL::OUString* pStyle )
{
    const Rectangle aRect( ImplMap( rRect ) );

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, GetValueString( aRect.Left() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, GetValueString( aRect.Top() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, GetValueString( aRect.GetWidth() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, GetValueString( aRect.GetHeight() ) );

    if( nRadX )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRX, GetValueString( ImplMap( nRadX ) ) );

    if( nRadY )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRY, GetValueString( ImplMap( nRadY ) ) );

    // add additional style if requested
    if( pStyle )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, *pStyle );

    {
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemRect, TRUE, TRUE );
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteEllipse( const Point& rCenter, long nRadX, long nRadY,
                                        const NMSP_RTL::OUString* pStyle )
{
    const Point aCenter( ImplMap( rCenter ) );

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCX, GetValueString( aCenter.X() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCY, GetValueString( aCenter.Y() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRX, GetValueString( ImplMap( nRadX ) ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRY, GetValueString( ImplMap( nRadY ) ) );

    // add additional style if requested
    if( pStyle )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, *pStyle );

    {
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemEllipse, TRUE, TRUE );
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWritePolyPolygon( const PolyPolygon& rPolyPoly, sal_Bool bLineOnly,
                                            const NMSP_RTL::OUString* pStyle )
{
    if( rPolyPoly.Count() )
    {
        PolyPolygon aMappedPolyPoly;
        FastString  aStyle;

        for( USHORT i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
        {
            const Polygon&  rPoly = rPolyPoly[ i ];
            const USHORT    nSize = rPoly.GetSize();

            // #i102224# congratulations, this throws away the curve flags
            // and makes ANY curved polygon look bad. The Flags HAVE to be
            // copied, too. It's NOT enough to copy the mapped points. Just
            // copy the original polygon completely and REPLACE the points

            // old: Polygon         aMappedPoly( nSize );
            // new:
            Polygon aMappedPoly(rPoly);

            for( USHORT n = 0; n < nSize; n++ )
                aMappedPoly[ n ] = ImplMap( rPoly[ n ] );

            aMappedPolyPoly.Insert( aMappedPoly );
        }

        if( bLineOnly )
        {
            aStyle += B2UCONST( "fill:none" );
            if( pStyle )
                aStyle += B2UCONST( ";" );
        }
        if( pStyle )
            aStyle += *pStyle;

        // add style attribute
        if( aStyle.GetLength() )
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aStyle.GetString() );

        // add path data attribute
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrD, GetPathString( aMappedPolyPoly, bLineOnly ) );
        {
            // write polyline/polygon element
            SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemPath, TRUE, TRUE );
        }
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWritePattern( const PolyPolygon& rPolyPoly,
                                        const Hatch* pHatch,
                                        const Gradient* pGradient,
                                        const NMSP_RTL::OUString* pStyle,
                                        sal_uInt32 nWriteFlags )
{
    if( rPolyPoly.Count() )
    {
        SvXMLElementExport aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );

        FastString aPatternId;
        aPatternId += B2UCONST( "pattern" );
        aPatternId += GetValueString( ImplGetNextPatternId() );

        {
            SvXMLElementExport aElemDefs( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, TRUE, TRUE );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, aPatternId.GetString() );

            Rectangle aRect( ImplMap( rPolyPoly.GetBoundRect() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, GetValueString( aRect.Left() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, GetValueString( aRect.Top() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, GetValueString( aRect.GetWidth() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, GetValueString( aRect.GetHeight() ) );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrPatternUnits, NMSP_RTL::OUString( RTL_CONSTASCII_USTRINGPARAM( "userSpaceOnUse") ) );

            {
                SvXMLElementExport aElemPattern( mrExport, XML_NAMESPACE_NONE, aXMLElemPattern, TRUE, TRUE );

                // The origin of a pattern is positioned at (aRect.Left(), aRect.Top()).
                // So we need to adjust the pattern coordinate.
                FastString aTransform;
                aTransform += B2UCONST( "translate" );
                aTransform += B2UCONST( "(" );
                aTransform += GetValueString( -aRect.Left() );
                aTransform += B2UCONST( "," );
                aTransform += GetValueString( -aRect.Top() );
                aTransform += B2UCONST( ")" );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTransform, aTransform.GetString() );

                {
                    SvXMLElementExport aElemG2( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );

                    GDIMetaFile aTmpMtf;
                    if( pHatch )
                        mpVDev->AddHatchActions( rPolyPoly, *pHatch, aTmpMtf );
                    else if ( pGradient )
                        mpVDev->AddGradientActions( rPolyPoly.GetBoundRect(), *pGradient, aTmpMtf );
                    ImplWriteActions( aTmpMtf, pStyle, nWriteFlags );
                }
            }
        }

        FastString aPatternStyle;
        aPatternStyle += B2UCONST( "fill:url(#" );
        aPatternStyle += aPatternId.GetString();
        aPatternStyle += B2UCONST( ")" );

        {
            ImplWritePolyPolygon( rPolyPoly, sal_False, &aPatternStyle.GetString() );
        }
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteGradientEx( const PolyPolygon& rPolyPoly, const Gradient& rGradient,
                                           const NMSP_RTL::OUString* pStyle, sal_uInt32 nWriteFlags )
{
    if ( rGradient.GetStyle() == GRADIENT_LINEAR ||
         rGradient.GetStyle() == GRADIENT_AXIAL )
    {
        ImplWriteGradientLinear( rPolyPoly, rGradient );
    }
    else
    {
        ImplWritePattern( rPolyPoly, NULL, &rGradient, pStyle, nWriteFlags );
    }
}

void SVGActionWriter::ImplWriteGradientLinear( const PolyPolygon& rPolyPoly,
                                               const Gradient& rGradient )
{
    if( rPolyPoly.Count() )
    {
        SvXMLElementExport aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );

        FastString aGradientId;
        aGradientId += B2UCONST( "gradient" );
        aGradientId += GetValueString( ImplGetNextGradientId() );

        {
            SvXMLElementExport aElemDefs( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, TRUE, TRUE );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, aGradientId.GetString() );
            {
                Rectangle aTmpRect;
                Point aTmpCenter;
                rGradient.GetBoundRect( rPolyPoly.GetBoundRect(), aTmpRect, aTmpCenter );
                const Rectangle aRect( ImplMap( aTmpRect) );
                const Point aCenter( ImplMap( aTmpCenter) );
                const sal_uInt16 nAngle = rGradient.GetAngle() % 3600;

                Polygon aPoly( 2 );
                // Setting x value of a gradient vector to rotation center to
                // place a gradient vector in a target polygon.
                // This would help editing it in SVG editors like inkscape.
                aPoly[ 0 ].X() = aPoly[ 1 ].X() = aCenter.X();
                aPoly[ 0 ].Y() = aRect.Top();
                aPoly[ 1 ].Y() = aRect.Bottom();
                aPoly.Rotate( aCenter, nAngle );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX1, GetValueString( aPoly[ 0 ].X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY1, GetValueString( aPoly[ 0 ].Y() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX2, GetValueString( aPoly[ 1 ].X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY2, GetValueString( aPoly[ 1 ].Y() ) );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrGradientUnits,
                                       NMSP_RTL::OUString( RTL_CONSTASCII_USTRINGPARAM( "userSpaceOnUse" ) ) );
            }

            {
                SvXMLElementExport aElemLinearGradient( mrExport, XML_NAMESPACE_NONE, aXMLElemLinearGradient, TRUE, TRUE );

                const Color aStartColor = ImplGetColorWithIntensity( rGradient.GetStartColor(), rGradient.GetStartIntensity() );
                const Color aEndColor = ImplGetColorWithIntensity( rGradient.GetEndColor(), rGradient.GetEndIntensity() );
                double fBorderOffset = rGradient.GetBorder() / 100.0;
                const sal_uInt16 nSteps = rGradient.GetSteps();
                if( rGradient.GetStyle() == GRADIENT_LINEAR )
                {
                    // Emulate non-smooth gradient
                    if( 0 < nSteps && nSteps < 100 )
                    {
                        double fOffsetStep = ( 1.0 - fBorderOffset ) / (double)nSteps;
                        for( sal_uInt16 i = 0; i < nSteps; i++ ) {
                            Color aColor = ImplGetGradientColor( aStartColor, aEndColor, i / (double) nSteps );
                            ImplWriteGradientStop( aColor, fBorderOffset + ( i + 1 ) * fOffsetStep );
                            aColor = ImplGetGradientColor( aStartColor, aEndColor, ( i + 1 ) / (double) nSteps );
                            ImplWriteGradientStop( aColor, fBorderOffset + ( i + 1 ) * fOffsetStep );
                        }
                    }
                    else
                    {
                        ImplWriteGradientStop( aStartColor, fBorderOffset );
                        ImplWriteGradientStop( aEndColor, 1.0 );
                    }
                }
                else
                {
                    fBorderOffset /= 2;
                    // Emulate non-smooth gradient
                    if( 0 < nSteps && nSteps < 100 )
                    {
                        double fOffsetStep = ( 0.5 - fBorderOffset ) / (double)nSteps;
                        // Upper half
                        for( sal_uInt16 i = 0; i < nSteps; i++ )
                        {
                            Color aColor = ImplGetGradientColor( aEndColor, aStartColor, i / (double) nSteps );
                            ImplWriteGradientStop( aColor, fBorderOffset + i * fOffsetStep );
                            aColor = ImplGetGradientColor( aEndColor, aStartColor, (i + 1 ) / (double) nSteps );
                            ImplWriteGradientStop( aColor, fBorderOffset + i * fOffsetStep );
                        }
                        // Lower half
                        for( sal_uInt16 i = 0; i < nSteps; i++ )
                        {
                            Color aColor = ImplGetGradientColor( aStartColor, aEndColor, i / (double) nSteps );
                            ImplWriteGradientStop( aColor, 0.5 + (i + 1) * fOffsetStep );
                            aColor = ImplGetGradientColor( aStartColor, aEndColor, (i + 1 ) / (double) nSteps );
                            ImplWriteGradientStop( aColor, 0.5 + (i + 1) * fOffsetStep );
                        }
                    }
                    else
                    {
                        ImplWriteGradientStop( aEndColor, fBorderOffset );
                        ImplWriteGradientStop( aStartColor, 0.5 );
                        ImplWriteGradientStop( aEndColor, 1.0 - fBorderOffset );
                    }
                }
            }
        }

        FastString aGradientStyle;
        aGradientStyle += B2UCONST( "fill:" );
        aGradientStyle += B2UCONST( "url(#" );
        aGradientStyle += aGradientId.GetString();
        aGradientStyle += B2UCONST( ")" );

        {
            ImplWritePolyPolygon( rPolyPoly, sal_False, &aGradientStyle.GetString() );
        }
    }
}

void SVGActionWriter::ImplWriteGradientStop( const Color& rColor, double fOffset )
{
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset, NMSP_RTL::OUString::valueOf( fOffset ) );

    FastString aStyle;
    aStyle += B2UCONST( "stop-color:" );
    aStyle += mpContext->GetColorStyle ( rColor );

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aStyle.GetString() );
    {
        SvXMLElementExport aElemStartStop( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, TRUE, TRUE );
    }
}

Color SVGActionWriter::ImplGetColorWithIntensity( const Color& rColor,
                                                  sal_uInt16 nIntensity )
{
     sal_uInt8 nNewRed = (sal_uInt8)( (long)rColor.GetRed() * nIntensity / 100L );
     sal_uInt8 nNewGreen = (sal_uInt8)( (long)rColor.GetGreen() * nIntensity / 100L );
     sal_uInt8 nNewBlue = (sal_uInt8)( (long)rColor.GetBlue() * nIntensity / 100L );
     return Color( nNewRed, nNewGreen, nNewBlue);
}

Color SVGActionWriter::ImplGetGradientColor( const Color& rStartColor,
                                             const Color& rEndColor,
                                             double fOffset )
{
    long nRedStep = rEndColor.GetRed() - rStartColor.GetRed();
    long nNewRed = rStartColor.GetRed() + (long)( nRedStep * fOffset );
    nNewRed = ( nNewRed < 0 ) ? 0 : ( nNewRed > 0xFF) ? 0xFF : nNewRed;

    long nGreenStep = rEndColor.GetGreen() - rStartColor.GetGreen();
    long nNewGreen = rStartColor.GetGreen() + (long)( nGreenStep * fOffset );
    nNewGreen = ( nNewGreen < 0 ) ? 0 : ( nNewGreen > 0xFF) ? 0xFF : nNewGreen;

    long nBlueStep = rEndColor.GetBlue() - rStartColor.GetBlue();
    long nNewBlue = rStartColor.GetBlue() + (long)( nBlueStep * fOffset );
    nNewBlue = ( nNewBlue < 0 ) ? 0 : ( nNewBlue > 0xFF) ? 0xFF : nNewBlue;

    return Color( (sal_uInt8)nNewRed, (sal_uInt8)nNewGreen, (sal_uInt8)nNewBlue );
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteMask( GDIMetaFile& rMtf,
                                     const Point& rDestPt,
                                     const Size& rDestSize,
                                     const Gradient& rGradient,
                                     const NMSP_RTL::OUString* pStyle,
                                     sal_uInt32 nWriteFlags )
{
    Point          aSrcPt( rMtf.GetPrefMapMode().GetOrigin() );
    const Size     aSrcSize( rMtf.GetPrefSize() );
    const double   fScaleX = aSrcSize.Width() ? (double) rDestSize.Width() / aSrcSize.Width() : 1.0;
    const double   fScaleY = aSrcSize.Height() ? (double) rDestSize.Height() / aSrcSize.Height() : 1.0;
    long           nMoveX, nMoveY;

    if( fScaleX != 1.0 || fScaleY != 1.0 )
    {
        rMtf.Scale( fScaleX, fScaleY );
        aSrcPt.X() = FRound( aSrcPt.X() * fScaleX ), aSrcPt.Y() = FRound( aSrcPt.Y() * fScaleY );
    }

    nMoveX = rDestPt.X() - aSrcPt.X(), nMoveY = rDestPt.Y() - aSrcPt.Y();

    if( nMoveX || nMoveY )
        rMtf.Move( nMoveX, nMoveY );

    FastString aMaskId;
    aMaskId += B2UCONST( "mask" );
    aMaskId += GetValueString( ImplGetNextMaskId() );

    {
        SvXMLElementExport aElemDefs( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, TRUE, TRUE );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, aMaskId.GetString() );
        {
            SvXMLElementExport aElemMask( mrExport, XML_NAMESPACE_NONE, aXMLElemMask, TRUE, TRUE );

            const PolyPolygon aPolyPolygon( PolyPolygon( Rectangle( rDestPt, rDestSize ) ) );
            Gradient aGradient( rGradient );

            // swap gradient stops to adopt SVG mask
            Color aTmpColor( aGradient.GetStartColor() );
            sal_uInt16 nTmpIntensity( aGradient.GetStartIntensity() );
            aGradient.SetStartColor( aGradient.GetEndColor() );
            aGradient.SetStartIntensity( aGradient.GetEndIntensity() ) ;
            aGradient.SetEndColor( aTmpColor );
            aGradient.SetEndIntensity( nTmpIntensity );

            ImplWriteGradientEx( aPolyPolygon, aGradient, pStyle, nWriteFlags );
        }
    }

    FastString aMaskStyle;
    aMaskStyle += B2UCONST( "mask:url(#" );
    aMaskStyle += aMaskId.GetString();
    aMaskStyle += B2UCONST( ")" );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aMaskStyle.GetString() );

    {
        SvXMLElementExport aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );

        mpVDev->Push();
        ImplWriteActions( rMtf, pStyle, nWriteFlags );
        mpVDev->Pop();
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteText( const Point& rPos, const String& rText,
                                     const sal_Int32* pDXArray, long nWidth,
                                     const NMSP_RTL::OUString* pStyle )
{
    const FontMetric aMetric( mpVDev->GetFontMetric() );

    bool bTextSpecial = aMetric.IsShadow() || aMetric.IsOutline() || (aMetric.GetRelief() != RELIEF_NONE);

    if( !bTextSpecial )
    {
        ImplWriteText( rPos, rText, pDXArray, nWidth, pStyle, mpVDev->GetTextColor() );
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
            Point aOffset( 6, 6 );

            if ( aMetric.GetRelief() == RELIEF_ENGRAVED )
            {
                aPos -= aOffset;
            }
            else
            {
                aPos += aOffset;
            }

            ImplWriteText( aPos, rText, pDXArray, nWidth, pStyle, aReliefColor );
            ImplWriteText( rPos, rText, pDXArray, nWidth, pStyle, aTextColor );
        }
        else
        {
            if( aMetric.IsShadow() )
            {
                long nOff = 1 + ((aMetric.GetLineHeight()-24)/24);
                if ( aMetric.IsOutline() )
                    nOff += 6;

                Color aTextColor( mpVDev->GetTextColor() );
                Color aShadowColor = Color( COL_BLACK );

                if ( (aTextColor.GetColor() == COL_BLACK) || (aTextColor.GetLuminance() < 8) )
                    aShadowColor = Color( COL_LIGHTGRAY );

                Point aPos( rPos );
                aPos += Point( nOff, nOff );
                ImplWriteText( aPos, rText, pDXArray, nWidth, pStyle, aShadowColor );

                if( !aMetric.IsOutline() )
                {
                    ImplWriteText( rPos, rText, pDXArray, nWidth, pStyle, aTextColor );
                }
            }

            if( aMetric.IsOutline() )
            {
                Point aPos = rPos + Point( -6, -6 );
                ImplWriteText( aPos, rText, pDXArray, nWidth, pStyle, mpVDev->GetTextColor() );
                aPos = rPos + Point( +6, +6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, pStyle, mpVDev->GetTextColor() );
                aPos = rPos + Point( -6, +0);
                ImplWriteText( aPos, rText, pDXArray, nWidth, pStyle, mpVDev->GetTextColor() );
                aPos = rPos + Point( -6, +6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, pStyle, mpVDev->GetTextColor() );
                aPos = rPos + Point( +0, +6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, pStyle, mpVDev->GetTextColor() );
                aPos = rPos + Point( +0, -6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, pStyle, mpVDev->GetTextColor() );
                aPos = rPos + Point( +6, -1);
                ImplWriteText( aPos, rText, pDXArray, nWidth, pStyle, mpVDev->GetTextColor() );
                aPos = rPos + Point( +6, +0);
                ImplWriteText( aPos, rText, pDXArray, nWidth, pStyle, mpVDev->GetTextColor() );

                ImplWriteText( rPos, rText, pDXArray, nWidth, pStyle, Color( COL_WHITE ) );
            }
        }
    }
}

void SVGActionWriter::ImplWriteText( const Point& rPos, const String& rText,
                                     const sal_Int32* pDXArray, long nWidth,
                                     const NMSP_RTL::OUString* pStyle,
                                     Color aTextColor )
{
    long nLen = rText.Len(), i;

    if( nLen )
    {
        Size    aNormSize;
        sal_Int32*  pOwnArray;
        sal_Int32*  pDX;

        // get text sizes
        if( pDXArray )
        {
            pOwnArray = NULL;
            aNormSize = Size( mpVDev->GetTextWidth( rText ), 0 );
            pDX = (sal_Int32*) pDXArray;
        }
        else
        {
            pOwnArray = new sal_Int32[ nLen ];
            aNormSize = Size( mpVDev->GetTextArray( rText, pOwnArray ), 0 );
            pDX = pOwnArray;
        }

        if( nLen > 1 )
        {
            aNormSize.Width() = pDX[ nLen - 2 ] + mpVDev->GetTextWidth( rText.GetChar( sal::static_int_cast<USHORT>( nLen - 1 ) ) );

            if( nWidth && aNormSize.Width() && ( nWidth != aNormSize.Width() ) )
            {
                const double fFactor = (double) nWidth / aNormSize.Width();

                for( i = 0; i < ( nLen - 1 ); i++ )
                    pDX[ i ] = FRound( pDX[ i ] * fFactor );
            }
        }

        FastString          aStyle;
        const Font&         rFont = mpVDev->GetFont();
        const FontMetric    aMetric( mpVDev->GetFontMetric() );
        Point               aBaseLinePos( rPos );

        // always adjust text position to match baseline alignment
        switch( rFont.GetAlign() )
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

        mpContext->SetPaintAttr( COL_TRANSPARENT, aTextColor );

        // get mapped text position
        const Point aPt( ImplMap( aBaseLinePos ) );

        // if text is italic, set transform at new g element
        if( ( rFont.GetItalic() != ITALIC_NONE ) || rFont.GetOrientation() )
        {
            String aTransform;

            aTransform = NMSP_RTL::OUString( RTL_CONSTASCII_USTRINGPARAM( "translate" ));
            aTransform += '(';
            aTransform += String( GetValueString( aPt.X() ) );
            aTransform += ',';
            aTransform += String( GetValueString( aPt.Y() ) );
            aTransform += ')';

            if( rFont.GetOrientation() )
            {
                aTransform += String( NMSP_RTL::OUString( RTL_CONSTASCII_USTRINGPARAM( " rotate" )) );
                aTransform += '(';
                aTransform += String( NMSP_RTL::OUString::valueOf( rFont.GetOrientation() * -0.1 ) );
                aTransform += ')';
            }

            if( rFont.GetItalic() != ITALIC_NONE )
            {
                aTransform += String( NMSP_RTL::OUString( RTL_CONSTASCII_USTRINGPARAM( " skewX" )) );
                aTransform += '(';
                aTransform += String( NMSP_RTL::OUString::valueOf( (sal_Int32) -10 ) );
                aTransform += ')';
            }

            aTransform += String( NMSP_RTL::OUString( RTL_CONSTASCII_USTRINGPARAM( " translate" )) );
            aTransform += '(';
            aTransform += String( GetValueString( -aPt.X() ) );
            aTransform += ',';
            aTransform += String( GetValueString( -aPt.Y() ) );
            aTransform += ')';

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTransform, aTransform );
        }

        // add additional style if requested
        if( pStyle && pStyle->getLength() )
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, *pStyle );

        // write text element
        {
#ifdef _SVG_USE_TSPANS
            SvXMLElementExport          aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemText, TRUE, TRUE );
            FastString                  aTSpanX;
            const NMSP_RTL::OUString    aSpace( ' ' );
            String                      aOutputText( rText );
            long                        nCurPos = 0;
            bool                        bIgnoreWhitespace = true;

            for( long j = 0, nX = aPt.X(); j < nLen; ++j )
            {
                const sal_Unicode cCode = rText.GetChar( sal::static_int_cast<USHORT>( j ) );

                // don't take more than one whitespace into account
                if( !bIgnoreWhitespace || ( ' ' != cCode ) )
                {
                    aOutputText.SetChar( sal::static_int_cast<USHORT>( nCurPos++ ), cCode );
                    ( aTSpanX += GetValueString( nX + ( ( j > 0 ) ? pDX[ j - 1 ] : 0 ) ) ) += aSpace;
                    bIgnoreWhitespace = ( ' ' == cCode );
                }
            }

            if( nCurPos < nLen )
                aOutputText.Erase( sal::static_int_cast<USHORT>( nCurPos ) );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, aTSpanX.GetString() );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, GetValueString( aPt.Y() ) );

            {
                SvXMLElementExport aElem2( mrExport, XML_NAMESPACE_NONE, aXMLElemTSpan, TRUE, TRUE );
                mrExport.GetDocHandler()->characters( aOutputText );
            }
#else
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, GetValueString( aPt.X() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, GetValueString( aPt.Y() ) );

            {
                SvXMLElementExport aElem2( mrExport, XML_NAMESPACE_NONE, aXMLElemText, TRUE, TRUE );
                mrExport.GetDocHandler()->characters( rText );
            }
#endif
        }

#ifndef _SVG_USE_NATIVE_TEXTDECORATION
        // write strikeout if neccessary
        if( rFont.GetStrikeout() || rFont.GetUnderline() )
        {
            Polygon     aPoly( 4 );
            const long  nLineHeight = Max( (long) FRound( aMetric.GetLineHeight() * 0.05 ), (long) 1 );

            if( rFont.GetStrikeout() )
            {
                const long  nYLinePos = aBaseLinePos.Y() - FRound( aMetric.GetAscent() * 0.26 );

                aPoly[ 0 ].X() = aBaseLinePos.X(); aPoly[ 0 ].Y() = nYLinePos - ( nLineHeight >> 1 );
                aPoly[ 1 ].X() = aBaseLinePos.X() + aNormSize.Width() - 1; aPoly[ 1 ].Y() = aPoly[ 0 ].Y();
                aPoly[ 2 ].X() = aPoly[ 1 ].X(); aPoly[ 2 ].Y() = aPoly[ 0 ].Y() + nLineHeight - 1;
                aPoly[ 3 ].X() = aPoly[ 0 ].X(); aPoly[ 3 ].Y() = aPoly[ 2 ].Y();

                ImplWritePolyPolygon( aPoly, sal_False );
            }

            if( rFont.GetUnderline() )
            {
                const long  nYLinePos = aBaseLinePos.Y() + ( nLineHeight << 1 );

                aPoly[ 0 ].X() = aBaseLinePos.X(); aPoly[ 0 ].Y() = nYLinePos - ( nLineHeight >> 1 );
                aPoly[ 1 ].X() = aBaseLinePos.X() + aNormSize.Width() - 1; aPoly[ 1 ].Y() = aPoly[ 0 ].Y();
                aPoly[ 2 ].X() = aPoly[ 1 ].X(); aPoly[ 2 ].Y() = aPoly[ 0 ].Y() + nLineHeight - 1;
                aPoly[ 3 ].X() = aPoly[ 0 ].X(); aPoly[ 3 ].Y() = aPoly[ 2 ].Y();

                ImplWritePolyPolygon( aPoly, sal_False );
            }
        }
#endif // _SVG_USE_NATIVE_TEXTDECORATION

        delete[] pOwnArray;
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteBmp( const BitmapEx& rBmpEx,
                                    const Point& rPt, const Size& rSz,
                                    const Point& rSrcPt, const Size& rSrcSz,
                                    const NMSP_RTL::OUString* /* pStyle */ )
{
    if( !!rBmpEx )
    {
        BitmapEx        aBmpEx( rBmpEx );
         Point aPoint = Point();
        const Rectangle aBmpRect( aPoint, rBmpEx.GetSizePixel() );
        const Rectangle aSrcRect( rSrcPt, rSrcSz );

        if( aSrcRect != aBmpRect )
            aBmpEx.Crop( aSrcRect );

        if( !!aBmpEx )
        {
            SvMemoryStream aOStm( 65535, 65535 );

            if( GraphicConverter::Export( aOStm, rBmpEx, CVT_PNG ) == ERRCODE_NONE )
            {
                const Point              aPt( ImplMap( rPt ) );
                const Size               aSz( ImplMap( rSz ) );
                Sequence< sal_Int8 >     aSeq( (sal_Int8*) aOStm.GetData(), aOStm.Tell() );
                NMSP_RTL::OUStringBuffer aBuffer;
                aBuffer.appendAscii( "data:image/png;base64," );
                SvXMLUnitConverter::encodeBase64( aBuffer, aSeq );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, GetValueString( aPt.X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, GetValueString( aPt.Y() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, GetValueString( aSz.Width() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, GetValueString( aSz.Height() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrXLinkHRef, aBuffer.makeStringAndClear() );

                {
                    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemImage, TRUE, TRUE );
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteActions( const GDIMetaFile& rMtf,
                                        const NMSP_RTL::OUString* pStyle,
                                        sal_uInt32 nWriteFlags )
{
    ImplAcquireContext();

    for( ULONG i = 0, nCount = rMtf.GetActionCount(); i < nCount; i++ )
    {
        const MetaAction*   pAction = rMtf.GetAction( i );
        const USHORT        nType = pAction->GetType();

        switch( nType )
        {
            case( META_PIXEL_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaPixelAction* pA = (const MetaPixelAction*) pAction;

                    mpContext->SetPaintAttr( pA->GetColor(), pA->GetColor() );
                    ImplWriteLine( pA->GetPoint(), pA->GetPoint(), &pA->GetColor(), pStyle );
                }
            }
            break;

            case( META_POINT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaPointAction* pA = (const MetaPointAction*) pAction;

                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetLineColor() );
                    ImplWriteLine( pA->GetPoint(), pA->GetPoint(), NULL, pStyle );
                }
            }
            break;

            case( META_LINE_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaLineAction* pA = (const MetaLineAction*) pAction;

                    if(pA->GetLineInfo().IsDefault())
                    {
                        mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetLineColor() );
                    }
                    else
                    {
                        const LineInfo aMappedLineInfo(ImplMap(pA->GetLineInfo()));
                        mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetLineColor(), &aMappedLineInfo );
                    }

                    ImplWriteLine( pA->GetStartPoint(), pA->GetEndPoint(), NULL, pStyle );
                }
            }
            break;

            case( META_RECT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteRect( ( (const MetaRectAction*) pAction )->GetRect(), 0, 0, pStyle );
                }
            }
            break;

            case( META_ROUNDRECT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pAction;

                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound(), pStyle );
                }
            }
            break;

            case( META_ELLIPSE_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaEllipseAction*    pA = (const MetaEllipseAction*) pAction;
                    const Rectangle&            rRect = pA->GetRect();

                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteEllipse( rRect.Center(), rRect.GetWidth() >> 1, rRect.GetHeight() >> 1, pStyle );
                }
            }
            break;

            case( META_ARC_ACTION ):
            case( META_PIE_ACTION ):
            case( META_CHORD_ACTION ):
            case( META_POLYGON_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    Polygon aPoly;

                    switch( nType )
                    {
                        case( META_ARC_ACTION ):
                        {
                            const MetaArcAction* pA = (const MetaArcAction*) pAction;
                            aPoly = Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_ARC );
                        }
                        break;

                        case( META_PIE_ACTION ):
                        {
                            const MetaPieAction* pA = (const MetaPieAction*) pAction;
                            aPoly = Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_PIE );
                        }
                        break;

                        case( META_CHORD_ACTION ):
                        {
                            const MetaChordAction* pA = (const MetaChordAction*) pAction;
                            aPoly = Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_CHORD );
                        }
                        break;

                        case( META_POLYGON_ACTION ):
                            aPoly = ( (const MetaPolygonAction*) pAction )->GetPolygon();
                        break;
                    }

                    if( aPoly.GetSize() )
                    {
                        mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                        ImplWritePolyPolygon( aPoly, sal_False, pStyle );
                    }
                }
            }
            break;

            case( META_POLYLINE_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaPolyLineAction*   pA = (const MetaPolyLineAction*) pAction;
                    const Polygon&              rPoly = pA->GetPolygon();

                    if( rPoly.GetSize() )
                    {
                        bool bNoLineJoin(false);

                        if(pA->GetLineInfo().IsDefault())
                        {
                            mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                        }
                        else
                        {
                            const LineInfo aMappedLineInfo(ImplMap(pA->GetLineInfo()));
                            bNoLineJoin = basegfx::B2DLINEJOIN_NONE == aMappedLineInfo.GetLineJoin();
                            mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor(), &aMappedLineInfo );
                        }

                        if(bNoLineJoin)
                        {
                            // emulate B2DLINEJOIN_NONE by creating single edges
                            const sal_uInt16 nPoints(rPoly.GetSize());
                            const bool bCurve(rPoly.HasFlags());

                            for(sal_uInt16 a(0); a + 1 < nPoints; a++)
                            {
                                if(bCurve
                                    && POLY_NORMAL != rPoly.GetFlags(a + 1)
                                    && a + 2 < nPoints
                                    && POLY_NORMAL != rPoly.GetFlags(a + 2)
                                    && a + 3 < nPoints)
                                {
                                    const Polygon aSnippet(4,
                                        rPoly.GetConstPointAry() + a,
                                        rPoly.GetConstFlagAry() + a);
                                    ImplWritePolyPolygon( aSnippet, sal_True, pStyle );
                                    a += 2;
                                }
                                else
                                {
                                    const Polygon aSnippet(2,
                                        rPoly.GetConstPointAry() + a);
                                    ImplWritePolyPolygon( aSnippet, sal_True, pStyle );
                                }
                            }
                        }
                        else
                        {
                            ImplWritePolyPolygon( rPoly, sal_True, pStyle );
                        }
                    }
                }
            }
            break;

            case( META_POLYPOLYGON_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaPolyPolygonAction*    pA = (const MetaPolyPolygonAction*) pAction;
                    const PolyPolygon&              rPolyPoly = pA->GetPolyPolygon();

                    if( rPolyPoly.Count() )
                    {
                        mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                        ImplWritePolyPolygon( rPolyPoly, sal_False, pStyle );
                    }
                }
            }
            break;

            case( META_GRADIENT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaGradientAction*   pA = (const MetaGradientAction*) pAction;
                    const Polygon               aRectPoly( pA->GetRect() );
                    const PolyPolygon           aRectPolyPoly( aRectPoly );

                    ImplWriteGradientEx( aRectPolyPoly, pA->GetGradient(), pStyle, nWriteFlags );
                }
            }
            break;

            case( META_GRADIENTEX_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaGradientExAction* pA = (const MetaGradientExAction*) pAction;
                    ImplWriteGradientEx( pA->GetPolyPolygon(), pA->GetGradient(), pStyle, nWriteFlags );
                }
            }
            break;

            case META_HATCH_ACTION:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaHatchAction*  pA = (const MetaHatchAction*) pAction;
                    ImplWritePattern( pA->GetPolyPolygon(), &pA->GetHatch(), NULL, pStyle, nWriteFlags );
                }
            }
            break;

            case( META_TRANSPARENT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaTransparentAction*    pA = (const MetaTransparentAction*) pAction;
                    const PolyPolygon&              rPolyPoly = pA->GetPolyPolygon();

                    if( rPolyPoly.Count() )
                    {
                        const Color aOldLineColor( mpVDev->GetLineColor() ), aOldFillColor( mpVDev->GetFillColor() );
                        Color       aNewLineColor( aOldLineColor ), aNewFillColor( aOldFillColor );

                        aNewLineColor.SetTransparency( sal::static_int_cast<UINT8>( FRound( pA->GetTransparence() * 2.55 ) ) );
                        aNewFillColor.SetTransparency( sal::static_int_cast<UINT8>( FRound( pA->GetTransparence() * 2.55 ) ) );

                        mpContext->SetPaintAttr( aNewLineColor, aNewFillColor );
                        ImplWritePolyPolygon( rPolyPoly, sal_False, pStyle );
                        mpContext->SetPaintAttr( aOldLineColor, aOldFillColor );
                    }
                }
            }
            break;

            case( META_FLOATTRANSPARENT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaFloatTransparentAction*   pA = (const MetaFloatTransparentAction*) pAction;
                    GDIMetaFile                         aTmpMtf( pA->GetGDIMetaFile() );
                    ImplWriteMask( aTmpMtf, pA->GetPoint(), pA->GetSize(),
                                   pA->GetGradient(), pStyle, nWriteFlags  );
                }
            }
            break;

            case( META_EPS_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaEPSAction*    pA = (const MetaEPSAction*) pAction;
                    const GDIMetaFile       aGDIMetaFile( pA->GetSubstitute() );
                    sal_Bool                bFound = sal_False;

                    for( ULONG j = 0, nC = aGDIMetaFile.GetActionCount(); ( j < nC ) && !bFound; j++ )
                    {
                        const MetaAction* pSubstAct = aGDIMetaFile.GetAction( j );

                        if( pSubstAct->GetType() == META_BMPSCALE_ACTION )
                        {
                            bFound = sal_True;
                            const MetaBmpScaleAction* pBmpScaleAction = (const MetaBmpScaleAction*) pSubstAct;
                            mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                            ImplWriteBmp( pBmpScaleAction->GetBitmap(),
                                          pA->GetPoint(), pA->GetSize(),
                                          Point(), pBmpScaleAction->GetBitmap().GetSizePixel(), pStyle );
                        }
                    }
                }
            }
            break;

            case( META_COMMENT_ACTION ):
            {
                const MetaCommentAction*    pA = (const MetaCommentAction*) pAction;
                String                      aSkipComment;

                if( ( pA->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL ) &&
                    ( nWriteFlags & SVGWRITER_WRITE_FILL ) )
                {
                    const MetaGradientExAction* pGradAction = NULL;
                    sal_Bool                    bDone = sal_False;

                    while( !bDone && ( ++i < nCount ) )
                    {
                        pAction = rMtf.GetAction( i );

                        if( pAction->GetType() == META_GRADIENTEX_ACTION )
                            pGradAction = (const MetaGradientExAction*) pAction;
                        else if( ( pAction->GetType() == META_COMMENT_ACTION ) &&
                                 ( ( (const MetaCommentAction*) pAction )->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_END" ) == COMPARE_EQUAL ) )
                        {
                            bDone = sal_True;
                        }
                    }

                    if( pGradAction )
                        ImplWriteGradientEx( pGradAction->GetPolyPolygon(), pGradAction->GetGradient(), pStyle, nWriteFlags );
                }
            }
            break;

            case( META_BMP_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpAction* pA = (const MetaBmpAction*) pAction;

                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteBmp( pA->GetBitmap(),
                                  pA->GetPoint(), mpVDev->PixelToLogic( pA->GetBitmap().GetSizePixel() ),
                                  Point(), pA->GetBitmap().GetSizePixel(), pStyle );
                }
            }
            break;

            case( META_BMPSCALE_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;

                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteBmp( pA->GetBitmap(),
                                  pA->GetPoint(), pA->GetSize(),
                                  Point(), pA->GetBitmap().GetSizePixel(), pStyle );
                }
            }
            break;

            case( META_BMPSCALEPART_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpScalePartAction* pA = (const MetaBmpScalePartAction*) pAction;

                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteBmp( pA->GetBitmap(),
                                  pA->GetDestPoint(), pA->GetDestSize(),
                                  pA->GetSrcPoint(), pA->GetSrcSize(), pStyle );
                }
            }
            break;

            case( META_BMPEX_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpExAction*  pA = (const MetaBmpExAction*) pAction;

                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteBmp( pA->GetBitmapEx(),
                                  pA->GetPoint(), mpVDev->PixelToLogic( pA->GetBitmapEx().GetSizePixel() ),
                                  Point(), pA->GetBitmapEx().GetSizePixel(), pStyle );
                }
            }
            break;

            case( META_BMPEXSCALE_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;

                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteBmp( pA->GetBitmapEx(),
                                  pA->GetPoint(), pA->GetSize(),
                                  Point(), pA->GetBitmapEx().GetSizePixel(), pStyle );
                }
            }
            break;

            case( META_BMPEXSCALEPART_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*) pAction;

                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteBmp( pA->GetBitmapEx(),
                                  pA->GetDestPoint(), pA->GetDestSize(),
                                  pA->GetSrcPoint(), pA->GetSrcSize(), pStyle );
                }
            }
            break;

            case( META_TEXT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaTextAction*   pA = (const MetaTextAction*) pAction;
                    Font                    aFont( mpVDev->GetFont() );

                    aFont.SetHeight( ImplMap( Size( 0, aFont.GetHeight() ) ).Height() );
                    mpContext->SetFontAttr( aFont );
                    ImplWriteText( pA->GetPoint(), String( pA->GetText(), pA->GetIndex(), pA->GetLen() ), NULL, 0, pStyle );
                }
            }
            break;

            case( META_TEXTRECT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaTextRectAction*   pA = (const MetaTextRectAction*) pAction;
                    Font                        aFont( mpVDev->GetFont() );

                    aFont.SetHeight( ImplMap( Size( 0, aFont.GetHeight() ) ).Height() );
                    mpContext->SetFontAttr( aFont );
                    ImplWriteText( pA->GetRect().TopLeft(), pA->GetText(), NULL, 0, pStyle );
                }
            }
            break;

            case( META_TEXTARRAY_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaTextArrayAction*  pA = (const MetaTextArrayAction*) pAction;
                    const Point                 aPos( ImplMap( pA->GetPoint() ) );
                    Font                        aFont( mpVDev->GetFont() );

                    aFont.SetHeight( ImplMap( Size( 0, aFont.GetHeight() ) ).Height() );
                    mpContext->SetFontAttr( aFont );
                    ImplWriteText( pA->GetPoint(), String( pA->GetText(), pA->GetIndex(), pA->GetLen() ), pA->GetDXArray(), 0, pStyle );
                }
            }
            break;

            case( META_STRETCHTEXT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaStretchTextAction*    pA = (const MetaStretchTextAction*) pAction;
                    Font                            aFont( mpVDev->GetFont() );

                    aFont.SetHeight( ImplMap( Size( 0, aFont.GetHeight() ) ).Height() );
                    mpContext->SetFontAttr( aFont );
                    ImplWriteText( pA->GetPoint(), String( pA->GetText(), pA->GetIndex(), pA->GetLen() ), NULL, pA->GetWidth(), pStyle );
                }
            }
            break;

            case( META_CLIPREGION_ACTION ):
            case( META_ISECTRECTCLIPREGION_ACTION ):
            case( META_ISECTREGIONCLIPREGION_ACTION ):
            case( META_MOVECLIPREGION_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( mpVDev );
                mbClipAttrChanged = sal_True;
            }
            break;

            case( META_REFPOINT_ACTION ):
            case( META_MAPMODE_ACTION ):
            case( META_LINECOLOR_ACTION ):
            case( META_FILLCOLOR_ACTION ):
            case( META_TEXTLINECOLOR_ACTION ):
            case( META_TEXTFILLCOLOR_ACTION ):
            case( META_TEXTCOLOR_ACTION ):
            case( META_TEXTALIGN_ACTION ):
            case( META_FONT_ACTION ):
            case( META_PUSH_ACTION ):
            case( META_POP_ACTION ):
            case( META_LAYOUTMODE_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( mpVDev );
            }
            break;

            case( META_RASTEROP_ACTION ):
            case( META_MASK_ACTION ):
            case( META_MASKSCALE_ACTION ):
            case( META_MASKSCALEPART_ACTION ):
            case( META_WALLPAPER_ACTION ):
            case( META_TEXTLINE_ACTION ):
            {
                // !!! >>> we don't want to support these actions
            }
            break;

            default:
                DBG_ERROR( "SVGActionWriter::ImplWriteActions: unsupported MetaAction #" );
            break;
        }
    }

    ImplReleaseContext();
}

// -----------------------------------------------------------------------------

void SVGActionWriter::WriteMetaFile( const Point& rPos100thmm,
                                     const Size& rSize100thmm,
                                     const GDIMetaFile& rMtf,
                                     sal_uInt32 nWriteFlags )
{
    MapMode     aMapMode( rMtf.GetPrefMapMode() );
    Size        aPrefSize( rMtf.GetPrefSize() );
    Fraction    aFractionX( aMapMode.GetScaleX() );
    Fraction    aFractionY( aMapMode.GetScaleY() );

    mpVDev->Push();

    Size aSize( OutputDevice::LogicToLogic( rSize100thmm, MAP_100TH_MM, aMapMode ) );
    aMapMode.SetScaleX( aFractionX *= Fraction( aSize.Width(), aPrefSize.Width() ) );
    aMapMode.SetScaleY( aFractionY *= Fraction( aSize.Height(), aPrefSize.Height() ) );

    Point aOffset( OutputDevice::LogicToLogic( rPos100thmm, MAP_100TH_MM, aMapMode ) );
    aMapMode.SetOrigin( aOffset += aMapMode.GetOrigin() );

    mpVDev->SetMapMode( aMapMode );

    ImplWriteActions( rMtf, NULL, nWriteFlags );

    mpVDev->Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
