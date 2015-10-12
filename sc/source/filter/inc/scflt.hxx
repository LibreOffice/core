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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_SCFLT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_SCFLT_HXX

#include "viewopti.hxx"
#include "collect.hxx"
#include <tools/solar.h>

// Error numbers
#define errUnknownFormat    1
#define errUnknownID        2
#define errOutOfMemory      3

// Identifiers in FileFormat
#define ColWidthID          1
#define ColAttrID           2
#define RowHeightID         3
#define RowAttrID           4
#define FontID              5
#define NameID              6
#define TableID             7
#define ObjectID            8
#define PatternID           9
#define DataBaseID          10

// Row/Column Flags
#define crfSoftBreak        1
#define crfHardBreak        2
#define crfHidden           4

// Cell types
#define ctValue             1
#define ctString            2
#define ctFormula           3
#define ctNote              4

// Font Styles
#define ffDontCare          0x00
#define ffRoman             0x10
#define ffSwiss             0x20
#define ffModern            0x30
#define ffScript            0x40
#define ffDecorative        0x50

// Font Weight
#define fwNormal            400

// Font attributes
#define atNone              0
#define atBold              1
#define atItalic            2
#define atUnderline         4
#define atStrikeOut         8

// Horizontal alignment
#define hjLeft              1
#define hjCenter            2
#define hjRight             3

// Vertical alignment
#define vjTop               1
#define vjCenter            2
#define vjBottom            3

// Allignment flags
#define ojWordBreak         0x01
#define ojBottomTop         0x02
#define ojTopBottom         0x04

// Cell grid
#define raNone              0
#define raGray12            1
#define raGray25            2
#define raGray50            3
#define raGray75            4

// Cell protection
#define paProtect           1
#define paHideFormula       2
#define paHideAll           4
#define paHidePrint         8

// Number format flags
#define vfStandard          0
#define vfMoney             1
#define vfThousend          2
#define vfPercent           3
#define vfExponent          4
#define vfZerro             5
#define vfDate              6
#define vfTime              7
#define vfBoolean           8
#define vfStandardRed       9
#define vfMoneyRed          10
#define vfThousendRed       11
#define vfPercentRed        12
#define vfExponentRed       13
#define vfFormula           14
#define vfString            15

// Date format flags
#define df_NDMY_Long        0
#define df_DMY_Long         1
#define df_MY_Long          2
#define df_NDM_Long         3
#define df_DM_Long          4
#define df_M_Long           5
#define df_NDMY_Short       6
#define df_DMY_Short        7
#define df_MY_Short         8
#define df_NDM_Short        9
#define df_DM_Short         10
#define df_M_Short          11
#define df_Q_Long           12
#define df_Q_Short          13

// Time format flags
#define tf_HMS_Long         0
#define tf_HM_Long          1
#define tf_HMS_Short        2
#define tf_HM_Short         3

// Attributes for format template
#define pfValue             0x01
#define pfFont              0x02
#define pfJustify           0x04
#define pfFrame             0x08
#define pfRaster            0x10
#define pfProtection        0x20

// Display flags for table
#define dfFormula           0x0001      // formula
#define dfZerro             0x0002      // zero values
#define dfGrid              0x0004      // grid
#define dfSyntax            0x0020      // Syntax Highlighting
#define dfObjectAll         0x0080      // show objects
#define dfObjectFrame       0x0100      // objects as place holder
#define dfObjectNone        0x0200      // do not show objects
#define dfNoteMark          0x0400      // NoteMark

// Object types
#define otOle               1
#define otImage             2
#define otChart             3

// Date/Time
struct Sc10DateTime
{
    sal_uInt16      Year;
    sal_uInt16      Month;
    sal_uInt16      Day;
    sal_uInt16      Hour;
    sal_uInt16      Min;
    sal_uInt16      Sec;
};

// Number formats
struct Sc10ValueFormat
{
    sal_uInt8       Format;         // number, currency, percentage,...
    sal_uInt8       Info;           // number of decimals, or positions, or Date/Time format
    Sc10ValueFormat()
        : Format(0)
        , Info(0)
    {
    }
};

