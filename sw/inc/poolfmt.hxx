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
#ifndef _POOLFMT_HXX
#define _POOLFMT_HXX

#include <limits.h>
#include <tools/solar.h>
#include <editeng/frmdir.hxx>

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
const sal_uInt16 POOL_FMT               =  (0 << 15);

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

/// Recognize whether it's a user defined style or not:
const sal_uInt16 POOL_IDUSER_FMT =
        USHRT_MAX & ~(COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID);

inline sal_Bool IsPoolUserFmt( sal_uInt16 nId )
{
    return POOL_IDUSER_FMT ==
                (nId & ~(COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID))
        ? sal_True : sal_False;
}



/// IDs for the ranges.
enum RES_POOLFMT
{
RES_POOLFMT_BEGIN = 1,
RES_POOL_CHRFMT = RES_POOLFMT_BEGIN,
RES_POOL_FRMFMT,
RES_POOL_TXTCOLL,
RES_POOL_PAGEFMT,

RES_POOL_PARFMT,
RES_POOL_GRFFMT,
RES_POOLFMT_END
};

/// Ranges for the IDs of the formats.

/// IDs for character styles.
enum RES_POOL_CHRFMT_TYPE
{
RES_POOLCHR_BEGIN = POOLGRP_CHARFMT,
RES_POOLCHR_NORMAL_BEGIN = POOLGRP_CHARFMT,

RES_POOLCHR_FOOTNOTE = RES_POOLCHR_NORMAL_BEGIN,    ///< Footnote.
RES_POOLCHR_PAGENO,                                 ///< Pages/field.
RES_POOLCHR_LABEL,                                  ///< Label.
RES_POOLCHR_DROPCAPS,                               ///< Dropcaps.
RES_POOLCHR_NUM_LEVEL,                              ///< Numbering symbols
RES_POOLCHR_BUL_LEVEL,                              ///< Bullets.

RES_POOLCHR_INET_NORMAL,                            ///< Internet normal.
RES_POOLCHR_INET_VISIT,                             ///< Internet visited.
RES_POOLCHR_JUMPEDIT,                               ///< Placeholder.
RES_POOLCHR_TOXJUMP,                                ///< Jump from index.
RES_POOLCHR_ENDNOTE,                                ///< Endnote.
RES_POOLCHR_LINENUM,                                ///< Line numbering.
RES_POOLCHR_IDX_MAIN_ENTRY,                         ///< Main entry in indices.
RES_POOLCHR_FOOTNOTE_ANCHOR,                        ///< Footnote anchor.
RES_POOLCHR_ENDNOTE_ANCHOR,                         ///< Endnote anchor.
RES_POOLCHR_RUBYTEXT,                               ///< Rubytext.
RES_POOLCHR_VERT_NUM,                               ///< Vertical numbering symbols.

RES_POOLCHR_NORMAL_END,

RES_POOLCHR_HTML_BEGIN = RES_POOLCHR_BEGIN + 50,    ///< HTML-styles.
RES_POOLCHR_HTML_EMPHASIS= RES_POOLCHR_HTML_BEGIN,
RES_POOLCHR_HTML_CITIATION,
RES_POOLCHR_HTML_STRONG,
RES_POOLCHR_HTML_CODE,
RES_POOLCHR_HTML_SAMPLE,
RES_POOLCHR_HTML_KEYBOARD,
RES_POOLCHR_HTML_VARIABLE,
RES_POOLCHR_HTML_DEFINSTANCE,
RES_POOLCHR_HTML_TELETYPE,
RES_POOLCHR_HTML_END,

RES_POOLCHR_END = RES_POOLCHR_HTML_END
};


/// IDs for frame styles.
enum RES_POOL_FRMFMT_TYPE
{
RES_POOLFRM_BEGIN = POOLGRP_FRAMEFMT,

RES_POOLFRM_FRAME = RES_POOLFRM_BEGIN,              ///< Frame.
RES_POOLFRM_GRAPHIC,                                ///< Graphics.
RES_POOLFRM_OLE,                                    ///< OLE.
RES_POOLFRM_FORMEL,                                 ///< Formula.
RES_POOLFRM_MARGINAL,                               ///< Marginalia.
RES_POOLFRM_WATERSIGN,                              ///< Watermark.
RES_POOLFRM_LABEL,                                  ///< Labels.

RES_POOLFRM_END
};

