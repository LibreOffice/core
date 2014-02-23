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

#ifndef INCLUDED_VCL_SALBTYPE_HXX
#define INCLUDED_VCL_SALBTYPE_HXX
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <osl/endian.h>
#include <tools/debug.hxx>
#include <vcl/salgtype.hxx>
#include <tools/color.hxx>
#include <tools/helpers.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>

// ----------
// - Memory -
// ----------

typedef sal_uInt8*      HPBYTE;
typedef HPBYTE      Scanline;
typedef const sal_uInt8*    ConstHPBYTE;
typedef ConstHPBYTE ConstScanline;

// ------------------
// - Bitmap formats -
// ------------------

#define BMP_FORMAT_BOTTOM_UP                        0x00000000UL
#define BMP_FORMAT_TOP_DOWN                         0x80000000UL

#define BMP_FORMAT_1BIT_MSB_PAL                     0x00000001UL
#define BMP_FORMAT_1BIT_LSB_PAL                     0x00000002UL

#define BMP_FORMAT_4BIT_MSN_PAL                     0x00000004UL
#define BMP_FORMAT_4BIT_LSN_PAL                     0x00000008UL

#define BMP_FORMAT_8BIT_PAL                         0x00000010UL
#define BMP_FORMAT_8BIT_TC_MASK                     0x00000020UL

// #define BMP_FORMAT_16BIT_TC_MASK                 0x00000040UL

#define BMP_FORMAT_24BIT_TC_BGR                     0x00000080UL
#define BMP_FORMAT_24BIT_TC_RGB                     0x00000100UL
#define BMP_FORMAT_24BIT_TC_MASK                    0x00000200UL

#define BMP_FORMAT_32BIT_TC_ABGR                    0x00000400UL
#define BMP_FORMAT_32BIT_TC_ARGB                    0x00000800UL
#define BMP_FORMAT_32BIT_TC_BGRA                    0x00001000UL
#define BMP_FORMAT_32BIT_TC_RGBA                    0x00002000UL
#define BMP_FORMAT_32BIT_TC_MASK                    0x00004000UL

#define BMP_FORMAT_16BIT_TC_MSB_MASK                0x00008000UL
#define BMP_FORMAT_16BIT_TC_LSB_MASK                0x00010000UL

#define BMP_SCANLINE_ADJUSTMENT( Mac_nBmpFormat )   ( (Mac_nBmpFormat) & 0x80000000UL )
#define BMP_SCANLINE_FORMAT( Mac_nBmpFormat )       ( (Mac_nBmpFormat) & 0x7FFFFFFFUL )

// ------------------------------------------------------------------

#define MASK_TO_COLOR( d_nVal, d_RM, d_GM, d_BM, d_RS, d_GS, d_BS, d_Col )                          \
sal_uLong _def_cR = (sal_uInt8) ( d_RS < 0L ? ( (d_nVal) & d_RM ) << -d_RS : ( (d_nVal) & d_RM ) >> d_RS ); \
sal_uLong _def_cG = (sal_uInt8) ( d_GS < 0L ? ( (d_nVal) & d_GM ) << -d_GS : ( (d_nVal) & d_GM ) >> d_GS ); \
sal_uLong _def_cB = (sal_uInt8) ( d_BS < 0L ? ( (d_nVal) & d_BM ) << -d_BS : ( (d_nVal) & d_BM ) >> d_BS ); \
d_Col = BitmapColor( (sal_uInt8) ( _def_cR | ( ( _def_cR & mnROr ) >> mnROrShift ) ),                   \
                     (sal_uInt8) ( _def_cG | ( ( _def_cG & mnGOr ) >> mnGOrShift ) ),                   \
                     (sal_uInt8) ( _def_cB | ( ( _def_cB & mnBOr ) >> mnBOrShift ) ) );

// ------------------------------------------------------------------

