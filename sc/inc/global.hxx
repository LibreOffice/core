/*************************************************************************
 *
 *  $RCSfile: global.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-20 10:26:42 $
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

#ifndef SC_SCGLOB_HXX
#define SC_SCGLOB_HXX

#ifndef _LANG_HXX //autogen
#include <tools/lang.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

class ImageList;
class Bitmap;
class International;
class SfxItemSet;
class Color;


#ifdef WIN
//  auf Windows 16 muessen die Zeilen weiter auf 8192 begrenzt werden
//  (zum Testen per Hand definieren)
#define SC_LIMIT_ROWS
#endif

// Macro fuer den Call-Profiler unter WinNT
// mit S_CAP kann eine Messung gestarted, mit E_CAP wieder gestoppt werden
#if defined( WNT ) && defined( PROFILE )

extern "C" {
    void StartCAP();
    void StopCAP();
    void DumpCAP();
};

#define S_CAP   StartCAP();
#define E_CAP   StopCAP(); DumpCAP();

#endif

//------------------------------------------------------------------------
struct LabelData;
//------------------------------------------------------------------------

//  die 1000 Namen des Calc...
//  Clipboard-Namen sind jetzt in so3/soapp.hxx
//  STRING_SCAPP war "scalc3", "scalc4", jetzt nur noch "scalc"

#define STRING_SCAPP    "scalc"
#define STRING_SCSTREAM "StarCalcDocument"

#define STRING_STANDARD "Standard"

//  '\r' geht auf'm Mac nicht...
#define CHAR_CR     char(13)

#define MINDOUBLE   1.7e-307
#define MAXDOUBLE   1.7e307

#define MAXROW_30   8191
#define MAXROW_40   31999
#ifdef SC_LIMIT_ROWS
#define MAXROW      8191
#else
#define MAXROW      31999
#endif

#define MAXCOL      255
#define MAXTAB      255

#define MINZOOM     20
#define MAXZOOM     400

#define MAXSUBTOTAL 3
#define MAXSORT     3
#define MAXQUERY    8
#define PIVOT_MAXFIELD  8

#define SC_START_INDEX_DB_COLL 50000
                                        // Oberhalb dieser Grenze liegen
                                        // die Indizes fuer DBBereiche

#define VALIDROW(nRow)                  (nRow>=0 && nRow<=MAXROW)
#define VALIDCOL(nCol)                  (nCol>=0 && nCol<=MAXCOL)
#define VALIDTAB(nTab)                  (nTab>=0 && nTab<=MAXTAB)
#define VALIDCOLROW(nCol,nRow)          (VALIDCOL(nCol) && VALIDROW(nRow))
#define VALIDCOLROWTAB(nCol,nRow,nTab)  (VALIDCOL(nCol) && VALIDROW(nRow) && VALIDTAB(nTab))

inline BOOL ValidColRow(USHORT nCol, USHORT nRow)
{
    return nCol <= MAXCOL && nRow <= MAXROW;
}

inline BOOL ValidColRowTab(USHORT nCol, USHORT nRow, USHORT nTab)
{
    return nCol <= MAXCOL && nRow <= MAXROW && nTab <= MAXTAB;
}

/*
#ifdef OS2
#define PIXEL_PER_INCH      72.0
#else
#define PIXEL_PER_INCH      96.0
#endif
*/

#define PIXEL_PER_INCH      96.0

#define CM_PER_INCH         2.54
#define POINTS_PER_INCH     72.27
#define PIXEL_PER_POINT     (PIXEL_PER_INCH / POINTS_PER_INCH)
#define INCHT_PER_CM        (1.0 / CM_PER_INCH)
#define POINTS_PER_CM       (POINTS_PER_INCH / CM_PER_INCH)
#define TWIPS_PER_POINT     20.0
#define TWIPS_PER_INCH      (TWIPS_PER_POINT * POINTS_PER_INCH)
#define TWIPS_PER_CM        (TWIPS_PER_INCH / CM_PER_INCH)
#define CM_PER_TWIPS        (CM_PER_INCH / TWIPS_PER_INCH)
#define TWIPS_PER_PIXEL     (TWIPS_PER_INCH / PIXEL_PER_INCH)
#define TWIPS_PER_CHAR      (TWIPS_PER_INCH / 13.6)
#define PIXEL_PER_TWIPS     (PIXEL_PER_INCH / TWIPS_PER_INCH)
#define HMM_PER_TWIPS       (CM_PER_TWIPS * 1000.0)

#define STD_COL_WIDTH       1285
#define STD_EXTRA_WIDTH     113     // 2mm Extra fuer optimale Breite
                                    // Standard Zeilenhoehe: Text + Rand - STD_ROWHEIGHT_DIFF


#define MAX_EXTRA_WIDTH     23811   // 42cm in TWIPS
#define MAX_EXTRA_HEIGHT    23811
#define MAX_COL_WIDTH       56693   // 1m in TWIPS
#define MAX_COL_HEIGHT      56693

#ifdef MAC
#define STD_ROWHEIGHT_DIFF  3
#else
#define STD_ROWHEIGHT_DIFF  23
#endif
#define STD_FONT_HEIGHT     200     // entspricht 10 Punkt

//!     statt STD_ROW_HEIGHT ScGlobal::nStdRowHeight benutzen !

#ifdef MAC                              // Mac braucht mehr Platz
#define STD_ROW_HEIGHT      (14.8 * TWIPS_PER_POINT)            //
#else
#define STD_ROW_HEIGHT      (12.8 * TWIPS_PER_POINT)            // 256 Twips, 0.45 cm
#endif

                                    // Standardgroesse als Ole-Server (Zellen)
#define OLE_STD_CELLS_X     4
#define OLE_STD_CELLS_Y     5

#define SC_SIZE_OPTIMUM     0xFFFF

                                    // Update-Flags
#define UF_SCROLL_LEFT      1
#define UF_SCROLL_RIGHT     2
#define UF_SCROLL_UP        4
#define UF_SCROLL_DOWN      8
#define UF_ROW              16
#define UF_VIEW             32

                                    // Repaint-Flags (fuer Messages)
#define PAINT_GRID          1
#define PAINT_TOP           2
#define PAINT_LEFT          4
#define PAINT_EXTRAS        8
#define PAINT_INVERT        16
#define PAINT_MARKS         32
#define PAINT_OBJECTS       64
#define PAINT_SIZE          128
#define PAINT_ALL           ( PAINT_GRID | PAINT_TOP | PAINT_LEFT | PAINT_EXTRAS | PAINT_OBJECTS | PAINT_SIZE )


                                    // Flags fuer Spalten / Zeilen
                                    // FILTERED immer zusammen mit HIDDEN
                                    // FILTERED und MANUALSIZE nur fuer Zeilen moeglich
#define CR_HIDDEN           1
//#define CR_MARKED         2
#define CR_PAGEBREAK        4
#define CR_MANUALBREAK      8
#define CR_FILTERED         16
#define CR_MANUALSIZE       32

//  was davon kommt in die Datei:
#define CR_SAVEMASK     ( ~CR_PAGEBREAK )

                                    // Insert-/Delete-Flags
