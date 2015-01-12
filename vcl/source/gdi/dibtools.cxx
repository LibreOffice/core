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

#include <sal/config.h>

#include <cassert>

#include <vcl/salbtype.hxx>
#include <vcl/dibtools.hxx>
#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include <tools/fract.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/outdev.hxx>
#include <boost/scoped_array.hpp>

// - Defines -

#define DIBCOREHEADERSIZE       ( 12UL )
#define DIBINFOHEADERSIZE       ( sizeof(DIBInfoHeader) )
#define DIBV5HEADERSIZE         ( sizeof(DIBV5Header) )

// - Compression defines

#define COMPRESS_OWN                ('S'|('D'<<8UL))
#define COMPRESS_NONE               ( 0UL )
#define RLE_8                       ( 1UL )
#define RLE_4                       ( 2UL )
#define BITFIELDS                   ( 3UL )
#define ZCOMPRESS                   ( COMPRESS_OWN | 0x01000000UL ) /* == 'SD01' (binary) */

// - DIBInfoHeader and DIBV5Header

typedef sal_Int32 FXPT2DOT30;

struct CIEXYZ
{
    FXPT2DOT30      aXyzX;
    FXPT2DOT30      aXyzY;
    FXPT2DOT30      aXyzZ;

    CIEXYZ()
    :   aXyzX(0L),
        aXyzY(0L),
        aXyzZ(0L)
    {}

    ~CIEXYZ()
    {}
};

struct CIEXYZTriple
{
    CIEXYZ          aXyzRed;
    CIEXYZ          aXyzGreen;
    CIEXYZ          aXyzBlue;

    CIEXYZTriple()
    :   aXyzRed(),
        aXyzGreen(),
        aXyzBlue()
    {}

    ~CIEXYZTriple()
    {}
};

struct DIBInfoHeader
{
    sal_uInt32      nSize;
    sal_Int32       nWidth;
    sal_Int32       nHeight;
    sal_uInt16      nPlanes;
    sal_uInt16      nBitCount;
    sal_uInt32      nCompression;
    sal_uInt32      nSizeImage;
    sal_Int32       nXPelsPerMeter;
    sal_Int32       nYPelsPerMeter;
    sal_uInt32      nColsUsed;
    sal_uInt32      nColsImportant;

    DIBInfoHeader()
    :   nSize(0UL),
        nWidth(0UL),
        nHeight(0UL),
        nPlanes(0),
        nBitCount(0),
        nCompression(0),
        nSizeImage(0),
        nXPelsPerMeter(0UL),
        nYPelsPerMeter(0UL),
        nColsUsed(0UL),
        nColsImportant(0UL)
    {}

    ~DIBInfoHeader()
    {}
};

struct DIBV5Header : public DIBInfoHeader
{
    sal_uInt32      nV5RedMask;
    sal_uInt32      nV5GreenMask;
    sal_uInt32      nV5BlueMask;
    sal_uInt32      nV5AlphaMask;
    sal_uInt32      nV5CSType;
    CIEXYZTriple    aV5Endpoints;
    sal_uInt32      nV5GammaRed;
    sal_uInt32      nV5GammaGreen;
    sal_uInt32      nV5GammaBlue;
    sal_uInt32      nV5Intent;
    sal_uInt32      nV5ProfileData;
    sal_uInt32      nV5ProfileSize;
    sal_uInt32      nV5Reserved;

    DIBV5Header()
    :   DIBInfoHeader(),
        nV5RedMask(0UL),
        nV5GreenMask(0UL),
        nV5BlueMask(0UL),
        nV5AlphaMask(0UL),
        nV5CSType(0UL),
        aV5Endpoints(),
        nV5GammaRed(0UL),
        nV5GammaGreen(0UL),
        nV5GammaBlue(0UL),
        nV5Intent(0UL),
        nV5ProfileData(0UL),
        nV5ProfileSize(0UL),
        nV5Reserved(0UL)
    {}

    ~DIBV5Header()
    {}
};

namespace
{
    inline sal_uInt16 discretizeBitcount( sal_uInt16 nInputCount )
    {
        return ( nInputCount <= 1 ) ? 1 :
               ( nInputCount <= 4 ) ? 4 :
               ( nInputCount <= 8 ) ? 8 : 24;
    }

    inline bool isBitfieldCompression( sal_uLong nScanlineFormat )
    {
        return (BMP_FORMAT_16BIT_TC_LSB_MASK == nScanlineFormat) || (BMP_FORMAT_32BIT_TC_MASK == nScanlineFormat);
    }
}

