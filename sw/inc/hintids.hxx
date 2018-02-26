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
#ifndef INCLUDED_SW_INC_HINTIDS_HXX
#define INCLUDED_SW_INC_HINTIDS_HXX

#include <tools/solar.h>
#include <sal/types.h>
#include <svx/xdef.hxx>
#include "swdllapi.h"
#include <svl/typedwhich.hxx>
#include <vector>

class SfxStringItem;

// For SwTextHints without end index the following char is added:

#define CH_TXTATR_BREAKWORD     u'\x0001'
#define CH_TXTATR_INWORD        u'\xFFF9'
#define CH_TXTATR_TAB           u'\t'
#define CH_TXTATR_NEWLINE       u'\n'
#define CH_TXT_ATR_INPUTFIELDSTART u'\x0004'
#define CH_TXT_ATR_INPUTFIELDEND u'\x0005'

#define CH_TXT_ATR_FORMELEMENT u'\x0006'

#define CH_TXT_ATR_FIELDSTART u'\x0007'
#define CH_TXT_ATR_FIELDEND u'\x0008'
#define CH_TXT_ATR_SUBST_FIELDSTART ("[")
#define CH_TXT_ATR_SUBST_FIELDEND ("]")

/*
 * Enums for the hints
 */

#define HINT_BEGIN  1

#define POOLATTR_BEGIN      HINT_BEGIN
#define POOLATTR_END        RES_UNKNOWNATR_END

// Ranges for the IDs of the format-attributes.
// Which-values for character-format attributes.
#define RES_CHRATR_BEGIN  HINT_BEGIN
#define     RES_CHRATR_CASEMAP                     TypedWhichId<SvxCaseMapItem>(RES_CHRATR_BEGIN)  //  1
#define     RES_CHRATR_CHARSETCOLOR                TypedWhichId<SvxCharSetColorItem>(2)
#define     RES_CHRATR_COLOR                       TypedWhichId<SvxColorItem>(3)
#define     RES_CHRATR_CONTOUR                     TypedWhichId<SvxContourItem>(4)
#define     RES_CHRATR_CROSSEDOUT                  TypedWhichId<SvxCrossedOutItem>(5)
#define     RES_CHRATR_ESCAPEMENT                  TypedWhichId<SvxEscapementItem>(6)
#define     RES_CHRATR_FONT                        TypedWhichId<SvxFontItem>(7)
#define     RES_CHRATR_FONTSIZE                    TypedWhichId<SvxFontHeightItem>(8)
#define     RES_CHRATR_KERNING                     TypedWhichId<SvxKerningItem>(9)
#define     RES_CHRATR_LANGUAGE                    TypedWhichId<SvxLanguageItem>(10)
#define     RES_CHRATR_POSTURE                     TypedWhichId<SvxPostureItem>(11)
#define     RES_CHRATR_UNUSED1                     TypedWhichId<SfxVoidItem>(12)
#define     RES_CHRATR_SHADOWED                    TypedWhichId<SvxShadowedItem>(13)
#define     RES_CHRATR_UNDERLINE                   TypedWhichId<SvxUnderlineItem>(14)
#define     RES_CHRATR_WEIGHT                      TypedWhichId<SvxWeightItem>(15)
#define     RES_CHRATR_WORDLINEMODE                TypedWhichId<SvxWordLineModeItem>(16)
#define     RES_CHRATR_AUTOKERN                    TypedWhichId<SvxAutoKernItem>(17)
#define     RES_CHRATR_BLINK                       TypedWhichId<SvxBlinkItem>(18)
#define     RES_CHRATR_NOHYPHEN                    TypedWhichId<SvxNoHyphenItem>(19)
#define     RES_CHRATR_UNUSED2                     TypedWhichId<SfxVoidItem>(20)
#define     RES_CHRATR_BACKGROUND                  TypedWhichId<SvxBrushItem>(21)
#define     RES_CHRATR_CJK_FONT                    TypedWhichId<SvxFontItem>(22)
#define     RES_CHRATR_CJK_FONTSIZE                TypedWhichId<SvxFontHeightItem>(23)
#define     RES_CHRATR_CJK_LANGUAGE                TypedWhichId<SvxLanguageItem>(24)
#define     RES_CHRATR_CJK_POSTURE                 TypedWhichId<SvxPostureItem>(25)
#define     RES_CHRATR_CJK_WEIGHT                  TypedWhichId<SvxWeightItem>(26)
#define     RES_CHRATR_CTL_FONT                    TypedWhichId<SvxFontItem>(27)
#define     RES_CHRATR_CTL_FONTSIZE                TypedWhichId<SvxFontHeightItem>(28)
#define     RES_CHRATR_CTL_LANGUAGE                TypedWhichId<SvxLanguageItem>(29)
#define     RES_CHRATR_CTL_POSTURE                 TypedWhichId<SvxPostureItem>(30)
#define     RES_CHRATR_CTL_WEIGHT                  TypedWhichId<SvxWeightItem>(31)
#define     RES_CHRATR_ROTATE                      TypedWhichId<SvxCharRotateItem>(32)
#define     RES_CHRATR_EMPHASIS_MARK               TypedWhichId<SvxEmphasisMarkItem>(33)
#define     RES_CHRATR_TWO_LINES                   TypedWhichId<SvxTwoLinesItem>(34)
#define     RES_CHRATR_SCALEW                      TypedWhichId<SvxCharScaleWidthItem>(35)
#define     RES_CHRATR_RELIEF                      TypedWhichId<SvxCharReliefItem>(36)
#define     RES_CHRATR_HIDDEN                      TypedWhichId<SvxCharHiddenItem>(37)
#define     RES_CHRATR_OVERLINE                    TypedWhichId<SvxOverlineItem>(38)
#define     RES_CHRATR_RSID                        TypedWhichId<SvxRsidItem>(39)
#define     RES_CHRATR_BOX                         TypedWhichId<SvxBoxItem>(40)
#define     RES_CHRATR_SHADOW                      TypedWhichId<SvxShadowItem>(41)
#define     RES_CHRATR_HIGHLIGHT                   TypedWhichId<SvxBrushItem>(42)
#define     RES_CHRATR_GRABBAG                     TypedWhichId<SfxGrabBagItem>(43)
#define     RES_CHRATR_BIDIRTL                     TypedWhichId<SfxInt16Item>(44)
#define     RES_CHRATR_IDCTHINT                    TypedWhichId<SfxInt16Item>(45)
#define RES_CHRATR_END  (46)

