/*************************************************************************
 *
 *  $RCSfile: scflt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-23 06:59:01 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/colritem.hxx>
#include <svx/crsditem.hxx>
#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/eeitem.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/fontitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/postitem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/wghtitem.hxx>
#include <svtools/zforlist.hxx>
#include <stdio.h>
#include <math.h>
#include <string.h>
#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#include <svtools/PasswordHelper.hxx>
#endif

#include "global.hxx"
#include "sc.hrc"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "document.hxx"
#include "collect.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "filter.hxx"
#include "scflt.hxx"
#include "cell.hxx"
#include "scfobj.hxx"
#include "docoptio.hxx"
#include "viewopti.hxx"
#include "fltprgrs.hxx"
#include "globstr.hrc"


using namespace com::sun::star;

#define DEFCHARSET          RTL_TEXTENCODING_MS_1252

#define SC10TOSTRING(p)     String(p,DEFCHARSET)


extern double ReadLongDouble( SvStream& rStr ); // -> exctools.cxx



void lcl_ReadDateTime(SvStream& rStream, Sc10DateTime& rDateTime)
{
    rStream >> rDateTime.Year;
    rStream >> rDateTime.Month;
    rStream >> rDateTime.Day;
    rStream >> rDateTime.Hour;
    rStream >> rDateTime.Min;
    rStream >> rDateTime.Sec;
}


void lcl_ReadFileHeader(SvStream& rStream, Sc10FileHeader& rFileHeader)
{
    rStream.Read(&rFileHeader.CopyRight, sizeof(rFileHeader.CopyRight));
    rStream >> rFileHeader.Version;
    rStream.Read(&rFileHeader.Reserved, sizeof(rFileHeader.Reserved));
}


void lcl_ReadFileInfo(SvStream& rStream, Sc10FileInfo& rFileInfo)
{
    rStream.Read(&rFileInfo.Title, sizeof(rFileInfo.Title));
    rStream.Read(&rFileInfo.Thema, sizeof(rFileInfo.Thema));
    rStream.Read(&rFileInfo.Keys, sizeof(rFileInfo.Keys));
    rStream.Read(&rFileInfo.Note, sizeof(rFileInfo.Note));
    rStream.Read(&rFileInfo.InfoLabel0, sizeof(rFileInfo.InfoLabel0));
    rStream.Read(&rFileInfo.InfoLabel1, sizeof(rFileInfo.InfoLabel1));
    rStream.Read(&rFileInfo.InfoLabel2, sizeof(rFileInfo.InfoLabel2));
    rStream.Read(&rFileInfo.InfoLabel3, sizeof(rFileInfo.InfoLabel3));
    rStream.Read(&rFileInfo.Info0, sizeof(rFileInfo.Info0));
    rStream.Read(&rFileInfo.Info1, sizeof(rFileInfo.Info1));
    rStream.Read(&rFileInfo.Info2, sizeof(rFileInfo.Info2));
    rStream.Read(&rFileInfo.Info3, sizeof(rFileInfo.Info3));
    rStream.Read(&rFileInfo.CreateAuthor, sizeof(rFileInfo.CreateAuthor));
    rStream.Read(&rFileInfo.ChangeAuthor, sizeof(rFileInfo.ChangeAuthor));
    rStream.Read(&rFileInfo.PrintAuthor, sizeof(rFileInfo.PrintAuthor));
    lcl_ReadDateTime(rStream, rFileInfo.CreateDate);
    lcl_ReadDateTime(rStream, rFileInfo.ChangeDate);
    lcl_ReadDateTime(rStream, rFileInfo.PrintDate);
    rStream >> rFileInfo.PageCount;
    rStream >> rFileInfo.ChartCount;
    rStream >> rFileInfo.PictureCount;
    rStream >> rFileInfo.GraphCount;
    rStream >> rFileInfo.OleCount;
    rStream >> rFileInfo.NoteCount;
    rStream >> rFileInfo.TextCellCount;
    rStream >> rFileInfo.ValueCellCount;
    rStream >> rFileInfo.FormulaCellCount;
    rStream >> rFileInfo.CellCount;
    rStream.Read(rFileInfo.Reserved, sizeof(rFileInfo.Reserved));
}


void lcl_ReadEditStateInfo(SvStream& rStream, Sc10EditStateInfo& rInfo)
{
    rStream >> rInfo.CarretX;
    rStream >> rInfo.CarretY;
    rStream >> rInfo.CarretZ;

    rStream >> rInfo.DeltaX;
    rStream >> rInfo.DeltaY;
    rStream >> rInfo.DeltaZ;

    rStream >> rInfo.DataBaseMode;
    rStream.Read(rInfo.Reserved, sizeof(rInfo.Reserved));
}


void lcl_ReadTabProtect(SvStream& rStream, Sc10TableProtect& rProtect)
{
    rStream.Read(&rProtect.PassWord, sizeof(rProtect.PassWord));
    rStream >> rProtect.Flags;
    rStream >> rProtect.Protect;
}


void lcl_ReadSheetProtect(SvStream& rStream, Sc10SheetProtect& rProtect)
{
    rStream.Read(&rProtect.PassWord, sizeof(rProtect.PassWord));
    rStream >> rProtect.Flags;
    rStream >> rProtect.Protect;
}


void lcl_ReadRGB(SvStream& rStream, Sc10Color& rColor)
{
    rStream >> rColor.Dummy;
    rStream >> rColor.Blue;
    rStream >> rColor.Green;
    rStream >> rColor.Red;
}


void lcl_ReadPalette(SvStream& rStream, Sc10Color* pPalette)
{
    for (USHORT i = 0; i < 16; i++)
        lcl_ReadRGB(rStream, pPalette[i]);
}


void lcl_ReadValueFormat(SvStream& rStream, Sc10ValueFormat& rFormat)
{
    rStream >> rFormat.Format;
    rStream >> rFormat.Info;
}


void lcl_ReadLogFont(SvStream& rStream, Sc10LogFont& rFont)
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
    rStream.Read(&rFont.lfFaceName, sizeof(rFont.lfFaceName));
}


void lcl_ReadBlockRect(SvStream& rStream, Sc10BlockRect& rBlock)
{
    rStream >> rBlock.x1;
    rStream >> rBlock.y1;
    rStream >> rBlock.x2;
    rStream >> rBlock.y2;
}


void lcl_ReadHeadFootLine(SvStream& rStream, Sc10HeadFootLine& rLine)
{
    rStream.Read(&rLine.Title, sizeof(rLine.Title));
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


void lcl_ReadPageFormat(SvStream& rStream, Sc10PageFormat& rFormat)
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
    rStream.Read(&rFormat.PrintAreaName, sizeof(rFormat.PrintAreaName));
    lcl_ReadBlockRect(rStream, rFormat.PrintArea);
    rStream.Read(&rFormat.PrnZoom, sizeof(rFormat.PrnZoom));
    rStream >> rFormat.FirstPageNo;
    rStream >> rFormat.RowRepeatStart;
    rStream >> rFormat.RowRepeatEnd;
    rStream >> rFormat.ColRepeatStart;
    rStream >> rFormat.ColRepeatEnd;
    rStream.Read(&rFormat.Reserved, sizeof(rFormat.Reserved));
}


void lcl_ReadGraphHeader(SvStream& rStream, Sc10GraphHeader& rHeader)
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


void lcl_ReadImageHeaer(SvStream& rStream, Sc10ImageHeader& rHeader)
{
    rStream.Read(&rHeader.FileName, sizeof(rHeader.FileName));
    rStream >> rHeader.Typ;
    rStream >> rHeader.Linked;
    rStream >> rHeader.x1;
    rStream >> rHeader.y1;
    rStream >> rHeader.x2;
    rStream >> rHeader.y2;
    rStream >> rHeader.Size;
}


void lcl_ReadChartHeader(SvStream& rStream, Sc10ChartHeader& rHeader)
{
    rStream >> rHeader.MM;
    rStream >> rHeader.xExt;
    rStream >> rHeader.yExt;
    rStream >> rHeader.Size;
}


void lcl_ReadChartSheetData(SvStream& rStream, Sc10ChartSheetData& rSheetData)
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


void lcl_ReadChartTypeData(SvStream& rStream, Sc10ChartTypeData& rTypeData)
{
    rStream >> rTypeData.NumSets;
    rStream >> rTypeData.NumPoints;
    rStream >> rTypeData.DrawMode;
    rStream >> rTypeData.GraphType;
    rStream >> rTypeData.GraphStyle;
    rStream.Read(&rTypeData.GraphTitle, sizeof(rTypeData.GraphTitle));
    rStream.Read(&rTypeData.BottomTitle, sizeof(rTypeData.BottomTitle));
    USHORT i;
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
        rStream.Read(&rTypeData.LegendText[i], sizeof(Sc10ChartText));
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
        rStream.Read(&rTypeData.LabelText[i], sizeof(Sc10ChartText));
    rStream.Read(&rTypeData.LeftTitle, sizeof(rTypeData.LeftTitle));
    rStream.Read(&rTypeData.Reserved, sizeof(rTypeData.Reserved));
    //rStream.Read(&rTypeData, sizeof(rTypeData));
}

//!     funktioniert das auf allen Systemen ???

double lcl_PascalToDouble(sal_Char* tp6)
{
    struct
    {
        UINT8           be  ;     /* biased exponent           */
        unsigned int    v1  ;     /* lower 16 bits of mantissa */
        unsigned int    v2  ;     /* next  16 bits of mantissa */
        unsigned int    v3:7;     /* upper  7 bits of mantissa */
        unsigned int    s :1;     /* sign bit                  */
    } real;

    memcpy (&real, tp6, 6);
    if (real.be == 0)
          return 0.0;
    return (((((128 +real.v3) * 65536.0) + real.v2) * 65536.0 + real.v1) *
          ldexp ((real.s? -1.0: 1.0), real.be - (129+39)));
}


