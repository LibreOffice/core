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

#include <memory>
#include <sal/config.h>

#include <vcl/FilterConfigItem.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/animate.hxx>
#include <tools/fract.hxx>
#include "lzwdecom.hxx"
#include "ccidecom.hxx"

namespace {

template< typename T > T BYTESWAP(T nByte) {
    return ( nByte << 7 ) | ( ( nByte & 2 ) << 5 ) | ( ( nByte & 4 ) << 3 ) |
        ( ( nByte & 8 ) << 1 ) | ( ( nByte & 16 ) >> 1 ) |
        ( ( nByte & 32 ) >> 3 ) | ( ( nByte & 64 ) >> 5 ) |
        ( ( nByte & 128 ) >> 7 );
}

}

//============================ TIFFReader ==================================

class TIFFReader
{

private:

    bool                bStatus;                    // Whether until now no error occurred
    Animation               aAnimation;

    SvStream*               pTIFF;                      // the TIFF file that should be read
    Bitmap                  aBitmap;
    Bitmap::ScopedWriteAccess xAcc;
    sal_uInt16              nDstBitsPerPixel;
    int                     nLargestPixelIndex;
    std::unique_ptr<AlphaMask>
                            pAlphaMask;
    AlphaMask::ScopedWriteAccess xMaskAcc;

    sal_uInt64              nOrigPos;                   // start position in pTIFF
    sal_uInt64              nEndOfFile;                 // end of file position in pTIFF


    sal_uInt16              nDataType;
    // Data taken from the TIFF tags:
    bool                    bByteSwap;                  // sal_True if bits 0..7 -> 7..0 should get converted ( FILLORDER = 2 );

    sal_uInt32              nNewSubFile;
    sal_uInt32              nSubFile;
    sal_Int32               nImageWidth;                // picture width in pixels
    sal_Int32               nImageLength;               // picture height in pixels
    sal_uInt32              nBitsPerSample;             // bits per pixel per layer
    sal_uInt32              nCompression;               // kind of compression
    sal_uInt32              nPhotometricInterpretation;
    sal_uInt32              nThresholding;
    sal_uInt32              nCellWidth;
    sal_uInt32              nCellLength;
    sal_uInt32              nFillOrder;
    std::vector<sal_uInt64> aStripOffsets;              // field of offsets to the Bitmap-Data-"Strips"
    sal_uInt32              nOrientation;
    sal_uInt32              nSamplesPerPixel;           // number of layers
    sal_uInt32              nRowsPerStrip;              // if it's not compressed: number of rows per Strip
    std::vector<sal_uInt32> aStripByteCounts;           // if compressed (in a certain way): size of the strips
    sal_uInt32              nMinSampleValue;
    sal_uInt32              nMaxSampleValue;
    double                  fXResolution;               // X-resolution or 0.0
    double                  fYResolution;               // Y-resolution or 0.0
    sal_uInt32              nPlanarConfiguration;
    sal_uInt32              nGroup3Options;
    sal_uInt32              nGroup4Options;
    sal_uInt32              nResolutionUnit;            // unit of fX/YResolution: 1=unknown, 2(default)=inch, 3=cm
    sal_uInt32              nPredictor;
    std::vector<sal_uInt32> aColorMap;                  // color palette
    sal_uInt32              nNumColors;                 // number of colors within the color palette

    sal_uInt32              nPlanes;                    // number of layers within the Tiff file
    sal_uInt32              nStripsPerPlane;            // number of Strips per layer
    sal_uInt32              nBytesPerRow;               // Bytes per line per Layer in the Tiff file ( uncompressed )
    std::vector<sal_uInt8>  aMap[4];                    // temporary Scanline


    sal_uInt32 DataTypeSize();
    sal_uInt32 ReadIntData();
    double  ReadDoubleData();

    void    ReadHeader();
    void    ReadTagData( sal_uInt16 nTagType, sal_uInt32 nDataLen );

    bool    ReadMap();
        // reads/decompresses the bitmap data and fills aMap

    sal_uInt32 GetBits(const sal_uInt8 * pSrc, sal_uInt32 nBitsPos, sal_uInt32 nBitsCount);
        // fetches BitsCount bits from pSrc[..] at the position nBitsPos

    void    MakePalCol();
        // Create the bitmap from the temporary bitmap aMap
        // and partly deletes aMap while doing this.

    bool    ConvertScanline(sal_Int32 nY);
        // converts a Scanline to the Windows-BMP format

    bool HasAlphaChannel() const;

    void SetPixelIndex(BitmapWriteAccess *pAcc, long nY, long nX, sal_uInt8 cIndex);

public:

    TIFFReader()
        : bStatus(false)
        , pTIFF(nullptr)
        , nDstBitsPerPixel(0)
        , nLargestPixelIndex(-1)
        , pAlphaMask(nullptr)
        , nOrigPos(0)
        , nEndOfFile(0)
        , nDataType(0)
        , bByteSwap(false)
        , nNewSubFile(0)
        , nSubFile(0)
        , nImageWidth(0)
        , nImageLength(0)
        , nBitsPerSample(1)
        , nCompression(1)
        , nPhotometricInterpretation(0)
        , nThresholding(1)
        , nCellWidth(1)
        , nCellLength(1)
        , nFillOrder(1)
        , nOrientation(1)
        , nSamplesPerPixel(1)
        , nRowsPerStrip(0xffffffff)
        , nMinSampleValue(0)
        , nMaxSampleValue(0)
        , fXResolution(0.0)
        , fYResolution(0.0)
        , nPlanarConfiguration(1)
        , nGroup3Options(0)
        , nGroup4Options(0)
        , nResolutionUnit(2)
        , nPredictor(0)
        , nNumColors(0)
        , nPlanes(0)
        , nStripsPerPlane(0)
        , nBytesPerRow(0)
    {
    }

    sal_uInt32 GetRowsPerStrip() const
    {
        //Rows Per Strip:
        //
        //(TIFF format only) The number of rows of pixels per strip to use for
        //encoding the TIFF image. A value greater than zero specifies the
        //number of rows per strip. A value of 0 sets the rows per strip equal
        //to the image length, resulting in a single strip. A value of -1 (the
        //default) sets the rows per strip equal to infinity, resulting in a
        //single strip.
        return nRowsPerStrip == 0 ? nImageLength : nRowsPerStrip;
    }

    bool ReadTIFF( SvStream & rTIFF, Graphic & rGraphic );
};

//=================== Methods of TIFFReader ==============================

sal_uInt32 TIFFReader::DataTypeSize()
{
    sal_uInt32 nSize;
    switch ( nDataType )
    {
        case 1 :            // BYTE
        case 2 :            // ASCII
        case 6 :            // SIGNED Byte
        case 7 :            // UNDEFINED
            nSize = 1;
            break;
        case 3 :            // UINT16
        case 8 :            // INT16
            nSize = 2;
            break;
        case 4 :            // UINT32
        case 9 :            // INT32
        case 11 :           // FLOAT
            nSize = 4;
            break;
        case 5 :            // RATIONAL
        case 10 :           // SIGNED RATIONAL
        case 12 :           // DOUBLE
            nSize = 8;
            break;
        default:
            pTIFF->SetError(SVSTREAM_FILEFORMAT_ERROR);
            nSize=1;
    }
    return nSize;
}

