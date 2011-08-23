/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#define ITEMID_SIZE			SID_ATTR_PAGE_SIZE
#include <hintids.hxx>


#include <tools/bigint.hxx>
#include <bf_svx/svdmodel.hxx>
#include <bf_svx/svdpage.hxx>
#include <bf_sfx2/progress.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svx/keepitem.hxx>
#include <bf_svx/shaditem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_sfx2/printer.hxx>


#include <horiornt.hxx>

#include <fmtornt.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <docary.hxx>
#include <lineinfo.hxx>
#include <swmodule.hxx>
#include "pagefrm.hxx"
#include "colfrm.hxx"
#include "doc.hxx"
#include "fesh.hxx"
#include "viewimp.hxx"
#include "pam.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "frmtool.hxx"
#include "docsh.hxx"
#include "tabfrm.hxx"
#include "rowfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "frmsh.hxx"
#include "layact.hxx"
#include "pagedesc.hxx"
#include "section.hxx"
#include "sectfrm.hxx"
#include "node2lay.hxx"
#include "ndole.hxx"
#include "ndtxt.hxx"
#include "hints.hxx"
#include <layhelp.hxx>
#include <laycache.hxx>

#include "mdiexp.hxx"
#include "statstr.hrc"
// OD 21.05.2003 #108789#
#include <paratr.hxx>
namespace binfilter {

// ftnfrm.cxx:
/*N*/ void lcl_RemoveFtns( SwFtnBossFrm* pBoss, BOOL bPageOnly, BOOL bEndNotes );

/*N*/ FASTBOOL bObjsDirect = TRUE;
/*N*/ FASTBOOL bDontCreateObjects = FALSE;
/*N*/ FASTBOOL bSetCompletePaintOnInvalidate = FALSE;

/*N*/ BYTE StackHack::nCnt = 0;
/*N*/ BOOL StackHack::bLocked = FALSE;



/*************************************************************************
|*
|*	SwFrmNotify::SwFrmNotify()
|*
|*	Ersterstellung		MA 27. Nov. 92
|*	Letzte Aenderung	MA 09. Apr. 97
|*
|*************************************************************************/

/*N*/ SwFrmNotify::SwFrmNotify( SwFrm *pF ) :
/*N*/ 	pFrm( pF ),
/*N*/ 	aFrm( pF->Frm() ),
/*N*/ 	aPrt( pF->Prt() ),
/*N*/ 	bInvaKeep( FALSE )
/*N*/ #ifdef ACCESSIBLE_LAYOUT
/*N*/ 	,bValidSize( pF->GetValidSizeFlag() )
/*N*/ #endif
/*N*/ {
/*N*/     if ( pF->IsTxtFrm() )
/*N*/     {
/*N*/         mnFlyAnchorOfst = ((SwTxtFrm*)pF)->GetBaseOfstForFly( sal_True );
/*N*/         mnFlyAnchorOfstNoWrap = ((SwTxtFrm*)pF)->GetBaseOfstForFly( sal_False );
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         mnFlyAnchorOfst = 0;
/*N*/         mnFlyAnchorOfstNoWrap = 0;
/*N*/     }
/*N*/ 
/*N*/ 	bHadFollow = pF->IsCntntFrm() ?
/*N*/ 					(((SwCntntFrm*)pF)->GetFollow() ? TRUE : FALSE) :
/*N*/ 					FALSE;
/*N*/ }

/*************************************************************************
|*
|*	SwFrmNotify::~SwFrmNotify()
|*
|*	Ersterstellung		MA 27. Nov. 92
|*	Letzte Aenderung	MA 09. Apr. 97
|*
|*************************************************************************/

/*N*/ SwFrmNotify::~SwFrmNotify()
/*N*/ {
/*N*/     SWRECTFN( pFrm )
/*N*/     const FASTBOOL bAbsP = POS_DIFF( aFrm, pFrm->Frm() );
/*N*/     const FASTBOOL bChgWidth =
/*N*/             (aFrm.*fnRect->fnGetWidth)() != (pFrm->Frm().*fnRect->fnGetWidth)();
/*N*/     const FASTBOOL bChgHeight =
/*N*/             (aFrm.*fnRect->fnGetHeight)()!=(pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/     const FASTBOOL bChgFlyBasePos = pFrm->IsTxtFrm() &&
/*N*/        ( ( mnFlyAnchorOfst != ((SwTxtFrm*)pFrm)->GetBaseOfstForFly( sal_True ) ) ||
/*N*/          ( mnFlyAnchorOfstNoWrap != ((SwTxtFrm*)pFrm)->GetBaseOfstForFly( sal_False ) ) );
/*N*/ 
/*N*/ 	if ( pFrm->IsFlowFrm() && !pFrm->IsInFtn() )
/*N*/ 	{
/*N*/ 		SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( pFrm );
/*N*/ 
/*N*/ 		if ( !pFlow->IsFollow() )
/*N*/ 		{
/*N*/ 			if ( !pFrm->GetIndPrev() )
/*N*/ 			{
/*N*/ 				if ( bInvaKeep )
/*N*/ 				{
/*N*/ 					//Wenn der Vorgaenger das Attribut fuer Zusammenhalten traegt
/*N*/ 					//muss er angestossen werden.
/*N*/ 					SwFrm *pPre;
/*N*/ 					if ( 0 != (pPre = pFrm->FindPrev()) &&
/*N*/ 						 pPre->GetAttrSet()->GetKeep().GetValue() )
/*N*/ 						pPre->InvalidatePos();
/*N*/ 				}
/*N*/ 			}
/*N*/             else if ( !pFlow->HasFollow() )
/*N*/             {
/*N*/                 long nOldHeight = (aFrm.*fnRect->fnGetHeight)();
/*N*/                 long nNewHeight = (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/                 if( (nOldHeight > nNewHeight) || (!nOldHeight && nNewHeight) )
/*N*/                     pFlow->CheckKeep();
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bAbsP )
/*N*/ 	{
/*N*/ 		pFrm->SetCompletePaint();
/*N*/ 
/*N*/ 		SwFrm* pNxt = pFrm->GetIndNext();
/*N*/ 
/*N*/ 		if ( pNxt )
/*N*/ 			pNxt->InvalidatePos();
/*N*/ 		else
/*N*/ 		{
/*N*/             // OD 04.11.2002 #104100# - correct condition for setting retouche
/*N*/             // flag for vertical layout.
/*N*/             if( pFrm->IsRetoucheFrm() &&
/*N*/                 (aFrm.*fnRect->fnTopDist)( (pFrm->Frm().*fnRect->fnGetTop)() ) > 0 )
/*N*/             {
/*N*/ 				pFrm->SetRetouche();
/*N*/             }
/*N*/ 
/*N*/ 			//Wenn ein TxtFrm gerade einen Follow erzeugt hat, so ist dieser
/*N*/ 			//frisch formatiert und braucht nicht nocheinmal angestossen werden.
/*N*/ 			if ( bHadFollow || !pFrm->IsCntntFrm() ||
/*N*/ 				 !((SwCntntFrm*)pFrm)->GetFollow() )
/*N*/ 				pFrm->InvalidateNextPos();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//Fuer Hintergrundgrafiken muss bei Groessenaenderungen ein Repaint her.
/*N*/     const FASTBOOL bPrtWidth =
/*N*/             (aPrt.*fnRect->fnGetWidth)() != (pFrm->Prt().*fnRect->fnGetWidth)();
/*N*/     const FASTBOOL bPrtHeight =
/*N*/             (aPrt.*fnRect->fnGetHeight)()!=(pFrm->Prt().*fnRect->fnGetHeight)();
/*N*/     if ( bPrtWidth || bPrtHeight )
/*N*/ 	{
/*N*/ 		const SvxGraphicPosition ePos = pFrm->GetAttrSet()->GetBackground().GetGraphicPos();
/*N*/ 		if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
/*N*/ 			pFrm->SetCompletePaint();
/*N*/ 	}
/*N*/     else
/*N*/     {
/*N*/         // OD 13.11.2002 #97597# - consider case that *only* margins between
/*N*/         // frame and printing area has changed. Then, frame has to be repainted,
/*N*/         // in order to force paint of the margin areas.
/*N*/         if ( !bAbsP && (bChgWidth || bChgHeight) )
/*N*/         {
/*N*/             pFrm->SetCompletePaint();
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/ 	const FASTBOOL bPrtP = POS_DIFF( aPrt, pFrm->Prt() );
/*N*/ 	if ( bAbsP || bPrtP || bChgWidth || bChgHeight ||
/*N*/          bPrtWidth || bPrtHeight || bChgFlyBasePos )
/*N*/ 	{
/*N*/ 		if( pFrm->IsAccessibleFrm() )
/*N*/ 		{
/*N*/ 			SwRootFrm *pRootFrm = pFrm->FindRootFrm();
/*N*/ 			if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
/*N*/ 				pRootFrm->GetCurrShell() )
/*N*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( pFrm, aFrm );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//Auch die Flys wollen etwas von den Veraenderungen mitbekommen,
/*N*/ 		//FlyInCnts brauchen hier nicht benachrichtigt werden.
/*N*/ 		if ( pFrm->GetDrawObjs() )
/*N*/ 		{
/*N*/ 			const SwDrawObjs &rObjs = *pFrm->GetDrawObjs();
/*N*/ 			SwPageFrm *pPage = 0;
/*N*/ 			for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/ 			{
/*N*/ 				FASTBOOL bNotify = FALSE;
/*N*/ 				FASTBOOL bNotifySize = FALSE;
/*N*/ 				SdrObject *pObj = rObjs[i];
/*N*/ 				if ( pObj->IsWriterFlyFrame() )
/*N*/ 				{
/*N*/ 					SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 					if ( !pFly->IsFlyInCntFrm() )
/*N*/ 					{
/*N*/ 						//Wenn sich die AbsPos geaendert hat oder der Anker kein
/*N*/ 						//CntntFrm ist, so benachrichten wir auf jeden Fall.
/*N*/ 						if ( bAbsP || !pFly->GetAnchor()->IsCntntFrm() )
/*N*/ 						{
/*N*/ 							bNotify = TRUE;
/*N*/ 							if ( bAbsP )
/*N*/ 							{
/*N*/ 								if ( !pPage )
/*N*/ 									pPage = pFrm->FindPageFrm();
/*N*/ 								SwPageFrm *pFlyPage = pFly->FindPageFrm();
/*N*/ 								// Am Rahmen gebundene Objekte wandern stets mit,
/*N*/ 								// an TxtFrms gebundene nicht unbedingt.
/*N*/ 								//MA 09. Jul. 98: An TxtFrms gebundene wurden
/*N*/ 								//bereits im MakeAll formatiert und sollten
/*N*/ 								//damit auf der richtigen Seite stehen.
/*N*/ 								if ( pPage != pFlyPage && pFrm->IsFlyFrm() )
/*N*/ //									 (pFrm->IsFlyFrm() || pOldPage != pPage ||
/*N*/ //									  WEIT_WECH == pFly->Frm().Top()) )
/*N*/ 								{
/*?*/ 									ASSERT( pFlyPage, "~SwFrmNotify: Fly from Nowhere" );
/*?*/ 									if( pFlyPage )
/*?*/ 										pFlyPage->MoveFly( pFly, pPage );
/*?*/ 									else
/*?*/ 										pPage->SwPageFrm::AppendFly( pFly );
/*N*/ 								}
/*N*/ 							}
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							//Andere benachrichtigen wir nur wenn sie eine
/*N*/ 							//automatische Ausrichtung haben.
/*N*/ 							//MA 16. Oct. 95: (fix:21063) Verfeinert.
/*N*/ 							const SwFmtVertOrient &rVert =
/*N*/ 										pFly->GetFmt()->GetVertOrient();
/*N*/ 							const SwFmtHoriOrient &rHori =
/*N*/ 										pFly->GetFmt()->GetHoriOrient();
/*N*/ 							if ( (rVert.GetVertOrient()    == VERT_CENTER  ||
/*N*/ 								  rVert.GetVertOrient()    == VERT_BOTTOM  ||
/*N*/ 								  rVert.GetRelationOrient()== PRTAREA)	&&
/*N*/                                  ( bChgHeight || bPrtHeight ) )
/*N*/                             {
/*N*/ 								bNotify = TRUE;
/*N*/ 							}
/*N*/ 							if ( ( rHori.GetHoriOrient() != HORI_NONE ||
/*N*/ 								   rHori.GetRelationOrient()== PRTAREA ||
/*N*/                                    rHori.GetRelationOrient()== FRAME ) &&
/*N*/                                  ( bChgWidth || bPrtWidth || bChgFlyBasePos ) )
/*N*/                             {
/*N*/ 								bNotify = TRUE;
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/                     else if( bPrtWidth )
/*N*/ 					{
/*N*/ 						bNotify = TRUE;
/*N*/ 						bNotifySize = TRUE;
/*N*/ 					}
/*N*/ 					if ( bNotify )
/*N*/ 					{
/*N*/ 						if ( bNotifySize )
/*N*/ 							pFly->_InvalidateSize();
/*N*/ 						pFly->_InvalidatePos();
/*N*/ 						pFly->_Invalidate();
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if ( bAbsP || bChgFlyBasePos )
/*N*/ 				{
/*N*/ 					SwFrmFmt *pFrmFmt = FindFrmFmt( pObj );
/*N*/ 					if( !pFrmFmt ||
/*N*/ 						FLY_IN_CNTNT != pFrmFmt->GetAnchor().GetAnchorId() )
/*N*/ 					{
/*N*/                         // OD 30.06.2003 #108784# - consider 'virtual' drawing objects.
/*N*/                         if ( pObj->ISA(SwDrawVirtObj) )
/*N*/                         {
/*N*/                             SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pObj);
/*N*/                             pDrawVirtObj->SetAnchorPos( pFrm->GetFrmAnchorPos( ::binfilter::HasWrap( pObj ) ) );
/*N*/                             pDrawVirtObj->AdjustRelativePosToReference();
/*N*/                         }
/*N*/                         else
/*N*/                         {
/*N*/                             pObj->SetAnchorPos( pFrm->GetFrmAnchorPos( ::binfilter::HasWrap( pObj ) ) );
/*N*/                             ((SwDrawContact*)GetUserCall(pObj))->ChkPage();
/*N*/                             // OD 30.06.2003 #108784# - correct relative position
/*N*/                             // of 'virtual' drawing objects.
/*N*/                             SwDrawContact* pDrawContact =
/*N*/                                 static_cast<SwDrawContact*>(pObj->GetUserCall());
/*N*/                             if ( pDrawContact )
/*N*/                             {
/*N*/                                 pDrawContact->CorrectRelativePosOfVirtObjs();
/*N*/                             }
/*N*/                         }
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ #ifdef ACCESSIBLE_LAYOUT
/*N*/ 	else if( pFrm->IsTxtFrm() && bValidSize != pFrm->GetValidSizeFlag() )
/*N*/ 	{
/*N*/ 		SwRootFrm *pRootFrm = pFrm->FindRootFrm();
/*N*/ 		if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
/*N*/ 			pRootFrm->GetCurrShell() )
/*N*/ 		{
/*N*/ 			pRootFrm->GetCurrShell()->Imp()->InvalidateAccessibleFrmContent( pFrm );
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|*	SwLayNotify::SwLayNotify()
|*
|*	Ersterstellung		MA 17. Nov. 92
|*	Letzte Aenderung	MA 03. Jun. 93
|*
|*************************************************************************/


/*N*/ SwLayNotify::SwLayNotify( SwLayoutFrm *pLayFrm ) :
/*N*/ 	SwFrmNotify( pLayFrm ),
/*N*/ 	nHeightOfst( 0 ),
/*N*/ 	nWidthOfst ( 0 ),
/*N*/ 	bLowersComplete( FALSE )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	SwLayNotify::~SwLayNotify()
|*
|*	Ersterstellung		MA 17. Nov. 92
|*	Letzte Aenderung	MA 13. Jun. 96
|*
|*************************************************************************/

/*M*/ void MA_FASTCALL lcl_MoveDrawObjs( SwFrm *pLow, const Point &rDiff,
/*M*/ 								   SwPageFrm *pNewPage )
/*M*/ {
/*M*/ 	for ( USHORT i = 0; pLow->GetDrawObjs() && i < pLow->GetDrawObjs()->Count();
/*M*/ 			++i )
/*M*/ 	{
/*M*/ 		SdrObject *pObj = (*pLow->GetDrawObjs())[i];
/*M*/ 		if ( pObj->IsWriterFlyFrame() )
/*M*/ 		{
/*M*/ 			SwFlyFrm *pF = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*M*/ 			if ( pF->Frm().Left() != WEIT_WECH )
/*M*/ 			{
/*M*/                 BOOL bOldBack = pF->IsNotifyBack();
/*M*/ 				{//Scope fuer Notify
/*M*/ 					SwFlyNotify aNotify( pF );
/*M*/                     pF->Frm().Pos() += rDiff;
/*M*/ 					//Wenn ein Fly die Position wechselt muss er
/*M*/ 					//natuerlich an der Seite umgemeldet werden.
/*M*/ 					if ( pF->IsFlyFreeFrm() )
/*M*/ 					{
/*M*/ 						if ( aNotify.GetOldPage() != pNewPage )
/*M*/                         {
/*M*/                             if( aNotify.GetOldPage() )
/*M*/                                 aNotify.GetOldPage()->MoveFly( pF, pNewPage );
/*M*/                             else
/*M*/                                 pNewPage->SwPageFrm::AppendFly( pF );
/*M*/                         }
/*M*/ 					}
/*M*/                     pF->ResetNotifyBack();
/*M*/ 				}
/*M*/                 if( bOldBack )
/*M*/                     pF->SetNotifyBack();
/*M*/ 			}
/*M*/ 		}
/*M*/ 		else
/*N*/ 		{
/*N*/             // OD 30.06.2003 #108784# - consider 'virtual' drawing objects.
/*N*/             if ( pObj->ISA(SwDrawVirtObj) )
/*N*/             {
/*N*/                 SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pObj);
/*N*/                 pDrawVirtObj->SetAnchorPos( pObj->GetAnchorPos() + rDiff );
/*N*/                 pDrawVirtObj->AdjustRelativePosToReference();
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 pObj->SetAnchorPos( pObj->GetAnchorPos() + rDiff );
/*N*/                 ((SwDrawContact*)GetUserCall(pObj))->ChkPage();
/*N*/                 // OD 30.06.2003 #108784# - correct relative position
/*N*/                 // of 'virtual' drawing objects.
/*N*/                 SwDrawContact* pDrawContact =
/*N*/                         static_cast<SwDrawContact*>(pObj->GetUserCall());
/*N*/                 if ( pDrawContact )
/*N*/                 {
/*N*/                     pDrawContact->CorrectRelativePosOfVirtObjs();
/*N*/                 }
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void MA_FASTCALL lcl_MoveLowerFlys( SwLayoutFrm *pLay, const Point &rDiff,
/*N*/ 									SwPageFrm *pNewPage )
/*N*/ {
/*N*/ 	if( pLay->IsFlyFrm() )
/*N*/ 		::binfilter::lcl_MoveDrawObjs( pLay, rDiff, pNewPage );
/*N*/ 
/*N*/ 	SwFrm *pLow = pLay->Lower();
/*N*/ 	if( !pLow )
/*N*/ 		return ;
/*N*/ 
/*N*/ 	do
/*N*/ 	{	if ( pLow->GetDrawObjs() )
/*N*/ 			::binfilter::lcl_MoveDrawObjs( pLow, rDiff, pNewPage );
/*N*/ 		pLow->Frm().Pos() += rDiff;
/*N*/ 		pLow->InvalidatePos();
/*N*/ 		if ( pLow->IsTxtFrm() )
/*N*/ 			((SwTxtFrm*)pLow)->Prepare( PREP_POS_CHGD );
/*N*/ 		else if ( pLow->IsTabFrm() )
/*N*/ 			pLow->InvalidatePrt();
/*N*/ 		if ( pLow->IsLayoutFrm() )
/*N*/ 			::binfilter::lcl_MoveLowerFlys( (SwLayoutFrm*)pLow, rDiff, pNewPage );
/*N*/ 
/*N*/ 		pLow = pLow->GetNext();
/*N*/ 	} while ( pLow );
/*N*/ }

/*N*/ SwLayNotify::~SwLayNotify()
/*N*/ {
/*N*/ 	SwLayoutFrm *pLay = GetLay();
/*N*/     SWRECTFN( pLay )
/*N*/ 	FASTBOOL bNotify = FALSE;
/*N*/ 	if ( pLay->Prt().SSize() != aPrt.SSize() )
/*N*/ 	{
/*N*/ 		if ( !IsLowersComplete() )
/*N*/ 		{
/*N*/ 			BOOL bInvaPercent;
/*N*/ 
/*N*/ 			if ( pLay->IsRowFrm() )
/*N*/ 			{
/*N*/ 				bInvaPercent = TRUE;
/*N*/                 long nNew = (pLay->Prt().*fnRect->fnGetHeight)();
/*N*/                 if( nNew != (aPrt.*fnRect->fnGetHeight)() )
/*N*/                      ((SwRowFrm*)pLay)->AdjustCells( nNew, TRUE);
/*N*/                 if( (pLay->Prt().*fnRect->fnGetWidth)()
/*N*/                     != (aPrt.*fnRect->fnGetWidth)() )
/*N*/ 					 ((SwRowFrm*)pLay)->AdjustCells( 0, FALSE );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				//Proportionale Anpassung der innenliegenden.
/*N*/ 				//1. Wenn der Formatierte kein Fly ist
/*N*/ 				//2. Wenn er keine Spalten enthaelt
/*N*/ 				//3. Wenn der Fly eine feste Hoehe hat und die Spalten in der
/*N*/ 				//	 Hoehe danebenliegen.
/*N*/ 				//4. niemals bei SectionFrms.
/*N*/ 				BOOL bLow;
/*N*/ 				if( pLay->IsFlyFrm() )
/*N*/ 				{
/*N*/ 					if ( pLay->Lower() )
/*N*/ 					{
/*N*/ 						bLow = !pLay->Lower()->IsColumnFrm() ||
/*N*/                             (pLay->Lower()->Frm().*fnRect->fnGetHeight)()
/*N*/                              != (pLay->Prt().*fnRect->fnGetHeight)();
/*N*/ 					}
/*N*/ 					else
/*?*/ 						bLow = FALSE;
/*N*/ 				}
/*N*/ 				else if( pLay->IsSctFrm() )
/*N*/ 				{
/*N*/ 					if ( pLay->Lower() )
/*N*/ 					{
/*N*/ 						if( pLay->Lower()->IsColumnFrm() && pLay->Lower()->GetNext() )
/*?*/ 							bLow = pLay->Lower()->Frm().Height() != pLay->Prt().Height();
/*N*/ 						else
/*N*/ 							bLow = pLay->Prt().Width() != aPrt.Width();
/*N*/ 					}
/*N*/ 					else
/*N*/ 						bLow = FALSE;
/*N*/ 				}
/*N*/                 else if( pLay->IsFooterFrm() && !pLay->HasFixSize() )
/*N*/                     bLow = pLay->Prt().Width() != aPrt.Width();
/*N*/                 else
/*N*/ 					bLow = TRUE;
/*N*/ 				bInvaPercent = bLow;
/*N*/ 				if ( bLow )
/*N*/ 				{
/*N*/ 					if ( nHeightOfst || nWidthOfst )
/*N*/ 					{
/*N*/ 						const Size aSz( aPrt.Width()  + nWidthOfst,
/*N*/ 										aPrt.Height() + nHeightOfst );
/*N*/ 						if ( pLay->Prt().SSize() != aSz )
/*N*/ 							pLay->ChgLowersProp( aSz );
/*N*/ 					}
/*N*/ 					else
/*N*/ 						pLay->ChgLowersProp( aPrt.SSize() );
/*N*/ 
/*N*/ 				}
/*N*/ 				//Wenn die PrtArea gewachsen ist, so ist es moeglich, dass die
/*N*/ 				//Kette der Untergeordneten einen weiteren Frm aufnehmen kann,
/*N*/ 				//mithin muss also der 'moeglicherweise passende' Invalidiert werden.
/*N*/ 				//Das invalidieren lohnt nur, wenn es sich beim mir bzw. meinen
/*N*/ 				//Uppers um eine Moveable-Section handelt.
/*N*/ 				//Die PrtArea ist gewachsen, wenn die Breite oder die Hoehe groesser
/*N*/ 				//geworden ist.
/*N*/ 				if ( (pLay->Prt().Height() > aPrt.Height() ||
/*N*/ 					  pLay->Prt().Width()  > aPrt.Width()) &&
/*N*/ 					 (pLay->IsMoveable() || pLay->IsFlyFrm()) )
/*N*/ 				{
/*N*/ 					SwFrm *pFrm = pLay->Lower();
/*N*/ 					if ( pFrm && pFrm->IsFlowFrm() )
/*N*/ 					{
/*N*/ 						while ( pFrm->GetNext() )
/*N*/ 							pFrm = pFrm->GetNext();
/*N*/ 						pFrm->InvalidateNextPos();
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			bNotify = TRUE;
/*N*/ 			//TEUER!! aber wie macht man es geschickter?
/*N*/ 			if( bInvaPercent )
/*N*/                 pLay->InvaPercentLowers( pLay->Prt().Height() - aPrt.Height() );
/*N*/ 		}
/*N*/ 		if ( pLay->IsTabFrm() )
/*N*/ 			//Damit _nur_ der Shatten bei Groessenaenderungen gemalt wird.
/*N*/ 			((SwTabFrm*)pLay)->SetComplete();
/*N*/ 		else if ( !pLay->GetFmt()->GetDoc()->IsBrowseMode() ||
/*N*/ 				  !(pLay->GetType() & (FRM_BODY | FRM_PAGE)) )
/*N*/ 			//Damit die untergeordneten sauber retouchiert werden.
/*N*/ 			//Problembsp: Flys an den Henkeln packen und verkleinern.
/*N*/ 			//Nicht fuer Body und Page, sonst flackerts beim HTML-Laden.
/*N*/ 			pLay->SetCompletePaint();
/*N*/ 
/*N*/ 	}
/*N*/ 	//Lower benachrichtigen wenn sich die Position veraendert hat.
/*N*/     const BOOL bPrtPos = POS_DIFF( aPrt, pLay->Prt() );
/*N*/     const BOOL bPos = bPrtPos || POS_DIFF( aFrm, pLay->Frm() );
/*N*/ 	const BOOL bSize = pLay->Frm().SSize() != aFrm.SSize();
/*N*/ 
/*N*/ 	if ( bPos && pLay->Lower() && !IsLowersComplete() )
/*N*/ 		pLay->Lower()->InvalidatePos();
/*N*/ 
/*N*/ 	if ( bPrtPos )
/*N*/ 		pLay->SetCompletePaint();
/*N*/ 
/*N*/ 	//Nachfolger benachrichtigen wenn sich die SSize geaendert hat.
/*N*/ 	if ( bSize )
/*N*/ 	{
/*N*/ 		if( pLay->GetNext() )
/*N*/ 		{
/*N*/ 			if ( pLay->GetNext()->IsLayoutFrm() )
/*N*/ 				pLay->GetNext()->_InvalidatePos();
/*N*/ 			else
/*N*/ 				pLay->GetNext()->InvalidatePos();
/*N*/ 		}
/*N*/ 		else if( pLay->IsSctFrm() )
/*N*/ 			pLay->InvalidateNextPos();
/*N*/ 	}
/*N*/ 	if ( !IsLowersComplete() &&
/*N*/ 		 !((pLay->GetType()&FRM_FLY|FRM_SECTION) &&
/*N*/ 			pLay->Lower() && pLay->Lower()->IsColumnFrm()) &&
/*N*/ 		 (bPos || bNotify) && !(pLay->GetType() & 0x1823) )  //Tab, Row, FtnCont, Root, Page
/*N*/ 	{
/*N*/ 		pLay->NotifyFlys();
/*N*/ 	}
/*N*/ 	if ( bPos && pLay->IsFtnFrm() && pLay->Lower() )
/*N*/ 	{
/*N*/         Point aDiff( (pLay->Frm().*fnRect->fnGetPos)() );
/*N*/         aDiff -= (aFrm.*fnRect->fnGetPos)();
/*N*/ 		lcl_MoveLowerFlys( pLay, aDiff, pLay->FindPageFrm() );
/*N*/ 	}
/*N*/ 	if( ( bPos || bSize ) && pLay->IsFlyFrm() && ((SwFlyFrm*)pLay)->GetAnchor()
/*N*/ 		  && ((SwFlyFrm*)pLay)->GetAnchor()->IsFlyFrm() )
/*?*/ 		((SwFlyFrm*)pLay)->GetAnchor()->InvalidateSize();
/*N*/ }

/*************************************************************************
|*
|*	SwFlyNotify::SwFlyNotify()
|*
|*	Ersterstellung		MA 17. Nov. 92
|*	Letzte Aenderung	MA 26. Aug. 93
|*
|*************************************************************************/

/*N*/ SwFlyNotify::SwFlyNotify( SwFlyFrm *pFlyFrm ) :
/*N*/ 	SwLayNotify( pFlyFrm ),
/*N*/ 	pOldPage( pFlyFrm->FindPageFrm() ),
/*N*/ 	aFrmAndSpace( pFlyFrm->AddSpacesToFrm() )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	SwFlyNotify::~SwFlyNotify()
|*
|*	Ersterstellung		MA 17. Nov. 92
|*	Letzte Aenderung	MA 09. Nov. 95
|*
|*************************************************************************/

/*N*/ SwFlyNotify::~SwFlyNotify()
/*N*/ {
/*N*/ 	SwFlyFrm *pFly = GetFly();
/*N*/ 	if ( pFly->IsNotifyBack() )
/*N*/ 	{
/*N*/ 		ViewShell *pSh = pFly->GetShell();
/*N*/ 		SwViewImp *pImp = pSh ? pSh->Imp() : 0;
/*N*/ 		if ( !pImp || !pImp->IsAction() || !pImp->GetLayAction().IsAgain() )
/*N*/ 		{
/*N*/ 			//Wenn in der LayAction das IsAgain gesetzt ist kann es sein,
/*N*/ 			//dass die alte Seite inzwischen vernichtet wurde!
/*N*/ 			::binfilter::Notify( pFly, pOldPage, aFrmAndSpace );
/*N*/ 		}
/*N*/ 		pFly->ResetNotifyBack();
/*N*/ 	}
/*N*/ 
/*N*/ 	//Haben sich Groesse oder Position geaendert, so sollte die View
/*N*/ 	//das wissen.
/*N*/     SWRECTFN( pFly )
/*N*/     const BOOL bPosChgd = POS_DIFF( aFrm, pFly->Frm() );
/*N*/     if ( bPosChgd || pFly->Frm().SSize() != aFrm.SSize() )
/*N*/ 	{
/*N*/ 		pFly->NotifyDrawObj();
/*N*/ 	}
/*N*/ 	if ( bPosChgd && aFrm.Pos().X() != WEIT_WECH )
/*N*/ 	{
/*N*/ 		//Bei Spalten sind die Lower wahrscheinlich bereits Formatiert und
/*N*/ 		//Positioniert. Bei zeichengebundenen Rahmen mit Spalten macht dies
/*N*/ 		//heftige Probleme #42867#
/*N*/ 		if ( pFly->Lower() &&
/*N*/ 			 (!pFly->IsFlyInCntFrm() || !pFly->Lower()->IsColumnFrm()) )
/*N*/ 		{
/*N*/             Point aDiff( (pFly->Frm().*fnRect->fnGetPos)() );
/*N*/             aDiff -= (aFrm.*fnRect->fnGetPos)();
/*N*/ 			lcl_MoveLowerFlys( pFly, aDiff, pFly->FindPageFrm() );
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( pFly->IsFlyAtCntFrm() )
/*N*/ 		{
/*N*/ 			SwFrm *pNxt = pFly->GetAnchor()->FindNext();
/*N*/ 			if ( pNxt )
/*N*/ 				pNxt->InvalidatePos();
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|*	SwCntntNotify::SwCntntNotify()
|*
|*	Ersterstellung		MA 24. Nov. 92
|*	Letzte Aenderung	MA 16. May. 95
|*
|*************************************************************************/

/*N*/ SwCntntNotify::SwCntntNotify( SwCntntFrm *pCntntFrm ) :
/*N*/ 	SwFrmNotify( pCntntFrm )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	SwCntntNotify::~SwCntntNotify()
|*
|*	Ersterstellung		MA 24. Nov. 92
|*	Letzte Aenderung	MA 09. Apr. 97
|*
|*************************************************************************/

/*N*/ SwCntntNotify::~SwCntntNotify()
/*N*/ {
/*N*/ 	SwCntntFrm *pCnt = GetCnt();
/*N*/ 	if ( bSetCompletePaintOnInvalidate )
/*N*/ 		pCnt->SetCompletePaint();
/*N*/ 
/*N*/ 
/*N*/ 	//Wenn sich meine PrtArea in der Fix-Size geaendert hat, so muss mein
/*N*/ 	//Nachfolger dazu angeregt werden sich auch neu zu Formatieren.
/*N*/ 
/*N*/ //MA: Ist das wirklich noetig? Auf keinen Fall sollte das doch notwendig sein,
/*N*/ //wenn der Frm das erste Mal formatiert wurde (alte PrtArea == 0).
/*	if ( pCnt->GetNext() &&
         pCnt->Prt().Width() != aPrt.Width() )
    {
        pCnt->GetNext()->Prepare( PREP_FIXSIZE_CHG );
        pCnt->GetNext()->_InvalidatePrt();
        pCnt->GetNext()->InvalidateSize();
    }
*/
/*N*/     SWRECTFN( pCnt )
/*N*/     if ( pCnt->IsInTab() && ( POS_DIFF( pCnt->Frm(), aFrm ) ||
/*N*/ 							 pCnt->Frm().SSize() != aFrm.SSize()))
/*N*/ 	{
/*N*/ 		SwLayoutFrm* pCell = pCnt->GetUpper();
/*N*/ 		while( !pCell->IsCellFrm() && pCell->GetUpper() )
/*?*/ 			pCell = pCell->GetUpper();
/*N*/ 		ASSERT( pCell->IsCellFrm(), "Where's my cell?" );
/*N*/ 		if ( VERT_NONE != pCell->GetFmt()->GetVertOrient().GetVertOrient() )
/*N*/ 			pCell->InvalidatePrt();	//fuer vertikale Ausrichtung.
/*N*/ 	}
/*N*/ 
/*N*/     FASTBOOL bFirst = (aFrm.*fnRect->fnGetWidth)() == 0;
/*N*/ 
/*N*/ 	if ( pCnt->IsNoTxtFrm() )
/*N*/ 	{
/*N*/ 		//Aktive PlugIn's oder OLE-Objekte sollten etwas von der Veraenderung
/*N*/ 		//mitbekommen, damit sie Ihr Window entsprechend verschieben.
/*N*/ 		ViewShell *pSh  = pCnt->GetShell();
/*N*/ 		if ( pSh )
/*N*/ 		{
/*N*/ 			SwOLENode *pNd;
/*N*/ 			if ( 0 != (pNd = pCnt->GetNode()->GetOLENode()) &&
/*N*/ 				 (pNd->GetOLEObj().IsOleRef() ||
/*N*/ 				  pNd->IsOLESizeInvalid()) )
/*N*/ 			{
/*N*/ 				ASSERT( pCnt->IsInFly(), "OLE not in FlyFrm" );
/*N*/ 				SwFlyFrm *pFly = pCnt->FindFlyFrm();
/*N*/ 				SvEmbeddedObjectRef xObj( (SvInPlaceObject*) pNd->GetOLEObj().GetOleRef() );
/*N*/ 				SwFEShell *pFESh = 0;
/*N*/ 				ViewShell *pTmp = pSh;
/*N*/ 				do
/*N*/ 				{	if ( pTmp->ISA( SwCrsrShell ) )
/*N*/ 					{
/*N*/ 						pFESh = (SwFEShell*)pTmp;
                        // #108369#: Here used to be the condition if (!bFirst).
                        // I think this should mean "do not call CalcAndSetScale"
                        // if the frame is formatted for the first time.
                        // Unfortunately this is not valid anymore since the
                        // SwNoTxtFrm already gets a width during CalcLowerPreps.
                        // Nevertheless, the indention of !bFirst seemed to be
                        // to assure that the OLE objects have already been notified
                        // if necessary before calling CalcAndSetScale.
                        // So I replaced !bFirst by !IsOLESizeInvalid. There is
                        // one additional problem specific to the word import:
                        // The layout is calculated _before_ calling PrtOLENotify,
                        // and the OLE objects are not invalidated during import.
                        // Therefore I added the condition !IsUpdateExpFld,
                        // have a look at the occurence of CalcLayout in
                        // uiview/view.cxx.
/*N*/                         if ( !pNd->IsOLESizeInvalid() &&
/*N*/                              !pSh->GetDoc()->IsUpdateExpFld() )
/*N*/ 							pFESh->CalcAndSetScale( xObj, &pFly->Prt(), &pFly->Frm());
/*N*/ 					}
/*N*/ 					pTmp = (ViewShell*)pTmp->GetNext();
/*N*/ 				} while ( pTmp != pSh );
/*N*/ 
/*N*/ 				if ( pFESh && pNd->IsOLESizeInvalid() )
/*N*/ 				{
/*N*/ 					pNd->SetOLESizeInvalid( FALSE );
/*N*/ 					xObj->OnDocumentPrinterChanged( pNd->GetDoc()->GetPrt() );
/*N*/ 					pFESh->CalcAndSetScale( xObj );//Client erzeugen lassen.
/*N*/ 				}
/*N*/ 			}
/*N*/ 			//dito Animierte Grafiken
/*N*/ 			if ( Frm().HasArea() && ((SwNoTxtFrm*)pCnt)->HasAnimation() )
/*N*/ 			{
/*?*/ 				((SwNoTxtFrm*)pCnt)->StopAnimation();
/*?*/ 				pSh->InvalidateWindows( Frm() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bFirst )
/*N*/ 	{
/*N*/ 		pCnt->SetRetouche();	//fix(13870)
/*N*/ 
/*N*/ 		SwDoc *pDoc = pCnt->GetNode()->GetDoc();
/*N*/ 		if ( pDoc->GetSpzFrmFmts()->Count() &&
/*N*/ 			 !pDoc->IsLoaded() && !pDoc->IsNewDoc() )
/*N*/ 		{
/*N*/ 			//Der Frm wurde wahrscheinlich zum ersten mal formatiert.
/*N*/ 			//Wenn ein Filter Flys oder Zeichenobjekte einliest und diese
/*N*/ 			//Seitengebunden sind, hat er ein Problem, weil er i.d.R. die
/*N*/ 			//Seitennummer nicht kennt. Er weiss lediglich welches der Inhalt
/*N*/ 			//(CntntNode) an dieser Stelle ist.
/*N*/ 			//Die Filter stellen dazu das Ankerattribut der Objekte so ein, dass
/*N*/ 			//sie vom Typ zwar Seitengebunden sind, aber der Index des Ankers
/*N*/ 			//auf diesen CntntNode zeigt.
/*N*/ 			//Hier werden diese vorlauefigen Verbindungen aufgeloest.
/*N*/ 
/*N*/ 			const SwPageFrm *pPage = 0;
/*N*/ 			SwNodeIndex   *pIdx  = 0;
/*N*/ 			SwSpzFrmFmts *pTbl = pDoc->GetSpzFrmFmts();
/*N*/ 
/*N*/ 			for ( USHORT i = 0; i < pTbl->Count(); ++i )
/*N*/ 			{
/*N*/ 				if ( !pPage )
/*N*/ 					pPage = pCnt->FindPageFrm();
/*N*/ 				SwFrmFmt *pFmt = (*pTbl)[i];
/*N*/ 				const SwFmtAnchor &rAnch = pFmt->GetAnchor();
/*N*/ 
/*N*/ 				if ( FLY_PAGE 		!= rAnch.GetAnchorId() &&
/*N*/ 					 FLY_AT_CNTNT	!= rAnch.GetAnchorId() )
/*N*/ 					continue;	//#60878# nicht etwa zeichengebundene.
/*N*/ 
/*N*/ 				FASTBOOL bCheckPos = FALSE;
/*N*/ 				if ( rAnch.GetCntntAnchor() )
/*N*/ 				{
/*N*/ 					if ( !pIdx )
/*N*/ 					{
/*N*/ 						pIdx = new SwNodeIndex( *pCnt->GetNode() );
/*N*/ 					}
/*N*/ 					if ( rAnch.GetCntntAnchor()->nNode == *pIdx )
/*N*/ 					{
/*N*/ 						bCheckPos = TRUE;
/*N*/ 						if ( FLY_PAGE == rAnch.GetAnchorId() )
/*N*/ 						{
/*?*/ 							SwFmtAnchor aAnch( rAnch );
/*?*/ 							aAnch.SetAnchor( 0 );
/*?*/ 							aAnch.SetPageNum( pPage->GetPhyPageNum() );
/*?*/ 							pFmt->SetAttr( aAnch );
/*?*/ 							if ( RES_DRAWFRMFMT != pFmt->Which() )
/*?*/ 							{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	pFmt->MakeFrms();
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				if ( !bCheckPos || RES_DRAWFRMFMT != pFmt->Which() )
/*N*/ 					continue;
/*N*/ 
/*N*/ 				SdrObject *pObj = pFmt->FindSdrObject();
/*N*/ 				const Point aAktPos( pObj->GetSnapRect().TopLeft() );
/*N*/ 				Point aPos( aAktPos );
/*N*/ 				FASTBOOL bSetPos = FALSE;
/*N*/ 				SwFmtVertOrient *pVert;
/*N*/ 				if ( SFX_ITEM_SET == pFmt->GetAttrSet().GetItemState(
/*N*/ 							RES_VERT_ORIENT, FALSE, (const SfxPoolItem**)&pVert ) )
/*N*/ 				{
/*?*/ 					bSetPos = TRUE;
/*?*/ 					switch ( pVert->GetRelationOrient() )
/*?*/ 					{
/*?*/ 						case REL_PG_FRAME: 		aPos.Y() = pPage->Frm().Top(); break;
/*?*/ 						case REL_PG_PRTAREA:	aPos.Y() = pPage->Frm().Top();
/*?*/ 												aPos.Y() += pPage->Prt().Top();	break;
/*?*/ 						case PRTAREA:			aPos.Y() = pCnt->Frm().Top();
/*?*/ 												aPos.Y() += pCnt->Prt().Top(); break;
/*?*/ 						case FRAME:				aPos.Y() = pCnt->Frm().Top(); break;
/*?*/ 						default:
/*?*/ 							bSetPos = FALSE;
/*?*/ 							ASSERT( !this,"neuer Trick vom WW Reader?" );
/*?*/ 					}
/*?*/ 					aPos.Y() += pVert->GetPos();
/*?*/ 					pFmt->ResetAttr( RES_VERT_ORIENT );
/*N*/ 				}
/*N*/ 				SwFmtHoriOrient *pHori;
/*N*/ 				if ( SFX_ITEM_SET == pFmt->GetAttrSet().GetItemState(
/*N*/ 							RES_HORI_ORIENT, FALSE, (const SfxPoolItem**)&pHori ) )
/*N*/ 				{
/*?*/ 					bSetPos = TRUE;
/*?*/ 					switch ( pHori->GetRelationOrient() )
/*?*/ 					{
/*?*/ 						case REL_PG_FRAME: 		aPos.X() = pPage->Frm().Left(); break;
/*?*/ 						case REL_PG_PRTAREA:	aPos.X() = pPage->Frm().Left();
/*?*/ 												aPos.X() += pPage->Prt().Left();	break;
/*?*/ 						case PRTAREA:
/*?*/ 						case FRAME:
/*?*/ 							// da es fuer den WW95/97 Import ist und die
/*?*/ 							// Horizontal nur Spalten kennen, muss hier die
/*?*/ 							// Spalte gesucht werden. Wenn es keine gibt,
/*?*/ 							// ist es die PrtArea der Seite.
/*?*/ 							{
/*?*/ 								SwFrm* pColFrm = pCnt->FindColFrm();
/*?*/ 								if( pColFrm )
/*?*/ 									aPos.X() = pColFrm->Frm().Left() +
/*?*/ 											   pColFrm->Prt().Left();
/*?*/ 								else
/*?*/ 									aPos.X() = pPage->Frm().Left() +
/*?*/ 											   pPage->Prt().Left();
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						default:
/*?*/ 							bSetPos = FALSE;
/*?*/ 							ASSERT( !this,"neuer Trick vom WW Reader?" );
/*?*/ 					}
/*?*/ 					aPos.X() += pHori->GetPos();
/*?*/ 					pFmt->ResetAttr( RES_HORI_ORIENT );
/*N*/ 				}
/*N*/ 				if ( bSetPos )
/*N*/ 				{
/*?*/ 					aPos -= aAktPos;
/*?*/ 					pObj->Move( Size( aPos.X(), aPos.Y() ) );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			delete pIdx;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	InsertCnt
|*
|*	Beschreibung		Hilfsfunktionen, die friend von irgendwem sind, damit
|*						nicht immer gleich 'ne ganze Klasse befreundet werden
|*						muss.
|*	Ersterstellung		MA 13. Apr. 93
|*	Letzte Aenderung	MA 11. May. 95
|*
|*************************************************************************/

void AppendObjs( const SwSpzFrmFmts *pTbl, ULONG nIndex,
                        SwFrm *pFrm, SwPageFrm *pPage )
{
    for ( USHORT i = 0; i < pTbl->Count(); ++i )
    {
        SwFrmFmt *pFmt = (SwFrmFmt*)(*pTbl)[i];
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        if ( rAnch.GetCntntAnchor() &&
             (rAnch.GetCntntAnchor()->nNode.GetIndex() == nIndex) )
        {
            const bool bFlyAtFly = rAnch.GetAnchorId() == FLY_AT_FLY; // LAYER_IMPL
            //Wird ein Rahmen oder ein SdrObject beschrieben?
            const bool bSdrObj = RES_DRAWFRMFMT == pFmt->Which();
            // OD 23.06.2003 #108784# - append also drawing objects anchored
            // as character.
            const bool bDrawObjInCntnt = bSdrObj &&
                                         rAnch.GetAnchorId() == FLY_IN_CNTNT;

            if( bFlyAtFly ||
                rAnch.GetAnchorId() == FLY_AT_CNTNT ||
                rAnch.GetAnchorId() == FLY_AUTO_CNTNT ||
                bDrawObjInCntnt )
            {
                SdrObject* pSdrObj = 0;
                if ( bSdrObj && 0 == (pSdrObj = pFmt->FindSdrObject()) )
                {
                    ASSERT( !bSdrObj, "DrawObject not found." );
                    pFmt->GetDoc()->DelFrmFmt( pFmt );
                    --i;
                    continue;
                }
                if ( pSdrObj )
                {
                    if ( !pSdrObj->GetPage() )
                    {
                        pFmt->GetDoc()->GetDrawModel()->GetPage(0)->
                                InsertObject(pSdrObj, pSdrObj->GetOrdNumDirect());
                    }
                    // OD 25.06.2003 #108784# - move object to visible layer,
                    // if necessary.
                    if ( !pFmt->GetDoc()->IsVisibleLayerId( pSdrObj->GetLayer() ) )
                    {
                        SdrLayerID nVisibleLayerId =
                            pFmt->GetDoc()->GetVisibleLayerIdByInvisibleOne( pSdrObj->GetLayer() );
                        pSdrObj->SetLayer( nVisibleLayerId );
                    }

                    SwDrawContact *pNew = (SwDrawContact*)GetUserCall(pSdrObj);
                    if( !pNew->GetAnchor() )
                    {
                        pFrm->AppendDrawObj( pNew );
                    }
                    // OD 19.06.2003 #108784# - add 'virtual' drawing object,
                    // if necessary. But control objects have to be excluded.
                    else if ( !CheckControlLayer( pSdrObj ) &&
                              pNew->GetAnchor() != pFrm &&
                              !pNew->GetDrawObjectByAnchorFrm( *pFrm ) )
                    {
                        SwDrawVirtObj* pDrawVirtObj = pNew->AddVirtObj();
                        pFrm->AppendVirtDrawObj( pNew, pDrawVirtObj );
                        pDrawVirtObj->SendRepaintBroadcast();
                    }

                }
                else
                {
                    SwFlyFrm *pFly;
                    if( bFlyAtFly )
                        pFly = new SwFlyLayFrm( (SwFlyFrmFmt*)pFmt, pFrm );
                    else
                        pFly = new SwFlyAtCntFrm( (SwFlyFrmFmt*)pFmt, pFrm );
                    pFly->Lock();
                    pFrm->AppendFly( pFly );
                    pFly->Unlock();
                    if ( pPage )
                        ::binfilter::RegistFlys( pPage, pFly );
                }
            }
        }
    }
}

/*N*/ FASTBOOL MA_FASTCALL lcl_ObjConnected( SwFrmFmt *pFmt )
/*N*/ {
/*N*/ 	SwClientIter aIter( *pFmt );
/*N*/ 	if ( RES_FLYFRMFMT == pFmt->Which() )
/*N*/ 		return 0 != aIter.First( TYPE(SwFlyFrm) );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwDrawContact *pContact;
/*N*/ 		if ( 0 != (pContact = (SwDrawContact*)aIter.First( TYPE(SwDrawContact))))
/*N*/ 			return pContact->GetAnchor() != 0;
/*N*/ 	}
/*?*/ 	return FALSE;
/*N*/ }

/** helper method to determine, if a <SwFrmFmt>, which has an object connected,
    is located in header or footer.

    OD 23.06.2003 #108784#

    @author OD
*/
bool lcl_InHeaderOrFooter( SwFrmFmt& _rFmt )
{
    ASSERT( lcl_ObjConnected( &_rFmt ),
            "::lcl_InHeaderOrFooter(..) - <SwFrmFmt> has no connected object" );

    bool bRetVal = false;

    const SwFmtAnchor& rAnch = _rFmt.GetAnchor();

    if ( rAnch.GetAnchorId() != FLY_PAGE )
    {
        bRetVal = _rFmt.GetDoc()->IsInHeaderFooter( rAnch.GetCntntAnchor()->nNode );
    }

    return bRetVal;
}

/*N*/ void AppendAllObjs( const SwSpzFrmFmts *pTbl )
/*N*/ {
/*N*/ 	//Verbinden aller Objekte, die in der SpzTbl beschrieben sind mit dem
/*N*/ 	//Layout.
/*N*/ 	//Wenn sich nix mehr tut hoeren wir auf. Dann koennen noch Formate
/*N*/ 	//uebrigbleiben, weil wir weder zeichengebunde Rahmen verbinden noch
/*N*/ 	//Objecte die in zeichengebundenen verankert sind.
/*N*/ 
/*N*/ 	SwSpzFrmFmts aCpy( 255, 255 );
/*N*/ 	aCpy.Insert( pTbl, 0 );
/*N*/ 
/*N*/ 	USHORT nOldCnt = USHRT_MAX;
/*N*/ 
/*N*/ 	while ( aCpy.Count() && aCpy.Count() != nOldCnt )
/*N*/ 	{
/*N*/ 		nOldCnt = aCpy.Count();
/*N*/ 		for ( int i = 0; i < int(aCpy.Count()); ++i )
/*N*/ 		{
/*N*/ 			SwFrmFmt *pFmt = (SwFrmFmt*)aCpy[ USHORT(i) ];
/*N*/ 			const SwFmtAnchor &rAnch = pFmt->GetAnchor();
/*N*/ 			FASTBOOL bRemove = FALSE;
/*N*/ 			if ( rAnch.GetAnchorId() == FLY_PAGE || rAnch.GetAnchorId() == FLY_IN_CNTNT )
/*N*/ 				//Seitengebunde sind bereits verankert, zeichengebundene
/*N*/ 				//will ich hier nicht.
/*N*/ 				bRemove = TRUE;
/*N*/             else if ( FALSE == (bRemove = ::binfilter::lcl_ObjConnected( pFmt )) ||
/*N*/                       ::binfilter::lcl_InHeaderOrFooter( *pFmt ) )
/*N*/ 			{
/*N*/             // OD 23.06.2003 #108784# - correction: for objects in header
/*N*/             // or footer create frames, in spite of the fact that an connected
/*N*/             // objects already exists.
/*N*/ 				//Fuer Flys und DrawObjs nur dann ein MakeFrms rufen wenn noch
/*N*/ 				//keine abhaengigen Existieren, andernfalls, oder wenn das
/*N*/ 				//MakeFrms keine abhaengigen erzeugt, entfernen.
/*N*/ 				pFmt->MakeFrms();
/*N*/ 				bRemove = ::binfilter::lcl_ObjConnected( pFmt );
/*N*/ 			}
/*N*/ 			if ( bRemove )
/*N*/ 			{
/*N*/ 				aCpy.Remove( USHORT(i) );
/*N*/ 				--i;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aCpy.Remove( 0, aCpy.Count() );
/*N*/ }

/*M*/ void MA_FASTCALL _InsertCnt( SwLayoutFrm *pLay, SwDoc *pDoc,
/*M*/ 							 ULONG nIndex, BOOL bPages, ULONG nEndIndex,
/*M*/ 							 SwFrm *pPrv )
/*M*/ {
/*M*/ 	const BOOL bOldIdle = pDoc->IsIdleTimerActive();
/*M*/ 	pDoc->StopIdleTimer();
/*M*/ 	const BOOL bOldCallbackActionEnabled = pDoc->GetRootFrm()->IsCallbackActionEnabled();
/*M*/ 	pDoc->GetRootFrm()->SetCallbackActionEnabled( FALSE );
/*M*/ 
/*M*/ 	//Bei der Erzeugung des Layouts wird bPages mit TRUE uebergeben. Dann
/*M*/ 	//werden schon mal alle x Absaetze neue Seiten angelegt. Bei umbruechen
/*M*/ 	//und/oder Pagedescriptorwechseln werden gleich die entsprechenden Seiten
/*M*/ 	//angelegt.
/*M*/ 	//Vorteil ist, das einerseits schon eine annaehernd realistische Zahl von
/*M*/ 	//Seiten angelegt wird, vor allem aber gibt es nicht mehr eine schier
/*M*/ 	//lange Kette von Absaetzen teuer verschoben werden muss, bis sie sich auf
/*M*/ 	//ertraegliches mass reduziert hat.
/*M*/ 	//Wir gehen mal davon aus, da?20 Absaetze auf eine Seite passen
/*M*/ 	//Damit es in extremen Faellen nicht gar so heftig rechenen wir je nach
/*M*/ 	//Node noch etwas drauf.
/*M*/ 	//Wenn in der DocStatistik eine brauchebare Seitenzahl angegeben ist
/*M*/ 	//(wird beim Schreiben gepflegt), so wird von dieser Seitenanzahl
/*M*/ 	//ausgegengen.
/*M*/ 	BOOL bStartPercent = bPages && !nEndIndex &&
/*M*/                         !SfxProgress::GetActiveProgress() &&
/*M*/                         !SfxProgress::GetActiveProgress( pDoc->GetDocShell() );
/*M*/ 
/*M*/ 	SwPageFrm *pPage = pLay->FindPageFrm();
/*M*/ 	const SwSpzFrmFmts *pTbl = pDoc->GetSpzFrmFmts();
/*M*/ 	SwFrm		*pFrm = 0;
/*M*/ 	BOOL   bBreakAfter	 = FALSE;
/*M*/ 
/*M*/     SwActualSection *pActualSection = 0;
/*M*/     SwLayHelper *pPageMaker;
/*M*/ 
/*M*/     //Wenn das Layout erzeugt wird (bPages == TRUE) steuern wir den Progress
/*M*/ 	//an. Flys und DrawObjekte werden dann nicht gleich verbunden, dies
/*M*/ 	//passiert erst am Ende der Funktion.
/*M*/     if ( bPages )
/*M*/     {
/*M*/         // Attention: the SwLayHelper class uses references to the content-,
/*M*/         // page-, layout-frame etc. and may change them!
/*M*/         pPageMaker = new SwLayHelper( pDoc, pFrm, pPrv, pPage, pLay,
/*M*/                 pActualSection, bBreakAfter, nIndex, 0 == nEndIndex );
/*M*/         if( bStartPercent )
/*M*/         {
/*M*/             ULONG nPageCount = pPageMaker->CalcPageCount();
/*M*/             if( nPageCount )
/*M*/             {
/*M*/                 ::binfilter::StartProgress( STR_STATSTR_LAYOUTINIT, 1, nPageCount,
/*M*/                                  pDoc->GetDocShell());
/*M*/                 bObjsDirect = FALSE;
/*M*/             }
/*M*/             else
/*M*/                 bStartPercent = FALSE;
/*M*/         }
/*M*/ 	}
/*M*/     else
/*M*/         pPageMaker = NULL;
/*M*/ 
/*M*/     if( pLay->IsInSct() &&
/*M*/ 		( pLay->IsSctFrm() || pLay->GetUpper() ) ) // Hierdurch werden Frischlinge
/*M*/ 			// abgefangen, deren Flags noch nicht ermittelt werden koennen,
/*M*/ 			// so z.B. beim Einfuegen einer Tabelle
/*M*/ 	{
/*M*/ 		SwSectionFrm* pSct = pLay->FindSctFrm();
/*M*/ 		// Wenn Inhalt in eine Fussnote eingefuegt wird, die in einem spaltigen
/*M*/ 		// Bereich liegt, so darf der spaltige Bereich nicht aufgebrochen werden.
/*M*/ 		// Nur wenn im Innern der Fussnote ein Bereich liegt, ist dies ein
/*M*/ 		// Kandidat fuer pActualSection.
/*M*/ 		// Gleiches gilt fuer Bereiche in Tabellen, wenn innerhalb einer Tabelle
/*M*/ 		// eingefuegt wird, duerfen nur Bereiche, die ebenfalls im Innern liegen,
/*M*/ 		// aufgebrochen werden.
/*M*/ 		if( ( !pLay->IsInFtn() || pSct->IsInFtn() ) &&
/*M*/ 			( !pLay->IsInTab() || pSct->IsInTab() ) )
/*M*/ 		{
/*M*/ 			pActualSection = new SwActualSection( 0, pSct, 0 );
/*M*/ 			ASSERT( !pLay->Lower() || !pLay->Lower()->IsColumnFrm(),
/*M*/ 				"_InsertCnt: Wrong Call" );
/*M*/ 		}
/*M*/ 	}
/*M*/ 
/*M*/     //If a section is "open", the pActualSection points to an SwActualSection.
/*M*/     //If the page breaks, for "open" sections a follow will created.
/*M*/     //For nested sections (which have, however, not a nested layout),
/*M*/     //the SwActualSection class has a member, which points to an upper(section).
/*M*/     //When the "inner" section finishs, the upper will used instead.
/*M*/ 
/*M*/ 	while( TRUE )
/*M*/ 	{
/*M*/ 		SwNode *pNd = pDoc->GetNodes()[nIndex];
/*M*/ 		if ( pNd->IsCntntNode() )
/*M*/ 		{
/*M*/ 			SwCntntNode* pNode = (SwCntntNode*)pNd;
/*M*/ 			pFrm = pNode->IsTxtNode() ? new SwTxtFrm( (SwTxtNode*)pNode ) :
/*M*/ 										pNode->MakeFrm();
/*M*/             if( pPageMaker && pPageMaker->CheckInsert( nIndex )
/*M*/                 && bStartPercent )
/*M*/                 ::binfilter::SetProgressState( pPage->GetPhyPageNum(),pDoc->GetDocShell());
/*M*/ 
/*M*/             pFrm->InsertBehind( pLay, pPrv );
/*M*/ 			pFrm->Frm().Pos() = pLay->Frm().Pos();
/*M*/ 			pFrm->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.
/*M*/ 			pPrv = pFrm;
/*M*/ 
/*M*/ 			if ( pTbl->Count() && bObjsDirect && !bDontCreateObjects )
/*M*/ 				AppendObjs( pTbl, nIndex, pFrm, pPage );
/*M*/ 		}
/*M*/ 		else if ( pNd->IsTableNode() )
/*M*/ 		{	//Sollten wir auf eine Tabelle gestossen sein?
/*M*/ 			SwTableNode *pTblNode = (SwTableNode*)pNd;

            // #108116# loading may produce table structures that GCLines 
            // needs to clean up. To keep table formulas correct, change
            // all table formulas to internal (BOXPTR) representation.
/*N*/             SwTableFmlUpdate aMsgHnt( &pTblNode->GetTable() );
/*N*/             aMsgHnt.eFlags = TBL_BOXPTR;
/*N*/             pDoc->UpdateTblFlds( &aMsgHnt );
/*N*/             pTblNode->GetTable().GCLines();
/*N*/ 
/*M*/ 			pFrm = pTblNode->MakeFrm();
/*M*/ 
/*M*/             if( pPageMaker && pPageMaker->CheckInsert( nIndex )
/*M*/                 && bStartPercent )
/*M*/                 ::binfilter::SetProgressState( pPage->GetPhyPageNum(),pDoc->GetDocShell());
/*M*/ 
/*M*/ 			pFrm->InsertBehind( pLay, pPrv );
/*M*/ 			if ( bObjsDirect && pTbl->Count() )
/*M*/ 				((SwTabFrm*)pFrm)->RegistFlys();
/*M*/ 			pFrm->Frm().Pos() = pLay->Frm().Pos();
/*M*/ 			pFrm->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.
/*M*/ 			pPrv = pFrm;
/*M*/ 			//Index auf den Endnode der Tabellensection setzen.
/*M*/ 			nIndex = pTblNode->EndOfSectionIndex();
/*N*/ 
/*N*/             SwTabFrm* pTmpFrm = (SwTabFrm*)pFrm;
/*N*/             while ( pTmpFrm )
/*N*/             {
/*N*/                 pTmpFrm->CheckDirChange();
/*N*/                 pTmpFrm = pTmpFrm->IsFollow() ? pTmpFrm->FindMaster() : NULL;
/*N*/             }
/*N*/ 
/*M*/ 		}
/*M*/ 		else if ( pNd->IsSectionNode() )
/*M*/ 		{
/*M*/ 			SwSectionNode *pNode = (SwSectionNode*)pNd;
/*M*/ 			if( pNode->GetSection().CalcHiddenFlag() )
/*M*/ 				// ist versteckt, ueberspringe den Bereich
/*M*/ 				nIndex = pNode->EndOfSectionIndex();
/*M*/ 			else
/*M*/ 			{
/*M*/ 				pFrm = pNode->MakeFrm();
/*M*/ 				pActualSection = new SwActualSection( pActualSection,
/*M*/ 												(SwSectionFrm*)pFrm, pNode );
/*M*/ 				if ( pActualSection->GetUpper() )
/*M*/ 				{
/*M*/ 					//Hinter den Upper einsetzen, beim EndNode wird der "Follow"
/*M*/ 					//des Uppers erzeugt.
/*M*/ 					SwSectionFrm *pTmp = pActualSection->GetUpper()->GetSectionFrm();
/*M*/ 					pFrm->InsertBehind( pTmp->GetUpper(), pTmp );
/*N*/                     // OD 25.03.2003 #108339# - direct initialization of section
/*N*/                     // after insertion in the layout
/*N*/                     static_cast<SwSectionFrm*>(pFrm)->Init();
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					pFrm->InsertBehind( pLay, pPrv );
/*N*/                     // OD 25.03.2003 #108339# - direct initialization of section
/*N*/                     // after insertion in the layout
/*N*/                     static_cast<SwSectionFrm*>(pFrm)->Init();
/*N*/ 					if( pPrv && pPrv->IsInFtn() )
/*N*/ 					{
/*N*/ 						if( pPrv->IsSctFrm() )
/*N*/ 							pPrv = ((SwSectionFrm*)pPrv)->ContainsCntnt();
/*N*/ 						if( pPrv && pPrv->IsTxtFrm() )
/*N*/ 							((SwTxtFrm*)pPrv)->Prepare( PREP_QUOVADIS, 0, FALSE );
/*N*/ 					}
/*N*/ 				}
/*N*/                 pFrm->CheckDirChange();
/*N*/ 
/*N*/                 pFrm->Frm().Pos() = pLay->Frm().Pos();
/*N*/ 				pFrm->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.
/*N*/                 // OD 20.11.2002 #105405# - no page, no invalidate.
/*N*/                 if ( pPage )
/*N*/                 {
/*N*/                     // OD 18.09.2002 #100522#
/*N*/                     // invalidate page in order to force format and paint of
/*N*/                     // inserted section frame
/*N*/                     pFrm->InvalidatePage( pPage );
/*N*/                     // OD 14.11.2002 #104684# - invalidate page content in order to
/*N*/                     // force format and paint of section content.
/*N*/                     pPage->InvalidateCntnt();
/*N*/                 }
/*N*/ 
/*N*/ 				pLay = (SwLayoutFrm*)pFrm;
/*N*/ 				if ( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
/*N*/ 					pLay = pLay->GetNextLayoutLeaf();
/*N*/ 				pPrv = 0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if ( pNd->IsEndNode() && pNd->FindStartNode()->IsSectionNode() )
/*N*/ 		{
/*N*/ 			ASSERT( pActualSection, "Sectionende ohne Anfang?" );
/*N*/ 			ASSERT( pActualSection->GetSectionNode() == pNd->FindStartNode(),
/*N*/ 							"Sectionende mit falschen Start Node?" );
/*N*/ 
/*N*/ 			//Section schliessen, ggf. die umgebende Section wieder
/*N*/ 			//aktivieren.
/*N*/ 			SwActualSection *pTmp = pActualSection->GetUpper();
/*N*/ 			delete pActualSection;
/*N*/ 			pLay = pLay->FindSctFrm();
/*N*/ 			if ( 0 != (pActualSection = pTmp) )
/*N*/ 			{
/*N*/ 				//Koennte noch sein, das der letzte SectionFrm leer geblieben
/*N*/ 				//ist. Dann ist es jetzt an der Zeit ihn zu entfernen.
/*N*/ 				if ( !pLay->ContainsCntnt() )
/*N*/ 				{
/*N*/ 					SwFrm *pTmp = pLay;
/*N*/ 					pLay = pTmp->GetUpper();
/*N*/ 					pPrv = pTmp->GetPrev();
/*N*/ 					pTmp->Remove();
/*N*/ 					delete pTmp;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					pPrv = pLay;
/*N*/ 					pLay = pLay->GetUpper();
/*N*/ 				}
/*N*/ 
/*N*/                 // new section frame
/*N*/                 pFrm = pActualSection->GetSectionNode()->MakeFrm();
/*N*/                 pFrm->InsertBehind( pLay, pPrv );
/*N*/                 static_cast<SwSectionFrm*>(pFrm)->Init();
/*N*/ 
/*N*/                 pFrm->Frm().Pos() = pLay->Frm().Pos();
/*N*/                 pFrm->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.
/*N*/ 
/*N*/                 SwSectionFrm* pOuterSectionFrm = pActualSection->GetSectionFrm();
/*N*/ 
/*N*/                 // a follow has to be appended to the new section frame
/*N*/                 SwSectionFrm* pFollow = pOuterSectionFrm->GetFollow();
/*N*/                 if ( pFollow )
/*N*/                 {
/*N*/                     pOuterSectionFrm->SetFollow( NULL );
/*N*/                     pOuterSectionFrm->InvalidateSize();
/*N*/                     ((SwSectionFrm*)pFrm)->SetFollow( pFollow );
/*N*/                 }
/*N*/ 
/*N*/ 				// Wir wollen keine leeren Teile zuruecklassen
/*N*/                 if( ! pOuterSectionFrm->IsColLocked() &&
/*N*/                     ! pOuterSectionFrm->ContainsCntnt() )
/*N*/ 				{
/*N*/                     pOuterSectionFrm->DelEmpty( TRUE );
/*N*/                     delete pOuterSectionFrm;
/*N*/ 				}
/*N*/ 				pActualSection->SetSectionFrm( (SwSectionFrm*)pFrm );
/*N*/ 
/*N*/ 				pLay = (SwLayoutFrm*)pFrm;
/*N*/ 				if ( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
/*N*/ 					pLay = pLay->GetNextLayoutLeaf();
/*N*/ 				pPrv = 0;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				//Nix mehr mit Sections, es geht direkt hinter dem SectionFrame
/*N*/ 				//weiter.
/*N*/ 				pPrv = pLay;
/*N*/ 				pLay = pLay->GetUpper();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if( pNd->IsStartNode() &&
/*N*/ 				 SwFlyStartNode	== ((SwStartNode*)pNd)->GetStartNodeType() )
/*N*/ 		{
/*N*/ 			if ( pTbl->Count() && bObjsDirect && !bDontCreateObjects )
/*N*/ 			{
/*N*/ 				SwFlyFrm* pFly = pLay->FindFlyFrm();
/*N*/ 				if( pFly )
/*N*/ 					AppendObjs( pTbl, nIndex, pFly, pPage );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			// Weder Cntnt noch Tabelle noch Section,
/*N*/ 			// also muessen wir fertig sein.
/*N*/ 			break;
/*N*/ 
/*N*/ 		++nIndex;
/*N*/ 		// Der Endnode wird nicht mehr mitgenommen, es muss vom
/*N*/ 		// Aufrufenden (Section/MakeFrms()) sichergestellt sein, dass das Ende
/*N*/ 		// des Bereichs vor dem EndIndex liegt!
/*N*/ 		if ( nEndIndex && nIndex >= nEndIndex )
/*N*/ 			break;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pActualSection )
/*N*/ 	{
/*N*/ 		//Kann passieren, dass noch eine leere (Follow-)Section uebrig geblieben ist.
/*N*/ 		if ( !(pLay = pActualSection->GetSectionFrm())->ContainsCntnt() )
/*N*/ 		{
/*N*/ 			pLay->Remove();
/*N*/ 			delete pLay;
/*N*/ 		}
/*N*/ 		delete pActualSection;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bPages )		//Jetzt noch die Flys verbinden lassen.
/*N*/ 	{
/*N*/ 		if ( !bDontCreateObjects )
/*N*/ 			AppendAllObjs( pTbl );
/*N*/ 		bObjsDirect = TRUE;
/*N*/ 		if ( bStartPercent )
/*N*/ 			::binfilter::EndProgress( pDoc->GetDocShell() );
/*N*/ 	}
/*N*/ 
/*N*/     if( pPageMaker )
/*N*/     {
/*N*/         pPageMaker->CheckFlyCache( pPage );
/*N*/         delete pPageMaker;
/*N*/         if( pDoc->GetLayoutCache() )
/*N*/         {
/*N*/ #ifdef DBG_UTIL
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/             DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pDoc->GetLayoutCache()->CompareLayout( *pDoc );
/*N*/ #endif
/*N*/ #endif
/*N*/             pDoc->GetLayoutCache()->ClearImpl();
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/ 	if ( bOldIdle )
/*N*/ 		pDoc->StartIdleTimer();
/*N*/ 	pDoc->GetRootFrm()->SetCallbackActionEnabled( bOldCallbackActionEnabled );
/*N*/ }


void MakeFrms( SwDoc *pDoc, const SwNodeIndex &rSttIdx,
               const SwNodeIndex &rEndIdx )
{
    bObjsDirect = FALSE;

    SwNodeIndex aTmp( rSttIdx );
    ULONG nEndIdx = rEndIdx.GetIndex();
    SwNode* pNd = pDoc->GetNodes().FindPrvNxtFrmNode( aTmp,
                                            pDoc->GetNodes()[ nEndIdx-1 ]);
    if ( pNd )
    {
        BOOL bApres = aTmp < rSttIdx;
        SwNode2Layout aNode2Layout( *pNd, rSttIdx.GetIndex() );
        SwFrm* pFrm;
        while( 0 != (pFrm = aNode2Layout.NextFrm()) )
        {
            SwLayoutFrm *pUpper = pFrm->GetUpper();
            SwFtnFrm* pFtnFrm = pUpper->FindFtnFrm();
            BOOL bOldLock, bOldFtn;
            if( pFtnFrm )
            {
                bOldFtn = pFtnFrm->IsColLocked();
                pFtnFrm->ColLock();
            }
            else
                bOldFtn = TRUE;
            SwSectionFrm* pSct = pUpper->FindSctFrm();
            // Es sind innerhalb von Fussnoten nur die Bereiche interessant,
            // die in den Fussnoten liegen, nicht etwa die (spaltigen) Bereiche,
            // in denen die Fussnoten(Container) liegen.
            // #109767# Table frame is in section, insert section in cell frame.
            if( pSct && ( pFtnFrm && !pSct->IsInFtn() ) || pUpper->IsCellFrm() )
                pSct = NULL;
            if( pSct )
            {   // damit der SectionFrm nicht zerstoert wird durch pTmp->MoveFwd()
                bOldLock = pSct->IsColLocked();
                pSct->ColLock();
            }
            else
                bOldLock = TRUE;

            // Wenn pFrm sich nicht bewegen kann, koennen wir auch niemanden
            // auf die naechste Seite schieben. Innerhalb eines Rahmens auch
            // nicht ( in der 1. Spalte eines Rahmens waere pFrm Moveable()! )
            // Auch in spaltigen Bereichen in Tabellen waere pFrm Moveable.
            BOOL bMoveNext = nEndIdx - rSttIdx.GetIndex() > 120;
            BOOL bAllowMove = !pFrm->IsInFly() && pFrm->IsMoveable() &&
                 (!pFrm->IsInTab() || pFrm->IsTabFrm() );
            if ( bMoveNext && bAllowMove )
            {
                SwFrm *pMove = pFrm;
                SwFrm *pPrev = pFrm->GetPrev();
                SwFlowFrm *pTmp = SwFlowFrm::CastFlowFrm( pMove );
                ASSERT( pTmp, "Missing FlowFrm" );

                if ( bApres )
                {
                    // Wir wollen, dass der Rest der Seite leer ist, d.h.
                    // der naechste muss auf die naechste Seite wandern.
                    // Dieser kann auch in der naechsten Spalte stehen!
                    ASSERT( !pTmp->HasFollow(), "Follows forbidden" );
                    pPrev = pFrm;
                    // Wenn unser umgebender SectionFrm einen Next besitzt,
                    // so soll dieser ebenfalls gemoved werden!
                    pMove = pFrm->GetIndNext();
                    SwColumnFrm* pCol = (SwColumnFrm*)pFrm->FindColFrm();
                    if( pCol )
                        pCol = (SwColumnFrm*)pCol->GetNext();
                    do
                    {
                        if( pCol && !pMove )
                        {   // Bisher haben wir keinen Nachfolger gefunden
                            // jetzt gucken wir in die naechste Spalte
                            pMove = pCol->ContainsAny();
                            if( pCol->GetNext() )
                                pCol = (SwColumnFrm*)pCol->GetNext();
                            else if( pCol->IsInSct() )
                            {   // Wenn es keine naechste Spalte gibt, wir aber
                                // innerhalb eines spaltigen Bereichs sind,
                                // koennte es noch ausserhalb des Bereich
                                // (Seiten-)Spalten geben
                                pCol = (SwColumnFrm*)pCol->FindSctFrm()->FindColFrm();
                                if( pCol )
                                    pCol = (SwColumnFrm*)pCol->GetNext();
                            }
                            else
                                pCol = NULL;
                        }
                        // Falls hier verschrottete SectionFrms herumgammeln,
                        // muessen diese uebersprungen werden.
                        while( pMove && pMove->IsSctFrm() &&
                               !((SwSectionFrm*)pMove)->GetSection() )
                            pMove = pMove->GetNext();
                    } while( !pMove && pCol );

                    if( pMove )
                    {
                        if ( pMove->IsCntntFrm() )
                            pTmp = (SwCntntFrm*)pMove;
                        else if ( pMove->IsTabFrm() )
                            pTmp = (SwTabFrm*)pMove;
                        else if ( pMove->IsSctFrm() )
                        {
                            pMove = ((SwSectionFrm*)pMove)->ContainsAny();
                            if( pMove )
                                pTmp = SwFlowFrm::CastFlowFrm( pMove );
                            else
                                pTmp = NULL;
                        }
                    }
                    else
                        pTmp = 0;
                }
                else
                {
                    ASSERT( !pTmp->IsFollow(), "Follows really forbidden" );
                    // Bei Bereichen muss natuerlich der Inhalt auf die Reise
                    // geschickt werden.
                    if( pMove->IsSctFrm() )
                    {
                        while( pMove && pMove->IsSctFrm() &&
                               !((SwSectionFrm*)pMove)->GetSection() )
                            pMove = pMove->GetNext();
                        if( pMove && pMove->IsSctFrm() )
                            pMove = ((SwSectionFrm*)pMove)->ContainsAny();
                        if( pMove )
                            pTmp = SwFlowFrm::CastFlowFrm( pMove );
                        else
                            pTmp = NULL;
                    }
                }

                if( pTmp )
                {
                    SwFrm* pOldUp = pTmp->GetFrm()->GetUpper();
                    // MoveFwd==TRUE bedeutet, dass wir auf der gleichen
                    // Seite geblieben sind, wir wollen aber die Seite wechseln,
                    // sofern dies moeglich ist
                    BOOL bOldLock = pTmp->IsJoinLocked();
                    pTmp->LockJoin();
                    while( pTmp->MoveFwd( TRUE, FALSE, TRUE ) )
                    {
                        if( pOldUp == pTmp->GetFrm()->GetUpper() )
                            break;
                        pOldUp = pTmp->GetFrm()->GetUpper();
                    }
                    if( !bOldLock )
                        pTmp->UnlockJoin();
                }
                ::binfilter::_InsertCnt( pUpper, pDoc, rSttIdx.GetIndex(),
                              pFrm->IsInDocBody(), nEndIdx, pPrev );
            }
            else
            {
                BOOL bSplit;
                SwFrm* pPrv = bApres ? pFrm : pFrm->GetPrev();
                // Wenn in einen SectionFrm ein anderer eingefuegt wird,
                // muss dieser aufgebrochen werden
                if( pSct && rSttIdx.GetNode().IsSectionNode() )
                {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
                }
                else
                    bSplit = FALSE;
                ::binfilter::_InsertCnt( pUpper, pDoc, rSttIdx.GetIndex(), FALSE,
                              nEndIdx, pPrv );
                // OD 23.06.2003 #108784# - correction: append objects doesn't
                // depend on value of <bAllowMove>
                if( !bDontCreateObjects )
                {
                    const SwSpzFrmFmts *pTbl = pDoc->GetSpzFrmFmts();
                    if( pTbl->Count() )
                        AppendAllObjs( pTbl );
                }

                // Wenn nichts eingefuegt wurde, z.B. ein ausgeblendeter Bereich,
                // muss das Splitten rueckgaengig gemacht werden
                if( bSplit && pSct && pSct->GetNext()
                    && pSct->GetNext()->IsSctFrm() )
                    pSct->MergeNext( (SwSectionFrm*)pSct->GetNext() );
                if( pFrm->IsInFly() )
                    pFrm->FindFlyFrm()->_Invalidate();
                if( pFrm->IsInTab() )
                    pFrm->InvalidateSize();
            }

            SwPageFrm *pPage = pUpper->FindPageFrm();
            SwFrm::CheckPageDescs( pPage, FALSE );
            if( !bOldFtn )
                pFtnFrm->ColUnlock();
            if( !bOldLock )
            {
                pSct->ColUnlock();
                // Zum Beispiel beim Einfuegen von gelinkten Bereichen,
                // die wiederum Bereiche enthalten, kann pSct jetzt leer sein
                // und damit ruhig zerstoert werden.
                if( !pSct->ContainsCntnt() )
                {
                    pSct->DelEmpty( TRUE );
                    pDoc->GetRootFrm()->RemoveFromList( pSct );
                    delete pSct;
                }
            }
        }
    }

    bObjsDirect = TRUE;
}


/*************************************************************************
|*
|*	SwBorderAttrs::Ctor, DTor
|*
|*	Ersterstellung		MA 19. May. 93
|*	Letzte Aenderung	MA 25. Jan. 97
|*
|*************************************************************************/

/*N*/ SwBorderAttrs::SwBorderAttrs( const SwModify *pMod, const SwFrm *pConstructor ) :
/*N*/ 	SwCacheObj( pMod ),
/*N*/ 	rAttrSet( pConstructor->IsCntntFrm()
/*N*/ 					? ((SwCntntFrm*)pConstructor)->GetNode()->GetSwAttrSet()
/*N*/ 					: ((SwLayoutFrm*)pConstructor)->GetFmt()->GetAttrSet() ),
/*N*/ 	rUL 	( rAttrSet.GetULSpace() ),
/*N*/ 	rLR 	( rAttrSet.GetLRSpace() ),
/*N*/ 	rBox	( rAttrSet.GetBox() 	),
/*N*/ 	rShadow ( rAttrSet.GetShadow()	),
/*N*/ 	aFrmSize( rAttrSet.GetFrmSize().GetSize() )
/*N*/ {
/*N*/ 	//Achtung: Die USHORTs fuer die gecache'ten Werte werden absichtlich
/*N*/ 	//nicht initialisiert!
/*N*/ 
/*N*/ 	//Muessen alle einmal berechnet werden:
/*N*/ 	bTopLine = bBottomLine = bLeftLine = bRightLine =
/*N*/     bTop     = bBottom     = bLine   = TRUE;
/*N*/ 
/*N*/ 	bCacheGetLine = bCachedGetTopLine = bCachedGetBottomLine = FALSE;
/*N*/     // OD 21.05.2003 #108789# - init cache status for values <bJoinedWithPrev>
/*N*/     // and <bJoinedWithNext>, which aren't initialized by default.
/*N*/     bCachedJoinedWithPrev = FALSE;
/*N*/     bCachedJoinedWithNext = FALSE;
/*N*/ 
/*N*/ 	bBorderDist = 0 != (pConstructor->GetType() & (FRM_CELL));
/*N*/ }

/*N*/ SwBorderAttrs::~SwBorderAttrs()
/*N*/ {
/*N*/ 	((SwModify*)pOwner)->SetInCache( FALSE );
/*N*/ }

/*************************************************************************
|*
|*	SwBorderAttrs::CalcTop(), CalcBottom(), CalcLeft(), CalcRight()
|*
|*	Beschreibung		Die Calc-Methoden errechnen zusaetzlich zu den
|*		von den Attributen vorgegebenen Groessen einen Sicherheitsabstand.
|*		der Sicherheitsabstand wird nur einkalkuliert, wenn Umrandung und/oder
|*		Schatten im Spiel sind; er soll vermeiden, dass aufgrund der
|*		groben physikalischen Gegebenheiten Raender usw. uebermalt werden.
|*	Ersterstellung		MA 19. May. 93
|*	Letzte Aenderung	MA 08. Jul. 93
|*
|*************************************************************************/

/*N*/ void SwBorderAttrs::_CalcTop()
/*N*/ {
/*N*/ 	nTop = CalcTopLine() + rUL.GetUpper();
/*N*/ 	bTop = FALSE;
/*N*/ }

/*N*/ void SwBorderAttrs::_CalcBottom()
/*N*/ {
/*N*/ 	nBottom = CalcBottomLine() + rUL.GetLower();
/*N*/ 	bBottom = FALSE;
/*N*/ }

/*N*/ long SwBorderAttrs::CalcRight( const SwFrm* pCaller ) const
/*N*/ {
/*N*/     long nRight;
/*N*/ 
/*N*/     // OD 23.01.2003 #106895# - for cell frame in R2L text direction the left
/*N*/     // and right border are painted on the right respectively left.
/*N*/     if ( pCaller->IsCellFrm() && pCaller->IsRightToLeft() )
/*N*/         nRight = CalcLeftLine();
/*N*/     else
/*N*/         nRight = CalcRightLine();
/*N*/ 
/*N*/     // for paragraphs, "left" is "before text" and "right" is "after text"
/*N*/     if ( pCaller->IsTxtFrm() && pCaller->IsRightToLeft() )
/*N*/         nRight += rLR.GetLeft();
/*N*/     else
/*N*/         nRight += rLR.GetRight();
/*N*/ 
/*N*/     return nRight;
/*N*/ }

/*N*/ long SwBorderAttrs::CalcLeft( const SwFrm *pCaller ) const
/*N*/ {
/*N*/     long nLeft;
/*N*/ 
/*N*/     // OD 23.01.2003 #106895# - for cell frame in R2L text direction the left
/*N*/     // and right border are painted on the right respectively left.
/*N*/     if ( pCaller->IsCellFrm() && pCaller->IsRightToLeft() )
/*N*/         nLeft = CalcRightLine();
/*N*/     else
/*N*/         nLeft = CalcLeftLine();
/*N*/ 
/*N*/     // for paragraphs, "left" is "before text" and "right" is "after text"
/*N*/     if ( pCaller->IsTxtFrm() && pCaller->IsRightToLeft() )
/*N*/         nLeft += rLR.GetRight();
/*N*/     else
/*N*/         nLeft += rLR.GetLeft();
/*N*/ 
/*N*/     if ( pCaller->IsTxtFrm() )
/*N*/         nLeft += ((SwTxtFrm*)pCaller)->GetTxtNode()->GetLeftMarginWithNum();
/*N*/ 
/*N*/     return nLeft;
/*N*/ }

/*************************************************************************
|*
|*	SwBorderAttrs::CalcTopLine(), CalcBottomLine(),
|*				   CalcLeftLine(), CalcRightLine()
|*
|*	Beschreibung		Berechnung der Groessen fuer Umrandung und Schatten.
|* 						Es kann auch ohne Linien ein Abstand erwuenscht sein,
|* 						dieser wird  dann nicht vom Attribut sondern hier
|* 						beruecksichtigt (bBorderDist, z.B. fuer Zellen).
|*	Ersterstellung		MA 21. May. 93
|*	Letzte Aenderung	MA 07. Jun. 99
|*
|*************************************************************************/

/*N*/ void SwBorderAttrs::_CalcTopLine()
/*N*/ {
/*N*/ 	nTopLine = (bBorderDist && !rBox.GetTop())
/*N*/ 							? rBox.GetDistance  (BOX_LINE_TOP)
/*N*/ 							: rBox.CalcLineSpace(BOX_LINE_TOP);
/*N*/ 	nTopLine += rShadow.CalcShadowSpace(SHADOW_TOP);
/*N*/ 	bTopLine = FALSE;
/*N*/ }

/*N*/ void SwBorderAttrs::_CalcBottomLine()
/*N*/ {
/*N*/ 	nBottomLine = (bBorderDist && !rBox.GetBottom())
/*N*/ 							? rBox.GetDistance  (BOX_LINE_BOTTOM)
/*N*/ 							: rBox.CalcLineSpace(BOX_LINE_BOTTOM);
/*N*/ 	nBottomLine += rShadow.CalcShadowSpace(SHADOW_BOTTOM);
/*N*/ 	bBottomLine = FALSE;
/*N*/ }

/*N*/ void SwBorderAttrs::_CalcLeftLine()
/*N*/ {
/*N*/ 	nLeftLine = (bBorderDist && !rBox.GetLeft())
/*N*/ 							? rBox.GetDistance  (BOX_LINE_LEFT)
/*N*/ 							: rBox.CalcLineSpace(BOX_LINE_LEFT);
/*N*/ 	nLeftLine += rShadow.CalcShadowSpace(SHADOW_LEFT);
/*N*/ 	bLeftLine = FALSE;
/*N*/ }

/*N*/ void SwBorderAttrs::_CalcRightLine()
/*N*/ {
/*N*/ 	nRightLine = (bBorderDist && !rBox.GetRight())
/*N*/ 							? rBox.GetDistance  (BOX_LINE_RIGHT)
/*N*/ 							: rBox.CalcLineSpace(BOX_LINE_RIGHT);
/*N*/ 	nRightLine += rShadow.CalcShadowSpace(SHADOW_RIGHT);
/*N*/ 	bRightLine = FALSE;
/*N*/ }

/*************************************************************************
|*
|*	SwBorderAttrs::_IsLine()
|*
|*	Ersterstellung		MA 29. Sep. 94
|*	Letzte Aenderung	MA 29. Sep. 94
|*
|*************************************************************************/


/*************************************************************************
|*
|*	SwBorderAttrs::CmpLeftRightLine(), IsTopLine(), IsBottomLine()
|*
|*		Die Umrandungen benachbarter Absaetze werden nach folgendem
|*		Algorithmus zusammengefasst:
|*
|*		1. Die Umrandung oben faellt weg, wenn der Vorgaenger dieselbe
|*		   Umrandung oben aufweist und 3. Zutrifft.
|*		   Zusaetzlich muss der Absatz mindestens rechts oder links oder
|*		   unten eine Umrandung haben.
|*		2. Die Umrandung unten faellt weg, wenn der Nachfolger dieselbe
|*		   Umrandung untern aufweist und 3. Zustrifft.
|*		   Zusaetzlich muss der Absatz mindestens rechts oder links oder
|*		   oben eine Umrandung haben.
|*		3. Die Umrandungen links und rechts vor Vorgaenger bzw. Nachfolger
|*		   sind identisch.
|*
|*	Ersterstellung		MA 22. Mar. 95
|*	Letzte Aenderung	MA 22. May. 95
|*
|*************************************************************************/
/*N*/ inline int CmpLines( const SvxBorderLine *pL1, const SvxBorderLine *pL2 )
/*N*/ {
/*N*/ 	return ( ((pL1 && pL2) && (*pL1 == *pL2)) || (!pL1 && !pL2) );
/*N*/ }

// OD 21.05.2003 #108789# - change name of 1st parameter - "rAttrs" -> "rCmpAttrs"
// OD 21.05.2003 #108789# - compare <CalcRight()> and <rCmpAttrs.CalcRight()>
//          instead of only the right LR-spacing, because R2L-layout has to be
//          considered.
BOOL SwBorderAttrs::CmpLeftRight( const SwBorderAttrs &rCmpAttrs,
                                  const SwFrm *pCaller,
                                  const SwFrm *pCmp ) const
{
    return ( CmpLines( rCmpAttrs.GetBox().GetLeft(), GetBox().GetLeft()  ) &&
             CmpLines( rCmpAttrs.GetBox().GetRight(),GetBox().GetRight() ) &&
             CalcLeft( pCaller ) == rCmpAttrs.CalcLeft( pCmp ) &&
             // OD 21.05.2003 #108789# - compare <CalcRight> with <rCmpAttrs.CalcRight>.
             CalcRight( pCaller ) == rCmpAttrs.CalcRight( pCmp ) );
}

BOOL SwBorderAttrs::_JoinWithCmp( const SwFrm& _rCallerFrm,
                                  const SwFrm& _rCmpFrm ) const
{
    BOOL bReturnVal = FALSE;

    SwBorderAttrAccess aCmpAccess( SwFrm::GetCache(), &_rCmpFrm );
    const SwBorderAttrs &rCmpAttrs = *aCmpAccess.Get();
    if ( rShadow == rCmpAttrs.GetShadow() &&
         CmpLines( rBox.GetTop(), rCmpAttrs.GetBox().GetTop() ) &&
         CmpLines( rBox.GetBottom(), rCmpAttrs.GetBox().GetBottom() ) &&
         CmpLeftRight( rCmpAttrs, &_rCallerFrm, &_rCmpFrm )
       )
    {
        bReturnVal = TRUE;
    }

    return bReturnVal;
}

// OD 21.05.2003 #108789# - method to determine, if borders are joined with
// previous frame. Calculated value saved in cached value <bJoinedWithPrev>
void SwBorderAttrs::_CalcJoinedWithPrev( const SwFrm& _rFrm )
{
    // set default
    bJoinedWithPrev = FALSE;

    // text frame can potentially join with previous text frame, if
    // corresponding attribute set is set at previous text frame.
    if ( _rFrm.GetPrev() &&
         _rFrm.IsTxtFrm() && _rFrm.GetPrev()->IsTxtFrm() &&
         _rFrm.GetPrev()->GetAttrSet()->GetParaConnectBorder().GetValue()
       )
    {
        bJoinedWithPrev = _JoinWithCmp( _rFrm, *(_rFrm.GetPrev()) );
    }

    // valid cache status, if demanded
    bCachedJoinedWithPrev = bCacheGetLine;
}

// OD 21.05.2003 #108789# - method to determine, if borders are joined with
// next frame. Calculated value saved in cached value <bJoinedWithNext>
void SwBorderAttrs::_CalcJoinedWithNext( const SwFrm& _rFrm )
{
    // set default
    bJoinedWithNext = FALSE;

    // text frame can potentially join with next text frame, if
    // corresponding attribute set is set at current text frame.
    if ( _rFrm.GetNext() &&
         _rFrm.IsTxtFrm() && _rFrm.GetNext()->IsTxtFrm() &&
         _rFrm.GetAttrSet()->GetParaConnectBorder().GetValue()
       )
    {
        bJoinedWithNext = _JoinWithCmp( _rFrm, *(_rFrm.GetNext()) );
    }

    // valid cache status, if demanded
    bCachedJoinedWithNext = bCacheGetLine;
}

// OD 21.05.2003 #108789# - accessor for cached values <bJoinedWithPrev>
BOOL SwBorderAttrs::JoinedWithPrev( const SwFrm& _rFrm ) const
{
    if ( !bCachedJoinedWithPrev )
    {
        const_cast<SwBorderAttrs*>(this)->_CalcJoinedWithPrev( _rFrm );
    }

    return bJoinedWithPrev;
}

BOOL SwBorderAttrs::JoinedWithNext( const SwFrm& _rFrm ) const
{
    if ( !bCachedJoinedWithNext )
    {
        const_cast<SwBorderAttrs*>(this)->_CalcJoinedWithNext( _rFrm );
    }

    return bJoinedWithNext;
}

void SwBorderAttrs::_GetTopLine( const SwFrm *pFrm )
{
    USHORT nRet = CalcTopLine();

    // OD 21.05.2003 #108789# - use new method <JoinWithPrev()>
    if ( JoinedWithPrev( *(pFrm) ) )
    {
        nRet = 0;
    }
    /*
    if ( nRet && pFrm->GetPrev() && pFrm->IsCntntFrm() && pFrm->GetPrev()->IsCntntFrm() )
    {
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm->GetPrev() );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( nRet == rAttrs.CalcTopLine() )
        {
            if ( (GetBox().GetLeft() || GetBox().GetRight() || GetBox().GetBottom()) &&
                 rAttrs.GetShadow() == rShadow	 &&
                 CmpLines( rAttrs.GetBox().GetTop(), rBox.GetTop() ) &&
                 CmpLeftRight( rAttrs, pFrm, pFrm->GetPrev() ) )
            {
                nRet = 0;
            }
        }
    }
    */

    bCachedGetTopLine = bCacheGetLine;

    nGetTopLine = nRet;
}

void SwBorderAttrs::_GetBottomLine( const SwFrm *pFrm )
{
    USHORT nRet = CalcBottomLine();

    // OD 21.05.2003 #108789# - use new method <JoinWithPrev()>
    if ( JoinedWithNext( *(pFrm) ) )
    {
        nRet = 0;
    }
    /*
    if ( nRet && pFrm->GetNext() && pFrm->IsCntntFrm() && pFrm->GetNext()->IsCntntFrm() )
    {
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm->GetNext() );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( nRet == rAttrs.CalcBottomLine() )
        {
            if ( (GetBox().GetLeft() || GetBox().GetRight() || GetBox().GetTop()) &&
                 rAttrs.GetShadow() == rShadow	 &&
                 CmpLines( rAttrs.GetBox().GetBottom(), rBox.GetBottom() ) &&
                 CmpLeftRight( rAttrs, pFrm, pFrm->GetNext() ) )
            {
                nRet = 0;
            }
        }
    }
    */
    bCachedGetBottomLine = bCacheGetLine;

    nGetBottomLine = nRet;
}

/*************************************************************************
|*
|*	SwBorderAttrAccess::CTor
|*
|*	Ersterstellung		MA 20. Mar. 95
|*	Letzte Aenderung	MA 29. Nov. 95
|*
|*************************************************************************/

/*N*/ SwBorderAttrAccess::SwBorderAttrAccess( SwCache &rCache, const SwFrm *pFrm ) :
/*N*/ 	SwCacheAccess( rCache, (pFrm->IsCntntFrm() ?
/*N*/ 								(void*)((SwCntntFrm*)pFrm)->GetNode() :
/*N*/ 								(void*)((SwLayoutFrm*)pFrm)->GetFmt()),
/*N*/ 						   (BOOL)(pFrm->IsCntntFrm() ?
/*N*/ 				(BOOL)((SwModify*)((SwCntntFrm*)pFrm)->GetNode())->IsInCache() :
/*N*/ 				(BOOL)((SwModify*)((SwLayoutFrm*)pFrm)->GetFmt())->IsInCache()) ),
/*N*/ 	pConstructor( pFrm )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	SwBorderAttrAccess::NewObj, Get
|*
|*	Ersterstellung		MA 20. Mar. 95
|*	Letzte Aenderung	MA 20. Mar. 95
|*
|*************************************************************************/

/*N*/ SwCacheObj *SwBorderAttrAccess::NewObj()
/*N*/ {
/*N*/ 	((SwModify*)pOwner)->SetInCache( TRUE );
/*N*/ 	return new SwBorderAttrs( (SwModify*)pOwner, pConstructor );
/*N*/ }

/*N*/ SwBorderAttrs *SwBorderAttrAccess::Get()
/*N*/ {
/*N*/ 	return (SwBorderAttrs*)SwCacheAccess::Get();
/*N*/ }

/*************************************************************************
|*
|*	SwOrderIter::Ctor
|*
|*	Ersterstellung		MA 06. Jan. 95
|*	Letzte Aenderung	MA 22. Nov. 95
|*
|*************************************************************************/

/*N*/ SwOrderIter::SwOrderIter( const SwPageFrm *pPg, FASTBOOL bFlys ) :
/*N*/ 	pPage( pPg ),
/*N*/ 	pCurrent( 0 ),
/*N*/ 	bFlysOnly( bFlys )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	SwOrderIter::Top()
|*
|*	Ersterstellung		MA 06. Jan. 95
|*	Letzte Aenderung	MA 22. Nov. 95
|*
|*************************************************************************/


/*************************************************************************
|*
|*	SwOrderIter::Bottom()
|*
|*	Ersterstellung		MA 06. Jan. 95
|*	Letzte Aenderung	MA 22. Nov. 95
|*
|*************************************************************************/

/*N*/ const SdrObject *SwOrderIter::Bottom()
/*N*/ {
/*N*/ 	pCurrent = 0;
/*N*/ 	if ( pPage->GetSortedObjs() )
/*N*/ 	{
/*N*/ 		UINT32 nBotOrd = USHRT_MAX;
/*N*/ 		const SwSortDrawObjs *pObjs = pPage->GetSortedObjs();
/*N*/ 		if ( pObjs->Count() )
/*N*/ 		{
/*N*/ 			(*pObjs)[0]->GetOrdNum();	//Aktualisieren erzwingen!
/*N*/ 			for ( USHORT i = 0; i < pObjs->Count(); ++i )
/*N*/ 			{
/*N*/ 				SdrObject *pObj = (*pObjs)[i];
/*N*/ 				if ( bFlysOnly && !pObj->IsWriterFlyFrame() )
/*N*/ 					continue;
/*N*/ 				UINT32 nTmp = pObj->GetOrdNumDirect();
/*N*/ 				if ( nTmp < nBotOrd )
/*N*/ 				{
/*N*/ 					nBotOrd = nTmp;
/*N*/ 					pCurrent = pObj;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pCurrent;
/*N*/ }

/*************************************************************************
|*
|*	SwOrderIter::Next()
|*
|*	Ersterstellung		MA 06. Jan. 95
|*	Letzte Aenderung	MA 22. Nov. 95
|*
|*************************************************************************/

/*N*/ const SdrObject *SwOrderIter::Next()
/*N*/ {
/*N*/ 	const UINT32 nCurOrd = pCurrent ? pCurrent->GetOrdNumDirect() : 0;
/*N*/ 	pCurrent = 0;
/*N*/ 	if ( pPage->GetSortedObjs() )
/*N*/ 	{
/*N*/ 		UINT32 nOrd = USHRT_MAX;
/*N*/ 		const SwSortDrawObjs *pObjs = pPage->GetSortedObjs();
/*N*/ 		if ( pObjs->Count() )
/*N*/ 		{
/*N*/ 			(*pObjs)[0]->GetOrdNum();	//Aktualisieren erzwingen!
/*N*/ 			for ( USHORT i = 0; i < pObjs->Count(); ++i )
/*N*/ 			{
/*N*/ 				SdrObject *pObj = (*pObjs)[i];
/*N*/ 				if ( bFlysOnly && !pObj->IsWriterFlyFrame() )
/*N*/ 					continue;
/*N*/ 				UINT32 nTmp = pObj->GetOrdNumDirect();
/*N*/ 				if ( nTmp > nCurOrd && nTmp < nOrd )
/*N*/ 				{
/*N*/ 					nOrd = nTmp;
/*N*/ 					pCurrent = pObj;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pCurrent;
/*N*/ }

/*************************************************************************
|*
|*	SwOrderIter::Prev()
|*
|*	Ersterstellung		MA 06. Jan. 95
|*	Letzte Aenderung	MA 22. Nov. 95
|*
|*************************************************************************/


/*************************************************************************
|*
|*	SaveCntnt(), RestoreCntnt()
|*
|*	Ersterstellung		MA 10. Jun. 93
|*	Letzte Aenderung	MA 07. Mar. 95
|*
|*************************************************************************/

//Unterstruktur eines LayoutFrms fuer eine Aktion aufheben und wieder
//restaurieren.
//Neuer Algorithmus: Es ist unuetz jeden Nachbarn einzeln zu betrachten und
//die Pointer sauber zu setzen (Upper, Nachbarn, usw.)
//Es reicht vollkommen jeweils eine Einzelkette zu loesen, und mit dem
//Letzen der Einzelkette nachzuschauen ob noch eine weitere Kette
//angeheangt werden muss. Es brauchen nur die Pointer korrigiert werden,
//die zur Verkettung notwendig sind. So koennen Beipspielsweise die Pointer
//auf die Upper auf den alten Uppern stehenbleiben. Korrigiert werden die
//Pointer dann im RestoreCntnt. Zwischenzeitlich ist sowieso jeder Zugriff
//verboten.
//Unterwegs werden die Flys bei der Seite abgemeldet.

/*N*/ void MA_FASTCALL lcl_RemoveFlysFromPage( SwCntntFrm *pCntnt )
/*N*/ {
/*N*/ 	ASSERT( pCntnt->GetDrawObjs(), "Keine DrawObjs fuer lcl_RemoveFlysFromPage." );
/*N*/ 	SwDrawObjs &rObjs = *pCntnt->GetDrawObjs();
/*N*/ 	for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pO = rObjs[i];
/*N*/ 		SwVirtFlyDrawObj *pObj = pO->IsWriterFlyFrame() ?
/*N*/ 														(SwVirtFlyDrawObj*)pO : 0;
/*N*/ 		if ( pObj && pObj->GetFlyFrm()->IsFlyFreeFrm() )
/*N*/ 		{
/*N*/ 			SwCntntFrm *pCnt = pObj->GetFlyFrm()->ContainsCntnt();
/*N*/ 			while ( pCnt )
/*N*/ 			{
/*N*/ 				if ( pCnt->GetDrawObjs() )
/*?*/ 					::binfilter::lcl_RemoveFlysFromPage( pCnt );
/*N*/ 				pCnt = pCnt->GetNextCntntFrm();
/*N*/ 			}
/*N*/ 			((SwFlyFreeFrm*)pObj->GetFlyFrm())->GetPage()->
/*N*/ 					SwPageFrm::RemoveFly( pObj->GetFlyFrm() );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ SwFrm *SaveCntnt( SwLayoutFrm *pLay, SwFrm *pStart )
/*N*/ {
/*N*/ 	if( pLay->IsSctFrm() && pLay->Lower() && pLay->Lower()->IsColumnFrm() )
/*?*/ 		lcl_RemoveFtns( (SwColumnFrm*)pLay->Lower(), TRUE, TRUE );
/*N*/ 
/*N*/ 	SwFrm *pSav;
/*N*/ 	if ( 0 == (pSav = pLay->ContainsAny()) )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	if( pSav->IsInFtn() && !pLay->IsInFtn() )
/*N*/ 	{
/*?*/ 		do
/*?*/ 			pSav = pSav->FindNext();
/*?*/ 		while( pSav && pSav->IsInFtn() );
/*?*/ 		if( !pSav || !pLay->IsAnLower( pSav ) )
/*?*/ 			return NULL;
/*N*/ 	}
/*N*/ 	// Tabellen sollen immer komplett gesichert werden, es sei denn, es wird
/*N*/ 	// der Inhalt eines Bereichs innerhalb einer Tabelle gesichert.
/*N*/ 	if ( pSav->IsInTab() && !( pLay->IsSctFrm() && pLay->IsInTab() ) )
/*?*/ 		while ( !pSav->IsTabFrm() )
/*?*/ 			pSav = pSav->GetUpper();
/*N*/ 
/*N*/ 	if( pSav->IsInSct() )
/*N*/ 	{ // Jetzt wird der oberste Bereich gesucht, der innerhalb von pLay ist.
/*N*/ 		SwFrm* pSect = pLay->FindSctFrm();
/*N*/ 		SwFrm *pTmp = pSav;
/*N*/ 		do
/*N*/ 		{
/*N*/ 			pSav = pTmp;
/*N*/ 			pTmp = pSav->GetUpper() ? pSav->GetUpper()->FindSctFrm() : NULL;
/*N*/ 		} while ( pTmp != pSect );
/*N*/ 	}
/*N*/ 
/*N*/ 	SwFrm *pFloat = pSav;
/*N*/ 	if( !pStart )
/*N*/ 		pStart = pSav;
/*N*/ 	BOOL bGo = pStart == pSav;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		if( bGo )
/*N*/ 			pFloat->GetUpper()->pLower = 0; 	//Die Teilkette ausklinken.
/*N*/ 
/*N*/ 		//Das Ende der Teilkette suchen, unterwegs die Flys abmelden.
/*N*/ 		do
/*N*/ 		{
/*N*/ 			if( bGo )
/*N*/ 			{
/*N*/ 				if ( pFloat->IsCntntFrm() )
/*N*/ 				{
/*N*/ 					if ( pFloat->GetDrawObjs() )
/*N*/ 						::binfilter::lcl_RemoveFlysFromPage( (SwCntntFrm*)pFloat );
/*N*/ 				}
/*N*/ 				else if ( pFloat->IsTabFrm() || pFloat->IsSctFrm() )
/*N*/ 				{
/*N*/ 					SwCntntFrm *pCnt = ((SwLayoutFrm*)pFloat)->ContainsCntnt();
/*N*/ 					if( pCnt )
/*N*/ 					{
/*N*/ 						do
/*N*/ 						{	if ( pCnt->GetDrawObjs() )
/*?*/ 								::binfilter::lcl_RemoveFlysFromPage( pCnt );
/*N*/ 							pCnt = pCnt->GetNextCntntFrm();
/*N*/ 						} while ( pCnt && ((SwLayoutFrm*)pFloat)->IsAnLower( pCnt ) );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*?*/ 					ASSERT( !pFloat, "Neuer Float-Frame?" );
/*N*/ 			}
/*N*/ 			if ( pFloat->GetNext()	)
/*N*/ 			{
/*N*/ 				if( bGo )
/*N*/ 					pFloat->pUpper = NULL;
/*N*/ 				pFloat = pFloat->GetNext();
/*N*/ 				if( !bGo && pFloat == pStart )
/*N*/ 				{
/*?*/ 					bGo = TRUE;
/*?*/ 					pFloat->pPrev->pNext = NULL;
/*?*/ 					pFloat->pPrev = NULL;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				break;
/*N*/ 
/*N*/ 		} while ( pFloat );
/*N*/ 
/*N*/ 		//Die naechste Teilkette suchen und die Ketten miteinander verbinden.
/*N*/ 		SwFrm *pTmp = pFloat->FindNext();
/*N*/ 		if( bGo )
/*N*/ 			pFloat->pUpper = NULL;
/*N*/ 
/*N*/ 		if( !pLay->IsInFtn() )
/*N*/ 			while( pTmp && pTmp->IsInFtn() )
/*?*/ 				pTmp = pTmp->FindNext();
/*N*/ 
/*N*/ 		if ( !pLay->IsAnLower( pTmp ) )
/*N*/ 			pTmp = 0;
/*N*/ 
/*N*/ 		if ( pTmp && bGo )
/*N*/ 		{
/*N*/ 			pFloat->pNext = pTmp;			//Die beiden Ketten verbinden.
/*N*/ 			pFloat->pNext->pPrev = pFloat;
/*N*/ 		}
/*N*/ 		pFloat = pTmp;
/*N*/ 		bGo = bGo || ( pStart == pFloat );
/*N*/ 	}  while ( pFloat );
/*N*/ 
/*N*/ 	return bGo ? pStart : NULL;
/*N*/ }

/*N*/ void MA_FASTCALL lcl_AddFlysToPage( SwCntntFrm *pCntnt, SwPageFrm *pPage )
/*N*/ {
/*N*/ 	ASSERT( pCntnt->GetDrawObjs(), "Keine DrawObjs fuer lcl_AddFlysToPage." );
/*N*/ 	SwDrawObjs &rObjs = *pCntnt->GetDrawObjs();
/*N*/ 	for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pO = rObjs[i];
/*N*/ 		SwVirtFlyDrawObj *pObj = pO->IsWriterFlyFrame() ?
/*N*/ 														(SwVirtFlyDrawObj*)pO : 0;
/*N*/ 		if ( pObj && pObj->GetFlyFrm()->IsFlyFreeFrm() )
/*N*/ 		{
/*N*/ 			SwFlyFrm *pFly = pObj->GetFlyFrm();
/*N*/ 			pPage->SwPageFrm::AppendFly( pFly );
/*N*/ 			pFly->_InvalidatePos();
/*N*/ 			pFly->_InvalidateSize();
/*N*/ 			pFly->InvalidatePage( pPage );
/*N*/ 			SwCntntFrm *pCnt = pFly->ContainsCntnt();
/*N*/ 			while ( pCnt )
/*N*/ 			{
/*N*/ 				if ( pCnt->GetDrawObjs() )
/*?*/ 					::binfilter::lcl_AddFlysToPage( pCnt, pPage );
/*N*/ 				pCnt = pCnt->GetNextCntntFrm();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void RestoreCntnt( SwFrm *pSav, SwLayoutFrm *pParent, SwFrm *pSibling )
/*N*/ {
/*N*/ 	ASSERT( pSav && pParent, "Kein Save oder Parent fuer Restore." );
/*N*/ 
/*N*/ 	//Wenn es bereits FlowFrms unterhalb des neuen Parent gibt, so wird die
/*N*/ 	//Kette, beginnend mit pSav,  hinter dem letzten angehaengt.
/*N*/ 	//Die Teile werden kurzerhand insertet und geeignet invalidiert.
/*N*/ 	//Unterwegs werden die Flys der CntntFrms bei der Seite angemeldet.
/*N*/ 
/*N*/ 	SwPageFrm *pPage = pParent->FindPageFrm();
/*N*/ 
/*N*/ 	if ( pPage )
/*N*/ 		pPage->InvalidatePage( pPage ); //Invalides Layout anmelden.
/*N*/ 
/*N*/ 	//Vorgaenger festellen und die Verbindung herstellen bzw. initialisieren.
/*N*/ 	pSav->pPrev = pSibling;
/*N*/ 	SwFrm* pNxt;
/*N*/ 	if ( pSibling )
/*N*/ 	{
/*N*/ 		pNxt = pSibling->pNext;
/*N*/ 		pSibling->pNext = pSav;
/*N*/ 		pSibling->_InvalidatePrt();
/*N*/ 		((SwCntntFrm*)pSibling)->InvalidatePage( pPage );//Invaliden Cntnt anmelden.
/*N*/ 		if ( ((SwCntntFrm*)pSibling)->GetFollow() )
/*?*/ 			pSibling->Prepare( PREP_CLEAR, 0, sal_False );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	pNxt = pParent->pLower;
/*N*/ 		pParent->pLower = pSav;
/*N*/ 		pSav->pUpper = pParent;		//Schon mal setzen, sonst ist fuer das
/*N*/ 									//invalidate der Parent (z.B. ein Fly) nicht klar.
/*N*/ 		//Invaliden Cntnt anmelden.
/*N*/ 		if ( pSav->IsCntntFrm() )
/*N*/ 			((SwCntntFrm*)pSav)->InvalidatePage( pPage );
/*N*/ 		else
/*N*/ 		{   // pSav koennte auch ein leerer SectFrm sein
/*N*/ 			SwCntntFrm* pCnt = pParent->ContainsCntnt();
/*N*/ 			if( pCnt )
/*N*/ 				pCnt->InvalidatePage( pPage );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//Der Parent muss entsprechend gegrow'ed werden.
/*N*/ 	SwTwips nGrowVal = 0;
/*N*/ 	SwFrm* pLast;
/*N*/ 	do
/*N*/ 	{	pSav->pUpper = pParent;
/*N*/ 		nGrowVal += pSav->Frm().Height();
/*N*/ 		pSav->_InvalidateAll();
/*N*/ 
/*N*/         //Jetzt die Flys anmelden, fuer TxtFrms gleich geeignet invalidieren.
/*N*/ 		if ( pSav->IsCntntFrm() )
/*N*/ 		{
/*N*/ 			if ( pSav->IsTxtFrm() &&
/*N*/ 				 ((SwTxtFrm*)pSav)->GetCacheIdx() != USHRT_MAX )
/*N*/ 				((SwTxtFrm*)pSav)->Init();	//Ich bin sein Freund.
/*N*/ 
/*N*/ 			if ( pPage && pSav->GetDrawObjs() )
/*N*/ 				::binfilter::lcl_AddFlysToPage( (SwCntntFrm*)pSav, pPage );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{	SwCntntFrm *pBlub = ((SwLayoutFrm*)pSav)->ContainsCntnt();
/*N*/ 			if( pBlub )
/*N*/ 			{
/*N*/ 				do
/*N*/ 				{	if ( pPage && pBlub->GetDrawObjs() )
/*?*/ 						::binfilter::lcl_AddFlysToPage( pBlub, pPage );
/*N*/ 					if( pBlub->IsTxtFrm() && ((SwTxtFrm*)pBlub)->HasFtn() &&
/*N*/ 				 		((SwTxtFrm*)pBlub)->GetCacheIdx() != USHRT_MAX )
/*?*/ 						((SwTxtFrm*)pBlub)->Init();	//Ich bin sein Freund.
/*N*/ 					pBlub = pBlub->GetNextCntntFrm();
/*N*/ 				} while ( pBlub && ((SwLayoutFrm*)pSav)->IsAnLower( pBlub ));
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pLast = pSav;
/*N*/ 		pSav = pSav->GetNext();
/*N*/ 
/*N*/ 	} while ( pSav );
/*N*/ 
/*N*/ 	if( pNxt )
/*N*/ 	{
/*?*/ 		pLast->pNext = pNxt;
/*?*/ 		pNxt->pPrev = pLast;
/*N*/ 	}
/*N*/     pParent->Grow( nGrowVal PHEIGHT );
/*N*/ }

/*************************************************************************
|*
|*	SqRt()				Berechnung der Quadratwurzel, damit die math.lib
|*		nicht auch noch dazugelinkt werden muss.
|*
|*	Ersterstellung		OK ??
|*	Letzte Aenderung	MA 09. Jan. 97
|*
|*************************************************************************/

/*N*/ ULONG MA_FASTCALL SqRt( BigInt nX )
/*N*/ {
/*N*/ 	BigInt nErg = 1;
/*N*/ 
/*N*/ 	if ( !nX.IsNeg() )
/*N*/ 	{
/*N*/ 		BigInt nOldErg = 1;
/*N*/ 		for ( int i = 0; i <= 5; i++ )
/*N*/ 		{
/*N*/ 			nErg = (nOldErg + (nX / nOldErg)) / BigInt(2);
/*N*/ 			nOldErg = nErg;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nErg >= BigInt(SAL_MAX_UINT32) ? ULONG_MAX : (ULONG)nErg;
/*N*/ }

/*************************************************************************
|*
|*	InsertNewPage() 	Einsetzen einer neuen Seite.
|*
|*	Ersterstellung		MA 01. Jul. 93
|*	Letzte Aenderung	MA 31. Jul. 95
|*
|*************************************************************************/

/*N*/ SwPageFrm * MA_FASTCALL InsertNewPage( SwPageDesc &rDesc, SwFrm *pUpper,
/*N*/ 						  BOOL bOdd, BOOL bInsertEmpty, BOOL bFtn,
/*N*/ 						  SwFrm *pSibling )
/*N*/ {
/*N*/ 	SwPageFrm *pRet;
/*N*/ 	SwDoc *pDoc = ((SwLayoutFrm*)pUpper)->GetFmt()->GetDoc();
/*N*/ 	SwFrmFmt *pFmt = bOdd ? rDesc.GetRightFmt() : rDesc.GetLeftFmt();
/*N*/ 	//Wenn ich kein FrmFmt fuer die Seite gefunden habe, muss ich eben
/*N*/ 	//eine Leerseite einfuegen.
/*N*/ 	if ( !pFmt )
/*N*/ 	{
/*N*/ 		pFmt = bOdd ? rDesc.GetLeftFmt() : rDesc.GetRightFmt();
/*N*/ 		ASSERT( pFmt, "Descriptor without any format?!" );
/*N*/ 		bInsertEmpty = !bInsertEmpty;
/*N*/ 	}
/*N*/ 	if( bInsertEmpty )
/*N*/ 	{
/*N*/ 		SwPageDesc *pTmpDesc = pSibling && pSibling->GetPrev() ?
/*N*/ 				((SwPageFrm*)pSibling->GetPrev())->GetPageDesc() : &rDesc;
/*N*/ 		pRet = new SwPageFrm( pDoc->GetEmptyPageFmt(), pTmpDesc );
/*N*/ 		pRet->Paste( pUpper, pSibling );
/*N*/ 		pRet->PreparePage( bFtn );
/*N*/ 	}
/*N*/ 	pRet = new SwPageFrm( pFmt, &rDesc );
/*N*/ 	pRet->Paste( pUpper, pSibling );
/*N*/ 	pRet->PreparePage( bFtn );
/*N*/ 	if ( pRet->GetNext() )
/*?*/ 		((SwRootFrm*)pRet->GetUpper())->AssertPageFlys( pRet );
/*N*/ 	return pRet;
/*N*/ }


/*************************************************************************
|*
|*	RegistFlys(), Regist()	Die beiden folgenden Methoden durchsuchen rekursiv
|*		eine Layoutstruktur und melden alle FlyFrms, die einen beliebigen Frm
|*		innerhalb der Struktur als Anker haben bei der Seite an.
|*
|*	Ersterstellung		MA 08. Jul. 93
|*	Letzte Aenderung	MA 07. Jul. 95
|*
|*************************************************************************/

/*N*/ void MA_FASTCALL lcl_Regist( SwPageFrm *pPage, const SwFrm *pAnch )
/*N*/ {
/*N*/ 	SwDrawObjs *pObjs = (SwDrawObjs*)pAnch->GetDrawObjs();
/*N*/ 	for ( USHORT i = 0; i < pObjs->Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pObj = (*pObjs)[i];
/*N*/ 		SwVirtFlyDrawObj *pFObj = pObj->IsWriterFlyFrame() ?
/*N*/ 													(SwVirtFlyDrawObj*)pObj : 0;
/*N*/ 		if ( pFObj )
/*N*/ 		{
/*N*/ 			SwFlyFrm *pFly = pFObj->GetFlyFrm();
/*N*/ 			//Ggf. ummelden, nicht anmelden wenn bereits bekannt.
/*N*/             SwPageFrm *pPg = pFly->IsFlyFreeFrm() ?
/*N*/                         ((SwFlyFreeFrm*)pFly)->GetPage() : pFly->FindPageFrm();
/*N*/ 			if ( pPg != pPage )
/*N*/ 			{
/*N*/ 				if ( pPg )
/*N*/ 					pPg->SwPageFrm::RemoveFly( pFly );
/*N*/ 				pPage->AppendFly( pFly );
/*N*/ 			}
/*N*/ 			::binfilter::RegistFlys( pPage, pFly );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/             SwDrawContact* pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
/*N*/             // OD 20.06.2003 #108784# - consider 'virtual' drawing objects
/*N*/             if ( pObj->ISA(SwDrawVirtObj) )
/*N*/             {
/*N*/                 SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pObj);
/*N*/                 if ( pDrawVirtObj->GetPageFrm() != pPage )
/*N*/                 {
/*N*/                     if ( pDrawVirtObj->GetPageFrm() )
/*N*/                     {
/*N*/                         pDrawVirtObj->GetPageFrm()->RemoveVirtDrawObj( pContact, pDrawVirtObj );
/*N*/                     }
/*N*/                     pPage->AppendVirtDrawObj( pContact, pDrawVirtObj );
/*N*/                 }
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 if ( pContact->GetPage() != pPage )
/*N*/                 {
/*N*/                     if ( pContact->GetPage() )
/*N*/                         pContact->GetPage()->SwPageFrm::RemoveDrawObj( pContact );
/*N*/                     pPage->AppendDrawObj( pContact );
/*N*/                 }
/*N*/             }
/*N*/ 		}
/*N*/ 
/*N*/ 		const SwFlyFrm *pFly = pAnch->FindFlyFrm();
/*N*/         if( pFly && pObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() &&
/*N*/             pObj->GetPage() )
/*N*/ 			pObj->GetPage()->SetObjectOrdNum( pObj->GetOrdNumDirect(),
/*N*/ 									pFly->GetVirtDrawObj()->GetOrdNumDirect() + 1 );
/*N*/ 	}
/*N*/ }

/*N*/ void RegistFlys( SwPageFrm *pPage, const SwLayoutFrm *pLay )
/*N*/ {
/*N*/ 	if ( pLay->GetDrawObjs() )
/*?*/ 		::binfilter::lcl_Regist( pPage, pLay );
/*N*/ 	const SwFrm *pFrm = pLay->Lower();
/*N*/ 	while ( pFrm )
/*N*/ 	{
/*N*/ 		if ( pFrm->IsLayoutFrm() )
/*N*/ 			::binfilter::RegistFlys( pPage, (const SwLayoutFrm*)pFrm );
/*N*/ 		else if ( pFrm->GetDrawObjs() )
/*N*/ 			::binfilter::lcl_Regist( pPage, pFrm );
/*N*/ 		pFrm = pFrm->GetNext();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	void Notify()
|*
|*	Beschreibung		Benachrichtigt den Hintergrund je nach der
|*		Veraenderung zwischen altem und neuem Rechteckt.
|*	Ersterstellung		MA 18. Jun. 93
|*	Letzte Aenderung	MA 06. Jun. 96
|*
|*************************************************************************/

/*N*/ void Notify( SwFlyFrm *pFly, SwPageFrm *pOld, const SwRect &rOld )
/*N*/ {
/*N*/ 	const SwRect aFrm( pFly->AddSpacesToFrm() );
/*N*/ 	if ( rOld.Pos() != aFrm.Pos() )
/*N*/ 	{	//Positionsaenderung, alten und neuen Bereich invalidieren
/*N*/ 		if ( rOld.HasArea() &&
/*N*/ 			 rOld.Left()+pFly->GetFmt()->GetLRSpace().GetLeft() < WEIT_WECH )
/*N*/ 		{
/*N*/ 			pFly->NotifyBackground( pOld, rOld, PREP_FLY_LEAVE );
/*N*/ 		}
/*N*/ 		pFly->NotifyBackground( pFly->FindPageFrm(), aFrm, PREP_FLY_ARRIVE );
/*N*/ 	}
/*N*/ 	else if ( rOld.SSize() != aFrm.SSize() )
/*N*/ 	{	//Groessenaenderung, den Bereich der Verlassen wurde bzw. jetzt
/*N*/ 		//ueberdeckt wird invalidieren.
/*N*/ 		//Der Einfachheit halber wird hier bewusst jeweils ein Twip
/*N*/ 		//unnoetig invalidiert.
/*N*/ 
/*N*/ 		ViewShell *pSh = pFly->GetShell();
/*N*/ 		if( pSh && rOld.HasArea() )
/*N*/ 			pSh->InvalidateWindows( rOld );
/*N*/ 
/*N*/ 		if ( rOld.Left() != aFrm.Left() )
/*?*/ 		{	SwRect aTmp( rOld );
/*?*/ 			aTmp.Union( aFrm );
/*?*/ 			aTmp.Left(	Min(aFrm.Left(), rOld.Left()) );
/*?*/ 			aTmp.Right( Max(aFrm.Left(), rOld.Left()) );
/*?*/ 			pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
/*N*/ 		}
/*N*/ 		SwTwips nOld = rOld.Right();
/*N*/ 		SwTwips nNew = aFrm.Right();
/*N*/ 		if ( nOld != nNew )
/*N*/ 		{	SwRect aTmp( rOld );
/*N*/ 			aTmp.Union( aFrm );
/*N*/ 			aTmp.Left(	Min(nNew, nOld) );
/*N*/ 			aTmp.Right( Max(nNew, nOld) );
/*N*/ 			pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
/*N*/ 		}
/*N*/ 		if ( rOld.Top() != aFrm.Top() )
/*?*/ 		{	SwRect aTmp( rOld );
/*?*/ 			aTmp.Union( aFrm );
/*?*/ 			aTmp.Top(	 Min(aFrm.Top(), rOld.Top()) );
/*?*/ 			aTmp.Bottom( Max(aFrm.Top(), rOld.Top()) );
/*?*/ 			pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
/*N*/ 		}
/*N*/ 		nOld = rOld.Bottom();
/*N*/ 		nNew = aFrm.Bottom();
/*N*/ 		if ( nOld != nNew )
/*N*/ 		{	SwRect aTmp( rOld );
/*N*/ 			aTmp.Union( aFrm );
/*N*/ 			aTmp.Top(	 Min(nNew, nOld) );
/*N*/ 			aTmp.Bottom( Max(nNew, nOld) );
/*N*/ 			pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	NotifyBackground()
|*
|*************************************************************************/

/*N*/ void lcl_CheckFlowBack( SwFrm* pFrm, const SwRect &rRect )
/*N*/ {
/*N*/     SwTwips nBottom = rRect.Bottom();
/*N*/     while( pFrm )
/*N*/     {
/*N*/         if( pFrm->IsLayoutFrm() )
/*N*/         {
/*N*/             if( rRect.IsOver( pFrm->Frm() ) )
/*N*/                 lcl_CheckFlowBack( ((SwLayoutFrm*)pFrm)->Lower(), rRect );
/*N*/         }
/*N*/         else if( !pFrm->GetNext() && nBottom > pFrm->Frm().Bottom() )
/*N*/         {
/*N*/             if( pFrm->IsCntntFrm() && ((SwCntntFrm*)pFrm)->HasFollow() )
/*N*/                 pFrm->InvalidateSize();
/*N*/             else
/*N*/                 pFrm->InvalidateNextPos();
/*N*/         }
/*N*/         pFrm = pFrm->GetNext();
/*N*/     }
/*N*/ }


/*N*/ #ifdef _MSC_VER
/*N*/ #pragma optimize("",off)
/*N*/ #endif

/*N*/ void MA_FASTCALL lcl_NotifyCntnt( SdrObject *pThis, SwCntntFrm *pCnt,
/*N*/ 	const SwRect &rRect, const PrepareHint eHint )
/*N*/ {
/*N*/ 	if ( pCnt->IsTxtFrm() )
/*N*/ 	{
/*N*/ 		SwRect aCntPrt( pCnt->Prt() );
/*N*/ 		aCntPrt.Pos() += pCnt->Frm().Pos();
/*N*/ 		if ( eHint == PREP_FLY_ATTR_CHG )
/*N*/ 		{
/*N*/ 			if ( aCntPrt.IsOver( pThis->GetBoundRect() ) )
/*?*/ 				pCnt->Prepare( PREP_FLY_ATTR_CHG );
/*N*/ 		}
/*N*/ 		else if ( aCntPrt.IsOver( rRect ) || pCnt->IsFollow() || pCnt->HasFollow() )
/*N*/ 			pCnt->Prepare( eHint, (void*)&aCntPrt._Intersection( rRect ) );
/*N*/ 		if ( pCnt->GetDrawObjs() )
/*N*/ 		{
/*N*/ 			const SwDrawObjs &rObjs = *pCnt->GetDrawObjs();
/*N*/ 			for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/ 			{
/*N*/ 				SdrObject *pO = rObjs[i];
/*N*/ 				if ( pO->IsWriterFlyFrame() )
/*N*/ 				{
/*N*/ 					SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 					if ( pFly->IsFlyInCntFrm() )
/*N*/ 					{
/*N*/ 						SwCntntFrm *pCntnt = pFly->ContainsCntnt();
/*N*/ 						while ( pCntnt )
/*N*/ 						{
/*N*/ 							::binfilter::lcl_NotifyCntnt( pThis, pCntnt, rRect, eHint );
/*N*/ 							pCntnt = pCntnt->GetNextCntntFrm();
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void Notify_Background( SdrObject *pObj, SwPageFrm *pPage, const SwRect& rRect,
/*N*/ 						const PrepareHint eHint, const BOOL bInva )
/*N*/ {
/*N*/ 
/*N*/ 	//Wenn der Frm gerade erstmalig sinnvoll positioniert wurde, braucht der
/*N*/ 	//alte Bereich nicht benachrichtigt werden.
/*N*/ 	if ( eHint == PREP_FLY_LEAVE && rRect.Top() == WEIT_WECH )
/*N*/ 		 return;
/*N*/ 
/*N*/ 	SwLayoutFrm *pArea;
/*N*/ 	SwFlyFrm *pFlyFrm = 0;
/*N*/ 	SwFrm* pAnchor;
/*N*/ 	if( pObj->IsWriterFlyFrame() )
/*N*/ 	{
/*N*/ 		pFlyFrm = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 
/*N*/ 		//MA: Wozu ausserhalb des Ankers invalidieren? Dort wird ja eh nicht
/*N*/ 		//auf den Rahmen Ruecksicht genommen; normalerweise kann er dort
/*N*/ 		//gar nicht hin, ausser temporaer beim Formatieren.
/*N*/ 		pAnchor = pFlyFrm->GetAnchor();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pFlyFrm = NULL;
/*N*/ 		pAnchor = ((SwDrawContact*)GetUserCall(pObj))->GetAnchor();
/*N*/ 	}
/*N*/ 	if( PREP_FLY_LEAVE != eHint && pAnchor->IsInFly() )
/*N*/ 		pArea = pAnchor->FindFlyFrm();
/*N*/ 	else
/*N*/ 		pArea = pPage;
/*N*/ 	SwCntntFrm *pCnt = 0;
/*N*/ 	if ( pArea )
/*N*/ 	{
/*N*/         if( PREP_FLY_ARRIVE != eHint )
/*N*/             lcl_CheckFlowBack( pArea, rRect );
/*N*/ 
/*N*/ 		//Es reagieren sowieso nur die auf den Anker folgenden auf den Fly, also
/*N*/ 		//brauchen diese nicht abgeklappert werden.
/*N*/ 		//Ausnahme sind ist natuerlich das LEAVE, denn der Fly koennte ja von
/*N*/ 		//"oben" kommen.
/*N*/ 		// Wenn der Anker auf der vorhergehenden Seite liegt, muss ebenfalls
/*N*/ 		// die gesamte Seite abgearbeitet werden. (47722)
/*N*/ 		if ( PREP_FLY_LEAVE != eHint && pAnchor->IsCntntFrm() &&
/*N*/ 			pArea->IsAnLower( pAnchor ) )
/*N*/ 			pCnt = (SwCntntFrm*)pAnchor;
/*N*/ 		else
/*N*/ 			pCnt = pArea->ContainsCntnt();
/*N*/ 	}
/*N*/ 	SwFrm *pLastTab = 0;
/*N*/ 
/*N*/ 	while ( pCnt && pArea->IsAnLower( pCnt ) )
/*N*/ 	{
/*N*/ 		::binfilter::lcl_NotifyCntnt( pObj, pCnt, rRect, eHint );
/*N*/ 		if ( pCnt->IsInTab() )
/*N*/ 		{
/*N*/ 			SwLayoutFrm* pCell = pCnt->GetUpper();
/*N*/ 			if( pCell->IsCellFrm() &&
/*N*/ 				( (pCell->Frm().IsOver( pObj->GetBoundRect() ) ||
/*N*/ 					pCell->Frm().IsOver( rRect )) ) )
/*N*/ 			{
/*N*/ 				const SwFmtVertOrient &rOri = pCell->GetFmt()->GetVertOrient();
/*N*/ 				if ( VERT_NONE != rOri.GetVertOrient() )
/*N*/ 					pCell->InvalidatePrt();
/*N*/ 			}
/*N*/ 			SwTabFrm *pTab = pCnt->FindTabFrm();
/*N*/ 			if ( pTab != pLastTab )
/*N*/ 			{
/*N*/ 				pLastTab = pTab;
/*N*/ 				if ( pTab->Frm().IsOver( pObj->GetBoundRect() ) ||
/*N*/ 						pTab->Frm().IsOver( rRect ) )
/*N*/ 				{
/*N*/ 					if ( !pFlyFrm || !pFlyFrm->IsLowerOf( pTab ) )
/*N*/ 						pTab->InvalidatePrt();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pCnt = pCnt->GetNextCntntFrm();
/*N*/ 	}
// #108745# Sorry, but this causes nothing but trouble. I remove these lines
// taking the risk that the footer frame will have a wrong height
//  if( pPage->Lower() )
//  {
//      SwFrm* pFrm = pPage->Lower();
//      while( pFrm->GetNext() )
//          pFrm = pFrm->GetNext();
//      if( pFrm->IsFooterFrm() &&
//          ( ( pFrm->Frm().IsOver( pObj->GetBoundRect() ) ||
//              pFrm->Frm().IsOver( rRect ) ) ) )
//           pFrm->InvalidateSize();
//  }

/*N*/ 	if( pPage->GetSortedObjs() )
/*N*/ 	{
/*N*/ 		pObj->GetOrdNum();
/*N*/ 		const SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
/*N*/ 		for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/ 		{
/*N*/ 			SdrObject *pO = rObjs[i];
/*N*/ 			if ( pO->IsWriterFlyFrame() )
/*N*/ 			{
/*N*/ 				if( pO == pObj )
/*N*/ 					continue;
/*N*/ 				SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 				if ( pFly->Frm().Top() == WEIT_WECH )
/*N*/ 					continue;
/*N*/ 
/*N*/ 				if ( !pFlyFrm ||
/*N*/ 						(!pFly->IsLowerOf( pFlyFrm ) &&
/*N*/ 						pFly->GetVirtDrawObj()->GetOrdNumDirect() < pObj->GetOrdNumDirect()))
/*N*/ 				{
/*N*/ 					pCnt = pFly->ContainsCntnt();
/*N*/ 					while ( pCnt )
/*N*/ 					{
/*N*/ 						::binfilter::lcl_NotifyCntnt( pObj, pCnt, rRect, eHint );
/*N*/ 						pCnt = pCnt->GetNextCntntFrm();
/*N*/ 					}
/*N*/ 				}
/*N*/ 				if( pFly->IsFlyLayFrm() )
/*N*/ 				{
/*N*/ 					if( pFly->Lower() && pFly->Lower()->IsColumnFrm() &&
/*N*/ 						pFly->Frm().Bottom() >= rRect.Top() &&
/*N*/ 						pFly->Frm().Top() <= rRect.Bottom() &&
/*N*/ 						pFly->Frm().Right() >= rRect.Left() &&
/*N*/ 						pFly->Frm().Left() <= rRect.Right() )
/*N*/ 					 {
/*N*/ 						const SwFmtFrmSize &rSz = pFly->GetFmt()->GetFrmSize();
/*N*/ 						pFly->InvalidateSize();
/*N*/ 					 }
/*N*/ 				}
/*N*/ 				//Flys, die ueber mir liegen muessen/mussten evtl.
/*N*/ 				//ausweichen, wenn sie eine automatische Ausrichtung haben.
/*N*/ 				//das ist unabhaengig von meinem Attribut, weil dies sich
/*N*/ 				//gerade geaendert haben kann und eben deshalb
/*N*/ 				//umformatiert wurde.
/*N*/ 				else if ( pFly->IsFlyAtCntFrm() &&
/*N*/ 						pObj->GetOrdNumDirect() <
/*N*/ 						pFly->GetVirtDrawObj()->GetOrdNumDirect() &&
/*N*/ 						pFlyFrm && !pFly->IsLowerOf( pFlyFrm ) )
/*N*/ 				{
/*N*/ 					const SwFmtHoriOrient &rH = pFly->GetFmt()->GetHoriOrient();
/*N*/ 					if ( HORI_NONE != rH.GetHoriOrient()  &&
/*N*/ 							HORI_CENTER != rH.GetHoriOrient()  &&
/*N*/ 							( !pFly->IsAutoPos() || REL_CHAR != rH.GetRelationOrient() ) &&
/*N*/ 							(pFly->Frm().Bottom() >= rRect.Top() &&
/*N*/ 							pFly->Frm().Top() <= rRect.Bottom()) )
/*N*/ 						pFly->InvalidatePos();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( pFlyFrm && pAnchor->GetUpper() && pAnchor->IsInTab() )//MA_FLY_HEIGHT
/*N*/ 		pAnchor->GetUpper()->InvalidateSize();
/*N*/ 
/*N*/ 	ViewShell *pSh;
/*N*/ 	if( bInva && 0 != (pSh = pPage->GetShell()) )
/*N*/ 		pSh->InvalidateWindows( rRect );
/*N*/ }

/*N*/ #ifdef _MSC_VER
/*N*/ #pragma optimize("",on)
/*N*/ #endif

/*************************************************************************
|*
|*	GetVirtualUpper() liefert bei absatzgebundenen Objekten den Upper
|*  des Ankers. Falls es sich dabei um verkettete Rahmen oder
|*	Fussnoten handelt, wird ggf. der "virtuelle" Upper ermittelt.
|*
|*************************************************************************/

/*N*/ const SwFrm* GetVirtualUpper( const SwFrm* pFrm, const Point& rPos )
/*N*/ {
/*N*/ 	if( pFrm->IsTxtFrm() )
/*N*/ 	{
/*N*/ 		pFrm = pFrm->GetUpper();
/*N*/ 		if( !pFrm->Frm().IsInside( rPos ) )
/*N*/ 		{
/*N*/ 			if( pFrm->IsFtnFrm() )
/*N*/ 			{
/*?*/ 				const SwFtnFrm* pTmp = ((SwFtnFrm*)pFrm)->GetFollow();
/*?*/ 				while( pTmp )
/*?*/ 				{
/*?*/ 					if( pTmp->Frm().IsInside( rPos ) )
/*?*/ 						return pTmp;
/*?*/ 					pTmp = pTmp->GetFollow();
/*?*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SwFlyFrm* pTmp = (SwFlyFrm*)pFrm->FindFlyFrm();
/*N*/ 				while( pTmp )
/*N*/ 				{
/*N*/ 					if( pTmp->Frm().IsInside( rPos ) )
/*N*/ 						return pTmp;
/*N*/ 					pTmp = pTmp->GetNextLink();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pFrm;
/*N*/ }

/*************************************************************************
|*
|*	IsLowerOf()
|*
|*************************************************************************/

/*N*/ BOOL Is_Lower_Of( const SwFrm *pCurrFrm, const SdrObject* pObj )
/*N*/ {
/*N*/ 	Point aPos;
/*N*/ 	const SwFrm* pFrm;
/*N*/ 	if( pObj->IsWriterFlyFrame() )
/*N*/ 	{
/*N*/ 		const SwFlyFrm* pFly = ( (SwVirtFlyDrawObj*)pObj )->GetFlyFrm();
/*N*/ 		pFrm = pFly->GetAnchor();
/*N*/ 		aPos = pFly->Frm().Pos();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pFrm = ( (SwDrawContact*)GetUserCall(pObj) )->GetAnchor();
/*N*/ 		aPos = pObj->GetBoundRect().TopLeft();
/*N*/ 	}
/*N*/ 	ASSERT( pFrm, "8-( Fly is lost in Space." );
/*N*/ 	pFrm = GetVirtualUpper( pFrm, aPos );
/*N*/ 	do
/*N*/ 	{	if ( pFrm == pCurrFrm )
/*N*/ 			return TRUE;
/*N*/ 		if( pFrm->IsFlyFrm() )
/*N*/ 		{
/*N*/ 			aPos = pFrm->Frm().Pos();
/*N*/ 			pFrm = GetVirtualUpper( ((const SwFlyFrm*)pFrm)->GetAnchor(), aPos );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pFrm = pFrm->GetUpper();
/*N*/ 	} while ( pFrm );
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ const SwFrm *FindKontext( const SwFrm *pFrm, USHORT nAdditionalKontextTyp )
/*N*/ {
/*N*/ 	//Liefert die Umgebung des Frm in die kein Fly aus einer anderen
/*N*/ 	//Umgebung hineinragen kann.
/*N*/ 	const USHORT nTyp = FRM_ROOT | FRM_HEADER   | FRM_FOOTER | FRM_FTNCONT  |
/*N*/ 						FRM_FTN  | FRM_FLY      |
/*N*/ 						FRM_TAB  | FRM_ROW		| FRM_CELL |
/*N*/ 						nAdditionalKontextTyp;
/*N*/ 	do
/*N*/ 	{	if ( pFrm->GetType() & nTyp )
/*N*/ 			break;
/*N*/ 		pFrm = pFrm->GetUpper();
/*N*/ 	} while( pFrm );
/*N*/ 	return pFrm;
/*N*/ }

/*N*/ BOOL IsFrmInSameKontext( const SwFrm *pInnerFrm, const SwFrm *pFrm )
/*N*/ {
/*N*/ 	const SwFrm *pKontext = FindKontext( pInnerFrm, 0 );
/*N*/ 
/*N*/ 	const USHORT nTyp = FRM_ROOT | FRM_HEADER   | FRM_FOOTER | FRM_FTNCONT  |
/*N*/ 						FRM_FTN  | FRM_FLY      |
/*N*/ 						FRM_TAB  | FRM_ROW 		| FRM_CELL;
/*N*/ 	do
/*N*/ 	{	if ( pFrm->GetType() & nTyp )
/*N*/ 		{
/*N*/ 			if( pFrm == pKontext )
/*N*/ 				return TRUE;
/*N*/ 			if( pFrm->IsCellFrm() )
/*N*/ 				return FALSE;
/*N*/ 		}
/*N*/ 		if( pFrm->IsFlyFrm() )
/*N*/ 		{
/*N*/ 			Point aPos( pFrm->Frm().Pos() );
/*N*/ 			pFrm = GetVirtualUpper( ((const SwFlyFrm*)pFrm)->GetAnchor(), aPos );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pFrm = pFrm->GetUpper();
/*N*/ 	} while( pFrm );
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }


//---------------------------------





/*N*/ const SwFrm* MA_FASTCALL FindPage( const SwRect &rRect, const SwFrm *pPage )
/*N*/ {
/*N*/ 	if ( !rRect.IsOver( pPage->Frm() ) )
/*N*/ 	{
/*N*/ 		BOOL bPrvAllowed = TRUE;
/*N*/ 		BOOL bNxtAllowed = TRUE;
/*N*/ 		do
/*N*/ 		{   if ( pPage->Frm().Top() > rRect.Top() && bPrvAllowed )
/*N*/ 			{
/*N*/ 				if ( pPage->GetPrev() )
/*N*/ 				{
/*N*/ 					bNxtAllowed = FALSE;
/*N*/ 					pPage = pPage->GetPrev();
/*N*/ 				}
/*N*/ 				else
/*N*/ 					break;
/*N*/ 			}
/*N*/ 			else if ( pPage->Frm().Bottom() < rRect.Top() && bNxtAllowed )
/*N*/ 			{
/*N*/ 				if ( pPage->GetNext() )
/*N*/ 				{
/*N*/ 					bPrvAllowed = FALSE;
/*N*/ 					pPage = pPage->GetNext();
/*N*/ 				}
/*N*/ 				else
/*N*/ 					break;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				break;
/*N*/ 
/*N*/ 		} while ( !rRect.IsOver( pPage->Frm() ) );
/*N*/ 	}
/*N*/ 	return pPage;
/*N*/ }


/*N*/ SwFrm* GetFrmOfModify( SwModify& rMod, USHORT nFrmType, const Point* pPoint,
/*N*/ 						const SwPosition *pPos, const BOOL bCalcFrm )
/*N*/ {
/*N*/ 	SwFrm *pMinFrm = 0, *pTmpFrm;
/*N*/ 	SwRect aCalcRect;
/*N*/ 
/*N*/ 	SwClientIter aIter( rMod );
/*N*/ 	do {
/*N*/ 		pMinFrm = 0;
/*N*/ 		Size aMinSize;
/*N*/ 
/*N*/ 		for( pTmpFrm = (SwFrm*)aIter.First( TYPE( SwFrm )); pTmpFrm;
/*N*/ 				pTmpFrm = (SwFrm*)aIter.Next() )
/*N*/ 			if( pTmpFrm->GetType() & nFrmType &&
/*N*/ 				(!pTmpFrm->IsFlowFrm() ||
/*N*/ 				 !SwFlowFrm::CastFlowFrm( pTmpFrm )->IsFollow() ))
/*N*/ 			{
/*N*/ 				if( pPoint )
/*N*/ 				{
/*N*/ 					if( bCalcFrm )
/*N*/ 						pTmpFrm->Calc();
/*N*/ 
/*N*/ 					if( aIter.IsChanged() )		// der Liste hat sich ver-
/*N*/ 						break;					// aendert, neu anfangen !!
/*N*/ 
/*N*/ 					// bei Flys ggfs. ueber den Parent gehen wenn sie selbst
/*N*/ 					// nocht nicht "formatiert" sind
/*N*/ 					if( !bCalcFrm && nFrmType & FRM_FLY &&
/*N*/ 						((SwFlyFrm*)pTmpFrm)->GetAnchor() &&
/*N*/ 						WEIT_WECH == pTmpFrm->Frm().Pos().X() &&
/*N*/ 						WEIT_WECH == pTmpFrm->Frm().Pos().Y() )
/*N*/ 						aCalcRect = ((SwFlyFrm*)pTmpFrm)->GetAnchor()->Frm();
/*N*/ 					else
/*N*/ 						aCalcRect = pTmpFrm->Frm();
/*N*/ 
/*N*/ 					// fasse den Point und das Recteck zusammen, falls
/*N*/ 					// er Point nicht innerhalb liegt. Liegt er ausserhalb,
/*N*/ 					// wird nach dem kleinsten Rectangle gesucht, also das,
/*N*/ 					// wo der Point am dichtesten dran liegt. Ist der Point im
/*N*/ 					// Rechteck, wird die Schleife beendet.
/*N*/ 					{
/*N*/ 						BOOL bInside = TRUE;
/*N*/ 						// die Left/Right-Position erweitern
/*N*/ 						if( pPoint->X() < aCalcRect.Left() )
/*N*/ 							{	bInside = FALSE; aCalcRect.Left( pPoint->X() ); }
/*N*/ 						if( pPoint->X() > aCalcRect.Right() )
/*N*/ 							{	bInside = FALSE; aCalcRect.Right( pPoint->X() ); }
/*N*/ 
/*N*/ 						if( pPoint->Y() > aCalcRect.Bottom() )
/*N*/ 							{	bInside = FALSE; aCalcRect.Bottom( pPoint->Y() ); }
/*N*/ 						if( pPoint->Y() < aCalcRect.Top() )
/*N*/ 							{	bInside = FALSE; aCalcRect.Top( pPoint->Y() ); }
/*N*/ 						if( bInside )
/*N*/ 						{
/*N*/ 							pMinFrm = pTmpFrm;
/*N*/ 							break;
/*N*/ 						}
/*N*/ 					}
/*N*/ 
/*N*/ 					if( pMinFrm )
/*N*/ 					{
/*?*/ 						long nDiffW = aMinSize.Width() - aCalcRect.Width();
/*?*/ 						long nDiffH = aMinSize.Height() - aCalcRect.Height();
/*?*/ 
/*?*/ 							// gleiche Hoehe, dann entscheided die Breite
/*?*/ 						if( !nDiffH )		{ if( 0 >= nDiffW ) continue; }
/*?*/ 							// gleiche Breite, dann entscheided die Hoehe
/*?*/ 						else if( !nDiffW )	{ if( 0 >= nDiffH ) continue; }
/*?*/ 
/*?*/ 							// hoehere Gewichtung auf die Hoehe !!
/*?*/ 						else if( !(0 < nDiffW && 0 < nDiffH ) &&
/*?*/ 								((0 > nDiffW && 0 > nDiffH ) ||
/*?*/ 								0 >= nDiffH ))
/*?*/ 							continue;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// Wenn kein pPoint angegeben ist, dann reichen
/*N*/ 					// wir irgendeinen raus: den ersten!
/*N*/ 					pMinFrm = pTmpFrm;
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				pMinFrm = pTmpFrm;
/*N*/ 				aMinSize = aCalcRect.SSize();
/*N*/ 			}
/*N*/ 	} while( aIter.IsChanged() );
/*N*/ 
/*N*/ 	if( pPos && pMinFrm && pMinFrm->IsTxtFrm() )
/*N*/ 		return ((SwTxtFrm*)pMinFrm)->GetFrmAtPos( *pPos );
/*N*/ 
/*N*/ 	return pMinFrm;
/*N*/ }

/*N*/ FASTBOOL IsExtraData( const SwDoc *pDoc )
/*N*/ {
/*N*/ 	const SwLineNumberInfo &rInf = pDoc->GetLineNumberInfo();
/*N*/ 	return rInf.IsPaintLineNumbers() ||
/*N*/ 		   rInf.IsCountInFlys() ||
/*N*/ 		   ((SwHoriOrient)SW_MOD()->GetRedlineMarkPos() != HORI_NONE &&
/*N*/ 			pDoc->GetRedlineTbl().Count());
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
