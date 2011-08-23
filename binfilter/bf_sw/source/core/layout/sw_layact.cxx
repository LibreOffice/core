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

#include <time.h>
#include "pagefrm.hxx"

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "doc.hxx"
#include "viewimp.hxx"
#include "crsrsh.hxx"
#include "dflyobj.hxx"
#include "frmtool.hxx"
#include "dcontact.hxx"
#include "frmfmt.hxx"
#include "swregion.hxx"
#include "viewopt.hxx"  // OnlineSpelling ueber Internal-TabPage testen.
#include "pam.hxx"		// OnlineSpelling wg. der aktuellen Cursorposition
#include "dbg_lay.hxx"
#include "layouter.hxx" // LoopControlling

#include <ftnidx.hxx>

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen wg. Application
#include <vcl/svapp.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <bf_svx/brshitem.hxx>
#endif

#define _SVSTDARR_BOOLS

#define _LAYACT_CXX
#include "layact.hxx"

#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX //autogen
#include <bf_sfx2/progress.hxx>
#endif

#include "tabfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "flyfrms.hxx"
#include "frmsh.hxx"
#include "mdiexp.hxx"
#include "fmtornt.hxx"
#include "sectfrm.hxx"
#ifndef _ACMPLWRD_HXX
#include <acmplwrd.hxx>
#endif
namespace binfilter {

//#pragma optimize("ity",on)

/*************************************************************************
|*
|*	SwLayAction Statisches Geraffel
|*
|*	Ersterstellung		MA 22. Dec. 93
|*	Letzte Aenderung	MA 22. Dec. 93
|*
|*************************************************************************/

/*N*/ #define IS_FLYS (pPage->GetSortedObjs())
/*N*/ #define IS_INVAFLY (pPage->IsInvalidFly())


//Sparen von Schreibarbeit um den Zugriff auf zerstoerte Seiten zu vermeiden.
/*N*/ #ifdef DBG_UTIL

/*N*/ static void BreakPoint()
/*N*/ {
/*N*/ 	return;
/*N*/ }

/*N*/ #define CHECKPAGE \
/*N*/ 			{	if ( IsAgain() ) \
/*N*/ 				{	BreakPoint(); \
/*N*/ 					return; \
/*N*/ 				} \
/*N*/ 			}
/*N*/
/*N*/ #define XCHECKPAGE \
/*N*/ 			{	if ( IsAgain() ) \
/*N*/ 				{	BreakPoint(); \
/*N*/ 					if( bNoLoop ) \
/*N*/ 						pDoc->GetLayouter()->EndLoopControl(); \
/*N*/ 					return; \
/*N*/ 				} \
/*N*/ 			}
/*N*/ #else
/*?*/ #define CHECKPAGE \
/*?*/ 			{	if ( IsAgain() ) \
/*?*/ 					return; \
/*?*/ 			}
/*?*/
/*?*/ #define XCHECKPAGE \
/*?*/ 			{	if ( IsAgain() ) \
/*?*/ 				{ \
/*?*/ 					if( bNoLoop ) \
/*?*/ 						pDoc->GetLayouter()->EndLoopControl(); \
/*?*/ 					return; \
/*?*/ 				} \
/*?*/ 			}
/*N*/ #endif

/*N*/ #define RESCHEDULE \
/*N*/ 	{ \
/*N*/ 		if ( IsReschedule() )  \
/*N*/         { \
/*N*/             if (pProgress) pProgress->Reschedule(); \
/*N*/ 			::binfilter::RescheduleProgress( pImp->GetShell()->GetDoc()->GetDocShell() ); \
/*N*/         } \
/*N*/ 	}

/*N*/ inline ULONG Ticks()
/*N*/ {
/*N*/ 	return 1000 * clock() / CLOCKS_PER_SEC;
/*N*/ }

/*N*/ void SwLayAction::CheckWaitCrsr()
/*N*/ {
/*N*/ 	RESCHEDULE
/*N*/ }

/*************************************************************************
|*
|*	SwLayAction::CheckIdleEnd()
|*
|*	Ersterstellung		MA 12. Aug. 94
|*	Letzte Aenderung	MA 24. Jun. 96
|*
|*************************************************************************/
//Ist es wirklich schon soweit...
/*N*/ inline void SwLayAction::CheckIdleEnd()
/*N*/ {
/*N*/ 	if ( !IsInput() )
/*N*/         bInput = GetInputType() && Application::AnyInput( GetInputType() );
/*N*/ }

/*************************************************************************
|*
|*	SwLayAction::SetStatBar()
|*
|*	Ersterstellung		MA 10. Aug. 94
|*	Letzte Aenderung	MA 06. Aug. 95
|*
|*************************************************************************/
/*N*/ void SwLayAction::SetStatBar( BOOL bNew )
/*N*/ {
/*N*/ 	if ( bNew )
/*N*/ 	{
/*N*/ 		nEndPage = pRoot->GetPageNum();
/*N*/ 		nEndPage += nEndPage * 10 / 100;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nEndPage = USHRT_MAX;
/*N*/ }

/*************************************************************************
|*
|*	SwLayAction::PaintCntnt()
|*
|* 	Beschreibung		Je nach Typ wird der Cntnt entsprechend seinen
|* 		Veraenderungen ausgegeben bzw. wird die auszugebende Flaeche in der
|* 		Region eingetragen.
|* 		PaintCntnt:  fuellt die Region,
|*	Ersterstellung		BP 19. Jan. 92
|*	Letzte Aenderung	MA 10. Sep. 96
|*
|*************************************************************************/
/*N*/ BOOL SwLayAction::PaintWithoutFlys( const SwRect &rRect, const SwCntntFrm *pCnt,
/*N*/ 									const SwPageFrm *pPage )
/*N*/ {
/*N*/ 	SwRegionRects aTmp( rRect );
/*N*/ 	const SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
/*N*/ 	const SwFlyFrm *pSelfFly = pCnt->FindFlyFrm();
/*N*/
        USHORT i=0;
/*N*/ 	for ( i = 0; i < rObjs.Count() && aTmp.Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pO = rObjs[i];
/*N*/ 		if ( !pO->IsWriterFlyFrame() )
/*N*/ 			continue;
/*N*/
/*N*/ 		SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/
/*N*/ 		if ( pFly == pSelfFly || !rRect.IsOver( pFly->Frm() ) )
/*N*/ 			continue;
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/
/*N*/ 	BOOL bPaint = FALSE;
/*N*/ 	const SwRect *pData = aTmp.GetData();
/*N*/ 	for ( i = 0; i < aTmp.Count(); ++pData, ++i )
/*N*/ 		bPaint |= pImp->GetShell()->AddPaintRect( *pData );
/*N*/ 	return bPaint;
/*N*/ }

/*N*/ inline BOOL SwLayAction::_PaintCntnt( const SwCntntFrm *pCntnt,
/*N*/ 									  const SwPageFrm *pPage,
/*N*/ 									  const SwRect &rRect )
/*N*/ {
/*N*/ 	if ( rRect.HasArea() )
/*N*/ 	{
/*N*/ 		if ( pPage->GetSortedObjs() )
/*N*/ 			return PaintWithoutFlys( rRect, pCntnt, pPage );
/*N*/ 		else
/*N*/ 			return pImp->GetShell()->AddPaintRect( rRect );
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void SwLayAction::PaintCntnt( const SwCntntFrm *pCnt,
/*N*/ 							  const SwPageFrm *pPage,
/*N*/                               const SwRect &rOldRect,
/*N*/                               long nOldBottom )
/*N*/ {
/*N*/     SWRECTFN( pCnt )
/*N*/
/*N*/     if ( pCnt->IsCompletePaint() || !pCnt->IsTxtFrm() )
/*N*/ 	{
/*N*/ 		SwRect aPaint( pCnt->PaintArea() );
/*N*/         // OD 06.11.2002 #104171#,#103931# - paint of old area no longer needed.
/*N*/         //if( rOldRect.HasArea() )
/*N*/         //    aPaint.Union( rOldRect );
/*N*/ 		if ( !_PaintCntnt( pCnt, pPage, aPaint ) )
/*N*/ 			pCnt->ResetCompletePaint();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         // paint the area between printing bottom and frame bottom and
/*N*/         // the area left and right beside the frame, if its height changed.
/*N*/         long nOldHeight = (rOldRect.*fnRect->fnGetHeight)();
/*N*/         long nNewHeight = (pCnt->Frm().*fnRect->fnGetHeight)();
/*N*/         const bool bHeightDiff = nOldHeight != nNewHeight;
/*N*/         if( bHeightDiff )
/*N*/         {
/*N*/             // OD 05.11.2002 #94454# - consider whole potential paint area.
/*N*/             //SwRect aDrawRect( pCnt->UnionFrm( TRUE ) );
/*N*/             SwRect aDrawRect( pCnt->PaintArea() );
/*N*/             if( nOldHeight > nNewHeight )
/*N*/                 nOldBottom = (pCnt->*fnRect->fnGetPrtBottom)();
/*N*/             (aDrawRect.*fnRect->fnSetTop)( nOldBottom );
/*N*/             _PaintCntnt( pCnt, pPage, aDrawRect );
/*N*/         }
/*N*/         // paint content area
/*N*/         SwRect aPaintRect = static_cast<SwTxtFrm*>(const_cast<SwCntntFrm*>(pCnt))->Paint();
/*N*/         _PaintCntnt( pCnt, pPage, aPaintRect );
/*N*/ 	}
/*N*/
/*N*/ 	if ( pCnt->IsRetouche() && !pCnt->GetNext() )
/*N*/ 	{
/*N*/ 		const SwFrm *pTmp = pCnt;
/*N*/ 		if( pCnt->IsInSct() )
/*N*/ 		{
/*N*/ 			const SwSectionFrm* pSct = pCnt->FindSctFrm();
/*N*/ 			if( pSct->IsRetouche() && !pSct->GetNext() )
/*N*/ 				pTmp = pSct;
/*N*/ 		}
/*N*/ 		SwRect aRect( pTmp->GetUpper()->PaintArea() );
/*N*/         (aRect.*fnRect->fnSetTop)( (pTmp->*fnRect->fnGetPrtBottom)() );
/*N*/ 		if ( !_PaintCntnt( pCnt, pPage, aRect ) )
/*N*/ 			pCnt->ResetRetouche();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwLayAction::_AddScrollRect()
|*
|*	Ersterstellung		MA 04. Mar. 94
|*	Letzte Aenderung	MA 04. Mar. 94
|*
|*************************************************************************/
/*N*/ BOOL MA_FASTCALL lcl_IsOverObj( const SwFrm *pFrm, const SwPageFrm *pPage,
/*N*/ 					   const SwRect &rRect1, const SwRect &rRect2,
/*N*/ 					   const SwLayoutFrm *pLay )
/*N*/ {
/*N*/ 	const SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
/*N*/ 	const SwFlyFrm *pSelfFly = pFrm->FindFlyFrm();
/*N*/ 	const BOOL bInCnt = pSelfFly && pSelfFly->IsFlyInCntFrm() ? TRUE : FALSE;
/*N*/
/*N*/ 	for ( USHORT j = 0; j < rObjs.Count(); ++j )
/*N*/ 	{
/*N*/ 		const SdrObject 		*pObj = rObjs[j];
/*N*/ 		const SwRect aRect( pObj->GetBoundRect() );
/*N*/ 		if ( !rRect1.IsOver( aRect ) && !rRect2.IsOver( aRect ) )
/*N*/ 			continue;		//Keine Ueberlappung, der naechste.
/*N*/
/*N*/ 		const SwVirtFlyDrawObj *pFlyObj = pObj->IsWriterFlyFrame() ?
/*N*/ 												(SwVirtFlyDrawObj*)pObj : 0;
/*N*/ 		const SwFlyFrm *pFly = pFlyObj ? pFlyObj->GetFlyFrm() : 0;
/*N*/
/*N*/ 		//Wenn der Rahmen innerhalb des LayFrm verankert ist, so darf er
/*N*/ 		//mitgescrollt werden, wenn er nicht seitlich aus dem Rechteck
/*N*/ 		//herausschaut.
/*N*/ 		if ( pLay && pFlyObj && pFlyObj->GetFlyFrm()->IsLowerOf( pLay ) )
/*N*/ 		{
/*?*/ 			 if ( pFly->Frm().Left() < rRect1.Left() ||
/*?*/ 				  pFly->Frm().Right()> rRect1.Right() )
/*?*/ 				return TRUE;
/*?*/ 			continue;
/*N*/ 		}
/*N*/
/*N*/ 		if ( !pSelfFly )	//Nur wenn der Frm in einem Fly steht kann
/*N*/ 			return TRUE;	//es Einschraenkungen geben.
/*N*/
/*?*/ 		if ( !pFlyObj )		//Keine Einschraenkung fuer Zeichenobjekte.
/*?*/ 			return TRUE;
/*?*/
/*?*/ 		if ( pFly != pSelfFly )
/*?*/ 		{
/*?*/ 			//Flys unter dem eigenen nur dann abziehen, wenn sie innerhalb des
/*?*/ 			//eigenen stehen.
/*?*/ 			//Fuer inhaltsgebundene Flys alle Flys abziehen fuer die gilt, dass
/*?*/ 			//pSelfFly nicht innerhalb von ihnen steht.
/*?*/ 			if ( bInCnt )
/*?*/ 			{
/*?*/ 				const SwFlyFrm *pTmp = pSelfFly->GetAnchor()->FindFlyFrm();
/*?*/ 				while ( pTmp )
/*?*/ 				{
/*?*/ 					if ( pTmp == pFly )
/*?*/ 						return FALSE;
/*?*/ 					else
/*?*/ 						pTmp = pTmp->GetAnchor()->FindFlyFrm();
/*?*/ 				}
/*?*/ 			} else if ( pObj->GetOrdNum() < pSelfFly->GetVirtDrawObj()->GetOrdNum() )
/*?*/ 			{
/*?*/ 				const SwFlyFrm *pTmp = pFly;
/*?*/ 				do
/*?*/ 				{	if ( pTmp == pSelfFly )
/*?*/ 						return TRUE;
/*?*/ 					else
/*?*/ 						pTmp = pTmp->GetAnchor()->FindFlyFrm();
/*?*/ 				} while ( pTmp );
/*?*/ 			} else
/*?*/ 				return TRUE;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void SwLayAction::_AddScrollRect( const SwCntntFrm *pCntnt,
/*N*/ 								  const SwPageFrm *pPage,
/*N*/ 								  const SwTwips nOfst,
/*N*/ 								  const SwTwips nOldBottom )
/*N*/ {
/*N*/ 	FASTBOOL bScroll = TRUE;
/*N*/ 	SwRect aPaintRect( pCntnt->PaintArea() );
/*N*/     SWRECTFN( pCntnt )
/*N*/
/*N*/ 	//Wenn altes oder neues Rechteck mit einem Fly ueberlappen, in dem der
/*N*/ 	//Cntnt nicht selbst steht, so ist nichts mit Scrollen.
/*N*/ 	if ( pPage->GetSortedObjs() )
/*N*/ 	{
/*N*/ 		SwRect aRect( aPaintRect );
/*N*/         if( bVert )
/*?*/             aPaintRect.Pos().X() += nOfst;
/*N*/         else
/*N*/ 		aPaintRect.Pos().Y() -= nOfst;
/*N*/ 		if ( ::binfilter::lcl_IsOverObj( pCntnt, pPage, aPaintRect, aRect, 0 ) )
/*N*/ 			bScroll = FALSE;
/*N*/         if( bVert )
/*?*/             aPaintRect.Pos().X() -= nOfst;
/*N*/         else
/*N*/ 		aPaintRect.Pos().Y() += nOfst;
/*N*/ 	}
/*N*/ 	if ( bScroll && pPage->GetFmt()->GetBackground().GetGraphicPos() != GPOS_NONE )
/*N*/ 		bScroll = FALSE;
/*N*/
    // OD 04.11.2002 #94454# - Don't intersect potential paint rectangle with
    // union of frame and printing area, because at scroll destination position
    // could be a frame that has filled up the potential paint area.
    //aPaintRect.Intersection( pCntnt->UnionFrm( TRUE ) );

/*N*/ 	if ( bScroll )
/*N*/ 	{
/*N*/ 		if( aPaintRect.HasArea() )
/*N*/ 			pImp->GetShell()->AddScrollRect( pCntnt, aPaintRect, nOfst );
/*N*/ 		if ( pCntnt->IsRetouche() && !pCntnt->GetNext() )
/*N*/ 		{
/*N*/ 			SwRect aRect( pCntnt->GetUpper()->PaintArea() );
/*N*/             (aRect.*fnRect->fnSetTop)( (pCntnt->*fnRect->fnGetPrtBottom)() );
/*N*/ 			if ( !pImp->GetShell()->AddPaintRect( aRect ) )
/*N*/ 				pCntnt->ResetRetouche();
/*N*/ 		}
/*N*/ 		pCntnt->ResetCompletePaint();
/*N*/ 	}
/*N*/ 	else if( aPaintRect.HasArea() )
/*N*/ 	{
/*N*/         if( bVert )
/*?*/             aPaintRect.Pos().X() += nOfst;
/*N*/         else
/*N*/ 		aPaintRect.Pos().Y() -= nOfst;
/*N*/ 		PaintCntnt( pCntnt, pPage, aPaintRect, nOldBottom );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwLayAction::SwLayAction()
|*
|*	Ersterstellung		MA 30. Oct. 92
|*	Letzte Aenderung	MA 09. Jun. 95
|*
|*************************************************************************/
/*N*/ SwLayAction::SwLayAction( SwRootFrm *pRt, SwViewImp *pI ) :
/*N*/ 	pRoot( pRt ),
/*N*/ 	pImp( pI ),
/*N*/ 	pOptTab( 0 ),
/*N*/ 	pWait( 0 ),
/*N*/ 	nPreInvaPage( USHRT_MAX ),
/*N*/ 	nCheckPageNum( USHRT_MAX ),
/*N*/ 	nStartTicks( Ticks() ),
/*N*/ 	nInputType( 0 ),
/*N*/ 	nEndPage( USHRT_MAX ),
/*N*/ 	pProgress(NULL)
/*N*/ {
/*N*/ 	bPaintExtraData = ::binfilter::IsExtraData( pImp->GetShell()->GetDoc() );
/*N*/ 	bPaint = bComplete = bWaitAllowed = bCheckPages = TRUE;
/*N*/ 	bInput = bAgain = bNextCycle = bCalcLayout = bIdle = bReschedule =
/*N*/ 	bUpdateExpFlds = bBrowseActionStop = bActionInProgress = FALSE;
/*N*/     // OD 14.04.2003 #106346# - init new flag <mbFormatCntntOnInterrupt>.
/*N*/     mbFormatCntntOnInterrupt = sal_False;
/*N*/
/*N*/     pImp->pLayAct = this;   //Anmelden
/*N*/ }

/*N*/ SwLayAction::~SwLayAction()
/*N*/ {
/*N*/ 	ASSERT( !pWait, "Wait object not destroyed" );
/*N*/ 	pImp->pLayAct = 0;		//Abmelden
/*N*/ }

/*************************************************************************
|*
|*	SwLayAction::Reset()
|*
|*	Ersterstellung		MA 11. Aug. 94
|*	Letzte Aenderung	MA 09. Jun. 95
|*
|*************************************************************************/
/*N*/ void SwLayAction::Reset()
/*N*/ {
/*N*/ 	pOptTab = 0;
/*N*/ 	nStartTicks = Ticks();
/*N*/ 	nInputType = 0;
/*N*/ 	nEndPage = nPreInvaPage = nCheckPageNum = USHRT_MAX;
/*N*/ 	bPaint = bComplete = bWaitAllowed = bCheckPages = TRUE;
/*N*/ 	bInput = bAgain = bNextCycle = bCalcLayout = bIdle = bReschedule =
/*N*/ 	bUpdateExpFlds = bBrowseActionStop = FALSE;
/*N*/ }

/*************************************************************************
|*
|*	SwLayAction::RemoveEmptyBrowserPages()
|*
|*	Ersterstellung		MA 10. Sep. 97
|*	Letzte Aenderung	MA 10. Sep. 97
|*
|*************************************************************************/

/*N*/ BOOL SwLayAction::RemoveEmptyBrowserPages()
/*N*/ {
/*N*/ 	//Beim umschalten vom normalen in den Browsermodus bleiben u.U. einige
/*N*/ 	//unangenehm lange stehen. Diese beseiten wir mal schnell.
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	if ( pRoot->GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/ 	{
/*N*/ 		SwPageFrm *pPage = (SwPageFrm*)pRoot->Lower();
/*N*/ 		do
/*N*/ 		{
/*N*/ 			if ( (pPage->GetSortedObjs() && pPage->GetSortedObjs()->Count()) ||
/*N*/ 				 pPage->ContainsCntnt() )
/*N*/ 				pPage = (SwPageFrm*)pPage->GetNext();
/*N*/ 			else
/*N*/ 			{
/*N*/ 				bRet = TRUE;
/*N*/ 				SwPageFrm *pDel = pPage;
/*N*/ 				pPage = (SwPageFrm*)pPage->GetNext();
/*N*/ 				pDel->Cut();
/*N*/ 				delete pDel;
/*N*/ 			}
/*N*/ 		} while ( pPage );
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*************************************************************************
|*
|*	SwLayAction::Action()
|*
|*	Ersterstellung		MA 10. Aug. 94
|*	Letzte Aenderung	MA 06. Aug. 95
|*
|*************************************************************************/
/*N*/ void SwLayAction::Action()
/*N*/ {
/*N*/ 	bActionInProgress = TRUE;
/*N*/ 	//TurboMode? Disqualifiziert fuer Idle-Format.
/*N*/ 	if ( IsPaint() && !IsIdle() && TurboAction() )
/*N*/ 	{
/*N*/ 		pRoot->ResetTurboFlag();
/*N*/ 		bActionInProgress = FALSE;
/*N*/ 		pRoot->DeleteEmptySct();
/*N*/ 		return;
/*N*/ 	}
/*N*/ 	else if ( pRoot->GetTurbo() )
/*N*/ 	{
/*N*/ 		pRoot->DisallowTurbo();
/*N*/ 		const SwFrm *pFrm = pRoot->GetTurbo();
/*N*/ 		pRoot->ResetTurbo();
/*N*/ 		pFrm->InvalidatePage();
/*N*/ 	}
/*N*/ 	pRoot->DisallowTurbo();
/*N*/
/*N*/ 	if ( IsCalcLayout() )
/*?*/ 		SetCheckPages( FALSE );
/*N*/
/*N*/ 	InternalAction();
/*N*/ 	bAgain |= RemoveEmptyBrowserPages();
/*N*/ 	while ( IsAgain() )
/*N*/ 	{
/*N*/ 		bAgain = bNextCycle = FALSE;
/*N*/ 		InternalAction();
/*N*/ 		bAgain |= RemoveEmptyBrowserPages();
/*N*/ 	}
/*N*/ 	pRoot->DeleteEmptySct();
/*N*/
/*N*/ 	//Turbo-Action ist auf jedenfall wieder erlaubt.
/*N*/ 	pRoot->ResetTurboFlag();
/*N*/ 	pRoot->ResetTurbo();
/*N*/
/*N*/ 	if ( IsInput() )
/*N*/ 		pImp->GetShell()->SetNoNextScroll();
/*N*/ 	SetCheckPages( TRUE );
/*N*/ 	bActionInProgress = FALSE;
/*N*/ }

/*N*/ SwPageFrm *SwLayAction::CheckFirstVisPage( SwPageFrm *pPage )
/*N*/ {
/*N*/ 	SwCntntFrm *pCnt = pPage->FindFirstBodyCntnt();
/*N*/ 	SwCntntFrm *pChk = pCnt;
/*N*/ 	BOOL bPageChgd = FALSE;
/*N*/ 	while ( pCnt && pCnt->IsFollow() )
/*?*/ 		pCnt = (SwCntntFrm*)pCnt->FindPrev();
/*N*/ 	if ( pCnt && pChk != pCnt )
/*?*/ 	{	bPageChgd = TRUE;
/*?*/ 		pPage = pCnt->FindPageFrm();
/*N*/ 	}
/*N*/
/*N*/ 	if ( pPage->GetFmt()->GetDoc()->GetFtnIdxs().Count() )
/*N*/ 	{
/*N*/ 		SwFtnContFrm *pCont = pPage->FindFtnCont();
/*N*/ 		if ( pCont )
/*N*/ 		{
/*N*/ 			pCnt = pCont->ContainsCntnt();
/*N*/ 			pChk = pCnt;
/*N*/ 			while ( pCnt && pCnt->IsFollow() )
/*?*/ 				pCnt = (SwCntntFrm*)pCnt->FindPrev();
/*N*/ 			if ( pCnt && pCnt != pChk )
/*N*/ 			{
/*?*/ 				if ( bPageChgd )
/*?*/ 				{
/*?*/ 					//Die 'oberste' Seite benutzten.
/*?*/ 					SwPageFrm *pTmp = pCnt->FindPageFrm();
/*?*/ 					if ( pPage->GetPhyPageNum() > pTmp->GetPhyPageNum() )
/*?*/ 						pPage = pTmp;
/*?*/ 				}
/*?*/ 				else
/*?*/ 					pPage = pCnt->FindPageFrm();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pPage;
/*N*/ }

/*N*/ void SwLayAction::InternalAction()
/*N*/ {
/*N*/ 	ASSERT( pRoot->Lower()->IsPageFrm(), ":-( Keine Seite unterhalb der Root.");
/*N*/
/*N*/ 	pRoot->Calc();
/*N*/
/*N*/ 	//Die erste ungueltige bzw. zu formatierende Seite ermitteln.
/*N*/ 	//Bei einer Complete-Action ist es die erste ungueltige; mithin ist die
/*N*/ 	//erste zu formatierende Seite diejenige Seite mit der Numemr eins.
/*N*/ 	//Bei einer Luegen-Formatierung ist die Nummer der erste Seite die Nummer
/*N*/ 	//der ersten Sichtbaren Seite.
/*N*/ 	SwPageFrm *pPage = IsComplete() ? (SwPageFrm*)pRoot->Lower() :
/*N*/ 				pImp->GetFirstVisPage();
/*N*/ 	if ( !pPage )
/*?*/ 		pPage = (SwPageFrm*)pRoot->Lower();
/*N*/
/*N*/ 	//Wenn ein "Erster-Fliess-Cntnt" innerhalb der der ersten sichtbaren Seite
/*N*/ 	//ein Follow ist, so schalten wir die Seite zurueck auf den Ur-Master dieses
/*N*/ 	//Cntnt's
/*N*/ 	if ( !IsComplete() )
/*N*/ 		pPage = CheckFirstVisPage( pPage );
/*N*/ 	USHORT nFirstPageNum = pPage->GetPhyPageNum();
/*N*/
/*N*/ 	while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
/*N*/ 		pPage = (SwPageFrm*)pPage->GetNext();
/*N*/
/*N*/ 	SwDoc* pDoc = pRoot->GetFmt()->GetDoc();
/*N*/ 	BOOL bNoLoop = pPage ? SwLayouter::StartLoopControl( pDoc, pPage ) : NULL;
/*N*/ 	USHORT nPercentPageNum = 0;
/*N*/ 	while ( (pPage && !IsInterrupt()) || nCheckPageNum != USHRT_MAX )
/*N*/ 	{
/*N*/ 		if ( !pPage && nCheckPageNum != USHRT_MAX &&
/*N*/ 			 (!pPage || pPage->GetPhyPageNum() >= nCheckPageNum) )
/*N*/ 		{
/*?*/ 			if ( !pPage || pPage->GetPhyPageNum() > nCheckPageNum )
/*?*/ 			{
/*?*/ 				SwPageFrm *pPg = (SwPageFrm*)pRoot->Lower();
/*?*/ 				while ( pPg && pPg->GetPhyPageNum() < nCheckPageNum )
/*?*/ 					pPg = (SwPageFrm*)pPg->GetNext();
/*?*/ 				if ( pPg )
/*?*/ 					pPage = pPg;
/*?*/ 				if ( !pPage )
/*?*/ 					break;
/*?*/ 			}
/*?*/ 			SwPageFrm *pTmp = pPage->GetPrev() ?
/*?*/ 										(SwPageFrm*)pPage->GetPrev() : pPage;
/*?*/ 			SetCheckPages( TRUE );
/*?*/ 			SwFrm::CheckPageDescs( pPage );
/*?*/ 			SetCheckPages( FALSE );
/*?*/ 			nCheckPageNum = USHRT_MAX;
/*?*/ 			pPage = pTmp;
/*?*/ 			continue;
/*N*/ 		}
/*N*/
/*N*/ #ifdef MA_DEBUG
/*?*/ 		static USHORT nStop = USHRT_MAX;
/*?*/ 		if ( pPage->GetPhyPageNum() == nStop )
/*?*/ 		{
/*?*/ 			int bla = 5;
/*?*/ 		}
/*?*/ 		Window *pWin = pImp->GetShell()->GetWin();
/*?*/ 		if ( pWin )
/*?*/ 		{
/*?*/ 			pWin->Push( PUSH_FILLCOLOR );
/*?*/ 			pWin->SetFillColor( COL_WHITE );
/*?*/ 			Point aOfst( pImp->GetShell()->VisArea().Pos() );
/*?*/ 			pWin->DrawRect( Rectangle( aOfst, Size( 2000, 1000 )));
/*?*/ 			pWin->DrawText( Point( 500, 500 ) + aOfst, pPage->GetPhyPageNum() );
/*?*/ 			pWin->Pop();
/*?*/ 		}
/*N*/ #endif
/*N*/ 		if ( nEndPage != USHRT_MAX && pPage->GetPhyPageNum() > nPercentPageNum )
/*N*/ 		{
/*?*/ 			nPercentPageNum = pPage->GetPhyPageNum();
/*?*/ 			::binfilter::SetProgressState( nPercentPageNum, pImp->GetShell()->GetDoc()->GetDocShell());
/*N*/ 		}
/*N*/ 		pOptTab = 0;
/*N*/ 			 //Kein ShortCut fuer Idle oder CalcLayout
/*N*/ 		if ( !IsIdle() && !IsComplete() && IsShortCut( pPage ) )
/*N*/ 		{
/*N*/ 			pRoot->DeleteEmptySct();
/*N*/ 			XCHECKPAGE;
/*N*/ 			if ( !IsInterrupt() &&
/*N*/ 				 (pRoot->IsSuperfluous() || pRoot->IsAssertFlyPages()) )
/*N*/ 			{
/*N*/ 				if ( pRoot->IsAssertFlyPages() )
/*N*/ 					pRoot->AssertFlyPages();
/*N*/ 				if ( pRoot->IsSuperfluous() )
/*N*/ 				{
/*N*/ 					BOOL bOld = IsAgain();
/*N*/ 					pRoot->RemoveSuperfluous();
/*N*/ 					bAgain = bOld;
/*N*/ 				}
/*N*/ 				if ( IsAgain() )
/*N*/ 				{
/*?*/ 					if( bNoLoop )
/*?*/ 						pDoc->GetLayouter()->EndLoopControl();
/*?*/ 					return;
/*N*/ 				}
/*N*/ 				pPage = (SwPageFrm*)pRoot->Lower();
/*N*/ 				while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
/*N*/ 					pPage = (SwPageFrm*)pPage->GetNext();
/*N*/ 				while ( pPage && pPage->GetNext() &&
/*N*/ 						pPage->GetPhyPageNum() < nFirstPageNum )
/*N*/ 					pPage = (SwPageFrm*)pPage->GetNext();
/*N*/ 				continue;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pRoot->DeleteEmptySct();
/*N*/ 			XCHECKPAGE;
/*N*/ 			//Erst das Layout der Seite formatieren. Erst wenn das Layout
/*N*/ 			//stabil ist lohnt sich die Inhaltsformatiertung.
/*N*/ 			//Wenn durch die Inhaltsformatierung das Layout wieder ungueltig
/*N*/ 			//wird, so wird die Inhaltsformatierung abgebrochen und das
/*N*/ 			//Layout wird wieder stabilisiert.
/*N*/ 			//Keine Angst: im Normafall kommt es nicht zu Oszillationen.
/*N*/ 			//Das Spielchen spielen wir zweimal. erst fuer die Flys, dann
/*N*/ 			//fuer den Rest.
/*N*/ 			//Die Flys haben Vorrang, d.h. wenn sich an den Flys waehrend der
/*N*/ 			//Formatierung des Bodys etwas aendert wird die Body-Formatierung
/*N*/ 			//unterbrochen und wieder bei den Flys angefangen.
/*N*/
/*N*/ 			while ( !IsInterrupt() && !IsNextCycle() &&
/*N*/ 					((IS_FLYS && IS_INVAFLY) || pPage->IsInvalid()) )
/*N*/ 			{
/*N*/                 USHORT nLoop = 0; // Loop control
/*N*/ 				while ( !IsInterrupt() && IS_INVAFLY && IS_FLYS )
/*N*/ 				{
/*N*/ 					XCHECKPAGE;
/*N*/ 					if ( pPage->IsInvalidFlyLayout() )
/*N*/ 					{
/*N*/ 						pPage->ValidateFlyLayout();
/*N*/ 						FormatFlyLayout( pPage );
/*N*/ 						XCHECKPAGE;
/*N*/ 					}
/*N*/ 					if ( pPage->IsInvalidFlyCntnt() && IS_FLYS )
/*N*/ 					{
/*N*/ 						pPage->ValidateFlyCntnt();
/*N*/                         // More than 20 calls of this function are enough,
/*N*/                         // then we disallow the shrinking of fly frames.
/*N*/                         if ( !FormatFlyCntnt( pPage, nLoop > 20 ) )
/*N*/ 						{	XCHECKPAGE;
/*N*/ 							pPage->InvalidateFlyCntnt();
/*N*/ 						}
/*N*/ 					}
/*N*/                     ++nLoop; // Loop count
/*N*/ 				}
/*N*/ 				if ( !IS_FLYS )
/*N*/ 				{
/*N*/ 					//Wenn keine Flys (mehr) da sind, sind die Flags
/*N*/ 					//mehr als fluessig.
/*N*/ 					pPage->ValidateFlyLayout();
/*N*/ 					pPage->ValidateFlyCntnt();
/*N*/ 				}
/*N*/ 				while ( !IsInterrupt() && !IsNextCycle() && pPage->IsInvalid() &&
/*N*/ 						(!IS_FLYS || (IS_FLYS && !IS_INVAFLY)) )
/*N*/ 				{
/*N*/ 					PROTOCOL( pPage, PROT_FILE_INIT, 0, 0)
/*N*/ 					XCHECKPAGE;
/*N*/ 					while ( !IsNextCycle() && pPage->IsInvalidLayout() )
/*N*/ 					{
/*N*/ 						pPage->ValidateLayout();
/*N*/ 						FormatLayout( pPage );
/*N*/ 						XCHECKPAGE;
/*N*/ 					}
/*N*/ 					if ( !IsNextCycle() && pPage->IsInvalidCntnt() &&
/*N*/ 						 (!IS_FLYS || (IS_FLYS && !IS_INVAFLY)) )
/*N*/ 					{
/*N*/ 						pPage->ValidateFlyInCnt();
/*N*/ 						pPage->ValidateCntnt();
/*N*/ 						if ( !FormatCntnt( pPage ) )
/*N*/ 						{
/*N*/ 							XCHECKPAGE;
/*N*/ 							pPage->InvalidateCntnt();
/*N*/ 							pPage->InvalidateFlyInCnt();
/*N*/ 							if ( IsBrowseActionStop() )
/*?*/ 								bInput = TRUE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					if( bNoLoop )
/*N*/ 						pDoc->GetLayouter()->LoopControl( pPage, LOOP_PAGE );
/*N*/
/*N*/ 				}
/*N*/ 			}
/*N*/ 			//Eine vorige Seite kann wieder invalid sein.
/*N*/ 			XCHECKPAGE;
/*N*/ 			if ( !IS_FLYS )
/*N*/ 			{
/*N*/ 				//Wenn keine Flys (mehr) da sind, sind die Flags
/*N*/ 				//mehr als fluessig.
/*N*/ 				pPage->ValidateFlyLayout();
/*N*/ 				pPage->ValidateFlyCntnt();
/*N*/ 			}
/*N*/ 			if ( !IsInterrupt() )
/*N*/ 			{
/*N*/ 				SetNextCycle( FALSE );
/*N*/
/*N*/ 				if ( nPreInvaPage != USHRT_MAX )
/*N*/ 				{
/*N*/ 					if( !IsComplete() && nPreInvaPage + 2 < nFirstPageNum )
/*N*/ 					{
/*?*/ 						pImp->SetFirstVisPageInvalid();
/*?*/ 						SwPageFrm *pTmpPage = pImp->GetFirstVisPage();
/*?*/ 						nFirstPageNum = pTmpPage->GetPhyPageNum();
/*?*/ 						if( nPreInvaPage < nFirstPageNum )
/*?*/ 						{
/*?*/ 							nPreInvaPage = nFirstPageNum;
/*?*/ 							pPage = pTmpPage;
/*?*/ 						}
/*N*/ 					}
/*N*/ 					while ( pPage->GetPrev() && pPage->GetPhyPageNum() > nPreInvaPage )
/*N*/ 						pPage = (SwPageFrm*)pPage->GetPrev();
/*N*/ 					nPreInvaPage = USHRT_MAX;
/*N*/ 				}
/*N*/
/*N*/ 				//Ist eine Vorseite invalid?
/*N*/ 				while ( pPage->GetPrev() &&
/*N*/ 						( ((SwPageFrm*)pPage->GetPrev())->IsInvalid() ||
/*N*/ 						  ( ((SwPageFrm*)pPage->GetPrev())->GetSortedObjs() &&
/*N*/ 							((SwPageFrm*)pPage->GetPrev())->IsInvalidFly())) &&
/*N*/ 						(((SwPageFrm*)pPage->GetPrev())->GetPhyPageNum() >=
/*N*/ 							nFirstPageNum) )
/*N*/ 				{
/*N*/ 					pPage = (SwPageFrm*)pPage->GetPrev();
/*N*/ 				}
/*N*/ 				//Weiter bis zur naechsten invaliden Seite.
/*N*/ 				while ( pPage && !pPage->IsInvalid() &&
/*N*/ 						(!IS_FLYS || (IS_FLYS && !IS_INVAFLY)) )
/*N*/ 				{
/*N*/ 					pPage = (SwPageFrm*)pPage->GetNext();
/*N*/ 				}
/*N*/ 				if( bNoLoop )
/*N*/ 					pDoc->GetLayouter()->LoopControl( pPage, LOOP_PAGE );
/*N*/ 			}
/*N*/             CheckIdleEnd();
/*N*/ 		}
/*N*/ 		if ( !pPage && !IsInterrupt() &&
/*N*/ 			 (pRoot->IsSuperfluous() || pRoot->IsAssertFlyPages()) )
/*N*/ 		{
/*N*/ 			if ( pRoot->IsAssertFlyPages() )
/*N*/ 				pRoot->AssertFlyPages();
/*N*/ 			if ( pRoot->IsSuperfluous() )
/*N*/ 			{
/*N*/ 				BOOL bOld = IsAgain();
/*N*/ 				pRoot->RemoveSuperfluous();
/*N*/ 				bAgain = bOld;
/*N*/ 			}
/*N*/ 			if ( IsAgain() )
/*N*/ 			{
/*?*/ 				if( bNoLoop )
/*?*/ 					pDoc->GetLayouter()->EndLoopControl();
/*?*/ 				return;
/*N*/ 			}
/*N*/ 			pPage = (SwPageFrm*)pRoot->Lower();
/*N*/ 			while ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
/*N*/ 				pPage = (SwPageFrm*)pPage->GetNext();
/*N*/ 			while ( pPage && pPage->GetNext() &&
/*N*/ 					pPage->GetPhyPageNum() < nFirstPageNum )
/*?*/ 				pPage = (SwPageFrm*)pPage->GetNext();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( IsInterrupt() && pPage )
/*N*/ 	{
/*N*/ 		//Wenn ein Input anliegt wollen wir keinen Inhalt mehr Formatieren,
/*N*/ 		//Das Layout muessen wir aber schon in Ordnung bringen.
/*N*/ 		//Andernfalls kann folgende Situation auftreten (Bug: 3244):
/*N*/ 		//Am Ende des Absatz der letzten Seite wird Text eingegeben, so das
/*N*/ 		//der Absatz einen Follow fuer die nachste Seite erzeugt, ausserdem
/*N*/ 		//wird gleich schnell weitergetippt - Es liegt waehrend der
/*N*/ 		//Verarbeitung ein Input an. Der Absatz auf der neuen Seite wurde
/*N*/ 		//bereits anformatiert, die neue Seite ist Formatiert und steht
/*N*/ 		//auf CompletePaint, hat sich aber noch nicht im Auszugebenden Bereich
/*N*/ 		//eingetragen. Es wird gepaintet, das CompletePaint der Seite wird
/*N*/ 		//zurueckgesetzt weil der neue Absatz sich bereits eingetragen hatte,
/*N*/ 		//aber die Raender der Seite werden nicht gepaintet. Naja, bei der
/*N*/ 		//zwangslaeufig auftretenden naechsten LayAction traegt sich die Seite
/*N*/ 		//nicht mehr ein, weil ihre (LayoutFrm-)Flags bereits zurueckgesetzt
/*N*/ 		//wurden -- Der Rand der Seite wird nie gepaintet.
/*N*/ 		SwPageFrm *pPg = pPage;
/*N*/ 		XCHECKPAGE;
/*N*/ 		const SwRect &rVis = pImp->GetShell()->VisArea();
/*N*/
/*N*/         while( pPg && pPg->Frm().Bottom() < rVis.Top() )
/*?*/ 			pPg = (SwPageFrm*)pPg->GetNext();
/*N*/         if( pPg != pPage )
/*?*/             pPg = pPg ? (SwPageFrm*)pPg->GetPrev() : pPage;
/*N*/
/*N*/         // OD 14.04.2003 #106346# - set flag for interrupt content formatting
/*N*/         mbFormatCntntOnInterrupt = IsInput() && !IsStopPrt();
/*N*/         long nBottom = rVis.Bottom();
/*N*/         while ( pPg && pPg->Frm().Top() < nBottom )
/*N*/ 		{
/*N*/             XCHECKPAGE;
/*N*/             // OD 14.04.2003 #106346# - special case: interrupt content formatting
/*N*/             while ( ( mbFormatCntntOnInterrupt &&
/*N*/                       pPg->IsInvalid() &&
/*N*/                       (!IS_FLYS || (IS_FLYS && !IS_INVAFLY))
/*N*/                     ) ||
/*N*/                     ( !mbFormatCntntOnInterrupt && pPg->IsInvalidLayout() )
/*N*/                   )
/*N*/ 		{
/*?*/ 			XCHECKPAGE;
/*?*/ 			while ( pPg->IsInvalidLayout() )
/*?*/ 			{
/*?*/ 				pPg->ValidateLayout();
/*?*/ 				FormatLayout( pPg );
/*?*/ 				XCHECKPAGE;
/*?*/ 			}
/*N*/                 if ( mbFormatCntntOnInterrupt &&
/*N*/                      pPg->IsInvalidCntnt() &&
/*N*/                      (!IS_FLYS || (IS_FLYS && !IS_INVAFLY))
/*N*/                    )
/*N*/                 {
/*N*/                     pPg->ValidateFlyInCnt();
/*N*/                     pPg->ValidateCntnt();
/*N*/                     if ( !FormatCntnt( pPg ) )
/*N*/                     {
/*N*/                         XCHECKPAGE;
/*N*/                         pPg->InvalidateCntnt();
/*N*/                         pPg->InvalidateFlyInCnt();
/*N*/                     }
/*N*/                 }
/*N*/             }
/*?*/ 			pPg = (SwPageFrm*)pPg->GetNext();
/*N*/ 		}
/*N*/         // OD 14.04.2003 #106346# - reset flag for special interrupt content formatting.
/*N*/         mbFormatCntntOnInterrupt = sal_False;
/*N*/ 	}
/*N*/ 	pOptTab = 0;
/*N*/ 	if( bNoLoop )
/*N*/ 		pDoc->GetLayouter()->EndLoopControl();
/*N*/ }
/*************************************************************************
|*
|*	SwLayAction::TurboAction(), _TurboAction()
|*
|*	Ersterstellung		MA 04. Dec. 92
|*	Letzte Aenderung	MA 15. Aug. 93
|*
|*************************************************************************/
/*N*/ BOOL SwLayAction::_TurboAction( const SwCntntFrm *pCnt )
/*N*/ {
/*N*/
/*N*/ 	const SwPageFrm *pPage = 0;
/*N*/ 	if ( !pCnt->IsValid() || pCnt->IsCompletePaint() || pCnt->IsRetouche() )
/*N*/ 	{
/*N*/ 		const SwRect aOldRect( pCnt->UnionFrm( TRUE ) );
/*N*/ 		const long	 nOldBottom = pCnt->Frm().Top() + pCnt->Prt().Bottom();
/*N*/ 		pCnt->Calc();
/*N*/ 		if ( pCnt->Frm().Bottom() < aOldRect.Bottom() )
/*N*/ 			pCnt->SetRetouche();
/*N*/
/*N*/ 		pPage = pCnt->FindPageFrm();
/*N*/ 		PaintCntnt( pCnt, pPage, aOldRect, nOldBottom );
/*N*/
/*N*/ 		if ( !pCnt->GetValidLineNumFlag() && pCnt->IsTxtFrm() )
/*N*/ 		{
/*N*/ 			const ULONG nAllLines = ((SwTxtFrm*)pCnt)->GetAllLines();
/*N*/ 			((SwTxtFrm*)pCnt)->RecalcAllLines();
/*N*/ 			if ( nAllLines != ((SwTxtFrm*)pCnt)->GetAllLines() )
/*N*/ 			{
/*N*/ 				if ( IsPaintExtraData() )
/*?*/ 					pImp->GetShell()->AddPaintRect( pCnt->Frm() );
/*N*/ 				//Damit die restlichen LineNums auf der Seite bereichnet werden
/*N*/ 				//und nicht hier abgebrochen wird.
/*N*/ 				//Das im RecalcAllLines zu erledigen waere teuer, weil dort
/*N*/ 				//auch in unnoetigen Faellen (normale Action) auch immer die
/*N*/ 				//Seite benachrichtigt werden muesste.
/*N*/ 				const SwCntntFrm *pNxt = pCnt->GetNextCntntFrm();
/*N*/ 				while ( pNxt &&
/*N*/ 						(pNxt->IsInTab() || pNxt->IsInDocBody() != pCnt->IsInDocBody()) )
/*N*/ 					pNxt = pNxt->GetNextCntntFrm();
/*N*/ 				if ( pNxt )
/*N*/ 					pNxt->InvalidatePage();
/*N*/ 			}
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/
/*N*/ 		if ( pPage->IsInvalidLayout() || (IS_FLYS && IS_INVAFLY) )
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ 	if ( !pPage )
/*?*/ 		pPage = pCnt->FindPageFrm();
/*N*/ 	//Die im Absatz verankerten Flys wollen auch beachtet werden.
/*N*/ 	if ( pPage->IsInvalidFlyInCnt() && pCnt->GetDrawObjs() )
/*N*/ 	{
/*N*/ 		const SwDrawObjs *pObjs = pCnt->GetDrawObjs();
/*N*/ 		for ( USHORT i = 0; i < pObjs->Count(); ++i )
/*N*/ 		{
/*N*/ 			SdrObject *pO = (*pObjs)[i];
/*N*/ 			if ( pO->IsWriterFlyFrame() )
/*N*/ 			{
/*N*/ 				SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 				if ( pFly->IsFlyInCntFrm() && ((SwFlyInCntFrm*)pFly)->IsInvalid() )
/*N*/ 				{
/*N*/ 					FormatFlyInCnt( (SwFlyInCntFrm*)pFly );
/*N*/ 					pObjs = pCnt->GetDrawObjs();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( pPage->IsInvalidCntnt() )
/*N*/ 		return FALSE;
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL SwLayAction::TurboAction()
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;
/*N*/
/*N*/ 	if ( pRoot->GetTurbo() )
/*N*/ 	{
/*N*/ 		if ( !_TurboAction( pRoot->GetTurbo() ) )
/*N*/ 		{
/*N*/ 			CheckIdleEnd();
/*N*/ 			bRet = FALSE;
/*N*/ 		}
/*N*/ 		pRoot->ResetTurbo();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bRet = FALSE;
/*N*/ 	return bRet;
/*N*/ }
/*************************************************************************
|*
|*	SwLayAction::IsShortCut()
|*
|*	Beschreibung:		Liefert ein True, wenn die Seite vollstaendig unter
|* 		oder rechts neben dem sichbaren Bereich liegt.
|* 		Es kann passieren, dass sich die Verhaeltnisse derart aendern, dass
|* 		die Verarbeitung (des Aufrufers!) mit der Vorgaengerseite der
|* 		uebergebenen Seite weitergefuehrt werden muss. Der Paramter wird also
|* 		ggf. veraendert!
|*		Fuer den BrowseMode kann auch dann der ShortCut aktiviert werden,
|*		wenn der ungueltige Inhalt der Seite unterhalb des sichbaren
|*		bereiches liegt.
|*	Ersterstellung		MA 30. Oct. 92
|*	Letzte Aenderung	MA 18. Jul. 96
|*
|*************************************************************************/
/*N*/ const SwFrm *lcl_FindFirstInvaLay( const SwFrm *pFrm, long nBottom )
/*N*/ {
/*N*/ 	ASSERT( pFrm->IsLayoutFrm(), "FindFirstInvaLay, no LayFrm" );
/*N*/
/*N*/ 	if ( !pFrm->IsValid() || pFrm->IsCompletePaint() &&
/*N*/ 		 pFrm->Frm().Top() < nBottom )
/*N*/ 		return pFrm;
/*N*/ 	pFrm = ((SwLayoutFrm*)pFrm)->Lower();
/*N*/ 	while ( pFrm )
/*N*/ 	{
/*N*/ 		if ( pFrm->IsLayoutFrm() )
/*N*/ 		{
/*N*/ 			if ( !pFrm->IsValid() || pFrm->IsCompletePaint() &&
/*N*/ 				 pFrm->Frm().Top() < nBottom )
/*N*/ 				return pFrm;
/*N*/ 			const SwFrm *pTmp;
/*N*/ 			if ( 0 != (pTmp = ::binfilter::lcl_FindFirstInvaLay( pFrm, nBottom )) )
/*N*/ 				return pTmp;
/*N*/ 		}
/*N*/ 		pFrm = pFrm->GetNext();
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ const SwFrm *lcl_FindFirstInvaCntnt( const SwLayoutFrm *pLay, long nBottom,
/*N*/ 									 const SwCntntFrm *pFirst )
/*N*/ {
/*N*/ 	const SwCntntFrm *pCnt = pFirst ? pFirst->GetNextCntntFrm() :
/*N*/ 									  pLay->ContainsCntnt();
/*N*/ 	while ( pCnt )
/*N*/ 	{
/*N*/ 		if ( !pCnt->IsValid() || pCnt->IsCompletePaint() )
/*N*/ 		{
/*N*/ 			if ( pCnt->Frm().Top() <= nBottom )
/*N*/ 				return pCnt;
/*N*/ 		}
/*N*/
/*N*/ 		if ( pCnt->GetDrawObjs() )
/*N*/ 		{
/*N*/ 			const SwDrawObjs &rObjs = *pCnt->GetDrawObjs();
/*N*/ 			for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/ 			{
/*N*/ 				const SdrObject *pO = rObjs[i];
/*N*/ 				if ( pO->IsWriterFlyFrame() )
/*N*/ 				{
/*N*/ 					const SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 					if ( pFly->IsFlyInCntFrm() )
/*N*/ 					{
/*N*/ 						if ( ((SwFlyInCntFrm*)pFly)->IsInvalid() ||
/*N*/ 							 pFly->IsCompletePaint() )
/*N*/ 						{
/*N*/ 							if ( pFly->Frm().Top() <= nBottom )
/*N*/ 								return pFly;
/*N*/ 						}
/*N*/ 						const SwFrm *pFrm = lcl_FindFirstInvaCntnt( pFly, nBottom, 0 );
/*N*/ 						if ( pFrm && pFrm->Frm().Bottom() <= nBottom )
/*N*/ 							return pFrm;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( pCnt->Frm().Top() > nBottom && !pCnt->IsInTab() )
/*N*/ 			return 0;
/*N*/ 		pCnt = pCnt->GetNextCntntFrm();
/*N*/ 		if ( !pLay->IsAnLower( pCnt ) )
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ const SwFrm *lcl_FindFirstInvaFly( const SwPageFrm *pPage, long nBottom )
/*N*/ {
/*N*/ 	ASSERT( pPage->GetSortedObjs(), "FindFirstInvaFly, no Flys" )
/*N*/
/*N*/ 	for ( USHORT i = 0; i < pPage->GetSortedObjs()->Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pO = (*pPage->GetSortedObjs())[i];
/*N*/ 		if ( pO->IsWriterFlyFrame() )
/*N*/ 		{
/*N*/ 			const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 			if ( pFly->Frm().Top() <= nBottom )
/*N*/ 			{
/*N*/ 				if ( pFly->IsInvalid() || pFly->IsCompletePaint() )
/*N*/ 					return pFly;
/*N*/
/*N*/ 				const SwFrm *pTmp;
/*N*/ 				if ( 0 != (pTmp = lcl_FindFirstInvaCntnt( pFly, nBottom, 0 )) &&
/*N*/ 					 pTmp->Frm().Top() <= nBottom )
/*N*/ 					return pTmp;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ BOOL SwLayAction::IsShortCut( SwPageFrm *&prPage )
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	const FASTBOOL bBrowse = pRoot->GetFmt()->GetDoc()->IsBrowseMode();
/*N*/
/*N*/ 	//Wenn die Seite nicht Gueltig ist wird sie schnell formatiert, sonst
/*N*/ 	//gibts nix als Aerger.
/*N*/ 	if ( !prPage->IsValid() )
/*N*/ 	{
/*N*/ 		if ( bBrowse )
/*N*/ 		{
            /// OD 15.10.2002 #103517# - format complete page
            /// Thus, loop on all lowers of the page <prPage>, instead of only
            /// format its first lower.
            /// NOTE: In online layout (bBrowse == TRUE) a page can contain
            ///     a header frame and/or a footer frame beside the body frame.
/*N*/ 			prPage->Calc();
/*N*/             SwFrm* pPageLowerFrm = prPage->Lower();
/*N*/             while ( pPageLowerFrm )
/*N*/             {
/*N*/                 pPageLowerFrm->Calc();
/*N*/                 pPageLowerFrm = pPageLowerFrm->GetNext();
/*N*/             }
/*N*/ 		}
/*N*/ 		else
/*N*/ 			FormatLayout( prPage );
/*N*/ 		if ( IsAgain() )
/*?*/ 			return FALSE;
/*N*/ 	}
/*N*/
/*N*/
/*N*/ 	const SwRect &rVis = pImp->GetShell()->VisArea();
/*N*/ 	if ( (prPage->Frm().Top() >= rVis.Bottom()) ||
/*N*/ 		 (prPage->Frm().Left()>= rVis.Right()) )
/*N*/ 	{
/*N*/ 		bRet = TRUE;
/*N*/
/*N*/ 		//Jetzt wird es ein bischen unangenehm: Der erste CntntFrm dieser Seite
/*N*/ 		//im Bodytext muss Formatiert werden, wenn er dabei die Seite
/*N*/ 		//wechselt, muss ich nochmal eine Seite zuvor anfangen, denn
/*N*/ 		//es wurde ein PageBreak verarbeitet.
/*N*/ //Noch unangenehmer: Der naechste CntntFrm ueberhaupt muss
/*N*/ 		//Formatiert werden, denn es kann passieren, dass kurzfristig
/*N*/ 		//leere Seiten existieren (Bsp. Absatz ueber mehrere Seiten
/*N*/ 		//wird geloescht oder verkleinert).
/*N*/
/*N*/ 		//Ist fuer den Browser uninteressant, wenn der letzte Cnt davor bereits
/*N*/ 		//nicht mehr sichbar ist.
/*N*/
/*N*/ 		const SwPageFrm *p2ndPage = prPage;
/*N*/ 		const SwCntntFrm *pCntnt;
/*N*/ 		const SwLayoutFrm* pBody = p2ndPage->FindBodyCont();
/*N*/ 		if( p2ndPage->IsFtnPage() && pBody )
/*?*/ 			pBody = (SwLayoutFrm*)pBody->GetNext();
/*N*/ 		pCntnt = pBody ? pBody->ContainsCntnt() : 0;
/*N*/ 		while ( p2ndPage && !pCntnt )
/*N*/ 		{
/*N*/ 			p2ndPage = (SwPageFrm*)p2ndPage->GetNext();
/*N*/ 			if( p2ndPage )
/*N*/ 			{
/*N*/ 				pBody = p2ndPage->FindBodyCont();
/*N*/ 				if( p2ndPage->IsFtnPage() && pBody )
/*?*/ 					pBody = (SwLayoutFrm*)pBody->GetNext();
/*N*/ 				pCntnt = pBody ? pBody->ContainsCntnt() : 0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( pCntnt )
/*N*/ 		{
/*N*/ 			FASTBOOL bTstCnt = TRUE;
/*N*/ 			if ( bBrowse )
/*N*/ 			{
/*N*/ 				//Der Cnt davor schon nicht mehr sichtbar?
/*N*/ 				const SwFrm *pLst = pCntnt;
/*N*/ 				if ( pLst->IsInTab() )
/*?*/ 					pLst = pCntnt->FindTabFrm();
/*N*/ 				if ( pLst->IsInSct() )
/*?*/ 					pLst = pCntnt->FindSctFrm();
/*N*/ 				pLst = pLst->FindPrev();
/*N*/ 				if ( pLst &&
/*N*/ 					 (pLst->Frm().Top() >= rVis.Bottom() ||
/*N*/ 					  pLst->Frm().Left()>= rVis.Right()) )
/*N*/ 				{
/*?*/ 					bTstCnt = FALSE;
/*N*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 			if ( bTstCnt )
/*N*/ 			{
/*N*/ 				if ( pCntnt->IsInSct() )
/*N*/ 				{
/*N*/ 					const SwSectionFrm *pSct = ((SwFrm*)pCntnt)->ImplFindSctFrm();
/*N*/ 					if ( !pSct->IsValid() )
/*N*/ 					{
/*N*/ 						pSct->Calc();
/*N*/ 						pSct->SetCompletePaint();
/*N*/ 						if ( IsAgain() )
/*?*/ 							return FALSE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				if ( !pCntnt->IsValid() )
/*N*/ 				{	pCntnt->Calc();
/*N*/ 					pCntnt->SetCompletePaint();
/*N*/ 					if ( IsAgain() )
/*N*/ 						return FALSE;
/*N*/ 				}
/*N*/ 				if ( pCntnt->IsInTab() )
/*N*/ 				{
/*N*/ 					const SwTabFrm *pTab = ((SwFrm*)pCntnt)->ImplFindTabFrm();
/*N*/ 					if ( !pTab->IsValid() )
/*N*/ 					{
/*N*/ 						pTab->Calc();
/*N*/ 						pTab->SetCompletePaint();
/*N*/ 						if ( IsAgain() )
/*?*/ 							return FALSE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				if ( pCntnt->IsInSct() )
/*N*/ 				{
/*N*/ 					const SwSectionFrm *pSct = ((SwFrm*)pCntnt)->ImplFindSctFrm();
/*N*/ 					if ( !pSct->IsValid() )
/*N*/ 					{
/*N*/ 						pSct->Calc();
/*N*/ 						pSct->SetCompletePaint();
/*N*/ 						if ( IsAgain() )
/*?*/ 							return FALSE;
/*N*/ 					}
/*N*/ 				}
/*N*/ #ifdef USED
/*?*/ 				if ( (pCntnt->FindPageFrm() != p2ndPage) &&
/*?*/ 					 prPage->GetPrev() )
/*?*/ 				{
/*?*/ 					prPage = (SwPageFrm*)prPage->GetPrev();
/*?*/ 					bRet = FALSE;
/*?*/ 				}
/*N*/ #else
/*N*/ 				const SwPageFrm* pTmp = pCntnt->FindPageFrm();
/*N*/ 				if ( pTmp != p2ndPage && prPage->GetPrev() )
/*N*/ 				{
/*N*/ 					bRet = FALSE;
/*N*/ 					if( pTmp->GetPhyPageNum() < prPage->GetPhyPageNum()
/*N*/ 						&& pTmp->IsInvalid() )
/*N*/ 						prPage = (SwPageFrm*)pTmp;
/*N*/ 					else
/*N*/ 						prPage = (SwPageFrm*)prPage->GetPrev();
/*N*/ 				}
/*N*/ #endif
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if ( !bRet && bBrowse )
/*N*/ 	{
/*N*/ 		const long nBottom = rVis.Bottom();
/*N*/ 		const SwFrm *pFrm;
/*N*/ 		if ( prPage->GetSortedObjs() &&
/*N*/ 			 (prPage->IsInvalidFlyLayout() || prPage->IsInvalidFlyCntnt()) &&
/*N*/ 			 0 != (pFrm = lcl_FindFirstInvaFly( prPage, nBottom )) &&
/*N*/ 			 pFrm->Frm().Top() <= nBottom )
/*N*/ 		{
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 		if ( prPage->IsInvalidLayout() &&
/*N*/ 			 0 != (pFrm = lcl_FindFirstInvaLay( prPage, nBottom )) &&
/*N*/ 			 pFrm->Frm().Top() <= nBottom )
/*N*/ 		{
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 		if ( (prPage->IsInvalidCntnt() || prPage->IsInvalidFlyInCnt()) &&
/*N*/ 			 0 != (pFrm = lcl_FindFirstInvaCntnt( prPage, nBottom, 0 )) &&
/*N*/ 			 pFrm->Frm().Top() <= nBottom )
/*N*/ 		{
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 		bRet = TRUE;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*************************************************************************
|*
|*	SwLayAction::ChkFlyAnchor()
|*
|*	Ersterstellung		MA 30. Oct. 92
|*	Letzte Aenderung	MA 02. Sep. 96
|*
|*************************************************************************/
/*N*/ void SwLayAction::ChkFlyAnchor( SwFlyFrm *pFly, const SwPageFrm *pPage )
/*N*/ {
/*N*/ 	//Wenn der Fly innerhalb eines anderen Rahmens gebunden ist, so sollte
/*N*/ 	//dieser zuerst Formatiert werden.
/*N*/
/*N*/ 	if ( pFly->GetAnchor()->IsInTab() )
/*N*/ 		pFly->GetAnchor()->FindTabFrm()->Calc();
/*N*/
/*N*/ 	SwFlyFrm *pAnch = pFly->GetAnchor()->FindFlyFrm();
/*N*/ 	if ( pAnch )
/*N*/ 	{
/*N*/ 		ChkFlyAnchor( pAnch, pPage );
/*N*/ 		CHECKPAGE;
/*N*/ 		while ( pPage == pAnch->FindPageFrm() && FormatLayoutFly( pAnch ) )
/*N*/ 			/* do nothing */;
/*N*/ 	}
/*N*/ }


/*************************************************************************
|*
|*	SwLayAction::FormatFlyLayout()
|*
|*	Ersterstellung		MA 30. Oct. 92
|*	Letzte Aenderung	MA 03. Jun. 96
|*
|*************************************************************************/
/*N*/ void SwLayAction::FormatFlyLayout( const SwPageFrm *pPage )
/*N*/ {
/*N*/ 	for ( USHORT i = 0; pPage->GetSortedObjs() &&
/*N*/ 						i < pPage->GetSortedObjs()->Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pO = (*pPage->GetSortedObjs())[i];
/*N*/ 		if ( pO->IsWriterFlyFrame() )
/*N*/ 		{
/*N*/ 			const USHORT nOld = i;
/*N*/ 			SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 			ChkFlyAnchor( pFly, pPage );
/*N*/ 			if ( IsAgain() )
/*?*/ 				return;
/*N*/ 			while ( pPage == pFly->FindPageFrm() )
/*N*/ 			{
/*N*/ 				SwFrmFmt *pFmt = pFly->GetFmt();
/*N*/ 				if( FLY_AUTO_CNTNT == pFmt->GetAnchor().GetAnchorId() &&
/*N*/ 					pFly->GetAnchor() &&
/*N*/ 					( REL_CHAR == pFmt->GetHoriOrient().GetRelationOrient() ||
/*N*/ 					  REL_CHAR == pFmt->GetVertOrient().GetRelationOrient() ) )
/*?*/ 					_FormatCntnt( (SwCntntFrm*)pFly->GetAnchor(), pPage );
/*N*/ 				 if( !FormatLayoutFly( pFly ) )
/*N*/ 					break;
/*N*/ 			}
/*N*/ 			CHECKPAGE;
/*N*/ 			if ( !IS_FLYS )
/*N*/ 				break;
/*N*/ 			if ( nOld > pPage->GetSortedObjs()->Count() )
/*?*/ 				i -= nOld - pPage->GetSortedObjs()->Count();
/*N*/ 			else
/*N*/ 			{	//Positionswechsel!
/*N*/ 				USHORT nAct;
/*N*/ 				pPage->GetSortedObjs()->Seek_Entry(pFly->GetVirtDrawObj(),&nAct);
/*N*/ 				if ( nAct < i )
/*?*/ 					i = nAct;
/*N*/ 				else if ( nAct > i )
/*?*/ 					--i;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|*	SwLayAction::FormatLayout(), FormatLayoutFly, FormatLayoutTab()
|*
|*	Ersterstellung		MA 30. Oct. 92
|*	Letzte Aenderung	MA 18. May. 98
|*
|*************************************************************************/
// OD 15.11.2002 #105155# - introduce support for vertical layout
/*N*/ BOOL SwLayAction::FormatLayout( SwLayoutFrm *pLay, BOOL bAddRect )
/*N*/ {
/*N*/ 	ASSERT( !IsAgain(), "Ungueltige Seite beachten." );
/*N*/ 	if ( IsAgain() )
/*?*/ 		return FALSE;
/*N*/
/*N*/ 	BOOL bChanged = FALSE;
/*N*/ 	BOOL bAlreadyPainted = FALSE;
/*N*/     // OD 11.11.2002 #104414# - remember frame at complete paint
/*N*/     SwRect aFrmAtCompletePaint;
/*N*/
/*N*/ 	if ( !pLay->IsValid() || pLay->IsCompletePaint() )
/*N*/ 	{
/*N*/ 		if ( pLay->GetPrev() && !pLay->GetPrev()->IsValid() )
/*N*/ 			pLay->GetPrev()->SetCompletePaint();
/*N*/
/*N*/ 		SwRect aOldRect( pLay->Frm() );
/*N*/ 		pLay->Calc();
/*N*/ 		if ( aOldRect != pLay->Frm() )
/*N*/ 			bChanged = TRUE;
/*N*/
/*N*/ 		FASTBOOL bNoPaint = FALSE;
/*N*/         if ( pLay->IsPageBodyFrm() &&
/*N*/              pLay->Frm().Pos() == aOldRect.Pos() &&
/*N*/              pLay->Lower() &&
/*N*/              pLay->GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/ 		{
/*N*/ 			//HotFix: Vobis Homepage, nicht so genau hinsehen, sonst
/*N*/ 			//rpaints
/*N*/
/*N*/ 			//Einschraenkungen wegen Kopf-/Fusszeilen
/*N*/             if ( !( pLay->IsCompletePaint() &&
/*N*/                    ( pLay->GetFmt()->GetDoc()->IsHeadInBrowse() ||
/*N*/                      pLay->GetFmt()->GetDoc()->IsFootInBrowse() ||
/*N*/                      pLay->FindPageFrm()->FindFtnCont() )
/*N*/                   )
/*N*/                )
/*N*/ 			{
/*N*/ 				bNoPaint = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if ( !bNoPaint && IsPaint() && bAddRect && (pLay->IsCompletePaint() || bChanged) )
/*N*/ 		{
/*N*/ 			SwRect aPaint( pLay->Frm() );
/*N*/             // OD 13.02.2003 #i9719#, #105645# - consider border and shadow for
/*N*/             // page frames -> enlarge paint rectangle correspondingly.
/*N*/             if ( pLay->IsPageFrm() )
/*N*/             {
/*N*/                 SwPageFrm* pPageFrm = static_cast<SwPageFrm*>(pLay);
/*N*/                 const int nBorderWidth =
/*N*/                         pImp->GetShell()->GetOut()->PixelToLogic( Size( pPageFrm->BorderPxWidth(), 0 ) ).Width();
/*N*/                 const int nShadowWidth =
/*N*/                         pImp->GetShell()->GetOut()->PixelToLogic( Size( pPageFrm->ShadowPxWidth(), 0 ) ).Width();
/*N*/                 aPaint.Left( aPaint.Left() - nBorderWidth );
/*N*/                 aPaint.Top( aPaint.Top() - nBorderWidth );
/*N*/                 aPaint.Right( aPaint.Right() + nBorderWidth + nShadowWidth );
/*N*/                 aPaint.Bottom( aPaint.Bottom() + nBorderWidth + nShadowWidth );
/*N*/             }
/*N*/
/*N*/             if ( pLay->IsPageFrm() &&
/*N*/                  pLay->GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/ 			{
/*N*/                 // NOTE: no vertical layout in online layout
/*N*/                 //Ist die Aenderung ueberhaupt sichtbar?
/*N*/ 				if ( pLay->IsCompletePaint() )
/*N*/ 				{
/*N*/ 					pImp->GetShell()->AddPaintRect( aPaint );
/*N*/ 					bAddRect = FALSE;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					SwRegionRects aRegion( aOldRect );
/*N*/ 					aRegion -= aPaint;
                        USHORT i=0;
/*N*/ 					for ( i = 0; i < aRegion.Count(); ++i )
/*N*/ 						pImp->GetShell()->AddPaintRect( aRegion[i] );
/*N*/                     aRegion.ChangeOrigin( aPaint );
/*N*/ 					aRegion.Remove( 0, aRegion.Count() );
/*N*/ 					aRegion.Insert( aPaint, 0 );
/*N*/ 					aRegion -= aOldRect;
/*N*/ 					for ( i = 0; i < aRegion.Count(); ++i )
/*N*/ 						pImp->GetShell()->AddPaintRect( aRegion[i] );
/*N*/ 				}
/*N*/
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pImp->GetShell()->AddPaintRect( aPaint );
/*N*/                 bAlreadyPainted = TRUE;
/*N*/                 // OD 11.11.2002 #104414# - remember frame at complete paint
/*N*/                 aFrmAtCompletePaint = pLay->Frm();
/*N*/ 			}
/*N*/
/*N*/             // OD 13.02.2003 #i9719#, #105645# - provide paint of spacing
/*N*/             // between pages (not only for in online mode).
/*N*/             if ( pLay->IsPageFrm() )
/*N*/             {
/*N*/                 if ( pLay->GetPrev() )
/*N*/                 {
/*N*/                     SwRect aSpaceToPrevPage( pLay->Frm() );
/*N*/                     SwTwips nTop = aSpaceToPrevPage.Top() - DOCUMENTBORDER/2;
/*N*/                     if ( nTop >= 0 )
/*N*/                         aSpaceToPrevPage.Top( nTop );
/*N*/                     aSpaceToPrevPage.Bottom( pLay->Frm().Top() );
/*N*/                     pImp->GetShell()->AddPaintRect( aSpaceToPrevPage );
/*N*/                 }
/*N*/                 if ( pLay->GetNext() )
/*N*/                 {
/*N*/                     SwRect aSpaceToNextPage( pLay->Frm() );
/*N*/                     aSpaceToNextPage.Bottom( aSpaceToNextPage.Bottom() + DOCUMENTBORDER/2 );
/*N*/                     aSpaceToNextPage.Top( pLay->Frm().Bottom() );
/*N*/                     pImp->GetShell()->AddPaintRect( aSpaceToNextPage );
/*N*/                 }
/*N*/             }
/*N*/
/*N*/         }
/*N*/ 		pLay->ResetCompletePaint();
/*N*/ 	}
/*N*/
/*N*/ 	if ( IsPaint() && bAddRect &&
/*N*/ 		 !pLay->GetNext() && pLay->IsRetoucheFrm() && pLay->IsRetouche() )
/*N*/ 	{
/*N*/         // OD 15.11.2002 #105155# - vertical layout support
/*N*/         SWRECTFN( pLay );
/*N*/         SwRect aRect( pLay->GetUpper()->PaintArea() );
/*N*/         (aRect.*fnRect->fnSetTop)( (pLay->*fnRect->fnGetPrtBottom)() );
/*N*/ 		if ( !pImp->GetShell()->AddPaintRect( aRect ) )
/*N*/ 			pLay->ResetRetouche();
/*N*/ 	}
/*N*/
/*N*/ 	if( bAlreadyPainted )
/*N*/ 		bAddRect = FALSE;
/*N*/
/*N*/ 	CheckWaitCrsr();
/*N*/
/*N*/ 	if ( IsAgain() )
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	//Jetzt noch diejenigen Lowers versorgen die LayoutFrm's sind
/*N*/
/*N*/ 	if ( pLay->IsFtnFrm() )	//Hat keine LayFrms als Lower.
/*N*/ 		return bChanged;
/*N*/
/*N*/ 	SwFrm *pLow = pLay->Lower();
/*N*/ 	BOOL bTabChanged = FALSE;
/*N*/ 	while ( pLow && pLow->GetUpper() == pLay )
/*N*/ 	{
/*N*/ 		if ( pLow->IsLayoutFrm() )
/*N*/ 		{
/*N*/ 			if ( pLow->IsTabFrm() )
/*N*/ 				bTabChanged |= FormatLayoutTab( (SwTabFrm*)pLow, bAddRect );
/*N*/ 			// bereits zum Loeschen angemeldete Ueberspringen
/*N*/ 			else if( !pLow->IsSctFrm() || ((SwSectionFrm*)pLow)->GetSection() )
/*N*/ 				bChanged |= FormatLayout( (SwLayoutFrm*)pLow, bAddRect );
/*N*/ 		}
/*N*/ 		else if ( pImp->GetShell()->IsPaintLocked() )
/*N*/ 			//Abkuerzung im die Zyklen zu minimieren, bei Lock kommt das
/*N*/ 			//Paint sowieso (Primaer fuer Browse)
/*N*/ 			pLow->OptCalc();
/*N*/
/*N*/ 		if ( IsAgain() )
/*N*/ 			return FALSE;
/*N*/ 		pLow = pLow->GetNext();
/*N*/ 	}
/*N*/     // OD 11.11.2002 #104414# - add complete frame area as paint area, if frame
/*N*/     // area has been already added and after formating its lowers the frame area
/*N*/     // is enlarged.
/*N*/     if ( bAlreadyPainted &&
/*N*/          ( pLay->Frm().Width() > aFrmAtCompletePaint.Width() ||
/*N*/            pLay->Frm().Height() > aFrmAtCompletePaint.Height() )
/*N*/        )
/*N*/     {
/*N*/         pImp->GetShell()->AddPaintRect( pLay->Frm() );
/*N*/     }
/*N*/     return bChanged || bTabChanged;
/*N*/ }

/*N*/ BOOL SwLayAction::FormatLayoutFly( SwFlyFrm *pFly, BOOL bAddRect )
/*N*/ {
/*N*/ 	ASSERT( !IsAgain(), "Ungueltige Seite beachten." );
/*N*/ 	if ( IsAgain() )
/*?*/ 		return FALSE;
/*N*/
/*N*/ 	BOOL bChanged = FALSE;
/*N*/
/*N*/ 	if ( !pFly->IsValid() || pFly->IsCompletePaint() || pFly->IsInvalid() )
/*N*/ 	{
/*N*/ 		//Der Frame hat sich veraendert, er wird jetzt Formatiert
/*N*/ 		const SwRect aOldRect( pFly->Frm() );
/*N*/ 		pFly->Calc();
/*N*/ 		bChanged = aOldRect != pFly->Frm();
/*N*/
/*N*/ 		if ( IsPaint() && bAddRect && (pFly->IsCompletePaint() || bChanged) &&
/*N*/ 			 pFly->Frm().Top() > 0 && pFly->Frm().Left() > 0 )
/*N*/ 			pImp->GetShell()->AddPaintRect( pFly->Frm() );
/*N*/
/*N*/ 		if ( bChanged )
/*N*/ 			pFly->Invalidate();
/*N*/ 		else
/*N*/ 			pFly->Validate();
/*N*/ 		bAddRect = FALSE;
/*N*/ 		pFly->ResetCompletePaint();
/*N*/ 	}
/*N*/
/*N*/ 	if ( IsAgain() )
/*?*/ 		return FALSE;
/*N*/
/*N*/ 	//Jetzt noch diejenigen Lowers versorgen die LayoutFrm's sind
/*N*/ 	BOOL bTabChanged = FALSE;
/*N*/ 	SwFrm *pLow = pFly->Lower();
/*N*/ 	while ( pLow )
/*N*/ 	{
/*N*/ 		if ( pLow->IsLayoutFrm() )
/*N*/ 		{
/*N*/ 			if ( pLow->IsTabFrm() )
/*N*/ 				bTabChanged |= FormatLayoutTab( (SwTabFrm*)pLow, bAddRect );
/*N*/ 			else
/*N*/ 				bChanged |= FormatLayout( (SwLayoutFrm*)pLow, bAddRect );
/*N*/ 		}
/*N*/ 		pLow = pLow->GetNext();
/*N*/ 	}
/*N*/ 	return bChanged || bTabChanged;
/*N*/ }

/*N*/ BOOL MA_FASTCALL lcl_AreLowersScrollable( const SwLayoutFrm *pLay )
/*N*/ {
/*N*/ 	const SwFrm *pLow = pLay->Lower();
/*N*/ 	while ( pLow )
/*N*/ 	{
/*N*/ 		if ( pLow->IsCompletePaint() || !pLow->IsValid() )
/*N*/ 			return FALSE;
/*N*/ 		if ( pLow->IsLayoutFrm() && !::binfilter::lcl_AreLowersScrollable( (SwLayoutFrm*)pLow ))
/*N*/ 			return FALSE;
/*N*/ 		pLow = pLow->GetNext();
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ SwLayoutFrm * MA_FASTCALL lcl_IsTabScrollable( SwTabFrm *pTab )
/*N*/ {
/*N*/ 	//returnt die erste unveraenderte Zeile, oder 0 wenn nicht
/*N*/ 	//gescrollt werden darf.
/*N*/ 	if ( !pTab->IsCompletePaint() )
/*N*/ 	{
/*N*/ 		SwLayoutFrm *pUnchgdRow = 0;
/*N*/ 		SwLayoutFrm *pRow = (SwLayoutFrm*)pTab->Lower();
/*N*/ 		while ( pRow )
/*N*/ 		{
/*N*/ 			if ( !::binfilter::lcl_AreLowersScrollable( pRow ) )
/*N*/ 				pUnchgdRow = 0;
/*N*/ 			else if ( !pUnchgdRow )
/*N*/ 				pUnchgdRow = pRow;
/*N*/ 			pRow = (SwLayoutFrm*)pRow->GetNext();
/*N*/ 		}
/*N*/ 		return pUnchgdRow;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ void MA_FASTCALL lcl_ValidateLowers( SwLayoutFrm *pLay, const SwTwips nOfst,
/*N*/ 						SwLayoutFrm *pRow, SwPageFrm *pPage,
/*N*/ 						BOOL bResetOnly )
/*N*/ {
/*N*/ 	pLay->ResetCompletePaint();
/*N*/ 	SwFrm *pLow = pLay->Lower();
/*N*/ 	if ( pRow )
/*N*/ 		while ( pLow != pRow )
/*N*/ 			pLow = pLow->GetNext();
/*N*/
/*N*/ 	SwRootFrm *pRootFrm = 0;
/*N*/
/*N*/ 	while ( pLow )
/*N*/ 	{
/*N*/ 		if ( !bResetOnly )
/*N*/ 		{
/*N*/ 			SwRect aOldFrm( pLow->Frm() );
/*N*/ 			pLow->Frm().Pos().Y() += nOfst;
/*N*/ 			if( pLow->IsAccessibleFrm() )
/*N*/ 			{
/*N*/ 				if( !pRootFrm )
/*N*/ 					pRootFrm = pPage->FindRootFrm();
/*N*/ 				if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
/*N*/ 					pRootFrm->GetCurrShell() )
/*N*/ 				{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( pLow->IsLayoutFrm() )
/*N*/ 		{
/*N*/ 			::binfilter::lcl_ValidateLowers( (SwLayoutFrm*)pLow, nOfst, 0, pPage, bResetOnly);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pLow->ResetCompletePaint();
/*N*/ 			if ( pLow->GetDrawObjs() )
/*N*/ 			{
/*N*/ 				for ( USHORT i = 0; i < pLow->GetDrawObjs()->Count(); ++i )
/*N*/ 				{
/*N*/ 					SdrObject *pO = (*pLow->GetDrawObjs())[i];
/*N*/ 					if ( pO->IsWriterFlyFrame() )
/*N*/ 					{
/*N*/ 						SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 						if ( !bResetOnly )
/*N*/ 						{
/*N*/ 							pFly->Frm().Pos().Y() += nOfst;
/*N*/                             pFly->GetVirtDrawObj()->_SetRectsDirty();
/*N*/ 							if ( pFly->IsFlyInCntFrm() )
/*N*/ 								((SwFlyInCntFrm*)pFly)->AddRefOfst( nOfst );
/*N*/ 						}
/*N*/ 						::binfilter::lcl_ValidateLowers( pFly, nOfst, 0, pPage, bResetOnly);
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/                         // OD 30.06.2003 #108784# - consider 'virtual' drawing objects.
/*N*/                         if ( pO->ISA(SwDrawVirtObj) )
/*N*/                         {
/*N*/                             SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pO);
/*N*/                             pDrawVirtObj->SetAnchorPos( pLow->GetFrmAnchorPos( ::binfilter::HasWrap( pO ) ) );
/*N*/                             pDrawVirtObj->AdjustRelativePosToReference();
/*N*/                         }
/*N*/                         else
/*N*/                         {
/*N*/                             pO->SetAnchorPos( pLow->GetFrmAnchorPos( ::binfilter::HasWrap( pO ) ) );
/*N*/                             SwFrmFmt *pFrmFmt = FindFrmFmt( pO );
/*N*/                             if( !pFrmFmt ||
/*N*/                                 FLY_IN_CNTNT != pFrmFmt->GetAnchor().GetAnchorId() )
/*N*/                             {
/*N*/                                 ((SwDrawContact*)pO->GetUserCall())->ChkPage();
/*N*/                             }
/*N*/                             // OD 30.06.2003 #108784# - correct relative position
/*N*/                             // of 'virtual' drawing objects.
/*N*/                             SwDrawContact* pDrawContact =
/*N*/                                 static_cast<SwDrawContact*>(pO->GetUserCall());
/*N*/                             if ( pDrawContact )
/*N*/                             {
/*N*/                                 pDrawContact->CorrectRelativePosOfVirtObjs();
/*N*/                             }
/*N*/                         }
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( !bResetOnly )
/*N*/ 			pLow->Calc();			//#55435# Stabil halten.
/*N*/ 		pLow = pLow->GetNext();
/*N*/ 	}
/*N*/ }

/*N*/ void MA_FASTCALL lcl_AddScrollRectTab( SwTabFrm *pTab, SwLayoutFrm *pRow,
/*N*/ 							   const SwRect &rRect,
/*N*/ 							   const SwTwips nOfst)
/*N*/ {
/*N*/ 	//Wenn altes oder neues Rechteck mit einem Fly ueberlappen, in dem der
/*N*/ 	//Frm nicht selbst steht, so ist nichts mit Scrollen.
/*N*/ 	const SwPageFrm *pPage = pTab->FindPageFrm();
/*N*/ 	SwRect aRect( rRect );
/*N*/     // OD 04.11.2002 #104100# - <SWRECTFN( pTab )> not needed.
/*N*/     if( pTab->IsVertical() )
/*?*/         aRect.Pos().X() -= nOfst;
/*N*/     else
/*N*/ 	aRect.Pos().Y() += nOfst;
/*N*/ 	if ( pPage->GetSortedObjs() )
/*N*/ 	{
/*N*/ 		if ( ::binfilter::lcl_IsOverObj( pTab, pPage, rRect, aRect, pTab ) )
/*N*/ 			return;
/*N*/ 	}
/*N*/ 	if ( pPage->GetFmt()->GetBackground().GetGraphicPos() != GPOS_NONE )
/*?*/ 		return;
/*N*/
/*N*/ 	ViewShell *pSh = pPage->GetShell();
/*N*/ 	if ( pSh )
/*N*/ 		pSh->AddScrollRect( pTab, aRect, nOfst );
/*N*/ 	::binfilter::lcl_ValidateLowers( pTab, nOfst, pRow, pTab->FindPageFrm(),
/*N*/ 												pTab->IsLowersFormatted() );
/*N*/ }

// OD 31.10.2002 #104100#
// NOTE: no adjustments for vertical layout support necessary
/*N*/ BOOL CheckPos( SwFrm *pFrm )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	return TRUE;
/*N*/ }

// OD 31.10.2002 #104100#
// Implement vertical layout support
/*N*/ BOOL SwLayAction::FormatLayoutTab( SwTabFrm *pTab, BOOL bAddRect )
/*N*/ {
/*N*/ 	ASSERT( !IsAgain(), "8-) Ungueltige Seite beachten." );
/*N*/ 	if ( IsAgain() )
/*?*/ 		return FALSE;
/*N*/
/*N*/ 	SwDoc* pDoc = pRoot->GetFmt()->GetDoc();
/*N*/ 	const BOOL bOldIdle = pDoc->IsIdleTimerActive();
/*N*/ 	pDoc->StopIdleTimer();
/*N*/
/*N*/     BOOL bChanged = FALSE;
/*N*/ 	FASTBOOL bPainted = FALSE;
/*N*/
/*N*/ 	const SwPageFrm *pOldPage = pTab->FindPageFrm();
/*N*/
/*N*/     // OD 31.10.2002 #104100# - vertical layout support
/*N*/     // use macro to declare and init <sal_Bool bVert>, <sal_Bool bRev> and
/*N*/     // <SwRectFn fnRect> for table frame <pTab>.
/*N*/     SWRECTFN( pTab );
/*N*/
/*N*/ 	if ( !pTab->IsValid() || pTab->IsCompletePaint() || pTab->IsComplete() )
/*N*/ 	{
/*N*/ 		if ( pTab->GetPrev() && !pTab->GetPrev()->IsValid() )
/*N*/ 			pTab->GetPrev()->SetCompletePaint();
/*N*/
/*N*/ 		//Potenzielles Scrollrect ist die ganze Tabelle. Da bereits ein
/*N*/ 		//Wachstum innerhalb der Tabelle - und damit der Tabelle selbst -
/*N*/ 		//stattgefunden haben kann, muss die untere Kante durch die
/*N*/ 		//Unterkante der letzten Zeile bestimmt werden.
/*N*/ 		SwLayoutFrm *pRow;
/*N*/ 		SwRect aScrollRect( pTab->PaintArea() );
/*N*/ 		if ( IsPaint() || bAddRect )
/*N*/ 		{
/*N*/ 			pRow = (SwLayoutFrm*)pTab->Lower();
/*N*/ 			while ( pRow->GetNext() )
/*N*/ 				pRow = (SwLayoutFrm*)pRow->GetNext();
/*N*/             // OD 31.10.2002 #104100# - vertical layout support
/*N*/             (aScrollRect.*fnRect->fnSetBottom)( (pRow->Frm().*fnRect->fnGetBottom)() );
/*N*/ 			//Die Oberkante wird ggf. durch die erste unveraenderte Zeile bestimmt.
/*N*/ 			pRow = ::binfilter::lcl_IsTabScrollable( pTab );
/*N*/ 			if ( pRow && pRow != pTab->Lower() )
/*N*/                 // OD 31.10.2002 #104100# - vertical layout support
/*N*/                 (aScrollRect.*fnRect->fnSetTop)( (pRow->Frm().*fnRect->fnGetTop)() );
/*N*/ 		}
/*N*/
/*N*/ 		const SwFrm *pOldUp = pTab->GetUpper();
/*N*/
/*N*/ 		SwRect aOldRect( pTab->Frm() );
/*N*/ 		pTab->SetLowersFormatted( FALSE );
/*N*/ 		pTab->Calc();
/*N*/ 		if ( aOldRect != pTab->Frm() )
/*N*/ 			bChanged = TRUE;
/*N*/ 		SwRect aPaintFrm = pTab->PaintArea();
/*N*/
/*N*/ 		if ( IsPaint() && bAddRect )
/*N*/ 		{
/*N*/ 			if ( pRow && pOldUp == pTab->GetUpper() &&
/*N*/ 				 pTab->Frm().SSize() == aOldRect.SSize() &&
/*N*/                  // OD 31.10.2002 #104100# - vertical layout support
/*N*/                  (pTab->Frm().*fnRect->fnGetLeft)() == (aOldRect.*fnRect->fnGetLeft)() &&
/*N*/ 				 pTab->IsAnLower( pRow ) )
/*N*/ 			{
/*N*/ 				SwTwips nOfst;
/*N*/ 				if ( pRow->GetPrev() )
/*N*/ 				{
/*N*/ 					if ( pRow->GetPrev()->IsValid() ||
/*N*/ 						 ::binfilter::CheckPos( pRow->GetPrev() ) )
/*N*/                     {
/*N*/                         // OD 31.10.2002 #104100# - vertical layout support
/*N*/                         nOfst = -(pRow->Frm().*fnRect->fnTopDist)( (pRow->GetPrev()->Frm().*fnRect->fnGetBottom)() );
/*N*/ 					}
/*N*/ 				else
/*N*/ 						nOfst = 0;
/*N*/ 				}
/*N*/ 				else
/*N*/                     // OD 31.10.2002 #104100# - vertical layout support
/*N*/                     nOfst = (pTab->Frm().*fnRect->fnTopDist)( (aOldRect.*fnRect->fnGetTop)() );
/*N*/
/*N*/ 				if ( nOfst )
/*N*/ 				{
/*N*/ 					 ::binfilter::lcl_AddScrollRectTab( pTab, pRow, aScrollRect, nOfst );
/*N*/ 					 bPainted = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/
/*N*/             // OD 01.11.2002 #104100# - add condition <pTab->Frm().HasArea()>
/*N*/             if ( !pTab->IsCompletePaint() && pTab->IsComplete() &&
/*N*/ 				 ( pTab->Frm().SSize() != pTab->Prt().SSize() ||
/*N*/                    // OD 31.10.2002 #104100# - vertical layout support
/*N*/                    (pTab->*fnRect->fnGetLeftMargin)()
/*N*/                  ) &&
/*N*/                  pTab->Frm().HasArea()
/*N*/                )
/*N*/ 			{
/*N*/                 // OD 01.11.2002 #104100# - re-implement calculation of margin rectangles.
/*N*/                 SwRect aMarginRect;
/*N*/
/*N*/                 SwTwips nLeftMargin = (pTab->*fnRect->fnGetLeftMargin)();
/*N*/                 if ( nLeftMargin > 0)
/*N*/                 {
/*N*/                     aMarginRect = pTab->Frm();
/*N*/                     (aMarginRect.*fnRect->fnSetWidth)( nLeftMargin );
/*N*/                     pImp->GetShell()->AddPaintRect( aMarginRect );
/*N*/                 }
/*N*/
/*N*/                 if ( (pTab->*fnRect->fnGetRightMargin)() > 0)
/*N*/                 {
/*N*/                     aMarginRect = pTab->Frm();
/*N*/                     (aMarginRect.*fnRect->fnSetLeft)( (pTab->*fnRect->fnGetPrtRight)() );
/*N*/                     pImp->GetShell()->AddPaintRect( aMarginRect );
/*N*/                 }
/*N*/
/*N*/                 SwTwips nTopMargin = (pTab->*fnRect->fnGetTopMargin)();
/*N*/                 if ( nTopMargin > 0)
/*N*/                 {
/*N*/                     aMarginRect = pTab->Frm();
/*N*/                     (aMarginRect.*fnRect->fnSetHeight)( nTopMargin );
/*N*/                     pImp->GetShell()->AddPaintRect( aMarginRect );
/*N*/                 }
/*N*/
/*N*/                 if ( (pTab->*fnRect->fnGetBottomMargin)() > 0)
/*N*/                 {
/*N*/                     aMarginRect = pTab->Frm();
/*N*/                     (aMarginRect.*fnRect->fnSetTop)( (pTab->*fnRect->fnGetPrtBottom)() );
/*N*/                     pImp->GetShell()->AddPaintRect( aMarginRect );
/*N*/                 }
/*N*/ 			}
/*N*/ 			else if ( pTab->IsCompletePaint() )
/*N*/ 			{
/*N*/ 				pImp->GetShell()->AddPaintRect( aPaintFrm );
/*N*/ 				bAddRect = FALSE;
/*N*/ 				bPainted = TRUE;
/*N*/ 			}
/*N*/
/*N*/             if ( pTab->IsRetouche() && !pTab->GetNext() )
/*N*/ 			{
/*N*/ 				SwRect aRect( pTab->GetUpper()->PaintArea() );
/*N*/                 // OD 04.11.2002 #104100# - vertical layout support
/*N*/                 (aRect.*fnRect->fnSetTop)( (pTab->*fnRect->fnGetPrtBottom)() );
/*N*/                 if ( !pImp->GetShell()->AddPaintRect( aRect ) )
/*N*/ 					pTab->ResetRetouche();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bAddRect = FALSE;
/*N*/
/*N*/         if ( pTab->IsCompletePaint() && !pOptTab )
/*N*/ 			pOptTab = pTab;
/*N*/ 		pTab->ResetCompletePaint();
/*N*/ 	}
/*N*/ 	if ( IsPaint() && bAddRect && pTab->IsRetouche() && !pTab->GetNext() )
/*N*/ 	{
/*N*/         // OD 04.10.2002 #102779#
/*N*/         // set correct rectangle for retouche: area between bottom of table frame
/*N*/         // and bottom of paint area of the upper frame.
/*N*/         SwRect aRect( pTab->GetUpper()->PaintArea() );
/*N*/         // OD 04.11.2002 #104100# - vertical layout support
/*N*/         (aRect.*fnRect->fnSetTop)( (pTab->*fnRect->fnGetPrtBottom)() );
/*N*/ 		if ( !pImp->GetShell()->AddPaintRect( aRect ) )
/*N*/ 			pTab->ResetRetouche();
/*N*/ 	}
/*N*/
/*N*/ 	CheckWaitCrsr();
/*N*/
/*N*/ 	if ( bOldIdle )
/*N*/ 		pDoc->StartIdleTimer();
/*N*/
/*N*/ 	//Heftige Abkuerzung!
/*N*/ 	if ( pTab->IsLowersFormatted() &&
/*N*/          (bPainted || !pImp->GetShell()->VisArea().IsOver( pTab->Frm())) )
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	//Jetzt noch die Lowers versorgen
/*N*/ 	if ( IsAgain() )
/*N*/ 		return FALSE;
/*N*/ 	SwLayoutFrm *pLow = (SwLayoutFrm*)pTab->Lower();
/*N*/ 	while ( pLow )
/*N*/ 	{
/*N*/ 		bChanged |= FormatLayout( (SwLayoutFrm*)pLow, bAddRect );
/*N*/ 		if ( IsAgain() )
/*N*/ 			return FALSE;
/*N*/ 		pLow = (SwLayoutFrm*)pLow->GetNext();
/*N*/ 	}
/*N*/
/*N*/ 	if ( pOldPage->GetPhyPageNum() > (pTab->FindPageFrm()->GetPhyPageNum() + 1) )
/*N*/ 		SetNextCycle( TRUE );
/*N*/
/*N*/ 	return bChanged;
/*N*/ }

/*************************************************************************
|*
|*	SwLayAction::FormatCntnt()
|*
|*	Ersterstellung		MA 30. Oct. 92
|*	Letzte Aenderung	MA 16. Nov. 95
|*
|*************************************************************************/
/*N*/ BOOL SwLayAction::FormatCntnt( const SwPageFrm *pPage )
/*N*/ {
/*N*/ 	const SwCntntFrm *pCntnt = pPage->ContainsCntnt();
/*N*/ 	const FASTBOOL bBrowse = pRoot->GetFmt()->GetDoc()->IsBrowseMode();
/*N*/
/*N*/ 	while ( pCntnt && pPage->IsAnLower( pCntnt ) )
/*N*/ 	{
/*N*/ 		//Wenn der Cntnt sich eh nicht veraendert koennen wir ein paar
/*N*/ 		//Abkuerzungen nutzen.
/*N*/ 		const BOOL bFull = !pCntnt->IsValid() || pCntnt->IsCompletePaint() ||
/*N*/ 						   pCntnt->IsRetouche() || pCntnt->GetDrawObjs();
/*N*/ 		if ( bFull )
/*N*/ 		{
/*N*/ 			//Damit wir nacher nicht suchen muessen.
/*N*/ 			const BOOL bNxtCnt = IsCalcLayout() && !pCntnt->GetFollow();
/*N*/ 			const SwCntntFrm *pCntntNext = bNxtCnt ? pCntnt->GetNextCntntFrm() : 0;
/*N*/ 			const SwCntntFrm *pCntntPrev = pCntnt->GetPrev() ? pCntnt->GetPrevCntntFrm() : 0;
/*N*/
/*N*/ 			const SwLayoutFrm*pOldUpper  = pCntnt->GetUpper();
/*N*/ 			const SwTabFrm *pTab = pCntnt->FindTabFrm();
/*N*/ 			const BOOL bInValid = !pCntnt->IsValid() || pCntnt->IsCompletePaint();
/*N*/ 			const BOOL bOldPaint = IsPaint();
/*N*/ 			bPaint = bOldPaint && !(pTab && pTab == pOptTab);
/*N*/ 			_FormatCntnt( pCntnt, pPage );
/*N*/ 			bPaint = bOldPaint;
/*N*/
/*N*/ 			if ( !pCntnt->GetValidLineNumFlag() && pCntnt->IsTxtFrm() )
/*N*/ 			{
/*N*/ 				const ULONG nAllLines = ((SwTxtFrm*)pCntnt)->GetAllLines();
/*N*/ 				((SwTxtFrm*)pCntnt)->RecalcAllLines();
/*N*/ 				if ( IsPaintExtraData() && IsPaint() &&
/*N*/ 					 nAllLines != ((SwTxtFrm*)pCntnt)->GetAllLines() )
/*?*/ 					pImp->GetShell()->AddPaintRect( pCntnt->Frm() );
/*N*/ 			}
/*N*/
/*N*/ 			if ( IsAgain() )
/*N*/ 				return FALSE;
/*N*/
/*N*/ 			//Wenn Layout oder Flys wieder Invalid sind breche ich die Verarbeitung
/*N*/ 			//vorlaeufig ab - allerdings nicht fuer die BrowseView, denn dort wird
/*N*/ 			//das Layout staendig ungueltig, weil die Seitenhoehe angepasst wird.
/*N*/ 			//Desgleichen wenn der Benutzer weiterarbeiten will und mindestens ein
/*N*/ 			//Absatz verarbeitet wurde.
/*N*/ 			if ( (!pTab || (pTab && !bInValid)) )
/*N*/ 			{
/*N*/ 				CheckIdleEnd();
/*N*/                 // OD 14.04.2003 #106346# - consider interrupt formatting.
/*N*/                 if ( ( IsInterrupt() && !mbFormatCntntOnInterrupt ) ||
/*N*/                      ( !bBrowse && pPage->IsInvalidLayout() ) ||
/*N*/                      // OD 07.05.2003 #109435# - consider interrupt formatting
/*N*/                      ( IS_FLYS && IS_INVAFLY && !mbFormatCntntOnInterrupt )
/*N*/                    )
/*N*/ 					return FALSE;
/*N*/ 			}
/*N*/ 			if ( pOldUpper != pCntnt->GetUpper() )
/*N*/ 			{
/*N*/ 				const USHORT nCurNum = pCntnt->FindPageFrm()->GetPhyPageNum();
/*N*/ 				if (  nCurNum < pPage->GetPhyPageNum() )
/*N*/ 					nPreInvaPage = nCurNum;
/*N*/
/*N*/ 				//Wenn der Frm mehr als eine Seite rueckwaerts geflossen ist, so
/*N*/ 				//fangen wir nocheinmal von vorn an damit wir nichts auslassen.
/*N*/ 				if ( !IsCalcLayout() && pPage->GetPhyPageNum() > nCurNum+1 )
/*N*/ 				{
/*N*/ 					SetNextCycle( TRUE );
/*N*/                     // OD 07.05.2003 #109435# - consider interrupt formatting
/*N*/                     if ( !mbFormatCntntOnInterrupt )
/*N*/                     {
/*N*/                         return FALSE;
/*N*/                     }
/*N*/ 				}
/*N*/ 			}
/*N*/ 			//Wenn der Frame die Seite vorwaerts gewechselt hat, so lassen wir
/*N*/ 			//den Vorgaenger nocheinmal durchlaufen.
/*N*/ 			//So werden einerseits Vorgaenger erwischt, die jetzt fr Retouche
/*N*/ 			//verantwortlich sind, andererseits werden die Fusszeilen
/*N*/ 			//auch angefasst.
/*N*/ 			FASTBOOL bSetCntnt = TRUE;
/*N*/ 			if ( pCntntPrev )
/*N*/ 			{
/*N*/                 if ( !pCntntPrev->IsValid() && pPage->IsAnLower( pCntntPrev ) )
/*N*/ 					pPage->InvalidateCntnt();
/*N*/ 				if ( pOldUpper != pCntnt->GetUpper() &&
/*N*/ 					 pPage->GetPhyPageNum() < pCntnt->FindPageFrm()->GetPhyPageNum() )
/*N*/ 				{
/*N*/ 					pCntnt = pCntntPrev;
/*N*/ 					bSetCntnt = FALSE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if ( bSetCntnt )
/*N*/ 			{
/*N*/ 				if ( bBrowse && !IsIdle() && !IsCalcLayout() && !IsComplete() &&
/*N*/ 					 pCntnt->Frm().Top() > pImp->GetShell()->VisArea().Bottom())
/*N*/ 				{
/*N*/ 					 const long nBottom = pImp->GetShell()->VisArea().Bottom();
/*N*/ 					const SwFrm *pTmp = lcl_FindFirstInvaCntnt( pPage,
/*N*/ 															nBottom, pCntnt );
/*N*/ 					if ( !pTmp )
/*N*/ 					{
/*N*/ 						if ( (!(IS_FLYS && IS_INVAFLY) ||
/*N*/ 							  !lcl_FindFirstInvaFly( pPage, nBottom )) &&
/*N*/ 							  (!pPage->IsInvalidLayout() ||
/*N*/ 							   !lcl_FindFirstInvaLay( pPage, nBottom )))
/*N*/ 							SetBrowseActionStop( TRUE );
/*N*/                         // OD 14.04.2003 #106346# - consider interrupt formatting.
/*N*/                         if ( !mbFormatCntntOnInterrupt )
/*N*/                         {
/*N*/                             return FALSE;
/*N*/                         }
/*N*/ 					}
/*N*/ 				}
/*N*/ 				pCntnt = bNxtCnt ? pCntntNext : pCntnt->GetNextCntntFrm();
/*N*/ 			}
/*N*/
/*N*/ 			RESCHEDULE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if ( !pCntnt->GetValidLineNumFlag() && pCntnt->IsTxtFrm() )
/*N*/ 			{
/*N*/ 				const ULONG nAllLines = ((SwTxtFrm*)pCntnt)->GetAllLines();
/*N*/ 				((SwTxtFrm*)pCntnt)->RecalcAllLines();
/*N*/ 				if ( IsPaintExtraData() && IsPaint() &&
/*N*/ 					 nAllLines != ((SwTxtFrm*)pCntnt)->GetAllLines() )
/*?*/ 					pImp->GetShell()->AddPaintRect( pCntnt->Frm() );
/*N*/ 			}
/*N*/
/*N*/ 			//Falls der Frm schon vor der Abarbeitung hier formatiert wurde.
/*N*/ 			if ( pCntnt->IsTxtFrm() && ((SwTxtFrm*)pCntnt)->HasRepaint() &&
/*N*/ 				  IsPaint() )
/*N*/ 				PaintCntnt( pCntnt, pPage, pCntnt->Frm(), pCntnt->Frm().Bottom());
/*N*/ 			if ( IsIdle() )
/*N*/ 			{
/*N*/ 				CheckIdleEnd();
/*N*/                 // OD 14.04.2003 #106346# - consider interrupt formatting.
/*N*/                 if ( IsInterrupt() && !mbFormatCntntOnInterrupt )
/*N*/ 					return FALSE;
/*N*/ 			}
/*N*/ 			if ( bBrowse && !IsIdle() && !IsCalcLayout() && !IsComplete() &&
/*N*/ 				 pCntnt->Frm().Top() > pImp->GetShell()->VisArea().Bottom())
/*N*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const long nBottom = pImp->GetShell()->VisArea().Bottom();
/*N*/ 			}
/*N*/ 			pCntnt = pCntnt->GetNextCntntFrm();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	CheckWaitCrsr();
/*N*/     // OD 14.04.2003 #106346# - consider interrupt formatting.
/*N*/     return !IsInterrupt() || mbFormatCntntOnInterrupt;
/*N*/ }
/*************************************************************************
|*
|*	SwLayAction::_FormatCntnt()
|*
|* 	Beschreibung		Returnt TRUE wenn der Absatz verarbeitet wurde,
|* 						FALSE wenn es nichts zu verarbeiten gab.
|*	Ersterstellung		MA 07. Dec. 92
|*	Letzte Aenderung	MA 11. Mar. 98
|*
|*************************************************************************/
/*N*/ void SwLayAction::_FormatCntnt( const SwCntntFrm *pCntnt,
/*N*/                                 const SwPageFrm  *pPage )
/*N*/ {
/*N*/ 	//wird sind hier evtl. nur angekommen, weil der Cntnt DrawObjekte haelt.
/*N*/ 	const BOOL bDrawObjsOnly = pCntnt->IsValid() && !pCntnt->IsCompletePaint() &&
/*N*/ 						 !pCntnt->IsRetouche();
/*N*/     SWRECTFN( pCntnt )
/*N*/ 	if ( !bDrawObjsOnly && IsPaint() )
/*N*/ 	{
/*N*/ 		const BOOL bPosOnly = !pCntnt->GetValidPosFlag() &&
/*N*/ 							  !pCntnt->IsCompletePaint() &&
/*N*/ 							  pCntnt->GetValidSizeFlag() &&
/*N*/ 							  pCntnt->GetValidPrtAreaFlag() &&
/*N*/ 							  ( !pCntnt->IsTxtFrm() ||
/*N*/ 								!((SwTxtFrm*)pCntnt)->HasAnimation() );
/*N*/ 		const SwFrm *pOldUp = pCntnt->GetUpper();
/*N*/ 		const SwRect aOldRect( pCntnt->UnionFrm() );
/*N*/         const long nOldBottom = (pCntnt->*fnRect->fnGetPrtBottom)();
/*N*/ 		pCntnt->OptCalc();
/*N*/ 		if( IsAgain() )
/*N*/ 			return;
/*N*/         if( (*fnRect->fnYDiff)( (pCntnt->Frm().*fnRect->fnGetBottom)(),
/*N*/                                 (aOldRect.*fnRect->fnGetBottom)() ) < 0 )
/*N*/ 			pCntnt->SetRetouche();
/*N*/ 		const SwRect aNewRect( pCntnt->UnionFrm() );
/*N*/         if ( bPosOnly &&
/*N*/              (aNewRect.*fnRect->fnGetTop)() != (aOldRect.*fnRect->fnGetTop)() &&
/*N*/ 			 !pCntnt->IsInTab() && !pCntnt->IsInSct() &&
/*N*/              ( !pCntnt->GetPrev() || !pCntnt->GetPrev()->IsTabFrm() ) &&
/*N*/ 			 pOldUp == pCntnt->GetUpper() &&
/*N*/              (aNewRect.*fnRect->fnGetLeft)() == (aOldRect.*fnRect->fnGetLeft)() &&
/*N*/              aNewRect.SSize() == aOldRect.SSize()
/*N*/            )
/*N*/ 		{
/*N*/             _AddScrollRect( pCntnt, pPage, (*fnRect->fnYDiff)(
/*N*/                             (pCntnt->Frm().*fnRect->fnGetTop)(),
/*N*/                             (aOldRect.*fnRect->fnGetTop)() ), nOldBottom );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			PaintCntnt( pCntnt, pCntnt->FindPageFrm(), aOldRect, nOldBottom);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( IsPaint() && pCntnt->IsTxtFrm() && ((SwTxtFrm*)pCntnt)->HasRepaint() )
/*N*/             PaintCntnt( pCntnt, pPage, pCntnt->Frm(),
/*N*/                         (pCntnt->Frm().*fnRect->fnGetBottom)() );
/*N*/ 		pCntnt->OptCalc();
/*N*/ 	}
/*N*/
/*N*/ 	//Die im Absatz verankerten Flys wollen auch mitspielen.
/*N*/ 	const SwDrawObjs *pObjs = pCntnt->GetDrawObjs();
/*N*/ 	for ( USHORT i = 0; pObjs && i < pObjs->Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pO = (*pObjs)[i];
/*N*/ 		if ( pO->IsWriterFlyFrame() )
/*N*/ 		{
/*N*/ 			SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 			if ( pFly->IsFlyInCntFrm() && ((SwFlyInCntFrm*)pFly)->IsInvalid() )
/*N*/ 			{
/*N*/                 FormatFlyInCnt( (SwFlyInCntFrm*)pFly );
/*N*/ 				pObjs = pCntnt->GetDrawObjs();
/*N*/ 				CHECKPAGE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|*	SwLayAction::FormatFlyCntnt()
|*
|* 		- Returnt TRUE wenn der Inhalt aller Flys vollstaendig verarbeitet
|* 		  wurde, FALSE bei einem vorzeitigen Abbruch.
|*	Ersterstellung		MA 02. Dec. 92
|*	Letzte Aenderung	MA 16. Sep. 93
|*
|*************************************************************************/
/*N*/ BOOL SwLayAction::FormatFlyCntnt( const SwPageFrm *pPage, sal_Bool bDontShrink )
/*N*/ {
/*N*/ 	for ( USHORT i = 0; pPage->GetSortedObjs() &&
/*N*/ 						i < pPage->GetSortedObjs()->Count(); ++i )
/*N*/ 	{
/*N*/ 		if ( IsAgain() )
/*?*/ 			return FALSE;
/*N*/ 		SdrObject *pO = (*pPage->GetSortedObjs())[i];
/*N*/ 		if ( pO->IsWriterFlyFrame() )
/*N*/ 		{
/*N*/             SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/             sal_Bool bOldShrink = pFly->IsNoShrink();
/*N*/             if( bDontShrink )
/*?*/                 pFly->SetNoShrink( sal_True );
/*N*/ 			if ( !_FormatFlyCntnt( pFly ) )
/*N*/             {
/*N*/                 if( bDontShrink )
/*?*/                     pFly->SetNoShrink( bOldShrink );
/*N*/ 				return FALSE;
/*N*/             }
/*N*/             if( bDontShrink )
/*?*/                 pFly->SetNoShrink( bOldShrink );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }
/*************************************************************************
|*
|*	SwLayAction::FormatFlyInCnt()
|*
|*	Beschreibung 		Da die Flys im Cntnt nix mit der Seite am Hut
|*		(bzw. in den Bits ;-)) haben werden sie vom Cntnt (FormatCntnt)
|* 		gerufen und hier verarbeitet. Die Verarebeitungsmimik ist
|* 		prinzipiell die gleich wie bei Seiten nur nicht ganz so
|*		kompliziert (SwLayAction::Action()).
|* 		- Returnt TRUE wenn der Fly vollstaendig verbeitet wurde, FALSE bei
|* 		  einem vorzeitigen Abbruch.
|*	Ersterstellung		MA 04. Dec. 92
|*	Letzte Aenderung	MA 24. Jun. 96
|*
|*************************************************************************/
/*N*/ void SwLayAction::FormatFlyInCnt( SwFlyInCntFrm *pFly )
/*N*/ {
/*N*/     if ( IsAgain() )
/*?*/         return;
/*N*/ 	//Wg. Aenderung eine kleine Vorsichtsmassnahme. Es wird jetzt vor der
/*N*/ 	//Cntntformatierung das Flag validiert und wenn die Formatierung mit
/*N*/ 	//FALSE returnt wird halt wieder invalidiert.
/*N*/ 	while ( pFly->IsInvalid() )
/*N*/ 	{
/*N*/ 		if ( pFly->IsInvalidLayout() )
/*N*/ 		{
/*N*/ 			while ( FormatLayoutFly( pFly ) )
/*N*/ 			{
/*N*/ 				if ( IsAgain() )
/*?*/ 					return;
/*N*/ 			}
/*N*/ 			if ( IsAgain() )
/*?*/ 				return;
/*N*/ 			pFly->ValidateLayout();
/*N*/ 		}
/*N*/ 		if ( pFly->IsInvalidCntnt() )
/*N*/ 		{
/*N*/ 			pFly->ValidateCntnt();
/*N*/ 			if ( !_FormatFlyCntnt( pFly ) )
/*?*/ 				pFly->InvalidateCntnt();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	CheckWaitCrsr();
/*N*/ }
/*************************************************************************
|*
|*	SwLayAction::_FormatFlyCntnt()
|*
|*	Beschreibung:
|* 		- Returnt TRUE wenn alle Cntnts des Flys vollstaendig verarbeitet
|* 		  wurden. FALSE wenn vorzeitig unterbrochen wurde.
|*	Ersterstellung		MA 02. Dec. 92
|*	Letzte Aenderung	MA 24. Jun. 96
|*
|*************************************************************************/
/*N*/ BOOL SwLayAction::_FormatFlyCntnt( const SwFlyFrm *pFly )
/*N*/ {
/*N*/ 	BOOL bOneProcessed = FALSE;
/*N*/ 	const SwCntntFrm *pCntnt = pFly->ContainsCntnt();
/*N*/
/*N*/ 	while ( pCntnt )
/*N*/ 	{
/*N*/ 		if ( __FormatFlyCntnt( pCntnt ) )
/*N*/ 			bOneProcessed = TRUE;
/*N*/
/*N*/ 		if ( !pCntnt->GetValidLineNumFlag() && pCntnt->IsTxtFrm() )
/*N*/ 		{
/*N*/ 			const ULONG nAllLines = ((SwTxtFrm*)pCntnt)->GetAllLines();
/*N*/ 			((SwTxtFrm*)pCntnt)->RecalcAllLines();
/*N*/ 			if ( IsPaintExtraData() && IsPaint() &&
/*N*/ 				 nAllLines != ((SwTxtFrm*)pCntnt)->GetAllLines() )
/*?*/ 				pImp->GetShell()->AddPaintRect( pCntnt->Frm() );
/*N*/ 		}
/*N*/
/*N*/ 		if ( IsAgain() )
/*N*/ 			return FALSE;
/*N*/
/*N*/ 		//wenn eine Eingabe anliegt breche ich die Verarbeitung ab.
/*N*/ 		if ( bOneProcessed && !pFly->IsFlyInCntFrm() )
/*N*/ 		{
/*N*/ 			CheckIdleEnd();
/*N*/             // OD 14.04.2003 #106346# - consider interrupt formatting.
/*N*/             if ( IsInterrupt() && !mbFormatCntntOnInterrupt )
/*N*/ 				return FALSE;
/*N*/ 		}
/*N*/ 		pCntnt = pCntnt->GetNextCntntFrm();
/*N*/ 	}
/*N*/ 	CheckWaitCrsr();
/*N*/     // OD 14.04.2003 #106346# - consider interrupt formatting.
/*N*/     return !(IsInterrupt() && !mbFormatCntntOnInterrupt);
/*N*/ }

/*************************************************************************
|*
|*	SwLayAction::__FormatFlyCntnt()
|*
|* 	Beschreibung:
|* 		- Returnt TRUE, wenn der Cntnt verarbeitet,
|*		  d.h. Kalkuliert und/oder gepaintet wurde.
|*
|*	Ersterstellung		MA 05. Jan. 93
|*	Letzte Aenderung	MA 18. May. 95
|*
|*************************************************************************/
/*N*/ BOOL SwLayAction::__FormatFlyCntnt( const SwCntntFrm *pCntnt )
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	if ( !pCntnt->IsValid()	|| pCntnt->IsCompletePaint() ||
/*N*/ 		 pCntnt->IsRetouche() )
/*N*/ 	{
/*N*/ 		if ( IsPaint() )
/*N*/ 		{
/*N*/ 			const SwRect aOldRect( pCntnt->UnionFrm( TRUE ) );
/*N*/ 			const long	 nOldBottom = pCntnt->Frm().Top() + pCntnt->Prt().Bottom();
/*N*/ 			pCntnt->OptCalc();
/*N*/ 			if ( pCntnt->Frm().Bottom() <  aOldRect.Bottom() )
/*N*/ 				pCntnt->SetRetouche();
/*N*/ 			PaintCntnt( pCntnt, pCntnt->FindPageFrm(), aOldRect, nOldBottom );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pCntnt->OptCalc();
/*N*/ 		if( IsAgain() )
/*N*/ 			return FALSE;
/*N*/ 		bRet = TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//Falls der Frm schon vor der Abarbeitung hier formatiert wurde.
/*N*/ 		if ( pCntnt->IsTxtFrm() && ((SwTxtFrm*)pCntnt)->HasRepaint() &&
/*N*/ 			 IsPaint() )
/*N*/ 			PaintCntnt( pCntnt, pCntnt->FindPageFrm(), pCntnt->Frm(), pCntnt->Frm().Bottom());
/*N*/ 	}
/*N*/ 	//Die im Absatz verankerten Flys wollen auch mitspielen.
/*N*/ 	if ( pCntnt->GetDrawObjs() )
/*N*/ 	{
/*N*/ 		const SwDrawObjs *pObjs = pCntnt->GetDrawObjs();
/*N*/ 		for ( USHORT i = 0; i < pObjs->Count(); ++i )
/*N*/ 		{
/*N*/ 			SdrObject *pO = (*pObjs)[i];
/*N*/ 			if ( pO->IsWriterFlyFrame() )
/*N*/ 			{
/*N*/ 				SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 				if ( pFly->IsFlyInCntFrm() && ((SwFlyInCntFrm*)pFly)->IsInvalid() )
/*N*/ 				{
/*N*/ 					FormatFlyInCnt( (SwFlyInCntFrm*)pFly );
/*N*/ 					if ( IsAgain() )
/*N*/ 						return FALSE;
/*N*/ 					pObjs = pCntnt->GetDrawObjs();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ BOOL SwLayAction::IsStopPrt() const
/*N*/ {
/*N*/ 	BOOL bResult = FALSE;
/*N*/
/*N*/ 	if (pImp != NULL && pProgress != NULL)
/*N*/ 		bResult = pImp->IsStopPrt();
/*N*/
/*N*/ 	return bResult;
/*N*/ }

/*N*/ #ifdef DBG_UTIL
/*N*/ #if OSL_DEBUG_LEVEL > 1

/*************************************************************************
|*
|*	void SwLayIdle::SwLayIdle()
|*
|*	Ersterstellung		MA ??
|*	Letzte Aenderung	MA 09. Jun. 94
|*
|*************************************************************************/
/*N*/ void SwLayIdle::ShowIdle( ColorData eName )
/*N*/ {
/*N*/ 	if ( !bIndicator )
/*N*/ 	{
/*N*/ 		bIndicator = TRUE;
/*N*/ 		Window *pWin = pImp->GetShell()->GetWin();
/*N*/ 		if ( pWin )
/*N*/ 		{
/*N*/ 			Rectangle aRect( 0, 0, 5, 5 );
/*N*/ 			aRect = pWin->PixelToLogic( aRect );
/*N*/ 			pWin->Push( PUSH_FILLCOLOR );
/*N*/ 			pWin->SetFillColor( eName );
/*N*/ 			pWin->DrawRect( aRect );
/*N*/ 			pWin->Pop();
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*N*/ #define SHOW_IDLE( ColorData ) ShowIdle( ColorData )
/*N*/ #else
/*N*/ #define SHOW_IDLE( ColorData )
/*N*/ #endif
/*N*/ #else
/*N*/ #define SHOW_IDLE( ColorData )
/*N*/ #endif

/*************************************************************************
|*
|*	void SwLayIdle::SwLayIdle()
|*
|*	Ersterstellung		MA 30. Oct. 92
|*	Letzte Aenderung	MA 23. May. 95
|*
|*************************************************************************/
/*N*/ SwLayIdle::SwLayIdle( SwRootFrm *pRt, SwViewImp *pI ) :
/*N*/ 	pRoot( pRt ),
/*N*/ 	pImp( pI )
/*N*/ #ifdef DBG_UTIL
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	, bIndicator( FALSE )
/*N*/ #endif
/*N*/ #endif
/*N*/ {
/*N*/ 	pImp->pIdleAct = this;
/*N*/
/*N*/ 	SHOW_IDLE( COL_LIGHTRED );
/*N*/
/*N*/ 	pImp->GetShell()->EnableSmooth( FALSE );
/*N*/
/*N*/ 	//Zuerst den Sichtbaren Bereich Spellchecken, nur wenn dort nichts
/*N*/ 	//zu tun war wird das IdleFormat angestossen.
/*N*/ 	{
/*N*/ 		//Formatieren und ggf. Repaint-Rechtecke an der ViewShell vormerken.
/*N*/ 		//Dabei muessen kuenstliche Actions laufen, damit es z.B. bei
/*N*/ 		//Veraenderungen der Seitenzahl nicht zu unerwuenschten Effekten kommt.
/*N*/ 		//Wir merken uns bei welchen Shells der Cursor sichtbar ist, damit
/*N*/ 		//wir ihn bei Dokumentaenderung ggf. wieder sichbar machen koennen.
/*N*/ 		SvBools aBools;
/*N*/ 		ViewShell *pSh = pImp->GetShell();
/*N*/ 		do
/*N*/ 		{	++pSh->nStartAction;
/*N*/ 			BOOL bVis = FALSE;
/*N*/ 			if ( pSh->ISA(SwCrsrShell) )
/*N*/ 			{
/*N*/ #ifdef SW_CRSR_TIMER
/*N*/ 				((SwCrsrShell*)pSh)->ChgCrsrTimerFlag( FALSE );
/*N*/ #endif
/*N*/ 				bVis = ((SwCrsrShell*)pSh)->GetCharRect().IsOver(pSh->VisArea());
/*N*/ 			}
/*N*/ 			aBools.Insert( bVis, aBools.Count() );
/*N*/ 			pSh = (ViewShell*)pSh->GetNext();
/*N*/ 		} while ( pSh != pImp->GetShell() );
/*N*/
/*N*/ 		SwLayAction aAction( pRoot, pImp );
/*N*/ 		aAction.SetInputType( INPUT_ANY );
/*N*/ 		aAction.SetIdle( TRUE );
/*N*/ 		aAction.SetWaitAllowed( FALSE );
/*N*/ 		aAction.Action();
/*N*/
/*N*/ 		//Weitere Start-/EndActions nur auf wenn irgendwo Paints aufgelaufen
/*N*/ 		//sind oder wenn sich die Sichtbarkeit des CharRects veraendert hat.
/*N*/ 		FASTBOOL bActions = FALSE;
/*N*/ 		USHORT nBoolIdx = 0;
/*N*/ 		do
/*N*/ 		{	--pSh->nStartAction;
/*N*/
/*N*/ 			if ( pSh->Imp()->GetRegion() || pSh->Imp()->GetScrollRects() )
/*?*/ 				bActions = TRUE;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SwRect aTmp( pSh->VisArea() );
/*N*/ 				pSh->UISizeNotify();
/*N*/
/*N*/ 				bActions |= aTmp != pSh->VisArea() ||
/*N*/ 							aBools[nBoolIdx] !=
/*N*/ 								   ((SwCrsrShell*)pSh)->GetCharRect().IsOver(pSh->VisArea());
/*N*/ 			}
/*N*/
/*N*/ 			pSh = (ViewShell*)pSh->GetNext();
/*N*/ 			++nBoolIdx;
/*N*/ 		} while ( pSh != pImp->GetShell() );
/*N*/
/*N*/ 		if ( bActions )
/*N*/ 		{
/*?*/ 			//Start- EndActions aufsetzen. ueber die CrsrShell, damit der
/*?*/ 			//Cursor/Selektion und die VisArea korrekt gesetzt werden.
/*?*/ 			nBoolIdx = 0;
/*?*/ 			do
/*?*/ 			{	FASTBOOL bCrsrShell = pSh->IsA( TYPE(SwCrsrShell) );
/*?*/
/*?*/ 				if ( bCrsrShell )
/*?*/ 					((SwCrsrShell*)pSh)->SttCrsrMove();
/*?*/ //				else
/*?*/ //					pSh->StartAction();
/*?*/
/*?*/ 				//Wenn Paints aufgelaufen sind, ist es am sinnvollsten schlicht das
/*?*/ 				//gesamte Window zu invalidieren. Anderfalls gibt es Paintprobleme
/*?*/ 				//deren Loesung unverhaeltnissmaessig aufwendig waere.
/*?*/ 				//fix(18176):
/*?*/ 				SwViewImp *pImp = pSh->Imp();
/*?*/ 				FASTBOOL bUnlock = FALSE;
/*?*/ 				if ( pImp->GetRegion() || pImp->GetScrollRects() )
/*?*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pImp->DelRegions();
/*?*/ 				}
/*?*/
/*?*/ 				if ( bCrsrShell )
/*?*/ 					//Wenn der Crsr sichbar war wieder sichbar machen, sonst
/*?*/ 					//EndCrsrMove mit TRUE fuer IdleEnd.
/*?*/ 					((SwCrsrShell*)pSh)->EndCrsrMove( TRUE^aBools[nBoolIdx] );
/*?*/ //				else
/*?*/ //					pSh->EndAction();
/*?*/ 				if( bUnlock )
/*?*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( bCrsrShell )
/*?*/ 				}
/*?*/
/*?*/ 				pSh = (ViewShell*)pSh->GetNext();
/*?*/ 				++nBoolIdx;
/*?*/
/*?*/ 			} while ( pSh != pImp->GetShell() );
/*N*/ 		}
/*N*/
/*N*/ 		FASTBOOL bInValid;
/*N*/ 		const SwViewOption& rVOpt = *pImp->GetShell()->GetViewOptions();
/*N*/ 		SwPageFrm *pPg = (SwPageFrm*)pRoot->Lower();
/*N*/ 		do
/*N*/ 		{	bInValid = pPg->IsInvalidCntnt() || pPg->IsInvalidLayout() ||
/*N*/ 					   pPg->IsInvalidFlyCntnt() || pPg->IsInvalidFlyLayout() ||
/*N*/ 					   pPg->IsInvalidFlyInCnt();
/*N*/
/*N*/ 			pPg = (SwPageFrm*)pPg->GetNext();
/*N*/
/*N*/ 		} while ( pPg && TRUE^bInValid );
/*N*/
/*N*/ 		if ( TRUE^bInValid )
/*N*/ 			pRoot->ResetIdleFormat();
/*N*/ 	}
/*N*/
/*N*/ 	pImp->GetShell()->EnableSmooth( TRUE );
/*N*/
/*N*/ #ifdef ACCESSIBLE_LAYOUT
/*N*/ #endif
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	if ( bIndicator && pImp->GetShell()->GetWin() )
/*N*/ 	{
/*N*/ 		Rectangle aRect( 0, 0, 5, 5 );
/*N*/ 		aRect = pImp->GetShell()->GetWin()->PixelToLogic( aRect );
/*N*/ 		pImp->GetShell()->GetWin()->Invalidate( aRect );
/*N*/ 	}
/*N*/ #endif
/*N*/ #endif
/*N*/ }

/*N*/ SwLayIdle::~SwLayIdle()
/*N*/ {
/*N*/ 	pImp->pIdleAct = 0;
/*N*/ }

}
