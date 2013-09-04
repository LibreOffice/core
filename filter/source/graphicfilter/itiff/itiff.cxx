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

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/animate.hxx>
#include "lzwdecom.hxx"
#include "ccidecom.hxx"

class FilterConfigItem;

#define OOODEBUG(str,Num) //(InfoBox(NULL,String(str)+String(" ")+String(Num)).Execute();

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

    sal_Bool                bStatus;                    // Whether until now no error occurred
    Animation               aAnimation;
    sal_uLong               nLastPercent;

    SvStream*               pTIFF;                      // the TIFF file that should be read
    Bitmap                  aBitmap;
    BitmapWriteAccess*      pAcc;
    sal_uInt16              nDstBitsPerPixel;
    AlphaMask*              pAlphaMask;
    BitmapWriteAccess*      pMaskAcc;

    sal_uLong               nOrigPos;                   // start position in pTIFF
    sal_uInt16              nOrigNumberFormat;          // number format of pTIFF at the beginning


    sal_uInt16              nDataType;
    // Data taken from the TIFF tags:
    sal_Bool                bByteSwap;                  // sal_True if bits 0..7 -> 7..0 should get converted ( FILLORDER = 2 );

    sal_uLong               nNewSubFile;                //
    sal_uLong               nSubFile;                   //
    sal_uLong               nImageWidth;                // picture width in pixels
    sal_uLong               nImageLength;               // picture height in pixels
    sal_uLong               nBitsPerSample;             // bits per pixel per layer
    sal_uLong               nCompression;               // kind of compression
    sal_uLong               nPhotometricInterpretation; //
    sal_uLong               nThresholding;              //
    sal_uLong               nCellWidth;                 //
    sal_uLong               nCellLength;                //
    sal_uLong               nFillOrder;                 //
    sal_uLong*              pStripOffsets;              // Fiels of offsets to the Bitmap-Data-"Strips"
    sal_uLong               nNumStripOffsets;           // size of the field above
    sal_uLong               nOrientation;               //
    sal_uLong               nSamplesPerPixel;           // number of layers
    sal_uLong               nRowsPerStrip;              // if it's not compressed: number of rows per Strip
    sal_uLong*              pStripByteCounts;           // if compressed (in a certain way): size of the strips
    sal_uLong               nNumStripByteCounts;        // number of entries in the field above
    sal_uLong               nMinSampleValue;            //
    sal_uLong               nMaxSampleValue;            //
    double                  fXResolution;               // X-resolution or 0.0
    double                  fYResolution;               // Y-resolution or 0.0
    sal_uLong               nPlanarConfiguration;       //
    sal_uLong               nGroup3Options;             //
    sal_uLong               nGroup4Options;             //
    sal_uLong               nResolutionUnit;            // unit of fX/YResolution: 1=unknown, 2(default)=inch, 3=cm
    sal_uLong               nPredictor;                 //
    sal_uLong*              pColorMap;                  // color palette
    sal_uLong               nNumColors;                 // number of colors within the color palette

    sal_uLong               nPlanes;                    // number of layers within the Tiff file
    sal_uLong               nStripsPerPlane;            // number of Strips per layer
    sal_uLong               nBytesPerRow;               // Bytes per line per Layer in the Tiff file ( uncompressed )
    sal_uInt8*              pMap[ 4 ];                  // temporary Scanline


    void    MayCallback( sal_uLong nPercent );

    sal_uLong   DataTypeSize();
    sal_uLong   ReadIntData();
    double  ReadDoubleData();

    void    ReadHeader();
    void    ReadTagData( sal_uInt16 nTagType, sal_uInt32 nDataLen );

    sal_Bool    ReadMap( sal_uLong nMinPercent, sal_uLong nMaxPercent );
        // reads/decompresses the bitmap data and fills pMap

    sal_uLong   GetBits( const sal_uInt8 * pSrc, sal_uLong nBitsPos, sal_uLong nBitsCount );
        // fetches BitsCount bits from pSrc[..] at the position nBitsPos

    void    MakePalCol( void );
        // Create the bitmap from the temporary bitmap pMap
        // and partly deletes pMap while doing this.

    sal_Bool    ConvertScanline( sal_uLong nY );
        // converts a Scanline to the Windows-BMP format

    bool HasAlphaChannel() const;
public:

    TIFFReader() : pAlphaMask(0), pMaskAcc(0) {}
    ~TIFFReader()
    {
        delete pAlphaMask;
    }

    sal_Bool ReadTIFF( SvStream & rTIFF, Graphic & rGraphic );
};

//=================== Methods of TIFFReader ==============================

void TIFFReader::MayCallback( sal_uLong /*nPercent*/ )
{
}

// ---------------------------------------------------------------------------------

sal_uLong TIFFReader::DataTypeSize()
{
    sal_uLong nSize;
    switch ( nDataType )
    {
        case 1 :            // BYTE
        case 2 :            // ACSII
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
        case 10 :           // SIGNED RATINAL
        case 12 :           // DOUBLE
            nSize = 8;
            break;
        default:
            pTIFF->SetError(SVSTREAM_FILEFORMAT_ERROR);
            nSize=1;
    }
    return nSize;
}

// ---------------------------------------------------------------------------------

