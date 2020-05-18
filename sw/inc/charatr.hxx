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

#include "format.hxx"
#include "hintids.hxx"
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
#include <editeng/fontitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/nhypitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/colritem.hxx>

// implementation of the character attribute methods of SwAttrSet

inline const SvxPostureItem      &SwAttrSet::GetPosture(bool bInP) const
    {   return Get( RES_CHRATR_POSTURE,bInP); }
inline const SvxPostureItem      &SwAttrSet::GetCJKPosture(bool bInP) const
    {   return Get( RES_CHRATR_CJK_POSTURE,bInP); }
inline const SvxPostureItem      &SwAttrSet::GetCTLPosture(bool bInP) const
    {   return Get( RES_CHRATR_CTL_POSTURE,bInP); }
inline const SvxWeightItem       &SwAttrSet::GetWeight(bool bInP) const
    {   return Get( RES_CHRATR_WEIGHT,bInP); }
inline const SvxWeightItem       &SwAttrSet::GetCJKWeight(bool bInP) const
    {   return Get( RES_CHRATR_CJK_WEIGHT,bInP); }
inline const SvxWeightItem       &SwAttrSet::GetCTLWeight(bool bInP) const
    {   return Get( RES_CHRATR_CTL_WEIGHT,bInP); }
inline const SvxShadowedItem     &SwAttrSet::GetShadowed(bool bInP) const
    {   return Get( RES_CHRATR_SHADOWED,bInP); }
inline const SvxAutoKernItem     &SwAttrSet::GetAutoKern(bool bInP) const
    {   return Get( RES_CHRATR_AUTOKERN,bInP); }
inline const SvxWordLineModeItem &SwAttrSet::GetWordLineMode(bool bInP) const
    {   return Get( RES_CHRATR_WORDLINEMODE,bInP); }
inline const SvxContourItem      &SwAttrSet::GetContour(bool bInP) const
    {   return Get( RES_CHRATR_CONTOUR,bInP); }
inline const SvxKerningItem      &SwAttrSet::GetKerning(bool bInP) const
    {   return Get( RES_CHRATR_KERNING,bInP); }
inline const SvxUnderlineItem    &SwAttrSet::GetUnderline(bool bInP) const
    {   return Get( RES_CHRATR_UNDERLINE,bInP); }
inline const SvxOverlineItem     &SwAttrSet::GetOverline(bool bInP) const
    {   return Get( RES_CHRATR_OVERLINE,bInP); }
inline const SvxCrossedOutItem   &SwAttrSet::GetCrossedOut(bool bInP) const
    {   return Get( RES_CHRATR_CROSSEDOUT,bInP); }
inline const SvxFontHeightItem   &SwAttrSet::GetSize(bool bInP) const
    {   return Get( RES_CHRATR_FONTSIZE,bInP); }
inline const SvxFontHeightItem   &SwAttrSet::GetCJKSize(bool bInP) const
    {   return Get( RES_CHRATR_CJK_FONTSIZE,bInP); }
inline const SvxFontHeightItem   &SwAttrSet::GetCTLSize(bool bInP) const
    {   return Get( RES_CHRATR_CTL_FONTSIZE,bInP); }
inline const SvxFontItem         &SwAttrSet::GetFont(bool bInP) const
    {   return Get( RES_CHRATR_FONT,bInP); }
inline const SvxFontItem         &SwAttrSet::GetCJKFont(bool bInP) const
    {   return Get( RES_CHRATR_CJK_FONT,bInP); }
inline const SvxFontItem         &SwAttrSet::GetCTLFont(bool bInP) const
    {   return Get( RES_CHRATR_CTL_FONT,bInP); }
inline const SvxColorItem        &SwAttrSet::GetColor(bool bInP) const
    {   return Get( RES_CHRATR_COLOR,bInP); }
inline const SvxLanguageItem     &SwAttrSet::GetLanguage(bool bInP) const
    {   return Get( RES_CHRATR_LANGUAGE,bInP); }
inline const SvxLanguageItem     &SwAttrSet::GetCJKLanguage(bool bInP) const
    {   return Get( RES_CHRATR_CJK_LANGUAGE,bInP); }
inline const SvxLanguageItem     &SwAttrSet::GetCTLLanguage(bool bInP) const
    {   return Get( RES_CHRATR_CTL_LANGUAGE,bInP); }
inline const SvxEscapementItem   &SwAttrSet::GetEscapement(bool bInP) const
    {   return Get( RES_CHRATR_ESCAPEMENT,bInP); }
inline const SvxCaseMapItem      &SwAttrSet::GetCaseMap(bool bInP) const
    {   return Get( RES_CHRATR_CASEMAP,bInP); }
inline const SvxNoHyphenItem     &SwAttrSet::GetNoHyphenHere(bool bInP) const
    {   return Get( RES_CHRATR_NOHYPHEN,bInP); }
inline const SvxEmphasisMarkItem &SwAttrSet::GetEmphasisMark( bool bInP ) const
    {   return Get( RES_CHRATR_EMPHASIS_MARK, bInP ); }
inline const SvxTwoLinesItem     &SwAttrSet::Get2Lines( bool bInP ) const
    {   return Get( RES_CHRATR_TWO_LINES, bInP ); }
inline const SvxCharScaleWidthItem  &SwAttrSet::GetCharScaleW( bool bInP ) const
    {   return Get( RES_CHRATR_SCALEW, bInP ); }
inline const SvxCharRotateItem   &SwAttrSet::GetCharRotate( bool bInP ) const
    {   return Get( RES_CHRATR_ROTATE, bInP ); }
inline const SvxCharReliefItem   &SwAttrSet::GetCharRelief( bool bInP ) const
    {   return Get( RES_CHRATR_RELIEF, bInP ); }
inline const SvxCharHiddenItem   &SwAttrSet::GetCharHidden( bool bInP ) const
    {   return Get( RES_CHRATR_HIDDEN, bInP ); }

// implementation of the character attribute methods of SwFormat

inline const SvxUnderlineItem        &SwFormat::GetUnderline(bool bInP) const
    {   return m_aSet.GetUnderline(bInP); }
inline const SvxFontHeightItem             &SwFormat::GetSize(bool bInP) const
    {   return m_aSet.GetSize(bInP); }
inline const SvxFontItem             &SwFormat::GetFont(bool bInP) const
    {   return m_aSet.GetFont(bInP); }
inline const SvxFontItem         &SwFormat::GetCJKFont(bool bInP) const
    {   return m_aSet.GetCJKFont(bInP); }
inline const SvxFontItem         &SwFormat::GetCTLFont(bool bInP) const
    {   return m_aSet.GetCTLFont(bInP); }
inline const SvxColorItem            &SwFormat::GetColor(bool bInP) const
    {   return m_aSet.GetColor(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
