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

#ifndef _FRAME_HXX
#include <frame.hxx>			// fuer AttrCache
#endif

#ifndef _HINTS_HXX
#include <hints.hxx>			// fuer SwFmtChg
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>			// fuer SwParaFmt - SwHyphenBug
#endif
#ifndef _SWCACHE_HXX
#include <swcache.hxx>
#endif
namespace binfilter {


/*N*/ TYPEINIT1( SwFmt, SwClient );	//rtti fuer SwFmt

/*************************************************************************
|*    SwFmt::SwFmt
*************************************************************************/


/*N*/ SwFmt::SwFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
/*N*/ 			const USHORT* pWhichRanges, SwFmt *pDrvdFrm, USHORT nFmtWhich )
/*N*/ 	: SwModify( pDrvdFrm ),
/*N*/ 	aSet( rPool, pWhichRanges ),
/*N*/ 	nPoolFmtId( USHRT_MAX ),
/*N*/ 	nPoolHelpId( USHRT_MAX ),
/*N*/ 	nPoolHlpFileId( UCHAR_MAX ),
/*N*/ 	nFmtId( 0 ),
/*N*/ 	nWhichId( nFmtWhich )
/*N*/ {
/*N*/ 	aFmtName.AssignAscii( pFmtNm );
/*N*/ 	bWritten = bFmtInDTOR = bAutoUpdateFmt = bLayerFmt = FALSE; // LAYER_IMPL
/*N*/ 	bAutoFmt = TRUE;
/*N*/ 
/*N*/ 	if( pDrvdFrm )
/*N*/ 		aSet.SetParent( &pDrvdFrm->aSet );
/*N*/ }


/*N*/ SwFmt::SwFmt( SwAttrPool& rPool, const String &rFmtNm,
/*N*/ 			const USHORT* pWhichRanges, SwFmt *pDrvdFrm, USHORT nFmtWhich )
/*N*/ 	: SwModify( pDrvdFrm ),
/*N*/ 	aFmtName( rFmtNm ),
/*N*/ 	aSet( rPool, pWhichRanges ),
/*N*/ 	nPoolFmtId( USHRT_MAX ),
/*N*/ 	nPoolHelpId( USHRT_MAX ),
/*N*/ 	nPoolHlpFileId( UCHAR_MAX ),
/*N*/ 	nFmtId( 0 ),
/*N*/ 	nWhichId( nFmtWhich )
/*N*/ {
/*N*/ 	bWritten = bFmtInDTOR = bAutoUpdateFmt = bLayerFmt = FALSE; // LAYER_IMPL
/*N*/ 	bAutoFmt = TRUE;
/*N*/ 
/*N*/ 	if( pDrvdFrm )
/*N*/ 		aSet.SetParent( &pDrvdFrm->aSet );
/*N*/ }


/*?*/ SwFmt::SwFmt( SwAttrPool& rPool, const String &rFmtNm, USHORT nWhich1,
/*?*/ 				USHORT nWhich2, SwFmt *pDrvdFrm, USHORT nFmtWhich )
/*?*/ 	: SwModify( pDrvdFrm ),
/*?*/ 	aFmtName( rFmtNm ),
/*?*/ 	aSet( rPool, nWhich1, nWhich2 ),
/*?*/ 	nPoolFmtId( USHRT_MAX ),
/*?*/ 	nPoolHelpId( USHRT_MAX ),
/*?*/ 	nPoolHlpFileId( UCHAR_MAX ),
/*?*/ 	nFmtId( 0 ),
/*?*/ 	nWhichId( nFmtWhich )
/*?*/ {
/*?*/ 	bWritten = bFmtInDTOR = bAutoUpdateFmt = bLayerFmt = FALSE; // LAYER_IMPL
/*?*/ 	bAutoFmt = TRUE;
/*?*/ 	if( pDrvdFrm )
/*?*/ 		aSet.SetParent( &pDrvdFrm->aSet );
/*?*/ }


