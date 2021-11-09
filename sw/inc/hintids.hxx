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
class SwAttrSetChg;
class SwDocPosUpdate;
class SwFormatMeta;
class SvXMLAttrContainerItem;
class SwMsgPoolItem;
class SwPtrMsgPoolItem;
class SfxBoolItem;
class SvxColorItem;
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
class SwInsText;
class SwDelChr;
class SwDelText;
class SwRefMarkFieldUpdate;
class SwTableFormulaUpdate;
class SwAutoFormatGetDocNode;
class SwVirtPageNumInfo;
class SwFindNearestNode;
class SwStringMsgPoolItem;
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
inline constexpr OUStringLiteral CH_TXT_ATR_SUBST_FIELDSTART = u"[";
inline constexpr OUStringLiteral CH_TXT_ATR_SUBST_FIELDEND = u"]";

// a non-visible dummy character to track deleted tables,
// table rows, and images anchored to characters
#define CH_TXT_TRACKED_DUMMY_CHAR u'\x200D'

/*
 * Enums for the hints
 */

constexpr sal_uInt16 HINT_BEGIN = 1;

constexpr sal_uInt16 POOLATTR_BEGIN(HINT_BEGIN);

// Ranges for the IDs of the format-attributes.
// Which-values for character-format attributes.
constexpr sal_uInt16 RES_CHRATR_BEGIN(HINT_BEGIN);
constexpr TypedWhichId<SvxCaseMapItem> RES_CHRATR_CASEMAP(RES_CHRATR_BEGIN); // 1
constexpr TypedWhichId<SvxColorItem> RES_CHRATR_CHARSETCOLOR(2);
constexpr TypedWhichId<SvxColorItem> RES_CHRATR_COLOR(3);
constexpr TypedWhichId<SvxContourItem> RES_CHRATR_CONTOUR(4);
constexpr TypedWhichId<SvxCrossedOutItem> RES_CHRATR_CROSSEDOUT(5);
constexpr TypedWhichId<SvxEscapementItem> RES_CHRATR_ESCAPEMENT(6);
constexpr TypedWhichId<SvxFontItem> RES_CHRATR_FONT(7);
constexpr TypedWhichId<SvxFontHeightItem> RES_CHRATR_FONTSIZE(8);
constexpr TypedWhichId<SvxKerningItem> RES_CHRATR_KERNING(9);
constexpr TypedWhichId<SvxLanguageItem> RES_CHRATR_LANGUAGE(10);
constexpr TypedWhichId<SvxPostureItem> RES_CHRATR_POSTURE(11);
constexpr TypedWhichId<SfxVoidItem> RES_CHRATR_UNUSED1(12);
constexpr TypedWhichId<SvxShadowedItem> RES_CHRATR_SHADOWED(13);
constexpr TypedWhichId<SvxUnderlineItem> RES_CHRATR_UNDERLINE(14);
constexpr TypedWhichId<SvxWeightItem> RES_CHRATR_WEIGHT(15);
constexpr TypedWhichId<SvxWordLineModeItem> RES_CHRATR_WORDLINEMODE(16);
constexpr TypedWhichId<SvxAutoKernItem> RES_CHRATR_AUTOKERN(17);
constexpr TypedWhichId<SvxBlinkItem> RES_CHRATR_BLINK(18);
constexpr TypedWhichId<SvxNoHyphenItem> RES_CHRATR_NOHYPHEN(19);
constexpr TypedWhichId<SfxVoidItem> RES_CHRATR_UNUSED2(20);
constexpr TypedWhichId<SvxBrushItem> RES_CHRATR_BACKGROUND(21);
constexpr TypedWhichId<SvxFontItem> RES_CHRATR_CJK_FONT(22);
constexpr TypedWhichId<SvxFontHeightItem> RES_CHRATR_CJK_FONTSIZE(23);
constexpr TypedWhichId<SvxLanguageItem> RES_CHRATR_CJK_LANGUAGE(24);
constexpr TypedWhichId<SvxPostureItem> RES_CHRATR_CJK_POSTURE(25);
constexpr TypedWhichId<SvxWeightItem> RES_CHRATR_CJK_WEIGHT(26);
constexpr TypedWhichId<SvxFontItem> RES_CHRATR_CTL_FONT(27);
constexpr TypedWhichId<SvxFontHeightItem> RES_CHRATR_CTL_FONTSIZE(28);
constexpr TypedWhichId<SvxLanguageItem> RES_CHRATR_CTL_LANGUAGE(29);
constexpr TypedWhichId<SvxPostureItem> RES_CHRATR_CTL_POSTURE(30);
constexpr TypedWhichId<SvxWeightItem> RES_CHRATR_CTL_WEIGHT(31);
constexpr TypedWhichId<SvxCharRotateItem> RES_CHRATR_ROTATE(32);
constexpr TypedWhichId<SvxEmphasisMarkItem> RES_CHRATR_EMPHASIS_MARK(33);
constexpr TypedWhichId<SvxTwoLinesItem> RES_CHRATR_TWO_LINES(34);
constexpr TypedWhichId<SvxCharScaleWidthItem> RES_CHRATR_SCALEW(35);
constexpr TypedWhichId<SvxCharReliefItem> RES_CHRATR_RELIEF(36);
constexpr TypedWhichId<SvxCharHiddenItem> RES_CHRATR_HIDDEN(37);
constexpr TypedWhichId<SvxOverlineItem> RES_CHRATR_OVERLINE(38);
constexpr TypedWhichId<SvxRsidItem> RES_CHRATR_RSID(39);
constexpr TypedWhichId<SvxBoxItem> RES_CHRATR_BOX(40);
constexpr TypedWhichId<SvxShadowItem> RES_CHRATR_SHADOW(41);
constexpr TypedWhichId<SvxBrushItem> RES_CHRATR_HIGHLIGHT(42);
constexpr TypedWhichId<SfxGrabBagItem> RES_CHRATR_GRABBAG(43);
constexpr TypedWhichId<SfxInt16Item> RES_CHRATR_BIDIRTL(44);
constexpr TypedWhichId<SfxInt16Item> RES_CHRATR_IDCTHINT(45);
constexpr sal_uInt16 RES_CHRATR_END(46);

