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

class FilterConfigItem;

//============================ TGAReader ==================================

struct TGAFileHeader
{
    sal_uInt8       nImageIDLength;
    sal_uInt8       nColorMapType;
    sal_uInt8       nImageType;
    sal_uInt16      nColorMapFirstEntryIndex;
    sal_uInt16      nColorMapLength;
    sal_uInt8       nColorMapEntrySize;
    sal_uInt16      nColorMapXOrigin;
    sal_uInt16      nColorMapYOrigin;
    sal_uInt16      nImageWidth;
    sal_uInt16      nImageHeight;
    sal_uInt8       nPixelDepth;
    sal_uInt8       nImageDescriptor;
};

#define SizeOfTGAFileFooter 26

struct TGAFileFooter
{
    sal_uInt32      nExtensionFileOffset;
    sal_uInt32      nDeveloperDirectoryOffset;
    sal_uInt32      nSignature[4];
    sal_uInt8       nPadByte;
    sal_uInt8       nStringTerminator;
};

#define SizeOfTGAExtension 495

struct TGAExtension
{
    sal_uInt16      nExtensionSize;
    char        sAuthorName[41];
    char        sAuthorComment[324];
    char        sDateTimeStamp[12];
    char        sJobNameID[41];
    sal_uInt16      nJobTime[3];
    char        sSoftwareID[41];
    sal_uInt16      nSoftwareVersionNumber;
    sal_uInt8       nSoftwareVersionLetter;
    sal_uInt32      nKeyColor;
    sal_uInt16      nPixelAspectRatioNumerator;
    sal_uInt16      nPixelAspectRatioDeNumerator;
    sal_uInt16      nGammaValueNumerator;
    sal_uInt16      nGammaValueDeNumerator;
    sal_uInt32      nColorCorrectionOffset;
    sal_uInt32      nPostageStampOffset;
    sal_uInt32      nScanLineOffset;
    sal_uInt8       nAttributesType;
};

class TGAReader {

private:

    SvStream&           m_rTGA;

    BitmapWriteAccess*  mpAcc;
    TGAFileHeader*      mpFileHeader;
    TGAFileFooter*      mpFileFooter;
    TGAExtension*       mpExtension;
    sal_uInt32*             mpColorMap;

    sal_Bool                mbStatus;

    sal_uLong               mnTGAVersion;       // Enhanced TGA is defined as Version 2.0
    sal_uInt16              mnDestBitDepth;
    sal_Bool                mbIndexing;         // sal_True if source contains indexing color values
    sal_Bool                mbEncoding;         // sal_True if source is compressed

    sal_Bool                ImplReadHeader();
    sal_Bool                ImplReadPalette();
    sal_Bool                ImplReadBody();

public:
                        TGAReader(SvStream &rTGA);
                        ~TGAReader();
    sal_Bool                ReadTGA(Graphic &rGraphic);
};

//=================== Methoden von TGAReader ==============================

TGAReader::TGAReader(SvStream &rTGA)
    : m_rTGA(rTGA)
    , mpAcc(NULL)
    , mpFileHeader(NULL)
    , mpFileFooter(NULL)
    , mpExtension(NULL)
    , mpColorMap(NULL)
    , mbStatus(sal_True)
    , mnTGAVersion(1)
    , mbIndexing(sal_False)
    , mbEncoding(sal_False)
{
}

TGAReader::~TGAReader()
{
    delete[] mpColorMap;
    delete mpFileHeader;
    delete mpExtension;
    delete mpFileFooter;
}

// -------------------------------------------------------------------------------------------

