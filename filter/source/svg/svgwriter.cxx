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

#include "svgfilter.hxx"
#include "svgfontexport.hxx"
#include "svgwriter.hxx"

#include <comphelper/base64.hxx>
#include <rtl/crc.h>
#include <sal/log.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <tools/helpers.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <sax/tools/converter.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <memory>


static const char   aPrefixClipPathId[] = "clip_path_";

static const char   aXMLElemG[] = "g";
static const char   aXMLElemDefs[] = "defs";
static const char   aXMLElemText[] = "text";
static const char   aXMLElemTspan[] = "tspan";
static const char   aXMLElemLinearGradient[] = "linearGradient";
static const char   aXMLElemStop[] = "stop";

static const char   aXMLAttrTransform[] = "transform";
static const char   aXMLAttrStyle[] = "style";
static const char   aXMLAttrId[] = "id";
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
static const char   aXMLAttrStrokeWidth[] = "stroke-width";
static const char   aXMLAttrFill[] = "fill";
static const char   aXMLAttrFontFamily[] = "font-family";
static const char   aXMLAttrFontSize[] = "font-size";
static const char   aXMLAttrFontStyle[] = "font-style";
static const char   aXMLAttrFontWeight[] = "font-weight";
static const char   aXMLAttrTextDecoration[] = "text-decoration";
static const char   aXMLAttrXLinkHRef[] = "xlink:href";
static const char   aXMLAttrGradientUnits[] = "gradientUnits";
static const char   aXMLAttrOffset[] = "offset";
static const char   aXMLAttrStopColor[] = "stop-color";
static const char   aXMLAttrStrokeLinejoin[] = "stroke-linejoin";
static const char   aXMLAttrStrokeLinecap[] = "stroke-linecap";


PushFlags SVGContextHandler::getPushFlags() const
{
    if (maStateStack.empty())
        return PushFlags::NONE;

    const PartialState& rPartialState = maStateStack.top();
    return rPartialState.meFlags;
}

SVGState& SVGContextHandler::getCurrentState()
{
    return maCurrentState;
}

void SVGContextHandler::pushState( PushFlags eFlags )
{
    PartialState aPartialState;
    aPartialState.meFlags = eFlags;

    if (eFlags & PushFlags::FONT)
    {
        aPartialState.setFont( maCurrentState.aFont );
    }

    if (eFlags & PushFlags::CLIPREGION)
    {
        aPartialState.mnRegionClipPathId = maCurrentState.nRegionClipPathId;
    }

    maStateStack.push( std::move(aPartialState) );
}

void SVGContextHandler::popState()
{
    if (maStateStack.empty())
        return;

    const PartialState& rPartialState = maStateStack.top();
    PushFlags eFlags = rPartialState.meFlags;

    if (eFlags & PushFlags::FONT)
    {
        maCurrentState.aFont = rPartialState.getFont( vcl::Font() );
    }

    if (eFlags & PushFlags::CLIPREGION)
    {
        maCurrentState.nRegionClipPathId = rPartialState.mnRegionClipPathId;
    }

    maStateStack.pop();
}

SVGAttributeWriter::SVGAttributeWriter( SVGExport& rExport, SVGFontExport& rFontExport, SVGState& rCurState )
    : mrExport( rExport )
    , mrFontExport( rFontExport )
    , mrCurrentState( rCurState )
{
}


SVGAttributeWriter::~SVGAttributeWriter()
{
}


double SVGAttributeWriter::ImplRound( double fValue )
{
      return floor( fValue * pow( 10.0, 3 ) + 0.5 ) / pow( 10.0, 3 );
}


void SVGAttributeWriter::ImplGetColorStr( const Color& rColor, OUString& rColorStr )
{
    if( rColor.GetTransparency() == 255 )
        rColorStr = "none";
    else
    {
        rColorStr = "rgb(" + OUString::number(rColor.GetRed()) + "," + OUString::number(rColor.GetGreen()) +
                    "," + OUString::number(rColor.GetBlue()) + ")";
    }
}


void SVGAttributeWriter::AddColorAttr( const char* pColorAttrName,
                                       const char* pColorOpacityAttrName,
                                       const Color& rColor )
{
    OUString aColor, aColorOpacity;

    ImplGetColorStr( rColor, aColor );

    if( rColor.GetTransparency() > 0 && rColor.GetTransparency() < 255 )
        aColorOpacity = OUString::number( ImplRound( ( 255.0 - rColor.GetTransparency() ) / 255.0 ) );

    mrExport.AddAttribute( XML_NAMESPACE_NONE, pColorAttrName, aColor );

    if( !aColorOpacity.isEmpty() && mrExport.IsUseOpacity() )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, pColorOpacityAttrName, aColorOpacity );
}


void SVGAttributeWriter::AddPaintAttr( const Color& rLineColor, const Color& rFillColor,
                                       const tools::Rectangle* pObjBoundRect, const Gradient* pFillGradient )
{
    // Fill
    if( pObjBoundRect && pFillGradient )
    {
        OUString aGradientId;

        AddGradientDef( *pObjBoundRect, *pFillGradient, aGradientId );

        if( !aGradientId.isEmpty() )
        {
            OUString aGradientURL = "url(#" + aGradientId + ")";
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFill, aGradientURL );
        }
    }
    else
        AddColorAttr( aXMLAttrFill, "fill-opacity", rFillColor );

    // Stroke
    AddColorAttr( "stroke", "stroke-opacity", rLineColor );
}


void SVGAttributeWriter::AddGradientDef( const tools::Rectangle& rObjRect, const Gradient& rGradient, OUString& rGradientId )
{
    if( rObjRect.GetWidth() && rObjRect.GetHeight() &&
        ( rGradient.GetStyle() == GradientStyle::Linear || rGradient.GetStyle() == GradientStyle::Axial ||
          rGradient.GetStyle() == GradientStyle::Radial || rGradient.GetStyle() == GradientStyle::Elliptical ) )
    {
        SvXMLElementExport aDesc( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, true, true );
        Color aStartColor( rGradient.GetStartColor() ), aEndColor( rGradient.GetEndColor() );
        sal_uInt16 nAngle = rGradient.GetAngle() % 3600;
        Point aObjRectCenter( rObjRect.Center() );
        tools::Polygon aPoly( rObjRect );
        static sal_Int32 nCurGradientId = 1;

        aPoly.Rotate( aObjRectCenter, nAngle );
        tools::Rectangle aRect( aPoly.GetBoundRect() );

        // adjust start/end colors with intensities
        aStartColor.SetRed( static_cast<sal_uInt8>( ( aStartColor.GetRed() * rGradient.GetStartIntensity() ) / 100 ) );
        aStartColor.SetGreen( static_cast<sal_uInt8>( ( aStartColor.GetGreen() * rGradient.GetStartIntensity() ) / 100 ) );
        aStartColor.SetBlue( static_cast<sal_uInt8>( ( aStartColor.GetBlue() * rGradient.GetStartIntensity() ) / 100 ) );

        aEndColor.SetRed( static_cast<sal_uInt8>( ( aEndColor.GetRed() * rGradient.GetEndIntensity() ) / 100 ) );
        aEndColor.SetGreen( static_cast<sal_uInt8>( ( aEndColor.GetGreen() * rGradient.GetEndIntensity() ) / 100 ) );
        aEndColor.SetBlue( static_cast<sal_uInt8>( ( aEndColor.GetBlue() * rGradient.GetEndIntensity() ) / 100 ) );

        rGradientId = "Gradient_" + OUString::number( nCurGradientId++ );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, rGradientId );

        {
            std::unique_ptr< SvXMLElementExport >   apGradient;
            OUString                         aColorStr;

            if( rGradient.GetStyle() == GradientStyle::Linear || rGradient.GetStyle() == GradientStyle::Axial )
            {
                tools::Polygon aLinePoly( 2 );

                aLinePoly[ 0 ] = Point( aObjRectCenter.X(), aRect.Top() );
                aLinePoly[ 1 ] = Point( aObjRectCenter.X(), aRect.Bottom() );

                aLinePoly.Rotate( aObjRectCenter, nAngle );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrGradientUnits, "userSpaceOnUse" );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX1, OUString::number( aLinePoly[ 0 ].X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY1, OUString::number( aLinePoly[ 0 ].Y() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX2, OUString::number( aLinePoly[ 1 ].X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY2, OUString::number( aLinePoly[ 1 ].Y() ) );

                apGradient.reset( new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemLinearGradient, true, true ) );

                // write stop values
                double fBorder = static_cast< double >( rGradient.GetBorder() ) *
                                ( ( rGradient.GetStyle() == GradientStyle::Axial ) ? 0.005 : 0.01 );

                ImplGetColorStr( ( rGradient.GetStyle() == GradientStyle::Axial ) ? aEndColor : aStartColor, aColorStr );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset, OUString::number( fBorder ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStopColor, aColorStr );

                {
                    SvXMLElementExport aDesc2( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, true, true );
                }

                if( rGradient.GetStyle() == GradientStyle::Axial )
                {
                    ImplGetColorStr( aStartColor, aColorStr );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset, OUString::number( 0.5 ) );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStopColor, aColorStr );

                    {
                        SvXMLElementExport aDesc3( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, true, true );
                    }
                }

                if( rGradient.GetStyle() != GradientStyle::Axial )
                    fBorder = 0.0;

                ImplGetColorStr( aEndColor, aColorStr );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset, OUString::number( ImplRound( 1.0 - fBorder ) ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStopColor, aColorStr );

                {
                    SvXMLElementExport aDesc4( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, true, true );
                }
            }
            else
            {
                const double    fCenterX = rObjRect.Left() + rObjRect.GetWidth() * rGradient.GetOfsX() * 0.01;
                const double    fCenterY = rObjRect.Top() + rObjRect.GetHeight() * rGradient.GetOfsY() * 0.01;
                const double    fRadius = sqrt( static_cast< double >( rObjRect.GetWidth() ) * rObjRect.GetWidth() +
                                                rObjRect.GetHeight() * rObjRect.GetHeight() ) * 0.5;

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrGradientUnits, "userSpaceOnUse" );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCX, OUString::number( ImplRound( fCenterX ) ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCY, OUString::number( ImplRound( fCenterY ) ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "r", OUString::number( ImplRound( fRadius ) ) );

                apGradient.reset( new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, "radialGradient", true, true ) );

                // write stop values
                ImplGetColorStr( aEndColor, aColorStr );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset, OUString::number( 0.0 ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStopColor, aColorStr );

                {
                    SvXMLElementExport aDesc5( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, true, true );
                }

                ImplGetColorStr( aStartColor, aColorStr );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset,
                                       OUString::number( ImplRound( 1.0 - rGradient.GetBorder() * 0.01 ) ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStopColor, aColorStr );

                {
                    SvXMLElementExport aDesc6( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, true, true );
                }
            }
        }
    }
    else
        rGradientId.clear();
}


void SVGAttributeWriter::SetFontAttr( const vcl::Font& rFont )
{
    vcl::Font& rCurFont = mrCurrentState.aFont;

    if( rFont != rCurFont )
    {
        OUString  aFontStyle, aTextDecoration;
        sal_Int32        nFontWeight;

        rCurFont = rFont;

        // Font Family
        setFontFamily();

        // Font Size
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontSize,
                               OUString::number( rFont.GetFontHeight() ) + "px" );

        // Font Style
        if( rFont.GetItalic() != ITALIC_NONE )
        {
            if( rFont.GetItalic() == ITALIC_OBLIQUE )
                aFontStyle = "oblique";
            else
                aFontStyle = "italic";
        }
        else
            aFontStyle = "normal";

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

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontWeight, OUString::number( nFontWeight ) );

        if( mrExport.IsUseNativeTextDecoration() )
        {
            if( rFont.GetUnderline() != LINESTYLE_NONE || rFont.GetStrikeout() != STRIKEOUT_NONE )
            {
                if( rFont.GetUnderline() != LINESTYLE_NONE )
                    aTextDecoration = "underline ";

                if( rFont.GetStrikeout() != STRIKEOUT_NONE )
                    aTextDecoration += "line-through ";
            }
            else
                aTextDecoration = "none";

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
        mpElemFont.reset( new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, true ) );
    }
    else
    {
        mpElemFont.reset( new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, true, true ) );
    }
}


void SVGAttributeWriter::endFontSettings()
{
    mpElemFont.reset();
}


void SVGAttributeWriter::setFontFamily()
{
    vcl::Font& rCurFont = mrCurrentState.aFont;

    if( mrExport.IsUsePositionedCharacters() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontFamily, mrFontExport.GetMappedFontName( rCurFont.GetFamilyName() ) );
    }
    else
    {
        const OUString& rsFontName = rCurFont.GetFamilyName();
        OUString sFontFamily( rsFontName.getToken( 0, ';' ) );
        FontPitch ePitch = rCurFont.GetPitch();
        if( ePitch == PITCH_FIXED )
        {
            sFontFamily += ", monospace";
        }
        else
        {
            FontFamily eFamily = rCurFont.GetFamilyType();
            if( eFamily == FAMILY_ROMAN )
                sFontFamily += ", serif";
            else if( eFamily == FAMILY_SWISS )
                sFontFamily += ", sans-serif";
        }
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontFamily, sFontFamily );
    }
}

SVGTextWriter::SVGTextWriter( SVGExport& rExport,  SVGAttributeWriter& rAttributeWriter  )
: mrExport( rExport ),
  mrAttributeWriter( rAttributeWriter ),
  mpVDev( nullptr ),
  mbIsTextShapeStarted( false ),
  mrTextShape(),
  msShapeId(),
  mrParagraphEnumeration(),
  mrCurrentTextParagraph(),
  mrTextPortionEnumeration(),
  mrCurrentTextPortion(),
  mpTextEmbeddedBitmapMtf( nullptr ),
  mpTargetMapMode( nullptr ),
  mnLeftTextPortionLength( 0 ),
  maTextPos(0,0),
  mnTextWidth(0),
  mbPositioningNeeded( false ),
  mbIsNewListItem( false ),
  meNumberingType(0),
  mcBulletChar(0),
  maBulletListItemMap(),
  mbIsListLevelStyleImage( false ),
  mbLineBreak( false ),
  mbIsURLField( false ),
  msUrl(),
  mbIsPlaceholderShape( false ),
  maCurrentFont(),
  maParentFont()
{
}