// this Attribute used only in a TextNodes SwpAttr-Array
constexpr sal_uInt16 RES_TXTATR_BEGIN(RES_CHRATR_END);

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
constexpr sal_uInt16 RES_TXTATR_WITHEND_BEGIN(RES_TXTATR_BEGIN);
constexpr TypedWhichId<SwFormatRefMark> RES_TXTATR_REFMARK(RES_TXTATR_WITHEND_BEGIN); // 46
constexpr TypedWhichId<SwTOXMark> RES_TXTATR_TOXMARK(47);
constexpr TypedWhichId<SwFormatMeta> RES_TXTATR_META(48);
constexpr TypedWhichId<SwFormatMeta> RES_TXTATR_METAFIELD(49);
constexpr TypedWhichId<SwFormatAutoFormat> RES_TXTATR_AUTOFMT(50);
constexpr TypedWhichId<SwFormatINetFormat> RES_TXTATR_INETFMT(51);
constexpr TypedWhichId<SwFormatCharFormat> RES_TXTATR_CHARFMT(52);
constexpr TypedWhichId<SwFormatRuby> RES_TXTATR_CJK_RUBY(53);
constexpr TypedWhichId<SvXMLAttrContainerItem> RES_TXTATR_UNKNOWN_CONTAINER(54);
constexpr TypedWhichId<SwFormatField> RES_TXTATR_INPUTFIELD(55);
constexpr sal_uInt16 RES_TXTATR_WITHEND_END(56);

// all TextAttributes without an end
constexpr sal_uInt16 RES_TXTATR_NOEND_BEGIN(RES_TXTATR_WITHEND_END);
constexpr TypedWhichId<SwFormatField> RES_TXTATR_FIELD(RES_TXTATR_NOEND_BEGIN); // 56
constexpr TypedWhichId<SwFormatFlyCnt> RES_TXTATR_FLYCNT(57);
constexpr TypedWhichId<SwFormatFootnote> RES_TXTATR_FTN(58);
constexpr TypedWhichId<SwFormatField> RES_TXTATR_ANNOTATION(59);
constexpr TypedWhichId<SfxBoolItem> RES_TXTATR_DUMMY3(60);
constexpr TypedWhichId<SfxBoolItem> RES_TXTATR_DUMMY1(61);
constexpr TypedWhichId<SfxBoolItem> RES_TXTATR_DUMMY2(62);
constexpr sal_uInt16 RES_TXTATR_NOEND_END(63);
constexpr sal_uInt16 RES_TXTATR_END(RES_TXTATR_NOEND_END);