sal_Bool TGAReader::ReadTGA(Graphic & rGraphic)
{
    if ( m_rTGA.GetError() )
        return sal_False;

    m_rTGA.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    // Kopf einlesen:

    if ( !m_rTGA.GetError() )
    {
        mbStatus = ImplReadHeader();
        if ( mbStatus )
        {
            Bitmap              aBitmap;

            aBitmap = Bitmap( Size( mpFileHeader->nImageWidth, mpFileHeader->nImageHeight ), mnDestBitDepth );
            mpAcc = aBitmap.AcquireWriteAccess();
            if ( mpAcc )
            {
                if ( mbIndexing )
                    mbStatus = ImplReadPalette();
                if ( mbStatus )
                    mbStatus = ImplReadBody();
            }
            else
                mbStatus = sal_False;

            if ( mpAcc )
                aBitmap.ReleaseAccess ( mpAcc), mpAcc = NULL;

            if ( mbStatus )
                rGraphic = aBitmap;
        }
    }
    return mbStatus;
}

// -------------------------------------------------------------------------------------------

sal_Bool TGAReader::ImplReadHeader()
{
    mpFileHeader = new TGAFileHeader;
    if ( mpFileHeader == NULL )
        return sal_False;

    m_rTGA >> mpFileHeader->nImageIDLength >> mpFileHeader->nColorMapType >> mpFileHeader->nImageType >>
        mpFileHeader->nColorMapFirstEntryIndex >> mpFileHeader->nColorMapLength >> mpFileHeader->nColorMapEntrySize >>
            mpFileHeader->nColorMapXOrigin >> mpFileHeader->nColorMapYOrigin >> mpFileHeader->nImageWidth >>
                mpFileHeader->nImageHeight >> mpFileHeader->nPixelDepth >> mpFileHeader->nImageDescriptor;

    if ( mpFileHeader->nColorMapType > 1 )
        return sal_False;
    if ( mpFileHeader->nColorMapType == 1 )
        mbIndexing = sal_True;

    // first we want to get the version
    mpFileFooter = new TGAFileFooter;       // read the TGA-File-Footer to determine whether
    if ( mpFileFooter )                     // we got an old TGA format or the new one
    {
        sal_uLong nCurStreamPos = m_rTGA.Tell();
        m_rTGA.Seek( STREAM_SEEK_TO_END );
        sal_uLong nTemp = m_rTGA.Tell();
        m_rTGA.Seek( nTemp - SizeOfTGAFileFooter );

        m_rTGA >> mpFileFooter->nExtensionFileOffset >> mpFileFooter->nDeveloperDirectoryOffset >>
            mpFileFooter->nSignature[0] >> mpFileFooter->nSignature[1] >> mpFileFooter->nSignature[2] >>
                mpFileFooter->nSignature[3] >> mpFileFooter->nPadByte >> mpFileFooter->nStringTerminator;

        // check for sal_True, VISI, ON-X, FILE in the signatures
        if ( mpFileFooter->nSignature[ 0 ] == (('T'<<24)|('R'<<16)|('U'<<8)|'E') &&
             mpFileFooter->nSignature[ 1 ] == (('V'<<24)|('I'<<16)|('S'<<8)|'I') &&
             mpFileFooter->nSignature[ 2 ] == (('O'<<24)|('N'<<16)|('-'<<8)|'X') &&
             mpFileFooter->nSignature[ 3 ] == (('F'<<24)|('I'<<16)|('L'<<8)|'E') )
        {
            mpExtension = new TGAExtension;
            if ( mpExtension )
            {
                m_rTGA.Seek( mpFileFooter->nExtensionFileOffset );
                m_rTGA >> mpExtension->nExtensionSize;
                if ( mpExtension->nExtensionSize >= SizeOfTGAExtension )
                {
                    mnTGAVersion = 2;

                    m_rTGA.Read( mpExtension->sAuthorName, 41 );
                    m_rTGA.Read( mpExtension->sAuthorComment, 324 );
                    m_rTGA.Read( mpExtension->sDateTimeStamp, 12 );
                    m_rTGA.Read( mpExtension->sJobNameID, 12 );
                    m_rTGA >> mpExtension->sJobNameID[ 0 ] >> mpExtension->sJobNameID[ 1 ] >> mpExtension->sJobNameID[ 2 ];
                    m_rTGA.Read( mpExtension->sSoftwareID, 41 );
                    m_rTGA >> mpExtension->nSoftwareVersionNumber >> mpExtension->nSoftwareVersionLetter
                        >> mpExtension->nKeyColor >> mpExtension->nPixelAspectRatioNumerator
                            >> mpExtension->nPixelAspectRatioDeNumerator >> mpExtension->nGammaValueNumerator
                                >> mpExtension->nGammaValueDeNumerator >> mpExtension->nColorCorrectionOffset
                                    >> mpExtension->nPostageStampOffset >> mpExtension->nScanLineOffset
                                        >> mpExtension->nAttributesType;

                }
            }
        }
        m_rTGA.Seek( nCurStreamPos );
    }

    //  using the TGA file specification this was the correct form but adobe photoshop sets nImageDescriptor
    //  equal to nPixelDepth
    //  mnDestBitDepth = mpFileHeader->nPixelDepth - ( mpFileHeader->nImageDescriptor & 0xf );
    mnDestBitDepth = mpFileHeader->nPixelDepth;

    if ( mnDestBitDepth == 8 )                  // this is a patch for grayscale pictures not including a palette
        mbIndexing = sal_True;

    if ( mnDestBitDepth > 32 )                  // maybe the pixeldepth is invalid
        return sal_False;
    else if ( mnDestBitDepth > 8 )
        mnDestBitDepth = 24;
    else if ( mnDestBitDepth > 4 )
        mnDestBitDepth = 8;
    else if ( mnDestBitDepth > 2 )
        mnDestBitDepth = 4;

    if ( !mbIndexing && ( mnDestBitDepth < 15 ) )
        return sal_False;

    switch ( mpFileHeader->nImageType )
    {
        case 9  :                               // encoding for colortype 9, 10, 11
        case 10 :
        case 11 :
            mbEncoding = sal_True;
            break;
    };

    if ( mpFileHeader->nImageIDLength )         // skip the Image ID
        m_rTGA.SeekRel( mpFileHeader->nImageIDLength );

    return mbStatus;
}