sal_uInt32 TIFFReader::ReadIntData()
{
    double  nDOUBLE(0.0);
    float   nFLOAT(0);
    sal_uInt32  nUINT32a(0), nUINT32b(0);
    sal_Int32   nINT32(0);
    sal_uInt16  nUINT16(0);
    sal_Int16   nINT16(0);
    sal_uInt8   nBYTE(0);
    char    nCHAR(0);

    switch( nDataType )
    {
        case 0 :    //??
        case 1 :
        case 2 :
        case 7 :
            pTIFF->ReadUChar( nBYTE );
            nUINT32a = nBYTE;
        break;
        case 3 :
             pTIFF->ReadUInt16( nUINT16 );
             nUINT32a = nUINT16;
        break;
        case 9 :
        case 4 :
            pTIFF->ReadUInt32( nUINT32a );
        break;
        case  5 :
            pTIFF->ReadUInt32( nUINT32a ).ReadUInt32( nUINT32b );
            if ( nUINT32b != 0 )
                nUINT32a /= nUINT32b;
        break;
        case 6 :
            pTIFF->ReadChar( nCHAR );
            nUINT32a = (sal_Int32)nCHAR;
        break;
        case 8 :
            pTIFF->ReadInt16( nINT16 );
            nUINT32a = (sal_Int32)nINT16;
        break;
        case 10 :
            pTIFF->ReadUInt32( nUINT32a ).ReadInt32( nINT32 );
            if ( nINT32 != 0 )
                nUINT32a /= nINT32;
        break;
        case 11 :
            pTIFF->ReadFloat( nFLOAT );
            nUINT32a = (sal_Int32)nFLOAT;
        break;
        case 12 :
            pTIFF->ReadDouble( nDOUBLE );
            nUINT32a = (sal_Int32)nDOUBLE;
        break;
        default:
            pTIFF->ReadUInt32( nUINT32a );
        break;
    }
    return nUINT32a;
}

double TIFFReader::ReadDoubleData()
{
    double  nd;

    if ( nDataType == 5 )
    {
        sal_uInt32 nulong(0);
        pTIFF->ReadUInt32( nulong );
        nd = (double)nulong;
        nulong = 0;
        pTIFF->ReadUInt32( nulong );
        if ( nulong != 0 )
            nd /= (double)nulong;
    }
    else
        nd = (double)ReadIntData();
    return nd;
}

void TIFFReader::ReadTagData( sal_uInt16 nTagType, sal_uInt32 nDataLen)
{
    if ( !bStatus )
        return;

    switch ( nTagType )
    {
        case 0x00fe:   // New Sub File
            nNewSubFile = ReadIntData();
            SAL_INFO("filter.tiff","NewSubFile: " << nNewSubFile);
            break;

        case 0x00ff:   // Sub File
            nSubFile = ReadIntData();
            SAL_INFO("filter.tiff","SubFile: " << nSubFile);
            break;

        case 0x0100:   // Image Width
            nImageWidth = ReadIntData();
            SAL_INFO("filter.tiff","ImageWidth: " << nImageWidth);
            break;

        case 0x0101:   // Image Length
            nImageLength = ReadIntData();
            SAL_INFO("filter.tiff","ImageLength: " << nImageLength);
            break;

        case 0x0102:   // Bits Per Sample
            nBitsPerSample = ReadIntData();
            SAL_INFO("filter.tiff","BitsPerSample: " << nBitsPerSample);
            if ( nBitsPerSample >= 32 ) // 32 bit and larger samples are not supported
                bStatus = false;
            break;

        case 0x0103:   // Compression
            nCompression = ReadIntData();
            SAL_INFO("filter.tiff","Compression: " << nCompression);
            break;

        case 0x0106:   // Photometric Interpretation
            nPhotometricInterpretation = ReadIntData();
            SAL_INFO("filter.tiff","PhotometricInterpretation: " << nPhotometricInterpretation);
            break;

        case 0x0107:   // Thresholding
            nThresholding = ReadIntData();
            SAL_INFO("filter.tiff","Thresholding: " << nThresholding);
            break;

        case 0x0108:   // Cell Width
            nCellWidth = ReadIntData();
            break;

        case 0x0109:   // Cell Length
            nCellLength = ReadIntData();
            break;

        case 0x010a:   // Fill Order
            nFillOrder = ReadIntData();
            SAL_INFO("filter.tiff","FillOrder: " << nFillOrder);
            break;

        case 0x0111: { // Strip Offset(s)
            size_t nOldNumSO = aStripOffsets.size();
            nDataLen += nOldNumSO;
            size_t const nMaxAllocAllowed = SAL_MAX_UINT32 / sizeof(sal_uInt32);
            size_t nMaxRecordsAvailable = pTIFF->remainingSize() / DataTypeSize();
            if (nDataLen > nOldNumSO && nDataLen < nMaxAllocAllowed &&
                (nDataLen - nOldNumSO) <= nMaxRecordsAvailable)
            {
                try
                {
                    aStripOffsets.resize(nDataLen);
                    for (size_t i = 0; i < nOldNumSO; ++i)
                        aStripOffsets[i] += nOrigPos;
                    for (size_t i = nOldNumSO; i < aStripOffsets.size(); ++i)
                        aStripOffsets[i] = ReadIntData() + nOrigPos;
                }
                catch (const std::bad_alloc &)
                {
                    aStripOffsets.clear();
                }
            }
            SAL_INFO("filter.tiff","StripOffsets (Number:) " << nDataLen);
            break;
        }
        case 0x0112:   // Orientation
            nOrientation = ReadIntData();
            SAL_INFO("filter.tiff","Orientation: " << nOrientation);
            break;

        case 0x0115:   // Samples Per Pixel
            nSamplesPerPixel = ReadIntData();
            SAL_INFO("filter.tiff","SamplesPerPixel: " << nSamplesPerPixel);
            break;

        case 0x0116:   // Rows Per Strip
            nRowsPerStrip = ReadIntData();
            SAL_INFO("filter.tiff","RowsPerStrip: " << nRowsPerStrip);
            break;

        case 0x0117: { // Strip Byte Counts
            size_t nOldNumSBC = aStripByteCounts.size();
            nDataLen += nOldNumSBC;
            size_t const nMaxAllocAllowed = SAL_MAX_UINT32 / sizeof(sal_uInt32);
            size_t nMaxRecordsAvailable = pTIFF->remainingSize() / DataTypeSize();
            if (nDataLen > nOldNumSBC && nDataLen < nMaxAllocAllowed &&
                (nDataLen - nOldNumSBC) <= nMaxRecordsAvailable)
            {
                try
                {
                    aStripByteCounts.resize(nDataLen);
                    for (size_t i = nOldNumSBC; i < aStripByteCounts.size(); ++i)
                        aStripByteCounts[i] = ReadIntData();
                }
                catch (const std::bad_alloc &)
                {
                    aStripByteCounts.clear();
                }
            }
            SAL_INFO("filter.tiff","StripByteCounts (Number:) " << nDataLen);
            break;
        }
        case 0x0118:   // Min Sample Value
            nMinSampleValue = ReadIntData();
            SAL_INFO("filter.tiff","MinSampleValue: " << nMinSampleValue);
            break;

        case 0x0119:   // Max Sample Value
            nMaxSampleValue = ReadIntData();
            SAL_INFO("filter.tiff","MaxSampleValue: " << nMaxSampleValue);
            break;

        case 0x011a:   // X Resolution
            fXResolution = ReadDoubleData();
            break;

        case 0x011b:   // Y Resolution
            fYResolution = ReadDoubleData();
            break;

        case 0x011c:   // Planar Configuration
            nPlanarConfiguration = ReadIntData();
            SAL_INFO("filter.tiff","PlanarConfiguration: " << nPlanarConfiguration);
            break;

        case 0x0124:   // Group 3 Options
            nGroup3Options = ReadIntData();
            SAL_INFO("filter.tiff","Group3Options: " << nGroup3Options);
            break;

        case 0x0125:   // Group 4 Options
            nGroup4Options = ReadIntData();
            SAL_INFO("filter.tiff","Group4Options: " << nGroup4Options);
            break;

        case 0x0128:   // Resolution Unit
            nResolutionUnit = ReadIntData();
            break;

        case 0x013d:   // Predictor
            nPredictor = ReadIntData();
            SAL_INFO("filter.tiff","Predictor: " << nPredictor);
            break;

        case 0x0140: { // Color Map
            sal_uInt16 nVal;
            nNumColors = ((sal_uInt32)1 << nBitsPerSample);
            if ( nDataType == 3 && nNumColors <= 256)
            {
                aColorMap.resize(256);
                for (sal_uInt32 i = 0; i < nNumColors; ++i)
                    aColorMap[i] = 0;
                for (sal_uInt32 i = 0; i < nNumColors; ++i)
                {
                    pTIFF->ReadUInt16( nVal );
                    aColorMap[i] |= ( ( (sal_uInt32)nVal ) << 8 ) & 0x00ff0000;
                }
                for (sal_uInt32 i = 0; i < nNumColors; ++i)
                {
                    pTIFF->ReadUInt16( nVal );
                    aColorMap[i] |= ( (sal_uInt32)nVal ) & 0x0000ff00;
                }
                for (sal_uInt32 i = 0; i < nNumColors; ++i)
                {
                    pTIFF->ReadUInt16( nVal );
                    aColorMap[i] |= ( ( (sal_uInt32)nVal ) >> 8 ) & 0x000000ff;
                }
            }
            else
                bStatus = false;
            SAL_INFO("filter.tiff","ColorMap (number of colors): " << nNumColors);
            break;
        }

        case 0x0153: { // SampleFormat
            sal_uInt32 nSampleFormat = ReadIntData();
            if ( nSampleFormat == 3 ) // IEEE floating point samples are not supported yet
                bStatus = false;
            break;
        }
    }

    if ( pTIFF->GetError() )
        bStatus = false;
}


