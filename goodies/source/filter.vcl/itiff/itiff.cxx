/*************************************************************************
 *
 *  $RCSfile: itiff.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-08 15:52:53 $
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

#ifndef _NEW_HXX
#include <tools/new.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SV_FLTCALL_HXX
#include <svtools/fltcall.hxx>
#endif
#ifndef _SV_ANIMATE_HXX
#include <vcl/animate.hxx>
#endif
#include "lzwdecom.hxx"
#include "ccidecom.hxx"

#define OOODEBUG(str,Num) //(InfoBox(NULL,String(str)+String(" ")+String(Num)).Execute();

#define BYTESWAP( nByte ) ( nByte << 7 ) | ( ( nByte & 2 ) << 5 ) | ( ( nByte & 4 ) << 3 ) | ( ( nByte & 8 ) << 1 ) | \
        ( ( nByte & 16 ) >> 1 ) | ( ( nByte & 32 ) >> 3 ) | ( ( nByte & 64 ) >> 5 ) | ( ( nByte & 128 ) >> 7 )


//============================ TIFFReader ==================================

class TIFFReader
{

private:

    BOOL                bStatus;                    // Ob bisher kein Fehler auftrat
    Animation           aAnimation;

    PFilterCallback     pCallback;
    void*               pCallerData;
    ULONG               nLastPercent;

    SvStream*           pTIFF;                      // Die einzulesende TIFF-Datei
    Bitmap              aBitmap;
    BitmapWriteAccess*  pAcc;
    USHORT              nDstBitsPerPixel;

    ULONG               nOrigPos;                   // Anfaengliche Position in pTIFF
    UINT16              nOrigNumberFormat;          // Anfaengliches Nummern-Format von pTIFF


    UINT16              nDataType;
    // Daten, die aus dem TIFF-Tags entnommen werden:
    BOOL                bByteSwap;                  // TRUE wenn bits 0..7 -> 7..0 invertiert werden sollen ( FILLORDER = 2 );
    BYTE                nByte1;                     // 'I', wenn Format LittleEndian

    ULONG               nNewSubFile;                //
    ULONG               nSubFile;                   //
    ULONG               nImageWidth;                // Bildbreite in Pixel
    ULONG               nImageLength;               // Bildhoehe in Pixel
    ULONG               nBitsPerSample;             // Bits pro Pixel pro Ebene
    ULONG               nCompression;               // Art der Kompriemierung
    ULONG               nPhotometricInterpretation; //
    ULONG               nThresholding;              //
    ULONG               nCellWidth;                 //
    ULONG               nCellLength;                //
    ULONG               nFillOrder;                 //
    ULONG*              pStripOffsets;              // Feld von Offsets zu den Bitmap-Daten-"Strips"
    ULONG               nNumStripOffsets;           // Groesse obigen Feldes
    ULONG               nOrientation;               //
    ULONG               nSamplesPerPixel;           // Anzahl der Ebenen
    ULONG               nRowsPerStrip;              // Wenn nicht komprimiert: Zahl der Zeilen pro Strip
    ULONG*              pStripByteCounts;           // Wenn komprimiert (bestimmte Art): Groesse der Strips
    ULONG               nNumStripByteCounts;        // Anzahl der Eintraege in obiges Feld
    ULONG               nMinSampleValue;            //
    ULONG               nMaxSampleValue;            //
    double              fXResolution;               // X-Aufloesung oder 0.0
    double              fYResolution;               // Y-Aufloesung oder 0.0
    ULONG               nPlanarConfiguration;       //
    ULONG               nGroup3Options;             //
    ULONG               nGroup4Options;             //
    ULONG               nResolutionUnit;            // Einheit von fX/YResolution: 1=unbekannt, 2(default)=Zoll, 3=cm
    ULONG               nPredictor;                 //
    ULONG*              pColorMap;                  // Farb-Palette
    ULONG               nNumColors;                 // Anzahl Farben in der Farbpalette

    ULONG               nPlanes;                    // Anzahl der Ebenen in der Tiff-Datei
    ULONG               nStripsPerPlane;            // Anzahl der Strips pro Ebene
    ULONG               nBytesPerRow;               // Bytes pro Zeile pro Ebene in der Tiff-Datei ( unkomprimiert )
    BYTE*               pMap[ 4 ];                  // Temporaere Scanline


    void    MayCallback( ULONG nPercent );

    ULONG   DataTypeSize();
    ULONG   ReadIntData();
    double  ReadDoubleData();

    void    ReadHeader();
    void    ReadTagData( USHORT nTagType, ULONG nDataLen );

    BOOL    ReadMap( ULONG nMinPercent, ULONG nMaxPercent );
        // Liesst/dekomprimert die Bitmap-Daten, und fuellt pMap

    ULONG   GetBits( const BYTE * pSrc, ULONG nBitsPos, ULONG nBitsCount );
        // Holt nBitsCount Bits aus pSrc[..] an der Bit-Position nBitsPos

    void    MakePalCol( void );
        // Erzeugt die Bitmap aus der temporaeren Bitmap pMap
        // und loescht dabei pMap teilweise
    BOOL    ConvertScanline( ULONG nY );
        // Konvertiert eine Scanline in das Windows-BMP-Format

public:

    TIFFReader() {}
    ~TIFFReader() {}

    BOOL ReadTIFF( SvStream & rTIFF, Graphic & rGraphic, PFilterCallback pcallback, void * pcallerdata );
};

//=================== Methoden von TIFFReader ==============================

void TIFFReader::MayCallback( ULONG nPercent )
{
    if ( nPercent >= nLastPercent + 3 )
    {
        nLastPercent=nPercent;
        if ( pCallback != NULL && nPercent <= 100 && bStatus == TRUE )
        {
            if (((*pCallback)(pCallerData,(USHORT)nPercent)) == TRUE )
                bStatus = FALSE;
        }
    }
}

// ---------------------------------------------------------------------------------

ULONG TIFFReader::DataTypeSize()
{
    ULONG nSize;
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

ULONG TIFFReader::ReadIntData()
{
    double  nDOUBLE;
    float   nFLOAT;
    UINT32  nUINT32a, nUINT32b;
    INT32   nINT32;
    UINT16  nUINT16;
    INT16   nINT16;
    BYTE    nBYTE;
    char    nCHAR;

    switch( nDataType )
    {
        case 0 :    //??
        case 1 :
        case 2 :
        case 7 :
            *pTIFF >> nBYTE;
            nUINT32a = (ULONG)nBYTE;
        break;
        case 3 :
             *pTIFF >> nUINT16;
             nUINT32a = (ULONG)nUINT16;
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
            nUINT32a = (INT32)nCHAR;
        break;
        case 8 :
            *pTIFF >> nINT16;
            nUINT32a = (INT32)nINT16;
        break;
        case 10 :
            *pTIFF >> nUINT32a >> nINT32;
            if ( nINT32 != 0 )
                nUINT32a /= nINT32;
        break;
        case 11 :
            *pTIFF >> nFLOAT;
            nUINT32a = (INT32)nFLOAT;
        break;
        case 12 :
            *pTIFF >> nDOUBLE;
            nUINT32a = (INT32)nDOUBLE;
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
    ULONG   nulong;
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

void TIFFReader::ReadTagData( USHORT nTagType, ULONG nDataLen)
{
    if ( bStatus == FALSE )
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
            ULONG nOldNumSO, i, * pOldSO;
            pOldSO = pStripOffsets;
            if ( pOldSO == NULL ) nNumStripOffsets = 0; // Sicherheitshalber
            nOldNumSO = nNumStripOffsets;
            nNumStripOffsets += nDataLen;
            pStripOffsets = new ULONG[ nNumStripOffsets ];
            for ( i = 0; i < nOldNumSO; i++ )
                pStripOffsets[ i ] = pOldSO[ i ] + nOrigPos;
            for ( i = nOldNumSO; i < nNumStripOffsets; i++ )
                pStripOffsets[ i ] = ReadIntData() + nOrigPos;
            if ( pOldSO != NULL )
                delete pOldSO;
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
            ULONG nOldNumSBC, i, * pOldSBC;
            pOldSBC = pStripByteCounts;
            if ( pOldSBC == NULL ) nNumStripByteCounts = 0; // Sicherheitshalber
            nOldNumSBC = nNumStripByteCounts;
            nNumStripByteCounts += nDataLen;
            pStripByteCounts = new ULONG[ nNumStripByteCounts ];
            for ( i = 0; i < nOldNumSBC; i++ )
                pStripByteCounts[ i ] = pOldSBC[ i ];
            for ( i = nOldNumSBC; i < nNumStripByteCounts; i++)
                pStripByteCounts[ i ] = ReadIntData();
            if ( pOldSBC != NULL )
                delete pOldSBC;
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
            USHORT nVal;
            ULONG i;
            nNumColors= ( 1 << nBitsPerSample );
            if ( nDataType == 3 && nNumColors <= 256)
            {
                pColorMap = new ULONG[ 256 ];
                for ( i = 0; i < nNumColors; i++ )
                    pColorMap[ i ] = 0;
                for ( i = 0; i < nNumColors; i++ )
                {
                    *pTIFF >> nVal;
                    pColorMap[ i ] |= ( ( (ULONG)nVal ) << 8 ) & 0x00ff0000;
                }
                for ( i = 0; i < nNumColors; i++ )
                {
                    *pTIFF >> nVal;
                    pColorMap[ i ] |= ( (ULONG)nVal ) & 0x0000ff00;
                }
                for ( i = 0; i < nNumColors; i++ )
                {
                    *pTIFF >> nVal;
                    pColorMap[ i ] |= ( ( (ULONG)nVal ) >> 8 ) & 0x000000ff;
                }
            }
            else
                bStatus = FALSE;
            OOODEBUG("ColorMap (Anzahl Farben:)", nNumColors);
            break;
        }
    }

    if ( pTIFF->GetError() )
        bStatus = FALSE;
}

// ---------------------------------------------------------------------------------

BOOL TIFFReader::ReadMap( ULONG nMinPercent, ULONG nMaxPercent )
{
    if ( nCompression == 1 || nCompression == 32771 )
    {
        ULONG ny, np, nStrip, nStripBytesPerRow;

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
                    return FALSE;
                pTIFF->Seek( pStripOffsets[ nStrip ] + ( ny % nRowsPerStrip ) * nStripBytesPerRow );
                pTIFF->Read( pMap[ np ], nBytesPerRow );
                if ( pTIFF->GetError() )
                    return FALSE;
                MayCallback( nMinPercent + ( nMaxPercent - nMinPercent ) * ( np * nImageLength + ny) / ( nImageLength * nPlanes ) );
            }
            if ( !ConvertScanline( ny ) )
                return FALSE;
        }
    }
    else if ( nCompression == 2 || nCompression == 3 || nCompression == 4 )
    {
        ULONG ny, np, nStrip, nOptions;
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
                return FALSE;
        }
        else
        {   // nCompression==4
            nOptions = CCI_OPTION_2D;
            if ( nGroup4Options & 0xffffffff )
                return FALSE;
        }
        if ( nFillOrder == 2 )
        {
            nOptions |= CCI_OPTION_INVERSEBITORDER;
            bByteSwap = FALSE;
        }
        nStrip = 0;
        if ( nStrip >= nNumStripOffsets )
            return FALSE;
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
                        return FALSE;
                    pTIFF->Seek( pStripOffsets[ nStrip ] );
                    aCCIDecom.StartDecompression( *pTIFF );
                }
                if ( aCCIDecom.DecompressScanline( pMap[ np ], nImageWidth * nBitsPerSample * nSamplesPerPixel / nPlanes ) == FALSE )
                    return FALSE;
                if ( pTIFF->GetError() )
                    return FALSE;
                MayCallback(nMinPercent+(nMaxPercent-nMinPercent)*(np*nImageLength+ny)/(nImageLength*nPlanes));
            }
            if ( !ConvertScanline( ny ) )
                return FALSE;
        }
    }
    else if ( nCompression == 5 )
    {
        LZWDecompressor aLZWDecom;
        ULONG ny, np, nStrip;
        nStrip=0;
        if ( nStrip >= nNumStripOffsets )
            return FALSE;
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
                        return FALSE;
                    pTIFF->Seek(pStripOffsets[nStrip]);
                    aLZWDecom.StartDecompression(*pTIFF);
                }
                if ( ( aLZWDecom.Decompress( pMap[ np ], nBytesPerRow ) != nBytesPerRow ) || pTIFF->GetError() )
                    return FALSE;
                MayCallback(nMinPercent+(nMaxPercent-nMinPercent)*(np*nImageLength+ny)/(nImageLength*nPlanes));
            }
            if ( !ConvertScanline( ny ) )
                return FALSE;
        }
    }
    else if ( nCompression == 32773 )
    {
        ULONG nStrip,nRecCount,nRowBytesLeft,ny,np,i;
        BYTE * pdst, nRecHeader, nRecData;
        nStrip = 0;
        if ( nStrip >= nNumStripOffsets )
            return FALSE;
        pTIFF->Seek(pStripOffsets[nStrip]);
        for ( ny = 0; ny < nImageLength; ny++ )
        {
            for ( np = 0; np < nPlanes; np++ )
            {
                if ( ny / nRowsPerStrip + np * nStripsPerPlane > nStrip )
                {
                    nStrip=ny/nRowsPerStrip+np*nStripsPerPlane;
                    if ( nStrip >= nNumStripOffsets )
                        return FALSE;
                    pTIFF->Seek(pStripOffsets[nStrip]);
                }
                nRowBytesLeft = nBytesPerRow;
                pdst=pMap[ np ];
                do
                {
                    *pTIFF >> nRecHeader;
                    if ((nRecHeader&0x80)==0)
                    {
                        nRecCount=0x00000001+((ULONG)nRecHeader);
                        if ( nRecCount > nRowBytesLeft )
                            return FALSE;
                        pTIFF->Read(pdst,nRecCount);
                        pdst+=nRecCount;
                        nRowBytesLeft-=nRecCount;
                    }
                    else if ( nRecHeader != 0x80 )
                    {
                        nRecCount = 0x000000101 - ( (ULONG)nRecHeader );
                        if ( nRecCount > nRowBytesLeft )
                        {
                            nRecCount = nRowBytesLeft;

//                          bStatus = FALSE;
//                          return;

                        }
                        *pTIFF >> nRecData;
                        for ( i = 0; i < nRecCount; i++ )
                            *(pdst++) = nRecData;
                        nRowBytesLeft -= nRecCount;
                    }
                } while ( nRowBytesLeft != 0 );
                if ( pTIFF->GetError() )
                    return FALSE;
                MayCallback(nMinPercent+(nMaxPercent-nMinPercent)*(np*nImageLength+ny)/(nImageLength*nPlanes));
            }
            if ( !ConvertScanline( ny ) )
                return FALSE;
        }
    }
    else
        return FALSE;
    return TRUE;
}

ULONG TIFFReader::GetBits( const BYTE * pSrc, ULONG nBitsPos, ULONG nBitsCount )
{
    ULONG nRes;
    if ( bByteSwap )
    {
        pSrc += ( nBitsPos >> 3 );
        nBitsPos &= 7;
        BYTE nDat = *pSrc;
        nRes = (ULONG)( BYTESWAP( nDat ) & ( 0xff >> nBitsPos ) );

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
                nRes = ( nRes << 8 ) | ((ULONG)BYTESWAP( nDat ) );
                nBitsCount -= 8;
            }
            if ( nBitsCount > 0 )
            {
                nDat = *pSrc;
                nRes = ( nRes << nBitsCount ) | (((ULONG)BYTESWAP(nDat))>>(8-nBitsCount));
            }
        }
    }
    else
    {
        pSrc += ( nBitsPos >> 3 );
        nBitsPos &= 7;
        nRes = (ULONG)((*pSrc)&(0xff>>nBitsPos));
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
                nRes = ( nRes << 8 ) | ((ULONG)*(pSrc++));
                nBitsCount -= 8;
            }
            if ( nBitsCount > 0 )
                nRes = ( nRes << nBitsCount ) | (((ULONG)*pSrc)>>(8-nBitsCount));
        }
    }
    return nRes;
}

// ---------------------------------------------------------------------------------

BOOL TIFFReader::ConvertScanline( ULONG nY )
{
    UINT32  nRed, nGreen, nBlue, ns, nx, nVal, nByteCount;
    BYTE    nByteVal;

    if ( nDstBitsPerPixel == 24 )
    {
        if ( nBitsPerSample == 8 && nSamplesPerPixel >= 3 &&
             nPlanes == 1 && nPhotometricInterpretation == 2 )
        {
            BYTE*   pt = pMap[ 0 ];

            // sind die Werte als Differenz abgelegt?
            if ( 2 == nPredictor )
            {
                BYTE  nLRed = 0;
                BYTE  nLGreen = 0;
                BYTE  nLBlue = 0;
                for ( nx = 0; nx < nImageWidth; nx++, pt += nSamplesPerPixel )
                {
                    pAcc->SetPixel( nY, nx, Color( ( nLRed+=pt[ 0 ] ), ( nLGreen += pt[ 1 ] ), ( nLBlue += pt[ 2 ] ) ) );
                }
            }
            else
            {
                for ( nx = 0; nx < nImageWidth; nx++, pt += nSamplesPerPixel )
                {
                    pAcc->SetPixel( nY, nx, Color( pt[0], pt[1], pt[2] ) );
                }
            }
        }
        else if ( nPhotometricInterpretation == 2 && nSamplesPerPixel >= 3 )
        {
            ULONG nMinMax = nMinSampleValue * 255 / ( nMaxSampleValue - nMinSampleValue );
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
                pAcc->SetPixel( nY, nx, Color( (BYTE)( nRed - nMinMax ), (BYTE)( nGreen - nMinMax ), (BYTE)(nBlue - nMinMax) ) );
            }
        }
        else if ( nPhotometricInterpretation == 5 && nSamplesPerPixel == 3 )
        {
            ULONG nMinMax =  nMinSampleValue * 255 / ( nMaxSampleValue - nMinSampleValue );
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
                nRed = 255 - (BYTE)( nRed - nMinMax );
                nGreen = 255 - (BYTE)( nGreen - nMinMax );
                nBlue = 255 - (BYTE)( nBlue - nMinMax );
                pAcc->SetPixel( nY, nx, Color( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue ) );
            }
        }
        else if( nPhotometricInterpretation == 5 && nSamplesPerPixel == 4 )
        {
            BYTE    nSamp[ 4 ];
            BYTE    nSampLast[ 4 ] = { 0, 0, 0, 0 };
            long    nBlack;

            for( nx = 0; nx < nImageWidth; nx++ )
            {
                // sind die Werte als Differenz abgelegt?
                if( 2 == nPredictor )
                {
                    for( ns = 0; ns < 4; ns++ )
                    {
                        if( nPlanes < 3 )
                            nSamp[ ns ] = ( nSampLast[ ns ] += (BYTE) GetBits( pMap[ 0 ], ( nx * nSamplesPerPixel + ns ) * nBitsPerSample, nBitsPerSample ) );
                        else
                            nSamp[ ns ] = ( nSampLast[ ns ] += (BYTE) GetBits( pMap[ ns ], nx * nBitsPerSample, nBitsPerSample ) );
                    }
                }
                else
                {
                    for( ns = 0; ns < 4; ns++ )
                    {
                        if( nPlanes < 3 )
                            nSamp[ ns ] = (BYTE) GetBits( pMap[ 0 ], ( nx * nSamplesPerPixel + ns ) * nBitsPerSample, nBitsPerSample );
                        else
                            nSamp[ ns ]= (BYTE) GetBits( pMap[ ns ], nx * nBitsPerSample, nBitsPerSample );
                    }
                }
                nBlack = nSamp[ 3 ];
                nRed = (BYTE) Max( 0L, 255L - ( ( (long) nSamp[ 0 ] + nBlack - ( ( (long) nMinSampleValue ) << 1 ) ) *
                            255L/(long)(nMaxSampleValue-nMinSampleValue) ) );
                nGreen = (BYTE) Max( 0L, 255L - ( ( (long) nSamp[ 1 ] + nBlack - ( ( (long) nMinSampleValue ) << 1 ) ) *
                            255L/(long)(nMaxSampleValue-nMinSampleValue) ) );
                nBlue = (BYTE) Max( 0L, 255L - ( ( (long) nSamp[ 2 ] + nBlack - ( ( (long) nMinSampleValue ) << 1 ) ) *
                            255L/(long)(nMaxSampleValue-nMinSampleValue) ) );
                pAcc->SetPixel( nY, nx, Color ( (BYTE)nRed, (BYTE)nGreen, (BYTE)nBlue ) );

            }
        }
    }
    else if ( nSamplesPerPixel == 1 && ( nPhotometricInterpretation <= 1 || nPhotometricInterpretation == 3 ) )
    {
        ULONG nMinMax = ( ( 1 << nDstBitsPerPixel ) - 1 ) / ( nMaxSampleValue - nMinSampleValue );
        BYTE* pt = pMap[ 0 ];
        BYTE nShift;

        switch ( nDstBitsPerPixel )
        {
            case 8 :
            {
                BYTE nLast;
                if ( bByteSwap )
                {
                    if ( nPredictor == 2 )
                    {
                        nLast = BYTESWAP( (BYTE)*pt++ );
                        for ( nx = 0; nx < nImageWidth; nx++ )
                        {
                            pAcc->SetPixel( nY, nx, nLast );
                            nLast += *pt++;
                        }
                    }
                    else
                    {
                        for ( nx = 0; nx < nImageWidth; nx++ )
                        {
                            nLast = *pt++;
                            pAcc->SetPixel( nY, nx, (BYTE)( ( (BYTESWAP((ULONG)nLast ) - nMinSampleValue ) * nMinMax ) ) );
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
                            pAcc->SetPixel( nY, nx, nLast );
                            nLast += *pt++;
                        }
                    }
                    else
                    {
                        for ( nx = 0; nx < nImageWidth; nx++ )
                        {
                            pAcc->SetPixel( nY, nx, (BYTE)( ( (ULONG)*pt++ - nMinSampleValue ) * nMinMax ) );

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
                    pAcc->SetPixel( nY, nx, (BYTE)nVal );
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
                        pAcc->SetPixel( nY, nx++, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, nx++, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, nx++, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, nx++, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, nx++, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, nx++, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, nx++, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, nx++, nByteVal );
                    }
                    if ( nImageWidth & 7 )
                    {
                        nByteVal = *pt++;
                        while ( nx < nImageWidth )
                        {
                            pAcc->SetPixel( nY, nx++, nByteVal & 1 );
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
                        pAcc->SetPixel( nY, nx, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, --nx, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, --nx, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, --nx, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, --nx, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, --nx, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, --nx, nByteVal & 1 );
                        nByteVal >>= 1;
                        pAcc->SetPixel( nY, --nx, nByteVal );
                        nx += 15;
                    }
                    if ( nImageWidth & 7 )
                    {
                        nx -= 7;
                        nByteVal = *pt++;
                        nShift = 7;
                        while ( nx < nImageWidth )
                        {
                            pAcc->SetPixel( nY, nx++, ( nByteVal >> nShift ) & 1);
                        }
                    }
                }
            }
            break;

            default :
                return FALSE;
        }
    }
    else if ( ( nSamplesPerPixel == 2 ) && ( nBitsPerSample == 8 ) &&
        ( nPlanarConfiguration == 1 ) && ( pColorMap == 0 ) )               // grayscale
    {
        ULONG nMinMax = ( ( 1 << 8 /*nDstBitsPerPixel*/ ) - 1 ) / ( nMaxSampleValue - nMinSampleValue );
        BYTE*   pt = pMap[ 0 ];
        if ( nByte1 == 'I' )
            pt++;
        for ( nx = 0; nx < nImageWidth; nx++, pt += 2 )
        {
            pAcc->SetPixel( nY, nx, (BYTE)( ( (ULONG)*pt - nMinSampleValue ) * nMinMax ) );
        }
    }
    else
        return FALSE;
    return TRUE;
}

