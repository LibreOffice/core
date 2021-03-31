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

#include <string.h>
#include <tools/stream.hxx>
#include <tools/fract.hxx>
#include <tools/urlobj.hxx>
#include <vcl/TypeSerializer.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include "graphicfilter_internal.hxx"

#define DATA_SIZE           640

GraphicDescriptor::GraphicDescriptor( const INetURLObject& rPath ) :
    pFileStm( ::utl::UcbStreamHelper::CreateStream( rPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ ).release() ),
    aPathExt( rPath.GetFileExtension().toAsciiLowerCase() ),
    bOwnStream( true )
{
    ImpConstruct();
}

GraphicDescriptor::GraphicDescriptor( SvStream& rInStream, const OUString* pPath) :
    pFileStm    ( &rInStream ),
    bOwnStream  ( false )
{
    ImpConstruct();

    if ( pPath )
    {
        INetURLObject aURL( *pPath );
        aPathExt = aURL.GetFileExtension().toAsciiLowerCase();
    }
}

GraphicDescriptor::~GraphicDescriptor()
{
    if ( bOwnStream )
        delete pFileStm;
}

bool GraphicDescriptor::Detect( bool bExtendedInfo )
{
    bool bRet = false;
    if ( pFileStm && !pFileStm->GetError() )
    {
        SvStream&      rStm = *pFileStm;
        SvStreamEndian nOldFormat = rStm.GetEndian();

        if      ( ImpDetectGIF( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectJPG( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectBMP( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectPNG( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectTIF( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectPCX( rStm ) ) bRet = true;
        else if ( ImpDetectDXF( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectMET( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectSVM( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectWMF( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectEMF( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectSVG( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectPCT( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectXBM( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectXPM( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectPBM( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectPGM( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectPPM( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectRAS( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectTGA( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectPSD( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectEPS( rStm, bExtendedInfo ) ) bRet = true;
        else if ( ImpDetectPCD( rStm, bExtendedInfo ) ) bRet = true;

        rStm.SetEndian( nOldFormat );
    }
    return bRet;
}

void GraphicDescriptor::ImpConstruct()
{
    nFormat = GraphicFileFormat::NOT;
    nBitsPerPixel = 0;
    nPlanes = 0;
    mnNumberOfImageComponents = 0;
    bIsTransparent = false;
    bIsAlpha = false;
}

bool GraphicDescriptor::ImpDetectBMP( SvStream& rStm, bool bExtendedInfo )
{
    sal_uInt16  nTemp16 = 0;
    bool    bRet = false;
    sal_Int32 nStmPos = rStm.Tell();

    rStm.SetEndian( SvStreamEndian::LITTLE );
    rStm.ReadUInt16( nTemp16 );

    // OS/2-BitmapArray
    if ( nTemp16 == 0x4142 )
    {
        rStm.SeekRel( 0x0c );
        rStm.ReadUInt16( nTemp16 );
    }

    // Bitmap
    if ( nTemp16 == 0x4d42 )
    {
        nFormat = GraphicFileFormat::BMP;
        bRet = true;

        if ( bExtendedInfo )
        {
            sal_uInt32  nTemp32;
            sal_uInt32  nCompression;

            // up to first info
            rStm.SeekRel( 0x10 );

            // Pixel width
            rStm.ReadUInt32( nTemp32 );
            aPixSize.setWidth( nTemp32 );

            // Pixel height
            rStm.ReadUInt32( nTemp32 );
            aPixSize.setHeight( nTemp32 );

            // Planes
            rStm.ReadUInt16( nTemp16 );
            nPlanes = nTemp16;

            // BitCount
            rStm.ReadUInt16( nTemp16 );
            nBitsPerPixel = nTemp16;

            // Compression
            rStm.ReadUInt32( nTemp32 );
            nCompression = nTemp32;

            // logical width
            rStm.SeekRel( 4 );
            rStm.ReadUInt32( nTemp32 );
            sal_uInt32 nXPelsPerMeter = 0;
            if ( nTemp32 )
            {
                aLogSize.setWidth( ( aPixSize.Width() * 100000 ) / nTemp32 );
                nXPelsPerMeter = nTemp32;
            }

            // logical height
            rStm.ReadUInt32( nTemp32 );
            sal_uInt32 nYPelsPerMeter = 0;
            if ( nTemp32 )
            {
                aLogSize.setHeight( ( aPixSize.Height() * 100000 ) / nTemp32 );
                nYPelsPerMeter = nTemp32;
            }

            // further validation, check for rational values
            if ( ( nBitsPerPixel > 24 ) || ( nCompression > 3 ) )
            {
                nFormat = GraphicFileFormat::NOT;
                bRet = false;
            }

            if (bRet && nXPelsPerMeter && nYPelsPerMeter)
            {
                maPreferredMapMode
                    = MapMode(MapUnit::MapMM, Point(), Fraction(1000, nXPelsPerMeter),
                              Fraction(1000, nYPelsPerMeter));

                maPreferredLogSize = Size(aPixSize.getWidth(), aPixSize.getHeight());
            }
        }
    }
    rStm.Seek( nStmPos );
    return bRet;
}

bool GraphicDescriptor::ImpDetectGIF( SvStream& rStm, bool bExtendedInfo )
{
    sal_uInt32  n32 = 0;
    bool    bRet = false;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetEndian( SvStreamEndian::LITTLE );
    rStm.ReadUInt32( n32 );

    if ( n32 == 0x38464947 )
    {
        sal_uInt16  n16 = 0;
        rStm.ReadUInt16( n16 );
        if ( ( n16 == 0x6137 ) || ( n16 == 0x6139 ) )
        {
            nFormat = GraphicFileFormat::GIF;
            bRet = true;

            if ( bExtendedInfo )
            {
                sal_uInt16 nTemp16 = 0;
                sal_uInt8  cByte = 0;

                // Pixel width
                rStm.ReadUInt16( nTemp16 );
                aPixSize.setWidth( nTemp16 );

                // Pixel height
                rStm.ReadUInt16( nTemp16 );
                aPixSize.setHeight( nTemp16 );

                // Bits/Pixel
                rStm.ReadUChar( cByte );
                nBitsPerPixel = ( ( cByte & 112 ) >> 4 ) + 1;
            }
        }
    }
    rStm.Seek( nStmPos );
    return bRet;
}

// returns the next jpeg marker, a return value of 0 represents an error
static sal_uInt8 ImpDetectJPG_GetNextMarker( SvStream& rStm )
{
    sal_uInt8 nByte;
    do
    {
        do
        {
            rStm.ReadUChar( nByte );
            if (!rStm.good())   // as 0 is not allowed as marker,
                return 0;       // we can use it as errorcode
        }
        while ( nByte != 0xff );
        do
        {
            rStm.ReadUChar( nByte );
            if (!rStm.good())
                return 0;
        }
        while( nByte == 0xff );
    }
    while( nByte == 0 );        // 0xff00 represents 0xff and not a marker,
                                // the marker detection has to be restarted.
    return nByte;
}

bool GraphicDescriptor::ImpDetectJPG( SvStream& rStm,  bool bExtendedInfo )
{
    sal_uInt32  nTemp32 = 0;
    bool    bRet = false;

    sal_Int32 nStmPos = rStm.Tell();

    rStm.SetEndian( SvStreamEndian::BIG );
    rStm.ReadUInt32( nTemp32 );

    // compare upper 24 bits
    if( 0xffd8ff00 == ( nTemp32 & 0xffffff00 ) )
    {
        nFormat = GraphicFileFormat::JPG;
        bRet = true;

        if ( bExtendedInfo )
        {
            rStm.SeekRel( -2 );

            ErrCode nError( rStm.GetError() );

            bool bScanFailure = false;
            bool bScanFinished = false;
            MapMode aMap;

            while (!bScanFailure && !bScanFinished && rStm.good())
            {
                sal_uInt8 nMarker = ImpDetectJPG_GetNextMarker( rStm );
                switch( nMarker )
                {
                    // fixed size marker, not having a two byte length parameter
                    case 0xd0 :     // RST0
                    case 0xd1 :
                    case 0xd2 :
                    case 0xd3 :
                    case 0xd4 :
                    case 0xd5 :
                    case 0xd6 :
                    case 0xd7 :     // RST7
                    case 0x01 :     // TEM
                    break;

                    case 0xd8 :     // SOI (has already been checked, there should not be a second one)
                    case 0x00 :     // marker is invalid, we should stop now
                        bScanFailure = true;
                    break;

                    case 0xd9 :     // EOI
                        bScanFinished = true;
                    break;

                    // per default we assume marker segments containing a length parameter
                    default :
                    {
                        sal_uInt16 nLength = 0;
                        rStm.ReadUInt16( nLength );

                        if ( nLength < 2 )
                            bScanFailure = true;
                        else
                        {
                            sal_uInt32 nNextMarkerPos = rStm.Tell() + nLength - 2;
                            switch( nMarker )
                            {
                                case 0xe0 : // APP0 Marker
                                {
                                    if ( nLength == 16 )
                                    {
                                        sal_Int32 nIdentifier = 0;
                                        rStm.ReadInt32( nIdentifier );
                                        if ( nIdentifier == 0x4a464946 )    // JFIF Identifier
                                        {
                                            sal_uInt8   nStringTerminator = 0;
                                            sal_uInt8   nMajorRevision = 0;
                                            sal_uInt8   nMinorRevision = 0;
                                            sal_uInt8   nUnits = 0;
                                            sal_uInt16  nHorizontalResolution = 0;
                                            sal_uInt16  nVerticalResolution = 0;
                                            sal_uInt8   nHorzThumbnailPixelCount = 0;
                                            sal_uInt8   nVertThumbnailPixelCount = 0;

                                            rStm.ReadUChar( nStringTerminator )
                                                .ReadUChar( nMajorRevision )
                                                .ReadUChar( nMinorRevision )
                                                .ReadUChar( nUnits )
                                                .ReadUInt16( nHorizontalResolution )
                                                .ReadUInt16( nVerticalResolution )
                                                .ReadUChar( nHorzThumbnailPixelCount )
                                                .ReadUChar( nVertThumbnailPixelCount );

                                            // setting the logical size
                                            if ( nUnits && nHorizontalResolution && nVerticalResolution )
                                            {
                                                aMap.SetMapUnit( nUnits == 1 ? MapUnit::MapInch : MapUnit::MapCM );
                                                aMap.SetScaleX( Fraction( 1, nHorizontalResolution ) );
                                                aMap.SetScaleY( Fraction( 1, nVerticalResolution ) );
                                                aLogSize = OutputDevice::LogicToLogic( aPixSize, aMap, MapMode( MapUnit::Map100thMM ) );
                                            }
                                        }
                                    }
                                }
                                break;

                                // Start of Frame Markers
                                case 0xc0 : // SOF0
                                case 0xc1 : // SOF1
                                case 0xc2 : // SOF2
                                case 0xc3 : // SOF3
                                case 0xc5 : // SOF5
                                case 0xc6 : // SOF6
                                case 0xc7 : // SOF7
                                case 0xc9 : // SOF9
                                case 0xca : // SOF10
                                case 0xcb : // SOF11
                                case 0xcd : // SOF13
                                case 0xce : // SOF14
                                case 0xcf : // SOF15
                                {
                                    sal_uInt8   nSamplePrecision = 0;
                                    sal_uInt16  nNumberOfLines = 0;
                                    sal_uInt16  nSamplesPerLine = 0;
                                    sal_uInt8   nNumberOfImageComponents = 0;
                                    sal_uInt8   nComponentsIdentifier = 0;
                                    sal_uInt8   nSamplingFactor = 0;
                                    sal_uInt8   nQuantizationTableDestinationSelector = 0;
                                    rStm.ReadUChar( nSamplePrecision )
                                        .ReadUInt16( nNumberOfLines )
                                        .ReadUInt16( nSamplesPerLine )
                                        .ReadUChar( nNumberOfImageComponents )
                                        .ReadUChar( nComponentsIdentifier )
                                        .ReadUChar( nSamplingFactor )
                                        .ReadUChar( nQuantizationTableDestinationSelector );
                                    mnNumberOfImageComponents = nNumberOfImageComponents;

                                    // nSamplingFactor (lower nibble: vertical,
                                    // upper nibble: horizontal) is unused

                                    aPixSize.setHeight( nNumberOfLines );
                                    aPixSize.setWidth( nSamplesPerLine );
                                    nBitsPerPixel = ( nNumberOfImageComponents == 3 ? 24 : nNumberOfImageComponents == 1 ? 8 : 0 );
                                    nPlanes = 1;

                                    if (aMap.GetMapUnit() != MapUnit::MapPixel)
                                        // We already know the DPI, but the
                                        // pixel size arrived later, so do the
                                        // conversion again.
                                        aLogSize = OutputDevice::LogicToLogic(
                                            aPixSize, aMap, MapMode(MapUnit::Map100thMM));

                                    bScanFinished = true;
                                }
                                break;
                            }
                            rStm.Seek( nNextMarkerPos );
                        }
                    }
                    break;
                }
            }
            rStm.SetError( nError );
        }
    }
    rStm.Seek( nStmPos );
    return bRet;
}

bool GraphicDescriptor::ImpDetectPCD( SvStream& rStm, bool )
{
    bool    bRet = false;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetEndian( SvStreamEndian::LITTLE );

    sal_uInt32  nTemp32 = 0;
    sal_uInt16  nTemp16 = 0;
    sal_uInt8   cByte = 0;

    rStm.SeekRel( 2048 );
    rStm.ReadUInt32( nTemp32 );
    rStm.ReadUInt16( nTemp16 );
    rStm.ReadUChar( cByte );

    if ( ( nTemp32 == 0x5f444350 ) &&
         ( nTemp16 == 0x5049 ) &&
         ( cByte == 0x49 ) )
    {
        nFormat = GraphicFileFormat::PCD;
        bRet = true;
    }
    rStm.Seek( nStmPos );
    return bRet;
}

bool GraphicDescriptor::ImpDetectPCX( SvStream& rStm )
{
    // ! Because 0x0a can be interpreted as LF too ...
    // we can't be sure that this special sign represent a PCX file only.
    // Every Ascii file is possible here :-(
    // We must detect the whole header.

    bool    bRet = false;
    sal_uInt8   cByte = 0;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetEndian( SvStreamEndian::LITTLE );
    rStm.ReadUChar( cByte );

    if ( cByte == 0x0a )
    {
        nFormat = GraphicFileFormat::PCX;

        rStm.SeekRel( 1 );

        // compression
        rStm.ReadUChar( cByte );

        bRet = (cByte==0 || cByte ==1);
        if (bRet)
        {
            sal_uInt16  nTemp16;
            sal_uInt16  nXmin;
            sal_uInt16  nXmax;
            sal_uInt16  nYmin;
            sal_uInt16  nYmax;
            sal_uInt16  nDPIx;
            sal_uInt16  nDPIy;

            // Bits/Pixel
            rStm.ReadUChar( cByte );
            nBitsPerPixel = cByte;

            // image dimensions
            rStm.ReadUInt16( nTemp16 );
            nXmin = nTemp16;
            rStm.ReadUInt16( nTemp16 );
            nYmin = nTemp16;
            rStm.ReadUInt16( nTemp16 );
            nXmax = nTemp16;
            rStm.ReadUInt16( nTemp16 );
            nYmax = nTemp16;

            aPixSize.setWidth( nXmax - nXmin + 1 );
            aPixSize.setHeight( nYmax - nYmin + 1 );

            // resolution
            rStm.ReadUInt16( nTemp16 );
            nDPIx = nTemp16;
            rStm.ReadUInt16( nTemp16 );
            nDPIy = nTemp16;

            // set logical size
            MapMode aMap( MapUnit::MapInch, Point(),
                          Fraction( 1, nDPIx ), Fraction( 1, nDPIy ) );
            aLogSize = OutputDevice::LogicToLogic( aPixSize, aMap,
                                                   MapMode( MapUnit::Map100thMM ) );

            // number of color planes
            cByte = 5; // Illegal value in case of EOF.
            rStm.SeekRel( 49 );
            rStm.ReadUChar( cByte );
            nPlanes = cByte;

            bRet = (nPlanes<=4);
        }
    }

    rStm.Seek( nStmPos );
    return bRet;
}

bool GraphicDescriptor::ImpDetectPNG( SvStream& rStm, bool bExtendedInfo )
{
    sal_uInt32  nTemp32 = 0;
    bool    bRet = false;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetEndian( SvStreamEndian::BIG );
    rStm.ReadUInt32( nTemp32 );

    if ( nTemp32 == 0x89504e47 )
    {
        rStm.ReadUInt32( nTemp32 );
        if ( nTemp32 == 0x0d0a1a0a )
        {
            nFormat = GraphicFileFormat::PNG;
            bRet = true;

            if ( bExtendedInfo )
            {
                do {
                    sal_uInt8 cByte = 0;

                    // IHDR-Chunk
                    rStm.SeekRel( 8 );

                    // width
                    rStm.ReadUInt32( nTemp32 );
                    if (!rStm.good())
                        break;
                    aPixSize.setWidth( nTemp32 );

                    // height
                    rStm.ReadUInt32( nTemp32 );
                    if (!rStm.good())
                        break;
                    aPixSize.setHeight( nTemp32 );

                    // Bits/Pixel
                    rStm.ReadUChar( cByte );
                    if (!rStm.good())
                        break;
                    nBitsPerPixel = cByte;

                    // Colour type - check whether it supports alpha values
                    sal_uInt8 cColType = 0;
                    rStm.ReadUChar( cColType );
                    if (!rStm.good())
                         break;
                    bIsAlpha = bIsTransparent = ( cColType == 4 || cColType == 6 );

                    // Planes always 1;
                    // compression always
                    nPlanes = 1;

                    sal_uInt32  nLen32 = 0;
                    nTemp32 = 0;

                    rStm.SeekRel( 7 );

                    // read up to the start of the image
                    rStm.ReadUInt32( nLen32 );
                    rStm.ReadUInt32( nTemp32 );
                    while( ( nTemp32 != 0x49444154 ) && rStm.good() )
                    {
                        if ( nTemp32 == 0x70485973 ) // physical pixel dimensions
                        {
                            sal_uLong   nXRes;
                            sal_uLong   nYRes;

                            // horizontal resolution
                            nTemp32 = 0;
                            rStm.ReadUInt32( nTemp32 );
                            nXRes = nTemp32;

                            // vertical resolution
                            nTemp32 = 0;
                            rStm.ReadUInt32( nTemp32 );
                            nYRes = nTemp32;

                            // unit
                            cByte = 0;
                            rStm.ReadUChar( cByte );

                            if ( cByte )
                            {
                                if ( nXRes )
                                    aLogSize.setWidth( (aPixSize.Width() * 100000) / nXRes );

                                if ( nYRes )
                                    aLogSize.setHeight( (aPixSize.Height() * 100000) / nYRes );
                            }

                            nLen32 -= 9;
                        }
                        else if ( nTemp32 == 0x74524e53 ) // transparency
                        {
                            bIsTransparent = true;
                            bIsAlpha = ( cColType != 0 && cColType != 2 );
                        }

                        // skip forward to next chunk
                        rStm.SeekRel( 4 + nLen32 );
                        rStm.ReadUInt32( nLen32 );
                        rStm.ReadUInt32( nTemp32 );
                    }
                } while (false);
            }
        }
    }
    rStm.Seek( nStmPos );
    return bRet;
}

bool GraphicDescriptor::ImpDetectTIF( SvStream& rStm, bool bExtendedInfo )
{
    bool    bRet = false;
    sal_uInt8   cByte1 = 0;
    sal_uInt8   cByte2 = 1;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.ReadUChar( cByte1 );
    rStm.ReadUChar( cByte2 );
    if ( cByte1 == cByte2 )
    {
        bool bDetectOk = false;

        if ( cByte1 == 0x49 )
        {
            rStm.SetEndian( SvStreamEndian::LITTLE );
            bDetectOk = true;
        }
        else if ( cByte1 == 0x4d )
        {
            rStm.SetEndian( SvStreamEndian::BIG );
            bDetectOk = true;
        }

        if ( bDetectOk )
        {
            sal_uInt16  nTemp16 = 0;

            rStm.ReadUInt16( nTemp16 );
            if ( nTemp16 == 0x2a )
            {
                nFormat = GraphicFileFormat::TIF;
                bRet = true;

                if ( bExtendedInfo )
                {
                    sal_uLong   nCount;
                    sal_uLong   nMax = DATA_SIZE - 48;
                    sal_uInt32  nTemp32 = 0;

                    // Offset of the first IFD
                    rStm.ReadUInt32( nTemp32 );
                    nCount = nTemp32 + 2;
                    rStm.SeekRel( nCount - 0x08 );

                    if ( nCount < nMax )
                    {
                        bool bOk = false;

                        // read tags till we find Tag256 ( Width )
                        // do not read more bytes than DATA_SIZE
                        rStm.ReadUInt16( nTemp16 );
                        while ( nTemp16 != 256 )
                        {
                            bOk = nCount < nMax;
                            if ( !bOk )
                            {
                                break;
                            }
                            rStm.SeekRel( 10 );
                            rStm.ReadUInt16( nTemp16 );
                            nCount += 12;
                        }

                        if ( bOk )
                        {
                            // width
                            rStm.ReadUInt16( nTemp16 );
                            rStm.SeekRel( 4 );
                            if ( nTemp16 == 3 )
                            {
                                rStm.ReadUInt16( nTemp16 );
                                aPixSize.setWidth( nTemp16 );
                                rStm.SeekRel( 2 );
                            }
                            else
                            {
                                rStm.ReadUInt32( nTemp32 );
                                aPixSize.setWidth( nTemp32 );
                            }

                            // height
                            rStm.SeekRel( 2 );
                            rStm.ReadUInt16( nTemp16 );
                            rStm.SeekRel( 4 );
                            if ( nTemp16 == 3 )
                            {
                                rStm.ReadUInt16( nTemp16 );
                                aPixSize.setHeight( nTemp16 );
                                rStm.SeekRel( 2 );
                            }
                            else
                            {
                                rStm.ReadUInt32( nTemp32 );
                                aPixSize.setHeight( nTemp32 );
                            }

                            // Bits/Pixel
                            rStm.ReadUInt16( nTemp16 );
                            if ( nTemp16 == 258 )
                            {
                                rStm.SeekRel( 6 );
                                rStm.ReadUInt16( nTemp16 );
                                nBitsPerPixel = nTemp16;
                                rStm.SeekRel( 2 );
                            }
                            else
                                rStm.SeekRel( -2 );

                            // compression
                            rStm.ReadUInt16( nTemp16 );
                            if ( nTemp16 == 259 )
                            {
                                rStm.SeekRel( 6 );
                                rStm.ReadUInt16( nTemp16 ); // compression
                                rStm.SeekRel( 2 );
                            }
                            else
                                rStm.SeekRel( -2 );
                        }
                    }
                }
            }
        }
    }
    rStm.Seek( nStmPos );
    return bRet;
}

bool GraphicDescriptor::ImpDetectXBM( SvStream&, bool )
{
    bool bRet = aPathExt.startsWith( "xbm" );
    if (bRet)
        nFormat = GraphicFileFormat::XBM;

    return bRet;
}

bool GraphicDescriptor::ImpDetectXPM( SvStream&, bool )
{
    bool bRet = aPathExt.startsWith( "xpm" );
    if (bRet)
        nFormat = GraphicFileFormat::XPM;

    return bRet;
}

bool GraphicDescriptor::ImpDetectPBM( SvStream& rStm, bool )
{
    bool bRet = false;

    // check file extension first, as this trumps the 2 ID bytes
    if ( aPathExt.startsWith( "pbm" ) )
        bRet = true;
    else
    {
        sal_Int32 nStmPos = rStm.Tell();
        sal_uInt8   nFirst = 0, nSecond = 0;
        rStm.ReadUChar( nFirst ).ReadUChar( nSecond );
        if ( nFirst == 'P' && ( ( nSecond == '1' ) || ( nSecond == '4' ) ) )
            bRet = true;
        rStm.Seek( nStmPos );
    }

    if ( bRet )
        nFormat = GraphicFileFormat::PBM;

    return bRet;
}

bool GraphicDescriptor::ImpDetectPGM( SvStream& rStm, bool )
{
    bool bRet = false;

    if ( aPathExt.startsWith( "pgm" ) )
        bRet = true;
    else
    {
        sal_uInt8 nFirst = 0, nSecond = 0;
        sal_Int32 nStmPos = rStm.Tell();
        rStm.ReadUChar( nFirst ).ReadUChar( nSecond );
        if ( nFirst == 'P' && ( ( nSecond == '2' ) || ( nSecond == '5' ) ) )
            bRet = true;
        rStm.Seek( nStmPos );
    }

    if ( bRet )
        nFormat = GraphicFileFormat::PGM;

    return bRet;
}

bool GraphicDescriptor::ImpDetectPPM( SvStream& rStm, bool )
{
    bool bRet = false;

    if ( aPathExt.startsWith( "ppm" ) )
        bRet = true;
    else
    {
        sal_uInt8   nFirst = 0, nSecond = 0;
        sal_Int32 nStmPos = rStm.Tell();
        rStm.ReadUChar( nFirst ).ReadUChar( nSecond );
        if ( nFirst == 'P' && ( ( nSecond == '3' ) || ( nSecond == '6' ) ) )
            bRet = true;
        rStm.Seek( nStmPos );
    }

    if ( bRet )
        nFormat = GraphicFileFormat::PPM;

    return bRet;
}

bool GraphicDescriptor::ImpDetectRAS( SvStream& rStm, bool )
{
    sal_uInt32 nMagicNumber = 0;
    bool bRet = false;
    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetEndian( SvStreamEndian::BIG );
    rStm.ReadUInt32( nMagicNumber );
    if ( nMagicNumber == 0x59a66a95 )
    {
        nFormat = GraphicFileFormat::RAS;
        bRet = true;
    }
    rStm.Seek( nStmPos );
    return bRet;
}

bool GraphicDescriptor::ImpDetectTGA( SvStream&, bool )
{
    bool bRet = aPathExt.startsWith( "tga" );
    if (bRet)
        nFormat = GraphicFileFormat::TGA;

    return bRet;
}

bool GraphicDescriptor::ImpDetectPSD( SvStream& rStm, bool bExtendedInfo )
{
    bool bRet = false;

    sal_uInt32  nMagicNumber = 0;
    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetEndian( SvStreamEndian::BIG );
    rStm.ReadUInt32( nMagicNumber );
    if ( nMagicNumber == 0x38425053 )
    {
        sal_uInt16 nVersion = 0;
        rStm.ReadUInt16( nVersion );
        if ( nVersion == 1 )
        {
            bRet = true;
            if ( bExtendedInfo )
            {
                sal_uInt16 nChannels = 0;
                sal_uInt32 nRows = 0;
                sal_uInt32 nColumns = 0;
                sal_uInt16 nDepth = 0;
                sal_uInt16 nMode = 0;
                rStm.SeekRel( 6 );  // Pad
                rStm.ReadUInt16( nChannels ).ReadUInt32( nRows ).ReadUInt32( nColumns ).ReadUInt16( nDepth ).ReadUInt16( nMode );
                if ( ( nDepth == 1 ) || ( nDepth == 8 ) || ( nDepth == 16 ) )
                {
                    nBitsPerPixel = ( nDepth == 16 ) ? 8 : nDepth;
                    switch ( nChannels )
                    {
                        case 4 :
                        case 3 :
                            nBitsPerPixel = 24;
                            [[fallthrough]];
                        case 2 :
                        case 1 :
                            aPixSize.setWidth( nColumns );
                            aPixSize.setHeight( nRows );
                        break;
                        default:
                            bRet = false;
                    }
                }
                else
                    bRet = false;
            }
        }
    }

    if ( bRet )
        nFormat = GraphicFileFormat::PSD;
    rStm.Seek( nStmPos );
    return bRet;
}

bool GraphicDescriptor::ImpDetectEPS( SvStream& rStm, bool )
{
    // check the EPS preview and the file extension
    sal_uInt32  nFirstLong = 0;
    sal_uInt8   nFirstBytes[20] = {};
    bool        bRet = false;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetEndian( SvStreamEndian::BIG );
    rStm.ReadUInt32( nFirstLong );
    rStm.SeekRel( -4 );
    rStm.ReadBytes( &nFirstBytes, 20 );

    if ( ( nFirstLong == 0xC5D0D3C6 ) || aPathExt.startsWith( "eps" ) ||
        ( ImplSearchEntry( nFirstBytes, reinterpret_cast<sal_uInt8 const *>("%!PS-Adobe"), 10, 10 )
            && ImplSearchEntry( &nFirstBytes[15], reinterpret_cast<sal_uInt8 const *>("EPS"), 3, 3 ) ) )
    {
        nFormat = GraphicFileFormat::EPS;
        bRet = true;
    }
    rStm.Seek( nStmPos );
    return bRet;
}

bool GraphicDescriptor::ImpDetectDXF( SvStream&, bool )
{
    bool bRet = aPathExt.startsWith( "dxf" );
    if (bRet)
        nFormat = GraphicFileFormat::DXF;

    return bRet;
}

bool GraphicDescriptor::ImpDetectMET( SvStream&, bool )
{
    bool bRet = aPathExt.startsWith( "met" );
    if (bRet)
        nFormat = GraphicFileFormat::MET;

    return bRet;
}

bool GraphicDescriptor::ImpDetectPCT( SvStream& rStm, bool )
{
    bool bRet = aPathExt.startsWith( "pct" );
    if (bRet)
        nFormat = GraphicFileFormat::PCT;
    else
    {
        sal_uInt64 const nStreamPos = rStm.Tell();
        sal_uInt64 const nStreamLen = rStm.remainingSize();
        if (isPCT(rStm, nStreamPos, nStreamLen))
        {
            bRet = true;
            nFormat = GraphicFileFormat::PCT;
        }
        rStm.Seek(nStreamPos);
    }

    return bRet;
}

bool GraphicDescriptor::ImpDetectSVM( SvStream& rStm, bool bExtendedInfo )
{
    sal_uInt32  n32 = 0;
    bool    bRet = false;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetEndian( SvStreamEndian::LITTLE );
    rStm.ReadUInt32( n32 );
    if ( n32 == 0x44475653 )
    {
        sal_uInt8 cByte = 0;
        rStm.ReadUChar( cByte );
        if ( cByte == 0x49 )
        {
            nFormat = GraphicFileFormat::SVM;
            bRet = true;

            if ( bExtendedInfo )
            {
                sal_uInt32  nTemp32;
                sal_uInt16  nTemp16;

                rStm.SeekRel( 0x04 );

                // width
                nTemp32 = 0;
                rStm.ReadUInt32( nTemp32 );
                aLogSize.setWidth( nTemp32 );

                // height
                nTemp32 = 0;
                rStm.ReadUInt32( nTemp32 );
                aLogSize.setHeight( nTemp32 );

                // read MapUnit and determine PrefSize
                nTemp16 = 0;
                rStm.ReadUInt16( nTemp16 );
                aLogSize = OutputDevice::LogicToLogic( aLogSize,
                                                       MapMode( static_cast<MapUnit>(nTemp16) ),
                                                       MapMode( MapUnit::Map100thMM ) );
            }
        }
    }
    else
    {
        rStm.SeekRel( -4 );
        n32 = 0;
        rStm.ReadUInt32( n32 );

        if( n32 == 0x4D4C4356 )
        {
            sal_uInt16 nTmp16 = 0;

            rStm.ReadUInt16( nTmp16 );

            if( nTmp16 == 0x4654 )
            {
                nFormat = GraphicFileFormat::SVM;
                bRet = true;

                if( bExtendedInfo )
                {
                    MapMode aMapMode;
                    rStm.SeekRel( 0x06 );
                    TypeSerializer aSerializer(rStm);
                    aSerializer.readMapMode(aMapMode);
                    aSerializer.readSize(aLogSize);
                    aLogSize = OutputDevice::LogicToLogic( aLogSize, aMapMode, MapMode( MapUnit::Map100thMM ) );
                }
            }
        }
    }
    rStm.Seek( nStmPos );
    return bRet;
}

bool GraphicDescriptor::ImpDetectWMF( SvStream&, bool )
{
    bool bRet = aPathExt.startsWith( "wmf" );
    if (bRet)
        nFormat = GraphicFileFormat::WMF;

    return bRet;
}

bool GraphicDescriptor::ImpDetectEMF( SvStream& rStm, bool bExtendedInfo )
{
    sal_uInt32 nRecordType = 0;
    bool bRet = false;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetEndian( SvStreamEndian::LITTLE );
    rStm.ReadUInt32( nRecordType );

    if ( nRecordType == 0x00000001 )
    {
        sal_uInt32 nHeaderSize = 0;
        sal_Int32 nBoundLeft = 0, nBoundTop = 0, nBoundRight = 0, nBoundBottom = 0;
        sal_Int32 nFrameLeft = 0, nFrameTop = 0, nFrameRight = 0, nFrameBottom = 0;
        sal_uInt32 nSignature = 0;

        rStm.ReadUInt32( nHeaderSize );
        rStm.ReadInt32( nBoundLeft );
        rStm.ReadInt32( nBoundTop );
        rStm.ReadInt32( nBoundRight );
        rStm.ReadInt32( nBoundBottom );
        rStm.ReadInt32( nFrameLeft );
        rStm.ReadInt32( nFrameTop );
        rStm.ReadInt32( nFrameRight );
        rStm.ReadInt32( nFrameBottom );
        rStm.ReadUInt32( nSignature );

        if ( nSignature == 0x464d4520 )
        {
            nFormat = GraphicFileFormat::EMF;
            bRet = true;

            if ( bExtendedInfo )
            {
                // size in pixels
                aPixSize.setWidth( nBoundRight - nBoundLeft + 1 );
                aPixSize.setHeight( nBoundBottom - nBoundTop + 1 );

                // size in 0.01mm units
                aLogSize.setWidth( nFrameRight - nFrameLeft + 1 );
                aLogSize.setHeight( nFrameBottom - nFrameTop + 1 );
            }
        }
    }

    rStm.Seek( nStmPos );
    return bRet;
}

bool GraphicDescriptor::ImpDetectSVG( SvStream& /*rStm*/, bool /*bExtendedInfo*/ )
{
    bool bRet = aPathExt.startsWith( "svg" );
    if (bRet)
        nFormat = GraphicFileFormat::SVG;

    return bRet;
}

OUString GraphicDescriptor::GetImportFormatShortName( GraphicFileFormat nFormat )
{
    const char *pKeyName = nullptr;

    switch( nFormat )
    {
        case GraphicFileFormat::BMP :   pKeyName = "bmp";   break;
        case GraphicFileFormat::GIF :   pKeyName = "gif";   break;
        case GraphicFileFormat::JPG :   pKeyName = "jpg";   break;
        case GraphicFileFormat::PCD :   pKeyName = "pcd";   break;
        case GraphicFileFormat::PCX :   pKeyName = "pcx";   break;
        case GraphicFileFormat::PNG :   pKeyName = "png";   break;
        case GraphicFileFormat::XBM :   pKeyName = "xbm";   break;
        case GraphicFileFormat::XPM :   pKeyName = "xpm";   break;
        case GraphicFileFormat::PBM :   pKeyName = "pbm";   break;
        case GraphicFileFormat::PGM :   pKeyName = "pgm";   break;
        case GraphicFileFormat::PPM :   pKeyName = "ppm";   break;
        case GraphicFileFormat::RAS :   pKeyName = "ras";   break;
        case GraphicFileFormat::TGA :   pKeyName = "tga";   break;
        case GraphicFileFormat::PSD :   pKeyName = "psd";   break;
        case GraphicFileFormat::EPS :   pKeyName = "eps";   break;
        case GraphicFileFormat::TIF :   pKeyName = "tif";   break;
        case GraphicFileFormat::DXF :   pKeyName = "dxf";   break;
        case GraphicFileFormat::MET :   pKeyName = "met";   break;
        case GraphicFileFormat::PCT :   pKeyName = "pct";   break;
        case GraphicFileFormat::SVM :   pKeyName = "svm";   break;
        case GraphicFileFormat::WMF :   pKeyName = "wmf";   break;
        case GraphicFileFormat::EMF :   pKeyName = "emf";   break;
        case GraphicFileFormat::SVG :   pKeyName = "svg";   break;
        default: assert(false);
    }

    return OUString::createFromAscii(pKeyName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
