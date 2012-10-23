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


#define _GIFPRIVATE

#include "decode.hxx"
#include "gifread.hxx"

// -----------
// - Defines -
// -----------

#define NO_PENDING( rStm ) ( ( rStm ).GetError() != ERRCODE_IO_PENDING )

// -------------
// - GIFReader -
// -------------

GIFReader::GIFReader( SvStream& rStm ) :
            aGPalette       ( 256 ),
            aLPalette       ( 256 ),
            rIStm           ( rStm ),
            pAcc8           ( NULL ),
            pAcc1           ( NULL ),
            nLastPos        ( rStm.Tell() ),
            nLogWidth100    ( 0UL ),
            nLogHeight100   ( 0UL ),
            nGlobalWidth    ( 0 ),
            nGlobalHeight   ( 0 ),
            nImageWidth     ( 0 ),
            nImageHeight    ( 0 ),
            nLoops          ( 1 ),
            eActAction      ( GLOBAL_HEADER_READING ),
            bGCTransparent  ( sal_False ),
            bImGraphicReady ( sal_False )
{
    maUpperName = rtl::OUString("SVIGIF");
    pSrcBuf = new sal_uInt8[ 256 ];
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
    bGCTransparent = sal_False;
    nTimer = 0;
}

// ------------------------------------------------------------------------

sal_Bool GIFReader::CreateBitmaps( long nWidth, long nHeight, BitmapPalette* pPal,
                               sal_Bool bWatchForBackgroundColor )
{
    const Size aSize( nWidth, nHeight );

#ifdef __LP64__
    // Don't bother allocating a bitmap of a size that would fail on a
    // 32-bit system. We have at least one unit tests that is expected
    // to fail (loading a 65535*65535 size GIF
    // svtools/qa/cppunit/data/gif/fail/CVE-2008-5937-1.gif), but
    // which doesn't fail on 64-bit Mac OS X at least. Why the loading
    // fails on 64-bit Linux, no idea.
    if (nWidth >= 64000 && nHeight >= 64000)
    {
        bStatus = sal_False;
        return bStatus;
    }
#endif

    if( bGCTransparent )
    {
        const Color aWhite( COL_WHITE );

        aBmp1 = Bitmap( aSize, 1 );

        if( !aAnimation.Count() )
            aBmp1.Erase( aWhite );

        pAcc1 = aBmp1.AcquireWriteAccess();

        if( pAcc1 )
        {
            cTransIndex1 = (sal_uInt8) pAcc1->GetBestPaletteIndex( aWhite );
            cNonTransIndex1 = cTransIndex1 ? 0 : 1;
        }
        else
            bStatus = sal_False;
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

sal_Bool GIFReader::ReadGlobalHeader()
{
    char    pBuf[ 7 ];
    sal_uInt8   nRF;
    sal_uInt8   nAspect;
    sal_Bool    bRet = sal_False;

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

                aMemStm.SetBuffer( pBuf, 7, sal_False, 7 );
                aMemStm >> nGlobalWidth;
                aMemStm >> nGlobalHeight;
                aMemStm >> nRF;
                aMemStm >> nBackgroundColor;
                aMemStm >> nAspect;

                bGlobalPalette = (sal_Bool) ( nRF & 0x80 );

                if( bGlobalPalette )
                    ReadPaletteEntries( &aGPalette, 1 << ( ( nRF & 7 ) + 1 ) );
                else
                    nBackgroundColor = 0;

                if( NO_PENDING( rIStm ) )
                    bRet = sal_True;
            }
        }
        else
            bStatus = sal_False;
    }

    return bRet;
}

// ------------------------------------------------------------------------

