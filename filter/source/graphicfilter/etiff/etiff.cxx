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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/bmpacc.hxx>
#include <svl/solar.hrc>
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>

#define NewSubfileType              254
#define ImageWidth                  256
#define ImageLength                 257
#define BitsPerSample               258
#define Compression                 259
#define PhotometricInterpretation   262
#define StripOffsets                273
#define SamplesPerPixel             277
#define RowsPerStrip                278
#define StripByteCounts             279
#define XResolution                 282
#define YResolution                 283
#define PlanarConfiguration         284
#define ResolutionUnit              296
#define ColorMap                    320
#define ReferenceBlackWhite         532

// -------------
// - TIFFWriter -
// -------------

struct TIFFLZWCTreeNode
{

    TIFFLZWCTreeNode*   pBrother;       // naechster Knoten, der den selben Vater hat
    TIFFLZWCTreeNode*   pFirstChild;    // erster Sohn
    sal_uInt16              nCode;          // Der Code fuer den String von Pixelwerten, der sich ergibt, wenn
    sal_uInt16              nValue;         // Der Pixelwert
};

class TIFFWriter
{
private:

    SvStream*           mpOStm;
    sal_uInt32              mnStreamOfs;

    sal_Bool                mbStatus;
    BitmapReadAccess*   mpAcc;

    sal_uInt32              mnWidth, mnHeight, mnColors;
    sal_uInt32              mnCurAllPictHeight;
    sal_uInt32              mnSumOfAllPictHeight;
    sal_uInt32              mnBitsPerPixel;
    sal_uInt32              mnLastPercent;

    sal_uInt32              mnLatestIfdPos;
    sal_uInt16              mnTagCount;                 // number of tags already written
    sal_uInt32              mnCurrentTagCountPos;       // offset to the position where the current
                                                    // tag count is to insert

    sal_uInt32              mnXResPos;                  // if != 0 this DWORDs stores the
    sal_uInt32              mnYResPos;                  // actual streamposition of the
    sal_uInt32              mnPalPos;                   // Tag Entry
    sal_uInt32              mnBitmapPos;
    sal_uInt32              mnStripByteCountPos;

    TIFFLZWCTreeNode*   pTable;
    TIFFLZWCTreeNode*   pPrefix;
    sal_uInt16              nDataSize;
    sal_uInt16              nClearCode;
    sal_uInt16              nEOICode;
    sal_uInt16              nTableSize;
    sal_uInt16              nCodeSize;
    sal_uLong               nOffset;
    sal_uLong               dwShift;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    void                ImplCallback( sal_uInt32 nPercent );
    sal_Bool                ImplWriteHeader( sal_Bool bMultiPage );
    void                ImplWritePalette();
    sal_Bool                ImplWriteBody();
    void                ImplWriteTag( sal_uInt16 TagID, sal_uInt16 DataType, sal_uInt32 NumberOfItems, sal_uInt32 Value);
    void                ImplWriteResolution( sal_uLong nStreamPos, sal_uInt32 nResolutionUnit );
    void                StartCompression();
    void                Compress( sal_uInt8 nSrc );
    void                EndCompression();
    inline void         WriteBits( sal_uInt16 nCode, sal_uInt16 nCodeLen );

public:

                        TIFFWriter();
                        ~TIFFWriter();

    sal_Bool                WriteTIFF( const Graphic& rGraphic, SvStream& rTIFF, FilterConfigItem* pFilterConfigItem );
};

// ------------------------------------------------------------------------

TIFFWriter::TIFFWriter() :
        mbStatus            ( sal_True ),
        mpAcc               ( NULL ),
        mnCurAllPictHeight  ( 0 ),
        mnSumOfAllPictHeight( 0 ),
        mnLastPercent       ( 0 ),
        mnXResPos           ( 0 ),
        mnYResPos           ( 0 ),
        mnBitmapPos         ( 0 ),
        mnStripByteCountPos ( 0 )
{
}

// ------------------------------------------------------------------------

TIFFWriter::~TIFFWriter()
{
}