sal_uLong TIFFReader::ReadIntData()
{
    double  nDOUBLE;
    float   nFLOAT;
    sal_uInt32  nUINT32a, nUINT32b;
    sal_Int32   nINT32;
    sal_uInt16  nUINT16;
    sal_Int16   nINT16;
    sal_uInt8   nBYTE;
    char    nCHAR;

    switch( nDataType )
    {
        case 0 :    //??
        case 1 :
        case 2 :
        case 7 :
            *pTIFF >> nBYTE;
            nUINT32a = (sal_uLong)nBYTE;
        break;
        case 3 :
             *pTIFF >> nUINT16;
             nUINT32a = (sal_uLong)nUINT16;
        break;
        case 9 :
        case 4 :
            *pTIFF >> nUINT32a;
        break;
        case  5 :
            *pTIFF >> nUINT32a >> nUINT32b;
            if ( nUINT32b != 0 )
                nUINT32a /= nUINT32b;
        break;
        case 6 :
            *pTIFF >> nCHAR;
            nUINT32a = (sal_Int32)nCHAR;
        break;
        case 8 :
            *pTIFF >> nINT16;
            nUINT32a = (sal_Int32)nINT16;
        break;
        case 10 :
            *pTIFF >> nUINT32a >> nINT32;
            if ( nINT32 != 0 )
                nUINT32a /= nINT32;
        break;
        case 11 :
            *pTIFF >> nFLOAT;
            nUINT32a = (sal_Int32)nFLOAT;
        break;
        case 12 :
            *pTIFF >> nDOUBLE;
            nUINT32a = (sal_Int32)nDOUBLE;
        break;
        default:
            *pTIFF >> nUINT32a;
        break;
    }
    return nUINT32a;
}

// ---------------------------------------------------------------------------------

double TIFFReader::ReadDoubleData()
{
    sal_uInt32 nulong;
    double  nd;

    if ( nDataType == 5 )
    {
        *pTIFF >> nulong;
        nd = (double)nulong;
        *pTIFF >> nulong;
        if ( nulong != 0 )
            nd /= (double)nulong;
    }
    else
        nd = (double)ReadIntData();
    return nd;
}

// ---------------------------------------------------------------------------------

void TIFFReader::ReadTagData( sal_uInt16 nTagType, sal_uInt32 nDataLen)
{
    if ( bStatus == sal_False )
        return;

    switch ( nTagType )
    {
        case 0x00fe:   // New Sub File
            nNewSubFile = ReadIntData();
            OOODEBUG("NewSubFile",nNewSubFile);
            break;

        case 0x00ff:   // Sub File
            nSubFile = ReadIntData();
            OOODEBUG("SubFile",nSubFile);
            break;

        case 0x0100:   // Image Width
            nImageWidth = ReadIntData();
            OOODEBUG("ImageWidth",nImageWidth);
            break;

        case 0x0101:   // Image Length
            nImageLength = ReadIntData();
            OOODEBUG("ImageLength",nImageLength);
            break;

        case 0x0102:   // Bits Per Sample
            nBitsPerSample = ReadIntData();
            OOODEBUG("BitsPerSample",nBitsPerSample);
            if ( nBitsPerSample >= 32 ) // 32 bit and larger samples are not supported
                bStatus = sal_False;
            break;

        case 0x0103:   // Compression
            nCompression = ReadIntData();
            OOODEBUG("Compression",nCompression);
            break;

        case 0x0106:   // Photometric Interpreation
            nPhotometricInterpretation = ReadIntData();
            OOODEBUG("PhotometricInterpretation",nPhotometricInterpretation);
            break;

        case 0x0107:   // Thresholding
            nThresholding = ReadIntData();
            OOODEBUG("Thresholding",nThresholding);
            break;

        case 0x0108:   // Cell Width
            nCellWidth = ReadIntData();
            break;

        case 0x0109:   // Cell Length
            nCellLength = ReadIntData();
            break;

        case 0x010a:   // Fill Order
            nFillOrder = ReadIntData();
            OOODEBUG("FillOrder",nFillOrder);
            break;

        case 0x0111: { // Strip Offset(s)
            sal_uLong nOldNumSO, i, * pOldSO;
            pOldSO = pStripOffsets;
            if ( pOldSO == NULL )
                nNumStripOffsets = 0;
            nOldNumSO = nNumStripOffsets;
            nDataLen += nOldNumSO;
            if ( ( nDataLen > nOldNumSO ) && ( nDataLen < SAL_MAX_UINT32 / sizeof( sal_uInt32 ) ) )
            {
                nNumStripOffsets = nDataLen;
                try
                {
                    pStripOffsets = new sal_uLong[ nNumStripOffsets ];
                }
                    catch (const std::bad_alloc &)
                {
                    pStripOffsets = NULL;
                    nNumStripOffsets = 0;
                }
                if ( pStripOffsets )
                {
                    for ( i = 0; i < nOldNumSO; i++ )
                        pStripOffsets[ i ] = pOldSO[ i ] + nOrigPos;
                    for ( i = nOldNumSO; i < nNumStripOffsets; i++ )
                        pStripOffsets[ i ] = ReadIntData() + nOrigPos;
                }
                delete[] pOldSO;
            }
            OOODEBUG("StripOffsets (Anzahl:)",nDataLen);
            break;
        }
        case 0x0112:   // Orientation
            nOrientation = ReadIntData();
            OOODEBUG("Orientation",nOrientation);
            break;

        case 0x0115:   // Samples Per Pixel
            nSamplesPerPixel = ReadIntData();
            OOODEBUG("SamplesPerPixel",nSamplesPerPixel);
            break;

        case 0x0116:   // Rows Per Strip
            nRowsPerStrip = ReadIntData();
            OOODEBUG("RowsPerStrip",nRowsPerStrip);
            break;

        case 0x0117: { // Strip Byte Counts
            sal_uLong nOldNumSBC, i, * pOldSBC;
            pOldSBC = pStripByteCounts;
            if ( pOldSBC == NULL )
                nNumStripByteCounts = 0; // to be on the safe side
            nOldNumSBC = nNumStripByteCounts;
            nDataLen += nOldNumSBC;
            if ( ( nDataLen > nOldNumSBC ) && ( nDataLen < SAL_MAX_UINT32 / sizeof( sal_uInt32 ) ) )
            {
                nNumStripByteCounts = nDataLen;
                try
                {
                    pStripByteCounts = new sal_uLong[ nNumStripByteCounts ];
                }
                    catch (const std::bad_alloc &)
                {
                    pStripByteCounts = NULL;
                    nNumStripByteCounts = 0;
                }
                if ( pStripByteCounts )
                {
                    for ( i = 0; i < nOldNumSBC; i++ )
                        pStripByteCounts[ i ] = pOldSBC[ i ];
                    for ( i = nOldNumSBC; i < nNumStripByteCounts; i++)
                        pStripByteCounts[ i ] = ReadIntData();
                }
                delete[] pOldSBC;
            }
            OOODEBUG("StripByteCounts (Anzahl:)",nDataLen);
            break;
        }
        case 0x0118:   // Min Sample Value
            nMinSampleValue = ReadIntData();
            OOODEBUG("MinSampleValue",nMinSampleValue);
            break;

        case 0x0119:   // Max Sample Value
            nMaxSampleValue = ReadIntData();
            OOODEBUG("MaxSampleValue",nMaxSampleValue);
            break;

        case 0x011a:   // X Resolution
            fXResolution = ReadDoubleData();
            break;

        case 0x011b:   // Y Resolution
            fYResolution = ReadDoubleData();
            break;

        case 0x011c:   // Planar Configuration
            nPlanarConfiguration = ReadIntData();
            OOODEBUG("PlanarConfiguration",nPlanarConfiguration);
            break;

        case 0x0124:   // Group 3 Options
            nGroup3Options = ReadIntData();
            OOODEBUG("Group3Options",nGroup3Options);
            break;

        case 0x0125:   // Group 4 Options
            nGroup4Options = ReadIntData();
            OOODEBUG("Group4Options",nGroup4Options);
            break;

        case 0x0128:   // Resolution Unit
            nResolutionUnit = ReadIntData();
            break;

        case 0x013d:   // Predictor
            nPredictor = ReadIntData();
            OOODEBUG("Predictor",nPredictor);
            break;

        case 0x0140: { // Color Map
            sal_uInt16 nVal;
            sal_uLong i;
            nNumColors= ( 1UL << nBitsPerSample );
            if ( nDataType == 3 && nNumColors <= 256)
            {
                pColorMap = new sal_uLong[ 256 ];
                for ( i = 0; i < nNumColors; i++ )
                    pColorMap[ i ] = 0;
                for ( i = 0; i < nNumColors; i++ )
                {
                    *pTIFF >> nVal;
                    pColorMap[ i ] |= ( ( (sal_uLong)nVal ) << 8 ) & 0x00ff0000;
                }
                for ( i = 0; i < nNumColors; i++ )
                {
                    *pTIFF >> nVal;
                    pColorMap[ i ] |= ( (sal_uLong)nVal ) & 0x0000ff00;
                }
                for ( i = 0; i < nNumColors; i++ )
                {
                    *pTIFF >> nVal;
                    pColorMap[ i ] |= ( ( (sal_uLong)nVal ) >> 8 ) & 0x000000ff;
                }
            }
            else
                bStatus = sal_False;
            OOODEBUG("ColorMap (Anzahl Farben:)", nNumColors);
            break;
        }

        case 0x0153: { // SampleFormat
            sal_uLong nSampleFormat = ReadIntData();
            if ( nSampleFormat == 3 ) // IEEE floating point samples are not supported yet
                bStatus = sal_False;
            break;
        }
    }

    if ( pTIFF->GetError() )
        bStatus = sal_False;
}

