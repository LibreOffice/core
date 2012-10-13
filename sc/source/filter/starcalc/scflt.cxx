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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crsditem.hxx>
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
#include <stdio.h>
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
#include "cell.hxx"
#include "scfobj.hxx"
#include "docoptio.hxx"
#include "viewopti.hxx"
#include "postit.hxx"
#include "globstr.hrc"
#include "ftools.hxx"
#include "tabprotection.hxx"

#include "fprogressbar.hxx"

using namespace com::sun::star;

#define DEFCHARSET          RTL_TEXTENCODING_MS_1252

#define SC10TOSTRING(p)     String((p),DEFCHARSET)

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
    rStream >> rFileHeader.Version;
    rStream.Read(&rFileHeader.Reserved, sizeof(rFileHeader.Reserved));
}


static void lcl_ReadTabProtect(SvStream& rStream, Sc10TableProtect& rProtect)
{
    lcl_ReadFixedString( rStream, &rProtect.PassWord, sizeof(rProtect.PassWord));
    rStream >> rProtect.Flags;
    rStream >> rProtect.Protect;
}


static void lcl_ReadSheetProtect(SvStream& rStream, Sc10SheetProtect& rProtect)
{
    lcl_ReadFixedString( rStream, &rProtect.PassWord, sizeof(rProtect.PassWord));
    rStream >> rProtect.Flags;
    rStream >> rProtect.Protect;
}


static void lcl_ReadRGB(SvStream& rStream, Sc10Color& rColor)
{
    rStream >> rColor.Dummy;
    rStream >> rColor.Blue;
    rStream >> rColor.Green;
    rStream >> rColor.Red;
}


static void lcl_ReadPalette(SvStream& rStream, Sc10Color* pPalette)
{
    for (sal_uInt16 i = 0; i < 16; i++)
        lcl_ReadRGB(rStream, pPalette[i]);
}


static void lcl_ReadValueFormat(SvStream& rStream, Sc10ValueFormat& rFormat)
{
    rStream >> rFormat.Format;
    rStream >> rFormat.Info;
}


static void lcl_ReadLogFont(SvStream& rStream, Sc10LogFont& rFont)
{
    rStream >> rFont.lfHeight;
    rStream >> rFont.lfWidth;
    rStream >> rFont.lfEscapement;
    rStream >> rFont.lfOrientation;
    rStream >> rFont.lfWeight;
    rStream >> rFont.lfItalic;
    rStream >> rFont.lfUnderline;
    rStream >> rFont.lfStrikeOut;
    rStream >> rFont.lfCharSet;
    rStream >> rFont.lfOutPrecision;
    rStream >> rFont.lfClipPrecision;
    rStream >> rFont.lfQuality;
    rStream >> rFont.lfPitchAndFamily;
    lcl_ReadFixedString( rStream, &rFont.lfFaceName, sizeof(rFont.lfFaceName));
}


static void lcl_ReadBlockRect(SvStream& rStream, Sc10BlockRect& rBlock)
{
    rStream >> rBlock.x1;
    rStream >> rBlock.y1;
    rStream >> rBlock.x2;
    rStream >> rBlock.y2;
}


static void lcl_ReadHeadFootLine(SvStream& rStream, Sc10HeadFootLine& rLine)
{
    lcl_ReadFixedString( rStream, &rLine.Title, sizeof(rLine.Title));
    lcl_ReadLogFont(rStream, rLine.LogFont);
    rStream >> rLine.HorJustify;
    rStream >> rLine.VerJustify;
    rStream >> rLine.Raster;
    rStream >> rLine.Frame;
    lcl_ReadRGB(rStream, rLine.TextColor);
    lcl_ReadRGB(rStream, rLine.BackColor);
    lcl_ReadRGB(rStream, rLine.RasterColor);
    rStream >> rLine.FrameColor;
    rStream >> rLine.Reserved;
}


static void lcl_ReadPageFormat(SvStream& rStream, Sc10PageFormat& rFormat)
{
    lcl_ReadHeadFootLine(rStream, rFormat.HeadLine);
    lcl_ReadHeadFootLine(rStream, rFormat.FootLine);
    rStream >> rFormat.Orientation;
    rStream >> rFormat.Width;
    rStream >> rFormat.Height;
    rStream >> rFormat.NonPrintableX;
    rStream >> rFormat.NonPrintableY;
    rStream >> rFormat.Left;
    rStream >> rFormat.Top;
    rStream >> rFormat.Right;
    rStream >> rFormat.Bottom;
    rStream >> rFormat.Head;
    rStream >> rFormat.Foot;
    rStream >> rFormat.HorCenter;
    rStream >> rFormat.VerCenter;
    rStream >> rFormat.PrintGrid;
    rStream >> rFormat.PrintColRow;
    rStream >> rFormat.PrintNote;
    rStream >> rFormat.TopBottomDir;
    lcl_ReadFixedString( rStream, &rFormat.PrintAreaName, sizeof(rFormat.PrintAreaName));
    lcl_ReadBlockRect(rStream, rFormat.PrintArea);
    rStream.Read(&rFormat.PrnZoom, sizeof(rFormat.PrnZoom));
    rStream >> rFormat.FirstPageNo;
    rStream >> rFormat.RowRepeatStart;
    rStream >> rFormat.RowRepeatEnd;
    rStream >> rFormat.ColRepeatStart;
    rStream >> rFormat.ColRepeatEnd;
    rStream.Read(&rFormat.Reserved, sizeof(rFormat.Reserved));
}


