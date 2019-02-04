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
#include "decode.hxx"
#include "gifread.hxx"
#include <memory>
#include <bitmapwriteaccess.hxx>

#define NO_PENDING( rStm ) ( ( rStm ).GetError() != ERRCODE_IO_PENDING )

enum GIFAction
{
    GLOBAL_HEADER_READING,
    MARKER_READING,
    EXTENSION_READING,
    LOCAL_HEADER_READING,
    FIRST_BLOCK_READING,
    NEXT_BLOCK_READING,
    ABORT_READING,
    END_READING
};

enum ReadState
{
    GIFREAD_OK,
    GIFREAD_ERROR,
    GIFREAD_NEED_MORE
};

class GIFLZWDecompressor;

class SvStream;

class GIFReader : public GraphicReader
{
    Animation           aAnimation;
    sal_uInt64          nAnimationByteSize;
    sal_uInt64          nAnimationMinFileData;
    Bitmap              aBmp8;
    Bitmap              aBmp1;
    BitmapPalette       aGPalette;
    BitmapPalette       aLPalette;
    SvStream&           rIStm;
    std::vector<sal_uInt8> aSrcBuf;
    std::unique_ptr<GIFLZWDecompressor> pDecomp;
    BitmapScopedWriteAccess pAcc8;
    BitmapScopedWriteAccess pAcc1;
    long                nYAcc;
    long                nLastPos;
    sal_uInt64          nMaxStreamData;
    sal_uInt32          nLogWidth100;
    sal_uInt32          nLogHeight100;
    sal_uInt16          nTimer;
    sal_uInt16          nGlobalWidth;           // maximum imagewidth from header
    sal_uInt16          nGlobalHeight;          // maximum imageheight from header
    sal_uInt16          nImageWidth;            // maximum screenwidth from header
    sal_uInt16          nImageHeight;           // maximum screenheight from header
    sal_uInt16          nImagePosX;
    sal_uInt16          nImagePosY;
    sal_uInt16          nImageX;                // maximum screenwidth from header
    sal_uInt16          nImageY;                // maximum screenheight from header
    sal_uInt16          nLastImageY;
    sal_uInt16          nLastInterCount;
    sal_uInt16          nLoops;
    GIFAction           eActAction;
    bool                bStatus;
    bool                bGCTransparent;         // is the image transparent, if yes:
    bool                bInterlaced;
    bool                bOverreadBlock;
    bool                bImGraphicReady;
    bool                bGlobalPalette;
    sal_uInt8           nBackgroundColor;       // backgroundcolour
    sal_uInt8           nGCTransparentIndex;    // pixels of this index are transparent
    sal_uInt8           nGCDisposalMethod;      // 'Disposal Method' (see GIF docs)
    sal_uInt8           cTransIndex1;
    sal_uInt8           cNonTransIndex1;

    void                ReadPaletteEntries( BitmapPalette* pPal, sal_uLong nCount );
    void                ClearImageExtensions();
    void                CreateBitmaps( long nWidth, long nHeight, BitmapPalette* pPal, bool bWatchForBackgroundColor );
    bool                ReadGlobalHeader();
    bool                ReadExtension();
    bool                ReadLocalHeader();
    sal_uLong           ReadNextBlock();
    void                FillImages( const sal_uInt8* pBytes, sal_uLong nCount );
    void                CreateNewBitmaps();
    bool                ProcessGIF();

public:

    ReadState           ReadGIF( Graphic& rGraphic );
    bool                ReadIsAnimated();
    Graphic             GetIntermediateGraphic();

    explicit            GIFReader( SvStream& rStm );
};

GIFReader::GIFReader( SvStream& rStm )
    : nAnimationByteSize(0)
    , nAnimationMinFileData(0)
    , aGPalette ( 256 )
    , aLPalette ( 256 )
    , rIStm ( rStm )
    , nYAcc ( 0 )
    , nLastPos ( rStm.Tell() )
    , nMaxStreamData( rStm.remainingSize() )
    , nLogWidth100 ( 0 )
    , nLogHeight100 ( 0 )
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
    aSrcBuf.resize(256);    // Memory buffer for ReadNextBlock
    ClearImageExtensions();
}