// ---------------------------------------------------------------------------------

sal_Bool TIFFReader::ReadMap( sal_uLong nMinPercent, sal_uLong nMaxPercent )
{
    if ( nCompression == 1 || nCompression == 32771 )
    {
        sal_uLong ny, np, nStrip, nStripBytesPerRow;

        if ( nCompression == 1 )
            nStripBytesPerRow = nBytesPerRow;
        else
            nStripBytesPerRow = ( nBytesPerRow + 1 ) & 0xfffffffe;
        for ( ny = 0; ny < nImageLength; ny++ )
        {
            for ( np = 0; np < nPlanes; np++ )
            {
                nStrip = ny / nRowsPerStrip + np * nStripsPerPlane;
                if ( nStrip >= nNumStripOffsets )
                    return sal_False;
                pTIFF->Seek( pStripOffsets[ nStrip ] + ( ny % nRowsPerStrip ) * nStripBytesPerRow );
                pTIFF->Read( pMap[ np ], nBytesPerRow );
                if ( pTIFF->GetError() )
                    return sal_False;
                MayCallback( nMinPercent + ( nMaxPercent - nMinPercent ) * ( np * nImageLength + ny) / ( nImageLength * nPlanes ) );
            }
            if ( !ConvertScanline( ny ) )
                return sal_False;
        }
    }
    else if ( nCompression == 2 || nCompression == 3 || nCompression == 4 )
    {
        sal_uLong ny, np, nStrip, nOptions;
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
                return sal_False;
        }
        else
        {   // nCompression==4
            nOptions = CCI_OPTION_2D;
            if ( nGroup4Options & 0xffffffff )
                return sal_False;
        }
        if ( nFillOrder == 2 )
        {
            nOptions |= CCI_OPTION_INVERSEBITORDER;
            bByteSwap = sal_False;
        }
        nStrip = 0;
        if ( nStrip >= nNumStripOffsets )
            return sal_False;
        pTIFF->Seek(pStripOffsets[nStrip]);

        CCIDecompressor aCCIDecom( nOptions, nImageWidth );

        aCCIDecom.StartDecompression( *pTIFF );

        for ( ny = 0; ny < nImageLength; ny++ )
        {
            for ( np = 0; np < nPlanes; np++ )
            {
                if ( ny / nRowsPerStrip + np * nStripsPerPlane > nStrip )
                {
                    nStrip=ny/nRowsPerStrip+np*nStripsPerPlane;
                    if ( nStrip >= nNumStripOffsets )
                        return sal_False;
                    pTIFF->Seek( pStripOffsets[ nStrip ] );
                    aCCIDecom.StartDecompression( *pTIFF );
                }
                if ( aCCIDecom.DecompressScanline( pMap[ np ], nImageWidth * nBitsPerSample * nSamplesPerPixel / nPlanes ) == sal_False )
                    return sal_False;
                if ( pTIFF->GetError() )
                    return sal_False;
                MayCallback(nMinPercent+(nMaxPercent-nMinPercent)*(np*nImageLength+ny)/(nImageLength*nPlanes));
            }
            if ( !ConvertScanline( ny ) )
                return sal_False;
        }
    }
    else if ( nCompression == 5 )
    {
        LZWDecompressor aLZWDecom;
        sal_uLong ny, np, nStrip;
        nStrip=0;
        if ( nStrip >= nNumStripOffsets )
            return sal_False;
        pTIFF->Seek(pStripOffsets[nStrip]);
        aLZWDecom.StartDecompression(*pTIFF);
        for ( ny = 0; ny < nImageLength; ny++ )
        {
            for ( np = 0; np < nPlanes; np++ )
            {
                if ( ny / nRowsPerStrip + np * nStripsPerPlane > nStrip )
                {
                    nStrip = ny / nRowsPerStrip + np * nStripsPerPlane;
                    if ( nStrip >= nNumStripOffsets )
                        return sal_False;
                    pTIFF->Seek(pStripOffsets[nStrip]);
                    aLZWDecom.StartDecompression(*pTIFF);
                }
                if ( ( aLZWDecom.Decompress( pMap[ np ], nBytesPerRow ) != nBytesPerRow ) || pTIFF->GetError() )
                    return sal_False;
                MayCallback(nMinPercent+(nMaxPercent-nMinPercent)*(np*nImageLength+ny)/(nImageLength*nPlanes));
            }
            if ( !ConvertScanline( ny ) )
                return sal_False;
        }
    }
    else if ( nCompression == 32773 )
    {
        sal_uLong nStrip,nRecCount,nRowBytesLeft,ny,np,i;
        sal_uInt8 * pdst, nRecHeader, nRecData;
        nStrip = 0;
        if ( nStrip >= nNumStripOffsets )
            return sal_False;
        pTIFF->Seek(pStripOffsets[nStrip]);
        for ( ny = 0; ny < nImageLength; ny++ )
        {
            for ( np = 0; np < nPlanes; np++ )
            {
                if ( ny / nRowsPerStrip + np * nStripsPerPlane > nStrip )
                {
                    nStrip=ny/nRowsPerStrip+np*nStripsPerPlane;
                    if ( nStrip >= nNumStripOffsets )
                        return sal_False;
                    pTIFF->Seek(pStripOffsets[nStrip]);
                }
                nRowBytesLeft = nBytesPerRow;
                pdst=pMap[ np ];
                do
                {
                    *pTIFF >> nRecHeader;
                    if ((nRecHeader&0x80)==0)
                    {
                        nRecCount=0x00000001+((sal_uLong)nRecHeader);
                        if ( nRecCount > nRowBytesLeft )
                            return sal_False;
                        pTIFF->Read(pdst,nRecCount);
                        pdst+=nRecCount;
                        nRowBytesLeft-=nRecCount;
                    }
                    else if ( nRecHeader != 0x80 )
                    {
                        nRecCount = 0x000000101 - ( (sal_uLong)nRecHeader );
                        if ( nRecCount > nRowBytesLeft )
                        {
                            nRecCount = nRowBytesLeft;

//                          bStatus = sal_False;
//                          return;

                        }
                        *pTIFF >> nRecData;
                        for ( i = 0; i < nRecCount; i++ )
                            *(pdst++) = nRecData;
                        nRowBytesLeft -= nRecCount;
                    }
                } while ( nRowBytesLeft != 0 );
                if ( pTIFF->GetError() )
                    return sal_False;
                MayCallback(nMinPercent+(nMaxPercent-nMinPercent)*(np*nImageLength+ny)/(nImageLength*nPlanes));
            }
            if ( !ConvertScanline( ny ) )
                return sal_False;
        }
    }
    else
        return sal_False;
    return sal_True;
}