#define COLOR_TO_MASK( d_rCol, d_RM, d_GM, d_BM, d_RS, d_GS, d_BS ) \
( ( ( ( d_RS < 0L ) ? ( (sal_uInt32) (d_rCol).GetRed() >> -d_RS ) :     \
    ( (sal_uInt32) (d_rCol).GetRed() << d_RS ) ) & d_RM ) |             \
  ( ( ( d_GS < 0L ) ? ( (sal_uInt32) (d_rCol).GetGreen() >> -d_GS ) :   \
    ( (sal_uInt32) (d_rCol).GetGreen() << d_GS ) ) & d_GM ) |           \
  ( ( ( d_BS < 0L ) ? ( (sal_uInt32) (d_rCol).GetBlue() >> -d_BS ) :    \
    ( (sal_uInt32) (d_rCol).GetBlue() << d_BS ) ) & d_BM ) )

// ---------------
// - BitmapColor -
// ---------------

class Color;

class VCL_DLLPUBLIC BitmapColor
{
private:

// ATTENTION:
//   Because the members of this class are accessed via memcpy,
//   you MUST NOT CHANGE the order of the members or the size of this class!
    sal_uInt8               mcBlueOrIndex;
    sal_uInt8               mcGreen;
    sal_uInt8               mcRed;
    sal_uInt8               mbIndex;

public:

    inline              BitmapColor();
    inline              BitmapColor( const BitmapColor& rBitmapColor );
    inline              BitmapColor( sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue );
    inline              BitmapColor( const Color& rColor );
    explicit inline     BitmapColor( sal_uInt8 cIndex );

    inline              ~BitmapColor() {};

    inline bool         operator==( const BitmapColor& rBitmapColor ) const;
    inline bool         operator!=( const BitmapColor& rBitmapColor ) const;
    inline BitmapColor& operator=( const BitmapColor& rBitmapColor );

    inline bool         IsIndex() const;

    inline sal_uInt8        GetRed() const;
    inline void         SetRed( sal_uInt8 cRed );

    inline sal_uInt8        GetGreen() const;
    inline void         SetGreen( sal_uInt8 cGreen );

    inline sal_uInt8        GetBlue() const;
    inline void         SetBlue( sal_uInt8 cBlue );

    inline sal_uInt8        GetIndex() const;
    inline void         SetIndex( sal_uInt8 cIndex );

    operator            Color() const;

    inline sal_uInt8         GetBlueOrIndex() const;

    inline BitmapColor& Invert();

    inline sal_uInt8        GetLuminance() const;
    inline BitmapColor& IncreaseLuminance( sal_uInt8 cGreyInc );
    inline BitmapColor& DecreaseLuminance( sal_uInt8 cGreyDec );

    inline BitmapColor& Merge( const BitmapColor& rColor, sal_uInt8 cTransparency );
    inline BitmapColor& Merge( sal_uInt8 cR, sal_uInt8 cG, sal_uInt8 cB, sal_uInt8 cTransparency );

    inline sal_uLong        GetColorError( const BitmapColor& rBitmapColor ) const;
};

// ---------------
// - BitmapPalette -
// ---------------

class Palette;

class VCL_DLLPUBLIC BitmapPalette
{
    friend class SalBitmap;
    friend class BitmapAccess;

private:

    BitmapColor*                mpBitmapColor;
    sal_uInt16                  mnCount;

public:

    SAL_DLLPRIVATE inline BitmapColor* ImplGetColorBuffer() const;


public:

    inline                      BitmapPalette();
    inline                      BitmapPalette( const BitmapPalette& rBitmapPalette );
    inline                      BitmapPalette( sal_uInt16 nCount );
    inline                      ~BitmapPalette();

    inline BitmapPalette&       operator=( const BitmapPalette& rBitmapPalette );
    inline bool             operator==( const BitmapPalette& rBitmapPalette ) const;
    inline bool             operator!=( const BitmapPalette& rBitmapPalette ) const;
    inline bool             operator!();