void lcl_ChangeColor( USHORT nIndex, Color& rColor )
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

String lcl_MakeOldPageStyleFormatName( USHORT i )
{
    String  aName = ScGlobal::GetRscString( STR_PAGESTYLE );
    aName.AppendAscii( " " );
    aName += String::CreateFromInt32( i + 1 );

    return aName;
}

//--------------------------------------------
// Font
//--------------------------------------------


Sc10FontData::Sc10FontData(SvStream& rStream)
{
    rStream >> Height;
    rStream >> CharSet;
    rStream >> PitchAndFamily;
    USHORT nLen;
    rStream >> nLen;
    rStream.Read(FaceName, nLen);
}


Sc10FontCollection::Sc10FontCollection(SvStream& rStream) :
    Collection (4, 4),
    nError     (0)
{
  USHORT ID;
  rStream >> ID;
  if (ID == FontID)
  {
    USHORT nAnz;
    rStream >> nAnz;
    for (USHORT i=0; (i < nAnz) && (nError == 0); i++)
    {
      Insert(new Sc10FontData(rStream));
      nError = rStream.GetError();
    }
  }
  else
  {
    DBG_ERROR( "FontID" );
    nError = errUnknownID;
  }
}


//--------------------------------------------
// Benannte-Bereiche
//--------------------------------------------


Sc10NameData::Sc10NameData(SvStream& rStream)
{
    BYTE nLen;
    rStream >> nLen;
    rStream.Read(Name, sizeof(Name) - 1);
    Name[nLen] = 0;

    rStream >> nLen;
    rStream.Read(Reference, sizeof(Reference) - 1);
    Reference[nLen] = 0;
    rStream.Read(Reserved, sizeof(Reserved));
}


Sc10NameCollection::Sc10NameCollection(SvStream& rStream) :
    Collection (4, 4),
    nError     (0)
{
  USHORT ID;
  rStream >> ID;
  if (ID == NameID)
  {
    USHORT nAnz;
    rStream >> nAnz;
    for (USHORT i=0; (i < nAnz) && (nError == 0); i++)
    {
      Insert(new Sc10NameData(rStream));
      nError = rStream.GetError();
    }
  }
  else
  {
    DBG_ERROR( "NameID" );
    nError = errUnknownID;
  }
}

//--------------------------------------------
// Vorlagen
//--------------------------------------------


