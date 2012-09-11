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

#ifndef SC_SCFLT_HXX
#define SC_SCFLT_HXX

#include "viewopti.hxx"
#include "collect.hxx"
#include <tools/solar.h>

// FehlerNummern
#define errUnknownFormat    1
#define errUnknownID        2
#define errOutOfMemory      3

// Identifiers im FileFormat
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

// Zeilen/Spalten Flags
#define crfSoftBreak        1
#define crfHardBreak        2
#define crfHidden           4

// Zelltypen
#define ctValue             1
#define ctString            2
#define ctFormula           3
#define ctNote              4

// FontStyles
#define ffDontCare          0x00
#define ffRoman             0x10
#define ffSwiss             0x20
#define ffModern            0x30
#define ffScript            0x40
#define ffDecorative        0x50

// FontWeight
#define fwNormal            400

// FontAttribute
#define atNone              0
#define atBold              1
#define atItalic            2
#define atUnderline         4
#define atStrikeOut         8

// Horizontale Ausrichtung
#define hjNone              0
#define hjLeft              1
#define hjCenter            2
#define hjRight             3

// Vertikale Ausrichtung
#define vjNone              0
#define vjTop               1
#define vjCenter            2
#define vjBottom            3

// AusrichtungsFlags
#define ojWordBreak         0x01
#define ojBottomTop         0x02
#define ojTopBottom         0x04

// ZellRaster
#define raNone              0
#define raGray12            1
#define raGray25            2
#define raGray50            3
#define raGray75            4
#define raGray100           5

// Zellschutz
#define paProtect           1
#define paHideFormula       2
#define paHideAll           4
#define paHidePrint         8

// ZahlenFormatFlags
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
#define vfNone              16

// DatumsFormatFlags
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

// ZeitFormatFlags
#define tf_HMS_Long         0
#define tf_HM_Long          1
#define tf_HMS_Short        2
#define tf_HM_Short         3

// Attribute fuer FormatVorlage
#define pfValue             0x01
#define pfFont              0x02
#define pfJustify           0x04
#define pfFrame             0x08
#define pfRaster            0x10
#define pfProtection        0x20

// Displayflags fuer die Tabelle
#define dfFormula           0x0001      // Formeln
#define dfZerro             0x0002      // Nullwerte
#define dfGrid              0x0004      // Gitternetz
#define dfPageBreak         0x0008      // Seitenumbruch
#define dfColRowBar         0x0010      // Zeilen/Spalten Koepfe (Dummy)
#define dfSyntax            0x0020      // Syntax Highlighting
#define dfPrintPage         0x0040      // Druckbildansicht (Dummy)
#define dfObjectAll         0x0080      // Objekte anzeigen
#define dfObjectFrame       0x0100      // Objekte als Platzhalter
#define dfObjectNone        0x0200      // Objekte nicht anzeigen
#define dfNoteMark          0x0400      // Notizanzeiger
#define dfProtectMark       0x0800      // Schutzanzeiger

// Objekt Typen
#define otNone              0           // s.u.
#define otOle               1
#define otImage             2
#define otChart             3

// Grafik Typen
#define gtNone              0           // Kann nicht vorkommen
#define gtOle               1           // Ole 1.0 Objekt
#define gtImage             2           // Image (Bitmap oder Metafile)
#define gtChart             3           // Chart

// Datum/Uhrzeit
struct Sc10DateTime
{
    sal_uInt16      Year;
    sal_uInt16      Month;
    sal_uInt16      Day;
    sal_uInt16      Hour;
    sal_uInt16      Min;
    sal_uInt16      Sec;
};

// ZahlenFormate
struct Sc10ValueFormat
{
    sal_uInt8       Format;         // Zahl, Waehrung, Prozent etc.
    sal_uInt8       Info;           // Anzahl Nachkommastellen, Anzahl Stellen, bzw. Datums/Zeitformat
};

// Fontbeschreibung
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

    int operator==( const Sc10LogFont& rData ) const;
};

// RGB-Frabwerte
struct Sc10Color
{
    sal_uInt8       Dummy;
    sal_uInt8       Blue;
    sal_uInt8       Green;
    sal_uInt8       Red;
    int operator==( const Sc10Color& rColor ) const;
};

// Blockbeschreibung
struct Sc10BlockRect
{
    sal_Int16       x1;
    sal_Int16       y1;
    sal_Int16       x2;
    sal_Int16       y2;
};

// Datenbank-Bereich
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
};

// Kopf/Fusszeilen-Beschreibung
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
    sal_uInt16          FrameColor; // Nibble Codierte Farben link oben rechts unten
    sal_uInt16          Reserved;

    int operator==( const Sc10HeadFootLine& rData ) const;
};