constexpr sal_uInt16 RES_PARATR_BEGIN(RES_TXTATR_END);
constexpr TypedWhichId<SvxLineSpacingItem> RES_PARATR_LINESPACING(RES_PARATR_BEGIN); // 63
constexpr TypedWhichId<SvxAdjustItem> RES_PARATR_ADJUST(64);
constexpr TypedWhichId<SvxFormatSplitItem> RES_PARATR_SPLIT(65);
constexpr TypedWhichId<SvxOrphansItem> RES_PARATR_ORPHANS(66);
constexpr TypedWhichId<SvxWidowsItem> RES_PARATR_WIDOWS(67);
constexpr TypedWhichId<SvxTabStopItem> RES_PARATR_TABSTOP(68);
constexpr TypedWhichId<SvxHyphenZoneItem> RES_PARATR_HYPHENZONE(69);
constexpr TypedWhichId<SwFormatDrop> RES_PARATR_DROP(70);
constexpr TypedWhichId<SwRegisterItem> RES_PARATR_REGISTER(71);
constexpr TypedWhichId<SwNumRuleItem> RES_PARATR_NUMRULE(72);
constexpr TypedWhichId<SvxScriptSpaceItem> RES_PARATR_SCRIPTSPACE(73);
constexpr TypedWhichId<SvxHangingPunctuationItem> RES_PARATR_HANGINGPUNCTUATION(74);
constexpr TypedWhichId<SvxForbiddenRuleItem> RES_PARATR_FORBIDDEN_RULES(75);
constexpr TypedWhichId<SvxParaVertAlignItem> RES_PARATR_VERTALIGN(76);
constexpr TypedWhichId<SvxParaGridItem> RES_PARATR_SNAPTOGRID(77);
constexpr TypedWhichId<SwParaConnectBorderItem> RES_PARATR_CONNECT_BORDER(78);
constexpr TypedWhichId<SfxUInt16Item> RES_PARATR_OUTLINELEVEL(79);
constexpr TypedWhichId<SvxRsidItem> RES_PARATR_RSID(80);
constexpr TypedWhichId<SfxGrabBagItem> RES_PARATR_GRABBAG(81);
constexpr sal_uInt16 RES_PARATR_END(82);

// list attributes for paragraphs.
// intentionally these list attributes are not contained in paragraph styles
constexpr sal_uInt16 RES_PARATR_LIST_BEGIN(RES_PARATR_END);
constexpr TypedWhichId<SfxStringItem> RES_PARATR_LIST_ID(RES_PARATR_LIST_BEGIN); // 82
constexpr TypedWhichId<SfxInt16Item> RES_PARATR_LIST_LEVEL(83);
constexpr TypedWhichId<SfxBoolItem> RES_PARATR_LIST_ISRESTART(84);
constexpr TypedWhichId<SfxInt16Item> RES_PARATR_LIST_RESTARTVALUE(85);
constexpr TypedWhichId<SfxBoolItem> RES_PARATR_LIST_ISCOUNTED(86);
constexpr TypedWhichId<SwFormatAutoFormat>
    RES_PARATR_LIST_AUTOFMT(87); //TypedWhichId<SfxSetItem>(87)
constexpr sal_uInt16 RES_PARATR_LIST_END(88);

