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


#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <vcl/outdev.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>

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

namespace {

struct TIFFLZWCTreeNode
{

    TIFFLZWCTreeNode*       pBrother;       // next node with the same father
    TIFFLZWCTreeNode*       pFirstChild;    // first son
    sal_uInt16              nCode;          // The code for the string of pixel values, which arises if... <missing comment>
    sal_uInt16              nValue;         // pixel value
};


class TIFFWriter
{
private:

    SvStream& m_rOStm;
    sal_uInt32              mnStreamOfs;

    bool                    mbStatus;
    BitmapReadAccess*       mpAcc;

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

    std::unique_ptr<TIFFLZWCTreeNode[]> pTable;
    TIFFLZWCTreeNode*       pPrefix;
    sal_uInt16              nDataSize;
    sal_uInt16              nClearCode;
    sal_uInt16              nEOICode;
    sal_uInt16              nTableSize;
    sal_uInt16              nCodeSize;
    sal_uInt32              nOffset;
    sal_uInt32              dwShift;

    css::uno::Reference< css::task::XStatusIndicator > xStatusIndicator;

    void                ImplCallback( sal_uInt32 nPercent );
    bool                ImplWriteHeader( bool bMultiPage );
    void                ImplWritePalette();
    void                ImplWriteBody();
    void                ImplWriteTag( sal_uInt16 TagID, sal_uInt16 DataType, sal_uInt32 NumberOfItems, sal_uInt32 Value);
    void                ImplWriteResolution( sal_uInt64 nStreamPos, sal_uInt32 nResolutionUnit );
    void                StartCompression();
    void                Compress( sal_uInt8 nSrc );
    void                EndCompression();
    inline void         WriteBits( sal_uInt16 nCode, sal_uInt16 nCodeLen );

public:

    explicit            TIFFWriter(SvStream &rStream);

    bool WriteTIFF( const Graphic& rGraphic, FilterConfigItem const * pFilterConfigItem );
};

}

TIFFWriter::TIFFWriter(SvStream &rStream)
    : m_rOStm(rStream)
    , mnStreamOfs(0)
    , mbStatus(true)
    , mpAcc(nullptr)
    , mnWidth(0)
    , mnHeight(0)
    , mnColors(0)
    , mnCurAllPictHeight(0)
    , mnSumOfAllPictHeight(0)
    , mnBitsPerPixel(0)
    , mnLastPercent(0)
    , mnLatestIfdPos(0)
    , mnTagCount(0)
    , mnCurrentTagCountPos(0)
    , mnXResPos(0)
    , mnYResPos(0)
    , mnPalPos(0)
    , mnBitmapPos(0)
    , mnStripByteCountPos(0)
    , pPrefix(nullptr)
    , nDataSize(0)
    , nClearCode(0)
    , nEOICode(0)
    , nTableSize(0)
    , nCodeSize(0)
    , nOffset(0)
    , dwShift(0)
{
}


