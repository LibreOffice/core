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


#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/outdev.hxx>
#include <vcl/bitmap.hxx>

#include <utility>

#if defined HAVE_VALGRIND_HEADERS
#include <valgrind/memcheck.h>
#endif

#define DIBCOREHEADERSIZE           ( 12UL )
#define DIBINFOHEADERSIZE           ( sizeof( DIBInfoHeader ) )
#define BITMAPINFOHEADER                        0x28

// Compression defines
#define COMPRESS_OWN                ('S'|('D'<<8UL))
#define COMPRESS_NONE               ( 0UL )
#define RLE_8                       ( 1UL )
#define RLE_4                       ( 2UL )
#define BITFIELDS                   ( 3UL )
#define ZCOMPRESS                   ( COMPRESS_OWN | 0x01000000UL ) /* == 'SD01' (binary) */

struct DIBInfoHeader
{
    sal_uInt32      nSize;
    sal_Int32       nWidth;
    sal_Int32       nHeight;
    sal_uInt16      nPlanes;
    sal_uInt16      nBitCount;
    sal_uInt32      nCompression;
    sal_uInt32      nSizeImage;
    sal_Int32       nXPelsPerMeter;
    sal_Int32       nYPelsPerMeter;
    sal_uInt32      nColsUsed;
    sal_uInt32      nColsImportant;

                    DIBInfoHeader() :
                        nSize( 0UL ),
                        nWidth( 0UL ),
                        nHeight( 0UL ),
                        nPlanes( 0 ),
                        nBitCount( 0 ),
                        nCompression( 0 ),
                        nSizeImage( 0 ),
                        nXPelsPerMeter( 0UL ),
                        nYPelsPerMeter( 0UL ),
                        nColsUsed( 0UL ),
                        nColsImportant( 0UL ) {}

                    ~DIBInfoHeader() {}
};

namespace
{
    inline sal_uInt16 discretizeBitcount( sal_uInt16 nInputCount )
    {
        return ( nInputCount <= 1 ) ? 1 :
               ( nInputCount <= 4 ) ? 4 :
               ( nInputCount <= 8 ) ? 8 : 24;
    }

    inline bool isBitfieldCompression( sal_uLong nScanlineFormat )
    {
        return nScanlineFormat == BMP_FORMAT_16BIT_TC_LSB_MASK ||
            nScanlineFormat == BMP_FORMAT_32BIT_TC_MASK;
    }
}

SvStream& operator>>( SvStream& rIStm, Bitmap& rBitmap )
{
    rBitmap.Read( rIStm, sal_True );
    return rIStm;
}

SvStream& operator<<( SvStream& rOStm, const Bitmap& rBitmap )
{
    rBitmap.Write( rOStm, sal_False, sal_True );
    return rOStm;
}

sal_Bool Bitmap::Read( SvStream& rIStm, sal_Bool bFileHeader, sal_Bool bIsMSOFormat )
{
    const sal_uInt16    nOldFormat = rIStm.GetNumberFormatInt();
    const sal_uLong     nOldPos = rIStm.Tell();
    sal_uLong           nOffset = 0UL;
    sal_Bool            bRet = sal_False;

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    if( bFileHeader )
    {
        if( ImplReadDIBFileHeader( rIStm, nOffset ) )
            bRet = ImplReadDIB( rIStm, *this, nOffset );
    }
    else
        bRet = ImplReadDIB( rIStm, *this, nOffset, bIsMSOFormat );

    if( !bRet )
    {
        if( !rIStm.GetError() )
            rIStm.SetError( SVSTREAM_GENERALERROR );

        rIStm.Seek( nOldPos );
    }

    rIStm.SetNumberFormatInt( nOldFormat );

    return bRet;
}

