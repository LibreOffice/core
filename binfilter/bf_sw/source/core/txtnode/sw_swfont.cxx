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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <bf_svx/brshitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX //autogen
#include <bf_svx/wrlmitem.hxx>
#endif
#ifndef _SVX_BLNKITEM_HXX //autogen
#include <bf_svx/blnkitem.hxx>
#endif
#ifndef _SVX_NHYPITEM_HXX //autogen
#include <bf_svx/nhypitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <bf_svx/kernitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <bf_svx/cmapitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <bf_svx/langitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <bf_svx/escpitem.hxx>
#endif
#ifndef _SVX_AKRNITEM_HXX //autogen
#include <bf_svx/akrnitem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <bf_svx/shdditem.hxx>
#endif
#ifndef _SVX_CHARRELIEFITEM_HXX
#include <bf_svx/charreliefitem.hxx>
#endif
#ifndef _SVX_CNTRITEM_HXX //autogen
#include <bf_svx/cntritem.hxx>
#endif
#ifndef _SVX_CSCOITEM_HXX //autogen
#include <bf_svx/cscoitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <bf_svx/crsditem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <bf_svx/udlnitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <bf_svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <bf_svx/postitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <bf_svx/fhgtitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <bf_svx/fontitem.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX //autogen
#include <bf_svx/emphitem.hxx>
#endif
#ifndef _SVX_CHARSCALEITEM_HXX
#include <bf_svx/charscaleitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <bf_svx/charrotateitem.hxx>
#endif
#ifndef _SVX_TWOLINESITEM_HXX
#include <bf_svx/twolinesitem.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>		// Bildschirmabgleich
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>     // FontCache
#endif
namespace binfilter {

#if defined(WIN) || defined(WNT) || defined(PM2)
#define FNT_LEADING_HACK
#endif

#if defined(WIN) || defined(WNT)
#define FNT_ATM_HACK
#endif

#ifdef DBG_UTIL
// globale Variable
SvStatistics aSvStat;
#endif

using namespace ::com::sun::star::i18n::ScriptType;

/************************************************************************
 * Hintergrundbrush setzen, z.B. bei Zeichenvorlagen
 ***********************************************************************/

/*N*/ void SwFont::SetBackColor( Color* pNewColor )
/*N*/ {
/*N*/ 	delete pBackColor;
/*N*/ 	pBackColor = pNewColor;
/*N*/ 	bFntChg = TRUE;
/*N*/ 	aSub[SW_LATIN].pMagic = aSub[SW_CJK].pMagic = aSub[SW_CTL].pMagic = 0;
/*N*/ }

/************************************************************************
 * Hintergrundbrush setzen,
 * die alte Brush wird _nicht_ destruiert, sondern ist der Rueckgabewert.
 ***********************************************************************/



// maps directions for vertical layout
/*N*/ USHORT MapDirection( USHORT nDir, const BOOL bVertFormat )
/*N*/ {
/*N*/     if ( bVertFormat )
/*N*/     {
/*N*/         switch ( nDir )
/*N*/         {
/*N*/         case 0 :
/*N*/             nDir = 2700;
/*N*/             break;
/*N*/         case 900 :
/*N*/             nDir = 0;
/*N*/             break;
/*N*/         case 2700 :
/*N*/             nDir = 1800;
/*N*/             break;
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/         default :
/*N*/             ASSERT( sal_False, "Unsupported direction" );
/*N*/             break;
/*N*/ #endif
/*N*/         }
/*N*/     }
/*N*/     return nDir;
/*N*/ }

// maps the absolute direction set at the font to its logical conterpart
// in the rotated environment
/*N*/ USHORT UnMapDirection( USHORT nDir, const BOOL bVertFormat )
/*N*/ {
/*N*/     if ( bVertFormat )
/*N*/     {
/*?*/         switch ( nDir )
/*?*/         {
/*?*/         case 0 :
/*?*/             nDir = 900;
/*?*/             break;
/*?*/         case 1800 :
/*?*/             nDir = 2700;
/*?*/             break;
/*?*/         case 2700 :
/*?*/             nDir = 0;
/*?*/             break;
/*?*/ #if OSL_DEBUG_LEVEL > 1
/*?*/         default :
/*?*/             ASSERT( sal_False, "Unsupported direction" );
/*?*/             break;
/*?*/ #endif
/*?*/         }
/*N*/     }
/*N*/     return nDir;
/*N*/ }

/*N*/ USHORT SwFont::GetOrientation( const BOOL bVertFormat ) const
/*N*/ {
/*N*/     return UnMapDirection( aSub[nActual].GetOrientation(), bVertFormat );
/*N*/ }

/*N*/ void SwFont::SetVertical( USHORT nDir, const BOOL bVertFormat )
/*N*/ {
/*N*/     // map direction if frame has vertical layout
/*N*/     nDir = MapDirection( nDir, bVertFormat );
/*N*/ 
/*N*/     if( nDir != aSub[0].GetOrientation() )
/*N*/ 	{
/*N*/ 		bFntChg = TRUE;
/*N*/ 		aSub[0].SetVertical( nDir, bVertFormat );
/*N*/         aSub[1].SetVertical( nDir, bVertFormat || nDir > 1000 );
/*N*/ 		aSub[2].SetVertical( nDir, bVertFormat );
/*N*/ 	}
/*N*/ }

/*************************************************************************
 Escapement:
    frEsc:  Fraction, Grad des Escapements
    Esc = resultierendes Escapement
    A1 = Original-Ascent			(nOrgAscent)
    A2 = verkleinerter Ascent		(nEscAscent)
    Ax = resultierender Ascent		(GetAscent())
    H1 = Original-Hoehe 			(nOrgHeight)
    H2 = verkleinerter Hoehe		(nEscHeight)
    Hx = resultierender Hoehe		(GetHeight())
    Bx = resultierende Baseline fuer die Textausgabe (CalcPos())
         (Vorsicht: Y - A1!)

    Escapement:
        Esc = H1 * frEsc;

    Hochstellung:
        Ax = A2 + Esc;
        Hx = H2 + Esc;
        Bx = A1 - Esc;

    Tiefstellung:
        Ax = A1;
        Hx = A1 + Esc + (H2 - A2);
        Bx = A1 + Esc;

*************************************************************************/

/*************************************************************************
 *					SwSubFont::CalcEscAscent( const USHORT nOldAscent )
 *************************************************************************/

// nEsc ist der Prozentwert
/*N*/ USHORT SwSubFont::CalcEscAscent( const USHORT nOldAscent ) const
/*N*/ {
/*N*/ 	if( DFLT_ESC_AUTO_SUPER != GetEscapement() &&
/*N*/ 		DFLT_ESC_AUTO_SUB != GetEscapement() )
/*N*/ 	{
/*N*/ 		const long nAscent = nOldAscent +
/*N*/ 							 ( (long) nOrgHeight * GetEscapement() ) / 100L;
/*N*/ 		if ( nAscent>0 )
/*N*/ 			return ( Max( USHORT (nAscent), nOrgAscent ));
/*N*/ 	}
/*N*/ 	return nOrgAscent;
/*N*/ }

/*************************************************************************
 *                      SwFont::SetDiffFnt()
 *************************************************************************/

/*N*/ void SwFont::SetDiffFnt( const SfxItemSet *pAttrSet, const SwDoc *pDoc )
/*N*/ {
/*N*/ 	delete pBackColor;
/*N*/ 	pBackColor = NULL;
/*N*/ 
/*N*/ 
/*N*/     if( pAttrSet )
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pItem;
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_FONT,
/*N*/ 			TRUE, &pItem ))
/*N*/ 		{
/*N*/ 			const SvxFontItem *pFont = (const SvxFontItem *)pItem;
/*N*/ 			aSub[SW_LATIN].SetFamily( pFont->GetFamily() );
/*N*/ 			aSub[SW_LATIN].Font::SetName( pFont->GetFamilyName() );
/*N*/ 			aSub[SW_LATIN].Font::SetStyleName( pFont->GetStyleName() );
/*N*/ 			aSub[SW_LATIN].Font::SetPitch( pFont->GetPitch() );
/*N*/ 			aSub[SW_LATIN].Font::SetCharSet( pFont->GetCharSet() );
/*N*/ 		}
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_FONTSIZE,
/*N*/ 			TRUE, &pItem ))
/*N*/ 		{
/*N*/ 			const SvxFontHeightItem *pHeight = (const SvxFontHeightItem *)pItem;
/*N*/ 			aSub[SW_LATIN].SvxFont::SetPropr( 100 );
/*N*/ 			aSub[SW_LATIN].aSize = aSub[SW_LATIN].Font::GetSize();
/*N*/ 			Size aTmpSize = aSub[SW_LATIN].aSize;
/*N*/ 			aTmpSize.Height() = pHeight->GetHeight();
/*N*/ 			aSub[SW_LATIN].SetSize( aTmpSize );
/*N*/ 		}
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_POSTURE,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			aSub[SW_LATIN].Font::SetItalic( ((SvxPostureItem*)pItem)->GetPosture() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_WEIGHT,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			aSub[SW_LATIN].Font::SetWeight( ((SvxWeightItem*)pItem)->GetWeight() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_LANGUAGE,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			aSub[SW_LATIN].SetLanguage( ((SvxLanguageItem*)pItem)->GetLanguage() );
/*N*/ 
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CJK_FONT,
/*N*/ 			TRUE, &pItem ))
/*N*/ 		{
/*?*/ 			const SvxFontItem *pFont = (const SvxFontItem *)pItem;
/*?*/ 			aSub[SW_CJK].SetFamily( pFont->GetFamily() );
/*?*/ 			aSub[SW_CJK].Font::SetName( pFont->GetFamilyName() );
/*?*/ 			aSub[SW_CJK].Font::SetStyleName( pFont->GetStyleName() );
/*?*/ 			aSub[SW_CJK].Font::SetPitch( pFont->GetPitch() );
/*?*/ 			aSub[SW_CJK].Font::SetCharSet( pFont->GetCharSet() );
/*N*/ 		}
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CJK_FONTSIZE,
/*N*/ 			TRUE, &pItem ))
/*N*/ 		{
/*?*/ 			const SvxFontHeightItem *pHeight = (const SvxFontHeightItem *)pItem;
/*?*/ 			aSub[SW_CJK].SvxFont::SetPropr( 100 );
/*?*/ 			aSub[SW_CJK].aSize = aSub[SW_CJK].Font::GetSize();
/*?*/ 			Size aTmpSize = aSub[SW_CJK].aSize;
/*?*/ 			aTmpSize.Height() = pHeight->GetHeight();
/*?*/ 			aSub[SW_CJK].SetSize( aTmpSize );
/*N*/ 		}
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CJK_POSTURE,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			aSub[SW_CJK].Font::SetItalic( ((SvxPostureItem*)pItem)->GetPosture() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CJK_WEIGHT,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			aSub[SW_CJK].Font::SetWeight( ((SvxWeightItem*)pItem)->GetWeight() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CJK_LANGUAGE,
/*N*/ 			TRUE, &pItem ))
/*N*/ 		{
/*?*/ 			LanguageType eNewLang = ((SvxLanguageItem*)pItem)->GetLanguage();
/*?*/ 			aSub[SW_CJK].SetLanguage( eNewLang );
/*?*/ 			aSub[SW_LATIN].SetCJKContextLanguage( eNewLang );
/*?*/ 			aSub[SW_CJK].SetCJKContextLanguage( eNewLang );
/*?*/ 			aSub[SW_CTL].SetCJKContextLanguage( eNewLang );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CTL_FONT,
/*N*/ 			TRUE, &pItem ))
/*N*/ 		{
/*?*/ 			const SvxFontItem *pFont = (const SvxFontItem *)pItem;
/*?*/ 			aSub[SW_CTL].SetFamily( pFont->GetFamily() );
/*?*/ 			aSub[SW_CTL].Font::SetName( pFont->GetFamilyName() );
/*?*/ 			aSub[SW_CTL].Font::SetStyleName( pFont->GetStyleName() );
/*?*/ 			aSub[SW_CTL].Font::SetPitch( pFont->GetPitch() );
/*?*/ 			aSub[SW_CTL].Font::SetCharSet( pFont->GetCharSet() );
/*N*/ 		}
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CTL_FONTSIZE,
/*N*/ 			TRUE, &pItem ))
/*N*/ 		{
/*?*/ 			const SvxFontHeightItem *pHeight = (const SvxFontHeightItem *)pItem;
/*?*/ 			aSub[SW_CTL].SvxFont::SetPropr( 100 );
/*?*/ 			aSub[SW_CTL].aSize = aSub[SW_CTL].Font::GetSize();
/*?*/ 			Size aTmpSize = aSub[SW_CTL].aSize;
/*?*/ 			aTmpSize.Height() = pHeight->GetHeight();
/*?*/ 			aSub[SW_CTL].SetSize( aTmpSize );
/*N*/ 		}
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CTL_POSTURE,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			aSub[SW_CTL].Font::SetItalic( ((SvxPostureItem*)pItem)->GetPosture() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CTL_WEIGHT,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			aSub[SW_CTL].Font::SetWeight( ((SvxWeightItem*)pItem)->GetWeight() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CTL_LANGUAGE,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			aSub[SW_CTL].SetLanguage( ((SvxLanguageItem*)pItem)->GetLanguage() );
/*N*/ 
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_UNDERLINE,
/*N*/ 			TRUE, &pItem ))
/*N*/ 		{
/*N*/ 			SetUnderline( ((SvxUnderlineItem*)pItem)->GetUnderline() );
/*N*/ 			SetUnderColor( ((SvxUnderlineItem*)pItem)->GetColor() );
/*N*/ 		}
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CROSSEDOUT,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			SetStrikeout( ((SvxCrossedOutItem*)pItem)->GetStrikeout() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_COLOR,
/*N*/ 			TRUE, &pItem ))
/*N*/ 			SetColor( ((SvxColorItem*)pItem)->GetValue() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_EMPHASIS_MARK,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			SetEmphasisMark( ((SvxEmphasisMarkItem*)pItem)->GetEmphasisMark() );
/*N*/ 
/*N*/ 		SetTransparent( TRUE );
/*N*/ 		SetAlign( ALIGN_BASELINE );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CONTOUR,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			SetOutline( ((SvxContourItem*)pItem)->GetValue() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_SHADOWED,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			SetShadow( ((SvxShadowedItem*)pItem)->GetValue() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_RELIEF,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			SetRelief( (FontRelief)((SvxCharReliefItem*)pItem)->GetValue() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_SHADOWED,
/*N*/ 			TRUE, &pItem ))
/*N*/ 			SetPropWidth(((SvxShadowedItem*)pItem)->GetValue() ? 50 : 100 );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_AUTOKERN,
/*N*/ 			TRUE, &pItem ))
/*N*/ 		{
/*?*/ 			if( ((SvxAutoKernItem*)pItem)->GetValue() )
/*?*/ 				SetAutoKern( ( !pDoc || !pDoc->IsKernAsianPunctuation() ) ?
/*?*/ 						 KERNING_FONTSPECIFIC : KERNING_ASIAN );
/*?*/ 			else
/*?*/     			SetAutoKern( 0 );
/*N*/ 		}
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_WORDLINEMODE,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			SetWordLineMode( ((SvxWordLineModeItem*)pItem)->GetValue() );
/*N*/ 
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_ESCAPEMENT,
/*N*/ 			TRUE, &pItem ))
/*N*/ 		{
/*N*/ 			const SvxEscapementItem *pEsc = (const SvxEscapementItem *)pItem;
/*N*/ 			SetEscapement( pEsc->GetEsc() );
/*N*/ 			if( aSub[SW_LATIN].IsEsc() )
/*N*/ 				SetProportion( pEsc->GetProp() );
/*N*/ 		}
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_CASEMAP,
/*N*/ 			TRUE, &pItem ))
/*N*/ 			SetCaseMap( ((SvxCaseMapItem*)pItem)->GetCaseMap() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_KERNING,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			SetFixKerning( ((SvxKerningItem*)pItem)->GetValue() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_NOHYPHEN,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			SetNoHyph( ((SvxNoHyphenItem*)pItem)->GetValue() );
/*N*/ 		if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_BLINK,
/*N*/ 			TRUE, &pItem ))
/*?*/ 			SetBlink( ((SvxBlinkItem*)pItem)->GetValue() );
/*N*/         if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_ROTATE,
/*N*/             TRUE, &pItem ))
/*?*/             SetVertical( ((SvxCharRotateItem*)pItem)->GetValue() );
/*N*/         if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_BACKGROUND,
/*N*/             TRUE, &pItem ))
/*N*/             pBackColor = new Color( ((SvxBrushItem*)pItem)->GetColor() );
/*N*/         else
/*N*/             pBackColor = NULL;
/*N*/         const SfxPoolItem* pTwoLinesItem = 0;
/*N*/         if( SFX_ITEM_SET ==
/*N*/                 pAttrSet->GetItemState( RES_CHRATR_TWO_LINES, TRUE, &pTwoLinesItem ))
/*?*/             if ( ((SvxTwoLinesItem*)pTwoLinesItem)->GetValue() )
/*?*/                 SetVertical( 0 );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		Invalidate();
/*?*/ 		bNoHyph = FALSE;
/*?*/ 		bBlink = FALSE;
/*N*/ 	}
/*N*/ 	bPaintBlank = FALSE;
/*N*/ 	bPaintWrong = FALSE;
/*N*/ 	ASSERT( aSub[SW_LATIN].IsTransparent(), "SwFont: Transparent revolution" );
/*N*/ }

