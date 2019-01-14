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

#include "pnghelper.hxx"
#include <sal/macros.h>

#include <zlib.h>

using namespace pdfi;

// checksum helpers, courtesy of libpng.org

/* Table of CRCs of all 8-bit messages. */
sal_uInt32 PngHelper::crc_table[256];

/* Flag: has the table been computed? Initially false. */
bool PngHelper::bCRCTableInit = true;

/* Make the table for a fast CRC. */
void PngHelper::initCRCTable()
{
    for (sal_uInt32 n = 0; n < 256; n++)
    {
        sal_uInt32 c = n;
        for (int k = 0; k < 8; k++)
        {
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc_table[n] = c;
    }
    bCRCTableInit = false;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
  should be initialized to all 1's, and the transmitted value
  is the 1's complement of the final running CRC (see the
  crc() routine below)). */

void PngHelper::updateCRC( sal_uInt32& io_rCRC, const sal_uInt8* i_pBuf, size_t i_nLen )
{
    if( bCRCTableInit )
        initCRCTable();

    sal_uInt32 nCRC = io_rCRC;
    for( size_t n = 0; n < i_nLen; n++ )
        nCRC = crc_table[(nCRC ^ i_pBuf[n]) & 0xff] ^ (nCRC >> 8);
    io_rCRC = nCRC;
}

sal_uInt32 PngHelper::getCRC( const sal_uInt8* i_pBuf, size_t i_nLen )
{
    sal_uInt32 nCRC = 0xffffffff;
    updateCRC( nCRC, i_pBuf, i_nLen );
    return nCRC ^ 0xffffffff;
}

sal_uInt32 PngHelper::deflateBuffer( const Output_t* i_pBuf, size_t i_nLen, OutputBuffer& o_rOut )
{
    size_t nOrigSize = o_rOut.size();

    // prepare z stream
    z_stream aStream;
    aStream.zalloc  = Z_NULL;
    aStream.zfree   = Z_NULL;
    aStream.opaque  = Z_NULL;
    deflateInit( &aStream, Z_BEST_COMPRESSION );
    aStream.avail_in = uInt(i_nLen);
    aStream.next_in = const_cast<Bytef*>(i_pBuf);

    sal_uInt8 aOutBuf[ 32768 ];
    do
    {
        aStream.avail_out = sizeof( aOutBuf );
        aStream.next_out = aOutBuf;

        if( deflate( &aStream, Z_FINISH ) == Z_STREAM_ERROR )
        {
            deflateEnd( &aStream );
            // scrap the data of this broken stream
            o_rOut.resize( nOrigSize );
            return 0;
        }

        // append compressed bytes
        sal_uInt32 nCompressedBytes = sizeof( aOutBuf ) - aStream.avail_out;
        if( nCompressedBytes )
            o_rOut.insert( o_rOut.end(), aOutBuf, aOutBuf+nCompressedBytes );

    } while( aStream.avail_out == 0 );

    // cleanup
    deflateEnd( &aStream );

    return sal_uInt32( o_rOut.size() - nOrigSize );
}

void PngHelper::appendFileHeader( OutputBuffer& o_rOutputBuf )
{
    static const unsigned char aHeader[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };

    o_rOutputBuf.insert( o_rOutputBuf.end(), aHeader, aHeader + SAL_N_ELEMENTS(aHeader) );
}

size_t PngHelper::startChunk( const char* pChunkName, OutputBuffer& o_rOutputBuf )
{
    size_t nIndex = sal_uInt32( o_rOutputBuf.size() );
    o_rOutputBuf.insert( o_rOutputBuf.end(), 4, Output_t(0) );
    o_rOutputBuf.push_back( pChunkName[0] );
    o_rOutputBuf.push_back( pChunkName[1] );
    o_rOutputBuf.push_back( pChunkName[2] );
    o_rOutputBuf.push_back( pChunkName[3] );
    return nIndex;
}

void PngHelper::set( sal_uInt32 i_nValue, OutputBuffer& o_rOutputBuf, size_t i_nIndex )
{
    o_rOutputBuf[ i_nIndex   ] = (i_nValue & 0xff000000) >> 24;
    o_rOutputBuf[ i_nIndex+1 ] = (i_nValue & 0x00ff0000) >> 16;
    o_rOutputBuf[ i_nIndex+2 ] = (i_nValue & 0x0000ff00) >> 8;
    o_rOutputBuf[ i_nIndex+3 ] = (i_nValue & 0x000000ff);
}

