/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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


#include "svgfilter.hxx"
#include "svgfontexport.hxx"
#include "svgwriter.hxx"

#include <rtl/crc.h>
#include <vcl/unohelp.hxx>
#include <tools/helpers.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <sax/tools/converter.hxx>

#include <boost/shared_array.hpp>

using ::rtl::OUString;

// -----------
// - statics -
// -----------

static const char   aXMLElemG[] = "g";
static const char   aXMLElemDefs[] = "defs";
static const char   aXMLElemLine[] = "line";
static const char   aXMLElemRect[] = "rect";
static const char   aXMLElemEllipse[] = "ellipse";
static const char   aXMLElemPath[] = "path";
static const char   aXMLElemText[] = "text";
static const char   aXMLElemTspan[] = "tspan";
static const char   aXMLElemImage[] = "image";
static const char   aXMLElemMask[] = "mask";
static const char   aXMLElemPattern[] = "pattern";
static const char   aXMLElemLinearGradient[] = "linearGradient";
static const char   aXMLElemRadialGradient[] = "radialGradient";
static const char   aXMLElemStop[] = "stop";

// -----------------------------------------------------------------------------

static const char   aXMLAttrTransform[] = "transform";
static const char   aXMLAttrStyle[] = "style";
static const char   aXMLAttrId[] = "id";
static const char   aXMLAttrDX[] = "dx";
static const char   aXMLAttrDY[] = "dy";
static const char   aXMLAttrD[] = "d";
static const char   aXMLAttrX[] = "x";
static const char   aXMLAttrY[] = "y";
static const char   aXMLAttrX1[] = "x1";
static const char   aXMLAttrY1[] = "y1";
static const char   aXMLAttrX2[] = "x2";
static const char   aXMLAttrY2[] = "y2";
static const char   aXMLAttrCX[] = "cx";
static const char   aXMLAttrCY[] = "cy";
static const char   aXMLAttrR[] = "r";
static const char   aXMLAttrRX[] = "rx";
static const char   aXMLAttrRY[] = "ry";
static const char   aXMLAttrWidth[] = "width";
static const char   aXMLAttrHeight[] = "height";
static const char   aXMLAttrStroke[] = "stroke";
static const char   aXMLAttrStrokeOpacity[] = "stroke-opacity";
static const char   aXMLAttrStrokeWidth[] = "stroke-width";
static const char   aXMLAttrStrokeDashArray[] = "stroke-dasharray";
static const char   aXMLAttrFill[] = "fill";
static const char   aXMLAttrFillOpacity[] = "fill-opacity";
static const char   aXMLAttrFontFamily[] = "font-family";
static const char   aXMLAttrFontSize[] = "font-size";
static const char   aXMLAttrFontStyle[] = "font-style";
static const char   aXMLAttrFontWeight[] = "font-weight";
static const char   aXMLAttrTextDecoration[] = "text-decoration";
static const char   aXMLAttrXLinkHRef[] = "xlink:href";
static const char   aXMLAttrGradientUnits[] = "gradientUnits";
static const char   aXMLAttrPatternUnits[] = "patternUnits";
static const char   aXMLAttrOffset[] = "offset";
static const char   aXMLAttrStopColor[] = "stop-color";


#define NSPREFIX "ooo:"

static const char aOOOAttrNumberingType[] = NSPREFIX "numbering-type";


static sal_Char const XML_UNO_NAME_NRULE_NUMBERINGTYPE[] = "NumberingType";
static sal_Char const XML_UNO_NAME_NRULE_BULLET_CHAR[] = "BulletChar";
// -----------------------------------------------------------------------------


// ----------------------
// - SVGAttributeWriter -
// ----------------------

SVGAttributeWriter::SVGAttributeWriter( SVGExport& rExport, SVGFontExport& rFontExport ) :
    mrExport( rExport ),
    mrFontExport( rFontExport ),
    mpElemFont( NULL ),
    mpElemPaint( NULL )
{
}

// -----------------------------------------------------------------------------

SVGAttributeWriter::~SVGAttributeWriter()
{
    if( mpElemPaint )
        delete mpElemPaint;
    if( mpElemFont )
        delete mpElemFont;
}

// -----------------------------------------------------------------------------

double SVGAttributeWriter::ImplRound( double fValue, sal_Int32 nDecs )
{
      return( floor( fValue * pow( 10.0, (int)nDecs ) + 0.5 ) / pow( 10.0, (int)nDecs ) );
}

// -----------------------------------------------------------------------------

void SVGAttributeWriter::ImplGetColorStr( const Color& rColor, ::rtl::OUString& rColorStr )
{
    if( rColor.GetTransparency() == 255 )
        rColorStr = B2UCONST( "none" );
    else
    {
        ::rtl::OUStringBuffer aStyle;
        aStyle.appendAscii( "rgb(" );
        aStyle.append( (sal_Int32) rColor.GetRed() );
        aStyle.appendAscii( "," );
        aStyle.append( (sal_Int32) rColor.GetGreen() );
        aStyle.appendAscii( "," );
        aStyle.append( (sal_Int32) rColor.GetBlue() );
        aStyle.appendAscii( ")" );
        rColorStr = aStyle.makeStringAndClear();
    }
}

// -----------------------------------------------------------------------------

void SVGAttributeWriter::AddColorAttr( const char* pColorAttrName,
                                       const char* pColorOpacityAttrName,
                                       const Color& rColor )
{
    ::rtl::OUString aColor, aColorOpacity;

    ImplGetColorStr( rColor, aColor );

    if( rColor.GetTransparency() > 0 && rColor.GetTransparency() < 255 )
        aColorOpacity = ::rtl::OUString::valueOf( ImplRound( ( 255.0 - rColor.GetTransparency() ) / 255.0 ) );

    mrExport.AddAttribute( XML_NAMESPACE_NONE, pColorAttrName, aColor );

    if( !aColorOpacity.isEmpty() && mrExport.IsUseOpacity() )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, pColorOpacityAttrName, aColorOpacity );
}

// -----------------------------------------------------------------------------

void SVGAttributeWriter::AddPaintAttr( const Color& rLineColor, const Color& rFillColor,
                                       const Rectangle* pObjBoundRect, const Gradient* pFillGradient )
{
    // Fill
    if( pObjBoundRect && pFillGradient )
    {
        ::rtl::OUString aGradientId;

        AddGradientDef( *pObjBoundRect, *pFillGradient, aGradientId );

        if( !aGradientId.isEmpty() )
        {
            ::rtl::OUString aGradientURL( B2UCONST( "url(#" ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFill, ( aGradientURL += aGradientId ) += B2UCONST( ")" ) );
        }
    }
    else
        AddColorAttr( aXMLAttrFill, aXMLAttrFillOpacity, rFillColor );

    // Stroke
    AddColorAttr( aXMLAttrStroke, aXMLAttrStrokeOpacity, rLineColor );
}

// -----------------------------------------------------------------------------

void SVGAttributeWriter::AddGradientDef( const Rectangle& rObjRect, const Gradient& rGradient, ::rtl::OUString& rGradientId )
{
    if( rObjRect.GetWidth() && rObjRect.GetHeight() &&
        ( rGradient.GetStyle() == GradientStyle_LINEAR || rGradient.GetStyle() == GradientStyle_AXIAL ||
          rGradient.GetStyle() == GradientStyle_RADIAL || rGradient.GetStyle() == GradientStyle_ELLIPTICAL ) )
    {
        SvXMLElementExport  aDesc( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, sal_True, sal_True );
        Color               aStartColor( rGradient.GetStartColor() ), aEndColor( rGradient.GetEndColor() );
        sal_uInt16          nAngle = rGradient.GetAngle() % 3600;
        Point               aObjRectCenter( rObjRect.Center() );
        Polygon             aPoly( rObjRect );
        static sal_Int32    nCurGradientId = 1;

        aPoly.Rotate( aObjRectCenter, nAngle );
        Rectangle aRect( aPoly.GetBoundRect() );

        // adjust start/end colors with intensities
        aStartColor.SetRed( (sal_uInt8)( (long) aStartColor.GetRed() * rGradient.GetStartIntensity() ) / 100 );
        aStartColor.SetGreen( (sal_uInt8)( (long) aStartColor.GetGreen() * rGradient.GetStartIntensity() ) / 100 );
        aStartColor.SetBlue( (sal_uInt8)( (long) aStartColor.GetBlue() * rGradient.GetStartIntensity() ) / 100 );

        aEndColor.SetRed( (sal_uInt8)( (long) aEndColor.GetRed() * rGradient.GetEndIntensity() ) / 100 );
        aEndColor.SetGreen( (sal_uInt8)( (long) aEndColor.GetGreen() * rGradient.GetEndIntensity() ) / 100 );
        aEndColor.SetBlue( (sal_uInt8)( (long) aEndColor.GetBlue() * rGradient.GetEndIntensity() ) / 100 );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId,
                            ( rGradientId = B2UCONST( "Gradient_" ) ) += ::rtl::OUString::valueOf( nCurGradientId++ ) );

        {
            ::std::auto_ptr< SvXMLElementExport >   apGradient;
            ::rtl::OUString                         aColorStr;

            if( rGradient.GetStyle() == GradientStyle_LINEAR || rGradient.GetStyle() == GradientStyle_AXIAL )
            {
                Polygon aLinePoly( 2 );

                aLinePoly[ 0 ] = Point( aObjRectCenter.X(), aRect.Top() );
                aLinePoly[ 1 ] = Point( aObjRectCenter.X(), aRect.Bottom() );

                aLinePoly.Rotate( aObjRectCenter, nAngle );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrGradientUnits, B2UCONST( "userSpaceOnUse" ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX1, ::rtl::OUString::valueOf( aLinePoly[ 0 ].X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY1, ::rtl::OUString::valueOf( aLinePoly[ 0 ].Y() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX2, ::rtl::OUString::valueOf( aLinePoly[ 1 ].X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY2, ::rtl::OUString::valueOf( aLinePoly[ 1 ].Y() ) );

                apGradient.reset( new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemLinearGradient, sal_True, sal_True ) );

                // write stop values
                double fBorder = static_cast< double >( rGradient.GetBorder() ) *
                                ( ( rGradient.GetStyle() == GradientStyle_AXIAL ) ? 0.005 : 0.01 );

                ImplGetColorStr( ( rGradient.GetStyle() == GradientStyle_AXIAL ) ? aEndColor : aStartColor, aColorStr );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset, ::rtl::OUString::valueOf( fBorder ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStopColor, aColorStr );

                {
                    SvXMLElementExport aDesc2( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, sal_True, sal_True );
                }

                if( rGradient.GetStyle() == GradientStyle_AXIAL )
                {
                    ImplGetColorStr( aStartColor, aColorStr );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset, ::rtl::OUString::valueOf( 0.5 ) );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStopColor, aColorStr );

                    {
                        SvXMLElementExport aDesc3( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, sal_True, sal_True );
                    }
                }

                if( rGradient.GetStyle() != GradientStyle_AXIAL )
                    fBorder = 0.0;

                ImplGetColorStr( aEndColor, aColorStr );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset, ::rtl::OUString::valueOf( ImplRound( 1.0 - fBorder ) ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStopColor, aColorStr );

                {
                    SvXMLElementExport aDesc4( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, sal_True, sal_True );
                }
            }
            else
            {
                const double    fCenterX = rObjRect.Left() + rObjRect.GetWidth() * rGradient.GetOfsX() * 0.01;
                const double    fCenterY = rObjRect.Top() + rObjRect.GetHeight() * rGradient.GetOfsY() * 0.01;
                const double    fRadius = sqrt( static_cast< double >( rObjRect.GetWidth() ) * rObjRect.GetWidth() +
                                                rObjRect.GetHeight() * rObjRect.GetHeight() ) * 0.5;

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrGradientUnits, B2UCONST( "userSpaceOnUse" ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCX, ::rtl::OUString::valueOf( ImplRound( fCenterX ) ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCY, ::rtl::OUString::valueOf( ImplRound( fCenterY ) ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrR, ::rtl::OUString::valueOf( ImplRound( fRadius ) ) );

                apGradient.reset( new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemRadialGradient, sal_True, sal_True ) );

                // write stop values
                ImplGetColorStr( aEndColor, aColorStr );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset, ::rtl::OUString::valueOf( 0.0 ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStopColor, aColorStr );

                {
                    SvXMLElementExport aDesc5( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, sal_True, sal_True );
                }

                ImplGetColorStr( aStartColor, aColorStr );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset,
                                       ::rtl::OUString::valueOf( ImplRound( 1.0 - rGradient.GetBorder() * 0.01 ) ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStopColor, aColorStr );

                {
                    SvXMLElementExport aDesc6( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, sal_True, sal_True );
                }
            }
        }
    }
    else
        rGradientId = ::rtl::OUString();
}

// -----------------------------------------------------------------------------

void SVGAttributeWriter::SetFontAttr( const Font& rFont )
{
    if( rFont != maCurFont )
    {
        ::rtl::OUString  aFontStyle, aFontWeight, aTextDecoration;
        sal_Int32        nFontWeight;

        maCurFont = rFont;

        // Font Family
        setFontFamily();

        // Font Size
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontSize,
                               ::rtl::OUString::valueOf( rFont.GetHeight() ) +  B2UCONST( "px" ) );

        // Font Style
        if( rFont.GetItalic() != ITALIC_NONE )
        {
            if( rFont.GetItalic() == ITALIC_OBLIQUE )
                aFontStyle = B2UCONST( "oblique" );
            else
                aFontStyle = B2UCONST( "italic" );
        }
        else
            aFontStyle = B2UCONST( "normal" );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontStyle, aFontStyle );

        // Font Weight
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

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontWeight, ::rtl::OUString::valueOf( nFontWeight ) );

        if( mrExport.IsUseNativeTextDecoration() )
        {
            if( rFont.GetUnderline() != UNDERLINE_NONE || rFont.GetStrikeout() != STRIKEOUT_NONE )
            {
                if( rFont.GetUnderline() != UNDERLINE_NONE )
                    aTextDecoration = B2UCONST( "underline " );

                if( rFont.GetStrikeout() != STRIKEOUT_NONE )
                    aTextDecoration += B2UCONST( "line-through " );
            }
            else
                aTextDecoration = B2UCONST( "none" );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTextDecoration, aTextDecoration );
        }

        startFontSettings();
    }
}

void SVGAttributeWriter::startFontSettings()
{
    endFontSettings();
    if( mrExport.IsUsePositionedCharacters() )
    {
        mpElemFont = new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemG, sal_True, sal_True );
    }
    else
    {
        mpElemFont = new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, sal_True, sal_True );
    }
}

