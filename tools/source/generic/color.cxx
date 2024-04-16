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

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdlib.h>

#include <tools/color.hxx>
#include <tools/helpers.hxx>
#include <tools/long.hxx>
#include <o3tl/string_view.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <basegfx/numeric/ftools.hxx>

void Color::IncreaseLuminance(sal_uInt8 cLumInc)
{
    R = sal_uInt8(std::clamp(R + cLumInc, 0, 255));
    G = sal_uInt8(std::clamp(G + cLumInc, 0, 255));
    B = sal_uInt8(std::clamp(B + cLumInc, 0, 255));
}

void Color::DecreaseLuminance(sal_uInt8 cLumDec)
{
    R = sal_uInt8(std::clamp(R - cLumDec, 0, 255));
    G = sal_uInt8(std::clamp(G - cLumDec, 0, 255));
    B = sal_uInt8(std::clamp(B - cLumDec, 0, 255));
}

void Color::DecreaseContrast(sal_uInt8 nContDec)
{
    if (nContDec)
    {
        const double fM = (128.0 - 0.4985 * nContDec) / 128.0;
        const double fOff = 128.0 - fM * 128.0;

        R = basegfx::fround<sal_uInt8>(R * fM + fOff);
        G = basegfx::fround<sal_uInt8>(G * fM + fOff);
        B = basegfx::fround<sal_uInt8>(B * fM + fOff);
    }
}

// color space conversion

void Color::RGBtoHSB( sal_uInt16& nHue, sal_uInt16& nSat, sal_uInt16& nBri ) const
{
    sal_uInt8 c[3];
    sal_uInt8 cMax, cMin;

    c[0] = R;
    c[1] = G;
    c[2] = B;

    cMax = c[0];
    if( c[1] > cMax )
        cMax = c[1];
    if( c[2] > cMax )
        cMax = c[2];

    // Brightness = max(R, G, B);
    nBri = cMax * 100 / 255;

    cMin = c[0];
    if( c[1] < cMin )
        cMin = c[1];
    if( c[2] < cMin )
        cMin = c[2];

    sal_uInt8 cDelta = cMax - cMin;

    // Saturation = max - min / max
    if( nBri > 0 )
        nSat = cDelta * 100 / cMax;
    else
        nSat = 0;

    if( nSat == 0 )
        nHue = 0; // Default = undefined
    else
    {
        double dHue = 0.0;

        if( c[0] == cMax )
        {
            dHue = static_cast<double>( c[1] - c[2] ) / static_cast<double>(cDelta);
        }
        else if( c[1] == cMax )
        {
            dHue = 2.0 + static_cast<double>( c[2] - c[0] ) / static_cast<double>(cDelta);
        }
        else if ( c[2] == cMax )
        {
            dHue = 4.0 + static_cast<double>( c[0] - c[1] ) / static_cast<double>(cDelta);
        }
        dHue *= 60.0;

        if( dHue < 0.0 )
            dHue += 360.0;

        nHue = static_cast<sal_uInt16>(dHue);
    }
}

Color Color::HSBtoRGB( sal_uInt16 nHue, sal_uInt16 nSat, sal_uInt16 nBri )
{
    sal_uInt8 cR=0,cG=0,cB=0;
    sal_uInt8 nB = static_cast<sal_uInt8>( nBri * 255 / 100 );

    if( nSat == 0 )
    {
        cR = nB;
        cG = nB;
        cB = nB;
    }
    else
    {
        double dH = nHue;
        double f;
        sal_uInt16 n;
        if( dH == 360.0 )
            dH = 0.0;

        dH /= 60.0;
        n = static_cast<sal_uInt16>(dH);
        f = dH - n;

        sal_uInt8 a = static_cast<sal_uInt8>( nB * ( 100 - nSat ) / 100 );
        sal_uInt8 b = static_cast<sal_uInt8>( nB * ( 100 - ( static_cast<double>(nSat) * f ) ) / 100 );
        sal_uInt8 c = static_cast<sal_uInt8>( nB * ( 100 - ( static_cast<double>(nSat) * ( 1.0 - f ) ) ) / 100 );

        switch( n )
        {
            case 0: cR = nB;    cG = c;     cB = a;     break;
            case 1: cR = b;     cG = nB;    cB = a;     break;
            case 2: cR = a;     cG = nB;    cB = c;     break;
            case 3: cR = a;     cG = b;     cB = nB;    break;
            case 4: cR = c;     cG = a;     cB = nB;    break;
            case 5: cR = nB;    cG = a;     cB = b;     break;
        }
    }

    return Color( cR, cG, cB );
}

