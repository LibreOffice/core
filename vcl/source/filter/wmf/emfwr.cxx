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

#include "sal/config.h"

#include <algorithm>

#include "emfwr.hxx"
#include <rtl/strbuf.hxx>
#include <tools/helpers.hxx>
#include <tools/fract.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/dibtools.hxx>
#include <memory>

#define WIN_EMR_POLYGON                     3
#define WIN_EMR_POLYLINE                    4
#define WIN_EMR_POLYBEZIERTO                5
#define WIN_EMR_POLYLINETO                  6
#define WIN_EMR_POLYPOLYGON                 8
#define WIN_EMR_SETWINDOWEXTEX              9
#define WIN_EMR_SETWINDOWORGEX              10
#define WIN_EMR_SETVIEWPORTEXTEX            11
#define WIN_EMR_SETVIEWPORTORGEX            12
#define WIN_EMR_EOF                         14
#define WIN_EMR_SETPIXELV                   15
#define WIN_EMR_SETMAPMODE                  17
#define WIN_EMR_SETBKMODE                   18
#define WIN_EMR_SETROP2                     20
#define WIN_EMR_SETTEXTALIGN                22
#define WIN_EMR_SETTEXTCOLOR                24
#define WIN_EMR_MOVETOEX                    27
#define WIN_EMR_INTERSECTCLIPRECT           30
#define WIN_EMR_SAVEDC                      33
#define WIN_EMR_RESTOREDC                   34
#define WIN_EMR_SELECTOBJECT                37
#define WIN_EMR_CREATEPEN                   38
#define WIN_EMR_CREATEBRUSHINDIRECT         39
#define WIN_EMR_DELETEOBJECT                40
#define WIN_EMR_ELLIPSE                     42
#define WIN_EMR_RECTANGLE                   43
#define WIN_EMR_ROUNDRECT                   44
#define WIN_EMR_LINETO                      54
#define WIN_EMR_BEGINPATH                   59
#define WIN_EMR_ENDPATH                     60
#define WIN_EMR_CLOSEFIGURE                 61
#define WIN_EMR_FILLPATH                    62
#define WIN_EMR_STROKEPATH                  64

#define WIN_EMR_GDICOMMENT                  70
#define WIN_EMR_STRETCHDIBITS               81
#define WIN_EMR_EXTCREATEFONTINDIRECTW      82
#define WIN_EMR_EXTTEXTOUTW                 84

#define WIN_SRCCOPY                         0x00CC0020L
#define WIN_SRCPAINT                        0x00EE0086L
#define WIN_SRCAND                          0x008800C6L
#define WIN_SRCINVERT                       0x00660046L
#define WIN_EMR_COMMENT_EMFPLUS             0x2B464D45L

#define HANDLE_INVALID                      0xffffffff
#define MAXHANDLES                          65000

#define LINE_SELECT                         0x00000001
#define FILL_SELECT                         0x00000002
#define TEXT_SELECT                         0x00000004

/* Text Alignment Options */
#define TA_RIGHT                            2

#define TA_TOP                              0
#define TA_BOTTOM                           8
#define TA_BASELINE                         24
#define TA_RTLREADING                       256

#define MM_ANISOTROPIC                      8

typedef enum
{
  EmfPlusHeader                     = 0x4001,
  EmfPlusEndOfFile                  = 0x4002,
  EmfPlusComment                    = 0x4003,
  EmfPlusGetDC                      = 0x4004,
  EmfPlusMultiFormatStart           = 0x4005,
  EmfPlusMultiFormatSection         = 0x4006,
  EmfPlusMultiFormatEnd             = 0x4007,
  EmfPlusObject                     = 0x4008,
  EmfPlusClear                      = 0x4009,
  EmfPlusFillRects                  = 0x400A,
  EmfPlusDrawRects                  = 0x400B,
  EmfPlusFillPolygon                = 0x400C,
  EmfPlusDrawLines                  = 0x400D,
  EmfPlusFillEllipse                = 0x400E,
  EmfPlusDrawEllipse                = 0x400F,
  EmfPlusFillPie                    = 0x4010,
  EmfPlusDrawPie                    = 0x4011,
  EmfPlusDrawArc                    = 0x4012,
  EmfPlusFillRegion                 = 0x4013,
  EmfPlusFillPath                   = 0x4014,
  EmfPlusDrawPath                   = 0x4015,
  EmfPlusFillClosedCurve            = 0x4016,
  EmfPlusDrawClosedCurve            = 0x4017,
  EmfPlusDrawCurve                  = 0x4018,
  EmfPlusDrawBeziers                = 0x4019,
  EmfPlusDrawImage                  = 0x401A,
  EmfPlusDrawImagePoints            = 0x401B,
  EmfPlusDrawstring                 = 0x401C,
  EmfPlusSetRenderingOrigin         = 0x401D,
  EmfPlusSetAntiAliasMode           = 0x401E,
  EmfPlusSetTextRenderingHint       = 0x401F,
  EmfPlusSetTextContrast            = 0x4020,
  EmfPlusSetInterpolationMode       = 0x4021,
  EmfPlusSetPixelOffsetMode         = 0x4022,
  EmfPlusSetCompositingMode         = 0x4023,
  EmfPlusSetCompositingQuality      = 0x4024,
  EmfPlusSave                       = 0x4025,
  EmfPlusRestore                    = 0x4026,
  EmfPlusBeginContainer             = 0x4027,
  EmfPlusBeginContainerNoParams     = 0x4028,
  EmfPlusEndContainer               = 0x4029,
  EmfPlusSetWorldTransform          = 0x402A,
  EmfPlusResetWorldTransform        = 0x402B,
  EmfPlusMultiplyWorldTransform     = 0x402C,
  EmfPlusTranslateWorldTransform    = 0x402D,
  EmfPlusScaleWorldTransform        = 0x402E,
  EmfPlusRotateWorldTransform       = 0x402F,
  EmfPlusSetPageTransform           = 0x4030,
  EmfPlusResetClip                  = 0x4031,
  EmfPlusSetClipRect                = 0x4032,
  EmfPlusSetClipPath                = 0x4033,
  EmfPlusSetClipRegion              = 0x4034,
  EmfPlusOffsetClip                 = 0x4035,
  EmfPlusDrawDriverstring           = 0x4036,
  EmfPlusStrokeFillPath             = 0x4037,
  EmfPlusSerializableObject         = 0x4038,
  EmfPlusSetTSGraphics              = 0x4039,
  EmfPlusSetTSClip                  = 0x403A
} EmfPlusRecordType;

void EMFWriter::ImplBeginCommentRecord( sal_Int32 nCommentType )
{
    ImplBeginRecord( WIN_EMR_GDICOMMENT );
    m_rStm.SeekRel( 4 );
    m_rStm.WriteInt32( nCommentType );
}

void EMFWriter::ImplEndCommentRecord()
{
    if( mbRecordOpen )
    {
        sal_Int32 nActPos = m_rStm.Tell();
        m_rStm.Seek( mnRecordPos + 8 );
        m_rStm.WriteUInt32( nActPos - mnRecordPos - 0xc );
        m_rStm.Seek( nActPos );
    }
    ImplEndRecord();
}

