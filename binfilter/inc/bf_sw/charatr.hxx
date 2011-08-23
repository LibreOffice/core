/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHARATR_HXX
#define _CHARATR_HXX

#ifndef _FORMAT_HXX
#include <format.hxx>
#endif
namespace binfilter {


/******************************************************************************
 *	Implementierung der Charakter-Attribut Methoden vom SwAttrSet
 *  AMA 12.10.94: Umstellung auf SvxItems.
 ******************************************************************************/

inline const SvxPostureItem      &SwAttrSet::GetPosture(BOOL bInP) const
    {   return (const SvxPostureItem&)Get( RES_CHRATR_POSTURE,bInP); }
inline const SvxPostureItem      &SwAttrSet::GetCJKPosture(BOOL bInP) const
    {   return (const SvxPostureItem&)Get( RES_CHRATR_CJK_POSTURE,bInP); }
inline const SvxPostureItem      &SwAttrSet::GetCTLPosture(BOOL bInP) const
    {   return (const SvxPostureItem&)Get( RES_CHRATR_CTL_POSTURE,bInP); }
inline const SvxWeightItem       &SwAttrSet::GetWeight(BOOL bInP) const
    {   return (const SvxWeightItem&)Get( RES_CHRATR_WEIGHT,bInP); }
inline const SvxWeightItem       &SwAttrSet::GetCJKWeight(BOOL bInP) const
    {   return (const SvxWeightItem&)Get( RES_CHRATR_CJK_WEIGHT,bInP); }
inline const SvxWeightItem       &SwAttrSet::GetCTLWeight(BOOL bInP) const
    {   return (const SvxWeightItem&)Get( RES_CHRATR_CTL_WEIGHT,bInP); }
inline const SvxShadowedItem     &SwAttrSet::GetShadowed(BOOL bInP) const
    {   return (const SvxShadowedItem&)Get( RES_CHRATR_SHADOWED,bInP); }
inline const SvxAutoKernItem     &SwAttrSet::GetAutoKern(BOOL bInP) const
    {   return (const SvxAutoKernItem&)Get( RES_CHRATR_AUTOKERN,bInP); }
inline const SvxWordLineModeItem     &SwAttrSet::GetWordLineMode(BOOL bInP) const
    {   return (const SvxWordLineModeItem&)Get( RES_CHRATR_WORDLINEMODE,bInP); }
inline const SvxContourItem      &SwAttrSet::GetContour(BOOL bInP) const
    {   return (const SvxContourItem&)Get( RES_CHRATR_CONTOUR,bInP); }
inline const SvxKerningItem      &SwAttrSet::GetKerning(BOOL bInP) const
    {   return (const SvxKerningItem&)Get( RES_CHRATR_KERNING,bInP); }
inline const SvxUnderlineItem    &SwAttrSet::GetUnderline(BOOL bInP) const
    {   return (const SvxUnderlineItem&)Get( RES_CHRATR_UNDERLINE,bInP); }
inline const SvxCrossedOutItem   &SwAttrSet::GetCrossedOut(BOOL bInP) const
    {   return (const SvxCrossedOutItem&)Get( RES_CHRATR_CROSSEDOUT,bInP); }
inline const SvxFontHeightItem         &SwAttrSet::GetSize(BOOL bInP) const
    {   return (const SvxFontHeightItem&)Get( RES_CHRATR_FONTSIZE,bInP); }
inline const SvxFontHeightItem         &SwAttrSet::GetCJKSize(BOOL bInP) const
    {   return (const SvxFontHeightItem&)Get( RES_CHRATR_CJK_FONTSIZE,bInP); }
inline const SvxFontHeightItem         &SwAttrSet::GetCTLSize(BOOL bInP) const
    {   return (const SvxFontHeightItem&)Get( RES_CHRATR_CTL_FONTSIZE,bInP); }
inline const SvxPropSizeItem     &SwAttrSet::GetPropSize(BOOL bInP) const
    {   return (const SvxPropSizeItem&)Get( RES_CHRATR_PROPORTIONALFONTSIZE,bInP); }

#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SvxFontItem         &SwAttrSet::GetFont(BOOL bInP) const
    {   return (const SvxFontItem&)Get( RES_CHRATR_FONT,bInP); }
#endif
inline const SvxFontItem         &SwAttrSet::GetCJKFont(BOOL bInP) const
    {   return (const SvxFontItem&)Get( RES_CHRATR_CJK_FONT,bInP); }
inline const SvxFontItem         &SwAttrSet::GetCTLFont(BOOL bInP) const
    {   return (const SvxFontItem&)Get( RES_CHRATR_CTL_FONT,bInP); }
inline const SvxColorItem        &SwAttrSet::GetColor(BOOL bInP) const
    {   return (const SvxColorItem&)Get( RES_CHRATR_COLOR,bInP); }
inline const SvxCharSetColorItem &SwAttrSet::GetCharSetColor(BOOL bInP) const
    {   return (const SvxCharSetColorItem&)Get( RES_CHRATR_CHARSETCOLOR,bInP); }
inline const SvxLanguageItem     &SwAttrSet::GetLanguage(BOOL bInP) const
    {   return (const SvxLanguageItem&)Get( RES_CHRATR_LANGUAGE,bInP); }
inline const SvxLanguageItem     &SwAttrSet::GetCJKLanguage(BOOL bInP) const
    {   return (const SvxLanguageItem&)Get( RES_CHRATR_CJK_LANGUAGE,bInP); }
inline const SvxLanguageItem     &SwAttrSet::GetCTLLanguage(BOOL bInP) const
    {   return (const SvxLanguageItem&)Get( RES_CHRATR_CTL_LANGUAGE,bInP); }
inline const SvxEscapementItem   &SwAttrSet::GetEscapement(BOOL bInP) const
    {   return (const SvxEscapementItem&)Get( RES_CHRATR_ESCAPEMENT,bInP); }
inline const SvxCaseMapItem		&SwAttrSet::GetCaseMap(BOOL bInP) const
    {   return (const SvxCaseMapItem&)Get( RES_CHRATR_CASEMAP,bInP); }
inline const SvxNoHyphenItem	&SwAttrSet::GetNoHyphenHere(BOOL bInP) const
    {   return (const SvxNoHyphenItem&)Get( RES_CHRATR_NOHYPHEN,bInP); }
inline const SvxBlinkItem  &SwAttrSet::GetBlink(BOOL bInP) const
    {   return (const SvxBlinkItem&)Get( RES_CHRATR_BLINK,bInP); }
inline const SvxBrushItem &SwAttrSet::GetChrBackground( BOOL bInP ) const
    {   return (const SvxBrushItem&)Get( RES_CHRATR_BACKGROUND, bInP ); }
inline const SvxEmphasisMarkItem &SwAttrSet::GetEmphasisMark( BOOL bInP ) const
    {   return (const SvxEmphasisMarkItem&)Get( RES_CHRATR_EMPHASIS_MARK, bInP ); }
inline const SvxTwoLinesItem	&SwAttrSet::Get2Lines( BOOL bInP ) const
    {   return (const SvxTwoLinesItem&)Get( RES_CHRATR_TWO_LINES, bInP ); }
inline const SvxCharScaleWidthItem	&SwAttrSet::GetCharScaleW( BOOL bInP ) const
    {   return (const SvxCharScaleWidthItem&)Get( RES_CHRATR_SCALEW, bInP ); }
inline const SvxCharRotateItem	&SwAttrSet::GetCharRotate( BOOL bInP ) const
    {   return (const SvxCharRotateItem&)Get( RES_CHRATR_ROTATE, bInP ); }
inline const SvxCharReliefItem	&SwAttrSet::GetCharRelief( BOOL bInP ) const
    {   return (const SvxCharReliefItem&)Get( RES_CHRATR_RELIEF, bInP ); }

/******************************************************************************
 *	Implementierung der Charakter-Attribut Methoden vom SwFmt
 *  AMA: 12.10.94: Umstellung auf SvxItems.
 ******************************************************************************/

inline const SvxPostureItem          &SwFmt::GetPosture(BOOL bInP) const
    {   return aSet.GetPosture(bInP); }
inline const SvxWeightItem           &SwFmt::GetWeight(BOOL bInP) const
    {   return aSet.GetWeight(bInP); }
inline const SvxShadowedItem         &SwFmt::GetShadowed(BOOL bInP) const
    {   return aSet.GetShadowed(bInP); }
inline const SvxAutoKernItem         &SwFmt::GetAutoKern(BOOL bInP) const
    {   return aSet.GetAutoKern(bInP); }
inline const SvxWordLineModeItem     &SwFmt::GetWordLineMode(BOOL bInP) const
    {   return aSet.GetWordLineMode(bInP); }
inline const SvxContourItem          &SwFmt::GetContour(BOOL bInP) const
    {   return aSet.GetContour(bInP); }
inline const SvxKerningItem          &SwFmt::GetKerning(BOOL bInP) const
    {   return aSet.GetKerning(bInP); }
inline const SvxUnderlineItem        &SwFmt::GetUnderline(BOOL bInP) const
    {   return aSet.GetUnderline(bInP); }
inline const SvxCrossedOutItem       &SwFmt::GetCrossedOut(BOOL bInP) const
    {   return aSet.GetCrossedOut(bInP); }
inline const SvxFontHeightItem             &SwFmt::GetSize(BOOL bInP) const
    {   return aSet.GetSize(bInP); }
inline const SvxPropSizeItem         &SwFmt::GetPropSize(BOOL bInP) const
    {   return aSet.GetPropSize(bInP); }
#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SvxFontItem             &SwFmt::GetFont(BOOL bInP) const
    {   return aSet.GetFont(bInP); }
#endif
inline const SvxFontItem         &SwFmt::GetCJKFont(BOOL bInP) const
    {   return aSet.GetCJKFont(bInP); }
inline const SvxFontItem         &SwFmt::GetCTLFont(BOOL bInP) const
    {   return aSet.GetCTLFont(bInP); }
inline const SvxColorItem            &SwFmt::GetColor(BOOL bInP) const
    {   return aSet.GetColor(bInP); }
inline const SvxCharSetColorItem	 &SwFmt::GetCharSetColor(BOOL bInP) const
    {   return aSet.GetCharSetColor(bInP); }
inline const SvxLanguageItem         &SwFmt::GetLanguage(BOOL bInP) const
    {   return aSet.GetLanguage(bInP); }
inline const SvxEscapementItem       &SwFmt::GetEscapement(BOOL bInP) const
    {   return aSet.GetEscapement(bInP); }
inline const SvxCaseMapItem          &SwFmt::GetCaseMap(BOOL bInP) const
    {   return aSet.GetCaseMap(bInP); }
inline const SvxNoHyphenItem	     &SwFmt::GetNoHyphenHere(BOOL bInP) const
    {   return aSet.GetNoHyphenHere(bInP); }
inline const SvxBlinkItem		 	 &SwFmt::GetBlink(BOOL bInP) const
    {   return aSet.GetBlink(bInP); }
inline const SvxBrushItem		 	 &SwFmt::GetChrBackground(BOOL bInP) const
    {   return aSet.GetChrBackground(bInP); }
inline const SvxEmphasisMarkItem	&SwFmt::GetEmphasisMark( BOOL bInP ) const
    {   return aSet.GetEmphasisMark(bInP); }
inline const SvxTwoLinesItem		&SwFmt::Get2Lines( BOOL bInP ) const
    {   return aSet.Get2Lines(bInP); }
inline const SvxCharScaleWidthItem	&SwFmt::GetCharScaleW( BOOL bInP ) const
    {   return aSet.GetCharScaleW(bInP); }
inline const SvxCharRotateItem		&SwFmt::GetCharRotate( BOOL bInP ) const
    {   return aSet.GetCharRotate(bInP); }
inline const SvxCharReliefItem		&SwFmt::GetCharRelief( BOOL bInP ) const
    {   return aSet.GetCharRelief(bInP); }

} //namespace binfilter
#endif
