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
#ifndef INCLUDED_SW_INC_POOLFMT_HXX
#define INCLUDED_SW_INC_POOLFMT_HXX

#include <limits.h>
#include <editeng/frmdir.hxx>
#include <i18nlangtag/lang.h>

/** POOLCOLL-IDs:
// +----+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// !User!    Range      ! 0 !               Offset                  !
// +----+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//
//  Range:                          1 - text
//                                  2 - lists
//                                  3 - special ranges
//                                  4 - indices
//                                  5 - chapter / document
//                                  6 - HTML-styles.

// Other IDs:
// +----+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// !User!    Range      ! 1 !           Offset                      !
// +----+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Range:                           0 - character styles
//                                  1 - frame styles
//                                  2 - page styles
//                                  3 - paragraph styles (?)
//                                  4 - graphics styles (?)

// For all IDs we have:
// Origin:                          0 -pool
//                                  1 -user
// Offset:                          within the group */

/** Attention: New IDs can only be added to the ends of the groups.
   These IDs are read and written by the Reader/Writer.
   They are only aware of the Offset to Start. */

/// Mask for recognition of COLLPOOL-IDs:

const sal_uInt16 POOLGRP_NOCOLLID       =  (1 << 10);

/// POLLCOLL-groups:

const sal_uInt16 USER_FMT               =  (1 << 15);

const sal_uInt16 COLL_TEXT_BITS         =  (1 << 11);
const sal_uInt16 COLL_LISTS_BITS        =  (2 << 11);
const sal_uInt16 COLL_EXTRA_BITS        =  (3 << 11);
const sal_uInt16 COLL_REGISTER_BITS     =  (4 << 11);
const sal_uInt16 COLL_DOC_BITS          =  (5 << 11);
const sal_uInt16 COLL_HTML_BITS         =  (6 << 11);
const sal_uInt16 COLL_GET_RANGE_BITS    = (15 << 11);

/// Other groups:

const sal_uInt16 POOLGRP_CHARFMT        = (0 << 11) + POOLGRP_NOCOLLID;
const sal_uInt16 POOLGRP_FRAMEFMT       = (1 << 11) + POOLGRP_NOCOLLID;
const sal_uInt16 POOLGRP_PAGEDESC       = (2 << 11) + POOLGRP_NOCOLLID;
const sal_uInt16 POOLGRP_NUMRULE        = (3 << 11) + POOLGRP_NOCOLLID;
const sal_uInt16 POOLGRP_TABSTYLE       = (4 << 11) + POOLGRP_NOCOLLID;
const sal_uInt16 POOLGRP_CELLSTYLE      = (5 << 11) + POOLGRP_NOCOLLID;

