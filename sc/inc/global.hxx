/*************************************************************************
 *
 *  $RCSfile: global.hxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 14:28:24 $
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

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _LANG_HXX //autogen
#include <tools/lang.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

class ImageList;
class Bitmap;
class SfxItemSet;
class Color;

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

#if 0
// I18N doesn't get this right, can't specify more than one to ignore
#define SC_COLLATOR_IGNORES ( \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE | \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_KANA | \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_WIDTH )
#else
#define SC_COLLATOR_IGNORES ( \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE )
#endif
#if 0
// #107998# Don't ignore Width and Kana. The issue was mainly with AutoInput,
// but affects also comparison of names in general.
#define SC_TRANSLITERATION_IGNORECASE ( \
    ::com::sun::star::i18n::TransliterationModules_IGNORE_CASE | \
    ::com::sun::star::i18n::TransliterationModules_IGNORE_KANA | \
    ::com::sun::star::i18n::TransliterationModules_IGNORE_WIDTH )
#define SC_TRANSLITERATION_CASESENSE ( \
    ::com::sun::star::i18n::TransliterationModules_IGNORE_KANA | \
    ::com::sun::star::i18n::TransliterationModules_IGNORE_WIDTH )
#else
#define SC_TRANSLITERATION_IGNORECASE ( \
    ::com::sun::star::i18n::TransliterationModules_IGNORE_CASE )
#define SC_TRANSLITERATION_CASESENSE 0
#endif


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

#define MINZOOM     20
#define MAXZOOM     400

#ifdef SC_ROWLIMIT_TYPECONVERSION_NOCONVPASS
const size_t MAXSUBTOTAL        = 3;
const size_t MAXQUERY           = 8;
const size_t PIVOT_MAXFIELD     = 8;
const size_t PIVOT_MAXPAGEFIELD = 10;
#else
const SCSIZE MAXSUBTOTAL        = 3;
const SCSIZE MAXQUERY           = 8;
const SCSIZE PIVOT_MAXFIELD     = 8;
const SCSIZE PIVOT_MAXPAGEFIELD = 10;
#endif

#define SC_START_INDEX_DB_COLL 50000
                                        // Oberhalb dieser Grenze liegen
                                        // die Indizes fuer DBBereiche

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
#define HASATTR_RTL             4096
#define HASATTR_RIGHTORCENTER   8192    // right or centered logical alignment

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
#define SC_SCENARIO_PROTECT     64


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
        SC_SIZE_DIRECT,             // set size or hide if value is 0
        SC_SIZE_OPTIMAL,            // set optimal size for everything
        SC_SIZE_SHOW,               // show with original size
        SC_SIZE_VISOPT,             // set optimal size only if visible
        SC_SIZE_ORIGINAL            // only set size, don't change visible flag
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
    SCCOL           nCol1;
    SCROW           nRow1;
    SCCOL           nCol2;
    SCROW           nRow2;
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
class LocaleDataWrapper;
class SvtSysLocale;
class CalendarWrapper;
class CollatorWrapper;
class IntlWrapper;
class OutputDevice;

namespace com { namespace sun { namespace star { namespace lang {
    struct Locale;
}}}}
namespace utl {
    class TransliterationWrapper;
}

#ifndef _SCALC_EXE
class ScGlobal
{
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
    static ImageList*       pOutlineBitmapsHC;

//  static Bitmap*          pAnchorBitmap;
//  static Bitmap*          pGrayAnchorBitmap;

    static ScFunctionList*  pStarCalcFunctionList;
    static ScFunctionMgr*   pStarCalcFunctionMgr;

    static ScUnitConverter* pUnitConverter;

    static  SvNumberFormatter*  pEnglishFormatter;          // for UNO / XML export

public:
    static ::com::sun::star::lang::Locale*      pLocale;
    static SvtSysLocale*        pSysLocale;
    // for faster access a pointer to the single instance provided by SvtSysLocale
    static const CharClass*     pCharClass;
    // for faster access a pointer to the single instance provided by SvtSysLocale
    static const LocaleDataWrapper* pLocaleData;
    static CalendarWrapper*     pCalendar;
    static CollatorWrapper*     pCollator;
    static CollatorWrapper*     pCaseCollator;
    static ::utl::TransliterationWrapper* pTransliteration;
    static ::utl::TransliterationWrapper* pCaseTransliteration;
    static IntlWrapper*         pScIntlWrapper;
    static LanguageType         eLnge;
    static sal_Unicode          cListDelimiter;
    static const String&        GetClipDocName();
    static void                 SetClipDocName( const String& rNew );
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

    static void             UpdatePPT(OutputDevice* pDev);

    static void             InitTextHeight(SfxItemPool* pPool);
    static SvxBrushItem*    GetEmptyBrushItem() { return pEmptyBrushItem; }
    static SvxBrushItem*    GetButtonBrushItem();
    static SvxBrushItem*    GetEmbeddedBrushItem()  { return pEmbeddedBrushItem; }
    static SvxBrushItem*    GetProtectedBrushItem() { return pProtectedBrushItem; }
       static const String& GetEmptyString();
    static const String&    GetScDocString();

    /** Returns the specified image list with outline symbols.
        @param bHC  false = standard symbols; true = high contrast symbols. */
    static ImageList*       GetOutlineSymbols( bool bHC );

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

    /** Adds the string rToken to rTokenList, using a list separator character.
        @param rTokenList  The string list where the token will be appended to.
        @param rToken  The token string to append to the token list.
        @param cSep  The character to separate the tokens.
        @param nSepCount  Specifies how often cSep is inserted between two tokens.
        @param bForceSep  true = Always insert separator; false = Only, if not at begin or end. */
    static void             AddToken(
                                String& rTokenList, const String& rToken,
                                sal_Unicode cSep, xub_StrLen nSepCount = 1,
                                bool bForceSep = false );

    /** Returns true, if the first and last character of the string is cQuote. */
    static bool             IsQuoted( const String& rString, sal_Unicode cQuote = '"' );
    /** Inserts the character cQuote at beginning and end of rString. */
    static void             AddQuotes( String& rString, sal_Unicode cQuote = '"' );
    /** Erases the character cQuote from rString, if it exists at beginning AND end. */
    static void             EraseQuotes( String& rString, sal_Unicode cQuote = '"' );


    static  CharSet         GetCharsetValue( const String& rCharSet );
    static  String          GetCharsetString( CharSet eVal );

    /// a "ReadOnly" formatter for UNO/XML export
    static  SvNumberFormatter*  GetEnglishFormatter();

    static BOOL IsSystemRTL();                      // depending on system language
    static LanguageType GetEditDefaultLanguage();   // for EditEngine::SetDefaultLanguage
    static BYTE GetDefaultScriptType();             // for all WEAK characters
    /** Map ATTR_((CJK|CTL)_)?FONT_... to proper WhichIDs.
        If more than one SCRIPTTYPE_... values are or'ed together, prefers
        first COMPLEX, then ASIAN */
    static USHORT GetScriptedWhichID( BYTE nScriptType, USHORT nWhich );

    /** Adds a language item to the item set, if the number format item contains
        a language that differs from its parent's language. */
    static void             AddLanguage( SfxItemSet& rSet, SvNumberFormatter& rFormatter );
};
#endif