#define IDF_VALUE           0x0001
#define IDF_DATETIME        0x0002
#define IDF_STRING          0x0004
#define IDF_NOTE            0x0008
#define IDF_FORMULA         0x0010
#define IDF_HARDATTR        0x0020
#define IDF_STYLES          0x0040
#define IDF_OBJECTS         0x0080
#define IDF_EDITATTR        0x0100
#define IDF_ATTRIB          ( IDF_HARDATTR | IDF_STYLES )
#define IDF_CONTENTS        ( IDF_VALUE | IDF_DATETIME | IDF_STRING | IDF_NOTE | IDF_FORMULA )
#define IDF_ALL             ( IDF_CONTENTS | IDF_ATTRIB | IDF_OBJECTS )
#define IDF_NONE            0x0000

#define PASTE_NOFUNC        0
#define PASTE_ADD           1
#define PASTE_SUB           2
#define PASTE_MUL           3
#define PASTE_DIV           4

#define PASTE_NONEMPTY      5

                                        // Bits fuer HasAttr
#define HASATTR_LINES           1
#define HASATTR_MERGED          2
#define HASATTR_OVERLAPPED      4
#define HASATTR_PROTECTED       8
#define HASATTR_SHADOW          16
#define HASATTR_NEEDHEIGHT      32
#define HASATTR_SHADOW_RIGHT    64
#define HASATTR_SHADOW_DOWN     128
#define HASATTR_AUTOFILTER      256
#define HASATTR_CONDITIONAL     512
#define HASATTR_ROTATE          1024
#define HASATTR_NOTOVERLAPPED   2048

#define HASATTR_PAINTEXT        ( HASATTR_LINES | HASATTR_SHADOW | HASATTR_CONDITIONAL )


#define EMPTY_STRING ScGlobal::GetEmptyString()

                                        //  Layer-ID's fuer Drawing
#define SC_LAYER_FRONT      0
#define SC_LAYER_BACK       1
#define SC_LAYER_INTERN     2
#define SC_LAYER_CONTROLS   3

                                        //  Tabellen linken
#define SC_LINK_NONE        0
#define SC_LINK_NORMAL      1
#define SC_LINK_VALUE       2

                                        //  Eingabe
#define SC_ENTER_NORMAL     0
#define SC_ENTER_BLOCK      1
#define SC_ENTER_MATRIX     2

                                        //  Step = 10pt, max. Einzug = 100 Schritte
#define SC_INDENT_STEP      200
#define SC_MAX_INDENT       20000

                                        //  Szenario-Flags
#define SC_SCENARIO_COPYALL     1
#define SC_SCENARIO_SHOWFRAME   2
#define SC_SCENARIO_PRINTFRAME  4
#define SC_SCENARIO_TWOWAY      8
#define SC_SCENARIO_ATTRIB      16
#define SC_SCENARIO_VALUE       32


#ifndef DELETEZ
#define DELETEZ(pPtr) { delete pPtr; pPtr = 0; }
#endif

                                    // Ist Bit in Set gesetzt?
#define IS_SET(bit,set)(((set)&(bit))==(bit))

#define MAX_FUNCCAT     12  // max. Kategorien fuer Funktionen
#define SEL_ALL         -1  // Eingabezeile: alles Selektieren
#define RES_CANCEL      0   // Resultate der Funk.AutoPilot-Seiten
#define RES_BACKWARD    1
#define RES_END         2

enum CellType
    {
        CELLTYPE_NONE,
        CELLTYPE_VALUE,
        CELLTYPE_STRING,
        CELLTYPE_FORMULA,
        CELLTYPE_NOTE,
        CELLTYPE_EDIT,
        CELLTYPE_SYMBOLS,       // fuer Laden/Speichern

        CELLTYPE_DESTROYED
    };

enum DelCellCmd
    {
        DEL_CELLSUP,
        DEL_CELLSLEFT,
        DEL_DELROWS,
        DEL_DELCOLS,
        DEL_NONE
    };

enum InsCellCmd
    {
        INS_CELLSDOWN,
        INS_CELLSRIGHT,
        INS_INSROWS,
        INS_INSCOLS,
        INS_NONE
    };

enum UpdateRefMode
    {
        URM_INSDEL,
        URM_COPY,
        URM_MOVE,
        URM_REORDER
    };

enum FillDir
    {
        FILL_TO_BOTTOM,
        FILL_TO_RIGHT,
        FILL_TO_TOP,
        FILL_TO_LEFT
    };

enum FillCmd
    {
        FILL_SIMPLE,
        FILL_LINEAR,
        FILL_GROWTH,
        FILL_DATE,
        FILL_AUTO
    };

enum FillDateCmd
    {
        FILL_DAY,
        FILL_WEEKDAY,
        FILL_MONTH,
        FILL_YEAR
    };

enum ScDirection
    {
        DIR_BOTTOM,
        DIR_RIGHT,
        DIR_TOP,
        DIR_LEFT
    };

enum ScSizeMode
    {
        SC_SIZE_DIRECT,             // setzen oder bei 0 ausblenden
        SC_SIZE_OPTIMAL,            // alles auf optimal
        SC_SIZE_SHOW,               // einblenden mit alter Hoehe
        SC_SIZE_VISOPT              // nur sichtbare auf optimal
    };

enum ScInputMode
    {
        SC_INPUT_NONE,
        SC_INPUT_TYPE,              // Eingabe, ohne im Inplace-Modus zu sein
        SC_INPUT_TABLE,             // Textcursor in der Tabelle
        SC_INPUT_TOP                // Textcursor in der Eingabezeile
    };

enum ScVObjMode                     // Ausgabemodi von Objekten auf einer Seite
{
    VOBJ_MODE_SHOW,
    VOBJ_MODE_HIDE,
    VOBJ_MODE_DUMMY
};

enum ScAnchorType                   // Verankerung eines Zeichenobjekts
{
    SCA_CELL,
    SCA_PAGE,
    SCA_DONTKNOW                    // bei Mehrfachselektion
};

enum ScGetDBMode
{
    SC_DB_MAKE,     // wenn noetig, "unbenannt" anlegen
    SC_DB_IMPORT,   // wenn noetig, "Importx" anlegen
    SC_DB_OLD       // nicht neu anlegen
};

enum ScLkUpdMode
{                   //Verknuepfungen
    LM_ALWAYS,      //immer aktualisieren
    LM_NEVER,       //niemals
    LM_ON_DEMAND,   //auf nachfrage
    LM_UNKNOWN      //Shit happens
};


// -----------------------------------------------------------------------

//==================================================================

// -----------------------------------------------------------------------

// enum with values equal to old DBObject enum from sdb
enum ScDBObject
{
    ScDbTable,
    ScDbQuery
};

struct ScImportParam
{
    USHORT          nCol1;
    USHORT          nRow1;
    USHORT          nCol2;
    USHORT          nRow2;
    BOOL            bImport;
    String          aDBName;                    // Alias der Datenbank
    String          aStatement;
    BOOL            bNative;
    BOOL            bSql;                       // Statement oder Name?
    BYTE            nType;                      // enum DBObject

    ScImportParam();
    ScImportParam( const ScImportParam& r );
    ~ScImportParam();

    ScImportParam&  operator=   ( const ScImportParam& r );
    BOOL            operator==  ( const ScImportParam& r ) const;
    void            Clear       ();
};

// -----------------------------------------------------------------------

class ScDocument;
class ScDocShell;
class ScDocShellRef;
class SvxSearchItem;
class ScAutoFormat;
class FuncCollection;
class ScUnoAddInCollection;
class ScUserList;
class SvxBrushItem;
class ScFunctionList;
class ScFunctionMgr;
class SfxItemPool;
class SdrModel;
class EditTextObject;
class SfxObjectShell;
class SvNumberFormatter;
class ScUnitConverter;
class CharClass;