Sc10PatternData::Sc10PatternData(SvStream& rStream)
{
  rStream.Read(Name, sizeof(Name));
  //rStream.Read(&ValueFormat, sizeof(ValueFormat));
  //rStream.Read(&LogFont, sizeof(LogFont));
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
  Collection (4, 4),
  nError     (0)
{
  USHORT ID;
  rStream >> ID;
  if (ID == PatternID)
  {
    USHORT nAnz;
    rStream >> nAnz;
    for (USHORT i=0; (i < nAnz) && (nError == 0); i++)
    {
      Insert(new Sc10PatternData(rStream));
      nError = rStream.GetError();
    }
  }
  else
  {
    DBG_ERROR( "PatternID" );
    nError = errUnknownID;
  }
}


//--------------------------------------------
// Datenbank
//--------------------------------------------


Sc10DataBaseData::Sc10DataBaseData(SvStream& rStream)
{
    //rStream.Read(&DataBaseRec, sizeof(DataBaseRec));
    rStream.Read(&DataBaseRec.Name, sizeof(DataBaseRec.Name));
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
    rStream.Read(&DataBaseRec.QueryString0, sizeof(DataBaseRec.QueryString0));
    DataBaseRec.QueryValue0 = ReadLongDouble(rStream);

    rStream >> DataBaseRec.QueryConnect1;
    rStream >> DataBaseRec.QueryField1;
    rStream >> DataBaseRec.QueryOp1;
    rStream >> DataBaseRec.QueryByString1;
    rStream.Read(&DataBaseRec.QueryString1, sizeof(DataBaseRec.QueryString1));
    DataBaseRec.QueryValue1 = ReadLongDouble(rStream);

    rStream >> DataBaseRec.QueryConnect2;
    rStream >> DataBaseRec.QueryField2;
    rStream >> DataBaseRec.QueryOp2;
    rStream >> DataBaseRec.QueryByString2;
    rStream.Read(&DataBaseRec.QueryString2, sizeof(DataBaseRec.QueryString2));
    DataBaseRec.QueryValue2 = ReadLongDouble(rStream);
}


Sc10DataBaseCollection::Sc10DataBaseCollection(SvStream& rStream) :
  Collection (4, 4),
  nError     (0)
{
  USHORT ID;
  rStream >> ID;
  if (ID == DataBaseID)
  {
    rStream.Read(ActName, sizeof(ActName));
    USHORT nAnz;
    rStream >> nAnz;
    for (USHORT i=0; (i < nAnz) && (nError == 0); i++)
    {
      Insert(new Sc10DataBaseData(rStream));
      nError = rStream.GetError();
    }
  }
  else
  {
    DBG_ERROR( "DataBaseID" );
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


USHORT Sc10PageCollection::Insert( const Sc10PageFormat& rData )
{
    for (USHORT i=0; i<nCount; i++)
        if (At(i)->aPageFormat == rData)
            return i;

    Collection::Insert( new Sc10PageData(rData) );

    return nCount-1;
}


static inline UINT8 GetMixedCol( const UINT8 nB, const UINT8 nF, const UINT16 nFak )
{
    INT32       nT = nB - nF;
                nT *= ( INT32 ) nFak;
                nT /= 0xFFFF;
                nT += nF;
    return ( UINT8 ) nT;
}
static inline Color GetMixedColor( const Color& rFore, const Color& rBack, UINT16 nFact )
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

    for (USHORT i=0; i<nCount; i++)
    {
        Sc10PageFormat* pPage = &At(i)->aPageFormat;

        pPage->Width = (short) ( pPage->Width * ( 72.0 / 72.27 ) + 0.5 );
        pPage->Height = (short) ( pPage->Height * ( 72.0 / 72.27 ) + 0.5 );
        pPage->Top = (short) ( pPage->Top * ( 72.0 / 72.27 ) + 0.5 );
        pPage->Bottom = (short) ( pPage->Bottom * ( 72.0 / 72.27 ) + 0.5 );
        pPage->Left = (short) ( pPage->Left * ( 72.0 / 72.27 ) + 0.5 );
        pPage->Right = (short) ( pPage->Right * ( 72.0 / 72.27 ) + 0.5 );
        pPage->Head = (short) ( pPage->Head * ( 72.0 / 72.27 ) + 0.5 );
        pPage->Foot = (short) ( pPage->Foot * ( 72.0 / 72.27 ) + 0.5 );

        String aName = lcl_MakeOldPageStyleFormatName( i );

        ScStyleSheet* pSheet = (ScStyleSheet*) &pStylePool->Make( aName,
                                    SFX_STYLE_FAMILY_PAGE,
                                    SFXSTYLEBIT_USERDEF | SCSTYLEBIT_STANDARD );
        SfxItemSet* pSet = &pSheet->GetItemSet();

        for (USHORT nHeadFoot=0; nHeadFoot<2; nHeadFoot++)
        {
            Sc10HeadFootLine* pHeadFootLine = nHeadFoot ? &pPage->FootLine : &pPage->HeadLine;

            SfxItemSet aEditAttribs(aEditEngine.GetEmptyItemSet());
            FontFamily eFam;
            switch (pPage->HeadLine.LogFont.lfPitchAndFamily & 0xF0)
            {
                case ffDontCare:    eFam = FAMILY_DONTKNOW;     break;
                case ffRoman:       eFam = FAMILY_ROMAN;        break;
                case ffSwiss:       eFam = FAMILY_SWISS;        break;
                case ffModern:      eFam = FAMILY_MODERN;       break;
                case ffScript:      eFam = FAMILY_SCRIPT;       break;
                case ffDecorative:  eFam = FAMILY_DECORATIVE;   break;
            }
            aEditAttribs.Put(   SvxFontItem(
                                    eFam,
                                    SC10TOSTRING( pHeadFootLine->LogFont.lfFaceName ), EMPTY_STRING ),
                                EE_CHAR_FONTINFO );
            aEditAttribs.Put(   SvxFontHeightItem( Abs( pHeadFootLine->LogFont.lfHeight ) ),
                                EE_CHAR_FONTHEIGHT);

            Sc10Color nColor = pHeadFootLine->TextColor;
            Color TextColor( nColor.Red, nColor.Green, nColor.Blue );
            aEditAttribs.Put(SvxColorItem(TextColor), EE_CHAR_COLOR);
            // FontAttr
            if (pHeadFootLine->LogFont.lfWeight != fwNormal)
                aEditAttribs.Put(SvxWeightItem(WEIGHT_BOLD), EE_CHAR_WEIGHT);
            if (pHeadFootLine->LogFont.lfItalic != 0)
                aEditAttribs.Put(SvxPostureItem(ITALIC_NORMAL), EE_CHAR_ITALIC);
            if (pHeadFootLine->LogFont.lfUnderline != 0)
                aEditAttribs.Put(SvxUnderlineItem(UNDERLINE_SINGLE), EE_CHAR_UNDERLINE);
            if (pHeadFootLine->LogFont.lfStrikeOut != 0)
                aEditAttribs.Put(SvxCrossedOutItem(STRIKEOUT_SINGLE), EE_CHAR_STRIKEOUT);
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

            UINT16 nFact;
            BOOL        bSwapCol = FALSE;
            switch (pHeadFootLine->Raster)
            {
               case raNone:     nFact = 0xffff; bSwapCol = TRUE; break;
               case raGray12:   nFact = (0xffff / 100) * 12;    break;
               case raGray25:   nFact = (0xffff / 100) * 25;    break;
               case raGray50:   nFact = (0xffff / 100) * 50;    break;
               case raGray75:   nFact = (0xffff / 100) * 75;    break;
               default: nFact = 0xffff;
            }
            if( bSwapCol )
                aSetItemItemSet.Put( SvxBrushItem( GetMixedColor( aBColor, aRColor, nFact ) ) );
            else
                aSetItemItemSet.Put( SvxBrushItem( GetMixedColor( aRColor, aBColor, nFact ) ) );

            if (pHeadFootLine->Frame != 0)
            {
              USHORT nLeft = 0;
              USHORT nTop = 0;
              USHORT nRight = 0;
              USHORT nBottom = 0;
              USHORT fLeft   = (pHeadFootLine->Frame & 0x000F);
              USHORT fTop    = (pHeadFootLine->Frame & 0x00F0) / 0x0010;
              USHORT fRight  = (pHeadFootLine->Frame & 0x0F00) / 0x0100;
              USHORT fBottom = (pHeadFootLine->Frame & 0xF000) / 0x1000;
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
              USHORT cLeft   = (pHeadFootLine->FrameColor & 0x000F);
              USHORT cTop    = (pHeadFootLine->FrameColor & 0x00F0) >> 4;
              USHORT cRight  = (pHeadFootLine->FrameColor & 0x0F00) >> 8;
              USHORT cBottom = (pHeadFootLine->FrameColor & 0xF000) >> 12;
              lcl_ChangeColor(cLeft, ColorLeft);
              lcl_ChangeColor(cTop, ColorTop);
              lcl_ChangeColor(cRight, ColorRight);
              lcl_ChangeColor(cBottom, ColorBottom);
              SvxBorderLine aLine;
              SvxBoxItem aBox;
              aLine.SetOutWidth(nLeft);
              aLine.SetColor(ColorLeft);
              aBox.SetLine(&aLine, BOX_LINE_LEFT);
              aLine.SetOutWidth(nTop);
              aLine.SetColor(ColorTop);
              aBox.SetLine(&aLine, BOX_LINE_TOP);
              aLine.SetOutWidth(nRight);
              aLine.SetColor(ColorRight);
              aBox.SetLine(&aLine, BOX_LINE_RIGHT);
              aLine.SetOutWidth(nBottom);
              aLine.SetColor(ColorBottom);
              aBox.SetLine(&aLine, BOX_LINE_BOTTOM);

              aSetItemItemSet.Put(aBox);
            }

            pSet->Put( SvxULSpaceItem( 0, 0, ATTR_ULSPACE ) );

            if (nHeadFoot==0)
                aSetItemItemSet.Put( SvxSizeItem( ATTR_PAGE_SIZE, Size( 0, pPage->Top - pPage->Head ) ) );
            else
                aSetItemItemSet.Put( SvxSizeItem( ATTR_PAGE_SIZE, Size( 0, pPage->Bottom - pPage->Foot ) ) );

            aSetItemItemSet.Put(SfxBoolItem( ATTR_PAGE_ON, TRUE ));
            aSetItemItemSet.Put(SfxBoolItem( ATTR_PAGE_DYNAMIC, FALSE ));
            aSetItemItemSet.Put(SfxBoolItem( ATTR_PAGE_SHARED, TRUE ));

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
                pRepeatCol = new ScRange( pPage->ColRepeatStart, 0, 0 );
            if ( pPage->RowRepeatStart >= 0 )
                pRepeatRow = new ScRange( 0, pPage->RowRepeatStart, 0 );


            if ( pRepeatRow || pRepeatCol )
            {
                //
                // an allen Tabellen setzen
                //
                const USHORT nLast = pDoc->GetMaxTableNumber();
                for ( USHORT i=0; i < nLast; i++ )
                {
                    pDoc->SetRepeatColRange( i, pRepeatCol );
                    pDoc->SetRepeatRowRange( i, pRepeatRow );
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
                                  (UINT16)( lcl_PascalToDouble( pPage->PrnZoom ) * 100 ) ) );
        pSet->Put( SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, 1 ) );

        pSet->Put( SvxSizeItem( ATTR_PAGE_SIZE, Size( pPage->Width, pPage->Height ) ) );
    }

    delete pEmptyObject;
}


DataObject* Sc10PageData::Clone() const
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

    DBG_ASSERT( pPrgrsBar == NULL,
        "*Sc10Import::Sc10Import(): Progressbar lebt noch!?" );
}


ULONG Sc10Import::Import()
{
    pPrgrsBar = new FilterProgressBar( rStream );

    ScDocOptions aOpt = pDoc->GetDocOptions();
    aOpt.SetDate( 1, 1, 1900 );
    aOpt.SetYear2000( 18 + 1901 );      // ab SO51 src513e vierstellig
    pDoc->SetDocOptions( aOpt );
    pDoc->GetFormatTable()->ChangeNullDate( 1, 1, 1900 );

    LoadFileHeader();                           pPrgrsBar->Progress();
    if (!nError) { LoadFileInfo();          pPrgrsBar->Progress(); }
    if (!nError) { LoadEditStateInfo();     pPrgrsBar->Progress(); }
    if (!nError) { LoadProtect();               pPrgrsBar->Progress(); }
    if (!nError) { LoadViewColRowBar();     pPrgrsBar->Progress(); }
    if (!nError) { LoadScrZoom();               pPrgrsBar->Progress(); }
    if (!nError) { LoadPalette();               pPrgrsBar->Progress(); }
    if (!nError) { LoadFontCollection();        pPrgrsBar->Progress(); }
    if (!nError) { LoadNameCollection();        pPrgrsBar->Progress(); }
    if (!nError) { LoadPatternCollection(); pPrgrsBar->Progress(); }
    if (!nError) { LoadDataBaseCollection();    pPrgrsBar->Progress(); }
    if (!nError) { LoadTables();                pPrgrsBar->Progress(); }
    if (!nError) { LoadObjects();               pPrgrsBar->Progress(); }
    if (!nError) { ImportNameCollection();  pPrgrsBar->Progress(); }
    pDoc->SetViewOptions( aSc30ViewOpt );

    if (nError)
    {
        sal_Char s[8];
        sprintf(s,"%d",(short)nError);
        DBG_ERROR(s);
    }

    delete pPrgrsBar;
#ifdef DBG_UTIL
    pPrgrsBar = NULL;
#endif

    return nError;
}


void Sc10Import::LoadFileHeader()
{
    Sc10FileHeader FileHeader;
    //rStream.Read(&FileHeader, sizeof(FileHeader));
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
    //lcl_ReadFileInfo(rStream, FileInfo);

    nError = rStream.GetError();
    // Achtung Info Uebertragen
}



void Sc10Import::LoadEditStateInfo()
{
    Sc10EditStateInfo EditStateInfo;
    rStream.Read(&EditStateInfo, sizeof(EditStateInfo));
    //lcl_ReadEditStateInfo(rStream, EditStateInfo);

    nError = rStream.GetError();
    nShowTab = EditStateInfo.DeltaZ;
    // Achtung Cursorposition und Offset der Tabelle Uebertragen (soll man das machen??)
}


void Sc10Import::LoadProtect()
{
    //rStream.Read(&SheetProtect, sizeof(SheetProtect));
    lcl_ReadSheetProtect(rStream, SheetProtect);
    nError = rStream.GetError();
    uno::Sequence<sal_Int8> aPass;
    SvPasswordHelper::GetHashPassword(aPass, SC10TOSTRING( SheetProtect.PassWord ));
    pDoc->SetDocProtection( SheetProtect.Protect,  aPass);
}


void Sc10Import::LoadViewColRowBar()
{
    BYTE ViewColRowBar;
    rStream >> ViewColRowBar;
    nError = rStream.GetError();
    aSc30ViewOpt.SetOption( VOPT_HEADER, (BOOL)ViewColRowBar );
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
    //rStream.Read(TextPalette, sizeof(TextPalette));
    //rStream.Read(BackPalette, sizeof(BackPalette));
    //rStream.Read(RasterPalette, sizeof(RasterPalette));
    //rStream.Read(FramePalette, sizeof(FramePalette));
    lcl_ReadPalette(rStream, TextPalette);
    lcl_ReadPalette(rStream, BackPalette);
    lcl_ReadPalette(rStream, RasterPalette);
    lcl_ReadPalette(rStream, FramePalette);

    nError = rStream.GetError();
}


void Sc10Import::LoadFontCollection()
{
    pFontCollection = new Sc10FontCollection(rStream);
}


void Sc10Import::LoadNameCollection()
{
    pNameCollection = new Sc10NameCollection(rStream);
}


void Sc10Import::ImportNameCollection()
{
    ScRangeName*        pRN = pDoc->GetRangeName();

    for (USHORT i = 0; i < pNameCollection->GetCount(); i++)
    {
        Sc10NameData*   pName = pNameCollection->At( i );
        pRN->Insert( new ScRangeData(   pDoc,
                                        SC10TOSTRING( pName->Name ),
                                        SC10TOSTRING( pName->Reference ) ) );
    }
}


void Sc10Import::LoadPatternCollection()
{
    pPatternCollection = new Sc10PatternCollection( rStream );
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    for( USHORT i = 0 ; i < pPatternCollection->GetCount() ; i++ )
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
                FontFamily eFam;
                switch( pPattern->LogFont.lfPitchAndFamily & 0xF0 )
                {
                    case ffDontCare   : eFam = FAMILY_DONTKNOW;     break;
                    case ffRoman      : eFam = FAMILY_ROMAN;        break;
                    case ffSwiss      : eFam = FAMILY_SWISS;        break;
                    case ffModern     : eFam = FAMILY_MODERN;       break;
                    case ffScript     : eFam = FAMILY_SCRIPT;       break;
                    case ffDecorative : eFam = FAMILY_DECORATIVE;   break;
                }
                rItemSet.Put( SvxFontItem( eFam, SC10TOSTRING( pPattern->LogFont.lfFaceName ), EMPTY_STRING ) );
                rItemSet.Put( SvxFontHeightItem( Abs( pPattern->LogFont.lfHeight ) ) );
                Color TextColor( COL_BLACK );
                lcl_ChangeColor( ( pPattern->nColor & 0x000F ), TextColor );
                rItemSet.Put( SvxColorItem( TextColor ) );
                // FontAttr
                if( pPattern->LogFont.lfWeight != fwNormal )
                    rItemSet.Put( SvxWeightItem( WEIGHT_BOLD ) );
                if( pPattern->LogFont.lfItalic != 0 )
                    rItemSet.Put( SvxPostureItem( ITALIC_NORMAL ) );
                if( pPattern->LogFont.lfUnderline != 0 )
                    rItemSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE ) );
                if( pPattern->LogFont.lfStrikeOut != 0 )
                    rItemSet.Put( SvxCrossedOutItem( STRIKEOUT_SINGLE ) );
            }
            // Ausrichtung
            if( ( pPattern->FormatFlags & pfJustify ) == pfJustify )
            {
                USHORT HorJustify = ( pPattern->Justify & 0x000F );
                USHORT VerJustify = ( pPattern->Justify & 0x00F0 ) >> 4;
                USHORT OJustify   = ( pPattern->Justify & 0x0F00 ) >> 8;
                USHORT EJustify   = ( pPattern->Justify & 0xF000 ) >> 12;
                if( HorJustify != 0 )
                    switch( HorJustify )
                    {
                        case hjLeft:
                            rItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT ) );
                            break;
                        case hjCenter:
                            rItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER ) );
                            break;
                        case hjRight:
                            rItemSet.Put( SvxHorJustifyItem( SVX_HOR_JUSTIFY_RIGHT ) );
                            break;
                    }
                if( VerJustify != 0 )
                    switch( VerJustify )
                    {
                        case vjTop:
                            rItemSet.Put( SvxVerJustifyItem( SVX_VER_JUSTIFY_TOP ) );
                            break;
                        case vjCenter:
                            rItemSet.Put( SvxVerJustifyItem( SVX_VER_JUSTIFY_CENTER ) );
                            break;
                        case vjBottom:
                            rItemSet.Put( SvxVerJustifyItem( SVX_VER_JUSTIFY_BOTTOM ) );
                            break;
                    }

                if( ( OJustify & ojWordBreak ) == ojWordBreak )
                    rItemSet.Put( SfxBoolItem( TRUE ) );
                if( ( OJustify & ojBottomTop ) == ojBottomTop )
                    rItemSet.Put( SvxOrientationItem( SVX_ORIENTATION_BOTTOMTOP ) );
                else if( ( OJustify & ojTopBottom ) == ojTopBottom )
                    rItemSet.Put( SvxOrientationItem( SVX_ORIENTATION_TOPBOTTOM ) );

                INT16 Margin = Max( ( USHORT ) 20, ( USHORT ) ( EJustify * 20 ) );
