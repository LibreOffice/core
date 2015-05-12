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
#ifndef INCLUDED_SW_INC_CHARATR_HXX
#define INCLUDED_SW_INC_CHARATR_HXX

#include <format.hxx>
#include <hintids.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/prszitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/charsetcoloritem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/nhypitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/langitem.hxx>

// implementation of the character attribute methods of SwAttrSet

inline const SvxPostureItem      &SwAttrSet::GetPosture(bool bInP) const
    {   return static_cast<const SvxPostureItem&>(Get( RES_CHRATR_POSTURE,bInP)); }
inline const SvxPostureItem      &SwAttrSet::GetCJKPosture(bool bInP) const
    {   return static_cast<const SvxPostureItem&>(Get( RES_CHRATR_CJK_POSTURE,bInP)); }
inline const SvxPostureItem      &SwAttrSet::GetCTLPosture(bool bInP) const
    {   return static_cast<const SvxPostureItem&>(Get( RES_CHRATR_CTL_POSTURE,bInP)); }
inline const SvxWeightItem       &SwAttrSet::GetWeight(bool bInP) const
    {   return static_cast<const SvxWeightItem&>(Get( RES_CHRATR_WEIGHT,bInP)); }
inline const SvxWeightItem       &SwAttrSet::GetCJKWeight(bool bInP) const
    {   return static_cast<const SvxWeightItem&>(Get( RES_CHRATR_CJK_WEIGHT,bInP)); }
inline const SvxWeightItem       &SwAttrSet::GetCTLWeight(bool bInP) const
    {   return static_cast<const SvxWeightItem&>(Get( RES_CHRATR_CTL_WEIGHT,bInP)); }
inline const SvxShadowedItem     &SwAttrSet::GetShadowed(bool bInP) const
    {   return static_cast<const SvxShadowedItem&>(Get( RES_CHRATR_SHADOWED,bInP)); }
inline const SvxAutoKernItem     &SwAttrSet::GetAutoKern(bool bInP) const
    {   return static_cast<const SvxAutoKernItem&>(Get( RES_CHRATR_AUTOKERN,bInP)); }
inline const SvxWordLineModeItem &SwAttrSet::GetWordLineMode(bool bInP) const
    {   return static_cast<const SvxWordLineModeItem&>(Get( RES_CHRATR_WORDLINEMODE,bInP)); }
inline const SvxContourItem      &SwAttrSet::GetContour(bool bInP) const
    {   return static_cast<const SvxContourItem&>(Get( RES_CHRATR_CONTOUR,bInP)); }
inline const SvxKerningItem      &SwAttrSet::GetKerning(bool bInP) const
    {   return static_cast<const SvxKerningItem&>(Get( RES_CHRATR_KERNING,bInP)); }
inline const SvxUnderlineItem    &SwAttrSet::GetUnderline(bool bInP) const
    {   return static_cast<const SvxUnderlineItem&>(Get( RES_CHRATR_UNDERLINE,bInP)); }
inline const SvxOverlineItem     &SwAttrSet::GetOverline(bool bInP) const
    {   return static_cast<const SvxOverlineItem&>(Get( RES_CHRATR_OVERLINE,bInP)); }
inline const SvxCrossedOutItem   &SwAttrSet::GetCrossedOut(bool bInP) const
    {   return static_cast<const SvxCrossedOutItem&>(Get( RES_CHRATR_CROSSEDOUT,bInP)); }
inline const SvxFontHeightItem   &SwAttrSet::GetSize(bool bInP) const
    {   return static_cast<const SvxFontHeightItem&>(Get( RES_CHRATR_FONTSIZE,bInP)); }
inline const SvxFontHeightItem   &SwAttrSet::GetCJKSize(bool bInP) const
    {   return static_cast<const SvxFontHeightItem&>(Get( RES_CHRATR_CJK_FONTSIZE,bInP)); }
inline const SvxFontHeightItem   &SwAttrSet::GetCTLSize(bool bInP) const
    {   return static_cast<const SvxFontHeightItem&>(Get( RES_CHRATR_CTL_FONTSIZE,bInP)); }
inline const SvxPropSizeItem     &SwAttrSet::GetPropSize(bool bInP) const
    {   return static_cast<const SvxPropSizeItem&>(Get( RES_CHRATR_PROPORTIONALFONTSIZE,bInP)); }
inline const SvxFontItem         &SwAttrSet::GetFont(bool bInP) const
    {   return static_cast<const SvxFontItem&>(Get( RES_CHRATR_FONT,bInP)); }
inline const SvxFontItem         &SwAttrSet::GetCJKFont(bool bInP) const
    {   return static_cast<const SvxFontItem&>(Get( RES_CHRATR_CJK_FONT,bInP)); }
