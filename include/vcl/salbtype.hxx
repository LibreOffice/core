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

#include <vcl/checksum.hxx>
#include <tools/color.hxx>
#include <vcl/dllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <vector>
#include <memory>

struct SalTwoRect;
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

    N32BitTcAbgr      = 0x00000800,
    N32BitTcArgb      = 0x00001000,
    N32BitTcBgra      = 0x00002000,
    N32BitTcRgba      = 0x00004000,
    N32BitTcMask      = 0x00008000,

    TopDown           = 0x00010000 // scanline adjustment
};
namespace o3tl {
    template<> struct typed_flags<ScanlineFormat> : is_typed_flags<ScanlineFormat, 0x0001fbff> {};
}
inline ScanlineFormat RemoveScanline(ScanlineFormat nFormat) { return nFormat & ~ScanlineFormat::TopDown; }


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


class VCL_DLLPUBLIC BitmapColor final
{
private:

    sal_uInt8 mcBlueOrIndex;
    sal_uInt8 mcGreen;
    sal_uInt8 mcRed;
    sal_uInt8 mcAlpha;

public:

    inline              BitmapColor();
    constexpr           BitmapColor( sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue, sal_uInt8 cAlpha = 0 );

    inline              BitmapColor( const Color& rColor );
    explicit inline     BitmapColor( sal_uInt8 cIndex );

    inline bool         operator==( const BitmapColor& rBitmapColor ) const;
    inline bool         operator!=( const BitmapColor& rBitmapColor ) const;

    inline sal_uInt8    GetRed() const;
    inline void         SetRed( sal_uInt8 cRed );

    inline sal_uInt8    GetGreen() const;
    inline void         SetGreen( sal_uInt8 cGreen );

    inline sal_uInt8    GetBlue() const;
    inline void         SetBlue( sal_uInt8 cBlue );

    inline sal_uInt8    GetIndex() const;
    inline void         SetIndex( sal_uInt8 cIndex );

    Color               GetColor() const;

    inline sal_uInt8    GetAlpha() const;
    inline void         SetAlpha( sal_uInt8 cAlpha );

    inline sal_uInt8    GetBlueOrIndex() const;

    inline BitmapColor& Invert();

    inline sal_uInt8    GetLuminance() const;

    inline BitmapColor& Merge( const BitmapColor& rColor, sal_uInt8 cTransparency );

    inline sal_uInt16   GetColorError( const BitmapColor& rBitmapColor ) const;
};

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator <<(std::basic_ostream<charT, traits>& rStream, const BitmapColor& rColor)
{
    return rStream << "#(" << std::hex << std::setfill ('0') << std::setw(2) << static_cast<int>(rColor.GetRed())
                           << std::setw(2) << static_cast<int>(rColor.GetGreen())
                           << std::setw(2) << static_cast<int>(rColor.GetBlueOrIndex())
                           << std::setw(2) << static_cast<int>(rColor.GetAlpha()) << ")";
}

class VCL_DLLPUBLIC BitmapPalette
{
    friend class SalBitmap;
    friend class BitmapAccess;

private:

    std::vector<BitmapColor> maBitmapColor;

public:

    SAL_DLLPRIVATE const BitmapColor* ImplGetColorBuffer() const
    {
        return maBitmapColor.data();
    }

    SAL_DLLPRIVATE BitmapColor* ImplGetColorBuffer()
    {
        return maBitmapColor.data();
    }

    BitmapChecksum GetChecksum() const
    {
        return vcl_get_checksum(0, maBitmapColor.data(), maBitmapColor.size() * sizeof(BitmapColor));
    }

public:

    BitmapPalette()
    {
    }

    BitmapPalette(sal_uInt16 nCount)
        : maBitmapColor(nCount)
    {
    }

    bool operator==( const BitmapPalette& rBitmapPalette ) const
    {
        return maBitmapColor == rBitmapPalette.maBitmapColor;
    }

    bool operator!=(const BitmapPalette& rBitmapPalette) const
    {
        return !( *this == rBitmapPalette );
    }

    bool operator!()
    {
        return maBitmapColor.empty();
    }

    sal_uInt16 GetEntryCount() const
    {
        return maBitmapColor.size();
    }

    void SetEntryCount(sal_uInt16 nCount)
    {
        maBitmapColor.resize(nCount);
    }

    const BitmapColor& operator[](sal_uInt16 nIndex) const
    {
        assert(nIndex < maBitmapColor.size() && "Palette index is out of range");
        return maBitmapColor[nIndex];
    }

