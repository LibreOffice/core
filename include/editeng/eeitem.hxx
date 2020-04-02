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

#ifndef INCLUDED_EDITENG_EEITEM_HXX
#define INCLUDED_EDITENG_EEITEM_HXX

#include <svl/solar.hrc>
#include <svl/typedwhich.hxx>

class SfxBoolItem;
class SfxGrabBagItem;
class SfxInt16Item;
class SvxAdjustItem;
class SvxAutoKernItem;
class SvxBackgroundColorItem;
class SvxCaseMapItem;
class SvxCharReliefItem;
class SvxCharScaleWidthItem;
class SvxColorItem;
class SvxContourItem;
class SvxCrossedOutItem;
class SvxEmphasisMarkItem;
class SvxEscapementItem;
class SvxFontHeightItem;
class SvxFontItem;
class SvxForbiddenRuleItem;
class SvxFrameDirectionItem;
class SvxHangingPunctuationItem;
class SvxKerningItem;
class SvxLanguageItem;
class SvxLineSpacingItem;
class SvxLRSpaceItem;
class SvxOverlineItem;
class SvxPostureItem;
class SvXMLAttrContainerItem;
class SvxScriptSpaceItem;
class SvxShadowedItem;
class SvxTabStopItem;
class SvxULSpaceItem;
class SvxUnderlineItem;
class SvxWeightItem;
class SvxWordLineModeItem;
class SvxBulletItem;
class SvxNumBulletItem;
class SvxJustifyMethodItem;
class SvxVerJustifyItem;
class SfxVoidItem;

/*
 * NOTE: Changes in this file will probably require
 *        consistent changes in eerdll.cxx and editdoc.cxx
 *        as well.
*/

constexpr sal_uInt16 EE_ITEMS_START(OWN_ATTR_VALUE_END + 1);

// Paragraph attributes:
constexpr sal_uInt16                              EE_PARA_START              (EE_ITEMS_START + 0);
constexpr TypedWhichId<SvxFrameDirectionItem>     EE_PARA_WRITINGDIR         (EE_PARA_START+0);
constexpr TypedWhichId<SvXMLAttrContainerItem>    EE_PARA_XMLATTRIBS         (EE_PARA_START+1);
constexpr TypedWhichId<SvxHangingPunctuationItem> EE_PARA_HANGINGPUNCTUATION (EE_PARA_START+2);
constexpr TypedWhichId<SvxForbiddenRuleItem>      EE_PARA_FORBIDDENRULES     (EE_PARA_START+3);
constexpr TypedWhichId<SvxScriptSpaceItem>        EE_PARA_ASIANCJKSPACING    (EE_PARA_START+4);
constexpr TypedWhichId<SvxNumBulletItem>          EE_PARA_NUMBULLET          (EE_PARA_START+5);
constexpr TypedWhichId<SfxBoolItem>               EE_PARA_HYPHENATE          (EE_PARA_START+6);
constexpr TypedWhichId<SfxBoolItem>               EE_PARA_HYPHENATE_NO_CAPS  (EE_PARA_START+7);
constexpr TypedWhichId<SfxBoolItem>               EE_PARA_BULLETSTATE        (EE_PARA_START+8);
constexpr TypedWhichId<SvxLRSpaceItem>            EE_PARA_OUTLLRSPACE        (EE_PARA_START+9);
constexpr TypedWhichId<SfxInt16Item>              EE_PARA_OUTLLEVEL          (EE_PARA_START+10);
constexpr TypedWhichId<SvxBulletItem>             EE_PARA_BULLET             (EE_PARA_START+11);
constexpr TypedWhichId<SvxLRSpaceItem>            EE_PARA_LRSPACE            (EE_PARA_START+12);
constexpr TypedWhichId<SvxULSpaceItem>            EE_PARA_ULSPACE            (EE_PARA_START+13);
constexpr TypedWhichId<SvxLineSpacingItem>        EE_PARA_SBL                (EE_PARA_START+14);
constexpr TypedWhichId<SvxAdjustItem>             EE_PARA_JUST               (EE_PARA_START+15);
constexpr TypedWhichId<SvxTabStopItem>            EE_PARA_TABS               (EE_PARA_START+16);
constexpr TypedWhichId<SvxJustifyMethodItem>      EE_PARA_JUST_METHOD        (EE_PARA_START+17);
constexpr TypedWhichId<SvxVerJustifyItem>         EE_PARA_VER_JUST           (EE_PARA_START+18);
constexpr sal_uInt16                              EE_PARA_END                (EE_PARA_START + 18);