void EMFWriter::ImplBeginPlusRecord( sal_uInt16 nType, sal_uInt16 nFlags )
{
    DBG_ASSERT( !mbRecordPlusOpen, "Another EMF+ record is already opened!" );

    if( !mbRecordPlusOpen )
    {
        mbRecordPlusOpen = true;
        mnRecordPlusPos = m_rStm.Tell();

        m_rStm.WriteUInt16( nType ).WriteUInt16( nFlags );
        m_rStm.SeekRel( 8 );
    }
}

void EMFWriter::ImplEndPlusRecord()
{
    DBG_ASSERT( mbRecordPlusOpen, "EMF+ Record was not opened!" );

    if( mbRecordPlusOpen )
    {
        sal_Int32 nActPos = m_rStm.Tell();
        sal_Int32 nSize = nActPos - mnRecordPlusPos;
        m_rStm.Seek( mnRecordPlusPos + 4 );
        m_rStm.WriteUInt32( nSize )         // Size
              .WriteUInt32( nSize - 0xc ); // Data Size
        m_rStm.Seek( nActPos );
        mbRecordPlusOpen = false;
    }
}

void EMFWriter::ImplPlusRecord( sal_uInt16 nType, sal_uInt16 nFlags )
{
    ImplBeginPlusRecord( nType, nFlags );
    ImplEndPlusRecord();
}

void EMFWriter::WriteEMFPlusHeader( const Size &rMtfSizePix, const Size &rMtfSizeLog )
{
    ImplBeginCommentRecord( WIN_EMR_COMMENT_EMFPLUS );

    sal_Int32 nDPIX = rMtfSizePix.Width()*25;
    sal_Int32 nDivX = rMtfSizeLog.Width()/100;
    if (nDivX)
        nDPIX /= nDivX;    // DPI X

    sal_Int32 nDPIY = rMtfSizePix.Height()*25;
    sal_Int32 nDivY = rMtfSizeLog.Height()/100;
    if (nDivY)
        nDPIY /= nDivY; // DPI Y

    m_rStm.WriteInt16( EmfPlusHeader );
    m_rStm.WriteInt16( 0x01 )  // Flags - Dual Mode // TODO: Check this
          .WriteInt32( 0x1C )  // Size
          .WriteInt32( 0x10 )  // Data Size
          .WriteInt32( 0xdbc01002 ) // (lower 12bits) 1-> v1 2-> v1.1 // TODO: Check this
          .WriteInt32( 0x01 ) // Video display
          .WriteInt32( nDPIX )
          .WriteInt32( nDPIY );
    ImplEndCommentRecord();

    // Write more properties
    ImplBeginCommentRecord( WIN_EMR_COMMENT_EMFPLUS );
    ImplPlusRecord( EmfPlusSetPixelOffsetMode, 0x0 );
    ImplPlusRecord( EmfPlusSetAntiAliasMode, 0x09 );      // TODO: Check actual values for AntiAlias
    ImplPlusRecord( EmfPlusSetCompositingQuality, 0x0100 ); // Default Quality
    ImplPlusRecord( EmfPlusSetPageTransform, 1 );
    ImplPlusRecord( EmfPlusSetInterpolationMode, 0x00 );  // Default
    ImplPlusRecord( EmfPlusGetDC, 0x00 );
    ImplEndCommentRecord();
}

void EMFWriter::ImplWritePlusEOF()
{
    ImplBeginCommentRecord( WIN_EMR_COMMENT_EMFPLUS );
    ImplPlusRecord( EmfPlusEndOfFile, 0x0 );
    ImplEndCommentRecord();
}

void EMFWriter::ImplWritePlusColor( const Color& rColor, const sal_uInt32& nTrans )
{
    sal_uInt32 nAlpha = ((100-nTrans)*0xFF)/100;
    sal_uInt32 nCol = rColor.GetBlue();

    nCol |= ( (sal_uInt32) rColor.GetGreen() ) << 8;
    nCol |= ( (sal_uInt32) rColor.GetRed() ) << 16;
    nCol |= ( nAlpha << 24 );
    m_rStm.WriteUInt32( nCol );
}

void EMFWriter::ImplWritePlusPoint( const Point& rPoint )
{
    // Convert to pixels
    const Point aPoint(maVDev->LogicToPixel( rPoint, maDestMapMode ));
    m_rStm.WriteUInt16( aPoint.X() ).WriteUInt16( aPoint.Y() );
}

void EMFWriter::ImplWritePlusFillPolygonRecord( const tools::Polygon& rPoly, const sal_uInt32& nTrans )
{
    ImplBeginCommentRecord( WIN_EMR_COMMENT_EMFPLUS );
    if( rPoly.GetSize() )
    {
        ImplBeginPlusRecord( EmfPlusFillPolygon, 0xC000 ); // Sets the color as well
        ImplWritePlusColor( maVDev->GetFillColor(), nTrans );
        m_rStm.WriteUInt32( rPoly.GetSize() );
        for( sal_uInt16 i = 0; i < rPoly.GetSize(); i++ )
            ImplWritePlusPoint( rPoly[ i ] );
        ImplEndPlusRecord();
    }
    ImplEndCommentRecord();
}

