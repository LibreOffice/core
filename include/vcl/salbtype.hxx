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

#include <string.h>
#include <stdlib.h>
#include <tools/debug.hxx>
#include <vcl/checksum.hxx>
#include <vcl/salgtype.hxx>
#include <tools/color.hxx>
#include <tools/helpers.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <o3tl/typed_flags_set.hxx>

typedef sal_uInt8*        Scanline;
typedef const sal_uInt8*  ConstScanline;

enum class ScanlineFormat {
    NONE              = 0x00000000,

    N1BitMsbPal       = 0x00000001,
    N1BitLsbPal       = 0x00000002,

    N4BitMsnPal       = 0x00000004,
    N4BitLsnPal       = 0x00000008,

    N8BitPal          = 0x00000010,
    N8BitTcMask       = 0x00000020,

    N16BitTcMsbMask   = 0x00000040,
    N16BitTcLsbMask   = 0x00000080,

    N24BitTcBgr       = 0x00000100,
    N24BitTcRgb       = 0x00000200,
    N24BitTcMask      = 0x00000400,

    N32BitTcAbgr      = 0x00000800,
    N32BitTcArgb      = 0x00001000,
    N32BitTcBgra      = 0x00002000,
    N32BitTcRgba      = 0x00004000,
    N32BitTcMask      = 0x00008000,

    TopDown           = 0x00010000 // scanline adjustment
};
namespace o3tl {
    template<> struct typed_flags<ScanlineFormat> : is_typed_flags<ScanlineFormat, 0x0001ffff> {};
}
inline ScanlineFormat RemoveScanline(ScanlineFormat nFormat) { return nFormat & ~ScanlineFormat::TopDown; }


#define MASK_TO_COLOR( d_nVal, d_RM, d_GM, d_BM, d_RS, d_GS, d_BS, d_Col )                          \
const sal_uInt8 _def_cR = static_cast<sal_uInt8>( d_RS < 0 ? ( (d_nVal) & d_RM ) << -d_RS : ( (d_nVal) & d_RM ) >> d_RS ); \
const sal_uInt8 _def_cG = static_cast<sal_uInt8>( d_GS < 0 ? ( (d_nVal) & d_GM ) << -d_GS : ( (d_nVal) & d_GM ) >> d_GS ); \
const sal_uInt8 _def_cB = static_cast<sal_uInt8>( d_BS < 0 ? ( (d_nVal) & d_BM ) << -d_BS : ( (d_nVal) & d_BM ) >> d_BS ); \
d_Col = BitmapColor( (sal_uInt8) ( _def_cR | ( ( _def_cR & maR.mnOr ) >> maR.mnOrShift ) ),                   \
                     (sal_uInt8) ( _def_cG | ( ( _def_cG & maG.mnOr ) >> maG.mnOrShift ) ),                   \
                     (sal_uInt8) ( _def_cB | ( ( _def_cB & maB.mnOr ) >> maB.mnOrShift ) ) );


#define COLOR_TO_MASK( d_rCol, d_RM, d_GM, d_BM, d_RS, d_GS, d_BS, d_ALPHA ) \
( ( ( ( d_RS < 0L ) ? ( (sal_uInt32) (d_rCol).GetRed() >> -d_RS ) :     \
    ( (sal_uInt32) (d_rCol).GetRed() << d_RS ) ) & d_RM ) |             \
  ( ( ( d_GS < 0L ) ? ( (sal_uInt32) (d_rCol).GetGreen() >> -d_GS ) :   \
    ( (sal_uInt32) (d_rCol).GetGreen() << d_GS ) ) & d_GM ) |           \
  ( ( ( d_BS < 0L ) ? ( (sal_uInt32) (d_rCol).GetBlue() >> -d_BS ) :    \
    ( (sal_uInt32) (d_rCol).GetBlue() << d_BS ) ) & d_BM ) | \
    d_ALPHA )


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
    sal_uInt8               mbIndex; // should be bool, but see above warning