// ------------------------------------------------------------------------

sal_Bool TIFFWriter::WriteTIFF( const Graphic& rGraphic, SvStream& rTIFF, FilterConfigItem* pFilterConfigItem)
{
    sal_uLong*  pDummy = new sal_uLong; delete pDummy; // damit unter OS/2
                                               // das richtige (Tools-)new
                                               // verwendet wird, da es sonst
                                               // in dieser DLL nur Vector-news
                                               // gibt;

    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            rtl::OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    // #i69169# copy stream
    mpOStm = &rTIFF;

    const sal_uInt16    nOldFormat = mpOStm->GetNumberFormatInt();
    mnStreamOfs = mpOStm->Tell();

    // we will use the BIG Endian Mode
    // TIFF header
    mpOStm->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    *mpOStm << (sal_uInt32)0x4d4d002a;      // TIFF identifier
    mnLatestIfdPos = mpOStm->Tell();
    *mpOStm << (sal_uInt32)0;

    Animation   aAnimation;
    Bitmap      aBmp;

    if( mbStatus )
    {
        if ( rGraphic.IsAnimated() )
            aAnimation = rGraphic.GetAnimation();
        else
        {
            AnimationBitmap aAnimationBitmap( rGraphic.GetBitmap(), Point(), Size() );
            aAnimation.Insert( aAnimationBitmap );
        }

        sal_uInt16 i;
        for ( i = 0; i < aAnimation.Count(); i++ )
            mnSumOfAllPictHeight += aAnimation.Get( i ).aBmpEx.GetSizePixel().Height();

        for ( i = 0; mbStatus && ( i < aAnimation.Count() ); i++ )
        {
            mnPalPos = 0;
            const AnimationBitmap& rAnimationBitmap = aAnimation.Get( i );
            aBmp = rAnimationBitmap.aBmpEx.GetBitmap();
            mpAcc = aBmp.AcquireReadAccess();
            if ( mpAcc )
            {
                mnBitsPerPixel = aBmp.GetBitCount();

                // export code below only handles four discrete cases
                mnBitsPerPixel =
                    mnBitsPerPixel <= 1 ? 1 : mnBitsPerPixel <= 4 ? 4 : mnBitsPerPixel <= 8 ? 8 : 24;

                if ( ImplWriteHeader( ( aAnimation.Count() > 0 ) ) )
                {
                    Size aDestMapSize( 300, 300 );
                    const MapMode aMapMode( aBmp.GetPrefMapMode() );
                    if ( aMapMode.GetMapUnit() != MAP_PIXEL )
                    {
                        const Size aPrefSize( rGraphic.GetPrefSize() );
                        aDestMapSize = OutputDevice::LogicToLogic( aPrefSize, aMapMode, MAP_INCH );
                    }
                    ImplWriteResolution( mnXResPos, aDestMapSize.Width() );
                    ImplWriteResolution( mnYResPos, aDestMapSize.Height() );
                    if  ( mnPalPos )
                        ImplWritePalette();
                    ImplWriteBody();
                }
                sal_uInt32 nCurPos = mpOStm->Tell();
                mpOStm->Seek( mnCurrentTagCountPos );
                *mpOStm << mnTagCount;
                mpOStm->Seek( nCurPos );

                aBmp.ReleaseAccess( mpAcc );
            }
            else
                mbStatus = sal_False;
        }
    }
    mpOStm->SetNumberFormatInt( nOldFormat );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}

// ------------------------------------------------------------------------

void TIFFWriter::ImplCallback( sal_uInt32 nPercent )
{
    if ( xStatusIndicator.is() )
    {
        if( nPercent >= mnLastPercent + 3 )
        {
            mnLastPercent = nPercent;
            if ( nPercent <= 100 )
                xStatusIndicator->setValue( nPercent );
        }
    }
}


// ------------------------------------------------------------------------