/*************************************************************************
 *						class SwFont
 *************************************************************************/

/*N*/ SwFont::SwFont( const SwFont &rFont )
/*N*/ {
/*N*/ 	aSub[SW_LATIN] = rFont.aSub[SW_LATIN];
/*N*/ 	aSub[SW_CJK] = rFont.aSub[SW_CJK];
/*N*/ 	aSub[SW_CTL] = rFont.aSub[SW_CTL];
/*N*/     nActual = rFont.nActual;
/*N*/ 	pBackColor = rFont.pBackColor ? new Color( *rFont.pBackColor ) : NULL;
/*N*/ 	aUnderColor = rFont.GetUnderColor();
/*N*/ 	nToxCnt = nRefCnt = 0;
/*N*/ 	bFntChg = rFont.bFntChg;
/*N*/ 	bOrgChg = rFont.bOrgChg;
/*N*/ 	bPaintBlank = rFont.bPaintBlank;
/*N*/ 	bPaintWrong = FALSE;
/*N*/ 	bURL = rFont.bURL;
/*N*/ 	bGreyWave = rFont.bGreyWave;
/*N*/ 	bNoColReplace = rFont.bNoColReplace;
/*N*/ 	bNoHyph = rFont.bNoHyph;
/*N*/ 	bBlink = rFont.bBlink;
/*N*/ }

