/*************************************************************************
 *
 *  $RCSfile: pngread.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-06-24 10:16:51 $
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

#include "pngread.hxx"

#include <cmath>
#include <rtl/crc.h>
#include <rtl/memory.h>
#include <rtl/alloc.h>
#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include "bmpacc.hxx"
#include "svapp.hxx"
#include "alpha.hxx"

// -----------
// - Defines -
// -----------

#define CHUNK_IS_OPEN       1
#define CHUNK_IS_CLOSED     2
#define CHUNK_IS_IN_USE     4

#define PNG_TRANS_VAL       0x1

#define VIEWING_GAMMA       2.5
#define DISPLAY_GAMMA       1.25

#define PNGCHUNK_IHDR       0x49484452
#define PNGCHUNK_PLTE       0x504c5445
#define PNGCHUNK_IDAT       0x49444154
#define PNGCHUNK_IEND       0x49454e44
#define PNGCHUNK_bKGD       0x624b4744
#define PNGCHUNK_cHRM       0x6348524d
#define PNGCHUNK_gAMA       0x67414d41
#define PNGCHUNK_hIST       0x68495354
#define PNGCHUNK_pHYs       0x70485973
#define PNGCHUNK_sBIT       0x73425420
#define PNGCHUNK_tIME       0x74494d45
#define PNGCHUNK_tEXt       0x74455874
#define PNGCHUNK_tRNS       0x74524e53
#define PNGCHUNK_zTXt       0x7a545874
#define PMGCHUNG_msOG       0x6d734f47      // Microsoft Office Animated GIF

namespace vcl
{
// -----------
// - statics -
// -----------

static const BYTE aBlockHeight[ 8 ] =
{
    0, 8, 8, 4, 4, 2, 2, 1
};

// ------------------------------------------------------------------------------

static const BYTE aBlockWidth[ 8 ]  =
{
    0, 8, 4, 4, 2, 2, 1, 1
};

// ------------------------------------------------------------------------------

static const BYTE mpDefaultColorTable[ 256 ] =
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
    SvStream*           mpIStm;
    Bitmap*             mpBmp;
    BitmapWriteAccess*  mpAcc;
    Bitmap*             mpMaskBmp;
    AlphaMask*          mpAlphaMask;
    BitmapWriteAccess*  mpMaskAcc;
    ZCodec*             mpZCodec;
    BYTE*               mpInflateInBuf; // as big as the size of a scanline + alphachannel + 1
    BYTE*               mpScanprior;    // pointer to the latest scanline
    BYTE*               mpTransTab;     //
    BYTE*               mpScan;         // pointer in the current scanline
    BYTE*               mpColorTable;   //
    UINT32              mnChunkStatus;
    UINT32              mnChunkStartPosition;
    UINT32              mnIDATCRCCount;
    sal_uInt32          mnChunkType;    // Chunk which is currently open
    sal_uInt32          mnCRC;
    sal_Int32           mnChunkDatSizeOrg;
    sal_Int32           mnChunkDatSize;
    sal_uInt32          mnWidth;
    sal_uInt32          mnHeight;
    sal_uInt32          mnBBP;          // number of bytes per pixel
    sal_uInt32          mnScansize;     // max size of scanline
    sal_uInt32          mnPrefWidth;    // preferred width in meter
    sal_uInt32          mnPrefHeight;   // preferred Height in meter
    sal_uInt32          mnYpos;         // latest y position;
    USHORT              mnIStmOldMode;
    USHORT              mnDepth;        // pixel depth
    BYTE                mnTransRed;
    BYTE                mnTransGreen;
    BYTE                mnTransBlue;
    BYTE                mnDummy;
    BYTE                mnBitDepth;     // sample depth
    BYTE                mnColorType;
    BYTE                mnCompressionType;
    BYTE                mnFilterType;
    BYTE                mnInterlaceType;
    BYTE                mnPass;         // if interlaced the latest pass ( 1..7 ) else 7
    BYTE                cTransIndex1;
    BYTE                cNonTransIndex1;
    BOOL                mbStatus;
    BOOL                mbFinished;
    BOOL                mbFirstEntry;
    BOOL                mbTransparent;  // graphic includes an tRNS Chunk or an alpha Channel
    BOOL                mbAlphaChannel;
    BOOL                mbRGBTriple;
    BOOL                mbPalette;      // FALSE if we need a Palette
    BOOL                mbGrayScale;
    BOOL                mbzCodecInUse;
    BOOL                mbIDAT;         // TRUE if finished with the complete IDAT...
    BOOL                mbGamma;        // TRUE if Gamma Correction available
    BOOL                mbpHYs;         // TRUE if pysical size of pixel available


    void                ImplSetPixel( sal_uInt32 y, sal_uInt32 x, const BitmapColor &, BOOL bTrans );
    void                ImplSetPixel( sal_uInt32 y, sal_uInt32 x, BYTE nPalIndex, BOOL bTrans );
    void                ImplSetAlphaPixel( sal_uInt32 y, sal_uInt32 x, const BitmapColor&, BYTE nAlpha );
    void                ImplReadIDAT();
    void                ImplResizeScanline();
    void                ImplGetFilter( sal_uInt32 nXStart=0, sal_uInt32 nXAdd=1 );
    void                ImplReadTransparent();
    void                ImplGetGamma();
    void                ImplGetBackground();
    BYTE                ImplScaleColor();
    BOOL                ImplReadHeader();
    BOOL                ImplReadPalette();
    void                ImplGetGrayPalette( sal_uInt32 );
    void                ImplOpenChunk();
    BYTE                ImplReadBYTE();
    sal_uInt32          ImplReadsal_uInt32();
    void                ImplReadDAT( unsigned char* pSource, long nDatSize );
    BOOL                ImplCloseChunk();
    void                ImplSkipChunk();

public:

                        PNGReaderImpl( SvStream& rStm );
                        ~PNGReaderImpl();

    BitmapEx            Read();
};

// ------------------------------------------------------------------------------

PNGReaderImpl::PNGReaderImpl( SvStream& rPNG ) :
    mnChunkStatus   ( CHUNK_IS_CLOSED ),
    mbStatus        ( TRUE ),
    mbFirstEntry    ( TRUE ),
    mpIStm          ( &rPNG ),
    mpAcc           ( NULL ),
    mpMaskAcc       ( NULL ),
    mpInflateInBuf  ( NULL ),
    mpScanprior     ( NULL ),
    mpTransTab      ( NULL ),
    mpBmp           ( NULL ),
    mpMaskBmp       ( NULL ),
    mpAlphaMask     ( NULL ),
    mnIDATCRCCount  ( 0 ),
    mbGamma         ( sal_False ),
    mbzCodecInUse   ( sal_False ),
    mbpHYs          ( sal_False ),
    mpColorTable    ( (sal_uInt8*) mpDefaultColorTable ),
    mpZCodec        ( new ZCodec( DEFAULT_IN_BUFSIZE, DEFAULT_OUT_BUFSIZE, MAX_MEM_USAGE ) )
{
}

// ------------------------------------------------------------------------

PNGReaderImpl::~PNGReaderImpl()
{
    if ( mbzCodecInUse )
        mpZCodec->EndCompression();

    if( mpColorTable != mpDefaultColorTable )
        delete[] mpColorTable;

    rtl_freeMemory( mpScanprior );

    delete mpBmp;
    delete mpAlphaMask;
    delete mpMaskBmp;
    delete[] mpTransTab;
    delete[] mpInflateInBuf;
    delete mpZCodec;
}

// ------------------------------------------------------------------------

BitmapEx PNGReaderImpl::Read()
{
    BitmapEx    aRet;
    BYTE        nDummy;

    mnIStmOldMode = mpIStm->GetNumberFormatInt();
    mpIStm->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    while( mbStatus )
    {
        if ( mbFirstEntry )
        {
            if ( !ImplReadHeader() )
            {
                mbStatus = FALSE;
                break;
            }

            mbFirstEntry = FALSE;
        }
        else
        {
            if ( ( mpBmp == NULL ) || ( ( mpMaskBmp == NULL ) && ( mpAlphaMask == NULL ) ) )
                mbStatus = FALSE;
            else
            {
                mpAcc = mpBmp->AcquireWriteAccess();

                if ( !mpAcc )
                    mbStatus = FALSE;

                if ( mbAlphaChannel )
                    mpMaskAcc = mpAlphaMask->AcquireWriteAccess();
                else
                    mpMaskAcc = mpMaskBmp->AcquireWriteAccess();

                if ( !mpMaskAcc )
                    mbStatus = FALSE;
            }
        }

        while( mbStatus && !mbFinished )
        {
            if( mpIStm->GetError() != ERRCODE_NONE )
            {
                mbStatus = FALSE;
                break;
            }

            if ( mpIStm->IsEof() )
                break;

            if ( mnChunkStatus == CHUNK_IS_CLOSED )
                ImplOpenChunk();

            switch ( mnChunkType )
            {
                case PNGCHUNK_gAMA :                                // the gamma chunk must precede
                {
                    if ( mbIDAT == FALSE )                          // the 'IDAT' and also the
                        ImplGetGamma();                             // 'PLTE'(if available )
                }
                break;

                case PNGCHUNK_PLTE :
                {
                    if ( mbPalette )
                        ImplSkipChunk();
                    else
                        mbStatus = ImplReadPalette();
                }
                break;

                case PNGCHUNK_tRNS :
                    ImplReadTransparent();
                break;

                case PNGCHUNK_bKGD :                                // the background chunk must appear
                {
                    if ( ( mbIDAT == FALSE ) && mbPalette )         // before the 'IDAT' and after the
                        ImplGetBackground();                        // PLTE(if available ) chunk.
                }
                break;

                case PNGCHUNK_IDAT :
                {
                    if ( mbIDAT )       // the gfx is finished, but there may be left a zlibCRC of about 4Bytes
                    {
                        ImplSkipChunk();
                        break;
                    }
                    else
                    {
                        ImplReadIDAT();

                        if ( ( mpIStm->Tell() - mnChunkDatSizeOrg ) == mnChunkStartPosition )
                        {
                            mnChunkStatus = CHUNK_IS_OPEN;
                            mnChunkDatSize = mnChunkDatSizeOrg;
                        }
                        else
                            mnChunkStatus = CHUNK_IS_IN_USE;
                    }
                }
                break;

                case PNGCHUNK_pHYs :
                {
                    if ( !mbIDAT && mnChunkDatSizeOrg == 9 )
                    {
                        mnPrefWidth = ImplReadsal_uInt32();
                        mnPrefHeight= ImplReadsal_uInt32();

                        sal_uInt8 nUnitSpecifier = ImplReadBYTE();

                        if ( nUnitSpecifier == 1 )
                            mbpHYs = sal_True;
                    }
                }
                break;

                case PNGCHUNK_IEND :
                    mbFinished = TRUE;
                break;

                default:
                    ImplSkipChunk();
                break;
            }

            if ( mbStatus && ( mnChunkStatus == CHUNK_IS_OPEN ) )
                mbStatus = ImplCloseChunk();
        }

        if ( mpAcc )
            mpBmp->ReleaseAccess( mpAcc ), mpAcc = NULL;

        if ( mpMaskAcc )
        {
            if ( mbAlphaChannel )
                mpAlphaMask->ReleaseAccess( mpMaskAcc );
            else
                mpMaskBmp->ReleaseAccess( mpMaskAcc );

            mpMaskAcc = NULL;
        }

        if ( mbStatus )
        {
            if ( ( mbFinished == FALSE ) || mbAlphaChannel || mbTransparent )
            {
                if ( mbAlphaChannel )
                    aRet = BitmapEx( *mpBmp, *mpAlphaMask );
                else
                    aRet = BitmapEx( *mpBmp, *mpMaskBmp );
            }
            else
                aRet = *mpBmp;

            if ( mbpHYs && mnPrefWidth && mnPrefHeight )
            {
                sal_Int32 nPrefSizeX = (sal_Int32)( 100000.0 * ( (double)mnWidth / mnPrefWidth ) );
                sal_Int32 nPrefSizeY = (sal_Int32)( 100000.0 * ( (double)mnHeight / mnPrefHeight ) );

                aRet.SetPrefMapMode( MAP_100TH_MM );
                aRet.SetPrefSize( Size( nPrefSizeX, nPrefSizeY ) );
            }
        }
        break;
    }

    if( !mbStatus )
        aRet.Clear();

    mpIStm->SetNumberFormatInt( mnIStmOldMode );

    return aRet;
}

// ------------------------------------------------------------------------

BOOL PNGReaderImpl::ImplReadHeader()
{
    sal_uInt32 nDummy[ 2 ];

    *mpIStm >> nDummy[0] >> nDummy[1];

    if( ! ( nDummy[0] == 0x89504e47 ) && ( nDummy[1] == 0x0d0a1a0a ) )
        return FALSE;

    ImplOpenChunk();

    // IHDR Cunk must appear first !
    if ( mnChunkType != PNGCHUNK_IHDR )
        return FALSE;

    mnWidth = ImplReadsal_uInt32();
    mnHeight = ImplReadsal_uInt32();

    if ( ( mnWidth == 0 ) || ( mnHeight == 0 ) )
        return FALSE;

    mnBitDepth = ImplReadBYTE();
    mnColorType = ImplReadBYTE();

    if ( mnCompressionType = ImplReadBYTE() )
        return FALSE;

    if ( mnFilterType = ImplReadBYTE() )
        return FALSE;

    switch ( mnInterlaceType = ImplReadBYTE() ) // filter type valid ?
    {
        case 0 :
            mnPass = 7;
            break;
        case 1 :
            mnPass = 0;
            break;
        default:
            return FALSE;
    }

    if ( !ImplCloseChunk() )
        return FALSE;

    mnYpos = 0;
    mbPalette = TRUE;
    mbFinished = mbIDAT = mbAlphaChannel = mbTransparent = FALSE;
    mbGrayScale = mbRGBTriple = FALSE;
    mnDepth = mnBitDepth;
    mnScansize = ( ( mnWidth * mnBitDepth ) + 7 ) >> 3;

    // valid color types are 0,2,3,4 & 6
    switch ( mnColorType )
    {
        case 0 :    // each pixel is a grayscale
        {
            switch ( mnBitDepth )
            {
                case 2 :
                    mnDepth = 8;    // we have to expand the bitmap
                case 16 :
                    mnDepth >>= 1;  // we have to reduce the bitmap
                case 1 :
                case 4 :
                case 8 :
                    mbGrayScale = TRUE;
                    break;
                default :
                    return FALSE;
            }
        }
        break;

        case 2 :    // each pixel is an RGB triple
        {
            mbRGBTriple = TRUE;
            mnScansize*=3;
            switch ( mnBitDepth )
            {
                case 16 :           // we have to reduce the bitmap
                case 8 :
                    mnDepth = 24;
                    break;
                default :
                    return FALSE;
            }
        }
        break;

        case 3 :    // each pixel is a palette index
        {
            switch ( mnBitDepth )
            {
                case 2 :
                    mnDepth = 4;    // we have to expand the bitmap
                case 1 :
                case 4 :
                case 8 :
                    mbPalette = FALSE;
                    break;
                default :
                    return FALSE;
            }
        }
        break;

        case 4 :    // each pixel is a grayscale sample followed by an alpha sample
        {
            mnScansize <<= 1;
            mbAlphaChannel = TRUE;
            switch ( mnBitDepth )
            {
                case 16 :
                    mnDepth >>= 1;  // we have to reduce the bitmap
                case 8 :
                    mbGrayScale = TRUE;
                    break;
                default :
                    return FALSE;
            }
        }
        break;

        case 6 :    // each pixel is an RGB triple followed by an alpha sample
        {
            mbRGBTriple = TRUE;
            mnScansize*=4;
            mbAlphaChannel = TRUE;
            switch (mnBitDepth )
            {
                case 16 :           // we have to reduce the bitmap
                case 8 :
                    mnDepth = 24;
                    break;
                default :
                    return FALSE;
            }
        }
        break;

        default :
            return FALSE;
    }
    mnBBP = mnScansize / mnWidth;

    if ( !mnBBP )
        mnBBP = 1;

    mnScansize++;       // each scanline includes one filterbyte
    mpScan = mpInflateInBuf = new BYTE[ mnScansize ];
    mpScanprior = (sal_uInt8*)rtl_allocateZeroMemory( mnScansize );

    if ( mnInterlaceType )
        ImplResizeScanline();

    mpBmp = new Bitmap( Size( mnWidth, mnHeight ), mnDepth);
    mpAcc = mpBmp->AcquireWriteAccess();

    if ( mbAlphaChannel )
    {
        mpAlphaMask = new AlphaMask( Size( mnWidth, mnHeight ) );
        mpAlphaMask->Erase( 128 );
        mpMaskAcc = mpAlphaMask->AcquireWriteAccess();
    }
    else
    {
        mpMaskBmp = new Bitmap( Size( mnWidth, mnHeight ), 1 );
        mpMaskBmp->Erase( Color( COL_WHITE ) );
        mpMaskAcc = mpMaskBmp->AcquireWriteAccess();
    }

    if ( ( mpAcc == NULL ) || ( mpMaskAcc == NULL ) )
        return FALSE;

    const Color aWhite( COL_WHITE );
    cTransIndex1 = (BYTE)mpMaskAcc->GetBestPaletteIndex( aWhite );
    cNonTransIndex1 = cTransIndex1 ? 0 : 1;

    if ( mbGrayScale )
        ImplGetGrayPalette( mnDepth );

    return TRUE;
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplGetGrayPalette( sal_uInt32 nDepth )
{
    sal_uInt32 nAdd, nStart = 0;

    mpAcc->SetPaletteEntryCount( 1 << nDepth );

    switch ( nDepth )
    {
        case 1 : nAdd = 255; break;
        case 2 : nAdd = 85; break;
        case 4 : nAdd = 17; break;
        case 8 : nAdd = 1; break;
    }

    for ( sal_uInt32 i = 0; nStart < 256; i++, nStart += nAdd )
        mpAcc->SetPaletteColor( (USHORT)i, BitmapColor( mpColorTable[ nStart ],
            mpColorTable[ nStart ], mpColorTable[ nStart ] ) );
}

// ------------------------------------------------------------------------

BOOL PNGReaderImpl::ImplReadPalette()
{
    sal_uInt32 nCount = mnChunkDatSizeOrg / 3;

    if ( ( ( mnChunkDatSizeOrg % 3 ) == 0 ) && ( ( 0 < nCount ) && ( nCount <= 256 ) ) && mpAcc )
    {
        BYTE*   pTempBuf = new BYTE[ mnChunkDatSizeOrg ];
        BYTE*   pTmp = pTempBuf;
        BYTE    nRed, nGreen, nBlue;

        mbPalette = TRUE;
        mpAcc->SetPaletteEntryCount( (USHORT) nCount );
        ImplReadDAT( pTempBuf, mnChunkDatSizeOrg );

        for ( USHORT i = 0; i < nCount; i++ )
        {
            nRed = mpColorTable[ *pTmp++ ];
            nGreen = mpColorTable[ *pTmp++ ];
            nBlue = mpColorTable[ *pTmp++ ];
            mpAcc->SetPaletteColor( i, Color( nRed, nGreen, nBlue ) );
        }

        delete[] pTempBuf;
    }
    else
        mbStatus = FALSE;

    return mbStatus;
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplReadTransparent()
{
    if ( mpTransTab == NULL )
    {
        switch ( mnColorType )
        {
            case 0 :
            {
                if ( mnChunkDatSizeOrg != 2 )
                    ImplSkipChunk();
                else
                {
                    mpTransTab = new sal_uInt8[ 256 ];
                    rtl_fillMemory( mpTransTab, 256, 0xff );
                    // color type 0 and 4 is always greyscale,
                    // so the return value can be used as index
                    sal_uInt8 nIndex = ImplScaleColor();
                    mpTransTab[ nIndex ] = 0;
                    mbTransparent = sal_True;
                }
            }
            break;

            case 2 :
            {
                if ( mnChunkDatSizeOrg != 6 )
                    ImplSkipChunk();
                else
                {
                    mnTransRed = ImplScaleColor();
                    mnTransGreen = ImplScaleColor();
                    mnTransBlue = ImplScaleColor();
                }
            }
            break;

            case 3 :
            {
                if ( mnChunkDatSizeOrg > 256 )
                    ImplSkipChunk();
                else
                {
                    mpTransTab = new BYTE [ 256 ];
                    rtl_fillMemory(mpTransTab, 256, 0xff );
                    ImplReadDAT( mpTransTab, mnChunkDatSizeOrg );
                    mbTransparent = TRUE;
                }
            }
            break;

            default :
                ImplSkipChunk();
            break;
        }
    }
    else
        ImplSkipChunk();
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplGetGamma()
{
    sal_uInt32  nGammaValue = ImplReadsal_uInt32();
    double      fGamma = ( ( VIEWING_GAMMA / DISPLAY_GAMMA ) * ( (double)nGammaValue / 100000 ) );
    double      fInvGamma = ( fGamma <= 0.0 || fGamma > 10.0 ) ? 1.0 : ( 1.0 / fGamma );

    if ( fInvGamma != 1.0 )
    {
        mbGamma = TRUE;

        if ( mpColorTable == mpDefaultColorTable )
            mpColorTable = new sal_uInt8[ 256 ];

        for ( sal_Int32 i = 0; i < 256; i++ )
            mpColorTable[ i ] = (sal_uInt8)(pow((double)i/255.0, fInvGamma) * 255.0 + 0.5);

        if ( mbGrayScale )
            ImplGetGrayPalette( mnDepth );
    }
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplGetBackground()
{
    Point       aPoint;
    Rectangle   aRectangle( aPoint, Size( mnWidth, mnHeight ) );

    switch ( mnColorType )
    {
        case 3 :
        {
            if ( mnChunkDatSizeOrg == 1 )
            {
                UINT16 nCol = ImplReadBYTE() ;
                if ( nCol < mpAcc->GetPaletteEntryCount() )
                {
                    mpAcc->SetFillColor( (const Color&)mpAcc->GetPaletteColor( (BYTE)nCol ) );
                    mpAcc->FillRect( aRectangle );
                    break;
                }
            }
            ImplSkipChunk();
        }
        break;

        case 0 :
        case 4 :
        {
            if ( mnChunkDatSizeOrg == 2 )
            {
                // the color type 0 and 4 is always greyscale,
                // so the return value can be used as index
                sal_uInt8 nIndex = ImplScaleColor();
                mpAcc->SetFillColor( (const Color&)mpAcc->GetPaletteColor( nIndex ) );
                mpAcc->FillRect( aRectangle );
            }
            else
                ImplSkipChunk();
        }
        break;

        case 2 :
        case 6 :
        {
            if ( mnChunkDatSizeOrg == 6 )
            {
                sal_uInt8 nRed = ImplScaleColor();
                sal_uInt8 nGreen = ImplScaleColor();
                sal_uInt8 nBlue = ImplScaleColor();
                mpAcc->SetFillColor( Color( nRed, nGreen, nBlue ) );
                mpAcc->FillRect( aRectangle );
            }
            else
                ImplSkipChunk();
        }
        break;

        default :
            ImplSkipChunk();
        break;
    }

}

// ------------------------------------------------------------------------

// for color type 0 and 4 (greyscale) the return value is always index to the color
//                2 and 6 (RGB)       the return value is always the 8 bit color component
sal_uInt8 PNGReaderImpl::ImplScaleColor()
{
    sal_uInt32 nMask = ( ( 1 << mnBitDepth ) - 1 );
    sal_uInt16 nCol = ( ImplReadBYTE() << 8 );

    nCol += ImplReadBYTE() & (sal_uInt16)nMask;

    if ( mnBitDepth > 8 )   // convert 16bit graphics to 8
        nCol >>= 8;

    return (sal_uInt8) nCol;
}

// ------------------------------------------------------------------------
// ImplReadIDAT reads as much bitmap data as possible

void PNGReaderImpl::ImplReadIDAT()
{
    sal_uInt32 nToRead, nRead;

    if ( mnChunkDatSizeOrg )                // Chunk empty ?
    {
        if ( mbzCodecInUse == FALSE )
        {
            mbzCodecInUse = TRUE;
            mpZCodec->BeginCompression( ZCODEC_PNG_DEFAULT );
        }

        mpZCodec->SetCRC( mnCRC );

        if ( mnChunkStartPosition == mpIStm->Tell() )
            mpZCodec->SetBreak( mnChunkDatSizeOrg );

        while ( ( mpZCodec->GetBreak() ) )
        {
            // get bytes needed to fill the current scanline
            nToRead = mnScansize - ( mpScan - mpInflateInBuf );

            if ( ( nRead = mpZCodec->ReadAsynchron( *mpIStm, mpScan, nToRead ) ) < 0 )
            {
                mbStatus = FALSE;
                break;
            }

            if ( nRead < nToRead )
            {
                mpScan += nRead;            // ZStream is Broken
                break;
            }
            else
            {
                mpScan = mpInflateInBuf;    // this scanline is Finished

                if ( mnInterlaceType == 0   )
                {
                    ImplGetFilter ( 0, 1 );
                    mnYpos++;
                }
                else
                {
                    // interlace mode
                    switch ( mnPass )
                    {
                        case 1 :                                                                            // pass 1
                        {
                            ImplGetFilter ( 0, 8 );
                            mnYpos += 8;
                        }
                        break;

                        case 2 :                                                                            // pass 2
                        {
                            ImplGetFilter ( 4, 8 );
                            mnYpos += 8;
                        }
                        break;

                        case 3 :                                                                            // pass 3
                        {
                            if ( mnYpos >= 4 )
                            {
                                ImplGetFilter ( 0, 4 );
                            }
                            mnYpos += 8;
                        }
                        break;

                        case 4 :                                                                            // pass 4
                        {
                            ImplGetFilter ( 2, 4 );
                            mnYpos += 4;
                        }
                        break;

                        case 5 :                                                                            // pass 5
                        {
                            if ( mnYpos >= 2 )
                            {
                                ImplGetFilter ( 0, 2 );
                            }
                            mnYpos += 4;
                        }
                        break;

                        case 6 :                                                                            // pass 6
                        {
                                ImplGetFilter ( 1, 2 );
                                mnYpos += 2;
                        }
                        break;

                        case 7 :                                                                            // pass 7
                        {
                            if ( mnYpos >= 1 )
                            {
                                ImplGetFilter ( 0, 1 );
                            }
                            mnYpos += 2;
                        }
                        break;
                    }
                }
            }

            if ( mnYpos >= mnHeight )
            {
                if ( mnPass == 7 )
                    break;
                else
                {
                    ImplResizeScanline();
                }
            }
        }

        mnCRC = mpZCodec->GetCRC();
    }

    if ( ( mnPass >= 7 ) && ( mnYpos >= mnHeight ) )
    {
        mpZCodec->EndCompression();
        mbzCodecInUse = FALSE;
        mbIDAT = TRUE;
    }
}

// ---------------------------------------------------------------------------------------------------

void PNGReaderImpl::ImplResizeScanline( void )
{
    long nScansize;

    while ( mnPass < 7 )
    {
        sal_uInt32 nX = 0;

        mnYpos = 0;
        mnPass++;

        switch ( mnPass )
        {
            case 1 :
                nScansize = ( mnWidth + 7 ) >> 3;
            break;

            case 2 :
                nX += 4;
                nScansize = ( mnWidth + 3 ) >> 3;
            break;

            case 3 :
                mnYpos += 4;
                nScansize = ( mnWidth + 3 ) >> 2;
            break;

            case 4 :
                nX += 2;
                nScansize = ( mnWidth + 1) >> 2;
            break;

            case 5 :
                mnYpos += 2;
                nScansize = ( mnWidth + 1 ) >> 1;
            break;

            case 6 :
                nX++;
                nScansize = mnWidth >> 1;
            break;

            case 7 :
                mnYpos++;
                nScansize = mnWidth;
            break;
        }

        if ( ( mnYpos >= mnHeight ) || ( nX >= mnWidth ) )  // is pass to be skipped ?
            continue;

        // in Interlace mode the size of scanline is not constant
        // so first we calculate the number of entrys
        mnScansize = nScansize * mnBitDepth;

        if ( mbRGBTriple )
            mnScansize *= 3;

        // convert bitsize to byte
        mnScansize = ( mnScansize + 7 ) >> 3;

        if ( mbAlphaChannel )
            mnScansize += ( ( nScansize * mnBitDepth ) >> 3 );

        rtl_zeroMemory( mpScanprior, ++mnScansize );
        break;
    }
}

// ---------------------------------------------------------------------------------------------------
// ImplGetFilter writes the complete Scanline (nY) - in interlace mode the parameter nXStart and nXAdd
// appends to the currently used pass
// the complete size of scanline will be returned - in interlace mode zero is possible!

void PNGReaderImpl::ImplGetFilter ( sal_uInt32 nXStart, sal_uInt32 nXAdd )
{
    BYTE*       pTmp = mpInflateInBuf;
    BYTE*       p1;
    BYTE*       p2;
    BYTE*       p3;
    BYTE*       p4;
    BYTE        nFilterType, nCol;
    sal_uInt32  nXIndex, nX, nY = mnYpos, n1, n2, na, nb, nc;
    sal_Int32   np, npa, npb, npc;

    sal_uInt32 nBBP = mnBBP;

    if ( nXStart < (sal_uInt32)mnWidth )
    {
        nFilterType = *pTmp++;  // the filter type may change each scanline

        switch ( nFilterType )
        {
            case 1 :
            {
                p1 = pTmp + nBBP;
                p2 = pTmp;

                while ( p1 < pTmp + mnScansize - 1 )
                    ( *p1++ ) += ( *p2++ );
            }
            break;

            case 2 :
            {
                p1 = pTmp;
                p2 = mpScanprior+1;

                while ( p1 < pTmp + mnScansize - 1 )
                    ( *p1++ ) += ( *p2++ );
            }
            break;

            case 3 :
            {
                p1 = pTmp;
                p2 = mpScanprior + 1;
                p3 = pTmp-nBBP;

                while ( p1 < pTmp + mnScansize - 1 )
                {
                    n1 = (BYTE)( *p2++ );
                    n2 = ( p3 >= pTmp ) ? (BYTE)*p3 : 0;
                    p3++;
                    ( *p1++ ) += (BYTE)( ( n1 + n2 ) >> 1 );
                }
            }
            break;

            case 4 :
            {
                p1 = pTmp;
                p2 = p1 - nBBP;
                p3 = mpScanprior + 1;
                p4 = p3 - nBBP;

                while ( p1 < pTmp + mnScansize - 1 )
                {
                    nb = *p3++;
                    if ( p2 >= pTmp )
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

                    if ( ( npa <= npb ) && ( npa <= npc ) )
                        *p1++ += (BYTE)na;
                    else if ( npb <= npc )
                        *p1++ += (BYTE)nb;
                    else
                        *p1++ += (BYTE)nc;

                    p2++;
                    p4++;
                }
            }
            break;
            // undefined FilterType -> we will let them pass and display the data unfiltered
        }
        if ( mpAcc->HasPalette() )  // alphachannel is not allowed by pictures including palette entries
        {
            switch ( mpAcc->GetBitCount() )
            {
                case 1 :
                {
                    if ( mbTransparent )
                    {
                        for ( nX = nXStart, nXIndex = 0; nX < (sal_uInt32)mnWidth; nX += nXAdd, nXIndex++ )
                        {
                            sal_uInt32 nShift = ( nXIndex & 7 ) ^ 7;

                            if ( nShift == 0 )
                                nCol = ( *pTmp++ ) & 1;
                            else
                                nCol = ( *pTmp >> nShift ) & 1;

                            ImplSetPixel( nY, nX, nCol, mpTransTab[ nCol ] < PNG_TRANS_VAL );
                        }
                    }
                    else
                    {
                        for ( nX = nXStart, nXIndex = 0; nX < (sal_uInt32)mnWidth; nX += nXAdd, nXIndex++ )
                        {
                            sal_uInt32 nShift = ( nXIndex & 7 ) ^ 7;

                            if ( nShift == 0 )
                                ImplSetPixel( nY, nX, ( *pTmp++ & 1 ), FALSE );
                            else
                                ImplSetPixel( nY, nX, ( *pTmp >> nShift ) & 1, FALSE );
                        }
                    }
                }
                break;

                case 4 :
                {
                    if ( mbTransparent )
                    {
                        if ( mnBitDepth == 4 )  // maybe the source is a two bitmap graphic
                        {
                            for ( nX = nXStart, nXIndex = 0; nX < mnWidth; nX += nXAdd, nXIndex++ )
                            {
                                if( nXIndex & 1 )
                                {
                                    ImplSetPixel( nY, nX, *pTmp & 0x0f, mpTransTab[ *pTmp & 0x0f ] < PNG_TRANS_VAL );
                                    pTmp++;
                                }
                                else
                                {
                                    ImplSetPixel( nY, nX, ( *pTmp >> 4 ) & 0x0f, mpTransTab[ *pTmp >> 4 ] < PNG_TRANS_VAL );
                                }
                            }
                        }
                        else
                        {
                            for ( nX = nXStart, nXIndex = 0; nX < mnWidth; nX += nXAdd, nXIndex++ )
                            {
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
                                }

                                ImplSetPixel( nY, nX, nCol, mpTransTab[ nCol ] < PNG_TRANS_VAL );
                            }
                        }
                    }
                    else
                    {
                        if ( mnBitDepth == 4 )  // maybe the source is a two bitmap graphic
                        {
                            for ( nX = nXStart, nXIndex = 0; nX < mnWidth; nX += nXAdd, nXIndex++ )
                            {
                                if( nXIndex & 1 )
                                    ImplSetPixel( nY, nX, *pTmp++ & 0x0f, FALSE );
                                else
                                    ImplSetPixel( nY, nX, ( *pTmp >> 4 ) & 0x0f, FALSE );
                            }
                        }
                        else
                        {
                            for ( nX = nXStart, nXIndex = 0; nX < mnWidth; nX += nXAdd, nXIndex++ )
                            {
                                switch( nXIndex & 3 )
                                {
                                    case 0 :
                                        ImplSetPixel( nY, nX, *pTmp >> 6, FALSE );
                                    break;

                                    case 1 :
                                        ImplSetPixel( nY, nX, ( *pTmp >> 4 ) & 0x03, FALSE );
                                    break;

                                    case 2 :
                                        ImplSetPixel( nY, nX, ( *pTmp >> 2 ) & 0x03, FALSE );
                                    break;

                                    case 3 :
                                        ImplSetPixel( nY, nX, *pTmp++ & 0x03, FALSE );
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
                        if ( mnBitDepth == 8 )  // maybe the source is a 16 bit grayscale
                        {
                            for ( nX = nXStart; nX < mnWidth; nX += nXAdd, pTmp += 2 )
                                ImplSetAlphaPixel( nY, nX, pTmp[ 0 ], pTmp[ 1 ] );
                        }
                        else
                        {
                            for ( nX = nXStart; nX < mnWidth; nX += nXAdd, pTmp += 4 )
                                ImplSetAlphaPixel( nY, nX, pTmp[ 0 ], pTmp[ 2 ] );
                        }
                    }
                    else
                    {
                        if ( mbTransparent )
                        {
                            if ( mnBitDepth == 8 )  // maybe the source is a 16 bit grayscale
                            {
                                for ( nX = nXStart; nX < mnWidth; nX += nXAdd, pTmp++ )
                                    ImplSetPixel( nY, nX, *pTmp, mpTransTab[ *pTmp ] < PNG_TRANS_VAL );
                            }
                            else
                            {
                                for ( nX = nXStart; nX < mnWidth; nX += nXAdd, pTmp += 2 )
                                    ImplSetPixel( nY, nX, *pTmp, mpTransTab[ *pTmp ] < PNG_TRANS_VAL );
                            }
                        }
                        else
                        {
                            if ( mnBitDepth == 8 )  // maybe the source is a 16 bit grayscale
                            {
                                for ( nX = nXStart; nX < mnWidth; nX += nXAdd )
                                    ImplSetPixel( nY, nX, *pTmp++, FALSE );
                            }
                            else
                            {
                                for ( nX = nXStart; nX < mnWidth; nX += nXAdd, pTmp += 2 )
                                    ImplSetPixel( nY, nX, *pTmp, FALSE );
                            }
                        }
                    }
                }
                break;

                default :
                    mbStatus = FALSE;
                break;
            }
        }
        else
        {
            if ( mbAlphaChannel || mbTransparent )
            {
                if ( mbAlphaChannel )
                {
                    if ( mnBitDepth == 8 )  // maybe the source is a 16 bit each sample
                    {
                        for ( nX = nXStart; nX < mnWidth; nX += nXAdd, pTmp += 4 )
                            ImplSetAlphaPixel( nY, nX, BitmapColor( mpColorTable[ pTmp[ 0 ] ],
                                                                        mpColorTable[ pTmp[ 1 ] ],
                                                                            mpColorTable[ pTmp[ 2 ] ] ), pTmp[ 3 ] );
                    }
                    else
                    {
                        for ( nX = nXStart; nX < mnWidth; nX += nXAdd, pTmp += 8 )
                            ImplSetAlphaPixel( nY, nX, BitmapColor( mpColorTable[ pTmp[ 0 ] ],
                                                                        mpColorTable[ pTmp[ 2 ] ],
                                                                            mpColorTable[ pTmp[ 4 ] ] ), pTmp[6] );
                    }
                }
                else // Transparency chunk
                {
                    sal_Bool    bTransparent;
                    sal_uInt8   nRed, nGreen, nBlue;

                    if ( mnBitDepth == 8 )  // maybe the source is a 16 bit each sample
                    {
                        for ( nX = nXStart; nX < mnWidth; nX += nXAdd, pTmp += 3 )
                        {
                            nRed = pTmp[ 0 ];
                            nGreen = pTmp[ 1 ];
                            nBlue = pTmp[ 2 ];
                            bTransparent = ( ( nRed == mnTransRed )
                                            && ( nGreen == mnTransGreen )
                                                && ( nBlue == mnTransBlue ) );

                            ImplSetPixel( nY, nX, BitmapColor( mpColorTable[ nRed ],
                                                                mpColorTable[ nGreen ],
                                                                    mpColorTable[ nBlue ] ), bTransparent );
                        }
                    }
                    else
                    {
                        for ( nX = nXStart; nX < mnWidth; nX += nXAdd, pTmp += 6 )
                        {
                            nRed = pTmp[ 0 ];
                            nGreen = pTmp[ 2 ];
                            nBlue = pTmp[ 4 ];
                            bTransparent = ( ( nRed == mnTransRed )
                                            && ( nGreen == mnTransGreen )
                                                && ( nBlue == mnTransBlue ) );

                            ImplSetPixel( nY, nX, BitmapColor( mpColorTable[ nRed ],
                                                                mpColorTable[ nGreen ],
                                                                    mpColorTable[ nBlue ] ), bTransparent );
                        }
                    }
                }
            }
            else
            {
                if ( mnBitDepth == 8 )  // maybe the source is a 16 bit each sample
                {
                    for ( nX = nXStart; nX < mnWidth; nX += nXAdd, pTmp += 3 )
                        ImplSetPixel( nY, nX, BitmapColor( mpColorTable[ pTmp[ 0 ] ],
                                                            mpColorTable[ pTmp[ 1 ] ],
                                                                mpColorTable[ pTmp[ 2 ] ] ), FALSE );
                }
                else
                {
                    for ( nX = nXStart; nX < mnWidth; nX += nXAdd, pTmp += 6 )
                        ImplSetPixel( nY, nX, BitmapColor( mpColorTable[ pTmp[ 0 ] ],
                                                            mpColorTable[ pTmp[ 2 ] ],
                                                                mpColorTable[ pTmp[ 4 ] ] ), FALSE );
                }
            }
        }
    }

    rtl_copyMemory( mpScanprior, mpInflateInBuf, mnScansize );
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplSetPixel( sal_uInt32 nY, sal_uInt32 nX, const BitmapColor& rBitmapColor, BOOL bTrans )
{
    if ( mnPass == 7 )
    {
        mpAcc->SetPixel( nY, nX, rBitmapColor );

        if ( bTrans )
            mpMaskAcc->SetPixel( nY, nX, cTransIndex1 );
        else
            mpMaskAcc->SetPixel( nY, nX, cNonTransIndex1 );
    }
    else
    {
        sal_uInt32 nTX;

        for ( int nBHeight = 0; nBHeight < aBlockHeight[ mnPass ]; nBHeight++ )
        {
            nTX = nX;

            for ( int nBWidth = 0; nBWidth < aBlockWidth[ mnPass ]; nBWidth++ )
            {
                mpAcc->SetPixel( nY, nTX, rBitmapColor );

                if ( bTrans )
                    mpMaskAcc->SetPixel( nY, nTX, cTransIndex1 );
                else
                    mpMaskAcc->SetPixel( nY, nTX, cNonTransIndex1 );

                if ( ++nTX == mnWidth )
                    break;
            }

            if ( ++nY == mnHeight )
                break;
        }
    }
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplSetPixel( sal_uInt32 nY, sal_uInt32 nX, BYTE nPalIndex, BOOL bTrans )
{
    if ( mnPass == 7 )
    {
        mpAcc->SetPixel( nY, nX, (BYTE)nPalIndex );

        if ( bTrans )
            mpMaskAcc->SetPixel( nY, nX, cTransIndex1 );
        else
            mpMaskAcc->SetPixel( nY, nX, cNonTransIndex1 );
    }
    else
    {
        sal_uInt32 nTX;

        for ( int nBHeight = 0; nBHeight < aBlockHeight[ mnPass ]; nBHeight++ )
        {
            nTX = nX;

            for ( int nBWidth = 0; nBWidth < aBlockWidth[ mnPass ]; nBWidth++ )
            {
                mpAcc->SetPixel( nY, nTX, (BYTE)nPalIndex );

                if ( bTrans )
                    mpMaskAcc->SetPixel( nY, nTX, cTransIndex1 );
                else
                    mpMaskAcc->SetPixel( nY, nTX, cNonTransIndex1 );

                if ( ++nTX == mnWidth )
                    break;
            }
            if ( ++nY == mnHeight )
                break;
        }
    }
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplSetAlphaPixel( sal_uInt32 nY, sal_uInt32 nX, const BitmapColor& rBitmapColor, BYTE nAlpha )
{
    if ( mnPass == 7 )
    {
        mpAcc->SetPixel( nY, nX, rBitmapColor );
        mpMaskAcc->SetPixel( nY, nX, 255 - nAlpha );
    }
    else
    {
        sal_uInt32 nTX;

        for ( int nBHeight = 0; nBHeight < aBlockHeight[ mnPass ]; nBHeight++ )
        {
            nTX = nX;

            for ( int nBWidth = 0; nBWidth < aBlockWidth[ mnPass ]; nBWidth++ )
            {
                mpAcc->SetPixel( nY, nTX, rBitmapColor );
                mpMaskAcc->SetPixel( nY, nTX, 255 - nAlpha );
                if ( ++nTX == mnWidth )
                    break;
            }

            if ( ++nY == mnHeight )
                break;
        }
    }
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplOpenChunk()
{
    sal_uInt32 nChunkDatSizeOrg, nChunkType;

    *mpIStm >> nChunkDatSizeOrg >> nChunkType;

    if ( mpIStm->GetError() != ERRCODE_NONE )
    {
        mbStatus = FALSE;
        return;
    }

    mnChunkDatSizeOrg = nChunkDatSizeOrg;
    mnChunkType = nChunkType;

#ifdef OSL_LITENDIAN
    nChunkType = SWAPLONG( nChunkType );
#endif
    mnCRC = rtl_crc32( 0, &nChunkType, 4 );

    mnChunkDatSize = 0;
    mnChunkStatus = CHUNK_IS_OPEN;
    mnChunkStartPosition = mpIStm->Tell();
}

// ------------------------------------------------------------------------

BYTE PNGReaderImpl::ImplReadBYTE()
{
    BYTE nRet;

    mnChunkDatSize++;
    *mpIStm >> nRet;
    mnCRC = rtl_crc32( mnCRC, &nRet, 1 );

    return nRet;
}

// ------------------------------------------------------------------------

sal_uInt32 PNGReaderImpl::ImplReadsal_uInt32()
{
    sal_uInt32 nRet;

    mnChunkDatSize+=4;
    *mpIStm >> nRet;

#ifdef OSL_LITENDIAN
    sal_uInt32 nTemp = SWAPLONG( nRet );
    mnCRC = rtl_crc32( mnCRC, &nTemp, 4 );
#else
    mnCRC = rtl_crc32( mnCRC, &nRet, 4 );
#endif

    return nRet;
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplReadDAT ( unsigned char* pDest, long nDatSize )
{
    mnChunkDatSize += nDatSize;
    mpIStm->Read( pDest, nDatSize );
    mnCRC = rtl_crc32( mnCRC, pDest, nDatSize );
}

// ------------------------------------------------------------------------

BOOL PNGReaderImpl::ImplCloseChunk ( void )
{
    sal_uInt32 nCheck;

    *mpIStm >> nCheck;

    if ( mpIStm->GetError() != ERRCODE_NONE )
    {
        mbStatus = FALSE;
        return FALSE;
    }

    mnChunkStatus = CHUNK_IS_CLOSED;

    return ( ( nCheck == mnCRC ) && ( mnChunkDatSizeOrg == mnChunkDatSize ) ) ? mbStatus : FALSE;
}

// ------------------------------------------------------------------------

void PNGReaderImpl::ImplSkipChunk( void )
{
    const long nSkipLen = mnChunkStartPosition + mnChunkDatSizeOrg + 4 - mpIStm->Tell();

    if( nSkipLen )
    {
        sal_Char* pBuffer = new sal_Char[ nSkipLen ];
        mpIStm->Read( pBuffer, nSkipLen );
        delete[] pBuffer;
    }

    mnChunkStatus = CHUNK_IS_CLOSED;
}

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

BitmapEx PNGReader::Read()
{
    return mpImpl->Read();
}

} // namespace vcl
