/*************************************************************************
 *
 *  $RCSfile: charatr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-02 17:26:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHARATR_HXX
#define _CHARATR_HXX

#ifndef _FORMAT_HXX
#include <format.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>      // fuer die WhichIds
#endif


/******************************************************************************
 *  Implementierung der Charakter-Attribut Methoden vom SwAttrSet
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
inline const SvxFontItem         &SwAttrSet::GetFont(BOOL bInP) const
    {   return (const SvxFontItem&)Get( RES_CHRATR_FONT,bInP); }
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
inline const SvxCaseMapItem      &SwAttrSet::GetCaseMap(BOOL bInP) const
    {   return (const SvxCaseMapItem&)Get( RES_CHRATR_CASEMAP,bInP); }
inline const SvxNoHyphenItem     &SwAttrSet::GetNoHyphenHere(BOOL bInP) const
    {   return (const SvxNoHyphenItem&)Get( RES_CHRATR_NOHYPHEN,bInP); }
inline const SvxBlinkItem  &SwAttrSet::GetBlink(BOOL bInP) const
    {   return (const SvxBlinkItem&)Get( RES_CHRATR_BLINK,bInP); }
inline const SvxBrushItem &SwAttrSet::GetChrBackground( BOOL bInP ) const
    {   return (const SvxBrushItem&)Get( RES_CHRATR_BACKGROUND, bInP ); }
inline const SvxEmphasisMarkItem &SwAttrSet::GetEmphasisMark( BOOL bInP ) const
    {   return (const SvxEmphasisMarkItem&)Get( RES_CHRATR_EMPHASIS_MARK, bInP ); }
inline const SwFmt2Lines        &SwAttrSet::Get2Lines( BOOL bInP ) const
    {   return (const SwFmt2Lines&)Get( RES_CHRATR_TWO_LINES, bInP ); }

/******************************************************************************
 *  Implementierung der Charakter-Attribut Methoden vom SwFmt
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
inline const SvxFontItem             &SwFmt::GetFont(BOOL bInP) const
    {   return aSet.GetFont(bInP); }
inline const SvxColorItem            &SwFmt::GetColor(BOOL bInP) const
    {   return aSet.GetColor(bInP); }
inline const SvxCharSetColorItem     &SwFmt::GetCharSetColor(BOOL bInP) const
    {   return aSet.GetCharSetColor(bInP); }
inline const SvxLanguageItem         &SwFmt::GetLanguage(BOOL bInP) const
    {   return aSet.GetLanguage(bInP); }
inline const SvxEscapementItem       &SwFmt::GetEscapement(BOOL bInP) const
    {   return aSet.GetEscapement(bInP); }
inline const SvxCaseMapItem          &SwFmt::GetCaseMap(BOOL bInP) const
    {   return aSet.GetCaseMap(bInP); }
inline const SvxNoHyphenItem         &SwFmt::GetNoHyphenHere(BOOL bInP) const
    {   return aSet.GetNoHyphenHere(bInP); }
inline const SvxBlinkItem            &SwFmt::GetBlink(BOOL bInP) const
    {   return aSet.GetBlink(bInP); }
inline const SvxBrushItem            &SwFmt::GetChrBackground(BOOL bInP) const
    {   return aSet.GetChrBackground(bInP); }
inline const SvxEmphasisMarkItem    &SwFmt::GetEmphasisMark( BOOL bInP ) const
    {   return aSet.GetEmphasisMark(bInP); }
inline const SwFmt2Lines            &SwFmt::Get2Lines( BOOL bInP ) const
    {   return aSet.Get2Lines(bInP); }

#endif
