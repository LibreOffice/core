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


#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/outdev.hxx>

class FilterConfigItem;

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
    sal_uInt16              mnDestBitDepth;
    sal_Bool                mbCompression;  // RLE decoding
    sal_uInt8*              mpPalette;

    sal_Bool                ImplReadBody();
    sal_Bool                ImplReadHeader();

public:
    PSDReader(SvStream &rStream);
    ~PSDReader();
    sal_Bool ReadPSD(Graphic & rGraphic);
};

//=================== Methods of PSDReader ==============================

PSDReader::PSDReader(SvStream &rStream)
    : m_rPSD(rStream)
    , mpFileHeader(NULL)
    , mnXResFixed(0)
    , mnYResFixed(0)
    , mbStatus(sal_True)
    , mbTransparent(sal_False)
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

sal_Bool PSDReader::ReadPSD(Graphic & rGraphic )
{
    if (m_rPSD.GetError())
        return sal_False;

    m_rPSD.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    // read header:

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
    // read bitmap data
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

    m_rPSD >> mpFileHeader->nSignature >> mpFileHeader->nVersion >> mpFileHeader->nPad1 >>
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

    m_rPSD >> nColorLength;
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
            m_rPSD.Read( mpPalette, 768 );
        }
        break;

        case PSD_DUOTONE :                  // we'll handle the duotone color like a normal grayscale picture
            m_rPSD.SeekRel( nColorLength );
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
        mbCompression = sal_False;
    }
    else if ( nCompression == 1 )
    {
        m_rPSD.SeekRel( ( mpFileHeader->nRows * mpFileHeader->nChannels ) << 1 );
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
                        m_rPSD >> nRunCount;
                }
                if ( nRunCount & 0x80 )     // a run length packet
                {
                    if ( nBitCount == -1 )  // bits left in nDat?
                    {
                        m_rPSD >> nDat;
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
                            m_rPSD >> nDat;
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
                    m_rPSD >> nRunCount;

                if ( nRunCount & 0x80 )     // a run length packet
                {
                    m_rPSD >> nDat;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        m_rPSD >> nDummy;
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
                        m_rPSD >> nDat;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            m_rPSD >> nDummy;
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
                    m_rPSD >> nRunCount;

                if ( nRunCount & 0x80 )     // a run length packet
                {
                    m_rPSD >> nRed;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        m_rPSD >> nDummy;
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
                        m_rPSD >> nRed;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            m_rPSD >> nDummy;
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
                    m_rPSD >> nRunCount;
                if ( nRunCount & 0x80 )     // a run length packet
                {
                    m_rPSD >> nGreen;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        m_rPSD >> nDummy;
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
                sal_uInt32  nBlack, nBlackMax = 0;
                sal_uInt8*  pBlack = new sal_uInt8[ mpFileHeader->nRows * mpFileHeader->nColumns ];
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
                            m_rPSD >> nDat;

                            if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                                m_rPSD >> nDummy;
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
                    m_rPSD >> nDat;
                    if ( nDat )
                        nDat = 0;
                    else
                        nDat = 1;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        m_rPSD >> nDummy;
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

//================== GraphicImport - the exported function ================

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool __LOADONCALLAPI
GraphicImport(SvStream & rStream, Graphic & rGraphic, FilterConfigItem*, sal_Bool)
{
    PSDReader aPSDReader(rStream);

    return aPSDReader.ReadPSD(rGraphic);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
