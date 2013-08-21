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
#ifndef _CHARATR_HXX
#define _CHARATR_HXX

#include <format.hxx>
#include <hintids.hxx>


// implementation of the character attribute methods of SwAttrSet

inline const SvxPostureItem      &SwAttrSet::GetPosture(sal_Bool bInP) const
    {   return (const SvxPostureItem&)Get( RES_CHRATR_POSTURE,bInP); }
inline const SvxPostureItem      &SwAttrSet::GetCJKPosture(sal_Bool bInP) const
    {   return (const SvxPostureItem&)Get( RES_CHRATR_CJK_POSTURE,bInP); }
inline const SvxPostureItem      &SwAttrSet::GetCTLPosture(sal_Bool bInP) const
    {   return (const SvxPostureItem&)Get( RES_CHRATR_CTL_POSTURE,bInP); }
inline const SvxWeightItem       &SwAttrSet::GetWeight(sal_Bool bInP) const
    {   return (const SvxWeightItem&)Get( RES_CHRATR_WEIGHT,bInP); }
inline const SvxWeightItem       &SwAttrSet::GetCJKWeight(sal_Bool bInP) const
    {   return (const SvxWeightItem&)Get( RES_CHRATR_CJK_WEIGHT,bInP); }
inline const SvxWeightItem       &SwAttrSet::GetCTLWeight(sal_Bool bInP) const
    {   return (const SvxWeightItem&)Get( RES_CHRATR_CTL_WEIGHT,bInP); }
inline const SvxShadowedItem     &SwAttrSet::GetShadowed(sal_Bool bInP) const
    {   return (const SvxShadowedItem&)Get( RES_CHRATR_SHADOWED,bInP); }
inline const SvxAutoKernItem     &SwAttrSet::GetAutoKern(sal_Bool bInP) const
    {   return (const SvxAutoKernItem&)Get( RES_CHRATR_AUTOKERN,bInP); }
inline const SvxWordLineModeItem     &SwAttrSet::GetWordLineMode(sal_Bool bInP) const
    {   return (const SvxWordLineModeItem&)Get( RES_CHRATR_WORDLINEMODE,bInP); }
inline const SvxContourItem      &SwAttrSet::GetContour(sal_Bool bInP) const
    {   return (const SvxContourItem&)Get( RES_CHRATR_CONTOUR,bInP); }
inline const SvxKerningItem      &SwAttrSet::GetKerning(sal_Bool bInP) const
    {   return (const SvxKerningItem&)Get( RES_CHRATR_KERNING,bInP); }
inline const SvxUnderlineItem    &SwAttrSet::GetUnderline(sal_Bool bInP) const
    {   return (const SvxUnderlineItem&)Get( RES_CHRATR_UNDERLINE,bInP); }
inline const SvxOverlineItem     &SwAttrSet::GetOverline(sal_Bool bInP) const
    {   return (const SvxOverlineItem&)Get( RES_CHRATR_OVERLINE,bInP); }
inline const SvxCrossedOutItem   &SwAttrSet::GetCrossedOut(sal_Bool bInP) const
    {   return (const SvxCrossedOutItem&)Get( RES_CHRATR_CROSSEDOUT,bInP); }
inline const SvxFontHeightItem         &SwAttrSet::GetSize(sal_Bool bInP) const
    {   return (const SvxFontHeightItem&)Get( RES_CHRATR_FONTSIZE,bInP); }
inline const SvxFontHeightItem         &SwAttrSet::GetCJKSize(sal_Bool bInP) const
    {   return (const SvxFontHeightItem&)Get( RES_CHRATR_CJK_FONTSIZE,bInP); }
inline const SvxFontHeightItem         &SwAttrSet::GetCTLSize(sal_Bool bInP) const
    {   return (const SvxFontHeightItem&)Get( RES_CHRATR_CTL_FONTSIZE,bInP); }
inline const SvxPropSizeItem     &SwAttrSet::GetPropSize(sal_Bool bInP) const
    {   return (const SvxPropSizeItem&)Get( RES_CHRATR_PROPORTIONALFONTSIZE,bInP); }
inline const SvxFontItem         &SwAttrSet::GetFont(sal_Bool bInP) const
    {   return (const SvxFontItem&)Get( RES_CHRATR_FONT,bInP); }
inline const SvxFontItem         &SwAttrSet::GetCJKFont(sal_Bool bInP) const
    {   return (const SvxFontItem&)Get( RES_CHRATR_CJK_FONT,bInP); }
