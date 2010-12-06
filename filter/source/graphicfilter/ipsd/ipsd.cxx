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
    UINT32  nSignature;
    UINT16  nVersion;
    UINT32  nPad1;
    UINT16  nPad2;
    UINT16  nChannels;
    UINT32  nRows;
    UINT32  nColumns;
    UINT16  nDepth;
    UINT16  nMode;

} PSDFileHeader;

class PSDReader {

private:

    SvStream& m_rPSD;           // Die einzulesende PSD-Datei
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
    USHORT              mnDestBitDepth;
    BOOL                mbCompression;  // RLE decoding
    BYTE*               mpPalette;

    BOOL                ImplReadBody();
    BOOL                ImplReadHeader();

public:
    PSDReader(SvStream &rStream);
    ~PSDReader();
    BOOL ReadPSD(Graphic & rGraphic);
};

//=================== Methoden von PSDReader ==============================

PSDReader::PSDReader(SvStream &rStream)
    : m_rPSD(rStream)
    , mpFileHeader(NULL)
    , mnXResFixed(0)
    , mnYResFixed(0)
    , mbStatus(TRUE)
    , mbTransparent(FALSE)
    , mpReadAcc(NULL)
    , mpWriteAcc(NULL)
    , mpMaskWriteAcc(NULL)
    , mpPalette(NULL)
{
}

PSDReader::~PSDReader()
{
    delete[] mpPalette;
    delete mpFileHeader;
}

// ------------------------------------------------------------------------