    inline sal_uInt16           GetEntryCount() const;
    inline void                 SetEntryCount( sal_uInt16 nCount );

    inline const BitmapColor&   operator[]( sal_uInt16 nIndex ) const;
    inline BitmapColor&         operator[]( sal_uInt16 nIndex );

    inline sal_uInt16           GetBestIndex( const BitmapColor& rCol ) const;
    bool                        IsGreyPalette() const;
};

// ---------------
// - ColorMask -
// ---------------

class VCL_DLLPUBLIC ColorMask
{
    sal_uLong               mnRMask;
    sal_uLong               mnGMask;
    sal_uLong               mnBMask;
    long                mnRShift;
    long                mnGShift;
    long                mnBShift;
    sal_uLong               mnROrShift;
    sal_uLong               mnGOrShift;
    sal_uLong               mnBOrShift;
    sal_uLong               mnROr;
    sal_uLong               mnGOr;
    sal_uLong               mnBOr;

    SAL_DLLPRIVATE inline long ImplCalcMaskShift( sal_uLong nMask, sal_uLong& rOr, sal_uLong& rOrShift ) const;

public:

    inline              ColorMask( sal_uLong nRedMask = 0UL, sal_uLong nGreenMask = 0UL, sal_uLong nBlueMask = 0UL );
    inline              ~ColorMask() {}

    inline sal_uLong        GetRedMask() const;
    inline sal_uLong        GetGreenMask() const;
    inline sal_uLong        GetBlueMask() const;

    inline void         GetColorFor8Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const;
    inline void         SetColorFor8Bit( const BitmapColor& rColor, HPBYTE pPixel ) const;

    inline void         GetColorFor16BitMSB( BitmapColor& rColor, ConstHPBYTE pPixel ) const;
    inline void         SetColorFor16BitMSB( const BitmapColor& rColor, HPBYTE pPixel ) const;
    inline void         GetColorFor16BitLSB( BitmapColor& rColor, ConstHPBYTE pPixel ) const;
    inline void         SetColorFor16BitLSB( const BitmapColor& rColor, HPBYTE pPixel ) const;

    inline void         GetColorFor24Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const;
    inline void         SetColorFor24Bit( const BitmapColor& rColor, HPBYTE pPixel ) const;

    inline void         GetColorFor32Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const;
    inline void         GetColorAndAlphaFor32Bit( BitmapColor& rColor, sal_uInt8& rAlpha, ConstHPBYTE pPixel ) const;
    inline void         SetColorFor32Bit( const BitmapColor& rColor, HPBYTE pPixel ) const;
};

// ---------------
// - BitmapBuffer -
// ---------------

struct VCL_DLLPUBLIC BitmapBuffer
{
    sal_uLong           mnFormat;
    long            mnWidth;
    long            mnHeight;
    long            mnScanlineSize;
    sal_uInt16          mnBitCount;
    ColorMask       maColorMask;
    BitmapPalette   maPalette;
    sal_uInt8*          mpBits;

                    BitmapBuffer(){}
                    ~BitmapBuffer() {}
};

// ---------------------
// - StretchAndConvert -
// ---------------------

VCL_DLLPUBLIC BitmapBuffer* StretchAndConvert(
    const BitmapBuffer& rSrcBuffer, const SalTwoRect& rTwoRect,
    sal_uLong nDstBitmapFormat, const BitmapPalette* pDstPal = NULL, const ColorMask* pDstMask = NULL );

// ------------------------------------------------------------------

inline BitmapColor::BitmapColor() :
            mcBlueOrIndex   ( 0 ),
            mcGreen         ( 0 ),
            mcRed           ( 0 ),
            mbIndex         ( false )
{
}

// ------------------------------------------------------------------

inline BitmapColor::BitmapColor( sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue ) :
            mcBlueOrIndex   ( cBlue ),
            mcGreen         ( cGreen ),
            mcRed           ( cRed ),
            mbIndex         ( false )
{
}

