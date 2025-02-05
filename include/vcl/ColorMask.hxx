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

#ifndef INCLUDED_VCL_COLORMASK_HXX
#define INCLUDED_VCL_COLORMASK_HXX

#include <vcl/dllapi.h>
#include <vcl/BitmapColor.hxx>

#define MASK_TO_COLOR( d_nVal, d_RM, d_GM, d_BM, d_RS, d_GS, d_BS, d_Col )                          \
const sal_uInt8 _def_cR = static_cast<sal_uInt8>( d_RS < 0 ? ( (d_nVal) & d_RM ) << -d_RS : ( (d_nVal) & d_RM ) >> d_RS ); \
const sal_uInt8 _def_cG = static_cast<sal_uInt8>( d_GS < 0 ? ( (d_nVal) & d_GM ) << -d_GS : ( (d_nVal) & d_GM ) >> d_GS ); \
const sal_uInt8 _def_cB = static_cast<sal_uInt8>( d_BS < 0 ? ( (d_nVal) & d_BM ) << -d_BS : ( (d_nVal) & d_BM ) >> d_BS ); \
d_Col = BitmapColor( static_cast<sal_uInt8>( _def_cR | ( ( _def_cR & maR.mnOr ) >> maR.mnOrShift ) ),                   \
                     static_cast<sal_uInt8>( _def_cG | ( ( _def_cG & maG.mnOr ) >> maG.mnOrShift ) ),                   \
                     static_cast<sal_uInt8>( _def_cB | ( ( _def_cB & maB.mnOr ) >> maB.mnOrShift ) ) );


#define COLOR_TO_MASK( d_rCol, d_RM, d_GM, d_BM, d_RS, d_GS, d_BS, d_ALPHA ) \
( ( ( ( d_RS < 0 ) ? ( static_cast<sal_uInt32>((d_rCol).GetRed()) >> -d_RS ) :     \
    ( static_cast<sal_uInt32>((d_rCol).GetRed()) << d_RS ) ) & d_RM ) |             \
  ( ( ( d_GS < 0 ) ? ( static_cast<sal_uInt32>((d_rCol).GetGreen()) >> -d_GS ) :   \
    ( static_cast<sal_uInt32>((d_rCol).GetGreen()) << d_GS ) ) & d_GM ) |           \
  ( ( ( d_BS < 0 ) ? ( static_cast<sal_uInt32>((d_rCol).GetBlue()) >> -d_BS ) :    \
    ( static_cast<sal_uInt32>((d_rCol).GetBlue()) << d_BS ) ) & d_BM ) | \
    d_ALPHA )


struct VCL_DLLPUBLIC ColorMaskElement
{
    sal_uInt32              mnMask;
    int                     mnShift;
    int                     mnOrShift;
    sal_uInt8               mnOr;
    explicit ColorMaskElement(sal_uInt32 nMask = 0)
        : mnMask(nMask)
        , mnShift(0)
        , mnOrShift(0)
        , mnOr(0)
    {
    }
    bool CalcMaskShift()
    {
        if (mnMask == 0)
            return true;

        // from which bit starts the mask?
        int nShift = 31;

        while( nShift >= 0 && !( mnMask & ( 1 << nShift ) ) )
            --nShift;

        mnShift = nShift - 7;
        int nLen = 0;

        // XXX determine number of bits set => walk right until null
        while( nShift >= 0 && ( mnMask & ( 1 << nShift ) ) )
        {
            nShift--;
            nLen++;
        }

        if (nLen > 8) // mask length must be 8 bits or less
            return false;

        mnOrShift = 8 - nLen;
        mnOr = static_cast<sal_uInt8>( ( 0xFF >> nLen ) << mnOrShift );

        return true;
    }
};

class VCL_DLLPUBLIC ColorMask
{
    ColorMaskElement        maR;
    ColorMaskElement        maG;
    ColorMaskElement        maB;

public:

    ColorMask(const ColorMaskElement& rRedMask = ColorMaskElement(),
              const ColorMaskElement& rGreenMask = ColorMaskElement(),
              const ColorMaskElement& rBlueMask = ColorMaskElement())
        : maR(rRedMask)
        , maG(rGreenMask)
        , maB(rBlueMask)
    {
    }

    inline sal_uInt32   GetRedMask() const;
    inline sal_uInt32   GetGreenMask() const;
    inline sal_uInt32   GetBlueMask() const;

    inline void         GetColorFor16BitLSB( BitmapColor& rColor, const sal_uInt8* pPixel ) const;

    inline void         GetColorFor32Bit( BitmapColor& rColor, const sal_uInt8* pPixel ) const;
    inline void         GetColorAndAlphaFor32Bit( BitmapColor& rColor, sal_uInt8& rAlpha, const sal_uInt8* pPixel ) const;
};

inline sal_uInt32 ColorMask::GetRedMask() const
{
    return maR.mnMask;
}

inline sal_uInt32 ColorMask::GetGreenMask() const
{
    return maG.mnMask;
}

inline sal_uInt32 ColorMask::GetBlueMask() const
{
    return maB.mnMask;
}

inline void ColorMask::GetColorFor16BitLSB( BitmapColor& rColor, const sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = pPixel[ 0 ] | ( static_cast<sal_uInt32>(pPixel[ 1 ]) << 8 );

    MASK_TO_COLOR( nVal, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, rColor );
}

inline void ColorMask::GetColorFor32Bit( BitmapColor& rColor, const sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = static_cast<sal_uInt32>(pPixel[ 0 ]) | ( static_cast<sal_uInt32>(pPixel[ 1 ]) << 8 ) |
                        ( static_cast<sal_uInt32>(pPixel[ 2 ]) << 16 ) | ( static_cast<sal_uInt32>(pPixel[ 3 ]) << 24 );

    MASK_TO_COLOR( nVal, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, rColor );
}

inline void ColorMask::GetColorAndAlphaFor32Bit( BitmapColor& rColor, sal_uInt8& rAlpha, const sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = static_cast<sal_uInt32>(pPixel[ 0 ]) | ( static_cast<sal_uInt32>(pPixel[ 1 ]) << 8 ) |
                        ( static_cast<sal_uInt32>(pPixel[ 2 ]) << 16 ) | ( static_cast<sal_uInt32>(pPixel[ 3 ]) << 24 );
    rAlpha = static_cast<sal_uInt8>(nVal >> 24);

    MASK_TO_COLOR( nVal, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, rColor );
}

#endif // INCLUDED_VCL_COLORMASK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