sal_Bool Bitmap::ImplReadDIB( SvStream& rIStm, Bitmap& rBmp, sal_uLong nOffset, sal_Bool bIsMSOFormat )
{
    DIBInfoHeader   aHeader;
    const sal_uLong     nStmPos = rIStm.Tell();
    sal_Bool            bRet = sal_False;
    sal_Bool        bTopDown = sal_False;

    if( ImplReadDIBInfoHeader( rIStm, aHeader, bTopDown, bIsMSOFormat ) && aHeader.nWidth && aHeader.nHeight && aHeader.nBitCount )
    {
        const sal_uInt16 nBitCount( discretizeBitcount(aHeader.nBitCount) );

        const Size          aSizePixel( aHeader.nWidth, abs(aHeader.nHeight) );
        BitmapPalette       aDummyPal;
        Bitmap              aNewBmp( aSizePixel, nBitCount, &aDummyPal );
        BitmapWriteAccess*  pAcc = aNewBmp.AcquireWriteAccess();

        if( pAcc )
        {
            sal_uInt16          nColors;
            SvStream*       pIStm;
            SvMemoryStream* pMemStm = NULL;
            sal_uInt8*          pData = NULL;

            if( nBitCount <= 8 )
            {
                if( aHeader.nColsUsed )
                    nColors = (sal_uInt16) aHeader.nColsUsed;
                else
                    nColors = ( 1 << aHeader.nBitCount );
            }
            else
                nColors = 0;

            if( ZCOMPRESS == aHeader.nCompression )
            {
                ZCodec  aCodec;
                sal_uInt32 nCodedSize, nUncodedSize;
                sal_uLong   nCodedPos;

                // read coding information
                rIStm >> nCodedSize >> nUncodedSize >> aHeader.nCompression;
                pData = (sal_uInt8*) rtl_allocateMemory( nUncodedSize );

                // decode buffer
                nCodedPos = rIStm.Tell();
                aCodec.BeginCompression();
                aCodec.Read( rIStm, pData, nUncodedSize );
                aCodec.EndCompression();

                // skip unread bytes from coded buffer
                rIStm.SeekRel( nCodedSize - ( rIStm.Tell() - nCodedPos ) );

                // set decoded bytes to memory stream,
                // from which we will read the bitmap data
                pMemStm = new SvMemoryStream;
                pIStm = pMemStm;
                pMemStm->SetBuffer( (char*) pData, nUncodedSize, sal_False, nUncodedSize );
                nOffset = 0;
            }
            else
                pIStm = &rIStm;

            // read palette
            if( nColors )
            {
                pAcc->SetPaletteEntryCount( nColors );
                ImplReadDIBPalette( *pIStm, *pAcc, aHeader.nSize != DIBCOREHEADERSIZE );
            }

            // read bits
            if( !pIStm->GetError() )
            {
                if( nOffset )
                    pIStm->SeekRel( nOffset - ( pIStm->Tell() - nStmPos ) );

                bRet = ImplReadDIBBits( *pIStm, aHeader, *pAcc, bTopDown );

                if( bRet && aHeader.nXPelsPerMeter && aHeader.nYPelsPerMeter )
                {
                    MapMode aMapMode( MAP_MM, Point(),
                                      Fraction( 1000, aHeader.nXPelsPerMeter ),
                                      Fraction( 1000, aHeader.nYPelsPerMeter ) );

                    aNewBmp.SetPrefMapMode( aMapMode );
                    aNewBmp.SetPrefSize( Size( aHeader.nWidth, abs(aHeader.nHeight) ) );
                }
            }

            if( pData )
                rtl_freeMemory( pData );

            delete pMemStm;
            aNewBmp.ReleaseAccess( pAcc );

            if( bRet )
                rBmp = aNewBmp;
        }
    }

    return bRet;
}

sal_Bool Bitmap::ImplReadDIBFileHeader( SvStream& rIStm, sal_uLong& rOffset )
{
    sal_uInt32  nTmp32;
    sal_uInt16  nTmp16 = 0;
    sal_Bool    bRet = sal_False;

    rIStm >> nTmp16;

    if ( ( 0x4D42 == nTmp16 ) || ( 0x4142 == nTmp16 ) )
    {
        if ( 0x4142 == nTmp16 )
        {
            rIStm.SeekRel( 12L );
            rIStm >> nTmp16;
            rIStm.SeekRel( 8L );
            rIStm >> nTmp32;
            rOffset = nTmp32 - 28UL;
            bRet = ( 0x4D42 == nTmp16 );
        }
        else
        {
            rIStm.SeekRel( 8L );
            rIStm >> nTmp32;
            rOffset = nTmp32 - 14UL;
            bRet = ( rIStm.GetError() == 0UL );
        }
    }
    else
        rIStm.SetError( SVSTREAM_FILEFORMAT_ERROR );

    return bRet;
}

sal_Bool Bitmap::ImplReadDIBInfoHeader( SvStream& rIStm, DIBInfoHeader& rHeader, sal_Bool& bTopDown, sal_Bool bIsMSOFormat )
{
    // BITMAPINFOHEADER or BITMAPCOREHEADER
    rIStm >> rHeader.nSize;

    // BITMAPCOREHEADER
    sal_Int16 nTmp16 = 0;
    if ( rHeader.nSize == DIBCOREHEADERSIZE )
    {

        rIStm >> nTmp16; rHeader.nWidth = nTmp16;
        rIStm >> nTmp16; rHeader.nHeight = nTmp16;
        rIStm >> rHeader.nPlanes;
        rIStm >> rHeader.nBitCount;
    }
    else if ( bIsMSOFormat && ( rHeader.nSize == BITMAPINFOHEADER ) )
    {
        sal_uInt8 nTmp8 = 0;
        rIStm >> nTmp16; rHeader.nWidth = nTmp16;
        rIStm >> nTmp16; rHeader.nHeight = nTmp16;
        rIStm >> nTmp8; rHeader.nPlanes = nTmp8;
        rIStm >> nTmp8; rHeader.nBitCount = nTmp8;
        rIStm >> nTmp16; rHeader.nSizeImage = nTmp16;
        rIStm >> nTmp16; rHeader.nCompression = nTmp16;
        if ( !rHeader.nSizeImage ) // uncompressed?
            rHeader.nSizeImage = ((rHeader.nWidth * rHeader.nBitCount + 31) & ~31) / 8 * rHeader.nHeight;
        rIStm >> rHeader.nXPelsPerMeter;
        rIStm >> rHeader.nYPelsPerMeter;
        rIStm >> rHeader.nColsUsed;
        rIStm >> rHeader.nColsImportant;
    }
    else
    {
        // unknown Header
        if( rHeader.nSize < DIBINFOHEADERSIZE )
        {
            sal_uLong nUnknownSize = sizeof( rHeader.nSize );

            rIStm >> rHeader.nWidth; nUnknownSize += sizeof( rHeader.nWidth );
            rIStm >> rHeader.nHeight; nUnknownSize += sizeof( rHeader.nHeight );
            rIStm >> rHeader.nPlanes; nUnknownSize += sizeof( rHeader.nPlanes );
            rIStm >> rHeader.nBitCount; nUnknownSize += sizeof( rHeader.nBitCount );

            if( nUnknownSize < rHeader.nSize )
            {
                rIStm >> rHeader.nCompression;
                nUnknownSize += sizeof( rHeader.nCompression );

                if( nUnknownSize < rHeader.nSize )
                {
                    rIStm >> rHeader.nSizeImage;
                    nUnknownSize += sizeof( rHeader.nSizeImage );

                    if( nUnknownSize < rHeader.nSize )
                    {
                        rIStm >> rHeader.nXPelsPerMeter;
                        nUnknownSize += sizeof( rHeader.nXPelsPerMeter );

                        if( nUnknownSize < rHeader.nSize )
                        {
                            rIStm >> rHeader.nYPelsPerMeter;
                            nUnknownSize += sizeof( rHeader.nYPelsPerMeter );
                        }

                        if( nUnknownSize < rHeader.nSize )
                        {
                            rIStm >> rHeader.nColsUsed;
                            nUnknownSize += sizeof( rHeader.nColsUsed );

                            if( nUnknownSize < rHeader.nSize )
                            {
                                rIStm >> rHeader.nColsImportant;
                                nUnknownSize += sizeof( rHeader.nColsImportant );
                            }
                        }
                    }
                }
            }
        }
        else
        {
            rIStm >> rHeader.nWidth;
            rIStm >> rHeader.nHeight; //rHeader.nHeight=abs(rHeader.nHeight);
            rIStm >> rHeader.nPlanes;
            rIStm >> rHeader.nBitCount;
            rIStm >> rHeader.nCompression;
            rIStm >> rHeader.nSizeImage;
            rIStm >> rHeader.nXPelsPerMeter;
            rIStm >> rHeader.nYPelsPerMeter;
            rIStm >> rHeader.nColsUsed;
            rIStm >> rHeader.nColsImportant;
        }

        // Skip to palette if we can
        if ( rHeader.nSize > DIBINFOHEADERSIZE )
            rIStm.SeekRel( rHeader.nSize - DIBINFOHEADERSIZE );
    }
    if ( rHeader.nHeight < 0 )
    {
        bTopDown = sal_True;
        rHeader.nHeight *= -1;
    }
    else
        bTopDown = sal_False;

    if ( rHeader.nWidth < 0 )
        rIStm.SetError( SVSTREAM_FILEFORMAT_ERROR );

    // Protect against damaged files a little bit
    if( rHeader.nSizeImage > ( 16 * static_cast< sal_uInt32 >( rHeader.nWidth * rHeader.nHeight ) ) )
        rHeader.nSizeImage = 0;

    return( ( rHeader.nPlanes == 1 ) && ( rIStm.GetError() == 0UL ) );
}