namespace com { namespace sun { namespace star { namespace lang {
    struct Locale;
}}}}

#ifndef _SCALC_EXE
class ScGlobal
{
    static ScDocument*      pClipDoc;
    static SdrModel*        pClipModel;
    static BYTE             nClipState;
    static SvxSearchItem*   pSearchItem;
    static ScAutoFormat*    pAutoFormat;
    static FuncCollection*  pFuncCollection;
    static ScUnoAddInCollection* pAddInCollection;
    static ScUserList*      pUserList;
    static String**         ppRscString;
    static String*          pStrScDoc;
    static String*          pEmptyString;
    static String*          pStrClipDocName;
    static SvxBrushItem*    pEmptyBrushItem;
    static SvxBrushItem*    pButtonBrushItem;
    static SvxBrushItem*    pEmbeddedBrushItem;
    static SvxBrushItem*    pProtectedBrushItem;

    static ImageList*       pOutlineBitmaps;

//  static Bitmap*          pAnchorBitmap;
//  static Bitmap*          pGrayAnchorBitmap;

    static ScFunctionList*  pStarCalcFunctionList;
    static ScFunctionMgr*   pStarCalcFunctionMgr;

    static ScUnitConverter* pUnitConverter;

public:
    static International*       pScInternational;
    static ::com::sun::star::lang::Locale*      pLocale;
    static CharClass*           pCharClass;
    static LanguageType         eLnge;
    static sal_Unicode          cListDelimiter;
    static ScDocument*          GetClipDoc();
    static BOOL                 HasClipDoc();
    static const String&        GetClipDocName();
    static void                 SetClipDocName( const String& rNew );
    static void                 CaptureClip();
    static void                 ReleaseClip();
    static BOOL                 IsClipCaptured();
    static void                 SetClipDraw(SdrModel* pModel);
    static BOOL                 IsClipDraw();
    static SdrModel*            GetClipModel();
    static const SvxSearchItem& GetSearchItem();
    static void                 SetSearchItem( const SvxSearchItem& rNew );
    static ScAutoFormat*        GetAutoFormat();
    static void                 ClearAutoFormat(); //BugId 54209
    static FuncCollection*      GetFuncCollection();
    static ScUnoAddInCollection* GetAddInCollection();
    static ScUserList*          GetUserList();
    static void                 SetUserList( const ScUserList* pNewList );
    static const String&        GetRscString( USHORT nIndex );
    static void                 OpenURL( const String& rURL, const String& rTarget );
    static String               GetAbsDocName( const String& rFileName,
                                                SfxObjectShell* pShell );
    static String               GetDocTabName( const String& rFileName,
                                                const String& rTabName );
    static ULONG                GetStandardFormat( SvNumberFormatter&,
                                    ULONG nFormat, short nType );
    static ULONG                GetStandardFormat( double, SvNumberFormatter&,
                                    ULONG nFormat, short nType );

    static double               nScreenPPTX;
    static double               nScreenPPTY;

    static ScDocShellRef*   pDrawClipDocShellRef;

    static USHORT           nDefFontHeight;
    static USHORT           nStdRowHeight;

    static long             nLastRowHeightExtra;
    static long             nLastColWidthExtra;

    static void             Init();                     // am Anfang
    static void             InitAddIns();
    static void             Clear();                    // bei Programmende

    static void             InitTextHeight(SfxItemPool* pPool);
    static SvxBrushItem*    GetEmptyBrushItem() { return pEmptyBrushItem; }
    static SvxBrushItem*    GetButtonBrushItem()    { return pButtonBrushItem; }
    static SvxBrushItem*    GetEmbeddedBrushItem()  { return pEmbeddedBrushItem; }
    static SvxBrushItem*    GetProtectedBrushItem() { return pProtectedBrushItem; }
       static const String& GetEmptyString();
    static const String&    GetScDocString();

    static ImageList*       GetOutlineBitmaps();
//  static const Bitmap&    GetAnchorBitmap();
//  static const Bitmap&    GetGrayAnchorBitmap();

    static ScFunctionList*  GetStarCalcFunctionList();
    static ScFunctionMgr*   GetStarCalcFunctionMgr();

    static String           GetErrorString(USHORT nErrNumber);
    static String           GetLongErrorString(USHORT nErrNumber);
    static BOOL             EETextObjEqual( const EditTextObject* pObj1,
                                            const EditTextObject* pObj2 );
    static BOOL             CheckWidthInvalidate( BOOL& bNumFormatChanged,
                                                  const SfxItemSet& rNewAttrs,
                                                  const SfxItemSet& rOldAttrs );
    static BOOL             HasAttrChanged( const SfxItemSet& rNewAttrs,
                                            const SfxItemSet& rOldAttrs,
                                            const USHORT      nWhich );

    static ScUnitConverter* GetUnitConverter();

    /// strchr() functionality on unicode, as long as we need it for ScToken etc.
    static const sal_Unicode* UnicodeStrChr( const sal_Unicode* pStr, sal_Unicode c );

    static inline sal_Unicode ToUpperAlpha( sal_Unicode c )
        { return ( c >= 'a' && c <= 'z' ) ? ( c-'a'+'A' ) : c; }
};
#endif

class ScTripel
{
public:
        USHORT  nCol;
        USHORT  nRow;
        USHORT  nTab;

public:
        ScTripel();
        ScTripel( USHORT nNewCol, USHORT nNewRow, USHORT nNewTab );
        ScTripel( const ScTripel& rRef );
        ~ScTripel();

        USHORT  GetCol() const          { return nCol; }
        USHORT  GetRow() const          { return nRow; }
        USHORT  GetTab() const          { return nTab; }

        void    SetCol(USHORT nNewCol)  { nCol = nNewCol; }
        void    SetRow(USHORT nNewRow)  { nRow = nNewRow; }
        void    SetTab(USHORT nNewTab)  { nTab = nNewTab; }

        void    Put(USHORT nNewCol, USHORT nNewRow, USHORT nNewTab)
                { nCol=nNewCol; nRow=nNewRow; nTab=nNewTab; }

        inline  int operator == ( const ScTripel& rTripel ) const;
                int operator != ( const ScTripel& rTripel ) const { return !(operator==(rTripel)); }

        String  GetText() const;                                // "(1,2,3)"
        String  GetColRowString( BOOL bAbolute = FALSE ) const; // "A1"||"$A$1"
};

inline ScTripel::ScTripel() :
    nCol(0), nRow(0), nTab(0)
{}

inline ScTripel::ScTripel( USHORT nNewCol, USHORT nNewRow, USHORT nNewTab ) :
    nCol(nNewCol), nRow(nNewRow), nTab(nNewTab)
{}

inline ScTripel::ScTripel( const ScTripel& rRef ) :
    nCol(rRef.nCol), nRow(rRef.nRow), nTab(rRef.nTab)
{}

inline ScTripel::~ScTripel()
{}

inline int ScTripel::operator==( const ScTripel& rTripel ) const
{
    return nCol==rTripel.nCol && nRow==rTripel.nRow && nTab==rTripel.nTab;
}

