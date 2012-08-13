/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <bmpfast.hxx>

#ifndef NO_OPTIMIZED_BITMAP_ACCESS

#include <tools/debug.hxx>
#include <vcl/bmpacc.hxx>

#define FAST_ARGB_BGRA

#include <stdlib.h>

typedef unsigned char PIXBYTE;

class BasePixelPtr
{
public:
            BasePixelPtr( PIXBYTE* p = NULL ) : mpPixel( p ) {}
    void    SetRawPtr( PIXBYTE* pRawPtr )               { mpPixel = pRawPtr; }
    PIXBYTE* GetRawPtr( void ) const                    { return mpPixel; }
    void    AddByteOffset( int nByteOffset )            { mpPixel += nByteOffset; }
    bool    operator<( const BasePixelPtr& rCmp ) const { return (mpPixel < rCmp.mpPixel); }

protected:
   PIXBYTE* mpPixel;
};

template <sal_uLong PIXFMT>
class TrueColorPixelPtr : public BasePixelPtr
{
public:
    PIXBYTE GetRed() const;
    PIXBYTE GetGreen() const;
    PIXBYTE GetBlue() const;
    PIXBYTE GetAlpha() const;

    void    SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const;
    void    SetAlpha( PIXBYTE a ) const;
    void    operator++(int);
};

// =======================================================================
// template specializations for truecolor pixel formats

template <>
class TrueColorPixelPtr<BMP_FORMAT_24BIT_TC_RGB> : public BasePixelPtr
{
public:
    void    operator++()       { mpPixel += 3; }

    PIXBYTE GetRed() const     { return mpPixel[0]; }
    PIXBYTE GetGreen() const   { return mpPixel[1]; }
    PIXBYTE GetBlue() const    { return mpPixel[2]; }
    PIXBYTE GetAlpha() const   { return 0; }
    void SetAlpha( PIXBYTE ) const {}

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[0] = r;
        mpPixel[1] = g;
        mpPixel[2] = b;
    }
};

template <>
class TrueColorPixelPtr<BMP_FORMAT_24BIT_TC_BGR> : public BasePixelPtr
{
public:
    void    operator++()        { mpPixel += 3; }

    PIXBYTE GetRed() const      { return mpPixel[2]; }
    PIXBYTE GetGreen() const    { return mpPixel[1]; }
    PIXBYTE GetBlue() const     { return mpPixel[0]; }
    PIXBYTE GetAlpha() const    { return 0; }
    void SetAlpha( PIXBYTE ) const  {}

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[0] = b;
        mpPixel[1] = g;
        mpPixel[2] = r;
    }
};

template <>
class TrueColorPixelPtr<BMP_FORMAT_32BIT_TC_ARGB> : public BasePixelPtr
{
public:
    void    operator++()        { mpPixel += 4; }

    PIXBYTE GetRed() const      { return mpPixel[1]; }
    PIXBYTE GetGreen() const    { return mpPixel[2]; }
    PIXBYTE GetBlue() const     { return mpPixel[3]; }
    PIXBYTE GetAlpha() const    { return mpPixel[0]; }
    void SetAlpha( PIXBYTE a ) const { mpPixel[0] = a; }

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[1] = r;
        mpPixel[2] = g;
        mpPixel[3] = b;
    }
};

template <>
class TrueColorPixelPtr<BMP_FORMAT_32BIT_TC_ABGR> : public BasePixelPtr
{
public:
    void    operator++()        { mpPixel += 4; }

    PIXBYTE GetRed() const      { return mpPixel[3]; }
    PIXBYTE GetGreen() const    { return mpPixel[2]; }
    PIXBYTE GetBlue() const     { return mpPixel[1]; }
    PIXBYTE GetAlpha() const    { return mpPixel[0]; }
    void SetAlpha( PIXBYTE a ) const { mpPixel[0] = a; }

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[1] = b;
        mpPixel[2] = g;
        mpPixel[3] = r;
    }
};

template <>
class TrueColorPixelPtr<BMP_FORMAT_32BIT_TC_RGBA> : public BasePixelPtr
{
public:
    void    operator++()            { mpPixel += 4; }

    PIXBYTE GetRed() const          { return mpPixel[0]; }
    PIXBYTE GetGreen() const        { return mpPixel[1]; }
    PIXBYTE GetBlue() const         { return mpPixel[2]; }
    PIXBYTE GetAlpha() const        { return mpPixel[3]; }
    void SetAlpha( PIXBYTE a ) const{ mpPixel[3] = a; }

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[0] = r;
        mpPixel[1] = g;
        mpPixel[2] = b;
    }
};

template <>
class TrueColorPixelPtr<BMP_FORMAT_32BIT_TC_BGRA> : public BasePixelPtr
{
public:
    void    operator++()            { mpPixel += 4; }

    PIXBYTE GetRed() const          { return mpPixel[2]; }
    PIXBYTE GetGreen() const        { return mpPixel[1]; }
    PIXBYTE GetBlue() const         { return mpPixel[0]; }
    PIXBYTE GetAlpha() const        { return mpPixel[3]; }
    void SetAlpha( PIXBYTE a ) const{ mpPixel[3] = a; }

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[0] = b;
        mpPixel[1] = g;
        mpPixel[2] = r;
    }
};