bool ImplReadDIBInfoHeader(SvStream& rIStm, DIBV5Header& rHeader, bool& bTopDown, bool bMSOFormat)
{
    // BITMAPINFOHEADER or BITMAPCOREHEADER or BITMAPV5HEADER
    const sal_Size aStartPos(rIStm.Tell());
    rIStm.ReadUInt32( rHeader.nSize );

    // BITMAPCOREHEADER
    if ( rHeader.nSize == DIBCOREHEADERSIZE )
    {
        sal_Int16 nTmp16;

        rIStm.ReadInt16( nTmp16 ); rHeader.nWidth = nTmp16;
        rIStm.ReadInt16( nTmp16 ); rHeader.nHeight = nTmp16;
        rIStm.ReadUInt16( rHeader.nPlanes );
        rIStm.ReadUInt16( rHeader.nBitCount );
    }
    else if ( bMSOFormat && rHeader.nSize == DIBINFOHEADERSIZE )
    {
        sal_Int16 nTmp16(0);
        rIStm.ReadInt16(nTmp16);
        rHeader.nWidth = nTmp16;
        rIStm.ReadInt16(nTmp16);
        rHeader.nHeight = nTmp16;
        sal_uInt8 nTmp8(0);
        rIStm.ReadUChar(nTmp8);
        rHeader.nPlanes = nTmp8;
        rIStm.ReadUChar(nTmp8);
        rHeader.nBitCount = nTmp8;
        rIStm.ReadInt16(nTmp16);
        rHeader.nSizeImage = nTmp16;
        rIStm.ReadInt16(nTmp16);
        rHeader.nCompression = nTmp16;
        if ( !rHeader.nSizeImage ) // uncompressed?
            rHeader.nSizeImage = ((rHeader.nWidth * rHeader.nBitCount + 31) & ~31) / 8 * rHeader.nHeight;
        rIStm.ReadInt32( rHeader.nXPelsPerMeter );
        rIStm.ReadInt32( rHeader.nYPelsPerMeter );
        rIStm.ReadUInt32( rHeader.nColsUsed );
        rIStm.ReadUInt32( rHeader.nColsImportant );
    }
    else
    {
        // BITMAPCOREHEADER, BITMAPV5HEADER or unknown. Read as far as possible
        sal_Size nUsed(sizeof(rHeader.nSize));

        // read DIBInfoHeader entries
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.nWidth ); nUsed += sizeof(rHeader.nWidth); }
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.nHeight ); nUsed += sizeof(rHeader.nHeight); }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt16( rHeader.nPlanes ); nUsed += sizeof(rHeader.nPlanes); }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt16( rHeader.nBitCount );  nUsed += sizeof(rHeader.nBitCount); }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nCompression );  nUsed += sizeof(rHeader.nCompression); }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nSizeImage );  nUsed += sizeof(rHeader.nSizeImage); }
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.nXPelsPerMeter );  nUsed += sizeof(rHeader.nXPelsPerMeter); }
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.nYPelsPerMeter );  nUsed += sizeof(rHeader.nYPelsPerMeter); }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nColsUsed );  nUsed += sizeof(rHeader.nColsUsed); }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nColsImportant );  nUsed += sizeof(rHeader.nColsImportant); }

        // read DIBV5HEADER members
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5RedMask ); nUsed += sizeof(rHeader.nV5RedMask); }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5GreenMask ); nUsed += sizeof(rHeader.nV5GreenMask);  }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5BlueMask ); nUsed += sizeof(rHeader.nV5BlueMask);  }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5AlphaMask ); nUsed += sizeof(rHeader.nV5AlphaMask);  }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5CSType ); nUsed += sizeof(rHeader.nV5CSType);  }

        // read contained CIEXYZTriple's
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.aV5Endpoints.aXyzRed.aXyzX ); nUsed += sizeof(rHeader.aV5Endpoints.aXyzRed.aXyzX); }
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.aV5Endpoints.aXyzRed.aXyzY ); nUsed += sizeof(rHeader.aV5Endpoints.aXyzRed.aXyzY); }
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.aV5Endpoints.aXyzRed.aXyzZ ); nUsed += sizeof(rHeader.aV5Endpoints.aXyzRed.aXyzZ); }
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.aV5Endpoints.aXyzGreen.aXyzX ); nUsed += sizeof(rHeader.aV5Endpoints.aXyzGreen.aXyzX); }
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.aV5Endpoints.aXyzGreen.aXyzY ); nUsed += sizeof(rHeader.aV5Endpoints.aXyzGreen.aXyzY); }
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.aV5Endpoints.aXyzGreen.aXyzZ ); nUsed += sizeof(rHeader.aV5Endpoints.aXyzGreen.aXyzZ); }
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.aV5Endpoints.aXyzBlue.aXyzX ); nUsed += sizeof(rHeader.aV5Endpoints.aXyzBlue.aXyzX); }
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.aV5Endpoints.aXyzBlue.aXyzY ); nUsed += sizeof(rHeader.aV5Endpoints.aXyzBlue.aXyzY); }
        if(nUsed < rHeader.nSize) { rIStm.ReadInt32( rHeader.aV5Endpoints.aXyzBlue.aXyzZ ); nUsed += sizeof(rHeader.aV5Endpoints.aXyzBlue.aXyzZ); }

        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5GammaRed ); nUsed += sizeof(rHeader.nV5GammaRed);  }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5GammaGreen ); nUsed += sizeof(rHeader.nV5GammaGreen);  }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5GammaBlue ); nUsed += sizeof(rHeader.nV5GammaBlue);  }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5Intent ); nUsed += sizeof(rHeader.nV5Intent);  }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5ProfileData ); nUsed += sizeof(rHeader.nV5ProfileData);  }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5ProfileSize ); nUsed += sizeof(rHeader.nV5ProfileSize);  }
        if(nUsed < rHeader.nSize) { rIStm.ReadUInt32( rHeader.nV5Reserved ); nUsed += sizeof(rHeader.nV5Reserved);  }

        // seek to EndPos
        rIStm.Seek(aStartPos + rHeader.nSize);
    }

    if ( rHeader.nHeight < 0 )
    {
        bTopDown = true;
        rHeader.nHeight *= -1;
    }
    else
    {
        bTopDown = false;
    }

    if ( rHeader.nWidth < 0 )
    {
        rIStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
    }

    // #144105# protect a little against damaged files
    assert(rHeader.nHeight >= 0);
    if (rHeader.nHeight != 0 && rHeader.nWidth >= 0
        && (rHeader.nSizeImage / 16 / static_cast<sal_uInt32>(rHeader.nHeight)
            > static_cast<sal_uInt32>(rHeader.nWidth)))
    {
        rHeader.nSizeImage = 0;
    }

    return( ( rHeader.nPlanes == 1 ) && ( rIStm.GetError() == 0UL ) );
}

bool ImplReadDIBPalette( SvStream& rIStm, BitmapWriteAccess& rAcc, bool bQuad )
{
    const sal_uInt16    nColors = rAcc.GetPaletteEntryCount();
    const sal_uLong     nPalSize = nColors * ( bQuad ? 4UL : 3UL );
    BitmapColor     aPalColor;

    boost::scoped_array<sal_uInt8> pEntries(new sal_uInt8[ nPalSize ]);
    if (rIStm.Read( pEntries.get(), nPalSize ) != nPalSize)
    {
        return false;
    }

    sal_uInt8* pTmpEntry = pEntries.get();
    for( sal_uInt16 i = 0; i < nColors; i++ )
    {
        aPalColor.SetBlue( *pTmpEntry++ );
        aPalColor.SetGreen( *pTmpEntry++ );
        aPalColor.SetRed( *pTmpEntry++ );

        if( bQuad )
            pTmpEntry++;

        rAcc.SetPaletteColor( i, aPalColor );
    }

    return( rIStm.GetError() == 0UL );
}

bool ImplDecodeRLE( sal_uInt8* pBuffer, DIBV5Header& rHeader, BitmapWriteAccess& rAcc, bool bRLE4 )
{
    Scanline pRLE = pBuffer;
    Scanline pEndRLE = pBuffer + rHeader.nSizeImage;
    long        nY = rHeader.nHeight - 1L;
    const sal_uLong nWidth = rAcc.Width();
    sal_uLong       nCountByte;
    sal_uLong       nRunByte;
    sal_uLong       nX = 0UL;
    sal_uInt8       cTmp;
    bool        bEndDecoding = false;

    do
    {
        if (pRLE == pEndRLE)
            return false;
        if( ( nCountByte = *pRLE++ ) == 0 )
        {
            if (pRLE == pEndRLE)
                return false;
            nRunByte = *pRLE++;

            if( nRunByte > 2 )
            {
                if( bRLE4 )
                {
                    nCountByte = nRunByte >> 1;

                    for( sal_uLong i = 0UL; i < nCountByte; i++ )
                    {
                        if (pRLE == pEndRLE)
                            return false;

                        cTmp = *pRLE++;

                        if( nX < nWidth )
                            rAcc.SetPixelIndex( nY, nX++, cTmp >> 4 );

                        if( nX < nWidth )
                            rAcc.SetPixelIndex( nY, nX++, cTmp & 0x0f );
                    }

                    if( nRunByte & 1 )
                    {
                        if (pRLE == pEndRLE)
                            return false;

                        if( nX < nWidth )
                            rAcc.SetPixelIndex( nY, nX++, *pRLE >> 4 );

                        pRLE++;
                    }

                    if( ( ( nRunByte + 1 ) >> 1 ) & 1 )
                    {
                        if (pRLE == pEndRLE)
                            return false;

                        pRLE++;
                    }
                }
                else
                {
                    for( sal_uLong i = 0UL; i < nRunByte; i++ )
                    {
                        if (pRLE == pEndRLE)
                            return false;

                        if( nX < nWidth )
                            rAcc.SetPixelIndex( nY, nX++, *pRLE );

                        pRLE++;
                    }

                    if( nRunByte & 1 )
                    {
                        if (pRLE == pEndRLE)
                            return false;

                        pRLE++;
                    }
                }
            }
            else if( !nRunByte )
            {
                nY--;
                nX = 0UL;
            }
            else if( nRunByte == 1 )
                bEndDecoding = true;
            else
            {
                if (pRLE == pEndRLE)
                    return false;

                nX += *pRLE++;

                if (pRLE == pEndRLE)
                    return false;

                nY -= *pRLE++;
            }
        }
        else
        {
            if (pRLE == pEndRLE)
                return false;
            cTmp = *pRLE++;

            if( bRLE4 )
            {
                nRunByte = nCountByte >> 1;

                for( sal_uLong i = 0UL; i < nRunByte; i++ )
                {
                    if( nX < nWidth )
                        rAcc.SetPixelIndex( nY, nX++, cTmp >> 4 );

                    if( nX < nWidth )
                        rAcc.SetPixelIndex( nY, nX++, cTmp & 0x0f );
                }

                if( ( nCountByte & 1 ) && ( nX < nWidth ) )
                    rAcc.SetPixelIndex( nY, nX++, cTmp >> 4 );
            }
            else
            {
                for( sal_uLong i = 0UL; ( i < nCountByte ) && ( nX < nWidth ); i++ )
                    rAcc.SetPixelIndex( nY, nX++, cTmp );
            }
        }
    }
    while (!bEndDecoding && (nY >= 0L));

    return true;
}