bool TIFFReader::ReadMap()
{
    if ( nCompression == 1 || nCompression == 32771 )
    {
        sal_uInt32 nStripBytesPerRow;

        if ( nCompression == 1 )
            nStripBytesPerRow = nBytesPerRow;
        else
            nStripBytesPerRow = ( nBytesPerRow + 1 ) & 0xfffffffe;
        for (sal_Int32 ny = 0; ny < nImageLength; ++ny)
        {
            for (sal_uInt32 np = 0; np < nPlanes; ++np)
            {
                if (np >= SAL_N_ELEMENTS(aMap))
                    return false;
                sal_uInt32 nStrip = ny / GetRowsPerStrip() + np * nStripsPerPlane;
                if ( nStrip >= aStripOffsets.size())
                    return false;
                pTIFF->Seek( aStripOffsets[ nStrip ] + ( ny % GetRowsPerStrip() ) * nStripBytesPerRow );
                pTIFF->ReadBytes(aMap[np].data(), nBytesPerRow);
                if (!pTIFF->good())
                    return false;
            }
            if ( !ConvertScanline( ny ) )
                return false;
        }
    }
    else if ( nCompression == 2 || nCompression == 3 || nCompression == 4 )
    {
        sal_uInt32 nOptions;
        if ( nCompression == 2 )
        {
            nOptions = CCI_OPTION_BYTEALIGNROW;
        }
        else if ( nCompression == 3 )
        {
            nOptions = CCI_OPTION_EOL;
            if ( nGroup3Options & 0x00000001 )
                nOptions |= CCI_OPTION_2D;
            if ( nGroup3Options & 0x00000004 )
                nOptions |= CCI_OPTION_BYTEALIGNEOL;
            if ( nGroup3Options & 0xfffffffa )
                return false;
        }
        else
        {   // nCompression==4
            nOptions = CCI_OPTION_2D;
            if ( nGroup4Options & 0xffffffff )
                return false;
        }
        if ( nFillOrder == 2 )
        {
            nOptions |= CCI_OPTION_INVERSEBITORDER;
            bByteSwap = false;
        }
        sal_uInt32 nStrip = 0;
        if (nStrip >= aStripOffsets.size())
            return false;
        sal_uInt64 nOffset = aStripOffsets[nStrip];
        if (nOffset > nEndOfFile)
            return false;
        pTIFF->Seek(aStripOffsets[nStrip]);

        CCIDecompressor aCCIDecom( nOptions, nImageWidth );

        aCCIDecom.StartDecompression( *pTIFF );

        bool bDifferentToPrev;
        for (sal_Int32 ny = 0; ny < nImageLength; ++ny)
        {
            bDifferentToPrev = ny == 0;
            for (sal_uInt32 np = 0; np < nPlanes; ++np)
            {
                if ( ny / GetRowsPerStrip() + np * nStripsPerPlane > nStrip )
                {
                    nStrip=ny/GetRowsPerStrip()+np*nStripsPerPlane;
                    if (nStrip >= aStripOffsets.size())
                        return false;
                    nOffset = aStripOffsets[nStrip];
                    if (nOffset > nEndOfFile)
                        return false;
                    pTIFF->Seek(nOffset);
                    aCCIDecom.StartDecompression( *pTIFF );
                }
                if (np >= SAL_N_ELEMENTS(aMap))
                    return false;
                DecompressStatus aResult = aCCIDecom.DecompressScanline(aMap[np].data(), nImageWidth * nBitsPerSample * nSamplesPerPixel / nPlanes, np + 1 == nPlanes);
                if (!aResult.m_bSuccess)
                    return false;
                bDifferentToPrev |= !aResult.m_bBufferUnchanged;
                if ( pTIFF->GetError() )
                    return false;
            }
            if (!bDifferentToPrev)
            {
                //if the buffer for this line didn't change, then just copy the
                //previous scanline instead of painfully decoding and setting
                //each pixel one by one again
                Scanline pScanline = xAcc->GetScanline(ny);
                Scanline pPrevline = xAcc->GetScanline(ny-1);
                memcpy(pScanline, pPrevline, xAcc->GetScanlineSize());
            }
            else
            {
                if (!ConvertScanline(ny))
                    return false;
            }
        }
    }
    else if ( nCompression == 5 )
    {
        LZWDecompressor aLZWDecom;
        sal_uInt32 nStrip(0);
        if (nStrip >= aStripOffsets.size())
            return false;
        pTIFF->Seek(aStripOffsets[nStrip]);
        aLZWDecom.StartDecompression(*pTIFF);
        for (sal_Int32 ny = 0; ny < nImageLength; ++ny)
        {
            for (sal_uInt32 np = 0; np < nPlanes; ++np)
            {
                if ( ny / GetRowsPerStrip() + np * nStripsPerPlane > nStrip )
                {
                    nStrip = ny / GetRowsPerStrip() + np * nStripsPerPlane;
                    if (nStrip >= aStripOffsets.size())
                        return false;
                    pTIFF->Seek(aStripOffsets[nStrip]);
                    aLZWDecom.StartDecompression(*pTIFF);
                }
                if (np >= SAL_N_ELEMENTS(aMap))
                    return false;
                if ( ( aLZWDecom.Decompress(aMap[np].data(), nBytesPerRow) != nBytesPerRow ) || pTIFF->GetError() )
                    return false;
            }
            if ( !ConvertScanline( ny ) )
                return false;
        }
    }
    else if ( nCompression == 32773 )
    {
        sal_uInt32 nStrip(0);
        if (nStrip >= aStripOffsets.size())
            return false;
        pTIFF->Seek(aStripOffsets[nStrip]);
        for (sal_Int32 ny = 0; ny < nImageLength; ++ny)
        {
            for (sal_uInt32 np = 0; np < nPlanes; ++np)
            {
                if ( ny / GetRowsPerStrip() + np * nStripsPerPlane > nStrip )
                {
                    nStrip=ny/GetRowsPerStrip()+np*nStripsPerPlane;
                    if (nStrip >= aStripOffsets.size())
                        return false;
                    pTIFF->Seek(aStripOffsets[nStrip]);
                }
                sal_uInt32 nRowBytesLeft = nBytesPerRow;
                if (np >= SAL_N_ELEMENTS(aMap))
                    return false;
                sal_uInt8* pdst = aMap[np].data();
                do
                {
                    sal_uInt8 nRecHeader(0);
                    pTIFF->ReadUChar(nRecHeader);
                    sal_uInt32 nRecCount;
                    if ((nRecHeader&0x80)==0)
                    {
                        nRecCount=0x00000001 + ((sal_uInt32)nRecHeader);
                        if ( nRecCount > nRowBytesLeft )
                            return false;
                        pTIFF->ReadBytes(pdst, nRecCount);
                        if (!pTIFF->good())
                            return false;
                        pdst+=nRecCount;
                        nRowBytesLeft-=nRecCount;
                    }
                    else if ( nRecHeader != 0x80 )
                    {
                        nRecCount = 0x000000101 - ((sal_uInt32)nRecHeader);
                        if ( nRecCount > nRowBytesLeft )
                        {
                            nRecCount = nRowBytesLeft;
                        }
                        sal_uInt8 nRecData(0);
                        pTIFF->ReadUChar( nRecData );
                        for (sal_uInt32 i = 0; i < nRecCount; ++i)
                            *(pdst++) = nRecData;
                        nRowBytesLeft -= nRecCount;
                    }
                } while ( nRowBytesLeft != 0 );
                if ( pTIFF->GetError() )
                    return false;
            }
            if ( !ConvertScanline( ny ) )
                return false;
        }
    }
    else
        return false;
    return true;
}