sal_Bool Bitmap::ImplReadDIBPalette( SvStream& rIStm, BitmapWriteAccess& rAcc, sal_Bool bQuad )
{
    const sal_uInt16    nColors = rAcc.GetPaletteEntryCount();
    const sal_uLong     nPalSize = nColors * ( bQuad ? 4UL : 3UL );
    BitmapColor     aPalColor;

    sal_uInt8* pEntries = new sal_uInt8[ nPalSize ];
    rIStm.Read( pEntries, nPalSize );

    sal_uInt8* pTmpEntry = pEntries;
    for( sal_uInt16 i = 0; i < nColors; i++ )
    {
        aPalColor.SetBlue( *pTmpEntry++ );
        aPalColor.SetGreen( *pTmpEntry++ );
        aPalColor.SetRed( *pTmpEntry++ );

        if( bQuad )
            pTmpEntry++;

        rAcc.SetPaletteColor( i, aPalColor );
    }

    delete[] pEntries;

    return( rIStm.GetError() == 0UL );
}

sal_Bool Bitmap::ImplReadDIBBits( SvStream& rIStm, DIBInfoHeader& rHeader, BitmapWriteAccess& rAcc, sal_Bool bTopDown )
{
    const sal_uLong nAlignedWidth = AlignedWidth4Bytes( rHeader.nWidth * rHeader.nBitCount );
    sal_uInt32      nRMask = 0;
    sal_uInt32      nGMask = 0;
    sal_uInt32      nBMask = 0;
    sal_Bool        bNative;
    sal_Bool        bTCMask = ( rHeader.nBitCount == 16 ) || ( rHeader.nBitCount == 32 );
    sal_Bool        bRLE = ( RLE_8 == rHeader.nCompression && rHeader.nBitCount == 8 ) ||
                       ( RLE_4 == rHeader.nCompression && rHeader.nBitCount == 4 );

    // Is native format?
    switch( rAcc.GetScanlineFormat() )
    {
        case( BMP_FORMAT_1BIT_MSB_PAL ):
        case( BMP_FORMAT_4BIT_MSN_PAL ):
        case( BMP_FORMAT_8BIT_PAL ):
        case( BMP_FORMAT_24BIT_TC_BGR ):
            bNative = ( ( rAcc.IsBottomUp() != bTopDown ) && !bRLE && !bTCMask && ( rAcc.GetScanlineSize() == nAlignedWidth ) );
        break;

        default:
            bNative = sal_False;
        break;
    }
    // Read data
    if( bNative )
    {
        // true color DIB's can have a (optimization) palette
        if( rHeader.nColsUsed && rHeader.nBitCount > 8 )
            rIStm.SeekRel( rHeader.nColsUsed * ( ( rHeader.nSize != DIBCOREHEADERSIZE ) ? 4 : 3 ) );

        if ( rHeader.nHeight > 0 )
            rIStm.Read( rAcc.GetBuffer(), rHeader.nHeight * nAlignedWidth );
        else
        {
            for( int i = abs(rHeader.nHeight)-1; i >= 0; i-- )
                rIStm.Read( ((char*)rAcc.GetBuffer()) + (nAlignedWidth*i), nAlignedWidth );
        }
    }
    else
    {
        // Read color mask
        if( bTCMask )
        {
            if( rHeader.nCompression == BITFIELDS )
            {
                rIStm.SeekRel( -12L );
                rIStm >> nRMask;
                rIStm >> nGMask;
                rIStm >> nBMask;
            }
            else
            {
                nRMask = ( rHeader.nBitCount == 16 ) ? 0x00007c00UL : 0x00ff0000UL;
                nGMask = ( rHeader.nBitCount == 16 ) ? 0x000003e0UL : 0x0000ff00UL;
                nBMask = ( rHeader.nBitCount == 16 ) ? 0x0000001fUL : 0x000000ffUL;
            }
        }

        if( bRLE )
        {
            if ( !rHeader.nSizeImage )
            {
                const sal_uLong nOldPos = rIStm.Tell();

                rIStm.Seek( STREAM_SEEK_TO_END );
                rHeader.nSizeImage = rIStm.Tell() - nOldPos;
                rIStm.Seek( nOldPos );
            }

            sal_uInt8* pBuffer = (sal_uInt8*) rtl_allocateMemory( rHeader.nSizeImage );

            rIStm.Read( (char*) pBuffer, rHeader.nSizeImage );
            ImplDecodeRLE( pBuffer, rHeader, rAcc, RLE_4 == rHeader.nCompression );

            rtl_freeMemory( pBuffer );
        }
        else
        {
            const long  nWidth = rHeader.nWidth;
            const long  nHeight = abs(rHeader.nHeight);
            sal_uInt8*      pBuf = new sal_uInt8[ nAlignedWidth ];

            // true color DIB's can have a (optimization) palette
            if( rHeader.nColsUsed && rHeader.nBitCount > 8 )
                rIStm.SeekRel( rHeader.nColsUsed * ( ( rHeader.nSize != DIBCOREHEADERSIZE ) ? 4 : 3 ) );

            const long nI = bTopDown ? 1 : -1;
            long nY = bTopDown ? 0 : nHeight - 1;
            long nCount = nHeight;

            switch( rHeader.nBitCount )
            {
                case( 1 ):
                {
                    sal_uInt8*  pTmp;
                    sal_uInt8   cTmp;

                    for( ; nCount--; nY += nI )
                    {
                        rIStm.Read( pTmp = pBuf, nAlignedWidth );
                        cTmp = *pTmp++;

                        for( long nX = 0L, nShift = 8L; nX < nWidth; nX++ )
                        {
                            if( !nShift )
                            {
                                nShift = 8L,
                                cTmp = *pTmp++;
                            }

                            rAcc.SetPixel( nY, nX, sal::static_int_cast<sal_uInt8>(( cTmp >> --nShift ) & 1) );
                        }
                    }
                }
                break;

                case( 4 ):
                {
                    sal_uInt8*  pTmp;
                    sal_uInt8   cTmp;

                    for( ; nCount--; nY += nI )
                    {
                        rIStm.Read( pTmp = pBuf, nAlignedWidth );
                        cTmp = *pTmp++;

                        for( long nX = 0L, nShift = 2L; nX < nWidth; nX++ )
                        {
                            if( !nShift )
                            {
                                nShift = 2UL,
                                cTmp = *pTmp++;
                            }

                            rAcc.SetPixel( nY, nX, sal::static_int_cast<sal_uInt8>(( cTmp >> ( --nShift << 2UL ) ) & 0x0f) );
                        }
                    }
                }
                break;

                case( 8 ):
                {
                    sal_uInt8*  pTmp;

                    for( ; nCount--; nY += nI )
                    {
                        rIStm.Read( pTmp = pBuf, nAlignedWidth );

                        for( long nX = 0L; nX < nWidth; nX++ )
                            rAcc.SetPixel( nY, nX, *pTmp++ );
                    }
                }
                break;

                case( 16 ):
                {
                    ColorMask   aMask( nRMask, nGMask, nBMask );
                    BitmapColor aColor;
                    sal_uInt16*     pTmp16;

                    for( ; nCount--; nY += nI )
                    {
                        rIStm.Read( (char*)( pTmp16 = (sal_uInt16*) pBuf ), nAlignedWidth );

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            aMask.GetColorFor16BitLSB( aColor, (sal_uInt8*) pTmp16++ );
                            rAcc.SetPixel( nY, nX, aColor );
                        }
                    }
                }
                break;

                case( 24 ):
                {
                    BitmapColor aPixelColor;
                    sal_uInt8*      pTmp;

                    for( ; nCount--; nY += nI )
                    {
                        rIStm.Read( pTmp = pBuf, nAlignedWidth );

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            aPixelColor.SetBlue( *pTmp++ );
                            aPixelColor.SetGreen( *pTmp++ );
                            aPixelColor.SetRed( *pTmp++ );
                            rAcc.SetPixel( nY, nX, aPixelColor );
                        }
                    }
                }
                break;

                case( 32 ):
                {
                    ColorMask   aMask( nRMask, nGMask, nBMask );
                    BitmapColor aColor;
                    sal_uInt32*     pTmp32;

                    for( ; nCount--; nY += nI )
                    {
                        rIStm.Read( (char*)( pTmp32 = (sal_uInt32*) pBuf ), nAlignedWidth );

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            aMask.GetColorFor32Bit( aColor, (sal_uInt8*) pTmp32++ );
                            rAcc.SetPixel( nY, nX, aColor );
                        }
                    }
                }
            }

            delete[] pBuf;
        }
    }

    return( rIStm.GetError() == 0UL );
}

