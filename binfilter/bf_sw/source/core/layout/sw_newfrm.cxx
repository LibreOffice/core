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

#ifndef _SVDMODEL_HXX //autogen
#include <bf_svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <bf_svx/svdpage.hxx>
#endif

#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _VIRTOUTP_HXX
#include <virtoutp.hxx>
#endif
#ifndef _BLINK_HXX
#include <blink.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#ifndef _NOTXTFRM_HXX
#include <notxtfrm.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
namespace binfilter {

/*N*/ #ifndef VERTICAL_LAYOUT
/*N*/ PtPtr pX = &Point::nA;
/*N*/ PtPtr pY = &Point::nB;
/*N*/ SzPtr pWidth = &Size::nA;
/*N*/ SzPtr pHeight = &Size::nB;
/*N*/ #endif

/*N*/ SwLayVout	  *SwRootFrm::pVout = 0;
/*N*/ BOOL 		   SwRootFrm::bInPaint = FALSE;
/*N*/ BOOL 		   SwRootFrm::bNoVirDev = FALSE;

/*N*/ SwCache *SwFrm::pCache = 0;

/*N*/ Bitmap* SwNoTxtFrm::pErrorBmp = 0;
/*N*/ Bitmap* SwNoTxtFrm::pReplaceBmp = 0;
/*N*/ 
/*N*/ #ifdef VERTICAL_LAYOUT

/*N*/ long FirstMinusSecond( long nFirst, long nSecond )
/*N*/     { return nFirst - nSecond; }
/*N*/ long SecondMinusFirst( long nFirst, long nSecond )
/*N*/     { return nSecond - nFirst; }
/*N*/ long SwIncrement( long nA, long nAdd )
/*N*/     { return nA + nAdd; }
/*N*/ long SwDecrement( long nA, long nSub )
/*N*/     { return nA - nSub; }

/*N*/ static SwRectFnCollection aHorizontal = {
/*N*/     /* fnRectGet      */
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::TopLeft,
/*N*/     &SwRect::_Size,
/*N*/     /* fnRectSet      */
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::_Height,
/*N*/ 
/*N*/     &SwRect::SubTop,
/*N*/     &SwRect::AddBottom,
/*N*/     &SwRect::SubLeft,
/*N*/     &SwRect::AddRight,
/*N*/     &SwRect::AddWidth,
/*N*/     &SwRect::AddHeight,
/*N*/ 
/*N*/     &SwRect::SetPosX,
/*N*/     &SwRect::SetPosY,
/*N*/ 
/*N*/     &SwFrm::GetTopMargin,
/*N*/     &SwFrm::GetBottomMargin,
/*N*/     &SwFrm::GetLeftMargin,
/*N*/     &SwFrm::GetRightMargin,
/*N*/     &SwFrm::SetLeftRightMargins,
/*N*/     &SwFrm::SetTopBottomMargins,
/*N*/     &SwFrm::GetPrtTop,
/*N*/     &SwFrm::GetPrtBottom,
/*N*/     &SwFrm::GetPrtLeft,
/*N*/     &SwFrm::GetPrtRight,
/*N*/     &SwRect::GetTopDistance,
/*N*/     &SwRect::GetBottomDistance,
/*N*/     &SwRect::GetLeftDistance,
/*N*/     &SwRect::GetRightDistance,
/*N*/     &SwFrm::SetMaxBottom,
/*N*/     &SwRect::OverStepBottom,
/*N*/ 
/*N*/     &SwRect::SetUpperLeftCorner,
/*N*/     &SwFrm::MakeBelowPos,
/*N*/     &FirstMinusSecond,
/*N*/     &FirstMinusSecond,
/*N*/     &SwIncrement,
/*N*/     &SwIncrement,
/*N*/     &SwRect::SetLeftAndWidth,
/*N*/     &SwRect::SetTopAndHeight
/*N*/ };
/*N*/ 
/*N*/ static SwRectFnCollection aVertical = {
/*N*/     /* fnRectGet      */
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::TopRight,
/*N*/     &SwRect::SwappedSize,
/*N*/     /* fnRectSet      */
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::_Width,
/*N*/ 
/*N*/     &SwRect::AddRight,
/*N*/     &SwRect::SubLeft,
/*N*/     &SwRect::SubTop,
/*N*/     &SwRect::AddBottom,
/*N*/     &SwRect::AddHeight,
/*N*/     &SwRect::AddWidth,
/*N*/ 
/*N*/     &SwRect::SetPosY,
/*N*/     &SwRect::SetPosX,
/*N*/ 
/*N*/     &SwFrm::GetRightMargin,
/*N*/     &SwFrm::GetLeftMargin,
/*N*/     &SwFrm::GetTopMargin,
/*N*/     &SwFrm::GetBottomMargin,
/*N*/     &SwFrm::SetTopBottomMargins,
/*N*/     &SwFrm::SetRightLeftMargins,
/*N*/     &SwFrm::GetPrtRight,
/*N*/     &SwFrm::GetPrtLeft,
/*N*/     &SwFrm::GetPrtTop,
/*N*/     &SwFrm::GetPrtBottom,
/*N*/     &SwRect::GetRightDistance,
/*N*/     &SwRect::GetLeftDistance,
/*N*/     &SwRect::GetTopDistance,
/*N*/     &SwRect::GetBottomDistance,
/*N*/     &SwFrm::SetMinLeft,
/*N*/     &SwRect::OverStepLeft,
/*N*/ 
/*N*/     &SwRect::SetUpperRightCorner,
/*N*/     &SwFrm::MakeLeftPos,
/*N*/     &FirstMinusSecond,
/*N*/     &SecondMinusFirst,
/*N*/     &SwIncrement,
/*N*/     &SwDecrement,
/*N*/     &SwRect::SetTopAndHeight,
/*N*/     &SwRect::SetRightAndWidth
/*N*/ };
/*N*/ 
/*N*/ static SwRectFnCollection aBottomToTop = {
/*N*/     /* fnRectGet      */
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::BottomLeft,
/*N*/     &SwRect::_Size,
/*N*/     /* fnRectSet      */
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::_Height,
/*N*/ 
/*N*/     &SwRect::AddBottom,
/*N*/     &SwRect::SubTop,
/*N*/     &SwRect::SubLeft,
/*N*/     &SwRect::AddRight,
/*N*/     &SwRect::AddWidth,
/*N*/     &SwRect::AddHeight,
/*N*/ 
/*N*/     &SwRect::SetPosX,
/*N*/     &SwRect::SetPosY,
/*N*/ 
/*N*/     &SwFrm::GetBottomMargin,
/*N*/     &SwFrm::GetTopMargin,
/*N*/     &SwFrm::GetLeftMargin,
/*N*/     &SwFrm::GetRightMargin,
/*N*/     &SwFrm::SetLeftRightMargins,
/*N*/     &SwFrm::SetBottomTopMargins,
/*N*/     &SwFrm::GetPrtBottom,
/*N*/     &SwFrm::GetPrtTop,
/*N*/     &SwFrm::GetPrtLeft,
/*N*/     &SwFrm::GetPrtRight,
/*N*/     &SwRect::GetBottomDistance,
/*N*/     &SwRect::GetTopDistance,
/*N*/     &SwRect::GetLeftDistance,
/*N*/     &SwRect::GetRightDistance,
/*N*/     &SwFrm::SetMinTop,
/*N*/     &SwRect::OverStepTop,
/*N*/ 
/*N*/     &SwRect::SetLowerLeftCorner,
/*N*/     &SwFrm::MakeUpperPos,
/*N*/     &FirstMinusSecond,
/*N*/     &SecondMinusFirst,
/*N*/     &SwIncrement,
/*N*/     &SwDecrement,
/*N*/     &SwRect::SetLeftAndWidth,
/*N*/     &SwRect::SetBottomAndHeight
/*N*/ };
/*N*/ 
/*N*/ static SwRectFnCollection aVerticalRightToLeft = {
/*N*/     /* fnRectGet      */
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::_Width,
/*N*/     &SwRect::BottomRight,
/*N*/     &SwRect::SwappedSize,
/*N*/     /* fnRectSet      */
/*N*/     &SwRect::_Left,
/*N*/     &SwRect::_Right,
/*N*/     &SwRect::_Top,
/*N*/     &SwRect::_Bottom,
/*N*/     &SwRect::_Height,
/*N*/     &SwRect::_Width,
/*N*/ 
/*N*/     &SwRect::SubLeft,
/*N*/     &SwRect::AddRight,
/*N*/     &SwRect::SubTop,
/*N*/     &SwRect::AddBottom,
/*N*/     &SwRect::AddHeight,
/*N*/     &SwRect::AddWidth,
/*N*/ 
/*N*/     &SwRect::SetPosY,
/*N*/     &SwRect::SetPosX,
/*N*/ 
/*N*/     &SwFrm::GetLeftMargin,
/*N*/     &SwFrm::GetRightMargin,
/*N*/     &SwFrm::GetTopMargin,
/*N*/     &SwFrm::GetBottomMargin,
/*N*/     &SwFrm::SetTopBottomMargins,
/*N*/     &SwFrm::SetLeftRightMargins,
/*N*/     &SwFrm::GetPrtLeft,
/*N*/     &SwFrm::GetPrtRight,
/*N*/     &SwFrm::GetPrtBottom,
/*N*/     &SwFrm::GetPrtTop,
/*N*/     &SwRect::GetLeftDistance,
/*N*/     &SwRect::GetRightDistance,
/*N*/     &SwRect::GetBottomDistance,
/*N*/     &SwRect::GetTopDistance,
/*N*/     &SwFrm::SetMaxRight,
/*N*/     &SwRect::OverStepRight,
/*N*/ 
/*N*/     &SwRect::SetLowerLeftCorner,
/*N*/     &SwFrm::MakeRightPos,
/*N*/     &FirstMinusSecond,
/*N*/     &FirstMinusSecond,
/*N*/     &SwDecrement,
/*N*/     &SwIncrement,
/*N*/     &SwRect::SetBottomAndHeight,
/*N*/     &SwRect::SetLeftAndWidth
/*N*/ };
/*N*/ 
/*N*/ SwRectFn fnRectHori = &aHorizontal;
/*N*/ SwRectFn fnRectVert = &aVertical;
/*N*/ SwRectFn fnRectB2T = &aBottomToTop;
/*N*/ SwRectFn fnRectVL2R = &aVerticalRightToLeft;

/*N*/ #endif

/*N*/ #ifdef DBG_UTIL
/*N*/ USHORT SwFrm::nLastFrmId=0;
/*N*/ #endif


/*N*/ TYPEINIT1(SwFrm,SwClient);		//rtti fuer SwFrm
/*N*/ TYPEINIT1(SwCntntFrm,SwFrm);	//rtti fuer SwCntntFrm


/*N*/ void _FrmInit()
/*N*/ {
/*N*/ 	SwRootFrm::pVout = new SwLayVout();
/*N*/ 	SwCache *pNew = new SwCache( 100, 100
/*N*/ #ifdef DBG_UTIL
/*N*/ 	, "static SwBorderAttrs::pCache"
/*N*/ #endif
/*N*/ 	);
/*N*/ 	SwFrm::SetCache( pNew );
/*N*/ }



/*N*/ void _FrmFinit()
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/ 	// im Chache duerfen nur noch 0-Pointer stehen
/*N*/ 	for( USHORT n = SwFrm::GetCachePtr()->Count(); n; )
/*N*/ 		if( (*SwFrm::GetCachePtr())[ --n ] )
/*N*/ 		{
/*N*/ 			SwCacheObj* pObj = (*SwFrm::GetCachePtr())[ n ];
/*N*/ 			ASSERT( !pObj, "Wer hat sich nicht ausgetragen?")
/*N*/ 		}
/*N*/ #endif
/*N*/ 	delete SwRootFrm::pVout;
/*N*/ 	delete SwFrm::GetCachePtr();
/*N*/ }