// this Attribute used only in a TextNodes SwpAttr-Array
#define RES_TXTATR_BEGIN RES_CHRATR_END

/** text attributes with start and end.
   #i105453#:
   Hints (SwTextAttr) with the same start and end position are sorted by
   WhichId, i.e., the TXTATR constants defined here.
   The text formatting (SwAttrIter) poses some requirements on TXTATR order:
   - AUTOFMT must precede CHARFMT, so that auto style can overwrite char style.
   - INETFMT must precede CHARFMT, so that link style can overwrite char style.
     (this is actually surprising: CHARFMT hints are not split at INETFMT
      hints on insertion, but on exporting to ODF. if CHARFMT would precede
      INETFMT, then exporting and importing will effectively change precedence)

   Nesting hints (SwTextAttrNesting) also have requirements on TXTATR order,
   to ensure proper nesting (because CJK_RUBY and INETFMT have no CH_TXTATR):
   - INETFMT should precede CJK_RUBY (for UNO API it does not matter...)
   - META and METAFIELD must precede CJK_RUBY and INETFMT
 */
#define RES_TXTATR_WITHEND_BEGIN RES_TXTATR_BEGIN
#define     RES_TXTATR_REFMARK            TypedWhichId<SwFormatRefMark>(RES_TXTATR_WITHEND_BEGIN) // 46
#define     RES_TXTATR_TOXMARK            TypedWhichId<SwTOXMark>(47)
#define     RES_TXTATR_META               TypedWhichId<SwFormatMeta>(48)
#define     RES_TXTATR_METAFIELD          TypedWhichId<SwFormatMeta>(49)
#define     RES_TXTATR_AUTOFMT            TypedWhichId<SwFormatAutoFormat>(50)
#define     RES_TXTATR_INETFMT            TypedWhichId<SwFormatINetFormat>(51)
#define     RES_TXTATR_CHARFMT            TypedWhichId<SwFormatCharFormat>(52)
#define     RES_TXTATR_CJK_RUBY           TypedWhichId<SwFormatRuby>(53)
#define     RES_TXTATR_UNKNOWN_CONTAINER  TypedWhichId<SvXMLAttrContainerItem>(54)
#define     RES_TXTATR_INPUTFIELD         TypedWhichId<SwFormatField>(55)
#define RES_TXTATR_WITHEND_END 56