public:

    inline              BitmapColor();
    inline              BitmapColor( sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue );
    inline              BitmapColor( const Color& rColor );
    explicit inline     BitmapColor( sal_uInt8 cIndex );

    inline bool         operator==( const BitmapColor& rBitmapColor ) const;
    inline bool         operator!=( const BitmapColor& rBitmapColor ) const;

    inline bool         IsIndex() const;

    inline sal_uInt8    GetRed() const;
    inline void         SetRed( sal_uInt8 cRed );

    inline sal_uInt8    GetGreen() const;
    inline void         SetGreen( sal_uInt8 cGreen );

    inline sal_uInt8    GetBlue() const;
    inline void         SetBlue( sal_uInt8 cBlue );

    inline sal_uInt8    GetIndex() const;
    inline void         SetIndex( sal_uInt8 cIndex );

    operator            Color() const;

    inline sal_uInt8    GetBlueOrIndex() const;

    inline BitmapColor& Invert();

    inline sal_uInt8    GetLuminance() const;

    inline BitmapColor& Merge( const BitmapColor& rColor, sal_uInt8 cTransparency );

    inline sal_uInt16   GetColorError( const BitmapColor& rBitmapColor ) const;
};

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

    BitmapChecksum GetChecksum() const
    {
        return vcl_get_checksum(0, mpBitmapColor, mnCount * sizeof(BitmapColor));
    }

public:

    inline                      BitmapPalette();
    inline                      BitmapPalette( const BitmapPalette& rBitmapPalette );
    inline                      BitmapPalette( sal_uInt16 nCount );
    inline                      ~BitmapPalette();

    inline BitmapPalette&       operator=( const BitmapPalette& rBitmapPalette );
    inline bool                 operator==( const BitmapPalette& rBitmapPalette ) const;
    inline bool                 operator!=( const BitmapPalette& rBitmapPalette ) const;
    inline bool                 operator!();

    inline sal_uInt16           GetEntryCount() const;
    inline void                 SetEntryCount( sal_uInt16 nCount );

    inline const BitmapColor&   operator[]( sal_uInt16 nIndex ) const;
    inline BitmapColor&         operator[]( sal_uInt16 nIndex );

    inline sal_uInt16           GetBestIndex( const BitmapColor& rCol ) const;
    bool                        IsGreyPalette() const;
};

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
    sal_uInt32              mnAlphaChannel;

public:

    ColorMask(const ColorMaskElement& rRedMask = ColorMaskElement(),
              const ColorMaskElement& rGreenMask = ColorMaskElement(),
              const ColorMaskElement& rBlueMask = ColorMaskElement())
        : maR(rRedMask)
        , maG(rGreenMask)
        , maB(rBlueMask)
        , mnAlphaChannel(0)
    {
    }

    inline sal_uInt32   GetRedMask() const;
    inline sal_uInt32   GetGreenMask() const;
    inline sal_uInt32   GetBlueMask() const;

    inline void         GetColorFor8Bit( BitmapColor& rColor, const sal_uInt8* pPixel ) const;
    inline void         SetColorFor8Bit( const BitmapColor& rColor, sal_uInt8* pPixel ) const;

    inline void         GetColorFor16BitMSB( BitmapColor& rColor, const sal_uInt8* pPixel ) const;
    inline void         SetColorFor16BitMSB( const BitmapColor& rColor, sal_uInt8* pPixel ) const;
    inline void         GetColorFor16BitLSB( BitmapColor& rColor, const sal_uInt8* pPixel ) const;
    inline void         SetColorFor16BitLSB( const BitmapColor& rColor, sal_uInt8* pPixel ) const;

    inline void         GetColorFor24Bit( BitmapColor& rColor, const sal_uInt8* pPixel ) const;
    inline void         SetColorFor24Bit( const BitmapColor& rColor, sal_uInt8* pPixel ) const;

    inline void         GetColorFor32Bit( BitmapColor& rColor, const sal_uInt8* pPixel ) const;
    inline void         GetColorAndAlphaFor32Bit( BitmapColor& rColor, sal_uInt8& rAlpha, const sal_uInt8* pPixel ) const;
    inline void         SetColorFor32Bit( const BitmapColor& rColor, sal_uInt8* pPixel ) const;
};

struct VCL_DLLPUBLIC BitmapBuffer
{
    ScanlineFormat       mnFormat;
    long            mnWidth;
    long            mnHeight;
    long            mnScanlineSize;
    sal_uInt16      mnBitCount;
    ColorMask       maColorMask;
    BitmapPalette   maPalette;
    sal_uInt8*      mpBits;
};

enum class BitmapAccessMode
{
    Info,
    Read,
    Write
};

VCL_DLLPUBLIC BitmapBuffer* StretchAndConvert(
    const BitmapBuffer& rSrcBuffer, const SalTwoRect& rTwoRect,
    ScanlineFormat nDstBitmapFormat, const BitmapPalette* pDstPal = nullptr, const ColorMask* pDstMask = nullptr );

