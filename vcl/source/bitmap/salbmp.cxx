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

#include <salbmp.hxx>
#include <o3tl/enumarray.hxx>

static BitmapChecksum scanlineChecksum(BitmapChecksum nCrc, const sal_uInt8* bits, int lineBitsCount, sal_uInt8 extraBitsMask)
{
    if( lineBitsCount / 8 > 0 )
        nCrc = vcl_get_checksum( nCrc, bits, lineBitsCount / 8 );
    if( extraBitsMask != 0 )
    {
        sal_uInt8 extraByte = bits[ lineBitsCount / 8 ] & extraBitsMask;
        nCrc = vcl_get_checksum( nCrc, &extraByte, 1 );
    }
    return nCrc;
}

void SalBitmap::updateChecksum() const
{
    if (mbChecksumValid)
        return;

    BitmapChecksum nCrc = 0;
    SalBitmap* pThis = const_cast<SalBitmap*>(this);
    BitmapBuffer* pBuf = pThis->AcquireBuffer(BitmapAccessMode::Read);
    if (pBuf)
    {
        nCrc = pBuf->maPalette.GetChecksum();
        const int lineBitsCount = pBuf->mnWidth * pBuf->mnBitCount;
        // With 1bpp/4bpp format we need to check only used bits in the last byte.
        sal_uInt8 extraBitsMask = 0;
        if( lineBitsCount % 8 != 0 )
        {
            const int extraBitsCount = lineBitsCount % 8;
            switch( RemoveScanline( pBuf->mnFormat ))
            {
                case ScanlineFormat::N1BitMsbPal:
                {
                    static const sal_uInt8 mask1Bit[] = { 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };
                    extraBitsMask = mask1Bit[ extraBitsCount ];
                    break;
                }
                case ScanlineFormat::N1BitLsbPal:
                {
                    static const sal_uInt8 mask1Bit[] = { 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
                    extraBitsMask = mask1Bit[ extraBitsCount ];
                    break;
                }
                default:
                    break;
            }
        }
        if( pBuf->mnFormat & ScanlineFormat::TopDown )
        {
            if( pBuf->mnScanlineSize == lineBitsCount / 8 )
                nCrc = vcl_get_checksum(nCrc, pBuf->mpBits, pBuf->mnScanlineSize * pBuf->mnHeight);
            else // Do not include padding with undefined content in the checksum.
                for( tools::Long y = 0; y < pBuf->mnHeight; ++y )
                    nCrc = scanlineChecksum(nCrc, pBuf->mpBits + y * pBuf->mnScanlineSize, lineBitsCount, extraBitsMask);
        }
        else // Compute checksum in the order of scanlines, to make it consistent between different bitmap implementations.
        {
            for( tools::Long y = pBuf->mnHeight - 1; y >= 0; --y )
                nCrc = scanlineChecksum(nCrc, pBuf->mpBits + y * pBuf->mnScanlineSize, lineBitsCount, extraBitsMask);
        }
        pThis->ReleaseBuffer(pBuf, BitmapAccessMode::Read);
        pThis->mnChecksum = nCrc;
        pThis->mbChecksumValid = true;
    }
    else
    {
        pThis->mbChecksumValid = false;
    }
}

namespace
{

class ImplPixelFormat
{
protected:
    const sal_uInt8* mpData;
public:
    static ImplPixelFormat* GetFormat( sal_uInt16 nBits, const BitmapPalette& rPalette );

    virtual void StartLine( const sal_uInt8* pLine ) { mpData = pLine; }
    virtual const BitmapColor& ReadPixel() = 0;
    virtual ~ImplPixelFormat() { }
};

class ImplPixelFormat8 : public ImplPixelFormat
{
private:
    const BitmapPalette& mrPalette;

public:
    explicit ImplPixelFormat8( const BitmapPalette& rPalette )
    : mrPalette( rPalette )
    {
    }
    virtual const BitmapColor& ReadPixel() override
    {
        assert( mrPalette.GetEntryCount() > *mpData );
        return mrPalette[ *mpData++ ];
    }
};

class ImplPixelFormat4 : public ImplPixelFormat
{
private:
    const BitmapPalette& mrPalette;
    sal_uInt32 mnX;
    sal_uInt32 mnShift;

public:
    explicit ImplPixelFormat4( const BitmapPalette& rPalette )
        : mrPalette( rPalette )
        , mnX(0)
        , mnShift(4)
    {
    }
    virtual void StartLine( const sal_uInt8* pLine ) override
    {
        mpData = pLine;
        mnX = 0;
        mnShift = 4;
    }
    virtual const BitmapColor& ReadPixel() override
    {
        sal_uInt32 nIdx = ( mpData[mnX >> 1] >> mnShift) & 0x0f;
        assert( mrPalette.GetEntryCount() > nIdx );
        const BitmapColor& rColor = mrPalette[nIdx];
        mnX++;
        mnShift ^= 4;
        return rColor;
    }
};

class ImplPixelFormat1 : public ImplPixelFormat
{
private:
    const BitmapPalette& mrPalette;
    sal_uInt32 mnX;

public:
    explicit ImplPixelFormat1( const BitmapPalette& rPalette )
        : mrPalette(rPalette)
        , mnX(0)
    {
    }
    virtual void StartLine( const sal_uInt8* pLine ) override
    {
        mpData = pLine;
        mnX = 0;
    }
    virtual const BitmapColor& ReadPixel() override
    {
        const BitmapColor& rColor = mrPalette[ (mpData[mnX >> 3 ] >> ( 7 - ( mnX & 7 ) )) & 1];
        mnX++;
        return rColor;
    }
};

ImplPixelFormat* ImplPixelFormat::GetFormat( sal_uInt16 nBits, const BitmapPalette& rPalette )
{
    switch( nBits )
    {
    case 1: return new ImplPixelFormat1( rPalette );
    case 4: return new ImplPixelFormat4( rPalette );
    case 8: return new ImplPixelFormat8( rPalette );
    }

    return nullptr;
}

// Optimized conversion from 1bpp. Currently LO uses 1bpp bitmaps for masks, which is nowadays
// a lousy obsolete format, as the memory saved is just not worth the cost of fiddling with the bits.
// Ideally LO should move to RGBA bitmaps. Until then, try to be faster with 1bpp bitmaps.
typedef void(*WriteColorFunction)( sal_uInt8 color8Bit, sal_uInt8*& dst );
void writeColorA8(sal_uInt8 color8Bit, sal_uInt8*& dst ) { *dst++ = color8Bit; };
void writeColorRGBA(sal_uInt8 color8Bit, sal_uInt8*& dst ) { *dst++ = color8Bit; *dst++ = color8Bit; *dst++ = color8Bit; *dst++ = 0xff; };
typedef void(*WriteBlackWhiteFunction)( sal_uInt8*& dst, int count );
void writeBlackA8(sal_uInt8*& dst, int count ) { memset( dst, 0, count ); dst += count; };
void writeWhiteA8(sal_uInt8*& dst, int count ) { memset( dst, 0xff, count ); dst += count; };
void writeWhiteRGBA(sal_uInt8*& dst, int count ) { memset( dst, 0xff, count * 4 ); dst += count * 4; };
void writeBlackRGBA(sal_uInt8*& dst, int count )
{
    for( int i = 0; i < count; ++i )
    {
        dst[0] = 0x00;
        dst[1] = 0x00;
        dst[2] = 0x00;
        dst[3] = 0xff;
        dst += 4;
    }
};

template< WriteColorFunction func, WriteBlackWhiteFunction funcBlack, WriteBlackWhiteFunction funcWhite >
void writeBlackWhiteData( const sal_uInt8* src, sal_uInt8* dst, int width, int height, int bytesPerRow )
{
    for( int y = 0; y < height; ++y )
    {
        const sal_uInt8* srcLine = src;
        int xsize = width;
        while( xsize >= 64 )
        {
            // TODO alignment?
            const sal_uInt64* src64 = reinterpret_cast< const sal_uInt64* >( src );
            if( *src64 == 0x00 )
                funcBlack( dst, 64 );
            else if( *src64 == static_cast< sal_uInt64 >( -1 ))
                funcWhite( dst, 64 );
            else
                break;
            src += sizeof( sal_uInt64 );
            xsize -= 64;
        }
        while( xsize >= 8 )
        {
            if( *src == 0x00 ) // => eight black pixels
                funcBlack( dst, 8 );
            else if( *src == 0xff ) // => eight white pixels
                funcWhite( dst, 8 );
            else
                for( int bit = 7; bit >= 0; --bit )
                    func(( *src >> bit ) & 1 ? 0xff : 0, dst );
            ++src;
            xsize -= 8;
        }
        for( int bit = 7; bit > 7 - xsize; --bit )
            func(( *src >> bit ) & 1 ? 0xff : 0, dst );
        ++src;
        src = srcLine + bytesPerRow;
    }
}

} // namespace

std::unique_ptr< sal_uInt8[] > SalBitmap::convertDataBitCount( const sal_uInt8* src,
    int width, int height, int bitCount, int bytesPerRow, const BitmapPalette& palette, BitConvert type )
{
    assert( bitCount == 1 || bitCount == 4 || bitCount == 8 );
    static const o3tl::enumarray<BitConvert, int> bpp = { 1, 4, 4 };
    std::unique_ptr< sal_uInt8[] > data( new sal_uInt8[width * height * bpp[ type ]] );

    if(type == BitConvert::A8 && bitCount == 8 && palette.IsGreyPalette8Bit())
    { // no actual data conversion
        for( int y = 0; y < height; ++y )
            memcpy( data.get() + y * width, src + y * bytesPerRow, width );
        return data;
    }

    if(bitCount == 1 && palette.GetEntryCount() == 2 && palette[ 0 ] == COL_BLACK && palette[ 1 ] == COL_WHITE)
    {
        switch( type )
        {
            case BitConvert::A8 :
                writeBlackWhiteData< writeColorA8, writeBlackA8, writeWhiteA8 >
                    ( src, data.get(), width, height, bytesPerRow );
                return data;
            case BitConvert::BGRA :
            case BitConvert::RGBA :
                // BGRA/RGBA is the same, all 3 values get the same value
                writeBlackWhiteData< writeColorRGBA, writeBlackRGBA, writeWhiteRGBA >
                    ( src, data.get(), width, height, bytesPerRow );
                return data;
        }
    }

    std::unique_ptr<ImplPixelFormat> pSrcFormat(ImplPixelFormat::GetFormat(bitCount, palette));

    const sal_uInt8* pSrcData = src;
    sal_uInt8* pDstData = data.get();

    sal_uInt32 nY = height;
    while( nY-- )
    {
        pSrcFormat->StartLine( pSrcData );

        sal_uInt32 nX = width;
        switch( type )
        {
            case BitConvert::A8 :
                while( nX-- )
                {
                    const BitmapColor& c = pSrcFormat->ReadPixel();
                    *pDstData++ = c.GetBlue();
                }
                break;
            case BitConvert::BGRA :
                while( nX-- )
                {
                    const BitmapColor& c = pSrcFormat->ReadPixel();
                    *pDstData++ = c.GetBlue();
                    *pDstData++ = c.GetGreen();
                    *pDstData++ = c.GetRed();
                    *pDstData++ = 0xff;
                }
                break;
            case BitConvert::RGBA :
                while( nX-- )
                {
                    const BitmapColor& c = pSrcFormat->ReadPixel();
                    *pDstData++ = c.GetRed();
                    *pDstData++ = c.GetGreen();
                    *pDstData++ = c.GetBlue();
                    *pDstData++ = 0xff;
                }
                break;
        }

        pSrcData += bytesPerRow;
    }
    return data;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
