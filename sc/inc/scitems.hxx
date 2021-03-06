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
class ScPatternAttr;
class SvxPageItem;
class ScViewObjectModeItem;
class SfxUInt16Item;
class ScPageHFItem;
class ScPageScaleToItem;
class SvxSetItem;

//  EditEngine is not allowed to define its own ITEMID's
#define INCLUDED_EDITENG_EEITEMID_HXX

// Item-IDs for UI-MsgPool:

//! shall be moved to the below-1000 range!

#define MSGPOOL_START               1100

#define SCITEM_STRING               1100
#define SCITEM_SEARCHDATA           1101
#define SCITEM_SORTDATA             1102
#define SCITEM_QUERYDATA            1103
#define SCITEM_SUBTDATA             1104
#define SCITEM_CONSOLIDATEDATA      1105
#define SCITEM_PIVOTDATA            1106
#define SCITEM_SOLVEDATA            1107
#define SCITEM_USERLIST             1108
#define SCITEM_CONDFORMATDLGDATA    1109

#define MSGPOOL_END                 1109

// Item-IDs for attributes:

constexpr sal_uInt16 ATTR_STARTINDEX(100);     // begin of attributes

constexpr sal_uInt16 ATTR_PATTERN_START(100);     // begin of cell-attribute-pattern

constexpr TypedWhichId<SvxFontItem> ATTR_FONT (100);     // begin of cell-attributes
constexpr TypedWhichId<SvxFontHeightItem> ATTR_FONT_HEIGHT (101);
constexpr TypedWhichId<SvxWeightItem> ATTR_FONT_WEIGHT (102);
constexpr TypedWhichId<SvxPostureItem> ATTR_FONT_POSTURE (103);
constexpr TypedWhichId<SvxUnderlineItem> ATTR_FONT_UNDERLINE (104);
constexpr TypedWhichId<SvxOverlineItem> ATTR_FONT_OVERLINE (105);
constexpr TypedWhichId<SvxCrossedOutItem> ATTR_FONT_CROSSEDOUT (106);
constexpr TypedWhichId<SvxContourItem> ATTR_FONT_CONTOUR (107);
constexpr TypedWhichId<SvxShadowedItem> ATTR_FONT_SHADOWED (108);
constexpr TypedWhichId<SvxColorItem> ATTR_FONT_COLOR (109);
constexpr TypedWhichId<SvxLanguageItem> ATTR_FONT_LANGUAGE (110);
constexpr TypedWhichId<SvxFontItem> ATTR_CJK_FONT (111);
constexpr TypedWhichId<SvxFontHeightItem> ATTR_CJK_FONT_HEIGHT (112);
constexpr TypedWhichId<SvxWeightItem> ATTR_CJK_FONT_WEIGHT (113);
constexpr TypedWhichId<SvxPostureItem> ATTR_CJK_FONT_POSTURE (114);
constexpr TypedWhichId<SvxLanguageItem> ATTR_CJK_FONT_LANGUAGE (115);
constexpr TypedWhichId<SvxFontItem> ATTR_CTL_FONT (116);
constexpr TypedWhichId<SvxFontHeightItem> ATTR_CTL_FONT_HEIGHT (117);
constexpr TypedWhichId<SvxWeightItem> ATTR_CTL_FONT_WEIGHT (118);
constexpr TypedWhichId<SvxPostureItem> ATTR_CTL_FONT_POSTURE (119);
constexpr TypedWhichId<SvxLanguageItem> ATTR_CTL_FONT_LANGUAGE (120);
constexpr TypedWhichId<SvxEmphasisMarkItem> ATTR_FONT_EMPHASISMARK (121);
constexpr TypedWhichId<SvXMLAttrContainerItem> ATTR_USERDEF (122);    // not saved in binary files
constexpr TypedWhichId<SvxWordLineModeItem> ATTR_FONT_WORDLINE (123);
constexpr TypedWhichId<SvxCharReliefItem> ATTR_FONT_RELIEF (124);
constexpr TypedWhichId<ScHyphenateCell> ATTR_HYPHENATE (125);
constexpr TypedWhichId<SvxScriptSpaceItem> ATTR_SCRIPTSPACE (126);
constexpr TypedWhichId<SvxHangingPunctuationItem> ATTR_HANGPUNCTUATION (127);
constexpr TypedWhichId<SvxForbiddenRuleItem> ATTR_FORBIDDEN_RULES (128);
constexpr TypedWhichId<SvxHorJustifyItem> ATTR_HOR_JUSTIFY (129);
constexpr TypedWhichId<SvxJustifyMethodItem> ATTR_HOR_JUSTIFY_METHOD (130);
constexpr TypedWhichId<ScIndentItem> ATTR_INDENT (131);
constexpr TypedWhichId<SvxVerJustifyItem> ATTR_VER_JUSTIFY (132);
constexpr TypedWhichId<SvxJustifyMethodItem> ATTR_VER_JUSTIFY_METHOD (133);
constexpr TypedWhichId<ScVerticalStackCell> ATTR_STACKED (134);
constexpr TypedWhichId<ScRotateValueItem> ATTR_ROTATE_VALUE (135);
constexpr TypedWhichId<SvxRotateModeItem> ATTR_ROTATE_MODE (136);
constexpr TypedWhichId<SfxBoolItem> ATTR_VERTICAL_ASIAN (137);
constexpr TypedWhichId<SvxFrameDirectionItem> ATTR_WRITINGDIR (138);
constexpr TypedWhichId<ScLineBreakCell> ATTR_LINEBREAK (139);
constexpr TypedWhichId<ScShrinkToFitCell> ATTR_SHRINKTOFIT (140);
constexpr TypedWhichId<SvxLineItem> ATTR_BORDER_TLBR (141);
constexpr TypedWhichId<SvxLineItem> ATTR_BORDER_BLTR (142);
constexpr TypedWhichId<SvxMarginItem> ATTR_MARGIN (143);
constexpr TypedWhichId<ScMergeAttr> ATTR_MERGE (144);
constexpr TypedWhichId<ScMergeFlagAttr> ATTR_MERGE_FLAG (145);
constexpr TypedWhichId<SfxUInt32Item> ATTR_VALUE_FORMAT (146);
constexpr TypedWhichId<SvxLanguageItem> ATTR_LANGUAGE_FORMAT (147);
constexpr TypedWhichId<SvxBrushItem> ATTR_BACKGROUND (148);
constexpr TypedWhichId<ScProtectionAttr> ATTR_PROTECTION (149);
constexpr TypedWhichId<SvxBoxItem> ATTR_BORDER (150);
constexpr TypedWhichId<SvxBoxInfoItem> ATTR_BORDER_INNER (151);     // inside, because of template-EditDialog
constexpr TypedWhichId<SvxShadowItem> ATTR_SHADOW (152);
constexpr TypedWhichId<SfxUInt32Item> ATTR_VALIDDATA (153);
constexpr TypedWhichId<ScCondFormatItem> ATTR_CONDITIONAL (154);
constexpr TypedWhichId<SfxStringItem> ATTR_HYPERLINK (155);

