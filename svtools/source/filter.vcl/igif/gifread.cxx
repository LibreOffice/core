/*************************************************************************
 *
 *  $RCSfile: gifread.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:59 $
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

#define _GIFPRIVATE

#include <tools/new.hxx>
#include "decode.hxx"
#include "gifread.hxx"

// -----------
// - Defines -
// -----------

#define NO_PENDING( rStm ) ( ( rStm ).GetError() != ERRCODE_IO_PENDING )

// -------------
// - GIFReader -
// -------------

GIFReader::GIFReader( SvStream& rStm, void* pCallData ) :
            rIStm           ( rStm ),
            aGPalette       ( 256 ),
            aLPalette       ( 256 ),
            pAcc8           ( NULL ),
            pAcc1           ( NULL ),
            nLastPos        ( rStm.Tell() ),
            eActAction      ( GLOBAL_HEADER_READING ),
            bImGraphicReady ( FALSE ),
            nLoops          ( 1 ),
            bGCTransparent  ( FALSE ),
            nLogWidth100    ( 0UL ),
            nLogHeight100   ( 0UL )
{
    maUpperName = UniString::CreateFromAscii( "SVIGIF", 6 );
    pSrcBuf = new BYTE[ 256 ];
    ClearImageExtensions();
}

// ------------------------------------------------------------------------

GIFReader::~GIFReader()
{
    aImGraphic.SetContext( NULL );

    if( pAcc1 )
        aBmp1.ReleaseAccess( pAcc1 );

    if( pAcc8 )
        aBmp8.ReleaseAccess( pAcc8 );

    delete[] pSrcBuf;
}

// ------------------------------------------------------------------------

void GIFReader::ClearImageExtensions()
{
    nGCDisposalMethod = 0;
    bGCTransparent = FALSE;
    nTimer = 0;
}

// ------------------------------------------------------------------------

BOOL GIFReader::CreateBitmaps( long nWidth, long nHeight, BitmapPalette* pPal,
                               BOOL bWatchForBackgroundColor )
{
    const Size aSize( nWidth, nHeight );

    if( bGCTransparent )
    {
        const Color aWhite( COL_WHITE );

        aBmp1 = Bitmap( aSize, 1 );

        if( !aAnimation.Count() )
            aBmp1.Erase( aWhite );

        pAcc1 = aBmp1.AcquireWriteAccess();

        if( pAcc1 )
        {
            cTransIndex1 = (BYTE) pAcc1->GetBestPaletteIndex( aWhite );
            cNonTransIndex1 = cTransIndex1 ? 0 : 1;
        }
        else
            bStatus = FALSE;
    }

    if( bStatus )
    {
        aBmp8 = Bitmap( aSize, 8, pPal );

        if( !!aBmp8 && bWatchForBackgroundColor && aAnimation.Count() )
            aBmp8.Erase( (*pPal)[ nBackgroundColor ] );
        else
          aBmp8.Erase( Color( COL_WHITE ) );

        pAcc8 = aBmp8.AcquireWriteAccess();
        bStatus = ( pAcc8 != NULL );
    }

    return bStatus;
}

// ------------------------------------------------------------------------

BOOL GIFReader::ReadGlobalHeader()
{
    char    pBuf[ 7 ];
    BYTE    nRF;
    BYTE    nAspect;
    BOOL    bRet = FALSE;

    rIStm.Read( pBuf, 6 );
    if( NO_PENDING( rIStm ) )
    {
        pBuf[ 6 ] = 0;
        if( !strcmp( pBuf, "GIF87a" ) || !strcmp( pBuf, "GIF89a" ) )
        {
            rIStm.Read( pBuf, 7 );
            if( NO_PENDING( rIStm ) )
            {
                SvMemoryStream aMemStm;

                aMemStm.SetBuffer( pBuf, 7, FALSE, 7 );
                aMemStm >> nGlobalWidth;
                aMemStm >> nGlobalHeight;
                aMemStm >> nRF;
                aMemStm >> nBackgroundColor;
                aMemStm >> nAspect;

                bGlobalPalette = (BOOL) ( nRF & 0x80 );

                if( bGlobalPalette )
                    ReadPaletteEntries( &aGPalette, 1 << ( ( nRF & 7 ) + 1 ) );
                else
                    nBackgroundColor = 0;

                if( NO_PENDING( rIStm ) )
                    bRet = TRUE;
            }
        }
        else
            bStatus = FALSE;
    }

    return bRet;
}

// ------------------------------------------------------------------------

void GIFReader::ReadPaletteEntries( BitmapPalette* pPal, ULONG nCount )
{
    const ULONG nLen = 3UL * nCount;
    BYTE*       pBuf = new BYTE[ nLen ];

    rIStm.Read( pBuf, nLen );
    if( NO_PENDING( rIStm ) )
    {
        BYTE* pTmp = pBuf;

        for( ULONG i = 0UL; i < nCount; )
        {
            BitmapColor& rColor = (*pPal)[ (USHORT) i++ ];

            rColor.SetRed( *pTmp++ );
            rColor.SetGreen( *pTmp++ );
            rColor.SetBlue( *pTmp++ );
        }

        // nach Moeglichkeit noch einige Standardfarben unterbringen
        if( nCount < 256UL )
        {
            (*pPal)[ 255UL ] = Color( COL_WHITE );

            if( nCount < 255UL )
                (*pPal)[ 254UL ] = Color( COL_BLACK );
        }
    }

    delete[] pBuf;
}

// ------------------------------------------------------------------------

BOOL GIFReader::ReadExtension()
{
    BYTE    cFunction;
    BYTE    cSize;
    BYTE    cByte;
    BOOL    bRet = FALSE;
    BOOL    bOverreadDataBlocks = FALSE;

    // Extension-Label
    rIStm >> cFunction;
    if( NO_PENDING( rIStm ) )
    {
        // Block-Laenge
        rIStm >> cSize;

        switch( cFunction )
        {
            // 'Graphic Control Extension'
            case( 0xf9 ) :
            {
                BYTE cFlags;

                rIStm >> cFlags;
                rIStm >> nTimer;
                rIStm >> nGCTransparentIndex;
                rIStm >> cByte;

                if ( NO_PENDING( rIStm ) )
                {
                    nGCDisposalMethod = ( cFlags >> 2) & 7;
                    bGCTransparent = ( cFlags & 1 ) ? TRUE : FALSE;
                    bStatus = ( cSize == 4 ) && ( cByte == 0 );
                    bRet = TRUE;
                }
            }
            break;

            // Application-Extension
            case ( 0xff ) :
            {
                if ( NO_PENDING( rIStm ) )
                {
                    // default diese Extension ueberlesen
                    bOverreadDataBlocks = TRUE;

                    // Appl.-Extension hat Laenge 11
                    if ( cSize == 0x0b )
                    {
                        ByteString  aAppId;
                        ByteString  aAppCode;

                        rIStm.Read( aAppId.AllocBuffer( 8 ), 8 );
                        rIStm.Read( aAppCode.AllocBuffer( 3 ), 3 );
                        rIStm >> cSize;

                        // NetScape-Extension
                        if( aAppId == "NETSCAPE" && aAppCode == "2.0" && cSize == 3 )
                        {
                            rIStm >> cByte;

                            // Loop-Extension
                            if ( cByte == 0x01 )
                            {
                                rIStm >> cByte;
                                nLoops = cByte;
                                rIStm >> cByte;
                                nLoops |= ( (USHORT) cByte << 8 );
                                rIStm >> cByte;

                                bStatus = ( cByte == 0 );
                                bRet = NO_PENDING( rIStm );
                                bOverreadDataBlocks = FALSE;

                                // Netscape interpretiert den LoopCount
                                // als reine Anzahl der _Wiederholungen_;
                                // bei uns ist es die Gesamtanzahl der
                                // Durchlaeufe
                                if( nLoops )
                                    nLoops++;
                            }
                            else
                                rIStm.SeekRel( -1 );
                        }
                        else if ( aAppId == "STARDIV " && aAppCode == "5.0" && cSize == 9 )
                        {
                            rIStm >> cByte;

                            // Loop-Extension
                            if ( cByte == 0x01 )
                            {
                                rIStm >> nLogWidth100 >> nLogHeight100;
                                rIStm >> cByte;
                                bStatus = ( cByte == 0 );
                                bRet = NO_PENDING( rIStm );
                                bOverreadDataBlocks = FALSE;
                            }
                            else
                                rIStm.SeekRel( -1 );
                        }

                    }
                }
            }
            break;

            // alles andere ueberlesen
            default:
                bOverreadDataBlocks = TRUE;
            break;
        }

        // Sub-Blocks ueberlesen
        if ( bOverreadDataBlocks )
        {
            bRet = TRUE;
            while( cSize && bStatus && !rIStm.IsEof() )
            {
                USHORT  nCount = (USHORT) cSize + 1;
                char*   pBuffer = new char[ nCount ];

                bRet = FALSE;
                rIStm.Read( pBuffer, nCount );
                if( NO_PENDING( rIStm ) )
                {
                    cSize = (BYTE) pBuffer[ cSize ];
                    bRet = TRUE;
                }
                else
                    cSize = 0;

                delete[] pBuffer;
            }
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL GIFReader::ReadLocalHeader()
{
    BYTE    pBuf[ 9 ];
    BOOL    bRet = FALSE;

    rIStm.Read( pBuf, 9 );
    if( NO_PENDING( rIStm ) )
    {
        SvMemoryStream  aMemStm;
        BitmapPalette*  pPal;
        BYTE            nFlags;

        aMemStm.SetBuffer( (char*) pBuf, 9, FALSE, 9 );
        aMemStm >> nImagePosX;
        aMemStm >> nImagePosY;
        aMemStm >> nImageWidth;
        aMemStm >> nImageHeight;
        aMemStm >> nFlags;

        // Falls Interlaced, ersten Startwert vorgeben
        bInterlaced = ( ( nFlags & 0x40 ) == 0x40 );
        nLastInterCount = 7;
        nLastImageY = 0;

        if( nFlags & 0x80 )
        {
            pPal = &aLPalette;
            ReadPaletteEntries( pPal, 1 << ( (nFlags & 7 ) + 1 ) );
        }
        else
            pPal = &aGPalette;

        // Falls alles soweit eingelesen werden konnte, kann
        // nun das lokale Bild angelegt werden;
        // es wird uebergeben, ob der BackgroundColorIndex evtl.
        // beruecksichtigt werden soll ( wenn Globale Farbtab. und
        // diese auch fuer dieses Bild gilt )
        if( NO_PENDING( rIStm ) )
        {
            CreateBitmaps( nImageWidth, nImageHeight, pPal, bGlobalPalette && ( pPal == &aGPalette ) );
            bRet = TRUE;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

ULONG GIFReader::ReadNextBlock()
{
    ULONG   nRet = 0UL;
    ULONG   nRead;
    BYTE    cBlockSize;

    rIStm >> cBlockSize;

    if ( rIStm.IsEof() )
        nRet = 4UL;
    else if ( NO_PENDING( rIStm ) )
    {
        if ( cBlockSize == 0 )
            nRet = 2UL;
        else
        {
            rIStm.Read( pSrcBuf, cBlockSize );

            if( NO_PENDING( rIStm ) )
            {
                if( bOverreadBlock )
                    nRet = 3UL;
                else
                {
                    BOOL    bEOI;
                    HPBYTE  pTarget = pDecomp->DecompressBlock( pSrcBuf, cBlockSize, nRead, bEOI );

                    nRet = ( bEOI ? 3 : 1 );

                    if( nRead && !bOverreadBlock )
                        FillImages( pTarget, nRead );

                    SvMemFree( pTarget );
                }
            }
        }
    }

    return nRet;
}

// ------------------------------------------------------------------------

void GIFReader::FillImages( HPBYTE pBytes, ULONG nCount )
{
    for( ULONG i = 0UL; i < nCount; i++ )
    {
        if( nImageX >= nImageWidth )
        {
            if( bInterlaced )
            {
                long nT1, nT2;

                // falls Interlaced, werden die Zeilen kopiert
                if( nLastInterCount )
                {
                    long nMinY = Min( (long) nLastImageY + 1, (long) nImageHeight - 1 );
                    long nMaxY = Min( (long) nLastImageY + nLastInterCount, (long) nImageHeight - 1 );

                    // letzte gelesene Zeile kopieren, wenn Zeilen
                    // nicht zusanmmenfallen ( kommt vorm wenn wir am Ende des Bildes sind )
                    if( ( nMinY > nLastImageY ) && ( nLastImageY < ( nImageHeight - 1 ) ) )
                    {
                        HPBYTE  pScanline8 = pAcc8->GetScanline( nYAcc );
                        ULONG   nSize8 = pAcc8->GetScanlineSize();
                        HPBYTE  pScanline1;
                        ULONG   nSize1;

                        if( bGCTransparent )
                        {
                            pScanline1 = pAcc1->GetScanline( nYAcc );
                            nSize1 = pAcc1->GetScanlineSize();
                        }

                        for( long j = nMinY; j <= nMaxY; j++ )
                        {
                            HMEMCPY( pAcc8->GetScanline( j ), pScanline8, nSize8 );

                            if( bGCTransparent )
                                HMEMCPY( pAcc1->GetScanline( j ), pScanline1, nSize1 );
                        }
                    }
                }

                nT1 = ( ++nImageY ) << 3;
                nLastInterCount = 7;

                if( nT1 >= nImageHeight )
                {
                    nT2 = nImageY - ( ( nImageHeight + 7 ) >> 3 );
                    nT1 = ( nT2 << 3 ) + 4;
                    nLastInterCount = 3;

                    if( nT1 >= nImageHeight )
                    {
                        nT2 -= ( nImageHeight + 3 ) >> 3;
                        nT1 = ( nT2 << 2 ) + 2;
                        nLastInterCount = 1;

                        if( nT1 >= nImageHeight )
                        {
                            nT2 -= ( nImageHeight + 1 ) >> 2;
                            nT1 = ( nT2 << 1 ) + 1;
                            nLastInterCount = 0;
                        }
                    }
                }

                nLastImageY = (USHORT) nT1;
                nYAcc = nT1;
            }
            else
            {
                nLastImageY = ++nImageY;
                nYAcc = nImageY;
            }

            // Zeile faengt von vorne an
            nImageX = 0;
        }

        if( nImageY < nImageHeight )
        {
            const BYTE cTmp = pBytes[ i ];

            if( bGCTransparent )
            {
                if( cTmp == nGCTransparentIndex )
                    pAcc1->SetPixel( nYAcc, nImageX++, cTransIndex1 );
                else
                {
                    pAcc8->SetPixel( nYAcc, nImageX, cTmp );
                    pAcc1->SetPixel( nYAcc, nImageX++, cNonTransIndex1 );
                }
            }
            else
                pAcc8->SetPixel( nYAcc, nImageX++, cTmp );
        }
        else
        {
            bOverreadBlock = TRUE;
            break;
        }
    }
}

// ------------------------------------------------------------------------

void GIFReader::CreateNewBitmaps()
{
    AnimationBitmap aAnimBmp;

    aBmp8.ReleaseAccess( pAcc8 );
    pAcc8 = NULL;

    if( bGCTransparent )
    {
        aBmp1.ReleaseAccess( pAcc1 );
        pAcc1 = NULL;
        aAnimBmp.aBmpEx = BitmapEx( aBmp8, aBmp1 );
    }
    else
        aAnimBmp.aBmpEx = BitmapEx( aBmp8 );

    aAnimBmp.aPosPix = Point( nImagePosX, nImagePosY );
    aAnimBmp.aSizePix = Size( nImageWidth, nImageHeight );
    aAnimBmp.nWait = ( nTimer != 65535 ) ? nTimer : ANIMATION_TIMEOUT_ON_CLICK;
    aAnimBmp.bUserInput = FALSE;

    if( nGCDisposalMethod == 2 )
        aAnimBmp.eDisposal = DISPOSE_BACK;
    else if( nGCDisposalMethod == 3 )
        aAnimBmp.eDisposal = DISPOSE_PREVIOUS;
    else
        aAnimBmp.eDisposal = DISPOSE_NOT;

    aAnimation.Insert( aAnimBmp );

    if( aAnimation.Count() == 1 )
    {
        aAnimation.SetDisplaySizePixel( Size( nGlobalWidth, nGlobalHeight ) );
        aAnimation.SetLoopCount( nLoops );
    }
}

// ------------------------------------------------------------------------

const Graphic& GIFReader::GetIntermediateGraphic()
{
    // Intermediate-Graphic nur erzeugen, wenn schon
    // Daten vorliegen, aber die Graphic noch nicht
    // vollstaendig eingelesen wurde
    if ( bImGraphicReady && !aAnimation.Count() )
    {
        Bitmap  aBmp;

        aBmp8.ReleaseAccess( pAcc8 );

        if ( bGCTransparent )
        {
            aBmp1.ReleaseAccess( pAcc1 );
            aImGraphic = BitmapEx( aBmp8, aBmp1 );

            pAcc1 = aBmp1.AcquireWriteAccess();
            bStatus = bStatus && ( pAcc1 != NULL );
        }
        else
            aImGraphic = aBmp8;

        pAcc8 = aBmp8.AcquireWriteAccess();
        bStatus = bStatus && ( pAcc8 != NULL );
    }

    return aImGraphic;
}

// ------------------------------------------------------------------------

BOOL GIFReader::ProcessGIF()
{
    BOOL bRead = FALSE;
    BOOL bEnd = FALSE;

    if ( !bStatus )
        eActAction = ABORT_READING;

    // Stream an die richtige Stelle bringen
    rIStm.Seek( nLastPos );

    switch( eActAction )
    {
        // naechsten Marker lesen
        case( MARKER_READING ):
        {
            BYTE cByte;

            rIStm >> cByte;

            if( rIStm.IsEof() )
                eActAction = END_READING;
            else if( NO_PENDING( rIStm ) )
            {
                bRead = TRUE;

                if( cByte == '!' )
                    eActAction = EXTENSION_READING;
                else if( cByte == ',' )
                    eActAction = LOCAL_HEADER_READING;
                else if( cByte == ';' )
                    eActAction = END_READING;
                else
                    eActAction = ABORT_READING;
            }
        }
        break;

        // ScreenDescriptor lesen
        case( GLOBAL_HEADER_READING ):
        {
            if( ( bRead = ReadGlobalHeader() ) == TRUE )
            {
                ClearImageExtensions();
                eActAction = MARKER_READING;
            }
        }
        break;


        // Extension lesen
        case( EXTENSION_READING ):
        {
            if( ( bRead = ReadExtension() ) == TRUE )
                eActAction = MARKER_READING;
        }
        break;


        // Image-Descriptor lesen
        case( LOCAL_HEADER_READING ):
        {
            if( ( bRead = ReadLocalHeader() ) == TRUE )
            {
                nYAcc = nImageX = nImageY = 0;
                eActAction = FIRST_BLOCK_READING;
            }
        }
        break;


        // ersten Datenblock lesen
        case( FIRST_BLOCK_READING ):
        {
            BYTE cDataSize;

            rIStm >> cDataSize;

            if( rIStm.IsEof() )
                eActAction = ABORT_READING;
            else if( cDataSize > 12 )
                bStatus = FALSE;
            else if( NO_PENDING( rIStm ) )
            {
                bRead = TRUE;
                pDecomp = new GIFLZWDecompressor( cDataSize );
                eActAction = NEXT_BLOCK_READING;
                bOverreadBlock = FALSE;
            }
            else
                eActAction = FIRST_BLOCK_READING;
        }
        break;

        // naechsten Datenblock lesen
        case( NEXT_BLOCK_READING ):
        {
            USHORT  nLastImageX = nImageX;
            USHORT  nLastImageY = nImageY;
            ULONG   nRet = ReadNextBlock();

            // Return: 0:Pending / 1:OK; / 2:OK und letzter Block: / 3:EOI / 4:HardAbort
            if( nRet )
            {
                bRead = TRUE;

                if ( nRet == 1UL )
                {
                    bImGraphicReady = TRUE;
                    eActAction = NEXT_BLOCK_READING;
                    bOverreadBlock = FALSE;
                }
                else
                {
                    if( nRet == 2UL )
                    {
                        delete pDecomp;
                        CreateNewBitmaps();
                        eActAction = MARKER_READING;
                        ClearImageExtensions();
                    }
                    else if( nRet == 3UL )
                    {
                        eActAction = NEXT_BLOCK_READING;
                        bOverreadBlock = TRUE;
                    }
                    else
                    {
                        delete pDecomp;
                        CreateNewBitmaps();
                        eActAction = ABORT_READING;
                        ClearImageExtensions();
                    }
                }
            }
            else
            {
                nImageX = nLastImageX;
                nImageY = nLastImageY;
            }
        }
        break;

        // ein Fehler trat auf
        case( ABORT_READING ):
        {
            bEnd = TRUE;
            eActAction = END_READING;
        }
        break;

        default:
        break;
    }

    // Stream an die richtige Stelle bringen,
    // falls Daten gelesen werden konnten
    // entweder alte Position oder aktuelle Position
    if( bRead || bEnd )
        nLastPos = rIStm.Tell();

    return bRead;
}

// ------------------------------------------------------------------------

ReadState GIFReader::ReadGIF( Graphic& rGraphic )
{
    ReadState eReadState;

    bStatus = TRUE;

    while( ProcessGIF() && ( eActAction != END_READING ) ) {}

    if( !bStatus )
        eReadState = GIFREAD_ERROR;
    else if( eActAction == END_READING )
        eReadState = GIFREAD_OK;
    else
    {
        if ( rIStm.GetError() == ERRCODE_IO_PENDING )
            rIStm.ResetError();

        eReadState = GIFREAD_NEED_MORE;
    }

    if( aAnimation.Count() == 1 )
    {
        rGraphic = aAnimation.Get( 0 ).aBmpEx;

        if( nLogWidth100 && nLogHeight100 )
        {
            rGraphic.SetPrefSize( Size( nLogWidth100, nLogHeight100 ) );
            rGraphic.SetPrefMapMode( MAP_100TH_MM );
        }
    }
    else
        rGraphic = aAnimation;

    return eReadState;
}


// -------------
// - ImportGIF -
// -------------

BOOL ImportGIF( SvStream & rStm, Graphic& rGraphic, void* pCallerData )
{
    GIFReader*  pGIFReader = (GIFReader*) rGraphic.GetContext();
    USHORT      nOldFormat = rStm.GetNumberFormatInt();
    ReadState   eReadState;
    BOOL        bRet = TRUE;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    if( !pGIFReader )
        pGIFReader = new GIFReader( rStm, pCallerData );

    rGraphic.SetContext( NULL );
    eReadState = pGIFReader->ReadGIF( rGraphic );

    if( eReadState == GIFREAD_ERROR )
    {
        bRet = FALSE;
        delete pGIFReader;
    }
    else if( eReadState == GIFREAD_OK )
        delete pGIFReader;
    else
    {
        rGraphic = pGIFReader->GetIntermediateGraphic();
        rGraphic.SetContext( pGIFReader );
    }

    rStm.SetNumberFormatInt( nOldFormat );

    return bRet;
}
