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
#include "precompiled_svtools.hxx"

#include <string.h>
#include <stdio.h>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <vcl/outdev.hxx>
#include <tools/config.hxx>
#include <svtools/filter.hxx>
#include "FilterConfigCache.hxx"
#include <unotools/ucbstreamhelper.hxx>

#define DATA_SIZE           640

sal_uInt8* ImplSearchEntry( sal_uInt8* , sal_uInt8* , sal_uIntPtr , sal_uIntPtr  );


/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphicDescriptor::GraphicDescriptor( const String* pPath ) :
    pFileStm        ( NULL )
{
    ImpConstruct();

    if ( pPath )
    {
        INetURLObject aURL( *pPath, INET_PROT_FILE );
        aPathExt = aURL.GetFileExtension().toAsciiLowerCase();
    }
    bLinked = sal_True;
    bLinkChanged = sal_False;
    bWideSearch = sal_False;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphicDescriptor::GraphicDescriptor( const INetURLObject& rPath ) :
    pFileStm( ::utl::UcbStreamHelper::CreateStream( rPath.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ ) ),
    aPathExt( rPath.GetFileExtension().toAsciiLowerCase() )
{
    if ( pFileStm )
    {
        nStmPos = 0;
        pFileStm->Seek( nStmPos );
        bDataReady = sal_True;
    }

    ImpConstruct();

    if ( pFileStm && !pFileStm->GetError() )
        bDataReady = sal_True;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphicDescriptor::GraphicDescriptor( SvStream& rInStream, const String* pPath) :
    pFileStm    ( NULL )
{
    ImpConstruct();

    if ( pPath )
    {
        INetURLObject aURL( *pPath );
        aPathExt = aURL.GetFileExtension().toAsciiLowerCase();
    }
    nStmPos = rInStream.Tell();
    pBaseStm = &rInStream;
    bBaseStm = sal_True;

    if ( !pBaseStm->GetError() )
        bDataReady = sal_True;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphicDescriptor::~GraphicDescriptor()
{
    delete pFileStm;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::Detect( sal_Bool bExtendedInfo )
{
    sal_Bool bRet = sal_False;

    // Link-Status ueberpruefen
    if ( bLinked && bLinkChanged )
    {
        DBG_ASSERT( aReqLink.IsSet(), "Wo ist der RequestHandler???" );
        pMemStm = (SvStream*) aReqLink.Call( this );
        if ( pMemStm )
        {
            nStmPos = pMemStm->Tell();
            bDataReady = sal_True;
        }
    }

    if ( bDataReady )
    {
        SvStream&   rStm = GetSearchStream();
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

        // diese Formate lassen sich nur bei WideSearch im gesamten
        // Stream ermitteln
        else if ( bWideSearch )
        {
            if ( ImpDetectPCD( rStm, bExtendedInfo ) )
                bRet = sal_True;
        }

        rStm.SetNumberFormatInt( nOldFormat );
        rStm.Seek( nStmPos );
    }

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::IsDataReady() const
{
    return bDataReady;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::IsWideSearch() const
{
    return bWideSearch;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

SvStream& GraphicDescriptor::GetSearchStream() const
{
    DBG_ASSERT( bDataReady, "Was laeuft hier falsch???" );

    if ( bLinked )
        return *pMemStm;
    else if ( bBaseStm )
        return *pBaseStm;
    else
        return *pFileStm;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphicDescriptor::SetRequestHdl( const Link& rRequestLink )
{
    aReqLink = rRequestLink;
    bLinkChanged = sal_True;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_uIntPtr GraphicDescriptor::GetRequestedByteCount() const
{
    return DATA_SIZE;
}


/******************************************************************************/
/*                               IMP-Methoden                                 */
/*                                                                            */


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphicDescriptor::ImpConstruct()
{
    if ( !pFileStm )
        pFileStm = new SvStream();
    nFormat = GFF_NOT;
    nBitsPerPixel = 0;
    nPlanes = 0;
    bCompressed = sal_False;
    bDataReady = sal_False;
    bLinked = sal_False;
    bWideSearch = sal_True;
    bBaseStm = sal_False;
    pMemStm = NULL;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectBMP( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_uInt16  nTemp16;
    sal_Bool    bRet = sal_False;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm.Seek( nStmPos );

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

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectGIF( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_uInt32  n32;
    sal_uInt16  n16;
    sal_Bool    bRet = sal_False;
    sal_uInt8   cByte;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm.Seek( nStmPos );

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

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectJPG( SvStream& rStm,  sal_Bool bExtendedInfo )
{
    sal_uInt32  nTemp32;
    sal_Bool    bRet = sal_False;
    sal_uInt8   cByte = 0;
    sal_Bool    bM_COM;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm.Seek( nStmPos );

    rStm >> nTemp32;

    // compare upper 28 bits
    if( 0xffd8ff00 == ( nTemp32 & 0xffffff00 ) )
    {
        nFormat = GFF_JPG;
        return sal_True;
    }

    bM_COM = ( nTemp32 == 0xffd8fffe );
    if ( ( nTemp32 == 0xffd8ffe0 ) || bM_COM )
    {
        if( !bM_COM )
        {
            rStm.SeekRel( 2 );
            rStm >> nTemp32;
        }

        if( bM_COM || ( nTemp32 == 0x4a464946 ) )
        {
            nFormat = GFF_JPG;
            bRet = sal_True;

            if( bExtendedInfo )
            {
                MapMode aMap;
                sal_uInt16  nTemp16;
                sal_uIntPtr nCount = 9;
                sal_uIntPtr nMax;
                sal_uIntPtr nResX;
                sal_uIntPtr nResY;
                sal_uInt8   cUnit;

                // Groesse des verbleibenden Puffers ermitteln
                if ( bLinked )
                    nMax = static_cast< SvMemoryStream& >(rStm).GetEndOfData()
                                - 16;
                else
                    nMax = DATA_SIZE - 16;

                // max. 8K
                nMax = Min( nMax, (sal_uIntPtr) 8192 );

                // Res-Unit ermitteln
                rStm.SeekRel( 3 );
                rStm >> cUnit;

                // ResX ermitteln
                rStm >> nTemp16;
                nResX = nTemp16;

                // ResY ermitteln
                rStm >> nTemp16;
                nResY = nTemp16;

                // SOF0/1-Marker finden, aber dabei
                // nicht mehr als DATA_SIZE Pixel lesen, falls
                // kein WideSearch
                do
                {
                    while ( ( cByte != 0xff ) &&
                            ( bWideSearch || ( nCount++ < nMax ) ) )
                    {
                        rStm >> cByte;
                    }

                    while ( ( cByte == 0xff ) &&
                            ( bWideSearch || ( nCount++ < nMax ) ) )
                    {
                        rStm >> cByte;
                    }
                }
                while ( ( cByte != 0xc0 ) &&
                        ( cByte != 0xc1 ) &&
                        ( bWideSearch || ( nCount < nMax ) ) );

                // wir haben den SOF0/1-Marker
                if ( ( cByte == 0xc0 ) || ( cByte == 0xc1 ) )
                {
                    // Hoehe einlesen
                    rStm.SeekRel( 3 );
                    rStm >> nTemp16;
                    aPixSize.Height() = nTemp16;

                    // Breite einlesen
                    rStm >> nTemp16;
                    aPixSize.Width() = nTemp16;

                    // Bit/Pixel einlesen
                    rStm >> cByte;
                    nBitsPerPixel = ( cByte == 3 ? 24 : cByte == 1 ? 8 : 0 );

                    // logische Groesse setzen
                    if ( cUnit && nResX && nResY )
                    {
                        aMap.SetMapUnit( cUnit == 1 ? MAP_INCH : MAP_CM );
                        aMap.SetScaleX( Fraction( 1, nResX ) );
                        aMap.SetScaleY( Fraction( 1, nResY ) );
                        aLogSize = OutputDevice::LogicToLogic( aPixSize, aMap,
                                                               MapMode( MAP_100TH_MM ) );
                    }

                    // Planes immer 1
                    nPlanes = 1;
                }
            }
        }
    }

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectPCD( SvStream& rStm, sal_Bool )
{
    sal_Bool    bRet = sal_False;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm.Seek( nStmPos );

    if ( bWideSearch )
    {
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
    }
    else
    {
        bRet = aPathExt.CompareToAscii( "pcd", 3 ) == COMPARE_EQUAL;
        if ( bRet )
        {
            nFormat = GFF_PCD;
        }
    }

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectPCX( SvStream& rStm, sal_Bool bExtendedInfo )
{
    // ! Because 0x0a can be interpreted as LF too ...
    // we cant be shure that this special sign represent a PCX file only.
    // Every Ascii file is possible here :-(
    // We must detect the whole header.
    bExtendedInfo = sal_True;

    sal_Bool    bRet = sal_False;
    sal_uInt8   cByte;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm.Seek( nStmPos );

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

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectPNG( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_uInt32  nTemp32;
    sal_Bool    bRet = sal_False;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm.Seek( nStmPos );

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

                if ( bWideSearch )
                {
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
                        sal_uIntPtr nXRes;
                        sal_uIntPtr nYRes;

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
    }

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectTIF( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_Bool    bDetectOk = sal_False;
    sal_Bool    bRet = sal_False;
    sal_uInt8   cByte1;
    sal_uInt8   cByte2;

    rStm.Seek( nStmPos );
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
                    sal_uIntPtr nCount;
                    sal_uIntPtr nMax = DATA_SIZE - 48;
                    sal_uInt32  nTemp32;
                    sal_Bool    bOk = sal_False;

                    // Offset des ersten IFD einlesen
                    rStm >> nTemp32;
                    rStm.SeekRel( ( nCount = ( nTemp32 + 2 ) ) - 0x08 );

                    if ( bWideSearch || ( nCount < nMax ) )
                    {
                        // Tag's lesen, bis wir auf Tag256 ( Width ) treffen
                        // nicht mehr Bytes als DATA_SIZE lesen
                        rStm >> nTemp16;
                        while ( nTemp16 != 256 )
                        {
                            bOk = bWideSearch || ( nCount < nMax );
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

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectXBM( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "xbm", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_XBM;

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectXPM( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "xpm", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_XPM;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectPBM( SvStream& rStm, sal_Bool )
{
    sal_Bool bRet = sal_False;

    // erst auf Datei Extension pruefen, da diese aussagekraeftiger ist
    // als die 2 ID Bytes

    if ( aPathExt.CompareToAscii( "pbm", 3 ) == COMPARE_EQUAL )
        bRet = sal_True;
    else
    {
        sal_uInt8   nFirst, nSecond;
        rStm.Seek( nStmPos );
        rStm >> nFirst >> nSecond;
        if ( nFirst == 'P' && ( ( nSecond == '1' ) || ( nSecond == '4' ) ) )
            bRet = sal_True;
    }

    if ( bRet )
        nFormat = GFF_PBM;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectPGM( SvStream& rStm, sal_Bool )
{
    sal_Bool bRet = sal_False;

    if ( aPathExt.CompareToAscii( "pgm", 3 ) == COMPARE_EQUAL )
        bRet = sal_True;
    else
    {
        sal_uInt8   nFirst, nSecond;
        rStm.Seek( nStmPos );
        rStm >> nFirst >> nSecond;
        if ( nFirst == 'P' && ( ( nSecond == '2' ) || ( nSecond == '5' ) ) )
            bRet = sal_True;
    }

    if ( bRet )
        nFormat = GFF_PGM;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectPPM( SvStream& rStm, sal_Bool )
{
    sal_Bool bRet = sal_False;

    if ( aPathExt.CompareToAscii( "ppm", 3 ) == COMPARE_EQUAL )
        bRet = sal_True;
    else
    {
        sal_uInt8   nFirst, nSecond;
        rStm.Seek( nStmPos );
        rStm >> nFirst >> nSecond;
        if ( nFirst == 'P' && ( ( nSecond == '3' ) || ( nSecond == '6' ) ) )
            bRet = sal_True;
    }

    if ( bRet )
        nFormat = GFF_PPM;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectRAS( SvStream& rStm, sal_Bool )
{
    sal_uInt32 nMagicNumber;
    rStm.Seek( nStmPos );
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm >> nMagicNumber;
    if ( nMagicNumber == 0x59a66a95 )
    {
        nFormat = GFF_RAS;
        return sal_True;
    }
    else
        return sal_False;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectTGA( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "tga", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_TGA;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectPSD( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_Bool bRet = sal_False;

    sal_uInt32  nMagicNumber;
    rStm.Seek( nStmPos );
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
    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectEPS( SvStream& rStm, sal_Bool )
{
    // es wird die EPS mit Vorschaubild Variante und die Extensionuebereinstimmung
    // geprueft

    sal_uInt32  nFirstLong;
    sal_uInt8   nFirstBytes[20];

    rStm.Seek( nStmPos );
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm >> nFirstLong;
    rStm.SeekRel( -4 );
    rStm.Read( &nFirstBytes, 20 );

    if ( ( nFirstLong == 0xC5D0D3C6 ) || ( aPathExt.CompareToAscii( "eps", 3 ) == COMPARE_EQUAL ) ||
        ( ImplSearchEntry( nFirstBytes, (sal_uInt8*)"%!PS-Adobe", 10, 10 )
            && ImplSearchEntry( &nFirstBytes[15], (sal_uInt8*)"EPS", 3, 3 ) ) )
    {
        nFormat = GFF_EPS;
        return sal_True;
    }
    else
        return sal_False;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectDXF( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "dxf", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_DXF;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectMET( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "met", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_MET;

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectPCT( SvStream& rStm, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "pct", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_PCT;
    else
    {
        sal_uInt8 sBuf[4];

        rStm.Seek( nStmPos + 522 );
        rStm.Read( sBuf, 3 );

        if( !rStm.GetError() )
        {
            if ( ( sBuf[0] == 0x00 ) && ( sBuf[1] == 0x11 ) &&
                 ( ( sBuf[2] == 0x01 ) || ( sBuf[2] == 0x02 ) ) )
            {
                bRet = sal_True;
                nFormat = GFF_PCT;
            }
        }
    }

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectSGF( SvStream& rStm, sal_Bool )
{
    sal_Bool bRet = sal_False;

    if( aPathExt.CompareToAscii( "sgf", 3 ) == COMPARE_EQUAL )
        bRet = sal_True;
    else
    {
        sal_uInt8 nFirst, nSecond;

        rStm.Seek( nStmPos );
        rStm >> nFirst >> nSecond;

        if( nFirst == 'J' && nSecond == 'J' )
            bRet = sal_True;
    }

    if( bRet )
        nFormat = GFF_SGF;

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectSGV( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "sgv", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_SGV;

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectSVM( SvStream& rStm, sal_Bool bExtendedInfo )
{
    sal_uInt32  n32;
    sal_Bool    bRet = sal_False;
    sal_uInt8   cByte;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm.Seek( nStmPos );

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

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectWMF( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "wmf",3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_WMF;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool GraphicDescriptor::ImpDetectEMF( SvStream&, sal_Bool )
{
    sal_Bool bRet = aPathExt.CompareToAscii( "emf", 3 ) == COMPARE_EQUAL;
    if (bRet)
        nFormat = GFF_EMF;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

String GraphicDescriptor::GetImportFormatShortName( sal_uInt16 nFormat )
{
    ByteString          aKeyName;

    switch( nFormat )
    {
        case( GFF_BMP ) :   aKeyName = "bmp";   break;
        case( GFF_GIF ) :   aKeyName = "gif";   break;
        case( GFF_JPG ) :   aKeyName = "jpg";   break;
        case( GFF_PCD ) :   aKeyName = "pcd";   break;
        case( GFF_PCX ) :   aKeyName = "pcx";   break;
        case( GFF_PNG ) :   aKeyName = "png";   break;
        case( GFF_XBM ) :   aKeyName = "xbm";   break;
        case( GFF_XPM ) :   aKeyName = "xpm";   break;
        case( GFF_PBM ) :   aKeyName = "pbm";   break;
        case( GFF_PGM ) :   aKeyName = "pgm";   break;
        case( GFF_PPM ) :   aKeyName = "ppm";   break;
        case( GFF_RAS ) :   aKeyName = "ras";   break;
        case( GFF_TGA ) :   aKeyName = "tga";   break;
        case( GFF_PSD ) :   aKeyName = "psd";   break;
        case( GFF_EPS ) :   aKeyName = "eps";   break;
        case( GFF_TIF ) :   aKeyName = "tif";   break;
        case( GFF_DXF ) :   aKeyName = "dxf";   break;
        case( GFF_MET ) :   aKeyName = "met";   break;
        case( GFF_PCT ) :   aKeyName = "pct";   break;
        case( GFF_SGF ) :   aKeyName = "sgf";   break;
        case( GFF_SGV ) :   aKeyName = "sgv";   break;
        case( GFF_SVM ) :   aKeyName = "svm";   break;
        case( GFF_WMF ) :   aKeyName = "wmf";   break;
        case( GFF_EMF ) :   aKeyName = "emf";   break;
    }

    return String( aKeyName, RTL_TEXTENCODING_ASCII_US );
}
