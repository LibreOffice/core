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
#ifndef INCLUDED_VCL_COLOR_HXX
#define INCLUDED_VCL_COLOR_HXX

#include <vcl/dllapi.h>
#include <vcl/colordata.hxx>

class SvStream;

#include <basegfx/color/bcolor.hxx>

class SAL_WARN_UNUSED VCL_DLLPUBLIC Color
{
    ColorData mnColor;

public:
    Color()
        : mnColor(COL_BLACK)
    {}
    Color(ColorData nColor)
        : mnColor(nColor)
    {}
    Color(sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue)
        : mnColor(RGB_COLORDATA(nRed, nGreen, nBlue))
    {}
    Color(sal_uInt8 nTransparency, sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue)
        : mnColor(TRGB_COLORDATA(nTransparency, nRed, nGreen, nBlue))
    {}

    // constructor to create a tools-Color from ::basegfx::BColor
    explicit Color(const basegfx::BColor& rBColor)
        : mnColor(RGB_COLORDATA(sal_uInt8((rBColor.getRed() * 255.0) + 0.5),
                                sal_uInt8((rBColor.getGreen() * 255.0) + 0.5),
                                sal_uInt8((rBColor.getBlue() * 255.0) + 0.5)))
    {}

    virtual ~Color() {}

    bool operator<(const Color& b) const { return mnColor < b.GetColor(); }

    virtual sal_uInt8 GetRed() const { return COLORDATA_RED(mnColor); }
    virtual sal_uInt8 GetGreen() const { return COLORDATA_GREEN(mnColor); }
    virtual sal_uInt8 GetBlue() const { return COLORDATA_BLUE(mnColor); }

    virtual void SetRed(sal_uInt8 nRed);
    virtual void SetGreen(sal_uInt8 nGreen);
    virtual void SetBlue(sal_uInt8 nBlue);

    sal_uInt8 GetTransparency() const { return COLORDATA_TRANSPARENCY(mnColor); }
    void SetTransparency(sal_uInt8 nTransparency);

    ColorData GetColor() const { return mnColor; }
    void SetColor(ColorData nColor) { mnColor = nColor; }

    ColorData GetRGBColor() const { return COLORDATA_RGB(mnColor); }
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
    static ColorData HSBtoRGB(sal_uInt16 nHue, sal_uInt16 nSaturation, sal_uInt16 nBrightness);
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

    VCL_DLLPUBLIC friend SvStream& ReadColor(SvStream& rIStream, Color& rColor);
    VCL_DLLPUBLIC friend SvStream& WriteColor(SvStream& rOStream, const Color& rColor);

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

inline void Color::Merge( const Color& rMergeColor, sal_uInt8 cTransparency )
{
    SetRed(ColorChannelMerge(COLORDATA_RED(mnColor), COLORDATA_RED(rMergeColor.mnColor), cTransparency));
    SetGreen(ColorChannelMerge(COLORDATA_GREEN(mnColor), COLORDATA_GREEN(rMergeColor.mnColor), cTransparency));
    SetBlue(ColorChannelMerge(COLORDATA_BLUE(mnColor), COLORDATA_BLUE(rMergeColor.mnColor), cTransparency));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