// ------------------------------------------------------------------

inline BitmapColor::BitmapColor( const BitmapColor& rBitmapColor ) :
            mcBlueOrIndex   ( rBitmapColor.mcBlueOrIndex ),
            mcGreen         ( rBitmapColor.mcGreen ),
            mcRed           ( rBitmapColor.mcRed ),
            mbIndex         ( rBitmapColor.mbIndex )
{
}

// ------------------------------------------------------------------

inline BitmapColor::BitmapColor( const Color& rColor ) :
            mcBlueOrIndex   ( rColor.GetBlue() ),
            mcGreen         ( rColor.GetGreen() ),
            mcRed           ( rColor.GetRed() ),
            mbIndex         ( 0 )
{
}

// ------------------------------------------------------------------

inline BitmapColor::BitmapColor( sal_uInt8 cIndex ) :
            mcBlueOrIndex   ( cIndex ),
            mcGreen         ( 0 ),
            mcRed           ( 0 ),
            mbIndex         ( true )
{
}

// ------------------------------------------------------------------

inline bool BitmapColor::operator==( const BitmapColor& rBitmapColor ) const
{
    return( ( mcBlueOrIndex == rBitmapColor.mcBlueOrIndex ) &&
            ( mbIndex ? rBitmapColor.mbIndex :
            ( mcGreen == rBitmapColor.mcGreen && mcRed == rBitmapColor.mcRed ) ) );
}

// ------------------------------------------------------------------

inline bool BitmapColor::operator!=( const BitmapColor& rBitmapColor ) const
{
    return !( *this == rBitmapColor );
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::operator=( const BitmapColor& rBitmapColor )
{
    mcBlueOrIndex = rBitmapColor.mcBlueOrIndex;
    mcGreen = rBitmapColor.mcGreen;
    mcRed = rBitmapColor.mcRed;
    mbIndex = rBitmapColor.mbIndex;

    return *this;
}

// ------------------------------------------------------------------

inline bool BitmapColor::IsIndex() const
{
    return mbIndex;
}

// ------------------------------------------------------------------

inline sal_uInt8 BitmapColor::GetRed() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return mcRed;
}

// ------------------------------------------------------------------

inline void BitmapColor::SetRed( sal_uInt8 cRed )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcRed = cRed;
}

// ------------------------------------------------------------------

inline sal_uInt8 BitmapColor::GetGreen() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return mcGreen;
}

// ------------------------------------------------------------------

inline void BitmapColor::SetGreen( sal_uInt8 cGreen )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcGreen = cGreen;
}

// ------------------------------------------------------------------

inline sal_uInt8 BitmapColor::GetBlue() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return mcBlueOrIndex;
}

// ------------------------------------------------------------------

inline void BitmapColor::SetBlue( sal_uInt8 cBlue )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = cBlue;
}

// ------------------------------------------------------------------

inline sal_uInt8 BitmapColor::GetIndex() const
{
    DBG_ASSERT( mbIndex, "Pixel represents color values!" );
    return mcBlueOrIndex;
}

// ------------------------------------------------------------------

inline void BitmapColor::SetIndex( sal_uInt8 cIndex )
{
    DBG_ASSERT( mbIndex, "Pixel represents color values!" );
    mcBlueOrIndex = cIndex;
}

// ------------------------------------------------------------------

inline BitmapColor::operator Color() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return Color( mcRed, mcGreen, mcBlueOrIndex );
}

// ------------------------------------------------------------------

inline sal_uInt8 BitmapColor::GetBlueOrIndex() const
{
    // #i47518# Yield a value regardless of mbIndex
    return mcBlueOrIndex;
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::Invert()
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = ~mcBlueOrIndex, mcGreen = ~mcGreen, mcRed = ~mcRed;

    return *this;
}

// ------------------------------------------------------------------

