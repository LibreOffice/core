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


#include <vcl/pngwrite.hxx>

#include <cmath>
#include <limits>
#include <rtl/crc.h>
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

#define PNG_DEF_COMPRESSION 6

#define PNGCHUNK_IHDR 0x49484452
#define PNGCHUNK_PLTE 0x504c5445
#define PNGCHUNK_IDAT 0x49444154
#define PNGCHUNK_IEND 0x49454e44
#define PNGCHUNK_bKGD 0x624b4744
#define PNGCHUNK_gAMA 0x67414d41
#define PNGCHUNK_pHYs 0x70485973
#define PNGCHUNK_tRNS 0x74524e53

namespace vcl
{
// -----------------
// - PNGWriterImplImpl -
// -----------------

class PNGWriterImpl
{
public:

                PNGWriterImpl( const BitmapEx& BmpEx,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData = NULL );
                ~PNGWriterImpl();

    sal_Bool    Write( SvStream& rOStm );

    std::vector< vcl::PNGWriter::ChunkData >&   GetChunks();

private:

    std::vector< vcl::PNGWriter::ChunkData >    maChunkSeq;

    sal_Int32           mnCompLevel;
    sal_Int32           mnInterlaced;
    sal_uInt32          mnMaxChunkSize;
    sal_Bool                mbStatus;

    BitmapReadAccess*   mpAccess;
    BitmapReadAccess*   mpMaskAccess;
    ZCodec*             mpZCodec;

    sal_uInt8*              mpDeflateInBuf;         // as big as the size of a scanline + alphachannel + 1
    sal_uInt8*              mpPreviousScan;         // as big as mpDeflateInBuf
    sal_uInt8*              mpCurrentScan;
    sal_uLong               mnDeflateInSize;

    sal_uLong               mnWidth, mnHeight;
    sal_uInt8               mnBitsPerPixel;
    sal_uInt8               mnFilterType;           // 0 oder 4;
    sal_uLong               mnBBP;                  // bytes per pixel ( needed for filtering )
    sal_Bool                mbTrueAlpha;
    sal_uLong               mnCRC;

