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
#ifndef INCLUDED_TOOLS_COLOR_HXX
#define INCLUDED_TOOLS_COLOR_HXX

#include <sal/types.h>
#include <tools/toolsdllapi.h>
#include <com/sun/star/uno/Any.hxx>
#include <basegfx/color/bcolor.hxx>

class SvStream;

constexpr sal_uInt8  COLORDATA_RED( sal_uInt32 n )          { return static_cast<sal_uInt8>(n>>16); }
constexpr sal_uInt8  COLORDATA_GREEN( sal_uInt32 n )        { return static_cast<sal_uInt8>(static_cast<sal_uInt16>(n) >> 8); }
constexpr sal_uInt8  COLORDATA_BLUE( sal_uInt32 n )         { return static_cast<sal_uInt8>(n); }
constexpr sal_uInt8  COLORDATA_TRANSPARENCY( sal_uInt32 n ) { return static_cast<sal_uInt8>(n>>24); }
constexpr sal_uInt32 COLORDATA_RGB( sal_uInt32 n )          { return static_cast<sal_uInt32>(n & 0x00FFFFFF); }

// Color

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Color final
{
    sal_uInt32 mnColor;

public:
    constexpr Color()
        : mnColor(0) // black
    {}
    constexpr Color(sal_uInt32 nColor)
        : mnColor(nColor)
    {}
    constexpr Color(sal_uInt8 nTransparency, sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue)
        : mnColor( sal_uInt32(nBlue) | (sal_uInt32(nGreen) << 8) | (sal_uInt32(nRed) << 16)
                   | (sal_uInt32(nTransparency) << 24) )
    {}
    constexpr Color(sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue)
        : Color(0, nRed, nGreen, nBlue)
    {}

    // constructor to create a tools-Color from ::basegfx::BColor
    explicit Color(const basegfx::BColor& rBColor)
        : Color(0,
                sal_uInt8((rBColor.getRed() * 255.0) + 0.5),
                sal_uInt8((rBColor.getGreen() * 255.0) + 0.5),
                sal_uInt8((rBColor.getBlue() * 255.0) + 0.5))
    {}

    /** Primarily used when passing Color objects to UNO API */
    constexpr explicit operator sal_uInt32() const
    {
        return mnColor;
    }

    constexpr explicit operator sal_Int32() const
    {
        return sal_Int32(mnColor);
    }

    bool operator<(const Color& b) const
    {
        return mnColor < b.mnColor;
    }

    void SetRed(sal_uInt8 nRed);
    sal_uInt8 GetRed() const
    {
        return COLORDATA_RED(mnColor);
    }
    void SetGreen(sal_uInt8 nGreen);
    sal_uInt8 GetGreen() const
    {
        return COLORDATA_GREEN(mnColor);
    }
    void SetBlue(sal_uInt8 nBlue);
    sal_uInt8 GetBlue() const
    {
        return COLORDATA_BLUE(mnColor);
    }
    void SetTransparency(sal_uInt8 nTransparency);
    sal_uInt8 GetTransparency() const
    {
        return COLORDATA_TRANSPARENCY(mnColor);
    }

    Color GetRGBColor() const
    {
        return COLORDATA_RGB(mnColor);
    }

    sal_uInt8 GetColorError(const Color& rCompareColor) const;

    sal_uInt8 GetLuminance() const;
    void IncreaseLuminance(sal_uInt8 cLumInc);
    void DecreaseLuminance(sal_uInt8 cLumDec);

    void DecreaseContrast(sal_uInt8 cContDec);

    /**
     * Apply tint or shade to a color.
     *
     * The input value is the percentage (in 100th of percent) of how much the
     * color changes towards the black (shade) or white (tint). If the value
     * is positive, the color is tinted, if the value is negative, the color is
     * shaded.
     **/
    void ApplyTintOrShade(sal_Int16 n100thPercent);

    void Invert();

    void Merge(const Color& rMergeColor, sal_uInt8 cTransparency);

    bool IsRGBEqual(const Color& rColor) const;

    // comparison with luminance thresholds
    bool IsDark() const;
    bool IsBright() const;

    // color space conversion tools
    // the range for h/s/b is:
    // Hue: 0-360 degree
    // Saturation: 0-100%
    // Brightness: 0-100%
    static Color HSBtoRGB(sal_uInt16 nHue, sal_uInt16 nSaturation, sal_uInt16 nBrightness);
    void RGBtoHSB(sal_uInt16& nHue, sal_uInt16& nSaturation, sal_uInt16& nBrightness) const;

    bool operator==(const Color& rColor) const
    {
        return mnColor == rColor.mnColor;
    }
    bool operator!=(const Color& rColor) const
    {
        return !(Color::operator==(rColor));
    }

    SvStream& Read(SvStream& rIStream);
    SvStream& Write(SvStream& rOStream) const;

    TOOLS_DLLPUBLIC friend SvStream& ReadColor(SvStream& rIStream, Color& rColor);
    TOOLS_DLLPUBLIC friend SvStream& WriteColor(SvStream& rOStream, const Color& rColor);

    // Return color as RGB hex string
    // for example "00ff00" for green color
    OUString AsRGBHexString() const;

    // get ::basegfx::BColor from this color
    basegfx::BColor getBColor() const
    {
        return basegfx::BColor(GetRed() / 255.0, GetGreen() / 255.0, GetBlue() / 255.0);
    }
};

