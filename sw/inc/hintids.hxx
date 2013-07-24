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
#ifndef _HINTIDS_HXX
#define _HINTIDS_HXX

#include <tools/solar.h>
#include <sal/types.h>          // for sal_Unicode
#include "swdllapi.h"

// For SwTxtHints without end index the following char is added:

#define CH_TXTATR_BREAKWORD     ((sal_Unicode)0x01)
#define CH_TXTATR_INWORD        ((sal_Unicode)0xFFF9)
#define CH_TXTATR_TAB           ((sal_Unicode)'\t')
#define CH_TXTATR_NEWLINE       ((sal_Unicode)'\n')
#define CH_TXT_ATR_FIELDSTART ((sal_Unicode)0x04)
#define CH_TXT_ATR_FIELDEND ((sal_Unicode)0x05)
#define CH_TXT_ATR_SUBST_FIELDSTART ("[")
#define CH_TXT_ATR_SUBST_FIELDEND ("]")
#define CH_TXT_ATR_FORMELEMENT ((sal_Unicode)0x06)

/*
 * Enums for the hints
 */

#define HINT_BEGIN  1

#define POOLATTR_BEGIN      HINT_BEGIN
#define POOLATTR_END        RES_UNKNOWNATR_END

// Ranges for the IDs of the format-attributes.
// Which-values for character-format attributes.
enum RES_CHRATR
{
RES_CHRATR_BEGIN = HINT_BEGIN,
    RES_CHRATR_CASEMAP = RES_CHRATR_BEGIN,  //  1
    RES_CHRATR_CHARSETCOLOR,                //  2
    RES_CHRATR_COLOR,                       //  3
    RES_CHRATR_CONTOUR,                     //  4
    RES_CHRATR_CROSSEDOUT,                  //  5
    RES_CHRATR_ESCAPEMENT,                  //  6
    RES_CHRATR_FONT,                        //  7
    RES_CHRATR_FONTSIZE,                    //  8
    RES_CHRATR_KERNING,                     //  9
    RES_CHRATR_LANGUAGE,                    // 10
    RES_CHRATR_POSTURE,                     // 11
    RES_CHRATR_PROPORTIONALFONTSIZE,        // 12
    RES_CHRATR_SHADOWED,                    // 13
    RES_CHRATR_UNDERLINE,                   // 14
    RES_CHRATR_WEIGHT,                      // 15
    RES_CHRATR_WORDLINEMODE,                // 16
    RES_CHRATR_AUTOKERN,                    // 17
    RES_CHRATR_BLINK,                       // 18
    RES_CHRATR_NOHYPHEN,                    // 19
    RES_CHRATR_NOLINEBREAK,                 // 20
    RES_CHRATR_BACKGROUND,                  // 21
    RES_CHRATR_CJK_FONT,                    // 22
    RES_CHRATR_CJK_FONTSIZE,                // 23
    RES_CHRATR_CJK_LANGUAGE,                // 24
    RES_CHRATR_CJK_POSTURE,                 // 25
    RES_CHRATR_CJK_WEIGHT,                  // 26
    RES_CHRATR_CTL_FONT,                    // 27
    RES_CHRATR_CTL_FONTSIZE,                // 28
    RES_CHRATR_CTL_LANGUAGE,                // 29
    RES_CHRATR_CTL_POSTURE,                 // 30
    RES_CHRATR_CTL_WEIGHT,                  // 31
    RES_CHRATR_ROTATE,                      // 32
    RES_CHRATR_EMPHASIS_MARK,               // 33
    RES_CHRATR_TWO_LINES,                   // 34
    RES_CHRATR_SCALEW,                      // 35
    RES_CHRATR_RELIEF,                      // 36
    RES_CHRATR_HIDDEN,                      // 37
    RES_CHRATR_OVERLINE,                    // 38
    RES_CHRATR_RSID,						// 39
    RES_CHRATR_DUMMY1,						// 40
RES_CHRATR_END
};