inline BitmapColor::BitmapColor() :
            mcBlueOrIndex   ( 0 ),
            mcGreen         ( 0 ),
            mcRed           ( 0 ),
            mbIndex         ( sal_uInt8(false) )
{
}

inline BitmapColor::BitmapColor( sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue ) :
            mcBlueOrIndex   ( cBlue ),
            mcGreen         ( cGreen ),
            mcRed           ( cRed ),
            mbIndex         ( sal_uInt8(false) )
{
}

inline BitmapColor::BitmapColor( const Color& rColor ) :
            mcBlueOrIndex   ( rColor.GetBlue() ),
            mcGreen         ( rColor.GetGreen() ),
            mcRed           ( rColor.GetRed() ),
            mbIndex         ( sal_uInt8(false) )
{
}

inline BitmapColor::BitmapColor( sal_uInt8 cIndex ) :
            mcBlueOrIndex   ( cIndex ),
            mcGreen         ( 0 ),
            mcRed           ( 0 ),
            mbIndex         ( sal_uInt8(true) )
{
}

inline bool BitmapColor::operator==( const BitmapColor& rBitmapColor ) const
{
    return( ( mcBlueOrIndex == rBitmapColor.mcBlueOrIndex ) &&
            ( mbIndex ? bool(rBitmapColor.mbIndex) :
            ( mcGreen == rBitmapColor.mcGreen && mcRed == rBitmapColor.mcRed ) ) );
}

inline bool BitmapColor::operator!=( const BitmapColor& rBitmapColor ) const
{
    return !( *this == rBitmapColor );
}

inline bool BitmapColor::IsIndex() const
{
    return mbIndex;
}

inline sal_uInt8 BitmapColor::GetRed() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return mcRed;
}

inline void BitmapColor::SetRed( sal_uInt8 cRed )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcRed = cRed;
}

inline sal_uInt8 BitmapColor::GetGreen() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return mcGreen;
}

inline void BitmapColor::SetGreen( sal_uInt8 cGreen )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcGreen = cGreen;
}

inline sal_uInt8 BitmapColor::GetBlue() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return mcBlueOrIndex;
}

inline void BitmapColor::SetBlue( sal_uInt8 cBlue )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = cBlue;
}

inline sal_uInt8 BitmapColor::GetIndex() const
{
    DBG_ASSERT( mbIndex, "Pixel represents color values!" );
    return mcBlueOrIndex;
}

inline void BitmapColor::SetIndex( sal_uInt8 cIndex )
{
    DBG_ASSERT( mbIndex, "Pixel represents color values!" );
    mcBlueOrIndex = cIndex;
}

inline BitmapColor::operator Color() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return Color( mcRed, mcGreen, mcBlueOrIndex );
}

inline sal_uInt8 BitmapColor::GetBlueOrIndex() const
{
    // #i47518# Yield a value regardless of mbIndex
    return mcBlueOrIndex;
}

inline BitmapColor& BitmapColor::Invert()
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = ~mcBlueOrIndex;
    mcGreen = ~mcGreen;
    mcRed = ~mcRed;

    return *this;
}

inline sal_uInt8 BitmapColor::GetLuminance() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return (static_cast<unsigned long>(mcBlueOrIndex) * 28UL + static_cast<unsigned long>(mcGreen) * 151UL + static_cast<unsigned long>(mcRed) * 77UL) >> 8;
}


inline BitmapColor& BitmapColor::Merge( const BitmapColor& rBitmapColor, sal_uInt8 cTransparency )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    DBG_ASSERT( !rBitmapColor.mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = COLOR_CHANNEL_MERGE( mcBlueOrIndex, rBitmapColor.mcBlueOrIndex, cTransparency );
    mcGreen = COLOR_CHANNEL_MERGE( mcGreen, rBitmapColor.mcGreen, cTransparency );
    mcRed = COLOR_CHANNEL_MERGE( mcRed, rBitmapColor.mcRed, cTransparency );

    return *this;
}


inline sal_uInt16 BitmapColor::GetColorError( const BitmapColor& rBitmapColor ) const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    DBG_ASSERT( !rBitmapColor.mbIndex, "Pixel represents index into colortable!" );
    return static_cast<sal_uInt16>(
        abs( static_cast<int>(mcBlueOrIndex) - static_cast<int>(rBitmapColor.mcBlueOrIndex) ) +
        abs( static_cast<int>(mcGreen) - static_cast<int>(rBitmapColor.mcGreen) ) +
        abs( static_cast<int>(mcRed) - static_cast<int>(rBitmapColor.mcRed) ) );
}

