/*************************************************************************
 *
 *  $RCSfile: scflt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:13 $
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

#ifndef SC_SCFLT_HXX
#define SC_SCFLT_HXX

#include <tools/solar.h>
#include "viewopti.hxx"

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

// Attribute für FormatVorlage
#define pfValue             0x01
#define pfFont              0x02
#define pfJustify           0x04
#define pfFrame             0x08
#define pfRaster            0x10
#define pfProtection        0x20

// Displayflags für die Tabelle
#define dfFormula           0x0001      // Formeln
#define dfZerro             0x0002      // Nullwerte
#define dfGrid              0x0004      // Gitternetz
#define dfPageBreak         0x0008      // Seitenumbruch
#define dfColRowBar         0x0010      // Zeilen/Spalten Köpfe (Dummy)
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
    USHORT      Year;
    USHORT      Month;
    USHORT      Day;
    USHORT      Hour;
    USHORT      Min;
    USHORT      Sec;
};

// ZahlenFormate
struct Sc10ValueFormat
{
    BYTE        Format;         // Zahl, Waehrung, Prozent etc.
    BYTE        Info;           // Anzahl Nachkommastellen, Anzahl Stellen, bzw. Datums/Zeitformat
};

// Fontbeschreibung
struct Sc10LogFont
{
    INT16       lfHeight;
    INT16       lfWidth;
    INT16       lfEscapement;
    INT16       lfOrientation;
    INT16       lfWeight;
    BYTE        lfItalic;
    BYTE        lfUnderline;
    BYTE        lfStrikeOut;
    BYTE        lfCharSet;
    BYTE        lfOutPrecision;
    BYTE        lfClipPrecision;
    BYTE        lfQuality;
    BYTE        lfPitchAndFamily;
    sal_Char    lfFaceName[32];

    int operator==( const Sc10LogFont& rData ) const;
};

// RGB-Frabwerte
struct Sc10Color
{
    BYTE        Dummy;
    BYTE        Blue;
    BYTE        Green;
    BYTE        Red;
    int operator==( const Sc10Color& rColor ) const;
};

// Blockbeschreibung
struct Sc10BlockRect
{
    INT16       x1;
    INT16       y1;
    INT16       x2;
    INT16       y2;
};

// Datenbank-Bereich
struct Sc10DataBaseRec
{
    sal_Char        Name[32];
    INT16           Tab;
    Sc10BlockRect   Block;
    BYTE            RowHeader;
    INT16           SortField0;
    BYTE            SortUpOrder0;
    INT16           SortField1;
    BYTE            SortUpOrder1;
    INT16           SortField2;
    BYTE            SortUpOrder2;
    BYTE            IncludeFormat;
    INT16           QueryField0;
    INT16           QueryOp0;
    BYTE            QueryByString0;
    sal_Char        QueryString0[64];
    double          QueryValue0;
    INT16           QueryConnect1;
    INT16           QueryField1;
    INT16           QueryOp1;
    BYTE            QueryByString1;
    sal_Char        QueryString1[64];
    double          QueryValue1;
    INT16           QueryConnect2;
    INT16           QueryField2;
    INT16           QueryOp2;
    BYTE            QueryByString2;
    sal_Char        QueryString2[64];
    double          QueryValue2;
};

// Kopf/Fußzeilen-Beschreibung
struct Sc10HeadFootLine
{
    sal_Char        Title[128];
    Sc10LogFont     LogFont;
    BYTE            HorJustify;
    BYTE            VerJustify;
    USHORT          Raster;
    USHORT          Frame;
    Sc10Color       TextColor;
    Sc10Color       BackColor;
    Sc10Color       RasterColor;
    USHORT          FrameColor; // Nibble Codierte Farben link oben rechts unten
    USHORT          Reserved;

    int operator==( const Sc10HeadFootLine& rData ) const;
};

// Seitenformat
struct Sc10PageFormat
{
    Sc10HeadFootLine    HeadLine;
    Sc10HeadFootLine    FootLine;
    INT16               Orientation;
    INT16               Width;
    INT16               Height;
    INT16               NonPrintableX;
    INT16               NonPrintableY;
    INT16               Left;
    INT16               Top;
    INT16               Right;
    INT16               Bottom;
    INT16               Head;
    INT16               Foot;
    BYTE                HorCenter;
    BYTE                VerCenter;
    BYTE                PrintGrid;
    BYTE                PrintColRow;
    BYTE                PrintNote;
    BYTE                TopBottomDir;
    sal_Char            PrintAreaName[32];
    Sc10BlockRect       PrintArea;
    sal_Char            PrnZoom[6]; // Pascal 6 Byte Realzahl
    USHORT              FirstPageNo;
    INT16               RowRepeatStart;
    INT16               RowRepeatEnd;
    INT16               ColRepeatStart;
    INT16               ColRepeatEnd;
    sal_Char            Reserved[26];

    int operator==( const Sc10PageFormat& rData ) const;
};

// Tabellenschutz
struct Sc10TableProtect
{
    sal_Char    PassWord[16];
    USHORT      Flags;
    BYTE        Protect;
};

// Documentschutz
struct Sc10SheetProtect
{
    sal_Char    PassWord[16];
    USHORT      Flags;
    BYTE        Protect;
};

// Dateikopf StarCalc 1.0 Datei
struct Sc10FileHeader
{
    sal_Char    CopyRight[30];
    USHORT      Version;
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
    ULONG           PageCount;
    ULONG           ChartCount;
    ULONG           PictureCount;
    ULONG           GraphCount;
    ULONG           OleCount;
    ULONG           NoteCount;
    ULONG           TextCellCount;
    ULONG           ValueCellCount;
    ULONG           FormulaCellCount;
    ULONG           CellCount;
    sal_Char        Reserved[52];
};

// Letze Cursorposition
struct Sc10EditStateInfo
{
    // Cursor Position
    USHORT      CarretX;
    USHORT      CarretY;
    USHORT      CarretZ;
    // Linke obere Ecke der Tabelle im Fenster
    USHORT      DeltaX;
    USHORT      DeltaY;
    USHORT      DeltaZ;
    // Ueberfluessig in StarCalc 3.0
    BYTE        DataBaseMode;
    sal_Char    Reserved[51];
};

// Attribut-Eintrag
struct Sc10ColData
{
    USHORT      Row;
    USHORT      Value;
};

// ZellAttribut-Beschreibung
struct Sc10ColAttr
{
    USHORT          Count;
    Sc10ColData*    pData;

    Sc10ColAttr() : pData(NULL) {}
};

// GraphHeader
struct Sc10GraphHeader
{
    BYTE        Typ;            // Typ der Grafik (Ole-Objekt, Image (Bitmap oder MetaFile), Chart-Object)
    INT16       CarretX;        // ZellPosition der Grafik
    INT16       CarretY;
    INT16       CarretZ;
    INT32       x;              // x,y Abstand zum Zellrand in Pixel (Pixel weil ich Grafiken in Fenstern ablege)
    INT32       y;
    INT32       w;              // w,h Breite und Hoehe in Pixel
    INT32       h;
    BYTE        IsRelPos;       // Ist die Position relativ zur Zelle oder absolute in der Tabelle
    BYTE        DoPrint;        // Soll die Grafik ausgedruckt werden
    USHORT      FrameType;      // Art der Umrandung um die Grafik (Keine, Einfach, Doppelt, Einfach Dick, Doppelt Dick)
    BYTE        IsTransparent;  // Soll der Hintergrund gezeichnet werden
    Sc10Color   FrameColor;     // Umrandungsfarbe als RGB-Wert
    Sc10Color   BackColor;      // Hintergrundfarbe als RGB-Wert
    sal_Char    Reserved[32];   // Na was wohl
};

// ImageHeader
struct Sc10ImageHeader
{
    sal_Char    FileName[128];  // Dateiname des urspruenglich eingefügten Bildes
    INT16   Typ;                // Typ der Grafik (Bitmap oder Metafile)
    BYTE    Linked;             // Kann nicht vorkommen
    INT16   x1;                 // Urspruengliche Groesse der Grafik (nur fuer Metafiles)
    INT16   y1;
    INT16   x2;
    INT16   y2;
    ULONG   Size;               // Groesse der Grafik in BYTES
};

// ChartHeader
struct Sc10ChartHeader
{
    INT16   MM;                 // Meatfile Struktur MapMode, Breite, Hoehe
    INT16   xExt;
    INT16   yExt;
    ULONG   Size;               // Groesse der Grafik in BYTES
};

// ChartSheetData
struct Sc10ChartSheetData
{
    BYTE        HasTitle;       // Hat das Chart Daten aus der Tabell fuer einen Titel
    INT16       TitleX;         // Zellposition des Titels
    INT16       TitleY;
    BYTE        HasSubTitle;    // Hat das Chart Daten aus der Tabell fuer einen Untertitel
    INT16       SubTitleX;      // Zellposition des Untertitels
    INT16       SubTitleY;
    BYTE        HasLeftTitle;   // Hat das Chart Daten aus der Tabelle fuer einen Linken-Titel
    INT16       LeftTitleX;     // Zellposition des Linken-Titels
    INT16       LeftTitleY;
    BYTE        HasLegend;      // Hat das Chart Daten aus der Tabelle fuer eine Legende
    INT16       LegendX1;       // Zellen der Legende
    INT16       LegendY1;
    INT16       LegendX2;
    INT16       LegendY2;
    BYTE        HasLabel;       // Hat das Chart Daten aus der Tabelle fuer die Achsbeschriftung
    INT16       LabelX1;        // Zellen der Achsbeschriftung
    INT16       LabelY1;
    INT16       LabelX2;
    INT16       LabelY2;
    INT16       DataX1;         // Zellen der Daten
    INT16       DataY1;
    INT16       DataX2;
    INT16       DataY2;
    sal_Char    Reserved[64];
};

#define AGPie2D     1           // Pie Chart 2D
#define AGPie3D     2           // Pie Chart 3D
#define AGBar2D     3           // Bar Chart 2D
#define AGBar3D     4           // Bar Chart 3D
#define AGGantt     5           // Gantt Diagramm
#define AGLine      6           // Line Chart
#define AGLogLine   7           // Logarythmic-Coordinate Chart
#define AGArea      8           // Area Chart
#define AGHLC       11          // High-Low-Close Chart
#define AGPolar     10          // Polar-Coordinate Chart

typedef sal_Char Sc10ChartText[30];

struct Sc10ChartTypeData
{
    INT16           NumSets;
    INT16           NumPoints;
    INT16           DrawMode;
    INT16           GraphType;
    INT16           GraphStyle;
    sal_Char        GraphTitle[80];
    sal_Char        BottomTitle[80];
    INT16           SymbolData[256];
    INT16           ColorData[256];
    INT16           ThickLines[256];
    INT16           PatternData[256];
    INT16           LinePatternData[256];
    INT16           NumGraphStyles[11];
    INT16           ShowLegend;
    Sc10ChartText   LegendText[256];
    INT16           ExplodePie;
    INT16           FontUse;
    INT16           FontFamily[5];
    INT16           FontStyle[5];
    INT16           FontSize[5];
    INT16           GridStyle;
    INT16           Labels;
    INT16           LabelEvery;
    Sc10ChartText   LabelText[50];
    sal_Char        LeftTitle[80];
    sal_Char        Reserved[4646];
};


// FontAttribut
class Sc10FontData : public DataObject
{
public:
    INT16               Height;
    BYTE                CharSet;
    BYTE                PitchAndFamily;
    sal_Char            FaceName[32];

                        Sc10FontData( const Sc10FontData& rData ) :
                            Height( rData.Height ),
                            CharSet( rData.CharSet ),
                            PitchAndFamily( rData.PitchAndFamily ) { strcpy( FaceName, rData.FaceName ); }
                        Sc10FontData( SvStream& rStream );
    virtual DataObject* Clone() const { return new Sc10FontData(*this); }
};


// Font-Collection
class Sc10FontCollection : public Collection
{
protected:
    ULONG nError;
public:
                        Sc10FontCollection( SvStream& rStream );
    ULONG               GetError() { return nError; }
    Sc10FontData*       At(USHORT nIndex) { return (Sc10FontData*)Collection::At(nIndex); }
};


//BereichsDaten
class Sc10NameData : public DataObject
{
public :
    sal_Char            Name[32];
    sal_Char            Reference[64];
    sal_Char            Reserved[12];

                        Sc10NameData(const Sc10NameData& rData)
                        {
                            strcpy(Name, rData.Name);
                            strcpy(Reference, rData.Reference);
                            memcpy(Reserved, rData.Reserved, sizeof(Reserved));
                        }
                        Sc10NameData(SvStream& rStream);
    virtual DataObject* Clone() const { return new Sc10NameData(*this); }
};


// Bereichs-Collection
class Sc10NameCollection : public Collection
{
protected:
    ULONG               nError;
public:
                        Sc10NameCollection(SvStream& rStream);
ULONG                   GetError() { return nError; }
Sc10NameData*           At(USHORT nIndex) { return (Sc10NameData*)Collection::At(nIndex); }
};


// Vorlage-Daten
class Sc10PatternData : public DataObject
{
public:
    sal_Char            Name[32];
    Sc10ValueFormat     ValueFormat;
    Sc10LogFont         LogFont;
    USHORT              Attr;
    USHORT              Justify;
    USHORT              Frame;
    USHORT              Raster;
    USHORT              nColor;
    USHORT              FrameColor;
    USHORT              Flags;
    USHORT              FormatFlags;
    sal_Char            Reserved[8];

                        Sc10PatternData(const Sc10PatternData& rData)
                        {
                            strcpy(Name, rData.Name);
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
virtual DataObject*     Clone() const { return new Sc10PatternData(*this); }
};


// Vorlage-Collection
class Sc10PatternCollection : public Collection
{
protected:
    ULONG               nError;
public:
                        Sc10PatternCollection(SvStream& rStream);
    ULONG               GetError() { return nError; }
    Sc10PatternData*    At(USHORT nIndex) { return (Sc10PatternData*)Collection::At(nIndex); }
};


// DatenBank-Daten
class Sc10DataBaseData : public DataObject
{
public:
    Sc10DataBaseRec     DataBaseRec;

                        Sc10DataBaseData(const Sc10DataBaseData& rData)
                        {
                            memcpy(&DataBaseRec, &rData.DataBaseRec, sizeof(DataBaseRec));
                        }
                        Sc10DataBaseData(SvStream& rStream);
virtual DataObject*     Clone() const { return new Sc10DataBaseData(*this); }
};


// DatenBank-Collection
class Sc10DataBaseCollection : public Collection
{
protected:
    ULONG               nError;
    sal_Char            ActName[32];
public:
                        Sc10DataBaseCollection(SvStream& rStream);
    ULONG               GetError() { return nError; }
    Sc10DataBaseData*   At(USHORT nIndex) { return (Sc10DataBaseData*)Collection::At(nIndex); }
};


class Sc10PageData : public DataObject
{
public:
    Sc10PageFormat      aPageFormat;
                        Sc10PageData( const Sc10PageFormat& rFormat ) : aPageFormat(rFormat) {}
    int                 operator==( const Sc10PageData& rData ) const
                            { return aPageFormat == rData.aPageFormat; }
    virtual DataObject* Clone() const;
};

// Seitenformat-Collection
class Sc10PageCollection : public Collection
{
public:
                        Sc10PageCollection() : Collection(1,1) {};
    Sc10PageData*       At(USHORT nIndex) { return (Sc10PageData*)Collection::At(nIndex); }
    USHORT              Insert( const Sc10PageFormat& rData );
    void                PutToDoc( ScDocument* pDoc );
};


class FilterProgressBar;

// Import-Klasse
class Sc10Import
{
    ULONG                   nError;
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
    INT16                   TabCount;
    USHORT                  nShowTab;
    ScViewOptions           aSc30ViewOpt;
    FilterProgressBar*      pPrgrsBar;

public:
                              Sc10Import( SvStream& rStr, ScDocument* pDocument );
                              ~Sc10Import();

    ULONG                   Import();
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
    void                    LoadCol(USHORT Col, USHORT Tab);
    void                    LoadColAttr(USHORT Col, USHORT Tab);
    void                    LoadAttr(Sc10ColAttr& rAttr);
    void                    ChangeFormat(USHORT nFormat, USHORT nInfo, ULONG& nKey);
    void                    LoadObjects();
};

#endif