sal_uInt32 TIFFReader::GetBits( const sal_uInt8 * pSrc, sal_uInt32 nBitsPos, sal_uInt32 nBitsCount)
{
    sal_uInt32 nRes;
    if ( bByteSwap )
    {
        pSrc += ( nBitsPos >> 3 );
        nBitsPos &= 7;
        sal_uInt8 nDat = *pSrc;
        nRes = (sal_uInt32)( BYTESWAP( nDat ) & ( 0xff >> nBitsPos ) );

        if ( nBitsCount <= 8 - nBitsPos )
        {
            nRes >>= ( 8 - nBitsPos - nBitsCount );
        }
        else
        {
            pSrc++;
            nBitsCount -= 8 - nBitsPos;
            while ( nBitsCount >= 8 )
            {
                nDat = *(pSrc++);
                nRes = ( nRes << 8 ) | ((sal_uInt32)BYTESWAP( nDat ) );
                nBitsCount -= 8;
            }
            if ( nBitsCount > 0 )
            {
                nDat = *pSrc;
                nRes = ( nRes << nBitsCount ) | (((sal_uInt32)BYTESWAP(nDat))>>(8-nBitsCount));
            }
        }
    }
    else
    {
        pSrc += ( nBitsPos >> 3 );
        nBitsPos &= 7;
        nRes = (sal_uInt32)((*pSrc)&(0xff>>nBitsPos));
        if ( nBitsCount <= 8 - nBitsPos )
        {
            nRes >>= ( 8 - nBitsPos - nBitsCount );
        }
        else
        {
            pSrc++;
            nBitsCount -= 8 - nBitsPos;
            while ( nBitsCount >= 8 )
            {
                nRes = ( nRes << 8 ) | ((sal_uInt32)*(pSrc++));
                nBitsCount -= 8;
            }
            if ( nBitsCount > 0 )
                nRes = ( nRes << nBitsCount ) | (((sal_uInt32)*pSrc)>>(8-nBitsCount));
        }
    }
    return nRes;
}

void TIFFReader::SetPixelIndex(BitmapWriteAccess *pWriteAcc, long nY, long nX, sal_uInt8 cIndex)
{
    pWriteAcc->SetPixelIndex(nY, nX, cIndex);
    nLargestPixelIndex = std::max<int>(nLargestPixelIndex, cIndex);
}

