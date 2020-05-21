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

#ifndef INCLUDED_SC_INC_GLOBAL_HXX
#define INCLUDED_SC_INC_GLOBAL_HXX

#include "address.hxx"
#include <i18nlangtag/lang.h>
#include <svx/svdtypes.hxx>
#include <tools/ref.hxx>
#include <sal/types.h>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include "scdllapi.h"
#include <rtl/ustring.hxx>

#include <atomic>
// HACK: <atomic> includes <stdbool.h>, which in some Clang versions does '#define bool bool',
// which confuses clang plugins.
#undef bool
#include <map>
#include <memory>

namespace com { namespace sun { namespace star { namespace uno { template <typename > class Reference; } } } }

class SfxItemSet;
class SfxViewShell;
struct ScCalcConfig;
enum class SvtScriptType;
enum class FormulaError : sal_uInt16;
enum class SvNumFormatType : sal_Int16;

#define SC_COLLATOR_IGNORES css::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE

//  Calc has lots of names...
//  Clipboard names are in so3/soapp.hxx now
//  STRING_SCAPP was "scalc3", "scalc4", now just "scalc"

#define STRING_SCAPP    "scalc"

#define STRING_STANDARD "Standard"

// Have the dreaded programmatic filter name defined in one place.
#define SC_TEXT_CSV_FILTER_NAME "Text - txt - csv (StarCalc)"

// characters

const sal_Unicode CHAR_NBSP     = 0x00A0;
const sal_Unicode CHAR_SHY      = 0x00AD;
const sal_Unicode CHAR_ZWSP     = 0x200B;
const sal_Unicode CHAR_LRM      = 0x200E;
const sal_Unicode CHAR_RLM      = 0x200F;
const sal_Unicode CHAR_NBHY     = 0x2011;
const sal_Unicode CHAR_ZWNBSP   = 0x2060;

#define MINDOUBLE   1.7e-307
#define MAXDOUBLE   1.7e307

#define MINZOOM     20
#define MAXZOOM     400

const SCSIZE MAXSUBTOTAL        = 3;

#define PIXEL_PER_INCH      96.0
#define CM_PER_INCH         2.54
#define POINTS_PER_INCH     72.0    /**< PostScript points per inch */
#define PIXEL_PER_POINT     (PIXEL_PER_INCH / POINTS_PER_INCH)
#define TWIPS_PER_POINT     20.0
#define TWIPS_PER_INCH      (TWIPS_PER_POINT * POINTS_PER_INCH)
#define TWIPS_PER_CM        (TWIPS_PER_INCH / CM_PER_INCH)
#define CM_PER_TWIPS        (CM_PER_INCH / TWIPS_PER_INCH)
#define TWIPS_PER_PIXEL     (TWIPS_PER_INCH / PIXEL_PER_INCH)
#define TWIPS_PER_CHAR      (TWIPS_PER_INCH / 13.6)
#define PIXEL_PER_TWIPS     (PIXEL_PER_INCH / TWIPS_PER_INCH)
#define HMM_PER_TWIPS       (CM_PER_TWIPS * 1000.0)

#define STD_COL_WIDTH       1280    /* 2.2577cm, 64.00pt PS */
#define STD_EXTRA_WIDTH     113     /* 2mm extra for optimal width,
                                     * 0.1986cm with TeX points,
                                     * 0.1993cm with PS points. */

#define MAX_EXTRA_WIDTH     23811   /* 42cm in TWIPS, 41.8430cm TeX, 41.9999cm PS */
#define MAX_EXTRA_HEIGHT    23811
#define MAX_COL_WIDTH       56693   /* 1m in TWIPS, 99.6266cm TeX, 100.0001cm PS */
#define MAX_ROW_HEIGHT      56693

                                    /* standard row height: text + margin - STD_ROWHEIGHT_DIFF */
#define STD_ROWHEIGHT_DIFF  23

namespace sc
{
    inline long TwipsToHMM( long nTwips )     { return (nTwips * 127 + 36) / 72; }
    inline long HMMToTwips( long nHMM )       { return (nHMM * 72 + 63) / 127; }
    inline long TwipsToEvenHMM( long nTwips ) { return ( (nTwips * 127 + 72) / 144 ) * 2; }
}

                                    // standard size as OLE server (cells)