void GIFReader::ClearImageExtensions()
{
    nGCDisposalMethod = 0;
    bGCTransparent = false;
    nTimer = 0;
}

void GIFReader::CreateBitmaps(long nWidth, long nHeight, BitmapPalette* pPal,
                              bool bWatchForBackgroundColor)
{
    const Size aSize(nWidth, nHeight);

    sal_uInt64 nCombinedPixSize = nWidth * nHeight;
    if (bGCTransparent)
        nCombinedPixSize += (nCombinedPixSize/8);

    // "Overall data compression asymptotically approaches 3839 × 8 / 12 = 2559 1/3"
    // so assume compression of 1:2560 is possible
    // (http://cloudinary.com/blog/a_one_color_image_is_worth_two_thousand_words suggests
    // 1:1472.88 [184.11 x 8] is more realistic)

    sal_uInt64 nMinFileData = nWidth * nHeight / 2560;

    nMinFileData += nAnimationMinFileData;
    nCombinedPixSize += nAnimationByteSize;

    if (nMaxStreamData < nMinFileData)
    {
        //there is nowhere near enough data in this stream to fill the claimed dimensions
        SAL_WARN("vcl.filter", "in gif frame index " << aAnimation.Count() << " gif claims dimensions " << nWidth << " x " << nHeight <<
                               " but filesize of " << nMaxStreamData << " is surely insufficiently large to fill all frame images");
        bStatus = false;
        return;
    }

    // Don't bother allocating a bitmap of a size that would fail on a
    // 32-bit system. We have at least one unit tests that is expected
    // to fail (loading a 65535*65535 size GIF
    // svtools/qa/cppunit/data/gif/fail/CVE-2008-5937-1.gif), but
    // which doesn't fail on 64-bit macOS at least. Why the loading
    // fails on 64-bit Linux, no idea.
    if (nCombinedPixSize >= SAL_MAX_INT32/3*2)
    {
        bStatus = false;
        return;
    }

    if (!aSize.Width() || !aSize.Height())
    {
        bStatus = false;
        return;
    }

    if (bGCTransparent)
    {
        const Color aWhite(COL_WHITE);

        aBmp1 = Bitmap(aSize, 1);

        if (!aAnimation.Count())
            aBmp1.Erase(aWhite);

        pAcc1 = BitmapScopedWriteAccess(aBmp1);

        if (pAcc1)
        {
            cTransIndex1 = static_cast<sal_uInt8>(pAcc1->GetBestPaletteIndex(aWhite));
            cNonTransIndex1 = cTransIndex1 ? 0 : 1;
        }
        else
        {
            bStatus = false;
        }
    }

    if (bStatus)
    {
        aBmp8 = Bitmap(aSize, 8, pPal);

        if (!!aBmp8 && bWatchForBackgroundColor && aAnimation.Count())
            aBmp8.Erase((*pPal)[nBackgroundColor].GetColor());
        else
            aBmp8.Erase(COL_WHITE);

        pAcc8 = BitmapScopedWriteAccess(aBmp8);
        bStatus = bool(pAcc8);
    }
}