/*N*/ SwFmt::SwFmt( const SwFmt& rFmt )
/*N*/ 	: SwModify( rFmt.DerivedFrom() ),
/*N*/ 	aFmtName( rFmt.aFmtName ),
/*N*/ 	aSet( rFmt.aSet ),
/*N*/ 	nPoolFmtId( rFmt.GetPoolFmtId() ),
/*N*/ 	nPoolHelpId( rFmt.GetPoolHelpId() ),
/*N*/ 	nPoolHlpFileId( rFmt.GetPoolHlpFileId() ),
/*N*/ 	nFmtId( 0 ),
/*N*/ 	nWhichId( rFmt.nWhichId )
/*N*/ {
/*N*/ 	bWritten = bFmtInDTOR = bLayerFmt = FALSE; // LAYER_IMPL
/*N*/ 	bAutoFmt = rFmt.bAutoFmt;
/*N*/ 	bAutoUpdateFmt = rFmt.bAutoUpdateFmt;
/*N*/ 
/*N*/ 	if( rFmt.DerivedFrom() )
/*N*/ 		aSet.SetParent( &rFmt.DerivedFrom()->aSet );
/*N*/ 	// einige Sonderbehandlungen fuer Attribute
/*N*/ 	aSet.SetModifyAtAttr( this );
/*N*/ }

/*************************************************************************
|*    SwFmt &SwFmt::operator=(const SwFmt& aFmt)
|*
|*    Beschreibung		Dokument 1.14
|*    Ersterstellung    JP 22.11.90
|*    Letzte Aenderung  JP 05.08.94
*************************************************************************/


/*N*/ SwFmt &SwFmt::operator=(const SwFmt& rFmt)
/*N*/ {
/*N*/ 	nWhichId = rFmt.nWhichId;
/*N*/ 	nPoolFmtId = rFmt.GetPoolFmtId();
/*N*/ 	nPoolHelpId = rFmt.GetPoolHelpId();
/*N*/ 	nPoolHlpFileId = rFmt.GetPoolHlpFileId();
/*N*/ 
/*N*/ 	if ( IsInCache() )
/*N*/ 	{
/*?*/ 		SwFrm::GetCache().Delete( this );
/*?*/ 		SetInCache( FALSE );
/*N*/ 	}
/*N*/ 	SetInSwFntCache( FALSE );
/*N*/ 
/*N*/ 	// kopiere nur das Attribut-Delta Array
/*N*/ 	SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
/*N*/ 				aNew( *aSet.GetPool(), aSet.GetRanges() );
/*N*/ 	aSet.Intersect_BC( rFmt.aSet, &aOld, &aNew );
/*N*/ 	aSet.Put_BC( rFmt.aSet, &aOld, &aNew );
/*N*/ 
/*N*/ 	// einige Sonderbehandlungen fuer Attribute
/*N*/ 	aSet.SetModifyAtAttr( this );
/*N*/ 
/*N*/ 	// PoolItem-Attr erzeugen fuers Modify !!!
/*N*/ 	if( aOld.Count() )
/*N*/ 	{
/*N*/ 		SwAttrSetChg aChgOld( aSet, aOld );
/*N*/ 		SwAttrSetChg aChgNew( aSet, aNew );
/*N*/ 		Modify( &aChgOld, &aChgNew );        // alle veraenderten werden verschickt
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pRegisteredIn != rFmt.pRegisteredIn )
/*N*/ 	{
/*?*/ 		if( pRegisteredIn )
/*?*/ 			pRegisteredIn->Remove(this);
/*?*/ 		if(rFmt.pRegisteredIn)
/*?*/ 		{
/*?*/ 			rFmt.pRegisteredIn->Add(this);
/*?*/ 			aSet.SetParent( &rFmt.aSet );
/*?*/ 		}
/*?*/ 		else
/*?*/ 			aSet.SetParent( 0 );
/*N*/ 	}
/*N*/ 	bAutoFmt = rFmt.bAutoFmt;
/*N*/ 	bAutoUpdateFmt = rFmt.bAutoUpdateFmt;
/*N*/ 	return *this;
/*N*/ }

