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
#include <hintids.hxx>		// fuer RES_..
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _SWFNTCCH_HXX
#include <swfntcch.hxx>
#endif
namespace binfilter {

static SwClientIter* pClientIters = 0;

/*N*/ TYPEINIT0(SwClient);	//rtti

/*************************************************************************
|*	  SwClient::SwClient(SwModify *)
|*
|*	  Beschreibung		callback.doc V1.14
|*	  Ersterstellung	VB 20.03.91
|*	  Letzte Aenderung	MA 20. Mar. 95
*************************************************************************/


/*N*/ SwClient::SwClient(SwModify *pToRegisterIn)
/*N*/ 	: pLeft( 0 ), pRight( 0 ), pRegisteredIn( 0 )
/*N*/ {
/*N*/ 	bModifyLocked =
/*N*/ 	bInModify =
/*N*/ 	bInDocDTOR =
/*N*/ 	bInCache = FALSE;
/*N*/ 	bInSwFntCache = FALSE;
/*N*/ 
/*N*/ 	if(pToRegisterIn)
/*N*/ 		pToRegisterIn->Add(this);
/*N*/ }

/*************************************************************************
|*	  SwClient::Modify()
|*
|*	  Beschreibung		callback.doc V1.14
|*	  Ersterstellung	VB 20.03.91
|*	  Letzte Aenderung	VB 20.03.91
*************************************************************************/


/*N*/ void SwClient::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
/*N*/ {
/*N*/ 	if( (!pOld || pOld->Which() != RES_OBJECTDYING) )
/*N*/ 		return;
/*N*/ 
/*N*/ 	SwPtrMsgPoolItem *pDead = (SwPtrMsgPoolItem *)pOld;
/*N*/ 	if(pDead->pObject == pRegisteredIn)
/*N*/ 	{
/*N*/ 		SwModify *pAbove = (SwModify*)pRegisteredIn->GetRegisteredIn();
/*N*/ 		if(pAbove)
/*N*/ 		{
/*?*/ 			pAbove->Add(this);
/*?*/ 			return;
/*N*/ 		}
/*N*/ 		pRegisteredIn->Remove(this);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*	  SwClient::~SwClient()
|*
|*	  Beschreibung		callback.doc V1.14
|*	  Ersterstellung	VB 20.03.91
|*	  Letzte Aenderung	MA 25. Jan. 94
*************************************************************************/


/*N*/ SwClient::~SwClient()
/*N*/ {
/*N*/ 	if( pRegisteredIn && pRegisteredIn->GetDepends() )
/*N*/ 		pRegisteredIn->Remove( this );
/*N*/ 
/*N*/ 	ASSERT( !IsModifyLocked(), "Modify destroyed but locked." );
/*N*/ }


    // erfrage vom Client Informationen
/*N*/ BOOL SwClient::GetInfo( SfxPoolItem& ) const
/*N*/ {
/*N*/ 	return TRUE;		// und weiter
/*N*/ }

/*************************************************************************
|*	  SwModify::SwModify( SwModify * )
|*
|*	  Beschreibung		Dokument 1.7
|*	  Ersterstellung	JP 20.11.90
|*	  Letzte Aenderung	VB 20.03.91
*************************************************************************/


/*N*/ SwModify::SwModify( SwModify *pToRegisterIn )
/*N*/ 	: SwClient(pToRegisterIn), pRoot( 0 )
/*N*/ {
/*N*/ }

/*************************************************************************
|*	  SwModify::~SwModify()
|*
|*	  Beschreibung		Dokument 1.7
|*	  Ersterstellung	JP 20.11.90
|*	  Letzte Aenderung	JP 15.04.94
*************************************************************************/



/*N*/ SwModify::~SwModify()
/*N*/ {
/*N*/ 	if ( IsInCache() )
/*N*/ 		SwFrm::GetCache().Delete( this );
/*N*/ 
/*N*/ 	if ( IsInSwFntCache() )
/*N*/ 		pSwFontCache->Delete( this );
/*N*/ 
/*N*/ 	if( pRoot )
/*N*/ 	{
/*N*/ 		if( IsInDocDTOR() )
/*N*/ 		{
/*N*/ 			// alle Clients "logisch" austragen
/*N*/ 			SwClientIter aIter( *this );
/*N*/ 			SwClient* p;
/*N*/ 			while( 0 != ( p = aIter++ ) )
/*N*/ 				p->pRegisteredIn = 0;
/*N*/ 
/*N*/ 			p = aIter.GoRoot(); 		// wieder ab Root (==Start) anfangen
/*N*/ 			do {
/*N*/ 				p->pRegisteredIn = 0;
/*N*/ 			} while( 0 != ( p = aIter-- ) );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// verschicke die Nachricht an die abgeleiteten Objekte.
/*N*/ 			SwPtrMsgPoolItem aDyObject( RES_OBJECTDYING, this );
/*N*/ 			Modify( &aDyObject, &aDyObject );
/*N*/ 
/*N*/ 			// Zwangsummeldung aller derjenigen, die sich nicht ausgetragen
/*N*/ 			// haben, durch Aufruf von SwClient::Modify()
/*N*/ 			while( pRoot )
/*N*/ 				pRoot->SwClient::Modify(&aDyObject, &aDyObject);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*	  SwModify::Modify( SwHint * pOldValue, SwHint * pNewValue )
|*
|*	  Beschreibung		Dokument 1.7
|*	  Ersterstellung	JP 20.11.90
|*	  Letzte Aenderung	MA 20. Mar. 95
*************************************************************************/



/*N*/ void SwModify::Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue )
/*N*/ {
/*N*/ 	if (IsInCache() || IsInSwFntCache())
/*N*/ 	{
/*N*/ 		const USHORT nWhich = pOldValue ? pOldValue->Which() :
/*N*/ 										pNewValue ? pNewValue->Which() : 0;
/*N*/ 		CheckCaching( nWhich );
/*N*/ 	}
/*N*/ 
/*N*/   	if (!pRoot || IsModifyLocked())
/*N*/ 		return;
/*N*/ 
/*N*/ 	LockModify();
/*N*/ 
/*N*/ #ifndef DBG_UTIL
/*N*/ 	bInModify = TRUE;
/*N*/ #else
/*N*/ 	if( !pOldValue )
/*N*/ 		bInModify = TRUE;
/*N*/ 	else
/*N*/ 		// following Modifies don't calls an ASSRT
/*N*/ 		switch( pOldValue->Which() )
/*N*/ 		{
/*N*/ 		case RES_OBJECTDYING:
/*N*/  		case RES_REMOVE_UNO_OBJECT:
/*N*/ 			bInModify = ((SwPtrMsgPoolItem *)pOldValue)->pObject != this;
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_FOOTNOTE_DELETED:
/*N*/ 		case RES_REFMARK_DELETED:
/*N*/ 		case RES_TOXMARK_DELETED:
/*N*/ 		case RES_FIELD_DELETED:
/*N*/ 			bInModify = FALSE;
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			bInModify = TRUE;
/*N*/ 		}
/*N*/ #endif
/*N*/ 
/*N*/ 	SwClientIter aIter( *this );
/*N*/ 	SwClient * pLast = aIter.GoStart();
/*N*/ 	if( pLast ) 	// konnte zum Anfang gesprungen werden ??
/*N*/ 		do {
/*N*/ 			pLast->Modify( pOldValue, pNewValue );
/*N*/ 			if( !pRoot )	// Baum schon Weg ??
/*N*/ 				break;
/*N*/ 		} while( 0 != ( pLast = aIter++ ));
/*N*/ 
/*N*/ 	bInModify = FALSE;
/*N*/ 	UnlockModify();
/*N*/ }

// erfrage vom Modify Informationen

/*N*/ BOOL SwModify::GetInfo( SfxPoolItem& rInfo ) const
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;		// bedeutet weiter zum naechsten
/*N*/ 
/*N*/ 	if( pRoot )
/*N*/ 	{
/*N*/ 		SwClientIter aIter( *(SwModify*)this );
/*N*/ 
/*N*/ 		SwClient* pLast = aIter.GoStart();
/*N*/ 		if( pLast )
/*N*/ 			while( 0 != ( bRet = pLast->GetInfo( rInfo )) &&
/*N*/ 					0 != ( pLast = aIter++ ) )
/*N*/ 				;
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRet;		// und weiter
/*N*/ }

/*************************************************************************
|*	  SwModify::Add( SwClient *pDepend )
|*
|*	  Beschreibung		Dokument 1.7
|*	  Ersterstellung	JP 20.11.90
|*	  Letzte Aenderung	JP 14.09.94
*************************************************************************/



/*N*/ void SwModify::Add(SwClient *pDepend)
/*N*/ {
/*N*/ 	ASSERT( !bInModify, "Client innerhalb des eigenen Modifies einfuegen?" );
/*N*/ 
/*N*/ 	// nur wenn das hier noch nicht eingetragen ist einfuegen
/*N*/ 	if(pDepend->pRegisteredIn != this )
/*N*/ 	{
/*N*/ #ifdef DBG_UTIL
/*N*/ 		SwClientIter* pTmp = pClientIters;
/*N*/ 		while( pTmp )
/*N*/ 		{
/*N*/ 			ASSERT( &pTmp->rRoot != pRoot,
/*N*/ 				"Client beim angemeldeten ClientIter einfuegen?" );
/*N*/ 			pTmp = pTmp->pNxtIter;
/*N*/ 		}
/*N*/ #endif
/*N*/ 		// wenn schon wanders angemeldet, dann dort abmelden
/*N*/ 		if( pDepend->pRegisteredIn != 0 )
/*N*/ 			pDepend->pRegisteredIn->Remove( pDepend );
/*N*/ 
/*N*/ 		if( !pRoot )
/*N*/ 		{
/*N*/ 			pRoot = pDepend;
/*N*/ 			pRoot->pLeft = 0;
/*N*/ 			pRoot->pRight = 0;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// immer hinter die Root haengen
/*N*/ 			pDepend->pRight = pRoot->pRight;
/*N*/ 			pRoot->pRight = pDepend;
/*N*/ 			pDepend->pLeft = pRoot;
/*N*/ 			if( pDepend->pRight )
/*N*/ 				pDepend->pRight->pLeft = pDepend;
/*N*/ 		}
/*N*/ 
/*N*/ 		pDepend->pRegisteredIn = this;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*	  SwModify::_Remove( SwClient *pDepend )
|*
|*	  Beschreibung		Dokument 1.7
|*	  Ersterstellung	JP 20.11.90
|*	  Letzte Aenderung	JP 14.09.94
*************************************************************************/



/*N*/ SwClient *SwModify::_Remove(SwClient * pDepend)
/*N*/ {
/*N*/ 	ASSERT( !bInModify, "Client innerhalb des eigenen Modifies loeschen?" );
/*N*/ 
/*N*/ 	// loesche das Object aus der Liste und setze den
/*N*/ 	// Registrierungs-Pointer zurueck
/*N*/ 	if( pDepend->pRegisteredIn == this )
/*N*/ 	{
/*N*/ 		register SwClient* pR = pDepend->pRight;
/*N*/ 		register SwClient* pL = pDepend->pLeft;
/*N*/ 		if( pRoot == pDepend )
/*N*/ 			pRoot = pL ? pL : pR;
/*N*/ 
/*N*/ 		if( pL )
/*N*/ 			pL->pRight = pR;
/*N*/ 		if( pR )
/*N*/ 			pR->pLeft = pL;
/*N*/ 
/*N*/ 		// alle Client-Iters updaten
/*N*/ 		SwClientIter* pTmp = pClientIters;
/*N*/ 		while( pTmp )
/*N*/ 		{
/*N*/ 			if( pTmp->pAkt == pDepend || pTmp->pDelNext == pDepend )
/*N*/ 				pTmp->pDelNext = pR;
/*N*/ 			pTmp = pTmp->pNxtIter;
/*N*/ 		}
/*N*/ 
/*N*/ 		pDepend->pLeft = 0;
/*N*/ 		pDepend->pRight = 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		ASSERT( FALSE, "SwModify::Remove(): pDepend nicht gefunden");
/*N*/ 	pDepend->pRegisteredIn = 0;
/*N*/ 	return pDepend;
/*N*/ }


/*************************************************************************
|*	  SwModify::CheckCaching( const USHORT nWhich )
|*
|*	  Ersterstellung	JP 25.06.95
|*	  Letzte Aenderung	JP 25.06.95
*************************************************************************/



/*N*/ void SwModify::CheckCaching( const USHORT nWhich )
/*N*/ {
/*N*/ 	if( RES_CHRATR_BEGIN <= nWhich && RES_CHRATR_END > nWhich )
/*N*/ 		SetInSwFntCache( FALSE );
/*N*/ 	else
/*N*/ 		switch ( nWhich )
/*N*/ 		{
/*N*/ 		case RES_OBJECTDYING:
/*N*/ 		case RES_FMT_CHG:
/*N*/ 		case RES_ATTRSET_CHG:
/*N*/ 			SetInSwFntCache( FALSE );
/*N*/ 
/*N*/ 		case RES_UL_SPACE:
/*N*/ 		case RES_LR_SPACE:
/*N*/ 		case RES_BOX:
/*N*/ 		case RES_SHADOW:
/*N*/ 		case RES_FRM_SIZE:
/*N*/ 		case RES_KEEP:
/*N*/ 		case RES_BREAK:
/*N*/ 			if ( IsInCache() )
/*N*/ 			{
/*N*/ 				SwFrm::GetCache().Delete( this );
/*N*/ 				SetInCache( FALSE );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		}
/*N*/ }


// ----------
// SwDepend
// ----------

/*************************************************************************
|*	  SwDepend::SwDepend(SwClient *pTellHim,SwModify *pDepend)
|*
|*	  Beschreibung		callback.doc V1.14
|*	  Ersterstellung	VB 20.03.91
|*	  Letzte Aenderung	VB 20.03.91
*************************************************************************/


/*N*/ SwDepend::SwDepend(SwClient *pTellHim, SwModify *pDepend)
/*N*/ 	: SwClient(pDepend)
/*N*/ {
/*N*/ 	pToTell  = pTellHim;
/*N*/ }

/*************************************************************************
|*
|*	  SwDepend::Modify(SwHint *, SwHint *)
|*
|*	  Beschreibung		callback.doc V1.14
|*	  Ersterstellung	VB 20.03.91
|*	  Letzte Aenderung	VB 20.03.91
|*
*************************************************************************/


/*N*/ void SwDepend::Modify( SfxPoolItem *pOldValue, SfxPoolItem *pNewValue )
/*N*/ {
/*N*/ 	if(pNewValue && pNewValue->Which() == RES_OBJECTDYING)
/*N*/ 		SwClient::Modify(pOldValue,pNewValue);
/*N*/ 	else if(pToTell)
/*N*/ 		pToTell->Modify(pOldValue, pNewValue);
/*N*/ }


    // erfrage vom Modify Informationen
/*N*/ BOOL SwDepend::GetInfo( SfxPoolItem& rInfo ) const
/*N*/ {
/*N*/ 	return pToTell ? pToTell->GetInfo( rInfo ) : TRUE;
/*N*/ }

/********************************************************************/


/*N*/ SwClientIter::SwClientIter( SwModify& rModify )
/*N*/ 	: rRoot( rModify )
/*N*/ {
/*N*/ 	// hinten einketten!
/*N*/ 	pNxtIter = 0;
/*N*/ 	if( pClientIters )
/*N*/ 	{
/*N*/ 		SwClientIter* pTmp = pClientIters;
/*N*/ 		while( pTmp->pNxtIter )
/*N*/ 			pTmp = pTmp->pNxtIter;
/*N*/ 		pTmp->pNxtIter = this;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pClientIters = this;
/*N*/ 
/*N*/ 	pAkt = rRoot.pRoot;
/*N*/ 	pDelNext = pAkt;
/*N*/ }



/*N*/ SwClientIter::~SwClientIter()
/*N*/ {
/*N*/ 	if( pClientIters )
/*N*/ 	{
/*N*/ 		if( pClientIters == this )
/*N*/ 			pClientIters = pNxtIter;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SwClientIter* pTmp = pClientIters;
/*N*/ 			while( pTmp->pNxtIter != this )
/*N*/ 				if( 0 == ( pTmp = pTmp->pNxtIter ) )
/*N*/ 				{
/*N*/ 					ASSERT( this, "wo ist mein Pointer" );
/*N*/ 					return ;
/*N*/ 				}
/*N*/ 			pTmp->pNxtIter = pNxtIter;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ #ifndef CFRONT
/*N*/ 	// Postfix Operator
/*N*/ SwClient* SwClientIter::operator++(int)
/*N*/ {
/*N*/ // solange der CFRONT Prefix und PostFix nicht unterscheiden kann, immer
/*N*/ // als Prefix-Operator arbeiten. Wenn der CFRONT es kann, muss auch der
/*N*/ // Code entsprechen umgestellt werden !!!
/*N*/ 	if( pDelNext == pAkt )
/*N*/ 	{
/*N*/ 		pAkt = pAkt->pRight;
/*N*/ 		pDelNext = pAkt;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pAkt = pDelNext;
/*N*/ 	return pAkt;
/*N*/ }
/*N*/ #endif


    // Prefix Operator
/*N*/ SwClient* SwClientIter::operator++()
/*N*/ {
/*N*/ 	if( pDelNext == pAkt )
/*N*/ 	{
/*N*/ 		pAkt = pAkt->pRight;
/*N*/ 		pDelNext = pAkt;
/*N*/ 	}
/*N*/ 	else
/*?*/ 		pAkt = pDelNext;
/*N*/ 	return pAkt;
/*N*/ }


/*N*/ #ifndef CFRONT
    // Postfix Operator
/*N*/ SwClient* SwClientIter::operator--(int)
/*N*/ {
// solange der CFRONT Prefix und PostFix nicht unterscheiden kann, immer
// als Prefix-Operator arbeiten. Wenn der CFRONT es kann, muss auch der
// Code entsprechen umgestellt werden !!!
/*N*/ 	if( pDelNext == pAkt )
/*?*/ 		pAkt = pAkt->pLeft;
/*N*/ 	else
/*N*/ 		pAkt = pDelNext->pLeft;
/*N*/ 	pDelNext = pAkt;
/*N*/ 	return pAkt;
/*N*/ }
/*N*/ #endif


    // Prefix Operator
/*N*/ SwClient* SwClientIter::operator--()
/*N*/ {
/*N*/ 	if( pDelNext == pAkt )
/*N*/ 		pAkt = pAkt->pLeft;
/*N*/ 	else
/*N*/ 		pAkt = pDelNext->pLeft;
/*N*/ 	pDelNext = pAkt;
/*N*/ 	return pAkt;
/*N*/ }


/*N*/ SwClient* SwClientIter::GoStart() 		// zum Anfang des Baums
/*N*/ {
/*N*/ 	pAkt = rRoot.pRoot;
/*N*/ 	if( pAkt )
/*N*/ 		while( pAkt->pLeft )
/*N*/ 			pAkt = pAkt->pLeft;
/*N*/ 	pDelNext = pAkt;
/*N*/ 	return pAkt;
/*N*/ }


/*N*/ SwClient* SwClientIter::GoEnd()			// zum End des Baums
/*N*/ {
/*N*/ 	pAkt = pDelNext;
/*N*/ 	if( !pAkt )
/*N*/ 		pAkt = rRoot.pRoot;
/*N*/ 	if( pAkt )
/*N*/ 		while( pAkt->pRight )
/*N*/ 			pAkt = pAkt->pRight;
/*N*/ 	pDelNext = pAkt;
/*N*/ 	return pAkt;
/*N*/ }



/*N*/ SwClient* SwClientIter::First( TypeId nType )
/*N*/ {
/*N*/ 	aSrchId = nType;
/*N*/ 	GoStart();
/*N*/ 	if( pAkt )
/*N*/ 		do {
/*N*/ 			if( pAkt->IsA( aSrchId ) )
/*N*/ 				break;
/*N*/ 
/*N*/ 			if( pDelNext == pAkt )
/*N*/ 			{
/*N*/ 				pAkt = pAkt->pRight;
/*N*/ 				pDelNext = pAkt;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				pAkt = pDelNext;
/*N*/ 
/*N*/ 		} while( pAkt );
/*N*/ 	return pAkt;
/*N*/ }


/*N*/ SwClient* SwClientIter::Next()
/*N*/ {
/*N*/ 	do {
/*N*/ 		// erstmal zum naechsten
/*N*/ 		if( pDelNext == pAkt )
/*N*/ 		{
/*N*/ 			pAkt = pAkt->pRight;
/*N*/ 			pDelNext = pAkt;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pAkt = pDelNext;
/*N*/ 
/*N*/ 		if( pAkt && pAkt->IsA( aSrchId ) )
/*N*/ 			break;
/*N*/ 	} while( pAkt );
/*N*/ 	return pAkt;
/*N*/ }



}