void SVGAttributeWriter::endFontSettings()
{
    if( mpElemFont )
    {
        delete mpElemFont;
        mpElemFont = NULL;
    }
}

void SVGAttributeWriter::setFontFamily()
{
    if( mrExport.IsUsePositionedCharacters() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontFamily, mrFontExport.GetMappedFontName( maCurFont.GetName() ) );
    }
    else
    {
        sal_Int32       nNextTokenPos( 0 );
        const ::rtl::OUString& rsFontName = maCurFont.GetName();
        ::rtl::OUString sFontFamily( rsFontName.getToken( 0, ';', nNextTokenPos ) );
        FontPitch ePitch = maCurFont.GetPitch();
        if( ePitch == PITCH_FIXED )
        {
            sFontFamily += B2UCONST( ", monospace" );
        }
        else
        {
            FontFamily eFamily = maCurFont.GetFamily();
            if( eFamily == FAMILY_ROMAN )
                sFontFamily += B2UCONST( ", serif" );
            else if( eFamily == FAMILY_SWISS )
                sFontFamily += B2UCONST( ", sans-serif" );
        }
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontFamily, sFontFamily );
    }
}

// -------------------
// - SVGTextWriter -
// -------------------

SVGTextWriter::SVGTextWriter( SVGExport& rExport, SVGFontExport& rFontExport )
    :   mrExport( rExport ),
        mrFontExport( rFontExport ),
        mpContext( NULL ),
        mpVDev( NULL ),
        mbIsTextShapeStarted( sal_False ),
        mrTextShape(),
        msShapeId(),
        mrParagraphEnumeration(),
        mrCurrentTextParagraph(),
        mrTextPortionEnumeration(),
        mrCurrentTextPortion(),
        mpTextEmbeddedBitmapMtf( NULL ),
        mpTargetMapMode( NULL ),
        mpTextShapeElem( NULL ),
        mpTextParagraphElem( NULL ),
        mpTextPositionElem( NULL ),
        mnLeftTextPortionLength( 0 ),
        maTextPos(0,0),
        mnTextWidth(0),
        mbPositioningNeeded( sal_False ),
        mbIsNewListItem( sal_False ),
        maBulletListItemMap(),
        mbIsListLevelStyleImage( sal_False ),
        mbLineBreak( sal_False ),
        mbIsURLField( sal_False ),
        msUrl(),
        mbIsPlacehlolderShape( sal_False ),
        mbIWS( sal_False ),
        maCurrentFont(),
        maParentFont()
{
}

// -----------------------------------------------------------------------------

SVGTextWriter::~SVGTextWriter()
{
    endTextParagraph();
}

// -----------------------------------------------------------------------------

void SVGTextWriter::implRegisterInterface( const Reference< XInterface >& rxIf )
{
    if( rxIf.is() )
        (mrExport.getInterfaceToIdentifierMapper()).registerReference( rxIf );
}

// -----------------------------------------------------------------------------

const ::rtl::OUString & SVGTextWriter::implGetValidIDFromInterface( const Reference< XInterface >& rxIf )
{
   return (mrExport.getInterfaceToIdentifierMapper()).getIdentifier( rxIf );
}

// -----------------------------------------------------------------------------

void SVGTextWriter::implMap( const Size& rSz, Size& rDstSz ) const
{
    if( mpVDev && mpTargetMapMode )
        rDstSz = mpVDev->LogicToLogic( rSz, mpVDev->GetMapMode(), *mpTargetMapMode );
    else
        OSL_FAIL( "SVGTextWriter::implMap: invalid virtual device or map mode." );
}

// -----------------------------------------------------------------------------

void SVGTextWriter::implMap( const Point& rPt, Point& rDstPt ) const
{
    if( mpVDev && mpTargetMapMode )
        rDstPt = mpVDev->LogicToLogic( rPt, mpVDev->GetMapMode(), *mpTargetMapMode );
    else
        OSL_FAIL( "SVGTextWriter::implMap: invalid virtual device or map mode." );
}

// -----------------------------------------------------------------------------

void SVGTextWriter::implSetCurrentFont()
{
    if( mpVDev )
    {
        maCurrentFont = mpVDev->GetFont();
        Size aSz;

        implMap( Size( 0, maCurrentFont.GetHeight() ), aSz );

        maCurrentFont.SetHeight( aSz.Height() );
    }
    else
    {
        OSL_FAIL( "SVGTextWriter::implSetCorrectFontHeight: invalid virtual device." );
    }
}
// -----------------------------------------------------------------------------

template< typename SubType >
sal_Bool SVGTextWriter::implGetTextPosition( const MetaAction* pAction, Point& raPos, sal_Bool& rbEmpty )
{
    const SubType* pA = (const SubType*) pAction;
    sal_uInt16 nLength = pA->GetLen();
    rbEmpty = ( nLength == 0 );
    if( !rbEmpty )
    {
        raPos = pA->GetPoint();
        return sal_True;
    }
    return sal_False;
}

// -----------------------------------------------------------------------------

template<>
sal_Bool SVGTextWriter::implGetTextPosition<MetaTextRectAction>( const MetaAction* pAction, Point& raPos, sal_Bool& rbEmpty )
{
    const MetaTextRectAction* pA = (const MetaTextRectAction*) pAction;
    sal_uInt16 nLength = pA->GetText().getLength();
    rbEmpty = ( nLength == 0 );
    if( !rbEmpty )
    {
        raPos = pA->GetRect().TopLeft();
        return sal_True;
    }
    return sal_False;
}

// -----------------------------------------------------------------------------

template< typename SubType >
sal_Bool SVGTextWriter::implGetTextPositionFromBitmap( const MetaAction* pAction, Point& raPos, sal_Bool& rbEmpty )
{
    const SubType* pA = (const SubType*) pAction;
    raPos = pA->GetPoint();
    rbEmpty = sal_False;
    return sal_True;
}

// -----------------------------------------------------------------------------

/** setTextPosition
 *  Set the start position of the next line of text. In case no text is found
 *  the current action index is updated to the index value we reached while
 *  searching for text.
 *
 *  @returns {sal_Int32}
 *    -2 if no text found and end of line is reached
 *    -1 if no text found and end of paragraph is reached
 *     0 if no text found and end of text shape is reached
 *     1 if text found!
 */
sal_Int32 SVGTextWriter::setTextPosition( const GDIMetaFile& rMtf, sal_uLong& nCurAction )
{
    Point aPos;
    sal_uLong nCount = rMtf.GetActionSize();
    sal_Bool bEOL = sal_False;
    sal_Bool bEOP = sal_False;
    sal_Bool bETS = sal_False;
    sal_Bool bConfigured = sal_False;
    sal_Bool bEmpty = sal_True;

    sal_uLong nActionIndex = nCurAction + 1;
    for( ; nActionIndex < nCount; ++nActionIndex )
    {
        const MetaAction*   pAction = rMtf.GetAction( nActionIndex );
        const sal_uInt16    nType = pAction->GetType();

        switch( nType )
        {
            case( META_TEXT_ACTION ):
            {
                bConfigured = implGetTextPosition<MetaTextAction>( pAction, aPos, bEmpty );
            }
            break;

            case( META_TEXTRECT_ACTION ):
            {
                bConfigured = implGetTextPosition<MetaTextRectAction>( pAction, aPos, bEmpty );
            }
            break;

            case( META_TEXTARRAY_ACTION ):
            {
                bConfigured = implGetTextPosition<MetaTextArrayAction>( pAction, aPos, bEmpty );
            }
            break;

            case( META_STRETCHTEXT_ACTION ):
            {
                bConfigured = implGetTextPosition<MetaStretchTextAction>( pAction, aPos, bEmpty );
            }
            break;

            case( META_BMPSCALE_ACTION ):
            {
                bConfigured = implGetTextPositionFromBitmap<MetaBmpScaleAction>( pAction, aPos, bEmpty );
            }
            break;

            case( META_BMPEXSCALE_ACTION ):
            {
                bConfigured = implGetTextPositionFromBitmap<MetaBmpExScaleAction>( pAction, aPos, bEmpty );
            }
            break;

            // If we reach the end of the current line, paragraph or text shape
            // without finding any text we stop searching
            case( META_COMMENT_ACTION ):
            {
                const MetaCommentAction* pA = (const MetaCommentAction*) pAction;
                const ::rtl::OString& rsComment = pA->GetComment();
                if( rsComment.equalsIgnoreAsciiCaseL( RTL_CONSTASCII_STRINGPARAM( "XTEXT_EOL" ) ) )
                {
                    bEOL = true;
                }
                else if( rsComment.equalsIgnoreAsciiCaseL( RTL_CONSTASCII_STRINGPARAM( "XTEXT_EOP" ) ) )
                {
                    bEOP = true;

                    ::rtl::OUString sContent;
                    while( nextTextPortion() )
                    {
                        sContent = mrCurrentTextPortion->getString();
                        if( sContent.isEmpty() )
                        {
                            continue;
                        }
                        else
                        {
                            if( sContent.equalsAscii( "\n" ) )
                                mbLineBreak = sal_True;
                        }
                    }
                    if( nextParagraph() )
                    {
                        while( nextTextPortion() )
                        {
                            sContent = mrCurrentTextPortion->getString();
                            if( sContent.isEmpty() )
                            {
                                continue;
                            }
                            else
                            {
                                if( sContent.equalsAscii( "\n" ) )
                                    mbLineBreak = sal_True;
                            }
                        }
                    }
                }
                else if( rsComment.equalsIgnoreAsciiCaseL( RTL_CONSTASCII_STRINGPARAM( "XTEXT_PAINTSHAPE_END" ) ) )
                {
                    bETS = true;
                }
            }
            break;
        }
        if( bConfigured || bEOL || bEOP || bETS ) break;
    }
    implMap( aPos, maTextPos );

    if( bEmpty )
    {
        nCurAction = nActionIndex;
        return ( (bEOL) ? -2 : ( (bEOP) ? -1 : 0 ) );
    }
    else
    {
        return 1;
    }
}

// -----------------------------------------------------------------------------

void SVGTextWriter::setTextProperties( const GDIMetaFile& rMtf, sal_uLong nCurAction )
{
    sal_uLong nCount = rMtf.GetActionSize();
    sal_Bool bEOP = sal_False;
    sal_Bool bConfigured = sal_False;
    for( sal_uLong nActionIndex = nCurAction + 1; nActionIndex < nCount; ++nActionIndex )
    {
        const MetaAction*   pAction = rMtf.GetAction( nActionIndex );
        const sal_uInt16    nType = pAction->GetType();
        switch( nType )
        {
            case( META_TEXTLINECOLOR_ACTION ):
            case( META_TEXTFILLCOLOR_ACTION ):
            case( META_TEXTCOLOR_ACTION ):
            case( META_TEXTALIGN_ACTION ):
            case( META_FONT_ACTION ):
            case( META_LAYOUTMODE_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( mpVDev );
            }
            break;

            case( META_TEXT_ACTION ):
            {
                const MetaTextAction* pA = (const MetaTextAction*) pAction;
                if( pA->GetLen() > 2 )
                    bConfigured = true;
            }
            break;
            case( META_TEXTRECT_ACTION ):
            {
                const MetaTextRectAction* pA = (const MetaTextRectAction*) pAction;
                if( pA->GetText().getLength() > 2 )
                    bConfigured = true;
            }
            break;
            case( META_TEXTARRAY_ACTION ):
            {
                const MetaTextArrayAction* pA = (const MetaTextArrayAction*) pAction;
                if( pA->GetLen() > 2 )
                    bConfigured = true;
            }
            break;
            case( META_STRETCHTEXT_ACTION ):
            {
                const MetaStretchTextAction* pA = (const MetaStretchTextAction*) pAction;
                if( pA->GetLen() > 2 )
                    bConfigured = true;
            }
            break;
            // If we reach the end of the paragraph without finding any text
            // we stop searching
            case( META_COMMENT_ACTION ):
            {
                const MetaCommentAction* pA = (const MetaCommentAction*) pAction;
                const ::rtl::OString& rsComment = pA->GetComment();
                if( rsComment.equalsIgnoreAsciiCaseL( RTL_CONSTASCII_STRINGPARAM( "XTEXT_EOP" ) ) )
                {
                    bEOP = true;
                }
            }
            break;
        }
        if( bConfigured || bEOP ) break;
    }
}

// -----------------------------------------------------------------------------

void SVGTextWriter::addFontAttributes( sal_Bool bIsTextContainer )
{
    implSetCurrentFont();

    if( maCurrentFont !=  maParentFont )
    {
        const String& rsCurFontName                 = maCurrentFont.GetName();
        long int nCurFontSize                       = maCurrentFont.GetHeight();
        FontItalic eCurFontItalic                   = maCurrentFont.GetItalic();
        FontWeight eCurFontWeight                   = maCurrentFont.GetWeight();

        const String& rsParFontName                 = maParentFont.GetName();
        long int nParFontSize                       = maParentFont.GetHeight();
        FontItalic eParFontItalic                   = maParentFont.GetItalic();
        FontWeight eParFontWeight                   = maParentFont.GetWeight();


        // Font Family
        if( rsCurFontName != rsParFontName )
        {
            implSetFontFamily();
        }

        // Font Size
        if( nCurFontSize != nParFontSize )
        {
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontSize,
                                   ::rtl::OUString::valueOf( nCurFontSize ) +  B2UCONST( "px" ) );
        }

        // Font Style
        if( eCurFontItalic != eParFontItalic )
        {
            ::rtl::OUString sFontStyle;
            if( eCurFontItalic != ITALIC_NONE )
            {
                if( eCurFontItalic == ITALIC_OBLIQUE )
                    sFontStyle = B2UCONST( "oblique" );
                else
                    sFontStyle = B2UCONST( "italic" );
            }
            else
            {
                sFontStyle = B2UCONST( "normal" );
            }
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontStyle, sFontStyle );
        }

        // Font Weight
        if( eCurFontWeight != eParFontWeight )
        {
            sal_Int32 nFontWeight;
            switch( eCurFontWeight )
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
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontWeight, ::rtl::OUString::valueOf( nFontWeight ) );
        }

        if( bIsTextContainer )
            maParentFont = maCurrentFont;
    }

    if( mrExport.IsUseNativeTextDecoration() )
    {
        FontUnderline eCurFontUnderline         = maCurrentFont.GetUnderline();
        FontStrikeout eCurFontStrikeout         = maCurrentFont.GetStrikeout();

        FontUnderline eParFontUnderline         = maParentFont.GetUnderline();
        FontStrikeout eParFontStrikeout         = maParentFont.GetStrikeout();

        ::rtl::OUString sTextDecoration;

        if( eCurFontUnderline != eParFontUnderline )
        {
            if( eCurFontUnderline != UNDERLINE_NONE )
                sTextDecoration = B2UCONST( "underline " );
        }
        if( eCurFontStrikeout != eParFontStrikeout )
        {
            if( eCurFontStrikeout != STRIKEOUT_NONE )
                sTextDecoration += B2UCONST( "line-through " );
        }
        if( !sTextDecoration.isEmpty() )
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTextDecoration, sTextDecoration );
    }
}