template <>
class TrueColorPixelPtr<BMP_FORMAT_16BIT_TC_MSB_MASK> : public BasePixelPtr
{
public:
    void    operator++()            { mpPixel += 2; }

    // TODO: non565-RGB
    PIXBYTE GetRed() const          { return (mpPixel[0] & 0xF8U); }
    PIXBYTE GetGreen() const        { return (mpPixel[0]<<5U) | ((mpPixel[1]>>3U)&28U); }
    PIXBYTE GetBlue() const         { return (mpPixel[1]<<3U); }
    PIXBYTE GetAlpha() const        { return 0; }
    void SetAlpha( PIXBYTE ) const  {}

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[0] = ((g >> 5U) & 7U) | (r & 0xF8U);
        mpPixel[1] = ((g & 28U)<< 3U) | (b >> 3U);
    }
};

template <>
class TrueColorPixelPtr<BMP_FORMAT_16BIT_TC_LSB_MASK> : public BasePixelPtr
{
public:
    void    operator++()            { mpPixel += 2; }

    // TODO: non565-RGB
    PIXBYTE GetRed() const          { return (mpPixel[1] & 0xF8U); }
    PIXBYTE GetGreen() const        { return (mpPixel[1]<<5U) | ((mpPixel[0]>>3U)&28U); }
    PIXBYTE GetBlue() const         { return (mpPixel[0]<<3U); }
    PIXBYTE GetAlpha() const        { return 0; }
    void SetAlpha( PIXBYTE ) const  {}

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[0] = ((g & 28U)<< 3U) | (b >> 3U);
        mpPixel[1] = ((g >> 5U) & 7U) | (r & 0xF8U);
    }
};

// -----------------------------------------------------------------------

template <>
class TrueColorPixelPtr<BMP_FORMAT_8BIT_TC_MASK> : public BasePixelPtr
{
public:
    void    operator++()                    { mpPixel += 1; }
    PIXBYTE GetAlpha() const                { return mpPixel[0]; }
    void    SetAlpha( PIXBYTE a ) const     { mpPixel[0] = a; }
    void    SetColor( PIXBYTE, PIXBYTE, PIXBYTE ) const {}
};

// TODO: for some reason many Alpha maps are BMP_FORMAT_8BIT_PAL
// they should be BMP_FORMAT_8BIT_TC_MASK
template <>
class TrueColorPixelPtr<BMP_FORMAT_8BIT_PAL>
: public TrueColorPixelPtr<BMP_FORMAT_8BIT_TC_MASK>
{};

// =======================================================================
// converting truecolor formats

template <sal_uLong SRCFMT, sal_uLong DSTFMT>
inline void ImplConvertPixel( const TrueColorPixelPtr<DSTFMT>& rDst,
    const TrueColorPixelPtr<SRCFMT>& rSrc )
{
    rDst.SetColor( rSrc.GetRed(), rSrc.GetGreen(), rSrc.GetBlue() );
    rDst.SetAlpha( rSrc.GetAlpha() );
}

// -----------------------------------------------------------------------

template <>
inline void ImplConvertPixel<BMP_FORMAT_16BIT_TC_LSB_MASK, BMP_FORMAT_16BIT_TC_MSB_MASK> (
    const TrueColorPixelPtr<BMP_FORMAT_16BIT_TC_MSB_MASK>& rDst,
    const TrueColorPixelPtr<BMP_FORMAT_16BIT_TC_LSB_MASK>& rSrc )
{
    // byte swapping
    const PIXBYTE* pSrc = rSrc.GetRawPtr();
    PIXBYTE* pDst = rDst.GetRawPtr();
    pDst[1] = pSrc[0];
    pDst[0] = pSrc[1];
}

// -----------------------------------------------------------------------

template <sal_uLong SRCFMT, sal_uLong DSTFMT>
inline void ImplConvertLine( const TrueColorPixelPtr<DSTFMT>& rDst,
    const TrueColorPixelPtr<SRCFMT>& rSrc, int nPixelCount )
{
    TrueColorPixelPtr<DSTFMT> aDst( rDst );
    TrueColorPixelPtr<SRCFMT> aSrc( rSrc );
    while( --nPixelCount >= 0 )
    {
        ImplConvertPixel( aDst, aSrc );
        ++aSrc;
        ++aDst;
    }
}

// =======================================================================
// alpha blending truecolor pixels

template <unsigned ALPHABITS, sal_uLong SRCFMT, sal_uLong DSTFMT>
inline void ImplBlendPixels( const TrueColorPixelPtr<DSTFMT>& rDst,
    const TrueColorPixelPtr<SRCFMT>& rSrc, unsigned nAlphaVal )
{
    if( !nAlphaVal )
        ImplConvertPixel( rDst, rSrc );
    else if( nAlphaVal != ~(~0 << ALPHABITS) )
    {
        static const unsigned nAlphaShift = (ALPHABITS > 8) ? 8 : ALPHABITS;
        if( ALPHABITS > nAlphaShift )
            nAlphaVal >>= ALPHABITS - nAlphaShift;

        int nR = rDst.GetRed();
        int nS = rSrc.GetRed();
        nR = nS + (((nR - nS) * nAlphaVal) >> nAlphaShift);

        int nG = rDst.GetGreen();
        nS = rSrc.GetGreen();
        nG = nS + (((nG - nS) * nAlphaVal) >> nAlphaShift);

        int nB = rDst.GetBlue();
        nS = rSrc.GetBlue();
        nB = nS + (((nB - nS) * nAlphaVal) >> nAlphaShift);

        rDst.SetColor( sal::static_int_cast<PIXBYTE>(nR),
                       sal::static_int_cast<PIXBYTE>(nG),
                       sal::static_int_cast<PIXBYTE>(nB) );
    }
}

