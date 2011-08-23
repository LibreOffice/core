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

static const char	aXMLElemG[] = "g";
static const char	aXMLElemDefs[] = "defs";
static const char	aXMLElemClipPath[] = "clipPath";
static const char	aXMLElemLine[] = "line";
static const char	aXMLElemRect[] = "rect";
static const char	aXMLElemEllipse[] = "ellipse";
static const char	aXMLElemPath[] = "path";
static const char	aXMLElemPolygon[] = "polygon";
static const char	aXMLElemPolyLine[] = "polyline";
static const char	aXMLElemText[] = "text";
static const char	aXMLElemTSpan[] = "tspan";
static const char	aXMLElemImage[] = "image";

static const char	aXMLAttrTransform[] = "transform";
static const char	aXMLAttrStyle[] = "style";
static const char	aXMLAttrId[] = "id";
static const char	aXMLAttrD[] = "d";
static const char	aXMLAttrX[] = "x";
static const char	aXMLAttrY[] = "y";
static const char	aXMLAttrX1[] = "x1";
static const char	aXMLAttrY1[] = "y1";
static const char	aXMLAttrX2[] = "x2";
static const char	aXMLAttrY2[] = "y2";
static const char	aXMLAttrCX[] = "cx";
static const char	aXMLAttrCY[] = "cy";
static const char	aXMLAttrRX[] = "rx";
static const char	aXMLAttrRY[] = "ry";
static const char	aXMLAttrWidth[] = "width";
static const char	aXMLAttrHeight[] = "height";
static const char	aXMLAttrPoints[] = "points";
static const char	aXMLAttrXLinkHRef[] = "xlink:href";

static const sal_Unicode pBase64[] = 
{
    //0   1   2   3   4   5   6   7
     'A','B','C','D','E','F','G','H', // 0
     'I','J','K','L','M','N','O','P', // 1
     'Q','R','S','T','U','V','W','X', // 2
     'Y','Z','a','b','c','d','e','f', // 3
     'g','h','i','j','k','l','m','n', // 4
     'o','p','q','r','s','t','u','v', // 5
     'w','x','y','z','0','1','2','3', // 6
     '4','5','6','7','8','9','+','/'  // 7
};

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