bool ImplReadDIBBits(SvStream& rIStm, DIBV5Header& rHeader, BitmapWriteAccess& rAcc, BitmapWriteAccess* pAccAlpha, bool bTopDown, bool& rAlphaUsed)
{
    const sal_Int64 nBitsPerLine (static_cast<sal_Int64>(rHeader.nWidth) * static_cast<sal_Int64>(rHeader.nBitCount));
    if (nBitsPerLine > SAL_MAX_UINT32)
        return false;

    const sal_uLong nAlignedWidth = AlignedWidth4Bytes(static_cast<sal_uLong>(nBitsPerLine));
    sal_uInt32 nRMask(( rHeader.nBitCount == 16 ) ? 0x00007c00UL : 0x00ff0000UL);
    sal_uInt32 nGMask(( rHeader.nBitCount == 16 ) ? 0x000003e0UL : 0x0000ff00UL);
    sal_uInt32 nBMask(( rHeader.nBitCount == 16 ) ? 0x0000001fUL : 0x000000ffUL);
    bool bNative(false);
    bool bTCMask(!pAccAlpha && ((16 == rHeader.nBitCount) || (32 == rHeader.nBitCount)));
    bool bRLE((RLE_8 == rHeader.nCompression && 8 == rHeader.nBitCount) || (RLE_4 == rHeader.nCompression && 4 == rHeader.nBitCount));

    // Is native format?
    switch(rAcc.GetScanlineFormat())
    {
        case( BMP_FORMAT_1BIT_MSB_PAL ):
        case( BMP_FORMAT_4BIT_MSN_PAL ):
        case( BMP_FORMAT_8BIT_PAL ):
        case( BMP_FORMAT_24BIT_TC_BGR ):
        {
            bNative = ( ( static_cast< bool >(rAcc.IsBottomUp()) != bTopDown ) && !bRLE && !bTCMask && ( rAcc.GetScanlineSize() == nAlignedWidth ) );
            break;
        }

        default:
        {
            break;
        }
    }

    // Read data
    if(bNative)
    {
        if (nAlignedWidth
            > std::numeric_limits<sal_Size>::max() / rHeader.nHeight)
        {
            return false;
        }
        sal_Size n = nAlignedWidth * rHeader.nHeight;
        if (rIStm.Read(rAcc.GetBuffer(), n) != n)
        {
            return false;
        }
    }
    else
    {
        // Read color mask
        if(bTCMask && BITFIELDS == rHeader.nCompression)
        {
            rIStm.SeekRel( -12L );
            rIStm.ReadUInt32( nRMask );
            rIStm.ReadUInt32( nGMask );
            rIStm.ReadUInt32( nBMask );
        }

        if(bRLE)
        {
            if(!rHeader.nSizeImage)
            {
                rHeader.nSizeImage = rIStm.remainingSize();
            }

            boost::scoped_array<sal_uInt8> pBuffer(
                new sal_uInt8[rHeader.nSizeImage]);
            if (rIStm.Read(pBuffer.get(), rHeader.nSizeImage)
                != rHeader.nSizeImage)
            {
                return false;
            }
            ImplDecodeRLE(pBuffer.get(), rHeader, rAcc, RLE_4 == rHeader.nCompression);
        }
        else
        {
            const long nWidth(rHeader.nWidth);
            const long nHeight(rHeader.nHeight);
            boost::scoped_array<sal_uInt8> pBuf(new sal_uInt8[nAlignedWidth]);

            const long nI(bTopDown ? 1 : -1);
            long nY(bTopDown ? 0 : nHeight - 1);
            long nCount(nHeight);

            switch(rHeader.nBitCount)
            {
                case( 1 ):
                {
                    sal_uInt8*  pTmp;
                    sal_uInt8   cTmp;

                    for( ; nCount--; nY += nI )
                    {
                        if (rIStm.Read( pTmp = pBuf.get(), nAlignedWidth )
                            != nAlignedWidth)
                        {
                            return false;
                        }
                        cTmp = *pTmp++;

                        for( long nX = 0L, nShift = 8L; nX < nWidth; nX++ )
                        {
                            if( !nShift )
                            {
                                nShift = 8L,
                                cTmp = *pTmp++;
                            }

                            rAcc.SetPixelIndex( nY, nX, (cTmp >> --nShift) & 1);
                        }
                    }
                }
                break;

                case( 4 ):
                {
                    sal_uInt8*  pTmp;
                    sal_uInt8   cTmp;

                    for( ; nCount--; nY += nI )
                    {
                        if (rIStm.Read( pTmp = pBuf.get(), nAlignedWidth )
                            != nAlignedWidth)
                        {
                            return false;
                        }
                        cTmp = *pTmp++;

                        for( long nX = 0L, nShift = 2L; nX < nWidth; nX++ )
                        {
                            if( !nShift )
                            {
                                nShift = 2UL,
                                cTmp = *pTmp++;
                            }

                            rAcc.SetPixelIndex( nY, nX, (cTmp >> ( --nShift << 2UL ) ) & 0x0f);
                        }
                    }
                }
                break;

                case( 8 ):
                {
                    sal_uInt8*  pTmp;

                    for( ; nCount--; nY += nI )
                    {
                        if (rIStm.Read( pTmp = pBuf.get(), nAlignedWidth )
                            != nAlignedWidth)
                        {
                            return false;
                        }

                        for( long nX = 0L; nX < nWidth; nX++ )
                            rAcc.SetPixelIndex( nY, nX, *pTmp++ );
                    }
                }
                break;

                case( 16 ):
                {
                    ColorMask   aMask( nRMask, nGMask, nBMask );
                    BitmapColor aColor;
                    sal_uInt16*     pTmp16;

                    for( ; nCount--; nY += nI )
                    {
                        if (rIStm.Read( (char*)( pTmp16 = (sal_uInt16*) pBuf.get() ), nAlignedWidth )
                            != nAlignedWidth)
                        {
                            return false;
                        }

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            aMask.GetColorFor16BitLSB( aColor, (sal_uInt8*) pTmp16++ );
                            rAcc.SetPixel( nY, nX, aColor );
                        }
                    }
                }
                break;

                case( 24 ):
                {
                    BitmapColor aPixelColor;
                    sal_uInt8*      pTmp;

                    for( ; nCount--; nY += nI )
                    {
                        if (rIStm.Read( pTmp = pBuf.get(), nAlignedWidth )
                            != nAlignedWidth)
                        {
                            return false;
                        }

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            aPixelColor.SetBlue( *pTmp++ );
                            aPixelColor.SetGreen( *pTmp++ );
                            aPixelColor.SetRed( *pTmp++ );
                            rAcc.SetPixel( nY, nX, aPixelColor );
                        }
                    }
                }
                break;

                case( 32 ):
                {
                    ColorMask aMask(nRMask, nGMask, nBMask);
                    BitmapColor aColor;
                    sal_uInt32* pTmp32;

                    if(pAccAlpha)
                    {
                        sal_uInt8 aAlpha;

                        for( ; nCount--; nY += nI )
                        {
                            if (rIStm.Read( (char*)( pTmp32 = (sal_uInt32*) pBuf.get() ), nAlignedWidth )
                                != nAlignedWidth)
                            {
                                return false;
                            }

                            for( long nX = 0L; nX < nWidth; nX++ )
                            {
                                aMask.GetColorAndAlphaFor32Bit( aColor, aAlpha, (sal_uInt8*) pTmp32++ );
                                rAcc.SetPixel( nY, nX, aColor );
                                pAccAlpha->SetPixelIndex(nY, nX, sal_uInt8(0xff) - aAlpha);
                                rAlphaUsed |= bool(0xff != aAlpha);
                            }
                        }
                    }
                    else
                    {
                        for( ; nCount--; nY += nI )
                        {
                            if (rIStm.Read( (char*)( pTmp32 = (sal_uInt32*) pBuf.get() ), nAlignedWidth )
                                != nAlignedWidth)
                            {
                                return false;
                            }

                            for( long nX = 0L; nX < nWidth; nX++ )
                            {
                                aMask.GetColorFor32Bit( aColor, (sal_uInt8*) pTmp32++ );
                                rAcc.SetPixel( nY, nX, aColor );
                            }
                        }
                    }
                }
            }
        }
    }

    return( rIStm.GetError() == 0UL );
}