class ScRefTripel : public ScTripel
{
    BOOL bRelCol;
    BOOL bRelRow;
    BOOL bRelTab;
public:
        ScRefTripel() :
            ScTripel(), bRelCol(FALSE), bRelRow(FALSE), bRelTab(FALSE)  {}
        ScRefTripel( USHORT nNewCol, USHORT nNewRow, USHORT nNewTab, BOOL bNewRelCol, BOOL bNewRelRow, BOOL bNewRelTab ) :
            ScTripel(nNewCol, nNewRow, nNewTab), bRelCol(bNewRelCol), bRelRow(bNewRelRow), bRelTab(bNewRelTab) {}
        ScRefTripel( const ScRefTripel& rRef ) :
            ScTripel(rRef.nCol, rRef.nRow, rRef.nTab), bRelCol(rRef.bRelCol), bRelRow(rRef.bRelRow), bRelTab(rRef.bRelTab) {}

        BOOL    GetRelCol() const { return bRelCol; }
        BOOL    GetRelRow() const { return bRelRow; }
        BOOL    GetRelTab() const { return bRelTab; }

        void    SetRelCol(BOOL bNewRelCol) { bRelCol = bNewRelCol; }
        void    SetRelRow(BOOL bNewRelRow) { bRelRow = bNewRelRow; }
        void    SetRelTab(BOOL bNewRelTab) { bRelTab = bNewRelTab; }

        void    Put(USHORT nNewCol, USHORT nNewRow, USHORT nNewTab, BOOL bNewRelCol, BOOL bNewRelRow, BOOL bNewRelTab )
                {   nCol = nNewCol; nRow = nNewRow; nTab = nNewTab;
                    bRelCol = bNewRelCol; bRelRow = bNewRelRow; bRelTab = bNewRelTab;}

        inline  int operator == ( const ScRefTripel& rRefTripel ) const;
                int operator != ( const ScRefTripel& rRefTripel ) const { return !(operator==(rRefTripel)); }

        String GetRefString(ScDocument* pDoc, USHORT nActTab) const;
};


inline int ScRefTripel::operator==( const ScRefTripel& rRefTripel ) const
{
    return nCol == rRefTripel.nCol && nRow==rRefTripel.nRow && nTab==rRefTripel.nTab &&
            bRelCol == rRefTripel.bRelCol && bRelRow == rRefTripel.bRelRow && bRelTab == rRefTripel.bRelTab;
}

//==================================================================

// Die neue Zelladressierung ist in einem UINT32 untergebracht:
// +---+---+-------+
// |Tab|Col|  Row  |
// +---+---+-------+
// Der Schnelligkeit halber wird beim Zugriff nicht geshiftet,
// sondern plattformabhaengig gecastet.

// Das Ergebnis von ConvertRef() ist eine Bitgruppe folgender Bits:

#define SCA_COL_ABSOLUTE    0x01
#define SCA_ROW_ABSOLUTE    0x02
#define SCA_TAB_ABSOLUTE    0x04
#define SCA_TAB_3D          0x08
#define SCA_COL2_ABSOLUTE   0x10
#define SCA_ROW2_ABSOLUTE   0x20
#define SCA_TAB2_ABSOLUTE   0x40
#define SCA_TAB2_3D         0x80
#define SCA_VALID_ROW       0x0100
#define SCA_VALID_COL       0x0200
#define SCA_VALID_TAB       0x0400
#define SCA_VALID_ROW2      0x1000
#define SCA_VALID_COL2      0x2000
#define SCA_VALID_TAB2      0x4000
#define SCA_VALID           0x8000

#define SCA_ABS               SCA_VALID \
                            | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define SCR_ABS               SCA_ABS \
                            | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define SCA_ABS_3D          SCA_ABS | SCA_TAB_3D
#define SCR_ABS_3D          SCR_ABS | SCA_TAB_3D

class ScAddress
{
private:
    UINT32 nAddress;
    inline UINT16* RowPos();
    inline BYTE* ColPos();
    inline BYTE* TabPos();
    inline const UINT16* RowPos() const;
    inline const BYTE* ColPos() const;
    inline const BYTE* TabPos() const;

public:
    inline ScAddress() { nAddress = 0; }
    inline ScAddress( USHORT nCol, USHORT nRow, USHORT nTab )
    { Set( nCol, nRow, nTab ); }
    inline ScAddress( const ScAddress& r ) { nAddress = r.nAddress; }
    inline ScAddress( UINT32 nNewAddress ) { nAddress = nNewAddress; }
    // TO BE REMOVED
    inline ScAddress( const ScTripel& r ) { Set( r.nCol, r.nRow, r.nTab ); }
    inline ScAddress& operator=( const ScAddress& r )
    { nAddress = r.nAddress; return *this; }
    inline void Set( USHORT nCol, USHORT nRow, USHORT nTab );
    inline USHORT Row() const { return *RowPos(); }
    inline USHORT Col() const { return *ColPos(); }
    inline USHORT Tab() const { return *TabPos(); }
    inline USHORT SetRow( USHORT nRow ) { return *RowPos() = nRow; }
    inline USHORT SetCol( USHORT nCol ) { return *ColPos() = (BYTE) nCol; }
    inline USHORT SetTab( USHORT nTab ) { return *TabPos() = (BYTE) nTab; }
    inline operator UINT32() const { return nAddress; }
    inline void PutInOrder( ScAddress& r );
    inline USHORT IncRow( short n=1 ) { return *RowPos() = (*RowPos() + n ) % (MAXROW+1); }
    inline USHORT IncCol( short n=1 ) { return *ColPos() = (BYTE) (*ColPos() + n ); }
    inline USHORT IncTab( short n=1 ) { return *TabPos() = (BYTE) (*TabPos() + n ); }
    inline void GetVars( USHORT& nCol, USHORT& nRow, USHORT& nTab ) const;
    USHORT Parse( const String&, ScDocument* = NULL );
    void Format( String&, USHORT = 0, ScDocument* = NULL ) const;
    // Das Doc fuer die maximal defineirte Tabelle
    BOOL Move( short dx, short dy, short dz, ScDocument* =NULL );
    inline int operator==( const ScAddress& r ) const;
    inline int operator!=( const ScAddress& r ) const;
    inline int operator<( const ScAddress& r ) const;
    inline int operator<=( const ScAddress& r ) const;
    inline int operator>( const ScAddress& r ) const;
    inline int operator>=( const ScAddress& r ) const;
    friend inline SvStream& operator<< ( SvStream& rStream, const ScAddress& rAdr );
    friend inline SvStream& operator>> ( SvStream& rStream, ScAddress& rAdr );
};

inline void ScAddress::Set( USHORT nCol, USHORT nRow, USHORT nTab )
{
//  nAddress = ((UINT32) ((((BYTE) nTab) << 8 ) + (BYTE) nCol ) << 16 )
//           | (UINT32) (UINT16) nRow;
// Shifterei: 5 mov, 2 xor, 2 shl, 1 add, 1 and, 1 or
// Casterei: 7 mov, ist weniger Code und schneller
    *ColPos() = (BYTE) nCol;
    *RowPos() = (UINT16) nRow;
    *TabPos() = (BYTE) nTab;
}

inline void ScAddress::PutInOrder( ScAddress& r )
{
    if( r.nAddress < nAddress )
    {
        UINT32 nTemp;
        nTemp = nAddress;
        nAddress = r.nAddress;
        r.nAddress = nTemp;
    }
    USHORT nTmp;
    if ( (nTmp = r.Col()) < Col() )
    {
        r.SetCol( Col() );
        SetCol( nTmp );
    }
    if ( (nTmp = r.Row()) < Row() )
    {
        r.SetRow( Row() );
        SetRow( nTmp );
    }
}

