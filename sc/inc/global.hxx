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

#ifndef SC_SCGLOB_HXX
#define SC_SCGLOB_HXX

#include "address.hxx"
#include <i18nlangtag/lang.h>
#include <tools/stream.hxx>
#include <osl/endian.h>
#include <com/sun/star/uno/Reference.hxx>
#include "scdllapi.h"
#include <rtl/ustring.hxx>

#include <boost/unordered_map.hpp>
#include <vector>

class ImageList;
class Bitmap;
class SfxItemSet;
class Color;

// Macro for call profiler (WinNT)
// S_CAP starts a measurement, E_CAP stops it
#if defined( WNT ) && defined( PROFILE )

extern "C" {
    void StartCAP();
    void StopCAP();
    void DumpCAP();
};

#define S_CAP   StartCAP();
#define E_CAP   StopCAP(); DumpCAP();

#endif

#define SC_COLLATOR_IGNORES ( \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE )

#define SC_TRANSLITERATION_IGNORECASE ( \
    ::com::sun::star::i18n::TransliterationModules_IGNORE_CASE )
#define SC_TRANSLITERATION_CASESENSE 0

//  Calc has lots of names...
//  Clipboard names are in so3/soapp.hxx now
//  STRING_SCAPP was "scalc3", "scalc4", now just "scalc"

#define STRING_SCAPP    "scalc"
#define STRING_SCSTREAM "StarCalcDocument"

#define STRING_STANDARD "Standard"

// characters

//  '\r' does not work on a Mac...
#define CHAR_CR     char(13)

const sal_Unicode CHAR_NBSP     = 0x00A0;
const sal_Unicode CHAR_SHY      = 0x00AD;
const sal_Unicode CHAR_ZWSP     = 0x200B;
const sal_Unicode CHAR_LRM      = 0x200E;
const sal_Unicode CHAR_RLM      = 0x200F;
const sal_Unicode CHAR_NBHY     = 0x2011;
const sal_Unicode CHAR_ZWNBSP   = 0x2060;

// ----------------------------------------------------------------------------

#define MINDOUBLE   1.7e-307
#define MAXDOUBLE   1.7e307

#define MINZOOM     20
#define MAXZOOM     400

const SCSIZE MAXSUBTOTAL        = 3;

#define SC_START_INDEX_DB_COLL 50000
                                        // Above this threshold are indices
                                        // for data base areas

#define SC_USE_PS_POINTS    1       /**< use PostScript points (72ppi) instead of old TeX points (72.27ppi) */

#define PIXEL_PER_INCH      96.0

#define CM_PER_INCH         2.54
#define PS_POINTS_PER_INCH  72.0    /**< PostScript points per inch */
#define TEX_POINTS_PER_INCH 72.27   /**< old printer points, or TeX points per inch */
#if SC_USE_PS_POINTS
#define POINTS_PER_INCH     PS_POINTS_PER_INCH  /**< the actual definition of points used */
#else
#define POINTS_PER_INCH     TEX_POINTS_PER_INCH
#endif
#define PIXEL_PER_POINT     (PIXEL_PER_INCH / POINTS_PER_INCH)
#define TWIPS_PER_POINT     20.0
#define TWIPS_PER_INCH      (TWIPS_PER_POINT * POINTS_PER_INCH)
#define TWIPS_PER_CM        (TWIPS_PER_INCH / CM_PER_INCH)
#define CM_PER_TWIPS        (CM_PER_INCH / TWIPS_PER_INCH)
#define TWIPS_PER_PIXEL     (TWIPS_PER_INCH / PIXEL_PER_INCH)
#define TWIPS_PER_CHAR      (TWIPS_PER_INCH / 13.6)
#define PIXEL_PER_TWIPS     (PIXEL_PER_INCH / TWIPS_PER_INCH)
#define HMM_PER_TWIPS       (CM_PER_TWIPS * 1000.0)

#if SC_USE_PS_POINTS
#define STD_COL_WIDTH       1280    /* 2.2577cm, 64.00pt PS */
#else
#define STD_COL_WIDTH       1285    /* 2.2581cm, 64.25pt TeX */
#endif
#define STD_EXTRA_WIDTH     113     /* 2mm extra for optimal width,
                                     * 0.1986cm with TeX points,
                                     * 0.1993cm with PS points. */