bool ImplReadDIBBody( SvStream& rIStm, Bitmap& rBmp, Bitmap* pBmpAlpha, sal_uLong nOffset, bool bMSOFormat = false )
{
    DIBV5Header aHeader;
    const sal_uLong nStmPos = rIStm.Tell();
    bool bRet(false);
    bool bTopDown(false);

    if(ImplReadDIBInfoHeader(rIStm, aHeader, bTopDown, bMSOFormat) && aHeader.nWidth && aHeader.nHeight && aHeader.nBitCount)
    {
        // In case ImplReadDIB() didn't call ImplReadDIBFileHeader() before
        // this method, nOffset is 0, that's OK.
        if (nOffset && aHeader.nSize > nOffset)
        {
            // Header size claims to extend into the image data.
            // Looks like an error.
            return false;
        }

        const sal_uInt16 nBitCount(discretizeBitcount(aHeader.nBitCount));
        const Size aSizePixel(aHeader.nWidth, aHeader.nHeight);
        BitmapPalette aDummyPal;
        Bitmap aNewBmp(aSizePixel, nBitCount, &aDummyPal);
        Bitmap aNewBmpAlpha;
        BitmapWriteAccess* pAcc = aNewBmp.AcquireWriteAccess();
        BitmapWriteAccess* pAccAlpha = 0;
        bool bAlphaPossible(pBmpAlpha && aHeader.nBitCount == 32);

        if(bAlphaPossible)
        {
            const bool bRedSet(0 != aHeader.nV5RedMask);
            const bool bGreenSet(0 != aHeader.nV5GreenMask);
            const bool bBlueSet(0 != aHeader.nV5BlueMask);

            // some clipboard entries have alpha mask on zero to say that there is
            // no alpha; do only use this when the other masks are set. The MS docu
            // says that masks are only to be set when bV5Compression is set to
            // BI_BITFIELDS, but there seem to exist a wild variety of usages...
            if((bRedSet || bGreenSet || bBlueSet) && (0 == aHeader.nV5AlphaMask))
            {
                bAlphaPossible = false;
            }
        }

        if(bAlphaPossible)
        {
            aNewBmpAlpha = Bitmap(aSizePixel, 8);
            pAccAlpha = aNewBmpAlpha.AcquireWriteAccess();
        }

        if(pAcc)
        {
            sal_uInt16 nColors(0);
            SvStream* pIStm;
            SvMemoryStream* pMemStm = NULL;
            sal_uInt8* pData = NULL;

            if(nBitCount <= 8)
            {
                if(aHeader.nColsUsed)
                {
                    nColors = (sal_uInt16)aHeader.nColsUsed;
                }
                else
                {
                    nColors = ( 1 << aHeader.nBitCount );
                }
            }

            if(ZCOMPRESS == aHeader.nCompression)
            {
                ZCodec aCodec;
                sal_uInt32 nCodedSize(0);
                sal_uInt32  nUncodedSize(0);
                sal_uLong nCodedPos(0);

                // read coding information
                rIStm.ReadUInt32( nCodedSize ).ReadUInt32( nUncodedSize ).ReadUInt32( aHeader.nCompression );
                pData = (sal_uInt8*) rtl_allocateMemory( nUncodedSize );

                // decode buffer
                nCodedPos = rIStm.Tell();
                aCodec.BeginCompression();
                aCodec.Read( rIStm, pData, nUncodedSize );
                aCodec.EndCompression();

                // skip unread bytes from coded buffer
                rIStm.SeekRel( nCodedSize - ( rIStm.Tell() - nCodedPos ) );

                // set decoded bytes to memory stream,
                // from which we will read the bitmap data
                pIStm = pMemStm = new SvMemoryStream;
                pMemStm->SetBuffer( (char*) pData, nUncodedSize, false, nUncodedSize );
                nOffset = 0;
            }
            else
            {
                pIStm = &rIStm;
            }

            // read palette
            if(nColors)
            {
                pAcc->SetPaletteEntryCount(nColors);
                ImplReadDIBPalette(*pIStm, *pAcc, aHeader.nSize != DIBCOREHEADERSIZE);
            }

            // read bits
            bool bAlphaUsed(false);

            if(!pIStm->GetError())
            {
                if(nOffset)
                {
                    pIStm->SeekRel(nOffset - (pIStm->Tell() - nStmPos));
                }

                bRet = ImplReadDIBBits(*pIStm, aHeader, *pAcc, pAccAlpha, bTopDown, bAlphaUsed);

                if(bRet && aHeader.nXPelsPerMeter && aHeader.nYPelsPerMeter)
                {
                    MapMode aMapMode(
                        MAP_MM,
                        Point(),
                        Fraction(1000, aHeader.nXPelsPerMeter),
                        Fraction(1000, aHeader.nYPelsPerMeter));

                    aNewBmp.SetPrefMapMode(aMapMode);
                    aNewBmp.SetPrefSize(Size(aHeader.nWidth, aHeader.nHeight));
                }
            }

            if( pData )
            {
                rtl_freeMemory(pData);
            }

            delete pMemStm;
            aNewBmp.ReleaseAccess(pAcc);

            if(bAlphaPossible)
            {
                aNewBmpAlpha.ReleaseAccess(pAccAlpha);

                if(!bAlphaUsed)
                {
                    bAlphaPossible = false;
                }
            }

            if(bRet)
            {
                rBmp = aNewBmp;

                if(bAlphaPossible)
                {
                    *pBmpAlpha = aNewBmpAlpha;
                }
            }
        }
    }

    return bRet;
}