#define OLE_STD_CELLS_X     4
#define OLE_STD_CELLS_Y     5


                                    // repaint flags (for messages)
enum class PaintPartFlags {
    NONE          = 0x00,
    Grid          = 0x01,
    Top           = 0x02,
    Left          = 0x04,
    Extras        = 0x08,
    Marks         = 0x10,
    Objects       = 0x20,
    Size          = 0x40,
    All           = Grid | Top | Left | Extras | Objects | Size,
};
namespace o3tl {
    template<> struct typed_flags<PaintPartFlags> : is_typed_flags<PaintPartFlags, 0x07f> {};
}

                                    // flags for columns / rows
enum class CRFlags : sal_uInt8 {
    // Filtered always together with Hidden
    // Filtered and ManualSize only valid for rows
    NONE        = 0x00,
    Hidden      = 0x01,
    ManualBreak = 0x02,
    Filtered    = 0x04,
    ManualSize  = 0x08,
    All         = Hidden | ManualBreak | Filtered | ManualSize
};
namespace o3tl {
    template<> struct typed_flags<CRFlags> : is_typed_flags<CRFlags, 0x0f> {};
}

enum class ScBreakType {
    NONE   = 0x00,
    Page   = 0x01,
    Manual = 0x02
};
namespace o3tl {
    template<> struct typed_flags<ScBreakType> : is_typed_flags<ScBreakType, 0x03> {};
}

enum class InsertDeleteFlags : sal_uInt16
{
    NONE             = 0x0000,
    VALUE            = 0x0001,   /// Numeric values (and numeric results if InsertDeleteFlags::FORMULA is not set).
    DATETIME         = 0x0002,   /// Dates, times, datetime values.
    STRING           = 0x0004,   /// Strings (and string results if InsertDeleteFlags::FORMULA is not set).
    NOTE             = 0x0008,   /// Cell notes.
    FORMULA          = 0x0010,   /// Formula cells.
    HARDATTR         = 0x0020,   /// Hard cell attributes.
    STYLES           = 0x0040,   /// Cell styles.
    OBJECTS          = 0x0080,   /// Drawing objects.
    EDITATTR         = 0x0100,   /// Rich-text attributes.
    OUTLINE          = 0x0800,   /// Sheet / outlining (grouping) information
    NOCAPTIONS       = 0x0200,   /// Internal use only (undo etc.): do not copy/delete caption objects of cell notes.
    ADDNOTES         = 0x0400,   /// Internal use only (copy from clip): do not delete existing cell contents when pasting notes.
    SPECIAL_BOOLEAN  = 0x1000,
    FORGETCAPTIONS   = 0x2000,   /// Internal use only (d&d undo): do not delete caption objects of cell notes.
    ATTRIB           = HARDATTR | STYLES,
    CONTENTS         = VALUE | DATETIME | STRING | NOTE | FORMULA | OUTLINE,
    ALL              = CONTENTS | ATTRIB | OBJECTS,
    /// Copy flags for auto/series fill functions: do not touch notes and drawing objects.
    AUTOFILL         = ALL & ~(NOTE | OBJECTS)
};
namespace o3tl
{
    template<> struct typed_flags<InsertDeleteFlags> : is_typed_flags<InsertDeleteFlags, 0x3fff> {};
}
// This doesn't work at the moment, perhaps when we have constexpr we can modify InsertDeleteFlags to make it work.
//static_assert((InsertDeleteFlags::ATTRIB & InsertDeleteFlags::CONTENTS) == InsertDeleteFlags::NONE, "these must match");


enum class ScPasteFunc {
    NONE, ADD, SUB, MUL, DIV
};
                                        // bits for HasAttr
