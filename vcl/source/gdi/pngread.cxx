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

#include <vcl/pngread.hxx>

#include <cmath>
#include <rtl/crc.h>
#include <rtl/alloc.h>
#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/svapp.hxx>
#include <vcl/alpha.hxx>
#include <osl/endian.h>

namespace vcl
{

#define PNGCHUNK_IHDR       0x49484452
#define PNGCHUNK_PLTE       0x504c5445
#define PNGCHUNK_IDAT       0x49444154
#define PNGCHUNK_IEND       0x49454e44
#define PNGCHUNK_bKGD       0x624b4744
#define PNGCHUNK_gAMA       0x67414d41
#define PNGCHUNK_pHYs       0x70485973
#define PNGCHUNK_tRNS       0x74524e53

#define VIEWING_GAMMA       2.35
#define DISPLAY_GAMMA       1.0


static const sal_uInt8 mpDefaultColorTable[ 256 ] =
{   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

class PNGReaderImpl
{
private:
    SvStream&           mrPNGStream;
    SvStreamEndian      mnOrigStreamMode;

    std::vector<vcl::PNGReader::ChunkData> maChunkSeq;
    std::vector<vcl::PNGReader::ChunkData>::iterator maChunkIter;
    std::vector<sal_uInt8>::iterator maDataIter;

    Bitmap*             mpBmp;
    BitmapWriteAccess*  mpAcc;
    Bitmap*             mpMaskBmp;
    AlphaMask*          mpAlphaMask;
    BitmapWriteAccess*  mpMaskAcc;
    ZCodec              mpZCodec;
    sal_uInt8*          mpInflateInBuf; // as big as the size of a scanline + alphachannel + 1
    sal_uInt8*          mpScanPrior;    // pointer to the latest scanline
    sal_uInt8*          mpTransTab;     // for transparency in images with palette colortype
    sal_uInt8*          mpScanCurrent;  // pointer into the current scanline
    sal_uInt8*          mpColorTable;
    sal_Size            mnStreamSize;   // estimate of PNG file size
    sal_uInt32          mnChunkType;    // Type of current PNG chunk
    sal_Int32           mnChunkLen;     // Length of current PNG chunk
    Size                maOrigSize;     // pixel size of the full image
    Size                maTargetSize;   // pixel size of the result image
    Size                maPhysSize;     // preferred size in MAP_100TH_MM units
    sal_uInt32          mnBPP;          // number of bytes per pixel
    sal_uInt32          mnScansize;     // max size of scanline
    sal_uInt32          mnYpos;         // latest y position in full image
    int                 mnPass;         // if interlaced the latest pass ( 1..7 ) else 7
    sal_uInt32          mnXStart;       // the starting X for the current pass
    sal_uInt32          mnXAdd;         // the increment for input images X coords for the current pass
    sal_uInt32          mnYAdd;         // the increment for input images Y coords for the current pass
    int                 mnPreviewShift; // shift to convert orig image coords into preview image coords
    int                 mnPreviewMask;  // == ((1 << mnPreviewShift) - 1)
    sal_uInt16          mnTargetDepth;      // pixel depth of target bitmap
    sal_uInt8           mnTransRed;
    sal_uInt8           mnTransGreen;
    sal_uInt8           mnTransBlue;
    sal_uInt8           mnPngDepth;     // pixel depth of PNG data
    sal_uInt8           mnColorType;
    sal_uInt8           mnCompressionType;
    sal_uInt8           mnFilterType;
    sal_uInt8           mnInterlaceType;
    BitmapColor         mcTranspColor;  // transparency mask's transparency "color"
    BitmapColor         mcOpaqueColor;  // transparency mask's opaque "color"
    bool                mbTransparent : 1;  // graphic includes an tRNS Chunk or an alpha Channel
    bool                mbAlphaChannel : 1; // is true for ColorType 4 and 6
    bool                mbRGBTriple : 1;
    bool                mbPalette : 1;      // false if we need a Palette
    bool                mbGrayScale : 1;
    bool                mbzCodecInUse : 1;
    bool                mbStatus : 1;
    bool                mbIDAT : 1;         // true if finished with enough IDAT chunks
    bool                mbGamma : 1;        // true if Gamma Correction available
    bool                mbpHYs : 1;         // true if physical size of pixel available
    bool                mbIgnoreGammaChunk : 1;

#if OSL_DEBUG_LEVEL > 0
    // do some checks in debug mode
    sal_Int32           mnAllocSizeScanline;
    sal_Int32           mnAllocSizeScanlineAlpha;
#endif
    // the temporary Scanline (and alpha) for direct scanline copy to Bitmap
    sal_uInt8*          mpScanline;
    sal_uInt8*          mpScanlineAlpha;

    bool                ReadNextChunk();
    void                ReadRemainingChunks();

    void                ImplSetPixel( sal_uInt32 y, sal_uInt32 x, const BitmapColor & );
    void                ImplSetPixel( sal_uInt32 y, sal_uInt32 x, sal_uInt8 nPalIndex );
    void                ImplSetTranspPixel( sal_uInt32 y, sal_uInt32 x, const BitmapColor &, bool bTrans );
    void                ImplSetAlphaPixel( sal_uInt32 y, sal_uInt32 x, sal_uInt8 nPalIndex, sal_uInt8 nAlpha );
    void                ImplSetAlphaPixel( sal_uInt32 y, sal_uInt32 x, const BitmapColor&, sal_uInt8 nAlpha );
    void                ImplReadIDAT();
    bool                ImplPreparePass();
    void                ImplApplyFilter();
    void                ImplDrawScanline( sal_uInt32 nXStart, sal_uInt32 nXAdd );
    bool                ImplReadTransparent();
    void                ImplGetGamma();
    void                ImplGetBackground();
    sal_uInt8           ImplScaleColor();
    bool                ImplReadHeader( const Size& rPreviewSizeHint );
    bool                ImplReadPalette();
    void                ImplGetGrayPalette( sal_uInt16 );
    sal_uInt32          ImplReadsal_uInt32();

public:

    explicit PNGReaderImpl( SvStream& );
    ~PNGReaderImpl();

    BitmapEx            GetBitmapEx( const Size& rPreviewSizeHint );
    const std::vector<vcl::PNGReader::ChunkData>& GetAllChunks();
    void                SetIgnoreGammaChunk( bool bIgnore ){ mbIgnoreGammaChunk = bIgnore; };
};

PNGReaderImpl::PNGReaderImpl( SvStream& rPNGStream )
:   mrPNGStream( rPNGStream ),
    mpBmp           ( nullptr ),
    mpAcc           ( nullptr ),
    mpMaskBmp       ( nullptr ),
    mpAlphaMask     ( nullptr ),
    mpMaskAcc       ( nullptr ),
    mpInflateInBuf  ( nullptr ),
    mpScanPrior     ( nullptr ),
    mpTransTab      ( nullptr ),
    mpScanCurrent   ( nullptr ),
    mpColorTable    ( const_cast<sal_uInt8*>(mpDefaultColorTable) ),
    mnChunkType     ( 0 ),
    mnChunkLen      ( 0 ),
    mnBPP           ( 0 ),
    mnScansize      ( 0 ),
    mnYpos          ( 0 ),
    mnPass          ( 0 ),
    mnXStart        ( 0 ),
    mnXAdd          ( 0 ),
    mnYAdd          ( 0 ),
    mnTargetDepth   ( 0 ),
    mnTransRed      ( 0 ),
    mnTransGreen    ( 0 ),
    mnTransBlue     ( 0 ),
    mnPngDepth      ( 0 ),
    mnColorType     ( 0 ),
    mnCompressionType( 0 ),
    mnFilterType    ( 0 ),
    mnInterlaceType ( 0 ),
    mbTransparent( false ),
    mbAlphaChannel( false ),
    mbRGBTriple( false ),
    mbPalette( false ),
    mbGrayScale( false ),
    mbzCodecInUse   ( false ),
    mbStatus( true ),
    mbIDAT( false ),
    mbGamma             ( false ),
    mbpHYs              ( false ),
    mbIgnoreGammaChunk  ( false ),
#if OSL_DEBUG_LEVEL > 0
    mnAllocSizeScanline(0),
    mnAllocSizeScanlineAlpha(0),
#endif
    mpScanline(nullptr),
    mpScanlineAlpha(nullptr)
{
    // prepare the PNG data stream
    mnOrigStreamMode = mrPNGStream.GetEndian();
    mrPNGStream.SetEndian( SvStreamEndian::BIG );

    // prepare the chunk reader
    maChunkSeq.reserve( 16 );
    maChunkIter = maChunkSeq.begin();

    // estimate PNG file size (to allow sanity checks)
    const sal_Size nStreamPos = mrPNGStream.Tell();
    mrPNGStream.Seek( STREAM_SEEK_TO_END );
    mnStreamSize = mrPNGStream.Tell();
    mrPNGStream.Seek( nStreamPos );

    // check the PNG header magic
    sal_uInt32 nDummy = 0;
    mrPNGStream.ReadUInt32( nDummy );
    mbStatus = (nDummy == 0x89504e47);
    mrPNGStream.ReadUInt32( nDummy );
    mbStatus = (nDummy == 0x0d0a1a0a) && mbStatus;

    mnPreviewShift = 0;
    mnPreviewMask = (1 << mnPreviewShift) - 1;
}

PNGReaderImpl::~PNGReaderImpl()
{
    mrPNGStream.SetEndian( mnOrigStreamMode );

    if ( mbzCodecInUse )
        mpZCodec.EndCompression();

    if( mpColorTable != mpDefaultColorTable )
        delete[] mpColorTable;

    delete mpBmp;
    delete mpAlphaMask;
    delete mpMaskBmp;
    delete[] mpTransTab;
    delete[] mpInflateInBuf;
    delete[] mpScanPrior;

    delete[] mpScanline;
    delete[] mpScanlineAlpha;
}

bool PNGReaderImpl::ReadNextChunk()
{
    if( maChunkIter == maChunkSeq.end() )
    {
        // get the next chunk from the stream

        // unless we are at the end of the PNG stream
        if( mrPNGStream.IsEof() || (mrPNGStream.GetError() != ERRCODE_NONE) )
            return false;
        if( !maChunkSeq.empty() && (maChunkSeq.back().nType == PNGCHUNK_IEND) )
            return false;

        PNGReader::ChunkData aDummyChunk;
        maChunkIter = maChunkSeq.insert( maChunkSeq.end(), aDummyChunk );
        PNGReader::ChunkData& rChunkData = *maChunkIter;

        // read the chunk header
        mrPNGStream.ReadInt32( mnChunkLen ).ReadUInt32( mnChunkType );
        rChunkData.nType = mnChunkType;

        // fdo#61847 truncate over-long, trailing chunks
        const sal_Size nStreamPos = mrPNGStream.Tell();
        if( mnChunkLen < 0 || nStreamPos + mnChunkLen >= mnStreamSize )
            mnChunkLen = mnStreamSize - nStreamPos;

        // calculate chunktype CRC (swap it back to original byte order)
        sal_uInt32 nChunkType = mnChunkType;
        #if defined(__LITTLEENDIAN) || defined(OSL_LITENDIAN)
        nChunkType = OSL_SWAPDWORD( nChunkType );
        #endif
        sal_uInt32 nCRC32 = rtl_crc32( 0, &nChunkType, 4 );

        // read the chunk data and check the CRC
        if( mnChunkLen && !mrPNGStream.IsEof() )
        {
            rChunkData.aData.resize( mnChunkLen );

            sal_Int32 nBytesRead = 0;
            do {
                sal_uInt8* pPtr = &rChunkData.aData[ nBytesRead ];
                nBytesRead += mrPNGStream.Read( pPtr, mnChunkLen - nBytesRead );
            } while ( ( nBytesRead < mnChunkLen ) && ( mrPNGStream.GetError() == ERRCODE_NONE ) );

            nCRC32 = rtl_crc32( nCRC32, &rChunkData.aData[ 0 ], mnChunkLen );
            maDataIter = rChunkData.aData.begin();
        }
        sal_uInt32 nCheck(0);
        mrPNGStream.ReadUInt32( nCheck );
        if( nCRC32 != nCheck )
            return false;
    }
    else
    {
        // the next chunk was already read
        mnChunkType = (*maChunkIter).nType;
        mnChunkLen = (*maChunkIter).aData.size();
        maDataIter = (*maChunkIter).aData.begin();
    }

    ++maChunkIter;
    if( mnChunkType == PNGCHUNK_IEND )
        return false;
    return true;
}

// read the remaining chunks from mrPNGStream
void PNGReaderImpl::ReadRemainingChunks()
{
    while( ReadNextChunk() ) ;
}

const std::vector< vcl::PNGReader::ChunkData >& PNGReaderImpl::GetAllChunks()
{
    ReadRemainingChunks();
    return maChunkSeq;
}

BitmapEx PNGReaderImpl::GetBitmapEx( const Size& rPreviewSizeHint )
{
    // reset to the first chunk
    maChunkIter = maChunkSeq.begin();

    // first chunk must be IDHR
    if( mbStatus && ReadNextChunk() )
    {
        if (mnChunkType == PNGCHUNK_IHDR)
            mbStatus = ImplReadHeader( rPreviewSizeHint );
        else
            mbStatus = false;
    }

    // parse the remaining chunks
    while (mbStatus && !mbIDAT && ReadNextChunk())
    {
        switch( mnChunkType )
        {
            case PNGCHUNK_IHDR :
            {
                mbStatus = false; //IHDR should only appear as the first chunk
            }
            break;

            case PNGCHUNK_gAMA :                                // the gamma chunk must precede
            {                                                   // the 'IDAT' and also the 'PLTE'(if available )
                if ( !mbIgnoreGammaChunk && !mbIDAT )
                    ImplGetGamma();
            }
            break;

            case PNGCHUNK_PLTE :
            {
                if ( !mbPalette )
                    mbStatus = ImplReadPalette();
            }
            break;

            case PNGCHUNK_tRNS :
            {
                if ( !mbIDAT )                                  // the tRNS chunk must precede the IDAT
                    mbStatus = ImplReadTransparent();
            }
            break;

            case PNGCHUNK_bKGD :                                // the background chunk must appear
            {
                if ( !mbIDAT && mbPalette )         // before the 'IDAT' and after the
                    ImplGetBackground();                        // PLTE(if available ) chunk.
            }
            break;

            case PNGCHUNK_IDAT :
            {
                if ( !mpInflateInBuf )  // taking care that the header has properly been read
                    mbStatus = false;
                else if ( !mbIDAT )     // the gfx is finished, but there may be left a zlibCRC of about 4Bytes
                    ImplReadIDAT();
            }
            break;

            case PNGCHUNK_pHYs :
            {
                if ( !mbIDAT && mnChunkLen == 9 )
                {
                    sal_uInt32 nXPixelPerMeter = ImplReadsal_uInt32();
                    sal_uInt32 nYPixelPerMeter = ImplReadsal_uInt32();

                    sal_uInt8 nUnitSpecifier = *maDataIter++;
                    if( (nUnitSpecifier == 1) && nXPixelPerMeter && nYPixelPerMeter )
                    {
                        mbpHYs = true;

                        // convert into MAP_100TH_MM
                        maPhysSize.Width()  = (sal_Int32)( (100000.0 * maOrigSize.Width()) / nXPixelPerMeter );
                        maPhysSize.Height() = (sal_Int32)( (100000.0 * maOrigSize.Height()) / nYPixelPerMeter );
                    }
                }
            }
            break;

            case PNGCHUNK_IEND:
                mbStatus = mbIDAT;  // there is a problem if the image is not complete yet
            break;
        }
    }

    // release write access of the bitmaps
    if ( mpAcc )
    {
        Bitmap::ReleaseAccess( mpAcc );
        mpAcc = nullptr;
    }

    if ( mpMaskAcc )
    {
        if ( mpAlphaMask )
            mpAlphaMask->ReleaseAccess( mpMaskAcc );
        else if ( mpMaskBmp )
            Bitmap::ReleaseAccess( mpMaskAcc );

        mpMaskAcc = nullptr;
    }

    // return the resulting BitmapEx
    BitmapEx aRet;

    if( !mbStatus || !mbIDAT )
        aRet.Clear();
    else
    {
        if ( mpAlphaMask )
            aRet = BitmapEx( *mpBmp, *mpAlphaMask );
        else if ( mpMaskBmp )
            aRet = BitmapEx( *mpBmp, *mpMaskBmp );
        else
            aRet = *mpBmp;

        if ( mbpHYs && maPhysSize.Width() && maPhysSize.Height() )
        {
            aRet.SetPrefMapMode( MAP_100TH_MM );
            aRet.SetPrefSize( maPhysSize );
        }
    }
    return aRet;
}

bool PNGReaderImpl::ImplReadHeader( const Size& rPreviewSizeHint )
{
    if( mnChunkLen < 13 )
        return false;

    maOrigSize.Width()  = ImplReadsal_uInt32();
    maOrigSize.Height() = ImplReadsal_uInt32();

    if (maOrigSize.Width() <= 0 || maOrigSize.Height() <= 0)
        return false;

    mnPngDepth = *(maDataIter++);
    mnColorType = *(maDataIter++);

    mnCompressionType = *(maDataIter++);
    if( mnCompressionType != 0 )    // unknown compression type
        return false;

    mnFilterType = *(maDataIter++);
    if( mnFilterType != 0 )         // unknown filter type
        return false;

    mnInterlaceType = *(maDataIter++);
    switch ( mnInterlaceType ) // filter type valid ?
    {
        case 0 :  // progressive image
            mnPass = 7;
            break;
        case 1 :  // Adam7-interlaced image
            mnPass = 0;
            break;
        default:
            return false;
    }

    mbPalette = true;
    mbIDAT = mbAlphaChannel = mbTransparent = false;
    mbGrayScale = mbRGBTriple = false;
    mnTargetDepth = mnPngDepth;
    sal_uInt64 nScansize64 = ( ( static_cast< sal_uInt64 >( maOrigSize.Width() ) * mnPngDepth ) + 7 ) >> 3;

    // valid color types are 0,2,3,4 & 6
    switch ( mnColorType )
    {
        case 0 :    // each pixel is a grayscale
        {
            switch ( mnPngDepth )
            {
                case 2 : // 2bit target not available -> use four bits
                    mnTargetDepth = 4;  // we have to expand the bitmap
                    mbGrayScale = true;
                    break;
                case 16 :
                    mnTargetDepth = 8;  // we have to reduce the bitmap
                    SAL_FALLTHROUGH;
                case 1 :
                case 4 :
                case 8 :
                    mbGrayScale = true;
                    break;
                default :
                    return false;
            }
        }
        break;

        case 2 :    // each pixel is an RGB triple
        {
            mbRGBTriple = true;
            nScansize64 *= 3;
            switch ( mnPngDepth )
            {
                case 16 :           // we have to reduce the bitmap
                case 8 :
                    mnTargetDepth = 24;
                    break;
                default :
                    return false;
            }
        }
        break;

        case 3 :    // each pixel is a palette index
        {
            switch ( mnPngDepth )
            {
                case 2 :
                    mnTargetDepth = 4;  // we have to expand the bitmap
                    SAL_FALLTHROUGH;
                case 1 :
                case 4 :
                case 8 :
                    mbPalette = false;
                    break;
                default :
                    return false;
            }
        }
        break;

        case 4 :    // each pixel is a grayscale sample followed by an alpha sample
        {
            nScansize64 *= 2;
            mbAlphaChannel = true;
            switch ( mnPngDepth )
            {
                case 16 :
                    mnTargetDepth = 8;  // we have to reduce the bitmap
                    SAL_FALLTHROUGH;
                case 8 :
                    mbGrayScale = true;
                    break;
                default :
                    return false;
            }
        }
        break;

        case 6 :    // each pixel is an RGB triple followed by an alpha sample
        {
            mbRGBTriple = true;
            nScansize64 *= 4;
            mbAlphaChannel = true;
            switch (mnPngDepth )
            {
                case 16 :           // we have to reduce the bitmap
                case 8 :
                    mnTargetDepth = 24;
                    break;
                default :
                    return false;
            }
        }
        break;

        default :
            return false;
    }

    mnBPP = static_cast< sal_uInt32 >( nScansize64 / maOrigSize.Width() );
    if ( !mnBPP )
        mnBPP = 1;

    nScansize64++;       // each scanline includes one filterbyte

    if ( nScansize64 > SAL_MAX_UINT32 )
        return false;

    mnScansize = static_cast< sal_uInt32 >( nScansize64 );

    // calculate target size from original size and the preview hint
    if( rPreviewSizeHint.Width() || rPreviewSizeHint.Height() )
    {
        Size aPreviewSize( rPreviewSizeHint.Width(), rPreviewSizeHint.Height() );
        maTargetSize = maOrigSize;

        if( aPreviewSize.Width() == 0 ) {
            aPreviewSize.setWidth( ( maOrigSize.Width()*aPreviewSize.Height() )/maOrigSize.Height() );
            if( aPreviewSize.Width() <= 0 )
                aPreviewSize.setWidth( 1 );
        } else if( aPreviewSize.Height() == 0 ) {
            aPreviewSize.setHeight( ( maOrigSize.Height()*aPreviewSize.Width() )/maOrigSize.Width() );
            if( aPreviewSize.Height() <= 0 )
                aPreviewSize.setHeight( 1 );
        }

        if( aPreviewSize.Width() < maOrigSize.Width() && aPreviewSize.Height() < maOrigSize.Height() ) {
            OSL_TRACE("preview size %ldx%ld", aPreviewSize.Width(), aPreviewSize.Height() );

            for( int i = 1; i < 5; ++i )
                {
                    if( (maTargetSize.Width() >> i) < aPreviewSize.Width() )
                        break;
                    if( (maTargetSize.Height() >> i) < aPreviewSize.Height() )
                        break;
                    mnPreviewShift = i;
                }
            mnPreviewMask = (1 << mnPreviewShift) - 1;
        }
    }

    maTargetSize.Width()  = (maOrigSize.Width() + mnPreviewMask) >> mnPreviewShift;
    maTargetSize.Height() = (maOrigSize.Height() + mnPreviewMask) >> mnPreviewShift;

    //round bits up to nearest multiple of 8 and divide by 8 to get num of bytes per pixel
    int nBytesPerPixel = ((mnTargetDepth + 7) & ~7)/8;

    //stupidly big, forget about it
    if (maTargetSize.Width() >= SAL_MAX_INT32 / nBytesPerPixel / maTargetSize.Height())
    {
        SAL_WARN( "vcl.gdi", "overlarge png dimensions: " <<
            maTargetSize.Width() << " x " << maTargetSize.Height() << " depth: " << mnTargetDepth);
        return false;
    }

    // TODO: switch between both scanlines instead of copying
    mpInflateInBuf = new (std::nothrow) sal_uInt8[ mnScansize ];
    mpScanCurrent = mpInflateInBuf;
    mpScanPrior = new (std::nothrow) sal_uInt8[ mnScansize ];

    if ( !mpInflateInBuf || !mpScanPrior )
        return false;

    mpBmp = new Bitmap( maTargetSize, mnTargetDepth );
    mpAcc = mpBmp->AcquireWriteAccess();
    if( !mpAcc )
        return false;

    if ( mbAlphaChannel )
    {
        mpAlphaMask = new AlphaMask( maTargetSize );
        mpAlphaMask->Erase( 128 );
        mpMaskAcc = mpAlphaMask->AcquireWriteAccess();
        if( !mpMaskAcc )
            return false;
    }

    if ( mbGrayScale )
        ImplGetGrayPalette( mnPngDepth );

    ImplPreparePass();

    return true;
}

void PNGReaderImpl::ImplGetGrayPalette( sal_uInt16 nBitDepth )
{
    if( nBitDepth > 8 )
        nBitDepth = 8;

    sal_uInt16  nPaletteEntryCount = 1 << nBitDepth;
    sal_uInt32  nAdd = nBitDepth ? 256 / (nPaletteEntryCount - 1) : 0;

    // no bitdepth==2 available
    // but bitdepth==4 with two unused bits is close enough
    if( nBitDepth == 2 )
        nPaletteEntryCount = 16;

    mpAcc->SetPaletteEntryCount( nPaletteEntryCount );
    for ( sal_uInt32 i = 0, nStart = 0; nStart < 256; i++, nStart += nAdd )
        mpAcc->SetPaletteColor( (sal_uInt16)i, BitmapColor( mpColorTable[ nStart ],
            mpColorTable[ nStart ], mpColorTable[ nStart ] ) );
}

bool PNGReaderImpl::ImplReadPalette()
{
    sal_uInt16 nCount = static_cast<sal_uInt16>( mnChunkLen / 3 );

    if ( ( ( mnChunkLen % 3 ) == 0 ) && ( ( 0 < nCount ) && ( nCount <= 256 ) ) && mpAcc )
    {
        mbPalette = true;
        mpAcc->SetPaletteEntryCount( (sal_uInt16) nCount );

        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            sal_uInt8 nRed =   mpColorTable[ *maDataIter++ ];
            sal_uInt8 nGreen = mpColorTable[ *maDataIter++ ];
            sal_uInt8 nBlue =  mpColorTable[ *maDataIter++ ];
            mpAcc->SetPaletteColor( i, Color( nRed, nGreen, nBlue ) );
        }
    }
    else
        mbStatus = false;

    return mbStatus;
}

bool PNGReaderImpl::ImplReadTransparent()
{
    bool bNeedAlpha = false;

    if ( mpTransTab == nullptr )
    {
        switch ( mnColorType )
        {
            case 0 :
            {
                if ( mnChunkLen == 2 )
                {
                    mpTransTab = new sal_uInt8[ 256 ];
                    memset( mpTransTab, 0xff, 256);
                    // color type 0 and 4 is always greyscale,
                    // so the return value can be used as index
                    sal_uInt8 nIndex = ImplScaleColor();
                    mpTransTab[ nIndex ] = 0;
                    mbTransparent = true;
                }
            }
            break;

            case 2 :
            {
                if ( mnChunkLen == 6 )
                {
                    mnTransRed = ImplScaleColor();
                    mnTransGreen = ImplScaleColor();
                    mnTransBlue = ImplScaleColor();
                    mbTransparent = true;
                }
            }
            break;

            case 3 :
            {
                if ( mnChunkLen <= 256 )
                {
                    mbTransparent = true;
                    mpTransTab = new sal_uInt8 [ 256 ];
                    memset( mpTransTab, 0xff, 256 );
                    if (mnChunkLen > 0)
                    {
                        memcpy( mpTransTab, &(*maDataIter), mnChunkLen );
                        maDataIter += mnChunkLen;
                        // need alpha transparency if not on/off masking
                        for( int i = 0; i < mnChunkLen; ++i )
                           bNeedAlpha |= (mpTransTab[i]!=0x00) && (mpTransTab[i]!=0xFF);
                    }
                }
            }
            break;
        }
    }

    if( mbTransparent && !mbAlphaChannel && !mpMaskBmp )
    {
        if( bNeedAlpha)
        {
            mpAlphaMask = new AlphaMask( maTargetSize );
            mpMaskAcc = mpAlphaMask->AcquireWriteAccess();
        }
        else
        {
            mpMaskBmp = new Bitmap( maTargetSize, 1 );
            mpMaskAcc = mpMaskBmp->AcquireWriteAccess();
        }
        mbTransparent = (mpMaskAcc != nullptr);
        if( !mbTransparent )
            return false;
        mcOpaqueColor = BitmapColor( 0x00 );
        mcTranspColor = BitmapColor( 0xFF );
        mpMaskAcc->Erase( 0x00 );
    }

    return true;
}

void PNGReaderImpl::ImplGetGamma()
{
    if( mnChunkLen < 4 )
        return;

    sal_uInt32  nGammaValue = ImplReadsal_uInt32();
    double      fGamma = ( ( VIEWING_GAMMA / DISPLAY_GAMMA ) * ( (double)nGammaValue / 100000 ) );
    double      fInvGamma = ( fGamma <= 0.0 || fGamma > 10.0 ) ? 1.0 : ( 1.0 / fGamma );

    if ( fInvGamma != 1.0 )
    {
        mbGamma = true;

        if ( mpColorTable == mpDefaultColorTable )
            mpColorTable = new sal_uInt8[ 256 ];

        for ( sal_Int32 i = 0; i < 256; i++ )
            mpColorTable[ i ] = (sal_uInt8)(pow((double)i/255.0, fInvGamma) * 255.0 + 0.5);

        if ( mbGrayScale )
            ImplGetGrayPalette( mnPngDepth );
    }
}

void PNGReaderImpl::ImplGetBackground()
{
    switch ( mnColorType )
    {
        case 3 :
        {
            if ( mnChunkLen == 1 )
            {
                sal_uInt16 nCol = *maDataIter++;
                if ( nCol < mpAcc->GetPaletteEntryCount() )
                {
                    mpAcc->Erase( mpAcc->GetPaletteColor( (sal_uInt8)nCol ) );
                    break;
                }
            }
        }
        break;

        case 0 :
        case 4 :
        {
            if ( mnChunkLen == 2 )
            {
                // the color type 0 and 4 is always greyscale,
                // so the return value can be used as index
                sal_uInt8 nIndex = ImplScaleColor();
                mpAcc->Erase( mpAcc->GetPaletteColor( nIndex ) );
            }
        }
        break;

        case 2 :
        case 6 :
        {
            if ( mnChunkLen == 6 )
            {
                sal_uInt8 nRed = ImplScaleColor();
                sal_uInt8 nGreen = ImplScaleColor();
                sal_uInt8 nBlue = ImplScaleColor();
                mpAcc->Erase( Color( nRed, nGreen, nBlue ) );
            }
        }
        break;
    }
}

// for color type 0 and 4 (greyscale) the return value is always index to the color
//                2 and 6 (RGB)       the return value is always the 8 bit color component
sal_uInt8 PNGReaderImpl::ImplScaleColor()
{
    sal_uInt32 nMask = ( ( 1 << mnPngDepth ) - 1 );
    sal_uInt16 nCol = ( *maDataIter++ << 8 );

    nCol += *maDataIter++ & (sal_uInt16)nMask;

    if ( mnPngDepth > 8 )   // convert 16bit graphics to 8
        nCol >>= 8;

    return (sal_uInt8) nCol;
}

// ImplReadIDAT reads as much image data as needed

void PNGReaderImpl::ImplReadIDAT()
{
    if( mnChunkLen > 0 )
    {
        if ( !mbzCodecInUse )
        {
            mbzCodecInUse = true;
            mpZCodec.BeginCompression( ZCODEC_NO_COMPRESSION, true );
        }
        mpZCodec.SetBreak( mnChunkLen );
        SvMemoryStream aIStrm( &(*maDataIter), mnChunkLen, StreamMode::READ );

        while ( ( mpZCodec.GetBreak() ) )
        {
            // get bytes needed to fill the current scanline
            sal_Int32 nToRead = mnScansize - (mpScanCurrent - mpInflateInBuf);
            sal_Int32 nRead = mpZCodec.ReadAsynchron( aIStrm, mpScanCurrent, nToRead );
            if ( nRead < 0 )
            {
                mbStatus = false;
                break;
            }
            if ( nRead < nToRead )
            {
                mpScanCurrent += nRead; // more ZStream data in the next IDAT chunk
                break;
            }
            else  // this scanline is Finished
            {
                mpScanCurrent = mpInflateInBuf;
                ImplApplyFilter();

                ImplDrawScanline( mnXStart, mnXAdd );
                mnYpos += mnYAdd;
            }

            if ( mnYpos >= (sal_uInt32)maOrigSize.Height() )
            {
                if( (mnPass < 7) && mnInterlaceType )
                    if( ImplPreparePass() )
                        continue;
                mbIDAT = true;
                break;
            }
        }
    }

    if( mbIDAT )
    {
        mpZCodec.EndCompression();
        mbzCodecInUse = false;
    }
}

bool PNGReaderImpl::ImplPreparePass()
{
    struct InterlaceParams{ int mnXStart, mnYStart, mnXAdd, mnYAdd; };
    static const InterlaceParams aInterlaceParams[8] =
    {
        // non-interlaced
        { 0, 0, 1, 1 },
        // Adam7-interlaced
        { 0, 0, 8, 8 },    // pass 1
        { 4, 0, 8, 8 },    // pass 2
        { 0, 4, 4, 8 },    // pass 3
        { 2, 0, 4, 4 },    // pass 4
        { 0, 2, 2, 4 },    // pass 5
        { 1, 0, 2, 2 },    // pass 6
        { 0, 1, 1, 2 }     // pass 7
    };

    const InterlaceParams* pParam = &aInterlaceParams[ 0 ];
    if( mnInterlaceType )
    {
        while( ++mnPass <= 7 )
        {
            pParam = &aInterlaceParams[ mnPass ];

            // skip this pass if the original image is too small for it
            if( (pParam->mnXStart < maOrigSize.Width())
            &&  (pParam->mnYStart < maOrigSize.Height()) )
                break;
        }
        if( mnPass > 7 )
            return false;

        // skip the last passes if possible (for scaled down target images)
        if( mnPreviewMask & (pParam->mnXStart | pParam->mnYStart) )
            return false;
    }

    mnYpos      = pParam->mnYStart;
    mnXStart    = pParam->mnXStart;
    mnXAdd      = pParam->mnXAdd;
    mnYAdd      = pParam->mnYAdd;

    // in Interlace mode the size of scanline is not constant
    // so first we calculate the number of entrys
    long nScanWidth = (maOrigSize.Width() - mnXStart + mnXAdd - 1) / mnXAdd;
    mnScansize = nScanWidth;

    if( mbRGBTriple )
        mnScansize = 3 * nScanWidth;

    if( mbAlphaChannel )
        mnScansize += nScanWidth;

    // convert to width in bytes
    mnScansize = ( mnScansize*mnPngDepth + 7 ) >> 3;

    ++mnScansize; // scan size also needs room for the filtertype byte
    memset( mpScanPrior, 0, mnScansize );

    return true;
}

// ImplApplyFilter writes the complete Scanline (nY)
// in interlace mode the parameter nXStart and nXAdd are non-zero

void PNGReaderImpl::ImplApplyFilter()
{
    OSL_ASSERT( mnScansize >= mnBPP + 1 );
    const sal_uInt8* const pScanEnd = mpInflateInBuf + mnScansize;

    sal_uInt8 nFilterType = *mpInflateInBuf; // the filter type may change each scanline
    switch ( nFilterType )
    {
        default: // unknown Scanline Filter Type
        case 0: // Filter Type "None"
            // we let the pixels pass and display the data unfiltered
            break;

        case 1: // Scanline Filter Type "Sub"
        {
            sal_uInt8* p1 = mpInflateInBuf + 1;
            const sal_uInt8* p2 = p1;
            p1 += mnBPP;

            // use left pixels
            while (p1 < pScanEnd)
            {
                *p1 = static_cast<sal_uInt8>( *p1 + *(p2++) );
                ++p1;
            }
        }
        break;

        case 2: // Scanline Filter Type "Up"
        {
            sal_uInt8* p1 = mpInflateInBuf + 1;
            const sal_uInt8* p2 = mpScanPrior + 1;

            // use pixels from prior line
            while( p1 < pScanEnd )
            {
                *p1 = static_cast<sal_uInt8>( *p1 + *(p2++) );
                ++p1;
            }
        }
        break;

        case 3: // Scanline Filter Type "Average"
        {
            sal_uInt8* p1 = mpInflateInBuf + 1;
            const sal_uInt8* p2 = mpScanPrior + 1;
            const sal_uInt8* p3 = p1;

            // use one pixel from prior line
            for( int n = mnBPP; --n >= 0; ++p1, ++p2)
                *p1 = static_cast<sal_uInt8>( *p1 + (*p2 >> 1) );

            // predict by averaging the left and prior line pixels
            while( p1 < pScanEnd )
            {
                *p1 = static_cast<sal_uInt8>( *p1 + ((*(p2++) + *(p3++)) >> 1) );
                ++p1;
            }
        }
        break;

        case 4: // Scanline Filter Type "PaethPredictor"
        {
            sal_uInt8* p1 = mpInflateInBuf + 1;
            const sal_uInt8* p2 = mpScanPrior + 1;
            const sal_uInt8* p3 = p1;
            const sal_uInt8* p4 = p2;

            // use one pixel from prior line
            for( int n = mnBPP; --n >= 0; ++p1)
                *p1 = static_cast<sal_uInt8>( *p1 + *(p2++) );

            // predict by using the left and the prior line pixels
            while( p1 < pScanEnd )
            {
                int na = *(p2++);
                int nb = *(p3++);
                int nc = *(p4++);

                int npa = nb - (int)nc;
                int npb = na - (int)nc;
                int npc = npa + npb;

                if( npa < 0 )
                    npa =-npa;
                if( npb < 0 )
                    npb =-npb;
                if( npc < 0 )
                    npc =-npc;

                if( npa > npb )
                {
                    na = nb;
                    npa = npb;
                }
                if( npa > npc )
                    na = nc;

                *p1 = static_cast<sal_uInt8>( *p1 + na );
                ++p1;
            }
        }
        break;
    }

    memcpy( mpScanPrior, mpInflateInBuf, mnScansize );
}

namespace
{
    sal_uInt8 SanitizePaletteIndex(sal_uInt8 nIndex, sal_uInt16 nPaletteEntryCount)
    {
        if (nIndex >= nPaletteEntryCount)
        {
            auto nSanitizedIndex = nIndex % nPaletteEntryCount;
            SAL_WARN_IF(nIndex != nSanitizedIndex, "vcl", "invalid colormap index: "
                        << static_cast<unsigned int>(nIndex) << ", colormap len is: "
                        << nPaletteEntryCount);
            nIndex = nSanitizedIndex;
        }
        return nIndex;
    }