sal_Bool Bitmap::Write( SvStream& rOStm, sal_Bool bCompressed, sal_Bool bFileHeader ) const
{
    DBG_ASSERT( mpImpBmp, "Empty Bitmaps can't be saved" );

    const Size  aSizePix( GetSizePixel() );
    sal_Bool        bRet = sal_False;

    if( mpImpBmp && aSizePix.Width() && aSizePix.Height() )
    {
        BitmapReadAccess*   pAcc = ( (Bitmap*) this)->AcquireReadAccess();
        const sal_uInt16        nOldFormat = rOStm.GetNumberFormatInt();
        const sal_uLong         nOldPos = rOStm.Tell();

        rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

        if( pAcc )
        {
            if( bFileHeader )
            {
                if( ImplWriteDIBFileHeader( rOStm, *pAcc ) )
                    bRet = ImplWriteDIB( rOStm, *pAcc, bCompressed );
            }
            else
                bRet = ImplWriteDIB( rOStm, *pAcc, bCompressed );

            ( (Bitmap*) this)->ReleaseAccess( pAcc );
        }

        if( !bRet )
        {
            rOStm.SetError( SVSTREAM_GENERALERROR );
            rOStm.Seek( nOldPos );
        }

        rOStm.SetNumberFormatInt( nOldFormat );
    }

    return bRet;
}

