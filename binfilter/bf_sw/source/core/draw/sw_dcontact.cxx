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

#include "hintids.hxx"

#include <bf_svx/ulspitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/svdpage.hxx>
#include <bf_svx/fmglob.hxx>
#include <bf_svx/svdogrp.hxx>
#include <bf_svx/svdviter.hxx>
#include <bf_svx/svdview.hxx>


#include <horiornt.hxx>

#include <fmtornt.hxx>
#include <viewimp.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <frmtool.hxx>	// Notify_Background
#include <flyfrm.hxx>
#include <frmfmt.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <hints.hxx>
#include <txtfrm.hxx>
#include <editsh.hxx>
#include <docary.hxx>

// #108784#
#include <bf_svx/xoutx.hxx>

// OD 18.06.2003 #108784#
#include <algorithm>
namespace binfilter {


/*N*/ TYPEINIT1( SwContact, SwClient )
/*N*/ TYPEINIT1( SwFlyDrawContact, SwContact )
/*N*/ TYPEINIT1( SwDrawContact, SwContact )


//Der Umgekehrte Weg: Sucht das Format zum angegebenen Objekt.
//Wenn das Object ein SwVirtFlyDrawObj ist so wird das Format von
//selbigem besorgt.
//Anderfalls ist es eben ein einfaches Zeichenobjekt. Diese hat einen
//UserCall und der ist Client vom gesuchten Format.

/*N*/ SwFrmFmt *FindFrmFmt( SdrObject *pObj )
/*N*/ {
/*N*/     SwFrmFmt* pRetval = 0L;
/*N*/ 
/*N*/ 	if ( pObj->IsWriterFlyFrame() )
/*N*/ 	{
/*N*/ 		pRetval = ((SwVirtFlyDrawObj*)pObj)->GetFmt();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         SwDrawContact* pContact = static_cast<SwDrawContact*>(GetUserCall( pObj ));
/*N*/         if ( pContact )
/*N*/ 		{
/*N*/ 			pRetval = pContact->GetFmt();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return pRetval;
/*N*/ }

/*N*/ sal_Bool HasWrap( const SdrObject* pObj )
/*N*/ {
/*N*/     if ( pObj )
/*N*/     {
/*N*/         const SwFrmFmt* pFmt = ::binfilter::FindFrmFmt( pObj );
/*N*/         if ( pFmt )
/*N*/         {
/*N*/             return SURROUND_THROUGHT != pFmt->GetSurround().GetSurround();
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     return sal_False;
/*N*/ }

/*****************************************************************************
 *
 * GetBoundRect liefert das BoundRect _inklusive_ Abstand des Objekts.
 *
 *****************************************************************************/

/*N*/ SwRect GetBoundRect( const SdrObject* pObj )
/*N*/ {
/*N*/ 	SwRect aRet( pObj->GetBoundRect() );
/*N*/ 	const SwFmt *pFmt = ((SwContact*)GetUserCall(pObj))->GetFmt();
/*N*/ 	const SvxULSpaceItem &rUL = pFmt->GetULSpace();
/*N*/ 	const SvxLRSpaceItem &rLR = pFmt->GetLRSpace();
/*N*/     {
/*N*/         aRet.Top ( Max( aRet.Top() - long(rUL.GetUpper()), 0L ));
/*N*/         aRet.Left( Max( aRet.Left()- long(rLR.GetLeft()),  0L ));
/*N*/         aRet.SSize().Height() += rUL.GetLower();
/*N*/         aRet.SSize().Width()  += rLR.GetRight();
/*N*/     }
/*N*/ 	return aRet;
/*N*/ }

/*****************************************************************************
 *
 *  Moves a SdrObj so that it fits to a given frame
 *
 *****************************************************************************/

/*N*/ void CaptureDrawObj( SdrObject& rObj, const SwRect& rFrm )
/*N*/ {
/*N*/     Rectangle aRect( rObj.GetBoundRect() );
/*N*/     if ( aRect.Right() >= rFrm.Right() + 10 )
/*N*/     {
/*N*/         Size aSize( rFrm.Right() - aRect.Right(), 0 );
/*N*/         rObj.Move( aSize );
/*N*/         aRect = rObj.GetBoundRect();
/*N*/     }
/*N*/ 
/*N*/     if ( aRect.Left() + 10 <= rFrm.Left() )
/*N*/     {
/*N*/         Size aSize( rFrm.Left() - aRect.Left(), 0 );
/*N*/         rObj.Move( aSize );
/*N*/     }
/*N*/ }

//Liefert den UserCall ggf. vom Gruppenobjekt
/*N*/ SdrObjUserCall* GetUserCall( const SdrObject* pObj )
/*N*/ {
/*N*/ 	SdrObject *pTmp;
/*N*/ 	while ( !pObj->GetUserCall() && 0 != (pTmp = pObj->GetUpGroup()) )
/*N*/ 		pObj = pTmp;
/*N*/ 	return pObj->GetUserCall();
/*N*/ }

// liefert TRUE falls das SrdObject ein Marquee-Object (Lauftext) ist

/*************************************************************************
|*
|*	SwContact, Ctor und Dtor
|*
|*	Ersterstellung		AMA 27.Sep.96 18:13
|*	Letzte Aenderung	AMA 27.Sep.96
|*
|*************************************************************************/

/*N*/ SwContact::SwContact( SwFrmFmt *pToRegisterIn, SdrObject *pObj ) :
/*N*/ 	SwClient( pToRegisterIn ),
/*N*/ 	pMasterObj( pObj )
/*N*/ {
/*N*/ 	pObj->SetUserCall( this );
/*N*/ }

/*N*/ SwContact::~SwContact()
/*N*/ {
/*N*/ 	if ( pMasterObj )
/*N*/ 	{
/*N*/ 		pMasterObj->SetUserCall( 0 );	//Soll mir nicht in den Ruecken fallen.
/*N*/ 		if ( pMasterObj->GetPage() )	//Der SdrPage die Verantwortung entziehen.
/*N*/ 			pMasterObj->GetPage()->RemoveObject( pMasterObj->GetOrdNum() );
/*N*/ 		delete pMasterObj;
/*N*/ 	}
/*N*/ }

// OD 13.05.2003 #108784# - copied inline-implementation of <GetMaster()> and
// <SetMaster()>
/*N*/ const SdrObject* SwContact::GetMaster() const
/*N*/ {
/*N*/     return pMasterObj;
/*N*/ }
/*N*/ SdrObject* SwContact::GetMaster()
/*N*/ {
/*N*/     return pMasterObj;
/*N*/ }
/*N*/ void SwContact::SetMaster( SdrObject* pNew )
/*N*/ {
/*N*/     pMasterObj = pNew;
/*N*/ }

/*************************************************************************
|*
|*	SwFlyDrawContact, Ctor und Dtor
|*
|*	Ersterstellung		OK 23.11.94 18:13
|*	Letzte Aenderung	MA 06. Apr. 95
|*
|*************************************************************************/

/*N*/ SwFlyDrawContact::SwFlyDrawContact( SwFlyFrmFmt *pToRegisterIn, SdrModel *pMod ) :
/*N*/ 	SwContact( pToRegisterIn )
/*N*/ {
/*N*/ 	SetMaster( new SwFlyDrawObj() );
/*N*/ 	GetMaster()->SetOrdNum( 0xFFFFFFFE );
/*N*/ 	GetMaster()->SetUserCall( this );
/*N*/ }

/*************************************************************************
|*
|*	SwFlyDrawContact::CreateNewRef()
|*
|*	Ersterstellung		MA 14. Dec. 94
|*	Letzte Aenderung	MA 24. Apr. 95
|*
|*************************************************************************/

/*N*/ SwVirtFlyDrawObj *SwFlyDrawContact::CreateNewRef( SwFlyFrm *pFly )
/*N*/ {
/*N*/ 	SwVirtFlyDrawObj *pDrawObj = new SwVirtFlyDrawObj( *GetMaster(), pFly );
/*N*/ 	pDrawObj->SetModel( GetMaster()->GetModel() );
/*N*/ 	pDrawObj->SetUserCall( this );
/*N*/ 
/*N*/ 	//Der Reader erzeugt die Master und setzt diese, um die Z-Order zu
/*N*/ 	//transportieren, in die Page ein. Beim erzeugen der ersten Referenz werden
/*N*/ 	//die Master aus der Liste entfernt und fuehren von da an ein
/*N*/ 	//Schattendasein.
/*N*/ 	SdrPage *pPg;
/*N*/ 	if ( 0 != ( pPg = GetMaster()->GetPage() ) )
/*N*/ 	{
/*N*/ 		const UINT32 nOrdNum = GetMaster()->GetOrdNum();
/*N*/ 		pPg->ReplaceObject( pDrawObj, nOrdNum );
/*N*/ 	}
/*N*/ 	return pDrawObj;
/*N*/ }

/*************************************************************************
|*
|*	SwFlyDrawContact::Modify()
|*
|*	Ersterstellung		OK 08.11.94 10:21
|*	Letzte Aenderung	MA 06. Dec. 94
|*
|*************************************************************************/

/*N*/ void SwFlyDrawContact::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	SwDrawContact, Ctor+Dtor
|*
|*	Ersterstellung		MA 09. Jan. 95
|*	Letzte Aenderung	MA 22. Jul. 98
|*
|*************************************************************************/
/*N*/ bool CheckControlLayer( const SdrObject *pObj )
/*N*/ {
/*N*/ 	if ( FmFormInventor == pObj->GetObjInventor() )
/*N*/         return true;
/*N*/ 	if ( pObj->ISA( SdrObjGroup ) )
/*N*/ 	{
/*N*/ 		const SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();
/*N*/ 		for ( USHORT i = 0; i < pLst->GetObjCount(); ++i )
/*N*/             if ( ::binfilter::CheckControlLayer( pLst->GetObj( i ) ) )
/*N*/                 return false;
/*N*/ 	}
/*N*/     return false;
/*N*/ }

/*N*/ SwDrawContact::SwDrawContact( SwFrmFmt *pToRegisterIn, SdrObject *pObj ) :
/*N*/ 	SwContact( pToRegisterIn, pObj ),
/*N*/ 	pAnchor( 0 ),
/*N*/     pPage( 0 )
/*N*/ {
/*N*/     // clear vector containing 'virtual' drawing objects.
/*N*/    maDrawVirtObjs.clear();
/*N*/ 
/*N*/     //Controls muessen immer im Control-Layer liegen. Das gilt auch fuer
/*N*/ 	//Gruppenobjekte, wenn diese Controls enthalten.
/*N*/     if ( ::binfilter::CheckControlLayer( pObj ) )
/*N*/     {
/*N*/         // OD 25.06.2003 #108784# - set layer of object to corresponding invisible layer.
/*N*/         pObj->SetLayer( pToRegisterIn->GetDoc()->GetInvisibleControlsId() );
/*N*/     }
/*N*/ }

/*N*/ SwDrawContact::~SwDrawContact()
/*N*/ {
/*N*/     DisconnectFromLayout();
/*N*/ 
/*N*/     // OD 25.06.2003 #108784# - remove 'master' from drawing page
/*N*/     RemoveMasterFromDrawPage();
/*N*/ 
/*N*/     // remove and destroy 'virtual' drawing objects.
/*N*/     RemoveAllVirtObjs();
/*N*/ }

// OD 23.06.2003 #108784# - method to create a new 'virtual' drawing object.
/*N*/ SwDrawVirtObj* SwDrawContact::CreateVirtObj()
/*N*/ {
/*N*/     // determine 'master'
/*N*/     SdrObject* pOrgMasterSdrObj = GetMaster();
/*N*/ 
/*N*/     // create 'virtual' drawing object
/*N*/     SwDrawVirtObj* pNewDrawVirtObj = new SwDrawVirtObj ( *(pOrgMasterSdrObj), *(this) );
/*N*/ 
/*N*/     // add new 'virtual' drawing object managing data structure
/*N*/     maDrawVirtObjs.push_back( pNewDrawVirtObj );
/*N*/ 
/*N*/     return pNewDrawVirtObj;
/*N*/ }

// OD 23.06.2003 #108784# - destroys a given 'virtual' drawing object.
// side effect: 'virtual' drawing object is removed from data structure
//              <maDrawVirtObjs>.
/*N*/ void SwDrawContact::DestroyVirtObj( SwDrawVirtObj* _pVirtObj )
/*N*/ {
/*N*/     if ( _pVirtObj )
/*N*/     {
/*N*/         delete _pVirtObj;
/*N*/         _pVirtObj = 0;
/*N*/     }
/*N*/ }

// OD 16.05.2003 #108784# - add a 'virtual' drawing object to drawing page.
// Use an already created one, which isn't used, or create a new one.
/*N*/ SwDrawVirtObj* SwDrawContact::AddVirtObj()
/*N*/ {
/*N*/     SwDrawVirtObj* pAddedDrawVirtObj = 0L;
/*N*/ 
/*N*/     // check, if a disconnected 'virtual' drawing object exist and use it
/*N*/     std::list<SwDrawVirtObj*>::const_iterator aFoundVirtObjIter =
/*N*/             std::find_if( maDrawVirtObjs.begin(), maDrawVirtObjs.end(),
/*N*/                           UsedOrUnusedVirtObjPred( false ) );
/*N*/ 
/*N*/     if ( aFoundVirtObjIter != maDrawVirtObjs.end() )
/*N*/     {
/*N*/         // use already created, disconnected 'virtual' drawing object
/*N*/         pAddedDrawVirtObj = (*aFoundVirtObjIter);
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         // create new 'virtual' drawing object.
/*N*/         pAddedDrawVirtObj = CreateVirtObj();
/*N*/     }
/*N*/     pAddedDrawVirtObj->AddToDrawingPage();
/*N*/ 
/*N*/     return pAddedDrawVirtObj;
/*N*/ }

// OD 16.05.2003 #108784# - remove 'virtual' drawing objects and destroy them.
/*N*/ void SwDrawContact::RemoveAllVirtObjs()
/*N*/ {
/*N*/     for ( std::list<SwDrawVirtObj*>::iterator aDrawVirtObjsIter = maDrawVirtObjs.begin();
/*N*/           aDrawVirtObjsIter != maDrawVirtObjs.end();
/*N*/           ++aDrawVirtObjsIter )
/*N*/     {
/*N*/         // remove and destroy 'virtual object'
/*N*/         SwDrawVirtObj* pDrawVirtObj = (*aDrawVirtObjsIter);
/*N*/         pDrawVirtObj->RemoveFromWriterLayout();
/*N*/         pDrawVirtObj->RemoveFromDrawingPage();
/*N*/         DestroyVirtObj( pDrawVirtObj );
/*N*/     }
/*N*/     maDrawVirtObjs.clear();
/*N*/ }

// OD 16.05.2003 #108784# - overload <SwContact::SetMaster(..)> in order to
// assert, if the 'master' drawing object is replaced.
// OD 10.07.2003 #110742# - replace of master object correctly handled, if
// handled by method <SwDrawContact::ChangeMasterObject(..)>. Thus, assert
// only, if a debug level is given.
/*N*/ void SwDrawContact::SetMaster( SdrObject* pNew )
/*N*/ {
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/     ASSERT( !pNew, "debug notification - master replaced!" );
/*N*/ #endif
/*N*/     SwContact::SetMaster( pNew );
/*N*/ }

// OD 19.06.2003 #108784# - get drawing object ('master' or 'virtual') by frame.
/*N*/ SdrObject* SwDrawContact::GetDrawObjectByAnchorFrm( const SwFrm& _rAnchorFrm )
/*N*/ {
/*N*/     SdrObject* pRetDrawObj = 0L;
/*N*/ 
/*N*/     if ( GetAnchor() && GetAnchor() == &_rAnchorFrm )
/*N*/     {
/*N*/         pRetDrawObj = GetMaster();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         std::list<SwDrawVirtObj*>::const_iterator aFoundVirtObjIter =
/*N*/                 std::find_if( maDrawVirtObjs.begin(), maDrawVirtObjs.end(),
/*N*/                               VirtObjAnchoredAtFrmPred( _rAnchorFrm ) );
/*N*/ 
/*N*/         if ( aFoundVirtObjIter != maDrawVirtObjs.end() )
/*N*/         {
/*N*/             pRetDrawObj = (*aFoundVirtObjIter);
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     return pRetDrawObj;
/*N*/ }

/*************************************************************************
|*
|*	SwDrawContact::Changed
|*
|*	Ersterstellung		MA 09. Jan. 95
|*	Letzte Aenderung	MA 29. May. 96
|*
|*************************************************************************/

/*N*/ void lcl_Notify( SwDrawContact* pThis, const Rectangle* pOldBoundRect )
/*N*/ {
/*N*/ 	SwFrm *pAnch = pThis->GetAnchor();
/*N*/ 	if( pAnch )
/*N*/ 	{
/*N*/ 		SwPageFrm *pPage = pAnch->FindPageFrm();
/*N*/ 		ASSERT( GetUserCall(pThis->GetMaster()) == pThis, "Wrong Master" );
/*N*/ 		if( pOldBoundRect && pPage )
/*N*/ 		{
/*N*/ 			SwRect aOldRect( *pOldBoundRect );
/*N*/ 			if( aOldRect.HasArea() )
/*N*/ 				Notify_Background( pThis->GetMaster(),pPage, aOldRect,
/*N*/ 									PREP_FLY_LEAVE,TRUE);
/*N*/ 		}
/*N*/ 		SwRect aRect( pThis->GetMaster()->GetBoundRect() );
/*N*/ 		if( aRect.HasArea() )
/*N*/ 		{
/*N*/ 			SwPageFrm *pPg = pThis->FindPage( aRect );
/*N*/ 			if( pPg )
/*N*/ 				Notify_Background( pThis->GetMaster(), pPg, aRect,
/*N*/ 									PREP_FLY_ARRIVE, TRUE );
/*N*/ 		}
/*N*/ 		ClrContourCache( pThis->GetMaster() );
/*N*/ 
/*N*/         // OD 03.07.2003 #108784# - notify background of all 'virtual' drawing objects
/*N*/         pThis->NotifyBackgrdOfAllVirtObjs( pOldBoundRect );
/*N*/ 	}
/*N*/ }

// OD 03.07.2003 #108784#
/*N*/ void SwDrawContact::NotifyBackgrdOfAllVirtObjs( const Rectangle* pOldBoundRect )
/*N*/ {
/*N*/     for ( std::list<SwDrawVirtObj*>::iterator aDrawVirtObjIter = maDrawVirtObjs.begin();
/*N*/           aDrawVirtObjIter != maDrawVirtObjs.end();
/*N*/           ++aDrawVirtObjIter )
/*N*/     {
/*N*/         SwDrawVirtObj* pDrawVirtObj = (*aDrawVirtObjIter);
/*N*/         if ( pDrawVirtObj->GetAnchorFrm() )
/*N*/         {
/*N*/             SwPageFrm* pPage = pDrawVirtObj->GetAnchorFrm()->FindPageFrm();
/*N*/             if( pOldBoundRect && pPage )
/*N*/             {
/*N*/                 SwRect aOldRect( *pOldBoundRect );
/*N*/                 aOldRect.Pos() += pDrawVirtObj->GetOffset();
/*N*/                 if( aOldRect.HasArea() )
/*N*/                     Notify_Background( pDrawVirtObj ,pPage,
/*N*/                                        aOldRect, PREP_FLY_LEAVE,TRUE);
/*N*/             }
/*N*/             SwRect aRect( pDrawVirtObj->GetBoundRect() );
/*N*/             if( aRect.HasArea() )
/*N*/             {
/*N*/                 SwPageFrm *pPg = pDrawVirtObj->GetPageFrm();
/*N*/                 if ( !pPg )
/*N*/                     pPg = pDrawVirtObj->GetAnchorFrm()->FindPageFrm();
/*N*/                 if ( pPg )
/*N*/                     pPg = (SwPageFrm*)::binfilter::FindPage( aRect, pPg );
/*N*/                 if( pPg )
/*N*/                     Notify_Background( pDrawVirtObj, pPg, aRect,
/*N*/                                         PREP_FLY_ARRIVE, TRUE );
/*N*/             }
/*N*/             ClrContourCache( pDrawVirtObj );
/*N*/         }
/*N*/     }
/*N*/ }

/*N*/ void SwDrawContact::Changed( const SdrObject& rObj,
/*N*/                              SdrUserCallType eType,
/*N*/                              const Rectangle& rOldBoundRect )
/*N*/ {
/*N*/ 	//Action aufsetzen, aber nicht wenn gerade irgendwo eine Action laeuft.
/*N*/ 	ViewShell *pSh = 0, *pOrg;
/*N*/ 	SwDoc *pDoc = GetFmt()->GetDoc();
/*N*/ 	if ( pDoc->GetRootFrm() && pDoc->GetRootFrm()->IsCallbackActionEnabled() )
/*N*/ 	{
/*N*/ 		pDoc->GetEditShell( &pOrg );
/*N*/ 		pSh = pOrg;
/*N*/ 		if ( pSh )
/*N*/ 			do
/*N*/ 			{   if ( pSh->Imp()->IsAction() || pSh->Imp()->IsIdleAction() )
/*N*/ 					pSh = 0;
/*N*/ 				else
/*N*/ 					pSh = (ViewShell*)pSh->GetNext();
/*N*/ 
/*N*/ 			} while ( pSh && pSh != pOrg );
/*N*/ 
/*N*/ 		if ( pSh )
/*N*/ 			pDoc->GetRootFrm()->StartAllAction();
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrObjUserCall::Changed( rObj, eType, rOldBoundRect );
/*N*/ 	_Changed( rObj, eType, &rOldBoundRect );	//Achtung, ggf. Suizid!
/*N*/ 
/*N*/ 	if ( pSh )
/*N*/ 		pDoc->GetRootFrm()->EndAllAction();
/*N*/ }

/*N*/ void SwDrawContact::_Changed( const SdrObject& rObj,
/*N*/                               SdrUserCallType eType,
/*N*/                               const Rectangle* pOldBoundRect )
/*N*/ {
/*N*/ 	BOOL bInCntnt = FLY_IN_CNTNT == GetFmt()->GetAnchor().GetAnchorId();
/*N*/     /// OD 05.08.2002 #100843# - do *not* notify, if document is destructing
/*N*/     BOOL bNotify = !bInCntnt &&
/*N*/                    !(GetFmt()->GetDoc()->IsInDtor()) &&
/*N*/                    ( SURROUND_THROUGHT != GetFmt()->GetSurround().GetSurround() );
/*N*/ 	switch( eType )
/*N*/ 	{
/*N*/ 		case SDRUSERCALL_DELETE:
/*N*/ 			{
/*N*/ 				if( bNotify )
/*N*/ 					lcl_Notify( this, pOldBoundRect );
/*N*/                 DisconnectFromLayout( false );
/*N*/ 				SetMaster( NULL );
/*N*/ 				delete this;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		case SDRUSERCALL_INSERTED:
/*N*/ 			{
/*N*/ 				ConnectToLayout();
/*N*/ 				if( bNotify )
/*N*/ 					lcl_Notify( this, pOldBoundRect );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		case SDRUSERCALL_REMOVED:
/*N*/ 			{
/*N*/ 				if( bNotify )
/*N*/ 					lcl_Notify( this, pOldBoundRect );
/*N*/                 DisconnectFromLayout( false );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		case SDRUSERCALL_MOVEONLY:
/*N*/ 		case SDRUSERCALL_RESIZE:
/*N*/ 		case SDRUSERCALL_CHILD_MOVEONLY :
/*N*/ 		case SDRUSERCALL_CHILD_RESIZE :
/*N*/ 		case SDRUSERCALL_CHILD_CHGATTR :
/*N*/ 		case SDRUSERCALL_CHILD_DELETE :
/*N*/ 		case SDRUSERCALL_CHILD_COPY :
/*N*/ 		case SDRUSERCALL_CHILD_INSERTED :
/*N*/ 		case SDRUSERCALL_CHILD_REMOVED :
/*N*/ 			if( bInCntnt )
/*N*/ 			{
/*N*/ 				SwFrm *pAnch = GetAnchor();
/*N*/ 				if( !pAnch )
/*N*/ 				{
/*N*/ 					ConnectToLayout();
/*N*/ 					pAnch = GetAnchor();
/*N*/ 				}
/*N*/                 if( pAnch && !((SwTxtFrm*)pAnch)->IsLocked() &&
/*N*/                     &rObj == GetMaster() )
/*N*/ 				{
/*N*/ 					SwFrmFmt *pFmt = GetFmt();
/*N*/ 					const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
/*N*/                     SwTwips nRel;
/*N*/                     if( pAnch->IsVertical() )
/*N*/                     {
/*N*/                         nRel = rObj.GetRelativePos().X();
/*N*/                         if( !pAnch->IsReverse() )
/*N*/                             nRel = - nRel - rObj.GetSnapRect().GetWidth();
/*N*/                     }
/*N*/                     else
/*N*/                     {
/*N*/                         nRel = rObj.GetRelativePos().Y();
/*N*/                     }
/*N*/                     if( rVert.GetPos() != nRel )
/*N*/ 					{
/*N*/ 						SwFmtVertOrient aVert( rVert );
/*N*/ 						aVert.SetVertOrient( VERT_NONE );
/*N*/                         aVert.SetPos( nRel );
/*N*/ 						pFmt->SetAttr( aVert );
/*N*/ 					}
/*N*/ 					((SwTxtFrm*)pAnch)->Prepare();
/*N*/                     InvalidateAnchorOfVirtObjs();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if( bNotify )
/*N*/ 				lcl_Notify( this, pOldBoundRect );
/*N*/ 			break;
/*N*/ 		case SDRUSERCALL_CHGATTR:
/*N*/ 			if( bNotify )
/*N*/ 				lcl_Notify( this, pOldBoundRect );
/*N*/ 			break;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwDrawContact::Modify()
|*
|*	Ersterstellung		MA 09. Jan. 95
|*	Letzte Aenderung	MA 03. Dec. 95
|*
|*************************************************************************/

/*N*/ void SwDrawContact::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
/*N*/ {
/*N*/ 	//Es kommen immer Sets herein.
/*N*/ 	//MA 03. Dec. 95: Falsch es kommen nicht immer Sets herein
/*N*/ 	//(siehe SwRootFrm::AssertPageFlys()
/*N*/ 	USHORT nWhich = pNew ? pNew->Which() : 0;
/*N*/ 	SwFmtAnchor *pAnch = 0;
/*N*/ 	if( RES_ATTRSET_CHG == nWhich )
/*N*/ 	{
/*N*/ 		if(	SFX_ITEM_SET ==	((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
/*N*/ 							RES_ANCHOR, FALSE, (const SfxPoolItem**)&pAnch ))
/*N*/ 		;		// Beim GetItemState wird der AnkerPointer gesetzt !
/*N*/ 		else if( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->
/*N*/ 								 GetItemState( RES_VERT_ORIENT, FALSE ))
/*N*/ 		{
/*N*/ 			SwFrm *pFrm = GetAnchor();
/*N*/ 			if( !pFrm )
/*N*/ 			{
/*N*/ 				ConnectToLayout();
/*N*/ 				pFrm = GetAnchor();
/*N*/ 			}
/*N*/ 			if( pFrm && pFrm->IsTxtFrm() )
/*N*/ 				((SwTxtFrm*)pFrm)->Prepare();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( RES_ANCHOR == nWhich )
/*?*/ 		pAnch = (SwFmtAnchor*)pNew;
/*N*/ 
/*N*/ 	if ( pAnch )
/*N*/ 	{
/*N*/ 		// JP 10.04.95: nicht auf ein Reset Anchor reagieren !!!!!
/*N*/ 		if( SFX_ITEM_SET == ((SwFrmFmt*)pRegisteredIn)->GetAttrSet().
/*N*/ 			GetItemState( RES_ANCHOR, FALSE ) )
/*N*/ 		{
/*N*/             if( !FLY_IN_CNTNT == pAnch->GetAnchorId() )
/*N*/                 ((SwFrmFmt*)pRegisteredIn)->ResetAttr( RES_VERT_ORIENT );
/*N*/ 
/*N*/             SwFrm *pOldAnch = GetAnchor();
/*N*/ 			SwPageFrm *pPg = NULL;
/*N*/ 			SwRect aOldRect;
/*N*/ 			if( pOldAnch )
/*N*/ 			{
/*N*/ 				pPg = pOldAnch->FindPageFrm();
/*N*/ 				aOldRect = SwRect( GetMaster()->GetBoundRect() );
/*N*/ 			}
/*N*/ 			ConnectToLayout( pAnch );
/*N*/ 			if( pPg && aOldRect.HasArea() )
/*N*/ 				Notify_Background(GetMaster(),pPg,aOldRect,PREP_FLY_LEAVE,TRUE);
/*N*/ 			lcl_Notify( this, NULL );
/*N*/ 
/*N*/ 			// #86973#
/*N*/             if(GetMaster())
/*N*/ 			{
/*N*/ 				GetMaster()->SetChanged();
/*N*/ 				GetMaster()->SendRepaintBroadcast();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*?*/ 			DisconnectFromLayout();
/*N*/ 	}
/*N*/ 	else if( RES_SURROUND == nWhich || RES_UL_SPACE == nWhich ||
/*N*/ 			 RES_LR_SPACE == nWhich ||
/*N*/ 			 ( RES_ATTRSET_CHG == nWhich &&
/*N*/ 			   ( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
/*N*/ 							RES_SURROUND, FALSE ) ||
/*N*/ 				 SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
/*N*/ 							RES_LR_SPACE, FALSE ) ||
/*N*/ 				 SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
/*N*/ 							RES_UL_SPACE, FALSE ) ) ) )
/*N*/ 		lcl_Notify( this, NULL );
/*N*/ }


/*************************************************************************
|*
|*	SwDrawContact::DisconnectFromLayout()
|*
|*	Ersterstellung		MA 09. Jan. 95
|*	Letzte Aenderung	MA 25. Mar. 99
|*
|*************************************************************************/

void SwDrawContact::DisconnectFromLayout( bool _bMoveMasterToInvisibleLayer )
{
    // OD 16.05.2003 #108784# - remove 'virtual' drawing objects from writer
    // layout and from drawing page
    for ( std::list<SwDrawVirtObj*>::iterator aDisconnectIter = maDrawVirtObjs.begin();
          aDisconnectIter != maDrawVirtObjs.end();
          ++aDisconnectIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDisconnectIter);
        pDrawVirtObj->RemoveFromWriterLayout();
        pDrawVirtObj->RemoveFromDrawingPage();
    }

    if ( pAnchor )
    {
        pAnchor->RemoveDrawObj( this );
    }

    if ( _bMoveMasterToInvisibleLayer && GetMaster() && GetMaster()->IsInserted() )
    {
        SdrViewIter aIter( GetMaster() );
        for( SdrView* pView = aIter.FirstView(); pView;
                    pView = aIter.NextView() )
        {
            pView->MarkObj( GetMaster(), pView->GetPageViewPvNum(0), TRUE );
        }

        // OD 25.06.2003 #108784# - Instead of removing 'master' object from
        // drawing page, move the 'master' drawing object into the corresponding
        // invisible layer.
        {
            //((SwFrmFmt*)pRegisteredIn)->GetDoc()->GetDrawModel()->GetPage(0)->
            //                            RemoveObject( GetMaster()->GetOrdNum() );
            SwDoc* pWriterDoc = ((SwFrmFmt*)pRegisteredIn)->GetDoc();
            if ( pWriterDoc->IsVisibleLayerId( GetMaster()->GetLayer() ) )
            {
                SdrLayerID nInvisibleLayerId =
                    pWriterDoc->GetInvisibleLayerIdByVisibleOne( GetMaster()->GetLayer() );
                GetMaster()->SetLayer( nInvisibleLayerId );
            }
        }
    }
}

// OD 26.06.2003 #108784# - method to remove 'master' drawing object
// from drawing page.
void SwDrawContact::RemoveMasterFromDrawPage()
{
    if ( GetMaster() )
    {
        GetMaster()->SetUserCall( 0 );
        if ( GetMaster()->IsInserted() )
        {
            ((SwFrmFmt*)pRegisteredIn)->GetDoc()->GetDrawModel()->GetPage(0)->
                                        RemoveObject( GetMaster()->GetOrdNum() );
        }
    }
}

// OD 19.06.2003 #108784# - disconnect for a dedicated drawing object -
// could be 'master' or 'virtual'.
// a 'master' drawing object will disconnect a 'virtual' drawing object
// in order to take its place.
/*N*/ void SwDrawContact::DisconnectObjFromLayout( SdrObject* _pDrawObj )
/*N*/ {
/*N*/     if ( !GetAnchor()->FindFooterOrHeader() )
/*N*/     {
/*N*/         // disconnect completely from layout
/*N*/         DisconnectFromLayout();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         if ( _pDrawObj->ISA(SwDrawVirtObj) )
/*N*/         {
/*N*/             SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(_pDrawObj);
/*N*/             pDrawVirtObj->RemoveFromWriterLayout();
/*N*/             pDrawVirtObj->RemoveFromDrawingPage();
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             std::list<SwDrawVirtObj*>::const_iterator aFoundVirtObjIter =
/*N*/                     std::find_if( maDrawVirtObjs.begin(), maDrawVirtObjs.end(),
/*N*/                                   UsedOrUnusedVirtObjPred( true ) );
/*N*/             if ( aFoundVirtObjIter != maDrawVirtObjs.end() )
/*N*/             {
/*N*/                 // replace found 'virtual' drawing object by 'master' drawing
/*N*/                 // object and disconnect the 'virtual' one
/*N*/                 SwDrawVirtObj* pDrawVirtObj = (*aFoundVirtObjIter);
/*N*/                 SwFrm* pNewAnchorFrmOfMaster = pDrawVirtObj->GetAnchorFrm();
/*N*/                 Point aNewAnchorPos = pDrawVirtObj->GetAnchorPos();
/*N*/                 // disconnect 'virtual' drawing object
/*N*/                 pDrawVirtObj->RemoveFromWriterLayout();
/*N*/                 pDrawVirtObj->RemoveFromDrawingPage();
/*N*/                 // disconnect 'master' drawing object from current frame
/*N*/                 pAnchor->RemoveDrawObj( this );
/*N*/                 // re-connect 'master' drawing object to frame of found 'virtual'
/*N*/                 // drawing object.
/*N*/                 pNewAnchorFrmOfMaster->AppendDrawObj( this );
/*N*/                 // set anchor position at 'master' virtual drawing object to the
/*N*/                 // one of the found 'virtual' drawing object.
/*N*/                 GetMaster()->SetAnchorPos( aNewAnchorPos );
/*N*/                 // restore positions of remaining 'virtual' drawing objects
/*N*/                 CorrectRelativePosOfVirtObjs();
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 // no connected 'virtual' drawing object found. Thus, disconnect
/*N*/                 // completely from layout.
/*N*/                 DisconnectFromLayout();
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ }

/*************************************************************************
|*
|*	SwDrawContact::ConnectToLayout()
|*
|*	Ersterstellung		MA 09. Jan. 95
|*	Letzte Aenderung	MA 25. Mar. 99
|*
|*************************************************************************/

void SwDrawContact::ConnectToLayout( const SwFmtAnchor* pAnch )
{
    SwFrmFmt* pDrawFrmFmt = (SwFrmFmt*)pRegisteredIn;

    SwRootFrm* pRoot = pDrawFrmFmt->GetDoc()->GetRootFrm();
    if ( !pRoot )
    {
        return;
    }

    // OD 16.05.2003 #108784# - remove 'virtual' drawing objects from writer
    // layout and from drawing page, and remove 'master' drawing object from
    // writer layout - 'master' object will remain in drawing page.
    DisconnectFromLayout( false );

    if ( !pAnch )
    {
        pAnch = &(pDrawFrmFmt->GetAnchor());
    }

    bool bSetAnchorPos = true;

    switch ( pAnch->GetAnchorId() )
    {
        case FLY_PAGE:
            {
                USHORT nPgNum = pAnch->GetPageNum();
                SwPageFrm *pPage = static_cast<SwPageFrm*>(pRoot->Lower());

                for ( USHORT i = 1; i < nPgNum && pPage; ++i )
                {
                    pPage = static_cast<SwPageFrm*>(pPage->GetNext());
                }

                if ( pPage )
                {
                    bSetAnchorPos = false;
                    pPage->SwFrm::AppendDrawObj( this );
                }
                else
                    //Sieht doof aus, ist aber erlaubt (vlg. SwFEShell::SetPageObjsNewPage)
                    pRoot->SetAssertFlyPages();
            }
            break;

        case FLY_AUTO_CNTNT:
        case FLY_AT_CNTNT:
        case FLY_AT_FLY:
        case FLY_IN_CNTNT:
            {
                if ( pAnch->GetAnchorId() == FLY_IN_CNTNT )
                {
                    ClrContourCache( GetMaster() );
                }
                // OD 16.05.2003 #108784# - support drawing objects in header/footer,
                // but not control objects:
                // anchor at first found frame the 'master' object and
                // at the following frames 'virtual' drawing objects.
                // Note: method is similar to <SwFlyFrmFmt::MakeFrms(..)>
                SwModify *pModify = 0;
                if( pAnch->GetCntntAnchor() )
                {
                    if ( pAnch->GetAnchorId() == FLY_AT_FLY )
                    {
                        SwNodeIndex aIdx( pAnch->GetCntntAnchor()->nNode );
                        SwCntntNode* pCNd = pDrawFrmFmt->GetDoc()->GetNodes().GoNext( &aIdx );
                        SwClientIter aIter( *pCNd );
                        if ( aIter.First( TYPE(SwFrm) ) )
                            pModify = pCNd;
                        else
                        {
                            const SwNodeIndex& rIdx = pAnch->GetCntntAnchor()->nNode;
                            SwSpzFrmFmts& rFmts = *(pDrawFrmFmt->GetDoc()->GetSpzFrmFmts());
                            for( sal_uInt16 i = 0; i < rFmts.Count(); ++i )
                            {
                                SwFrmFmt* pFlyFmt = rFmts[i];
                                if( pFlyFmt->GetCntnt().GetCntntIdx() &&
                                    rIdx == *(pFlyFmt->GetCntnt().GetCntntIdx()) )
                                {
                                    pModify = pFlyFmt;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        pModify = pAnch->GetCntntAnchor()->nNode.GetNode().GetCntntNode();
                    }
                }
                SwClientIter aIter( *pModify );
                SwFrm* pAnchorFrmOfMaster = 0;
                for( SwFrm *pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
                     pFrm;
                     pFrm = (SwFrm*)aIter.Next() )
                {
                    // append drawing object, if
                    // (1) proposed anchor frame isn't a follow and
                    // (2) drawing object isn't a control object to be anchored
                    //     in header/footer.
                    const bool bAdd = ( !pFrm->IsCntntFrm() ||
                                        !((SwCntntFrm*)pFrm)->IsFollow() ) &&
                                      ( !CheckControlLayer( GetMaster() ) ||
                                        !pFrm->FindFooterOrHeader() );

                    if( bAdd )
                    {
                        if ( FLY_AT_FLY == pAnch->GetAnchorId() && !pFrm->IsFlyFrm() )
                        {
                            pFrm = pFrm->FindFlyFrm();
                        }

                        if ( !pAnchorFrmOfMaster )
                        {
                            // OD 02.07.2003 #108784# - check, if 'master' drawing
                            // objects is inserted into drawing page. If not,
                            // assert and insert 'master' drawing object.
                            if ( !GetMaster()->IsInserted() )
                            {
                                // OD 25.06.2003 #108784# - debug assert
                                ASSERT( false, "<SwDrawContact::ConnectToLayout(..)> - master drawing object not inserted!?" );

                                pDrawFrmFmt->GetDoc()->GetDrawModel()->GetPage(0)->
                                    InsertObject( GetMaster(), GetMaster()->GetOrdNumDirect() );
                            }
                            // append 'master' drawing object
                            pAnchorFrmOfMaster = pFrm;
                            pFrm->AppendDrawObj( this );
                        }
                        else
                        {
                            // append 'virtual' drawing object
                            SwDrawVirtObj* pDrawVirtObj = AddVirtObj();
                            if ( pAnch->GetAnchorId() == FLY_IN_CNTNT )
                            {
                                ClrContourCache( pDrawVirtObj );
                            }
                            pFrm->AppendVirtDrawObj( this, pDrawVirtObj );
                            pDrawVirtObj->SendRepaintBroadcast();
                        }

                        if ( pAnch->GetAnchorId() == FLY_IN_CNTNT )
                        {
                            pFrm->InvalidatePrt();
                        }

                        bSetAnchorPos = false;
                    }
                }
            }
            break;
/*
        case FLY_AT_FLY:
            {
                if( pAnch->GetCntntAnchor() ) // LAYER_IMPL
                {
                    SwFrm *pAnchor = 0;
                    //Erst einmal ueber den Inhalt suchen, weil konstant schnell. Kann
                    //Bei verketteten Rahmen aber auch schief gehen, weil dann evtl.
                    //niemals ein Frame zu dem Inhalt existiert. Dann muss leider noch
                    //die Suche vom StartNode zum FrameFormat sein.
                    SwNodeIndex aIdx( pAnch->GetCntntAnchor()->nNode );
                    SwCntntNode *pCNd = pFmt->GetDoc()->GetNodes().GoNext( &aIdx );
                    if ( pCNd && 0 != (pAnchor = pCNd->GetFrm( 0, 0, FALSE ) ) )
                        pAnchor = pAnchor->FindFlyFrm();
                    else
                    {
                        const SwNodeIndex &rIdx = pAnch->GetCntntAnchor()->nNode;
                        SwSpzFrmFmts& rFmts = *pFmt->GetDoc()->GetSpzFrmFmts();
                        for( USHORT i = 0; i < rFmts.Count(); ++i )
                        {
                            SwFrmFmt *pFmt = rFmts[i];
                            SwFlyFrmFmt* pFlyFmt;
                            if( 0 != (pFlyFmt = PTR_CAST( SwFlyFrmFmt, pFmt )) &&
                                pFlyFmt->GetCntnt().GetCntntIdx() && //#57390#, Reader
                                rIdx == *pFlyFmt->GetCntnt().GetCntntIdx() )
                            {
                                pAnchor = pFlyFmt->GetFrm( 0, FALSE );
                                break;
                            }
                        }
                    }
                    if ( pAnchor )	//Kann sein, dass der Anker noch nicht existiert
                    {
                        pAnchor->FindFlyFrm()->AppendDrawObj( this );
                        bSetAnchorPos = false;
                    }
                }
            }
            break;
*/
/*
        case FLY_IN_CNTNT:
            {
                ClrContourCache( GetMaster() );
                SwCntntNode *pNode = GetFmt()->GetDoc()->
                        GetNodes()[pAnch->GetCntntAnchor()->nNode]->GetCntntNode();
                SwCntntFrm *pCntnt = pNode->GetFrm( 0, 0, FALSE );
                if ( pCntnt )
                {
                    //Kann sein, dass der Anker noch nicht existiert
                    pCntnt->AppendDrawObj( this );
                    pCntnt->InvalidatePrt();
                }
                bSetAnchorPos = false;
            }
            break;
*/
#ifdef DBG_UTIL
        default:	ASSERT( FALSE, "Unknown Anchor." );
#endif
    }
    if ( GetAnchor() )
    {
        if( bSetAnchorPos )
        {
            GetMaster()->SetAnchorPos( GetAnchor()->GetFrmAnchorPos( ::binfilter::HasWrap( GetMaster() ) ) );
    }
        if ( !pDrawFrmFmt->GetDoc()->IsVisibleLayerId( GetMaster()->GetLayer() ) )
        {
            SdrLayerID nVisibleLayerId =
                pDrawFrmFmt->GetDoc()->GetVisibleLayerIdByInvisibleOne( GetMaster()->GetLayer() );
            GetMaster()->SetLayer( nVisibleLayerId );
        }
    }
}

// OD 20.06.2003 #108784# - method in order to correct relative positioning of
// 'virtual' drawing objects, e.g. caused by a <NbcSetAnchorPos(..)> at the
// 'master' drawing object.
void SwDrawContact::CorrectRelativePosOfVirtObjs()
{
    for ( std::list<SwDrawVirtObj*>::iterator aDrawVirtObjIter = maDrawVirtObjs.begin();
          aDrawVirtObjIter != maDrawVirtObjs.end();
          ++aDrawVirtObjIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDrawVirtObjIter);
        pDrawVirtObj->AdjustRelativePosToReference();
    }
}

// OD 20.06.2003 #108784# - correct in-direct movements of the 'virtual'
// drawing objects, e.g. caused by a <NbcMove(..)> at the 'master' drawing object
void SwDrawContact::MoveOffsetOfVirtObjs( const Size& _rMoveSize )
{
    for ( std::list<SwDrawVirtObj*>::iterator aDrawVirtObjIter = maDrawVirtObjs.begin();
          aDrawVirtObjIter != maDrawVirtObjs.end();
          ++aDrawVirtObjIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDrawVirtObjIter);
        pDrawVirtObj->SetOffset( pDrawVirtObj->GetOffset() - Point(_rMoveSize.Width(), _rMoveSize.Height()) );
    }
}

// OD 26.06.2003 #108784# - invalidate anchor frames of 'virtual' drawing objects.
// Needed for, e.g., 'as character' anchored drawing objects and a change of the
// 'master' drawing object.
void SwDrawContact::InvalidateAnchorOfVirtObjs()
{
    for ( std::list<SwDrawVirtObj*>::iterator aDrawVirtObjIter = maDrawVirtObjs.begin();
          aDrawVirtObjIter != maDrawVirtObjs.end();
          ++aDrawVirtObjIter )
    {
        SwDrawVirtObj* pDrawVirtObj = (*aDrawVirtObjIter);
        if ( pDrawVirtObj->GetAnchorFrm() )
        {
            pDrawVirtObj->GetAnchorFrm()->Prepare();
        }
    }
}

/*************************************************************************
|*
|*	SwDrawContact::FindPage(), ChkPage()
|*
|*	Ersterstellung		MA 21. Mar. 95
|*	Letzte Aenderung	MA 19. Jul. 96
|*
|*************************************************************************/

SwPageFrm* SwDrawContact::FindPage( const SwRect &rRect )
{
    SwPageFrm *pPg = pPage;
    if ( !pPg && pAnchor )
        pPg = pAnchor->FindPageFrm();
    if ( pPg )
        pPg = (SwPageFrm*)::binfilter::FindPage( rRect, pPg );
    return pPg;
}

void SwDrawContact::ChkPage()
{
    SwPageFrm* pPg = pAnchor && pAnchor->IsPageFrm() ?
        pPage : FindPage( GetMaster()->GetBoundRect() );
    if ( pPage != pPg )
    {
        // OD 27.06.2003 #108784# - if drawing object is anchor in header/footer
        // a change of the page is a dramatic change. Thus, completely re-connect
        // to the layout
        if ( pAnchor && pAnchor->FindFooterOrHeader() )
        {
            ConnectToLayout();
        }
        else
        {
            if ( pPage )
                pPage->SwPageFrm::RemoveDrawObj( this );
            pPg->SwPageFrm::AppendDrawObj( this );
            ChgPage( pPg );
        }
    }
}

/*************************************************************************
|*
|*	SwDrawContact::ChangeMasterObject()
|*
|*	Ersterstellung		MA 07. Aug. 95
|*	Letzte Aenderung	MA 20. Apr. 99
|*
|*************************************************************************/

// OD 10.07.2003 #110742# - Important note:
// method is called by method <SwDPage::ReplaceObject(..)>, which called its
// corresponding superclass method <FmFormPage::ReplaceObject(..)>.
// Note: 'master' drawing object *has* to be connected to layout (GetAnchor()
//       returns a frame), if method is called.

// =============================================================================
/** implementation of class <SwDrawVirtObj>

    OD 14.05.2003 #108784#

    @author OD
*/

TYPEINIT1(SwDrawVirtObj,SdrVirtObj);

// #108784#
// implemetation of SwDrawVirtObj
SwDrawVirtObj::SwDrawVirtObj( SdrObject&        _rNewObj,
                              SwDrawContact&    _rDrawContact )
    : SdrVirtObj( _rNewObj ),
      mpAnchorFrm( 0 ),
      mpPageFrm( 0 ),
      mrDrawContact( _rDrawContact )
{}

SwDrawVirtObj::~SwDrawVirtObj()
{}

void SwDrawVirtObj::SetOffset(const Point& rNewOffset)
{
    maOffset = rNewOffset;
    SetRectsDirty();
}

const Point SwDrawVirtObj::GetOffset() const
{
    return maOffset;
}

void SwDrawVirtObj::operator=( const SdrObject& rObj )
{
DBG_BF_ASSERT(0, "STRIP");//STRIP001 
}

SdrObject* SwDrawVirtObj::Clone() const
{
    SwDrawVirtObj* pObj = new SwDrawVirtObj( rRefObj, mrDrawContact );

    if ( pObj )
    {
        pObj->operator=(static_cast<const SdrObject&>(*this));
    //*pObj = *this;

        // members <mpAnchorFrm> and <mpPageFrm> not cloned.
    }

    return pObj;
}

// --------------------------------------------------------------------
// connection to writer layout: <SetAnchorFrm(..)>, <GetAnchorFrm()>,
// <SetPageFrm(..)>, <GetPageFrm()> and <RemoveFromWriterLayout()>
// --------------------------------------------------------------------
void SwDrawVirtObj::SetAnchorFrm( SwFrm* _pNewAnchorFrm )
{
    mpAnchorFrm = _pNewAnchorFrm;
}

SwFrm* SwDrawVirtObj::GetAnchorFrm() const
{
    return mpAnchorFrm;
}

void SwDrawVirtObj::SetPageFrm( SwPageFrm* _pNewPageFrm )
{
    mpPageFrm = _pNewPageFrm;
}

SwPageFrm* SwDrawVirtObj::GetPageFrm() const
{
    return mpPageFrm;
}

void SwDrawVirtObj::RemoveFromWriterLayout()
{
    // remove contact object from frame for 'virtual' drawing object
    if ( GetAnchorFrm() )
    {
        GetAnchorFrm()->RemoveVirtDrawObj( &mrDrawContact, this );
    }
}

// --------------------------------------------------------------------
// connection to writer layout: <AddToDrawingPage()>, <RemoveFromDrawingPage()>
// --------------------------------------------------------------------
void SwDrawVirtObj::AddToDrawingPage()
{
    // determine 'master'
    SdrObject* pOrgMasterSdrObj = mrDrawContact.GetMaster();

    // insert 'virtual' drawing object into page, set layer and user call.
    SdrPage *pPg;
    if ( 0 != ( pPg = pOrgMasterSdrObj->GetPage() ) )
    {
        pPg->InsertObject( this );
    }
    SetUserCall( &mrDrawContact );
}

void SwDrawVirtObj::RemoveFromDrawingPage()
{
    SetUserCall( 0 );
    if ( GetPage() )
    {
        GetPage()->RemoveObject( GetOrdNum() );
    }
}

// is 'virtual' drawing object connected to writer layout and  to drawing layer.
bool SwDrawVirtObj::IsConnected() const
{
    bool bRetVal = GetAnchorFrm() &&
                   ( GetPage() && GetUserCall() );

    return bRetVal;
}

// OD 19.06.2003 #108784# - adjust the relative postion of the 'virtual'
// drawing object to the reference object
void SwDrawVirtObj::AdjustRelativePosToReference()
{
    Point aVirtRelPos = GetRelativePos();
    Point aOrigialRelativePos = GetReferencedObj().GetRelativePos();
    Point aDiff = aVirtRelPos - aOrigialRelativePos;
    if( aDiff.X() || aDiff.Y() )
    {
        SetOffset( GetOffset() - aDiff );
    }
}

// #108784#
// All AnchorPos influencing methods which shall not change the
// original object positions.

void SwDrawVirtObj::NbcSetRelativePos(const Point& rPnt)
{
    ReferencedObj().NbcSetRelativePos( rPnt );
}

void SwDrawVirtObj::NbcSetAnchorPos(const Point& rPnt)
{
    Point aDelta(rPnt - aAnchor);

    aAnchor = rPnt;
    maOffset += aDelta;
    SetRectsDirty();
}

// #108784#
// All overloaded methods which need to use the offset to move
// the object position virtually.

const Rectangle& SwDrawVirtObj::GetBoundRect() const
{
    ((SwDrawVirtObj*)this)->aOutRect = rRefObj.GetBoundRect(); // Hier noch optimieren
    ((SwDrawVirtObj*)this)->aOutRect += maOffset;

    return aOutRect;
}

void SwDrawVirtObj::RecalcBoundRect()
{
    aOutRect = rRefObj.GetBoundRect();
    aOutRect += maOffset;
}

FASTBOOL SwDrawVirtObj::Paint(ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const
{
    FASTBOOL bRet;

    Point aOfs(rOut.GetOffset());
    rOut.SetOffset(aOfs + maOffset);
    if ( GetSubList() && GetSubList()->GetObjCount() )
    {
        SdrPaintInfoRec aCopyInfoRec = rInfoRec;
        aCopyInfoRec.aCheckRect.Move( -maOffset.X(), -maOffset.Y() );
        aCopyInfoRec.aDirtyRect.Move( -maOffset.X(), -maOffset.Y() );
        bRet = rRefObj.Paint(rOut, aCopyInfoRec);
    }
    else
    {
        bRet = rRefObj.Paint(rOut, rInfoRec);
    }
    rOut.SetOffset(aOfs);

    return bRet;
}

SdrObject* SwDrawVirtObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    Point aPnt(rPnt - maOffset);
    FASTBOOL bRet = rRefObj.CheckHit(aPnt, nTol, pVisiLayer) != NULL;

    return bRet ? (SdrObject*)this : NULL;
}

void SwDrawVirtObj::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const
{
    rRefObj.TakeXorPoly(rPoly, bDetail);
    rPoly.Move(maOffset.X(), maOffset.Y());
}

void SwDrawVirtObj::TakeContour(XPolyPolygon& rPoly) const
{
    rRefObj.TakeContour(rPoly);
    rPoly.Move(maOffset.X(), maOffset.Y());
}

SdrHdl* SwDrawVirtObj::GetHdl(USHORT nHdlNum) const
{DBG_BF_ASSERT(0, "STRIP");return NULL;//STRIP001 
}

SdrHdl* SwDrawVirtObj::GetPlusHdl(const SdrHdl& rHdl, USHORT nPlNum) const
{DBG_BF_ASSERT(0, "STRIP");return NULL;//STRIP001 
}

void SwDrawVirtObj::NbcMove(const Size& rSiz)
{
    rRefObj.NbcMove( rSiz );
    SetRectsDirty();
}

void SwDrawVirtObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    rRefObj.NbcResize(rRef - maOffset, xFact, yFact);
    SetRectsDirty();
}

void SwDrawVirtObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    rRefObj.NbcRotate(rRef - maOffset, nWink, sn, cs);
    SetRectsDirty();
}

void SwDrawVirtObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    rRefObj.NbcMirror(rRef1 - maOffset, rRef2 - maOffset);
    SetRectsDirty();
}

void SwDrawVirtObj::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    rRefObj.NbcShear(rRef - maOffset, nWink, tn, bVShear);
    SetRectsDirty();
}

void SwDrawVirtObj::Move(const Size& rSiz)
{
    Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetBoundRect();
    rRefObj.Move( rSiz );
    SetRectsDirty();
    SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
}

void SwDrawVirtObj::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    if(xFact.GetNumerator() != xFact.GetDenominator() || yFact.GetNumerator() != yFact.GetDenominator())
    {
        Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetBoundRect();
        rRefObj.Resize(rRef - maOffset, xFact, yFact);
        SetRectsDirty();
        SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
    }
}

void SwDrawVirtObj::Rotate(const Point& rRef, long nWink, double sn, double cs)
{
    if(nWink)
    {
        Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetBoundRect();
        rRefObj.Rotate(rRef - maOffset, nWink, sn, cs);
        SetRectsDirty();
        SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
    }
}

void SwDrawVirtObj::Mirror(const Point& rRef1, const Point& rRef2)
{DBG_BF_ASSERT(0, "STRIP");//STRIP001 
}

void SwDrawVirtObj::Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    if(nWink)
    {
        Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetBoundRect();
        rRefObj.Shear(rRef - maOffset, nWink, tn, bVShear);
        SetRectsDirty();
        SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
    }
}

void SwDrawVirtObj::RecalcSnapRect()
{
    aSnapRect = rRefObj.GetSnapRect();
    aSnapRect += maOffset;
}

const Rectangle& SwDrawVirtObj::GetSnapRect() const
{
    ((SwDrawVirtObj*)this)->aSnapRect = rRefObj.GetSnapRect();
    ((SwDrawVirtObj*)this)->aSnapRect += maOffset;

    return aSnapRect;
}

void SwDrawVirtObj::SetSnapRect(const Rectangle& rRect)
{
    Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetBoundRect();
    Rectangle aR(rRect);
    aR -= maOffset;
    rRefObj.SetSnapRect(aR);
    SetRectsDirty();
    SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
}

void SwDrawVirtObj::NbcSetSnapRect(const Rectangle& rRect)
{
    Rectangle aR(rRect);
    aR -= maOffset;
    SetRectsDirty();
    rRefObj.NbcSetSnapRect(aR);
}

const Rectangle& SwDrawVirtObj::GetLogicRect() const
{
    ((SwDrawVirtObj*)this)->aSnapRect = rRefObj.GetLogicRect();
    ((SwDrawVirtObj*)this)->aSnapRect += maOffset;

    return aSnapRect;
}

void SwDrawVirtObj::SetLogicRect(const Rectangle& rRect)
{
    Rectangle aBoundRect0; if(pUserCall) aBoundRect0 = GetBoundRect();
    Rectangle aR(rRect);
    aR -= maOffset;
    rRefObj.SetLogicRect(aR);
    SetRectsDirty();
    SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
}

void SwDrawVirtObj::NbcSetLogicRect(const Rectangle& rRect)
{
    Rectangle aR(rRect);
    aR -= maOffset;
    rRefObj.NbcSetLogicRect(aR);
    SetRectsDirty();
}

Point SwDrawVirtObj::GetSnapPoint(USHORT i) const
{DBG_BF_ASSERT(0, "STRIP");Point ap; return ap;//STRIP001 
}

const Point& SwDrawVirtObj::GetPoint(USHORT i) const
{
    ((SwDrawVirtObj*)this)->aHack = rRefObj.GetPoint(i);
    ((SwDrawVirtObj*)this)->aHack += maOffset;

    return aHack;
}

void SwDrawVirtObj::NbcSetPoint(const Point& rPnt, USHORT i)
{
    Point aP(rPnt);
    aP -= maOffset;
    rRefObj.SetPoint(aP, i);
    SetRectsDirty();
}

// #108784#
FASTBOOL SwDrawVirtObj::HasTextEdit() const
{
    return rRefObj.HasTextEdit();
}

// OD 18.06.2003 #108784# - overloaded 'layer' methods for 'virtual' drawing
// object to assure, that layer of 'virtual' object is the layer of the referenced
// object.
// NOTE: member <
SdrLayerID SwDrawVirtObj::GetLayer() const
{
    return GetReferencedObj().GetLayer();
}

void SwDrawVirtObj::NbcSetLayer(SdrLayerID nLayer)
{
    ReferencedObj().NbcSetLayer( nLayer );
    SdrVirtObj::NbcSetLayer( ReferencedObj().GetLayer() );
}

void SwDrawVirtObj::SetLayer(SdrLayerID nLayer)
{
    ReferencedObj().SetLayer( nLayer );
    SdrVirtObj::NbcSetLayer( ReferencedObj().GetLayer() );
}
// eof



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