inline sal_uInt8 BitmapColor::GetLuminance() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return( (sal_uInt8) ( ( mcBlueOrIndex * 28UL + mcGreen * 151UL + mcRed * 77UL ) >> 8UL ) );
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::IncreaseLuminance( sal_uInt8 cGreyInc )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = (sal_uInt8) MinMax( (long) mcBlueOrIndex + cGreyInc, 0L, 255L );
    mcGreen = (sal_uInt8) MinMax( (long) mcGreen + cGreyInc, 0L, 255L );
    mcRed = (sal_uInt8) MinMax( (long) mcRed + cGreyInc, 0L, 255L );

    return *this;
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::DecreaseLuminance( sal_uInt8 cGreyDec )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = (sal_uInt8) MinMax( (long) mcBlueOrIndex - cGreyDec, 0L, 255L );
    mcGreen = (sal_uInt8) MinMax( (long) mcGreen - cGreyDec, 0L, 255L );
    mcRed = (sal_uInt8) MinMax( (long) mcRed - cGreyDec, 0L, 255L );

    return *this;
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::Merge( const BitmapColor& rBitmapColor, sal_uInt8 cTransparency )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    DBG_ASSERT( !rBitmapColor.mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = COLOR_CHANNEL_MERGE( mcBlueOrIndex, rBitmapColor.mcBlueOrIndex, cTransparency );
    mcGreen = COLOR_CHANNEL_MERGE( mcGreen, rBitmapColor.mcGreen, cTransparency );
    mcRed = COLOR_CHANNEL_MERGE( mcRed, rBitmapColor.mcRed, cTransparency );

    return *this;
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::Merge( sal_uInt8 cR, sal_uInt8 cG, sal_uInt8 cB, sal_uInt8 cTransparency )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = COLOR_CHANNEL_MERGE( mcBlueOrIndex, cB, cTransparency );
    mcGreen = COLOR_CHANNEL_MERGE( mcGreen, cG, cTransparency );
    mcRed = COLOR_CHANNEL_MERGE( mcRed, cR, cTransparency );

    return *this;
}

// ------------------------------------------------------------------

inline sal_uLong BitmapColor::GetColorError( const BitmapColor& rBitmapColor ) const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    DBG_ASSERT( !rBitmapColor.mbIndex, "Pixel represents index into colortable!" );
    return( (sal_uLong) ( labs( mcBlueOrIndex - rBitmapColor.mcBlueOrIndex ) +
                      labs( mcGreen - rBitmapColor.mcGreen ) +
                      labs( mcRed - rBitmapColor.mcRed ) ) );
}

// ------------------------------------------------------------------

inline BitmapPalette::BitmapPalette() :
            mpBitmapColor   ( NULL ),
            mnCount         ( 0 )
{
}

// ------------------------------------------------------------------

inline BitmapPalette::BitmapPalette( const BitmapPalette& rBitmapPalette ) :
            mnCount( rBitmapPalette.mnCount )
{
    if( mnCount )
    {
        const sal_uLong nSize = mnCount * sizeof( BitmapColor );
        mpBitmapColor = (BitmapColor*) new sal_uInt8[ nSize ];
        memcpy( mpBitmapColor, rBitmapPalette.mpBitmapColor, nSize );
    }
    else
        mpBitmapColor = NULL;
}

// ------------------------------------------------------------------

inline BitmapPalette::BitmapPalette( sal_uInt16 nCount ) :
            mnCount( nCount )
{
    if( mnCount )
    {
        const sal_uLong nSize = mnCount * sizeof( BitmapColor );
        mpBitmapColor = (BitmapColor*) new sal_uInt8[ nSize ];
        memset( mpBitmapColor, 0, nSize );
    }
    else
        mpBitmapColor = NULL;
}

// ------------------------------------------------------------------

inline BitmapPalette::~BitmapPalette()
{
    delete[] (sal_uInt8*) mpBitmapColor;
}

// ------------------------------------------------------------------