/// IDs for page styles.
enum RES_POOL_PAGEFMT_TYPE
{
RES_POOLPAGE_BEGIN = POOLGRP_PAGEDESC,

RES_POOLPAGE_STANDARD = RES_POOLPAGE_BEGIN,         ///< Standard page.
RES_POOLPAGE_FIRST,                                 ///< First page.
RES_POOLPAGE_LEFT,                                  ///< Left page.
RES_POOLPAGE_RIGHT,                                 ///< Right page.
RES_POOLPAGE_JAKET,                                 ///< Envelope.
RES_POOLPAGE_REGISTER,                              ///< Index.
RES_POOLPAGE_HTML,                                  ///< HTML.
RES_POOLPAGE_FOOTNOTE,                              ///< Footnote at end of document.
RES_POOLPAGE_ENDNOTE,                               ///< Endnote page.
RES_POOLPAGE_LANDSCAPE,                             ///< Landscape page style.

RES_POOLPAGE_END
};

// IDs for list styles.
enum RES_POOL_NUMRULE_TYPE
{
RES_POOLNUMRULE_BEGIN = POOLGRP_NUMRULE,
RES_POOLNUMRULE_NUM1 = RES_POOLNUMRULE_BEGIN,       ///< NumRule Numbering 1.
RES_POOLNUMRULE_NUM2,                               ///< NumRule Numbering 2.
RES_POOLNUMRULE_NUM3,                               ///< NumRule Numbering 3.
RES_POOLNUMRULE_NUM4,                               ///< NumRule Numbering 4.
RES_POOLNUMRULE_NUM5,                               ///< NumRule Numbering 5.
RES_POOLNUMRULE_BUL1,                               ///< NumRule Bullets 1.
RES_POOLNUMRULE_BUL2,                               ///< NumRule Bullets 2.
RES_POOLNUMRULE_BUL3,                               ///< NumRule Bullets 3.
RES_POOLNUMRULE_BUL4,                               ///< NumRule Bullets 4.
RES_POOLNUMRULE_BUL5,                               ///< NumRule Bullets 5.
RES_POOLNUMRULE_END
};

