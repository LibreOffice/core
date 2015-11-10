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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/zforlist.hxx>
#include <svl/PasswordHelper.hxx>
#include <cassert>
#include <math.h>
#include <string.h>

#include "global.hxx"
#include "sc.hrc"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "document.hxx"
#include "collect.hxx"
#include "rangenam.hxx"
#include "dbdata.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "filter.hxx"
#include "scflt.hxx"
#include "formulacell.hxx"
#include "scfobj.hxx"
#include "docoptio.hxx"
#include "viewopti.hxx"
#include "postit.hxx"
#include "globstr.hrc"
#include "ftools.hxx"
#include "tabprotection.hxx"

#include "fprogressbar.hxx"
#include <memory>

using namespace com::sun::star;

#define DEFCHARSET          RTL_TEXTENCODING_MS_1252

#define SC10TOSTRING(p)     OUString((p), strlen(p), DEFCHARSET)

const SCCOL SC10MAXCOL = 255;   // #i85906# don't try to load more columns than there are in the file

/** Those strings are used with SC10TOSTRING() and strcmp() and such, hence
    need to be 0-terminated. */
static void lcl_ReadFixedString( SvStream& rStream, void* pData, size_t nLen )
{
    sal_Char* pBuf = static_cast<sal_Char*>(pData);
    if (!nLen)
        pBuf[0] = 0;
    else
    {
        rStream.Read( pBuf, nLen);
        pBuf[nLen-1] = 0;
    }
}

static void lcl_ReadFileHeader(SvStream& rStream, Sc10FileHeader& rFileHeader)
{
    lcl_ReadFixedString( rStream, &rFileHeader.CopyRight, sizeof(rFileHeader.CopyRight));
    rStream.ReadUInt16( rFileHeader.Version );
    rStream.Read(&rFileHeader.Reserved, sizeof(rFileHeader.Reserved));
}

static void lcl_ReadTabProtect(SvStream& rStream, Sc10TableProtect& rProtect)
{
    lcl_ReadFixedString( rStream, &rProtect.PassWord, sizeof(rProtect.PassWord));
    rStream.ReadUInt16( rProtect.Flags );
    rStream.ReadUChar( rProtect.Protect );
}

static void lcl_ReadSheetProtect(SvStream& rStream, Sc10SheetProtect& rProtect)
{
    lcl_ReadFixedString( rStream, &rProtect.PassWord, sizeof(rProtect.PassWord));
    rStream.ReadUInt16( rProtect.Flags );
    rStream.ReadUChar( rProtect.Protect );
}

static void lcl_ReadRGB(SvStream& rStream, Sc10Color& rColor)
{
    rStream.ReadUChar( rColor.Dummy );
    rStream.ReadUChar( rColor.Blue );
    rStream.ReadUChar( rColor.Green );
    rStream.ReadUChar( rColor.Red );
}

static void lcl_ReadPalette(SvStream& rStream, Sc10Color* pPalette)
{
    for (sal_uInt16 i = 0; i < 16; i++)
        lcl_ReadRGB(rStream, pPalette[i]);
}

static void lcl_ReadValueFormat(SvStream& rStream, Sc10ValueFormat& rFormat)
{
    rStream.ReadUChar( rFormat.Format );
    rStream.ReadUChar( rFormat.Info );
}

static void lcl_ReadLogFont(SvStream& rStream, Sc10LogFont& rFont)
{
    rStream.ReadInt16( rFont.lfHeight );
    rStream.ReadInt16( rFont.lfWidth );
    rStream.ReadInt16( rFont.lfEscapement );
    rStream.ReadInt16( rFont.lfOrientation );
    rStream.ReadInt16( rFont.lfWeight );
    rStream.ReadUChar( rFont.lfItalic );
    rStream.ReadUChar( rFont.lfUnderline );
    rStream.ReadUChar( rFont.lfStrikeOut );
    rStream.ReadUChar( rFont.lfCharSet );
    rStream.ReadUChar( rFont.lfOutPrecision );
    rStream.ReadUChar( rFont.lfClipPrecision );
    rStream.ReadUChar( rFont.lfQuality );
    rStream.ReadUChar( rFont.lfPitchAndFamily );
    lcl_ReadFixedString( rStream, &rFont.lfFaceName, sizeof(rFont.lfFaceName));
}

static void lcl_ReadBlockRect(SvStream& rStream, Sc10BlockRect& rBlock)
{
    rStream.ReadInt16( rBlock.x1 );
    rStream.ReadInt16( rBlock.y1 );
    rStream.ReadInt16( rBlock.x2 );
    rStream.ReadInt16( rBlock.y2 );
}

static void lcl_ReadHeadFootLine(SvStream& rStream, Sc10HeadFootLine& rLine)
{
    lcl_ReadFixedString( rStream, &rLine.Title, sizeof(rLine.Title));
    lcl_ReadLogFont(rStream, rLine.LogFont);
    rStream.ReadUChar( rLine.HorJustify );
    rStream.ReadUChar( rLine.VerJustify );
    rStream.ReadUInt16( rLine.Raster );
    rStream.ReadUInt16( rLine.Frame );
    lcl_ReadRGB(rStream, rLine.TextColor);
    lcl_ReadRGB(rStream, rLine.BackColor);
    lcl_ReadRGB(rStream, rLine.RasterColor);
    rStream.ReadUInt16( rLine.FrameColor );
    rStream.ReadUInt16( rLine.Reserved );
}

static void lcl_ReadPageFormat(SvStream& rStream, Sc10PageFormat& rFormat)
{
    lcl_ReadHeadFootLine(rStream, rFormat.HeadLine);
    lcl_ReadHeadFootLine(rStream, rFormat.FootLine);
    rStream.ReadInt16( rFormat.Orientation );
    rStream.ReadInt16( rFormat.Width );
    rStream.ReadInt16( rFormat.Height );
    rStream.ReadInt16( rFormat.NonPrintableX );
    rStream.ReadInt16( rFormat.NonPrintableY );
    rStream.ReadInt16( rFormat.Left );
    rStream.ReadInt16( rFormat.Top );
    rStream.ReadInt16( rFormat.Right );
    rStream.ReadInt16( rFormat.Bottom );
    rStream.ReadInt16( rFormat.Head );
    rStream.ReadInt16( rFormat.Foot );
    rStream.ReadUChar( rFormat.HorCenter );
    rStream.ReadUChar( rFormat.VerCenter );
    rStream.ReadUChar( rFormat.PrintGrid );
    rStream.ReadUChar( rFormat.PrintColRow );
    rStream.ReadUChar( rFormat.PrintNote );
    rStream.ReadUChar( rFormat.TopBottomDir );
    lcl_ReadFixedString( rStream, &rFormat.PrintAreaName, sizeof(rFormat.PrintAreaName));
    lcl_ReadBlockRect(rStream, rFormat.PrintArea);
    rStream.Read(&rFormat.PrnZoom, sizeof(rFormat.PrnZoom));
    rStream.ReadInt16( rFormat.FirstPageNo );
    rStream.ReadInt16( rFormat.RowRepeatStart );
    rStream.ReadInt16( rFormat.RowRepeatEnd );
    rStream.ReadInt16( rFormat.ColRepeatStart );
    rStream.ReadInt16( rFormat.ColRepeatEnd );
    rStream.Read(&rFormat.Reserved, sizeof(rFormat.Reserved));
}

static void lcl_ReadGraphHeader(SvStream& rStream, Sc10GraphHeader& rHeader)
{
    rStream.ReadUChar( rHeader.Typ );
    rStream.ReadInt16( rHeader.CarretX );
    rStream.ReadInt16( rHeader.CarretY );
    rStream.ReadInt16( rHeader.CarretZ );
    rStream.ReadInt32( rHeader.x );
    rStream.ReadInt32( rHeader.y );
    rStream.ReadInt32( rHeader.w );
    rStream.ReadInt32( rHeader.h );
    rStream.ReadUChar( rHeader.IsRelPos );
    rStream.ReadUChar( rHeader.DoPrint );
    rStream.ReadUInt16( rHeader.FrameType );
    rStream.ReadUChar( rHeader.IsTransparent );
    lcl_ReadRGB(rStream, rHeader.FrameColor);
    lcl_ReadRGB(rStream, rHeader.BackColor);
    rStream.Read(&rHeader.Reserved, sizeof(rHeader.Reserved));
}

static void lcl_ReadImageHeaer(SvStream& rStream, Sc10ImageHeader& rHeader)
{
    lcl_ReadFixedString( rStream, &rHeader.FileName, sizeof(rHeader.FileName));
    rStream.ReadInt16( rHeader.Typ );
    rStream.ReadUChar( rHeader.Linked );
    rStream.ReadInt16( rHeader.x1 );
    rStream.ReadInt16( rHeader.y1 );
    rStream.ReadInt16( rHeader.x2 );
    rStream.ReadInt16( rHeader.y2 );
    rStream.ReadUInt32( rHeader.Size );
}

static void lcl_ReadChartHeader(SvStream& rStream, Sc10ChartHeader& rHeader)
{
    rStream.ReadInt16( rHeader.MM );
    rStream.ReadInt16( rHeader.xExt );
    rStream.ReadInt16( rHeader.yExt );
    rStream.ReadUInt32( rHeader.Size );
}

static void lcl_ReadChartSheetData(SvStream& rStream, Sc10ChartSheetData& rSheetData)
{
    rStream.ReadUChar( rSheetData.HasTitle );
    rStream.ReadInt16( rSheetData.TitleX );
    rStream.ReadInt16( rSheetData.TitleY );
    rStream.ReadUChar( rSheetData.HasSubTitle );
    rStream.ReadInt16( rSheetData.SubTitleX );
    rStream.ReadInt16( rSheetData.SubTitleY );
    rStream.ReadUChar( rSheetData.HasLeftTitle );
    rStream.ReadInt16( rSheetData.LeftTitleX );
    rStream.ReadInt16( rSheetData.LeftTitleY );
    rStream.ReadUChar( rSheetData.HasLegend );
    rStream.ReadInt16( rSheetData.LegendX1 );
    rStream.ReadInt16( rSheetData.LegendY1 );
    rStream.ReadInt16( rSheetData.LegendX2 );
    rStream.ReadInt16( rSheetData.LegendY2 );
    rStream.ReadUChar( rSheetData.HasLabel );
    rStream.ReadInt16( rSheetData.LabelX1 );
    rStream.ReadInt16( rSheetData.LabelY1 );
    rStream.ReadInt16( rSheetData.LabelX2 );
    rStream.ReadInt16( rSheetData.LabelY2 );
    rStream.ReadInt16( rSheetData.DataX1 );
    rStream.ReadInt16( rSheetData.DataY1 );
    rStream.ReadInt16( rSheetData.DataX2 );
    rStream.ReadInt16( rSheetData.DataY2 );
    rStream.Read(&rSheetData.Reserved, sizeof(rSheetData.Reserved));
}

static void lcl_ReadChartTypeData(SvStream& rStream, Sc10ChartTypeData& rTypeData)
{
    rStream.ReadInt16( rTypeData.NumSets );
    rStream.ReadInt16( rTypeData.NumPoints );
    rStream.ReadInt16( rTypeData.DrawMode );
    rStream.ReadInt16( rTypeData.GraphType );
    rStream.ReadInt16( rTypeData.GraphStyle );
    lcl_ReadFixedString( rStream, &rTypeData.GraphTitle, sizeof(rTypeData.GraphTitle));
    lcl_ReadFixedString( rStream, &rTypeData.BottomTitle, sizeof(rTypeData.BottomTitle));
    sal_uInt16 i;
    for (i = 0; i < 256; i++)
        rStream.ReadInt16( rTypeData.SymbolData[i] );
    for (i = 0; i < 256; i++)
        rStream.ReadInt16( rTypeData.ColorData[i] );
    for (i = 0; i < 256; i++)
        rStream.ReadInt16( rTypeData.ThickLines[i] );
    for (i = 0; i < 256; i++)
        rStream.ReadInt16( rTypeData.PatternData[i] );
    for (i = 0; i < 256; i++)
        rStream.ReadInt16( rTypeData.LinePatternData[i] );
    for (i = 0; i < 11; i++)
        rStream.ReadInt16( rTypeData.NumGraphStyles[i] );
    rStream.ReadInt16( rTypeData.ShowLegend );
    for (i = 0; i < 256; i++)
        lcl_ReadFixedString( rStream, &rTypeData.LegendText[i], sizeof(Sc10ChartText));
    rStream.ReadInt16( rTypeData.ExplodePie );
    rStream.ReadInt16( rTypeData.FontUse );
    for (i = 0; i < 5; i++)
        rStream.ReadInt16( rTypeData.FontFamily[i] );
    for (i = 0; i < 5; i++)
        rStream.ReadInt16( rTypeData.FontStyle[i] );
    for (i = 0; i < 5; i++)
        rStream.ReadInt16( rTypeData.FontSize[i] );
    rStream.ReadInt16( rTypeData.GridStyle );
    rStream.ReadInt16( rTypeData.Labels );
    rStream.ReadInt16( rTypeData.LabelEvery );
    for (i = 0; i < 50; i++)
        lcl_ReadFixedString( rStream, &rTypeData.LabelText[i], sizeof(Sc10ChartText));
    lcl_ReadFixedString( rStream, &rTypeData.LeftTitle, sizeof(rTypeData.LeftTitle));
    rStream.Read(&rTypeData.Reserved, sizeof(rTypeData.Reserved));
}