sal_Bool TIFFWriter::ImplWriteHeader( sal_Bool bMultiPage )
{
    mnTagCount = 0;
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();

    if ( mnWidth && mnHeight && mnBitsPerPixel && mbStatus )
    {
        sal_uInt32 nCurrentPos = mpOStm->Tell();
        mpOStm->Seek( mnLatestIfdPos );
        *mpOStm << (sal_uInt32)( nCurrentPos - mnStreamOfs );   // offset to the IFD
        mpOStm->Seek( nCurrentPos );

        // (OFS8) TIFF image file directory (IFD)
        mnCurrentTagCountPos = mpOStm->Tell();
        *mpOStm << (sal_uInt16)0;               // the number of tagentrys is to insert later

        sal_uInt32 nSubFileFlags = 0;
        if ( bMultiPage )
            nSubFileFlags |= 2;
        ImplWriteTag( NewSubfileType, 4, 1, nSubFileFlags );
        ImplWriteTag( ImageWidth, 4, 1, mnWidth );
        ImplWriteTag( ImageLength, 4, 1, mnHeight);
        ImplWriteTag( BitsPerSample, 3, 1, ( mnBitsPerPixel == 24 ) ? 8 : mnBitsPerPixel );
        ImplWriteTag( Compression, 3, 1, 5 );
        sal_uInt8 nTemp;
        switch ( mnBitsPerPixel )
        {
            case 1 :
                nTemp = 1;
                break;
            case 4 :
            case 8 :
                nTemp = 3;
                break;
            case 24:
                nTemp = 2;
                break;
            default:
                nTemp = 0;  // -Wall set a default...
                break;
        }
        ImplWriteTag( PhotometricInterpretation, 3, 1, nTemp );
        mnBitmapPos = mpOStm->Tell();
        ImplWriteTag( StripOffsets, 4, 1, 0 );
        ImplWriteTag( SamplesPerPixel, 3, 1, ( mnBitsPerPixel == 24 ) ? 3 : 1 );
        ImplWriteTag( RowsPerStrip, 4, 1, mnHeight );   //0xffffffff );
        mnStripByteCountPos = mpOStm->Tell();
        ImplWriteTag( StripByteCounts, 4, 1, ( ( mnWidth * mnBitsPerPixel * mnHeight ) + 7 ) >> 3 );
        mnXResPos = mpOStm->Tell();
        ImplWriteTag( XResolution, 5, 1, 0 );
        mnYResPos = mpOStm->Tell();
        ImplWriteTag( YResolution, 5, 1, 0 );
        if ( mnBitsPerPixel != 1 )
            ImplWriteTag( PlanarConfiguration, 3, 1, 1 );   //  ( RGB ORDER )
        ImplWriteTag( ResolutionUnit, 3, 1, 2);             // Resolution Unit is Inch
        if ( ( mnBitsPerPixel == 4 ) || ( mnBitsPerPixel == 8 ) )
        {
            mnColors = mpAcc->GetPaletteEntryCount();
            mnPalPos = mpOStm->Tell();
            ImplWriteTag( ColorMap, 3, 3 * mnColors, 0 );
        }

        // and last we write zero to close the num dir entries list
        mnLatestIfdPos = mpOStm->Tell();
        *mpOStm << (sal_uInt32)0;               // there are no more IFD
    }
    else
        mbStatus = sal_False;

    return mbStatus;
}

// ------------------------------------------------------------------------

void TIFFWriter::ImplWritePalette()
{
    sal_uInt16 i;
    sal_uLong nCurrentPos = mpOStm->Tell();
    mpOStm->Seek( mnPalPos + 8 );           // the palette tag entry needs the offset
    *mpOStm << static_cast<sal_uInt32>(nCurrentPos - mnStreamOfs);  // to the palette colors
    mpOStm->Seek( nCurrentPos );

    for ( i = 0; i < mnColors; i++ )
    {
        const BitmapColor& rColor = mpAcc->GetPaletteColor( i );
        *mpOStm << (sal_uInt16)( rColor.GetRed() << 8 );
    }
    for ( i = 0; i < mnColors; i++ )
    {
        const BitmapColor& rColor = mpAcc->GetPaletteColor( i );
        *mpOStm << (sal_uInt16)( rColor.GetGreen() << 8 );
    }
    for ( i = 0; i < mnColors; i++ )
    {
        const BitmapColor& rColor = mpAcc->GetPaletteColor( i );
        *mpOStm << (sal_uInt16)( rColor.GetBlue() << 8 );
    }
}