inline const SvxFontItem         &SwAttrSet::GetCTLFont(sal_Bool bInP) const
    {   return (const SvxFontItem&)Get( RES_CHRATR_CTL_FONT,bInP); }
inline const SvxColorItem        &SwAttrSet::GetColor(sal_Bool bInP) const
    {   return (const SvxColorItem&)Get( RES_CHRATR_COLOR,bInP); }
inline const SvxCharSetColorItem &SwAttrSet::GetCharSetColor(sal_Bool bInP) const
    {   return (const SvxCharSetColorItem&)Get( RES_CHRATR_CHARSETCOLOR,bInP); }
inline const SvxLanguageItem     &SwAttrSet::GetLanguage(sal_Bool bInP) const
    {   return (const SvxLanguageItem&)Get( RES_CHRATR_LANGUAGE,bInP); }
inline const SvxLanguageItem     &SwAttrSet::GetCJKLanguage(sal_Bool bInP) const
    {   return (const SvxLanguageItem&)Get( RES_CHRATR_CJK_LANGUAGE,bInP); }
inline const SvxLanguageItem     &SwAttrSet::GetCTLLanguage(sal_Bool bInP) const
    {   return (const SvxLanguageItem&)Get( RES_CHRATR_CTL_LANGUAGE,bInP); }
inline const SvxEscapementItem   &SwAttrSet::GetEscapement(sal_Bool bInP) const
    {   return (const SvxEscapementItem&)Get( RES_CHRATR_ESCAPEMENT,bInP); }
inline const SvxCaseMapItem     &SwAttrSet::GetCaseMap(sal_Bool bInP) const
    {   return (const SvxCaseMapItem&)Get( RES_CHRATR_CASEMAP,bInP); }
inline const SvxNoHyphenItem    &SwAttrSet::GetNoHyphenHere(sal_Bool bInP) const
    {   return (const SvxNoHyphenItem&)Get( RES_CHRATR_NOHYPHEN,bInP); }
inline const SvxBlinkItem  &SwAttrSet::GetBlink(sal_Bool bInP) const
    {   return (const SvxBlinkItem&)Get( RES_CHRATR_BLINK,bInP); }
inline const SvxBrushItem &SwAttrSet::GetChrBackground( sal_Bool bInP ) const
    {   return (const SvxBrushItem&)Get( RES_CHRATR_BACKGROUND, bInP ); }
inline const SvxEmphasisMarkItem &SwAttrSet::GetEmphasisMark( sal_Bool bInP ) const
    {   return (const SvxEmphasisMarkItem&)Get( RES_CHRATR_EMPHASIS_MARK, bInP ); }
inline const SvxTwoLinesItem    &SwAttrSet::Get2Lines( sal_Bool bInP ) const
    {   return (const SvxTwoLinesItem&)Get( RES_CHRATR_TWO_LINES, bInP ); }
inline const SvxCharScaleWidthItem  &SwAttrSet::GetCharScaleW( sal_Bool bInP ) const
    {   return (const SvxCharScaleWidthItem&)Get( RES_CHRATR_SCALEW, bInP ); }
inline const SvxCharRotateItem  &SwAttrSet::GetCharRotate( sal_Bool bInP ) const
    {   return (const SvxCharRotateItem&)Get( RES_CHRATR_ROTATE, bInP ); }
inline const SvxCharReliefItem  &SwAttrSet::GetCharRelief( sal_Bool bInP ) const
    {   return (const SvxCharReliefItem&)Get( RES_CHRATR_RELIEF, bInP ); }
inline const SvxCharHiddenItem  &SwAttrSet::GetCharHidden( sal_Bool bInP ) const
    {   return (const SvxCharHiddenItem&)Get( RES_CHRATR_HIDDEN, bInP ); }
inline const SvxBoxItem  &SwAttrSet::GetCharBorder( sal_Bool bInP ) const
    {   return (const SvxBoxItem&)Get( RES_CHRATR_BOX, bInP ); }
inline const SvxShadowItem  &SwAttrSet::GetCharShadow( sal_Bool bInP ) const
    {   return (const SvxShadowItem&)Get( RES_CHRATR_SHADOW, bInP ); }

// implementation of the character attribute methods of SwFmt

inline const SvxPostureItem          &SwFmt::GetPosture(sal_Bool bInP) const
    {   return aSet.GetPosture(bInP); }
inline const SvxWeightItem           &SwFmt::GetWeight(sal_Bool bInP) const
    {   return aSet.GetWeight(bInP); }