    void SanitizePaletteIndexes(sal_uInt8* pEntries, int nLen, BitmapWriteAccess* pAcc)
    {
        sal_uInt16 nPaletteEntryCount = pAcc->GetPaletteEntryCount();
        for (int nX = 0; nX < nLen; ++nX)
        {
            if (pEntries[nX] >= nPaletteEntryCount)
            {
                SAL_WARN("vcl.gdi", "invalid colormap index: "
                          << static_cast<unsigned int>(pEntries[nX]) << ", colormap len is: "
                          << nPaletteEntryCount);
                pEntries[nX] = pEntries[nX] % nPaletteEntryCount;
            }
        }
    }
}

// ImplDrawScanlines draws the complete Scanline (nY) into the target bitmap
// In interlace mode the parameter nXStart and nXAdd append to the currently used pass

void PNGReaderImpl::ImplDrawScanline( sal_uInt32 nXStart, sal_uInt32 nXAdd )
{
    // optimization for downscaling
    if( mnYpos & mnPreviewMask )
        return;
    if( nXStart & mnPreviewMask )
        return;

    // convert nY to pixel units in the target image
    // => TODO; also do this for nX here instead of in the ImplSet*Pixel() methods
    const sal_uInt32 nY = mnYpos >> mnPreviewShift;

    sal_uInt8* pTmp = mpInflateInBuf + 1;
    if ( mpAcc->HasPalette() ) // alphachannel is not allowed by pictures including palette entries
    {
        switch ( mpAcc->GetBitCount() )
        {
            case 1 :
            {
                if ( mbTransparent )
                {
                    for ( sal_Int32 nX = nXStart, nShift = 0; nX < maOrigSize.Width(); nX += nXAdd )
                    {
                        sal_uInt8 nCol;
                        nShift = (nShift - 1) & 7;
                        if ( nShift == 0 )
                            nCol = *(pTmp++);
                        else
                            nCol = static_cast<sal_uInt8>( *pTmp >> nShift );
                        nCol &= 1;

                        ImplSetAlphaPixel( nY, nX, nCol, mpTransTab[ nCol ] );
                    }
                }
                else
                {   // ScanlineFormat::N1BitMsbPal
                    for ( sal_Int32 nX = nXStart, nShift = 0; nX < maOrigSize.Width(); nX += nXAdd )
                    {
                        nShift = (nShift - 1) & 7;

                        sal_uInt8 nCol;
                        if ( nShift == 0 )
                            nCol = *(pTmp++);
                        else
                            nCol = static_cast<sal_uInt8>( *pTmp >> nShift );
                        nCol &= 1;

                        ImplSetPixel( nY, nX, nCol );
                    }
                }
            }
            break;

            case 4 :
            {
                if ( mbTransparent )
                {
                    if ( mnPngDepth == 4 )  // check if source has a two bit pixel format
                    {
                        for ( sal_Int32 nX = nXStart, nXIndex = 0; nX < maOrigSize.Width(); nX += nXAdd, ++nXIndex )
                        {
                            if( nXIndex & 1 )
                            {
                                ImplSetAlphaPixel( nY, nX, *pTmp & 0x0f, mpTransTab[ *pTmp & 0x0f ] );
                                pTmp++;
                            }
                            else
                            {
                                ImplSetAlphaPixel( nY, nX, ( *pTmp >> 4 ) & 0x0f, mpTransTab[ *pTmp >> 4 ] );
                            }
                        }
                    }
                    else // if ( mnPngDepth == 2 )
                    {
                        for ( sal_Int32 nX = nXStart, nXIndex = 0; nX < maOrigSize.Width(); nX += nXAdd, nXIndex++ )
                        {
                            sal_uInt8 nCol;
                            switch( nXIndex & 3 )
                            {
                                case 0 :
                                    nCol = *pTmp >> 6;
                                break;

                                case 1 :
                                    nCol = ( *pTmp >> 4 ) & 0x03 ;
                                break;

                                case 2 :
                                    nCol = ( *pTmp >> 2 ) & 0x03;
                                break;

                                case 3 :
                                    nCol = ( *pTmp++ ) & 0x03;
                                break;

                                default:    // get rid of nCol uninitialized warning
                                    nCol = 0;
                                    break;
                            }

                            ImplSetAlphaPixel( nY, nX, nCol, mpTransTab[ nCol ] );
                        }
                    }
                }
                else
                {
                    if ( mnPngDepth == 4 )  // maybe the source is a two bitmap graphic
                    {   // ScanlineFormat::N4BitLsnPal
                        for ( sal_Int32 nX = nXStart, nXIndex = 0; nX < maOrigSize.Width(); nX += nXAdd, nXIndex++ )
                        {
                            if( nXIndex & 1 )
                                ImplSetPixel( nY, nX, *pTmp++ & 0x0f );
                            else
                                ImplSetPixel( nY, nX, ( *pTmp >> 4 ) & 0x0f );
                        }
                    }
                    else // if ( mnPngDepth == 2 )
                    {
                        for ( sal_Int32 nX = nXStart, nXIndex = 0; nX < maOrigSize.Width(); nX += nXAdd, nXIndex++ )
                        {
                            switch( nXIndex & 3 )
                            {
                                case 0 :
                                    ImplSetPixel( nY, nX, *pTmp >> 6 );
                                break;

                                case 1 :
                                    ImplSetPixel( nY, nX, ( *pTmp >> 4 ) & 0x03 );
                                break;

                                case 2 :
                                    ImplSetPixel( nY, nX, ( *pTmp >> 2 ) & 0x03 );
                                break;

                                case 3 :
                                    ImplSetPixel( nY, nX, *pTmp++ & 0x03 );
                                break;
                            }
                        }
                    }
                }
            }
            break;

            case 8 :
            {
                if ( mbAlphaChannel )
                {
                    if ( mnPngDepth == 8 )  // maybe the source is a 16 bit grayscale
                    {
                        for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 2 )
                            ImplSetAlphaPixel( nY, nX, pTmp[ 0 ], pTmp[ 1 ] );
                    }
                    else
                    {
                        for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 4 )
                            ImplSetAlphaPixel( nY, nX, pTmp[ 0 ], pTmp[ 2 ] );
                    }
                }
                else if ( mbTransparent )
                {
                    if ( mnPngDepth == 8 )  // maybe the source is a 16 bit grayscale
                    {
                        for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp++ )
                            ImplSetAlphaPixel( nY, nX, *pTmp, mpTransTab[ *pTmp ] );
                    }
                    else
                    {
                        for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 2 )
                            ImplSetAlphaPixel( nY, nX, *pTmp, mpTransTab[ *pTmp ] );
                    }
                }
                else // neither alpha nor transparency
                {
                    if ( mnPngDepth == 8 )  // maybe the source is a 16 bit grayscale
                    {
                        if( nXAdd == 1 && mnPreviewShift == 0 )  // copy raw line data if possible
                        {
                            int nLineBytes = maOrigSize.Width();
                            if (mbPalette)
                                SanitizePaletteIndexes(pTmp, nLineBytes, mpAcc);
                            mpAcc->CopyScanline( nY, pTmp, ScanlineFormat::N8BitPal, nLineBytes );
                        }
                        else
                        {
                            for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd )
                                ImplSetPixel( nY, nX, *pTmp++ );
                        }
                    }
                    else
                    {
                        for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 2 )
                            ImplSetPixel( nY, nX, *pTmp );
                    }
                }
            }
            break;

            default :
                mbStatus = false;
            break;
        }
    }
    else // no palette => truecolor
    {
        // #i122985# Added fast-lane implementations using CopyScanline with direct supported mem formats
        static bool bCkeckDirectScanline(true);

        if( mbAlphaChannel )
        {
            // has RGB + alpha
            if ( mnPngDepth == 8 )  // maybe the source has 16 bit per sample
            {
                // ScanlineFormat::N32BitTcRgba
                // only use DirectScanline when we have no preview shifting stuff and accesses to content and alpha
                const bool bDoDirectScanline(
                    bCkeckDirectScanline && !nXStart && 1 == nXAdd && !mnPreviewShift && mpMaskAcc);
                const bool bCustomColorTable(mpColorTable != mpDefaultColorTable);

                if(bDoDirectScanline)
                {
                    // allocate scanlines on demand, reused for next line
                    if(!mpScanline)
                    {
#if OSL_DEBUG_LEVEL > 0
                        mnAllocSizeScanline = maOrigSize.Width() * 3;
#endif
                        mpScanline = new sal_uInt8[maOrigSize.Width() * 3];
                    }

                    if(!mpScanlineAlpha)
                    {
#if OSL_DEBUG_LEVEL > 0
                        mnAllocSizeScanlineAlpha = maOrigSize.Width();
#endif
                        mpScanlineAlpha = new sal_uInt8[maOrigSize.Width()];
                    }
                }

                if(bDoDirectScanline)
                {
                    OSL_ENSURE(mpScanline, "No Scanline allocated (!)");
                    OSL_ENSURE(mpScanlineAlpha, "No ScanlineAlpha allocated (!)");
#if OSL_DEBUG_LEVEL > 0
                    OSL_ENSURE(mnAllocSizeScanline >= maOrigSize.Width() * 3, "Allocated Scanline too small (!)");
                    OSL_ENSURE(mnAllocSizeScanlineAlpha >= maOrigSize.Width(), "Allocated ScanlineAlpha too small (!)");
#endif
                    sal_uInt8* pScanline(mpScanline);
                    sal_uInt8* pScanlineAlpha(mpScanlineAlpha);

                    for (long nX(0); nX < maOrigSize.Width(); nX++, pTmp += 4)
                    {
                        // prepare content line as BGR by reordering when copying
                        // do not forget to invert alpha (source is alpha, target is opacity)
                        if(bCustomColorTable)
                        {
                            *pScanline++ = mpColorTable[pTmp[2]];
                            *pScanline++ = mpColorTable[pTmp[1]];
                            *pScanline++ = mpColorTable[pTmp[0]];
                            *pScanlineAlpha++ = ~pTmp[3];
                        }
                        else
                        {
                            *pScanline++ = pTmp[2];
                            *pScanline++ = pTmp[1];
                            *pScanline++ = pTmp[0];
                            *pScanlineAlpha++ = ~pTmp[3];
                        }
                    }

                    // copy scanlines directly to bitmaps for content and alpha; use the formats which
                    // are able to copy directly to BitmapBuffer
                    mpAcc->CopyScanline(nY, mpScanline, ScanlineFormat::N24BitTcBgr, maOrigSize.Width() * 3);
                    mpMaskAcc->CopyScanline(nY, mpScanlineAlpha, ScanlineFormat::N8BitPal, maOrigSize.Width());
                }
                else
                {
                    for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 4 )
                    {
                        if(bCustomColorTable)
                        {
                            ImplSetAlphaPixel(
                                nY,
                                nX,
                                BitmapColor(
                                    mpColorTable[ pTmp[ 0 ] ],
                                    mpColorTable[ pTmp[ 1 ] ],
                                    mpColorTable[ pTmp[ 2 ] ]),
                                pTmp[ 3 ]);
                        }
                        else
                        {
                            ImplSetAlphaPixel(
                                nY,
                                nX,
                                BitmapColor(
                                    pTmp[0],
                                    pTmp[1],
                                    pTmp[2]),
                                pTmp[3]);
                        }
                    }
                }
            }
            else
            {
                // BMP_FORMAT_64BIT_TC_RGBA
                for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 8 )
                {
                    ImplSetAlphaPixel(
                        nY,
                        nX,
                        BitmapColor(
                            mpColorTable[ pTmp[ 0 ] ],
                            mpColorTable[ pTmp[ 2 ] ],
                            mpColorTable[ pTmp[ 4 ] ]),
                        pTmp[6]);
                }
            }
        }
        else if( mbTransparent ) // has RGB + transparency
        {
            // ScanlineFormat::N24BitTcRgb
            // no support currently for DirectScanline, found no real usages in current PNGs, may be added on demand
            if ( mnPngDepth == 8 )  // maybe the source has 16 bit per sample
            {
                for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 3 )
                {
                    sal_uInt8 nRed = pTmp[ 0 ];
                    sal_uInt8 nGreen = pTmp[ 1 ];
                    sal_uInt8 nBlue = pTmp[ 2 ];
                    bool bTransparent = ( ( nRed == mnTransRed )
                                        && ( nGreen == mnTransGreen )
                                        && ( nBlue == mnTransBlue ) );

                    ImplSetTranspPixel( nY, nX, BitmapColor( mpColorTable[ nRed ],
                                                        mpColorTable[ nGreen ],
                                                        mpColorTable[ nBlue ] ), bTransparent );
                }
            }
            else
            {
                // BMP_FORMAT_48BIT_TC_RGB
                for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 6 )
                {
                    sal_uInt8 nRed = pTmp[ 0 ];
                    sal_uInt8 nGreen = pTmp[ 2 ];
                    sal_uInt8 nBlue = pTmp[ 4 ];
                    bool bTransparent = ( ( nRed == mnTransRed )
                                        && ( nGreen == mnTransGreen )
                                        && ( nBlue == mnTransBlue ) );

                    ImplSetTranspPixel( nY, nX, BitmapColor( mpColorTable[ nRed ],
                                                        mpColorTable[ nGreen ],
                                                        mpColorTable[ nBlue ] ), bTransparent );
                }
            }
        }
        else  // has RGB but neither alpha nor transparency
        {
            // ScanlineFormat::N24BitTcRgb
            // only use DirectScanline when we have no preview shifting stuff and access to content
            const bool bDoDirectScanline(
                bCkeckDirectScanline && !nXStart && 1 == nXAdd && !mnPreviewShift);
            const bool bCustomColorTable(mpColorTable != mpDefaultColorTable);

            if(bDoDirectScanline && !mpScanline)
            {
                // allocate scanlines on demand, reused for next line
#if OSL_DEBUG_LEVEL > 0
                mnAllocSizeScanline = maOrigSize.Width() * 3;
#endif
                mpScanline = new sal_uInt8[maOrigSize.Width() * 3];
            }

            if ( mnPngDepth == 8 )   // maybe the source has 16 bit per sample
            {
                if(bDoDirectScanline)
                {
                    OSL_ENSURE(mpScanline, "No Scanline allocated (!)");
#if OSL_DEBUG_LEVEL > 0
                    OSL_ENSURE(mnAllocSizeScanline >= maOrigSize.Width() * 3, "Allocated Scanline too small (!)");
#endif
                    sal_uInt8* pScanline(mpScanline);

                    for (long nX(0); nX < maOrigSize.Width(); nX++, pTmp += 3)
                    {
                        // prepare content line as BGR by reordering when copying
                        if(bCustomColorTable)
                        {
                            *pScanline++ = mpColorTable[pTmp[2]];
                            *pScanline++ = mpColorTable[pTmp[1]];
                            *pScanline++ = mpColorTable[pTmp[0]];
                        }
                        else
                        {
                            *pScanline++ = pTmp[2];
                            *pScanline++ = pTmp[1];
                            *pScanline++ = pTmp[0];
                        }
                    }

                    // copy scanline directly to bitmap for content; use the format which is able to
                    // copy directly to BitmapBuffer
                    mpAcc->CopyScanline(nY, mpScanline, ScanlineFormat::N24BitTcBgr, maOrigSize.Width() * 3);
                }
                else
                {
                    for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 3 )
                    {
                        if(bCustomColorTable)
                        {
                            ImplSetPixel(
                                nY,
                                nX,
                                BitmapColor(
                                    mpColorTable[ pTmp[ 0 ] ],
                                    mpColorTable[ pTmp[ 1 ] ],
                                    mpColorTable[ pTmp[ 2 ] ]));
                        }
                        else
                        {
                            ImplSetPixel(
                                nY,
                                nX,
                                BitmapColor(
                                    pTmp[0],
                                    pTmp[1],
                                    pTmp[2]));
                        }
                    }
                }
            }
            else
            {
                // BMP_FORMAT_48BIT_TC_RGB
                // no support currently for DirectScanline, found no real usages in current PNGs, may be added on demand
                for ( long nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 6 )
                {
                    ImplSetPixel(
                        nY,
                        nX,
                        BitmapColor(
                            mpColorTable[ pTmp[ 0 ] ],
                            mpColorTable[ pTmp[ 2 ] ],
                            mpColorTable[ pTmp[ 4 ] ]));
                }
            }
        }
    }
}