enum class HasAttrFlags {
    NONE            = 0x0000,
    Lines           = 0x0001,
    Merged          = 0x0002,
    Overlapped      = 0x0004,
    Protected       = 0x0008,
    Shadow          = 0x0010,
    NeedHeight      = 0x0020,
    ShadowRight     = 0x0040,
    ShadowDown      = 0x0080,
    AutoFilter      = 0x0100,
    Conditional     = 0x0200,
    Rotate          = 0x0400,
    NotOverlapped   = 0x0800,
    RightOrCenter   = 0x1000,   // right or centered logical alignment
};
namespace o3tl {
    template<> struct typed_flags<HasAttrFlags> : is_typed_flags<HasAttrFlags, 0x1fff> {};
}


#define EMPTY_OUSTRING ScGlobal::GetEmptyOUString()

// Layer id's for drawing.
// These are both id's and positions.
constexpr SdrLayerID SC_LAYER_FRONT   (0);
constexpr SdrLayerID SC_LAYER_BACK    (1);
constexpr SdrLayerID SC_LAYER_INTERN  (2);
constexpr SdrLayerID SC_LAYER_CONTROLS(3);
constexpr SdrLayerID SC_LAYER_HIDDEN  (4);

//  link tables
enum class ScLinkMode {
    NONE, NORMAL, VALUE
};
                                        //  input
enum class ScEnterMode {
    NORMAL, BLOCK, MATRIX
};

                                        //  step = 10pt, max. indention = 100 steps
#define SC_INDENT_STEP      200

enum class ScScenarioFlags{             //  scenario flags
    NONE       = 0,
    CopyAll    = 1,
    ShowFrame  = 2,
    PrintFrame = 4,
    TwoWay     = 8,
    Attrib     = 16,
    Value      = 32,
    Protected  = 64
};
namespace o3tl {
    template<> struct typed_flags<ScScenarioFlags> : is_typed_flags<ScScenarioFlags, 127> {};
}

enum class SubtotalFlags {
    NONE              = 0x00,
    IgnoreNestedStAg  = 0x08,
    IgnoreErrVal      = 0x04,
    IgnoreHidden      = 0x02,
    IgnoreFiltered    = 0x01
};
namespace o3tl {
    template<> struct typed_flags<SubtotalFlags> : is_typed_flags<SubtotalFlags, 0x0f> {};
}

enum class ScCloneFlags{
/** Default cell clone flags: do not start listening, do not adjust 3D refs to
    old position, clone note captions of cell notes. */
    Default          = 0x0000,

/** If set, cloned formula cells will start to listen to the document. */
    StartListening   = 0x0001,

/** If set, absolute refs will not transformed to external references */
    NoMakeAbsExternal = 0x0002,

/** If set, global named expressions will be converted to sheet-local named
    expressions. */
    NamesToLocal   = 0x0004
};
namespace o3tl
{
    template<> struct typed_flags<ScCloneFlags> : is_typed_flags<ScCloneFlags, 0x0007> {};
}

enum CellType
    {
        CELLTYPE_NONE,
        CELLTYPE_VALUE,
        CELLTYPE_STRING,
        CELLTYPE_FORMULA,
        CELLTYPE_EDIT,
    };

enum class DelCellCmd
    {
        CellsUp,
        CellsLeft,
        Rows,
        Cols,
        NONE
    };

enum InsCellCmd
    {
        INS_CELLSDOWN,
        INS_CELLSRIGHT,
        INS_INSROWS_BEFORE,
        INS_INSCOLS_BEFORE,
        INS_NONE,
        INS_INSROWS_AFTER,
        INS_INSCOLS_AFTER
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
    SCA_CELL,                       // anchor to cell, move with cell
    SCA_CELL_RESIZE,                // anchor to cell, move and resize with cell
    SCA_PAGE,                       // anchor to page, independent of any cells
    SCA_DONTKNOW                    // for multi selection
};

enum ScGetDBMode
{
    SC_DB_MAKE,         ///< create "untitled" (if necessary)
    SC_DB_AUTOFILTER,   ///< force use of sheet-local instead of document global anonymous range
    SC_DB_IMPORT,       ///< create "Importx" (if necessary)
    SC_DB_OLD           ///< don't create
};

/// For ScDBFunc::GetDBData()
enum class ScGetDBSelection
{
    /** Keep selection as is, expand to used data area if no selection. */
    Keep,

