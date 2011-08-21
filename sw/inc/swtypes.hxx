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
#ifndef _SWTYPES_HXX
#define _SWTYPES_HXX
#include <tools/solar.h>
#include <SwGetPoolIdFromName.hxx>

#ifndef INCLUDED_LIMITS_H
#include <limits.h>     //For LONG_MAX.
#define INCLUDED_LIMITS_H
#endif

#include <com/sun/star/uno/Reference.h>
#include "swdllapi.h"
#include <i18npool/lang.h>

namespace com { namespace sun { namespace star {
    namespace linguistic2{
        class XDictionaryList;
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
class UniString;
class ByteString;
class SwPathFinder;
class Graphic;
class OutputDevice;
class CharClass;
class LocaleDataWrapper;
class CollatorWrapper;

typedef long SwTwips;
#define INVALID_TWIPS   LONG_MAX
#define TWIPS_MAX       (LONG_MAX - 1)

#define MM50   283  // 1/2 cm in TWIPS.

const SwTwips cMinHdFtHeight = 56;

#define MINFLY 23   // Minimal size for FlyFrms.
#define MINLAY 23   // Minimal size for other Frms.

// Default column distance of two text columns corresponds to 0.3 cm.
#define DEF_GUTTER_WIDTH (MM50 / 5 * 3)

// Minimal distance (distance to text) for border attribute
// in order not to crock up aligned lines.
// 28 Twips == 0,4mm
#define MIN_BORDER_DIST 28

// Minimal document border.
const SwTwips lMinBorder = 1134;

// Margin left and above document.
// Half of it is gap between the pages.
#define DOCUMENTBORDER  568L
#define GAPBETWEENPAGES 284L

#define UNDO_ACTION_COUNT 20

// Constant strings.
SW_DLLPUBLIC extern UniString aEmptyStr;    // ""
extern ByteString aEmptyByteStr;            // ""
SW_DLLPUBLIC extern UniString aDotStr;      // '.'

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


const sal_uInt8 OLD_MAXLEVEL = 5;
const sal_uInt8 MAXLEVEL = 10;      // Was: numrule.hxx.
const sal_uInt8 NO_NUM      = 200;  // Was:  numrule.hxx.


// For paragraphs with NO_NUM but on different levels.
// This makes the NO_NUM inapplicable.
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
// There are IDs for the anchors (SwFmtAnchor) and some others
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


extern ResMgr* pSwResMgr;           // Is in swapp0.cxx.
#define SW_RES(i)       ResId(i,*pSwResMgr)
#define SW_RESSTR(i)    UniString(ResId(i,*pSwResMgr))

::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XSpellChecker1 > GetSpellChecker();
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XHyphenator >    GetHyphenator();
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XThesaurus >     GetThesaurus();
::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >         GetLinguPropertySet();

// Returns the twip size of this graphic.
SW_DLLPUBLIC Size GetGraphicSizeTwip( const Graphic&, OutputDevice* pOutDev );


// Separator for jumps to different content types in document.
const sal_Unicode cMarkSeperator = '|';
extern const sal_Char* pMarkToTable;        // Strings are
extern const sal_Char* pMarkToFrame;        // in Init.cxx.
extern const sal_Char* pMarkToRegion;
SW_DLLPUBLIC extern const sal_Char* pMarkToOutline;
extern const sal_Char* pMarkToText;
extern const sal_Char* pMarkToGraphic;
extern const sal_Char* pMarkToOLE;

#ifndef DB_DELIM                            // This is defined in OFA!
#define DB_DELIM ((sal_Unicode)0xff)        // Database <-> table separator.
#endif


typedef sal_uInt16 SetAttrMode;

namespace nsSetAttrMode
{
    const SetAttrMode SETATTR_DEFAULT        = 0x0000;  // Default.
    const SetAttrMode SETATTR_DONTEXPAND     = 0x0001;  // Don't expand text attribute any further.
    const SetAttrMode SETATTR_DONTREPLACE    = 0x0002;  // Don't replace another text attribute.