/*N*/ SwFont::SwFont( const SwAttrSet* pAttrSet, const SwDoc *pDoc )
/*N*/ {
/*N*/     nActual = SW_LATIN;
/*N*/     nToxCnt = nRefCnt = 0;
/*N*/     bPaintBlank = FALSE;
/*N*/     bPaintWrong = FALSE;
/*N*/     bURL = FALSE;
/*N*/     bGreyWave = FALSE;
/*N*/     bNoColReplace = FALSE;
/*N*/     bNoHyph = pAttrSet->GetNoHyphenHere().GetValue();
/*N*/     bBlink = pAttrSet->GetBlink().GetValue();
/*N*/     {
/*N*/         const SvxFontItem& rFont = pAttrSet->GetFont();
/*N*/         aSub[SW_LATIN].SetFamily( rFont.GetFamily() );
/*N*/         aSub[SW_LATIN].SetName( rFont.GetFamilyName() );
/*N*/         aSub[SW_LATIN].SetStyleName( rFont.GetStyleName() );
/*N*/         aSub[SW_LATIN].SetPitch( rFont.GetPitch() );
/*N*/         aSub[SW_LATIN].SetCharSet( rFont.GetCharSet() );
/*N*/         aSub[SW_LATIN].SvxFont::SetPropr( 100 );   // 100% der FontSize
/*N*/         Size aTmpSize = aSub[SW_LATIN].aSize;
/*N*/         aTmpSize.Height() = pAttrSet->GetSize().GetHeight();
/*N*/         aSub[SW_LATIN].SetSize( aTmpSize );
/*N*/         aSub[SW_LATIN].SetItalic( pAttrSet->GetPosture().GetPosture() );
/*N*/         aSub[SW_LATIN].SetWeight( pAttrSet->GetWeight().GetWeight() );
/*N*/         aSub[SW_LATIN].SetLanguage( pAttrSet->GetLanguage().GetLanguage() );
/*N*/     }
/*N*/ 
/*N*/     {
/*N*/         const SvxFontItem& rFont = pAttrSet->GetCJKFont();
/*N*/         aSub[SW_CJK].SetFamily( rFont.GetFamily() );
/*N*/         aSub[SW_CJK].SetName( rFont.GetFamilyName() );
/*N*/         aSub[SW_CJK].SetStyleName( rFont.GetStyleName() );
/*N*/         aSub[SW_CJK].SetPitch( rFont.GetPitch() );
/*N*/         aSub[SW_CJK].SetCharSet( rFont.GetCharSet() );
/*N*/         aSub[SW_CJK].SvxFont::SetPropr( 100 );   // 100% der FontSize
/*N*/         Size aTmpSize = aSub[SW_CJK].aSize;
/*N*/         aTmpSize.Height() = pAttrSet->GetCJKSize().GetHeight();
/*N*/         aSub[SW_CJK].SetSize( aTmpSize );
/*N*/         aSub[SW_CJK].SetItalic( pAttrSet->GetCJKPosture().GetPosture() );
/*N*/         aSub[SW_CJK].SetWeight( pAttrSet->GetCJKWeight().GetWeight() );
/*N*/         LanguageType eNewLang = pAttrSet->GetCJKLanguage().GetLanguage();
/*N*/         aSub[SW_CJK].SetLanguage( eNewLang );
/*N*/         aSub[SW_LATIN].SetCJKContextLanguage( eNewLang );
/*N*/         aSub[SW_CJK].SetCJKContextLanguage( eNewLang );
/*N*/         aSub[SW_CTL].SetCJKContextLanguage( eNewLang );
/*N*/     }
/*N*/ 
/*N*/     {
/*N*/         const SvxFontItem& rFont = pAttrSet->GetCTLFont();
/*N*/         aSub[SW_CTL].SetFamily( rFont.GetFamily() );
/*N*/         aSub[SW_CTL].SetName( rFont.GetFamilyName() );
/*N*/         aSub[SW_CTL].SetStyleName( rFont.GetStyleName() );
/*N*/         aSub[SW_CTL].SetPitch( rFont.GetPitch() );
/*N*/         aSub[SW_CTL].SetCharSet( rFont.GetCharSet() );
/*N*/         aSub[SW_CTL].SvxFont::SetPropr( 100 );   // 100% der FontSize
/*N*/         Size aTmpSize = aSub[SW_CTL].aSize;
/*N*/         aTmpSize.Height() = pAttrSet->GetCTLSize().GetHeight();
/*N*/         aSub[SW_CTL].SetSize( aTmpSize );
/*N*/         aSub[SW_CTL].SetItalic( pAttrSet->GetCTLPosture().GetPosture() );
/*N*/         aSub[SW_CTL].SetWeight( pAttrSet->GetCTLWeight().GetWeight() );
/*N*/         aSub[SW_CTL].SetLanguage( pAttrSet->GetCTLLanguage().GetLanguage() );
/*N*/     }
/*N*/ 
/*N*/     SetUnderline( pAttrSet->GetUnderline().GetUnderline() );
/*N*/     SetUnderColor( pAttrSet->GetUnderline().GetColor() );
/*N*/     SetEmphasisMark( pAttrSet->GetEmphasisMark().GetEmphasisMark() );
/*N*/     SetStrikeout( pAttrSet->GetCrossedOut().GetStrikeout() );
/*N*/     SetColor( pAttrSet->GetColor().GetValue() );
/*N*/     SetTransparent( TRUE );
/*N*/     SetAlign( ALIGN_BASELINE );
/*N*/     SetOutline( pAttrSet->GetContour().GetValue() );
/*N*/     SetShadow( pAttrSet->GetShadowed().GetValue() );
/*M*/     SetPropWidth( pAttrSet->GetCharScaleW().GetValue() );
/*N*/     SetRelief( (FontRelief)pAttrSet->GetCharRelief().GetValue() );
/*N*/ 	if( pAttrSet->GetAutoKern().GetValue() )
/*?*/ 		SetAutoKern( ( !pDoc || !pDoc->IsKernAsianPunctuation() ) ?
/*?*/ 					 KERNING_FONTSPECIFIC : KERNING_ASIAN );
/*N*/ 	else
/*N*/     	SetAutoKern( 0 );
/*N*/     SetWordLineMode( pAttrSet->GetWordLineMode().GetValue() );
/*N*/     const SvxEscapementItem &rEsc = pAttrSet->GetEscapement();
/*N*/     SetEscapement( rEsc.GetEsc() );
/*N*/     if( aSub[SW_LATIN].IsEsc() )
/*?*/         SetProportion( rEsc.GetProp() );
/*N*/     SetCaseMap( pAttrSet->GetCaseMap().GetCaseMap() );
/*N*/     SetFixKerning( pAttrSet->GetKerning().GetValue() );
/*N*/     const SfxPoolItem* pItem;
/*N*/     if( SFX_ITEM_SET == pAttrSet->GetItemState( RES_CHRATR_BACKGROUND,
/*N*/         TRUE, &pItem ))
/*N*/         pBackColor = new Color( ((SvxBrushItem*)pItem)->GetColor() );
/*N*/     else
/*N*/         pBackColor = NULL;
/*N*/     const SvxTwoLinesItem& rTwoLinesItem = pAttrSet->Get2Lines();
/*N*/     if ( ! rTwoLinesItem.GetValue() )
/*N*/         SetVertical( pAttrSet->GetCharRotate().GetValue() );
/*N*/     else
/*?*/         SetVertical( 0 );
/*N*/ }

