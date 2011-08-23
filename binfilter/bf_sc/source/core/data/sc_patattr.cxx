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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <bf_svx/adjitem.hxx>
#include <bf_svx/algitem.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svx/charreliefitem.hxx>
#include <bf_svx/cntritem.hxx>
#include <bf_svtools/colorcfg.hxx>
#include <bf_svx/colritem.hxx>
#include <bf_svx/crsditem.hxx>
#include <bf_svx/emphitem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/fontitem.hxx>
#include <bf_svx/forbiddenruleitem.hxx>
#include <bf_svx/frmdiritem.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/postitem.hxx>
#include <bf_svx/rotmodit.hxx>
#include <bf_svx/scripttypeitem.hxx>
#include <bf_svx/shaditem.hxx>
#include <bf_svx/shdditem.hxx>
#include <bf_svx/udlnitem.hxx>
#include <bf_svx/wghtitem.hxx>
#include <bf_svx/wrlmitem.hxx>
#include <bf_svtools/zforlist.hxx>
#include <vcl/outdev.hxx>

#include "patattr.hxx"
#include "docpool.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "document.hxx"
#include "globstr.hrc"
#include "validat.hxx"
#include "scmod.hxx"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

ScDocument* ScPatternAttr::pDoc = NULL;

// -----------------------------------------------------------------------

//!	move to some header file
inline long TwipsToHMM(long nTwips)	{ return (nTwips * 127 + 36) / 72; }
inline long HMMToTwips(long nHMM)	{ return (nHMM * 72 + 63) / 127; }

// -----------------------------------------------------------------------

/*N*/ ScPatternAttr::ScPatternAttr( SfxItemSet* pItemSet, const String& rStyleName )
/*N*/ 	:	SfxSetItem	( ATTR_PATTERN, pItemSet ),
/*N*/ 		pName		( new String( rStyleName ) ),
/*N*/ 		pStyle		( NULL )
/*N*/ {
/*N*/ }

/*N*/ ScPatternAttr::ScPatternAttr( SfxItemSet* pItemSet, ScStyleSheet* pStyleSheet )
/*N*/ 	:	SfxSetItem	( ATTR_PATTERN, pItemSet ),
/*N*/ 		pName		( NULL ),
/*N*/ 		pStyle		( pStyleSheet )
/*N*/ {
/*N*/ 	if ( pStyleSheet )
/*?*/ 		GetItemSet().SetParent( &pStyleSheet->GetItemSet() );
/*N*/ }

/*N*/ ScPatternAttr::ScPatternAttr( SfxItemPool* pItemPool )
/*N*/ 	:	SfxSetItem	( ATTR_PATTERN, new SfxItemSet( *pItemPool, ATTR_PATTERN_START, ATTR_PATTERN_END ) ),
/*N*/ 		pName		( NULL ),
/*N*/ 		pStyle		( NULL )
/*N*/ {
/*N*/ }

/*N*/ ScPatternAttr::ScPatternAttr( const ScPatternAttr& rPatternAttr )
/*N*/ 	:	SfxSetItem	( rPatternAttr ),
/*N*/ 		pStyle		( rPatternAttr.pStyle )
/*N*/ {
/*N*/ 	if (rPatternAttr.pName)
/*N*/ 		pName = new String(*rPatternAttr.pName);
/*N*/ 	else
/*N*/ 		pName = NULL;
/*N*/ }

/*N*/ __EXPORT ScPatternAttr::~ScPatternAttr()
/*N*/ {
/*N*/ 	delete pName;
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT ScPatternAttr::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	ScPatternAttr* pPattern = new ScPatternAttr( GetItemSet().Clone(TRUE, pPool) );
/*N*/ 
/*N*/ 	pPattern->pStyle = pStyle;
/*N*/ 	pPattern->pName  = pName ? new String(*pName) : NULL;
/*N*/ 
/*N*/ 	return pPattern;
/*N*/ }

/*N*/ inline int StrCmp( const String* pStr1, const String* pStr2 )
/*N*/ {
/*N*/ 	return ( pStr1 ? ( pStr2 ? ( *pStr1 == *pStr2 ) : FALSE ) : ( pStr2 ? FALSE : TRUE ) );
/*N*/ }