static double lcl_PascalToDouble(sal_Char* tp6)
{
    sal_uInt8* pnUnsigned = reinterpret_cast< sal_uInt8* >( tp6 );
    // biased exponent
    sal_uInt8 be = pnUnsigned[ 0 ];
    // lower 16 bits of mantissa
    sal_uInt16 v1 = static_cast< sal_uInt16 >( pnUnsigned[ 2 ] * 256 + pnUnsigned[ 1 ] );
    // next 16 bits of mantissa
    sal_uInt16 v2 = static_cast< sal_uInt16 >( pnUnsigned[ 4 ] * 256 + pnUnsigned[ 3 ] );
    // upper 7 bits of mantissa
    sal_uInt8 v3 = static_cast< sal_uInt8 >( pnUnsigned[ 5 ] & 0x7F );
    // sign bit
    bool s = (pnUnsigned[ 5 ] & 0x80) != 0;

    if (be == 0)
        return 0.0;
    return (((((128 + v3) * 65536.0) + v2) * 65536.0 + v1) *
        ldexp ((s ? -1.0 : 1.0), be - (129+39)));
}

static void lcl_ChangeColor( sal_uInt16 nIndex, Color& rColor )
{
    ColorData aCol;

    switch( nIndex )
        {
        case 1:     aCol = COL_RED;             break;
        case 2:     aCol = COL_GREEN;           break;
        case 3:     aCol = COL_BROWN;           break;
        case 4:     aCol = COL_BLUE;            break;
        case 5:     aCol = COL_MAGENTA;         break;
        case 6:     aCol = COL_CYAN;            break;
        case 7:     aCol = COL_GRAY;            break;
        case 8:     aCol = COL_LIGHTGRAY;       break;
        case 9:     aCol = COL_LIGHTRED;        break;
        case 10:    aCol = COL_LIGHTGREEN;      break;
        case 11:    aCol = COL_YELLOW;          break;
        case 12:    aCol = COL_LIGHTBLUE;       break;
        case 13:    aCol = COL_LIGHTMAGENTA;    break;
        case 14:    aCol = COL_LIGHTCYAN;       break;
        case 15:    aCol = COL_WHITE;           break;
        default:    aCol = COL_BLACK;
        }

    rColor.SetColor( aCol );
}

static OUString lcl_MakeOldPageStyleFormatName( sal_uInt16 i )
{
    OUString  aName = ScGlobal::GetRscString( STR_PAGESTYLE ) + " " + OUString::number( i + 1 );
    return aName;
}

template < typename T > sal_uLong insert_new( ScCollection* pCollection, SvStream& rStream )
{
    std::unique_ptr<T> pData(new (::std::nothrow) T( rStream));
    sal_uLong nError = rStream.GetError();
    if (pData)
    {
        if (!nError)
            pCollection->Insert( pData.release() );
    }
    else
        nError = errOutOfMemory;
    return nError;
}

// Font

Sc10FontData::Sc10FontData(SvStream& rStream)
    : Height(0)
    , CharSet(0)
    , PitchAndFamily(0)
{
    rStream.ReadInt16( Height );
    rStream.ReadUChar( CharSet );
    rStream.ReadUChar( PitchAndFamily );
    sal_uInt16 nLen(0);
    rStream.ReadUInt16( nLen );
    if (nLen < sizeof(FaceName))
        rStream.Read(FaceName, nLen);
    else
        rStream.SetError(ERRCODE_IO_WRONGFORMAT);
}

Sc10FontCollection::Sc10FontCollection(SvStream& rStream)
    : ScCollection(4, 4)
    , nError(0)
{
    sal_uInt16 ID(0);
    rStream.ReadUInt16( ID );
    if (ID == FontID)
    {
        sal_uInt16 nAnz(0);
        rStream.ReadUInt16( nAnz );
        for (sal_uInt16 i=0; (i < nAnz) && (nError == 0); i++)
        {
            nError = insert_new<Sc10FontData>( this, rStream);
        }
    }
    else
    {
        OSL_FAIL( "FontID" );
        nError = errUnknownID;
    }
}

// named regions

Sc10NameData::Sc10NameData(SvStream& rStream)
{
    sal_uInt8 nLen;
    rStream.ReadUChar( nLen );
    rStream.Read(Name, sizeof(Name) - 1);
    if (nLen >= sizeof(Name))
        nLen = sizeof(Name) - 1;
    Name[nLen] = 0;

    rStream.ReadUChar( nLen );
    rStream.Read(Reference, sizeof(Reference) - 1);
    if (nLen >= sizeof(Reference))
        nLen = sizeof(Reference) - 1;
    Reference[nLen] = 0;
    rStream.Read(Reserved, sizeof(Reserved));
}

Sc10NameCollection::Sc10NameCollection(SvStream& rStream) :
    ScCollection (4, 4),
    nError     (0)
{
    sal_uInt16 ID;
    rStream.ReadUInt16( ID );
    if (ID == NameID)
    {
        sal_uInt16 nAnz;
        rStream.ReadUInt16( nAnz );
        for (sal_uInt16 i=0; (i < nAnz) && (nError == 0); i++)
        {
            nError = insert_new<Sc10NameData>( this, rStream);
        }
    }
    else
    {
        OSL_FAIL( "NameID" );
        nError = errUnknownID;
    }
}

// templates
Sc10PatternData::Sc10PatternData(SvStream& rStream)
    : Attr(0)
    , Justify(0)
    , Frame(0)
    , Raster(0)
    , nColor(0)
    , FrameColor(0)
    , Flags(0)
    , FormatFlags(0)
{
    memset(Name, 0, sizeof(Name));
    memset(Reserved, 0, sizeof(Reserved));
    lcl_ReadFixedString( rStream, Name, sizeof(Name));
    lcl_ReadValueFormat(rStream, ValueFormat);
    lcl_ReadLogFont(rStream, LogFont);

    rStream.ReadUInt16( Attr );
    rStream.ReadUInt16( Justify );
    rStream.ReadUInt16( Frame );
    rStream.ReadUInt16( Raster );
    rStream.ReadUInt16( nColor );
    rStream.ReadUInt16( FrameColor );
    rStream.ReadUInt16( Flags );
    rStream.ReadUInt16( FormatFlags );
    rStream.Read(Reserved, sizeof(Reserved));
}

Sc10PatternCollection::Sc10PatternCollection(SvStream& rStream)
    : ScCollection(4, 4)
    , nError(0)
{
    sal_uInt16 ID;
    rStream.ReadUInt16( ID );
    if (ID == PatternID)
    {
        sal_uInt16 nAnz;
        rStream.ReadUInt16( nAnz );
        for (sal_uInt16 i=0; (i < nAnz) && (nError == 0); i++)
        {
            nError = insert_new<Sc10PatternData>( this, rStream);
        }
    }
    else
    {
        OSL_FAIL( "PatternID" );
        nError = errUnknownID;
    }
}

// database

Sc10DataBaseData::Sc10DataBaseData(SvStream& rStream)
{
    lcl_ReadFixedString( rStream, &DataBaseRec.Name, sizeof(DataBaseRec.Name));
    rStream.ReadInt16( DataBaseRec.Tab );
    lcl_ReadBlockRect(rStream, DataBaseRec.Block);
    rStream.ReadUChar( DataBaseRec.RowHeader );
    rStream.ReadInt16( DataBaseRec.SortField0 );
    rStream.ReadUChar( DataBaseRec.SortUpOrder0 );
    rStream.ReadInt16( DataBaseRec.SortField1 );
    rStream.ReadUChar( DataBaseRec.SortUpOrder1 );
    rStream.ReadInt16( DataBaseRec.SortField2 );
    rStream.ReadUChar( DataBaseRec.SortUpOrder2 );
    rStream.ReadUChar( DataBaseRec.IncludeFormat );

    rStream.ReadInt16( DataBaseRec.QueryField0 );
    rStream.ReadInt16( DataBaseRec.QueryOp0 );
    rStream.ReadUChar( DataBaseRec.QueryByString0 );
    lcl_ReadFixedString( rStream, &DataBaseRec.QueryString0, sizeof(DataBaseRec.QueryString0));
    DataBaseRec.QueryValue0 = ScfTools::ReadLongDouble(rStream);

    rStream.ReadInt16( DataBaseRec.QueryConnect1 );
    rStream.ReadInt16( DataBaseRec.QueryField1 );
    rStream.ReadInt16( DataBaseRec.QueryOp1 );
    rStream.ReadUChar( DataBaseRec.QueryByString1 );
    lcl_ReadFixedString( rStream, &DataBaseRec.QueryString1, sizeof(DataBaseRec.QueryString1));
    DataBaseRec.QueryValue1 = ScfTools::ReadLongDouble(rStream);

    rStream.ReadInt16( DataBaseRec.QueryConnect2 );
    rStream.ReadInt16( DataBaseRec.QueryField2 );
    rStream.ReadInt16( DataBaseRec.QueryOp2 );
    rStream.ReadUChar( DataBaseRec.QueryByString2 );
    lcl_ReadFixedString( rStream, &DataBaseRec.QueryString2, sizeof(DataBaseRec.QueryString2));
    DataBaseRec.QueryValue2 = ScfTools::ReadLongDouble(rStream);
}

Sc10DataBaseCollection::Sc10DataBaseCollection(SvStream& rStream)
    : ScCollection(4, 4)
    , nError(0)
{
    sal_uInt16 ID;
    rStream.ReadUInt16( ID );
    if (ID == DataBaseID)
    {
        lcl_ReadFixedString( rStream, ActName, sizeof(ActName));
        sal_uInt16 nAnz;
        rStream.ReadUInt16( nAnz );
        for (sal_uInt16 i=0; (i < nAnz) && (nError == 0); i++)
        {
            nError = insert_new<Sc10DataBaseData>( this, rStream);
        }
    }
    else
    {
        OSL_FAIL( "DataBaseID" );
        nError = errUnknownID;
    }
}

bool Sc10LogFont::operator==( const Sc10LogFont& rData ) const
{
    return !strcmp( lfFaceName, rData.lfFaceName )
        && lfHeight == rData.lfHeight
        && lfWidth == rData.lfWidth
        && lfEscapement == rData.lfEscapement
        && lfOrientation == rData.lfOrientation
        && lfWeight == rData.lfWeight
        && lfItalic == rData.lfItalic
        && lfUnderline == rData.lfUnderline
        && lfStrikeOut == rData.lfStrikeOut
        && lfCharSet == rData.lfCharSet
        && lfOutPrecision == rData.lfOutPrecision
        && lfClipPrecision == rData.lfClipPrecision
        && lfQuality == rData.lfQuality
        && lfPitchAndFamily == rData.lfPitchAndFamily;
}

bool Sc10Color::operator==( const Sc10Color& rColor ) const
{
    return ((Red == rColor.Red) && (Green == rColor.Green) && (Blue == rColor.Blue));
}

bool Sc10HeadFootLine::operator==( const Sc10HeadFootLine& rData ) const
{
    return !strcmp(Title, rData.Title)
        && LogFont == rData.LogFont
        && HorJustify == rData.HorJustify
        && VerJustify == rData.VerJustify
        && Raster == rData.Raster
        && Frame == rData.Frame
        && TextColor == rData.TextColor
        && BackColor == rData.BackColor
        && RasterColor == rData.RasterColor
        && FrameColor == rData.FrameColor
        && Reserved == rData.Reserved;
}