bool EMFWriter::WriteEMF(const GDIMetaFile& rMtf)
{
    const sal_uLong nHeaderPos = m_rStm.Tell();

    maVDev->EnableOutput( false );
    maVDev->SetMapMode( rMtf.GetPrefMapMode() );
    // don't work with pixel as destination map mode -> higher resolution preferrable
    maDestMapMode.SetMapUnit( MAP_100TH_MM );
    mpHandlesUsed = new bool[ MAXHANDLES ];
    memset( mpHandlesUsed, 0, MAXHANDLES * sizeof( bool ) );
    mnHandleCount = mnRecordCount = mnRecordPos = mnRecordPlusPos = 0;
    mbRecordOpen = mbRecordPlusOpen = false;
    mbLineChanged = mbFillChanged = mbTextChanged = false;
    mnLineHandle = mnFillHandle = mnTextHandle = HANDLE_INVALID;
    mnHorTextAlign = 0;

    const Size aMtfSizePix( maVDev->LogicToPixel( rMtf.GetPrefSize(), rMtf.GetPrefMapMode() ) );
    const Size aMtfSizeLog( OutputDevice::LogicToLogic( rMtf.GetPrefSize(), rMtf.GetPrefMapMode(), MAP_100TH_MM ) );

    // seek over header
    // use [MS-EMF 2.2.11] HeaderExtension2 Object, otherwise resulting EMF cannot be converted with GetWinMetaFileBits()
    m_rStm.SeekRel( 108 );

    // Write EMF+ Header
    WriteEMFPlusHeader( aMtfSizePix, aMtfSizeLog );

    // write initial values

    // set 100th mm map mode in EMF
    ImplBeginRecord( WIN_EMR_SETMAPMODE );
    m_rStm.WriteInt32( MM_ANISOTROPIC );
    ImplEndRecord();

    ImplBeginRecord( WIN_EMR_SETVIEWPORTEXTEX );
    m_rStm.WriteInt32( maVDev->GetDPIX() ).WriteInt32( maVDev->GetDPIY() );
    ImplEndRecord();

    ImplBeginRecord( WIN_EMR_SETWINDOWEXTEX );
    m_rStm.WriteInt32( 2540 ).WriteInt32( 2540 );
    ImplEndRecord();

    ImplBeginRecord( WIN_EMR_SETVIEWPORTORGEX );
    m_rStm.WriteInt32( 0 ).WriteInt32( 0 );
    ImplEndRecord();

    ImplBeginRecord( WIN_EMR_SETWINDOWORGEX );
    m_rStm.WriteInt32( 0 ).WriteInt32( 0 );
    ImplEndRecord();

    ImplWriteRasterOp( ROP_OVERPAINT );

    ImplBeginRecord( WIN_EMR_SETBKMODE );
    m_rStm.WriteUInt32( 1 ); // TRANSPARENT
    ImplEndRecord();

    // write emf data
    ImplWrite( rMtf );

    ImplWritePlusEOF();

    ImplBeginRecord( WIN_EMR_EOF );
    m_rStm.WriteUInt32( 0 )      // nPalEntries
          .WriteUInt32( 0x10 )     // offPalEntries
          .WriteUInt32( 0x14 );    // nSizeLast
    ImplEndRecord();

    // write header
    const sal_uLong nEndPos = m_rStm.Tell(); m_rStm.Seek( nHeaderPos );

    m_rStm.WriteUInt32( 0x00000001 ).WriteUInt32( 108 )   //use [MS-EMF 2.2.11] HeaderExtension2 Object
            .WriteInt32( 0 ).WriteInt32( 0 ).WriteInt32( aMtfSizePix.Width() - 1 ).WriteInt32( aMtfSizePix.Height() - 1 )
            .WriteInt32( 0 ).WriteInt32( 0 ).WriteInt32( aMtfSizeLog.Width() - 1 ).WriteInt32( aMtfSizeLog.Height() - 1 )
            .WriteUInt32( 0x464d4520 ).WriteUInt32( 0x10000 ).WriteUInt32( nEndPos - nHeaderPos )
            .WriteUInt32( mnRecordCount ).WriteUInt16( mnHandleCount + 1 ).WriteUInt16( 0 ).WriteUInt32( 0 ).WriteUInt32( 0 ).WriteUInt32( 0 )
            .WriteInt32( aMtfSizePix.Width() ).WriteInt32( aMtfSizePix.Height() )
            .WriteInt32( aMtfSizeLog.Width() / 100 ).WriteInt32( aMtfSizeLog.Height() / 100 )
            .WriteUInt32( 0 ).WriteUInt32( 0 ).WriteUInt32( 0 )
            .WriteInt32( aMtfSizeLog.Width() * 10 ).WriteInt32( aMtfSizeLog.Height() * 10 ); //use [MS-EMF 2.2.11] HeaderExtension2 Object

    m_rStm.Seek( nEndPos );
    delete[] mpHandlesUsed;

    return( m_rStm.GetError() == ERRCODE_NONE );
}

sal_uLong EMFWriter::ImplAcquireHandle()
{
    sal_uLong nHandle = HANDLE_INVALID;

    for( sal_uLong i = 0; i < MAXHANDLES && ( HANDLE_INVALID == nHandle ); i++ )
    {
        if( !mpHandlesUsed[ i ] )
        {
            mpHandlesUsed[ i ] = true;

            if( ( nHandle = i ) == mnHandleCount )
                mnHandleCount++;
        }
    }

    DBG_ASSERT( nHandle != HANDLE_INVALID, "No more handles available" );
    return( nHandle != HANDLE_INVALID ? nHandle + 1 : HANDLE_INVALID );
}

void EMFWriter::ImplReleaseHandle( sal_uLong nHandle )
{
    DBG_ASSERT( nHandle && ( nHandle < MAXHANDLES ), "Handle out of range" );
    mpHandlesUsed[ nHandle - 1 ] = false;
}

void EMFWriter::ImplBeginRecord( sal_uInt32 nType )
{
    DBG_ASSERT( !mbRecordOpen, "Another record is already opened!" );

    if( !mbRecordOpen )
    {
        mbRecordOpen = true;
        mnRecordPos = m_rStm.Tell();

        m_rStm.WriteUInt32( nType );
        m_rStm.SeekRel( 4 );
    }
}

void EMFWriter::ImplEndRecord()
{
    DBG_ASSERT( mbRecordOpen, "Record was not opened!" );

    if( mbRecordOpen )
    {
        sal_Int32 nFillBytes, nActPos = m_rStm.Tell();
        m_rStm.Seek( mnRecordPos + 4 );
        nFillBytes = nActPos - mnRecordPos;
        nFillBytes += 3;    // each record has to be dword aligned
        nFillBytes ^= 3;
        nFillBytes &= 3;
        m_rStm.WriteUInt32( ( nActPos - mnRecordPos ) + nFillBytes );
        m_rStm.Seek( nActPos );
        while( nFillBytes-- )
            m_rStm.WriteUChar( 0 );
        mnRecordCount++;
        mbRecordOpen = false;
    }
}

bool EMFWriter::ImplPrepareHandleSelect( sal_uInt32& rHandle, sal_uLong nSelectType )
{
    if( rHandle != HANDLE_INVALID )
    {
        sal_uInt32 nStockObject = 0x80000000;

        if( LINE_SELECT == nSelectType )
            nStockObject |= 0x00000007;
        else if( FILL_SELECT == nSelectType )
            nStockObject |= 0x00000001;
        else if( TEXT_SELECT == nSelectType )
            nStockObject |= 0x0000000a;

        // select stock object first
        ImplBeginRecord( WIN_EMR_SELECTOBJECT );
        m_rStm.WriteUInt32( nStockObject );
        ImplEndRecord();

        // destroy handle of created object
        ImplBeginRecord( WIN_EMR_DELETEOBJECT );
        m_rStm.WriteUInt32( rHandle );
        ImplEndRecord();

        // mark handle as free
        ImplReleaseHandle( rHandle );
    }

    rHandle = ImplAcquireHandle();

    return( HANDLE_INVALID != rHandle );
}

void EMFWriter::ImplCheckLineAttr()
{
    if( mbLineChanged && ImplPrepareHandleSelect( mnLineHandle, LINE_SELECT ) )
    {
        sal_uInt32 nStyle = maVDev->IsLineColor() ? 0 : 5;
        sal_uInt32 nWidth = 0, nHeight = 0;

        ImplBeginRecord( WIN_EMR_CREATEPEN );
        m_rStm.WriteUInt32( mnLineHandle ).WriteUInt32( nStyle ).WriteUInt32( nWidth ).WriteUInt32( nHeight );
        ImplWriteColor( maVDev->GetLineColor() );
        ImplEndRecord();

        ImplBeginRecord( WIN_EMR_SELECTOBJECT );
        m_rStm.WriteUInt32( mnLineHandle );
        ImplEndRecord();
    }
}

void EMFWriter::ImplCheckFillAttr()
{
    if( mbFillChanged && ImplPrepareHandleSelect( mnFillHandle, FILL_SELECT ) )
    {
        sal_uInt32 nStyle = maVDev->IsFillColor() ? 0 : 1;
        sal_uInt32 nPatternStyle = 0;

        ImplBeginRecord( WIN_EMR_CREATEBRUSHINDIRECT );
        m_rStm.WriteUInt32( mnFillHandle ).WriteUInt32( nStyle );
        ImplWriteColor( maVDev->GetFillColor() );
        m_rStm.WriteUInt32( nPatternStyle );
        ImplEndRecord();

        ImplBeginRecord( WIN_EMR_SELECTOBJECT );
        m_rStm.WriteUInt32( mnFillHandle );
        ImplEndRecord();
    }
}