inline const SvxFontItem         &SwAttrSet::GetCTLFont(bool bInP) const
    {   return static_cast<const SvxFontItem&>(Get( RES_CHRATR_CTL_FONT,bInP)); }
inline const SvxColorItem        &SwAttrSet::GetColor(bool bInP) const
    {   return static_cast<const SvxColorItem&>(Get( RES_CHRATR_COLOR,bInP)); }
inline const SvxCharSetColorItem &SwAttrSet::GetCharSetColor(bool bInP) const
    {   return static_cast<const SvxCharSetColorItem&>(Get( RES_CHRATR_CHARSETCOLOR,bInP)); }
inline const SvxLanguageItem     &SwAttrSet::GetLanguage(bool bInP) const
    {   return static_cast<const SvxLanguageItem&>(Get( RES_CHRATR_LANGUAGE,bInP)); }
inline const SvxLanguageItem     &SwAttrSet::GetCJKLanguage(bool bInP) const
    {   return static_cast<const SvxLanguageItem&>(Get( RES_CHRATR_CJK_LANGUAGE,bInP)); }
inline const SvxLanguageItem     &SwAttrSet::GetCTLLanguage(bool bInP) const
    {   return static_cast<const SvxLanguageItem&>(Get( RES_CHRATR_CTL_LANGUAGE,bInP)); }
inline const SvxEscapementItem   &SwAttrSet::GetEscapement(bool bInP) const
    {   return static_cast<const SvxEscapementItem&>(Get( RES_CHRATR_ESCAPEMENT,bInP)); }
inline const SvxCaseMapItem      &SwAttrSet::GetCaseMap(bool bInP) const
    {   return static_cast<const SvxCaseMapItem&>(Get( RES_CHRATR_CASEMAP,bInP)); }
inline const SvxNoHyphenItem     &SwAttrSet::GetNoHyphenHere(bool bInP) const
    {   return static_cast<const SvxNoHyphenItem&>(Get( RES_CHRATR_NOHYPHEN,bInP)); }
inline const SvxBlinkItem        &SwAttrSet::GetBlink(bool bInP) const
    {   return static_cast<const SvxBlinkItem&>(Get( RES_CHRATR_BLINK,bInP)); }
inline const SvxBrushItem        &SwAttrSet::GetChrBackground( bool bInP ) const
    {   return static_cast<const SvxBrushItem&>(Get( RES_CHRATR_BACKGROUND, bInP )); }
inline const SvxBrushItem        &SwAttrSet::GetChrHighlight( bool bInP ) const
    {   return static_cast<const SvxBrushItem&>(Get( RES_CHRATR_HIGHLIGHT, bInP )); }
inline const SvxEmphasisMarkItem &SwAttrSet::GetEmphasisMark( bool bInP ) const
    {   return static_cast<const SvxEmphasisMarkItem&>(Get( RES_CHRATR_EMPHASIS_MARK, bInP )); }
inline const SvxTwoLinesItem     &SwAttrSet::Get2Lines( bool bInP ) const
    {   return static_cast<const SvxTwoLinesItem&>(Get( RES_CHRATR_TWO_LINES, bInP )); }
inline const SvxCharScaleWidthItem  &SwAttrSet::GetCharScaleW( bool bInP ) const
    {   return static_cast<const SvxCharScaleWidthItem&>(Get( RES_CHRATR_SCALEW, bInP )); }
inline const SvxCharRotateItem   &SwAttrSet::GetCharRotate( bool bInP ) const
    {   return static_cast<const SvxCharRotateItem&>(Get( RES_CHRATR_ROTATE, bInP )); }
inline const SvxCharReliefItem   &SwAttrSet::GetCharRelief( bool bInP ) const
    {   return static_cast<const SvxCharReliefItem&>(Get( RES_CHRATR_RELIEF, bInP )); }
inline const SvxCharHiddenItem   &SwAttrSet::GetCharHidden( bool bInP ) const
    {   return static_cast<const SvxCharHiddenItem&>(Get( RES_CHRATR_HIDDEN, bInP )); }
inline const SvxBoxItem          &SwAttrSet::GetCharBorder( bool bInP ) const
    {   return static_cast<const SvxBoxItem&>(Get( RES_CHRATR_BOX, bInP )); }
inline const SvxShadowItem       &SwAttrSet::GetCharShadow( bool bInP ) const
    {   return static_cast<const SvxShadowItem&>(Get( RES_CHRATR_SHADOW, bInP )); }

// implementation of the character attribute methods of SwFormat

inline const SvxPostureItem          &SwFormat::GetPosture(bool bInP) const
    {   return m_aSet.GetPosture(bInP); }