bool ImplReadDIBFileHeader( SvStream& rIStm, sal_uLong& rOffset )
{
    bool bRet = false;

    const sal_uInt64 nSavedStreamPos( rIStm.Tell() );
    const sal_uInt64 nStreamLength( rIStm.Seek( STREAM_SEEK_TO_END ) );
    rIStm.Seek( nSavedStreamPos );

    sal_uInt16 nTmp16 = 0;
    rIStm.ReadUInt16( nTmp16 );

    if ( ( 0x4D42 == nTmp16 ) || ( 0x4142 == nTmp16 ) )
    {
        sal_uInt32 nTmp32(0);
        if ( 0x4142 == nTmp16 )
        {
            rIStm.SeekRel( 12L );
            rIStm.ReadUInt16( nTmp16 );
            rIStm.SeekRel( 8L );
            rIStm.ReadUInt32( nTmp32 );
            rOffset = nTmp32 - 28UL;
            bRet = ( 0x4D42 == nTmp16 );
        }
        else // 0x4D42 == nTmp16, 'MB' from BITMAPFILEHEADER
        {
            rIStm.SeekRel( 8L );        // we are on bfSize member of BITMAPFILEHEADER, forward to bfOffBits
            rIStm.ReadUInt32( nTmp32 );            // read bfOffBits
            rOffset = nTmp32 - 14UL;    // adapt offset by sizeof(BITMAPFILEHEADER)
            bRet = ( rIStm.GetError() == 0UL );
        }

        if ( rOffset >= nStreamLength )
        {
            // Offset claims that image starts past the end of the
            // stream.  Unlikely.
            rIStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
            bRet = false;
        }
    }
    else
        rIStm.SetError( SVSTREAM_FILEFORMAT_ERROR );

    return bRet;
}

bool ImplWriteDIBPalette( SvStream& rOStm, BitmapReadAccess& rAcc )
{
    const sal_uInt16    nColors = rAcc.GetPaletteEntryCount();
    const sal_uLong     nPalSize = nColors * 4UL;
    boost::scoped_array<sal_uInt8> pEntries(new sal_uInt8[ nPalSize ]);
    sal_uInt8*          pTmpEntry = pEntries.get();
    BitmapColor     aPalColor;

    for( sal_uInt16 i = 0; i < nColors; i++ )
    {
        const BitmapColor& rPalColor = rAcc.GetPaletteColor( i );

        *pTmpEntry++ = rPalColor.GetBlue();
        *pTmpEntry++ = rPalColor.GetGreen();
        *pTmpEntry++ = rPalColor.GetRed();
        *pTmpEntry++ = 0;
    }

    rOStm.Write( pEntries.get(), nPalSize );

    return( rOStm.GetError() == 0UL );
}

bool ImplWriteRLE( SvStream& rOStm, BitmapReadAccess& rAcc, bool bRLE4 )
{
    const sal_uLong nWidth = rAcc.Width();
    const sal_uLong nHeight = rAcc.Height();
    sal_uLong       nX;
    sal_uLong       nSaveIndex;
    sal_uLong       nCount;
    sal_uLong       nBufCount;
    boost::scoped_array<sal_uInt8> pBuf(new sal_uInt8[ ( nWidth << 1 ) + 2 ]);
    sal_uInt8*      pTmp;
    sal_uInt8       cPix;
    sal_uInt8       cLast;
    bool        bFound;

    for ( long nY = nHeight - 1L; nY >= 0L; nY-- )
    {
        pTmp = pBuf.get();
        nX = nBufCount = 0UL;

        while( nX < nWidth )
        {
            nCount = 1L;
            cPix = rAcc.GetPixelIndex( nY, nX++ );

            while( ( nX < nWidth ) && ( nCount < 255L )
                && ( cPix == rAcc.GetPixelIndex( nY, nX ) ) )
            {
                nX++;
                nCount++;
            }

            if ( nCount > 1 )
            {
                *pTmp++ = (sal_uInt8) nCount;
                *pTmp++ = ( bRLE4 ? ( ( cPix << 4 ) | cPix ) : cPix );
                nBufCount += 2;
            }
            else
            {
                cLast = cPix;
                nSaveIndex = nX - 1UL;
                bFound = false;

                while( ( nX < nWidth ) && ( nCount < 256L )
                    && ( cPix = rAcc.GetPixelIndex( nY, nX ) ) != cLast )
                {
                    nX++; nCount++;
                    cLast = cPix;
                    bFound = true;
                }

                if ( bFound )
                    nX--;

                if ( nCount > 3 )
                {
                    *pTmp++ = 0;
                    *pTmp++ = (sal_uInt8) --nCount;

                    if( bRLE4 )
                    {
                        for ( sal_uLong i = 0; i < nCount; i++, pTmp++ )
                        {
                            *pTmp = rAcc.GetPixelIndex( nY, nSaveIndex++ ) << 4;

                            if ( ++i < nCount )
                                *pTmp |= rAcc.GetPixelIndex( nY, nSaveIndex++ );
                        }

                        nCount = ( nCount + 1 ) >> 1;
                    }
                    else
                    {
                        for( sal_uLong i = 0UL; i < nCount; i++ )
                            *pTmp++ = rAcc.GetPixelIndex( nY, nSaveIndex++ );
                    }

                    if ( nCount & 1 )
                    {
                        *pTmp++ = 0;
                        nBufCount += ( nCount + 3 );
                    }
                    else
                        nBufCount += ( nCount + 2 );
                }
                else
                {
                    *pTmp++ = 1;
                    *pTmp++ = rAcc.GetPixelIndex( nY, nSaveIndex ) << (bRLE4 ? 4 : 0);

                    if ( nCount == 3 )
                    {
                        *pTmp++ = 1;
                        *pTmp++ = rAcc.GetPixelIndex( nY, ++nSaveIndex ) << ( bRLE4 ? 4 : 0 );
                        nBufCount += 4;
                    }
                    else
                        nBufCount += 2;
                }
            }
        }

        pBuf[ nBufCount++ ] = 0;
        pBuf[ nBufCount++ ] = 0;

        rOStm.Write( pBuf.get(), nBufCount );
    }

    rOStm.WriteUChar( 0 );
    rOStm.WriteUChar( 1 );

    return( rOStm.GetError() == 0UL );
}