inline BitmapPalette& BitmapPalette::operator=( const BitmapPalette& rBitmapPalette )
{
    delete[] (sal_uInt8*) mpBitmapColor;
    mnCount = rBitmapPalette.mnCount;

    if( mnCount )
    {
        const sal_uLong nSize = mnCount * sizeof( BitmapColor );
        mpBitmapColor = (BitmapColor*) new sal_uInt8[ nSize ];
        memcpy( mpBitmapColor, rBitmapPalette.mpBitmapColor, nSize );
    }
    else
        mpBitmapColor = NULL;

    return *this;
}

// ------------------------------------------------------------------

inline bool BitmapPalette::operator==( const BitmapPalette& rBitmapPalette ) const
{
    bool bRet = false;

    if( rBitmapPalette.mnCount == mnCount )
    {
        bRet = true;

        for( sal_uInt16 i = 0; i < mnCount; i++ )
        {
            if( mpBitmapColor[ i ] != rBitmapPalette.mpBitmapColor[ i ] )
            {
                bRet = false;
                break;
            }
        }
    }

    return bRet;
}

// ------------------------------------------------------------------

inline bool BitmapPalette::operator!=( const BitmapPalette& rBitmapPalette ) const
{
    return !( *this == rBitmapPalette );
}

// ------------------------------------------------------------------

inline bool BitmapPalette::operator!()
{
    return( !mnCount || !mpBitmapColor );
}

// ------------------------------------------------------------------

inline sal_uInt16 BitmapPalette::GetEntryCount() const
{
    return mnCount;
}

// ------------------------------------------------------------------

inline void BitmapPalette::SetEntryCount( sal_uInt16 nCount )
{
    if( !nCount )
    {
        delete[] (sal_uInt8*) mpBitmapColor;
        mpBitmapColor = NULL;
        mnCount = 0;
    }
    else if( nCount != mnCount )
    {
        const sal_uLong nNewSize = nCount * sizeof( BitmapColor );
        const sal_uLong nMinSize = std::min( mnCount, nCount ) * sizeof( BitmapColor );
        sal_uInt8*      pNewColor = new sal_uInt8[ nNewSize ];

        if ( nMinSize && mpBitmapColor )
            memcpy( pNewColor, mpBitmapColor, nMinSize );
        delete[] (sal_uInt8*) mpBitmapColor;
        memset( pNewColor + nMinSize, 0, nNewSize - nMinSize );
        mpBitmapColor = (BitmapColor*) pNewColor;
        mnCount = nCount;
    }
}

// ------------------------------------------------------------------