    void                ImplWritepHYs( const BitmapEx& rBitmapEx );
    void                ImplWriteIDAT();
    sal_uLong               ImplGetFilter( sal_uLong nY, sal_uLong nXStart=0, sal_uLong nXAdd=1 );
    void                ImplClearFirstScanline();
    void                ImplWriteTransparent();
    sal_Bool                ImplWriteHeader();
    void                ImplWritePalette();
    void                ImplOpenChunk( sal_uLong nChunkType );
    void                ImplWriteChunk( sal_uInt8 nNumb );
    void                ImplWriteChunk( sal_uInt32 nNumb );
    void                ImplWriteChunk( unsigned char* pSource, sal_uInt32 nDatSize );
    void                ImplCloseChunk( void ) const;
};

// ------------------------------------------------------------------------

PNGWriterImpl::PNGWriterImpl( const BitmapEx& rBmpEx,
    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData ) :
        mnCompLevel     ( PNG_DEF_COMPRESSION ),
        mbStatus        ( sal_True ),
        mpAccess        ( NULL ),
        mpMaskAccess    ( NULL ),
        mpZCodec        ( new ZCodec( DEFAULT_IN_BUFSIZE, DEFAULT_OUT_BUFSIZE, MAX_MEM_USAGE ) ),
        mnCRC(0UL)
{
    if ( !rBmpEx.IsEmpty() )
    {
        Bitmap aBmp( rBmpEx.GetBitmap() );

        mnInterlaced = 0;   // ( aBmp.GetSizePixel().Width() > 128 ) || ( aBmp.GetSizePixel().Height() > 128 ) ? 1 : 0; #i67236#

        // #i67234# defaulting max chunk size to 256kb when using interlace mode
        mnMaxChunkSize = mnInterlaced == 0 ? std::numeric_limits< sal_uInt32 >::max() : 0x40000;

        if ( pFilterData )
        {
            sal_Int32 i = 0;
            for ( i = 0; i < pFilterData->getLength(); i++ )
            {
                if ( (*pFilterData)[ i ].Name == "Compression" )
                    (*pFilterData)[ i ].Value >>= mnCompLevel;
                else if ( (*pFilterData)[ i ].Name == "Interlaced" )
                    (*pFilterData)[ i ].Value >>= mnInterlaced;
                else if ( (*pFilterData)[ i ].Name == "MaxChunkSize" )
                {
                    sal_Int32 nVal = 0;
                    if ( (*pFilterData)[ i ].Value >>= nVal )
                        mnMaxChunkSize = (sal_uInt32)nVal;
                }
            }
        }
        mnBitsPerPixel = (sal_uInt8)aBmp.GetBitCount();

        if( rBmpEx.IsTransparent() )
        {
            if ( mnBitsPerPixel <= 8 && rBmpEx.IsAlpha() )
            {
                aBmp.Convert( BMP_CONVERSION_24BIT );
                mnBitsPerPixel = 24;
            }

            if ( mnBitsPerPixel <= 8 )                  // transparent palette
            {
                aBmp.Convert( BMP_CONVERSION_8BIT_TRANS );
                aBmp.Replace( rBmpEx.GetMask(), BMP_COL_TRANS );
                mnBitsPerPixel = 8;
                mpAccess = aBmp.AcquireReadAccess();
                if ( mpAccess )
                {
                    if ( ImplWriteHeader() )
                    {
                        ImplWritepHYs( rBmpEx );
                        ImplWritePalette();
                        ImplWriteTransparent();
                        ImplWriteIDAT();
                    }
                    aBmp.ReleaseAccess( mpAccess ), mpAccess = 0;
                }
                else
                    mbStatus = sal_False;
            }
            else
            {
                mpAccess = aBmp.AcquireReadAccess();    // sal_True RGB with alphachannel
                if( mpAccess )
                {
                    if ( ( mbTrueAlpha = rBmpEx.IsAlpha() ) != sal_False )
                    {
                        AlphaMask aMask( rBmpEx.GetAlpha() );
                        mpMaskAccess = aMask.AcquireReadAccess();
                        if ( mpMaskAccess )
                        {
                            if ( ImplWriteHeader() )
                            {
                                ImplWritepHYs( rBmpEx );
                                ImplWriteIDAT();
                            }
                            aMask.ReleaseAccess( mpMaskAccess ), mpMaskAccess = 0;
                        }
                        else
                            mbStatus = sal_False;
                    }
                    else
                    {
                        Bitmap aMask( rBmpEx.GetMask() );
                        mpMaskAccess = aMask.AcquireReadAccess();
                        if( mpMaskAccess )
                        {
                            if ( ImplWriteHeader() )
                            {
                                ImplWritepHYs( rBmpEx );
                                ImplWriteIDAT();
                            }
                            aMask.ReleaseAccess( mpMaskAccess ), mpMaskAccess = 0;
                        }
                        else
                            mbStatus = sal_False;
                    }
                    aBmp.ReleaseAccess( mpAccess ), mpAccess = 0;
                }
                else
                    mbStatus = sal_False;
            }
        }
        else
        {
            mpAccess = aBmp.AcquireReadAccess();        // palette + RGB without alphachannel
            if( mpAccess )
            {
                if ( ImplWriteHeader() )
                {
                    ImplWritepHYs( rBmpEx );
                    if( mpAccess->HasPalette() )
                        ImplWritePalette();

                    ImplWriteIDAT();
                }
                aBmp.ReleaseAccess( mpAccess ), mpAccess = 0;
            }
            else
                mbStatus = sal_False;
        }
        if ( mbStatus )
        {
            ImplOpenChunk( PNGCHUNK_IEND );     // create an IEND chunk
            ImplCloseChunk();
        }
    }
}

// ------------------------------------------------------------------------

PNGWriterImpl::~PNGWriterImpl()
{
    delete mpZCodec;
}

// ------------------------------------------------------------------------

sal_Bool PNGWriterImpl::Write( SvStream& rOStm )
{
   /* png signature is always an array of 8 bytes */
    sal_uInt16 nOldMode = rOStm.GetNumberFormatInt();
    rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rOStm << static_cast<sal_uInt32>(0x89504e47);
    rOStm << static_cast<sal_uInt32>(0x0d0a1a0a);

    std::vector< vcl::PNGWriter::ChunkData >::iterator aBeg( maChunkSeq.begin() );
    std::vector< vcl::PNGWriter::ChunkData >::iterator aEnd( maChunkSeq.end() );
    while( aBeg != aEnd )
    {
        sal_uInt32 nType = aBeg->nType;
    #if defined(__LITTLEENDIAN) || defined(OSL_LITENDIAN)
        nType = OSL_SWAPDWORD( nType );
    #endif
        sal_uInt32 nCRC = rtl_crc32( 0, &nType, 4 );
        sal_uInt32 nDataSize = aBeg->aData.size();
        if ( nDataSize )
            nCRC = rtl_crc32( nCRC, &aBeg->aData[ 0 ], nDataSize );
        rOStm << nDataSize
              << aBeg->nType;
        if ( nDataSize )
            rOStm.Write( &aBeg->aData[ 0 ], nDataSize );
        rOStm << nCRC;
        aBeg++;
    }
    rOStm.SetNumberFormatInt( nOldMode );
    return mbStatus;
}

// ------------------------------------------------------------------------

std::vector< vcl::PNGWriter::ChunkData >& PNGWriterImpl::GetChunks()
{
    return maChunkSeq;
}

// ------------------------------------------------------------------------

sal_Bool PNGWriterImpl::ImplWriteHeader()
{
    ImplOpenChunk(PNGCHUNK_IHDR);
    ImplWriteChunk( sal_uInt32( mnWidth =  mpAccess->Width() ) );
    ImplWriteChunk( sal_uInt32( mnHeight = mpAccess->Height() ) );

    if ( mnWidth && mnHeight && mnBitsPerPixel && mbStatus )
    {
        sal_uInt8 nBitDepth = mnBitsPerPixel;
        if ( mnBitsPerPixel <= 8 )
            mnFilterType = 0;
        else
            mnFilterType = 4;

        sal_uInt8 nColorType = 2;                   // colortype:
                                                // bit 0 -> palette is used
        if ( mpAccess->HasPalette() )           // bit 1 -> color is used
            nColorType |= 1;                    // bit 2 -> alpha channel is used
        else
            nBitDepth /= 3;

        if ( mpMaskAccess )
            nColorType |= 4;

        ImplWriteChunk( nBitDepth );
        ImplWriteChunk( nColorType );           // colortype
        ImplWriteChunk((sal_uInt8) 0 );             // compression type
        ImplWriteChunk((sal_uInt8) 0 );             // filter type - is not supported in this version
        ImplWriteChunk((sal_uInt8) mnInterlaced );  // interlace type
        ImplCloseChunk();
    }
    else
        mbStatus = sal_False;
    return mbStatus;
}

// ------------------------------------------------------------------------

void PNGWriterImpl::ImplWritePalette()
{
    const sal_uLong nCount = mpAccess->GetPaletteEntryCount();
    sal_uInt8*      pTempBuf = new sal_uInt8[ nCount*3 ];
    sal_uInt8*      pTmp = pTempBuf;

    ImplOpenChunk( PNGCHUNK_PLTE );

    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        const BitmapColor& rColor = mpAccess->GetPaletteColor( i );
        *pTmp++ = rColor.GetRed();
        *pTmp++ = rColor.GetGreen();
        *pTmp++ = rColor.GetBlue();
    }
    ImplWriteChunk( pTempBuf, nCount*3 );
    ImplCloseChunk();
    delete[] pTempBuf;
}