    BitmapColor& operator[](sal_uInt16 nIndex)
    {
        assert(nIndex < maBitmapColor.size() && "Palette index is out of range");
        return maBitmapColor[nIndex];
    }

    sal_uInt16 GetBestIndex(const BitmapColor& rCol) const
    {
        sal_uInt16 nRetIndex = 0;

        if (!maBitmapColor.empty())
        {
            for (size_t j = 0; j < maBitmapColor.size(); ++j)
            {
                if (rCol == maBitmapColor[j])
                {
                    return j;
                }
            }

            sal_uInt16 nLastErr = SAL_MAX_UINT16;
            for (size_t i = 0; i < maBitmapColor.size(); ++i)
            {
                const sal_uInt16 nActErr = rCol.GetColorError(maBitmapColor[i]);
                if ( nActErr < nLastErr )
                {
                    nLastErr = nActErr;
                    nRetIndex = i;
                }
            }
        }

        return nRetIndex;
    }

    bool IsGreyPalette() const;
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

    inline void         GetColorFor8Bit( BitmapColor& rColor, const sal_uInt8* pPixel ) const;
    inline void         SetColorFor8Bit( const BitmapColor& rColor, sal_uInt8* pPixel ) const;

    inline void         GetColorFor16BitMSB( BitmapColor& rColor, const sal_uInt8* pPixel ) const;
    inline void         SetColorFor16BitMSB( const BitmapColor& rColor, sal_uInt8* pPixel ) const;
    inline void         GetColorFor16BitLSB( BitmapColor& rColor, const sal_uInt8* pPixel ) const;
    inline void         SetColorFor16BitLSB( const BitmapColor& rColor, sal_uInt8* pPixel ) const;

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

VCL_DLLPUBLIC std::unique_ptr<BitmapBuffer> StretchAndConvert(
    const BitmapBuffer& rSrcBuffer, const SalTwoRect& rTwoRect,
    ScanlineFormat nDstBitmapFormat, const BitmapPalette* pDstPal = nullptr, const ColorMask* pDstMask = nullptr );

inline BitmapColor::BitmapColor() :
            mcBlueOrIndex   (0),
            mcGreen         (0),
            mcRed           (0),
            mcAlpha         (0)
{
}

constexpr BitmapColor::BitmapColor(sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue, sal_uInt8 cAlpha) :
            mcBlueOrIndex   ( cBlue ),
            mcGreen         ( cGreen ),
            mcRed           ( cRed ),
            mcAlpha         ( cAlpha )
{
}

inline BitmapColor::BitmapColor( const Color& rColor ) :
            mcBlueOrIndex   ( rColor.GetBlue() ),
            mcGreen         ( rColor.GetGreen() ),
            mcRed           ( rColor.GetRed() ),
            mcAlpha         ( rColor.GetTransparency() )
{
}

inline BitmapColor::BitmapColor( sal_uInt8 cIndex ) :
            mcBlueOrIndex   ( cIndex ),
            mcGreen         ( 0 ),
            mcRed           ( 0 ),
            mcAlpha         ( 0 )
{
}

inline bool BitmapColor::operator==( const BitmapColor& rBitmapColor ) const
{
    return mcBlueOrIndex == rBitmapColor.mcBlueOrIndex &&
           mcGreen == rBitmapColor.mcGreen &&
           mcRed == rBitmapColor.mcRed &&
           mcAlpha == rBitmapColor.mcAlpha;
}

inline bool BitmapColor::operator!=( const BitmapColor& rBitmapColor ) const
{
    return !( *this == rBitmapColor );
}

inline sal_uInt8 BitmapColor::GetRed() const
{
    return mcRed;
}

inline void BitmapColor::SetRed( sal_uInt8 cRed )
{
    mcRed = cRed;
}

inline sal_uInt8 BitmapColor::GetGreen() const
{
    return mcGreen;
}

inline void BitmapColor::SetGreen( sal_uInt8 cGreen )
{
    mcGreen = cGreen;
}

inline sal_uInt8 BitmapColor::GetBlue() const
{
    return mcBlueOrIndex;
}

inline void BitmapColor::SetBlue( sal_uInt8 cBlue )
{
    mcBlueOrIndex = cBlue;
}

inline sal_uInt8 BitmapColor::GetIndex() const
{
    return mcBlueOrIndex;
}

inline void BitmapColor::SetIndex( sal_uInt8 cIndex )
{
    mcBlueOrIndex = cIndex;
}

inline Color BitmapColor::GetColor() const
{
    return Color(mcAlpha, mcRed, mcGreen, mcBlueOrIndex);
}

inline sal_uInt8 BitmapColor::GetAlpha() const
{
    return mcAlpha;
}

inline void BitmapColor::SetAlpha( sal_uInt8 cAlpha )
{
    mcAlpha = cAlpha;
}

inline sal_uInt8 BitmapColor::GetBlueOrIndex() const
{
    // #i47518# Yield a value regardless of mbIndex
    return mcBlueOrIndex;
}

inline BitmapColor& BitmapColor::Invert()
{
    mcBlueOrIndex = ~mcBlueOrIndex;
    mcGreen = ~mcGreen;
    mcRed = ~mcRed;

    return *this;
}

inline sal_uInt8 BitmapColor::GetLuminance() const
{
    return (static_cast<sal_uInt32>(mcBlueOrIndex) * 28
            + static_cast<sal_uInt32>(mcGreen) * 151
            + static_cast<sal_uInt32>(mcRed) * 77) >> 8;
}


inline BitmapColor& BitmapColor::Merge( const BitmapColor& rBitmapColor, sal_uInt8 cTransparency )
{
    mcBlueOrIndex = ColorChannelMerge( mcBlueOrIndex, rBitmapColor.mcBlueOrIndex, cTransparency );
    mcGreen = ColorChannelMerge( mcGreen, rBitmapColor.mcGreen, cTransparency );
    mcRed = ColorChannelMerge( mcRed, rBitmapColor.mcRed, cTransparency );

    return *this;
}


inline sal_uInt16 BitmapColor::GetColorError( const BitmapColor& rBitmapColor ) const
{
    return static_cast<sal_uInt16>(
        abs( static_cast<int>(mcBlueOrIndex) - static_cast<int>(rBitmapColor.mcBlueOrIndex) ) +
        abs( static_cast<int>(mcGreen) - static_cast<int>(rBitmapColor.mcGreen) ) +
        abs( static_cast<int>(mcRed) - static_cast<int>(rBitmapColor.mcRed) ) );
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
    *pPixel = static_cast<sal_uInt8>(COLOR_TO_MASK( rColor, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, 0/*nAlphaChannel*/ ));
}

inline void ColorMask::GetColorFor16BitMSB( BitmapColor& rColor, const sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = pPixel[ 1 ] | ( static_cast<sal_uInt32>(pPixel[ 0 ]) << 8 );

    MASK_TO_COLOR( nVal, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, rColor );
}

inline void ColorMask::SetColorFor16BitMSB( const BitmapColor& rColor, sal_uInt8* pPixel ) const
{
    const sal_uInt16 nVal = static_cast<sal_uInt16>(COLOR_TO_MASK( rColor, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, 0/*nAlphaChannel*/ ));

    pPixel[ 0 ] = static_cast<sal_uInt8>(nVal >> 8);
    pPixel[ 1 ] = static_cast<sal_uInt8>(nVal);
}

inline void ColorMask::GetColorFor16BitLSB( BitmapColor& rColor, const sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = pPixel[ 0 ] | ( static_cast<sal_uInt32>(pPixel[ 1 ]) << 8 );

    MASK_TO_COLOR( nVal, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, rColor );
}

inline void ColorMask::SetColorFor16BitLSB( const BitmapColor& rColor, sal_uInt8* pPixel ) const
{
    const sal_uInt16 nVal = static_cast<sal_uInt16>(COLOR_TO_MASK( rColor, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, 0/*nAlphaChannel*/ ));

    pPixel[ 0 ] = static_cast<sal_uInt8>(nVal);
    pPixel[ 1 ] = static_cast<sal_uInt8>(nVal >> 8);
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

inline void ColorMask::SetColorFor32Bit( const BitmapColor& rColor, sal_uInt8* pPixel ) const
{
    const sal_uInt32 nVal = COLOR_TO_MASK( rColor, maR.mnMask, maG.mnMask, maB.mnMask, maR.mnShift, maG.mnShift, maB.mnShift, 0/*nAlphaChannel*/ );
    pPixel[ 0 ] = static_cast<sal_uInt8>(nVal);
    pPixel[ 1 ] = static_cast<sal_uInt8>( nVal >> 8 );
    pPixel[ 2 ] = static_cast<sal_uInt8>( nVal >> 16 );
    pPixel[ 3 ] = static_cast<sal_uInt8>( nVal >> 24 );
}

#endif // INCLUDED_VCL_SALBTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