//==================================================================

//===================================================================
// Funktionsautopilot: Klassen zur Verwaltung der StarCalc-Funktionen
//===================================================================

class ScFuncDesc
{
public:
                ScFuncDesc();
                ~ScFuncDesc();

    /** Returns a semicolon separated list of all parameter names. */
    String  GetParamList        () const;
    /** Returns the full function siganture: "FUNCTIONNAME( parameter list )". */
    String  GetSignature        () const;
    /** Returns the function siganture with parameters from the passed string array. */
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

    const ScFuncDesc*   Get( const String& rFName );
    const ScFuncDesc*   Get( USHORT nFIndex );
    const ScFuncDesc*   First( USHORT nCategory = 0 );
    const ScFuncDesc*   Next() const;

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

    const ScFuncDesc*   First()
                        { return (const ScFuncDesc*) aFunctionList.First(); }

    const ScFuncDesc*   Next()
                        { return (const ScFuncDesc*) aFunctionList.Next(); }

    const ScFuncDesc*   GetFunction( ULONG nIndex ) const
                    { return (const ScFuncDesc*) aFunctionList.GetObject( nIndex ); }

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
                                             const ScFuncDesc** ppFDesc = NULL,
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
    SCCOLROW        nField;
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
    SCCOL           nCol1;
    SCROW           nRow1;
    SCCOL           nCol2;
    SCROW           nRow2;
    SCTAB           nTab;
    BOOL            bHasHeader;
    BOOL            bByRow;
    BOOL            bInplace;
    BOOL            bCaseSens;
    BOOL            bRegExp;
    BOOL            bDuplicate;
    BOOL            bDestPers;          // nicht gespeichert
    SCTAB           nDestTab;
    SCCOL           nDestCol;
    SCROW           nDestRow;

private:
    SCSIZE          nEntryCount;
    ScQueryEntry*   pEntries;

public:
    ScQueryParam();
    ScQueryParam( const ScQueryParam& r );
    ~ScQueryParam();