constexpr sal_uInt16 ATTR_PATTERN_END(155);     // end cell-attribute-pattern

constexpr TypedWhichId<ScPatternAttr> ATTR_PATTERN (156);
                                        // page attributes
constexpr TypedWhichId<SvxLRSpaceItem> ATTR_LRSPACE (157);    // editor: PageDesc-TabPage
constexpr TypedWhichId<SvxULSpaceItem> ATTR_ULSPACE (158);
constexpr TypedWhichId<SvxPageItem> ATTR_PAGE (159);
constexpr TypedWhichId<SvxPaperBinItem> ATTR_PAGE_PAPERBIN (160);
constexpr TypedWhichId<SvxSizeItem> ATTR_PAGE_SIZE (161);
constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_HORCENTER (162);
constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_VERCENTER (163);

constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_ON (164);     // editor: header/footer-page
constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_DYNAMIC (165);
constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_SHARED (166);

constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_NOTES (167);     // editor: table
constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_GRID (168);
constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_HEADERS (169);
constexpr TypedWhichId<ScViewObjectModeItem> ATTR_PAGE_CHARTS (170);
constexpr TypedWhichId<ScViewObjectModeItem> ATTR_PAGE_OBJECTS (171);
constexpr TypedWhichId<ScViewObjectModeItem> ATTR_PAGE_DRAWINGS (172);
constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_TOPDOWN (173);
constexpr TypedWhichId<SfxUInt16Item> ATTR_PAGE_SCALE (174);
constexpr TypedWhichId<SfxUInt16Item> ATTR_PAGE_SCALETOPAGES (175);
constexpr TypedWhichId<SfxUInt16Item> ATTR_PAGE_FIRSTPAGENO (176);

constexpr TypedWhichId<ScPageHFItem> ATTR_PAGE_HEADERLEFT (177);     // contents of header/
constexpr TypedWhichId<ScPageHFItem> ATTR_PAGE_FOOTERLEFT (178);     // footer (left)
constexpr TypedWhichId<ScPageHFItem> ATTR_PAGE_HEADERRIGHT (179);     // contents of header/
constexpr TypedWhichId<ScPageHFItem> ATTR_PAGE_FOOTERRIGHT (180);     // footer (right)
constexpr TypedWhichId<SvxSetItem> ATTR_PAGE_HEADERSET (181);     // the corresponding sets
constexpr TypedWhichId<SvxSetItem> ATTR_PAGE_FOOTERSET (182);

constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_FORMULAS (183);
constexpr TypedWhichId<SfxBoolItem> ATTR_PAGE_NULLVALS (184);

constexpr TypedWhichId<ScPageScaleToItem> ATTR_PAGE_SCALETO (185);     // #i8868# scale printout to width/height

constexpr TypedWhichId<SfxBoolItem> ATTR_HIDDEN (186);

constexpr sal_uInt16 ATTR_ENDINDEX(ATTR_HIDDEN);        // end of pool-range

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
#define SID_SCATTR_PAGE_FORMULAS        ATTR_PAGE_FORMULAS
#define SID_SCATTR_PAGE_NULLVALS        ATTR_PAGE_NULLVALS
#define SID_SCATTR_PAGE_SCALETO         ATTR_PAGE_SCALETO

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