// ------------------------------------------------------------------------

void PNGWriterImpl::ImplWriteTransparent ()
{
    const sal_uLong nTransIndex = mpAccess->GetBestMatchingColor( BMP_COL_TRANS );

    ImplOpenChunk( PNGCHUNK_tRNS );

    for ( sal_uLong n = 0UL; n <= nTransIndex; n++ )
        ImplWriteChunk( ( nTransIndex == n ) ? (sal_uInt8) 0x0 : (sal_uInt8) 0xff );

    ImplCloseChunk();
}

// ------------------------------------------------------------------------

void PNGWriterImpl::ImplWritepHYs( const BitmapEx& rBmpEx )
{
    if ( rBmpEx.GetPrefMapMode() == MAP_100TH_MM )
    {
        Size aPrefSize( rBmpEx.GetPrefSize() );
        if ( aPrefSize.Width() && aPrefSize.Height() )
        {
            ImplOpenChunk( PNGCHUNK_pHYs );
            sal_uInt8 nMapUnit = 1;
            sal_uInt32 nPrefSizeX = (sal_uInt32)( (double)100000.0 / ( (double)aPrefSize.Width() / mnWidth ) + 0.5 );
            sal_uInt32 nPrefSizeY = (sal_uInt32)( (double)100000.0 / ( (double)aPrefSize.Height() / mnHeight ) + 0.5 );
            ImplWriteChunk( nPrefSizeX );
            ImplWriteChunk( nPrefSizeY );
            ImplWriteChunk( nMapUnit );
            ImplCloseChunk();
        }
    }
}

