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


#include <string.h>
#include <stdio.h>
#include <tools/stream.hxx>
#include <vcl/outdev.hxx>
#include <svtools/filter.hxx>
#include "FilterConfigCache.hxx"
#include <unotools/ucbstreamhelper.hxx>

#define DATA_SIZE           640

sal_uInt8* ImplSearchEntry( sal_uInt8* , sal_uInt8* , sal_uLong , sal_uLong  );

GraphicDescriptor::GraphicDescriptor( const INetURLObject& rPath ) :
    pFileStm( ::utl::UcbStreamHelper::CreateStream( rPath.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ ) ),
    aPathExt( rPath.GetFileExtension().toAsciiLowerCase() ),
    bOwnStream( sal_True )
{
    ImpConstruct();
}

GraphicDescriptor::GraphicDescriptor( SvStream& rInStream, const String* pPath) :
    pFileStm    ( &rInStream ),
    bOwnStream  ( sal_False )
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

sal_Bool GraphicDescriptor::Detect( sal_Bool bExtendedInfo )
{
    sal_Bool bRet = sal_False;
    if ( pFileStm && !pFileStm->GetError() )
    {
        SvStream&   rStm = *pFileStm;
        sal_uInt16      nOldFormat = rStm.GetNumberFormatInt();

        if      ( ImpDetectGIF( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectJPG( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectBMP( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectPNG( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectTIF( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectPCX( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectDXF( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectMET( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectSGF( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectSGV( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectSVM( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectWMF( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectEMF( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectSVG( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectPCT( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectXBM( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectXPM( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectPBM( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectPGM( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectPPM( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectRAS( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectTGA( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectPSD( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectEPS( rStm, bExtendedInfo ) ) bRet = sal_True;
        else if ( ImpDetectPCD( rStm, bExtendedInfo ) ) bRet = sal_True;

        rStm.SetNumberFormatInt( nOldFormat );
    }
    return bRet;
}

void GraphicDescriptor::ImpConstruct()
{
    nFormat = GFF_NOT;
    nBitsPerPixel = 0;
    nPlanes = 0;
    bCompressed = sal_False;
}

sal_Bool GraphicDescriptor::ImpDetectBMP( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_uInt16  nTemp16;
    sal_Bool    bRet = sal_False;
    sal_Int32 nStmPos = rStm.Tell();

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm >> nTemp16;

    // OS/2-BitmapArray
    if ( nTemp16 == 0x4142 )
    {
        rStm.SeekRel( 0x0c );
        rStm >> nTemp16;
    }

    // Bitmap
    if ( nTemp16 == 0x4d42 )
    {
        nFormat = GFF_BMP;
        bRet = sal_True;

        if ( bExtendedInfo )
        {
            sal_uInt32  nTemp32;
            sal_uInt32  nCompression;

            // bis zur ersten Information
            rStm.SeekRel( 0x10 );

            // PixelBreite auslesen
            rStm >> nTemp32;
            aPixSize.Width() = nTemp32;

            // PixelHoehe auslesen
            rStm >> nTemp32;
            aPixSize.Height() = nTemp32;

            // Planes auslesen
            rStm >> nTemp16;
            nPlanes = nTemp16;

            // BitCount auslesen
            rStm >> nTemp16;
            nBitsPerPixel = nTemp16;

            // Compression auslesen
            rStm >> nTemp32;
            bCompressed = ( ( nCompression = nTemp32 ) > 0 );

            // logische Breite
            rStm.SeekRel( 4 );
            rStm >> nTemp32;
            if ( nTemp32 )
                aLogSize.Width() = ( aPixSize.Width() * 100000 ) / nTemp32;

            // logische Hoehe
            rStm >> nTemp32;
            if ( nTemp32 )
                aLogSize.Height() = ( aPixSize.Height() * 100000 ) / nTemp32;

            // Wir wollen noch etwas feiner differenzieren und
            // auf sinnvolle Werte ueberpruefen ( Bug-Id #29001 )
            if ( ( nBitsPerPixel > 24 ) || ( nCompression > 3 ) )
            {
                nFormat = GFF_NOT;
                bRet = sal_False;
            }
        }
    }
    rStm.Seek( nStmPos );
    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectGIF( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_uInt32  n32;
    sal_uInt16  n16;
    sal_Bool    bRet = sal_False;
    sal_uInt8   cByte;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm >> n32;

    if ( n32 == 0x38464947 )
    {
        rStm >> n16;
        if ( ( n16 == 0x6137 ) || ( n16 == 0x6139 ) )
        {
            nFormat = GFF_GIF;
            bRet = sal_True;

            if ( bExtendedInfo )
            {
                sal_uInt16 nTemp16;

                // PixelBreite auslesen
                rStm >> nTemp16;
                aPixSize.Width() = nTemp16;

                // PixelHoehe auslesen
                rStm >> nTemp16;
                aPixSize.Height() = nTemp16;

                // Bits/Pixel auslesen
                rStm >> cByte;
                nBitsPerPixel = ( ( cByte & 112 ) >> 4 ) + 1;
            }
        }
    }
    rStm.Seek( nStmPos );
    return bRet;
}

// returns the next jpeg marker, a return value of 0 represents an error
sal_uInt8 ImpDetectJPG_GetNextMarker( SvStream& rStm )
{
    sal_uInt8 nByte;
    do
    {
        do
        {
            rStm >> nByte;
            if ( rStm.IsEof() || rStm.GetError() )  // as 0 is not allowed as marker,
                return 0;                           // we can use it as errorcode
        }
        while ( nByte != 0xff );
        do
        {
            rStm >> nByte;
            if ( rStm.IsEof() || rStm.GetError() )
                return 0;
        }
        while( nByte == 0xff );
    }
    while( nByte == 0 );        // 0xff00 represents 0xff and not a marker,
                                // the marker detection has to be restartet.
    return nByte;
}

sal_Bool GraphicDescriptor::ImpDetectJPG( SvStream& rStm,  sal_Bool bExtendedInfo )
{
    sal_uInt32  nTemp32;
    sal_Bool    bRet = sal_False;

    sal_Int32 nStmPos = rStm.Tell();

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm >> nTemp32;

    // compare upper 24 bits
    if( 0xffd8ff00 == ( nTemp32 & 0xffffff00 ) )
    {
        nFormat = GFF_JPG;
        bRet = sal_True;

        if ( bExtendedInfo )
        {
            rStm.SeekRel( -2 );

            sal_uInt32 nError( rStm.GetError() );

            sal_Bool bScanFailure = sal_False;
            sal_Bool bScanFinished = sal_False;

            while( !bScanFailure && !bScanFinished && !rStm.IsEof() && !rStm.GetError() )
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
                        bScanFailure = sal_True;
                    break;

                    case 0xd9 :     // EOI
                        bScanFinished = sal_True;
                    break;

                    // per default we assume marker segments conaining a length parameter
                    default :
                    {
                        sal_uInt16 nLength;
                        rStm >> nLength;

                        if ( nLength < 2 )
                            bScanFailure = sal_True;
                        else
                        {
                            sal_uInt32 nNextMarkerPos = rStm.Tell() + nLength - 2;
                            switch( nMarker )
                            {
                                case 0xe0 : // APP0 Marker
                                {
                                    if ( nLength == 16 )
                                    {
                                        sal_Int32 nIdentifier;
                                        rStm >> nIdentifier;
                                        if ( nIdentifier == 0x4a464946 )    // JFIF Identifier
                                        {
                                            sal_uInt8   nStringTerminator;
                                            sal_uInt8   nMajorRevision;
                                            sal_uInt8   nMinorRevision;
                                            sal_uInt8   nUnits;
                                            sal_uInt16  nHorizontalResolution;
                                            sal_uInt16  nVerticalResolution;
                                            sal_uInt8   nHorzThumbnailPixelCount;
                                            sal_uInt8   nVertThumbnailPixelCount;

                                            rStm >> nStringTerminator
                                                 >> nMajorRevision
                                                 >> nMinorRevision
                                                 >> nUnits
                                                 >> nHorizontalResolution
                                                 >> nVerticalResolution
                                                 >> nHorzThumbnailPixelCount
                                                 >> nVertThumbnailPixelCount;

                                            // setting the logical size
                                            if ( nUnits && nHorizontalResolution && nVerticalResolution )
                                            {
                                                MapMode aMap;
                                                aMap.SetMapUnit( nUnits == 1 ? MAP_INCH : MAP_CM );
                                                aMap.SetScaleX( Fraction( 1, nHorizontalResolution ) );
                                                aMap.SetScaleY( Fraction( 1, nVerticalResolution ) );
                                                aLogSize = OutputDevice::LogicToLogic( aPixSize, aMap, MapMode( MAP_100TH_MM ) );
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
                                    sal_uInt8   nSamplePrecision;
                                    sal_uInt16  nNumberOfLines;
                                    sal_uInt16  nSamplesPerLine;
                                    sal_uInt8   nNumberOfImageComponents;
                                    sal_uInt8   nComponentsIdentifier;
                                    sal_uInt8   nHorizontalSamplingFactor;
                                    sal_uInt8   nQuantizationTableDestinationSelector;
                                    rStm >> nSamplePrecision
                                         >> nNumberOfLines
                                         >> nSamplesPerLine
                                         >> nNumberOfImageComponents
                                         >> nComponentsIdentifier
                                         >> nHorizontalSamplingFactor
                                         >> nQuantizationTableDestinationSelector;
                                    nHorizontalSamplingFactor >>= 4;

                                    aPixSize.Height() = nNumberOfLines;
                                    aPixSize.Width() = nSamplesPerLine;
                                    nBitsPerPixel = ( nNumberOfImageComponents == 3 ? 24 : nNumberOfImageComponents == 1 ? 8 : 0 );
                                    nPlanes = 1;

                                    bScanFinished = sal_True;
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

sal_Bool GraphicDescriptor::ImpDetectPCD( SvStream& rStm, sal_Bool )
{
    sal_Bool    bRet = sal_False;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    sal_uInt32  nTemp32;
    sal_uInt16  nTemp16;
    sal_uInt8   cByte;

    rStm.SeekRel( 2048 );
    rStm >> nTemp32;
    rStm >> nTemp16;
    rStm >> cByte;

    if ( ( nTemp32 == 0x5f444350 ) &&
         ( nTemp16 == 0x5049 ) &&
         ( cByte == 0x49 ) )
    {
        nFormat = GFF_PCD;
        bRet = sal_True;
    }
    rStm.Seek( nStmPos );
    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectPCX( SvStream& rStm, sal_Bool bExtendedInfo )
{
    // ! Because 0x0a can be interpreted as LF too ...
    // we cant be shure that this special sign represent a PCX file only.
    // Every Ascii file is possible here :-(
    // We must detect the whole header.
    bExtendedInfo = sal_True;

    sal_Bool    bRet = sal_False;
    sal_uInt8   cByte;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm >> cByte;

    if ( cByte == 0x0a )
    {
        nFormat = GFF_PCX;
        bRet = sal_True;

        if ( bExtendedInfo )
        {
            sal_uInt16  nTemp16;
            sal_uInt16  nXmin;
            sal_uInt16  nXmax;
            sal_uInt16  nYmin;
            sal_uInt16  nYmax;
            sal_uInt16  nDPIx;
            sal_uInt16  nDPIy;


            rStm.SeekRel( 1 );

            // Kompression lesen
            rStm >> cByte;
            bCompressed = ( cByte > 0 );

            bRet = (cByte==0 || cByte ==1);

            // Bits/Pixel lesen
            rStm >> cByte;
            nBitsPerPixel = cByte;

            // Bildabmessungen
            rStm >> nTemp16;
            nXmin = nTemp16;
            rStm >> nTemp16;
            nYmin = nTemp16;
            rStm >> nTemp16;
            nXmax = nTemp16;
            rStm >> nTemp16;
            nYmax = nTemp16;

            aPixSize.Width() = nXmax - nXmin + 1;
            aPixSize.Height() = nYmax - nYmin + 1;

            // Aufloesung
            rStm >> nTemp16;
            nDPIx = nTemp16;
            rStm >> nTemp16;
            nDPIy = nTemp16;

            // logische Groesse setzen
            MapMode aMap( MAP_INCH, Point(),
                          Fraction( 1, nDPIx ), Fraction( 1, nDPIy ) );
            aLogSize = OutputDevice::LogicToLogic( aPixSize, aMap,
                                                   MapMode( MAP_100TH_MM ) );


            // Anzahl Farbebenen
            rStm.SeekRel( 49 );
            rStm >> cByte;
            nPlanes = cByte;

            bRet = (nPlanes<=4);
        }
    }

    rStm.Seek( nStmPos );
    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectPNG( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_uInt32  nTemp32;
    sal_Bool    bRet = sal_False;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm >> nTemp32;

    if ( nTemp32 == 0x89504e47 )
    {
        rStm >> nTemp32;
        if ( nTemp32 == 0x0d0a1a0a )
        {
            nFormat = GFF_PNG;
            bRet = sal_True;

            if ( bExtendedInfo )
            {
                sal_uInt8 cByte;

                // IHDR-Chunk
                rStm.SeekRel( 8 );

                // Breite einlesen
                rStm >> nTemp32;
                aPixSize.Width() = nTemp32;

                // Hoehe einlesen
                rStm >> nTemp32;
                aPixSize.Height() = nTemp32;

                // Bits/Pixel einlesen
                rStm >> cByte;
                nBitsPerPixel = cByte;

                // Planes immer 1;
                // Kompression immer
                nPlanes = 1;
                bCompressed = sal_True;

                sal_uInt32  nLen32;

                rStm.SeekRel( 8 );

                // so lange ueberlesen, bis wir den pHYs-Chunk haben oder
                // den Anfang der Bilddaten
                rStm >> nLen32;
                rStm >> nTemp32;
                while( ( nTemp32 != 0x70485973 ) && ( nTemp32 != 0x49444154 ) )
                {
                    rStm.SeekRel( 4 + nLen32 );
                    rStm >> nLen32;
                    rStm >> nTemp32;
                }

                if ( nTemp32 == 0x70485973 )
                {
                    sal_uLong   nXRes;
                    sal_uLong   nYRes;

                    // horizontale Aufloesung
                    rStm >> nTemp32;
                    nXRes = nTemp32;

                    // vertikale Aufloesung
                    rStm >> nTemp32;
                    nYRes = nTemp32;

                    // Unit einlesen
                    rStm >> cByte;

                    if ( cByte )
                    {
                        if ( nXRes )
                            aLogSize.Width() = ( aPixSize.Width() * 100000 ) /
                                               nTemp32;

                        if ( nYRes )
                            aLogSize.Height() = ( aPixSize.Height() * 100000 ) /
                                                nTemp32;
                    }
                }
            }
        }
    }
    rStm.Seek( nStmPos );
    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectTIF( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_Bool    bDetectOk = sal_False;
    sal_Bool    bRet = sal_False;
    sal_uInt8   cByte1;
    sal_uInt8   cByte2;

    sal_Int32 nStmPos = rStm.Tell();
    rStm >> cByte1;
    rStm >> cByte2;
    if ( cByte1 == cByte2 )
    {
        if ( cByte1 == 0x49 )
        {
            rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
            bDetectOk = sal_True;
        }
        else if ( cByte1 == 0x4d )
        {
            rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
            bDetectOk = sal_True;
        }

        if ( bDetectOk )
        {
            sal_uInt16  nTemp16;

            rStm >> nTemp16;
            if ( nTemp16 == 0x2a )
            {
                nFormat = GFF_TIF;
                bRet = sal_True;

                if ( bExtendedInfo )
                {
                    sal_uLong   nCount;
                    sal_uLong   nMax = DATA_SIZE - 48;
                    sal_uInt32  nTemp32;
                    sal_Bool    bOk = sal_False;

                    // Offset des ersten IFD einlesen
                    rStm >> nTemp32;
                    rStm.SeekRel( ( nCount = ( nTemp32 + 2 ) ) - 0x08 );

                    if ( nCount < nMax )
                    {
                        // Tag's lesen, bis wir auf Tag256 ( Width ) treffen
                        // nicht mehr Bytes als DATA_SIZE lesen
                        rStm >> nTemp16;
                        while ( nTemp16 != 256 )
                        {
                            bOk = nCount < nMax;
                            if ( !bOk )
                            {
                                break;
                            }
                            rStm.SeekRel( 10 );
                            rStm >> nTemp16;
                            nCount += 12;
                        }

                        if ( bOk )
                        {
                            // Breite lesen
                            rStm >> nTemp16;
                            rStm.SeekRel( 4 );
                            if ( nTemp16 == 3 )
                            {
                                rStm >> nTemp16;
                                aPixSize.Width() = nTemp16;
                                rStm.SeekRel( 2 );
                            }
                            else
                            {
                                rStm >> nTemp32;
                                aPixSize.Width() = nTemp32;
                            }
                            nCount += 12;

                            // Hoehe lesen
                            rStm.SeekRel( 2 );
                            rStm >> nTemp16;
                            rStm.SeekRel( 4 );
                            if ( nTemp16 == 3 )
                            {
                                rStm >> nTemp16;
                                aPixSize.Height() = nTemp16;
                                rStm.SeekRel( 2 );
                            }
                            else
                            {
                                rStm >> nTemp32;
                                aPixSize.Height() = nTemp32;
                            }
                            nCount += 12;

                            // ggf. Bits/Pixel lesen
                            rStm >> nTemp16;
                            if ( nTemp16 == 258 )
                            {
                                rStm.SeekRel( 6 );
                                rStm >> nTemp16;
                                nBitsPerPixel = nTemp16;
                                rStm.SeekRel( 2 );
                                nCount += 12;
                            }
                            else
                                rStm.SeekRel( -2 );

                            // ggf. Compression lesen
                            rStm >> nTemp16;
                            if ( nTemp16 == 259 )
                            {
                                rStm.SeekRel( 6 );
                                rStm >> nTemp16;
                                bCompressed = ( nTemp16 > 1 );
                                rStm.SeekRel( 2 );
                                nCount += 12;
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

sal_Bool GraphicDescriptor::ImpDetectXBM( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "xbm", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_XBM;

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectXPM( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "xpm", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_XPM;

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectPBM( SvStream& rStm, sal_Bool )
{
    sal_Bool bRet = sal_False;

    // erst auf Datei Extension pruefen, da diese aussagekraeftiger ist
    // als die 2 ID Bytes

    if ( aPathExt.CompareToAscii( "pbm", 3 ) == COMPARE_EQUAL )
        bRet = sal_True;
    else
    {
        sal_Int32 nStmPos = rStm.Tell();
        sal_uInt8   nFirst, nSecond;
        rStm >> nFirst >> nSecond;
        if ( nFirst == 'P' && ( ( nSecond == '1' ) || ( nSecond == '4' ) ) )
            bRet = sal_True;
        rStm.Seek( nStmPos );
    }

    if ( bRet )
        nFormat = GFF_PBM;

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectPGM( SvStream& rStm, sal_Bool )
{
    sal_Bool bRet = sal_False;

    if ( aPathExt.CompareToAscii( "pgm", 3 ) == COMPARE_EQUAL )
        bRet = sal_True;
    else
    {
        sal_uInt8 nFirst, nSecond;
        sal_Int32 nStmPos = rStm.Tell();
        rStm >> nFirst >> nSecond;
        if ( nFirst == 'P' && ( ( nSecond == '2' ) || ( nSecond == '5' ) ) )
            bRet = sal_True;
        rStm.Seek( nStmPos );
    }

    if ( bRet )
        nFormat = GFF_PGM;

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectPPM( SvStream& rStm, sal_Bool )
{
    sal_Bool bRet = sal_False;

    if ( aPathExt.CompareToAscii( "ppm", 3 ) == COMPARE_EQUAL )
        bRet = sal_True;
    else
    {
        sal_uInt8   nFirst, nSecond;
        sal_Int32 nStmPos = rStm.Tell();
        rStm >> nFirst >> nSecond;
        if ( nFirst == 'P' && ( ( nSecond == '3' ) || ( nSecond == '6' ) ) )
            bRet = sal_True;
        rStm.Seek( nStmPos );
    }

    if ( bRet )
        nFormat = GFF_PPM;

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectRAS( SvStream& rStm, sal_Bool )
{
    sal_uInt32 nMagicNumber;
    sal_Bool bRet = sal_False;
    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm >> nMagicNumber;
    if ( nMagicNumber == 0x59a66a95 )
    {
        nFormat = GFF_RAS;
        bRet = sal_True;
    }
    rStm.Seek( nStmPos );
    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectTGA( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "tga", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_TGA;

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectPSD( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_Bool bRet = sal_False;

    sal_uInt32  nMagicNumber;
    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm >> nMagicNumber;
    if ( nMagicNumber == 0x38425053 )
    {
        sal_uInt16 nVersion;
        rStm >> nVersion;
        if ( nVersion == 1 )
        {
            bRet = sal_True;
            if ( bExtendedInfo )
            {
                sal_uInt16 nChannels;
                sal_uInt32 nRows;
                sal_uInt32 nColumns;
                sal_uInt16 nDepth;
                sal_uInt16 nMode;
                rStm.SeekRel( 6 );  // Pad
                rStm >> nChannels >> nRows >> nColumns >> nDepth >> nMode;
                if ( ( nDepth == 1 ) || ( nDepth == 8 ) || ( nDepth == 16 ) )
                {
                    nBitsPerPixel = ( nDepth == 16 ) ? 8 : nDepth;
                    switch ( nChannels )
                    {
                        case 4 :
                        case 3 :
                            nBitsPerPixel = 24;
                        case 2 :
                        case 1 :
                            aPixSize.Width() = nColumns;
                            aPixSize.Height() = nRows;
                        break;
                        default:
                            bRet = sal_False;
                    }
                }
                else
                    bRet = sal_False;
            }
        }
    }

    if ( bRet )
        nFormat = GFF_PSD;
    rStm.Seek( nStmPos );
    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectEPS( SvStream& rStm, sal_Bool )
{
    // es wird die EPS mit Vorschaubild Variante und die Extensionuebereinstimmung
    // geprueft

    sal_uInt32  nFirstLong;
    sal_uInt8   nFirstBytes[20];
    sal_Bool        bRet = sal_False;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm >> nFirstLong;
    rStm.SeekRel( -4 );
    rStm.Read( &nFirstBytes, 20 );

    if ( ( nFirstLong == 0xC5D0D3C6 ) || ( aPathExt.CompareToAscii( "eps", 3 ) == COMPARE_EQUAL ) ||
        ( ImplSearchEntry( nFirstBytes, (sal_uInt8*)"%!PS-Adobe", 10, 10 )
            && ImplSearchEntry( &nFirstBytes[15], (sal_uInt8*)"EPS", 3, 3 ) ) )
    {
        nFormat = GFF_EPS;
        bRet = sal_True;
    }
    rStm.Seek( nStmPos );
    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectDXF( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "dxf", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_DXF;

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectMET( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "met", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_MET;

    return bRet;
}

extern bool isPCT(SvStream& rStream, sal_uLong nStreamPos, sal_uLong nStreamLen);

sal_Bool GraphicDescriptor::ImpDetectPCT( SvStream& rStm, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "pct", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_PCT;
    else
    {
        sal_Size nStreamPos = rStm.Tell();
        sal_Size nStreamLen = rStm.remainingSize();
        if (isPCT(rStm, nStreamPos, nStreamLen))
        {
            bRet = sal_True;
            nFormat = GFF_PCT;
        }
        rStm.Seek(nStreamPos);
    }

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectSGF( SvStream& rStm, sal_Bool )
{
    sal_Bool bRet = sal_False;
    if( aPathExt.CompareToAscii( "sgf", 3 ) == COMPARE_EQUAL )
        bRet = sal_True;
    else
    {
        sal_Int32 nStmPos = rStm.Tell();

        sal_uInt8 nFirst, nSecond;

        rStm >> nFirst >> nSecond;

        if( nFirst == 'J' && nSecond == 'J' )
            bRet = sal_True;

        rStm.Seek( nStmPos );
    }

    if( bRet )
        nFormat = GFF_SGF;

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectSGV( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "sgv", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_SGV;

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectSVM( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_uInt32  n32;
    sal_Bool    bRet = sal_False;
    sal_uInt8   cByte;

    sal_Int32 nStmPos = rStm.Tell();
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm >> n32;
    if ( n32 == 0x44475653 )
    {
        rStm >> cByte;
        if ( cByte == 0x49 )
        {
            nFormat = GFF_SVM;
            bRet = sal_True;

            if ( bExtendedInfo )
            {
                sal_uInt32  nTemp32;
                sal_uInt16  nTemp16;

                rStm.SeekRel( 0x04 );

                // Breite auslesen
                rStm >> nTemp32;
                aLogSize.Width() = nTemp32;

                // Hoehe auslesen
                rStm >> nTemp32;
                aLogSize.Height() = nTemp32;

                // Map-Unit auslesen und PrefSize ermitteln
                rStm >> nTemp16;
                aLogSize = OutputDevice::LogicToLogic( aLogSize,
                                                       MapMode( (MapUnit) nTemp16 ),
                                                       MapMode( MAP_100TH_MM ) );
            }
        }
    }
    else
    {
        rStm.SeekRel( -4L );
        rStm >> n32;

        if( n32 == 0x4D4C4356 )
        {
            sal_uInt16 nTmp16;

            rStm >> nTmp16;

            if( nTmp16 == 0x4654 )
            {
                nFormat = GFF_SVM;
                bRet = sal_True;

                if( bExtendedInfo )
                {
                    MapMode aMapMode;

                    rStm.SeekRel( 0x06 );
                    rStm >> aMapMode;
                    rStm >> aLogSize;
                    aLogSize = OutputDevice::LogicToLogic( aLogSize, aMapMode, MapMode( MAP_100TH_MM ) );
                }
            }
        }
    }
    rStm.Seek( nStmPos );
    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectWMF( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "wmf",3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_WMF;

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectEMF( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "emf", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_EMF;

    return bRet;
}

sal_Bool GraphicDescriptor::ImpDetectSVG( SvStream& /*rStm*/, sal_Bool /*bExtendedInfo*/ )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "svg", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_SVG;

    return bRet;
}

String GraphicDescriptor::GetImportFormatShortName( sal_uInt16 nFormat )
{
    const char *pKeyName = 0;

    switch( nFormat )
    {
        case( GFF_BMP ) :   pKeyName = "bmp";   break;
        case( GFF_GIF ) :   pKeyName = "gif";   break;
        case( GFF_JPG ) :   pKeyName = "jpg";   break;
        case( GFF_PCD ) :   pKeyName = "pcd";   break;
        case( GFF_PCX ) :   pKeyName = "pcx";   break;
        case( GFF_PNG ) :   pKeyName = "png";   break;
        case( GFF_XBM ) :   pKeyName = "xbm";   break;
        case( GFF_XPM ) :   pKeyName = "xpm";   break;
        case( GFF_PBM ) :   pKeyName = "pbm";   break;
        case( GFF_PGM ) :   pKeyName = "pgm";   break;
        case( GFF_PPM ) :   pKeyName = "ppm";   break;
        case( GFF_RAS ) :   pKeyName = "ras";   break;
        case( GFF_TGA ) :   pKeyName = "tga";   break;
        case( GFF_PSD ) :   pKeyName = "psd";   break;
        case( GFF_EPS ) :   pKeyName = "eps";   break;
        case( GFF_TIF ) :   pKeyName = "tif";   break;
        case( GFF_DXF ) :   pKeyName = "dxf";   break;
        case( GFF_MET ) :   pKeyName = "met";   break;
        case( GFF_PCT ) :   pKeyName = "pct";   break;
        case( GFF_SGF ) :   pKeyName = "sgf";   break;
        case( GFF_SGV ) :   pKeyName = "sgv";   break;
        case( GFF_SVM ) :   pKeyName = "svm";   break;
        case( GFF_WMF ) :   pKeyName = "wmf";   break;
        case( GFF_EMF ) :   pKeyName = "emf";   break;
        case( GFF_SVG ) :   pKeyName = "svg";   break;
    }

    return rtl::OUString::createFromAscii(pKeyName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