SVGTextWriter::~SVGTextWriter()
{
    endTextParagraph();
}


void SVGTextWriter::implRegisterInterface( const Reference< XInterface >& rxIf )
{
    if( rxIf.is() )
        mrExport.getInterfaceToIdentifierMapper().registerReference( rxIf );
}


const OUString & SVGTextWriter::implGetValidIDFromInterface( const Reference< XInterface >& rxIf )
{
   return mrExport.getInterfaceToIdentifierMapper().getIdentifier( rxIf );
}


void SVGTextWriter::implMap( const Size& rSz, Size& rDstSz ) const
{
    if( mpVDev && mpTargetMapMode )
        rDstSz = OutputDevice::LogicToLogic( rSz, mpVDev->GetMapMode(), *mpTargetMapMode );
    else
        OSL_FAIL( "SVGTextWriter::implMap: invalid virtual device or map mode." );
}


void SVGTextWriter::implMap( const Point& rPt, Point& rDstPt ) const
{
    if( mpVDev && mpTargetMapMode )
        rDstPt = OutputDevice::LogicToLogic( rPt, mpVDev->GetMapMode(), *mpTargetMapMode );
    else
        OSL_FAIL( "SVGTextWriter::implMap: invalid virtual device or map mode." );
}


void SVGTextWriter::implSetCurrentFont()
{
    if( mpVDev )
    {
        maCurrentFont = mpVDev->GetFont();
        Size aSz;

        implMap( Size( 0, maCurrentFont.GetFontHeight() ), aSz );

        maCurrentFont.SetFontHeight( aSz.Height() );
    }
    else
    {
        OSL_FAIL( "SVGTextWriter::implSetCorrectFontHeight: invalid virtual device." );
    }
}


template< typename SubType >
bool SVGTextWriter::implGetTextPosition( const MetaAction* pAction, Point& raPos, bool& rbEmpty )
{
    const SubType* pA = static_cast<const SubType*>(pAction);
    sal_uInt16 nLength = pA->GetLen();
    rbEmpty = ( nLength == 0 );
    if( !rbEmpty )
    {
        raPos = pA->GetPoint();
        return true;
    }
    return false;
}


template<>
bool SVGTextWriter::implGetTextPosition<MetaTextRectAction>( const MetaAction* pAction, Point& raPos, bool& rbEmpty )
{
    const MetaTextRectAction* pA = static_cast<const MetaTextRectAction*>(pAction);
    sal_uInt16 nLength = pA->GetText().getLength();
    rbEmpty = ( nLength == 0 );
    if( !rbEmpty )
    {
        raPos = pA->GetRect().TopLeft();
        return true;
    }
    return false;
}


template< typename SubType >
bool SVGTextWriter::implGetTextPositionFromBitmap( const MetaAction* pAction, Point& raPos, bool& rbEmpty )
{
    const SubType* pA = static_cast<const SubType*>(pAction);
    raPos = pA->GetPoint();
    rbEmpty = false;
    return true;
}


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
    bool bEOL = false;
    bool bEOP = false;
    bool bETS = false;
    bool bConfigured = false;
    bool bEmpty = true;

    sal_uLong nActionIndex = nCurAction + 1;
    for( ; nActionIndex < nCount; ++nActionIndex )
    {
        const MetaAction*    pAction = rMtf.GetAction( nActionIndex );
        const MetaActionType nType = pAction->GetType();

        switch( nType )
        {
            case MetaActionType::TEXT:
            {
                bConfigured = implGetTextPosition<MetaTextAction>( pAction, aPos, bEmpty );
            }
            break;

            case MetaActionType::TEXTRECT:
            {
                bConfigured = implGetTextPosition<MetaTextRectAction>( pAction, aPos, bEmpty );
            }
            break;

            case MetaActionType::TEXTARRAY:
            {
                bConfigured = implGetTextPosition<MetaTextArrayAction>( pAction, aPos, bEmpty );
            }
            break;

            case MetaActionType::STRETCHTEXT:
            {
                bConfigured = implGetTextPosition<MetaStretchTextAction>( pAction, aPos, bEmpty );
            }
            break;

            case MetaActionType::BMPSCALE:
            {
                bConfigured = implGetTextPositionFromBitmap<MetaBmpScaleAction>( pAction, aPos, bEmpty );
            }
            break;

            case MetaActionType::BMPEXSCALE:
            {
                bConfigured = implGetTextPositionFromBitmap<MetaBmpExScaleAction>( pAction, aPos, bEmpty );
            }
            break;

            // If we reach the end of the current line, paragraph or text shape
            // without finding any text we stop searching
            case MetaActionType::COMMENT:
            {
                const MetaCommentAction* pA = static_cast<const MetaCommentAction*>(pAction);
                const OString& rsComment = pA->GetComment();
                if( rsComment.equalsIgnoreAsciiCase( "XTEXT_EOL" ) )
                {
                    bEOL = true;
                }
                else if( rsComment.equalsIgnoreAsciiCase( "XTEXT_EOP" ) )
                {
                    bEOP = true;

                    OUString sContent;
                    while( nextTextPortion() )
                    {
                        sContent = mrCurrentTextPortion->getString();
                        if( sContent.isEmpty() )
                        {
                            continue;
                        }
                        else
                        {
                            if( sContent == "\n" )
                                mbLineBreak = true;
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
                                if( sContent == "\n" )
                                    mbLineBreak = true;
                            }
                        }
                    }
                }
                else if( rsComment.equalsIgnoreAsciiCase( "XTEXT_PAINTSHAPE_END" ) )
                {
                    bETS = true;
                }
            }
            break;
            default: break;
        }
        if( bConfigured || bEOL || bEOP || bETS ) break;
    }
    implMap( aPos, maTextPos );

    if( bEmpty )
    {
        nCurAction = nActionIndex;
        return ( bEOL ? -2 : ( bEOP ? -1 : 0 ) );
    }
    else
    {
        return 1;
    }
}


void SVGTextWriter::setTextProperties( const GDIMetaFile& rMtf, sal_uLong nCurAction )
{
    sal_uLong nCount = rMtf.GetActionSize();
    bool bEOP = false;
    bool bConfigured = false;
    for( sal_uLong nActionIndex = nCurAction + 1; nActionIndex < nCount; ++nActionIndex )
    {
        const MetaAction*    pAction = rMtf.GetAction( nActionIndex );
        const MetaActionType nType = pAction->GetType();
        switch( nType )
        {
            case MetaActionType::TEXTLINECOLOR:
            case MetaActionType::TEXTFILLCOLOR:
            case MetaActionType::TEXTCOLOR:
            case MetaActionType::TEXTALIGN:
            case MetaActionType::FONT:
            case MetaActionType::LAYOUTMODE:
            {
                const_cast<MetaAction*>(pAction)->Execute( mpVDev );
            }
            break;

            case MetaActionType::TEXT:
            {
                const MetaTextAction* pA = static_cast<const MetaTextAction*>(pAction);
                if( pA->GetLen() > 2 )
                    bConfigured = true;
            }
            break;
            case MetaActionType::TEXTRECT:
            {
                const MetaTextRectAction* pA = static_cast<const MetaTextRectAction*>(pAction);
                if( pA->GetText().getLength() > 2 )
                    bConfigured = true;
            }
            break;
            case MetaActionType::TEXTARRAY:
            {
                const MetaTextArrayAction* pA = static_cast<const MetaTextArrayAction*>(pAction);
                if( pA->GetLen() > 2 )
                    bConfigured = true;
            }
            break;
            case MetaActionType::STRETCHTEXT:
            {
                const MetaStretchTextAction* pA = static_cast<const MetaStretchTextAction*>(pAction);
                if( pA->GetLen() > 2 )
                    bConfigured = true;
            }
            break;
            // If we reach the end of the paragraph without finding any text
            // we stop searching
            case MetaActionType::COMMENT:
            {
                const MetaCommentAction* pA = static_cast<const MetaCommentAction*>(pAction);
                const OString& rsComment = pA->GetComment();
                if( rsComment.equalsIgnoreAsciiCase( "XTEXT_EOP" ) )
                {
                    bEOP = true;
                }
            }
            break;
            default: break;
        }
        if( bConfigured || bEOP ) break;
    }
}


void SVGTextWriter::addFontAttributes( bool bIsTextContainer )
{
    implSetCurrentFont();

    if( maCurrentFont !=  maParentFont )
    {
        const OUString& rsCurFontName               = maCurrentFont.GetFamilyName();
        long int nCurFontSize                       = maCurrentFont.GetFontHeight();
        FontItalic eCurFontItalic                   = maCurrentFont.GetItalic();
        FontWeight eCurFontWeight                   = maCurrentFont.GetWeight();

        const OUString& rsParFontName               = maParentFont.GetFamilyName();
        long int nParFontSize                       = maParentFont.GetFontHeight();
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
                                   OUString::number( nCurFontSize ) +  "px" );
        }

        // Font Style
        if( eCurFontItalic != eParFontItalic )
        {
            OUString sFontStyle;
            if( eCurFontItalic != ITALIC_NONE )
            {
                if( eCurFontItalic == ITALIC_OBLIQUE )
                    sFontStyle = "oblique";
                else
                    sFontStyle = "italic";
            }
            else
            {
                sFontStyle = "normal";
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
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontWeight, OUString::number( nFontWeight ) );
        }


        if( mrExport.IsUseNativeTextDecoration() )
        {
            FontLineStyle eCurFontLineStyle         = maCurrentFont.GetUnderline();
            FontStrikeout eCurFontStrikeout         = maCurrentFont.GetStrikeout();

            FontLineStyle eParFontLineStyle         = maParentFont.GetUnderline();
            FontStrikeout eParFontStrikeout         = maParentFont.GetStrikeout();

            OUString sTextDecoration;
            bool bIsDecorationChanged = false;
            if( eCurFontLineStyle != eParFontLineStyle )
            {
                if( eCurFontLineStyle != LINESTYLE_NONE )
                    sTextDecoration = "underline";
                bIsDecorationChanged = true;
            }
            if( eCurFontStrikeout != eParFontStrikeout )
            {
                if( eCurFontStrikeout != STRIKEOUT_NONE )
                {
                    if( !sTextDecoration.isEmpty() )
                        sTextDecoration += " ";
                    sTextDecoration += "line-through";
                }
                bIsDecorationChanged = true;
            }

            if( !sTextDecoration.isEmpty() )
            {
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTextDecoration, sTextDecoration );
            }
            else if( bIsDecorationChanged )
            {
                sTextDecoration = "none";
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTextDecoration, sTextDecoration );
            }
        }

        if( bIsTextContainer )
            maParentFont = maCurrentFont;

    }
}


void SVGTextWriter::implSetFontFamily()
{
    const OUString& rsFontName = maCurrentFont.GetFamilyName();
    OUString sFontFamily( rsFontName.getToken( 0, ';' ) );
    FontPitch ePitch = maCurrentFont.GetPitch();
    if( ePitch == PITCH_FIXED )
    {
        sFontFamily += ", monospace";
    }
    else
    {
        FontFamily eFamily = maCurrentFont.GetFamilyType();
        if( eFamily == FAMILY_ROMAN )
            sFontFamily += ", serif";
        else if( eFamily == FAMILY_SWISS )
            sFontFamily += ", sans-serif";
    }
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrFontFamily, sFontFamily );
}