    SCSIZE          GetEntryCount() const           { return nEntryCount; }
    ScQueryEntry&   GetEntry(SCSIZE n) const        { return pEntries[n]; }
    void            Resize(SCSIZE nNew);

    ScQueryParam&   operator=   ( const ScQueryParam& r );
    BOOL            operator==  ( const ScQueryParam& rOther ) const;
    void            Clear       ();
    void            DeleteQuery( SCSIZE nPos );

    void            MoveToDest();
    void            FillInExcelSyntax(String& aCellStr, SCSIZE nIndex);

    void            Load(SvStream& rStream);
    void            Store(SvStream& rStream) const;
};

// -----------------------------------------------------------------------

struct ScSubTotalParam
{
    SCCOL           nCol1;          // Selektierter Bereich
    SCROW           nRow1;
    SCCOL           nCol2;
    SCROW           nRow2;
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
    SCCOL           nField[MAXSUBTOTAL];        // zugehoeriges Feld
    SCCOL           nSubTotals[MAXSUBTOTAL];    // Anzahl der SubTotals
    SCCOL*          pSubTotals[MAXSUBTOTAL];    // Array der zu berechnenden Spalten
    ScSubTotalFunc* pFunctions[MAXSUBTOTAL];    // Array der zugehoerige Funktionen

    ScSubTotalParam();
    ScSubTotalParam( const ScSubTotalParam& r );

    ScSubTotalParam&    operator=       ( const ScSubTotalParam& r );
    BOOL                operator==      ( const ScSubTotalParam& r ) const;
    void                Clear           ();
    void                SetSubTotals    ( USHORT                nGroup,
                                          const SCCOL*          ptrSubTotals,
                                          const ScSubTotalFunc* ptrFuncions,
                                          USHORT                nCount );
};

// -----------------------------------------------------------------------
class ScArea;

struct ScConsolidateParam
{
    SCCOL           nCol;                   // Cursor Position /
    SCROW           nRow;                   // bzw. Anfang des Zielbereiches
    SCTAB           nTab;
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
    SCsCOL  nCol;
    USHORT  nFuncMask;
    USHORT  nFuncCount;

    PivotField() :
        nCol(0),nFuncMask(0),nFuncCount(0) {}

    PivotField( const PivotField& rCpy ) :
        nCol(rCpy.nCol),nFuncMask(rCpy.nFuncMask),nFuncCount(rCpy.nFuncCount) {}

    PivotField(SCsCOL nNewCol, USHORT nNewFuncMask = 0) :
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
    SCCOL           nCol;           // Cursor Position /
    SCROW           nRow;           // bzw. Anfang des Zielbereiches
    SCTAB           nTab;
    LabelData**     ppLabelArr;
    SCSIZE          nLabels;
    PivotField      aPageArr[PIVOT_MAXPAGEFIELD];
    PivotField      aColArr[PIVOT_MAXFIELD];
    PivotField      aRowArr[PIVOT_MAXFIELD];
    PivotField      aDataArr[PIVOT_MAXFIELD];
    SCSIZE          nPageCount;
    SCSIZE          nColCount;
    SCSIZE          nRowCount;
    SCSIZE          nDataCount;
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
                                      SCSIZE        nLab );
    void            SetPivotArrays  ( const PivotField* pPageArr,
                                      const PivotField* pColArr,
                                      const PivotField* pRowArr,
                                      const PivotField* pDataArr,
                                      SCSIZE            nPageCnt,
                                      SCSIZE            nColCnt,
                                      SCSIZE            nRowCnt,
                                      SCSIZE            nDataCnt );
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
extern ::utl::TransliterationWrapper* GetScGlobalpTransliteration();//CHINA001
extern const LocaleDataWrapper* GetScGlobalpLocaleData();

#endif
