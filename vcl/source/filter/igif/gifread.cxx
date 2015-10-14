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

#define _GIFPRIVATE

#include "decode.hxx"
#include "gifread.hxx"
#include <memory>

#define NO_PENDING( rStm ) ( ( rStm ).GetError() != ERRCODE_IO_PENDING )

GIFReader::GIFReader( SvStream& rStm )
    : aGPalette ( 256 )
    , aLPalette ( 256 )
    , rIStm ( rStm )
    , pDecomp ( NULL )
    , pAcc8 ( NULL )
    , pAcc1 ( NULL )
    , nYAcc ( 0 )
    , nLastPos ( rStm.Tell() )
    , nLogWidth100 ( 0UL )
    , nLogHeight100 ( 0UL )
    , nGlobalWidth ( 0 )
    , nGlobalHeight ( 0 )
    , nImageWidth ( 0 )
    , nImageHeight ( 0 )
    , nImagePosX ( 0 )
    , nImagePosY ( 0 )
    , nImageX ( 0 )
    , nImageY ( 0 )
    , nLastImageY ( 0 )
    , nLastInterCount ( 0 )
    , nLoops ( 1 )
    , eActAction ( GLOBAL_HEADER_READING )
    , bStatus ( false )
    , bGCTransparent  ( false )
    , bInterlaced ( false)
    , bOverreadBlock ( false )
    , bImGraphicReady ( false )
    , bGlobalPalette ( false )
    , nBackgroundColor ( 0 )
    , nGCTransparentIndex ( 0 )
    , cTransIndex1 ( 0 )
    , cNonTransIndex1 ( 0 )
{
    maUpperName = "SVIGIF";
    pSrcBuf = new sal_uInt8[ 256 ];
    ClearImageExtensions();
}

GIFReader::~GIFReader()
{
    aImGraphic.SetContext( NULL );

    if( pAcc1 )
        Bitmap::ReleaseAccess( pAcc1 );

    if( pAcc8 )
        Bitmap::ReleaseAccess( pAcc8 );

    delete[] pSrcBuf;
}

void GIFReader::ClearImageExtensions()
{
    nGCDisposalMethod = 0;
    bGCTransparent = false;
    nTimer = 0;
}

bool GIFReader::CreateBitmaps( long nWidth, long nHeight, BitmapPalette* pPal,
                               bool bWatchForBackgroundColor )
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
        bStatus = false;
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
            bStatus = false;
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

bool GIFReader::ReadGlobalHeader()
{
    char    pBuf[ 7 ];
    sal_uInt8   nRF;
    sal_uInt8   nAspect;
    bool    bRet = false;

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

                aMemStm.SetBuffer( pBuf, 7, false, 7 );
                aMemStm.ReadUInt16( nGlobalWidth );
                aMemStm.ReadUInt16( nGlobalHeight );
                aMemStm.ReadUChar( nRF );
                aMemStm.ReadUChar( nBackgroundColor );
                aMemStm.ReadUChar( nAspect );

                bGlobalPalette = ( nRF & 0x80 );

                if( bGlobalPalette )
                    ReadPaletteEntries( &aGPalette, 1 << ( ( nRF & 7 ) + 1 ) );
                else
                    nBackgroundColor = 0;

                if( NO_PENDING( rIStm ) )
                    bRet = true;
            }
        }
        else
            bStatus = false;
    }

    return bRet;
}

void GIFReader::ReadPaletteEntries( BitmapPalette* pPal, sal_uLong nCount )
{
    sal_uLong nLen = 3UL * nCount;
    const sal_uInt64 nMaxPossible = rIStm.remainingSize();
    if (nLen > nMaxPossible)
        nLen = nMaxPossible;
    std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8[ nLen ]);
    sal_Size nRead = rIStm.Read(pBuf.get(), nLen);
    nCount = nRead/3UL;
    if( NO_PENDING( rIStm ) )
    {
        sal_uInt8* pTmp = pBuf.get();

        for (sal_uLong i = 0UL; i < nCount; ++i)
        {
            BitmapColor& rColor = (*pPal)[i];

            rColor.SetRed( *pTmp++ );
            rColor.SetGreen( *pTmp++ );
            rColor.SetBlue( *pTmp++ );
        }

        // if possible accommodate some standard colours
        if( nCount < 256UL )
        {
            (*pPal)[ 255UL ] = Color( COL_WHITE );

            if( nCount < 255UL )
                (*pPal)[ 254UL ] = Color( COL_BLACK );
        }
    }
}