constexpr sal_uInt16 RES_FRMATR_BEGIN(RES_PARATR_LIST_END);
constexpr TypedWhichId<SwFormatFillOrder> RES_FILL_ORDER(RES_FRMATR_BEGIN);
constexpr TypedWhichId<SwFormatFrameSize> RES_FRM_SIZE(89);
constexpr TypedWhichId<SvxPaperBinItem> RES_PAPER_BIN(90);
constexpr TypedWhichId<SvxLRSpaceItem> RES_LR_SPACE(91);
constexpr TypedWhichId<SvxULSpaceItem> RES_UL_SPACE(92);
constexpr TypedWhichId<SwFormatPageDesc> RES_PAGEDESC(93);
constexpr TypedWhichId<SvxFormatBreakItem> RES_BREAK(94);
constexpr TypedWhichId<SwFormatContent> RES_CNTNT(95);
constexpr TypedWhichId<SwFormatHeader> RES_HEADER(96);
constexpr TypedWhichId<SwFormatFooter> RES_FOOTER(97);
constexpr TypedWhichId<SvxPrintItem> RES_PRINT(98);
constexpr TypedWhichId<SvxOpaqueItem> RES_OPAQUE(99);
constexpr TypedWhichId<SvxProtectItem> RES_PROTECT(100);
constexpr TypedWhichId<SwFormatSurround> RES_SURROUND(101);
constexpr TypedWhichId<SwFormatVertOrient> RES_VERT_ORIENT(102);
constexpr TypedWhichId<SwFormatHoriOrient> RES_HORI_ORIENT(103);
constexpr TypedWhichId<SwFormatAnchor> RES_ANCHOR(104);
constexpr TypedWhichId<SvxBrushItem> RES_BACKGROUND(105);
constexpr TypedWhichId<SvxBoxItem> RES_BOX(106);
constexpr TypedWhichId<SvxShadowItem> RES_SHADOW(107);
constexpr TypedWhichId<SvxMacroItem> RES_FRMMACRO(108);
constexpr TypedWhichId<SwFormatCol> RES_COL(109);
constexpr TypedWhichId<SvxFormatKeepItem> RES_KEEP(110);
constexpr TypedWhichId<SwFormatURL> RES_URL(111);
constexpr TypedWhichId<SwFormatEditInReadonly> RES_EDIT_IN_READONLY(112);
constexpr TypedWhichId<SwFormatLayoutSplit> RES_LAYOUT_SPLIT(113);
constexpr TypedWhichId<SwFormatChain> RES_CHAIN(114);
constexpr TypedWhichId<SwTextGridItem> RES_TEXTGRID(115);
constexpr TypedWhichId<SwFormatLineNumber> RES_LINENUMBER(116);
constexpr TypedWhichId<SwFormatFootnoteAtTextEnd> RES_FTN_AT_TXTEND(117);
constexpr TypedWhichId<SwFormatEndAtTextEnd> RES_END_AT_TXTEND(118);
constexpr TypedWhichId<SwFormatNoBalancedColumns> RES_COLUMNBALANCE(119);
constexpr TypedWhichId<SvxFrameDirectionItem> RES_FRAMEDIR(120);
constexpr TypedWhichId<SwHeaderAndFooterEatSpacingItem> RES_HEADER_FOOTER_EAT_SPACING(121);
constexpr TypedWhichId<SwFormatRowSplit> RES_ROW_SPLIT(122);
constexpr TypedWhichId<SwFormatFollowTextFlow> RES_FOLLOW_TEXT_FLOW(123);
constexpr TypedWhichId<SfxBoolItem> RES_COLLAPSING_BORDERS(124);
constexpr TypedWhichId<SwFormatWrapInfluenceOnObjPos> RES_WRAP_INFLUENCE_ON_OBJPOS(125);
constexpr TypedWhichId<SwFormatAutoFormat> RES_AUTO_STYLE(126);
constexpr TypedWhichId<SfxStringItem> RES_FRMATR_STYLE_NAME(127);
constexpr TypedWhichId<SfxStringItem> RES_FRMATR_CONDITIONAL_STYLE_NAME(128);
constexpr TypedWhichId<SfxGrabBagItem> RES_FRMATR_GRABBAG(129);
constexpr TypedWhichId<SdrTextVertAdjustItem> RES_TEXT_VERT_ADJUST(130);
constexpr TypedWhichId<SfxBoolItem> RES_BACKGROUND_FULL_SIZE(131);
constexpr TypedWhichId<SfxBoolItem> RES_RTL_GUTTER(132);
constexpr sal_uInt16 RES_FRMATR_END(133);