void EMFWriter::ImplCheckTextAttr()
{
    if( mbTextChanged && ImplPrepareHandleSelect( mnTextHandle, TEXT_SELECT ) )
    {
        const vcl::Font& rFont = maVDev->GetFont();
        OUString         aFontName( rFont.GetName() );
        sal_Int32        nWeight;
        sal_uInt16       i;
        sal_uInt8        nPitchAndFamily;

        ImplBeginRecord( WIN_EMR_EXTCREATEFONTINDIRECTW );
        m_rStm.WriteUInt32( mnTextHandle );
        ImplWriteExtent( -rFont.GetSize().Height() );
        ImplWriteExtent( rFont.GetSize().Width() );
        m_rStm.WriteInt32( rFont.GetOrientation() ).WriteInt32( rFont.GetOrientation() );

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

        m_rStm.WriteInt32( nWeight );
        m_rStm.WriteUChar( ( ITALIC_NONE == rFont.GetItalic() ) ? 0 : 1 );
        m_rStm.WriteUChar( ( UNDERLINE_NONE == rFont.GetUnderline() ) ? 0 : 1 );
        m_rStm.WriteUChar( ( STRIKEOUT_NONE == rFont.GetStrikeout() ) ? 0 : 1 );
        m_rStm.WriteUChar( ( RTL_TEXTENCODING_SYMBOL == rFont.GetCharSet() ) ? 2 : 0 );
        m_rStm.WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 );

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

        m_rStm.WriteUChar( nPitchAndFamily );

        for( i = 0; i < 32; i++ )
            m_rStm.WriteUInt16( ( i < aFontName.getLength() ) ? aFontName[ i ] : 0 );

        // dummy elfFullName
        for( i = 0; i < 64; i++ )
            m_rStm.WriteUInt16( 0 );

        // dummy elfStyle
        for( i = 0; i < 32; i++ )
            m_rStm.WriteUInt16( 0 );

        // dummy elfVersion, elfStyleSize, elfMatch, elfReserved
        m_rStm.WriteUInt32( 0 ).WriteUInt32( 0 ).WriteUInt32( 0 ).WriteUInt32( 0 ) ;

        // dummy elfVendorId
        m_rStm.WriteUInt32( 0 );

        // dummy elfCulture
        m_rStm.WriteUInt32( 0 );

        // dummy elfPanose
        m_rStm.WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 );

        // fill record to get a record size divideable by 4
        m_rStm.WriteUInt16( 0 );

        ImplEndRecord();

        // TextAlign
        sal_uInt32 nTextAlign;

        switch( rFont.GetAlign() )
        {
            case ALIGN_TOP:    nTextAlign = TA_TOP; break;
            case ALIGN_BOTTOM: nTextAlign = TA_BOTTOM; break;
            default:           nTextAlign = TA_BASELINE; break;
        }
        nTextAlign |= mnHorTextAlign;

        ImplBeginRecord( WIN_EMR_SETTEXTALIGN );
        m_rStm.WriteUInt32( nTextAlign );
        ImplEndRecord();

        // Text color
        ImplBeginRecord( WIN_EMR_SETTEXTCOLOR );
        ImplWriteColor( maVDev->GetTextColor() );
        ImplEndRecord();

        ImplBeginRecord( WIN_EMR_SELECTOBJECT );
        m_rStm.WriteUInt32( mnTextHandle );
        ImplEndRecord();
    }
}

void EMFWriter::ImplWriteColor( const Color& rColor )
{
    sal_uInt32 nCol = rColor.GetRed();

    nCol |= ( (sal_uInt32) rColor.GetGreen() ) << 8;
    nCol |= ( (sal_uInt32) rColor.GetBlue() ) << 16;

    m_rStm.WriteUInt32( nCol );
}

void EMFWriter::ImplWriteRasterOp( RasterOp eRop )
{
    sal_uInt32 nROP2;

    switch( eRop )
    {
        case ROP_INVERT: nROP2 = 6; break;
        case ROP_XOR:    nROP2 = 7; break;
        default:         nROP2 = 13;break;
    }

    ImplBeginRecord( WIN_EMR_SETROP2 );
    m_rStm.WriteUInt32( nROP2 );
    ImplEndRecord();
}

void EMFWriter::ImplWriteExtent( long nExtent )
{
    nExtent = OutputDevice::LogicToLogic( Size( nExtent, 0 ), maVDev->GetMapMode(), maDestMapMode ).Width();
    m_rStm.WriteInt32( nExtent );
}

void EMFWriter::ImplWritePoint( const Point& rPoint )
{
    const Point aPoint( OutputDevice::LogicToLogic( rPoint, maVDev->GetMapMode(), maDestMapMode ));
     m_rStm.WriteInt32( aPoint.X() ).WriteInt32( aPoint.Y() );
}

void EMFWriter::ImplWriteSize( const Size& rSize)
{
    const Size aSize( OutputDevice::LogicToLogic( rSize, maVDev->GetMapMode(), maDestMapMode ));
     m_rStm.WriteInt32( aSize.Width() ).WriteInt32( aSize.Height() );
}

void EMFWriter::ImplWriteRect( const Rectangle& rRect )
{
    const Rectangle aRect( OutputDevice::LogicToLogic ( rRect, maVDev->GetMapMode(), maDestMapMode ));
    m_rStm
       .WriteInt32( aRect.Left() )
       .WriteInt32( aRect.Top() )
       .WriteInt32( aRect.Right() )
       .WriteInt32( aRect.Bottom() );
}

void EMFWriter::ImplWritePolygonRecord( const tools::Polygon& rPoly, bool bClose )
{
    if( rPoly.GetSize() )
    {
        if( rPoly.HasFlags() )
            ImplWritePath( rPoly, bClose );
        else
        {
            if( bClose )
                ImplCheckFillAttr();

            ImplCheckLineAttr();

            ImplBeginRecord( bClose ? WIN_EMR_POLYGON : WIN_EMR_POLYLINE );
            ImplWriteRect( rPoly.GetBoundRect() );
            m_rStm.WriteUInt32( rPoly.GetSize() );

            for( sal_uInt16 i = 0; i < rPoly.GetSize(); i++ )
                ImplWritePoint( rPoly[ i ] );

            ImplEndRecord();
        }
    }
}

