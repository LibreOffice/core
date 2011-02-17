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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <svtools/fltcall.hxx>

//============================ PSDReader ==================================

#define PSD_BITMAP          0
#define PSD_GRAYSCALE       1
#define PSD_INDEXED         2
#define PSD_RGB             3
#define PSD_CMYK            4
#define PSD_MULTICHANNEL    7
#define PSD_DUOTONE         8
#define PSD_LAB             9

typedef struct
{
    sal_uInt32  nSignature;
    sal_uInt16  nVersion;
    sal_uInt32  nPad1;
    sal_uInt16  nPad2;
    sal_uInt16  nChannels;
    sal_uInt32  nRows;
    sal_uInt32  nColumns;
    sal_uInt16  nDepth;
    sal_uInt16  nMode;

} PSDFileHeader;

class PSDReader {

private:

    SvStream*           mpPSD;          // Die einzulesende PSD-Datei
    PSDFileHeader*      mpFileHeader;

    sal_uInt32          mnXResFixed;
    sal_uInt32          mnYResFixed;

    sal_Bool            mbStatus;
    sal_Bool            mbTransparent;

    Bitmap              maBmp;
    Bitmap              maMaskBmp;
    BitmapReadAccess*   mpReadAcc;
    BitmapWriteAccess*  mpWriteAcc;
    BitmapWriteAccess*  mpMaskWriteAcc;
    sal_uInt16              mnDestBitDepth;
    sal_Bool                mbCompression;  // RLE decoding
    sal_uInt8*              mpPalette;

    sal_Bool                ImplReadBody();
    sal_Bool                ImplReadHeader();

public:
                        PSDReader();
                        ~PSDReader();
    sal_Bool                ReadPSD( SvStream & rPSD, Graphic & rGraphic );
};

//=================== Methoden von PSDReader ==============================

PSDReader::PSDReader() :
    mpFileHeader    ( NULL ),
    mnXResFixed     ( 0 ),
    mnYResFixed     ( 0 ),
    mbStatus        ( sal_True ),
    mbTransparent   ( sal_False ),
    mpReadAcc       ( NULL ),
    mpWriteAcc      ( NULL ),
    mpMaskWriteAcc  ( NULL ),
    mpPalette       ( NULL )
{
}

PSDReader::~PSDReader()
{
    delete[] mpPalette;
    delete mpFileHeader;
}

// ------------------------------------------------------------------------

sal_Bool PSDReader::ReadPSD( SvStream & rPSD, Graphic & rGraphic )
{
    if ( rPSD.GetError() )
        return sal_False;

    mpPSD = &rPSD;
    mpPSD->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    // Kopf einlesen:

    if ( ImplReadHeader() == sal_False )
        return sal_False;

    Size aBitmapSize( mpFileHeader->nColumns, mpFileHeader->nRows );
    maBmp = Bitmap( aBitmapSize, mnDestBitDepth );
    if ( ( mpWriteAcc = maBmp.AcquireWriteAccess() ) == NULL )
        mbStatus = sal_False;
    if ( ( mpReadAcc = maBmp.AcquireReadAccess() ) == NULL )
        mbStatus = sal_False;
    if ( mbTransparent && mbStatus )
    {
        maMaskBmp = Bitmap( aBitmapSize, 1 );
        if ( ( mpMaskWriteAcc = maMaskBmp.AcquireWriteAccess() ) == NULL )
            mbStatus = sal_False;
    }
    if ( mpPalette && mbStatus )
    {
        mpWriteAcc->SetPaletteEntryCount( 256 );
        for ( sal_uInt16 i = 0; i < 256; i++ )
        {
            mpWriteAcc->SetPaletteColor( i, Color( mpPalette[ i ], mpPalette[ i + 256 ], mpPalette[ i + 512 ] ) );
        }
    }
    // Bitmap-Daten einlesen
    if ( mbStatus && ImplReadBody() )
    {
        if ( mbTransparent )
            rGraphic = Graphic( BitmapEx( maBmp, maMaskBmp ) );
        else
            rGraphic = maBmp;

        if ( mnXResFixed && mnYResFixed )
        {
            Point       aEmptyPoint;
            Fraction    aFractX( 1, mnXResFixed >> 16 );
            Fraction    aFractY( 1, mnYResFixed >> 16 );
            MapMode     aMapMode( MAP_INCH, aEmptyPoint, aFractX, aFractY );
            Size        aPrefSize = OutputDevice::LogicToLogic( aBitmapSize, aMapMode, MAP_100TH_MM );
            rGraphic.SetPrefSize( aPrefSize );
            rGraphic.SetPrefMapMode( MapMode( MAP_100TH_MM ) );
        }
    }
    else
        mbStatus = sal_False;
    if ( mpWriteAcc )
        maBmp.ReleaseAccess( mpWriteAcc );
    if ( mpReadAcc )
        maBmp.ReleaseAccess( mpReadAcc );
    if ( mpMaskWriteAcc )
        maMaskBmp.ReleaseAccess( mpMaskWriteAcc );
    return mbStatus;
}