// Seitenformat
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
    sal_Char            PrnZoom[6]; // Pascal 6 Byte Realzahl
    SCTAB               FirstPageNo;
    sal_Int16               RowRepeatStart;
    sal_Int16               RowRepeatEnd;
    sal_Int16               ColRepeatStart;
    sal_Int16               ColRepeatEnd;
    sal_Char            Reserved[26];

    int operator==( const Sc10PageFormat& rData ) const;
};

// Tabellenschutz
struct Sc10TableProtect
{
    sal_Char    PassWord[16];
    sal_uInt16      Flags;
    sal_uInt8       Protect;
};

// Documentschutz
struct Sc10SheetProtect
{
    sal_Char    PassWord[16];
    sal_uInt16      Flags;
    sal_uInt8       Protect;
};

// Dateikopf StarCalc 1.0 Datei
struct Sc10FileHeader
{
    sal_Char    CopyRight[30];
    sal_uInt16      Version;
    sal_Char    Reserved[32];
};

// Benutzer-Definierte Datei-Beschreibung
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

// Letze Cursorposition
struct Sc10EditStateInfo
{
    // Cursor Position
    sal_uInt16      CarretX;
    sal_uInt16      CarretY;
    sal_uInt16      CarretZ;
    // Linke obere Ecke der Tabelle im Fenster
    sal_uInt16      DeltaX;
    sal_uInt16      DeltaY;
    sal_uInt16      DeltaZ;
    // Ueberfluessig in StarCalc 3.0
    sal_uInt8       DataBaseMode;
    sal_Char    Reserved[51];
};

// Attribut-Eintrag
struct Sc10ColData
{
    sal_uInt16      Row;
    sal_uInt16      Value;
};

// ZellAttribut-Beschreibung
struct Sc10ColAttr
{
    sal_uInt16          Count;
    Sc10ColData*    pData;

    Sc10ColAttr() : pData(NULL) {}
    ~Sc10ColAttr() { delete [] pData; }
};

// GraphHeader
struct Sc10GraphHeader
{
    sal_uInt8       Typ;            // Typ der Grafik (Ole-Objekt, Image (Bitmap oder MetaFile), Chart-Object)
    sal_Int16       CarretX;        // ZellPosition der Grafik
    sal_Int16       CarretY;
    sal_Int16       CarretZ;
    sal_Int32       x;              // x,y Abstand zum Zellrand in Pixel (Pixel weil ich Grafiken in Fenstern ablege)
    sal_Int32       y;
    sal_Int32       w;              // w,h Breite und Hoehe in Pixel
    sal_Int32       h;
    sal_uInt8       IsRelPos;       // Ist die Position relativ zur Zelle oder absolute in der Tabelle
    sal_uInt8       DoPrint;        // Soll die Grafik ausgedruckt werden
    sal_uInt16      FrameType;      // Art der Umrandung um die Grafik (Keine, Einfach, Doppelt, Einfach Dick, Doppelt Dick)
    sal_uInt8       IsTransparent;  // Soll der Hintergrund gezeichnet werden
    Sc10Color   FrameColor;     // Umrandungsfarbe als RGB-Wert
    Sc10Color   BackColor;      // Hintergrundfarbe als RGB-Wert
    sal_Char    Reserved[32];   // Na was wohl
};

// ImageHeader
struct Sc10ImageHeader
{
    sal_Char    FileName[128];  // Dateiname des urspruenglich eingefuegten Bildes
    sal_Int16   Typ;                // Typ der Grafik (Bitmap oder Metafile)
    sal_uInt8   Linked;             // Kann nicht vorkommen
    sal_Int16   x1;                 // Urspruengliche Groesse der Grafik (nur fuer Metafiles)
    sal_Int16   y1;
    sal_Int16   x2;
    sal_Int16   y2;
    sal_uInt32 Size;                // Groesse der Grafik in BYTES
};

// ChartHeader
struct Sc10ChartHeader
{
    sal_Int16   MM;                 // Meatfile Struktur MapMode, Breite, Hoehe
    sal_Int16   xExt;
    sal_Int16   yExt;
    sal_uInt32 Size;                // Groesse der Grafik in BYTES
};

