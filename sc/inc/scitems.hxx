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

#ifndef INCLUDED_SC_INC_SCITEMS_HXX
#define INCLUDED_SC_INC_SCITEMS_HXX

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

#define ATTR_STARTINDEX         100     // begin of attributes

#define ATTR_PATTERN_START      100     // begin of cell-attribute-pattern

#define ATTR_FONT               TypedWhichId<SvxFontItem>(100)     // begin of cell-attributes
#define ATTR_FONT_HEIGHT        TypedWhichId<SvxFontHeightItem>(101)
#define ATTR_FONT_WEIGHT        TypedWhichId<SvxWeightItem>(102)
#define ATTR_FONT_POSTURE       TypedWhichId<SvxPostureItem>(103)
#define ATTR_FONT_UNDERLINE     TypedWhichId<SvxUnderlineItem>(104)
#define ATTR_FONT_OVERLINE      TypedWhichId<SvxOverlineItem>(105)
#define ATTR_FONT_CROSSEDOUT    TypedWhichId<SvxCrossedOutItem>(106)
#define ATTR_FONT_CONTOUR       TypedWhichId<SvxContourItem>(107)
#define ATTR_FONT_SHADOWED      TypedWhichId<SvxShadowedItem>(108)
#define ATTR_FONT_COLOR         TypedWhichId<SvxColorItem>(109)
#define ATTR_FONT_LANGUAGE      TypedWhichId<SvxLanguageItem>(110)
#define ATTR_CJK_FONT           TypedWhichId<SvxFontItem>(111)
#define ATTR_CJK_FONT_HEIGHT    TypedWhichId<SvxFontHeightItem>(112)
#define ATTR_CJK_FONT_WEIGHT    TypedWhichId<SvxWeightItem>(113)
#define ATTR_CJK_FONT_POSTURE   TypedWhichId<SvxPostureItem>(114)
#define ATTR_CJK_FONT_LANGUAGE  TypedWhichId<SvxLanguageItem>(115)
#define ATTR_CTL_FONT           TypedWhichId<SvxFontItem>(116)
#define ATTR_CTL_FONT_HEIGHT    TypedWhichId<SvxFontHeightItem>(117)
#define ATTR_CTL_FONT_WEIGHT    TypedWhichId<SvxWeightItem>(118)
#define ATTR_CTL_FONT_POSTURE   TypedWhichId<SvxPostureItem>(119)
#define ATTR_CTL_FONT_LANGUAGE  TypedWhichId<SvxLanguageItem>(120)
#define ATTR_FONT_EMPHASISMARK  TypedWhichId<SvxEmphasisMarkItem>(121)
#define ATTR_USERDEF            TypedWhichId<SvXMLAttrContainerItem>(122)    // not saved in binary files
#define ATTR_FONT_WORDLINE      TypedWhichId<SvxWordLineModeItem>(123)
#define ATTR_FONT_RELIEF        TypedWhichId<SvxCharReliefItem>(124)
#define ATTR_HYPHENATE          TypedWhichId<SfxBoolItem>(125)
#define ATTR_SCRIPTSPACE        TypedWhichId<SvxScriptSpaceItem>(126)
#define ATTR_HANGPUNCTUATION    TypedWhichId<SvxHangingPunctuationItem>(127)
#define ATTR_FORBIDDEN_RULES    TypedWhichId<SvxForbiddenRuleItem>(128)
#define ATTR_HOR_JUSTIFY        TypedWhichId<SvxHorJustifyItem>(129)
#define ATTR_HOR_JUSTIFY_METHOD TypedWhichId<SvxJustifyMethodItem>(130)
#define ATTR_INDENT             TypedWhichId<SfxUInt16Item>(131)
#define ATTR_VER_JUSTIFY        TypedWhichId<SvxVerJustifyItem>(132)
#define ATTR_VER_JUSTIFY_METHOD TypedWhichId<SvxJustifyMethodItem>(133)
#define ATTR_STACKED            TypedWhichId<SfxBoolItem>(134)
#define ATTR_ROTATE_VALUE       TypedWhichId<SfxInt32Item>(135)
#define ATTR_ROTATE_MODE        TypedWhichId<SvxRotateModeItem>(136)
#define ATTR_VERTICAL_ASIAN     TypedWhichId<SfxBoolItem>(137)
#define ATTR_WRITINGDIR         TypedWhichId<SvxFrameDirectionItem>(138)
#define ATTR_LINEBREAK          TypedWhichId<SfxBoolItem>(139)
#define ATTR_SHRINKTOFIT        TypedWhichId<SfxBoolItem>(140)
#define ATTR_BORDER_TLBR        TypedWhichId<SvxLineItem>(141)
#define ATTR_BORDER_BLTR        TypedWhichId<SvxLineItem>(142)
#define ATTR_MARGIN             TypedWhichId<SvxMarginItem>(143)
#define ATTR_MERGE              TypedWhichId<ScMergeAttr>(144)
#define ATTR_MERGE_FLAG         TypedWhichId<ScMergeFlagAttr>(145)
#define ATTR_VALUE_FORMAT       TypedWhichId<SfxUInt32Item>(146)
#define ATTR_LANGUAGE_FORMAT    TypedWhichId<SvxLanguageItem>(147)
#define ATTR_BACKGROUND         TypedWhichId<SvxBrushItem>(148)
#define ATTR_PROTECTION         TypedWhichId<ScProtectionAttr>(149)
#define ATTR_BORDER             TypedWhichId<SvxBoxItem>(150)
#define ATTR_BORDER_INNER       TypedWhichId<SvxBoxInfoItem>(151)     // inside, because of template-EditDialog
#define ATTR_SHADOW             TypedWhichId<SvxShadowItem>(152)
#define ATTR_VALIDDATA          TypedWhichId<SfxUInt32Item>(153)
#define ATTR_CONDITIONAL        TypedWhichId<ScCondFormatItem>(154)
#define ATTR_HYPERLINK          TypedWhichId<SfxStringItem>(155)