// ------------------------------------------------------------------------

sal_Bool PSDReader::ImplReadHeader()
{
    sal_uInt16  nCompression;
    sal_uInt32  nColorLength, nResourceLength, nLayerMaskLength;

    mpFileHeader = new PSDFileHeader;

    if ( !mpFileHeader )
        return sal_False;

    *mpPSD >> mpFileHeader->nSignature >> mpFileHeader->nVersion >> mpFileHeader->nPad1 >>
        mpFileHeader->nPad2 >> mpFileHeader->nChannels >> mpFileHeader->nRows >>
            mpFileHeader->nColumns >> mpFileHeader->nDepth >> mpFileHeader->nMode;

    if ( ( mpFileHeader->nSignature != 0x38425053 ) || ( mpFileHeader->nVersion != 1 ) )
        return sal_False;

    if ( mpFileHeader->nRows == 0 || mpFileHeader->nColumns == 0 )
        return sal_False;

    if ( ( mpFileHeader->nRows > 30000 ) || ( mpFileHeader->nColumns > 30000 ) )
        return sal_False;

    sal_uInt16 nDepth = mpFileHeader->nDepth;
    if (!( ( nDepth == 1 ) || ( nDepth == 8 ) || ( nDepth == 16 ) ) )
        return sal_False;

    mnDestBitDepth = ( nDepth == 16 ) ? 8 : nDepth;

    *mpPSD >> nColorLength;
    if ( mpFileHeader->nMode == PSD_CMYK )
    {
        switch ( mpFileHeader->nChannels )
        {
            case 5 :
                mbTransparent = sal_True;
            case 4 :
                mnDestBitDepth = 24;
            break;
            default :
                return sal_False;
        }
    }
    else switch ( mpFileHeader->nChannels )
    {
        case 2 :
            mbTransparent = sal_True;
        case 1 :
            break;
        case 4 :
            mbTransparent = sal_True;
        case 3 :
            mnDestBitDepth = 24;
            break;
        default:
            return sal_False;
    }

    switch ( mpFileHeader->nMode )
    {
        case PSD_BITMAP :
        {
            if ( nColorLength || ( nDepth != 1 ) )
                return sal_False;
        }
        break;

        case PSD_INDEXED :
        {
            if ( nColorLength != 768 )      // we need the color map
                return sal_False;
            mpPalette = new sal_uInt8[ 768 ];
            if ( mpPalette == NULL )
                return sal_False;
            mpPSD->Read( mpPalette, 768 );
        }
        break;

        case PSD_DUOTONE :                  // we'll handle the doutone color like a normal grayscale picture
            mpPSD->SeekRel( nColorLength );
            nColorLength = 0;
        case PSD_GRAYSCALE :
        {
            if ( nColorLength )
                return sal_False;
            mpPalette = new sal_uInt8[ 768 ];
            if ( mpPalette == NULL )
                return sal_False;
            for ( sal_uInt16 i = 0; i < 256; i++ )
            {
                mpPalette[ i ] = mpPalette[ i + 256 ] = mpPalette[ i + 512 ] = (sal_uInt8)i;
            }
        }
        break;

        case PSD_CMYK :
        case PSD_RGB :
        case PSD_MULTICHANNEL :
        case PSD_LAB :
        {
            if ( nColorLength )     // color table is not supported by the other graphic modes
                return sal_False;
        }
        break;

        default:
            return sal_False;
    }
    *mpPSD >> nResourceLength;
    sal_uInt32 nLayerPos = mpPSD->Tell() + nResourceLength;

    // this is a loop over the resource entries to get the resolution info
    while( mpPSD->Tell() < nLayerPos )
    {
        sal_uInt8 n8;
        sal_uInt32 nType, nPStringLen, nResEntryLen;
        sal_uInt16 nUniqueID;

        *mpPSD >> nType >> nUniqueID >> n8;
        nPStringLen = n8;
        if ( nType != 0x3842494d )
            break;
        if ( ! ( nPStringLen & 1 ) )
            nPStringLen++;
        mpPSD->SeekRel( nPStringLen );  // skipping the pstring
        *mpPSD >> nResEntryLen;
        if ( nResEntryLen & 1 )
            nResEntryLen++;             // the resource entries are padded
        sal_uInt32 nCurrentPos = mpPSD->Tell();
        if ( ( nResEntryLen + nCurrentPos ) > nLayerPos )   // check if size
            break;                                          // is possible
        switch( nUniqueID )
        {
            case 0x3ed :    // UID for the resolution info
            {
                sal_Int16   nUnit;

                *mpPSD >> mnXResFixed >> nUnit >> nUnit
                       >> mnYResFixed >> nUnit >> nUnit;
            }
            break;
        }
        mpPSD->Seek( nCurrentPos + nResEntryLen );          // set the stream to the next
    }                                                       // resource entry
    mpPSD->Seek( nLayerPos );
    *mpPSD >> nLayerMaskLength;
    mpPSD->SeekRel( nLayerMaskLength );

    *mpPSD >> nCompression;
    if ( nCompression == 0 )
    {
        mbCompression = sal_False;
    }
    else if ( nCompression == 1 )
    {
        mpPSD->SeekRel( ( mpFileHeader->nRows * mpFileHeader->nChannels ) << 1 );
        mbCompression = sal_True;
    }
    else
        return sal_False;

    return sal_True;
}

