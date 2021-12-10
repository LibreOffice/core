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
#include <sal/log.hxx>

#include <cassert>

#include <o3tl/safeint.hxx>
#include <vcl/dibtools.hxx>
#include <comphelper/fileformat.h>
#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include <tools/fract.hxx>
#include <tools/helpers.hxx>
#include <tools/GenericTypeSerializer.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/outdev.hxx>
#include <bitmap/BitmapWriteAccess.hxx>
#include <memory>

#define DIBCOREHEADERSIZE       ( 12UL )
#define DIBINFOHEADERSIZE       ( sizeof(DIBInfoHeader) )
#define DIBV5HEADERSIZE         ( sizeof(DIBV5Header) )

// - DIBInfoHeader and DIBV5Header

typedef sal_Int32 FXPT2DOT30;

namespace
{

struct CIEXYZ
{
    FXPT2DOT30      aXyzX;
    FXPT2DOT30      aXyzY;
    FXPT2DOT30      aXyzZ;

    CIEXYZ()
    :   aXyzX(0),
        aXyzY(0),
        aXyzZ(0)
    {}
};

struct CIEXYZTriple
{
    CIEXYZ          aXyzRed;
    CIEXYZ          aXyzGreen;
    CIEXYZ          aXyzBlue;

    CIEXYZTriple()
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
    :   nSize(0),
        nWidth(0),
        nHeight(0),
        nPlanes(0),
        nBitCount(0),
        nCompression(0),
        nSizeImage(0),
        nXPelsPerMeter(0),
        nYPelsPerMeter(0),
        nColsUsed(0),
        nColsImportant(0)
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
    :   nV5RedMask(0),
        nV5GreenMask(0),
        nV5BlueMask(0),
        nV5AlphaMask(0),
        nV5CSType(0),
        nV5GammaRed(0),
        nV5GammaGreen(0),
        nV5GammaBlue(0),
        nV5Intent(0),
        nV5ProfileData(0),
        nV5ProfileSize(0),
        nV5Reserved(0)
    {}
};

vcl::PixelFormat convertToBPP(sal_uInt16 nCount)
{
    return (nCount <= 1) ? vcl::PixelFormat::N1_BPP :
           (nCount <= 8) ? vcl::PixelFormat::N8_BPP :
                           vcl::PixelFormat::N24_BPP;
}

bool isBitfieldCompression( ScanlineFormat nScanlineFormat )
{
    return ScanlineFormat::N32BitTcMask == nScanlineFormat;
}

bool ImplReadDIBInfoHeader(SvStream& rIStm, DIBV5Header& rHeader, bool& bTopDown, bool bMSOFormat)
{
    if (rIStm.remainingSize() <= 4)
        return false;
    // BITMAPINFOHEADER or BITMAPCOREHEADER or BITMAPV5HEADER
    sal_uInt64 const aStartPos(rIStm.Tell());
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
        std::size_t nUsed(sizeof(rHeader.nSize));

        auto readUInt16 = [&nUsed, &rHeader, &rIStm](sal_uInt16 & v) {
            if (nUsed < rHeader.nSize) {
                rIStm.ReadUInt16(v);
                nUsed += sizeof(v);
            }
        };
        auto readInt32 = [&nUsed, &rHeader, &rIStm](sal_Int32 & v) {
            if (nUsed < rHeader.nSize) {
                rIStm.ReadInt32(v);
                nUsed += sizeof(v);
            }
        };
        auto readUInt32 = [&nUsed, &rHeader, &rIStm](sal_uInt32 & v) {
            if (nUsed < rHeader.nSize) {
                rIStm.ReadUInt32(v);
                nUsed += sizeof(v);
            }
        };

        // read DIBInfoHeader entries
        readInt32( rHeader.nWidth );
        readInt32( rHeader.nHeight );
        readUInt16( rHeader.nPlanes );
        readUInt16( rHeader.nBitCount );
        readUInt32( rHeader.nCompression );
        readUInt32( rHeader.nSizeImage );
        readInt32( rHeader.nXPelsPerMeter );
        readInt32( rHeader.nYPelsPerMeter );
        readUInt32( rHeader.nColsUsed );
        readUInt32( rHeader.nColsImportant );

        // read DIBV5HEADER members
        readUInt32( rHeader.nV5RedMask );
        readUInt32( rHeader.nV5GreenMask );
        readUInt32( rHeader.nV5BlueMask );
        readUInt32( rHeader.nV5AlphaMask );
        readUInt32( rHeader.nV5CSType );

        // read contained CIEXYZTriple's
        readInt32( rHeader.aV5Endpoints.aXyzRed.aXyzX );
        readInt32( rHeader.aV5Endpoints.aXyzRed.aXyzY );
        readInt32( rHeader.aV5Endpoints.aXyzRed.aXyzZ );
        readInt32( rHeader.aV5Endpoints.aXyzGreen.aXyzX );
        readInt32( rHeader.aV5Endpoints.aXyzGreen.aXyzY );
        readInt32( rHeader.aV5Endpoints.aXyzGreen.aXyzZ );
        readInt32( rHeader.aV5Endpoints.aXyzBlue.aXyzX );
        readInt32( rHeader.aV5Endpoints.aXyzBlue.aXyzY );
        readInt32( rHeader.aV5Endpoints.aXyzBlue.aXyzZ );

        readUInt32( rHeader.nV5GammaRed );
        readUInt32( rHeader.nV5GammaGreen );
        readUInt32( rHeader.nV5GammaBlue );
        readUInt32( rHeader.nV5Intent );
        readUInt32( rHeader.nV5ProfileData );
        readUInt32( rHeader.nV5ProfileSize );
        readUInt32( rHeader.nV5Reserved );

        // seek to EndPos
        if (!checkSeek(rIStm, aStartPos + rHeader.nSize))
            return false;
    }

    if (!rIStm.good() || rHeader.nHeight == SAL_MIN_INT32)
        return false;

    if ( rHeader.nHeight < 0 )
    {
        bTopDown = true;
        rHeader.nHeight *= -1;
    }
    else
    {
        bTopDown = false;
    }

    if ( rHeader.nWidth < 0 || rHeader.nXPelsPerMeter < 0 || rHeader.nYPelsPerMeter < 0 )
    {
        rIStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
    }

    // #144105# protect a little against damaged files
    assert(rHeader.nHeight >= 0);
    if (rHeader.nHeight != 0 && rHeader.nWidth >= 0
        && (rHeader.nSizeImage / 16 / static_cast<sal_uInt32>(rHeader.nHeight)
            > o3tl::make_unsigned(rHeader.nWidth)))
    {
        rHeader.nSizeImage = 0;
    }


    if (rHeader.nPlanes != 1)
        return false;