// -----------------------------------------------------------------------------

void SVGTextWriter::implSetFontFamily()
{
    sal_Int32       nNextTokenPos( 0 );
    const ::rtl::OUString& rsFontName = maCurrentFont.GetName();
    ::rtl::OUString sFontFamily( rsFontName.getToken( 0, ';', nNextTokenPos ) );
    FontPitch ePitch = maCurrentFont.GetPitch();
    if( ePitch == PITCH_FIXED )
    {
        sFontFamily += B2UCONST( ", monospace" );
    }
    else
    {
        FontFamily eFamily = maCurrentFont.GetFamily();
        if( eFamily == FAMILY_ROMAN )
            sFontFamily += B2UCONST( ", serif" );
        else if( eFamily == FAMILY_SWISS )
            sFontFamily += B2UCONST( ", sans-serif" );
    }
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontFamily, sFontFamily );
}

// -----------------------------------------------------------------------------

sal_Bool SVGTextWriter::createParagraphEnumeration()
{
    if( mrTextShape.is() )
    {
        Reference< XInterface > xRef( mrTextShape, UNO_QUERY );
        msShapeId = implGetValidIDFromInterface( xRef );

        Reference< XEnumerationAccess > xEnumerationAccess( mrTextShape, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        if( xEnumeration.is() )
        {
            mrParagraphEnumeration.set( xEnumeration );
            return sal_True;
        }
        else
        {
            OSL_FAIL( "SVGTextWriter::createParagraphEnumeration: no valid xEnumeration interface found." );
        }
    }
    else
    {
        OSL_FAIL( "SVGTextWriter::createParagraphEnumeration: no valid XText interface found." );
    }
    return sal_False;
}

// -----------------------------------------------------------------------------

sal_Bool SVGTextWriter::nextParagraph()
{
    mrTextPortionEnumeration.clear();
    mrCurrentTextParagraph.clear();
    mbIsNewListItem = sal_False;
    mbIsListLevelStyleImage = sal_False;

#if OSL_DEBUG_LEVEL > 0
    if( mrParagraphEnumeration.is() && mrParagraphEnumeration->hasMoreElements() )
    {
        Reference < XTextContent >  xTextContent( mrParagraphEnumeration->nextElement(), UNO_QUERY_THROW );
        if( xTextContent.is() )
        {
            Reference< XServiceInfo > xServiceInfo( xTextContent, UNO_QUERY_THROW );
            if( xServiceInfo.is() )
            {
                OUString sInfo;
                if( xServiceInfo->supportsService( B2UCONST( "com.sun.star.text.Paragraph" ) ) )
                {
                    mrCurrentTextParagraph.set( xTextContent );
                    Reference< XPropertySet > xPropSet( xTextContent, UNO_QUERY_THROW );
                    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
                    if( xPropSetInfo->hasPropertyByName( B2UCONST( "NumberingLevel" ) ) )
                    {
                        sal_Int16 nListLevel = 0;
                        if( xPropSet->getPropertyValue( B2UCONST( "NumberingLevel" ) ) >>= nListLevel )
                        {
                            mbIsNewListItem = sal_True;
                            sInfo = B2UCONST( "NumberingLevel: " );
                            sInfo += OUString::valueOf( (sal_Int32)nListLevel );
                            mrExport.AddAttribute( XML_NAMESPACE_NONE, "style", sInfo );

                            Reference< XIndexReplace > xNumRules;
                            if( xPropSetInfo->hasPropertyByName( B2UCONST( "NumberingRules" ) ) )
                            {
                                xPropSet->getPropertyValue( B2UCONST( "NumberingRules" ) ) >>= xNumRules;
                            }
                            if( xNumRules.is() && ( nListLevel < xNumRules->getCount() ) )
                            {
                                sal_Bool bIsNumbered = sal_True;
                                OUString msNumberingIsNumber(RTL_CONSTASCII_USTRINGPARAM("NumberingIsNumber"));
                                if( xPropSetInfo->hasPropertyByName( msNumberingIsNumber ) )
                                {
                                    if( !(xPropSet->getPropertyValue( msNumberingIsNumber ) >>= bIsNumbered ) )
                                    {
                                        OSL_FAIL( "numbered paragraph without number info" );
                                        bIsNumbered = sal_False;
                                    }
                                    if( bIsNumbered )
                                    {
                                        sInfo = B2UCONST( "true" );
                                        mrExport.AddAttribute( XML_NAMESPACE_NONE, "is-numbered", sInfo );
                                    }
                                }
                                mbIsNewListItem = bIsNumbered;

                                if( bIsNumbered )
                                {
                                    Sequence<PropertyValue> aProps;
                                    if( xNumRules->getByIndex( nListLevel ) >>= aProps )
                                    {
                                        sal_Int16 eType = NumberingType::CHAR_SPECIAL;
                                        sal_Unicode cBullet = 0xf095;
                                        const sal_Int32 nCount = aProps.getLength();
                                        const PropertyValue* pPropArray = aProps.getConstArray();
                                        for( sal_Int32 i = 0; i < nCount; ++i )
                                        {
                                            const PropertyValue& rProp = pPropArray[i];
                                            if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_NUMBERINGTYPE, sizeof(XML_UNO_NAME_NRULE_NUMBERINGTYPE)-1 ) )
                                            {
                                                rProp.Value >>= eType;
                                            }
                                            else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_BULLET_CHAR, sizeof(XML_UNO_NAME_NRULE_BULLET_CHAR)-1 ) )
                                            {
                                                OUString sValue;
                                                rProp.Value >>= sValue;
                                                if( !sValue.isEmpty() )
                                                {
                                                    cBullet = (sal_Unicode)sValue[0];
                                                }
                                            }
                                        }
                                        meNumberingType = eType;
                                        mbIsListLevelStyleImage = ( NumberingType::BITMAP == meNumberingType );
                                        if( NumberingType::CHAR_SPECIAL == meNumberingType )
                                        {
                                            if( cBullet )
                                            {
                                                if( cBullet < ' ' )
                                                {
                                                    cBullet = 0xF000 + 149;
                                                }
                                                mcBulletChar = cBullet;
                                                sInfo = OUString::valueOf( (sal_Int32) cBullet );
                                                mrExport.AddAttribute( XML_NAMESPACE_NONE, "bullet-char", sInfo );
                                            }

                                        }
                                    }
                                }
                            }

                        }
                    }

                    Reference< XEnumerationAccess > xEnumerationAccess( xTextContent, UNO_QUERY_THROW );
                    Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
                    if( xEnumeration.is() && xEnumeration->hasMoreElements() )
                    {
                        mrTextPortionEnumeration.set( xEnumeration );
                    }
                    sInfo = B2UCONST( "Paragraph" );
                }
                else if( xServiceInfo->supportsService( B2UCONST( "com.sun.star.text.Table" ) ) )
                {
                    OSL_FAIL( "SVGTextWriter::nextParagraph: text tables are not handled." );
                    sInfo = B2UCONST( "Table" );
                }
                else
                {
                    OSL_FAIL( "SVGTextWriter::nextParagraph: Unknown text content." );
                    return sal_False;
                }
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", sInfo );
                SvXMLElementExport aParaElem( mrExport, XML_NAMESPACE_NONE, "desc", mbIWS, mbIWS );
            }
            else
            {
                OSL_FAIL( "SVGTextWriter::nextParagraph: no XServiceInfo interface available for text content." );
                return sal_False;
            }

            Reference< XInterface > xRef( xTextContent, UNO_QUERY );
            const OUString& rParagraphId = implGetValidIDFromInterface( xRef );
            if( !rParagraphId.isEmpty() )
            {
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", rParagraphId );
            }
            return sal_True;
        }
    }
#else
    if( mrParagraphEnumeration.is() && mrParagraphEnumeration->hasMoreElements() )
    {
        Reference < XTextContent >  xTextContent( mrParagraphEnumeration->nextElement(), UNO_QUERY_THROW );
        if( xTextContent.is() )
        {
            Reference< XServiceInfo > xServiceInfo( xTextContent, UNO_QUERY_THROW );
            if( xServiceInfo.is() )
            {
                if( xServiceInfo->supportsService( B2UCONST( "com.sun.star.text.Paragraph" ) ) )
                {
                    mrCurrentTextParagraph.set( xTextContent );
                    Reference< XPropertySet > xPropSet( xTextContent, UNO_QUERY_THROW );
                    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
                    if( xPropSetInfo->hasPropertyByName( B2UCONST( "NumberingLevel" ) ) )
                    {
                        sal_Int16 nListLevel = 0;
                        if( xPropSet->getPropertyValue( B2UCONST( "NumberingLevel" ) ) >>= nListLevel )
                        {
                            mbIsNewListItem = sal_True;

                            Reference< XIndexReplace > xNumRules;
                            if( xPropSetInfo->hasPropertyByName( B2UCONST( "NumberingRules" ) ) )
                            {
                                xPropSet->getPropertyValue( B2UCONST( "NumberingRules" ) ) >>= xNumRules;
                            }
                            if( xNumRules.is() && ( nListLevel < xNumRules->getCount() ) )
                            {
                                sal_Bool bIsNumbered = sal_True;
                                OUString msNumberingIsNumber(RTL_CONSTASCII_USTRINGPARAM("NumberingIsNumber"));
                                if( xPropSetInfo->hasPropertyByName( msNumberingIsNumber ) )
                                {
                                    if( !(xPropSet->getPropertyValue( msNumberingIsNumber ) >>= bIsNumbered ) )
                                    {
                                        OSL_FAIL( "numbered paragraph without number info" );
                                        bIsNumbered = sal_False;
                                    }
                                }
                                mbIsNewListItem = bIsNumbered;

                                if( bIsNumbered )
                                {
                                    Sequence<PropertyValue> aProps;
                                    if( xNumRules->getByIndex( nListLevel ) >>= aProps )
                                    {
                                        sal_Int16 eType = NumberingType::CHAR_SPECIAL;
                                        sal_Unicode cBullet = 0xf095;
                                        const sal_Int32 nCount = aProps.getLength();
                                        const PropertyValue* pPropArray = aProps.getConstArray();
                                        for( sal_Int32 i = 0; i < nCount; ++i )
                                        {
                                            const PropertyValue& rProp = pPropArray[i];
                                            if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_NUMBERINGTYPE, sizeof(XML_UNO_NAME_NRULE_NUMBERINGTYPE)-1 ) )
                                            {
                                                rProp.Value >>= eType;
                                            }
                                            else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_BULLET_CHAR, sizeof(XML_UNO_NAME_NRULE_BULLET_CHAR)-1 ) )
                                            {
                                                OUString sValue;
                                                rProp.Value >>= sValue;
                                                if( !sValue.isEmpty() )
                                                {
                                                    cBullet = (sal_Unicode)sValue[0];
                                                }
                                            }
                                        }
                                        meNumberingType = eType;
                                        mbIsListLevelStyleImage = ( NumberingType::BITMAP == meNumberingType );
                                        if( NumberingType::CHAR_SPECIAL == meNumberingType )
                                        {
                                            if( cBullet )
                                            {
                                                if( cBullet < ' ' )
                                                {
                                                    cBullet = 0xF000 + 149;
                                                }
                                                mcBulletChar = cBullet;
                                            }

                                        }
                                    }
                                }
                            }

                        }
                    }

                    Reference< XEnumerationAccess > xEnumerationAccess( xTextContent, UNO_QUERY_THROW );
                    Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
                    if( xEnumeration.is() && xEnumeration->hasMoreElements() )
                    {
                        mrTextPortionEnumeration.set( xEnumeration );
                    }
                }
                else if( xServiceInfo->supportsService( B2UCONST( "com.sun.star.text.Table" ) ) )
                {
                    OSL_FAIL( "SVGTextWriter::nextParagraph: text tables are not handled." );
                }
                else
                {
                    OSL_FAIL( "SVGTextWriter::nextParagraph: Unknown text content." );
                    return sal_False;
                }
            }
            else
            {
                OSL_FAIL( "SVGTextWriter::nextParagraph: no XServiceInfo interface available for text content." );
                return sal_False;
            }

            Reference< XInterface > xRef( xTextContent, UNO_QUERY );
            const OUString& rParagraphId = implGetValidIDFromInterface( xRef );
            if( !rParagraphId.isEmpty() )
            {
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", rParagraphId );
            }
            return sal_True;
        }
    }
#endif
    return sal_False;
}

// -----------------------------------------------------------------------------

