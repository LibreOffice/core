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

#include <bmpfast.hxx>
#include <tools/debug.hxx>
#include <vcl/bitmapaccess.hxx>

#define FAST_ARGB_BGRA

#include <stdlib.h>

typedef unsigned char PIXBYTE;

class BasePixelPtr
{
public:
    explicit BasePixelPtr( PIXBYTE* p = nullptr ) : mpPixel( p ) {}
    void    SetRawPtr( PIXBYTE* pRawPtr )               { mpPixel = pRawPtr; }
    PIXBYTE* GetRawPtr() const                    { return mpPixel; }
    void    AddByteOffset( int nByteOffset )            { mpPixel += nByteOffset; }

protected:
   PIXBYTE* mpPixel;
};

template <ScanlineFormat PIXFMT>
class TrueColorPixelPtr : public BasePixelPtr
{
public:
    PIXBYTE GetRed() const;
    PIXBYTE GetGreen() const;
    PIXBYTE GetBlue() const;
    PIXBYTE GetAlpha() const;

    void    SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const;
    void    SetAlpha( PIXBYTE a ) const;
};

// template specializations for truecolor pixel formats
template <>
class TrueColorPixelPtr<ScanlineFormat::N24BitTcRgb> : public BasePixelPtr
{
public:
    void    operator++()       { mpPixel += 3; }

    PIXBYTE GetRed() const     { return mpPixel[0]; }
    PIXBYTE GetGreen() const   { return mpPixel[1]; }
    PIXBYTE GetBlue() const    { return mpPixel[2]; }
    static PIXBYTE GetAlpha()  { return 0; }
    static void SetAlpha( PIXBYTE ) {}

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[0] = r;
        mpPixel[1] = g;
        mpPixel[2] = b;
    }
};

template <>
class TrueColorPixelPtr<ScanlineFormat::N24BitTcBgr> : public BasePixelPtr
{
public:
    void    operator++()        { mpPixel += 3; }

    PIXBYTE GetRed() const      { return mpPixel[2]; }
    PIXBYTE GetGreen() const    { return mpPixel[1]; }
    PIXBYTE GetBlue() const     { return mpPixel[0]; }
    static PIXBYTE GetAlpha()   { return 0; }
    static void SetAlpha( PIXBYTE ) {}

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[0] = b;
        mpPixel[1] = g;
        mpPixel[2] = r;
    }
};

template <>
class TrueColorPixelPtr<ScanlineFormat::N32BitTcArgb> : public BasePixelPtr
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
class TrueColorPixelPtr<ScanlineFormat::N32BitTcAbgr> : public BasePixelPtr
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
class TrueColorPixelPtr<ScanlineFormat::N32BitTcRgba> : public BasePixelPtr
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
class TrueColorPixelPtr<ScanlineFormat::N32BitTcBgra> : public BasePixelPtr
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
class TrueColorPixelPtr<ScanlineFormat::N16BitTcMsbMask> : public BasePixelPtr
{
public:
    void    operator++()            { mpPixel += 2; }

    // TODO: non565-RGB
    PIXBYTE GetRed() const          { return (mpPixel[0] & 0xF8U); }
    PIXBYTE GetGreen() const        { return (mpPixel[0]<<5U) | ((mpPixel[1]>>3U)&28U); }
    PIXBYTE GetBlue() const         { return (mpPixel[1]<<3U); }
    static PIXBYTE GetAlpha()       { return 0; }
    static void SetAlpha( PIXBYTE ) {}

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[0] = ((g >> 5U) & 7U) | (r & 0xF8U);
        mpPixel[1] = ((g & 28U)<< 3U) | (b >> 3U);
    }
};

template <>
class TrueColorPixelPtr<ScanlineFormat::N16BitTcLsbMask> : public BasePixelPtr
{
public:
    void    operator++()            { mpPixel += 2; }

    // TODO: non565-RGB
    PIXBYTE GetRed() const          { return (mpPixel[1] & 0xF8U); }
    PIXBYTE GetGreen() const        { return (mpPixel[1]<<5U) | ((mpPixel[0]>>3U)&28U); }
    PIXBYTE GetBlue() const         { return (mpPixel[0]<<3U); }
    static PIXBYTE GetAlpha()       { return 0; }
    static void SetAlpha( PIXBYTE ) {}