    /** Shrink selection to actually used data area within the selection. */
    ShrinkToUsedData,

    /** If only one row or portion thereof is selected, shrink row to used data
        columns and select further rows down until end of data. If an area is
        selected, shrink rows to actually used columns. Else, no selection,
        expand to used data area. */
    RowDown,

    /** Behave as if the range corresponding to a ScDBData area was selected,
        for API use. */
    ForceMark
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

namespace sc {

enum class ColRowEditAction
{
    Unknown,
    InsertColumnsBefore,
    InsertColumnsAfter,
    InsertRowsBefore,
    InsertRowsAfter,
    DeleteColumns,
    DeleteRows
};

}

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

class ScDocShell;
class ScAutoFormat;
class LegacyFuncCollection;
class ScUnoAddInCollection;
class ScUserList;
class SvxBrushItem;
class ScFunctionList;
class ScFunctionMgr;
class SfxItemPool;
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

class ScGlobal
{
    static ScAutoFormat*    pAutoFormat;
    static std::atomic<LegacyFuncCollection*> pLegacyFuncCollection;
    static std::atomic<ScUnoAddInCollection*> pAddInCollection;
    static ScUserList*      pUserList;
    static std::map<const char*, OUString>* pRscString;
    static OUString*        pStrScDoc;
    static OUString*        pEmptyOUString;
    static OUString*        pStrClipDocName;
    static SvxBrushItem*    pEmptyBrushItem;
    static SvxBrushItem*    pButtonBrushItem;
    static SvxBrushItem*    pEmbeddedBrushItem;

    static ScFunctionList*  pStarCalcFunctionList;
    static ScFunctionMgr*   pStarCalcFunctionMgr;

    static std::atomic<ScUnitConverter*> pUnitConverter;

    static  SvNumberFormatter*  pEnglishFormatter;          // for UNO / XML export

    static css::uno::Reference< css::i18n::XOrdinalSuffix> xOrdinalSuffix;
    static CalendarWrapper*     pCalendar;
    static std::atomic<CollatorWrapper*>     pCaseCollator;
    static std::atomic<CollatorWrapper*>     pCollator;
    static std::atomic<::utl::TransliterationWrapper*> pTransliteration;
    static std::atomic<::utl::TransliterationWrapper*> pCaseTransliteration;
    static IntlWrapper*         pScIntlWrapper;
    static std::atomic<css::lang::Locale*>   pLocale;

    static ScFieldEditEngine*   pFieldEditEngine;

    static void                 InitPPT();

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
    static css::lang::Locale*   GetLocale();

    SC_DLLPUBLIC static ::utl::TransliterationWrapper* GetpTransliteration();
    static ::utl::TransliterationWrapper* GetCaseTransliteration();

    SC_DLLPUBLIC static LanguageType            eLnge;
    static sal_Unicode          cListDelimiter;

    static const OUString&      GetClipDocName();
    static void                 SetClipDocName( const OUString& rNew );
    SC_DLLPUBLIC static ScAutoFormat*       GetAutoFormat();
    SC_DLLPUBLIC static ScAutoFormat*       GetOrCreateAutoFormat();
    static void                 ClearAutoFormat(); //BugId 54209
    static LegacyFuncCollection*      GetLegacyFuncCollection();
    SC_DLLPUBLIC static ScUnoAddInCollection* GetAddInCollection();
    SC_DLLPUBLIC static ScUserList*         GetUserList();
    static void                 SetUserList( const ScUserList* pNewList );
    /// Open the specified URL.
    static void                 OpenURL(const OUString& rURL, const OUString& rTarget, bool bBypassCtrlClickSecurity = false);
    SC_DLLPUBLIC static OUString            GetAbsDocName( const OUString& rFileName,
                                                const SfxObjectShell* pShell );
    SC_DLLPUBLIC static OUString            GetDocTabName( const OUString& rFileName,
                                                const OUString& rTabName );
    SC_DLLPUBLIC static sal_uInt32 GetStandardFormat( SvNumberFormatter&, sal_uInt32 nFormat, SvNumFormatType nType );