#define MAX_EXTRA_WIDTH     23811   /* 42cm in TWIPS, 41.8430cm TeX, 41.9999cm PS */
#define MAX_EXTRA_HEIGHT    23811
#define MAX_COL_WIDTH       56693   /* 1m in TWIPS, 99.6266cm TeX, 100.0001cm PS */
#define MAX_ROW_HEIGHT      56693

                                    /* standard row height: text + margin - STD_ROWHEIGHT_DIFF */
#define STD_ROWHEIGHT_DIFF  23

///     use ScGlobal::nStdRowHeight instead of STD_ROW_HEIGHT !

#define STD_ROW_HEIGHT      (12.8 * TWIPS_PER_POINT)    /* 256 Twips, 0.45 cm */

namespace sc
{
    inline long TwipsToHMM( long nTwips )     { return (nTwips * 127 + 36) / 72; }
    inline long HMMToTwips( long nHMM )       { return (nHMM * 72 + 63) / 127; }
    inline long TwipsToEvenHMM( long nTwips ) { return ( (nTwips * 127 + 72) / 144 ) * 2; }
}

                                    // standard size as OLE server (cells)
#define OLE_STD_CELLS_X     4
#define OLE_STD_CELLS_Y     5

#define SC_SIZE_OPTIMUM     0xFFFF

                                    // repaint flags (for messages)
#define PAINT_GRID          1
#define PAINT_TOP           2
#define PAINT_LEFT          4
#define PAINT_EXTRAS        8
#define PAINT_MARKS         16
#define PAINT_OBJECTS       32
#define PAINT_SIZE          64
#define PAINT_ALL           ( PAINT_GRID | PAINT_TOP | PAINT_LEFT | PAINT_EXTRAS | PAINT_OBJECTS | PAINT_SIZE )


                                    // flags for columns / rows
                                    // FILTERED always together with HIDDEN
                                    // FILTERED and MANUALSIZE only valid for rows
const sal_uInt8   CR_HIDDEN      = 1;
const sal_uInt8   CR_MANUALBREAK = 8;
const sal_uInt8   CR_FILTERED    = 16;
const sal_uInt8   CR_MANUALSIZE  = 32;
const sal_uInt8   CR_ALL         = (CR_HIDDEN | CR_MANUALBREAK | CR_FILTERED | CR_MANUALSIZE);

typedef sal_uInt8 ScBreakType;
const ScBreakType BREAK_NONE   = 0;
const ScBreakType BREAK_PAGE   = 1;
const ScBreakType BREAK_MANUAL = 2;

// insert/delete flags
const sal_uInt16 IDF_NONE       = 0x0000;
const sal_uInt16 IDF_VALUE      = 0x0001;   /// Numeric values (and numeric results if IDF_FORMULA is not set).
const sal_uInt16 IDF_DATETIME   = 0x0002;   /// Dates, times, datetime values.
const sal_uInt16 IDF_STRING     = 0x0004;   /// Strings (and string results if IDF_FORMULA is not set).
const sal_uInt16 IDF_NOTE       = 0x0008;   /// Cell notes.
const sal_uInt16 IDF_FORMULA    = 0x0010;   /// Formula cells.
const sal_uInt16 IDF_HARDATTR   = 0x0020;   /// Hard cell attributes.
const sal_uInt16 IDF_STYLES     = 0x0040;   /// Cell styles.
const sal_uInt16 IDF_OBJECTS    = 0x0080;   /// Drawing objects.
const sal_uInt16 IDF_EDITATTR   = 0x0100;   /// Rich-text attributes.
const sal_uInt16 IDF_OUTLINE    = 0x0800;   /// Sheet / outlining (grouping) information
const sal_uInt16 IDF_NOCAPTIONS = 0x0200;   /// Internal use only (undo etc.): do not copy/delete caption objects of cell notes.
const sal_uInt16 IDF_ADDNOTES   = 0x0400;   /// Internal use only (copy from clip): do not delete existing cell contents when pasting notes.
const sal_uInt16 IDF_SPECIAL_BOOLEAN = 0x1000;
const sal_uInt16 IDF_ATTRIB     = IDF_HARDATTR | IDF_STYLES | IDF_OUTLINE;
const sal_uInt16 IDF_CONTENTS   = IDF_VALUE | IDF_DATETIME | IDF_STRING | IDF_NOTE | IDF_FORMULA | IDF_OUTLINE;
const sal_uInt16 IDF_ALL        = IDF_CONTENTS | IDF_ATTRIB | IDF_OBJECTS;