//              if( ( ( OJustify & ojBottomTop ) == ojBottomTop ) ||
//                  ( ( OJustify & ojBottomTop ) == ojBottomTop ) )
// vielleicht so?
                if( ( ( OJustify & ojBottomTop ) == ojBottomTop ) )
                    rItemSet.Put( SvxMarginItem( 20, Margin, 20, Margin ) );
                else
                    rItemSet.Put( SvxMarginItem( Margin, 20, Margin, 20 ) );
            }

            // Frame
            if( ( pPattern->FormatFlags & pfFrame ) == pfFrame )
            {
                if( pPattern->Frame != 0 )
                {
                    USHORT  nLeft   = 0;
                    USHORT  nTop    = 0;
                    USHORT  nRight  = 0;
                    USHORT  nBottom = 0;
                    USHORT  fLeft   = ( pPattern->Frame & 0x000F );
                    USHORT  fTop    = ( pPattern->Frame & 0x00F0 ) / 0x0010;
                    USHORT  fRight  = ( pPattern->Frame & 0x0F00 ) / 0x0100;
                    USHORT  fBottom = ( pPattern->Frame & 0xF000 ) / 0x1000;

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

                    USHORT  cLeft   = ( pPattern->FrameColor & 0x000F );
                    USHORT  cTop    = ( pPattern->FrameColor & 0x00F0 ) >> 4;
                    USHORT  cRight  = ( pPattern->FrameColor & 0x0F00 ) >> 8;
                    USHORT  cBottom = ( pPattern->FrameColor & 0xF000 ) >> 12;

                    lcl_ChangeColor( cLeft, ColorLeft );
                    lcl_ChangeColor( cTop, ColorTop );
                    lcl_ChangeColor( cRight, ColorRight );
                    lcl_ChangeColor( cBottom, ColorBottom );

                    SvxBorderLine   aLine;
                    SvxBoxItem      aBox;

                    aLine.SetOutWidth( nLeft );
                    aLine.SetColor( ColorLeft );
                    aBox.SetLine( &aLine, BOX_LINE_LEFT );
                    aLine.SetOutWidth( nTop );
                    aLine.SetColor( ColorTop );
                    aBox.SetLine( &aLine, BOX_LINE_TOP );
                    aLine.SetOutWidth( nRight );
                    aLine.SetColor( ColorRight );
                    aBox.SetLine( &aLine, BOX_LINE_RIGHT );
                    aLine.SetOutWidth( nBottom );
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
                    USHORT nBColor = ( pPattern->nColor & 0x00F0 ) >> 4;
                    USHORT nRColor = ( pPattern->nColor & 0x0F00 ) >> 8;
                    Color aBColor( COL_BLACK );

                    lcl_ChangeColor( nBColor, aBColor );

                    if( nBColor == 0 )
                        aBColor.SetColor( COL_WHITE );
                    else if( nBColor == 15 )
                        aBColor.SetColor( COL_BLACK );

                    Color aRColor( COL_BLACK );
                    lcl_ChangeColor( nRColor, aRColor );
                    UINT16 nFact;
                    BOOL        bSwapCol = FALSE;
                    BOOL        bSetItem = TRUE;
                    switch (pPattern->Raster)
                    {
                       case raNone:     nFact = 0xffff; bSwapCol = TRUE; bSetItem = (nBColor > 0); break;
                       case raGray12:   nFact = (0xffff / 100) * 12;    break;
                       case raGray25:   nFact = (0xffff / 100) * 25;    break;
                       case raGray50:   nFact = (0xffff / 100) * 50;    break;
                       case raGray75:   nFact = (0xffff / 100) * 75;    break;
                       default: nFact = 0xffff; bSetItem = (nRColor < 15);
                    }
                    if ( bSetItem )
                    {
                        if( bSwapCol )
                            rItemSet.Put( SvxBrushItem( GetMixedColor( aBColor, aRColor, nFact ) ) );
                        else
                            rItemSet.Put( SvxBrushItem( GetMixedColor( aRColor, aBColor, nFact ) ) );
                    }
                }
            }
            // ZahlenFormate
            if( ( pPattern->ValueFormat.Format != 0 ) &&
                ( ( pPattern->FormatFlags & pfValue ) == pfValue ) )
            {
                ULONG nKey = 0;
                ChangeFormat( pPattern->ValueFormat.Format, pPattern->ValueFormat.Info, nKey );
                rItemSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, ( UINT32 ) nKey ) );
            }

            // Zellattribute (Schutz, Versteckt...)
            if( ( pPattern->Flags != 0 ) &&
                ( ( pPattern->FormatFlags & pfProtection ) == pfProtection ) )
            {
                BOOL bProtect  = ( ( pPattern->Flags & paProtect ) == paProtect );
                BOOL bHFormula = ( ( pPattern->Flags & paHideFormula ) == paHideFormula );
                BOOL bHCell    = ( ( pPattern->Flags & paHideAll ) == paHideAll );
                BOOL bHPrint   = ( ( pPattern->Flags & paHidePrint ) == paHidePrint );
                rItemSet.Put( ScProtectionAttr( bProtect, bHFormula, bHCell, bHPrint ) );
            }
        } // if Style != 0
    } // for (i = 0; i < GetCount()
}