inline void ScAddress::GetVars( USHORT& nCol, USHORT& nRow, USHORT& nTab ) const
{
    nCol = Col();
    nRow = Row();
    nTab = Tab();
}

inline int ScAddress::operator==( const ScAddress& r ) const
{
    return ( nAddress == r.nAddress );
}

inline int ScAddress::operator!=( const ScAddress& r ) const
{
    return !operator==( r );
}

inline int ScAddress::operator<( const ScAddress& r ) const
{
    return ( nAddress < r.nAddress );
}

inline int ScAddress::operator<=( const ScAddress& r ) const
{
    return operator<( r ) || operator==( r );
}

inline int ScAddress::operator>( const ScAddress& r ) const
{
    return !operator<=( r );
}

inline int ScAddress::operator>=( const ScAddress& r ) const
{
    return !operator<( r );
}

#ifdef __LITTLEENDIAN

inline USHORT* ScAddress::RowPos() { return (USHORT*) &nAddress; }
inline BYTE*   ScAddress::ColPos() { return (BYTE*) &nAddress + 2; }
inline BYTE*   ScAddress::TabPos() { return (BYTE*) &nAddress + 3; }
inline const USHORT* ScAddress::RowPos() const { return (USHORT*) &nAddress; }
inline const BYTE*   ScAddress::ColPos() const { return (BYTE*) &nAddress + 2; }
inline const BYTE*   ScAddress::TabPos() const { return (BYTE*) &nAddress + 3; }

#else

inline USHORT* ScAddress::RowPos() { return (USHORT*) ((BYTE*) &nAddress + 2 ); }
inline BYTE*   ScAddress::ColPos() { return (BYTE*) &nAddress + 1; }
inline BYTE*   ScAddress::TabPos() { return (BYTE*) &nAddress; }
inline const USHORT* ScAddress::RowPos() const { return (USHORT*) ((BYTE*) &nAddress + 2 ); }
inline const BYTE*   ScAddress::ColPos() const { return (BYTE*) &nAddress + 1; }
inline const BYTE*   ScAddress::TabPos() const { return (BYTE*) &nAddress; }

#endif

inline SvStream& operator<< ( SvStream& rStream, const ScAddress& rAdr )
{
    rStream << rAdr.nAddress;
    return rStream;
}

inline SvStream& operator>> ( SvStream& rStream, ScAddress& rAdr )
{
    rStream >> rAdr.nAddress;
    return rStream;
}

class ScRange
{
public:
    ScAddress aStart, aEnd;
    ScRange() : aStart(), aEnd() {}
    ScRange( const ScAddress& s, const ScAddress& e )
        : aStart( s ), aEnd( e ) { aStart.PutInOrder( aEnd ); }
    ScRange( const ScRange& r ) : aStart( r.aStart ), aEnd( r.aEnd ) {}
    ScRange( const ScAddress& r ) : aStart( r ), aEnd( r ) {}
    ScRange( USHORT nCol, USHORT nRow, USHORT nTab )
        : aStart( nCol, nRow, nTab ), aEnd( aStart ) {}
    ScRange( USHORT nCol1, USHORT nRow1, USHORT nTab1,
             USHORT nCol2, USHORT nRow2, USHORT nTab2 )
        : aStart( nCol1, nRow1, nTab1 ), aEnd( nCol2, nRow2, nTab2 ) {}
    // TO BE REMOVED
    ScRange( const ScTripel& r )
        : aStart( r.nCol, r.nRow, r.nTab ), aEnd( aStart ) {}
    ScRange( const ScTripel& r1, const ScTripel& r2 )
        : aStart( r1.nCol, r1.nRow, r1.nTab ),
          aEnd( r2.nCol, r2.nRow, r2.nTab ) {}

    ScRange& operator=( const ScRange& r )
    { aStart = r.aStart; aEnd = r.aEnd; return *this; }
    inline BOOL In( const ScAddress& ) const;   // ist Address& in Range?
    inline BOOL In( const ScRange& ) const;     // ist Range& in Range?
    USHORT Parse( const String&, ScDocument* = NULL );
    USHORT ParseAny( const String&, ScDocument* = NULL );
    inline void GetVars( USHORT& nCol1, USHORT& nRow1, USHORT& nTab1,
        USHORT& nCol2, USHORT& nRow2, USHORT& nTab2 ) const;
    void Format( String&, USHORT = 0, ScDocument* = NULL ) const;
    // Das Doc fuer die maximal definierte Tabelle
    BOOL Move( short dx, short dy, short dz, ScDocument* =NULL );
    void Justify();
    void ExtendOne();
    BOOL Intersects( const ScRange& ) const;    // ueberschneiden sich zwei Ranges?
    inline int operator==( const ScRange& r ) const;
    inline int operator!=( const ScRange& r ) const;
    inline int operator<( const ScRange& r ) const;
    inline int operator<=( const ScRange& r ) const;
    inline int operator>( const ScRange& r ) const;
    inline int operator>=( const ScRange& r ) const;
    friend inline SvStream& operator<< ( SvStream& rStream, const ScRange& rRange );
    friend inline SvStream& operator>> ( SvStream& rStream, ScRange& rRange );
};

inline void ScRange::GetVars( USHORT& nCol1, USHORT& nRow1, USHORT& nTab1,
        USHORT& nCol2, USHORT& nRow2, USHORT& nTab2 ) const
{
    aStart.GetVars( nCol1, nRow1, nTab1 );
    aEnd.GetVars( nCol2, nRow2, nTab2 );
}

inline int ScRange::operator==( const ScRange& r ) const
{
    return ( (aStart == r.aStart) && (aEnd == r.aEnd) );
}

inline int ScRange::operator!=( const ScRange& r ) const
{
    return !operator==( r );
}

// Sortierung auf linke obere Ecke, wenn die gleich dann auch rechte untere
inline int ScRange::operator<( const ScRange& r ) const
{
    return aStart < r.aStart || (aStart == r.aStart && aEnd < r.aEnd) ;
}

inline int ScRange::operator<=( const ScRange& r ) const
{
    return operator<( r ) || operator==( r );
}

inline int ScRange::operator>( const ScRange& r ) const
{
    return !operator<=( r );
}

inline int ScRange::operator>=( const ScRange& r ) const
{
    return !operator<( r );
}

// damit das inlinig funkt die Address reference lassen!
inline BOOL ScRange::In( const ScAddress& rAddr ) const
{
    return
        aStart.Col() <= rAddr.Col() && rAddr.Col() <= aEnd.Col() &&
        aStart.Row() <= rAddr.Row() && rAddr.Row() <= aEnd.Row() &&
        aStart.Tab() <= rAddr.Tab() && rAddr.Tab() <= aEnd.Tab();
}

inline BOOL ScRange::In( const ScRange& r ) const
{
    return
        aStart.Col() <= r.aStart.Col() && r.aEnd.Col() <= aEnd.Col() &&
        aStart.Row() <= r.aStart.Row() && r.aEnd.Row() <= aEnd.Row() &&
        aStart.Tab() <= r.aStart.Tab() && r.aEnd.Tab() <= aEnd.Tab();
}

