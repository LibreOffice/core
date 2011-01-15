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
    USHORT              nCode;          // Der Code fuer den String von Pixelwerten, der sich ergibt, wenn
    USHORT              nValue;         // Der Pixelwert
};

class TIFFWriter
{
private:

    SvStream& m_rOStm;
    UINT32              mnStreamOfs;

    BOOL                mbStatus;
    BitmapReadAccess*   mpAcc;

    UINT32              mnWidth, mnHeight, mnColors;
    UINT32              mnCurAllPictHeight;
    UINT32              mnSumOfAllPictHeight;
    UINT32              mnBitsPerPixel;
    UINT32              mnLastPercent;

    UINT32              mnLatestIfdPos;
    UINT16              mnTagCount;                 // number of tags already written
    UINT32              mnCurrentTagCountPos;       // offset to the position where the current
                                                    // tag count is to insert

    UINT32              mnXResPos;                  // if != 0 this DWORDs stores the
    UINT32              mnYResPos;                  // actual streamposition of the
    UINT32              mnPalPos;                   // Tag Entry
    UINT32              mnBitmapPos;
    UINT32              mnStripByteCountPos;

    TIFFLZWCTreeNode*   pTable;
    TIFFLZWCTreeNode*   pPrefix;
    USHORT              nDataSize;
    USHORT              nClearCode;
    USHORT              nEOICode;
    USHORT              nTableSize;
    USHORT              nCodeSize;
    ULONG               nOffset;
    ULONG               dwShift;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    void                ImplCallback( UINT32 nPercent );
    BOOL                ImplWriteHeader( BOOL bMultiPage );
    void                ImplWritePalette();
    BOOL                ImplWriteBody();
    void                ImplWriteTag( UINT16 TagID, UINT16 DataType, UINT32 NumberOfItems, UINT32 Value);
    void                ImplWriteResolution( ULONG nStreamPos, sal_uInt32 nResolutionUnit );
    void                StartCompression();
    void                Compress( BYTE nSrc );
    void                EndCompression();
    inline void         WriteBits( USHORT nCode, USHORT nCodeLen );

public:

    TIFFWriter(SvStream &rStream);
    ~TIFFWriter();

    BOOL WriteTIFF( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem );
};

// ------------------------------------------------------------------------

TIFFWriter::TIFFWriter(SvStream &rStream)
    : m_rOStm(rStream)
    , mbStatus(TRUE)
    , mpAcc(NULL)
    , mnCurAllPictHeight(0)
    , mnSumOfAllPictHeight(0)
    , mnLastPercent(0)
    , mnXResPos(0)
    , mnYResPos(0)
    , mnBitmapPos(0)
    , mnStripByteCountPos(0)
{
}

// ------------------------------------------------------------------------

TIFFWriter::~TIFFWriter()
{
}

// ------------------------------------------------------------------------

BOOL TIFFWriter::WriteTIFF( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem)
{
    ULONG*  pDummy = new ULONG; delete pDummy; // damit unter OS/2
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

    const UINT16    nOldFormat = m_rOStm.GetNumberFormatInt();
    mnStreamOfs = m_rOStm.Tell();

    // we will use the BIG Endian Mode
    // TIFF header
    m_rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    m_rOStm << (UINT32)0x4d4d002a;      // TIFF identifier
    mnLatestIfdPos = m_rOStm.Tell();
    m_rOStm << (UINT32)0;

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

        USHORT i;
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
                UINT32 nCurPos = m_rOStm.Tell();
                m_rOStm.Seek( mnCurrentTagCountPos );
                m_rOStm << mnTagCount;
                m_rOStm.Seek( nCurPos );

                aBmp.ReleaseAccess( mpAcc );
            }
            else
                mbStatus = FALSE;
        }
    }
    m_rOStm.SetNumberFormatInt( nOldFormat );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}

// ------------------------------------------------------------------------

void TIFFWriter::ImplCallback( UINT32 nPercent )
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

BOOL TIFFWriter::ImplWriteHeader( BOOL bMultiPage )
{
    mnTagCount = 0;
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();

    if ( mnWidth && mnHeight && mnBitsPerPixel && mbStatus )
    {
        UINT32 nCurrentPos = m_rOStm.Tell();
        m_rOStm.Seek( mnLatestIfdPos );
        m_rOStm << (UINT32)( nCurrentPos - mnStreamOfs );   // offset to the IFD
        m_rOStm.Seek( nCurrentPos );

        // (OFS8) TIFF image file directory (IFD)
        mnCurrentTagCountPos = m_rOStm.Tell();
        m_rOStm << (UINT16)0;               // the number of tagentrys is to insert later

        UINT32 nSubFileFlags = 0;
        if ( bMultiPage )
            nSubFileFlags |= 2;
        ImplWriteTag( NewSubfileType, 4, 1, nSubFileFlags );
        ImplWriteTag( ImageWidth, 4, 1, mnWidth );
        ImplWriteTag( ImageLength, 4, 1, mnHeight);
        ImplWriteTag( BitsPerSample, 3, 1, ( mnBitsPerPixel == 24 ) ? 8 : mnBitsPerPixel );
        ImplWriteTag( Compression, 3, 1, 5 );
        BYTE nTemp;
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
        ImplWriteTag( RowsPerStrip, 4, 1, mnHeight );   //0xffffffff );
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
        m_rOStm << (UINT32)0;               // there are no more IFD
    }
    else
        mbStatus = FALSE;

    return mbStatus;
}