// Font definition
struct Sc10LogFont
{
    sal_Int16       lfHeight;
    sal_Int16       lfWidth;
    sal_Int16       lfEscapement;
    sal_Int16       lfOrientation;
    sal_Int16       lfWeight;
    sal_uInt8       lfItalic;
    sal_uInt8       lfUnderline;
    sal_uInt8       lfStrikeOut;
    sal_uInt8       lfCharSet;
    sal_uInt8       lfOutPrecision;
    sal_uInt8       lfClipPrecision;
    sal_uInt8       lfQuality;
    sal_uInt8       lfPitchAndFamily;
    sal_Char    lfFaceName[32];

    Sc10LogFont()
        : lfHeight(0)
        , lfWidth(0)
        , lfEscapement(0)
        , lfOrientation(0)
        , lfWeight(0)
        , lfItalic(0)
        , lfUnderline(0)
        , lfStrikeOut(0)
        , lfCharSet(0)
        , lfOutPrecision(0)
        , lfClipPrecision(0)
        , lfQuality(0)
        , lfPitchAndFamily(0)
    {
        memset(lfFaceName, 0, sizeof(lfFaceName));
    }

    bool operator==( const Sc10LogFont& rData ) const;
};

// RGB-Color values
struct Sc10Color
{
    sal_uInt8       Dummy;
    sal_uInt8       Blue;
    sal_uInt8       Green;
    sal_uInt8       Red;
    bool operator==( const Sc10Color& rColor ) const;
    Sc10Color()
        : Dummy(0)
        , Blue(0)
        , Green(0)
        , Red(0)
    {
    }
};

// Block definition
struct Sc10BlockRect
{
    sal_Int16       x1;
    sal_Int16       y1;
    sal_Int16       x2;
    sal_Int16       y2;
    Sc10BlockRect()
        : x1(0)
        , y1(0)
        , x2(0)
        , y2(0)
    {
    }
};

// Data range
struct Sc10DataBaseRec
{
    sal_Char        Name[32];
    SCTAB           Tab;
    Sc10BlockRect   Block;
    sal_uInt8           RowHeader;
    sal_Int16           SortField0;
    sal_uInt8           SortUpOrder0;
    sal_Int16           SortField1;
    sal_uInt8           SortUpOrder1;
    sal_Int16           SortField2;
    sal_uInt8           SortUpOrder2;
    sal_uInt8           IncludeFormat;
    sal_Int16           QueryField0;
    sal_Int16           QueryOp0;
    sal_uInt8           QueryByString0;
    sal_Char        QueryString0[64];
    double          QueryValue0;
    sal_Int16           QueryConnect1;
    sal_Int16           QueryField1;
    sal_Int16           QueryOp1;
    sal_uInt8           QueryByString1;
    sal_Char        QueryString1[64];
    double          QueryValue1;
    sal_Int16           QueryConnect2;
    sal_Int16           QueryField2;
    sal_Int16           QueryOp2;
    sal_uInt8           QueryByString2;
    sal_Char        QueryString2[64];
    double          QueryValue2;
    Sc10DataBaseRec()
        : Tab(0)
        , RowHeader(0)
        , SortField0(0)
        , SortUpOrder0(0)
        , SortField1(0)
        , SortUpOrder1(0)
        , SortField2(0)
        , SortUpOrder2(0)
        , IncludeFormat(0)
        , QueryField0(0)
        , QueryOp0(0)
        , QueryByString0(0)
        , QueryValue0(0.0)
        , QueryConnect1(0)
        , QueryField1(0)
        , QueryOp1(0)
        , QueryByString1(0)
        , QueryValue1(0.0)
        , QueryConnect2(0)
        , QueryField2(0)
        , QueryOp2(0)
        , QueryByString2(0)
        , QueryValue2(0.0)
    {
        memset(Name, 0, sizeof(Name));
        memset(QueryString0, 0, sizeof(QueryString0));
        memset(QueryString1, 0, sizeof(QueryString1));
        memset(QueryString2, 0, sizeof(QueryString2));
    }
};

