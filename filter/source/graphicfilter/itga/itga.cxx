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


#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <vcl/BitmapTools.hxx>
#include <memory>

class FilterConfigItem;

//============================ TGAReader ==================================

namespace {

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
    char            sAuthorName[41];
    char            sAuthorComment[324];
    char            sDateTimeStamp[12];
    char            sJobNameID[41];
    char            sSoftwareID[41];
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

    std::unique_ptr<vcl::bitmap::RawBitmap> mpBitmap;
    std::vector<Color>  mvPalette;
    std::unique_ptr<TGAFileHeader>
                        mpFileHeader;
    std::unique_ptr<TGAFileFooter>
                        mpFileFooter;
    std::unique_ptr<TGAExtension>
                        mpExtension;
    std::unique_ptr<sal_uInt32[]>
                        mpColorMap;

    bool                mbStatus;

    sal_uInt8           mnTGAVersion;       // Enhanced TGA is defined as Version 2.0
    sal_uInt16          mnDestBitDepth;
    bool                mbIndexing;         // sal_True if source contains indexing color values
    bool                mbEncoding;         // sal_True if source is compressed

    bool                ImplReadHeader();
    bool                ImplReadPalette();
    bool                ImplReadBody();

public:
    explicit TGAReader(SvStream &rTGA);
    bool                ReadTGA(Graphic &rGraphic);
};

}

//=================== Methods of TGAReader ==============================

TGAReader::TGAReader(SvStream &rTGA)
    : m_rTGA(rTGA)
    , mbStatus(true)
    , mnTGAVersion(1)
    , mnDestBitDepth(8)
    , mbIndexing(false)
    , mbEncoding(false)
{
}

bool TGAReader::ReadTGA(Graphic & rGraphic)
{
    if ( m_rTGA.GetError() )
        return false;

    m_rTGA.SetEndian( SvStreamEndian::LITTLE );

    // Kopf einlesen:

    if ( !m_rTGA.GetError() )
    {
        mbStatus = ImplReadHeader();
        if (mbStatus)
            mbStatus = mpFileHeader->nImageWidth && mpFileHeader->nImageHeight;
        if (mbStatus)
        {
            sal_Size nSize = mpFileHeader->nImageWidth;
            nSize *= mpFileHeader->nImageHeight;
            if (nSize > SAL_MAX_INT32/2/3)
                return false;

            mpBitmap.reset( new vcl::bitmap::RawBitmap( Size( mpFileHeader->nImageWidth, mpFileHeader->nImageHeight ), 24 ) );
            if ( mbIndexing )
                mbStatus = ImplReadPalette();
            if ( mbStatus )
                mbStatus = ImplReadBody();

            if ( mbStatus )
                rGraphic = vcl::bitmap::CreateFromData(std::move(*mpBitmap));
        }
    }
    return mbStatus;
}