void EMFWriter::ImplWritePolyPolygonRecord( const tools::PolyPolygon& rPolyPoly )
{
    sal_uInt16 n, i, nPolyCount = rPolyPoly.Count();

    if( nPolyCount )
    {
        if( 1 == nPolyCount )
            ImplWritePolygonRecord( rPolyPoly[ 0 ], true );
        else
        {
            bool    bHasFlags = false;
            sal_uInt32  nTotalPoints = 0;

            for( i = 0; i < nPolyCount; i++ )
            {
                nTotalPoints += rPolyPoly[ i ].GetSize();
                if ( rPolyPoly[ i ].HasFlags() )
                    bHasFlags = true;
            }
            if( nTotalPoints )
            {
                if ( bHasFlags )
                    ImplWritePath( rPolyPoly, true );
                else
                {
                    ImplCheckFillAttr();
                    ImplCheckLineAttr();

                    ImplBeginRecord( WIN_EMR_POLYPOLYGON );
                    ImplWriteRect( rPolyPoly.GetBoundRect() );
                    m_rStm.WriteUInt32( nPolyCount ).WriteUInt32( nTotalPoints );

                    for( i = 0; i < nPolyCount; i++ )
                        m_rStm.WriteUInt32( rPolyPoly[ i ].GetSize() );

                    for( i = 0; i < nPolyCount; i++ )
                    {
                        const tools::Polygon& rPoly = rPolyPoly[ i ];

                        for( n = 0; n < rPoly.GetSize(); n++ )
                            ImplWritePoint( rPoly[ n ] );
                    }
                    ImplEndRecord();
                }
            }
        }
    }
}

void EMFWriter::ImplWritePath( const tools::PolyPolygon& rPolyPoly, bool bClosed )
{
    if ( bClosed )
        ImplCheckFillAttr();
    ImplCheckLineAttr();

    ImplBeginRecord( WIN_EMR_BEGINPATH );
    ImplEndRecord();

    sal_uInt16 i, n, o, nPolyCount = rPolyPoly.Count();
    for ( i = 0; i < nPolyCount; i++ )
    {
        n = 0;
        const tools::Polygon& rPoly = rPolyPoly[ i ];
        while ( n < rPoly.GetSize() )
        {
            if( n == 0 )
            {
                ImplBeginRecord( WIN_EMR_MOVETOEX );
                ImplWritePoint( rPoly[ 0 ] );
                ImplEndRecord();
                n++;
                continue;
            }

            sal_uInt16 nBezPoints = 0;

            while ( ( ( nBezPoints + n + 2 ) < rPoly.GetSize() ) && ( rPoly.GetFlags( nBezPoints + n ) == POLY_CONTROL ) )
                nBezPoints += 3;

            if ( nBezPoints )
            {
                ImplBeginRecord( WIN_EMR_POLYBEZIERTO );
                tools::Polygon aNewPoly( nBezPoints + 1 );
                aNewPoly[ 0 ] = rPoly[ n - 1 ];
                for ( o = 0; o < nBezPoints; o++ )
                    aNewPoly[ o + 1 ] = rPoly[ n + o ];
                ImplWriteRect( aNewPoly.GetBoundRect() );
                m_rStm.WriteUInt32( nBezPoints );
                for( o = 1; o < aNewPoly.GetSize(); o++ )
                    ImplWritePoint( aNewPoly[ o ] );
                ImplEndRecord();
                n = n + nBezPoints;
            }
            else
            {
                sal_uInt16 nPoints = 1;
                while( ( nPoints + n ) < rPoly.GetSize() && ( rPoly.GetFlags( nPoints + n ) != POLY_CONTROL ) )
                    nPoints++;

                if ( nPoints > 1 )
                {
                    ImplBeginRecord( WIN_EMR_POLYLINETO );
                    tools::Polygon aNewPoly( nPoints + 1 );
                    aNewPoly[ 0 ] = rPoly[ n - 1];
                    for ( o = 1; o <= nPoints; o++ )
                        aNewPoly[ o ] = rPoly[ n - 1 + o ];
                    ImplWriteRect( aNewPoly.GetBoundRect() );
                    m_rStm.WriteUInt32( nPoints );
                    for( o = 1; o < aNewPoly.GetSize(); o++ )
                        ImplWritePoint( aNewPoly[ o ] );
                    ImplEndRecord();
                }
                else
                {
                    ImplBeginRecord( WIN_EMR_LINETO );
                    ImplWritePoint( rPoly[ n ] );
                    ImplEndRecord();
                }
                n = n + nPoints;
            }
            if ( bClosed && ( n == rPoly.GetSize() ) )
            {
                ImplBeginRecord( WIN_EMR_CLOSEFIGURE );
                ImplEndRecord();
            }
        }
    }
    ImplBeginRecord( WIN_EMR_ENDPATH );
    ImplEndRecord();
    ImplBeginRecord( bClosed ? WIN_EMR_FILLPATH : WIN_EMR_STROKEPATH );
    ImplWriteRect( rPolyPoly.GetBoundRect() );
    ImplEndRecord();
}

void EMFWriter::ImplWriteBmpRecord( const Bitmap& rBmp, const Point& rPt,
                                    const Size& rSz, sal_uInt32 nROP )
{
    if( !!rBmp )
    {
        SvMemoryStream  aMemStm( 65535, 65535 );
        const Size      aBmpSizePixel( rBmp.GetSizePixel() );

        ImplBeginRecord( WIN_EMR_STRETCHDIBITS );
        ImplWriteRect( Rectangle( rPt, rSz ) );
        ImplWritePoint( rPt );
        m_rStm.WriteInt32( 0 ).WriteInt32( 0 ).WriteInt32( aBmpSizePixel.Width() ).WriteInt32( aBmpSizePixel.Height() );

        // write offset positions and sizes later
        const sal_uLong nOffPos = m_rStm.Tell();
        m_rStm.SeekRel( 16 );

        m_rStm.WriteUInt32( 0 ).WriteInt32( ( ROP_XOR == maVDev->GetRasterOp() && WIN_SRCCOPY == nROP ) ? WIN_SRCINVERT : nROP );
        ImplWriteSize( rSz );

        WriteDIB(rBmp, aMemStm, true, false);

        sal_uInt32  nDIBSize = aMemStm.Tell(), nHeaderSize, nCompression, nColsUsed, nPalCount, nImageSize;
        sal_uInt16  nBitCount;

        // get DIB parameters
        aMemStm.Seek( 0 );
        aMemStm.ReadUInt32( nHeaderSize );
        aMemStm.SeekRel( 10 );
        aMemStm.ReadUInt16( nBitCount ).ReadUInt32( nCompression ).ReadUInt32( nImageSize );
        aMemStm.SeekRel( 8 );
        aMemStm.ReadUInt32( nColsUsed );

        if (nBitCount <= 8)
        {
            if (nColsUsed)
                nPalCount = nColsUsed;
            else
                nPalCount = 1 << (sal_uInt32)nBitCount;
        }
        else
        {
            if (nCompression == BITFIELDS)
                nPalCount = 3;
            else
                nPalCount = 0;
        }

        sal_uInt32 nPalSize = nPalCount * 4;

        m_rStm.Write( aMemStm.GetData(), nDIBSize );

        const sal_uLong nEndPos = m_rStm.Tell();
        m_rStm.Seek( nOffPos );
        m_rStm.WriteUInt32( 80 ).WriteUInt32( nHeaderSize + nPalSize );
        m_rStm.WriteUInt32( 80 + nHeaderSize + nPalSize ).WriteUInt32( nImageSize );
        m_rStm.Seek( nEndPos );

        ImplEndRecord();
    }
}