sal_Bool SVGTextWriter::nextTextPortion()
{
    mrCurrentTextPortion.clear();
    mbIsURLField = sal_False;
    mbIsPlacehlolderShape = sal_False;
#if OSL_DEBUG_LEVEL > 0
    if( mrTextPortionEnumeration.is() && mrTextPortionEnumeration->hasMoreElements() )
    {
        OUString sInfo;
        Reference< XPropertySet > xPortionPropSet( mrTextPortionEnumeration->nextElement(), UNO_QUERY );
        Reference< XPropertySetInfo > xPortionPropInfo( xPortionPropSet->getPropertySetInfo() );
        Reference < XTextRange > xPortionTextRange( xPortionPropSet, UNO_QUERY);
        if( xPortionPropSet.is() && xPortionPropInfo.is()
                && xPortionPropInfo->hasPropertyByName( B2UCONST( "TextPortionType" ) ) )
        {
            ::rtl::OUString sPortionType;
            if( xPortionPropSet->getPropertyValue( B2UCONST( "TextPortionType" ) ) >>= sPortionType )
            {
                sInfo = B2UCONST( "type: " );
                sInfo += sPortionType;
                sInfo += B2UCONST( "; " );
            }
            if( xPortionTextRange.is() )
            {
                sInfo += B2UCONST( "content: " );
                sInfo += xPortionTextRange->getString();
                sInfo += B2UCONST( "; " );

                mrCurrentTextPortion.set( xPortionTextRange );

                Reference < XPropertySet > xRangePropSet( xPortionTextRange, UNO_QUERY );
                if( xRangePropSet.is() && xRangePropSet->getPropertySetInfo()->hasPropertyByName( B2UCONST( "TextField" ) ) )
                {
                    Reference < XTextField > xTextField( xRangePropSet->getPropertyValue( B2UCONST( "TextField" ) ), UNO_QUERY );
                    if( xTextField.is() )
                    {
                        const ::rtl::OUString sServicePrefix( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.textfield.") );
                        const ::rtl::OUString sPresentationServicePrefix( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.TextField.") );

                        Reference< XServiceInfo > xService( xTextField, UNO_QUERY );
                        const Sequence< OUString > aServices = xService->getSupportedServiceNames();

                        const OUString* pNames = aServices.getConstArray();
                        sal_Int32 nCount = aServices.getLength();

                        OUString sFieldName;    // service name postfix of current field

                        // search for TextField service name
                        while( nCount-- )
                        {
                            if ( pNames->matchIgnoreAsciiCase( sServicePrefix ) )
                            {
                                // TextField found => postfix is field type!
                                sFieldName = pNames->copy( sServicePrefix.getLength() );
                                break;
                            }
                            else if( 0 == pNames->compareTo( sPresentationServicePrefix, sPresentationServicePrefix.getLength() ) )
                            {
                                // TextField found => postfix is field type!
                                sFieldName = pNames->copy( sPresentationServicePrefix.getLength() );
                                break;
                            }

                            ++pNames;
                        }

                        sInfo += B2UCONST( "text field type: " );
                        sInfo += sFieldName;
                        sInfo += B2UCONST( "; " );

                        sInfo += B2UCONST( "content: " );
                        sInfo += xTextField->getPresentation( /* show command: */ sal_False );
                        sInfo += B2UCONST( "; " );

                        if( sFieldName.equalsAscii( "DateTime" ) || sFieldName.equalsAscii( "Header" )
                                || sFieldName.equalsAscii( "Footer" ) || sFieldName.equalsAscii( "PageNumber" ) )
                        {
                            mbIsPlacehlolderShape = sal_True;
                        }
                        else
                        {
                            mbIsURLField = sFieldName.equalsAscii( "URL" );

                            if( mbIsURLField )
                            {
                                Reference<XPropertySet> xTextFieldPropSet(xTextField, UNO_QUERY);
                                if( xTextFieldPropSet.is() )
                                {
                                    OUString sURL;
                                    if( ( xTextFieldPropSet->getPropertyValue( sFieldName ) ) >>= sURL )
                                    {
                                        sInfo += B2UCONST( "url: " );
                                        sInfo += mrExport.GetRelativeReference( sURL );

                                        msUrl = mrExport.GetRelativeReference( sURL );
                                        if( !msUrl.isEmpty() )
                                        {
                                            implRegisterInterface( xPortionTextRange );

                                            Reference< XInterface > xRef( xPortionTextRange, UNO_QUERY );
                                            const OUString& rTextPortionId = implGetValidIDFromInterface( xRef );
                                            if( !rTextPortionId.isEmpty() )
                                            {
                                                msHyperlinkIdList += rTextPortionId;
                                                msHyperlinkIdList += B2UCONST( " " );
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "TextPortion" ) );
            SvXMLElementExport aPortionElem( mrExport, XML_NAMESPACE_NONE, "desc", mbIWS, mbIWS );
            mrExport.GetDocHandler()->characters( sInfo );
            return sal_True;
        }
    }
#else
    if( mrTextPortionEnumeration.is() && mrTextPortionEnumeration->hasMoreElements() )
    {
        OUString sInfo;
        Reference< XPropertySet > xPortionPropSet( mrTextPortionEnumeration->nextElement(), UNO_QUERY );
        Reference< XPropertySetInfo > xPortionPropInfo( xPortionPropSet->getPropertySetInfo() );
        Reference < XTextRange > xPortionTextRange( xPortionPropSet, UNO_QUERY);
        if( xPortionPropSet.is() && xPortionPropInfo.is()
                && xPortionPropInfo->hasPropertyByName( B2UCONST( "TextPortionType" ) ) )
        {
            if( xPortionTextRange.is() )
            {
                mrCurrentTextPortion.set( xPortionTextRange );

                Reference < XPropertySet > xRangePropSet( xPortionTextRange, UNO_QUERY );
                if( xRangePropSet.is() && xRangePropSet->getPropertySetInfo()->hasPropertyByName( B2UCONST( "TextField" ) ) )
                {
                    Reference < XTextField > xTextField( xRangePropSet->getPropertyValue( B2UCONST( "TextField" ) ), UNO_QUERY );
                    if( xTextField.is() )
                    {
                        const ::rtl::OUString sServicePrefix( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.textfield.") );
                        const ::rtl::OUString sPresentationServicePrefix( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.TextField.") );

                        Reference< XServiceInfo > xService( xTextField, UNO_QUERY );
                        const Sequence< OUString > aServices = xService->getSupportedServiceNames();

                        const OUString* pNames = aServices.getConstArray();
                        sal_Int32 nCount = aServices.getLength();

                        OUString sFieldName;    // service name postfix of current field

                        // search for TextField service name
                        while( nCount-- )
                        {
                            if ( pNames->matchIgnoreAsciiCase( sServicePrefix ) )
                            {
                                // TextField found => postfix is field type!
                                sFieldName = pNames->copy( sServicePrefix.getLength() );
                                break;
                            }
                            else if( 0 == pNames->compareTo( sPresentationServicePrefix, sPresentationServicePrefix.getLength() ) )
                            {
                                // TextField found => postfix is field type!
                                sFieldName = pNames->copy( sPresentationServicePrefix.getLength() );
                                break;
                            }

                            ++pNames;
                        }

                        if( sFieldName.equalsAscii( "DateTime" ) || sFieldName.equalsAscii( "Header" )
                                || sFieldName.equalsAscii( "Footer" ) || sFieldName.equalsAscii( "PageNumber" ) )
                        {
                            mbIsPlacehlolderShape = sal_True;
                        }
                        else
                        {
                            mbIsURLField = sFieldName.equalsAscii( "URL" );
                            if( mbIsURLField )
                            {
                                Reference<XPropertySet> xTextFieldPropSet(xTextField, UNO_QUERY);
                                if( xTextFieldPropSet.is() )
                                {
                                    OUString sURL;
                                    if( ( xTextFieldPropSet->getPropertyValue( sFieldName ) ) >>= sURL )
                                    {
                                        msUrl = mrExport.GetRelativeReference( sURL );
                                        if( !msUrl.isEmpty() )
                                        {
                                            implRegisterInterface( xPortionTextRange );

                                            Reference< XInterface > xRef( xPortionTextRange, UNO_QUERY );
                                            const OUString& rTextPortionId = implGetValidIDFromInterface( xRef );
                                            if( !rTextPortionId.isEmpty() )
                                            {
                                                msHyperlinkIdList += rTextPortionId;
                                                msHyperlinkIdList += B2UCONST( " " );
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return sal_True;
        }
    }
#endif
    return sal_False;
}

// -----------------------------------------------------------------------------

void SVGTextWriter::startTextShape()
{
    if( mpTextShapeElem )
    {
        OSL_FAIL( "SVGTextWriter::startTextShape: text shape already defined." );
    }

    {
        mbIsTextShapeStarted = sal_True;
        maParentFont = Font();
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "TextShape" ) );
        mpTextShapeElem = new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemText, sal_True, mbIWS );
        startTextParagraph();
    }
}

// -----------------------------------------------------------------------------

void SVGTextWriter::endTextShape()
{
    endTextParagraph();
    if( mrTextShape.is() )
        mrTextShape.clear();
    if( mrParagraphEnumeration.is() )
        mrParagraphEnumeration.clear();
    if( mrCurrentTextParagraph.is() )
        mrCurrentTextParagraph.clear();
    if( mpTextShapeElem )
    {
        delete mpTextShapeElem;
        mpTextShapeElem = NULL;
    }
    mbIsTextShapeStarted = sal_False;
    // these need to be invoked after the <text> element has been closed
    implExportHyperlinkIds();
    implWriteBulletChars();
    implWriteEmbeddedBitmaps();

}

// -----------------------------------------------------------------------------

void SVGTextWriter::startTextParagraph()
{
    endTextParagraph();
    nextParagraph();
    if( mbIsNewListItem )
    {
        OUString sNumberingType;
        switch( meNumberingType )
        {
            case( NumberingType::CHAR_SPECIAL ):
                    sNumberingType = B2UCONST( "bullet-style" );
                    break;
            case( NumberingType::BITMAP ):
                    sNumberingType = B2UCONST( "image-style" );
                    break;
            default:
                    sNumberingType = B2UCONST( "number-style" );
                    break;
        }
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aOOOAttrNumberingType, sNumberingType );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "ListItem" ) );
    }
    else
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "TextParagraph" ) );
    }
    maParentFont = Font();
    addFontAttributes( /* isTexTContainer: */ true );
    mpTextParagraphElem = new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, mbIWS, mbIWS );
    if( !mbIsListLevelStyleImage )
    {
        startTextPosition();
    }
}

// -----------------------------------------------------------------------------

void SVGTextWriter::endTextParagraph()
{
    mrCurrentTextPortion.clear();
    endTextPosition();
    mbIsNewListItem = sal_False;
    mbIsListLevelStyleImage = sal_False;
    mbPositioningNeeded = sal_False;

    if( mpTextParagraphElem )
    {
        delete mpTextParagraphElem;
        mpTextParagraphElem = NULL;
    }

}

// -----------------------------------------------------------------------------

void SVGTextWriter::startTextPosition( sal_Bool bExportX, sal_Bool bExportY )
{
    endTextPosition();
    mnTextWidth = 0;
    mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "TextPosition" ) );
    if( bExportX )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, ::rtl::OUString::valueOf( maTextPos.X() ) );
    if( bExportY )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, ::rtl::OUString::valueOf( maTextPos.Y() ) );
    mpTextPositionElem = new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, mbIWS, mbIWS );
}

// -----------------------------------------------------------------------------

void SVGTextWriter::endTextPosition()
{
    if( mpTextPositionElem )
    {
        delete mpTextPositionElem;
        mpTextPositionElem = NULL;
    }
}

// -----------------------------------------------------------------------------

void SVGTextWriter::implExportHyperlinkIds()
{
    if( !msHyperlinkIdList.isEmpty() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "HyperlinkIdList" ) );
        SvXMLElementExport aDescElem( mrExport, XML_NAMESPACE_NONE, "desc", sal_True, sal_False );
        mrExport.GetDocHandler()->characters( msHyperlinkIdList.trim() );
        msHyperlinkIdList = OUString();
    }
}

// -----------------------------------------------------------------------------

void SVGTextWriter::implWriteBulletChars()
{
    if( maBulletListItemMap.empty() )
        return;

    mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "BulletChars" ) );
    SvXMLElementExport aGroupElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, sal_True, sal_True );

    BulletListItemInfoMap::const_iterator it = maBulletListItemMap.begin();
    BulletListItemInfoMap::const_iterator end = maBulletListItemMap.end();
    OUString sId, sPosition, sScaling, sRefId;
    for( ; it != end; ++it )
    {
        // <g id="?" > (used by animations)
        {
            // As id we use the id of the text portion placeholder wrapped
            // by bullet-char(*)
            sId = B2UCONST( "bullet-char(" );
            sId += it->first;
            sId += B2UCONST( ")" );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", sId );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "BulletChar" ) );
            SvXMLElementExport aBulletCharElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, sal_True, sal_True );

            // <g transform="translate(x,y)" >
            {
                const BulletListItemInfo& rInfo = it->second;

                // Add positioning attribute through a translation
                sPosition = B2UCONST( "translate(" );
                sPosition += OUString::valueOf( rInfo.aPos.X() );
                sPosition += B2UCONST( "," );
                sPosition += OUString::valueOf( rInfo.aPos.Y() );
                sPosition += B2UCONST( ")" );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "transform", sPosition );

                mpContext->AddPaintAttr( COL_TRANSPARENT, rInfo.aColor );

                SvXMLElementExport aPositioningElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, sal_True, sal_True );

                // <use transform="scale(font-size)" xlink:ref="/" >
                {
                    // Add size attribute through a scaling
                    sScaling = B2UCONST( "scale(" );
                    sScaling += OUString::valueOf( rInfo.nFontSize );
                    sScaling += B2UCONST( "," );
                    sScaling += OUString::valueOf( rInfo.nFontSize );
                    sScaling += B2UCONST( ")" );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "transform", sScaling );

                    // Add ref attribute
                    sRefId = B2UCONST( "#bullet-char-template(" );
                    sRefId += OUString::valueOf( (sal_Int32)( rInfo.cBulletChar )  );
                    sRefId += B2UCONST( ")" );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrXLinkHRef, sRefId );

                    SvXMLElementExport aRefElem( mrExport, XML_NAMESPACE_NONE, "use", sal_True, sal_True );
                }
            } // close aPositioningElem
        } // close aBulletCharElem
    }


    // clear the map
    maBulletListItemMap.clear();
}

// -----------------------------------------------------------------------------

template< typename MetaBitmapActionType >
void SVGTextWriter::writeBitmapPlaceholder( const MetaBitmapActionType* pAction )
{
    // text position element
    const Point& rPos = pAction->GetPoint();
    implMap( rPos, maTextPos );
    startTextPosition();
    mbPositioningNeeded = sal_True;
    if( mbIsNewListItem )
    {
        mbIsNewListItem = sal_False;
        mbIsListLevelStyleImage = sal_False;
    }

    // bitmap placeholder element
    sal_uLong nId = SVGActionWriter::GetChecksum( pAction );
    OUString sId = B2UCONST( "bitmap-placeholder(" );
    sId += msShapeId;
    sId += B2UCONST( "." );
    sId += OUString::valueOf( (sal_Int64)nId );
    sId += B2UCONST( ")" );

    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", sId );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "BitmapPlaceholder" ) );
        SvXMLElementExport aSVGTspanElem( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, mbIWS, mbIWS );
    }
    endTextPosition();
}

// -----------------------------------------------------------------------------

