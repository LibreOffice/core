/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: color.cxx,v $
 * $Revision: 1.7 $
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

#include "oox/drawingml/color.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "tokens.hxx"
#include <algorithm>
#include <math.h>

namespace oox {
namespace drawingml {

// ============================================================================

namespace {

const sal_Int32 PER_PERCENT     = 1000;
const sal_Int32 MAX_PERCENT     = 100 * PER_PERCENT;

const sal_Int32 PER_DEGREE      = 60000;
const sal_Int32 MAX_DEGREE      = 360 * PER_DEGREE;

const double DEC_GAMMA          = 2.3;
const double INC_GAMMA          = 1.0 / DEC_GAMMA;

sal_Int32 lclGetPresetColor( sal_Int32 nToken )
{
    switch( nToken )
    {
        case XML_aliceBlue:         return 0xF0F8FF;
        case XML_antiqueWhite:      return 0xFAEBD7;
        case XML_aqua:              return 0x00FFFF;
        case XML_aquamarine:        return 0x7FFFD4;
        case XML_azure:             return 0xF0FFFF;
        case XML_beige:             return 0xF5F5DC;
        case XML_bisque:            return 0xFFE4C4;
        case XML_black:             return 0x000000;
        case XML_blanchedAlmond:    return 0xFFEBCD;
        case XML_blue:              return 0x0000FF;
        case XML_blueViolet:        return 0x8A2BE2;
        case XML_brown:             return 0xA52A2A;
        case XML_burlyWood:         return 0xDEB887;
        case XML_cadetBlue:         return 0x5F9EA0;
        case XML_chartreuse:        return 0x7FFF00;
        case XML_chocolate:         return 0xD2691E;
        case XML_coral:             return 0xFF7F50;
        case XML_cornflowerBlue:    return 0x6495ED;
        case XML_cornsilk:          return 0xFFF8DC;
        case XML_crimson:           return 0xDC143C;
        case XML_cyan:              return 0x00FFFF;
        case XML_deepPink:          return 0xFF1493;
        case XML_deepSkyBlue:       return 0x00BFFF;
        case XML_dimGray:           return 0x696969;
        case XML_dkBlue:            return 0x00008B;
        case XML_dkCyan:            return 0x008B8B;
        case XML_dkGoldenrod:       return 0xB8860B;
        case XML_dkGray:            return 0xA9A9A9;
        case XML_dkGreen:           return 0x006400;
        case XML_dkKhaki:           return 0xBDB76B;
        case XML_dkMagenta:         return 0x8B008B;
        case XML_dkOliveGreen:      return 0x556B2F;
        case XML_dkOrange:          return 0xFF8C00;
        case XML_dkOrchid:          return 0x9932CC;
        case XML_dkRed:             return 0x8B0000;
        case XML_dkSalmon:          return 0xE9967A;
        case XML_dkSeaGreen:        return 0x8FBC8B;
        case XML_dkSlateBlue:       return 0x483D8B;
        case XML_dkSlateGray:       return 0x2F4F4F;
        case XML_dkTurquoise:       return 0x00CED1;
        case XML_dkViolet:          return 0x9400D3;
        case XML_dodgerBlue:        return 0x1E90FF;
        case XML_firebrick:         return 0xB22222;
        case XML_floralWhite:       return 0xFFFAF0;
        case XML_forestGreen:       return 0x228B22;
        case XML_fuchsia:           return 0xFF00FF;
        case XML_gainsboro:         return 0xDCDCDC;
        case XML_ghostWhite:        return 0xF8F8FF;
        case XML_gold:              return 0xFFD700;
        case XML_goldenrod:         return 0xDAA520;
        case XML_gray:              return 0x808080;
        case XML_green:             return 0x008000;
        case XML_greenYellow:       return 0xADFF2F;
        case XML_honeydew:          return 0xF0FFF0;
        case XML_hotPink:           return 0xFF69B4;
        case XML_indianRed:         return 0xCD5C5C;
        case XML_indigo:            return 0x4B0082;
        case XML_ivory:             return 0xFFFFF0;
        case XML_khaki:             return 0xF0E68C;
        case XML_lavender:          return 0xE6E6FA;
        case XML_lavenderBlush:     return 0xFFF0F5;
        case XML_lawnGreen:         return 0x7CFC00;
        case XML_lemonChiffon:      return 0xFFFACD;
        case XML_lime:              return 0x00FF00;
        case XML_limeGreen:         return 0x32CD32;
        case XML_linen:             return 0xFAF0E6;
        case XML_ltBlue:            return 0xADD8E6;
        case XML_ltCoral:           return 0xF08080;
        case XML_ltCyan:            return 0xE0FFFF;
        case XML_ltGoldenrodYellow: return 0xFAFA78;
        case XML_ltGray:            return 0xD3D3D3;
        case XML_ltGreen:           return 0x90EE90;
        case XML_ltPink:            return 0xFFB6C1;
        case XML_ltSalmon:          return 0xFFA07A;
        case XML_ltSeaGreen:        return 0x20B2AA;
        case XML_ltSkyBlue:         return 0x87CEFA;
        case XML_ltSlateGray:       return 0x778899;
        case XML_ltSteelBlue:       return 0xB0C4DE;
        case XML_ltYellow:          return 0xFFFFE0;
        case XML_magenta:           return 0xFF00FF;
        case XML_maroon:            return 0x800000;
        case XML_medAquamarine:     return 0x66CDAA;
        case XML_medBlue:           return 0x0000CD;
        case XML_medOrchid:         return 0xBA55D3;
        case XML_medPurple:         return 0x9370DB;
        case XML_medSeaGreen:       return 0x3CB371;
        case XML_medSlateBlue:      return 0x7B68EE;
        case XML_medSpringGreen:    return 0x00FA9A;
        case XML_medTurquoise:      return 0x48D1CC;
        case XML_medVioletRed:      return 0xC71585;
        case XML_midnightBlue:      return 0x191970;
        case XML_mintCream:         return 0xF5FFFA;
        case XML_mistyRose:         return 0xFFE4E1;
        case XML_moccasin:          return 0xFFE4B5;
        case XML_navajoWhite:       return 0xFFDEAD;
        case XML_navy:              return 0x000080;
        case XML_oldLace:           return 0xFDF5E6;
        case XML_olive:             return 0x808000;
        case XML_oliveDrab:         return 0x6B8E23;
        case XML_orange:            return 0xFFA500;
        case XML_orangeRed:         return 0xFF4500;
        case XML_orchid:            return 0xDA70D6;
        case XML_paleGoldenrod:     return 0xEEE8AA;
        case XML_paleGreen:         return 0x98FB98;
        case XML_paleTurquoise:     return 0xAFEEEE;
        case XML_paleVioletRed:     return 0xDB7093;
        case XML_papayaWhip:        return 0xFFEFD5;
        case XML_peachPuff:         return 0xFFDAB9;
        case XML_peru:              return 0xCD853F;
        case XML_pink:              return 0xFFC0CB;
        case XML_plum:              return 0xDDA0DD;
        case XML_powderBlue:        return 0xB0E0E6;
        case XML_purple:            return 0x800080;
        case XML_red:               return 0xFF0000;
        case XML_rosyBrown:         return 0xBC8F8F;
        case XML_royalBlue:         return 0x4169E1;
        case XML_saddleBrown:       return 0x8B4513;
        case XML_salmon:            return 0xFA8072;
        case XML_sandyBrown:        return 0xF4A460;
        case XML_seaGreen:          return 0x2E8B57;
        case XML_seaShell:          return 0xFFF5EE;
        case XML_sienna:            return 0xA0522D;
        case XML_silver:            return 0xC0C0C0;
        case XML_skyBlue:           return 0x87CEEB;
        case XML_slateBlue:         return 0x6A5ACD;
        case XML_slateGray:         return 0x708090;
        case XML_snow:              return 0xFFFAFA;
        case XML_springGreen:       return 0x00FF7F;
        case XML_steelBlue:         return 0x4682B4;
        case XML_tan:               return 0xD2B48C;
        case XML_teal:              return 0x008080;
        case XML_thistle:           return 0xD8BFD8;
        case XML_tomato:            return 0xFF6347;
        case XML_turquoise:         return 0x40E0D0;
        case XML_violet:            return 0xEE82EE;
        case XML_wheat:             return 0xF5DEB3;
        case XML_white:             return 0xFFFFFF;
        case XML_whiteSmoke:        return 0xF5F5F5;
        case XML_yellow:            return 0xFFFF00;
        case XML_yellowGreen:       return 0x9ACD32;
    }
    OSL_ENSURE( false, "lclGetPresetColor - invalid preset color token" );
    return API_RGB_BLACK;
}

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

// ----------------------------------------------------------------------------

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
    setSrgbClr( lclGetPresetColor( nToken ) );
}

void Color::setSchemeClr( sal_Int32 nToken )
{
    OSL_ENSURE( nToken != XML_TOKEN_INVALID, "Color::setSchemeClr - invalid color token" );
    meMode = (nToken == XML_phClr) ? COLOR_PH : COLOR_SCHEME;
    mnC1 = nToken;
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
    sal_Int32 nToken = getToken( nElement );
    switch( nToken )
    {
        case XML_alpha:     lclSetValue( mnAlpha, nValue ); break;
        case XML_alphaMod:  lclModValue( mnAlpha, nValue ); break;
        case XML_alphaOff:  lclOffValue( mnAlpha, nValue ); break;
        default:            maTransforms.push_back( Transformation( nToken, nValue ) );
    }
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
    maTransforms.push_back( Transformation( NMSP_XLS | XML_tint, nValue ) );
}

void Color::clearTransparence()
{
    mnAlpha = MAX_PERCENT;
}

sal_Int32 Color::getColor( const ::oox::core::XmlFilterBase& rFilter, sal_Int32 nPhClr ) const
{
    /*  Special handling for theme style list placeholder colors (state
        COLOR_PH), Color::getColor() may be called with different placeholder
        colors in the nPhClr parameter. Therefore, the resolved color will not
        be stored in this object, thus the state COLOR_FINAL will not be
        reached and the transformation container will not be cleared, but the
        original COLOR_PH state will be restored instead. */
    bool bIsPh = false;

    switch( meMode )
    {
        case COLOR_UNUSED:  return -1;
        case COLOR_FINAL:   return mnC1;

        case COLOR_RGB:     break;  // nothing to do
        case COLOR_CRGB:    break;  // nothing to do
        case COLOR_HSL:     break;  // nothing to do

        case COLOR_SCHEME:
            meMode = COLOR_RGB;
            lclRgbToRgbComponents( mnC1, mnC2, mnC3, rFilter.getSchemeClr( mnC1 ) );
        break;
        case COLOR_PH:
            meMode = COLOR_RGB;
            lclRgbToRgbComponents( mnC1, mnC2, mnC3, nPhClr );
            bIsPh = true;
        break;
        case COLOR_SYSTEM:
            meMode = COLOR_RGB;
            lclRgbToRgbComponents( mnC1, mnC2, mnC3, rFilter.getSystemColor( mnC1, mnC2 ) );
        break;
    }

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

    toRgb();
    meMode = bIsPh ? COLOR_PH : COLOR_FINAL;
    if( meMode == COLOR_FINAL )
        maTransforms.clear();
    return mnC1 = lclRgbComponentsToRgb( mnC1, mnC2, mnC3 );
}

bool Color::hasTransparence() const
{
    return mnAlpha < MAX_PERCENT;
}

sal_Int16 Color::getTransparence() const
{
    return static_cast< sal_Int16 >( (MAX_PERCENT - mnAlpha) / PER_PERCENT );
}

// private --------------------------------------------------------------------

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
            OSL_ENSURE( false, "Color::toRgb - unexpected color mode" );
    }
}