// all TextAttributes without an end
#define RES_TXTATR_NOEND_BEGIN RES_TXTATR_WITHEND_END
#define     RES_TXTATR_FIELD              TypedWhichId<SwFormatField>(RES_TXTATR_NOEND_BEGIN) // 56
#define     RES_TXTATR_FLYCNT             TypedWhichId<SwFormatFlyCnt>(57)
#define     RES_TXTATR_FTN                TypedWhichId<SwFormatFootnote>(58)
#define     RES_TXTATR_ANNOTATION         TypedWhichId<SwFormatField>(59)
#define     RES_TXTATR_DUMMY3             TypedWhichId<SfxBoolItem>(60)
#define     RES_TXTATR_DUMMY1             TypedWhichId<SfxBoolItem>(61)
#define     RES_TXTATR_DUMMY2             TypedWhichId<SfxBoolItem>(62)
#define RES_TXTATR_NOEND_END 63
#define RES_TXTATR_END RES_TXTATR_NOEND_END

#define RES_PARATR_BEGIN RES_TXTATR_END
#define    RES_PARATR_LINESPACING                  TypedWhichId<SvxLineSpacingItem>(RES_PARATR_BEGIN) // 63
#define    RES_PARATR_ADJUST                       TypedWhichId<SvxAdjustItem>(64)
#define    RES_PARATR_SPLIT                        TypedWhichId<SvxFormatSplitItem>(65)
#define    RES_PARATR_ORPHANS                      TypedWhichId<SvxOrphansItem>(66)
#define    RES_PARATR_WIDOWS                       TypedWhichId<SvxWidowsItem>(67)
#define    RES_PARATR_TABSTOP                      TypedWhichId<SvxTabStopItem>(68)
#define    RES_PARATR_HYPHENZONE                   TypedWhichId<SvxHyphenZoneItem>(69)
#define    RES_PARATR_DROP                         TypedWhichId<SwFormatDrop>(70)
#define    RES_PARATR_REGISTER                     TypedWhichId<SwRegisterItem>(71)
#define    RES_PARATR_NUMRULE                      TypedWhichId<SwNumRuleItem>(72)
#define    RES_PARATR_SCRIPTSPACE                  TypedWhichId<SvxScriptSpaceItem>(73)
#define    RES_PARATR_HANGINGPUNCTUATION           TypedWhichId<SvxHangingPunctuationItem>(74)
#define    RES_PARATR_FORBIDDEN_RULES              TypedWhichId<SvxForbiddenRuleItem>(75)
#define    RES_PARATR_VERTALIGN                    TypedWhichId<SvxParaVertAlignItem>(76)
#define    RES_PARATR_SNAPTOGRID                   TypedWhichId<SvxParaGridItem>(77)
#define    RES_PARATR_CONNECT_BORDER               TypedWhichId<SwParaConnectBorderItem>(78)
#define    RES_PARATR_OUTLINELEVEL                 TypedWhichId<SfxUInt16Item>(79)
#define    RES_PARATR_RSID                         TypedWhichId<SvxRsidItem>(80)
#define    RES_PARATR_GRABBAG                      TypedWhichId<SfxGrabBagItem>(81)
#define RES_PARATR_END (82)