sal_Bool Bitmap::ImplWriteDIB( SvStream& rOStm, BitmapReadAccess& rAcc, sal_Bool bCompressed ) const
{
    const MapMode   aMapPixel( MAP_PIXEL );
    DIBInfoHeader   aHeader;
    sal_uLong           nImageSizePos;
    sal_uLong           nEndPos;
    sal_uInt32      nCompression = 0;
    sal_Bool            bRet = sal_False;

    aHeader.nSize = DIBINFOHEADERSIZE;
    aHeader.nWidth = rAcc.Width();
    aHeader.nHeight = rAcc.Height();
    aHeader.nPlanes = 1;

    if( isBitfieldCompression( rAcc.GetScanlineFormat() ) )
    {
        aHeader.nBitCount = ( rAcc.GetScanlineFormat() == BMP_FORMAT_16BIT_TC_LSB_MASK ) ? 16 : 32;
        aHeader.nSizeImage = rAcc.Height() * rAcc.GetScanlineSize();

        nCompression = BITFIELDS;
    }
    else
    {
        // #i5xxx# Limit bitcount to 24bit, the 32 bit cases are
        // not handled properly below (would have to set color
        // masks, and nCompression=BITFIELDS - but color mask is
        // not set for formats != *_TC_*). Note that this very
        // problem might cause trouble at other places - the
        // introduction of 32 bit RGBA bitmaps is relatively
        // recent.
        // #i59239# discretize bitcount to 1,4,8,24 (other cases
        // are not written below)
        const sal_uInt16 nBitCount( sal::static_int_cast<sal_uInt16>(rAcc.GetBitCount()) );

        aHeader.nBitCount = discretizeBitcount( nBitCount );
        aHeader.nSizeImage = rAcc.Height() *
            AlignedWidth4Bytes( rAcc.Width()*aHeader.nBitCount );

        if( bCompressed )
        {
            if( 4 == nBitCount )
                nCompression = RLE_4;
            else if( 8 == nBitCount )
                nCompression = RLE_8;
        }
        else
            nCompression = COMPRESS_NONE;
    }

    if( ( rOStm.GetCompressMode() & COMPRESSMODE_ZBITMAP ) &&
        ( rOStm.GetVersion() >= SOFFICE_FILEFORMAT_40 ) )
    {
        aHeader.nCompression = ZCOMPRESS;
    }
    else
        aHeader.nCompression = nCompression;

    if( maPrefSize.Width() && maPrefSize.Height() && ( maPrefMapMode != aMapPixel ) )
    {
        // #i48108# Try to recover xpels/ypels as previously stored on
        // disk. The problem with just converting maPrefSize to 100th
        // mm and then relating that to the bitmap pixel size is that
        // MapMode is integer-based, and suffers from roundoffs,
        // especially if maPrefSize is small. Trying to circumvent
        // that by performing part of the math in floating point.
        const Size aScale100000(
            OutputDevice::LogicToLogic( Size(100000L,
                                             100000L),
                                        MAP_100TH_MM,
                                        maPrefMapMode ) );
        const double fBmpWidthM((double)maPrefSize.Width() / aScale100000.Width() );
        const double fBmpHeightM((double)maPrefSize.Height() / aScale100000.Height() );
        if( fabs(fBmpWidthM) > 0.000000001 &&
            fabs(fBmpHeightM) > 0.000000001 )
        {
            aHeader.nXPelsPerMeter = (sal_uInt32)(rAcc.Width() / fBmpWidthM + .5);
            aHeader.nYPelsPerMeter = (sal_uInt32)(rAcc.Height() / fBmpHeightM + .5);
        }
    }

    aHeader.nColsUsed = ( ( aHeader.nBitCount <= 8 ) ? rAcc.GetPaletteEntryCount() : 0 );
    aHeader.nColsImportant = 0;

    rOStm << aHeader.nSize;
    rOStm << aHeader.nWidth;
    rOStm << aHeader.nHeight;
    rOStm << aHeader.nPlanes;
    rOStm << aHeader.nBitCount;
    rOStm << aHeader.nCompression;

    nImageSizePos = rOStm.Tell();
    rOStm.SeekRel( sizeof( aHeader.nSizeImage ) );

    rOStm << aHeader.nXPelsPerMeter;
    rOStm << aHeader.nYPelsPerMeter;
    rOStm << aHeader.nColsUsed;
    rOStm << aHeader.nColsImportant;

    if( aHeader.nCompression == ZCOMPRESS )
    {
        ZCodec          aCodec;
        SvMemoryStream  aMemStm( aHeader.nSizeImage + 4096, 65535 );
        sal_uLong           nCodedPos = rOStm.Tell(), nLastPos;
        sal_uInt32      nCodedSize, nUncodedSize;

        // write uncoded data palette
        if( aHeader.nColsUsed )
            ImplWriteDIBPalette( aMemStm, rAcc );

        // write uncoded bits
        bRet = ImplWriteDIBBits( aMemStm, rAcc, nCompression, aHeader.nSizeImage );

        // get uncoded size
        nUncodedSize = aMemStm.Tell();

        // seek over compress info
        rOStm.SeekRel( 12 );

        // write compressed data
        aCodec.BeginCompression( 3 );
        aCodec.Write( rOStm, (sal_uInt8*) aMemStm.GetData(), nUncodedSize );
        aCodec.EndCompression();

        // update compress info ( coded size, uncoded size, uncoded compression )
        nCodedSize = ( nLastPos = rOStm.Tell() ) - nCodedPos - 12;
        rOStm.Seek( nCodedPos );
        rOStm << nCodedSize << nUncodedSize << nCompression;
        rOStm.Seek( nLastPos );

        if( bRet )
            bRet = ( rOStm.GetError() == ERRCODE_NONE );
    }
    else
    {
        if( aHeader.nColsUsed )
            ImplWriteDIBPalette( rOStm, rAcc );

        bRet = ImplWriteDIBBits( rOStm, rAcc, aHeader.nCompression, aHeader.nSizeImage );
    }

    nEndPos = rOStm.Tell();
    rOStm.Seek( nImageSizePos );
    rOStm << aHeader.nSizeImage;
    rOStm.Seek( nEndPos );

    return bRet;
}