// ------------------------------------------------------------------------

sal_Bool TIFFWriter::ImplWriteBody()
{
    sal_uInt8   nTemp = 0;
    sal_uInt8    nShift;
    sal_uLong   j, x, y;

    sal_uLong nGfxBegin = mpOStm->Tell();
    mpOStm->Seek( mnBitmapPos + 8 );        // the strip offset tag entry needs the offset
    *mpOStm << static_cast<sal_uInt32>(nGfxBegin - mnStreamOfs);        // to the bitmap data
    mpOStm->Seek( nGfxBegin );

    StartCompression();

    switch( mnBitsPerPixel )
    {
        case 24 :
        {
            for ( y = 0; y < mnHeight; y++, mnCurAllPictHeight++ )
            {
                ImplCallback( 100 * mnCurAllPictHeight / mnSumOfAllPictHeight );
                for ( x = 0; x < mnWidth; x++ )
                {
                    const BitmapColor& rColor = mpAcc->GetPixel( y, x );
                    Compress( rColor.GetRed() );
                    Compress( rColor.GetGreen() );
                    Compress( rColor.GetBlue() );
                }
            }
        }
        break;

        case 8 :
        {
            for ( y = 0; y < mnHeight; y++, mnCurAllPictHeight++ )
            {
                ImplCallback( 100 * mnCurAllPictHeight / mnSumOfAllPictHeight );
                for ( x = 0; x < mnWidth; x++ )
                {
                    Compress( mpAcc->GetPixel( y, x ) );
                }
            }
        }
        break;

        case 4 :
        {
            for ( nShift = 0, y = 0; y < mnHeight; y++, mnCurAllPictHeight++ )
            {
                ImplCallback( 100 * mnCurAllPictHeight / mnSumOfAllPictHeight );
                for ( x = 0; x < mnWidth; x++, nShift++ )
                {
                    if (!( nShift & 1 ))
                        nTemp = ( (sal_uInt8)mpAcc->GetPixel( y, x ) << 4 );
                    else
                        Compress( (sal_uInt8)( nTemp | ( mpAcc->GetPixel( y, x ) & 0xf ) ) );
                }
                if ( nShift & 1 )
                    Compress( nTemp );
            }
        }
        break;

        case 1 :
        {
            j = 1;
            for ( y = 0; y < mnHeight; y++, mnCurAllPictHeight++ )
            {
                ImplCallback( 100 * mnCurAllPictHeight / mnSumOfAllPictHeight );
                for ( x = 0; x < mnWidth; x++)
                {
                    j <<= 1;
                    j |= ( ( ~mpAcc->GetPixel( y, x ) ) & 1 );
                    if ( j & 0x100 )
                    {
                        Compress( (sal_uInt8)j );
                        j = 1;
                    }
                }
                if ( j != 1 )
                {
                    Compress( (sal_uInt8)(j << ( ( ( x & 7) ^ 7 ) + 1 ) ) );
                    j = 1;
                }
            }
        }
        break;

        default:
        {
            mbStatus = sal_False;
        }
        break;
    }

    EndCompression();

    if ( mnStripByteCountPos && mbStatus )
    {
        sal_uLong nGfxEnd = mpOStm->Tell();
        mpOStm->Seek( mnStripByteCountPos + 8 );
        *mpOStm << static_cast<sal_uInt32>( nGfxEnd - nGfxBegin );      // mnStripByteCountPos needs the size of the compression data
        mpOStm->Seek( nGfxEnd );
    }
    return mbStatus;
}

// ------------------------------------------------------------------------