    SC_DLLPUBLIC static sal_uInt16 GetStandardRowHeight();
    /// Horizontal pixel per twips factor.
    SC_DLLPUBLIC static double              nScreenPPTX;
    /// Vertical pixel per twips factor.
    SC_DLLPUBLIC static double              nScreenPPTY;

    static tools::SvRef<ScDocShell>   xDrawClipDocShellRef;

    static sal_uInt16           nDefFontHeight;
    SC_DLLPUBLIC static sal_uInt16           nStdRowHeight;

    SC_DLLPUBLIC static long                nLastRowHeightExtra;
    static long             nLastColWidthExtra;

    SC_DLLPUBLIC static void Init();                     // during start up
    static void             InitAddIns();
    static void             Clear();                    // at the end of the program

    static void             InitTextHeight(const SfxItemPool* pPool);
    static SvxBrushItem*    GetEmptyBrushItem() { return pEmptyBrushItem; }
    static SvxBrushItem*    GetButtonBrushItem();
    SC_DLLPUBLIC    static const OUString&    GetEmptyOUString();

    static bool             HasStarCalcFunctionList();
    static ScFunctionList*  GetStarCalcFunctionList();
    static ScFunctionMgr*   GetStarCalcFunctionMgr();
    static void             ResetFunctionList();

    static OUString         GetErrorString(FormulaError nErrNumber);
    static OUString         GetLongErrorString(FormulaError nErrNumber);
    static bool             EETextObjEqual( const EditTextObject* pObj1,
                                            const EditTextObject* pObj2 );
    static bool             CheckWidthInvalidate( bool& bNumFormatChanged,
                                                  const SfxItemSet& rNewAttrs,
                                                  const SfxItemSet& rOldAttrs );
    static bool             HasAttrChanged( const SfxItemSet& rNewAttrs,
                                            const SfxItemSet& rOldAttrs,
                                            const sal_uInt16      nWhich );

    static ScUnitConverter* GetUnitConverter();

    /// strchr() functionality on unicode, as long as we need it for FormulaToken etc.
    static const sal_Unicode* UnicodeStrChr( const sal_Unicode* pStr, sal_Unicode c );

    static sal_Unicode ToUpperAlpha( sal_Unicode c )
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
    SC_DLLPUBLIC static bool             IsQuoted( const OUString& rString, sal_Unicode cQuote );

    /** Inserts the character cQuote at beginning and end of rString.
        @param bEscapeEmbedded      If <TRUE/>, embedded quote characters are
                                    escaped by doubling them.
     */
    SC_DLLPUBLIC static void             AddQuotes( OUString& rString, sal_Unicode cQuote, bool bEscapeEmbedded = true );

    /** Erases the character cQuote from rString, if it exists at beginning AND end.
        @param bUnescapeEmbedded    If <TRUE/>, embedded doubled quote characters
                                    are unescaped by replacing them with a
                                    single instance.
     */
    SC_DLLPUBLIC static void             EraseQuotes( OUString& rString, sal_Unicode cQuote, bool bUnescapeEmbedded = true );

    /** Finds an unquoted instance of cChar in rString, starting at
        offset nStart. Unquoted instances may occur when concatenating two
        quoted strings with a separator, for example, 's1':'s2'. Embedded
        quotes have to be escaped by being doubled. Caller must ensure that
        nStart points into an unquoted range or the opening quote. Specialty:
        if cChar==cQuote the first cQuote character from nStart on is found.
        @returns offset if found, else -1
     */
    SC_DLLPUBLIC static sal_Int32       FindUnquoted( const OUString& rString, sal_Unicode cChar);

    /** Finds an unquoted instance of cChar in null-terminated pString. Same
        semantics as FindUnquoted( const String&, ...)
        @returns: pointer to cChar if found, else NULL
     */
    SC_DLLPUBLIC static const sal_Unicode* FindUnquoted( const sal_Unicode* pString, sal_Unicode cChar );

    static  rtl_TextEncoding GetCharsetValue( const OUString& rCharSet );
    static  OUString        GetCharsetString( rtl_TextEncoding eVal );