inline void Color::SetRed( sal_uInt8 nRed )
{
    mnColor &= 0xFF00FFFF;
    mnColor |= static_cast<sal_uInt32>(nRed)<<16;
}

inline void Color::SetGreen( sal_uInt8 nGreen )
{
    mnColor &= 0xFFFF00FF;
    mnColor |= static_cast<sal_uInt16>(nGreen)<<8;
}

inline void Color::SetBlue( sal_uInt8 nBlue )
{
    mnColor &= 0xFFFFFF00;
    mnColor |= nBlue;
}

inline void Color::SetTransparency( sal_uInt8 nTransparency )
{
    mnColor &= 0x00FFFFFF;
    mnColor |= static_cast<sal_uInt32>(nTransparency)<<24;
}

inline bool Color::IsRGBEqual( const Color& rColor ) const
{
    return COLORDATA_RGB( mnColor ) == COLORDATA_RGB(rColor.mnColor);
}

inline sal_uInt8 Color::GetLuminance() const
{
    return static_cast<sal_uInt8>((COLORDATA_BLUE(mnColor) * 29UL +
                                   COLORDATA_GREEN(mnColor) * 151UL +
                                   COLORDATA_RED(mnColor) * 76UL) >> 8);
}

constexpr sal_uInt8 ColorChannelMerge(sal_uInt8 nDst, sal_uInt8 nSrc, sal_uInt8 nSrcTrans)
{
    return static_cast<sal_uInt8>(((static_cast<sal_Int32>(nDst)-nSrc)*nSrcTrans+((nSrc<<8)|nDst))>>8);
};

inline void Color::Merge( const Color& rMergeColor, sal_uInt8 cTransparency )
{
    SetRed(ColorChannelMerge(COLORDATA_RED(mnColor), COLORDATA_RED(rMergeColor.mnColor), cTransparency));
    SetGreen(ColorChannelMerge(COLORDATA_GREEN(mnColor), COLORDATA_GREEN(rMergeColor.mnColor), cTransparency));
    SetBlue(ColorChannelMerge(COLORDATA_BLUE(mnColor), COLORDATA_BLUE(rMergeColor.mnColor), cTransparency));
}

// to reduce the noise when moving these into and out of Any
inline bool operator >>=( const css::uno::Any & rAny, Color & value )
{
  sal_Int32 nTmp;
  if (!(rAny >>= nTmp))
      return false;
  value = Color(nTmp);
  return true;
}
inline void operator <<=( css::uno::Any & rAny, Color value )
{
    rAny <<= sal_Int32(value);
}
namespace com { namespace sun { namespace star { namespace uno {
    template<>
    inline Any makeAny( Color const & value )
    {
        return Any(sal_Int32(value));
    }
} } } }

// Color types

