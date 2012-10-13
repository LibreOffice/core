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
#ifndef _TOOLS_COLOR_HXX
#define _TOOLS_COLOR_HXX

#include "tools/toolsdllapi.h"

class SvStream;
class ResId;

#include <tools/solar.h>
#include <basegfx/color/bcolor.hxx>

// Color types
typedef sal_uInt32 ColorData;
#define RGB_COLORDATA( r,g,b )      ((ColorData)(((sal_uInt32)((sal_uInt8)(b))))|(((sal_uInt32)((sal_uInt8)(g)))<<8)|(((sal_uInt32)((sal_uInt8)(r)))<<16))
#define TRGB_COLORDATA( t,r,g,b )   ((ColorData)(((sal_uInt32)((sal_uInt8)(b))))|(((sal_uInt32)((sal_uInt8)(g)))<<8)|(((sal_uInt32)((sal_uInt8)(r)))<<16)|(((sal_uInt32)((sal_uInt8)(t)))<<24))
#define COLORDATA_RED( n )          ((sal_uInt8)((n)>>16))
#define COLORDATA_GREEN( n )        ((sal_uInt8)(((sal_uInt16)(n)) >> 8))
#define COLORDATA_BLUE( n )         ((sal_uInt8)(n))
#define COLORDATA_TRANSPARENCY( n ) ((sal_uInt8)((n)>>24))
#define COLORDATA_RGB( n )          ((ColorData)((n) & 0x00FFFFFF))

#define COL_BLACK                   RGB_COLORDATA( 0x00, 0x00, 0x00 )
#define COL_BLUE                    RGB_COLORDATA( 0x00, 0x00, 0x80 )
#define COL_GREEN                   RGB_COLORDATA( 0x00, 0x80, 0x00 )
#define COL_CYAN                    RGB_COLORDATA( 0x00, 0x80, 0x80 )
#define COL_RED                     RGB_COLORDATA( 0x80, 0x00, 0x00 )
#define COL_MAGENTA                 RGB_COLORDATA( 0x80, 0x00, 0x80 )
#define COL_BROWN                   RGB_COLORDATA( 0x80, 0x80, 0x00 )
#define COL_GRAY                    RGB_COLORDATA( 0x80, 0x80, 0x80 )
#define COL_LIGHTGRAY               RGB_COLORDATA( 0xC0, 0xC0, 0xC0 )
#define COL_LIGHTBLUE               RGB_COLORDATA( 0x00, 0x00, 0xFF )
#define COL_LIGHTGREEN              RGB_COLORDATA( 0x00, 0xFF, 0x00 )
#define COL_LIGHTCYAN               RGB_COLORDATA( 0x00, 0xFF, 0xFF )
#define COL_LIGHTRED                RGB_COLORDATA( 0xFF, 0x00, 0x00 )
#define COL_LIGHTMAGENTA            RGB_COLORDATA( 0xFF, 0x00, 0xFF )
#define COL_LIGHTGRAYBLUE           RGB_COLORDATA( 0xE0, 0xE0, 0xFF )
#define COL_YELLOW                  RGB_COLORDATA( 0xFF, 0xFF, 0x00 )
#define COL_WHITE                   RGB_COLORDATA( 0xFF, 0xFF, 0xFF )
#define COL_TRANSPARENT             TRGB_COLORDATA( 0xFF, 0xFF, 0xFF, 0xFF )
#define COL_AUTO                    (ColorData)0xFFFFFFFF
#define COL_AUTHOR1_DARK            RGB_COLORDATA(198, 146, 0)
#define COL_AUTHOR1_NORMAL          RGB_COLORDATA(255, 255, 158)
#define COL_AUTHOR1_LIGHT           RGB_COLORDATA(255, 255, 195)
#define COL_AUTHOR2_DARK            RGB_COLORDATA(6,  70, 162)
#define COL_AUTHOR2_NORMAL          RGB_COLORDATA(216, 232, 255)
#define COL_AUTHOR2_LIGHT           RGB_COLORDATA(233, 242, 255)
#define COL_AUTHOR3_DARK            RGB_COLORDATA(87, 157,  28)
#define COL_AUTHOR3_NORMAL          RGB_COLORDATA(218, 248, 193)
#define COL_AUTHOR3_LIGHT           RGB_COLORDATA(226, 250, 207)
#define COL_AUTHOR4_DARK            RGB_COLORDATA(105,  43, 157)
#define COL_AUTHOR4_NORMAL          RGB_COLORDATA(228, 210, 245)
#define COL_AUTHOR4_LIGHT           RGB_COLORDATA(239, 228, 248)
#define COL_AUTHOR5_DARK            RGB_COLORDATA(197,   0,  11)
#define COL_AUTHOR5_NORMAL          RGB_COLORDATA(254, 205, 208)
#define COL_AUTHOR5_LIGHT           RGB_COLORDATA(255, 227, 229)
#define COL_AUTHOR6_DARK            RGB_COLORDATA(0, 128, 128)
#define COL_AUTHOR6_NORMAL          RGB_COLORDATA(210, 246, 246)
#define COL_AUTHOR6_LIGHT           RGB_COLORDATA(230, 250, 250)
#define COL_AUTHOR7_DARK            RGB_COLORDATA(140, 132,  0)
#define COL_AUTHOR7_NORMAL          RGB_COLORDATA(237, 252, 163)
#define COL_AUTHOR7_LIGHT           RGB_COLORDATA(242, 254, 181)
#define COL_AUTHOR8_DARK            RGB_COLORDATA(53,  85, 107)
#define COL_AUTHOR8_NORMAL          RGB_COLORDATA(211, 222, 232)
#define COL_AUTHOR8_LIGHT           RGB_COLORDATA(226, 234, 241)
#define COL_AUTHOR9_DARK            RGB_COLORDATA(209, 118,   0)
#define COL_AUTHOR9_NORMAL          RGB_COLORDATA(255, 226, 185)
#define COL_AUTHOR9_LIGHT           RGB_COLORDATA(255, 231, 199)