BOOL PSDReader::ReadPSD(Graphic & rGraphic )
{
    if (m_rPSD.GetError())
        return FALSE;

    m_rPSD.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    // Kopf einlesen:

    if ( ImplReadHeader() == FALSE )
        return FALSE;

    Size aBitmapSize( mpFileHeader->nColumns, mpFileHeader->nRows );
    maBmp = Bitmap( aBitmapSize, mnDestBitDepth );
    if ( ( mpWriteAcc = maBmp.AcquireWriteAccess() ) == NULL )
        mbStatus = FALSE;
    if ( ( mpReadAcc = maBmp.AcquireReadAccess() ) == NULL )
        mbStatus = FALSE;
    if ( mbTransparent && mbStatus )
    {
        maMaskBmp = Bitmap( aBitmapSize, 1 );
        if ( ( mpMaskWriteAcc = maMaskBmp.AcquireWriteAccess() ) == NULL )
            mbStatus = FALSE;
    }
    if ( mpPalette && mbStatus )
    {
        mpWriteAcc->SetPaletteEntryCount( 256 );
        for ( USHORT i = 0; i < 256; i++ )
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
        mbStatus = FALSE;
    if ( mpWriteAcc )
        maBmp.ReleaseAccess( mpWriteAcc );
    if ( mpReadAcc )
        maBmp.ReleaseAccess( mpReadAcc );
    if ( mpMaskWriteAcc )
        maMaskBmp.ReleaseAccess( mpMaskWriteAcc );
    return mbStatus;
}

// ------------------------------------------------------------------------

BOOL PSDReader::ImplReadHeader()
{
    UINT16  nCompression;
    UINT32  nColorLength, nResourceLength, nLayerMaskLength;

    mpFileHeader = new PSDFileHeader;

    if ( !mpFileHeader )
        return FALSE;

    m_rPSD >> mpFileHeader->nSignature >> mpFileHeader->nVersion >> mpFileHeader->nPad1 >>
        mpFileHeader->nPad2 >> mpFileHeader->nChannels >> mpFileHeader->nRows >>
            mpFileHeader->nColumns >> mpFileHeader->nDepth >> mpFileHeader->nMode;

    if ( ( mpFileHeader->nSignature != 0x38425053 ) || ( mpFileHeader->nVersion != 1 ) )
        return FALSE;

    if ( mpFileHeader->nRows == 0 || mpFileHeader->nColumns == 0 )
        return FALSE;

    if ( ( mpFileHeader->nRows > 30000 ) || ( mpFileHeader->nColumns > 30000 ) )
        return FALSE;

    UINT16 nDepth = mpFileHeader->nDepth;
    if (!( ( nDepth == 1 ) || ( nDepth == 8 ) || ( nDepth == 16 ) ) )
        return FALSE;

    mnDestBitDepth = ( nDepth == 16 ) ? 8 : nDepth;

    m_rPSD >> nColorLength;
    if ( mpFileHeader->nMode == PSD_CMYK )
    {
        switch ( mpFileHeader->nChannels )
        {
            case 5 :
                mbTransparent = TRUE;
            case 4 :
                mnDestBitDepth = 24;
            break;
            default :
                return FALSE;
        }
    }
    else switch ( mpFileHeader->nChannels )
    {
        case 2 :
            mbTransparent = TRUE;
        case 1 :
            break;
        case 4 :
            mbTransparent = TRUE;
        case 3 :
            mnDestBitDepth = 24;
            break;
        default:
            return FALSE;
    }

    switch ( mpFileHeader->nMode )
    {
        case PSD_BITMAP :
        {
            if ( nColorLength || ( nDepth != 1 ) )
                return FALSE;
        }
        break;

        case PSD_INDEXED :
        {
            if ( nColorLength != 768 )      // we need the color map
                return FALSE;
            mpPalette = new BYTE[ 768 ];
            if ( mpPalette == NULL )
                return FALSE;
            m_rPSD.Read( mpPalette, 768 );
        }
        break;

        case PSD_DUOTONE :                  // we'll handle the doutone color like a normal grayscale picture
            m_rPSD.SeekRel( nColorLength );
            nColorLength = 0;
        case PSD_GRAYSCALE :
        {
            if ( nColorLength )
                return FALSE;
            mpPalette = new BYTE[ 768 ];
            if ( mpPalette == NULL )
                return FALSE;
            for ( USHORT i = 0; i < 256; i++ )
            {
                mpPalette[ i ] = mpPalette[ i + 256 ] = mpPalette[ i + 512 ] = (BYTE)i;
            }
        }
        break;

        case PSD_CMYK :
        case PSD_RGB :
        case PSD_MULTICHANNEL :
        case PSD_LAB :
        {
            if ( nColorLength )     // color table is not supported by the other graphic modes
                return FALSE;
        }
        break;

        default:
            return FALSE;
    }
    m_rPSD >> nResourceLength;
    sal_uInt32 nLayerPos = m_rPSD.Tell() + nResourceLength;

    // this is a loop over the resource entries to get the resolution info
    while( m_rPSD.Tell() < nLayerPos )
    {
        sal_uInt8 n8;
        sal_uInt32 nType, nPStringLen, nResEntryLen;
        sal_uInt16 nUniqueID;

        m_rPSD >> nType >> nUniqueID >> n8;
        nPStringLen = n8;
        if ( nType != 0x3842494d )
            break;
        if ( ! ( nPStringLen & 1 ) )
            nPStringLen++;
        m_rPSD.SeekRel( nPStringLen );  // skipping the pstring
        m_rPSD >> nResEntryLen;
        if ( nResEntryLen & 1 )
            nResEntryLen++;             // the resource entries are padded
        sal_uInt32 nCurrentPos = m_rPSD.Tell();
        if ( ( nResEntryLen + nCurrentPos ) > nLayerPos )   // check if size
            break;                                          // is possible
        switch( nUniqueID )
        {
            case 0x3ed :    // UID for the resolution info
            {
                sal_Int16   nUnit;

                m_rPSD >> mnXResFixed >> nUnit >> nUnit
                       >> mnYResFixed >> nUnit >> nUnit;
            }
            break;
        }
        m_rPSD.Seek( nCurrentPos + nResEntryLen );          // set the stream to the next
    }                                                       // resource entry
    m_rPSD.Seek( nLayerPos );
    m_rPSD >> nLayerMaskLength;
    m_rPSD.SeekRel( nLayerMaskLength );

    m_rPSD >> nCompression;
    if ( nCompression == 0 )
    {
        mbCompression = FALSE;
    }
    else if ( nCompression == 1 )
    {
        m_rPSD.SeekRel( ( mpFileHeader->nRows * mpFileHeader->nChannels ) << 1 );
        mbCompression = TRUE;
    }
    else
        return FALSE;

    return TRUE;
}

// ------------------------------------------------------------------------

BOOL PSDReader::ImplReadBody()
{
    ULONG       nX, nY;
    char        nRunCount = 0;
    signed char nBitCount = -1;
    BYTE        nDat = 0, nDummy, nRed, nGreen, nBlue;
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
                        m_rPSD >> nRunCount;
                }
                if ( nRunCount & 0x80 )     // a run length packet
                {
                    if ( nBitCount == -1 )  // bits left in nDat ?
                    {
                        m_rPSD >> nDat;
                        nDat ^= 0xff;
                        nBitCount = 7;
                    }
                    for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        mpWriteAcc->SetPixel( nY, nX, (BYTE)nDat >> nBitCount-- );
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
                    for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        if ( nBitCount == -1 )  // bits left in nDat ?
                        {
                            m_rPSD >> nDat;
                            nDat ^= 0xff;
                            nBitCount = 7;
                        }
                        mpWriteAcc->SetPixel( nY, nX, (BYTE)nDat >> nBitCount-- );
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
                    m_rPSD >> nRunCount;

                if ( nRunCount & 0x80 )     // a run length packet
                {
                    m_rPSD >> nDat;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        m_rPSD >> nDummy;
                    for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        mpWriteAcc->SetPixel( nY, nX, (BYTE)nDat );
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
                    for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        m_rPSD >> nDat;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            m_rPSD >> nDummy;
                        mpWriteAcc->SetPixel( nY, nX, (BYTE)nDat );
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
                    m_rPSD >> nRunCount;

                if ( nRunCount & 0x80 )     // a run length packet
                {
                    m_rPSD >> nRed;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        m_rPSD >> nDummy;
                    for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( nRed, (BYTE)0, (BYTE)0 ) );
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
                    for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        m_rPSD >> nRed;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            m_rPSD >> nDummy;
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( nRed, (BYTE)0, (BYTE)0 ) );
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
                    m_rPSD >> nRunCount;
                if ( nRunCount & 0x80 )     // a run length packet
                {
                    m_rPSD >> nGreen;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        m_rPSD >> nDummy;
                    for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
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
                    for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        m_rPSD >> nGreen;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            m_rPSD >> nDummy;
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
                    m_rPSD >> nRunCount;
                if ( nRunCount & 0x80 )     // a run length packet
                {
                    m_rPSD >> nBlue;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        m_rPSD >> nDummy;
                    for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
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
                    for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        m_rPSD >> nBlue;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            m_rPSD >> nDummy;
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
                UINT32  nBlack, nBlackMax = 0;
                BYTE*   pBlack = new BYTE[ mpFileHeader->nRows * mpFileHeader->nColumns ];
                nY = 0;
                while ( nY < mpFileHeader->nRows )
                {
                    if ( mbCompression )        // else nRunCount = 0 -> so we use only single raw packets
                        m_rPSD >> nRunCount;

                    if ( nRunCount & 0x80 )     // a run length packet
                    {
                        m_rPSD >> nDat;

                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            m_rPSD >> nDummy;

                        for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                        {
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetRed() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetGreen() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetBlue() + nDat;
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
                        for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                        {
                            m_rPSD >> nDat;

                            if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                                m_rPSD >> nDummy;
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetRed() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetGreen() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetBlue() + nDat;
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
                        INT32 nDAT = pBlack[ nX + nY * mpFileHeader->nColumns ] * ( nBlackMax - 256 ) / 0x1ff;

                        aBitmapColor = mpReadAcc->GetPixel( nY, nX );
                        BYTE cR = (BYTE) MinMax( aBitmapColor.GetRed() - nDAT, 0L, 255L );
                        BYTE cG = (BYTE) MinMax( aBitmapColor.GetGreen() - nDAT, 0L, 255L );
                        BYTE cB = (BYTE) MinMax( aBitmapColor.GetBlue() - nDAT, 0L, 255L );
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
                m_rPSD >> nRunCount;

            if ( nRunCount & 0x80 )     // a run length packet
            {
                m_rPSD >> nDat;
                if ( nDat )
                    nDat = 0;
                else
                    nDat = 1;
                if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                    m_rPSD >> nDummy;
                for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                {
                    mpMaskWriteAcc->SetPixel( nY, nX, (BYTE)nDat );
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
                for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                {
                    m_rPSD >> nDat;
                    if ( nDat )
                        nDat = 0;
                    else
                        nDat = 1;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        m_rPSD >> nDummy;
                    mpMaskWriteAcc->SetPixel( nY, nX, (BYTE)nDat );
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
    return TRUE;
}

//================== GraphicImport - die exportierte Funktion ================

extern "C" BOOL __LOADONCALLAPI GraphicImport(SvStream & rStream, Graphic & rGraphic, FilterConfigItem*, BOOL )
{
    PSDReader aPSDReader(rStream);

    return aPSDReader.ReadPSD(rGraphic);
}

//================== ein bischen Muell fuer Windows ==========================

#ifdef WIN

static HINSTANCE hDLLInst = 0;      // HANDLE der DLL

extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
#ifndef WNT
    if ( nHeap )
        UnlockData( 0 );
#endif

    hDLLInst = hDLL;

    return TRUE;
}

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