#define ATTR_PATTERN_END        155     // end cell-attribute-pattern

#define ATTR_PATTERN            TypedWhichId<ScPatternAttr>(156)
                                        // page attributes
#define ATTR_LRSPACE            TypedWhichId<SvxLRSpaceItem>(157)    // editor: PageDesc-TabPage
#define ATTR_ULSPACE            TypedWhichId<SvxULSpaceItem>(158)
#define ATTR_PAGE               TypedWhichId<SvxPageItem>(159)
#define ATTR_PAGE_PAPERBIN      TypedWhichId<SvxPaperBinItem>(160)
#define ATTR_PAGE_SIZE          TypedWhichId<SvxSizeItem>(161)
#define ATTR_PAGE_HORCENTER     TypedWhichId<SfxBoolItem>(162)
#define ATTR_PAGE_VERCENTER     TypedWhichId<SfxBoolItem>(163)

#define ATTR_PAGE_ON            TypedWhichId<SfxBoolItem>(164)     // editor: header/footer-page
#define ATTR_PAGE_DYNAMIC       TypedWhichId<SfxBoolItem>(165)
#define ATTR_PAGE_SHARED        TypedWhichId<SfxBoolItem>(166)

#define ATTR_PAGE_NOTES         TypedWhichId<SfxBoolItem>(167)     // editor: table
#define ATTR_PAGE_GRID          TypedWhichId<SfxBoolItem>(168)
#define ATTR_PAGE_HEADERS       TypedWhichId<SfxBoolItem>(169)
#define ATTR_PAGE_CHARTS        TypedWhichId<ScViewObjectModeItem>(170)
#define ATTR_PAGE_OBJECTS       TypedWhichId<ScViewObjectModeItem>(171)
#define ATTR_PAGE_DRAWINGS      TypedWhichId<ScViewObjectModeItem>(172)
#define ATTR_PAGE_TOPDOWN       TypedWhichId<SfxBoolItem>(173)
#define ATTR_PAGE_SCALE         TypedWhichId<SfxUInt16Item>(174)
#define ATTR_PAGE_SCALETOPAGES  TypedWhichId<SfxUInt16Item>(175)
#define ATTR_PAGE_FIRSTPAGENO   TypedWhichId<SfxUInt16Item>(176)

#define ATTR_PAGE_HEADERLEFT    TypedWhichId<ScPageHFItem>(177)     // contents of header/
#define ATTR_PAGE_FOOTERLEFT    TypedWhichId<ScPageHFItem>(178)     // footer (left)
#define ATTR_PAGE_HEADERRIGHT   TypedWhichId<ScPageHFItem>(179)     // contents of header/
#define ATTR_PAGE_FOOTERRIGHT   TypedWhichId<ScPageHFItem>(180)     // footer (right)
#define ATTR_PAGE_HEADERSET     TypedWhichId<SvxSetItem>(181)     // the corresponding sets
#define ATTR_PAGE_FOOTERSET     TypedWhichId<SvxSetItem>(182)

#define ATTR_PAGE_FORMULAS      TypedWhichId<SfxBoolItem>(183)
#define ATTR_PAGE_NULLVALS      TypedWhichId<SfxBoolItem>(184)

#define ATTR_PAGE_SCALETO       TypedWhichId<ScPageScaleToItem>(185)     // #i8868# scale printout to width/height

#define ATTR_HIDDEN             TypedWhichId<SfxBoolItem>(186)

#define ATTR_ENDINDEX           ATTR_HIDDEN        // end of pool-range

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