/*N*/ int __EXPORT ScPatternAttr::operator==( const SfxPoolItem& rCmp ) const
/*N*/ {
/*N*/ 	return ( SfxSetItem::operator==(rCmp) &&
/*N*/ 			 StrCmp( GetStyleName(), ((const ScPatternAttr&)rCmp).GetStyleName() ) );
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT ScPatternAttr::Create( SvStream& rStream, USHORT nVersion ) const
/*N*/ {
/*N*/ 	String* pStr;
/*N*/ 	BOOL	bHasStyle;
/*N*/ 	short	eFamDummy;
/*N*/ 
/*N*/ 	rStream >> bHasStyle;
/*N*/ 
/*N*/ 	if ( bHasStyle )
/*N*/ 	{
/*N*/ 		pStr = new String;
/*N*/ 		rStream.ReadByteString( *pStr, rStream.GetStreamCharSet() );
/*N*/ 		rStream >> eFamDummy; // wg. altem Dateiformat
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pStr = new String( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );
/*N*/ 
/*N*/ 	SfxItemSet *pSet = new SfxItemSet( *GetItemSet().GetPool(),
/*N*/ 									   ATTR_PATTERN_START, ATTR_PATTERN_END );
/*N*/ 	pSet->Load( rStream );
/*N*/ 
/*N*/ 	ScPatternAttr* pPattern = new ScPatternAttr( pSet );
/*N*/ 
/*N*/ 	pPattern->pName = pStr;
/*N*/ 
/*N*/ 	return pPattern;
/*N*/ }

/*N*/ SvStream& __EXPORT ScPatternAttr::Store(SvStream& rStream, USHORT nItemVersion) const
/*N*/ {
/*N*/ 	rStream << (BOOL)TRUE;
/*N*/ 
/*N*/ 	if ( pStyle )
/*N*/ 		rStream.WriteByteString( pStyle->GetName(), rStream.GetStreamCharSet() );
/*N*/ 	else if ( pName )					// wenn Style geloescht ist/war
/*?*/ 		rStream.WriteByteString( *pName, rStream.GetStreamCharSet() );
/*N*/ 	else
/*N*/ 		rStream.WriteByteString( ScGlobal::GetRscString(STR_STYLENAME_STANDARD),
/*N*/ 									rStream.GetStreamCharSet() );
/*N*/ 
/*N*/ 	rStream << (short)SFX_STYLE_FAMILY_PARA;  // wg. altem Dateiformat
/*N*/ 
/*N*/ 	GetItemSet().Store( rStream );
/*N*/ 
/*N*/ 	return rStream;
/*N*/ }

/*N*/ void ScPatternAttr::GetFont(
/*N*/         Font& rFont, const SfxItemSet& rItemSet, ScAutoFontColorMode eAutoMode,
/*N*/         OutputDevice* pOutDev, const Fraction* pScale,
/*N*/         const SfxItemSet* pCondSet, BYTE nScript,
/*N*/         const Color* pBackConfigColor, const Color* pTextConfigColor )
/*N*/ {
/*N*/ 	//	Items auslesen
/*N*/ 
/*N*/ 	const SvxFontItem* pFontAttr;
/*N*/ 	UINT32 nFontHeight;
/*N*/ 	FontWeight eWeight;
/*N*/ 	FontItalic eItalic;
/*N*/ 	FontUnderline eUnder;
/*N*/ 	BOOL bWordLine;
/*N*/ 	FontStrikeout eStrike;
/*N*/ 	BOOL bOutline;
/*N*/ 	BOOL bShadow;
/*N*/ 	FontEmphasisMark eEmphasis;
/*N*/ 	FontRelief eRelief;
/*N*/ 	Color aColor;
/*N*/ 
/*N*/ 	USHORT nFontId, nHeightId, nWeightId, nPostureId;
/*N*/ 	if ( nScript == SCRIPTTYPE_ASIAN )
/*N*/ 	{
/*?*/ 		nFontId    = ATTR_CJK_FONT;
/*?*/ 		nHeightId  = ATTR_CJK_FONT_HEIGHT;
/*?*/ 		nWeightId  = ATTR_CJK_FONT_WEIGHT;
/*?*/ 		nPostureId = ATTR_CJK_FONT_POSTURE;
/*N*/ 	}
/*N*/ 	else if ( nScript == SCRIPTTYPE_COMPLEX )
/*N*/ 	{
/*?*/ 		nFontId    = ATTR_CTL_FONT;
/*?*/ 		nHeightId  = ATTR_CTL_FONT_HEIGHT;
/*?*/ 		nWeightId  = ATTR_CTL_FONT_WEIGHT;
/*?*/ 		nPostureId = ATTR_CTL_FONT_POSTURE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nFontId    = ATTR_FONT;
/*N*/ 		nHeightId  = ATTR_FONT_HEIGHT;
/*N*/ 		nWeightId  = ATTR_FONT_WEIGHT;
/*N*/ 		nPostureId = ATTR_FONT_POSTURE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pCondSet )
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pItem;
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( nFontId, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( nFontId );
/*N*/ 		pFontAttr = (const SvxFontItem*) pItem;
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( nHeightId, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( nHeightId );
/*N*/ 		nFontHeight = ((const SvxFontHeightItem*)pItem)->GetHeight();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( nWeightId, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( nWeightId );
/*N*/ 		eWeight = (FontWeight)((const SvxWeightItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( nPostureId, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( nPostureId );
/*N*/ 		eItalic = (FontItalic)((const SvxPostureItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_UNDERLINE, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( ATTR_FONT_UNDERLINE );
/*N*/ 		eUnder = (FontUnderline)((const SvxUnderlineItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_WORDLINE, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( ATTR_FONT_WORDLINE );
/*N*/ 		bWordLine = ((const SvxWordLineModeItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_CROSSEDOUT, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( ATTR_FONT_CROSSEDOUT );
/*N*/ 		eStrike = (FontStrikeout)((const SvxCrossedOutItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_CONTOUR, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( ATTR_FONT_CONTOUR );
/*N*/ 		bOutline = ((const SvxContourItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_SHADOWED, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( ATTR_FONT_SHADOWED );
/*N*/ 		bShadow = ((const SvxShadowedItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_EMPHASISMARK, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( ATTR_FONT_EMPHASISMARK );
/*N*/ 		eEmphasis = ((const SvxEmphasisMarkItem*)pItem)->GetEmphasisMark();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_RELIEF, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( ATTR_FONT_RELIEF );
/*N*/ 		eRelief = (FontRelief)((const SvxCharReliefItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_COLOR, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rItemSet.Get( ATTR_FONT_COLOR );
/*N*/ 		aColor = ((const SvxColorItem*)pItem)->GetValue();
/*N*/ 	}
/*N*/     else    // alles aus rItemSet
/*N*/ 	{
/*N*/         pFontAttr = &(const SvxFontItem&)rItemSet.Get( nFontId );
/*N*/ 		nFontHeight = ((const SvxFontHeightItem&)
/*N*/                         rItemSet.Get( nHeightId )).GetHeight();
/*N*/ 		eWeight = (FontWeight)((const SvxWeightItem&)
/*N*/                         rItemSet.Get( nWeightId )).GetValue();
/*N*/ 		eItalic = (FontItalic)((const SvxPostureItem&)
/*N*/                         rItemSet.Get( nPostureId )).GetValue();
/*N*/ 		eUnder = (FontUnderline)((const SvxUnderlineItem&)
/*N*/                         rItemSet.Get( ATTR_FONT_UNDERLINE )).GetValue();
/*N*/ 		bWordLine = ((const SvxWordLineModeItem&)
/*N*/                         rItemSet.Get( ATTR_FONT_WORDLINE )).GetValue();
/*N*/ 		eStrike = (FontStrikeout)((const SvxCrossedOutItem&)
/*N*/                         rItemSet.Get( ATTR_FONT_CROSSEDOUT )).GetValue();
/*N*/ 		bOutline = ((const SvxContourItem&)
/*N*/                         rItemSet.Get( ATTR_FONT_CONTOUR )).GetValue();
/*N*/ 		bShadow = ((const SvxShadowedItem&)
/*N*/                         rItemSet.Get( ATTR_FONT_SHADOWED )).GetValue();
/*N*/ 		eEmphasis = ((const SvxEmphasisMarkItem&)
/*N*/                         rItemSet.Get( ATTR_FONT_EMPHASISMARK )).GetEmphasisMark();
/*N*/ 		eRelief = (FontRelief)((const SvxCharReliefItem&)
/*N*/                         rItemSet.Get( ATTR_FONT_RELIEF )).GetValue();
/*N*/ 		aColor = ((const SvxColorItem&)
/*N*/                         rItemSet.Get( ATTR_FONT_COLOR )).GetValue();
/*N*/ 	}
/*N*/ 	DBG_ASSERT(pFontAttr,"nanu?");
/*N*/ 
/*N*/ 	//	auswerten
/*N*/ 
/*N*/ 	//	FontItem:
/*N*/ 
/*N*/ 	if (rFont.GetName() != pFontAttr->GetFamilyName())
/*N*/ 		rFont.SetName( pFontAttr->GetFamilyName() );
/*N*/ 	if (rFont.GetStyleName() != pFontAttr->GetStyleName())
/*?*/ 		rFont.SetStyleName( pFontAttr->GetStyleName() );
/*N*/ 	if (rFont.GetFamily() != pFontAttr->GetFamily())
/*N*/ 		rFont.SetFamily( pFontAttr->GetFamily() );
/*N*/ 	if (rFont.GetCharSet() != pFontAttr->GetCharSet())
/*N*/ 		rFont.SetCharSet( pFontAttr->GetCharSet() );
/*N*/ 	if (rFont.GetPitch() != pFontAttr->GetPitch())
/*N*/ 		rFont.SetPitch( pFontAttr->GetPitch() );
/*N*/ 
/*N*/ 	//	Groesse
/*N*/ 
/*N*/ 	if ( pOutDev != NULL )
/*N*/ 	{
/*N*/ 		Size aEffSize;
/*N*/ 		Fraction aFraction( 1,1 );
/*N*/ 		if (pScale)
/*N*/ 			aFraction = *pScale;
/*N*/ 		Size aSize( 0, (long) nFontHeight );
/*N*/ 		MapMode aDestMode = pOutDev->GetMapMode();
/*N*/ 		MapMode aSrcMode( MAP_TWIP, Point(), aFraction, aFraction );
/*N*/ 		if (aDestMode.GetMapUnit() == MAP_PIXEL)
/*N*/ 			aEffSize = pOutDev->LogicToPixel( aSize, aSrcMode );
/*N*/ 		else
/*N*/ 		{
/*?*/ 			Fraction aFractOne(1,1);
/*?*/ 			aDestMode.SetScaleX( aFractOne );
/*?*/ 			aDestMode.SetScaleY( aFractOne );
/*?*/ 			aEffSize = OutputDevice::LogicToLogic( aSize, aSrcMode, aDestMode );
/*N*/ 		}
/*N*/ 		rFont.SetSize( aEffSize );
/*N*/ 	}
/*N*/ 	else /* if pOutDev != NULL */
/*N*/ 	{
/*?*/ 		rFont.SetSize( Size( 0, (long) nFontHeight ) );
/*N*/ 	}
/*N*/ 
    //	determine effective font color

/*M*/ 	if ( ( aColor.GetColor() == COL_AUTO && eAutoMode != SC_AUTOCOL_RAW ) ||
/*M*/ 			eAutoMode == SC_AUTOCOL_IGNOREFONT || eAutoMode == SC_AUTOCOL_IGNOREALL )
/*M*/ 	{
/*M*/ 		if ( eAutoMode == SC_AUTOCOL_BLACK )
/*M*/ 			aColor.SetColor( COL_BLACK );
/*M*/ 		else
/*M*/ 		{
/*M*/ 			//	get background color from conditional or own set
/*M*/ 			Color aBackColor;
/*M*/ 			if ( pCondSet )
/*M*/ 			{
/*M*/ 				const SfxPoolItem* pItem;
/*M*/ 				if ( pCondSet->GetItemState( ATTR_BACKGROUND, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/                     pItem = &rItemSet.Get( ATTR_BACKGROUND );
/*M*/ 				aBackColor = ((const SvxBrushItem*)pItem)->GetColor();
/*M*/ 			}
/*M*/ 			else
/*N*/                 aBackColor = ((const SvxBrushItem&)rItemSet.Get( ATTR_BACKGROUND )).GetColor();
/*M*/ 
/*M*/ 			//	if background color attribute is transparent, use window color for brightness comparisons
/*M*/ 			if ( aBackColor == COL_TRANSPARENT ||
/*M*/ 					eAutoMode == SC_AUTOCOL_IGNOREBACK || eAutoMode == SC_AUTOCOL_IGNOREALL )
/*M*/ 			{
/*M*/ 				if ( eAutoMode == SC_AUTOCOL_PRINT )
/*M*/ 					aBackColor.SetColor( COL_WHITE );
/*M*/ 				else if ( pBackConfigColor )
/*M*/ 				{
/*M*/ 					// pBackConfigColor can be used to avoid repeated lookup of the configured color
/*M*/ 					aBackColor = *pBackConfigColor;
/*M*/ 				}
/*M*/ 				else
/*N*/                     aBackColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(DOCCOLOR).nColor );
/*M*/ 			}
/*M*/ 
/*M*/ 			//	get system text color for comparison
/*M*/ 			Color aSysTextColor;
/*M*/ 			if ( eAutoMode == SC_AUTOCOL_PRINT )
/*M*/ 				aSysTextColor.SetColor( COL_BLACK );
/*N*/ 			else if ( pTextConfigColor )
/*N*/ 			{
/*N*/ 				// pTextConfigColor can be used to avoid repeated lookup of the configured color
/*N*/ 				aSysTextColor = *pTextConfigColor;
/*N*/ 			}
/*N*/ 			else
/*N*/                 aSysTextColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(FONTCOLOR).nColor );
/*M*/ 
/*M*/ 			//	select the resulting color
/*M*/ 			if ( aBackColor.IsDark() && aSysTextColor.IsDark() )
/*M*/ 			{
/*M*/ 				//	use white instead of dark on dark
/*M*/ 				aColor.SetColor( COL_WHITE );
/*M*/ 			}
/*M*/ 			else if ( aBackColor.IsBright() && aSysTextColor.IsBright() )
/*M*/ 			{
/*M*/ 				//	use black instead of bright on bright
/*M*/ 				aColor.SetColor( COL_BLACK );
/*M*/ 			}
/*M*/ 			else
/*M*/ 			{
/*M*/ 				//	use aSysTextColor (black for SC_AUTOCOL_PRINT, from style settings otherwise)
/*M*/ 				aColor = aSysTextColor;
/*M*/ 			}
/*M*/ 		}
/*M*/ 	}

    //	set font effects

/*N*/ 	if (rFont.GetWeight() != eWeight)
/*N*/ 		rFont.SetWeight( eWeight );
/*N*/ 	if (rFont.GetItalic() != eItalic)
/*N*/ 		rFont.SetItalic( eItalic );
/*N*/ 	if (rFont.GetUnderline() != eUnder)
/*N*/ 		rFont.SetUnderline( eUnder );
/*N*/ 	if (rFont.IsWordLineMode() != bWordLine)
/*N*/ 		rFont.SetWordLineMode( bWordLine );
/*N*/ 	if (rFont.GetStrikeout() != eStrike)
/*N*/ 		rFont.SetStrikeout( eStrike );
/*N*/ 	if (rFont.IsOutline() != bOutline)
/*N*/ 		rFont.SetOutline( bOutline );
/*N*/ 	if (rFont.IsShadow() != bShadow)
/*N*/ 		rFont.SetShadow( bShadow );
/*N*/ 	if (rFont.GetEmphasisMark() != eEmphasis)
/*N*/ 		rFont.SetEmphasisMark( eEmphasis );
/*N*/ 	if (rFont.GetRelief() != eRelief)
/*N*/ 		rFont.SetRelief( eRelief );
/*N*/ 	if (rFont.GetColor() != aColor)
/*N*/ 		rFont.SetColor( aColor );
/*N*/ 	if (!rFont.IsTransparent())
/*N*/ 		rFont.SetTransparent( TRUE );
/*N*/ }

/*N*/ void ScPatternAttr::GetFont(
/*N*/         Font& rFont, ScAutoFontColorMode eAutoMode,
/*N*/         OutputDevice* pOutDev, const Fraction* pScale,
/*N*/         const SfxItemSet* pCondSet, BYTE nScript,
/*N*/         const Color* pBackConfigColor, const Color* pTextConfigColor ) const
/*N*/ {
/*N*/     GetFont( rFont, GetItemSet(), eAutoMode, pOutDev, pScale, pCondSet, nScript, pBackConfigColor, pTextConfigColor );
/*N*/ }


/*N*/ void ScPatternAttr::FillToEditItemSet( SfxItemSet& rEditSet, const SfxItemSet& rSrcSet, const SfxItemSet* pCondSet )
/*N*/ {
/*N*/ 	//	Items auslesen
/*N*/ 
/*N*/ 	SvxColorItem	aColorItem(EE_CHAR_COLOR);				// use item as-is
/*N*/ 	SvxFontItem		aFontItem(EE_CHAR_FONTINFO);			// use item as-is
/*N*/ 	SvxFontItem		aCjkFontItem(EE_CHAR_FONTINFO_CJK);
/*N*/ 	SvxFontItem		aCtlFontItem(EE_CHAR_FONTINFO_CTL);
/*N*/ 	long			nTHeight, nCjkTHeight, nCtlTHeight;		// Twips
/*N*/ 	FontWeight		eWeight, eCjkWeight, eCtlWeight;
/*N*/ 	SvxUnderlineItem aUnderlineItem(UNDERLINE_NONE, EE_CHAR_UNDERLINE);
/*N*/ 	BOOL			bWordLine;
/*N*/ 	FontStrikeout	eStrike;
/*N*/ 	FontItalic		eItalic, eCjkItalic, eCtlItalic;
/*N*/ 	BOOL			bOutline;
/*N*/ 	BOOL			bShadow;
/*N*/ 	BOOL			bForbidden;
/*N*/ 	FontEmphasisMark eEmphasis;
/*N*/ 	FontRelief		eRelief;
/*N*/ 	LanguageType	eLang, eCjkLang, eCtlLang;
/*N*/ 	BOOL			bHyphenate;
/*N*/ 	SvxFrameDirection eDirection;
/*N*/ 
/*N*/ 	//!	additional parameter to control if language is needed?
/*N*/ 
/*N*/ 	if ( pCondSet )
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pItem;
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_COLOR, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_COLOR );
/*N*/ 		aColorItem = *(const SvxColorItem*)pItem;
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT );
/*N*/ 		aFontItem = *(const SvxFontItem*)pItem;
/*N*/ 		if ( pCondSet->GetItemState( ATTR_CJK_FONT, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_CJK_FONT );
/*N*/ 		aCjkFontItem = *(const SvxFontItem*)pItem;
/*N*/ 		if ( pCondSet->GetItemState( ATTR_CTL_FONT, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_CTL_FONT );
/*N*/ 		aCtlFontItem = *(const SvxFontItem*)pItem;
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_HEIGHT, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_HEIGHT );
/*N*/ 		nTHeight = ((const SvxFontHeightItem*)pItem)->GetHeight();
/*N*/ 		if ( pCondSet->GetItemState( ATTR_CJK_FONT_HEIGHT, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_CJK_FONT_HEIGHT );
/*N*/ 		nCjkTHeight = ((const SvxFontHeightItem*)pItem)->GetHeight();
/*N*/ 		if ( pCondSet->GetItemState( ATTR_CTL_FONT_HEIGHT, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_CTL_FONT_HEIGHT );
/*N*/ 		nCtlTHeight = ((const SvxFontHeightItem*)pItem)->GetHeight();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_WEIGHT, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_WEIGHT );
/*N*/ 		eWeight = (FontWeight)((const SvxWeightItem*)pItem)->GetValue();
/*N*/ 		if ( pCondSet->GetItemState( ATTR_CJK_FONT_WEIGHT, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_CJK_FONT_WEIGHT );
/*N*/ 		eCjkWeight = (FontWeight)((const SvxWeightItem*)pItem)->GetValue();
/*N*/ 		if ( pCondSet->GetItemState( ATTR_CTL_FONT_WEIGHT, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_CTL_FONT_WEIGHT );
/*N*/ 		eCtlWeight = (FontWeight)((const SvxWeightItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_POSTURE, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_POSTURE );
/*N*/ 		eItalic = (FontItalic)((const SvxPostureItem*)pItem)->GetValue();
/*N*/ 		if ( pCondSet->GetItemState( ATTR_CJK_FONT_POSTURE, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_CJK_FONT_POSTURE );
/*N*/ 		eCjkItalic = (FontItalic)((const SvxPostureItem*)pItem)->GetValue();
/*N*/ 		if ( pCondSet->GetItemState( ATTR_CTL_FONT_POSTURE, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_CTL_FONT_POSTURE );
/*N*/ 		eCtlItalic = (FontItalic)((const SvxPostureItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_UNDERLINE, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_UNDERLINE );
/*N*/ 		aUnderlineItem = *(const SvxUnderlineItem*)pItem;
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_WORDLINE, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_WORDLINE );
/*N*/ 		bWordLine = ((const SvxWordLineModeItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_CROSSEDOUT, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_CROSSEDOUT );
/*N*/ 		eStrike = (FontStrikeout)((const SvxCrossedOutItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_CONTOUR, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_CONTOUR );
/*N*/ 		bOutline = ((const SvxContourItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_SHADOWED, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_SHADOWED );
/*N*/ 		bShadow = ((const SvxShadowedItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FORBIDDEN_RULES, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FORBIDDEN_RULES );
/*N*/ 		bForbidden = ((const SvxForbiddenRuleItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_EMPHASISMARK, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_EMPHASISMARK );
/*N*/ 		eEmphasis = ((const SvxEmphasisMarkItem*)pItem)->GetEmphasisMark();
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_RELIEF, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_RELIEF );
/*N*/ 		eRelief = (FontRelief)((const SvxCharReliefItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_FONT_LANGUAGE, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_FONT_LANGUAGE );
/*N*/ 		eLang = ((const SvxLanguageItem*)pItem)->GetLanguage();
/*N*/ 		if ( pCondSet->GetItemState( ATTR_CJK_FONT_LANGUAGE, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_CJK_FONT_LANGUAGE );
/*N*/ 		eCjkLang = ((const SvxLanguageItem*)pItem)->GetLanguage();
/*N*/ 		if ( pCondSet->GetItemState( ATTR_CTL_FONT_LANGUAGE, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_CTL_FONT_LANGUAGE );
/*N*/ 		eCtlLang = ((const SvxLanguageItem*)pItem)->GetLanguage();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_HYPHENATE, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_HYPHENATE );
/*N*/ 		bHyphenate = ((const SfxBoolItem*)pItem)->GetValue();
/*N*/ 
/*N*/ 		if ( pCondSet->GetItemState( ATTR_WRITINGDIR, TRUE, &pItem ) != SFX_ITEM_SET )
/*N*/             pItem = &rSrcSet.Get( ATTR_WRITINGDIR );
/*N*/ 		eDirection = (SvxFrameDirection)((const SvxFrameDirectionItem*)pItem)->GetValue();
/*N*/ 	}
/*N*/ 	else		// alles direkt aus Pattern
/*N*/ 	{
/*N*/         aColorItem = (const SvxColorItem&) rSrcSet.Get( ATTR_FONT_COLOR );
/*N*/         aFontItem = (const SvxFontItem&) rSrcSet.Get( ATTR_FONT );
/*N*/         aCjkFontItem = (const SvxFontItem&) rSrcSet.Get( ATTR_CJK_FONT );
/*N*/         aCtlFontItem = (const SvxFontItem&) rSrcSet.Get( ATTR_CTL_FONT );
/*N*/ 		nTHeight = ((const SvxFontHeightItem&)
/*N*/                         rSrcSet.Get( ATTR_FONT_HEIGHT )).GetHeight();
/*N*/ 		nCjkTHeight = ((const SvxFontHeightItem&)
/*N*/                         rSrcSet.Get( ATTR_CJK_FONT_HEIGHT )).GetHeight();
/*N*/ 		nCtlTHeight = ((const SvxFontHeightItem&)
/*N*/                         rSrcSet.Get( ATTR_CTL_FONT_HEIGHT )).GetHeight();
/*N*/ 		eWeight = (FontWeight)((const SvxWeightItem&)
/*N*/                         rSrcSet.Get( ATTR_FONT_WEIGHT )).GetValue();
/*N*/ 		eCjkWeight = (FontWeight)((const SvxWeightItem&)
/*N*/                         rSrcSet.Get( ATTR_CJK_FONT_WEIGHT )).GetValue();
/*N*/ 		eCtlWeight = (FontWeight)((const SvxWeightItem&)
/*N*/                         rSrcSet.Get( ATTR_CTL_FONT_WEIGHT )).GetValue();
/*N*/ 		eItalic = (FontItalic)((const SvxPostureItem&)
/*N*/                         rSrcSet.Get( ATTR_FONT_POSTURE )).GetValue();
/*N*/ 		eCjkItalic = (FontItalic)((const SvxPostureItem&)
/*N*/                         rSrcSet.Get( ATTR_CJK_FONT_POSTURE )).GetValue();
/*N*/ 		eCtlItalic = (FontItalic)((const SvxPostureItem&)
/*N*/                         rSrcSet.Get( ATTR_CTL_FONT_POSTURE )).GetValue();
/*N*/         aUnderlineItem = (const SvxUnderlineItem&) rSrcSet.Get( ATTR_FONT_UNDERLINE );
/*N*/ 		bWordLine = ((const SvxWordLineModeItem&)
/*N*/                         rSrcSet.Get( ATTR_FONT_WORDLINE )).GetValue();
/*N*/ 		eStrike = (FontStrikeout)((const SvxCrossedOutItem&)
/*N*/                         rSrcSet.Get( ATTR_FONT_CROSSEDOUT )).GetValue();
/*N*/ 		bOutline = ((const SvxContourItem&)
/*N*/                         rSrcSet.Get( ATTR_FONT_CONTOUR )).GetValue();
/*N*/ 		bShadow = ((const SvxShadowedItem&)
/*N*/                         rSrcSet.Get( ATTR_FONT_SHADOWED )).GetValue();
/*N*/ 		bForbidden = ((const SvxForbiddenRuleItem&)
/*N*/                         rSrcSet.Get( ATTR_FORBIDDEN_RULES )).GetValue();
/*N*/ 		eEmphasis = ((const SvxEmphasisMarkItem&)
/*N*/                         rSrcSet.Get( ATTR_FONT_EMPHASISMARK )).GetEmphasisMark();
/*N*/ 		eRelief = (FontRelief)((const SvxCharReliefItem&)
/*N*/                         rSrcSet.Get( ATTR_FONT_RELIEF )).GetValue();
/*N*/ 		eLang = ((const SvxLanguageItem&)
/*N*/                         rSrcSet.Get( ATTR_FONT_LANGUAGE )).GetLanguage();
/*N*/ 		eCjkLang = ((const SvxLanguageItem&)
/*N*/                         rSrcSet.Get( ATTR_CJK_FONT_LANGUAGE )).GetLanguage();
/*N*/ 		eCtlLang = ((const SvxLanguageItem&)
/*N*/                         rSrcSet.Get( ATTR_CTL_FONT_LANGUAGE )).GetLanguage();
/*N*/ 		bHyphenate = ((const SfxBoolItem&)
/*N*/                         rSrcSet.Get( ATTR_HYPHENATE )).GetValue();
/*N*/ 		eDirection = (SvxFrameDirection)((const SvxFrameDirectionItem&)
/*N*/                         rSrcSet.Get( ATTR_WRITINGDIR )).GetValue();
/*N*/ 	}
/*N*/ 
/*N*/ 	// kompatibel zu LogicToLogic rechnen, also 2540/1440 = 127/72, und runden
/*N*/ 
/*N*/ 	long nHeight = TwipsToHMM(nTHeight);
/*N*/ 	long nCjkHeight = TwipsToHMM(nCjkTHeight);
/*N*/ 	long nCtlHeight = TwipsToHMM(nCtlTHeight);
/*N*/ 
/*N*/ 	//	put items into EditEngine ItemSet
/*N*/ 
/*N*/ 	if ( aColorItem.GetValue().GetColor() == COL_AUTO )
/*N*/ 	{
/*N*/ 		//	#108979# When cell attributes are converted to EditEngine paragraph attributes,
/*N*/ 		//	don't create a hard item for automatic color, because that would be converted
/*N*/ 		//	to black when the item's Store method is used in CreateTransferable/WriteBin.
/*N*/ 		//	COL_AUTO is the EditEngine's pool default, so ClearItem will result in automatic
/*N*/ 		//	color, too, without having to store the item.
/*N*/ 		rEditSet.ClearItem( EE_CHAR_COLOR );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	    rEditSet.Put( aColorItem );
/*N*/     rEditSet.Put( aFontItem );
/*N*/     rEditSet.Put( aCjkFontItem );
/*N*/     rEditSet.Put( aCtlFontItem );
/*N*/     rEditSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT ) );
/*N*/     rEditSet.Put( SvxFontHeightItem( nCjkHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
/*N*/     rEditSet.Put( SvxFontHeightItem( nCtlHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );
/*N*/     rEditSet.Put( SvxWeightItem ( eWeight,      EE_CHAR_WEIGHT ) );
/*N*/     rEditSet.Put( SvxWeightItem ( eCjkWeight,   EE_CHAR_WEIGHT_CJK ) );
/*N*/     rEditSet.Put( SvxWeightItem ( eCtlWeight,   EE_CHAR_WEIGHT_CTL ) );
/*N*/     rEditSet.Put( aUnderlineItem );
/*N*/     rEditSet.Put( SvxWordLineModeItem( bWordLine,   EE_CHAR_WLM ) );
/*N*/     rEditSet.Put( SvxCrossedOutItem( eStrike,       EE_CHAR_STRIKEOUT ) );
/*N*/     rEditSet.Put( SvxPostureItem    ( eItalic,      EE_CHAR_ITALIC ) );
/*N*/     rEditSet.Put( SvxPostureItem    ( eCjkItalic,   EE_CHAR_ITALIC_CJK ) );
/*N*/     rEditSet.Put( SvxPostureItem    ( eCtlItalic,   EE_CHAR_ITALIC_CTL ) );
/*N*/     rEditSet.Put( SvxContourItem    ( bOutline,     EE_CHAR_OUTLINE ) );
/*N*/     rEditSet.Put( SvxShadowedItem   ( bShadow,      EE_CHAR_SHADOW ) );
/*N*/     rEditSet.Put( SfxBoolItem       ( EE_PARA_FORBIDDENRULES, bForbidden ) );
/*N*/     rEditSet.Put( SvxEmphasisMarkItem( eEmphasis,   EE_CHAR_EMPHASISMARK ) );
/*N*/     rEditSet.Put( SvxCharReliefItem( eRelief,       EE_CHAR_RELIEF ) );
/*N*/     rEditSet.Put( SvxLanguageItem   ( eLang,        EE_CHAR_LANGUAGE ) );
/*N*/     rEditSet.Put( SvxLanguageItem   ( eCjkLang,     EE_CHAR_LANGUAGE_CJK ) );
/*N*/     rEditSet.Put( SvxLanguageItem   ( eCtlLang,     EE_CHAR_LANGUAGE_CTL ) );
/*N*/     rEditSet.Put( SfxBoolItem       ( EE_PARA_HYPHENATE, bHyphenate ) );
/*N*/     rEditSet.Put( SvxFrameDirectionItem( eDirection, EE_PARA_WRITINGDIR ) );
/*N*/ }

/*N*/ void ScPatternAttr::FillEditItemSet( SfxItemSet* pEditSet, const SfxItemSet* pCondSet ) const
/*N*/ {
/*N*/     if( pEditSet )
/*N*/         FillToEditItemSet( *pEditSet, GetItemSet(), pCondSet );
/*N*/ }


/*N*/ void ScPatternAttr::GetFromEditItemSet( SfxItemSet& rDestSet, const SfxItemSet& rEditSet )
/*N*/ {
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 
/*N*/     if (rEditSet.GetItemState(EE_CHAR_COLOR,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxColorItem(ATTR_FONT_COLOR) = *(const SvxColorItem*)pItem );
/*N*/ 
/*N*/     if (rEditSet.GetItemState(EE_CHAR_FONTINFO,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxFontItem(ATTR_FONT) = *(const SvxFontItem*)pItem );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_FONTINFO_CJK,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxFontItem(ATTR_CJK_FONT) = *(const SvxFontItem*)pItem );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_FONTINFO_CTL,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxFontItem(ATTR_CTL_FONT) = *(const SvxFontItem*)pItem );
/*N*/ 
/*N*/     if (rEditSet.GetItemState(EE_CHAR_FONTHEIGHT,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxFontHeightItem( HMMToTwips( ((const SvxFontHeightItem*)pItem)->GetHeight() ),
/*N*/ 						100, ATTR_FONT_HEIGHT ) );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_FONTHEIGHT_CJK,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxFontHeightItem( HMMToTwips( ((const SvxFontHeightItem*)pItem)->GetHeight() ),
/*N*/ 						100, ATTR_CJK_FONT_HEIGHT ) );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_FONTHEIGHT_CTL,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxFontHeightItem( HMMToTwips( ((const SvxFontHeightItem*)pItem)->GetHeight() ),
/*N*/ 						100, ATTR_CTL_FONT_HEIGHT ) );
/*N*/ 
/*N*/     if (rEditSet.GetItemState(EE_CHAR_WEIGHT,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxWeightItem( (FontWeight)((const SvxWeightItem*)pItem)->GetValue(),
/*N*/ 						ATTR_FONT_WEIGHT) );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_WEIGHT_CJK,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxWeightItem( (FontWeight)((const SvxWeightItem*)pItem)->GetValue(),
/*N*/ 						ATTR_CJK_FONT_WEIGHT) );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_WEIGHT_CTL,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxWeightItem( (FontWeight)((const SvxWeightItem*)pItem)->GetValue(),
/*N*/ 						ATTR_CTL_FONT_WEIGHT) );
/*N*/ 
/*N*/ 	// SvxUnderlineItem contains enum and color
/*N*/     if (rEditSet.GetItemState(EE_CHAR_UNDERLINE,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxUnderlineItem(UNDERLINE_NONE,ATTR_FONT_UNDERLINE) = *(const SvxUnderlineItem*)pItem );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_WLM,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxWordLineModeItem( ((const SvxWordLineModeItem*)pItem)->GetValue(),
/*N*/ 						ATTR_FONT_WORDLINE) );
/*N*/ 
/*N*/     if (rEditSet.GetItemState(EE_CHAR_STRIKEOUT,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxCrossedOutItem( (FontStrikeout)((const SvxCrossedOutItem*)pItem)->GetValue(),
/*N*/ 						ATTR_FONT_CROSSEDOUT) );
/*N*/ 
/*N*/     if (rEditSet.GetItemState(EE_CHAR_ITALIC,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxPostureItem( (FontItalic)((const SvxPostureItem*)pItem)->GetValue(),
/*N*/ 						ATTR_FONT_POSTURE) );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_ITALIC_CJK,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxPostureItem( (FontItalic)((const SvxPostureItem*)pItem)->GetValue(),
/*N*/ 						ATTR_CJK_FONT_POSTURE) );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_ITALIC_CTL,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxPostureItem( (FontItalic)((const SvxPostureItem*)pItem)->GetValue(),
/*N*/ 						ATTR_CTL_FONT_POSTURE) );
/*N*/ 
/*N*/     if (rEditSet.GetItemState(EE_CHAR_OUTLINE,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxContourItem( ((const SvxContourItem*)pItem)->GetValue(),
/*N*/ 						ATTR_FONT_CONTOUR) );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_SHADOW,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxShadowedItem( ((const SvxShadowedItem*)pItem)->GetValue(),
/*N*/ 						ATTR_FONT_SHADOWED) );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_EMPHASISMARK,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxEmphasisMarkItem( ((const SvxEmphasisMarkItem*)pItem)->GetEmphasisMark(),
/*N*/ 						ATTR_FONT_EMPHASISMARK) );
/*N*/     if (rEditSet.GetItemState(EE_CHAR_RELIEF,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/         rDestSet.Put( SvxCharReliefItem( (FontRelief)((const SvxCharReliefItem*)pItem)->GetValue(),
/*N*/ 						ATTR_FONT_RELIEF) );
/*N*/ 
/*N*/     if (rEditSet.GetItemState(EE_PARA_JUST,TRUE,&pItem) == SFX_ITEM_SET)
/*N*/ 	{
/*N*/ 		SvxCellHorJustify eVal;
/*N*/ 		switch ( ((const SvxAdjustItem*)pItem)->GetAdjust() )
/*N*/ 		{
/*N*/ 			case SVX_ADJUST_LEFT:
/*N*/ 				// #30154# EditEngine Default ist bei dem GetAttribs() ItemSet
/*N*/ 				// immer gesetzt!
/*N*/ 				// ob links oder rechts entscheiden wir selbst bei Text/Zahl
/*N*/ 				eVal = SVX_HOR_JUSTIFY_STANDARD;
/*N*/ 				break;
/*N*/ 			case SVX_ADJUST_RIGHT:
/*N*/ 				eVal = SVX_HOR_JUSTIFY_RIGHT;
/*N*/ 				break;
/*N*/ 			case SVX_ADJUST_BLOCK:
/*N*/ 				eVal = SVX_HOR_JUSTIFY_BLOCK;
/*N*/ 				break;
/*N*/ 			case SVX_ADJUST_CENTER:
/*N*/ 				eVal = SVX_HOR_JUSTIFY_CENTER;
/*N*/ 				break;
/*N*/ 			case SVX_ADJUST_BLOCKLINE:
/*N*/ 				eVal = SVX_HOR_JUSTIFY_BLOCK;
/*N*/ 				break;
/*N*/ 			case SVX_ADJUST_END:
/*N*/ 				eVal = SVX_HOR_JUSTIFY_RIGHT;
/*N*/ 				break;
/*N*/ 			default:
/*N*/ 				eVal = SVX_HOR_JUSTIFY_STANDARD;
/*N*/ 		}
/*N*/ 		if ( eVal != SVX_HOR_JUSTIFY_STANDARD )
/*N*/             rDestSet.Put( SvxHorJustifyItem( eVal, ATTR_HOR_JUSTIFY) );
/*N*/ 	}
/*N*/ }

/*N*/ void ScPatternAttr::GetFromEditItemSet( const SfxItemSet* pEditSet )
/*N*/ {
/*N*/     if( pEditSet )
/*N*/         GetFromEditItemSet( GetItemSet(), *pEditSet );
/*N*/ }

/*N*/ void ScPatternAttr::FillEditParaItems( SfxItemSet* pEditSet ) const
/*N*/ {
/*N*/ 	//	in GetFromEditItemSet schon dabei, in FillEditItemSet aber nicht
/*N*/ 	//	Hor. Ausrichtung Standard wird immer als "links" umgesetzt
/*N*/ 
/*N*/ 	const SfxItemSet& rMySet = GetItemSet();
/*N*/ 
/*N*/ 	SvxCellHorJustify eHorJust = (SvxCellHorJustify)
/*N*/ 		((const SvxHorJustifyItem&)rMySet.Get(ATTR_HOR_JUSTIFY)).GetValue();
/*N*/ 
/*N*/ 	SvxAdjust eSvxAdjust;
/*N*/ 	switch (eHorJust)
/*N*/ 	{
/*N*/ 		case SVX_HOR_JUSTIFY_RIGHT:	 eSvxAdjust = SVX_ADJUST_RIGHT;	 break;
/*N*/ 		case SVX_HOR_JUSTIFY_CENTER: eSvxAdjust = SVX_ADJUST_CENTER; break;
/*N*/ 		case SVX_HOR_JUSTIFY_BLOCK:	 eSvxAdjust = SVX_ADJUST_BLOCK;	 break;
/*N*/ 		default:					 eSvxAdjust = SVX_ADJUST_LEFT;	 break;
/*N*/ 	}
/*N*/ 	pEditSet->Put( SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );
/*N*/ }

/*N*/ void ScPatternAttr::DeleteUnchanged( const ScPatternAttr* pOldAttrs )
/*N*/ {
/*N*/ 	SfxItemSet& rThisSet = GetItemSet();
/*N*/ 	const SfxItemSet& rOldSet = pOldAttrs->GetItemSet();
/*N*/ 
/*N*/ 	const SfxPoolItem* pThisItem;
/*N*/ 	const SfxPoolItem* pOldItem;
/*N*/ 
/*N*/ 	for ( USHORT nWhich=ATTR_PATTERN_START; nWhich<=ATTR_PATTERN_END; nWhich++ )
/*N*/ 	{
/*N*/ 		//	only items that are set are interesting
/*N*/ 		if ( rThisSet.GetItemState( nWhich, FALSE, &pThisItem ) == SFX_ITEM_SET )
/*N*/         {
/*N*/             SfxItemState eOldState = rOldSet.GetItemState( nWhich, TRUE, &pOldItem );
/*N*/             if ( eOldState == SFX_ITEM_SET )
/*N*/ 			{
/*?*/ 				//	item is set in OldAttrs (or its parent) -> compare pointers
/*?*/ 				if ( pThisItem == pOldItem )
/*?*/ 					rThisSet.ClearItem( nWhich );
/*N*/ 			}
/*N*/             else if ( eOldState != SFX_ITEM_DONTCARE )
/*N*/ 			{
/*N*/ 				//	not set in OldAttrs -> compare item value to default item
/*N*/ 				if ( *pThisItem == rThisSet.GetPool()->GetDefaultItem( nWhich ) )
/*N*/ 					rThisSet.ClearItem( nWhich );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScPatternAttr::HasItemsSet( const USHORT* pWhich ) const
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	for (USHORT i=0; pWhich[i]; i++)
/*N*/ 		if ( rSet.GetItemState( pWhich[i], FALSE ) == SFX_ITEM_SET )
/*N*/ 			return TRUE;
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void ScPatternAttr::ClearItems( const USHORT* pWhich )
/*N*/ {
/*N*/ 	SfxItemSet& rSet = GetItemSet();
/*N*/ 	for (USHORT i=0; pWhich[i]; i++)
/*N*/ 		rSet.ClearItem(pWhich[i]);
/*N*/ }

/*N*/ SfxStyleSheetBase* lcl_CopyStyleToPool
/*N*/ 	(
/*N*/ 		SfxStyleSheetBase*		pSrcStyle,
/*N*/ 		SfxStyleSheetBasePool*	pSrcPool,
/*N*/ 		SfxStyleSheetBasePool*	pDestPool
/*N*/ 	)
/*N*/ {
/*N*/ 	if ( !pSrcStyle || !pDestPool || !pSrcPool )
/*N*/ 	{
/*N*/ 		DBG_ERROR( "CopyStyleToPool: Invalid Arguments :-/" );
/*N*/ 		return NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	//--------------------------------------------------------
/*N*/ 
/*N*/ 	const String		 aStrSrcStyle = pSrcStyle->GetName();
/*N*/ 	const SfxStyleFamily eFamily	  = pSrcStyle->GetFamily();
/*N*/ 	SfxStyleSheetBase*	 pDestStyle   = pDestPool->Find( aStrSrcStyle, eFamily );
/*N*/ 
/*N*/ 	if ( !pDestStyle )
/*N*/ 	{
/*?*/ 		const String  aStrParent = pSrcStyle->GetParent();
/*?*/ 
/*?*/ 		pDestStyle = &pDestPool->Make( aStrSrcStyle, eFamily, SFXSTYLEBIT_USERDEF );
/*?*/ 		pDestStyle->GetItemSet().Put( pSrcStyle->GetItemSet() );
/*?*/ 
/*?*/ 		// ggF. abgeleitete Styles erzeugen, wenn nicht vorhanden:
/*?*/ 
/*?*/ 		if ( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) != aStrParent &&
/*?*/ 			 aStrSrcStyle != aStrParent &&
/*?*/ 			 !pDestPool->Find( aStrParent, eFamily ) )
/*?*/ 		{
/*?*/ 			lcl_CopyStyleToPool( pSrcPool->Find( aStrParent, eFamily ),
/*?*/ 								 pSrcPool, pDestPool );
/*?*/ 		}
/*?*/ 
/*?*/ 		pDestStyle->SetParent( aStrParent );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pDestStyle;
/*N*/ }

/*N*/ ScPatternAttr* ScPatternAttr::PutInPool( ScDocument* pDestDoc, ScDocument* pSrcDoc ) const
/*N*/ {
/*N*/ 	const SfxItemSet* pSrcSet = &GetItemSet();
/*N*/ 
/*N*/ 	ScPatternAttr* pDestPattern = new ScPatternAttr(pDestDoc->GetPool());
/*N*/ 	SfxItemSet* pDestSet = &pDestPattern->GetItemSet();
/*N*/ 
/*N*/ 	// Zellformatvorlage in anderes Dokument kopieren:
/*N*/ 
/*N*/ 	if ( pDestDoc != pSrcDoc )
/*N*/ 	{
/*N*/ 		DBG_ASSERT( pStyle, "Missing Pattern-Style! :-/" );
/*N*/ 
/*N*/ 		// wenn Vorlage im DestDoc vorhanden, dieses benutzen, sonst Style
/*N*/ 		// mit Parent-Vorlagen kopieren/ggF. erzeugen und dem DestDoc hinzufuegen
/*N*/ 
/*N*/ 		SfxStyleSheetBase* pStyleCpy = lcl_CopyStyleToPool( pStyle,
/*N*/ 															pSrcDoc->GetStyleSheetPool(),
/*N*/ 															pDestDoc->GetStyleSheetPool() );
/*N*/ 
/*N*/ 		pDestPattern->SetStyleSheet( (ScStyleSheet*)pStyleCpy );
/*N*/ 	}
/*N*/ 
/*N*/ 	for ( USHORT nAttrId = ATTR_PATTERN_START; nAttrId <= ATTR_PATTERN_END; nAttrId++ )
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pSrcItem;
/*N*/ 		SfxItemState eItemState = pSrcSet->GetItemState( nAttrId, FALSE, &pSrcItem );
/*N*/ 		if (eItemState==SFX_ITEM_ON)
/*N*/ 		{
/*N*/ 			SfxPoolItem* pNewItem = NULL;
/*N*/ 
/*N*/ 			if ( nAttrId == ATTR_CONDITIONAL )
/*N*/ 			{
/*?*/ 				//	Bedingte Formate ins neue Dokument kopieren
/*?*/ 
/*?*/ 				ULONG nNewIndex = 0;
/*?*/ 				ScConditionalFormatList* pSrcList = pSrcDoc->GetCondFormList();
/*?*/ 				if ( pSrcList )
/*?*/ 				{
/*?*/ 					ULONG nOldIndex = ((const SfxUInt32Item*)pSrcItem)->GetValue();
/*?*/ 					const ScConditionalFormat* pOldData = pSrcList->GetFormat( nOldIndex );
/*?*/ 					if ( pOldData )
/*?*/ 					{
/*?*/ 						nNewIndex = pDestDoc->AddCondFormat( *pOldData );
/*?*/ 
/*?*/ 						//	zugehoerige Styles auch mitkopieren
/*?*/ 						//!	nur wenn Format bei Add neu angelegt
/*?*/ 
/*?*/ 						ScStyleSheetPool* pSrcSPool = pSrcDoc->GetStyleSheetPool();
/*?*/ 						ScStyleSheetPool* pDestSPool = pDestDoc->GetStyleSheetPool();
/*?*/ 						USHORT nStlCnt = pOldData->Count();
/*?*/ 						for (USHORT i=0; i<nStlCnt; i++)
/*?*/ 						{
/*?*/ 							String aName = pOldData->GetEntry(i)->GetStyle();
/*?*/ 							SfxStyleSheetBase* pSrcStl =
/*?*/ 								pSrcDoc->GetStyleSheetPool()->Find(aName, SFX_STYLE_FAMILY_PARA);
/*?*/ 							lcl_CopyStyleToPool( pSrcStl, pSrcSPool, pDestSPool );
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 				pNewItem = new SfxUInt32Item( ATTR_CONDITIONAL, nNewIndex );
/*N*/ 			}
/*N*/ 			else if ( nAttrId == ATTR_VALIDDATA )
/*N*/ 			{
/*?*/ 				//	Gueltigkeit ins neue Dokument kopieren
/*?*/ 
/*?*/ 				ULONG nNewIndex = 0;
/*?*/ 				ScValidationDataList* pSrcList = pSrcDoc->GetValidationList();
/*?*/ 				if ( pSrcList )
/*?*/ 				{
/*?*/ 					ULONG nOldIndex = ((const SfxUInt32Item*)pSrcItem)->GetValue();
/*?*/ 					const ScValidationData* pOldData = pSrcList->GetData( nOldIndex );
/*?*/ 					if ( pOldData )
/*?*/ 						nNewIndex = pDestDoc->AddValidationEntry( *pOldData );
/*?*/ 				}
/*?*/ 				pNewItem = new SfxUInt32Item( ATTR_VALIDDATA, nNewIndex );
/*N*/ 			}
/*N*/ 			else if ( nAttrId == ATTR_VALUE_FORMAT && pDestDoc->GetFormatExchangeList() )
/*N*/ 			{
/*N*/ 				//	Zahlformate nach Exchange-Liste
/*N*/ 
/*N*/ 				ULONG nOldFormat = ((const SfxUInt32Item*)pSrcItem)->GetValue();
/*N*/ 				ULONG* pNewFormat = (ULONG*)pDestDoc->GetFormatExchangeList()->Get(nOldFormat);
/*N*/ 				if (pNewFormat)
/*?*/ 					pNewItem = new SfxUInt32Item( ATTR_VALUE_FORMAT, (UINT32) (*pNewFormat) );
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( pNewItem )
/*N*/ 			{
/*?*/ 				pDestSet->Put(*pNewItem);
/*?*/ 				delete pNewItem;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				pDestSet->Put(*pSrcItem);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	ScPatternAttr* pPatternAttr =
/*N*/ 		(ScPatternAttr*) &pDestDoc->GetPool()->Put(*pDestPattern);
/*N*/ 	delete pDestPattern;
/*N*/ 	return pPatternAttr;
/*N*/ }

/*N*/ BOOL ScPatternAttr::IsVisible() const
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	const SfxItemPool* pPool = rSet.GetPool();
/*N*/ 
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	SfxItemState eState;
/*N*/ 
/*N*/ 	eState = rSet.GetItemState( ATTR_BACKGROUND, TRUE, &pItem );
/*N*/ 	if ( eState == SFX_ITEM_SET )
/*N*/ 		if ( ((SvxBrushItem*)pItem)->GetColor().GetColor() != COL_TRANSPARENT )
/*N*/ 			return TRUE;
/*N*/ 
/*N*/ 	eState = rSet.GetItemState( ATTR_BORDER, TRUE, &pItem );
/*N*/ 	if ( eState == SFX_ITEM_SET )
/*N*/ 	{
/*N*/ 		SvxBoxItem* pBoxItem = (SvxBoxItem*) pItem;
/*N*/ 		if ( pBoxItem->GetTop() || pBoxItem->GetBottom() ||
/*N*/ 			 pBoxItem->GetLeft() || pBoxItem->GetRight() )
/*N*/ 			return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	eState = rSet.GetItemState( ATTR_SHADOW, TRUE, &pItem );
/*N*/ 	if ( eState == SFX_ITEM_SET )
/*?*/ 		if ( ((SvxShadowItem*)pItem)->GetLocation() != SVX_SHADOW_NONE )
/*?*/ 			return TRUE;
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ inline BOOL OneEqual( const SfxItemSet& rSet1, const SfxItemSet& rSet2, USHORT nId )
/*N*/ {
/*N*/ 	const SfxPoolItem* pItem1 = &rSet1.Get(nId);
/*N*/ 	const SfxPoolItem* pItem2 = &rSet2.Get(nId);
/*N*/ 	return ( pItem1 == pItem2 || *pItem1 == *pItem2 );
/*N*/ }

/*N*/ BOOL ScPatternAttr::IsVisibleEqual( const ScPatternAttr& rOther ) const
/*N*/ {
/*N*/ 	const SfxItemSet& rThisSet = GetItemSet();
/*N*/ 	const SfxItemSet& rOtherSet = rOther.GetItemSet();
/*N*/ 
/*N*/ 	return OneEqual( rThisSet, rOtherSet, ATTR_BACKGROUND ) &&
/*N*/ 			OneEqual( rThisSet, rOtherSet, ATTR_BORDER ) &&
/*N*/ 			OneEqual( rThisSet, rOtherSet, ATTR_SHADOW );
/*N*/ 
/*N*/ 	//!		auch hier nur wirklich sichtbare Werte testen !!!
/*N*/ }

/*N*/ const String* ScPatternAttr::GetStyleName() const
/*N*/ {
/*N*/ 	return pName ? pName : ( pStyle ? &pStyle->GetName() : NULL );
/*N*/ }


/*N*/ void ScPatternAttr::SetStyleSheet( ScStyleSheet* pNewStyle )
/*N*/ {
/*N*/ 	if (pNewStyle)
/*N*/ 	{
/*N*/ 		SfxItemSet&		  rPatternSet = GetItemSet();
/*N*/ 		const SfxItemSet& rStyleSet = pNewStyle->GetItemSet();
/*N*/ 
/*N*/ 		for (USHORT i=ATTR_PATTERN_START; i<=ATTR_PATTERN_END; i++)
/*N*/ 		{
/*N*/ 			if (rStyleSet.GetItemState(i, TRUE) == SFX_ITEM_SET)
/*N*/ 				rPatternSet.ClearItem(i);
/*N*/ 		}
/*N*/ 		rPatternSet.SetParent(&pNewStyle->GetItemSet());
/*N*/ 		pStyle = pNewStyle;
/*N*/ 		DELETEZ( pName );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ERROR( "ScPatternAttr::SetStyleSheet( NULL ) :-|" );
/*N*/ 		GetItemSet().SetParent(NULL);
/*N*/ 		pStyle = NULL;
/*N*/ 	}
/*N*/ }

/*N*/ void ScPatternAttr::UpdateStyleSheet()
/*N*/ {
/*N*/ 	if (pName)
/*N*/ 	{
/*N*/ 		pStyle = (ScStyleSheet*)pDoc->GetStyleSheetPool()->Find(*pName, SFX_STYLE_FAMILY_PARA);
/*N*/ 
/*N*/ 		//	wenn Style nicht gefunden, Standard nehmen,
/*N*/ 		//	damit keine leere Anzeige im Toolbox-Controller
/*N*/ 		//!	es wird vorausgesetzt, dass "Standard" immer der erste Eintrag ist!
/*N*/ 		if (!pStyle)
/*N*/ 		{
/*N*/ 			SfxStyleSheetIterator* pIter = pDoc->GetStyleSheetPool()->CreateIterator(
/*N*/ 													SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL );
/*N*/ 			pStyle = (ScStyleSheet*)pIter->First();
/*N*/ 		}
/*N*/ 
/*N*/ 		if (pStyle)
/*N*/ 		{
/*N*/ 			GetItemSet().SetParent(&pStyle->GetItemSet());
/*N*/ 			DELETEZ( pName );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*?*/ 		pStyle = NULL;
/*N*/ }


/*N*/ BOOL ScPatternAttr::IsSymbolFont() const
/*N*/ {
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	if( GetItemSet().GetItemState( ATTR_FONT, TRUE, &pItem ) == SFX_ITEM_SET )
/*N*/ 		return BOOL( ((const SvxFontItem*) pItem)->GetCharSet()
/*N*/ 			== RTL_TEXTENCODING_SYMBOL );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }

/*N*/ FontToSubsFontConverter ScPatternAttr::GetSubsFontConverter( ULONG nFlags ) const
/*N*/ {
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	if( GetItemSet().GetItemState( ATTR_FONT, TRUE, &pItem ) == SFX_ITEM_SET )
/*N*/         return CreateFontToSubsFontConverter(
/*N*/             ((const SvxFontItem*) pItem)->GetFamilyName(), nFlags );
/*N*/ 	else
/*N*/         return 0;
/*N*/ }


/*N*/ ULONG ScPatternAttr::GetNumberFormat( SvNumberFormatter* pFormatter ) const
/*N*/ {
/*N*/ 	ULONG nFormat =
/*N*/ 		((SfxUInt32Item*)&GetItemSet().Get( ATTR_VALUE_FORMAT ))->GetValue();
/*N*/ 	LanguageType eLang =
/*N*/ 		((SvxLanguageItem*)&GetItemSet().Get( ATTR_LANGUAGE_FORMAT ))->GetLanguage();
/*N*/ 	if ( nFormat < SV_COUNTRY_LANGUAGE_OFFSET && eLang == LANGUAGE_SYSTEM )
/*N*/ 		;		// es bleibt wie es ist
/*N*/ 	else if ( pFormatter )
/*N*/ 		nFormat = pFormatter->GetFormatForLanguageIfBuiltIn( nFormat, eLang );
/*N*/ 	return nFormat;
/*N*/ }

//	dasselbe, wenn bedingte Formatierung im Spiel ist:

/*N*/ ULONG ScPatternAttr::GetNumberFormat( SvNumberFormatter* pFormatter,
/*N*/ 										const SfxItemSet* pCondSet ) const
/*N*/ {
/*N*/ 	DBG_ASSERT(pFormatter,"GetNumberFormat ohne Formatter");
/*N*/ 
/*N*/ 	const SfxPoolItem* pFormItem;
/*N*/ 	if ( !pCondSet || pCondSet->GetItemState(ATTR_VALUE_FORMAT,TRUE,&pFormItem) != SFX_ITEM_SET )
/*N*/ 		pFormItem = &GetItemSet().Get(ATTR_VALUE_FORMAT);
/*N*/ 
/*N*/ 	const SfxPoolItem* pLangItem;
/*N*/ 	if ( !pCondSet || pCondSet->GetItemState(ATTR_LANGUAGE_FORMAT,TRUE,&pLangItem) != SFX_ITEM_SET )
/*N*/ 		pLangItem = &GetItemSet().Get(ATTR_LANGUAGE_FORMAT);
/*N*/ 
/*N*/ 	return pFormatter->GetFormatForLanguageIfBuiltIn(
/*N*/ 					((SfxUInt32Item*)pFormItem)->GetValue(),
/*N*/ 					((SvxLanguageItem*)pLangItem)->GetLanguage() );
/*N*/ }

/*N*/ const SfxPoolItem& ScPatternAttr::GetItem( USHORT nWhich, const SfxItemSet* pCondSet ) const
/*N*/ {
/*N*/ 	const SfxPoolItem* pCondItem;
/*N*/ 	if ( pCondSet && pCondSet->GetItemState( nWhich, TRUE, &pCondItem ) == SFX_ITEM_SET )
/*?*/ 		return *pCondItem;
/*N*/ 
/*N*/ 	return GetItemSet().Get(nWhich);
/*N*/ }
}