    void SetColor( PIXBYTE r, PIXBYTE g, PIXBYTE b ) const
    {
        mpPixel[0] = ((g & 28U)<< 3U) | (b >> 3U);
        mpPixel[1] = ((g >> 5U) & 7U) | (r & 0xF8U);
    }
};

template <>
class TrueColorPixelPtr<ScanlineFormat::N8BitTcMask> : public BasePixelPtr
{
public:
    void    operator++()                    { mpPixel += 1; }
    PIXBYTE GetAlpha() const                { return mpPixel[0]; }
    void    SetAlpha( PIXBYTE a ) const     { mpPixel[0] = a; }
};

// TODO: for some reason many Alpha maps are ScanlineFormat::N8BitPal
// they should be ScanlineFormat::N8BitTcMask
template <>
class TrueColorPixelPtr<ScanlineFormat::N8BitPal>
: public TrueColorPixelPtr<ScanlineFormat::N8BitTcMask>
{};

// converting truecolor formats
template <ScanlineFormat SRCFMT, ScanlineFormat DSTFMT>
inline void ImplConvertPixel( const TrueColorPixelPtr<DSTFMT>& rDst,
    const TrueColorPixelPtr<SRCFMT>& rSrc )
{
    rDst.SetColor( rSrc.GetRed(), rSrc.GetGreen(), rSrc.GetBlue() );
    rDst.SetAlpha( rSrc.GetAlpha() );
}

template <>
inline void ImplConvertPixel<ScanlineFormat::N16BitTcLsbMask, ScanlineFormat::N16BitTcMsbMask> (
    const TrueColorPixelPtr<ScanlineFormat::N16BitTcMsbMask>& rDst,
    const TrueColorPixelPtr<ScanlineFormat::N16BitTcLsbMask>& rSrc )
{
    // byte swapping
    const PIXBYTE* pSrc = rSrc.GetRawPtr();
    PIXBYTE* pDst = rDst.GetRawPtr();
    pDst[1] = pSrc[0];
    pDst[0] = pSrc[1];
}

template <ScanlineFormat SRCFMT, ScanlineFormat DSTFMT>
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