inline BitmapPalette::BitmapPalette() :
            mpBitmapColor   ( nullptr ),
            mnCount         ( 0 )
{
}

inline BitmapPalette::BitmapPalette( const BitmapPalette& rBitmapPalette ) :
            mnCount( rBitmapPalette.mnCount )
{
    if( mnCount )
    {
        const size_t nSize = mnCount * sizeof( BitmapColor );
        mpBitmapColor = reinterpret_cast<BitmapColor*>(new sal_uInt8[ nSize ]);
        memcpy( mpBitmapColor, rBitmapPalette.mpBitmapColor, nSize );
    }
    else
        mpBitmapColor = nullptr;
}

inline BitmapPalette::BitmapPalette( sal_uInt16 nCount ) :
            mnCount( nCount )
{
    if( mnCount )
    {
        const size_t nSize = mnCount * sizeof( BitmapColor );
        mpBitmapColor = reinterpret_cast<BitmapColor*>(new sal_uInt8[ nSize ]);
        memset( mpBitmapColor, 0, nSize );
    }
    else
        mpBitmapColor = nullptr;
}

inline BitmapPalette::~BitmapPalette()
{
    delete[] reinterpret_cast<sal_uInt8*>(mpBitmapColor);
}

inline BitmapPalette& BitmapPalette::operator=( const BitmapPalette& rBitmapPalette )
{
    delete[] reinterpret_cast<sal_uInt8*>(mpBitmapColor);
    mnCount = rBitmapPalette.mnCount;

    if( mnCount )
    {
        const size_t nSize = mnCount * sizeof( BitmapColor );
        mpBitmapColor = reinterpret_cast<BitmapColor*>(new sal_uInt8[ nSize ]);
        memcpy( mpBitmapColor, rBitmapPalette.mpBitmapColor, nSize );
    }
    else
        mpBitmapColor = nullptr;

    return *this;
}

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

inline bool BitmapPalette::operator!=( const BitmapPalette& rBitmapPalette ) const
{
    return !( *this == rBitmapPalette );
}

inline bool BitmapPalette::operator!()
{
    return( !mnCount || !mpBitmapColor );
}

inline sal_uInt16 BitmapPalette::GetEntryCount() const
{
    return mnCount;
}

inline void BitmapPalette::SetEntryCount( sal_uInt16 nCount )
{
    if( !nCount )
    {
        delete[] reinterpret_cast<sal_uInt8*>(mpBitmapColor);
        mpBitmapColor = nullptr;
        mnCount = 0;
    }
    else if( nCount != mnCount )
    {
        const size_t nNewSize = nCount * sizeof( BitmapColor );
        const size_t nMinSize = std::min( mnCount, nCount ) * sizeof( BitmapColor );
        sal_uInt8*      pNewColor = new sal_uInt8[ nNewSize ];

        if ( nMinSize && mpBitmapColor )
            memcpy( pNewColor, mpBitmapColor, nMinSize );
        delete[] reinterpret_cast<sal_uInt8*>(mpBitmapColor);
        memset( pNewColor + nMinSize, 0, nNewSize - nMinSize );
        mpBitmapColor = reinterpret_cast<BitmapColor*>(pNewColor);
        mnCount = nCount;
    }
}

inline const BitmapColor& BitmapPalette::operator[]( sal_uInt16 nIndex ) const
{
    DBG_ASSERT( nIndex < mnCount, "Palette index is out of range!" );
    return mpBitmapColor[ nIndex ];
}

inline BitmapColor& BitmapPalette::operator[]( sal_uInt16 nIndex )
{
    DBG_ASSERT( nIndex < mnCount, "Palette index is out of range!" );
    return mpBitmapColor[ nIndex ];
}

inline BitmapColor* BitmapPalette::ImplGetColorBuffer() const
{
    DBG_ASSERT( mpBitmapColor, "No color buffer available!" );
    return mpBitmapColor;
}

inline sal_uInt16 BitmapPalette::GetBestIndex( const BitmapColor& rCol ) const
{
    sal_uInt16 nRetIndex = 0;

    if( mpBitmapColor && mnCount )
    {
        for( sal_uInt16 j = 0; j < mnCount; ++j )
            if( rCol == mpBitmapColor[ j ] )
            {
                return j;
            }

        sal_uInt16 nLastErr = SAL_MAX_UINT16;
        for( sal_uInt16 i = 0; i < mnCount; ++i )
        {
            const sal_uInt16 nActErr = rCol.GetColorError( mpBitmapColor[ i ] );
            if ( nActErr < nLastErr )
            {
                nLastErr = nActErr;
                nRetIndex = i;
            }
        }
    }

    return nRetIndex;
}

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