void TIFFWriter::ImplWriteResolution( sal_uLong nStreamPos, sal_uInt32 nResolutionUnit )
{
    sal_uLong nCurrentPos = mpOStm->Tell();
    mpOStm->Seek( nStreamPos + 8 );
    *mpOStm << (sal_uInt32)nCurrentPos - mnStreamOfs;
    mpOStm->Seek( nCurrentPos );
    *mpOStm << (sal_uInt32)1;
    *mpOStm << nResolutionUnit;
}

// ------------------------------------------------------------------------

void TIFFWriter::ImplWriteTag( sal_uInt16 nTagID, sal_uInt16 nDataType, sal_uInt32 nNumberOfItems, sal_uInt32 nValue)
{
        mnTagCount++;

        *mpOStm << nTagID;
        *mpOStm << nDataType;
        *mpOStm << nNumberOfItems;
        if ( nDataType == 3 )
            nValue <<=16;           // in Big Endian Mode WORDS needed to be shifted to a DWORD
        *mpOStm << nValue;
}

// ------------------------------------------------------------------------

inline void TIFFWriter::WriteBits( sal_uInt16 nCode, sal_uInt16 nCodeLen )
{
    dwShift |= ( nCode << ( nOffset - nCodeLen ) );
    nOffset -= nCodeLen;
    while ( nOffset < 24 )
    {
        *mpOStm << (sal_uInt8)( dwShift >> 24 );
        dwShift <<= 8;
        nOffset += 8;
    }
    if ( nCode == 257 && nOffset != 32 )
    {
        *mpOStm << (sal_uInt8)( dwShift >> 24 );
    }
}

// ------------------------------------------------------------------------

void TIFFWriter::StartCompression()
{
    sal_uInt16 i;
    nDataSize = 8;

    nClearCode = 1 << nDataSize;
    nEOICode = nClearCode + 1;
    nTableSize = nEOICode + 1;
    nCodeSize = nDataSize + 1;

    nOffset = 32;                       // anzahl freier bits in dwShift
    dwShift = 0;

    pTable = new TIFFLZWCTreeNode[ 4096 ];

    for ( i = 0; i < 4096; i++)
    {
        pTable[ i ].pBrother = pTable[ i ].pFirstChild = NULL;
        pTable[ i ].nValue = (sal_uInt8)( pTable[ i ].nCode = i );
    }

    pPrefix = NULL;
    WriteBits( nClearCode, nCodeSize );
}

// ------------------------------------------------------------------------

void TIFFWriter::Compress( sal_uInt8 nCompThis )
{
    TIFFLZWCTreeNode*    p;
    sal_uInt16              i;
    sal_uInt8               nV;

    if( !pPrefix )
    {
        pPrefix = pTable + nCompThis;
    }
    else
    {
        nV = nCompThis;
        for( p = pPrefix->pFirstChild; p != NULL; p = p->pBrother )
        {
            if ( p->nValue == nV )
                break;
        }

        if( p )
            pPrefix = p;
        else
        {
            WriteBits( pPrefix->nCode, nCodeSize );

            if ( nTableSize == 409 )
            {
                WriteBits( nClearCode, nCodeSize );

                for ( i = 0; i < nClearCode; i++ )
                    pTable[ i ].pFirstChild = NULL;

                nCodeSize = nDataSize + 1;
                nTableSize = nEOICode + 1;
            }
            else
            {
                if( nTableSize == (sal_uInt16)( ( 1 << nCodeSize ) - 1 ) )
                    nCodeSize++;

                p = pTable + ( nTableSize++ );
                p->pBrother = pPrefix->pFirstChild;
                pPrefix->pFirstChild = p;
                p->nValue = nV;
                p->pFirstChild = NULL;
            }

            pPrefix = pTable + nV;
        }
    }
}

// ------------------------------------------------------------------------

void TIFFWriter::EndCompression()
{
    if( pPrefix )
        WriteBits( pPrefix->nCode, nCodeSize );

    WriteBits( nEOICode, nCodeSize );
    delete[] pTable;
}

// ------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" sal_Bool __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool )
{
    return TIFFWriter().WriteTIFF( rGraphic, rStream, pFilterConfigItem );
}