void SVGTextWriter::createParagraphEnumeration()
{
    if( mrTextShape.is() )
    {
        msShapeId = implGetValidIDFromInterface( Reference<XInterface>(mrTextShape, UNO_QUERY) );

        Reference< XEnumerationAccess > xEnumerationAccess( mrTextShape, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        if( xEnumeration.is() )
        {
            mrParagraphEnumeration.set( xEnumeration );
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
}


bool SVGTextWriter::nextParagraph()
{
    mrTextPortionEnumeration.clear();
    mrCurrentTextParagraph.clear();
    mbIsNewListItem = false;
    mbIsListLevelStyleImage = false;

    if( mrParagraphEnumeration.is() && mrParagraphEnumeration->hasMoreElements() )
    {
        Reference < XTextContent >  xTextContent( mrParagraphEnumeration->nextElement(), UNO_QUERY_THROW );
        if( xTextContent.is() )
        {
            Reference< XServiceInfo > xServiceInfo( xTextContent, UNO_QUERY_THROW );
#if OSL_DEBUG_LEVEL > 0
            OUString sInfo;
#endif
            if( xServiceInfo->supportsService( "com.sun.star.text.Paragraph" ) )
            {
                mrCurrentTextParagraph.set( xTextContent );
                Reference< XPropertySet > xPropSet( xTextContent, UNO_QUERY_THROW );
                Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
                if( xPropSetInfo->hasPropertyByName( "NumberingLevel" ) )
                {
                    sal_Int16 nListLevel = 0;
                    if( xPropSet->getPropertyValue( "NumberingLevel" ) >>= nListLevel )
                    {
                        mbIsNewListItem = true;
#if OSL_DEBUG_LEVEL > 0
                        sInfo = "NumberingLevel: " + OUString::number( nListLevel );
                        mrExport.AddAttribute( XML_NAMESPACE_NONE, "style", sInfo );
#endif
                        Reference< XIndexReplace > xNumRules;
                        if( xPropSetInfo->hasPropertyByName( "NumberingRules" ) )
                        {
                            xPropSet->getPropertyValue( "NumberingRules" ) >>= xNumRules;
                        }
                        if( xNumRules.is() && ( nListLevel < xNumRules->getCount() ) )
                        {
                            bool bIsNumbered = true;
                            OUString sNumberingIsNumber("NumberingIsNumber");
                            if( xPropSetInfo->hasPropertyByName( sNumberingIsNumber ) )
                            {
                                if( !(xPropSet->getPropertyValue( sNumberingIsNumber ) >>= bIsNumbered ) )
                                {
                                    OSL_FAIL( "numbered paragraph without number info" );
                                    bIsNumbered = false;
                                }
#if OSL_DEBUG_LEVEL > 0
                                if( bIsNumbered )
                                {
                                    sInfo = "true";
                                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "is-numbered", sInfo );
                                }
#endif
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
                                        if( rProp.Name == "NumberingType" )
                                        {
                                            rProp.Value >>= eType;
                                        }
                                        else if( rProp.Name == "BulletChar" )
                                        {
                                            OUString sValue;
                                            rProp.Value >>= sValue;
                                            if( !sValue.isEmpty() )
                                            {
                                                cBullet = sValue[0];
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
#if OSL_DEBUG_LEVEL > 0
                                            sInfo = OUString::number( static_cast<sal_Int32>(cBullet) );
                                            mrExport.AddAttribute( XML_NAMESPACE_NONE, "bullet-char", sInfo );
#endif
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
#if OSL_DEBUG_LEVEL > 0
                sInfo = "Paragraph";
#endif
            }
            else if( xServiceInfo->supportsService( "com.sun.star.text.Table" ) )
            {
                OSL_FAIL( "SVGTextWriter::nextParagraph: text tables are not handled." );
#if OSL_DEBUG_LEVEL > 0
                sInfo = "Table";
#endif
            }
            else
            {
                OSL_FAIL( "SVGTextWriter::nextParagraph: Unknown text content." );
                return false;
            }
#if OSL_DEBUG_LEVEL > 0
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", sInfo );
            SvXMLElementExport aParaElem( mrExport, XML_NAMESPACE_NONE, "desc", mbIWS, mbIWS );
#endif
        }
        else
        {
            OSL_FAIL( "SVGTextWriter::nextParagraph: no XServiceInfo interface available for text content." );
            return false;
        }

        const OUString& rParagraphId = implGetValidIDFromInterface( Reference<XInterface>(xTextContent, UNO_QUERY) );
        if( !rParagraphId.isEmpty() )
        {
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", rParagraphId );
        }
        return true;
    }

    return false;
}


bool SVGTextWriter::nextTextPortion()
{
    mrCurrentTextPortion.clear();
    mbIsURLField = false;
    mbIsPlaceholderShape = false;
    if( mrTextPortionEnumeration.is() && mrTextPortionEnumeration->hasMoreElements() )
    {
#if OSL_DEBUG_LEVEL > 0
        OUString sInfo;
#endif
        Reference< XPropertySet > xPortionPropSet( mrTextPortionEnumeration->nextElement(), UNO_QUERY );
        Reference< XPropertySetInfo > xPortionPropInfo( xPortionPropSet->getPropertySetInfo() );
        Reference < XTextRange > xPortionTextRange( xPortionPropSet, UNO_QUERY);
        if( xPortionPropSet.is() && xPortionPropInfo.is()
                && xPortionPropInfo->hasPropertyByName( "TextPortionType" ) )
        {
#if OSL_DEBUG_LEVEL > 0
            OUString sPortionType;
            if( xPortionPropSet->getPropertyValue( "TextPortionType" ) >>= sPortionType )
            {
                sInfo = "type: " + sPortionType + "; ";
            }
#endif
            if( xPortionTextRange.is() )
            {
#if OSL_DEBUG_LEVEL > 0
                sInfo += "content: " + xPortionTextRange->getString() + "; ";
#endif
                mrCurrentTextPortion.set( xPortionTextRange );

                Reference < XPropertySet > xRangePropSet( xPortionTextRange, UNO_QUERY );
                if( xRangePropSet.is() && xRangePropSet->getPropertySetInfo()->hasPropertyByName( "TextField" ) )
                {
                    Reference < XTextField > xTextField( xRangePropSet->getPropertyValue( "TextField" ), UNO_QUERY );
                    if( xTextField.is() )
                    {
                        const OUString sServicePrefix("com.sun.star.text.textfield.");
                        const OUString sPresentationServicePrefix("com.sun.star.presentation.TextField.");

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
                            else if( pNames->startsWith( sPresentationServicePrefix ) )
                            {
                                // TextField found => postfix is field type!
                                sFieldName = pNames->copy( sPresentationServicePrefix.getLength() );
                                break;
                            }

                            ++pNames;
                        }

#if OSL_DEBUG_LEVEL > 0
                        sInfo += "text field type: " + sFieldName + "; content: " + xTextField->getPresentation( /* show command: */ false ) + "; ";
#endif
                        if( sFieldName == "DateTime" || sFieldName == "Header"
                                || sFieldName == "Footer" || sFieldName == "PageNumber" )
                        {
                            mbIsPlaceholderShape = true;
                        }
                        else
                        {
                            mbIsURLField = sFieldName == "URL";

                            if( mbIsURLField )
                            {
                                Reference<XPropertySet> xTextFieldPropSet(xTextField, UNO_QUERY);
                                if( xTextFieldPropSet.is() )
                                {
                                    OUString sURL;
                                    if( ( xTextFieldPropSet->getPropertyValue( sFieldName ) ) >>= sURL )
                                    {
#if OSL_DEBUG_LEVEL > 0
                                        sInfo += "url: ";
                                        sInfo += mrExport.GetRelativeReference( sURL );
#endif
                                        msUrl = mrExport.GetRelativeReference( sURL );
                                        if( !msUrl.isEmpty() )
                                        {
                                            implRegisterInterface( xPortionTextRange );

                                            const OUString& rTextPortionId = implGetValidIDFromInterface( Reference<XInterface>(xPortionTextRange, UNO_QUERY) );
                                            if( !rTextPortionId.isEmpty() )
                                            {
                                                msHyperlinkIdList += rTextPortionId + " ";
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
#if OSL_DEBUG_LEVEL > 0
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "TextPortion" );
            SvXMLElementExport aPortionElem( mrExport, XML_NAMESPACE_NONE, "desc", mbIWS, mbIWS );
            mrExport.GetDocHandler()->characters( sInfo );
#endif
            return true;
        }
    }

    return false;
}


void SVGTextWriter::startTextShape()
{
    if( mpTextShapeElem )
    {
        OSL_FAIL( "SVGTextWriter::startTextShape: text shape already defined." );
    }

    {
        mbIsTextShapeStarted = true;
        maParentFont = vcl::Font();
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "TextShape" );

        // if text is rotated, set transform matrix at text element
        const vcl::Font& rFont = mpVDev->GetFont();
        if( rFont.GetOrientation() )
            {
                Point   aRot( maTextPos );
                OUString aTransform = "rotate(" +
                    OUString::number( rFont.GetOrientation() * -0.1 ) + " " +
                    OUString::number( aRot.X() ) + " " +
                    OUString::number( aRot.Y() ) + ")";
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTransform, aTransform );
            }

        mpTextShapeElem.reset(new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemText, true, mbIWS ));
        startTextParagraph();
    }
}


void SVGTextWriter::endTextShape()
{
    endTextParagraph();
    mrTextShape.clear();
    mrParagraphEnumeration.clear();
    mrCurrentTextParagraph.clear();
    mpTextShapeElem.reset();
    mbIsTextShapeStarted = false;
    // these need to be invoked after the <text> element has been closed
    implExportHyperlinkIds();
    implWriteBulletChars();
    implWriteEmbeddedBitmaps();

}


void SVGTextWriter::startTextParagraph()
{
    endTextParagraph();
    nextParagraph();
    if( mbIsNewListItem )
    {
        OUString sNumberingType;
        switch( meNumberingType )
        {
            case NumberingType::CHAR_SPECIAL:
                    sNumberingType = "bullet-style";
                    break;
            case NumberingType::BITMAP:
                    sNumberingType = "image-style";
                    break;
            default:
                    sNumberingType = "number-style";
                    break;
        }
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "ooo:numbering-type", sNumberingType );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "ListItem" );
    }
    else
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "TextParagraph" );
    }
    maParentFont = vcl::Font();
    addFontAttributes( /* isTexTContainer: */ true );
    mpTextParagraphElem.reset(new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, mbIWS, mbIWS ));

    if( !mbIsListLevelStyleImage )
    {
        mbPositioningNeeded = true;
    }
}


void SVGTextWriter::endTextParagraph()
{
    mrCurrentTextPortion.clear();
    endTextPosition();
    mbIsNewListItem = false;
    mbIsListLevelStyleImage = false;
    mbPositioningNeeded = false;
    mpTextParagraphElem.reset();
}


void SVGTextWriter::startTextPosition( bool bExportX, bool bExportY )
{
    endTextPosition();
    mnTextWidth = 0;
    mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "TextPosition" );
    if( bExportX )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, OUString::number( maTextPos.X() ) );
    if( bExportY )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, OUString::number( maTextPos.Y() ) );

    mpTextPositionElem.reset( new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, mbIWS, mbIWS ) );
}


void SVGTextWriter::endTextPosition()
{
    mpTextPositionElem.reset();
}


void SVGTextWriter::implExportHyperlinkIds()
{
    if( !msHyperlinkIdList.isEmpty() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "HyperlinkIdList" );
        SvXMLElementExport aDescElem( mrExport, XML_NAMESPACE_NONE, "desc", true, false );
        mrExport.GetDocHandler()->characters( msHyperlinkIdList.trim() );
        msHyperlinkIdList.clear();
    }
}


void SVGTextWriter::implWriteBulletChars()
{
    if( maBulletListItemMap.empty() )
        return;

    mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "BulletChars" );
    SvXMLElementExport aGroupElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, true );

    OUString sId, sPosition, sScaling, sRefId;
    for (auto const& bulletListItem : maBulletListItemMap)
    {
        // <g id="?" > (used by animations)
        // As id we use the id of the text portion placeholder with prefix
        // bullet-char-*
        sId = "bullet-char-" + bulletListItem.first;
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", sId );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "BulletChar" );
        SvXMLElementExport aBulletCharElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, true );

        // <g transform="translate(x,y)" >
        {
            const BulletListItemInfo& rInfo = bulletListItem.second;

            // Add positioning attribute through a translation
            sPosition = "translate(" +
                        OUString::number( rInfo.aPos.X() ) +
                        "," + OUString::number( rInfo.aPos.Y() ) + ")";
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "transform", sPosition );

            mrAttributeWriter.AddPaintAttr( COL_TRANSPARENT, rInfo.aColor );

            SvXMLElementExport aPositioningElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, true );

            // <use transform="scale(font-size)" xlink:ref="/" >
            {
                // Add size attribute through a scaling
                sScaling = "scale(" + OUString::number( rInfo.nFontSize ) +
                           "," + OUString::number( rInfo.nFontSize )+ ")";
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "transform", sScaling );

                // Add ref attribute
                sRefId = "#bullet-char-template-" +
                         OUString::number( ( rInfo.cBulletChar ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrXLinkHRef, sRefId );

                SvXMLElementExport aRefElem( mrExport, XML_NAMESPACE_NONE, "use", true, true );
            }
        } // close aPositioningElem
    }

    // clear the map
    maBulletListItemMap.clear();
}


template< typename MetaBitmapActionType >
void SVGTextWriter::writeBitmapPlaceholder( const MetaBitmapActionType* pAction )
{
    // text position element
    const Point& rPos = pAction->GetPoint();
    implMap( rPos, maTextPos );
    startTextPosition();
    mbPositioningNeeded = true;
    if( mbIsNewListItem )
    {
        mbIsNewListItem = false;
        mbIsListLevelStyleImage = false;
    }

    // bitmap placeholder element
    BitmapChecksum nId = SVGActionWriter::GetChecksum( pAction );
    OUString sId = "bitmap-placeholder("  + msShapeId + "." +
                   OUString::number( nId ) + ")";

    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", sId );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "BitmapPlaceholder" );
        SvXMLElementExport aSVGTspanElem( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, mbIWS, mbIWS );
    }
    endTextPosition();
}


void SVGTextWriter::implWriteEmbeddedBitmaps()
{
    if( mpTextEmbeddedBitmapMtf && mpTextEmbeddedBitmapMtf->GetActionSize() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "EmbeddedBitmaps" );
        SvXMLElementExport aEmbBitmapGroupElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, true );

        const GDIMetaFile& rMtf = *mpTextEmbeddedBitmapMtf;

        BitmapChecksum nId, nChecksum = 0;
        Point aPt;
        Size  aSz;
        sal_uLong nCount = rMtf.GetActionSize();
        for( sal_uLong nCurAction = 0; nCurAction < nCount; nCurAction++ )
        {

            const MetaAction*    pAction = rMtf.GetAction( nCurAction );
            const MetaActionType nType = pAction->GetType();

            switch( nType )
            {
                case MetaActionType::BMPSCALE:
                {
                    const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pAction);
                    nChecksum = pA->GetBitmap().GetChecksum();
                    aPt = pA->GetPoint();
                    aSz = pA->GetSize();
                }
                break;
                case MetaActionType::BMPEXSCALE:
                {
                    const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pAction);
                    nChecksum = pA->GetBitmapEx().GetChecksum();
                    aPt = pA->GetPoint();
                    aSz = pA->GetSize();
                }
                break;
                default: break;
            }

            // <g id="?" > (used by animations)
            {
                // embedded bitmap id
                nId = SVGActionWriter::GetChecksum( pAction );
                OUString sId = "embedded-bitmap(" + msShapeId + "." + OUString::number( nId ) + ")";
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", sId );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "EmbeddedBitmap" );

                SvXMLElementExport aEmbBitmapElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, true );

                // <use x="?" y="?" xlink:ref="?" >
                {
                    // referenced bitmap template
                    OUString sRefId = "#bitmap(" + OUString::number( nChecksum ) + ")";

                    Point aPoint;
                    Size  aSize;
                    implMap( aPt, aPoint );
                    implMap( aSz, aSize );

                    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, OUString::number( aPoint.X() ) );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, OUString::number( aPoint.Y() ) );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrXLinkHRef, sRefId );

                    SvXMLElementExport aRefElem( mrExport, XML_NAMESPACE_NONE, "use", true, true );
                }
            } // close aEmbBitmapElem
        }
    }
}