/*N*/ SwSubFont& SwSubFont::operator=( const SwSubFont &rFont )
/*N*/ {
/*N*/ 	SvxFont::operator=( rFont );
/*N*/ 	pMagic = rFont.pMagic;
/*N*/ 	nFntIndex = rFont.nFntIndex;
/*N*/ 	nOrgHeight = rFont.nOrgHeight;
/*N*/ 	nOrgAscent = rFont.nOrgAscent;
/*N*/ 	nPropWidth = rFont.nPropWidth;
/*N*/ 	aSize = rFont.aSize;
/*N*/ 	return *this;
/*N*/ }

/*N*/ SwFont& SwFont::operator=( const SwFont &rFont )
/*N*/ {
/*N*/ 	aSub[SW_LATIN] = rFont.aSub[SW_LATIN];
/*N*/ 	aSub[SW_CJK] = rFont.aSub[SW_CJK];
/*N*/ 	aSub[SW_CTL] = rFont.aSub[SW_CTL];
/*N*/     nActual = rFont.nActual;
/*N*/ 	delete pBackColor;
/*N*/ 	pBackColor = rFont.pBackColor ? new Color( *rFont.pBackColor ) : NULL;
/*N*/ 	aUnderColor = rFont.GetUnderColor();
/*N*/ 	nToxCnt = nRefCnt = 0;
/*N*/ 	bFntChg = rFont.bFntChg;
/*N*/ 	bOrgChg = rFont.bOrgChg;
/*N*/ 	bPaintBlank = rFont.bPaintBlank;
/*N*/ 	bPaintWrong = FALSE;
/*N*/ 	bURL = rFont.bURL;
/*N*/ 	bGreyWave = rFont.bGreyWave;
/*N*/ 	bNoColReplace = rFont.bNoColReplace;
/*N*/ 	bNoHyph = rFont.bNoHyph;
/*N*/ 	bBlink = rFont.bBlink;
/*N*/ 	return *this;
/*N*/ }