    if (rHeader.nBitCount != 0 && rHeader.nBitCount != 1 &&
        rHeader.nBitCount != 4 && rHeader.nBitCount != 8 &&
        rHeader.nBitCount != 16 && rHeader.nBitCount != 24 &&
        rHeader.nBitCount != 32)
    {
        return false;
    }

    return rIStm.good();
}

bool ImplReadDIBPalette(SvStream& rIStm, BitmapPalette& rPal, bool bQuad)
{
    const sal_uInt16    nColors = rPal.GetEntryCount();
    const sal_uLong     nPalSize = nColors * ( bQuad ? 4UL : 3UL );
    BitmapColor     aPalColor;

    std::unique_ptr<sal_uInt8[]> pEntries(new sal_uInt8[ nPalSize ]);
    if (rIStm.ReadBytes(pEntries.get(), nPalSize) != nPalSize)
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

        rPal[i] = aPalColor;
    }

    return rIStm.GetError() == ERRCODE_NONE;
}

BitmapColor SanitizePaletteIndex(sal_uInt8 nIndex, BitmapPalette& rPalette, bool bForceToMonoWhileReading)
{
    const sal_uInt16 nPaletteEntryCount = rPalette.GetEntryCount();
    if (nPaletteEntryCount && nIndex >= nPaletteEntryCount)
    {
        auto nSanitizedIndex = nIndex % nPaletteEntryCount;
        SAL_WARN_IF(nIndex != nSanitizedIndex, "vcl", "invalid colormap index: "
                    << static_cast<unsigned int>(nIndex) << ", colormap len is: "
                    << nPaletteEntryCount);
        nIndex = nSanitizedIndex;
    }

    if (nPaletteEntryCount && bForceToMonoWhileReading)
    {
        return BitmapColor(static_cast<sal_uInt8>(rPalette[nIndex].GetLuminance() >= 255));
    }

    return BitmapColor(nIndex);
}

BitmapColor SanitizeColor(const BitmapColor &rColor, bool bForceToMonoWhileReading)
{
    if (!bForceToMonoWhileReading)
        return rColor;
    return BitmapColor(static_cast<sal_uInt8>(rColor.GetLuminance() >= 255));
}