bool GIFReader::ReadExtension()
{
    bool    bRet = false;

    // Extension-Label
    sal_uInt8 cFunction(0);
    rIStm.ReadUChar( cFunction );
    if( NO_PENDING( rIStm ) )
    {
        bool    bOverreadDataBlocks = false;
        sal_uInt8 cSize(0);
        // Block length
        rIStm.ReadUChar( cSize );

        switch( cFunction )
        {
            // 'Graphic Control Extension'
            case( 0xf9 ) :
            {
                sal_uInt8 cFlags(0);
                rIStm.ReadUChar(cFlags);
                rIStm.ReadUInt16(nTimer);
                rIStm.ReadUChar(nGCTransparentIndex);
                sal_uInt8 cByte(0);
                rIStm.ReadUChar(cByte);

                if ( NO_PENDING( rIStm ) )
                {
                    nGCDisposalMethod = ( cFlags >> 2) & 7;
                    bGCTransparent = ( cFlags & 1 );
                    bStatus = ( cSize == 4 ) && ( cByte == 0 );
                    bRet = true;
                }
            }
            break;

            // Application extension
            case ( 0xff ) :
            {
                if ( NO_PENDING( rIStm ) )
                {
                    // by default overread this extension
                    bOverreadDataBlocks = true;

                    // Appl. extension has length 11
                    if ( cSize == 0x0b )
                    {
                        OString aAppId = read_uInt8s_ToOString(rIStm, 8);
                        OString aAppCode = read_uInt8s_ToOString(rIStm, 3);
                        rIStm.ReadUChar( cSize );

                        // NetScape-Extension
                        if( aAppId == "NETSCAPE" && aAppCode == "2.0" && cSize == 3 )
                        {
                            sal_uInt8 cByte(0);
                            rIStm.ReadUChar( cByte );

                            // Loop-Extension
                            if ( cByte == 0x01 )
                            {
                                rIStm.ReadUChar( cByte );
                                nLoops = cByte;
                                rIStm.ReadUChar( cByte );
                                nLoops |= ( (sal_uInt16) cByte << 8 );
                                rIStm.ReadUChar( cByte );

                                bStatus = ( cByte == 0 );
                                bRet = NO_PENDING( rIStm );
                                bOverreadDataBlocks = false;

                                // Netscape interpretes the loop count
                                // as pure number of _repeats_;
                                // here it is the total number of loops
                                if( nLoops )
                                    nLoops++;
                            }
                            else
                                rIStm.SeekRel( -1 );
                        }
                        else if ( aAppId == "STARDIV " && aAppCode == "5.0" && cSize == 9 )
                        {
                            sal_uInt8 cByte(0);
                            rIStm.ReadUChar( cByte );

                            // Loop extension
                            if ( cByte == 0x01 )
                            {
                                rIStm.ReadUInt32( nLogWidth100 ).ReadUInt32( nLogHeight100 );
                                rIStm.ReadUChar( cByte );
                                bStatus = ( cByte == 0 );
                                bRet = NO_PENDING( rIStm );
                                bOverreadDataBlocks = false;
                            }
                            else
                                rIStm.SeekRel( -1 );
                        }

                    }
                }
            }
            break;

            // overread everything else
            default:
                bOverreadDataBlocks = true;
            break;
        }

        // overread sub-blocks
        if ( bOverreadDataBlocks )
        {
            bRet = true;
            while( cSize && bStatus && !rIStm.IsEof() )
            {
                sal_uInt16 nCount = (sal_uInt16) cSize + 1;
                const sal_uInt64 nMaxPossible = rIStm.remainingSize();
                if (nCount > nMaxPossible)
                    nCount = nMaxPossible;
                std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[nCount]);

                bRet = false;
                sal_Size nRead = rIStm.Read(pBuffer.get(), nCount);
                if (NO_PENDING(rIStm) && cSize < nRead)
                {
                    cSize = pBuffer[cSize];
                    bRet = true;
                }
                else
                    cSize = 0;
            }
        }
    }

    return bRet;
}