/// Copy flags for auto/series fill functions: do not touch notes and drawing objects.
const sal_uInt16 IDF_AUTOFILL   = IDF_ALL & ~(IDF_NOTE | IDF_OBJECTS);

#define PASTE_NOFUNC        0
#define PASTE_ADD           1
#define PASTE_SUB           2
#define PASTE_MUL           3
#define PASTE_DIV           4

                                        // bits for HasAttr
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
#define EMPTY_OUSTRING ScGlobal::GetEmptyOUString()

                                        //  layer id's for drawing
#define SC_LAYER_FRONT      0
#define SC_LAYER_BACK       1
#define SC_LAYER_INTERN     2
#define SC_LAYER_CONTROLS   3
#define SC_LAYER_HIDDEN     4

                                        //  link tables
#define SC_LINK_NONE        0
#define SC_LINK_NORMAL      1
#define SC_LINK_VALUE       2

                                        //  input
#define SC_ENTER_NORMAL     0
#define SC_ENTER_BLOCK      1
#define SC_ENTER_MATRIX     2

                                        //  step = 10pt, max. indention = 100 steps
#define SC_INDENT_STEP      200
#define SC_MAX_INDENT       20000

                                        //  scenario flags
#define SC_SCENARIO_COPYALL     1
#define SC_SCENARIO_SHOWFRAME   2
#define SC_SCENARIO_PRINTFRAME  4
#define SC_SCENARIO_TWOWAY      8
#define SC_SCENARIO_ATTRIB      16
#define SC_SCENARIO_VALUE       32
#define SC_SCENARIO_PROTECT     64

/** Default cell clone flags: do not start listening, do not adjust 3D refs to
    old position, clone note captions of cell notes. */
const int SC_CLONECELL_DEFAULT          = 0x0000;

/** If set, cloned formula cells will start to listen to the document. */
const int SC_CLONECELL_STARTLISTENING   = 0x0001;

/** If set, relative 3D references of cloned formula cells will be adjusted to
    old position (used while swapping cells for sorting a cell range). */
const int SC_CLONECELL_ADJUST3DREL      = 0x0002;

/** If set, the caption object of a cell note will not be cloned (used while
    copying cells to undo document, where captions are handled in drawing undo). */
const int SC_CLONECELL_NOCAPTION        = 0x0004;

/** If set, absolute refs will not transformed to external references */
const int SC_CLONECELL_NOMAKEABS_EXTERNAL = 0x0008;

#ifndef DELETEZ
#define DELETEZ(pPtr) { delete pPtr; pPtr = 0; }
#endif

                                    // is bit set in set?
#define IS_SET(bit,set)(((set)&(bit))==(bit))

enum CellType
    {
        CELLTYPE_NONE,
        CELLTYPE_VALUE,
        CELLTYPE_STRING,
        CELLTYPE_FORMULA,
        CELLTYPE_EDIT,
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

enum ScMoveDirection
{
    SC_MOVE_RIGHT,
    SC_MOVE_LEFT,
    SC_MOVE_UP,
    SC_MOVE_DOWN
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
        SC_INPUT_TYPE,              // input, while not in inplace mode
        SC_INPUT_TABLE,             // text cursor in the table
        SC_INPUT_TOP                // text cursor in the input line
    };

enum ScVObjMode                     // output modes of objects on a page
{
    VOBJ_MODE_SHOW,
    VOBJ_MODE_HIDE
};

enum ScAnchorType                   // anchor of a character object
{
    SCA_CELL,
    SCA_PAGE,
    SCA_DONTKNOW                    // for multi selection
};

enum ScGetDBMode
{
    SC_DB_MAKE,     // create "untitled" (if necessary)
    SC_DB_IMPORT,   // create "Importx" (if necessary)
    SC_DB_OLD       // don't create
};

/// For ScDBFunc::GetDBData()
enum ScGetDBSelection
{
    /** Keep selection as is, expand to used data area if no selection. */
    SC_DBSEL_KEEP,

    /** Shrink selection to sheet's data area. */
    SC_DBSEL_SHRINK_TO_SHEET_DATA,

    /** Shrink selection to actually used data area within the selection. */
    SC_DBSEL_SHRINK_TO_USED_DATA,

    /** If only one row or portion thereof is selected, shrink row to used data
        columns and select further rows down until end of data. If an area is
        selected, shrink rows to actually used columns. Else, no selection,
        expand to used data area. */
    SC_DBSEL_ROW_DOWN,

    /** Behave as if the range corresponding to a ScDBData area was selected,
        for API use. */
    SC_DBSEL_FORCE_MARK
};

enum ScLkUpdMode    // modes for updating links
{
    LM_ALWAYS,
    LM_NEVER,
    LM_ON_DEMAND,
    LM_UNKNOWN
};


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
    bool            bImport;
    OUString aDBName;                    // alias of data base
    OUString aStatement;
    bool            bNative;
    bool            bSql;                       // statement or name?
    sal_uInt8       nType;                      // enum DBObject

    ScImportParam();
    ScImportParam( const ScImportParam& r );
    ~ScImportParam();

    ScImportParam&  operator=   ( const ScImportParam& r );
    bool            operator==  ( const ScImportParam& r ) const;
};