// -------------------------------------------------------------------------------------------

sal_Bool TGAReader::ImplReadBody()
{

    sal_uInt16  nXCount, nYCount, nRGB16;
    sal_uInt8   nRed, nGreen, nBlue, nRunCount, nDummy, nDepth;

    // this four variables match the image direction
    long    nY, nYAdd, nX, nXAdd, nXStart;

    nX = nXStart = nY = 0;
    nXCount = nYCount = 0;
    nYAdd = nXAdd = 1;

    if ( mpFileHeader->nImageDescriptor & 0x10 )
    {
        nX = nXStart = mpFileHeader->nImageWidth - 1;
        nXAdd -= 2;
    }

    if ( !(mpFileHeader->nImageDescriptor & 0x20 ) )
    {
        nY = mpFileHeader->nImageHeight - 1;
        nYAdd -=2;
    }

    nDepth = mpFileHeader->nPixelDepth;

    if ( mbEncoding )
    {
        if ( mbIndexing )
        {
            switch( nDepth )
            {
                // 16 bit encoding + indexing
                case 16 :
                    while ( nYCount < mpFileHeader->nImageHeight )
                    {
                        m_rTGA >> nRunCount;
                        if ( nRunCount & 0x80 )     // a run length packet
                        {
                            m_rTGA >> nRGB16;
                            if ( nRGB16 >= mpFileHeader->nColorMapLength )
                                return sal_False;
                            nRed = (sal_uInt8)( mpColorMap[ nRGB16 ] >> 16 );
                            nGreen = (sal_uInt8)( mpColorMap[ nRGB16 ] >> 8 );
                            nBlue = (sal_uInt8)( mpColorMap[ nRGB16 ] );
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {
                                mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                                nX += nXAdd;
                                nXCount++;
                                if ( nXCount == mpFileHeader->nImageWidth )
                                {
                                    nX = nXStart;
                                    nXCount = 0;
                                    nY += nYAdd;
                                    nYCount++;

                                    if( nYCount >= mpFileHeader->nImageHeight )
                                        break;
                                }
                            }
                        }
                        else                        // a raw packet
                        {
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {
                                m_rTGA >> nRGB16;
                                if ( nRGB16 >= mpFileHeader->nColorMapLength )
                                    return sal_False;
                                nRed = (sal_uInt8)( mpColorMap[ nRGB16 ] >> 16 );
                                nGreen = (sal_uInt8)( mpColorMap[ nRGB16 ] >> 8 );
                                nBlue = (sal_uInt8)( mpColorMap[ nRGB16 ] );
                                mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                                nX += nXAdd;
                                nXCount++;
                                if ( nXCount == mpFileHeader->nImageWidth )
                                {
                                    nX = nXStart;
                                    nXCount = 0;
                                    nY += nYAdd;
                                    nYCount++;

                                    if( nYCount >= mpFileHeader->nImageHeight )
                                        break;
                                }
                            }
                        }
                    }
                    break;

                // 8 bit encoding + indexing
                case 8 :
                    while ( nYCount < mpFileHeader->nImageHeight )
                    {
                        m_rTGA >> nRunCount;
                        if ( nRunCount & 0x80 )     // a run length packet
                        {
                            m_rTGA >> nDummy;
                            if ( nDummy >= mpFileHeader->nColorMapLength )
                                return sal_False;
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {
                                mpAcc->SetPixel( nY, nX, (sal_uInt8)nDummy );
                                nX += nXAdd;
                                nXCount++;
                                if ( nXCount == mpFileHeader->nImageWidth )
                                {
                                    nX = nXStart;
                                    nXCount = 0;
                                    nY += nYAdd;
                                    nYCount++;

                                    if( nYCount >= mpFileHeader->nImageHeight )
                                        break;
                                }
                            }
                        }
                        else                        // a raw packet
                        {
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {

                                m_rTGA >> nDummy;
                                if ( nDummy >= mpFileHeader->nColorMapLength )
                                    return sal_False;
                                mpAcc->SetPixel( nY, nX, (sal_uInt8)nDummy );
                                nX += nXAdd;
                                nXCount++;
                                if ( nXCount == mpFileHeader->nImageWidth )
                                {
                                    nX = nXStart;
                                    nXCount = 0;
                                    nY += nYAdd;
                                    nYCount++;

                                    if( nYCount >= mpFileHeader->nImageHeight )
                                        break;
                                }
                            }
                        }
                    }
                    break;
                default:
                    return sal_False;
            }
        }
        else
        {
            switch( nDepth )
            {
                // 32 bit transparent true color encoding
                case 32 :
                    {
                        while ( nYCount < mpFileHeader->nImageHeight )
                        {
                            m_rTGA >> nRunCount;
                            if ( nRunCount & 0x80 )     // a run length packet
                            {
                                m_rTGA >> nBlue >> nGreen >> nRed >> nDummy;
                                for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                                {
                                    mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                                    nX += nXAdd;
                                    nXCount++;
                                    if ( nXCount == mpFileHeader->nImageWidth )
                                    {
                                        nX = nXStart;
                                        nXCount = 0;
                                        nY += nYAdd;
                                        nYCount++;

                                        if( nYCount >= mpFileHeader->nImageHeight )
                                            break;
                                    }
                                }
                            }
                            else                        // a raw packet
                            {
                                for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                                {
                                    m_rTGA >> nBlue >> nGreen >> nRed >> nDummy;
                                    mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                                    nX += nXAdd;
                                    nXCount++;
                                    if ( nXCount == mpFileHeader->nImageWidth )
                                    {
                                        nX = nXStart;
                                        nXCount = 0;
                                        nY += nYAdd;
                                        nYCount++;

                                        if( nYCount >= mpFileHeader->nImageHeight )
                                            break;
                                    }
                                }
                            }
                        }
                    }
                    break;

                // 24 bit true color encoding
                case 24 :
                    while ( nYCount < mpFileHeader->nImageHeight )
                    {
                        m_rTGA >> nRunCount;
                        if ( nRunCount & 0x80 )     // a run length packet
                        {
                            m_rTGA >> nBlue >> nGreen >> nRed;
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {
                                mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                                nX += nXAdd;
                                nXCount++;
                                if ( nXCount == mpFileHeader->nImageWidth )
                                {
                                    nX = nXStart;
                                    nXCount = 0;
                                    nY += nYAdd;
                                    nYCount++;

                                    if( nYCount >= mpFileHeader->nImageHeight )
                                        break;
                                }
                            }
                        }
                        else                        // a raw packet
                        {
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {
                                m_rTGA >> nBlue >> nGreen >> nRed;
                                mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                                nX += nXAdd;
                                nXCount++;
                                if ( nXCount == mpFileHeader->nImageWidth )
                                {
                                    nX = nXStart;
                                    nXCount = 0;
                                    nY += nYAdd;
                                    nYCount++;

                                    if( nYCount >= mpFileHeader->nImageHeight )
                                        break;
                                }
                            }
                        }
                    }
                    break;

                // 16 bit true color encoding
                case 16 :
                    while ( nYCount < mpFileHeader->nImageHeight )
                    {
                        m_rTGA >> nRunCount;
                        if ( nRunCount & 0x80 )     // a run length packet
                        {
                            m_rTGA >> nRGB16;
                            nRed = (sal_uInt8)( nRGB16 >> 7 ) & 0xf8;
                            nGreen = (sal_uInt8)( nRGB16 >> 2 ) & 0xf8;
                            nBlue = (sal_uInt8)( nRGB16 << 3 ) & 0xf8;
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {
                                mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                                nX += nXAdd;
                                nXCount++;
                                if ( nXCount == mpFileHeader->nImageWidth )
                                {
                                    nX = nXStart;
                                    nXCount = 0;
                                    nY += nYAdd;
                                    nYCount++;

                                    if( nYCount >= mpFileHeader->nImageHeight )
                                        break;
                                }
                            }
                        }
                        else                        // a raw packet
                        {
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {
                                m_rTGA >> nRGB16;
                                nRed = (sal_uInt8)( nRGB16 >> 7 ) & 0xf8;
                                nGreen = (sal_uInt8)( nRGB16 >> 2 ) & 0xf8;
                                nBlue = (sal_uInt8)( nRGB16 << 3 ) & 0xf8;
                                mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                                nX += nXAdd;
                                nXCount++;
                                if ( nXCount == mpFileHeader->nImageWidth )
                                {
                                    nX = nXStart;
                                    nXCount = 0;
                                    nY += nYAdd;
                                    nYCount++;

                                    if( nYCount >= mpFileHeader->nImageHeight )
                                        break;
                                }
                            }
                        }
                    }
                    break;

                default:
                    return sal_False;
            }
        }
    }
    else
    {
        for ( nYCount = 0; nYCount < mpFileHeader->nImageHeight; nYCount++, nY += nYAdd )
        {
            nX = nXStart;
            nXCount = 0;

            if ( mbIndexing )
            {
                switch( nDepth )
                {
                    // 16 bit indexing
                    case 16 :
                        for (;nXCount < mpFileHeader->nImageWidth; nXCount++, nX += nXAdd )
                        {
                            m_rTGA >> nRGB16;
                            if ( nRGB16 >= mpFileHeader->nColorMapLength )
                                return sal_False;
                            nRed = (sal_uInt8)( mpColorMap[ nRGB16 ] >> 16 );
                            nGreen = (sal_uInt8)( mpColorMap[ nRGB16 ] >> 8 );
                            nBlue = (sal_uInt8)( mpColorMap[ nRGB16 ] );
                            mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                        }
                        break;

                    // 8 bit indexing
                    case 8 :
                        for (;nXCount < mpFileHeader->nImageWidth; nXCount++, nX += nXAdd )
                        {
                            m_rTGA >> nDummy;
                            if ( nDummy >= mpFileHeader->nColorMapLength )
                                return sal_False;
                            mpAcc->SetPixel( nY, nX, (sal_uInt8)nDummy );
                        }
                        break;
                    default:
                        return sal_False;
                }
            }
            else
            {
                switch( nDepth )
                {
                    // 32 bit true color
                    case 32 :
                        {
                            for (;nXCount < mpFileHeader->nImageWidth; nXCount++, nX += nXAdd )
                            {
                                m_rTGA >> nBlue >> nGreen >> nRed >> nDummy;
                                mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                            }
                        }
                        break;

                    // 24 bit true color
                    case 24 :
                        for (;nXCount < mpFileHeader->nImageWidth; nXCount++, nX += nXAdd )
                        {
                            m_rTGA >> nBlue >> nGreen >> nRed;
                            mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                        }
                        break;

                    // 16 bit true color
                    case 16 :
                        for (;nXCount < mpFileHeader->nImageWidth; nXCount++, nX += nXAdd )
                        {
                            m_rTGA >> nRGB16;
                            nRed = (sal_uInt8)( nRGB16 >> 7 ) & 0xf8;
                            nGreen = (sal_uInt8)( nRGB16 >> 2 ) & 0xf8;
                            nBlue = (sal_uInt8)( nRGB16 << 3 ) & 0xf8;
                            mpAcc->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                        }
                        break;
                    default:
                        return sal_False;
                }
            }
        }
    }
    return mbStatus;
}

