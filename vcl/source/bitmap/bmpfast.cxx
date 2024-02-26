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

#include <vcl/salgtype.hxx>

#include <vcl/BitmapWriteAccess.hxx>
#include <bitmap/bmpfast.hxx>

#include <sal/log.hxx>

typedef unsigned char PIXBYTE;

namespace {

class BasePixelPtr
{
public:
    explicit BasePixelPtr( PIXBYTE* p = nullptr ) : mpPixel( p ) {}
    void    SetRawPtr( PIXBYTE* pRawPtr )               { mpPixel = pRawPtr; }
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
    static PIXBYTE GetAlpha()  { return 255; }
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
    static PIXBYTE GetAlpha()   { return 255; }
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

// This assumes the content uses the grayscale palette (needs to be checked
// by code allowing the use of the format).
// Only reading color is implemented, since e.g. 24bpp input couldn't be
// easily guaranteed to be grayscale.
template <>
class TrueColorPixelPtr<ScanlineFormat::N8BitPal> : public BasePixelPtr
{
public:
    void    operator++()                    { mpPixel += 1; }

    PIXBYTE GetRed() const      { return mpPixel[0]; }
    PIXBYTE GetGreen() const    { return mpPixel[0]; }
    PIXBYTE GetBlue() const     { return mpPixel[0]; }
    static PIXBYTE GetAlpha()   { return 255; }
};

}

// converting truecolor formats
template <ScanlineFormat SRCFMT, ScanlineFormat DSTFMT>
static void ImplConvertPixel( const TrueColorPixelPtr<DSTFMT>& rDst,
    const TrueColorPixelPtr<SRCFMT>& rSrc )
{
    rDst.SetColor( rSrc.GetRed(), rSrc.GetGreen(), rSrc.GetBlue() );
    rDst.SetAlpha( rSrc.GetAlpha() );
}

template <ScanlineFormat SRCFMT, ScanlineFormat DSTFMT>
static void ImplConvertLine( const TrueColorPixelPtr<DSTFMT>& rDst,
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
static void ImplBlendPixels( const TrueColorPixelPtr<DSTFMT>& rDst,
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
static void ImplBlendLines( const TrueColorPixelPtr<DSTFMT>& rDst,
    const TrueColorPixelPtr<SRCFMT>& rSrc, const TrueColorPixelPtr<MASKFMT>& rMsk,
    int nPixelCount )
{
    TrueColorPixelPtr<MASKFMT> aMsk( rMsk );
    TrueColorPixelPtr<DSTFMT> aDst( rDst );
    TrueColorPixelPtr<SRCFMT> aSrc( rSrc );
    while( --nPixelCount >= 0 )
    {
        // VCL masks store alpha as color, hence the GetRed() and not GetAlpha().
        ImplBlendPixels(aDst, aSrc, aMsk.GetRed());
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
    if (rSrcBuffer.meDirection != rDstBuffer.meDirection)
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
static bool ImplConvertToBitmap( TrueColorPixelPtr<SRCFMT>& rSrcLine,
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
    if (rSrcBuffer.meDirection != rDstBuffer.meDirection)
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
static bool ImplConvertFromBitmap( BitmapBuffer& rDst, const BitmapBuffer& rSrc )
{
    TrueColorPixelPtr<SRCFMT> aSrcType; aSrcType.SetRawPtr( rSrc.mpBits );

    // select the matching instantiation for the destination's bitmap format
    switch (rDst.meFormat)
    {
        case ScanlineFormat::N1BitMsbPal:
        case ScanlineFormat::N8BitPal:
            break;

        case ScanlineFormat::N32BitTcMask:
//            return ImplConvertToBitmap<ScanlineFormat::N32BitTcMask>( aSrcType, rDst, rSrc );
            break;

        case ScanlineFormat::N24BitTcBgr:
            return ImplConvertToBitmap<ScanlineFormat::N24BitTcBgr>( aSrcType, rDst, rSrc );
        case ScanlineFormat::N24BitTcRgb:
            return ImplConvertToBitmap<ScanlineFormat::N24BitTcRgb>( aSrcType, rDst, rSrc );

        case ScanlineFormat::N32BitTcAbgr:
            return ImplConvertToBitmap<ScanlineFormat::N32BitTcAbgr>( aSrcType, rDst, rSrc );
        case ScanlineFormat::N32BitTcArgb:
            return ImplConvertToBitmap<ScanlineFormat::N32BitTcArgb>( aSrcType, rDst, rSrc );
        case ScanlineFormat::N32BitTcBgra:
            return ImplConvertToBitmap<ScanlineFormat::N32BitTcBgra>( aSrcType, rDst, rSrc );
        case ScanlineFormat::N32BitTcRgba:
            return ImplConvertToBitmap<ScanlineFormat::N32BitTcRgba>( aSrcType, rDst, rSrc );
        default: break;
    }

    static int nNotAccelerated = 0;
    SAL_WARN_IF( rSrc.mnWidth * rSrc.mnHeight >= 4000 && ++nNotAccelerated == 100,
                 "vcl.gdi",
                 "ImplConvertFromBitmap for not accelerated case (" << std::hex << int(rSrc.meFormat) << "->" << int(rDst.meFormat) << ")" );

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
        // TODO: rDst.meDirection != ScanlineDirection::TopDown;
        return false;

    // offsetted conversion is not implemented yet
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

    const ScanlineFormat nSrcFormat = rSrc.meFormat;
    const ScanlineFormat nDstFormat = rDst.meFormat;

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
            break;

        case ScanlineFormat::N32BitTcMask:
//            return ImplConvertFromBitmap<ScanlineFormat::N32BitTcMask>( rDst, rSrc );
            break;

        case ScanlineFormat::N8BitPal:
            if(rSrc.maPalette.IsGreyPalette8Bit())
                return ImplConvertFromBitmap<ScanlineFormat::N8BitPal>( rDst, rSrc );
            break;

        case ScanlineFormat::N24BitTcBgr:
            return ImplConvertFromBitmap<ScanlineFormat::N24BitTcBgr>( rDst, rSrc );
        case ScanlineFormat::N24BitTcRgb:
            return ImplConvertFromBitmap<ScanlineFormat::N24BitTcRgb>( rDst, rSrc );

        case ScanlineFormat::N32BitTcAbgr:
            return ImplConvertFromBitmap<ScanlineFormat::N32BitTcAbgr>( rDst, rSrc );
        case ScanlineFormat::N32BitTcArgb:
            return ImplConvertFromBitmap<ScanlineFormat::N32BitTcArgb>( rDst, rSrc );
        case ScanlineFormat::N32BitTcBgra:
            return ImplConvertFromBitmap<ScanlineFormat::N32BitTcBgra>( rDst, rSrc );
        case ScanlineFormat::N32BitTcRgba:
            return ImplConvertFromBitmap<ScanlineFormat::N32BitTcRgba>( rDst, rSrc );
        default: break;
    }

    static int nNotAccelerated = 0;
    SAL_WARN_IF( rSrc.mnWidth * rSrc.mnHeight >= 4000 && ++nNotAccelerated == 100,
                 "vcl.gdi",
                 "ImplFastBitmapConversion for not accelerated case (" << std::hex << int(rSrc.meFormat) << "->" << int(rDst.meFormat) << ")" );

    return false;
}

static inline ConstScanline ImplGetScanline( const BitmapBuffer& rBuf, tools::Long nY )
{
    if (rBuf.meDirection == ScanlineDirection::TopDown)
        return rBuf.mpBits + nY * rBuf.mnScanlineSize;
    else
        return rBuf.mpBits + (rBuf.mnHeight - 1 - nY) * rBuf.mnScanlineSize;
}

static inline Scanline ImplGetScanline( BitmapBuffer& rBuf, tools::Long nY )
{
    return const_cast<Scanline>(ImplGetScanline( const_cast<const BitmapBuffer&>(rBuf), nY ));
}

template <ScanlineFormat DSTFMT, ScanlineFormat SRCFMT>
static bool ImplCopyToScanline( tools::Long nY, BitmapBuffer& rDst, TrueColorPixelPtr<SRCFMT>& rSrcLine, tools::Long nSrcWidth )
{
    TrueColorPixelPtr<DSTFMT> aDstType;
    aDstType.SetRawPtr( ImplGetScanline( rDst, nY ));
    ImplConvertLine( aDstType, rSrcLine, std::min( nSrcWidth, rDst.mnWidth ));
    return true;
}

template <ScanlineFormat SRCFMT>
static bool ImplCopyFromScanline( tools::Long nY, BitmapBuffer& rDst, ConstScanline aSrcScanline, tools::Long nSrcWidth )
{
    TrueColorPixelPtr<SRCFMT> aSrcType;
    aSrcType.SetRawPtr( const_cast<Scanline>( aSrcScanline ));
    // select the matching instantiation for the destination's bitmap format
    switch (rDst.meFormat)
    {
        case ScanlineFormat::N24BitTcBgr:
            return ImplCopyToScanline<ScanlineFormat::N24BitTcBgr>( nY, rDst, aSrcType, nSrcWidth );
        case ScanlineFormat::N24BitTcRgb:
            return ImplCopyToScanline<ScanlineFormat::N24BitTcRgb>( nY, rDst, aSrcType, nSrcWidth );

        case ScanlineFormat::N32BitTcAbgr:
            return ImplCopyToScanline<ScanlineFormat::N32BitTcAbgr>( nY, rDst, aSrcType, nSrcWidth );
        case ScanlineFormat::N32BitTcArgb:
            return ImplCopyToScanline<ScanlineFormat::N32BitTcArgb>( nY, rDst, aSrcType, nSrcWidth );
        case ScanlineFormat::N32BitTcBgra:
            return ImplCopyToScanline<ScanlineFormat::N32BitTcBgra>( nY, rDst, aSrcType, nSrcWidth );
        case ScanlineFormat::N32BitTcRgba:
            return ImplCopyToScanline<ScanlineFormat::N32BitTcRgba>( nY, rDst, aSrcType, nSrcWidth );
        default:
            break;
    }
    return false;

}

bool ImplFastCopyScanline( tools::Long nY, BitmapBuffer& rDst, ConstScanline aSrcScanline,
    ScanlineFormat nSrcScanlineFormat, sal_uInt32 nSrcScanlineSize)
{
    if( rDst.mnHeight <= nY )
        return false;

    const ScanlineFormat nSrcFormat = nSrcScanlineFormat;
    const ScanlineFormat nDstFormat = rDst.meFormat;

    // special handling of trivial cases
    if( nSrcFormat == nDstFormat )
    {
        memcpy( ImplGetScanline( rDst, nY ), aSrcScanline, std::min<tools::Long>(nSrcScanlineSize, rDst.mnScanlineSize));
        return true;
    }

    // select the matching instantiation for the source's bitmap format
    switch( nSrcFormat )
    {
        case ScanlineFormat::N24BitTcBgr:
            return ImplCopyFromScanline<ScanlineFormat::N24BitTcBgr>( nY, rDst, aSrcScanline, nSrcScanlineSize / 3 );
        case ScanlineFormat::N24BitTcRgb:
            return ImplCopyFromScanline<ScanlineFormat::N24BitTcRgb>( nY, rDst, aSrcScanline, nSrcScanlineSize / 3 );

        case ScanlineFormat::N32BitTcAbgr:
            return ImplCopyFromScanline<ScanlineFormat::N32BitTcAbgr>( nY, rDst, aSrcScanline, nSrcScanlineSize / 4 );
        case ScanlineFormat::N32BitTcArgb:
            return ImplCopyFromScanline<ScanlineFormat::N32BitTcArgb>( nY, rDst, aSrcScanline, nSrcScanlineSize / 4 );
        case ScanlineFormat::N32BitTcBgra:
            return ImplCopyFromScanline<ScanlineFormat::N32BitTcBgra>( nY, rDst, aSrcScanline, nSrcScanlineSize / 4 );
        case ScanlineFormat::N32BitTcRgba:
            return ImplCopyFromScanline<ScanlineFormat::N32BitTcRgba>( nY, rDst, aSrcScanline, nSrcScanlineSize / 4 );
        default:
            break;
    }
    return false;
}

bool ImplFastCopyScanline( tools::Long nY, BitmapBuffer& rDst, const BitmapBuffer& rSrc)
{
    if( nY >= rDst.mnHeight )
        return false;
    if( rSrc.maPalette != rDst.maPalette )
        return false;
    return ImplFastCopyScanline( nY, rDst, ImplGetScanline( rSrc, nY ), rSrc.meFormat, rSrc.mnScanlineSize);
}

template <ScanlineFormat DSTFMT, ScanlineFormat SRCFMT> //,sal_uLong MSKFMT>
static bool ImplBlendToBitmap( TrueColorPixelPtr<SRCFMT>& rSrcLine,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
{
    SAL_WARN_IF(rMskBuffer.meFormat != ScanlineFormat::N8BitPal, "vcl.gdi", "FastBmp BlendImage: unusual MSKFMT");

    const int nSrcLinestep = rSrcBuffer.mnScanlineSize;
    int nMskLinestep = rMskBuffer.mnScanlineSize;
    int nDstLinestep = rDstBuffer.mnScanlineSize;

    TrueColorPixelPtr<ScanlineFormat::N8BitPal> aMskLine; aMskLine.SetRawPtr( rMskBuffer.mpBits );
    TrueColorPixelPtr<DSTFMT> aDstLine; aDstLine.SetRawPtr( rDstBuffer.mpBits );

    // special case for single line masks
    if( rMskBuffer.mnHeight == 1 )
        nMskLinestep = 0;

    // source and mask don't match: upside down
    if (rSrcBuffer.meDirection != rMskBuffer.meDirection)
    {
        aMskLine.AddByteOffset( (rSrcBuffer.mnHeight - 1) * nMskLinestep );
        nMskLinestep = -nMskLinestep;
    }

    // source and destination don't match: upside down
    if (rSrcBuffer.meDirection != rDstBuffer.meDirection)
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
bool ImplBlendToBitmap<ScanlineFormat::N24BitTcBgr,ScanlineFormat::N24BitTcBgr>(
    TrueColorPixelPtr<ScanlineFormat::N24BitTcBgr>&,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
 {
    TrueColorPixelPtr<ScanlineFormat::N24BitTcRgb> aSrcType; aSrcType.SetRawPtr( rSrcBuffer.mpBits );
    return ImplBlendToBitmap<ScanlineFormat::N24BitTcRgb>( aSrcType, rDstBuffer, rSrcBuffer, rMskBuffer );
 }

template <>
bool ImplBlendToBitmap<ScanlineFormat::N32BitTcAbgr,ScanlineFormat::N32BitTcAbgr>(
    TrueColorPixelPtr<ScanlineFormat::N32BitTcAbgr>&,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
 {
    TrueColorPixelPtr<ScanlineFormat::N32BitTcArgb> aSrcType; aSrcType.SetRawPtr( rSrcBuffer.mpBits );
    return ImplBlendToBitmap<ScanlineFormat::N32BitTcArgb>( aSrcType, rDstBuffer, rSrcBuffer, rMskBuffer );
 }

template <>
bool ImplBlendToBitmap<ScanlineFormat::N32BitTcBgra,ScanlineFormat::N32BitTcBgra>(
    TrueColorPixelPtr<ScanlineFormat::N32BitTcBgra>&,
    BitmapBuffer& rDstBuffer, const BitmapBuffer& rSrcBuffer,
    const BitmapBuffer& rMskBuffer )
 {
    TrueColorPixelPtr<ScanlineFormat::N32BitTcRgba> aSrcType; aSrcType.SetRawPtr( rSrcBuffer.mpBits );
    return ImplBlendToBitmap<ScanlineFormat::N32BitTcRgba>( aSrcType, rDstBuffer, rSrcBuffer, rMskBuffer );
 }

template <ScanlineFormat SRCFMT>
static bool ImplBlendFromBitmap( BitmapBuffer& rDst, const BitmapBuffer& rSrc, const BitmapBuffer& rMsk )
{
    TrueColorPixelPtr<SRCFMT> aSrcType; aSrcType.SetRawPtr( rSrc.mpBits );

    // select the matching instantiation for the destination's bitmap format
    switch (rDst.meFormat)
    {
        case ScanlineFormat::N1BitMsbPal:
        case ScanlineFormat::N8BitPal:
            break;

        case ScanlineFormat::N32BitTcMask:
//            return ImplBlendToBitmap<ScanlineFormat::N32BitTcMask>( aSrcType, rDst, rSrc, rMsk );
            break;

        case ScanlineFormat::N24BitTcBgr:
            return ImplBlendToBitmap<ScanlineFormat::N24BitTcBgr>( aSrcType, rDst, rSrc, rMsk );
        case ScanlineFormat::N24BitTcRgb:
            return ImplBlendToBitmap<ScanlineFormat::N24BitTcRgb>( aSrcType, rDst, rSrc, rMsk );

        case ScanlineFormat::N32BitTcAbgr:
            return ImplBlendToBitmap<ScanlineFormat::N32BitTcAbgr>( aSrcType, rDst, rSrc, rMsk );
        case ScanlineFormat::N32BitTcArgb:
            return ImplBlendToBitmap<ScanlineFormat::N32BitTcArgb>( aSrcType, rDst, rSrc, rMsk );
        case ScanlineFormat::N32BitTcBgra:
            return ImplBlendToBitmap<ScanlineFormat::N32BitTcBgra>( aSrcType, rDst, rSrc, rMsk );
        case ScanlineFormat::N32BitTcRgba:
            return ImplBlendToBitmap<ScanlineFormat::N32BitTcRgba>( aSrcType, rDst, rSrc, rMsk );
        default: break;
    }

    static int nNotAccelerated = 0;
    SAL_WARN_IF( rSrc.mnWidth * rSrc.mnHeight >= 4000 && ++nNotAccelerated == 100,
                 "vcl.gdi",
                 "ImplBlendFromBitmap for not accelerated case (" << std::hex << int(rSrc.meFormat) << "*" << int(rMsk.meFormat) << "->" << int(rDst.meFormat) );
    return false;
}

bool ImplFastBitmapBlending( BitmapWriteAccess const & rDstWA,
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
        // TODO: rDst.meDirection != ScanlineDirection::TopDown;
        return false;

    // offsetted blending is not implemented yet
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

    // select the matching instantiation for the source's bitmap format
    switch (rSrc.meFormat)
    {
        case ScanlineFormat::N1BitMsbPal:
            break;

        case ScanlineFormat::N32BitTcMask:
//            return ImplBlendFromBitmap<ScanlineFormat::N32BitTcMask>( rDst, rSrc );
            break;

        case ScanlineFormat::N8BitPal:
            if(rSrc.maPalette.IsGreyPalette8Bit())
                return ImplBlendFromBitmap<ScanlineFormat::N8BitPal>( rDst, rSrc, rMsk );
            break;

        case ScanlineFormat::N24BitTcBgr:
            return ImplBlendFromBitmap<ScanlineFormat::N24BitTcBgr>( rDst, rSrc, rMsk );
        case ScanlineFormat::N24BitTcRgb:
            return ImplBlendFromBitmap<ScanlineFormat::N24BitTcRgb>( rDst, rSrc, rMsk );

        case ScanlineFormat::N32BitTcAbgr:
            return ImplBlendFromBitmap<ScanlineFormat::N32BitTcAbgr>( rDst, rSrc, rMsk );
        case ScanlineFormat::N32BitTcArgb:
            return ImplBlendFromBitmap<ScanlineFormat::N32BitTcArgb>( rDst, rSrc, rMsk );
        case ScanlineFormat::N32BitTcBgra:
            return ImplBlendFromBitmap<ScanlineFormat::N32BitTcBgra>( rDst, rSrc, rMsk );
        case ScanlineFormat::N32BitTcRgba:
            return ImplBlendFromBitmap<ScanlineFormat::N32BitTcRgba>( rDst, rSrc, rMsk );
        default: break;
    }

    static int nNotAccelerated = 0;
    SAL_WARN_IF( rSrc.mnWidth * rSrc.mnHeight >= 4000 && ++nNotAccelerated == 100,
                 "vcl.gdi",
                 "ImplFastBlend for not accelerated case (" << std::hex << int(rSrc.meFormat) << "*" << int(rMsk.meFormat) << "->" << int(rDst.meFormat) << ")" );

    return false;
}

bool ImplFastEraseBitmap( BitmapBuffer& rDst, const BitmapColor& rColor )
{
    // erasing a bitmap is often just a byte-wise memory fill
    bool bByteFill = true;
    sal_uInt8 nFillByte;

    switch (rDst.meFormat)
    {
        case ScanlineFormat::N1BitMsbPal:
            nFillByte = rColor.GetIndex();
            nFillByte = static_cast<sal_uInt8>( -(nFillByte & 1) ); // 0x00 or 0xFF
            break;
        case ScanlineFormat::N8BitPal:
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
        tools::Long nByteCount = rDst.mnHeight * rDst.mnScanlineSize;
        memset( rDst.mpBits, nFillByte, nByteCount );
        return true;
    }

    // TODO: handle other bitmap formats
    switch (rDst.meFormat)
    {
        case ScanlineFormat::N32BitTcMask:

        case ScanlineFormat::N24BitTcBgr:
        case ScanlineFormat::N24BitTcRgb:

        case ScanlineFormat::N32BitTcAbgr:
        case ScanlineFormat::N32BitTcArgb:
        case ScanlineFormat::N32BitTcBgra:
        case ScanlineFormat::N32BitTcRgba:
            break;

        default:
            break;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