struct ScStringHashCode
{
    size_t operator()( const String& rStr ) const
    {
        return rtl_ustr_hashCode_WithLength( rStr.GetBuffer(), rStr.Len() );
    }
};

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
class ScFieldEditEngine;

namespace com { namespace sun { namespace star {
    namespace lang {
        struct Locale;
    }
    namespace i18n {
        class XOrdinalSuffix;
    }
}}}
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
    static OUString**       ppRscString;
    static String*          pStrScDoc;
    static String*          pEmptyString;
    static OUString* pEmptyOUString;
    static String*          pStrClipDocName;
    static SvxBrushItem*    pEmptyBrushItem;
    static SvxBrushItem*    pButtonBrushItem;
    static SvxBrushItem*    pEmbeddedBrushItem;
    static SvxBrushItem*    pProtectedBrushItem;

    static ImageList*       pOutlineBitmaps;

    static ScFunctionList*  pStarCalcFunctionList;
    static ScFunctionMgr*   pStarCalcFunctionMgr;

    static ScUnitConverter* pUnitConverter;

    static  SvNumberFormatter*  pEnglishFormatter;          // for UNO / XML export

    static ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XOrdinalSuffix> xOrdinalSuffix;
    static CalendarWrapper*     pCalendar;
    static CollatorWrapper*     pCaseCollator;
    static CollatorWrapper*     pCollator;
    static ::utl::TransliterationWrapper* pTransliteration;
    static ::utl::TransliterationWrapper* pCaseTransliteration;
    static IntlWrapper*         pScIntlWrapper;
    static ::com::sun::star::lang::Locale*      pLocale;

    static ScFieldEditEngine*   pFieldEditEngine;

