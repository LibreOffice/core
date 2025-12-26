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
#include <vcl/BitmapColor.hxx>
#include <rtl/crc.h>

static BitmapChecksum scanlineChecksum(BitmapChecksum nCrc, const sal_uInt8* bits, int lineBitsCount, sal_uInt8 extraBitsMask)
{
    if( lineBitsCount / 8 > 0 )
        nCrc = rtl_crc32( nCrc, bits, lineBitsCount / 8 );
    if( extraBitsMask != 0 )
    {
        sal_uInt8 extraByte = bits[ lineBitsCount / 8 ] & extraBitsMask;
        nCrc = rtl_crc32( nCrc, &extraByte, 1 );
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
        if (pBuf->meDirection == ScanlineDirection::TopDown)
        {
            if( pBuf->mnScanlineSize == lineBitsCount / 8 )
                nCrc = rtl_crc32(nCrc, pBuf->mpBits, pBuf->mnScanlineSize * pBuf->mnHeight);
            else // Do not include padding with undefined content in the checksum.
                for( tools::Long y = 0; y < pBuf->mnHeight; ++y )
                    nCrc = scanlineChecksum(nCrc, pBuf->mpBits + y * pBuf->mnScanlineSize, lineBitsCount, 0);
        }
        else // Compute checksum in the order of scanlines, to make it consistent between different bitmap implementations.
        {
            for( tools::Long y = pBuf->mnHeight - 1; y >= 0; --y )
                nCrc = scanlineChecksum(nCrc, pBuf->mpBits + y * pBuf->mnScanlineSize, lineBitsCount, 0);
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

sal_uInt16 SalBitmap::GetBitCount() const
{
    switch (GetScanlineFormat())
    {
        case ScanlineFormat::NONE: return 0;
        case ScanlineFormat::N8BitPal: return 8;
        case ScanlineFormat::N24BitTcBgr: return 24;
        case ScanlineFormat::N24BitTcRgb: return 24;
        case ScanlineFormat::N32BitTcAbgr:
        case ScanlineFormat::N32BitTcXbgr:
        case ScanlineFormat::N32BitTcArgb:
        case ScanlineFormat::N32BitTcXrgb:
        case ScanlineFormat::N32BitTcBgra:
        case ScanlineFormat::N32BitTcBgrx:
        case ScanlineFormat::N32BitTcRgba:
        case ScanlineFormat::N32BitTcRgbx:
            return 32;
        default: abort();
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

} // namespace

std::unique_ptr< sal_uInt8[] > SalBitmap::convertDataBitCount( const sal_uInt8* src,
    int width, int height, int bitCount, int bytesPerRow, const BitmapPalette& palette, BitConvert type )
{
    assert( bitCount == 4 || bitCount == 8 );
    static const o3tl::enumarray<BitConvert, int> bpp = { 1, 4, 4 };
    std::unique_ptr< sal_uInt8[] > data( new sal_uInt8[width * height * bpp[ type ]] );

    if(type == BitConvert::A8 && bitCount == 8 && palette.IsGreyPalette8Bit())
    { // no actual data conversion
        for( int y = 0; y < height; ++y )
            memcpy( data.get() + y * width, src + y * bytesPerRow, width );
        return data;
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
