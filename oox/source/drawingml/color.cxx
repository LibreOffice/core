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

#include <oox/drawingml/color.hxx>
#include <algorithm>
#include <math.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

namespace oox::drawingml {

namespace {

/** Global storage for predefined color values used in OOXML file formats. */
struct PresetColorsPool
{
    typedef ::std::vector< ::Color > ColorVector;

    ColorVector         maDmlColors;        /// Predefined colors in DrawingML, indexed by XML token.
    ColorVector         maVmlColors;        /// Predefined colors in VML, indexed by XML token.
    ColorVector         maHighlightColors;  /// Predefined colors in DrawingML for highlight, indexed by XML token.

    explicit            PresetColorsPool();
};

PresetColorsPool::PresetColorsPool() :
    maDmlColors( static_cast< size_t >( XML_TOKEN_COUNT ), API_RGB_TRANSPARENT ),
    maVmlColors( static_cast< size_t >( XML_TOKEN_COUNT ), API_RGB_TRANSPARENT ),
    maHighlightColors( static_cast<size_t>(XML_TOKEN_COUNT), API_RGB_TRANSPARENT )
{
    // predefined colors in DrawingML (map XML token identifiers to RGB values)
    static const std::pair<sal_Int32, ::Color> spnDmlColors[] =
    {
        {XML_aliceBlue,         ::Color(0xF0,0xF8,0xFF)},    {XML_antiqueWhite,      ::Color(0xFA,0xEB,0xD7)},
        {XML_aqua,              ::Color(0x00,0xFF,0xFF)},    {XML_aquamarine,        ::Color(0x7F,0xFF,0xD4)},
        {XML_azure,             ::Color(0xF0,0xFF,0xFF)},    {XML_beige,             ::Color(0xF5,0xF5,0xDC)},
        {XML_bisque,            ::Color(0xFF,0xE4,0xC4)},    {XML_black,             ::Color(0x00,0x00,0x00)},
        {XML_blanchedAlmond,    ::Color(0xFF,0xEB,0xCD)},    {XML_blue,              ::Color(0x00,0x00,0xFF)},
        {XML_blueViolet,        ::Color(0x8A,0x2B,0xE2)},    {XML_brown,             ::Color(0xA5,0x2A,0x2A)},
        {XML_burlyWood,         ::Color(0xDE,0xB8,0x87)},    {XML_cadetBlue,         ::Color(0x5F,0x9E,0xA0)},
        {XML_chartreuse,        ::Color(0x7F,0xFF,0x00)},    {XML_chocolate,         ::Color(0xD2,0x69,0x1E)},
        {XML_coral,             ::Color(0xFF,0x7F,0x50)},    {XML_cornflowerBlue,    ::Color(0x64,0x95,0xED)},
        {XML_cornsilk,          ::Color(0xFF,0xF8,0xDC)},    {XML_crimson,           ::Color(0xDC,0x14,0x3C)},
        {XML_cyan,              ::Color(0x00,0xFF,0xFF)},    {XML_deepPink,          ::Color(0xFF,0x14,0x93)},
        {XML_deepSkyBlue,       ::Color(0x00,0xBF,0xFF)},    {XML_dimGray,           ::Color(0x69,0x69,0x69)},
        {XML_dkBlue,            ::Color(0x00,0x00,0x8B)},    {XML_dkCyan,            ::Color(0x00,0x8B,0x8B)},
        {XML_dkGoldenrod,       ::Color(0xB8,0x86,0x0B)},    {XML_dkGray,            ::Color(0xA9,0xA9,0xA9)},
        {XML_dkGreen,           ::Color(0x00,0x64,0x00)},    {XML_dkKhaki,           ::Color(0xBD,0xB7,0x6B)},
        {XML_dkMagenta,         ::Color(0x8B,0x00,0x8B)},    {XML_dkOliveGreen,      ::Color(0x55,0x6B,0x2F)},
        {XML_dkOrange,          ::Color(0xFF,0x8C,0x00)},    {XML_dkOrchid,          ::Color(0x99,0x32,0xCC)},
        {XML_dkRed,             ::Color(0x8B,0x00,0x00)},    {XML_dkSalmon,          ::Color(0xE9,0x96,0x7A)},
        {XML_dkSeaGreen,        ::Color(0x8F,0xBC,0x8B)},    {XML_dkSlateBlue,       ::Color(0x48,0x3D,0x8B)},
        {XML_dkSlateGray,       ::Color(0x2F,0x4F,0x4F)},    {XML_dkTurquoise,       ::Color(0x00,0xCE,0xD1)},
        {XML_dkViolet,          ::Color(0x94,0x00,0xD3)},    {XML_dodgerBlue,        ::Color(0x1E,0x90,0xFF)},
        {XML_firebrick,         ::Color(0xB2,0x22,0x22)},    {XML_floralWhite,       ::Color(0xFF,0xFA,0xF0)},
        {XML_forestGreen,       ::Color(0x22,0x8B,0x22)},    {XML_fuchsia,           ::Color(0xFF,0x00,0xFF)},
        {XML_gainsboro,         ::Color(0xDC,0xDC,0xDC)},    {XML_ghostWhite,        ::Color(0xF8,0xF8,0xFF)},
        {XML_gold,              ::Color(0xFF,0xD7,0x00)},    {XML_goldenrod,         ::Color(0xDA,0xA5,0x20)},
        {XML_gray,              ::Color(0x80,0x80,0x80)},    {XML_green,             ::Color(0x00,0x80,0x00)},
        {XML_greenYellow,       ::Color(0xAD,0xFF,0x2F)},    {XML_honeydew,          ::Color(0xF0,0xFF,0xF0)},
        {XML_hotPink,           ::Color(0xFF,0x69,0xB4)},    {XML_indianRed,         ::Color(0xCD,0x5C,0x5C)},
        {XML_indigo,            ::Color(0x4B,0x00,0x82)},    {XML_ivory,             ::Color(0xFF,0xFF,0xF0)},
        {XML_khaki,             ::Color(0xF0,0xE6,0x8C)},    {XML_lavender,          ::Color(0xE6,0xE6,0xFA)},
        {XML_lavenderBlush,     ::Color(0xFF,0xF0,0xF5)},    {XML_lawnGreen,         ::Color(0x7C,0xFC,0x00)},
        {XML_lemonChiffon,      ::Color(0xFF,0xFA,0xCD)},    {XML_lime,              ::Color(0x00,0xFF,0x00)},
        {XML_limeGreen,         ::Color(0x32,0xCD,0x32)},    {XML_linen,             ::Color(0xFA,0xF0,0xE6)},
        {XML_ltBlue,            ::Color(0xAD,0xD8,0xE6)},    {XML_ltCoral,           ::Color(0xF0,0x80,0x80)},
        {XML_ltCyan,            ::Color(0xE0,0xFF,0xFF)},    {XML_ltGoldenrodYellow, ::Color(0xFA,0xFA,0x78)},
        {XML_ltGray,            ::Color(0xD3,0xD3,0xD3)},    {XML_ltGreen,           ::Color(0x90,0xEE,0x90)},
        {XML_ltPink,            ::Color(0xFF,0xB6,0xC1)},    {XML_ltSalmon,          ::Color(0xFF,0xA0,0x7A)},
        {XML_ltSeaGreen,        ::Color(0x20,0xB2,0xAA)},    {XML_ltSkyBlue,         ::Color(0x87,0xCE,0xFA)},
        {XML_ltSlateGray,       ::Color(0x77,0x88,0x99)},    {XML_ltSteelBlue,       ::Color(0xB0,0xC4,0xDE)},
        {XML_ltYellow,          ::Color(0xFF,0xFF,0xE0)},    {XML_magenta,           ::Color(0xFF,0x00,0xFF)},
        {XML_maroon,            ::Color(0x80,0x00,0x00)},    {XML_medAquamarine,     ::Color(0x66,0xCD,0xAA)},
        {XML_medBlue,           ::Color(0x00,0x00,0xCD)},    {XML_medOrchid,         ::Color(0xBA,0x55,0xD3)},
        {XML_medPurple,         ::Color(0x93,0x70,0xDB)},    {XML_medSeaGreen,       ::Color(0x3C,0xB3,0x71)},
        {XML_medSlateBlue,      ::Color(0x7B,0x68,0xEE)},    {XML_medSpringGreen,    ::Color(0x00,0xFA,0x9A)},
        {XML_medTurquoise,      ::Color(0x48,0xD1,0xCC)},    {XML_medVioletRed,      ::Color(0xC7,0x15,0x85)},
        {XML_midnightBlue,      ::Color(0x19,0x19,0x70)},    {XML_mintCream,         ::Color(0xF5,0xFF,0xFA)},
        {XML_mistyRose,         ::Color(0xFF,0xE4,0xE1)},    {XML_moccasin,          ::Color(0xFF,0xE4,0xB5)},
        {XML_navajoWhite,       ::Color(0xFF,0xDE,0xAD)},    {XML_navy,              ::Color(0x00,0x00,0x80)},
        {XML_oldLace,           ::Color(0xFD,0xF5,0xE6)},    {XML_olive,             ::Color(0x80,0x80,0x00)},
        {XML_oliveDrab,         ::Color(0x6B,0x8E,0x23)},    {XML_orange,            ::Color(0xFF,0xA5,0x00)},
        {XML_orangeRed,         ::Color(0xFF,0x45,0x00)},    {XML_orchid,            ::Color(0xDA,0x70,0xD6)},
        {XML_paleGoldenrod,     ::Color(0xEE,0xE8,0xAA)},    {XML_paleGreen,         ::Color(0x98,0xFB,0x98)},
        {XML_paleTurquoise,     ::Color(0xAF,0xEE,0xEE)},    {XML_paleVioletRed,     ::Color(0xDB,0x70,0x93)},
        {XML_papayaWhip,        ::Color(0xFF,0xEF,0xD5)},    {XML_peachPuff,         ::Color(0xFF,0xDA,0xB9)},
        {XML_peru,              ::Color(0xCD,0x85,0x3F)},    {XML_pink,              ::Color(0xFF,0xC0,0xCB)},
        {XML_plum,              ::Color(0xDD,0xA0,0xDD)},    {XML_powderBlue,        ::Color(0xB0,0xE0,0xE6)},
        {XML_purple,            ::Color(0x80,0x00,0x80)},    {XML_red,               ::Color(0xFF,0x00,0x00)},
        {XML_rosyBrown,         ::Color(0xBC,0x8F,0x8F)},    {XML_royalBlue,         ::Color(0x41,0x69,0xE1)},
        {XML_saddleBrown,       ::Color(0x8B,0x45,0x13)},    {XML_salmon,            ::Color(0xFA,0x80,0x72)},
        {XML_sandyBrown,        ::Color(0xF4,0xA4,0x60)},    {XML_seaGreen,          ::Color(0x2E,0x8B,0x57)},
        {XML_seaShell,          ::Color(0xFF,0xF5,0xEE)},    {XML_sienna,            ::Color(0xA0,0x52,0x2D)},
        {XML_silver,            ::Color(0xC0,0xC0,0xC0)},    {XML_skyBlue,           ::Color(0x87,0xCE,0xEB)},
        {XML_slateBlue,         ::Color(0x6A,0x5A,0xCD)},    {XML_slateGray,         ::Color(0x70,0x80,0x90)},
        {XML_snow,              ::Color(0xFF,0xFA,0xFA)},    {XML_springGreen,       ::Color(0x00,0xFF,0x7F)},
        {XML_steelBlue,         ::Color(0x46,0x82,0xB4)},    {XML_tan,               ::Color(0xD2,0xB4,0x8C)},
        {XML_teal,              ::Color(0x00,0x80,0x80)},    {XML_thistle,           ::Color(0xD8,0xBF,0xD8)},
        {XML_tomato,            ::Color(0xFF,0x63,0x47)},    {XML_turquoise,         ::Color(0x40,0xE0,0xD0)},
        {XML_violet,            ::Color(0xEE,0x82,0xEE)},    {XML_wheat,             ::Color(0xF5,0xDE,0xB3)},
        {XML_white,             ::Color(0xFF,0xFF,0xFF)},    {XML_whiteSmoke,        ::Color(0xF5,0xF5,0xF5)},
        {XML_yellow,            ::Color(0xFF,0xFF,0x00)},    {XML_yellowGreen,       ::Color(0x9A,0xCD,0x32)}
    };
    for(auto const& nEntry : spnDmlColors)
        maDmlColors[ static_cast< size_t >(nEntry.first) ] = nEntry.second;

    // predefined colors in VML (map XML token identifiers to RGB values)
    static const std::pair<sal_Int32, ::Color> spnVmlColors[] =
    {
        {XML_aqua,              ::Color(0x00,0xFF,0xFF)},    {XML_black,             ::Color(0x00,0x00,0x00)},
        {XML_blue,              ::Color(0x00,0x00,0xFF)},    {XML_fuchsia,           ::Color(0xFF,0x00,0xFF)},
        {XML_gray,              ::Color(0x80,0x80,0x80)},    {XML_green,             ::Color(0x00,0x80,0x00)},
        {XML_lime,              ::Color(0x00,0xFF,0x00)},    {XML_maroon,            ::Color(0x80,0x00,0x00)},
        {XML_navy,              ::Color(0x00,0x00,0x80)},    {XML_olive,             ::Color(0x80,0x80,0x00)},
        {XML_purple,            ::Color(0x80,0x00,0x80)},    {XML_red,               ::Color(0xFF,0x00,0x00)},
        {XML_silver,            ::Color(0xC0,0xC0,0xC0)},    {XML_teal,              ::Color(0x00,0x80,0x80)},
        {XML_white,             ::Color(0xFF,0xFF,0xFF)},    {XML_yellow,            ::Color(0xFF,0xFF,0x00)}
    };
    for(auto const& nEntry : spnVmlColors)
        maVmlColors[ static_cast< size_t >(nEntry.first) ] = nEntry.second;

    // predefined highlight colors in DML (map XML token identifiers to RGB values)
    static const std::pair<sal_Int32, ::Color> spnHighlightColors[] =
    {
        // tdf#131841 Predefined color for OOXML highlight.
        {XML_black,             ::Color(0x00,0x00,0x00)},    {XML_blue,              ::Color(0x00,0x00,0xFF)},
        {XML_cyan,              ::Color(0x00,0xFF,0xFF)},    {XML_darkBlue,          ::Color(0x00,0x00,0x8B)},
        {XML_darkCyan,          ::Color(0x00,0x8B,0x8B)},    {XML_darkGray,          ::Color(0xA9,0xA9,0xA9)},
        {XML_darkGreen,         ::Color(0x00,0x64,0x00)},    {XML_darkMagenta,       ::Color(0x80,0x00,0x80)},
        {XML_darkRed,           ::Color(0x8B,0x00,0x00)},    {XML_darkYellow,        ::Color(0x80,0x80,0x00)},
        {XML_green,             ::Color(0x00,0xFF,0x00)},    {XML_lightGray,         ::Color(0xD3,0xD3,0xD3)},
        {XML_magenta,           ::Color(0xFF,0x00,0xFF)},    {XML_red,               ::Color(0xFF,0x00,0x00)},
        {XML_white,             ::Color(0xFF,0xFF,0xFF)},    {XML_yellow,            ::Color(0xFF,0xFF,0x00)}
    };
    for (auto const& nEntry : spnHighlightColors)
        maHighlightColors[static_cast<size_t>(nEntry.first)] = nEntry.second;
}

struct StaticPresetColorsPool : public ::rtl::Static< PresetColorsPool, StaticPresetColorsPool > {};

const double DEC_GAMMA          = 2.3;
const double INC_GAMMA          = 1.0 / DEC_GAMMA;

void lclRgbToRgbComponents( sal_Int32& ornR, sal_Int32& ornG, sal_Int32& ornB, ::Color nRgb )
{
    ornR = nRgb.GetRed();
    ornG = nRgb.GetGreen();
    ornB = nRgb.GetBlue();
}

sal_Int32 lclRgbComponentsToRgb( sal_Int32 nR, sal_Int32 nG, sal_Int32 nB )
{
    return static_cast< sal_Int32 >( (nR << 16) | (nG << 8) | nB );
}

sal_Int32 lclRgbCompToCrgbComp( sal_Int32 nRgbComp )
{
    return static_cast< sal_Int32 >( nRgbComp * MAX_PERCENT / 255 );
}

sal_Int32 lclCrgbCompToRgbComp( sal_Int32 nCrgbComp )
{
    return static_cast< sal_Int32 >( nCrgbComp * 255 / MAX_PERCENT );
}

sal_Int32 lclGamma( sal_Int32 nComp, double fGamma )
{
    return static_cast< sal_Int32 >( pow( static_cast< double >( nComp ) / MAX_PERCENT, fGamma ) * MAX_PERCENT + 0.5 );
}

void lclSetValue( sal_Int32& ornValue, sal_Int32 nNew, sal_Int32 nMax = MAX_PERCENT )
{
    OSL_ENSURE( (0 <= nNew) && (nNew <= nMax), "lclSetValue - invalid value" );
    if( (0 <= nNew) && (nNew <= nMax) )
        ornValue = nNew;
}

void lclModValue( sal_Int32& ornValue, sal_Int32 nMod, sal_Int32 nMax = MAX_PERCENT )
{
    OSL_ENSURE( (0 <= nMod), "lclModValue - invalid modificator" );
    ornValue = getLimitedValue< sal_Int32, double >( static_cast< double >( ornValue ) * nMod / MAX_PERCENT, 0, nMax );
}

void lclOffValue( sal_Int32& ornValue, sal_Int32 nOff, sal_Int32 nMax = MAX_PERCENT )
{
    OSL_ENSURE( (-nMax <= nOff) && (nOff <= nMax), "lclOffValue - invalid offset" );
    ornValue = getLimitedValue< sal_Int32, sal_Int32 >( ornValue + nOff, 0, nMax );
}

} // namespace

Color::Color() :
    meMode( COLOR_UNUSED ),
    mnC1( 0 ),
    mnC2( 0 ),
    mnC3( 0 ),
    mnAlpha( MAX_PERCENT )
{
}

::Color Color::getDmlPresetColor( sal_Int32 nToken, ::Color nDefaultRgb )
{
    /*  Do not pass nDefaultRgb to ContainerHelper::getVectorElement(), to be
        able to catch the existing vector entries without corresponding XML
        token identifier. */
    ::Color nRgbValue = ContainerHelper::getVectorElement( StaticPresetColorsPool::get().maDmlColors, nToken, API_RGB_TRANSPARENT );
    return (nRgbValue.toUnoInt32() >= 0) ? nRgbValue : nDefaultRgb;
}

::Color Color::getVmlPresetColor( sal_Int32 nToken, ::Color nDefaultRgb )
{
    /*  Do not pass nDefaultRgb to ContainerHelper::getVectorElement(), to be
        able to catch the existing vector entries without corresponding XML
        token identifier. */
    ::Color nRgbValue = ContainerHelper::getVectorElement( StaticPresetColorsPool::get().maVmlColors, nToken, API_RGB_TRANSPARENT );
    return (nRgbValue.toUnoInt32() >= 0) ? nRgbValue : nDefaultRgb;
}

::Color Color::getHighlightColor(sal_Int32 nToken, ::Color nDefaultRgb)
{
    /*  Do not pass nDefaultRgb to ContainerHelper::getVectorElement(), to be
        able to catch the existing vector entries without corresponding XML
        token identifier. */
    ::Color nRgbValue = ContainerHelper::getVectorElement( StaticPresetColorsPool::get().maHighlightColors, nToken, API_RGB_TRANSPARENT );
    return (nRgbValue.toUnoInt32() >= 0) ? nRgbValue : nDefaultRgb;
}

void Color::setUnused()
{
    meMode = COLOR_UNUSED;
}

void Color::setSrgbClr( ::Color nRgb )
{
    setSrgbClr(nRgb.toUnoInt32());
}

void Color::setSrgbClr( sal_Int32 nRgb )
{
    OSL_ENSURE( (0 <= nRgb) && (nRgb <= 0xFFFFFF), "Color::setSrgbClr - invalid RGB value" );
    meMode = COLOR_RGB;
    lclRgbToRgbComponents( mnC1, mnC2, mnC3, ::Color(FromUno, nRgb) );
}

void Color::setScrgbClr( sal_Int32 nR, sal_Int32 nG, sal_Int32 nB )
{
    OSL_ENSURE( (0 <= nR) && (nR <= MAX_PERCENT), "Color::setScrgbClr - invalid red value" );
    OSL_ENSURE( (0 <= nG) && (nG <= MAX_PERCENT), "Color::setScrgbClr - invalid green value" );
    OSL_ENSURE( (0 <= nB) && (nB <= MAX_PERCENT), "Color::setScrgbClr - invalid blue value" );
    meMode = COLOR_CRGB;
    mnC1 = getLimitedValue< sal_Int32, sal_Int32 >( nR, 0, MAX_PERCENT );
    mnC2 = getLimitedValue< sal_Int32, sal_Int32 >( nG, 0, MAX_PERCENT );
    mnC3 = getLimitedValue< sal_Int32, sal_Int32 >( nB, 0, MAX_PERCENT );
}

void Color::setHslClr( sal_Int32 nHue, sal_Int32 nSat, sal_Int32 nLum )
{
    OSL_ENSURE( (0 <= nHue) && (nHue <= MAX_DEGREE), "Color::setHslClr - invalid hue value" );
    OSL_ENSURE( (0 <= nSat) && (nSat <= MAX_PERCENT), "Color::setHslClr - invalid saturation value" );
    OSL_ENSURE( (0 <= nLum) && (nLum <= MAX_PERCENT), "Color::setHslClr - invalid luminance value" );
    meMode = COLOR_HSL;
    mnC1 = getLimitedValue< sal_Int32, sal_Int32 >( nHue, 0, MAX_DEGREE );
    mnC2 = getLimitedValue< sal_Int32, sal_Int32 >( nSat, 0, MAX_PERCENT );
    mnC3 = getLimitedValue< sal_Int32, sal_Int32 >( nLum, 0, MAX_PERCENT );
}

void Color::setPrstClr( sal_Int32 nToken )
{
    ::Color nRgbValue = getDmlPresetColor( nToken, API_RGB_TRANSPARENT );
    OSL_ENSURE( nRgbValue.toUnoInt32() >= 0, "Color::setPrstClr - invalid preset color token" );
    if( nRgbValue.toUnoInt32() >= 0 )
        setSrgbClr( nRgbValue );
}

void Color::setHighlight(sal_Int32 nToken)
{
    ::Color nRgbValue = getHighlightColor(nToken, API_RGB_TRANSPARENT);
    OSL_ENSURE( nRgbValue.toUnoInt32() >= 0, "Color::setPrstClr - invalid preset color token" );
    if ( nRgbValue.toUnoInt32() >= 0 )
        setSrgbClr( nRgbValue );
}

void Color::setSchemeClr( sal_Int32 nToken )
{
    OSL_ENSURE( nToken != XML_TOKEN_INVALID, "Color::setSchemeClr - invalid color token" );
    meMode = (nToken == XML_phClr) ? COLOR_PH : COLOR_SCHEME;
    mnC1 = nToken;
}

void Color::setPaletteClr( sal_Int32 nPaletteIdx )
{
    OSL_ENSURE( nPaletteIdx >= 0, "Color::setPaletteClr - invalid palette index" );
    meMode = COLOR_PALETTE;
    mnC1 = nPaletteIdx;
}

void Color::setSysClr( sal_Int32 nToken, sal_Int32 nLastRgb )
{
    OSL_ENSURE( (-1 <= nLastRgb) && (nLastRgb <= 0xFFFFFF), "Color::setSysClr - invalid RGB value" );
    meMode = COLOR_SYSTEM;
    mnC1 = nToken;
    mnC2 = nLastRgb;
}

void Color::addTransformation( sal_Int32 nElement, sal_Int32 nValue )
{
    /*  Execute alpha transformations directly, store other transformations in
        a vector, they may depend on a scheme base color which will be resolved
        in Color::getColor(). */
    sal_Int32 nToken = getBaseToken( nElement );
    switch( nToken )
    {
        case XML_alpha:     lclSetValue( mnAlpha, nValue ); break;
        case XML_alphaMod:  lclModValue( mnAlpha, nValue ); break;
        case XML_alphaOff:  lclOffValue( mnAlpha, nValue ); break;
        default:            maTransforms.emplace_back( nToken, nValue );
    }
    sal_Int32 nSize = maInteropTransformations.getLength();
    maInteropTransformations.realloc(nSize + 1);
    maInteropTransformations[nSize].Name = getColorTransformationName( nToken );
    maInteropTransformations[nSize].Value <<= nValue;
}

void Color::addChartTintTransformation( double fTint )
{
    sal_Int32 nValue = getLimitedValue< sal_Int32, double >( fTint * MAX_PERCENT + 0.5, -MAX_PERCENT, MAX_PERCENT );
    if( nValue < 0 )
        maTransforms.emplace_back( XML_shade, nValue + MAX_PERCENT );
    else if( nValue > 0 )
        maTransforms.emplace_back( XML_tint, MAX_PERCENT - nValue );
}

void Color::addExcelTintTransformation( double fTint )
{
    sal_Int32 nValue = getLimitedValue< sal_Int32, double >( fTint * MAX_PERCENT + 0.5, -MAX_PERCENT, MAX_PERCENT );
    maTransforms.emplace_back( XLS_TOKEN( tint ), nValue );
}

void Color::clearTransformations()
{
    maTransforms.clear();
    maInteropTransformations.realloc(0);
    clearTransparence();
}

OUString Color::getColorTransformationName( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XML_red:       return "red";
        case XML_redMod:    return "redMod";
        case XML_redOff:    return "redOff";
        case XML_green:     return "green";
        case XML_greenMod:  return "greenMod";
        case XML_greenOff:  return "greenOff";
        case XML_blue:      return "blue";
        case XML_blueMod:   return "blueMod";
        case XML_blueOff:   return "blueOff";
        case XML_alpha:     return "alpha";
        case XML_alphaMod:  return "alphaMod";
        case XML_alphaOff:  return "alphaOff";
        case XML_hue:       return "hue";
        case XML_hueMod:    return "hueMod";
        case XML_hueOff:    return "hueOff";
        case XML_sat:       return "sat";
        case XML_satMod:    return "satMod";
        case XML_satOff:    return "satOff";
        case XML_lum:       return "lum";
        case XML_lumMod:    return "lumMod";
        case XML_lumOff:    return "lumOff";
        case XML_shade:     return "shade";
        case XML_tint:      return "tint";
        case XML_gray:      return "gray";
        case XML_comp:      return "comp";
        case XML_inv:       return "inv";
        case XML_gamma:     return "gamma";
        case XML_invGamma:  return "invGamma";
    }
    SAL_WARN( "oox.drawingml", "Color::getColorTransformationName - unexpected transformation type" );
    return OUString();
}