void PNGReaderImpl::ImplSetPixel( sal_uInt32 nY, sal_uInt32 nX, const BitmapColor& rBitmapColor )
{
    // TODO: get preview mode checks out of inner loop
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixel( nY, nX, rBitmapColor );
}

void PNGReaderImpl::ImplSetPixel( sal_uInt32 nY, sal_uInt32 nX, sal_uInt8 nPalIndex )
{
    // TODO: get preview mode checks out of inner loop
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixelIndex(nY, nX, SanitizePaletteIndex(nPalIndex, mpAcc->GetPaletteEntryCount()));
}

void PNGReaderImpl::ImplSetTranspPixel( sal_uInt32 nY, sal_uInt32 nX, const BitmapColor& rBitmapColor, bool bTrans )
{
    // TODO: get preview mode checks out of inner loop
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixel( nY, nX, rBitmapColor );

    if ( bTrans )
        mpMaskAcc->SetPixel( nY, nX, mcTranspColor );
    else
        mpMaskAcc->SetPixel( nY, nX, mcOpaqueColor );
}

void PNGReaderImpl::ImplSetAlphaPixel( sal_uInt32 nY, sal_uInt32 nX,
    sal_uInt8 nPalIndex, sal_uInt8 nAlpha )
{
    // TODO: get preview mode checks out of inner loop
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixelIndex(nY, nX, SanitizePaletteIndex(nPalIndex, mpAcc->GetPaletteEntryCount()));
    mpMaskAcc->SetPixel(nY, nX, BitmapColor(~nAlpha));
}