bool Sc10PageFormat::operator==( const Sc10PageFormat& rData ) const
{
    return !strcmp(PrintAreaName, rData.PrintAreaName)
        && HeadLine == rData.HeadLine
        && FootLine == rData.FootLine
        && Orientation == rData.Orientation
        && Width == rData.Width
        && Height == rData.Height
        && NonPrintableX == rData.NonPrintableX
        && NonPrintableY == rData.NonPrintableY
        && Left == rData.Left
        && Top == rData.Top
        && Right == rData.Right
        && Bottom == rData.Bottom
        && Head == rData.Head
        && Foot == rData.Foot
        && HorCenter == rData.HorCenter
        && VerCenter == rData.VerCenter
        && PrintGrid == rData.PrintGrid
        && PrintColRow == rData.PrintColRow
        && PrintNote == rData.PrintNote
        && TopBottomDir == rData.TopBottomDir
        && FirstPageNo == rData.FirstPageNo
        && RowRepeatStart == rData.RowRepeatStart
        && RowRepeatEnd == rData.RowRepeatEnd
        && ColRepeatStart == rData.ColRepeatStart
        && ColRepeatEnd == rData.ColRepeatEnd
        && !memcmp( PrnZoom, rData.PrnZoom, sizeof(PrnZoom) )
        && !memcmp( &PrintArea, &rData.PrintArea, sizeof(PrintArea) );
}

sal_uInt16 Sc10PageCollection::InsertFormat( const Sc10PageFormat& rData )
{
    for (sal_uInt16 i=0; i<nCount; i++)
        if (At(i)->aPageFormat == rData)
            return i;

    Insert( new Sc10PageData(rData) );

    return nCount-1;
}

static inline sal_uInt8 GetMixedCol( const sal_uInt8 nB, const sal_uInt8 nF, const sal_uInt16 nFak )
{
    sal_Int32 nT = nB - nF;
    nT *= ( sal_Int32 ) nFak;
    nT /= 0xFFFF;
    nT += nF;
    return ( sal_uInt8 ) nT;
}
static inline Color GetMixedColor( const Color& rFore, const Color& rBack, sal_uInt16 nFact )
{
    return Color(   GetMixedCol( rBack.GetRed(), rFore.GetRed(), nFact ),
                    GetMixedCol( rBack.GetGreen(), rFore.GetGreen(), nFact ),
                    GetMixedCol( rBack.GetBlue(), rFore.GetBlue(), nFact ) );
}

void Sc10PageCollection::PutToDoc( ScDocument* pDoc )
{
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    EditEngine aEditEngine( pDoc->GetEnginePool() );
    EditTextObject* pEmptyObject = aEditEngine.CreateTextObject();

    for (sal_uInt16 i=0; i<nCount; i++)
    {
        Sc10PageFormat* pPage = &At(i)->aPageFormat;

        pPage->Width = (short) ( pPage->Width + 0.5 );
        pPage->Height = (short) ( pPage->Height + 0.5 );
        pPage->Top = (short) ( pPage->Top + 0.5 );
        pPage->Bottom = (short) ( pPage->Bottom + 0.5 );
        pPage->Left = (short) ( pPage->Left + 0.5 );
        pPage->Right = (short) ( pPage->Right + 0.5 );
        pPage->Head = (short) ( pPage->Head + 0.5 );
        pPage->Foot = (short) ( pPage->Foot + 0.5 );

        OUString aName = lcl_MakeOldPageStyleFormatName( i );

        ScStyleSheet* pSheet = static_cast<ScStyleSheet*>( &pStylePool->Make( aName,
                                    SFX_STYLE_FAMILY_PAGE,
                                    SFXSTYLEBIT_USERDEF | SCSTYLEBIT_STANDARD ) );
        // #i68483# set page style name at sheet...
        pDoc->SetPageStyle( static_cast< SCTAB >( i ), aName );

        SfxItemSet* pSet = &pSheet->GetItemSet();

        for (sal_uInt16 nHeadFoot=0; nHeadFoot<2; nHeadFoot++)
        {
            Sc10HeadFootLine* pHeadFootLine = nHeadFoot ? &pPage->FootLine : &pPage->HeadLine;

            SfxItemSet aEditAttribs(aEditEngine.GetEmptyItemSet());
            FontFamily eFam = FAMILY_DONTKNOW;
            switch (pPage->HeadLine.LogFont.lfPitchAndFamily & 0xF0)
            {
                case ffDontCare:    eFam = FAMILY_DONTKNOW;     break;
                case ffRoman:       eFam = FAMILY_ROMAN;        break;
                case ffSwiss:       eFam = FAMILY_SWISS;        break;
                case ffModern:      eFam = FAMILY_MODERN;       break;
                case ffScript:      eFam = FAMILY_SCRIPT;       break;
                case ffDecorative:  eFam = FAMILY_DECORATIVE;   break;
                default:    eFam = FAMILY_DONTKNOW;     break;
            }
            aEditAttribs.Put(   SvxFontItem(
                                    eFam,
                                    SC10TOSTRING( pHeadFootLine->LogFont.lfFaceName ), EMPTY_OUSTRING,
                                    PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO ),
                                EE_CHAR_FONTINFO );
            aEditAttribs.Put(   SvxFontHeightItem( std::abs( pHeadFootLine->LogFont.lfHeight ), 100, EE_CHAR_FONTHEIGHT ),
                                EE_CHAR_FONTHEIGHT);

            Sc10Color nColor = pHeadFootLine->TextColor;
            Color TextColor( nColor.Red, nColor.Green, nColor.Blue );
            aEditAttribs.Put(SvxColorItem(TextColor, EE_CHAR_COLOR), EE_CHAR_COLOR);
            // FontAttr
            if (pHeadFootLine->LogFont.lfWeight != fwNormal)
                aEditAttribs.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT), EE_CHAR_WEIGHT);
            if (pHeadFootLine->LogFont.lfItalic != 0)
                aEditAttribs.Put(SvxPostureItem(ITALIC_NORMAL, EE_CHAR_ITALIC), EE_CHAR_ITALIC);
            if (pHeadFootLine->LogFont.lfUnderline != 0)
                aEditAttribs.Put(SvxUnderlineItem(UNDERLINE_SINGLE, EE_CHAR_UNDERLINE), EE_CHAR_UNDERLINE);
            if (pHeadFootLine->LogFont.lfStrikeOut != 0)
                aEditAttribs.Put(SvxCrossedOutItem(STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT), EE_CHAR_STRIKEOUT);
            OUString aText( pHeadFootLine->Title, strlen(pHeadFootLine->Title), DEFCHARSET );
            aEditEngine.SetText( aText );
            aEditEngine.QuickSetAttribs( aEditAttribs, ESelection( 0, 0, 0, aText.getLength() ) );

            EditTextObject* pObject = aEditEngine.CreateTextObject();
            ScPageHFItem aHeaderItem(nHeadFoot ? ATTR_PAGE_FOOTERRIGHT : ATTR_PAGE_HEADERRIGHT);
            switch (pHeadFootLine->HorJustify)
            {
                case hjCenter:
                    aHeaderItem.SetLeftArea(*pEmptyObject);
                    aHeaderItem.SetCenterArea(*pObject);
                    aHeaderItem.SetRightArea(*pEmptyObject);
                    break;
                case hjRight:
                    aHeaderItem.SetLeftArea(*pEmptyObject);
                    aHeaderItem.SetCenterArea(*pEmptyObject);
                    aHeaderItem.SetRightArea(*pObject);
                    break;
                default:
                    aHeaderItem.SetLeftArea(*pObject);
                    aHeaderItem.SetCenterArea(*pEmptyObject);
                    aHeaderItem.SetRightArea(*pEmptyObject);
                    break;
            }
            delete pObject;
            pSet->Put( aHeaderItem );

            SfxItemSet aSetItemItemSet( *pDoc->GetPool(),
                                  ATTR_BACKGROUND, ATTR_BACKGROUND,
                                  ATTR_BORDER, ATTR_SHADOW,
                                  ATTR_PAGE_SIZE, ATTR_PAGE_SIZE,
                                  ATTR_LRSPACE, ATTR_ULSPACE,
                                  ATTR_PAGE_ON, ATTR_PAGE_SHARED,
                                  0 );
            nColor = pHeadFootLine->BackColor;
            Color aBColor( nColor.Red, nColor.Green, nColor.Blue );
            nColor = pHeadFootLine->RasterColor;
            Color aRColor( nColor.Red, nColor.Green, nColor.Blue );

            sal_uInt16 nFact;
            bool        bSwapCol = false;
            switch (pHeadFootLine->Raster)
            {
                case raNone:     nFact = 0xffff; bSwapCol = true; break;
                case raGray12:   nFact = (0xffff / 100) * 12;    break;
                case raGray25:   nFact = (0xffff / 100) * 25;    break;
                case raGray50:   nFact = (0xffff / 100) * 50;    break;
                case raGray75:   nFact = (0xffff / 100) * 75;    break;
                default: nFact = 0xffff;
            }
            if( bSwapCol )
                aSetItemItemSet.Put( SvxBrushItem( GetMixedColor( aBColor, aRColor, nFact ), ATTR_BACKGROUND ) );
            else
                aSetItemItemSet.Put( SvxBrushItem( GetMixedColor( aRColor, aBColor, nFact ), ATTR_BACKGROUND ) );

            if (pHeadFootLine->Frame != 0)
            {
                sal_uInt16 nLeft = 0;
                sal_uInt16 nTop = 0;
                sal_uInt16 nRight = 0;
                sal_uInt16 nBottom = 0;
                sal_uInt16 fLeft   = (pHeadFootLine->Frame & 0x000F);
                sal_uInt16 fTop    = (pHeadFootLine->Frame & 0x00F0) / 0x0010;
                sal_uInt16 fRight  = (pHeadFootLine->Frame & 0x0F00) / 0x0100;
                sal_uInt16 fBottom = (pHeadFootLine->Frame & 0xF000) / 0x1000;
                if (fLeft > 1)
                    nLeft = 50;
                else if (fLeft > 0)
                    nLeft = 20;
                if (fTop > 1)
                    nTop = 50;
                else if (fTop > 0)
                    nTop = 20;
                if (fRight > 1)
                    nRight = 50;
                else if (fRight > 0)
                    nRight = 20;
                if (fBottom > 1)
                    nBottom = 50;
                else if (fBottom > 0)
                    nBottom = 20;
                Color  ColorLeft(COL_BLACK);
                Color  ColorTop(COL_BLACK);
                Color  ColorRight(COL_BLACK);
                Color  ColorBottom(COL_BLACK);
                sal_uInt16 cLeft   = (pHeadFootLine->FrameColor & 0x000F);
                sal_uInt16 cTop    = (pHeadFootLine->FrameColor & 0x00F0) >> 4;
                sal_uInt16 cRight  = (pHeadFootLine->FrameColor & 0x0F00) >> 8;
                sal_uInt16 cBottom = (pHeadFootLine->FrameColor & 0xF000) >> 12;
                lcl_ChangeColor(cLeft, ColorLeft);
                lcl_ChangeColor(cTop, ColorTop);
                lcl_ChangeColor(cRight, ColorRight);
                lcl_ChangeColor(cBottom, ColorBottom);
                ::editeng::SvxBorderLine aLine;
                SvxBoxItem aBox( ATTR_BORDER );
                aLine.SetWidth(nLeft);
                aLine.SetColor(ColorLeft);
                aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);
                aLine.SetWidth(nTop);
                aLine.SetColor(ColorTop);
                aBox.SetLine(&aLine, SvxBoxItemLine::TOP);
                aLine.SetWidth(nRight);
                aLine.SetColor(ColorRight);
                aBox.SetLine(&aLine, SvxBoxItemLine::RIGHT);
                aLine.SetWidth(nBottom);
                aLine.SetColor(ColorBottom);
                aBox.SetLine(&aLine, SvxBoxItemLine::BOTTOM);

                aSetItemItemSet.Put(aBox);
            }

            pSet->Put( SvxULSpaceItem( 0, 0, ATTR_ULSPACE ) );

            if (nHeadFoot==0)
                aSetItemItemSet.Put( SvxSizeItem( ATTR_PAGE_SIZE, Size( 0, pPage->Top - pPage->Head ) ) );
            else
                aSetItemItemSet.Put( SvxSizeItem( ATTR_PAGE_SIZE, Size( 0, pPage->Bottom - pPage->Foot ) ) );

            aSetItemItemSet.Put(SfxBoolItem( ATTR_PAGE_ON, true ));
            aSetItemItemSet.Put(SfxBoolItem( ATTR_PAGE_DYNAMIC, false ));
            aSetItemItemSet.Put(SfxBoolItem( ATTR_PAGE_SHARED, true ));

            pSet->Put( SvxSetItem( nHeadFoot ? ATTR_PAGE_FOOTERSET : ATTR_PAGE_HEADERSET,
                                    aSetItemItemSet ) );
        }

        SvxPageItem aPageItem(ATTR_PAGE);
        aPageItem.SetPageUsage( SVX_PAGE_ALL );
        aPageItem.SetLandscape( pPage->Orientation != 1 );
        aPageItem.SetNumType( SVX_ARABIC );
        pSet->Put(aPageItem);

        pSet->Put(SvxLRSpaceItem( pPage->Left, pPage->Right, 0,0, ATTR_LRSPACE ));
        pSet->Put(SvxULSpaceItem( pPage->Top, pPage->Bottom, ATTR_ULSPACE ));

        pSet->Put(SfxBoolItem( ATTR_PAGE_HORCENTER, pPage->HorCenter ));
        pSet->Put(SfxBoolItem( ATTR_PAGE_VERCENTER, pPage->VerCenter ));

        // Area-Parameter:

        {
            ScRange* pRepeatRow = nullptr;
            ScRange* pRepeatCol = nullptr;

            if ( pPage->ColRepeatStart >= 0 )
                pRepeatCol = new ScRange( static_cast<SCCOL> (pPage->ColRepeatStart), 0, 0 );
            if ( pPage->RowRepeatStart >= 0 )
                pRepeatRow = new ScRange( 0, static_cast<SCROW> (pPage->RowRepeatStart), 0 );

            if ( pRepeatRow || pRepeatCol )
            {

                // set for all tables

                for ( SCTAB nTab = 0, nTabCount = pDoc->GetTableCount(); nTab < nTabCount; ++nTab )
                {
                    pDoc->SetRepeatColRange( nTab, pRepeatCol );
                    pDoc->SetRepeatRowRange( nTab, pRepeatRow );
                }
            }

            delete pRepeatRow;
            delete pRepeatCol;
        }

        // Table-Parameter:

        pSet->Put( SfxBoolItem( ATTR_PAGE_NOTES,   pPage->PrintNote ) );
        pSet->Put( SfxBoolItem( ATTR_PAGE_GRID,    pPage->PrintGrid ) );
        pSet->Put( SfxBoolItem( ATTR_PAGE_HEADERS, pPage->PrintColRow ) );
        pSet->Put( SfxBoolItem( ATTR_PAGE_TOPDOWN, pPage->TopBottomDir ) );
        pSet->Put( ScViewObjectModeItem( ATTR_PAGE_CHARTS,   VOBJ_MODE_SHOW ) );
        pSet->Put( ScViewObjectModeItem( ATTR_PAGE_OBJECTS,  VOBJ_MODE_SHOW ) );
        pSet->Put( ScViewObjectModeItem( ATTR_PAGE_DRAWINGS, VOBJ_MODE_SHOW ) );
        pSet->Put( SfxUInt16Item( ATTR_PAGE_SCALE,
                                  (sal_uInt16)( lcl_PascalToDouble( pPage->PrnZoom ) * 100 ) ) );
        pSet->Put( SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, 1 ) );

        pSet->Put( SvxSizeItem( ATTR_PAGE_SIZE, Size( pPage->Width, pPage->Height ) ) );
    }

    delete pEmptyObject;
}