bool TIFFWriter::WriteTIFF( const Graphic& rGraphic, FilterConfigItem const * pFilterConfigItem)
{
    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            xStatusIndicator->start( OUString(), 100 );
        }
    }

    const SvStreamEndian nOldFormat = m_rOStm.GetEndian();
    mnStreamOfs = m_rOStm.Tell();

    // we will use the BIG Endian Mode
    // TIFF header
    m_rOStm.SetEndian( SvStreamEndian::BIG );
    m_rOStm.WriteUInt32( 0x4d4d002a );      // TIFF identifier
    mnLatestIfdPos = m_rOStm.Tell();
    m_rOStm.WriteUInt32( 0 );

    if( mbStatus )
    {
        Animation aAnimation = rGraphic.IsAnimated() ? rGraphic.GetAnimation() : Animation();
        if (!rGraphic.IsAnimated())
            aAnimation.Insert(AnimationBitmap(rGraphic.GetBitmapEx(), Point(), Size()));

        for (size_t i = 0; i < aAnimation.Count(); ++i)
            mnSumOfAllPictHeight += aAnimation.Get(i).maBitmapEx.GetSizePixel().Height();

        for (size_t i = 0; mbStatus && i < aAnimation.Count(); ++i)
        {
            mnPalPos = 0;
            const AnimationBitmap& rAnimationBitmap = aAnimation.Get( i );
            Bitmap aBmp = rAnimationBitmap.maBitmapEx.GetBitmap();
            mpAcc = aBmp.AcquireReadAccess();
            if ( mpAcc )
            {
                mnBitsPerPixel = aBmp.GetBitCount();

                // export code below only handles four discrete cases
                mnBitsPerPixel =
                    mnBitsPerPixel <= 1 ? 1 : mnBitsPerPixel <= 4 ? 4 : mnBitsPerPixel <= 8 ? 8 : 24;

                if ( ImplWriteHeader( aAnimation.Count() > 0 ) )
                {
                    Size aDestMapSize( 300, 300 );
                    const MapMode& aMapMode( aBmp.GetPrefMapMode() );
                    if ( aMapMode.GetMapUnit() != MapUnit::MapPixel )
                    {
                        const Size aPrefSize( rGraphic.GetPrefSize() );
                        aDestMapSize = OutputDevice::LogicToLogic(aPrefSize, aMapMode, MapMode(MapUnit::MapInch));
                    }
                    ImplWriteResolution( mnXResPos, aDestMapSize.Width() );
                    ImplWriteResolution( mnYResPos, aDestMapSize.Height() );
                    if  ( mnPalPos )
                        ImplWritePalette();
                    ImplWriteBody();
                }
                sal_uInt32 nCurPos = m_rOStm.Tell();
                m_rOStm.Seek( mnCurrentTagCountPos );
                m_rOStm.WriteUInt16( mnTagCount );
                m_rOStm.Seek( nCurPos );

                Bitmap::ReleaseAccess( mpAcc );
            }
            else
                mbStatus = false;
        }
    }
    m_rOStm.SetEndian( nOldFormat );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}


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


bool TIFFWriter::ImplWriteHeader( bool bMultiPage )
{
    mnTagCount = 0;
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();

    if ( mnWidth && mnHeight && mnBitsPerPixel && mbStatus )
    {
        sal_uInt32 nCurrentPos = m_rOStm.Tell();
        m_rOStm.Seek( mnLatestIfdPos );
        m_rOStm.WriteUInt32( nCurrentPos - mnStreamOfs );   // offset to the IFD
        m_rOStm.Seek( nCurrentPos );

        // (OFS8) TIFF image file directory (IFD)
        mnCurrentTagCountPos = m_rOStm.Tell();
        m_rOStm.WriteUInt16( 0 );               // the number of tangents to insert later

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
        mnBitmapPos = m_rOStm.Tell();
        ImplWriteTag( StripOffsets, 4, 1, 0 );
        ImplWriteTag( SamplesPerPixel, 3, 1, ( mnBitsPerPixel == 24 ) ? 3 : 1 );
        ImplWriteTag( RowsPerStrip, 4, 1, mnHeight );
        mnStripByteCountPos = m_rOStm.Tell();
        ImplWriteTag( StripByteCounts, 4, 1, ( ( mnWidth * mnBitsPerPixel * mnHeight ) + 7 ) >> 3 );
        mnXResPos = m_rOStm.Tell();
        ImplWriteTag( XResolution, 5, 1, 0 );
        mnYResPos = m_rOStm.Tell();
        ImplWriteTag( YResolution, 5, 1, 0 );
        if ( mnBitsPerPixel != 1 )
            ImplWriteTag( PlanarConfiguration, 3, 1, 1 );   //  ( RGB ORDER )
        ImplWriteTag( ResolutionUnit, 3, 1, 2);             // Resolution Unit is Inch
        if ( ( mnBitsPerPixel == 4 ) || ( mnBitsPerPixel == 8 ) )
        {
            mnColors = mpAcc->GetPaletteEntryCount();
            mnPalPos = m_rOStm.Tell();
            ImplWriteTag( ColorMap, 3, 3 * mnColors, 0 );
        }

        // and last we write zero to close the num dir entries list
        mnLatestIfdPos = m_rOStm.Tell();
        m_rOStm.WriteUInt32( 0 );               // there are no more IFD
    }
    else
        mbStatus = false;

    return mbStatus;
}