bool TIFFReader::ConvertScanline(sal_Int32 nY)
{
    sal_uInt32  nRed, nGreen, nBlue, ns, nVal;
    sal_uInt8   nByteVal;

    if ( nDstBitsPerPixel == 24 )
    {
        if ( nBitsPerSample == 8 && nSamplesPerPixel >= 3 &&
             nPlanes == 1 && nPhotometricInterpretation == 2 )
        {
            sal_uInt8*  pt = aMap[0].data();

            // are the values being saved as difference?
            if ( 2 == nPredictor )
            {
                sal_uInt8  nLRed = 0;
                sal_uInt8  nLGreen = 0;
                sal_uInt8  nLBlue = 0;
                sal_uInt8  nLAlpha = 0;
                for (sal_Int32 nx = 0; nx < nImageWidth; nx++, pt += nSamplesPerPixel)
                {
                    nLRed = nLRed + pt[ 0 ];
                    nLGreen = nLGreen + pt[ 1 ];
                    nLBlue = nLBlue + pt[ 2 ];
                    xAcc->SetPixel( nY, nx, Color( nLRed, nLGreen, nLBlue ) );
                    if (nSamplesPerPixel >= 4 && xMaskAcc)
                    {
                        nLAlpha = nLAlpha + pt[ 3 ];
                        xMaskAcc->SetPixel( nY, nx, BitmapColor(~nLAlpha) );
                    }
                }
            }
            else
            {
                for (sal_Int32 nx = 0; nx < nImageWidth; nx++, pt += nSamplesPerPixel)
                {
                    xAcc->SetPixel( nY, nx, Color( pt[0], pt[1], pt[2] ) );
                    if (nSamplesPerPixel >= 4 && xMaskAcc)
                    {
                        sal_uInt8 nAlpha = pt[3];
                        xMaskAcc->SetPixel( nY, nx, BitmapColor(~nAlpha) );
                    }
                }
            }
        }
        else if ( nPhotometricInterpretation == 2 && nSamplesPerPixel >= 3 )
        {
            if ( nMaxSampleValue > nMinSampleValue )
            {
                sal_uInt32 nMinMax = nMinSampleValue * 255 / ( nMaxSampleValue - nMinSampleValue );
                for (sal_Int32 nx = 0; nx < nImageWidth; ++nx)
                {
                    if ( nPlanes < 3 )
                    {
                        nRed = GetBits( aMap[0].data(), ( nx * nSamplesPerPixel + 0 ) * nBitsPerSample, nBitsPerSample );
                        nGreen = GetBits( aMap[1].data(), ( nx * nSamplesPerPixel + 1 ) * nBitsPerSample, nBitsPerSample );
                        nBlue = GetBits( aMap[2].data(), ( nx * nSamplesPerPixel + 2 ) * nBitsPerSample, nBitsPerSample );
                    }
                    else
                    {
                        nRed = GetBits( aMap[0].data(), nx * nBitsPerSample, nBitsPerSample );
                        nGreen = GetBits( aMap[1].data(), nx * nBitsPerSample, nBitsPerSample );
                        nBlue = GetBits( aMap[2].data(), nx * nBitsPerSample, nBitsPerSample );
                    }
                    xAcc->SetPixel( nY, nx, Color( (sal_uInt8)( nRed - nMinMax ), (sal_uInt8)( nGreen - nMinMax ), (sal_uInt8)(nBlue - nMinMax) ) );
                }
            }
        }
        else if ( nPhotometricInterpretation == 5 && nSamplesPerPixel == 3 )
        {
            if ( nMaxSampleValue > nMinSampleValue )
            {
                sal_uInt32 nMinMax =  nMinSampleValue * 255 / ( nMaxSampleValue - nMinSampleValue );
                for (sal_Int32 nx = 0; nx < nImageWidth; ++nx)
                {
                    if ( nPlanes < 3 )
                    {
                        nRed = GetBits( aMap[0].data(), ( nx * nSamplesPerPixel + 0 ) * nBitsPerSample, nBitsPerSample );
                        nGreen = GetBits( aMap[0].data(), ( nx * nSamplesPerPixel + 1 ) * nBitsPerSample, nBitsPerSample );
                        nBlue = GetBits( aMap[0].data(), ( nx * nSamplesPerPixel + 2 ) * nBitsPerSample, nBitsPerSample );
                    }
                    else
                    {
                        nRed = GetBits( aMap[0].data(), nx * nBitsPerSample, nBitsPerSample );
                        nGreen = GetBits( aMap[1].data(), nx * nBitsPerSample, nBitsPerSample );
                        nBlue = GetBits( aMap[2].data(), nx * nBitsPerSample, nBitsPerSample );
                    }
                    nRed = 255 - (sal_uInt8)( nRed - nMinMax );
                    nGreen = 255 - (sal_uInt8)( nGreen - nMinMax );
                    nBlue = 255 - (sal_uInt8)( nBlue - nMinMax );
                    xAcc->SetPixel( nY, nx, Color( (sal_uInt8) nRed, (sal_uInt8) nGreen, (sal_uInt8) nBlue ) );
                }
            }
        }
        else if( nPhotometricInterpretation == 5 && nSamplesPerPixel == 4 )
        {
            if ( nMaxSampleValue > nMinSampleValue )
            {
                sal_uInt8   nSamp[ 4 ];
                sal_uInt8   nSampLast[ 4 ] = { 0, 0, 0, 0 };

                for(sal_Int32 nx = 0; nx < nImageWidth; ++nx)
                {
                    // are the values being saved as difference?
                    if( 2 == nPredictor )
                    {
                        for( ns = 0; ns < 4; ns++ )
                        {
                            if( nPlanes < 3 )
                                nSampLast[ ns ] = nSampLast[ ns ] + (sal_uInt8) GetBits( aMap[0].data(), ( nx * nSamplesPerPixel + ns ) * nBitsPerSample, nBitsPerSample );
                            else
                                nSampLast[ ns ] = nSampLast[ ns ] + (sal_uInt8) GetBits( aMap[ns].data(), nx * nBitsPerSample, nBitsPerSample );
                            nSamp[ ns ] = nSampLast[ ns ];
                        }
                    }
                    else
                    {
                        for( ns = 0; ns < 4; ns++ )
                        {
                            if( nPlanes < 3 )
                                nSamp[ ns ] = (sal_uInt8) GetBits( aMap[0].data(), ( nx * nSamplesPerPixel + ns ) * nBitsPerSample, nBitsPerSample );
                            else
                                nSamp[ ns ]= (sal_uInt8) GetBits( aMap[ns].data(), nx * nBitsPerSample, nBitsPerSample );
                        }
                    }
                    const long nBlack = nSamp[ 3 ];
                    nRed = (sal_uInt8) std::max( 0L, 255L - ( ( (sal_Int32) nSamp[ 0 ] + nBlack - ( ( (sal_Int32) nMinSampleValue ) << 1 ) ) *
                                255L/(sal_Int32)(nMaxSampleValue-nMinSampleValue) ) );
                    nGreen = (sal_uInt8) std::max( 0L, 255L - ( ( (sal_Int32) nSamp[ 1 ] + nBlack - ( ( (sal_Int32) nMinSampleValue ) << 1 ) ) *
                                255L/(sal_Int32)(nMaxSampleValue-nMinSampleValue) ) );
                    nBlue = (sal_uInt8) std::max( 0L, 255L - ( ( (sal_Int32) nSamp[ 2 ] + nBlack - ( ( (sal_Int32) nMinSampleValue ) << 1 ) ) *
                                255L/(sal_Int32)(nMaxSampleValue-nMinSampleValue) ) );
                    xAcc->SetPixel( nY, nx, Color ( (sal_uInt8)nRed, (sal_uInt8)nGreen, (sal_uInt8)nBlue ) );
                }
            }
        }
    }
    else if ( nSamplesPerPixel == 1 && ( nPhotometricInterpretation <= 1 || nPhotometricInterpretation == 3 ) )
    {
        if ( nMaxSampleValue > nMinSampleValue )
        {
            sal_uInt32 nMinMax = ( ( 1 << nDstBitsPerPixel ) - 1 ) / ( nMaxSampleValue - nMinSampleValue );
            sal_uInt8* pt = aMap[0].data();
            sal_uInt8* ptend = pt + nBytesPerRow;
            sal_uInt8 nShift;

            switch ( nDstBitsPerPixel )
            {
                case 8 :
                {
                    if (pt + nImageWidth > ptend)
                        return false;

                    if ( bByteSwap )
                    {
                        if ( nPredictor == 2 )
                        {
                            sal_uInt8 nLast = 0;
                            for (sal_Int32 nx = 0; nx < nImageWidth; ++nx)
                            {
                                nLast += nx == 0 ? BYTESWAP( *pt++ ) : *pt++;
                                SetPixelIndex(xAcc.get(), nY, nx, nLast);
                            }
                        }
                        else
                        {
                            for (sal_Int32 nx = 0; nx < nImageWidth; ++nx)
                            {
                                sal_uInt8 nLast = *pt++;
                                SetPixelIndex(xAcc.get(), nY, nx, static_cast<sal_uInt8>( (BYTESWAP((sal_uInt32)nLast) - nMinSampleValue) * nMinMax ));
                            }
                        }
                    }
                    else
                    {
                        if ( nPredictor == 2 )
                        {
                            sal_uInt8 nLast = 0;
                            for (sal_Int32 nx = 0; nx < nImageWidth; ++nx)
                            {
                                nLast += *pt++;
                                SetPixelIndex(xAcc.get(), nY, nx, nLast);
                            }
                        }
                        else
                        {
                            for (sal_Int32 nx = 0; nx < nImageWidth; ++nx)
                            {
                                SetPixelIndex(xAcc.get(), nY, nx, static_cast<sal_uInt8>( ((sal_uInt32)*pt++ - nMinSampleValue) * nMinMax ));
                            }
                        }
                    }
                }
                break;

                case 7 :
                case 6 :
                case 5 :
                case 4 :
                case 3 :
                case 2 :
                {
                    for (sal_Int32 nx = 0; nx < nImageWidth; ++nx)
                    {
                        nVal = ( GetBits( pt, nx * nBitsPerSample, nBitsPerSample ) - nMinSampleValue ) * nMinMax;
                        SetPixelIndex(xAcc.get(), nY, nx, static_cast<sal_uInt8>(nVal));
                    }
                }
                break;

                case 1 :
                {
                    sal_uInt32 nByteCount = nImageWidth >> 3;

                    sal_uInt32 nBytesNeeded = nByteCount;
                    if (nImageWidth & 7)
                        ++nBytesNeeded;
                    if (pt + nBytesNeeded > ptend)
                        return false;

                    if ( bByteSwap )
                    {
                        sal_Int32 nx = 0;
                        while (nByteCount--)
                        {
                            nByteVal = *pt++;
                            SetPixelIndex(xAcc.get(), nY, nx++, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, nx++, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, nx++, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, nx++, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, nx++, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, nx++, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, nx++, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, nx++, nByteVal);
                        }
                        if ( nImageWidth & 7 )
                        {
                            nByteVal = *pt++;
                            while ( nx < nImageWidth )
                            {
                                SetPixelIndex(xAcc.get(), nY, nx++, nByteVal & 1);
                                nByteVal >>= 1;
                            }
                        }
                    }
                    else
                    {
                        sal_Int32 nx = 7;
                        while (nByteCount--)
                        {
                            nByteVal = *pt++;
                            SetPixelIndex(xAcc.get(), nY, nx, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, --nx, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, --nx, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, --nx, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, --nx, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, --nx, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, --nx, nByteVal & 1);
                            nByteVal >>= 1;
                            SetPixelIndex(xAcc.get(), nY, --nx, nByteVal);
                            nx += 15;
                        }
                        if ( nImageWidth & 7 )
                        {
                            nx -= 7;
                            nByteVal = *pt++;
                            nShift = 7;
                            while ( nx < nImageWidth )
                            {
                                SetPixelIndex(xAcc.get(), nY, nx++, ( nByteVal >> nShift ) & 1);
                            }
                        }
                    }
                }
                break;

                default :
                    return false;
            }
        }
    }
    else if ( ( nSamplesPerPixel == 2 ) && ( nBitsPerSample == 8 ) &&
        ( nPlanarConfiguration == 1 ) && aColorMap.empty() )               // grayscale
    {
        if ( nMaxSampleValue > nMinSampleValue )
        {
            sal_uInt32 nMinMax = ( ( 1 << 8 /*nDstBitsPerPixel*/ ) - 1 ) / ( nMaxSampleValue - nMinSampleValue );
            sal_uInt8*  pt = aMap[0].data();
            for (sal_Int32 nx = 0; nx < nImageWidth; nx++, pt += 2 )
            {
                SetPixelIndex(xAcc.get(), nY, nx, static_cast<sal_uInt8>( ((sal_uInt32)*pt - nMinSampleValue) * nMinMax));
            }
        }
    }
    else
        return false;
    return true;
}