bool TGAReader::ImplReadHeader()
{
    mpFileHeader.reset( new TGAFileHeader );

    m_rTGA.ReadUChar( mpFileHeader->nImageIDLength ).ReadUChar( mpFileHeader->nColorMapType ).ReadUChar( mpFileHeader->nImageType ).        ReadUInt16( mpFileHeader->nColorMapFirstEntryIndex ).ReadUInt16( mpFileHeader->nColorMapLength ).ReadUChar( mpFileHeader->nColorMapEntrySize ).            ReadUInt16( mpFileHeader->nColorMapXOrigin ).ReadUInt16( mpFileHeader->nColorMapYOrigin ).ReadUInt16( mpFileHeader->nImageWidth ).                ReadUInt16( mpFileHeader->nImageHeight ).ReadUChar( mpFileHeader->nPixelDepth ).ReadUChar( mpFileHeader->nImageDescriptor );

    if ( !m_rTGA.good())
        return false;

    if ( mpFileHeader->nColorMapType > 1 )
        return false;
    if ( mpFileHeader->nColorMapType == 1 )
        mbIndexing = true;

    // first we want to get the version
    mpFileFooter.reset( new TGAFileFooter );  // read the TGA-File-Footer to determine whether
                                              // we got an old TGA format or the new one

    sal_uInt64 nCurStreamPos = m_rTGA.Tell();
    m_rTGA.Seek( STREAM_SEEK_TO_END );
    sal_uInt64 nTemp = m_rTGA.Tell();
    m_rTGA.Seek( nTemp - SizeOfTGAFileFooter );

    m_rTGA.ReadUInt32( mpFileFooter->nExtensionFileOffset ).ReadUInt32( mpFileFooter->nDeveloperDirectoryOffset ).            ReadUInt32( mpFileFooter->nSignature[0] ).ReadUInt32( mpFileFooter->nSignature[1] ).ReadUInt32( mpFileFooter->nSignature[2] ).                ReadUInt32( mpFileFooter->nSignature[3] ).ReadUChar( mpFileFooter->nPadByte ).ReadUChar( mpFileFooter->nStringTerminator );


    if ( !m_rTGA.good())
        return false;

    // check for sal_True, VISI, ON-X, FILE in the signatures
    if ( mpFileFooter->nSignature[ 0 ] == (('T'<<24)|('R'<<16)|('U'<<8)|'E') &&
         mpFileFooter->nSignature[ 1 ] == (('V'<<24)|('I'<<16)|('S'<<8)|'I') &&
         mpFileFooter->nSignature[ 2 ] == (('O'<<24)|('N'<<16)|('-'<<8)|'X') &&
         mpFileFooter->nSignature[ 3 ] == (('F'<<24)|('I'<<16)|('L'<<8)|'E') )
    {
        mpExtension.reset( new TGAExtension );

        m_rTGA.Seek( mpFileFooter->nExtensionFileOffset );
        m_rTGA.ReadUInt16( mpExtension->nExtensionSize );
        if ( !m_rTGA.good())
            return false;
        if ( mpExtension->nExtensionSize >= SizeOfTGAExtension )
        {
            mnTGAVersion = 2;

            m_rTGA.ReadBytes(mpExtension->sAuthorName, 41);
            m_rTGA.ReadBytes(mpExtension->sAuthorComment, 324);
            m_rTGA.ReadBytes(mpExtension->sDateTimeStamp, 12);
            m_rTGA.ReadBytes(mpExtension->sJobNameID, 12);
            m_rTGA.ReadChar( mpExtension->sJobNameID[ 0 ] ).ReadChar( mpExtension->sJobNameID[ 1 ] ).ReadChar( mpExtension->sJobNameID[ 2 ] );
            m_rTGA.ReadBytes(mpExtension->sSoftwareID, 41);
            m_rTGA.ReadUInt16( mpExtension->nSoftwareVersionNumber ).ReadUChar( mpExtension->nSoftwareVersionLetter )
               .ReadUInt32( mpExtension->nKeyColor ).ReadUInt16( mpExtension->nPixelAspectRatioNumerator )
                   .ReadUInt16( mpExtension->nPixelAspectRatioDeNumerator ).ReadUInt16( mpExtension->nGammaValueNumerator )
                       .ReadUInt16( mpExtension->nGammaValueDeNumerator ).ReadUInt32( mpExtension->nColorCorrectionOffset )
                           .ReadUInt32( mpExtension->nPostageStampOffset ).ReadUInt32( mpExtension->nScanLineOffset )
                               .ReadUChar( mpExtension->nAttributesType );

            if ( !m_rTGA.good())
                return false;
        }
    }
    m_rTGA.Seek( nCurStreamPos );

    //  using the TGA file specification this was the correct form but adobe photoshop sets nImageDescriptor
    //  equal to nPixelDepth
    //  mnDestBitDepth = mpFileHeader->nPixelDepth - ( mpFileHeader->nImageDescriptor & 0xf );
    mnDestBitDepth = mpFileHeader->nPixelDepth;

    if ( mnDestBitDepth == 8 )                  // this is a patch for grayscale pictures not including a palette
        mbIndexing = true;

    if ( mnDestBitDepth > 32 )                  // maybe the pixeldepth is invalid
        return false;
    else if ( mnDestBitDepth > 8 )
        mnDestBitDepth = 24;
    else if ( mnDestBitDepth > 4 )
        mnDestBitDepth = 8;
    else if ( mnDestBitDepth > 2 )
        mnDestBitDepth = 4;

    if ( !mbIndexing && ( mnDestBitDepth < 15 ) )
        return false;

    switch ( mpFileHeader->nImageType )
    {
        case 9  :                               // encoding for colortype 9, 10, 11
        case 10 :
        case 11 :
            mbEncoding = true;
            break;
    }

    if ( mpFileHeader->nImageIDLength )         // skip the Image ID
        m_rTGA.SeekRel( mpFileHeader->nImageIDLength );

    return mbStatus;
}


