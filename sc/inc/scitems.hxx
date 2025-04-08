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

#pragma once

#include <svl/typedwhich.hxx>

class ScConsolidateItem;
class SfxBoolItem;
class SvxCharReliefItem;
class SvxColorItem;
class SvxContourItem;
class SvxCrossedOutItem;
class SvxEmphasisMarkItem;
class SvxFontHeightItem;
class SvxFontItem;
class SvxForbiddenRuleItem;
class SvxFrameDirectionItem;
class SvxHangingPunctuationItem;
class SvxLanguageItem;
class SvxLRSpaceItem;
class SvxOverlineItem;
class SvxPostureItem;
class SvXMLAttrContainerItem;
class SvxScriptSpaceItem;
class SvxShadowedItem;
class SvxULSpaceItem;
class SvxUnderlineItem;
class SvxWeightItem;
class SvxWordLineModeItem;
class SvxShadowItem;
class SvxHorJustifyItem;
class SvxVerJustifyItem;
class SvxBrushItem;
class SvxBoxItem;
class ScCondFormatItem;
class ScLineBreakCell;
class ScRotateValueItem;
class ScVerticalStackCell;
class SvxLineItem;
class SvxRotateModeItem;
class SfxStringItem;
class SvxSizeItem;
class SvxJustifyMethodItem;
class SvxMarginItem;
class SvxPaperBinItem;
class ScHyphenateCell;
class SfxUInt32Item;
class ScIndentItem;
class ScShrinkToFitCell;
class ScMergeAttr;
class ScMergeFlagAttr;
class ScProtectionAttr;
class SvxBoxInfoItem;
class SvxPageItem;
class ScViewObjectModeItem;
class SfxUInt16Item;
class ScPageHFItem;
class ScPageScaleToItem;
class SvxSetItem;

// Item-IDs for UI-MsgPool:

//! shall be moved to the below-1000 range!

#define MSGPOOL_START               1100

#define SCITEM_STRING               TypedWhichId<SfxStringItem>(1100)
#define SCITEM_SEARCHDATA           TypedWhichId<SvxSearchItem>(1101)
#define SCITEM_SORTDATA             TypedWhichId<ScSortItem>(1102)
#define SCITEM_QUERYDATA            TypedWhichId<ScQueryItem>(1103)
#define SCITEM_SUBTDATA             TypedWhichId<ScSubTotalItem>(1104)
#define SCITEM_CONSOLIDATEDATA      TypedWhichId<ScConsolidateItem>(1105)
#define SCITEM_PIVOTDATA            TypedWhichId<ScPivotItem>(1106)
#define SCITEM_SOLVEDATA            TypedWhichId<ScSolveItem>(1107)
#define SCITEM_USERLIST             TypedWhichId<ScUserListItem>(1108)

#define MSGPOOL_END                 1108

// Item-IDs for attributes:

inline constexpr sal_uInt16 ATTR_STARTINDEX(100);     // begin of attributes

inline constexpr sal_uInt16 ATTR_PATTERN_START(100);     // begin of cell-attribute-pattern