sal_Bool Bitmap::ImplWriteDIBFileHeader( SvStream& rOStm, BitmapReadAccess& rAcc )
{
    sal_uInt32  nPalCount = ( rAcc.HasPalette() ? rAcc.GetPaletteEntryCount() :
                          isBitfieldCompression( rAcc.GetScanlineFormat() ) ? 3UL : 0UL );
    sal_uInt32  nOffset = 14 + DIBINFOHEADERSIZE + nPalCount * 4UL;

    rOStm << (sal_uInt16) 0x4D42;
    rOStm << (sal_uInt32) ( nOffset + ( rAcc.Height() * rAcc.GetScanlineSize() ) );
    rOStm << (sal_uInt16) 0;
    rOStm << (sal_uInt16) 0;
    rOStm << nOffset;

    return( rOStm.GetError() == 0UL );
}

sal_Bool Bitmap::ImplWriteDIBPalette( SvStream& rOStm, BitmapReadAccess& rAcc )
{
    const sal_uInt16    nColors = rAcc.GetPaletteEntryCount();
    const sal_uLong     nPalSize = nColors * 4UL;
    sal_uInt8*          pEntries = new sal_uInt8[ nPalSize ];
    sal_uInt8*          pTmpEntry = pEntries;
    BitmapColor     aPalColor;

    for( sal_uInt16 i = 0; i < nColors; i++ )
    {
        const BitmapColor& rPalColor = rAcc.GetPaletteColor( i );

        *pTmpEntry++ = rPalColor.GetBlue();
        *pTmpEntry++ = rPalColor.GetGreen();
        *pTmpEntry++ = rPalColor.GetRed();
        *pTmpEntry++ = 0;
    }

    rOStm.Write( pEntries, nPalSize );
    delete[] pEntries;

    return( rOStm.GetError() == 0UL );
}

#if defined HAVE_VALGRIND_HEADERS
namespace
{
    void blankExtraSpace(sal_uInt8 *pBits, long nWidth, long nScanlineSize, int nBitCount)
    {
        size_t nExtraSpaceInScanLine = nScanlineSize - nWidth * nBitCount / 8;
        if (nExtraSpaceInScanLine)
            memset(pBits + (nScanlineSize - nExtraSpaceInScanLine), 0, nExtraSpaceInScanLine);
    }
}
#endif