inline SvStream& operator<< ( SvStream& rStream, const ScRange& rRange )
{
    rStream << rRange.aStart;
    rStream << rRange.aEnd;
    return rStream;
}

inline SvStream& operator>> ( SvStream& rStream, ScRange& rRange )
{
    rStream >> rRange.aStart;
    rStream >> rRange.aEnd;
    return rStream;
}


class ScRangePair
{
private:
    ScRange aRange[2];

public:
    ScRangePair() {}
    ScRangePair( const ScRangePair& r )
        { aRange[0] = r.aRange[0]; aRange[1] = r.aRange[1]; }
    ScRangePair( const ScRange& r1, const ScRange& r2 )
        {  aRange[0] = r1; aRange[1] = r2; }

    inline ScRangePair& operator= ( const ScRangePair& r );
    const ScRange&      GetRange( USHORT n ) const { return aRange[n]; }
    ScRange&            GetRange( USHORT n ) { return aRange[n]; }
    inline int operator==( const ScRangePair& ) const;
    inline int operator!=( const ScRangePair& ) const;
    friend inline SvStream& operator<< ( SvStream&, const ScRangePair& );
    friend inline SvStream& operator>> ( SvStream&, ScRange& );
};

inline ScRangePair& ScRangePair::operator= ( const ScRangePair& r )
{
    aRange[0] = r.aRange[0];
    aRange[1] = r.aRange[1];
    return *this;
}

inline int ScRangePair::operator==( const ScRangePair& r ) const
{
    return ( (aRange[0] == r.aRange[0]) && (aRange[1] == r.aRange[1]) );
}

inline int ScRangePair::operator!=( const ScRangePair& r ) const
{
    return !operator==( r );
}

inline SvStream& operator<< ( SvStream& rStream, const ScRangePair& rPair )
{
    rStream << rPair.GetRange(0);
    rStream << rPair.GetRange(1);
    return rStream;
}

inline SvStream& operator>> ( SvStream& rStream, ScRangePair& rPair )
{
    rStream >> rPair.GetRange(0);
    rStream >> rPair.GetRange(1);
    return rStream;
}

//==================================================================

inline void PutInOrder( USHORT& nStart, USHORT& nEnd )
{
    if (nEnd < nStart)
    {
        USHORT nTemp;
        nTemp = nEnd;
        nEnd = nStart;
        nStart = nTemp;
    }
}

//===================================================================
// Globale Funktionen
//===================================================================

BOOL    ConvertSingleRef(ScDocument* pDoc, const String& rRefString, USHORT nDefTab, ScRefTripel& rRefTripel);
BOOL    ConvertDoubleRef(ScDocument* pDoc, const String& rRefString, USHORT nDefTab, ScRefTripel& rStartRefTripel, ScRefTripel& rEndRefTripel);
String  ColToAlpha( const USHORT nCol );
CharSet GetCharsetValue(const String& rCharSet);

//===================================================================
// Funktionsautopilot: Klassen zur Verwaltung der StarCalc-Funktionen
//===================================================================

class ScFuncDesc
{
public:
                ScFuncDesc();
                ~ScFuncDesc();

    String  GetSignature        () const;
    String  GetFormulaString    ( String** aArgArr ) const;

    USHORT      nFIndex;        // eindeutiger Funktionsindex
    USHORT      nCategory;      // Kategorie
    String*     pFuncName;      // Funktionsname
    String*     pFuncDesc;      // Funktionsbeschreibung
    USHORT      nArgCount;      // Parameteranzahl
    String**    aDefArgNames;   // Parametername(n)
    String**    aDefArgDescs;   // Parameterbeschreibung(en)
    BOOL*       aDefArgOpt;     // Flags ob Parameter optional ist
    USHORT      nHelpId;        // HilfeId der Funktion
};

//==================================================================

class ScFunctionMgr
{
public:
                ScFunctionMgr();
                ~ScFunctionMgr();

    ScFuncDesc*     Get( const String& rFName );
    ScFuncDesc*     Get( USHORT nFIndex );
    ScFuncDesc*     First( USHORT nCategory = 0 );
    ScFuncDesc*     Next();

private:
    ScFunctionList* pFuncList;
    List*           aCatLists[MAX_FUNCCAT];
    List*           pCurCatList;
};

//==================================================================

class ScFunctionList
{
public:
    ScFunctionList();
    ~ScFunctionList();

    ULONG           GetCount() const
                    { return aFunctionList.Count(); }

    ScFuncDesc*     GetFunction( ULONG nIndex ) const
                    { return (ScFuncDesc*)aFunctionList.GetObject( nIndex ); }

    xub_StrLen      GetMaxFuncNameLen() const
                    { return nMaxFuncNameLen; }

private:
    List        aFunctionList;
    xub_StrLen  nMaxFuncNameLen;
};

//==================================================================

#define FUNC_NOTFOUND 0xffff

class ScFormulaUtil
{
public:
    static BOOL                 GetNextFunc( const String&  rFormula,
                                             BOOL           bBack,
                                             xub_StrLen&    rFStart, // Ein- und Ausgabe
                                             xub_StrLen*    pFEnd = NULL,
                                             ScFuncDesc**   ppFDesc = NULL,
                                             String***      pppArgs = NULL );

    static const ScFuncDesc*    GetDefaultFuncDesc();

    static xub_StrLen           GetFunctionStart( const String& rFormula, xub_StrLen nStart,
                                                    BOOL bBack, String* pFuncName = NULL );

    static xub_StrLen           GetFunctionEnd  ( const String& rFormula, xub_StrLen nStart );

    static xub_StrLen           GetArgStart     ( const String& rFormula, xub_StrLen nStart,
                                                  USHORT nArg );

    static String**             GetArgStrings   ( const String& rFormula,
                                                  xub_StrLen    nFuncPos,
                                                  USHORT        nArgs );

    static void                 FillArgStrings  ( const String& rFormula,
                                                  xub_StrLen    nFuncPos,
                                                  USHORT        nArgs,
                                                  String**      pArgs );
};

//==================================================================
// Notiz
//==================================================================

class ScPostIt
{
private:
    String  aStrText;
    String  aStrDate;
    String  aStrAuthor;
    BOOL    bShown;

public:
        ScPostIt();
        ScPostIt( const String& rText );
        ScPostIt( const String& rText, const String& rDate, const String& rAuthor );
        ScPostIt( const ScPostIt& rCpy );
        ~ScPostIt();

    const String&           GetText() const     { return aStrText;   }
    const String&           GetDate() const     { return aStrDate;   }
    const String&           GetAuthor() const   { return aStrAuthor; }
    BOOL                    IsShown() const     { return bShown;     }

    void                    SetText( const String& rNew )   { aStrText   = rNew; }
    void                    SetDate( const String& rNew )   { aStrDate   = rNew; }
    void                    SetAuthor( const String& rNew ) { aStrAuthor = rNew; }
    void                    SetShown( BOOL bNew )           { bShown     = bNew; }

    void                    Clear() { aStrText.Erase(); aStrDate.Erase();
                                      aStrAuthor.Erase(); bShown = FALSE; }
    void                    AutoSetText( const String& rNewText );

    inline const ScPostIt&  operator= ( const ScPostIt& rCpy );

    inline int              operator==( const ScPostIt& rPostIt ) const;
    int                     operator!=( const ScPostIt& rPostIt ) const { return !(operator==(rPostIt)); }