constexpr ::Color COL_BLACK                   ( 0x00, 0x00, 0x00 );
constexpr ::Color COL_BLUE                    ( 0x00, 0x00, 0x80 );
constexpr ::Color COL_GREEN                   ( 0x00, 0x80, 0x00 );
constexpr ::Color COL_CYAN                    ( 0x00, 0x80, 0x80 );
constexpr ::Color COL_RED                     ( 0x80, 0x00, 0x00 );
constexpr ::Color COL_MAGENTA                 ( 0x80, 0x00, 0x80 );
constexpr ::Color COL_BROWN                   ( 0x80, 0x80, 0x00 );
constexpr ::Color COL_GRAY                    ( 0x80, 0x80, 0x80 );
constexpr ::Color COL_GRAY3                   ( 0xCC, 0xCC, 0xCC );
constexpr ::Color COL_GRAY7                   ( 0x66, 0x66, 0x66 );
constexpr ::Color COL_LIGHTGRAY               ( 0xC0, 0xC0, 0xC0 );
constexpr ::Color COL_LIGHTBLUE               ( 0x00, 0x00, 0xFF );
constexpr ::Color COL_LIGHTGREEN              ( 0x00, 0xFF, 0x00 );
constexpr ::Color COL_LIGHTCYAN               ( 0x00, 0xFF, 0xFF );
constexpr ::Color COL_LIGHTRED                ( 0xFF, 0x00, 0x00 );
constexpr ::Color COL_LIGHTMAGENTA            ( 0xFF, 0x00, 0xFF );
constexpr ::Color COL_LIGHTGRAYBLUE           ( 0xE0, 0xE0, 0xFF );
constexpr ::Color COL_YELLOW                  ( 0xFF, 0xFF, 0x00 );
constexpr ::Color COL_WHITE                   ( 0xFF, 0xFF, 0xFF );
constexpr ::Color COL_TRANSPARENT             ( 0xFF, 0xFF, 0xFF, 0xFF );
constexpr ::Color COL_AUTO                    ( 0xFF, 0xFF, 0xFF, 0xFF );
constexpr ::Color COL_AUTHOR1_DARK            ( 198,  146,   0 );
constexpr ::Color COL_AUTHOR1_NORMAL          ( 255,  255, 158 );
constexpr ::Color COL_AUTHOR1_LIGHT           ( 255,  255, 195 );
constexpr ::Color COL_AUTHOR2_DARK            (   6,   70, 162 );
constexpr ::Color COL_AUTHOR2_NORMAL          ( 216,  232, 255 );
constexpr ::Color COL_AUTHOR2_LIGHT           ( 233,  242, 255 );
constexpr ::Color COL_AUTHOR3_DARK            (  87,  157,  28 );
constexpr ::Color COL_AUTHOR3_NORMAL          ( 218,  248, 193 );
constexpr ::Color COL_AUTHOR3_LIGHT           ( 226,  250, 207 );
constexpr ::Color COL_AUTHOR4_DARK            ( 105,   43, 157 );
constexpr ::Color COL_AUTHOR4_NORMAL          ( 228,  210, 245 );
constexpr ::Color COL_AUTHOR4_LIGHT           ( 239,  228, 248 );
constexpr ::Color COL_AUTHOR5_DARK            ( 197,    0,  11 );
constexpr ::Color COL_AUTHOR5_NORMAL          ( 254,  205, 208 );
constexpr ::Color COL_AUTHOR5_LIGHT           ( 255,  227, 229 );
constexpr ::Color COL_AUTHOR6_DARK            (   0,  128, 128 );
constexpr ::Color COL_AUTHOR6_NORMAL          ( 210,  246, 246 );
constexpr ::Color COL_AUTHOR6_LIGHT           ( 230,  250, 250 );
constexpr ::Color COL_AUTHOR7_DARK            ( 140,  132,   0 );
constexpr ::Color COL_AUTHOR7_NORMAL          ( 237,  252, 163 );
constexpr ::Color COL_AUTHOR7_LIGHT           ( 242,  254, 181 );
constexpr ::Color COL_AUTHOR8_DARK            (  53,   85, 107 );
constexpr ::Color COL_AUTHOR8_NORMAL          ( 211,  222, 232 );
constexpr ::Color COL_AUTHOR8_LIGHT           ( 226,  234, 241 );
constexpr ::Color COL_AUTHOR9_DARK            ( 209,  118,   0 );
constexpr ::Color COL_AUTHOR9_NORMAL          ( 255,  226, 185 );
constexpr ::Color COL_AUTHOR9_LIGHT           ( 255,  231, 199 );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
