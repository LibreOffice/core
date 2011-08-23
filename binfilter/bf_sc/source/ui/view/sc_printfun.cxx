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
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <bf_svx/svxids.hrc>
#include <bf_svx/adjitem.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_svx/editstat.hxx>		// EE_CNTRL_RTFSTYLESHEETS
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/paperinf.hxx>
#include <bf_svx/shaditem.hxx>
#include <bf_svx/sizeitem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_sfx2/printer.hxx>
#ifndef _SFXDOCFILE_HXX //autogen
#include <bf_sfx2/docfile.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#ifdef MAC
#define RGBColor MAC_RGBColor
#undef RGBColor
#else
#endif


//#if defined( WIN ) || defined( WNT )
//#include <svwin.h>
//#endif


#include "docsh.hxx"
#include "stlpool.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "dociter.hxx"
#include "cell.hxx"
#include "globstr.hrc"
#include "bf_sc.hrc"
#include "printopt.hxx"

#define _PRINTFUN_CXX
#include "printfun.hxx"
namespace binfilter {



#define ZOOM_MIN	10

#define GET_BOOL(set,which)   ((const SfxBoolItem&)(set)->Get((which))).GetValue()
#define GET_USHORT(set,which) ((const SfxUInt16Item&)(set)->Get((which))).GetValue()
#define GET_SHOW(set,which)   ( VOBJ_MODE_SHOW == ScVObjMode( ((const ScViewObjectModeItem&)(set)->Get((which))).GetValue()) )

//------------------------------------------------------------------------



/*N*/ void ScPageRowEntry::SetPagesX(USHORT nNew)
/*N*/ {
/*N*/ 	if (pHidden)
/*N*/ 	{
/*N*/ 		DBG_ERROR("SetPagesX nicht nach SetHidden");
/*N*/ 		delete[] pHidden;
/*N*/ 		pHidden = NULL;
/*N*/ 	}
/*N*/ 	nPagesX = nNew;
/*N*/ }




//------------------------------------------------------------------------

/*N*/ long lcl_LineTotal(const SvxBorderLine* pLine)
/*N*/ {
/*N*/ 	return pLine ? ( pLine->GetOutWidth() + pLine->GetInWidth() + pLine->GetDistance() ) : 0;
/*N*/ }

/*M*/ void ScPrintFunc::Construct( const ScPrintOptions* pOptions )
/*M*/ {
/*M*/ 	pDoc = pDocShell->GetDocument();
/*M*/ 
/*M*/ 	SfxPrinter* pDocPrinter = pDoc->GetPrinter();	// auch fuer Preview den Drucker nehmen
/*M*/ 	if (pDocPrinter)
/*M*/ 		aOldPrinterMode = pDocPrinter->GetMapMode();
/*M*/ 
/*M*/ 	//	einheitlicher MapMode ueber alle Aufrufe (z.B. Repaint !!!),
/*M*/ 	//	weil die EditEngine sonst unterschiedliche Texthoehen liefert
/*M*/ 	pDev->SetMapMode(MAP_PIXEL);
/*M*/ 
/*M*/ 	pPageEndX = NULL;
/*M*/ 	pPageEndY = NULL;
/*M*/ 	pPageRows = NULL;
/*M*/ 	pBorderItem = NULL;
/*M*/ 	pBackgroundItem = NULL;
/*M*/ 	pShadowItem = NULL;
/*M*/ 
/*M*/ 	pEditEngine = NULL;
/*M*/ 	pEditDefaults = NULL;
/*M*/ 
/*M*/ 	ScStyleSheetPool* pStylePool	= pDoc->GetStyleSheetPool();
/*M*/ 	SfxStyleSheetBase* pStyleSheet  = pStylePool->Find(
/*M*/ 											pDoc->GetPageStyle( nPrintTab ),
/*M*/ 											SFX_STYLE_FAMILY_PAGE );
/*M*/ 	if (pStyleSheet)
/*M*/ 		pParamSet = &pStyleSheet->GetItemSet();
/*M*/ 	else
/*M*/ 	{
/*M*/ 		DBG_ERROR("Seitenvorlage nicht gefunden" );
/*M*/ 		pParamSet = NULL;
/*M*/ 	}
/*M*/ 
/*M*/ 	if (!bState)
/*M*/ 		nZoom = 100;
/*M*/ 	nManualZoom = 100;
/*M*/ 	bClearWin = FALSE;
/*M*/ 	bUseStyleColor = FALSE;
/*M*/ 	bIsRender = FALSE;
/*M*/ 
/*M*/ 	InitParam(pOptions);
/*M*/ 
/*M*/ 	pPageData = NULL;		// wird nur zur Initialisierung gebraucht
/*M*/ }

/*N*/ ScPrintFunc::ScPrintFunc( ScDocShell* pShell, SfxPrinter* pNewPrinter, USHORT nTab,
/*N*/ 							long nPage, long nDocP, const ScRange* pArea,
/*N*/ 							const ScPrintOptions* pOptions,
/*N*/ 							ScPageBreakData* pData )
/*N*/ 	:	pDocShell			( pShell ),
/*N*/ 		pPrinter			( pNewPrinter ),
/*N*/ 		pDrawView			( NULL ),
/*N*/ 		nPrintTab			( nTab ),
/*N*/ 		nPageStart			( nPage ),
/*N*/ 		nDocPages			( nDocP ),
/*N*/ 		pUserArea			( pArea ),
/*N*/ 		pPageData			( pData ),
/*N*/ 		nTotalPages			( 0 ),
/*N*/ 		nTabPages			( 0 ),
/*N*/ 		bState				( FALSE ),
/*N*/ 		bPrintCurrentTable	( FALSE ),
/*N*/ 		bMultiArea			( FALSE ),
/*N*/ 		bSourceRangeValid	( FALSE )
/*N*/ {
/*N*/ 	pDev = pPrinter;
/*N*/ 	aSrcOffset = pPrinter->PixelToLogic( pPrinter->GetPageOffsetPixel(), MAP_100TH_MM );
/*N*/ 	Construct( pOptions );
/*N*/ }

/*N*/ ScPrintFunc::~ScPrintFunc()
/*N*/ {
/*N*/ 	ScTripel* pTripel = (ScTripel*) aNotePosList.First();
/*N*/ 	while (pTripel)
/*N*/ 	{
/*?*/ 		delete pTripel;
/*?*/ 		pTripel = (ScTripel*) aNotePosList.Next();
/*N*/ 	}
/*N*/ 	aNotePosList.Clear();
/*N*/ 
/*N*/ 	delete[] pPageEndX;
/*N*/ 	delete[] pPageEndY;
/*N*/ 	delete[] pPageRows;
/*N*/ 	delete pEditDefaults;
/*N*/ 	delete pEditEngine;
/*N*/ 
/*N*/ 	//	Druckereinstellungen werden jetzt von aussen wiederhergestellt
/*N*/ 
/*N*/ 	//	#64294# Fuer DrawingLayer/Charts muss der MapMode am Drucker (RefDevice) immer stimmen
/*N*/ 	SfxPrinter* pDocPrinter = pDoc->GetPrinter();	// auch fuer Preview den Drucker nehmen
/*N*/ 	if (pDocPrinter)
/*N*/ 		pDocPrinter->SetMapMode(aOldPrinterMode);
/*N*/ }

//
//			Drucken
//

/*N*/ void lcl_FillHFParam( ScPrintHFParam& rParam, const SfxItemSet* pHFSet )
/*N*/ {
/*N*/ 	//	nDistance muss vorher unterschiedlich initalisiert sein
/*N*/ 
/*N*/ 	if ( pHFSet == NULL )
/*N*/ 	{
/*N*/ 		rParam.bEnable	= FALSE;
/*N*/ 		rParam.pBorder	= NULL;
/*N*/ 		rParam.pBack	= NULL;
/*N*/ 		rParam.pShadow	= NULL;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rParam.bEnable	= ((const SfxBoolItem&) pHFSet->Get(ATTR_PAGE_ON)).GetValue();
/*N*/ 		rParam.bDynamic	= ((const SfxBoolItem&) pHFSet->Get(ATTR_PAGE_DYNAMIC)).GetValue();
/*N*/ 		rParam.bShared	= ((const SfxBoolItem&) pHFSet->Get(ATTR_PAGE_SHARED)).GetValue();
/*N*/ 		rParam.nHeight	= ((const SvxSizeItem&) pHFSet->Get(ATTR_PAGE_SIZE)).GetSize().Height();
/*N*/ 		const SvxLRSpaceItem* pHFLR = &(const SvxLRSpaceItem&) pHFSet->Get(ATTR_LRSPACE);
/*N*/ 		long nTmp;
/*N*/ 		nTmp = pHFLR->GetLeft();
/*N*/ 		rParam.nLeft = nTmp < 0 ? 0 : USHORT(nTmp);
/*N*/ 		nTmp = pHFLR->GetRight();
/*N*/ 		rParam.nRight = nTmp < 0 ? 0 : USHORT(nTmp);
/*N*/ 		rParam.pBorder	= (const SvxBoxItem*)   &pHFSet->Get(ATTR_BORDER);
/*N*/ 		rParam.pBack	= (const SvxBrushItem*) &pHFSet->Get(ATTR_BACKGROUND);
/*N*/ 		rParam.pShadow	= (const SvxShadowItem*)&pHFSet->Get(ATTR_SHADOW);;
/*N*/ 
/*N*/ //	jetzt doch wieder schon im Dialog:
/*N*/ //		rParam.nHeight += rParam.nDistance;				// nicht mehr im Dialog ???
/*N*/ 
/*N*/ 		if (rParam.pBorder)
/*N*/ 			rParam.nHeight += lcl_LineTotal( rParam.pBorder->GetTop() ) +
/*N*/ 							  lcl_LineTotal( rParam.pBorder->GetBottom() );
/*N*/ 
/*N*/ 		rParam.nManHeight = rParam.nHeight;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!rParam.bEnable)
/*N*/ 		rParam.nHeight = 0;
/*N*/ }

//	bNew = TRUE:	benutzten Bereich aus dem Dokument suchen
//	bNew = FALSE:	nur ganze Zeilen/Spalten begrenzen

/*N*/ BOOL ScPrintFunc::AdjustPrintArea( BOOL bNew )
/*N*/ {
/*N*/ 	USHORT nOldEndCol = nEndCol;	// nur wichtig bei !bNew
/*N*/ 	USHORT nOldEndRow = nEndRow;
/*N*/ 	BOOL bChangeCol = TRUE;			// bei bNew werden beide angepasst
/*N*/ 	BOOL bChangeRow = TRUE;
/*N*/ 
/*N*/ 	BOOL bNotes = aTableParam.bNotes;
/*N*/ 	if ( bNew )
/*N*/ 	{
/*N*/ 		nStartCol = nStartRow = 0;
/*N*/ 		if (!pDoc->GetPrintArea( nPrintTab, nEndCol, nEndRow, bNotes ))
/*N*/ 			return FALSE;	// nix
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		BOOL bFound = TRUE;
/*N*/ 		bChangeCol = ( nStartCol == 0 && nEndCol == MAXCOL );
/*N*/ 		bChangeRow = ( nStartRow == 0 && nEndRow == MAXROW );
/*N*/ 		if ( bChangeCol && bChangeRow )
/*?*/ 			bFound = pDoc->GetPrintArea( nPrintTab, nEndCol, nEndRow, bNotes );
/*N*/ 		else if ( bChangeCol )
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 bFound = pDoc->GetPrintAreaHor( nPrintTab, nStartRow, nEndRow, nEndCol, bNotes );
/*N*/ 		else if ( bChangeRow )
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP");}//STRIP001 bFound = pDoc->GetPrintAreaVer( nPrintTab, nStartCol, nEndCol, nEndRow, bNotes );
/*N*/ 
/*N*/ 		if (!bFound)
/*N*/ 			return FALSE;	// leer
/*N*/ 	}
/*N*/ 
/*N*/ 	pDoc->ExtendMerge( nStartCol,nStartRow, nEndCol,nEndRow, nPrintTab,
/*N*/ 						FALSE, TRUE );		// kein Refresh, incl. Attrs
/*N*/ 
/*N*/ 	if ( bChangeCol )
/*N*/ 	{
/*N*/ 		OutputDevice* pRefDev = pDoc->GetPrinter();		// auch fuer Preview den Drucker nehmen
/*N*/ 		pRefDev->SetMapMode( MAP_PIXEL );				// wichtig fuer GetNeededSize
/*N*/ 
/*N*/ 		pDoc->ExtendPrintArea( pRefDev,
/*N*/ 							nPrintTab, nStartCol, nStartRow, nEndCol, nEndRow );
/*N*/ 		//	nEndCol wird veraendert
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( nEndCol < MAXCOL && pDoc->HasAttrib(
/*N*/ 					nEndCol,nStartRow,nPrintTab, nEndCol,nEndRow,nPrintTab, HASATTR_SHADOW_RIGHT ) )
/*N*/ 		++nEndCol;
/*N*/ 	if ( nEndRow < MAXROW && pDoc->HasAttrib(
/*N*/ 					nStartCol,nEndRow,nPrintTab, nEndCol,nEndRow,nPrintTab, HASATTR_SHADOW_DOWN ) )
/*N*/ 		++nEndRow;
/*N*/ 
/*N*/ 	if (!bChangeCol) nEndCol = nOldEndCol;
/*N*/ 	if (!bChangeRow) nEndRow = nOldEndRow;
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ long ScPrintFunc::TextHeight( const EditTextObject* pObject )
/*N*/ {
/*N*/ 	if (!pObject)
/*N*/ 		return 0;
/*N*/ 
/*N*/ //	pEditEngine->SetPageNo( nTotalPages );
/*N*/ 	pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, FALSE );
/*N*/ 
/*N*/ 	return (long) pEditEngine->GetTextHeight();
/*N*/ }

//	nZoom muss gesetzt sein !!!
//	und der entsprechende Twip-MapMode eingestellt

/*N*/ void ScPrintFunc::UpdateHFHeight( ScPrintHFParam& rParam )
/*N*/ {
/*N*/ 	DBG_ASSERT( aPageSize.Width(), "UpdateHFHeight ohne aPageSize");
/*N*/ 
/*N*/ 	if (rParam.bEnable && rParam.bDynamic)
/*N*/ 	{
/*N*/ 		//	nHeight aus Inhalten berechnen
/*N*/ 
/*N*/ 		MakeEditEngine();
/*N*/ 		long nPaperWidth = ( aPageSize.Width() - nLeftMargin - nRightMargin -
/*N*/ 								rParam.nLeft - rParam.nRight ) * 100 / nZoom;
/*N*/ 		if (rParam.pBorder)
/*N*/ 			nPaperWidth -= ( rParam.pBorder->GetDistance(BOX_LINE_LEFT) +
/*N*/ 							 rParam.pBorder->GetDistance(BOX_LINE_RIGHT) +
/*N*/ 							 lcl_LineTotal(rParam.pBorder->GetLeft()) +
/*N*/ 							 lcl_LineTotal(rParam.pBorder->GetRight()) ) * 100 / nZoom;
/*N*/ 
/*N*/ 		if (rParam.pShadow && rParam.pShadow->GetLocation() != SVX_SHADOW_NONE)
/*N*/ 			nPaperWidth -= ( rParam.pShadow->CalcShadowSpace(SHADOW_LEFT) +
/*N*/ 							 rParam.pShadow->CalcShadowSpace(SHADOW_RIGHT) ) * 100L / nZoom;
/*N*/ 
/*N*/ 		pEditEngine->SetPaperSize( Size( nPaperWidth, 10000 ) );
/*N*/ 
/*N*/ 		long nMaxHeight = 0;
/*N*/ 		if ( rParam.pLeft )
/*N*/ 		{
/*N*/ 			nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pLeft->GetLeftArea() ) );
/*N*/ 			nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pLeft->GetCenterArea() ) );
/*N*/ 			nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pLeft->GetRightArea() ) );
/*N*/ 		}
/*N*/ 		if ( rParam.pRight )
/*N*/ 		{
/*N*/ 			nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pRight->GetLeftArea() ) );
/*N*/ 			nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pRight->GetCenterArea() ) );
/*N*/ 			nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pRight->GetRightArea() ) );
/*N*/ 		}
/*N*/ 
/*N*/ 		rParam.nHeight = nMaxHeight + rParam.nDistance;
/*N*/ 		if (rParam.pBorder)
/*N*/ 			rParam.nHeight += rParam.pBorder->GetDistance(BOX_LINE_TOP) +
/*N*/ 							  rParam.pBorder->GetDistance(BOX_LINE_BOTTOM) +
/*N*/ 							  lcl_LineTotal( rParam.pBorder->GetTop() ) +
/*N*/ 							  lcl_LineTotal( rParam.pBorder->GetBottom() );
/*N*/ 		if (rParam.pShadow && rParam.pShadow->GetLocation() != SVX_SHADOW_NONE)
/*N*/ 			rParam.nHeight += rParam.pShadow->CalcShadowSpace(SHADOW_TOP) +
/*N*/ 							  rParam.pShadow->CalcShadowSpace(SHADOW_BOTTOM);
/*N*/ 
/*N*/ 		if (rParam.nHeight < rParam.nManHeight)
/*N*/ 			rParam.nHeight = rParam.nManHeight;			// eingestelltes Minimum
/*N*/ 	}
/*N*/ }