static void lcl_ReadGraphHeader(SvStream& rStream, Sc10GraphHeader& rHeader)
{
    rStream >> rHeader.Typ;
    rStream >> rHeader.CarretX;
    rStream >> rHeader.CarretY;
    rStream >> rHeader.CarretZ;
    rStream >> rHeader.x;
    rStream >> rHeader.y;
    rStream >> rHeader.w;
    rStream >> rHeader.h;
    rStream >> rHeader.IsRelPos;
    rStream >> rHeader.DoPrint;
    rStream >> rHeader.FrameType;
    rStream >> rHeader.IsTransparent;
    lcl_ReadRGB(rStream, rHeader.FrameColor);
    lcl_ReadRGB(rStream, rHeader.BackColor);
    rStream.Read(&rHeader.Reserved, sizeof(rHeader.Reserved));
}


static void lcl_ReadImageHeaer(SvStream& rStream, Sc10ImageHeader& rHeader)
{
    lcl_ReadFixedString( rStream, &rHeader.FileName, sizeof(rHeader.FileName));
    rStream >> rHeader.Typ;
    rStream >> rHeader.Linked;
    rStream >> rHeader.x1;
    rStream >> rHeader.y1;
    rStream >> rHeader.x2;
    rStream >> rHeader.y2;
    rStream >> rHeader.Size;
}


static void lcl_ReadChartHeader(SvStream& rStream, Sc10ChartHeader& rHeader)
{
    rStream >> rHeader.MM;
    rStream >> rHeader.xExt;
    rStream >> rHeader.yExt;
    rStream >> rHeader.Size;
}


static void lcl_ReadChartSheetData(SvStream& rStream, Sc10ChartSheetData& rSheetData)
{
    rStream >> rSheetData.HasTitle;
    rStream >> rSheetData.TitleX;
    rStream >> rSheetData.TitleY;
    rStream >> rSheetData.HasSubTitle;
    rStream >> rSheetData.SubTitleX;
    rStream >> rSheetData.SubTitleY;
    rStream >> rSheetData.HasLeftTitle;
    rStream >> rSheetData.LeftTitleX;
    rStream >> rSheetData.LeftTitleY;
    rStream >> rSheetData.HasLegend;
    rStream >> rSheetData.LegendX1;
    rStream >> rSheetData.LegendY1;
    rStream >> rSheetData.LegendX2;
    rStream >> rSheetData.LegendY2;
    rStream >> rSheetData.HasLabel;
    rStream >> rSheetData.LabelX1;
    rStream >> rSheetData.LabelY1;
    rStream >> rSheetData.LabelX2;
    rStream >> rSheetData.LabelY2;
    rStream >> rSheetData.DataX1;
    rStream >> rSheetData.DataY1;
    rStream >> rSheetData.DataX2;
    rStream >> rSheetData.DataY2;
    rStream.Read(&rSheetData.Reserved, sizeof(rSheetData.Reserved));
}