// alpha blending truecolor pixels
template <ScanlineFormat SRCFMT, ScanlineFormat DSTFMT>
inline void ImplBlendPixels( const TrueColorPixelPtr<DSTFMT>& rDst,
    const TrueColorPixelPtr<SRCFMT>& rSrc, unsigned nAlphaVal )
{
    static const unsigned nAlphaShift = 8;
    if( !nAlphaVal )
        ImplConvertPixel( rDst, rSrc );
    else if( nAlphaVal != ~(~0U << nAlphaShift) )
    {
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

template <ScanlineFormat MASKFMT, ScanlineFormat SRCFMT, ScanlineFormat DSTFMT>
inline void ImplBlendLines( const TrueColorPixelPtr<DSTFMT>& rDst,
    const TrueColorPixelPtr<SRCFMT>& rSrc, const TrueColorPixelPtr<MASKFMT>& rMsk,
    int nPixelCount )
{
    TrueColorPixelPtr<MASKFMT> aMsk( rMsk );
    TrueColorPixelPtr<DSTFMT> aDst( rDst );
    TrueColorPixelPtr<SRCFMT> aSrc( rSrc );
    while( --nPixelCount >= 0 )
    {
        ImplBlendPixels(aDst, aSrc, aMsk.GetAlpha());
        ++aDst;
        ++aSrc;
        ++aMsk;
    }
}

static bool ImplCopyImage( BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer )
{
    const int nSrcLinestep = rSrcBuffer.mnScanlineSize;
    int nDstLinestep = rDstBuffer.mnScanlineSize;

    const PIXBYTE* pRawSrc = rSrcBuffer.mpBits;
    PIXBYTE* pRawDst = rDstBuffer.mpBits;

    // source and destination don't match upside down
    if( ScanlineFormat::TopDown & (rSrcBuffer.mnFormat ^ rDstBuffer.mnFormat)  )
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

template <ScanlineFormat DSTFMT,ScanlineFormat SRCFMT>
bool ImplConvertToBitmap( TrueColorPixelPtr<SRCFMT>& rSrcLine,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer )
{
    // help the compiler to avoid instantiations of unneeded conversions
    SAL_WARN_IF( SRCFMT == DSTFMT, "vcl.gdi", "ImplConvertToBitmap into same format");
    if( SRCFMT == DSTFMT )
        return false;

    const int nSrcLinestep = rSrcBuffer.mnScanlineSize;
    int nDstLinestep = rDstBuffer.mnScanlineSize;

    TrueColorPixelPtr<DSTFMT> aDstLine; aDstLine.SetRawPtr( rDstBuffer.mpBits );

    // source and destination don't match upside down
    if( ScanlineFormat::TopDown & (rSrcBuffer.mnFormat ^ rDstBuffer.mnFormat) )
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

template <ScanlineFormat SRCFMT>
inline bool ImplConvertFromBitmap( BitmapBuffer& rDst, const BitmapBuffer& rSrc )
{
    TrueColorPixelPtr<SRCFMT> aSrcType; aSrcType.SetRawPtr( rSrc.mpBits );

    // select the matching instantiation for the destination's bitmap format
    switch( rDst.mnFormat & ~ScanlineFormat::TopDown )
    {
        case ScanlineFormat::N1BitMsbPal:
        case ScanlineFormat::N1BitLsbPal:
        case ScanlineFormat::N4BitMsnPal:
        case ScanlineFormat::N4BitLsnPal:
        case ScanlineFormat::N8BitPal:
            break;

        case ScanlineFormat::N8BitTcMask:
//            return ImplConvertToBitmap<ScanlineFormat::N8BitTcMask>( aSrcType, rDst, rSrc );
        case ScanlineFormat::N32BitTcMask:
//            return ImplConvertToBitmap<ScanlineFormat::N32BitTcMask>( aSrcType, rDst, rSrc );
            break;

        case ScanlineFormat::N16BitTcMsbMask:
            return ImplConvertToBitmap<ScanlineFormat::N16BitTcMsbMask>( aSrcType, rDst, rSrc );
        case ScanlineFormat::N16BitTcLsbMask:
            return ImplConvertToBitmap<ScanlineFormat::N16BitTcLsbMask>( aSrcType, rDst, rSrc );

        case ScanlineFormat::N24BitTcBgr:
            return ImplConvertToBitmap<ScanlineFormat::N24BitTcBgr>( aSrcType, rDst, rSrc );
        case ScanlineFormat::N24BitTcRgb:
            return ImplConvertToBitmap<ScanlineFormat::N24BitTcRgb>( aSrcType, rDst, rSrc );

        case ScanlineFormat::N32BitTcAbgr:
            return ImplConvertToBitmap<ScanlineFormat::N32BitTcAbgr>( aSrcType, rDst, rSrc );
#ifdef FAST_ARGB_BGRA
        case ScanlineFormat::N32BitTcArgb:
            return ImplConvertToBitmap<ScanlineFormat::N32BitTcArgb>( aSrcType, rDst, rSrc );
        case ScanlineFormat::N32BitTcBgra:
            return ImplConvertToBitmap<ScanlineFormat::N32BitTcBgra>( aSrcType, rDst, rSrc );
#endif
        case ScanlineFormat::N32BitTcRgba:
            return ImplConvertToBitmap<ScanlineFormat::N32BitTcRgba>( aSrcType, rDst, rSrc );
        default: break;
    }

    static int nNotAccelerated = 0;
    SAL_WARN_IF( rSrc.mnWidth * rSrc.mnHeight >= 4000 && ++nNotAccelerated == 100,
                 "vcl.gdi",
                 "ImplConvertFromBitmap for not accelerated case (" << std::hex << (int)rSrc.mnFormat << "->" << (int)rDst.mnFormat << ")" );

    return false;
}

// A universal stretching conversion is overkill in most common situations
// => performance benefits for speeding up the non-stretching cases
bool ImplFastBitmapConversion( BitmapBuffer& rDst, const BitmapBuffer& rSrc,
    const SalTwoRect& rTR )
{
    // TODO:horizontal mirroring not implemented yet
    if( rTR.mnDestWidth < 0 )
        return false;
    // vertical mirroring
    if( rTR.mnDestHeight < 0 )
        // TODO: rDst.mnFormat ^= ScanlineFormat::TopDown;
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

    const ScanlineFormat nSrcFormat = rSrc.mnFormat & ~ScanlineFormat::TopDown;
    const ScanlineFormat nDstFormat = rDst.mnFormat & ~ScanlineFormat::TopDown;

    // TODO: also implement conversions for 16bit colormasks with non-565 format
    if( nSrcFormat & (ScanlineFormat::N16BitTcLsbMask | ScanlineFormat::N16BitTcMsbMask) )
        if( rSrc.maColorMask.GetRedMask()  != 0xF800
        ||  rSrc.maColorMask.GetGreenMask()!= 0x07E0
        ||  rSrc.maColorMask.GetBlueMask() != 0x001F )
            return false;
    if( nDstFormat & (ScanlineFormat::N16BitTcLsbMask | ScanlineFormat::N16BitTcMsbMask) )
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
        case ScanlineFormat::N1BitMsbPal:
        case ScanlineFormat::N1BitLsbPal:
        case ScanlineFormat::N4BitMsnPal:
        case ScanlineFormat::N4BitLsnPal:
        case ScanlineFormat::N8BitPal:
            break;

        case ScanlineFormat::N8BitTcMask:
//            return ImplConvertFromBitmap<ScanlineFormat::N8BitTcMask>( rDst, rSrc );
        case ScanlineFormat::N32BitTcMask:
//            return ImplConvertFromBitmap<ScanlineFormat::N32BitTcMask>( rDst, rSrc );
            break;

        case ScanlineFormat::N16BitTcMsbMask:
            return ImplConvertFromBitmap<ScanlineFormat::N16BitTcMsbMask>( rDst, rSrc );
        case ScanlineFormat::N16BitTcLsbMask:
            return ImplConvertFromBitmap<ScanlineFormat::N16BitTcLsbMask>( rDst, rSrc );

        case ScanlineFormat::N24BitTcBgr:
            return ImplConvertFromBitmap<ScanlineFormat::N24BitTcBgr>( rDst, rSrc );
        case ScanlineFormat::N24BitTcRgb:
            return ImplConvertFromBitmap<ScanlineFormat::N24BitTcRgb>( rDst, rSrc );

        case ScanlineFormat::N32BitTcAbgr:
            return ImplConvertFromBitmap<ScanlineFormat::N32BitTcAbgr>( rDst, rSrc );
#ifdef FAST_ARGB_BGRA
        case ScanlineFormat::N32BitTcArgb:
            return ImplConvertFromBitmap<ScanlineFormat::N32BitTcArgb>( rDst, rSrc );
        case ScanlineFormat::N32BitTcBgra:
            return ImplConvertFromBitmap<ScanlineFormat::N32BitTcBgra>( rDst, rSrc );
#endif
        case ScanlineFormat::N32BitTcRgba:
            return ImplConvertFromBitmap<ScanlineFormat::N32BitTcRgba>( rDst, rSrc );
        default: break;
    }

    static int nNotAccelerated = 0;
    SAL_WARN_IF( rSrc.mnWidth * rSrc.mnHeight >= 4000 && ++nNotAccelerated == 100,
                 "vcl.gdi",
                 "ImplFastBitmapConversion for not accelerated case (" << std::hex << (int)rSrc.mnFormat << "->" << (int)rDst.mnFormat << ")" );

    return false;
}

template <ScanlineFormat DSTFMT, ScanlineFormat SRCFMT> //,sal_uLong MSKFMT>
bool ImplBlendToBitmap( TrueColorPixelPtr<SRCFMT>& rSrcLine,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
{
    SAL_WARN_IF( rMskBuffer.mnFormat != ScanlineFormat::N8BitPal, "vcl.gdi", "FastBmp BlendImage: unusual MSKFMT" );

    const int nSrcLinestep = rSrcBuffer.mnScanlineSize;
    int nMskLinestep = rMskBuffer.mnScanlineSize;
    int nDstLinestep = rDstBuffer.mnScanlineSize;

    TrueColorPixelPtr<ScanlineFormat::N8BitPal> aMskLine; aMskLine.SetRawPtr( rMskBuffer.mpBits );
    TrueColorPixelPtr<DSTFMT> aDstLine; aDstLine.SetRawPtr( rDstBuffer.mpBits );

    // special case for single line masks
    if( rMskBuffer.mnHeight == 1 )
        nMskLinestep = 0;

    // source and mask don't match: upside down
    if( (rSrcBuffer.mnFormat ^ rMskBuffer.mnFormat) & ScanlineFormat::TopDown )
    {
        aMskLine.AddByteOffset( (rSrcBuffer.mnHeight - 1) * nMskLinestep );
        nMskLinestep = -nMskLinestep;
    }

    // source and destination don't match: upside down
    if( (rSrcBuffer.mnFormat ^ rDstBuffer.mnFormat) & ScanlineFormat::TopDown )
    {
        aDstLine.AddByteOffset( (rDstBuffer.mnHeight - 1) * nDstLinestep );
        nDstLinestep = -nDstLinestep;
    }

    assert(rDstBuffer.mnHeight <= rSrcBuffer.mnHeight && "not sure about that?");
    for (int y = rDstBuffer.mnHeight; --y >= 0;)
    {
        ImplBlendLines(aDstLine, rSrcLine, aMskLine, rDstBuffer.mnWidth);
        aDstLine.AddByteOffset( nDstLinestep );
        rSrcLine.AddByteOffset( nSrcLinestep );
        aMskLine.AddByteOffset( nMskLinestep );
    }

    return true;
}

// some specializations to reduce the code size
template <>
inline bool ImplBlendToBitmap<ScanlineFormat::N24BitTcBgr,ScanlineFormat::N24BitTcBgr>(
    TrueColorPixelPtr<ScanlineFormat::N24BitTcBgr>&,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
 {
    TrueColorPixelPtr<ScanlineFormat::N24BitTcRgb> aSrcType; aSrcType.SetRawPtr( rSrcBuffer.mpBits );
    return ImplBlendToBitmap<ScanlineFormat::N24BitTcRgb>( aSrcType, rDstBuffer, rSrcBuffer, rMskBuffer );
 }

template <>
inline bool ImplBlendToBitmap<ScanlineFormat::N32BitTcAbgr,ScanlineFormat::N32BitTcAbgr>(
    TrueColorPixelPtr<ScanlineFormat::N32BitTcAbgr>&,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
 {
    TrueColorPixelPtr<ScanlineFormat::N32BitTcArgb> aSrcType; aSrcType.SetRawPtr( rSrcBuffer.mpBits );
    return ImplBlendToBitmap<ScanlineFormat::N32BitTcArgb>( aSrcType, rDstBuffer, rSrcBuffer, rMskBuffer );
 }

template <>
inline bool ImplBlendToBitmap<ScanlineFormat::N32BitTcBgra,ScanlineFormat::N32BitTcBgra>(
    TrueColorPixelPtr<ScanlineFormat::N32BitTcBgra>&,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
 {
    TrueColorPixelPtr<ScanlineFormat::N32BitTcRgba> aSrcType; aSrcType.SetRawPtr( rSrcBuffer.mpBits );
    return ImplBlendToBitmap<ScanlineFormat::N32BitTcRgba>( aSrcType, rDstBuffer, rSrcBuffer, rMskBuffer );
 }

template <ScanlineFormat SRCFMT>
bool ImplBlendFromBitmap( BitmapBuffer& rDst, const BitmapBuffer& rSrc, const BitmapBuffer& rMsk )
{
    TrueColorPixelPtr<SRCFMT> aSrcType; aSrcType.SetRawPtr( rSrc.mpBits );

    // select the matching instantiation for the destination's bitmap format
    switch( rDst.mnFormat & ~ScanlineFormat::TopDown )
    {
        case ScanlineFormat::N1BitMsbPal:
        case ScanlineFormat::N1BitLsbPal:
        case ScanlineFormat::N4BitMsnPal:
        case ScanlineFormat::N4BitLsnPal:
        case ScanlineFormat::N8BitPal:
            break;

        case ScanlineFormat::N8BitTcMask:
//            return ImplBlendToBitmap<ScanlineFormat::N8BitTcMask>( aSrcType, rDst, rSrc, rMsk );
        case ScanlineFormat::N32BitTcMask:
//            return ImplBlendToBitmap<ScanlineFormat::N32BitTcMask>( aSrcType, rDst, rSrc, rMsk );
            break;

        case ScanlineFormat::N16BitTcMsbMask:
            return ImplBlendToBitmap<ScanlineFormat::N16BitTcMsbMask>( aSrcType, rDst, rSrc, rMsk );
        case ScanlineFormat::N16BitTcLsbMask:
            return ImplBlendToBitmap<ScanlineFormat::N16BitTcLsbMask>( aSrcType, rDst, rSrc, rMsk );

        case ScanlineFormat::N24BitTcBgr:
            return ImplBlendToBitmap<ScanlineFormat::N24BitTcBgr>( aSrcType, rDst, rSrc, rMsk );
        case ScanlineFormat::N24BitTcRgb:
            return ImplBlendToBitmap<ScanlineFormat::N24BitTcRgb>( aSrcType, rDst, rSrc, rMsk );

        case ScanlineFormat::N32BitTcAbgr:
            return ImplBlendToBitmap<ScanlineFormat::N32BitTcAbgr>( aSrcType, rDst, rSrc, rMsk );
#ifdef FAST_ARGB_BGRA
        case ScanlineFormat::N32BitTcArgb:
            return ImplBlendToBitmap<ScanlineFormat::N32BitTcArgb>( aSrcType, rDst, rSrc, rMsk );
        case ScanlineFormat::N32BitTcBgra:
            return ImplBlendToBitmap<ScanlineFormat::N32BitTcBgra>( aSrcType, rDst, rSrc, rMsk );
#endif
        case ScanlineFormat::N32BitTcRgba:
            return ImplBlendToBitmap<ScanlineFormat::N32BitTcRgba>( aSrcType, rDst, rSrc, rMsk );
        default: break;
    }

    static int nNotAccelerated = 0;
    SAL_WARN_IF( rSrc.mnWidth * rSrc.mnHeight >= 4000 && ++nNotAccelerated == 100,
                 "vcl.gdi",
                 "ImplBlendFromBitmap for not accelerated case (" << std::hex << (int)rSrc.mnFormat << "*" << (int)rMsk.mnFormat << "->" << (int)rDst.mnFormat );
    return false;
}

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
        // TODO: rDst.mnFormat ^= ScanlineFormat::TopDown;
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

    const ScanlineFormat nSrcFormat = rSrc.mnFormat & ~ScanlineFormat::TopDown;
    const ScanlineFormat nDstFormat = rDst.mnFormat & ~ScanlineFormat::TopDown;

    // accelerated conversions for 16bit colormasks with non-565 format are not yet implemented
    if( nSrcFormat & (ScanlineFormat::N16BitTcLsbMask | ScanlineFormat::N16BitTcMsbMask) )
        if( rSrc.maColorMask.GetRedMask()  != 0xF800
        ||  rSrc.maColorMask.GetGreenMask()!= 0x07E0
        ||  rSrc.maColorMask.GetBlueMask() != 0x001F)
            return false;
    if( nDstFormat & (ScanlineFormat::N16BitTcLsbMask | ScanlineFormat::N16BitTcMsbMask) )
        if( rDst.maColorMask.GetRedMask()  != 0xF800
        ||  rDst.maColorMask.GetGreenMask()!= 0x07E0
        ||  rDst.maColorMask.GetBlueMask() != 0x001F)
            return false;

    // select the matching instantiation for the source's bitmap format
    switch( nSrcFormat )
    {
        case ScanlineFormat::N1BitMsbPal:
        case ScanlineFormat::N1BitLsbPal:
        case ScanlineFormat::N4BitMsnPal:
        case ScanlineFormat::N4BitLsnPal:
        case ScanlineFormat::N8BitPal:
            break;

        case ScanlineFormat::N8BitTcMask:
//            return ImplBlendFromBitmap<ScanlineFormat::N8BitTcMask>( rDst, rSrc );
        case ScanlineFormat::N32BitTcMask:
//            return ImplBlendFromBitmap<ScanlineFormat::N32BitTcMask>( rDst, rSrc );
            break;

        case ScanlineFormat::N16BitTcMsbMask:
            return ImplBlendFromBitmap<ScanlineFormat::N16BitTcMsbMask>( rDst, rSrc, rMsk );
        case ScanlineFormat::N16BitTcLsbMask:
            return ImplBlendFromBitmap<ScanlineFormat::N16BitTcLsbMask>( rDst, rSrc, rMsk );

        case ScanlineFormat::N24BitTcBgr:
            return ImplBlendFromBitmap<ScanlineFormat::N24BitTcBgr>( rDst, rSrc, rMsk );
        case ScanlineFormat::N24BitTcRgb:
            return ImplBlendFromBitmap<ScanlineFormat::N24BitTcRgb>( rDst, rSrc, rMsk );

        case ScanlineFormat::N32BitTcAbgr:
            return ImplBlendFromBitmap<ScanlineFormat::N32BitTcAbgr>( rDst, rSrc, rMsk );
#ifdef FAST_ARGB_BGRA
        case ScanlineFormat::N32BitTcArgb:
            return ImplBlendFromBitmap<ScanlineFormat::N32BitTcArgb>( rDst, rSrc, rMsk );
        case ScanlineFormat::N32BitTcBgra:
            return ImplBlendFromBitmap<ScanlineFormat::N32BitTcBgra>( rDst, rSrc, rMsk );
#endif
        case ScanlineFormat::N32BitTcRgba:
            return ImplBlendFromBitmap<ScanlineFormat::N32BitTcRgba>( rDst, rSrc, rMsk );
        default: break;
    }

    static int nNotAccelerated = 0;
    SAL_WARN_IF( rSrc.mnWidth * rSrc.mnHeight >= 4000 && ++nNotAccelerated == 100,
                 "vcl.gdi",
                 "ImplFastBlend for not accelerated case (" << std::hex << (int)rSrc.mnFormat << "*" << (int)rMsk.mnFormat << "->" << (int)rDst.mnFormat << ")" );

    return false;
}

bool ImplFastEraseBitmap( BitmapBuffer& rDst, const BitmapColor& rColor )
{
    const ScanlineFormat nDstFormat = rDst.mnFormat & ~ScanlineFormat::TopDown;

    // erasing a bitmap is often just a byte-wise memory fill
    bool bByteFill = true;
    sal_uInt8 nFillByte;

    switch( nDstFormat )
    {
        case ScanlineFormat::N1BitMsbPal:
        case ScanlineFormat::N1BitLsbPal:
            nFillByte = rColor.GetIndex();
            nFillByte = static_cast<sal_uInt8>( -(nFillByte & 1) ); // 0x00 or 0xFF
            break;
        case ScanlineFormat::N4BitMsnPal:
        case ScanlineFormat::N4BitLsnPal:
            nFillByte = rColor.GetIndex();
            nFillByte &= 0x0F;
            nFillByte |= (nFillByte << 4);
            break;
        case ScanlineFormat::N8BitPal:
        case ScanlineFormat::N8BitTcMask:
            nFillByte = rColor.GetIndex();
            break;

        case ScanlineFormat::N24BitTcBgr:
        case ScanlineFormat::N24BitTcRgb:
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
        case ScanlineFormat::N32BitTcMask:
        case ScanlineFormat::N16BitTcMsbMask:
        case ScanlineFormat::N16BitTcLsbMask:

        case ScanlineFormat::N24BitTcBgr:
        case ScanlineFormat::N24BitTcRgb:

        case ScanlineFormat::N32BitTcAbgr:
#ifdef FAST_ARGB_BGRA
        case ScanlineFormat::N32BitTcArgb:
        case ScanlineFormat::N32BitTcBgra:
#endif
        case ScanlineFormat::N32BitTcRgba:
            break;

        default:
            break;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