// list attributes for paragraphs.
// intentionally these list attributes are not contained in paragraph styles
#define RES_PARATR_LIST_BEGIN RES_PARATR_END
#define     RES_PARATR_LIST_ID                     TypedWhichId<SfxStringItem>(RES_PARATR_LIST_BEGIN) // 82
#define     RES_PARATR_LIST_LEVEL                  TypedWhichId<SfxInt16Item>(83)
#define     RES_PARATR_LIST_ISRESTART              TypedWhichId<SfxBoolItem>(84)
#define     RES_PARATR_LIST_RESTARTVALUE           TypedWhichId<SfxInt16Item>(85)
#define     RES_PARATR_LIST_ISCOUNTED              TypedWhichId<SfxBoolItem>(86)
#define RES_PARATR_LIST_END (87)

#define RES_FRMATR_BEGIN RES_PARATR_LIST_END
#define     RES_FILL_ORDER                         TypedWhichId<SwFormatFillOrder>(RES_FRMATR_BEGIN)
#define     RES_FRM_SIZE                           TypedWhichId<SwFormatFrameSize>(88)
#define     RES_PAPER_BIN                          TypedWhichId<SvxPaperBinItem>(89)
#define     RES_LR_SPACE                           TypedWhichId<SvxLRSpaceItem>(90)
#define     RES_UL_SPACE                           TypedWhichId<SvxULSpaceItem>(91)
#define     RES_PAGEDESC                           TypedWhichId<SwFormatPageDesc>(92)
#define     RES_BREAK                              TypedWhichId<SvxFormatBreakItem>(93)
#define     RES_CNTNT                              TypedWhichId<SwFormatContent>(94)
#define     RES_HEADER                             TypedWhichId<SwFormatHeader>(95)
#define     RES_FOOTER                             TypedWhichId<SwFormatFooter>(96)
#define     RES_PRINT                              TypedWhichId<SvxPrintItem>(97)
#define     RES_OPAQUE                             TypedWhichId<SvxOpaqueItem>(98)
#define     RES_PROTECT                            TypedWhichId<SvxProtectItem>(99)
#define     RES_SURROUND                           TypedWhichId<SwFormatSurround>(100)
#define     RES_VERT_ORIENT                        TypedWhichId<SwFormatVertOrient>(101)
#define     RES_HORI_ORIENT                        TypedWhichId<SwFormatHoriOrient>(102)
#define     RES_ANCHOR                             TypedWhichId<SwFormatAnchor>(103)
#define     RES_BACKGROUND                         TypedWhichId<SvxBrushItem>(104)
#define     RES_BOX                                TypedWhichId<SvxBoxItem>(105)
#define     RES_SHADOW                             TypedWhichId<SvxShadowItem>(106)
#define     RES_FRMMACRO                           TypedWhichId<SvxMacroItem>(107)
#define     RES_COL                                TypedWhichId<SwFormatCol>(108)
#define     RES_KEEP                               TypedWhichId<SvxFormatKeepItem>(109)
#define     RES_URL                                TypedWhichId<SwFormatURL>(110)
#define     RES_EDIT_IN_READONLY                   TypedWhichId<SwFormatEditInReadonly>(111)
#define     RES_LAYOUT_SPLIT                       TypedWhichId<SwFormatLayoutSplit>(112)
#define     RES_CHAIN                              TypedWhichId<SwFormatChain>(113)
#define     RES_TEXTGRID                           TypedWhichId<SwTextGridItem>(114)
#define     RES_LINENUMBER                         TypedWhichId<SwFormatLineNumber>(115)
#define     RES_FTN_AT_TXTEND                      TypedWhichId<SwFormatFootnoteAtTextEnd>(116)
#define     RES_END_AT_TXTEND                      TypedWhichId<SwFormatEndAtTextEnd>(117)
#define     RES_COLUMNBALANCE                      TypedWhichId<SwFormatNoBalancedColumns>(118)
#define     RES_FRAMEDIR                           TypedWhichId<SvxFrameDirectionItem>(119)
#define     RES_HEADER_FOOTER_EAT_SPACING          TypedWhichId<SwHeaderAndFooterEatSpacingItem>(120)
#define     RES_ROW_SPLIT                          TypedWhichId<SwFormatRowSplit>(121)
#define     RES_FOLLOW_TEXT_FLOW                   TypedWhichId<SwFormatFollowTextFlow>(122)
#define     RES_COLLAPSING_BORDERS                 TypedWhichId<SfxBoolItem>(123)
#define     RES_WRAP_INFLUENCE_ON_OBJPOS           TypedWhichId<SwFormatWrapInfluenceOnObjPos>(124)
#define     RES_AUTO_STYLE                         TypedWhichId<SwFormatAutoFormat>(125)
#define     RES_FRMATR_STYLE_NAME                  TypedWhichId<SfxStringItem>(126)
#define     RES_FRMATR_CONDITIONAL_STYLE_NAME      TypedWhichId<SfxStringItem>(127)
#define     RES_FRMATR_GRABBAG                     TypedWhichId<SfxGrabBagItem>(128)
#define     RES_TEXT_VERT_ADJUST                   TypedWhichId<SdrTextVertAdjustItem>(129)
#define RES_FRMATR_END 130