void PngHelper::endChunk( size_t nStart, OutputBuffer& o_rOutputBuf )
{
    if( nStart+8 > o_rOutputBuf.size() )
        return; // something broken is going on

    // update chunk length
    size_t nLen = o_rOutputBuf.size() - nStart;
    sal_uInt32 nDataLen = sal_uInt32(nLen)-8;
    set( nDataLen, o_rOutputBuf, nStart );

    // append chunk crc
    sal_uInt32 nChunkCRC = getCRC( reinterpret_cast<sal_uInt8*>(&o_rOutputBuf[nStart+4]), nLen-4 );
    append( nChunkCRC, o_rOutputBuf );
}

void PngHelper::appendIHDR( OutputBuffer& o_rOutputBuf, int width, int height, int depth, int colortype )
{
    size_t nStart = startChunk( "IHDR", o_rOutputBuf );
    append( width, o_rOutputBuf );
    append( height, o_rOutputBuf );
    o_rOutputBuf.push_back( Output_t(depth) );
    o_rOutputBuf.push_back( Output_t(colortype) );
    o_rOutputBuf.push_back( 0 ); // compression method deflate
    o_rOutputBuf.push_back( 0 ); // filtering method 0 (default)
    o_rOutputBuf.push_back( 0 ); // no interlacing
    endChunk( nStart, o_rOutputBuf );
}

void PngHelper::appendIEND( OutputBuffer& o_rOutputBuf )
{
    size_t nStart = startChunk( "IEND", o_rOutputBuf );
    endChunk( nStart, o_rOutputBuf );
}

void PngHelper::createPng( OutputBuffer&     o_rOutputBuf,
                           Stream*           str,
                           int               width,
                           int               height,
                           GfxRGB const &    zeroColor,
                           GfxRGB const &    oneColor,
                           bool              bIsMask
                           )
{
    appendFileHeader( o_rOutputBuf );
    appendIHDR( o_rOutputBuf, width, height, 1, 3 );

    // write palette
    size_t nIdx = startChunk( "PLTE", o_rOutputBuf );
    // write colors 0 and 1
    o_rOutputBuf.push_back(colToByte(zeroColor.r));
    o_rOutputBuf.push_back(colToByte(zeroColor.g));
    o_rOutputBuf.push_back(colToByte(zeroColor.b));
    o_rOutputBuf.push_back(colToByte(oneColor.r));
    o_rOutputBuf.push_back(colToByte(oneColor.g));
    o_rOutputBuf.push_back(colToByte(oneColor.b));
    // end PLTE chunk
    endChunk( nIdx, o_rOutputBuf );

    if( bIsMask )
    {
        // write tRNS chunk
        nIdx = startChunk( "tRNS", o_rOutputBuf );
        o_rOutputBuf.push_back( 0xff );
        o_rOutputBuf.push_back( 0 );
        // end tRNS chunk
        endChunk( nIdx, o_rOutputBuf );
    }

    // create scan line data buffer
    OutputBuffer aScanlines;
    int nLineSize = (width + 7)/8;
    aScanlines.reserve( nLineSize * height + height );

    str->reset();
    for( int y = 0; y < height; y++ )
    {
        // determine filter type (none) for this scanline
        aScanlines.push_back( 0 );
        for( int x = 0; x < nLineSize; x++ )
            aScanlines.push_back( str->getChar() );
    }

    // begin IDAT chunk for scanline data
    nIdx = startChunk( "IDAT", o_rOutputBuf );
    // compress scanlines
    deflateBuffer( &aScanlines[0], aScanlines.size(), o_rOutputBuf );
    // end IDAT chunk
    endChunk( nIdx, o_rOutputBuf );

    // output IEND
    appendIEND( o_rOutputBuf );
}

