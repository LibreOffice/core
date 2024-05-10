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

#include <o3tl/float_int_conversion.hxx>
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
#include <o3tl/string_view.hxx>
#include <svx/xbitmap.hxx>
#include <vcl/BitmapTools.hxx>
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

bool lclExtractDouble( double& orfValue, size_t& ornEndPos, std::u16string_view aValue )
{
    // extract the double value and find start position of unit characters
    rtl_math_ConversionStatus eConvStatus = rtl_math_ConversionStatus_Ok;
    sal_Int32 nEndPos;
    orfValue = ::rtl::math::stringToDouble( aValue, '.', '\0', &eConvStatus, &nEndPos );
    ornEndPos = nEndPos;
    return eConvStatus == rtl_math_ConversionStatus_Ok;
}

} // namespace

bool ConversionHelper::separatePair( std::u16string_view& orValue1, std::u16string_view& orValue2,
        std::u16string_view rValue, sal_Unicode cSep )
{
    size_t nSepPos = rValue.find( cSep );
    if( nSepPos != std::u16string_view::npos )
    {
        orValue1 = o3tl::trim(rValue.substr( 0, nSepPos ));
        orValue2 = o3tl::trim(rValue.substr( nSepPos + 1 ));
    }
    else
    {
        orValue1 = o3tl::trim(rValue);
        orValue2 = std::u16string_view();
    }
    return !orValue1.empty() && !orValue2.empty();
}

bool ConversionHelper::decodeBool( std::u16string_view rValue )
{
    sal_Int32 nToken = AttributeConversion::decodeToken( rValue );
    // anything else than 't' or 'true' is considered to be false, as specified
    return (nToken == XML_t) || (nToken == XML_true);
}

double ConversionHelper::decodePercent( std::u16string_view rValue, double fDefValue )
{
    if( rValue.empty() )
        return fDefValue;

    double fValue = 0.0;
    size_t nEndPos = 0;
    if( !lclExtractDouble( fValue, nEndPos, rValue ) )
        return fDefValue;

    if( nEndPos == rValue.size() )
        return fValue;

    if( (nEndPos + 1 == rValue.size()) && (rValue[ nEndPos ] == '%') )
        return fValue / 100.0;

    if( (nEndPos + 1 == rValue.size()) && (rValue[ nEndPos ] == 'f') )
        return fValue / 65536.0;

    OSL_FAIL( "ConversionHelper::decodePercent - unknown measure unit" );
    return fDefValue;
}

Degree100 ConversionHelper::decodeRotation( std::u16string_view rValue )
{
    if( rValue.empty() )
        return 0_deg100;

    double fValue = 0.0;
    double fRotation = 0.0;
    size_t nEndPos = 0;
    if( !lclExtractDouble(fValue, nEndPos, rValue) )
        return 0_deg100;

    if( nEndPos == rValue.size() )
        fRotation = fValue;
    else if( (nEndPos + 2 == rValue.size()) && (rValue[nEndPos] == 'f') && (rValue[nEndPos+1] == 'd') )
        fRotation = fValue / 65536.0;
    else
    {
        OSL_FAIL("ConversionHelper::decodeRotation - unknown measure unit");
        return 0_deg100;
    }

    return NormAngle36000(Degree100(static_cast<sal_Int32>(fRotation * -100)));
}