void EMFWriter::ImplWriteTextRecord( const Point& rPos, const OUString& rText, const long* pDXArray, sal_uInt32 nWidth )
{
    sal_Int32 nLen = rText.getLength(), i;

    if( nLen )
    {
        sal_uInt32  nNormWidth;
        std::unique_ptr<long[]> pOwnArray;
        long*  pDX;

        // get text sizes
        if( pDXArray )
        {
            nNormWidth = maVDev->GetTextWidth( rText );
            pDX = const_cast<long*>(pDXArray);
        }
        else
        {
            pOwnArray.reset(new long[ nLen ]);
            nNormWidth = maVDev->GetTextArray( rText, pOwnArray.get() );
            pDX = pOwnArray.get();
        }

        if( nLen > 1 )
        {
            nNormWidth = pDX[ nLen - 2 ] + maVDev->GetTextWidth( OUString(rText[ nLen - 1 ]) );

            if( nWidth && nNormWidth && ( nWidth != nNormWidth ) )
            {
                const double fFactor = (double) nWidth / nNormWidth;

                for( i = 0; i < ( nLen - 1 ); i++ )
                    pDX[ i ] = FRound( pDX[ i ] * fFactor );
            }
        }

        // write text record
        ImplBeginRecord( WIN_EMR_EXTTEXTOUTW );

        ImplWriteRect( Rectangle( rPos, Size( nNormWidth, maVDev->GetTextHeight() ) ) );
        m_rStm.WriteUInt32( 1 );
        m_rStm.WriteInt32( 0 ).WriteInt32( 0 );
        ImplWritePoint( rPos );
        m_rStm.WriteUInt32( nLen ).WriteUInt32( 76 ).WriteUInt32( 2 );
        m_rStm.WriteInt32( 0 ).WriteInt32( 0 ).WriteInt32( 0 ).WriteInt32( 0 );
        m_rStm.WriteUInt32( 76 + ( nLen << 1 ) + ( (nLen & 1 ) ? 2 : 0 ) );

        // write text
        for( i = 0; i < nLen; i++ )
            m_rStm.WriteUInt16( rText[ i ] );

        // padding word
        if( nLen & 1 )
            m_rStm.WriteUInt16( 0 );

        // write DX array
        ImplWriteExtent( pDX[ 0 ] );

        if( nLen > 1 )
        {
            for( i = 1; i < ( nLen - 1 ); i++ )
                ImplWriteExtent( pDX[ i ] - pDX[ i - 1 ] );

            ImplWriteExtent( pDX[ nLen - 2 ] / ( nLen - 1 ) );
        }

        ImplEndRecord();
    }
}

void EMFWriter::Impl_handleLineInfoPolyPolygons(const LineInfo& rInfo, const basegfx::B2DPolygon& rLinePolygon)
{
    if(rLinePolygon.count())
    {
        basegfx::B2DPolyPolygon aLinePolyPolygon(rLinePolygon);
        basegfx::B2DPolyPolygon aFillPolyPolygon;

        rInfo.applyToB2DPolyPolygon(aLinePolyPolygon, aFillPolyPolygon);

        if(aLinePolyPolygon.count())
        {
            for(sal_uInt32 a(0); a < aLinePolyPolygon.count(); a++)
            {
                const basegfx::B2DPolygon aCandidate(aLinePolyPolygon.getB2DPolygon(a));
                ImplWritePolygonRecord( tools::Polygon(aCandidate), false );
            }
        }

        if(aFillPolyPolygon.count())
        {
            const Color aOldLineColor(maVDev->GetLineColor());
            const Color aOldFillColor(maVDev->GetFillColor());

            maVDev->SetLineColor();
            maVDev->SetFillColor(aOldLineColor);

            for(sal_uInt32 a(0); a < aFillPolyPolygon.count(); a++)
            {
                const tools::Polygon aPolygon(aFillPolyPolygon.getB2DPolygon(a));
                ImplWritePolyPolygonRecord(tools::PolyPolygon( tools::Polygon(aPolygon) ));
            }

            maVDev->SetLineColor(aOldLineColor);
            maVDev->SetFillColor(aOldFillColor);
        }
    }
}