void TIFFWriter::ImplWritePalette()
{
    sal_uInt64 nCurrentPos = m_rOStm.Tell();
    m_rOStm.Seek( mnPalPos + 8 );           // the palette tag entry needs the offset
    m_rOStm.WriteUInt32( nCurrentPos - mnStreamOfs );  // to the palette colors
    m_rOStm.Seek( nCurrentPos );

    for ( sal_uInt32 i = 0; i < mnColors; i++ )
    {
        const BitmapColor& rColor = mpAcc->GetPaletteColor( i );
        m_rOStm.WriteUInt16( rColor.GetRed() << 8 );
    }
    for ( sal_uInt32 i = 0; i < mnColors; i++ )
    {
        const BitmapColor& rColor = mpAcc->GetPaletteColor( i );
        m_rOStm.WriteUInt16( rColor.GetGreen() << 8 );
    }
    for ( sal_uInt32 i = 0; i < mnColors; i++ )
    {
        const BitmapColor& rColor = mpAcc->GetPaletteColor( i );
        m_rOStm.WriteUInt16( rColor.GetBlue() << 8 );
    }
}


void TIFFWriter::ImplWriteBody()
{
    sal_uInt8   nTemp = 0;
    sal_uInt8    nShift;
    sal_uInt32   j, x, y;

    sal_uInt64 nGfxBegin = m_rOStm.Tell();
    m_rOStm.Seek( mnBitmapPos + 8 );                // the strip offset tag entry needs the offset
    m_rOStm.WriteUInt32( nGfxBegin - mnStreamOfs ); // to the bitmap data
    m_rOStm.Seek( nGfxBegin );

    StartCompression();

    switch( mnBitsPerPixel )
    {
        case 24 :
        {
            for ( y = 0; y < mnHeight; y++, mnCurAllPictHeight++ )
            {
                ImplCallback( 100 * mnCurAllPictHeight / mnSumOfAllPictHeight );
                Scanline pScanline = mpAcc->GetScanline( y );
                for ( x = 0; x < mnWidth; x++ )
                {
                    const BitmapColor& rColor = mpAcc->GetPixelFromData( pScanline, x );
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
                Scanline pScanline = mpAcc->GetScanline( y );
                for ( x = 0; x < mnWidth; x++ )
                {
                    Compress( mpAcc->GetIndexFromData( pScanline, x ) );
                }
            }
        }
        break;

        case 4 :
        {
            for ( nShift = 0, y = 0; y < mnHeight; y++, mnCurAllPictHeight++ )
            {
                ImplCallback( 100 * mnCurAllPictHeight / mnSumOfAllPictHeight );
                Scanline pScanline = mpAcc->GetScanline( y );
                for ( x = 0; x < mnWidth; x++, nShift++ )
                {
                    if (!( nShift & 1 ))
                        nTemp = ( mpAcc->GetIndexFromData( pScanline, x ) << 4 );
                    else
                        Compress( static_cast<sal_uInt8>( nTemp | ( mpAcc->GetIndexFromData( pScanline, x ) & 0xf ) ) );
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
                Scanline pScanline = mpAcc->GetScanline( y );
                for ( x = 0; x < mnWidth; x++)
                {
                    j <<= 1;
                    j |= ( ( ~mpAcc->GetIndexFromData( pScanline, x ) ) & 1 );
                    if ( j & 0x100 )
                    {
                        Compress( static_cast<sal_uInt8>(j) );
                        j = 1;
                    }
                }
                if ( j != 1 )
                {
                    Compress( static_cast<sal_uInt8>(j << ( ( ( x & 7) ^ 7 ) + 1 ) ) );
                    j = 1;
                }
            }
        }
        break;

        default:
        {
            mbStatus = false;
        }
        break;
    }

    EndCompression();

    if ( mnStripByteCountPos && mbStatus )
    {
        sal_uInt64 nGfxEnd = m_rOStm.Tell();
        m_rOStm.Seek( mnStripByteCountPos + 8 );
        m_rOStm.WriteUInt32( nGfxEnd - nGfxBegin );      // mnStripByteCountPos needs the size of the compression data
        m_rOStm.Seek( nGfxEnd );
    }
}


void TIFFWriter::ImplWriteResolution( sal_uInt64 nStreamPos, sal_uInt32 nResolutionUnit )
{
    sal_uInt64 nCurrentPos = m_rOStm.Tell();
    m_rOStm.Seek( nStreamPos + 8 );
    m_rOStm.WriteUInt32( nCurrentPos - mnStreamOfs );
    m_rOStm.Seek( nCurrentPos );
    m_rOStm.WriteUInt32( 1 );
    m_rOStm.WriteUInt32( nResolutionUnit );
}


void TIFFWriter::ImplWriteTag( sal_uInt16 nTagID, sal_uInt16 nDataType, sal_uInt32 nNumberOfItems, sal_uInt32 nValue)
{
        mnTagCount++;

        m_rOStm.WriteUInt16( nTagID );
        m_rOStm.WriteUInt16( nDataType );
        m_rOStm.WriteUInt32( nNumberOfItems );
        if ( nDataType == 3 )
            nValue <<=16;           // in Big Endian Mode WORDS needed to be shifted to a DWORD
        m_rOStm.WriteUInt32( nValue );
}


inline void TIFFWriter::WriteBits( sal_uInt16 nCode, sal_uInt16 nCodeLen )
{
    dwShift |= ( nCode << ( nOffset - nCodeLen ) );
    nOffset -= nCodeLen;
    while ( nOffset < 24 )
    {
        m_rOStm.WriteUChar( dwShift >> 24 );
        dwShift <<= 8;
        nOffset += 8;
    }
    if ( nCode == 257 && nOffset != 32 )
    {
        m_rOStm.WriteUChar( dwShift >> 24 );
    }
}


void TIFFWriter::StartCompression()
{
    sal_uInt16 i;
    nDataSize = 8;

    nClearCode = 1 << nDataSize;
    nEOICode = nClearCode + 1;
    nTableSize = nEOICode + 1;
    nCodeSize = nDataSize + 1;

    nOffset = 32;                       // number of free bits in dwShift
    dwShift = 0;

    pTable.reset(new TIFFLZWCTreeNode[ 4096 ]);

    for ( i = 0; i < 4096; i++)
    {
        pTable[ i ].pBrother = pTable[ i ].pFirstChild = nullptr;
        pTable[ i ].nCode = i;
        pTable[ i ].nValue = static_cast<sal_uInt8>( i );
    }

    pPrefix = nullptr;
    WriteBits( nClearCode, nCodeSize );
}


void TIFFWriter::Compress( sal_uInt8 nCompThis )
{
    TIFFLZWCTreeNode*    p;
    sal_uInt16              i;
    sal_uInt8               nV;

    if( !pPrefix )
    {
        pPrefix = &pTable[nCompThis];
    }
    else
    {
        nV = nCompThis;
        for( p = pPrefix->pFirstChild; p != nullptr; p = p->pBrother )
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
                    pTable[ i ].pFirstChild = nullptr;

                nCodeSize = nDataSize + 1;
                nTableSize = nEOICode + 1;
            }
            else
            {
                if( nTableSize == static_cast<sal_uInt16>( ( 1 << nCodeSize ) - 1 ) )
                    nCodeSize++;

                p = &pTable[ nTableSize++ ];
                p->pBrother = pPrefix->pFirstChild;
                pPrefix->pFirstChild = p;
                p->nValue = nV;
                p->pFirstChild = nullptr;
            }

            pPrefix = &pTable[nV];
        }
    }
}


void TIFFWriter::EndCompression()
{
    if( pPrefix )
        WriteBits( pPrefix->nCode, nCodeSize );

    WriteBits( nEOICode, nCodeSize );
    pTable.reset();
}


extern "C" SAL_DLLPUBLIC_EXPORT bool
etiGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem )
{
    TIFFWriter aWriter(rStream);
    return aWriter.WriteTIFF( rGraphic, pFilterConfigItem );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