void SVGTextWriter::implWriteEmbeddedBitmaps()
{
    if( mpTextEmbeddedBitmapMtf && mpTextEmbeddedBitmapMtf->GetActionSize() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "EmbeddedBitmaps" ) );
        SvXMLElementExport aEmbBitmapGroupElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, sal_True, sal_True );

        const GDIMetaFile& rMtf = *mpTextEmbeddedBitmapMtf;

        OUString sId, sRefId;
        sal_uLong nId, nChecksum = 0;
        Point aPt;
        Size  aSz;
        sal_uLong nCount = rMtf.GetActionSize();
        for( sal_uLong nCurAction = 0; nCurAction < nCount; nCurAction++ )
        {

            const MetaAction* pAction = rMtf.GetAction( nCurAction );
            const sal_uInt16 nType = pAction->GetType();

            switch( nType )
            {
                case( META_BMPSCALE_ACTION ):
                {
                    const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;
                    nChecksum = pA->GetBitmap().GetChecksum();
                    aPt = pA->GetPoint();
                    aSz = pA->GetSize();
                }
                break;
                case( META_BMPEXSCALE_ACTION ):
                {
                    const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;
                    nChecksum = pA->GetBitmapEx().GetChecksum();
                    aPt = pA->GetPoint();
                    aSz = pA->GetSize();
                }
                break;
            }

            // <g id="?" > (used by animations)
            {
                // embedded bitmap id
                nId = SVGActionWriter::GetChecksum( pAction );
                sId = B2UCONST( "embedded-bitmap(" );
                sId += msShapeId;
                sId += B2UCONST( "." );
                sId += OUString::valueOf( (sal_Int64)nId );
                sId += B2UCONST( ")" );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", sId );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "EmbeddedBitmap" ) );

                SvXMLElementExport aEmbBitmapElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, sal_True, sal_True );

                // <use x="?" y="?" xlink:ref="?" >
                {
                    // referenced bitmap template
                    sRefId = B2UCONST( "#bitmap(" );
                    sRefId += OUString::valueOf( (sal_Int64)nChecksum );
                    sRefId += B2UCONST( ")" );

                    Point aPoint;
                    Size  aSize;
                    implMap( aPt, aPoint );
                    implMap( aSz, aSize );

                    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, OUString::valueOf( aPoint.X() ) );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, OUString::valueOf( aPoint.Y() ) );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrXLinkHRef, sRefId );

                    SvXMLElementExport aRefElem( mrExport, XML_NAMESPACE_NONE, "use", sal_True, sal_True );
                }
            } // close aEmbBitmapElem
        }
    }
}

// -----------------------------------------------------------------------------

void SVGTextWriter::writeTextPortion( const Point& rPos,
                                      const String& rText,
                                      sal_Bool bApplyMapping )
{
    if( rText.Len() == 0 )
        return;

    mbLineBreak = sal_False;

    if( !mbIsNewListItem || mbIsListLevelStyleImage )
    {
        bool bNotSync = true;
        OUString sContent;
        sal_Int32 nStartPos;
        while( bNotSync )
        {
            if( mnLeftTextPortionLength <= 0 )
            {
                if( !nextTextPortion() )
                    break;
                else
                {
                    sContent = mrCurrentTextPortion->getString();
                    if( mbIsURLField && sContent.isEmpty() )
                    {
                        Reference < XPropertySet > xPropSet( mrCurrentTextPortion, UNO_QUERY );
                        Reference < XTextField > xTextField( xPropSet->getPropertyValue( B2UCONST( "TextField" ) ), UNO_QUERY );
                        sContent = xTextField->getPresentation( /* show command: */ sal_False );
                        if( sContent.isEmpty() )
                            OSL_FAIL( "SVGTextWriter::writeTextPortion: content of URL TextField is empty." );
                    }
                    mnLeftTextPortionLength = sContent.getLength();
                }
            }
            else
            {
                sContent = mrCurrentTextPortion->getString();
            }

            nStartPos = sContent.getLength() - mnLeftTextPortionLength;
            if( nStartPos < 0 ) nStartPos = 0;
            mnLeftTextPortionLength -= rText.Len();

            if( sContent.isEmpty() )
                continue;
            if( sContent.equalsAscii( "\n" ) )
                mbLineBreak = sal_True;
            if( sContent.match( rText, nStartPos ) )
                bNotSync = false;
        }
    }

    if( !mpVDev )
        OSL_FAIL( "SVGTextWriter::writeTextPortion: invalid virtual device." );

    const FontMetric aMetric( mpVDev->GetFontMetric() );

    bool bTextSpecial = aMetric.IsShadow() || aMetric.IsOutline() || (aMetric.GetRelief() != RELIEF_NONE);

    if( true || !bTextSpecial )
    {
        implWriteTextPortion( rPos, rText, mpVDev->GetTextColor(), bApplyMapping );
    }
    else
    {
        // to be implemented
    }

}

// -----------------------------------------------------------------------------

void SVGTextWriter::implWriteTextPortion( const Point& rPos,
                                          const String& rText,
                                          Color aTextColor,
                                          sal_Bool bApplyMapping )
{
    if( !mpContext )
        OSL_FAIL( "SVGTextWriter::implWriteTextPortion: invalid context object." );

    Point                                   aPos;
    Point                                   aBaseLinePos( rPos );
    const FontMetric                        aMetric( mpVDev->GetFontMetric() );
    const Font&                             rFont = mpVDev->GetFont();

    if( rFont.GetAlign() == ALIGN_TOP )
        aBaseLinePos.Y() += aMetric.GetAscent();
    else if( rFont.GetAlign() == ALIGN_BOTTOM )
        aBaseLinePos.Y() -= aMetric.GetDescent();

    if( bApplyMapping )
        implMap( rPos, aPos );
    else
        aPos = rPos;

    if( mbPositioningNeeded )
    {
        mbPositioningNeeded = sal_False;
        maTextPos.setX( aPos.X() );
        maTextPos.setY( aPos.Y() );
        startTextPosition();
    }
    else if( maTextPos.Y() != aPos.Y() )
    {
        // In case the text position moved backward we could have a line break
        // so we end the current line and start a new one.
        if( mbLineBreak || ( ( maTextPos.X() + mnTextWidth ) > aPos.X() ) )
        {
            mbLineBreak = sal_False;
            maTextPos.setX( aPos.X() );
            maTextPos.setY( aPos.Y() );
            startTextPosition();
        }
        else // superscript, subscript, list item numbering
        {
            maTextPos.setY( aPos.Y() );
            startTextPosition( sal_False /* do not export x attribute */ );
        }
    }
    // we are dealing with a bullet, so set up this for the next text portion
    if( mbIsNewListItem )
    {
        mbIsNewListItem = sal_False;
        mbPositioningNeeded = sal_True;

        if( meNumberingType == NumberingType::CHAR_SPECIAL )
        {
            // Create an id for the current text portion
            implRegisterInterface( mrCurrentTextParagraph );

            // Add the needed info to the BulletListItemMap
            Reference< XInterface > xRef( mrCurrentTextParagraph, UNO_QUERY );
            OUString sId = implGetValidIDFromInterface( xRef );
            if( !sId.isEmpty() )
            {
                sId += ".bp";
                BulletListItemInfo& aBulletListItemInfo = maBulletListItemMap[ sId ];
                aBulletListItemInfo.nFontSize = rFont.GetHeight();
                aBulletListItemInfo.aColor = aTextColor;
                aBulletListItemInfo.aPos = maTextPos;
                aBulletListItemInfo.cBulletChar = mcBulletChar;

                // Make this text portion a bullet placeholder
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", sId );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "BulletPlaceholder" ) );
                SvXMLElementExport aSVGTspanElem( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, mbIWS, mbIWS );
                return;
            }
        }
    }

    Reference< XInterface > xRef( mrCurrentTextPortion, UNO_QUERY );
    const OUString& rTextPortionId = implGetValidIDFromInterface( xRef );
    if( !rTextPortionId.isEmpty() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", rTextPortionId );
    }

    if( mbIsPlacehlolderShape )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "PlaceholderText" ) );
        mbIsPlacehlolderShape = sal_False;
    }
    else if( mbIsURLField && !msUrl.isEmpty() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "UrlField" ) );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrXLinkHRef, msUrl );
        mbIsURLField = sal_False;
    }


    addFontAttributes( /* isTexTContainer: */ false );
    mpContext->AddPaintAttr( COL_TRANSPARENT, aTextColor );

    SvXMLElementExport aSVGTspanElem( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, mbIWS, mbIWS );
    OUString sTextContent = rText;
    mrExport.GetDocHandler()->characters( sTextContent );
    mnTextWidth += mpVDev->GetTextWidth( sTextContent );
}

// -------------------
// - SVGActionWriter -
// -------------------

SVGActionWriter::SVGActionWriter( SVGExport& rExport, SVGFontExport& rFontExport ) :
    mnCurGradientId( 1 ),
    mnCurMaskId( 1 ),
    mnCurPatternId( 1 ),
    mrExport( rExport ),
    mrFontExport( rFontExport ),
    mpContext( NULL ),
    maTextWriter( rExport, rFontExport ),
    mnInnerMtfCount( 0 ),
    mbClipAttrChanged( sal_False )
{
    mpVDev = new VirtualDevice;
    mpVDev->EnableOutput( sal_False );
    maTargetMapMode = MAP_100TH_MM;
    maTextWriter.setVirtualDevice( mpVDev, maTargetMapMode );
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
    Size aSz( nVal, nVal );

    return( ImplMap( aSz, aSz ).Width() );
}

// -----------------------------------------------------------------------------

Point& SVGActionWriter::ImplMap( const Point& rPt, Point& rDstPt ) const
{
    return( rDstPt = mpVDev->LogicToLogic( rPt, mpVDev->GetMapMode(), maTargetMapMode ) );
}

// -----------------------------------------------------------------------------

Size& SVGActionWriter::ImplMap( const Size& rSz, Size& rDstSz ) const
{
    return( rDstSz = mpVDev->LogicToLogic( rSz, mpVDev->GetMapMode(), maTargetMapMode ) );
}

// -----------------------------------------------------------------------------

Rectangle& SVGActionWriter::ImplMap( const Rectangle& rRect, Rectangle& rDstRect ) const
{
    Point   aTL( rRect.TopLeft() );
    Size    aSz( rRect.GetSize() );

    return( rDstRect = Rectangle( ImplMap( aTL, aTL ), ImplMap( aSz, aSz ) ) );
}


// -----------------------------------------------------------------------------

Polygon& SVGActionWriter::ImplMap( const Polygon& rPoly, Polygon& rDstPoly ) const
{
    rDstPoly = Polygon( rPoly.GetSize() );

    for( sal_uInt16 i = 0, nSize = rPoly.GetSize(); i < nSize; ++i )
    {
        ImplMap( rPoly[ i ], rDstPoly[ i ] );
        rDstPoly.SetFlags( i, rPoly.GetFlags( i ) );
    }

    return( rDstPoly );
}

// -----------------------------------------------------------------------------

PolyPolygon& SVGActionWriter::ImplMap( const PolyPolygon& rPolyPoly, PolyPolygon& rDstPolyPoly ) const
{
    Polygon aPoly;

    rDstPolyPoly = PolyPolygon();

    for( sal_uInt16 i = 0, nCount = rPolyPoly.Count(); i < nCount; ++i )
    {
        rDstPolyPoly.Insert( ImplMap( rPolyPoly[ i ], aPoly ) );
    }

    return( rDstPolyPoly );
}

// -----------------------------------------------------------------------------

::rtl::OUString SVGActionWriter::GetPathString( const PolyPolygon& rPolyPoly, sal_Bool bLine )
{
    ::rtl::OUString         aPathData;
    const ::rtl::OUString   aBlank( B2UCONST( " " ) );
    const ::rtl::OUString   aComma( B2UCONST( "," ) );
    Point                      aPolyPoint;

    for( long i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
    {
        const Polygon&  rPoly = rPolyPoly[ (sal_uInt16) i ];
        sal_uInt16 n = 1, nSize = rPoly.GetSize();

        if( nSize > 1 )
        {
            aPathData += B2UCONST( "M " );
            aPathData += ::rtl::OUString::valueOf( ( aPolyPoint = rPoly[ 0 ] ).X() );
            aPathData += aComma;
            aPathData += ::rtl::OUString::valueOf( aPolyPoint.Y() );

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
                        aPathData += ::rtl::OUString::valueOf( ( aPolyPoint = rPoly[ n++ ] ).X() );
                        aPathData += aComma;
                        aPathData += ::rtl::OUString::valueOf( aPolyPoint.Y() );
                    }
                }
                else
                {
                    if ( nCurrentMode != 'L' )
                    {
                        nCurrentMode = 'L';
                        aPathData += B2UCONST( "L " );
                    }
                    aPathData += ::rtl::OUString::valueOf( ( aPolyPoint = rPoly[ n++ ] ).X() );
                    aPathData += aComma;
                    aPathData += ::rtl::OUString::valueOf( aPolyPoint.Y() );
                }
            }

            if( !bLine )
                aPathData += B2UCONST( " Z" );

            if( i < ( nCount - 1 ) )
                aPathData += aBlank;
        }
    }

     return aPathData;
}

// -----------------------------------------------------------------------------