// ------------------------------------------------------------------------

void TIFFWriter::ImplWritePalette()
{
    USHORT i;
    ULONG nCurrentPos = m_rOStm.Tell();
    m_rOStm.Seek( mnPalPos + 8 );           // the palette tag entry needs the offset
    m_rOStm << static_cast<sal_uInt32>(nCurrentPos - mnStreamOfs);  // to the palette colors
    m_rOStm.Seek( nCurrentPos );

    for ( i = 0; i < mnColors; i++ )
    {
        const BitmapColor& rColor = mpAcc->GetPaletteColor( i );
        m_rOStm << (UINT16)( rColor.GetRed() << 8 );
    }
    for ( i = 0; i < mnColors; i++ )
    {
        const BitmapColor& rColor = mpAcc->GetPaletteColor( i );
        m_rOStm << (UINT16)( rColor.GetGreen() << 8 );
    }
    for ( i = 0; i < mnColors; i++ )
    {
        const BitmapColor& rColor = mpAcc->GetPaletteColor( i );
        m_rOStm << (UINT16)( rColor.GetBlue() << 8 );
    }
}

// ------------------------------------------------------------------------

BOOL TIFFWriter::ImplWriteBody()
{
    BYTE    nTemp = 0;
    BYTE    nShift;
    ULONG   j, x, y;

    ULONG nGfxBegin = m_rOStm.Tell();
    m_rOStm.Seek( mnBitmapPos + 8 );        // the strip offset tag entry needs the offset
    m_rOStm << static_cast<sal_uInt32>(nGfxBegin - mnStreamOfs);        // to the bitmap data
    m_rOStm.Seek( nGfxBegin );

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
                        nTemp = ( (BYTE)mpAcc->GetPixel( y, x ) << 4 );
                    else
                        Compress( (BYTE)( nTemp | ( mpAcc->GetPixel( y, x ) & 0xf ) ) );
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
                        Compress( (BYTE)j );
                        j = 1;
                    }
                }
                if ( j != 1 )
                {
                    Compress( (BYTE)(j << ( ( ( x & 7) ^ 7 ) + 1 ) ) );
                    j = 1;
                }
            }
        }
        break;

        default:
        {
            mbStatus = FALSE;
        }
        break;
    }

    EndCompression();

    if ( mnStripByteCountPos && mbStatus )
    {
        ULONG nGfxEnd = m_rOStm.Tell();
        m_rOStm.Seek( mnStripByteCountPos + 8 );
        m_rOStm << static_cast<sal_uInt32>( nGfxEnd - nGfxBegin );      // mnStripByteCountPos needs the size of the compression data
        m_rOStm.Seek( nGfxEnd );
    }
    return mbStatus;
}

// ------------------------------------------------------------------------

void TIFFWriter::ImplWriteResolution( ULONG nStreamPos, sal_uInt32 nResolutionUnit )
{
    ULONG nCurrentPos = m_rOStm.Tell();
    m_rOStm.Seek( nStreamPos + 8 );
    m_rOStm << (UINT32)nCurrentPos - mnStreamOfs;
    m_rOStm.Seek( nCurrentPos );
    m_rOStm << (UINT32)1;
    m_rOStm << nResolutionUnit;
}

// ------------------------------------------------------------------------

void TIFFWriter::ImplWriteTag( UINT16 nTagID, UINT16 nDataType, UINT32 nNumberOfItems, UINT32 nValue)
{
        mnTagCount++;

        m_rOStm << nTagID;
        m_rOStm << nDataType;
        m_rOStm << nNumberOfItems;
        if ( nDataType == 3 )
            nValue <<=16;           // in Big Endian Mode WORDS needed to be shifted to a DWORD
        m_rOStm << nValue;
}

// ------------------------------------------------------------------------

inline void TIFFWriter::WriteBits( USHORT nCode, USHORT nCodeLen )
{
    dwShift |= ( nCode << ( nOffset - nCodeLen ) );
    nOffset -= nCodeLen;
    while ( nOffset < 24 )
    {
        m_rOStm << (BYTE)( dwShift >> 24 );
        dwShift <<= 8;
        nOffset += 8;
    }
    if ( nCode == 257 && nOffset != 32 )
    {
        m_rOStm << (BYTE)( dwShift >> 24 );
    }
}

// ------------------------------------------------------------------------

void TIFFWriter::StartCompression()
{
    USHORT i;
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
        pTable[ i ].nValue = (BYTE)( pTable[ i ].nCode = i );
    }

    pPrefix = NULL;
    WriteBits( nClearCode, nCodeSize );
}

// ------------------------------------------------------------------------

void TIFFWriter::Compress( BYTE nCompThis )
{
    TIFFLZWCTreeNode*    p;
    USHORT              i;
    BYTE                nV;

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
                if( nTableSize == (USHORT)( ( 1 << nCodeSize ) - 1 ) )
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

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, BOOL )
{
    TIFFWriter aWriter(rStream);
    return aWriter.WriteTIFF( rGraphic, pFilterConfigItem );
}

// ---------------
// - Win16 trash -
// ---------------

#ifdef WIN

static HINSTANCE hDLLInst = 0;

extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
    if ( nHeap )
        UnlockData( 0 );

    hDLLInst = hDLL;

    return TRUE;
}

// ------------------------------------------------------------------------

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