// ------------------------------------------------------------------------

void PNGWriterImpl::ImplWriteIDAT ()
{
    mnDeflateInSize = mnBitsPerPixel;

    if( mpMaskAccess )
        mnDeflateInSize += 8;

    mnBBP = ( mnDeflateInSize + 7 ) >> 3;

    mnDeflateInSize = mnBBP * mnWidth + 1;

    mpDeflateInBuf = new sal_uInt8[ mnDeflateInSize ];

    if ( mnFilterType )         // using filter type 4 we need memory for the scanline 3 times
    {
        mpPreviousScan = new sal_uInt8[ mnDeflateInSize ];
        mpCurrentScan = new sal_uInt8[ mnDeflateInSize ];
        ImplClearFirstScanline();
    }
    mpZCodec->BeginCompression( ZCODEC_PNG_DEFAULT + mnCompLevel );
    mpZCodec->SetCRC( mnCRC );
    SvMemoryStream aOStm;
    if ( mnInterlaced == 0 )
    {
        for ( sal_uLong nY = 0; nY < mnHeight; nY++ )
            mpZCodec->Write( aOStm, mpDeflateInBuf, ImplGetFilter( nY ) );
    }
    else
    {
        // interlace mode
        sal_uLong nY;
        for ( nY = 0; nY < mnHeight; nY+=8 )                                                // pass 1
            mpZCodec->Write( aOStm, mpDeflateInBuf, ImplGetFilter ( nY, 0, 8 ) );
        ImplClearFirstScanline();

        for ( nY = 0; nY < mnHeight; nY+=8 )                                                // pass 2
            mpZCodec->Write( aOStm, mpDeflateInBuf, ImplGetFilter ( nY, 4, 8 ) );
        ImplClearFirstScanline();

        if ( mnHeight >= 5 )                                                                // pass 3
        {
            for ( nY = 4; nY < mnHeight; nY+=8 )
                mpZCodec->Write( aOStm, mpDeflateInBuf, ImplGetFilter ( nY, 0, 4 ) );
            ImplClearFirstScanline();
        }

        for ( nY = 0; nY < mnHeight; nY+=4 )                                                // pass 4
            mpZCodec->Write( aOStm, mpDeflateInBuf, ImplGetFilter ( nY, 2, 4 ) );
        ImplClearFirstScanline();

        if ( mnHeight >= 3 )                                                                // pass 5
        {
            for ( nY = 2; nY < mnHeight; nY+=4 )
                mpZCodec->Write( aOStm, mpDeflateInBuf, ImplGetFilter ( nY, 0, 2 ) );
            ImplClearFirstScanline();
        }

        for ( nY = 0; nY < mnHeight; nY+=2 )                                                // pass 6
            mpZCodec->Write( aOStm, mpDeflateInBuf, ImplGetFilter ( nY, 1, 2 ) );
        ImplClearFirstScanline();

        if ( mnHeight >= 2 )                                                                // pass 7
        {
            for ( nY = 1; nY < mnHeight; nY+=2 )
                mpZCodec->Write( aOStm, mpDeflateInBuf, ImplGetFilter ( nY, 0, 1 ) );
        }
    }
    mpZCodec->EndCompression();
    mnCRC = mpZCodec->GetCRC();

    if ( mnFilterType )         // using filter type 4 we need memory for the scanline 3 times
    {
        delete[] mpCurrentScan;
        delete[] mpPreviousScan;
    }
    delete[] mpDeflateInBuf;

    sal_uInt32 nIDATSize = aOStm.Tell();
    sal_uInt32 nBytes, nBytesToWrite = nIDATSize;
    while( nBytesToWrite )
    {
        nBytes = nBytesToWrite <= mnMaxChunkSize ? nBytesToWrite : mnMaxChunkSize;
        ImplOpenChunk( PNGCHUNK_IDAT );
        ImplWriteChunk( (unsigned char*)aOStm.GetData() + ( nIDATSize - nBytesToWrite ), nBytes );
        ImplCloseChunk();
        nBytesToWrite -= nBytes;
    }
}