bool TGAReader::ImplReadBody()
{

    sal_uInt16  nXCount, nYCount, nRGB16;
    sal_uInt8   nRed, nGreen, nBlue, nRunCount, nDummy, nDepth;

    // this four variables match the image direction
    tools::Long    nY, nYAdd, nX, nXAdd, nXStart;

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
                        m_rTGA.ReadUChar( nRunCount );
                        if ( !m_rTGA.good())
                            return false;
                        if ( nRunCount & 0x80 )     // a run length packet
                        {
                            m_rTGA.ReadUInt16( nRGB16 );
                            if ( nRGB16 >= mpFileHeader->nColorMapLength )
                                return false;
                            nRed = static_cast<sal_uInt8>( mpColorMap[ nRGB16 ] >> 16 );
                            nGreen = static_cast<sal_uInt8>( mpColorMap[ nRGB16 ] >> 8 );
                            nBlue = static_cast<sal_uInt8>( mpColorMap[ nRGB16 ] );
                            if ( !m_rTGA.good())
                                return false;
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {
                                mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
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
                                m_rTGA.ReadUInt16( nRGB16 );
                                if ( !m_rTGA.good())
                                    return false;
                                if ( nRGB16 >= mpFileHeader->nColorMapLength )
                                    return false;
                                nRed = static_cast<sal_uInt8>( mpColorMap[ nRGB16 ] >> 16 );
                                nGreen = static_cast<sal_uInt8>( mpColorMap[ nRGB16 ] >> 8 );
                                nBlue = static_cast<sal_uInt8>( mpColorMap[ nRGB16 ] );
                                if ( !m_rTGA.good())
                                    return false;
                                mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
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
                        m_rTGA.ReadUChar( nRunCount );
                        if ( !m_rTGA.good())
                            return false;
                        if ( nRunCount & 0x80 )     // a run length packet
                        {
                            m_rTGA.ReadUChar( nDummy );
                            if ( !m_rTGA.good())
                                return false;
                            if ( nDummy >= mpFileHeader->nColorMapLength )
                                return false;
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {
                                mpBitmap->SetPixel( nY, nX, mvPalette[nDummy] );
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

                                m_rTGA.ReadUChar( nDummy );
                                if ( !m_rTGA.good())
                                    return false;
                                if ( nDummy >= mpFileHeader->nColorMapLength )
                                    return false;
                                mpBitmap->SetPixel( nY, nX, mvPalette[nDummy] );
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
                    return false;
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
                            m_rTGA.ReadUChar( nRunCount );
                            if ( !m_rTGA.good())
                                return false;
                            if ( nRunCount & 0x80 )     // a run length packet
                            {
                                m_rTGA.ReadUChar( nBlue ).ReadUChar( nGreen ).ReadUChar( nRed ).ReadUChar( nDummy );
                                if ( !m_rTGA.good())
                                    return false;
                                for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                                {
                                    mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
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
                                    m_rTGA.ReadUChar( nBlue ).ReadUChar( nGreen ).ReadUChar( nRed ).ReadUChar( nDummy );
                                    if ( !m_rTGA.good())
                                        return false;
                                    mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
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
                        m_rTGA.ReadUChar( nRunCount );
                        if ( !m_rTGA.good())
                            return false;
                        if ( nRunCount & 0x80 )     // a run length packet
                        {
                            m_rTGA.ReadUChar( nBlue ).ReadUChar( nGreen ).ReadUChar( nRed );
                            if ( !m_rTGA.good())
                                return false;
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {
                                mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
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
                                m_rTGA.ReadUChar( nBlue ).ReadUChar( nGreen ).ReadUChar( nRed );
                                if ( !m_rTGA.good())
                                    return false;
                                mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
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
                        m_rTGA.ReadUChar( nRunCount );
                        if ( !m_rTGA.good())
                            return false;
                        if ( nRunCount & 0x80 )     // a run length packet
                        {
                            m_rTGA.ReadUInt16( nRGB16 );
                            if ( !m_rTGA.good())
                                return false;
                            nRed = static_cast<sal_uInt8>( nRGB16 >> 7 ) & 0xf8;
                            nGreen = static_cast<sal_uInt8>( nRGB16 >> 2 ) & 0xf8;
                            nBlue = static_cast<sal_uInt8>( nRGB16 << 3 ) & 0xf8;
                            for ( sal_uInt16 i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                            {
                                mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
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
                                m_rTGA.ReadUInt16( nRGB16 );
                                if ( !m_rTGA.good())
                                    return false;
                                nRed = static_cast<sal_uInt8>( nRGB16 >> 7 ) & 0xf8;
                                nGreen = static_cast<sal_uInt8>( nRGB16 >> 2 ) & 0xf8;
                                nBlue = static_cast<sal_uInt8>( nRGB16 << 3 ) & 0xf8;
                                mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
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
                    return false;
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
                            m_rTGA.ReadUInt16( nRGB16 );
                            if ( !m_rTGA.good())
                                return false;
                            if ( nRGB16 >= mpFileHeader->nColorMapLength )
                                return false;
                            nRed = static_cast<sal_uInt8>( mpColorMap[ nRGB16 ] >> 16 );
                            nGreen = static_cast<sal_uInt8>( mpColorMap[ nRGB16 ] >> 8 );
                            nBlue = static_cast<sal_uInt8>( mpColorMap[ nRGB16 ] );
                            mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
                        }
                        break;

                    // 8 bit indexing
                    case 8 :
                        for (;nXCount < mpFileHeader->nImageWidth; nXCount++, nX += nXAdd )
                        {
                            m_rTGA.ReadUChar( nDummy );
                            if ( !m_rTGA.good())
                                return false;
                            if ( nDummy >= mpFileHeader->nColorMapLength )
                                return false;
                            mpBitmap->SetPixel( nY, nX, Color(nDummy) );
                        }
                        break;
                    default:
                        return false;
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
                                m_rTGA.ReadUChar( nBlue ).ReadUChar( nGreen ).ReadUChar( nRed ).ReadUChar( nDummy );
                                if ( !m_rTGA.good())
                                    return false;
                                mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
                            }
                        }
                        break;

                    // 24 bit true color
                    case 24 :
                        for (;nXCount < mpFileHeader->nImageWidth; nXCount++, nX += nXAdd )
                        {
                            m_rTGA.ReadUChar( nBlue ).ReadUChar( nGreen ).ReadUChar( nRed );
                            if ( !m_rTGA.good())
                                return false;
                            mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
                        }
                        break;

                    // 16 bit true color
                    case 16 :
                        for (;nXCount < mpFileHeader->nImageWidth; nXCount++, nX += nXAdd )
                        {
                            m_rTGA.ReadUInt16( nRGB16 );
                            if ( !m_rTGA.good())
                                return false;
                            nRed = static_cast<sal_uInt8>( nRGB16 >> 7 ) & 0xf8;
                            nGreen = static_cast<sal_uInt8>( nRGB16 >> 2 ) & 0xf8;
                            nBlue = static_cast<sal_uInt8>( nRGB16 << 3 ) & 0xf8;
                            mpBitmap->SetPixel( nY, nX, Color( nRed, nGreen, nBlue ) );
                        }
                        break;
                    default:
                        return false;
                }
            }
        }
    }
    return mbStatus;
}


bool TGAReader::ImplReadPalette()
{
    if ( mbIndexing )                           // read the colormap
    {
        sal_uInt16 nColors = mpFileHeader->nColorMapLength;

        if ( !nColors )                             // colors == 0 ? -> we will build a grayscale palette
        {
            if ( mpFileHeader->nPixelDepth != 8 )
                return false;
            nColors = 256;
            mpFileHeader->nColorMapLength = 256;
            mpFileHeader->nColorMapEntrySize = 0x3f;    // patch for the following switch routine
        }
        mpColorMap.reset( new sal_uInt32[ nColors ] );     // we will always index dwords

        switch( mpFileHeader->nColorMapEntrySize )
        {
            case 0x3f :
                {
                    for (sal_uInt32 i = 0; i < nColors; ++i)
                    {
                        mpColorMap[ i ] = ( i << 16 ) + ( i << 8 ) + i;
                    }
                }
                break;

            case 32 :
                for (sal_uInt16 i = 0; i < nColors; i++)
                {
                    m_rTGA.ReadUInt32(mpColorMap[i]);
                }
                break;

            case 24 :
                {
                    for ( sal_uInt16 i = 0; i < nColors; i++ )
                    {
                        sal_uInt8 nBlue;
                        sal_uInt8 nGreen;
                        sal_uInt8 nRed;
                        m_rTGA.ReadUChar(nBlue).ReadUChar(nGreen).ReadUChar(nRed);
                        mpColorMap[i] = (nRed << 16) | (nGreen << 8) | nBlue;
                    }
                }
                break;

            case 15 :
            case 16 :
                {
                    for ( sal_uInt16 i = 0; i < nColors; i++ )
                    {
                        sal_uInt16 nTemp;
                        m_rTGA.ReadUInt16( nTemp );
                        if ( !m_rTGA.good() )
                            return false;
                        mpColorMap[ i ] = ( ( nTemp & 0x7c00 ) << 9 ) + ( ( nTemp & 0x01e0 ) << 6 ) +
                            ( ( nTemp & 0x1f ) << 3 );
                    }
                }
                break;

            default :
                return false;
        }
        if ( mnDestBitDepth <= 8 )
        {
            sal_uInt16 nDestColors = ( 1 << mnDestBitDepth );
            if ( nColors > nDestColors )
                return false;

            mvPalette.resize( nColors );
            for ( sal_uInt16 i = 0; i < nColors; i++ )
            {
                mvPalette[i] = Color( static_cast<sal_uInt8>( mpColorMap[ i ] >> 16 ),
                    static_cast<sal_uInt8>( mpColorMap[ i ] >> 8 ), static_cast<sal_uInt8>(mpColorMap[ i ] ) );
            }
        }
    }

    return mbStatus;
}

//================== GraphicImport - the exported function ================

extern "C" SAL_DLLPUBLIC_EXPORT bool
itgGraphicImport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* )
{
    TGAReader aTGAReader(rStream);

    bool bRet;
    try
    {
        bRet = aTGAReader.ReadTGA(rGraphic);
    }
    catch (const SvStreamEOFException&)
    {
        SAL_WARN("filter.tga", "EOF");
        bRet = false;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