static void lcl_ReadChartTypeData(SvStream& rStream, Sc10ChartTypeData& rTypeData)
{
    rStream >> rTypeData.NumSets;
    rStream >> rTypeData.NumPoints;
    rStream >> rTypeData.DrawMode;
    rStream >> rTypeData.GraphType;
    rStream >> rTypeData.GraphStyle;
    lcl_ReadFixedString( rStream, &rTypeData.GraphTitle, sizeof(rTypeData.GraphTitle));
    lcl_ReadFixedString( rStream, &rTypeData.BottomTitle, sizeof(rTypeData.BottomTitle));
    sal_uInt16 i;
    for (i = 0; i < 256; i++)
        rStream >> rTypeData.SymbolData[i];
    for (i = 0; i < 256; i++)
        rStream >> rTypeData.ColorData[i];
    for (i = 0; i < 256; i++)
        rStream >> rTypeData.ThickLines[i];
    for (i = 0; i < 256; i++)
        rStream >> rTypeData.PatternData[i];
    for (i = 0; i < 256; i++)
        rStream >> rTypeData.LinePatternData[i];
    for (i = 0; i < 11; i++)
        rStream >> rTypeData.NumGraphStyles[i];
    rStream >> rTypeData.ShowLegend;
    for (i = 0; i < 256; i++)
        lcl_ReadFixedString( rStream, &rTypeData.LegendText[i], sizeof(Sc10ChartText));
    rStream >> rTypeData.ExplodePie;
    rStream >> rTypeData.FontUse;
    for (i = 0; i < 5; i++)
        rStream >> rTypeData.FontFamily[i];
    for (i = 0; i < 5; i++)
        rStream >> rTypeData.FontStyle[i];
    for (i = 0; i < 5; i++)
        rStream >> rTypeData.FontSize[i];
    rStream >> rTypeData.GridStyle;
    rStream >> rTypeData.Labels;
    rStream >> rTypeData.LabelEvery;
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

static String lcl_MakeOldPageStyleFormatName( sal_uInt16 i )
{
    String  aName = ScGlobal::GetRscString( STR_PAGESTYLE );
    aName.AppendAscii( " " );
    aName += String::CreateFromInt32( i + 1 );

    return aName;
}


template < typename T > sal_uLong insert_new( ScCollection* pCollection, SvStream& rStream )
{
    T* pData = new (::std::nothrow) T( rStream);
    sal_uLong nError = rStream.GetError();
    if (pData)
    {
        if (nError)
            delete pData;
        else
            pCollection->Insert( pData);
    }
    else
        nError = errOutOfMemory;
    return nError;
}

//--------------------------------------------
// Font
//--------------------------------------------

Sc10FontData::Sc10FontData(SvStream& rStream)
{
    rStream >> Height;
    rStream >> CharSet;
    rStream >> PitchAndFamily;
    sal_uInt16 nLen;
    rStream >> nLen;
    if (nLen < sizeof(FaceName))
        rStream.Read(FaceName, nLen);
    else
        rStream.SetError(ERRCODE_IO_WRONGFORMAT);
}


Sc10FontCollection::Sc10FontCollection(SvStream& rStream) :
    ScCollection (4, 4),
    nError     (0)
{
  sal_uInt16 ID;
  rStream >> ID;
  if (ID == FontID)
  {
    sal_uInt16 nAnz;
    rStream >> nAnz;
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

//--------------------------------------------
// Benannte-Bereiche
//--------------------------------------------

Sc10NameData::Sc10NameData(SvStream& rStream)
{
    sal_uInt8 nLen;
    rStream >> nLen;
    rStream.Read(Name, sizeof(Name) - 1);
    if (nLen >= sizeof(Name))
        nLen = sizeof(Name) - 1;
    Name[nLen] = 0;

    rStream >> nLen;
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
  rStream >> ID;
  if (ID == NameID)
  {
    sal_uInt16 nAnz;
    rStream >> nAnz;
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

//--------------------------------------------
// Vorlagen
//--------------------------------------------

Sc10PatternData::Sc10PatternData(SvStream& rStream)
{
  lcl_ReadFixedString( rStream, Name, sizeof(Name));
  lcl_ReadValueFormat(rStream, ValueFormat);
  lcl_ReadLogFont(rStream, LogFont);

  rStream >> Attr;
  rStream >> Justify;
  rStream >> Frame;
  rStream >> Raster;
  rStream >> nColor;
  rStream >> FrameColor;
  rStream >> Flags;
  rStream >> FormatFlags;
  rStream.Read(Reserved, sizeof(Reserved));
}


Sc10PatternCollection::Sc10PatternCollection(SvStream& rStream) :
  ScCollection (4, 4),
  nError     (0)
{
  sal_uInt16 ID;
  rStream >> ID;
  if (ID == PatternID)
  {
    sal_uInt16 nAnz;
    rStream >> nAnz;
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

//--------------------------------------------
// Datenbank
//--------------------------------------------

Sc10DataBaseData::Sc10DataBaseData(SvStream& rStream)
{
    lcl_ReadFixedString( rStream, &DataBaseRec.Name, sizeof(DataBaseRec.Name));
    rStream >> DataBaseRec.Tab;
    lcl_ReadBlockRect(rStream, DataBaseRec.Block);
    rStream >> DataBaseRec.RowHeader;
    rStream >> DataBaseRec.SortField0;
    rStream >> DataBaseRec.SortUpOrder0;
    rStream >> DataBaseRec.SortField1;
    rStream >> DataBaseRec.SortUpOrder1;
    rStream >> DataBaseRec.SortField2;
    rStream >> DataBaseRec.SortUpOrder2;
    rStream >> DataBaseRec.IncludeFormat;

    rStream >> DataBaseRec.QueryField0;
    rStream >> DataBaseRec.QueryOp0;
    rStream >> DataBaseRec.QueryByString0;
    lcl_ReadFixedString( rStream, &DataBaseRec.QueryString0, sizeof(DataBaseRec.QueryString0));
    DataBaseRec.QueryValue0 = ScfTools::ReadLongDouble(rStream);

    rStream >> DataBaseRec.QueryConnect1;
    rStream >> DataBaseRec.QueryField1;
    rStream >> DataBaseRec.QueryOp1;
    rStream >> DataBaseRec.QueryByString1;
    lcl_ReadFixedString( rStream, &DataBaseRec.QueryString1, sizeof(DataBaseRec.QueryString1));
    DataBaseRec.QueryValue1 = ScfTools::ReadLongDouble(rStream);

    rStream >> DataBaseRec.QueryConnect2;
    rStream >> DataBaseRec.QueryField2;
    rStream >> DataBaseRec.QueryOp2;
    rStream >> DataBaseRec.QueryByString2;
    lcl_ReadFixedString( rStream, &DataBaseRec.QueryString2, sizeof(DataBaseRec.QueryString2));
    DataBaseRec.QueryValue2 = ScfTools::ReadLongDouble(rStream);
}


Sc10DataBaseCollection::Sc10DataBaseCollection(SvStream& rStream) :
  ScCollection (4, 4),
  nError     (0)
{
  sal_uInt16 ID;
  rStream >> ID;
  if (ID == DataBaseID)
  {
    lcl_ReadFixedString( rStream, ActName, sizeof(ActName));
    sal_uInt16 nAnz;
    rStream >> nAnz;
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


int Sc10LogFont::operator==( const Sc10LogFont& rData ) const
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


int Sc10Color::operator==( const Sc10Color& rColor ) const
{
    return ((Red == rColor.Red) && (Green == rColor.Green) && (Blue == rColor.Blue));
}


int Sc10HeadFootLine::operator==( const Sc10HeadFootLine& rData ) const
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


int Sc10PageFormat::operator==( const Sc10PageFormat& rData ) const
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
    sal_Int32       nT = nB - nF;
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

        pPage->Width = (short) ( pPage->Width * ( PS_POINTS_PER_INCH / POINTS_PER_INCH ) + 0.5 );
        pPage->Height = (short) ( pPage->Height * ( PS_POINTS_PER_INCH / POINTS_PER_INCH ) + 0.5 );
        pPage->Top = (short) ( pPage->Top * ( PS_POINTS_PER_INCH / POINTS_PER_INCH ) + 0.5 );
        pPage->Bottom = (short) ( pPage->Bottom * ( PS_POINTS_PER_INCH / POINTS_PER_INCH ) + 0.5 );
        pPage->Left = (short) ( pPage->Left * ( PS_POINTS_PER_INCH / POINTS_PER_INCH ) + 0.5 );
        pPage->Right = (short) ( pPage->Right * ( PS_POINTS_PER_INCH / POINTS_PER_INCH ) + 0.5 );
        pPage->Head = (short) ( pPage->Head * ( PS_POINTS_PER_INCH / POINTS_PER_INCH ) + 0.5 );
        pPage->Foot = (short) ( pPage->Foot * ( PS_POINTS_PER_INCH / POINTS_PER_INCH ) + 0.5 );

        String aName = lcl_MakeOldPageStyleFormatName( i );

        ScStyleSheet* pSheet = (ScStyleSheet*) &pStylePool->Make( aName,
                                    SFX_STYLE_FAMILY_PAGE,
                                    SFXSTYLEBIT_USERDEF | SCSTYLEBIT_STANDARD );
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
                                    SC10TOSTRING( pHeadFootLine->LogFont.lfFaceName ), EMPTY_STRING,
                                    PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, EE_CHAR_FONTINFO ),
                                EE_CHAR_FONTINFO );
            aEditAttribs.Put(   SvxFontHeightItem( Abs( pHeadFootLine->LogFont.lfHeight ), 100, EE_CHAR_FONTHEIGHT ),
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
            String aText( pHeadFootLine->Title, DEFCHARSET );
            aEditEngine.SetText( aText );
            aEditEngine.QuickSetAttribs( aEditAttribs, ESelection( 0, 0, 0, aText.Len() ) );

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
            sal_Bool        bSwapCol = false;
            switch (pHeadFootLine->Raster)
            {
               case raNone:     nFact = 0xffff; bSwapCol = sal_True; break;
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
              aBox.SetLine(&aLine, BOX_LINE_LEFT);
              aLine.SetWidth(nTop);
              aLine.SetColor(ColorTop);
              aBox.SetLine(&aLine, BOX_LINE_TOP);
              aLine.SetWidth(nRight);
              aLine.SetColor(ColorRight);
              aBox.SetLine(&aLine, BOX_LINE_RIGHT);
              aLine.SetWidth(nBottom);
              aLine.SetColor(ColorBottom);
              aBox.SetLine(&aLine, BOX_LINE_BOTTOM);

              aSetItemItemSet.Put(aBox);
            }

            pSet->Put( SvxULSpaceItem( 0, 0, ATTR_ULSPACE ) );

            if (nHeadFoot==0)
                aSetItemItemSet.Put( SvxSizeItem( ATTR_PAGE_SIZE, Size( 0, pPage->Top - pPage->Head ) ) );
            else
                aSetItemItemSet.Put( SvxSizeItem( ATTR_PAGE_SIZE, Size( 0, pPage->Bottom - pPage->Foot ) ) );

            aSetItemItemSet.Put(SfxBoolItem( ATTR_PAGE_ON, sal_True ));
            aSetItemItemSet.Put(SfxBoolItem( ATTR_PAGE_DYNAMIC, false ));
            aSetItemItemSet.Put(SfxBoolItem( ATTR_PAGE_SHARED, sal_True ));

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

        //----------------
        // Area-Parameter:
        //----------------
        {
            ScRange* pRepeatRow = NULL;
            ScRange* pRepeatCol = NULL;

            if ( pPage->ColRepeatStart >= 0 )
                pRepeatCol = new ScRange( static_cast<SCCOL> (pPage->ColRepeatStart), 0, 0 );
            if ( pPage->RowRepeatStart >= 0 )
                pRepeatRow = new ScRange( 0, static_cast<SCROW> (pPage->RowRepeatStart), 0 );


            if ( pRepeatRow || pRepeatCol )
            {
                //
                // an allen Tabellen setzen
                //
                for ( SCTAB nTab = 0, nTabCount = pDoc->GetTableCount(); nTab < nTabCount; ++nTab )
                {
                    pDoc->SetRepeatColRange( nTab, pRepeatCol );
                    pDoc->SetRepeatRowRange( nTab, pRepeatRow );
                }
            }

            delete pRepeatRow;
            delete pRepeatCol;
        }

        //-----------------
        // Table-Parameter:
        //-----------------
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


//--------------------------------------------
// Import
//--------------------------------------------


Sc10Import::Sc10Import(SvStream& rStr, ScDocument* pDocument ) :
    rStream             (rStr),
    pDoc                (pDocument),
    pFontCollection     (NULL),
    pNameCollection     (NULL),
    pPatternCollection  (NULL),
    pDataBaseCollection (NULL),
    nError              (0),
    nShowTab            (0)
{
    pPrgrsBar = NULL;
}


Sc10Import::~Sc10Import()
{
    pDoc->CalcAfterLoad();
    pDoc->UpdateAllCharts();

    delete pFontCollection;
    delete pNameCollection;
    delete pPatternCollection;
    delete pDataBaseCollection;

    OSL_ENSURE( pPrgrsBar == NULL,
        "*Sc10Import::Sc10Import(): Progressbar lebt noch!?" );
}


sal_uLong Sc10Import::Import()
{
    pPrgrsBar = new ScfStreamProgressBar( rStream, pDoc->GetDocumentShell() );

    ScDocOptions aOpt = pDoc->GetDocOptions();
    aOpt.SetDate( 1, 1, 1900 );
    aOpt.SetYear2000( 18 + 1901 );      // ab SO51 src513e vierstellig
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
        OSL_FAIL( rtl::OString::valueOf(static_cast<sal_Int32>(nError)).getStr());
    }
#endif

    delete pPrgrsBar;
#if OSL_DEBUG_LEVEL > 0
    pPrgrsBar = NULL;
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
    // Achtung Info Uebertragen
}



void Sc10Import::LoadEditStateInfo()
{
    Sc10EditStateInfo EditStateInfo;
    rStream.Read(&EditStateInfo, sizeof(EditStateInfo));

    nError = rStream.GetError();
    nShowTab = static_cast<SCTAB>(EditStateInfo.DeltaZ);
    // Achtung Cursorposition und Offset der Tabelle Uebertragen (soll man das machen??)
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
    sal_uInt8 ViewColRowBar;
    rStream >> ViewColRowBar;
    nError = rStream.GetError();
    aSc30ViewOpt.SetOption( VOPT_HEADER, (sal_Bool)ViewColRowBar );
}


void Sc10Import::LoadScrZoom()
{
    // Achtung Zoom ist leider eine 6Byte TP real Zahl (keine Ahnung wie die Umzusetzen ist)
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
        String aName( pPattern->Name, DEFCHARSET );
        SfxStyleSheetBase* pStyle = pStylePool->Find( aName, SFX_STYLE_FAMILY_PARA );
        if( pStyle == NULL )
            pStylePool->Make( aName, SFX_STYLE_FAMILY_PARA );
        else
        {
            pPattern->Name[ 27 ] = 0;
            strcat( pPattern->Name, "_Old" );
            aName = SC10TOSTRING( pPattern->Name );
            pStylePool->Make( aName, SFX_STYLE_FAMILY_PARA );
        }
        pStyle = pStylePool->Find( aName, SFX_STYLE_FAMILY_PARA );
        if( pStyle != NULL )
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
                rItemSet.Put( SvxFontItem( eFam, SC10TOSTRING( pPattern->LogFont.lfFaceName ), EMPTY_STRING,
                        PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ) );
                rItemSet.Put( SvxFontHeightItem( Abs( pPattern->LogFont.lfHeight ), 100, ATTR_FONT_HEIGHT ) );
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
            // Ausrichtung
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
                    rItemSet.Put( SfxBoolItem( sal_True ) );
                if( ( OJustify & ojBottomTop ) == ojBottomTop )
                    rItemSet.Put( SfxInt32Item( ATTR_ROTATE_VALUE, 9000 ) );
                else if( ( OJustify & ojTopBottom ) == ojTopBottom )
                    rItemSet.Put( SfxInt32Item( ATTR_ROTATE_VALUE, 27000 ) );

                sal_Int16 Margin = Max( ( sal_uInt16 ) 20, ( sal_uInt16 ) ( EJustify * 20 ) );
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
                    aBox.SetLine( &aLine, BOX_LINE_LEFT );
                    aLine.SetWidth( nTop );
                    aLine.SetColor( ColorTop );
                    aBox.SetLine( &aLine, BOX_LINE_TOP );
                    aLine.SetWidth( nRight );
                    aLine.SetColor( ColorRight );
                    aBox.SetLine( &aLine, BOX_LINE_RIGHT );
                    aLine.SetWidth( nBottom );
                    aLine.SetColor( ColorBottom );
                    aBox.SetLine( &aLine, BOX_LINE_BOTTOM );
                    rItemSet.Put( aBox );
                }
            }
            // Raster
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
                    sal_Bool        bSwapCol = false;
                    sal_Bool        bSetItem = sal_True;
                    switch (pPattern->Raster)
                    {
                       case raNone:     nFact = 0xffff; bSwapCol = sal_True; bSetItem = (nBColor > 0); break;
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
            // ZahlenFormate
            if( ( pPattern->ValueFormat.Format != 0 ) &&
                ( ( pPattern->FormatFlags & pfValue ) == pfValue ) )
            {
                sal_uLong nKey = 0;
                ChangeFormat( pPattern->ValueFormat.Format, pPattern->ValueFormat.Info, nKey );
                rItemSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, ( sal_uInt32 ) nKey ) );
            }

            // Zellattribute (Schutz, Versteckt...)
            if( ( pPattern->Flags != 0 ) &&
                ( ( pPattern->FormatFlags & pfProtection ) == pfProtection ) )
            {
                sal_Bool bProtect  = ( ( pPattern->Flags & paProtect ) == paProtect );
                sal_Bool bHFormula = ( ( pPattern->Flags & paHideFormula ) == paHideFormula );
                sal_Bool bHCell    = ( ( pPattern->Flags & paHideAll ) == paHideAll );
                sal_Bool bHPrint   = ( ( pPattern->Flags & paHidePrint ) == paHidePrint );
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
                                    sal_True,
                                    ( sal_Bool) pOldData->DataBaseRec.RowHeader );
        pDoc->GetDBCollection()->getNamedDBs().insert(pNewData);
    }
}


void Sc10Import::LoadTables()
{
    Sc10PageCollection aPageCollection;

    sal_Int16 nTabCount;
    rStream >> nTabCount;
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
        String           aStr;  // Universal-Konvertierungs-String


        lcl_ReadPageFormat(rStream, PageFormat);

        sal_uInt16 nAt = aPageCollection.InsertFormat(PageFormat);
        String aPageName = lcl_MakeOldPageStyleFormatName( nAt );

        pPrgrsBar->Progress();

        rStream >> DataBaseIndex;

        lcl_ReadTabProtect(rStream, TabProtect);

        ScTableProtection aProtection;
        aProtection.setProtected(static_cast<bool>(TabProtect.Protect));
        aProtection.setPassword(SC10TOSTRING(TabProtect.PassWord));
        pDoc->SetTabProtection(static_cast<SCTAB>(Tab), &aProtection);

        rStream >> TabNo;

        sal_uInt8 nLen;
        rStream >> nLen;
        rStream.Read(TabName, sizeof(TabName) - 1);
        if (nLen >= sizeof(TabName))
            nLen = sizeof(TabName) - 1;
        TabName[nLen] = 0;

        //----------------------------------------------------------
        rStream >> Display;

        if ( Tab == (sal_Int16)nShowTab )
        {
            ScVObjMode  eObjMode = VOBJ_MODE_SHOW;

            aSc30ViewOpt.SetOption( VOPT_FORMULAS,    IS_SET(dfFormula,Display) );
            aSc30ViewOpt.SetOption( VOPT_NULLVALS,    IS_SET(dfZerro,Display) );
            aSc30ViewOpt.SetOption( VOPT_SYNTAX,      IS_SET(dfSyntax,Display) );
            aSc30ViewOpt.SetOption( VOPT_NOTES,       IS_SET(dfNoteMark,Display) );
            aSc30ViewOpt.SetOption( VOPT_VSCROLL,     sal_True );
            aSc30ViewOpt.SetOption( VOPT_HSCROLL,     sal_True );
            aSc30ViewOpt.SetOption( VOPT_TABCONTROLS, sal_True );
            aSc30ViewOpt.SetOption( VOPT_OUTLINER,    sal_True );
            aSc30ViewOpt.SetOption( VOPT_GRID,        IS_SET(dfGrid,Display) );

            // VOPT_HEADER wird in LoadViewColRowBar() gesetzt

            if ( IS_SET(dfObjectAll,Display) )          // Objekte anzeigen
                eObjMode = VOBJ_MODE_SHOW;
            else if ( IS_SET(dfObjectFrame,Display) )   // Objekte als Platzhalter
                eObjMode = VOBJ_MODE_SHOW;
            else if ( IS_SET(dfObjectNone,Display) )    // Objekte nicht anzeigen
                eObjMode = VOBJ_MODE_HIDE;

            aSc30ViewOpt.SetObjMode( VOBJ_TYPE_OLE,   eObjMode );
            aSc30ViewOpt.SetObjMode( VOBJ_TYPE_CHART, eObjMode );
            aSc30ViewOpt.SetObjMode( VOBJ_TYPE_DRAW,  eObjMode );
        }

        //--------------------------------------------------------------------
        rStream >> Visible;

        nError = rStream.GetError();
        if (nError != 0) return;

        if (TabNo == 0)
            pDoc->RenameTab(static_cast<SCTAB> (TabNo), SC10TOSTRING( TabName ), false);
        else
            pDoc->InsertTab(SC_TAB_APPEND, SC10TOSTRING( TabName ) );

        pDoc->SetPageStyle( static_cast<SCTAB>(Tab), aPageName );

        if (Visible == 0) pDoc->SetVisible(static_cast<SCTAB> (TabNo), false);

        // ColWidth
        rStream >> ID;
        if (ID != ColWidthID)
        {
            OSL_FAIL( "ColWidthID" );
            nError = errUnknownID;
            return;
        }
        rStream >> DataCount;
        DataStart = 0;
        for (i=0; i < DataCount; i++)
        {
            rStream >> DataEnd;
            rStream >> DataValue;
            for (SCCOL j = static_cast<SCCOL>(DataStart); j <= static_cast<SCCOL>(DataEnd); j++) pDoc->SetColWidth(j, static_cast<SCTAB> (TabNo), DataValue);
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // ColAttr
        rStream >> ID;
        if (ID != ColAttrID)
        {
            OSL_FAIL( "ColAttrID" );
            nError = errUnknownID;
            return;
        }

        rStream >> DataCount;
        DataStart = 0;
        for (i=0; i < DataCount; i++)
        {
            rStream >> DataEnd;
            rStream >> DataValue;
            if (DataValue != 0)
            {
                bool bPageBreak   = ((DataValue & crfSoftBreak) == crfSoftBreak);
                bool bManualBreak = ((DataValue & crfHardBreak) == crfHardBreak);
                bool bHidden = ((DataValue & crfHidden) == crfHidden);
                for (SCCOL k = static_cast<SCCOL>(DataStart); k <= static_cast<SCCOL>(DataEnd); k++)
                {
                    pDoc->SetColHidden(k, k, static_cast<SCTAB>(TabNo), bHidden);
                    pDoc->SetColBreak(k, static_cast<SCTAB> (TabNo), bPageBreak, bManualBreak);
                }
            }
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // RowHeight
        rStream >> ID;
        if (ID != RowHeightID)
        {
            OSL_FAIL( "RowHeightID" );
            nError = errUnknownID;
            return;
        }

        rStream >> DataCount;
        DataStart = 0;
        for (i=0; i < DataCount; i++)
        {
            rStream >> DataEnd;
            rStream >> DataValue;
            pDoc->SetRowHeightRange(static_cast<SCROW> (DataStart), static_cast<SCROW> (DataEnd), static_cast<SCTAB> (TabNo), DataValue);
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // RowAttr
        rStream >> ID;
        if (ID != RowAttrID)
        {
            OSL_FAIL( "RowAttrID" );
            nError = errUnknownID;
            return;
        }

        rStream >> DataCount;
        DataStart = 0;
        for (i=0; i < DataCount; i++)
        {
            rStream >> DataEnd;
            rStream >> DataValue;
            if (DataValue != 0)
            {
                bool bPageBreak   = ((DataValue & crfSoftBreak) == crfSoftBreak);
                bool bManualBreak = ((DataValue & crfHardBreak) == crfHardBreak);
                bool bHidden      = ((DataValue & crfHidden) == crfHidden);
                for (SCROW l = static_cast<SCROW>(DataStart); l <= static_cast<SCROW>(DataEnd); l++)
                {
                    pDoc->SetRowHidden(l, l, static_cast<SCTAB> (TabNo), bHidden);
                    pDoc->SetRowBreak(l, static_cast<SCTAB> (TabNo), bPageBreak, bManualBreak);
                }
            }
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // DataTable
        rStream >> ID;
        if (ID != TableID)
        {
            OSL_FAIL( "TableID" );
            nError = errUnknownID;
            return;
        }
        for (SCCOL Col = 0; (Col <= SC10MAXCOL) && (nError == 0); Col++)
        {
            rStream >> Count;
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
    rStream >> CellCount;
    SCROW nScCount = static_cast< SCROW >( CellCount );
    if (nScCount > MAXROW) nError = errUnknownFormat;
    for (sal_uInt16 i = 0; (i < CellCount) && (nError == 0); i++)
    {
        rStream >> CellType;
        rStream >> Row;
        nError = rStream.GetError();
        if (nError == 0)
        {
            switch (CellType)
            {
                case ctValue :
                {
                    const SfxPoolItem* pValueFormat = pDoc->GetAttr(Col, static_cast<SCROW> (Row), Tab, ATTR_VALUE_FORMAT);
                    sal_uLong nFormat = ((SfxUInt32Item*)pValueFormat)->GetValue();
                    double Value = ScfTools::ReadLongDouble(rStream);
                    //rStream.Read(&Value, sizeof(Value));

                    // Achtung hier ist eine Anpassung Notwendig wenn Ihr das Basisdatum aendert
                    // In StarCalc 1.0 entspricht 0 dem 01.01.1900
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
                    rStream >> Len;
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
                    rStream >> Len;
                    rStream.Read(&s[1], Len);
                    s[0] = '=';
                    s[Len + 1] = 0;
                    ScFormulaCell* pCell = new ScFormulaCell( pDoc, ScAddress( Col, static_cast<SCROW> (Row), Tab ) );
                    pCell->SetHybridFormula( SC10TOSTRING( s ),formula::FormulaGrammar::GRAM_NATIVE );
                    pDoc->PutCell( Col, static_cast<SCROW> (Row), Tab, pCell, true );
                    break;
                }
                case ctNote :
                    break;
                default :
                    nError = errUnknownFormat;
                    break;
            }
            sal_uInt16 NoteLen;
            rStream >> NoteLen;
            if (NoteLen != 0)
            {
                sal_Char* pNote = new sal_Char[NoteLen+1];
                rStream.Read(pNote, NoteLen);
                pNote[NoteLen] = 0;
                String aNoteText( SC10TOSTRING(pNote));
                delete [] pNote;
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

    if (nError == 0)
    {
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
                    aScPattern.GetItemSet().Put(SvxFontItem(eFam, SC10TOSTRING( pFont->FaceName ), EMPTY_STRING,
                        PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ));
                    aScPattern.GetItemSet().Put(SvxFontHeightItem(Abs(pFont->Height), 100, ATTR_FONT_HEIGHT ));
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
                aScPattern.GetItemSet().Put(SfxBoolItem(sal_True));
            if (OJustify & ojBottomTop)
                aScPattern.GetItemSet().Put(SfxInt32Item(ATTR_ROTATE_VALUE,9000));
            else if (OJustify & ojTopBottom)
                aScPattern.GetItemSet().Put(SfxInt32Item(ATTR_ROTATE_VALUE,27000));

            sal_Int16 Margin = Max((sal_uInt16)20, (sal_uInt16)(EJustify * 20));
            if (OJustify & ojBottomTop)
                aScPattern.GetItemSet().Put(SvxMarginItem(20, Margin, 20, Margin, ATTR_MARGIN));
            else
                aScPattern.GetItemSet().Put(SvxMarginItem(Margin, 20, Margin, 20, ATTR_MARGIN));
            pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aScPattern);
        }
        nStart = nEnd + 1;
    }
    // Border
    sal_Bool            bEnd = false;
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
            aBox.SetLine( &aLine, BOX_LINE_LEFT );

            aLine.SetWidth( nTop );
            aLine.SetColor( ColorTop );
            aBox.SetLine( &aLine, BOX_LINE_TOP );

            aLine.SetWidth( nRight );
            aLine.SetColor( ColorRight );
            aBox.SetLine( &aLine, BOX_LINE_RIGHT );

            aLine.SetWidth( nBottom );
            aLine.SetColor( ColorBottom );
            aBox.SetLine( &aLine, BOX_LINE_BOTTOM );

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

    // ATTENTION: Code up to here works more or less ... from here I've had enough ! (GT)

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
        sal_Bool        bSwapCol = false;
        sal_Bool        bSetItem = sal_True;
        switch ( aRaster.pData[ nRasterIndex ].Value )
        {
        case raNone:        nFact = 0xffff; bSwapCol = sal_True; bSetItem = (nBColor > 0); break;
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

    // Cell attributes (Protected, hidden...)
    nStart = 0;
    nEnd = 0;
    for (i=0; i<aFlag.Count; i++)
    {
        nEnd = static_cast<SCROW>(aFlag.pData[i].Row);
        if ((nStart <= nEnd) && (aFlag.pData[i].Value != 0))
        {
            sal_Bool bProtect  = ((aFlag.pData[i].Value & paProtect) == paProtect);
            sal_Bool bHFormula = ((aFlag.pData[i].Value & paHideFormula) == paHideFormula);
            sal_Bool bHCell    = ((aFlag.pData[i].Value & paHideAll) == paHideAll);
            sal_Bool bHPrint   = ((aFlag.pData[i].Value & paHidePrint) == paHidePrint);
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
            if (pPattern != NULL)
            {
                ScStyleSheet* pStyle = (ScStyleSheet*) pStylePool->Find(
                                    SC10TOSTRING( pPattern->Name ), SFX_STYLE_FAMILY_PARA);

                if (pStyle != NULL)
                    pDoc->ApplyStyleAreaTab(Col, nStart, Col, nEnd, Tab, *pStyle);
            }
        }
        nStart = nEnd + 1;
    }
  }
}


void Sc10Import::LoadAttr(Sc10ColAttr& rAttr)
{
    // rAttr is not reused, otherwise we'd have to delete [] rAttr.pData;
    rStream >> rAttr.Count;
    if (rAttr.Count)
    {
        rAttr.pData = new (::std::nothrow) Sc10ColData[rAttr.Count];
        if (rAttr.pData != NULL)
        {
            for (sal_uInt16 i = 0; i < rAttr.Count; i++)
            {
                rStream >> rAttr.pData[i].Row;
                rStream >> rAttr.pData[i].Value;
            }
            nError = rStream.GetError();
        }
        else
        {
            nError = errOutOfMemory;
            rAttr.Count = 0;
        }
    }
}


void Sc10Import::ChangeFormat(sal_uInt16 nFormat, sal_uInt16 nInfo, sal_uLong& nKey)
{
  // Achtung: Die Formate werden nur auf die StarCalc 3.0 internen Formate gemappt
  //          Korrekterweise muessten zum Teil neue Formate erzeugt werden (sollte Stephan sich ansehen)
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
     // Achtung kein Aequivalent
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
  rStream >> ID;
  if (rStream.IsEof()) return;
  if (ID == ObjectID)
  {
    sal_uInt16 nAnz;
    rStream >> nAnz;
    sal_Char Reserved[32];
    rStream.Read(Reserved, sizeof(Reserved));
    nError = rStream.GetError();
    if ((nAnz > 0) && (nError == 0))
    {
      sal_uInt8 ObjectType;
      Sc10GraphHeader GraphHeader;
      sal_Bool IsOleObject = false; // Achtung dies ist nur ein Notnagel
      for (sal_uInt16 i = 0; (i < nAnz) && (nError == 0) && !rStream.IsEof() && !IsOleObject; i++)
      {
        rStream >> ObjectType;
        lcl_ReadGraphHeader(rStream, GraphHeader);

        double nPPTX = ScGlobal::nScreenPPTX;
        double nPPTY = ScGlobal::nScreenPPTY;

        long nStartX = 0;
        for (SCsCOL nX=0; nX<GraphHeader.CarretX; nX++)
            nStartX += pDoc->GetColWidth(nX, static_cast<SCTAB>(GraphHeader.CarretZ));
        nStartX = (long) ( nStartX * HMM_PER_TWIPS );
        nStartX += (long) ( GraphHeader.x / nPPTX * HMM_PER_TWIPS );
        long nSizeX = (long) ( GraphHeader.w / nPPTX * HMM_PER_TWIPS );
        long nStartY = pDoc->GetRowHeight( 0,
                static_cast<SCsROW>(GraphHeader.CarretY) - 1,
                static_cast<SCTAB>(GraphHeader.CarretZ));
        nStartY = (long) ( nStartY * HMM_PER_TWIPS );
        nStartY += (long) ( GraphHeader.y / nPPTY * HMM_PER_TWIPS );
        long nSizeY = (long) ( GraphHeader.h / nPPTY * HMM_PER_TWIPS );

        switch (ObjectType)
        {
          case otOle :
           // Achtung hier muss sowas wie OleLoadFromStream passieren
           IsOleObject = sal_True;
           break;
          case otImage :
          {
           Sc10ImageHeader ImageHeader;
           lcl_ReadImageHeaer(rStream, ImageHeader);

           // Achtung nun kommen die Daten (Bitmap oder Metafile)
           // Typ = 1 Device Dependend Bitmap DIB
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

                //! altes Metafile verwenden ??
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
          }
          break;
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

//-----------------------------------------------------------------------------------------------

FltError ScFormatFilterPluginImpl::ScImportStarCalc10( SvStream& rStream, ScDocument* pDocument )
{
    rStream.Seek( 0UL );
    Sc10Import  aImport( rStream, pDocument );
    return ( FltError ) aImport.Import();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