// ---------------------------------------------------------------------------------

void TIFFReader::MakePalCol( void )
{
    if ( nDstBitsPerPixel <= 8 )
    {
        ULONG i, nVal, n0RGB;
        if  ( pColorMap == NULL )
            pColorMap = new ULONG[ 256 ];
        if ( nPhotometricInterpretation <= 1 )
        {
            nNumColors = 1 << nBitsPerSample;
            if ( nNumColors > 256 )
                nNumColors = 256;
            pAcc->SetPaletteEntryCount( (USHORT)nNumColors );
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
            pAcc->SetPaletteColor( (USHORT)i, BitmapColor( (BYTE)( pColorMap[ i ] >> 16 ),
                (BYTE)( pColorMap[ i ] >> 8 ), (BYTE)pColorMap[ i ] ) );
        }
    }

    if ( fXResolution > 1.0 && fYResolution > 1.0 && ( nResolutionUnit == 2 || nResolutionUnit == 3 ) )
    {
        ULONG nRX,nRY;
        if (nResolutionUnit==2)
        {
            nRX=(ULONG)(fXResolution+0.5);
            nRY=(ULONG)(fYResolution+0.5);
        }
        else
        {
            nRX=(ULONG)(fXResolution*2.54+0.5);
            nRY=(ULONG)(fYResolution*2.54+0.5);
        }
        MapMode aMapMode(MAP_INCH,Point(0,0),Fraction(1,nRX),Fraction(1,nRY));
        aBitmap.SetPrefMapMode(aMapMode);
        aBitmap.SetPrefSize(Size(nImageWidth,nImageLength));
    }
}