void TIFFReader::MakePalCol()
{
    if ( nDstBitsPerPixel <= 8 )
    {
        aColorMap.resize(256);
        if ( nPhotometricInterpretation <= 1 )
        {
            nNumColors = (sal_uInt32)1 << nBitsPerSample;
            if ( nNumColors > 256 )
                nNumColors = 256;

            if (nLargestPixelIndex >= static_cast<int>(nNumColors))
            {
                SAL_WARN("filter.tiff", "palette has less entries that largest index used. Expanding palette to match");
                nNumColors = nLargestPixelIndex + 1;
            }

            for (sal_uInt32 i = 0; i < nNumColors; ++i)
            {
                sal_uInt32 nVal = ( i * 255 / ( nNumColors - 1 ) ) & 0xff;
                sal_uInt32 n0RGB = nVal | ( nVal << 8 ) | ( nVal << 16 );
                if ( nPhotometricInterpretation == 1 )
                    aColorMap[i] = n0RGB;
                else
                    aColorMap[nNumColors - i - 1] = n0RGB;
            }
        }
        xAcc->SetPaletteEntryCount(std::max<sal_uInt16>(nNumColors, xAcc->GetPaletteEntryCount()));
        for (sal_uInt32 i = 0; i < nNumColors; ++i)
        {
            xAcc->SetPaletteColor(i, BitmapColor( (sal_uInt8)( aColorMap[ i ] >> 16 ),
                (sal_uInt8)( aColorMap[ i ] >> 8 ), (sal_uInt8)aColorMap[ i ] ) );
        }
    }

    if ( fXResolution > 1.0 && fYResolution > 1.0 && ( nResolutionUnit == 2 || nResolutionUnit == 3 ) )
    {
        sal_uInt32 nRX, nRY;
        if (nResolutionUnit==2)
        {
            nRX=(sal_uInt32)(fXResolution+0.5);
            nRY=(sal_uInt32)(fYResolution+0.5);
        }
        else
        {
            nRX=(sal_uInt32)(fXResolution*2.54+0.5);
            nRY=(sal_uInt32)(fYResolution*2.54+0.5);
        }
        MapMode aMapMode(MapUnit::MapInch,Point(0,0),Fraction(1,nRX),Fraction(1,nRY));
        aBitmap.SetPrefMapMode(aMapMode);
        aBitmap.SetPrefSize(Size(nImageWidth,nImageLength));
    }
}


