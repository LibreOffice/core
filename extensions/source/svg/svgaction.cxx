/*************************************************************************
 *
 *  $RCSfile: svgaction.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:53 $
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

#include "svgaction.hxx"
#include <vcl/virdev.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/metric.hxx>

// -----------
// - Defines -
// -----------

// #define _SVG_WRITE_TSPAN
// #define _SVG_WRITE_STRIKEOUT

// -----------
// - statics -
// -----------

static const char   aXMLElemSVG[] = "svg";
static const char   aXMLElemG[] = "g";
static const char   aXMLElemDefs[] = "defs";
static const char   aXMLElemClipPath[] = "clipPath";
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
static const char   aXMLAttrXLinkHRef[] = "xlink:href";

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

        sal_Char*       pTmpSrc = pBufferForBase64Encoding;
        sal_Unicode*    pTmpDst = mpBuffer;

        for( sal_uInt32 i = 0, nCharCount = 0; i < nQuadCount; i++ )
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
            const sal_uInt32    nNewBufLen = ( mnBufLen + ( ( ( mnCurLen + rStr.getLength() ) - mnBufLen ) / mnBufInc + 1 ) * mnBufInc );
            sal_Unicode*        pNewBuffer = new sal_Unicode[ nNewBufLen * sizeof( sal_Unicode ) ];

            HMEMCPY( pNewBuffer, mpBuffer, mnBufLen * sizeof( sal_Unicode )  );
            delete[] mpBuffer;
            mpBuffer = pNewBuffer;
            mnBufLen = nNewBufLen;
        }

        HMEMCPY( mpBuffer + mnCurLen, rStr.getStr(), rStr.getLength() * sizeof( sal_Unicode ) );
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

SVGAttributeWriter::SVGAttributeWriter( SVGActionWriter& rParent, SvXMLExport& rExport ) :
    mrParent( rParent ),
    mrExport( rExport ),
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
    aStyle += NMSP_RTL::OUString( UniString( rFont.GetName() ) );

    // font size
    const Size aFontSize( mrParent.GetVDev().LogicToLogic( Size( 0, rFont.GetHeight() ), mrParent.GetVDev().GetMapMode(), MAP_POINT ) );

    aStyle += B2UCONST( ";" );
    aStyle += B2UCONST( "font-size:" );
    aStyle += NMSP_RTL::OUString::valueOf( aFontSize.Height() );

    if( !mrParent.HasDoublePoints() )
        aStyle += B2UCONST( "pt" );

    // font style
    if( rFont.GetItalic() != ITALIC_NONE )
    {
        aStyle += B2UCONST( ";" );
        aStyle += B2UCONST( "font-style:" );

        if( rFont.GetItalic() == ITALIC_OBLIQUE )
            aStyle += B2UCONST( "oblique" );
        else
            aStyle += B2UCONST( "italic" );
    }

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

#ifdef _SVG_WRITE_STRIKEOUT

    if( rFont.GetUnderline() != UNDERLINE_NONE || rFont.GetStrikeout() != STRIKEOUT_NONE )
    {
        aStyle += B2UCONST( ";" );
        aStyle += B2UCONST( "text-decoration:" );

        if( rFont.GetUnderline() != UNDERLINE_NONE )
            aStyle += B2UCONST( " underline" );

        if( rFont.GetStrikeout() != STRIKEOUT_NONE )
            aStyle += B2UCONST( " line-trough" );
    }

#else // _SVG_WRITE_STRIKEOUT

    if( rFont.GetUnderline() != UNDERLINE_NONE )
    {
        aStyle += B2UCONST( ";" );
        aStyle += B2UCONST( "text-decoration:" );
        aStyle += B2UCONST( "underline" );
    }

#endif // _SVG_WRITE_STRIKEOUT

    return aStyle.GetString();
}

// -----------------------------------------------------------------------------

NMSP_RTL::OUString SVGAttributeWriter::GetPaintStyle( const Color& rLineColor, const Color& rFillColor )
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
            aStyle += NMSP_RTL::OUString::valueOf( (sal_Int32) ( ( 255 - (double) rLineColor.GetTransparency() ) / 2.55 ) );
            aStyle += B2UCONST( "%" );
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
            aStyle += NMSP_RTL::OUString::valueOf( (sal_Int32) ( ( 255 - (double) rFillColor.GetTransparency() ) / 2.55 ) );
            aStyle += B2UCONST( "%" );
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

void SVGAttributeWriter::SetPaintAttr( const Color& rLineColor, const Color& rFillColor )
{
    if( !mpElemPaint || ( rLineColor != maCurLineColor ) || ( rFillColor != maCurFillColor ) )
    {
        delete mpElemPaint;
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, GetPaintStyle( maCurLineColor = rLineColor, maCurFillColor = rFillColor ) );
        mpElemPaint = new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );
    }
}

// -------------------
// - SVGActionWriter -
// -------------------

SVGActionWriter::SVGActionWriter( SvXMLExport& rExport, const GDIMetaFile& rMtf,
                                  VirtualDevice* pParentVDev, sal_Bool bWriteDoublePoints ) :
    mrExport( rExport ),
    mrMtf( rMtf ),
    mpContext( NULL ),
    mbClipAttrChanged( sal_False ),
    mnCurClipId( 1 ),
    mbDoublePoints( bWriteDoublePoints )
{
    if( pParentVDev )
    {
        mpVDev = pParentVDev;
        mbDestroyVDev = sal_False;
        maTargetMapMode = mpVDev->GetMapMode();
    }
    else
    {
        mpVDev = new VirtualDevice;
        mpVDev->EnableOutput( sal_False );
        mbDestroyVDev = sal_True;
        maTargetMapMode = MAP_100TH_MM;
    }

    mpVDev->Push();
    mpVDev->SetMapMode( mrMtf.GetPrefMapMode() );
    ImplWriteActions( mrMtf );
    mpVDev->Pop();
}

// -----------------------------------------------------------------------------

SVGActionWriter::~SVGActionWriter()
{
    DBG_ASSERT( !mpContext, "Not all contexts are closed" );

    if( mbDestroyVDev )
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

NMSP_RTL::OUString SVGActionWriter::GetValueString( sal_Int32 nVal, sal_Bool bDoublePoints )
{
    if( !bDoublePoints )
        return NMSP_RTL::OUString::valueOf( nVal );
    else
    {
        const double    fPoints = nVal * 72.0 / 2540.0;
        const sal_Int32 nInt = (sal_Int32) fPoints;

        return( ( NMSP_RTL::OUString::valueOf( nInt ) +=
                NMSP_RTL::OUString::valueOf( (sal_Unicode) '.' ) ) +=
                NMSP_RTL::OUString::valueOf( labs( (sal_Int32) ( ( fPoints - nInt ) * 100.0 ) ) ) );
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteLine( const Point& rPt1, const Point& rPt2, const Color* pLineColor,
                                     const NMSP_RTL::OUString* pStyle )
{
    const Point aPt1( ImplMap( rPt1 ) );
    const Point aPt2( ImplMap( rPt2 ) );

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX1, GetValueString( aPt1.X(), mbDoublePoints ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY1, GetValueString( aPt1.Y(), mbDoublePoints ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX2, GetValueString( aPt2.X(), mbDoublePoints ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY2, GetValueString( aPt2.Y(), mbDoublePoints ) );

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

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, GetValueString( aRect.Left(), mbDoublePoints ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, GetValueString( aRect.Top(), mbDoublePoints ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, GetValueString( aRect.GetWidth(), mbDoublePoints ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, GetValueString( aRect.GetHeight(), mbDoublePoints ) );

    if( nRadX )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRX, GetValueString( ImplMap( nRadX ), mbDoublePoints ) );

    if( nRadY )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRY, GetValueString( ImplMap( nRadY ), mbDoublePoints ) );

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

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCX, GetValueString( aCenter.X(), mbDoublePoints ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCY, GetValueString( aCenter.Y(), mbDoublePoints ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRX, GetValueString( ImplMap( nRadX ), mbDoublePoints ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRY, GetValueString( ImplMap( nRadY ), mbDoublePoints ) );

    // add additional style if requested
    if( pStyle )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, *pStyle );

    {
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemEllipse, TRUE, TRUE );
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWritePolygon( const Polygon& rPoly, sal_Bool bLineOnly,
                                        const NMSP_RTL::OUString* pStyle )
{
    if( rPoly.GetSize() )
    {
        FastString                  aStyle;
        FastString                  aPoints;
        USHORT                      i = 0, nSize = rPoly.GetSize();
        const NMSP_RTL::OUString    aBlank( B2UCONST( " " ) );

        // points
        while( i < nSize )
        {
            const Point aPolyPoint( ImplMap( rPoly[ i ] ) );

            aPoints += GetValueString( aPolyPoint.X(), mbDoublePoints );
            aPoints += B2UCONST( "," );
            aPoints += GetValueString( aPolyPoint.Y(), mbDoublePoints );

            if( ++i < nSize )
                aPoints += aBlank;
        }

        // style
        if( bLineOnly )
        {
            aStyle += B2UCONST( "fill:none" );

            if( pStyle )
            {
                aStyle += B2UCONST( ";" );
                aStyle += *pStyle;
            }
        }
        else if( pStyle )
            aStyle += *pStyle;

        // add point attribute
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrPoints, aPoints.GetString() );

        // add style attribute
        if( aStyle.GetLength() )
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aStyle.GetString() );

        {
            // write polyline/polygon element
            SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, bLineOnly ? aXMLElemPolyLine : aXMLElemPolygon, TRUE, TRUE );
        }
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWritePolyPolygon( const PolyPolygon& rPolyPoly, sal_Bool bLineOnly,
                                            const NMSP_RTL::OUString* pStyle )
{
    if( rPolyPoly.Count() )
    {
        if( rPolyPoly.Count() == 1 )
            ImplWritePolygon( rPolyPoly[ 0 ], bLineOnly, pStyle );
        else if( bLineOnly )
        {
            for( long i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
                ImplWritePolygon( rPolyPoly[ i ], sal_False, pStyle );
        }
        else
        {
            FastString                  aPathData;
            const NMSP_RTL::OUString    aBlank( B2UCONST( " " ) );
            const NMSP_RTL::OUString    aComma( B2UCONST( "," ) );
            Point                       aPolyPoint;

            for( long i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
            {
                const Polygon&  rPoly = rPolyPoly[ i ];
                USHORT          n = 1, nSize = rPoly.GetSize();

                if( nSize > 1 )
                {
                    aPathData += B2UCONST( "M " );
                    aPathData += GetValueString( ( aPolyPoint = ImplMap( rPoly[ 0 ] ) ).X(), mbDoublePoints );
                    aPathData += aComma;
                    aPathData += GetValueString( aPolyPoint.Y(), mbDoublePoints );
                    aPathData += B2UCONST( " L " );

                    while( n < nSize )
                    {
                        aPathData += GetValueString( ( aPolyPoint = ImplMap( rPoly[ n ] ) ).X(), mbDoublePoints );
                        aPathData += aComma;
                        aPathData += GetValueString( aPolyPoint.Y(), mbDoublePoints );

                        if( ++n < nSize )
                            aPathData += aBlank;
                    }

                    aPathData += B2UCONST( " Z" );

                    if( i < ( nCount - 1 ) )
                        aPathData += aBlank;
                }
            }

            // add style attribute
            if( pStyle && pStyle->getLength() )
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, *pStyle );

            // add path data attribute
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrD, aPathData.GetString() );

            {
                // write polyline/polygon element
                SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemPath, TRUE, TRUE );
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteGradientEx( const PolyPolygon& rPolyPoly, const Gradient& rGradient,
                                           const NMSP_RTL::OUString* pStyle )
{
    if( rPolyPoly.Count() )
    {
        SvXMLElementExport  aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );
        FastString          aClipId;
        FastString          aClipStyle;

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
            GDIMetaFile         aTmpMtf;
            SvXMLElementExport  aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, TRUE, TRUE );

            mpVDev->AddGradientActions( rPolyPoly.GetBoundRect(), rGradient, aTmpMtf );
            ImplWriteActions( aTmpMtf, pStyle );
        }
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteText( const Point& rPos, const String& rText,
                                     const long* pDXArray, long nWidth,
                                     const NMSP_RTL::OUString* pStyle )
{
    String  aText( rText ); aText.EraseLeadingChars( ' ' );
    UINT32  nLen = aText.Len(), i;

    if( nLen )
    {
        Size    aNormSize;
        long*   pOwnArray;
        long*   pDX;

        // get text sizes
        if( pDXArray )
        {
            pOwnArray = NULL;
#ifdef _SVG_UNO3
            aNormSize = Size( mpVDev->GetTextWidth( aText ), 0 );
#else
            aNormSize = mpVDev->GetTextSize( aText );
#endif
            pDX = (long*) pDXArray;
        }
        else
        {
            pOwnArray = new long[ nLen ];
#ifdef _SVG_UNO3
            aNormSize = Size( mpVDev->GetTextArray( aText, pOwnArray ), 0 );
#else
            aNormSize = mpVDev->GetTextArray( aText, pOwnArray );
#endif
            pDX = pOwnArray;
        }

        if( nLen > 1 )
        {
#ifdef _SVG_UNO3
            aNormSize.Width() = pDX[ nLen - 2 ] + mpVDev->GetTextWidth( aText.GetChar(  nLen - 1 ) );
#else
            aNormSize.Width() = pDX[ nLen - 2 ] + mpVDev->GetTextSize( aText[ (USHORT) ( nLen - 1 ) ] ).Width();
#endif

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

        // leading whitespaces erased? => adjust position
        if( nLen < rText.Len() )
        {
#ifdef _SVG_UNO3
            aBaseLinePos.X() += mpVDev->GetTextWidth( ' ' ) * ( rText.Len() - nLen );
#else
            aBaseLinePos.X() += mpVDev->GetTextSize( ' ' ).Width() * ( rText.Len() - nLen );
#endif
        }

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

        // add additional style if requested
        if( pStyle && pStyle->getLength() )
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, *pStyle );

        // write text element
#ifdef _SVG_WRITE_TSPAN
        if( pDXArray )
        {
            SvXMLElementExport          aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemText, TRUE, TRUE );
            FastString                  aTSpanX;
            const NMSP_RTL::OUString    aSpace( ' ' );
            long                        i, nX, nCount;

            aTSpanX += GetValueString( aPt.X(), mbDoublePoints );
            aTSpanX += aSpace;

            for( i = 0, nX = aPt.X(), nCount = ( nLen - 1 ); i < nCount;  )
            {
                aTSpanX += GetValueString( aPt.X() + pDX[ i++ ], mbDoublePoints );
                aTSpanX += aSpace;
            }

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, aTSpanX.GetString() );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, GetValueString( aPt.Y(), mbDoublePoints ) );

            {
                SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemTSpan, TRUE, TRUE );
                mrExport.GetDocHandler()->characters( NMSP_RTL::OUString( UniString( aText ) ) );
            }
        }
        else
#endif // _SVG_WRITE_TSPAN
        {
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, GetValueString( aPt.X(), mbDoublePoints ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, GetValueString( aPt.Y(), mbDoublePoints ) );

            SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemText, TRUE, TRUE );
            mrExport.GetDocHandler()->characters( NMSP_RTL::OUString( UniString( aText ) ) );
        }
#ifndef _SVG_WRITE_STRIKEOUT

        // write strikeout if neccessary
        if( rFont.GetStrikeout() )
        {
            const long  nYStrikeoutPos = aBaseLinePos.Y() - FRound( aMetric.GetAscent() * 0.26 );
            const long  nStrikeoutHeight = Max( (long) FRound( aMetric.GetLineHeight() * 0.05 ), (long) 1 );
            Polygon     aPoly( 4 );

            aPoly[ 0 ].X() = aBaseLinePos.X(); aPoly[ 0 ].Y() = nYStrikeoutPos - ( nStrikeoutHeight >> 1 );
            aPoly[ 1 ].X() = aBaseLinePos.X() + aNormSize.Width() - 1; aPoly[ 1 ].Y() = aPoly[ 0 ].Y();
            aPoly[ 2 ].X() = aPoly[ 1 ].X(); aPoly[ 2 ].Y() = aPoly[ 0 ].Y() + nStrikeoutHeight - 1;
            aPoly[ 3 ].X() = aPoly[ 0 ].X(); aPoly[ 3 ].Y() = aPoly[ 2 ].Y();

            ImplWritePolygon( aPoly, FALSE );
        }

#endif // _SVG_WRITE_STRIKEOUT

        delete[] pOwnArray;
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteBmp( const BitmapEx& rBmpEx,
                                    const Point& rPt, const Size& rSz,
                                    const Point& rSrcPt, const Size& rSrcSz,
                                    const NMSP_RTL::OUString* pStyle )
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
                const Point                                 aPt( ImplMap( rPt ) );
                const Size                                  aSz( ImplMap( rSz ) );
                FastString                                  aImageData( (sal_Char*) aOStm.GetData(), aOStm.Tell() );
#ifndef _SVG_UNO3
                XExtendedDocumentHandlerRef                 xExtDocHandler( mrExport.GetDocHandler(), USR_QUERY );
#else
                REF( NMSP_SAX::XExtendedDocumentHandler )   xExtDocHandler( mrExport.GetDocHandler(), NMSP_UNO::UNO_QUERY );
#endif

                if( xExtDocHandler.is() )
                {
                    static const sal_uInt32     nPartLen = 64;
                    const NMSP_RTL::OUString    aSpace( ' ' );
                    const NMSP_RTL::OUString    aLineFeed( NMSP_RTL::OUString::valueOf( (sal_Unicode) 0x0a ) );
                    NMSP_RTL::OUString          aString;
                    NMSP_RTL::OUString          aImageString;

                    aString = aLineFeed;
                    aString +=  B2UCONST( "<" );
                    aString += NMSP_RTL::OUString::createFromAscii( aXMLElemImage );
                    aString += aSpace;

                    aString += NMSP_RTL::OUString::createFromAscii( aXMLAttrX );
                    aString += B2UCONST( "=\"" );
                    aString += GetValueString( aPt.X(), mbDoublePoints );
                    aString += B2UCONST( "\" " );

                    aString += NMSP_RTL::OUString::createFromAscii( aXMLAttrY );
                    aString += B2UCONST( "=\"" );
                    aString += GetValueString( aPt.Y(), mbDoublePoints );
                    aString += B2UCONST( "\" " );

                    aString += NMSP_RTL::OUString::createFromAscii( aXMLAttrWidth );
                    aString += B2UCONST( "=\"" );
                    aString += GetValueString( aSz.Width(), mbDoublePoints );
                    aString += B2UCONST( "\" " );

                    aString += NMSP_RTL::OUString::createFromAscii( aXMLAttrHeight );
                    aString += B2UCONST( "=\"" );
                    aString += GetValueString( aSz.Height(), mbDoublePoints );
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

void SVGActionWriter::ImplWriteActions( const GDIMetaFile& rMtf, const NMSP_RTL::OUString* pStyle )
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
                const MetaPixelAction* pA = (const MetaPixelAction*) pAction;

                mpContext->SetPaintAttr( pA->GetColor(), pA->GetColor() );
                ImplWriteLine( pA->GetPoint(), pA->GetPoint(), &pA->GetColor(), pStyle );
            }
            break;

            case( META_POINT_ACTION ):
            {
                const MetaPointAction* pA = (const MetaPointAction*) pAction;

                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetLineColor() );
                ImplWriteLine( pA->GetPoint(), pA->GetPoint(), NULL, pStyle );
            }
            break;

            case( META_LINE_ACTION ):
            {
                const MetaLineAction* pA = (const MetaLineAction*) pAction;

                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetLineColor() );
                ImplWriteLine( pA->GetStartPoint(), pA->GetEndPoint(), NULL, pStyle );
            }
            break;

            case( META_RECT_ACTION ):
            {
                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                ImplWriteRect( ( (const MetaRectAction*) pAction )->GetRect(), 0, 0, pStyle );
            }
            break;

            case( META_ROUNDRECT_ACTION ):
            {
                const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pAction;

                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                ImplWriteRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound(), pStyle );
            }
            break;

            case( META_ELLIPSE_ACTION ):
            {
                const MetaEllipseAction*    pA = (const MetaEllipseAction*) pAction;
                const Rectangle&            rRect = pA->GetRect();

                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                ImplWriteEllipse( rRect.Center(), rRect.GetWidth() >> 1, rRect.GetHeight() >> 1, pStyle );
            }
            break;

            case( META_ARC_ACTION ):
            case( META_PIE_ACTION ):
            case( META_CHORD_ACTION ):
            case( META_POLYGON_ACTION ):
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
                    ImplWritePolygon( aPoly, sal_False, pStyle );
                }
            }
            break;

            case( META_POLYLINE_ACTION ):
            {
                const MetaPolyLineAction*   pA = (const MetaPolyLineAction*) pAction;
                const Polygon&              rPoly = pA->GetPolygon();

                if( rPoly.GetSize() )
                {
                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWritePolygon( rPoly, sal_True, pStyle );
                }
            }
            break;

            case( META_POLYPOLYGON_ACTION ):
            {
                const MetaPolyPolygonAction*    pA = (const MetaPolyPolygonAction*) pAction;
                const PolyPolygon&              rPolyPoly = pA->GetPolyPolygon();

                if( rPolyPoly.Count() )
                {
                    mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWritePolyPolygon( rPolyPoly, sal_False, pStyle );
                }
            }
            break;

            case( META_GRADIENT_ACTION ):
            {
                const MetaGradientAction*   pA = (const MetaGradientAction*) pAction;
                GDIMetaFile                 aTmpMtf;

                mpVDev->AddGradientActions( pA->GetRect(), pA->GetGradient(), aTmpMtf );
                ImplWriteActions( aTmpMtf, pStyle );
            }
            break;

            case( META_GRADIENTEX_ACTION ):
            {
                const MetaGradientExAction* pA = (const MetaGradientExAction*) pAction;
                ImplWriteGradientEx( pA->GetPolyPolygon(), pA->GetGradient(), pStyle );
            }
            break;

            case META_HATCH_ACTION:
            {
                const MetaHatchAction*  pA = (const MetaHatchAction*) pAction;
                GDIMetaFile             aTmpMtf;

                mpVDev->AddHatchActions( pA->GetPolyPolygon(), pA->GetHatch(), aTmpMtf );
                ImplWriteActions( aTmpMtf, pStyle );
            }
            break;

            case( META_TRANSPARENT_ACTION ):
            {
                // !!!
                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                DBG_ERROR( "META_TRANSPARENT_ACTION not supported" );
            }
            break;

            case( META_FLOATTRANSPARENT_ACTION ):
            {
                // !!!
                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                DBG_ERROR( "META_FLOATTRANSPARENT_ACTION not supported" );
            }
            break;

            case( META_EPS_ACTION ):
            {
                const MetaEPSAction*    pA = (const MetaEPSAction*) pAction;
                const GDIMetaFile       aGDIMetaFile( pA->GetSubstitute() );
                sal_Bool                bFound = sal_False;

                for( ULONG i = 0, nCount = aGDIMetaFile.GetActionCount(); ( i < nCount ) && !bFound; i++ )
                {
                    const MetaAction* pSubstAct = aGDIMetaFile.GetAction( i );

                    if( pSubstAct->GetType() == META_BMPSCALE_ACTION )
                    {
                        bFound = sal_True;
                        const MetaBmpScaleAction* pBmpScaleAction = (const MetaBmpScaleAction*) pAction;
                        mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                        ImplWriteBmp( pBmpScaleAction->GetBitmap(),
                                      pBmpScaleAction->GetPoint(), pBmpScaleAction->GetSize(),
                                      Point(), pBmpScaleAction->GetBitmap().GetSizePixel(), pStyle );
                    }
                }
            }
            break;

            case( META_COMMENT_ACTION ):
            {
                const MetaCommentAction*    pA = (const MetaCommentAction*) pAction;
                String                      aSkipComment;

#ifndef _SVG_UNO3
                if( pA->GetComment().ICompare( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL )
#else
                if( pA->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL )
#endif
                {
                    const MetaGradientExAction* pGradAction = NULL;
                    sal_Bool                    bDone = sal_False;

                    while( !bDone && ( ++i < nCount ) )
                    {
                        pAction = rMtf.GetAction( i );

                        if( pAction->GetType() == META_GRADIENTEX_ACTION )
                            pGradAction = (const MetaGradientExAction*) pAction;
                        else if( ( pAction->GetType() == META_COMMENT_ACTION ) &&
#ifndef _SVG_UNO3
                                 ( ( (const MetaCommentAction*) pAction )->GetComment().ICompare( "XGRAD_SEQ_END" ) == COMPARE_EQUAL ) )

#else
                                 ( ( (const MetaCommentAction*) pAction )->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_END" ) == COMPARE_EQUAL ) )
#endif
                        {
                            bDone = sal_True;
                        }
                    }

                    if( pGradAction )
                        ImplWriteGradientEx( pGradAction->GetPolyPolygon(), pGradAction->GetGradient(), pStyle );
                }
            }
            break;

            case( META_BMP_ACTION ):
            {
                const MetaBmpAction* pA = (const MetaBmpAction*) pAction;

                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                ImplWriteBmp( pA->GetBitmap(),
                              pA->GetPoint(), mpVDev->PixelToLogic( pA->GetBitmap().GetSizePixel() ),
                              Point(), pA->GetBitmap().GetSizePixel(), pStyle );
            }
            break;

            case( META_BMPSCALE_ACTION ):
            {
                const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;

                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                ImplWriteBmp( pA->GetBitmap(),
                              pA->GetPoint(), pA->GetSize(),
                              Point(), pA->GetBitmap().GetSizePixel(), pStyle );
            }
            break;

            case( META_BMPSCALEPART_ACTION ):
            {
                const MetaBmpScalePartAction* pA = (const MetaBmpScalePartAction*) pAction;

                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                ImplWriteBmp( pA->GetBitmap(),
                              pA->GetDestPoint(), pA->GetDestSize(),
                              pA->GetSrcPoint(), pA->GetSrcSize(), pStyle );
            }
            break;

            case( META_BMPEX_ACTION ):
            {
                const MetaBmpExAction*  pA = (const MetaBmpExAction*) pAction;

                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                ImplWriteBmp( pA->GetBitmapEx(),
                              pA->GetPoint(), mpVDev->PixelToLogic( pA->GetBitmapEx().GetSizePixel() ),
                              Point(), pA->GetBitmapEx().GetSizePixel(), pStyle );
            }
            break;

            case( META_BMPEXSCALE_ACTION ):
            {
                const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;

                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                ImplWriteBmp( pA->GetBitmapEx(),
                              pA->GetPoint(), pA->GetSize(),
                              Point(), pA->GetBitmapEx().GetSizePixel(), pStyle );
            }
            break;

            case( META_BMPEXSCALEPART_ACTION ):
            {
                const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*) pAction;

                mpContext->SetPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                ImplWriteBmp( pA->GetBitmapEx(),
                              pA->GetDestPoint(), pA->GetDestSize(),
                              pA->GetSrcPoint(), pA->GetSrcSize(), pStyle );
            }
            break;

            case( META_TEXT_ACTION ):
            {
                const MetaTextAction* pA = (const MetaTextAction*) pAction;

                mpContext->SetFontAttr( mpVDev->GetFont() );
                mpContext->SetPaintAttr( COL_TRANSPARENT, mpVDev->GetFont().GetColor() );
                ImplWriteText( pA->GetPoint(), String( pA->GetText(), pA->GetIndex(), pA->GetLen() ), NULL, 0, pStyle );
            }
            break;

            case( META_TEXTRECT_ACTION ):
            {
                const MetaTextRectAction* pA = (const MetaTextRectAction*) pAction;

                mpContext->SetFontAttr( mpVDev->GetFont() );
                mpContext->SetPaintAttr( COL_TRANSPARENT, mpVDev->GetFont().GetColor() );
                ImplWriteText( pA->GetRect().TopLeft(), pA->GetText(), NULL, 0, pStyle );
            }
            break;

            case( META_TEXTARRAY_ACTION ):
            {
                const MetaTextArrayAction*  pA = (const MetaTextArrayAction*) pAction;
                const Point                 aPos( ImplMap( pA->GetPoint() ) );

                mpContext->SetFontAttr( mpVDev->GetFont() );
                mpContext->SetPaintAttr( COL_TRANSPARENT, mpVDev->GetFont().GetColor() );
                ImplWriteText( pA->GetPoint(), String( pA->GetText(), pA->GetIndex(), pA->GetLen() ), pA->GetDXArray(), 0, pStyle );
            }
            break;

            case( META_STRETCHTEXT_ACTION ):
            {
                const MetaStretchTextAction* pA = (const MetaStretchTextAction*) pAction;

                mpContext->SetFontAttr( mpVDev->GetFont() );
                mpContext->SetPaintAttr( COL_TRANSPARENT, mpVDev->GetFont().GetColor() );
                ImplWriteText( pA->GetPoint(), String( pA->GetText(), pA->GetIndex(), pA->GetLen() ), NULL, pA->GetWidth(), pStyle );
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