// this Attribute used only in a TextNodes SwpAttr-Array
enum RES_TXTATR
{
RES_TXTATR_BEGIN = RES_CHRATR_END,

/** text attributes with start and end.
   #i105453#:
   Hints (SwTxtAttr) with the same start and end position are sorted by
   WhichId, i.e., the TXTATR constants defined here.
   The text formatting (SwAttrIter) poses some requirements on TXTATR order:
   - AUTOFMT must precede CHARFMT, so that auto style can overwrite char style.
   - INETFMT must precede CHARFMT, so that link style can overwrite char style.
     (this is actually surprising: CHARFMT hints are not split at INETFMT
      hints on insertion, but on exporting to ODF. if CHARFMT would precede
      INETFMT, then exporting and importing will effectively change precedence)

   Nesting hints (SwTxtAttrNesting) also have requirements on TXTATR order,
   to ensure proper nesting (because CJK_RUBY and INETFMT have no CH_TXTATR):
   - INETFMT should precede CJK_RUBY (for UNO API it does not matter...)
   - META and METAFIELD must precede CJK_RUBY and INETFMT
 */
RES_TXTATR_WITHEND_BEGIN = RES_TXTATR_BEGIN ,
    RES_TXTATR_REFMARK = RES_TXTATR_WITHEND_BEGIN,  // 41
    RES_TXTATR_TOXMARK,                             // 42
    RES_TXTATR_META,                                // 43
    RES_TXTATR_METAFIELD,                           // 44
    RES_TXTATR_AUTOFMT,                             // 45
    RES_TXTATR_INETFMT,                             // 46
    RES_TXTATR_CHARFMT,                             // 47
    RES_TXTATR_CJK_RUBY,                            // 48
    RES_TXTATR_UNKNOWN_CONTAINER,                   // 49
    RES_TXTATR_DUMMY5,                              // 50
RES_TXTATR_WITHEND_END,

// all TextAttributes without an end
RES_TXTATR_NOEND_BEGIN = RES_TXTATR_WITHEND_END,
    RES_TXTATR_FIELD = RES_TXTATR_NOEND_BEGIN,      // 51
    RES_TXTATR_FLYCNT,                              // 52
    RES_TXTATR_FTN,                                 // 53
    RES_TXTATR_DUMMY4,                              // 54
    RES_TXTATR_DUMMY3,                              // 55
    RES_TXTATR_DUMMY1,                              // 56
    RES_TXTATR_DUMMY2,                              // 57
RES_TXTATR_NOEND_END,
RES_TXTATR_END = RES_TXTATR_NOEND_END
};

enum RES_PARATR
{
RES_PARATR_BEGIN = RES_TXTATR_END,
    RES_PARATR_LINESPACING = RES_PARATR_BEGIN,      // 58
    RES_PARATR_ADJUST,                              // 59
    RES_PARATR_SPLIT,                               // 60
    RES_PARATR_ORPHANS,                             // 61
    RES_PARATR_WIDOWS,                              // 62
    RES_PARATR_TABSTOP,                             // 63
    RES_PARATR_HYPHENZONE,                          // 64
    RES_PARATR_DROP,                                // 65
    RES_PARATR_REGISTER,                            // 66
    RES_PARATR_NUMRULE,                             // 67
    RES_PARATR_SCRIPTSPACE,                         // 68
    RES_PARATR_HANGINGPUNCTUATION,                  // 69
    RES_PARATR_FORBIDDEN_RULES,                     // 70
    RES_PARATR_VERTALIGN,                           // 71
    RES_PARATR_SNAPTOGRID,                          // 72
    RES_PARATR_CONNECT_BORDER,                      // 73
    RES_PARATR_OUTLINELEVEL,                        // 74
    RES_PARATR_RSID,								// 75
    RES_PARATR_GRABBAG,
RES_PARATR_END
};

// list attributes for paragraphs.
// intentionally these list attributes are not contained in paragraph styles
enum RES_PARATR_LIST
{
RES_PARATR_LIST_BEGIN = RES_PARATR_END,
    RES_PARATR_LIST_ID = RES_PARATR_LIST_BEGIN,     // 76
    RES_PARATR_LIST_LEVEL,                          // 77
    RES_PARATR_LIST_ISRESTART,                      // 78
    RES_PARATR_LIST_RESTARTVALUE,                   // 79
    RES_PARATR_LIST_ISCOUNTED,                      // 80
RES_PARATR_LIST_END
};