// ---------------------------------------------------------------------------------------------------
// ImplGetFilter writes the complete Scanline (nY) - in interlace mode the parameter nXStart and nXAdd
// appends to the currently used pass
// the complete size of scanline will be returned - in interlace mode zero is possible!

sal_uLong PNGWriterImpl::ImplGetFilter ( sal_uLong nY, sal_uLong nXStart, sal_uLong nXAdd )
{
    sal_uInt8* pDest;

    if ( mnFilterType )
        pDest = mpCurrentScan;
    else
        pDest = mpDeflateInBuf;

    if ( nXStart < mnWidth )
    {
        *pDest++ = mnFilterType;        // in this version the filter type is either 0 or 4

        if ( mpAccess->HasPalette() )   // alphachannel is not allowed by pictures including palette entries
        {
            switch ( mnBitsPerPixel )
            {
                case( 1 ):
                {
                    sal_uLong nX, nXIndex;
                    for ( nX = nXStart, nXIndex = 0; nX < mnWidth; nX+=nXAdd, nXIndex++ )
                    {
                        sal_uLong nShift = ( nXIndex & 7 ) ^ 7;
                        if ( nShift == 7)
                            *pDest = (sal_uInt8)(mpAccess->GetPixel( nY, nX ) << nShift);
                        else if  ( nShift == 0 )
                            *pDest++ |= (sal_uInt8) mpAccess->GetPixel( nY, nX ) << nShift;
                        else
                            *pDest |= (sal_uInt8) mpAccess->GetPixel( nY, nX ) << nShift;
                    }
                    if ( ( nXIndex & 7 ) != 0 ) pDest++;    // byte is not completely used, so the
                }                                           // bufferpointer is to correct
                break;

                case( 4 ):
                {
                    sal_uLong nX, nXIndex;
                    for ( nX = nXStart, nXIndex = 0; nX < mnWidth; nX+= nXAdd, nXIndex++ )
                    {
                        if( nXIndex & 1 )
                            *pDest++ |= (sal_uInt8) mpAccess->GetPixel( nY, nX );
                        else
                            *pDest = (sal_uInt8) mpAccess->GetPixel( nY, nX ) << 4;
                    }
                    if ( nXIndex & 1 ) pDest++;
                }
                break;

                case( 8 ):
                {
                    for ( sal_uLong nX = nXStart; nX < mnWidth; nX+=nXAdd )
                        *pDest++ = mpAccess->GetPixel( nY, nX );
                }
                break;

                default :
                    mbStatus = sal_False;
                break;
            }
        }
        else
        {
            if ( mpMaskAccess )             // mpMaskAccess != NULL -> alphachannel is to create
            {
                if ( mbTrueAlpha )
                {
                    for ( sal_uLong nX = nXStart; nX < mnWidth; nX += nXAdd )
                    {
                        const BitmapColor& rColor = mpAccess->GetPixel( nY, nX );
                        *pDest++ = rColor.GetRed();
                        *pDest++ = rColor.GetGreen();
                        *pDest++ = rColor.GetBlue();
                        *pDest++ = 255 - mpMaskAccess->GetPixel( nY, nX );
                    }
                }
                else
                {
                    const BitmapColor aTrans( mpMaskAccess->GetBestMatchingColor( Color( COL_WHITE ) ) );

                    for ( sal_uLong nX = nXStart; nX < mnWidth; nX+=nXAdd )
                    {
                        const BitmapColor& rColor = mpAccess->GetPixel( nY, nX );
                        *pDest++ = rColor.GetRed();
                        *pDest++ = rColor.GetGreen();
                        *pDest++ = rColor.GetBlue();

                        if( mpMaskAccess->GetPixel( nY, nX ) == aTrans )
                            *pDest++ = 0;
                        else
                            *pDest++ = 0xff;
                    }
                }
            }
            else
            {
                for ( sal_uLong nX = nXStart; nX < mnWidth; nX+=nXAdd )
                {
                    const BitmapColor& rColor = mpAccess->GetPixel( nY, nX );
                    *pDest++ = rColor.GetRed();
                    *pDest++ = rColor.GetGreen();
                    *pDest++ = rColor.GetBlue();
                }
            }
        }
    }
    // filter type4 ( PAETH ) will be used only for 24bit graphics
    if ( mnFilterType )
    {
        mnDeflateInSize = pDest - mpCurrentScan;
        pDest = mpDeflateInBuf;
        *pDest++ = 4;                                   // filter type

        sal_uLong na, nb, nc;
        long  np, npa, npb, npc;

        sal_uInt8* p1 = mpCurrentScan + 1;                  // Current Pixel
        sal_uInt8* p2 = p1 - mnBBP;                         // left pixel
        sal_uInt8* p3 = mpPreviousScan;                     // upper pixel
        sal_uInt8* p4 = p3 - mnBBP;                         // upperleft Pixel;

        while ( pDest < mpDeflateInBuf + mnDeflateInSize )
        {
            nb = *p3++;
            if ( p2 >= mpCurrentScan + 1 )
            {
                na = *p2;
                nc = *p4;
            }
            else
                na = nc = 0;

            np = na + nb;
            np -= nc;
            npa = np - na;
            npb = np - nb;
            npc = np - nc;
            if ( npa < 0 )
                npa =-npa;
            if ( npb < 0 )
                npb =-npb;
            if ( npc < 0 )
                npc =-npc;
            if ( ( npa <= npb ) && ( npa <= npc ) ) *pDest++ = *p1++ - (sal_uInt8)na;
            else if ( npb <= npc ) *pDest++ = *p1++ - (sal_uInt8)nb;
            else *pDest++ = *p1++ - (sal_uInt8)nc;
            p4++;
            p2++;
        }
        for ( long i = 0; i < (long)( mnDeflateInSize - 1 ); i++ )
            mpPreviousScan[ i ] = mpCurrentScan[ i + 1 ];
    }
    else
        mnDeflateInSize = pDest - mpDeflateInBuf;
    return ( mnDeflateInSize );
}