void Color::toCrgb() const
{
    switch( meMode )
    {
        case COLOR_HSL:
            toRgb();
            // run through!
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
            OSL_ENSURE( false, "Color::toCrgb - unexpected color mode" );
    }
}

void Color::toHsl() const
{
    switch( meMode )
    {
        case COLOR_CRGB:
            toRgb();
            // run through!
        case COLOR_RGB:
        {
            meMode = COLOR_HSL;
            double fR = static_cast< double >( mnC1 ) / 255.0;  // red [0.0, 1.0]
            double fG = static_cast< double >( mnC2 ) / 255.0;  // green [0.0, 1.0]
            double fB = static_cast< double >( mnC3 ) / 255.0;  // blue [0.0, 1.0]
            double fMin = ::std::min( ::std::min( fR, fG ), fB );
            double fMax = ::std::max( ::std::max( fR, fG ), fB );
            double fD = fMax - fMin;

            // hue: 0deg = red, 120deg = green, 240deg = blue
            if( fD == 0.0 )         // black/gray/white
                mnC1 = 0;
            else if( fMax == fR )   // magenta...red...yellow
                mnC1 = static_cast< sal_Int32 >( ((fG - fB) / fD * 60.0 + 360.0) * PER_DEGREE + 0.5 ) % MAX_DEGREE;
            else if( fMax == fG )   // yellow...green...cyan
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
            OSL_ENSURE( false, "Color::toHsl - unexpected color mode" );
    }
}

// ============================================================================

} // namespace drawingml
} // namespace oox