    /// a "ReadOnly" formatter for UNO/XML export
    static  SvNumberFormatter*  GetEnglishFormatter();

    static bool IsSystemRTL();                      // depending on system language
    static LanguageType GetEditDefaultLanguage();   // for EditEngine::SetDefaultLanguage
    SC_DLLPUBLIC static SvtScriptType GetDefaultScriptType();             // for all WEAK characters
    /** Map ATTR_((CJK|CTL)_)?FONT_... to proper WhichIDs.
        If more than one SvtScriptType::... values are or'ed together, prefers
        first COMPLEX, then ASIAN */
    SC_DLLPUBLIC static sal_uInt16 GetScriptedWhichID( SvtScriptType nScriptType, sal_uInt16 nWhich );

    /** Adds a language item to the item set, if the number format item contains
        a language that differs from its parent's language. */
    SC_DLLPUBLIC static void             AddLanguage( SfxItemSet& rSet, const SvNumberFormatter& rFormatter );

    /** Obtain the ordinal suffix for a number according to the system locale */
    static OUString         GetOrdinalSuffix( sal_Int32 nNumber);

    /** A static instance of ScFieldEditEngine not capable of resolving
        document specific fields, to be used only by ScEditUtil::GetString(). */
    static ScFieldEditEngine&   GetStaticFieldEditEngine();

    /** Replaces the first occurrence of rPlaceholder in rString with
        rReplacement, or if rPlaceholder is not found appends one space if
        rString does not end in a space and appends rReplacement.

        Meant to be used with resource strings ala "Column %1" where a
        translation may have omitted the %1 placeholder and a simple
        replacement would end up with nothing replaced so no column indicator
        in the result string.
     */
    SC_DLLPUBLIC static OUString    ReplaceOrAppend( const OUString& rString,
                                                     const OUString& rPlaceholder,
                                                     const OUString& rReplacement );


    /** Convert string content to numeric value.

        In any case, if rError is set 0.0 is returned.

        If nStringNoValueError is FormulaError::CellNoValue, that is unconditionally
        assigned to rError and 0.0 is returned. The caller is expected to
        handle this situation. Used by the interpreter.

        Usually FormulaError::NoValue is passed as nStringNoValueError.

        Otherwise, depending on the string conversion configuration different
        approaches are taken:


        For ScCalcConfig::StringConversion::ILLEGAL
        The error value passed in nStringNoValueError is assigned to rError
        (and 0.0 returned).


        For ScCalcConfig::StringConversion::ZERO
        A zero value is returned and no error assigned.


        For ScCalcConfig::StringConversion::LOCALE

        If the string is empty or consists only of spaces, if "treat empty
        string as zero" is set 0.0 is returned, else nStringNoValueError
        assigned to rError (and 0.0 returned).

        Else a non-empty string is passed to the number formatter's scanner to
        be parsed locale dependent. If that does not detect a numeric value
        nStringNoValueError is assigned to rError (and 0.0 returned).

        If no number formatter was passed, the conversion falls back to
        UNAMBIGUOUS.


        For ScCalcConfig::StringConversion::UNAMBIGUOUS

        If the string is empty or consists only of spaces, if "treat empty
        string as zero" is set 0.0 is returned, else nStringNoValueError
        assigned to rError (and 0.0 returned).

        If the string is not empty the following conversion rules are applied:

        Converted are only integer numbers including exponent, and ISO 8601 dates
        and times in their extended formats with separators. Anything else,
        especially fractional numeric values with decimal separators or dates other
        than ISO 8601 would be locale dependent and is a no-no. Leading and
        trailing blanks are ignored.

        The following ISO 8601 formats are converted:

        CCYY-MM-DD
        CCYY-MM-DDThh:mm
        CCYY-MM-DDThh:mm:ss
        CCYY-MM-DDThh:mm:ss,s
        CCYY-MM-DDThh:mm:ss.s
        hh:mm
        hh:mm:ss
        hh:mm:ss,s
        hh:mm:ss.s

        The century CC may not be omitted and the two-digit year setting is not
        taken into account. Instead of the T date and time separator exactly one
        blank may be used.

        If a date is given, it must be a valid Gregorian calendar date. In this
        case the optional time must be in the range 00:00 to 23:59:59.99999...
        If only time is given, it may have any value for hours, taking elapsed time
        into account; minutes and seconds are limited to the value 59 as well.

        If the string can not be converted to a numeric value, the error value
        passed in nStringNoValueError is assigned to rError.


        @param rStr
            The string to be converted.

        @param rConfig
            The calculation configuration.

        @param rError
            Contains the error on return, if any. If an error was set before
            and the conversion did not result in an error, still 0.0 is
            returned.

        @param nStringNoValueError
            The error value to be assigned to rError if string could not be
            converted to number.

        @param pFormatter
            The number formatter to use in case of
            ScCalcConfig::StringConversion::LOCALE. Can but should not be
            nullptr in which case conversion falls back to
            ScCalcConfig::StringConversion::UNAMBIGUOUS and if a date is
            detected the null date is assumed to be the standard 1899-12-30
            instead of the configured null date.

        @param rCurFmtType
            Can be assigned a format type in case a date or time or date+time
            string was converted, e.g. SvNumFormatType::DATE or
            SvNumFormatType::TIME or a combination thereof.

     */
    static double ConvertStringToValue( const OUString& rStr, const ScCalcConfig& rConfig,
            FormulaError & rError, FormulaError nStringNoValueError,
            SvNumberFormatter* pFormatter, SvNumFormatType & rCurFmtType );

