/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <vcl/pngread.hxx>

#include <cmath>
#include <rtl/crc.h>
#include <rtl/alloc.h>
#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/svapp.hxx>
#include <vcl/alpha.hxx>
#include <osl/endian.h>

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
    sal_uInt8*              mpInflateInBuf; 
    sal_uInt8*              mpScanPrior;    
    sal_uInt8*              mpTransTab;     
    sal_uInt8*              mpScanCurrent;  
    sal_uInt8*              mpColorTable;   //
    sal_Size            mnStreamSize;   
    sal_uInt32          mnChunkType;    
    sal_Int32           mnChunkLen;     
    Size                maOrigSize;     
    Size                maTargetSize;   
    Size                maPhysSize;     
    sal_uInt32          mnBPP;          
    sal_uInt32          mnScansize;     
    sal_uInt32          mnYpos;         
    int                 mnPass;         
    sal_uInt32          mnXStart;       
    sal_uInt32          mnXAdd;         
    sal_uInt32          mnYAdd;         
    int                 mnPreviewShift; 
    int                 mnPreviewMask;  
    sal_uInt16              mnTargetDepth;      
    sal_uInt8               mnTransRed;
    sal_uInt8               mnTransGreen;
    sal_uInt8               mnTransBlue;
    sal_uInt8               mnPngDepth;     
    sal_uInt8               mnColorType;
    sal_uInt8               mnCompressionType;
    sal_uInt8               mnFilterType;
    sal_uInt8               mnInterlaceType;
    BitmapColor         mcTranspColor;  
    BitmapColor         mcOpaqueColor;  
    bool                mbTransparent;  
    bool                mbAlphaChannel; 
    bool                mbRGBTriple;
    bool                mbPalette;      
    bool                mbGrayScale;
    bool                mbzCodecInUse;
    bool                mbStatus;
    bool                mbIDAT;         
    bool                mbGamma;        
    bool                mbpHYs;         
    bool                mbIgnoreGammaChunk;

#if OSL_DEBUG_LEVEL > 0
    
    sal_Int32           mnAllocSizeScanline;
    sal_Int32           mnAllocSizeScanlineAlpha;
#endif
    
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
    sal_uInt8               ImplScaleColor();
    bool                ImplReadHeader( const Size& rPreviewSizeHint );
    bool                ImplReadPalette();
    void                ImplGetGrayPalette( sal_uInt16 );
    sal_uInt32          ImplReadsal_uInt32();

public:

                        PNGReaderImpl( SvStream& );
                        ~PNGReaderImpl();

    BitmapEx            GetBitmapEx( const Size& rPreviewSizeHint );
    const std::vector< PNGReader::ChunkData >& GetAllChunks();
    void                SetIgnoreGammaChunk( bool bIgnore ){ mbIgnoreGammaChunk = bIgnore; };
};

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
    mpScanline(0),
    mpScanlineAlpha(0)
{
    
    mnOrigStreamMode = mrPNGStream.GetNumberFormatInt();
    mrPNGStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    
    maChunkSeq.reserve( 16 );
    maChunkIter = maChunkSeq.begin();

    
    const sal_Size nStreamPos = mrPNGStream.Tell();
    mrPNGStream.Seek( STREAM_SEEK_TO_END );
    mnStreamSize = mrPNGStream.Tell();
    mrPNGStream.Seek( nStreamPos );

    
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

    delete[] mpScanline;
    delete[] mpScanlineAlpha;
}