bool ImplWriteDIBBits(SvStream& rOStm, BitmapReadAccess& rAcc, BitmapReadAccess* pAccAlpha, sal_uLong nCompression, sal_uInt32& rImageSize)
{
    if(!pAccAlpha && BITFIELDS == nCompression)
    {
        const ColorMask&    rMask = rAcc.GetColorMask();
        SVBT32              aVal32;

        UInt32ToSVBT32( rMask.GetRedMask(), aVal32 );
        rOStm.Write( (sal_uInt8*) aVal32, 4UL );

        UInt32ToSVBT32( rMask.GetGreenMask(), aVal32 );
        rOStm.Write( (sal_uInt8*) aVal32, 4UL );

        UInt32ToSVBT32( rMask.GetBlueMask(), aVal32 );
        rOStm.Write( (sal_uInt8*) aVal32, 4UL );

        rImageSize = rOStm.Tell();

        if( rAcc.IsBottomUp() )
            rOStm.Write( rAcc.GetBuffer(), rAcc.Height() * rAcc.GetScanlineSize() );
        else
        {
            for( long nY = rAcc.Height() - 1, nScanlineSize = rAcc.GetScanlineSize(); nY >= 0L; nY-- )
                rOStm.Write( rAcc.GetScanline( nY ), nScanlineSize );
        }
    }
    else if(!pAccAlpha && ((RLE_4 == nCompression) || (RLE_8 == nCompression)))
    {
        rImageSize = rOStm.Tell();
        ImplWriteRLE( rOStm, rAcc, RLE_4 == nCompression );
    }
    else if(!nCompression)
    {
        // #i5xxx# Limit bitcount to 24bit, the 32 bit cases are not
        // handled properly below (would have to set color masks, and
        // nCompression=BITFIELDS - but color mask is not set for
        // formats != *_TC_*). Note that this very problem might cause
        // trouble at other places - the introduction of 32 bit RGBA
        // bitmaps is relatively recent.
        // #i59239# discretize bitcount for aligned width to 1,4,8,24
        // (other cases are not written below)
        const sal_uInt16 nBitCount(pAccAlpha ? 32 : discretizeBitcount(static_cast< sal_uInt16 >(rAcc.GetBitCount())));
        const sal_uLong nAlignedWidth(AlignedWidth4Bytes(rAcc.Width() * nBitCount));
        bool bNative(false);

        switch(rAcc.GetScanlineFormat())
        {
            case( BMP_FORMAT_1BIT_MSB_PAL ):
            case( BMP_FORMAT_4BIT_MSN_PAL ):
            case( BMP_FORMAT_8BIT_PAL ):
            case( BMP_FORMAT_24BIT_TC_BGR ):
            {
                if(!pAccAlpha && rAcc.IsBottomUp() && (rAcc.GetScanlineSize() == nAlignedWidth))
                {
                    bNative = true;
                }

                break;
            }

            default:
            {
                break;
            }
        }

        rImageSize = rOStm.Tell();

        if(bNative)
        {
            rOStm.Write(rAcc.GetBuffer(), nAlignedWidth * rAcc.Height());
        }
        else
        {
            const long nWidth(rAcc.Width());
            const long nHeight(rAcc.Height());
            boost::scoped_array<sal_uInt8> pBuf(new sal_uInt8[ nAlignedWidth ]);
            sal_uInt8* pTmp(0);
            sal_uInt8 cTmp(0);

            switch( nBitCount )
            {
                case( 1 ):
                {
                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
                    {
                        pTmp = pBuf.get();
                        cTmp = 0;

                        for( long nX = 0L, nShift = 8L; nX < nWidth; nX++ )
                        {
                            if( !nShift )
                            {
                                nShift = 8L;
                                *pTmp++ = cTmp;
                                cTmp = 0;
                            }

                            cTmp |= rAcc.GetPixelIndex( nY, nX ) << --nShift;
                        }

                        *pTmp = cTmp;
                        rOStm.Write( pBuf.get(), nAlignedWidth );
                    }
                }
                break;

                case( 4 ):
                {
                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
                    {
                        pTmp = pBuf.get();
                        cTmp = 0;

                        for( long nX = 0L, nShift = 2L; nX < nWidth; nX++ )
                        {
                            if( !nShift )
                            {
                                nShift = 2L;
                                *pTmp++ = cTmp;
                                cTmp = 0;
                            }

                            cTmp |= rAcc.GetPixelIndex( nY, nX ) << ( --nShift << 2L );
                        }
                        *pTmp = cTmp;
                        rOStm.Write( pBuf.get(), nAlignedWidth );
                    }
                }
                break;

                case( 8 ):
                {
                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
                    {
                        pTmp = pBuf.get();

                        for( long nX = 0L; nX < nWidth; nX++ )
                            *pTmp++ = rAcc.GetPixelIndex( nY, nX );

                        rOStm.Write( pBuf.get(), nAlignedWidth );
                    }
                }
                break;

                // #i59239# fallback to 24 bit format, if bitcount is non-default
                default:
                    // FALLTHROUGH intended
                case( 24 ):
                {
                    BitmapColor aPixelColor;
                    const bool bWriteAlpha(32 == nBitCount && pAccAlpha);

                    for( long nY = nHeight - 1; nY >= 0L; nY-- )
                    {
                        pTmp = pBuf.get();

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            // when alpha is used, this may be non-24bit main bitmap, so use GetColor
                            // instead of GetPixel to ensure RGB value
                            aPixelColor = rAcc.GetColor( nY, nX );

                            *pTmp++ = aPixelColor.GetBlue();
                            *pTmp++ = aPixelColor.GetGreen();
                            *pTmp++ = aPixelColor.GetRed();

                            if(bWriteAlpha)
                            {
                                *pTmp++ = (sal_uInt8)0xff - (sal_uInt8)pAccAlpha->GetPixelIndex( nY, nX );
                            }
                        }

                        rOStm.Write( pBuf.get(), nAlignedWidth );
                    }
                }
                break;
            }
        }
    }

    rImageSize = rOStm.Tell() - rImageSize;

    return (!rOStm.GetError());
}