inline constexpr TypedWhichId<SvxFontItem> ATTR_FONT (100);     // begin of cell-attributes
inline constexpr TypedWhichId<SvxFontHeightItem> ATTR_FONT_HEIGHT (101);
inline constexpr TypedWhichId<SvxWeightItem> ATTR_FONT_WEIGHT (102);
inline constexpr TypedWhichId<SvxPostureItem> ATTR_FONT_POSTURE (103);
inline constexpr TypedWhichId<SvxUnderlineItem> ATTR_FONT_UNDERLINE (104);
inline constexpr TypedWhichId<SvxOverlineItem> ATTR_FONT_OVERLINE (105);
inline constexpr TypedWhichId<SvxCrossedOutItem> ATTR_FONT_CROSSEDOUT (106);
inline constexpr TypedWhichId<SvxContourItem> ATTR_FONT_CONTOUR (107);
inline constexpr TypedWhichId<SvxShadowedItem> ATTR_FONT_SHADOWED (108);
inline constexpr TypedWhichId<SvxColorItem> ATTR_FONT_COLOR (109);
inline constexpr TypedWhichId<SvxLanguageItem> ATTR_FONT_LANGUAGE (110);
inline constexpr TypedWhichId<SvxFontItem> ATTR_CJK_FONT (111);
inline constexpr TypedWhichId<SvxFontHeightItem> ATTR_CJK_FONT_HEIGHT (112);
inline constexpr TypedWhichId<SvxWeightItem> ATTR_CJK_FONT_WEIGHT (113);
inline constexpr TypedWhichId<SvxPostureItem> ATTR_CJK_FONT_POSTURE (114);
inline constexpr TypedWhichId<SvxLanguageItem> ATTR_CJK_FONT_LANGUAGE (115);
inline constexpr TypedWhichId<SvxFontItem> ATTR_CTL_FONT (116);
inline constexpr TypedWhichId<SvxFontHeightItem> ATTR_CTL_FONT_HEIGHT (117);
inline constexpr TypedWhichId<SvxWeightItem> ATTR_CTL_FONT_WEIGHT (118);
inline constexpr TypedWhichId<SvxPostureItem> ATTR_CTL_FONT_POSTURE (119);
inline constexpr TypedWhichId<SvxLanguageItem> ATTR_CTL_FONT_LANGUAGE (120);
inline constexpr TypedWhichId<SvxEmphasisMarkItem> ATTR_FONT_EMPHASISMARK (121);
inline constexpr TypedWhichId<SvXMLAttrContainerItem> ATTR_USERDEF (122);    // not saved in binary files
inline constexpr TypedWhichId<SvxWordLineModeItem> ATTR_FONT_WORDLINE (123);
inline constexpr TypedWhichId<SvxCharReliefItem> ATTR_FONT_RELIEF (124);
inline constexpr TypedWhichId<ScHyphenateCell> ATTR_HYPHENATE (125);
inline constexpr TypedWhichId<SvxScriptSpaceItem> ATTR_SCRIPTSPACE (126);
inline constexpr TypedWhichId<SvxHangingPunctuationItem> ATTR_HANGPUNCTUATION (127);
inline constexpr TypedWhichId<SvxForbiddenRuleItem> ATTR_FORBIDDEN_RULES (128);
inline constexpr TypedWhichId<SvxHorJustifyItem> ATTR_HOR_JUSTIFY (129);
inline constexpr TypedWhichId<SvxJustifyMethodItem> ATTR_HOR_JUSTIFY_METHOD (130);
inline constexpr TypedWhichId<ScIndentItem> ATTR_INDENT (131);
inline constexpr TypedWhichId<SvxVerJustifyItem> ATTR_VER_JUSTIFY (132);
inline constexpr TypedWhichId<SvxJustifyMethodItem> ATTR_VER_JUSTIFY_METHOD (133);
inline constexpr TypedWhichId<ScVerticalStackCell> ATTR_STACKED (134);
inline constexpr TypedWhichId<ScRotateValueItem> ATTR_ROTATE_VALUE (135);
inline constexpr TypedWhichId<SvxRotateModeItem> ATTR_ROTATE_MODE (136);
inline constexpr TypedWhichId<SfxBoolItem> ATTR_VERTICAL_ASIAN (137);
inline constexpr TypedWhichId<SvxFrameDirectionItem> ATTR_WRITINGDIR (138);
inline constexpr TypedWhichId<ScLineBreakCell> ATTR_LINEBREAK (139);
inline constexpr TypedWhichId<ScShrinkToFitCell> ATTR_SHRINKTOFIT (140);
inline constexpr TypedWhichId<SvxLineItem> ATTR_BORDER_TLBR (141);
inline constexpr TypedWhichId<SvxLineItem> ATTR_BORDER_BLTR (142);
inline constexpr TypedWhichId<SvxMarginItem> ATTR_MARGIN (143);
inline constexpr TypedWhichId<ScMergeAttr> ATTR_MERGE (144);
inline constexpr TypedWhichId<ScMergeFlagAttr> ATTR_MERGE_FLAG (145);
inline constexpr TypedWhichId<SfxUInt32Item> ATTR_VALUE_FORMAT (146);
inline constexpr TypedWhichId<SvxLanguageItem> ATTR_LANGUAGE_FORMAT (147);
inline constexpr TypedWhichId<SvxBrushItem> ATTR_BACKGROUND (148);
inline constexpr TypedWhichId<ScProtectionAttr> ATTR_PROTECTION (149);
inline constexpr TypedWhichId<SvxBoxItem> ATTR_BORDER (150);
inline constexpr TypedWhichId<SvxBoxInfoItem> ATTR_BORDER_INNER (151);     // inside, because of template-EditDialog
inline constexpr TypedWhichId<SvxShadowItem> ATTR_SHADOW (152);
inline constexpr TypedWhichId<SfxUInt32Item> ATTR_VALIDDATA (153);
inline constexpr TypedWhichId<ScCondFormatItem> ATTR_CONDITIONAL (154);
inline constexpr TypedWhichId<SfxStringItem> ATTR_HYPERLINK (155);

inline constexpr sal_uInt16 ATTR_PATTERN_END(155);     // end cell-attribute-pattern
                                        // page attributes
