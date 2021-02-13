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

#include <sal/config.h>

#include <cstdlib>

#include <oox/vml/vmlformatting.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextPathMode.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <o3tl/unit_conversion.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <oox/drawingml/color.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/lineproperties.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <svx/svdtrans.hxx>
#include <comphelper/propertysequence.hxx>
#include <vcl/virdev.hxx>

namespace oox::vml {

using namespace ::com::sun::star;
using namespace ::com::sun::star::geometry;

using ::oox::drawingml::Color;
using ::oox::drawingml::FillProperties;
using ::oox::drawingml::LineArrowProperties;
using ::oox::drawingml::LineProperties;
using ::oox::drawingml::ShapePropertyMap;
using ::com::sun::star::awt::Point;
using ::com::sun::star::drawing::PolygonFlags;
using ::com::sun::star::drawing::PolygonFlags_NORMAL;
using ::com::sun::star::drawing::PolygonFlags_CONTROL;

namespace {

bool lclExtractDouble( double& orfValue, sal_Int32& ornEndPos, const OUString& rValue )
{
    // extract the double value and find start position of unit characters
    rtl_math_ConversionStatus eConvStatus = rtl_math_ConversionStatus_Ok;
    orfValue = ::rtl::math::stringToDouble( rValue, '.', '\0', &eConvStatus, &ornEndPos );
    return eConvStatus == rtl_math_ConversionStatus_Ok;
}

} // namespace

bool ConversionHelper::separatePair( OUString& orValue1, OUString& orValue2,
        const OUString& rValue, sal_Unicode cSep )
{
    sal_Int32 nSepPos = rValue.indexOf( cSep );
    if( nSepPos >= 0 )
    {
        orValue1 = rValue.copy( 0, nSepPos ).trim();
        orValue2 = rValue.copy( nSepPos + 1 ).trim();
    }
    else
    {
        orValue1 = rValue.trim();
    }
    return !orValue1.isEmpty() && !orValue2.isEmpty();
}

bool ConversionHelper::decodeBool( std::u16string_view rValue )
{
    sal_Int32 nToken = AttributeConversion::decodeToken( rValue );
    // anything else than 't' or 'true' is considered to be false, as specified
    return (nToken == XML_t) || (nToken == XML_true);
}

double ConversionHelper::decodePercent( const OUString& rValue, double fDefValue )
{
    if( rValue.isEmpty() )
        return fDefValue;

    double fValue = 0.0;
    sal_Int32 nEndPos = 0;
    if( !lclExtractDouble( fValue, nEndPos, rValue ) )
        return fDefValue;

    if( nEndPos == rValue.getLength() )
        return fValue;

    if( (nEndPos + 1 == rValue.getLength()) && (rValue[ nEndPos ] == '%') )
        return fValue / 100.0;

    if( (nEndPos + 1 == rValue.getLength()) && (rValue[ nEndPos ] == 'f') )
        return fValue / 65536.0;

    OSL_FAIL( "ConversionHelper::decodePercent - unknown measure unit" );
    return fDefValue;
}

Degree100 ConversionHelper::decodeRotation( const OUString& rValue )
{
    if( rValue.isEmpty() )
        return 0_deg100;

    double fValue = 0.0;
    double fRotation = 0.0;
    sal_Int32 nEndPos = 0;
    if( !lclExtractDouble(fValue, nEndPos, rValue) )
        return 0_deg100;

    if( nEndPos == rValue.getLength() )
        fRotation = fValue;
    else if( (nEndPos + 2 == rValue.getLength()) && (rValue[nEndPos] == 'f') && (rValue[nEndPos+1] == 'd') )
        fRotation = fValue / 65536.0;
    else
    {
        OSL_FAIL("ConversionHelper::decodeRotation - unknown measure unit");
        return 0_deg100;
    }

    return NormAngle36000(Degree100(static_cast<sal_Int32>(fRotation * -100)));
}

sal_Int64 ConversionHelper::decodeMeasureToEmu( const GraphicHelper& rGraphicHelper,
        const OUString& rValue, sal_Int32 nRefValue, bool bPixelX, bool bDefaultAsPixel )
{
    // default for missing values is 0
    if( rValue.isEmpty() )
        return 0;

    // TODO: according to spec, value may contain "auto"
    if ( rValue == "auto" )
    {
        OSL_FAIL( "ConversionHelper::decodeMeasureToEmu - special value 'auto' must be handled by caller" );
        return nRefValue;
    }

    // extract the double value and find start position of unit characters
    double fValue = 0.0;
    sal_Int32 nEndPos = 0;
    if( !lclExtractDouble( fValue, nEndPos, rValue ) || (fValue == 0.0) )
        return 0;

    // process trailing unit, convert to EMU
    OUString aUnit;
    if( (0 < nEndPos) && (nEndPos < rValue.getLength()) )
        aUnit = rValue.copy( nEndPos );
    else if( bDefaultAsPixel )
        aUnit = "px";
    // else default is EMU

    if( aUnit.getLength() == 2 )
    {
        sal_Unicode cChar1 = aUnit[ 0 ];
        sal_Unicode cChar2 = aUnit[ 1 ];
        if ((cChar1 == 'i') && (cChar2 == 'n'))
            fValue = o3tl::convert(fValue, o3tl::Length::in, o3tl::Length::emu);
        else if ((cChar1 == 'c') && (cChar2 == 'm'))
            fValue = o3tl::convert(fValue, o3tl::Length::cm, o3tl::Length::emu);
        else if ((cChar1 == 'm') && (cChar2 == 'm'))
            fValue = o3tl::convert(fValue, o3tl::Length::mm, o3tl::Length::emu);
        else if ((cChar1 == 'p') && (cChar2 == 't'))
            fValue = o3tl::convert(fValue, o3tl::Length::pt, o3tl::Length::emu);
        else if ((cChar1 == 'p') && (cChar2 == 'c'))
            fValue = o3tl::convert(fValue, o3tl::Length::pc, o3tl::Length::emu);
        else if( (cChar1 == 'p') && (cChar2 == 'x') )   // 1 pixel, dependent on output device
            fValue = o3tl::convert(bPixelX ? rGraphicHelper.convertScreenPixelXToHmm(fValue)
                                           : rGraphicHelper.convertScreenPixelYToHmm(fValue),
                                   o3tl::Length::mm100, o3tl::Length::emu);
    }
    else if( (aUnit.getLength() == 1) && (aUnit[ 0 ] == '%') )
    {
        fValue *= nRefValue / 100.0;
    }
    else if( bDefaultAsPixel || !aUnit.isEmpty() )   // default as EMU and no unit -> do nothing
    {
        OSL_FAIL( "ConversionHelper::decodeMeasureToEmu - unknown measure unit" );
        fValue = nRefValue;
    }
    return static_cast< sal_Int64 >( fValue + 0.5 );
}

sal_Int32 ConversionHelper::decodeMeasureToHmm( const GraphicHelper& rGraphicHelper,
        const OUString& rValue, sal_Int32 nRefValue, bool bPixelX, bool bDefaultAsPixel )
{
    return ::oox::drawingml::convertEmuToHmm( decodeMeasureToEmu( rGraphicHelper, rValue, nRefValue, bPixelX, bDefaultAsPixel ) );
}

Color ConversionHelper::decodeColor( const GraphicHelper& rGraphicHelper,
        const OptValue< OUString >& roVmlColor, const OptValue< double >& roVmlOpacity,
        ::Color nDefaultRgb, ::Color nPrimaryRgb )
{
    Color aDmlColor;

    // convert opacity
    const sal_Int32 DML_FULL_OPAQUE = ::oox::drawingml::MAX_PERCENT;
    double fOpacity = roVmlOpacity.get( 1.0 );
    sal_Int32 nOpacity = getLimitedValue< sal_Int32, double >( fOpacity * DML_FULL_OPAQUE, 0, DML_FULL_OPAQUE );
    if( nOpacity < DML_FULL_OPAQUE )
        aDmlColor.addTransformation( XML_alpha, nOpacity );

    // color attribute not present - set passed default color
    if( !roVmlColor.has() )
    {
        aDmlColor.setSrgbClr( nDefaultRgb );
        return aDmlColor;
    }

    // separate leading color name or RGB value from following palette index
    OUString aColorName, aColorIndex;
    separatePair( aColorName, aColorIndex, roVmlColor.get(), ' ' );

    // RGB colors in the format '#RRGGBB'
    if( (aColorName.getLength() == 7) && (aColorName[ 0 ] == '#') )
    {
        aDmlColor.setSrgbClr( aColorName.copy( 1 ).toUInt32( 16 ) );
        return aDmlColor;
    }

    // RGB colors in the format '#RGB'
    if( (aColorName.getLength() == 4) && (aColorName[ 0 ] == '#') )
    {
        sal_Int32 nR = aColorName.copy( 1, 1 ).toUInt32( 16 ) * 0x11;
        sal_Int32 nG = aColorName.copy( 2, 1 ).toUInt32( 16 ) * 0x11;
        sal_Int32 nB = aColorName.copy( 3, 1 ).toUInt32( 16 ) * 0x11;
        aDmlColor.setSrgbClr( (nR << 16) | (nG << 8) | nB );
        return aDmlColor;
    }

    /*  Predefined color names or system color names (resolve to RGB to detect
        valid color name). */
    sal_Int32 nColorToken = AttributeConversion::decodeToken( aColorName );
    ::Color nRgbValue = Color::getVmlPresetColor( nColorToken, API_RGB_TRANSPARENT );
    if( nRgbValue == API_RGB_TRANSPARENT )
        nRgbValue = rGraphicHelper.getSystemColor( nColorToken );
    if( nRgbValue != API_RGB_TRANSPARENT )
    {
        aDmlColor.setSrgbClr( nRgbValue );
        return aDmlColor;
    }

    // try palette colors enclosed in brackets
    if( (aColorIndex.getLength() >= 3) && (aColorIndex[ 0 ] == '[') && (aColorIndex[ aColorIndex.getLength() - 1 ] == ']') )
    {
        aDmlColor.setPaletteClr( aColorIndex.copy( 1, aColorIndex.getLength() - 2 ).toInt32() );
        return aDmlColor;
    }

    // try fill gradient modificator 'fill <modifier>(<amount>)'
    if( (nPrimaryRgb != API_RGB_TRANSPARENT) && (nColorToken == XML_fill) )
    {
        sal_Int32 nOpenParen = aColorIndex.indexOf( '(' );
        sal_Int32 nCloseParen = aColorIndex.indexOf( ')' );
        if( (2 <= nOpenParen) && (nOpenParen + 1 < nCloseParen) && (nCloseParen + 1 == aColorIndex.getLength()) )
        {
            sal_Int32 nModToken = XML_TOKEN_INVALID;
            switch( AttributeConversion::decodeToken( aColorIndex.subView( 0, nOpenParen ) ) )
            {
                case XML_darken:    nModToken = XML_shade;break;
                case XML_lighten:   nModToken = XML_tint;
            }
            sal_Int32 nValue = aColorIndex.copy( nOpenParen + 1, nCloseParen - nOpenParen - 1 ).toInt32();
            if( (nModToken != XML_TOKEN_INVALID) && (0 <= nValue) && (nValue < 255) )
            {
                /*  Simulate this modifier color by a color with related transformation.
                    The modifier amount has to be converted from the range [0;255] to
                    percentage [0;100000] used by DrawingML. */
                aDmlColor.setSrgbClr( nPrimaryRgb );
                aDmlColor.addTransformation( nModToken, static_cast< sal_Int32 >( nValue * ::oox::drawingml::MAX_PERCENT / 255 ) );
                return aDmlColor;
            }
        }
    }

    OSL_FAIL( OStringBuffer( "lclGetColor - invalid VML color name '" ).
        append( OUStringToOString( roVmlColor.get(), RTL_TEXTENCODING_ASCII_US ) ).append( '\'' ).getStr() );
    aDmlColor.setSrgbClr( nDefaultRgb );
    return aDmlColor;
}

void ConversionHelper::decodeVmlPath( ::std::vector< ::std::vector< Point > >& rPointLists, ::std::vector< ::std::vector< PolygonFlags > >& rFlagLists, const OUString& rPath )
{
    ::std::vector< sal_Int32 > aCoordList;
    Point aCurrentPoint;
    sal_Int32 nTokenStart = 0;
    sal_Int32 nTokenLen = 0;
    sal_Int32 nParamCount = 0;
    bool bCommand = false;
    enum VML_State { START, MOVE_REL, MOVE_ABS, BEZIER_REL, BEZIER_ABS,
                     LINE_REL, LINE_ABS, CLOSE, END, UNSUPPORTED };
    VML_State state = START;

    rPointLists.emplace_back( );
    rFlagLists.emplace_back( );

    for ( sal_Int32 i = 0; i < rPath.getLength(); i++ )
    {
        // Keep track of current integer token
        if ( ( rPath[ i ] >= '0' && rPath[ i ] <= '9' ) || rPath[ i ] == '-' )
            nTokenLen++;
        else if ( rPath[ i ] != ' ' )
        {
            // Store coordinate from current token
            if ( state != START && state != UNSUPPORTED )
            {
                if ( nTokenLen > 0 )
                    aCoordList.push_back( rPath.copy( nTokenStart, nTokenLen ).toInt32() );
                else
                    aCoordList.push_back( 0 );
                nTokenLen = 0;
            }

            if (rPath[ i ] == ',' )
            {
                nParamCount--;
            }

            // Upon finding the next command code, deal with stored
            // coordinates for previous command and reset parameters counter if needed.
            // See http://www.w3.org/TR/NOTE-VML#_Toc416858382 for params count reference
            if ( rPath[ i ] != ',' || nParamCount == 0 )
            {
                switch ( state )
                {
                case MOVE_REL:
                    aCoordList.resize(2, 0); // 2* params -> param count reset
                    if ( !rPointLists.empty() && !rPointLists.back().empty() )
                    {
                        rPointLists.emplace_back( );
                        rFlagLists.emplace_back( );
                    }
                    rPointLists.back().emplace_back( aCoordList[ 0 ], aCoordList[ 1 ] );
                    rFlagLists.back().push_back( PolygonFlags_NORMAL );
                    aCurrentPoint = rPointLists.back().back();
                    nParamCount = 2;
                    break;

                case MOVE_ABS:
                    aCoordList.resize(2, 0); // 2 params -> no param count reset
                    if ( !rPointLists.empty() && !rPointLists.back().empty() )
                    {
                        rPointLists.emplace_back( );
                        rFlagLists.emplace_back( );
                    }
                    rPointLists.back().emplace_back( (aCoordList[ 0 ]), aCoordList[ 1 ] );
                    rFlagLists.back().push_back( PolygonFlags_NORMAL );
                    aCurrentPoint = rPointLists.back().back();
                    break;

                case BEZIER_REL:
                    aCoordList.resize(6, 0); // 6* params -> param count reset
                    rPointLists.back().emplace_back( aCurrentPoint.X + aCoordList[ 0 ],
                                            aCurrentPoint.Y + aCoordList[ 1 ] );
                    rPointLists.back().emplace_back( aCurrentPoint.X + aCoordList[ 2 ],
                                            aCurrentPoint.Y + aCoordList[ 3 ] );
                    rPointLists.back().emplace_back( aCurrentPoint.X + aCoordList[ 4 ],
                                            aCurrentPoint.Y + aCoordList[ 5 ] );
                    rFlagLists.back().push_back( PolygonFlags_CONTROL );
                    rFlagLists.back().push_back( PolygonFlags_CONTROL );
                    rFlagLists.back().push_back( PolygonFlags_NORMAL );
                    aCurrentPoint = rPointLists.back().back();
                    nParamCount = 6;
                    break;

                case BEZIER_ABS:
                    aCoordList.resize(6, 0); // 6* params -> param count reset
                    rPointLists.back().emplace_back( aCoordList[ 0 ], aCoordList[ 1 ] );
                    rPointLists.back().emplace_back( aCoordList[ 2 ], aCoordList[ 3 ] );
                    rPointLists.back().emplace_back( aCoordList[ 4 ], aCoordList[ 5 ] );
                    rFlagLists.back().push_back( PolygonFlags_CONTROL );
                    rFlagLists.back().push_back( PolygonFlags_CONTROL );
                    rFlagLists.back().push_back( PolygonFlags_NORMAL );
                    aCurrentPoint = rPointLists.back().back();
                    nParamCount = 6;
                    break;

                case LINE_REL:
                    aCoordList.resize(2, 0); // 2* params -> param count reset
                    rPointLists.back().emplace_back( aCurrentPoint.X + aCoordList[ 0 ],
                                            aCurrentPoint.Y + aCoordList[ 1 ] );
                    rFlagLists.back().push_back( PolygonFlags_NORMAL );
                    aCurrentPoint = rPointLists.back().back();
                    nParamCount = 2;
                    break;

                case LINE_ABS:
                    aCoordList.resize(2, 0); // 2* params -> param count reset
                    rPointLists.back().emplace_back( aCoordList[ 0 ], (aCoordList.size() > 1 ? aCoordList[ 1 ] : 0) );
                    rFlagLists.back().push_back( PolygonFlags_NORMAL );
                    aCurrentPoint = rPointLists.back().back();
                    nParamCount = 2;
                    break;

                case CLOSE: // 0 param
                    SAL_WARN_IF(rPointLists.back().empty() || rFlagLists.back().empty(), "oox", "empty pointlists at close");
                    if (!rPointLists.back().empty() && !rFlagLists.back().empty())
                    {
                        rPointLists.back().push_back( rPointLists.back()[ 0 ] );
                        rFlagLists.back().push_back( rFlagLists.back()[ 0 ] );
                        aCurrentPoint = rPointLists.back().back();
                    }
                    break;

                case END: // 0 param
                    rPointLists.emplace_back( );
                    rFlagLists.emplace_back( );
                    break;

                case START:
                case UNSUPPORTED:
                    break;
                }

                aCoordList.clear();
            }

            // Allow two-char commands to peek ahead to the next character
            sal_Unicode nextChar = '\0';
            if (i+1 < rPath.getLength())
                nextChar = rPath[i+1];

            // Move to relevant state upon finding a command
            bCommand = true;
            switch ( rPath[ i ] )
            {
            // Single-character commands
            case 't': // rmoveto
                state = MOVE_REL; nParamCount = 2; break;
            case 'm': // moveto
                state = MOVE_ABS; nParamCount = 2; break;
            case 'v': // rcurveto
                state = BEZIER_REL; nParamCount = 6; break;
            case 'c': // curveto
                state = BEZIER_ABS; nParamCount = 6; break;
            case 'r': // rlineto
                state = LINE_REL; nParamCount = 2; break;
            case 'l': // lineto
                state = LINE_ABS; nParamCount = 2; break;
            case 'x': // close
                state = CLOSE; break;
            case 'e': // end
                state = END; break;

            // Two-character commands
            case 'n':
            {
                switch ( nextChar )
                {
                case 'f': // nf - nofill
                case 's': // ns - nostroke
                    state = UNSUPPORTED; i++; break;
                }
                break;
            }
            case 'a': // Elliptical curves
            {
                switch ( nextChar )
                {
                case 'e': // ae - angleellipseto
                case 'l': // al - angleellipse
                    state = UNSUPPORTED; i++; break;
                case 't': // at - arcto
                case 'r': // ar - arc
                    state = UNSUPPORTED; i++; break;
                }
                break;
            }
            case 'w': // Clockwise elliptical arcs
            {
                switch ( nextChar )
                {
                case 'a': // wa - clockwisearcto
                case 'r': // wr - clockwisearc
                    state = UNSUPPORTED; i++; break;
                }
                break;
            }
            case 'q':
            {
                switch ( nextChar )
                {
                case 'x': // qx - ellipticalquadrantx
                case 'y': // qy - ellipticalquadranty
                    state = UNSUPPORTED; i++; break;
                case 'b': // qb - quadraticbezier
                    state = UNSUPPORTED; i++; break;
                }
                break;
            }
            case 'h': // behaviour extensions
            {
                switch ( nextChar )
                {
                case 'a': // ha - AutoLine
                case 'b': // hb - AutoCurve
                case 'c': // hc - CornerLine
                case 'd': // hd - CornerCurve
                case 'e': // he - SmoothLine
                case 'f': // hf - SmoothCurve
                case 'g': // hg - SymmetricLine
                case 'h': // hh - SymmetricCurve
                case 'i': // hi - Freeform
                    state = UNSUPPORTED; i++; break;
                }
                break;
            }
            default:
                bCommand = false;
                break;
            }

            if (bCommand) nTokenLen = 0;
            nTokenStart = i+1;
        }
    }
}

namespace {

sal_Int64 lclGetEmu( const GraphicHelper& rGraphicHelper, const OptValue< OUString >& roValue, sal_Int64 nDefValue )
{
    return roValue.has() ? ConversionHelper::decodeMeasureToEmu( rGraphicHelper, roValue.get(), 0, false, false ) : nDefValue;
}

void lclGetDmlLineDash( OptValue< sal_Int32 >& oroPresetDash, LineProperties::DashStopVector& orCustomDash, const OptValue< OUString >& roDashStyle )
{
    if( !roDashStyle.has() )
        return;

    const OUString& rDashStyle = roDashStyle.get();
    switch( AttributeConversion::decodeToken( rDashStyle ) )
    {
        case XML_solid:             oroPresetDash = XML_solid;          return;
        case XML_shortdot:          oroPresetDash = XML_sysDot;         return;
        case XML_shortdash:         oroPresetDash = XML_sysDash;        return;
        case XML_shortdashdot:      oroPresetDash = XML_sysDashDot;     return;
        case XML_shortdashdotdot:   oroPresetDash = XML_sysDashDotDot;  return;
        case XML_dot:               oroPresetDash = XML_dot;            return;
        case XML_dash:              oroPresetDash = XML_dash;           return;
        case XML_dashdot:           oroPresetDash = XML_dashDot;        return;
        case XML_longdash:          oroPresetDash = XML_lgDash;         return;
        case XML_longdashdot:       oroPresetDash = XML_lgDashDot;      return;
        case XML_longdashdotdot:    oroPresetDash = XML_lgDashDotDot;   return;

        // try to convert user-defined dash style
        default:
        {
            ::std::vector< sal_Int32 > aValues;
            sal_Int32 nIndex = 0;
            while( nIndex >= 0 )
                aValues.push_back( rDashStyle.getToken( 0, ' ', nIndex ).toInt32() );
            size_t nPairs = aValues.size() / 2; // ignore last value if size is odd
            for( size_t nPairIdx = 0; nPairIdx < nPairs; ++nPairIdx )
                orCustomDash.emplace_back( aValues[ 2 * nPairIdx ], aValues[ 2 * nPairIdx + 1 ] );
        }
    }
}

sal_Int32 lclGetDmlArrowType( const OptValue< sal_Int32 >& roArrowType )
{
    if( roArrowType.has() ) switch( roArrowType.get() )
    {
        case XML_none:      return XML_none;
        case XML_block:     return XML_triangle;
        case XML_classic:   return XML_stealth;
        case XML_diamond:   return XML_diamond;
        case XML_oval:      return XML_oval;
        case XML_open:      return XML_arrow;
    }
    return XML_none;
}

sal_Int32 lclGetDmlArrowWidth( const OptValue< sal_Int32 >& roArrowWidth )
{
    if( roArrowWidth.has() ) switch( roArrowWidth.get() )
    {
        case XML_narrow:    return XML_sm;
        case XML_medium:    return XML_med;
        case XML_wide:      return XML_lg;
    }
    return XML_med;
}

sal_Int32 lclGetDmlArrowLength( const OptValue< sal_Int32 >& roArrowLength )
{
    if( roArrowLength.has() ) switch( roArrowLength.get() )
    {
        case XML_short:     return XML_sm;
        case XML_medium:    return XML_med;
        case XML_long:      return XML_lg;
    }
    return XML_med;
}

void lclConvertArrow( LineArrowProperties& orArrowProp, const StrokeArrowModel& rStrokeArrow )
{
    orArrowProp.moArrowType = lclGetDmlArrowType( rStrokeArrow.moArrowType );
    orArrowProp.moArrowWidth = lclGetDmlArrowWidth( rStrokeArrow.moArrowWidth );
    orArrowProp.moArrowLength = lclGetDmlArrowLength( rStrokeArrow.moArrowLength );
}

sal_Int32 lclGetDmlLineCompound( const OptValue< sal_Int32 >& roLineStyle )
{
    if( roLineStyle.has() ) switch( roLineStyle.get() )
    {
        case XML_single:            return XML_sng;
        case XML_thinThin:          return XML_dbl;
        case XML_thinThick:         return XML_thinThick;
        case XML_thickThin:         return XML_thickThin;
        case XML_thickBetweenThin:  return XML_tri;
    }
    return XML_sng;
}

sal_Int32 lclGetDmlLineCap( const OptValue< sal_Int32 >& roEndCap )
{
    if( roEndCap.has() ) switch( roEndCap.get() )
    {
        case XML_flat:      return XML_flat;
        case XML_square:    return XML_sq;
        case XML_round:     return XML_rnd;
    }
    return XML_flat;    // different defaults in VML (flat) and DrawingML (square)
}

sal_Int32 lclGetDmlLineJoint( const OptValue< sal_Int32 >& roJoinStyle )
{
    if( roJoinStyle.has() ) switch( roJoinStyle.get() )
    {
        case XML_round: return XML_round;
        case XML_bevel: return XML_bevel;
        case XML_miter: return XML_miter;
    }
    return XML_round;
}

} // namespace

void StrokeArrowModel::assignUsed( const StrokeArrowModel& rSource )
{
    moArrowType.assignIfUsed( rSource.moArrowType );
    moArrowWidth.assignIfUsed( rSource.moArrowWidth );
    moArrowLength.assignIfUsed( rSource.moArrowLength );
}

void StrokeModel::assignUsed( const StrokeModel& rSource )
{
    moStroked.assignIfUsed( rSource.moStroked );
    maStartArrow.assignUsed( rSource.maStartArrow );
    maEndArrow.assignUsed( rSource.maEndArrow );
    moColor.assignIfUsed( rSource.moColor );
    moOpacity.assignIfUsed( rSource.moOpacity );
    moWeight.assignIfUsed( rSource.moWeight );
    moDashStyle.assignIfUsed( rSource.moDashStyle );
    moLineStyle.assignIfUsed( rSource.moLineStyle );
    moEndCap.assignIfUsed( rSource.moEndCap );
    moJoinStyle.assignIfUsed( rSource.moJoinStyle );
}

void StrokeModel::pushToPropMap( ShapePropertyMap& rPropMap, const GraphicHelper& rGraphicHelper ) const
{
    /*  Convert VML line formatting to DrawingML line formatting and let the
        DrawingML code do the hard work. */
    LineProperties aLineProps;

    if( moStroked.get( true ) )
    {
        aLineProps.maLineFill.moFillType = XML_solidFill;
        lclConvertArrow( aLineProps.maStartArrow, maStartArrow );
        lclConvertArrow( aLineProps.maEndArrow, maEndArrow );
        aLineProps.maLineFill.maFillColor = ConversionHelper::decodeColor( rGraphicHelper, moColor, moOpacity, API_RGB_BLACK );
        aLineProps.moLineWidth = getLimitedValue< sal_Int32, sal_Int64 >( lclGetEmu( rGraphicHelper, moWeight, 1 ), 0, SAL_MAX_INT32 );
        lclGetDmlLineDash( aLineProps.moPresetDash, aLineProps.maCustomDash, moDashStyle );
        aLineProps.moLineCompound = lclGetDmlLineCompound( moLineStyle );
        aLineProps.moLineCap = lclGetDmlLineCap( moEndCap );
        aLineProps.moLineJoint = lclGetDmlLineJoint( moJoinStyle );
    }
    else
    {
        aLineProps.maLineFill.moFillType = XML_noFill;
    }

    aLineProps.pushToPropMap( rPropMap, rGraphicHelper );
}

void FillModel::assignUsed( const FillModel& rSource )
{
    moFilled.assignIfUsed( rSource.moFilled );
    moColor.assignIfUsed( rSource.moColor );
    moOpacity.assignIfUsed( rSource.moOpacity );
    moColor2.assignIfUsed( rSource.moColor2 );
    moOpacity2.assignIfUsed( rSource.moOpacity2 );
    moType.assignIfUsed( rSource.moType );
    moAngle.assignIfUsed( rSource.moAngle );
    moFocus.assignIfUsed( rSource.moFocus );
    moFocusPos.assignIfUsed( rSource.moFocusPos );
    moFocusSize.assignIfUsed( rSource.moFocusSize );
    moBitmapPath.assignIfUsed( rSource.moBitmapPath );
    moRotate.assignIfUsed( rSource.moRotate );
}

static void lcl_setGradientStop( std::multimap< double, Color >& rMap, const double fKey, const Color& rValue ) {
    auto aElement = rMap.find( fKey );

    if (aElement != rMap.end())
        aElement->second = rValue;
    else
        rMap.emplace( fKey, rValue );
}

void FillModel::pushToPropMap( ShapePropertyMap& rPropMap, const GraphicHelper& rGraphicHelper ) const
{
    /*  Convert VML fill formatting to DrawingML fill formatting and let the
        DrawingML code do the hard work. */
    FillProperties aFillProps;

    if( moFilled.get( true ) )
    {
        sal_Int32 nFillType = moType.get( XML_solid );
        switch( nFillType )
        {
            case XML_gradient:
            case XML_gradientRadial:
            {
                aFillProps.moFillType = XML_gradFill;
                aFillProps.maGradientProps.moRotateWithShape = moRotate.get( false );
                double fFocus = moFocus.get( 0.0 );

                // prepare colors
                Color aColor1 = ConversionHelper::decodeColor( rGraphicHelper, moColor, moOpacity, API_RGB_WHITE );
                Color aColor2 = ConversionHelper::decodeColor( rGraphicHelper, moColor2, moOpacity2, API_RGB_WHITE, aColor1.getColor( rGraphicHelper ) );

                // type XML_gradient is linear or axial gradient
                if( nFillType == XML_gradient )
                {
                    // normalize angle to range [0;360) degrees
                    sal_Int32 nVmlAngle = getIntervalValue< sal_Int32, sal_Int32 >( moAngle.get( 0 ), 0, 360 );

                    // focus of -50% or 50% is axial gradient
                    if( ((-0.75 <= fFocus) && (fFocus <= -0.25)) || ((0.25 <= fFocus) && (fFocus <= 0.75)) )
                    {
                        /*  According to spec, focus of 50% is outer-to-inner,
                            and -50% is inner-to-outer (color to color2).
                            BUT: For angles >= 180 deg., the behaviour is
                            reversed... that's not spec'ed of course. So,
                            [0;180) deg. and 50%, or [180;360) deg. and -50% is
                            outer-to-inner in fact. */
                        bool bOuterToInner = (fFocus > 0.0) == (nVmlAngle < 180);
                        // simulate axial gradient by 3-step DrawingML gradient
                        const Color& rOuterColor = bOuterToInner ? aColor1 : aColor2;
                        const Color& rInnerColor = bOuterToInner ? aColor2 : aColor1;

                        lcl_setGradientStop( aFillProps.maGradientProps.maGradientStops, 0.0, rOuterColor);
                        lcl_setGradientStop( aFillProps.maGradientProps.maGradientStops, 1.0, rOuterColor);
                        lcl_setGradientStop( aFillProps.maGradientProps.maGradientStops, 0.5, rInnerColor );
                    }
                    else    // focus of -100%, 0%, and 100% is linear gradient
                    {
                        /*  According to spec, focus of -100% or 100% swaps the
                            start and stop colors, effectively reversing the
                            gradient. BUT: For angles >= 180 deg., the
                            behaviour is reversed. This means that in this case
                            a focus of 0% swaps the gradient. */
                        if( fFocus < -0.5 || fFocus > 0.5 )
                            nVmlAngle = (nVmlAngle + 180) % 360;
                        // set the start and stop colors
                        lcl_setGradientStop( aFillProps.maGradientProps.maGradientStops, 0.0, aColor1 );
                        lcl_setGradientStop( aFillProps.maGradientProps.maGradientStops, 1.0, aColor2 );
                    }

                    // VML counts counterclockwise from bottom, DrawingML clockwise from left
                    sal_Int32 nDmlAngle = (630 - nVmlAngle) % 360;
                    aFillProps.maGradientProps.moShadeAngle = nDmlAngle * ::oox::drawingml::PER_DEGREE;
                }
                else    // XML_gradientRadial is rectangular gradient
                {
                    aFillProps.maGradientProps.moGradientPath = XML_rect;
                    // convert VML focus position and size to DrawingML fill-to-rect
                    DoublePair aFocusPos = moFocusPos.get( DoublePair( 0.0, 0.0 ) );
                    DoublePair aFocusSize = moFocusSize.get( DoublePair( 0.0, 0.0 ) );
                    double fLeft   = getLimitedValue< double, double >( aFocusPos.first, 0.0, 1.0 );
                    double fTop    = getLimitedValue< double, double >( aFocusPos.second, 0.0, 1.0 );
                    double fRight  = getLimitedValue< double, double >( fLeft + aFocusSize.first, fLeft, 1.0 );
                    double fBottom = getLimitedValue< double, double >( fTop + aFocusSize.second, fTop, 1.0 );
                    aFillProps.maGradientProps.moFillToRect = IntegerRectangle2D(
                        static_cast< sal_Int32 >( fLeft * ::oox::drawingml::MAX_PERCENT ),
                        static_cast< sal_Int32 >( fTop * ::oox::drawingml::MAX_PERCENT ),
                        static_cast< sal_Int32 >( (1.0 - fRight) * ::oox::drawingml::MAX_PERCENT ),
                        static_cast< sal_Int32 >( (1.0 - fBottom) * ::oox::drawingml::MAX_PERCENT ) );

                    // set the start and stop colors (focus of 0% means outer-to-inner)
                    bool bOuterToInner = (-0.5 <= fFocus) && (fFocus <= 0.5);
                    lcl_setGradientStop( aFillProps.maGradientProps.maGradientStops, 0.0, bOuterToInner ? aColor2 : aColor1 );
                    lcl_setGradientStop( aFillProps.maGradientProps.maGradientStops, 1.0, bOuterToInner ? aColor1 : aColor2 );
                }
            }
            break;

            case XML_pattern:
            case XML_tile:
            case XML_frame:
            {
                if( moBitmapPath.has() && !moBitmapPath.get().isEmpty() )
                {
                    aFillProps.maBlipProps.mxFillGraphic = rGraphicHelper.importEmbeddedGraphic(moBitmapPath.get());
                    if (aFillProps.maBlipProps.mxFillGraphic.is())
                    {
                        aFillProps.moFillType = XML_blipFill;
                        aFillProps.maBlipProps.moBitmapMode = (nFillType == XML_frame) ? XML_stretch : XML_tile;
                        break;  // do not break if bitmap is missing, but run to XML_solid instead
                    }
                }
            }
            [[fallthrough]]; // to XML_solid in case of missing bitmap path intended!

            case XML_solid:
            default:
            {
                aFillProps.moFillType = XML_solidFill;
                // fill color (default is white)
                aFillProps.maFillColor = ConversionHelper::decodeColor( rGraphicHelper, moColor, moOpacity, API_RGB_WHITE );
            }
        }
    }
    else
    {
        aFillProps.moFillType = XML_noFill;
    }

    aFillProps.pushToPropMap( rPropMap, rGraphicHelper );
}

ShadowModel::ShadowModel()
        : mbHasShadow(false)
{
}

void ShadowModel::pushToPropMap(ShapePropertyMap& rPropMap, const GraphicHelper& rGraphicHelper) const
{
    if (!mbHasShadow || (moShadowOn.has() && !moShadowOn.get()))
        return;

    drawingml::Color aColor = ConversionHelper::decodeColor(rGraphicHelper, moColor, moOpacity, API_RGB_GRAY);
    // nOffset* is in mm100, default value is 35 twips, see DffPropertyReader::ApplyAttributes() in msfilter.
    sal_Int32 nOffsetX = 62, nOffsetY = 62;
    if (moOffset.has())
    {
        OUString aOffsetX, aOffsetY;
        ConversionHelper::separatePair(aOffsetX, aOffsetY, moOffset.get(), ',');
        if (!aOffsetX.isEmpty())
            nOffsetX = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, aOffsetX, 0, false, false );
        if (!aOffsetY.isEmpty())
            nOffsetY = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, aOffsetY, 0, false, false );
    }

    table::ShadowFormat aFormat;
    aFormat.Color = sal_Int32(aColor.getColor(rGraphicHelper));
    aFormat.Location = nOffsetX < 0
        ? nOffsetY < 0 ? table::ShadowLocation_TOP_LEFT : table::ShadowLocation_BOTTOM_LEFT
        : nOffsetY < 0 ? table::ShadowLocation_TOP_RIGHT : table::ShadowLocation_BOTTOM_RIGHT;
    // The width of the shadow is the average of the x and y values, see SwWW8ImplReader::MatchSdrItemsIntoFlySet().
    aFormat.ShadowWidth = ((std::abs(nOffsetX) + std::abs(nOffsetY)) / 2);
    rPropMap.setProperty(PROP_ShadowFormat, aFormat);
}

TextpathModel::TextpathModel()
{
}

static beans::PropertyValue lcl_createTextpathProps()
{
    uno::Sequence<beans::PropertyValue> aTextpathPropSeq( comphelper::InitPropertySequence({
            { "TextPath", uno::Any(true) },
            { "TextPathMode", uno::Any(drawing::EnhancedCustomShapeTextPathMode_SHAPE) },
            { "ScaleX", uno::Any(false) },
            { "SameLetterHeights", uno::Any(false) }
        }));

    beans::PropertyValue aRet;
    aRet.Name = "TextPath";
    aRet.Value <<= aTextpathPropSeq;
    return aRet;
}

void TextpathModel::pushToPropMap(ShapePropertyMap& rPropMap, const uno::Reference<drawing::XShape>& xShape, const GraphicHelper& rGraphicHelper) const
{
    OUString sFont = "";

    if (moString.has())
    {
        uno::Reference<text::XTextRange> xTextRange(xShape, uno::UNO_QUERY);
        xTextRange->setString(moString.get());

        uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aGeomPropSeq = xPropertySet->getPropertyValue("CustomShapeGeometry").get< uno::Sequence<beans::PropertyValue> >();
        bool bFound = false;
        for (beans::PropertyValue& rProp : aGeomPropSeq)
        {
            if (rProp.Name == "TextPath")
            {
                bFound = true;
                rProp = lcl_createTextpathProps();
            }
        }
        if (!bFound)
        {
            sal_Int32 nSize = aGeomPropSeq.getLength();
            aGeomPropSeq.realloc(nSize+1);
            aGeomPropSeq[nSize] = lcl_createTextpathProps();
        }
        rPropMap.setAnyProperty(PROP_CustomShapeGeometry, uno::makeAny(aGeomPropSeq));
    }
    if (moStyle.has())
    {
        OUString aStyle = moStyle.get(OUString());

        sal_Int32 nIndex = 0;
        while( nIndex >= 0 )
        {
            OUString aName, aValue;
            if (ConversionHelper::separatePair(aName, aValue, aStyle.getToken(0, ';', nIndex), ':'))
            {
                if (aName == "font-family")
                {
                    // remove " (first, and last character)
                    if (aValue.getLength() > 2)
                        aValue = aValue.copy(1, aValue.getLength() - 2);

                    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
                    xPropertySet->setPropertyValue("CharFontName", uno::makeAny(aValue));
                    sFont = aValue;
                }
                else if (aName == "font-size")
                {
                    oox::OptValue<OUString> aOptString(aValue);
                    float nSize = drawingml::convertEmuToPoints(lclGetEmu(rGraphicHelper, aOptString, 1));

                    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
                    xPropertySet->setPropertyValue("CharHeight", uno::makeAny(nSize));
                }
            }
        }
    }
    if (moTrim.has() && moTrim.get())
        return;

    OUString sText = moString.get();
    ScopedVclPtrInstance<VirtualDevice> pDevice;
    vcl::Font aFont = pDevice->GetFont();
    aFont.SetFamilyName(sFont);
    aFont.SetFontSize(Size(0, 96));
    pDevice->SetFont(aFont);

    auto nTextWidth = pDevice->GetTextWidth(sText);
    if (nTextWidth)
    {
        sal_Int32 nNewHeight = (static_cast<double>(pDevice->GetTextHeight()) / nTextWidth) * xShape->getSize().Width;
        xShape->setSize(awt::Size(xShape->getSize().Width, nNewHeight));
    }
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