bool PNGReaderImpl::ReadNextChunk()
{
    if( maChunkIter == maChunkSeq.end() )
    {
        

        
        if( mrPNGStream.IsEof() || (mrPNGStream.GetError() != ERRCODE_NONE) )
            return false;
        if( !maChunkSeq.empty() && (maChunkSeq.back().nType == PNGCHUNK_IEND) )
            return false;

        PNGReader::ChunkData aDummyChunk;
        maChunkIter = maChunkSeq.insert( maChunkSeq.end(), aDummyChunk );
        PNGReader::ChunkData& rChunkData = *maChunkIter;

        
        mrPNGStream.ReadInt32( mnChunkLen ).ReadUInt32( mnChunkType );
        rChunkData.nType = mnChunkType;

        
        const sal_Size nStreamPos = mrPNGStream.Tell();
        if( mnChunkLen < 0 || nStreamPos + mnChunkLen >= mnStreamSize )
            mnChunkLen = mnStreamSize - nStreamPos;

        
        sal_uInt32 nChunkType = mnChunkType;
        #if defined(__LITTLEENDIAN) || defined(OSL_LITENDIAN)
        nChunkType = OSL_SWAPDWORD( nChunkType );
        #endif
        sal_uInt32 nCRC32 = rtl_crc32( 0, &nChunkType, 4 );

        
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
        
        mnChunkType = (*maChunkIter).nType;
        mnChunkLen = (*maChunkIter).aData.size();
        maDataIter = (*maChunkIter).aData.begin();
    }

    ++maChunkIter;
    if( mnChunkType == PNGCHUNK_IEND )
        return false;
    return true;
}


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
    
    maChunkIter = maChunkSeq.begin();

    
    if( mbStatus && ReadNextChunk() )
    {
        if (mnChunkType == PNGCHUNK_IHDR)
            mbStatus = ImplReadHeader( rPreviewSizeHint );
        else
            mbStatus = false;
    }

    
    bool bRetFromNextChunk;
    while( mbStatus && !mbIDAT && (bRetFromNextChunk = ReadNextChunk()) )
    {
        switch( mnChunkType )
        {
            case PNGCHUNK_IHDR :
            {
                mbStatus = false; 
            }
            break;

            case PNGCHUNK_gAMA :                                
            {                                                   
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
                if ( !mbIDAT )                                  
                    mbStatus = ImplReadTransparent();
            }
            break;

            case PNGCHUNK_bKGD :                                
            {
                if ( !mbIDAT && mbPalette )         
                    ImplGetBackground();                        
            }
            break;

            case PNGCHUNK_IDAT :
            {
                if ( !mpInflateInBuf )  
                    mbStatus = false;
                else if ( !mbIDAT )     
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

                        
                        maPhysSize.Width()  = (sal_Int32)( (100000.0 * maOrigSize.Width()) / nXPixelPerMeter );
                        maPhysSize.Height() = (sal_Int32)( (100000.0 * maOrigSize.Height()) / nYPixelPerMeter );
                    }
                }
            }
            break;

            case PNGCHUNK_IEND:
                mbStatus = mbIDAT;  
            break;
        }
    }

    
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
    if( mnCompressionType != 0 )    
        return false;

    mnFilterType = *(maDataIter++);
    if( mnFilterType != 0 )         
        return false;

    mnInterlaceType = *(maDataIter++);
    switch ( mnInterlaceType ) 
    {
        case 0 :  
            mnPass = 7;
            break;
        case 1 :  
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

    
    switch ( mnColorType )
    {
        case 0 :    
        {
            switch ( mnPngDepth )
            {
                case 2 : 
                    mnTargetDepth = 4;  
                    mbGrayScale = true;
                    break;
                case 16 :
                    mnTargetDepth = 8;  
                    
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

        case 2 :    
        {
            mbRGBTriple = true;
            nScansize64 *= 3;
            switch ( mnPngDepth )
            {
                case 16 :           
                case 8 :
                    mnTargetDepth = 24;
                    break;
                default :
                    return false;
            }
        }
        break;

        case 3 :    
        {
            switch ( mnPngDepth )
            {
                case 2 :
                    mnTargetDepth = 4;  
                    
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

        case 4 :    
        {
            nScansize64 *= 2;
            mbAlphaChannel = true;
            switch ( mnPngDepth )
            {
                case 16 :
                    mnTargetDepth = 8;  
                case 8 :
                    mbGrayScale = true;
                    break;
                default :
                    return false;
            }
        }
        break;

        case 6 :    
        {
            mbRGBTriple = true;
            nScansize64 *= 4;
            mbAlphaChannel = true;
            switch (mnPngDepth )
            {
                case 16 :           
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

    nScansize64++;       

    if ( nScansize64 > SAL_MAX_UINT32 )
        return false;

    mnScansize = static_cast< sal_uInt32 >( nScansize64 );

    
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

    
    int nBytesPerPixel = ((mnTargetDepth + 7) & ~7)/8;

    
    if (maTargetSize.Width() >= SAL_MAX_INT32 / nBytesPerPixel / maTargetSize.Height())
    {
        SAL_WARN( "vcl.gdi", "overlarge png dimensions: " <<
            maTargetSize.Width() << " x " << maTargetSize.Height() << " depth: " << mnTargetDepth);
        return false;
    }

    
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

    if ( mpTransTab == NULL )
    {
        switch ( mnColorType )
        {
            case 0 :
            {
                if ( mnChunkLen == 2 )
                {
                    mpTransTab = new sal_uInt8[ 256 ];
                    memset( mpTransTab, 0xff, 256);
                    
                    
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



sal_uInt8 PNGReaderImpl::ImplScaleColor()
{
    sal_uInt32 nMask = ( ( 1 << mnPngDepth ) - 1 );
    sal_uInt16 nCol = ( *maDataIter++ << 8 );

    nCol += *maDataIter++ & (sal_uInt16)nMask;

    if ( mnPngDepth > 8 )   
        nCol >>= 8;

    return (sal_uInt8) nCol;
}



void PNGReaderImpl::ImplReadIDAT()
{
    if( mnChunkLen > 0 )
    {
        if ( !mbzCodecInUse )
        {
            mbzCodecInUse = true;
            mpZCodec->BeginCompression( ZCODEC_PNG_DEFAULT );
        }
        mpZCodec->SetBreak( mnChunkLen );
        SvMemoryStream aIStrm( &(*maDataIter), mnChunkLen, STREAM_READ );

        while ( ( mpZCodec->GetBreak() ) )
        {
            
            sal_Int32 nToRead = mnScansize - (mpScanCurrent - mpInflateInBuf);
            sal_Int32 nRead = mpZCodec->ReadAsynchron( aIStrm, mpScanCurrent, nToRead );
            if ( nRead < 0 )
            {
                mbStatus = false;
                break;
            }
            if ( nRead < nToRead )
            {
                mpScanCurrent += nRead; 
                break;
            }
            else  
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
        mbzCodecInUse = false;
    }
}

bool PNGReaderImpl::ImplPreparePass()
{
    struct InterlaceParams{ int mnXStart, mnYStart, mnXAdd, mnYAdd; };
    static const InterlaceParams aInterlaceParams[8] =
    {
        
        { 0, 0, 1, 1 },
        
        { 0, 0, 8, 8 },    
        { 4, 0, 8, 8 },    
        { 0, 4, 4, 8 },    
        { 2, 0, 4, 4 },    
        { 0, 2, 2, 4 },    
        { 1, 0, 2, 2 },    
        { 0, 1, 1, 2 }     
    };

    const InterlaceParams* pParam = &aInterlaceParams[ 0 ];
    if( mnInterlaceType )
    {
        while( ++mnPass <= 7 )
        {
            pParam = &aInterlaceParams[ mnPass ];

            
            if( (pParam->mnXStart < maOrigSize.Width())
            &&  (pParam->mnYStart < maOrigSize.Height()) )
                break;
        }
        if( mnPass > 7 )
            return false;

        
        if( mnPreviewMask & (pParam->mnXStart | pParam->mnYStart) )
            return false;
    }

    mnYpos      = pParam->mnYStart;
    mnXStart    = pParam->mnXStart;
    mnXAdd      = pParam->mnXAdd;
    mnYAdd      = pParam->mnYAdd;

    
    
    long nScanWidth = (maOrigSize.Width() - mnXStart + mnXAdd - 1) / mnXAdd;
    mnScansize = nScanWidth;

    if( mbRGBTriple )
        mnScansize = 3 * nScanWidth;

    if( mbAlphaChannel )
        mnScansize += nScanWidth;

    
    mnScansize = ( mnScansize*mnPngDepth + 7 ) >> 3;

    ++mnScansize; 
    memset( mpScanPrior, 0, mnScansize );

    return true;
}




void PNGReaderImpl::ImplApplyFilter()
{
    OSL_ASSERT( mnScansize >= mnBPP + 1 );
    const sal_uInt8* const pScanEnd = mpInflateInBuf + mnScansize;

    sal_uInt8 nFilterType = *mpInflateInBuf; 
    switch ( nFilterType )
    {
        default: 
        case 0: 
            
            break;

        case 1: 
        {
            sal_uInt8* p1 = mpInflateInBuf + 1;
            const sal_uInt8* p2 = p1;
            p1 += mnBPP;

            
            do
                *p1 = static_cast<sal_uInt8>( *p1 + *(p2++) );
            while( ++p1 < pScanEnd );
        }
        break;

        case 2: 
        {
            sal_uInt8* p1 = mpInflateInBuf + 1;
            const sal_uInt8* p2 = mpScanPrior + 1;

            
            while( p1 < pScanEnd )
            {
                *p1 = static_cast<sal_uInt8>( *p1 + *(p2++) );
                ++p1;
            }
        }
        break;

        case 3: 
        {
            sal_uInt8* p1 = mpInflateInBuf + 1;
            const sal_uInt8* p2 = mpScanPrior + 1;
            const sal_uInt8* p3 = p1;

            
            for( int n = mnBPP; --n >= 0; ++p1, ++p2)
                *p1 = static_cast<sal_uInt8>( *p1 + (*p2 >> 1) );

            
            while( p1 < pScanEnd )
            {
                *p1 = static_cast<sal_uInt8>( *p1 + ((*(p2++) + *(p3++)) >> 1) );
                ++p1;
            }
        }
        break;

        case 4: 
        {
            sal_uInt8* p1 = mpInflateInBuf + 1;
            const sal_uInt8* p2 = mpScanPrior + 1;
            const sal_uInt8* p3 = p1;
            const sal_uInt8* p4 = p2;

            
            for( int n = mnBPP; --n >= 0; ++p1)
                *p1 = static_cast<sal_uInt8>( *p1 + *(p2++) );

            
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

    memcpy( mpScanPrior, mpInflateInBuf, mnScansize );
}




void PNGReaderImpl::ImplDrawScanline( sal_uInt32 nXStart, sal_uInt32 nXAdd )
{
    
    if( mnYpos & mnPreviewMask )
        return;
    if( nXStart & mnPreviewMask )
        return;

    
    
    const sal_uInt32 nY = mnYpos >> mnPreviewShift;

    const sal_uInt8* pTmp = mpInflateInBuf + 1;
    if ( mpAcc->HasPalette() ) 
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
                {   
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
                    if ( mnPngDepth == 4 )  
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
                    else 
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

                                default:    
                                    nCol = 0;
                                    break;
                            }

                            ImplSetAlphaPixel( nY, nX, nCol, mpTransTab[ nCol ] );
                        }
                    }
                }
                else
                {
                    if ( mnPngDepth == 4 )  
                    {   
                        for ( sal_Int32 nX = nXStart, nXIndex = 0; nX < maOrigSize.Width(); nX += nXAdd, nXIndex++ )
                        {
                            if( nXIndex & 1 )
                                ImplSetPixel( nY, nX, *pTmp++ & 0x0f );
                            else
                                ImplSetPixel( nY, nX, ( *pTmp >> 4 ) & 0x0f );
                        }
                    }
                    else 
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
                    if ( mnPngDepth == 8 )  
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
                    if ( mnPngDepth == 8 )  
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
                else 
                {
                    if ( mnPngDepth == 8 )  
                    {
                        if( nXAdd == 1 && mnPreviewShift == 0 )  
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
                mbStatus = false;
            break;
        }
    }
    else 
    {
        
        static bool bCkeckDirectScanline(true);

        if( mbAlphaChannel )
        {
            
            if ( mnPngDepth == 8 )  
            {
                
                
                const bool bDoDirectScanline(
                    bCkeckDirectScanline && !nXStart && 1 == nXAdd && !mnPreviewShift && mpAcc && mpMaskAcc);
                const bool bCustomColorTable(mpColorTable != mpDefaultColorTable);

                if(bDoDirectScanline)
                {
                    
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
                    OSL_ENSURE(mnAllocSizeScanline >= maOrigSize.Width() * 3, "Allocated Scanline too small (!)");
                    OSL_ENSURE(mnAllocSizeScanlineAlpha >= maOrigSize.Width(), "Allocated ScanlineAlpha too small (!)");
                    sal_uInt8* pScanline(mpScanline);
                    sal_uInt8* pScanlineAlpha(mpScanlineAlpha);

                    for (sal_Int32 nX(0); nX < maOrigSize.Width(); nX++, pTmp += 4)
                    {
                        
                        
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

                    
                    
                    mpAcc->CopyScanline(nY, mpScanline, BMP_FORMAT_24BIT_TC_BGR, maOrigSize.Width() * 3);
                    mpMaskAcc->CopyScanline(nY, mpScanlineAlpha, BMP_FORMAT_8BIT_PAL, maOrigSize.Width());
                }
                else
                {
                    for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 4 )
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
                
                for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 8 )
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
        else if( mbTransparent ) 
        {
            
            
            if ( mnPngDepth == 8 )  
            {
                for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 3 )
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
                
                for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 6 )
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
        else  
        {
            
            
            const bool bDoDirectScanline(
                bCkeckDirectScanline && !nXStart && 1 == nXAdd && !mnPreviewShift);
            const bool bCustomColorTable(mpColorTable != mpDefaultColorTable);

            if(bDoDirectScanline && !mpScanline)
            {
                
#if OSL_DEBUG_LEVEL > 0
                mnAllocSizeScanline = maOrigSize.Width() * 3;
#endif
                mpScanline = new sal_uInt8[maOrigSize.Width() * 3];
            }

            if ( mnPngDepth == 8 )   
            {
                if(bDoDirectScanline)
                {
                    OSL_ENSURE(mpScanline, "No Scanline allocated (!)");
                    OSL_ENSURE(mnAllocSizeScanline >= maOrigSize.Width() * 3, "Allocated Scanline too small (!)");
                    sal_uInt8* pScanline(mpScanline);

                    for (sal_Int32 nX(0); nX < maOrigSize.Width(); nX++, pTmp += 3)
                    {
                        
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

                    
                    
                    mpAcc->CopyScanline(nY, mpScanline, BMP_FORMAT_24BIT_TC_BGR, maOrigSize.Width() * 3);
                }
                else
                {
                    for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 3 )
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
                
                
                for ( sal_Int32 nX = nXStart; nX < maOrigSize.Width(); nX += nXAdd, pTmp += 6 )
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
    
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixel( nY, nX, rBitmapColor );
}

void PNGReaderImpl::ImplSetPixel( sal_uInt32 nY, sal_uInt32 nX, sal_uInt8 nPalIndex )
{
    
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixelIndex( nY, nX, nPalIndex );
}

void PNGReaderImpl::ImplSetTranspPixel( sal_uInt32 nY, sal_uInt32 nX, const BitmapColor& rBitmapColor, bool bTrans )
{
    
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
    
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixelIndex( nY, nX, nPalIndex );
    mpMaskAcc->SetPixelIndex( nY, nX, ~nAlpha );
}

void PNGReaderImpl::ImplSetAlphaPixel( sal_uInt32 nY, sal_uInt32 nX,
    const BitmapColor& rBitmapColor, sal_uInt8 nAlpha )
{
    
    if( nX & mnPreviewMask )
        return;
    nX >>= mnPreviewShift;

    mpAcc->SetPixel( nY, nX, rBitmapColor );
    mpMaskAcc->SetPixelIndex( nY, nX, ~nAlpha );
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

PNGReader::PNGReader( SvStream& rIStm ) :
    mpImpl( new ::vcl::PNGReaderImpl( rIStm ) )
{
}

PNGReader::~PNGReader()
{
    delete mpImpl;
}

BitmapEx PNGReader::Read( const Size& i_rPreviewSizeHint )
{
    return mpImpl->GetBitmapEx( i_rPreviewSizeHint );
}

const std::vector< vcl::PNGReader::ChunkData >& PNGReader::GetChunks() const
{
    return mpImpl->GetAllChunks();
}

void PNGReader::SetIgnoreGammaChunk( bool b )
{
    mpImpl->SetIgnoreGammaChunk( b );
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