enum RES_FRMATR
{
RES_FRMATR_BEGIN = RES_PARATR_LIST_END,
    RES_FILL_ORDER = RES_FRMATR_BEGIN,              // 81
    RES_FRM_SIZE,                                   // 82
    RES_PAPER_BIN,                                  // 83
    RES_LR_SPACE,                                   // 84
    RES_UL_SPACE,                                   // 85
    RES_PAGEDESC,                                   // 86
    RES_BREAK,                                      // 87
    RES_CNTNT,                                      // 88
    RES_HEADER,                                     // 89
    RES_FOOTER,                                     // 90
    RES_PRINT,                                      // 91
    RES_OPAQUE,                                     // 92
    RES_PROTECT,                                    // 93
    RES_SURROUND,                                   // 94
    RES_VERT_ORIENT,                                // 95
    RES_HORI_ORIENT,                                // 96
    RES_ANCHOR,                                     // 97
    RES_BACKGROUND,                                 // 98
    RES_BOX,                                        // 99
    RES_SHADOW,                                     // 100
    RES_FRMMACRO,                                   // 101
    RES_COL,                                        // 102
    RES_KEEP,                                       // 103
    RES_URL,                                        // 104
    RES_EDIT_IN_READONLY,                           // 105
    RES_LAYOUT_SPLIT,                               // 106
    RES_CHAIN,                                      // 107
    RES_TEXTGRID,                                   // 108
    RES_LINENUMBER  ,                               // 109
    RES_FTN_AT_TXTEND,                              // 110
    RES_END_AT_TXTEND,                              // 111
    RES_COLUMNBALANCE,                              // 112
    RES_FRAMEDIR,                                   // 113
    RES_HEADER_FOOTER_EAT_SPACING,                  // 114
    RES_ROW_SPLIT,                                  // 115
    RES_FOLLOW_TEXT_FLOW,                           // 116
    RES_COLLAPSING_BORDERS,                         // 117
    RES_WRAP_INFLUENCE_ON_OBJPOS,                   // 118
    RES_AUTO_STYLE,                                 // 119
    RES_FRMATR_STYLE_NAME,                          // 120
    RES_FRMATR_CONDITIONAL_STYLE_NAME,              // 121
    RES_FILL_STYLE,                                 // 122
    RES_FILL_GRADIENT,                              // 123
RES_FRMATR_END
};

enum RES_GRFATR
{
RES_GRFATR_BEGIN = RES_FRMATR_END,
    RES_GRFATR_MIRRORGRF = RES_GRFATR_BEGIN,        // 124
    RES_GRFATR_CROPGRF,                             // 125

    RES_GRFATR_ROTATION,                            // 126
    RES_GRFATR_LUMINANCE,                           // 127
    RES_GRFATR_CONTRAST,                            // 128
    RES_GRFATR_CHANNELR,                            // 129
    RES_GRFATR_CHANNELG,                            // 130
    RES_GRFATR_CHANNELB,                            // 131
    RES_GRFATR_GAMMA,                               // 132
    RES_GRFATR_INVERT,                              // 133
    RES_GRFATR_TRANSPARENCY,                        // 134
    RES_GRFATR_DRAWMODE,                            // 135

    RES_GRFATR_DUMMY1,                              // 136
    RES_GRFATR_DUMMY2,                              // 137
    RES_GRFATR_DUMMY3,                              // 138
    RES_GRFATR_DUMMY4,                              // 139
    RES_GRFATR_DUMMY5,                              // 140
RES_GRFATR_END
};

enum RES_BOXATR
{
RES_BOXATR_BEGIN = RES_GRFATR_END,
    RES_BOXATR_FORMAT = RES_BOXATR_BEGIN,           // 141
    RES_BOXATR_FORMULA,                             // 142
    RES_BOXATR_VALUE,                               // 143
RES_BOXATR_END
};

enum RES_UNKNOWNATR
{
RES_UNKNOWNATR_BEGIN = RES_BOXATR_END,
    RES_UNKNOWNATR_CONTAINER = RES_UNKNOWNATR_BEGIN,// 142
RES_UNKNOWNATR_END
};