void SVGTextWriter::writeTextPortion( const Point& rPos,
                                      const OUString& rText )
{
    if( rText.isEmpty() )
        return;

    bool bStandAloneTextPortion = false;
    if( !isTextShapeStarted() )
    {
        bStandAloneTextPortion = true;
        startTextShape();
    }

    mbLineBreak = false;

    if( !mbIsNewListItem || mbIsListLevelStyleImage )
    {
        bool bNotSync = true;
        OUString sContent;
        sal_Int32 nStartPos;
        while( bNotSync )
        {
            if( mnLeftTextPortionLength <= 0  || !mrCurrentTextPortion.is() )
            {
                if( !nextTextPortion() )
                    break;
                else
                {
                    sContent = mrCurrentTextPortion->getString();
                    if( mbIsURLField && sContent.isEmpty() )
                    {
                        Reference < XPropertySet > xPropSet( mrCurrentTextPortion, UNO_QUERY );
                        Reference < XTextField > xTextField( xPropSet->getPropertyValue( "TextField" ), UNO_QUERY );
                        sContent = xTextField->getPresentation( /* show command: */ false );
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
            mnLeftTextPortionLength -= rText.getLength();

            if( sContent.isEmpty() )
                continue;
            if( sContent == "\n" )
                mbLineBreak = true;
            if( sContent.match( rText, nStartPos ) )
                bNotSync = false;
        }
    }

    assert(mpVDev); //invalid virtual device

#if 0
    const FontMetric aMetric( mpVDev->GetFontMetric() );

    bool bTextSpecial = aMetric.IsShadow() || aMetric.IsOutline() || (aMetric.GetRelief() != FontRelief::NONE);

    if( true || !bTextSpecial )
    {
        implWriteTextPortion( rPos, rText, mpVDev->GetTextColor() );
    }
    else
    {
        // to be implemented
    }
#else
    implWriteTextPortion( rPos, rText, mpVDev->GetTextColor() );
#endif

    if( bStandAloneTextPortion )
    {
        endTextShape();
    }
}


void SVGTextWriter::implWriteTextPortion( const Point& rPos,
                                          const OUString& rText,
                                          Color aTextColor )
{
    Point                                   aPos;
    Point                                   aBaseLinePos( rPos );
    const FontMetric                        aMetric( mpVDev->GetFontMetric() );
    const vcl::Font&                        rFont = mpVDev->GetFont();

    if( rFont.GetAlignment() == ALIGN_TOP )
        aBaseLinePos.AdjustY(aMetric.GetAscent() );
    else if( rFont.GetAlignment() == ALIGN_BOTTOM )
        aBaseLinePos.AdjustY( -(aMetric.GetDescent()) );

    implMap( rPos, aPos );

    if( mbPositioningNeeded )
    {
        mbPositioningNeeded = false;
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
            mbLineBreak = false;
            maTextPos.setX( aPos.X() );
            maTextPos.setY( aPos.Y() );
            startTextPosition();
        }
        else // superscript, subscript, list item numbering
        {
            maTextPos.setY( aPos.Y() );
            startTextPosition( false /* do not export x attribute */ );
        }
    }
    // we are dealing with a bullet, so set up this for the next text portion
    if( mbIsNewListItem )
    {
        mbIsNewListItem = false;
        mbPositioningNeeded = true;

        if( meNumberingType == NumberingType::CHAR_SPECIAL )
        {
            // Create an id for the current text portion
            implRegisterInterface( mrCurrentTextParagraph );

            // Add the needed info to the BulletListItemMap
            OUString sId = implGetValidIDFromInterface( Reference<XInterface>(mrCurrentTextParagraph, UNO_QUERY) );
            if( !sId.isEmpty() )
            {
                sId += ".bp";
                BulletListItemInfo& aBulletListItemInfo = maBulletListItemMap[ sId ];
                aBulletListItemInfo.nFontSize = rFont.GetFontHeight();
                aBulletListItemInfo.aColor = aTextColor;
                aBulletListItemInfo.aPos = maTextPos;
                aBulletListItemInfo.cBulletChar = mcBulletChar;

                // Make this text portion a bullet placeholder
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", sId );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "BulletPlaceholder" );
                SvXMLElementExport aSVGTspanElem( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, mbIWS, mbIWS );
                return;
            }
        }
    }

    const OUString& rTextPortionId = implGetValidIDFromInterface( Reference<XInterface>(mrCurrentTextPortion, UNO_QUERY) );
    if( !rTextPortionId.isEmpty() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", rTextPortionId );
    }

    if( mbIsPlaceholderShape )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "PlaceholderText" );
        mbIsPlaceholderShape = false;
    }

    addFontAttributes( /* isTexTContainer: */ false );

    mrAttributeWriter.AddPaintAttr( COL_TRANSPARENT, aTextColor );

    // <a> tag for link should be the innermost tag, inside <tspan>
    if( !mbIsPlaceholderShape && mbIsURLField && !msUrl.isEmpty() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "UrlField" );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrXLinkHRef, msUrl );

        SvXMLElementExport aSVGTspanElem( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, mbIWS, mbIWS );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrXLinkHRef, msUrl );
        {
            SvXMLElementExport aSVGAElem( mrExport, XML_NAMESPACE_NONE, "a", mbIWS, mbIWS );
            mrExport.GetDocHandler()->characters( rText );
        }
    }
    else
    {
        SvXMLElementExport aSVGTspanElem( mrExport, XML_NAMESPACE_NONE, aXMLElemTspan, mbIWS, mbIWS );
        mrExport.GetDocHandler()->characters( rText );
    }

    mnTextWidth += mpVDev->GetTextWidth( rText );
}


SVGActionWriter::SVGActionWriter( SVGExport& rExport, SVGFontExport& rFontExport ) :
    mnCurGradientId( 1 ),
    mnCurMaskId( 1 ),
    mnCurPatternId( 1 ),
    mnCurClipPathId( 1 ),
    mpCurrentClipRegionElem(),
    mrExport( rExport ),
    maContextHandler(),
    mrCurrentState( maContextHandler.getCurrentState() ),
    maAttributeWriter( rExport, rFontExport, mrCurrentState ),
    maTextWriter( rExport, maAttributeWriter ),
    mbClipAttrChanged( false ),
    mbIsPlaceholderShape( false )
{
    mpVDev = VclPtr<VirtualDevice>::Create();
    mpVDev->EnableOutput( false );
    maTargetMapMode = MapMode(MapUnit::Map100thMM);
    maTextWriter.setVirtualDevice( mpVDev, maTargetMapMode );
}


SVGActionWriter::~SVGActionWriter()
{
    mpVDev.disposeAndClear();
}


long SVGActionWriter::ImplMap( sal_Int32 nVal ) const
{
    Size aSz( nVal, nVal );

    return ImplMap( aSz, aSz ).Width();
}


Point& SVGActionWriter::ImplMap( const Point& rPt, Point& rDstPt ) const
{
    return( rDstPt = OutputDevice::LogicToLogic( rPt, mpVDev->GetMapMode(), maTargetMapMode ) );
}


Size& SVGActionWriter::ImplMap( const Size& rSz, Size& rDstSz ) const
{
    return( rDstSz = OutputDevice::LogicToLogic( rSz, mpVDev->GetMapMode(), maTargetMapMode ) );
}


void SVGActionWriter::ImplMap( const tools::Rectangle& rRect, tools::Rectangle& rDstRect ) const
{
    Point   aTL( rRect.TopLeft() );
    Size    aSz( rRect.GetSize() );

    rDstRect = tools::Rectangle( ImplMap( aTL, aTL ), ImplMap( aSz, aSz ) );
}


tools::Polygon& SVGActionWriter::ImplMap( const tools::Polygon& rPoly, tools::Polygon& rDstPoly ) const
{
    rDstPoly = tools::Polygon( rPoly.GetSize() );

    for( sal_uInt16 i = 0, nSize = rPoly.GetSize(); i < nSize; ++i )
    {
        ImplMap( rPoly[ i ], rDstPoly[ i ] );
        rDstPoly.SetFlags( i, rPoly.GetFlags( i ) );
    }

    return rDstPoly;
}


tools::PolyPolygon& SVGActionWriter::ImplMap( const tools::PolyPolygon& rPolyPoly, tools::PolyPolygon& rDstPolyPoly ) const
{
    tools::Polygon aPoly;

    rDstPolyPoly = tools::PolyPolygon();

    for( sal_uInt16 i = 0, nCount = rPolyPoly.Count(); i < nCount; ++i )
    {
        rDstPolyPoly.Insert( ImplMap( rPolyPoly[ i ], aPoly ) );
    }

    return rDstPolyPoly;
}


OUString SVGActionWriter::GetPathString( const tools::PolyPolygon& rPolyPoly, bool bLine )
{
    OUStringBuffer   aPathData;
    const OUString   aBlank( " " );
    const OUString   aComma( "," );
    Point                      aPolyPoint;

    for( long i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
    {
        const tools::Polygon&  rPoly = rPolyPoly[ static_cast<sal_uInt16>(i) ];
        sal_uInt16 n = 1, nSize = rPoly.GetSize();

        if( nSize > 1 )
        {
            aPolyPoint = rPoly[ 0 ];
            aPathData.append("M ")
                     .append(OUString::number( aPolyPoint.X() ))
                     .append(aComma)
                     .append(OUString::number( aPolyPoint.Y() ));

            sal_Char nCurrentMode = 0;
            const bool bClose(!bLine || rPoly[0] == rPoly[nSize - 1]);
            while( n < nSize )
            {
                aPathData.append(aBlank);

                if ( ( rPoly.GetFlags( n ) == PolyFlags::Control ) && ( ( n + 2 ) < nSize ) )
                {
                    if ( nCurrentMode != 'C' )
                    {
                        nCurrentMode = 'C';
                        aPathData.append("C ");
                    }
                    for ( int j = 0; j < 3; j++ )
                    {
                        if ( j )
                            aPathData.append(aBlank);

                        aPolyPoint = rPoly[ n++ ];
                        aPathData.append(OUString::number( aPolyPoint.X() ))
                                 .append(aComma)
                                 .append(OUString::number( aPolyPoint.Y() ));
                    }
                }
                else
                {
                    if ( nCurrentMode != 'L' )
                    {
                        nCurrentMode = 'L';
                        aPathData.append("L ");
                    }

                    aPolyPoint = rPoly[ n++ ];
                    aPathData.append(OUString::number( aPolyPoint.X() ))
                             .append(aComma)
                             .append(OUString::number( aPolyPoint.Y() ));
                }
            }

            if(bClose)
                aPathData.append(" Z");

            if( i < ( nCount - 1 ) )
                aPathData.append(aBlank);
        }
    }

    return aPathData.makeStringAndClear();
}


BitmapChecksum SVGActionWriter::GetChecksum( const MetaAction* pAction )
{
    GDIMetaFile aMtf;
    MetaAction* pA = const_cast<MetaAction*>(pAction);
    aMtf.AddAction( pA );
    return aMtf.GetChecksum();
}


void SVGActionWriter::ImplWriteLine( const Point& rPt1, const Point& rPt2,
                                     const Color* pLineColor )
{
    Point aPt1, aPt2;

    ImplMap( rPt1, aPt1 );
    ImplMap( rPt2, aPt2 );

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX1, OUString::number( aPt1.X() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY1, OUString::number( aPt1.Y() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX2, OUString::number( aPt2.X() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY2, OUString::number( aPt2.Y() ) );

    if( pLineColor )
    {
        // !!! mrExport.AddAttribute( XML_NAMESPACE_NONE, ... )
        OSL_FAIL( "SVGActionWriter::ImplWriteLine: Line color not implemented" );
    }

    {
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, "line", true, true );
    }
}


void SVGActionWriter::ImplWriteRect( const tools::Rectangle& rRect, long nRadX, long nRadY )
{
    tools::Rectangle aRect;

    ImplMap( rRect, aRect );

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, OUString::number( aRect.Left() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, OUString::number( aRect.Top() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, OUString::number( aRect.GetWidth() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, OUString::number( aRect.GetHeight() ) );

    if( nRadX )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRX, OUString::number( ImplMap( nRadX ) ) );

    if( nRadY )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRY, OUString::number( ImplMap( nRadY ) ) );

    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, "rect", true, true );
}


void SVGActionWriter::ImplWriteEllipse( const Point& rCenter, long nRadX, long nRadY )
{
    Point aCenter;

    ImplMap( rCenter, aCenter );

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCX, OUString::number( aCenter.X() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrCY, OUString::number( aCenter.Y() ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRX, OUString::number( ImplMap( nRadX ) ) );
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrRY, OUString::number( ImplMap( nRadY ) ) );

    {
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, "ellipse", true, true );
    }
}


void SVGActionWriter::ImplAddLineAttr( const LineInfo &rAttrs )
{
    if ( !rAttrs.IsDefault() )
    {
        sal_Int32 nStrokeWidth = ImplMap( rAttrs.GetWidth() );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStrokeWidth,
                               OUString::number( nStrokeWidth ) );
        // support for LineJoint
        switch(rAttrs.GetLineJoin())
        {
            case basegfx::B2DLineJoin::NONE:
            case basegfx::B2DLineJoin::Miter:
            {
                mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinejoin, "miter");
                break;
            }
            case basegfx::B2DLineJoin::Bevel:
            {
                mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinejoin, "bevel");
                break;
            }
            case basegfx::B2DLineJoin::Round:
            {
                mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinejoin, "round");
                break;
            }
        }

        // support for LineCap
        switch(rAttrs.GetLineCap())
        {
            default: /* css::drawing::LineCap_BUTT */
            {
                // butt is Svg default, so no need to write until the exporter might write styles.
                // If this happens, activate here
                // mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinecap, "butt");
                break;
            }
            case css::drawing::LineCap_ROUND:
            {
                mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinecap, "round");
                break;
            }
            case css::drawing::LineCap_SQUARE:
            {
                mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinecap, "square");
                break;
            }
        }

    }

}


void SVGActionWriter::ImplWritePolyPolygon( const tools::PolyPolygon& rPolyPoly, bool bLineOnly,
                                            bool bApplyMapping )
{
    tools::PolyPolygon aPolyPoly;

    if( bApplyMapping )
        ImplMap( rPolyPoly, aPolyPoly );
    else
        aPolyPoly = rPolyPoly;

    // add path data attribute
    mrExport.AddAttribute( XML_NAMESPACE_NONE, "d", GetPathString( aPolyPoly, bLineOnly ) );

    {
        // write polyline/polygon element
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, "path", true, true );
    }
}