/*************************************************************************
 *						SwFont::GoMagic()
 *************************************************************************/

/*N*/ void SwFont::GoMagic( ViewShell *pSh, BYTE nWhich )
/*N*/ {
/*N*/ 	SwFntAccess aFntAccess( aSub[nWhich].pMagic, aSub[nWhich].nFntIndex,
/*N*/ 							&aSub[nWhich], pSh, TRUE );
/*N*/ }

/*************************************************************************
 *						SwSubFont::IsSymbol()
 *************************************************************************/

/*N*/ BOOL SwSubFont::IsSymbol( ViewShell *pSh )
/*N*/ {
/*N*/ 	SwFntAccess aFntAccess( pMagic, nFntIndex, this, pSh, FALSE );
/*N*/ 	return aFntAccess.Get()->IsSymbol();
/*N*/ }

/*************************************************************************
 *						SwSubFont::ChgFnt()
 *************************************************************************/

/*N*/ BOOL SwSubFont::ChgFnt( ViewShell *pSh, OutputDevice *pOut )
/*N*/ {
/*N*/ 	if ( pLastFont )
/*N*/ 		pLastFont->Unlock();
/*N*/ 	SwFntAccess aFntAccess( pMagic, nFntIndex, this, pSh, TRUE );
/*N*/ 	SV_STAT( nChangeFont );
/*N*/ 
/*N*/ 	pLastFont = aFntAccess.Get();
/*N*/ 
/*N*/ 	pLastFont->SetDevFont( pSh, pOut );
/*N*/ 
/*N*/ 	pLastFont->Lock();
/*N*/ 	return UNDERLINE_NONE != GetUnderline() || STRIKEOUT_NONE != GetStrikeout();
/*N*/ }