void TIFFReader::ReadHeader()
{
    sal_uInt8 nbyte1(0), nbyte2(0);
    sal_uInt16 nushort(0);

    pTIFF->ReadUChar( nbyte1 );
    if ( nbyte1 == 'I' )
        pTIFF->SetEndian( SvStreamEndian::LITTLE );
    else
        pTIFF->SetEndian( SvStreamEndian::BIG );

    pTIFF->ReadUChar( nbyte2 ).ReadUInt16( nushort );
    if ( nbyte1 != nbyte2 || ( nbyte1 != 'I' && nbyte1 != 'M' ) || nushort != 0x002a )
        bStatus = false;
}

bool TIFFReader::HasAlphaChannel() const
{
    /*There are undoubtedly more variants we could support, but keep it simple for now*/
    return (
             nDstBitsPerPixel == 24 &&
             nBitsPerSample == 8 &&
             nSamplesPerPixel >= 4 &&
             nPlanes == 1 &&
             nPhotometricInterpretation == 2
           );
}


bool TIFFReader::ReadTIFF(SvStream & rTIFF, Graphic & rGraphic )
{
    sal_uInt16  i, nNumTags(0), nTagType(0);
    sal_uInt32 nFirstIfd(0), nDataLen;

    bStatus = true;

    pTIFF = &rTIFF;
    sal_uInt64 nMaxPos = nOrigPos = pTIFF->Tell();
    nEndOfFile = nOrigPos + pTIFF->remainingSize();
    // number format of pTIFF at the beginning
    SvStreamEndian nOrigNumberFormat = pTIFF->GetEndian();

    // read header:
    ReadHeader();

    // read first IFD:
    pTIFF->ReadUInt32( nFirstIfd );

    if( !nFirstIfd || pTIFF->GetError() )
        bStatus = false;

    if ( bStatus )
    {
        sal_uInt32 nOffset = nFirstIfd;

        std::vector<sal_uInt32> aSeenOffsets;
        // calculate length of TIFF file
        do
        {
            if (std::find(aSeenOffsets.begin(), aSeenOffsets.end(), nOffset) != aSeenOffsets.end())
            {
                SAL_WARN("filter.tiff", "Parsing error: " << nOffset <<
                         " already processed, format loop");
                bStatus = false;
                break;
            }
            pTIFF->Seek(nOrigPos + nOffset);
            aSeenOffsets.push_back(nOffset);

            if( pTIFF->GetError() )
            {
                pTIFF->ResetError();
                break;
            };
            nMaxPos = std::max( pTIFF->Tell(), nMaxPos );

            pTIFF->ReadUInt16( nNumTags );

            // loop through tags:
            for( i = 0; i < nNumTags; i++ )
            {
                nTagType = 0;
                nDataType = USHRT_MAX;
                nDataLen = 0;
                nOffset = 0;
                pTIFF->ReadUInt16( nTagType ).ReadUInt16( nDataType ).ReadUInt32( nDataLen ).ReadUInt32( nOffset );

                if( DataTypeSize() * nDataLen > 4 )
                    nMaxPos = std::max(nOrigPos + nOffset + DataTypeSize() * nDataLen, nMaxPos);
            }
            pTIFF->ReadUInt32( nOffset );
            if ( pTIFF->IsEof() )
                nOffset = 0;

            nMaxPos = std::max( pTIFF->Tell(), nMaxPos );
            if ( !nOffset )
                nMaxPos = std::max( pTIFF->Tell(), nMaxPos );
        }
        while( nOffset );

        std::vector<sal_uInt32> aSeenIfds;

        for ( sal_uInt32 nNextIfd = nFirstIfd; nNextIfd && bStatus; )
        {
            if (std::find(aSeenIfds.begin(), aSeenIfds.end(), nNextIfd) != aSeenIfds.end())
            {
                SAL_WARN("filter.tiff", "Parsing error: " << nNextIfd <<
                         " already processed, format loop");
                bStatus = false;
                break;
            }
            pTIFF->Seek(nOrigPos + nNextIfd);
            aSeenIfds.push_back(nNextIfd);
            {
                bByteSwap = false;

                nNewSubFile = 0;
                nSubFile = 0;
                nImageWidth = 0;
                nImageLength = 0;
                nBitsPerSample = 1;                         // default value according to the documentation
                nCompression = 1;
                nPhotometricInterpretation = 0;
                nThresholding = 1;                          // default value according to the documentation
                nCellWidth = 1;
                nCellLength = 1;
                nFillOrder = 1;                             // default value according to the documentation
                nOrientation = 1;
                nSamplesPerPixel = 1;                       // default value according to the documentation
                nRowsPerStrip = 0xffffffff;                 // default value according to the documentation
                nMinSampleValue = 0;                        // default value according to the documentation
                nMaxSampleValue = 0;
                fXResolution = 0.0;
                fYResolution = 0.0;
                nPlanarConfiguration = 1;
                nGroup3Options = 0;                         // default value according to the documentation
                nGroup4Options = 0;                         // default value according to the documentation
                nResolutionUnit = 2;                        // default value according to the documentation
                nPredictor = 1;
                nNumColors = 0;

                xAcc.reset();
                aStripOffsets.clear();
                aStripByteCounts.clear();
                for (auto& j : aMap)
                    j.clear();

                pTIFF->ReadUInt16( nNumTags );
                sal_uInt64 nPos = pTIFF->Tell();

                const size_t nMinRecordSize = 8;
                const size_t nMaxRecords = pTIFF->remainingSize() / nMinRecordSize;
                if (nNumTags > nMaxRecords)
                {
                    SAL_WARN("filter.tiff", "Parsing error: " << nMaxRecords <<
                             " max possible entries, but " << nNumTags << " claimed, truncating");
                    nNumTags = nMaxRecords;
                }

                for( i = 0; i < nNumTags; i++ )
                {
                    pTIFF->ReadUInt16( nTagType ).ReadUInt16( nDataType ).ReadUInt32( nDataLen );

                    if( DataTypeSize() * nDataLen > 4 )
                    {
                        pTIFF->ReadUInt32( nOffset );
                        pTIFF->Seek(nOrigPos + nOffset);
                    }
                    ReadTagData( nTagType, nDataLen );
                    nPos += 12; pTIFF->Seek( nPos );

                    if ( pTIFF->GetError() )
                        bStatus = false;

                    if ( !bStatus )
                        break;
                }
                pTIFF->ReadUInt32( nNextIfd );
                if ( pTIFF->IsEof() )
                    nNextIfd = 0;
            }
            if ( !nBitsPerSample || ( nBitsPerSample > 32 ) )
                bStatus = false;
            if (nImageWidth < 0 || nImageLength < 0)
                bStatus = false;
            if ( bStatus )
            {
                nLargestPixelIndex = -1;
                if ( nMaxSampleValue == 0 )
                {
                    if ( nBitsPerSample == 32 )         // sj: i93300, compiler bug, 1 << 32 gives 1 one 32bit windows platforms,
                        nMaxSampleValue = 0xffffffff;   // (up from 80286 only the lower 5 bits are used when shifting a 32bit register)
                    else
                        nMaxSampleValue = ( 1 << nBitsPerSample ) - 1;
                }
                if ( nPhotometricInterpretation == 2 || nPhotometricInterpretation == 5 || nPhotometricInterpretation == 6 )
                    nDstBitsPerPixel = 24;
                else if ( nBitsPerSample*nSamplesPerPixel <= 1 )
                    nDstBitsPerPixel = 1;
                else if ( nBitsPerSample*nSamplesPerPixel <= 4 )
                    nDstBitsPerPixel = 4;
                else
                    nDstBitsPerPixel = 8;

                if ( nPlanarConfiguration == 1 )
                    nPlanes = 1;
                else
                    nPlanes = nSamplesPerPixel;

                bStatus = nPlanes != 0;
            }

            sal_uInt32 nDiv = GetRowsPerStrip() + 1;

            if ( bStatus )
            {
                bStatus = (nDiv != 0);
            }

            if ( bStatus )
            {
                if ( ( nFillOrder == 2 ) && ( nCompression != 5 ) )     // in the LZW mode bits are already being inverted
                    bByteSwap = true;
                nStripsPerPlane = ( nImageLength - 1 ) / nDiv;
            }

            if ( bStatus )
            {
                sal_uInt64 nRowSize = (static_cast<sal_uInt64>(nImageWidth) * nSamplesPerPixel / nPlanes * nBitsPerSample + 7) >> 3;
                if (nRowSize > SAL_MAX_INT32 / SAL_N_ELEMENTS(aMap))
                {
                    SAL_WARN("filter.tiff", "Ludicrous row size of: " << nRowSize << " required");
                    bStatus = false;
                }
                else
                    nBytesPerRow = nRowSize;
            }

            if ( bStatus )
            {
                //sanity check consider ReadMap condition for last row and
                //last plane
                if (nCompression == 1 || nCompression == 32771)
                {
                    sal_uInt32 nStripBytesPerRow;
                    if (nCompression == 1)
                        nStripBytesPerRow = nBytesPerRow;
                    else
                        nStripBytesPerRow = ( nBytesPerRow + 1 ) & 0xfffffffe;
                    sal_uInt32 np = nPlanes - 1;
                    if (np >= SAL_N_ELEMENTS(aMap))
                        bStatus = false;
                    sal_Int32 ny = nImageLength - 1;
                    sal_uInt32 nStrip(0);
                    nDiv = GetRowsPerStrip();
                    if (bStatus)
                        bStatus = nDiv != 0;
                    if (bStatus)
                    {
                        nStrip = ny / nDiv + np * nStripsPerPlane;
                        if (nStrip >= aStripOffsets.size())
                            bStatus = false;
                    }
                    if (bStatus)
                    {
                        auto nStart = aStripOffsets[ nStrip ] + ( ny % GetRowsPerStrip() ) * nStripBytesPerRow;
                        auto nEnd = nStart + nBytesPerRow;
                        if (nEnd > nEndOfFile)
                            bStatus = false;
                    }
                }
                else if (nCompression == 2 || nCompression == 3 || nCompression == 4)
                {
                    if (nCompression == 3 && nGroup3Options & 0xfffffffa)
                        bStatus = false;
                    else if (nCompression == 4 && nGroup4Options & 0xffffffff)
                        bStatus = false;
                    sal_uInt32 np = nPlanes - 1;
                    if (np >= SAL_N_ELEMENTS(aMap))
                        bStatus = false;
                    sal_Int32 ny = nImageLength - 1;
                    sal_uInt32 nStrip(0);
                    nDiv = GetRowsPerStrip();
                    if (bStatus)
                        bStatus = nDiv != 0;
                    if (bStatus)
                    {
                        nStrip = ny / nDiv + np * nStripsPerPlane;
                        if (nStrip >= aStripOffsets.size())
                            bStatus = false;
                    }
                    if (bStatus)
                    {
                        auto nStart = aStripOffsets[nStrip];
                        if (nStart > nEndOfFile)
                            bStatus = false;
                    }
                }
                else if ( nCompression == 5 )
                {
                    sal_uInt32 np = nPlanes - 1;
                    if (np >= SAL_N_ELEMENTS(aMap))
                        bStatus = false;
                    sal_Int32 ny = nImageLength - 1;
                    sal_uInt32 nStrip(0);
                    nDiv = GetRowsPerStrip();
                    if (bStatus)
                        bStatus = nDiv != 0;
                    if (bStatus)
                    {
                        nStrip = ny / nDiv + np * nStripsPerPlane;
                        if (nStrip >= aStripOffsets.size())
                            bStatus = false;
                    }
                    if (bStatus)
                    {
                        auto nStart = aStripOffsets[nStrip];
                        if (nStart > nEndOfFile)
                            bStatus = false;
                    }
                }
            }

            if ( bStatus )
            {
                pAlphaMask.reset();
                Size aTargetSize(nImageWidth, nImageLength);
                aBitmap = Bitmap(aTargetSize, nDstBitsPerPixel);
                xAcc = Bitmap::ScopedWriteAccess(aBitmap);
                if (xAcc && xAcc->Width() == nImageWidth && xAcc->Height() == nImageLength)
                {
                    for (auto& j : aMap)
                    {
                        try
                        {
                            j.resize(nBytesPerRow);
                        }
                        catch (const std::bad_alloc &)
                        {
                            j.clear();
                            bStatus = false;
                            break;
                        }
                    }

                    if (bStatus && HasAlphaChannel())
                    {
                        pAlphaMask.reset( new AlphaMask( aTargetSize ) );
                        xMaskAcc = AlphaMask::ScopedWriteAccess(*pAlphaMask);
                    }

                    if (bStatus && ReadMap())
                    {
                        nMaxPos = std::max( pTIFF->Tell(), nMaxPos );
                        MakePalCol();
                        nMaxPos = std::max( pTIFF->Tell(), nMaxPos );
                    }
                    else
                        bStatus = false;

                    xAcc.reset();
                    xMaskAcc.reset();

                    if ( bStatus )
                    {
                        BitmapEx aImage;

                        if (pAlphaMask)
                            aImage = BitmapEx( aBitmap, *pAlphaMask );
                        else
                            aImage = aBitmap;

                        AnimationBitmap aAnimationBitmap( aImage, Point( 0, 0 ), aBitmap.GetSizePixel(),
                                                          ANIMATION_TIMEOUT_ON_CLICK, Disposal::Back );

                        aAnimation.Insert( aAnimationBitmap );
                    }
                }
            }

            // Clean up:
            for (auto& j : aMap)
                j.clear();
            aColorMap.clear();
            aStripOffsets.clear();
            aStripByteCounts.clear();
        }
    }

    // seek to end of TIFF if succeeded
    pTIFF->SetEndian( nOrigNumberFormat );
    pTIFF->Seek(bStatus ? nMaxPos : nOrigPos);

    if ( aAnimation.Count() )
    {
        if ( aAnimation.Count() == 1 )
            rGraphic = aAnimation.GetBitmapEx();
        else
            rGraphic = aAnimation;  //aBitmap;

        return true;
    }
    else
        return false;
}


//================== GraphicImport - the exported function ================

extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
itiGraphicImport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* )
{
    TIFFReader aTIFFReader;

    return aTIFFReader.ReadTIFF( rStream, rGraphic );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
