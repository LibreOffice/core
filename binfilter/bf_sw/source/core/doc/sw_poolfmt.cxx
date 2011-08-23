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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#ifndef _SVX_WGHTITEM_HXX //autogen
#include <bf_svx/wghtitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <bf_svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <bf_svx/fhgtitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <bf_svx/tstpitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <bf_svx/ulspitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <bf_svx/adjitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <bf_svx/postitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <bf_svx/keepitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <bf_svx/opaqitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <bf_svx/boxitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <bf_svx/cmapitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <bf_svx/udlnitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <bf_svx/colritem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <bf_svx/protitem.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <bf_svx/paperinf.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX
#include <bf_svx/escpitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <bf_svx/langitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <bf_svx/charrotateitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <bf_svx/frmdiritem.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX
#include <bf_svx/emphitem.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef SW_FMTLINE_HXX
#include <fmtline.hxx>
#endif

#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif
#ifndef _GETMETRICVAL_HXX
#include <GetMetricVal.hxx>
#endif
namespace binfilter {
const USHORT PT_3	=  3 * 20;		//  3 pt
const USHORT PT_6	=  6 * 20;		//  6 pt
const USHORT PT_7	=  7 * 20;		//  6 pt
const USHORT PT_8	=  8 * 20;		//  8 pt
const USHORT PT_9	=  9 * 20;		//  9 pt
const USHORT PT_10 	= 10 * 20;		// 10 pt
const USHORT PT_11 	= 11 * 20;		// 11 pt
const USHORT PT_12 	= 12 * 20;		// 12 pt
const USHORT PT_14	= 14 * 20;		// 14 pt
const USHORT PT_16	= 16 * 20;		// 16 pt
const USHORT PT_18	= 18 * 20;		// 18 pt
const USHORT PT_22	= 22 * 20;		// 22 pt
const USHORT PT_24	= 24 * 20;		// 22 pt


//const USHORT HTML_PARSPACE = ((CM_05 * 7) / 10);
#define HTML_PARSPACE 	GetMetricVal( CM_05 )

static const sal_Char __FAR_DATA sKomma[] = ", ";

static const USHORT aHeadlineSizes[ 2 * MAXLEVEL ] = {
//	PT_16, PT_14, PT_14, PT_12, PT_12, 			// normal
//JP 10.12.96: jetzt soll alles prozentual sein:
    115, 100, 100, 85, 85,
    75,   75,  75, 75, 75,	// normal
//	PT_22, PT_16, PT_12, PT_11, PT_9			// HTML-Mode
    PT_24, PT_18, PT_14, PT_12, PT_10,
    PT_7, PT_7, PT_7, PT_7, PT_7 			// HTML-Mode
};

/*N*/ long lcl_GetRightMargin( SwDoc& rDoc )
/*N*/ {
/*N*/ 	// sorge dafuer, dass die Druckereinstellungen in die Standard-
/*N*/ 	// Seitenvorlage uebernommen wurden.
/*N*/ 	long nLeft, nRight, nWidth;
/*N*/ 	const SfxPrinter* pPrt = rDoc.GetPrt( !rDoc.IsInReading() );
/*N*/ 	if( !pPrt )
/*N*/ 	{
/*N*/ 		SvxPaper ePaper;
/*N*/ 		if( MEASURE_METRIC == GetAppLocaleData().getMeasurementSystemEnum() )
/*N*/ 		{
/*?*/ 			ePaper = SVX_PAPER_A4;
/*?*/ 			nLeft = nRight = 1134;	//2 Zentimeter
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			ePaper = SVX_PAPER_LEGAL;
/*N*/ 			nLeft = nRight = 1800;	// 1,25 Inch
/*N*/ 		}
/*N*/
/*N*/ 		nWidth = SvxPaperInfo::GetPaperSize( ePaper ).Width();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const SwFrmFmt& rPgDscFmt = rDoc.GetPageDesc( 0 ).GetMaster();
/*N*/ 		const SvxLRSpaceItem& rLR = rPgDscFmt.GetLRSpace();
/*N*/ 		nLeft = rLR.GetLeft();
/*N*/ 		nRight = rLR.GetRight();
/*N*/ 		nWidth = rPgDscFmt.GetFrmSize().GetWidth();
/*N*/ 	}
/*N*/ 	return nWidth - nLeft - nRight;
/*N*/ }

/*N*/ void SetAllScriptItem( SfxItemSet& rSet, const SfxPoolItem& rItem )
/*N*/ {
/*N*/ 	rSet.Put( rItem );
/*N*/ 	USHORT nWhCJK = 0, nWhCTL = 0;
/*N*/ 	switch( rItem.Which() )
/*N*/ 	{
/*N*/ 	case RES_CHRATR_FONTSIZE:
/*N*/ 		nWhCJK = RES_CHRATR_CJK_FONTSIZE, nWhCTL = RES_CHRATR_CTL_FONTSIZE;
/*N*/ 		break;
/*N*/ 	case RES_CHRATR_FONT:
/*N*/ 		nWhCJK = RES_CHRATR_CJK_FONT, nWhCTL = RES_CHRATR_CTL_FONT;
/*N*/ 		break;
/*?*/ 	case RES_CHRATR_LANGUAGE:
/*?*/ 		nWhCJK = RES_CHRATR_CJK_LANGUAGE, nWhCTL = RES_CHRATR_CTL_LANGUAGE;
/*?*/ 		break;
/*N*/ 	case RES_CHRATR_POSTURE:
/*N*/ 		nWhCJK = RES_CHRATR_CJK_POSTURE, nWhCTL = RES_CHRATR_CTL_POSTURE;
/*N*/ 		break;
/*N*/ 	case RES_CHRATR_WEIGHT:
/*N*/ 		nWhCJK = RES_CHRATR_CJK_WEIGHT, nWhCTL = RES_CHRATR_CTL_WEIGHT;
/*N*/ 		break;
/*N*/ 	}
/*N*/
/*N*/ 	if( nWhCJK )
/*N*/ 		rSet.Put( rItem, nWhCJK );
/*N*/ 	if( nWhCTL )
/*N*/ 		rSet.Put( rItem, nWhCTL );
/*N*/ }

/*N*/ void lcl_SetDfltFont( USHORT nFntType, SfxItemSet& rSet )
/*N*/ {
/*N*/ 	static struct {
/*N*/ 		USHORT nResLngId;
/*N*/ 		USHORT nResFntId;
/*N*/ 	} aArr[ 3 ] = {
/*N*/ 		{ RES_CHRATR_LANGUAGE, RES_CHRATR_FONT },
/*N*/ 		{ RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_FONT },
/*N*/ 		{ RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_FONT }
/*N*/ 	};
/*N*/ 	for( USHORT n = 0; n < 3; ++n )
/*N*/ 	{
/*N*/ 		USHORT nLng = ((SvxLanguageItem&)rSet.GetPool()->GetDefaultItem(
/*N*/ 							aArr[n].nResLngId )).GetLanguage();
/*N*/ 		Font aFnt( OutputDevice::GetDefaultFont( nFntType,
/*N*/ 								nLng, DEFAULTFONT_FLAGS_ONLYONE ) );
/*N*/
/*N*/ 		rSet.Put( SvxFontItem( aFnt.GetFamily(), aFnt.GetName(),
/*N*/ 							aEmptyStr, aFnt.GetPitch(),
/*N*/ 							aFnt.GetCharSet(), aArr[n].nResFntId ));
/*N*/ 	}
/*N*/ }


/*N*/ void lcl_SetHeadline( SwDoc* pDoc, SwTxtFmtColl* pColl,
/*N*/ 						SfxItemSet& rSet,
/*N*/ 						USHORT nOutLvlBits, BYTE nLevel, BOOL bItalic )
/*N*/ {
/*N*/ 	SetAllScriptItem( rSet, SvxWeightItem( WEIGHT_BOLD ) );
/*N*/ 	SvxFontHeightItem aHItem;
/*N*/ 	if( pDoc->IsHTMLMode() )
/*?*/ 		aHItem.SetHeight( aHeadlineSizes[ MAXLEVEL + nLevel ] );
/*N*/ 	else
/*N*/ 		aHItem.SetHeight( PT_14, aHeadlineSizes[ nLevel ] );
/*N*/ 	SetAllScriptItem( rSet, aHItem );
/*N*/
/*N*/ 	if( bItalic && !pDoc->IsHTMLMode() )
/*N*/ 		SetAllScriptItem( rSet, SvxPostureItem( ITALIC_NORMAL ) );
/*N*/
/*N*/ 	if( pDoc->IsHTMLMode() )
/*N*/ 	{
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	::lcl_SetDfltFont( DEFAULTFONT_LATIN_TEXT, DEFAULTFONT_CJK_TEXT,
/*N*/ 	}
/*N*/
/*N*/ 	if( pColl )
/*N*/ 	{
/*N*/ 		if( !( nOutLvlBits & ( 1 << nLevel )) )
/*N*/ 		{
/*N*/ 			pColl->SetOutlineLevel( nLevel );
/*N*/ 			if( !pDoc->IsHTMLMode() )
/*N*/ 			{
/*N*/ 				const SwNumFmt& rNFmt = pDoc->GetOutlineNumRule()->Get( nLevel );
/*N*/ 				if( rNFmt.GetAbsLSpace() || rNFmt.GetFirstLineOffset() )
/*N*/ 				{
/*?*/ 					SvxLRSpaceItem aLR( (SvxLRSpaceItem&)pColl->GetAttr( RES_LR_SPACE ) );
/*?*/ 					aLR.SetTxtFirstLineOfstValue( rNFmt.GetFirstLineOffset() );
/*?*/ 					aLR.SetTxtLeft( rNFmt.GetAbsLSpace() );
/*?*/ 					pColl->SetAttr( aLR );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pColl->SetNextTxtFmtColl( *pDoc->GetTxtCollFromPool(
/*N*/ 										RES_POOLCOLL_TEXT ));
/*N*/ 	}
/*N*/ }



/*N*/ void lcl_SetRegister( SwDoc* pDoc, SfxItemSet& rSet, USHORT nFact,
/*N*/ 						BOOL bHeader, BOOL bTab )
/*N*/ {
/*N*/ 	SvxLRSpaceItem aLR;
/*N*/ 	USHORT nLeft = nFact ? GetMetricVal( CM_05 ) * nFact : 0;
/*N*/ 	aLR.SetTxtLeft( nLeft );
/*N*/
/*N*/ 	rSet.Put( aLR );
/*N*/ 	if( bHeader )
/*N*/ 	{
/*N*/ 		SetAllScriptItem( rSet, SvxWeightItem( WEIGHT_BOLD ) );
/*N*/ 		SetAllScriptItem( rSet, SvxFontHeightItem( PT_16 ) );
/*N*/ 	}
/*N*/ 	if( bTab )
/*N*/ 	{
/*N*/ 		long nRightMargin = lcl_GetRightMargin( *pDoc );
/*N*/ 		SvxTabStopItem aTStops( 0, 0 );
/*N*/ 		aTStops.Insert( SvxTabStop( nRightMargin - nLeft,
/*N*/ 									SVX_TAB_ADJUST_RIGHT,
/*N*/ 									cDfltDecimalChar, '.' ));
/*N*/ 		rSet.Put( aTStops );
/*N*/ 	}
/*N*/ }



/*N*/ void lcl_SetNumBul( SwDoc* pDoc, SwTxtFmtColl* pColl,
/*N*/ 						SfxItemSet& rSet,
/*N*/ 						USHORT nNxt, SwTwips nEZ, SwTwips nLeft,
/*N*/ 						SwTwips nUpper, SwTwips nLower )
/*N*/ {
/*N*/
/*N*/ 	SvxLRSpaceItem aLR; SvxULSpaceItem aUL;
/*N*/ 	aLR.SetTxtFirstLineOfst( USHORT(nEZ) ); aLR.SetTxtLeft( USHORT(nLeft) );
/*N*/ 	aUL.SetUpper( USHORT(nUpper) ); aUL.SetLower( USHORT(nLower) );
/*N*/ 	rSet.Put( aLR );
/*N*/ 	rSet.Put( aUL );
/*N*/
/*N*/ 	if( !pColl )
/*?*/ 		pColl->SetNextTxtFmtColl( *pDoc->GetTxtCollFromPool( nNxt ));
/*N*/ }



    // Gebe die "Auto-Collection" mit der Id zurueck. Existiert
    // sie noch nicht, dann erzeuge sie
    // Ist der String-Pointer definiert, dann erfrage nur die
    // Beschreibung der Attribute, !! es legt keine Vorlage an !!

/*N*/ SvxFrameDirection GetDefaultFrameDirection(ULONG nLanguage)
/*N*/ {
/*N*/     SvxFrameDirection eResult = FRMDIR_HORI_LEFT_TOP;
/*N*/
/*N*/     switch (nLanguage)
/*N*/     {
/*N*/     case LANGUAGE_ARABIC_PRIMARY_ONLY:
/*N*/     case LANGUAGE_ARABIC_SAUDI_ARABIA:
/*N*/     case LANGUAGE_ARABIC_IRAQ:
/*N*/     case LANGUAGE_ARABIC_EGYPT:
/*N*/     case LANGUAGE_ARABIC_LIBYA:
/*N*/     case LANGUAGE_ARABIC_ALGERIA:
/*N*/     case LANGUAGE_ARABIC_MOROCCO:
/*N*/     case LANGUAGE_ARABIC_TUNISIA:
/*N*/     case LANGUAGE_ARABIC_OMAN:
/*N*/     case LANGUAGE_ARABIC_YEMEN:
/*N*/     case LANGUAGE_ARABIC_SYRIA:
/*N*/     case LANGUAGE_ARABIC_JORDAN:
/*N*/     case LANGUAGE_ARABIC_LEBANON:
/*N*/     case LANGUAGE_ARABIC_KUWAIT:
/*N*/     case LANGUAGE_ARABIC_UAE:
/*N*/     case LANGUAGE_ARABIC_BAHRAIN:
/*N*/     case LANGUAGE_ARABIC_QATAR:
/*N*/     case LANGUAGE_HEBREW:
/*N*/     case LANGUAGE_URDU:
/*N*/         eResult = FRMDIR_HORI_RIGHT_TOP;
/*N*/
/*N*/         break;
/*N*/
/*N*/     default:
/*N*/         break;
/*N*/     }
/*N*/
/*N*/     return eResult;
/*N*/ }

/*N*/ SwTxtFmtColl* SwDoc::GetTxtCollFromPool
/*N*/ ( USHORT nId, String* pDesc, SfxItemPresentation ePres,
/*N*/   SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, BOOL bRegardLanguage)
/*N*/ {
/*N*/ 	ASSERT(
/*N*/ 		(RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END) ||
/*N*/ 		(RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END) ||
/*N*/ 		(RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END) ||
/*N*/ 		(RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END) ||
/*N*/ 		(RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END) ||
/*N*/ 		(RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END),
/*N*/ 			"Falsche AutoFormat-Id" );
/*N*/
/*N*/ 	SwTxtFmtColl* pNewColl;
/*N*/ 	USHORT nOutLvlBits = 0;
/*N*/ 	for( USHORT n = 0; n < pTxtFmtCollTbl->Count(); ++n )
/*N*/ 	{
/*N*/ 		if( nId == ( pNewColl = (*pTxtFmtCollTbl)[ n ] )->GetPoolFmtId() )
/*N*/ 		{
/*N*/ 			if( pDesc )
/*N*/ 				pNewColl->GetPresentation( ePres, eCoreMetric,
/*N*/ 										   ePresMetric, *pDesc );
/*N*/ 			return pNewColl;
/*N*/ 		}
/*N*/ 		if( !pDesc && pNewColl->GetOutlineLevel() < MAXLEVEL )
/*N*/ 			nOutLvlBits |= ( 1 << pNewColl->GetOutlineLevel() );
/*N*/ 	}
/*N*/
/*N*/ 	// bis hierher nicht gefunden -> neu anlegen
/*N*/ 	USHORT nResId = 0;
/*N*/ 	if( RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END )
/*N*/ 		nResId = RC_POOLCOLL_TEXT_BEGIN - RES_POOLCOLL_TEXT_BEGIN;
/*N*/ 	else if (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END)
/*N*/ 		nResId = RC_POOLCOLL_LISTS_BEGIN - RES_POOLCOLL_LISTS_BEGIN;
/*N*/ 	else if (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END)
/*N*/ 		nResId = RC_POOLCOLL_EXTRA_BEGIN - RES_POOLCOLL_EXTRA_BEGIN;
/*N*/ 	else if (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END)
/*N*/ 		nResId = RC_POOLCOLL_REGISTER_BEGIN - RES_POOLCOLL_REGISTER_BEGIN;
/*N*/ 	else if (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END)
/*N*/ 		nResId = RC_POOLCOLL_DOC_BEGIN - RES_POOLCOLL_DOC_BEGIN;
/*N*/ 	else if (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END)
/*N*/ 		nResId = RC_POOLCOLL_HTML_BEGIN - RES_POOLCOLL_HTML_BEGIN;
/*N*/
/*N*/ 	ASSERT( nResId, "Ungueltige Pool-ID" );
/*N*/ 	if( !nResId )
/*N*/ 		return GetTxtCollFromPool( RES_POOLCOLL_STANDARD, pDesc, ePres,
/*N*/ 									eCoreMetric, ePresMetric );
/*N*/
/*N*/ 	ResId aResId( nResId + nId, *pSwResMgr );
/*N*/ 	String aNm( aResId );
/*N*/
/*N*/ 	// ein Set fuer alle zusetzenden Attribute
/*N*/ 	SwAttrSet aSet( GetAttrPool(), aTxtFmtCollSetRange );
/*N*/ 	USHORT nParent = GetPoolParent( nId );
/*N*/ 	if( pDesc )
/*N*/ 	{
/*?*/ 		pNewColl = 0;
/*?*/ 		if( nParent )
/*?*/ 			*pDesc = SW_RESSTR( nResId + nParent );
/*?*/ 		else
/*?*/ 			*pDesc = aEmptyStr;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/
/*N*/ //FEATURE::CONDCOLL
/*N*/ 		if(::binfilter::IsConditionalByPoolId( nId ))
/*N*/ 			pNewColl = new SwConditionTxtFmtColl( GetAttrPool(), aNm, !nParent
/*N*/ 												? pDfltTxtFmtColl
/*N*/ 												: GetTxtCollFromPool( nParent ));
/*N*/ 		else
/*N*/ //FEATURE::CONDCOLL
/*N*/ 		pNewColl = new SwTxtFmtColl( GetAttrPool(), aNm, !nParent
/*N*/ 											? pDfltTxtFmtColl
/*N*/ 											: GetTxtCollFromPool( nParent ));
/*N*/ 		pNewColl->SetPoolFmtId( nId );
/*N*/ 		pTxtFmtCollTbl->Insert( pNewColl, pTxtFmtCollTbl->Count() );
/*N*/ 	}
/*N*/
/*N*/ 	switch( nId )
/*N*/ 	{
/*N*/ 	// allgemeine Inhaltsformen
/*N*/ 	case RES_POOLCOLL_STANDARD:
/*N*/         if (bRegardLanguage &&
/*N*/             GetDefaultFrameDirection(GetAppLanguage()) ==
/*N*/             FRMDIR_HORI_RIGHT_TOP)
/*N*/         {
/*N*/             SvxAdjustItem aAdjust(SVX_ADJUST_RIGHT);
/*N*/             aSet.Put(aAdjust);
/*N*/         }
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_TEXT:					// Textkoerper
/*N*/ 		{
/*N*/ 			SvxULSpaceItem aUL( 0, PT_6 );
/*N*/ 			if( IsHTMLMode() ) aUL.SetLower( HTML_PARSPACE );
/*N*/ 			aSet.Put( aUL );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TEXT_IDENT:			// Textkoerper Einzug
/*N*/ 		{
/*N*/ 			SvxLRSpaceItem aLR;
/*N*/ 			aLR.SetTxtFirstLineOfst( GetMetricVal( CM_05 ));
/*N*/ 			aSet.Put( aLR );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TEXT_NEGIDENT:		// Textkoerper neg. Einzug
/*N*/ 		{
/*N*/ 			SvxLRSpaceItem aLR;
/*N*/ 			aLR.SetTxtFirstLineOfst( -(short)GetMetricVal( CM_05 ));
/*N*/ 			aLR.SetTxtLeft( GetMetricVal( CM_1 ));
/*N*/ 			SvxTabStopItem aTStops; 	aTStops.Insert( SvxTabStop( 0 ));
/*N*/
/*N*/ 			aSet.Put( aLR );
/*N*/ 			aSet.Put( aTStops );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TEXT_MOVE:			// Textkoerper Einrueckung
/*N*/ 		{
/*N*/ 			SvxLRSpaceItem aLR;
/*N*/ 			aLR.SetTxtLeft( GetMetricVal( CM_05 ));
/*N*/ 			aSet.Put( aLR );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_CONFRONTATION: 	// Textkoerper Gegenueberstellung
/*N*/ 		{
/*N*/ 			SvxLRSpaceItem aLR;
/*N*/ 			aLR.SetTxtFirstLineOfst( - short( GetMetricVal( CM_1 ) * 4 +
/*N*/ 											  GetMetricVal( CM_05)) );
/*N*/ 			aLR.SetTxtLeft( GetMetricVal( CM_1 ) * 5 );
/*N*/ 			SvxTabStopItem aTStops; 	aTStops.Insert( SvxTabStop( 0 ));
/*N*/
/*N*/ 			aSet.Put( aLR );
/*N*/ 			aSet.Put( aTStops );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_MARGINAL: 		// Textkoerper maginalie
/*N*/ 		{
/*N*/ 			SvxLRSpaceItem aLR;
/*N*/ 			aLR.SetTxtLeft( GetMetricVal( CM_1 ) * 4 );
/*N*/ 			aSet.Put( aLR );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_HEADLINE_BASE:			// Basis Ueberschrift
/*N*/ 		{
/*N*/ 			static const USHORT aFntInit[] = {
/*N*/ 				DEFAULTFONT_LATIN_HEADING, 	RES_CHRATR_FONT,
/*N*/ 								RES_CHRATR_LANGUAGE, LANGUAGE_ENGLISH_US,
/*N*/ 				DEFAULTFONT_CJK_HEADING, 	RES_CHRATR_CJK_FONT,
/*N*/ 								RES_CHRATR_CJK_LANGUAGE, LANGUAGE_ENGLISH_US,
/*N*/ 				DEFAULTFONT_CTL_HEADING, 	RES_CHRATR_CTL_FONT,
/*N*/ 								RES_CHRATR_CTL_LANGUAGE, LANGUAGE_ARABIC_SAUDI_ARABIA,
/*N*/ 				0
/*N*/ 			};
/*N*/
/*N*/ 			for( const USHORT* pArr = aFntInit; *pArr; pArr += 4 )
/*N*/ 			{
/*N*/ 				USHORT nLng = ((SvxLanguageItem&)GetDefault( *(pArr+2) )).GetLanguage();
/*N*/ 				if( LANGUAGE_DONTKNOW == nLng )
/*N*/ 					nLng = *(pArr+3);
/*N*/
/*N*/ 				Font aFnt( OutputDevice::GetDefaultFont( *pArr,
/*N*/ 										nLng, DEFAULTFONT_FLAGS_ONLYONE ) );
/*N*/
/*N*/ 				aSet.Put( SvxFontItem( aFnt.GetFamily(), aFnt.GetName(),
/*N*/ 										aEmptyStr, aFnt.GetPitch(),
/*N*/ 										aFnt.GetCharSet(), *(pArr+1) ));
/*N*/ 			}
/*N*/
/*N*/ 			SvxFontHeightItem aFntSize( PT_14 );
/*N*/ 			SvxULSpaceItem aUL( PT_12, PT_6 );
/*N*/ 			if( IsHTMLMode() )
/*?*/ 				aUL.SetLower( HTML_PARSPACE );
/*N*/ 			aSet.Put( SvxFmtKeepItem( TRUE ));
/*N*/
/*N*/ 			if( !pDesc )
/*N*/ 				pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
/*N*/ 												RES_POOLCOLL_TEXT ));
/*N*/
/*N*/ 			aSet.Put( aUL );
/*N*/ 			SetAllScriptItem( aSet, aFntSize );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_NUMBUL_BASE:			// Basis Numerierung/Aufzaehlung
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_GREETING:				// Grussformel
/*N*/ 	case RES_POOLCOLL_REGISTER_BASE: 		// Basis Verzeichnisse
/*N*/ 	case RES_POOLCOLL_SIGNATURE:			// Unterschrift
/*N*/ 	case RES_POOLCOLL_TABLE:				// Tabelle-Inhalt
/*N*/ 		{
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_HEADLINE1:		// Ueberschrift 1
/*N*/ 		lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 0, FALSE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_HEADLINE2:		// Ueberschrift 2
/*N*/ 		lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 1, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_HEADLINE3:		// Ueberschrift 3
/*N*/ 		lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 2, FALSE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_HEADLINE4:		// Ueberschrift 4
/*N*/ 		lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 3, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_HEADLINE5:		// Ueberschrift 5
/*N*/ 		lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 4, FALSE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_HEADLINE6:		// Ueberschrift 6
/*N*/ 		lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 5, FALSE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_HEADLINE7:		// Ueberschrift 7
/*N*/ 		lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 6, FALSE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_HEADLINE8:		// Ueberschrift 8
/*N*/ 		lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 7, FALSE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_HEADLINE9:		// Ueberschrift 9
/*N*/ 		lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 8, FALSE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_HEADLINE10:		// Ueberschrift 10
/*N*/ 		lcl_SetHeadline( this, pNewColl, aSet, nOutLvlBits, 9, FALSE );
/*N*/ 		break;
/*N*/
/*N*/
/*N*/ 	// Sonderbereiche:
/*N*/ 	// Kopfzeilen
/*N*/ 	case RES_POOLCOLL_HEADER:
/*N*/ 	case RES_POOLCOLL_HEADERL:
/*N*/ 	case RES_POOLCOLL_HEADERR:
/*N*/ 	// Fusszeilen
/*N*/ 	case RES_POOLCOLL_FOOTER:
/*N*/ 	case RES_POOLCOLL_FOOTERL:
/*N*/ 	case RES_POOLCOLL_FOOTERR:
/*N*/ 		{
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/
/*N*/ 			long nRightMargin = lcl_GetRightMargin( *this );
/*N*/
/*N*/ 			SvxTabStopItem aTStops( 0, 0 );
/*N*/ 			aTStops.Insert( SvxTabStop( nRightMargin / 2, SVX_TAB_ADJUST_CENTER ) );
/*N*/ 			aTStops.Insert( SvxTabStop( nRightMargin, SVX_TAB_ADJUST_RIGHT ) );
/*N*/
/*N*/ 			aSet.Put( aTStops );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_TABLE_HDLN:
/*N*/ 		{
/*N*/ 			SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD ) );
/*N*/ 			if( !IsHTMLMode() )
/*N*/ 				SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL ) );
/*N*/ 			aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER ) );
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_FOOTNOTE:				// Fussnote
/*N*/ 	case RES_POOLCOLL_ENDNOTE:
/*N*/ 		{
/*N*/ 			SvxLRSpaceItem aLR;
/*N*/ 			aLR.SetTxtFirstLineOfst( -(short)GetMetricVal( CM_05 ));
/*N*/ 			aLR.SetTxtLeft( GetMetricVal( CM_05 ));
/*N*/ 			SetAllScriptItem( aSet, SvxFontHeightItem( PT_10 ) );
/*N*/ 			aSet.Put( aLR );
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_LABEL:				// Beschriftung-Basis
/*N*/ 		{
/*N*/ 			SvxULSpaceItem aUL; aUL.SetUpper( PT_6 ); aUL.SetLower( PT_6 );
/*N*/ 			aSet.Put( aUL );
/*N*/ 			SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL ) );
/*N*/ 			SetAllScriptItem( aSet, SvxFontHeightItem( PT_10 ) );
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_FRAME:				// Rahmen Inhalt
/*N*/ 	case RES_POOLCOLL_LABEL_ABB:			// Beschriftung-Abbildung
/*N*/ 	case RES_POOLCOLL_LABEL_TABLE:			// Beschriftung-Tabelle
/*N*/ 	case RES_POOLCOLL_LABEL_FRAME:			// Beschriftung-Rahmen
/*N*/ 	case RES_POOLCOLL_LABEL_DRAWING:		// Beschriftung-Zeichnung
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_JAKETADRESS:			// UmschlagAdresse
/*N*/ 		{
/*N*/ 			SvxULSpaceItem aUL; aUL.SetLower( PT_3 );
/*N*/ 			aSet.Put( aUL );
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_SENDADRESS:			// AbsenderAdresse
/*N*/ 		{
/*N*/ 			if( IsHTMLMode() )
/*?*/ 				SetAllScriptItem( aSet, SvxPostureItem(ITALIC_NORMAL) );
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SvxULSpaceItem aUL; aUL.SetLower( PT_3 );
/*N*/ 				aSet.Put( aUL );
/*N*/ 			}
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	// Benutzer-Verzeichnisse:
/*N*/ 	case RES_POOLCOLL_TOX_USERH:			// Header
/*N*/ 		lcl_SetRegister( this, aSet, 0, TRUE, FALSE );
/*N*/ 		{
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_USER1:			// 1. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 0, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_USER2:			// 2. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 1, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_USER3:			// 3. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 2, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_USER4:			// 4. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 3, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_USER5:			// 5. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 4, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_USER6:			// 6. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 5, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_USER7:			// 7. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 6, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_USER8:			// 8. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 7, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_USER9:			// 9. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 8, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_USER10:			// 10. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 9, FALSE, TRUE );
/*N*/ 		break;
/*N*/
/*N*/ 	// Index-Verzeichnisse
/*N*/ 	case RES_POOLCOLL_TOX_IDXH:			// Header
/*N*/ 		lcl_SetRegister( this, aSet, 0, TRUE, FALSE );
/*N*/ 		{
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_IDX1:			// 1. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 0, FALSE, FALSE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_IDX2:			// 2. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 1, FALSE, FALSE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_IDX3:			// 3. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 2, FALSE, FALSE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_IDXBREAK:		// Trenner
/*N*/ 		lcl_SetRegister( this, aSet, 0, FALSE, FALSE );
/*N*/ 		break;
/*N*/
/*N*/ 	// Inhalts-Verzeichnisse
/*N*/ 	case RES_POOLCOLL_TOX_CNTNTH:		// Header
/*N*/ 		lcl_SetRegister( this, aSet, 0, TRUE, FALSE );
/*N*/ 		{
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_CNTNT1:		// 1. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 0, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_CNTNT2:		// 2. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 1, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_CNTNT3:		// 3. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 2, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_CNTNT4:		// 4. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 3, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_CNTNT5:		// 5. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 4, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_CNTNT6:		// 6. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 5, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_CNTNT7:		// 7. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 6, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_CNTNT8:		// 8. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 7, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_CNTNT9:		// 9. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 8, FALSE, TRUE );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_CNTNT10:		// 10. Ebene
/*N*/ 		lcl_SetRegister( this, aSet, 9, FALSE, TRUE );
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_TOX_ILLUSH:
/*N*/ 	case RES_POOLCOLL_TOX_OBJECTH:
/*N*/ 	case RES_POOLCOLL_TOX_TABLESH:
/*N*/ 	case RES_POOLCOLL_TOX_AUTHORITIESH:
/*N*/ 		lcl_SetRegister( this, aSet, 0, TRUE, FALSE );
/*N*/ 		{
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_TOX_ILLUS1:
/*N*/ 	case RES_POOLCOLL_TOX_OBJECT1:
/*N*/ 	case RES_POOLCOLL_TOX_TABLES1:
/*N*/ 	case RES_POOLCOLL_TOX_AUTHORITIES1:
/*N*/ 		lcl_SetRegister( this, aSet, 0, FALSE, TRUE );
/*N*/ 	break;
/*N*/
/*N*/
/*N*/
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL1S:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
/*N*/ 						PT_12, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL1:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
/*N*/ 						0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL1E:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
/*N*/ 						0, PT_12 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_NONUM1:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM1,
/*N*/ 						0, SwNumRule::GetNumIndent( 0 ), 0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL2S:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
/*N*/ 						PT_12, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL2:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
/*N*/ 						0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL2E:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
/*N*/ 						0, PT_12 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_NONUM2:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM2,
/*N*/ 						0, SwNumRule::GetNumIndent( 1 ), 0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL3S:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
/*N*/ 						PT_12, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL3:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
/*N*/ 						0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL3E:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
/*N*/ 						0, PT_12 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_NONUM3:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM3,
/*N*/ 						0, SwNumRule::GetNumIndent( 2 ), 0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL4S:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
/*N*/ 						PT_12, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL4:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
/*N*/ 						0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL4E:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
/*N*/ 						0, PT_12 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_NONUM4:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM4,
/*N*/ 						0, SwNumRule::GetNumIndent( 3 ), 0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL5S:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
/*N*/ 						PT_12, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL5:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
/*N*/ 						0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_LEVEL5E:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
/*N*/ 						lNumFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
/*N*/ 						0, PT_12 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_NUM_NONUM5:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM5,
/*N*/ 						0, SwNumRule::GetNumIndent( 4 ), 0, PT_6 );
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL1S:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
/*N*/ 						PT_12, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL1:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
/*N*/ 						0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL1E:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL1,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
/*N*/ 						0, PT_12 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_NONUM1:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM1,
/*N*/ 						0, SwNumRule::GetBullIndent( 0 ), 0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL2S:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
/*N*/ 						PT_12, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL2:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
/*N*/ 						0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL2E:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL2,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
/*N*/ 						0, PT_12 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_NONUM2:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM2,
/*N*/ 						0, SwNumRule::GetBullIndent( 1 ), 0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL3S:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
/*N*/ 						PT_12, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL3:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
/*N*/ 						0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL3E:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL3,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
/*N*/ 						0, PT_12 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_NONUM3:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM3,
/*N*/ 						0, SwNumRule::GetBullIndent( 2 ), 0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL4S:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
/*N*/ 						PT_12, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL4:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
/*N*/ 						0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL4E:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL4,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
/*N*/ 						0, PT_12 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_NONUM4:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM4,
/*N*/ 						0, SwNumRule::GetBullIndent( 3 ), 0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL5S:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
/*N*/ 						PT_12, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL5:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
/*N*/ 						0, PT_6 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_LEVEL5E:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_LEVEL5,
/*N*/ 						lBullFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
/*N*/ 						0, PT_12 );
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_BUL_NONUM5:
/*N*/ 		lcl_SetNumBul( this, pNewColl, aSet, RES_POOLCOLL_BUL_NONUM5,
/*N*/ 						0, SwNumRule::GetBullIndent( 4 ), 0, PT_6 );
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_DOC_TITEL:	   		// Doc. Titel
/*N*/ 		{
/*N*/ 			SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD ) );
/*N*/ 			SetAllScriptItem( aSet, SvxFontHeightItem( PT_18 ) );
/*N*/
/*N*/ 			aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER ) );
/*N*/
/*N*/ 			if( !pDesc )
/*N*/ 				pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
/*N*/ 												RES_POOLCOLL_DOC_SUBTITEL ));
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_DOC_SUBTITEL:			// Doc. UnterTitel
/*N*/ 		{
/*N*/ 			SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL ));
/*N*/ 			SetAllScriptItem( aSet, SvxFontHeightItem( PT_14 ));
/*N*/
/*N*/ 			aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER ));
/*N*/
/*N*/ 			if( !pDesc )
/*N*/ 				pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
/*N*/ 												RES_POOLCOLL_TEXT ));
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_HTML_BLOCKQUOTE:
/*N*/ 		{
/*N*/ 			SvxLRSpaceItem aLR;
/*N*/ 			aLR.SetLeft( GetMetricVal( CM_1 ));
/*N*/ 			aLR.SetRight( GetMetricVal( CM_1 ));
/*N*/ 			aSet.Put( aLR );
/*N*/ //			aSet.Put( SvxAdjustItem( SVX_ADJUST_BLOCK ) );
/*N*/ 			SvxULSpaceItem aUL;
/*N*/ 			if( !pDesc )
/*N*/ 				aUL = pNewColl->GetULSpace();
/*N*/ 			aUL.SetLower( HTML_PARSPACE );
/*N*/ 			aSet.Put( aUL);
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_HTML_PRE:
/*N*/ 		{
/*N*/ 			::binfilter::lcl_SetDfltFont( DEFAULTFONT_FIXED, aSet );
/*N*/
/*N*/ // WORKAROUND: PRE auf 10pt setzten
/*N*/ 			SetAllScriptItem( aSet, SvxFontHeightItem(PT_10) );
/*N*/ // WORKAROUND: PRE auf 10pt setzten
/*N*/
/*N*/ 			// der untere Absatz-Abstand wird explizit gesetzt (macht
/*N*/ 			// die harte Attributierung einfacher)
/*N*/ 			SvxULSpaceItem aULSpaceItem;
/*N*/ 			if( !pDesc )
/*N*/ 				aULSpaceItem = pNewColl->GetULSpace();
/*N*/ 			aULSpaceItem.SetLower( 0 );
/*N*/ 			aSet.Put( aULSpaceItem );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_HTML_HR:
/*N*/ 		{
/*N*/ 			SvxBoxItem aBox;
/*N*/ 			Color aColor( COL_GRAY );
/*N*/ 			SvxBorderLine aNew( &aColor, DEF_DOUBLE_LINE0_OUT,
/*N*/ 										 DEF_DOUBLE_LINE0_IN,
/*N*/ 										 DEF_DOUBLE_LINE0_DIST );
/*N*/ 			aBox.SetLine( &aNew, BOX_LINE_BOTTOM );
/*N*/
/*N*/ 			aSet.Put( aBox );
/*N*/           aSet.Put( SwParaConnectBorderItem( FALSE ) );
/*N*/ 			SetAllScriptItem( aSet, SvxFontHeightItem(120) );
/*N*/
/*N*/ 			SvxULSpaceItem aUL;
/*N*/ 			if( !pDesc )
/*N*/ 			{
/*N*/ 				pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
/*N*/ 												RES_POOLCOLL_TEXT ));
/*N*/ 				aUL = pNewColl->GetULSpace();
/*N*/ 			}
/*N*/ 			aUL.SetLower( HTML_PARSPACE );
/*N*/ 			aSet.Put( aUL);
/*N*/ 			SwFmtLineNumber aLN; aLN.SetCountLines( FALSE );
/*N*/ 			aSet.Put( aLN );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLCOLL_HTML_DD:
/*N*/ 		{
/*N*/ 			SvxLRSpaceItem aLR;
/*N*/ 			if( !pDesc )
/*N*/ 				aLR = pNewColl->GetLRSpace();
/*N*/ 			// es wird um 1cm eingerueckt. Die IDs liegen immer 2 auseinander!
/*N*/ 			aLR.SetLeft( GetMetricVal( CM_1 ));
/*N*/ 			aSet.Put( aLR );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_HTML_DT:
/*N*/ 		{
/*N*/ 			SvxLRSpaceItem aLR;
/*N*/ 			if( !pDesc )
/*N*/ 			{
/*N*/ 				pNewColl->SetNextTxtFmtColl( *GetTxtCollFromPool(
/*N*/ 													RES_POOLCOLL_HTML_DD ));
/*N*/ 				aLR = pNewColl->GetLRSpace();
/*N*/ 			}
/*N*/ 			// es wird um 0cm eingerueckt. Die IDs liegen immer 2 auseinander!
/*N*/ 			aLR.SetLeft( 0 );
/*N*/ 			aSet.Put( aLR );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/
/*N*/ 	if( aSet.Count() )
/*N*/ 	{
/*N*/ 		if( pDesc )
/*N*/ 		{
/*?*/ 			String aStr;
/*?*/ 			aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, aStr );
/*?*/ 			pDesc->AppendAscii( sKomma );
/*?*/ 			*pDesc += aStr;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pNewColl->SetAttr( aSet );
/*N*/ 			// JP 31.08.95: erzeugen einer PoolVorlage ist keine Modifikation
/*N*/ 			//				(Bug: 18545)
/*N*/ 			// SetModified();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pNewColl;
/*N*/ }



    // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
    // benutzt wird





    // Gebe das "Auto[matische]-Format" mit der Id zurueck. Existiert
    // es noch nicht, dann erzeuge es

/*N*/ typedef SwFmt* (SwDoc::*FnMakeFmt)( const String &, SwFmt * );
/*M*/ SwFmt* SwDoc::GetFmtFromPool( USHORT nId, String* pDesc,
/*M*/ 	SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric )
/*M*/ {
/*M*/ 	SwFmt *pNewFmt, *pDeriveFmt;
/*M*/
/*M*/ 	SvPtrarr* pArray[ 2 ];
/*M*/ 	USHORT nArrCnt = 1, nRCId = 0;
/*M*/ 	FnMakeFmt fnMkFmt;
/*M*/ 	USHORT* pWhichRange = 0;
/*M*/
/*M*/ 	switch( nId & (COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID) )
/*M*/ 	{
/*M*/ 	case POOLGRP_CHARFMT:
/*M*/ 		{
/*M*/ 			pArray[0] = pCharFmtTbl;
/*M*/ 			pDeriveFmt = pDfltCharFmt;
/*M*/ 			fnMkFmt= (FnMakeFmt)&SwDoc::MakeCharFmt;
/*M*/
/*M*/ 			if( nId > RES_POOLCHR_NORMAL_END )
/*M*/ 				nRCId = RC_POOLCHRFMT_HTML_BEGIN - RES_POOLCHR_HTML_BEGIN;
/*M*/ 			else
/*M*/ 				nRCId = RC_POOLCHRFMT_BEGIN - RES_POOLCHR_BEGIN;
/*M*/ 			pWhichRange =  aCharFmtSetRange;
/*M*/
/*M*/ 			// Fehlerfall: unbekanntes Format, aber CharFormat
/*M*/ 			//			-> returne das erste
/*M*/ 			if( RES_POOLCHR_BEGIN > nId || nId >= RES_POOLCHR_END )
/*M*/ 			{
/*M*/ 				ASSERT( !this, "ungueltige Id" );
/*M*/ 				nId = RES_POOLCHR_BEGIN;
/*M*/ 			}
/*M*/ 		}
/*M*/ 		break;
/*M*/ 	case POOLGRP_FRAMEFMT:
/*M*/ 		{
/*M*/ 			pArray[0] = pFrmFmtTbl;
/*M*/ 			pArray[1] = pSpzFrmFmtTbl;
/*M*/ 			pDeriveFmt = pDfltFrmFmt;
/*M*/ 			fnMkFmt= (FnMakeFmt)&SwDoc::MakeFrmFmt;
/*M*/ 			nArrCnt = 2;
/*M*/ 			nRCId = RC_POOLFRMFMT_BEGIN - RES_POOLFRM_BEGIN;
/*M*/ 			pWhichRange = aFrmFmtSetRange;
/*M*/
/*M*/ 			// Fehlerfall: unbekanntes Format, aber FrameFormat
/*M*/ 			//			-> returne das erste
/*M*/ 			if( RES_POOLFRM_BEGIN > nId || nId >= RES_POOLFRM_END )
/*M*/ 			{
/*M*/ 				ASSERT( !this, "ungueltige Id" );
/*M*/ 				nId = RES_POOLFRM_BEGIN;
/*M*/ 			}
/*M*/ 		}
/*M*/ 		break;
/*M*/
/*M*/ 	default:
/*M*/ 		// Fehlerfall, unbekanntes Format
/*M*/ 		ASSERT( nId, "ungueltige Id" );
/*M*/ 		return 0;
/*M*/ 	}
/*M*/ 	ASSERT( nRCId, "ungueltige Id" );
/*M*/
/*M*/ 	while( nArrCnt-- )
/*M*/ 		for( USHORT n = 0; n < (*pArray[nArrCnt]).Count(); ++n )
/*M*/ 			if( nId == ( pNewFmt = (SwFmt*)(*pArray[ nArrCnt ] )[ n ] )->
/*M*/ 					GetPoolFmtId() )
/*M*/ 			{
/*M*/ 				if( pDesc )
/*M*/ 					pNewFmt->GetPresentation( ePres, eCoreMetric,
/*M*/ 											  ePresMetric, *pDesc );
/*M*/ 				return pNewFmt;
/*M*/ 			}
/*M*/
/*M*/ 	ResId aResId( nRCId + nId, *pSwResMgr );
/*M*/ 	String aNm( aResId );
/*M*/ 	SwAttrSet aSet( GetAttrPool(), pWhichRange );
/*M*/
/*M*/ 	if( pDesc )
/*M*/ 	{
/*M*/ 		pNewFmt = 0;
/*M*/ //		*pDesc = aEmptyStr;	// oder den Namen ?? aNm;
/*M*/ 		*pDesc = aNm;
/*M*/ 	}
/*M*/ 	else
/*M*/ 	{
/*M*/ 		BOOL bIsModified = IsModified();
/*M*/ 		pNewFmt = (this->*fnMkFmt)( aNm, pDeriveFmt );
/*M*/ 		if( !bIsModified )
/*M*/ 			ResetModified();
/*M*/ 		pNewFmt->SetPoolFmtId( nId );
/*M*/ 		pNewFmt->SetAuto( FALSE );		// kein Auto-Format
/*M*/ 	}
/*M*/
/*M*/ 	switch( nId )
/*M*/ 	{
/*M*/ 	case RES_POOLCHR_FOOTNOTE:				// Fussnote
/*M*/ 	case RES_POOLCHR_PAGENO:				// Seiten/Feld
/*M*/ 	case RES_POOLCHR_LABEL:					// Beschriftung
/*M*/ 	case RES_POOLCHR_DROPCAPS: 				// Initialien
/*M*/ 	case RES_POOLCHR_NUM_LEVEL:				// Aufzaehlungszeichen
/*M*/ 	case RES_POOLCHR_TOXJUMP:				// Verzeichnissprung
/*M*/ 	case RES_POOLCHR_ENDNOTE:				// Endnote
/*M*/ 	case RES_POOLCHR_LINENUM:				// Zeilennummerierung
/*M*/ 		break;
/*M*/
/*M*/ 	case RES_POOLCHR_ENDNOTE_ANCHOR:		// Endnotenanker
/*M*/ 	case RES_POOLCHR_FOOTNOTE_ANCHOR:		// Fussnotenanker
/*M*/ 		{
/*M*/ 			aSet.Put( SvxEscapementItem( DFLT_ESC_AUTO_SUPER, 58 ) );
/*M*/ 		}
/*M*/ 		break;
/*M*/
/*M*/
/*M*/ 	case RES_POOLCHR_BUL_LEVEL:				// Aufzaehlungszeichen
/*M*/ 		{
/*M*/ 			const Font& rBulletFont = SwNumRule::GetDefBulletFont();
/*M*/ 			SetAllScriptItem( aSet, SvxFontItem( rBulletFont.GetFamily(),
/*M*/             			rBulletFont.GetName(), rBulletFont.GetStyleName(),
/*M*/             			rBulletFont.GetPitch(), rBulletFont.GetCharSet() ));
/*M*/ 			SetAllScriptItem( aSet, SvxFontHeightItem( PT_9 ));
/*M*/ 		}
/*M*/ 		break;
/*M*/
/*M*/ 	case RES_POOLCHR_INET_NORMAL:
/*M*/ 		{
/*M*/             Color aCol( COL_BLUE );
/*M*/ 			aSet.Put( SvxColorItem( aCol ) );
/*M*/ 			aSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE ) );
/*M*/         }
/*M*/ 		break;
/*M*/ 	case RES_POOLCHR_INET_VISIT:
/*M*/ 		{
/*M*/             Color aCol( COL_RED );
/*M*/ 			aSet.Put( SvxColorItem( aCol ) );
/*M*/ 			aSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE ) );
/*M*/         }
/*M*/ 		break;
/*M*/ 	case RES_POOLCHR_JUMPEDIT:
/*M*/ 		{
/*M*/             Color aCol( COL_CYAN );
/*M*/             aSet.Put( SvxColorItem( aCol ) );
/*M*/             aSet.Put( SvxUnderlineItem( UNDERLINE_DOTTED ) );
/*M*/             aSet.Put( SvxCaseMapItem( SVX_CASEMAP_KAPITAELCHEN ) );
/*M*/ 		}
/*M*/ 		break;
/*M*/
/*M*/ 	case RES_POOLCHR_RUBYTEXT:
/*M*/ 		{
/*M*/ 			long nH = ((SvxFontHeightItem*)GetDfltAttr(
/*M*/ 								RES_CHRATR_CJK_FONTSIZE ))->GetHeight() / 2;
/*M*/ 			SetAllScriptItem( aSet, SvxFontHeightItem( nH ));
/*N*/ 			aSet.Put(SvxUnderlineItem( UNDERLINE_NONE ));
/*N*/ 			aSet.Put(SvxEmphasisMarkItem( EMPHASISMARK_NONE) );
/*M*/ 		}
/*M*/ 		break;
/*M*/
/*M*/ 	case RES_POOLCHR_HTML_EMPHASIS:
/*M*/ 	case RES_POOLCHR_HTML_CITIATION:
/*M*/ 	case RES_POOLCHR_HTML_VARIABLE:
/*M*/ 		{
/*M*/ 			SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL ) );
/*M*/ 		}
/*M*/ 		break;
/*M*/
/*M*/ 	case RES_POOLCHR_IDX_MAIN_ENTRY:
/*M*/ 	case RES_POOLCHR_HTML_STRONG:
/*M*/ 		{
/*M*/ 			SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD ));
/*M*/ 		}
/*M*/ 		break;
/*M*/
/*M*/ 	case RES_POOLCHR_HTML_CODE:
/*M*/ 	case RES_POOLCHR_HTML_SAMPLE:
/*M*/ 	case RES_POOLCHR_HTML_KEYBOARD:
/*M*/ 	case RES_POOLCHR_HTML_TELETYPE:
/*M*/ 		{
/*M*/ 			::binfilter::lcl_SetDfltFont( DEFAULTFONT_FIXED, aSet );
/*M*/ 		}
/*M*/ 		break;
/*M*/    case RES_POOLCHR_VERT_NUM:
/*M*/             aSet.Put( SvxCharRotateItem( 900 ) );
/*M*/     break;
/*M*/ //nichts besonderes
/*M*/ //	case RES_POOLCHR_HTML_DEFINSTANCE:
/*M*/ //			break;
/*M*/
/*M*/
/*M*/ 	case RES_POOLFRM_FRAME:
/*M*/ 		{
/*M*/ 			if ( IsBrowseMode() )
/*M*/ 			{
/*M*/ 				aSet.Put( SwFmtAnchor( FLY_IN_CNTNT ));
/*M*/ 				aSet.Put( SwFmtVertOrient( 0, VERT_LINE_CENTER, PRTAREA ) );
/*M*/ 				aSet.Put( SwFmtSurround( SURROUND_NONE ) );
/*M*/ 			}
/*M*/ 			else
/*M*/ 			{
/*M*/ 				aSet.Put( SwFmtAnchor( FLY_AT_CNTNT ));
/*M*/ 				aSet.Put( SwFmtSurround( SURROUND_PARALLEL ) );
/*M*/ 				aSet.Put( SwFmtHoriOrient( 0, HORI_CENTER, PRTAREA ) );
/*M*/ 				aSet.Put( SwFmtVertOrient( 0, VERT_TOP, PRTAREA ) );
/*M*/ 				Color aCol( COL_BLACK );
/*M*/ 				SvxBorderLine aLine( &aCol, DEF_LINE_WIDTH_0 );
/*M*/ 				SvxBoxItem aBox;
/*M*/ 				aBox.SetLine( &aLine, BOX_LINE_TOP );
/*M*/ 				aBox.SetLine( &aLine, BOX_LINE_BOTTOM );
/*M*/ 				aBox.SetLine( &aLine, BOX_LINE_LEFT );
/*M*/ 				aBox.SetLine( &aLine, BOX_LINE_RIGHT );
/*M*/ 				aBox.SetDistance( 85 );
/*M*/ 				aSet.Put( aBox );
/*M*/ 				aSet.Put( SvxLRSpaceItem( 114, 114 ) );
/*M*/ 				aSet.Put( SvxULSpaceItem( 114, 114 ) );
/*M*/ 			}
/*M*/ 		}
/*M*/ 		break;
/*M*/ 	case RES_POOLFRM_GRAPHIC:
/*M*/ 	case RES_POOLFRM_OLE:
/*M*/ 		{
/*M*/ 			aSet.Put( SwFmtAnchor( FLY_AT_CNTNT ));
/*M*/ 			aSet.Put( SwFmtHoriOrient( 0, HORI_CENTER, FRAME ));
/*M*/ 			aSet.Put( SwFmtVertOrient( 0, VERT_TOP, FRAME ));
/*M*/ 			aSet.Put( SwFmtSurround( SURROUND_NONE ));
/*M*/ 		}
/*M*/ 		break;
/*M*/ 	case RES_POOLFRM_FORMEL:
/*M*/ 		{
/*M*/ 			aSet.Put( SwFmtAnchor( FLY_IN_CNTNT ) );
/*M*/ 			aSet.Put( SwFmtVertOrient( 0, VERT_CHAR_CENTER, FRAME ) );
/*M*/ 			aSet.Put( SvxLRSpaceItem( 114, 114 ) );
/*M*/ 		}
/*M*/ 		break;
/*M*/ 	case RES_POOLFRM_MARGINAL:
/*M*/ 		{
/*M*/ 			aSet.Put( SwFmtAnchor( FLY_AT_CNTNT ));
/*M*/ 			aSet.Put( SwFmtHoriOrient( 0, HORI_LEFT, FRAME ));
/*M*/ 			aSet.Put( SwFmtVertOrient( 0, VERT_TOP, FRAME ));
/*M*/ 			aSet.Put( SwFmtSurround( SURROUND_PARALLEL ));
/*M*/ 			// Breite 3.5 centimeter vorgegeben, als Hoehe nur den
/*M*/ 			// min. Wert benutzen
/*M*/ 			aSet.Put( SwFmtFrmSize( ATT_MIN_SIZE,
/*M*/ 					GetMetricVal( CM_1 ) * 3 + GetMetricVal( CM_05 ),
/*M*/ 					MM50 ));
/*M*/ 		}
/*M*/ 		break;
/*M*/ 	case RES_POOLFRM_WATERSIGN:
/*M*/ 		{
/*M*/ 			aSet.Put( SwFmtAnchor( FLY_PAGE ));
/*M*/ 			aSet.Put( SwFmtHoriOrient( 0, HORI_CENTER, FRAME ));
/*M*/ 			aSet.Put( SwFmtVertOrient( 0, VERT_CENTER, FRAME ));
/*M*/ 			aSet.Put( SvxOpaqueItem( FALSE ));
/*M*/ 			aSet.Put( SwFmtSurround( SURROUND_THROUGHT ));
/*M*/ 		}
/*M*/ 		break;
/*M*/
/*M*/ 	case RES_POOLFRM_LABEL:
/*M*/ 		{
/*M*/ 			aSet.Put( SwFmtAnchor( FLY_IN_CNTNT ) );
/*M*/ 			aSet.Put( SwFmtVertOrient( 0, VERT_TOP, FRAME ) );
/*M*/ 			aSet.Put( SvxLRSpaceItem( 114, 114 ) );
/*M*/
/*M*/ 			SvxProtectItem aProtect;
/*M*/ 			aProtect.SetSizeProtect( TRUE );
/*M*/ 			aProtect.SetPosProtect( TRUE );
/*M*/ 			aSet.Put( aProtect );
/*M*/
/*M*/ 			if( !pDesc )
/*M*/ 				pNewFmt->SetAutoUpdateFmt( TRUE );
/*M*/ 		}
/*M*/ 		break;
/*M*/ 	}
/*M*/ 	if( aSet.Count() )
/*M*/ 	{
/*M*/ 		if( pDesc )
/*M*/ 		{
/*M*/ 			String aStr;
/*M*/ 			aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, aStr );
/*M*/ 			pDesc->AppendAscii( sKomma );
/*M*/ 			*pDesc += aStr;
/*M*/ 		}
/*M*/ 		else
/*M*/ 		{
/*M*/ 			pNewFmt->SetAttr( aSet );
/*M*/ 			// JP 31.08.95: erzeugen einer PoolVorlage ist keine Modifikation
/*M*/ 			//				(Bug: 18545)
/*M*/ 			// SetModified();
/*M*/ 		}
/*M*/ 	}
/*M*/ 	return pNewFmt;
/*M*/ }



    // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
    // benutzt wird