void SVGActionWriter::ImplWriteShape( const SVGShapeDescriptor& rShape )
{
    tools::PolyPolygon aPolyPoly;

    ImplMap( rShape.maShapePolyPoly, aPolyPoly );

    const bool bLineOnly
        = (rShape.maShapeFillColor == COL_TRANSPARENT) && (!rShape.mapShapeGradient);
    tools::Rectangle   aBoundRect( aPolyPoly.GetBoundRect() );

    maAttributeWriter.AddPaintAttr( rShape.maShapeLineColor, rShape.maShapeFillColor, &aBoundRect, rShape.mapShapeGradient.get() );

    if( !rShape.maId.isEmpty() )
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, rShape.maId );

    if( rShape.mnStrokeWidth )
    {
        sal_Int32 nStrokeWidth = ImplMap( rShape.mnStrokeWidth );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStrokeWidth, OUString::number( nStrokeWidth ) );
    }

    // support for LineJoin
    switch(rShape.maLineJoin)
    {
        case basegfx::B2DLineJoin::NONE:
        case basegfx::B2DLineJoin::Miter:
        {
            // miter is Svg default, so no need to write until the exporter might write styles.
            // If this happens, activate here
            // mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinejoin, "miter");
            break;
        }
        case basegfx::B2DLineJoin::Bevel:
        {
            mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinejoin, "bevel");
            break;
        }
        case basegfx::B2DLineJoin::Round:
        {
            mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinejoin, "round");
            break;
        }
    }

    // support for LineCap
    switch(rShape.maLineCap)
    {
        default: /* css::drawing::LineCap_BUTT */
        {
            // butt is Svg default, so no need to write until the exporter might write styles.
            // If this happens, activate here
            // mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinecap, "butt");
            break;
        }
        case css::drawing::LineCap_ROUND:
        {
            mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinecap, "round");
            break;
        }
        case css::drawing::LineCap_SQUARE:
        {
            mrExport.AddAttribute(XML_NAMESPACE_NONE, aXMLAttrStrokeLinecap, "square");
            break;
        }
    }

    if( !rShape.maDashArray.empty() )
    {
        OUStringBuffer   aDashArrayStr;

        for( size_t k = 0; k < rShape.maDashArray.size(); ++k )
        {
            const sal_Int32 nDash = ImplMap( FRound( rShape.maDashArray[ k ] ) );

            if( k )
                aDashArrayStr.append(",");

            aDashArrayStr.append(OUString::number( nDash ));
        }

        mrExport.AddAttribute( XML_NAMESPACE_NONE, "stroke-dasharray", aDashArrayStr.makeStringAndClear() );
    }

    ImplWritePolyPolygon( aPolyPoly, bLineOnly, false );
}



void SVGActionWriter::ImplCreateClipPathDef( const tools::PolyPolygon& rPolyPoly )
{
    OUString aClipPathId = aPrefixClipPathId + OUString::number( mnCurClipPathId++ );

    SvXMLElementExport aElemDefs( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, true, true );

    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, aClipPathId );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "clipPathUnits", "userSpaceOnUse" );
        SvXMLElementExport aElemClipPath( mrExport, XML_NAMESPACE_NONE, "clipPath", true, true );

        ImplWritePolyPolygon(rPolyPoly, false);
    }
}

void SVGActionWriter::ImplStartClipRegion(sal_Int32 nClipPathId)
{
    assert(!mpCurrentClipRegionElem);

    if (nClipPathId == 0)
        return;

    OUString aUrl = OUStringLiteral("url(#") + aPrefixClipPathId + OUString::number( nClipPathId ) + ")";
    mrExport.AddAttribute( XML_NAMESPACE_NONE, "clip-path", aUrl );
    mpCurrentClipRegionElem.reset( new SvXMLElementExport( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, true ) );
}

void SVGActionWriter::ImplEndClipRegion()
{
    if (mpCurrentClipRegionElem)
    {
        mpCurrentClipRegionElem.reset();
    }
}

void SVGActionWriter::ImplWriteClipPath( const tools::PolyPolygon& rPolyPoly )
{
    ImplEndClipRegion();

    if( rPolyPoly.Count() == 0 )
        return;

    ImplCreateClipPathDef(rPolyPoly);
    mrCurrentState.nRegionClipPathId = mnCurClipPathId - 1;
    ImplStartClipRegion( mrCurrentState.nRegionClipPathId );
}

void SVGActionWriter::ImplWritePattern( const tools::PolyPolygon& rPolyPoly,
                                        const Hatch* pHatch,
                                        const Gradient* pGradient,
                                        sal_uInt32 nWriteFlags )
{
    if( rPolyPoly.Count() )
    {
        SvXMLElementExport aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, true );

        OUString aPatternId = "pattern" + OUString::number( mnCurPatternId++ );

        {
            SvXMLElementExport aElemDefs( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, true, true );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, aPatternId );

            tools::Rectangle aRect;
            ImplMap( rPolyPoly.GetBoundRect(), aRect );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, OUString::number( aRect.Left() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, OUString::number( aRect.Top() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, OUString::number( aRect.GetWidth() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, OUString::number( aRect.GetHeight() ) );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, "patternUnits", OUString( "userSpaceOnUse") );

            {
                SvXMLElementExport aElemPattern( mrExport, XML_NAMESPACE_NONE, "pattern", true, true );

                // The origin of a pattern is positioned at (aRect.Left(), aRect.Top()).
                // So we need to adjust the pattern coordinate.
                OUString aTransform = "translate(" +
                                      OUString::number( -aRect.Left() ) +
                                      "," + OUString::number( -aRect.Top() ) +
                                      ")";
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTransform, aTransform );

                {
                    SvXMLElementExport aElemG2( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, true );

                    GDIMetaFile aTmpMtf;
                    if( pHatch )
                        mpVDev->AddHatchActions( rPolyPoly, *pHatch, aTmpMtf );
                    else if ( pGradient )
                        mpVDev->AddGradientActions( rPolyPoly.GetBoundRect(), *pGradient, aTmpMtf );
                    ImplWriteActions( aTmpMtf, nWriteFlags, nullptr );
                }
            }
        }

        OUString aPatternStyle = "fill:url(#" + aPatternId + ")";

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aPatternStyle );
        ImplWritePolyPolygon( rPolyPoly, false );
    }
}


void SVGActionWriter::ImplWriteGradientEx( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient,
                                           sal_uInt32 nWriteFlags)
{
    if ( rGradient.GetStyle() == GradientStyle::Linear ||
         rGradient.GetStyle() == GradientStyle::Axial )
    {
        ImplWriteGradientLinear( rPolyPoly, rGradient );
    }
    else
    {
        ImplWritePattern( rPolyPoly, nullptr, &rGradient, nWriteFlags );
    }
}