    friend inline SvStream&     operator>>( SvStream& rStream, ScPostIt& rPostIt );
    friend inline SvStream&     operator<<( SvStream& rStream, const ScPostIt& rPostIt );
};

inline const ScPostIt& ScPostIt::operator=( const ScPostIt& rCpy )
{
    aStrText    = rCpy.aStrText;
    aStrDate    = rCpy.aStrDate;
    aStrAuthor  = rCpy.aStrAuthor;
    bShown      = rCpy.bShown;

    return *this;
}

inline int ScPostIt::operator==( const ScPostIt& rPostIt ) const
{
    return (    aStrText    == rPostIt.aStrText
            &&  aStrDate    == rPostIt.aStrDate
            &&  aStrAuthor  == rPostIt.aStrAuthor
            &&  bShown      == rPostIt.bShown );
}

inline SvStream& operator>>( SvStream& rStream, ScPostIt& rPostIt )
{
    //  ohne bShown !!!

    CharSet eSet = rStream.GetStreamCharSet();
    rStream.ReadByteString( rPostIt.aStrText, eSet );
    rStream.ReadByteString( rPostIt.aStrDate, eSet );
    rStream.ReadByteString( rPostIt.aStrAuthor, eSet );

    return rStream;
}

inline SvStream& operator<<( SvStream& rStream, const ScPostIt& rPostIt )
{
    //  ohne bShown !!!

    CharSet eSet = rStream.GetStreamCharSet();
    rStream.WriteByteString( rPostIt.aStrText, eSet );
    rStream.WriteByteString( rPostIt.aStrDate, eSet );
    rStream.WriteByteString( rPostIt.aStrAuthor, eSet );

    return rStream;
}

//==================================================================
// evtl. in dbdata.hxx auslagern (?):

enum ScQueryOp
    {
        SC_EQUAL,
        SC_LESS,
        SC_GREATER,
        SC_LESS_EQUAL,
        SC_GREATER_EQUAL,
        SC_NOT_EQUAL,
        SC_TOPVAL,
        SC_BOTVAL,
        SC_TOPPERC,
        SC_BOTPERC
    };

// -----------------------------------------------------------------------

enum ScQueryConnect
    {
        SC_AND,
        SC_OR
    };

// -----------------------------------------------------------------------

enum ScSubTotalFunc
    {
        SUBTOTAL_FUNC_NONE  = 0,
        SUBTOTAL_FUNC_AVE   = 1,
        SUBTOTAL_FUNC_CNT   = 2,
        SUBTOTAL_FUNC_CNT2  = 3,
        SUBTOTAL_FUNC_MAX   = 4,
        SUBTOTAL_FUNC_MIN   = 5,
        SUBTOTAL_FUNC_PROD  = 6,
        SUBTOTAL_FUNC_STD   = 7,
        SUBTOTAL_FUNC_STDP  = 8,
        SUBTOTAL_FUNC_SUM   = 9,
        SUBTOTAL_FUNC_VAR   = 10,
        SUBTOTAL_FUNC_VARP  = 11
    };


#define     PIVOT_MAXFUNC           11
#define     PIVOT_FUNC_NONE         0x0000
#define     PIVOT_FUNC_SUM          0x0001
#define     PIVOT_FUNC_COUNT        0x0002
#define     PIVOT_FUNC_AVERAGE      0x0004
#define     PIVOT_FUNC_MAX          0x0008
#define     PIVOT_FUNC_MIN          0x0010
#define     PIVOT_FUNC_PRODUCT      0x0020
#define     PIVOT_FUNC_COUNT_NUM    0x0040
#define     PIVOT_FUNC_STD_DEV      0x0080
#define     PIVOT_FUNC_STD_DEVP     0x0100
#define     PIVOT_FUNC_STD_VAR      0x0200
#define     PIVOT_FUNC_STD_VARP     0x0400
#define     PIVOT_FUNC_AUTO         0x1000

// -----------------------------------------------------------------------

struct ScSubTotalParam;
struct ScQueryParam;

// -----------------------------------------------------------------------

struct ScSortParam
{
    USHORT      nCol1;
    USHORT      nRow1;
    USHORT      nCol2;
    USHORT      nRow2;
    BOOL        bHasHeader;
    BOOL        bByRow;
    BOOL        bCaseSens;
    BOOL        bUserDef;
    USHORT      nUserIndex;
    BOOL        bIncludePattern;
    BOOL        bInplace;
    USHORT      nDestTab;
    USHORT      nDestCol;
    USHORT      nDestRow;
    BOOL        bDoSort[MAXSORT];
    USHORT      nField[MAXSORT];
    BOOL        bAscending[MAXSORT];

    ScSortParam();
    ScSortParam( const ScSortParam& r );
    // zum Sortieren von Zwischenergebnissen:
    ScSortParam( const ScSubTotalParam& rSub, const ScSortParam& rOld );
    ScSortParam( const ScQueryParam&, USHORT nCol );    // TopTen Sortierung

    ScSortParam&    operator=   ( const ScSortParam& r );
    BOOL            operator==  ( const ScSortParam& rOther ) const;
    void            Clear       ();

    void            MoveToDest();
};

// -----------------------------------------------------------------------

/*
 * Dialog liefert die ausgezeichneten Feldwerte "leer"/"nicht leer"
 * als Konstanten SC_EMPTYFIELDS bzw. SC_NONEMPTYFIELDS in nVal in
 * Verbindung mit dem Schalter bQueryByString auf FALSE.
 */

#define SC_EMPTYFIELDS      ((double)0x0042)
#define SC_NONEMPTYFIELDS   ((double)0x0043)

namespace utl
{
    class SearchParam;
    class TextSearch;
}

struct ScQueryEntry
{
    BOOL            bDoQuery;
    BOOL            bQueryByString;
    USHORT          nField;
    ScQueryOp       eOp;
    ScQueryConnect  eConnect;
    String*         pStr;
    double          nVal;
    utl::SearchParam*   pSearchParam;       // falls RegExp, nicht gespeichert
    utl::TextSearch*    pSearchText;        // falls RegExp, nicht gespeichert

    ScQueryEntry();
    ScQueryEntry(const ScQueryEntry& r);
    ~ScQueryEntry();

    // legt ggbf. pSearchParam und pSearchText an, immer RegExp!
    utl::TextSearch*    GetSearchTextPtr( BOOL bCaseSens );

    void            Clear();
    ScQueryEntry&   operator=( const ScQueryEntry& r );
    BOOL            operator==( const ScQueryEntry& r ) const;

    void            Load(SvStream& rStream);
    void            Store(SvStream& rStream) const;
};

struct ScQueryParam
{
    USHORT          nCol1;
    USHORT          nRow1;
    USHORT          nCol2;
    USHORT          nRow2;
    USHORT          nTab;
    BOOL            bHasHeader;
    BOOL            bByRow;
    BOOL            bInplace;
    BOOL            bCaseSens;
    BOOL            bRegExp;
    BOOL            bDuplicate;
    BOOL            bDestPers;          // nicht gespeichert
    USHORT          nDestTab;
    USHORT          nDestCol;
    USHORT          nDestRow;

private:
    USHORT          nEntryCount;
    ScQueryEntry*   pEntries;

public:
    ScQueryParam();
    ScQueryParam( const ScQueryParam& r );
    ~ScQueryParam();