    /// Calc's threaded group calculation is in progress.
    SC_DLLPUBLIC static bool bThreadedGroupCalcInProgress;
};

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
        SUBTOTAL_FUNC_MED   = 12,
        SUBTOTAL_FUNC_SELECTION_COUNT = 13
    };

enum ScAggregateFunc
    {
        AGGREGATE_FUNC_AVE     = 1,
        AGGREGATE_FUNC_CNT     = 2,
        AGGREGATE_FUNC_CNT2    = 3,
        AGGREGATE_FUNC_MAX     = 4,
        AGGREGATE_FUNC_MIN     = 5,
        AGGREGATE_FUNC_PROD    = 6,
        AGGREGATE_FUNC_STD     = 7,
        AGGREGATE_FUNC_STDP    = 8,
        AGGREGATE_FUNC_SUM     = 9,
        AGGREGATE_FUNC_VAR     = 10,
        AGGREGATE_FUNC_VARP    = 11,
        AGGREGATE_FUNC_MEDIAN  = 12,
        AGGREGATE_FUNC_MODSNGL = 13,
        AGGREGATE_FUNC_LARGE   = 14,
        AGGREGATE_FUNC_SMALL   = 15,
        AGGREGATE_FUNC_PERCINC = 16,
        AGGREGATE_FUNC_QRTINC  = 17,
        AGGREGATE_FUNC_PERCEXC = 18,
        AGGREGATE_FUNC_QRTEXC  = 19
    };

class ScArea;

struct ScConsolidateParam
{
    SCCOL           nCol;                   // cursor position /
    SCROW           nRow;                   // or start of destination area respectively
    SCTAB           nTab;
    ScSubTotalFunc  eFunction;
    sal_uInt16      nDataAreaCount;         // number of data areas
    std::unique_ptr<ScArea[]> pDataAreas; // array of pointers into data areas
    bool            bByCol;
    bool            bByRow;
    bool            bReferenceData;         // reference source data

    ScConsolidateParam();
    ScConsolidateParam( const ScConsolidateParam& r );
    ~ScConsolidateParam();

    ScConsolidateParam& operator=       ( const ScConsolidateParam& r );
    bool                operator==      ( const ScConsolidateParam& r ) const;
    void                Clear           (); // = ClearDataAreas()+Members
    void                ClearDataAreas  ();
    void                SetAreas        ( std::unique_ptr<ScArea[]> pAreas, sal_uInt16 nCount );
};

extern SfxViewShell* pScActiveViewShell;
extern sal_uInt16 nScClickMouseModifier;
extern sal_uInt16 nScFillModeMouseModifier;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