inline constexpr TypedWhichId<SvxLRSpaceItem> ATTR_LRSPACE (156);    // editor: PageDesc-TabPage
inline constexpr TypedWhichId<SvxULSpaceItem> ATTR_ULSPACE (157);
inline constexpr TypedWhichId<SvxPageItem> ATTR_PAGE (158);
inline constexpr TypedWhichId<SvxPaperBinItem> ATTR_PAGE_PAPERBIN (159);
inline constexpr TypedWhichId<SvxSizeItem> ATTR_PAGE_SIZE (160);
inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_HORCENTER (161);
inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_VERCENTER (162);

inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_ON (163);     // editor: header/footer-page
inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_DYNAMIC (164);
inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_SHARED (165);
inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_SHARED_FIRST (166);

inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_NOTES (167);     // editor: table
inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_GRID (168);
inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_HEADERS (169);
inline constexpr TypedWhichId<ScViewObjectModeItem> ATTR_PAGE_CHARTS (170);
inline constexpr TypedWhichId<ScViewObjectModeItem> ATTR_PAGE_OBJECTS (171);
inline constexpr TypedWhichId<ScViewObjectModeItem> ATTR_PAGE_DRAWINGS (172);
inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_TOPDOWN (173);
inline constexpr TypedWhichId<SfxUInt16Item> ATTR_PAGE_SCALE (174);
inline constexpr TypedWhichId<SfxUInt16Item> ATTR_PAGE_SCALETOPAGES (175);
inline constexpr TypedWhichId<SfxUInt16Item> ATTR_PAGE_FIRSTPAGENO (176);

inline constexpr TypedWhichId<ScPageHFItem> ATTR_PAGE_HEADERLEFT (177);     // contents of header/
inline constexpr TypedWhichId<ScPageHFItem> ATTR_PAGE_FOOTERLEFT (178);     // footer (left)
inline constexpr TypedWhichId<ScPageHFItem> ATTR_PAGE_HEADERRIGHT (179);    // contents of header/
inline constexpr TypedWhichId<ScPageHFItem> ATTR_PAGE_FOOTERRIGHT (180);    // footer (right)
inline constexpr TypedWhichId<ScPageHFItem> ATTR_PAGE_HEADERFIRST (181);    // contents of header/
inline constexpr TypedWhichId<ScPageHFItem> ATTR_PAGE_FOOTERFIRST (182);    // footer (first page)
inline constexpr TypedWhichId<SvxSetItem> ATTR_PAGE_HEADERSET (183);     // the corresponding sets
inline constexpr TypedWhichId<SvxSetItem> ATTR_PAGE_FOOTERSET (184);

inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_FORMULAS (185);
inline constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_NULLVALS (186);

inline constexpr TypedWhichId<ScPageScaleToItem> ATTR_PAGE_SCALETO (187);     // #i8868# scale printout to width/height

inline constexpr TypedWhichId<SfxBoolItem> ATTR_HIDDEN (188);

inline constexpr sal_uInt16 ATTR_ENDINDEX(ATTR_HIDDEN);        // end of pool-range

// Dummy Slot-IDs for dialogs

#define SID_SCATTR_PAGE_NOTES           ATTR_PAGE_NOTES
#define SID_SCATTR_PAGE_GRID            ATTR_PAGE_GRID
#define SID_SCATTR_PAGE_HEADERS         ATTR_PAGE_HEADERS
#define SID_SCATTR_PAGE_CHARTS          ATTR_PAGE_CHARTS
#define SID_SCATTR_PAGE_OBJECTS         ATTR_PAGE_OBJECTS
#define SID_SCATTR_PAGE_DRAWINGS        ATTR_PAGE_DRAWINGS
#define SID_SCATTR_PAGE_TOPDOWN         ATTR_PAGE_TOPDOWN
#define SID_SCATTR_PAGE_SCALE           ATTR_PAGE_SCALE
#define SID_SCATTR_PAGE_SCALETOPAGES    ATTR_PAGE_SCALETOPAGES
#define SID_SCATTR_PAGE_FIRSTPAGENO     ATTR_PAGE_FIRSTPAGENO
#define SID_SCATTR_PAGE_HEADERLEFT      ATTR_PAGE_HEADERLEFT
#define SID_SCATTR_PAGE_FOOTERLEFT      ATTR_PAGE_FOOTERLEFT
#define SID_SCATTR_PAGE_HEADERRIGHT     ATTR_PAGE_HEADERRIGHT
#define SID_SCATTR_PAGE_FOOTERRIGHT     ATTR_PAGE_FOOTERRIGHT
#define SID_SCATTR_PAGE_HEADERFIRST     ATTR_PAGE_HEADERFIRST
#define SID_SCATTR_PAGE_FOOTERFIRST     ATTR_PAGE_FOOTERFIRST
#define SID_SCATTR_PAGE_FORMULAS        ATTR_PAGE_FORMULAS
#define SID_SCATTR_PAGE_NULLVALS        ATTR_PAGE_NULLVALS
#define SID_SCATTR_PAGE_SCALETO         ATTR_PAGE_SCALETO

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