void Sc10Import::LoadDataBaseCollection()
{
    pDataBaseCollection = new Sc10DataBaseCollection(rStream);
    for( USHORT i = 0 ; i < pDataBaseCollection->GetCount() ; i++ )
    {
        Sc10DataBaseData* pOldData = pDataBaseCollection->At(i);
        ScDBData* pNewData = new ScDBData( SC10TOSTRING( pOldData->DataBaseRec.Name ),
                                    ( USHORT ) pOldData->DataBaseRec.Tab,
                                    ( USHORT ) pOldData->DataBaseRec.Block.x1,
                                    ( USHORT ) pOldData->DataBaseRec.Block.y1,
                                    ( USHORT ) pOldData->DataBaseRec.Block.x2,
                                    ( USHORT ) pOldData->DataBaseRec.Block.y2,
                                    TRUE,
                                    ( BOOL) pOldData->DataBaseRec.RowHeader );
        pDoc->GetDBCollection()->Insert( pNewData );
    }
}


void Sc10Import::LoadTables()
{
    Sc10PageCollection aPageCollection;
    USHORT nDisplayMask = 0xFFFF;
    USHORT nDisplayValue = 0;

    INT16 TabCount;
    rStream >> TabCount;
    for (INT16 Tab=0; (Tab < TabCount) && (nError == 0); Tab++)
    {
        Sc10PageFormat   PageFormat;
        INT16            DataBaseIndex;
        Sc10TableProtect TabProtect;
        INT16            TabNo;
        sal_Char             TabName[128];
        USHORT           Display;
        BYTE             Visible;
        USHORT           ID;
        USHORT           DataCount;
        USHORT           DataStart;
        USHORT           DataEnd;
        USHORT           DataValue;
        USHORT           Count;
        USHORT           i;
        USHORT           j;
        String           aStr;  // Universal-Konvertierungs-String


        //rStream.Read(&PageFormat, sizeof(PageFormat));
        lcl_ReadPageFormat(rStream, PageFormat);

        USHORT nAt = aPageCollection.Insert(PageFormat);
        String aPageName = lcl_MakeOldPageStyleFormatName( nAt );

        pPrgrsBar->Progress();

        rStream >> DataBaseIndex;

        //rStream.Read(&TabProtect, sizeof(TabProtect));
        lcl_ReadTabProtect(rStream, TabProtect);
        uno::Sequence<sal_Int8> aPass;
        SvPasswordHelper::GetHashPassword(aPass, SC10TOSTRING( TabProtect.PassWord ));

        pDoc->SetTabProtection( Tab, TabProtect.Protect, aPass);

        rStream >> TabNo;

        BYTE nLen;
        rStream >> nLen;
        rStream.Read(TabName, sizeof(TabName) - 1);
        TabName[nLen] = 0;

        //----------------------------------------------------------
        rStream >> Display;

        if ( Tab == (INT16)nShowTab )
        {
            ScVObjMode  eObjMode = VOBJ_MODE_SHOW;

            aSc30ViewOpt.SetOption( VOPT_FORMULAS,    IS_SET(dfFormula,Display) );
            aSc30ViewOpt.SetOption( VOPT_NULLVALS,    IS_SET(dfZerro,Display) );
            aSc30ViewOpt.SetOption( VOPT_SYNTAX,      IS_SET(dfSyntax,Display) );
            aSc30ViewOpt.SetOption( VOPT_NOTES,       IS_SET(dfNoteMark,Display) );
            aSc30ViewOpt.SetOption( VOPT_VSCROLL,     TRUE );
            aSc30ViewOpt.SetOption( VOPT_HSCROLL,     TRUE );
            aSc30ViewOpt.SetOption( VOPT_TABCONTROLS, TRUE );
            aSc30ViewOpt.SetOption( VOPT_OUTLINER,    TRUE );
            aSc30ViewOpt.SetOption( VOPT_GRID,        IS_SET(dfGrid,Display) );

            // VOPT_HEADER wird in LoadViewColRowBar() gesetzt

            if ( IS_SET(dfObjectAll,Display) )          // Objekte anzeigen
                eObjMode = VOBJ_MODE_SHOW;
            else if ( IS_SET(dfObjectFrame,Display) )   // Objekte als Platzhalter
                eObjMode = VOBJ_MODE_DUMMY;
            else if ( IS_SET(dfObjectNone,Display) )    // Objekte nicht anzeigen
                eObjMode = VOBJ_MODE_HIDE;

            aSc30ViewOpt.SetObjMode( VOBJ_TYPE_OLE,   eObjMode );
            aSc30ViewOpt.SetObjMode( VOBJ_TYPE_CHART, eObjMode );
            aSc30ViewOpt.SetObjMode( VOBJ_TYPE_DRAW,  eObjMode );
        }

    /*  wofuer wird das benoetigt? Da in SC 1.0 die Anzeigeflags pro Tabelle gelten und nicht pro View
        Dieses Flag in die ViewOptions eintragen bei Gelegenheit, Sollte der Stephan Olk machen
        if (Tab == 0)
            nDisplayValue = Display;
        else
        {
            USHORT nDiff = Display ^ nDisplayValue;
            nDisplayMask &= ~nDiff;
        }
    */
        //--------------------------------------------------------------------
        rStream >> Visible;

        nError = rStream.GetError();
        if (nError != 0) return;

        if (TabNo == 0)
            pDoc->RenameTab(TabNo, SC10TOSTRING( TabName ), FALSE);
        else
            pDoc->InsertTab(SC_TAB_APPEND, SC10TOSTRING( TabName ) );

        pDoc->SetPageStyle( Tab, aPageName );

        if (Visible == 0) pDoc->SetVisible(TabNo, FALSE);

        // ColWidth
        rStream >> ID;
        if (ID != ColWidthID)
        {
            DBG_ERROR( "ColWidthID" );
            nError = errUnknownID;
            return;
        }
        rStream >> DataCount;
        DataStart = 0;
        for (i=0; i < DataCount; i++)
        {
            rStream >> DataEnd;
            rStream >> DataValue;
            for (j = DataStart; j <= DataEnd; j++) pDoc->SetColWidth(j, TabNo, DataValue);
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // ColAttr
        rStream >> ID;
        if (ID != ColAttrID)
        {
            DBG_ERROR( "ColAttrID" );
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
                BYTE nFlags = 0;
                if ((DataValue & crfSoftBreak) == crfSoftBreak)
                    nFlags |= CR_PAGEBREAK;
                if ((DataValue & crfHardBreak) == crfHardBreak)
                    nFlags |= CR_MANUALBREAK;
                if ((DataValue & crfHidden) == crfHidden)
                    nFlags |= CR_HIDDEN;
                for (j = DataStart; j <= DataEnd; j++) pDoc->SetColFlags(j, TabNo, nFlags);
            }
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // RowHeight
        rStream >> ID;
        if (ID != RowHeightID)
        {
            DBG_ERROR( "RowHeightID" );
            nError = errUnknownID;
            return;
        }

        rStream >> DataCount;
        DataStart = 0;
        for (i=0; i < DataCount; i++)
        {
            rStream >> DataEnd;
            rStream >> DataValue;
            pDoc->SetRowHeightRange(DataStart, DataEnd, TabNo, DataValue);
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // RowAttr
        rStream >> ID;
        if (ID != RowAttrID)
        {
            DBG_ERROR( "RowAttrID" );
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
                BYTE nFlags = 0;
                if ((DataValue & crfSoftBreak) == crfSoftBreak)
                    nFlags |= CR_PAGEBREAK;
                if ((DataValue & crfHardBreak) == crfHardBreak)
                    nFlags |= CR_MANUALBREAK;
                if ((DataValue & crfHidden) == crfHidden)
                    nFlags |= CR_HIDDEN;
                for (j = DataStart; j <= DataEnd; j++) pDoc->SetRowFlags(j, TabNo, nFlags);
            }
            DataStart = DataEnd + 1;
        }
        pPrgrsBar->Progress();

        // DataTable
        rStream >> ID;
        if (ID != TableID)
        {
            DBG_ERROR( "TableID" );
            nError = errUnknownID;
            return;
        }
        for (USHORT Col = 0; (Col <= MAXCOL) && (nError == 0); Col++)
        {
            rStream >> Count;
            nError = rStream.GetError();
            if ((Count != 0) && (nError == 0))
                LoadCol(Col, TabNo);
        }
        DBG_ASSERT( nError == 0, "Stream" );
    }
    pPrgrsBar->Progress();

    aPageCollection.PutToDoc( pDoc );
}


void Sc10Import::LoadCol(USHORT Col, USHORT Tab)
{
    LoadColAttr(Col, Tab);

    USHORT CellCount;
    BYTE   CellType;
    USHORT Row;
    rStream >> CellCount;
    if (CellCount > MAXROW) nError = errUnknownFormat;
    for (USHORT i = 0; (i < CellCount) && (nError == 0); i++)
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
                    const SfxPoolItem* pValueFormat = pDoc->GetAttr(Col, Row, Tab, ATTR_VALUE_FORMAT);
                    ULONG nFormat = ((SfxUInt32Item*)pValueFormat)->GetValue();
                    double Value = ReadLongDouble(rStream);
                    //rStream.Read(&Value, sizeof(Value));

                    // Achtung hier ist eine Anpassung Notwendig wenn Ihr das Basisdatum aendert
                    // In StarCalc 1.0 entspricht 0 dem 01.01.1900
                    // if ((nFormat >= 30) && (nFormat <= 35))
                    // Value += 0;
                    if ((nFormat >= 40) && (nFormat <= 45))
                        Value /= 86400.0;
                    pDoc->SetValue(Col, Row, Tab, Value);
                    break;
                }
                case ctString :
                {
                    BYTE Len;
                    sal_Char s[256];
                    rStream >> Len;
                    rStream.Read(s, Len);
                    s[Len] = 0;

                    pDoc->SetString( Col, Row, Tab, SC10TOSTRING( s ) );
                    break;
                }
                case ctFormula :
                {
                    double Value = ReadLongDouble(rStream);
                    BYTE Len;
                    sal_Char s[256];
                    //rStream.Read(&Value, sizeof(Value));
                    rStream >> Len;
                    rStream.Read(&s[1], Len);
                    s[0] = '=';
                    s[Len + 1] = 0;
                    ScFormulaCell* pCell = new ScFormulaCell( pDoc, ScAddress( Col, Row, Tab ) );
                    pCell->SetString( SC10TOSTRING( s ) );
                    pDoc->PutCell( Col, Row, Tab, pCell, (BOOL)TRUE );
                    break;
                }
                case ctNote :
                    break;
                default :
                    nError = errUnknownFormat;
                    break;
            }
            USHORT NoteLen;
            rStream >> NoteLen;
            if (NoteLen != 0)
            {
                sal_Char Note[4096];
                rStream.Read(Note, NoteLen);
                Note[NoteLen] = 0;
                pDoc->SetNote(Col, Row, Tab, ScPostIt( SC10TOSTRING( Note )) );
            }
        }
        pPrgrsBar->Progress();
    }
}