sal_Bool Bitmap::ImplWriteDIBBits( SvStream& rOStm, BitmapReadAccess& rAcc,
                               sal_uLong nCompression, sal_uInt32& rImageSize )
{
    if( BITFIELDS == nCompression )
    {
        const ColorMask&    rMask = rAcc.GetColorMask();
        SVBT32              aVal32;

        UInt32ToSVBT32( rMask.GetRedMask(), aVal32 );
        rOStm.Write( (sal_uInt8*) aVal32, 4UL );

        UInt32ToSVBT32( rMask.GetGreenMask(), aVal32 );
        rOStm.Write( (sal_uInt8*) aVal32, 4UL );

        UInt32ToSVBT32( rMask.GetBlueMask(), aVal32 );
        rOStm.Write( (sal_uInt8*) aVal32, 4UL );

        rImageSize = rOStm.Tell();

        if( rAcc.IsBottomUp() )
            rOStm.Write( rAcc.GetBuffer(), rAcc.Height() * rAcc.GetScanlineSize() );
        else
        {
            for( long nY = rAcc.Height() - 1, nScanlineSize = rAcc.GetScanlineSize(); nY >= 0L; nY-- )
                rOStm.Write( rAcc.GetScanline( nY ), nScanlineSize );
        }
    }
    else if( ( RLE_4 == nCompression ) || ( RLE_8 == nCompression ) )
    {
        rImageSize = rOStm.Tell();
        ImplWriteRLE( rOStm, rAcc, RLE_4 == nCompression );
    }
    else if( !nCompression )
    {
        // #i5xxx# Limit bitcount to 24bit, the 32 bit cases are not
        // handled properly below (would have to set color masks, and
        // nCompression=BITFIELDS - but color mask is not set for
        // formats != *_TC_*). Note that this very problem might cause
        // trouble at other places - the introduction of 32 bit RGBA
        // bitmaps is relatively recent.
        // #i59239# discretize bitcount for aligned width to 1,4,8,24
        // (other cases are not written below)
        const sal_uInt16 nBitCount( sal::static_int_cast<sal_uInt16>(rAcc.GetBitCount()) );
        const sal_uLong  nAlignedWidth = AlignedWidth4Bytes( rAcc.Width() *
                                                         discretizeBitcount(nBitCount));
        sal_Bool         bNative = sal_False;

        switch( rAcc.GetScanlineFormat() )
        {
            case( BMP_FORMAT_1BIT_MSB_PAL ):
            case( BMP_FORMAT_4BIT_MSN_PAL ):
            case( BMP_FORMAT_8BIT_PAL ):
            case( BMP_FORMAT_24BIT_TC_BGR ):
            {
                if( rAcc.IsBottomUp() && ( rAcc.GetScanlineSize() == nAlignedWidth ) )
                    bNative = sal_True;
            }
            break;

            default:
            break;
        }

        rImageSize = rOStm.Tell();

        if( bNative )
            rOStm.Write( rAcc.GetBuffer(), nAlignedWidth * rAcc.Height() );
        else
        {
            const long  nWidth = rAcc.Width();
            const long  nHeight = rAcc.Height();
            sal_uInt8*      pBuf = new sal_uInt8[ nAlignedWidth ];
#if defined HAVE_VALGRIND_HEADERS
            if (RUNNING_ON_VALGRIND)
                blankExtraSpace(pBuf, nWidth, nAlignedWidth, discretizeBitcount(nBitCount));
#endif
            sal_uInt8*      pTmp;
            sal_uInt8       cTmp;

            switch( nBitCount )
            {
                case( 1 ):
                {
                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
                    {
                        pTmp = pBuf;
                        cTmp = 0;

                        for( long nX = 0L, nShift = 8L; nX < nWidth; nX++ )
                        {
                            if( !nShift )
                            {
                                nShift = 8L;
                                *pTmp++ = cTmp;
                                cTmp = 0;
                            }

                            cTmp |= ( (sal_uInt8) rAcc.GetPixel( nY, nX ) << --nShift );
                        }

                        *pTmp = cTmp;
                        rOStm.Write( pBuf, nAlignedWidth );
                    }
                }
                break;

                case( 4 ):
                {
                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
                    {
                        pTmp = pBuf;
                        cTmp = 0;

                        for( long nX = 0L, nShift = 2L; nX < nWidth; nX++ )
                        {
                            if( !nShift )
                            {
                                nShift = 2L;
                                *pTmp++ = cTmp;
                                cTmp = 0;
                            }

                            cTmp |= ( (sal_uInt8) rAcc.GetPixel( nY, nX ) << ( --nShift << 2L ) );
                        }
                        *pTmp = cTmp;
                        rOStm.Write( pBuf, nAlignedWidth );
                    }
                }
                break;

                case( 8 ):
                {
                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
                    {
                        pTmp = pBuf;

                        for( long nX = 0L; nX < nWidth; nX++ )
                            *pTmp++ = rAcc.GetPixel( nY, nX );

                        rOStm.Write( pBuf, nAlignedWidth );
                    }
                }
                break;

                // #i59239# fallback to 24 bit format, if bitcount is non-default
                default:
                    // FALLTHROUGH intended
                case( 24 ):
                {
                    BitmapColor aPixelColor;

                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
                    {
                        pTmp = pBuf;

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            aPixelColor = rAcc.GetPixel( nY, nX );
                            *pTmp++ = aPixelColor.GetBlue();
                            *pTmp++ = aPixelColor.GetGreen();
                            *pTmp++ = aPixelColor.GetRed();
                        }

                        rOStm.Write( pBuf, nAlignedWidth );
                    }
                }
                break;
            }

            delete[] pBuf;
        }
    }

    rImageSize = rOStm.Tell() - rImageSize;

    return( rOStm.GetError() == 0UL );
}