/// IDs for the ranges.
enum class SwPoolFormatId : sal_uInt16
{
ZERO = 0,
RES_POOLFMT_BEGIN = 1,
RES_POOL_CHRFMT = RES_POOLFMT_BEGIN,
RES_POOL_FRMFMT,
RES_POOL_TXTCOLL,
RES_POOL_PAGEFMT,

RES_POOL_PARFMT,
RES_POOL_GRFFMT,
RES_POOLFMT_END,

/// Ranges for the IDs of the formats.

/// IDs for character styles.
CHR_BEGIN = POOLGRP_CHARFMT,
CHR_NORMAL_BEGIN = POOLGRP_CHARFMT,

CHR_FOOTNOTE = CHR_NORMAL_BEGIN,    ///< Footnote.
CHR_PAGENO,                                 ///< Pages/field.
CHR_LABEL,                                  ///< Label.
CHR_DROPCAPS,                               ///< Dropcaps.
CHR_NUM_LEVEL,                              ///< Numbering symbols
CHR_BULLET_LEVEL,                           ///< Bullets.

CHR_INET_NORMAL,                            ///< Internet normal.
CHR_INET_VISIT,                             ///< Internet visited.
CHR_JUMPEDIT,                               ///< Placeholder.
CHR_TOXJUMP,                                ///< Jump from index.
CHR_ENDNOTE,                                ///< Endnote.
CHR_LINENUM,                                ///< Line numbering.
CHR_IDX_MAIN_ENTRY,                         ///< Main entry in indices.
CHR_FOOTNOTE_ANCHOR,                        ///< Footnote anchor.
CHR_ENDNOTE_ANCHOR,                         ///< Endnote anchor.
CHR_RUBYTEXT,                               ///< Rubytext.
CHR_VERT_NUM,                               ///< Vertical numbering symbols.

CHR_NORMAL_END,

CHR_HTML_BEGIN = CHR_BEGIN + 50,    ///< HTML-styles.
CHR_HTML_EMPHASIS = CHR_HTML_BEGIN,
CHR_HTML_CITATION,
CHR_HTML_STRONG,
CHR_HTML_CODE,
CHR_HTML_SAMPLE,
CHR_HTML_KEYBOARD,
CHR_HTML_VARIABLE,
CHR_HTML_DEFINSTANCE,
CHR_HTML_TELETYPE,
CHR_HTML_END,

CHR_END = CHR_HTML_END,

/// IDs for frame styles.
FRM_BEGIN = POOLGRP_FRAMEFMT,

FRM_FRAME = FRM_BEGIN,              ///< Frame.
FRM_GRAPHIC,                                ///< Graphics.
FRM_OLE,                                    ///< OLE.
FRM_FORMEL,                                 ///< Formula.
FRM_MARGINAL,                               ///< Marginalia.
FRM_WATERSIGN,                              ///< Watermark.
FRM_LABEL,                                  ///< Labels.
FRM_INLINE_HEADING,                         ///< Inline Heading.

FRM_END,

/// IDs for page styles.
PAGE_BEGIN = POOLGRP_PAGEDESC,

PAGE_STANDARD = PAGE_BEGIN,         ///< Standard page.
PAGE_FIRST,                                 ///< First page.
PAGE_LEFT,                                  ///< Left page.
PAGE_RIGHT,                                 ///< Right page.
PAGE_ENVELOPE,                              ///< Envelope.
PAGE_REGISTER,                              ///< Index.
PAGE_HTML,                                  ///< HTML.
PAGE_FOOTNOTE,                              ///< Footnote at end of document.
PAGE_ENDNOTE,                               ///< Endnote page.
PAGE_LANDSCAPE,                             ///< Landscape page style.

PAGE_END,

// IDs for list styles.
NUMRULE_BEGIN = POOLGRP_NUMRULE,
NUMRULE_NOLIST = NUMRULE_BEGIN,
NUMRULE_NUM1,                               ///< NumRule Numbering 123.
NUMRULE_NUM2,                               ///< NumRule Numbering ABC.
NUMRULE_NUM3,                               ///< NumRule Numbering abc.
NUMRULE_NUM4,                               ///< NumRule Numbering IVX.
NUMRULE_NUM5,                               ///< NumRule Numbering ivx.
NUMRULE_BUL1,                               ///< NumRule Bullets 1.
NUMRULE_BUL2,                               ///< NumRule Bullets 2.
NUMRULE_BUL3,                               ///< NumRule Bullets 3.
NUMRULE_BUL4,                               ///< NumRule Bullets 4.
NUMRULE_BUL5,                               ///< NumRule Bullets 5.
NUMRULE_END,

// IDs for table styles.
TABLESTYLE_BEGIN = POOLGRP_TABSTYLE,
TABLESTYLE_DEFAULT = TABLESTYLE_BEGIN,
// 16 old styles
TABLESTYLE_3D,
TABLESTYLE_BLACK1,
TABLESTYLE_BLACK2,
TABLESTYLE_BLUE,
TABLESTYLE_BROWN,
TABLESTYLE_CURRENCY,
TABLESTYLE_CURRENCY_3D,
TABLESTYLE_CURRENCY_GRAY,
TABLESTYLE_CURRENCY_LAVENDER,
TABLESTYLE_CURRENCY_TURQUOISE,
TABLESTYLE_GRAY,
TABLESTYLE_GREEN,
TABLESTYLE_LAVENDER,
TABLESTYLE_RED,
TABLESTYLE_TURQUOISE,
TABLESTYLE_YELLOW,
// 10 new styles since LibreOffice 6.0
TABLESTYLE_LO6_ACADEMIC,
TABLESTYLE_LO6_BOX_LIST_BLUE,
TABLESTYLE_LO6_BOX_LIST_GREEN,
TABLESTYLE_LO6_BOX_LIST_RED,
TABLESTYLE_LO6_BOX_LIST_YELLOW,
TABLESTYLE_LO6_ELEGANT,
TABLESTYLE_LO6_FINANCIAL,
TABLESTYLE_LO6_SIMPLE_GRID_COLUMNS,
TABLESTYLE_LO6_SIMPLE_GRID_ROWS,
TABLESTYLE_LO6_SIMPLE_LIST_SHADED,
TABLESTYLE_END,

CELLSTYLE_BEGIN = POOLGRP_CELLSTYLE,
CELLSTYLE_END = CELLSTYLE_BEGIN,

// IDs for paragraph styles.
// Group text.
COLL_TEXT_BEGIN = COLL_TEXT_BITS,

COLL_STANDARD = COLL_TEXT_BEGIN,        ///< Standard.
COLL_TEXT,                                      ///< Text body.
COLL_TEXT_IDENT,                                ///< Text body first line indent.
COLL_TEXT_NEGIDENT,                             ///< Text body hanging indent.
COLL_TEXT_MOVE,                                 ///< Text body indent.
COLL_GREETING,                                  ///< Complimentary close.
COLL_SIGNATURE,                                 ///< Signature.
COLL_CONFRONTATION,                             ///< List indent.
COLL_MARGINAL,                                  ///< Marginalia.

COLL_TEXT_END,

/// Group lists.
COLL_LISTS_BEGIN = COLL_LISTS_BITS,

COLL_NUMBER_BULLET_BASE = COLL_LISTS_BEGIN,    ///< Base list.

/// Subgroup numberings.
COLL_NUM_LEVEL1S,                               ///< Start 1st level.
COLL_NUM_LEVEL1,                                ///< 1st level.
COLL_NUM_LEVEL1E,                               ///< End 1st level.
COLL_NUM_NONUM1,                                ///< No numbering.
COLL_NUM_LEVEL2S,                               ///< Start 2nd level.
COLL_NUM_LEVEL2,                                ///< 2nd level.
COLL_NUM_LEVEL2E,                               ///< End 2nd level.
COLL_NUM_NONUM2,                                ///< No numbering.
COLL_NUM_LEVEL3S,                               ///< Start 3rd level.
COLL_NUM_LEVEL3,                                ///< 3rd level.
COLL_NUM_LEVEL3E,                               ///< End 3rd level.
COLL_NUM_NONUM3,                                ///< No numbering.
COLL_NUM_LEVEL4S,                               ///< Start 4th level.
COLL_NUM_LEVEL4,                                ///< 4th level.
COLL_NUM_LEVEL4E,                               ///< End 4th level.
COLL_NUM_NONUM4,                                ///< No numbering.
COLL_NUM_LEVEL5S,                               ///< Start 5th level.
COLL_NUM_LEVEL5,                                ///< 5th level.
COLL_NUM_LEVEL5E,                               ///< End 5th level.
COLL_NUM_NONUM5,                                ///< No numbering.

///Subgroup bullets.
COLL_BULLET_LEVEL1S,                               ///< Start 1st level.
COLL_BULLET_LEVEL1,                                ///< 1st level.
COLL_BULLET_LEVEL1E,                               ///< End 1st level
COLL_BULLET_NONUM1,                                ///< No numbering.
COLL_BULLET_LEVEL2S,                               ///< Start 2nd level.
COLL_BULLET_LEVEL2,                                ///< 2nd level.
COLL_BULLET_LEVEL2E,                               ///< End 2nd level.
COLL_BULLET_NONUM2,                                ///< No numbering.
COLL_BULLET_LEVEL3S,                               ///< Start 3rd level.
COLL_BULLET_LEVEL3,                                ///< 3rd Level.
COLL_BULLET_LEVEL3E,                               ///< End 3rd level.
COLL_BULLET_NONUM3,                                ///< No numbering.
COLL_BULLET_LEVEL4S,                               ///< Start 4th level.
COLL_BULLET_LEVEL4,                                ///< 4th level.
COLL_BULLET_LEVEL4E,                               ///< End 4th level.
COLL_BULLET_NONUM4,                                ///< No numbering.
COLL_BULLET_LEVEL5S,                               ///< Start 5th level.
COLL_BULLET_LEVEL5,                                ///< 5th level.
COLL_BULLET_LEVEL5E,                               ///< End 5th Level.
COLL_BULLET_NONUM5,                                ///< No numbering.

COLL_LISTS_END,

/// Special ranges.
COLL_EXTRA_BEGIN = COLL_EXTRA_BITS,

/// Subgroup header.
COLL_HEADERFOOTER = COLL_EXTRA_BEGIN,   ///< Header and Footer.
COLL_HEADER,                                    ///< Header Left&Right.
COLL_HEADERL,                                   ///< Header Left.
COLL_HEADERR,                                   ///< Header Right.

/// Subgroup footer.
COLL_FOOTER,                                    ///< Footer Left&Right.
COLL_FOOTERL,                                   ///< Footer Left.
COLL_FOOTERR,                                   ///< Footer Right.

/// Subgroup table.
COLL_TABLE,                                     ///< Table of Contents.
COLL_TABLE_HDLN,                                ///< Table of Contents - heading.

/// Subgroup labels.
COLL_LABEL,                                     ///< Base labels.
COLL_LABEL_ABB,                                 ///< Label illustration.
COLL_LABEL_TABLE,                               ///< Label table.
COLL_LABEL_FRAME,                               ///< Label frame.
COLL_LABEL_FIGURE,                              ///< Label figure

/// Other stuff.
COLL_FRAME,                                     ///< Frames.
COLL_FOOTNOTE,                                  ///< Footnotes.
COLL_ENVELOPE_ADDRESS,                          ///< Addressee.
COLL_SEND_ADDRESS,                              ///< Sender.
COLL_ENDNOTE,                                   ///< Endnotes.

COLL_LABEL_DRAWING,                             ///< Label drawing objects.
COLL_COMMENT,                                   ///< Comment

COLL_EXTRA_END,

/// Group indices.
COLL_REGISTER_BEGIN = COLL_REGISTER_BITS,

COLL_REGISTER_BASE = COLL_REGISTER_BEGIN,   ///< Base index.

/// Subgroup index tables.
COLL_TOX_IDXH,                                  ///< Header.
COLL_TOX_IDX1,                                  ///< 1st level.
COLL_TOX_IDX2,                                  ///< 2nd level.
COLL_TOX_IDX3,                                  ///< 3rd level.
COLL_TOX_IDXBREAK,                              ///< Separator.

/// Subgroup table of contents.
COLL_TOX_CNTNTH,                                ///< Header.
COLL_TOX_CNTNT1,                                ///< Content 1st level.
COLL_TOX_CNTNT2,                                ///< Content 2nd level.
COLL_TOX_CNTNT3,                                ///< Content 3rd level.
COLL_TOX_CNTNT4,                                ///< Content 4th level.
COLL_TOX_CNTNT5,                                ///< Content 5th level.

/// Subgroup user indices.
COLL_TOX_USERH,                                 ///< Header.
COLL_TOX_USER1,                                 ///< 1st level.
COLL_TOX_USER2,                                 ///< 2nd level.
COLL_TOX_USER3,                                 ///< 3rd level.
COLL_TOX_USER4,                                 ///< 4th level.
COLL_TOX_USER5,                                 ///< 5th level.

COLL_TOX_CNTNT6,                                ///< Content  6th level.
COLL_TOX_CNTNT7,                                ///< Content  7th level.
COLL_TOX_CNTNT8,                                ///< Content  8th level.
COLL_TOX_CNTNT9,                                ///< Content  9th level.
COLL_TOX_CNTNT10,                               ///< Content 10th level.

/// illustrations index.
COLL_TOX_ILLUSH,                                ///< Illustrations header.
COLL_TOX_ILLUS1,                                ///< Illustrations all levels.

///  object index.
COLL_TOX_OBJECTH,                               ///< Objects header.
COLL_TOX_OBJECT1,                               ///< Objects all levels.

///  tables index.
COLL_TOX_TABLESH,                               ///< Tables header.
COLL_TOX_TABLES1,                               ///< Tables all levels.

///  index of authorities.
COLL_TOX_AUTHORITIESH,                          ///< Authorities header.
COLL_TOX_AUTHORITIES1,                          ///< Authorities all levels.

/// user index 6..10.
COLL_TOX_USER6,                                 ///< 6th  level.
COLL_TOX_USER7,                                 ///< 7th  level.
COLL_TOX_USER8,                                 ///< 8th  level.
COLL_TOX_USER9,                                 ///< 9th  level.
COLL_TOX_USER10,                                ///< 10th level.

COLL_REGISTER_END,

/// Group chapter / document.
COLL_DOC_BEGIN = COLL_DOC_BITS,

COLL_DOC_TITLE = COLL_DOC_BEGIN,        ///< Doc. title.
COLL_DOC_SUBTITLE,                              ///< Doc. subtitle.
COLL_DOC_APPENDIX,                              ///< Doc. appendix.

/// Subgroup headings.
COLL_HEADLINE_BASE, ///< Base heading.
COLL_HEADLINE1, ///< Heading 1.
COLL_HEADLINE2, ///< Heading 2.
COLL_HEADLINE3, ///< Heading 3.
COLL_HEADLINE4, ///< Heading 4.
COLL_HEADLINE5, ///< Heading 5.
COLL_HEADLINE6, ///< Heading 6.
COLL_HEADLINE7, ///< Heading 7.
COLL_HEADLINE8, ///< Heading 8.
COLL_HEADLINE9, ///< Heading 9.
COLL_HEADLINE10, ///< Heading 10.

COLL_DOC_END,

/// Group HTML-styles.
COLL_HTML_BEGIN = COLL_HTML_BITS,

COLL_HTML_BLOCKQUOTE = COLL_HTML_BEGIN,
COLL_HTML_PRE,
COLL_HTML_HR,
COLL_HTML_DD,
COLL_HTML_DT,

COLL_HTML_END,

/// End of text styles collection.

UNKNOWN = USHRT_MAX

};
/// End of SwPoolFormatId

