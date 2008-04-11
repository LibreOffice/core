/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bitmap2.cxx,v $
 * $Revision: 1.18 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <tools/zcodec.hxx>
#ifndef _TOOLS_STREAM_HXX
#include <tools/stream.hxx>
#endif
#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/outdev.hxx>
#include <vcl/bitmap.hxx>

#include <utility>


// -----------
// - Defines -
// -----------

#define DIBCOREHEADERSIZE           ( 12UL )
#define DIBINFOHEADERSIZE           ( sizeof( DIBInfoHeader ) )
#define SETPIXEL4( pBuf, nX, cChar )( (pBuf)[ (nX) >> 1 ] |= ( (nX) & 1 ) ? ( cChar ): (cChar) << 4 );

// ----------------------
// - Compression defines
// ----------------------

#define COMPRESS_OWN                ('S'|('D'<<8UL))
#define COMPRESS_NONE               ( 0UL )
#define RLE_8                       ( 1UL )
#define RLE_4                       ( 2UL )
#define BITFIELDS                   ( 3UL )
#define ZCOMPRESS                   ( COMPRESS_OWN | 0x01000000UL ) /* == 'SD01' (binary) */

// -----------------
// - DIBInfoHeader -
// -----------------

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
    inline USHORT discretizeBitcount( UINT16 nInputCount )
    {
        return ( nInputCount <= 1 ) ? 1 :
               ( nInputCount <= 4 ) ? 4 :
               ( nInputCount <= 8 ) ? 8 : 24;
    }

    inline bool isBitfieldCompression( ULONG nScanlineFormat )
    {
        return nScanlineFormat == BMP_FORMAT_16BIT_TC_LSB_MASK ||
            nScanlineFormat == BMP_FORMAT_32BIT_TC_MASK;
    }
}

// ----------
// - Bitmap -
// ----------

SvStream& operator>>( SvStream& rIStm, Bitmap& rBitmap )
{
    rBitmap.Read( rIStm, TRUE );
    return rIStm;
}

// ------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Bitmap& rBitmap )
{
    rBitmap.Write( rOStm, FALSE, TRUE );
    return rOStm;
}

// ------------------------------------------------------------------

BOOL Bitmap::Read( SvStream& rIStm, BOOL bFileHeader )
{
    const USHORT    nOldFormat = rIStm.GetNumberFormatInt();
    const ULONG     nOldPos = rIStm.Tell();
    ULONG           nOffset = 0UL;
    BOOL            bRet = FALSE;

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    if( bFileHeader )
    {
        if( ImplReadDIBFileHeader( rIStm, nOffset ) )
            bRet = ImplReadDIB( rIStm, *this, nOffset );
    }
    else
        bRet = ImplReadDIB( rIStm, *this, nOffset );

    if( !bRet )
    {
        if( !rIStm.GetError() )
            rIStm.SetError( SVSTREAM_GENERALERROR );

        rIStm.Seek( nOldPos );
    }

    rIStm.SetNumberFormatInt( nOldFormat );

    return bRet;
}

// ------------------------------------------------------------------