#define RES_GRFATR_BEGIN RES_FRMATR_END
#define     RES_GRFATR_MIRRORGRF    TypedWhichId<SwMirrorGrf>(RES_GRFATR_BEGIN) // 130
#define     RES_GRFATR_CROPGRF      TypedWhichId<SwCropGrf>(131)

#define     RES_GRFATR_ROTATION     TypedWhichId<SwRotationGrf>(132)
#define     RES_GRFATR_LUMINANCE    TypedWhichId<SwLuminanceGrf>(133)
#define     RES_GRFATR_CONTRAST     TypedWhichId<SwContrastGrf>(134)
#define     RES_GRFATR_CHANNELR     TypedWhichId<SwChannelRGrf>(135)
#define     RES_GRFATR_CHANNELG     TypedWhichId<SwChannelGGrf>(136)
#define     RES_GRFATR_CHANNELB     TypedWhichId<SwChannelBGrf>(137)
#define     RES_GRFATR_GAMMA        TypedWhichId<SwGammaGrf>(138)
#define     RES_GRFATR_INVERT       TypedWhichId<SwInvertGrf>(139)
#define     RES_GRFATR_TRANSPARENCY TypedWhichId<SwTransparencyGrf>(140)
#define     RES_GRFATR_DRAWMODE     TypedWhichId<SwDrawModeGrf>(141)

#define     RES_GRFATR_DUMMY1       TypedWhichId<SfxBoolItem>(142)
#define     RES_GRFATR_DUMMY2       TypedWhichId<SfxBoolItem>(143)
#define     RES_GRFATR_DUMMY3       TypedWhichId<SfxBoolItem>(144)
#define     RES_GRFATR_DUMMY4       TypedWhichId<SfxBoolItem>(145)
#define     RES_GRFATR_DUMMY5       TypedWhichId<SfxBoolItem>(146)
#define RES_GRFATR_END (147)

#define RES_BOXATR_BEGIN RES_GRFATR_END
#define     RES_BOXATR_FORMAT       TypedWhichId<SwTableBoxNumFormat>(RES_BOXATR_BEGIN) // 147
#define     RES_BOXATR_FORMULA      TypedWhichId<SwTableBoxFormula>(148)
#define     RES_BOXATR_VALUE        TypedWhichId<SwTableBoxValue>(149)
#define RES_BOXATR_END (150)

#define RES_UNKNOWNATR_BEGIN RES_BOXATR_END
#define     RES_UNKNOWNATR_CONTAINER TypedWhichId<SvXMLAttrContainerItem>(RES_UNKNOWNATR_BEGIN)// 150
#define RES_UNKNOWNATR_END (151)