ScDataObject* Sc10PageData::Clone() const
{
    return new Sc10PageData(aPageFormat);
}

// Import

Sc10Import::Sc10Import(SvStream& rStr, ScDocument* pDocument ) :
    rStream             (rStr),
    pDoc                (pDocument),
    pFontCollection     (nullptr),
    pNameCollection     (nullptr),
    pPatternCollection  (nullptr),
    pDataBaseCollection (nullptr),
    nError              (0),
    nShowTab            (0)
{
    pPrgrsBar = nullptr;
}

Sc10Import::~Sc10Import()
{
    pDoc->CalcAfterLoad();
    pDoc->UpdateAllCharts();

    delete pFontCollection;
    delete pNameCollection;
    delete pPatternCollection;
    delete pDataBaseCollection;

    OSL_ENSURE( pPrgrsBar == nullptr,
        "*Sc10Import::Sc10Import(): Progressbar lebt noch!?" );
}

sal_uLong Sc10Import::Import()
{
    pPrgrsBar = new ScfStreamProgressBar( rStream, pDoc->GetDocumentShell() );

    ScDocOptions aOpt = pDoc->GetDocOptions();
    aOpt.SetDate( 1, 1, 1900 );
    aOpt.SetYear2000( 18 + 1901 );      // 4-digit since SO51 src513e
    pDoc->SetDocOptions( aOpt );
    pDoc->GetFormatTable()->ChangeNullDate( 1, 1, 1900 );

    LoadFileHeader();                           pPrgrsBar->Progress();
    if (!nError) { LoadFileInfo();              pPrgrsBar->Progress(); }
    if (!nError) { LoadEditStateInfo();         pPrgrsBar->Progress(); }
    if (!nError) { LoadProtect();               pPrgrsBar->Progress(); }
    if (!nError) { LoadViewColRowBar();         pPrgrsBar->Progress(); }
    if (!nError) { LoadScrZoom();               pPrgrsBar->Progress(); }
    if (!nError) { LoadPalette();               pPrgrsBar->Progress(); }
    if (!nError) { LoadFontCollection();        pPrgrsBar->Progress(); }
    if (!nError) { LoadNameCollection();        pPrgrsBar->Progress(); }
    if (!nError) { LoadPatternCollection();     pPrgrsBar->Progress(); }
    if (!nError) { LoadDataBaseCollection();    pPrgrsBar->Progress(); }
    if (!nError) { LoadTables();                pPrgrsBar->Progress(); }
    if (!nError) { LoadObjects();               pPrgrsBar->Progress(); }
    if (!nError) { ImportNameCollection();      pPrgrsBar->Progress(); }
    pDoc->SetViewOptions( aSc30ViewOpt );

#if OSL_DEBUG_LEVEL > 0
    if (nError)
    {
        OSL_FAIL( OString::number(nError).getStr());
    }
#endif

    delete pPrgrsBar;
#if OSL_DEBUG_LEVEL > 0
    pPrgrsBar = nullptr;
#endif

    return nError;
}

void Sc10Import::LoadFileHeader()
{
    Sc10FileHeader FileHeader;
    lcl_ReadFileHeader(rStream, FileHeader);

    nError = rStream.GetError();
    if ( nError == 0 )
    {
        sal_Char Sc10CopyRight[32];
        strcpy(Sc10CopyRight, "Blaise-Tabelle");
        Sc10CopyRight[14] = 10;
        Sc10CopyRight[15] = 13;
        Sc10CopyRight[16] = 0;
        if ((strcmp(FileHeader.CopyRight, Sc10CopyRight) != 0)
            || (FileHeader.Version < 101)
            || (FileHeader.Version > 102))
            nError = errUnknownFormat;
    }
}

void Sc10Import::LoadFileInfo()
{
    Sc10FileInfo FileInfo;
    rStream.Read(&FileInfo, sizeof(FileInfo));

    nError = rStream.GetError();
    // TODO: ? copy info
}

void Sc10Import::LoadEditStateInfo()
{
    Sc10EditStateInfo EditStateInfo;
    rStream.Read(&EditStateInfo, sizeof(EditStateInfo));

    nError = rStream.GetError();
    nShowTab = static_cast<SCTAB>(EditStateInfo.DeltaZ);
    // TODO: ? copy cursor position and offset of the table (shall we do that??)

}

void Sc10Import::LoadProtect()
{
    lcl_ReadSheetProtect(rStream, SheetProtect);
    nError = rStream.GetError();

    ScDocProtection aProtection;
    aProtection.setProtected(static_cast<bool>(SheetProtect.Protect));
    aProtection.setPassword(SC10TOSTRING(SheetProtect.PassWord));
    pDoc->SetDocProtection(&aProtection);
}

void Sc10Import::LoadViewColRowBar()
{
    bool bViewColRowBar;
    rStream.ReadCharAsBool( bViewColRowBar );
    nError = rStream.GetError();
    aSc30ViewOpt.SetOption( VOPT_HEADER, bViewColRowBar );
}

void Sc10Import::LoadScrZoom()
{
    // TODO: unfortunately Zoom is a 6-byte TP real number (don't know how to translate that)
    sal_Char cZoom[6];
    rStream.Read(cZoom, sizeof(cZoom));
    nError = rStream.GetError();
}

void Sc10Import::LoadPalette()
{
    lcl_ReadPalette(rStream, TextPalette);
    lcl_ReadPalette(rStream, BackPalette);
    lcl_ReadPalette(rStream, RasterPalette);
    lcl_ReadPalette(rStream, FramePalette);

    nError = rStream.GetError();
}

void Sc10Import::LoadFontCollection()
{
    pFontCollection = new Sc10FontCollection(rStream);
    if (!nError)
        nError = pFontCollection->GetError();
}

void Sc10Import::LoadNameCollection()
{
    pNameCollection = new Sc10NameCollection(rStream);
    if (!nError)
        nError = pNameCollection->GetError();
}

void Sc10Import::ImportNameCollection()
{
    ScRangeName*        pRN = pDoc->GetRangeName();

    for (sal_uInt16 i = 0; i < pNameCollection->GetCount(); i++)
    {
        Sc10NameData*   pName = pNameCollection->At( i );
        pRN->insert(
            new ScRangeData(
                pDoc, SC10TOSTRING(pName->Name), SC10TOSTRING(pName->Reference)));
    }
}