sal_Int32 Color::getColorTransformationToken( std::u16string_view sName )
{
    if( sName == u"red" )
        return XML_red;
    else if( sName == u"redMod" )
        return XML_redMod;
    else if( sName == u"redOff" )
        return XML_redOff;
    else if( sName == u"green" )
        return XML_green;
    else if( sName == u"greenMod" )
        return XML_greenMod;
    else if( sName == u"greenOff" )
        return XML_greenOff;
    else if( sName == u"blue" )
        return XML_blue;
    else if( sName == u"blueMod" )
        return XML_blueMod;
    else if( sName == u"blueOff" )
        return XML_blueOff;
    else if( sName == u"alpha" )
        return XML_alpha;
    else if( sName == u"alphaMod" )
        return XML_alphaMod;
    else if( sName == u"alphaOff" )
        return XML_alphaOff;
    else if( sName == u"hue" )
        return XML_hue;
    else if( sName == u"hueMod" )
        return XML_hueMod;
    else if( sName == u"hueOff" )
        return XML_hueOff;
    else if( sName == u"sat" )
        return XML_sat;
    else if( sName == u"satMod" )
        return XML_satMod;
    else if( sName == u"satOff" )
        return XML_satOff;
    else if( sName == u"lum" )
        return XML_lum;
    else if( sName == u"lumMod" )
        return XML_lumMod;
    else if( sName == u"lumOff" )
        return XML_lumOff;
    else if( sName == u"shade" )
        return XML_shade;
    else if( sName == u"tint" )
        return XML_tint;
    else if( sName == u"gray" )
        return XML_gray;
    else if( sName == u"comp" )
        return XML_comp;
    else if( sName == u"inv" )
        return XML_inv;
    else if( sName == u"gamma" )
        return XML_gamma;
    else if( sName == u"invGamma" )
        return XML_invGamma;

    SAL_WARN( "oox.drawingml", "Color::getColorTransformationToken - unexpected transformation type" );
    return XML_TOKEN_INVALID;
}

