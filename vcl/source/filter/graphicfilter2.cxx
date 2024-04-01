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
#include <tools/zcodec.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <graphic/GraphicFormatDetector.hxx>

GraphicDescriptor::GraphicDescriptor( const INetURLObject& rPath ) :
    pFileStm( ::utl::UcbStreamHelper::CreateStream( rPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ ).release() ),
    aPathExt( rPath.GetFileExtension().toAsciiLowerCase() ),
    bOwnStream( true )
{
}

GraphicDescriptor::GraphicDescriptor( SvStream& rInStream, const OUString* pPath) :
    pFileStm    ( &rInStream ),
    bOwnStream  ( false )
{
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
        else if ( ImpDetectWEBP( rStm, bExtendedInfo ) ) bRet = true;

        rStm.SetEndian( nOldFormat );
    }
    return bRet;
}

bool GraphicDescriptor::ImpDetectBMP( SvStream& rStm, bool bExtendedInfo )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, bExtendedInfo );
    bool bRet = aDetector.detect() && aDetector.checkBMP();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectGIF( SvStream& rStm, bool bExtendedInfo )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, bExtendedInfo );
    bool bRet = aDetector.detect() && aDetector.checkGIF();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
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

    sal_uInt64 nStmPos = rStm.Tell();

    rStm.SetEndian( SvStreamEndian::BIG );
    rStm.ReadUInt32( nTemp32 );

    // compare upper 24 bits
    if( 0xffd8ff00 == ( nTemp32 & 0xffffff00 ) )
    {
        aMetadata.mnFormat = GraphicFileFormat::JPG;
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
                            sal_uInt64 nNextMarkerPos = rStm.Tell() + nLength - 2;
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
                                                aMetadata.maLogSize = OutputDevice::LogicToLogic( aMetadata.maPixSize, aMap, MapMode( MapUnit::Map100thMM ) );
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
                                    aMetadata.mnNumberOfImageComponents = nNumberOfImageComponents;

                                    // nSamplingFactor (lower nibble: vertical,
                                    // upper nibble: horizontal) is unused

                                    aMetadata.maPixSize.setHeight( nNumberOfLines );
                                    aMetadata.maPixSize.setWidth( nSamplesPerLine );
                                    aMetadata.mnBitsPerPixel = ( nNumberOfImageComponents == 3 ? 24 : nNumberOfImageComponents == 1 ? 8 : 0 );
                                    aMetadata.mnPlanes = 1;

                                    if (aMap.GetMapUnit() != MapUnit::MapPixel)
                                        // We already know the DPI, but the
                                        // pixel size arrived later, so do the
                                        // conversion again.
                                        aMetadata.maLogSize = OutputDevice::LogicToLogic(
                                            aMetadata.maPixSize, aMap, MapMode(MapUnit::Map100thMM));

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
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /*bExtendedInfo*/ );
    bool bRet = aDetector.detect() && aDetector.checkPCD();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectPCX( SvStream& rStm )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, true /*bExtendedInfo*/ );
    bool bRet = aDetector.detect() && aDetector.checkPCX();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectPNG( SvStream& rStm, bool bExtendedInfo )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, bExtendedInfo );
    bool bRet = aDetector.detect() && aDetector.checkPNG();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectTIF( SvStream& rStm, bool bExtendedInfo )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, bExtendedInfo );
    bool bRet = aDetector.detect() && aDetector.checkTIF();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectXBM( SvStream& rStm, bool )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /* bExtendedInfo */ );
    bool bRet = aDetector.detect() && aDetector.checkXBM();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectXPM( SvStream& rStm, bool )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /* bExtendedInfo */ );
    bool bRet = aDetector.detect() && aDetector.checkXPM();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectPBM( SvStream& rStm, bool )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /* bExtendedInfo */ );
    bool bRet = aDetector.detect() && aDetector.checkPBM();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectPGM( SvStream& rStm, bool )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /* bExtendedInfo */ );
    bool bRet = aDetector.detect() && aDetector.checkPGM();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectPPM( SvStream& rStm, bool )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /* bExtendedInfo */ );
    bool bRet = aDetector.detect() && aDetector.checkPPM();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectRAS( SvStream& rStm, bool )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /* bExtendedInfo */ );
    bool bRet = aDetector.detect() && aDetector.checkRAS();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectTGA( SvStream& rStm, bool )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /* bExtendedInfo */ );
    bool bRet = aDetector.detect() && aDetector.checkTGA();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectPSD( SvStream& rStm, bool bExtendedInfo )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, bExtendedInfo );
    bool bRet = aDetector.detect() && aDetector.checkPSD();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectEPS( SvStream& rStm, bool )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /*bExtendedInfo*/ );
    bool bRet = aDetector.detect() && aDetector.checkEPS();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectDXF( SvStream& rStm, bool )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /*bExtendedInfo*/ );
    bool bRet = aDetector.detect() && aDetector.checkDXF();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectMET( SvStream& rStm, bool )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /*bExtendedInfo*/ );
    bool bRet = aDetector.detect() && aDetector.checkMET();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectPCT( SvStream& rStm, bool )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /*bExtendedInfo*/ );
    bool bRet = aDetector.detect() && aDetector.checkPCT();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectSVM( SvStream& rStm, bool bExtendedInfo )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, bExtendedInfo );
    bool bRet = aDetector.detect() && aDetector.checkSVM();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectWMF(SvStream& rStm, bool /*bExtendedInfo*/)
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /*bExtendedInfo*/ );
    bool bRet = aDetector.detect() && aDetector.checkWMF();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectEMF(SvStream& rStm, bool bExtendedInfo)
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, bExtendedInfo );
    bool bRet = aDetector.detect() && aDetector.checkEMF();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectSVG( SvStream& rStm, bool /*bExtendedInfo*/ )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, false /*bExtendedInfo*/ );
    bool bRet = aDetector.detect() && aDetector.checkSVG();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

bool GraphicDescriptor::ImpDetectWEBP( SvStream& rStm, bool bExtendedInfo )
{
    vcl::GraphicFormatDetector aDetector( rStm, aPathExt, bExtendedInfo );
    bool bRet = aDetector.detect() && aDetector.checkWEBP();
    if ( bRet )
        aMetadata = aDetector.getMetadata();
    return bRet;
}

OUString GraphicDescriptor::GetImportFormatShortName( GraphicFileFormat nFormat )
{
    return vcl::getImportFormatShortName( nFormat );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
