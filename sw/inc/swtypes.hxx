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
#ifndef INCLUDED_SW_INC_SWTYPES_HXX
#define INCLUDED_SW_INC_SWTYPES_HXX
#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <tools/mapunit.hxx>
#include <SwGetPoolIdFromName.hxx>

#include <limits.h>
#include <com/sun/star/uno/Reference.h>
#include "swdllapi.h"
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/typed_flags_set.hxx>

namespace com { namespace sun { namespace star {
    namespace linguistic2{
        class XDictionaryList;
        class XLinguProperties;
        class XSpellChecker1;
        class XHyphenator;
        class XThesaurus;
    }
    namespace beans{
        class XPropertySet;
    }
}}}
namespace utl{
    class TransliterationWrapper;
}

class Size;
class ResMgr;
class SwPathFinder;
class Graphic;
class OutputDevice;
class CharClass;
class LocaleDataWrapper;
class CollatorWrapper;

typedef long SwTwips;
#define INVALID_TWIPS   LONG_MAX
#define TWIPS_MAX       (LONG_MAX - 1)

// Converts Millimeters to Twips (1 mm == 56.905479 twips).
template <typename T = SwTwips>
static SAL_CONSTEXPR T MmToTwips(const double mm) { return static_cast<T>(mm / 0.017573); }

#define MM50   283  // 1/2 cm in TWIPS.

const sal_Int32 COMPLETE_STRING = SAL_MAX_INT32;

const SwTwips cMinHdFtHeight = 56;

#define MINFLY 23   // Minimal size for FlyFrames.
#define MINLAY 23   // Minimal size for other Frames.

// Default column distance of two text columns corresponds to 0.3 cm.
#define DEF_GUTTER_WIDTH (MM50 / 5 * 3)

// Minimal distance (distance to text) for border attribute
// in order not to crock up aligned lines.
// 28 Twips == 0,4mm
#define MIN_BORDER_DIST 28

// Minimal document border: 20mm.
const SwTwips lMinBorder = 1134;

// Margin left and above document.
// Half of it is gap between the pages.
//TODO: Replace with SwViewOption::GetDefDocumentBorder()
#define DOCUMENTBORDER  284L

// Constant strings.
SW_DLLPUBLIC extern OUString aEmptyOUStr;  // remove once aEmptyOUStr can be changed to OUString

// For inserting of captions (what and where to insert).
// It's here because it is not big enough to justify its own hxx
// and does not seem to fit somewhere else.
enum SwLabelType
{
    LTYPE_TABLE,    // Caption for a table.
    LTYPE_OBJECT,   // Caption for a graphic or OLE.
    LTYPE_FLY,      // Caption for a text frame.
    LTYPE_DRAW      // Caption for a draw object.
};

const sal_uInt8 MAXLEVEL = 10;

const sal_uInt8 NO_NUMLEVEL  = 0x20;    // "or" with the levels.

// Some helper functions as macros or inlines.

// One kilobyte is 1024 bytes:
#define KB 1024

#define SET_CURR_SHELL( shell ) CurrShell aCurr( shell )

// pPathFinder is initialized by the UI.
// The class delivers all paths needed.
extern SwPathFinder *pPathFinder;

//  Values for indents at numbering and bullet lists.
//  (For more levels the values have to be multiplied with the levels+1;
//  levels 0 ..4!)

const sal_uInt16 lBullIndent = 1440/4;
const short lBullFirstLineOffset = -lBullIndent;
const sal_uInt16 lNumIndent = 1440/4;
const short lNumFirstLineOffset = -lNumIndent;
const short lOutlineMinTextDistance = 216; // 0.15 inch = 0.38 cm

// Count of SystemField-types of SwDoc.
#define INIT_FLDTYPES   32

// Count of predefined Seq-field types. It is always the last
// fields before INIT_FLDTYPES.
#define INIT_SEQ_FLDTYPES   4