bool Color::equals(const Color& rOther, const GraphicHelper& rGraphicHelper, ::Color nPhClr) const
{
    if (getColor(rGraphicHelper, nPhClr) != rOther.getColor(rGraphicHelper, nPhClr))
        return false;

    return getTransparency() == rOther.getTransparency();
}

void Color::clearTransparence()
{
    mnAlpha = MAX_PERCENT;
}

::Color Color::getColor( const GraphicHelper& rGraphicHelper, ::Color nPhClr ) const
{
    const sal_Int32 nTempC1 = mnC1;
    const sal_Int32 nTempC2 = mnC2;
    const sal_Int32 nTempC3 = mnC3;
    const ColorMode eTempMode = meMode;

    switch( meMode )
    {
        case COLOR_UNUSED:  mnC1 = API_RGB_TRANSPARENT.toUnoInt32(); break;

        case COLOR_RGB:     break;  // nothing to do
        case COLOR_CRGB:    break;  // nothing to do
        case COLOR_HSL:     break;  // nothing to do

        case COLOR_SCHEME:  setResolvedRgb( rGraphicHelper.getSchemeColor( mnC1 ) );        break;
        case COLOR_PALETTE: setResolvedRgb( rGraphicHelper.getPaletteColor( mnC1 ) );       break;
        case COLOR_SYSTEM:  setResolvedRgb( rGraphicHelper.getSystemColor( mnC1, ::Color(FromUno, mnC2) ) );  break;
        case COLOR_PH:      setResolvedRgb( nPhClr );                                       break;

        case COLOR_FINAL:   return ::Color(FromUno, mnC1);
    }

    // if color is UNUSED or turns to UNUSED in setResolvedRgb, do not perform transformations
    if( meMode != COLOR_UNUSED )
    {
        for (auto const& transform : maTransforms)
        {
            switch( transform.mnToken )
            {
                case XML_red:       toCrgb(); lclSetValue( mnC1, transform.mnValue );    break;
                case XML_redMod:    toCrgb(); lclModValue( mnC1, transform.mnValue );    break;
                case XML_redOff:    toCrgb(); lclOffValue( mnC1, transform.mnValue );    break;
                case XML_green:     toCrgb(); lclSetValue( mnC2, transform.mnValue );    break;
                case XML_greenMod:  toCrgb(); lclModValue( mnC2, transform.mnValue );    break;
                case XML_greenOff:  toCrgb(); lclOffValue( mnC2, transform.mnValue );    break;
                case XML_blue:      toCrgb(); lclSetValue( mnC3, transform.mnValue );    break;
                case XML_blueMod:   toCrgb(); lclModValue( mnC3, transform.mnValue );    break;
                case XML_blueOff:   toCrgb(); lclOffValue( mnC3, transform.mnValue );    break;

                case XML_hue:       toHsl(); lclSetValue( mnC1, transform.mnValue, MAX_DEGREE ); break;
                case XML_hueMod:    toHsl(); lclModValue( mnC1, transform.mnValue, MAX_DEGREE ); break;
                case XML_hueOff:    toHsl(); lclOffValue( mnC1, transform.mnValue, MAX_DEGREE ); break;
                case XML_sat:       toHsl(); lclSetValue( mnC2, transform.mnValue );             break;
                case XML_satMod:    toHsl(); lclModValue( mnC2, transform.mnValue );             break;
                case XML_satOff:    toHsl(); lclOffValue( mnC2, transform.mnValue );             break;

                case XML_lum:
                    toHsl();
                    lclSetValue( mnC3, transform.mnValue );
                    // if color changes to black or white, it will stay gray if luminance changes again
                    if( (mnC3 == 0) || (mnC3 == MAX_PERCENT) ) mnC2 = 0;
                break;
                case XML_lumMod:
                    toHsl();
                    lclModValue( mnC3, transform.mnValue );
                    // if color changes to black or white, it will stay gray if luminance changes again
                    if( (mnC3 == 0) || (mnC3 == MAX_PERCENT) ) mnC2 = 0;
                break;
                case XML_lumOff:
                    toHsl();
                    lclOffValue( mnC3, transform.mnValue );
                    // if color changes to black or white, it will stay gray if luminance changes again
                    if( (mnC3 == 0) || (mnC3 == MAX_PERCENT) ) mnC2 = 0;
                break;

                case XML_shade:
                    // shade: 0% = black, 100% = original color
                    toCrgb();
                    OSL_ENSURE( (0 <= transform.mnValue) && (transform.mnValue <= MAX_PERCENT), "Color::getColor - invalid shade value" );
                    if( (0 <= transform.mnValue) && (transform.mnValue <= MAX_PERCENT) )
                    {
                        double fFactor = static_cast< double >( transform.mnValue ) / MAX_PERCENT;
                        mnC1 = static_cast< sal_Int32 >( mnC1 * fFactor );
                        mnC2 = static_cast< sal_Int32 >( mnC2 * fFactor );
                        mnC3 = static_cast< sal_Int32 >( mnC3 * fFactor );
                    }
                break;
                case XML_tint:
                    // tint: 0% = white, 100% = original color
                    toCrgb();
                    OSL_ENSURE( (0 <= transform.mnValue) && (transform.mnValue <= MAX_PERCENT), "Color::getColor - invalid tint value" );
                    if( (0 <= transform.mnValue) && (transform.mnValue <= MAX_PERCENT) )
                    {
                        double fFactor = static_cast< double >( transform.mnValue ) / MAX_PERCENT;
                        mnC1 = static_cast< sal_Int32 >( MAX_PERCENT - (MAX_PERCENT - mnC1) * fFactor );
                        mnC2 = static_cast< sal_Int32 >( MAX_PERCENT - (MAX_PERCENT - mnC2) * fFactor );
                        mnC3 = static_cast< sal_Int32 >( MAX_PERCENT - (MAX_PERCENT - mnC3) * fFactor );
                    }
                break;
                case XLS_TOKEN( tint ):
                    // Excel tint: move luminance relative to current value
                    toHsl();
                    OSL_ENSURE( (-MAX_PERCENT <= transform.mnValue) && (transform.mnValue <= MAX_PERCENT), "Color::getColor - invalid tint value" );
                    if( (-MAX_PERCENT <= transform.mnValue) && (transform.mnValue < 0) )
                    {
                        // negative: luminance towards 0% (black)
                        lclModValue( mnC3, transform.mnValue + MAX_PERCENT );
                    }
                    else if( (0 < transform.mnValue) && (transform.mnValue <= MAX_PERCENT) )
                    {
                        // positive: luminance towards 100% (white)
                        mnC3 = MAX_PERCENT - mnC3;
                        lclModValue( mnC3, MAX_PERCENT - transform.mnValue );
                        mnC3 = MAX_PERCENT - mnC3;
                    }
                break;

                case XML_gray:
                    // change color to gray, weighted RGB: 22% red, 72% green, 6% blue
                    toRgb();
                    mnC1 = mnC2 = mnC3 = (mnC1 * 22 + mnC2 * 72 + mnC3 * 6) / 100;
                break;

                case XML_comp:
                    // comp: rotate hue by 180 degrees, do not change lum/sat
                    toHsl();
                    mnC1 = (mnC1 + (180 * PER_DEGREE)) % MAX_DEGREE;
                break;
                case XML_inv:
                    // invert percentual RGB values
                    toCrgb();
                    mnC1 = MAX_PERCENT - mnC1;
                    mnC2 = MAX_PERCENT - mnC2;
                    mnC3 = MAX_PERCENT - mnC3;
                break;

                case XML_gamma:
                    // increase gamma of color
                    toCrgb();
                    mnC1 = lclGamma( mnC1, INC_GAMMA );
                    mnC2 = lclGamma( mnC2, INC_GAMMA );
                    mnC3 = lclGamma( mnC3, INC_GAMMA );
                break;
                case XML_invGamma:
                    // decrease gamma of color
                    toCrgb();
                    mnC1 = lclGamma( mnC1, DEC_GAMMA );
                    mnC2 = lclGamma( mnC2, DEC_GAMMA );
                    mnC3 = lclGamma( mnC3, DEC_GAMMA );
                break;
            }
        }

        // store resulting RGB value in mnC1
        toRgb();
        mnC1 = lclRgbComponentsToRgb( mnC1, mnC2, mnC3 );
    }
    else // if( meMode != COLOR_UNUSED )
    {
        mnC1 = API_RGB_TRANSPARENT.toUnoInt32();
    }

    sal_Int32 nRet = mnC1;
    // Restore the original values when the color depends on one of the input
    // parameters (rGraphicHelper or nPhClr)
    if( eTempMode >= COLOR_SCHEME && eTempMode <= COLOR_PH )
    {
        mnC1 = nTempC1;
        mnC2 = nTempC2;
        mnC3 = nTempC3;
        meMode = eTempMode;
    }
    else
    {
        meMode = COLOR_FINAL;
    }
    if( meMode == COLOR_FINAL )
        maTransforms.clear();
    return ::Color(FromUno, nRet);
}

