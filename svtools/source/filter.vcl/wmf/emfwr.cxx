/*************************************************************************
 *
 *  $RCSfile: emfwr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:00 $
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

#include "emfwr.hxx"
#include <vcl/salbtype.hxx>

// -----------
// - Defines -
// -----------

#define WIN_EMR_HEADER                      1
#define WIN_EMR_POLYBEZIER                  2
#define WIN_EMR_POLYGON                     3
#define WIN_EMR_POLYLINE                    4
#define WIN_EMR_POLYBEZIERTO                5
#define WIN_EMR_POLYLINETO                  6
#define WIN_EMR_POLYPOLYLINE                7
#define WIN_EMR_POLYPOLYGON                 8
#define WIN_EMR_SETWINDOWEXTEX              9
#define WIN_EMR_SETWINDOWORGEX              10
#define WIN_EMR_SETVIEWPORTEXTEX            11
#define WIN_EMR_SETVIEWPORTORGEX            12
#define WIN_EMR_SETBRUSHORGEX               13
#define WIN_EMR_EOF                         14
#define WIN_EMR_SETPIXELV                   15
#define WIN_EMR_SETMAPPERFLAGS              16
#define WIN_EMR_SETMAPMODE                  17
#define WIN_EMR_SETBKMODE                   18
#define WIN_EMR_SETPOLYFILLMODE             19
#define WIN_EMR_SETROP2                     20
#define WIN_EMR_SETSTRETCHBLTMODE           21
#define WIN_EMR_SETTEXTALIGN                22
#define WIN_EMR_SETCOLORADJUSTMENT          23
#define WIN_EMR_SETTEXTCOLOR                24
#define WIN_EMR_SETBKCOLOR                  25
#define WIN_EMR_OFFSETCLIPRGN               26
#define WIN_EMR_MOVETOEX                    27
#define WIN_EMR_SETMETARGN                  28
#define WIN_EMR_EXCLUDECLIPRECT             29
#define WIN_EMR_INTERSECTCLIPRECT           30
#define WIN_EMR_SCALEVIEWPORTEXTEX          31
#define WIN_EMR_SCALEWINDOWEXTEX            32
#define WIN_EMR_SAVEDC                      33
#define WIN_EMR_RESTOREDC                   34
#define WIN_EMR_SETWORLDTRANSFORM           35
#define WIN_EMR_MODIFYWORLDTRANSFORM        36
#define WIN_EMR_SELECTOBJECT                37
#define WIN_EMR_CREATEPEN                   38
#define WIN_EMR_CREATEBRUSHINDIRECT         39
#define WIN_EMR_DELETEOBJECT                40
#define WIN_EMR_ANGLEARC                    41
#define WIN_EMR_ELLIPSE                     42
#define WIN_EMR_RECTANGLE                   43
#define WIN_EMR_ROUNDRECT                   44
#define WIN_EMR_ARC                         45
#define WIN_EMR_CHORD                       46
#define WIN_EMR_PIE                         47
#define WIN_EMR_SELECTPALETTE               48
#define WIN_EMR_CREATEPALETTE               49
#define WIN_EMR_SETPALETTEENTRIES           50
#define WIN_EMR_RESIZEPALETTE               51
#define WIN_EMR_REALIZEPALETTE              52
#define WIN_EMR_EXTFLOODFILL                53
#define WIN_EMR_LINETO                      54
#define WIN_EMR_ARCTO                       55
#define WIN_EMR_POLYDRAW                    56
#define WIN_EMR_SETARCDIRECTION             57
#define WIN_EMR_SETMITERLIMIT               58
#define WIN_EMR_BEGINPATH                   59
#define WIN_EMR_ENDPATH                     60
#define WIN_EMR_CLOSEFIGURE                 61
#define WIN_EMR_FILLPATH                    62
#define WIN_EMR_STROKEANDFILLPATH           63
#define WIN_EMR_STROKEPATH                  64
#define WIN_EMR_FLATTENPATH                 65
#define WIN_EMR_WIDENPATH                   66
#define WIN_EMR_SELECTCLIPPATH              67
#define WIN_EMR_ABORTPATH                   68

#define WIN_EMR_GDICOMMENT                  70
#define WIN_EMR_FILLRGN                     71
#define WIN_EMR_FRAMERGN                    72
#define WIN_EMR_INVERTRGN                   73
#define WIN_EMR_PAINTRGN                    74
#define WIN_EMR_EXTSELECTCLIPRGN            75
#define WIN_EMR_BITBLT                      76
#define WIN_EMR_STRETCHBLT                  77
#define WIN_EMR_MASKBLT                     78
#define WIN_EMR_PLGBLT                      79
#define WIN_EMR_SETDIBITSTODEVICE           80
#define WIN_EMR_STRETCHDIBITS               81
#define WIN_EMR_EXTCREATEFONTINDIRECTW      82
#define WIN_EMR_EXTTEXTOUTA                 83
#define WIN_EMR_EXTTEXTOUTW                 84
#define WIN_EMR_POLYBEZIER16                85
#define WIN_EMR_POLYGON16                   86
#define WIN_EMR_POLYLINE16                  87
#define WIN_EMR_POLYBEZIERTO16              88
#define WIN_EMR_POLYLINETO16                89
#define WIN_EMR_POLYPOLYLINE16              90
#define WIN_EMR_POLYPOLYGON16               91
#define WIN_EMR_POLYDRAW16                  92
#define WIN_EMR_CREATEMONOBRUSH             93
#define WIN_EMR_CREATEDIBPATTERNBRUSHPT     94
#define WIN_EMR_EXTCREATEPEN                95
#define WIN_EMR_POLYTEXTOUTA                96
#define WIN_EMR_POLYTEXTOUTW                97

#define WIN_SRCCOPY                         0x00CC0020L
#define WIN_SRCPAINT                        0x00EE0086L
#define WIN_SRCAND                          0x008800C6L
#define WIN_SRCINVERT                       0x00660046L

#define HANDLE_INVALID                      0xffffffff
#define MAXHANDLES                          65000

#define LINE_SELECT                         0x00000001
#define FILL_SELECT                         0x00000002
#define TEXT_SELECT                         0x00000004

// -------------
// - EMFWriter -
// -------------

BOOL EMFWriter::WriteEMF( const GDIMetaFile& rMtf, SvStream& rOStm,
                          PFilterCallback pCallback, void* pCallerData )
{
    const ULONG nHeaderPos = rOStm.Tell();

    mpHandlesUsed = new BOOL[ MAXHANDLES ];
    HMEMSET( mpHandlesUsed, 0, MAXHANDLES * sizeof( BOOL ) );
    mnHandleCount = mnLastPercent = mnRecordPos = mnRecordCount = 0;
    mnLineHandle = mnFillHandle = mnTextHandle = HANDLE_INVALID;
    mbRecordOpen = FALSE;

    mpStm = &rOStm;
    mpCallback = pCallback;
    mpCallerData = pCallerData;
    maVDev.EnableOutput( FALSE );
    maVDev.SetMapMode( rMtf.GetPrefMapMode() );

    const Size aMtfSizePix( maVDev.LogicToPixel( rMtf.GetPrefSize(), rMtf.GetPrefMapMode() ) );
    const Size aMtfSizeLog( maVDev.LogicToLogic( rMtf.GetPrefSize(), rMtf.GetPrefMapMode(), MAP_100TH_MM ) );

    // seek over header
    rOStm.SeekRel( 100 );

    // write initial values
    ImplBeginRecord( WIN_EMR_SETWINDOWORGEX );
    (*mpStm) << (INT32) 0 << (INT32) 0;
    ImplEndRecord();

    ImplBeginRecord( WIN_EMR_SETWINDOWEXTEX );
    (*mpStm) << (INT32) aMtfSizePix.Width() << (INT32) aMtfSizePix.Height();
    ImplEndRecord();

    ImplWriteRasterOp( ROP_OVERPAINT );

    ImplBeginRecord( WIN_EMR_SETBKMODE );
    (*mpStm) << (UINT32) 1; // TRANSPARENT
    ImplEndRecord();

    // write emf data
    ImplWrite( rMtf );

    // write header
    const ULONG nEndPos = mpStm->Tell(); mpStm->Seek( nHeaderPos );

    (*mpStm) << (UINT32) 0x00000001 << (UINT32) 100;
    (*mpStm) << (INT32) 0 << (INT32) 0 << (INT32) ( aMtfSizePix.Width() - 1 ) << (INT32) ( aMtfSizePix.Height() - 1 );
    (*mpStm) << (INT32) 0 << (INT32) 0 << (INT32) ( aMtfSizeLog.Width() - 1 ) << (INT32) ( aMtfSizeLog.Height() - 1 );
    (*mpStm) << (UINT32) 0x464d4520 << (UINT32) 0x10000 << (UINT32) ( nEndPos - nHeaderPos );
    (*mpStm) << (UINT32) mnRecordCount << (UINT16) ( mnHandleCount + 1 ) << (UINT16) 0 << (UINT32) 0 << (UINT32) 0 << (UINT32) 0;
    (*mpStm) << (INT32) aMtfSizePix.Width() << (INT32) aMtfSizePix.Height();
    (*mpStm) << (INT32) ( aMtfSizeLog.Width() / 100 ) << (INT32) ( aMtfSizeLog.Height() / 100 );
    (*mpStm) << (UINT32) 0 << (UINT32) 0 << (UINT32) 0;

    mpStm->Seek( nEndPos );
    delete[] mpHandlesUsed;

    return( mpStm->GetError() == ERRCODE_NONE );
}

// -----------------------------------------------------------------------------

ULONG EMFWriter::ImplAcquireHandle()
{
    ULONG nHandle = HANDLE_INVALID;

    for( ULONG i = 0; i < MAXHANDLES && ( HANDLE_INVALID == nHandle ); i++ )
    {
        if( !mpHandlesUsed[ i ] )
        {
            mpHandlesUsed[ i ] = TRUE;

            if( ( nHandle = i ) == mnHandleCount )
                mnHandleCount++;
        }
    }

    DBG_ASSERT( nHandle != HANDLE_INVALID, "No more handles available" );
    return( nHandle != HANDLE_INVALID ? nHandle + 1 : HANDLE_INVALID );
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplReleaseHandle( ULONG nHandle )
{
    DBG_ASSERT( nHandle && ( nHandle < MAXHANDLES ), "Handle out of range" );
    mpHandlesUsed[ nHandle - 1 ] = FALSE;
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplBeginRecord( ULONG nType )
{
    DBG_ASSERT( !mbRecordOpen, "Another record is already opened!" );

    if( !mbRecordOpen )
    {
        mbRecordOpen = TRUE;
        mnRecordPos = mpStm->Tell();

        (*mpStm) << nType;
        mpStm->SeekRel( 4 );
    }
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplEndRecord()
{
    DBG_ASSERT( mbRecordOpen, "Record was not opened!" );

    if( mbRecordOpen )
    {
        const ULONG nActPos = mpStm->Tell();

        mpStm->Seek( mnRecordPos + 4 );
        ( *mpStm ) << ( nActPos - mnRecordPos );
        mpStm->Seek( nActPos );

        mnRecordCount++;
        mbRecordOpen = FALSE;
    }
}

// -----------------------------------------------------------------------------

BOOL EMFWriter::ImplPrepareHandleSelect( ULONG& rHandle, ULONG nSelectType )
{
    if( rHandle != HANDLE_INVALID )
    {
        UINT32 nStockObject = 0x80000000;

        if( LINE_SELECT == nSelectType )
            nStockObject |= 0x00000007;
        else if( FILL_SELECT == nSelectType )
            nStockObject |= 0x00000001;
        else if( TEXT_SELECT == nSelectType )
            nStockObject |= 0x0000000a;

        // select stock object first
        ImplBeginRecord( WIN_EMR_SELECTOBJECT );
        ( *mpStm ) << nStockObject;
        ImplEndRecord();

        // destroy handle of created object
        ImplBeginRecord( WIN_EMR_DELETEOBJECT );
        ( *mpStm ) << rHandle;
        ImplEndRecord();

        // mark handle as free
        ImplReleaseHandle( rHandle );
    }

    rHandle = ImplAcquireHandle();

    return( HANDLE_INVALID != rHandle );
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplCheckLineAttr()
{
    if( mbLineChanged && ImplPrepareHandleSelect( mnLineHandle, LINE_SELECT ) )
    {
        ULONG nStyle = maVDev.IsLineColor() ? 0 : 5;
        ULONG nWidth = 0, nHeight = 0;

        ImplBeginRecord( WIN_EMR_CREATEPEN );
        (*mpStm) << mnLineHandle << nStyle << nWidth << nHeight;
        ImplWriteColor( maVDev.GetLineColor() );
        ImplEndRecord();

        ImplBeginRecord( WIN_EMR_SELECTOBJECT );
        (*mpStm) << mnLineHandle;
        ImplEndRecord();
    }
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplCheckFillAttr()
{
    if( mbFillChanged && ImplPrepareHandleSelect( mnFillHandle, FILL_SELECT ) )
    {
        ULONG nStyle = maVDev.IsFillColor() ? 0 : 1;
        ULONG nPatternStyle = 0;

        ImplBeginRecord( WIN_EMR_CREATEBRUSHINDIRECT );
        (*mpStm) << mnFillHandle << nStyle;
        ImplWriteColor( maVDev.GetFillColor() );
        (*mpStm) << nPatternStyle;
        ImplEndRecord();

        ImplBeginRecord( WIN_EMR_SELECTOBJECT );
        (*mpStm) << mnFillHandle;
        ImplEndRecord();
    }
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplCheckTextAttr()
{
    if( mbTextChanged && ImplPrepareHandleSelect( mnTextHandle, TEXT_SELECT ) )
    {
        const Font&     rFont = maVDev.GetFont();
        String          aFontName( rFont.GetName() );
        INT32           i, nWeight;
        BYTE            nPitchAndFamily;

        ImplBeginRecord( WIN_EMR_EXTCREATEFONTINDIRECTW );
        (*mpStm) << mnTextHandle;
        ImplWriteExtent( rFont.GetSize().Height() );
        ImplWriteExtent( rFont.GetSize().Width() );
        (*mpStm) << (INT32) rFont.GetOrientation() << (INT32) rFont.GetOrientation();

        switch( rFont.GetWeight() )
        {
            case WEIGHT_THIN:       nWeight = 100; break;
            case WEIGHT_ULTRALIGHT: nWeight = 200; break;
            case WEIGHT_LIGHT:      nWeight = 300; break;
            case WEIGHT_SEMILIGHT:  nWeight = 300; break;
            case WEIGHT_NORMAL:     nWeight = 400; break;
            case WEIGHT_MEDIUM:     nWeight = 500; break;
            case WEIGHT_SEMIBOLD:   nWeight = 600; break;
            case WEIGHT_BOLD:       nWeight = 700; break;
            case WEIGHT_ULTRABOLD:  nWeight = 800; break;
            case WEIGHT_BLACK:      nWeight = 900; break;
            default:                nWeight = 0; break;
        }

        (*mpStm) << nWeight;
        (*mpStm) << (BYTE) ( ( ITALIC_NONE == rFont.GetItalic() ) ? 0 : 1 );
        (*mpStm) << (BYTE) ( ( UNDERLINE_NONE == rFont.GetUnderline() ) ? 0 : 1 );
        (*mpStm) << (BYTE) ( ( STRIKEOUT_NONE == rFont.GetStrikeout() ) ? 0 : 1 );
        (*mpStm) << (BYTE) ( ( RTL_TEXTENCODING_SYMBOL == rFont.GetCharSet() ) ? 2 : 0 );
        (*mpStm) << (BYTE) 0 << (BYTE) 0 << (BYTE) 0;

        switch( rFont.GetPitch() )
        {
            case PITCH_FIXED:    nPitchAndFamily = 0x01; break;
            case PITCH_VARIABLE: nPitchAndFamily = 0x02; break;
            default:             nPitchAndFamily = 0x00; break;
        }

        switch( rFont.GetFamily() )
        {
            case FAMILY_DECORATIVE: nPitchAndFamily |= 0x50; break;
            case FAMILY_MODERN:     nPitchAndFamily |= 0x30; break;
            case FAMILY_ROMAN:      nPitchAndFamily |= 0x10; break;
            case FAMILY_SCRIPT:     nPitchAndFamily |= 0x40; break;
            case FAMILY_SWISS:      nPitchAndFamily |= 0x20; break;
            default: break;
        }

        (*mpStm) << nPitchAndFamily;

        for( i = 0; i < 32; i++ )
            (*mpStm) << (sal_Unicode) ( ( i < aFontName.Len() ) ? aFontName.GetChar( i ) : 0 );

        // dummy elfFullName
        for( i = 0; i < 64; i++ )
            (*mpStm) << (sal_Unicode) 0;

        // dummy elfStyle
        for( i = 0; i < 32; i++ )
            (*mpStm) << (sal_Unicode) 0;

        // dummy elfVersion, elfStyleSize, elfMatch, elfReserved
        (*mpStm) << (UINT32) 0 << (UINT32) 0 << (UINT32) 0 << (UINT32) 0 ;

        // dummy elfVendorId
        (*mpStm) << (UINT32) 0;

        // dummy elfCulture
        (*mpStm) << (UINT32) 0;

        // dummy elfPanose
        (*mpStm) << (BYTE) 0 << (BYTE) 0 << (BYTE) 0 << (BYTE) 0 << (BYTE) 0 << (BYTE) 0 << (BYTE) 0 << (BYTE) 0 << (BYTE) 0 << (BYTE) 0;

        // fill record to get a record size divideable by 4
        (*mpStm) << (UINT16) 0;

        ImplEndRecord();

        // TextAlign
        UINT32 nTextAlign;

        switch( rFont.GetAlign() )
        {
            case ALIGN_TOP:    nTextAlign = 0; break;
            case ALIGN_BOTTOM: nTextAlign = 8; break;
            default:           nTextAlign = 24; break;
        }

        ImplBeginRecord( WIN_EMR_SETTEXTALIGN );
        (*mpStm) << nTextAlign;
        ImplEndRecord();

        // Text color
        ImplBeginRecord( WIN_EMR_SETTEXTCOLOR );
        ImplWriteColor( rFont.GetColor() );
        ImplEndRecord();

        ImplBeginRecord( WIN_EMR_SELECTOBJECT );
        (*mpStm) << mnTextHandle;
        ImplEndRecord();
    }
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplWriteColor( const Color& rColor )
{
    UINT32 nCol = rColor.GetRed();

    nCol |= ( (UINT32) rColor.GetGreen() ) << 8;
    nCol |= ( (UINT32) rColor.GetBlue() ) << 16;

    (*mpStm) << nCol;
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplWriteRasterOp( RasterOp eRop )
{
    UINT32 nROP2;

    switch( eRop )
    {
        case ROP_INVERT: nROP2 = 6; break;
        case ROP_XOR:    nROP2 = 7; break;
        default:         nROP2 = 13;break;
    }

    ImplBeginRecord( WIN_EMR_SETROP2 );
    (*mpStm) << nROP2;
    ImplEndRecord();
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplWriteExtent( long nExtent )
{
    const Size aSize( maVDev.LogicToPixel( Size( nExtent, nExtent ) ) );
    (*mpStm) << (INT32) aSize.Width();
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplWritePoint( const Point& rPoint )
{
    const Point aPoint( maVDev.LogicToPixel( rPoint ) );

    (*mpStm) << (INT32) aPoint.X() << (INT32) aPoint.Y();
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplWriteSize( const Size& rSize)
{
    const Size aSize( maVDev.LogicToPixel( rSize ) );

    (*mpStm) << (INT32) aSize.Width() << (INT32) aSize.Height();
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplWriteRect( const Rectangle& rRect )
{
    const Rectangle aRect( maVDev.LogicToPixel( rRect ) );

    (*mpStm) << aRect.Left() << aRect.Top() << aRect.Right() << aRect.Bottom();
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplWritePolygonRecord( const Polygon& rPoly, BOOL bClose )
{
    if( rPoly.GetSize() )
    {
        if( bClose )
            ImplCheckFillAttr();

        ImplCheckLineAttr();

        ImplBeginRecord( bClose ? WIN_EMR_POLYGON : WIN_EMR_POLYLINE );
        ImplWriteRect( rPoly.GetBoundRect() );
        (*mpStm) << (UINT32) rPoly.GetSize();

        for( USHORT i = 0; i < rPoly.GetSize(); i++ )
            ImplWritePoint( rPoly[ i ] );

        ImplEndRecord();
    }
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplWritePolyPolygonRecord( const PolyPolygon& rPolyPoly )
{
    const UINT32 nPolyCount = rPolyPoly.Count();

    if( nPolyCount )
    {
        if( 1 == nPolyCount )
            ImplWritePolygonRecord( rPolyPoly[ 0 ], TRUE );
        else
        {
            UINT32 nTotalPoints = 0, i;

            for( i = 0; i < nPolyCount; i++ )
                nTotalPoints += rPolyPoly[ i ].GetSize();

            if( nTotalPoints )
            {
                ImplCheckFillAttr();
                ImplCheckLineAttr();

                ImplBeginRecord( WIN_EMR_POLYPOLYGON );
                ImplWriteRect( rPolyPoly.GetBoundRect() );
                (*mpStm) << nPolyCount << nTotalPoints;

                for( i = 0; i < nPolyCount; i++ )
                    (*mpStm) << (UINT32) rPolyPoly[ i ].GetSize();

                for( i = 0; i < nPolyCount; i++ )
                {
                    const Polygon& rPoly = rPolyPoly[ i ];

                    for( USHORT n = 0; n < rPoly.GetSize(); n++ )
                        ImplWritePoint( rPoly[ n ] );
                }

                ImplEndRecord();
            }
        }
    }
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplWriteBmpRecord( const Bitmap& rBmp, const Point& rPt,
                                    const Size& rSz, UINT32 nROP )
{
    if( !!rBmp )
    {
        SvMemoryStream  aMemStm( 65535, 65535 );
        const Size      aBmpSizePixel( rBmp.GetSizePixel() );

        ImplBeginRecord( WIN_EMR_STRETCHDIBITS );
        ImplWriteRect( Rectangle( rPt, rSz ) );
        ImplWritePoint( rPt );
        (*mpStm) << (INT32) 0 << (INT32) 0 << (INT32) aBmpSizePixel.Width() << (INT32) aBmpSizePixel.Height();

        // write offset positions and sizes later
        const ULONG nOffPos = mpStm->Tell();
        mpStm->SeekRel( 16 );

        (*mpStm) << (UINT32) 0 << ( ( ROP_XOR == maVDev.GetRasterOp() && WIN_SRCCOPY == nROP ) ? WIN_SRCINVERT : nROP );
        ImplWriteSize( rSz );

        rBmp.Write( aMemStm, TRUE, FALSE );

        UINT32  nDIBSize = aMemStm.Tell(), nHeaderSize, nCompression, nColsUsed, nPalCount, nImageSize;
        UINT16  nBitCount;

        // get DIB parameters
        aMemStm.Seek( 0 );
        aMemStm >> nHeaderSize;
        aMemStm.SeekRel( 10 );
        aMemStm >> nBitCount >> nCompression >> nImageSize;
        aMemStm.SeekRel( 8 );
        aMemStm >> nColsUsed;

        nPalCount = ( nBitCount <= 8 ) ? ( nColsUsed ? nColsUsed : ( 1 << (UINT32) nBitCount ) ) :
                                         ( ( 3 == nCompression ) ? 12 : 0 );

        mpStm->Write( aMemStm.GetData(), nDIBSize );

        const ULONG nEndPos = mpStm->Tell();
        mpStm->Seek( nOffPos );
        (*mpStm) << (UINT32) 80 << (UINT32)( nHeaderSize + ( nPalCount << 2 ) );
        (*mpStm) << (UINT32)( 80 + ( nHeaderSize + ( nPalCount << 2 ) ) ) << nImageSize;
        mpStm->Seek( nEndPos );

        ImplEndRecord();
    }
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplWriteTextRecord( const Point& rPos, const String rText, const long* pDXArray, UINT32 nWidth )
{
    UINT32 nLen = rText.Len(), i;

    if( nLen )
    {
        sal_Int32   nNormWidth;
        long*       pOwnArray;
        long*       pDX;

        // get text sizes
        if( pDXArray )
        {
            pOwnArray = NULL;
            nNormWidth = maVDev.GetTextWidth( rText );
            pDX = (long*) pDXArray;
        }
        else
        {
            pOwnArray = new long[ nLen ];
            nNormWidth = maVDev.GetTextArray( rText, pOwnArray );
            pDX = pOwnArray;
        }

        if( nLen > 1 )
        {
            nNormWidth = pDX[ nLen - 2 ] + maVDev.GetTextWidth( rText.GetChar( nLen - 1 ) );

            if( nWidth && nNormWidth && ( nWidth != nNormWidth ) )
            {
                const double fFactor = (double) nWidth / nNormWidth;

                for( i = 0; i < ( nLen - 1 ); i++ )
                    pDX[ i ] = FRound( pDX[ i ] * fFactor );
            }
        }

        // write text record
        ImplBeginRecord( WIN_EMR_EXTTEXTOUTW );

        ImplWriteRect( Rectangle( rPos, Size( nNormWidth, maVDev.GetTextHeight() ) ) );
        (*mpStm) << (UINT32) 2; // GM_ADVANCED
        (*mpStm) << (INT32) 0 << (INT32) 0;
        ImplWritePoint( rPos );
        (*mpStm) << (UINT32) nLen << (UINT32) 76 << (UINT32) 2;
        (*mpStm) << (INT32) 0 << (INT32) 0 << (INT32) 0 << (INT32) 0;
        (*mpStm) << (UINT32) ( 76 + ( nLen << 1 ) + ( (nLen & 1 ) ? 2 : 0 ) );

        // write text
        for( i = 0; i < nLen; i++ )
            (*mpStm) << (sal_Unicode)rText.GetChar( i );

        // padding word
        if( nLen & 1 )
            (*mpStm) << (UINT16) 0;

        // write DX array
        ImplWriteExtent( pDX[ 0 ] );

        if( nLen > 1 )
        {
            for( i = 1; i < ( nLen - 1 ); i++ )
                ImplWriteExtent( pDX[ i ] - pDX[ i - 1 ] );

            ImplWriteExtent( pDX[ nLen - 2 ] / ( nLen - 1 ) );
        }

        ImplEndRecord();
        delete[] pOwnArray;
    }
}

// -----------------------------------------------------------------------------

void EMFWriter::ImplWrite( const GDIMetaFile& rMtf )
{
    for( ULONG i = 0, nCount = rMtf.GetActionCount(); i < nCount; i++ )
    {
        const MetaAction*   pAction = rMtf.GetAction( i );
        const USHORT        nType = pAction->GetType();

        switch( nType )
        {
            case( META_PIXEL_ACTION ):
            {
                const MetaPixelAction* pA = (const MetaPixelAction*) pAction;

                ImplCheckLineAttr();
                ImplBeginRecord( WIN_EMR_SETPIXELV );
                ImplWritePoint( pA->GetPoint() );
                ImplWriteColor( pA->GetColor() );
                ImplEndRecord();
            }
            break;

            case( META_POINT_ACTION ):
            {
                if( maVDev.IsLineColor() )
                {
                    const MetaPointAction* pA = (const MetaPointAction*) pAction;

                    ImplCheckLineAttr();
                    ImplBeginRecord( WIN_EMR_SETPIXELV );
                    ImplWritePoint( pA->GetPoint() );
                    ImplWriteColor( maVDev.GetLineColor() );
                    ImplEndRecord();
                }
            }
            break;

            case( META_LINE_ACTION ):
            {
                if( maVDev.IsLineColor() )
                {
                    const MetaLineAction* pA = (const MetaLineAction*) pAction;

                    ImplCheckLineAttr();

                    ImplBeginRecord( WIN_EMR_MOVETOEX );
                    ImplWritePoint( pA->GetStartPoint() );
                    ImplEndRecord();

                    ImplBeginRecord( WIN_EMR_LINETO );
                    ImplWritePoint( pA->GetEndPoint() );
                    ImplEndRecord();

                    ImplBeginRecord( WIN_EMR_SETPIXELV );
                    ImplWritePoint( pA->GetEndPoint() );
                    ImplWriteColor( maVDev.GetLineColor() );
                    ImplEndRecord();
                }
            }
            break;

            case( META_RECT_ACTION ):
            {
                if( maVDev.IsLineColor() || maVDev.IsFillColor() )
                {
                    const MetaRectAction* pA = (const MetaRectAction*) pAction;

                    ImplCheckFillAttr();
                    ImplCheckLineAttr();

                    ImplBeginRecord( WIN_EMR_RECTANGLE );
                    ImplWriteRect( pA->GetRect() );
                    ImplEndRecord();
                }
            }
            break;

            case( META_ROUNDRECT_ACTION ):
            {
                if( maVDev.IsLineColor() || maVDev.IsFillColor() )
                {
                    const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pAction;

                    ImplCheckFillAttr();
                    ImplCheckLineAttr();

                    ImplBeginRecord( WIN_EMR_ROUNDRECT );
                    ImplWriteRect( pA->GetRect() );
                    ImplWriteSize( Size( pA->GetHorzRound(), pA->GetVertRound() ) );
                    ImplEndRecord();
                }
            }
            break;

            case( META_ELLIPSE_ACTION ):
            {
                if( maVDev.IsLineColor() || maVDev.IsFillColor() )
                {
                    const MetaEllipseAction* pA = (const MetaEllipseAction*) pAction;

                    ImplCheckFillAttr();
                    ImplCheckLineAttr();

                    ImplBeginRecord( WIN_EMR_ELLIPSE );
                    ImplWriteRect( pA->GetRect() );
                    ImplEndRecord();
                }
            }
            break;

            case( META_ARC_ACTION ):
            case( META_PIE_ACTION ):
            case( META_CHORD_ACTION ):
            case( META_POLYGON_ACTION ):
            {
                if( maVDev.IsLineColor() || maVDev.IsFillColor() )
                {
                    Polygon aPoly;

                    switch( nType )
                    {
                        case( META_ARC_ACTION ):
                        {
                            const MetaArcAction* pA = (const MetaArcAction*) pAction;
                            aPoly = Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_ARC );
                        }
                        break;

                        case( META_PIE_ACTION ):
                        {
                            const MetaPieAction* pA = (const MetaPieAction*) pAction;
                            aPoly = Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_PIE );
                        }
                        break;

                        case( META_CHORD_ACTION ):
                        {
                            const MetaChordAction* pA = (const MetaChordAction*) pAction;
                            aPoly = Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_CHORD );
                        }
                        break;

                        case( META_POLYGON_ACTION ):
                            aPoly = ( (const MetaPolygonAction*) pAction )->GetPolygon();
                        break;
                    }

                    ImplWritePolygonRecord( aPoly, nType != META_ARC_ACTION );
                }
            }
            break;

            case( META_POLYLINE_ACTION ):
            {
                if( maVDev.IsLineColor() )
                    ImplWritePolygonRecord( ( (const MetaPolyLineAction*) pAction )->GetPolygon(), FALSE );
            }
            break;

            case( META_POLYPOLYGON_ACTION ):
            {
                if( maVDev.IsLineColor() || maVDev.IsFillColor() )
                    ImplWritePolyPolygonRecord( ( (const MetaPolyPolygonAction*) pAction )->GetPolyPolygon() );
            }
            break;

            case( META_GRADIENT_ACTION ):
            {
                const MetaGradientAction*   pA = (const MetaGradientAction*) pAction;
                GDIMetaFile                 aTmpMtf;

                maVDev.AddGradientActions( pA->GetRect(), pA->GetGradient(), aTmpMtf );
                ImplWrite( aTmpMtf );
            }
            break;

            case META_HATCH_ACTION:
            {
                const MetaHatchAction*  pA = (const MetaHatchAction*) pAction;
                GDIMetaFile             aTmpMtf;

                maVDev.AddHatchActions( pA->GetPolyPolygon(), pA->GetHatch(), aTmpMtf );
                ImplWrite( aTmpMtf );
            }
            break;

            case META_TRANSPARENT_ACTION:
            {
                ImplCheckFillAttr();
                ImplCheckLineAttr();
                ImplWritePolyPolygonRecord( ( (MetaTransparentAction*) pAction )->GetPolyPolygon() );
            }
            break;

            case META_FLOATTRANSPARENT_ACTION:
            {
                const MetaFloatTransparentAction* pA = (const MetaFloatTransparentAction*) pAction;

                GDIMetaFile     aTmpMtf( pA->GetGDIMetaFile() );
                Point           aSrcPt( aTmpMtf.GetPrefMapMode().GetOrigin() );
                const Size      aSrcSize( aTmpMtf.GetPrefSize() );
                const Point     aDestPt( pA->GetPoint() );
                const Size      aDestSize( pA->GetSize() );
                const double    fScaleX = aSrcSize.Width() ? (double) aDestSize.Width() / aSrcSize.Width() : 1.0;
                const double    fScaleY = aSrcSize.Height() ? (double) aDestSize.Height() / aSrcSize.Height() : 1.0;
                long            nMoveX, nMoveY;

                if( fScaleX != 1.0 || fScaleY != 1.0 )
                {
                    aTmpMtf.Scale( fScaleX, fScaleY );
                    aSrcPt.X() = FRound( aSrcPt.X() * fScaleX ), aSrcPt.Y() = FRound( aSrcPt.Y() * fScaleY );
                }

                nMoveX = aDestPt.X() - aSrcPt.X(), nMoveY = aDestPt.Y() - aSrcPt.Y();

                if( nMoveX || nMoveY )
                    aTmpMtf.Move( nMoveX, nMoveY );

                ImplCheckFillAttr();
                ImplCheckLineAttr();
                ImplCheckTextAttr();
                ImplWrite( aTmpMtf );
            }
            break;

            case( META_EPS_ACTION ):
            {
                const MetaEPSAction*    pA = (const MetaEPSAction*) pAction;
                const GDIMetaFile       aGDIMetaFile( pA->GetSubstitute() );
                sal_Bool                bFound = sal_False;

                for( ULONG i = 0, nCount = aGDIMetaFile.GetActionCount(); ( i < nCount ) && !bFound; i++ )
                {
                    const MetaAction* pSubstAct = aGDIMetaFile.GetAction( i );

                    if( pSubstAct->GetType() == META_BMPSCALE_ACTION )
                    {
                        bFound = sal_True;
                        const MetaBmpScaleAction* pBmpScaleAction = (const MetaBmpScaleAction*) pAction;
                        ImplWriteBmpRecord( pBmpScaleAction->GetBitmap(), pBmpScaleAction->GetPoint(),
                                            pBmpScaleAction->GetSize(), WIN_SRCCOPY );
                    }
                }
            }
            break;

            case META_BMP_ACTION:
            {
                const MetaBmpAction* pA = (const MetaBmpAction *) pAction;
                ImplWriteBmpRecord( pA->GetBitmap(), pA->GetPoint(), pA->GetBitmap().GetSizePixel(), WIN_SRCCOPY );
            }
            break;

            case META_BMPSCALE_ACTION:
            {
                const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;
                ImplWriteBmpRecord( pA->GetBitmap(), pA->GetPoint(), pA->GetSize(), WIN_SRCCOPY );
            }
            break;

            case META_BMPSCALEPART_ACTION:
            {
                const MetaBmpScalePartAction*   pA = (const MetaBmpScalePartAction*) pAction;
                Bitmap                          aTmp( pA->GetBitmap() );

                if( aTmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) ) )
                    ImplWriteBmpRecord( aTmp, pA->GetDestPoint(), pA->GetDestSize(), WIN_SRCCOPY );
            }
            break;

            case META_BMPEX_ACTION:
            {
                const MetaBmpExAction*  pA = (const MetaBmpExAction *) pAction;
                Bitmap                  aBmp( pA->GetBitmapEx().GetBitmap() );
                Bitmap                  aMsk( pA->GetBitmapEx().GetMask() );

                if( !!aMsk )
                {
                    aBmp.Replace( aMsk, COL_WHITE );
                    aMsk.Invert();
                    ImplWriteBmpRecord( aMsk, pA->GetPoint(), aMsk.GetSizePixel(), WIN_SRCPAINT );
                    ImplWriteBmpRecord( aBmp, pA->GetPoint(), aBmp.GetSizePixel(), WIN_SRCAND );
                }
                else
                    ImplWriteBmpRecord( aBmp, pA->GetPoint(), aBmp.GetSizePixel(), WIN_SRCCOPY );
            }
            break;

            case META_BMPEXSCALE_ACTION:
            {
                const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;
                Bitmap                      aBmp( pA->GetBitmapEx().GetBitmap() );
                Bitmap                      aMsk( pA->GetBitmapEx().GetMask() );

                if( !!aMsk )
                {
                    aBmp.Replace( aMsk, COL_WHITE );
                    aMsk.Invert();
                    ImplWriteBmpRecord( aMsk, pA->GetPoint(), pA->GetSize(), WIN_SRCPAINT );
                    ImplWriteBmpRecord( aBmp, pA->GetPoint(), pA->GetSize(), WIN_SRCAND );
                }
                else
                    ImplWriteBmpRecord( aBmp, pA->GetPoint(), pA->GetSize(), WIN_SRCCOPY );
            }
            break;

            case META_BMPEXSCALEPART_ACTION:
            {
                const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*) pAction;
                BitmapEx                        aBmpEx( pA->GetBitmapEx() );
                aBmpEx.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                Bitmap                          aBmp( aBmpEx.GetBitmap() );
                Bitmap                          aMsk( aBmpEx.GetMask() );

                if( !!aMsk )
                {
                    aBmp.Replace( aMsk, COL_WHITE );
                    aMsk.Invert();
                    ImplWriteBmpRecord( aMsk, pA->GetDestPoint(), pA->GetDestSize(), WIN_SRCPAINT );
                    ImplWriteBmpRecord( aBmp, pA->GetDestPoint(), pA->GetDestSize(), WIN_SRCAND );
                }
                else
                    ImplWriteBmpRecord( aBmp, pA->GetDestPoint(), pA->GetDestSize(), WIN_SRCCOPY );
            }
            break;

            case META_TEXT_ACTION:
            {
                const MetaTextAction*   pA = (const MetaTextAction*) pAction;
                const String            aText( pA->GetText(), pA->GetIndex(), pA->GetLen() );

                ImplCheckTextAttr();
                ImplWriteTextRecord( pA->GetPoint(), aText, NULL, 0 );
            }
            break;

            case META_TEXTRECT_ACTION:
            {
                const MetaTextRectAction*   pA = (const MetaTextRectAction*) pAction;
                const String                aText( pA->GetText() );

                ImplCheckTextAttr();
                ImplWriteTextRecord( pA->GetRect().TopLeft(), aText, NULL, 0 );
            }
            break;

            case META_TEXTARRAY_ACTION:
            {
                const MetaTextArrayAction*  pA = (const MetaTextArrayAction*) pAction;
                const String                aText( pA->GetText(), pA->GetIndex(), pA->GetLen() );

                ImplCheckTextAttr();
                ImplWriteTextRecord( pA->GetPoint(), aText, pA->GetDXArray(), 0 );
            }
            break;

            case META_STRETCHTEXT_ACTION:
            {
                const MetaStretchTextAction*    pA = (const MetaStretchTextAction*) pAction;
                const String                    aText( pA->GetText(), pA->GetIndex(), pA->GetLen() );

                ImplCheckTextAttr();
                ImplWriteTextRecord( pA->GetPoint(), aText, NULL, pA->GetWidth() );
            }
            break;

            case( META_LINECOLOR_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( &maVDev );
                mbLineChanged = TRUE;
            }
            break;

            case( META_FILLCOLOR_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( &maVDev );
                mbFillChanged = TRUE;
            }
            break;

            case( META_TEXTCOLOR_ACTION ):
            case( META_TEXTLINECOLOR_ACTION ):
            case( META_TEXTFILLCOLOR_ACTION ):
            case( META_TEXTALIGN_ACTION ):
            case( META_FONT_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( &maVDev );
                mbTextChanged = TRUE;
            }
            break;

            case( META_ISECTRECTCLIPREGION_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( &maVDev );

                ImplBeginRecord( WIN_EMR_INTERSECTCLIPRECT );
                ImplWriteRect( ( (MetaISectRectClipRegionAction*) pAction )->GetRect() );
                ImplEndRecord();
            }
            break;

            case( META_CLIPREGION_ACTION ):
            case( META_ISECTREGIONCLIPREGION_ACTION ):
            case( META_MOVECLIPREGION_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( &maVDev );
            }
            break;

            case( META_REFPOINT_ACTION ):
            case( META_MAPMODE_ACTION ):
                ( (MetaAction*) pAction )->Execute( &maVDev );
            break;

            case( META_PUSH_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( &maVDev );

                ImplBeginRecord( WIN_EMR_SAVEDC );
                ImplEndRecord();
            }
            break;

            case( META_POP_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( &maVDev );

                ImplBeginRecord( WIN_EMR_RESTOREDC );
                (*mpStm) << (INT32) -1;
                ImplEndRecord();

                ImplWriteRasterOp( maVDev.GetRasterOp() );
                mbLineChanged = mbFillChanged = mbTextChanged = TRUE;
            }
            break;

            case( META_RASTEROP_ACTION ):
            {
                ( (MetaAction*) pAction )->Execute( &maVDev );
                ImplWriteRasterOp( ( (MetaRasterOpAction*) pAction )->GetRasterOp() );
            }
            break;

            case( META_MASK_ACTION ):
            case( META_MASKSCALE_ACTION ):
            case( META_MASKSCALEPART_ACTION ):
            case( META_WALLPAPER_ACTION ):
            case( META_TEXTLINE_ACTION ):
            case( META_COMMENT_ACTION ):
            case( META_GRADIENTEX_ACTION ):
            {
                // !!! >>> we don't want to support these actions
            }
            break;

            default:
                DBG_ERROR( ( ByteString( "SVGActionWriter::ImplWriteActions: unsupported MetaAction #" ) += ByteString( nType ) ).GetBuffer() );
            break;
        }
    }
}