// -----------------------------------------------------------------------

template <unsigned ALPHABITS, sal_uLong MASKFMT, sal_uLong SRCFMT, sal_uLong DSTFMT>
inline void ImplBlendLines( const TrueColorPixelPtr<DSTFMT>& rDst,
    const TrueColorPixelPtr<SRCFMT>& rSrc, const TrueColorPixelPtr<MASKFMT>& rMsk,
    int nPixelCount )
{
    TrueColorPixelPtr<MASKFMT> aMsk( rMsk );
    TrueColorPixelPtr<DSTFMT> aDst( rDst );
    TrueColorPixelPtr<SRCFMT> aSrc( rSrc );
    while( --nPixelCount >= 0 )
    {
        ImplBlendPixels<ALPHABITS>( aDst, aSrc, aMsk.GetAlpha() );
        ++aDst;
        ++aSrc;
        ++aMsk;
    }
}

// -----------------------------------------------------------------------

template <unsigned ALPHABITS, sal_uLong SRCFMT, sal_uLong DSTFMT>
inline void ImplBlendLines( const TrueColorPixelPtr<DSTFMT>& rDst,
    const TrueColorPixelPtr<SRCFMT>& rSrc, unsigned nAlphaVal,
    int nPixelCount )
{
    if( nAlphaVal == ~(~0 << ALPHABITS) )
        ImplConvertLine( rDst, rSrc, nPixelCount );
    else if( nAlphaVal )
    {
        TrueColorPixelPtr<SRCFMT> aSrc( rSrc );
        TrueColorPixelPtr<DSTFMT> aDst( rDst );
        while( --nPixelCount >= 0 )
        {
            ImplBlendPixels<ALPHABITS>( aDst, aSrc, nAlphaVal );
            ++aDst;
            ++aSrc;
        }
    }
}

// =======================================================================

static bool ImplCopyImage( BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer )
{
    const int nSrcLinestep = rSrcBuffer.mnScanlineSize;
    int nDstLinestep = rDstBuffer.mnScanlineSize;

    const PIXBYTE* pRawSrc = rSrcBuffer.mpBits;
    PIXBYTE* pRawDst = rDstBuffer.mpBits;

    // source and destination don't match upside down
    if( BMP_FORMAT_TOP_DOWN & (rSrcBuffer.mnFormat ^ rDstBuffer.mnFormat)  )
    {
        pRawDst += (rSrcBuffer.mnHeight - 1) * nDstLinestep;
        nDstLinestep = -rDstBuffer.mnScanlineSize;
    }
    else if( nSrcLinestep == nDstLinestep )
    {
        memcpy( pRawDst, pRawSrc, rSrcBuffer.mnHeight * nDstLinestep );
        return true;
    }

    int nByteWidth = nSrcLinestep;
    if( nByteWidth > rDstBuffer.mnScanlineSize )
        nByteWidth = rDstBuffer.mnScanlineSize;

    for( int y = rSrcBuffer.mnHeight; --y >= 0; )
    {
        memcpy( pRawDst, pRawSrc, nByteWidth );
        pRawSrc += nSrcLinestep;
        pRawDst += nDstLinestep;
    }

    return true;
}

// -----------------------------------------------------------------------

template <sal_uLong DSTFMT,sal_uLong SRCFMT>
bool ImplConvertToBitmap( TrueColorPixelPtr<SRCFMT>& rSrcLine,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer )
{
    // help the compiler to avoid instantiations of unneeded conversions
    DBG_ASSERT( SRCFMT != DSTFMT, "ImplConvertToBitmap into same format");
    if( SRCFMT == DSTFMT )
        return false;

    const int nSrcLinestep = rSrcBuffer.mnScanlineSize;
    int nDstLinestep = rDstBuffer.mnScanlineSize;

    TrueColorPixelPtr<DSTFMT> aDstLine; aDstLine.SetRawPtr( rDstBuffer.mpBits );

    // source and destination don't match upside down
    if( BMP_FORMAT_TOP_DOWN & (rSrcBuffer.mnFormat ^ rDstBuffer.mnFormat) )
    {
        aDstLine.AddByteOffset( (rSrcBuffer.mnHeight - 1) * nDstLinestep );
        nDstLinestep = -nDstLinestep;
    }

    for( int y = rSrcBuffer.mnHeight; --y >= 0; )
    {
        ImplConvertLine( aDstLine, rSrcLine, rSrcBuffer.mnWidth );
        rSrcLine.AddByteOffset( nSrcLinestep );
        aDstLine.AddByteOffset( nDstLinestep );
    }

    return true;
}

// -----------------------------------------------------------------------