#define COLOR_CHANNEL_MERGE( _def_cDst, _def_cSrc, _def_cSrcTrans ) \
    ((sal_uInt8)((((long)(_def_cDst)-(_def_cSrc))*(_def_cSrcTrans)+(((_def_cSrc)<<8L)|(_def_cDst)))>>8L))

// Color

class TOOLS_DLLPUBLIC SAL_WARN_UNUSED Color
{
protected:
    ColorData           mnColor;

public:
                        Color() { mnColor = COL_BLACK; }
                        Color( ColorData nColor ) { mnColor = nColor; }
                        Color( sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue )
                            { mnColor = RGB_COLORDATA( nRed, nGreen, nBlue ); }
                        Color( sal_uInt8 nTransparency, sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue )
                            { mnColor = TRGB_COLORDATA( nTransparency, nRed, nGreen, nBlue ); }
                        // This ctor is defined in svtools, not tools!
                        Color( const ResId& rResId );

                        // constructor to create a tools-Color from ::basegfx::BColor
                        explicit Color(const ::basegfx::BColor& rBColor)
                        {
                            mnColor = RGB_COLORDATA(
                                sal_uInt8((rBColor.getRed() * 255.0) + 0.5),
                                sal_uInt8((rBColor.getGreen() * 255.0) + 0.5),
                                sal_uInt8((rBColor.getBlue() * 255.0) + 0.5));
                        }

    void                SetRed( sal_uInt8 nRed );
    sal_uInt8           GetRed() const { return COLORDATA_RED( mnColor ); }
    void                SetGreen( sal_uInt8 nGreen );
    sal_uInt8           GetGreen() const { return COLORDATA_GREEN( mnColor ); }
    void                SetBlue( sal_uInt8 nBlue );
    sal_uInt8           GetBlue() const { return COLORDATA_BLUE( mnColor ); }
    void                SetTransparency( sal_uInt8 nTransparency );
    sal_uInt8           GetTransparency() const { return COLORDATA_TRANSPARENCY( mnColor ); }