// ------------------------------------------------------------------------

void PNGWriterImpl::ImplClearFirstScanline()
{
    if ( mnFilterType )
        memset( mpPreviousScan, 0, mnDeflateInSize );
}

// ------------------------------------------------------------------------

void PNGWriterImpl::ImplOpenChunk ( sal_uLong nChunkType )
{
    maChunkSeq.resize( maChunkSeq.size() + 1 );
    maChunkSeq.back().nType = nChunkType;
}

// ------------------------------------------------------------------------

void PNGWriterImpl::ImplWriteChunk ( sal_uInt8 nSource )
{
    maChunkSeq.back().aData.push_back( nSource );
}

void PNGWriterImpl::ImplWriteChunk ( sal_uInt32 nSource )
{
    vcl::PNGWriter::ChunkData& rChunkData = maChunkSeq.back();
    rChunkData.aData.push_back( (sal_uInt8)( nSource >> 24 ) );
    rChunkData.aData.push_back( (sal_uInt8)( nSource >> 16 ) );
    rChunkData.aData.push_back( (sal_uInt8)( nSource >> 8 ) );
    rChunkData.aData.push_back( (sal_uInt8)( nSource ) );
}

void PNGWriterImpl::ImplWriteChunk ( unsigned char* pSource, sal_uInt32 nDatSize )
{
    if ( nDatSize )
    {
        vcl::PNGWriter::ChunkData& rChunkData = maChunkSeq.back();
        sal_uInt32 nSize = rChunkData.aData.size();
        rChunkData.aData.resize( nSize + nDatSize );
        memcpy( &rChunkData.aData[ nSize ], pSource, nDatSize );
    }
}

// ------------------------------------------------------------------------
// nothing to do
void PNGWriterImpl::ImplCloseChunk ( void ) const
{
}

// -------------
// - PNGWriter -
// -------------

PNGWriter::PNGWriter( const BitmapEx& rBmpEx,
    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData ) :
    mpImpl( new ::vcl::PNGWriterImpl( rBmpEx, pFilterData ) )
{
}

// ------------------------------------------------------------------------

PNGWriter::~PNGWriter()
{
    delete mpImpl;
}

// ------------------------------------------------------------------------

sal_Bool PNGWriter::Write( SvStream& rIStm )
{
    return mpImpl->Write( rIStm );
}

// ------------------------------------------------------------------------

std::vector< vcl::PNGWriter::ChunkData >& PNGWriter::GetChunks()
{
    return mpImpl->GetChunks();
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