template <sal_uLong SRCFMT>
inline bool ImplConvertFromBitmap( BitmapBuffer& rDst, const BitmapBuffer& rSrc )
{
    TrueColorPixelPtr<SRCFMT> aSrcType; aSrcType.SetRawPtr( rSrc.mpBits );

    // select the matching instantiation for the destination's bitmap format
    switch( rDst.mnFormat & ~BMP_FORMAT_TOP_DOWN )
    {
        case BMP_FORMAT_1BIT_MSB_PAL:
        case BMP_FORMAT_1BIT_LSB_PAL:
        case BMP_FORMAT_4BIT_MSN_PAL:
        case BMP_FORMAT_4BIT_LSN_PAL:
        case BMP_FORMAT_8BIT_PAL:
            break;

        case BMP_FORMAT_8BIT_TC_MASK:
//            return ImplConvertToBitmap<BMP_FORMAT_8BIT_TC_MASK>( aSrcType, rDst, rSrc );
        case BMP_FORMAT_24BIT_TC_MASK:
//            return ImplConvertToBitmap<BMP_FORMAT_24BIT_TC_MASK>( aSrcType, rDst, rSrc );
        case BMP_FORMAT_32BIT_TC_MASK:
//            return ImplConvertToBitmap<BMP_FORMAT_32BIT_TC_MASK>( aSrcType, rDst, rSrc );
            break;

        case BMP_FORMAT_16BIT_TC_MSB_MASK:
            return ImplConvertToBitmap<BMP_FORMAT_16BIT_TC_MSB_MASK>( aSrcType, rDst, rSrc );
        case BMP_FORMAT_16BIT_TC_LSB_MASK:
            return ImplConvertToBitmap<BMP_FORMAT_16BIT_TC_LSB_MASK>( aSrcType, rDst, rSrc );

        case BMP_FORMAT_24BIT_TC_BGR:
            return ImplConvertToBitmap<BMP_FORMAT_24BIT_TC_BGR>( aSrcType, rDst, rSrc );
        case BMP_FORMAT_24BIT_TC_RGB:
            return ImplConvertToBitmap<BMP_FORMAT_24BIT_TC_RGB>( aSrcType, rDst, rSrc );

        case BMP_FORMAT_32BIT_TC_ABGR:
            return ImplConvertToBitmap<BMP_FORMAT_32BIT_TC_ABGR>( aSrcType, rDst, rSrc );
#ifdef FAST_ARGB_BGRA
        case BMP_FORMAT_32BIT_TC_ARGB:
            return ImplConvertToBitmap<BMP_FORMAT_32BIT_TC_ARGB>( aSrcType, rDst, rSrc );
        case BMP_FORMAT_32BIT_TC_BGRA:
            return ImplConvertToBitmap<BMP_FORMAT_32BIT_TC_BGRA>( aSrcType, rDst, rSrc );
#endif
        case BMP_FORMAT_32BIT_TC_RGBA:
            return ImplConvertToBitmap<BMP_FORMAT_32BIT_TC_RGBA>( aSrcType, rDst, rSrc );
    }

#ifdef DEBUG
    static int nNotAccelerated = 0;
    if( rSrc.mnWidth * rSrc.mnHeight >= 4000 )
        if( ++nNotAccelerated == 100 )
        {
            int foo = 0; (void)foo; // so no warning is created when building on pro with debug
            DBG_WARNING2( "ImplConvertFromBitmap for not accelerated case (0x%04X->0x%04X)",
                rSrc.mnFormat, rDst.mnFormat );
        }
#endif

    return false;
}

// =======================================================================