    void                SetColor( ColorData nColor ) { mnColor = nColor; }
    ColorData           GetColor() const { return mnColor; }
    ColorData           GetRGBColor() const { return COLORDATA_RGB( mnColor ); }

    sal_uInt8           GetColorError( const Color& rCompareColor ) const;

    sal_uInt8           GetLuminance() const;
    void                IncreaseLuminance( sal_uInt8 cLumInc );
    void                DecreaseLuminance( sal_uInt8 cLumDec );

    void                DecreaseContrast( sal_uInt8 cContDec );

    void                Invert();

    void                Merge( const Color& rMergeColor, sal_uInt8 cTransparency );

    sal_Bool            IsRGBEqual( const Color& rColor ) const;

    // comparison with luminance thresholds
    sal_Bool            IsDark()    const;
    sal_Bool            IsBright()  const;

    // color space conversion tools
    // the range for h/s/b is:
    // Hue: 0-360 degree
    // Saturation: 0-100 %
    // Brightness: 0-100 %
    static ColorData    HSBtoRGB( sal_uInt16 nHue, sal_uInt16 nSat, sal_uInt16 nBri );
    void                RGBtoHSB( sal_uInt16& nHue, sal_uInt16& nSat, sal_uInt16& nBri ) const;

    sal_Bool            operator==( const Color& rColor ) const
                        { return (mnColor == rColor.mnColor); }
    sal_Bool            operator!=( const Color& rColor ) const
                        { return !(Color::operator==( rColor )); }

    SvStream&           Read( SvStream& rIStm, sal_Bool bNewFormat = sal_True );
    SvStream&           Write( SvStream& rOStm, sal_Bool bNewFormat = sal_True );

    TOOLS_DLLPUBLIC friend SvStream&    operator>>( SvStream& rIStream, Color& rColor );
    TOOLS_DLLPUBLIC friend SvStream&    operator<<( SvStream& rOStream, const Color& rColor );

    // get ::basegfx::BColor from this color
    ::basegfx::BColor getBColor() const { return ::basegfx::BColor(GetRed() / 255.0, GetGreen() / 255.0, GetBlue() / 255.0); }
};

inline void Color::SetRed( sal_uInt8 nRed )
{
    mnColor &= 0xFF00FFFF;
    mnColor |= ((sal_uInt32)nRed)<<16;
}

inline void Color::SetGreen( sal_uInt8 nGreen )
{
    mnColor &= 0xFFFF00FF;
    mnColor |= ((sal_uInt16)nGreen)<<8;
}

inline void Color::SetBlue( sal_uInt8 nBlue )
{
    mnColor &= 0xFFFFFF00;
    mnColor |= nBlue;
}

inline void Color::SetTransparency( sal_uInt8 nTransparency )
{
    mnColor &= 0x00FFFFFF;
    mnColor |= ((sal_uInt32)nTransparency)<<24;
}

inline sal_Bool Color::IsRGBEqual( const Color& rColor ) const
{
    return (COLORDATA_RGB( mnColor ) == COLORDATA_RGB( rColor.mnColor ));
}

inline sal_uInt8 Color::GetLuminance() const
{
    return( (sal_uInt8) ( ( COLORDATA_BLUE( mnColor ) * 28UL +
                        COLORDATA_GREEN( mnColor ) * 151UL +
                        COLORDATA_RED( mnColor ) * 77UL ) >> 8UL ) );
}

inline void Color::Merge( const Color& rMergeColor, sal_uInt8 cTransparency )
{
    SetRed( COLOR_CHANNEL_MERGE( COLORDATA_RED( mnColor ), COLORDATA_RED( rMergeColor.mnColor ), cTransparency ) );
    SetGreen( COLOR_CHANNEL_MERGE( COLORDATA_GREEN( mnColor ), COLORDATA_GREEN( rMergeColor.mnColor ), cTransparency ) );
    SetBlue( COLOR_CHANNEL_MERGE( COLORDATA_BLUE( mnColor ), COLORDATA_BLUE( rMergeColor.mnColor ), cTransparency ) );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