// Format IDs
#define RES_FMT_BEGIN RES_UNKNOWNATR_END
#define     RES_CHRFMT                TypedWhichId<SwCharFormat>(RES_FMT_BEGIN)  // 151
#define     RES_FRMFMT                TypedWhichId<SwFrameFormat>(152)
#define     RES_FLYFRMFMT             TypedWhichId<SwFlyFrameFormat>(153)
#define     RES_TXTFMTCOLL            TypedWhichId<SwTextFormatColl>(154)
#define     RES_GRFFMTCOLL            TypedWhichId<SwGrfFormatColl>(155)
#define     RES_DRAWFRMFMT            TypedWhichId<SwDrawFrameFormat>(156)
#define     RES_CONDTXTFMTCOLL        TypedWhichId<SwConditionTextFormatColl>(157)
#define RES_FMT_END 158

// ID's for Messages in the Formats
#define RES_MSG_BEGIN RES_FMT_END
#define     RES_OBJECTDYING           TypedWhichId<SwPtrMsgPoolItem>(RES_MSG_BEGIN)                // 158
#define     RES_FMT_CHG               TypedWhichId<SwFormatChg>(159)
#define     RES_ATTRSET_CHG           TypedWhichId<SwAttrSetChg>(160)
#define     RES_INS_TXT               TypedWhichId<SwInsText>(161)
#define     RES_DEL_CHR               TypedWhichId<SwDelChr>(162)
#define     RES_DEL_TXT               TypedWhichId<SwDelText>(163)
#define     RES_UPDATE_ATTR           TypedWhichId<SwUpdateAttr>(164)
#define     RES_REFMARKFLD_UPDATE     TypedWhichId<SwRefMarkFieldUpdate>(165)
#define     RES_DOCPOS_UPDATE         TypedWhichId<SwDocPosUpdate>(166)
#define     RES_TABLEFML_UPDATE       TypedWhichId<SwTableFormulaUpdate>(167)
#define     RES_UPDATEDDETBL          TypedWhichId<SwMsgPoolItem>(168)
#define     RES_TBLHEADLINECHG        TypedWhichId<SwMsgPoolItem>(169)
#define     RES_AUTOFMT_DOCNODE       TypedWhichId<SwAutoFormatGetDocNode>(170)
#define     RES_SECTION_HIDDEN        TypedWhichId<SwMsgPoolItem>(171)
#define     RES_SECTION_NOT_HIDDEN    TypedWhichId<SwMsgPoolItem>(172)
#define     RES_GRAPHIC_ARRIVED       TypedWhichId<SwMsgPoolItem>(173)
#define     RES_GRAPHIC_PIECE_ARRIVED TypedWhichId<SwMsgPoolItem>(174)
#define     RES_HIDDENPARA_PRINT      TypedWhichId<SwMsgPoolItem>(175)
#define     RES_CONDCOLL_CONDCHG      TypedWhichId<SwCondCollCondChg>(176)
#define     RES_VIRTPAGENUM_INFO      TypedWhichId<SwVirtPageNumInfo>(177)
// empty
#define     RES_REMOVE_UNO_OBJECT     TypedWhichId<SwPtrMsgPoolItem>(179)
#define     RES_GRF_REREAD_AND_INCACHE   TypedWhichId<SwMsgPoolItem>(180)
// empty
#define     RES_FINDNEARESTNODE       TypedWhichId<SwFindNearestNode>(182)
#define     RES_CONTENT_VISIBLE       TypedWhichId<SwPtrMsgPoolItem>(183)
#define     RES_GRAPHIC_SWAPIN        TypedWhichId<SwMsgPoolItem>(184)
#define     RES_NAME_CHANGED          TypedWhichId<SwStringMsgPoolItem>(185)
#define     RES_TITLE_CHANGED         TypedWhichId<SwStringMsgPoolItem>(186)
#define     RES_DESCRIPTION_CHANGED   TypedWhichId<SwStringMsgPoolItem>(187)
#define     RES_LINKED_GRAPHIC_STREAM_ARRIVED TypedWhichId<SwMsgPoolItem>(187)
#define RES_MSG_END (188)