sal_Int64 ConversionHelper::decodeMeasureToEmu( const GraphicHelper& rGraphicHelper,
        std::u16string_view rValue, sal_Int32 nRefValue, bool bPixelX, bool bDefaultAsPixel )
{
    // default for missing values is 0
    if( rValue.empty() )
        return 0;

    // TODO: according to spec, value may contain "auto"
    if ( rValue == u"auto" )
    {
        OSL_FAIL( "ConversionHelper::decodeMeasureToEmu - special value 'auto' must be handled by caller" );
        return nRefValue;
    }

    // extract the double value and find start position of unit characters
    double fValue = 0.0;
    size_t nEndPos = 0;
    if( !lclExtractDouble( fValue, nEndPos, rValue ) || (fValue == 0.0) )
        return 0;

    // process trailing unit, convert to EMU
    std::u16string_view aUnit;
    if( (0 < nEndPos) && (nEndPos < rValue.size()) )
        aUnit = rValue.substr( nEndPos );
    else if( bDefaultAsPixel )
        aUnit = u"px";
    // else default is EMU

    if( aUnit.size() == 2 )
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
    else if( (aUnit.size() == 1) && (aUnit[ 0 ] == '%') )
    {
        fValue *= nRefValue / 100.0;
    }
    else if( bDefaultAsPixel || !aUnit.empty() )   // default as EMU and no unit -> do nothing
    {
        OSL_FAIL( "ConversionHelper::decodeMeasureToEmu - unknown measure unit" );
        fValue = nRefValue;
    }
    return o3tl::saturating_cast< sal_Int64 >( fValue + 0.5 );
}

sal_Int32 ConversionHelper::decodeMeasureToHmm( const GraphicHelper& rGraphicHelper,
        std::u16string_view rValue, sal_Int32 nRefValue, bool bPixelX, bool bDefaultAsPixel )
{
    return ::oox::drawingml::convertEmuToHmm( decodeMeasureToEmu( rGraphicHelper, rValue, nRefValue, bPixelX, bDefaultAsPixel ) );
}

sal_Int32 ConversionHelper::decodeMeasureToTwip(const GraphicHelper& rGraphicHelper,
                                                std::u16string_view rValue, sal_Int32 nRefValue,
                                                bool bPixelX, bool bDefaultAsPixel)
{
    return ::o3tl::convert(
        decodeMeasureToEmu(rGraphicHelper, rValue, nRefValue, bPixelX, bDefaultAsPixel),
        o3tl::Length::emu, o3tl::Length::twip);
}

Color ConversionHelper::decodeColor( const GraphicHelper& rGraphicHelper,
        const std::optional< OUString >& roVmlColor, const std::optional< double >& roVmlOpacity,
        ::Color nDefaultRgb, ::Color nPrimaryRgb )
{
    Color aDmlColor;

    // convert opacity
    const sal_Int32 DML_FULL_OPAQUE = ::oox::drawingml::MAX_PERCENT;
    double fOpacity = roVmlOpacity.value_or( 1.0 );
    sal_Int32 nOpacity = getLimitedValue< sal_Int32, double >( fOpacity * DML_FULL_OPAQUE, 0, DML_FULL_OPAQUE );
    if( nOpacity < DML_FULL_OPAQUE )
        aDmlColor.addTransformation( XML_alpha, nOpacity );

    // color attribute not present - set passed default color
    if( !roVmlColor.has_value() )
    {
        aDmlColor.setSrgbClr( nDefaultRgb );
        return aDmlColor;
    }

    // separate leading color name or RGB value from following palette index
    std::u16string_view aColorName, aColorIndex;
    separatePair( aColorName, aColorIndex, roVmlColor.value(), ' ' );

    // RGB colors in the format '#RRGGBB'
    if( (aColorName.size() == 7) && (aColorName[ 0 ] == '#') )
    {
        aDmlColor.setSrgbClr( o3tl::toUInt32(aColorName.substr( 1 ), 16) );
        return aDmlColor;
    }

    // RGB colors in the format '#RGB'
    if( (aColorName.size() == 4) && (aColorName[ 0 ] == '#') )
    {
        sal_Int32 nR = o3tl::toUInt32(aColorName.substr( 1, 1 ), 16 ) * 0x11;
        sal_Int32 nG = o3tl::toUInt32(aColorName.substr( 2, 1 ), 16 ) * 0x11;
        sal_Int32 nB = o3tl::toUInt32(aColorName.substr( 3, 1 ), 16 ) * 0x11;
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
    if( (aColorIndex.size() >= 3) && (aColorIndex[ 0 ] == '[') && (aColorIndex[ aColorIndex.size() - 1 ] == ']') )
    {
        aDmlColor.setPaletteClr( o3tl::toInt32(aColorIndex.substr( 1, aColorIndex.size() - 2 )) );
        return aDmlColor;
    }

    // try fill gradient modificator 'fill <modifier>(<amount>)'
    if( (nPrimaryRgb != API_RGB_TRANSPARENT) && (nColorToken == XML_fill) )
    {
        size_t nOpenParen = aColorIndex.find( '(' );
        size_t nCloseParen = aColorIndex.find( ')' );
        if( nOpenParen != std::u16string_view::npos && nCloseParen != std::u16string_view::npos &&
            (2 <= nOpenParen) && (nOpenParen + 1 < nCloseParen) && (nCloseParen + 1 == aColorIndex.size()) )
        {
            sal_Int32 nModToken = XML_TOKEN_INVALID;
            switch( AttributeConversion::decodeToken( aColorIndex.substr( 0, nOpenParen ) ) )
            {
                case XML_darken:    nModToken = XML_shade;break;
                case XML_lighten:   nModToken = XML_tint;
            }
            sal_Int32 nValue = o3tl::toInt32(aColorIndex.substr( nOpenParen + 1, nCloseParen - nOpenParen - 1 ));
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

    OSL_FAIL( OStringBuffer( "lclGetColor - invalid VML color name '" +
            OUStringToOString( roVmlColor.value(), RTL_TEXTENCODING_ASCII_US ) + "'" ).getStr() );
    aDmlColor.setSrgbClr( nDefaultRgb );
    return aDmlColor;
}

void ConversionHelper::decodeVmlPath( ::std::vector< ::std::vector< Point > >& rPointLists, ::std::vector< ::std::vector< PolygonFlags > >& rFlagLists, std::u16string_view rPath )
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

    for ( size_t i = 0; i < rPath.size(); i++ )
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
                    aCoordList.push_back( o3tl::toInt32(rPath.substr( nTokenStart, nTokenLen )) );
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
            if (i+1 < rPath.size())
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

sal_Int64 lclGetEmu( const GraphicHelper& rGraphicHelper, const std::optional< OUString >& roValue, sal_Int64 nDefValue )
{
    return roValue.has_value() ? ConversionHelper::decodeMeasureToEmu( rGraphicHelper, roValue.value(), 0, false, false ) : nDefValue;
}

void lclGetDmlLineDash( std::optional< sal_Int32 >& oroPresetDash, LineProperties::DashStopVector& orCustomDash, const std::optional< OUString >& roDashStyle )
{
    if( !roDashStyle.has_value() )
        return;

    const OUString& rDashStyle = roDashStyle.value();
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
                aValues.push_back( o3tl::toInt32(o3tl::getToken(rDashStyle, 0, ' ', nIndex )) );
            size_t nPairs = aValues.size() / 2; // ignore last value if size is odd
            for( size_t nPairIdx = 0; nPairIdx < nPairs; ++nPairIdx )
                orCustomDash.emplace_back( aValues[ 2 * nPairIdx ], aValues[ 2 * nPairIdx + 1 ] );
        }
    }
}

sal_Int32 lclGetDmlArrowType( const std::optional< sal_Int32 >& roArrowType )
{
    if( roArrowType.has_value() ) switch( roArrowType.value() )
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

sal_Int32 lclGetDmlArrowWidth( const std::optional< sal_Int32 >& roArrowWidth )
{
    if( roArrowWidth.has_value() ) switch( roArrowWidth.value() )
    {
        case XML_narrow:    return XML_sm;
        case XML_medium:    return XML_med;
        case XML_wide:      return XML_lg;
    }
    return XML_med;
}

sal_Int32 lclGetDmlArrowLength( const std::optional< sal_Int32 >& roArrowLength )
{
    if( roArrowLength.has_value() ) switch( roArrowLength.value() )
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

sal_Int32 lclGetDmlLineCompound( const std::optional< sal_Int32 >& roLineStyle )
{
    if( roLineStyle.has_value() ) switch( roLineStyle.value() )
    {
        case XML_single:            return XML_sng;
        case XML_thinThin:          return XML_dbl;
        case XML_thinThick:         return XML_thinThick;
        case XML_thickThin:         return XML_thickThin;
        case XML_thickBetweenThin:  return XML_tri;
    }
    return XML_sng;
}

sal_Int32 lclGetDmlLineCap( const std::optional< sal_Int32 >& roEndCap )
{
    if( roEndCap.has_value() ) switch( roEndCap.value() )
    {
        case XML_flat:      return XML_flat;
        case XML_square:    return XML_sq;
        case XML_round:     return XML_rnd;
    }
    return XML_flat;    // different defaults in VML (flat) and DrawingML (square)
}

sal_Int32 lclGetDmlLineJoint( const std::optional< sal_Int32 >& roJoinStyle )
{
    if( roJoinStyle.has_value() ) switch( roJoinStyle.value() )
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
    assignIfUsed( moArrowType, rSource.moArrowType );
    assignIfUsed( moArrowWidth, rSource.moArrowWidth );
    assignIfUsed( moArrowLength, rSource.moArrowLength );
}

void StrokeModel::assignUsed( const StrokeModel& rSource )
{
    assignIfUsed( moStroked, rSource.moStroked );
    maStartArrow.assignUsed( rSource.maStartArrow );
    maEndArrow.assignUsed( rSource.maEndArrow );
    assignIfUsed( moColor, rSource.moColor );
    assignIfUsed( moOpacity, rSource.moOpacity );
    assignIfUsed( moWeight, rSource.moWeight );
    assignIfUsed( moDashStyle, rSource.moDashStyle );
    assignIfUsed( moLineStyle, rSource.moLineStyle );
    assignIfUsed( moEndCap, rSource.moEndCap );
    assignIfUsed( moJoinStyle, rSource.moJoinStyle );
}

void StrokeModel::pushToPropMap( ShapePropertyMap& rPropMap, const GraphicHelper& rGraphicHelper ) const
{
    /*  Convert VML line formatting to DrawingML line formatting and let the
        DrawingML code do the hard work. */
    LineProperties aLineProps;

    if( moStroked.value_or( true ) )
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
    assignIfUsed( moFilled, rSource.moFilled );
    assignIfUsed( moColor, rSource.moColor );
    assignIfUsed( moOpacity, rSource.moOpacity );
    assignIfUsed( moColor2, rSource.moColor2 );
    assignIfUsed( moOpacity2, rSource.moOpacity2 );
    assignIfUsed( moType, rSource.moType );
    assignIfUsed( moAngle, rSource.moAngle );
    assignIfUsed( moFocus, rSource.moFocus );
    assignIfUsed( moFocusPos, rSource.moFocusPos );
    assignIfUsed( moFocusSize, rSource.moFocusSize );
    assignIfUsed( moBitmapPath, rSource.moBitmapPath );
    assignIfUsed( moRotate, rSource.moRotate );
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

    if( moFilled.value_or( true ) )
    {
        sal_Int32 nFillType = moType.value_or( XML_solid );
        switch( nFillType )
        {
            case XML_gradient:
            case XML_gradientRadial:
            {
                aFillProps.moFillType = XML_gradFill;
                aFillProps.maGradientProps.moRotateWithShape = moRotate.value_or( false );
                double fFocus = moFocus.value_or( 0.0 );

                // prepare colors
                Color aColor1 = ConversionHelper::decodeColor( rGraphicHelper, moColor, moOpacity, API_RGB_WHITE );
                Color aColor2 = ConversionHelper::decodeColor( rGraphicHelper, moColor2, moOpacity2, API_RGB_WHITE, aColor1.getColor( rGraphicHelper ) );

                // type XML_gradient is linear or axial gradient
                if( nFillType == XML_gradient )
                {
                    // normalize angle to range [0;360) degrees
                    sal_Int32 nVmlAngle = getIntervalValue< sal_Int32, sal_Int32 >( moAngle.value_or( 0 ), 0, 360 );

                    // focus of -50% or 50% is axial gradient
                    // so approximate anything with a similar focus by using LO's axial gradient,
                    // (otherwise drop the radial aspect; linear gradient becomes the closest match)
                    if( ((-0.75 <= fFocus) && (fFocus <= -0.25)) || ((0.25 <= fFocus) && (fFocus <= 0.75)) )
                    {
                        /*  According to spec, a focus of positive 50% is outer-to-inner,
                            and -50% is inner-to-outer (color to color2).
                            If the angle was provided as a negative,
                            then the colors are also (again) reversed. */
                        bool bOuterToInner = fFocus > 0.0;
                        if (moAngle.value_or(0) < 0)
                            bOuterToInner = !bOuterToInner;

                        // simulate axial gradient by 3-step DrawingML gradient
                        const Color& rOuterColor = bOuterToInner ? aColor1 : aColor2;
                        const Color& rInnerColor = bOuterToInner ? aColor2 : aColor1;

                        // add in order of offset
                        lcl_setGradientStop( aFillProps.maGradientProps.maGradientStops, 0.0, rOuterColor);
                        lcl_setGradientStop( aFillProps.maGradientProps.maGradientStops, 0.5, rInnerColor);
                        lcl_setGradientStop( aFillProps.maGradientProps.maGradientStops, 1.0, rOuterColor);
                    }
                    else    // focus of -100%, 0%, and 100% is linear gradient
                    {
                        // LO linear gradients: top == start, but for MSO bottom == start == moColor
                        bool bSwapColors = true;

                        /*  According to spec, a focus of -100% or 100% swaps the
                            start and stop colors, effectively reversing the gradient.
                            If the angle was provided as a negative,
                            then the colors are also (again) reversed. */
                        if( fFocus < -0.5 || fFocus > 0.5 )
                            bSwapColors = !bSwapColors;
                        if (moAngle.value_or(0) < 0)
                            bSwapColors = !bSwapColors;

                        const Color& rStartColor = bSwapColors ? aColor2 : aColor1;
                        const Color& rEndColor = bSwapColors ? aColor1 : aColor2;
                        // set the start and stop colors
                        lcl_setGradientStop(aFillProps.maGradientProps.maGradientStops, 0.0,
                                            rStartColor);
                        lcl_setGradientStop(aFillProps.maGradientProps.maGradientStops, 1.0,
                                            rEndColor);
                    }

                    // VML counts counterclockwise from bottom, DrawingML clockwise from left
                    sal_Int32 nDmlAngle = NormAngle360(90 - nVmlAngle);
                    aFillProps.maGradientProps.moShadeAngle = nDmlAngle * ::oox::drawingml::PER_DEGREE;
                }
                else    // XML_gradientRadial is rectangular gradient
                {
                    aFillProps.maGradientProps.moGradientPath = XML_rect;
                    // convert VML focus position and size to DrawingML fill-to-rect
                    DoublePair aFocusPos = moFocusPos.value_or( DoublePair( 0.0, 0.0 ) );
                    DoublePair aFocusSize = moFocusSize.value_or( DoublePair( 0.0, 0.0 ) );
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
                if( moBitmapPath.has_value() && !moBitmapPath.value().isEmpty() )
                {
                    aFillProps.maBlipProps.mxFillGraphic = rGraphicHelper.importEmbeddedGraphic(moBitmapPath.value());
                    if (aFillProps.maBlipProps.mxFillGraphic.is())
                    {
                        if (nFillType == XML_pattern)
                        {
                            // VML provides an 8x8 black(background) and white(foreground) pattern
                            // along with specified background(color2) and foreground(color) colors,
                            // while LO needs the color applied directly to the pattern.
                            const Graphic aGraphic(aFillProps.maBlipProps.mxFillGraphic);
                            ::Color nBackColor;
                            ::Color nPixelColor;
                            bool bIs8x8 = vcl::bitmap::isHistorical8x8(aGraphic.GetBitmapEx(),
                                                                       nBackColor, nPixelColor);
                            if (bIs8x8)
                            {
                                nBackColor
                                    = ConversionHelper::decodeColor(rGraphicHelper, moColor2,
                                                                    moOpacity2, API_RGB_WHITE)
                                          .getColor(rGraphicHelper);
                                // Documentation says undefined == white; observation says lightgray
                                nPixelColor
                                    = ConversionHelper::decodeColor(rGraphicHelper, moColor,
                                                                    moOpacity, COL_LIGHTGRAY)
                                          .getColor(rGraphicHelper);

                                XOBitmap aXOB(aGraphic.GetBitmapEx());
                                aXOB.Bitmap2Array();
                                // LO uses the first pixel's color to represent background pixels
                                if (aXOB.GetBackgroundColor() == COL_WHITE)
                                {
                                    // White always represents the foreground in VML => swap
                                    aXOB.SetPixelColor(nBackColor);
                                    aXOB.SetBackgroundColor(nPixelColor);
                                }
                                else
                                {
                                    assert(aXOB.GetBackgroundColor() == COL_BLACK);
                                    aXOB.SetPixelColor(nPixelColor);
                                    aXOB.SetBackgroundColor(nBackColor);
                                }
                                aXOB.Array2Bitmap();

                                Graphic aLOPattern(aXOB.GetBitmap());
                                aLOPattern.setOriginURL(aGraphic.getOriginURL());
                                aFillProps.maBlipProps.mxFillGraphic = aLOPattern.GetXGraphic();
                            }
                        }

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
    if (!mbHasShadow || (moShadowOn.has_value() && !moShadowOn.value()))
        return;

    drawingml::Color aColor = ConversionHelper::decodeColor(rGraphicHelper, moColor, moOpacity, API_RGB_GRAY);
    // nOffset* is in mm100, default value is 35 twips, see DffPropertyReader::ApplyAttributes() in msfilter.
    sal_Int32 nOffsetX = 62, nOffsetY = 62;
    if (moOffset.has_value())
    {
        std::u16string_view aOffsetX, aOffsetY;
        ConversionHelper::separatePair(aOffsetX, aOffsetY, moOffset.value(), ',');
        if (!aOffsetX.empty())
            nOffsetX = ConversionHelper::decodeMeasureToHmm(rGraphicHelper, aOffsetX, 0, false, false );
        if (!aOffsetY.empty())
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
    OUString sFont = u""_ustr;

    if (moString.has_value())
    {
        uno::Reference<text::XTextRange> xTextRange(xShape, uno::UNO_QUERY);
        xTextRange->setString(moString.value());

        uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aGeomPropSeq = xPropertySet->getPropertyValue(u"CustomShapeGeometry"_ustr).get< uno::Sequence<beans::PropertyValue> >();
        bool bFound = false;
        for (beans::PropertyValue& rProp : asNonConstRange(aGeomPropSeq))
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
            aGeomPropSeq.getArray()[nSize] = lcl_createTextpathProps();
        }
        rPropMap.setAnyProperty(PROP_CustomShapeGeometry, uno::Any(aGeomPropSeq));
    }
    if (moStyle.has_value())
    {
        OUString aStyle = moStyle.value_or(OUString());

        sal_Int32 nIndex = 0;
        while( nIndex >= 0 )
        {
            std::u16string_view aName, aValue;
            if (ConversionHelper::separatePair(aName, aValue, o3tl::getToken(aStyle, 0, ';', nIndex), ':'))
            {
                if (aName == u"font-family")
                {
                    // remove " (first, and last character)
                    if (aValue.size() > 2)
                        aValue = aValue.substr(1, aValue.size() - 2);

                    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
                    xPropertySet->setPropertyValue(u"CharFontName"_ustr, uno::Any(OUString(aValue)));
                    sFont = aValue;
                }
                else if (aName == u"font-size")
                {
                    std::optional<OUString> aOptString {OUString(aValue)};
                    float nSize = drawingml::convertEmuToPoints(lclGetEmu(rGraphicHelper, aOptString, 1));

                    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
                    xPropertySet->setPropertyValue(u"CharHeight"_ustr, uno::Any(nSize));
                }
            }
        }
    }
    if (moTrim.has_value() && moTrim.value())
        return;

    OUString sText = moString.value_or("");
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