void Sc10Import::LoadPatternCollection()
{
    pPatternCollection = new Sc10PatternCollection( rStream );
    if (!nError)
        nError = pPatternCollection->GetError();
    if (nError == errOutOfMemory)
        return;     // hopeless
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    for( sal_uInt16 i = 0 ; i < pPatternCollection->GetCount() ; i++ )
    {
        Sc10PatternData* pPattern = pPatternCollection->At( i );
        OUString aName( pPattern->Name, strlen(pPattern->Name), DEFCHARSET );
        SfxStyleSheetBase* pStyle = pStylePool->Find( aName, SFX_STYLE_FAMILY_PARA );
        if( pStyle == nullptr )
            pStylePool->Make( aName, SFX_STYLE_FAMILY_PARA );
        else
        {
            pPattern->Name[ 27 ] = 0;
            strcat( pPattern->Name, "_Old" );
            aName = SC10TOSTRING( pPattern->Name );
            pStylePool->Make( aName, SFX_STYLE_FAMILY_PARA );
        }
        pStyle = pStylePool->Find( aName, SFX_STYLE_FAMILY_PARA );
        if( pStyle != nullptr )
        {
            SfxItemSet &rItemSet = pStyle->GetItemSet();
            // Font
            if( ( pPattern->FormatFlags & pfFont ) == pfFont )
            {
                FontFamily eFam = FAMILY_DONTKNOW;
                switch( pPattern->LogFont.lfPitchAndFamily & 0xF0 )
                {
                    case ffDontCare   : eFam = FAMILY_DONTKNOW;     break;
                    case ffRoman      : eFam = FAMILY_ROMAN;        break;
                    case ffSwiss      : eFam = FAMILY_SWISS;        break;
                    case ffModern     : eFam = FAMILY_MODERN;       break;
                    case ffScript     : eFam = FAMILY_SCRIPT;       break;
                    case ffDecorative : eFam = FAMILY_DECORATIVE;   break;
                    default: eFam = FAMILY_DONTKNOW;        break;
                }
                rItemSet.Put( SvxFontItem( eFam, SC10TOSTRING( pPattern->LogFont.lfFaceName ), EMPTY_OUSTRING,
                        PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ) );
                rItemSet.Put( SvxFontHeightItem( std::abs( pPattern->LogFont.lfHeight ), 100, ATTR_FONT_HEIGHT ) );
                Color TextColor( COL_BLACK );
                lcl_ChangeColor( ( pPattern->nColor & 0x000F ), TextColor );
                rItemSet.Put( SvxColorItem( TextColor, ATTR_FONT_COLOR ) );
                // FontAttr
                if( pPattern->LogFont.lfWeight != fwNormal )
                    rItemSet.Put( SvxWeightItem( WEIGHT_BOLD, ATTR_FONT_WEIGHT ) );
                if( pPattern->LogFont.lfItalic != 0 )
                    rItemSet.Put( SvxPostureItem( ITALIC_NORMAL, ATTR_FONT_POSTURE ) );
                if( pPattern->LogFont.lfUnderline != 0 )
                    rItemSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE, ATTR_FONT_UNDERLINE ) );
                if( pPattern->LogFont.lfStrikeOut != 0 )
                    rItemSet.Put( SvxCrossedOutItem( STRIKEOUT_SINGLE, ATTR_FONT_CROSSEDOUT ) );
            }
            // alignment
            if( ( pPattern->FormatFlags & pfJustify ) == pfJustify )
            {
                sal_uInt16 HorJustify = ( pPattern->Justify & 0x000F );
                sal_uInt16 VerJustify = ( pPattern->Justify & 0x00F0 ) >> 4;
                sal_uInt16 OJustify   = ( pPattern->Justify & 0x0F00 ) >> 8;
                sal_uInt16 EJustify   = ( pPattern->Justify & 0xF000 ) >> 12;
                if( HorJustify != 0 )
                    switch( HorJustify )
                    {
                        case hjLeft:
                            rItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT, ATTR_HOR_JUSTIFY ) );
                            break;
                        case hjCenter:
                            rItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY ) );
                            break;
                        case hjRight:
                            rItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_RIGHT, ATTR_HOR_JUSTIFY ) );
                            break;
                    }
                if( VerJustify != 0 )
                    switch( VerJustify )
                    {
                        case vjTop:
                            rItemSet.Put( SvxVerJustifyItem( SVX_VER_JUSTIFY_TOP, ATTR_VER_JUSTIFY ) );
                            break;
                        case vjCenter:
                            rItemSet.Put( SvxVerJustifyItem( SVX_VER_JUSTIFY_CENTER, ATTR_VER_JUSTIFY ) );
                            break;
                        case vjBottom:
                            rItemSet.Put( SvxVerJustifyItem( SVX_VER_JUSTIFY_BOTTOM, ATTR_VER_JUSTIFY ) );
                            break;
                    }

                if( ( OJustify & ojWordBreak ) == ojWordBreak )
                    rItemSet.Put( SfxBoolItem( ATTR_LINEBREAK, true ) );
                if( ( OJustify & ojBottomTop ) == ojBottomTop )
                    rItemSet.Put( SfxInt32Item( ATTR_ROTATE_VALUE, 9000 ) );
                else if( ( OJustify & ojTopBottom ) == ojTopBottom )
                    rItemSet.Put( SfxInt32Item( ATTR_ROTATE_VALUE, 27000 ) );

                sal_Int16 Margin = std::max( ( sal_uInt16 ) 20, ( sal_uInt16 ) ( EJustify * 20 ) );
                if( ( ( OJustify & ojBottomTop ) == ojBottomTop ) )
                    rItemSet.Put( SvxMarginItem( 20, Margin, 20, Margin, ATTR_MARGIN ) );
                else
                    rItemSet.Put( SvxMarginItem( Margin, 20, Margin, 20, ATTR_MARGIN ) );
            }

            // Frame
            if( ( pPattern->FormatFlags & pfFrame ) == pfFrame )
            {
                if( pPattern->Frame != 0 )
                {
                    sal_uInt16  nLeft   = 0;
                    sal_uInt16  nTop    = 0;
                    sal_uInt16  nRight  = 0;
                    sal_uInt16  nBottom = 0;
                    sal_uInt16  fLeft   = ( pPattern->Frame & 0x000F );
                    sal_uInt16  fTop    = ( pPattern->Frame & 0x00F0 ) / 0x0010;
                    sal_uInt16  fRight  = ( pPattern->Frame & 0x0F00 ) / 0x0100;
                    sal_uInt16  fBottom = ( pPattern->Frame & 0xF000 ) / 0x1000;

                    if( fLeft > 1 )
                        nLeft = 50;
                    else if( fLeft > 0 )
                        nLeft = 20;

                    if( fTop > 1 )
                        nTop = 50;
                    else if( fTop > 0 )
                        nTop = 20;

                    if( fRight > 1 )
                        nRight = 50;
                    else if( fRight > 0 )
                        nRight = 20;

                    if( fBottom > 1 )
                        nBottom = 50;
                    else if( fBottom > 0 )
                        nBottom = 20;

                    Color   ColorLeft( COL_BLACK );
                    Color   ColorTop( COL_BLACK );
                    Color   ColorRight( COL_BLACK );
                    Color   ColorBottom( COL_BLACK );

                    sal_uInt16  cLeft   = ( pPattern->FrameColor & 0x000F );
                    sal_uInt16  cTop    = ( pPattern->FrameColor & 0x00F0 ) >> 4;
                    sal_uInt16  cRight  = ( pPattern->FrameColor & 0x0F00 ) >> 8;
                    sal_uInt16  cBottom = ( pPattern->FrameColor & 0xF000 ) >> 12;

                    lcl_ChangeColor( cLeft, ColorLeft );
                    lcl_ChangeColor( cTop, ColorTop );
                    lcl_ChangeColor( cRight, ColorRight );
                    lcl_ChangeColor( cBottom, ColorBottom );

                    ::editeng::SvxBorderLine    aLine;
                    SvxBoxItem      aBox( ATTR_BORDER );

                    aLine.SetWidth( nLeft );
                    aLine.SetColor( ColorLeft );
                    aBox.SetLine( &aLine, SvxBoxItemLine::LEFT );
                    aLine.SetWidth( nTop );
                    aLine.SetColor( ColorTop );
                    aBox.SetLine( &aLine, SvxBoxItemLine::TOP );
                    aLine.SetWidth( nRight );
                    aLine.SetColor( ColorRight );
                    aBox.SetLine( &aLine, SvxBoxItemLine::RIGHT );
                    aLine.SetWidth( nBottom );
                    aLine.SetColor( ColorBottom );
                    aBox.SetLine( &aLine, SvxBoxItemLine::BOTTOM );
                    rItemSet.Put( aBox );
                }
            }
            // grid
            if( ( pPattern->FormatFlags & pfRaster ) == pfRaster )
            {
                if( pPattern->Raster != 0 )
                {
                    sal_uInt16 nBColor = ( pPattern->nColor & 0x00F0 ) >> 4;
                    sal_uInt16 nRColor = ( pPattern->nColor & 0x0F00 ) >> 8;
                    Color aBColor( COL_BLACK );

                    lcl_ChangeColor( nBColor, aBColor );

                    if( nBColor == 0 )
                        aBColor.SetColor( COL_WHITE );
                    else if( nBColor == 15 )
                        aBColor.SetColor( COL_BLACK );

                    Color aRColor( COL_BLACK );
                    lcl_ChangeColor( nRColor, aRColor );
                    sal_uInt16 nFact;
                    bool        bSwapCol = false;
                    bool        bSetItem = true;
                    switch (pPattern->Raster)
                    {
                       case raNone:     nFact = 0xffff; bSwapCol = true; bSetItem = (nBColor > 0); break;
                       case raGray12:   nFact = (0xffff / 100) * 12;    break;
                       case raGray25:   nFact = (0xffff / 100) * 25;    break;
                       case raGray50:   nFact = (0xffff / 100) * 50;    break;
                       case raGray75:   nFact = (0xffff / 100) * 75;    break;
                       default: nFact = 0xffff; bSetItem = (nRColor < 15);
                    }
                    if ( bSetItem )
                    {
                        if( bSwapCol )
                            rItemSet.Put( SvxBrushItem( GetMixedColor( aBColor, aRColor, nFact ), ATTR_BACKGROUND ) );
                        else
                            rItemSet.Put( SvxBrushItem( GetMixedColor( aRColor, aBColor, nFact ), ATTR_BACKGROUND ) );
                    }
                }
            }
            // number formats
            if( ( pPattern->ValueFormat.Format != 0 ) &&
                ( ( pPattern->FormatFlags & pfValue ) == pfValue ) )
            {
                sal_uLong nKey = 0;
                ChangeFormat( pPattern->ValueFormat.Format, pPattern->ValueFormat.Info, nKey );
                rItemSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, ( sal_uInt32 ) nKey ) );
            }

            // cell attributes (protected, hidden...)
            if( ( pPattern->Flags != 0 ) &&
                ( ( pPattern->FormatFlags & pfProtection ) == pfProtection ) )
            {
                bool bProtect  = ( ( pPattern->Flags & paProtect ) == paProtect );
                bool bHFormula = ( ( pPattern->Flags & paHideFormula ) == paHideFormula );
                bool bHCell    = ( ( pPattern->Flags & paHideAll ) == paHideAll );
                bool bHPrint   = ( ( pPattern->Flags & paHidePrint ) == paHidePrint );
                rItemSet.Put( ScProtectionAttr( bProtect, bHFormula, bHCell, bHPrint ) );
            }
        } // if Style != 0
    } // for (i = 0; i < GetCount()
}

void Sc10Import::LoadDataBaseCollection()
{
    pDataBaseCollection = new Sc10DataBaseCollection(rStream);
    if (!nError)
        nError = pDataBaseCollection->GetError();
    if (nError == errOutOfMemory)
        return;     // hopeless
    for( sal_uInt16 i = 0 ; i < pDataBaseCollection->GetCount() ; i++ )
    {
        Sc10DataBaseData* pOldData = pDataBaseCollection->At(i);
        ScDBData* pNewData = new ScDBData( SC10TOSTRING( pOldData->DataBaseRec.Name ),
                                    ( SCTAB ) pOldData->DataBaseRec.Tab,
                                    ( SCCOL ) pOldData->DataBaseRec.Block.x1,
                                    ( SCROW ) pOldData->DataBaseRec.Block.y1,
                                    ( SCCOL ) pOldData->DataBaseRec.Block.x2,
                                    ( SCROW ) pOldData->DataBaseRec.Block.y2,
                                    true,
                                    (bool) pOldData->DataBaseRec.RowHeader );
        bool ins = pDoc->GetDBCollection()->getNamedDBs().insert(pNewData);
        assert(ins); (void)ins;
            //TODO: or can this fail (and need delete pNewData)?
    }
}

namespace
{
    sal_uInt16 ReadAndSanitizeDataCount(SvStream &rStream)
    {
        sal_uInt16 nDataCount(0);
        rStream.ReadUInt16(nDataCount);
        const size_t nMinRecordSize = sizeof(sal_uInt16)*2;
        const size_t nMaxRecords = rStream.remainingSize() / nMinRecordSize;
        if (nDataCount > nMaxRecords)
        {
            SAL_WARN("sc", "Parsing error: " << nMaxRecords <<
                     " max possible entries, but " << nDataCount << " claimed, truncating");
            nDataCount = nMaxRecords;
        }
        return nDataCount;
    }
}