void PngHelper::createPng( OutputBuffer& o_rOutputBuf,
                           Stream* str,
                           int width, int height, GfxImageColorMap* colorMap,
                           Stream* maskStr,
                           int maskWidth, int maskHeight, GfxImageColorMap* maskColorMap )
{
    appendFileHeader( o_rOutputBuf );
    appendIHDR( o_rOutputBuf, width, height, 8, 6 ); // RGBA image

    // initialize stream
    unsigned char *p, *pm;
    GfxRGB rgb;
    GfxGray alpha;
    std::unique_ptr<ImageStream> imgStr(
        new ImageStream(str,
                        width,
                        colorMap->getNumPixelComps(),
                        colorMap->getBits()));
    imgStr->reset();

    // create scan line data buffer
    OutputBuffer aScanlines;
    aScanlines.reserve( width*height*4 + height );

    for( int y=0; y<height; ++y)
    {
        aScanlines.push_back( 0 );
        p = imgStr->getLine();
        for( int x=0; x<width; ++x)
        {
            colorMap->getRGB(p, &rgb);
            aScanlines.push_back(colToByte(rgb.r));
            aScanlines.push_back(colToByte(rgb.g));
            aScanlines.push_back(colToByte(rgb.b));
            aScanlines.push_back( 0xff );

            p +=colorMap->getNumPixelComps();
        }
    }


    // now fill in the mask data

    // CAUTION: originally this was done in one single loop
    // it caused merry chaos; the reason is that maskStr and str are
    // not independent streams, it happens that reading one advances
    // the other, too. Hence the two passes are imperative !

    // initialize mask stream
    std::unique_ptr<ImageStream> imgStrMask(
        new ImageStream(maskStr,
                        maskWidth,
                        maskColorMap->getNumPixelComps(),
                        maskColorMap->getBits()));

    imgStrMask->reset();
    for( int y = 0; y < maskHeight; ++y )
    {
        pm = imgStrMask->getLine();
        for( int x = 0; x < maskWidth; ++x )
        {
            maskColorMap->getGray(pm,&alpha);
            pm += maskColorMap->getNumPixelComps();
            int nIndex = (y*height/maskHeight) * (width*4+1) + // mapped line
                         (x*width/maskWidth)*4 + 1  + 3        // mapped column
                         ;
            aScanlines[ nIndex ] = colToByte(alpha);
        }
    }

    imgStr.reset();
    imgStrMask.reset();

    // begind IDAT chunk for scanline data
    size_t nIdx = startChunk( "IDAT", o_rOutputBuf );
    // compress scanlines
    deflateBuffer( &aScanlines[0], aScanlines.size(), o_rOutputBuf );
    // end IDAT chunk
    endChunk( nIdx, o_rOutputBuf );
    // output IEND
    appendIEND( o_rOutputBuf );
}

// one bit mask; 0 bits opaque
void PngHelper::createPng( OutputBuffer& o_rOutputBuf,
                           Stream* str,
                           int width, int height, GfxImageColorMap* colorMap,
                           Stream* maskStr,
                           int maskWidth, int maskHeight,
                           bool maskInvert
                          )
{
    appendFileHeader( o_rOutputBuf );
    appendIHDR( o_rOutputBuf, width, height, 8, 6 ); // RGBA image

    // initialize stream
    unsigned char *p;
    GfxRGB rgb;
    std::unique_ptr<ImageStream> imgStr(
        new ImageStream(str,
                        width,
                        colorMap->getNumPixelComps(),
                        colorMap->getBits()));
    imgStr->reset();

    // create scan line data buffer
    OutputBuffer aScanlines;
    aScanlines.reserve( width*height*4 + height );

    for( int y=0; y<height; ++y)
    {
        aScanlines.push_back( 0 );
        p = imgStr->getLine();
        for( int x=0; x<width; ++x)
        {
            colorMap->getRGB(p, &rgb);
            aScanlines.push_back(colToByte(rgb.r));
            aScanlines.push_back(colToByte(rgb.g));
            aScanlines.push_back(colToByte(rgb.b));
            aScanlines.push_back( 0xff );

            p +=colorMap->getNumPixelComps();
        }
    }


    // now fill in the mask data

    // CAUTION: originally this was done in one single loop
    // it caused merry chaos; the reason is that maskStr and str are
    // not independent streams, it happens that reading one advances
    // the other, too. Hence the two passes are imperative !

    // initialize mask stream
    std::unique_ptr<ImageStream> imgStrMask(
        new ImageStream(maskStr, maskWidth, 1, 1));

    imgStrMask->reset();
    for( int y = 0; y < maskHeight; ++y )
    {
        for( int x = 0; x < maskWidth; ++x )
        {
            unsigned char aPixel = 0;
            imgStrMask->getPixel( &aPixel );
            int nIndex = (y*height/maskHeight) * (width*4+1) + // mapped line
                         (x*width/maskWidth)*4 + 1  + 3        // mapped column
                         ;
            if( maskInvert )
                aScanlines[ nIndex ] = aPixel ? 0xff : 0x00;
            else
                aScanlines[ nIndex ] = aPixel ? 0x00 : 0xff;
        }
    }

    imgStr.reset();
    imgStrMask.reset();

    // begind IDAT chunk for scanline data
    size_t nIdx = startChunk( "IDAT", o_rOutputBuf );
    // compress scanlines
    deflateBuffer( &aScanlines[0], aScanlines.size(), o_rOutputBuf );
    // end IDAT chunk
    endChunk( nIdx, o_rOutputBuf );
    // output IEND
    appendIEND( o_rOutputBuf );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