inline const SvxWeightItem           &SwFormat::GetWeight(bool bInP) const
    {   return m_aSet.GetWeight(bInP); }
inline const SvxShadowedItem         &SwFormat::GetShadowed(bool bInP) const
    {   return m_aSet.GetShadowed(bInP); }
inline const SvxAutoKernItem         &SwFormat::GetAutoKern(bool bInP) const
    {   return m_aSet.GetAutoKern(bInP); }
inline const SvxWordLineModeItem     &SwFormat::GetWordLineMode(bool bInP) const
    {   return m_aSet.GetWordLineMode(bInP); }
inline const SvxContourItem          &SwFormat::GetContour(bool bInP) const
    {   return m_aSet.GetContour(bInP); }
inline const SvxKerningItem          &SwFormat::GetKerning(bool bInP) const
    {   return m_aSet.GetKerning(bInP); }
inline const SvxUnderlineItem        &SwFormat::GetUnderline(bool bInP) const
    {   return m_aSet.GetUnderline(bInP); }
inline const SvxOverlineItem         &SwFormat::GetOverline(bool bInP) const
    {   return m_aSet.GetOverline(bInP); }
inline const SvxCrossedOutItem       &SwFormat::GetCrossedOut(bool bInP) const
    {   return m_aSet.GetCrossedOut(bInP); }
inline const SvxFontHeightItem             &SwFormat::GetSize(bool bInP) const
    {   return m_aSet.GetSize(bInP); }
inline const SvxPropSizeItem         &SwFormat::GetPropSize(bool bInP) const
    {   return m_aSet.GetPropSize(bInP); }
inline const SvxFontItem             &SwFormat::GetFont(bool bInP) const
    {   return m_aSet.GetFont(bInP); }
inline const SvxFontItem         &SwFormat::GetCJKFont(bool bInP) const
    {   return m_aSet.GetCJKFont(bInP); }
inline const SvxFontItem         &SwFormat::GetCTLFont(bool bInP) const
    {   return m_aSet.GetCTLFont(bInP); }
inline const SvxColorItem            &SwFormat::GetColor(bool bInP) const
    {   return m_aSet.GetColor(bInP); }
    inline const SvxCharSetColorItem    &SwFormat::GetCharSetColor(bool bInP) const
    {   return m_aSet.GetCharSetColor(bInP); }
inline const SvxLanguageItem         &SwFormat::GetLanguage(bool bInP) const
    {   return m_aSet.GetLanguage(bInP); }
inline const SvxEscapementItem       &SwFormat::GetEscapement(bool bInP) const
    {   return m_aSet.GetEscapement(bInP); }
inline const SvxCaseMapItem          &SwFormat::GetCaseMap(bool bInP) const
    {   return m_aSet.GetCaseMap(bInP); }
inline const SvxNoHyphenItem        &SwFormat::GetNoHyphenHere(bool bInP) const
    {   return m_aSet.GetNoHyphenHere(bInP); }
inline const SvxBlinkItem           &SwFormat::GetBlink(bool bInP) const
    {   return m_aSet.GetBlink(bInP); }
inline const SvxBrushItem           &SwFormat::GetChrBackground(bool bInP) const
    {   return m_aSet.GetChrBackground(bInP); }
inline const SvxBrushItem &SwFormat::GetChrHighlight(bool bInP) const
    {   return m_aSet.GetChrHighlight(bInP); }
inline const SvxEmphasisMarkItem   &SwFormat::GetEmphasisMark( bool bInP ) const
    {   return m_aSet.GetEmphasisMark(bInP); }
inline const SvxTwoLinesItem       &SwFormat::Get2Lines( bool bInP ) const
    {   return m_aSet.Get2Lines(bInP); }
inline const SvxCharScaleWidthItem &SwFormat::GetCharScaleW( bool bInP ) const
    {   return m_aSet.GetCharScaleW(bInP); }
inline const SvxCharRotateItem     &SwFormat::GetCharRotate( bool bInP ) const
    {   return m_aSet.GetCharRotate(bInP); }
inline const SvxCharReliefItem     &SwFormat::GetCharRelief( bool bInP ) const
    {   return m_aSet.GetCharRelief(bInP); }
inline const SvxCharHiddenItem      &SwFormat::GetCharHidden( bool bInP ) const
    {   return m_aSet.GetCharHidden(bInP); }
inline const SvxBoxItem     &SwFormat::GetCharBorder( bool bInP ) const
    {   return m_aSet.GetCharBorder(bInP); }
inline const SvxShadowItem  &SwFormat::GetCharShadow( bool bInP ) const
    {   return m_aSet.GetCharShadow(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