bool GIFReader::ReadLocalHeader()
{
    sal_uInt8   pBuf[ 9 ];
    bool    bRet = false;

    sal_Size nRead = rIStm.Read(pBuf, 9);
    if (NO_PENDING(rIStm) && nRead == 9)
    {
        SvMemoryStream  aMemStm;
        BitmapPalette*  pPal;

        aMemStm.SetBuffer( pBuf, 9, false, 9 );
        aMemStm.ReadUInt16( nImagePosX );
        aMemStm.ReadUInt16( nImagePosY );
        aMemStm.ReadUInt16( nImageWidth );
        aMemStm.ReadUInt16( nImageHeight );
        sal_uInt8 nFlags(0);
        aMemStm.ReadUChar(nFlags);

        // if interlaced, first define startvalue
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

        // if we could read everything, we will create the local image;
        // if the global colour table is valid for the image, we will
        // consider the BackGroudColorIndex.
        if( NO_PENDING( rIStm ) )
        {
            CreateBitmaps( nImageWidth, nImageHeight, pPal, bGlobalPalette && ( pPal == &aGPalette ) );
            bRet = true;
        }
    }

    return bRet;
}

sal_uLong GIFReader::ReadNextBlock()
{
    sal_uLong   nRet = 0UL;
    sal_uLong   nRead;
    sal_uInt8   cBlockSize;

    rIStm.ReadUChar( cBlockSize );

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
                    bool       bEOI;
                    sal_uInt8* pTarget = pDecomp->DecompressBlock( pSrcBuf, cBlockSize, nRead, bEOI );

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

void GIFReader::FillImages( sal_uInt8* pBytes, sal_uLong nCount )
{
    for( sal_uLong i = 0UL; i < nCount; i++ )
    {
        if( nImageX >= nImageWidth )
        {
            if( bInterlaced )
            {
                long nT1;

                // lines will be copied if interlaced
                if( nLastInterCount )
                {
                    long nMinY = std::min( (long) nLastImageY + 1, (long) nImageHeight - 1 );
                    long nMaxY = std::min( (long) nLastImageY + nLastInterCount, (long) nImageHeight - 1 );

                    // copy last line read, if lines do not coincide
                    // ( happens at the end of the image )
                    if( ( nMinY > nLastImageY ) && ( nLastImageY < ( nImageHeight - 1 ) ) )
                    {
                        sal_uInt8*  pScanline8 = pAcc8->GetScanline( nYAcc );
                        sal_uLong   nSize8 = pAcc8->GetScanlineSize();
                        sal_uInt8*  pScanline1 = 0;
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
                    long nT2 = nImageY - ( ( nImageHeight + 7 ) >> 3 );
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

            // line starts from the beginning
            nImageX = 0;
        }

        if( nImageY < nImageHeight )
        {
            const sal_uInt8 cTmp = pBytes[ i ];

            if( bGCTransparent )
            {
                if( cTmp == nGCTransparentIndex )
                    pAcc1->SetPixelIndex( nYAcc, nImageX++, cTransIndex1 );
                else
                {
                    pAcc8->SetPixelIndex( nYAcc, nImageX, cTmp );
                    pAcc1->SetPixelIndex( nYAcc, nImageX++, cNonTransIndex1 );
                }
            }
            else
                pAcc8->SetPixelIndex( nYAcc, nImageX++, cTmp );
        }
        else
        {
            bOverreadBlock = true;
            break;
        }
    }
}

void GIFReader::CreateNewBitmaps()
{
    AnimationBitmap aAnimBmp;

    Bitmap::ReleaseAccess( pAcc8 );
    pAcc8 = NULL;

    if( bGCTransparent )
    {
        Bitmap::ReleaseAccess( pAcc1 );
        pAcc1 = NULL;
        aAnimBmp.aBmpEx = BitmapEx( aBmp8, aBmp1 );
    }
    else
        aAnimBmp.aBmpEx = BitmapEx( aBmp8 );

    aAnimBmp.aPosPix = Point( nImagePosX, nImagePosY );
    aAnimBmp.aSizePix = Size( nImageWidth, nImageHeight );
    aAnimBmp.nWait = ( nTimer != 65535 ) ? nTimer : ANIMATION_TIMEOUT_ON_CLICK;
    aAnimBmp.bUserInput = false;

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

const Graphic& GIFReader::GetIntermediateGraphic()
{
    // only create intermediate graphic, if data is available
    // but graphic still not completely read
    if ( bImGraphicReady && !aAnimation.Count() )
    {
        Bitmap  aBmp;

        Bitmap::ReleaseAccess( pAcc8 );

        if ( bGCTransparent )
        {
            Bitmap::ReleaseAccess( pAcc1 );
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

bool GIFReader::ProcessGIF()
{
    bool bRead = false;
    bool bEnd = false;

    if ( !bStatus )
        eActAction = ABORT_READING;

    // set stream to right position
    rIStm.Seek( nLastPos );

    switch( eActAction )
    {
        // read next marker
        case( MARKER_READING ):
        {
            sal_uInt8 cByte;

            rIStm.ReadUChar( cByte );

            if( rIStm.IsEof() )
                eActAction = END_READING;
            else if( NO_PENDING( rIStm ) )
            {
                bRead = true;

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

        // read ScreenDescriptor
        case( GLOBAL_HEADER_READING ):
        {
            if( ( bRead = ReadGlobalHeader() ) )
            {
                ClearImageExtensions();
                eActAction = MARKER_READING;
            }
        }
        break;

        // read extension
        case( EXTENSION_READING ):
        {
            if( ( bRead = ReadExtension() ) )
                eActAction = MARKER_READING;
        }
        break;

        // read Image-Descriptor
        case( LOCAL_HEADER_READING ):
        {
            if( ( bRead = ReadLocalHeader() ) )
            {
                nYAcc = nImageX = nImageY = 0;
                eActAction = FIRST_BLOCK_READING;
            }
        }
        break;

        // read first data block
        case( FIRST_BLOCK_READING ):
        {
            sal_uInt8 cDataSize;

            rIStm.ReadUChar( cDataSize );

            if( rIStm.IsEof() )
                eActAction = ABORT_READING;
            else if( cDataSize > 12 )
                bStatus = false;
            else if( NO_PENDING( rIStm ) )
            {
                bRead = true;
                pDecomp = new GIFLZWDecompressor( cDataSize );
                eActAction = NEXT_BLOCK_READING;
                bOverreadBlock = false;
            }
            else
                eActAction = FIRST_BLOCK_READING;
        }
        break;

        // read next data block
        case( NEXT_BLOCK_READING ):
        {
            sal_uInt16  nLastX = nImageX;
            sal_uInt16  nLastY = nImageY;
            sal_uLong   nRet = ReadNextBlock();

            // Return: 0:Pending / 1:OK; / 2:OK and last block: / 3:EOI / 4:HardAbort
            if( nRet )
            {
                bRead = true;

                if ( nRet == 1UL )
                {
                    bImGraphicReady = true;
                    eActAction = NEXT_BLOCK_READING;
                    bOverreadBlock = false;
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
                        bOverreadBlock = true;
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

        // an error occurred
        case( ABORT_READING ):
        {
            bEnd = true;
            eActAction = END_READING;
        }
        break;

        default:
        break;
    }

    // set stream to right position,
    // if data could be read put it a the old
    // position otherwise at the actual one
    if( bRead || bEnd )
        nLastPos = rIStm.Tell();

    return bRead;
}

ReadState GIFReader::ReadGIF( Graphic& rGraphic )
{
    ReadState eReadState;

    bStatus = true;

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

VCL_DLLPUBLIC bool ImportGIF( SvStream & rStm, Graphic& rGraphic )
{
    GIFReader*  pGIFReader = static_cast<GIFReader*>(rGraphic.GetContext());
    SvStreamEndian nOldFormat = rStm.GetEndian();
    ReadState   eReadState;
    bool        bRet = true;

    rStm.SetEndian( SvStreamEndian::LITTLE );

    if( !pGIFReader )
        pGIFReader = new GIFReader( rStm );

    rGraphic.SetContext( NULL );
    eReadState = pGIFReader->ReadGIF( rGraphic );

    if( eReadState == GIFREAD_ERROR )
    {
        bRet = false;
        delete pGIFReader;
    }
    else if( eReadState == GIFREAD_OK )
        delete pGIFReader;
    else
    {
        rGraphic = pGIFReader->GetIntermediateGraphic();
        rGraphic.SetContext( pGIFReader );
    }

    rStm.SetEndian( nOldFormat );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