FastString::FastString( sal_Char* pBufferForBase64Encoding, sal_uInt32 nBufLen ) :
    mnBufInc( 2048 ),
    mnPartPos( 0 )
{
    DBG_ASSERT( pBufferForBase64Encoding && nBufLen, "invalid arguments" );

    const sal_uInt32 nQuadCount = nBufLen / 3;
    const sal_uInt32 nRest = nBufLen % 3;

    if( nQuadCount || nRest )
    {
        mnBufLen = mnCurLen = ( ( nQuadCount + ( nRest ? 1 : 0 ) ) << 2 );
        mpBuffer = new sal_Unicode[ mnBufLen * sizeof( sal_Unicode ) ];

        sal_Char*		pTmpSrc = pBufferForBase64Encoding;
        sal_Unicode*	pTmpDst = mpBuffer;

        for( sal_uInt32 i = 0; i < nQuadCount; i++ )
        {
            const sal_Int32 nA = *pTmpSrc++;
            const sal_Int32 nB = *pTmpSrc++;
            const sal_Int32 nC = *pTmpSrc++;

            *pTmpDst++ = pBase64[ ( nA >> 2 ) & 0x3f ];
            *pTmpDst++ = pBase64[ ( ( nA << 4 ) & 0x30 ) + ( ( nB >> 4 ) & 0xf ) ];
            *pTmpDst++ = pBase64[ ( ( nB << 2 ) & 0x3c ) + ( ( nC >> 6 ) & 0x3 ) ];
            *pTmpDst++ = pBase64[ nC & 0x3f ];
        }

        if( 1 == nRest )
        {
            const sal_Int32 nA = *pTmpSrc;

            *pTmpDst++ = pBase64[ ( nA >> 2 ) & 0x3f ];
            *pTmpDst++ = pBase64[ ( nA << 4 ) & 0x30 ];
            *pTmpDst++ = '=';
            *pTmpDst = '=';
        }
        else if( 2 == nRest )
        {
            const sal_Int32 nA = *pTmpSrc++;
            const sal_Int32 nB = *pTmpSrc;

            *pTmpDst++ = pBase64[ ( nA >> 2 ) & 0x3f ];
            *pTmpDst++ = pBase64[ ( ( nA << 4 ) & 0x30 ) + ( ( nB >> 4 ) & 0xf ) ];
            *pTmpDst++ = pBase64[ ( nB << 2 ) & 0x3c ];
            *pTmpDst = '=';
        }
    }
    else
    {
        mpBuffer = new sal_Unicode[ ( mnBufLen = 1 ) * sizeof( sal_Unicode ) ];
        mnCurLen = 0;
    }
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
            const sal_uInt32	nNewBufLen = ( mnBufLen + ( ( ( mnCurLen + rStr.getLength() ) - mnBufLen ) / mnBufInc + 1 ) * mnBufInc );
            sal_Unicode*		pNewBuffer = new sal_Unicode[ nNewBufLen * sizeof( sal_Unicode ) ];

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

// -----------------------------------------------------------------------------

sal_Bool FastString::GetFirstPartString( const sal_uInt32 nPartLen, NMSP_RTL::OUString& rPartString )
{
    const sal_uInt32 nLength = Min( mnCurLen, nPartLen );

    mnPartPos = 0;

    if( nLength )
    {
        rPartString = NMSP_RTL::OUString( mpBuffer, nLength );
        mnPartPos = nLength;
    }

    return( rPartString.getLength() > 0 );
}

// -----------------------------------------------------------------------------

sal_Bool FastString::GetNextPartString( const sal_uInt32 nPartLen, NMSP_RTL::OUString& rPartString )
{
    if( mnPartPos < mnCurLen )
    {
        const sal_uInt32 nLength = Min( mnCurLen - mnPartPos, nPartLen );
        rPartString = NMSP_RTL::OUString( mpBuffer + mnPartPos, nLength );
        mnPartPos += nLength;
    }
    else
        rPartString = NMSP_RTL::OUString();

    return( rPartString.getLength() > 0 );
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
        case WEIGHT_THIN:			nFontWeight = 100; break;
        case WEIGHT_ULTRALIGHT:		nFontWeight = 200; break;
        case WEIGHT_LIGHT:			nFontWeight = 300; break;
        case WEIGHT_SEMILIGHT:		nFontWeight = 400; break;
        case WEIGHT_NORMAL:			nFontWeight = 400; break;
        case WEIGHT_MEDIUM:			nFontWeight = 500; break;
        case WEIGHT_SEMIBOLD:		nFontWeight = 600; break;
        case WEIGHT_BOLD:			nFontWeight = 700; break;
        case WEIGHT_ULTRABOLD:		nFontWeight = 800; break;
        case WEIGHT_BLACK:			nFontWeight = 900; break;
        default:					nFontWeight = 400; break;
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
        aStyle += B2UCONST( "rgb(" );
        aStyle += NMSP_RTL::OUString::valueOf( (sal_Int32) rLineColor.GetRed() );
        aStyle += B2UCONST( "," );
        aStyle += NMSP_RTL::OUString::valueOf( (sal_Int32) rLineColor.GetGreen() );
        aStyle += B2UCONST( "," );
        aStyle += NMSP_RTL::OUString::valueOf( (sal_Int32) rLineColor.GetBlue() );
        aStyle += B2UCONST( ")" );

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
        aStyle += B2UCONST( "rgb(" );
        aStyle += NMSP_RTL::OUString::valueOf( (sal_Int32) rFillColor.GetRed() );
        aStyle += B2UCONST( "," );
        aStyle += NMSP_RTL::OUString::valueOf( (sal_Int32) rFillColor.GetGreen() );
        aStyle += B2UCONST( "," );
        aStyle += NMSP_RTL::OUString::valueOf( (sal_Int32) rFillColor.GetBlue() );
        aStyle += B2UCONST( ")" );

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
    mnCurClipId( 1 )
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

NMSP_RTL::OUString	SVGActionWriter::GetPathString( const PolyPolygon& rPolyPoly, sal_Bool bLine )
{
    FastString					aPathData;
    const NMSP_RTL::OUString	aBlank( B2UCONST( " " ) );
    const NMSP_RTL::OUString	aComma( B2UCONST( "," ) );
    Point						aPolyPoint;

    for( long i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
    {
        const Polygon&	rPoly = rPolyPoly[ (USHORT) i ];
        USHORT			n = 1, nSize = rPoly.GetSize();

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
        PolyPolygon	aMappedPolyPoly;
        FastString	aStyle;

        for( USHORT i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
        {
            const Polygon&	rPoly = rPolyPoly[ i ];
            const USHORT	nSize = rPoly.GetSize();

            // #i102224# congratulations, this throws away the curve flags
            // and makes ANY curved polygon look bad. The Flags HAVE to be
            // copied, too. It's NOT enough to copy the mapped points. Just
            // copy the original polygon completely and REPLACE the points

            // old: Polygon			aMappedPoly( nSize );
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

void SVGActionWriter::ImplWriteGradientEx( const PolyPolygon& rPolyPoly, const Gradient& rGradient,
                                           const NMSP_RTL::OUString* pStyle, sal_uInt32 nWriteFlags )
{
    if( rPolyPoly.Count() )
    {
        SvXMLElementExport	aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );
        FastString			aClipId;
        FastString			aClipStyle;

        aClipId += B2UCONST( "clip" );
        aClipId += NMSP_RTL::OUString::valueOf( ImplGetNextClipId() );

        {
            SvXMLElementExport aElemDefs( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, TRUE, TRUE );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, aClipId.GetString() );

            {
                SvXMLElementExport aElemClipPath( mrExport, XML_NAMESPACE_NONE, aXMLElemClipPath, TRUE, TRUE );
                ImplWritePolyPolygon( rPolyPoly, sal_False );
            }
        }

        // create new context with clippath set
        aClipStyle += B2UCONST( "clip-path:URL(#" );
        aClipStyle += aClipId.GetString();
        aClipStyle += B2UCONST( ")" );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aClipStyle.GetString() );

        {
            GDIMetaFile			aTmpMtf;
            SvXMLElementExport	aElemG2( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );

            mpVDev->AddGradientActions( rPolyPoly.GetBoundRect(), rGradient, aTmpMtf );
            ImplWriteActions( aTmpMtf, pStyle, nWriteFlags );
        }
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteText( const Point& rPos, const String& rText, 
                                     const sal_Int32* pDXArray, long nWidth,
                                     const NMSP_RTL::OUString* pStyle )
{
    long nLen = rText.Len(), i;

    if( nLen )
    {
        Size	aNormSize;
        sal_Int32*	pOwnArray;
        sal_Int32*	pDX;

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

        FastString			aStyle;
        const Font&			rFont = mpVDev->GetFont();
        const FontMetric	aMetric( mpVDev->GetFontMetric() );
        Point				aBaseLinePos( rPos );
        SvXMLElementExport*	pTransform = NULL;

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

        // get mapped text position
        const Point aPt( ImplMap( aBaseLinePos ) );

        // if text is italic, set transform at new g element
        if( ( rFont.GetItalic() != ITALIC_NONE ) || rFont.GetOrientation() )
        {
            String aTransform;

            aTransform = NMSP_RTL::OUString::createFromAscii( "translate" );
            aTransform += '(';
            aTransform += String( GetValueString( aPt.X() ) );
            aTransform += ',';
            aTransform += String( GetValueString( aPt.Y() ) );
            aTransform += ')';

            if( rFont.GetOrientation() )
            {
                aTransform += String( NMSP_RTL::OUString::createFromAscii( " rotate" ) );
                aTransform += '(';
                aTransform += String( NMSP_RTL::OUString::valueOf( rFont.GetOrientation() * -0.1 ) );
                aTransform += ')';
            }
            
            if( rFont.GetItalic() != ITALIC_NONE )
            {
                aTransform += String( NMSP_RTL::OUString::createFromAscii( " skewX" ) );
                aTransform += '(';
                aTransform += String( NMSP_RTL::OUString::valueOf( (sal_Int32) -10 ) );
                aTransform += ')';
            }

            aTransform += String( NMSP_RTL::OUString::createFromAscii( " translate" ) );
            aTransform += '(';
            aTransform += String( GetValueString( -aPt.X() ) );
            aTransform += ',';
            aTransform += String( GetValueString( -aPt.Y() ) );
            aTransform += ')';

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTransform, aTransform );
            pTransform = new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );
        }

        // add additional style if requested
        if( pStyle && pStyle->getLength() )
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, *pStyle );

        // write text element
        {
#ifdef _SVG_USE_TSPANS
            SvXMLElementExport			aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemText, TRUE, TRUE );
            FastString					aTSpanX;
            const NMSP_RTL::OUString	aSpace( ' ' );
            String 						aOutputText( rText );
            long						nCurPos = 0;
            bool						bIgnoreWhitespace = true;
    
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
            Polygon		aPoly( 4 );
            const long	nLineHeight = Max( (long) FRound( aMetric.GetLineHeight() * 0.05 ), (long) 1 );

            if( rFont.GetStrikeout() )
            {
                const long	nYLinePos = aBaseLinePos.Y() - FRound( aMetric.GetAscent() * 0.26 );

                aPoly[ 0 ].X() = aBaseLinePos.X(); aPoly[ 0 ].Y() = nYLinePos - ( nLineHeight >> 1 );
                aPoly[ 1 ].X() = aBaseLinePos.X() + aNormSize.Width() - 1; aPoly[ 1 ].Y() = aPoly[ 0 ].Y();
                aPoly[ 2 ].X() = aPoly[ 1 ].X(); aPoly[ 2 ].Y() = aPoly[ 0 ].Y() + nLineHeight - 1;
                aPoly[ 3 ].X() = aPoly[ 0 ].X(); aPoly[ 3 ].Y() = aPoly[ 2 ].Y();
                        
                ImplWritePolyPolygon( aPoly, sal_False );
            }

            if( rFont.GetUnderline() )
            {
                const long	nYLinePos = aBaseLinePos.Y() + ( nLineHeight << 1 );

                aPoly[ 0 ].X() = aBaseLinePos.X(); aPoly[ 0 ].Y() = nYLinePos - ( nLineHeight >> 1 );
                aPoly[ 1 ].X() = aBaseLinePos.X() + aNormSize.Width() - 1; aPoly[ 1 ].Y() = aPoly[ 0 ].Y();
                aPoly[ 2 ].X() = aPoly[ 1 ].X(); aPoly[ 2 ].Y() = aPoly[ 0 ].Y() + nLineHeight - 1;
                aPoly[ 3 ].X() = aPoly[ 0 ].X(); aPoly[ 3 ].Y() = aPoly[ 2 ].Y();
                        
                ImplWritePolyPolygon( aPoly, sal_False );
            }
        }
#endif // _SVG_USE_NATIVE_TEXTDECORATION

        delete[] pOwnArray;
        delete pTransform;
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
        BitmapEx		aBmpEx( rBmpEx );
         Point aPoint = Point(); 	
        const Rectangle	aBmpRect( aPoint, rBmpEx.GetSizePixel() );
        const Rectangle aSrcRect( rSrcPt, rSrcSz );

        if( aSrcRect != aBmpRect )
            aBmpEx.Crop( aSrcRect );

        if( !!aBmpEx )
        {
            SvMemoryStream aOStm( 65535, 65535 );

            if( GraphicConverter::Export( aOStm, rBmpEx, CVT_PNG ) == ERRCODE_NONE )
            {
                const Point									aPt( ImplMap( rPt ) );
                const Size									aSz( ImplMap( rSz ) );
                FastString									aImageData( (sal_Char*) aOStm.GetData(), aOStm.Tell() );
                REF( NMSP_SAX::XExtendedDocumentHandler )	xExtDocHandler( mrExport.GetDocHandler(), NMSP_UNO::UNO_QUERY );

                if( xExtDocHandler.is() )
                {
                    static const sal_uInt32		nPartLen = 64;
                    const NMSP_RTL::OUString	aSpace( ' ' );
                    const NMSP_RTL::OUString	aLineFeed( NMSP_RTL::OUString::valueOf( (sal_Unicode) 0x0a ) );
                    NMSP_RTL::OUString			aString;
                    NMSP_RTL::OUString			aImageString;

                    aString = aLineFeed;
                    aString +=  B2UCONST( "<" );
                    aString += NMSP_RTL::OUString::createFromAscii( aXMLElemImage );
                    aString += aSpace;
                    
                    aString += NMSP_RTL::OUString::createFromAscii( aXMLAttrX );
                    aString += B2UCONST( "=\"" );
                    aString += GetValueString( aPt.X() );
                    aString += B2UCONST( "\" " );

                    aString += NMSP_RTL::OUString::createFromAscii( aXMLAttrY );
                    aString += B2UCONST( "=\"" );
                    aString += GetValueString( aPt.Y() );
                    aString += B2UCONST( "\" " );

                    aString += NMSP_RTL::OUString::createFromAscii( aXMLAttrWidth );
                    aString += B2UCONST( "=\"" );
                    aString += GetValueString( aSz.Width() );
                    aString += B2UCONST( "\" " );

                    aString += NMSP_RTL::OUString::createFromAscii( aXMLAttrHeight );
                    aString += B2UCONST( "=\"" );
                    aString += GetValueString( aSz.Height() );
                    aString += B2UCONST( "\" " );

                    aString += NMSP_RTL::OUString::createFromAscii( aXMLAttrXLinkHRef );
                    aString += B2UCONST( "=\"data:image/png;base64," );

                    if( aImageData.GetFirstPartString( nPartLen, aImageString ) )
                    {
                        xExtDocHandler->unknown( aString += aImageString );

                        while( aImageData.GetNextPartString( nPartLen, aImageString ) )
                        {
                            xExtDocHandler->unknown( aLineFeed );
                            xExtDocHandler->unknown( aImageString );
                        }
                    }

                    xExtDocHandler->unknown( B2UCONST( "\"/>" ) );
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
        const MetaAction*	pAction = rMtf.GetAction( i );
        const USHORT		nType = pAction->GetType();

        switch( nType )
        {
            case( META_PIXEL_ACTION	):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaPixelAction* pA = (const MetaPixelAction*) pAction;

                    mpContext->SetPaintAttr( pA->GetColor(), pA->GetColor() );
                    ImplWriteLine( pA->GetPoint(), pA->GetPoint(), &pA->GetColor(), pStyle );
                }
            }
            break;

            case( META_POINT_ACTION	):
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

            case( META_ROUNDRECT_ACTION	):
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
                    const MetaEllipseAction*	pA = (const MetaEllipseAction*) pAction;
                    const Rectangle&			rRect = pA->GetRect();

                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteEllipse( rRect.Center(), rRect.GetWidth() >> 1, rRect.GetHeight() >> 1, pStyle );
                }
            }
            break;

            case( META_ARC_ACTION ):
            case( META_PIE_ACTION ):
            case( META_CHORD_ACTION	):
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

                        case( META_CHORD_ACTION	):
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
                    const MetaPolyLineAction*	pA = (const MetaPolyLineAction*) pAction;
                    const Polygon&				rPoly = pA->GetPolygon();

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
                    const MetaPolyPolygonAction*	pA = (const MetaPolyPolygonAction*) pAction;
                    const PolyPolygon&				rPolyPoly = pA->GetPolyPolygon();

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
                    const MetaGradientAction*	pA = (const MetaGradientAction*) pAction;
                    const Polygon				aRectPoly( pA->GetRect() );
                    const PolyPolygon			aRectPolyPoly( aRectPoly );

                    ImplWriteGradientEx( aRectPolyPoly, pA->GetGradient(), pStyle, nWriteFlags );
                }
            }
            break;

            case( META_GRADIENTEX_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaGradientExAction*	pA = (const MetaGradientExAction*) pAction;
                    ImplWriteGradientEx( pA->GetPolyPolygon(), pA->GetGradient(), pStyle, nWriteFlags );
                }
            }
            break;

            case META_HATCH_ACTION:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaHatchAction*	pA = (const MetaHatchAction*) pAction;
                    GDIMetaFile				aTmpMtf;

                    mpVDev->AddHatchActions( pA->GetPolyPolygon(), pA->GetHatch(), aTmpMtf );
                    ImplWriteActions( aTmpMtf, pStyle, nWriteFlags );
                }
            }
            break;

            case( META_TRANSPARENT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaTransparentAction*	pA = (const MetaTransparentAction*) pAction;
                    const PolyPolygon&				rPolyPoly = pA->GetPolyPolygon();

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
                    const MetaFloatTransparentAction*	pA = (const MetaFloatTransparentAction*) pAction;
                    GDIMetaFile							aTmpMtf( pA->GetGDIMetaFile() );
                    Point								aSrcPt( aTmpMtf.GetPrefMapMode().GetOrigin() );
                    const Size							aSrcSize( aTmpMtf.GetPrefSize() );
                    const Point							aDestPt( pA->GetPoint() );
                    const Size							aDestSize( pA->GetSize() );
                    const double						fScaleX = aSrcSize.Width() ? (double) aDestSize.Width() / aSrcSize.Width() : 1.0;
                    const double						fScaleY = aSrcSize.Height() ? (double) aDestSize.Height() / aSrcSize.Height() : 1.0;
                    long								nMoveX, nMoveY;

                    if( fScaleX != 1.0 || fScaleY != 1.0 )
                    {
                        aTmpMtf.Scale( fScaleX, fScaleY );
                        aSrcPt.X() = FRound( aSrcPt.X() * fScaleX ), aSrcPt.Y() = FRound( aSrcPt.Y() * fScaleY );
                    }

                    nMoveX = aDestPt.X() - aSrcPt.X(), nMoveY = aDestPt.Y() - aSrcPt.Y();

                    if( nMoveX || nMoveY )
                        aTmpMtf.Move( nMoveX, nMoveY );

                    mpVDev->Push();
                    ImplWriteActions( aTmpMtf, pStyle, nWriteFlags );
                    mpVDev->Pop();
                }
            }
            break;

            case( META_EPS_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaEPSAction*	pA = (const MetaEPSAction*) pAction;
                    const GDIMetaFile		aGDIMetaFile( pA->GetSubstitute() );
                    sal_Bool				bFound = sal_False;

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
                const MetaCommentAction*	pA = (const MetaCommentAction*) pAction;
                String						aSkipComment;

                if( ( pA->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL ) && 
                    ( nWriteFlags & SVGWRITER_WRITE_FILL ) )
                {
                    const MetaGradientExAction*	pGradAction = NULL;
                    sal_Bool					bDone = sal_False;

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

            case( META_BMPEX_ACTION	):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpExAction*	pA = (const MetaBmpExAction*) pAction;
                    
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
                    mpContext->SetPaintAttr( COL_TRANSPARENT, aFont.GetColor() );
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
                    mpContext->SetPaintAttr( COL_TRANSPARENT, aFont.GetColor() );
                    ImplWriteText( pA->GetRect().TopLeft(), pA->GetText(), NULL, 0, pStyle );
                }
            }
            break;

            case( META_TEXTARRAY_ACTION	):
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaTextArrayAction*	pA = (const MetaTextArrayAction*) pAction;
                    const Point					aPos( ImplMap( pA->GetPoint() ) );
                    Font                        aFont( mpVDev->GetFont() );

                    aFont.SetHeight( ImplMap( Size( 0, aFont.GetHeight() ) ).Height() );
                    mpContext->SetFontAttr( aFont );
                    mpContext->SetPaintAttr( COL_TRANSPARENT, aFont.GetColor() );
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
                    mpContext->SetPaintAttr( COL_TRANSPARENT, aFont.GetColor() );
                    ImplWriteText( pA->GetPoint(), String( pA->GetText(), pA->GetIndex(), pA->GetLen() ), NULL, pA->GetWidth(), pStyle );
                }
            }
            break;

            case( META_CLIPREGION_ACTION ):
            case( META_ISECTRECTCLIPREGION_ACTION ):
            case( META_ISECTREGIONCLIPREGION_ACTION	):
            case( META_MOVECLIPREGION_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( mpVDev );
                mbClipAttrChanged = sal_True;
            }
            break;

            case( META_REFPOINT_ACTION ):
            case( META_MAPMODE_ACTION ):
            case( META_LINECOLOR_ACTION	):
            case( META_FILLCOLOR_ACTION	):
            case( META_TEXTLINECOLOR_ACTION ):
            case( META_TEXTFILLCOLOR_ACTION	):
            case( META_TEXTCOLOR_ACTION	):
            case( META_TEXTALIGN_ACTION	):
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
            case( META_MASKSCALE_ACTION	):
            case( META_MASKSCALEPART_ACTION	):
            case( META_WALLPAPER_ACTION	):
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
    MapMode	    aMapMode( rMtf.GetPrefMapMode() );
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