void Sc10Import::LoadTables()
{
    Sc10PageCollection aPageCollection;

    sal_Int16 nTabCount;
    rStream.ReadInt16( nTabCount );
    for (sal_Int16 Tab = 0; (Tab < nTabCount) && (nError == 0); Tab++)
    {
        Sc10PageFormat   PageFormat;
        sal_Int16            DataBaseIndex;
        Sc10TableProtect TabProtect;
        sal_Int16            TabNo;
        sal_Char             TabName[128];
        sal_uInt16           Display;
        sal_uInt8             Visible;
        sal_uInt16           ID;
        sal_uInt16           DataCount;
        sal_uInt16           DataStart;
        sal_uInt16           DataEnd;
        sal_uInt16           DataValue;
        sal_uInt16           Count;
        sal_uInt16           i;

        lcl_ReadPageFormat(rStream, PageFormat);

        sal_uInt16 nAt = aPageCollection.InsertFormat(PageFormat);
        OUString aPageName = lcl_MakeOldPageStyleFormatName( nAt );

        pPrgrsBar->Progress();

        rStream.ReadInt16( DataBaseIndex );

        lcl_ReadTabProtect(rStream, TabProtect);

        ScTableProtection aProtection;
        aProtection.setProtected(static_cast<bool>(TabProtect.Protect));
        aProtection.setPassword(SC10TOSTRING(TabProtect.PassWord));
        pDoc->SetTabProtection(static_cast<SCTAB>(Tab), &aProtection);

        rStream.ReadInt16( TabNo );

        sal_uInt8 nLen;
        rStream.ReadUChar( nLen );
        rStream.Read(TabName, sizeof(TabName) - 1);
        if (nLen >= sizeof(TabName))
            nLen = sizeof(TabName) - 1;
        TabName[nLen] = 0;

        rStream.ReadUInt16( Display );

        if ( Tab == (sal_Int16)nShowTab )
        {
            ScVObjMode  eObjMode = VOBJ_MODE_SHOW;

            aSc30ViewOpt.SetOption( VOPT_FORMULAS,    IS_SET(dfFormula,Display) );
            aSc30ViewOpt.SetOption( VOPT_NULLVALS,    IS_SET(dfZerro,Display) );
            aSc30ViewOpt.SetOption( VOPT_SYNTAX,      IS_SET(dfSyntax,Display) );
            aSc30ViewOpt.SetOption( VOPT_NOTES,       IS_SET(dfNoteMark,Display) );
            aSc30ViewOpt.SetOption( VOPT_VSCROLL );
            aSc30ViewOpt.SetOption( VOPT_HSCROLL );
            aSc30ViewOpt.SetOption( VOPT_TABCONTROLS );
            aSc30ViewOpt.SetOption( VOPT_OUTLINER );
            aSc30ViewOpt.SetOption( VOPT_GRID,        IS_SET(dfGrid,Display) );

            // VOPT_HEADER is set in LoadViewColRowBar()

            if ( IS_SET(dfObjectAll,Display) )          // show objects
                eObjMode = VOBJ_MODE_SHOW;
            else if ( IS_SET(dfObjectFrame,Display) )   // object as placeholder
                eObjMode = VOBJ_MODE_SHOW;
            else if ( IS_SET(dfObjectNone,Display) )    // don't show objects
                eObjMode = VOBJ_MODE_HIDE;

            aSc30ViewOpt.SetObjMode( VOBJ_TYPE_OLE,   eObjMode );
            aSc30ViewOpt.SetObjMode( VOBJ_TYPE_CHART, eObjMode );
            aSc30ViewOpt.SetObjMode( VOBJ_TYPE_DRAW,  eObjMode );
        }

        rStream.ReadUChar( Visible );

        nError = rStream.GetError();
        if (nError != 0) return;

        if (TabNo == 0)
            pDoc->RenameTab(static_cast<SCTAB> (TabNo), SC10TOSTRING( TabName ), false);
        else
            pDoc->InsertTab(SC_TAB_APPEND, SC10TOSTRING( TabName ) );

        pDoc->SetPageStyle( static_cast<SCTAB>(Tab), aPageName );

        if (Visible == 0) pDoc->SetVisible(static_cast<SCTAB> (TabNo), false);

        // ColWidth
        rStream.ReadUInt16( ID );
        if (ID != ColWidthID)
        {
            OSL_FAIL( "ColWidthID" );
            nError = errUnknownID;
            return;
        }
        DataCount = ReadAndSanitizeDataCount(rStream);
        DataStart = 0;
        for (i=0; i < DataCount; i++)
        {
            rStream.ReadUInt16( DataEnd );
            DataEnd = SanitizeCol(DataEnd);
            rStream.ReadUInt16( DataValue );
            for (SCCOL j = static_cast<SCCOL>(DataStart); j <= static_cast<SCCOL>(DataEnd); j++)
                pDoc->SetColWidth(j, static_cast<SCTAB> (TabNo), DataValue);
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // ColAttr
        rStream.ReadUInt16( ID );
        if (ID != ColAttrID)
        {
            OSL_FAIL( "ColAttrID" );
            nError = errUnknownID;
            return;
        }

        DataCount = ReadAndSanitizeDataCount(rStream);
        DataStart = 0;
        for (i=0; i < DataCount; i++)
        {
            rStream.ReadUInt16( DataEnd );
            rStream.ReadUInt16( DataValue );
            if (DataValue != 0)
            {
                bool bPageBreak   = ((DataValue & crfSoftBreak) == crfSoftBreak);
                bool bManualBreak = ((DataValue & crfHardBreak) == crfHardBreak);
                bool bHidden = ((DataValue & crfHidden) == crfHidden);
                for (SCCOL k = SanitizeCol(static_cast<SCCOL>(DataStart)); k <= SanitizeCol(static_cast<SCCOL>(DataEnd)); k++)
                {
                    pDoc->SetColHidden(k, k, static_cast<SCTAB>(TabNo), bHidden);
                    pDoc->SetColBreak(k, static_cast<SCTAB> (TabNo), bPageBreak, bManualBreak);
                }
            }
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // RowHeight
        rStream.ReadUInt16( ID );
        if (ID != RowHeightID)
        {
            OSL_FAIL( "RowHeightID" );
            nError = errUnknownID;
            return;
        }

        DataCount = ReadAndSanitizeDataCount(rStream);
        DataStart = 0;
        for (i=0; i < DataCount; i++)
        {
            rStream.ReadUInt16( DataEnd );
            rStream.ReadUInt16( DataValue );
            pDoc->SetRowHeightRange(static_cast<SCROW> (DataStart), static_cast<SCROW> (DataEnd), static_cast<SCTAB> (TabNo), DataValue);
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // RowAttr
        rStream.ReadUInt16( ID );
        if (ID != RowAttrID)
        {
            OSL_FAIL( "RowAttrID" );
            nError = errUnknownID;
            return;
        }

        DataCount = ReadAndSanitizeDataCount(rStream);
        DataStart = 0;
        for (i=0; i < DataCount; i++)
        {
            rStream.ReadUInt16( DataEnd );
            rStream.ReadUInt16( DataValue );
            if (DataValue != 0)
            {
                bool bPageBreak   = ((DataValue & crfSoftBreak) == crfSoftBreak);
                bool bManualBreak = ((DataValue & crfHardBreak) == crfHardBreak);
                bool bHidden      = ((DataValue & crfHidden) == crfHidden);
                for (SCROW l = SanitizeRow(static_cast<SCROW>(DataStart)); l <= SanitizeRow(static_cast<SCROW>(DataEnd)); ++l)
                {
                    pDoc->SetRowHidden(l, l, static_cast<SCTAB> (TabNo), bHidden);
                    pDoc->SetRowBreak(l, static_cast<SCTAB> (TabNo), bPageBreak, bManualBreak);
                }
            }
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // Data table
        rStream.ReadUInt16( ID );
        if (ID != TableID)
        {
            OSL_FAIL( "TableID" );
            nError = errUnknownID;
            return;
        }
        for (SCCOL Col = 0; (Col <= SC10MAXCOL) && (nError == 0); Col++)
        {
            rStream.ReadUInt16( Count );
            nError = rStream.GetError();
            if ((Count != 0) && (nError == 0))
                LoadCol(Col, static_cast<SCTAB> (TabNo));
        }
        OSL_ENSURE( nError == 0, "Stream" );
    }
    pPrgrsBar->Progress();

    aPageCollection.PutToDoc( pDoc );
}

void Sc10Import::LoadCol(SCCOL Col, SCTAB Tab)
{
    LoadColAttr(Col, Tab);

    sal_uInt16 CellCount;
    sal_uInt8   CellType;
    sal_uInt16 Row;
    rStream.ReadUInt16( CellCount );
    SCROW nScCount = static_cast< SCROW >( CellCount );
    if (nScCount > MAXROW) nError = errUnknownFormat;
    for (sal_uInt16 i = 0; (i < CellCount) && (nError == 0); i++)
    {
        rStream.ReadUChar( CellType );
        rStream.ReadUInt16( Row );
        nError = rStream.GetError();
        if (nError == 0)
        {
            switch (CellType)
            {
                case ctValue :
                {
                    const SfxPoolItem* pValueFormat = pDoc->GetAttr(Col, static_cast<SCROW> (Row), Tab, ATTR_VALUE_FORMAT);
                    sal_uLong nFormat = static_cast<const SfxUInt32Item*>(pValueFormat)->GetValue();
                    double Value = ScfTools::ReadLongDouble(rStream);
                    //rStream.Read(&Value, sizeof(Value));

                    // TODO: adjustment is needed if we change the Basis Date
                    // StarCalc 1.0: 01.01.1900
                    // if ((nFormat >= 30) && (nFormat <= 35))
                    // Value += 0;
                    if ((nFormat >= 40) && (nFormat <= 45))
                        Value /= 86400.0;
                    pDoc->SetValue(Col, static_cast<SCROW> (Row), Tab, Value);
                    break;
                }
                case ctString :
                {
                    sal_uInt8 Len;
                    sal_Char s[256];
                    rStream.ReadUChar( Len );
                    rStream.Read(s, Len);
                    s[Len] = 0;

                    pDoc->SetString( Col, static_cast<SCROW> (Row), Tab, SC10TOSTRING( s ) );
                    break;
                }
                case ctFormula :
                {
                    /*double Value =*/ ScfTools::ReadLongDouble(rStream);
                    sal_uInt8 Len;
                    sal_Char s[256+1];
                    rStream.ReadUChar( Len );
                    rStream.Read(&s[1], Len);
                    s[0] = '=';
                    s[Len + 1] = 0;
                    ScFormulaCell* pCell = new ScFormulaCell( pDoc, ScAddress( Col, static_cast<SCROW> (Row), Tab ) );
                    pCell->SetHybridFormula( SC10TOSTRING( s ),formula::FormulaGrammar::GRAM_NATIVE );
                    pDoc->EnsureTable(Tab);
                    pDoc->SetFormulaCell(ScAddress(Col,Row,Tab), pCell);
                    break;
                }
                case ctNote :
                    break;
                default :
                    nError = errUnknownFormat;
                    break;
            }
            sal_uInt16 nNoteLen(0);
            rStream.ReadUInt16(nNoteLen);
            size_t nAvailable = rStream.remainingSize();
            if (nNoteLen > nAvailable)
                nNoteLen = nAvailable;
            if (nNoteLen != 0)
            {
                std::unique_ptr<sal_Char[]> xNote(new sal_Char[nNoteLen+1]);
                nNoteLen = rStream.Read(xNote.get(), nNoteLen);
                xNote[nNoteLen] = 0;
                OUString aNoteText( SC10TOSTRING(xNote.get()));
                xNote.reset();
                ScAddress aPos( Col, static_cast<SCROW>(Row), Tab );
                ScNoteUtil::CreateNoteFromString( *pDoc, aPos, aNoteText, false, false );
            }
        }
        pPrgrsBar->Progress();
    }
}

void Sc10Import::LoadColAttr(SCCOL Col, SCTAB Tab)
{
    Sc10ColAttr aFont;
    Sc10ColAttr aAttr;
    Sc10ColAttr aJustify;
    Sc10ColAttr aFrame;
    Sc10ColAttr aRaster;
    Sc10ColAttr aValue;
    Sc10ColAttr aColor;
    Sc10ColAttr aFrameColor;
    Sc10ColAttr aFlag;
    Sc10ColAttr aPattern;

    if (nError == 0) LoadAttr(aFont);
    if (nError == 0) LoadAttr(aAttr);
    if (nError == 0) LoadAttr(aJustify);
    if (nError == 0) LoadAttr(aFrame);
    if (nError == 0) LoadAttr(aRaster);
    if (nError == 0) LoadAttr(aValue);
    if (nError == 0) LoadAttr(aColor);
    if (nError == 0) LoadAttr(aFrameColor);
    if (nError == 0) LoadAttr(aFlag);
    if (nError == 0) LoadAttr(aPattern);

    if (nError)
        return;

    SCROW nStart;
    SCROW nEnd;
    sal_uInt16 i;
    sal_uInt16 nLimit;
    sal_uInt16 nValue1;
    Sc10ColData *pColData;

    // Font (Name, Size)
    nStart = 0;
    nEnd = 0;
    nLimit = aFont.Count;
    pColData = aFont.pData;
    for( i = 0 ; i < nLimit ; i++, pColData++ )
    {
        nEnd = static_cast<SCROW>(pColData->Row);
        if ((nStart <= nEnd) && (pColData->Value))
        {
            FontFamily eFam = FAMILY_DONTKNOW;
            Sc10FontData* pFont = pFontCollection->At(pColData->Value);
            if (pFont)
            {
                switch (pFont->PitchAndFamily & 0xF0)
                {
                    case ffDontCare   : eFam = FAMILY_DONTKNOW;     break;
                    case ffRoman      : eFam = FAMILY_ROMAN;        break;
                    case ffSwiss      : eFam = FAMILY_SWISS;        break;
                    case ffModern     : eFam = FAMILY_MODERN;       break;
                    case ffScript     : eFam = FAMILY_SCRIPT;       break;
                    case ffDecorative : eFam = FAMILY_DECORATIVE;   break;
                    default: eFam = FAMILY_DONTKNOW;        break;
                }
                ScPatternAttr aScPattern(pDoc->GetPool());
                aScPattern.GetItemSet().Put(SvxFontItem(eFam, SC10TOSTRING( pFont->FaceName ), EMPTY_OUSTRING,
                    PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ));
                aScPattern.GetItemSet().Put(SvxFontHeightItem(std::abs(pFont->Height), 100, ATTR_FONT_HEIGHT ));
                pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aScPattern);
            }
        }
        nStart = nEnd + 1;
    }

    // Font color
    nStart = 0;
    nEnd = 0;
    nLimit = aColor.Count;
    pColData = aColor.pData;
    for( i = 0 ; i < nLimit ; i++, pColData++ )
    {
        nEnd = static_cast<SCROW>(pColData->Row);
        if ((nStart <= nEnd) && (pColData->Value))
        {
            Color TextColor(COL_BLACK);
            lcl_ChangeColor((pColData->Value & 0x000F), TextColor);
            ScPatternAttr aScPattern(pDoc->GetPool());
            aScPattern.GetItemSet().Put(SvxColorItem(TextColor, ATTR_FONT_COLOR ));
            pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aScPattern);
        }
        nStart = nEnd + 1;
    }

    // Font attributes (Bold, Italic...)
    nStart = 0;
    nEnd = 0;
    nLimit = aAttr.Count;
    pColData = aAttr.pData;
    for( i = 0 ; i < nLimit ; i++, pColData++ )
    {
        nEnd = static_cast<SCROW>(pColData->Row);
        nValue1 = pColData->Value;
        if ((nStart <= nEnd) && (nValue1))
        {
            ScPatternAttr aScPattern(pDoc->GetPool());
            if ((nValue1 & atBold) == atBold)
                aScPattern.GetItemSet().Put(SvxWeightItem(WEIGHT_BOLD, ATTR_FONT_WEIGHT));
            if ((nValue1 & atItalic) == atItalic)
                aScPattern.GetItemSet().Put(SvxPostureItem(ITALIC_NORMAL, ATTR_FONT_POSTURE));
            if ((nValue1 & atUnderline) == atUnderline)
                aScPattern.GetItemSet().Put(SvxUnderlineItem(UNDERLINE_SINGLE, ATTR_FONT_UNDERLINE));
            if ((nValue1 & atStrikeOut) == atStrikeOut)
                aScPattern.GetItemSet().Put(SvxCrossedOutItem(STRIKEOUT_SINGLE, ATTR_FONT_CROSSEDOUT));
            pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aScPattern);
        }
        nStart = nEnd + 1;
    }

    // Cell alignment
    nStart = 0;
    nEnd = 0;
    nLimit = aJustify.Count;
    pColData = aJustify.pData;
    for( i = 0 ; i < nLimit ; i++, pColData++ )
    {
        nEnd = static_cast<SCROW>(pColData->Row);
        nValue1 = pColData->Value;
        if ((nStart <= nEnd) && (nValue1))
        {
            ScPatternAttr aScPattern(pDoc->GetPool());
            sal_uInt16 HorJustify = (nValue1 & 0x000F);
            sal_uInt16 VerJustify = (nValue1 & 0x00F0) >> 4;
            sal_uInt16 OJustify   = (nValue1 & 0x0F00) >> 8;
            sal_uInt16 EJustify   = (nValue1 & 0xF000) >> 12;

            switch (HorJustify)
            {
                case hjLeft:
                    aScPattern.GetItemSet().Put(SvxHorJustifyItem(SVX_HOR_JUSTIFY_LEFT, ATTR_HOR_JUSTIFY));
                    break;
                case hjCenter:
                    aScPattern.GetItemSet().Put(SvxHorJustifyItem(SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY));
                    break;
                case hjRight:
                    aScPattern.GetItemSet().Put(SvxHorJustifyItem(SVX_HOR_JUSTIFY_RIGHT, ATTR_HOR_JUSTIFY));
                    break;
            }

            switch (VerJustify)
            {
                case vjTop:
                    aScPattern.GetItemSet().Put(SvxVerJustifyItem(SVX_VER_JUSTIFY_TOP, ATTR_VER_JUSTIFY));
                    break;
                case vjCenter:
                    aScPattern.GetItemSet().Put(SvxVerJustifyItem(SVX_VER_JUSTIFY_CENTER, ATTR_VER_JUSTIFY));
                    break;
                case vjBottom:
                    aScPattern.GetItemSet().Put(SvxVerJustifyItem(SVX_VER_JUSTIFY_BOTTOM, ATTR_VER_JUSTIFY));
                    break;
            }

            if (OJustify & ojWordBreak)
                aScPattern.GetItemSet().Put(SfxBoolItem(ATTR_LINEBREAK, true));
            if (OJustify & ojBottomTop)
                aScPattern.GetItemSet().Put(SfxInt32Item(ATTR_ROTATE_VALUE,9000));
            else if (OJustify & ojTopBottom)
                aScPattern.GetItemSet().Put(SfxInt32Item(ATTR_ROTATE_VALUE,27000));

            sal_Int16 Margin = std::max((sal_uInt16)20, (sal_uInt16)(EJustify * 20));
            if (OJustify & ojBottomTop)
                aScPattern.GetItemSet().Put(SvxMarginItem(20, Margin, 20, Margin, ATTR_MARGIN));
            else
                aScPattern.GetItemSet().Put(SvxMarginItem(Margin, 20, Margin, 20, ATTR_MARGIN));
            pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aScPattern);
        }
        nStart = nEnd + 1;
    }
    // Border
    bool            bEnd = false;
    sal_uInt16          nColorIndex = 0;
    sal_uInt16          nFrameIndex = 0;

    // Special Fix...
    const sal_uInt32    nHelpMeStart = 100;
    sal_uInt32          nHelpMe = nHelpMeStart;
    sal_uInt16          nColorIndexOld = nColorIndex;
    sal_uInt16          nFrameIndexOld = nColorIndex;

    nEnd = 0;
    nStart = 0;
    while( !bEnd && nHelpMe )
    {
        pColData = &aFrame.pData[ nFrameIndex ];

        sal_uInt16  nValue  = pColData->Value;
        sal_uInt16  nLeft   = 0;
        sal_uInt16  nTop    = 0;
        sal_uInt16  nRight  = 0;
        sal_uInt16  nBottom = 0;
        sal_uInt16  fLeft   = ( nValue & 0x000F );
        sal_uInt16  fTop    = ( nValue & 0x00F0 ) >> 4;
        sal_uInt16  fRight  = ( nValue & 0x0F00 ) >> 8;
        sal_uInt16  fBottom = ( nValue & 0xF000 ) >> 12;

        if( fLeft > 1 )
            nLeft = 50;
        else if( fLeft > 0 )
            nLeft = 20;

        if( fTop > 1 )
            nTop = 50;
        else if( fTop > 0 )
            nTop = 20;

        if( fRight > 1 )
            nRight = 50;
        else if( fRight > 0 )
            nRight = 20;

        if( fBottom > 1 )
            nBottom = 50;
        else if( fBottom > 0 )
            nBottom = 20;

        Color   ColorLeft( COL_BLACK );
        Color   ColorTop( COL_BLACK );
        Color   ColorRight( COL_BLACK );
        Color   ColorBottom( COL_BLACK );
        sal_uInt16  nFrmColVal  = aFrameColor.pData[ nColorIndex ].Value;
        SCROW   nFrmColRow  = static_cast<SCROW>(aFrameColor.pData[ nColorIndex ].Row);
        sal_uInt16  cLeft       = ( nFrmColVal & 0x000F );
        sal_uInt16  cTop        = ( nFrmColVal & 0x00F0 ) >> 4;
        sal_uInt16  cRight      = ( nFrmColVal & 0x0F00 ) >> 8;
        sal_uInt16  cBottom     = ( nFrmColVal & 0xF000 ) >> 12;

        lcl_ChangeColor( cLeft, ColorLeft );
        lcl_ChangeColor( cTop, ColorTop );
        lcl_ChangeColor( cRight, ColorRight );
        lcl_ChangeColor( cBottom, ColorBottom );

        if( static_cast<SCROW>(pColData->Row) < nFrmColRow )
        {
            nEnd = static_cast<SCROW>(pColData->Row);
            if( nFrameIndex < ( aFrame.Count - 1 ) )
                nFrameIndex++;
        }
        else if( static_cast<SCROW>(pColData->Row) > nFrmColRow )
        {
            nEnd = static_cast<SCROW>(aFrameColor.pData[ nColorIndex ].Row);
            if( nColorIndex < ( aFrameColor.Count - 1 ) )
                nColorIndex++;
        }
        else
        {
            nEnd = nFrmColRow;
            if( nFrameIndex < (aFrame.Count - 1 ) )
                nFrameIndex++;
            if( nColorIndex < ( aFrameColor.Count - 1 ) )
                nColorIndex++;
        }
        if( ( nStart <= nEnd ) && ( nValue != 0 ) )
        {
            ScPatternAttr   aScPattern(pDoc->GetPool());
            ::editeng::SvxBorderLine    aLine;
            SvxBoxItem      aBox( ATTR_BORDER );

            aLine.SetWidth( nLeft );
            aLine.SetColor( ColorLeft );
            aBox.SetLine( &aLine, SvxBoxItemLine::LEFT );

            aLine.SetWidth( nTop );
            aLine.SetColor( ColorTop );
            aBox.SetLine( &aLine, SvxBoxItemLine::TOP );

            aLine.SetWidth( nRight );
            aLine.SetColor( ColorRight );
            aBox.SetLine( &aLine, SvxBoxItemLine::RIGHT );

            aLine.SetWidth( nBottom );
            aLine.SetColor( ColorBottom );
            aBox.SetLine( &aLine, SvxBoxItemLine::BOTTOM );

            aScPattern.GetItemSet().Put( aBox );
            pDoc->ApplyPatternAreaTab( Col, nStart, Col, nEnd, Tab, aScPattern );
        }
        nStart = nEnd + 1;

        bEnd = ( nFrameIndex == ( aFrame.Count - 1 ) ) && ( nColorIndex == ( aFrameColor.Count - 1 ) );

        if( nColorIndexOld != nColorIndex || nFrameIndexOld != nFrameIndex )
        {
            nColorIndexOld = nColorIndex;
            nFrameIndexOld = nFrameIndex;
            nHelpMe = nHelpMeStart;
        }
        else
            nHelpMe--;

        pColData++;
    }

    // TODO: Code up to here works more or less ... from here I've had enough ! (GT)

    // Background (Color, Raster)
    sal_uInt16      nRasterIndex = 0;
    bEnd        = false;
    nColorIndex = 0;
    nEnd        = 0;
    nStart      = 0;

    // Special Fix...
    nHelpMe     = nHelpMeStart;
    sal_uInt16      nRasterIndexOld = nRasterIndex;

    while( !bEnd && nHelpMe )
    {
        sal_uInt16  nBColor = ( aColor.pData[ nColorIndex ].Value & 0x00F0 ) >> 4;
        sal_uInt16  nRColor = ( aColor.pData[ nColorIndex ].Value & 0x0F00 ) >> 8;
        Color   aBColor( COL_BLACK );

        lcl_ChangeColor( nBColor, aBColor );

        if( nBColor == 0 )
            aBColor.SetColor( COL_WHITE );
        else if( nBColor == 15 )
            aBColor.SetColor( COL_BLACK );

        Color   aRColor( COL_BLACK );

        lcl_ChangeColor( nRColor, aRColor );

        ScPatternAttr aScPattern( pDoc->GetPool() );

        sal_uInt16 nFact;
        bool        bSwapCol = false;
        bool        bSetItem = true;
        switch ( aRaster.pData[ nRasterIndex ].Value )
        {
        case raNone:        nFact = 0xffff; bSwapCol = true; bSetItem = (nBColor > 0); break;
        case raGray12:  nFact = (0xffff / 100) * 12;    break;
        case raGray25:  nFact = (0xffff / 100) * 25;    break;
        case raGray50:  nFact = (0xffff / 100) * 50;    break;
        case raGray75:  nFact = (0xffff / 100) * 75;    break;
        default:    nFact = 0xffff; bSetItem = (nRColor < 15);
        }
        if ( bSetItem )
        {
            if( bSwapCol )
                aScPattern.GetItemSet().Put( SvxBrushItem( GetMixedColor( aBColor, aRColor, nFact ), ATTR_BACKGROUND ) );
            else
                aScPattern.GetItemSet().Put( SvxBrushItem( GetMixedColor( aRColor, aBColor, nFact ), ATTR_BACKGROUND ) );
        }
        if( aRaster.pData[ nRasterIndex ].Row < aColor.pData[ nColorIndex ].Row )
        {
            nEnd = static_cast<SCROW>(aRaster.pData[ nRasterIndex ].Row);
            if( nRasterIndex < ( aRaster.Count - 1 ) )
                nRasterIndex++;
        }
        else if( aRaster.pData[ nRasterIndex ].Row > aColor.pData[ nColorIndex ].Row )
        {
            nEnd = static_cast<SCROW>(aColor.pData[ nColorIndex ].Row);
            if( nColorIndex < ( aColor.Count - 1 ) )
                nColorIndex++;
        }
        else
        {
            nEnd = static_cast<SCROW>(aColor.pData[ nColorIndex ].Row);
            if( nRasterIndex < ( aRaster.Count - 1 ) )
                nRasterIndex++;
            if( nColorIndex < ( aColor.Count - 1 ) )
                nColorIndex++;
        }
        if( nStart <= nEnd )
            pDoc->ApplyPatternAreaTab( Col, nStart, Col, nEnd, Tab, aScPattern );

        nStart = nEnd + 1;

        bEnd = ( nRasterIndex == ( aRaster.Count - 1 ) ) && ( nColorIndex == ( aColor.Count - 1 ) );

        if( nColorIndexOld != nColorIndex || nRasterIndexOld != nRasterIndex )
        {
            nColorIndexOld = nColorIndex;
            nRasterIndexOld = nRasterIndex;
            nHelpMe = nHelpMeStart;
        }
        else
            nHelpMe--;

        nHelpMe--;
    }

    // Number format
    nStart = 0;
    nEnd = 0;
    nLimit = aValue.Count;
    pColData = aValue.pData;
    for (i=0; i<nLimit; i++, pColData++)
    {
        nEnd = static_cast<SCROW>(pColData->Row);
        nValue1 = pColData->Value;
        if ((nStart <= nEnd) && (nValue1))
        {
            sal_uLong  nKey    = 0;
            sal_uInt16 nFormat = (nValue1 & 0x00FF);
            sal_uInt16 nInfo   = (nValue1 & 0xFF00) >> 8;
            ChangeFormat(nFormat, nInfo, nKey);
            ScPatternAttr aScPattern(pDoc->GetPool());
            aScPattern.GetItemSet().Put(SfxUInt32Item(ATTR_VALUE_FORMAT, (sal_uInt32)nKey));
            pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aScPattern);
        }
        nStart = nEnd + 1;
    }

    // Cell attributes (protected, hidden...)
    nStart = 0;
    nEnd = 0;
    for (i=0; i<aFlag.Count; i++)
    {
        nEnd = static_cast<SCROW>(aFlag.pData[i].Row);
        if ((nStart <= nEnd) && (aFlag.pData[i].Value != 0))
        {
            bool bProtect  = ((aFlag.pData[i].Value & paProtect) == paProtect);
            bool bHFormula = ((aFlag.pData[i].Value & paHideFormula) == paHideFormula);
            bool bHCell    = ((aFlag.pData[i].Value & paHideAll) == paHideAll);
            bool bHPrint   = ((aFlag.pData[i].Value & paHidePrint) == paHidePrint);
            ScPatternAttr aScPattern(pDoc->GetPool());
            aScPattern.GetItemSet().Put(ScProtectionAttr(bProtect, bHFormula, bHCell, bHPrint));
            pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aScPattern);
        }
        nStart = nEnd + 1;
    }

    // Cell style
    nStart = 0;
    nEnd = 0;
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    for (i=0; i<aPattern.Count; i++)
    {
        nEnd = static_cast<SCROW>(aPattern.pData[i].Row);
        if ((nStart <= nEnd) && (aPattern.pData[i].Value != 0))
        {
            sal_uInt16 nPatternIndex = (aPattern.pData[i].Value & 0x00FF) - 1;
            Sc10PatternData* pPattern = pPatternCollection->At(nPatternIndex);
            if (pPattern != nullptr)
            {
                ScStyleSheet* pStyle = static_cast<ScStyleSheet*>( pStylePool->Find(
                                    SC10TOSTRING( pPattern->Name ), SFX_STYLE_FAMILY_PARA) );

                if (pStyle != nullptr)
                    pDoc->ApplyStyleAreaTab(Col, nStart, Col, nEnd, Tab, *pStyle);
            }
        }
        nStart = nEnd + 1;
    }
}

