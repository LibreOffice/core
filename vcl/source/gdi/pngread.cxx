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


#include <vcl/pngread.hxx>

#include <cmath>
#include <rtl/crc.h>
#include <rtl/memory.h>
#include <rtl/alloc.h>
#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/svapp.hxx>
#include <vcl/alpha.hxx>
#include <osl/endian.h>

// -----------
// - Defines -
// -----------

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

namespace vcl
{
// -----------
// - statics -
// -----------

// ------------------------------------------------------------------------------

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

// -------------
// - PNGReaderImpl -
// -------------

class PNGReaderImpl
{
private:
    SvStream&           mrPNGStream;
    sal_uInt16          mnOrigStreamMode;

    std::vector< vcl::PNGReader::ChunkData >    maChunkSeq;
    std::vector< vcl::PNGReader::ChunkData >::iterator maChunkIter;
    std::vector< sal_uInt8 >::iterator          maDataIter;

    Bitmap*             mpBmp;
    BitmapWriteAccess*  mpAcc;
    Bitmap*             mpMaskBmp;
    AlphaMask*          mpAlphaMask;
    BitmapWriteAccess*  mpMaskAcc;
    ZCodec*             mpZCodec;
    sal_uInt8*              mpInflateInBuf; // as big as the size of a scanline + alphachannel + 1
    sal_uInt8*              mpScanPrior;    // pointer to the latest scanline
    sal_uInt8*              mpTransTab;     // for transparency in images with palette colortype
    sal_uInt8*              mpScanCurrent;  // pointer into the current scanline
    sal_uInt8*              mpColorTable;   //
    sal_Size            mnStreamSize;   // estimate of PNG file size
    sal_uInt32          mnChunkType;    // Type of current PNG chunk
    sal_Int32           mnChunkLen;     // Length of current PNG chunk
    Size                maOrigSize;     // pixel size of the full image
    Size                maTargetSize;   // pixel size of the result image
    Size                maPhysSize;     // prefered size in MAP_100TH_MM units
    sal_uInt32          mnBPP;          // number of bytes per pixel
    sal_uInt32          mnScansize;     // max size of scanline
    sal_uInt32          mnYpos;         // latest y position in full image
    int                 mnPass;         // if interlaced the latest pass ( 1..7 ) else 7
    sal_uInt32          mnXStart;       // the starting X for the current pass
    sal_uInt32          mnXAdd;         // the increment for input images X coords for the current pass
    sal_uInt32          mnYAdd;         // the increment for input images Y coords for the current pass
    int                 mnPreviewShift; // shift to convert orig image coords into preview image coords
    int                 mnPreviewMask;  // == ((1 << mnPreviewShift) - 1)
    sal_uInt16              mnTargetDepth;      // pixel depth of target bitmap
    sal_uInt8               mnTransRed;
    sal_uInt8               mnTransGreen;
    sal_uInt8               mnTransBlue;
    sal_uInt8               mnPngDepth;     // pixel depth of PNG data
    sal_uInt8               mnColorType;
    sal_uInt8               mnCompressionType;
    sal_uInt8               mnFilterType;
    sal_uInt8               mnInterlaceType;
    BitmapColor         mcTranspColor;  // transparency mask's transparency "color"
    BitmapColor         mcOpaqueColor;  // transparency mask's opaque "color"
    sal_Bool                mbTransparent;  // graphic includes an tRNS Chunk or an alpha Channel
    sal_Bool                mbAlphaChannel; // is true for ColorType 4 and 6
    sal_Bool                mbRGBTriple;
    sal_Bool                mbPalette;      // sal_False if we need a Palette
    sal_Bool                mbGrayScale;
    sal_Bool                mbzCodecInUse;
    sal_Bool                mbStatus;
    sal_Bool                mbIDAT;         // sal_True if finished with enough IDAT chunks
    sal_Bool                mbGamma;        // sal_True if Gamma Correction available
    sal_Bool                mbpHYs;         // sal_True if pysical size of pixel available
    sal_Bool            mbIgnoreGammaChunk;

    bool                ReadNextChunk();
    void                ReadRemainingChunks();