// Header/Footer
struct Sc10HeadFootLine
{
    sal_Char        Title[128];
    Sc10LogFont     LogFont;
    sal_uInt8           HorJustify;
    sal_uInt8           VerJustify;
    sal_uInt16          Raster;
    sal_uInt16          Frame;
    Sc10Color       TextColor;
    Sc10Color       BackColor;
    Sc10Color       RasterColor;
    sal_uInt16          FrameColor; // Nibble coded colors left-top right-bottom
    sal_uInt16          Reserved;

    bool operator==( const Sc10HeadFootLine& rData ) const;
};

// page format
struct Sc10PageFormat
{
    Sc10HeadFootLine    HeadLine;
    Sc10HeadFootLine    FootLine;
    sal_Int16               Orientation;
    sal_Int16               Width;
    sal_Int16               Height;
    sal_Int16               NonPrintableX;
    sal_Int16               NonPrintableY;
    sal_Int16               Left;
    sal_Int16               Top;
    sal_Int16               Right;
    sal_Int16               Bottom;
    sal_Int16               Head;
    sal_Int16               Foot;
    sal_uInt8               HorCenter;
    sal_uInt8               VerCenter;
    sal_uInt8               PrintGrid;
    sal_uInt8               PrintColRow;
    sal_uInt8               PrintNote;
    sal_uInt8               TopBottomDir;
    sal_Char            PrintAreaName[32];
    Sc10BlockRect       PrintArea;
    sal_Char            PrnZoom[6]; // Pascal 6 Byte RealNumber
    SCTAB               FirstPageNo;
    sal_Int16               RowRepeatStart;
    sal_Int16               RowRepeatEnd;
    sal_Int16               ColRepeatStart;
    sal_Int16               ColRepeatEnd;
    sal_Char            Reserved[26];

    bool operator==( const Sc10PageFormat& rData ) const;
};

// Table protection
struct Sc10TableProtect
{
    sal_Char    PassWord[16];
    sal_uInt16      Flags;
    sal_uInt8       Protect;
};

// Document protection
struct Sc10SheetProtect
{
    sal_Char    PassWord[16];
    sal_uInt16      Flags;
    sal_uInt8       Protect;
    Sc10SheetProtect()
        : Flags(0)
        , Protect(0)
    {
        memset(PassWord, 0, sizeof(PassWord));
    }
};

// FileHeader StarCalc 1.0 Datei
struct Sc10FileHeader
{
    sal_Char    CopyRight[30];
    sal_uInt16      Version;
    sal_Char    Reserved[32];
};

// User-Defined File Definition
struct Sc10FileInfo
{
    sal_Char        Title[64];
    sal_Char        Thema[64];
    sal_Char        Keys[64];
    sal_Char        Note[256];
    sal_Char        InfoLabel0[16];
    sal_Char        InfoLabel1[16];
    sal_Char        InfoLabel2[16];
    sal_Char        InfoLabel3[16];
    sal_Char        Info0[32];
    sal_Char        Info1[32];
    sal_Char        Info2[32];
    sal_Char        Info3[32];
    sal_Char        CreateAuthor[64];
    sal_Char        ChangeAuthor[64];
    sal_Char        PrintAuthor[64];
    Sc10DateTime    CreateDate;
    Sc10DateTime    ChangeDate;
    Sc10DateTime    PrintDate;
    sal_uInt32      PageCount;
    sal_uInt32      ChartCount;
    sal_uInt32      PictureCount;
    sal_uInt32      GraphCount;
    sal_uInt32      OleCount;
    sal_uInt32      NoteCount;
    sal_uInt32      TextCellCount;
    sal_uInt32      ValueCellCount;
    sal_uInt32      FormulaCellCount;
    sal_uInt32      CellCount;
    sal_Char        Reserved[52];
};

// Last Cursor position
struct Sc10EditStateInfo
{
    // Cursor Position
    sal_uInt16      CarretX;
    sal_uInt16      CarretY;
    sal_uInt16      CarretZ;
    // left-top corner of the table
    sal_uInt16      DeltaX;
    sal_uInt16      DeltaY;
    sal_uInt16      DeltaZ;
    // redundant in StarCalc 3.0
    sal_uInt8       DataBaseMode;
    sal_Char    Reserved[51];
};

