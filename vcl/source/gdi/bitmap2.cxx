/*************************************************************************
 *
 *  $RCSfile: bitmap2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef WIN
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#endif

#define _SV_BITMAP_CXX

#include <tools/zcodec.hxx>
#ifndef _TOOLS_NEW_HXX
#include <tools/new.hxx>
#endif
#ifndef _TOOLS_STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif

#define USE_ZCODEC

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
    UINT32          nSize;
    UINT32          nWidth;
    UINT32          nHeight;
    UINT16          nPlanes;
    UINT16          nBitCount;
    UINT32          nCompression;
    UINT32          nSizeImage;
    UINT32          nXPelsPerMeter;
    UINT32          nYPelsPerMeter;
    UINT32          nColsUsed;
    UINT32          nColsImportant;

                    DIBInfoHeader() :
                        nSize( 0UL ),
                        nWidth( 0UL ),
                        nHeight( 0UL ),
                        nPlanes( 0 ),
                        nBitCount( 0 ),
                        nCompression( 0UL ),
                        nSizeImage( 0UL ),
                        nXPelsPerMeter( 0UL ),
                        nYPelsPerMeter( 0UL ),
                        nColsUsed( 0UL ),
                        nColsImportant( 0UL ) {}

                    ~DIBInfoHeader() {}
};

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
    BitmapPalette*  pPalette = NULL;
    BOOL            bHeaderRead = FALSE;
    BOOL            bRet = FALSE;

    if( ImplReadDIBInfoHeader( rIStm, aHeader ) && aHeader.nWidth && aHeader.nHeight && aHeader.nBitCount )
    {
        USHORT nBitCount = ( aHeader.nBitCount <= 1 ) ? 1 :
                           ( aHeader.nBitCount <= 4 ) ? 4 :
                           ( aHeader.nBitCount <= 8 ) ? 8 : 24;

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

#ifdef USE_ZCODEC
            if( ZCOMPRESS == aHeader.nCompression )
            {
                ZCodec  aCodec;
                ULONG   nCodedSize, nUncodedSize;
                ULONG   nCodedPos;

                // read coding information
                rIStm >> nCodedSize >> nUncodedSize >> aHeader.nCompression;
                pData = (BYTE*) SvMemAlloc( nUncodedSize );

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
#endif // USE_ZCODEC
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

                bRet = ImplReadDIBBits( *pIStm, aHeader, *pAcc );

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
                SvMemFree( pData );

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
    UINT16  nTmp16;
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

BOOL Bitmap::ImplReadDIBInfoHeader( SvStream& rIStm, DIBInfoHeader& rHeader )
{
    // BITMAPINFOHEADER or BITMAPCOREHEADER
    rIStm >> rHeader.nSize;

    // BITMAPCOREHEADER
    if ( rHeader.nSize == DIBCOREHEADERSIZE )
    {
        UINT16  nTmp16;

        rIStm >> nTmp16; rHeader.nWidth = nTmp16;
        rIStm >> nTmp16; rHeader.nHeight = nTmp16;
        rIStm >> nTmp16; rHeader.nPlanes = nTmp16;
        rIStm >> nTmp16; rHeader.nBitCount = nTmp16;
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

BOOL Bitmap::ImplReadDIBBits( SvStream& rIStm, DIBInfoHeader& rHeader, BitmapWriteAccess& rAcc )
{
    const ULONG nAlignedWidth = AlignedWidth4Bytes( rHeader.nWidth * rHeader.nBitCount );
    UINT32      nRMask;
    UINT32      nGMask;
    UINT32      nBMask;
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
            bNative = ( rAcc.IsBottomUp() && !bRLE && !bTCMask && ( rAcc.GetScanlineSize() == nAlignedWidth ) );
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

            BYTE* pBuffer = (BYTE*) SvMemAlloc( rHeader.nSizeImage );

            rIStm.Read( (char*) pBuffer, rHeader.nSizeImage );
            ImplDecodeRLE( pBuffer, rHeader, rAcc, RLE_4 == rHeader.nCompression );

            SvMemFree( pBuffer );
        }
        else
        {
            const long  nWidth = rHeader.nWidth;
            const long  nHeight = rHeader.nHeight;
            BYTE*       pBuf = new BYTE[ nAlignedWidth ];

            // true color DIB's can have a (optimization) palette
            if( rHeader.nColsUsed && rHeader.nBitCount > 8 )
                rIStm.SeekRel( rHeader.nColsUsed * ( ( rHeader.nSize != DIBCOREHEADERSIZE ) ? 4 : 3 ) );

            switch( rHeader.nBitCount )
            {
                case( 1 ):
                {
                    BYTE*   pTmp;
                    BYTE    cTmp;

                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
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

                            rAcc.SetPixel( nY, nX, ( cTmp >> --nShift ) & 1 );
                        }
                    }
                }
                break;

                case( 4 ):
                {
                    BYTE*   pTmp;
                    BYTE    cTmp;

                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
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

                            rAcc.SetPixel( nY, nX, ( cTmp >> ( --nShift << 2UL ) ) & 0x0f );
                        }
                    }
                }
                break;

                case( 8 ):
                {
                    BYTE*   pTmp;

                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
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

                    for( long nY = rHeader.nHeight - 1L; nY >= 0L; nY-- )
                    {
                        rIStm.Read( (char*)( pTmp16 = (UINT16*) pBuf ), nAlignedWidth );

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            aMask.GetColorFor16Bit( aColor, (BYTE*) pTmp16++ );
                            rAcc.SetPixel( nY, nX, aColor );
                        }
                    }
                }
                break;

                case( 24 ):
                {
                    BitmapColor aPixelColor;
                    BYTE*       pTmp;

                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
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

                    for( long nY = rHeader.nHeight - 1L; nY >= 0L; nY-- )
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
    BMP_ASSERT( mpImpBmp, "Empty Bitmaps can't be saved" );

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
    DIBInfoHeader   aHeader;
    ULONG           nImageSizePos;
    ULONG           nEndPos;
    ULONG           nCompression = 0UL;
    BOOL            bRet = FALSE;

    aHeader.nSize = DIBINFOHEADERSIZE;
    aHeader.nWidth = rAcc.Width();
    aHeader.nHeight = rAcc.Height();
    aHeader.nPlanes = 1;

    switch( rAcc.GetScanlineFormat() )
    {
        case( BMP_FORMAT_16BIT_TC_MASK ):
        case( BMP_FORMAT_32BIT_TC_MASK ):
        {
            aHeader.nBitCount = ( rAcc.GetScanlineFormat() == BMP_FORMAT_16BIT_TC_MASK ) ? 16 : 32;
            nCompression = BITFIELDS;
        }
        break;

        default:
        {
            aHeader.nBitCount = rAcc.GetBitCount();

            if( bCompressed )
            {
                if( 4 == aHeader.nBitCount )
                    nCompression = RLE_4;
                else if( 8 == aHeader.nBitCount )
                    nCompression = RLE_8;
            }
            else
                nCompression = COMPRESS_NONE;
        }
        break;
    }

#ifdef USE_ZCODEC
    if( ( rOStm.GetCompressMode() & COMPRESSMODE_ZBITMAP ) &&
        ( rOStm.GetVersion() >= SOFFICE_FILEFORMAT_40 ) )
    {
        aHeader.nCompression = ZCOMPRESS;
    }
    else
#endif // USE_ZCODEC
        aHeader.nCompression = nCompression;

    aHeader.nSizeImage = rAcc.Height() * rAcc.GetScanlineSize();

    if( maPrefSize.Width() && maPrefSize.Height() )
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

#ifdef USE_ZCODEC
    if( aHeader.nCompression == ZCOMPRESS )
    {
        ZCodec          aCodec;
        SvMemoryStream  aMemStm( aHeader.nSizeImage + 4096, 65535 );
        ULONG           nCodedPos = rOStm.Tell(), nLastPos;
        ULONG           nCodedSize, nUncodedSize;

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
#endif // USE_ZCODEC
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
    UINT32  nPalCount = ( rAcc.HasPalette() ? rAcc.GetPaletteEntryCount() : rAcc.HasColorMask() ? 3UL : 0UL );
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
                               ULONG nCompression, ULONG& rImageSize )
{
    if( BITFIELDS == nCompression )
    {
        const ColorMask&    rMask = rAcc.GetColorMask();
        SVBT32              aVal32;

        LongToSVBT32( rMask.GetRedMask(), aVal32 );
        rOStm.Write( (BYTE*) aVal32, 4UL );

        LongToSVBT32( rMask.GetGreenMask(), aVal32 );
        rOStm.Write( (BYTE*) aVal32, 4UL );

        LongToSVBT32( rMask.GetBlueMask(), aVal32 );
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
        const ULONG nAlignedWidth = AlignedWidth4Bytes( rAcc.Width() * rAcc.GetBitCount() );
        BOOL        bNative = FALSE;

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

            switch( rAcc.GetBitCount() )
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

                default:
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
        if( !( nCountByte = *pRLE++ ) )
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
    BYTE*       pBuf = new BYTE[ nWidth << 1 ];
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