/*N*/ void lcl_GetStdPgSize( SwDoc* pDoc, SfxItemSet& rSet )
/*N*/ {
/*N*/ 	SwPageDesc* pStdPgDsc = pDoc->GetPageDescFromPool( RES_POOLPAGE_STANDARD );
/*N*/ 	SwFmtFrmSize aFrmSz( pStdPgDsc->GetMaster().GetFrmSize() );
/*N*/ 	if( pStdPgDsc->GetLandscape() )
/*N*/ 	{
/*N*/ 		SwTwips nTmp = aFrmSz.GetHeight();
/*N*/ 		aFrmSz.SetHeight( aFrmSz.GetWidth() );
/*N*/ 		aFrmSz.SetWidth( nTmp );
/*N*/ 	}
/*N*/ 	rSet.Put( aFrmSz );
/*N*/ }



/*N*/ SwPageDesc* SwDoc::GetPageDescFromPool( USHORT nId, String* pDesc,
/*N*/ 	SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
/*N*/     BOOL bRegardLanguage)
/*N*/ {
/*N*/ 	ASSERT( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END,
/*N*/ 			"Falsche AutoFormat-Id" );
/*N*/
/*N*/ 	SwPageDesc *pNewPgDsc;
        USHORT n=0;
/*N*/ 	for( n = 0; n < aPageDescs.Count(); ++n )
/*N*/ 		if( nId == ( pNewPgDsc = aPageDescs[ n ] )->GetPoolFmtId() )
/*N*/ 		{
/*N*/ 			if( pDesc )
/*N*/ 				pNewPgDsc->GetPresentation( ePres, eCoreMetric,
/*N*/ 											ePresMetric, *pDesc );
/*N*/ 			return pNewPgDsc;
/*N*/ 		}
/*N*/
/*N*/ 	// Fehlerfall: unbekannte Poolvorlage
/*N*/ 	if( RES_POOLPAGE_BEGIN > nId ||  nId >= RES_POOLPAGE_END )
/*N*/ 	{
/*?*/ 		ASSERT( !this, "ungueltige Id" );
/*?*/ 		nId = RES_POOLPAGE_BEGIN;
/*N*/ 	}
/*N*/
/*N*/ 	ResId aResId( sal_uInt32(RC_POOLPAGEDESC_BEGIN + nId - RES_POOLPAGE_BEGIN), *pSwResMgr );
/*N*/ 	String aNm( aResId );
/*N*/ 	if( pDesc )
/*N*/ 	{
/*?*/ 		pNewPgDsc = 0;
/*?*/ //		*pDesc = aEmptyStr;	// oder den Namen ?? aNm;
/*?*/ 		*pDesc = aNm;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		BOOL bIsModified = IsModified();
/*N*/ 		n = MakePageDesc( aNm, 0, bRegardLanguage );
/*N*/ 		pNewPgDsc = aPageDescs[ n ];
/*N*/ 		pNewPgDsc->SetPoolFmtId( nId );
/*N*/ 		if( !bIsModified )
/*N*/ 			ResetModified();
/*N*/ 	}
/*N*/
/*N*/
/*N*/ 	SvxLRSpaceItem aLR;
/*N*/ 	aLR.SetLeft( GetMetricVal( CM_1 ) * 2 );
/*N*/ 	aLR.SetRight( aLR.GetLeft() );
/*N*/ 	SvxULSpaceItem aUL;
/*N*/ 	aUL.SetUpper( (USHORT)aLR.GetLeft() );
/*N*/ 	aUL.SetLower( (USHORT)aLR.GetLeft() );
/*N*/
/*N*/ 	SwAttrSet aSet( GetAttrPool(), aPgFrmFmtSetRange );
/*N*/ 	BOOL bSetLeft = TRUE;
/*N*/
/*N*/ 	switch( nId )
/*N*/ 	{
/*N*/ 	case RES_POOLPAGE_STANDARD:				// Standard-Seite
/*N*/ 		{
/*N*/ 			aSet.Put( aLR );
/*N*/ 			aSet.Put( aUL );
/*N*/ 			if( pNewPgDsc )
/*N*/ 				pNewPgDsc->SetUseOn( PD_ALL );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLPAGE_FIRST:				// Erste Seite
/*N*/ 	case RES_POOLPAGE_REGISTER:				// Verzeichnis
/*N*/ 		{
/*N*/ 			lcl_GetStdPgSize( this, aSet );
/*N*/ 			aSet.Put( aLR );
/*N*/ 			aSet.Put( aUL );
/*N*/ 			if( pNewPgDsc )
/*N*/ 			{
/*N*/ 				pNewPgDsc->SetUseOn( PD_ALL );
/*N*/ 				if( RES_POOLPAGE_FIRST == nId )
/*N*/ 					pNewPgDsc->SetFollow( GetPageDescFromPool( RES_POOLPAGE_STANDARD ));
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLPAGE_LEFT:					// Linke Seite
/*N*/ 		{
/*N*/ 			lcl_GetStdPgSize( this, aSet );
/*N*/ 			aSet.Put( aLR );
/*N*/ 			aSet.Put( aUL );
/*N*/ 			bSetLeft = FALSE;
/*N*/ 			if( pNewPgDsc )
/*N*/ 				pNewPgDsc->SetUseOn( PD_LEFT );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLPAGE_RIGHT:				// Rechte Seite
/*N*/ 		{
/*N*/ 			lcl_GetStdPgSize( this, aSet );
/*N*/ 			aSet.Put( aLR );
/*N*/ 			aSet.Put( aUL );
/*N*/ 			bSetLeft = FALSE;
/*N*/ 			if( pNewPgDsc )
/*N*/ 				pNewPgDsc->SetUseOn( PD_RIGHT );
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLPAGE_JAKET:				// Umschlag
/*N*/ 		{
/*N*/ 			aLR.SetLeft( 0 ); aLR.SetRight( 0 );
/*N*/ 			aUL.SetUpper( 0 ); aUL.SetLower( 0 );
/*N*/ 			Size aPSize( SvxPaperInfo::GetPaperSize( SVX_PAPER_C65 ) );
/*N*/ 			LandscapeSwap( aPSize );
/*N*/ 			aSet.Put( SwFmtFrmSize( ATT_FIX_SIZE, aPSize.Width(), aPSize.Height() ));
/*N*/ 			aSet.Put( aLR );
/*N*/ 			aSet.Put( aUL );
/*N*/
/*N*/ 			if( pNewPgDsc )
/*N*/ 			{
/*N*/ 				pNewPgDsc->SetUseOn( PD_ALL );
/*N*/ 				pNewPgDsc->SetLandscape( TRUE );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLPAGE_HTML:				// HTML
/*N*/ 		{
/*N*/ 			lcl_GetStdPgSize( this, aSet );
/*N*/ 			aLR.SetRight( GetMetricVal( CM_1 ));
/*N*/ 			aUL.SetUpper( (USHORT)aLR.GetRight() );
/*N*/ 			aUL.SetLower( (USHORT)aLR.GetRight() );
/*N*/ 			aSet.Put( aLR );
/*N*/ 			aSet.Put( aUL );
/*N*/
/*N*/ 			if( pNewPgDsc )
/*N*/ 				pNewPgDsc->SetUseOn( PD_ALL );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLPAGE_FOOTNOTE:
/*N*/ 	case RES_POOLPAGE_ENDNOTE:
/*N*/ 		{
/*N*/ 			lcl_GetStdPgSize( this, aSet );
/*N*/ 			aSet.Put( aLR );
/*N*/ 			aSet.Put( aUL );
/*N*/ 			if( pNewPgDsc )
/*N*/ 				pNewPgDsc->SetUseOn( PD_ALL );
/*N*/ 			SwPageFtnInfo aInf( pNewPgDsc->GetFtnInfo() );
/*N*/ 			aInf.SetLineWidth( 0 );
/*N*/ 			aInf.SetTopDist( 0 );
/*N*/ 			aInf.SetBottomDist( 0 );
/*N*/ 			pNewPgDsc->SetFtnInfo( aInf );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/
/*N*/ 	if( aSet.Count() )
/*N*/ 	{
/*N*/ 		if( pDesc )
/*N*/ 		{
/*?*/ 			String aStr;
/*?*/ 			aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, aStr );
/*?*/ 			pDesc->AppendAscii( sKomma );
/*?*/ 			*pDesc += aStr;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( bSetLeft )
/*N*/ 				pNewPgDsc->GetLeft().SetAttr( aSet );
/*N*/ 			pNewPgDsc->GetMaster().SetAttr( aSet );
/*N*/ 			// JP 31.08.95: erzeugen einer PoolVorlage ist keine Modifikation
/*N*/ 			//				(Bug: 18545)
/*N*/ 			// SetModified();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pNewPgDsc;
/*N*/ }

/*N*/ SwNumRule* SwDoc::GetNumRuleFromPool( USHORT nId, String* pDesc,
/*N*/ 	SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric )
/*N*/ {
/*N*/ 	ASSERT( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END,
/*N*/ 			"Falsche AutoFormat-Id" );
/*N*/
/*N*/ 	SwNumRule* pNewRule;
        USHORT n=0;
/*N*/ 	for( n = 0; n < GetNumRuleTbl().Count(); ++n )
/*N*/ 		if( nId == ( pNewRule = GetNumRuleTbl()[ n ] )->GetPoolFmtId() )
/*N*/ 		{
/*?*/ 			if( pDesc )
/*?*/ 				*pDesc = pNewRule->GetName();
/*?*/ 			return pNewRule;
/*N*/ 		}

    // Fehlerfall: unbekannte Poolvorlage
/*N*/ 	if( RES_POOLNUMRULE_BEGIN > nId ||  nId >= RES_POOLNUMRULE_END )
/*N*/ 	{
/*?*/ 		ASSERT( !this, "ungueltige Id" );
/*?*/ 		nId = RES_POOLNUMRULE_BEGIN;
/*N*/ 	}

/*N*/ 	ResId aResId( sal_uInt32(RC_POOLNUMRULE_BEGIN + nId - RES_POOLNUMRULE_BEGIN), *pSwResMgr );
/*N*/ 	String aNm( aResId );
/*N*/
/*N*/ 	SwCharFmt *pNumCFmt = 0, *pBullCFmt = 0;
/*N*/
/*N*/ 	if( pDesc )
/*N*/ 	{
/*?*/ 		pNewRule = new SwNumRule( aNm );
/*?*/ 		*pDesc = aNm;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		BOOL bIsModified = IsModified();
/*N*/ 		n = MakeNumRule( aNm );
/*N*/ 		pNewRule = GetNumRuleTbl()[ n ];
/*N*/ 		pNewRule->SetPoolFmtId( nId );
/*N*/ 		pNewRule->SetAutoRule( FALSE );
/*N*/
/*N*/ 		if( RES_POOLNUMRULE_NUM1 <= nId && nId <= RES_POOLNUMRULE_NUM5 )
/*N*/ 			pNumCFmt = GetCharFmtFromPool( RES_POOLCHR_NUM_LEVEL );
/*N*/
/*N*/ 		if( ( RES_POOLNUMRULE_BUL1 <= nId && nId <= RES_POOLNUMRULE_BUL5 ) ||
/*N*/ 			RES_POOLNUMRULE_NUM5 == nId )
/*N*/ 			pBullCFmt = GetCharFmtFromPool( RES_POOLCHR_NUM_LEVEL );
/*N*/
/*N*/ 		if( !bIsModified )
/*?*/ 			ResetModified();
/*N*/ 	}
/*N*/
/*N*/ 	switch( nId )
/*N*/ 	{
/*N*/ 	case RES_POOLNUMRULE_NUM1:
/*N*/ 		{
/*N*/ 			SwNumFmt aFmt;
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_ARABIC);
/*N*/ 			aFmt.SetCharFmt( pNumCFmt );
/*N*/ 			aFmt.SetStart( 1 );
/*N*/ 			aFmt.SetIncludeUpperLevels( 1 );
/*N*/ 			aFmt.SetSuffix( aDotStr );
/*N*/
/*N*/ 			static const USHORT aAbsSpace[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ //				cm: 0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
/*N*/ 					283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
/*N*/ 				};
/*N*/ #ifdef USE_MEASUREMENT
/*N*/ 			static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ 					283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
/*N*/ 				};
/*N*/ 			const USHORT* pArr = MEASURE_METRIC ==
/*N*/ 								GetAppLocaleData().getMeasurementSystemEnum()
/*N*/ 									? aAbsSpace
/*N*/ 									: aAbsSpaceInch;
/*N*/ #else
/*N*/ 			const USHORT* pArr = aAbsSpace;
/*N*/ #endif
/*N*/
/*N*/ 			aFmt.SetFirstLineOffset( - (*pArr) );
/*N*/ 			for( n = 0; n < MAXLEVEL; ++n, ++pArr )
/*N*/ 			{
/*N*/ 				aFmt.SetAbsLSpace( *pArr );
/*N*/ 				pNewRule->Set( n, aFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLNUMRULE_NUM2:
/*N*/ 		{
/*N*/ 			static const USHORT aAbsSpace[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ 					283,  283,  567,  709,		// 0.50, 0.50, 1.00, 1.25
/*N*/ 					850, 1021, 1304, 1474,		// 1.50, 1.80, 2.30, 2.60
/*N*/ 				   1588, 1758					// 2.80, 3.10
/*N*/ 				};
/*N*/
/*N*/ #ifdef USE_MEASUREMENT
/*N*/ 			static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ 					385,  385,  770,  963,
/*N*/ 				   1155, 1386, 1771, 2002,
/*N*/ 				   2156, 2387
/*N*/ 				};
/*N*/
/*N*/ 			const USHORT* pArr = MEASURE_METRIC ==
/*N*/ 								GetAppLocaleData().getMeasurementSystemEnum()
/*N*/ 									? aAbsSpace
/*N*/ 									: aAbsSpaceInch;
/*N*/ #else
/*N*/ 			const USHORT* pArr = aAbsSpace;
/*N*/ #endif
/*N*/ 			SwNumFmt aFmt;
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_ARABIC);
/*N*/ 			aFmt.SetCharFmt( pNumCFmt );
/*N*/ 			aFmt.SetIncludeUpperLevels( 1 );
/*N*/ 			USHORT nSpace = 0;
/*N*/ 			for( n = 0; n < MAXLEVEL; ++n )
/*N*/ 			{
/*N*/ 				aFmt.SetAbsLSpace( nSpace += pArr[ n ] );
/*N*/ 				aFmt.SetFirstLineOffset( - pArr[ n ] );
/*N*/ 				aFmt.SetStart( n+1 );
/*N*/ 				pNewRule->Set( n, aFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLNUMRULE_NUM3:
/*N*/ 		{
/*N*/ 			SwNumFmt aFmt;
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_ARABIC);
/*N*/ 			aFmt.SetCharFmt( pNumCFmt );
/*N*/ 			aFmt.SetIncludeUpperLevels( 1 );
/*N*/ 			USHORT nOffs = GetMetricVal( CM_1 ) * 3;
/*N*/ 			aFmt.SetFirstLineOffset( - nOffs );
/*N*/
/*N*/ 			for( n = 0; n < MAXLEVEL; ++n )
/*N*/ 			{
/*N*/ 				aFmt.SetAbsLSpace( (n+1) * nOffs );
/*N*/ 				aFmt.SetStart( n+1 );
/*N*/ 				pNewRule->Set( n, aFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLNUMRULE_NUM4:
/*N*/ 		{
/*N*/ 			SwNumFmt aFmt;
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_ROMAN_UPPER);
/*N*/ 			aFmt.SetCharFmt( pNumCFmt );
/*N*/ 			aFmt.SetIncludeUpperLevels( 1 );
/*N*/ 			aFmt.SetSuffix( aDotStr );
/*N*/
/*N*/ 			static const USHORT aAbsSpace[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ //				cm: 0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
/*N*/ 					283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
/*N*/ 				};
/*N*/ #ifdef USE_MEASUREMENT
/*N*/ 			static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ 					283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
/*N*/ 				};
/*N*/ 			const USHORT* pArr = MEASURE_METRIC ==
/*N*/ 								GetAppLocaleData().getMeasurementSystemEnum()
/*N*/ 									? aAbsSpace
/*N*/ 									: aAbsSpaceInch;
/*N*/ #else
/*N*/ 			const USHORT* pArr = aAbsSpace;
/*N*/ #endif
/*N*/
/*N*/ 			aFmt.SetFirstLineOffset( - (*pArr) );
/*N*/ 			for( n = 0; n < MAXLEVEL; ++n, ++pArr )
/*N*/ 			{
/*N*/ 				aFmt.SetStart( n + 1 );
/*N*/ 				aFmt.SetAbsLSpace( *pArr );
/*N*/ 				pNewRule->Set( n, aFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLNUMRULE_NUM5:
/*N*/ 		{
/*N*/ 			// [ First, LSpace ]
/*N*/ 			static const USHORT aAbsSpace0to2[] =
/*N*/ 				{
/*N*/ 					227,  227,  	// 0.40, 0.40,
/*N*/ 					369,  624,		// 0.65, 1.10,
/*N*/ 					255,  879 		// 0.45, 1.55
/*N*/ 				};
/*N*/ 			static const USHORT aAbsSpaceInch0to2[] =
/*N*/ 				{
/*N*/ 					308,  308,
/*N*/ 					501,  847,
/*N*/ 					347, 1194
/*N*/ 				};
/*N*/
/*N*/ #ifdef USE_MEASUREMENT
/*N*/ 			const USHORT* pArr0to2 = MEASURE_METRIC ==
/*N*/ 							GetAppLocaleData().getMeasurementSystemEnum()
/*N*/ 								? aAbsSpace0to2
/*N*/ 								: aAbsSpaceInch0to2;
/*N*/ #else
/*N*/ 			const USHORT* pArr0to2 = aAbsSpace0to2;
/*N*/ #endif
/*N*/ 			SwNumFmt aFmt;
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_ARABIC);
/*N*/ 			aFmt.SetStart( 1 );
/*N*/ 			aFmt.SetIncludeUpperLevels( 1 );
/*N*/ 			aFmt.SetSuffix( aDotStr );
/*N*/ 			aFmt.SetFirstLineOffset( -pArr0to2[0] );	// == 0.40 cm
/*N*/ 			aFmt.SetAbsLSpace( pArr0to2[1] );			// == 0.40 cm
/*N*/
/*N*/ 			aFmt.SetCharFmt( pNumCFmt );
/*N*/ 			pNewRule->Set( 0, aFmt );
/*N*/
/*N*/ 			aFmt.SetIncludeUpperLevels( 2 );
/*N*/ 			aFmt.SetStart( 2 );
/*N*/ 			aFmt.SetFirstLineOffset( -pArr0to2[2] );	// == 0.65 cm
/*N*/ 			aFmt.SetAbsLSpace( pArr0to2[3] );			// == 1.10 cm
/*N*/ 			pNewRule->Set( 1, aFmt );
/*N*/
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER);
/*N*/ 			aFmt.SetSuffix( ')');
/*N*/ 			aFmt.SetIncludeUpperLevels( 1 );
/*N*/ 			aFmt.SetStart( 3 );
/*N*/ 			aFmt.SetFirstLineOffset( - pArr0to2[4] );	// == 0.45cm
/*N*/ 			aFmt.SetAbsLSpace( pArr0to2[5] );			// == 1.55 cm
/*N*/ 			pNewRule->Set( 2, aFmt );
/*N*/
/*N*/
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
/*N*/ 			aFmt.SetCharFmt( pBullCFmt );
/*N*/ 			aFmt.SetBulletFont(  &SwNumRule::GetDefBulletFont() );
/*N*/ 			aFmt.SetBulletChar( cBulletChar );
/*N*/ 			USHORT nOffs = GetMetricVal( CM_01 ) * 4,
/*N*/ 				   nOffs2 = GetMetricVal( CM_1 ) * 2;
/*N*/
/*N*/ 			aFmt.SetFirstLineOffset( - nOffs );
/*N*/ 			aFmt.SetSuffix( aEmptyStr );
/*N*/ 			for( n = 3; n < MAXLEVEL; ++n )
/*N*/ 			{
/*N*/ 				aFmt.SetStart( n+1 );
/*N*/ 				aFmt.SetAbsLSpace( nOffs2 + ((n-3) * nOffs) );
/*N*/ 				pNewRule->Set( n, aFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_POOLNUMRULE_BUL1:
/*N*/ 		{
/*N*/ 			SwNumFmt aFmt;
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
/*N*/ 			aFmt.SetCharFmt( pBullCFmt );
/*N*/ 			aFmt.SetStart( 1 );
/*N*/ 			aFmt.SetIncludeUpperLevels( 1 );
/*N*/ 			aFmt.SetBulletFont( &SwNumRule::GetDefBulletFont() );
/*N*/ 			aFmt.SetBulletChar( cBulletChar );
/*N*/
/*N*/ 			static const USHORT aAbsSpace[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ //				cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
/*N*/ 					227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
/*N*/ 				};
/*N*/ #ifdef USE_MEASUREMENT
/*N*/ 			static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ 					227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
/*N*/ 				};
/*N*/ 			const USHORT* pArr = MEASURE_METRIC ==
/*N*/ 								GetAppLocaleData().getMeasurementSystemEnum()
/*N*/ 									? aAbsSpace
/*N*/ 									: aAbsSpaceInch;
/*N*/ #else
/*N*/ 			const USHORT* pArr = aAbsSpace;
/*N*/ #endif
/*N*/
/*N*/ 			aFmt.SetFirstLineOffset( - (*pArr) );
/*N*/ 			for( n = 0; n < MAXLEVEL; ++n, ++pArr )
/*N*/ 			{
/*N*/ 				aFmt.SetAbsLSpace( *pArr );
/*N*/ 				pNewRule->Set( n, aFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLNUMRULE_BUL2:
/*N*/ 		{
/*N*/ 			SwNumFmt aFmt;
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
/*N*/ 			aFmt.SetCharFmt( pBullCFmt );
/*N*/ 			aFmt.SetStart( 1 );
/*N*/ 			aFmt.SetIncludeUpperLevels( 1 );
/*N*/ 			aFmt.SetBulletFont(  &SwNumRule::GetDefBulletFont() );
/*N*/ 			aFmt.SetBulletChar( 0x2013 );
/*N*/
/*N*/ 			static const USHORT aAbsSpace[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ //				cm: 0,3  0,6  0,9  1,2  1,5  1,8   2,1   2,4   2,7   3,0
/*N*/ 					170, 340, 510, 680, 850, 1020, 1191, 1361, 1531, 1701
/*N*/ 				};
/*N*/ #ifdef USE_MEASUREMENT
/*N*/ 			static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ 					170, 340, 510, 680, 850, 1020, 1191, 1361, 1531, 1701
/*N*/ 				};
/*N*/ 			const USHORT* pArr = MEASURE_METRIC ==
/*N*/ 								GetAppLocaleData().getMeasurementSystemEnum()
/*N*/ 									? aAbsSpace
/*N*/ 									: aAbsSpaceInch;
/*N*/ #else
/*N*/ 			const USHORT* pArr = aAbsSpace;
/*N*/ #endif
/*N*/
/*N*/ 			aFmt.SetFirstLineOffset( - (*pArr) );
/*N*/ 			for( n = 0; n < MAXLEVEL; ++n, ++pArr )
/*N*/ 			{
/*N*/ 				aFmt.SetAbsLSpace( *pArr );
/*N*/ 				pNewRule->Set( n, aFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLNUMRULE_BUL3:
/*N*/ 		{
/*N*/ 			SwNumFmt aFmt;
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
/*N*/ 			aFmt.SetCharFmt( pBullCFmt );
/*N*/ 			aFmt.SetStart( 1 );
/*N*/ 			aFmt.SetIncludeUpperLevels( 1 );
/*N*/ 			aFmt.SetBulletFont(  &SwNumRule::GetDefBulletFont() );
/*N*/ 			USHORT nOffs = GetMetricVal( CM_01 ) * 4;
/*N*/ 			aFmt.SetFirstLineOffset( - nOffs );
/*N*/
/*N*/ 			for( n = 0; n < MAXLEVEL; ++n )
/*N*/ 			{
/*N*/ 				aFmt.SetBulletChar( ( n & 1 ? 0x25a1 : 0x2611 ) );
/*N*/ 				aFmt.SetAbsLSpace( ((n & 1) +1) * nOffs );
/*N*/ 				pNewRule->Set( n, aFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLNUMRULE_BUL4:
/*N*/ 		{
/*N*/ 			SwNumFmt aFmt;
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
/*N*/ 			aFmt.SetCharFmt( pBullCFmt );
/*N*/ 			aFmt.SetStart( 1 );
/*N*/ 			aFmt.SetIncludeUpperLevels( 1 );
/*N*/ 			aFmt.SetBulletFont(  &SwNumRule::GetDefBulletFont() );
/*N*/
/*N*/ 			static const USHORT aAbsSpace[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ //				cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
/*N*/ 					227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
/*N*/ 				};
/*N*/ #ifdef USE_MEASUREMENT
/*N*/ 			static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ 					227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
/*N*/ 				};
/*N*/ 			const USHORT* pArr = MEASURE_METRIC ==
/*N*/ 								GetAppLocaleData().getMeasurementSystemEnum()
/*N*/ 									? aAbsSpace
/*N*/ 									: aAbsSpaceInch;
/*N*/ #else
/*N*/ 			const USHORT* pArr = aAbsSpace;
/*N*/ #endif
/*N*/
/*N*/ 			aFmt.SetFirstLineOffset( - (*pArr) );
/*N*/ 			for( n = 0; n < MAXLEVEL; ++n, ++pArr )
/*N*/ 			{
/*N*/ 				switch( n )
/*N*/ 				{
/*N*/ 				case 0: 	aFmt.SetBulletChar( 0x27a2 );	break;
/*N*/ 				case 1:		aFmt.SetBulletChar( 0xE006 );	break;
/*N*/ 				default:	aFmt.SetBulletChar( 0xE004 );	break;
/*N*/ 				}
/*N*/ 				aFmt.SetAbsLSpace( *pArr );
/*N*/ 				pNewRule->Set( n, aFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_POOLNUMRULE_BUL5:
/*N*/ 		{
/*N*/ 			SwNumFmt aFmt;
/*N*/ 			aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
/*N*/ 			aFmt.SetCharFmt( pBullCFmt );
/*N*/ 			aFmt.SetStart( 1 );
/*N*/ 			aFmt.SetIncludeUpperLevels( 1 );
/*N*/ 			aFmt.SetBulletChar( 0x2717 );
/*N*/ 			aFmt.SetBulletFont(  &SwNumRule::GetDefBulletFont() );
/*N*/
/*N*/ 			static const USHORT aAbsSpace[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ //				cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
/*N*/ 					227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
/*N*/ 				};
/*N*/ #ifdef USE_MEASUREMENT
/*N*/ 			static const USHORT aAbsSpaceInch[ MAXLEVEL ] =
/*N*/ 				{
/*N*/ 					227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
/*N*/ 				};
/*N*/ 			const USHORT* pArr = MEASURE_METRIC ==
/*N*/ 								GetAppLocaleData().getMeasurementSystemEnum()
/*N*/ 									? aAbsSpace
/*N*/ 									: aAbsSpaceInch;
/*N*/ #else
/*N*/ 			const USHORT* pArr = aAbsSpace;
/*N*/ #endif
/*N*/
/*N*/ 			aFmt.SetFirstLineOffset( - (*pArr) );
/*N*/ 			for( n = 0; n < MAXLEVEL; ++n, ++pArr )
/*N*/ 			{
/*N*/ 				aFmt.SetAbsLSpace( *pArr );
/*N*/ 				pNewRule->Set( n, aFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/
/*N*/ 	if( pDesc )
/*N*/ 	{
/*N*/ //JP 25.02.98: wie soll die Beschreibung sein??
/*N*/ //		String aStr;
/*N*/ //		aSet.GetPresentation( ePres, eCoreMetric, ePresMetric, aStr );
/*N*/ //		*pDesc += sKomma;
/*N*/ //		*pDesc += aStr;
/*N*/
/*?*/ 		delete pNewRule, pNewRule = 0;
/*N*/ 	}
/*N*/
/*N*/ 	return pNewRule;
/*N*/ }



    // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
    // benutzt wird

    // pruefe, ob diese "Auto-Collection" in Dokument schon/noch
    // benutzt wird



// erfrage ob die Absatz-/Zeichen-/Rahmen-/Seiten - Vorlage benutzt wird
/*N*/ BOOL SwDoc::IsUsed( const SwModify& rModify ) const
/*N*/ {
/*N*/ 	// dann teste mal, ob es abhaengige ContentNodes im Nodes Array gibt
/*N*/ 	// (auch indirekte fuer Format-Ableitung! )
/*N*/ 	SwAutoFmtGetDocNode aGetHt( &aNodes );
/*N*/ 	return !rModify.GetInfo( aGetHt );
/*N*/ }

// erfrage ob die NumRule benutzt wird
/*N*/ BOOL SwDoc::IsUsed( const SwNumRule& rRule ) const
/*N*/ {
/*N*/ 	// dann teste mal, ob es abhaengige ContentNodes im Nodes Array gibt
/*N*/ 	// (auch indirekte fuer Format-Ableitung! )
/*N*/ 	BOOL bUsed = FALSE;
/*N*/ 	SwAutoFmtGetDocNode aGetHt( &aNodes );
/*N*/ 	SwModify* pMod;
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	USHORT i, nMaxItems = GetAttrPool().GetItemCount( RES_PARATR_NUMRULE);
/*N*/ 	for( i = 0; i < nMaxItems; ++i )
/*N*/ 		if( 0 != (pItem = GetAttrPool().GetItem( RES_PARATR_NUMRULE, i ) ) &&
/*N*/ 			0 != ( pMod = (SwModify*)((SwNumRuleItem*)pItem)->GetDefinedIn()) &&
/*N*/ 			((SwNumRuleItem*)pItem)->GetValue().Len() &&
/*N*/ 			((SwNumRuleItem*)pItem)->GetValue() == rRule.GetName() )
/*N*/ 		{
/*N*/ 			if( pMod->IsA( TYPE( SwFmt )) )
/*N*/ 			{
/*?*/ 				bUsed = !pMod->GetInfo( aGetHt );
/*?*/ 				if( bUsed )
/*?*/ 					break;
/*N*/ 			}
/*N*/ 			else if( ((SwTxtNode*)pMod)->GetNodes().IsDocNodes() )
/*N*/ 			{
/*N*/ 				bUsed = TRUE;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 	return bUsed;
/*N*/ }

    // loeche die nicht mehr benutzten Pattern-Namen aus dem Array.
    // alle nicht mehr referenzierten Namen werden durch 0-Pointer
    // ersetzt. Diese Positionen koennen wieder vergeben werden.

    // Suche die Position vom Vorlagen-Namen. Ist nicht vorhanden
    // dann fuege neu ein
USHORT SwDoc::SetDocPattern( const String& rPatternName )
{
    ASSERT( rPatternName.Len(), "kein Dokument-Vorlagenname" );

    USHORT nNewPos = aPatternNms.Count();
    for( USHORT n = 0; n < aPatternNms.Count(); ++n )
        if( !aPatternNms[n] )
        {
            if( nNewPos == aPatternNms.Count() )
                nNewPos = n;
        }
        else if( rPatternName == *aPatternNms[n] )
            return n;

    if( nNewPos < aPatternNms.Count() )
        aPatternNms.Remove( nNewPos );      // Platz wieder frei machen

    String* pNewNm = new String( rPatternName );
    aPatternNms.Insert( pNewNm, nNewPos );
    SetModified();
    return nNewPos;
}

/*N*/ USHORT GetPoolParent( USHORT nId )
/*N*/ {
/*N*/ 	USHORT nRet = USHRT_MAX;
/*N*/ 	if( POOLGRP_NOCOLLID & nId )		// 1 == Formate / 0 == Collections
/*N*/ 	{
/*N*/ 		switch( ( COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID ) & nId )
/*?*/ 		{
/*?*/ 		case POOLGRP_CHARFMT:
/*?*/ 		case POOLGRP_FRAMEFMT:
/*?*/ 			nRet = 0; 			// vom default abgeleitet
/*?*/ 			break;
/*?*/ 		case POOLGRP_PAGEDESC:
/*?*/ 		case POOLGRP_NUMRULE:
/*?*/ 			break;				// es gibt keine Ableitung
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		switch( COLL_GET_RANGE_BITS & nId )
/*N*/ 		{
/*N*/ 		case COLL_TEXT_BITS:
/*N*/ 			switch( nId )
/*N*/ 			{
/*N*/ 			case RES_POOLCOLL_STANDARD:
/*N*/ 					nRet = 0;									break;
/*N*/ 			case RES_POOLCOLL_TEXT_IDENT:
/*N*/ 			case RES_POOLCOLL_TEXT_NEGIDENT:
/*N*/ 			case RES_POOLCOLL_TEXT_MOVE:
/*N*/ 			case RES_POOLCOLL_CONFRONTATION:
/*N*/ 			case RES_POOLCOLL_MARGINAL:
/*N*/ 					nRet = RES_POOLCOLL_TEXT;					break;
/*N*/
/*N*/ 			case RES_POOLCOLL_TEXT:
/*N*/ 			case RES_POOLCOLL_GREETING:
/*N*/ 			case RES_POOLCOLL_SIGNATURE:
/*N*/ 			case RES_POOLCOLL_HEADLINE_BASE:
/*N*/ 					nRet = RES_POOLCOLL_STANDARD;				break;
/*N*/
/*N*/ 			case RES_POOLCOLL_HEADLINE1:
/*N*/ 			case RES_POOLCOLL_HEADLINE2:
/*N*/ 			case RES_POOLCOLL_HEADLINE3:
/*N*/ 			case RES_POOLCOLL_HEADLINE4:
/*N*/ 			case RES_POOLCOLL_HEADLINE5:
/*N*/ 			case RES_POOLCOLL_HEADLINE6:
/*N*/ 			case RES_POOLCOLL_HEADLINE7:
/*N*/ 			case RES_POOLCOLL_HEADLINE8:
/*N*/ 			case RES_POOLCOLL_HEADLINE9:
/*N*/ 			case RES_POOLCOLL_HEADLINE10:
/*N*/ 					nRet = RES_POOLCOLL_HEADLINE_BASE; 			break;
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 		case COLL_LISTS_BITS:
/*N*/ 			switch( nId )
/*N*/ 			{
/*N*/ 			case RES_POOLCOLL_NUMBUL_BASE:
/*N*/ 					nRet = RES_POOLCOLL_TEXT;					break;
/*N*/
/*N*/ 			default:
/*N*/ 				nRet = RES_POOLCOLL_NUMBUL_BASE;				break;
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 		case COLL_EXTRA_BITS:
/*N*/ 			switch( nId )
/*N*/ 			{
/*N*/ 			case RES_POOLCOLL_FRAME:
/*N*/ 			case RES_POOLCOLL_TABLE:
/*N*/ 					nRet = RES_POOLCOLL_TEXT;					break;
/*N*/
/*N*/ 			case RES_POOLCOLL_TABLE_HDLN:
/*N*/ 					nRet = RES_POOLCOLL_TABLE;					break;
/*N*/
/*N*/ 			case RES_POOLCOLL_FOOTNOTE:
/*N*/ 			case RES_POOLCOLL_ENDNOTE:
/*N*/ 			case RES_POOLCOLL_JAKETADRESS:
/*N*/ 			case RES_POOLCOLL_SENDADRESS:
/*N*/ 			case RES_POOLCOLL_HEADER:
/*N*/ 			case RES_POOLCOLL_HEADERL:
/*N*/ 			case RES_POOLCOLL_HEADERR:
/*N*/ 			case RES_POOLCOLL_FOOTER:
/*N*/ 			case RES_POOLCOLL_FOOTERL:
/*N*/ 			case RES_POOLCOLL_FOOTERR:
/*N*/ 			case RES_POOLCOLL_LABEL:
/*N*/ 					nRet = RES_POOLCOLL_STANDARD;				break;
/*N*/
/*N*/ 			case RES_POOLCOLL_LABEL_ABB:
/*N*/ 			case RES_POOLCOLL_LABEL_TABLE:
/*N*/ 			case RES_POOLCOLL_LABEL_FRAME:
/*N*/ 			case RES_POOLCOLL_LABEL_DRAWING:
/*N*/ 					nRet = RES_POOLCOLL_LABEL;					break;
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 		case COLL_REGISTER_BITS:
/*N*/ 			switch( nId )
/*N*/ 			{
/*N*/ 			case RES_POOLCOLL_REGISTER_BASE:
/*N*/ 					nRet = RES_POOLCOLL_STANDARD;				break;
/*N*/
/*N*/ 			case RES_POOLCOLL_TOX_USERH:
/*N*/ 			case RES_POOLCOLL_TOX_CNTNTH:
/*N*/ 			case RES_POOLCOLL_TOX_IDXH:
/*N*/ 			case RES_POOLCOLL_TOX_ILLUSH:
/*N*/ 			case RES_POOLCOLL_TOX_OBJECTH:
/*N*/ 			case RES_POOLCOLL_TOX_TABLESH:
/*N*/ 			case RES_POOLCOLL_TOX_AUTHORITIESH:
/*N*/ 					nRet = RES_POOLCOLL_HEADLINE_BASE;  		break;
/*N*/
/*N*/ 			default:
/*N*/ 					nRet = RES_POOLCOLL_REGISTER_BASE;  		break;
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 		case COLL_DOC_BITS:
/*N*/ 			nRet = RES_POOLCOLL_HEADLINE_BASE;
/*N*/ 			break;
/*N*/
/*N*/ 		case COLL_HTML_BITS:
/*N*/ 			nRet = RES_POOLCOLL_STANDARD;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return nRet;
/*N*/ }

void SwDoc::RemoveAllFmtLanguageDependencies()
{
    /* #106748# Restore the language independ pool defaults and styles. */
    GetAttrPool().ResetPoolDefaultItem( RES_PARATR_ADJUST );
    GetTxtCollFromPool( RES_POOLCOLL_STANDARD )->ResetAttr( RES_PARATR_ADJUST );

    SvxFrameDirectionItem aFrameDir( FRMDIR_HORI_LEFT_TOP );

    sal_uInt16 nCount = GetPageDescCnt();
    for( sal_uInt16 i=0; i<nCount; ++i )
    {
        SwPageDesc& rDesc = _GetPageDesc( i );
        rDesc.GetMaster().SetAttr( aFrameDir );
        rDesc.GetLeft().SetAttr( aFrameDir );
    }
}

}