bool ImplWriteDIBBody(const Bitmap& rBitmap, SvStream& rOStm, BitmapReadAccess& rAcc, BitmapReadAccess* pAccAlpha, bool bCompressed)
{
    const MapMode aMapPixel(MAP_PIXEL);
    DIBV5Header aHeader;
    sal_uLong nImageSizePos(0);
    sal_uLong nEndPos(0);
    sal_uInt32 nCompression(COMPRESS_NONE);
    bool bRet(false);

    aHeader.nSize = pAccAlpha ? DIBV5HEADERSIZE : DIBINFOHEADERSIZE; // size dependent on CF_DIB type to use
    aHeader.nWidth = rAcc.Width();
    aHeader.nHeight = rAcc.Height();
    aHeader.nPlanes = 1;

    if(!pAccAlpha && isBitfieldCompression(rAcc.GetScanlineFormat()))
    {
        aHeader.nBitCount = (BMP_FORMAT_16BIT_TC_LSB_MASK == rAcc.GetScanlineFormat()) ? 16 : 32;
        aHeader.nSizeImage = rAcc.Height() * rAcc.GetScanlineSize();
        nCompression = BITFIELDS;
    }
    else
    {
        // #i5xxx# Limit bitcount to 24bit, the 32 bit cases are
        // not handled properly below (would have to set color
        // masks, and nCompression=BITFIELDS - but color mask is
        // not set for formats != *_TC_*). Note that this very
        // problem might cause trouble at other places - the
        // introduction of 32 bit RGBA bitmaps is relatively
        // recent.
        // #i59239# discretize bitcount to 1,4,8,24 (other cases
        // are not written below)
        const sal_uInt16 nBitCount(pAccAlpha ? 32 : discretizeBitcount(static_cast< sal_uInt16 >(rAcc.GetBitCount())));
        aHeader.nBitCount = nBitCount;
        aHeader.nSizeImage = rAcc.Height() * AlignedWidth4Bytes(rAcc.Width() * aHeader.nBitCount);

        if(bCompressed)
        {
            if(4 == nBitCount)
            {
                nCompression = RLE_4;
            }
            else if(8 == nBitCount)
            {
                nCompression = RLE_8;
            }
        }
    }

    if((rOStm.GetCompressMode() & COMPRESSMODE_ZBITMAP) && (rOStm.GetVersion() >= SOFFICE_FILEFORMAT_40))
    {
        aHeader.nCompression = ZCOMPRESS;
    }
    else
    {
        aHeader.nCompression = nCompression;
    }

    if(rBitmap.GetPrefSize().Width() && rBitmap.GetPrefSize().Height() && (rBitmap.GetPrefMapMode() != aMapPixel))
    {
        // #i48108# Try to recover xpels/ypels as previously stored on
        // disk. The problem with just converting maPrefSize to 100th
        // mm and then relating that to the bitmap pixel size is that
        // MapMode is integer-based, and suffers from roundoffs,
        // especially if maPrefSize is small. Trying to circumvent
        // that by performing part of the math in floating point.
        const Size aScale100000(OutputDevice::LogicToLogic(Size(100000L, 100000L), MAP_100TH_MM, rBitmap.GetPrefMapMode()));
        const double fBmpWidthM((double)rBitmap.GetPrefSize().Width() / aScale100000.Width());
        const double fBmpHeightM((double)rBitmap.GetPrefSize().Height() / aScale100000.Height());

        if(!basegfx::fTools::equalZero(fBmpWidthM) && !basegfx::fTools::equalZero(fBmpHeightM))
        {
            aHeader.nXPelsPerMeter = basegfx::fround(rAcc.Width() / fabs(fBmpWidthM));
            aHeader.nYPelsPerMeter = basegfx::fround(rAcc.Height() / fabs(fBmpHeightM));
        }
    }

    aHeader.nColsUsed = ((!pAccAlpha && aHeader.nBitCount <= 8) ? rAcc.GetPaletteEntryCount() : 0);
    aHeader.nColsImportant = 0;

    rOStm.WriteUInt32( aHeader.nSize );
    rOStm.WriteInt32( aHeader.nWidth );
    rOStm.WriteInt32( aHeader.nHeight );
    rOStm.WriteUInt16( aHeader.nPlanes );
    rOStm.WriteUInt16( aHeader.nBitCount );
    rOStm.WriteUInt32( aHeader.nCompression );

    nImageSizePos = rOStm.Tell();
    rOStm.SeekRel( sizeof( aHeader.nSizeImage ) );

    rOStm.WriteInt32( aHeader.nXPelsPerMeter );
    rOStm.WriteInt32( aHeader.nYPelsPerMeter );
    rOStm.WriteUInt32( aHeader.nColsUsed );
    rOStm.WriteUInt32( aHeader.nColsImportant );

    if(pAccAlpha) // only write DIBV5 when asked to do so
    {
        aHeader.nV5CSType = 0x57696E20; // LCS_WINDOWS_COLOR_SPACE
        aHeader.nV5Intent = 0x00000004; // LCS_GM_IMAGES

        rOStm.WriteUInt32( aHeader.nV5RedMask );
        rOStm.WriteUInt32( aHeader.nV5GreenMask );
        rOStm.WriteUInt32( aHeader.nV5BlueMask );
        rOStm.WriteUInt32( aHeader.nV5AlphaMask );
        rOStm.WriteUInt32( aHeader.nV5CSType );

        rOStm.WriteInt32( aHeader.aV5Endpoints.aXyzRed.aXyzX );
        rOStm.WriteInt32( aHeader.aV5Endpoints.aXyzRed.aXyzY );
        rOStm.WriteInt32( aHeader.aV5Endpoints.aXyzRed.aXyzZ );
        rOStm.WriteInt32( aHeader.aV5Endpoints.aXyzGreen.aXyzX );
        rOStm.WriteInt32( aHeader.aV5Endpoints.aXyzGreen.aXyzY );
        rOStm.WriteInt32( aHeader.aV5Endpoints.aXyzGreen.aXyzZ );
        rOStm.WriteInt32( aHeader.aV5Endpoints.aXyzBlue.aXyzX );
        rOStm.WriteInt32( aHeader.aV5Endpoints.aXyzBlue.aXyzY );
        rOStm.WriteInt32( aHeader.aV5Endpoints.aXyzBlue.aXyzZ );

        rOStm.WriteUInt32( aHeader.nV5GammaRed );
        rOStm.WriteUInt32( aHeader.nV5GammaGreen );
        rOStm.WriteUInt32( aHeader.nV5GammaBlue );
        rOStm.WriteUInt32( aHeader.nV5Intent );
        rOStm.WriteUInt32( aHeader.nV5ProfileData );
        rOStm.WriteUInt32( aHeader.nV5ProfileSize );
        rOStm.WriteUInt32( aHeader.nV5Reserved );
    }

    if(ZCOMPRESS == aHeader.nCompression)
    {
        ZCodec aCodec;
        SvMemoryStream aMemStm(aHeader.nSizeImage + 4096, 65535);
        sal_uLong nCodedPos(rOStm.Tell());
        sal_uLong nLastPos(0);
        sal_uInt32 nCodedSize(0);
        sal_uInt32 nUncodedSize(0);

        // write uncoded data palette
        if(aHeader.nColsUsed)
        {
            ImplWriteDIBPalette(aMemStm, rAcc);
        }

        // write uncoded bits
        bRet = ImplWriteDIBBits(aMemStm, rAcc, pAccAlpha, nCompression, aHeader.nSizeImage);

        // get uncoded size
        nUncodedSize = aMemStm.Tell();

        // seek over compress info
        rOStm.SeekRel(12);

        // write compressed data
        aCodec.BeginCompression(3);
        aCodec.Write(rOStm, (sal_uInt8*)aMemStm.GetData(), nUncodedSize);
        aCodec.EndCompression();

        // update compress info ( coded size, uncoded size, uncoded compression )
        nLastPos = rOStm.Tell();
        nCodedSize = nLastPos - nCodedPos - 12;
        rOStm.Seek(nCodedPos);
        rOStm.WriteUInt32( nCodedSize ).WriteUInt32( nUncodedSize ).WriteUInt32( nCompression );
        rOStm.Seek(nLastPos);

        if(bRet)
        {
            bRet = (ERRCODE_NONE == rOStm.GetError());
        }
    }
    else
    {
        if(aHeader.nColsUsed)
        {
            ImplWriteDIBPalette(rOStm, rAcc);
        }

        bRet = ImplWriteDIBBits(rOStm, rAcc, pAccAlpha, aHeader.nCompression, aHeader.nSizeImage);
    }

    nEndPos = rOStm.Tell();
    rOStm.Seek(nImageSizePos);
    rOStm.WriteUInt32( aHeader.nSizeImage );
    rOStm.Seek(nEndPos);

    return bRet;
}

bool ImplWriteDIBFileHeader(SvStream& rOStm, BitmapReadAccess& rAcc, bool bUseDIBV5)
{
    const sal_uInt32 nPalCount((rAcc.HasPalette() ? rAcc.GetPaletteEntryCount() : isBitfieldCompression(rAcc.GetScanlineFormat()) ? 3UL : 0UL));
    const sal_uInt32 nOffset(14 + (bUseDIBV5 ? DIBV5HEADERSIZE : DIBINFOHEADERSIZE) + nPalCount * 4UL);

    rOStm.WriteUInt16( 0x4D42 ); // 'MB' from BITMAPFILEHEADER
    rOStm.WriteUInt32( (nOffset + (rAcc.Height() * rAcc.GetScanlineSize())) );
    rOStm.WriteUInt16( 0 );
    rOStm.WriteUInt16( 0 );
    rOStm.WriteUInt32( nOffset );

    return( rOStm.GetError() == 0UL );
}