// An ID for the RTF-reader. The stylesheets are treated like attributes,
// i.e. there is a StyleSheet-attribute. To avoid collision with other
// Which()-values, the value is listed here. (The help system too defines
// new attributes!)
#define     RES_FLTRATTR_BEGIN RES_MSG_END
#define     RES_FLTR_BOOKMARK       TypedWhichId<SfxStringItem>(RES_FLTRATTR_BEGIN)
#define     RES_FLTR_ANCHOR         TypedWhichId<SwFltAnchor>(189)
#define     RES_FLTR_NUMRULE        TypedWhichId<SfxStringItem>(190)
#define     RES_FLTR_TOX            TypedWhichId<SwFltTOX>(191)
#define     RES_FLTR_REDLINE        TypedWhichId<SwFltRedline>(192)
#define     RES_FLTR_ANNOTATIONMARK TypedWhichId<CntUInt16Item>(193)
#define     RES_FLTR_RDFMARK        TypedWhichId<SwFltRDFMark>(194)
#define     RES_FLTRATTR_END 195

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
    return (RES_PARATR_LIST_BEGIN <= nWhich) && (RES_PARATR_LIST_END > nWhich);
}
inline bool isFRMATR(const sal_uInt16 nWhich)
{
    return (RES_FRMATR_BEGIN <= nWhich) && (RES_FRMATR_END > nWhich);
}
inline bool isDrawingLayerAttribute(const sal_uInt16 nWhich)
{
    return (XATTR_START <= nWhich) && (XATTR_END > nWhich);
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
typedef std::vector<SfxPoolItem*> SwDfltAttrTab;

extern SwDfltAttrTab aAttrTab;
extern SfxItemInfo   aSlotTab[];

/** Get the default attribute from corresponding default attribute table.

    @param[in] nWhich Position in table
    @return Attribute if found, null pointer otherwise
*/
SW_DLLPUBLIC const SfxPoolItem* GetDfltAttr( sal_uInt16 nWhich );

template<class T> inline const T* GetDfltAttr( TypedWhichId<T> nWhich )
{ return static_cast<const T*>(GetDfltAttr(sal_uInt16(nWhich))); }

SW_DLLPUBLIC sal_uInt16 GetWhichOfScript( sal_uInt16 nWhich, sal_uInt16 nScript );

// return for the given TextAttribute without an end the correct character.
// This function returns
//      CH_TXTATR_BREAKWORD for Textattribute which breaks a word (default)
//      CH_TXTATR_INWORD    for Textattribute which doesn't breaks a word
class SwTextAttr;
sal_Unicode GetCharOfTextAttr( const SwTextAttr& rAttr );

// all Sets defined in init.cxx

// AttrSet-Range for the 3 Break-Attribute
extern sal_uInt16 aBreakSetRange[];
// AttrSet-Range for TextFormatColl
extern sal_uInt16 aTextFormatCollSetRange[];
// AttrSet-Range for GrfFormatColl
extern sal_uInt16 aGrfFormatCollSetRange[];
// AttrSet-Range for TextNode
SW_DLLPUBLIC extern sal_uInt16 aTextNodeSetRange[];
// AttrSet-Range for NoTextNode
extern sal_uInt16 aNoTextNodeSetRange[];
// AttrSet-Range for SwTable
extern sal_uInt16 aTableSetRange[];
// AttrSet-Range for SwTableLine
extern sal_uInt16 aTableLineSetRange[];
// AttrSet-Range for SwTableBox
extern sal_uInt16 aTableBoxSetRange[];
// AttrSet-Range for SwFrameFormat
SW_DLLPUBLIC extern sal_uInt16 aFrameFormatSetRange[];
// AttrSet-Range for SwCharFormat
extern sal_uInt16 aCharFormatSetRange[];
// AttrSet-Range for the autostyles
extern sal_uInt16 aCharAutoFormatSetRange[];
// AttrSet-Range for SwPageDescFormat
extern sal_uInt16 aPgFrameFormatSetRange[];

// check if ID is InRange of AttrSet-Ids
bool IsInRange( const sal_uInt16* pRange, const sal_uInt16 nId );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