void EMFWriter::ImplWrite( const GDIMetaFile& rMtf )
{
    for( size_t j = 0, nActionCount = rMtf.GetActionSize(); j < nActionCount; j++ )
    {
        const MetaAction*    pAction = rMtf.GetAction( j );
        const MetaActionType nType = pAction->GetType();

        switch( nType )
        {
            case( MetaActionType::PIXEL ):
            {
                const MetaPixelAction* pA = static_cast<const MetaPixelAction*>(pAction);

                ImplCheckLineAttr();
                ImplBeginRecord( WIN_EMR_SETPIXELV );
                ImplWritePoint( pA->GetPoint() );
                ImplWriteColor( pA->GetColor() );
                ImplEndRecord();
            }
            break;

            case( MetaActionType::POINT ):
            {
                if( maVDev->IsLineColor() )
                {
                    const MetaPointAction* pA = static_cast<const MetaPointAction*>(pAction);

                    ImplCheckLineAttr();
                    ImplBeginRecord( WIN_EMR_SETPIXELV );
                    ImplWritePoint( pA->GetPoint() );
                    ImplWriteColor( maVDev->GetLineColor() );
                    ImplEndRecord();
                }
            }
            break;

            case( MetaActionType::LINE ):
            {
                if( maVDev->IsLineColor() )
                {
                    const MetaLineAction* pA = static_cast<const MetaLineAction*>(pAction);

                    if(pA->GetLineInfo().IsDefault())
                    {
                        ImplCheckLineAttr();

                        ImplBeginRecord( WIN_EMR_MOVETOEX );
                        ImplWritePoint( pA->GetStartPoint() );
                        ImplEndRecord();

                        ImplBeginRecord( WIN_EMR_LINETO );
                        ImplWritePoint( pA->GetEndPoint() );
                        ImplEndRecord();

                        ImplBeginRecord( WIN_EMR_SETPIXELV );
                        ImplWritePoint( pA->GetEndPoint() );
                        ImplWriteColor( maVDev->GetLineColor() );
                        ImplEndRecord();
                    }
                    else
                    {
                        // LineInfo used; handle Dash/Dot and fat lines
                        basegfx::B2DPolygon aPolygon;
                        aPolygon.append(basegfx::B2DPoint(pA->GetStartPoint().X(), pA->GetStartPoint().Y()));
                        aPolygon.append(basegfx::B2DPoint(pA->GetEndPoint().X(), pA->GetEndPoint().Y()));
                        Impl_handleLineInfoPolyPolygons(pA->GetLineInfo(), aPolygon);
                    }
                }
            }
            break;

            case( MetaActionType::RECT ):
            {
                if( maVDev->IsLineColor() || maVDev->IsFillColor() )
                {
                    const MetaRectAction* pA = static_cast<const MetaRectAction*>(pAction);

                    ImplCheckFillAttr();
                    ImplCheckLineAttr();

                    ImplBeginRecord( WIN_EMR_RECTANGLE );
                    ImplWriteRect( pA->GetRect() );
                    ImplEndRecord();
                }
            }
            break;

            case( MetaActionType::ROUNDRECT ):
            {
                if( maVDev->IsLineColor() || maVDev->IsFillColor() )
                {
                    const MetaRoundRectAction* pA = static_cast<const MetaRoundRectAction*>(pAction);

                    ImplCheckFillAttr();
                    ImplCheckLineAttr();

                    ImplBeginRecord( WIN_EMR_ROUNDRECT );
                    ImplWriteRect( pA->GetRect() );
                    ImplWriteSize( Size( pA->GetHorzRound(), pA->GetVertRound() ) );
                    ImplEndRecord();
                }
            }
            break;

            case( MetaActionType::ELLIPSE ):
            {
                if( maVDev->IsLineColor() || maVDev->IsFillColor() )
                {
                    const MetaEllipseAction* pA = static_cast<const MetaEllipseAction*>(pAction);

                    ImplCheckFillAttr();
                    ImplCheckLineAttr();

                    ImplBeginRecord( WIN_EMR_ELLIPSE );
                    ImplWriteRect( pA->GetRect() );
                    ImplEndRecord();
                }
            }
            break;

            case( MetaActionType::ARC ):
            case( MetaActionType::PIE ):
            case( MetaActionType::CHORD ):
            case( MetaActionType::POLYGON ):
            {
                if( maVDev->IsLineColor() || maVDev->IsFillColor() )
                {
                    tools::Polygon aPoly;

                    switch( nType )
                    {
                        case( MetaActionType::ARC ):
                        {
                            const MetaArcAction* pA = static_cast<const MetaArcAction*>(pAction);
                            aPoly = tools::Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_ARC );
                        }
                        break;

                        case( MetaActionType::PIE ):
                        {
                            const MetaPieAction* pA = static_cast<const MetaPieAction*>(pAction);
                            aPoly = tools::Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_PIE );
                        }
                        break;

                        case( MetaActionType::CHORD ):
                        {
                            const MetaChordAction* pA = static_cast<const MetaChordAction*>(pAction);
                            aPoly = tools::Polygon( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_CHORD );
                        }
                        break;

                        case( MetaActionType::POLYGON ):
                            aPoly = static_cast<const MetaPolygonAction*>(pAction)->GetPolygon();
                        break;
                        default: break;
                    }

                    ImplWritePolygonRecord( aPoly, nType != MetaActionType::ARC );
                }
            }
            break;

            case( MetaActionType::POLYLINE ):
            {
                if( maVDev->IsLineColor() )
                {
                    const MetaPolyLineAction*   pA = static_cast<const MetaPolyLineAction*>(pAction);
                    const tools::Polygon& rPoly = pA->GetPolygon();

                    if( rPoly.GetSize() )
                    {
                        if(pA->GetLineInfo().IsDefault())
                        {
                            ImplWritePolygonRecord( rPoly, false );
                        }
                        else
                        {
                            // LineInfo used; handle Dash/Dot and fat lines
                            Impl_handleLineInfoPolyPolygons(pA->GetLineInfo(), rPoly.getB2DPolygon());
                        }
                    }
                }
            }
            break;

            case( MetaActionType::POLYPOLYGON ):
            {
                if( maVDev->IsLineColor() || maVDev->IsFillColor() )
                    ImplWritePolyPolygonRecord( static_cast<const MetaPolyPolygonAction*>(pAction)->GetPolyPolygon() );
            }
            break;

            case( MetaActionType::GRADIENT ):
            {
                const MetaGradientAction*   pA = static_cast<const MetaGradientAction*>(pAction);
                GDIMetaFile                 aTmpMtf;

                maVDev->AddGradientActions( pA->GetRect(), pA->GetGradient(), aTmpMtf );
                ImplWrite( aTmpMtf );
            }
            break;

            case MetaActionType::HATCH:
            {
                const MetaHatchAction*  pA = static_cast<const MetaHatchAction*>(pAction);
                GDIMetaFile             aTmpMtf;

                maVDev->AddHatchActions( pA->GetPolyPolygon(), pA->GetHatch(), aTmpMtf );
                ImplWrite( aTmpMtf );
            }
            break;

            case MetaActionType::Transparent:
            {
                const tools::PolyPolygon& rPolyPoly = static_cast<const MetaTransparentAction*>(pAction)->GetPolyPolygon();
                if( rPolyPoly.Count() )
                    ImplWritePlusFillPolygonRecord( rPolyPoly[0], static_cast<const MetaTransparentAction*>(pAction)->GetTransparence() );
                ImplCheckFillAttr();
                ImplCheckLineAttr();
                ImplWritePolyPolygonRecord( rPolyPoly );

                ImplBeginCommentRecord( WIN_EMR_COMMENT_EMFPLUS );
                ImplPlusRecord( EmfPlusGetDC, 0x00 );
                ImplEndCommentRecord();
            }
            break;

            case MetaActionType::FLOATTRANSPARENT:
            {
                const MetaFloatTransparentAction* pA = static_cast<const MetaFloatTransparentAction*>(pAction);

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

            case( MetaActionType::EPS ):
            {
                const MetaEPSAction*    pA = static_cast<const MetaEPSAction*>(pAction);
                const GDIMetaFile       aSubstitute( pA->GetSubstitute() );

                for( size_t i = 0, nCount = aSubstitute.GetActionSize(); i < nCount; i++ )
                {
                    const MetaAction* pSubstAct = aSubstitute.GetAction( i );
                    if( pSubstAct->GetType() == MetaActionType::BMPSCALE )
                    {
                        maVDev->Push();
                        ImplBeginRecord( WIN_EMR_SAVEDC );
                        ImplEndRecord();

                        MapMode aMapMode( aSubstitute.GetPrefMapMode() );
                        Size aOutSize( OutputDevice::LogicToLogic( pA->GetSize(), maVDev->GetMapMode(), aMapMode ) );
                        aMapMode.SetScaleX( Fraction( aOutSize.Width(), aSubstitute.GetPrefSize().Width() ) );
                        aMapMode.SetScaleY( Fraction( aOutSize.Height(), aSubstitute.GetPrefSize().Height() ) );
                        aMapMode.SetOrigin( OutputDevice::LogicToLogic( pA->GetPoint(), maVDev->GetMapMode(), aMapMode ) );
                        maVDev->SetMapMode( aMapMode );
                        ImplWrite( aSubstitute );

                        maVDev->Pop();
                        ImplBeginRecord( WIN_EMR_RESTOREDC );
                        m_rStm.WriteInt32( -1 );
                        ImplEndRecord();
                        break;
                    }
                }
            }
            break;

            case MetaActionType::BMP:
            {
                const MetaBmpAction* pA = static_cast<const MetaBmpAction *>(pAction);
                ImplWriteBmpRecord( pA->GetBitmap(), pA->GetPoint(), maVDev->PixelToLogic( pA->GetBitmap().GetSizePixel() ), WIN_SRCCOPY );
            }
            break;

            case MetaActionType::BMPSCALE:
            {
                const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pAction);
                ImplWriteBmpRecord( pA->GetBitmap(), pA->GetPoint(), pA->GetSize(), WIN_SRCCOPY );
            }
            break;

            case MetaActionType::BMPSCALEPART:
            {
                const MetaBmpScalePartAction*   pA = static_cast<const MetaBmpScalePartAction*>(pAction);
                Bitmap                          aTmp( pA->GetBitmap() );

                if( aTmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) ) )
                    ImplWriteBmpRecord( aTmp, pA->GetDestPoint(), pA->GetDestSize(), WIN_SRCCOPY );
            }
            break;

            case MetaActionType::BMPEX:
            {
                const MetaBmpExAction*  pA = static_cast<const MetaBmpExAction *>(pAction);
                Bitmap                  aBmp( pA->GetBitmapEx().GetBitmap() );
                Bitmap                  aMsk( pA->GetBitmapEx().GetMask() );

                if( !!aMsk )
                {
                    aBmp.Replace( aMsk, COL_WHITE );
                    aMsk.Invert();
                    ImplWriteBmpRecord( aMsk, pA->GetPoint(), maVDev->PixelToLogic( aMsk.GetSizePixel() ), WIN_SRCPAINT );
                    ImplWriteBmpRecord( aBmp, pA->GetPoint(), maVDev->PixelToLogic( aBmp.GetSizePixel() ), WIN_SRCAND );
                }
                else
                    ImplWriteBmpRecord( aBmp, pA->GetPoint(), aBmp.GetSizePixel(), WIN_SRCCOPY );
            }
            break;

            case MetaActionType::BMPEXSCALE:
            {
                const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pAction);
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

            case MetaActionType::BMPEXSCALEPART:
            {
                const MetaBmpExScalePartAction* pA = static_cast<const MetaBmpExScalePartAction*>(pAction);
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

            case MetaActionType::TEXT:
            {
                const MetaTextAction*   pA = static_cast<const MetaTextAction*>(pAction);
                const OUString          aText = pA->GetText().copy( pA->GetIndex(), std::min(pA->GetText().getLength() - pA->GetIndex(), pA->GetLen()) );

                ImplCheckTextAttr();
                ImplWriteTextRecord( pA->GetPoint(), aText, nullptr, 0 );
            }
            break;

            case MetaActionType::TEXTRECT:
            {
                const MetaTextRectAction*   pA = static_cast<const MetaTextRectAction*>(pAction);
                const OUString              aText( pA->GetText() );

                ImplCheckTextAttr();
                ImplWriteTextRecord( pA->GetRect().TopLeft(), aText, nullptr, 0 );
            }
            break;

            case MetaActionType::TEXTARRAY:
            {
                const MetaTextArrayAction*  pA = static_cast<const MetaTextArrayAction*>(pAction);
                const OUString              aText = pA->GetText().copy( pA->GetIndex(), std::min(pA->GetText().getLength() - pA->GetIndex(), pA->GetLen()) );

                ImplCheckTextAttr();
                ImplWriteTextRecord( pA->GetPoint(), aText, pA->GetDXArray(), 0 );
            }
            break;

            case MetaActionType::STRETCHTEXT:
            {
                const MetaStretchTextAction*    pA = static_cast<const MetaStretchTextAction*>(pAction);
                const OUString                  aText = pA->GetText().copy( pA->GetIndex(), std::min(pA->GetText().getLength() - pA->GetIndex(), pA->GetLen()) );

                ImplCheckTextAttr();
                ImplWriteTextRecord( pA->GetPoint(), aText, nullptr, pA->GetWidth() );
            }
            break;

            case( MetaActionType::LINECOLOR ):
            {
                const_cast<MetaAction*>(pAction)->Execute( maVDev );
                mbLineChanged = true;
            }
            break;

            case( MetaActionType::FILLCOLOR ):
            {
                const_cast<MetaAction*>(pAction)->Execute( maVDev );
                mbFillChanged = true;
            }
            break;

            case( MetaActionType::TEXTCOLOR ):
            case( MetaActionType::TEXTLINECOLOR ):
            case( MetaActionType::TEXTFILLCOLOR ):
            case( MetaActionType::TEXTALIGN ):
            case( MetaActionType::FONT ):
            {
                const_cast<MetaAction*>(pAction)->Execute( maVDev );
                mbTextChanged = true;
            }
            break;

            case( MetaActionType::ISECTRECTCLIPREGION ):
            {
                const_cast<MetaAction*>(pAction)->Execute( maVDev );

                ImplBeginRecord( WIN_EMR_INTERSECTCLIPRECT );
                ImplWriteRect( static_cast<const MetaISectRectClipRegionAction*>(pAction)->GetRect() );
                ImplEndRecord();
            }
            break;

            case( MetaActionType::CLIPREGION ):
            case( MetaActionType::ISECTREGIONCLIPREGION ):
            case( MetaActionType::MOVECLIPREGION ):
            {
                const_cast<MetaAction*>(pAction)->Execute( maVDev );
            }
            break;

            case( MetaActionType::REFPOINT ):
            case( MetaActionType::MAPMODE ):
                const_cast<MetaAction*>(pAction)->Execute( maVDev );
            break;

            case( MetaActionType::PUSH ):
            {
                const_cast<MetaAction*>(pAction)->Execute( maVDev );

                ImplBeginRecord( WIN_EMR_SAVEDC );
                ImplEndRecord();
            }
            break;

            case( MetaActionType::POP ):
            {
                const_cast<MetaAction*>(pAction)->Execute( maVDev );

                ImplBeginRecord( WIN_EMR_RESTOREDC );
                m_rStm.WriteInt32( -1 );
                ImplEndRecord();

                ImplWriteRasterOp( maVDev->GetRasterOp() );
                mbLineChanged = mbFillChanged = mbTextChanged = true;
            }
            break;

            case( MetaActionType::RASTEROP ):
            {
                const_cast<MetaAction*>(pAction)->Execute( maVDev );
                ImplWriteRasterOp( static_cast<const MetaRasterOpAction*>(pAction)->GetRasterOp() );
            }
            break;

            case( MetaActionType::LAYOUTMODE ):
            {
                ComplexTextLayoutMode nLayoutMode = static_cast<const MetaLayoutModeAction*>(pAction)->GetLayoutMode();
                mnHorTextAlign = 0;
                if ((nLayoutMode & TEXT_LAYOUT_BIDI_RTL) != TEXT_LAYOUT_DEFAULT)
                {
                    mnHorTextAlign = TA_RIGHT | TA_RTLREADING;
                }
                if ((nLayoutMode & TEXT_LAYOUT_TEXTORIGIN_RIGHT) != TEXT_LAYOUT_DEFAULT)
                    mnHorTextAlign |= TA_RIGHT;
                else if ((nLayoutMode & TEXT_LAYOUT_TEXTORIGIN_LEFT) != TEXT_LAYOUT_DEFAULT)
                    mnHorTextAlign &= ~TA_RIGHT;
                break;
            }

            case( MetaActionType::COMMENT ):
            {
                MetaCommentAction const*const pCommentAction(
                        static_cast<MetaCommentAction const*>(pAction));
                if (pCommentAction->GetComment() == "EMF_PLUS")
                {
                    ImplBeginCommentRecord(WIN_EMR_COMMENT_EMFPLUS);
                    m_rStm.Write(pCommentAction->GetData(),
                                 pCommentAction->GetDataSize());
                    ImplEndCommentRecord();
                }
            }
            break;

            case( MetaActionType::MASK ):
            case( MetaActionType::MASKSCALE ):
            case( MetaActionType::MASKSCALEPART ):
            case( MetaActionType::WALLPAPER ):
            case( MetaActionType::TEXTLINE ):
            case( MetaActionType::GRADIENTEX ):
            {
                // !!! >>> we don't want to support these actions
            }
            break;

            default:
                OSL_FAIL(OStringBuffer(
                    "EMFWriter::ImplWriteActions: unsupported MetaAction #" ).
                     append(static_cast<sal_Int32>(nType)).getStr());
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