void GIFReader::ReadPaletteEntries( BitmapPalette* pPal, sal_uLong nCount )
{
    const sal_uLong nLen = 3UL * nCount;
    sal_uInt8*      pBuf = new sal_uInt8[ nLen ];

    rIStm.Read( pBuf, nLen );
    if( NO_PENDING( rIStm ) )
    {
        sal_uInt8* pTmp = pBuf;

        for( sal_uLong i = 0UL; i < nCount; )
        {
            BitmapColor& rColor = (*pPal)[ (sal_uInt16) i++ ];

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

sal_Bool GIFReader::ReadExtension()
{
    sal_uInt8   cFunction;
    sal_uInt8   cSize;
    sal_uInt8   cByte;
    sal_Bool    bRet = sal_False;
    sal_Bool    bOverreadDataBlocks = sal_False;

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
                sal_uInt8 cFlags;

                rIStm >> cFlags;
                rIStm >> nTimer;
                rIStm >> nGCTransparentIndex;
                rIStm >> cByte;

                if ( NO_PENDING( rIStm ) )
                {
                    nGCDisposalMethod = ( cFlags >> 2) & 7;
                    bGCTransparent = ( cFlags & 1 ) ? sal_True : sal_False;
                    bStatus = ( cSize == 4 ) && ( cByte == 0 );
                    bRet = sal_True;
                }
            }
            break;

            // Application-Extension
            case ( 0xff ) :
            {
                if ( NO_PENDING( rIStm ) )
                {
                    // default diese Extension ueberlesen
                    bOverreadDataBlocks = sal_True;

                    // Appl.-Extension hat Laenge 11
                    if ( cSize == 0x0b )
                    {
                        rtl::OString aAppId = read_uInt8s_ToOString(rIStm, 8);
                        rtl::OString aAppCode = read_uInt8s_ToOString(rIStm, 3);
                        rIStm >> cSize;

                        // NetScape-Extension
                        if( aAppId.equalsL(RTL_CONSTASCII_STRINGPARAM("NETSCAPE")) && aAppCode.equalsL(RTL_CONSTASCII_STRINGPARAM("2.0")) && cSize == 3 )
                        {
                            rIStm >> cByte;

                            // Loop-Extension
                            if ( cByte == 0x01 )
                            {
                                rIStm >> cByte;
                                nLoops = cByte;
                                rIStm >> cByte;
                                nLoops |= ( (sal_uInt16) cByte << 8 );
                                rIStm >> cByte;

                                bStatus = ( cByte == 0 );
                                bRet = NO_PENDING( rIStm );
                                bOverreadDataBlocks = sal_False;

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
                        else if ( aAppId.equalsL(RTL_CONSTASCII_STRINGPARAM("STARDIV ")) && aAppCode.equalsL(RTL_CONSTASCII_STRINGPARAM("5.0")) && cSize == 9 )
                        {
                            rIStm >> cByte;

                            // Loop-Extension
                            if ( cByte == 0x01 )
                            {
                                rIStm >> nLogWidth100 >> nLogHeight100;
                                rIStm >> cByte;
                                bStatus = ( cByte == 0 );
                                bRet = NO_PENDING( rIStm );
                                bOverreadDataBlocks = sal_False;
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
                bOverreadDataBlocks = sal_True;
            break;
        }

        // Sub-Blocks ueberlesen
        if ( bOverreadDataBlocks )
        {
            bRet = sal_True;
            while( cSize && bStatus && !rIStm.IsEof() )
            {
                sal_uInt16  nCount = (sal_uInt16) cSize + 1;
                char*   pBuffer = new char[ nCount ];

                bRet = sal_False;
                rIStm.Read( pBuffer, nCount );
                if( NO_PENDING( rIStm ) )
                {
                    cSize = (sal_uInt8) pBuffer[ cSize ];
                    bRet = sal_True;
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

sal_Bool GIFReader::ReadLocalHeader()
{
    sal_uInt8   pBuf[ 9 ];
    sal_Bool    bRet = sal_False;

    rIStm.Read( pBuf, 9 );
    if( NO_PENDING( rIStm ) )
    {
        SvMemoryStream  aMemStm;
        BitmapPalette*  pPal;
        sal_uInt8           nFlags;

        aMemStm.SetBuffer( (char*) pBuf, 9, sal_False, 9 );
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
            bRet = sal_True;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_uLong GIFReader::ReadNextBlock()
{
    sal_uLong   nRet = 0UL;
    sal_uLong   nRead;
    sal_uInt8   cBlockSize;

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
                    sal_Bool    bEOI;
                    HPBYTE  pTarget = pDecomp->DecompressBlock( pSrcBuf, cBlockSize, nRead, bEOI );

                    nRet = ( bEOI ? 3 : 1 );

                    if( nRead && !bOverreadBlock )
                        FillImages( pTarget, nRead );

                    rtl_freeMemory( pTarget );
                }
            }
        }
    }

    return nRet;
}

// ------------------------------------------------------------------------

void GIFReader::FillImages( HPBYTE pBytes, sal_uLong nCount )
{
    for( sal_uLong i = 0UL; i < nCount; i++ )
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
                        sal_uLong   nSize8 = pAcc8->GetScanlineSize();
                        HPBYTE  pScanline1 = 0;
                        sal_uLong   nSize1 = 0;

                        if( bGCTransparent )
                        {
                            pScanline1 = pAcc1->GetScanline( nYAcc );
                            nSize1 = pAcc1->GetScanlineSize();
                        }

                        for( long j = nMinY; j <= nMaxY; j++ )
                        {
                            memcpy( pAcc8->GetScanline( j ), pScanline8, nSize8 );

                            if( bGCTransparent )
                                memcpy( pAcc1->GetScanline( j ), pScanline1, nSize1 );
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

                nLastImageY = (sal_uInt16) nT1;
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
            const sal_uInt8 cTmp = pBytes[ i ];

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
            bOverreadBlock = sal_True;
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
    aAnimBmp.bUserInput = sal_False;

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

sal_Bool GIFReader::ProcessGIF()
{
    sal_Bool bRead = sal_False;
    sal_Bool bEnd = sal_False;

    if ( !bStatus )
        eActAction = ABORT_READING;

    // Stream an die richtige Stelle bringen
    rIStm.Seek( nLastPos );

    switch( eActAction )
    {
        // naechsten Marker lesen
        case( MARKER_READING ):
        {
            sal_uInt8 cByte;

            rIStm >> cByte;

            if( rIStm.IsEof() )
                eActAction = END_READING;
            else if( NO_PENDING( rIStm ) )
            {
                bRead = sal_True;

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
            if( ( bRead = ReadGlobalHeader() ) == sal_True )
            {
                ClearImageExtensions();
                eActAction = MARKER_READING;
            }
        }
        break;


        // Extension lesen
        case( EXTENSION_READING ):
        {
            if( ( bRead = ReadExtension() ) == sal_True )
                eActAction = MARKER_READING;
        }
        break;


        // Image-Descriptor lesen
        case( LOCAL_HEADER_READING ):
        {
            if( ( bRead = ReadLocalHeader() ) == sal_True )
            {
                nYAcc = nImageX = nImageY = 0;
                eActAction = FIRST_BLOCK_READING;
            }
        }
        break;


        // ersten Datenblock lesen
        case( FIRST_BLOCK_READING ):
        {
            sal_uInt8 cDataSize;

            rIStm >> cDataSize;

            if( rIStm.IsEof() )
                eActAction = ABORT_READING;
            else if( cDataSize > 12 )
                bStatus = sal_False;
            else if( NO_PENDING( rIStm ) )
            {
                bRead = sal_True;
                pDecomp = new GIFLZWDecompressor( cDataSize );
                eActAction = NEXT_BLOCK_READING;
                bOverreadBlock = sal_False;
            }
            else
                eActAction = FIRST_BLOCK_READING;
        }
        break;

        // naechsten Datenblock lesen
        case( NEXT_BLOCK_READING ):
        {
            sal_uInt16  nLastX = nImageX;
            sal_uInt16  nLastY = nImageY;
            sal_uLong   nRet = ReadNextBlock();

            // Return: 0:Pending / 1:OK; / 2:OK und letzter Block: / 3:EOI / 4:HardAbort
            if( nRet )
            {
                bRead = sal_True;

                if ( nRet == 1UL )
                {
                    bImGraphicReady = sal_True;
                    eActAction = NEXT_BLOCK_READING;
                    bOverreadBlock = sal_False;
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
                        bOverreadBlock = sal_True;
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
                nImageX = nLastX;
                nImageY = nLastY;
            }
        }
        break;

        // ein Fehler trat auf
        case( ABORT_READING ):
        {
            bEnd = sal_True;
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

    bStatus = sal_True;

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

sal_Bool ImportGIF( SvStream & rStm, Graphic& rGraphic )
{
    GIFReader*  pGIFReader = (GIFReader*) rGraphic.GetContext();
    sal_uInt16      nOldFormat = rStm.GetNumberFormatInt();
    ReadState   eReadState;
    sal_Bool        bRet = sal_True;

    rStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    if( !pGIFReader )
        pGIFReader = new GIFReader( rStm );

    rGraphic.SetContext( NULL );
    eReadState = pGIFReader->ReadGIF( rGraphic );

    if( eReadState == GIFREAD_ERROR )
    {
        bRet = sal_False;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