// The former Rendevouz-IDs live on:
// There are IDs for the anchors (SwFormatAnchor) and some others
// that are only of importance for interfaces (SwDoc).
enum RndStdIds
{
    FLY_AT_PARA,        // Anchored at paragraph.
    FLY_AS_CHAR,        // Anchored as character.
    FLY_AT_PAGE,        // Anchored at page.
    FLY_AT_FLY,         // Anchored at frame.
    FLY_AT_CHAR,        // Anchored at character.

    RND_STD_HEADER,
    RND_STD_FOOTER,
    RND_STD_HEADERL,
    RND_STD_HEADERR,
    RND_STD_FOOTERL,
    RND_STD_FOOTERR,

    RND_DRAW_OBJECT     // A draw-Object! For the SwDoc-interface only!
};

extern ResMgr* pSwResMgr;
    // defined in sw/source/uibase/app/swmodule.cxx for the sw library and in
    // sw/source/ui/dialog/swdialmgr.cxx for the swui library
#define SW_RES(i)       ResId(i,*pSwResMgr)
#define SW_RESSTR(i)    SW_RES(i).toString()

css::uno::Reference< css::linguistic2::XSpellChecker1 > GetSpellChecker();
css::uno::Reference< css::linguistic2::XHyphenator >    GetHyphenator();
css::uno::Reference< css::linguistic2::XThesaurus >     GetThesaurus();
css::uno::Reference< css::linguistic2::XLinguProperties > GetLinguPropertySet();

// Returns the twip size of this graphic.
SW_DLLPUBLIC Size GetGraphicSizeTwip( const Graphic&, OutputDevice* pOutDev );

// Separator for jumps to different content types in document.
const sal_Unicode cMarkSeparator = '|';
// Sequences names for jumps are <name of sequence>!<no>
const char cSequenceMarkSeparator = '!';

#define DB_DELIM ((sal_Unicode)0xff)        // Database <-> table separator.

enum class SetAttrMode
{
    DEFAULT         = 0x0000,  // Default.
    /// @attention: DONTEXPAND does not work very well for CHARATR
    ///             because it can expand only the whole AUTOFMT or nothing
    DONTEXPAND      = 0x0001,  // Don't expand text attribute any further.
    DONTREPLACE     = 0x0002,  // Don't replace another text attribute.

    NOTXTATRCHR     = 0x0004,  // Don't insert 0xFF at attributes with no end.
    /// attention: NOHINTADJUST prevents MergePortions!
    /// when using this need to pay attention to ignore start/end flags of hint
    NOHINTADJUST    = 0x0008,  // No merging of ranges.
    NOFORMATATTR    = 0x0010,  // Do not change into format attribute.
    APICALL         = 0x0020,  // Called from API (all UI related
                                                        // functionality will be disabled).
    /// Force hint expand (only matters for hints with CH_TXTATR).
    FORCEHINTEXPAND = 0x0040,
    /// The inserted item is a copy -- intended for use in ndtxt.cxx.
    IS_COPY         = 0x0080
};
namespace o3tl
{
    template<> struct typed_flags<SetAttrMode> : is_typed_flags<SetAttrMode, 0x0ff> {};
}

#define SW_ISPRINTABLE( c ) ( c >= ' ' && 127 != c )

#ifndef SW_CONSTASCII_DECL
#define SW_CONSTASCII_DECL( n, s ) n[sizeof(s)]
#endif
#ifndef SW_CONSTASCII_DEF
#define SW_CONSTASCII_DEF( n, s ) n[sizeof(s)] = s
#endif

#define CHAR_HARDBLANK      ((sal_Unicode)0x00A0)
#define CHAR_HARDHYPHEN     ((sal_Unicode)0x2011)
#define CHAR_SOFTHYPHEN     ((sal_Unicode)0x00AD)
#define CHAR_RLM            ((sal_Unicode)0x200F)
#define CHAR_LRM            ((sal_Unicode)0x200E)
#define CHAR_ZWSP           ((sal_Unicode)0x200B)
#define CHAR_ZWNBSP         ((sal_Unicode)0x2060)