bool ImplReadDIB(
    Bitmap& rTarget, Bitmap*
    pTargetAlpha,
    SvStream& rIStm,
    bool bFileHeader,
    bool bMSOFormat=false)
{
    const sal_uInt16 nOldFormat(rIStm.GetNumberFormatInt());
    const sal_uLong nOldPos(rIStm.Tell());
    sal_uLong nOffset(0UL);
    bool bRet(false);

    rIStm.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);

    if(bFileHeader)
    {
        if(ImplReadDIBFileHeader(rIStm, nOffset))
        {
            bRet = ImplReadDIBBody(rIStm, rTarget, nOffset >= DIBV5HEADERSIZE ? pTargetAlpha : 0, nOffset);
        }
    }
    else
    {
        bRet = ImplReadDIBBody(rIStm, rTarget, 0, nOffset, bMSOFormat);
    }

    if(!bRet)
    {
        if(!rIStm.GetError())
        {
            rIStm.SetError(SVSTREAM_GENERALERROR);
        }

        rIStm.Seek(nOldPos);
    }

    rIStm.SetNumberFormatInt(nOldFormat);

    return bRet;
}

bool ImplWriteDIB(
    const Bitmap& rSource,
    const Bitmap* pSourceAlpha,
    SvStream& rOStm,
    bool bCompressed,
    bool bFileHeader)
{
    const Size aSizePix(rSource.GetSizePixel());
    bool bRet(false);

    if(aSizePix.Width() && aSizePix.Height())
    {
        BitmapReadAccess* pAcc = const_cast< Bitmap& >(rSource).AcquireReadAccess();
        BitmapReadAccess* pAccAlpha = 0;
        const sal_uInt16 nOldFormat(rOStm.GetNumberFormatInt());
        const sal_uLong nOldPos(rOStm.Tell());

        if(pSourceAlpha)
        {
            const Size aSizePixAlpha(pSourceAlpha->GetSizePixel());

            if(aSizePixAlpha == aSizePix)
            {
                pAccAlpha = const_cast< Bitmap* >(pSourceAlpha)->AcquireReadAccess();
            }
            else
            {
                OSL_ENSURE(false, "WriteDIB got an alpha channel, but it's pixel size differs from the base bitmap (!)");
            }
        }

        rOStm.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);

        if(pAcc)
        {
            if(bFileHeader)
            {
                if(ImplWriteDIBFileHeader(rOStm, *pAcc, 0 != pSourceAlpha))
                {
                    bRet = ImplWriteDIBBody(rSource, rOStm, *pAcc, pAccAlpha, bCompressed);
                }
            }
            else
            {
                bRet = ImplWriteDIBBody(rSource, rOStm, *pAcc, pAccAlpha, bCompressed);
            }

            const_cast< Bitmap& >(rSource).ReleaseAccess(pAcc);

            if(pAccAlpha)
            {
                const_cast< Bitmap* >(pSourceAlpha)->ReleaseAccess(pAccAlpha);
            }
        }

        if(!bRet)
        {
            rOStm.SetError(SVSTREAM_GENERALERROR);
            rOStm.Seek(nOldPos);
        }

        rOStm.SetNumberFormatInt(nOldFormat);
    }

    return bRet;
}

bool ReadDIB(
    Bitmap& rTarget,
    SvStream& rIStm,
    bool bFileHeader,
    bool bMSOFormat)
{
    return ImplReadDIB(rTarget, 0, rIStm, bFileHeader, bMSOFormat);
}

bool ReadDIBBitmapEx(
    BitmapEx& rTarget,
    SvStream& rIStm)
{
    Bitmap aBmp;
    bool bRetval(ImplReadDIB(aBmp, 0, rIStm, true) && !rIStm.GetError());

    if(bRetval)
    {
        // base bitmap was read, set as return value and try to read alpha extra-data
        const sal_uLong nStmPos(rIStm.Tell());
        sal_uInt32 nMagic1(0);
        sal_uInt32 nMagic2(0);

        rTarget = BitmapEx(aBmp);
        rIStm.ReadUInt32( nMagic1 ).ReadUInt32( nMagic2 );
        bRetval = (0x25091962 == nMagic1) && (0xACB20201 == nMagic2) && !rIStm.GetError();

        if(bRetval)
        {
            sal_uInt8 bTransparent(false);

            rIStm.ReadUChar( bTransparent );
            bRetval = !rIStm.GetError();

            if(bRetval)
            {
                if((sal_uInt8)TRANSPARENT_BITMAP == bTransparent)
                {
                    Bitmap aMask;

                    bRetval = ImplReadDIB(aMask, 0, rIStm, true);

                    if(bRetval)
                    {
                        if(!!aMask)
                        {
                            // do we have an alpha mask?
                            if((8 == aMask.GetBitCount()) && aMask.HasGreyPalette())
                            {
                                AlphaMask aAlpha;

                                // create alpha mask quickly (without greyscale conversion)
                                aAlpha.ImplSetBitmap(aMask);
                                rTarget = BitmapEx(aBmp, aAlpha);
                            }
                            else
                            {
                                rTarget = BitmapEx(aBmp, aMask);
                            }
                        }
                    }
                }
                else if((sal_uInt8)TRANSPARENT_COLOR == bTransparent)
                {
                    Color aTransparentColor;

                    ReadColor( rIStm, aTransparentColor );
                    bRetval = !rIStm.GetError();

                    if(bRetval)
                    {
                        rTarget = BitmapEx(aBmp, aTransparentColor);
                    }
                }
            }
        }

        if(!bRetval)
        {
            // alpha extra data could not be read; reset, but use base bitmap as result
            rIStm.ResetError();
            rIStm.Seek(nStmPos);
            bRetval = true;
        }
    }

    return bRetval;
}

bool ReadDIBV5(
    Bitmap& rTarget,
    Bitmap& rTargetAlpha,
    SvStream& rIStm)
{
    return ImplReadDIB(rTarget, &rTargetAlpha, rIStm, true);
}

bool WriteDIB(
    const Bitmap& rSource,
    SvStream& rOStm,
    bool bCompressed,
    bool bFileHeader)
{
    return ImplWriteDIB(rSource, 0, rOStm, bCompressed, bFileHeader);
}

bool WriteDIBBitmapEx(
    const BitmapEx& rSource,
    SvStream& rOStm)
{
    if(ImplWriteDIB(rSource.GetBitmap(), 0, rOStm, true, true))
    {
        rOStm.WriteUInt32( 0x25091962 );
        rOStm.WriteUInt32( 0xACB20201 );
        rOStm.WriteUChar( rSource.eTransparent );

        if(TRANSPARENT_BITMAP == rSource.eTransparent)
        {
            return ImplWriteDIB(rSource.aMask, 0, rOStm, true, true);
        }
        else if(TRANSPARENT_COLOR == rSource.eTransparent)
        {
            WriteColor( rOStm, rSource.aTransparentColor );
            return true;
        }
    }

    return false;
}

bool WriteDIBV5(
    const Bitmap& rSource,
    const Bitmap& rSourceAlpha,
    SvStream& rOStm)
{
    return ImplWriteDIB(rSource, &rSourceAlpha, rOStm, false, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