void Sc10Import::LoadAttr(Sc10ColAttr& rAttr)
{
    // rAttr is not reused, otherwise we'd have to delete [] rAttr.pData;
    rStream.ReadUInt16(rAttr.Count);

    const size_t nMaxEntries = rStream.remainingSize() / (sizeof(sal_uInt16) * 2);
    if (rAttr.Count > nMaxEntries)
        rAttr.Count = nMaxEntries;

    if (!rAttr.Count)
        return;

    rAttr.pData = new (::std::nothrow) Sc10ColData[rAttr.Count];
    if (rAttr.pData == nullptr)
    {
        nError = errOutOfMemory;
        rAttr.Count = 0;
        return;
    }

    for (sal_uInt16 i = 0; i < rAttr.Count; ++i)
    {
        rStream.ReadUInt16( rAttr.pData[i].Row );
        rStream.ReadUInt16( rAttr.pData[i].Value );
    }

    nError = rStream.GetError();
}

void Sc10Import::ChangeFormat(sal_uInt16 nFormat, sal_uInt16 nInfo, sal_uLong& nKey)
{
    // TODO: formats are mapped only for StarCalc 3.0 internal formats
    //       more correctly, at times new formats need to be created (Stephan: please check!)
    nKey = 0;
    switch (nFormat)
    {
        case vfStandard :
            if (nInfo > 0)
                nKey = 2;
            break;
        case vfMoney :
            if (nInfo > 0)
                nKey = 21;
            else
                nKey = 20;
            break;
        case vfThousend :
            if (nInfo > 0)
                nKey = 4;
            else
                nKey = 5;
            break;
        case vfPercent :
            if (nInfo > 0)
                nKey = 11;
            else
                nKey = 10;
            break;
        case vfExponent :
            nKey = 60;
            break;
        case vfZerro :
            // TODO: no equivalent
            break;
        case vfDate :
            switch (nInfo)
            {
                case df_NDMY_Long :
                    nKey = 31;
                    break;
                case df_DMY_Long :
                    nKey = 30;
                    break;
                case df_MY_Long :
                    nKey = 32;
                    break;
                case df_NDM_Long :
                    nKey = 31;
                    break;
                case df_DM_Long :
                    nKey = 33;
                    break;
                case df_M_Long :
                    nKey = 34;
                    break;
                case df_NDMY_Short :
                    nKey = 31;
                    break;
                case df_DMY_Short :
                    nKey = 30;
                    break;
                case df_MY_Short :
                    nKey = 32;
                    break;
                case df_NDM_Short :
                    nKey = 31;
                    break;
                case df_DM_Short :
                    nKey = 33;
                    break;
                case df_M_Short :
                    nKey = 34;
                    break;
                case df_Q_Long :
                    nKey = 35;
                    break;
                case df_Q_Short :
                    nKey = 35;
                    break;
                default :
                    nKey = 30;
                    break;
            }
            break;
        case vfTime :
            switch (nInfo)
            {
                case tf_HMS_Long :
                    nKey = 41;
                    break;
                case tf_HM_Long :
                    nKey = 40;
                    break;
                case tf_HMS_Short :
                    nKey = 43;
                    break;
                case tf_HM_Short :
                    nKey = 42;
                    break;
                default :
                    nKey = 41;
                    break;
            }
            break;
        case vfBoolean :
            nKey = 99;
            break;
        case vfStandardRed :
            if (nInfo > 0)
                nKey = 2;
            break;
        case vfMoneyRed :
            if (nInfo > 0)
                nKey = 23;
            else
                nKey = 22;
            break;
        case vfThousendRed :
            if (nInfo > 0)
                nKey = 4;
            else
                nKey = 5;
            break;
        case vfPercentRed :
            if (nInfo > 0)
                nKey = 11;
            else
                nKey = 10;
            break;
        case vfExponentRed :
            nKey = 60;
            break;
        case vfFormula :
            break;
        case vfString :
            break;
        default :
            break;
    }
}