public:
    static SvtSysLocale*        pSysLocale;
    // for faster access a pointer to the single instance provided by SvtSysLocale
    SC_DLLPUBLIC static const CharClass*     pCharClass;
    // for faster access a pointer to the single instance provided by SvtSysLocale
    SC_DLLPUBLIC static const LocaleDataWrapper* pLocaleData;
    SC_DLLPUBLIC static const LocaleDataWrapper* GetpLocaleData();

    static CalendarWrapper*     GetCalendar();
    SC_DLLPUBLIC static CollatorWrapper*        GetCollator();
    static CollatorWrapper*     GetCaseCollator();
    static IntlWrapper*         GetScIntlWrapper();
    static ::com::sun::star::lang::Locale*      GetLocale();

    SC_DLLPUBLIC static ::utl::TransliterationWrapper* GetpTransliteration();
    static ::utl::TransliterationWrapper* GetCaseTransliteration();

    SC_DLLPUBLIC static LanguageType            eLnge;
    static sal_Unicode          cListDelimiter;

    static const String&        GetClipDocName();
    static void                 SetClipDocName( const String& rNew );
    SC_DLLPUBLIC static const SvxSearchItem&    GetSearchItem();
    SC_DLLPUBLIC static void                    SetSearchItem( const SvxSearchItem& rNew );
    SC_DLLPUBLIC static ScAutoFormat*       GetAutoFormat();
    SC_DLLPUBLIC static ScAutoFormat*       GetOrCreateAutoFormat();
    static void                 ClearAutoFormat(); //BugId 54209
    static FuncCollection*      GetFuncCollection();
    SC_DLLPUBLIC static ScUnoAddInCollection* GetAddInCollection();
    SC_DLLPUBLIC static ScUserList*         GetUserList();
    static void                 SetUserList( const ScUserList* pNewList );
    SC_DLLPUBLIC static const OUString&       GetRscString( sal_uInt16 nIndex );
    static void                 OpenURL( const String& rURL, const String& rTarget );
    SC_DLLPUBLIC static String              GetAbsDocName( const String& rFileName,
                                                SfxObjectShell* pShell );
    SC_DLLPUBLIC static String              GetDocTabName( const String& rFileName,
                                                const String& rTabName );
    SC_DLLPUBLIC static sal_uLong               GetStandardFormat( SvNumberFormatter&,
                                    sal_uLong nFormat, short nType );
    SC_DLLPUBLIC static sal_uLong               GetStandardFormat( double, SvNumberFormatter&,
                                    sal_uLong nFormat, short nType );

    SC_DLLPUBLIC static sal_uInt16 GetStandardRowHeight();
    SC_DLLPUBLIC static double              nScreenPPTX;
    SC_DLLPUBLIC static double              nScreenPPTY;

    static ScDocShellRef*   pDrawClipDocShellRef;

    static sal_uInt16           nDefFontHeight;
    SC_DLLPUBLIC static sal_uInt16           nStdRowHeight;

    SC_DLLPUBLIC static long                nLastRowHeightExtra;
    static long             nLastColWidthExtra;

    static void             Init();                     // during start up
    static void             InitAddIns();
    static void             Clear();                    // at the end of the program

    static void             UpdatePPT(OutputDevice* pDev);

    static void             InitTextHeight(SfxItemPool* pPool);
    static SvxBrushItem*    GetEmptyBrushItem() { return pEmptyBrushItem; }
    static SvxBrushItem*    GetButtonBrushItem();
    static SvxBrushItem*    GetEmbeddedBrushItem()  { return pEmbeddedBrushItem; }
    static SvxBrushItem*    GetProtectedBrushItem() { return pProtectedBrushItem; }
    SC_DLLPUBLIC    static const String&    GetEmptyString();
    SC_DLLPUBLIC    static const OUString&    GetEmptyOUString();
    static const String&    GetScDocString();

    /** Returns the specified image list with outline symbols. */
    static ImageList*       GetOutlineSymbols();

    static bool             HasStarCalcFunctionList();
    static ScFunctionList*  GetStarCalcFunctionList();
    static ScFunctionMgr*   GetStarCalcFunctionMgr();
    static void             ResetFunctionList();

    static String           GetErrorString(sal_uInt16 nErrNumber);
    static String           GetLongErrorString(sal_uInt16 nErrNumber);
    static sal_Bool             EETextObjEqual( const EditTextObject* pObj1,
                                            const EditTextObject* pObj2 );
    static sal_Bool             CheckWidthInvalidate( bool& bNumFormatChanged,
                                                  const SfxItemSet& rNewAttrs,
                                                  const SfxItemSet& rOldAttrs );
    static sal_Bool             HasAttrChanged( const SfxItemSet& rNewAttrs,
                                            const SfxItemSet& rOldAttrs,
                                            const sal_uInt16      nWhich );

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
    SC_DLLPUBLIC static OUString addToken(
                                const OUString& rTokenList, const OUString& rToken,
                                sal_Unicode cSep, sal_Int32 nSepCount = 1,
                                bool bForceSep = false );

    /** Returns true, if the first and last character of the string is cQuote. */
    SC_DLLPUBLIC static bool             IsQuoted( const String& rString, sal_Unicode cQuote = '\'' );

    /** Inserts the character cQuote at beginning and end of rString.
        @param bEscapeEmbedded      If <TRUE/>, embedded quote characters are
                                    escaped by doubling them.
     */
SC_DLLPUBLIC    static void             AddQuotes( OUString& rString, sal_Unicode cQuote = '\'', bool bEscapeEmbedded = true );

    /** Erases the character cQuote from rString, if it exists at beginning AND end.
        @param bUnescapeEmbedded    If <TRUE/>, embedded doubled quote characters
                                    are unescaped by replacing them with a
                                    single instance.
     */