void SVGActionWriter::ImplWriteGradientLinear( const tools::PolyPolygon& rPolyPoly,
                                               const Gradient& rGradient )
{
    if( rPolyPoly.Count() )
    {
        SvXMLElementExport aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, true );

        OUString aGradientId = "gradient" + OUString::number( mnCurGradientId++ );

        {
            SvXMLElementExport aElemDefs( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, true, true );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, aGradientId );
            {
                tools::Rectangle aTmpRect, aRect;
                Point aTmpCenter, aCenter;

                rGradient.GetBoundRect( rPolyPoly.GetBoundRect(), aTmpRect, aTmpCenter );
                ImplMap( aTmpRect, aRect );
                ImplMap( aTmpCenter, aCenter );
                const sal_uInt16 nAngle = rGradient.GetAngle() % 3600;

                tools::Polygon aPoly( 2 );
                // Setting x value of a gradient vector to rotation center to
                // place a gradient vector in a target polygon.
                // This would help editing it in SVG editors like inkscape.
                aPoly[ 0 ].setX( aCenter.X() );
                aPoly[ 1 ].setX( aCenter.X() );
                aPoly[ 0 ].setY( aRect.Top() );
                aPoly[ 1 ].setY( aRect.Bottom() );
                aPoly.Rotate( aCenter, nAngle );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX1, OUString::number( aPoly[ 0 ].X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY1, OUString::number( aPoly[ 0 ].Y() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX2, OUString::number( aPoly[ 1 ].X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY2, OUString::number( aPoly[ 1 ].Y() ) );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrGradientUnits,
                                       OUString( "userSpaceOnUse" ) );
            }

            {
                SvXMLElementExport aElemLinearGradient( mrExport, XML_NAMESPACE_NONE, aXMLElemLinearGradient, true, true );

                const Color aStartColor = ImplGetColorWithIntensity( rGradient.GetStartColor(), rGradient.GetStartIntensity() );
                const Color aEndColor = ImplGetColorWithIntensity( rGradient.GetEndColor(), rGradient.GetEndIntensity() );
                double fBorderOffset = rGradient.GetBorder() / 100.0;
                const sal_uInt16 nSteps = rGradient.GetSteps();
                if( rGradient.GetStyle() == GradientStyle::Linear )
                {
                    // Emulate non-smooth gradient
                    if( 0 < nSteps && nSteps < 100 )
                    {
                        double fOffsetStep = ( 1.0 - fBorderOffset ) / static_cast<double>(nSteps);
                        for( sal_uInt16 i = 0; i < nSteps; i++ ) {
                            Color aColor = ImplGetGradientColor( aStartColor, aEndColor, i / static_cast<double>(nSteps) );
                            ImplWriteGradientStop( aColor, fBorderOffset + ( i + 1 ) * fOffsetStep );
                            aColor = ImplGetGradientColor( aStartColor, aEndColor, ( i + 1 ) / static_cast<double>(nSteps) );
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
                        double fOffsetStep = ( 0.5 - fBorderOffset ) / static_cast<double>(nSteps);
                        // Upper half
                        for( sal_uInt16 i = 0; i < nSteps; i++ )
                        {
                            Color aColor = ImplGetGradientColor( aEndColor, aStartColor, i / static_cast<double>(nSteps) );
                            ImplWriteGradientStop( aColor, fBorderOffset + i * fOffsetStep );
                            aColor = ImplGetGradientColor( aEndColor, aStartColor, (i + 1 ) / static_cast<double>(nSteps) );
                            ImplWriteGradientStop( aColor, fBorderOffset + i * fOffsetStep );
                        }
                        // Lower half
                        for( sal_uInt16 i = 0; i < nSteps; i++ )
                        {
                            Color aColor = ImplGetGradientColor( aStartColor, aEndColor, i / static_cast<double>(nSteps) );
                            ImplWriteGradientStop( aColor, 0.5 + (i + 1) * fOffsetStep );
                            aColor = ImplGetGradientColor( aStartColor, aEndColor, (i + 1 ) / static_cast<double>(nSteps) );
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

        OUString aGradientStyle = "fill:url(#" + aGradientId + ")";

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aGradientStyle );
        ImplWritePolyPolygon( rPolyPoly, false );
    }
}


void SVGActionWriter::ImplWriteGradientStop( const Color& rColor, double fOffset )
{
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrOffset, OUString::number( fOffset ) );

    OUString aStyle, aColor;
    aStyle += "stop-color:";
    SVGAttributeWriter::ImplGetColorStr ( rColor, aColor );
    aStyle += aColor;

    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aStyle );
    {
        SvXMLElementExport aElemStartStop( mrExport, XML_NAMESPACE_NONE, aXMLElemStop, true, true );
    }
}


Color SVGActionWriter::ImplGetColorWithIntensity( const Color& rColor,
                                                  sal_uInt16 nIntensity )
{
     sal_uInt8 nNewRed = static_cast<sal_uInt8>( static_cast<long>(rColor.GetRed()) * nIntensity / 100L );
     sal_uInt8 nNewGreen = static_cast<sal_uInt8>( static_cast<long>(rColor.GetGreen()) * nIntensity / 100L );
     sal_uInt8 nNewBlue = static_cast<sal_uInt8>( static_cast<long>(rColor.GetBlue()) * nIntensity / 100L );
     return Color( nNewRed, nNewGreen, nNewBlue);
}


Color SVGActionWriter::ImplGetGradientColor( const Color& rStartColor,
                                             const Color& rEndColor,
                                             double fOffset )
{
    long nRedStep = rEndColor.GetRed() - rStartColor.GetRed();
    long nNewRed = rStartColor.GetRed() + static_cast<long>( nRedStep * fOffset );
    nNewRed = ( nNewRed < 0 ) ? 0 : ( nNewRed > 0xFF) ? 0xFF : nNewRed;

    long nGreenStep = rEndColor.GetGreen() - rStartColor.GetGreen();
    long nNewGreen = rStartColor.GetGreen() + static_cast<long>( nGreenStep * fOffset );
    nNewGreen = ( nNewGreen < 0 ) ? 0 : ( nNewGreen > 0xFF) ? 0xFF : nNewGreen;

    long nBlueStep = rEndColor.GetBlue() - rStartColor.GetBlue();
    long nNewBlue = rStartColor.GetBlue() + static_cast<long>( nBlueStep * fOffset );
    nNewBlue = ( nNewBlue < 0 ) ? 0 : ( nNewBlue > 0xFF) ? 0xFF : nNewBlue;

    return Color( static_cast<sal_uInt8>(nNewRed), static_cast<sal_uInt8>(nNewGreen), static_cast<sal_uInt8>(nNewBlue) );
}


void SVGActionWriter::ImplWriteMask( GDIMetaFile& rMtf,
                                     const Point& rDestPt,
                                     const Size& rDestSize,
                                     const Gradient& rGradient,
                                     sal_uInt32 nWriteFlags )
{
    Point          aSrcPt( rMtf.GetPrefMapMode().GetOrigin() );
    const Size     aSrcSize( rMtf.GetPrefSize() );
    const double   fScaleX = aSrcSize.Width() ? static_cast<double>(rDestSize.Width()) / aSrcSize.Width() : 1.0;
    const double   fScaleY = aSrcSize.Height() ? static_cast<double>(rDestSize.Height()) / aSrcSize.Height() : 1.0;
    long           nMoveX, nMoveY;

    if( fScaleX != 1.0 || fScaleY != 1.0 )
    {
        rMtf.Scale( fScaleX, fScaleY );
        aSrcPt.setX( FRound( aSrcPt.X() * fScaleX ) );
        aSrcPt.setY( FRound( aSrcPt.Y() * fScaleY ) );
    }

    nMoveX = rDestPt.X() - aSrcPt.X();
    nMoveY = rDestPt.Y() - aSrcPt.Y();

    if( nMoveX || nMoveY )
        rMtf.Move( nMoveX, nMoveY );

    OUString aMaskId = "mask" + OUString::number( mnCurMaskId++ );

    {
        SvXMLElementExport aElemDefs( mrExport, XML_NAMESPACE_NONE, aXMLElemDefs, true, true );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrId, aMaskId );
        {
            SvXMLElementExport aElemMask( mrExport, XML_NAMESPACE_NONE, "mask", true, true );

            const tools::PolyPolygon aPolyPolygon( tools::PolyPolygon( tools::Rectangle( rDestPt, rDestSize ) ) );
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

    OUString aMaskStyle = "mask:url(#" + aMaskId + ")";
    mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrStyle, aMaskStyle );

    {
        SvXMLElementExport aElemG( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, true );

        mpVDev->Push();
        ImplWriteActions( rMtf, nWriteFlags, nullptr );
        mpVDev->Pop();
    }
}


void SVGActionWriter::ImplWriteText( const Point& rPos, const OUString& rText,
                                     const long* pDXArray, long nWidth )
{
    const FontMetric aMetric( mpVDev->GetFontMetric() );

    bool bTextSpecial = aMetric.IsShadow() || aMetric.IsOutline() || (aMetric.GetRelief() != FontRelief::NONE);

    if( !bTextSpecial )
    {
        ImplWriteText( rPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
    }
    else
    {
        if( aMetric.GetRelief() != FontRelief::NONE )
        {
            Color aReliefColor( COL_LIGHTGRAY );
            Color aTextColor( mpVDev->GetTextColor() );

            if ( aTextColor == COL_BLACK )
                aTextColor = COL_WHITE;

            if ( aTextColor == COL_WHITE )
                aReliefColor = COL_BLACK;


            Point aPos( rPos );
            Point aOffset( 6, 6 );

            if ( aMetric.GetRelief() == FontRelief::Engraved )
            {
                aPos -= aOffset;
            }
            else
            {
                aPos += aOffset;
            }

            ImplWriteText( aPos, rText, pDXArray, nWidth, aReliefColor );
            ImplWriteText( rPos, rText, pDXArray, nWidth, aTextColor );
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

                if ( (aTextColor == COL_BLACK) || (aTextColor.GetLuminance() < 8) )
                    aShadowColor = COL_LIGHTGRAY;

                Point aPos( rPos );
                aPos += Point( nOff, nOff );
                ImplWriteText( aPos, rText, pDXArray, nWidth, aShadowColor );

                if( !aMetric.IsOutline() )
                {
                    ImplWriteText( rPos, rText, pDXArray, nWidth, aTextColor );
                }
            }

            if( aMetric.IsOutline() )
            {
                Point aPos = rPos + Point( -6, -6 );
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point( +6, +6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point( -6, +0);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point( -6, +6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point( +0, +6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point( +0, -6);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point( +6, -1);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );
                aPos = rPos + Point( +6, +0);
                ImplWriteText( aPos, rText, pDXArray, nWidth, mpVDev->GetTextColor() );

                ImplWriteText( rPos, rText, pDXArray, nWidth, COL_WHITE );
            }
        }
    }
}


void SVGActionWriter::ImplWriteText( const Point& rPos, const OUString& rText,
                                     const long* pDXArray, long nWidth,
                                     Color aTextColor )
{
    sal_Int32                               nLen = rText.getLength();
    Size                                    aNormSize;
    Point                                   aPos;
    Point                                   aBaseLinePos( rPos );
    const FontMetric                        aMetric( mpVDev->GetFontMetric() );
    const vcl::Font&                        rFont = mpVDev->GetFont();

    if( rFont.GetAlignment() == ALIGN_TOP )
        aBaseLinePos.AdjustY(aMetric.GetAscent() );
    else if( rFont.GetAlignment() == ALIGN_BOTTOM )
        aBaseLinePos.AdjustY( -(aMetric.GetDescent()) );

    ImplMap( rPos, aPos );

    std::unique_ptr<long[]> xTmpArray(new long[nLen]);
    // get text sizes
    if( pDXArray )
    {
        aNormSize = Size( mpVDev->GetTextWidth( rText ), 0 );
        memcpy(xTmpArray.get(), pDXArray, nLen * sizeof(long));
    }
    else
    {
        aNormSize = Size( mpVDev->GetTextArray( rText, xTmpArray.get() ), 0 );
    }
    long* pDX = xTmpArray.get();

    // if text is rotated, set transform matrix at new g element
    if( rFont.GetOrientation() )
    {
        Point   aRot( aPos );
        OUString  aTransform = "rotate(" +
                    OUString::number( rFont.GetOrientation() * -0.1 ) + " " +
                    OUString::number( aRot.X() ) + " " +
                    OUString::number( aRot.Y() ) + ")";
        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrTransform, aTransform );
    }


    maAttributeWriter.AddPaintAttr( COL_TRANSPARENT, aTextColor );

    // for each line of text there should be at least one group element
    SvXMLElementExport aSVGGElem( mrExport, XML_NAMESPACE_NONE, aXMLElemG, true, false );

    bool bIsPlaceholderField = false;

    if( mbIsPlaceholderShape )
    {
        bIsPlaceholderField = rText.match( sPlaceholderTag );
        // for a placeholder text field we export only one <text> svg element
        if( bIsPlaceholderField )
        {
            OUString sCleanTextContent;
            static const sal_Int32 nFrom = sPlaceholderTag.getLength();
            if( rText.getLength() > nFrom )
            {
                sCleanTextContent = rText.copy( nFrom );
            }
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "class", "PlaceholderText" );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, OUString::number( aPos.X() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, OUString::number( aPos.Y() ) );
            {
                SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemText, true, false );
                // At least for the single slide case we need really to  export placeholder text
                mrExport.GetDocHandler()->characters( sCleanTextContent );
            }
        }
    }

    if( !bIsPlaceholderField )
    {
        if( nLen > 1 )
        {
            aNormSize.setWidth( pDX[ nLen - 2 ] + mpVDev->GetTextWidth( OUString(rText[nLen - 1]) ) );

            if( nWidth && aNormSize.Width() && ( nWidth != aNormSize.Width() ) )
            {
                long i;
                const double fFactor = static_cast<double>(nWidth) / aNormSize.Width();

                for( i = 0; i < ( nLen - 1 ); i++ )
                    pDX[ i ] = FRound( pDX[ i ] * fFactor );
            }
            else
            {
                css::uno::Reference< css::i18n::XBreakIterator > xBI( vcl::unohelper::CreateBreakIterator() );
                const css::lang::Locale& rLocale = Application::GetSettings().GetLanguageTag().getLocale();
                sal_Int32 nCurPos = 0, nLastPos = 0, nX = aPos.X();

                // write single glyphs at absolute text positions
                for( bool bCont = true; bCont; )
                {
                    sal_Int32 nCount = 1;

                    nLastPos = nCurPos;
                    nCurPos = xBI->nextCharacters( rText, nCurPos, rLocale,
                                                css::i18n::CharacterIteratorMode::SKIPCELL,
                                                nCount, nCount );

                    nCount = nCurPos - nLastPos;
                    bCont = ( nCurPos < rText.getLength() ) && nCount;

                    if( nCount )
                    {
                        const OUString aGlyph( rText.copy( nLastPos, nCount ) );

                        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, OUString::number( nX ) );
                        mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, OUString::number( aPos.Y() ) );

                        {
                            SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemText, true, false );
                            mrExport.GetDocHandler()->characters( aGlyph );
                        }

                        if( bCont )
                        {
                            // #118796# do NOT access pDXArray, it may be zero (!)
                            sal_Int32 nDXWidth = pDX[ nCurPos - 1 ];
                            nDXWidth = ImplMap( nDXWidth );
                            nX = aPos.X() + nDXWidth;
                        }
                    }
                }
            }
        }
        else
        {
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, OUString::number( aPos.X() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, OUString::number( aPos.Y() ) );

            {
                SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, aXMLElemText, true, false );
                mrExport.GetDocHandler()->characters( rText );
            }
        }
    }


    if( !mrExport.IsUseNativeTextDecoration() )
    {
        if( rFont.GetStrikeout() != STRIKEOUT_NONE || rFont.GetUnderline() != LINESTYLE_NONE )
        {
            tools::Polygon aPoly( 4 );
            const long  nLineHeight = std::max<long>( FRound( aMetric.GetLineHeight() * 0.05 ), 1 );

            if( rFont.GetStrikeout() )
            {
                const long nYLinePos = aBaseLinePos.Y() - FRound( aMetric.GetAscent() * 0.26 );

                aPoly[ 0 ].setX( aBaseLinePos.X() ); aPoly[ 0 ].setY( nYLinePos - ( nLineHeight >> 1 ) );
                aPoly[ 1 ].setX( aBaseLinePos.X() + aNormSize.Width() - 1 ); aPoly[ 1 ].setY( aPoly[ 0 ].Y() );
                aPoly[ 2 ].setX( aPoly[ 1 ].X() ); aPoly[ 2 ].setY( aPoly[ 0 ].Y() + nLineHeight - 1 );
                aPoly[ 3 ].setX( aPoly[ 0 ].X() ); aPoly[ 3 ].setY( aPoly[ 2 ].Y() );

                ImplWritePolyPolygon( aPoly, false );
            }

            if( rFont.GetUnderline() )
            {
                const long  nYLinePos = aBaseLinePos.Y() + ( nLineHeight << 1 );

                aPoly[ 0 ].setX( aBaseLinePos.X() ); aPoly[ 0 ].setY( nYLinePos - ( nLineHeight >> 1 ) );
                aPoly[ 1 ].setX( aBaseLinePos.X() + aNormSize.Width() - 1 ); aPoly[ 1 ].setY( aPoly[ 0 ].Y() );
                aPoly[ 2 ].setX( aPoly[ 1 ].X() ); aPoly[ 2 ].setY( aPoly[ 0 ].Y() + nLineHeight - 1 );
                aPoly[ 3 ].setX( aPoly[ 0 ].X() ); aPoly[ 3 ].setY( aPoly[ 2 ].Y() );

                ImplWritePolyPolygon( aPoly, false );
            }
        }
    }
}


void SVGActionWriter::ImplWriteBmp( const BitmapEx& rBmpEx,
                                    const Point& rPt, const Size& rSz,
                                    const Point& rSrcPt, const Size& rSrcSz )
{
    if( !!rBmpEx )
    {
        BitmapEx aBmpEx( rBmpEx );
        const tools::Rectangle aBmpRect( Point(), rBmpEx.GetSizePixel() );
        const tools::Rectangle aSrcRect( rSrcPt, rSrcSz );

        if( aSrcRect != aBmpRect )
            aBmpEx.Crop( aSrcRect );

        if( !!aBmpEx )
        {
            SvMemoryStream aOStm( 65535, 65535 );

            if( GraphicConverter::Export( aOStm, rBmpEx, ConvertDataFormat::PNG ) == ERRCODE_NONE )
            {
                Point                    aPt;
                Size                     aSz;
                Sequence< sal_Int8 >     aSeq( static_cast<sal_Int8 const *>(aOStm.GetData()), aOStm.Tell() );
                OUStringBuffer aBuffer( "data:image/png;base64," );
                ::comphelper::Base64::encode( aBuffer, aSeq );

                ImplMap( rPt, aPt );
                ImplMap( rSz, aSz );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, OUString::number( aPt.X() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, OUString::number( aPt.Y() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, OUString::number( aSz.Width() ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, OUString::number( aSz.Height() ) );

                // the image must be scaled to aSz in a non-uniform way
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "preserveAspectRatio", "none" );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, aXMLAttrXLinkHRef, aBuffer.makeStringAndClear() );
                {
                    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, "image", true, true );
                }
            }
        }
    }
}