    USHORT          GetEntryCount() const           { return nEntryCount; }
    ScQueryEntry&   GetEntry(USHORT n) const        { return pEntries[n]; }
    void            Resize(USHORT nNew);

    ScQueryParam&   operator=   ( const ScQueryParam& r );
    BOOL            operator==  ( const ScQueryParam& rOther ) const;
    void            Clear       ();
    void            DeleteQuery( USHORT nPos );

    void            MoveToDest();
    void            FillInExcelSyntax(String& aCellStr, USHORT nIndex);

    void            Load(SvStream& rStream);
    void            Store(SvStream& rStream) const;
};

// -----------------------------------------------------------------------

struct ScSubTotalParam
{
    USHORT          nCol1;          // Selektierter Bereich
    USHORT          nRow1;
    USHORT          nCol2;
    USHORT          nRow2;
    BOOL            bRemoveOnly;
    BOOL            bReplace;                   // vorhandene Ergebnisse ersetzen
    BOOL            bPagebreak;                 // Seitenumbruch bei Gruppenwechsel
    BOOL            bCaseSens;                  // Gross-/Kleinschreibung
    BOOL            bDoSort;                    // vorher sortieren
    BOOL            bAscending;                 // aufsteigend sortieren
    BOOL            bUserDef;                   // Benutzer-def. Sort.Reihenfolge
    USHORT          nUserIndex;                 // Index auf Liste
    BOOL            bIncludePattern;            // Formate mit sortieren
    BOOL            bGroupActive[MAXSUBTOTAL];  // aktive Gruppen
    USHORT          nField[MAXSUBTOTAL];        // zugehoeriges Feld
    USHORT          nSubTotals[MAXSUBTOTAL];    // Anzahl der SubTotals
    USHORT*         pSubTotals[MAXSUBTOTAL];    // Array der zu berechnenden Spalten
    ScSubTotalFunc* pFunctions[MAXSUBTOTAL];    // Array der zugehoerige Funktionen

    ScSubTotalParam();
    ScSubTotalParam( const ScSubTotalParam& r );

    ScSubTotalParam&    operator=       ( const ScSubTotalParam& r );
    BOOL                operator==      ( const ScSubTotalParam& r ) const;
    void                Clear           ();
    void                SetSubTotals    ( USHORT                nGroup,
                                          const USHORT*         ptrSubTotals,
                                          const ScSubTotalFunc* ptrFuncions,
                                          USHORT                nCount );
};

// -----------------------------------------------------------------------
class ScArea;

struct ScConsolidateParam
{
    USHORT          nCol;                   // Cursor Position /
    USHORT          nRow;                   // bzw. Anfang des Zielbereiches
    USHORT          nTab;
    ScSubTotalFunc  eFunction;              // Berechnungsvorschrift
    USHORT          nDataAreaCount;         // Anzahl der Datenbereiche
    ScArea**        ppDataAreas;            // Zeiger-Array auf Datenbereiche
    BOOL            bByCol;                 // nach Spalten
    BOOL            bByRow;                 // nach Zeilen
    BOOL            bReferenceData;         // Quelldaten referenzieren

    ScConsolidateParam();
    ScConsolidateParam( const ScConsolidateParam& r );
    ~ScConsolidateParam();

    ScConsolidateParam& operator=       ( const ScConsolidateParam& r );
    BOOL                operator==      ( const ScConsolidateParam& r ) const;
    void                Clear           (); // = ClearDataAreas()+Members
    void                ClearDataAreas  ();
    void                SetAreas        ( ScArea* const* ppAreas, USHORT nCount );

    void            Load( SvStream& rStream );
    void            Store( SvStream& rStream ) const;
};

// -----------------------------------------------------------------------
struct PivotField
{
    short   nCol;
    USHORT  nFuncMask;
    USHORT  nFuncCount;

    PivotField() :
        nCol(0),nFuncMask(0),nFuncCount(0) {}

    PivotField( const PivotField& rCpy ) :
        nCol(rCpy.nCol),nFuncMask(rCpy.nFuncMask),nFuncCount(rCpy.nFuncCount) {}

    PivotField(short nNewCol, USHORT nNewFuncMask = 0) :
        nCol(nNewCol),nFuncMask(nNewFuncMask),nFuncCount(0) {}

    PivotField  operator = (const PivotField& r)
                {
                    nCol        = r.nCol;
                    nFuncMask   = r.nFuncMask;
                    nFuncCount  = r.nFuncCount;
                    return *this;
                }

    BOOL        operator == (const PivotField& r) const
                {
                    return (   (nCol == r.nCol)
                            && (nFuncMask == r.nFuncMask)
                            && (nFuncCount == r.nFuncCount));
                }
};

// -----------------------------------------------------------------------
struct ScPivotParam
{
    USHORT          nCol;           // Cursor Position /
    USHORT          nRow;           // bzw. Anfang des Zielbereiches
    USHORT          nTab;
    LabelData**     ppLabelArr;
    USHORT          nLabels;
    PivotField      aColArr[PIVOT_MAXFIELD];
    PivotField      aRowArr[PIVOT_MAXFIELD];
    PivotField      aDataArr[PIVOT_MAXFIELD];
    USHORT          nColCount;
    USHORT          nRowCount;
    USHORT          nDataCount;
    BOOL            bIgnoreEmptyRows;
    BOOL            bDetectCategories;
    BOOL            bMakeTotalCol;
    BOOL            bMakeTotalRow;

    ScPivotParam();
    ScPivotParam( const ScPivotParam& r );
    ~ScPivotParam();

    ScPivotParam&   operator=       ( const ScPivotParam& r );
    BOOL            operator==      ( const ScPivotParam& r ) const;
    void            Clear           ();
    void            ClearLabelData  ();
    void            ClearPivotArrays();
    void            SetLabelData    ( LabelData**   ppLabArr,
                                      USHORT        nLab );
    void            SetPivotArrays  ( const PivotField* pColArr,
                                      const PivotField* pRowArr,
                                      const PivotField* pDataArr,
                                      USHORT            nColCnt,
                                      USHORT            nRowCnt,
                                      USHORT            nDataCnt );
};


//-----------------------------------------------------------------------

struct ScSolveParam
{
    ScAddress   aRefFormulaCell;
    ScAddress   aRefVariableCell;
    String*     pStrTargetVal;

    ScSolveParam();
    ScSolveParam( const ScSolveParam& r );
    ScSolveParam( const ScAddress&  rFormulaCell,
                  const ScAddress&  rVariableCell,
                  const String& rTargetValStr );
    ~ScSolveParam();

    ScSolveParam&   operator=   ( const ScSolveParam& r );
    BOOL            operator==  ( const ScSolveParam& r ) const;
};

struct ScTabOpParam
{
    ScRefTripel     aRefFormulaCell;
    ScRefTripel     aRefFormulaEnd;
    ScRefTripel     aRefRowCell;
    ScRefTripel     aRefColCell;
    BYTE            nMode;

    ScTabOpParam() {};
    ScTabOpParam( const ScTabOpParam& r );
    ScTabOpParam( const ScRefTripel& rFormulaCell,
                  const ScRefTripel& rFormulaEnd,
                  const ScRefTripel& rRowCell,
                  const ScRefTripel& rColCell,
                        BYTE         nMd);
    ~ScTabOpParam() {};

    ScTabOpParam&   operator=       ( const ScTabOpParam& r );
    BOOL            operator==      ( const ScTabOpParam& r ) const;
};

#endif
