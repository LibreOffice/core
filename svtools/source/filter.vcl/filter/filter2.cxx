/*************************************************************************
 *
 *  $RCSfile: filter2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sj $ $Date: 2000-11-09 19:49:57 $
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

#include <string.h>
#include <stdio.h>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <vcl/outdev.hxx>
#include <vcl/config.hxx>
#include "filter.hxx"
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif

#define DATA_SIZE           640
#define IMP_FILTERSECTION   "Graphics Filters - Import"

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
        aPathExt = aURL.GetFileExtension().ToLowerAscii();
    }
    bLinked = TRUE;
    bLinkChanged = FALSE;
    bWideSearch = FALSE;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphicDescriptor::GraphicDescriptor( const INetURLObject& rPath ) :
    pFileStm( ::utl::UcbStreamHelper::CreateStream( rPath.GetMainURL(), STREAM_READ ) ),
    aPathExt( rPath.GetFileExtension().ToLowerAscii() )
{
    if ( pFileStm )
    {
        nStmPos = 0;
        pFileStm->Seek( nStmPos );
        bDataReady = TRUE;
    }
    ImpConstruct();
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
        aPathExt = aURL.GetFileExtension().ToLowerAscii();
    }
    nStmPos = rInStream.Tell();
    pBaseStm = &rInStream;
    bBaseStm = TRUE;

    if ( !pBaseStm->GetError() )
        bDataReady = TRUE;
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

BOOL GraphicDescriptor::Detect( BOOL bExtendedInfo )
{
    BOOL bRet = FALSE;

    // Link-Status ueberpruefen
    if ( bLinked && bLinkChanged )
    {
        DBG_ASSERT( aReqLink.IsSet(), "Wo ist der RequestHandler???" );
        if ( pMemStm = (SvStream*) aReqLink.Call( this ) )
        {
            nStmPos = pMemStm->Tell();
            bDataReady = TRUE;
        }
    }

    if ( bDataReady )
    {
        SvStream&   rStm = GetSearchStream();
        UINT16      nOldFormat = rStm.GetNumberFormatInt();

        if      ( ImpDetectGIF( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectJPG( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectBMP( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectPNG( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectTIF( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectPCX( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectDXF( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectMET( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectSGF( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectSGV( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectSVM( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectWMF( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectEMF( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectPCT( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectXBM( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectXPM( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectPBM( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectPGM( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectPPM( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectRAS( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectTGA( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectPSD( rStm, bExtendedInfo ) ) bRet = TRUE;
        else if ( ImpDetectEPS( rStm, bExtendedInfo ) ) bRet = TRUE;

        // diese Formate lassen sich nur bei WideSearch im gesamten
        // Stream ermitteln
        else if ( bWideSearch )
        {
            if ( ImpDetectPCD( rStm, bExtendedInfo ) )
                bRet = TRUE;
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

BOOL GraphicDescriptor::IsDataReady() const
{
    return bDataReady;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::IsWideSearch() const
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
    bLinkChanged = TRUE;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

ULONG GraphicDescriptor::GetRequestedByteCount() const
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
    bCompressed = FALSE;
    bDataReady = FALSE;
    bLinked = FALSE;
    bWideSearch = TRUE;
    bBaseStm = FALSE;
    pMemStm = NULL;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectBMP( SvStream& rStm, BOOL bExtendedInfo )
{
    UINT16  nTemp16;
    BOOL    bRet = FALSE;

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
        bRet = TRUE;

        if ( bExtendedInfo )
        {
            UINT32  nTemp32;
            UINT32  nCompression;

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
                bRet = FALSE;
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

BOOL GraphicDescriptor::ImpDetectGIF( SvStream& rStm, BOOL bExtendedInfo )
{
    UINT32  nTemp32;
    UINT16  nTemp16;
    BOOL    bRet = FALSE;
    BYTE    cByte;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm.Seek( nStmPos );

    rStm >> nTemp32;
    if ( nTemp32 == 0x38464947 )
    {
        rStm >> nTemp16;
        if ( ( nTemp16 == 0x6137 ) || ( nTemp16 == 0x6139 ) )
        {
            nFormat = GFF_GIF;
            bRet = TRUE;

            if ( bExtendedInfo )
            {
                UINT16 nTemp16;

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

BOOL GraphicDescriptor::ImpDetectJPG( SvStream& rStm,  BOOL bExtendedInfo )
{
    UINT32  nTemp32;
    BOOL    bRet = FALSE;
    BYTE    cByte;
    BOOL    bM_COM;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm.Seek( nStmPos );

    rStm >> nTemp32;

    // compare upper 28 bits
    if( 0xffd8ff00 == ( nTemp32 & 0xffffff00 ) )
    {
        nFormat = GFF_JPG;
        return TRUE;
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
            bRet = TRUE;

            if( bExtendedInfo )
            {
                MapMode aMap;
                UINT16  nTemp16;
                ULONG   nCount = 9;
                ULONG   nMax;
                ULONG   nResX;
                ULONG   nResY;
                BYTE    cUnit;

                // Groesse des verbleibenden Puffers ermitteln
                if ( bLinked )
                    nMax = ( (SvMemoryStream&) rStm ).GetSize() - 16;
                else
                    nMax = DATA_SIZE - 16;

                // max. 8K
                nMax = Min( nMax, (ULONG) 8192 );

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

BOOL GraphicDescriptor::ImpDetectPCD( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL    bRet = FALSE;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm.Seek( nStmPos );

    if ( bWideSearch )
    {
        UINT32  nTemp32;
        UINT16  nTemp16;
        BYTE    cByte;

        rStm.SeekRel( 2048 );
        rStm >> nTemp32;
        rStm >> nTemp16;
        rStm >> cByte;

        if ( ( nTemp32 == 0x5f444350 ) &&
             ( nTemp16 == 0x5049 ) &&
             ( cByte == 0x49 ) )
        {
            nFormat = GFF_PCD;
            bRet = TRUE;
        }
    }
    else if ( bRet = ( aPathExt.CompareToAscii( "pcd", 3 ) == COMPARE_EQUAL ) )
        nFormat = GFF_PCD;

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectPCX( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL    bRet = FALSE;
    BYTE    cByte;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm.Seek( nStmPos );

    rStm >> cByte;
    if ( cByte == 0x0a )
    {
        nFormat = GFF_PCX;
        bRet = TRUE;

        if ( bExtendedInfo )
        {
            UINT16  nTemp16;
            USHORT  nXmin;
            USHORT  nXmax;
            USHORT  nYmin;
            USHORT  nYmax;
            USHORT  nDPIx;
            USHORT  nDPIy;


            rStm.SeekRel( 1 );

            // Kompression lesen
            rStm >> cByte;
            bCompressed = ( cByte > 0 );

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
        }
    }

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectPNG( SvStream& rStm, BOOL bExtendedInfo )
{
    UINT32  nTemp32;
    BOOL    bRet = FALSE;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm.Seek( nStmPos );

    rStm >> nTemp32;
    if ( nTemp32 == 0x89504e47 )
    {
        rStm >> nTemp32;
        if ( nTemp32 == 0x0d0a1a0a )
        {
            nFormat = GFF_PNG;
            bRet = TRUE;

            if ( bExtendedInfo )
            {
                BYTE cByte;

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
                bCompressed = TRUE;

                if ( bWideSearch )
                {
                    BOOL    bOk = FALSE;
                    UINT32  nLen32;

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
                        ULONG   nXRes;
                        ULONG   nYRes;

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

BOOL GraphicDescriptor::ImpDetectTIF( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL    bOk = FALSE;
    BOOL    bRet = FALSE;
    BYTE    cByte1;
    BYTE    cByte2;

    rStm.Seek( nStmPos );
    rStm >> cByte1;
    rStm >> cByte2;
    if ( cByte1 == cByte2 )
    {
        if ( cByte1 == 0x49 )
        {
            rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
            bOk = TRUE;
        }
        else if ( cByte1 == 0x4d )
        {
            rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
            bOk = TRUE;
        }

        if ( bOk )
        {
            UINT16  nTemp16;

            rStm >> nTemp16;
            if ( nTemp16 == 0x2a )
            {
                nFormat = GFF_TIF;
                bRet = TRUE;

                if ( bExtendedInfo )
                {
                    ULONG   nCount;
                    ULONG   nMax = DATA_SIZE - 48;
                    UINT32  nTemp32;
                    BOOL    bOk;

                    // Offset des ersten IFD einlesen
                    rStm >> nTemp32;
                    rStm.SeekRel( ( nCount = ( nTemp32 + 2 ) ) - 0x08 );

                    if ( bWideSearch || ( nCount < nMax ) )
                    {
                        // Tag's lesen, bis wir auf Tag256 ( Width ) treffen
                        // nicht mehr Bytes als DATA_SIZE lesen
                        rStm >> nTemp16;
                        while ( ( nTemp16 != 256 ) && ( bOk = ( bWideSearch || ( nCount < nMax ) ) ) )
                        {
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

BOOL GraphicDescriptor::ImpDetectXBM( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet;

    if ( bRet = ( aPathExt.CompareToAscii( "xbm", 3 ) == COMPARE_EQUAL ) )
        nFormat = GFF_XBM;

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectXPM( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet;

    if ( bRet = ( aPathExt.CompareToAscii( "xpm", 3 ) == COMPARE_EQUAL ) )
        nFormat = GFF_XPM;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectPBM( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet = FALSE;

    // erst auf Datei Extension pruefen, da diese aussagekraeftiger ist
    // als die 2 ID Bytes

    if ( aPathExt.CompareToAscii( "pbm", 3 ) == COMPARE_EQUAL )
        bRet = TRUE;
    else
    {
        BYTE    nFirst, nSecond;
        rStm.Seek( nStmPos );
        rStm >> nFirst >> nSecond;
        if ( nFirst == 'P' && ( ( nSecond == '1' ) || ( nSecond == '4' ) ) )
            bRet = TRUE;
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

BOOL GraphicDescriptor::ImpDetectPGM( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet = FALSE;

    if ( aPathExt.CompareToAscii( "pgm", 3 ) == COMPARE_EQUAL )
        bRet = TRUE;
    else
    {
        BYTE    nFirst, nSecond;
        rStm.Seek( nStmPos );
        rStm >> nFirst >> nSecond;
        if ( nFirst == 'P' && ( ( nSecond == '2' ) || ( nSecond == '5' ) ) )
            bRet = TRUE;
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

BOOL GraphicDescriptor::ImpDetectPPM( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet = FALSE;

    if ( aPathExt.CompareToAscii( "ppm", 3 ) == COMPARE_EQUAL )
        bRet = TRUE;
    else
    {
        BYTE    nFirst, nSecond;
        rStm.Seek( nStmPos );
        rStm >> nFirst >> nSecond;
        if ( nFirst == 'P' && ( ( nSecond == '3' ) || ( nSecond == '6' ) ) )
            bRet = TRUE;
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

BOOL GraphicDescriptor::ImpDetectRAS( SvStream& rStm, BOOL bExtendedInfo )
{
    UINT32 nMagicNumber;
    rStm.Seek( nStmPos );
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm >> nMagicNumber;
    if ( nMagicNumber == 0x59a66a95 )
    {
        nFormat = GFF_RAS;
        return TRUE;
    }
    else
        return FALSE;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectTGA( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet;

    if ( bRet = ( aPathExt.CompareToAscii( "tga", 3 ) == COMPARE_EQUAL ) )
        nFormat = GFF_TGA;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectPSD( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet = FALSE;

    UINT32  nMagicNumber;
    rStm.Seek( nStmPos );
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm >> nMagicNumber;
    if ( nMagicNumber == 0x38425053 )
    {
        UINT16 nVersion;
        rStm >> nVersion;
        if ( nVersion == 1 )
        {
            bRet = TRUE;
            if ( bExtendedInfo )
            {
                UINT16 nChannels;
                UINT32 nRows;
                UINT32 nColumns;
                UINT16 nDepth;
                UINT16 nMode;
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
                            bRet = FALSE;
                    }
                }
                else
                    bRet = FALSE;
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

BOOL GraphicDescriptor::ImpDetectEPS( SvStream& rStm, BOOL bExtendedInfo )
{
    // es wird die EPS mit Vorschaubild Variante und die Extensionübereinstimmung
    // geprüft

    UINT32 nFirstLong;
    rStm.Seek( nStmPos );
    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStm >> nFirstLong;
    if ( ( nFirstLong == 0xC5D0D3C6 ) || ( aPathExt.CompareToAscii( "eps", 3 ) == COMPARE_EQUAL ) )
    {
        nFormat = GFF_EPS;
        return TRUE;
    }
    else
        return FALSE;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectDXF( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet;

    if ( bRet = ( aPathExt.CompareToAscii( "dxf", 3 ) == COMPARE_EQUAL ) )
        nFormat = GFF_DXF;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectMET( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet;

    if ( bRet = ( aPathExt.CompareToAscii( "met", 3 ) == COMPARE_EQUAL ) )
        nFormat = GFF_MET;

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectPCT( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet;

    if ( bRet = ( aPathExt.CompareToAscii( "pct", 3 ) == COMPARE_EQUAL ) )
        nFormat = GFF_PCT;
    else
    {
        BYTE sBuf[4];

        rStm.Seek( nStmPos + 522 );
        rStm.Read( sBuf, 3 );

        if( !rStm.GetError() )
        {
            if ( ( sBuf[0] == 0x00 ) && ( sBuf[1] == 0x11 ) &&
                 ( ( sBuf[2] == 0x01 ) || ( sBuf[2] == 0x02 ) ) )
            {
                bRet = TRUE;
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

BOOL GraphicDescriptor::ImpDetectSGF( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet;

    if ( bRet = ( aPathExt.CompareToAscii( "sgf",3 ) == COMPARE_EQUAL ) )
        nFormat = GFF_SGF;

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectSGV( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet;

    if ( bRet = ( aPathExt.CompareToAscii( "sgv", 3 ) == COMPARE_EQUAL ) )
        nFormat = GFF_SGV;

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectSVM( SvStream& rStm, BOOL bExtendedInfo )
{
    UINT32  nTemp32;
    BOOL    bRet = FALSE;
    BYTE    cByte;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStm.Seek( nStmPos );

    rStm >> nTemp32;
    if ( nTemp32 == 0x44475653 )
    {
        rStm >> cByte;
        if ( cByte == 0x49 )
        {
            nFormat = GFF_SVM;
            bRet = TRUE;

            if ( bExtendedInfo )
            {
                UINT32  nTemp32;
                UINT16  nTemp16;

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
        rStm >> nTemp32;

        if( nTemp32 == 0x4D4C4356 )
        {
            UINT16 nTmp16;

            rStm >> nTmp16;

            if( nTmp16 == 0x4654 )
            {
                nFormat = GFF_SVM;
                bRet = TRUE;

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

BOOL GraphicDescriptor::ImpDetectWMF( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet;

    if ( bRet = ( aPathExt.CompareToAscii( "wmf",3 ) == COMPARE_EQUAL ) )
        nFormat = GFF_WMF;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL GraphicDescriptor::ImpDetectEMF( SvStream& rStm, BOOL bExtendedInfo )
{
    BOOL bRet;

    if ( bRet = ( aPathExt.CompareToAscii( "emf", 3 ) == COMPARE_EQUAL ) )
        nFormat = GFF_EMF;

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

USHORT GraphicDescriptor::GetImportFormatNumber( USHORT nFormat, Config& rConfig )
{
    ByteString          aKeyName;
    const ByteString    aOldGroup( rConfig.GetGroup() );
    USHORT              nKeyNumber = GRFILTER_FORMAT_NOTFOUND;

    rConfig.SetGroup( IMP_FILTERSECTION );

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

    if( aKeyName.Len() )
    {
        for( USHORT i = 0, nCount = rConfig.GetKeyCount(); i < nCount; i++ )
        {
            if( rConfig.GetKeyName( i ).CompareIgnoreCaseToAscii( aKeyName ) == COMPARE_EQUAL )
            {
                nKeyNumber = i;
                break;
            }
        }
    }

    rConfig.SetGroup( aOldGroup );

    return nKeyNumber;
}