inline const BitmapColor& BitmapPalette::operator[]( sal_uInt16 nIndex ) const
{
    DBG_ASSERT( nIndex < mnCount, "Palette index is out of range!" );
    return mpBitmapColor[ nIndex ];
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapPalette::operator[]( sal_uInt16 nIndex )
{
    DBG_ASSERT( nIndex < mnCount, "Palette index is out of range!" );
    return mpBitmapColor[ nIndex ];
}

// ------------------------------------------------------------------

inline BitmapColor* BitmapPalette::ImplGetColorBuffer() const
{
    DBG_ASSERT( mpBitmapColor, "No color buffer available!" );
    return mpBitmapColor;
}
// ------------------------------------------------------------------

inline sal_uInt16 BitmapPalette::GetBestIndex( const BitmapColor& rCol ) const
{
    sal_uInt16 nRetIndex = 0;

    if( mpBitmapColor && mnCount )
    {
        bool bFound = false;

        for( long j = 0L; ( j < mnCount ) && !bFound; j++ )
            if( rCol == mpBitmapColor[ j ] )
                nRetIndex = ( (sal_uInt16) j ), bFound = true;

        if( !bFound )
        {
            long nActErr, nLastErr = rCol.GetColorError( mpBitmapColor[ nRetIndex = mnCount - 1 ] );

            for( long i = nRetIndex - 1; i >= 0L; i-- )
                if ( ( nActErr = rCol.GetColorError( mpBitmapColor[ i ] ) ) < nLastErr )
                    nLastErr = nActErr, nRetIndex = (sal_uInt16) i;
        }
    }

    return nRetIndex;
}

// ------------------------------------------------------------------

inline ColorMask::ColorMask( sal_uLong nRedMask, sal_uLong nGreenMask, sal_uLong nBlueMask ) :
            mnRMask( nRedMask ),
            mnGMask( nGreenMask ),
            mnBMask( nBlueMask ),
            mnROrShift( 0L ),
            mnGOrShift( 0L ),
            mnBOrShift( 0L ),
            mnROr( 0L ),
            mnGOr( 0L ),
            mnBOr( 0L )
{
    mnRShift = ( mnRMask ? ImplCalcMaskShift( mnRMask, mnROr, mnROrShift ) : 0L );
    mnGShift = ( mnGMask ? ImplCalcMaskShift( mnGMask, mnGOr, mnGOrShift ) : 0L );
    mnBShift = ( mnBMask ? ImplCalcMaskShift( mnBMask, mnBOr, mnBOrShift ) : 0L );
}

// ------------------------------------------------------------------

inline long ColorMask::ImplCalcMaskShift( sal_uLong nMask, sal_uLong& rOr, sal_uLong& rOrShift ) const
{
    long    nShift;
    long    nRet;
    sal_uLong   nLen = 0UL;

    // from which bit starts the mask?
    for( nShift = 31L; ( nShift >= 0L ) && !( nMask & ( 1 << (sal_uLong) nShift ) ); nShift-- )
    {}

    nRet = nShift;

    // XXX determine number of bits set => walk right until null
    while( ( nShift >= 0L ) && ( nMask & ( 1 << (sal_uLong) nShift ) ) )
    {
        nShift--;
        nLen++;
    }

    rOrShift = 8L - nLen;
    rOr = (sal_uInt8) ( ( 0xffUL >> nLen ) << rOrShift );

    return( nRet -= 7 );
}

// ------------------------------------------------------------------

inline sal_uLong ColorMask::GetRedMask() const
{
    return mnRMask;
}

// ------------------------------------------------------------------

inline sal_uLong ColorMask::GetGreenMask() const
{
    return mnGMask;
}

// ------------------------------------------------------------------

inline sal_uLong ColorMask::GetBlueMask() const
{
    return mnBMask;
}

// ------------------------------------------------------------------

inline void ColorMask::GetColorFor8Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const
{
    const sal_uInt32 nVal = *pPixel;
    MASK_TO_COLOR( nVal, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift, rColor );
}

// ------------------------------------------------------------------

inline void ColorMask::SetColorFor8Bit( const BitmapColor& rColor, HPBYTE pPixel ) const
{
    *pPixel = (sal_uInt8) COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );
}

// ------------------------------------------------------------------

inline void ColorMask::GetColorFor16BitMSB( BitmapColor& rColor, ConstHPBYTE pPixel ) const
{
#ifdef OSL_BIGENDIAN
    const sal_uInt32 nVal = *(sal_uInt16*) pPixel;
#else
    const sal_uInt32 nVal = pPixel[ 1 ] | ( (sal_uInt32) pPixel[ 0 ] << 8UL );
#endif

    MASK_TO_COLOR( nVal, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift, rColor );
}

// ------------------------------------------------------------------

inline void ColorMask::SetColorFor16BitMSB( const BitmapColor& rColor, HPBYTE pPixel ) const
{
    const sal_uInt16 nVal = (sal_uInt16)COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );

#ifdef OSL_BIGENDIAN
    *(sal_uInt16*) pPixel = nVal;
#else
    pPixel[ 0 ] = (sal_uInt8)(nVal >> 8U);
    pPixel[ 1 ] = (sal_uInt8) nVal;
#endif
}

// ------------------------------------------------------------------