SC_DLLPUBLIC    static void             EraseQuotes( OUString& rString, sal_Unicode cQuote = '\'', bool bUnescapeEmbedded = true );

    /** Finds an unquoted instance of cChar in rString, starting at
        offset nStart. Unquoted instances may occur when concatenating two
        quoted strings with a separator, for example, 's1':'s2'. Embedded
        quotes have to be escaped by being doubled. Caller must ensure that
        nStart points into an unquoted range or the opening quote. Specialty:
        if cChar==cQuote the first cQuote character from nStart on is found.
        @returns offset if found, else STRING_NOTFOUND
     */
SC_DLLPUBLIC    static xub_StrLen       FindUnquoted( const String& rString, sal_Unicode cChar, xub_StrLen nStart = 0, sal_Unicode cQuote = '\'' );

    /** Finds an unquoted instance of cChar in null-terminated pString. Same
        semantics as FindUnquoted( const String&, ...)
        @returns: pointer to cChar if found, else NULL
     */
SC_DLLPUBLIC    static const sal_Unicode* FindUnquoted( const sal_Unicode* pString, sal_Unicode cChar, sal_Unicode cQuote = '\'' );


    static  CharSet         GetCharsetValue( const String& rCharSet );
    static  String          GetCharsetString( CharSet eVal );

    /// a "ReadOnly" formatter for UNO/XML export
    static  SvNumberFormatter*  GetEnglishFormatter();

    static sal_Bool IsSystemRTL();                      // depending on system language
    static LanguageType GetEditDefaultLanguage();   // for EditEngine::SetDefaultLanguage
    SC_DLLPUBLIC static sal_uInt8   GetDefaultScriptType();             // for all WEAK characters
    /** Map ATTR_((CJK|CTL)_)?FONT_... to proper WhichIDs.
        If more than one SCRIPTTYPE_... values are or'ed together, prefers
        first COMPLEX, then ASIAN */
    SC_DLLPUBLIC static sal_uInt16 GetScriptedWhichID( sal_uInt8 nScriptType, sal_uInt16 nWhich );

    /** Adds a language item to the item set, if the number format item contains
        a language that differs from its parent's language. */
    SC_DLLPUBLIC static void             AddLanguage( SfxItemSet& rSet, SvNumberFormatter& rFormatter );

    /** Obtain the ordinal suffix for a number according to the system locale */
    static String           GetOrdinalSuffix( sal_Int32 nNumber);

    /** A static instance of ScFieldEditEngine not capable of resolving
        document specific fields, to be used only by ScEditUtil::GetString(). */
    static ScFieldEditEngine&   GetStaticFieldEditEngine();
};
#endif

// maybe move to dbdata.hxx (?):

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
        SC_BOTPERC,
        SC_CONTAINS,
        SC_DOES_NOT_CONTAIN,
        SC_BEGINS_WITH,
        SC_DOES_NOT_BEGIN_WITH,
        SC_ENDS_WITH,
        SC_DOES_NOT_END_WITH
    };

enum ScQueryConnect
    {
        SC_AND,
        SC_OR
    };

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
        SUBTOTAL_FUNC_VARP  = 11,
        SUBTOTAL_FUNC_SELECTION_COUNT = 12
    };

class ScArea;

struct ScConsolidateParam
{
    SCCOL           nCol;                   // cursor position /
    SCROW           nRow;                   // or start of destination area respectively
    SCTAB           nTab;
    ScSubTotalFunc  eFunction;
    sal_uInt16          nDataAreaCount;         // number of data areas
    ScArea**        ppDataAreas;            // array of pointers into data areas
    sal_Bool            bByCol;
    sal_Bool            bByRow;
    sal_Bool            bReferenceData;         // reference source data

    ScConsolidateParam();
    ScConsolidateParam( const ScConsolidateParam& r );
    ~ScConsolidateParam();

    ScConsolidateParam& operator=       ( const ScConsolidateParam& r );
    sal_Bool                operator==      ( const ScConsolidateParam& r ) const;
    void                Clear           (); // = ClearDataAreas()+Members
    void                ClearDataAreas  ();
    void                SetAreas        ( ScArea* const* ppAreas, sal_uInt16 nCount );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