// Attribut entry
struct Sc10ColData
{
    sal_uInt16      Row;
    sal_uInt16      Value;
};

// Cell-Attribute definition
struct Sc10ColAttr
{
    sal_uInt16          Count;
    Sc10ColData*    pData;

    Sc10ColAttr()
        : Count(0)
        , pData(NULL)
    {
    }
    ~Sc10ColAttr() { delete [] pData; }
};

// GraphHeader
struct Sc10GraphHeader
{
    sal_uInt8       Typ;            // Type of Grafik (Ole-Objekt, Image (Bitmap of MetaFile), Chart-Object)
    sal_Int16       CarretX;        // Cell Position of Grafik
    sal_Int16       CarretY;
    sal_Int16       CarretZ;
    sal_Int32       x;              // x,y distance to cell border in pixels (pixels, as Grafics are stored in window)
    sal_Int32       y;
    sal_Int32       w;              // w,h width and height in pixels
    sal_Int32       h;
    sal_uInt8       IsRelPos;       // position relativ to cell, or absolute in the table
    sal_uInt8       DoPrint;        // print Grafic?
    sal_uInt16      FrameType;      // Frame type around Grafic (none, simple, double, simple thick, double thick)
    sal_uInt8       IsTransparent;  // show background?
    Sc10Color   FrameColor;         // FrameColor as RGB value
    Sc10Color   BackColor;          // BackGroundColor as RGB value
    sal_Char    Reserved[32];       // Reserved
};

// ImageHeader
struct Sc10ImageHeader
{
    sal_Char    FileName[128];      // file name of the original inserted image
    sal_Int16   Typ;                // image type (Bitmap or Metafile)
    sal_uInt8   Linked;             // not possible
    sal_Int16   x1;                 // original image size (only for Metafiles)
    sal_Int16   y1;
    sal_Int16   x2;
    sal_Int16   y2;
    sal_uInt32 Size;                // image size in BYTES
};

// ChartHeader
struct Sc10ChartHeader
{
    sal_Int16   MM;                 // Meatfile structure MapMode, Width, Hight
    sal_Int16   xExt;
    sal_Int16   yExt;
    sal_uInt32 Size;                // image size in BYTES
};

// ChartSheetData
struct Sc10ChartSheetData
{
    sal_uInt8       HasTitle;       // has the Chart a title in the table
    sal_Int16       TitleX;         // cell position of title
    sal_Int16       TitleY;
    sal_uInt8       HasSubTitle;    // has the Chart a sub-title in the table
    sal_Int16       SubTitleX;      // cell position of sub-title
    sal_Int16       SubTitleY;
    sal_uInt8       HasLeftTitle;   // has the Chart a left-title in the table
    sal_Int16       LeftTitleX;     // cell position of left-title
    sal_Int16       LeftTitleY;
    sal_uInt8       HasLegend;      // has the Chart a legend in the table
    sal_Int16       LegendX1;       // cells for legend
    sal_Int16       LegendY1;
    sal_Int16       LegendX2;
    sal_Int16       LegendY2;
    sal_uInt8       HasLabel;       // has the Chart axis labels in the table
    sal_Int16       LabelX1;        // cells for axis labels
    sal_Int16       LabelY1;
    sal_Int16       LabelX2;
    sal_Int16       LabelY2;
    sal_Int16       DataX1;         // cells for Data
    sal_Int16       DataY1;
    sal_Int16       DataX2;
    sal_Int16       DataY2;
    sal_Char    Reserved[64];
};

typedef sal_Char Sc10ChartText[30];

struct Sc10ChartTypeData
{
    sal_Int16           NumSets;
    sal_Int16           NumPoints;
    sal_Int16           DrawMode;
    sal_Int16           GraphType;
    sal_Int16           GraphStyle;
    sal_Char        GraphTitle[80];
    sal_Char        BottomTitle[80];
    sal_Int16           SymbolData[256];
    sal_Int16           ColorData[256];
    sal_Int16           ThickLines[256];
    sal_Int16           PatternData[256];
    sal_Int16           LinePatternData[256];
    sal_Int16           NumGraphStyles[11];
    sal_Int16           ShowLegend;
    Sc10ChartText   LegendText[256];
    sal_Int16           ExplodePie;
    sal_Int16           FontUse;
    sal_Int16           FontFamily[5];
    sal_Int16           FontStyle[5];
    sal_Int16           FontSize[5];
    sal_Int16           GridStyle;
    sal_Int16           Labels;
    sal_Int16           LabelEvery;
    Sc10ChartText   LabelText[50];
    sal_Char        LeftTitle[80];
    sal_Char        Reserved[4646];
};