sal_uLong TIFFReader::GetBits( const sal_uInt8 * pSrc, sal_uLong nBitsPos, sal_uLong nBitsCount )
{
    sal_uLong nRes;
    if ( bByteSwap )
    {
        pSrc += ( nBitsPos >> 3 );
        nBitsPos &= 7;
        sal_uInt8 nDat = *pSrc;
        nRes = (sal_uLong)( BYTESWAP( nDat ) & ( 0xff >> nBitsPos ) );

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
                nRes = ( nRes << 8 ) | ((sal_uLong)BYTESWAP( nDat ) );
                nBitsCount -= 8;
            }
            if ( nBitsCount > 0 )
            {
                nDat = *pSrc;
                nRes = ( nRes << nBitsCount ) | (((sal_uLong)BYTESWAP(nDat))>>(8-nBitsCount));
            }
        }
    }
    else
    {
        pSrc += ( nBitsPos >> 3 );
        nBitsPos &= 7;
        nRes = (sal_uLong)((*pSrc)&(0xff>>nBitsPos));
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
                nRes = ( nRes << 8 ) | ((sal_uLong)*(pSrc++));
                nBitsCount -= 8;
            }
            if ( nBitsCount > 0 )
                nRes = ( nRes << nBitsCount ) | (((sal_uLong)*pSrc)>>(8-nBitsCount));
        }
    }
    return nRes;
}