    const SetAttrMode SETATTR_NOTXTATRCHR    = 0x0004;  // Don't insert 0xFF at attributes with no end.
    const SetAttrMode SETATTR_NOHINTADJUST   = 0x0008;  // No merging of ranges.
    const SetAttrMode SETATTR_NOFORMATATTR   = 0x0010;  // Do not change into format attribute.
    const SetAttrMode SETATTR_DONTCHGNUMRULE = 0x0020;  // Do not change NumRule.
    const SetAttrMode SETATTR_APICALL        = 0x0040;  // Called from API (all UI related
                                                        // functionality will be disabled).
    /// Force hint expand (only matters for hints with CH_TXTATR).
    const SetAttrMode SETATTR_FORCEHINTEXPAND= 0x0080;
    /// The inserted item is a copy -- intended for use in ndtxt.cxx.
    const SetAttrMode SETATTR_IS_COPY        = 0x0100;
}

// Conversion Twip <-> 1/100 mm for UNO

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))
#define TWIP_TO_MM100_UNSIGNED(TWIP)     ((((TWIP)*127L+36L)/72L))
#define MM100_TO_TWIP_UNSIGNED(MM100)    ((((MM100)*72L+63L)/127L))

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


#if 0
// I18N doesn't get this right, can't specify more than one to ignore
#define SW_COLLATOR_IGNORES ( \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE | \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_KANA | \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_WIDTH )
#else
#define SW_COLLATOR_IGNORES ( \
    ::com::sun::star::i18n::CollatorOptions::CollatorOptions_IGNORE_CASE )
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
    PREP_ADJUST_FRM,        // Adjust size via grow/shrink without formating.
    PREP_FLY_CHGD,          // A FlyFrm has changed its size.
    PREP_FLY_ATTR_CHG,      // A FlyFrm hat has changed its attributes
                            // (e. g. wrap).
    PREP_FLY_ARRIVE,        // A FlyFrm now overlaps range.
    PREP_FLY_LEAVE,         // A FlyFrm has left range.
    PREP_FTN,               // Invalidation of footnotes.
    PREP_POS_CHGD,          // Position of Frm has changed.
                            // (Check for Fly-break). In void* of Prepare()
                            // a sal_Bool& is passed. If this is sal_True,
                            // it indicateds that a format has been executed.
    PREP_UL_SPACE,          // UL-Space has changed, TxtFrms have to
                            // re-calculate line space.
    PREP_MUST_FIT,          // Make frm fit (split) even if the attributes do
                            // not allow that (e.g. "keep together").
    PREP_WIDOWS,            // A follow realizes that the orphan rule will be applied
                            // for it and sends a PREP_WIDOWS to its predecessor
                            // (Master/Follow).
    PREP_QUOVADIS,          // If a footnote has to be split between two paragraphs
                            // the last on the page has to receive a QUOVADIS in or-
                            // der to format the text into it.
    PREP_BOSS_CHGD,         // If a Frm changes its column/page this additional
                            // Prepare is sended to POS_CHGD in MoveFwd/Bwd
                            // (join Ftn-numbers etc.)
                            // Direction is communicated via pVoid:
                            //     MoveFwd: pVoid == 0
                            //     MoveBwd: pVoid == pOldPage
    PREP_SWAP,              // Swap graphic; for graphics in visible area.
    PREP_REGISTER,          // Invalidate frames with registers.
    PREP_FTN_GONE,          // A Follow loses its footnote, possibly its first line can move up.
    PREP_MOVEFTN,           // A footnote changes its page. Its contents receives at first a
                            // height of zero in order to avoid too much noise. At formating
                            // it checks whether it fits and if necessary changes its page again.
    PREP_ERGOSUM,           // Needed because of movement in FtnFrms. Check QuoVadis/ErgoSum.
    PREP_END                // END.
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