bool Color::hasTransparency() const
{
    return mnAlpha < MAX_PERCENT;
}

sal_Int16 Color::getTransparency() const
{
    return sal_Int16(std::round( (1.0 * (MAX_PERCENT - mnAlpha)) / PER_PERCENT) );
}

// private --------------------------------------------------------------------

void Color::setResolvedRgb( ::Color nRgb ) const
{
    meMode = (nRgb.toUnoInt32() < 0) ? COLOR_UNUSED : COLOR_RGB;
    lclRgbToRgbComponents( mnC1, mnC2, mnC3, nRgb );
}

void Color::toRgb() const
{
    switch( meMode )
    {
        case COLOR_RGB:
            // nothing to do
        break;
        case COLOR_CRGB:
            meMode = COLOR_RGB;
            mnC1 = lclCrgbCompToRgbComp( lclGamma( mnC1, INC_GAMMA ) );
            mnC2 = lclCrgbCompToRgbComp( lclGamma( mnC2, INC_GAMMA ) );
            mnC3 = lclCrgbCompToRgbComp( lclGamma( mnC3, INC_GAMMA ) );
        break;
        case COLOR_HSL:
        {
            meMode = COLOR_RGB;
            double fR = 0.0, fG = 0.0, fB = 0.0;
            if( (mnC2 == 0) || (mnC3 == MAX_PERCENT) )
            {
                fR = fG = fB = static_cast< double >( mnC3 ) / MAX_PERCENT;
            }
            else if( mnC3 > 0 )
            {
                // base color from hue
                double fHue = static_cast< double >( mnC1 ) / MAX_DEGREE * 6.0; // interval [0.0, 6.0)
                if( fHue <= 1.0 )       { fR = 1.0; fG = fHue; }        // red...yellow
                else if( fHue <= 2.0 )  { fR = 2.0 - fHue; fG = 1.0; }  // yellow...green
                else if( fHue <= 3.0 )  { fG = 1.0; fB = fHue - 2.0; }  // green...cyan
                else if( fHue <= 4.0 )  { fG = 4.0 - fHue; fB = 1.0; }  // cyan...blue
                else if( fHue <= 5.0 )  { fR = fHue - 4.0; fB = 1.0; }  // blue...magenta
                else                    { fR = 1.0; fB = 6.0 - fHue; }  // magenta...red

                // apply saturation
                double fSat = static_cast< double >( mnC2 ) / MAX_PERCENT;
                fR = (fR - 0.5) * fSat + 0.5;
                fG = (fG - 0.5) * fSat + 0.5;
                fB = (fB - 0.5) * fSat + 0.5;

                // apply luminance
                double fLum = 2.0 * static_cast< double >( mnC3 ) / MAX_PERCENT - 1.0;  // interval [-1.0, 1.0]
                if( fLum < 0.0 )
                {
                    double fShade = fLum + 1.0; // interval [0.0, 1.0] (black...full color)
                    fR *= fShade;
                    fG *= fShade;
                    fB *= fShade;
                }
                else if( fLum > 0.0 )
                {
                    double fTint = 1.0 - fLum;  // interval [0.0, 1.0] (white...full color)
                    fR = 1.0 - ((1.0 - fR) * fTint);
                    fG = 1.0 - ((1.0 - fG) * fTint);
                    fB = 1.0 - ((1.0 - fB) * fTint);
                }
            }
            mnC1 = static_cast< sal_Int32 >( fR * 255.0 + 0.5 );
            mnC2 = static_cast< sal_Int32 >( fG * 255.0 + 0.5 );
            mnC3 = static_cast< sal_Int32 >( fB * 255.0 + 0.5 );
        }
        break;
        default:
            OSL_FAIL( "Color::toRgb - unexpected color mode" );
    }
}