// Format IDs
enum RES_FMT
{
RES_FMT_BEGIN = RES_UNKNOWNATR_END,
    RES_CHRFMT = RES_FMT_BEGIN,
    RES_FRMFMT,
    RES_FLYFRMFMT,
    RES_TXTFMTCOLL,
    RES_GRFFMTCOLL,
    RES_DRAWFRMFMT,
    RES_CONDTXTFMTCOLL,
RES_FMT_END
};

// ID's for Messages in the Formats
enum RES_MSG
{
RES_MSG_BEGIN = RES_FMT_END,
    RES_OBJECTDYING = RES_MSG_BEGIN,
    RES_FMT_CHG,
    RES_ATTRSET_CHG,
    RES_FRM_SIZECHG,
    RES_TXTATR_FLDCHG,
    RES_TXTATR_EMPTYCHG,
    RES_INS_CHR,
    RES_INS_TXT,
    RES_DEL_CHR,
    RES_DEL_TXT,
    RES_UPDATE_ATTR,
    RES_PAGEDESC_FTNINFO,
    RES_REFMARKFLD_UPDATE,
    RES_DOCPOS_UPDATE,
    RES_TABLEFML_UPDATE,
    RES_UPDATEDDETBL,
    RES_TBLHEADLINECHG,
    RES_AUTOFMT_DOCNODE,
    RES_REPAINT,
    RES_DOC_DTOR,
    RES_SECTION_HIDDEN,
    RES_SECTION_NOT_HIDDEN,
    RES_GRAPHIC_ARRIVED,
    RES_GRAPHIC_PIECE_ARRIVED,
    RES_HIDDENPARA_PRINT,
    RES_CONDCOLL_CONDCHG,
    RES_VIRTPAGENUM_INFO,
    RES_GETLOWERNUMLEVEL,
    RES_RESET_FMTWRITTEN,
    RES_REMOVE_UNO_OBJECT,
    RES_GRF_REREAD_AND_INCACHE,
    RES_SECTION_RESETHIDDENFLAG,
    RES_FINDNEARESTNODE,
    RES_CONTENT_VISIBLE,
    RES_FOOTNOTE_DELETED,
    RES_REFMARK_DELETED,
    RES_TOXMARK_DELETED,
    RES_GRAPHIC_SWAPIN,
    RES_FIELD_DELETED,
    RES_NAME_CHANGED,
    RES_TITLE_CHANGED,
    RES_DESCRIPTION_CHANGED,
    RES_UNOCURSOR_LEAVES_SECTION,
    RES_LINKED_GRAPHIC_STREAM_ARRIVED,
RES_MSG_END
};

// An ID for the RTF-reader. The stylesheets are treated like attributes,
// i.e. there is a StyleSheet-attribute. To avoid collision with other
// Which()-values, the value is listed here. (The help system too defines
// new attributes!)
enum RES_FLTRATTR
{
RES_FLTRATTR_BEGIN = RES_MSG_END,
    RES_FLTR_STYLESHEET = RES_FLTRATTR_BEGIN,
    RES_FLTR_BOOKMARK,
    RES_FLTR_ANCHOR,
    RES_FLTR_BORDER,
    RES_FLTR_NUMRULE,
    RES_FLTR_NUMRULE_NUM,
    RES_FLTR_SDR_ANCHOR,
    RES_FLTR_TOX,
    RES_FLTR_SECTION,
    RES_FLTR_REDLINE,
    RES_FLTR_SCRIPTTYPE,
RES_FLTRATTR_END
};

#define RES_TBX_DUMMY RES_FLTRATTR_END + 1

#define HINT_END RES_TBX_DUMMY

// Error recognition!!
#define INVALID_HINT HINT_END
#define RES_WHICHHINT_END HINT_END