inline const SvxShadowedItem         &SwFmt::GetShadowed(sal_Bool bInP) const
    {   return aSet.GetShadowed(bInP); }
inline const SvxAutoKernItem         &SwFmt::GetAutoKern(sal_Bool bInP) const
    {   return aSet.GetAutoKern(bInP); }
inline const SvxWordLineModeItem     &SwFmt::GetWordLineMode(sal_Bool bInP) const
    {   return aSet.GetWordLineMode(bInP); }
inline const SvxContourItem          &SwFmt::GetContour(sal_Bool bInP) const
    {   return aSet.GetContour(bInP); }
inline const SvxKerningItem          &SwFmt::GetKerning(sal_Bool bInP) const
    {   return aSet.GetKerning(bInP); }
inline const SvxUnderlineItem        &SwFmt::GetUnderline(sal_Bool bInP) const
    {   return aSet.GetUnderline(bInP); }
inline const SvxOverlineItem         &SwFmt::GetOverline(sal_Bool bInP) const
    {   return aSet.GetOverline(bInP); }
inline const SvxCrossedOutItem       &SwFmt::GetCrossedOut(sal_Bool bInP) const
    {   return aSet.GetCrossedOut(bInP); }
inline const SvxFontHeightItem             &SwFmt::GetSize(sal_Bool bInP) const
    {   return aSet.GetSize(bInP); }
inline const SvxPropSizeItem         &SwFmt::GetPropSize(sal_Bool bInP) const
    {   return aSet.GetPropSize(bInP); }
inline const SvxFontItem             &SwFmt::GetFont(sal_Bool bInP) const
    {   return aSet.GetFont(bInP); }
inline const SvxFontItem         &SwFmt::GetCJKFont(sal_Bool bInP) const
    {   return aSet.GetCJKFont(bInP); }
inline const SvxFontItem         &SwFmt::GetCTLFont(sal_Bool bInP) const
    {   return aSet.GetCTLFont(bInP); }
inline const SvxColorItem            &SwFmt::GetColor(sal_Bool bInP) const
    {   return aSet.GetColor(bInP); }
    inline const SvxCharSetColorItem    &SwFmt::GetCharSetColor(sal_Bool bInP) const
    {   return aSet.GetCharSetColor(bInP); }
inline const SvxLanguageItem         &SwFmt::GetLanguage(sal_Bool bInP) const
    {   return aSet.GetLanguage(bInP); }
inline const SvxEscapementItem       &SwFmt::GetEscapement(sal_Bool bInP) const
    {   return aSet.GetEscapement(bInP); }
inline const SvxCaseMapItem          &SwFmt::GetCaseMap(sal_Bool bInP) const
    {   return aSet.GetCaseMap(bInP); }
inline const SvxNoHyphenItem        &SwFmt::GetNoHyphenHere(sal_Bool bInP) const
    {   return aSet.GetNoHyphenHere(bInP); }
inline const SvxBlinkItem           &SwFmt::GetBlink(sal_Bool bInP) const
    {   return aSet.GetBlink(bInP); }
inline const SvxBrushItem           &SwFmt::GetChrBackground(sal_Bool bInP) const
    {   return aSet.GetChrBackground(bInP); }
inline const SvxEmphasisMarkItem   &SwFmt::GetEmphasisMark( sal_Bool bInP ) const
    {   return aSet.GetEmphasisMark(bInP); }
inline const SvxTwoLinesItem       &SwFmt::Get2Lines( sal_Bool bInP ) const
    {   return aSet.Get2Lines(bInP); }
inline const SvxCharScaleWidthItem &SwFmt::GetCharScaleW( sal_Bool bInP ) const
    {   return aSet.GetCharScaleW(bInP); }
inline const SvxCharRotateItem     &SwFmt::GetCharRotate( sal_Bool bInP ) const
    {   return aSet.GetCharRotate(bInP); }
inline const SvxCharReliefItem     &SwFmt::GetCharRelief( sal_Bool bInP ) const
    {   return aSet.GetCharRelief(bInP); }
inline const SvxCharHiddenItem      &SwFmt::GetCharHidden( sal_Bool bInP ) const
    {   return aSet.GetCharHidden(bInP); }
inline const SvxBoxItem     &SwFmt::GetCharBorder( sal_Bool bInP ) const
    {   return aSet.GetCharBorder(bInP); }
inline const SvxShadowItem  &SwFmt::GetCharShadow( sal_Bool bInP ) const
    {   return aSet.GetCharShadow(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