    void                ImplSetPixel( sal_uInt32 y, sal_uInt32 x, const BitmapColor & );
    void                ImplSetPixel( sal_uInt32 y, sal_uInt32 x, sal_uInt8 nPalIndex );
    void                ImplSetTranspPixel( sal_uInt32 y, sal_uInt32 x, const BitmapColor &, sal_Bool bTrans );
    void                ImplSetAlphaPixel( sal_uInt32 y, sal_uInt32 x, sal_uInt8 nPalIndex, sal_uInt8 nAlpha );
    void                ImplSetAlphaPixel( sal_uInt32 y, sal_uInt32 x, const BitmapColor&, sal_uInt8 nAlpha );
    void                ImplReadIDAT();
    bool                ImplPreparePass();
    void                ImplApplyFilter();
    void                ImplDrawScanline( sal_uInt32 nXStart, sal_uInt32 nXAdd );
    sal_Bool                ImplReadTransparent();
    void                ImplGetGamma();
    void                ImplGetBackground();
    sal_uInt8               ImplScaleColor();
    sal_Bool                ImplReadHeader( const Size& rPreviewSizeHint );
    sal_Bool                ImplReadPalette();
    void                ImplGetGrayPalette( sal_uInt16 );
    sal_uInt32          ImplReadsal_uInt32();

public:

                        PNGReaderImpl( SvStream& );
                        ~PNGReaderImpl();

    BitmapEx            GetBitmapEx( const Size& rPreviewSizeHint );
    const std::vector< PNGReader::ChunkData >& GetAllChunks();
    void                SetIgnoreGammaChunk( sal_Bool bIgnore ){ mbIgnoreGammaChunk = bIgnore; };
};

// ------------------------------------------------------------------------------

PNGReaderImpl::PNGReaderImpl( SvStream& rPNGStream )
:   mrPNGStream( rPNGStream ),
    mpBmp           ( NULL ),
    mpAcc           ( NULL ),
    mpMaskBmp       ( NULL ),
    mpAlphaMask     ( NULL ),
    mpMaskAcc       ( NULL ),
    mpZCodec        ( new ZCodec( DEFAULT_IN_BUFSIZE, DEFAULT_OUT_BUFSIZE, MAX_MEM_USAGE ) ),
    mpInflateInBuf  ( NULL ),
    mpScanPrior     ( NULL ),
    mpTransTab      ( NULL ),
    mpScanCurrent   ( NULL ),
    mpColorTable    ( (sal_uInt8*) mpDefaultColorTable ),
    mnPass ( 0 ),
    mbPalette( sal_False ),
    mbzCodecInUse   ( sal_False ),
    mbStatus( sal_True),
    mbIDAT( sal_False ),
    mbGamma             ( sal_False ),
    mbpHYs              ( sal_False ),
    mbIgnoreGammaChunk  ( sal_False )
{
    // prepare the PNG data stream
    mnOrigStreamMode = mrPNGStream.GetNumberFormatInt();
    mrPNGStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

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
    mrPNGStream >> nDummy;
    mbStatus = (nDummy == 0x89504e47);
    mrPNGStream >> nDummy;
    mbStatus &= (nDummy == 0x0d0a1a0a);

    mnPreviewShift = 0;
    mnPreviewMask = (1 << mnPreviewShift) - 1;
}

// ------------------------------------------------------------------------

PNGReaderImpl::~PNGReaderImpl()
{
    mrPNGStream.SetNumberFormatInt( mnOrigStreamMode );

    if ( mbzCodecInUse )
        mpZCodec->EndCompression();

    if( mpColorTable != mpDefaultColorTable )
        delete[] mpColorTable;

    delete mpBmp;
    delete mpAlphaMask;
    delete mpMaskBmp;
    delete[] mpTransTab;
    delete[] mpInflateInBuf;
    delete[] mpScanPrior;
    delete mpZCodec;
}

// ------------------------------------------------------------------------

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
        mrPNGStream >> mnChunkLen >> mnChunkType;
        rChunkData.nType = mnChunkType;