/*N*/ void SwFmt::SetName( const String& rNewName, sal_Bool bBroadcast )
/*N*/ {
/*N*/ 	ASSERT(!IsDefault(), "SetName: Defaultformat" );
/*N*/ 	if( bBroadcast )
/*N*/ 	{
/*N*/ 		SwStringMsgPoolItem aOld( RES_NAME_CHANGED, aFmtName );
/*N*/ 		SwStringMsgPoolItem aNew( RES_NAME_CHANGED, rNewName );
/*N*/ 		aFmtName = rNewName;
/*N*/ 		Modify( &aOld, &aNew );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aFmtName = rNewName;
/*N*/ 	}
/*N*/ }

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * diese Funktion wird in jedem Copy-Ctor gerufen, damit die
 * Attribute kopiert werden. Diese koennen erst kopiert werden,
 * wenn die abgeleitet Klasse existiert, denn beim Setzen der
 * Attribute wird die Which()-Methode gerufen, die hier in der
 * Basis-Klasse auf 0 defaultet ist.
 *
 * Zusatz: JP 8.4.1994
 * 	Wird ueber Dokumentgrenzen kopiert, so muss das neue Dokument
 *	mit angeben werden, in dem this steht. Z.Z. ist das fuers
 *	DropCaps wichtig, dieses haelt Daten, die tief kopiert werden
 *	muessen !!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