// FontAttribut
class Sc10FontData : public ScDataObject
{
public:
    sal_Int16               Height;
    sal_uInt8               CharSet;
    sal_uInt8               PitchAndFamily;
    sal_Char            FaceName[32];

    Sc10FontData( const Sc10FontData& rData )
        : ScDataObject( rData )
        , Height( rData.Height )
        , CharSet( rData.CharSet )
        , PitchAndFamily( rData.PitchAndFamily )
    {
        strncpy( FaceName, rData.FaceName, sizeof(FaceName) );
        FaceName[sizeof(FaceName)-1] = 0;
    }
    Sc10FontData( SvStream& rStream );
    virtual ScDataObject*   Clone() const override { return new Sc10FontData(*this); }
};

// Font-Collection
class Sc10FontCollection : public ScCollection
{
protected:
    sal_uLong nError;
public:
                        Sc10FontCollection( SvStream& rStream );
    sal_uLong           GetError() { return nError; }
    Sc10FontData*       At(sal_uInt16 nIndex) { return static_cast<Sc10FontData*>(ScCollection::At(nIndex)); }
private:
    using               ScCollection::At;
};

//Range-Data
class Sc10NameData : public ScDataObject
{
public:
    sal_Char            Name[32];
    sal_Char            Reference[64];
    sal_Char            Reserved[12];

                        Sc10NameData(const Sc10NameData& rData) :
                            ScDataObject( rData )
                        {
                            strncpy(Name, rData.Name, sizeof(Name));
                            Name[sizeof(Name)-1] = 0;
                            strncpy(Reference, rData.Reference, sizeof(Reference));
                            Reference[sizeof(Reference)-1] = 0;
                            memcpy(Reserved, rData.Reserved, sizeof(Reserved));
                        }
                        Sc10NameData(SvStream& rStream);
    virtual ScDataObject*   Clone() const override { return new Sc10NameData(*this); }
};

// Range-Collection
class Sc10NameCollection : public ScCollection
{
protected:
    sal_uLong           nError;
public:
                        Sc10NameCollection(SvStream& rStream);
    sal_uLong           GetError() { return nError; }
    Sc10NameData*       At(sal_uInt16 nIndex) { return static_cast<Sc10NameData*>(ScCollection::At(nIndex)); }
private:
    using               ScCollection::At;
};

// Template-Data
class Sc10PatternData : public ScDataObject
{
public:
    sal_Char            Name[32];
    Sc10ValueFormat     ValueFormat;
    Sc10LogFont         LogFont;
    sal_uInt16              Attr;
    sal_uInt16              Justify;
    sal_uInt16              Frame;
    sal_uInt16              Raster;
    sal_uInt16              nColor;
    sal_uInt16              FrameColor;
    sal_uInt16              Flags;
    sal_uInt16              FormatFlags;
    sal_Char            Reserved[8];

                        Sc10PatternData(const Sc10PatternData& rData) :
                            ScDataObject( rData )
                        {
                            strncpy(Name, rData.Name, sizeof(Name));
                            Name[sizeof(Name)-1] = 0;
                            memcpy(&ValueFormat, &rData.ValueFormat, sizeof(ValueFormat));
                            memcpy(&LogFont, &rData.LogFont, sizeof(LogFont));
                            Attr = rData.Attr;
                            Justify = rData.Justify;
                            Frame = rData.Frame;
                            Raster = rData.Raster;
                            nColor = rData.nColor;
                            FrameColor = rData.FrameColor;
                            Flags = rData.Flags;
                            FormatFlags = rData.FormatFlags;
                            memcpy(Reserved, rData.Reserved, sizeof(Reserved));
                        }
                        Sc10PatternData(SvStream& rStream);
virtual ScDataObject*       Clone() const override { return new Sc10PatternData(*this); }
};