/*************************************************************************
 *					  SwFont::ChgPhysFnt()
 *************************************************************************/

/*N*/ void SwFont::ChgPhysFnt( ViewShell *pSh, OutputDevice *pOut )
/*N*/ {
/*N*/ 	ASSERT( pOut, "SwFont:;ChgPhysFnt, not OutDev." );
/*N*/ 
/*N*/ 	if( bOrgChg && aSub[nActual].IsEsc() )
/*N*/ 	{
/*N*/ 		const BYTE nOldProp = aSub[nActual].GetPropr();
/*N*/         SetProportion( 100 );
/*N*/ 		ChgFnt( pSh, pOut );
/*N*/ 		SwFntAccess aFntAccess( aSub[nActual].pMagic, aSub[nActual].nFntIndex,
/*N*/ 								&aSub[nActual], pSh );
/*N*/ 		aSub[nActual].nOrgHeight = aFntAccess.Get()->GetHeight( pSh, pOut );
/*N*/ 		aSub[nActual].nOrgAscent = aFntAccess.Get()->GetAscent( pSh, pOut );
/*N*/ 		SetProportion( nOldProp );
/*N*/ 		bOrgChg = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bFntChg )
/*N*/ 	{
/*N*/ 		ChgFnt( pSh, pOut );
/*N*/ 		bFntChg = bOrgChg;
/*N*/ 	}
/*N*/ 	if( pOut->GetTextLineColor() != aUnderColor )
/*?*/ 		pOut->SetTextLineColor( aUnderColor );
/*N*/ }