void SVGActionWriter::ImplWriteActions( const GDIMetaFile& rMtf,
                                        sal_uInt32 nWriteFlags,
                                        const OUString* pElementId,
                                        const Reference< css::drawing::XShape >* pxShape,
                                        const GDIMetaFile* pTextEmbeddedBitmapMtf )
{
    // need a counter for the actions written per shape to avoid double ID
    // generation
    sal_Int32 nEntryCount(0);

#if OSL_DEBUG_LEVEL > 0
    bool bIsTextShape = false;
    if( !mrExport.IsUsePositionedCharacters() && pxShape
            && Reference< XText >( *pxShape, UNO_QUERY ).is() )
    {
        bIsTextShape = true;
    }
#endif
    mbIsPlaceholderShape = false;
    if( ( pElementId != nullptr ) && ( *pElementId == sPlaceholderTag ) )
    {
        mbIsPlaceholderShape = true;
        // since we utilize pElementId in an improper way we reset it to NULL before to go on
        pElementId = nullptr;
    }

    for( sal_uLong nCurAction = 0, nCount = rMtf.GetActionSize(); nCurAction < nCount; nCurAction++ )
    {
        const MetaAction*    pAction = rMtf.GetAction( nCurAction );
        const MetaActionType nType = pAction->GetType();

#if OSL_DEBUG_LEVEL > 0
        if( bIsTextShape )
        {
            try
            {
                SvXMLElementExport aElem( mrExport,
                        XML_NAMESPACE_NONE, "desc", false, false );
                OUStringBuffer sType(OUString::number(static_cast<sal_uInt16>(nType)));
                if (pAction && (nType == MetaActionType::COMMENT))
                {
                    sType.append(": ");
                    const MetaCommentAction* pA = static_cast<const MetaCommentAction*>(pAction);
                    OString sComment = pA->GetComment();
                    if (!sComment.isEmpty())
                    {
                        sType.append(OStringToOUString(
                                        sComment, RTL_TEXTENCODING_UTF8));
                    }
                    if (sComment.equalsIgnoreAsciiCase("FIELD_SEQ_BEGIN"))
                    {
                        sal_uInt8 const*const pData = pA->GetData();
                        if (pData && (pA->GetDataSize()))
                        {
                            sal_uInt16 sz = static_cast<sal_uInt16>((pA->GetDataSize()) / 2);
                            if (sz)
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
                    mrExport.GetDocHandler()->characters(
                            sType.makeStringAndClear());
                }
            }
            catch( ... )
            {
                const MetaCommentAction* pA = static_cast<const MetaCommentAction*>(pAction);
                SAL_WARN( "filter.svg", pA->GetComment() );
            }

        }
#endif
        switch( nType )
        {
            case MetaActionType::PIXEL:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaPixelAction* pA = static_cast<const MetaPixelAction*>(pAction);

                    maAttributeWriter.AddPaintAttr( pA->GetColor(), pA->GetColor() );
                    ImplWriteLine( pA->GetPoint(), pA->GetPoint(), &pA->GetColor() );
                }
            }
            break;

            case MetaActionType::POINT:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaPointAction* pA = static_cast<const MetaPointAction*>(pAction);

                    maAttributeWriter.AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetLineColor() );
                    ImplWriteLine( pA->GetPoint(), pA->GetPoint() );
                }
            }
            break;

            case MetaActionType::LINE:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaLineAction* pA = static_cast<const MetaLineAction*>(pAction);

                    maAttributeWriter.AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetLineColor() );
                    ImplWriteLine( pA->GetStartPoint(), pA->GetEndPoint() );
                }
            }
            break;

            case MetaActionType::RECT:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    maAttributeWriter.AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteRect( static_cast<const MetaRectAction*>(pAction)->GetRect() );
                }
            }
            break;

            case MetaActionType::ROUNDRECT:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaRoundRectAction* pA = static_cast<const MetaRoundRectAction*>(pAction);

                    maAttributeWriter.AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }
            }
            break;

            case MetaActionType::ELLIPSE:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaEllipseAction*    pA = static_cast<const MetaEllipseAction*>(pAction);
                    const tools::Rectangle&            rRect = pA->GetRect();

                    maAttributeWriter.AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                    ImplWriteEllipse( rRect.Center(), rRect.GetWidth() >> 1, rRect.GetHeight() >> 1 );
                }
            }
            break;

            case MetaActionType::ARC:
            case MetaActionType::PIE:
            case MetaActionType::CHORD:
            case MetaActionType::POLYGON:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    tools::Polygon aPoly;

                    switch( nType )
                    {
                        case MetaActionType::ARC:
                        {
                            const MetaArcAction* pA = static_cast<const MetaArcAction*>(pAction);
                            aPoly = tools::Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), PolyStyle::Arc );
                        }
                        break;

                        case MetaActionType::PIE:
                        {
                            const MetaPieAction* pA = static_cast<const MetaPieAction*>(pAction);
                            aPoly = tools::Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), PolyStyle::Pie );
                        }
                        break;

                        case MetaActionType::CHORD:
                        {
                            const MetaChordAction* pA = static_cast<const MetaChordAction*>(pAction);
                            aPoly = tools::Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), PolyStyle::Chord );
                        }
                        break;

                        case MetaActionType::POLYGON:
                            aPoly = static_cast<const MetaPolygonAction*>(pAction)->GetPolygon();
                        break;
                        default: break;
                    }

                    if( aPoly.GetSize() )
                    {
                        maAttributeWriter.AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                        ImplWritePolyPolygon( aPoly, false );
                    }
                }
            }
            break;

            case MetaActionType::POLYLINE:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaPolyLineAction* pA = static_cast<const MetaPolyLineAction*>(pAction);
                    const tools::Polygon& rPoly = pA->GetPolygon();

                    if( rPoly.GetSize() )
                    {
                        maAttributeWriter.AddPaintAttr( mpVDev->GetLineColor(), COL_TRANSPARENT );
                        ImplAddLineAttr( pA->GetLineInfo() );
                        ImplWritePolyPolygon( rPoly, true );
                    }
                }
            }
            break;

            case MetaActionType::POLYPOLYGON:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaPolyPolygonAction*    pA = static_cast<const MetaPolyPolygonAction*>(pAction);
                    const tools::PolyPolygon&              rPolyPoly = pA->GetPolyPolygon();

                    if( rPolyPoly.Count() )
                    {
                        maAttributeWriter.AddPaintAttr( mpVDev->GetLineColor(), mpVDev->GetFillColor() );
                        ImplWritePolyPolygon( rPolyPoly, false );
                    }
                }
            }
            break;

            case MetaActionType::GRADIENT:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaGradientAction* pA = static_cast<const MetaGradientAction*>(pAction);
                    const tools::Polygon aRectPoly( pA->GetRect() );
                    const tools::PolyPolygon aRectPolyPoly( aRectPoly );

                    ImplWriteGradientEx( aRectPolyPoly, pA->GetGradient(), nWriteFlags );
                }
            }
            break;

            case MetaActionType::GRADIENTEX:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaGradientExAction* pA = static_cast<const MetaGradientExAction*>(pAction);
                    ImplWriteGradientEx( pA->GetPolyPolygon(), pA->GetGradient(), nWriteFlags );
                }
            }
            break;

            case MetaActionType::HATCH:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaHatchAction*  pA = static_cast<const MetaHatchAction*>(pAction);
                    ImplWritePattern( pA->GetPolyPolygon(), &pA->GetHatch(), nullptr, nWriteFlags );
                }
            }
            break;

            case MetaActionType::Transparent:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaTransparentAction*    pA = static_cast<const MetaTransparentAction*>(pAction);
                    const tools::PolyPolygon& rPolyPoly = pA->GetPolyPolygon();

                    if( rPolyPoly.Count() )
                    {
                        Color aNewLineColor( mpVDev->GetLineColor() ), aNewFillColor( mpVDev->GetFillColor() );

                        aNewLineColor.SetTransparency( sal::static_int_cast<sal_uInt8>( FRound( pA->GetTransparence() * 2.55 ) ) );
                        aNewFillColor.SetTransparency( sal::static_int_cast<sal_uInt8>( FRound( pA->GetTransparence() * 2.55 ) ) );

                        maAttributeWriter.AddPaintAttr( aNewLineColor, aNewFillColor );
                        ImplWritePolyPolygon( rPolyPoly, false );
                    }
                }
            }
            break;

            case MetaActionType::FLOATTRANSPARENT:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaFloatTransparentAction*   pA = static_cast<const MetaFloatTransparentAction*>(pAction);
                    GDIMetaFile                         aTmpMtf( pA->GetGDIMetaFile() );
                    ImplWriteMask( aTmpMtf, pA->GetPoint(), pA->GetSize(),
                                   pA->GetGradient(), nWriteFlags  );
                }
            }
            break;

            case MetaActionType::EPS:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaEPSAction*    pA = static_cast<const MetaEPSAction*>(pAction);
                    const GDIMetaFile&      aGDIMetaFile( pA->GetSubstitute() );
                    bool                bFound = false;

                    for( sal_uInt32 k = 0, nCount2 = aGDIMetaFile.GetActionSize(); ( k < nCount2 ) && !bFound; ++k )
                    {
                        const MetaAction* pSubstAct = aGDIMetaFile.GetAction( k );

                        if( pSubstAct->GetType() == MetaActionType::BMPSCALE )
                        {
                            bFound = true;
                            const MetaBmpScaleAction* pBmpScaleAction = static_cast<const MetaBmpScaleAction*>(pSubstAct);
                            ImplWriteBmp( BitmapEx(pBmpScaleAction->GetBitmap()),
                                          pA->GetPoint(), pA->GetSize(),
                                          Point(), pBmpScaleAction->GetBitmap().GetSizePixel() );
                        }
                    }
                }
            }
            break;

            case MetaActionType::COMMENT:
            {
                const MetaCommentAction*    pA = static_cast<const MetaCommentAction*>(pAction);

                if( ( pA->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_BEGIN") ) &&
                    ( nWriteFlags & SVGWRITER_WRITE_FILL ) )
                {
                    const MetaGradientExAction* pGradAction = nullptr;
                    bool                    bDone = false;

                    while( !bDone && ( ++nCurAction < nCount ) )
                    {
                        pAction = rMtf.GetAction( nCurAction );

                        if( pAction->GetType() == MetaActionType::GRADIENTEX )
                            pGradAction = static_cast<const MetaGradientExAction*>(pAction);
                        else if( ( pAction->GetType() == MetaActionType::COMMENT ) &&
                                 ( static_cast<const MetaCommentAction*>( pAction )->GetComment().
                                        equalsIgnoreAsciiCase("XGRAD_SEQ_END") ) )
                        {
                            bDone = true;
                        }
                    }

                    if( pGradAction )
                        ImplWriteGradientEx( pGradAction->GetPolyPolygon(), pGradAction->GetGradient(), nWriteFlags );
                }
                else if( ( pA->GetComment().equalsIgnoreAsciiCase("XPATHFILL_SEQ_BEGIN") ) &&
                         ( nWriteFlags & SVGWRITER_WRITE_FILL ) && !( nWriteFlags & SVGWRITER_NO_SHAPE_COMMENTS ) &&
                         pA->GetDataSize() )
                {
                    // write open shape in every case
                    if (mapCurShape)
                    {
                        ImplWriteShape( *mapCurShape );
                        mapCurShape.reset();
                    }

                    SvMemoryStream  aMemStm( const_cast<sal_uInt8 *>(pA->GetData()), pA->GetDataSize(), StreamMode::READ );
                    SvtGraphicFill  aFill;

                    ReadSvtGraphicFill( aMemStm, aFill );

                    bool bGradient = SvtGraphicFill::fillGradient == aFill.getFillType() &&
                                     ( SvtGraphicFill::GradientType::Linear == aFill.getGradientType() ||
                                       SvtGraphicFill::GradientType::Radial == aFill.getGradientType() );
                    bool bSkip = ( SvtGraphicFill::fillSolid == aFill.getFillType() || bGradient );

                    if( bSkip )
                    {
                        tools::PolyPolygon aShapePolyPoly;

                        aFill.getPath( aShapePolyPoly );

                        if( aShapePolyPoly.Count() )
                        {
                            mapCurShape.reset( new SVGShapeDescriptor );

                            if( pElementId )
                            {
                                mapCurShape->maId = *pElementId + "_" + OUString::number(nEntryCount++);
                            }

                            mapCurShape->maShapePolyPoly = aShapePolyPoly;
                            mapCurShape->maShapeFillColor = aFill.getFillColor();
                            mapCurShape->maShapeFillColor.SetTransparency( static_cast<sal_uInt8>(FRound( 255.0 * aFill.getTransparency() )) );

                            if( bGradient )
                            {
                                // step through following actions until the first Gradient/GradientEx action is found
                                while (!mapCurShape->mapShapeGradient && bSkip
                                       && (++nCurAction < nCount))
                                {
                                    pAction = rMtf.GetAction( nCurAction );

                                    if( ( pAction->GetType() == MetaActionType::COMMENT ) &&
                                        ( static_cast<const MetaCommentAction*>(pAction)->GetComment().
                                               equalsIgnoreAsciiCase("XPATHFILL_SEQ_END") ) )
                                    {
                                        bSkip = false;
                                    }
                                    else if( pAction->GetType() == MetaActionType::GRADIENTEX )
                                    {
                                        mapCurShape->mapShapeGradient.reset( new Gradient(
                                            static_cast< const MetaGradientExAction* >( pAction )->GetGradient() ) );
                                    }
                                    else if( pAction->GetType() == MetaActionType::GRADIENT )
                                    {
                                        mapCurShape->mapShapeGradient.reset( new Gradient(
                                            static_cast< const MetaGradientAction* >( pAction )->GetGradient() ) );
                                    }
                                }
                            }
                        }
                        else
                            bSkip = false;
                    }

                    // skip rest of comment
                    while( bSkip && ( ++nCurAction < nCount ) )
                    {
                        pAction = rMtf.GetAction( nCurAction );

                        if( ( pAction->GetType() == MetaActionType::COMMENT ) &&
                                    ( static_cast<const MetaCommentAction*>( pAction )->GetComment().
                                            equalsIgnoreAsciiCase("XPATHFILL_SEQ_END") ) )
                        {
                            bSkip = false;
                        }
                    }
                }
                else if( ( pA->GetComment().equalsIgnoreAsciiCase("XPATHSTROKE_SEQ_BEGIN") ) &&
                         ( nWriteFlags & SVGWRITER_WRITE_FILL ) && !( nWriteFlags & SVGWRITER_NO_SHAPE_COMMENTS ) &&
                         pA->GetDataSize() )
                {
                    SvMemoryStream aMemStm( const_cast<sal_uInt8 *>(pA->GetData()), pA->GetDataSize(), StreamMode::READ );
                    SvtGraphicStroke aStroke;
                    tools::PolyPolygon aStartArrow, aEndArrow;

                    ReadSvtGraphicStroke( aMemStm, aStroke );
                    aStroke.getStartArrow( aStartArrow );
                    aStroke.getEndArrow( aEndArrow );

                    // Currently no support for strokes with start/end arrow(s)
                    // added that support
                    tools::Polygon aPoly;

                    aStroke.getPath(aPoly);

                    if (mapCurShape)
                    {
                        if(1 != mapCurShape->maShapePolyPoly.Count()
                            || !mapCurShape->maShapePolyPoly[0].IsEqual(aPoly))
                        {
                            // this path action is not covering the same path than the already existing
                            // fill polypolygon, so write out the fill polygon
                            ImplWriteShape( *mapCurShape );
                            mapCurShape.reset();
                        }
                    }

                    if (!mapCurShape)
                    {

                        mapCurShape.reset( new SVGShapeDescriptor );

                        if( pElementId )
                        {
                            mapCurShape->maId = *pElementId + "_" + OUString::number(nEntryCount++);
                        }

                        mapCurShape->maShapePolyPoly = aPoly;
                    }

                    mapCurShape->maShapeLineColor = mpVDev->GetLineColor();
                    mapCurShape->maShapeLineColor.SetTransparency( static_cast<sal_uInt8>(FRound( aStroke.getTransparency() * 255.0 )) );
                    mapCurShape->mnStrokeWidth = FRound( aStroke.getStrokeWidth() );
                    aStroke.getDashArray( mapCurShape->maDashArray );

                    // added support for LineJoin
                    switch(aStroke.getJoinType())
                    {
                        default: /* SvtGraphicStroke::joinMiter,  SvtGraphicStroke::joinNone */
                        {
                            mapCurShape->maLineJoin = basegfx::B2DLineJoin::Miter;
                            break;
                        }
                        case SvtGraphicStroke::joinRound:
                        {
                            mapCurShape->maLineJoin = basegfx::B2DLineJoin::Round;
                            break;
                        }
                        case SvtGraphicStroke::joinBevel:
                        {
                            mapCurShape->maLineJoin = basegfx::B2DLineJoin::Bevel;
                            break;
                        }
                    }

                    // added support for LineCap
                    switch(aStroke.getCapType())
                    {
                        default: /* SvtGraphicStroke::capButt */
                        {
                            mapCurShape->maLineCap = css::drawing::LineCap_BUTT;
                            break;
                        }
                        case SvtGraphicStroke::capRound:
                        {
                            mapCurShape->maLineCap = css::drawing::LineCap_ROUND;
                            break;
                        }
                        case SvtGraphicStroke::capSquare:
                        {
                            mapCurShape->maLineCap = css::drawing::LineCap_SQUARE;
                            break;
                        }
                    }

                    if(mapCurShape.get() && mapCurShape->maShapePolyPoly.Count() && (aStartArrow.Count() || aEndArrow.Count()))
                    {
                        ImplWriteShape( *mapCurShape );

                        mapCurShape->maShapeFillColor = mapCurShape->maShapeLineColor;
                        mapCurShape->maShapeLineColor = COL_TRANSPARENT;
                        mapCurShape->mnStrokeWidth = 0;
                        mapCurShape->maDashArray.clear();
                        mapCurShape->maLineJoin = basegfx::B2DLineJoin::Miter;
                        mapCurShape->maLineCap = css::drawing::LineCap_BUTT;

                        if(aStartArrow.Count())
                        {
                            mapCurShape->maShapePolyPoly = aStartArrow;

                            if( pElementId ) // #i124825# pElementId is optional, may be zero
                            {
                                mapCurShape->maId = *pElementId + "_" + OUString::number(nEntryCount++);
                            }

                            ImplWriteShape( *mapCurShape );
                        }

                        if(aEndArrow.Count())
                        {
                            mapCurShape->maShapePolyPoly = aEndArrow;

                            if( pElementId ) // #i124825# pElementId is optional, may be zero
                            {
                                mapCurShape->maId = *pElementId + "_" + OUString::number(nEntryCount++);
                            }

                            ImplWriteShape( *mapCurShape );
                        }

                        mapCurShape.reset();
                    }

                    // write open shape in every case
                    if (mapCurShape)
                    {
                        ImplWriteShape( *mapCurShape );
                        mapCurShape.reset();
                    }

                    // skip rest of comment
                    bool bSkip = true;

                    while( bSkip && ( ++nCurAction < nCount ) )
                    {
                        pAction = rMtf.GetAction( nCurAction );

                        if( ( pAction->GetType() == MetaActionType::COMMENT ) &&
                                    ( static_cast<const MetaCommentAction*>(pAction)->GetComment().
                                    equalsIgnoreAsciiCase("XPATHSTROKE_SEQ_END") ) )
                        {
                            bSkip = false;
                        }
                    }
                }
                else if( !mrExport.IsUsePositionedCharacters() && ( nWriteFlags & SVGWRITER_WRITE_TEXT ) )
                {
                    if( pA->GetComment().equalsIgnoreAsciiCase( "XTEXT_PAINTSHAPE_BEGIN" ) )
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
                    else if( pA->GetComment().equalsIgnoreAsciiCase( "XTEXT_PAINTSHAPE_END" ) )
                    {
                        maTextWriter.endTextShape();
                    }
                    else if( pA->GetComment().equalsIgnoreAsciiCase( "XTEXT_EOP" ) )
                    {
                        const MetaAction* pNextAction = rMtf.GetAction( nCurAction + 1 );
                        if( !( ( pNextAction->GetType() == MetaActionType::COMMENT ) &&
                               ( static_cast<const MetaCommentAction*>(pNextAction)->GetComment().equalsIgnoreAsciiCase("XTEXT_PAINTSHAPE_END") )  ))
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
                    else if( pA->GetComment().equalsIgnoreAsciiCase( "XTEXT_EOL" ) )
                    {
                        const MetaAction* pNextAction = rMtf.GetAction( nCurAction + 1 );
                        if( !( ( pNextAction->GetType() == MetaActionType::COMMENT ) &&
                               ( static_cast<const MetaCommentAction*>(pNextAction)->GetComment().equalsIgnoreAsciiCase("XTEXT_EOP") ) ) )
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

            case MetaActionType::BMP:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpAction* pA = static_cast<const MetaBmpAction*>(pAction);

                    ImplWriteBmp( BitmapEx(pA->GetBitmap()),
                                  pA->GetPoint(), mpVDev->PixelToLogic( pA->GetBitmap().GetSizePixel() ),
                                  Point(), pA->GetBitmap().GetSizePixel() );
                }
            }
            break;

            case MetaActionType::BMPSCALE:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pAction);

                    // Bitmaps embedded into text shapes are collected and exported elsewhere.
                    if( maTextWriter.isTextShapeStarted() )
                    {
                        maTextWriter.writeBitmapPlaceholder( pA );
                    }
                    else
                    {
                        ImplWriteBmp( BitmapEx(pA->GetBitmap()),
                                      pA->GetPoint(), pA->GetSize(),
                                      Point(), pA->GetBitmap().GetSizePixel() );
                    }
                }
            }
            break;

            case MetaActionType::BMPSCALEPART:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpScalePartAction* pA = static_cast<const MetaBmpScalePartAction*>(pAction);

                    ImplWriteBmp( BitmapEx(pA->GetBitmap()),
                                  pA->GetDestPoint(), pA->GetDestSize(),
                                  pA->GetSrcPoint(), pA->GetSrcSize() );
                }
            }
            break;

            case MetaActionType::BMPEX:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpExAction*  pA = static_cast<const MetaBmpExAction*>(pAction);

                    ImplWriteBmp( pA->GetBitmapEx(),
                                  pA->GetPoint(), mpVDev->PixelToLogic( pA->GetBitmapEx().GetSizePixel() ),
                                  Point(), pA->GetBitmapEx().GetSizePixel() );
                }
            }
            break;

            case MetaActionType::BMPEXSCALE:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pAction);

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

            case MetaActionType::BMPEXSCALEPART:
            {
                if( nWriteFlags & SVGWRITER_WRITE_FILL )
                {
                    const MetaBmpExScalePartAction* pA = static_cast<const MetaBmpExScalePartAction*>(pAction);

                    ImplWriteBmp( pA->GetBitmapEx(),
                                  pA->GetDestPoint(), pA->GetDestSize(),
                                  pA->GetSrcPoint(), pA->GetSrcSize() );
                }
            }
            break;

            case MetaActionType::TEXT:
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaTextAction*   pA = static_cast<const MetaTextAction*>(pAction);
                    sal_Int32               aLength = std::min( pA->GetText().getLength(), pA->GetLen() );
                    const OUString          aText = pA->GetText().copy( pA->GetIndex(), aLength );

                    if( !aText.isEmpty() )
                    {
                        if( mrExport.IsUsePositionedCharacters() )
                        {
                            vcl::Font aFont = ImplSetCorrectFontHeight();
                            maAttributeWriter.SetFontAttr( aFont );
                            ImplWriteText( pA->GetPoint(), aText, nullptr, 0 );
                        }
                        else
                        {
                            maTextWriter.writeTextPortion( pA->GetPoint(), aText );
                        }
                    }
                }
            }
            break;

            case MetaActionType::TEXTRECT:
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaTextRectAction* pA = static_cast<const MetaTextRectAction*>(pAction);

                    if (!pA->GetText().isEmpty())
                    {
                        if( mrExport.IsUsePositionedCharacters() )
                        {
                            vcl::Font aFont = ImplSetCorrectFontHeight();
                            maAttributeWriter.SetFontAttr( aFont );
                            ImplWriteText( pA->GetRect().TopLeft(), pA->GetText(), nullptr, 0 );
                        }
                        maTextWriter.writeTextPortion( pA->GetRect().TopLeft(), pA->GetText() );
                    }
                }
            }
            break;

            case MetaActionType::TEXTARRAY:
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaTextArrayAction*  pA = static_cast<const MetaTextArrayAction*>(pAction);
                    sal_Int32                   aLength = std::min( pA->GetText().getLength(), pA->GetLen() );
                    const OUString              aText = pA->GetText().copy( pA->GetIndex(), aLength );

                    if( !aText.isEmpty() )
                    {
                        if( mrExport.IsUsePositionedCharacters() )
                        {
                            vcl::Font aFont = ImplSetCorrectFontHeight();
                            maAttributeWriter.SetFontAttr( aFont );
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

            case MetaActionType::STRETCHTEXT:
            {
                if( nWriteFlags & SVGWRITER_WRITE_TEXT )
                {
                    const MetaStretchTextAction*    pA = static_cast<const MetaStretchTextAction*>(pAction);
                    sal_Int32                       aLength = std::min( pA->GetText().getLength(), pA->GetLen() );
                    const OUString                  aText = pA->GetText().copy( pA->GetIndex(), aLength );

                    if( !aText.isEmpty() )
                    {
                        if( mrExport.IsUsePositionedCharacters() )
                        {
                            vcl::Font aFont = ImplSetCorrectFontHeight();
                            maAttributeWriter.SetFontAttr( aFont );
                            ImplWriteText( pA->GetPoint(), aText, nullptr, pA->GetWidth() );
                        }
                        else
                        {
                            maTextWriter.writeTextPortion( pA->GetPoint(), aText );
                        }
                    }
                }
            }
            break;

            case MetaActionType::CLIPREGION:
            case MetaActionType::ISECTRECTCLIPREGION:
            case MetaActionType::ISECTREGIONCLIPREGION:
            case MetaActionType::MOVECLIPREGION:
            {
                const_cast<MetaAction*>(pAction)->Execute( mpVDev );
                const vcl::Region& rClipRegion = mpVDev->GetActiveClipRegion();
                ImplWriteClipPath( rClipRegion.GetAsPolyPolygon() );

                mbClipAttrChanged = true;
            }
            break;

            case MetaActionType::PUSH:
            {
                const MetaPushAction*  pA = static_cast<const MetaPushAction*>(pAction);
                PushFlags mnFlags = pA->GetFlags();

                const_cast<MetaAction*>(pAction)->Execute( mpVDev );

                maContextHandler.pushState( mnFlags );
            }
            break;

            case MetaActionType::POP:
            {
                const_cast<MetaAction*>(pAction)->Execute( mpVDev );

                PushFlags mnFlags = maContextHandler.getPushFlags();

                maContextHandler.popState();

                if( mnFlags & PushFlags::CLIPREGION )
                {
                    ImplEndClipRegion();
                    ImplStartClipRegion( mrCurrentState.nRegionClipPathId );
                }
            }
            break;

            case MetaActionType::REFPOINT:
            case MetaActionType::MAPMODE:
            case MetaActionType::LINECOLOR:
            case MetaActionType::FILLCOLOR:
            case MetaActionType::TEXTLINECOLOR:
            case MetaActionType::TEXTFILLCOLOR:
            case MetaActionType::TEXTCOLOR:
            case MetaActionType::TEXTALIGN:
            case MetaActionType::FONT:
            case MetaActionType::LAYOUTMODE:
            {
                const_cast<MetaAction*>(pAction)->Execute( mpVDev );
            }
            break;

            case MetaActionType::RASTEROP:
            case MetaActionType::MASK:
            case MetaActionType::MASKSCALE:
            case MetaActionType::MASKSCALEPART:
            case MetaActionType::WALLPAPER:
            case MetaActionType::TEXTLINE:
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


vcl::Font SVGActionWriter::ImplSetCorrectFontHeight() const
{
    vcl::Font aFont( mpVDev->GetFont() );
    Size      aSz;

    ImplMap( Size( 0, aFont.GetFontHeight() ), aSz );

    aFont.SetFontHeight( aSz.Height() );

    return aFont;
}


void SVGActionWriter::WriteMetaFile( const Point& rPos100thmm,
                                     const Size& rSize100thmm,
                                     const GDIMetaFile& rMtf,
                                     sal_uInt32 nWriteFlags,
                                     const OUString* pElementId,
                                     const Reference< css::drawing::XShape >* pXShape,
                                     const GDIMetaFile* pTextEmbeddedBitmapMtf )
{
    MapMode     aMapMode( rMtf.GetPrefMapMode() );
    Size        aPrefSize( rMtf.GetPrefSize() );
    Fraction    aFractionX( aMapMode.GetScaleX() );
    Fraction    aFractionY( aMapMode.GetScaleY() );

    mpVDev->Push();

    Size aSize( OutputDevice::LogicToLogic(rSize100thmm, MapMode(MapUnit::Map100thMM), aMapMode) );
    aFractionX *= Fraction( aSize.Width(), aPrefSize.Width() );
    aMapMode.SetScaleX( aFractionX );
    aFractionY *= Fraction( aSize.Height(), aPrefSize.Height() );
    aMapMode.SetScaleY( aFractionY );

    Point aOffset( OutputDevice::LogicToLogic(rPos100thmm, MapMode(MapUnit::Map100thMM), aMapMode ) );
    aOffset += aMapMode.GetOrigin();
    aMapMode.SetOrigin( aOffset );

    mpVDev->SetMapMode( aMapMode );

    mapCurShape.reset();

    ImplWriteActions( rMtf, nWriteFlags, pElementId, pXShape, pTextEmbeddedBitmapMtf );
    maTextWriter.endTextParagraph();
    ImplEndClipRegion();

    // draw open shape that doesn't have a border
    if (mapCurShape)
    {
        ImplWriteShape( *mapCurShape );
        mapCurShape.reset();
    }

    mpVDev->Pop();
}


SVGWriter::SVGWriter( const Sequence<Any>& args, const Reference< XComponentContext >& rxCtx )
    : mxContext(rxCtx)
{
    if(args.getLength()==1)
        args[0]>>=maFilterData;
}


SVGWriter::~SVGWriter()
{
}


void SAL_CALL SVGWriter::write( const Reference<XDocumentHandler>& rxDocHandler,
                                const Sequence<sal_Int8>& rMtfSeq )
{
    SvMemoryStream  aMemStm( const_cast<sal_Int8 *>(rMtfSeq.getConstArray()), rMtfSeq.getLength(), StreamMode::READ );
    GDIMetaFile     aMtf;

    ReadGDIMetaFile( aMemStm, aMtf );

    rtl::Reference<SVGExport> pWriter(new SVGExport( mxContext, rxDocHandler, maFilterData ));
    pWriter->writeMtf( aMtf );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