void Sc10Import::LoadObjects()
{
    sal_uInt16 ID;
    rStream.ReadUInt16( ID );
    if (rStream.IsEof())
        return;
    if (ID == ObjectID)
    {
        sal_uInt16 nAnz;
        rStream.ReadUInt16( nAnz );
        sal_Char Reserved[32];
        rStream.Read(Reserved, sizeof(Reserved));
        nError = rStream.GetError();
        if ((nAnz > 0) && (nError == 0))
        {
            sal_uInt8 ObjectType;
            Sc10GraphHeader GraphHeader;
            bool IsOleObject = false; // TODO: this is only a band-aid
            for (sal_uInt16 i = 0; (i < nAnz) && (nError == 0) && !rStream.IsEof() && !IsOleObject; i++)
            {
                rStream.ReadUChar( ObjectType );
                lcl_ReadGraphHeader(rStream, GraphHeader);

                double nPPTX = ScGlobal::nScreenPPTX;
                double nPPTY = ScGlobal::nScreenPPTY;

                long nStartX = 0;
                SCCOL nMaxCol = SanitizeCol(GraphHeader.CarretX);
                for (SCCOL nX = 0; nX < nMaxCol; ++nX)
                    nStartX += pDoc->GetColWidth(nX, static_cast<SCTAB>(GraphHeader.CarretZ));
                nStartX = (long) ( nStartX * HMM_PER_TWIPS );
                nStartX += (long) ( GraphHeader.x / nPPTX * HMM_PER_TWIPS );
                long nSizeX = (long) ( GraphHeader.w / nPPTX * HMM_PER_TWIPS );
                long nStartY = pDoc->GetRowHeight( 0,
                        SanitizeRow(static_cast<SCsROW>(GraphHeader.CarretY) - 1),
                        SanitizeTab(static_cast<SCTAB>(GraphHeader.CarretZ)));
                nStartY = (long) ( nStartY * HMM_PER_TWIPS );
                nStartY += (long) ( GraphHeader.y / nPPTY * HMM_PER_TWIPS );
                long nSizeY = (long) ( GraphHeader.h / nPPTY * HMM_PER_TWIPS );

                switch (ObjectType)
                {
                    case otOle :
                        // TODO: here we need to do something like OleLoadFromStream
                        IsOleObject = true;
                        break;
                    case otImage :
                    {
                        Sc10ImageHeader ImageHeader;
                        lcl_ReadImageHeaer(rStream, ImageHeader);

                        // Attention: here come the data (Bitmap oder Metafile)
                        // Typ = 1 Device-dependend Bitmap DIB
                        // Typ = 2 MetaFile
                        rStream.SeekRel(ImageHeader.Size);

                        if( ImageHeader.Typ != 1 && ImageHeader.Typ != 2 )
                            nError = errUnknownFormat;
                        break;
                    }
                    case otChart :
                    {
                        Sc10ChartHeader ChartHeader;
                        Sc10ChartSheetData ChartSheetData;
                        Sc10ChartTypeData* pTypeData = new (::std::nothrow) Sc10ChartTypeData;
                        if (!pTypeData)
                            nError = errOutOfMemory;
                        else
                        {
                            lcl_ReadChartHeader(rStream, ChartHeader);

                            // TODO: use old Metafile ??
                            rStream.SeekRel(ChartHeader.Size);

                            lcl_ReadChartSheetData(rStream, ChartSheetData);

                            lcl_ReadChartTypeData(rStream, *pTypeData);

                            Rectangle aRect( Point(nStartX,nStartY), Size(nSizeX,nSizeY) );
                            Sc10InsertObject::InsertChart( pDoc, static_cast<SCTAB>(GraphHeader.CarretZ), aRect,
                                    static_cast<SCTAB>(GraphHeader.CarretZ),
                                    ChartSheetData.DataX1, ChartSheetData.DataY1,
                                    ChartSheetData.DataX2, ChartSheetData.DataY2 );

                            delete pTypeData;
                        }
                        break;
                    }
                    default :
                        nError = errUnknownFormat;
                        break;
                }
                nError = rStream.GetError();
            }
        }
    }
    else
    {
        OSL_FAIL( "ObjectID" );
        nError = errUnknownID;
    }
}

FltError ScFormatFilterPluginImpl::ScImportStarCalc10( SvStream& rStream, ScDocument* pDocument )
{
    rStream.Seek( 0UL );
    Sc10Import  aImport( rStream, pDocument );
    return ( FltError ) aImport.Import();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