/*************************************************************************
 *						SwFont::CalcEscHeight()
 *         Height = MaxAscent + MaxDescent
 *      MaxAscent = Max (T1_ascent, T2_ascent + (Esc * T1_height) );
 *     MaxDescent = Max (T1_height-T1_ascent,
 * 						 T2_height-T2_ascent - (Esc * T1_height)
 *************************************************************************/

/*N*/ USHORT SwSubFont::CalcEscHeight( const USHORT nOldHeight,
/*N*/ 							  const USHORT nOldAscent  ) const
/*N*/ {
/*N*/ 	if( DFLT_ESC_AUTO_SUPER != GetEscapement() &&
/*N*/ 		DFLT_ESC_AUTO_SUB != GetEscapement() )
/*N*/ 	{
/*N*/ 		long nDescent = nOldHeight - nOldAscent -
/*N*/ 							 ( (long) nOrgHeight * GetEscapement() ) / 100L;
/*N*/ 		const USHORT nDesc = ( nDescent>0 ) ? Max ( USHORT(nDescent),
/*N*/ 				   USHORT(nOrgHeight - nOrgAscent) ) : nOrgHeight - nOrgAscent;
/*N*/ 		return ( nDesc + CalcEscAscent( nOldAscent ) );
/*N*/ 	}
/*N*/ 	return nOrgHeight;
/*N*/ }

/*N*/ short SwSubFont::_CheckKerning( )
/*N*/ {
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	static short nTst = 6;
/*N*/ 	short nKernx = - short( Font::GetSize().Height() / nTst );
/*N*/ #else
/*N*/ 	short nKernx = - short( Font::GetSize().Height() / 6 );
/*N*/ #endif
/*N*/ 	if ( nKernx < GetFixKerning() )
/*N*/ 		return GetFixKerning();
/*N*/ 	return nKernx;
/*N*/ }

/*************************************************************************
 *                    SwSubFont::GetAscent()
 *************************************************************************/

/*N*/ USHORT SwSubFont::GetAscent( ViewShell *pSh, const OutputDevice *pOut )
/*N*/ {
/*N*/ 	register USHORT nAscent;
/*N*/ 	SwFntAccess aFntAccess( pMagic, nFntIndex, this, pSh );
/*N*/ 	nAscent = aFntAccess.Get()->GetAscent( pSh, pOut );
/*N*/ 	if( GetEscapement() )
/*N*/ 		nAscent = CalcEscAscent( nAscent );
/*N*/ 	return nAscent;
/*N*/ }

/*************************************************************************
 *					  SwSubFont::GetHeight()
 *************************************************************************/

/*N*/ USHORT SwSubFont::GetHeight( ViewShell *pSh, const OutputDevice *pOut )
/*N*/ {
/*N*/ 	SV_STAT( nGetTextSize );
/*N*/ 	SwFntAccess aFntAccess( pMagic, nFntIndex, this, pSh );
/*N*/ 	const USHORT nHeight = aFntAccess.Get()->GetHeight( pSh, pOut );
/*N*/ 	if ( GetEscapement() )
/*N*/ 	{
/*N*/ 		const USHORT nAscent = aFntAccess.Get()->GetAscent( pSh, pOut );
/*N*/ 		return CalcEscHeight( nHeight, nAscent ); // + nLeading;
/*N*/ 	}
/*N*/ 	return nHeight; // + nLeading;
/*N*/ }

/*************************************************************************
 *					  SwSubFont::_GetTxtSize()
 *************************************************************************/