/*************************************************************************
|*
|*	RootFrm::Alles was so zur CurrShell gehoert
|*
|*	Ersterstellung		MA 09. Sep. 98
|*	Letzte Aenderung	MA 18. Feb. 99
|*
|*************************************************************************/

/*N*/ typedef CurrShell* CurrShellPtr;
/*N*/ SV_DECL_PTRARR_SORT(SwCurrShells,CurrShellPtr,4,4)
/*N*/ SV_IMPL_PTRARR_SORT(SwCurrShells,CurrShellPtr)

/*N*/ CurrShell::CurrShell( ViewShell *pNew )
/*N*/ {
/*N*/ 	ASSERT( pNew, "0-Shell einsetzen?" );
/*N*/ 	pRoot = pNew->GetLayout();
/*N*/ 	if ( pRoot )
/*N*/ 	{
/*N*/ 		pPrev = pRoot->pCurrShell;
/*N*/ 		pRoot->pCurrShell = pNew;
/*N*/ 		pRoot->pCurrShells->Insert( this );
/*N*/ 	}
/*N*/ 	else
/*?*/ 		pPrev = 0;
/*N*/ }

/*N*/ CurrShell::~CurrShell()
/*N*/ {
/*N*/ 	if ( pRoot )
/*N*/ 	{
/*N*/ 		pRoot->pCurrShells->Remove( this );
/*N*/ 		if ( pPrev )
/*N*/ 			pRoot->pCurrShell = pPrev;
/*N*/ 		if ( !pRoot->pCurrShells->Count() && pRoot->pWaitingCurrShell )
/*N*/ 		{
/*?*/ 			pRoot->pCurrShell = pRoot->pWaitingCurrShell;
/*?*/ 			pRoot->pWaitingCurrShell = 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwRootFrm::DeRegisterShell( ViewShell *pSh )
/*N*/ {
/*N*/ 	//Wenn moeglich irgendeine Shell aktivieren
/*N*/ 	if ( pCurrShell == pSh )
/*N*/ 		pCurrShell = pSh->GetNext() != pSh ? (ViewShell*)pSh->GetNext() : 0;
/*N*/ 
/*N*/ 	//Das hat sich eruebrigt
/*N*/ 	if ( pWaitingCurrShell == pSh )
/*?*/ 		pWaitingCurrShell = 0;
/*N*/ 
/*N*/ 	//Referenzen entfernen.
/*N*/ 	for ( USHORT i = 0; i < pCurrShells->Count(); ++i )
/*N*/ 	{
/*?*/ 		CurrShell *pC = (*pCurrShells)[i];
/*?*/ 		if (pC->pPrev == pSh)
/*?*/ 			pC->pPrev = 0;
/*N*/ 	}
/*N*/ }

/*N*/ void InitCurrShells( SwRootFrm *pRoot )
/*N*/ {
/*N*/ 	pRoot->pCurrShells = new SwCurrShells;
/*N*/ }


/*************************************************************************
|*
|*	SwRootFrm::SwRootFrm()
|*
|*	Beschreibung:
|* 		Der RootFrm laesst sich grundsaetzlich vom Dokument ein eigenes
|* 		FrmFmt geben. Dieses loescht er dann selbst im DTor.
|* 		Das eigene FrmFmt wird vom uebergebenen Format abgeleitet.
|*	Ersterstellung		SS 05-Apr-1991
|*	Letzte Aenderung	MA 12. Dec. 94
|*
|*************************************************************************/


/*N*/ SwRootFrm::SwRootFrm( SwFrmFmt *pFmt, ViewShell * pSh ) :
/*N*/ 	SwLayoutFrm( pFmt->GetDoc()->MakeFrmFmt(
/*N*/ 		XubString( "Root", RTL_TEXTENCODING_MS_1252 ), pFmt ) ),
/*N*/ 	pTurbo( 0 ),
/*N*/ 	pLastPage( 0 ),
/*N*/ 	pCurrShell( pSh ),
/*N*/ 	pWaitingCurrShell( 0 ),
/*N*/ 	pDestroy( 0 ),
/*N*/ 	nPhyPageNums( 0 ),
/*N*/ 	pDrawPage( 0 ),
/*N*/ 	nBrowseWidth( MM50*4 )	//2cm Minimum
/*N*/ #ifdef ACCESSIBLE_LAYOUT
/*N*/ 	,nAccessibleShells( 0 )
/*N*/ #endif
/*N*/ {
/*N*/     nType = FRMC_ROOT;
/*N*/ 	bIdleFormat = bTurboAllowed = bAssertFlyPages = bIsNewLayout = TRUE;
/*N*/ 	bCheckSuperfluous = bBrowseWidthValid = FALSE;
/*N*/ 
/*N*/ 	InitCurrShells( this );
/*N*/ 
/*N*/ 	SwDoc *pDoc = pFmt->GetDoc();
/*N*/ 	const BOOL bOldIdle = pDoc->IsIdleTimerActive();
/*N*/ 	pDoc->StopIdleTimer();
/*N*/ 	pDoc->SetRootFrm( this );		//Fuer das Erzeugen der Flys durch MakeFrms()
/*N*/ 	bCallbackActionEnabled = FALSE;	//vor Verlassen auf TRUE setzen!
/*N*/ 
/*N*/ #ifndef VERTICAL_LAYOUT
/*N*/ #ifdef QUER
/*N*/ 	//StarWriter /QUER ? bitteschoen:
/*N*/ 	SetFixSize( pHeight );
/*N*/ #else
/*N*/ 	SetFixSize( pWidth );
/*N*/ #endif
/*N*/ #endif
/*N*/ 
/*N*/ 	SdrModel *pMd = pDoc->GetDrawModel();
/*N*/ 	if ( pMd )
/*N*/ 	{
/*N*/ 		pDrawPage = pMd->GetPage( 0 );
/*N*/ 		pDrawPage->SetSize( Frm().SSize() );
/*N*/ 	}
/*N*/ 
/*N*/ 	//Initialisierung des Layouts: Seiten erzeugen. Inhalt mit cntnt verbinden
/*N*/ 	//usw.
/*N*/ 	//Zuerst einiges initialiseren und den ersten Node besorgen (der wird
/*N*/ 	//fuer den PageDesc benoetigt).
/*N*/ 
/*N*/ 	SwNodeIndex aIndex( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
/*N*/ 	SwCntntNode *pNode = pDoc->GetNodes().GoNextSection( &aIndex, TRUE, FALSE );
/*N*/ 	SwTableNode *pTblNd= pNode->FindTableNode();
/*N*/ 
/*N*/ 	//PageDesc besorgen (entweder vom FrmFmt des ersten Node oder den
/*N*/ 	//initialen.)
/*N*/ 	SwPageDesc *pDesc = 0;
/*N*/ 	USHORT nPgNum = 1;
/*N*/ 
/*N*/ 	if ( pTblNd )
/*N*/ 	{
/*N*/ 		const SwFmtPageDesc &rDesc = pTblNd->GetTable().GetFrmFmt()->GetPageDesc();
/*N*/ 		pDesc = (SwPageDesc*)rDesc.GetPageDesc();
/*N*/ 		//#19104# Seitennummeroffset beruecksictigen!!
/*N*/ 		bIsVirtPageNum = 0 != ( nPgNum = rDesc.GetNumOffset() );
/*N*/ 	}
/*N*/ 	else if ( pNode )
/*N*/ 	{
/*N*/ 		const SwFmtPageDesc &rDesc = pNode->GetSwAttrSet().GetPageDesc();
/*N*/ 		pDesc = (SwPageDesc*)rDesc.GetPageDesc();
/*N*/ 		//#19104# Seitennummeroffset beruecksictigen!!
/*N*/ 		bIsVirtPageNum = 0 != ( nPgNum = rDesc.GetNumOffset() );
/*N*/ 	}
/*N*/ 	else
/*?*/ 		bIsVirtPageNum = FALSE;
/*N*/ 	if ( !pDesc )
/*N*/ 		pDesc = (SwPageDesc*)&pDoc->GetPageDesc( 0 );
/*N*/ 	const BOOL bOdd = !nPgNum || 0 != ( nPgNum % 2 );
/*N*/ 
/*N*/ 	//Eine Seite erzeugen und in das Layout stellen
/*N*/ 	SwPageFrm *pPage = ::binfilter::InsertNewPage( *pDesc, this, bOdd, FALSE, FALSE, 0 );
/*N*/ 
/*N*/ 	//Erstes Blatt im Bodytext-Bereich suchen.
/*N*/ 	SwLayoutFrm *pLay = pPage->FindBodyCont();
/*N*/ 	while( pLay->Lower() )
/*N*/ 		pLay = (SwLayoutFrm*)pLay->Lower();
/*N*/ 
/*N*/ 	SwNodeIndex aTmp( *pDoc->GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
/*N*/ 	::binfilter::_InsertCnt( pLay, pDoc, aTmp.GetIndex(), TRUE );
/*N*/ 	//Noch nicht ersetzte Master aus der Liste entfernen.
/*N*/ 	RemoveMasterObjs( pDrawPage );
/*N*/ 	if( pDoc->IsGlobalDoc() )
/*N*/ 		pDoc->UpdateRefFlds( NULL );
/*N*/ 	if ( bOldIdle )
/*N*/ 		pDoc->StartIdleTimer();
/*N*/ 	bCallbackActionEnabled = TRUE;
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::~SwRootFrm()
|*
|*	Ersterstellung		SS 05-Apr-1991
|*	Letzte Aenderung	MA 12. Dec. 94
|*
|*************************************************************************/



/*N*/ SwRootFrm::~SwRootFrm()
/*N*/ {
/*N*/ 	bTurboAllowed = FALSE;
/*N*/ 	pTurbo = 0;
/*N*/ 	((SwFrmFmt*)pRegisteredIn)->GetDoc()->DelFrmFmt( (SwFrmFmt*)pRegisteredIn );
/*N*/ 	delete pDestroy;
/*N*/ 
/*N*/ 	//Referenzen entfernen.
/*N*/ 	for ( USHORT i = 0; i < pCurrShells->Count(); ++i )
/*?*/ 		(*pCurrShells)[i]->pRoot = 0;
/*N*/ 
/*N*/ 	delete pCurrShells;
/*N*/ 
/*N*/ #ifdef ACCESSIBLE_LAYOUT
/*N*/ 	ASSERT( 0==nAccessibleShells, "Some accessible shells are left" );
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::SetFixSize()
|*
|*	Ersterstellung		MA 23. Jul. 92
|*	Letzte Aenderung	MA 11. Mar. 93
|*
|*************************************************************************/

/*N*/ #ifndef VERTICAL_LAYOUT

/*?*/ void SwRootFrm::SetFixSize( SzPtr pNew )
/*?*/ {
/*?*/ 	if ( pNew == pHeight )
/*?*/ 	{
/*?*/ 		GetFmt()->SetAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );
/*?*/ 		bVarHeight = bFixWidth = FALSE;
/*?*/ 		bFixHeight = TRUE;
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/ 		GetFmt()->SetAttr( SwFmtFillOrder( ATT_TOP_DOWN ) );
/*?*/ 		bVarHeight = bFixWidth = TRUE;
/*?*/ 		bFixHeight = FALSE;
/*?*/ 	}
/*?*/ }

/*N*/ #endif

/*************************************************************************
|*
|*	SwRootFrm::RemoveMasterObjs()
|*
|*	Ersterstellung		MA 19.10.95
|*	Letzte Aenderung	MA 19.10.95
|*
|*************************************************************************/


/*N*/ void SwRootFrm::RemoveMasterObjs( SdrPage *pPg )
/*N*/ {
/*N*/ 	//Alle Masterobjekte aus der Page entfernen. Nicht loeschen!!
/*N*/ 	for( ULONG i = pPg ? pPg->GetObjCount() : 0; i; )
/*N*/ 	{
/*N*/ 		SdrObject* pObj = pPg->GetObj( --i );
/*N*/ 		if( pObj->ISA(SwFlyDrawObj ) )
/*N*/ 			pPg->RemoveObject( i );
/*N*/ 	}
/*N*/ }




}