// Returns the APP - CharClass instance - used for all ToUpper/ToLower/...
SW_DLLPUBLIC CharClass& GetAppCharClass();
SW_DLLPUBLIC LanguageType GetAppLanguage();
SW_DLLPUBLIC const LanguageTag& GetAppLanguageTag();

#if 0
// I18N doesn't get this right, can't specify more than one to ignore
#define SW_COLLATOR_IGNORES ( \
    css::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE | \
    css::i18n::CollatorOptions::CollatorOptions_IGNORE_KANA | \
    css::i18n::CollatorOptions::CollatorOptions_IGNORE_WIDTH )
#else
#define SW_COLLATOR_IGNORES ( \
    css::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE )
#endif

SW_DLLPUBLIC CollatorWrapper& GetAppCollator();
SW_DLLPUBLIC CollatorWrapper& GetAppCaseCollator();

SW_DLLPUBLIC const ::utl::TransliterationWrapper& GetAppCmpStrIgnore();

// Official shortcut for Prepare() regarding notification of Content by the Layout.
// Content provides for calculation of minimal requirements at the next call of ::Format().
enum PrepareHint
{
    PREP_BEGIN,             // BEGIN.
    PREP_CLEAR = PREP_BEGIN,// Reformat completely.
    PREP_WIDOWS_ORPHANS,    // Only check for widows and orphans and split in case of need.
    PREP_FIXSIZE_CHG,       // FixSize has changed.
    PREP_FOLLOW_FOLLOWS,    // Follow is now possibly adjacent.
    PREP_ADJUST_FRM,        // Adjust size via grow/shrink without formatting.
    PREP_FLY_CHGD,          // A FlyFrame has changed its size.
    PREP_FLY_ATTR_CHG,      // A FlyFrame hat has changed its attributes
                            // (e. g. wrap).
    PREP_FLY_ARRIVE,        // A FlyFrame now overlaps range.
    PREP_FLY_LEAVE,         // A FlyFrame has left range.
    PREP_FTN,               // Invalidation of footnotes.
    PREP_POS_CHGD,          // Position of Frame has changed.
                            // (Check for Fly-break). In void* of Prepare()
                            // a sal_Bool& is passed. If this is sal_True,
                            // it indicates that a format has been executed.
    PREP_UL_SPACE,          // UL-Space has changed, TextFrames have to
                            // re-calculate line space.
    PREP_MUST_FIT,          // Make frm fit (split) even if the attributes do
                            // not allow that (e.g. "keep together").
    PREP_WIDOWS,            // A follow realizes that the orphan rule will be applied
                            // for it and sends a PREP_WIDOWS to its predecessor
                            // (Master/Follow).
    PREP_QUOVADIS,          // If a footnote has to be split between two paragraphs
                            // the last on the page has to receive a QUOVADIS in
                            // order to format the text into it.
    PREP_BOSS_CHGD,         // If a Frame changes its column/page this additional
                            // Prepare is sended to POS_CHGD in MoveFwd/Bwd
                            // (join Footnote-numbers etc.)
                            // Direction is communicated via pVoid:
                            //     MoveFwd: pVoid == 0
                            //     MoveBwd: pVoid == pOldPage
    PREP_REGISTER,          // Invalidate frames with registers.
    PREP_FTN_GONE,          // A Follow loses its footnote, possibly its first line can move up.
    PREP_MOVEFTN,           // A footnote changes its page. Its contents receives at first a
                            // height of zero in order to avoid too much noise. At formatting
                            // it checks whether it fits and if necessary changes its page again.
    PREP_ERGOSUM,           // Needed because of movement in FootnoteFrames. Check QuoVadis/ErgoSum.
};

enum FrameControlType
{
    PageBreak,
    Header,
    Footer
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