constexpr sal_uInt16 RES_GRFATR_BEGIN(RES_FRMATR_END);
constexpr TypedWhichId<SwMirrorGrf> RES_GRFATR_MIRRORGRF(RES_GRFATR_BEGIN); // 133
constexpr TypedWhichId<SwCropGrf> RES_GRFATR_CROPGRF(134);

constexpr TypedWhichId<SwRotationGrf> RES_GRFATR_ROTATION(135);
constexpr TypedWhichId<SwLuminanceGrf> RES_GRFATR_LUMINANCE(136);
constexpr TypedWhichId<SwContrastGrf> RES_GRFATR_CONTRAST(137);
constexpr TypedWhichId<SwChannelRGrf> RES_GRFATR_CHANNELR(138);
constexpr TypedWhichId<SwChannelGGrf> RES_GRFATR_CHANNELG(139);
constexpr TypedWhichId<SwChannelBGrf> RES_GRFATR_CHANNELB(140);
constexpr TypedWhichId<SwGammaGrf> RES_GRFATR_GAMMA(141);
constexpr TypedWhichId<SwInvertGrf> RES_GRFATR_INVERT(142);
constexpr TypedWhichId<SwTransparencyGrf> RES_GRFATR_TRANSPARENCY(143);
constexpr TypedWhichId<SwDrawModeGrf> RES_GRFATR_DRAWMODE(144);

constexpr TypedWhichId<SfxBoolItem> RES_GRFATR_DUMMY1(145);
constexpr TypedWhichId<SfxBoolItem> RES_GRFATR_DUMMY2(146);
constexpr TypedWhichId<SfxBoolItem> RES_GRFATR_DUMMY3(147);
constexpr TypedWhichId<SfxBoolItem> RES_GRFATR_DUMMY4(148);
constexpr TypedWhichId<SfxBoolItem> RES_GRFATR_DUMMY5(149);
constexpr sal_uInt16 RES_GRFATR_END(150);

constexpr sal_uInt16 RES_BOXATR_BEGIN(RES_GRFATR_END);
constexpr TypedWhichId<SwTableBoxNumFormat> RES_BOXATR_FORMAT(RES_BOXATR_BEGIN); // 150
constexpr TypedWhichId<SwTableBoxFormula> RES_BOXATR_FORMULA(151);
constexpr TypedWhichId<SwTableBoxValue> RES_BOXATR_VALUE(152);
constexpr sal_uInt16 RES_BOXATR_END(153);

constexpr sal_uInt16 RES_UNKNOWNATR_BEGIN(RES_BOXATR_END);
constexpr TypedWhichId<SvXMLAttrContainerItem>
    RES_UNKNOWNATR_CONTAINER(RES_UNKNOWNATR_BEGIN); // 153
constexpr sal_uInt16 RES_UNKNOWNATR_END(154);

constexpr sal_uInt16 POOLATTR_END(RES_UNKNOWNATR_END);

// Format IDs
constexpr sal_uInt16 RES_FMT_BEGIN(RES_UNKNOWNATR_END);
constexpr TypedWhichId<SwCharFormat> RES_CHRFMT(RES_FMT_BEGIN); // 154
constexpr TypedWhichId<SwFrameFormat> RES_FRMFMT(155);
constexpr TypedWhichId<SwFlyFrameFormat> RES_FLYFRMFMT(156);
constexpr TypedWhichId<SwTextFormatColl> RES_TXTFMTCOLL(157);
constexpr TypedWhichId<SwGrfFormatColl> RES_GRFFMTCOLL(158);
constexpr TypedWhichId<SwDrawFrameFormat> RES_DRAWFRMFMT(159);
constexpr TypedWhichId<SwConditionTextFormatColl> RES_CONDTXTFMTCOLL(160);
constexpr sal_uInt16 RES_FMT_END(161);