// ---------------------------------------------------------------------------------

sal_Bool TIFFReader::ConvertScanline( sal_uLong nY )
{
    sal_uInt32  nRed, nGreen, nBlue, ns, nx, nVal, nByteCount;
    sal_uInt8   nByteVal;

    if ( nDstBitsPerPixel == 24 )
    {
        if ( nBitsPerSample == 8 && nSamplesPerPixel >= 3 &&
             nPlanes == 1 && nPhotometricInterpretation == 2 )
        {
            sal_uInt8*  pt = pMap[ 0 ];

            // are the values being saved as difference?
            if ( 2 == nPredictor )
            {
                sal_uInt8  nLRed = 0;
                sal_uInt8  nLGreen = 0;
                sal_uInt8  nLBlue = 0;
                sal_uInt8  nLAlpha = 0;
                for ( nx = 0; nx < nImageWidth; nx++, pt += nSamplesPerPixel )
                {
                    nLRed = nLRed + pt[ 0 ];
                    nLGreen = nLGreen + pt[ 1 ];
                    nLBlue = nLBlue + pt[ 2 ];
                    pAcc->SetPixel( nY, nx, Color( nLRed, nLGreen, nLBlue ) );
                    if (nSamplesPerPixel >= 4 && pMaskAcc)
                    {
                        nLAlpha = nLAlpha + pt[ 3 ];
                        pMaskAcc->SetPixel( nY, nx, BitmapColor(~nLAlpha) );
                    }
                }
            }
            else
            {
                for ( nx = 0; nx < nImageWidth; nx++, pt += nSamplesPerPixel )
                {
                    pAcc->SetPixel( nY, nx, Color( pt[0], pt[1], pt[2] ) );
                    if (nSamplesPerPixel >= 4 && pMaskAcc)
                    {
                        sal_uInt8 nAlpha = pt[3];
                        pMaskAcc->SetPixel( nY, nx, BitmapColor(~nAlpha) );
                    }
                }
            }
        }
        else if ( nPhotometricInterpretation == 2 && nSamplesPerPixel >= 3 )
        {
            if ( nMaxSampleValue > nMinSampleValue )
            {
                sal_uLong nMinMax = nMinSampleValue * 255 / ( nMaxSampleValue - nMinSampleValue );
                for ( nx = 0; nx < nImageWidth; nx++ )
                {
                    if ( nPlanes < 3 )
                    {
                        nRed = GetBits( pMap[ 0 ], ( nx * nSamplesPerPixel + 0 ) * nBitsPerSample, nBitsPerSample );
                        nGreen = GetBits( pMap[ 1 ], ( nx * nSamplesPerPixel + 1 ) * nBitsPerSample, nBitsPerSample );
                        nBlue = GetBits( pMap[ 2 ], ( nx * nSamplesPerPixel + 2 ) * nBitsPerSample, nBitsPerSample );
                    }
                    else
                    {
                        nRed = GetBits( pMap[ 0 ], nx * nBitsPerSample, nBitsPerSample );
                        nGreen = GetBits( pMap[ 1 ], nx * nBitsPerSample, nBitsPerSample );
                        nBlue = GetBits( pMap[ 2 ], nx * nBitsPerSample, nBitsPerSample );
                    }
                    pAcc->SetPixel( nY, nx, Color( (sal_uInt8)( nRed - nMinMax ), (sal_uInt8)( nGreen - nMinMax ), (sal_uInt8)(nBlue - nMinMax) ) );
                }
            }
        }
        else if ( nPhotometricInterpretation == 5 && nSamplesPerPixel == 3 )
        {
            if ( nMaxSampleValue > nMinSampleValue )
            {
                sal_uLong nMinMax =  nMinSampleValue * 255 / ( nMaxSampleValue - nMinSampleValue );
                for ( nx = 0; nx < nImageWidth; nx++ )
                {
                    if ( nPlanes < 3 )
                    {
                        nRed = GetBits( pMap[ 0 ],( nx * nSamplesPerPixel + 0 ) * nBitsPerSample, nBitsPerSample );
                        nGreen = GetBits( pMap[ 0 ],( nx * nSamplesPerPixel + 1 ) * nBitsPerSample, nBitsPerSample );
                        nBlue = GetBits( pMap[ 0 ],( nx * nSamplesPerPixel + 2 ) * nBitsPerSample, nBitsPerSample );
                    }
                    else
                    {
                        nRed = GetBits( pMap[ 0 ], nx * nBitsPerSample, nBitsPerSample );
                        nGreen = GetBits( pMap[ 1 ], nx * nBitsPerSample, nBitsPerSample );
                        nBlue = GetBits( pMap[ 2 ], nx * nBitsPerSample, nBitsPerSample );
                    }
                    nRed = 255 - (sal_uInt8)( nRed - nMinMax );
                    nGreen = 255 - (sal_uInt8)( nGreen - nMinMax );
                    nBlue = 255 - (sal_uInt8)( nBlue - nMinMax );
                    pAcc->SetPixel( nY, nx, Color( (sal_uInt8) nRed, (sal_uInt8) nGreen, (sal_uInt8) nBlue ) );
                }
            }
        }
        else if( nPhotometricInterpretation == 5 && nSamplesPerPixel == 4 )
        {
            if ( nMaxSampleValue > nMinSampleValue )
            {
                sal_uInt8   nSamp[ 4 ];
                sal_uInt8   nSampLast[ 4 ] = { 0, 0, 0, 0 };
                long    nBlack;

                for( nx = 0; nx < nImageWidth; nx++ )
                {
                    // are the values being saved as difference?
                    if( 2 == nPredictor )
                    {
                        for( ns = 0; ns < 4; ns++ )
                        {
                            if( nPlanes < 3 )
                                nSampLast[ ns ] = nSampLast[ ns ] + (sal_uInt8) GetBits( pMap[ 0 ], ( nx * nSamplesPerPixel + ns ) * nBitsPerSample, nBitsPerSample );
                            else
                                nSampLast[ ns ] = nSampLast[ ns ] + (sal_uInt8) GetBits( pMap[ ns ], nx * nBitsPerSample, nBitsPerSample );
                            nSamp[ ns ] = nSampLast[ ns ];
                        }
                    }
                    else
                    {
                        for( ns = 0; ns < 4; ns++ )
                        {
                            if( nPlanes < 3 )
                                nSamp[ ns ] = (sal_uInt8) GetBits( pMap[ 0 ], ( nx * nSamplesPerPixel + ns ) * nBitsPerSample, nBitsPerSample );
                            else
                                nSamp[ ns ]= (sal_uInt8) GetBits( pMap[ ns ], nx * nBitsPerSample, nBitsPerSample );
                        }
                    }
                    nBlack = nSamp[ 3 ];
                    nRed = (sal_uInt8) std::max( 0L, 255L - ( ( (long) nSamp[ 0 ] + nBlack - ( ( (long) nMinSampleValue ) << 1 ) ) *
                                255L/(long)(nMaxSampleValue-nMinSampleValue) ) );
                    nGreen = (sal_uInt8) std::max( 0L, 255L - ( ( (long) nSamp[ 1 ] + nBlack - ( ( (long) nMinSampleValue ) << 1 ) ) *
                                255L/(long)(nMaxSampleValue-nMinSampleValue) ) );
                    nBlue = (sal_uInt8) std::max( 0L, 255L - ( ( (long) nSamp[ 2 ] + nBlack - ( ( (long) nMinSampleValue ) << 1 ) ) *
                                255L/(long)(nMaxSampleValue-nMinSampleValue) ) );
                    pAcc->SetPixel( nY, nx, Color ( (sal_uInt8)nRed, (sal_uInt8)nGreen, (sal_uInt8)nBlue ) );
                }
            }
        }
    }
    else if ( nSamplesPerPixel == 1 && ( nPhotometricInterpretation <= 1 || nPhotometricInterpretation == 3 ) )
    {
        if ( nMaxSampleValue > nMinSampleValue )
        {
            sal_uLong nMinMax = ( ( 1 << nDstBitsPerPixel ) - 1 ) / ( nMaxSampleValue - nMinSampleValue );
            sal_uInt8* pt = pMap[ 0 ];
            sal_uInt8 nShift;

            switch ( nDstBitsPerPixel )
            {
                case 8 :
                {
                    sal_uInt8 nLast;
                    if ( bByteSwap )
                    {
                        if ( nPredictor == 2 )
                        {
                            nLast = BYTESWAP( (sal_uInt8)*pt++ );
                            for ( nx = 0; nx < nImageWidth; nx++ )
                            {
                                pAcc->SetPixelIndex( nY, nx, nLast );
                                nLast = nLast + *pt++;
                            }
                        }
                        else
                        {
                            for ( nx = 0; nx < nImageWidth; nx++ )
                            {
                                nLast = *pt++;
                                pAcc->SetPixelIndex( nY, nx, static_cast<sal_uInt8>( (BYTESWAP((sal_uLong)nLast) - nMinSampleValue) * nMinMax ) );
                            }
                        }
                    }
                    else
                    {
                        if ( nPredictor == 2 )
                        {
                            nLast = *pt++;
                            for ( nx = 0; nx < nImageWidth; nx++ )
                            {
                                pAcc->SetPixelIndex( nY, nx, nLast );
                                nLast = nLast + *pt++;
                            }
                        }
                        else
                        {
                            for ( nx = 0; nx < nImageWidth; nx++ )
                            {
                                pAcc->SetPixelIndex( nY, nx, static_cast<sal_uInt8>( ((sal_uLong)*pt++ - nMinSampleValue) * nMinMax ) );

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
                    for ( nx = 0; nx < nImageWidth; nx++ )
                    {
                        nVal = ( GetBits( pt, nx * nBitsPerSample, nBitsPerSample ) - nMinSampleValue ) * nMinMax;
                        pAcc->SetPixelIndex( nY, nx, static_cast<sal_uInt8>(nVal));
                    }
                }
                break;

                case 1 :
                {
                    if ( bByteSwap )
                    {
                        nx = 0;
                        nByteCount = ( nImageWidth >> 3 ) + 1;
                        while ( --nByteCount )
                        {
                            nByteVal = *pt++;
                            pAcc->SetPixelIndex( nY, nx++, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, nx++, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, nx++, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, nx++, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, nx++, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, nx++, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, nx++, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, nx++, nByteVal );
                        }
                        if ( nImageWidth & 7 )
                        {
                            nByteVal = *pt++;
                            while ( nx < nImageWidth )
                            {
                                pAcc->SetPixelIndex( nY, nx++, nByteVal & 1 );
                                nByteVal >>= 1;
                            }
                        }
                    }
                    else
                    {
                        nx = 7;
                        nByteCount = ( nImageWidth >> 3 ) + 1;
                        while ( --nByteCount )
                        {
                            nByteVal = *pt++;
                            pAcc->SetPixelIndex( nY, nx, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, --nx, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, --nx, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, --nx, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, --nx, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, --nx, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, --nx, nByteVal & 1 );
                            nByteVal >>= 1;
                            pAcc->SetPixelIndex( nY, --nx, nByteVal );
                            nx += 15;
                        }
                        if ( nImageWidth & 7 )
                        {
                            nx -= 7;
                            nByteVal = *pt++;
                            nShift = 7;
                            while ( nx < nImageWidth )
                            {
                                pAcc->SetPixelIndex( nY, nx++, ( nByteVal >> nShift ) & 1);
                            }
                        }
                    }
                }
                break;

                default :
                    return sal_False;
            }
        }
    }
    else if ( ( nSamplesPerPixel == 2 ) && ( nBitsPerSample == 8 ) &&
        ( nPlanarConfiguration == 1 ) && ( pColorMap == 0 ) )               // grayscale
    {
        if ( nMaxSampleValue > nMinSampleValue )
        {
            sal_uLong nMinMax = ( ( 1 << 8 /*nDstBitsPerPixel*/ ) - 1 ) / ( nMaxSampleValue - nMinSampleValue );
            sal_uInt8*  pt = pMap[ 0 ];
            for ( nx = 0; nx < nImageWidth; nx++, pt += 2 )
            {
                pAcc->SetPixelIndex( nY, nx, static_cast<sal_uInt8>( ((sal_uLong)*pt - nMinSampleValue) * nMinMax) );
            }
        }
    }
    else
        return sal_False;
    return sal_True;
}

// ---------------------------------------------------------------------------------

void TIFFReader::MakePalCol( void )
{
    if ( nDstBitsPerPixel <= 8 )
    {
        sal_uLong i, nVal, n0RGB;
        if  ( pColorMap == NULL )
            pColorMap = new sal_uLong[ 256 ];
        if ( nPhotometricInterpretation <= 1 )
        {
            nNumColors = 1UL << nBitsPerSample;
            if ( nNumColors > 256 )
                nNumColors = 256;
            pAcc->SetPaletteEntryCount( (sal_uInt16)nNumColors );
            for ( i = 0; i < nNumColors; i++ )
            {
                nVal = ( i * 255 / ( nNumColors - 1 ) ) & 0xff;
                n0RGB = nVal | ( nVal << 8 ) | ( nVal << 16 );
                if ( nPhotometricInterpretation == 1 )
                    pColorMap[ i ] = n0RGB;
                else
                    pColorMap[ nNumColors - i - 1 ] = n0RGB;
            }
        }
        for ( i = 0; i < nNumColors; i++ )
        {
            pAcc->SetPaletteColor( (sal_uInt16)i, BitmapColor( (sal_uInt8)( pColorMap[ i ] >> 16 ),
                (sal_uInt8)( pColorMap[ i ] >> 8 ), (sal_uInt8)pColorMap[ i ] ) );
        }
    }

    if ( fXResolution > 1.0 && fYResolution > 1.0 && ( nResolutionUnit == 2 || nResolutionUnit == 3 ) )
    {
        sal_uLong nRX,nRY;
        if (nResolutionUnit==2)
        {
            nRX=(sal_uLong)(fXResolution+0.5);
            nRY=(sal_uLong)(fYResolution+0.5);
        }
        else
        {
            nRX=(sal_uLong)(fXResolution*2.54+0.5);
            nRY=(sal_uLong)(fYResolution*2.54+0.5);
        }
        MapMode aMapMode(MAP_INCH,Point(0,0),Fraction(1,nRX),Fraction(1,nRY));
        aBitmap.SetPrefMapMode(aMapMode);
        aBitmap.SetPrefSize(Size(nImageWidth,nImageLength));
    }
}

// ---------------------------------------------------------------------------------

void TIFFReader::ReadHeader()
{
    sal_uInt8 nbyte1, nbyte2;
    sal_uInt16 nushort;

    *pTIFF >> nbyte1;
    if ( nbyte1 == 'I' )
        pTIFF->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    else
        pTIFF->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    *pTIFF >> nbyte2 >> nushort;
    if ( nbyte1 != nbyte2 || ( nbyte1 != 'I' && nbyte1 != 'M' ) || nushort != 0x002a )
        bStatus = sal_False;
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

// ---------------------------------------------------------------------------------

sal_Bool TIFFReader::ReadTIFF(SvStream & rTIFF, Graphic & rGraphic )
{
    sal_uInt16  i, nNumTags, nTagType;
    sal_uLong   nMaxPos;
    sal_uLong   nPos;
    sal_uInt32 nFirstIfd, nDataLen;

    bStatus = sal_True;
    nLastPercent = 0;

    pTIFF = &rTIFF;
    nMaxPos = nOrigPos = pTIFF->Tell();
    nOrigNumberFormat = pTIFF->GetNumberFormatInt();

    MayCallback( 0 );

    // read header:
    ReadHeader();

    // read first IFD:
    *pTIFF >> nFirstIfd;

    if( !nFirstIfd || pTIFF->GetError() )
        bStatus = sal_False;

    if ( bStatus )
    {
        sal_uInt32 nOffset = nFirstIfd;

        // calculate length of TIFF file
        do
        {
            pTIFF->Seek( nOrigPos + nOffset );

            if( pTIFF->GetError() )
            {
                pTIFF->ResetError();
                break;
            };
            nMaxPos = std::max( pTIFF->Tell(), nMaxPos );

            *pTIFF >> nNumTags;

            // loop through tags:
            for( i = 0; i < nNumTags; i++ )
            {
                *pTIFF >> nTagType >> nDataType >> nDataLen >> nOffset;

                if( DataTypeSize() * nDataLen > 4 )
                    nMaxPos = std::max( nOrigPos + nOffset + DataTypeSize() * nDataLen, nMaxPos );
            }
            *pTIFF >> nOffset;
            if ( pTIFF->IsEof() )
                nOffset = 0;

            nMaxPos = std::max( pTIFF->Tell(), nMaxPos );
            if ( !nOffset )
                nMaxPos = std::max( pTIFF->Tell(), nMaxPos );
        }
        while( nOffset );

        for ( sal_uInt32 nNextIfd = nFirstIfd; nNextIfd && bStatus; )
        {
            pTIFF->Seek( nOrigPos + nNextIfd );
            {
                bByteSwap = sal_False;

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
                nNumStripOffsets = 0;
                nOrientation = 1;
                nSamplesPerPixel = 1;                       // default value according to the documentation
                nRowsPerStrip = 0xffffffff;                 // default value according to the documentation
                nNumStripByteCounts = 0;
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

                pAcc = NULL;
                pColorMap = NULL;
                pStripOffsets = NULL;
                pStripByteCounts = NULL;
                pMap[ 0 ] = pMap[ 1 ] = pMap[ 2 ] = pMap[ 3 ] = NULL;

                *pTIFF >> nNumTags;
                nPos = pTIFF->Tell();

                // Schleife ueber Tags:
                for( i = 0; i < nNumTags; i++ )
                {
                    *pTIFF >> nTagType >> nDataType >> nDataLen;

                    if( DataTypeSize() * nDataLen > 4 )
                    {
                        *pTIFF >> nOffset;
                        pTIFF->Seek( nOrigPos + nOffset );
                    }
                    ReadTagData( nTagType, nDataLen );
                    nPos += 12; pTIFF->Seek( nPos );

                    if ( pTIFF->GetError() )
                        bStatus = sal_False;

                    if ( bStatus == sal_False )
                        break;
                }
                *pTIFF >> nNextIfd;
                if ( pTIFF->IsEof() )
                    nNextIfd = 0;
            }
            if ( !nBitsPerSample || ( nBitsPerSample > 32 ) )
                bStatus = sal_False;
            if ( bStatus )
            {
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

                Size aTargetSize( nImageWidth, nImageLength );
                aBitmap = Bitmap( aTargetSize, nDstBitsPerPixel );
                pAcc = aBitmap.AcquireWriteAccess();
                if ( pAcc )
                {
                    if ( nPlanarConfiguration == 1 )
                        nPlanes = 1;
                    else
                        nPlanes = nSamplesPerPixel;

                    if ( ( nFillOrder == 2 ) && ( nCompression != 5 ) )     // in the LZW mode bits are already being inverted
                        bByteSwap = sal_True;

                    nStripsPerPlane = ( nImageLength - 1 ) / nRowsPerStrip + 1;
                    nBytesPerRow = ( nImageWidth * nSamplesPerPixel / nPlanes * nBitsPerSample + 7 ) >> 3;

                    for ( sal_uLong j = 0; j < 4; j++ )
                    {
                        try
                        {
                            pMap[ j ] = new sal_uInt8[ nBytesPerRow ];
                        }
                        catch (const std::bad_alloc &)
                        {
                            pMap[ j ] = NULL;
                            bStatus = sal_False;
                            break;
                        }
                    }

                    if (HasAlphaChannel())
                    {
                        pAlphaMask = new AlphaMask( aTargetSize );
                        pMaskAcc = pAlphaMask->AcquireWriteAccess();
                    }

                    if ( bStatus && ReadMap( 10, 60 ) )
                    {
                        nMaxPos = std::max( pTIFF->Tell(), nMaxPos );
                        MakePalCol();
                        nMaxPos = std::max( pTIFF->Tell(), nMaxPos );
                    }
                    else
                        bStatus = sal_False;

                    if( pAcc )
                    {
                        aBitmap.ReleaseAccess( pAcc );

                        if ( pMaskAcc )
                        {
                            if ( pAlphaMask )
                                pAlphaMask->ReleaseAccess( pMaskAcc );
                            pMaskAcc = NULL;
                        }

                        if ( bStatus )
                        {
                            BitmapEx aImage;

                            if (pAlphaMask)
                                aImage = BitmapEx( aBitmap, *pAlphaMask );
                            else
                                aImage = aBitmap;

                            AnimationBitmap aAnimationBitmap( aImage, Point( 0, 0 ), aBitmap.GetSizePixel(),
                                                              ANIMATION_TIMEOUT_ON_CLICK, DISPOSE_BACK );

                            aAnimation.Insert( aAnimationBitmap );
                        }
                    }
                    // Aufraeumen:
                    for ( i = 0; i < 4; i++ )
                        delete[] pMap[ i ];

                    delete[] pColorMap;
                    delete[] pStripOffsets;
                    delete[] pStripByteCounts;
                }
            }
        }
    }

    // seek to end of TIFF if succeeded
    pTIFF->SetNumberFormatInt( nOrigNumberFormat );
    pTIFF->Seek( bStatus ? nMaxPos : nOrigPos );

    if ( aAnimation.Count() )
    {
        if ( aAnimation.Count() == 1 )
            rGraphic = aAnimation.GetBitmapEx();
        else
            rGraphic = aAnimation;  //aBitmap;

        return sal_True;
    }
    else
        return sal_False;
}


//================== GraphicImport - the exported function ================

// this needs to be kept in sync with
// ImpFilterLibCacheEntry::GetImportFunction() from
// vcl/source/filter/graphicfilter.cxx
#if defined(DISABLE_DYNLOADING)
#define GraphicImport itiGraphicImport
#endif

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL
GraphicImport(SvStream & rStream, Graphic & rGraphic, FilterConfigItem*, sal_Bool)
{
    TIFFReader aTIFFReader;

    if ( aTIFFReader.ReadTIFF( rStream, rGraphic ) == sal_False )
        return sal_False;

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