BOOL Bitmap::ImplReadDIB( SvStream& rIStm, Bitmap& rBmp, ULONG nOffset )
{
    DIBInfoHeader   aHeader;
    const ULONG     nStmPos = rIStm.Tell();
    BOOL            bRet = FALSE;
    sal_Bool        bTopDown = sal_False;

    if( ImplReadDIBInfoHeader( rIStm, aHeader, bTopDown ) && aHeader.nWidth && aHeader.nHeight && aHeader.nBitCount )
    {
        const USHORT nBitCount( discretizeBitcount(aHeader.nBitCount) );

        const Size          aSizePixel( aHeader.nWidth, aHeader.nHeight );
        BitmapPalette       aDummyPal;
        Bitmap              aNewBmp( aSizePixel, nBitCount, &aDummyPal );
        BitmapWriteAccess*  pAcc = aNewBmp.AcquireWriteAccess();

        if( pAcc )
        {
            USHORT          nColors;
            SvStream*       pIStm;
            SvMemoryStream* pMemStm = NULL;
            BYTE*           pData = NULL;

            if( nBitCount <= 8 )
            {
                if( aHeader.nColsUsed )
                    nColors = (USHORT) aHeader.nColsUsed;
                else
                    nColors = ( 1 << aHeader.nBitCount );
            }
            else
                nColors = 0;

            if( ZCOMPRESS == aHeader.nCompression )
            {
                ZCodec  aCodec;
                sal_uInt32 nCodedSize, nUncodedSize;
                ULONG   nCodedPos;

                // read coding information
                rIStm >> nCodedSize >> nUncodedSize >> aHeader.nCompression;
                pData = (BYTE*) rtl_allocateMemory( nUncodedSize );

                // decode buffer
                nCodedPos = rIStm.Tell();
                aCodec.BeginCompression();
                aCodec.Read( rIStm, pData, nUncodedSize );
                aCodec.EndCompression();

                // skip unread bytes from coded buffer
                rIStm.SeekRel( nCodedSize - ( rIStm.Tell() - nCodedPos ) );

                // set decoded bytes to memory stream,
                // from which we will read the bitmap data
                pIStm = pMemStm = new SvMemoryStream;
                pMemStm->SetBuffer( (char*) pData, nUncodedSize, FALSE, nUncodedSize );
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
                    aNewBmp.SetPrefSize( Size( aHeader.nWidth, aHeader.nHeight ) );
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

// ------------------------------------------------------------------

BOOL Bitmap::ImplReadDIBFileHeader( SvStream& rIStm, ULONG& rOffset )
{
    UINT32  nTmp32;
    UINT16  nTmp16 = 0;
    BOOL    bRet = FALSE;

    rIStm >> nTmp16;

    if ( ( 0x4D42 == nTmp16 ) || ( 0x4142 == nTmp16 ) )
    {
        if ( 0x4142 == nTmp16 )
        {
            rIStm.SeekRel( 12L );
            rIStm >> nTmp16;
            rIStm.SeekRel( 8L );
            rIStm >> nTmp32;
            rOffset = nTmp32 - 28UL;;
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

// ------------------------------------------------------------------

BOOL Bitmap::ImplReadDIBInfoHeader( SvStream& rIStm, DIBInfoHeader& rHeader, sal_Bool& bTopDown )
{
    // BITMAPINFOHEADER or BITMAPCOREHEADER
    rIStm >> rHeader.nSize;

    // BITMAPCOREHEADER
    if ( rHeader.nSize == DIBCOREHEADERSIZE )
    {
        sal_Int16 nTmp16;

        rIStm >> nTmp16; rHeader.nWidth = nTmp16;
        rIStm >> nTmp16; rHeader.nHeight = nTmp16;
        rIStm >> rHeader.nPlanes;
        rIStm >> rHeader.nBitCount;
    }
    else
    {
        // unknown Header
        if( rHeader.nSize < DIBINFOHEADERSIZE )
        {
            ULONG nUnknownSize = sizeof( rHeader.nSize );

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
            rIStm >> rHeader.nHeight;
            rIStm >> rHeader.nPlanes;
            rIStm >> rHeader.nBitCount;
            rIStm >> rHeader.nCompression;
            rIStm >> rHeader.nSizeImage;
            rIStm >> rHeader.nXPelsPerMeter;
            rIStm >> rHeader.nYPelsPerMeter;
            rIStm >> rHeader.nColsUsed;
            rIStm >> rHeader.nColsImportant;
        }

        // Eventuell bis zur Palette ueberlesen
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

    // #144105# protect a little against damaged files
    if( rHeader.nSizeImage > ( 16 * static_cast< sal_uInt32 >( rHeader.nWidth * rHeader.nHeight ) ) )
        rHeader.nSizeImage = 0;

    return( ( rHeader.nPlanes == 1 ) && ( rIStm.GetError() == 0UL ) );
}

// ------------------------------------------------------------------

BOOL Bitmap::ImplReadDIBPalette( SvStream& rIStm, BitmapWriteAccess& rAcc, BOOL bQuad )
{
    const USHORT    nColors = rAcc.GetPaletteEntryCount();
    const ULONG     nPalSize = nColors * ( bQuad ? 4UL : 3UL );
    BitmapColor     aPalColor;

    BYTE* pEntries = new BYTE[ nPalSize ];
    rIStm.Read( pEntries, nPalSize );

    BYTE* pTmpEntry = pEntries;
    for( USHORT i = 0; i < nColors; i++ )
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

// ------------------------------------------------------------------

BOOL Bitmap::ImplReadDIBBits( SvStream& rIStm, DIBInfoHeader& rHeader, BitmapWriteAccess& rAcc, sal_Bool bTopDown )
{
    const ULONG nAlignedWidth = AlignedWidth4Bytes( rHeader.nWidth * rHeader.nBitCount );
    UINT32      nRMask = 0;
    UINT32      nGMask = 0;
    UINT32      nBMask = 0;
    BOOL        bNative;
    BOOL        bTCMask = ( rHeader.nBitCount == 16 ) || ( rHeader.nBitCount == 32 );
    BOOL        bRLE = ( RLE_8 == rHeader.nCompression && rHeader.nBitCount == 8 ) ||
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
            bNative = FALSE;
        break;
    }
    // Read data
    if( bNative )
    {
        // true color DIB's can have a (optimization) palette
        if( rHeader.nColsUsed && rHeader.nBitCount > 8 )
            rIStm.SeekRel( rHeader.nColsUsed * ( ( rHeader.nSize != DIBCOREHEADERSIZE ) ? 4 : 3 ) );

        rIStm.Read( rAcc.GetBuffer(), rHeader.nHeight * nAlignedWidth );
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
                const ULONG nOldPos = rIStm.Tell();

                rIStm.Seek( STREAM_SEEK_TO_END );
                rHeader.nSizeImage = rIStm.Tell() - nOldPos;
                rIStm.Seek( nOldPos );
            }

            BYTE* pBuffer = (BYTE*) rtl_allocateMemory( rHeader.nSizeImage );

            rIStm.Read( (char*) pBuffer, rHeader.nSizeImage );
            ImplDecodeRLE( pBuffer, rHeader, rAcc, RLE_4 == rHeader.nCompression );

            rtl_freeMemory( pBuffer );
        }
        else
        {
            const long  nWidth = rHeader.nWidth;
            const long  nHeight = rHeader.nHeight;
            BYTE*       pBuf = new BYTE[ nAlignedWidth ];

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
                    BYTE*   pTmp;
                    BYTE    cTmp;

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

                            rAcc.SetPixel( nY, nX, sal::static_int_cast<BYTE>(( cTmp >> --nShift ) & 1) );
                        }
                    }
                }
                break;

                case( 4 ):
                {
                    BYTE*   pTmp;
                    BYTE    cTmp;

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

                            rAcc.SetPixel( nY, nX, sal::static_int_cast<BYTE>(( cTmp >> ( --nShift << 2UL ) ) & 0x0f) );
                        }
                    }
                }
                break;

                case( 8 ):
                {
                    BYTE*   pTmp;

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
                    UINT16*     pTmp16;

                    for( ; nCount--; nY += nI )
                    {
                        rIStm.Read( (char*)( pTmp16 = (UINT16*) pBuf ), nAlignedWidth );

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            aMask.GetColorFor16BitLSB( aColor, (BYTE*) pTmp16++ );
                            rAcc.SetPixel( nY, nX, aColor );
                        }
                    }
                }
                break;

                case( 24 ):
                {
                    BitmapColor aPixelColor;
                    BYTE*       pTmp;

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
                    UINT32*     pTmp32;

                    for( ; nCount--; nY += nI )
                    {
                        rIStm.Read( (char*)( pTmp32 = (UINT32*) pBuf ), nAlignedWidth );

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            aMask.GetColorFor32Bit( aColor, (BYTE*) pTmp32++ );
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

// ------------------------------------------------------------------

BOOL Bitmap::Write( SvStream& rOStm, BOOL bCompressed, BOOL bFileHeader ) const
{
    DBG_ASSERT( mpImpBmp, "Empty Bitmaps can't be saved" );

    const Size  aSizePix( GetSizePixel() );
    BOOL        bRet = FALSE;

    if( mpImpBmp && aSizePix.Width() && aSizePix.Height() )
    {
        BitmapReadAccess*   pAcc = ( (Bitmap*) this)->AcquireReadAccess();
        const USHORT        nOldFormat = rOStm.GetNumberFormatInt();
        const ULONG         nOldPos = rOStm.Tell();

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

// ------------------------------------------------------------------

BOOL Bitmap::ImplWriteDIB( SvStream& rOStm, BitmapReadAccess& rAcc, BOOL bCompressed ) const
{
    const MapMode   aMapPixel( MAP_PIXEL );
    DIBInfoHeader   aHeader;
    ULONG           nImageSizePos;
    ULONG           nEndPos;
    sal_uInt32      nCompression = 0;
    BOOL            bRet = FALSE;

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
        const UINT16 nBitCount( sal::static_int_cast<UINT16>(rAcc.GetBitCount()) );

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
        const Size aSize100( OutputDevice::LogicToLogic( maPrefSize, maPrefMapMode, MAP_100TH_MM ) );

        if( aSize100.Width() && aSize100.Height() )
        {
            aHeader.nXPelsPerMeter = ( rAcc.Width() * 100000UL ) / aSize100.Width();
            aHeader.nYPelsPerMeter = ( rAcc.Height() * 100000UL ) / aSize100.Height();
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
        ULONG           nCodedPos = rOStm.Tell(), nLastPos;
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
        aCodec.Write( rOStm, (BYTE*) aMemStm.GetData(), nUncodedSize );
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

// ------------------------------------------------------------------

BOOL Bitmap::ImplWriteDIBFileHeader( SvStream& rOStm, BitmapReadAccess& rAcc )
{
    UINT32  nPalCount = ( rAcc.HasPalette() ? rAcc.GetPaletteEntryCount() :
                          isBitfieldCompression( rAcc.GetScanlineFormat() ) ? 3UL : 0UL );
    UINT32  nOffset = 14 + DIBINFOHEADERSIZE + nPalCount * 4UL;

    rOStm << (UINT16) 0x4D42;
    rOStm << (UINT32) ( nOffset + ( rAcc.Height() * rAcc.GetScanlineSize() ) );
    rOStm << (UINT16) 0;
    rOStm << (UINT16) 0;
    rOStm << nOffset;

    return( rOStm.GetError() == 0UL );
}

// ------------------------------------------------------------------

BOOL Bitmap::ImplWriteDIBPalette( SvStream& rOStm, BitmapReadAccess& rAcc )
{
    const USHORT    nColors = rAcc.GetPaletteEntryCount();
    const ULONG     nPalSize = nColors * 4UL;
    BYTE*           pEntries = new BYTE[ nPalSize ];
    BYTE*           pTmpEntry = pEntries;
    BitmapColor     aPalColor;

    for( USHORT i = 0; i < nColors; i++ )
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

// ------------------------------------------------------------------

BOOL Bitmap::ImplWriteDIBBits( SvStream& rOStm, BitmapReadAccess& rAcc,
                               ULONG nCompression, sal_uInt32& rImageSize )
{
    if( BITFIELDS == nCompression )
    {
        const ColorMask&    rMask = rAcc.GetColorMask();
        SVBT32              aVal32;

        UInt32ToSVBT32( rMask.GetRedMask(), aVal32 );
        rOStm.Write( (BYTE*) aVal32, 4UL );

        UInt32ToSVBT32( rMask.GetGreenMask(), aVal32 );
        rOStm.Write( (BYTE*) aVal32, 4UL );

        UInt32ToSVBT32( rMask.GetBlueMask(), aVal32 );
        rOStm.Write( (BYTE*) aVal32, 4UL );

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
        const USHORT nBitCount( sal::static_int_cast<USHORT>(rAcc.GetBitCount()) );
        const ULONG  nAlignedWidth = AlignedWidth4Bytes( rAcc.Width() *
                                                         discretizeBitcount(nBitCount));
        BOOL         bNative = FALSE;

        switch( rAcc.GetScanlineFormat() )
        {
            case( BMP_FORMAT_1BIT_MSB_PAL ):
            case( BMP_FORMAT_4BIT_MSN_PAL ):
            case( BMP_FORMAT_8BIT_PAL ):
            case( BMP_FORMAT_24BIT_TC_BGR ):
            {
                if( rAcc.IsBottomUp() && ( rAcc.GetScanlineSize() == nAlignedWidth ) )
                    bNative = TRUE;
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
            BYTE*       pBuf = new BYTE[ nAlignedWidth ];
            BYTE*       pTmp;
            BYTE        cTmp;

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

                            cTmp |= ( (BYTE) rAcc.GetPixel( nY, nX ) << --nShift );
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

                            cTmp |= ( (BYTE) rAcc.GetPixel( nY, nX ) << ( --nShift << 2L ) );
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

// ------------------------------------------------------------------

void Bitmap::ImplDecodeRLE( BYTE* pBuffer, DIBInfoHeader& rHeader,
                            BitmapWriteAccess& rAcc, BOOL bRLE4 )
{
    Scanline    pRLE = pBuffer;
    long        nY = rHeader.nHeight - 1L;
    const ULONG nWidth = rAcc.Width();
    ULONG       nCountByte;
    ULONG       nRunByte;
    ULONG       nX = 0UL;
    BYTE        cTmp;
    BOOL        bEndDecoding = FALSE;

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

                    for( ULONG i = 0UL; i < nCountByte; i++ )
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
                    for( ULONG i = 0UL; i < nRunByte; i++ )
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
                bEndDecoding = TRUE;
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

                for( ULONG i = 0UL; i < nRunByte; i++ )
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
                for( ULONG i = 0UL; ( i < nCountByte ) && ( nX < nWidth ); i++ )
                    rAcc.SetPixel( nY, nX++, cTmp );
            }
        }
    }
    while ( !bEndDecoding && ( nY >= 0L ) );
}

// ------------------------------------------------------------------

BOOL Bitmap::ImplWriteRLE( SvStream& rOStm, BitmapReadAccess& rAcc, BOOL bRLE4 )
{
    const ULONG nWidth = rAcc.Width();
    const ULONG nHeight = rAcc.Height();
    ULONG       nX;
    ULONG       nSaveIndex;
    ULONG       nCount;
    ULONG       nBufCount;
    BYTE*       pBuf = new BYTE[ ( nWidth << 1 ) + 2 ];
    BYTE*       pTmp;
    BYTE        cPix;
    BYTE        cLast;
    BOOL        bFound;

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
                *pTmp++ = (BYTE) nCount;
                *pTmp++ = ( bRLE4 ? ( ( cPix << 4 ) | cPix ) : cPix );
                nBufCount += 2;
            }
            else
            {
                cLast = cPix;
                nSaveIndex = nX - 1UL;
                bFound = FALSE;

                while( ( nX < nWidth ) && ( nCount < 256L ) && ( cPix = rAcc.GetPixel( nY, nX ) ) != cLast )
                {
                    nX++; nCount++;
                    cLast = cPix;
                    bFound = TRUE;
                }

                if ( bFound )
                    nX--;

                if ( nCount > 3 )
                {
                    *pTmp++ = 0;
                    *pTmp++ = (BYTE) --nCount;

                    if( bRLE4 )
                    {
                        for ( ULONG i = 0; i < nCount; i++, pTmp++ )
                        {
                            *pTmp = (BYTE) rAcc.GetPixel( nY, nSaveIndex++ ) << 4;

                            if ( ++i < nCount )
                                *pTmp |= rAcc.GetPixel( nY, nSaveIndex++ );
                        }

                        nCount = ( nCount + 1 ) >> 1;
                    }
                    else
                    {
                        for( ULONG i = 0UL; i < nCount; i++ )
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
                    *pTmp++ = (BYTE) rAcc.GetPixel( nY, nSaveIndex ) << ( bRLE4 ? 4 : 0 );

                    if ( nCount == 3 )
                    {
                        *pTmp++ = 1;
                        *pTmp++ = (BYTE) rAcc.GetPixel( nY, ++nSaveIndex ) << ( bRLE4 ? 4 : 0 );
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

    rOStm << (BYTE) 0;
    rOStm << (BYTE) 1;

    delete[] pBuf;

    return( rOStm.GetError() == 0UL );
}