        // #128377#/#149343# sanity check for chunk length
        if( mnChunkLen < 0 )
            return false;
        const sal_Size nStreamPos = mrPNGStream.Tell();
        if( nStreamPos + mnChunkLen >= mnStreamSize )
            return false;

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
        mrPNGStream >> nCheck;
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

// ------------------------------------------------------------------------

// read the remaining chunks from mrPNGStream
void PNGReaderImpl::ReadRemainingChunks()
{
    while( ReadNextChunk() ) ;
}

// ------------------------------------------------------------------------

const std::vector< vcl::PNGReader::ChunkData >& PNGReaderImpl::GetAllChunks()
{
    ReadRemainingChunks();
    return maChunkSeq;
}

// ------------------------------------------------------------------------

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
    while( mbStatus && !mbIDAT && ReadNextChunk() )
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
                if ( !mbIgnoreGammaChunk && ( mbIDAT == sal_False ) )
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
                if ( ( mbIDAT == sal_False ) && mbPalette )         // before the 'IDAT' and after the
                    ImplGetBackground();                        // PLTE(if available ) chunk.
            }
            break;

            case PNGCHUNK_IDAT :
            {
                if ( !mpInflateInBuf )  // taking care that the header has properly been read
                    mbStatus = sal_False;
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
                        mbpHYs = sal_True;

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
        mpBmp->ReleaseAccess( mpAcc ), mpAcc = NULL;

    if ( mpMaskAcc )
    {
        if ( mpAlphaMask )
            mpAlphaMask->ReleaseAccess( mpMaskAcc );
        else if ( mpMaskBmp )
            mpMaskBmp->ReleaseAccess( mpMaskAcc );

        mpMaskAcc = NULL;
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

// ------------------------------------------------------------------------

sal_Bool PNGReaderImpl::ImplReadHeader( const Size& rPreviewSizeHint )
{
    if( mnChunkLen < 13 )
        return sal_False;

    maOrigSize.Width()  = ImplReadsal_uInt32();
    maOrigSize.Height() = ImplReadsal_uInt32();

    if (maOrigSize.Width() <= 0 || maOrigSize.Height() <= 0)
        return sal_False;

    mnPngDepth = *(maDataIter++);
    mnColorType = *(maDataIter++);

    mnCompressionType = *(maDataIter++);
    if( mnCompressionType != 0 )    // unknown compression type
        return sal_False;

    mnFilterType = *(maDataIter++);
    if( mnFilterType != 0 )         // unknown filter type
        return sal_False;

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
            return sal_False;
    }

    mbPalette = sal_True;
    mbIDAT = mbAlphaChannel = mbTransparent = sal_False;
    mbGrayScale = mbRGBTriple = sal_False;
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
                    mbGrayScale = sal_True;
                    break;
                case 16 :
                    mnTargetDepth = 8;  // we have to reduce the bitmap
                    // fall through
                case 1 :
                case 4 :
                case 8 :
                    mbGrayScale = sal_True;
                    break;
                default :
                    return sal_False;
            }
        }
        break;

        case 2 :    // each pixel is an RGB triple
        {
            mbRGBTriple = sal_True;
            nScansize64 *= 3;
            switch ( mnPngDepth )
            {
                case 16 :           // we have to reduce the bitmap
                case 8 :
                    mnTargetDepth = 24;
                    break;
                default :
                    return sal_False;
            }
        }
        break;

        case 3 :    // each pixel is a palette index
        {
            switch ( mnPngDepth )
            {
                case 2 :
                    mnTargetDepth = 4;  // we have to expand the bitmap
                    // fall through
                case 1 :
                case 4 :
                case 8 :
                    mbPalette = sal_False;
                    break;
                default :
                    return sal_False;
            }
        }
        break;

        case 4 :    // each pixel is a grayscale sample followed by an alpha sample
        {
            nScansize64 *= 2;
            mbAlphaChannel = sal_True;
            switch ( mnPngDepth )
            {
                case 16 :
                    mnTargetDepth = 8;  // we have to reduce the bitmap
                case 8 :
                    mbGrayScale = sal_True;
                    break;
                default :
                    return sal_False;
            }
        }
        break;

        case 6 :    // each pixel is an RGB triple followed by an alpha sample
        {
            mbRGBTriple = sal_True;
            nScansize64 *= 4;
            mbAlphaChannel = sal_True;
            switch (mnPngDepth )
            {
                case 16 :           // we have to reduce the bitmap
                case 8 :
                    mnTargetDepth = 24;
                    break;
                default :
                    return sal_False;
            }
        }
        break;

        default :
            return sal_False;
    }

    mnBPP = static_cast< sal_uInt32 >( nScansize64 / maOrigSize.Width() );
    if ( !mnBPP )
        mnBPP = 1;

    nScansize64++;       // each scanline includes one filterbyte

    if ( nScansize64 > SAL_MAX_UINT32 )
        return sal_False;

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
            OSL_TRACE("preview size %dx%d", aPreviewSize.Width(), aPreviewSize.Height() );

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
        SAL_WARN( "vcl", "overlarge png dimensions: " <<
            maTargetSize.Width() << " x " << maTargetSize.Height() << " depth: " << mnTargetDepth);
        return sal_False;
    }

    // TODO: switch between both scanlines instead of copying
    mpInflateInBuf = new (std::nothrow) sal_uInt8[ mnScansize ];
    mpScanCurrent = mpInflateInBuf;
    mpScanPrior = new (std::nothrow) sal_uInt8[ mnScansize ];

    if ( !mpInflateInBuf || !mpScanPrior )
        return sal_False;

    mpBmp = new Bitmap( maTargetSize, mnTargetDepth );
    mpAcc = mpBmp->AcquireWriteAccess();
    if( !mpAcc )
        return sal_False;

    mpBmp->SetSourceSizePixel( maOrigSize );

    if ( mbAlphaChannel )
    {
        mpAlphaMask = new AlphaMask( maTargetSize );
        mpAlphaMask->Erase( 128 );
        mpMaskAcc = mpAlphaMask->AcquireWriteAccess();
        if( !mpMaskAcc )
            return sal_False;
    }

    if ( mbGrayScale )
        ImplGetGrayPalette( mnPngDepth );

    ImplPreparePass();

    return sal_True;
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