bool GIFReader::ReadGlobalHeader()
{
    char    pBuf[ 7 ];
    sal_uInt8   nRF;
    sal_uInt8   nAspect;
    bool    bRet = false;

    rIStm.ReadBytes( pBuf, 6 );
    if( NO_PENDING( rIStm ) )
    {
        pBuf[ 6 ] = 0;
        if( !strcmp( pBuf, "GIF87a" ) || !strcmp( pBuf, "GIF89a" ) )
        {
            rIStm.ReadBytes( pBuf, 7 );
            if( NO_PENDING( rIStm ) )
            {
                SvMemoryStream aMemStm;

                aMemStm.SetBuffer( pBuf, 7, 7 );
                aMemStm.ReadUInt16( nGlobalWidth );
                aMemStm.ReadUInt16( nGlobalHeight );
                aMemStm.ReadUChar( nRF );
                aMemStm.ReadUChar( nBackgroundColor );
                aMemStm.ReadUChar( nAspect );

                bGlobalPalette = ( nRF & 0x80 );

                if( bGlobalPalette )
                    ReadPaletteEntries( &aGPalette, sal_uLong(1) << ( ( nRF & 7 ) + 1 ) );
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
    sal_uLong nLen = 3 * nCount;
    const sal_uInt64 nMaxPossible = rIStm.remainingSize();
    if (nLen > nMaxPossible)
        nLen = nMaxPossible;
    std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8[ nLen ]);
    std::size_t nRead = rIStm.ReadBytes(pBuf.get(), nLen);
    nCount = nRead/3UL;
    if( NO_PENDING( rIStm ) )
    {
        sal_uInt8* pTmp = pBuf.get();

        for (sal_uLong i = 0; i < nCount; ++i)
        {
            BitmapColor& rColor = (*pPal)[i];

            rColor.SetRed( *pTmp++ );
            rColor.SetGreen( *pTmp++ );
            rColor.SetBlue( *pTmp++ );
        }

        // if possible accommodate some standard colours
        if( nCount < 256 )
        {
            (*pPal)[ 255UL ] = COL_WHITE;

            if( nCount < 255 )
                (*pPal)[ 254UL ] = COL_BLACK;
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
            case 0xf9 :
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
            case 0xff :
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
                                nLoops |= ( static_cast<sal_uInt16>(cByte) << 8 );
                                rIStm.ReadUChar( cByte );

                                bStatus = ( cByte == 0 );
                                bRet = NO_PENDING( rIStm );
                                bOverreadDataBlocks = false;

                                // Netscape interprets the loop count
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
            while( cSize && bStatus && !rIStm.eof() )
            {
                sal_uInt16 nCount = static_cast<sal_uInt16>(cSize) + 1;
                const sal_uInt64 nMaxPossible = rIStm.remainingSize();
                if (nCount > nMaxPossible)
                    nCount = nMaxPossible;

                if (nCount)
                    rIStm.SeekRel( nCount - 1 );    // Skip subblock data

                bRet = false;
                std::size_t nRead = rIStm.ReadBytes(&cSize, 1);
                if (NO_PENDING(rIStm) && nRead == 1)
                {
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

    std::size_t nRead = rIStm.ReadBytes(pBuf, 9);
    if (NO_PENDING(rIStm) && nRead == 9)
    {
        SvMemoryStream  aMemStm;
        BitmapPalette*  pPal;

        aMemStm.SetBuffer( pBuf, 9, 9 );
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
            ReadPaletteEntries( pPal, sal_uLong(1) << ( (nFlags & 7 ) + 1 ) );
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
    sal_uLong   nRet = 0;
    sal_uLong   nRead;
    sal_uInt8   cBlockSize;

    rIStm.ReadUChar( cBlockSize );

    if ( rIStm.eof() )
        nRet = 4;
    else if ( NO_PENDING( rIStm ) )
    {
        if ( cBlockSize == 0 )
            nRet = 2;
        else
        {
            rIStm.ReadBytes( aSrcBuf.data(), cBlockSize );

            if( NO_PENDING( rIStm ) )
            {
                if( bOverreadBlock )
                    nRet = 3;
                else
                {
                    bool       bEOI;
                    sal_uInt8* pTarget = pDecomp->DecompressBlock( aSrcBuf.data(), cBlockSize, nRead, bEOI );

                    nRet = ( bEOI ? 3 : 1 );

                    if( nRead && !bOverreadBlock )
                        FillImages( pTarget, nRead );

                    std::free( pTarget );
                }
            }
        }
    }

    return nRet;
}

void GIFReader::FillImages( const sal_uInt8* pBytes, sal_uLong nCount )
{
    for( sal_uLong i = 0; i < nCount; i++ )
    {
        if( nImageX >= nImageWidth )
        {
            if( bInterlaced )
            {
                long nT1;

                // lines will be copied if interlaced
                if( nLastInterCount )
                {
                    long nMinY = std::min( static_cast<long>(nLastImageY) + 1, static_cast<long>(nImageHeight) - 1 );
                    long nMaxY = std::min( static_cast<long>(nLastImageY) + nLastInterCount, static_cast<long>(nImageHeight) - 1 );

                    // copy last line read, if lines do not coincide
                    // ( happens at the end of the image )
                    if( ( nMinY > nLastImageY ) && ( nLastImageY < ( nImageHeight - 1 ) ) )
                    {
                        sal_uInt8*  pScanline8 = pAcc8->GetScanline( nYAcc );
                        sal_uInt32  nSize8 = pAcc8->GetScanlineSize();
                        sal_uInt8*  pScanline1 = nullptr;
                        sal_uInt32  nSize1 = 0;

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

                nLastImageY = static_cast<sal_uInt16>(nT1);
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

    pAcc8.reset();

    if( bGCTransparent )
    {
        pAcc1.reset();
        aAnimBmp.aBmpEx = BitmapEx( aBmp8, aBmp1 );
    }
    else
        aAnimBmp.aBmpEx = BitmapEx( aBmp8 );

    aAnimBmp.aPosPix = Point( nImagePosX, nImagePosY );
    aAnimBmp.aSizePix = Size( nImageWidth, nImageHeight );
    aAnimBmp.nWait = ( nTimer != 65535 ) ? nTimer : ANIMATION_TIMEOUT_ON_CLICK;
    aAnimBmp.bUserInput = false;

    // tdf#104121 . Internet Explorer, Firefox, Chrome and Safari all set a minimum default playback speed.
    // IE10 Consumer Preview sets default of 100ms for rates less that 20ms. We do the same
    if (aAnimBmp.nWait < 2) // 20ms, specified in 100's of a second
        aAnimBmp.nWait = 10;

    if( nGCDisposalMethod == 2 )
        aAnimBmp.eDisposal = Disposal::Back;
    else if( nGCDisposalMethod == 3 )
        aAnimBmp.eDisposal = Disposal::Previous;
    else
        aAnimBmp.eDisposal = Disposal::Not;

    nAnimationByteSize += aAnimBmp.aBmpEx.GetSizeBytes();
    nAnimationMinFileData += static_cast<sal_uInt64>(nImageWidth) * nImageHeight / 2560;
    aAnimation.Insert( aAnimBmp );

    if( aAnimation.Count() == 1 )
    {
        aAnimation.SetDisplaySizePixel( Size( nGlobalWidth, nGlobalHeight ) );
        aAnimation.SetLoopCount( nLoops );
    }
}

Graphic GIFReader::GetIntermediateGraphic()
{
    Graphic aImGraphic;

    // only create intermediate graphic, if data is available
    // but graphic still not completely read
    if ( bImGraphicReady && !aAnimation.Count() )
    {
        pAcc8.reset();

        if ( bGCTransparent )
        {
            pAcc1.reset();
            aImGraphic = BitmapEx( aBmp8, aBmp1 );

            pAcc1 = BitmapScopedWriteAccess(aBmp1);
            bStatus = bStatus && pAcc1;
        }
        else
            aImGraphic = aBmp8;

        pAcc8 = BitmapScopedWriteAccess(aBmp8);
        bStatus = bStatus && pAcc8;
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
        case MARKER_READING:
        {
            sal_uInt8 cByte;

            rIStm.ReadUChar( cByte );

            if( rIStm.eof() )
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
        case GLOBAL_HEADER_READING:
        {
            bRead = ReadGlobalHeader();
            if( bRead )
            {
                ClearImageExtensions();
                eActAction = MARKER_READING;
            }
        }
        break;

        // read extension
        case EXTENSION_READING:
        {
            bRead = ReadExtension();
            if( bRead )
                eActAction = MARKER_READING;
        }
        break;

        // read Image-Descriptor
        case LOCAL_HEADER_READING:
        {
            bRead = ReadLocalHeader();
            if( bRead )
            {
                nYAcc = nImageX = nImageY = 0;
                eActAction = FIRST_BLOCK_READING;
            }
        }
        break;

        // read first data block
        case FIRST_BLOCK_READING:
        {
            sal_uInt8 cDataSize;

            rIStm.ReadUChar( cDataSize );

            if( rIStm.eof() )
                eActAction = ABORT_READING;
            else if( cDataSize > 12 )
                bStatus = false;
            else if( NO_PENDING( rIStm ) )
            {
                bRead = true;
                pDecomp = std::make_unique<GIFLZWDecompressor>( cDataSize );
                eActAction = NEXT_BLOCK_READING;
                bOverreadBlock = false;
            }
            else
                eActAction = FIRST_BLOCK_READING;
        }
        break;

        // read next data block
        case NEXT_BLOCK_READING:
        {
            sal_uInt16  nLastX = nImageX;
            sal_uInt16  nLastY = nImageY;
            sal_uLong   nRet = ReadNextBlock();

            // Return: 0:Pending / 1:OK; / 2:OK and last block: / 3:EOI / 4:HardAbort
            if( nRet )
            {
                bRead = true;

                if ( nRet == 1 )
                {
                    bImGraphicReady = true;
                    eActAction = NEXT_BLOCK_READING;
                    bOverreadBlock = false;
                }
                else
                {
                    if( nRet == 2 )
                    {
                        pDecomp.reset();
                        CreateNewBitmaps();
                        eActAction = MARKER_READING;
                        ClearImageExtensions();
                    }
                    else if( nRet == 3 )
                    {
                        eActAction = NEXT_BLOCK_READING;
                        bOverreadBlock = true;
                    }
                    else
                    {
                        pDecomp.reset();
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
        case ABORT_READING:
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

bool GIFReader::ReadIsAnimated()
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

    if (eReadState == GIFREAD_OK)
        return aAnimation.Count() > 1;
    return false;
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
            rGraphic.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
        }
    }
    else
        rGraphic = aAnimation;

    return eReadState;
}

VCL_DLLPUBLIC bool IsGIFAnimated(SvStream & rStm)
{
    GIFReader aReader(rStm);

    SvStreamEndian nOldFormat = rStm.GetEndian();
    rStm.SetEndian(SvStreamEndian::LITTLE);
    bool bResult = aReader.ReadIsAnimated();
    rStm.SetEndian(nOldFormat);

    return bResult;
}

VCL_DLLPUBLIC bool ImportGIF( SvStream & rStm, Graphic& rGraphic )
{
    std::shared_ptr<GraphicReader> pContext = rGraphic.GetContext();
    rGraphic.SetContext(nullptr);
    GIFReader* pGIFReader = dynamic_cast<GIFReader*>( pContext.get() );
    if (!pGIFReader)
    {
        pContext = std::make_shared<GIFReader>( rStm );
        pGIFReader = static_cast<GIFReader*>( pContext.get() );
    }

    SvStreamEndian nOldFormat = rStm.GetEndian();
    rStm.SetEndian( SvStreamEndian::LITTLE );

    bool bRet = true;

    ReadState eReadState = pGIFReader->ReadGIF(rGraphic);

    if (eReadState == GIFREAD_ERROR)
    {
        bRet = false;
    }
    else if (eReadState == GIFREAD_NEED_MORE)
    {
        rGraphic = pGIFReader->GetIntermediateGraphic();
        rGraphic.SetContext(pContext);
    }

    rStm.SetEndian(nOldFormat);

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