void Color::toCrgb() const
{
    switch( meMode )
    {
        case COLOR_HSL:
            toRgb();
            [[fallthrough]];
        case COLOR_RGB:
            meMode = COLOR_CRGB;
            mnC1 = lclGamma( lclRgbCompToCrgbComp( mnC1 ), DEC_GAMMA );
            mnC2 = lclGamma( lclRgbCompToCrgbComp( mnC2 ), DEC_GAMMA );
            mnC3 = lclGamma( lclRgbCompToCrgbComp( mnC3 ), DEC_GAMMA );
        break;
        case COLOR_CRGB:
            // nothing to do
        break;
        default:
            OSL_FAIL( "Color::toCrgb - unexpected color mode" );
    }
}

void Color::toHsl() const
{
    switch( meMode )
    {
        case COLOR_CRGB:
            toRgb();
            [[fallthrough]];
        case COLOR_RGB:
        {
            meMode = COLOR_HSL;
            double fR = static_cast< double >( mnC1 ) / 255.0;  // red [0.0, 1.0]
            double fG = static_cast< double >( mnC2 ) / 255.0;  // green [0.0, 1.0]
            double fB = static_cast< double >( mnC3 ) / 255.0;  // blue [0.0, 1.0]
            double fMin = ::std::min( ::std::min( fR, fG ), fB );
            double fMax = ::std::max( ::std::max( fR, fG ), fB );
            double fD = fMax - fMin;

            using ::rtl::math::approxEqual;

            // hue: 0deg = red, 120deg = green, 240deg = blue
            if( fD == 0.0 )         // black/gray/white
                mnC1 = 0;
            else if( approxEqual(fMax, fR, 64) )   // magenta...red...yellow
                mnC1 = static_cast< sal_Int32 >( ((fG - fB) / fD * 60.0 + 360.0) * PER_DEGREE + 0.5 ) % MAX_DEGREE;
            else if( approxEqual(fMax, fG, 64) )   // yellow...green...cyan
                mnC1 = static_cast< sal_Int32 >( ((fB - fR) / fD * 60.0 + 120.0) * PER_DEGREE + 0.5 );
            else                    // cyan...blue...magenta
                mnC1 = static_cast< sal_Int32 >( ((fR - fG) / fD * 60.0 + 240.0) * PER_DEGREE + 0.5 );

            // luminance: 0% = black, 50% = full color, 100% = white
            mnC3 = static_cast< sal_Int32 >( (fMin + fMax) / 2.0 * MAX_PERCENT + 0.5 );

            // saturation: 0% = gray, 100% = full color
            if( (mnC3 == 0) || (mnC3 == MAX_PERCENT) )  // black/white
                mnC2 = 0;
            else if( mnC3 <= 50 * PER_PERCENT )         // dark...full color
                mnC2 = static_cast< sal_Int32 >( fD / (fMin + fMax) * MAX_PERCENT + 0.5 );
            else                                        // full color...light
                mnC2 = static_cast< sal_Int32 >( fD / (2.0 - fMax - fMin) * MAX_PERCENT + 0.5 );
        }
        break;
        case COLOR_HSL:
            // nothing to do
        break;
        default:
            OSL_FAIL( "Color::toHsl - unexpected color mode" );
    }
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