inline void ColorMask::GetColorFor16BitLSB( BitmapColor& rColor, ConstHPBYTE pPixel ) const
{
#ifdef OSL_BIGENDIAN
    const sal_uInt32 nVal = pPixel[ 0 ] | ( (sal_uInt32) pPixel[ 1 ] << 8UL );
#else
    const sal_uInt32 nVal = *(sal_uInt16*) pPixel;
#endif

    MASK_TO_COLOR( nVal, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift, rColor );
}

// ------------------------------------------------------------------

inline void ColorMask::SetColorFor16BitLSB( const BitmapColor& rColor, HPBYTE pPixel ) const
{
    const sal_uInt16 nVal = (sal_uInt16)COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );

#ifdef OSL_BIGENDIAN
    pPixel[ 0 ] = (sal_uInt8) nVal;
    pPixel[ 1 ] = (sal_uInt8)(nVal >> 8U);
#else
    *(sal_uInt16*) pPixel = nVal;
#endif
}


// ------------------------------------------------------------------

inline void ColorMask::GetColorFor24Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const
{
    const sal_uInt32 nVal = pPixel[ 0 ] | ( (sal_uInt32) pPixel[ 1 ] << 8UL ) | ( (sal_uInt32) pPixel[ 2 ] << 16UL );
    MASK_TO_COLOR( nVal, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift, rColor );
}

// ------------------------------------------------------------------

inline void ColorMask::SetColorFor24Bit( const BitmapColor& rColor, HPBYTE pPixel ) const
{
    const sal_uInt32 nVal = COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );
    pPixel[ 0 ] = (sal_uInt8) nVal; pPixel[ 1 ] = (sal_uInt8) ( nVal >> 8UL ); pPixel[ 2 ] = (sal_uInt8) ( nVal >> 16UL );
}

// ------------------------------------------------------------------

inline void ColorMask::GetColorFor32Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const
{
#ifdef OSL_BIGENDIAN
    const sal_uInt32 nVal = (sal_uInt32) pPixel[ 0 ] | ( (sal_uInt32) pPixel[ 1 ] << 8UL ) |
                        ( (sal_uInt32) pPixel[ 2 ] << 16UL ) | ( (sal_uInt32) pPixel[ 3 ] << 24UL );
#else
    const sal_uInt32 nVal = *(sal_uInt32*) pPixel;
#endif

    MASK_TO_COLOR( nVal, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift, rColor );
}

// ------------------------------------------------------------------

inline void ColorMask::GetColorAndAlphaFor32Bit( BitmapColor& rColor, sal_uInt8& rAlpha, ConstHPBYTE pPixel ) const
{
#ifdef OSL_BIGENDIAN
    const sal_uInt32 nVal = (sal_uInt32) pPixel[ 0 ] | ( (sal_uInt32) pPixel[ 1 ] << 8UL ) |
                        ( (sal_uInt32) pPixel[ 2 ] << 16UL ) | ( (sal_uInt32) pPixel[ 3 ] << 24UL );
#else
    const sal_uInt32 nVal = *(sal_uInt32*) pPixel;
#endif
    rAlpha = (sal_uInt8)(nVal >> 24);

    MASK_TO_COLOR( nVal, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift, rColor );
}

// ------------------------------------------------------------------

inline void ColorMask::SetColorFor32Bit( const BitmapColor& rColor, HPBYTE pPixel ) const
{
#ifdef OSL_BIGENDIAN
    const sal_uInt32 nVal = COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );
    pPixel[ 0 ] = (sal_uInt8) nVal; pPixel[ 1 ] = (sal_uInt8) ( nVal >> 8UL );
    pPixel[ 2 ] = (sal_uInt8) ( nVal >> 16UL ); pPixel[ 3 ] = (sal_uInt8) ( nVal >> 24UL );
#else
    *(sal_uInt32*) pPixel = COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );
#endif
}

#endif // INCLUDED_VCL_SALBTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