// ChartSheetData
struct Sc10ChartSheetData
{
    sal_uInt8       HasTitle;       // Hat das Chart Daten aus der Tabell fuer einen Titel
    sal_Int16       TitleX;         // Zellposition des Titels
    sal_Int16       TitleY;
    sal_uInt8       HasSubTitle;    // Hat das Chart Daten aus der Tabell fuer einen Untertitel
    sal_Int16       SubTitleX;      // Zellposition des Untertitels
    sal_Int16       SubTitleY;
    sal_uInt8       HasLeftTitle;   // Hat das Chart Daten aus der Tabelle fuer einen Linken-Titel
    sal_Int16       LeftTitleX;     // Zellposition des Linken-Titels
    sal_Int16       LeftTitleY;
    sal_uInt8       HasLegend;      // Hat das Chart Daten aus der Tabelle fuer eine Legende
    sal_Int16       LegendX1;       // Zellen der Legende
    sal_Int16       LegendY1;
    sal_Int16       LegendX2;
    sal_Int16       LegendY2;
    sal_uInt8       HasLabel;       // Hat das Chart Daten aus der Tabelle fuer die Achsbeschriftung
    sal_Int16       LabelX1;        // Zellen der Achsbeschriftung
    sal_Int16       LabelY1;
    sal_Int16       LabelX2;
    sal_Int16       LabelY2;
    sal_Int16       DataX1;         // Zellen der Daten
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

                        Sc10FontData( const Sc10FontData& rData ) :
                            ScDataObject( rData ),
                            Height( rData.Height ),
                            CharSet( rData.CharSet ),
                            PitchAndFamily( rData.PitchAndFamily )
                                {
                                    strncpy( FaceName, rData.FaceName, sizeof(FaceName) );
                                    FaceName[sizeof(FaceName)-1] = 0;
                                }
                        Sc10FontData( SvStream& rStream );
    virtual ScDataObject*   Clone() const { return new Sc10FontData(*this); }
};


// Font-Collection
class Sc10FontCollection : public ScCollection
{
protected:
    sal_uLong nError;
public:
                        Sc10FontCollection( SvStream& rStream );
    sal_uLong               GetError() { return nError; }
    Sc10FontData*       At(sal_uInt16 nIndex) { return (Sc10FontData*)ScCollection::At(nIndex); }
private:
    using               ScCollection::At;
};


//BereichsDaten
class Sc10NameData : public ScDataObject
{
public :
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
    virtual ScDataObject*   Clone() const { return new Sc10NameData(*this); }
};


// Bereichs-Collection
class Sc10NameCollection : public ScCollection
{
protected:
    sal_uLong               nError;
public:
                        Sc10NameCollection(SvStream& rStream);
sal_uLong                   GetError() { return nError; }
Sc10NameData*           At(sal_uInt16 nIndex) { return (Sc10NameData*)ScCollection::At(nIndex); }
private:
    using               ScCollection::At;
};


// Vorlage-Daten
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
virtual ScDataObject*       Clone() const { return new Sc10PatternData(*this); }
};


// Vorlage-Collection
class Sc10PatternCollection : public ScCollection
{
protected:
    sal_uLong               nError;
public:
                        Sc10PatternCollection(SvStream& rStream);
    sal_uLong               GetError() { return nError; }
    Sc10PatternData*    At(sal_uInt16 nIndex) { return (Sc10PatternData*)ScCollection::At(nIndex); }
private:
    using               ScCollection::At;
};


// DatenBank-Daten
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
virtual ScDataObject*       Clone() const { return new Sc10DataBaseData(*this); }
};


// DatenBank-Collection
class Sc10DataBaseCollection : public ScCollection
{
protected:
    sal_uLong               nError;
    sal_Char            ActName[32];
public:
                        Sc10DataBaseCollection(SvStream& rStream);
    sal_uLong               GetError() { return nError; }
    Sc10DataBaseData*   At(sal_uInt16 nIndex) { return (Sc10DataBaseData*)ScCollection::At(nIndex); }
private:
    using               ScCollection::At;
};


class Sc10PageData : public ScDataObject
{
public:
    Sc10PageFormat      aPageFormat;
                        Sc10PageData( const Sc10PageFormat& rFormat ) : aPageFormat(rFormat) {}
    int                 operator==( const Sc10PageData& rData ) const
                            { return aPageFormat == rData.aPageFormat; }
    virtual ScDataObject*   Clone() const;
};

// Seitenformat-Collection
class Sc10PageCollection : public ScCollection
{
public:
                        Sc10PageCollection() : ScCollection(1,1) {};
    Sc10PageData*       At(sal_uInt16 nIndex) { return (Sc10PageData*)ScCollection::At(nIndex); }
    sal_uInt16              InsertFormat( const Sc10PageFormat& rData );
    void                PutToDoc( ScDocument* pDoc );
private:
    using               ScCollection::At;
};


class ScfStreamProgressBar;

// Import-Klasse
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
    void                    ChangeFormat(sal_uInt16 nFormat, sal_uInt16 nInfo, sal_uLong& nKey);
    void                    LoadObjects();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
