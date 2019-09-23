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
        nCrc = vcl_get_checksum(nCrc, pBuf->mpBits, pBuf->mnScanlineSize * pBuf->mnHeight);
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

} // namespace

std::unique_ptr< sal_uInt8[] > SalBitmap::convertDataTo24Bpp( const sal_uInt8* src,
    int width, int height, int bitCount, int bytesPerRow, const BitmapPalette& palette, bool toBgr )
{
    std::unique_ptr< sal_uInt8[] > data( new sal_uInt8[width * height * 3] );
    std::unique_ptr<ImplPixelFormat> pSrcFormat(ImplPixelFormat::GetFormat(bitCount, palette));

    const sal_uInt8* pSrcData = src;
    sal_uInt8* pDstData = data.get();

    sal_uInt32 nY = height;
    while( nY-- )
    {
        pSrcFormat->StartLine( pSrcData );

        sal_uInt32 nX = width;
        if (toBgr)
        {
            while( nX-- )
            {
                const BitmapColor& c = pSrcFormat->ReadPixel();
                *pDstData++ = c.GetBlue();
                *pDstData++ = c.GetGreen();
                *pDstData++ = c.GetRed();
            }
        }
        else // RGB
        {
            while( nX-- )
            {
                const BitmapColor& c = pSrcFormat->ReadPixel();
                *pDstData++ = c.GetRed();
                *pDstData++ = c.GetGreen();
                *pDstData++ = c.GetBlue();
            }
        }

        pSrcData += bytesPerRow;
    }
    return data;
}

std::unique_ptr< sal_uInt8[] > SalBitmap::convertDataTo32Bpp( const sal_uInt8* src,
    int width, int height, int bitCount, int bytesPerRow, const BitmapPalette& palette, bool toBgra )
{
    std::unique_ptr< sal_uInt8[] > data( new sal_uInt8[width * height * 4] );
    std::unique_ptr<ImplPixelFormat> pSrcFormat(ImplPixelFormat::GetFormat(bitCount, palette));

    const sal_uInt8* pSrcData = src;
    sal_uInt8* pDstData = data.get();

    sal_uInt32 nY = height;
    while( nY-- )
    {
        pSrcFormat->StartLine( pSrcData );

        sal_uInt32 nX = width;
        if (toBgra)
        {
            while( nX-- )
            {
                const BitmapColor& c = pSrcFormat->ReadPixel();
                *pDstData++ = c.GetBlue();
                *pDstData++ = c.GetGreen();
                *pDstData++ = c.GetRed();
                *pDstData++ = 0xff;
            }
        }
        else // RGBA
        {
            while( nX-- )
            {
                const BitmapColor& c = pSrcFormat->ReadPixel();
                *pDstData++ = c.GetRed();
                *pDstData++ = c.GetGreen();
                *pDstData++ = c.GetBlue();
                *pDstData++ = 0xff;
            }
        }

        pSrcData += bytesPerRow;
    }
    return data;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