// IDs for paragraph styles.
enum RES_POOL_COLLFMT_TYPE
{
// Group text.
RES_POOLCOLL_TEXT_BEGIN = COLL_TEXT_BITS,

RES_POOLCOLL_STANDARD = RES_POOLCOLL_TEXT_BEGIN,        ///< Standard.
RES_POOLCOLL_TEXT,                                      ///< Text body.
RES_POOLCOLL_TEXT_IDENT,                                ///< Text body first line indent.
RES_POOLCOLL_TEXT_NEGIDENT,                             ///< Text body hanging indent.
RES_POOLCOLL_TEXT_MOVE,                                 ///< Text body indent.
RES_POOLCOLL_GREETING,                                  ///< Complimentary close.
RES_POOLCOLL_SIGNATURE,                                 ///< Signature.
RES_POOLCOLL_CONFRONTATION,                             ///< List indent.
RES_POOLCOLL_MARGINAL,                                  ///< Marginalia.

/// Subgroup headings.
RES_POOLCOLL_HEADLINE_BASE,                             ///< Base heading.
RES_POOLCOLL_HEADLINE1,                                 ///< Heading 1.
RES_POOLCOLL_HEADLINE2,                                 ///< Heading 2.
RES_POOLCOLL_HEADLINE3,                                 ///< Heading 3.
RES_POOLCOLL_HEADLINE4,                                 ///< Heading 4.
RES_POOLCOLL_HEADLINE5,                                 ///< Heading 5.
RES_POOLCOLL_HEADLINE6,                                 ///< Heading 6.
RES_POOLCOLL_HEADLINE7,                                 ///< Heading 7.
RES_POOLCOLL_HEADLINE8,                                 ///< Heading 8.
RES_POOLCOLL_HEADLINE9,                                 ///< Heading 9.
RES_POOLCOLL_HEADLINE10,                                ///< Heading 10.

RES_POOLCOLL_TEXT_END,


/// Group lists.
RES_POOLCOLL_LISTS_BEGIN = COLL_LISTS_BITS,

RES_POOLCOLL_NUMBUL_BASE = RES_POOLCOLL_LISTS_BEGIN,    ///< Base list.

/// Subgroup numberings.
RES_POOLCOLL_NUM_LEVEL1S,                               ///< Start 1st level.
RES_POOLCOLL_NUM_LEVEL1,                                ///< 1st level.
RES_POOLCOLL_NUM_LEVEL1E,                               ///< End 1st level.
RES_POOLCOLL_NUM_NONUM1,                                ///< No numbering.
RES_POOLCOLL_NUM_LEVEL2S,                               ///< Start 2nd level.
RES_POOLCOLL_NUM_LEVEL2,                                ///< 2nd level.
RES_POOLCOLL_NUM_LEVEL2E,                               ///< End 2nd level.
RES_POOLCOLL_NUM_NONUM2,                                ///< No numbering.
RES_POOLCOLL_NUM_LEVEL3S,                               ///< Start 3rd level.
RES_POOLCOLL_NUM_LEVEL3,                                ///< 3rd level.
RES_POOLCOLL_NUM_LEVEL3E,                               ///< End 3rd level.
RES_POOLCOLL_NUM_NONUM3,                                ///< No numbering.
RES_POOLCOLL_NUM_LEVEL4S,                               ///< Start 4th level.
RES_POOLCOLL_NUM_LEVEL4,                                ///< 4th level.
RES_POOLCOLL_NUM_LEVEL4E,                               ///< End 4th level.
RES_POOLCOLL_NUM_NONUM4,                                ///< No numbering.
RES_POOLCOLL_NUM_LEVEL5S,                               ///< Start 5th level.
RES_POOLCOLL_NUM_LEVEL5,                                ///< 5th level.
RES_POOLCOLL_NUM_LEVEL5E,                               ///< End 5th level.
RES_POOLCOLL_NUM_NONUM5,                                ///< No numbering.


///Subgroup bullets.
RES_POOLCOLL_BUL_LEVEL1S,                               ///< Start 1st level.
RES_POOLCOLL_BUL_LEVEL1,                                ///< 1st level.
RES_POOLCOLL_BUL_LEVEL1E,                               ///< End 1st level
RES_POOLCOLL_BUL_NONUM1,                                ///< No numbering.
RES_POOLCOLL_BUL_LEVEL2S,                               ///< Start 2nd level.
RES_POOLCOLL_BUL_LEVEL2,                                ///< 2nd level.
RES_POOLCOLL_BUL_LEVEL2E,                               ///< End 2nd level.
RES_POOLCOLL_BUL_NONUM2,                                ///< No numbering.
RES_POOLCOLL_BUL_LEVEL3S,                               ///< Start 3rd level.
RES_POOLCOLL_BUL_LEVEL3,                                ///< 3rd Level.
RES_POOLCOLL_BUL_LEVEL3E,                               ///< End 3rd level.
RES_POOLCOLL_BUL_NONUM3,                                ///< No numbering.
RES_POOLCOLL_BUL_LEVEL4S,                               ///< Start 4th level.
RES_POOLCOLL_BUL_LEVEL4,                                ///< 4th level.
RES_POOLCOLL_BUL_LEVEL4E,                               ///< End 4th level.
RES_POOLCOLL_BUL_NONUM4,                                ///< No numbering.
RES_POOLCOLL_BUL_LEVEL5S,                               ///< Start 5th level.
RES_POOLCOLL_BUL_LEVEL5,                                ///< 5th level.
RES_POOLCOLL_BUL_LEVEL5E,                               ///< End 5th Level.
RES_POOLCOLL_BUL_NONUM5,                                ///< No numbering.

RES_POOLCOLL_LISTS_END,


/// Special ranges.
RES_POOLCOLL_EXTRA_BEGIN = COLL_EXTRA_BITS,

/// Subgroup header.
RES_POOLCOLL_HEADER = RES_POOLCOLL_EXTRA_BEGIN,         ///< Header Left&Right.
RES_POOLCOLL_HEADERL,                                   ///< Header Left.
RES_POOLCOLL_HEADERR,                                   ///< Header Right.

/// Subgroup footer.
RES_POOLCOLL_FOOTER,                                    ///< Footer Left&Right.
RES_POOLCOLL_FOOTERL,                                   ///< Footer Left.
RES_POOLCOLL_FOOTERR,                                   ///< Footer Right.

/// Subgroup table.
RES_POOLCOLL_TABLE,                                     ///< Table of Contents.
RES_POOLCOLL_TABLE_HDLN,                                ///< Table of Contents - heading.


/// Subgroup labels.
RES_POOLCOLL_LABEL,                                     ///< Base labels.
RES_POOLCOLL_LABEL_ABB,                                 ///< Label illustration.
RES_POOLCOLL_LABEL_TABLE,                               ///< Label table.
RES_POOLCOLL_LABEL_FRAME,                               ///< Label frame.

/// Other stuff.
RES_POOLCOLL_FRAME,                                     ///< Frames.
RES_POOLCOLL_FOOTNOTE,                                  ///< Footnotes.
RES_POOLCOLL_JAKETADRESS,                               ///< Addressee.
RES_POOLCOLL_SENDADRESS,                                ///< Sender.
RES_POOLCOLL_ENDNOTE,                                   ///< Endnotes.

RES_POOLCOLL_LABEL_DRAWING,                             ///< Label drawing objects.
RES_POOLCOLL_EXTRA_END,


/// Group indices.
RES_POOLCOLL_REGISTER_BEGIN = COLL_REGISTER_BITS,

RES_POOLCOLL_REGISTER_BASE = RES_POOLCOLL_REGISTER_BEGIN,   ///< Base index.

/// Subgroup index tables.
RES_POOLCOLL_TOX_IDXH,                                  ///< Header.
RES_POOLCOLL_TOX_IDX1,                                  ///< 1st level.
RES_POOLCOLL_TOX_IDX2,                                  ///< 2nd level.
RES_POOLCOLL_TOX_IDX3,                                  ///< 3rd level.
RES_POOLCOLL_TOX_IDXBREAK,                              ///< Separator.

/// Subgroup table of contents.
RES_POOLCOLL_TOX_CNTNTH,                                ///< Header.
RES_POOLCOLL_TOX_CNTNT1,                                ///< Content 1st level.
RES_POOLCOLL_TOX_CNTNT2,                                ///< Content 2nd level.
RES_POOLCOLL_TOX_CNTNT3,                                ///< Content 3rd level.
RES_POOLCOLL_TOX_CNTNT4,                                ///< Content 4th level.
RES_POOLCOLL_TOX_CNTNT5,                                ///< Content 5th level.


/// Subgroup user indices.
RES_POOLCOLL_TOX_USERH,                                 ///< Header.
RES_POOLCOLL_TOX_USER1,                                 ///< 1st level.
RES_POOLCOLL_TOX_USER2,                                 ///< 2nd level.
RES_POOLCOLL_TOX_USER3,                                 ///< 3rd level.
RES_POOLCOLL_TOX_USER4,                                 ///< 4th level.
RES_POOLCOLL_TOX_USER5,                                 ///< 5th level.


RES_POOLCOLL_TOX_CNTNT6,                                ///< Content  6th level.
RES_POOLCOLL_TOX_CNTNT7,                                ///< Content  7th level.
RES_POOLCOLL_TOX_CNTNT8,                                ///< Content  8th level.
RES_POOLCOLL_TOX_CNTNT9,                                ///< Content  9th level.
RES_POOLCOLL_TOX_CNTNT10,                               ///< Content 10th level.

/// illustrations index.
RES_POOLCOLL_TOX_ILLUSH,                                ///< Illustrations header.
RES_POOLCOLL_TOX_ILLUS1,                                ///< Illustrations all levels.

///  object index.
RES_POOLCOLL_TOX_OBJECTH,                               ///< Objects header.
RES_POOLCOLL_TOX_OBJECT1,                               ///< Objects all levels.

///  tables index.
RES_POOLCOLL_TOX_TABLESH,                               ///< Tables header.
RES_POOLCOLL_TOX_TABLES1,                               ///< Tables all levels.

///  index of authorities.
RES_POOLCOLL_TOX_AUTHORITIESH,                          ///< Authorities header.
RES_POOLCOLL_TOX_AUTHORITIES1,                          ///< Authorities all levels.

/// user index 6..10.
RES_POOLCOLL_TOX_USER6,                                 ///< 6th  level.
RES_POOLCOLL_TOX_USER7,                                 ///< 7th  level.
RES_POOLCOLL_TOX_USER8,                                 ///< 8th  level.
RES_POOLCOLL_TOX_USER9,                                 ///< 9th  level.
RES_POOLCOLL_TOX_USER10,                                ///< 10th level.

RES_POOLCOLL_REGISTER_END,


/// Group chapter / document.
RES_POOLCOLL_DOC_BEGIN = COLL_DOC_BITS,

RES_POOLCOLL_DOC_TITEL = RES_POOLCOLL_DOC_BEGIN,        ///< Doc. titel.
RES_POOLCOLL_DOC_SUBTITEL,                              ///< Doc. subtitel.

RES_POOLCOLL_DOC_END,

/// Group HTML-styles.
RES_POOLCOLL_HTML_BEGIN = COLL_HTML_BITS,

RES_POOLCOLL_HTML_BLOCKQUOTE = RES_POOLCOLL_HTML_BEGIN,
RES_POOLCOLL_HTML_PRE,
RES_POOLCOLL_HTML_HR,
RES_POOLCOLL_HTML_DD,
RES_POOLCOLL_HTML_DT,

RES_POOLCOLL_HTML_END

/// End of text styles collection.
};


/** Query defined parent of a POOL-ID
   Returns  0 if standard
            USHRT_MAX if no parent
            the parent in all other cases. */
sal_uInt16 GetPoolParent( sal_uInt16 nId );

SvxFrameDirection GetDefaultFrameDirection(sal_uLong nLanguage);

inline sal_Bool IsConditionalByPoolId(sal_uInt16 nId)
    {
        return RES_POOLCOLL_TEXT == nId;
    }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