inline void ColorMask::GetColorFor8Bit( BitmapColor& rColor, const sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = *pPixel;
    MASK_TO_COLOR( nVal, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maR.mnShift, rColor );
}

inline void ColorMask::SetColorFor8Bit( const BitmapColor& rColor, sal_uInt8* pPixel ) const
{
    *pPixel = (sal_uInt8) COLOR_TO_MASK( rColor, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, mnAlphaChannel );
}

inline void ColorMask::GetColorFor16BitMSB( BitmapColor& rColor, const sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = pPixel[ 1 ] | ( (sal_uInt32) pPixel[ 0 ] << 8UL );

    MASK_TO_COLOR( nVal, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, rColor );
}

inline void ColorMask::SetColorFor16BitMSB( const BitmapColor& rColor, sal_uInt8* pPixel ) const
{
    const sal_uInt16 nVal = (sal_uInt16)COLOR_TO_MASK( rColor, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, mnAlphaChannel );

    pPixel[ 0 ] = (sal_uInt8)(nVal >> 8U);
    pPixel[ 1 ] = (sal_uInt8) nVal;
}

inline void ColorMask::GetColorFor16BitLSB( BitmapColor& rColor, const sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = pPixel[ 0 ] | ( (sal_uInt32) pPixel[ 1 ] << 8UL );

    MASK_TO_COLOR( nVal, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, rColor );
}

inline void ColorMask::SetColorFor16BitLSB( const BitmapColor& rColor, sal_uInt8* pPixel ) const
{
    const sal_uInt16 nVal = (sal_uInt16)COLOR_TO_MASK( rColor, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, mnAlphaChannel );

    pPixel[ 0 ] = (sal_uInt8) nVal;
    pPixel[ 1 ] = (sal_uInt8)(nVal >> 8U);
}

inline void ColorMask::GetColorFor24Bit( BitmapColor& rColor, const sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = pPixel[ 0 ] | ( (sal_uInt32) pPixel[ 1 ] << 8UL ) | ( (sal_uInt32) pPixel[ 2 ] << 16UL );
    MASK_TO_COLOR( nVal, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, rColor );
}

inline void ColorMask::SetColorFor24Bit( const BitmapColor& rColor, sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = COLOR_TO_MASK( rColor, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, mnAlphaChannel );
    pPixel[ 0 ] = (sal_uInt8) nVal;
    pPixel[ 1 ] = (sal_uInt8) ( nVal >> 8UL );
    pPixel[ 2 ] = (sal_uInt8) ( nVal >> 16UL );
}

inline void ColorMask::GetColorFor32Bit( BitmapColor& rColor, const sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = (sal_uInt32) pPixel[ 0 ] | ( (sal_uInt32) pPixel[ 1 ] << 8UL ) |
                        ( (sal_uInt32) pPixel[ 2 ] << 16UL ) | ( (sal_uInt32) pPixel[ 3 ] << 24UL );

    MASK_TO_COLOR( nVal, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, rColor );
}

inline void ColorMask::GetColorAndAlphaFor32Bit( BitmapColor& rColor, sal_uInt8& rAlpha, const sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = (sal_uInt32) pPixel[ 0 ] | ( (sal_uInt32) pPixel[ 1 ] << 8UL ) |
                        ( (sal_uInt32) pPixel[ 2 ] << 16UL ) | ( (sal_uInt32) pPixel[ 3 ] << 24UL );
    rAlpha = (sal_uInt8)(nVal >> 24);

    MASK_TO_COLOR( nVal, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, rColor );
}

inline void ColorMask::SetColorFor32Bit( const BitmapColor& rColor, sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = COLOR_TO_MASK( rColor, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, mnAlphaChannel );
    pPixel[ 0 ] = (sal_uInt8) nVal;
    pPixel[ 1 ] = (sal_uInt8) ( nVal >> 8UL );
    pPixel[ 2 ] = (sal_uInt8) ( nVal >> 16UL );
    pPixel[ 3 ] = (sal_uInt8) ( nVal >> 24UL );
}

#endif // INCLUDED_VCL_SALBTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
