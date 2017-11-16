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

/*
 * NOTE: Changes in this file will probably require
 *        consistent changes in eerdll.cxx and editdoc.cxx
 *        as well.
*/

#define EE_ITEMS_START              (OWN_ATTR_VALUE_END+1)

// Paragraph attributes:
#define EE_PARA_START               (EE_ITEMS_START+0)
#define EE_PARA_WRITINGDIR          TypedWhichId<SvxFrameDirectionItem>(EE_ITEMS_START+0)
#define EE_PARA_XMLATTRIBS          TypedWhichId<SvXMLAttrContainerItem>(EE_ITEMS_START+1)
#define EE_PARA_HANGINGPUNCTUATION  TypedWhichId<SvxHangingPunctuationItem>(EE_ITEMS_START+2)
#define EE_PARA_FORBIDDENRULES      TypedWhichId<SvxForbiddenRuleItem>(EE_ITEMS_START+3)
#define EE_PARA_ASIANCJKSPACING     TypedWhichId<SvxScriptSpaceItem>(EE_ITEMS_START+4)
#define EE_PARA_NUMBULLET           TypedWhichId<SvxNumBulletItem>(EE_ITEMS_START+5)
#define EE_PARA_HYPHENATE           TypedWhichId<SfxBoolItem>(EE_ITEMS_START+6)
#define EE_PARA_BULLETSTATE         TypedWhichId<SfxBoolItem>(EE_ITEMS_START+7)
#define EE_PARA_OUTLLRSPACE         TypedWhichId<SvxLRSpaceItem>(EE_ITEMS_START+8)
#define EE_PARA_OUTLLEVEL           TypedWhichId<SfxInt16Item>(EE_ITEMS_START+9)
#define EE_PARA_BULLET              TypedWhichId<SvxBulletItem>(EE_ITEMS_START+10)
#define EE_PARA_LRSPACE             TypedWhichId<SvxLRSpaceItem>(EE_ITEMS_START+11)
#define EE_PARA_ULSPACE             TypedWhichId<SvxULSpaceItem>(EE_ITEMS_START+12)
#define EE_PARA_SBL                 TypedWhichId<SvxLineSpacingItem>(EE_ITEMS_START+13)
#define EE_PARA_JUST                TypedWhichId<SvxAdjustItem>(EE_ITEMS_START+14)
#define EE_PARA_TABS                TypedWhichId<SvxTabStopItem>(EE_ITEMS_START+15)
#define EE_PARA_JUST_METHOD         TypedWhichId<SvxJustifyMethodItem>(EE_ITEMS_START+16)
#define EE_PARA_VER_JUST            TypedWhichId<SvxVerJustifyItem>(EE_ITEMS_START+17)
#define EE_PARA_END                 (EE_ITEMS_START+17)