Color Color::STRtoRGB(std::u16string_view colorname)
{
    Color col;
    if(colorname.empty()) return col;

    switch(colorname.size()){
        case 7:
            col.mValue = o3tl::toUInt32(colorname.substr(1,6), 16);
            break;
        case 6:
            col.mValue = o3tl::toUInt32(colorname, 16);
            break;
        case 4:
        {
            sal_Unicode data[6] = { colorname[1], colorname[1], colorname[2],
                                     colorname[2], colorname[3], colorname[3] };
            col.mValue = o3tl::toUInt32(std::u16string_view(data,6), 16);
            break;
        }
        case 3:
        {
            sal_Unicode data[6] = { colorname[0], colorname[0], colorname[1],
                                     colorname[1], colorname[2], colorname[2] };
            col.mValue = o3tl::toUInt32(std::u16string_view(data,6), 16);
            break;
        }
        default:
            break;
    }
    return col;
}

OUString Color::AsRGBHexString() const
{
    std::stringstream ss;
    ss << std::hex << std::setfill ('0') << std::setw(6) << sal_uInt32(GetRGBColor());
    return OUString::createFromAscii(ss.str());
}

OUString Color::AsRGBHEXString() const
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill ('0') << std::setw(6) << sal_uInt32(GetRGBColor());
    return OUString::createFromAscii(ss.str());
}

void Color::ApplyTintOrShade(sal_Int16 n100thPercent)
{
    if (n100thPercent == 0)
        return;

    basegfx::BColor aBColor = basegfx::utils::rgb2hsl(getBColor());
    double fFactor = 1.0 - (std::abs(double(n100thPercent)) / 10000.0);
    double fResult;

    if (n100thPercent > 0) // tint
    {
        fResult = aBColor.getBlue() * fFactor + (1.0 - fFactor);
    }
    else // shade
    {
        fResult = aBColor.getBlue() * fFactor;
    }

    aBColor.setBlue(fResult);
    aBColor = basegfx::utils::hsl2rgb(aBColor);

    R = sal_uInt8(std::lround(aBColor.getRed()   * 255.0));
    G = sal_uInt8(std::lround(aBColor.getGreen() * 255.0));
    B = sal_uInt8(std::lround(aBColor.getBlue()  * 255.0));
}

void Color::ApplyLumModOff(sal_Int16 nMod, sal_Int16 nOff)
{
    if (nMod == 10000 && nOff == 0)
    {
        return;
    }
    // Switch to HSL, where applying these transforms is easier.
    basegfx::BColor aBColor = basegfx::utils::rgb2hsl(getBColor());

    // 50% is half luminance, 200% is double luminance. Unit is 100th percent.
    aBColor.setBlue(std::clamp(aBColor.getBlue() * nMod / 10000, 0.0, 1.0));
    // If color changes to black or white, it will stay gray if luminance changes again.
    if ((aBColor.getBlue() == 0.0) || (aBColor.getBlue() == 1.0))
    {
        aBColor.setGreen(0.0);
    }

    // Luminance offset means hue and saturation is left unchanged. Unit is 100th percent.
    aBColor.setBlue(std::clamp(aBColor.getBlue() + static_cast<double>(nOff) / 10000, 0.0, 1.0));
    // If color changes to black or white, it will stay gray if luminance changes again.
    if ((aBColor.getBlue() == 0.0) || (aBColor.getBlue() == 1.0))
    {
        aBColor.setGreen(0.0);
    }

    // Switch back to RGB.
    aBColor = basegfx::utils::hsl2rgb(aBColor);
    R = sal_uInt8(std::lround(aBColor.getRed()   * 255.0));
    G = sal_uInt8(std::lround(aBColor.getGreen() * 255.0));
    B = sal_uInt8(std::lround(aBColor.getBlue()  * 255.0));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