// -------------------------------------------------------------------------------------------

sal_Bool TGAReader::ImplReadPalette()
{
    if ( mbIndexing )                           // read the colormap
    {
        sal_uInt16 nColors = mpFileHeader->nColorMapLength;

        if ( !nColors )                             // colors == 0 ? -> we will build a grayscale palette
        {
            if ( mpFileHeader->nPixelDepth != 8 )
                return sal_False;
            nColors = 256;
            mpFileHeader->nColorMapLength = 256;
            mpFileHeader->nColorMapEntrySize = 0x3f;    // patch for the following switch routine
        }
        mpColorMap = new sal_uInt32[ nColors ];     // we will always index dwords
        if ( mpColorMap == sal_False )
            return sal_False;                       // out of memory %&!$&/!"�$

        switch( mpFileHeader->nColorMapEntrySize )
        {
            case 0x3f :
                {
                    for ( sal_uLong i = 0; i < nColors; i++ )
                    {
                        mpColorMap[ i ] = ( i << 16 ) + ( i << 8 ) + i;
                    }
                }
                break;

            case 32 :
                m_rTGA.Read( mpColorMap, 4 * nColors );
                break;

            case 24 :
                {
                    for ( sal_uLong i = 0; i < nColors; i++ )
                    {
                        m_rTGA.Read( &mpColorMap[ i ], 3 );
                    }
                }
                break;

            case 15 :
            case 16 :
                {
                    for ( sal_uLong i = 0; i < nColors; i++ )
                    {
                        sal_uInt16 nTemp;
                        m_rTGA >> nTemp;
                        mpColorMap[ i ] = ( ( nTemp & 0x7c00 ) << 9 ) + ( ( nTemp & 0x01e0 ) << 6 ) +
                            ( ( nTemp & 0x1f ) << 3 );
                    }
                }
                break;

            default :
                return sal_False;
        }
        if ( mnDestBitDepth <= 8 )
        {
            sal_uInt16 nDestColors = ( 1 << mnDestBitDepth );
            if ( nColors > nDestColors )
                return sal_False;

            mpAcc->SetPaletteEntryCount( nColors );
            for ( sal_uInt16 i = 0; i < nColors; i++ )
            {
                mpAcc->SetPaletteColor( i, Color( (sal_uInt8)( mpColorMap[ i ] >> 16 ),
                    (sal_uInt8)( mpColorMap[ i ] >> 8 ), (sal_uInt8)(mpColorMap[ i ] ) ) );
            }
        }
    }

    return mbStatus;
}

//================== GraphicImport - die exportierte Funktion ================

#ifdef DISABLE_DYNLOADING
#define GraphicImport itgGraphicImport
#endif

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool __LOADONCALLAPI
GraphicImport(SvStream & rStream, Graphic & rGraphic, FilterConfigItem*, sal_Bool)
{
    TGAReader aTGAReader(rStream);

    return aTGAReader.ReadTGA(rGraphic);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