// an universal stretching conversion is overkill in most common situations
// => performance benefits for speeding up the non-stretching cases
bool ImplFastBitmapConversion( BitmapBuffer& rDst, const BitmapBuffer& rSrc,
    const SalTwoRect& rTR )
{
    // horizontal mirroring not implemented yet
    if( rTR.mnDestWidth < 0 )
        return false;
    // vertical mirroring
    if( rTR.mnDestHeight < 0 )
        // TODO: rDst.mnFormat ^= BMP_FORMAT_TOP_DOWN;
        return false;

    // offseted conversion is not implemented yet
    if( rTR.mnSrcX || rTR.mnSrcY )
        return false;
    if( rTR.mnDestX || rTR.mnDestY )
        return false;

    // stretched conversion is not implemented yet
    if( rTR.mnDestWidth != rTR.mnSrcWidth )
        return false;
    if( rTR.mnDestHeight!= rTR.mnSrcHeight )
        return false;

    // check source image size
    if( rSrc.mnWidth < rTR.mnSrcX + rTR.mnSrcWidth )
        return false;
    if( rSrc.mnHeight < rTR.mnSrcY + rTR.mnSrcHeight )
        return false;

    // check dest image size
    if( rDst.mnWidth < rTR.mnDestX + rTR.mnDestWidth )
        return false;
    if( rDst.mnHeight < rTR.mnDestY + rTR.mnDestHeight )
        return false;

    const sal_uLong nSrcFormat = rSrc.mnFormat & ~BMP_FORMAT_TOP_DOWN;
    const sal_uLong nDstFormat = rDst.mnFormat & ~BMP_FORMAT_TOP_DOWN;

    // TODO: also implement conversions for 16bit colormasks with non-565 format
    if( nSrcFormat & (BMP_FORMAT_16BIT_TC_LSB_MASK | BMP_FORMAT_16BIT_TC_MSB_MASK) )
        if( rSrc.maColorMask.GetRedMask()  != 0xF800
        ||  rSrc.maColorMask.GetGreenMask()!= 0x07E0
        ||  rSrc.maColorMask.GetBlueMask() != 0x001F )
            return false;
    if( nDstFormat & (BMP_FORMAT_16BIT_TC_LSB_MASK | BMP_FORMAT_16BIT_TC_MSB_MASK) )
        if( rDst.maColorMask.GetRedMask()  != 0xF800
        ||  rDst.maColorMask.GetGreenMask()!= 0x07E0
        ||  rDst.maColorMask.GetBlueMask() != 0x001F )
            return false;

    // special handling of trivial cases
    if( nSrcFormat == nDstFormat )
    {
        // accelerated palette conversions not yet implemented
        if( rSrc.maPalette != rDst.maPalette )
            return false;
        return ImplCopyImage( rDst, rSrc );
    }

    // select the matching instantiation for the source's bitmap format
    switch( nSrcFormat )
    {
        case BMP_FORMAT_1BIT_MSB_PAL:
        case BMP_FORMAT_1BIT_LSB_PAL:
        case BMP_FORMAT_4BIT_MSN_PAL:
        case BMP_FORMAT_4BIT_LSN_PAL:
        case BMP_FORMAT_8BIT_PAL:
            break;

        case BMP_FORMAT_8BIT_TC_MASK:
//            return ImplConvertFromBitmap<BMP_FORMAT_8BIT_TC_MASK>( rDst, rSrc );
        case BMP_FORMAT_24BIT_TC_MASK:
//            return ImplConvertFromBitmap<BMP_FORMAT_24BIT_TC_MASK>( rDst, rSrc );
        case BMP_FORMAT_32BIT_TC_MASK:
//            return ImplConvertFromBitmap<BMP_FORMAT_32BIT_TC_MASK>( rDst, rSrc );
            break;

        case BMP_FORMAT_16BIT_TC_MSB_MASK:
            return ImplConvertFromBitmap<BMP_FORMAT_16BIT_TC_MSB_MASK>( rDst, rSrc );
        case BMP_FORMAT_16BIT_TC_LSB_MASK:
            return ImplConvertFromBitmap<BMP_FORMAT_16BIT_TC_LSB_MASK>( rDst, rSrc );

        case BMP_FORMAT_24BIT_TC_BGR:
            return ImplConvertFromBitmap<BMP_FORMAT_24BIT_TC_BGR>( rDst, rSrc );
        case BMP_FORMAT_24BIT_TC_RGB:
            return ImplConvertFromBitmap<BMP_FORMAT_24BIT_TC_RGB>( rDst, rSrc );

        case BMP_FORMAT_32BIT_TC_ABGR:
            return ImplConvertFromBitmap<BMP_FORMAT_32BIT_TC_ABGR>( rDst, rSrc );
#ifdef FAST_ARGB_BGRA
        case BMP_FORMAT_32BIT_TC_ARGB:
            return ImplConvertFromBitmap<BMP_FORMAT_32BIT_TC_ARGB>( rDst, rSrc );
        case BMP_FORMAT_32BIT_TC_BGRA:
            return ImplConvertFromBitmap<BMP_FORMAT_32BIT_TC_BGRA>( rDst, rSrc );
#endif
        case BMP_FORMAT_32BIT_TC_RGBA:
            return ImplConvertFromBitmap<BMP_FORMAT_32BIT_TC_RGBA>( rDst, rSrc );
    }

#ifdef DEBUG
    static int nNotAccelerated = 0;
    if( rSrc.mnWidth * rSrc.mnHeight >= 4000 )
    {
        if( ++nNotAccelerated == 100 )
        {
            int foo = 0; (void)foo; // so no warning is created when building on pro with debug
            DBG_WARNING2( "ImplFastBitmapConversion for not accelerated case (0x%04X->0x%04X)", rSrc.mnFormat, rDst.mnFormat );
        }
    }
#endif

    return false;
}

// =======================================================================