sal_uLong SVGActionWriter::GetChecksum( const MetaAction* pAction )
{
    GDIMetaFile aMtf;
    MetaAction* pA = (MetaAction*)pAction;
    pA->Duplicate();
    aMtf.AddAction( pA );
    return aMtf.GetChecksum();
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteLine( const Point& rPt1, const Point& rPt2,
                                     const Color* pLineColor, sal_Bool bApplyMapping )
{
    Point aPt1, aPt2;

    if( bApplyMapping )
    {
        ImplMap( rPt1, aPt1 );
        ImplMap( rPt2, aPt2 );
    }
    else
    {
        aPt1 = rPt1;
        aPt2 = rPt2;
    }

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX1, ::rtl::OUString::valueOf( aPt1.X() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY1, ::rtl::OUString::valueOf( aPt1.Y() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX2, ::rtl::OUString::valueOf( aPt2.X() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY2, ::rtl::OUString::valueOf( aPt2.Y() ) );

    if( pLineColor )
    {
        // !!! mrExport.AddAttribute( XML_NAMESPACE_NONE, ... )
        OSL_FAIL( "SVGActionWriter::ImplWriteLine: Line color not implemented" );
    }

    {
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemLine, sal_True, sal_True );
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteRect( const Rectangle& rRect, long nRadX, long nRadY,
                                     sal_Bool bApplyMapping )
{
    Rectangle aRect;

    if( bApplyMapping )
        ImplMap( rRect, aRect );
    else
        aRect = rRect;

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, ::rtl::OUString::valueOf( aRect.Left() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, ::rtl::OUString::valueOf( aRect.Top() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, ::rtl::OUString::valueOf( aRect.GetWidth() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, ::rtl::OUString::valueOf( aRect.GetHeight() ) );

    if( nRadX )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRX, ::rtl::OUString::valueOf( bApplyMapping ? ImplMap( nRadX ) : nRadX ) );

    if( nRadY )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRY, ::rtl::OUString::valueOf( bApplyMapping ? ImplMap( nRadY ) : nRadY ) );

    {
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemRect, sal_True, sal_True );
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteEllipse( const Point& rCenter, long nRadX, long nRadY,
                                        sal_Bool bApplyMapping )
{
    Point aCenter;

    if( bApplyMapping )
        ImplMap( rCenter, aCenter );
    else
        aCenter = rCenter;

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCX, ::rtl::OUString::valueOf( aCenter.X() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCY, ::rtl::OUString::valueOf( aCenter.Y() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRX, ::rtl::OUString::valueOf( bApplyMapping ? ImplMap( nRadX ) : nRadX ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRY, ::rtl::OUString::valueOf( bApplyMapping ? ImplMap( nRadY ) : nRadY ) );

    {
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemEllipse, sal_True, sal_True );
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWritePolyPolygon( const PolyPolygon& rPolyPoly, sal_Bool bLineOnly,
                                            sal_Bool bApplyMapping )
{
    PolyPolygon aPolyPoly;

    if( bApplyMapping )
        ImplMap( rPolyPoly, aPolyPoly );
    else
        aPolyPoly = rPolyPoly;

    // add path data attribute
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrD, GetPathString( aPolyPoly, bLineOnly ) );

    {
        // write polyline/polygon element
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemPath, sal_True, sal_True );
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteShape( const SVGShapeDescriptor& rShape, sal_Bool bApplyMapping )
{
    PolyPolygon aPolyPoly;

    if( bApplyMapping )
        ImplMap( rShape.maShapePolyPoly, aPolyPoly );
    else
        aPolyPoly = rShape.maShapePolyPoly;

    const sal_Bool  bLineOnly = ( rShape.maShapeFillColor == Color( COL_TRANSPARENT ) ) && ( !rShape.mapShapeGradient.get() );
    Rectangle   aBoundRect( aPolyPoly.GetBoundRect() );

    mpContext->AddPaintAttr( rShape.maShapeLineColor, rShape.maShapeFillColor, &aBoundRect, rShape.mapShapeGradient.get() );

    if( !rShape.maId.isEmpty() )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, rShape.maId );

    if( rShape.mnStrokeWidth )
    {
        sal_Int32 nStrokeWidth = ( bApplyMapping ? ImplMap( rShape.mnStrokeWidth ) : rShape.mnStrokeWidth );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStrokeWidth, ::rtl::OUString::valueOf( nStrokeWidth ) );
    }

    if( rShape.maDashArray.size() )
    {
        const ::rtl::OUString   aComma( B2UCONST( "," ) );
        ::rtl::OUString         aDashArrayStr;

        for( unsigned int k = 0; k < rShape.maDashArray.size(); ++k )
        {
            const sal_Int32 nDash = ( bApplyMapping ?
                                        ImplMap( FRound( rShape.maDashArray[ k ] ) ) :
                                        FRound( rShape.maDashArray[ k ] ) );

            if( k )
                aDashArrayStr += aComma;

            aDashArrayStr += ::rtl::OUString::valueOf( nDash );
        }

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStrokeDashArray, aDashArrayStr );
    }

    ImplWritePolyPolygon( aPolyPoly, bLineOnly, sal_False );
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWritePattern( const PolyPolygon& rPolyPoly,
                                        const Hatch* pHatch,
                                        const Gradient* pGradient,
                                        sal_uInt32 nWriteFlags )
{
    if( rPolyPoly.Count() )
    {
        SvXMLElementExport aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, sal_True, sal_True );

        ::rtl::OUString aPatternId;
        aPatternId += B2UCONST( "pattern" );
        aPatternId += OUString::valueOf( mnCurPatternId++ );

        {
            SvXMLElementExport aElemDefs( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, sal_True, sal_True );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, aPatternId );

            Rectangle aRect;
            ImplMap( rPolyPoly.GetBoundRect(), aRect );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, OUString::valueOf( aRect.Left() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, OUString::valueOf( aRect.Top() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, OUString::valueOf( aRect.GetWidth() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, OUString::valueOf( aRect.GetHeight() ) );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrPatternUnits, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "userSpaceOnUse") ) );

            {
                SvXMLElementExport aElemPattern( mrExport, XML_NAMESPACE_NONE, aXMLElemPattern, sal_True, sal_True );

                // The origin of a pattern is positioned at (aRect.Left(), aRect.Top()).
                // So we need to adjust the pattern coordinate.
                ::rtl::OUString aTransform;
                aTransform += B2UCONST( "translate" );
                aTransform += B2UCONST( "(" );
                aTransform += OUString::valueOf( -aRect.Left() );
                aTransform += B2UCONST( "," );
                aTransform += OUString::valueOf( -aRect.Top() );
                aTransform += B2UCONST( ")" );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTransform, aTransform );

                {
                    SvXMLElementExport aElemG2( mrExport, XML_NAMESPACE_NONE, aXMLElemG, sal_True, sal_True );

                    GDIMetaFile aTmpMtf;
                    if( pHatch )
                        mpVDev->AddHatchActions( rPolyPoly, *pHatch, aTmpMtf );
                    else if ( pGradient )
                        mpVDev->AddGradientActions( rPolyPoly.GetBoundRect(), *pGradient, aTmpMtf );
                    ImplWriteActions( aTmpMtf, nWriteFlags, NULL );
                }
            }
        }

        ::rtl::OUString aPatternStyle;
        aPatternStyle += B2UCONST( "fill:url(#" );
        aPatternStyle += aPatternId;
        aPatternStyle += B2UCONST( ")" );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aPatternStyle );
        ImplWritePolyPolygon( rPolyPoly, sal_False );
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteGradientEx( const PolyPolygon& rPolyPoly, const Gradient& rGradient,
                                           sal_uInt32 nWriteFlags, sal_Bool bApplyMapping )
{
    PolyPolygon aPolyPoly;

    if( bApplyMapping )
        ImplMap( rPolyPoly, aPolyPoly );
    else
        aPolyPoly = rPolyPoly;

    if ( rGradient.GetStyle() == GradientStyle_LINEAR ||
         rGradient.GetStyle() == GradientStyle_AXIAL )
    {
        ImplWriteGradientLinear( aPolyPoly, rGradient );
    }
    else
    {
        ImplWritePattern( aPolyPoly, NULL, &rGradient, nWriteFlags );
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteGradientLinear( const PolyPolygon& rPolyPoly,
                                               const Gradient& rGradient )
{
    if( rPolyPoly.Count() )
    {
        SvXMLElementExport aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, sal_True, sal_True );

        ::rtl::OUString aGradientId;
        aGradientId += B2UCONST( "gradient" );
        aGradientId += OUString::valueOf( mnCurGradientId++ );

        {
            SvXMLElementExport aElemDefs( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, sal_True, sal_True );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, aGradientId );
            {
                Rectangle aTmpRect, aRect;
                Point aTmpCenter, aCenter;

                rGradient.GetBoundRect( rPolyPoly.GetBoundRect(), aTmpRect, aTmpCenter );
                ImplMap( aTmpRect, aRect );
                ImplMap( aTmpCenter, aCenter );
                const sal_uInt16 nAngle = rGradient.GetAngle() % 3600;

                Polygon aPoly( 2 );
                // Setting x value of a gradient vector to rotation center to
                // place a gradient vector in a target polygon.
                // This would help editing it in SVG editors like inkscape.
                aPoly[ 0 ].X() = aPoly[ 1 ].X() = aCenter.X();
                aPoly[ 0 ].Y() = aRect.Top();
                aPoly[ 1 ].Y() = aRect.Bottom();
                aPoly.Rotate( aCenter, nAngle );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX1, OUString::valueOf( aPoly[ 0 ].X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY1, OUString::valueOf( aPoly[ 0 ].Y() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX2, OUString::valueOf( aPoly[ 1 ].X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY2, OUString::valueOf( aPoly[ 1 ].Y() ) );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrGradientUnits,
                                       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "userSpaceOnUse" ) ) );
            }

            {
                SvXMLElementExport aElemLinearGradient( mrExport, XML_NAMESPACE_NONE, aXMLElemLinearGradient, sal_True, sal_True );

                const Color aStartColor = ImplGetColorWithIntensity( rGradient.GetStartColor(), rGradient.GetStartIntensity() );
                const Color aEndColor = ImplGetColorWithIntensity( rGradient.GetEndColor(), rGradient.GetEndIntensity() );
                double fBorderOffset = rGradient.GetBorder() / 100.0;
                const sal_uInt16 nSteps = rGradient.GetSteps();
                if( rGradient.GetStyle() == GradientStyle_LINEAR )
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

        ::rtl::OUString aGradientStyle;
        aGradientStyle += B2UCONST( "fill:" );
        aGradientStyle += B2UCONST( "url(#" );
        aGradientStyle += aGradientId;
        aGradientStyle += B2UCONST( ")" );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aGradientStyle );
        ImplWritePolyPolygon( rPolyPoly, sal_False );
    }
}

void SVGActionWriter::ImplWriteGradientStop( const Color& rColor, double fOffset )
{
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset, rtl::OUString::valueOf( fOffset ) );

    ::rtl::OUString aStyle, aColor;
    aStyle += B2UCONST( "stop-color:" );
    SVGAttributeWriter::ImplGetColorStr ( rColor, aColor );
    aStyle += aColor;

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aStyle );
    {
        SvXMLElementExport aElemStartStop( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, sal_True, sal_True );
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

    ::rtl::OUString aMaskId;
    aMaskId += B2UCONST( "mask" );
    aMaskId += OUString::valueOf( mnCurMaskId++ );

    {
        SvXMLElementExport aElemDefs( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, sal_True, sal_True );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, aMaskId );
        {
            SvXMLElementExport aElemMask( mrExport, XML_NAMESPACE_NONE, aXMLElemMask, sal_True, sal_True );

            const PolyPolygon aPolyPolygon( PolyPolygon( Rectangle( rDestPt, rDestSize ) ) );
            Gradient aGradient( rGradient );

            // swap gradient stops to adopt SVG mask
            Color aTmpColor( aGradient.GetStartColor() );
            sal_uInt16 nTmpIntensity( aGradient.GetStartIntensity() );
            aGradient.SetStartColor( aGradient.GetEndColor() );
            aGradient.SetStartIntensity( aGradient.GetEndIntensity() ) ;
            aGradient.SetEndColor( aTmpColor );
            aGradient.SetEndIntensity( nTmpIntensity );

            ImplWriteGradientEx( aPolyPolygon, aGradient, nWriteFlags );
        }
    }

    ::rtl::OUString aMaskStyle;
    aMaskStyle += B2UCONST( "mask:url(#" );
    aMaskStyle += aMaskId;
    aMaskStyle += B2UCONST( ")" );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aMaskStyle );

    {
        SvXMLElementExport aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, sal_True, sal_True );

        mpVDev->Push();
        ImplWriteActions( rMtf, nWriteFlags, NULL );
        mpVDev->Pop();
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteText( const Point& rPos, const String& rText,
                                     const sal_Int32* pDXArray, long nWidth,
                                     sal_Bool bApplyMapping )
{
    const FontMetric aMetric( mpVDev->GetFontMetric() );

    bool bTextSpecial = aMetric.IsShadow() || aMetric.IsOutline() || (aMetric.GetRelief() != RELIEF_NONE);

    if( !bTextSpecial )
    {
        ImplWriteText( rPos, rText, pDXArray, nWidth, mpVDev->GetTextColor(), bApplyMapping );
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

            ImplWriteText( aPos, rText, pDXArray, nWidth, aReliefColor, bApplyMapping );
            ImplWriteText( rPos, rText, pDXArray, nWidth, aTextColor, bApplyMapping );
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
                ImplWriteText( aPos, rText, pDXArray, nWidth, aShadowColor, bApplyMapping );

                if( !aMetric.IsOutline() )
                {
                    ImplWriteText( rPos, rText, pDXArray, nWidth, aTextColor, bApplyMapping );
                }
            }

            if( aMetric.IsOutline() )
            {
                Point aPos = rPos + Point( -6, -6 );
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor(), bApplyMapping );
                aPos = rPos + Point( +6, +6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor(), bApplyMapping );
                aPos = rPos + Point( -6, +0);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor(), bApplyMapping );
                aPos = rPos + Point( -6, +6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor(), bApplyMapping );
                aPos = rPos + Point( +0, +6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor(), bApplyMapping );
                aPos = rPos + Point( +0, -6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor(), bApplyMapping );
                aPos = rPos + Point( +6, -1);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor(), bApplyMapping );
                aPos = rPos + Point( +6, +0);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor(), bApplyMapping );

                ImplWriteText( rPos, rText, pDXArray, nWidth, Color( COL_WHITE ), bApplyMapping );
            }
        }
    }
}