void Bitmap::ImplDecodeRLE( sal_uInt8* pBuffer, DIBInfoHeader& rHeader,
                            BitmapWriteAccess& rAcc, sal_Bool bRLE4 )
{
    Scanline    pRLE = pBuffer;
    long        nY = abs(rHeader.nHeight) - 1L;
    const sal_uLong nWidth = rAcc.Width();
    sal_uLong       nCountByte;
    sal_uLong       nRunByte;
    sal_uLong       nX = 0UL;
    sal_uInt8       cTmp;
    sal_Bool        bEndDecoding = sal_False;

    do
    {
        if( ( nCountByte = *pRLE++ ) == 0 )
        {
            nRunByte = *pRLE++;

            if( nRunByte > 2 )
            {
                if( bRLE4 )
                {
                    nCountByte = nRunByte >> 1;

                    for( sal_uLong i = 0UL; i < nCountByte; i++ )
                    {
                        cTmp = *pRLE++;

                        if( nX < nWidth )
                            rAcc.SetPixel( nY, nX++, cTmp >> 4 );

                        if( nX < nWidth )
                            rAcc.SetPixel( nY, nX++, cTmp & 0x0f );
                    }

                    if( nRunByte & 1 )
                    {
                        if( nX < nWidth )
                            rAcc.SetPixel( nY, nX++, *pRLE >> 4 );

                        pRLE++;
                    }

                    if( ( ( nRunByte + 1 ) >> 1 ) & 1 )
                        pRLE++;
                }
                else
                {
                    for( sal_uLong i = 0UL; i < nRunByte; i++ )
                    {
                        if( nX < nWidth )
                            rAcc.SetPixel( nY, nX++, *pRLE );

                        pRLE++;
                    }

                    if( nRunByte & 1 )
                        pRLE++;
                }
            }
            else if( !nRunByte )
            {
                nY--;
                nX = 0UL;
            }
            else if( nRunByte == 1 )
                bEndDecoding = sal_True;
            else
            {
                nX += *pRLE++;
                nY -= *pRLE++;
            }
        }
        else
        {
            cTmp = *pRLE++;

            if( bRLE4 )
            {
                nRunByte = nCountByte >> 1;

                for( sal_uLong i = 0UL; i < nRunByte; i++ )
                {
                    if( nX < nWidth )
                        rAcc.SetPixel( nY, nX++, cTmp >> 4 );

                    if( nX < nWidth )
                        rAcc.SetPixel( nY, nX++, cTmp & 0x0f );
                }

                if( ( nCountByte & 1 ) && ( nX < nWidth ) )
                    rAcc.SetPixel( nY, nX++, cTmp >> 4 );
            }
            else
            {
                for( sal_uLong i = 0UL; ( i < nCountByte ) && ( nX < nWidth ); i++ )
                    rAcc.SetPixel( nY, nX++, cTmp );
            }
        }
    }
    while ( !bEndDecoding && ( nY >= 0L ) );
}

sal_Bool Bitmap::ImplWriteRLE( SvStream& rOStm, BitmapReadAccess& rAcc, sal_Bool bRLE4 )
{
    const sal_uLong nWidth = rAcc.Width();
    const sal_uLong nHeight = rAcc.Height();
    sal_uLong       nX;
    sal_uLong       nSaveIndex;
    sal_uLong       nCount;
    sal_uLong       nBufCount;
    sal_uInt8*      pBuf = new sal_uInt8[ ( nWidth << 1 ) + 2 ];
    sal_uInt8*      pTmp;
    sal_uInt8       cPix;
    sal_uInt8       cLast;
    sal_Bool        bFound;

    for ( long nY = nHeight - 1L; nY >= 0L; nY-- )
    {
        pTmp = pBuf;
        nX = nBufCount = 0UL;

        while( nX < nWidth )
        {
            nCount = 1L;
            cPix = rAcc.GetPixel( nY, nX++ );

            while( ( nX < nWidth ) && ( nCount < 255L ) && ( cPix == rAcc.GetPixel( nY, nX ) ) )
            {
                nX++;
                nCount++;
            }

            if ( nCount > 1 )
            {
                *pTmp++ = (sal_uInt8) nCount;
                *pTmp++ = ( bRLE4 ? ( ( cPix << 4 ) | cPix ) : cPix );
                nBufCount += 2;
            }
            else
            {
                cLast = cPix;
                nSaveIndex = nX - 1UL;
                bFound = sal_False;

                while( ( nX < nWidth ) && ( nCount < 256L ) && ( cPix = rAcc.GetPixel( nY, nX ) ) != cLast )
                {
                    nX++; nCount++;
                    cLast = cPix;
                    bFound = sal_True;
                }

                if ( bFound )
                    nX--;

                if ( nCount > 3 )
                {
                    *pTmp++ = 0;
                    *pTmp++ = (sal_uInt8) --nCount;

                    if( bRLE4 )
                    {
                        for ( sal_uLong i = 0; i < nCount; i++, pTmp++ )
                        {
                            *pTmp = (sal_uInt8) rAcc.GetPixel( nY, nSaveIndex++ ) << 4;

                            if ( ++i < nCount )
                                *pTmp |= rAcc.GetPixel( nY, nSaveIndex++ );
                        }

                        nCount = ( nCount + 1 ) >> 1;
                    }
                    else
                    {
                        for( sal_uLong i = 0UL; i < nCount; i++ )
                            *pTmp++ = rAcc.GetPixel( nY, nSaveIndex++ );
                    }

                    if ( nCount & 1 )
                    {
                        *pTmp++ = 0;
                        nBufCount += ( nCount + 3 );
                    }
                    else
                        nBufCount += ( nCount + 2 );
                }
                else
                {
                    *pTmp++ = 1;
                    *pTmp++ = (sal_uInt8) rAcc.GetPixel( nY, nSaveIndex ) << ( bRLE4 ? 4 : 0 );

                    if ( nCount == 3 )
                    {
                        *pTmp++ = 1;
                        *pTmp++ = (sal_uInt8) rAcc.GetPixel( nY, ++nSaveIndex ) << ( bRLE4 ? 4 : 0 );
                        nBufCount += 4;
                    }
                    else
                        nBufCount += 2;
                }
            }
        }

        pBuf[ nBufCount++ ] = 0;
        pBuf[ nBufCount++ ] = 0;

        rOStm.Write( pBuf, nBufCount );
    }

    rOStm << (sal_uInt8) 0;
    rOStm << (sal_uInt8) 1;

    delete[] pBuf;

    return( rOStm.GetError() == 0UL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