bool ImplDecodeRLE(sal_uInt8* pBuffer, DIBV5Header const & rHeader, BitmapWriteAccess& rAcc, BitmapPalette& rPalette, bool bForceToMonoWhileReading, bool bRLE4)
{
    Scanline pRLE = pBuffer;
    Scanline pEndRLE = pBuffer + rHeader.nSizeImage;
    tools::Long        nY = rHeader.nHeight - 1;
    const sal_uLong nWidth = rAcc.Width();
    sal_uLong       nCountByte;
    sal_uLong       nRunByte;
    sal_uLong       nX = 0;
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
                Scanline pScanline = rAcc.GetScanline(nY);
                if( bRLE4 )
                {
                    nCountByte = nRunByte >> 1;

                    for( sal_uLong i = 0; i < nCountByte; i++ )
                    {
                        if (pRLE == pEndRLE)
                            return false;

                        cTmp = *pRLE++;

                        if( nX < nWidth )
                            rAcc.SetPixelOnData(pScanline, nX++, SanitizePaletteIndex(cTmp >> 4, rPalette, bForceToMonoWhileReading));

                        if( nX < nWidth )
                            rAcc.SetPixelOnData(pScanline, nX++, SanitizePaletteIndex(cTmp & 0x0f, rPalette, bForceToMonoWhileReading));
                    }

                    if( nRunByte & 1 )
                    {
                        if (pRLE == pEndRLE)
                            return false;

                        if( nX < nWidth )
                            rAcc.SetPixelOnData(pScanline, nX++, SanitizePaletteIndex(*pRLE >> 4, rPalette, bForceToMonoWhileReading));

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
                    for( sal_uLong i = 0; i < nRunByte; i++ )
                    {
                        if (pRLE == pEndRLE)
                            return false;

                        if( nX < nWidth )
                            rAcc.SetPixelOnData(pScanline, nX++, SanitizePaletteIndex(*pRLE, rPalette, bForceToMonoWhileReading));

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
                nX = 0;
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

            Scanline pScanline = rAcc.GetScanline(nY);
            if( bRLE4 )
            {
                nRunByte = nCountByte >> 1;

                for (sal_uLong i = 0; i < nRunByte && nX < nWidth; ++i)
                {
                    rAcc.SetPixelOnData(pScanline, nX++, SanitizePaletteIndex(cTmp >> 4, rPalette, bForceToMonoWhileReading));
                    if( nX < nWidth )
                        rAcc.SetPixelOnData(pScanline, nX++, SanitizePaletteIndex(cTmp & 0x0f, rPalette, bForceToMonoWhileReading));
                }

                if( ( nCountByte & 1 ) && ( nX < nWidth ) )
                    rAcc.SetPixelOnData(pScanline, nX++, SanitizePaletteIndex(cTmp >> 4, rPalette, bForceToMonoWhileReading));
            }
            else
            {
                for (sal_uLong i = 0; i < nCountByte && nX < nWidth; ++i)
                    rAcc.SetPixelOnData(pScanline, nX++, SanitizePaletteIndex(cTmp, rPalette, bForceToMonoWhileReading));
            }
        }
    }
    while (!bEndDecoding && (nY >= 0));

    return true;
}

bool ImplReadDIBBits(SvStream& rIStm, DIBV5Header& rHeader, BitmapWriteAccess& rAcc, BitmapPalette& rPalette, BitmapWriteAccess* pAccAlpha,
                     bool bTopDown, bool& rAlphaUsed, const sal_uInt64 nAlignedWidth,
                     const bool bForceToMonoWhileReading)
{
    sal_uInt32 nRMask(( rHeader.nBitCount == 16 ) ? 0x00007c00UL : 0x00ff0000UL);
    sal_uInt32 nGMask(( rHeader.nBitCount == 16 ) ? 0x000003e0UL : 0x0000ff00UL);
    sal_uInt32 nBMask(( rHeader.nBitCount == 16 ) ? 0x0000001fUL : 0x000000ffUL);
    bool bNative(false);
    bool bTCMask(!pAccAlpha && ((16 == rHeader.nBitCount) || (32 == rHeader.nBitCount)));
    bool bRLE((RLE_8 == rHeader.nCompression && 8 == rHeader.nBitCount) || (RLE_4 == rHeader.nCompression && 4 == rHeader.nBitCount));

    // Is native format?
    switch(rAcc.GetScanlineFormat())
    {
        case ScanlineFormat::N1BitMsbPal:
        case ScanlineFormat::N24BitTcBgr:
        {
            // we can't trust arbitrary-sourced index based formats to have correct indexes, so we exclude the pal formats
            // from raw read and force checking their colormap indexes
            bNative = ( ( rAcc.IsBottomUp() != bTopDown ) && !bRLE && !bTCMask && ( rAcc.GetScanlineSize() == nAlignedWidth ) );
            break;
        }

        default:
        {
            break;
        }
    }

    // Read data
    if (bNative)
    {
        if (nAlignedWidth
            > std::numeric_limits<std::size_t>::max() / rHeader.nHeight)
        {
            return false;
        }
        std::size_t n = nAlignedWidth * rHeader.nHeight;
        if (rIStm.ReadBytes(rAcc.GetBuffer(), n) != n)
        {
            return false;
        }
    }
    else
    {
        // Read color mask
        if(bTCMask && BITFIELDS == rHeader.nCompression)
        {
            rIStm.SeekRel( -12 );
            rIStm.ReadUInt32( nRMask );
            rIStm.ReadUInt32( nGMask );
            rIStm.ReadUInt32( nBMask );
        }

        const tools::Long nWidth(rHeader.nWidth);
        const tools::Long nHeight(rHeader.nHeight);
        tools::Long nResult = 0;
        if (utl::ConfigManager::IsFuzzing() && (o3tl::checked_multiply(nWidth, nHeight, nResult) || nResult > 4000000))
            return false;

        if (bRLE)
        {
            if(!rHeader.nSizeImage)
            {
                rHeader.nSizeImage = rIStm.remainingSize();
            }

            if (rHeader.nSizeImage > rIStm.remainingSize())
                return false;
            std::vector<sal_uInt8> aBuffer(rHeader.nSizeImage);
            if (rIStm.ReadBytes(aBuffer.data(), rHeader.nSizeImage) != rHeader.nSizeImage)
                return false;
            if (!ImplDecodeRLE(aBuffer.data(), rHeader, rAcc, rPalette, bForceToMonoWhileReading, RLE_4 == rHeader.nCompression))
                return false;
        }
        else
        {
            if (nAlignedWidth > rIStm.remainingSize())
            {
                // ofz#11188 avoid timeout
                // all following paths will enter a case statement, and nCount
                // is always at least 1, so we can check here before allocation
                // if at least one row can be read
                return false;
            }
            std::vector<sal_uInt8> aBuf(nAlignedWidth);

            const tools::Long nI(bTopDown ? 1 : -1);
            tools::Long nY(bTopDown ? 0 : nHeight - 1);
            tools::Long nCount(nHeight);

            switch(rHeader.nBitCount)
            {
                case 1:
                {
                    for( ; nCount--; nY += nI )
                    {
                        sal_uInt8 * pTmp = aBuf.data();
                        if (rIStm.ReadBytes(pTmp, nAlignedWidth)
                            != nAlignedWidth)
                        {
                            return false;
                        }
                        sal_uInt8   cTmp = *pTmp++;
                        Scanline pScanline = rAcc.GetScanline(nY);
                        for( tools::Long nX = 0, nShift = 8; nX < nWidth; nX++ )
                        {
                            if( !nShift )
                            {
                                nShift = 8;
                                cTmp = *pTmp++;
                            }

                            auto nIndex = (cTmp >> --nShift) & 1;
                            rAcc.SetPixelOnData(pScanline, nX, SanitizePaletteIndex(nIndex, rPalette, bForceToMonoWhileReading));
                        }
                    }
                }
                break;

                case 4:
                {
                    for( ; nCount--; nY += nI )
                    {
                        sal_uInt8 * pTmp = aBuf.data();
                        if (rIStm.ReadBytes(pTmp, nAlignedWidth)
                            != nAlignedWidth)
                        {
                            return false;
                        }
                        sal_uInt8   cTmp = *pTmp++;
                        Scanline pScanline = rAcc.GetScanline(nY);
                        for( tools::Long nX = 0, nShift = 2; nX < nWidth; nX++ )
                        {
                            if( !nShift )
                            {
                                nShift = 2;
                                cTmp = *pTmp++;
                            }

                            auto nIndex = (cTmp >> ( --nShift << 2 ) ) & 0x0f;
                            rAcc.SetPixelOnData(pScanline, nX, SanitizePaletteIndex(nIndex, rPalette, bForceToMonoWhileReading));
                        }
                    }
                }
                break;

                case 8:
                {
                    for( ; nCount--; nY += nI )
                    {
                        sal_uInt8 * pTmp = aBuf.data();
                        if (rIStm.ReadBytes(pTmp, nAlignedWidth)
                            != nAlignedWidth)
                        {
                            return false;
                        }

                        Scanline pScanline = rAcc.GetScanline(nY);
                        for( tools::Long nX = 0; nX < nWidth; nX++ )
                        {
                            auto nIndex = *pTmp++;
                            rAcc.SetPixelOnData(pScanline, nX, SanitizePaletteIndex(nIndex, rPalette, bForceToMonoWhileReading));
                        }
                    }
                }
                break;

                case 16:
                {
                    ColorMaskElement aRedMask(nRMask);
                    if (!aRedMask.CalcMaskShift())
                        return false;
                    ColorMaskElement aGreenMask(nGMask);
                    if (!aGreenMask.CalcMaskShift())
                        return false;
                    ColorMaskElement aBlueMask(nBMask);
                    if (!aBlueMask.CalcMaskShift())
                        return false;

                    ColorMask   aMask(aRedMask, aGreenMask, aBlueMask);
                    BitmapColor aColor;

                    for( ; nCount--; nY += nI )
                    {
                        sal_uInt16 * pTmp16 = reinterpret_cast<sal_uInt16*>(aBuf.data());
                        if (rIStm.ReadBytes(pTmp16, nAlignedWidth)
                            != nAlignedWidth)
                        {
                            return false;
                        }

                        Scanline pScanline = rAcc.GetScanline(nY);
                        for( tools::Long nX = 0; nX < nWidth; nX++ )
                        {
                            aMask.GetColorFor16BitLSB( aColor, reinterpret_cast<sal_uInt8*>(pTmp16++) );
                            rAcc.SetPixelOnData(pScanline, nX, SanitizeColor(aColor, bForceToMonoWhileReading));
                        }
                    }
                }
                break;

                case 24:
                {
                    BitmapColor aPixelColor;

                    for( ; nCount--; nY += nI )
                    {
                        sal_uInt8* pTmp = aBuf.data();
                        if (rIStm.ReadBytes(pTmp, nAlignedWidth)
                            != nAlignedWidth)
                        {
                            return false;
                        }

                        Scanline pScanline = rAcc.GetScanline(nY);
                        for( tools::Long nX = 0; nX < nWidth; nX++ )
                        {
                            aPixelColor.SetBlue( *pTmp++ );
                            aPixelColor.SetGreen( *pTmp++ );
                            aPixelColor.SetRed( *pTmp++ );
                            rAcc.SetPixelOnData(pScanline, nX, SanitizeColor(aPixelColor, bForceToMonoWhileReading));
                        }
                    }
                }
                break;

                case 32:
                {
                    ColorMaskElement aRedMask(nRMask);
                    if (!aRedMask.CalcMaskShift())
                        return false;
                    ColorMaskElement aGreenMask(nGMask);
                    if (!aGreenMask.CalcMaskShift())
                        return false;
                    ColorMaskElement aBlueMask(nBMask);
                    if (!aBlueMask.CalcMaskShift())
                        return false;
                    ColorMask aMask(aRedMask, aGreenMask, aBlueMask);

                    BitmapColor aColor;
                    sal_uInt32* pTmp32;

                    if(pAccAlpha)
                    {
                        sal_uInt8 aAlpha;

                        for( ; nCount--; nY += nI )
                        {
                            pTmp32 = reinterpret_cast<sal_uInt32*>(aBuf.data());
                            if (rIStm.ReadBytes(pTmp32, nAlignedWidth)
                                != nAlignedWidth)
                            {
                                return false;
                            }

                            Scanline pScanline = rAcc.GetScanline(nY);
                            Scanline pAlphaScanline = pAccAlpha->GetScanline(nY);
                            for( tools::Long nX = 0; nX < nWidth; nX++ )
                            {
                                aMask.GetColorAndAlphaFor32Bit( aColor, aAlpha, reinterpret_cast<sal_uInt8*>(pTmp32++) );
                                rAcc.SetPixelOnData(pScanline, nX, SanitizeColor(aColor, bForceToMonoWhileReading));
                                pAccAlpha->SetPixelOnData(pAlphaScanline, nX, BitmapColor(sal_uInt8(0xff) - aAlpha));
                                rAlphaUsed |= 0xff != aAlpha;
                            }
                        }
                    }
                    else
                    {
                        for( ; nCount--; nY += nI )
                        {
                            pTmp32 = reinterpret_cast<sal_uInt32*>(aBuf.data());
                            if (rIStm.ReadBytes(pTmp32, nAlignedWidth)
                                != nAlignedWidth)
                            {
                                return false;
                            }

                            Scanline pScanline = rAcc.GetScanline(nY);
                            for( tools::Long nX = 0; nX < nWidth; nX++ )
                            {
                                aMask.GetColorFor32Bit( aColor, reinterpret_cast<sal_uInt8*>(pTmp32++) );
                                rAcc.SetPixelOnData(pScanline, nX, SanitizeColor(aColor, bForceToMonoWhileReading));
                            }
                        }
                    }
                }
            }
        }
    }

    return rIStm.GetError() == ERRCODE_NONE;
}

bool ImplReadDIBBody(SvStream& rIStm, Bitmap& rBmp, AlphaMask* pBmpAlpha, sal_uInt64 nOffset, bool bIsMask, bool bMSOFormat)
{
    DIBV5Header aHeader;
    const sal_uInt64 nStmPos = rIStm.Tell();
    bool bTopDown(false);

    if (!ImplReadDIBInfoHeader(rIStm, aHeader, bTopDown, bMSOFormat))
        return false;

    //BI_BITCOUNT_0 jpeg/png is unsupported
    if (aHeader.nBitCount == 0)
        return false;

    if (aHeader.nWidth <= 0 || aHeader.nHeight <= 0)
        return false;

    // In case ImplReadDIB() didn't call ImplReadDIBFileHeader() before
    // this method, nOffset is 0, that's OK.
    if (nOffset && aHeader.nSize > nOffset)
    {
        // Header size claims to extend into the image data.
        // Looks like an error.
        return false;
    }

    sal_uInt16 nColors(0);
    SvStream* pIStm;
    std::unique_ptr<SvMemoryStream> pMemStm;
    std::vector<sal_uInt8> aData;

    if (aHeader.nBitCount <= 8)
    {
        if(aHeader.nColsUsed)
        {
            nColors = static_cast<sal_uInt16>(aHeader.nColsUsed);
        }
        else
        {
            nColors = ( 1 << aHeader.nBitCount );
        }
    }

    if (ZCOMPRESS == aHeader.nCompression)
    {
        sal_uInt32 nCodedSize(0);
        sal_uInt32  nUncodedSize(0);

        // read coding information
        rIStm.ReadUInt32( nCodedSize ).ReadUInt32( nUncodedSize ).ReadUInt32( aHeader.nCompression );
        if (nCodedSize > rIStm.remainingSize())
           nCodedSize = sal_uInt32(rIStm.remainingSize());

        pMemStm.reset(new SvMemoryStream);
        // There may be bytes left over or the codec might read more than
        // necessary. So to preserve the correctness of the source stream copy
        // the encoded block
        pMemStm->WriteStream(rIStm, nCodedSize);
        pMemStm->Seek(0);

        size_t nSizeInc(4 * pMemStm->remainingSize());
        if (nUncodedSize < nSizeInc)
            nSizeInc = nUncodedSize;

        if (nSizeInc > 0)
        {
            // decode buffer
            ZCodec aCodec;
            aCodec.BeginCompression();
            aData.resize(nSizeInc);
            size_t nDataPos(0);
            while (nUncodedSize > nDataPos)
            {
                assert(aData.size() > nDataPos);
                const size_t nToRead(std::min<size_t>(nUncodedSize - nDataPos, aData.size() - nDataPos));
                assert(nToRead > 0);
                assert(!aData.empty());
                const tools::Long nRead = aCodec.Read(*pMemStm, aData.data() + nDataPos, sal_uInt32(nToRead));
                if (nRead > 0)
                {
                    nDataPos += static_cast<tools::ULong>(nRead);
                    // we haven't read everything yet: resize buffer and continue
                    if (nDataPos < nUncodedSize)
                        aData.resize(aData.size() + nSizeInc);
                }
                else
                {
                    break;
                }
            }
            // truncate the data buffer to actually read size
            aData.resize(nDataPos);
            // set the real uncoded size
            nUncodedSize = sal_uInt32(aData.size());
            aCodec.EndCompression();
        }

        if (aData.empty())
        {
            // add something so we can take address of the first element
            aData.resize(1);
            nUncodedSize = 0;
        }

        // set decoded bytes to memory stream,
        // from which we will read the bitmap data
        pMemStm.reset(new SvMemoryStream);
        pIStm = pMemStm.get();
        assert(!aData.empty());
        pMemStm->SetBuffer(aData.data(), nUncodedSize, nUncodedSize);
        nOffset = 0;
    }
    else
    {
        pIStm = &rIStm;
    }

    // read palette
    BitmapPalette aPalette;
    if (nColors)
    {
        aPalette.SetEntryCount(nColors);
        ImplReadDIBPalette(*pIStm, aPalette, aHeader.nSize != DIBCOREHEADERSIZE);
    }

    if (pIStm->GetError())
        return false;

    if (nOffset)
    {
        pIStm->SeekRel(nOffset - (pIStm->Tell() - nStmPos));
    }

    const sal_Int64 nBitsPerLine (static_cast<sal_Int64>(aHeader.nWidth) * static_cast<sal_Int64>(aHeader.nBitCount));
    if (nBitsPerLine > SAL_MAX_UINT32)
        return false;
    const sal_uInt64 nAlignedWidth(AlignedWidth4Bytes(static_cast<sal_uLong>(nBitsPerLine)));

    switch (aHeader.nCompression)
    {
        case RLE_8:
        {
            if (aHeader.nBitCount != 8)
                return false;
            // (partially) check the image dimensions to avoid potential large bitmap allocation if the input is damaged
            sal_uInt64 nMaxWidth = pIStm->remainingSize();
            nMaxWidth *= 256;   //assume generous compression ratio
            nMaxWidth /= aHeader.nHeight;
            if (nMaxWidth < o3tl::make_unsigned(aHeader.nWidth))
                return false;
            break;
        }
        case RLE_4:
        {
            if (aHeader.nBitCount != 4)
                return false;
            sal_uInt64 nMaxWidth = pIStm->remainingSize();
            nMaxWidth *= 512;   //assume generous compression ratio
            nMaxWidth /= aHeader.nHeight;
            if (nMaxWidth < o3tl::make_unsigned(aHeader.nWidth))
                return false;
            break;
        }
        default:
            // tdf#122958 invalid compression value used
            if (aHeader.nCompression & 0x000F)
            {
                // lets assume that there was an error in the generating application
                // and allow through as COMPRESS_NONE if the bottom byte is 0
                SAL_WARN( "vcl", "bad bmp compression scheme: " << aHeader.nCompression << ", rejecting bmp");
                return false;
            }
            else
                SAL_WARN( "vcl", "bad bmp compression scheme: " << aHeader.nCompression << ", assuming meant to be COMPRESS_NONE");
        [[fallthrough]];
        case BITFIELDS:
        case ZCOMPRESS:
        case COMPRESS_NONE:
        {
            // (partially) check the image dimensions to avoid potential large bitmap allocation if the input is damaged
            sal_uInt64 nMaxWidth = pIStm->remainingSize();
            nMaxWidth /= aHeader.nHeight;
            if (nMaxWidth < nAlignedWidth)
                return false;
            break;
        }
    }

    const Size aSizePixel(aHeader.nWidth, aHeader.nHeight);
    AlphaMask aNewBmpAlpha;
    AlphaScopedWriteAccess pAccAlpha;
    bool bAlphaPossible(pBmpAlpha && aHeader.nBitCount == 32);

    if (bAlphaPossible)
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

    if (bAlphaPossible)
    {
        aNewBmpAlpha = AlphaMask(aSizePixel);
        pAccAlpha = AlphaScopedWriteAccess(aNewBmpAlpha);
    }

    vcl::PixelFormat ePixelFormat(convertToBPP(aHeader.nBitCount));
    const BitmapPalette* pPal = &aPalette;
    //ofz#948 match the surrounding logic of case TransparentType::Bitmap of
    //ReadDIBBitmapEx but do it while reading for performance
    const bool bIsAlpha = (ePixelFormat == vcl::PixelFormat::N8_BPP &&
                          !!aPalette && aPalette.IsGreyPalette8Bit());
    const bool bForceToMonoWhileReading = (bIsMask && !bIsAlpha && ePixelFormat != vcl::PixelFormat::N1_BPP);
    if (bForceToMonoWhileReading)
    {
        pPal = nullptr;
        ePixelFormat = vcl::PixelFormat::N1_BPP;
        SAL_WARN( "vcl", "forcing mask to monochrome");
    }

    Bitmap aNewBmp(aSizePixel, ePixelFormat, pPal);
    BitmapScopedWriteAccess pAcc(aNewBmp);
    if (!pAcc)
        return false;
    if (pAcc->Width() != aHeader.nWidth || pAcc->Height() != aHeader.nHeight)
    {
        return false;
    }

    // read bits
    bool bAlphaUsed(false);
    bool bRet = ImplReadDIBBits(*pIStm, aHeader, *pAcc, aPalette, pAccAlpha.get(), bTopDown, bAlphaUsed, nAlignedWidth, bForceToMonoWhileReading);

    if (bRet && aHeader.nXPelsPerMeter && aHeader.nYPelsPerMeter)
    {
        MapMode aMapMode(
            MapUnit::MapMM,
            Point(),
            Fraction(1000, aHeader.nXPelsPerMeter),
            Fraction(1000, aHeader.nYPelsPerMeter));

        aNewBmp.SetPrefMapMode(aMapMode);
        aNewBmp.SetPrefSize(Size(aHeader.nWidth, aHeader.nHeight));
    }

    pAcc.reset();

    if (bAlphaPossible)
    {
        pAccAlpha.reset();

        if(!bAlphaUsed)
        {
            bAlphaPossible = false;
        }
    }

    if (bRet)
    {
        rBmp = aNewBmp;

        if(bAlphaPossible)
        {
            *pBmpAlpha = aNewBmpAlpha;
        }
    }

    return bRet;
}

bool ImplReadDIBFileHeader( SvStream& rIStm, sal_uLong& rOffset )
{
    bool bRet = false;

    const sal_uInt64 nStreamLength = rIStm.TellEnd();

    sal_uInt16 nTmp16 = 0;
    rIStm.ReadUInt16( nTmp16 );

    if ( ( 0x4D42 == nTmp16 ) || ( 0x4142 == nTmp16 ) )
    {
        sal_uInt32 nTmp32(0);
        if ( 0x4142 == nTmp16 )
        {
            rIStm.SeekRel( 12 );
            rIStm.ReadUInt16( nTmp16 );
            rIStm.SeekRel( 8 );
            rIStm.ReadUInt32( nTmp32 );
            rOffset = nTmp32 - 28;
            bRet = ( 0x4D42 == nTmp16 );
        }
        else // 0x4D42 == nTmp16, 'MB' from BITMAPFILEHEADER
        {
            rIStm.SeekRel( 8 );        // we are on bfSize member of BITMAPFILEHEADER, forward to bfOffBits
            rIStm.ReadUInt32( nTmp32 );            // read bfOffBits
            rOffset = nTmp32 - 14;    // adapt offset by sizeof(BITMAPFILEHEADER)
            bRet = rIStm.GetError() == ERRCODE_NONE;
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

bool ImplWriteDIBPalette( SvStream& rOStm, BitmapReadAccess const & rAcc )
{
    const sal_uInt16    nColors = rAcc.GetPaletteEntryCount();
    const sal_uLong     nPalSize = nColors * 4UL;
    std::unique_ptr<sal_uInt8[]> pEntries(new sal_uInt8[ nPalSize ]);
    sal_uInt8*          pTmpEntry = pEntries.get();

    for( sal_uInt16 i = 0; i < nColors; i++ )
    {
        const BitmapColor& rPalColor = rAcc.GetPaletteColor( i );

        *pTmpEntry++ = rPalColor.GetBlue();
        *pTmpEntry++ = rPalColor.GetGreen();
        *pTmpEntry++ = rPalColor.GetRed();
        *pTmpEntry++ = 0;
    }

    rOStm.WriteBytes( pEntries.get(), nPalSize );

    return rOStm.GetError() == ERRCODE_NONE;
}

bool ImplWriteRLE( SvStream& rOStm, BitmapReadAccess const & rAcc, bool bRLE4 )
{
    const sal_uLong nWidth = rAcc.Width();
    const sal_uLong nHeight = rAcc.Height();
    sal_uLong       nX;
    sal_uLong       nSaveIndex;
    sal_uLong       nCount;
    sal_uLong       nBufCount;
    std::vector<sal_uInt8> aBuf(( nWidth << 1 ) + 2);
    sal_uInt8       cPix;
    sal_uInt8       cLast;
    bool        bFound;

    for ( tools::Long nY = nHeight - 1; nY >= 0; nY-- )
    {
        sal_uInt8* pTmp = aBuf.data();
        nX = nBufCount = 0;
        Scanline pScanline = rAcc.GetScanline( nY );

        while( nX < nWidth )
        {
            nCount = 1;
            cPix = rAcc.GetIndexFromData( pScanline, nX++ );

            while( ( nX < nWidth ) && ( nCount < 255 )
                && ( cPix == rAcc.GetIndexFromData( pScanline, nX ) ) )
            {
                nX++;
                nCount++;
            }

            if ( nCount > 1 )
            {
                *pTmp++ = static_cast<sal_uInt8>(nCount);
                *pTmp++ = ( bRLE4 ? ( ( cPix << 4 ) | cPix ) : cPix );
                nBufCount += 2;
            }
            else
            {
                cLast = cPix;
                nSaveIndex = nX - 1;
                bFound = false;

                while( ( nX < nWidth ) && ( nCount < 256 ) )
                {
                    cPix = rAcc.GetIndexFromData( pScanline, nX );
                    if (cPix == cLast)
                        break;
                    nX++; nCount++;
                    cLast = cPix;
                    bFound = true;
                }

                if ( bFound )
                    nX--;

                if ( nCount > 3 )
                {
                    *pTmp++ = 0;
                    *pTmp++ = static_cast<sal_uInt8>(--nCount);

                    if( bRLE4 )
                    {
                        for ( sal_uLong i = 0; i < nCount; i++, pTmp++ )
                        {
                            *pTmp = rAcc.GetIndexFromData( pScanline, nSaveIndex++ ) << 4;

                            if ( ++i < nCount )
                                *pTmp |= rAcc.GetIndexFromData( pScanline, nSaveIndex++ );
                        }

                        nCount = ( nCount + 1 ) >> 1;
                    }
                    else
                    {
                        for( sal_uLong i = 0; i < nCount; i++ )
                            *pTmp++ = rAcc.GetIndexFromData( pScanline, nSaveIndex++ );
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
                    *pTmp++ = rAcc.GetIndexFromData( pScanline, nSaveIndex ) << (bRLE4 ? 4 : 0);

                    if ( nCount == 3 )
                    {
                        *pTmp++ = 1;
                        *pTmp++ = rAcc.GetIndexFromData( pScanline, ++nSaveIndex ) << ( bRLE4 ? 4 : 0 );
                        nBufCount += 4;
                    }
                    else
                        nBufCount += 2;
                }
            }
        }

        aBuf[ nBufCount++ ] = 0;
        aBuf[ nBufCount++ ] = 0;

        rOStm.WriteBytes(aBuf.data(), nBufCount);
    }

    rOStm.WriteUChar( 0 );
    rOStm.WriteUChar( 1 );

    return rOStm.GetError() == ERRCODE_NONE;
}

bool ImplWriteDIBBits(SvStream& rOStm, BitmapReadAccess const & rAcc, BitmapReadAccess const * pAccAlpha, sal_uLong nCompression, sal_uInt32& rImageSize)
{
    if(!pAccAlpha && BITFIELDS == nCompression)
    {
        const ColorMask&    rMask = rAcc.GetColorMask();
        SVBT32              aVal32;

        UInt32ToSVBT32( rMask.GetRedMask(), aVal32 );
        rOStm.WriteBytes( aVal32, 4UL );

        UInt32ToSVBT32( rMask.GetGreenMask(), aVal32 );
        rOStm.WriteBytes( aVal32, 4UL );

        UInt32ToSVBT32( rMask.GetBlueMask(), aVal32 );
        rOStm.WriteBytes( aVal32, 4UL );

        rImageSize = rOStm.Tell();

        if( rAcc.IsBottomUp() )
            rOStm.WriteBytes(rAcc.GetBuffer(), rAcc.Height() * rAcc.GetScanlineSize());
        else
        {
            for( tools::Long nY = rAcc.Height() - 1, nScanlineSize = rAcc.GetScanlineSize(); nY >= 0; nY-- )
                rOStm.WriteBytes( rAcc.GetScanline(nY), nScanlineSize );
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
        // #i59239# discretize bitcount for aligned width to 1,8,24
        // (other cases are not written below)
        const auto ePixelFormat(pAccAlpha ? vcl::PixelFormat::N32_BPP : convertToBPP(rAcc.GetBitCount()));
        const sal_uLong nAlignedWidth(AlignedWidth4Bytes(rAcc.Width() * sal_Int32(ePixelFormat)));
        bool bNative(false);

        switch(rAcc.GetScanlineFormat())
        {
            case ScanlineFormat::N1BitMsbPal:
            case ScanlineFormat::N8BitPal:
            case ScanlineFormat::N24BitTcBgr:
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
            rOStm.WriteBytes(rAcc.GetBuffer(), nAlignedWidth * rAcc.Height());
        }
        else
        {
            const tools::Long nWidth(rAcc.Width());
            const tools::Long nHeight(rAcc.Height());
            std::vector<sal_uInt8> aBuf(nAlignedWidth);
            switch(ePixelFormat)
            {
                case vcl::PixelFormat::N1_BPP:
                {
                    //valgrind, zero out the trailing unused alignment bytes
                    size_t nUnusedBytes = nAlignedWidth - ((nWidth+7) / 8);
                    memset(aBuf.data() + nAlignedWidth - nUnusedBytes, 0, nUnusedBytes);

                    for( tools::Long nY = nHeight - 1; nY >= 0; nY-- )
                    {
                        sal_uInt8* pTmp = aBuf.data();
                        sal_uInt8 cTmp = 0;
                        Scanline pScanline = rAcc.GetScanline( nY );

                        for( tools::Long nX = 0, nShift = 8; nX < nWidth; nX++ )
                        {
                            if( !nShift )
                            {
                                nShift = 8;
                                *pTmp++ = cTmp;
                                cTmp = 0;
                            }

                            cTmp |= rAcc.GetIndexFromData( pScanline, nX ) << --nShift;
                        }

                        *pTmp = cTmp;
                        rOStm.WriteBytes(aBuf.data(), nAlignedWidth);
                    }
                }
                break;

                case vcl::PixelFormat::N8_BPP:
                {
                    for( tools::Long nY = nHeight - 1; nY >= 0; nY-- )
                    {
                        sal_uInt8* pTmp = aBuf.data();
                        Scanline pScanline = rAcc.GetScanline( nY );

                        for( tools::Long nX = 0; nX < nWidth; nX++ )
                            *pTmp++ = rAcc.GetIndexFromData( pScanline, nX );

                        rOStm.WriteBytes(aBuf.data(), nAlignedWidth);
                    }
                }
                break;

                case vcl::PixelFormat::N24_BPP:
                {
                    //valgrind, zero out the trailing unused alignment bytes
                    size_t nUnusedBytes = nAlignedWidth - nWidth * 3;
                    memset(aBuf.data() + nAlignedWidth - nUnusedBytes, 0, nUnusedBytes);
                }
                [[fallthrough]];
                // #i59239# fallback to 24 bit format, if bitcount is non-default
                default:
                {
                    BitmapColor aPixelColor;
                    const bool bWriteAlpha(ePixelFormat == vcl::PixelFormat::N32_BPP && pAccAlpha);

                    for( tools::Long nY = nHeight - 1; nY >= 0; nY-- )
                    {
                        sal_uInt8* pTmp = aBuf.data();
                        Scanline pScanlineAlpha = bWriteAlpha ? pAccAlpha->GetScanline( nY ) : nullptr;

                        for( tools::Long nX = 0; nX < nWidth; nX++ )
                        {
                            // when alpha is used, this may be non-24bit main bitmap, so use GetColor
                            // instead of GetPixel to ensure RGB value
                            aPixelColor = rAcc.GetColor( nY, nX );

                            *pTmp++ = aPixelColor.GetBlue();
                            *pTmp++ = aPixelColor.GetGreen();
                            *pTmp++ = aPixelColor.GetRed();

                            if(bWriteAlpha)
                            {
                                *pTmp++ = sal_uInt8(0xff) - pAccAlpha->GetIndexFromData( pScanlineAlpha, nX );
                            }
                        }

                        rOStm.WriteBytes(aBuf.data(), nAlignedWidth);
                    }
                }
                break;
            }
        }
    }

    rImageSize = rOStm.Tell() - rImageSize;

    return (!rOStm.GetError());
}

bool ImplWriteDIBBody(const Bitmap& rBitmap, SvStream& rOStm, BitmapReadAccess const & rAcc, BitmapReadAccess const * pAccAlpha, bool bCompressed)
{
    const MapMode aMapPixel(MapUnit::MapPixel);
    DIBV5Header aHeader;
    sal_uInt64 nImageSizePos(0);
    sal_uInt64 nEndPos(0);
    sal_uInt32 nCompression(COMPRESS_NONE);
    bool bRet(false);

    aHeader.nSize = pAccAlpha ? DIBV5HEADERSIZE : DIBINFOHEADERSIZE; // size dependent on CF_DIB type to use
    aHeader.nWidth = rAcc.Width();
    aHeader.nHeight = rAcc.Height();
    aHeader.nPlanes = 1;

    if(!pAccAlpha && isBitfieldCompression(rAcc.GetScanlineFormat()))
    {
        aHeader.nBitCount = 32;
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
        // #i59239# discretize bitcount to 1,8,24 (other cases
        // are not written below)
        const auto ePixelFormat(pAccAlpha ? vcl::PixelFormat::N32_BPP : convertToBPP(rAcc.GetBitCount()));
        aHeader.nBitCount = sal_uInt16(ePixelFormat);
        aHeader.nSizeImage = rAcc.Height() * AlignedWidth4Bytes(rAcc.Width() * aHeader.nBitCount);

        if (bCompressed)
        {
            if (ePixelFormat == vcl::PixelFormat::N8_BPP)
                nCompression = RLE_8;
        }
    }

    if((rOStm.GetCompressMode() & SvStreamCompressFlags::ZBITMAP) && (rOStm.GetVersion() >= SOFFICE_FILEFORMAT_40))
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
        const Size aScale100000(OutputDevice::LogicToLogic(Size(100000, 100000), MapMode(MapUnit::Map100thMM), rBitmap.GetPrefMapMode()));
        const double fBmpWidthM(static_cast<double>(rBitmap.GetPrefSize().Width()) / aScale100000.Width());
        const double fBmpHeightM(static_cast<double>(rBitmap.GetPrefSize().Height()) / aScale100000.Height());

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
        sal_uInt64 nCodedPos(rOStm.Tell());
        sal_uInt64 nLastPos(0);
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
        aCodec.Write(rOStm, static_cast<sal_uInt8 const *>(aMemStm.GetData()), nUncodedSize);
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

bool ImplWriteDIBFileHeader(SvStream& rOStm, BitmapReadAccess const & rAcc)
{
    const sal_uInt32 nPalCount((rAcc.HasPalette() ? rAcc.GetPaletteEntryCount() : isBitfieldCompression(rAcc.GetScanlineFormat()) ? 3UL : 0UL));
    const sal_uInt32 nOffset(14 + DIBINFOHEADERSIZE + nPalCount * 4UL);

    rOStm.WriteUInt16( 0x4D42 ); // 'MB' from BITMAPFILEHEADER
    rOStm.WriteUInt32( nOffset + (rAcc.Height() * rAcc.GetScanlineSize()) );
    rOStm.WriteUInt16( 0 );
    rOStm.WriteUInt16( 0 );
    rOStm.WriteUInt32( nOffset );

    return rOStm.GetError() == ERRCODE_NONE;
}

bool ImplReadDIB(
    Bitmap& rTarget,
    AlphaMask* pTargetAlpha,
    SvStream& rIStm,
    bool bFileHeader,
    bool bIsMask=false,
    bool bMSOFormat=false)
{
    const SvStreamEndian nOldFormat(rIStm.GetEndian());
    const auto nOldPos(rIStm.Tell());
    sal_uLong nOffset(0);
    bool bRet(false);

    rIStm.SetEndian(SvStreamEndian::LITTLE);

    if(bFileHeader)
    {
        if(ImplReadDIBFileHeader(rIStm, nOffset))
        {
            bRet = ImplReadDIBBody(rIStm, rTarget, nOffset >= DIBV5HEADERSIZE ? pTargetAlpha : nullptr, nOffset, bIsMask, bMSOFormat);
        }
    }
    else
    {
        bRet = ImplReadDIBBody(rIStm, rTarget, nullptr, nOffset, bIsMask, bMSOFormat);
    }

    if(!bRet)
    {
        if(!rIStm.GetError()) // Set error and stop processing whole stream due to security reason
        {
            rIStm.SetError(SVSTREAM_GENERALERROR);
        }

        rIStm.Seek(nOldPos);
    }

    rIStm.SetEndian(nOldFormat);

    return bRet;
}

bool ImplWriteDIB(
    const Bitmap& rSource,
    SvStream& rOStm,
    bool bCompressed,
    bool bFileHeader)
{
    const Size aSizePix(rSource.GetSizePixel());
    bool bRet(false);

    if(aSizePix.Width() && aSizePix.Height())
    {
        Bitmap::ScopedReadAccess pAcc(const_cast< Bitmap& >(rSource));
        Bitmap::ScopedReadAccess pAccAlpha;
        const SvStreamEndian nOldFormat(rOStm.GetEndian());
        const sal_uInt64 nOldPos(rOStm.Tell());

        rOStm.SetEndian(SvStreamEndian::LITTLE);

        if (pAcc)
        {
            if(bFileHeader)
            {
                if(ImplWriteDIBFileHeader(rOStm, *pAcc))
                {
                    bRet = ImplWriteDIBBody(rSource, rOStm, *pAcc, pAccAlpha.get(), bCompressed);
                }
            }
            else
            {
                bRet = ImplWriteDIBBody(rSource, rOStm, *pAcc, pAccAlpha.get(), bCompressed);
            }

            pAcc.reset();
        }

        pAccAlpha.reset();

        if(!bRet)
        {
            rOStm.SetError(SVSTREAM_GENERALERROR);
            rOStm.Seek(nOldPos);
        }

        rOStm.SetEndian(nOldFormat);
    }

    return bRet;
}

} // unnamed namespace

bool ReadDIB(
    Bitmap& rTarget,
    SvStream& rIStm,
    bool bFileHeader,
    bool bMSOFormat)
{
    return ImplReadDIB(rTarget, nullptr, rIStm, bFileHeader, false, bMSOFormat);
}

bool ReadDIBBitmapEx(
    BitmapEx& rTarget,
    SvStream& rIStm,
    bool bFileHeader,
    bool bMSOFormat)
{
    Bitmap aBmp;
    bool bRetval(ImplReadDIB(aBmp, nullptr, rIStm, bFileHeader, /*bMask*/false, bMSOFormat) && !rIStm.GetError());

    if(bRetval)
    {
        // base bitmap was read, set as return value and try to read alpha extra-data
        const sal_uInt64 nStmPos(rIStm.Tell());
        sal_uInt32 nMagic1(0);
        sal_uInt32 nMagic2(0);

        rTarget = BitmapEx(aBmp);
        if (rIStm.remainingSize() >= 4)
            rIStm.ReadUInt32( nMagic1 ).ReadUInt32( nMagic2 );
        bRetval = (0x25091962 == nMagic1) && (0xACB20201 == nMagic2) && !rIStm.GetError();

        if(bRetval)
        {
            sal_uInt8 tmp = 0;
            rIStm.ReadUChar( tmp );
            bRetval = !rIStm.GetError();

            if(bRetval)
            {
                switch (tmp)
                {
                case 2: // TransparentType::Bitmap
                    {
                        Bitmap aMask;

                        bRetval = ImplReadDIB(aMask, nullptr, rIStm, true, true);

                        if(bRetval)
                        {
                            if(!aMask.IsEmpty())
                            {
                                // do we have an alpha mask?
                                if (aMask.getPixelFormat() == vcl::PixelFormat::N8_BPP && aMask.HasGreyPalette8Bit())
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
                        break;
                    }
                case 1: // backwards compat for old option TransparentType::Color
                    {
                        Color aTransparentColor;

                        tools::GenericTypeSerializer aSerializer(rIStm);
                        aSerializer.readColor(aTransparentColor);

                        bRetval = rIStm.good();

                        if(bRetval)
                        {
                            rTarget = BitmapEx(aBmp, aTransparentColor);
                        }
                        break;
                    }
                default: break;
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
    AlphaMask& rTargetAlpha,
    SvStream& rIStm)
{
    return ImplReadDIB(rTarget, &rTargetAlpha, rIStm, true);
}

bool ReadRawDIB(
    BitmapEx& rTarget,
    const unsigned char* pBuf,
    const ScanlineFormat nFormat,
    const int nHeight,
    const int nStride)
{
    BitmapScopedWriteAccess pWriteAccess(rTarget.maBitmap.AcquireWriteAccess(), rTarget.maBitmap);
    for (int nRow = 0; nRow < nHeight; ++nRow)
    {
        pWriteAccess->CopyScanline(nRow, pBuf + (nStride * nRow), nFormat, nStride);
    }

    return true;
}

bool WriteDIB(
    const Bitmap& rSource,
    SvStream& rOStm,
    bool bCompressed,
    bool bFileHeader)
{
    return ImplWriteDIB(rSource, rOStm, bCompressed, bFileHeader);
}

bool WriteDIB(
    const BitmapEx& rSource,
    SvStream& rOStm,
    bool bCompressed)
{
    return ImplWriteDIB(rSource.GetBitmap(), rOStm, bCompressed, /*bFileHeader*/true);
}

bool WriteDIBBitmapEx(
    const BitmapEx& rSource,
    SvStream& rOStm)
{
    if(ImplWriteDIB(rSource.GetBitmap(), rOStm, true, true))
    {
        rOStm.WriteUInt32( 0x25091962 );
        rOStm.WriteUInt32( 0xACB20201 );
        rOStm.WriteUChar( rSource.IsAlpha() ? 2 : 0 ); // Used to be TransparentType enum

        if(rSource.IsAlpha())
        {
            return ImplWriteDIB(rSource.maAlphaMask, rOStm, true, true);
        }
    }

    return false;
}

sal_uInt32 getDIBV5HeaderSize()
{
    return DIBV5HEADERSIZE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