void SVGActionWriter::ImplWriteText( const Point& rPos, const String& rText,
                                     const sal_Int32* pDXArray, long nWidth,
                                     Color aTextColor, sal_Bool bApplyMapping )
{
    sal_Int32                               nLen = rText.Len();
    Size                                    aNormSize;
    Point                                   aPos;
    Point                                   aBaseLinePos( rPos );
    const FontMetric                        aMetric( mpVDev->GetFontMetric() );
    const Font&                             rFont = mpVDev->GetFont();

    if( rFont.GetAlign() == ALIGN_TOP )
        aBaseLinePos.Y() += aMetric.GetAscent();
    else if( rFont.GetAlign() == ALIGN_BOTTOM )
        aBaseLinePos.Y() -= aMetric.GetDescent();

    if( bApplyMapping )
        ImplMap( rPos, aPos );
    else
        aPos = rPos;

    boost::shared_array<sal_Int32> xTmpArray(new sal_Int32[nLen]);
    // get text sizes
    if( pDXArray )
    {
        aNormSize = Size( mpVDev->GetTextWidth( rText ), 0 );
        memcpy(xTmpArray.get(), pDXArray, nLen * sizeof(sal_Int32));
    }
    else
    {
        aNormSize = Size( mpVDev->GetTextArray( rText, xTmpArray.get() ), 0 );
    }
    sal_Int32* pDX = xTmpArray.get();

    // if text is rotated, set transform matrix at new g element
    if( rFont.GetOrientation() )
    {
        Point   aRot( aPos );
        String  aTransform;

        aTransform = String( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "translate" ) ) );
        aTransform += '(';
        aTransform += String( ::rtl::OUString::valueOf( aRot.X() ) );
        aTransform += ',';
        aTransform += String( ::rtl::OUString::valueOf( aRot.Y() ) );
        aTransform += ')';

        aTransform += String( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " rotate" ) ) );
        aTransform += '(';
        aTransform += String( ::rtl::OUString::valueOf( rFont.GetOrientation() * -0.1 ) );
        aTransform += ')';

        aTransform += String( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " translate" ) ) );
        aTransform += '(';
        aTransform += String( ::rtl::OUString::valueOf( -aRot.X() ) );
        aTransform += ',';
        aTransform += String( ::rtl::OUString::valueOf( -aRot.Y() ) );
        aTransform += ')';

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTransform, aTransform );
    }


    mpContext->AddPaintAttr( COL_TRANSPARENT, aTextColor );

    // for each line of text there should be at least one group element
    SvXMLElementExport aSVGGElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, sal_True, sal_False );

    sal_Bool bIsPlaceholderField = sal_False;

    if( mbIsPlacehlolderShape )
    {
        OUString sTextContent = rText;
        bIsPlaceholderField = sTextContent.match( sPlaceholderTag );
        // for a placeholder text field we export only one <text> svg element
        if( bIsPlaceholderField )
        {
            OUString sCleanTextContent;
            static const sal_Int32 nFrom = sPlaceholderTag.getLength();
            if( sTextContent.getLength() > nFrom )
            {
                sCleanTextContent = sTextContent.copy( nFrom );
            }
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", B2UCONST( "PlaceholderText" ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, ::rtl::OUString::valueOf( aPos.X() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, ::rtl::OUString::valueOf( aPos.Y() ) );
            {
                SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemText, sal_True, sal_False );
                // At least for the single slide case we need really to  export placeholder text
                mrExport.GetDocHandler()->characters( sCleanTextContent );
            }
        }
    }

    if( !bIsPlaceholderField )
    {
        if( nLen > 1 )
        {
            aNormSize.Width() = pDX[ nLen - 2 ] + mpVDev->GetTextWidth( rtl::OUString(rText.GetChar(nLen - 1)) );

            if( nWidth && aNormSize.Width() && ( nWidth != aNormSize.Width() ) )
            {
                long i;
                const double fFactor = (double) nWidth / aNormSize.Width();

                for( i = 0; i < ( nLen - 1 ); i++ )
                    pDX[ i ] = FRound( pDX[ i ] * fFactor );
            }
            else
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > xBI( ::vcl::unohelper::CreateBreakIterator() );
                const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetLocale();
                sal_Int32 nCurPos = 0, nLastPos = 0, nX = aPos.X();

                // write single glyphs at absolute text positions
                for( sal_Bool bCont = sal_True; bCont; )
                {
                    sal_Int32 nCount = 1;

                    nLastPos = nCurPos;
                    nCurPos = xBI->nextCharacters( rText, nCurPos, rLocale,
                                                ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL,
                                                nCount, nCount );

                    nCount = nCurPos - nLastPos;
                    bCont = ( nCurPos < rText.Len() ) && nCount;

                    if( nCount )
                    {
                        const ::rtl::OUString aGlyph( rText.Copy( nLastPos, nCount ) );

                        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, ::rtl::OUString::valueOf( nX ) );
                        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, ::rtl::OUString::valueOf( aPos.Y() ) );

                        {
                            SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemText, sal_True, sal_False );
                            mrExport.GetDocHandler()->characters( aGlyph );
                        }

                        if( bCont )
                            nX = aPos.X() + pDX[ nCurPos - 1 ];
                    }
                }
            }
        }
        else
        {
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, ::rtl::OUString::valueOf( aPos.X() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, ::rtl::OUString::valueOf( aPos.Y() ) );

            {
                SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemText, sal_True, sal_False );
                mrExport.GetDocHandler()->characters( rText );
            }
        }
    }


    if( !mrExport.IsUseNativeTextDecoration() )
    {
        if( rFont.GetStrikeout() != STRIKEOUT_NONE || rFont.GetUnderline() != UNDERLINE_NONE )
        {
            Polygon     aPoly( 4 );
            const long  nLineHeight = Max( (long) FRound( aMetric.GetLineHeight() * 0.05 ), (long) 1 );

            if( rFont.GetStrikeout() )
            {
                const long nYLinePos = aBaseLinePos.Y() - FRound( aMetric.GetAscent() * 0.26 );

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
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteBmp( const BitmapEx& rBmpEx,
                                    const Point& rPt, const Size& rSz,
                                    const Point& rSrcPt, const Size& rSrcSz,
                                    sal_Bool bApplyMapping )
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
                Point                    aPt;
                Size                     aSz;
                Sequence< sal_Int8 >     aSeq( (sal_Int8*) aOStm.GetData(), aOStm.Tell() );
                rtl::OUStringBuffer aBuffer;
                aBuffer.appendAscii( "data:image/png;base64," );
                ::sax::Converter::encodeBase64( aBuffer, aSeq );

                if( bApplyMapping )
                {
                    ImplMap( rPt, aPt );
                    ImplMap( rSz, aSz );
                }
                else
                {
                    aPt = rPt;
                    aSz = rSz;
                }

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, OUString::valueOf( aPt.X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, OUString::valueOf( aPt.Y() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, OUString::valueOf( aSz.Width() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, OUString::valueOf( aSz.Height() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrXLinkHRef, aBuffer.makeStringAndClear() );
                {
                    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemImage, sal_True, sal_True );
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SVGActionWriter::ImplWriteActions( const GDIMetaFile& rMtf,
                                        sal_uInt32 nWriteFlags,
                                        const ::rtl::OUString* pElementId,
                                        const Reference< XShape >* pxShape,
                                        const GDIMetaFile* pTextEmbeddedBitmapMtf )
{
    if( mnInnerMtfCount )
        nWriteFlags |= SVGWRITER_NO_SHAPE_COMMENTS;


#if OSL_DEBUG_LEVEL > 0
    bool bIsTextShape = false;
    if( !mrExport.IsUsePositionedCharacters() && pxShape
            && Reference< XText >( *pxShape, UNO_QUERY ).is() )
    {
        bIsTextShape = true;
    }
#endif
    mbIsPlacehlolderShape = false;
    if( ( pElementId != NULL ) && ( *pElementId == sPlaceholderTag ) )
    {
        mbIsPlacehlolderShape = true;
        // since we utilize pElementId in an improper way we reset it to NULL before to go on
        pElementId = NULL;
    }

    for( sal_uLong nCurAction = 0, nCount = rMtf.GetActionSize(); nCurAction < nCount; nCurAction++ )
    {
        const MetaAction*   pAction = rMtf.GetAction( nCurAction );
        const sal_uInt16        nType = pAction->GetType();

#if OSL_DEBUG_LEVEL > 0
        if( bIsTextShape )
        {
            try
            {

            SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, "desc", sal_False, sal_False );
            OUStringBuffer sType;
            sType.append(static_cast<sal_Int32>(nType));
            if( pAction && ( nType == META_COMMENT_ACTION ) )
            {
                sType.append(": ");
                const MetaCommentAction* pA = (const MetaCommentAction*) pAction;
                rtl::OString sComment = pA->GetComment();
                if( !sComment.isEmpty() )
                {
                    OUString ssComment = OUString( sComment.getStr(), sComment.getLength(), RTL_TEXTENCODING_UTF8 );
                    sType.append(ssComment);
                }
                if( sComment.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("FIELD_SEQ_BEGIN")) )
                {
                    sal_uInt8 const*const pData = pA->GetData();
                    if( pData && ( pA->GetDataSize() ) )
                    {
                        sal_uInt16 sz = (sal_uInt16)( ( pA->GetDataSize() ) / 2 );
                        if( sz )
                        {
                            sType.append("; ");
                            sType.append(
                                reinterpret_cast<sal_Unicode const*>(pData),
                                sz);
                        }
                    }
                }
            }
            if (sType.getLength())
            {
                mrExport.GetDocHandler()->characters(sType.makeStringAndClear());
            }
            }
            catch( ... )
            {
                const MetaCommentAction* pA = (const MetaCommentAction*) pAction;
                OSL_FAIL( pA->GetComment().getStr() );
            }

        }
#endif
        switch( nType )
        {
            case( META_PIXEL_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaPixelAction* pA = (const MetaPixelAction*) pAction;

                    mpContext->AddPaintAttr( pA->GetColor(), pA->GetColor() );
                    ImplWriteLine( pA->GetPoint(), pA->GetPoint(), &pA->GetColor() );
                }
            }
            break;

            case( META_POINT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaPointAction* pA = (const MetaPointAction*) pAction;

                    mpContext->AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetLineColor() );
                    ImplWriteLine( pA->GetPoint(), pA->GetPoint(), NULL );
                }
            }
            break;

            case( META_LINE_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaLineAction* pA = (const MetaLineAction*) pAction;

                    mpContext->AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetLineColor() );
                    ImplWriteLine( pA->GetStartPoint(), pA->GetEndPoint(), NULL );
                }
            }
            break;

            case( META_RECT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    mpContext->AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteRect( ( (const MetaRectAction*) pAction )->GetRect(), 0, 0 );
                }
            }
            break;

            case( META_ROUNDRECT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pAction;

                    mpContext->AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }
            }
            break;

            case( META_ELLIPSE_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaEllipseAction*    pA = (const MetaEllipseAction*) pAction;
                    const Rectangle&            rRect = pA->GetRect();

                    mpContext->AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteEllipse( rRect.Center(), rRect.GetWidth() >> 1, rRect.GetHeight() >> 1 );
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
                        mpContext->AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                        ImplWritePolyPolygon( aPoly, sal_False );
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
                        mpContext->AddPaintAttr( mpVDev->GetLineColor(), Color( COL_TRANSPARENT ) );
                        ImplWritePolyPolygon( rPoly, sal_True );
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
                        mpContext->AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                        ImplWritePolyPolygon( rPolyPoly, sal_False );
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

                    ImplWriteGradientEx( aRectPolyPoly, pA->GetGradient(), nWriteFlags );
                }
            }
            break;

            case( META_GRADIENTEX_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaGradientExAction* pA = (const MetaGradientExAction*) pAction;
                    ImplWriteGradientEx( pA->GetPolyPolygon(), pA->GetGradient(), nWriteFlags );
                }
            }
            break;

            case META_HATCH_ACTION:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaHatchAction*  pA = (const MetaHatchAction*) pAction;
                    ImplWritePattern( pA->GetPolyPolygon(), &pA->GetHatch(), NULL, nWriteFlags );
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
                        Color aNewLineColor( mpVDev->GetLineColor() ), aNewFillColor( mpVDev->GetFillColor() );

                        aNewLineColor.SetTransparency( sal::static_int_cast<sal_uInt8>( FRound( pA->GetTransparence() * 2.55 ) ) );
                        aNewFillColor.SetTransparency( sal::static_int_cast<sal_uInt8>( FRound( pA->GetTransparence() * 2.55 ) ) );

                        mpContext->AddPaintAttr( aNewLineColor, aNewFillColor );
                        ImplWritePolyPolygon( rPolyPoly, sal_False );
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
                                   pA->GetGradient(), nWriteFlags  );
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

                    for( sal_uInt32 k = 0, nCount2 = aGDIMetaFile.GetActionSize(); ( k < nCount2 ) && !bFound; ++k )
                    {
                        const MetaAction* pSubstAct = aGDIMetaFile.GetAction( k );

                        if( pSubstAct->GetType() == META_BMPSCALE_ACTION )
                        {
                            bFound = sal_True;
                            const MetaBmpScaleAction* pBmpScaleAction = (const MetaBmpScaleAction*) pSubstAct;
                            ImplWriteBmp( pBmpScaleAction->GetBitmap(),
                                          pA->GetPoint(), pA->GetSize(),
                                          Point(), pBmpScaleAction->GetBitmap().GetSizePixel() );
                        }
                    }
                }
            }
            break;

            case( META_COMMENT_ACTION ):
            {
                const MetaCommentAction*    pA = (const MetaCommentAction*) pAction;
                String                      aSkipComment;

                if( ( pA->GetComment().equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XGRAD_SEQ_BEGIN")) ) &&
                    ( nWriteFlags & SVGWRITER_WRITE_FILL ) )
                {
                    const MetaGradientExAction* pGradAction = NULL;
                    sal_Bool                    bDone = sal_False;

                    while( !bDone && ( ++nCurAction < nCount ) )
                    {
                        pAction = rMtf.GetAction( nCurAction );

                        if( pAction->GetType() == META_GRADIENTEX_ACTION )
                            pGradAction = (const MetaGradientExAction*) pAction;
                        else if( ( pAction->GetType() == META_COMMENT_ACTION ) &&
                                 ( ( (const MetaCommentAction*) pAction )->GetComment().
                                        equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XGRAD_SEQ_END")) ) )
                        {
                            bDone = sal_True;
                        }
                    }

                    if( pGradAction )
                        ImplWriteGradientEx( pGradAction->GetPolyPolygon(), pGradAction->GetGradient(), nWriteFlags );
                }
                else if( ( pA->GetComment().equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XPATHFILL_SEQ_BEGIN")) ) &&
                         ( nWriteFlags & SVGWRITER_WRITE_FILL ) && !( nWriteFlags & SVGWRITER_NO_SHAPE_COMMENTS ) &&
                         pA->GetDataSize() )
                {
                    // write open shape in every case
                    if( mapCurShape.get() )
                    {
                        ImplWriteShape( *mapCurShape );
                        mapCurShape.reset();
                    }

                    SvMemoryStream  aMemStm( (void*) pA->GetData(), pA->GetDataSize(), STREAM_READ );
                    SvtGraphicFill  aFill;

                    aMemStm >> aFill;

                    sal_Bool bGradient = SvtGraphicFill::fillGradient == aFill.getFillType() &&
                                     ( SvtGraphicFill::gradientLinear == aFill.getGradientType() ||
                                       SvtGraphicFill::gradientRadial == aFill.getGradientType() );
                    sal_Bool bSkip = ( SvtGraphicFill::fillSolid == aFill.getFillType() || bGradient );

                    if( bSkip )
                    {
                        PolyPolygon aShapePolyPoly;

                        aFill.getPath( aShapePolyPoly );

                        if( aShapePolyPoly.Count() )
                        {
                            mapCurShape.reset( new SVGShapeDescriptor );

                            if( pElementId )
                                mapCurShape->maId = *pElementId;

                            mapCurShape->maShapePolyPoly = aShapePolyPoly;
                            mapCurShape->maShapeFillColor = aFill.getFillColor();
                            mapCurShape->maShapeFillColor.SetTransparency( (sal_uInt8) FRound( 255.0 * aFill.getTransparency() ) );

                            if( bGradient )
                            {
                                // step through following actions until the first Gradient/GradientEx action is found
                                while( !mapCurShape->mapShapeGradient.get() && bSkip && ( ++nCurAction < nCount ) )
                                {
                                    pAction = rMtf.GetAction( nCurAction );

                                    if( ( pAction->GetType() == META_COMMENT_ACTION ) &&
                                        ( ( (const MetaCommentAction*) pAction )->GetComment().
                                        equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XPATHFILL_SEQ_END")) ) )
                                    {
                                        bSkip = sal_False;
                                    }
                                    else if( pAction->GetType() == META_GRADIENTEX_ACTION )
                                    {
                                        mapCurShape->mapShapeGradient.reset( new Gradient(
                                            static_cast< const MetaGradientExAction* >( pAction )->GetGradient() ) );
                                    }
                                    else if( pAction->GetType() == META_GRADIENT_ACTION )
                                    {
                                        mapCurShape->mapShapeGradient.reset( new Gradient(
                                            static_cast< const MetaGradientAction* >( pAction )->GetGradient() ) );
                                    }
                                }
                            }
                        }
                        else
                            bSkip = sal_False;
                    }

                    // skip rest of comment
                    while( bSkip && ( ++nCurAction < nCount ) )
                    {
                        pAction = rMtf.GetAction( nCurAction );

                        if( ( pAction->GetType() == META_COMMENT_ACTION ) &&
                                    ( ( (const MetaCommentAction*) pAction )->GetComment().
                                    equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XPATHFILL_SEQ_END")) ) )
                        {
                            bSkip = sal_False;
                        }
                    }
                }
                else if( ( pA->GetComment().equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XPATHSTROKE_SEQ_BEGIN")) ) &&
                         ( nWriteFlags & SVGWRITER_WRITE_FILL ) && !( nWriteFlags & SVGWRITER_NO_SHAPE_COMMENTS ) &&
                         pA->GetDataSize() )
                {
                    SvMemoryStream      aMemStm( (void*) pA->GetData(), pA->GetDataSize(), STREAM_READ );
                    SvtGraphicStroke    aStroke;
                    PolyPolygon         aStartArrow, aEndArrow;

                    aMemStm >> aStroke;
                    aStroke.getStartArrow( aStartArrow );
                    aStroke.getEndArrow( aEndArrow );

                    // Currently no support for strokes with start/end arrow(s)
                    sal_Bool bSkip = ( !aStartArrow.Count() && !aEndArrow.Count() );

                    if( bSkip )
                    {
                        if( !mapCurShape.get() )
                        {
                            Polygon aPoly;

                            mapCurShape.reset( new SVGShapeDescriptor );

                            if( pElementId )
                                mapCurShape->maId = *pElementId;

                            aStroke.getPath( aPoly );
                            mapCurShape->maShapePolyPoly = aPoly;
                        }

                        mapCurShape->maShapeLineColor = mpVDev->GetLineColor();
                        mapCurShape->maShapeLineColor.SetTransparency( (sal_uInt8) FRound( aStroke.getTransparency() * 255.0 ) );
                        mapCurShape->mnStrokeWidth = FRound( aStroke.getStrokeWidth() );
                        aStroke.getDashArray( mapCurShape->maDashArray );
                    }

                    // write open shape in every case
                    if( mapCurShape.get() )
                    {
                        ImplWriteShape( *mapCurShape );
                        mapCurShape.reset();
                    }

                    // skip rest of comment
                    while( bSkip && ( ++nCurAction < nCount ) )
                    {
                        pAction = rMtf.GetAction( nCurAction );

                        if( ( pAction->GetType() == META_COMMENT_ACTION ) &&
                                    ( ( (const MetaCommentAction*) pAction )->GetComment().
                                    equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XPATHSTROKE_SEQ_END")) ) )
                        {
                            bSkip = sal_False;
                        }
                    }
                }
                else if( !mrExport.IsUsePositionedCharacters() && ( nWriteFlags & SVGWRITER_WRITE_TEXT ) )
                {
                    if( ( pA->GetComment().equalsIgnoreAsciiCaseL( RTL_CONSTASCII_STRINGPARAM( "XTEXT_PAINTSHAPE_BEGIN" ) ) ) )
                    {
                        if( pxShape )
                        {
                            Reference< XText > xText( *pxShape, UNO_QUERY );
                            if( xText.is() )
                                maTextWriter.setTextShape( xText, pTextEmbeddedBitmapMtf );
                        }
                        maTextWriter.createParagraphEnumeration();
                        {
                            // nTextFound == -1 => no text found
                            // nTextFound ==  0 => no text found and end of text shape reached
                            // nTextFound ==  1 => text found!
                            sal_Int32 nTextFound = -1;
                            while( ( nTextFound < 0 ) && ( nCurAction < nCount ) )
                            {
                                nTextFound = maTextWriter.setTextPosition( rMtf, nCurAction );
                            }
                            // We found some text in the current text shape.
                            if( nTextFound > 0 )
                            {
                                maTextWriter.setTextProperties( rMtf, nCurAction );
                                maTextWriter.startTextShape();
                            }
                            // We reached the end of the current text shape
                            // without finding any text. So we need to go back
                            // by one action in order to handle the
                            // XTEXT_PAINTSHAPE_END action because on the next
                            // loop the nCurAction is incremented by one.
                            else
                            {
                                --nCurAction;
                            }
                        }
                    }
                    else if( ( pA->GetComment().equalsIgnoreAsciiCaseL( RTL_CONSTASCII_STRINGPARAM( "XTEXT_PAINTSHAPE_END" ) ) ) )
                    {
                        maTextWriter.endTextShape();
                    }
                    else if( ( pA->GetComment().equalsIgnoreAsciiCaseL( RTL_CONSTASCII_STRINGPARAM( "XTEXT_EOP" ) ) ) )
                    {
                        const MetaAction* pNextAction = rMtf.GetAction( nCurAction + 1 );
                        if( !( ( pNextAction->GetType() == META_COMMENT_ACTION ) &&
                               ( ( (const MetaCommentAction*) pNextAction )->GetComment().equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XTEXT_PAINTSHAPE_END") ) )  ))
                        {
                            // nTextFound == -1 => no text found and end of paragraph reached
                            // nTextFound ==  0 => no text found and end of text shape reached
                            // nTextFound ==  1 => text found!
                            sal_Int32 nTextFound = -1;
                            while( ( nTextFound < 0 ) && ( nCurAction < nCount ) )
                            {
                                nTextFound = maTextWriter.setTextPosition( rMtf, nCurAction );
                            }
                            // We found a paragraph with some text in the
                            // current text shape.
                            if( nTextFound > 0 )
                            {
                                maTextWriter.setTextProperties( rMtf, nCurAction );
                                maTextWriter.startTextParagraph();
                            }
                            // We reached the end of the current text shape
                            // without finding any text. So we need to go back
                            // by one action in order to handle the
                            // XTEXT_PAINTSHAPE_END action because on the next
                            // loop the nCurAction is incremented by one.
                            else
                            {
                                --nCurAction;
                            }

                        }
                    }
                    else if( ( pA->GetComment().equalsIgnoreAsciiCaseL( RTL_CONSTASCII_STRINGPARAM( "XTEXT_EOL" ) ) ) )
                    {
                        const MetaAction* pNextAction = rMtf.GetAction( nCurAction + 1 );
                        if( !( ( pNextAction->GetType() == META_COMMENT_ACTION ) &&
                               ( ( (const MetaCommentAction*) pNextAction )->GetComment().equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XTEXT_EOP") ) ) ) )
                        {
                            // nTextFound == -2 => no text found and end of line reached
                            // nTextFound == -1 => no text found and end of paragraph reached
                            // nTextFound ==  1 => text found!
                            sal_Int32 nTextFound = -2;
                            while( ( nTextFound < -1 ) && ( nCurAction < nCount ) )
                            {
                                nTextFound = maTextWriter.setTextPosition( rMtf, nCurAction );
                            }
                            // We found a line with some text in the current
                            // paragraph.
                            if( nTextFound > 0 )
                            {
                                maTextWriter.startTextPosition();
                            }
                            // We reached the end of the current paragraph
                            // without finding any text. So we need to go back
                            // by one action in order to handle the XTEXT_EOP
                            // action because on the next loop the nCurAction is
                            // incremented by one.
                            else
                            {
                                --nCurAction;
                            }
                        }
                    }
                }
            }
            break;

            case( META_BMP_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpAction* pA = (const MetaBmpAction*) pAction;

                    ImplWriteBmp( pA->GetBitmap(),
                                  pA->GetPoint(), mpVDev->PixelToLogic( pA->GetBitmap().GetSizePixel() ),
                                  Point(), pA->GetBitmap().GetSizePixel() );
                }
            }
            break;

            case( META_BMPSCALE_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;

                    // Bitmaps embedded into text shapes are collected and exported elsewhere.
                    if( maTextWriter.isTextShapeStarted() )
                    {
                        maTextWriter.writeBitmapPlaceholder( pA );
                    }
                    else
                    {
                        ImplWriteBmp( pA->GetBitmap(),
                                      pA->GetPoint(), pA->GetSize(),
                                      Point(), pA->GetBitmap().GetSizePixel() );
                    }
                }
            }
            break;

            case( META_BMPSCALEPART_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpScalePartAction* pA = (const MetaBmpScalePartAction*) pAction;

                    ImplWriteBmp( pA->GetBitmap(),
                                  pA->GetDestPoint(), pA->GetDestSize(),
                                  pA->GetSrcPoint(), pA->GetSrcSize() );
                }
            }
            break;

            case( META_BMPEX_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpExAction*  pA = (const MetaBmpExAction*) pAction;

                    ImplWriteBmp( pA->GetBitmapEx(),
                                  pA->GetPoint(), mpVDev->PixelToLogic( pA->GetBitmapEx().GetSizePixel() ),
                                  Point(), pA->GetBitmapEx().GetSizePixel() );
                }
            }
            break;

            case( META_BMPEXSCALE_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;

                    // Bitmaps embedded into text shapes are collected and exported elsewhere.
                    if( maTextWriter.isTextShapeStarted() )
                    {
                        maTextWriter.writeBitmapPlaceholder( pA );
                    }
                    else
                    {
                        ImplWriteBmp( pA->GetBitmapEx(),
                                      pA->GetPoint(), pA->GetSize(),
                                      Point(), pA->GetBitmapEx().GetSizePixel() );
                    }
                }
            }
            break;

            case( META_BMPEXSCALEPART_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*) pAction;

                    ImplWriteBmp( pA->GetBitmapEx(),
                                  pA->GetDestPoint(), pA->GetDestSize(),
                                  pA->GetSrcPoint(), pA->GetSrcSize() );
                }
            }
            break;

            case( META_TEXT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaTextAction*   pA = (const MetaTextAction*) pAction;
                    const String            aText( pA->GetText(), pA->GetIndex(), pA->GetLen() );

                    if( aText.Len() )
                    {
                        if( mrExport.IsUsePositionedCharacters() )
                        {
                            Font aFont = ImplSetCorrectFontHeight();
                            mpContext->SetFontAttr( aFont );
                            ImplWriteText( pA->GetPoint(), aText, NULL, 0 );
                        }
                        else
                        {
                            maTextWriter.writeTextPortion( pA->GetPoint(), aText );
                        }

                    }
                }
            }
            break;

            case( META_TEXTRECT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaTextRectAction* pA = (const MetaTextRectAction*) pAction;

                    if (!pA->GetText().isEmpty())
                    {
                        if( mrExport.IsUsePositionedCharacters() )
                        {
                            Font aFont = ImplSetCorrectFontHeight();
                            mpContext->SetFontAttr( aFont );
                            ImplWriteText( pA->GetRect().TopLeft(), pA->GetText(), NULL, 0 );
                        }
                        {
                            maTextWriter.writeTextPortion( pA->GetRect().TopLeft(), pA->GetText() );
                        }
                    }
                }
            }
            break;

            case( META_TEXTARRAY_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaTextArrayAction*  pA = (const MetaTextArrayAction*) pAction;
                    const String                aText( pA->GetText(), pA->GetIndex(), pA->GetLen() );

                    if( aText.Len() )
                    {
                        if( mrExport.IsUsePositionedCharacters() )
                        {
                            Font aFont = ImplSetCorrectFontHeight();
                            mpContext->SetFontAttr( aFont );
                            ImplWriteText( pA->GetPoint(), aText, pA->GetDXArray(), 0 );
                        }
                        else
                        {
                            maTextWriter.writeTextPortion( pA->GetPoint(), aText );
                        }
                    }
                }
            }
            break;

            case( META_STRETCHTEXT_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaStretchTextAction*    pA = (const MetaStretchTextAction*) pAction;
                    const String                    aText( pA->GetText(), pA->GetIndex(), pA->GetLen() );

                    if( aText.Len() )
                    {
                        if( mrExport.IsUsePositionedCharacters() )
                        {
                            Font aFont = ImplSetCorrectFontHeight();
                            mpContext->SetFontAttr( aFont );
                            ImplWriteText( pA->GetPoint(), aText, NULL, pA->GetWidth() );
                        }
                        else
                        {
                            maTextWriter.writeTextPortion( pA->GetPoint(), aText );
                        }
                    }
                }
            }
            break;

            case( META_RENDERGRAPHIC_ACTION ):
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    // TODO KA: try to embed the native data in case the RenderGraphic
                    // contains valid SVG data (MimeType "image/svg+xml")
                    // => incorporate 'use' or 'image' element (KA 01/2011)
                    const MetaRenderGraphicAction*          pA = (const MetaRenderGraphicAction*) pAction;
                    const ::vcl::RenderGraphicRasterizer    aRasterizer( pA->GetRenderGraphic() );
                    const Point                             aPointPixel;
                    const Size                              aSizePixel( mpVDev->LogicToPixel( pA->GetSize() ) );
                    const BitmapEx                          aBmpEx( aRasterizer.Rasterize( aSizePixel ) );

                    ImplWriteBmp( aBmpEx, pA->GetPoint(), pA->GetSize(), aPointPixel, aBmpEx.GetSizePixel() );
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
                OSL_FAIL( "SVGActionWriter::ImplWriteActions: unsupported MetaAction #" );
            break;
        }
    }
}

// -----------------------------------------------------------------------------
Font SVGActionWriter::ImplSetCorrectFontHeight() const
{
    Font    aFont( mpVDev->GetFont() );
    Size    aSz;

    ImplMap( Size( 0, aFont.GetHeight() ), aSz );

    aFont.SetHeight( aSz.Height() );

    return aFont;
}

// -----------------------------------------------------------------------------

void SVGActionWriter::WriteMetaFile( const Point& rPos100thmm,
                                     const Size& rSize100thmm,
                                     const GDIMetaFile& rMtf,
                                     sal_uInt32 nWriteFlags,
                                     const ::rtl::OUString* pElementId,
                                     const Reference< XShape >* pXShape,
                                     const GDIMetaFile* pTextEmbeddedBitmapMtf )
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
    ImplAcquireContext();

    mapCurShape.reset();

    ImplWriteActions( rMtf, nWriteFlags, pElementId, pXShape, pTextEmbeddedBitmapMtf );

    // draw open shape that doesn't have a border
    if( mapCurShape.get() )
    {
        ImplWriteShape( *mapCurShape );
        mapCurShape.reset();
    }

    ImplReleaseContext();
    mpVDev->Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