template <sal_uLong DSTFMT,sal_uLong SRCFMT> //,sal_uLong MSKFMT>
bool ImplBlendToBitmap( TrueColorPixelPtr<SRCFMT>& rSrcLine,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
{
    //DBG_ASSERT( rMskBuffer.mnFormat == MSKFMT, "FastBmp BlendImage: wrong MSKFMT" );
    DBG_ASSERT( rMskBuffer.mnFormat == BMP_FORMAT_8BIT_PAL, "FastBmp BlendImage: unusual MSKFMT" );

    const int nSrcLinestep = rSrcBuffer.mnScanlineSize;
    int nMskLinestep = rMskBuffer.mnScanlineSize;
    int nDstLinestep = rDstBuffer.mnScanlineSize;

    TrueColorPixelPtr<BMP_FORMAT_8BIT_PAL> aMskLine; aMskLine.SetRawPtr( rMskBuffer.mpBits );
    TrueColorPixelPtr<DSTFMT> aDstLine; aDstLine.SetRawPtr( rDstBuffer.mpBits );

    // special case for single line masks
    if( rMskBuffer.mnHeight == 1 )
        nMskLinestep = 0;

    // source and mask don't match: upside down
    if( (rSrcBuffer.mnFormat ^ rMskBuffer.mnFormat) & BMP_FORMAT_TOP_DOWN )
    {
        aMskLine.AddByteOffset( (rSrcBuffer.mnHeight - 1) * nMskLinestep );
        nMskLinestep = -nMskLinestep;
    }

    // source and destination don't match: upside down
    if( (rSrcBuffer.mnFormat ^ rDstBuffer.mnFormat) & BMP_FORMAT_TOP_DOWN )
    {
        aDstLine.AddByteOffset( (rSrcBuffer.mnHeight - 1) * nDstLinestep );
        nDstLinestep = -nDstLinestep;
    }

    for( int y = rSrcBuffer.mnHeight; --y >= 0; )
    {
        ImplBlendLines<8>( aDstLine, rSrcLine, aMskLine, rDstBuffer.mnWidth );
        aDstLine.AddByteOffset( nDstLinestep );
        rSrcLine.AddByteOffset( nSrcLinestep );
        aMskLine.AddByteOffset( nMskLinestep );
    }

    return true;
}

// some specializations to reduce the code size
template <>
inline bool ImplBlendToBitmap<BMP_FORMAT_24BIT_TC_BGR,BMP_FORMAT_24BIT_TC_BGR>(
    TrueColorPixelPtr<BMP_FORMAT_24BIT_TC_BGR>&,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
 {
    TrueColorPixelPtr<BMP_FORMAT_24BIT_TC_RGB> aSrcType; aSrcType.SetRawPtr( rSrcBuffer.mpBits );
    return ImplBlendToBitmap<BMP_FORMAT_24BIT_TC_RGB>( aSrcType, rDstBuffer, rSrcBuffer, rMskBuffer );
 }

template <>
inline bool ImplBlendToBitmap<BMP_FORMAT_32BIT_TC_ABGR,BMP_FORMAT_32BIT_TC_ABGR>(
    TrueColorPixelPtr<BMP_FORMAT_32BIT_TC_ABGR>&,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
 {
    TrueColorPixelPtr<BMP_FORMAT_32BIT_TC_ARGB> aSrcType; aSrcType.SetRawPtr( rSrcBuffer.mpBits );
    return ImplBlendToBitmap<BMP_FORMAT_32BIT_TC_ARGB>( aSrcType, rDstBuffer, rSrcBuffer, rMskBuffer );
 }

template <>
inline bool ImplBlendToBitmap<BMP_FORMAT_32BIT_TC_BGRA,BMP_FORMAT_32BIT_TC_BGRA>(
    TrueColorPixelPtr<BMP_FORMAT_32BIT_TC_BGRA>&,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
 {
    TrueColorPixelPtr<BMP_FORMAT_32BIT_TC_RGBA> aSrcType; aSrcType.SetRawPtr( rSrcBuffer.mpBits );
    return ImplBlendToBitmap<BMP_FORMAT_32BIT_TC_RGBA>( aSrcType, rDstBuffer, rSrcBuffer, rMskBuffer );
 }

// -----------------------------------------------------------------------

template <sal_uLong SRCFMT>
bool ImplBlendFromBitmap( BitmapBuffer& rDst, const BitmapBuffer& rSrc, const BitmapBuffer& rMsk )
{
    TrueColorPixelPtr<SRCFMT> aSrcType; aSrcType.SetRawPtr( rSrc.mpBits );

    // select the matching instantiation for the destination's bitmap format
    switch( rDst.mnFormat & ~BMP_FORMAT_TOP_DOWN )
    {
        case BMP_FORMAT_1BIT_MSB_PAL:
        case BMP_FORMAT_1BIT_LSB_PAL:
        case BMP_FORMAT_4BIT_MSN_PAL:
        case BMP_FORMAT_4BIT_LSN_PAL:
        case BMP_FORMAT_8BIT_PAL:
            break;

        case BMP_FORMAT_8BIT_TC_MASK:
//            return ImplBlendToBitmap<BMP_FORMAT_8BIT_TC_MASK>( aSrcType, rDst, rSrc, rMsk );
        case BMP_FORMAT_24BIT_TC_MASK:
//            return ImplBlendToBitmap<BMP_FORMAT_24BIT_TC_MASK>( aSrcType, rDst, rSrc, rMsk );
        case BMP_FORMAT_32BIT_TC_MASK:
//            return ImplBlendToBitmap<BMP_FORMAT_32BIT_TC_MASK>( aSrcType, rDst, rSrc, rMsk );
            break;

        case BMP_FORMAT_16BIT_TC_MSB_MASK:
            return ImplBlendToBitmap<BMP_FORMAT_16BIT_TC_MSB_MASK>( aSrcType, rDst, rSrc, rMsk );
        case BMP_FORMAT_16BIT_TC_LSB_MASK:
            return ImplBlendToBitmap<BMP_FORMAT_16BIT_TC_LSB_MASK>( aSrcType, rDst, rSrc, rMsk );

        case BMP_FORMAT_24BIT_TC_BGR:
            return ImplBlendToBitmap<BMP_FORMAT_24BIT_TC_BGR>( aSrcType, rDst, rSrc, rMsk );
        case BMP_FORMAT_24BIT_TC_RGB:
            return ImplBlendToBitmap<BMP_FORMAT_24BIT_TC_RGB>( aSrcType, rDst, rSrc, rMsk );

        case BMP_FORMAT_32BIT_TC_ABGR:
            return ImplBlendToBitmap<BMP_FORMAT_32BIT_TC_ABGR>( aSrcType, rDst, rSrc, rMsk );
#ifdef FAST_ARGB_BGRA
        case BMP_FORMAT_32BIT_TC_ARGB:
            return ImplBlendToBitmap<BMP_FORMAT_32BIT_TC_ARGB>( aSrcType, rDst, rSrc, rMsk );
        case BMP_FORMAT_32BIT_TC_BGRA:
            return ImplBlendToBitmap<BMP_FORMAT_32BIT_TC_BGRA>( aSrcType, rDst, rSrc, rMsk );
#endif
        case BMP_FORMAT_32BIT_TC_RGBA:
            return ImplBlendToBitmap<BMP_FORMAT_32BIT_TC_RGBA>( aSrcType, rDst, rSrc, rMsk );
    }

#ifdef DEBUG
    static int nNotAccelerated = 0;
    if( rSrc.mnWidth * rSrc.mnHeight >= 4000 )
        if( ++nNotAccelerated == 100 )
        {
            int foo = 0; (void)foo; // so no warning is created when building on pro with debug
            DBG_WARNING3( "ImplBlendFromBitmap for not accelerated case (0x%04X*0x%04X->0x%04X)",
                rSrc.mnFormat, rMsk.mnFormat, rDst.mnFormat );
        }
#endif

    return false;
}

// -----------------------------------------------------------------------

bool ImplFastBitmapBlending( BitmapWriteAccess& rDstWA,
    const BitmapReadAccess& rSrcRA, const BitmapReadAccess& rMskRA,
    const SalTwoRect& rTR )
{
    // accelerated blending of paletted bitmaps not implemented yet
    if( rSrcRA.HasPalette() )
        return false;
    if( rDstWA.HasPalette() )
        return false;
    // TODO: either get rid of mask's use of 8BIT_PAL or check the palette

    // horizontal mirroring not implemented yet
    if( rTR.mnDestWidth < 0 )
        return false;
    // vertical mirroring
    if( rTR.mnDestHeight < 0 )
        // TODO: rDst.mnFormat ^= BMP_FORMAT_TOP_DOWN;
        return false;

    // offseted blending is not implemented yet
    if( rTR.mnSrcX || rTR.mnSrcY )
        return false;
    if( rTR.mnDestX || rTR.mnDestY )
        return false;

    // stretched blending is not implemented yet
    if( rTR.mnDestWidth != rTR.mnSrcWidth )
        return false;
    if( rTR.mnDestHeight!= rTR.mnSrcHeight )
        return false;

    // check source image size
    if( rSrcRA.Width() < rTR.mnSrcX + rTR.mnSrcWidth )
        return false;
    if( rSrcRA.Height() < rTR.mnSrcY + rTR.mnSrcHeight )
        return false;

    // check mask image size
    if( rMskRA.Width() < rTR.mnSrcX + rTR.mnSrcWidth )
        return false;
    if( rMskRA.Height() < rTR.mnSrcY + rTR.mnSrcHeight )
        if( rMskRA.Height() != 1 )
            return false;

    // check dest image size
    if( rDstWA.Width() < rTR.mnDestX + rTR.mnDestWidth )
        return false;
    if( rDstWA.Height() < rTR.mnDestY + rTR.mnDestHeight )
        return false;

    BitmapBuffer& rDst = *rDstWA.ImplGetBitmapBuffer();
    const BitmapBuffer& rSrc = *rSrcRA.ImplGetBitmapBuffer();
    const BitmapBuffer& rMsk = *rMskRA.ImplGetBitmapBuffer();

    const sal_uLong nSrcFormat = rSrc.mnFormat & ~BMP_FORMAT_TOP_DOWN;
    const sal_uLong nDstFormat = rDst.mnFormat & ~BMP_FORMAT_TOP_DOWN;

    // accelerated conversions for 16bit colormasks with non-565 format are not yet implemented
    if( nSrcFormat & (BMP_FORMAT_16BIT_TC_LSB_MASK | BMP_FORMAT_16BIT_TC_MSB_MASK) )
        if( rSrc.maColorMask.GetRedMask()  != 0xF800
        ||  rSrc.maColorMask.GetGreenMask()!= 0x07E0
        ||  rSrc.maColorMask.GetBlueMask() != 0x001F)
            return false;
    if( nDstFormat & (BMP_FORMAT_16BIT_TC_LSB_MASK | BMP_FORMAT_16BIT_TC_MSB_MASK) )
        if( rDst.maColorMask.GetRedMask()  != 0xF800
        ||  rDst.maColorMask.GetGreenMask()!= 0x07E0
        ||  rDst.maColorMask.GetBlueMask() != 0x001F)
            return false;

    // select the matching instantiation for the source's bitmap format
    switch( nSrcFormat )
    {
        case BMP_FORMAT_1BIT_MSB_PAL:
        case BMP_FORMAT_1BIT_LSB_PAL:
        case BMP_FORMAT_4BIT_MSN_PAL:
        case BMP_FORMAT_4BIT_LSN_PAL:
        case BMP_FORMAT_8BIT_PAL:
            break;

        case BMP_FORMAT_8BIT_TC_MASK:
//            return ImplBlendFromBitmap<BMP_FORMAT_8BIT_TC_MASK>( rDst, rSrc );
        case BMP_FORMAT_24BIT_TC_MASK:
//            return ImplBlendFromBitmap<BMP_FORMAT_24BIT_TC_MASK>( rDst, rSrc );
        case BMP_FORMAT_32BIT_TC_MASK:
//            return ImplBlendFromBitmap<BMP_FORMAT_32BIT_TC_MASK>( rDst, rSrc );
            break;

        case BMP_FORMAT_16BIT_TC_MSB_MASK:
            return ImplBlendFromBitmap<BMP_FORMAT_16BIT_TC_MSB_MASK>( rDst, rSrc, rMsk );
        case BMP_FORMAT_16BIT_TC_LSB_MASK:
            return ImplBlendFromBitmap<BMP_FORMAT_16BIT_TC_LSB_MASK>( rDst, rSrc, rMsk );

        case BMP_FORMAT_24BIT_TC_BGR:
            return ImplBlendFromBitmap<BMP_FORMAT_24BIT_TC_BGR>( rDst, rSrc, rMsk );
        case BMP_FORMAT_24BIT_TC_RGB:
            return ImplBlendFromBitmap<BMP_FORMAT_24BIT_TC_RGB>( rDst, rSrc, rMsk );

        case BMP_FORMAT_32BIT_TC_ABGR:
            return ImplBlendFromBitmap<BMP_FORMAT_32BIT_TC_ABGR>( rDst, rSrc, rMsk );
#ifdef FAST_ARGB_BGRA
        case BMP_FORMAT_32BIT_TC_ARGB:
            return ImplBlendFromBitmap<BMP_FORMAT_32BIT_TC_ARGB>( rDst, rSrc, rMsk );
        case BMP_FORMAT_32BIT_TC_BGRA:
            return ImplBlendFromBitmap<BMP_FORMAT_32BIT_TC_BGRA>( rDst, rSrc, rMsk );
#endif
        case BMP_FORMAT_32BIT_TC_RGBA:
            return ImplBlendFromBitmap<BMP_FORMAT_32BIT_TC_RGBA>( rDst, rSrc, rMsk );
    }

#ifdef DEBUG
    static int nNotAccelerated = 0;
    if( rSrc.mnWidth * rSrc.mnHeight >= 4000 )
        if( ++nNotAccelerated == 100 )
        {
            int foo = 0; (void)foo; // so no warning is created when building on pro with debug
            DBG_WARNING3( "ImplFastBlend for not accelerated case (0x%04X*0x%04X->0x%04X)",
                rSrc.mnFormat, rMsk.mnFormat, rDst.mnFormat );
        }
#endif

    return false;
}

bool ImplFastEraseBitmap( BitmapBuffer& rDst, const BitmapColor& rColor )
{
    const sal_uLong nDstFormat = rDst.mnFormat & ~BMP_FORMAT_TOP_DOWN;

    // erasing a bitmap is often just a byte-wise memory fill
    bool bByteFill = true;
    sal_uInt8 nFillByte;

    switch( nDstFormat )
    {
        case BMP_FORMAT_1BIT_MSB_PAL:
        case BMP_FORMAT_1BIT_LSB_PAL:
            nFillByte = rColor.GetIndex();
            nFillByte = static_cast<sal_uInt8>( -(nFillByte & 1) ); // 0x00 or 0xFF
            break;
        case BMP_FORMAT_4BIT_MSN_PAL:
        case BMP_FORMAT_4BIT_LSN_PAL:
            nFillByte = rColor.GetIndex();
            nFillByte &= 0x0F;
            nFillByte |= (nFillByte << 4);
            break;
        case BMP_FORMAT_8BIT_PAL:
        case BMP_FORMAT_8BIT_TC_MASK:
            nFillByte = rColor.GetIndex();
            break;

        case BMP_FORMAT_24BIT_TC_MASK:
        case BMP_FORMAT_24BIT_TC_BGR:
        case BMP_FORMAT_24BIT_TC_RGB:
            nFillByte = rColor.GetRed();
            if( (nFillByte != rColor.GetGreen())
            ||  (nFillByte != rColor.GetBlue()) )
                bByteFill = false;
            break;

        default:
            bByteFill = false;
            nFillByte = 0x00;
            break;
    }

    if( bByteFill )
    {
        long nByteCount = rDst.mnHeight * rDst.mnScanlineSize;
        memset( rDst.mpBits, nFillByte, nByteCount );
        return true;
    }

    // TODO: handle other bitmap formats
    switch( nDstFormat )
    {
        case BMP_FORMAT_32BIT_TC_MASK:
        case BMP_FORMAT_16BIT_TC_MSB_MASK:
        case BMP_FORMAT_16BIT_TC_LSB_MASK:

        case BMP_FORMAT_24BIT_TC_BGR:
        case BMP_FORMAT_24BIT_TC_RGB:

        case BMP_FORMAT_32BIT_TC_ABGR:
#ifdef FAST_ARGB_BGRA
        case BMP_FORMAT_32BIT_TC_ARGB:
        case BMP_FORMAT_32BIT_TC_BGRA:
#endif
        case BMP_FORMAT_32BIT_TC_RGBA:
            break;

        default:
            break;
    }

    return false;
}

// =======================================================================

#else // NO_OPTIMIZED_BITMAP_ACCESS

bool ImplFastBitmapConversion( BitmapBuffer&, const BitmapBuffer& )
{
    return false;
}

bool ImplFastBitmapBlending( BitmapWriteAccess&,
    const BitmapReadAccess&, const BitmapReadAccess&,
    const Size&, const Point& )
{
    return false;
}

bool ImplFastEraseBitmap( BitmapBuffer&, const BitmapColor& )
{
    return false;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