/*N*/ void ScPrintFunc::InitParam( const ScPrintOptions* pOptions )
/*N*/ {
/*N*/ 	if (!pParamSet)
/*N*/ 		return;
/*N*/ 
/*N*/ 								// TabPage "Seite"
/*N*/ 	const SvxLRSpaceItem* pLRItem = (const SvxLRSpaceItem*) &pParamSet->Get( ATTR_LRSPACE );
/*N*/ 	long nTmp;
/*N*/ 	nTmp = pLRItem->GetLeft();
/*N*/ 	nLeftMargin = nTmp < 0 ? 0 : USHORT(nTmp);
/*N*/ 	nTmp = pLRItem->GetRight();
/*N*/ 	nRightMargin = nTmp < 0 ? 0 : USHORT(nTmp);
/*N*/ 	const SvxULSpaceItem* pULItem = (const SvxULSpaceItem*) &pParamSet->Get( ATTR_ULSPACE );
/*N*/ 	nTopMargin    = pULItem->GetUpper();
/*N*/ 	nBottomMargin = pULItem->GetLower();
/*N*/ 
/*N*/ 	const SvxPageItem* pPageItem = (const SvxPageItem*) &pParamSet->Get( ATTR_PAGE );
/*N*/ 	nPageUsage			= pPageItem->GetPageUsage();
/*N*/ 	bLandscape			= pPageItem->IsLandscape();
/*N*/ 	aFieldData.eNumType	= pPageItem->GetNumType();
/*N*/ 
/*N*/ 	bCenterHor = ((const SfxBoolItem&) pParamSet->Get(ATTR_PAGE_HORCENTER)).GetValue();
/*N*/ 	bCenterVer = ((const SfxBoolItem&) pParamSet->Get(ATTR_PAGE_VERCENTER)).GetValue();
/*N*/ 
/*N*/ 	aPageSize = ((const SvxSizeItem&) pParamSet->Get(ATTR_PAGE_SIZE)).GetSize();
/*N*/ 	if ( !aPageSize.Width() || !aPageSize.Height() )
/*N*/ 	{
/*?*/ 		DBG_ERROR("PageSize Null ?!?!?");
/*?*/ 		aPageSize = SvxPaperInfo::GetPaperSize( SVX_PAPER_A4 );
/*N*/ 	}
/*N*/ 
/*N*/ 	pBorderItem		= (const SvxBoxItem*)    &pParamSet->Get(ATTR_BORDER);
/*N*/ 	pBackgroundItem	= (const SvxBrushItem*)	 &pParamSet->Get(ATTR_BACKGROUND);
/*N*/ 	pShadowItem		= (const SvxShadowItem*) &pParamSet->Get(ATTR_SHADOW);
/*N*/ 
/*N*/ 								// TabPage "Kopfzeile"
/*N*/ 
/*N*/ 	aHdr.pLeft		= (const ScPageHFItem*)	&pParamSet->Get(ATTR_PAGE_HEADERLEFT);		// Inhalt
/*N*/ 	aHdr.pRight		= (const ScPageHFItem*)	&pParamSet->Get(ATTR_PAGE_HEADERRIGHT);
/*N*/ 
/*N*/ 	const SvxSetItem* pHeaderSetItem;
/*N*/ 	const SfxItemSet* pHeaderSet = NULL;
/*N*/ 	if ( pParamSet->GetItemState( ATTR_PAGE_HEADERSET, FALSE,
/*N*/ 							(const SfxPoolItem**)&pHeaderSetItem ) == SFX_ITEM_SET )
/*N*/ 	{
/*N*/ 		pHeaderSet = &pHeaderSetItem->GetItemSet();
/*N*/ 														// Kopfzeile hat unteren Abstand
/*N*/ 		aHdr.nDistance	= ((const SvxULSpaceItem&) pHeaderSet->Get(ATTR_ULSPACE)).GetLower();
/*N*/ 	}
/*N*/ 	lcl_FillHFParam( aHdr, pHeaderSet );
/*N*/ 
/*N*/ 								// TabPage "Fusszeile"
/*N*/ 
/*N*/ 	aFtr.pLeft		= (const ScPageHFItem*)	&pParamSet->Get(ATTR_PAGE_FOOTERLEFT);		// Inhalt
/*N*/ 	aFtr.pRight		= (const ScPageHFItem*)	&pParamSet->Get(ATTR_PAGE_FOOTERRIGHT);
/*N*/ 
/*N*/ 	const SvxSetItem* pFooterSetItem;
/*N*/ 	const SfxItemSet* pFooterSet = NULL;
/*N*/ 	if ( pParamSet->GetItemState( ATTR_PAGE_FOOTERSET, FALSE,
/*N*/ 							(const SfxPoolItem**)&pFooterSetItem ) == SFX_ITEM_SET )
/*N*/ 	{
/*N*/ 		pFooterSet = &pFooterSetItem->GetItemSet();
/*N*/ 														// Fusszeile hat oberen Abstand
/*N*/ 		aFtr.nDistance	= ((const SvxULSpaceItem&) pFooterSet->Get(ATTR_ULSPACE)).GetUpper();
/*N*/ 	}
/*N*/ 	lcl_FillHFParam( aFtr, pFooterSet );
/*N*/ 
/*N*/ 	//------------------------------------------------------
/*N*/ 	// Table-/Area-Params aus einzelnen Items zusammenbauen:
/*N*/ 	//------------------------------------------------------
/*N*/ 	// TabPage "Tabelle"
/*N*/ 
/*N*/ 	const SfxUInt16Item* pScaleItem			= NULL;
/*N*/ 	const SfxUInt16Item* pScaleToPagesItem	= NULL;
/*N*/ 	SfxItemState		 eState;
/*N*/ 
/*N*/ 	eState = pParamSet->GetItemState( ATTR_PAGE_SCALE, FALSE,
/*N*/ 									  (const SfxPoolItem**)&pScaleItem );
/*N*/ 	if ( SFX_ITEM_DEFAULT == eState )
/*N*/ 		pScaleItem = (const SfxUInt16Item*)
/*N*/ 					 &pParamSet->GetPool()->
/*N*/ 						GetDefaultItem( ATTR_PAGE_SCALE );
/*N*/ 
/*N*/ 	eState = pParamSet->GetItemState( ATTR_PAGE_SCALETOPAGES, FALSE,
/*N*/ 									  (const SfxPoolItem**)&pScaleToPagesItem );
/*N*/ 	if ( SFX_ITEM_DEFAULT == eState )
/*N*/ 		pScaleToPagesItem = (const SfxUInt16Item*)
/*N*/ 							&pParamSet->GetPool()->
/*N*/ 								GetDefaultItem( ATTR_PAGE_SCALETOPAGES );
/*N*/ 
/*N*/ 	DBG_ASSERT( pScaleItem && pScaleToPagesItem, "Missing ScaleItem! :-/" );
/*N*/ 
/*N*/ 	aTableParam.bNotes			= GET_BOOL(pParamSet,ATTR_PAGE_NOTES);
/*N*/ 	aTableParam.bGrid			= GET_BOOL(pParamSet,ATTR_PAGE_GRID);
/*N*/ 	aTableParam.bHeaders		= GET_BOOL(pParamSet,ATTR_PAGE_HEADERS);
/*N*/ 	aTableParam.bFormulas		= GET_BOOL(pParamSet,ATTR_PAGE_FORMULAS);
/*N*/ 	aTableParam.bNullVals		= GET_BOOL(pParamSet,ATTR_PAGE_NULLVALS);
/*N*/ 	aTableParam.bCharts			= GET_SHOW(pParamSet,ATTR_PAGE_CHARTS);
/*N*/ 	aTableParam.bObjects		= GET_SHOW(pParamSet,ATTR_PAGE_OBJECTS);
/*N*/ 	aTableParam.bDrawings		= GET_SHOW(pParamSet,ATTR_PAGE_DRAWINGS);
/*N*/ 	aTableParam.bTopDown		= GET_BOOL(pParamSet,ATTR_PAGE_TOPDOWN);
/*N*/ 	aTableParam.bLeftRight		= !aTableParam.bLeftRight;
/*N*/ 	aTableParam.nFirstPageNo	= GET_USHORT(pParamSet,ATTR_PAGE_FIRSTPAGENO);
/*N*/ 	if (!aTableParam.nFirstPageNo)
/*N*/ 		aTableParam.nFirstPageNo = (USHORT) nPageStart;		// von vorheriger Tabelle
/*N*/ 
/*N*/ 	if ( pScaleItem && pScaleToPagesItem )
/*N*/ 	{
/*N*/ 		UINT16	nScaleAll     = pScaleItem->GetValue();
/*N*/ 		UINT16	nScaleToPages = pScaleToPagesItem->GetValue();
/*N*/ 
/*N*/ 		aTableParam.bScaleNone		= (nScaleAll     == 100);
/*N*/ 		aTableParam.bScaleAll		= (nScaleAll      > 0  );
/*N*/ 		aTableParam.bScalePageNum	= (nScaleToPages  > 0  );
/*N*/ 		aTableParam.nScaleAll		= nScaleAll;
/*N*/ 		aTableParam.nScalePageNum	= nScaleToPages;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aTableParam.bScaleNone		= TRUE;
/*N*/ 		aTableParam.bScaleAll		= FALSE;
/*N*/ 		aTableParam.bScalePageNum	= FALSE;
/*N*/ 		aTableParam.nScaleAll		= 0;
/*N*/ 		aTableParam.nScalePageNum	= 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	//	skip empty pages only if options with that flag are passed
/*N*/ 	aTableParam.bSkipEmpty = pOptions && pOptions->GetSkipEmpty();
/*N*/ 	if ( pPageData )
/*N*/ 		aTableParam.bSkipEmpty = FALSE;
/*N*/ 	// Wenn pPageData gesetzt ist, interessieren fuer die Umbruch-Vorschau
/*N*/ 	// nur die Umbrueche, leere Seiten werden nicht speziell behandelt
/*N*/ 
/*N*/ 	//------------------------------------------------------
/*N*/ 	// TabPage "Bereiche":
/*N*/ 	//------------------------------------------------------
/*N*/ 
/*N*/ 	//!	alle PrintAreas der Tabelle durchgehen !!!
/*N*/ 	const ScRange*	pPrintArea = pDoc->GetPrintRange( nPrintTab, 0 );
/*N*/ 	const ScRange*	pRepeatCol = pDoc->GetRepeatColRange( nPrintTab );
/*N*/ 	const ScRange*	pRepeatRow = pDoc->GetRepeatRowRange( nPrintTab );
/*N*/ 
/*N*/ 	//	ATTR_PAGE_PRINTTABLES wird ignoriert
/*N*/ 
/*N*/ 	if ( pUserArea )				// UserArea (Selektion) hat Vorrang
/*N*/ 	{
/*?*/ 		bPrintCurrentTable    =
/*?*/ 		aAreaParam.bPrintArea = TRUE;					// Selektion
/*?*/ 		aAreaParam.aPrintArea = *pUserArea;
/*?*/ 
/*?*/ 		//	Die Tabellen-Abfrage ist schon in DocShell::Print, hier immer
/*?*/ 		aAreaParam.aPrintArea.aStart.SetTab(nPrintTab);
/*?*/ 		aAreaParam.aPrintArea.aEnd.SetTab(nPrintTab);
/*?*/ 
/*?*/ //		lcl_LimitRange( aAreaParam.aPrintArea, nPrintTab );			// ganze Zeilen/Spalten...
/*N*/ 	}
/*N*/ 	else if ( pDoc->HasPrintRange() )
/*N*/ 	{
/*N*/ 		if ( pPrintArea )								// mindestens eine gesetzt ?
/*N*/ 		{
/*N*/ 			bPrintCurrentTable    =
/*N*/ 			aAreaParam.bPrintArea = TRUE;
/*N*/ 			aAreaParam.aPrintArea = *pPrintArea;
/*N*/ 
/*N*/ 			bMultiArea = ( pDoc->GetPrintRangeCount(nPrintTab) > 1 );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			aAreaParam.bPrintArea = TRUE;
/*N*/ 			bPrintCurrentTable = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//	#74834# don't print hidden tables if there's no print range defined there
/*N*/ 		if ( pDoc->IsVisible( nPrintTab ) )
/*N*/ 		{
/*N*/ 			aAreaParam.bPrintArea = FALSE;
/*N*/ 			bPrintCurrentTable = TRUE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			aAreaParam.bPrintArea = TRUE;	// otherwise the table is always counted
/*N*/ 			bPrintCurrentTable = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pRepeatCol )
/*N*/ 	{
/*N*/ 		aAreaParam.bRepeatCol = TRUE;
/*N*/ 		aAreaParam.aRepeatCol = *pRepeatCol;
/*N*/ 		nRepeatStartCol	= pRepeatCol->aStart.Col();
/*N*/ 		nRepeatEndCol	= pRepeatCol->aEnd  .Col();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aAreaParam.bRepeatCol = FALSE;
/*N*/ 		nRepeatStartCol = nRepeatEndCol = REPEAT_NONE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pRepeatRow )
/*N*/ 	{
/*?*/ 		aAreaParam.bRepeatRow = TRUE;
/*?*/ 		aAreaParam.aRepeatRow = *pRepeatRow;
/*?*/ 		nRepeatStartRow	= pRepeatRow->aStart.Row();
/*?*/ 		nRepeatEndRow	= pRepeatRow->aEnd  .Row();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aAreaParam.bRepeatRow = FALSE;
/*N*/ 		nRepeatStartRow = nRepeatEndRow = REPEAT_NONE;
/*N*/ 	}
/*N*/ 
/*N*/ 			//
/*N*/ 			//	Seiten aufteilen
/*N*/ 			//
/*N*/ 
/*N*/ 	if (!bState)
/*N*/ 	{
/*N*/ 		nTabPages = CountPages();									// berechnet auch Zoom
/*N*/ 		nTotalPages = nTabPages;
/*N*/ 		nTotalPages += CountNotePages();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		CalcPages();			// nur Umbrueche suchen
/*?*/ 		CountNotePages();		// Notizen zaehlen, auch wenn Seitenzahl schon bekannt
/*N*/ 	}
/*N*/ 
/*N*/ 	if (nDocPages)
/*N*/ 		aFieldData.nTotalPages = nDocPages;
/*N*/ 	else
/*N*/ 		aFieldData.nTotalPages = nTotalPages;
/*N*/ 
/*N*/ 	SetDateTime( Date(), Time() );
/*N*/ 
/*N*/ 	aFieldData.aTitle		= pDocShell->GetTitle();
/*N*/ 	const INetURLObject& rURLObj = pDocShell->GetMedium()->GetURLObject();
/*N*/ 	aFieldData.aLongDocName	= rURLObj.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
/*N*/ 	if ( aFieldData.aLongDocName.Len() )
/*?*/ 		aFieldData.aShortDocName = rURLObj.GetName( INetURLObject::DECODE_UNAMBIGUOUS );
/*N*/ 	else
/*N*/ 		aFieldData.aShortDocName = aFieldData.aLongDocName = aFieldData.aTitle;
/*N*/ 
/*N*/ }



/*N*/ void ScPrintFunc::SetDateTime( const Date& rDate, const Time& rTime )
/*N*/ {
/*N*/ 	aFieldData.aDate = rDate;
/*N*/ 	aFieldData.aTime = rTime;
/*N*/ }


//	Rahmen wird nach innen gezeichnet








/*!!!!!!!!!!!		Notizen in Tabelle markieren ??????????????????????????

    if (aTableParam.bNotes)
    {
        pDev->SetMapMode(aOffsetMode);
        aOutputData.PrintNoteMarks(aNotePosList);
        pDev->SetMapMode(aLogicMode);
    }
*/


/*N*/ void ScPrintFunc::MakeEditEngine()
/*N*/ {
/*N*/ 	if (!pEditEngine)
/*N*/ 	{
/*N*/ 		//	can't use document's edit engine pool here,
/*N*/ 		//	because pool must have twips as default metric
/*N*/ 		pEditEngine = new ScHeaderEditEngine( EditEngine::CreatePool(), TRUE );
/*N*/ 
/*N*/ 		pEditEngine->EnableUndo(FALSE);
/*N*/ 		pEditEngine->SetRefDevice( pDev );
/*N*/ 		pEditEngine->SetWordDelimiters(
/*N*/ 				ScEditUtil::ModifyDelimiters( pEditEngine->GetWordDelimiters() ) );
/*N*/ 		pEditEngine->SetControlWord( pEditEngine->GetControlWord() & ~EE_CNTRL_RTFSTYLESHEETS );
/*N*/ 		pEditEngine->EnableAutoColor( bUseStyleColor );
/*N*/ 
/*N*/ 		//	Default-Set fuer Ausrichtung
/*N*/ 		pEditDefaults = new SfxItemSet( pEditEngine->GetEmptyItemSet() );
/*N*/ 
/*N*/ 		const ScPatternAttr& rPattern = (const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN);
/*N*/ 		rPattern.FillEditItemSet( pEditDefaults );
/*N*/ 		//	FillEditItemSet adjusts font height to 1/100th mm,
/*N*/ 		//	but for header/footer twips is needed, as in the PatternAttr:
/*N*/ 		pEditDefaults->Put( rPattern.GetItem(ATTR_FONT_HEIGHT), EE_CHAR_FONTHEIGHT );
/*N*/ 		pEditDefaults->Put( rPattern.GetItem(ATTR_CJK_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CJK );
/*N*/ 		pEditDefaults->Put( rPattern.GetItem(ATTR_CTL_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CTL );
/*N*/ 		//	#69193# dont use font color, because background color is not used
/*N*/ 		//!	there's no way to set the background for note pages
/*N*/ 		pEditDefaults->ClearItem( EE_CHAR_COLOR );
/*N*/ 	}
/*N*/ 
/*N*/ 	pEditEngine->SetData( aFieldData );		// Seitennummer etc. setzen
/*N*/ }

//
//	UpdatePages wird nur von aussen gerufen, um die Umbrueche fuer die Anzeige
//	richtig zu setzen - immer ohne UserArea
//

/*N*/ BOOL ScPrintFunc::UpdatePages()
/*N*/ {
/*N*/ 	if (!pParamSet)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	//	Zoom
/*N*/ 
/*N*/ 	nZoom = 100;
/*N*/ 	if (aTableParam.bScalePageNum)
/*N*/ 		nZoom = ZOOM_MIN;						// stimmt fuer Umbrueche
/*N*/ 	else if (aTableParam.bScaleAll)
/*N*/ 	{
/*N*/ 		nZoom = aTableParam.nScaleAll;
/*N*/ 		if ( nZoom <= ZOOM_MIN )
/*N*/ 			nZoom = ZOOM_MIN;
/*N*/ 	}
/*N*/ 
/*N*/ 	String aName = pDoc->GetPageStyle( nPrintTab );
/*N*/ 	USHORT nTabCount = pDoc->GetTableCount();
/*N*/ 	for (USHORT nTab=0; nTab<nTabCount; nTab++)
/*N*/ 		if ( nTab==nPrintTab || pDoc->GetPageStyle(nTab)==aName )
/*N*/ 		{
/*N*/ 			//	Wiederholungszeilen / Spalten
/*N*/ 			pDoc->SetRepeatArea( nTab, nRepeatStartCol,nRepeatEndCol, nRepeatStartRow,nRepeatEndRow );
/*N*/ 
/*N*/ 			//	Umbrueche setzen
/*N*/ 			ResetBreaks(nTab);
/*N*/ 			pDocShell->PostPaint(0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID);
/*N*/ 		}
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ long ScPrintFunc::CountPages()							// setzt auch nPagesX, nPagesY
/*N*/ {
/*N*/ 	BOOL bAreaOk = FALSE;
/*N*/ 
/*N*/ 	if (pDoc->HasTable( nPrintTab ))
/*N*/ 	{
/*N*/ 		if (aAreaParam.bPrintArea)							// Druckbereich angegeben?
/*N*/ 		{
/*N*/ 			if ( bPrintCurrentTable )
/*N*/ 			{
/*N*/ 				ScRange& rRange = aAreaParam.aPrintArea;
/*N*/ 
/*N*/ 				//	hier kein Vergleich der Tabellen mehr, die Area gilt immer fuer diese Tabelle
/*N*/ 				//	wenn hier verglichen werden soll, muss die Tabelle der Druckbereiche beim
/*N*/ 				//	Einfuegen von Tabellen etc. angepasst werden !
/*N*/ 
/*N*/ 				nStartCol = rRange.aStart.Col();
/*N*/ 				nStartRow = rRange.aStart.Row();
/*N*/ 				nEndCol   = rRange.aEnd  .Col();
/*N*/ 				nEndRow   = rRange.aEnd  .Row();
/*N*/ 				bAreaOk   = AdjustPrintArea(FALSE);			// begrenzen
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bAreaOk = FALSE;
/*N*/ 		}
/*N*/ 		else												// aus Dokument suchen
/*N*/ 			bAreaOk = AdjustPrintArea(TRUE);
/*N*/ 	}
/*N*/ 
/*N*/ 	if (bAreaOk)
/*N*/ 	{
/*N*/ 		long nPages = 0;
/*N*/ 		USHORT nY;
/*N*/ 		if (bMultiArea)
/*N*/ 		{
/*?*/ 			USHORT nRCount = pDoc->GetPrintRangeCount( nPrintTab );
/*?*/ 			for (USHORT i=0; i<nRCount; i++)
/*?*/ 			{
/*?*/ 				CalcZoom(i);
/*?*/ 				if ( aTableParam.bSkipEmpty )
/*?*/ 					for (nY=0; nY<nPagesY; nY++)
                            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 						nPages += pPageRows[nY].CountVisible();
/*?*/ 				else
/*?*/ 					nPages += ((long) nPagesX) * nPagesY;
/*?*/ 				if ( pPageData )
                        {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 					FillPageData();
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			CalcZoom(RANGENO_NORANGE);						// Zoom berechnen
/*N*/ 			if ( aTableParam.bSkipEmpty )
/*?*/ 				for (nY=0; nY<nPagesY; nY++)
                        {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 					nPages += pPageRows[nY].CountVisible();
/*N*/ 			else
/*N*/ 				nPages += ((long) nPagesX) * nPagesY;
/*N*/ 			if ( pPageData )
                    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 				FillPageData();
/*N*/ 		}
/*N*/ 		return nPages;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ //		nZoom = 100;						// nZoom auf letztem Wert stehenlassen !!!
/*N*/ 		nPagesX = nPagesY = nTotalY = 0;
/*N*/ 		return 0;
/*N*/ 	}
/*N*/ }

/*N*/ long ScPrintFunc::CountNotePages()
/*N*/ {
/*N*/ 	if ( !aTableParam.bNotes || !bPrintCurrentTable )
/*N*/ 		return 0;
/*N*/ 
/*?*/ 	long nCount=0;
/*?*/ 	USHORT nCol;
/*?*/ 	USHORT nRow;
/*?*/ 
/*?*/ 	BOOL bError = FALSE;
/*?*/ 	if (!aAreaParam.bPrintArea)
/*?*/ 		bError = !AdjustPrintArea(TRUE);			// komplett aus Dok suchen
/*?*/ 
/*?*/ 	USHORT nRepeats = 1;							// wie oft durchgehen ?
/*?*/ 	if (bMultiArea)
/*?*/ 		nRepeats = pDoc->GetPrintRangeCount(nPrintTab);
/*?*/ 	if (bError)
/*?*/ 		nRepeats = 0;
/*?*/ 
/*?*/ 	for (USHORT nStep=0; nStep<nRepeats; nStep++)
/*?*/ 	{
/*?*/ 		BOOL bDoThis = TRUE;
/*?*/ 		if (bMultiArea)				// alle Areas durchgehen
/*?*/ 		{
/*?*/ 			const ScRange* pThisRange = pDoc->GetPrintRange( nPrintTab, nStep );
/*?*/ 			if ( pThisRange )
/*?*/ 			{
/*?*/ 				nStartCol = pThisRange->aStart.Col();
/*?*/ 				nStartRow = pThisRange->aStart.Row();
/*?*/ 				nEndCol   = pThisRange->aEnd  .Col();
/*?*/ 				nEndRow   = pThisRange->aEnd  .Row();
/*?*/ 				bDoThis = AdjustPrintArea(FALSE);
/*?*/ 			}
/*?*/ 		}
/*?*/ 
/*?*/ 		if (bDoThis)
/*?*/ 		{
/*?*/ 			ScHorizontalCellIterator aIter( pDoc, nPrintTab, nStartCol,nStartRow, nEndCol,nEndRow );
/*?*/ 			ScBaseCell* pCell = aIter.GetNext( nCol, nRow );
/*?*/ 			while (pCell)
/*?*/ 			{
/*?*/ 				if (pCell->GetNotePtr())
/*?*/ 				{
/*?*/ 					aNotePosList.Insert( new ScTripel( nCol,nRow,nPrintTab ), LIST_APPEND );
/*?*/ 					++nCount;
/*?*/ 				}
/*?*/ 
/*?*/ 				pCell = aIter.GetNext( nCol, nRow );
/*?*/ 			}
/*?*/ 		}
/*?*/ 	}
/*?*/ 
/*?*/ 	long nPages = 0;
/*?*/ 	long nNoteNr = 0;
/*?*/ 	long nNoteAdd;
/*?*/ 	do
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP"); nNoteAdd=0;//STRIP001 
/*?*/ 	}
/*?*/ 	while (nNoteAdd);
/*?*/ 
/*?*/ 	return nPages;
/*N*/ }

/*N*/ void ScPrintFunc::InitModes()				// aus nZoom etc. die MapModes setzen
/*N*/ {
/*N*/ 	aOffset = Point( aSrcOffset.X()*100/nZoom, aSrcOffset.Y()*100/nZoom );
/*N*/ 
/*N*/ 	long nEffZoom = nZoom * (long) nManualZoom;
/*N*/ 
/*N*/ //	nScaleX = nScaleY = 1.0;			// Ausgabe in Twips
/*N*/ 	nScaleX = nScaleY = HMM_PER_TWIPS;	// Ausgabe in 1/100 mm
/*N*/ 
/*N*/ 	Fraction aZoomFract( nEffZoom,10000 );
/*N*/ 	Fraction aHorFract = aZoomFract;
/*N*/ 
/*N*/ 	if ( !pPrinter && !bIsRender )							// adjust scale for preview
/*N*/ 	{
/*?*/ 		double nFact = pDocShell->GetOutputFactor();
/*?*/ 		aHorFract = Fraction( (long)( nEffZoom / nFact ), 10000 );
/*N*/ 	}
/*N*/ 
/*N*/ 	aLogicMode = MapMode( MAP_100TH_MM, Point(), aHorFract, aZoomFract );
/*N*/ 
/*N*/ 	Point aLogicOfs( -aOffset.X(), -aOffset.Y() );
/*N*/ 	aOffsetMode = MapMode( MAP_100TH_MM, aLogicOfs, aHorFract, aZoomFract );
/*N*/ 
/*N*/ 	Point aTwipsOfs( (long) ( -aOffset.X() / nScaleX + 0.5 ), (long) ( -aOffset.Y() / nScaleY + 0.5 ) );
/*N*/ 	aTwipMode = MapMode( MAP_TWIP, aTwipsOfs, aHorFract, aZoomFract );
/*N*/ }

/*N*/ void ScPrintFunc::CalcZoom( USHORT nRangeNo )						// Zoom berechnen
/*N*/ {
/*N*/ 	USHORT nRCount = pDoc->GetPrintRangeCount( nPrintTab );
/*N*/ 	const ScRange* pThisRange = NULL;
/*N*/ 	if ( nRangeNo != RANGENO_NORANGE || nRangeNo < nRCount )
/*?*/ 		pThisRange = pDoc->GetPrintRange( nPrintTab, nRangeNo );
/*N*/ 	if ( pThisRange )
/*N*/ 	{
/*?*/ 		nStartCol = pThisRange->aStart.Col();
/*?*/ 		nStartRow = pThisRange->aStart.Row();
/*?*/ 		nEndCol   = pThisRange->aEnd  .Col();
/*?*/ 		nEndRow   = pThisRange->aEnd  .Row();
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!AdjustPrintArea(FALSE))						// leer
/*N*/ 	{
/*N*/ 		nZoom = 100;
/*N*/ 		nPagesX = nPagesY = nTotalY = 0;
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	pDoc->SetRepeatArea( nPrintTab, nRepeatStartCol,nRepeatEndCol, nRepeatStartRow,nRepeatEndRow );
/*N*/ 
/*N*/ 	if (aTableParam.bScalePageNum)
/*N*/ 	{
/*N*/ 		nZoom = 100;
/*N*/ 		BOOL bFound = FALSE;
/*N*/ 		USHORT nPagesToFit = aTableParam.nScalePageNum;
/*N*/ 		while (!bFound)
/*N*/ 		{
/*N*/ 			CalcPages();
/*N*/ 			if ( nPagesX * nPagesY <= nPagesToFit || nZoom <= ZOOM_MIN )
/*N*/ 				bFound = TRUE;
/*N*/ 			else
/*N*/ 				--nZoom;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if (aTableParam.bScaleAll)
/*N*/ 	{
/*N*/ 		nZoom = aTableParam.nScaleAll;
/*N*/ 		if ( nZoom <= ZOOM_MIN )
/*N*/ 			nZoom = ZOOM_MIN;
/*N*/ 		CalcPages();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ASSERT( aTableParam.bScaleNone, "kein Scale-Flag gesetzt" );
/*N*/ 		nZoom = 100;
/*N*/ 		CalcPages();
/*N*/ 	}
/*N*/ }

/*N*/ Size ScPrintFunc::GetDocPageSize()
/*N*/ {
/*N*/ 						// Hoehe Kopf-/Fusszeile anpassen
/*N*/ 
/*N*/ 	InitModes();							// aTwipMode aus nZoom initialisieren
/*N*/ 	pDev->SetMapMode( aTwipMode );			// Kopf-/Fusszeilen in Twips
/*N*/ 	UpdateHFHeight( aHdr );
/*N*/ 	UpdateHFHeight( aFtr );
/*N*/ 
/*N*/ 						// Seitengroesse in Document-Twips
/*N*/ 						// 	Berechnung Left / Right auch in PrintPage
/*N*/ 
/*N*/ 	aPageRect = Rectangle( Point(), aPageSize );
/*N*/ 	aPageRect.Left()   = ( aPageRect.Left()   + nLeftMargin					 ) * 100 / nZoom;
/*N*/ 	aPageRect.Right()  = ( aPageRect.Right()  - nRightMargin				 ) * 100 / nZoom;
/*N*/ 	aPageRect.Top()    = ( aPageRect.Top()    + nTopMargin    ) * 100 / nZoom + aHdr.nHeight;
/*N*/ 	aPageRect.Bottom() = ( aPageRect.Bottom() - nBottomMargin ) * 100 / nZoom - aFtr.nHeight;
/*N*/ 
/*N*/ 	Size aDocPageSize = aPageRect.GetSize();
/*N*/ 	if (aTableParam.bHeaders)
/*N*/ 	{
/*N*/ 		aDocPageSize.Width()  -= (long) PRINT_HEADER_WIDTH;
/*N*/ 		aDocPageSize.Height() -= (long) PRINT_HEADER_HEIGHT;
/*N*/ 	}
/*N*/ 	if (pBorderItem)
/*N*/ 	{
/*N*/ 		aDocPageSize.Width()  -= lcl_LineTotal(pBorderItem->GetLeft()) +
/*N*/ 								 lcl_LineTotal(pBorderItem->GetRight()) +
/*N*/ 								 pBorderItem->GetDistance(BOX_LINE_LEFT) +
/*N*/ 								 pBorderItem->GetDistance(BOX_LINE_RIGHT);
/*N*/ 		aDocPageSize.Height() -= lcl_LineTotal(pBorderItem->GetTop()) +
/*N*/ 								 lcl_LineTotal(pBorderItem->GetBottom()) +
/*N*/ 								 pBorderItem->GetDistance(BOX_LINE_TOP) +
/*N*/ 								 pBorderItem->GetDistance(BOX_LINE_BOTTOM);
/*N*/ 	}
/*N*/ 	if (pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE)
/*N*/ 	{
/*?*/ 		aDocPageSize.Width()  -= pShadowItem->CalcShadowSpace(SHADOW_LEFT) +
/*?*/ 								 pShadowItem->CalcShadowSpace(SHADOW_RIGHT);
/*?*/ 		aDocPageSize.Height() -= pShadowItem->CalcShadowSpace(SHADOW_TOP) +
/*?*/ 								 pShadowItem->CalcShadowSpace(SHADOW_BOTTOM);
/*N*/ 	}
/*N*/ 	return aDocPageSize;
/*N*/ }

/*N*/ void ScPrintFunc::ResetBreaks( USHORT nTab )			// Breaks fuer Anzeige richtig setzen
/*N*/ {
/*N*/ 	pDoc->SetPageSize( nTab, GetDocPageSize() );
/*N*/ 	pDoc->UpdatePageBreaks( nTab, NULL );
/*N*/ }


/*N*/ void ScPrintFunc::CalcPages()				// berechnet aPageRect und Seiten aus nZoom
/*N*/ {
/*N*/ 	if (!pPageEndX) pPageEndX = new USHORT[MAXCOL+1];
/*N*/ 	if (!pPageEndY) pPageEndY = new USHORT[MAXROW+1];
/*N*/ 	if (!pPageRows) pPageRows = new ScPageRowEntry[MAXROW+1];	//! vorher zaehlen !!!!
/*N*/ 
/*N*/ 	pDoc->SetPageSize( nPrintTab, GetDocPageSize() );
/*N*/ 	if (aAreaParam.bPrintArea)
/*N*/ 	{
/*N*/ 		ScRange aRange( nStartCol, nStartRow, nPrintTab, nEndCol, nEndRow, nPrintTab );
/*N*/ 		pDoc->UpdatePageBreaks( nPrintTab, &aRange );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pDoc->UpdatePageBreaks( nPrintTab, NULL );		// sonst wird das Ende markiert
/*N*/ 
/*N*/ 	//
/*N*/ 	//	Seiteneinteilung nach Umbruechen in Col/RowFlags
/*N*/ 	//	Von mehreren Umbruechen in einem ausgeblendeten Bereich zaehlt nur einer.
/*N*/ 	//
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	nPagesX = 0;
/*N*/ 	nPagesY = 0;
/*N*/ 	nTotalY = 0;
/*N*/ 
/*N*/ 	BOOL bVisCol = FALSE;
/*N*/ 	for (i=nStartCol; i<=nEndCol; i++)
/*N*/ 	{
/*N*/ 		BYTE nFlags = pDoc->GetColFlags(i,nPrintTab);
/*N*/ 		if ( i>nStartCol && bVisCol && (nFlags & CR_PAGEBREAK) )
/*N*/ 		{
/*N*/ 			pPageEndX[nPagesX] = i-1;
/*N*/ 			++nPagesX;
/*N*/ 			bVisCol = FALSE;
/*N*/ 		}
/*N*/ 		if (!(nFlags & CR_HIDDEN))
/*N*/ 			bVisCol = TRUE;
/*N*/ 	}
/*N*/ 	if (bVisCol)	// auch am Ende keine leeren Seiten
/*N*/ 	{
/*N*/ 		pPageEndX[nPagesX] = nEndCol;
/*N*/ 		++nPagesX;
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bVisRow = FALSE;
/*N*/ 	USHORT nPageStartRow = nStartRow;
/*N*/ 	for (i=nStartRow; i<=nEndRow; i++)
/*N*/ 	{
/*N*/ 		BYTE nFlags = pDoc->GetRowFlags(i,nPrintTab);
/*N*/ 		if ( i>nStartRow && bVisRow && (nFlags & CR_PAGEBREAK) )
/*N*/ 		{
/*N*/ 			pPageEndY[nTotalY] = i-1;
/*N*/ 			++nTotalY;
/*N*/ 
/*N*/ 			if ( !aTableParam.bSkipEmpty ||
/*N*/ 					!pDoc->IsPrintEmpty( nPrintTab, nStartCol, nPageStartRow, nEndCol, i-1 ) )
/*N*/ 			{
/*N*/ 				pPageRows[nPagesY].SetStartRow( nPageStartRow );
/*N*/ 				pPageRows[nPagesY].SetEndRow( i-1 );
/*N*/ 				pPageRows[nPagesY].SetPagesX( nPagesX );
/*N*/ 				if (aTableParam.bSkipEmpty)
                        {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 					lcl_SetHidden( pDoc, nPrintTab, pPageRows[nPagesY], nStartCol, pPageEndX );
/*N*/ 				++nPagesY;
/*N*/ 			}
/*N*/ 
/*N*/ 			nPageStartRow = i;
/*N*/ 			bVisRow = FALSE;
/*N*/ 		}
/*N*/ 		if (!(nFlags & CR_HIDDEN))
/*N*/ 			bVisRow = TRUE;
/*N*/ 	}
/*N*/ 	if (bVisRow)
/*N*/ 	{
/*N*/ 		pPageEndY[nTotalY] = nEndRow;
/*N*/ 		++nTotalY;
/*N*/ 
/*N*/ 		if ( !aTableParam.bSkipEmpty ||
/*N*/ 				!pDoc->IsPrintEmpty( nPrintTab, nStartCol, nPageStartRow, nEndCol, nEndRow ) )
/*N*/ 		{
/*N*/ 			pPageRows[nPagesY].SetStartRow( nPageStartRow );
/*N*/ 			pPageRows[nPagesY].SetEndRow( nEndRow );
/*N*/ 			pPageRows[nPagesY].SetPagesX( nPagesX );
/*N*/ 			if (aTableParam.bSkipEmpty)
                    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 				lcl_SetHidden( pDoc, nPrintTab, pPageRows[nPagesY], nStartCol, pPageEndX );
/*N*/ 			++nPagesY;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

}