// We do a fair amount of calculation on these values to extract sub-ranges, etc, so
// add some operators to make that less verbose.
inline constexpr SwPoolFormatId operator-(SwPoolFormatId lhs, SwPoolFormatId rhs)
{ return SwPoolFormatId(sal_uInt16(lhs) - sal_uInt16(rhs)); }
inline constexpr SwPoolFormatId operator&(SwPoolFormatId lhs, SwPoolFormatId rhs)
{ return SwPoolFormatId(sal_uInt16(lhs) & sal_uInt16(rhs)); }
inline constexpr SwPoolFormatId operator+(SwPoolFormatId lhs, sal_uInt16 rhs)
{ return SwPoolFormatId(sal_uInt16(lhs) + rhs); }
inline constexpr SwPoolFormatId operator-(SwPoolFormatId lhs, sal_uInt16 rhs)
{ return SwPoolFormatId(sal_uInt16(lhs) - rhs); }

/// Recognize whether it's a user defined style or not:
const sal_uInt16 POOL_IDUSER_FMT =
        USHRT_MAX & ~(COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID);

inline bool IsPoolUserFormat( SwPoolFormatId nId )
{
    return POOL_IDUSER_FMT == (sal_uInt16(nId) & ~(COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID));
}

/** Query defined parent of a POOL-ID
   Returns  0 if standard
            SwPoolFormatId::UNKNOWN if no parent
            the parent in all other cases. */
SwPoolFormatId GetPoolParent( SwPoolFormatId nId );

SvxFrameDirection GetDefaultFrameDirection(LanguageType nLanguage);

bool IsConditionalByPoolId(SwPoolFormatId nId);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