/*N*/ void SwFmt::CopyAttrs( const SwFmt& rFmt, BOOL bReplace )
/*N*/ {
/*N*/ 	// kopiere nur das Attribut-Delta Array
/*N*/ 	register SwCharFmt* pDropCharFmt = 0;
/*N*/ 
/*N*/ 	if ( IsInCache() )
/*N*/ 	{
/*?*/ 		SwFrm::GetCache().Delete( this );
/*?*/ 		SetInCache( FALSE );
/*N*/ 	}
/*N*/ 	SetInSwFntCache( FALSE );
/*N*/ 
/*N*/ 	// Sonderbehandlung fuer einige Attribute
/*N*/ 	SwAttrSet* pChgSet = (SwAttrSet*)&rFmt.aSet;
/*N*/ 
/*N*/ 	if( !bReplace )		// nur die neu, die nicht gesetzt sind ??
/*N*/ 	{
/*?*/ 		if( pChgSet == (SwAttrSet*)&rFmt.aSet )		// Set hier kopieren
/*?*/ 			pChgSet = new SwAttrSet( rFmt.aSet );
/*?*/ 		pChgSet->Differentiate( aSet );
/*N*/ 	}
/*N*/ 
/*N*/ 	// kopiere nur das Attribut-Delta Array
/*N*/ 	if( pChgSet->GetPool() != aSet.GetPool() )
/*N*/ 		pChgSet->CopyToModify( *this );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
/*N*/ 				  aNew( *aSet.GetPool(), aSet.GetRanges() );
/*N*/ 
/*N*/ 		if ( 0 != aSet.Put_BC( *pChgSet, &aOld, &aNew ) )
/*N*/ 		{
/*N*/ 			// einige Sonderbehandlungen fuer Attribute
/*N*/ 			aSet.SetModifyAtAttr( this );
/*N*/ 
/*N*/ 			SwAttrSetChg aChgOld( aSet, aOld );
/*N*/ 			SwAttrSetChg aChgNew( aSet, aNew );
/*N*/ 			Modify( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pChgSet != (SwAttrSet*)&rFmt.aSet )		// Set hier angelegt ?
/*?*/ 		delete pChgSet;
/*N*/ }

/*************************************************************************
|*    SwFmt::~SwFmt()
|*
|*    Beschreibung		Dokument 1.14
|*    Ersterstellung    JP 22.11.90
|*    Letzte Aenderung  JP 14.02.91
*************************************************************************/


/*N*/ SwFmt::~SwFmt()
/*N*/ {
/*N*/ 	/* das passiert bei der ObjectDying Message */
/*N*/ 	/* alle Abhaengigen auf DerivedFrom umhaengen */
/*N*/ 	if( GetDepends() )
/*N*/ 	{
/*N*/ 		ASSERT(DerivedFrom(), "SwFmt::~SwFmt: Def Abhaengige!" );
/*N*/ 
/*N*/ 		bFmtInDTOR = TRUE;
/*N*/ 
/*N*/ 		SwFmt *pParentFmt = DerivedFrom();
/*N*/ 		while( GetDepends() )
/*N*/ 		{
/*N*/ 			SwFmtChg aOldFmt(this);
/*N*/ 			SwFmtChg aNewFmt(pParentFmt);
/*N*/ 			SwClient * pDepend = (SwClient*)GetDepends();
/*N*/ 			pParentFmt->Add(pDepend);
/*N*/ 			pDepend->Modify(&aOldFmt, &aNewFmt);
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*************************************************************************
|*    void SwFmt::Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue )
|*
|*    Beschreibung		Dokument 1.14
|*    Ersterstellung    JP 22.11.90
|*    Letzte Aenderung  JP 05.08.94
*************************************************************************/


/*N*/ void SwFmt::Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue )
/*N*/ {
/*N*/ 	BOOL bWeiter = TRUE;	// TRUE = Propagierung an die Abhaengigen
/*N*/ 
/*N*/ 	USHORT nWhich = pOldValue ? pOldValue->Which() :
/*N*/ 					pNewValue ? pNewValue->Which() : 0 ;
/*N*/ 	switch( nWhich )
/*N*/ 	{
/*?*/ 	case 0:		break;			// Which-Id von 0 ???
/*?*/ 
/*?*/ 	case RES_OBJECTDYING :
/*?*/ 		{
/*?*/ 			// ist das sterbende Object das "Parent"-Format von diesen Format,
/*?*/ 			// dann haengt sich dieses Format an den Parent vom Parent
/*?*/ 			SwFmt * pFmt = (SwFmt *) ((SwPtrMsgPoolItem *)pNewValue)->pObject;
/*?*/ 
/*?*/ 			// nicht umhaengen wenn dieses das oberste Format ist !!
/*?*/ 			if( pRegisteredIn && pRegisteredIn == pFmt )
/*?*/ 			{
/*?*/ 				if( pFmt->pRegisteredIn )
/*?*/ 				{
/*?*/ 					// wenn Parent, dann im neuen Parent wieder anmelden
/*?*/ 					pFmt->DerivedFrom()->Add( this );
/*?*/ 					aSet.SetParent( &DerivedFrom()->aSet );
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					// sonst auf jeden Fall beim sterbenden abmelden
/*?*/ 					DerivedFrom()->Remove( this );
/*?*/ 					aSet.SetParent( 0 );
/*?*/ 				}
/*?*/ 			}
/*?*/ 		} // OBJECTDYING
/*?*/ 		break;
/*?*/ 
/*N*/ 	case RES_ATTRSET_CHG:
/*N*/ 		if( ((SwAttrSetChg*)pOldValue)->GetTheChgdSet() != &aSet )
/*N*/ 		{
/*N*/ 			//nur die weiter geben, die hier nicht gesetzt sind !!
/*N*/ 			SwAttrSetChg aOld( *(SwAttrSetChg*)pOldValue );
/*N*/ 			SwAttrSetChg aNew( *(SwAttrSetChg*)pNewValue );
/*N*/ 
/*N*/ 			aOld.GetChgSet()->Differentiate( aSet );
/*N*/ 			aNew.GetChgSet()->Differentiate( aSet );
/*N*/ 
/*N*/ 			if( aNew.Count() )
/*N*/ 				// keine mehr gesetzt, dann Ende !!
/*N*/ 				SwModify::Modify( &aOld, &aNew );
/*N*/ 			bWeiter = FALSE;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case RES_FMT_CHG:
/*N*/ 		// falls mein Format Parent umgesetzt wird, dann melde ich
/*N*/ 		// meinen Attrset beim Neuen an.
/*N*/ 
/*N*/ 		// sein eigenes Modify ueberspringen !!
/*N*/ 		if( ((SwFmtChg*)pOldValue)->pChangedFmt != this &&
/*N*/ 			((SwFmtChg*)pNewValue)->pChangedFmt == DerivedFrom() )
/*N*/ 		{
/*N*/ 			// den Set an den neuen Parent haengen
/*N*/ 			aSet.SetParent( DerivedFrom() ? &DerivedFrom()->aSet : 0 );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	case RES_RESET_FMTWRITTEN:
/*N*/ 		{
/*N*/ 			// IsWritten-Flag zuruecksetzen. Hint nur an abhanegige
/*N*/ 			// Formate (und keine Frames) propagieren.
/*N*/ 			ResetWritten();
/*N*/ 			SwClientIter aIter( *this );
/*N*/ 			for( SwClient *pClient = aIter.First( TYPE(SwFmt) ); pClient;
/*N*/ 						pClient = aIter.Next() )
/*N*/ 				pClient->Modify( pOldValue, pNewValue );
/*N*/ 
/*N*/ 			bWeiter = FALSE;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	default:
/*N*/ 		{
/*N*/ 			// Ist das Attribut in diesem Format definiert, dann auf
/*N*/ 			// NICHT weiter propagieren !!
/*N*/ 			if( SFX_ITEM_SET == aSet.GetItemState( nWhich, FALSE ))
/*N*/ 			{
// wie finde ich heraus, ob nicht ich die Message versende ??
// aber wer ruft das hier ????
//ASSERT( FALSE, "Modify ohne Absender verschickt" );
//JP 11.06.96: DropCaps koennen hierher kommen
/*N*/ ASSERT( RES_PARATR_DROP == nWhich, "Modify ohne Absender verschickt" );
/*N*/ 				bWeiter = FALSE;
/*N*/ 			}
/*N*/ 
/*N*/ 		} // default
/*N*/ 	} // switch
/*N*/ 
/*N*/ 	if( bWeiter )
/*N*/ 	{
/*N*/ 		// laufe durch alle abhaengigen Formate
/*N*/ 		SwModify::Modify( pOldValue, pNewValue );
/*N*/ 	}
/*N*/ 
/*N*/ }


/*N*/ BOOL SwFmt::SetDerivedFrom(SwFmt *pDerFrom)
/*N*/ {
/*N*/ 	if ( pDerFrom )
/*N*/ 	{
/*N*/ 		// Zyklus?
/*N*/ 		const SwFmt* pFmt = pDerFrom;
/*N*/ 		while ( pFmt != 0 )
/*N*/ 		{
/*N*/ 			if ( pFmt == this )
/*?*/ 				return FALSE;
/*N*/ 
/*N*/ 			pFmt=pFmt->DerivedFrom();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		// Nichts angegeben, Dflt-Format suchen
/*?*/ 		pDerFrom = this;
/*?*/ 		while ( pDerFrom->DerivedFrom() )
/*?*/ 			pDerFrom = pDerFrom->DerivedFrom();
/*N*/ 	}
/*N*/ 	if ( (pDerFrom == DerivedFrom()) || (pDerFrom == this) )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ASSERT( Which()==pDerFrom->Which()
/*N*/ 			|| ( Which()==RES_CONDTXTFMTCOLL && pDerFrom->Which()==RES_TXTFMTCOLL)
/*N*/ 			|| ( Which()==RES_TXTFMTCOLL && pDerFrom->Which()==RES_CONDTXTFMTCOLL)
/*N*/ 			|| ( Which()==RES_FLYFRMFMT && pDerFrom->Which()==RES_FRMFMT ),
/*N*/ 			"SetDerivedFrom: Aepfel von Birnen ableiten?");
/*N*/ 
/*N*/ 	if ( IsInCache() )
/*N*/ 	{
/*N*/ 		SwFrm::GetCache().Delete( this );
/*N*/ 		SetInCache( FALSE );
/*N*/ 	}
/*N*/ 	SetInSwFntCache( FALSE );
/*N*/ 
/*N*/ 	pDerFrom->Add(this);
/*N*/ 	aSet.SetParent( &pDerFrom->aSet );
/*N*/ 
/*N*/ 	SwFmtChg aOldFmt(this);
/*N*/ 	SwFmtChg aNewFmt(this);
/*N*/ 	Modify( &aOldFmt, &aNewFmt );
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ BOOL SwFmt::SetAttr(const SfxPoolItem& rAttr )
/*N*/ {
/*N*/ 	if ( IsInCache() || IsInSwFntCache() )
/*N*/ 	{
/*N*/ 		const USHORT nWhich = rAttr.Which();
/*N*/ 		CheckCaching( nWhich );
/*N*/ 	}
/*N*/ 
/*N*/ 	// wenn Modify gelockt ist, werden keine Modifies verschickt;
/*N*/ 	// fuer FrmFmt's immer das Modify verschicken!
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	USHORT nFmtWhich;
/*N*/ 	if( IsModifyLocked() || (!GetDepends() &&
/*N*/ 		(RES_GRFFMTCOLL == (nFmtWhich = Which()) ||
/*N*/ 		 RES_TXTFMTCOLL == nFmtWhich ) ) )
/*N*/ 	{
/*N*/ 		if( 0 != ( bRet = (0 != aSet.Put( rAttr ))) )
/*N*/ 			aSet.SetModifyAtAttr( this );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// kopiere nur das Attribut-Delta Array
/*N*/ 		SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
/*N*/ 					aNew( *aSet.GetPool(), aSet.GetRanges() );
/*N*/ 
/*N*/ 		if( 0 != (bRet = aSet.Put_BC( rAttr, &aOld, &aNew )))
/*N*/ 		{
/*N*/ 			// einige Sonderbehandlungen fuer Attribute
/*N*/ 			aSet.SetModifyAtAttr( this );
/*N*/ 
/*N*/ 			SwAttrSetChg aChgOld( aSet, aOld );
/*N*/ 			SwAttrSetChg aChgNew( aSet, aNew );
/*N*/ 			Modify( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL SwFmt::SetAttr( const SfxItemSet& rSet )
/*N*/ {
/*N*/ 	if( !rSet.Count() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	if ( IsInCache() )
/*N*/ 	{
/*N*/ 		SwFrm::GetCache().Delete( this );
/*N*/ 		SetInCache( FALSE );
/*N*/ 	}
/*N*/ 	SetInSwFntCache( FALSE );
/*N*/ 
/*N*/ 	// wenn Modify gelockt ist, werden keine Modifies verschickt;
/*N*/ 	// fuer FrmFmt's immer das Modify verschicken!
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	USHORT nFmtWhich;
/*N*/ 	if( IsModifyLocked() || (!GetDepends() &&
/*N*/ 		(RES_GRFFMTCOLL == (nFmtWhich = Which()) ||
/*N*/ 		 RES_TXTFMTCOLL == nFmtWhich ) ) )
/*N*/ 	{
/*N*/ 		if( 0 != ( bRet = (0 != aSet.Put( rSet ))) )
/*N*/ 			aSet.SetModifyAtAttr( this );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
/*N*/ 					aNew( *aSet.GetPool(), aSet.GetRanges() );
/*N*/ 		if( 0 != ( bRet = aSet.Put_BC( rSet, &aOld, &aNew ) ) )
/*N*/ 		{
/*N*/ 			// einige Sonderbehandlungen fuer Attribute
/*N*/ 			aSet.SetModifyAtAttr( this );
/*N*/ 			SwAttrSetChg aChgOld( aSet, aOld );
/*N*/ 			SwAttrSetChg aChgNew( aSet, aNew );
/*N*/ 			Modify( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

// Nimmt den Hint mit nWhich aus dem Delta-Array


/*N*/ BOOL SwFmt::ResetAttr( USHORT nWhich1, USHORT nWhich2 )
/*N*/ {
/*N*/ 	if( !aSet.Count() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	if( !nWhich2 || nWhich2 < nWhich1 )
/*N*/ 		nWhich2 = nWhich1;		// dann setze auf 1. Id, nur dieses Item
/*N*/ 
/*N*/ 	if ( IsInCache() || IsInSwFntCache() )
/*N*/ 	{
/*N*/ 		for( USHORT n = nWhich1; n < nWhich2; ++n )
/*?*/ 			CheckCaching( n );
/*N*/ 	}
/*N*/ 
/*N*/ 	// wenn Modify gelockt ist, werden keine Modifies verschickt
/*N*/ 	if( IsModifyLocked() )
/*N*/ 		return 0 != (( nWhich2 == nWhich1 )
/*N*/ 				? aSet.ClearItem( nWhich1 )
/*N*/ 				: aSet.ClearItem_BC( nWhich1, nWhich2 ));
/*N*/ 
/*N*/ 	SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
/*N*/ 				aNew( *aSet.GetPool(), aSet.GetRanges() );
/*N*/ 	BOOL bRet = 0 != aSet.ClearItem_BC( nWhich1, nWhich2, &aOld, &aNew );
/*N*/ 
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/ 		SwAttrSetChg aChgOld( aSet, aOld );
/*N*/ 		SwAttrSetChg aChgNew( aSet, aNew );
/*N*/ 		Modify( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }



/*N*/ USHORT SwFmt::ResetAllAttr()
/*N*/ {
/*N*/ 	if( !aSet.Count() )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	if ( IsInCache() )
/*N*/ 	{
/*N*/ 		SwFrm::GetCache().Delete( this );
/*N*/ 		SetInCache( FALSE );
/*N*/ 	}
/*N*/ 	SetInSwFntCache( FALSE );
/*N*/ 
/*N*/ 	// wenn Modify gelockt ist, werden keine Modifies verschickt
/*N*/ 	if( IsModifyLocked() )
/*?*/ 		return aSet.ClearItem( 0 );
/*N*/ 
/*N*/ 	SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
/*N*/ 				aNew( *aSet.GetPool(), aSet.GetRanges() );
/*N*/ 	BOOL bRet = 0 != aSet.ClearItem_BC( 0, &aOld, &aNew );
/*N*/ 
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/ 		SwAttrSetChg aChgOld( aSet, aOld );
/*N*/ 		SwAttrSetChg aChgNew( aSet, aNew );
/*N*/ 		Modify( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
/*N*/ 	}
/*N*/ 	return aNew.Count();
/*N*/ }


/*************************************************************************
|*    void SwFmt::GetInfo( const SfxPoolItem& ) const
|*
|*    Beschreibung
|*    Ersterstellung    JP 18.04.94
|*    Letzte Aenderung  JP 05.08.94
*************************************************************************/

/*N*/ 
/*N*/ BOOL SwFmt::GetInfo( SfxPoolItem& rInfo ) const
/*N*/ {
/*N*/ 	BOOL bRet = SwModify::GetInfo( rInfo );
/*N*/ 	return bRet;
/*N*/ }


/** SwFmt::IsBackgroundTransparent - for feature #99657#

    OD 22.08.2002
    Virtual method to determine, if background of format is transparent.
    Default implementation returns false. Thus, subclasses have to overload
    method, if the specific subclass can have a transparent background.

    @author OD

    @return false, default implementation
*/
/*M*/ sal_Bool SwFmt::IsBackgroundTransparent() const
/*M*/ {
/*M*/     return sal_False;
/*M*/ }

}
