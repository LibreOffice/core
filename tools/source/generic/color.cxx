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
#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <basegfx/color/bcolortools.hxx>

sal_uInt8 Color::GetColorError( const Color& rCompareColor ) const
{
    const long nErrAbs = labs( static_cast<long>(rCompareColor.GetRed()) - GetRed() ) +
                         labs( static_cast<long>(rCompareColor.GetGreen()) - GetGreen() ) +
                         labs( static_cast<long>(rCompareColor.GetBlue()) - GetBlue() );

    return static_cast<sal_uInt8>(FRound( nErrAbs * 0.3333333333 ));
}

void Color::IncreaseLuminance( sal_uInt8 cLumInc )
{
    SetRed( static_cast<sal_uInt8>(std::clamp( static_cast<long>(COLORDATA_RED( mnColor )) + cLumInc, 0L, 255L )) );
    SetGreen( static_cast<sal_uInt8>(std::clamp( static_cast<long>(COLORDATA_GREEN( mnColor )) + cLumInc, 0L, 255L )) );
    SetBlue( static_cast<sal_uInt8>(std::clamp( static_cast<long>(COLORDATA_BLUE( mnColor )) + cLumInc, 0L, 255L )) );
}

void Color::DecreaseLuminance( sal_uInt8 cLumDec )
{
    SetRed( static_cast<sal_uInt8>(std::clamp( static_cast<long>(COLORDATA_RED( mnColor )) - cLumDec, 0L, 255L )) );
    SetGreen( static_cast<sal_uInt8>(std::clamp( static_cast<long>(COLORDATA_GREEN( mnColor )) - cLumDec, 0L, 255L )) );
    SetBlue( static_cast<sal_uInt8>(std::clamp( static_cast<long>(COLORDATA_BLUE( mnColor )) - cLumDec, 0L, 255L )) );
}

void Color::DecreaseContrast( sal_uInt8 cContDec )
{
    if( cContDec )
    {
        const double fM = ( 128.0 - 0.4985 * cContDec ) / 128.0;
        const double fOff = 128.0 - fM * 128.0;

        SetRed( static_cast<sal_uInt8>(std::clamp( FRound( COLORDATA_RED( mnColor ) * fM + fOff ), 0L, 255L )) );
        SetGreen( static_cast<sal_uInt8>(std::clamp( FRound( COLORDATA_GREEN( mnColor ) * fM + fOff ), 0L, 255L )) );
        SetBlue( static_cast<sal_uInt8>(std::clamp( FRound( COLORDATA_BLUE( mnColor ) * fM + fOff ), 0L, 255L )) );
    }
}

void Color::Invert()
{
    SetRed( ~COLORDATA_RED( mnColor ) );
    SetGreen( ~COLORDATA_GREEN( mnColor ) );
    SetBlue( ~COLORDATA_BLUE( mnColor ) );
}

bool Color::IsDark() const
{
    return GetLuminance() <= 60;
}

bool Color::IsBright() const
{
    return GetLuminance() >= 245;
}

// color space conversion

void Color::RGBtoHSB( sal_uInt16& nHue, sal_uInt16& nSat, sal_uInt16& nBri ) const
{
    sal_uInt8 c[3];
    sal_uInt8 cMax, cMin;

    c[0] = GetRed();
    c[1] = GetGreen();
    c[2] = GetBlue();

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

SvStream& Color::Read( SvStream& rIStm )
{
    rIStm.ReadUInt32( mnColor );
    return rIStm;
}

SvStream& Color::Write( SvStream& rOStm ) const
{
    rOStm.WriteUInt32( mnColor );
    return rOStm;
}

OUString Color::AsRGBHexString() const
{
    std::stringstream ss;
    ss << std::hex << std::setfill ('0') << std::setw(6) << sal_uInt32(GetRGBColor());
    return OUString::createFromAscii(ss.str().c_str());
}

#define COL_NAME_USER       (sal_uInt16(0x8000))

SvStream& ReadColor( SvStream& rIStream, Color& rColor )
{
    sal_uInt16 nColorName(0);

    rIStream.ReadUInt16( nColorName );

    if ( nColorName & COL_NAME_USER )
    {
        sal_uInt16 nRed;
        sal_uInt16 nGreen;
        sal_uInt16 nBlue;

        rIStream.ReadUInt16( nRed );
        rIStream.ReadUInt16( nGreen );
        rIStream.ReadUInt16( nBlue );

        rColor = Color( nRed>>8, nGreen>>8, nBlue>>8 );
    }
    else
    {
        static const Color aColAry[] =
        {
            COL_BLACK,                          // COL_BLACK
            COL_BLUE,                           // COL_BLUE
            COL_GREEN,                          // COL_GREEN
            COL_CYAN,                           // COL_CYAN
            COL_RED,                            // COL_RED
            COL_MAGENTA,                        // COL_MAGENTA
            COL_BROWN,                          // COL_BROWN
            COL_GRAY,                           // COL_GRAY
            COL_LIGHTGRAY,                      // COL_LIGHTGRAY
            COL_LIGHTBLUE,                      // COL_LIGHTBLUE
            COL_LIGHTGREEN,                     // COL_LIGHTGREEN
            COL_LIGHTCYAN,                      // COL_LIGHTCYAN
            COL_LIGHTRED,                       // COL_LIGHTRED
            COL_LIGHTMAGENTA,                   // COL_LIGHTMAGENTA
            COL_YELLOW,                         // COL_YELLOW
            COL_WHITE,                          // COL_WHITE
            COL_WHITE,                          // COL_MENUBAR
            COL_BLACK,                          // COL_MENUBARTEXT
            COL_WHITE,                          // COL_POPUPMENU
            COL_BLACK,                          // COL_POPUPMENUTEXT
            COL_BLACK,                          // COL_WINDOWTEXT
            COL_WHITE,                          // COL_WINDOWWORKSPACE
            COL_BLACK,                          // COL_HIGHLIGHT
            COL_WHITE,                          // COL_HIGHLIGHTTEXT
            COL_BLACK,                          // COL_3DTEXT
            COL_LIGHTGRAY,                      // COL_3DFACE
            COL_WHITE,                          // COL_3DLIGHT
            COL_GRAY,                           // COL_3DSHADOW
            COL_LIGHTGRAY,                      // COL_SCROLLBAR
            COL_WHITE,                          // COL_FIELD
            COL_BLACK                           // COL_FIELDTEXT
        };

        if ( nColorName < SAL_N_ELEMENTS( aColAry ) )
            rColor = aColAry[nColorName];
        else
            rColor = COL_BLACK;
    }

    return rIStream;
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

    SetRed(sal_uInt8((  aBColor.getRed()   * 255.0) + 0.5));
    SetGreen(sal_uInt8((aBColor.getGreen() * 255.0) + 0.5));
    SetBlue(sal_uInt8(( aBColor.getBlue()  * 255.0) + 0.5));
}

SvStream& WriteColor( SvStream& rOStream, const Color& rColor )
{
    sal_uInt16 nRed         = rColor.GetRed();
    sal_uInt16 nGreen       = rColor.GetGreen();
    sal_uInt16 nBlue        = rColor.GetBlue();
    nRed    = (nRed<<8) + nRed;
    nGreen  = (nGreen<<8) + nGreen;
    nBlue   = (nBlue<<8) + nBlue;

    rOStream.WriteUInt16( COL_NAME_USER );
    rOStream.WriteUInt16( nRed );
    rOStream.WriteUInt16( nGreen );
    rOStream.WriteUInt16( nBlue );

    return rOStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