// Character attributes:
#define EE_CHAR_START               (EE_ITEMS_START+18)
#define EE_CHAR_COLOR               TypedWhichId<SvxColorItem>(EE_ITEMS_START+18)
#define EE_CHAR_FONTINFO            TypedWhichId<SvxFontItem>(EE_ITEMS_START+19)
#define EE_CHAR_FONTHEIGHT          TypedWhichId<SvxFontHeightItem>(EE_ITEMS_START+20)
#define EE_CHAR_FONTWIDTH           TypedWhichId<SvxCharScaleWidthItem>(EE_ITEMS_START+21)
#define EE_CHAR_WEIGHT              TypedWhichId<SvxWeightItem>(EE_ITEMS_START+22)
#define EE_CHAR_UNDERLINE           TypedWhichId<SvxUnderlineItem>(EE_ITEMS_START+23)
#define EE_CHAR_STRIKEOUT           TypedWhichId<SvxCrossedOutItem>(EE_ITEMS_START+24)
#define EE_CHAR_ITALIC              TypedWhichId<SvxPostureItem>(EE_ITEMS_START+25)
#define EE_CHAR_OUTLINE             TypedWhichId<SvxContourItem>(EE_ITEMS_START+26)
#define EE_CHAR_SHADOW              TypedWhichId<SvxShadowedItem>(EE_ITEMS_START+27)
#define EE_CHAR_ESCAPEMENT          TypedWhichId<SvxEscapementItem>(EE_ITEMS_START+28)
#define EE_CHAR_PAIRKERNING         TypedWhichId<SvxAutoKernItem>(EE_ITEMS_START+29)
#define EE_CHAR_KERNING             TypedWhichId<SvxKerningItem>(EE_ITEMS_START+30)
#define EE_CHAR_WLM                 TypedWhichId<SvxWordLineModeItem>(EE_ITEMS_START+31)
#define EE_CHAR_LANGUAGE            TypedWhichId<SvxLanguageItem>(EE_ITEMS_START+32)
#define EE_CHAR_LANGUAGE_CJK        TypedWhichId<SvxLanguageItem>(EE_ITEMS_START+33)
#define EE_CHAR_LANGUAGE_CTL        TypedWhichId<SvxLanguageItem>(EE_ITEMS_START+34)
#define EE_CHAR_FONTINFO_CJK        TypedWhichId<SvxFontItem>(EE_ITEMS_START+35)
#define EE_CHAR_FONTINFO_CTL        TypedWhichId<SvxFontItem>(EE_ITEMS_START+36)
#define EE_CHAR_FONTHEIGHT_CJK      TypedWhichId<SvxFontHeightItem>(EE_ITEMS_START+37)
#define EE_CHAR_FONTHEIGHT_CTL      TypedWhichId<SvxFontHeightItem>(EE_ITEMS_START+38)
#define EE_CHAR_WEIGHT_CJK          TypedWhichId<SvxWeightItem>(EE_ITEMS_START+39)
#define EE_CHAR_WEIGHT_CTL          TypedWhichId<SvxWeightItem>(EE_ITEMS_START+40)
#define EE_CHAR_ITALIC_CJK          TypedWhichId<SvxPostureItem>(EE_ITEMS_START+41)
#define EE_CHAR_ITALIC_CTL          TypedWhichId<SvxPostureItem>(EE_ITEMS_START+42)
#define EE_CHAR_EMPHASISMARK        TypedWhichId<SvxEmphasisMarkItem>(EE_ITEMS_START+43)
#define EE_CHAR_RELIEF              TypedWhichId<SvxCharReliefItem>(EE_ITEMS_START+44)
#define EE_CHAR_RUBI_DUMMY          TypedWhichId<SfxVoidItem>(EE_ITEMS_START+45)
#define EE_CHAR_XMLATTRIBS          TypedWhichId<SvXMLAttrContainerItem>(EE_ITEMS_START+46)
#define EE_CHAR_OVERLINE            TypedWhichId<SvxOverlineItem>(EE_ITEMS_START+47)
#define EE_CHAR_CASEMAP             TypedWhichId<SvxCaseMapItem>(EE_ITEMS_START+48)
#define EE_CHAR_GRABBAG             TypedWhichId<SfxGrabBagItem>(EE_ITEMS_START+49)
#define EE_CHAR_BKGCOLOR            TypedWhichId<SvxBackgroundColorItem>(EE_ITEMS_START+50)

#define EE_CHAR_END                 (EE_ITEMS_START+50)

#define EE_FEATURE_START            (EE_ITEMS_START+51)
#define EE_FEATURE_TAB              (EE_ITEMS_START+51)
#define EE_FEATURE_LINEBR           (EE_ITEMS_START+52)
#define EE_FEATURE_NOTCONV          (EE_ITEMS_START+53)
#define EE_FEATURE_FIELD            (EE_ITEMS_START+54)
#define EE_FEATURE_END              (EE_ITEMS_START+54)

#define EE_ITEMS_END                (EE_ITEMS_START+54)

#define EDITITEMCOUNT ( EE_ITEMS_END - EE_ITEMS_START + 1 )

#endif // _ INCLUDED_EDITENG_EEITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