void Sc10Import::LoadColAttr(USHORT Col, USHORT Tab)
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
        USHORT nStart;
        USHORT nEnd;
        USHORT i;
        UINT16 nLimit;
        UINT16 nValue;
        Sc10ColData *pColData;

        // Font (Name, Groesse)
        nStart = 0;
        nEnd = 0;
        nLimit = aFont.Count;
        pColData = aFont.pData;
        for( i = 0 ; i < nLimit ; i++, pColData++ )
        {
            //nEnd = aFont.pData[i].Row;
            nEnd = pColData->Row;
            //if ((nStart <= nEnd) && (aFont.pData[i].Value != 0))
            if ((nStart <= nEnd) && (pColData->Value))
            {
                FontFamily eFam;
                //Sc10FontData* pFont = pFontCollection->At(aFont.pData[i].Value);
                Sc10FontData* pFont = pFontCollection->At(pColData->Value);
                switch (pFont->PitchAndFamily & 0xF0)
                {
                    case ffDontCare   : eFam = FAMILY_DONTKNOW;     break;
                    case ffRoman      : eFam = FAMILY_ROMAN;        break;
                    case ffSwiss      : eFam = FAMILY_SWISS;        break;
                    case ffModern     : eFam = FAMILY_MODERN;       break;
                    case ffScript     : eFam = FAMILY_SCRIPT;       break;
                    case ffDecorative : eFam = FAMILY_DECORATIVE;   break;
                }
                ScPatternAttr aPattern(pDoc->GetPool());
                aPattern.GetItemSet().Put(SvxFontItem(eFam, SC10TOSTRING( pFont->FaceName ), EMPTY_STRING));
                aPattern.GetItemSet().Put(SvxFontHeightItem(Abs(pFont->Height)));
                pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aPattern);
            }
            nStart = nEnd + 1;
        }

    // Fontfarbe
    nStart = 0;
    nEnd = 0;
    nLimit = aColor.Count;
    pColData = aColor.pData;
    for( i = 0 ; i < nLimit ; i++, pColData++ )
    {
        //nEnd = aColor.pData[i].Row;
        nEnd = pColData->Row;
        //if ((nStart <= nEnd) && (aColor.pData[i].Value != 0))
        if ((nStart <= nEnd) && (pColData->Value))
        {
            Color TextColor(COL_BLACK);
            lcl_ChangeColor((pColData->Value & 0x000F), TextColor);
            ScPatternAttr aPattern(pDoc->GetPool());
            aPattern.GetItemSet().Put(SvxColorItem(TextColor));
            pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aPattern);
        }
        nStart = nEnd + 1;
    }

    // Fontattribute (Fett, Kursiv...)
    nStart = 0;
    nEnd = 0;
    nLimit = aAttr.Count;
    pColData = aAttr.pData;
    for( i = 0 ; i < nLimit ; i++, pColData++ )
    {
        nEnd = pColData->Row;
        nValue = pColData->Value;
        if ((nStart <= nEnd) && (nValue))
        {
            ScPatternAttr aPattern(pDoc->GetPool());
            if ((nValue & atBold) == atBold)
             aPattern.GetItemSet().Put(SvxWeightItem(WEIGHT_BOLD));
            if ((nValue & atItalic) == atItalic)
             aPattern.GetItemSet().Put(SvxPostureItem(ITALIC_NORMAL));
            if ((nValue & atUnderline) == atUnderline)
             aPattern.GetItemSet().Put(SvxUnderlineItem(UNDERLINE_SINGLE));
            if ((nValue & atStrikeOut) == atStrikeOut)
             aPattern.GetItemSet().Put(SvxCrossedOutItem(STRIKEOUT_SINGLE));
            pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aPattern);
        }
        nStart = nEnd + 1;
    }

    // Zellausrichtung
    nStart = 0;
    nEnd = 0;
    nLimit = aJustify.Count;
    pColData = aJustify.pData;
    for( i = 0 ; i < nLimit ; i++, pColData++ )
    {
        nEnd = pColData->Row;
        nValue = pColData->Value;
        if ((nStart <= nEnd) && (nValue))
        {
            ScPatternAttr aPattern(pDoc->GetPool());
            USHORT HorJustify = (nValue & 0x000F);
            USHORT VerJustify = (nValue & 0x00F0) >> 4;
            USHORT OJustify   = (nValue & 0x0F00) >> 8;
            USHORT EJustify   = (nValue & 0xF000) >> 12;

            switch (HorJustify)
            {
                case hjLeft:
                    aPattern.GetItemSet().Put(SvxHorJustifyItem(SVX_HOR_JUSTIFY_LEFT));
                    break;
                case hjCenter:
                    aPattern.GetItemSet().Put(SvxHorJustifyItem(SVX_HOR_JUSTIFY_CENTER));
                    break;
                case hjRight:
                    aPattern.GetItemSet().Put(SvxHorJustifyItem(SVX_HOR_JUSTIFY_RIGHT));
                    break;
            }

            switch (VerJustify)
            {
                case vjTop:
                    aPattern.GetItemSet().Put(SvxVerJustifyItem(SVX_VER_JUSTIFY_TOP));
                    break;
                case vjCenter:
                    aPattern.GetItemSet().Put(SvxVerJustifyItem(SVX_VER_JUSTIFY_CENTER));
                    break;
                case vjBottom:
                    aPattern.GetItemSet().Put(SvxVerJustifyItem(SVX_VER_JUSTIFY_BOTTOM));
                    break;
            }

            if (OJustify & ojWordBreak)
                aPattern.GetItemSet().Put(SfxBoolItem(TRUE));
            if (OJustify & ojBottomTop)
                aPattern.GetItemSet().Put(SvxOrientationItem(SVX_ORIENTATION_BOTTOMTOP));
            else if (OJustify & ojTopBottom)
                aPattern.GetItemSet().Put(SvxOrientationItem(SVX_ORIENTATION_TOPBOTTOM));

            INT16 Margin = Max((USHORT)20, (USHORT)(EJustify * 20));
            if (((OJustify & ojBottomTop) == ojBottomTop) || ((OJustify & ojBottomTop) == ojBottomTop))
                aPattern.GetItemSet().Put(SvxMarginItem(20, Margin, 20, Margin));
            else
                aPattern.GetItemSet().Put(SvxMarginItem(Margin, 20, Margin, 20));
            pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aPattern);
        }
    nStart = nEnd + 1;
    }
    // Umrandung
    BOOL            bEnd = FALSE;
    USHORT          nColorIndex = 0;
    USHORT          nFrameIndex = 0;

    // Special Fix...
    const UINT32    nHelpMeStart = 100;
    UINT32          nHelpMe = nHelpMeStart;
    USHORT          nColorIndexOld = nColorIndex;
    USHORT          nFrameIndexOld = nColorIndex;

    nEnd = 0;
    nStart = 0;
    while( !bEnd && nHelpMe )
    {
        pColData = &aFrame.pData[ nFrameIndex ];

        USHORT  nValue  = pColData->Value;
        USHORT  nLeft   = 0;
        USHORT  nTop    = 0;
        USHORT  nRight  = 0;
        USHORT  nBottom = 0;
        USHORT  fLeft   = ( nValue & 0x000F );
        USHORT  fTop    = ( nValue & 0x00F0 ) >> 4;
        USHORT  fRight  = ( nValue & 0x0F00 ) >> 8;
        USHORT  fBottom = ( nValue & 0xF000 ) >> 12;

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
        USHORT  nFrmColVal  = aFrameColor.pData[ nColorIndex ].Value;
        USHORT  nFrmColRow  = aFrameColor.pData[ nColorIndex ].Row;
        USHORT  cLeft       = ( nFrmColVal & 0x000F );
        USHORT  cTop        = ( nFrmColVal & 0x00F0 ) >> 4;
        USHORT  cRight      = ( nFrmColVal & 0x0F00 ) >> 8;
        USHORT  cBottom     = ( nFrmColVal & 0xF000 ) >> 12;

        lcl_ChangeColor( cLeft, ColorLeft );
        lcl_ChangeColor( cTop, ColorTop );
        lcl_ChangeColor( cRight, ColorRight );
        lcl_ChangeColor( cBottom, ColorBottom );

        if( pColData->Row < nFrmColRow )
        {
            nEnd = pColData->Row;
            if( nFrameIndex < ( aFrame.Count - 1 ) )
                nFrameIndex++;
        }
        else if( pColData->Row > nFrmColRow )
        {
            nEnd = aFrameColor.pData[ nColorIndex ].Row;
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
            ScPatternAttr   aPattern(pDoc->GetPool());
            SvxBorderLine   aLine;
            SvxBoxItem      aBox;

            aLine.SetOutWidth( nLeft );
            aLine.SetColor( ColorLeft );
            aBox.SetLine( &aLine, BOX_LINE_LEFT );

            aLine.SetOutWidth( nTop );
            aLine.SetColor( ColorTop );
            aBox.SetLine( &aLine, BOX_LINE_TOP );

            aLine.SetOutWidth( nRight );
            aLine.SetColor( ColorRight );
            aBox.SetLine( &aLine, BOX_LINE_RIGHT );

            aLine.SetOutWidth( nBottom );
            aLine.SetColor( ColorBottom );
            aBox.SetLine( &aLine, BOX_LINE_BOTTOM );

            aPattern.GetItemSet().Put( aBox );
            pDoc->ApplyPatternAreaTab( Col, nStart, Col, nEnd, Tab, aPattern );
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

    // ACHTUNG: Code bis hier ueberarbeitet ... jetzt hab' ich keinen Bock mehr! (GT)

    // Hintergrund (Farbe, Raster)
    USHORT      nRasterIndex = 0;
    bEnd        = FALSE;
    nColorIndex = 0;
    nEnd        = 0;
    nStart      = 0;

    // Special Fix...
    nHelpMe     = nHelpMeStart;
    USHORT      nRasterIndexOld = nRasterIndex;

    while( !bEnd && nHelpMe )
    {
        USHORT  nBColor = ( aColor.pData[ nColorIndex ].Value & 0x00F0 ) >> 4;
        USHORT  nRColor = ( aColor.pData[ nColorIndex ].Value & 0x0F00 ) >> 8;
        Color   aBColor( COL_BLACK );

        lcl_ChangeColor( nBColor, aBColor );

        if( nBColor == 0 )
            aBColor.SetColor( COL_WHITE );
        else if( nBColor == 15 )
            aBColor.SetColor( COL_BLACK );

        Color   aRColor( COL_BLACK );

        lcl_ChangeColor( nRColor, aRColor );

        ScPatternAttr aPattern( pDoc->GetPool() );

        UINT16 nFact;
        BOOL        bSwapCol = FALSE;
        BOOL        bSetItem = TRUE;
        switch ( aRaster.pData[ nRasterIndex ].Value )
        {
        case raNone:        nFact = 0xffff; bSwapCol = TRUE; bSetItem = (nBColor > 0); break;
        case raGray12:  nFact = (0xffff / 100) * 12;    break;
        case raGray25:  nFact = (0xffff / 100) * 25;    break;
        case raGray50:  nFact = (0xffff / 100) * 50;    break;
        case raGray75:  nFact = (0xffff / 100) * 75;    break;
        default:    nFact = 0xffff; bSetItem = (nRColor < 15);
        }
        if ( bSetItem )
        {
            if( bSwapCol )
                aPattern.GetItemSet().Put( SvxBrushItem( GetMixedColor( aBColor, aRColor, nFact ) ) );
            else
                aPattern.GetItemSet().Put( SvxBrushItem( GetMixedColor( aRColor, aBColor, nFact ) ) );
        }
        if( aRaster.pData[ nRasterIndex ].Row < aColor.pData[ nColorIndex ].Row )
        {
            nEnd = aRaster.pData[ nRasterIndex ].Row;
            if( nRasterIndex < ( aRaster.Count - 1 ) )
                nRasterIndex++;
        }
        else if( aRaster.pData[ nRasterIndex ].Row > aColor.pData[ nColorIndex ].Row )
        {
            nEnd = aColor.pData[ nColorIndex ].Row;
            if( nColorIndex < ( aColor.Count - 1 ) )
                nColorIndex++;
        }
        else
        {
            nEnd = aColor.pData[ nColorIndex ].Row;
            if( nRasterIndex < ( aRaster.Count - 1 ) )
                nRasterIndex++;
            if( nColorIndex < ( aColor.Count - 1 ) )
                nColorIndex++;
        }
        if( nStart <= nEnd )
            pDoc->ApplyPatternAreaTab( Col, nStart, Col, nEnd, Tab, aPattern );

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

    // Zahlenformate
    nStart = 0;
    nEnd = 0;
    nLimit = aValue.Count;
    pColData = aValue.pData;
    for (i=0; i<nLimit; i++, pColData++)
    {
        nEnd = pColData->Row;
        nValue = pColData->Value;
        if ((nStart <= nEnd) && (nValue))
        {
            ULONG  nKey    = 0;
            USHORT nFormat = (nValue & 0x00FF);
            USHORT nInfo   = (nValue & 0xFF00) >> 8;
            ChangeFormat(nFormat, nInfo, nKey);
            ScPatternAttr aPattern(pDoc->GetPool());
            aPattern.GetItemSet().Put(SfxUInt32Item(ATTR_VALUE_FORMAT, (UINT32)nKey));
            pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aPattern);
        }
        nStart = nEnd + 1;
    }

    // Zellattribute (Schutz, Versteckt...)
    nStart = 0;
    nEnd = 0;
    for (i=0; i<aFlag.Count; i++)
    {
        nEnd = aFlag.pData[i].Row;
        if ((nStart <= nEnd) && (aFlag.pData[i].Value != 0))
        {
            BOOL bProtect  = ((aFlag.pData[i].Value & paProtect) == paProtect);
            BOOL bHFormula = ((aFlag.pData[i].Value & paHideFormula) == paHideFormula);
            BOOL bHCell    = ((aFlag.pData[i].Value & paHideAll) == paHideAll);
            BOOL bHPrint   = ((aFlag.pData[i].Value & paHidePrint) == paHidePrint);
            ScPatternAttr aPattern(pDoc->GetPool());
            aPattern.GetItemSet().Put(ScProtectionAttr(bProtect, bHFormula, bHCell, bHPrint));
            pDoc->ApplyPatternAreaTab(Col, nStart, Col, nEnd, Tab, aPattern);
        }
        nStart = nEnd + 1;
    }

    // ZellVorlagen
    nStart = 0;
    nEnd = 0;
    USHORT Row = 10;
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    for (i=0; i<aPattern.Count; i++)
    {
        nEnd = aPattern.pData[i].Row;
        if ((nStart <= nEnd) && (aPattern.pData[i].Value != 0))
        {
            USHORT nPatternIndex = (aPattern.pData[i].Value & 0x00FF) - 1;
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

  delete[] aFont.pData;
  delete[] aAttr.pData;
  delete[] aJustify.pData;
  delete[] aFrame.pData;
  delete[] aRaster.pData;
  delete[] aValue.pData;
  delete[] aColor.pData;
  delete[] aFrameColor.pData;
  delete[] aFlag.pData;
  delete[] aPattern.pData;
}


void Sc10Import::LoadAttr(Sc10ColAttr& rAttr)
{
  rStream >> rAttr.Count;
  rAttr.pData = new Sc10ColData[rAttr.Count];
  if (rAttr.pData != NULL)
  {
    for (USHORT i = 0; i < rAttr.Count; i++)
    {
      rStream >> rAttr.pData[i].Row;
      rStream >> rAttr.pData[i].Value;
    }
    //rStream.Read(rAttr.pData, rAttr.Count * sizeof(Sc10ColData));
    nError = rStream.GetError();
  }
  else
    nError = errOutOfMemory;
}


void Sc10Import::ChangeFormat(USHORT nFormat, USHORT nInfo, ULONG& nKey)
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
  }
}


void Sc10Import::LoadObjects()
{
  USHORT ID;
  rStream >> ID;
  if (rStream.IsEof()) return;
  if (ID == ObjectID)
  {
#ifdef SC10_SHOW_OBJECTS
    // Achtung nur zu Debugzwecken
    //-----------------------------------
    pDoc->InsertTab(SC_TAB_APPEND, "GraphObjects");
    USHORT nCol = 0;
    USHORT nRow = 0;
    USHORT nTab = 0;
    pDoc->GetTable("GraphObjects", nTab);
    pDoc->SetString(nCol++, nRow, nTab, "ObjectTyp");
    pDoc->SetString(nCol++, nRow, nTab, "Col");
    pDoc->SetString(nCol++, nRow, nTab, "Row");
    pDoc->SetString(nCol++, nRow, nTab, "Tab");
    pDoc->SetString(nCol++, nRow, nTab, "X");
    pDoc->SetString(nCol++, nRow, nTab, "Y");
    pDoc->SetString(nCol++, nRow, nTab, "W");
    pDoc->SetString(nCol++, nRow, nTab, "H");
    //-----------------------------------
#endif

    USHORT nAnz;
    rStream >> nAnz;
    sal_Char Reserved[32];
    rStream.Read(Reserved, sizeof(Reserved));
    nError = rStream.GetError();
    if ((nAnz > 0) && (nError == 0))
    {
      BYTE ObjectType;
      Sc10GraphHeader GraphHeader;
      BOOL IsOleObject = FALSE; // Achtung dies ist nur ein Notnagel
      for (USHORT i = 0; (i < nAnz) && (nError == 0) && !rStream.IsEof() && !IsOleObject; i++)
      {
        rStream >> ObjectType;
        //rStream.Read(&GraphHeader, sizeof(GraphHeader));
        lcl_ReadGraphHeader(rStream, GraphHeader);

        double nPPTX = ScGlobal::nScreenPPTX;
        double nPPTY = ScGlobal::nScreenPPTY;

        long nStartX = 0;
        for (short nX=0; nX<GraphHeader.CarretX; nX++)
            nStartX += pDoc->GetColWidth(nX, GraphHeader.CarretZ);
        nStartX = (long) ( nStartX * HMM_PER_TWIPS );
        nStartX += (long) ( GraphHeader.x / nPPTX * HMM_PER_TWIPS );
        long nSizeX = (long) ( GraphHeader.w / nPPTX * HMM_PER_TWIPS );
        long nStartY = 0;
        for (short nY=0; nY<GraphHeader.CarretY; nY++)
            nStartY += pDoc->FastGetRowHeight(nY, GraphHeader.CarretZ);
        nStartY = (long) ( nStartY * HMM_PER_TWIPS );
        nStartY += (long) ( GraphHeader.y / nPPTY * HMM_PER_TWIPS );
        long nSizeY = (long) ( GraphHeader.h / nPPTY * HMM_PER_TWIPS );

#ifdef SC10_SHOW_OBJECTS
         // Achtung nur zu Debugzwecken
         //-----------------------------------
         nCol = 0;
         nRow++;
         switch (ObjectType)
         {
          case otOle :
           pDoc->SetString(nCol++, nRow, nTab, "Ole-Object");
           break;
          case otImage :
           pDoc->SetString(nCol++, nRow, nTab, "Image-Object");
           break;
          case otChart :
           pDoc->SetString(nCol++, nRow, nTab, "Chart-Object");
           break;
          default :
           pDoc->SetString(nCol++, nRow, nTab, "ERROR");
           break;
         }
         pDoc->SetValue(nCol++, nRow, nTab, GraphHeader.CarretX);
         pDoc->SetValue(nCol++, nRow, nTab, GraphHeader.CarretY);
         pDoc->SetValue(nCol++, nRow, nTab, GraphHeader.CarretZ);
         pDoc->SetValue(nCol++, nRow, nTab, GraphHeader.x);
         pDoc->SetValue(nCol++, nRow, nTab, GraphHeader.y);
         pDoc->SetValue(nCol++, nRow, nTab, GraphHeader.w);
         pDoc->SetValue(nCol++, nRow, nTab, GraphHeader.h);
         //-----------------------------------
#endif

        switch (ObjectType)
        {
          case otOle :
           // Achtung hier muss sowas wie OleLoadFromStream passieren
           IsOleObject = TRUE;
           break;
          case otImage :
          {
           Sc10ImageHeader ImageHeader;
           //rStream.Read(&ImageHeader, sizeof(ImageHeader));
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
            Sc10ChartTypeData* pTypeData = new Sc10ChartTypeData;
            //rStream.Read(&ChartHeader, sizeof(ChartHeader));
            lcl_ReadChartHeader(rStream, ChartHeader);

            //! altes Metafile verwenden ??
            rStream.SeekRel(ChartHeader.Size);

            //rStream.Read(&ChartSheetData, sizeof(ChartSheetData));
            lcl_ReadChartSheetData(rStream, ChartSheetData);

            //rStream.Read(pTypeData, sizeof(Sc10ChartTypeData));
            lcl_ReadChartTypeData(rStream, *pTypeData);

            Rectangle aRect( Point(nStartX,nStartY), Size(nSizeX,nSizeY) );
            Sc10InsertObject::InsertChart( pDoc, GraphHeader.CarretZ, aRect,
                                GraphHeader.CarretZ,
                                ChartSheetData.DataX1, ChartSheetData.DataY1,
                                ChartSheetData.DataX2, ChartSheetData.DataY2 );

            delete pTypeData;
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
    DBG_ERROR( "ObjectID" );
    nError = errUnknownID;
  }
}




//-----------------------------------------------------------------------------------------------

FltError ScImportStarCalc10( SvStream& rStream, ScDocument* pDocument )
{
    rStream.Seek( 0UL );
    Sc10Import  aImport( rStream, pDocument );
    return ( FltError ) aImport.Import();
}