/*N*/ Size SwSubFont::_GetTxtSize( SwDrawTextInfo& rInf )
/*N*/ {
/*N*/ 	// Robust: Eigentlich sollte der Font bereits eingestellt sein, aber
/*N*/ 	// sicher ist sicher ...
/*N*/ 	if ( !pLastFont || pLastFont->GetOwner()!=pMagic ||
/*N*/ 		 !IsSameInstance( rInf.GetpOut()->GetFont() ) )
/*N*/ 		ChgFnt( rInf.GetShell(), rInf.GetpOut() );
/*N*/ 
/*N*/ 	Size aTxtSize;
/*N*/ 	xub_StrLen nLn = ( rInf.GetLen() == STRING_LEN ? rInf.GetText().Len()
/*N*/ 												   : rInf.GetLen() );
/*N*/ 	rInf.SetLen( nLn );
/*N*/ 	if( IsCapital() && nLn )
/*N*/ 		aTxtSize = GetCapitalSize( rInf );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SV_STAT( nGetTextSize );
/*N*/ 		short nOldKern = rInf.GetKern();
/*N*/ 		const XubString &rOldTxt = rInf.GetText();
/*N*/ 		rInf.SetKern( CheckKerning() );
/*N*/ 		if ( !IsCaseMap() )
/*N*/ 			aTxtSize = pLastFont->GetTextSize( rInf );
/*N*/ 		else
/*N*/ 		{
/*N*/ 			String aTmp = CalcCaseMap( rInf.GetText() );
/*N*/ 			const XubString &rOldStr = rInf.GetText();
/*N*/ 			sal_Bool bCaseMapLengthDiffers(aTmp.Len() != rOldStr.Len());
/*N*/ 
/*N*/ 			if(bCaseMapLengthDiffers && rInf.GetLen())
/*N*/ 			{
/*N*/ 				// #108203#
/*N*/ 				// If the length of the original string and the CaseMapped one
/*N*/ 				// are different, it is necessary to handle the given text part as
/*N*/ 				// a single snippet since itÄs size may differ, too.
/*N*/ 				xub_StrLen nOldIdx(rInf.GetIdx());
/*N*/ 				xub_StrLen nOldLen(rInf.GetLen());
/*N*/ 				const XubString aSnippet(rOldStr, nOldIdx, nOldLen);
/*N*/ 				XubString aNewText(CalcCaseMap(aSnippet));
/*N*/ 				
/*N*/ 				rInf.SetText( aNewText );
/*N*/ 				rInf.SetIdx( 0 );
/*N*/ 				rInf.SetLen( aNewText.Len() );
/*N*/ 
/*N*/ 				aTxtSize = pLastFont->GetTextSize( rInf );
/*N*/ 
/*N*/ 				rInf.SetIdx( nOldIdx );
/*N*/ 				rInf.SetLen( nOldLen );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				rInf.SetText( aTmp );
/*N*/ 				aTxtSize = pLastFont->GetTextSize( rInf );
/*N*/ 			}
/*N*/ 		
/*N*/ 			rInf.SetText( rOldStr );
/*N*/ 		}
/*N*/ 		rInf.SetKern( nOldKern );
/*N*/ 		rInf.SetText( rOldTxt );
/*N*/ 		// 15142: Ein Wort laenger als eine Zeile, beim Zeilenumbruch
/*N*/ 		//        hochgestellt, muss seine effektive Hoehe melden.
/*N*/ 		if( GetEscapement() )
/*N*/ 		{
/*N*/ 			const USHORT nAscent = pLastFont->GetAscent( rInf.GetShell(),
/*N*/ 														 rInf.GetpOut() );
/*N*/ 			aTxtSize.Height() =
/*N*/ 				(long)CalcEscHeight( (USHORT)aTxtSize.Height(), nAscent);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return aTxtSize;
/*N*/ }

/*************************************************************************
 *					  SwFont::GetTxtBreak()
 *************************************************************************/


/*************************************************************************
 *					  SwFont::GetTxtBreak()
 *************************************************************************/


/*************************************************************************
 *					  SwSubFont::_DrawText()
 *************************************************************************/



/*************************************************************************
 *					  SwSubFont::_GetCrsrOfst()
 *************************************************************************/

/*N*/ xub_StrLen SwSubFont::_GetCrsrOfst( SwDrawTextInfo& rInf )
/*N*/ {
/*N*/ 	if ( !pLastFont || pLastFont->GetOwner()!=pMagic )
/*?*/ 		ChgFnt( rInf.GetShell(), rInf.GetpOut() );
/*N*/ 
/*N*/ 	xub_StrLen nLn = rInf.GetLen() == STRING_LEN ? rInf.GetText().Len()
/*N*/ 												 : rInf.GetLen();
/*N*/ 	rInf.SetLen( nLn );
/*N*/ 	xub_StrLen nCrsr = 0;
/*N*/ 	if( IsCapital() && nLn )
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 		nCrsr = GetCapitalCrsrOfst( rInf );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const XubString &rOldTxt = rInf.GetText();
/*N*/ 		short nOldKern = rInf.GetKern();
/*N*/ 		rInf.SetKern( CheckKerning() );
/*N*/ 		SV_STAT( nGetTextSize );
/*N*/ 		if ( !IsCaseMap() )
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 nCrsr = pLastFont->GetCrsrOfst( rInf );
/*N*/ 		else
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 		rInf.SetKern( nOldKern );
/*N*/ 		rInf.SetText( rOldTxt );
/*N*/ 	}
/*N*/ 	return nCrsr;
/*N*/ }

/*************************************************************************
 *                    SwSubFont::CalcEsc()
 *************************************************************************/


// used during painting of small capitals

/*************************************************************************
 *                      SwUnderlineFont::~SwUnderlineFont
 *
 * Used for the "continuous underline" feature.
 *************************************************************************/



//Helper for filters to find true lineheight of a font

}
