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

#include "oox/drawingml/color.hxx"
#include <algorithm>
#include <math.h>
#include <osl/diagnose.h>
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/token/namespaces.hxx"
#include "oox/token/tokens.hxx"

namespace oox {
namespace drawingml {

namespace {

/** Global storage for predefined color values used in OOXML file formats. */
struct PresetColorsPool
{
    typedef ::std::vector< sal_Int32 > ColorVector;

    ColorVector         maDmlColors;        /// Predefined colors in DrawingML, indexed by XML token.
    ColorVector         maVmlColors;        /// Predefined colors in VML, indexed by XML token.

    explicit            PresetColorsPool();
};

PresetColorsPool::PresetColorsPool() :
    maDmlColors( static_cast< size_t >( XML_TOKEN_COUNT ), API_RGB_TRANSPARENT ),
    maVmlColors( static_cast< size_t >( XML_TOKEN_COUNT ), API_RGB_TRANSPARENT )
{
    // predefined colors in DrawingML (map XML token identifiers to RGB values)
    static const sal_Int32 spnDmlColors[] =
    {
        XML_aliceBlue,         0xF0F8FF,    XML_antiqueWhite,      0xFAEBD7,
        XML_aqua,              0x00FFFF,    XML_aquamarine,        0x7FFFD4,
        XML_azure,             0xF0FFFF,    XML_beige,             0xF5F5DC,
        XML_bisque,            0xFFE4C4,    XML_black,             0x000000,
        XML_blanchedAlmond,    0xFFEBCD,    XML_blue,              0x0000FF,
        XML_blueViolet,        0x8A2BE2,    XML_brown,             0xA52A2A,
        XML_burlyWood,         0xDEB887,    XML_cadetBlue,         0x5F9EA0,
        XML_chartreuse,        0x7FFF00,    XML_chocolate,         0xD2691E,
        XML_coral,             0xFF7F50,    XML_cornflowerBlue,    0x6495ED,
        XML_cornsilk,          0xFFF8DC,    XML_crimson,           0xDC143C,
        XML_cyan,              0x00FFFF,    XML_deepPink,          0xFF1493,
        XML_deepSkyBlue,       0x00BFFF,    XML_dimGray,           0x696969,
        XML_dkBlue,            0x00008B,    XML_dkCyan,            0x008B8B,
        XML_dkGoldenrod,       0xB8860B,    XML_dkGray,            0xA9A9A9,
        XML_dkGreen,           0x006400,    XML_dkKhaki,           0xBDB76B,
        XML_dkMagenta,         0x8B008B,    XML_dkOliveGreen,      0x556B2F,
        XML_dkOrange,          0xFF8C00,    XML_dkOrchid,          0x9932CC,
        XML_dkRed,             0x8B0000,    XML_dkSalmon,          0xE9967A,
        XML_dkSeaGreen,        0x8FBC8B,    XML_dkSlateBlue,       0x483D8B,
        XML_dkSlateGray,       0x2F4F4F,    XML_dkTurquoise,       0x00CED1,
        XML_dkViolet,          0x9400D3,    XML_dodgerBlue,        0x1E90FF,
        XML_firebrick,         0xB22222,    XML_floralWhite,       0xFFFAF0,
        XML_forestGreen,       0x228B22,    XML_fuchsia,           0xFF00FF,
        XML_gainsboro,         0xDCDCDC,    XML_ghostWhite,        0xF8F8FF,
        XML_gold,              0xFFD700,    XML_goldenrod,         0xDAA520,
        XML_gray,              0x808080,    XML_green,             0x008000,
        XML_greenYellow,       0xADFF2F,    XML_honeydew,          0xF0FFF0,
        XML_hotPink,           0xFF69B4,    XML_indianRed,         0xCD5C5C,
        XML_indigo,            0x4B0082,    XML_ivory,             0xFFFFF0,
        XML_khaki,             0xF0E68C,    XML_lavender,          0xE6E6FA,
        XML_lavenderBlush,     0xFFF0F5,    XML_lawnGreen,         0x7CFC00,
        XML_lemonChiffon,      0xFFFACD,    XML_lime,              0x00FF00,
        XML_limeGreen,         0x32CD32,    XML_linen,             0xFAF0E6,
        XML_ltBlue,            0xADD8E6,    XML_ltCoral,           0xF08080,
        XML_ltCyan,            0xE0FFFF,    XML_ltGoldenrodYellow, 0xFAFA78,
        XML_ltGray,            0xD3D3D3,    XML_ltGreen,           0x90EE90,
        XML_ltPink,            0xFFB6C1,    XML_ltSalmon,          0xFFA07A,
        XML_ltSeaGreen,        0x20B2AA,    XML_ltSkyBlue,         0x87CEFA,
        XML_ltSlateGray,       0x778899,    XML_ltSteelBlue,       0xB0C4DE,
        XML_ltYellow,          0xFFFFE0,    XML_magenta,           0xFF00FF,
        XML_maroon,            0x800000,    XML_medAquamarine,     0x66CDAA,
        XML_medBlue,           0x0000CD,    XML_medOrchid,         0xBA55D3,
        XML_medPurple,         0x9370DB,    XML_medSeaGreen,       0x3CB371,
        XML_medSlateBlue,      0x7B68EE,    XML_medSpringGreen,    0x00FA9A,
        XML_medTurquoise,      0x48D1CC,    XML_medVioletRed,      0xC71585,
        XML_midnightBlue,      0x191970,    XML_mintCream,         0xF5FFFA,
        XML_mistyRose,         0xFFE4E1,    XML_moccasin,          0xFFE4B5,
        XML_navajoWhite,       0xFFDEAD,    XML_navy,              0x000080,
        XML_oldLace,           0xFDF5E6,    XML_olive,             0x808000,
        XML_oliveDrab,         0x6B8E23,    XML_orange,            0xFFA500,
        XML_orangeRed,         0xFF4500,    XML_orchid,            0xDA70D6,
        XML_paleGoldenrod,     0xEEE8AA,    XML_paleGreen,         0x98FB98,
        XML_paleTurquoise,     0xAFEEEE,    XML_paleVioletRed,     0xDB7093,
        XML_papayaWhip,        0xFFEFD5,    XML_peachPuff,         0xFFDAB9,
        XML_peru,              0xCD853F,    XML_pink,              0xFFC0CB,
        XML_plum,              0xDDA0DD,    XML_powderBlue,        0xB0E0E6,
        XML_purple,            0x800080,    XML_red,               0xFF0000,
        XML_rosyBrown,         0xBC8F8F,    XML_royalBlue,         0x4169E1,
        XML_saddleBrown,       0x8B4513,    XML_salmon,            0xFA8072,
        XML_sandyBrown,        0xF4A460,    XML_seaGreen,          0x2E8B57,
        XML_seaShell,          0xFFF5EE,    XML_sienna,            0xA0522D,
        XML_silver,            0xC0C0C0,    XML_skyBlue,           0x87CEEB,
        XML_slateBlue,         0x6A5ACD,    XML_slateGray,         0x708090,
        XML_snow,              0xFFFAFA,    XML_springGreen,       0x00FF7F,
        XML_steelBlue,         0x4682B4,    XML_tan,               0xD2B48C,
        XML_teal,              0x008080,    XML_thistle,           0xD8BFD8,
        XML_tomato,            0xFF6347,    XML_turquoise,         0x40E0D0,
        XML_violet,            0xEE82EE,    XML_wheat,             0xF5DEB3,
        XML_white,             0xFFFFFF,    XML_whiteSmoke,        0xF5F5F5,
        XML_yellow,            0xFFFF00,    XML_yellowGreen,       0x9ACD32
    };
    for( const sal_Int32* pnEntry = spnDmlColors; pnEntry < STATIC_ARRAY_END( spnDmlColors ); pnEntry += 2 )
        maDmlColors[ static_cast< size_t >( pnEntry[ 0 ] ) ] = pnEntry[ 1 ];

    // predefined colors in VML (map XML token identifiers to RGB values)
    static const sal_Int32 spnVmlColors[] =
    {
        XML_aqua,              0x00FFFF,    XML_black,             0x000000,
        XML_blue,              0x0000FF,    XML_fuchsia,           0xFF00FF,
        XML_gray,              0x808080,    XML_green,             0x008000,
        XML_lime,              0x00FF00,    XML_maroon,            0x800000,
        XML_navy,              0x000080,    XML_olive,             0x808000,
        XML_purple,            0x800080,    XML_red,               0xFF0000,
        XML_silver,            0xC0C0C0,    XML_teal,              0x008080,
        XML_white,             0xFFFFFF,    XML_yellow,            0xFFFF00
    };
    for( const sal_Int32* pnEntry = spnVmlColors; pnEntry < STATIC_ARRAY_END( spnVmlColors ); pnEntry += 2 )
        maVmlColors[ static_cast< size_t >( pnEntry[ 0 ] ) ] = pnEntry[ 1 ];
}

struct StaticPresetColorsPool : public ::rtl::Static< PresetColorsPool, StaticPresetColorsPool > {};

const double DEC_GAMMA          = 2.3;
const double INC_GAMMA          = 1.0 / DEC_GAMMA;

inline void lclRgbToRgbComponents( sal_Int32& ornR, sal_Int32& ornG, sal_Int32& ornB, sal_Int32 nRgb )
{
    ornR = (nRgb >> 16) & 0xFF;
    ornG = (nRgb >> 8) & 0xFF;
    ornB = nRgb & 0xFF;
}

inline sal_Int32 lclRgbComponentsToRgb( sal_Int32 nR, sal_Int32 nG, sal_Int32 nB )
{
    return static_cast< sal_Int32 >( (nR << 16) | (nG << 8) | nB );
}

inline sal_Int32 lclRgbCompToCrgbComp( sal_Int32 nRgbComp )
{
    return static_cast< sal_Int32 >( nRgbComp * MAX_PERCENT / 255 );
}

inline sal_Int32 lclCrgbCompToRgbComp( sal_Int32 nCrgbComp )
{
    return static_cast< sal_Int32 >( nCrgbComp * 255 / MAX_PERCENT );
}

inline sal_Int32 lclGamma( sal_Int32 nComp, double fGamma )
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

Color::~Color()
{
}

sal_Int32 Color::getDmlPresetColor( sal_Int32 nToken, sal_Int32 nDefaultRgb )
{
    /*  Do not pass nDefaultRgb to ContainerHelper::getVectorElement(), to be
        able to catch the existing vector entries without corresponding XML
        token identifier. */
    sal_Int32 nRgbValue = ContainerHelper::getVectorElement( StaticPresetColorsPool::get().maDmlColors, nToken, API_RGB_TRANSPARENT );
    return (nRgbValue >= 0) ? nRgbValue : nDefaultRgb;
}

sal_Int32 Color::getVmlPresetColor( sal_Int32 nToken, sal_Int32 nDefaultRgb )
{
    /*  Do not pass nDefaultRgb to ContainerHelper::getVectorElement(), to be
        able to catch the existing vector entries without corresponding XML
        token identifier. */
    sal_Int32 nRgbValue = ContainerHelper::getVectorElement( StaticPresetColorsPool::get().maVmlColors, nToken, API_RGB_TRANSPARENT );
    return (nRgbValue >= 0) ? nRgbValue : nDefaultRgb;
}

void Color::setUnused()
{
    meMode = COLOR_UNUSED;
}

void Color::setSrgbClr( sal_Int32 nRgb )
{
    OSL_ENSURE( (0 <= nRgb) && (nRgb <= 0xFFFFFF), "Color::setSrgbClr - invalid RGB value" );
    meMode = COLOR_RGB;
    lclRgbToRgbComponents( mnC1, mnC2, mnC3, nRgb );
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
    sal_Int32 nRgbValue = getDmlPresetColor( nToken, API_RGB_TRANSPARENT );
    OSL_ENSURE( nRgbValue >= 0, "Color::setPrstClr - invalid preset color token" );
    if( nRgbValue >= 0 )
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
        default:            maTransforms.push_back( Transformation( nToken, nValue ) );
    }
    sal_Int32 nSize = maInteropTransformations.getLength();
    maInteropTransformations.realloc(nSize + 1);
    maInteropTransformations[nSize].Name = getColorTransformationName( nToken );
    maInteropTransformations[nSize].Value = css::uno::Any( nValue );
}

void Color::addChartTintTransformation( double fTint )
{
    sal_Int32 nValue = getLimitedValue< sal_Int32, double >( fTint * MAX_PERCENT + 0.5, -MAX_PERCENT, MAX_PERCENT );
    if( nValue < 0 )
        maTransforms.push_back( Transformation( XML_shade, nValue + MAX_PERCENT ) );
    else if( nValue > 0 )
        maTransforms.push_back( Transformation( XML_tint, MAX_PERCENT - nValue ) );
}

void Color::addExcelTintTransformation( double fTint )
{
    sal_Int32 nValue = getLimitedValue< sal_Int32, double >( fTint * MAX_PERCENT + 0.5, -MAX_PERCENT, MAX_PERCENT );
    maTransforms.push_back( Transformation( XLS_TOKEN( tint ), nValue ) );
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
        case XML_red:       return OUString( "red" );
        case XML_redMod:    return OUString( "redMod" );
        case XML_redOff:    return OUString( "redOff" );
        case XML_green:     return OUString( "green" );
        case XML_greenMod:  return OUString( "greenMod" );
        case XML_greenOff:  return OUString( "greenOff" );
        case XML_blue:      return OUString( "blue" );
        case XML_blueMod:   return OUString( "blueMod" );
        case XML_blueOff:   return OUString( "blueOff" );
        case XML_alpha:     return OUString( "alpha" );
        case XML_alphaMod:  return OUString( "alphaMod" );
        case XML_alphaOff:  return OUString( "alphaOff" );
        case XML_hue:       return OUString( "hue" );
        case XML_hueMod:    return OUString( "hueMod" );
        case XML_hueOff:    return OUString( "hueOff" );
        case XML_sat:       return OUString( "sat" );
        case XML_satMod:    return OUString( "satMod" );
        case XML_satOff:    return OUString( "satOff" );
        case XML_lum:       return OUString( "lum" );
        case XML_lumMod:    return OUString( "lumMod" );
        case XML_lumOff:    return OUString( "lumOff" );
        case XML_shade:     return OUString( "shade" );
        case XML_tint:      return OUString( "tint" );
        case XML_gray:      return OUString( "gray" );
        case XML_comp:      return OUString( "comp" );
        case XML_inv:       return OUString( "inv" );
        case XML_gamma:     return OUString( "gamma" );
        case XML_invGamma:  return OUString( "invGamma" );
    }
    SAL_WARN( "oox.drawingml", "Color::getColorTransformationName - unexpected transformation type" );
    return OUString();
}

sal_Int32 Color::getColorTransformationToken( const OUString& sName )
{
    if( sName == "red" )
        return XML_red;
    else if( sName == "redMod" )
        return XML_redMod;
    else if( sName == "redOff" )
        return XML_redOff;
    else if( sName == "green" )
        return XML_green;
    else if( sName == "greenMod" )
        return XML_greenMod;
    else if( sName == "greenOff" )
        return XML_greenOff;
    else if( sName == "blue" )
        return XML_blue;
    else if( sName == "blueMod" )
        return XML_blueMod;
    else if( sName == "blueOff" )
        return XML_blueOff;
    else if( sName == "alpha" )
        return XML_alpha;
    else if( sName == "alphaMod" )
        return XML_alphaMod;
    else if( sName == "alphaOff" )
        return XML_alphaOff;
    else if( sName == "hue" )
        return XML_hue;
    else if( sName == "hueMod" )
        return XML_hueMod;
    else if( sName == "hueOff" )
        return XML_hueOff;
    else if( sName == "sat" )
        return XML_sat;
    else if( sName == "satMod" )
        return XML_satMod;
    else if( sName == "satOff" )
        return XML_satOff;
    else if( sName == "lum" )
        return XML_lum;
    else if( sName == "lumMod" )
        return XML_lumMod;
    else if( sName == "lumOff" )
        return XML_lumOff;
    else if( sName == "shade" )
        return XML_shade;
    else if( sName == "tint" )
        return XML_tint;
    else if( sName == "gray" )
        return XML_gray;
    else if( sName == "comp" )
        return XML_comp;
    else if( sName == "inv" )
        return XML_inv;
    else if( sName == "gamma" )
        return XML_gamma;
    else if( sName == "invGamma" )
        return XML_invGamma;

    SAL_WARN( "oox.drawingml", "Color::getColorTransformationToken - unexpected transformation type" );
    return XML_TOKEN_INVALID;
}

void Color::clearTransparence()
{
    mnAlpha = MAX_PERCENT;
}

sal_Int32 Color::getColor( const GraphicHelper& rGraphicHelper, sal_Int32 nPhClr ) const
{
    const sal_Int32 nTempC1 = mnC1;
    const sal_Int32 nTempC2 = mnC2;
    const sal_Int32 nTempC3 = mnC3;
    const ColorMode eTempMode = meMode;

    switch( meMode )
    {
        case COLOR_UNUSED:  mnC1 = API_RGB_TRANSPARENT; break;

        case COLOR_RGB:     break;  // nothing to do
        case COLOR_CRGB:    break;  // nothing to do
        case COLOR_HSL:     break;  // nothing to do

        case COLOR_SCHEME:  setResolvedRgb( rGraphicHelper.getSchemeColor( mnC1 ) );        break;
        case COLOR_PALETTE: setResolvedRgb( rGraphicHelper.getPaletteColor( mnC1 ) );       break;
        case COLOR_SYSTEM:  setResolvedRgb( rGraphicHelper.getSystemColor( mnC1, mnC2 ) );  break;
        case COLOR_PH:      setResolvedRgb( nPhClr );                                       break;

        case COLOR_FINAL:   return mnC1;
    }

    // if color is UNUSED or turns to UNUSED in setResolvedRgb, do not perform transformations
    if( meMode != COLOR_UNUSED )
    {
        for( TransformVec::const_iterator aIt = maTransforms.begin(), aEnd = maTransforms.end(); aIt != aEnd; ++aIt )
        {
            switch( aIt->mnToken )
            {
                case XML_red:       toCrgb(); lclSetValue( mnC1, aIt->mnValue );    break;
                case XML_redMod:    toCrgb(); lclModValue( mnC1, aIt->mnValue );    break;
                case XML_redOff:    toCrgb(); lclOffValue( mnC1, aIt->mnValue );    break;
                case XML_green:     toCrgb(); lclSetValue( mnC2, aIt->mnValue );    break;
                case XML_greenMod:  toCrgb(); lclModValue( mnC2, aIt->mnValue );    break;
                case XML_greenOff:  toCrgb(); lclOffValue( mnC2, aIt->mnValue );    break;
                case XML_blue:      toCrgb(); lclSetValue( mnC3, aIt->mnValue );    break;
                case XML_blueMod:   toCrgb(); lclModValue( mnC3, aIt->mnValue );    break;
                case XML_blueOff:   toCrgb(); lclOffValue( mnC3, aIt->mnValue );    break;

                case XML_hue:       toHsl(); lclSetValue( mnC1, aIt->mnValue, MAX_DEGREE ); break;
                case XML_hueMod:    toHsl(); lclModValue( mnC1, aIt->mnValue, MAX_DEGREE ); break;
                case XML_hueOff:    toHsl(); lclOffValue( mnC1, aIt->mnValue, MAX_DEGREE ); break;
                case XML_sat:       toHsl(); lclSetValue( mnC2, aIt->mnValue );             break;
                case XML_satMod:    toHsl(); lclModValue( mnC2, aIt->mnValue );             break;
                case XML_satOff:    toHsl(); lclOffValue( mnC2, aIt->mnValue );             break;

                case XML_lum:
                    toHsl();
                    lclSetValue( mnC3, aIt->mnValue );
                    // if color changes to black or white, it will stay gray if luminance changes again
                    if( (mnC3 == 0) || (mnC3 == MAX_PERCENT) ) mnC2 = 0;
                break;
                case XML_lumMod:
                    toHsl();
                    lclModValue( mnC3, aIt->mnValue );
                    // if color changes to black or white, it will stay gray if luminance changes again
                    if( (mnC3 == 0) || (mnC3 == MAX_PERCENT) ) mnC2 = 0;
                break;
                case XML_lumOff:
                    toHsl();
                    lclOffValue( mnC3, aIt->mnValue );
                    // if color changes to black or white, it will stay gray if luminance changes again
                    if( (mnC3 == 0) || (mnC3 == MAX_PERCENT) ) mnC2 = 0;
                break;

                case XML_shade:
                    // shade: 0% = black, 100% = original color
                    toCrgb();
                    OSL_ENSURE( (0 <= aIt->mnValue) && (aIt->mnValue <= MAX_PERCENT), "Color::getColor - invalid shade value" );
                    if( (0 <= aIt->mnValue) && (aIt->mnValue <= MAX_PERCENT) )
                    {
                        double fFactor = static_cast< double >( aIt->mnValue ) / MAX_PERCENT;
                        mnC1 = static_cast< sal_Int32 >( mnC1 * fFactor );
                        mnC2 = static_cast< sal_Int32 >( mnC2 * fFactor );
                        mnC3 = static_cast< sal_Int32 >( mnC3 * fFactor );
                    }
                break;
                case XML_tint:
                    // tint: 0% = white, 100% = original color
                    toCrgb();
                    OSL_ENSURE( (0 <= aIt->mnValue) && (aIt->mnValue <= MAX_PERCENT), "Color::getColor - invalid tint value" );
                    if( (0 <= aIt->mnValue) && (aIt->mnValue <= MAX_PERCENT) )
                    {
                        double fFactor = static_cast< double >( aIt->mnValue ) / MAX_PERCENT;
                        mnC1 = static_cast< sal_Int32 >( MAX_PERCENT - (MAX_PERCENT - mnC1) * fFactor );
                        mnC2 = static_cast< sal_Int32 >( MAX_PERCENT - (MAX_PERCENT - mnC2) * fFactor );
                        mnC3 = static_cast< sal_Int32 >( MAX_PERCENT - (MAX_PERCENT - mnC3) * fFactor );
                    }
                break;
                case XLS_TOKEN( tint ):
                    // Excel tint: move luminance relative to current value
                    toHsl();
                    OSL_ENSURE( (-MAX_PERCENT <= aIt->mnValue) && (aIt->mnValue <= MAX_PERCENT), "Color::getColor - invalid tint value" );
                    if( (-MAX_PERCENT <= aIt->mnValue) && (aIt->mnValue < 0) )
                    {
                        // negative: luminance towards 0% (black)
                        lclModValue( mnC3, aIt->mnValue + MAX_PERCENT );
                    }
                    else if( (0 < aIt->mnValue) && (aIt->mnValue <= MAX_PERCENT) )
                    {
                        // positive: luminance towards 100% (white)
                        mnC3 = MAX_PERCENT - mnC3;
                        lclModValue( mnC3, MAX_PERCENT - aIt->mnValue );
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
                    (mnC1 += 180 * PER_DEGREE) %= MAX_DEGREE;
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
        mnC1 = API_RGB_TRANSPARENT;
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
    return nRet;
}

bool Color::hasTransparency() const
{
    return mnAlpha < MAX_PERCENT;
}

sal_Int16 Color::getTransparency() const
{
    return static_cast< sal_Int16 >( (MAX_PERCENT - mnAlpha) / PER_PERCENT );
}

// private --------------------------------------------------------------------

void Color::setResolvedRgb( sal_Int32 nRgb ) const
{
    meMode = (nRgb < 0) ? COLOR_UNUSED : COLOR_RGB;
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
            SAL_FALLTHROUGH;
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
            SAL_FALLTHROUGH;
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

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