// Character attributes:
constexpr sal_uInt16                           EE_CHAR_START          (EE_PARA_END + 1);
constexpr TypedWhichId<SvxColorItem>           EE_CHAR_COLOR          (EE_CHAR_START+0);
constexpr TypedWhichId<SvxFontItem>            EE_CHAR_FONTINFO       (EE_CHAR_START+1);
constexpr TypedWhichId<SvxFontHeightItem>      EE_CHAR_FONTHEIGHT     (EE_CHAR_START+2);
constexpr TypedWhichId<SvxCharScaleWidthItem>  EE_CHAR_FONTWIDTH      (EE_CHAR_START+3);
constexpr TypedWhichId<SvxWeightItem>          EE_CHAR_WEIGHT         (EE_CHAR_START+4);
constexpr TypedWhichId<SvxUnderlineItem>       EE_CHAR_UNDERLINE      (EE_CHAR_START+5);
constexpr TypedWhichId<SvxCrossedOutItem>      EE_CHAR_STRIKEOUT      (EE_CHAR_START+6);
constexpr TypedWhichId<SvxPostureItem>         EE_CHAR_ITALIC         (EE_CHAR_START+7);
constexpr TypedWhichId<SvxContourItem>         EE_CHAR_OUTLINE        (EE_CHAR_START+8);
constexpr TypedWhichId<SvxShadowedItem>        EE_CHAR_SHADOW         (EE_CHAR_START+9);
constexpr TypedWhichId<SvxEscapementItem>      EE_CHAR_ESCAPEMENT     (EE_CHAR_START+10);
constexpr TypedWhichId<SvxAutoKernItem>        EE_CHAR_PAIRKERNING    (EE_CHAR_START+11);
constexpr TypedWhichId<SvxKerningItem>         EE_CHAR_KERNING        (EE_CHAR_START+12);
constexpr TypedWhichId<SvxWordLineModeItem>    EE_CHAR_WLM            (EE_CHAR_START+13);
constexpr TypedWhichId<SvxLanguageItem>        EE_CHAR_LANGUAGE       (EE_CHAR_START+14);
constexpr TypedWhichId<SvxLanguageItem>        EE_CHAR_LANGUAGE_CJK   (EE_CHAR_START+15);
constexpr TypedWhichId<SvxLanguageItem>        EE_CHAR_LANGUAGE_CTL   (EE_CHAR_START+16);
constexpr TypedWhichId<SvxFontItem>            EE_CHAR_FONTINFO_CJK   (EE_CHAR_START+17);
constexpr TypedWhichId<SvxFontItem>            EE_CHAR_FONTINFO_CTL   (EE_CHAR_START+18);
constexpr TypedWhichId<SvxFontHeightItem>      EE_CHAR_FONTHEIGHT_CJK (EE_CHAR_START+19);
constexpr TypedWhichId<SvxFontHeightItem>      EE_CHAR_FONTHEIGHT_CTL (EE_CHAR_START+20);
constexpr TypedWhichId<SvxWeightItem>          EE_CHAR_WEIGHT_CJK     (EE_CHAR_START+21);
constexpr TypedWhichId<SvxWeightItem>          EE_CHAR_WEIGHT_CTL     (EE_CHAR_START+22);
constexpr TypedWhichId<SvxPostureItem>         EE_CHAR_ITALIC_CJK     (EE_CHAR_START+23);
constexpr TypedWhichId<SvxPostureItem>         EE_CHAR_ITALIC_CTL     (EE_CHAR_START+24);
constexpr TypedWhichId<SvxEmphasisMarkItem>    EE_CHAR_EMPHASISMARK   (EE_CHAR_START+25);
constexpr TypedWhichId<SvxCharReliefItem>      EE_CHAR_RELIEF         (EE_CHAR_START+26);
constexpr TypedWhichId<SfxVoidItem>            EE_CHAR_RUBI_DUMMY     (EE_CHAR_START+27);
constexpr TypedWhichId<SvXMLAttrContainerItem> EE_CHAR_XMLATTRIBS     (EE_CHAR_START+28);
constexpr TypedWhichId<SvxOverlineItem>        EE_CHAR_OVERLINE       (EE_CHAR_START+29);
constexpr TypedWhichId<SvxCaseMapItem>         EE_CHAR_CASEMAP        (EE_CHAR_START+30);
constexpr TypedWhichId<SfxGrabBagItem>         EE_CHAR_GRABBAG        (EE_CHAR_START+31);
constexpr TypedWhichId<SvxBackgroundColorItem> EE_CHAR_BKGCOLOR       (EE_CHAR_START+32);

constexpr sal_uInt16                           EE_CHAR_END            (EE_CHAR_START + 32);

constexpr sal_uInt16 EE_FEATURE_START   (EE_CHAR_END + 1);
constexpr sal_uInt16 EE_FEATURE_TAB     (EE_FEATURE_START + 0);
constexpr sal_uInt16 EE_FEATURE_LINEBR  (EE_FEATURE_TAB + 1);
constexpr sal_uInt16 EE_FEATURE_NOTCONV (EE_FEATURE_LINEBR + 1);
constexpr sal_uInt16 EE_FEATURE_FIELD   (EE_FEATURE_NOTCONV + 1);
constexpr sal_uInt16 EE_FEATURE_END     (EE_FEATURE_FIELD + 0);

constexpr sal_uInt16 EE_ITEMS_END       (EE_FEATURE_END);

constexpr sal_uInt16 EDITITEMCOUNT (EE_ITEMS_END - EE_ITEMS_START + 1);

#endif // _ INCLUDED_EDITENG_EEITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