void PNGReaderImpl::ImplSetAlphaPixel( sal_uInt32 nY, sal_uInt32 nX,
    const BitmapColor& rBitmapColor, sal_uInt8 nAlpha )
{
    // TODO: get preview mode checks out of inner loop
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixel( nY, nX, rBitmapColor );
    if (!mpMaskAcc)
        return;
    mpMaskAcc->SetPixel(nY, nX, BitmapColor(~nAlpha));
}

sal_uInt32 PNGReaderImpl::ImplReadsal_uInt32()
{
    sal_uInt32 nRet;
    nRet = *maDataIter++;
    nRet <<= 8;
    nRet |= *maDataIter++;
    nRet <<= 8;
    nRet |= *maDataIter++;
    nRet <<= 8;
    nRet |= *maDataIter++;
    return nRet;
}

PNGReader::PNGReader(SvStream& rIStream) :
    mpImpl(new vcl::PNGReaderImpl(rIStream))
{
}

PNGReader::~PNGReader()
{
}

BitmapEx PNGReader::Read( const Size& i_rPreviewSizeHint )
{
    return mpImpl->GetBitmapEx( i_rPreviewSizeHint );
}

const std::vector< vcl::PNGReader::ChunkData >& PNGReader::GetChunks() const
{
    return mpImpl->GetAllChunks();
}

void PNGReader::SetIgnoreGammaChunk(bool bIgnoreGammaChunk)
{
    mpImpl->SetIgnoreGammaChunk(bIgnoreGammaChunk);
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