// Template-Collection
class Sc10PatternCollection : public ScCollection
{
protected:
    sal_uLong           nError;
public:
                        Sc10PatternCollection(SvStream& rStream);
    sal_uLong           GetError() { return nError; }
    Sc10PatternData*    At(sal_uInt16 nIndex) { return static_cast<Sc10PatternData*>(ScCollection::At(nIndex)); }
private:
    using               ScCollection::At;
};

// DataBank-Data
class Sc10DataBaseData : public ScDataObject
{
public:
    Sc10DataBaseRec     DataBaseRec;

                        Sc10DataBaseData(const Sc10DataBaseData& rData) :
                            ScDataObject( rData )
                        {
                            memcpy(&DataBaseRec, &rData.DataBaseRec, sizeof(DataBaseRec));
                        }
                        Sc10DataBaseData(SvStream& rStream);
virtual ScDataObject*       Clone() const override { return new Sc10DataBaseData(*this); }
};

// DataBank-Collection
class Sc10DataBaseCollection : public ScCollection
{
protected:
    sal_uLong           nError;
    sal_Char            ActName[32];
public:
                        Sc10DataBaseCollection(SvStream& rStream);
    sal_uLong           GetError() { return nError; }
    Sc10DataBaseData*   At(sal_uInt16 nIndex) { return static_cast<Sc10DataBaseData*>(ScCollection::At(nIndex)); }
private:
    using               ScCollection::At;
};

class Sc10PageData : public ScDataObject
{
public:
    Sc10PageFormat      aPageFormat;
                        Sc10PageData( const Sc10PageFormat& rFormat ) : aPageFormat(rFormat) {}
    bool                operator==( const Sc10PageData& rData ) const
                            { return aPageFormat == rData.aPageFormat; }
    virtual ScDataObject*   Clone() const override;
};

// PageFormat-Collection
class Sc10PageCollection : public ScCollection
{
public:
                        Sc10PageCollection() : ScCollection(1,1) {};
    Sc10PageData*       At(sal_uInt16 nIndex) { return static_cast<Sc10PageData*>(ScCollection::At(nIndex)); }
    sal_uInt16              InsertFormat( const Sc10PageFormat& rData );
    void                PutToDoc( ScDocument* pDoc );
private:
    using               ScCollection::At;
};

class ScfStreamProgressBar;

// Import-Class
class Sc10Import
{
    SvStream&               rStream;
    ScDocument*             pDoc;
    Sc10Color               TextPalette[16];
    Sc10Color               BackPalette[16];
    Sc10Color               RasterPalette[16];
    Sc10Color               FramePalette[16];
    Sc10SheetProtect        SheetProtect;
    Sc10FontCollection*     pFontCollection;
    Sc10NameCollection*     pNameCollection;
    Sc10PatternCollection*  pPatternCollection;
    Sc10DataBaseCollection* pDataBaseCollection;
    sal_uLong                   nError;
    SCTAB                   nShowTab;
    ScViewOptions           aSc30ViewOpt;
    ScfStreamProgressBar*   pPrgrsBar;

public:
                              Sc10Import( SvStream& rStr, ScDocument* pDocument );
                              ~Sc10Import();

    sal_uLong                   Import();
    void                    LoadFileHeader();
    void                    LoadFileInfo();
    void                    LoadEditStateInfo();
    void                    LoadProtect();
    void                    LoadViewColRowBar();
    void                    LoadScrZoom();
    void                    LoadPalette();
    void                    LoadFontCollection();
    void                    LoadNameCollection();
    void                    ImportNameCollection();
    void                    LoadPatternCollection();
    void                    LoadDataBaseCollection();
    void                    LoadTables();
    void                    LoadCol(SCCOL Col, SCTAB Tab);
    void                    LoadColAttr(SCCOL Col, SCTAB Tab);
    void                    LoadAttr(Sc10ColAttr& rAttr);
    static void             ChangeFormat(sal_uInt16 nFormat, sal_uInt16 nInfo, sal_uLong& nKey);
    void                    LoadObjects();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