// ------------------------------------------------------------------------

sal_Bool PSDReader::ImplReadBody()
{
    sal_uLong       nX, nY;
    char        nRunCount = 0;
    signed char nBitCount = -1;
    sal_uInt8       nDat = 0, nDummy, nRed, nGreen, nBlue;
    BitmapColor aBitmapColor;
    nX = nY = 0;

    switch ( mnDestBitDepth )
    {
        case 1 :
        {
            while ( nY < mpFileHeader->nRows )
            {
                if ( nBitCount == -1 )
                {
                    if ( mbCompression )    // else nRunCount = 0 -> so we use only single raw packets
                        *mpPSD >> nRunCount;
                }
                if ( nRunCount & 0x80 )     // a run length packet
                {
                    if ( nBitCount == -1 )  // bits left in nDat ?
                    {
                        *mpPSD >> nDat;
                        nDat ^= 0xff;
                        nBitCount = 7;
                    }
                    for ( sal_uInt16 i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        mpWriteAcc->SetPixel( nY, nX, (sal_uInt8)nDat >> nBitCount-- );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            nBitCount = -1;
                            if ( nY == mpFileHeader->nRows )
                                break;
                        }
                    }
                }
                else                        // a raw packet
                {
                    for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        if ( nBitCount == -1 )  // bits left in nDat ?
                        {
                            *mpPSD >> nDat;
                            nDat ^= 0xff;
                            nBitCount = 7;
                        }
                        mpWriteAcc->SetPixel( nY, nX, (sal_uInt8)nDat >> nBitCount-- );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            nBitCount = -1;
                            if ( nY == mpFileHeader->nRows )
                                break;
                        }
                    }
                }
            }
        }
        break;

        case 8 :
        {
            while ( nY < mpFileHeader->nRows )
            {
                if ( mbCompression )        // else nRunCount = 0 -> so we use only single raw packets
                    *mpPSD >> nRunCount;

                if ( nRunCount & 0x80 )     // a run length packet
                {
                    *mpPSD >> nDat;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        *mpPSD >> nDummy;
                    for ( sal_uInt16 i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        mpWriteAcc->SetPixel( nY, nX, (sal_uInt8)nDat );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            if ( nY == mpFileHeader->nRows )
                                break;
                        }
                    }
                }
                else                        // a raw packet
                {
                    for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        *mpPSD >> nDat;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            *mpPSD >> nDummy;
                        mpWriteAcc->SetPixel( nY, nX, (sal_uInt8)nDat );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            if ( nY == mpFileHeader->nRows )
                                break;
                        }
                    }
                }
            }
        }
        break;

        case 24 :
        {

            // the psd format is in plain order (RRRR GGGG BBBB) so we have to set each pixel three times
            // maybe the format is CCCC MMMM YYYY KKKK

            while ( nY < mpFileHeader->nRows )
            {
                if ( mbCompression )        // else nRunCount = 0 -> so we use only single raw packets
                    *mpPSD >> nRunCount;

                if ( nRunCount & 0x80 )     // a run length packet
                {
                    *mpPSD >> nRed;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        *mpPSD >> nDummy;
                    for ( sal_uInt16 i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( nRed, (sal_uInt8)0, (sal_uInt8)0 ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            if ( nY == mpFileHeader->nRows )
                                break;
                        }
                    }
                }
                else                        // a raw packet
                {
                    for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        *mpPSD >> nRed;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            *mpPSD >> nDummy;
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( nRed, (sal_uInt8)0, (sal_uInt8)0 ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            if ( nY == mpFileHeader->nRows )
                                break;
                        }
                    }
                }
            }
            nY = 0;
            while ( nY < mpFileHeader->nRows )
            {
                if ( mbCompression )
                    *mpPSD >> nRunCount;
                if ( nRunCount & 0x80 )     // a run length packet
                {
                    *mpPSD >> nGreen;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        *mpPSD >> nDummy;
                    for ( sal_uInt16 i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        aBitmapColor = mpReadAcc->GetPixel( nY, nX );
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( aBitmapColor.GetRed(), nGreen, aBitmapColor.GetBlue() ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            if ( nY == mpFileHeader->nRows )
                                break;
                        }
                    }
                }
                else                        // a raw packet
                {
                    for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        *mpPSD >> nGreen;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            *mpPSD >> nDummy;
                        aBitmapColor = mpReadAcc->GetPixel( nY, nX );
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( aBitmapColor.GetRed(), nGreen, aBitmapColor.GetBlue() ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            if ( nY == mpFileHeader->nRows )
                                break;
                        }
                    }
                }
            }
            nY = 0;
            while ( nY < mpFileHeader->nRows )
            {
                if ( mbCompression )
                    *mpPSD >> nRunCount;
                if ( nRunCount & 0x80 )     // a run length packet
                {
                    *mpPSD >> nBlue;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        *mpPSD >> nDummy;
                    for ( sal_uInt16 i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        aBitmapColor = mpReadAcc->GetPixel( nY, nX );
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( aBitmapColor.GetRed(), aBitmapColor.GetGreen(), nBlue ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            if ( nY == mpFileHeader->nRows )
                                break;
                        }
                    }
                }
                else                        // a raw packet
                {
                    for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        *mpPSD >> nBlue;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            *mpPSD >> nDummy;
                        aBitmapColor = mpReadAcc->GetPixel( nY, nX );
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( aBitmapColor.GetRed(), aBitmapColor.GetGreen(), nBlue ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            if ( nY == mpFileHeader->nRows )
                                break;
                        }
                    }
                }
            }
            if ( mpFileHeader->nMode == PSD_CMYK )
            {
                sal_uInt32  nBlack, nBlackMax = 0;
                sal_uInt8*  pBlack = new sal_uInt8[ mpFileHeader->nRows * mpFileHeader->nColumns ];
                nY = 0;
                while ( nY < mpFileHeader->nRows )
                {
                    if ( mbCompression )        // else nRunCount = 0 -> so we use only single raw packets
                        *mpPSD >> nRunCount;

                    if ( nRunCount & 0x80 )     // a run length packet
                    {
                        *mpPSD >> nDat;

                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            *mpPSD >> nDummy;

                        for ( sal_uInt16 i = 0; i < ( -nRunCount + 1 ); i++ )
                        {
                            nBlack = (sal_uInt8)mpReadAcc->GetPixel( nY, nX ).GetRed() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (sal_uInt8)mpReadAcc->GetPixel( nY, nX ).GetGreen() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (sal_uInt8)mpReadAcc->GetPixel( nY, nX ).GetBlue() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            pBlack[ nX + nY * mpFileHeader->nColumns ] = nDat ^ 0xff;
                            if ( ++nX == mpFileHeader->nColumns )
                            {
                                nX = 0;
                                nY++;
                            if ( nY == mpFileHeader->nRows )
                                break;
                            }
                        }
                    }
                    else                        // a raw packet
                    {
                        for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                        {
                            *mpPSD >> nDat;

                            if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                                *mpPSD >> nDummy;
                            nBlack = (sal_uInt8)mpReadAcc->GetPixel( nY, nX ).GetRed() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (sal_uInt8)mpReadAcc->GetPixel( nY, nX ).GetGreen() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (sal_uInt8)mpReadAcc->GetPixel( nY, nX ).GetBlue() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            pBlack[ nX + nY * mpFileHeader->nColumns ] = nDat ^ 0xff;
                            if ( ++nX == mpFileHeader->nColumns )
                            {
                                nX = 0;
                                nY++;
                                if ( nY == mpFileHeader->nRows )
                                    break;
                            }
                        }
                    }
                }

                for ( nY = 0; nY < mpFileHeader->nRows; nY++ )
                {
                    for ( nX = 0; nX < mpFileHeader->nColumns; nX++ )
                    {
                        sal_Int32 nDAT = pBlack[ nX + nY * mpFileHeader->nColumns ] * ( nBlackMax - 256 ) / 0x1ff;

                        aBitmapColor = mpReadAcc->GetPixel( nY, nX );
                        sal_uInt8 cR = (sal_uInt8) MinMax( aBitmapColor.GetRed() - nDAT, 0L, 255L );
                        sal_uInt8 cG = (sal_uInt8) MinMax( aBitmapColor.GetGreen() - nDAT, 0L, 255L );
                        sal_uInt8 cB = (sal_uInt8) MinMax( aBitmapColor.GetBlue() - nDAT, 0L, 255L );
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( cR, cG, cB ) );
                    }
                }
                delete[] pBlack;
            }
        }
        break;
    }

    if ( mbTransparent )
    {
        // the psd is 24 or 8 bit grafix + alphachannel

        nY = nX = 0;
        while ( nY < mpFileHeader->nRows )
        {
            if ( mbCompression )        // else nRunCount = 0 -> so we use only single raw packets
                *mpPSD >> nRunCount;

            if ( nRunCount & 0x80 )     // a run length packet
            {
                *mpPSD >> nDat;
                if ( nDat )
                    nDat = 0;
                else
                    nDat = 1;
                if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                    *mpPSD >> nDummy;
                for ( sal_uInt16 i = 0; i < ( -nRunCount + 1 ); i++ )
                {
                    mpMaskWriteAcc->SetPixel( nY, nX, (sal_uInt8)nDat );
                    if ( ++nX == mpFileHeader->nColumns )
                    {
                        nX = 0;
                        nY++;
                        if ( nY == mpFileHeader->nRows )
                            break;
                    }
                }
            }
            else                        // a raw packet
            {
                for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                {
                    *mpPSD >> nDat;
                    if ( nDat )
                        nDat = 0;
                    else
                        nDat = 1;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        *mpPSD >> nDummy;
                    mpMaskWriteAcc->SetPixel( nY, nX, (sal_uInt8)nDat );
                    if ( ++nX == mpFileHeader->nColumns )
                    {
                        nX = 0;
                        nY++;
                        if ( nY == mpFileHeader->nRows )
                            break;
                    }
                }
            }
        }
    }
    return sal_True;
}

//================== GraphicImport - die exportierte Funktion ================

extern "C" sal_Bool __LOADONCALLAPI GraphicImport(SvStream & rStream, Graphic & rGraphic, FilterConfigItem*, sal_Bool )
{
    PSDReader aPSDReader;

    return aPSDReader.ReadPSD( rStream, rGraphic );
}