sal_Bool PNGReaderImpl::ImplReadPalette()
{
    sal_uInt16 nCount = static_cast<sal_uInt16>( mnChunkLen / 3 );

    if ( ( ( mnChunkLen % 3 ) == 0 ) && ( ( 0 < nCount ) && ( nCount <= 256 ) ) && mpAcc )
    {
        mbPalette = sal_True;
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
        mbStatus = sal_False;

    return mbStatus;
}

// ------------------------------------------------------------------------

sal_Bool PNGReaderImpl::ImplReadTransparent()
{
    bool bNeedAlpha = false;

    if ( mpTransTab == NULL )
    {
        switch ( mnColorType )
        {
            case 0 :
            {
                if ( mnChunkLen == 2 )
                {
                    mpTransTab = new sal_uInt8[ 256 ];
                    rtl_fillMemory( mpTransTab, 256, 0xff );
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
                    rtl_fillMemory( mpTransTab, 256, 0xff );
                    if (mnChunkLen > 0)
                    {
                        rtl_copyMemory( mpTransTab, &(*maDataIter), mnChunkLen );
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
        mbTransparent = (mpMaskAcc != NULL);
        if( !mbTransparent )
            return sal_False;
        mcOpaqueColor = BitmapColor( 0x00 );
        mcTranspColor = BitmapColor( 0xFF );
        mpMaskAcc->Erase( 0x00 );
    }

    return sal_True;
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplGetGamma()
{
    if( mnChunkLen < 4 )
        return;

    sal_uInt32  nGammaValue = ImplReadsal_uInt32();
    double      fGamma = ( ( VIEWING_GAMMA / DISPLAY_GAMMA ) * ( (double)nGammaValue / 100000 ) );
    double      fInvGamma = ( fGamma <= 0.0 || fGamma > 10.0 ) ? 1.0 : ( 1.0 / fGamma );

    if ( fInvGamma != 1.0 )
    {
        mbGamma = sal_True;

        if ( mpColorTable == mpDefaultColorTable )
            mpColorTable = new sal_uInt8[ 256 ];

        for ( sal_Int32 i = 0; i < 256; i++ )
            mpColorTable[ i ] = (sal_uInt8)(pow((double)i/255.0, fInvGamma) * 255.0 + 0.5);

        if ( mbGrayScale )
            ImplGetGrayPalette( mnPngDepth );
    }
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------
// ImplReadIDAT reads as much image data as needed

void PNGReaderImpl::ImplReadIDAT()
{
    if( mnChunkLen > 0 )
    {
        if ( mbzCodecInUse == sal_False )
        {
            mbzCodecInUse = sal_True;
            mpZCodec->BeginCompression( ZCODEC_PNG_DEFAULT );
        }
        mpZCodec->SetBreak( mnChunkLen );
        SvMemoryStream aIStrm( &(*maDataIter), mnChunkLen, STREAM_READ );

        while ( ( mpZCodec->GetBreak() ) )
        {
            // get bytes needed to fill the current scanline
            sal_Int32 nToRead = mnScansize - (mpScanCurrent - mpInflateInBuf);
            sal_Int32 nRead = mpZCodec->ReadAsynchron( aIStrm, mpScanCurrent, nToRead );
            if ( nRead < 0 )
            {
                mbStatus = sal_False;
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
        mpZCodec->EndCompression();
        mbzCodecInUse = sal_False;
    }
}

// ---------------------------------------------------------------------------------------------------

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
    rtl_zeroMemory( mpScanPrior, mnScansize );

    return true;
}

// ----------------------------------------------------------------------------
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
            do
                *p1 = static_cast<sal_uInt8>( *p1 + *(p2++) );
            while( ++p1 < pScanEnd );
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
                    na = nb, npa = npb;
                if( npa > npc )
                    na = nc;

                *p1 = static_cast<sal_uInt8>( *p1 + na );
                ++p1;
            }
        }
        break;
    }

    rtl_copyMemory( mpScanPrior, mpInflateInBuf, mnScansize );
}

// ---------------------------------------------------------------------------------------------------
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

    const sal_uInt8* pTmp = mpInflateInBuf + 1;
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
                {   // BMP_FORMAT_1BIT_MSB_PAL
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
                    {   // BMP_FORMAT_4BIT_LSN_PAL
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
                        for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 2 )
                            ImplSetAlphaPixel( nY, nX, pTmp[ 0 ], pTmp[ 1 ] );
                    }
                    else
                    {
                        for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 4 )
                            ImplSetAlphaPixel( nY, nX, pTmp[ 0 ], pTmp[ 2 ] );
                    }
                }
                else if ( mbTransparent )
                {
                    if ( mnPngDepth == 8 )  // maybe the source is a 16 bit grayscale
                    {
                        for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp++ )
                            ImplSetAlphaPixel( nY, nX, *pTmp, mpTransTab[ *pTmp ] );
                    }
                    else
                    {
                        for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 2 )
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
                            mpAcc->CopyScanline( nY, pTmp, BMP_FORMAT_8BIT_PAL, nLineBytes );
                            pTmp += nLineBytes;
                        }
                        else
                        {
                            for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd )
                                ImplSetPixel( nY, nX, *pTmp++ );
                        }
                    }
                    else
                    {
                        for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 2 )
                            ImplSetPixel( nY, nX, *pTmp );
                    }
                }
            }
            break;

            default :
                mbStatus = sal_False;
            break;
        }
    }
    else // no palette => truecolor
    {
        if( mbAlphaChannel ) // has RGB + alpha
        {   // BMP_FORMAT_32BIT_TC_RGBA
            if ( mnPngDepth == 8 )  // maybe the source has 16 bit per sample
            {
                if ( mpColorTable != mpDefaultColorTable )
                {
                    for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 4 )
                       ImplSetAlphaPixel( nY, nX, BitmapColor( mpColorTable[ pTmp[ 0 ] ],
                                                               mpColorTable[ pTmp[ 1 ] ],
                                                               mpColorTable[ pTmp[ 2 ] ] ), pTmp[ 3 ] );
                }
                else
                {
//                  if ( nXAdd == 1 && mnPreviewShift == 0 ) // copy raw line data if possible
//                  {
//                      int nLineBytes = 4 * maOrigSize.Width();
//                      mpAcc->CopyScanline( nY, pTmp, BMP_FORMAT_32BIT_TC_RGBA, nLineBytes );
//                      pTmp += nLineBytes;
//                  }
//                  else
                    {
                        for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 4 )
                            ImplSetAlphaPixel( nY, nX, BitmapColor( pTmp[0], pTmp[1], pTmp[2] ), pTmp[3] );
                    }
                }
            }
            else
            {   // BMP_FORMAT_64BIT_TC_RGBA
                for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 8 )
                    ImplSetAlphaPixel( nY, nX, BitmapColor( mpColorTable[ pTmp[ 0 ] ],
                                                        mpColorTable[ pTmp[ 2 ] ],
                                                        mpColorTable[ pTmp[ 4 ] ] ), pTmp[6] );
            }
        }
        else if( mbTransparent ) // has RGB + transparency
        {   // BMP_FORMAT_24BIT_TC_RGB
            if ( mnPngDepth == 8 )  // maybe the source has 16 bit per sample
            {
                for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 3 )
                {
                    sal_uInt8 nRed = pTmp[ 0 ];
                    sal_uInt8 nGreen = pTmp[ 1 ];
                    sal_uInt8 nBlue = pTmp[ 2 ];
                    sal_Bool bTransparent = ( ( nRed == mnTransRed )
                                         && ( nGreen == mnTransGreen )
                                        && ( nBlue == mnTransBlue ) );

                    ImplSetTranspPixel( nY, nX, BitmapColor( mpColorTable[ nRed ],
                                                        mpColorTable[ nGreen ],
                                                        mpColorTable[ nBlue ] ), bTransparent );
                }
            }
            else
            {   // BMP_FORMAT_48BIT_TC_RGB
                for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 6 )
                {
                    sal_uInt8 nRed = pTmp[ 0 ];
                    sal_uInt8 nGreen = pTmp[ 2 ];
                    sal_uInt8 nBlue = pTmp[ 4 ];
                    sal_Bool bTransparent = ( ( nRed == mnTransRed )
                                        && ( nGreen == mnTransGreen )
                                        && ( nBlue == mnTransBlue ) );

                    ImplSetTranspPixel( nY, nX, BitmapColor( mpColorTable[ nRed ],
                                                        mpColorTable[ nGreen ],
                                                        mpColorTable[ nBlue ] ), bTransparent );
                }
            }
        }
        else  // has RGB but neither alpha nor transparency
        {   // BMP_FORMAT_24BIT_TC_RGB
            if ( mnPngDepth == 8 )   // maybe the source has 16 bit per sample
            {
                if ( mpColorTable != mpDefaultColorTable )
                {
                    for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 3 )
                        ImplSetPixel( nY, nX, BitmapColor( mpColorTable[ pTmp[ 0 ] ],
                                                            mpColorTable[ pTmp[ 1 ] ],
                                                            mpColorTable[ pTmp[ 2 ] ] ) );
                }
                else
                {
                    if( nXAdd == 1 && mnPreviewShift == 0 ) // copy raw line data if possible
                    {
                        int nLineBytes = maOrigSize.Width() * 3;
                        mpAcc->CopyScanline( nY, pTmp, BMP_FORMAT_24BIT_TC_RGB, nLineBytes );
                        pTmp += nLineBytes;
                    }
                    else
                    {
                        for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 3 )
                            ImplSetPixel( nY, nX, BitmapColor( pTmp[0], pTmp[1], pTmp[2] ) );
                    }
                }
            }
            else
            {   // BMP_FORMAT_48BIT_TC_RGB
                for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 6 )
                    ImplSetPixel( nY, nX, BitmapColor( mpColorTable[ pTmp[ 0 ] ],
                                                        mpColorTable[ pTmp[ 2 ] ],
                                                        mpColorTable[ pTmp[ 4 ] ] ) );
            }
        }
    }
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplSetPixel( sal_uInt32 nY, sal_uInt32 nX, const BitmapColor& rBitmapColor )
{
    // TODO: get preview mode checks out of inner loop
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixel( nY, nX, rBitmapColor );
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplSetPixel( sal_uInt32 nY, sal_uInt32 nX, sal_uInt8 nPalIndex )
{
    // TODO: get preview mode checks out of inner loop
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixel( nY, nX, nPalIndex );
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplSetTranspPixel( sal_uInt32 nY, sal_uInt32 nX, const BitmapColor& rBitmapColor, sal_Bool bTrans )
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

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplSetAlphaPixel( sal_uInt32 nY, sal_uInt32 nX,
    sal_uInt8 nPalIndex, sal_uInt8 nAlpha )
{
    // TODO: get preview mode checks out of inner loop
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixel( nY, nX, nPalIndex );
    mpMaskAcc->SetPixel( nY, nX, ~nAlpha );
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplSetAlphaPixel( sal_uInt32 nY, sal_uInt32 nX,
    const BitmapColor& rBitmapColor, sal_uInt8 nAlpha )
{
    // TODO: get preview mode checks out of inner loop
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixel( nY, nX, rBitmapColor );
    mpMaskAcc->SetPixel( nY, nX, ~nAlpha );
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

// -------------
// - PNGReader -
// -------------

PNGReader::PNGReader( SvStream& rIStm ) :
    mpImpl( new ::vcl::PNGReaderImpl( rIStm ) )
{
}

// ------------------------------------------------------------------------

PNGReader::~PNGReader()
{
    delete mpImpl;
}

// ------------------------------------------------------------------------

BitmapEx PNGReader::Read( const Size& i_rPreviewSizeHint )
{
    return mpImpl->GetBitmapEx( i_rPreviewSizeHint );
}

// ------------------------------------------------------------------------

const std::vector< vcl::PNGReader::ChunkData >& PNGReader::GetChunks() const
{
    return mpImpl->GetAllChunks();
}

// ------------------------------------------------------------------------

void PNGReader::SetIgnoreGammaChunk( sal_Bool b )
{
    mpImpl->SetIgnoreGammaChunk( b );
}


} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
