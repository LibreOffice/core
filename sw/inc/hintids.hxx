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

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <svx/xdef.hxx>
#include "swdllapi.h"
#include <svl/typedwhich.hxx>
#include <svl/whichranges.hxx>
#include <vector>

class SfxStringItem;
class SwFormatChg;
class SwUpdateAttr;
class SwFormatMeta;
class SwFormatContentControl;
class SvXMLAttrContainerItem;
class SwMsgPoolItem;
class SfxBoolItem;
class SvxColorItem;
class SvxLeftMarginItem;
class SvxTextLeftMarginItem;
class SvxFirstLineIndentItem;
class SvxRightMarginItem;
class SvxGutterLeftMarginItem;
class SvxGutterRightMarginItem;
class SvxLRSpaceItem;
class SdrTextVertAdjustItem;
class SfxGrabBagItem;
class SfxInt16Item;
class SfxUInt16Item;
class SvxAdjustItem;
class SvxAutoKernItem;
class SvxBlinkItem;
class SvxBoxItem;
class SvxBrushItem;
class SvxCaseMapItem;
class SvxCharHiddenItem;
class SvxCharReliefItem;
class SvxCharRotateItem;
class SvxCharScaleWidthItem;
class SvxContourItem;
class SvxCrossedOutItem;
class SvxEmphasisMarkItem;
class SvxEscapementItem;
class SvxFontHeightItem;
class SvxFontItem;
class SvxForbiddenRuleItem;
class SvxFormatBreakItem;
class SvxFormatKeepItem;
class SvxFormatSplitItem;
class SvxFrameDirectionItem;
class SvxHangingPunctuationItem;
class SvxHyphenZoneItem;
class SvxKerningItem;
class SvxLanguageItem;
class SvxLineSpacingItem;
class SvxNoHyphenItem;
class SvxOpaqueItem;
class SvxOrphansItem;
class SvxOverlineItem;
class SvxPaperBinItem;
class SvxParaGridItem;
class SvxParaVertAlignItem;
class SvxPostureItem;
class SvxPrintItem;
class SvxProtectItem;
class SvxRsidItem;
class SvxScriptSpaceItem;
class SvxScriptHintItem;
class SvxShadowedItem;
class SvxShadowItem;
class SvxTabStopItem;
class SvxTwoLinesItem;
class SvxULSpaceItem;
class SvxUnderlineItem;
class SvxWeightItem;
class SvxWidowsItem;
class SvxWordLineModeItem;
class SwChannelBGrf;
class SwChannelGGrf;
class SwChannelRGrf;
class SwContrastGrf;
class SwCropGrf;
class SwDrawModeGrf;
class SwFormatAnchor;
class SwFormatAutoFormat;
class SwFormatChain;
class SwFormatCharFormat;
class SwFormatCol;
class SwFormatDrop;
class SwFormatEditInReadonly;
class SwFormatEndAtTextEnd;
class SwFormatFlyCnt;
class SwFormatFollowTextFlow;
class SwFormatFootnoteAtTextEnd;
class SwFormatFrameSize;
class SwFormatHoriOrient;
class SwFormatINetFormat;
class SwFormatLayoutSplit;
class SwFormatLineNumber;
class SwFormatNoBalancedColumns;
class SwFormatPageDesc;
class SwFormatRowSplit;
class SwFormatFlySplit;
class SwFormatWrapTextAtFlyStart;
class SwFormatSurround;
class SwFormatURL;
class SwFormatVertOrient;
class SwFormatWrapInfluenceOnObjPos;
class SwGammaGrf;
class SwHeaderAndFooterEatSpacingItem;
class SwInvertGrf;
class SwLuminanceGrf;
class SwMirrorGrf;
class SwNumRuleItem;
class SwParaConnectBorderItem;
class SwRegisterItem;
class SwRotationGrf;
class SwTableBoxNumFormat;
class SwTextGridItem;
class SwTransparencyGrf;
class SwFormatRuby;
class SwFltAnchor;
class SwFltTOX;
class SwFltRedline;
class SwFltRDFMark;
class SwCharFormat;
class SwConditionTextFormatColl;
class SwDrawFrameFormat;
class SwFlyFrameFormat;
class SwFormatField;
class SwFormatFootnote;
class SwFormatLineBreak;
class SwFormatRefMark;
class SwGrfFormatColl;
class SwTextFormatColl;
class SwTOXMark;
class CntUInt16Item;
class SwFormatFillOrder;
class SwFormatHeader;
class SwFormatFooter;
class SwFormatContent;
class SvxMacroItem;
class SwTableBoxFormula;
class SwTableBoxValue;
class SwFrameFormat;
class SfxVoidItem;

// For SwTextHints without end index the following char is added:

#define CH_TXTATR_BREAKWORD u'\x0001'
#define CH_TXTATR_INWORD u'\xFFF9'
#define CH_TXTATR_TAB u'\t'
#define CH_TXTATR_NEWLINE u'\n'
#define CH_TXT_ATR_INPUTFIELDSTART u'\x0004'
#define CH_TXT_ATR_INPUTFIELDEND u'\x0005'

#define CH_TXT_ATR_FORMELEMENT u'\x0006'

#define CH_TXT_ATR_FIELDSTART u'\x0007'
#define CH_TXT_ATR_FIELDSEP u'\x0003'
#define CH_TXT_ATR_FIELDEND u'\x0008'
inline constexpr OUString CH_TXT_ATR_SUBST_FIELDSTART = u"["_ustr;
inline constexpr OUString CH_TXT_ATR_SUBST_FIELDEND = u"]"_ustr;

// a non-visible dummy character to track deleted tables,
// table rows, and images anchored to characters
#define CH_TXT_TRACKED_DUMMY_CHAR u'\x200D'

/*
 * Enums for the hints
 */

inline constexpr sal_uInt16 HINT_BEGIN = 1;

inline constexpr sal_uInt16 POOLATTR_BEGIN(HINT_BEGIN);

// Ranges for the IDs of the format-attributes.
// Which-values for character-format attributes.
inline constexpr sal_uInt16 RES_CHRATR_BEGIN(HINT_BEGIN);
inline constexpr TypedWhichId<SvxCaseMapItem> RES_CHRATR_CASEMAP(RES_CHRATR_BEGIN + 0); // 1
inline constexpr TypedWhichId<SvxColorItem> RES_CHRATR_CHARSETCOLOR(RES_CHRATR_BEGIN + 1);
inline constexpr TypedWhichId<SvxColorItem> RES_CHRATR_COLOR(RES_CHRATR_BEGIN + 2);
inline constexpr TypedWhichId<SvxContourItem> RES_CHRATR_CONTOUR(RES_CHRATR_BEGIN + 3);
inline constexpr TypedWhichId<SvxCrossedOutItem> RES_CHRATR_CROSSEDOUT(RES_CHRATR_BEGIN + 4);
inline constexpr TypedWhichId<SvxEscapementItem> RES_CHRATR_ESCAPEMENT(RES_CHRATR_BEGIN + 5);
inline constexpr TypedWhichId<SvxFontItem> RES_CHRATR_FONT(RES_CHRATR_BEGIN + 6);
inline constexpr TypedWhichId<SvxFontHeightItem> RES_CHRATR_FONTSIZE(RES_CHRATR_BEGIN + 7);
inline constexpr TypedWhichId<SvxKerningItem> RES_CHRATR_KERNING(RES_CHRATR_BEGIN + 8);
inline constexpr TypedWhichId<SvxLanguageItem> RES_CHRATR_LANGUAGE(RES_CHRATR_BEGIN + 9);
inline constexpr TypedWhichId<SvxPostureItem> RES_CHRATR_POSTURE(RES_CHRATR_BEGIN + 10);
inline constexpr TypedWhichId<SfxVoidItem> RES_CHRATR_UNUSED1(RES_CHRATR_BEGIN + 11);
inline constexpr TypedWhichId<SvxShadowedItem> RES_CHRATR_SHADOWED(RES_CHRATR_BEGIN + 12);
inline constexpr TypedWhichId<SvxUnderlineItem> RES_CHRATR_UNDERLINE(RES_CHRATR_BEGIN + 13);
inline constexpr TypedWhichId<SvxWeightItem> RES_CHRATR_WEIGHT(RES_CHRATR_BEGIN + 14);
inline constexpr TypedWhichId<SvxWordLineModeItem> RES_CHRATR_WORDLINEMODE(RES_CHRATR_BEGIN + 15);
inline constexpr TypedWhichId<SvxAutoKernItem> RES_CHRATR_AUTOKERN(RES_CHRATR_BEGIN + 16);
inline constexpr TypedWhichId<SvxBlinkItem> RES_CHRATR_BLINK(RES_CHRATR_BEGIN + 17);
inline constexpr TypedWhichId<SvxNoHyphenItem> RES_CHRATR_NOHYPHEN(RES_CHRATR_BEGIN + 18);
inline constexpr TypedWhichId<SfxVoidItem> RES_CHRATR_UNUSED2(RES_CHRATR_BEGIN + 19);
inline constexpr TypedWhichId<SvxBrushItem> RES_CHRATR_BACKGROUND(RES_CHRATR_BEGIN + 20);
inline constexpr TypedWhichId<SvxFontItem> RES_CHRATR_CJK_FONT(RES_CHRATR_BEGIN + 21);
inline constexpr TypedWhichId<SvxFontHeightItem> RES_CHRATR_CJK_FONTSIZE(RES_CHRATR_BEGIN + 22);
inline constexpr TypedWhichId<SvxLanguageItem> RES_CHRATR_CJK_LANGUAGE(RES_CHRATR_BEGIN + 23);
inline constexpr TypedWhichId<SvxPostureItem> RES_CHRATR_CJK_POSTURE(RES_CHRATR_BEGIN + 24);
inline constexpr TypedWhichId<SvxWeightItem> RES_CHRATR_CJK_WEIGHT(RES_CHRATR_BEGIN + 25);
inline constexpr TypedWhichId<SvxFontItem> RES_CHRATR_CTL_FONT(RES_CHRATR_BEGIN + 26);
inline constexpr TypedWhichId<SvxFontHeightItem> RES_CHRATR_CTL_FONTSIZE(RES_CHRATR_BEGIN + 27);
inline constexpr TypedWhichId<SvxLanguageItem> RES_CHRATR_CTL_LANGUAGE(RES_CHRATR_BEGIN + 28);
inline constexpr TypedWhichId<SvxPostureItem> RES_CHRATR_CTL_POSTURE(RES_CHRATR_BEGIN + 29);
inline constexpr TypedWhichId<SvxWeightItem> RES_CHRATR_CTL_WEIGHT(RES_CHRATR_BEGIN + 30);
inline constexpr TypedWhichId<SvxCharRotateItem> RES_CHRATR_ROTATE(RES_CHRATR_BEGIN + 31);
inline constexpr TypedWhichId<SvxEmphasisMarkItem> RES_CHRATR_EMPHASIS_MARK(RES_CHRATR_BEGIN + 32);
inline constexpr TypedWhichId<SvxTwoLinesItem> RES_CHRATR_TWO_LINES(RES_CHRATR_BEGIN + 33);
inline constexpr TypedWhichId<SvxCharScaleWidthItem> RES_CHRATR_SCALEW(RES_CHRATR_BEGIN + 34);
inline constexpr TypedWhichId<SvxCharReliefItem> RES_CHRATR_RELIEF(RES_CHRATR_BEGIN + 35);
inline constexpr TypedWhichId<SvxCharHiddenItem> RES_CHRATR_HIDDEN(RES_CHRATR_BEGIN + 36);
inline constexpr TypedWhichId<SvxOverlineItem> RES_CHRATR_OVERLINE(RES_CHRATR_BEGIN + 37);
inline constexpr TypedWhichId<SvxRsidItem> RES_CHRATR_RSID(RES_CHRATR_BEGIN + 38);
inline constexpr TypedWhichId<SvxBoxItem> RES_CHRATR_BOX(RES_CHRATR_BEGIN + 39);
inline constexpr TypedWhichId<SvxShadowItem> RES_CHRATR_SHADOW(RES_CHRATR_BEGIN + 40);
inline constexpr TypedWhichId<SvxBrushItem> RES_CHRATR_HIGHLIGHT(RES_CHRATR_BEGIN + 41);
inline constexpr TypedWhichId<SfxGrabBagItem> RES_CHRATR_GRABBAG(RES_CHRATR_BEGIN + 42);
inline constexpr TypedWhichId<SfxInt16Item> RES_CHRATR_BIDIRTL(RES_CHRATR_BEGIN + 43);
inline constexpr TypedWhichId<SfxInt16Item> RES_CHRATR_IDCTHINT(RES_CHRATR_BEGIN + 44);
inline constexpr TypedWhichId<SvxScriptHintItem> RES_CHRATR_SCRIPT_HINT(RES_CHRATR_BEGIN + 45);
inline constexpr sal_uInt16 RES_CHRATR_END(RES_CHRATR_BEGIN + 46);

// this Attribute used only in a TextNodes SwpAttr-Array
inline constexpr sal_uInt16 RES_TXTATR_BEGIN(RES_CHRATR_END);

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
inline constexpr sal_uInt16 RES_TXTATR_WITHEND_BEGIN(RES_TXTATR_BEGIN);
inline constexpr TypedWhichId<SwFormatRefMark> RES_TXTATR_REFMARK(RES_TXTATR_WITHEND_BEGIN
                                                                  + 0); // 47
inline constexpr TypedWhichId<SwTOXMark> RES_TXTATR_TOXMARK(RES_TXTATR_WITHEND_BEGIN + 1);
inline constexpr TypedWhichId<SwFormatMeta> RES_TXTATR_META(RES_TXTATR_WITHEND_BEGIN + 2);
inline constexpr TypedWhichId<SwFormatMeta> RES_TXTATR_METAFIELD(RES_TXTATR_WITHEND_BEGIN + 3);
inline constexpr TypedWhichId<SwFormatAutoFormat> RES_TXTATR_AUTOFMT(RES_TXTATR_WITHEND_BEGIN + 4);
inline constexpr TypedWhichId<SwFormatINetFormat> RES_TXTATR_INETFMT(RES_TXTATR_WITHEND_BEGIN + 5);
inline constexpr TypedWhichId<SwFormatCharFormat> RES_TXTATR_CHARFMT(RES_TXTATR_WITHEND_BEGIN + 6);
inline constexpr TypedWhichId<SwFormatRuby> RES_TXTATR_CJK_RUBY(RES_TXTATR_WITHEND_BEGIN + 7);
inline constexpr TypedWhichId<SvXMLAttrContainerItem>
    RES_TXTATR_UNKNOWN_CONTAINER(RES_TXTATR_WITHEND_BEGIN + 8);
inline constexpr TypedWhichId<SwFormatField> RES_TXTATR_INPUTFIELD(RES_TXTATR_WITHEND_BEGIN + 9);
inline constexpr TypedWhichId<SwFormatContentControl>
    RES_TXTATR_CONTENTCONTROL(RES_TXTATR_WITHEND_BEGIN + 10);
inline constexpr sal_uInt16 RES_TXTATR_WITHEND_END(RES_TXTATR_WITHEND_BEGIN + 11);

// all TextAttributes without an end
inline constexpr sal_uInt16 RES_TXTATR_NOEND_BEGIN(RES_TXTATR_WITHEND_END);
inline constexpr TypedWhichId<SwFormatField> RES_TXTATR_FIELD(RES_TXTATR_NOEND_BEGIN + 0); // 58
inline constexpr TypedWhichId<SwFormatFlyCnt> RES_TXTATR_FLYCNT(RES_TXTATR_NOEND_BEGIN + 1);
inline constexpr TypedWhichId<SwFormatFootnote> RES_TXTATR_FTN(RES_TXTATR_NOEND_BEGIN + 2);
inline constexpr TypedWhichId<SwFormatField> RES_TXTATR_ANNOTATION(RES_TXTATR_NOEND_BEGIN + 3);
inline constexpr TypedWhichId<SwFormatLineBreak> RES_TXTATR_LINEBREAK(RES_TXTATR_NOEND_BEGIN + 4);
inline constexpr TypedWhichId<SfxBoolItem> RES_TXTATR_DUMMY1(RES_TXTATR_NOEND_BEGIN + 5);
inline constexpr sal_uInt16 RES_TXTATR_NOEND_END(RES_TXTATR_NOEND_BEGIN + 6);
inline constexpr sal_uInt16 RES_TXTATR_END(RES_TXTATR_NOEND_END);

inline constexpr sal_uInt16 RES_PARATR_BEGIN(RES_TXTATR_END);
inline constexpr TypedWhichId<SvxLineSpacingItem> RES_PARATR_LINESPACING(RES_PARATR_BEGIN
                                                                         + 0); // 64
inline constexpr TypedWhichId<SvxAdjustItem> RES_PARATR_ADJUST(RES_PARATR_BEGIN + 1);
inline constexpr TypedWhichId<SvxFormatSplitItem> RES_PARATR_SPLIT(RES_PARATR_BEGIN + 2);
inline constexpr TypedWhichId<SvxOrphansItem> RES_PARATR_ORPHANS(RES_PARATR_BEGIN + 3);
inline constexpr TypedWhichId<SvxWidowsItem> RES_PARATR_WIDOWS(RES_PARATR_BEGIN + 4);
inline constexpr TypedWhichId<SvxTabStopItem> RES_PARATR_TABSTOP(RES_PARATR_BEGIN + 5);
inline constexpr TypedWhichId<SvxHyphenZoneItem> RES_PARATR_HYPHENZONE(RES_PARATR_BEGIN + 6);
inline constexpr TypedWhichId<SwFormatDrop> RES_PARATR_DROP(RES_PARATR_BEGIN + 7);
inline constexpr TypedWhichId<SwRegisterItem> RES_PARATR_REGISTER(RES_PARATR_BEGIN + 8);
inline constexpr TypedWhichId<SwNumRuleItem> RES_PARATR_NUMRULE(RES_PARATR_BEGIN + 9);
inline constexpr TypedWhichId<SvxScriptSpaceItem> RES_PARATR_SCRIPTSPACE(RES_PARATR_BEGIN + 10);
inline constexpr TypedWhichId<SvxHangingPunctuationItem>
    RES_PARATR_HANGINGPUNCTUATION(RES_PARATR_BEGIN + 11);
inline constexpr TypedWhichId<SvxForbiddenRuleItem> RES_PARATR_FORBIDDEN_RULES(RES_PARATR_BEGIN
                                                                               + 12);
inline constexpr TypedWhichId<SvxParaVertAlignItem> RES_PARATR_VERTALIGN(RES_PARATR_BEGIN + 13);
inline constexpr TypedWhichId<SvxParaGridItem> RES_PARATR_SNAPTOGRID(RES_PARATR_BEGIN + 14);
inline constexpr TypedWhichId<SwParaConnectBorderItem> RES_PARATR_CONNECT_BORDER(RES_PARATR_BEGIN
                                                                                 + 15);
inline constexpr TypedWhichId<SfxUInt16Item> RES_PARATR_OUTLINELEVEL(RES_PARATR_BEGIN + 16);
inline constexpr TypedWhichId<SvxRsidItem> RES_PARATR_RSID(RES_PARATR_BEGIN + 17);
inline constexpr TypedWhichId<SfxGrabBagItem> RES_PARATR_GRABBAG(RES_PARATR_BEGIN + 18);
inline constexpr sal_uInt16 RES_PARATR_END(RES_PARATR_BEGIN + 19);

// list attributes for paragraphs.
// intentionally these list attributes are not contained in paragraph styles
inline constexpr sal_uInt16 RES_PARATR_LIST_BEGIN(RES_PARATR_END);
inline constexpr TypedWhichId<SfxStringItem> RES_PARATR_LIST_ID(RES_PARATR_LIST_BEGIN + 0); // 83
inline constexpr TypedWhichId<SfxInt16Item> RES_PARATR_LIST_LEVEL(RES_PARATR_LIST_BEGIN + 1);
inline constexpr TypedWhichId<SfxBoolItem> RES_PARATR_LIST_ISRESTART(RES_PARATR_LIST_BEGIN + 2);
inline constexpr TypedWhichId<SfxInt16Item> RES_PARATR_LIST_RESTARTVALUE(RES_PARATR_LIST_BEGIN + 3);
inline constexpr TypedWhichId<SfxBoolItem> RES_PARATR_LIST_ISCOUNTED(RES_PARATR_LIST_BEGIN + 4);
inline constexpr TypedWhichId<SwFormatAutoFormat>
    RES_PARATR_LIST_AUTOFMT(RES_PARATR_LIST_BEGIN + 5); //TypedWhichId<SfxSetItem>(88)
inline constexpr sal_uInt16 RES_PARATR_LIST_END(RES_PARATR_LIST_BEGIN + 6);

inline constexpr sal_uInt16 RES_FRMATR_BEGIN(RES_PARATR_LIST_END);
inline constexpr TypedWhichId<SwFormatFillOrder> RES_FILL_ORDER(RES_FRMATR_BEGIN + 0);
inline constexpr TypedWhichId<SwFormatFrameSize> RES_FRM_SIZE(RES_FRMATR_BEGIN + 1);
inline constexpr TypedWhichId<SvxPaperBinItem> RES_PAPER_BIN(RES_FRMATR_BEGIN + 2);
inline constexpr TypedWhichId<SvxFirstLineIndentItem> RES_MARGIN_FIRSTLINE(RES_FRMATR_BEGIN + 3);
inline constexpr TypedWhichId<SvxTextLeftMarginItem> RES_MARGIN_TEXTLEFT(RES_FRMATR_BEGIN + 4);
inline constexpr TypedWhichId<SvxRightMarginItem> RES_MARGIN_RIGHT(RES_FRMATR_BEGIN + 5);
inline constexpr TypedWhichId<SvxLeftMarginItem> RES_MARGIN_LEFT(RES_FRMATR_BEGIN + 6);
inline constexpr TypedWhichId<SvxGutterLeftMarginItem> RES_MARGIN_GUTTER(RES_FRMATR_BEGIN + 7);
inline constexpr TypedWhichId<SvxGutterRightMarginItem> RES_MARGIN_GUTTER_RIGHT(RES_FRMATR_BEGIN
                                                                                + 8);
inline constexpr TypedWhichId<SvxLRSpaceItem> RES_LR_SPACE(RES_FRMATR_BEGIN + 9);
inline constexpr TypedWhichId<SvxULSpaceItem> RES_UL_SPACE(RES_FRMATR_BEGIN + 10);
inline constexpr TypedWhichId<SwFormatPageDesc> RES_PAGEDESC(RES_FRMATR_BEGIN + 11);
inline constexpr TypedWhichId<SvxFormatBreakItem> RES_BREAK(RES_FRMATR_BEGIN + 12);
inline constexpr TypedWhichId<SwFormatContent> RES_CNTNT(RES_FRMATR_BEGIN + 13);
inline constexpr TypedWhichId<SwFormatHeader> RES_HEADER(RES_FRMATR_BEGIN + 14);
inline constexpr TypedWhichId<SwFormatFooter> RES_FOOTER(RES_FRMATR_BEGIN + 15);
inline constexpr TypedWhichId<SvxPrintItem> RES_PRINT(RES_FRMATR_BEGIN + 16);
inline constexpr TypedWhichId<SvxOpaqueItem> RES_OPAQUE(RES_FRMATR_BEGIN + 17);
inline constexpr TypedWhichId<SvxProtectItem> RES_PROTECT(RES_FRMATR_BEGIN + 18);
inline constexpr TypedWhichId<SwFormatSurround> RES_SURROUND(RES_FRMATR_BEGIN + 19);
inline constexpr TypedWhichId<SwFormatVertOrient> RES_VERT_ORIENT(RES_FRMATR_BEGIN + 20);
inline constexpr TypedWhichId<SwFormatHoriOrient> RES_HORI_ORIENT(RES_FRMATR_BEGIN + 21);
inline constexpr TypedWhichId<SwFormatAnchor> RES_ANCHOR(RES_FRMATR_BEGIN + 22);
inline constexpr TypedWhichId<SvxBrushItem> RES_BACKGROUND(RES_FRMATR_BEGIN + 23);
inline constexpr TypedWhichId<SvxBoxItem> RES_BOX(RES_FRMATR_BEGIN + 24);
inline constexpr TypedWhichId<SvxShadowItem> RES_SHADOW(RES_FRMATR_BEGIN + 25);
inline constexpr TypedWhichId<SvxMacroItem> RES_FRMMACRO(RES_FRMATR_BEGIN + 26);
inline constexpr TypedWhichId<SwFormatCol> RES_COL(RES_FRMATR_BEGIN + 27);
inline constexpr TypedWhichId<SvxFormatKeepItem> RES_KEEP(RES_FRMATR_BEGIN + 28);
inline constexpr TypedWhichId<SwFormatURL> RES_URL(RES_FRMATR_BEGIN + 29);
inline constexpr TypedWhichId<SwFormatEditInReadonly> RES_EDIT_IN_READONLY(RES_FRMATR_BEGIN + 30);
inline constexpr TypedWhichId<SwFormatLayoutSplit> RES_LAYOUT_SPLIT(RES_FRMATR_BEGIN + 31);
inline constexpr TypedWhichId<SwFormatChain> RES_CHAIN(RES_FRMATR_BEGIN + 32);
inline constexpr TypedWhichId<SwTextGridItem> RES_TEXTGRID(RES_FRMATR_BEGIN + 33);
inline constexpr TypedWhichId<SwFormatLineNumber> RES_LINENUMBER(RES_FRMATR_BEGIN + 34);
inline constexpr TypedWhichId<SwFormatFootnoteAtTextEnd> RES_FTN_AT_TXTEND(RES_FRMATR_BEGIN + 35);
inline constexpr TypedWhichId<SwFormatEndAtTextEnd> RES_END_AT_TXTEND(RES_FRMATR_BEGIN + 36);
inline constexpr TypedWhichId<SwFormatNoBalancedColumns> RES_COLUMNBALANCE(RES_FRMATR_BEGIN + 37);
inline constexpr TypedWhichId<SvxFrameDirectionItem> RES_FRAMEDIR(RES_FRMATR_BEGIN + 38);
inline constexpr TypedWhichId<SwHeaderAndFooterEatSpacingItem>
    RES_HEADER_FOOTER_EAT_SPACING(RES_FRMATR_BEGIN + 39);
inline constexpr TypedWhichId<SwFormatRowSplit> RES_ROW_SPLIT(RES_FRMATR_BEGIN + 40);
inline constexpr TypedWhichId<SwFormatFlySplit> RES_FLY_SPLIT(RES_FRMATR_BEGIN + 41);
inline constexpr TypedWhichId<SwFormatFollowTextFlow> RES_FOLLOW_TEXT_FLOW(RES_FRMATR_BEGIN + 42);
inline constexpr TypedWhichId<SfxBoolItem> RES_COLLAPSING_BORDERS(RES_FRMATR_BEGIN + 43);
inline constexpr TypedWhichId<SwFormatWrapInfluenceOnObjPos>
    RES_WRAP_INFLUENCE_ON_OBJPOS(RES_FRMATR_BEGIN + 44);
inline constexpr TypedWhichId<SwFormatAutoFormat> RES_AUTO_STYLE(RES_FRMATR_BEGIN + 45);
inline constexpr TypedWhichId<SfxStringItem> RES_FRMATR_STYLE_NAME(RES_FRMATR_BEGIN + 46);
inline constexpr TypedWhichId<SfxStringItem> RES_FRMATR_CONDITIONAL_STYLE_NAME(RES_FRMATR_BEGIN
                                                                               + 47);
inline constexpr TypedWhichId<SfxGrabBagItem> RES_FRMATR_GRABBAG(RES_FRMATR_BEGIN + 48);
inline constexpr TypedWhichId<SdrTextVertAdjustItem> RES_TEXT_VERT_ADJUST(RES_FRMATR_BEGIN + 49);
inline constexpr TypedWhichId<SfxBoolItem> RES_BACKGROUND_FULL_SIZE(RES_FRMATR_BEGIN + 50);
inline constexpr TypedWhichId<SfxBoolItem> RES_RTL_GUTTER(RES_FRMATR_BEGIN + 51);
inline constexpr TypedWhichId<SfxBoolItem> RES_DECORATIVE(RES_FRMATR_BEGIN + 52);
inline constexpr TypedWhichId<SwFormatWrapTextAtFlyStart>
    RES_WRAP_TEXT_AT_FLY_START(RES_FRMATR_BEGIN + 53);
inline constexpr sal_uInt16 RES_FRMATR_END(RES_FRMATR_BEGIN + 54);

inline constexpr sal_uInt16 RES_GRFATR_BEGIN(RES_FRMATR_END);
inline constexpr TypedWhichId<SwMirrorGrf> RES_GRFATR_MIRRORGRF(RES_GRFATR_BEGIN + 0);
inline constexpr TypedWhichId<SwCropGrf> RES_GRFATR_CROPGRF(RES_GRFATR_BEGIN + 1);

inline constexpr TypedWhichId<SwRotationGrf> RES_GRFATR_ROTATION(RES_GRFATR_BEGIN + 2);
inline constexpr TypedWhichId<SwLuminanceGrf> RES_GRFATR_LUMINANCE(RES_GRFATR_BEGIN + 3);
inline constexpr TypedWhichId<SwContrastGrf> RES_GRFATR_CONTRAST(RES_GRFATR_BEGIN + 4);
inline constexpr TypedWhichId<SwChannelRGrf> RES_GRFATR_CHANNELR(RES_GRFATR_BEGIN + 5);
inline constexpr TypedWhichId<SwChannelGGrf> RES_GRFATR_CHANNELG(RES_GRFATR_BEGIN + 6);
inline constexpr TypedWhichId<SwChannelBGrf> RES_GRFATR_CHANNELB(RES_GRFATR_BEGIN + 7);
inline constexpr TypedWhichId<SwGammaGrf> RES_GRFATR_GAMMA(RES_GRFATR_BEGIN + 8);
inline constexpr TypedWhichId<SwInvertGrf> RES_GRFATR_INVERT(RES_GRFATR_BEGIN + 9);
inline constexpr TypedWhichId<SwTransparencyGrf> RES_GRFATR_TRANSPARENCY(RES_GRFATR_BEGIN + 10);
inline constexpr TypedWhichId<SwDrawModeGrf> RES_GRFATR_DRAWMODE(RES_GRFATR_BEGIN + 11);

inline constexpr TypedWhichId<SfxBoolItem> RES_GRFATR_DUMMY4(RES_GRFATR_BEGIN + 12);
inline constexpr TypedWhichId<SfxBoolItem> RES_GRFATR_DUMMY5(RES_GRFATR_BEGIN + 13);
inline constexpr sal_uInt16 RES_GRFATR_END(RES_GRFATR_BEGIN + 14);

inline constexpr sal_uInt16 RES_BOXATR_BEGIN(RES_GRFATR_END);
inline constexpr TypedWhichId<SwTableBoxNumFormat> RES_BOXATR_FORMAT(RES_BOXATR_BEGIN + 0);
inline constexpr TypedWhichId<SwTableBoxFormula> RES_BOXATR_FORMULA(RES_BOXATR_BEGIN + 1);
inline constexpr TypedWhichId<SwTableBoxValue> RES_BOXATR_VALUE(RES_BOXATR_BEGIN + 2);
inline constexpr sal_uInt16 RES_BOXATR_END(RES_BOXATR_BEGIN + 3);

inline constexpr sal_uInt16 RES_UNKNOWNATR_BEGIN(RES_BOXATR_END);
inline constexpr TypedWhichId<SvXMLAttrContainerItem> RES_UNKNOWNATR_CONTAINER(RES_UNKNOWNATR_BEGIN
                                                                               + 0);
inline constexpr sal_uInt16 RES_UNKNOWNATR_END(RES_UNKNOWNATR_BEGIN + 1);

inline constexpr sal_uInt16 POOLATTR_END(RES_UNKNOWNATR_END);

// Format IDs
inline constexpr sal_uInt16 RES_FMT_BEGIN(RES_UNKNOWNATR_END);
inline constexpr TypedWhichId<SwCharFormat> RES_CHRFMT(RES_FMT_BEGIN + 0);
inline constexpr TypedWhichId<SwFrameFormat> RES_FRMFMT(RES_FMT_BEGIN + 1);
inline constexpr TypedWhichId<SwFlyFrameFormat> RES_FLYFRMFMT(RES_FMT_BEGIN + 2);
inline constexpr TypedWhichId<SwTextFormatColl> RES_TXTFMTCOLL(RES_FMT_BEGIN + 3);
inline constexpr TypedWhichId<SwGrfFormatColl> RES_GRFFMTCOLL(RES_FMT_BEGIN + 4);
inline constexpr TypedWhichId<SwDrawFrameFormat> RES_DRAWFRMFMT(RES_FMT_BEGIN + 5);
inline constexpr TypedWhichId<SwConditionTextFormatColl> RES_CONDTXTFMTCOLL(RES_FMT_BEGIN + 6);
inline constexpr sal_uInt16 RES_FMT_END(RES_FMT_BEGIN + 7);

// ID's for Messages in the Formats
inline constexpr sal_uInt16 RES_FORMAT_MSG_BEGIN(RES_FMT_END);
inline constexpr sal_uInt16 RES_UPDATEATTR_FMT_CHG(
    RES_FORMAT_MSG_BEGIN + 0); // used by SwUpdateAttr just as an ID to communicate what has changed
inline constexpr sal_uInt16 RES_UPDATEATTR_ATTRSET_CHG(
    RES_FORMAT_MSG_BEGIN + 1); // used by SwUpdateAttr just as an ID to communicate what has changed
inline constexpr sal_uInt16 RES_UPDATEATTR_OBJECTDYING(
    RES_FORMAT_MSG_BEGIN + 2); // used by SwUpdateAttr just as an ID to communicate what has changed
// empty
inline constexpr sal_uInt16 RES_FORMAT_MSG_END(RES_FORMAT_MSG_BEGIN + 3);

// An ID for the RTF-reader. The stylesheets are treated like attributes,
// i.e. there is a StyleSheet-attribute. To avoid collision with other
// Which()-values, the value is listed here. (The help system too defines
// new attributes!)
inline constexpr sal_uInt16 RES_FLTRATTR_BEGIN(RES_FORMAT_MSG_END);
inline constexpr TypedWhichId<SfxStringItem> RES_FLTR_BOOKMARK(RES_FLTRATTR_BEGIN + 0);
inline constexpr TypedWhichId<SwFltAnchor> RES_FLTR_ANCHOR(RES_FLTRATTR_BEGIN + 1);
inline constexpr TypedWhichId<SfxStringItem> RES_FLTR_NUMRULE(RES_FLTRATTR_BEGIN + 2);
inline constexpr TypedWhichId<SwFltTOX> RES_FLTR_TOX(RES_FLTRATTR_BEGIN + 3);
inline constexpr TypedWhichId<SwFltRedline> RES_FLTR_REDLINE(RES_FLTRATTR_BEGIN + 4);
inline constexpr TypedWhichId<CntUInt16Item> RES_FLTR_ANNOTATIONMARK(RES_FLTRATTR_BEGIN + 5);
inline constexpr TypedWhichId<SwFltRDFMark> RES_FLTR_RDFMARK(RES_FLTRATTR_BEGIN + 6);
inline constexpr sal_uInt16 RES_FLTRATTR_END(RES_FLTRATTR_BEGIN + 7);

inline constexpr sal_uInt16 RES_TBX_DUMMY(RES_FLTRATTR_END + 1);

inline constexpr TypedWhichId<SfxStringItem> HINT_END(RES_TBX_DUMMY);

// Error recognition!!
inline constexpr sal_uInt16 INVALID_HINT(HINT_END);
inline constexpr sal_uInt16 RES_WHICHHINT_END(HINT_END);

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
    return (RES_TXTATR_WITHEND_BEGIN <= nWhich) && (RES_TXTATR_WITHEND_END > nWhich);
}
inline bool isTXTATR_NOEND(const sal_uInt16 nWhich)
{
    return (RES_TXTATR_NOEND_BEGIN <= nWhich) && (RES_TXTATR_NOEND_END > nWhich);
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
inline bool isFormatMessage(const sal_uInt16 nWhich)
{
    return (RES_FORMAT_MSG_BEGIN <= nWhich) && (RES_FORMAT_MSG_END > nWhich);
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

/** Get the default attribute from corresponding default attribute table.

    @param[in] nWhich Position in table
    @return Attribute if found, null pointer otherwise
*/
SW_DLLPUBLIC const SfxPoolItem* GetDfltAttr(sal_uInt16 nWhich);

template <class T> inline const T* GetDfltAttr(TypedWhichId<T> nWhich)
{
    return static_cast<const T*>(GetDfltAttr(sal_uInt16(nWhich)));
}

SW_DLLPUBLIC sal_uInt16 GetWhichOfScript(sal_uInt16 nWhich, sal_uInt16 nScript);

template <class T>
inline TypedWhichId<T> GetWhichOfScript(TypedWhichId<T> nWhich, sal_uInt16 nScript)
{
    return TypedWhichId<T>(GetWhichOfScript(sal_uInt16(nWhich), nScript));
}

// return for the given TextAttribute without an end the correct character.
// This function returns
//      CH_TXTATR_BREAKWORD for Textattribute which breaks a word (default)
//      CH_TXTATR_INWORD    for Textattribute which doesn't breaks a word
class SwTextAttr;
sal_Unicode GetCharOfTextAttr(const SwTextAttr& rAttr);

// all Sets defined in init.cxx

// AttrSet-Range for the 3 Break-Attribute
extern WhichRangesContainer const aBreakSetRange;
// AttrSet-Range for TextFormatColl
extern WhichRangesContainer const aTextFormatCollSetRange;
// AttrSet-Range for GrfFormatColl
extern WhichRangesContainer const aGrfFormatCollSetRange;
// AttrSet-Range for TextNode
SW_DLLPUBLIC extern WhichRangesContainer const aTextNodeSetRange;
// AttrSet-Range for NoTextNode
extern WhichRangesContainer const aNoTextNodeSetRange;
// AttrSet-Range for SwTable
extern WhichRangesContainer const aTableSetRange;
// AttrSet-Range for SwTableLine
extern WhichRangesContainer const aTableLineSetRange;
// AttrSet-Range for SwTableBox
extern WhichRangesContainer const aTableBoxSetRange;
// AttrSet-Range for SwFrameFormat
SW_DLLPUBLIC extern WhichRangesContainer const aFrameFormatSetRange;
// AttrSet-Range for SwCharFormat
extern WhichRangesContainer const aCharFormatSetRange;
// AttrSet-Range for the autostyles
extern WhichRangesContainer const aCharAutoFormatSetRange;
// AttrSet-Range for SwPageDescFormat
extern WhichRangesContainer const aPgFrameFormatSetRange;

// check if ID is InRange of AttrSet-Ids
bool IsInRange(const WhichRangesContainer& pRange, const sal_uInt16 nId);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