inline bool isATR(const sal_uInt16 nWhich)
{
    return (RES_CHRATR_BEGIN <= nWhich) && (RES_UNKNOWNATR_END > nWhich);
}
inline bool isCHRATR(const sal_uInt16 nWhich)
{
    return (RES_CHRATR_BEGIN <= nWhich) && (RES_CHRATR_END > nWhich);
}
inline bool isTXTATR_WITHEND(const sal_uInt16 nWhich)
{
    return (RES_TXTATR_WITHEND_BEGIN <= nWhich)
        && (RES_TXTATR_WITHEND_END > nWhich);
}
inline bool isTXTATR_NOEND(const sal_uInt16 nWhich)
{
    return (RES_TXTATR_NOEND_BEGIN <= nWhich)
        && (RES_TXTATR_NOEND_END > nWhich);
}
inline bool isTXTATR(const sal_uInt16 nWhich)
{
    return (RES_TXTATR_BEGIN <= nWhich) && (RES_TXTATR_END > nWhich);
}
inline bool isPARATR(const sal_uInt16 nWhich)
{
    return (RES_PARATR_BEGIN <= nWhich) && (RES_PARATR_END > nWhich);
}
inline bool isPARATR_LIST(const sal_uInt16 nWhich)
{
    return (RES_PARATR_LIST_BEGIN <= nWhich) && (RES_PARATR_LIST_END > nWhich); }
inline bool isFRMATR(const sal_uInt16 nWhich)
{
    return (RES_FRMATR_BEGIN <= nWhich) && (RES_FRMATR_END > nWhich);
}
inline bool isGRFATR(const sal_uInt16 nWhich)
{
    return (RES_GRFATR_BEGIN <= nWhich) && (RES_GRFATR_END > nWhich);
}
inline bool isBOXATR(const sal_uInt16 nWhich)
{
    return (RES_BOXATR_BEGIN <= nWhich) && (RES_BOXATR_END > nWhich);
}
inline bool isUNKNOWNATR(const sal_uInt16 nWhich)
{
    return (RES_UNKNOWNATR_BEGIN <= nWhich) && (RES_UNKNOWNATR_END > nWhich);
}


// Take the respective default attribute from the statistical default
// attributes table over the Which-value.
// If none exists, return a 0 pointer!!!
// This function is implemented in Init.cxx. It is declared here as external
// in order to allow the formats to access it.
// Inline in PRODUCT.
class SfxPoolItem;
struct SfxItemInfo;
typedef SfxPoolItem* SwDfltAttrTab[ POOLATTR_END - POOLATTR_BEGIN  ];

extern SwDfltAttrTab aAttrTab;
extern SfxItemInfo   aSlotTab[];

SW_DLLPUBLIC const SfxPoolItem* GetDfltAttr( sal_uInt16 nWhich );

SW_DLLPUBLIC sal_uInt16 GetWhichOfScript( sal_uInt16 nWhich, sal_uInt16 nScript );

// return for the given TextAttribute without an end the correct character.
// This function returns
//      CH_TXTATR_BREAKWORD for Textattribute which breaks a word (default)
//      CH_TXTATR_INWORD    for Textattribute which dont breaks a word
class SwTxtAttr;
sal_Unicode GetCharOfTxtAttr( const SwTxtAttr& rAttr );

// all Sets defined in init.cxx

// AttrSet-Range for the 3 Break-Attribute
extern sal_uInt16 aBreakSetRange[];
// AttrSet-Range for TxtFmtColl
extern sal_uInt16 aTxtFmtCollSetRange[];
// AttrSet-Range for GrfFmtColl
extern sal_uInt16 aGrfFmtCollSetRange[];
// AttrSet-Range for TextNode
SW_DLLPUBLIC extern sal_uInt16 aTxtNodeSetRange[];
// AttrSet-Range for NoTxtNode
extern sal_uInt16 aNoTxtNodeSetRange[];
// AttrSet-Range for SwTable
extern sal_uInt16 aTableSetRange[];
// AttrSet-Range for SwTableLine
extern sal_uInt16 aTableLineSetRange[];
// AttrSet-Range for SwTableBox
extern sal_uInt16 aTableBoxSetRange[];
// AttrSet-Range for SwFrmFmt
SW_DLLPUBLIC extern sal_uInt16 aFrmFmtSetRange[];
// AttrSet-Range for SwCharFmt
extern sal_uInt16 aCharFmtSetRange[];
// AttrSet-Range for the autostyles
extern sal_uInt16 aCharAutoFmtSetRange[];
// AttrSet-Range for SwPageDescFmt
extern sal_uInt16 aPgFrmFmtSetRange[];

// check if ID is InRange of AttrSet-Ids
bool IsInRange( const sal_uInt16* pRange, const sal_uInt16 nId );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