// ID's for Messages in the Formats
constexpr sal_uInt16 RES_MSG_BEGIN(RES_FMT_END);
constexpr TypedWhichId<SwPtrMsgPoolItem> RES_OBJECTDYING(RES_MSG_BEGIN); // 161
constexpr TypedWhichId<SwFormatChg> RES_FMT_CHG(162);
constexpr TypedWhichId<SwAttrSetChg> RES_ATTRSET_CHG(163);
constexpr TypedWhichId<SwInsText> RES_INS_TXT(164);
constexpr TypedWhichId<SwDelChr> RES_DEL_CHR(165);
constexpr TypedWhichId<SwDelText> RES_DEL_TXT(166);
constexpr TypedWhichId<SwUpdateAttr> RES_UPDATE_ATTR(167);
constexpr TypedWhichId<SwRefMarkFieldUpdate> RES_REFMARKFLD_UPDATE(168);
constexpr TypedWhichId<SwDocPosUpdate> RES_DOCPOS_UPDATE(169);
constexpr TypedWhichId<SwTableFormulaUpdate> RES_TABLEFML_UPDATE(170);
constexpr TypedWhichId<SwMsgPoolItem> RES_UPDATEDDETBL(171);
constexpr TypedWhichId<SwMsgPoolItem> RES_TBLHEADLINECHG(172);
constexpr TypedWhichId<SwAutoFormatGetDocNode> RES_AUTOFMT_DOCNODE(173);
constexpr TypedWhichId<SwMsgPoolItem> RES_SECTION_HIDDEN(174);
constexpr TypedWhichId<SwMsgPoolItem> RES_SECTION_NOT_HIDDEN(175);
constexpr TypedWhichId<SwMsgPoolItem> RES_GRAPHIC_PIECE_ARRIVED(177);
constexpr TypedWhichId<SwMsgPoolItem> RES_HIDDENPARA_PRINT(178);
constexpr TypedWhichId<SwVirtPageNumInfo> RES_VIRTPAGENUM_INFO(180);
constexpr TypedWhichId<SwPtrMsgPoolItem> RES_REMOVE_UNO_OBJECT(181);
// empty
constexpr TypedWhichId<SwFindNearestNode> RES_FINDNEARESTNODE(184);
constexpr TypedWhichId<SwPtrMsgPoolItem> RES_CONTENT_VISIBLE(185);
constexpr TypedWhichId<SwMsgPoolItem> RES_GRAPHIC_SWAPIN(186);
constexpr TypedWhichId<SwStringMsgPoolItem> RES_NAME_CHANGED(187);
constexpr TypedWhichId<SwStringMsgPoolItem> RES_TITLE_CHANGED(188);
constexpr TypedWhichId<SwStringMsgPoolItem> RES_DESCRIPTION_CHANGED(189);
constexpr TypedWhichId<SwMsgPoolItem> RES_LINKED_GRAPHIC_STREAM_ARRIVED(189);
constexpr sal_uInt16 RES_MSG_END(190);

// An ID for the RTF-reader. The stylesheets are treated like attributes,
// i.e. there is a StyleSheet-attribute. To avoid collision with other
// Which()-values, the value is listed here. (The help system too defines
// new attributes!)
constexpr sal_uInt16 RES_FLTRATTR_BEGIN(RES_MSG_END);
constexpr TypedWhichId<SfxStringItem> RES_FLTR_BOOKMARK(RES_FLTRATTR_BEGIN);
constexpr TypedWhichId<SwFltAnchor> RES_FLTR_ANCHOR(191);
constexpr TypedWhichId<SfxStringItem> RES_FLTR_NUMRULE(192);
constexpr TypedWhichId<SwFltTOX> RES_FLTR_TOX(193);
constexpr TypedWhichId<SwFltRedline> RES_FLTR_REDLINE(194);
constexpr TypedWhichId<CntUInt16Item> RES_FLTR_ANNOTATIONMARK(195);
constexpr TypedWhichId<SwFltRDFMark> RES_FLTR_RDFMARK(196);
constexpr sal_uInt16 RES_FLTRATTR_END(197);

constexpr sal_uInt16 RES_TBX_DUMMY(RES_FLTRATTR_END + 1);

constexpr sal_uInt16 HINT_END(RES_TBX_DUMMY);

// Error recognition!!
constexpr sal_uInt16 INVALID_HINT(HINT_END);
constexpr sal_uInt16 RES_WHICHHINT_END(HINT_END);

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
extern SfxItemInfo aSlotTab[];

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