// ---------------------------------------------------------------------------------

void TIFFReader::ReadHeader()
{
    BYTE nbyte1, nbyte2;
    USHORT nushort;

    *pTIFF >> nbyte1;
    if ( nbyte1 == 'I' )
        pTIFF->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    else
        pTIFF->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    *pTIFF >> nbyte2 >> nushort;
    if ( nbyte1 != nbyte2 || ( nbyte1 != 'I' && nbyte1 != 'M' ) || nushort != 0x002a )
        bStatus = FALSE;
}

// ---------------------------------------------------------------------------------

BOOL TIFFReader::ReadTIFF(SvStream & rTIFF, Graphic & rGraphic,
                          PFilterCallback pcallback, void * pcallerdata)
{
    USHORT  i, nNumTags, nTagType;
    ULONG   nMaxPos;
    ULONG   nFirstIfd, nDataLen, nPos;
    ULONG*  pDummy = new ULONG; delete pDummy;  // damit unter OS/2
                                                // das richtige (Tools-)new
                                                // verwendet wird, da es sonst
                                                // in dieser DLL nur Vector-news
                                                // gibt;
    bStatus = TRUE;

    pCallback    = pcallback;
    pCallerData  = pcallerdata;
    nLastPercent = 0;

    pTIFF = &rTIFF;
    nMaxPos = nOrigPos = pTIFF->Tell();
    nOrigNumberFormat = pTIFF->GetNumberFormatInt();

    MayCallback( 0 );

    // Kopf einlesen:
    ReadHeader();

    // Ersten IFD einlesen:
    *pTIFF >> nFirstIfd;

    if( !nFirstIfd || pTIFF->GetError() )
        bStatus = FALSE;

    if ( bStatus )
    {
        ULONG nOffset = nFirstIfd;

        // calculate length of TIFF file
        do
        {
            pTIFF->Seek( nOrigPos + nOffset );

            if( pTIFF->GetError() )
            {
                pTIFF->ResetError();
                break;
            };
            nMaxPos = Max( pTIFF->Tell(), nMaxPos );

            *pTIFF >> nNumTags;

            // Schleife ueber Tags:
            for( i = 0; i < nNumTags; i++ )
            {
                *pTIFF >> nTagType >> nDataType >> nDataLen >> nOffset;

                if( DataTypeSize() * nDataLen > 4 )
                    nMaxPos = Max( nOrigPos + nOffset + DataTypeSize() * nDataLen, nMaxPos );
            }
            *pTIFF >> nOffset;

            nMaxPos = Max( pTIFF->Tell(), nMaxPos );
            if ( !nOffset )
                nMaxPos = Max( pTIFF->Tell(), nMaxPos );
        }
        while( nOffset );

        for ( UINT32 nNextIfd = nFirstIfd; nNextIfd && bStatus; )
        {
            pTIFF->Seek( nOrigPos + nNextIfd );
            {
                bByteSwap = FALSE;

                nNewSubFile = 0;
                nSubFile = 0;
                nImageWidth = 0;
                nImageLength = 0;
                nBitsPerSample = 1;                         // Defaultwert laut Doku
                nCompression = 1;
                nPhotometricInterpretation = 0;
                nThresholding = 1;                          // Defaultwert laut Doku
                nCellWidth = 1;
                nCellLength = 1;
                nFillOrder = 1;                             // Defaultwert laut Doku
                nNumStripOffsets = 0;
                nOrientation = 1;
                nSamplesPerPixel = 1;                       // Defaultwert laut Doku
                nRowsPerStrip = 0xffffffff;                 // Defaultwert laut Doku
                nNumStripByteCounts = 0;
                nMinSampleValue = 0;                        // Defaultwert laut Doku
                nMaxSampleValue = 0;
                fXResolution = 0.0;
                fYResolution = 0.0;
                nPlanarConfiguration = 1;
                nGroup3Options = 0;                         // Defaultwert laut Doku
                nGroup4Options = 0;                         // Defaultwert laut Doku
                nResolutionUnit = 2;                        // Defaultwert laut Doku
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
                        bStatus = FALSE;

                    if ( bStatus == FALSE )
                        break;
                }
                *pTIFF >> nNextIfd;
            }
            if ( bStatus )
            {
                if ( nMaxSampleValue == 0 )
                    nMaxSampleValue = ( 1 << nBitsPerSample ) - 1;

                if ( nPhotometricInterpretation == 2 || nPhotometricInterpretation == 5 || nPhotometricInterpretation == 6 )
                    nDstBitsPerPixel = 24;
                else if ( nBitsPerSample*nSamplesPerPixel <= 1 )
                    nDstBitsPerPixel = 1;
                else if ( nBitsPerSample*nSamplesPerPixel <= 4 )
                    nDstBitsPerPixel = 4;
                else
                    nDstBitsPerPixel = 8;

                aBitmap = Bitmap( Size( nImageWidth, nImageLength ), nDstBitsPerPixel );
                if ( ( pAcc = aBitmap.AcquireWriteAccess() ) )
                {
                    if ( nPlanarConfiguration == 1 )
                        nPlanes = 1;
                    else
                        nPlanes = nSamplesPerPixel;

                    if ( ( nFillOrder == 2 ) && ( nCompression != 5 ) )     // im LZW Mode werden die bits schon invertiert
                        bByteSwap = TRUE;

                    nStripsPerPlane = ( nImageLength - 1 ) / nRowsPerStrip + 1;
                    nBytesPerRow = ( nImageWidth * nSamplesPerPixel / nPlanes * nBitsPerSample + 7 ) >> 3;

                    for ( ULONG i = 0; i < 4; i++ )
                        pMap[ i ] = new BYTE[ nBytesPerRow ];

                    if ( ReadMap( 10, 60 ) )
                    {
                        nMaxPos = Max( pTIFF->Tell(), nMaxPos );
                        MakePalCol();
                        nMaxPos = Max( pTIFF->Tell(), nMaxPos );
                    }
                    else
                        bStatus = FALSE;

                    if( pAcc )
                    {
                        aBitmap.ReleaseAccess( pAcc );
                        if ( bStatus )
                        {
                            AnimationBitmap aAnimationBitmap( aBitmap, Point( 0, 0 ), aBitmap.GetSizePixel(),
                                                              ANIMATION_TIMEOUT_ON_CLICK, DISPOSE_FULL );

                            aAnimation.Insert( aAnimationBitmap );
                        }
                    }
                    // Aufraeumen:
                    for ( i = 0; i < 4; i++ )
                        delete[] pMap[ i ];

                    delete pColorMap;
                    delete pStripOffsets;
                    delete pStripByteCounts;
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

        return TRUE;
    }
    else
        return FALSE;
}


//================== GraphicImport - die exportierte Funktion ================

#ifdef WNT
extern "C" BOOL _cdecl GraphicImport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData, FilterConfigItem*, BOOL)
#else
extern "C" BOOL GraphicImport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData, FilterConfigItem*, BOOL)
#endif
{
    TIFFReader aTIFFReader;

    if (aTIFFReader.ReadTIFF( rStream, rGraphic, pCallback, pCallerData ) == FALSE )
        return FALSE;

    return TRUE;
}

//============================= fuer Windows ==================================

#pragma hdrstop

#ifdef WIN

static HINSTANCE hDLLInst = 0;      // HANDLE der DLL

extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
#ifndef WNT
    if ( nHeap )
        UnlockData( 0 );
#endif

    hDLLInst = hDLL;

    return TRUE;
}

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif


