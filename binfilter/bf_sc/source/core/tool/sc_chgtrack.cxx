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

#include <bf_svtools/zforlist.hxx>
#include <bf_svtools/isethint.hxx>
#include <bf_svtools/itempool.hxx>
#include <bf_sfx2/app.hxx>
#include <bf_svtools/useroptions.hxx>
#include <bf_sfx2/sfxsids.hrc>
#include <bf_svx/adritem.hxx>

#include "cell.hxx"
#include "dociter.hxx"
#include "rechead.hxx"
#include "scerrors.hxx"
#include "scmod.hxx"   		// SC_MOD
#include "inputopt.hxx" 	// GetExpandRefs
#include "patattr.hxx"
#include "hints.hxx"

#include "globstr.hrc"

#define SC_CHGTRACK_CXX
#include "chgtrack.hxx"

#include "refupdat.hxx"

namespace binfilter {

/*N*/ DECLARE_STACK( ScChangeActionStack, ScChangeAction* )

const USHORT nMemPoolChangeActionCellListEntry = (0x2000 - 64) / sizeof(ScChangeActionCellListEntry);
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionCellListEntry, nMemPoolChangeActionCellListEntry, nMemPoolChangeActionCellListEntry )//STRIP008 ;

const USHORT nMemPoolChangeActionLinkEntry = (0x8000 - 64) / sizeof(ScChangeActionLinkEntry);
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionLinkEntry, nMemPoolChangeActionLinkEntry, nMemPoolChangeActionLinkEntry )//STRIP008 ;

// loaded MSB > eigenes => inkompatibel
#define SC_CHGTRACK_FILEFORMAT_FIRST	0x0001
#define SC_CHGTRACK_FILEFORMAT	0x0001

// --- ScChangeAction ------------------------------------------------------

/*N*/  ScChangeAction::ScChangeAction( ScChangeActionType eTypeP, const ScRange& rRange )
/*N*/  		:
/*N*/  		aBigRange( rRange ),
/*N*/  		pNext( NULL ),
/*N*/  		pPrev( NULL ),
/*N*/  		pLinkAny( NULL ),
/*N*/  		pLinkDeletedIn( NULL ),
/*N*/  		pLinkDeleted( NULL ),
/*N*/  		pLinkDependent( NULL ),
/*N*/  		nAction( 0 ),
/*N*/  		nRejectAction( 0 ),
/*N*/  		eType( eTypeP ),
/*N*/  		eState( SC_CAS_VIRGIN )
/*N*/  {
/*N*/  	aDateTime.ConvertToUTC();
/*N*/  }

/*N*/  ScChangeAction::ScChangeAction( ScChangeActionType eTypeP, const ScBigRange& rRange,
/*N*/  						const ULONG nTempAction, const ULONG nTempRejectAction,
/*N*/  						const ScChangeActionState eTempState, const DateTime& aTempDateTime,
/*N*/  						const String& aTempUser,  const String& aTempComment)
/*N*/  		:
/*N*/  		aBigRange( rRange ),
/*N*/  		pNext( NULL ),
/*N*/  		pPrev( NULL ),
/*N*/  		pLinkAny( NULL ),
/*N*/  		pLinkDeletedIn( NULL ),
/*N*/  		pLinkDeleted( NULL ),
/*N*/  		pLinkDependent( NULL ),
/*N*/  		nAction( nTempAction ),
/*N*/  		nRejectAction( nTempRejectAction ),
/*N*/  		eType( eTypeP ),
/*N*/  		eState( eTempState ),
/*N*/  		aDateTime( aTempDateTime ),
/*N*/  		aUser( aTempUser ),
/*N*/  		aComment( aTempComment )
/*N*/  {
/*N*/  }

/*N*/  ScChangeAction::ScChangeAction( ScChangeActionType eTypeP, const ScBigRange& rRange,
/*N*/  						const ULONG nTempAction)
/*N*/  		:
/*N*/  		aBigRange( rRange ),
/*N*/  		pNext( NULL ),
/*N*/  		pPrev( NULL ),
/*N*/  		pLinkAny( NULL ),
/*N*/  		pLinkDeletedIn( NULL ),
/*N*/  		pLinkDeleted( NULL ),
/*N*/  		pLinkDependent( NULL ),
/*N*/  		nAction( nTempAction ),
/*N*/  		nRejectAction( 0 ),
/*N*/  		eType( eTypeP ),
/*N*/  		eState( SC_CAS_VIRGIN )
/*N*/  {
/*N*/  	aDateTime.ConvertToUTC();
/*N*/  }

/*N*/  ScChangeAction::ScChangeAction( SvStream& rStrm, ScMultipleReadHeader& rHdr,
/*N*/  			ScChangeTrack* pTrack )
/*N*/  		:
/*N*/  		pNext( NULL ),
/*N*/  		pPrev( NULL ),
/*N*/  		pLinkAny( NULL ),
/*N*/  		pLinkDeletedIn( NULL ),
/*N*/  		pLinkDeleted( NULL ),
/*N*/  		pLinkDependent( NULL )
/*N*/  {
/*N*/  	// ScChangeTrack speichert aUser als Index auf Collection und eType selber
/*N*/  	UINT32 n32;
/*N*/  	UINT16 n16;
/*N*/  	rStrm >> aBigRange;
/*N*/  	rStrm >> n32; aDateTime.SetDate( n32 );
/*N*/  	rStrm >> n32; aDateTime.SetTime( n32 );
/*N*/  	rStrm >> n32; nAction = n32;
/*N*/  	rStrm >> n32; nRejectAction = n32;
/*N*/  	rStrm >> n16; eState = (ScChangeActionState) n16;
/*N*/  	rStrm.ReadByteString( aComment, rStrm.GetStreamCharSet() );
/*N*/  	// LinkEntries in zweiter Runde
/*N*/  }


/*N*/  ScChangeAction::~ScChangeAction()
/*N*/  {
/*N*/  	RemoveAllLinks();
/*N*/  }


/*N*/ BOOL ScChangeAction::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
/*N*/ {
/*N*/ 	// ScChangeTrack speichert aUser als Index auf Collection und eType selber
/*N*/ 	rStrm << aBigRange;
/*N*/ 	rStrm << (UINT32) aDateTime.GetDate();
/*N*/ 	rStrm << (UINT32) aDateTime.GetTime();
/*N*/ 	rStrm << (UINT32) nAction;
/*N*/ 	rStrm << (UINT32) nRejectAction;
/*N*/ 	rStrm << (UINT16) eState;
/*N*/ 	rStrm.WriteByteString( aComment, rStrm.GetStreamCharSet() );
/*N*/ 	// LinkEntries in zweiter Runde
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ BOOL ScChangeAction::StoreLinks( SvStream& rStrm ) const
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::StoreLinkChain( pLinkDeleted, rStrm );
/*N*/ 	bOk &= ScChangeAction::StoreLinkChain( pLinkDependent, rStrm );
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScChangeAction::LoadLinks( SvStream& rStrm, ScChangeTrack* pTrack ) // Changetracking.sdc
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::LoadLinkChain( this, &pLinkDeleted, rStrm,
/*N*/ 		pTrack, TRUE );
/*N*/ 	bOk &= ScChangeAction::LoadLinkChain( this, &pLinkDependent, rStrm,
/*N*/ 		pTrack, FALSE );
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScChangeAction::IsTouchable() const
/*N*/ {
/*N*/ 	//! sequence order of execution is significant
/*N*/ 	if ( IsRejected() || GetType() == SC_CAT_REJECT || IsDeletedIn() )
/*N*/ 		return FALSE;
/*N*/ 	// content may reject and be touchable if on top
/*N*/ 	if ( GetType() == SC_CAT_CONTENT )
/*N*/ 		return ((ScChangeActionContent*)this)->IsTopContent();
/*N*/ 	if ( IsRejecting() )
/*N*/ 		return FALSE;
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL ScChangeAction::IsInternalRejectable() const
/*N*/ {
/*N*/ 	//! sequence order of execution is significant
/*N*/ 	if ( !IsVirgin() )
/*N*/ 		return FALSE;
/*N*/ 	if ( IsDeletedIn() )
/*N*/ 		return FALSE;
/*N*/ 	if ( GetType() == SC_CAT_CONTENT )
/*N*/ 	{
/*N*/ 		ScChangeActionContent* pNextContent =
/*N*/ 			((ScChangeActionContent*)this)->GetNextContent();
/*N*/ 		if ( pNextContent == NULL )
/*N*/ 			return TRUE;		// *this is TopContent
/*N*/ 		return pNextContent->IsRejected();		// *this is next rejectable
/*N*/ 	}
/*N*/ 	return IsTouchable();
/*N*/ }


/*N*/ void ScChangeAction::RemoveAllLinks()
/*N*/ {
/*N*/ 	RemoveAllAnyLinks();
/*N*/ 	RemoveAllDeletedIn();
/*N*/ 	RemoveAllDeleted();
/*N*/ 	RemoveAllDependent();
/*N*/ }


/*N*/ void ScChangeAction::RemoveAllAnyLinks()
/*N*/ {
/*N*/ 	while ( pLinkAny )
/*N*/ 		delete pLinkAny;		// rueckt sich selbst hoch
/*N*/ }


/*N*/ BOOL ScChangeAction::RemoveDeletedIn( const ScChangeAction* p )
/*N*/ {
/*N*/ 	BOOL bRemoved = FALSE;
/*N*/ 	ScChangeActionLinkEntry* pL = GetDeletedIn();
/*N*/ 	while ( pL )
/*N*/ 	{
/*N*/ 		ScChangeActionLinkEntry* pNextLink = pL->GetNext();
/*N*/ 		if ( pL->GetAction() == p )
/*N*/ 		{
/*N*/ 			delete pL;
/*N*/ 			bRemoved = TRUE;
/*N*/ 		}
/*N*/ 		pL = pNextLink;
/*N*/ 	}
/*N*/ 	return bRemoved;
/*N*/ }


/*N*/ BOOL ScChangeAction::IsDeletedIn( const ScChangeAction* p ) const
/*N*/ {
/*N*/ 	ScChangeActionLinkEntry* pL = GetDeletedIn();
/*N*/ 	while ( pL )
/*N*/ 	{
/*N*/ 		if ( pL->GetAction() == p )
/*N*/ 			return TRUE;
/*N*/ 		pL = pL->GetNext();
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ void ScChangeAction::RemoveAllDeletedIn()
/*N*/ {
/*N*/ 	//! nicht vom evtl. TopContent sondern wirklich dieser
/*N*/ 	while ( pLinkDeletedIn )
/*N*/ 		delete pLinkDeletedIn;		// rueckt sich selbst hoch
/*N*/ }


/*N*/ BOOL ScChangeAction::IsDeletedInDelType( ScChangeActionType eDelType ) const
/*N*/ {
/*N*/ 	ScChangeAction* p;
/*N*/ 	ScChangeActionLinkEntry* pL = GetDeletedIn();
/*N*/ 	if ( pL )
/*N*/ 	{
/*N*/ 		// InsertType fuer MergePrepare/MergeOwn
/*N*/ 		ScChangeActionType eInsType;
/*N*/ 		switch ( eDelType )
/*N*/ 		{
/*N*/ 			case SC_CAT_DELETE_COLS :
/*N*/ 				eInsType = SC_CAT_INSERT_COLS;
/*N*/ 			break;
/*N*/ 			case SC_CAT_DELETE_ROWS :
/*N*/ 				eInsType = SC_CAT_INSERT_ROWS;
/*N*/ 			break;
/*N*/ 			case SC_CAT_DELETE_TABS :
/*N*/ 				eInsType = SC_CAT_INSERT_TABS;
/*N*/ 			break;
/*N*/ 			default:
/*N*/ 				eInsType = SC_CAT_NONE;
/*N*/ 		}
/*N*/ 		while ( pL )
/*N*/ 		{
/*N*/ 			if ( (p = pL->GetAction()) &&
/*N*/ 					(p->GetType() == eDelType || p->GetType() == eInsType) )
/*N*/ 				return TRUE;
/*N*/ 			pL = pL->GetNext();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ void ScChangeAction::SetDeletedIn( ScChangeAction* p ) // Changetracking.sdc
/*N*/ {
/*N*/ 	ScChangeActionLinkEntry* pLink1 = AddDeletedIn( p );
/*N*/ 	ScChangeActionLinkEntry* pLink2;
/*N*/ 	if ( GetType() == SC_CAT_CONTENT )
/*N*/ 		pLink2 = p->AddDeleted( ((ScChangeActionContent*)this)->GetTopContent() );
/*N*/ 	else
/*N*/ 		pLink2 = p->AddDeleted( this );
/*N*/ 	pLink1->SetLink( pLink2 );
/*N*/ }


/*N*/ void ScChangeAction::RemoveAllDeleted()
/*N*/ {
/*N*/ 	while ( pLinkDeleted )
/*N*/ 		delete pLinkDeleted;		// rueckt sich selbst hoch
/*N*/ }


/*N*/ void ScChangeAction::RemoveAllDependent()
/*N*/ {
/*N*/ 	while ( pLinkDependent )
/*N*/ 		delete pLinkDependent;		// rueckt sich selbst hoch
/*N*/ }


/*N*/ void ScChangeAction::UpdateReference( const ScChangeTrack* pTrack,
/*N*/ 		UpdateRefMode eMode, const ScBigRange& rRange,
/*N*/ 		INT32 nDx, INT32 nDy, INT32 nDz )
/*N*/ {
/*N*/ 	ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, GetBigRange() );
/*N*/ }


/*N*/ String ScChangeAction::GetRefString( const ScBigRange& rRange,
/*N*/ 		ScDocument* pDoc, BOOL bFlag3D ) const
/*N*/ {
/*N*/ 	String aStr;
/*N*/ 	USHORT nFlags = ( rRange.IsValid( pDoc ) ? SCA_VALID : 0 );
/*N*/ 	if ( !nFlags )
/*N*/ 		aStr = ScGlobal::GetRscString( STR_NOREF_STR );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ScRange aTmpRange( rRange.MakeRange() );
/*N*/ 		switch ( GetType() )
/*N*/ 		{
/*N*/ 			case SC_CAT_INSERT_COLS :
/*N*/ 			case SC_CAT_DELETE_COLS :
/*N*/ 				if ( bFlag3D )
/*N*/ 				{
/*N*/ 					pDoc->GetName( aTmpRange.aStart.Tab(), aStr );
/*N*/ 					aStr += '.';
/*N*/ 				}
/*N*/ 				aStr += binfilter::ColToAlpha( aTmpRange.aStart.Col() );
/*N*/ 				aStr += ':';
/*N*/ 				aStr += binfilter::ColToAlpha( aTmpRange.aEnd.Col() );
/*N*/ 			break;
/*N*/ 			case SC_CAT_INSERT_ROWS :
/*N*/ 			case SC_CAT_DELETE_ROWS :
/*N*/ 				if ( bFlag3D )
/*N*/ 				{
/*N*/ 					pDoc->GetName( aTmpRange.aStart.Tab(), aStr );
/*N*/ 					aStr += '.';
/*N*/ 				}
/*N*/ 				aStr += String::CreateFromInt32( aTmpRange.aStart.Row() + 1 );
/*N*/ 				aStr += ':';
/*N*/ 				aStr += String::CreateFromInt32( aTmpRange.aEnd.Row() + 1 );
/*N*/ 			break;
/*N*/ 			default:
/*N*/ 				if ( bFlag3D || GetType() == SC_CAT_INSERT_TABS )
/*N*/ 					nFlags |= SCA_TAB_3D;
/*N*/ 				aTmpRange.Format( aStr, nFlags, pDoc );
/*N*/ 		}
/*N*/ 		if ( (bFlag3D && IsDeleteType()) || IsDeletedIn() )
/*N*/ 		{
/*N*/ 			aStr.Insert( '(', 0 );
/*N*/ 			aStr += ')';
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return aStr;
/*N*/ }


/*N*/ void ScChangeAction::GetRefString( String& rStr, ScDocument* pDoc,
/*N*/ 		BOOL bFlag3D ) const
/*N*/ {
/*N*/ 	rStr = GetRefString( GetBigRange(), pDoc, bFlag3D );
/*N*/ }


/*N*/ void ScChangeAction::Accept()
/*N*/ {
/*N*/ 	if ( IsVirgin() )
/*N*/ 	{
/*N*/ 		SetState( SC_CAS_ACCEPTED );
/*N*/ 		DeleteCellEntries();
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeAction::SetRejected()
/*N*/ {
/*N*/ 	if ( IsVirgin() )
/*N*/ 	{
/*N*/ 		SetState( SC_CAS_REJECTED );
/*N*/ 		RemoveAllLinks();
/*N*/ 		DeleteCellEntries();
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeAction::RejectRestoreContents( ScChangeTrack* pTrack,
/*N*/ 		short nDx, short nDy )
/*N*/ {
/*N*/ 	// Liste der Contents aufbauen
/*N*/ 	ScChangeActionCellListEntry* pListContents = NULL;
/*N*/ 	for ( ScChangeActionLinkEntry* pL = pLinkDeleted; pL; pL = pL->GetNext() )
/*N*/ 	{
/*N*/ 		ScChangeAction* p = pL->GetAction();
/*N*/ 		if ( p && p->GetType() == SC_CAT_CONTENT )
/*N*/ 		{
/*N*/ 			ScChangeActionCellListEntry* pE = new ScChangeActionCellListEntry(
/*N*/ 				(ScChangeActionContent*) p, pListContents );
/*N*/ 			pListContents = pE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SetState( SC_CAS_REJECTED );		// vor UpdateReference fuer Move
/*N*/ 	pTrack->UpdateReference( this, TRUE );		// LinkDeleted freigeben
/*N*/ 	DBG_ASSERT( !pLinkDeleted, "ScChangeAction::RejectRestoreContents: pLinkDeleted != NULL" );
/*N*/ 	// Liste der Contents abarbeiten und loeschen
/*N*/ 	ScDocument* pDoc = pTrack->GetDocument();
/*N*/ 	ScChangeActionCellListEntry* pE = pListContents;
/*N*/ 	while ( pE )
/*N*/ 	{
/*N*/ 		if ( !pE->pContent->IsDeletedIn() &&
/*N*/ 				pE->pContent->GetBigRange().aStart.IsValid( pDoc ) )
/*N*/ 			pE->pContent->PutNewValueToDoc( pDoc, nDx, nDy );
/*N*/ 		ScChangeActionCellListEntry* pNext;
/*N*/ 		pNext = pE->pNext;
/*N*/ 		delete pE;
/*N*/ 		pE = pNext;
/*N*/ 	}
/*N*/ 	DeleteCellEntries();		// weg mit den generierten
/*N*/ }


// static
/*N*/ void ScChangeAction::StoreCell( ScBaseCell* pCell, SvStream& rStrm,
/*N*/ 		ScMultipleWriteHeader& rHdr )
/*N*/ {
/*N*/ 	if ( pCell )
/*N*/ 	{
/*N*/ 		CellType eCellType = pCell->GetCellType();
/*N*/ 		switch( eCellType )
/*N*/ 		{
/*N*/ 			case CELLTYPE_VALUE:
/*N*/ 				rStrm << (BYTE) eCellType;
/*N*/ 				((ScValueCell*)pCell)->Save( rStrm );
/*N*/ 			break;
/*N*/ 			case CELLTYPE_STRING:
/*N*/ 				rStrm << (BYTE) eCellType;
/*N*/ 				((ScStringCell*)pCell)->Save( rStrm );
/*N*/ 			break;
/*N*/ 			case CELLTYPE_EDIT:
/*N*/ 				rStrm << (BYTE) eCellType;
/*N*/ 				((ScEditCell*)pCell)->Save( rStrm );
/*N*/ 			break;
/*N*/ 			case CELLTYPE_FORMULA:
/*N*/ 				rStrm << (BYTE) eCellType;
/*N*/ 				rStrm << ((ScFormulaCell*)pCell)->aPos;
/*N*/ 				((ScFormulaCell*)pCell)->Save( rStrm, rHdr );
/*N*/ 			break;
/*N*/ 			default:
/*N*/ 				DBG_ERROR( "ScChangeAction::StoreCell: unknown CellType" );
/*N*/ 				rStrm << (BYTE) CELLTYPE_NONE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rStrm << (BYTE) CELLTYPE_NONE;
/*N*/ }


// static
/*N*/ ScBaseCell* ScChangeAction::LoadCell( SvStream& rStrm,
/*N*/ 		ScMultipleReadHeader& rHdr, ScDocument* pDoc, USHORT nVer )
/*N*/ {
/*N*/ 	ScBaseCell* pCell;
/*N*/ 	BYTE nByte;
/*N*/ 	rStrm >> nByte;
/*N*/ 	switch ( (CellType) nByte )
/*N*/ 	{
/*N*/ 		case CELLTYPE_VALUE:
/*N*/ 		{
/*N*/ 			pCell = new ScValueCell( rStrm, nVer );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case CELLTYPE_STRING:
/*N*/ 		{
/*N*/ 			pCell = new ScStringCell( rStrm, nVer );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case CELLTYPE_EDIT:
/*N*/ 		{
/*N*/ 			pCell = new ScEditCell( rStrm, nVer, pDoc );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case CELLTYPE_FORMULA:
/*N*/ 		{
/*N*/ 			ScAddress aPos;
/*N*/ 			rStrm >> aPos;
/*N*/ 			pCell = new ScFormulaCell( pDoc, aPos, rStrm, rHdr );
/*N*/ 			((ScFormulaCell*)pCell)->SetInChangeTrack( TRUE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case CELLTYPE_NONE :
/*N*/ 			pCell = NULL;
/*N*/ 		break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR( "ScChangeAction::LoadCell: unknown CellType" );
/*N*/ 			rStrm.SetError( SVSTREAM_FILEFORMAT_ERROR );
/*N*/ 			pCell = NULL;
/*N*/ 	}
/*N*/ 	return pCell;
/*N*/ }


// static
/*N*/ BOOL ScChangeAction::StoreLinkChain( ScChangeActionLinkEntry* pLinkFirst,
/*N*/ 		SvStream& rStrm )
/*N*/ {
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	UINT32 nCount = 0;
/*N*/ 	if ( pLinkFirst )
/*N*/ 	{
/*N*/ 		// rueckwaerts speichern, damit onLoad mit Insert die Reihenfolge wieder stimmt
/*N*/ 		Stack* pStack = new Stack;
/*N*/ 		for ( ScChangeActionLinkEntry* pL = pLinkFirst; pL; pL = pL->GetNext() )
/*N*/ 		{
/*N*/ 			++nCount;
/*N*/ 			pStack->Push( pL );
/*N*/ 		}
/*N*/ 		rStrm << nCount;
/*N*/ 		ScChangeActionLinkEntry* pHere;
/*N*/ 		while ( pHere = (ScChangeActionLinkEntry*) pStack->Pop() )
/*N*/ 		{
/*N*/ 			ScChangeAction* p = pHere->GetAction();
/*N*/ 			rStrm << (UINT32) ( p ? p->GetActionNumber() : 0 );
/*N*/ 		}
/*N*/ 		delete pStack;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rStrm << nCount;
/*N*/ 	return bOk;
/*N*/ }


// static
/*N*/ BOOL ScChangeAction::LoadLinkChain( ScChangeAction* pOfAction, // Changetracking.sdc
/*N*/ 		ScChangeActionLinkEntry** ppLinkFirst, SvStream& rStrm,
/*N*/ 		ScChangeTrack* pTrack, BOOL bLinkDeleted )
/*N*/ {
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	UINT32 nCount;
/*N*/ 	rStrm >> nCount;
/*N*/ 	for ( UINT32 j = 0; j < nCount; j++ )
/*N*/ 	{
/*N*/ 		ScChangeAction* pAct = NULL;
/*N*/ 		UINT32 nAct;
/*N*/ 		rStrm >> nAct;
/*N*/ 		if ( nAct )
/*N*/ 		{
/*N*/ 			pAct = pTrack->GetActionOrGenerated( nAct );
/*N*/ 			DBG_ASSERT( pAct, "ScChangeAction::LoadLinkChain: missing Action" );
/*N*/ 		}
/*N*/ 		if ( bLinkDeleted )
/*N*/ 		{
/*N*/ 			if ( pAct )
/*N*/ 				pAct->SetDeletedIn( pOfAction );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
/*N*/ 				ppLinkFirst, pAct );
/*N*/ 			if ( pAct )
/*N*/ 				pAct->AddLink( pOfAction, pLink );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bOk;
/*N*/ }


/*N*/  void ScChangeAction::SetDeletedInThis( ULONG nActionNumber,
/*N*/  		const ScChangeTrack* pTrack )
/*N*/  {
/*N*/ 	if ( nActionNumber )
/*N*/ 	{
/*N*/ 		ScChangeAction* pAct = pTrack->GetActionOrGenerated( nActionNumber );
/*N*/ 		DBG_ASSERT( pAct, "ScChangeAction::SetDeletedInThis: missing Action" );
/*N*/ 		if ( pAct )
/*N*/ 			pAct->SetDeletedIn( this );
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeAction::AddDependent( ULONG nActionNumber,
/*N*/ 		const ScChangeTrack* pTrack )
/*N*/ {
/*N*/  	if ( nActionNumber )
/*N*/ 	{
/*N*/ 		ScChangeAction* pAct = pTrack->GetActionOrGenerated( nActionNumber );
/*N*/ 		DBG_ASSERT( pAct, "ScChangeAction::AddDependent: missing Action" );
/*N*/ 		if ( pAct )
/*N*/ 		{
/*N*/ 			ScChangeActionLinkEntry* pLink = AddDependent( pAct );
/*N*/ 			pAct->AddLink( this, pLink );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// static
/*N*/ BOOL ScChangeAction::StoreCellList( ScChangeActionCellListEntry* pFirstCell,
/*N*/ 		SvStream& rStrm )
/*N*/ {
/*N*/ 	UINT32 nCount = 0;
/*N*/ 	for ( const ScChangeActionCellListEntry* pE = pFirstCell; pE;
/*N*/ 			pE = pE->pNext )
/*N*/ 		++nCount;
/*N*/ 	rStrm << nCount;
/*N*/ 
/*N*/ 	if ( nCount )
/*N*/ 	{
/*N*/ 		for ( const ScChangeActionCellListEntry* pE = pFirstCell; pE;
/*N*/ 				pE = pE->pNext )
/*N*/ 		{	// Store/Load vertauscht die Reihenfolge, aber das ist hierbei egal
/*N*/ 			rStrm << (UINT32) pE->pContent->GetActionNumber();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ BOOL ScChangeAction::LoadCellList( ScChangeAction* pOfAction,
/*N*/ 		ScChangeActionCellListEntry*& pFirstCell, SvStream& rStrm,
/*N*/ 		ScChangeTrack* pTrack )
/*N*/ {
/*N*/ 	UINT32 nCount;
/*N*/ 	rStrm >> nCount;
/*N*/ 	if ( nCount )
/*N*/ 	{
/*N*/ 		for ( UINT32 j = 0; j < nCount; j++ )
/*N*/ 		{
/*N*/ 			ScChangeActionContent* pContent;
/*N*/ 			UINT32 nContent;
/*N*/ 			rStrm >> nContent;
/*N*/ 			pContent = (ScChangeActionContent*) pTrack->GetActionOrGenerated( nContent );
/*N*/ 			if ( pContent )
/*N*/ 				pOfAction->AddContent( pContent );
/*N*/ 			else
/*N*/ 				DBG_ERROR( "ScChangeActionDel::LoadLinks: missing Content" );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }


// --- ScChangeActionIns ---------------------------------------------------

/*N*/ ScChangeActionIns::ScChangeActionIns( const ScRange& rRange )
/*N*/ 		: ScChangeAction( SC_CAT_NONE, rRange )
/*N*/ {
/*N*/ if ( rRange.aStart.Col() == 0 && rRange.aEnd.Col() == MAXCOL )
/*N*/ 	{
/*N*/ 		aBigRange.aStart.SetCol( nInt32Min );
/*N*/ 		aBigRange.aEnd.SetCol( nInt32Max );
/*N*/ 		if ( rRange.aStart.Row() == 0 && rRange.aEnd.Row() == MAXROW )
/*N*/ 		{
/*N*/ 			SetType( SC_CAT_INSERT_TABS );
/*N*/ 			aBigRange.aStart.SetRow( nInt32Min );
/*N*/ 			aBigRange.aEnd.SetRow( nInt32Max );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			SetType( SC_CAT_INSERT_ROWS );
/*N*/ 	}
/*N*/ 	else if ( rRange.aStart.Row() == 0 && rRange.aEnd.Row() == MAXROW )
/*N*/ 	{
/*N*/ 		SetType( SC_CAT_INSERT_COLS );
/*N*/ 		aBigRange.aStart.SetRow( nInt32Min );
/*N*/ 		aBigRange.aEnd.SetRow( nInt32Max );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR( "ScChangeActionIns: Block not supported!" );
/*N*/ }


/*N*/ ScChangeActionIns::ScChangeActionIns( SvStream& rStrm,
/*N*/ 			ScMultipleReadHeader& rHdr, ScChangeTrack* pTrack )
/*N*/ 		:
/*N*/ 		ScChangeAction( rStrm, rHdr, pTrack )
/*N*/ {
/*N*/ }

/*N*/ ScChangeActionIns::ScChangeActionIns(const ULONG nActionNumber, const ScChangeActionState eState, const ULONG nRejectingNumber,
/*N*/ 												const ScBigRange& aBigRange, const String& aUser, const DateTime& aDateTime, const String& sComment,
/*N*/ 												const ScChangeActionType eType)
/*N*/ 		:
/*N*/ 		ScChangeAction(eType, aBigRange, nActionNumber, nRejectingNumber, eState, aDateTime, aUser, sComment)
/*N*/ {
/*N*/ }

/*N*/  ScChangeActionIns::~ScChangeActionIns()
/*N*/  {
/*N*/  }


/*N*/ BOOL ScChangeActionIns::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::Store( rStrm, rHdr );
/*N*/ 	return bOk;
/*N*/ }


/*N*/ void ScChangeActionIns::GetDescription( String& rStr, ScDocument* pDoc,
/*N*/ 		BOOL bSplitRange ) const
/*N*/ {
/*N*/ 	USHORT nWhatId;
/*N*/ 	switch ( GetType() )
/*N*/ 	{
/*N*/ 		case SC_CAT_INSERT_COLS :
/*N*/ 			nWhatId = STR_COLUMN;
/*N*/ 		break;
/*N*/ 		case SC_CAT_INSERT_ROWS :
/*N*/ 			nWhatId = STR_ROW;
/*N*/ 		break;
/*N*/ 		default:
/*N*/ 			nWhatId = STR_AREA;
/*N*/ 	}
/*N*/ 
/*N*/ 	String aRsc( ScGlobal::GetRscString( STR_CHANGED_INSERT ) );
/*N*/ 	xub_StrLen nPos = aRsc.SearchAscii( "#1" );
/*N*/ 	rStr += aRsc.Copy( 0, nPos );
/*N*/ 	rStr += ScGlobal::GetRscString( nWhatId );
/*N*/ 	rStr += ' ';
/*N*/ 	rStr += GetRefString( GetBigRange(), pDoc );
/*N*/ 	rStr += aRsc.Copy( nPos+2 );
/*N*/ }


/*N*/ BOOL ScChangeActionIns::Reject( ScDocument* pDoc )
/*N*/ {
/*N*/ 	if ( !aBigRange.IsValid( pDoc ) )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ScRange aRange( aBigRange.MakeRange() );
/*N*/ 	if ( !pDoc->IsBlockEditable( aRange.aStart.Tab(), aRange.aStart.Col(),
/*N*/ 			aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row() ) )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	switch ( GetType() )
/*N*/ 	{
/*N*/ 		case SC_CAT_INSERT_COLS :
/*N*/ 			pDoc->DeleteCol( aRange );
/*N*/ 		break;
/*N*/ 		case SC_CAT_INSERT_ROWS :
/*N*/ 			pDoc->DeleteRow( aRange );
/*N*/ 		break;
/*N*/ 		case SC_CAT_INSERT_TABS :
/*N*/ 			pDoc->DeleteTab( aRange.aStart.Tab() );
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	SetState( SC_CAS_REJECTED );
/*N*/ 	RemoveAllLinks();
/*N*/ 	return TRUE;
/*N*/ }


// --- ScChangeActionDel ---------------------------------------------------

/*N*/ ScChangeActionDel::ScChangeActionDel( const ScRange& rRange,
/*N*/ 			short nDxP, short nDyP, ScChangeTrack* pTrackP )
/*N*/ 		:
/*N*/ 		ScChangeAction( SC_CAT_NONE, rRange ),
/*N*/ 		pTrack( pTrackP ),
/*N*/ 		pFirstCell( NULL ),
/*N*/ 		pLinkMove( NULL ),
/*N*/ 		pCutOff( NULL ),
/*N*/ 		nCutOff( 0 ),
/*N*/ 		nDx( nDxP ),
/*N*/ 		nDy( nDyP )
/*N*/ {
/*N*/ if ( rRange.aStart.Col() == 0 && rRange.aEnd.Col() == MAXCOL )
/*N*/ 	{
/*N*/ 		aBigRange.aStart.SetCol( nInt32Min );
/*N*/ 		aBigRange.aEnd.SetCol( nInt32Max );
/*N*/ 		if ( rRange.aStart.Row() == 0 && rRange.aEnd.Row() == MAXROW )
/*N*/ 		{
/*N*/ 			SetType( SC_CAT_DELETE_TABS );
/*N*/ 			aBigRange.aStart.SetRow( nInt32Min );
/*N*/ 			aBigRange.aEnd.SetRow( nInt32Max );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			SetType( SC_CAT_DELETE_ROWS );
/*N*/ 	}
/*N*/ 	else if ( rRange.aStart.Row() == 0 && rRange.aEnd.Row() == MAXROW )
/*N*/ 	{
/*N*/ 		SetType( SC_CAT_DELETE_COLS );
/*N*/ 		aBigRange.aStart.SetRow( nInt32Min );
/*N*/ 		aBigRange.aEnd.SetRow( nInt32Max );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR( "ScChangeActionDel: Block not supported!" );
/*N*/ }


/*N*/ ScChangeActionDel::ScChangeActionDel( SvStream& rStrm,
/*N*/ 			ScMultipleReadHeader& rHdr, ScDocument* pDoc, USHORT nVer,
/*N*/ 			ScChangeTrack* pTrackP )
/*N*/ 		:
/*N*/ 		ScChangeAction( rStrm, rHdr, pTrackP ),
/*N*/ 		pTrack( pTrackP ),
/*N*/ 		pFirstCell( NULL ),
/*N*/ 		pLinkMove( NULL )
/*N*/ {
/*N*/ UINT32 n32;
/*N*/ 	INT16 n16s;
/*N*/ 	rStrm >> n32; pCutOff = (ScChangeActionIns*)(ULONG) n32;
/*N*/ 	rStrm >> n16s; nCutOff = n16s;
/*N*/ 	rStrm >> n16s; nDx = n16s;
/*N*/ 	rStrm >> n16s; nDy = n16s;
/*N*/ }

/*N*/ ScChangeActionDel::ScChangeActionDel(const ULONG nActionNumber, const ScChangeActionState eState, const ULONG nRejectingNumber,
/*N*/ 									const ScBigRange& aBigRange, const String& aUser, const DateTime& aDateTime, const String &sComment,
/*N*/ 									const ScChangeActionType eType, const short nD, ScChangeTrack* pTrackP) // wich of nDx and nDy is set is depend on the type
/*N*/ 		:
/*N*/ 		ScChangeAction(eType, aBigRange, nActionNumber, nRejectingNumber, eState, aDateTime, aUser, sComment),
/*N*/ 		pTrack( pTrackP ),
/*N*/ 		pFirstCell( NULL ),
/*N*/ 		pLinkMove( NULL ),
/*N*/ 		pCutOff( NULL ),
/*N*/ 		nCutOff( 0 ),
/*N*/ 		nDx( 0 ),
/*N*/ 		nDy( 0 )
/*N*/ {
/*N*/ if (eType == SC_CAT_DELETE_COLS)
/*N*/ 		nDx = nD;
/*N*/ 	else if (eType == SC_CAT_DELETE_ROWS)
/*N*/ 		nDy = nD;
/*N*/ }

/*N*/ ScChangeActionDel::~ScChangeActionDel()
/*N*/ {
/*N*/ 	DeleteCellEntries();
/*N*/ 	while ( pLinkMove )
/*N*/ 		delete pLinkMove;
/*N*/ }


/*N*/ BOOL ScChangeActionDel::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::Store( rStrm, rHdr );
/*N*/ 	rStrm << (UINT32) ( pCutOff ? pCutOff->GetActionNumber() : 0 );
/*N*/ 	rStrm << (INT16) nCutOff;
/*N*/ 	rStrm << (INT16) nDx;
/*N*/ 	rStrm << (INT16) nDy;
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScChangeActionDel::StoreLinks( SvStream& rStrm ) const
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::StoreLinks( rStrm );
/*N*/ 	UINT32 nCount = 0;
/*N*/ 	if ( pLinkMove )
/*N*/ 	{
/*N*/ 		// rueckwaerts speichern, damit onLoad mit Insert die Reihenfolge wieder stimmt
/*N*/ 		Stack* pStack = new Stack;
/*N*/ 		for ( ScChangeActionDelMoveEntry* pL = pLinkMove; pL; pL = pL->GetNext() )
/*N*/ 		{
/*N*/ 			++nCount;
/*N*/ 			pStack->Push( pL );
/*N*/ 		}
/*N*/ 		rStrm << nCount;
/*N*/ 		ScChangeActionDelMoveEntry* pHere;
/*N*/ 		while ( pHere = (ScChangeActionDelMoveEntry*) pStack->Pop() )
/*N*/ 		{
/*N*/ 			ScChangeAction* p = pHere->GetAction();
/*N*/ 			rStrm << (UINT32) ( p ? p->GetActionNumber() : 0 );
/*N*/ 			rStrm << (INT16) pHere->GetCutOffFrom();
/*N*/ 			rStrm << (INT16) pHere->GetCutOffTo();
/*N*/ 		}
/*N*/ 		delete pStack;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rStrm << nCount;
/*N*/ 
/*N*/ 	bOk &= ScChangeAction::StoreCellList( pFirstCell, rStrm );
/*N*/ 
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScChangeActionDel::LoadLinks( SvStream& rStrm, ScChangeTrack* pTrack )
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::LoadLinks( rStrm, pTrack );
/*N*/ 	UINT32 nCount;
/*N*/ 	rStrm >> nCount;
/*N*/ 	for ( UINT32 j = 0; j < nCount; j++ )
/*N*/ 	{
/*N*/ 		ScChangeActionMove* pAct = NULL;
/*N*/ 		UINT32 nAct;
/*N*/ 		rStrm >> nAct;
/*N*/ 		if ( nAct )
/*N*/ 		{
/*N*/ 			pAct = (ScChangeActionMove*) pTrack->GetAction( nAct );
/*N*/ 			DBG_ASSERT( pAct, "ScChangeActionDel::LoadLinks: missing Move" );
/*N*/ 		}
/*N*/ 		INT16 nFrom, nTo;
/*N*/ 		rStrm >> nFrom >> nTo;
/*N*/ 		ScChangeActionDelMoveEntry* pLink = new ScChangeActionDelMoveEntry(
/*N*/ 			&pLinkMove, pAct, nFrom, nTo );
/*N*/ 		if ( pAct )
/*N*/ 			pAct->AddLink( this, pLink );
/*N*/ 	}
/*N*/ 	if ( pCutOff )
/*N*/ 	{
/*N*/ 		pCutOff = (ScChangeActionIns*) pTrack->GetAction( (ULONG) pCutOff );
/*N*/ 		DBG_ASSERT( pCutOff, "ScChangeActionDel::LoadLinks: missing Insert" );
/*N*/ 	}
/*N*/ 
/*N*/ 	bOk &= ScChangeAction::LoadCellList( this, pFirstCell, rStrm, pTrack );
/*N*/ 
/*N*/ 	return bOk;
/*N*/ }

/*N*/ void ScChangeActionDel::AddContent( ScChangeActionContent* pContent )
/*N*/ {
/*N*/ 	ScChangeActionCellListEntry* pE = new ScChangeActionCellListEntry(
/*N*/ 		pContent, pFirstCell );
/*N*/ 	pFirstCell = pE;
/*N*/ }


/*N*/ void ScChangeActionDel::DeleteCellEntries()
/*N*/ {
/*N*/ 	pTrack->DeleteCellEntries( pFirstCell, this );
/*N*/ }


/*N*/ BOOL ScChangeActionDel::IsBaseDelete() const
/*N*/ {
/*N*/ 	return !GetDx() && !GetDy();
/*N*/ }


/*N*/ BOOL ScChangeActionDel::IsTopDelete() const
/*N*/ {
/*N*/ 	const ScChangeAction* p = GetNext();
/*N*/ 	if ( !p || p->GetType() != GetType() )
/*N*/ 		return TRUE;
/*N*/ 	return ((ScChangeActionDel*)p)->IsBaseDelete();
/*N*/ }


/*N*/ BOOL ScChangeActionDel::IsMultiDelete() const
/*N*/ {
/*N*/ 	if ( GetDx() || GetDy() )
/*N*/ 		return TRUE;
/*N*/ 	const ScChangeAction* p = GetNext();
/*N*/ 	if ( !p || p->GetType() != GetType() )
/*N*/ 		return FALSE;
/*N*/ 	const ScChangeActionDel* pDel = (const ScChangeActionDel*) p;
/*N*/ 	if ( (pDel->GetDx() > GetDx() || pDel->GetDy() > GetDy()) &&
/*N*/ 			pDel->GetBigRange() == aBigRange )
/*N*/ 		return TRUE;
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ void ScChangeActionDel::UpdateReference( const ScChangeTrack* pTrack,
/*N*/ 		UpdateRefMode eMode, const ScBigRange& rRange,
/*N*/ 		INT32 nDx, INT32 nDy, INT32 nDz )
/*N*/ {
/*N*/ 	ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, GetBigRange() );
/*N*/ 	if ( !IsDeletedIn() )
/*N*/ 		return ;
/*N*/ 	// evtl. in "druntergerutschten" anpassen
/*N*/ 	for ( ScChangeActionLinkEntry* pL = pLinkDeleted; pL; pL = pL->GetNext() )
/*N*/ 	{
/*N*/ 		ScChangeAction* p = pL->GetAction();
/*N*/ 		if ( p && p->GetType() == SC_CAT_CONTENT &&
/*N*/ 				!GetBigRange().In( p->GetBigRange() ) )
/*N*/ 		{
/*N*/ 			switch ( GetType() )
/*N*/ 			{
/*N*/ 				case SC_CAT_DELETE_COLS :
/*N*/ 					p->GetBigRange().aStart.SetCol( GetBigRange().aStart.Col() );
/*N*/ 					p->GetBigRange().aEnd.SetCol( GetBigRange().aStart.Col() );
/*N*/ 				break;
/*N*/ 				case SC_CAT_DELETE_ROWS :
/*N*/ 					p->GetBigRange().aStart.SetRow( GetBigRange().aStart.Row() );
/*N*/ 					p->GetBigRange().aEnd.SetRow( GetBigRange().aStart.Row() );
/*N*/ 				break;
/*N*/ 				case SC_CAT_DELETE_TABS :
/*N*/ 					p->GetBigRange().aStart.SetTab( GetBigRange().aStart.Tab() );
/*N*/ 					p->GetBigRange().aEnd.SetTab( GetBigRange().aStart.Tab() );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ ScBigRange ScChangeActionDel::GetOverAllRange() const
/*N*/ {
/*N*/ 	ScBigRange aTmpRange( GetBigRange() );
/*N*/ 	aTmpRange.aEnd.SetCol( aTmpRange.aEnd.Col() + GetDx() );
/*N*/ 	aTmpRange.aEnd.SetRow( aTmpRange.aEnd.Row() + GetDy() );
/*N*/ 	return aTmpRange;
/*N*/ }


/*N*/ void ScChangeActionDel::GetDescription( String& rStr, ScDocument* pDoc,
/*N*/ 		BOOL bSplitRange ) const
/*N*/ {
/*N*/ 	USHORT nWhatId;
/*N*/ 	switch ( GetType() )
/*N*/ 	{
/*N*/ 		case SC_CAT_DELETE_COLS :
/*N*/ 			nWhatId = STR_COLUMN;
/*N*/ 		break;
/*N*/ 		case SC_CAT_DELETE_ROWS :
/*N*/ 			nWhatId = STR_ROW;
/*N*/ 		break;
/*N*/ 		default:
/*N*/ 			nWhatId = STR_AREA;
/*N*/ 	}
/*N*/ 
/*N*/ 	ScBigRange aTmpRange( GetBigRange() );
/*N*/ 	if ( !IsRejected() )
/*N*/ 	{
/*N*/ 		if ( bSplitRange )
/*N*/ 		{
/*N*/ 			aTmpRange.aStart.SetCol( aTmpRange.aStart.Col() + GetDx() );
/*N*/ 			aTmpRange.aStart.SetRow( aTmpRange.aStart.Row() + GetDy() );
/*N*/ 		}
/*N*/ 		aTmpRange.aEnd.SetCol( aTmpRange.aEnd.Col() + GetDx() );
/*N*/ 		aTmpRange.aEnd.SetRow( aTmpRange.aEnd.Row() + GetDy() );
/*N*/ 	}
/*N*/ 
/*N*/ 	String aRsc( ScGlobal::GetRscString( STR_CHANGED_DELETE ) );
/*N*/ 	xub_StrLen nPos = aRsc.SearchAscii( "#1" );
/*N*/ 	rStr += aRsc.Copy( 0, nPos );
/*N*/ 	rStr += ScGlobal::GetRscString( nWhatId );
/*N*/ 	rStr += ' ';
/*N*/ 	rStr += GetRefString( aTmpRange, pDoc );
/*N*/ 	rStr += aRsc.Copy( nPos+2 );
/*N*/ }


/*N*/ BOOL ScChangeActionDel::Reject( ScDocument* pDoc )
/*N*/ {
/*N*/ 	if ( !aBigRange.IsValid( pDoc ) && GetType() != SC_CAT_DELETE_TABS )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 
/*N*/ 	if ( IsTopDelete() )
/*N*/ 	{	// den kompletten Bereich in einem Rutsch restaurieren
/*N*/ 		ScBigRange aTmpRange( GetOverAllRange() );
/*N*/ 		if ( !aTmpRange.IsValid( pDoc ) )
/*N*/ 		{
/*N*/ 			if ( GetType() == SC_CAT_DELETE_TABS )
/*N*/ 			{	// wird Tab angehaengt?
/*N*/ 				if ( aTmpRange.aStart.Tab() > pDoc->GetMaxTableNumber() )
/*N*/ 					bOk = FALSE;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bOk = FALSE;
/*N*/ 		}
/*N*/ 		if ( bOk )
/*N*/ 		{
/*N*/ 			ScRange aRange( aTmpRange.MakeRange() );
/*N*/ 			// InDelete... fuer Formel UpdateReference in Document
/*N*/ 			pTrack->SetInDeleteRange( aRange );
/*N*/ 			pTrack->SetInDeleteTop( TRUE );
/*N*/ 			pTrack->SetInDeleteUndo( TRUE );
/*N*/ 			pTrack->SetInDelete( TRUE );
/*N*/ 			switch ( GetType() )
/*N*/ 			{
/*N*/ 				case SC_CAT_DELETE_COLS :
/*N*/ 					if ( !(aRange.aStart.Col() == 0 && aRange.aEnd.Col() == MAXCOL) )
/*N*/ 					{	// nur wenn nicht TabDelete
/*N*/ 						if ( bOk = pDoc->CanInsertCol( aRange ) )
/*N*/ 							bOk = pDoc->InsertCol( aRange );
/*N*/ 					}
/*N*/ 				break;
/*N*/ 				case SC_CAT_DELETE_ROWS :
/*N*/ 					if ( bOk = pDoc->CanInsertRow( aRange ) )
/*N*/ 						bOk = pDoc->InsertRow( aRange );
/*N*/ 				break;
/*N*/ 				case SC_CAT_DELETE_TABS :
/*N*/ 				{
/*N*/ //2do: Tabellennamen merken?
/*N*/ 					String aName;
/*N*/ 					pDoc->CreateValidTabName( aName );
/*N*/ 					if ( bOk = pDoc->ValidNewTabName( aName ) )
/*N*/ 						bOk = pDoc->InsertTab( aRange.aStart.Tab(), aName );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			pTrack->SetInDelete( FALSE );
/*N*/ 			pTrack->SetInDeleteUndo( FALSE );
/*N*/ 		}
/*N*/ 		if ( !bOk )
/*N*/ 		{
/*N*/ 			pTrack->SetInDeleteTop( FALSE );
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 		// InDeleteTop fuer UpdateReference-Undo behalten
/*N*/ 	}
/*N*/ 
/*N*/ 	// setzt rejected und ruft UpdateReference-Undo und DeleteCellEntries
/*N*/ 	RejectRestoreContents( pTrack, GetDx(), GetDy() );
/*N*/ 
/*N*/ 	pTrack->SetInDeleteTop( FALSE );
/*N*/ 	RemoveAllLinks();
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ void ScChangeActionDel::UndoCutOffMoves()
/*N*/ {	// abgeschnittene Moves wiederherstellen, Entries/Links deleten
/*N*/ 	while ( pLinkMove )
/*N*/ 	{
/*N*/ 		ScChangeActionMove* pMove = pLinkMove->GetMove();
/*N*/ 		short nFrom = pLinkMove->GetCutOffFrom();
/*N*/ 		short nTo = pLinkMove->GetCutOffTo();
/*N*/ 		switch ( GetType() )
/*N*/ 		{
/*N*/ 			case SC_CAT_DELETE_COLS :
/*N*/ 				if ( nFrom > 0 )
/*N*/ 					pMove->GetFromRange().aStart.IncCol( -nFrom );
/*N*/ 				else if ( nFrom < 0 )
/*N*/ 					pMove->GetFromRange().aEnd.IncCol( -nFrom );
/*N*/ 				if ( nTo > 0 )
/*N*/ 					pMove->GetBigRange().aStart.IncCol( -nTo );
/*N*/ 				else if ( nTo < 0 )
/*N*/ 					pMove->GetBigRange().aEnd.IncCol( -nTo );
/*N*/ 			break;
/*N*/ 			case SC_CAT_DELETE_ROWS :
/*N*/ 				if ( nFrom > 0 )
/*N*/ 					pMove->GetFromRange().aStart.IncRow( -nFrom );
/*N*/ 				else if ( nFrom < 0 )
/*N*/ 					pMove->GetFromRange().aEnd.IncRow( -nFrom );
/*N*/ 				if ( nTo > 0 )
/*N*/ 					pMove->GetBigRange().aStart.IncRow( -nTo );
/*N*/ 				else if ( nTo < 0 )
/*N*/ 					pMove->GetBigRange().aEnd.IncRow( -nTo );
/*N*/ 			break;
/*N*/ 			case SC_CAT_DELETE_TABS :
/*N*/ 				if ( nFrom > 0 )
/*N*/ 					pMove->GetFromRange().aStart.IncTab( -nFrom );
/*N*/ 				else if ( nFrom < 0 )
/*N*/ 					pMove->GetFromRange().aEnd.IncTab( -nFrom );
/*N*/ 				if ( nTo > 0 )
/*N*/ 					pMove->GetBigRange().aStart.IncTab( -nTo );
/*N*/ 				else if ( nTo < 0 )
/*N*/ 					pMove->GetBigRange().aEnd.IncTab( -nTo );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		delete pLinkMove;		// rueckt sich selbst hoch
/*N*/ 	}
/*N*/ }

/*N*/ void ScChangeActionDel::UndoCutOffInsert()
/*N*/ {	// abgeschnittenes Insert wiederherstellen
/*N*/ 	if ( pCutOff )
/*N*/ 	{
/*N*/ 		switch ( pCutOff->GetType() )
/*N*/ 		{
/*N*/ 			case SC_CAT_INSERT_COLS :
/*N*/ 				if ( nCutOff < 0 )
/*N*/ 					pCutOff->GetBigRange().aEnd.IncCol( -nCutOff );
/*N*/ 				else
/*N*/ 					pCutOff->GetBigRange().aStart.IncCol( -nCutOff );
/*N*/ 			break;
/*N*/ 			case SC_CAT_INSERT_ROWS :
/*N*/ 				if ( nCutOff < 0 )
/*N*/ 					pCutOff->GetBigRange().aEnd.IncRow( -nCutOff );
/*N*/ 				else
/*N*/ 					pCutOff->GetBigRange().aStart.IncRow( -nCutOff );
/*N*/ 			break;
/*N*/ 			case SC_CAT_INSERT_TABS :
/*N*/ 				if ( nCutOff < 0 )
/*N*/ 					pCutOff->GetBigRange().aEnd.IncTab( -nCutOff );
/*N*/ 				else
/*N*/ 					pCutOff->GetBigRange().aStart.IncTab( -nCutOff );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		SetCutOffInsert( NULL, 0 );
/*N*/ 	}
/*N*/ }


// --- ScChangeActionMove --------------------------------------------------

/*N*/ ScChangeActionMove::ScChangeActionMove( SvStream& rStrm,
/*N*/ 			ScMultipleReadHeader& rHdr, ScChangeTrack* pTrackP )
/*N*/ 		:
/*N*/ 		ScChangeAction( rStrm, rHdr, pTrackP ),
/*N*/ 		pTrack( pTrackP ),
/*N*/ 		pFirstCell( NULL ),
/*N*/ 		nStartLastCut(0),
/*N*/ 		nEndLastCut(0)
/*N*/ {
/*N*/ rStrm >> aFromRange;
/*N*/ }

/*N*/ ScChangeActionMove::ScChangeActionMove(const ULONG nActionNumber, const ScChangeActionState eState, const ULONG nRejectingNumber,
/*N*/ 									const ScBigRange& aToBigRange, const String& aUser, const DateTime& aDateTime, const String &sComment,
/*N*/ 									const ScBigRange& aFromBigRange, ScChangeTrack* pTrackP) // wich of nDx and nDy is set is depend on the type
/*N*/ 		:
/*N*/ 		ScChangeAction(SC_CAT_MOVE, aToBigRange, nActionNumber, nRejectingNumber, eState, aDateTime, aUser, sComment),
/*N*/ 		aFromRange(aFromBigRange),
/*N*/ 		pTrack( pTrackP ),
/*N*/ 		pFirstCell( NULL ),
/*N*/ 		nStartLastCut(0),
/*N*/ 		nEndLastCut(0)
/*N*/ {
/*N*/ }

/*N*/ ScChangeActionMove::~ScChangeActionMove()
/*N*/ {
/*N*/ 	DeleteCellEntries();
/*N*/ }


/*N*/ BOOL ScChangeActionMove::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::Store( rStrm, rHdr );
/*N*/ 	rStrm << aFromRange;
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScChangeActionMove::StoreLinks( SvStream& rStrm ) const
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::StoreLinks( rStrm );
/*N*/ 	bOk &= ScChangeAction::StoreCellList( pFirstCell, rStrm );
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScChangeActionMove::LoadLinks( SvStream& rStrm, ScChangeTrack* pTrack )
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::LoadLinks( rStrm, pTrack );
/*N*/ 	bOk &= ScChangeAction::LoadCellList( this, pFirstCell, rStrm, pTrack );
/*N*/ 	return bOk;
/*N*/ }


/*N*/ void ScChangeActionMove::AddContent( ScChangeActionContent* pContent )
/*N*/ {
/*N*/ 	ScChangeActionCellListEntry* pE = new ScChangeActionCellListEntry(
/*N*/ 		pContent, pFirstCell );
/*N*/ 	pFirstCell = pE;
/*N*/ }


/*N*/ void ScChangeActionMove::DeleteCellEntries()
/*N*/ {
/*N*/ 	pTrack->DeleteCellEntries( pFirstCell, this );
/*N*/ }


/*N*/ void ScChangeActionMove::UpdateReference( const ScChangeTrack* pTrack,
/*N*/ 		UpdateRefMode eMode, const ScBigRange& rRange,
/*N*/ 		INT32 nDx, INT32 nDy, INT32 nDz )
/*N*/ {
/*N*/ 	ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, aFromRange );
/*N*/ 	ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, GetBigRange() );
/*N*/ }


/*N*/ void ScChangeActionMove::GetDelta( INT32& nDx, INT32& nDy, INT32& nDz ) const
/*N*/ {
/*N*/ 	const ScBigAddress& rToPos = GetBigRange().aStart;
/*N*/ 	const ScBigAddress& rFromPos = GetFromRange().aStart;
/*N*/ 	nDx = rToPos.Col() - rFromPos.Col();
/*N*/ 	nDy = rToPos.Row() - rFromPos.Row();
/*N*/ 	nDz = rToPos.Tab() - rFromPos.Tab();
/*N*/ }


/*N*/ void ScChangeActionMove::GetDescription( String& rStr, ScDocument* pDoc,
/*N*/ 		BOOL bSplitRange ) const
/*N*/ {
/*N*/ 	BOOL bFlag3D = ( GetFromRange().aStart.Tab() != GetBigRange().aStart.Tab() );
/*N*/ 
/*N*/ 	String aRsc( ScGlobal::GetRscString( STR_CHANGED_MOVE ) );
/*N*/ 
/*N*/ 	xub_StrLen nPos = 0;
/*N*/ 	String aTmpStr = ScChangeAction::GetRefString( GetFromRange(), pDoc, bFlag3D );
/*N*/ 	nPos = aRsc.SearchAscii( "#1", nPos );
/*N*/ 	aRsc.Erase( nPos, 2 );
/*N*/ 	aRsc.Insert( aTmpStr, nPos );
/*N*/ 	nPos += aTmpStr.Len();
/*N*/ 
/*N*/ 	aTmpStr = ScChangeAction::GetRefString( GetBigRange(), pDoc, bFlag3D );
/*N*/ 	nPos = aRsc.SearchAscii( "#2", nPos );
/*N*/ 	aRsc.Erase( nPos, 2 );
/*N*/ 	aRsc.Insert( aTmpStr, nPos );
/*N*/ 	nPos += aTmpStr.Len();
/*N*/ 
/*N*/ 	rStr += aRsc;
/*N*/ }


/*N*/ void ScChangeActionMove::GetRefString( String& rStr, ScDocument* pDoc,
/*N*/ 		BOOL bFlag3D ) const
/*N*/ {
/*N*/ 	if ( !bFlag3D )
/*N*/ 		bFlag3D = ( GetFromRange().aStart.Tab() != GetBigRange().aStart.Tab() );
/*N*/ 	rStr = ScChangeAction::GetRefString( GetFromRange(), pDoc, bFlag3D );
/*N*/ 	rStr += ',';
/*N*/ 	rStr += ' ';
/*N*/ 	rStr += ScChangeAction::GetRefString( GetBigRange(), pDoc, bFlag3D );
/*N*/ }


/*N*/ BOOL ScChangeActionMove::Reject( ScDocument* pDoc )
/*N*/ {
/*N*/ 	if ( !(aBigRange.IsValid( pDoc ) && aFromRange.IsValid( pDoc )) )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ScRange aToRange( aBigRange.MakeRange() );
/*N*/ 	ScRange aFrmRange( aFromRange.MakeRange() );
/*N*/ 
/*N*/ 	BOOL bOk = pDoc->IsBlockEditable( aToRange.aStart.Tab(),
/*N*/ 		aToRange.aStart.Col(), aToRange.aStart.Row(),
/*N*/ 		aToRange.aEnd.Col(), aToRange.aEnd.Row() );
/*N*/ 	if ( bOk )
/*N*/ 		bOk = pDoc->IsBlockEditable( aFrmRange.aStart.Tab(),
/*N*/ 			aFrmRange.aStart.Col(), aFrmRange.aStart.Row(),
/*N*/ 			aFrmRange.aEnd.Col(), aFrmRange.aEnd.Row() );
/*N*/ 	if ( !bOk )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	pTrack->LookUpContents( aToRange, pDoc, 0, 0, 0 );	// zu movende Contents
/*N*/ 
/*N*/ 	pDoc->DeleteAreaTab( aToRange, IDF_ALL );
/*N*/ 	pDoc->DeleteAreaTab( aFrmRange, IDF_ALL );
/*N*/ 	// Formeln im Dokument anpassen
/*N*/ 	pDoc->UpdateReference( URM_MOVE,
/*N*/ 		aFrmRange.aStart.Col(), aFrmRange.aStart.Row(), aFrmRange.aStart.Tab(),
/*N*/ 		aFrmRange.aEnd.Col(), aFrmRange.aEnd.Row(), aFrmRange.aEnd.Tab(),
/*N*/ 		(short) aFrmRange.aStart.Col() - aToRange.aStart.Col(),
/*N*/ 		(short) aFrmRange.aStart.Row() - aToRange.aStart.Row(),
/*N*/ 		(short) aFrmRange.aStart.Tab() - aToRange.aStart.Tab(), NULL );
/*N*/ 
/*N*/ 	// LinkDependent freigeben, nachfolgendes UpdateReference-Undo setzt
/*N*/ 	// ToRange->FromRange Dependents
/*N*/ 	RemoveAllDependent();
/*N*/ 
/*N*/ 	// setzt rejected und ruft UpdateReference-Undo und DeleteCellEntries
/*N*/ 	RejectRestoreContents( pTrack, 0, 0 );
/*N*/ 
/*N*/ 	while ( pLinkDependent )
/*N*/ 	{
/*N*/ 		ScChangeAction* p = pLinkDependent->GetAction();
/*N*/ 		if ( p && p->GetType() == SC_CAT_CONTENT )
/*N*/ 		{
/*N*/ 			ScChangeActionContent* pContent = (ScChangeActionContent*) p;
/*N*/ 			if ( !pContent->IsDeletedIn() &&
/*N*/ 					pContent->GetBigRange().aStart.IsValid( pDoc ) )
/*N*/ 				pContent->PutNewValueToDoc( pDoc, 0, 0 );
/*N*/ 			// in LookUpContents generierte loeschen
/*N*/ 			if ( pTrack->IsGenerated( pContent->GetActionNumber() ) &&
/*N*/ 					!pContent->IsDeletedIn() )
/*N*/ 			{
/*N*/ 				pLinkDependent->UnLink();		//! sonst wird der mitgeloescht
/*N*/ 				pTrack->DeleteGeneratedDelContent( pContent );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		delete pLinkDependent;
/*N*/ 	}
/*N*/ 
/*N*/ 	RemoveAllLinks();
/*N*/ 	return TRUE;
/*N*/ }


// --- ScChangeActionContent -----------------------------------------------

const USHORT nMemPoolChangeActionContent = (0x8000 - 64) / sizeof(ScChangeActionContent);
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeActionContent, nMemPoolChangeActionContent, nMemPoolChangeActionContent )//STRIP008 ;


/*N*/ ScChangeActionContent::ScChangeActionContent( SvStream& rStrm,
/*N*/ 			ScMultipleReadHeader& rHdr, ScDocument* pDoc, USHORT nVer,
/*N*/ 			ScChangeTrack* pTrack )
/*N*/ 		:
/*N*/ 		ScChangeAction( rStrm, rHdr, pTrack ),
/*N*/ 		pNextInSlot( NULL ),
/*N*/ 		ppPrevInSlot( NULL )
/*N*/ {
/*N*/ UINT32 n32; // Changetracking.sdc
/*N*/ 	rStrm.ReadByteString( aOldValue, rStrm.GetStreamCharSet() );
/*N*/ 	rStrm.ReadByteString( aNewValue, rStrm.GetStreamCharSet() );
/*N*/ 	rStrm >> n32; pNextContent = (ScChangeActionContent*)(ULONG) n32;
/*N*/ 	rStrm >> n32; pPrevContent = (ScChangeActionContent*)(ULONG) n32;
/*N*/ 
/*N*/ 	{
/*N*/ 		ScMultipleReadHeader aDataHdr( rStrm );
/*N*/ 		pOldCell = ScChangeAction::LoadCell( rStrm, aDataHdr, pDoc, nVer );
/*N*/ 		pNewCell = ScChangeAction::LoadCell( rStrm, aDataHdr, pDoc, nVer );
/*N*/ 	}
/*N*/ }

/*N*/ ScChangeActionContent::ScChangeActionContent( const ULONG nActionNumber,
/*N*/ 			const ScChangeActionState eState, const ULONG nRejectingNumber,
/*N*/ 			const ScBigRange& aBigRange, const String& aUser,
/*N*/ 			const DateTime& aDateTime, const String& sComment,
/*N*/ 			ScBaseCell* pTempOldCell, ScDocument* pDoc, const String& sResult )
/*N*/ 		:
/*N*/ 		ScChangeAction(SC_CAT_CONTENT, aBigRange, nActionNumber, nRejectingNumber, eState, aDateTime, aUser, sComment),
/*N*/ 		pOldCell(pTempOldCell),
/*N*/ 		pNewCell(NULL),
/*N*/ 		pNextContent(NULL),
/*N*/ 		pPrevContent(NULL),
/*N*/ 		pNextInSlot(NULL),
/*N*/ 		ppPrevInSlot(NULL),
/*N*/ 		aOldValue(sResult)
/*N*/ 
/*N*/ {
/*N*/ if (pOldCell)
/*N*/ 		ScChangeActionContent::SetCell( aOldValue, pOldCell, 0, pDoc );
/*N*/ }

/*N*/ ScChangeActionContent::ScChangeActionContent( const ULONG nActionNumber,
/*N*/ 			ScBaseCell* pTempNewCell, const ScBigRange& aBigRange,
/*N*/ 			ScDocument* pDoc )
/*N*/ 		:
/*N*/ 		ScChangeAction(SC_CAT_CONTENT, aBigRange, nActionNumber),
/*N*/ 		pNewCell(pTempNewCell),
/*N*/ 		pOldCell(NULL),
/*N*/ 		pNextContent(NULL),
/*N*/ 		pPrevContent(NULL),
/*N*/ 		pNextInSlot(NULL),
/*N*/ 		ppPrevInSlot(NULL)
/*N*/ {
/*N*/ if (pNewCell)
/*N*/ 		ScChangeActionContent::SetCell( aNewValue, pNewCell, 0, pDoc );
/*N*/ }

/*N*/ ScChangeActionContent::~ScChangeActionContent()
/*N*/ {
/*N*/ 	ClearTrack();
/*N*/ }


/*N*/ void ScChangeActionContent::ClearTrack()
/*N*/ {
/*N*/ 	RemoveFromSlot();
/*N*/ 	if ( pPrevContent )
/*N*/ 		pPrevContent->pNextContent = pNextContent;
/*N*/ 	if ( pNextContent )
/*N*/ 		pNextContent->pPrevContent = pPrevContent;
/*N*/ }


/*N*/ BOOL ScChangeActionContent::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::Store( rStrm, rHdr );
/*N*/ 	rStrm.WriteByteString( aOldValue, rStrm.GetStreamCharSet() );
/*N*/ 	rStrm.WriteByteString( aNewValue, rStrm.GetStreamCharSet() );
/*N*/ 	rStrm << (UINT32) ( pNextContent ? pNextContent->GetActionNumber() : 0 );
/*N*/ 	rStrm << (UINT32) ( pPrevContent ? pPrevContent->GetActionNumber() : 0 );
/*N*/ 
/*N*/ 	{
/*N*/ 		ScMultipleWriteHeader aDataHdr( rStrm );
/*N*/ 		ScChangeAction::StoreCell( pOldCell, rStrm, aDataHdr );
/*N*/ 		ScChangeAction::StoreCell( pNewCell, rStrm, aDataHdr );
/*N*/ 	}
/*N*/ 
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScChangeActionContent::StoreLinks( SvStream& rStrm ) const
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::StoreLinks( rStrm );
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScChangeActionContent::LoadLinks( SvStream& rStrm, ScChangeTrack* pTrack )
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::LoadLinks( rStrm, pTrack );
/*N*/ 	if ( pNextContent )
/*N*/ 	{
/*N*/ 		pNextContent = (ScChangeActionContent*) pTrack->GetAction(
/*N*/ 			(ULONG) pNextContent );
/*N*/ 		DBG_ASSERT( pNextContent,
/*N*/ 			"ScChangeActionContent::LoadLinks: missing NextContent" );
/*N*/ 	}
/*N*/ 	if ( pPrevContent )
/*N*/ 	{
/*N*/ 		pPrevContent = (ScChangeActionContent*) pTrack->GetAction(
/*N*/ 			(ULONG) pPrevContent );
/*N*/ 		DBG_ASSERT( pPrevContent,
/*N*/ 			"ScChangeActionContent::LoadLinks: missing PrevContent" );
/*N*/ 	}
/*N*/ 	return bOk;
/*N*/ }


/*N*/ ScChangeActionContent* ScChangeActionContent::GetTopContent() const // Changetracking.sdc
/*N*/ {
/*N*/ 	if ( pNextContent )
/*N*/ 	{
/*N*/ 		ScChangeActionContent* pContent = pNextContent;
/*N*/ 		while ( pContent->pNextContent )
/*N*/ 			pContent = pContent->pNextContent;
/*N*/ 		return pContent;
/*N*/ 	}
/*N*/ 	return (ScChangeActionContent*) this;
/*N*/ }


/*N*/ ScChangeActionLinkEntry* ScChangeActionContent::GetDeletedIn() const
/*N*/ {
/*N*/ 	if ( pNextContent )
/*N*/ 		return GetTopContent()->pLinkDeletedIn;
/*N*/ 	return pLinkDeletedIn;
/*N*/ }


/*N*/ ScChangeActionLinkEntry** ScChangeActionContent::GetDeletedInAddress()
/*N*/ {
/*N*/ 	if ( pNextContent )
/*N*/ 		return GetTopContent()->GetDeletedInAddress();
/*N*/ 	return &pLinkDeletedIn;
/*N*/ }


/*N*/ void ScChangeActionContent::SetOldValue( const ScBaseCell* pCell,
/*N*/ 		const ScDocument* pFromDoc, ScDocument* pToDoc )
/*N*/ {
/*N*/ 	ScChangeActionContent::SetValue( aOldValue, pOldCell,
/*N*/ 		aBigRange.aStart.MakeAddress(), pCell, pFromDoc, pToDoc );
/*N*/ }


/*N*/ void ScChangeActionContent::SetNewValue( const ScBaseCell* pCell,
/*N*/ 		ScDocument* pDoc )
/*N*/ {
/*N*/ 	ScChangeActionContent::SetValue( aNewValue, pNewCell,
/*N*/ 		aBigRange.aStart.MakeAddress(), pCell, pDoc, pDoc );
/*N*/ }


/*N*/  void ScChangeActionContent::SetNewCell( ScBaseCell* pCell, ScDocument* pDoc )
/*N*/  {
/*N*/ 	DBG_ASSERT( !pNewCell, "ScChangeActionContent::SetNewCell: overwriting existing cell" );
/*N*/ 	pNewCell = pCell;
/*N*/ 	ScChangeActionContent::SetCell( aNewValue, pNewCell, 0, pDoc );
/*N*/ }

/*N*/ void ScChangeActionContent::GetOldString( String& rStr ) const
/*N*/ {
/*N*/ 	GetValueString( rStr, aOldValue, pOldCell );
/*N*/ }


/*N*/ void ScChangeActionContent::GetNewString( String& rStr ) const
/*N*/ {
/*N*/ 	GetValueString( rStr, aNewValue, pNewCell );
/*N*/ }


/*N*/ void ScChangeActionContent::GetDescription( String& rStr, ScDocument* pDoc,
/*N*/ 		BOOL bSplitRange ) const
/*N*/ {
/*N*/ 
/*N*/ 	String aRsc( ScGlobal::GetRscString( STR_CHANGED_CELL ) );
/*N*/ 
/*N*/ 	String aTmpStr;
/*N*/ 	GetRefString( aTmpStr, pDoc );
/*N*/ 
/*N*/ 	xub_StrLen nPos = 0;
/*N*/ 	nPos = aRsc.SearchAscii( "#1", nPos );
/*N*/ 	aRsc.Erase( nPos, 2 );
/*N*/ 	aRsc.Insert( aTmpStr, nPos );
/*N*/ 	nPos += aTmpStr.Len();
/*N*/ 
/*N*/ 	GetOldString( aTmpStr );
/*N*/ 	if ( !aTmpStr.Len() )
/*N*/ 		aTmpStr = ScGlobal::GetRscString( STR_CHANGED_BLANK );
/*N*/ 	nPos = aRsc.SearchAscii( "#2", nPos );
/*N*/ 	aRsc.Erase( nPos, 2 );
/*N*/ 	aRsc.Insert( aTmpStr, nPos );
/*N*/ 	nPos += aTmpStr.Len();
/*N*/ 
/*N*/ 	GetNewString( aTmpStr );
/*N*/ 	if ( !aTmpStr.Len() )
/*N*/ 		aTmpStr = ScGlobal::GetRscString( STR_CHANGED_BLANK );
/*N*/ 	nPos = aRsc.SearchAscii( "#3", nPos );
/*N*/ 	aRsc.Erase( nPos, 2 );
/*N*/ 	aRsc.Insert( aTmpStr, nPos );
/*N*/ 
/*N*/ 	rStr += aRsc;
/*N*/ }


/*N*/ void ScChangeActionContent::GetRefString( String& rStr, ScDocument* pDoc,
/*N*/ 		BOOL bFlag3D ) const
/*N*/ {
/*N*/ 	USHORT nFlags = ( GetBigRange().IsValid( pDoc ) ? SCA_VALID : 0 );
/*N*/ 	if ( nFlags )
/*N*/ 	{
/*N*/ 		const ScBaseCell* pCell = GetNewCell();
/*N*/ 		if ( ScChangeActionContent::GetContentCellType( pCell ) == SC_CACCT_MATORG )
/*N*/ 		{
/*N*/ 			ScBigRange aBigRange( GetBigRange() );
/*N*/ 			USHORT nC, nR;
/*N*/ 			((const ScFormulaCell*)pCell)->GetMatColsRows( nC, nR );
/*N*/ 			aBigRange.aEnd.IncCol( nC-1 );
/*N*/ 			aBigRange.aEnd.IncRow( nR-1 );
/*N*/ 			rStr = ScChangeAction::GetRefString( aBigRange, pDoc, bFlag3D );
/*N*/ 
/*N*/ 			return ;
/*N*/ 		}
/*N*/ 
/*N*/ 		ScAddress aTmpAddress( GetBigRange().aStart.MakeAddress() );
/*N*/ 		if ( bFlag3D )
/*N*/ 			nFlags |= SCA_TAB_3D;
/*N*/ 		aTmpAddress.Format( rStr, nFlags, pDoc );
/*N*/ 		if ( IsDeletedIn() )
/*N*/ 		{
/*N*/ 			rStr.Insert( '(', 0 );
/*N*/ 			rStr += ')';
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rStr = ScGlobal::GetRscString( STR_NOREF_STR );
/*N*/ }


/*N*/ BOOL ScChangeActionContent::Reject( ScDocument* pDoc )
/*N*/ {
/*N*/ 	if ( !aBigRange.IsValid( pDoc ) )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	PutOldValueToDoc( pDoc, 0, 0 );
/*N*/ 
/*N*/ 	SetState( SC_CAS_REJECTED );
/*N*/ 	RemoveAllLinks();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ ScChangeActionContentCellType ScChangeActionContent::GetContentCellType( const ScBaseCell* pCell )
/*N*/ {
/*N*/ 	if ( pCell )
/*N*/ 	{
/*N*/ 		switch ( pCell->GetCellType() )
/*N*/ 		{
/*N*/ 			case CELLTYPE_VALUE :
/*N*/ 			case CELLTYPE_STRING :
/*N*/ 			case CELLTYPE_EDIT :
/*N*/ 				return SC_CACCT_NORMAL;
/*N*/ 			break;
/*N*/ 			case CELLTYPE_FORMULA :
/*N*/ 				switch ( ((const ScFormulaCell*)pCell)->GetMatrixFlag() )
/*N*/ 				{
/*N*/ 					case MM_NONE :
/*N*/ 						return SC_CACCT_NORMAL;
/*N*/ 					break;
/*N*/ 					case MM_FORMULA :
/*N*/ 					case MM_FAKE :
/*N*/ 						return SC_CACCT_MATORG;
/*N*/ 					break;
/*N*/ 					case MM_REFERENCE :
/*N*/ 						return SC_CACCT_MATREF;
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				return SC_CACCT_NORMAL;
/*N*/ 			break;
/*N*/ 			default:
/*N*/ 				return SC_CACCT_NONE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return SC_CACCT_NONE;
/*N*/ }


// static
/*N*/ BOOL ScChangeActionContent::NeedsNumberFormat( const ScBaseCell* pCell )
/*N*/ {
/*N*/ 	return pCell && pCell->GetCellType() == CELLTYPE_VALUE;
/*N*/ }


// static
/*N*/ void ScChangeActionContent::SetValue( String& rStr, ScBaseCell*& pCell,
/*N*/ 		const ScAddress& rPos, const ScBaseCell* pOrgCell,
/*N*/ 		const ScDocument* pFromDoc, ScDocument* pToDoc )
/*N*/ {
/*N*/ 	if ( ScChangeActionContent::NeedsNumberFormat( pOrgCell ) )
/*N*/ 		ScChangeActionContent::SetValue( rStr, pCell,
/*N*/ 			pFromDoc->GetNumberFormat( rPos ), pOrgCell, pFromDoc, pToDoc );
/*N*/ 	else
/*N*/ 		ScChangeActionContent::SetValue( rStr, pCell,
/*N*/ 			0, pOrgCell, pFromDoc, pToDoc );
/*N*/ }


// static
/*N*/ void ScChangeActionContent::SetValue( String& rStr, ScBaseCell*& pCell,
/*N*/ 		ULONG nFormat, const ScBaseCell* pOrgCell,
/*N*/ 		const ScDocument* pFromDoc, ScDocument* pToDoc )
/*N*/ {
/*N*/ 	rStr.Erase();
/*N*/ 	if ( pCell )
/*N*/ 		pCell->Delete();
/*N*/ 	if ( ScChangeActionContent::GetContentCellType( pOrgCell ) )
/*N*/ 	{
/*N*/ 		pCell = pOrgCell->Clone( pToDoc );
/*N*/ 		switch ( pOrgCell->GetCellType() )
/*N*/ 		{
/*N*/ 			case CELLTYPE_VALUE :
/*N*/ 			{	// z.B. Datum auch als solches merken
/*N*/ 				double nValue = ((ScValueCell*)pOrgCell)->GetValue();
/*N*/ 				pFromDoc->GetFormatTable()->GetInputLineString( nValue,
/*N*/ 					nFormat, rStr );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case CELLTYPE_FORMULA :
/*N*/ 				((ScFormulaCell*)pCell)->SetInChangeTrack( TRUE );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pCell = NULL;
/*N*/ }


// static
/*N*/ void ScChangeActionContent::SetCell( String& rStr, ScBaseCell* pCell,
/*N*/ 		ULONG nFormat, const ScDocument* pDoc )
/*N*/ {
/*N*/ 	rStr.Erase();
/*N*/ 	if ( pCell )
/*N*/ 	{
/*N*/ 		switch ( pCell->GetCellType() )
/*N*/ 		{
/*N*/ 			case CELLTYPE_VALUE :
/*N*/ 			{	// e.g. remember date as date string
/*N*/ 				double nValue = ((ScValueCell*)pCell)->GetValue();
/*N*/ 				pDoc->GetFormatTable()->GetInputLineString( nValue,
/*N*/ 					nFormat, rStr );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case CELLTYPE_FORMULA :
/*N*/ 				((ScFormulaCell*)pCell)->SetInChangeTrack( TRUE );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeActionContent::GetValueString( String& rStr,
/*N*/ 		const String& rValue, const ScBaseCell* pCell ) const
/*N*/ {
/*N*/ 	if ( !rValue.Len() )
/*N*/ 	{
/*N*/ 		if ( pCell )
/*N*/ 		{
/*N*/ 			switch ( pCell->GetCellType() )
/*N*/ 			{
/*N*/ 				case CELLTYPE_STRING :
/*N*/ 					((ScStringCell*)pCell)->GetString( rStr );
/*N*/ 				break;
/*N*/ 				case CELLTYPE_EDIT :
/*N*/ 					((ScEditCell*)pCell)->GetString( rStr );
/*N*/ 				break;
/*N*/ 				case CELLTYPE_VALUE :	// ist immer in rValue
/*N*/ 					rStr = rValue;
/*N*/ 				break;
/*N*/ 				case CELLTYPE_FORMULA :
/*N*/ 					GetFormulaString( rStr, (ScFormulaCell*) pCell );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			rStr.Erase();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rStr = rValue;
/*N*/ }


/*N*/ void ScChangeActionContent::GetFormulaString( String& rStr,
/*N*/ 		const ScFormulaCell* pCell ) const
/*N*/ {
/*N*/ 	ScAddress aPos( aBigRange.aStart.MakeAddress() );
/*N*/ 	if ( aPos == pCell->aPos || IsDeletedIn() )
/*N*/ 		pCell->GetFormula( rStr );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ERROR( "ScChangeActionContent::GetFormulaString: aPos != pCell->aPos" );
/*N*/ 		ScFormulaCell* pNew = (ScFormulaCell*) pCell->Clone(
/*N*/ 			pCell->GetDocument(), aPos, TRUE );		// TRUE: bNoListening
/*N*/ 		pNew->GetFormula( rStr );
/*N*/ 		delete pNew;
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeActionContent::PutOldValueToDoc( ScDocument* pDoc,
/*N*/ 		short nDx, short nDy ) const
/*N*/ {
/*N*/ 	PutValueToDoc( pOldCell, aOldValue, pDoc, nDx, nDy );
/*N*/ }


/*N*/ void ScChangeActionContent::PutNewValueToDoc( ScDocument* pDoc,
/*N*/ 		short nDx, short nDy ) const
/*N*/ {
/*N*/ 	PutValueToDoc( pNewCell, aNewValue, pDoc, nDx, nDy );
/*N*/ }


/*N*/ void ScChangeActionContent::PutValueToDoc( ScBaseCell* pCell,
/*N*/ 		const String& rValue, ScDocument* pDoc, short nDx, short nDy ) const
/*N*/ {
/*N*/ 	ScAddress aPos( aBigRange.aStart.MakeAddress() );
/*N*/ 	if ( nDx )
/*N*/ 		aPos.IncCol( nDx );
/*N*/ 	if ( nDy )
/*N*/ 		aPos.IncRow( nDy );
/*N*/ 	if ( !rValue.Len() )
/*N*/ 	{
/*N*/ 		if ( pCell )
/*N*/ 		{
/*N*/ 			switch ( pCell->GetCellType() )
/*N*/ 			{
/*N*/ 				case CELLTYPE_VALUE :	// ist immer in rValue
/*N*/ 					pDoc->SetString( aPos.Col(), aPos.Row(), aPos.Tab(), rValue );
/*N*/ 				break;
/*N*/ 				default:
/*N*/ 					switch ( ScChangeActionContent::GetContentCellType( pCell ) )
/*N*/ 					{
/*N*/ 						case SC_CACCT_MATORG :
/*N*/ 						{
/*N*/ 							USHORT nC, nR;
/*N*/ 							((const ScFormulaCell*)pCell)->GetMatColsRows( nC, nR );
/*N*/ 							DBG_ASSERT( nC>0 && nR>0, "ScChangeActionContent::PutValueToDoc: MatColsRows?" );
/*N*/ 							ScRange aRange( aPos );
/*N*/ 							if ( nC > 1 )
/*N*/ 								aRange.aEnd.IncCol( nC-1 );
/*N*/ 							if ( nR > 1 )
/*N*/ 								aRange.aEnd.IncRow( nR-1 );
/*N*/ 							ScMarkData aDestMark;
/*N*/ 							aDestMark.SelectOneTable( aPos.Tab() );
/*N*/ 							aDestMark.SetMarkArea( aRange );
/*N*/ 							pDoc->InsertMatrixFormula( aPos.Col(), aPos.Row(),
/*N*/ 								aRange.aEnd.Col(), aRange.aEnd.Row(),
/*N*/ 								aDestMark, EMPTY_STRING,
/*N*/ 								((const ScFormulaCell*)pCell)->GetCode() );
/*N*/ 						}
/*N*/ 						break;
/*N*/ 						case SC_CACCT_MATREF :
/*N*/ 							// nothing
/*N*/ 						break;
/*N*/ 						default:
/*N*/ 							pDoc->PutCell( aPos, pCell->Clone( pDoc ) );
/*N*/ 					}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pDoc->PutCell( aPos, NULL );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pDoc->SetString( aPos.Col(), aPos.Row(), aPos.Tab(), rValue );
/*N*/ }


/*N*/ void lcl_InvalidateReference( ScToken& rTok, const ScBigAddress& rPos )
/*N*/ {
/*N*/ 	SingleRefData& rRef1 = rTok.GetSingleRef();
/*N*/ 	if ( rPos.Col() < 0 || MAXCOL < rPos.Col() )
/*N*/ 	{
/*N*/ 		rRef1.nCol = (INT16)(~0);
/*N*/ 		rRef1.nRelCol = (INT16)(~0);
/*N*/ 		rRef1.SetColDeleted( TRUE );
/*N*/ 	}
/*N*/ 	if ( rPos.Row() < 0 || MAXROW < rPos.Row() )
/*N*/ 	{
/*N*/ 		rRef1.nRow = (INT16)(~0);
/*N*/ 		rRef1.nRelRow = (INT16)(~0);
/*N*/ 		rRef1.SetRowDeleted( TRUE );
/*N*/ 	}
/*N*/ 	if ( rPos.Tab() < 0 || MAXTAB < rPos.Tab() )
/*N*/ 	{
/*N*/ 		rRef1.nTab = (INT16)(~0);
/*N*/ 		rRef1.nRelTab = (INT16)(~0);
/*N*/ 		rRef1.SetTabDeleted( TRUE );
/*N*/ 	}
/*N*/ 	if ( rTok.GetType() == svDoubleRef )
/*N*/ 	{
/*N*/ 		SingleRefData& rRef2 = rTok.GetDoubleRef().Ref2;
/*N*/ 		if ( rPos.Col() < 0 || MAXCOL < rPos.Col() )
/*N*/ 		{
/*N*/ 			rRef2.nCol = (INT16)(~0);
/*N*/ 			rRef2.nRelCol = (INT16)(~0);
/*N*/ 			rRef2.SetColDeleted( TRUE );
/*N*/ 		}
/*N*/ 		if ( rPos.Row() < 0 || MAXROW < rPos.Row() )
/*N*/ 		{
/*N*/ 			rRef2.nRow = (INT16)(~0);
/*N*/ 			rRef2.nRelRow = (INT16)(~0);
/*N*/ 			rRef2.SetRowDeleted( TRUE );
/*N*/ 		}
/*N*/ 		if ( rPos.Tab() < 0 || MAXTAB < rPos.Tab() )
/*N*/ 		{
/*N*/ 			rRef2.nTab = (INT16)(~0);
/*N*/ 			rRef2.nRelTab = (INT16)(~0);
/*N*/ 			rRef2.SetTabDeleted( TRUE );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeActionContent::UpdateReference( const ScChangeTrack* pTrack,
/*N*/ 		UpdateRefMode eMode, const ScBigRange& rRange,
/*N*/ 		INT32 nDx, INT32 nDy, INT32 nDz )
/*N*/ {
/*N*/ 	USHORT nOldSlot = ScChangeTrack::ComputeContentSlot( aBigRange.aStart.Row() );
/*N*/ 	ScRefUpdate::Update( eMode, rRange, nDx, nDy, nDz, aBigRange );
/*N*/ 	USHORT nNewSlot = ScChangeTrack::ComputeContentSlot( aBigRange.aStart.Row() );
/*N*/ 	if ( nNewSlot != nOldSlot )
/*N*/ 	{
/*N*/ 		RemoveFromSlot();
/*N*/ 		InsertInSlot( &(pTrack->GetContentSlots()[nNewSlot]) );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pTrack->IsInDelete() && !pTrack->IsInDeleteTop() )
/*N*/ 		return ;		// Formeln nur kompletten Bereich updaten
/*N*/ 
/*N*/ 	BOOL bOldFormula = ( pOldCell && pOldCell->GetCellType() == CELLTYPE_FORMULA );
/*N*/ 	BOOL bNewFormula = ( pNewCell && pNewCell->GetCellType() == CELLTYPE_FORMULA );
/*N*/ 	if ( bOldFormula || bNewFormula )
/*N*/ 	{	// via ScFormulaCell UpdateReference anpassen (dort)
/*N*/ 		if ( pTrack->IsInDelete() )
/*N*/ 		{
/*N*/ 			const ScRange& rDelRange = pTrack->GetInDeleteRange();
/*N*/ 			if ( nDx > 0 )
/*N*/ 				nDx = rDelRange.aEnd.Col() - rDelRange.aStart.Col() + 1;
/*N*/ 			else if ( nDx < 0 )
/*N*/ 				nDx = -(rDelRange.aEnd.Col() - rDelRange.aStart.Col() + 1);
/*N*/ 			if ( nDy > 0 )
/*N*/ 				nDy = rDelRange.aEnd.Row() - rDelRange.aStart.Row() + 1;
/*N*/ 			else if ( nDy < 0 )
/*N*/ 				nDy = -(rDelRange.aEnd.Row() - rDelRange.aStart.Row() + 1);
/*N*/ 			if ( nDz > 0 )
/*N*/ 				nDz = rDelRange.aEnd.Tab() - rDelRange.aStart.Tab() + 1;
/*N*/ 			else if ( nDz < 0 )
/*N*/ 				nDz = -(rDelRange.aEnd.Tab() - rDelRange.aStart.Tab() + 1);
/*N*/ 		}
/*N*/ 		ScBigRange aTmpRange( rRange );
/*N*/ 		switch ( eMode )
/*N*/ 		{
/*N*/ 			case URM_INSDEL :
/*N*/ 				if ( nDx < 0 || nDy < 0 || nDz < 0 )
/*N*/ 				{	// Delete startet dort hinter geloeschtem Bereich,
/*N*/ 					// Position wird dort angepasst.
/*N*/ 					if ( nDx )
/*N*/ 						aTmpRange.aStart.IncCol( -nDx );
/*N*/ 					if ( nDy )
/*N*/ 						aTmpRange.aStart.IncRow( -nDy );
/*N*/ 					if ( nDz )
/*N*/ 						aTmpRange.aStart.IncTab( -nDz );
/*N*/ 				}
/*N*/ 			break;
/*N*/ 			case URM_MOVE :
/*N*/ 				// Move ist hier Quelle, dort Ziel,
/*N*/ 				// Position muss vorher angepasst sein.
/*N*/ 				if ( bOldFormula )
/*N*/ 					((ScFormulaCell*)pOldCell)->aPos = aBigRange.aStart.MakeAddress();
/*N*/ 				if ( bNewFormula )
/*N*/ 					((ScFormulaCell*)pNewCell)->aPos = aBigRange.aStart.MakeAddress();
/*N*/ 				if ( nDx )
/*N*/ 				{
/*N*/ 					aTmpRange.aStart.IncCol( nDx );
/*N*/ 					aTmpRange.aEnd.IncCol( nDx );
/*N*/ 				}
/*N*/ 				if ( nDy )
/*N*/ 				{
/*N*/ 					aTmpRange.aStart.IncRow( nDy );
/*N*/ 					aTmpRange.aEnd.IncRow( nDy );
/*N*/ 				}
/*N*/ 				if ( nDz )
/*N*/ 				{
/*N*/ 					aTmpRange.aStart.IncTab( nDz );
/*N*/ 					aTmpRange.aEnd.IncTab( nDz );
/*N*/ 				}
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		ScRange aRange( aTmpRange.MakeRange() );
/*N*/ 		if ( bOldFormula )
/*N*/ 			((ScFormulaCell*)pOldCell)->UpdateReference( eMode, aRange,
/*N*/ 				(short) nDx, (short) nDy, (short) nDz, NULL );
/*N*/ 		if ( bNewFormula )
/*N*/ 			((ScFormulaCell*)pNewCell)->UpdateReference( eMode, aRange,
/*N*/ 				(short) nDx, (short) nDy, (short) nDz, NULL );
/*N*/ 		if ( !aBigRange.aStart.IsValid( pTrack->GetDocument() ) )
/*N*/ 		{	//! HACK!
/*N*/ 			//! UpdateReference kann nicht mit Positionen ausserhalb des
/*N*/ 			//! Dokuments umgehen, deswegen alles auf #REF! setzen
/*N*/ //2do: make it possible! das bedeutet grossen Umbau von ScAddress etc.!
/*N*/ 			const ScBigAddress& rPos = aBigRange.aStart;
/*N*/ 			if ( bOldFormula )
/*N*/ 			{
/*N*/ 				ScToken* t;
/*N*/ 				ScTokenArray* pArr = ((ScFormulaCell*)pOldCell)->GetCode();
/*N*/ 				pArr->Reset();
/*N*/ 				while ( t = pArr->GetNextReference() )
/*N*/ 					lcl_InvalidateReference( *t, rPos );
/*N*/ 				pArr->Reset();
/*N*/ 				while ( t = pArr->GetNextReferenceRPN() )
/*N*/ 					lcl_InvalidateReference( *t, rPos );
/*N*/ 			}
/*N*/ 			if ( bNewFormula )
/*N*/ 			{
/*N*/ 				ScToken* t;
/*N*/ 				ScTokenArray* pArr = ((ScFormulaCell*)pNewCell)->GetCode();
/*N*/ 				pArr->Reset();
/*N*/ 				while ( t = pArr->GetNextReference() )
/*N*/ 					lcl_InvalidateReference( *t, rPos );
/*N*/ 				pArr->Reset();
/*N*/ 				while ( t = pArr->GetNextReferenceRPN() )
/*N*/ 					lcl_InvalidateReference( *t, rPos );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


// --- ScChangeActionReject ------------------------------------------------

/*N*/ ScChangeActionReject::ScChangeActionReject( SvStream& rStrm,
/*N*/ 			ScMultipleReadHeader& rHdr, ScChangeTrack* pTrack )
/*N*/ 		:
/*N*/ 		ScChangeAction( rStrm, rHdr, pTrack )
/*N*/ {
/*N*/ }

/*N*/ ScChangeActionReject::ScChangeActionReject(const ULONG nActionNumber, const ScChangeActionState eState, const ULONG nRejectingNumber,
/*N*/ 												const ScBigRange& aBigRange, const String& aUser, const DateTime& aDateTime, const String& sComment)
/*N*/ 		:
/*N*/ 		ScChangeAction(SC_CAT_CONTENT, aBigRange, nActionNumber, nRejectingNumber, eState, aDateTime, aUser, sComment)
/*N*/ {
/*N*/ }

/*N*/ BOOL ScChangeActionReject::Store( SvStream& rStrm, ScMultipleWriteHeader& rHdr ) const
/*N*/ {
/*N*/ 	BOOL bOk = ScChangeAction::Store( rStrm, rHdr );
/*N*/ 	return TRUE;
/*N*/ }


// --- ScChangeTrack -------------------------------------------------------

/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScChangeTrackMsgInfo, 16, 16 )//STRIP008 ;

const USHORT ScChangeTrack::nContentRowsPerSlot = InitContentRowsPerSlot();
const USHORT ScChangeTrack::nContentSlots =
    (MAXROW+1) / InitContentRowsPerSlot() + 2;

// static
/*N*/ USHORT ScChangeTrack::InitContentRowsPerSlot()
/*N*/ {
/*N*/ 	const USHORT nMaxSlots = 0xffe0 / sizeof( ScChangeActionContent* ) - 2;
/*N*/ 	USHORT nRowsPerSlot = (MAXROW+1) / nMaxSlots;
/*N*/ 	if ( nRowsPerSlot * nMaxSlots < (MAXROW+1) )
/*N*/ 		++nRowsPerSlot;
/*N*/ 	return nRowsPerSlot;
/*N*/ }


/*N*/ ScChangeTrack::ScChangeTrack( ScDocument* pDocP ) : // Changetracking.sdc
/*N*/ 		pDoc( pDocP )
/*N*/ {
/*N*/ Init();
/*N*/ 	StartListening( *SfxGetpApp() );
/*N*/ 	ppContentSlots = new ScChangeActionContent* [ nContentSlots ];
/*N*/ 	memset( ppContentSlots, 0, nContentSlots * sizeof( ScChangeActionContent* ) );
/*N*/ }

/*N*/ ScChangeTrack::ScChangeTrack( ScDocument* pDocP, const StrCollection& aTempUserCollection) :
/*N*/ 		pDoc( pDocP ),
/*N*/ 		aUserCollection(aTempUserCollection)
/*N*/ {
/*N*/ Init();
/*N*/ 	StartListening( *SfxGetpApp() );
/*N*/ 	ppContentSlots = new ScChangeActionContent* [ nContentSlots ];
/*N*/ 	memset( ppContentSlots, 0, nContentSlots * sizeof( ScChangeActionContent* ) );
/*N*/ }

/*N*/ ScChangeTrack::~ScChangeTrack() // Changetracking.sdc
/*N*/ {
/*N*/ 	DtorClear();
/*N*/ 	delete [] ppContentSlots;
/*N*/ }


/*N*/ void ScChangeTrack::Init() // Changetracking.sdc
/*N*/ {
/*N*/ 	pFirst = NULL;
/*N*/ 	pLast = NULL;
/*N*/ 	pFirstGeneratedDelContent = NULL;
/*N*/ 	pLastCutMove = NULL;
/*N*/ 	pLinkInsertCol = NULL;
/*N*/ 	pLinkInsertRow = NULL;
/*N*/ 	pLinkInsertTab = NULL;
/*N*/ 	pLinkMove = NULL;
/*N*/ 	pBlockModifyMsg = NULL;
/*N*/ 	nActionMax = 0;
/*N*/ 	nGeneratedMin = SC_CHGTRACK_GENERATED_START;
/*N*/ 	nMarkLastSaved = 0;
/*N*/ 	nStartLastCut = 0;
/*N*/ 	nEndLastCut = 0;
/*N*/ 	nLastMerge = 0;
/*N*/ 	eMergeState = SC_CTMS_NONE;
/*N*/ 	nLoadedFileFormatVersion = SC_CHGTRACK_FILEFORMAT;
/*N*/ 	bLoadSave = FALSE;
/*N*/ 	bInDelete = FALSE;
/*N*/ 	bInDeleteTop = FALSE;
/*N*/ 	bInDeleteUndo = FALSE;
/*N*/ 	bInPasteCut = FALSE;
/*N*/ 	bUseFixDateTime = FALSE;
/*N*/     bTime100thSeconds = TRUE;
/*N*/ 
/*N*/ 	SvtUserOptions aUserOpt;
/*N*/ 	aUser = aUserOpt.GetFirstName();
/*N*/ 	aUser += ' ';
/*N*/ 	aUser += aUserOpt.GetLastName();
/*N*/ 	aUserCollection.Insert( new StrData( aUser ) );
/*N*/ }


/*N*/ void ScChangeTrack::DtorClear() // Changetracking.sdc
/*N*/ {
/*N*/ 	ScChangeAction* p;
/*N*/ 	ScChangeAction* pNext;
/*N*/ 	for ( p = GetFirst(); p; p = pNext )
/*N*/ 	{
/*N*/ 		pNext = p->GetNext();
/*N*/ 		delete p;
/*N*/ 	}
/*N*/ 	for ( p = pFirstGeneratedDelContent; p; p = pNext )
/*N*/ 	{
/*N*/ 		pNext = p->GetNext();
/*N*/ 		delete p;
/*N*/ 	}
/*N*/ 	for ( p = aPasteCutTable.First(); p; p = aPasteCutTable.Next() )
/*N*/ 	{
/*N*/ 		delete p;
/*N*/ 	}
/*N*/ 	delete pLastCutMove;
/*N*/ 	ClearMsgQueue();
/*N*/ }


/*N*/ void ScChangeTrack::ClearMsgQueue() // Changetracking.sdc
/*N*/ {
/*N*/ 	if ( pBlockModifyMsg )
/*N*/ 	{
/*N*/ 		delete pBlockModifyMsg;
/*N*/ 		pBlockModifyMsg = NULL;
/*N*/ 	}
/*N*/ 	ScChangeTrackMsgInfo* pMsgInfo;
/*N*/ 	while ( pMsgInfo = aMsgStackTmp.Pop() )
/*N*/ 		delete pMsgInfo;
/*N*/ 	while ( pMsgInfo = aMsgStackFinal.Pop() )
/*N*/ 		delete pMsgInfo;
/*N*/ 	while ( pMsgInfo = aMsgQueue.Get() )
/*N*/ 		delete pMsgInfo;
/*N*/ }


/*N*/ void ScChangeTrack::Clear() // Changetracking.sdc
/*N*/ {
/*N*/ 	DtorClear();
/*N*/ 	aTable.Clear();
/*N*/ 	aGeneratedTable.Clear();
/*N*/ 	aPasteCutTable.Clear();
/*N*/ 	aUserCollection.FreeAll();
/*N*/ 	aUser.Erase();
/*N*/ 	Init();
/*N*/ }


/*N*/ void __EXPORT ScChangeTrack::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
/*N*/ {
/*N*/ 	if ( !pDoc->IsInDtorClear() )
/*N*/ 	{
/*N*/ 		const SfxItemSetHint* pHint = PTR_CAST( SfxItemSetHint, &rHint );
/*N*/ 		if ( pHint )
/*N*/ 		{
/*N*/ 			const SfxItemSet& rSet = pHint->GetItemSet();
/*N*/ 			const SfxPoolItem* pItem;
/*N*/ 			if ( rSet.GetItemState(
/*N*/ 					rSet.GetPool()->GetWhich( SID_ATTR_ADDRESS ),
/*N*/ 					TRUE, &pItem ) == SFX_ITEM_SET )
/*N*/ 			{
/*N*/ 				USHORT nOldCount = aUserCollection.GetCount();
/*N*/ 
/*N*/ 				String aStr( ((SvxAddressItem*)pItem)->GetFirstName() );
/*N*/ 				aStr += ' ';
/*N*/ 				aStr += ((SvxAddressItem*)pItem)->GetName();
/*N*/ 				SetUser( aStr );
/*N*/ 
/*N*/ 				if ( aUserCollection.GetCount() != nOldCount )
/*N*/ 				{
/*N*/ 					//	New user in collection -> have to repaint because
/*N*/ 					//	colors may be different now (#106697#).
/*N*/ 					//	(Has to be done in the Notify handler, to be sure
/*N*/ 					//	the user collection has already been updated)
/*N*/ 
/*N*/ 					SfxObjectShell* pDocSh = pDoc->GetDocumentShell();
/*N*/ 					if (pDocSh)
/*N*/ 						pDocSh->Broadcast( ScPaintHint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB), PAINT_GRID ) );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeTrack::SetUser( const String& rUser ) // Changetracking.sdc
/*N*/ {
/*N*/ 	if ( IsLoadSave() )
/*N*/ 		return ;		// nicht die Collection zerschiessen
/*N*/ 
/*N*/ 	aUser = rUser;
/*N*/ 	StrData* pStrData = new StrData( aUser );
/*N*/ 	if ( !aUserCollection.Insert( pStrData ) )
/*N*/ 		delete pStrData;
/*N*/ }


/*N*/ void ScChangeTrack::StartBlockModify( ScChangeTrackMsgType eMsgType,
/*N*/ 		ULONG nStartAction )
/*N*/ {
/*N*/ 	if ( aModifiedLink.IsSet() )
/*N*/ 	{
/*N*/ 		if ( pBlockModifyMsg )
/*N*/ 			aMsgStackTmp.Push( pBlockModifyMsg );	// Block im Block
/*N*/ 		pBlockModifyMsg = new ScChangeTrackMsgInfo;
/*N*/ 		pBlockModifyMsg->eMsgType = eMsgType;
/*N*/ 		pBlockModifyMsg->nStartAction = nStartAction;
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeTrack::EndBlockModify( ULONG nEndAction )
/*N*/ {
/*N*/ 	if ( aModifiedLink.IsSet() )
/*N*/ 	{
/*N*/ 		if ( pBlockModifyMsg )
/*N*/ 		{
/*N*/ 			if ( pBlockModifyMsg->nStartAction <= nEndAction )
/*N*/ 			{
/*N*/ 				pBlockModifyMsg->nEndAction = nEndAction;
/*N*/ 				// Blocks in Blocks aufgeloest
/*N*/ 				aMsgStackFinal.Push( pBlockModifyMsg );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				delete pBlockModifyMsg;
/*N*/ 			pBlockModifyMsg = aMsgStackTmp.Pop();	// evtl. Block im Block
/*N*/ 		}
/*N*/ 		if ( !pBlockModifyMsg )
/*N*/ 		{
/*N*/ 			BOOL bNew = FALSE;
/*N*/ 			ScChangeTrackMsgInfo* pMsg;
/*N*/ 			while ( pMsg = aMsgStackFinal.Pop() )
/*N*/ 			{
/*N*/ 				aMsgQueue.Put( pMsg );
/*N*/ 				bNew = TRUE;
/*N*/ 			}
/*N*/ 			if ( bNew )
/*N*/ 				aModifiedLink.Call( this );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeTrack::NotifyModified( ScChangeTrackMsgType eMsgType,
/*N*/ 		ULONG nStartAction, ULONG nEndAction )
/*N*/ {
/*N*/ 	if ( aModifiedLink.IsSet() )
/*N*/ 	{
/*N*/ 		if ( !pBlockModifyMsg || pBlockModifyMsg->eMsgType != eMsgType ||
/*N*/ 				(IsGenerated( nStartAction ) &&
/*N*/ 				(eMsgType == SC_CTM_APPEND || eMsgType == SC_CTM_REMOVE)) )
/*N*/ 		{	// Append innerhalb von Append z.B. nicht
/*N*/ 			StartBlockModify( eMsgType, nStartAction );
/*N*/ 			EndBlockModify( nEndAction );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void lcl_EnsureSorting( StrCollection& rCollection ) // Changetracking.sdc
/*N*/ {
/*N*/ 	BOOL bSorted = TRUE;
/*N*/ 	USHORT nCount = rCollection.GetCount();
/*N*/ 	USHORT i;
/*N*/ 	for (i=0; i+1<nCount; i++)
/*N*/ 		if ( rCollection.Compare( rCollection[i], rCollection[i+1] ) != -1 )
/*N*/ 			bSorted = FALSE;
/*N*/ 
/*N*/ 	if ( !bSorted )
/*N*/ 	{
/*N*/ 		//	if not sorted, rebuild collection
/*N*/ 		StrCollection aNewColl;
/*N*/ 		for (i=0; i<nCount; i++)
/*N*/ 		{
/*N*/ 			DataObject* pNewObj = rCollection[i]->Clone();
/*N*/ 			if (!aNewColl.Insert(pNewObj))
/*N*/ 				delete pNewObj;
/*N*/ 		}
/*N*/ 		rCollection = aNewColl;
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScChangeTrack::Load( SvStream& rStrm, USHORT nVer ) // Changetracking.sdc
/*N*/ {
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	SetLoadSave( TRUE );
/*N*/ 
/*N*/ 	ScReadHeader aGlobalHdr( rStrm );
/*N*/ 
/*N*/ 	BYTE n8;
/*N*/ 	UINT16 n16;
/*N*/ 	UINT32 n32;
/*N*/ 
/*N*/ 	rStrm >> n16; nLoadedFileFormatVersion = n16;
/*N*/ 	if ( (nLoadedFileFormatVersion & 0xFF00) > (SC_CHGTRACK_FILEFORMAT & 0xFF00) )
/*N*/ 	{	// inkompatible neuere Version
/*N*/ 		Clear();
/*N*/ 		rStrm.SetError( SCWARN_IMPORT_INFOLOST );
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	aUserCollection.Load( rStrm );
/*N*/ 
/*N*/ 	ULONG nCount, nLastAction, nGeneratedCount;
/*N*/ 	rStrm >> n32; nCount = n32;
/*N*/ 	rStrm >> n32; nActionMax = n32;
/*N*/ 	rStrm >> n32; nLastAction = n32;
/*N*/ 
/*N*/ 	rStrm >> n32; nGeneratedCount = n32;
/*N*/ 
/*N*/ 	// GeneratedDelContents laden
/*N*/ 	{
/*N*/ 		ScMultipleReadHeader aHdr( rStrm );
/*N*/ 		for ( ULONG j = 0; j < nGeneratedCount && bOk; j++ )
/*N*/ 		{
/*N*/ 			ScChangeActionContent* pAct;
/*N*/ 
/*N*/ 			aHdr.StartEntry();
/*N*/ 
/*N*/ 			ScChangeActionType eType;
/*N*/ 			rStrm >> n8; eType = (ScChangeActionType) n8;
/*N*/ 
/*N*/ 			switch ( eType )
/*N*/ 			{
/*N*/ 				case SC_CAT_CONTENT :
/*N*/ 					pAct = new ScChangeActionContent( rStrm, aHdr, pDoc, nVer, this );
/*N*/ 				break;
/*N*/ 				default:
/*N*/ 					DBG_ERROR( "ScChangeTrack::Load: unknown GeneratedType" );
/*N*/ 					pAct = NULL;
/*N*/ 					bOk = FALSE;
/*N*/ 			}
/*N*/ 
/*N*/ 			aHdr.EndEntry();
/*N*/ 
/*N*/ 			if ( pAct )
/*N*/ 			{
/*N*/ 				pAct->SetType( eType );
/*N*/ 				if ( pFirstGeneratedDelContent )
/*N*/ 					pFirstGeneratedDelContent->pPrev = pAct;
/*N*/ 				pAct->pNext = pFirstGeneratedDelContent;
/*N*/ 				pFirstGeneratedDelContent = pAct;
/*N*/ 				aGeneratedTable.Insert( pAct->GetActionNumber(), pAct );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		rStrm >> n32; nGeneratedMin = n32;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bOk )
/*N*/ 		bOk = ( nGeneratedCount == aGeneratedTable.Count() );
/*N*/ 	DBG_ASSERT( bOk, "ScChangeTrack::Load: Generated failed" );
/*N*/ 
/*N*/ 
/*N*/ 	// erste Runde: Actions laden
/*N*/ 	{
/*N*/ 		ScMultipleReadHeader aHdr( rStrm );
/*N*/ 		for ( ULONG j = 0; j < nCount && bOk; j++ )
/*N*/ 		{
/*N*/ 			ScChangeAction* pAct;
/*N*/ 
/*N*/ 			aHdr.StartEntry();
/*N*/ 
/*N*/ 			USHORT nUserIndex;
/*N*/ 			rStrm >> n16; nUserIndex = n16;
/*N*/ 
/*N*/ 			ScChangeActionType eType;
/*N*/ 			rStrm >> n8; eType = (ScChangeActionType) n8;
/*N*/ 
/*N*/ 			switch ( eType )
/*N*/ 			{
/*N*/ 				case SC_CAT_INSERT_COLS :
/*N*/ 				case SC_CAT_INSERT_ROWS :
/*N*/ 				case SC_CAT_INSERT_TABS :
/*N*/ 					pAct = new ScChangeActionIns( rStrm, aHdr, this );
/*N*/ 				break;
/*N*/ 				case SC_CAT_DELETE_COLS :
/*N*/ 				case SC_CAT_DELETE_ROWS :
/*N*/ 				case SC_CAT_DELETE_TABS :
/*N*/ 					pAct = new ScChangeActionDel( rStrm, aHdr, pDoc, nVer, this );
/*N*/ 				break;
/*N*/ 				case SC_CAT_MOVE :
/*N*/ 					pAct = new ScChangeActionMove( rStrm, aHdr, this );
/*N*/ 				break;
/*N*/ 				case SC_CAT_CONTENT :
/*N*/ 					pAct = new ScChangeActionContent( rStrm, aHdr, pDoc, nVer, this );
/*N*/ 				break;
/*N*/ 				case SC_CAT_REJECT :
/*N*/ 					pAct = new ScChangeActionReject( rStrm, aHdr, this );
/*N*/ 				break;
/*N*/ 				default:
/*N*/ 					DBG_ERROR( "ScChangeTrack::Load: unknown ScChangeActionType" );
/*N*/ 					pAct = NULL;
/*N*/ 					bOk = FALSE;
/*N*/ 			}
/*N*/ 
/*N*/ 			aHdr.EndEntry();
/*N*/ 
/*N*/ 			if ( pAct )
/*N*/ 			{
/*N*/ 				pAct->SetType( eType );
/*N*/ 				if ( nUserIndex != 0xffff )
/*N*/ 				{
/*N*/ 					StrData* pUser = (StrData*) aUserCollection.At( nUserIndex );
/*N*/ 					if ( pUser )
/*N*/ 						pAct->SetUser( pUser->GetString() );
/*N*/ 				}
/*N*/ 				AppendLoaded( pAct );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pLast )
/*N*/ 		nMarkLastSaved = pLast->GetActionNumber();
/*N*/ 
/*N*/ 	if ( bOk )
/*N*/ 		bOk = ( nMarkLastSaved == nLastAction && nCount == aTable.Count() );
/*N*/ 	DBG_ASSERT( bOk, "ScChangeTrack::Load: failed" );
/*N*/ 
/*N*/ 	// zweite Runde: Links laden und alles verpointern
/*N*/ 	{
/*N*/ 		ScMultipleReadHeader aHdr( rStrm );
/*N*/ 		for ( ScChangeAction* p = GetFirst(); p && bOk; p = p->GetNext() )
/*N*/ 		{
/*N*/ 			aHdr.StartEntry();
/*N*/ 			bOk = p->LoadLinks( rStrm, this );
/*N*/ 			aHdr.EndEntry();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SetLoadSave( FALSE );
/*N*/ 
/*N*/ 	// versions between 583 and 633 had the sorting wrong -> correct (after loading the actions)
/*N*/ 	lcl_EnsureSorting( aUserCollection );
/*N*/ 
/*N*/ 	// den aktuellen User erst einfuegen, wenn die Actions bereits ihre User haben
/*N*/ 	SetUser( aUser );
/*N*/ 
/*N*/ 	if ( !bOk )
/*N*/ 	{
/*N*/ 		Clear();		// eindeutiger Zustand
/*N*/ 		rStrm.SetError( SCWARN_IMPORT_INFOLOST );
/*N*/ 	}
/*N*/ 
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScChangeTrack::Store( SvStream& rStrm )
/*N*/ {
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	SetLoadSave( TRUE );
/*N*/ 
/*N*/ 	ScWriteHeader aGlobalHdr( rStrm );
/*N*/ 
/*N*/ 	rStrm << (UINT16) SC_CHGTRACK_FILEFORMAT;
/*N*/ 
/*N*/ 	aUserCollection.Store( rStrm );
/*N*/ 
/*N*/ 	ULONG nCount = aTable.Count();
/*N*/ 	ULONG nLastAction = ( pLast ? pLast->GetActionNumber() : 0 );
/*N*/ 	ULONG nGeneratedCount = aGeneratedTable.Count();
/*N*/ 	rStrm << (UINT32) nCount << (UINT32) nActionMax << (UINT32) nLastAction;
/*N*/ 	rStrm << (UINT32) nGeneratedCount;
/*N*/ 
/*N*/ 	// GeneratedDelContents speichern
/*N*/ 	ULONG nSave = 0;
/*N*/ 	{
/*N*/ 		ScMultipleWriteHeader aHdr( rStrm );
/*N*/ 		ULONG nNewGeneratedMin = SC_CHGTRACK_GENERATED_START;
/*N*/ 		for ( ScChangeAction* p = pFirstGeneratedDelContent; p && bOk;
/*N*/ 				p = p->GetNext() )
/*N*/ 		{
/*N*/ 			++nSave;
/*N*/ 			aHdr.StartEntry();
/*N*/ 			rStrm << (BYTE) p->GetType();
/*N*/ 			bOk = p->Store( rStrm, aHdr );
/*N*/ 			aHdr.EndEntry();
/*N*/ 			ULONG nAct = p->GetActionNumber();
/*N*/ 			if ( nNewGeneratedMin > nAct )
/*N*/ 				nNewGeneratedMin = nAct;
/*N*/ 		}
/*N*/ 		nGeneratedMin = nNewGeneratedMin;	// evtl. unbenutzten Bereich freigeben
/*N*/ 		rStrm << (UINT32) nGeneratedMin;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bOk )
/*N*/ 		bOk = ( nGeneratedCount == nSave );
/*N*/ 	DBG_ASSERT( bOk, "ScChangeTrack::Store: failed" );
/*N*/ 
/*N*/ 	// erste Runde: Actions speichern
/*N*/ 	nSave = 0;
/*N*/ 	{
/*N*/ 		ScMultipleWriteHeader aHdr( rStrm );
/*N*/ 		StrData* pUserSearch = new StrData( aUser );
/*N*/ 		USHORT nUserIndex;
/*N*/ 		for ( ScChangeAction* p = GetFirst(); p && bOk; p = p->GetNext() )
/*N*/ 		{
/*N*/ 			++nSave;
/*N*/ 			aHdr.StartEntry();
/*N*/ 
/*N*/ 			pUserSearch->SetString( p->GetUser() );
/*N*/ 			if ( aUserCollection.Search( pUserSearch, nUserIndex ) )
/*N*/ 				rStrm << (UINT16) nUserIndex;
/*N*/ 			else
/*N*/ 				rStrm << (UINT16) 0xffff;
/*N*/ 			rStrm << (BYTE) p->GetType();
/*N*/ 
/*N*/ 			bOk = p->Store( rStrm, aHdr );
/*N*/ 
/*N*/ 			aHdr.EndEntry();
/*N*/ 		}
/*N*/ 		delete pUserSearch;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pLast )
/*N*/ 		nMarkLastSaved = pLast->GetActionNumber();
/*N*/ 
/*N*/ 	if ( bOk )
/*N*/ 		bOk = ( nCount == nSave );
/*N*/ 	DBG_ASSERT( bOk, "ScChangeTrack::Store: failed" );
/*N*/ 
/*N*/ 	// zweite Runde: Links speichern
/*N*/ 	{
/*N*/ 		ScMultipleWriteHeader aHdr( rStrm );
/*N*/ 		for ( ScChangeAction* p = GetFirst(); p && bOk; p = p->GetNext() )
/*N*/ 		{
/*N*/ 			aHdr.StartEntry();
/*N*/ 			bOk = p->StoreLinks( rStrm );
/*N*/ 			aHdr.EndEntry();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SetLoadSave( FALSE );
/*N*/ 	return bOk;
/*N*/ }


/*N*/ void ScChangeTrack::MasterLinks( ScChangeAction* pAppend )
/*N*/ {
/*N*/ 	ScChangeActionType eType = pAppend->GetType();
/*N*/ 
/*N*/ 	if ( eType == SC_CAT_CONTENT )
/*N*/ 	{
/*N*/ 		if ( !IsGenerated( pAppend->GetActionNumber() ) )
/*N*/ 		{
/*N*/ 			USHORT nSlot = ComputeContentSlot(
/*N*/ 				pAppend->GetBigRange().aStart.Row() );
/*N*/ 			((ScChangeActionContent*)pAppend)->InsertInSlot(
/*N*/ 				&ppContentSlots[nSlot] );
/*N*/ 		}
/*N*/ 		return ;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pAppend->IsRejecting() )
/*N*/ 		return ;		// Rejects haben keine Abhaengigkeiten
/*N*/ 
/*N*/ 	switch ( eType )
/*N*/ 	{
/*N*/ 		case SC_CAT_INSERT_COLS :
/*N*/ 		{
/*N*/ 			ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
/*N*/ 				&pLinkInsertCol, pAppend );
/*N*/ 			pAppend->AddLink( NULL, pLink );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case SC_CAT_INSERT_ROWS :
/*N*/ 		{
/*N*/ 			ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
/*N*/ 				&pLinkInsertRow, pAppend );
/*N*/ 			pAppend->AddLink( NULL, pLink );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case SC_CAT_INSERT_TABS :
/*N*/ 		{
/*N*/ 			ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
/*N*/ 				&pLinkInsertTab, pAppend );
/*N*/ 			pAppend->AddLink( NULL, pLink );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case SC_CAT_MOVE :
/*N*/ 		{
/*N*/ 			ScChangeActionLinkEntry* pLink = new ScChangeActionLinkEntry(
/*N*/ 				&pLinkMove, pAppend );
/*N*/ 			pAppend->AddLink( NULL, pLink );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeTrack::AppendLoaded( ScChangeAction* pAppend )
/*N*/ {
/*N*/ 	aTable.Insert( pAppend->GetActionNumber(), pAppend ); // Changetracking.sdc
/*N*/ 	if ( !pLast )
/*N*/ 		pFirst = pLast = pAppend;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pLast->pNext = pAppend;
/*N*/ 		pAppend->pPrev = pLast;
/*N*/ 		pLast = pAppend;
/*N*/ 	}
/*N*/ 	MasterLinks( pAppend );
/*N*/ }


/*N*/ void ScChangeTrack::Append( ScChangeAction* pAppend, ULONG nAction )
/*N*/ {
/*N*/ 	if ( nActionMax < nAction )
/*N*/ 		nActionMax = nAction;
/*N*/ 	pAppend->SetUser( aUser );
/*N*/ 	if ( bUseFixDateTime )
/*N*/ 		pAppend->SetDateTimeUTC( aFixDateTime );
/*N*/ 	pAppend->SetActionNumber( nAction );
/*N*/ 	aTable.Insert( nAction, pAppend );
/*N*/ 	// UpdateReference Inserts vor Dependencies.
/*N*/ 	// Delete rejectendes Insert hatte UpdateReference mit Delete-Undo.
/*N*/ 	// UpdateReference auch wenn pLast==NULL, weil pAppend ein Delete sein
/*N*/ 	// kann, dass DelContents generiert haben kann
/*N*/ 	if ( pAppend->IsInsertType() && !pAppend->IsRejecting() )
/*N*/ 		UpdateReference( pAppend, FALSE );
/*N*/ 	if ( !pLast )
/*N*/ 		pFirst = pLast = pAppend;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pLast->pNext = pAppend;
/*N*/ 		pAppend->pPrev = pLast;
/*N*/ 		pLast = pAppend;
/*N*/ 		Dependencies( pAppend );
/*N*/ 	}
/*N*/ 	// UpdateReference Inserts nicht nach Dependencies.
/*N*/ 	// Move rejectendes Move hatte UpdateReference mit Move-Undo, Inhalt in
/*N*/ 	// ToRange nicht deleten.
/*N*/ 	if ( !pAppend->IsInsertType() &&
/*N*/ 			!(pAppend->GetType() == SC_CAT_MOVE && pAppend->IsRejecting()) )
/*N*/ 		UpdateReference( pAppend, FALSE );
/*N*/ 	MasterLinks( pAppend );
/*N*/ 
/*N*/ 	if ( aModifiedLink.IsSet() )
/*N*/ 	{
/*N*/ 		NotifyModified( SC_CTM_APPEND, nAction, nAction );
/*N*/ 		if ( pAppend->GetType() == SC_CAT_CONTENT )
/*N*/ 		{
/*N*/ 			ScChangeActionContent* pContent = (ScChangeActionContent*) pAppend;
/*N*/ 			if ( pContent = pContent->GetPrevContent() )
/*N*/ 			{
/*N*/ 				ULONG nMod = pContent->GetActionNumber();
/*N*/ 				NotifyModified( SC_CTM_CHANGE, nMod, nMod );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			NotifyModified( SC_CTM_CHANGE, pFirst->GetActionNumber(),
/*N*/ 				pLast->GetActionNumber() );
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeTrack::Append( ScChangeAction* pAppend )
/*N*/ {
/*N*/ 	Append( pAppend, ++nActionMax );
/*N*/ }


/*N*/ void ScChangeTrack::AppendDeleteRange( const ScRange& rRange,
/*N*/ 		ScDocument* pRefDoc, short nDz, ULONG nRejectingInsert )
/*N*/ {
/*N*/ 	SetInDeleteRange( rRange );
/*N*/ 	StartBlockModify( SC_CTM_APPEND, GetActionMax() + 1 );
/*N*/ 	USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*N*/ 	rRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
/*N*/ 	for ( USHORT nTab = nTab1; nTab <= nTab2; nTab++ )
/*N*/ 	{
/*N*/ 		if ( !pRefDoc || nTab < pRefDoc->GetTableCount() )
/*N*/ 		{
/*N*/ 			if ( nCol1 == 0 && nCol2 == MAXCOL )
/*N*/ 			{	// ganze Zeilen und/oder Tabellen
/*N*/ 				if ( nRow1 == 0 && nRow2 == MAXROW )
/*N*/ 				{	// ganze Tabellen
/*N*/ //2do: geht nicht auch komplette Tabelle als ganzes?
/*N*/ 					ScRange aRange( 0, 0, nTab, 0, MAXROW, nTab );
/*N*/ 					for ( USHORT nCol = nCol1; nCol <= nCol2; nCol++ )
/*N*/ 					{	// spaltenweise ist weniger als zeilenweise
/*N*/ 						aRange.aStart.SetCol( nCol );
/*N*/ 						aRange.aEnd.SetCol( nCol );
/*N*/ 						if ( nCol == nCol2 )
/*N*/ 							SetInDeleteTop( TRUE );
/*N*/ 						AppendOneDeleteRange( aRange, pRefDoc, nCol-nCol1, 0,
/*N*/ 							nTab-nTab1 + nDz, nRejectingInsert );
/*N*/ 					}
/*N*/ 					//! immer noch InDeleteTop
/*N*/ 					AppendOneDeleteRange( rRange, pRefDoc, 0, 0,
/*N*/ 						nTab-nTab1 + nDz, nRejectingInsert );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{	// ganze Zeilen
/*N*/ 					ScRange aRange( 0, 0, nTab, MAXCOL, 0, nTab );
/*N*/ 					for ( USHORT nRow = nRow1; nRow <= nRow2; nRow++ )
/*N*/ 					{
/*N*/ 						aRange.aStart.SetRow( nRow );
/*N*/ 						aRange.aEnd.SetRow( nRow );
/*N*/ 						if ( nRow == nRow2 )
/*N*/ 							SetInDeleteTop( TRUE );
/*N*/ 						AppendOneDeleteRange( aRange, pRefDoc, 0, nRow-nRow1,
/*N*/ 							0, nRejectingInsert );
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( nRow1 == 0 && nRow2 == MAXROW )
/*N*/ 			{	// ganze Spalten
/*N*/ 				ScRange aRange( 0, 0, nTab, 0, MAXROW, nTab );
/*N*/ 				for ( USHORT nCol = nCol1; nCol <= nCol2; nCol++ )
/*N*/ 				{
/*N*/ 					aRange.aStart.SetCol( nCol );
/*N*/ 					aRange.aEnd.SetCol( nCol );
/*N*/ 					if ( nCol == nCol2 )
/*N*/ 						SetInDeleteTop( TRUE );
/*N*/ 					AppendOneDeleteRange( aRange, pRefDoc, nCol-nCol1, 0,
/*N*/ 						0, nRejectingInsert );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				DBG_ERROR( "ScChangeTrack::AppendDeleteRange: Block not supported!" );
/*N*/ 			SetInDeleteTop( FALSE );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	EndBlockModify( GetActionMax() );
/*N*/ }


/*N*/ void ScChangeTrack::AppendOneDeleteRange( const ScRange& rOrgRange,
/*N*/ 		ScDocument* pRefDoc, short nDx, short nDy, short nDz,
/*N*/ 		ULONG nRejectingInsert )
/*N*/ {
/*N*/ 	ScRange aTrackRange( rOrgRange );
/*N*/ 	if ( nDx )
/*N*/ 	{
/*N*/ 		aTrackRange.aStart.IncCol( -nDx );
/*N*/ 		aTrackRange.aEnd.IncCol( -nDx );
/*N*/ 	}
/*N*/ 	if ( nDy )
/*N*/ 	{
/*N*/ 		aTrackRange.aStart.IncRow( -nDy );
/*N*/ 		aTrackRange.aEnd.IncRow( -nDy );
/*N*/ 	}
/*N*/ 	if ( nDz )
/*N*/ 	{
/*N*/ 		aTrackRange.aStart.IncTab( -nDz );
/*N*/ 		aTrackRange.aEnd.IncTab( -nDz );
/*N*/ 	}
/*N*/ 	ScChangeActionDel* pAct = new ScChangeActionDel( aTrackRange, nDx, nDy,
/*N*/ 		this );
/*N*/ 	// TabDelete keine Contents, sind in einzelnen Spalten
/*N*/ 	if ( !(rOrgRange.aStart.Col() == 0 && rOrgRange.aStart.Row() == 0 &&
/*N*/ 			rOrgRange.aEnd.Col() == MAXCOL && rOrgRange.aEnd.Row() == MAXROW) )
/*N*/ 		LookUpContents( rOrgRange, pRefDoc, -nDx, -nDy, -nDz );
/*N*/ 	if ( nRejectingInsert )
/*N*/ 	{
/*N*/ 		pAct->SetRejectAction( nRejectingInsert );
/*N*/ 		pAct->SetState( SC_CAS_ACCEPTED );
/*N*/ 	}
/*N*/ 	Append( pAct );
/*N*/ }


/*N*/ void ScChangeTrack::LookUpContents( const ScRange& rOrgRange,
/*N*/ 		ScDocument* pRefDoc, short nDx, short nDy, short nDz )
/*N*/ {
/*N*/ 	if ( pRefDoc )
/*N*/ 	{
/*N*/ 		ScAddress aPos;
/*N*/ 		ScBigAddress aBigPos;
/*N*/ 		ScCellIterator aIter( pRefDoc, rOrgRange );
/*N*/ 		ScBaseCell* pCell = aIter.GetFirst();
/*N*/ 		while ( pCell )
/*N*/ 		{
/*N*/ 			if ( ScChangeActionContent::GetContentCellType( pCell ) )
/*N*/ 			{
/*N*/ 				aBigPos.Set( aIter.GetCol() + nDx, aIter.GetRow() + nDy,
/*N*/ 					aIter.GetTab() + nDz );
/*N*/ 				ScChangeActionContent* pContent = SearchContentAt( aBigPos, NULL );
/*N*/ 				if ( !pContent )
/*N*/ 				{	// nicht getrackte Contents
/*N*/ 					aPos.Set( aIter.GetCol() + nDx, aIter.GetRow() + nDy,
/*N*/ 						aIter.GetTab() + nDz );
/*N*/ 					GenerateDelContent( aPos, pCell, pRefDoc );
/*N*/ 					//! der Content wird hier _nicht_ per AddContent hinzugefuegt,
/*N*/ 					//! sondern in UpdateReference, um z.B. auch kreuzende Deletes
/*N*/ 					//! korrekt zu erfassen
/*N*/ 				}
/*N*/ 			}
/*N*/ 			pCell = aIter.GetNext();
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeTrack::DeleteCellEntries( ScChangeActionCellListEntry*& pCellList,
/*N*/ 		ScChangeAction* pDeletor )
/*N*/ {
/*N*/ 	ScChangeActionCellListEntry* pE = pCellList;
/*N*/ 	while ( pE )
/*N*/ 	{
/*N*/ 		ScChangeActionCellListEntry* pNext = pE->pNext;
/*N*/ 		pE->pContent->RemoveDeletedIn( pDeletor );
/*N*/ 		if ( IsGenerated( pE->pContent->GetActionNumber() ) &&
/*N*/ 				!pE->pContent->IsDeletedIn() )
/*N*/ 			DeleteGeneratedDelContent( pE->pContent );
/*N*/ 		delete pE;
/*N*/ 		pE = pNext;
/*N*/ 	}
/*N*/ 	pCellList = NULL;
/*N*/ }


/*N*/ ScChangeActionContent* ScChangeTrack::GenerateDelContent(
/*N*/ 		const ScAddress& rPos, const ScBaseCell* pCell,
/*N*/ 		const ScDocument* pFromDoc )
/*N*/ {
/*N*/ 	ScChangeActionContent* pContent = new ScChangeActionContent(
/*N*/ 		ScRange( rPos ) );
/*N*/ 	pContent->SetActionNumber( --nGeneratedMin );
/*N*/ 	// nur NewValue
/*N*/ 	ScChangeActionContent::SetValue( pContent->aNewValue, pContent->pNewCell,
/*N*/ 		rPos, pCell, pFromDoc, pDoc );
/*N*/ 	// pNextContent und pPrevContent werden nicht gesetzt
/*N*/ 	if ( pFirstGeneratedDelContent )
/*N*/ 	{	// vorne reinhaengen
/*N*/ 		pFirstGeneratedDelContent->pPrev = pContent;
/*N*/ 		pContent->pNext = pFirstGeneratedDelContent;
/*N*/ 	}
/*N*/ 	pFirstGeneratedDelContent = pContent;
/*N*/ 	aGeneratedTable.Insert( nGeneratedMin, pContent );
/*N*/ 	NotifyModified( SC_CTM_APPEND, nGeneratedMin, nGeneratedMin );
/*N*/ 	return pContent;
/*N*/ }


/*N*/ void ScChangeTrack::DeleteGeneratedDelContent( ScChangeActionContent* pContent )
/*N*/ {
/*N*/ 	ULONG nAct = pContent->GetActionNumber();
/*N*/ 	aGeneratedTable.Remove( nAct );
/*N*/ 	if ( pFirstGeneratedDelContent == pContent )
/*N*/ 		pFirstGeneratedDelContent = (ScChangeActionContent*) pContent->pNext;
/*N*/ 	if ( pContent->pNext )
/*N*/ 		pContent->pNext->pPrev = pContent->pPrev;
/*N*/ 	if ( pContent->pPrev )
/*N*/ 		pContent->pPrev->pNext = pContent->pNext;
/*N*/ 	delete pContent;
/*N*/ 	NotifyModified( SC_CTM_REMOVE, nAct, nAct );
/*N*/ 	if ( nAct == nGeneratedMin )
/*N*/ 		++nGeneratedMin;		//! erst nach NotifyModified wg. IsGenerated
/*N*/ }

/*N*/ ScChangeActionContent* ScChangeTrack::SearchContentAt(
/*N*/          const ScBigAddress& rPos, ScChangeAction* pButNotThis ) const
/*N*/ {
/*N*/  USHORT nSlot = ComputeContentSlot( rPos.Row() );
/*N*/  for ( ScChangeActionContent* p = ppContentSlots[nSlot]; p;
/*N*/                  p = p->GetNextInSlot() )
/*N*/  {
/*N*/          if ( p != pButNotThis && !p->IsDeletedIn() &&
/*N*/                          p->GetBigRange().aStart == rPos )
/*N*/          {
/*N*/                  ScChangeActionContent* pContent = p->GetTopContent();
/*N*/                  if ( !pContent->IsDeletedIn() )
/*N*/                          return pContent;
/*N*/          }
/*N*/  }
/*N*/  return NULL;
/*N*/ }


/*N*/ void ScChangeTrack::AddDependentWithNotify( ScChangeAction* pParent,
/*N*/ 		ScChangeAction* pDependent )
/*N*/ {
/*N*/ 	ScChangeActionLinkEntry* pLink = pParent->AddDependent( pDependent );
/*N*/ 	pDependent->AddLink( pParent, pLink );
/*N*/ 	if ( aModifiedLink.IsSet() )
/*N*/ 	{
/*N*/ 		ULONG nMod = pParent->GetActionNumber();
/*N*/ 		NotifyModified( SC_CTM_PARENT, nMod, nMod );
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeTrack::Dependencies( ScChangeAction* pAct )
/*N*/ {
/*N*/ 	// Finde die letzte Abhaengigkeit fuer jeweils Col/Row/Tab.
/*N*/ 	// Content an gleicher Position verketten.
/*N*/ 	// Move Abhaengigkeiten.
/*N*/ 	ScChangeActionType eActType = pAct->GetType();
/*N*/ 	if ( eActType == SC_CAT_REJECT ||
/*N*/ 			(eActType == SC_CAT_MOVE && pAct->IsRejecting()) )
/*N*/ 		return ;		// diese Rejects sind nicht abhaengig
/*N*/ 
/*N*/ 	if ( eActType == SC_CAT_CONTENT )
/*N*/ 	{
/*N*/ 		if ( !(((ScChangeActionContent*)pAct)->GetNextContent() ||
/*N*/ 			((ScChangeActionContent*)pAct)->GetPrevContent()) )
/*N*/ 		{	// Contents an gleicher Position verketten
/*N*/ 			ScChangeActionContent* pContent = SearchContentAt(
/*N*/ 				pAct->GetBigRange().aStart, pAct );
/*N*/ 			if ( pContent )
/*N*/ 			{
/*N*/ 				pContent->SetNextContent( (ScChangeActionContent*) pAct );
/*N*/ 				((ScChangeActionContent*)pAct)->SetPrevContent( pContent );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		const ScBaseCell* pCell = ((ScChangeActionContent*)pAct)->GetNewCell();
/*N*/ 		if ( ScChangeActionContent::GetContentCellType( pCell ) == SC_CACCT_MATREF )
/*N*/ 		{
/*N*/ 			ScAddress aOrg;
/*N*/ 			((const ScFormulaCell*)pCell)->GetMatrixOrigin( aOrg );
/*N*/ 			ScChangeActionContent* pContent = SearchContentAt( aOrg, pAct );
/*N*/ 			if ( pContent && pContent->IsMatrixOrigin() )
/*N*/ 			{
/*N*/ 				AddDependentWithNotify( pContent, pAct );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				DBG_ERRORFILE( "ScChangeTrack::Dependencies: MatOrg not found" );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !(pLinkInsertCol || pLinkInsertRow || pLinkInsertTab || pLinkMove) )
/*N*/ 		return ;		// keine Dependencies
/*N*/ 	if ( pAct->IsRejecting() )
/*N*/ 		return ;		// ausser Content keine Dependencies
/*N*/ 
/*N*/ 	// Insert in einem entsprechenden Insert haengt davon ab, sonst muesste
/*N*/ 	// der vorherige Insert gesplittet werden.
/*N*/ 	// Sich kreuzende Inserts und Deletes sind nicht abhaengig.
/*N*/ 	// Alles andere ist abhaengig.
/*N*/ 
/*N*/ 	// Der zuletzt eingelinkte Insert steht am Anfang einer Kette,
/*N*/ 	// also genau richtig
/*N*/ 
/*N*/ 	const ScBigRange& rRange = pAct->GetBigRange();
/*N*/ 	BOOL bActNoInsert = !pAct->IsInsertType();
/*N*/ 	BOOL bActColDel = ( eActType == SC_CAT_DELETE_COLS );
/*N*/ 	BOOL bActRowDel = ( eActType == SC_CAT_DELETE_ROWS );
/*N*/ 	BOOL bActTabDel = ( eActType == SC_CAT_DELETE_TABS );
/*N*/ 
/*N*/ 	if ( pLinkInsertCol && (eActType == SC_CAT_INSERT_COLS ||
/*N*/ 			(bActNoInsert && !bActRowDel && !bActTabDel)) )
/*N*/ 	{
/*N*/ 		for ( ScChangeActionLinkEntry* pL = pLinkInsertCol; pL; pL = pL->GetNext() )
/*N*/ 		{
/*N*/ 			ScChangeActionIns* pTest = (ScChangeActionIns*) pL->GetAction();
/*N*/ 			if ( !pTest->IsRejected() &&
/*N*/ 					pTest->GetBigRange().Intersects( rRange ) )
/*N*/ 			{
/*N*/ 				AddDependentWithNotify( pTest, pAct );
/*N*/ 				break;	// for
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( pLinkInsertRow && (eActType == SC_CAT_INSERT_ROWS ||
/*N*/ 			(bActNoInsert && !bActColDel && !bActTabDel)) )
/*N*/ 	{
/*N*/ 		for ( ScChangeActionLinkEntry* pL = pLinkInsertRow; pL; pL = pL->GetNext() )
/*N*/ 		{
/*N*/ 			ScChangeActionIns* pTest = (ScChangeActionIns*) pL->GetAction();
/*N*/ 			if ( !pTest->IsRejected() &&
/*N*/ 					pTest->GetBigRange().Intersects( rRange ) )
/*N*/ 			{
/*N*/ 				AddDependentWithNotify( pTest, pAct );
/*N*/ 				break;	// for
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( pLinkInsertTab && (eActType == SC_CAT_INSERT_TABS ||
/*N*/ 			(bActNoInsert && !bActColDel &&  !bActRowDel)) )
/*N*/ 	{
/*N*/ 		for ( ScChangeActionLinkEntry* pL = pLinkInsertTab; pL; pL = pL->GetNext() )
/*N*/ 		{
/*N*/ 			ScChangeActionIns* pTest = (ScChangeActionIns*) pL->GetAction();
/*N*/ 			if ( !pTest->IsRejected() &&
/*N*/ 					pTest->GetBigRange().Intersects( rRange ) )
/*N*/ 			{
/*N*/ 				AddDependentWithNotify( pTest, pAct );
/*N*/ 				break;	// for
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pLinkMove )
/*N*/ 	{
/*N*/ 		if ( eActType == SC_CAT_CONTENT )
/*N*/ 		{	// Content ist von FromRange abhaengig
/*N*/ 			const ScBigAddress& rPos = rRange.aStart;
/*N*/ 			for ( ScChangeActionLinkEntry* pL = pLinkMove; pL; pL = pL->GetNext() )
/*N*/ 			{
/*N*/ 				ScChangeActionMove* pTest = (ScChangeActionMove*) pL->GetAction();
/*N*/ 				if ( !pTest->IsRejected() &&
/*N*/ 						pTest->GetFromRange().In( rPos ) )
/*N*/ 				{
/*N*/ 					AddDependentWithNotify( pTest, pAct );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if ( eActType == SC_CAT_MOVE )
/*N*/ 		{	// Move FromRange ist von ToRange abhaengig
/*N*/ 			const ScBigRange& rFromRange = ((ScChangeActionMove*)pAct)->GetFromRange();
/*N*/ 			for ( ScChangeActionLinkEntry* pL = pLinkMove; pL; pL = pL->GetNext() )
/*N*/ 			{
/*N*/ 				ScChangeActionMove* pTest = (ScChangeActionMove*) pL->GetAction();
/*N*/ 				if ( !pTest->IsRejected() &&
/*N*/ 						pTest->GetBigRange().Intersects( rFromRange ) )
/*N*/ 				{
/*N*/ 					AddDependentWithNotify( pTest, pAct );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{	// Inserts und Deletes sind abhaengig, sobald sie FromRange oder
/*N*/ 			// ToRange kreuzen
/*N*/ 			for ( ScChangeActionLinkEntry* pL = pLinkMove; pL; pL = pL->GetNext() )
/*N*/ 			{
/*N*/ 				ScChangeActionMove* pTest = (ScChangeActionMove*) pL->GetAction();
/*N*/ 				if ( !pTest->IsRejected() &&
/*N*/ 						(pTest->GetFromRange().Intersects( rRange ) ||
/*N*/ 						pTest->GetBigRange().Intersects( rRange )) )
/*N*/ 				{
/*N*/ 					AddDependentWithNotify( pTest, pAct );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeTrack::Remove( ScChangeAction* pRemove )
/*N*/ {
/*N*/ 	// aus Track ausklinken
/*N*/ 	ULONG nAct = pRemove->GetActionNumber();
/*N*/ 	aTable.Remove( nAct );
/*N*/ 	if ( nAct == nActionMax )
/*N*/ 		--nActionMax;
/*N*/ 	if ( pRemove == pLast )
/*N*/ 		pLast = pRemove->pPrev;
/*N*/ 	if ( pRemove == pFirst )
/*N*/ 		pFirst = pRemove->pNext;
/*N*/ 	if ( nAct == nMarkLastSaved )
/*N*/ 		nMarkLastSaved =
/*N*/ 			( pRemove->pPrev ? pRemove->pPrev->GetActionNumber() : 0 );
/*N*/ 
/*N*/ 	// aus der globalen Kette ausklinken
/*N*/ 	if ( pRemove->pNext )
/*N*/ 		pRemove->pNext->pPrev = pRemove->pPrev;
/*N*/ 	if ( pRemove->pPrev )
/*N*/ 		pRemove->pPrev->pNext = pRemove->pNext;
/*N*/ 
/*N*/ 	// Dependencies nicht loeschen, passiert on delete automatisch durch
/*N*/ 	// LinkEntry, ohne Listen abzuklappern
/*N*/ 
/*N*/ 	if ( aModifiedLink.IsSet() )
/*N*/ 	{
/*N*/ 		NotifyModified( SC_CTM_REMOVE, nAct, nAct );
/*N*/ 		if ( pRemove->GetType() == SC_CAT_CONTENT )
/*N*/ 		{
/*N*/ 			ScChangeActionContent* pContent = (ScChangeActionContent*) pRemove;
/*N*/ 			if ( pContent = pContent->GetPrevContent() )
/*N*/ 			{
/*N*/ 				ULONG nMod = pContent->GetActionNumber();
/*N*/ 				NotifyModified( SC_CTM_CHANGE, nMod, nMod );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if ( pLast )
/*N*/ 			NotifyModified( SC_CTM_CHANGE, pFirst->GetActionNumber(),
/*N*/ 				pLast->GetActionNumber() );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( IsInPasteCut() && pRemove->GetType() == SC_CAT_CONTENT )
/*N*/ 	{	//! Content wird wiederverwertet
/*N*/ 		ScChangeActionContent* pContent = (ScChangeActionContent*) pRemove;
/*N*/ 		pContent->RemoveAllLinks();
/*N*/ 		pContent->ClearTrack();
/*N*/ 		pContent->pNext = pContent->pPrev = NULL;
/*N*/ 		pContent->pNextContent = pContent->pPrevContent = NULL;
/*N*/ 	}
/*N*/ }


/*N*/ void ScChangeTrack::UpdateReference( ScChangeAction* pAct, BOOL bUndo )
/*N*/ {
/*N*/ 	ScChangeActionType eActType = pAct->GetType();
/*N*/ 	if ( eActType == SC_CAT_CONTENT || eActType == SC_CAT_REJECT )
/*N*/ 		return ;
/*N*/ 
/*N*/ 	//! Formelzellen haengen nicht im Dokument
/*N*/ 	BOOL bOldAutoCalc = pDoc->GetAutoCalc();
/*N*/ 	pDoc->SetAutoCalc( FALSE );
/*N*/ 	BOOL bOldNoListening = pDoc->GetNoListening();
/*N*/ 	pDoc->SetNoListening( TRUE );
/*N*/ 	//! Formelzellen ExpandRefs synchronisiert zu denen im Dokument
/*N*/ 	BOOL bOldExpandRefs = pDoc->IsExpandRefs();
/*N*/ 	if ( (!bUndo && pAct->IsInsertType()) || (bUndo && pAct->IsDeleteType()) )
/*N*/ 		pDoc->SetExpandRefs( SC_MOD()->GetInputOptions().GetExpandRefs() );
/*N*/ 
/*N*/ 	if ( pAct->IsDeleteType() )
/*N*/ 	{
/*N*/ 		SetInDeleteUndo( bUndo );
/*N*/ 		SetInDelete( TRUE );
/*N*/ 	}
/*N*/ 	else if ( GetMergeState() == SC_CTMS_OWN )
/*N*/ 	{
/*N*/ 		// Referenzen von Formelzellen wiederherstellen,
/*N*/ 		// vorheriges MergePrepare war bei einem Insert wie ein Delete
/*N*/ 		if ( pAct->IsInsertType() )
/*N*/ 			SetInDeleteUndo( TRUE );
/*N*/ 	}
/*N*/ 
/*N*/ 	//! erst die generated, als waeren sie vorher getrackt worden
/*N*/ 	if ( pFirstGeneratedDelContent )
/*N*/ 		UpdateReference( (ScChangeAction**)&pFirstGeneratedDelContent, pAct,
/*N*/ 			bUndo );
/*N*/ 	UpdateReference( &pFirst, pAct, bUndo );
/*N*/ 
/*N*/ 	SetInDelete( FALSE );
/*N*/ 	SetInDeleteUndo( FALSE );
/*N*/ 
/*N*/ 	pDoc->SetExpandRefs( bOldExpandRefs );
/*N*/ 	pDoc->SetNoListening( bOldNoListening );
/*N*/ 	pDoc->SetAutoCalc( bOldAutoCalc );
/*N*/ }


/*N*/ void ScChangeTrack::UpdateReference( ScChangeAction** ppFirstAction,
/*N*/ 		ScChangeAction* pAct, BOOL bUndo )
/*N*/ {
/*N*/ 	ScChangeActionType eActType = pAct->GetType();
/*N*/ 	BOOL bGeneratedDelContents =
/*N*/ 		( ppFirstAction == (ScChangeAction**)&pFirstGeneratedDelContent );
/*N*/ 	const ScBigRange& rOrgRange = pAct->GetBigRange();
/*N*/ 	ScBigRange aRange( rOrgRange );
/*N*/ 	ScBigRange aDelRange( rOrgRange );
/*N*/ 	INT32 nDx, nDy, nDz;
/*N*/ 	nDx = nDy = nDz = 0;
/*N*/ 	UpdateRefMode eMode = URM_INSDEL;
/*N*/ 	BOOL bDel = FALSE;
/*N*/ 	switch ( eActType )
/*N*/ 	{
/*N*/ 		case SC_CAT_INSERT_COLS :
/*N*/ 			aRange.aEnd.SetCol( nInt32Max );
/*N*/ 			nDx = rOrgRange.aEnd.Col() - rOrgRange.aStart.Col() + 1;
/*N*/ 		break;
/*N*/ 		case SC_CAT_INSERT_ROWS :
/*N*/ 			aRange.aEnd.SetRow( nInt32Max );
/*N*/ 			nDy = rOrgRange.aEnd.Row() - rOrgRange.aStart.Row() + 1;
/*N*/ 		break;
/*N*/ 		case SC_CAT_INSERT_TABS :
/*N*/ 			aRange.aEnd.SetTab( nInt32Max );
/*N*/ 			nDz = rOrgRange.aEnd.Tab() - rOrgRange.aStart.Tab() + 1;
/*N*/ 		break;
/*N*/ 		case SC_CAT_DELETE_COLS :
/*N*/ 			aRange.aEnd.SetCol( nInt32Max );
/*N*/ 			nDx = -(rOrgRange.aEnd.Col() - rOrgRange.aStart.Col() + 1);
/*N*/ 			aDelRange.aEnd.SetCol( aDelRange.aStart.Col() - nDx - 1 );
/*N*/ 			bDel = TRUE;
/*N*/ 		break;
/*N*/ 		case SC_CAT_DELETE_ROWS :
/*N*/ 			aRange.aEnd.SetRow( nInt32Max );
/*N*/ 			nDy = -(rOrgRange.aEnd.Row() - rOrgRange.aStart.Row() + 1);
/*N*/ 			aDelRange.aEnd.SetRow( aDelRange.aStart.Row() - nDy - 1 );
/*N*/ 			bDel = TRUE;
/*N*/ 		break;
/*N*/ 		case SC_CAT_DELETE_TABS :
/*N*/ 			aRange.aEnd.SetTab( nInt32Max );
/*N*/ 			nDz = -(rOrgRange.aEnd.Tab() - rOrgRange.aStart.Tab() + 1);
/*N*/ 			aDelRange.aEnd.SetTab( aDelRange.aStart.Tab() - nDz - 1 );
/*N*/ 			bDel = TRUE;
/*N*/ 		break;
/*N*/ 		case SC_CAT_MOVE :
/*N*/ 			eMode = URM_MOVE;
/*N*/ 			((ScChangeActionMove*)pAct)->GetDelta( nDx, nDy, nDz );
/*N*/ 		break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR( "ScChangeTrack::UpdateReference: unknown Type" );
/*N*/ 	}
/*N*/ 	if ( bUndo )
/*N*/ 	{
/*N*/ 		nDx = -nDx;
/*N*/ 		nDy = -nDy;
/*N*/ 		nDz = -nDz;
/*N*/ 	}
/*N*/ 	if ( bDel )
/*N*/ 	{	//! fuer diesen Mechanismus gilt:
/*N*/ 		//! es gibt nur ganze, einfache geloeschte Spalten/Zeilen
/*N*/ 		ScChangeActionDel* pActDel = (ScChangeActionDel*) pAct;
/*N*/ 		if ( !bUndo )
/*N*/ 		{	// Delete
/*N*/ 			ScChangeActionType eInsType;		// fuer Insert-Undo-"Deletes"
/*N*/ 			switch ( eActType )
/*N*/ 			{
/*N*/ 				case SC_CAT_DELETE_COLS :
/*N*/ 					eInsType = SC_CAT_INSERT_COLS;
/*N*/ 				break;
/*N*/ 				case SC_CAT_DELETE_ROWS :
/*N*/ 					eInsType = SC_CAT_INSERT_ROWS;
/*N*/ 				break;
/*N*/ 				case SC_CAT_DELETE_TABS :
/*N*/ 					eInsType = SC_CAT_INSERT_TABS;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
/*N*/ 			{
/*N*/ 				if ( p == pAct )
/*N*/ 					continue;	// for
/*N*/ 				BOOL bUpdate = TRUE;
/*N*/ 				if ( GetMergeState() == SC_CTMS_OTHER &&
/*N*/ 						p->GetActionNumber() <= GetLastMerge() )
/*N*/ 				{	// Delete in mergendem Dokument, Action im zu mergenden
/*N*/ 					if ( p->IsInsertType() )
/*N*/ 					{
/*N*/ 						// Bei Insert Referenzen nur anpassen, wenn das Delete
/*N*/ 						// das Insert nicht schneidet.
/*N*/ 						if ( !aDelRange.Intersects( p->GetBigRange() ) )
/*N*/ 							p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
/*N*/ 						bUpdate = FALSE;
/*N*/ 					}
/*N*/ 					else if ( p->GetType() == SC_CAT_CONTENT &&
/*N*/ 							p->IsDeletedInDelType( eInsType ) )
/*N*/ 					{	// Content in Insert-Undo-"Delete"
/*N*/ 						// Nicht anpassen, wenn dieses Delete in dem
/*N*/ 						// Insert-"Delete" sein wuerde (ist nur verschoben).
/*N*/ 						if ( aDelRange.In( p->GetBigRange().aStart ) )
/*N*/ 							bUpdate = FALSE;
/*N*/ 						else
/*N*/ 						{
/*N*/ 							const ScChangeActionLinkEntry* pLink = p->GetDeletedIn();
/*N*/ 							while ( pLink && bUpdate )
/*N*/ 							{
/*N*/ 								const ScChangeAction* pDel = pLink->GetAction();
/*N*/ 								if ( pDel && pDel->GetType() == eInsType &&
/*N*/ 										pDel->GetBigRange().In( aDelRange ) )
/*N*/ 									bUpdate = FALSE;
/*N*/ 								pLink = pLink->GetNext();
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 					if ( !bUpdate )
/*N*/ 						continue;	// for
/*N*/ 				}
/*N*/ 				if ( aDelRange.In( p->GetBigRange() ) )
/*N*/ 				{
/*N*/ 					// Innerhalb eines gerade geloeschten Bereiches nicht
/*N*/ 					// anpassen, stattdessen dem Bereich zuordnen.
/*N*/ 					// Mehrfache geloeschte Bereiche "stapeln".
/*N*/ 					// Kreuzende Deletes setzen mehrfach geloescht.
/*N*/ 					if ( !p->IsDeletedInDelType( eActType ) )
/*N*/ 					{
/*N*/ 						p->SetDeletedIn( pActDel );
/*N*/ 						// GeneratedDelContent in zu loeschende Liste aufnehmen
/*N*/ 						if ( bGeneratedDelContents )
/*N*/ 							pActDel->AddContent( (ScChangeActionContent*) p );
/*N*/ 					}
/*N*/ 					bUpdate = FALSE;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// Eingefuegte Bereiche abschneiden, wenn Start/End im
/*N*/ 					// Delete liegt, aber das Insert nicht komplett innerhalb
/*N*/ 					// des Delete liegt bzw. das Delete nicht komplett im
/*N*/ 					// Insert. Das Delete merkt sich, welchem Insert es was
/*N*/ 					// abgeschnitten hat, es kann auch nur ein einziges Insert
/*N*/ 					// sein (weil Delete einspaltig/einzeilig ist).
/*N*/ 					// Abgeschnittene Moves kann es viele geben.
/*N*/ 					//! Ein Delete ist immer einspaltig/einzeilig, deswegen 1
/*N*/ 					//! ohne die Ueberlappung auszurechnen.
/*N*/ 					switch ( p->GetType() )
/*N*/ 					{
/*N*/ 						case SC_CAT_INSERT_COLS :
/*N*/ 							if ( eActType == SC_CAT_DELETE_COLS )
/*N*/ 							{
/*N*/ 								if ( aDelRange.In( p->GetBigRange().aStart ) )
/*N*/ 								{
/*N*/ 									pActDel->SetCutOffInsert(
/*N*/ 										(ScChangeActionIns*) p, 1 );
/*N*/ 									p->GetBigRange().aStart.IncCol( 1 );
/*N*/ 								}
/*N*/ 								else if ( aDelRange.In( p->GetBigRange().aEnd ) )
/*N*/ 								{
/*N*/ 									pActDel->SetCutOffInsert(
/*N*/ 										(ScChangeActionIns*) p, -1 );
/*N*/ 									p->GetBigRange().aEnd.IncCol( -1 );
/*N*/ 								}
/*N*/ 							}
/*N*/ 						break;
/*N*/ 						case SC_CAT_INSERT_ROWS :
/*N*/ 							if ( eActType == SC_CAT_DELETE_ROWS )
/*N*/ 							{
/*N*/ 								if ( aDelRange.In( p->GetBigRange().aStart ) )
/*N*/ 								{
/*N*/ 									pActDel->SetCutOffInsert(
/*N*/ 										(ScChangeActionIns*) p, 1 );
/*N*/ 									p->GetBigRange().aStart.IncRow( 1 );
/*N*/ 								}
/*N*/ 								else if ( aDelRange.In( p->GetBigRange().aEnd ) )
/*N*/ 								{
/*N*/ 									pActDel->SetCutOffInsert(
/*N*/ 										(ScChangeActionIns*) p, -1 );
/*N*/ 									p->GetBigRange().aEnd.IncRow( -1 );
/*N*/ 								}
/*N*/ 							}
/*N*/ 						break;
/*N*/ 						case SC_CAT_INSERT_TABS :
/*N*/ 							if ( eActType == SC_CAT_DELETE_TABS )
/*N*/ 							{
/*N*/ 								if ( aDelRange.In( p->GetBigRange().aStart ) )
/*N*/ 								{
/*N*/ 									pActDel->SetCutOffInsert(
/*N*/ 										(ScChangeActionIns*) p, 1 );
/*N*/ 									p->GetBigRange().aStart.IncTab( 1 );
/*N*/ 								}
/*N*/ 								else if ( aDelRange.In( p->GetBigRange().aEnd ) )
/*N*/ 								{
/*N*/ 									pActDel->SetCutOffInsert(
/*N*/ 										(ScChangeActionIns*) p, -1 );
/*N*/ 									p->GetBigRange().aEnd.IncTab( -1 );
/*N*/ 								}
/*N*/ 							}
/*N*/ 						break;
/*N*/ 						case SC_CAT_MOVE :
/*N*/ 						{
/*N*/ 							ScChangeActionMove* pMove = (ScChangeActionMove*) p;
/*N*/ 							short nFrom = 0;
/*N*/ 							short nTo = 0;
/*N*/ 							if ( aDelRange.In( pMove->GetBigRange().aStart ) )
/*N*/ 								nTo = 1;
/*N*/ 							else if ( aDelRange.In( pMove->GetBigRange().aEnd ) )
/*N*/ 								nTo = -1;
/*N*/ 							if ( aDelRange.In( pMove->GetFromRange().aStart ) )
/*N*/ 								nFrom = 1;
/*N*/ 							else if ( aDelRange.In( pMove->GetFromRange().aEnd ) )
/*N*/ 								nFrom = -1;
/*N*/ 							if ( nFrom )
/*N*/ 							{
/*N*/ 								switch ( eActType )
/*N*/ 								{
/*N*/ 									case SC_CAT_DELETE_COLS :
/*N*/ 										if ( nFrom > 0 )
/*N*/ 											pMove->GetFromRange().aStart.IncCol( nFrom );
/*N*/ 										else
/*N*/ 											pMove->GetFromRange().aEnd.IncCol( nFrom );
/*N*/ 									break;
/*N*/ 									case SC_CAT_DELETE_ROWS :
/*N*/ 										if ( nFrom > 0 )
/*N*/ 											pMove->GetFromRange().aStart.IncRow( nFrom );
/*N*/ 										else
/*N*/ 											pMove->GetFromRange().aEnd.IncRow( nFrom );
/*N*/ 									break;
/*N*/ 									case SC_CAT_DELETE_TABS :
/*N*/ 										if ( nFrom > 0 )
/*N*/ 											pMove->GetFromRange().aStart.IncTab( nFrom );
/*N*/ 										else
/*N*/ 											pMove->GetFromRange().aEnd.IncTab( nFrom );
/*N*/ 									break;
/*N*/ 								}
/*N*/ 							}
/*N*/ 							if ( nTo )
/*N*/ 							{
/*N*/ 								switch ( eActType )
/*N*/ 								{
/*N*/ 									case SC_CAT_DELETE_COLS :
/*N*/ 										if ( nTo > 0 )
/*N*/ 											pMove->GetBigRange().aStart.IncCol( nTo );
/*N*/ 										else
/*N*/ 											pMove->GetBigRange().aEnd.IncCol( nTo );
/*N*/ 									break;
/*N*/ 									case SC_CAT_DELETE_ROWS :
/*N*/ 										if ( nTo > 0 )
/*N*/ 											pMove->GetBigRange().aStart.IncRow( nTo );
/*N*/ 										else
/*N*/ 											pMove->GetBigRange().aEnd.IncRow( nTo );
/*N*/ 									break;
/*N*/ 									case SC_CAT_DELETE_TABS :
/*N*/ 										if ( nTo > 0 )
/*N*/ 											pMove->GetBigRange().aStart.IncTab( nTo );
/*N*/ 										else
/*N*/ 											pMove->GetBigRange().aEnd.IncTab( nTo );
/*N*/ 									break;
/*N*/ 								}
/*N*/ 							}
/*N*/ 							if ( nFrom || nTo )
/*N*/ 							{
/*N*/ 								ScChangeActionDelMoveEntry* pLink =
/*N*/ 									pActDel->AddCutOffMove( pMove, nFrom, nTo );
/*N*/ 								pMove->AddLink( pActDel, pLink );
/*N*/ 							}
/*N*/ 						}
/*N*/ 						break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				if ( bUpdate )
/*N*/ 				{
/*N*/ 					p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
/*N*/ 					if ( p->GetType() == eActType && !p->IsRejected() &&
/*N*/ 							!pActDel->IsDeletedIn() &&
/*N*/ 							p->GetBigRange().In( aDelRange ) )
/*N*/ 						pActDel->SetDeletedIn( p );		// "druntergerutscht"
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{	// Undo Delete
/*N*/ 			ScChangeAction* pNextAction = NULL;
/*N*/ 			for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
/*N*/ 			{
/*N*/ 				if ( p == pAct )
/*N*/ 					continue;	// for
/*N*/ 				BOOL bUpdate = TRUE;
/*N*/ 				if ( aDelRange.In( p->GetBigRange() ) )
/*N*/ 				{
/*N*/ 					if ( p->IsDeletedInDelType( eActType ) )
/*N*/ 					{
/*N*/ 						if ( p->IsDeletedIn( pActDel ) )
/*N*/ 						{
/*N*/ 							if ( p->GetType() != SC_CAT_CONTENT ||
/*N*/ 									((ScChangeActionContent*)p)->IsTopContent() )
/*N*/ 							{	// erst der TopContent wird wirklich entfernt
/*N*/ 								p->RemoveDeletedIn( pActDel );
/*N*/ 								// GeneratedDelContent _nicht_ aus Liste loeschen,
/*N*/ 								// wir brauchen ihn evtl. noch fuer Reject,
/*N*/ 								// geloescht wird in DeleteCellEntries
/*N*/ 							}
/*N*/ 						}
/*N*/ 						bUpdate = FALSE;
/*N*/ 					}
/*N*/ 					else if ( eActType != SC_CAT_DELETE_TABS &&
/*N*/ 							p->IsDeletedInDelType( SC_CAT_DELETE_TABS ) )
/*N*/ 					{	// in geloeschten Tabellen nicht updaten,
/*N*/ 						// ausser wenn Tabelle verschoben wird
/*N*/ 						bUpdate = FALSE;
/*N*/ 					}
/*N*/ 					if ( p->GetType() == eActType && pActDel->IsDeletedIn( p ) )
/*N*/ 					{
/*N*/ 						pActDel->RemoveDeletedIn( p );	// "druntergerutscht"
/*N*/ 						bUpdate = TRUE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				if ( bUpdate )
/*N*/ 					p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
/*N*/ 			}
/*N*/ 			if ( !bGeneratedDelContents )
/*N*/ 			{	// die werden sonst noch fuer das echte Undo gebraucht
/*N*/ 				pActDel->UndoCutOffInsert();
/*N*/ 				pActDel->UndoCutOffMoves();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( eActType == SC_CAT_MOVE )
/*N*/ 	{
/*N*/ 		ScChangeActionMove* pActMove = (ScChangeActionMove*) pAct;
/*N*/ 		BOOL bLastCutMove = ( pActMove == pLastCutMove );
/*N*/ 		const ScBigRange& rTo = pActMove->GetBigRange();
/*N*/ 		const ScBigRange& rFrom = pActMove->GetFromRange();
/*N*/ 		if ( !bUndo )
/*N*/ 		{	// Move
/*N*/ 			for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
/*N*/ 			{
/*N*/ 				if ( p == pAct )
/*N*/ 					continue;	// for
/*N*/ 				if ( p->GetType() == SC_CAT_CONTENT )
/*N*/ 				{
/*N*/ 					// Inhalt in Ziel deleten (Inhalt in Quelle moven)
/*N*/ 					if ( rTo.In( p->GetBigRange() ) )
/*N*/ 					{
/*N*/ 						if ( !p->IsDeletedIn( pActMove ) )
/*N*/ 						{
/*N*/ 							p->SetDeletedIn( pActMove );
/*N*/ 							// GeneratedDelContent in zu loeschende Liste aufnehmen
/*N*/ 							if ( bGeneratedDelContents )
/*N*/ 								pActMove->AddContent( (ScChangeActionContent*) p );
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else if ( bLastCutMove &&
/*N*/ 							p->GetActionNumber() > nEndLastCut &&
/*N*/ 							rFrom.In( p->GetBigRange() ) )
/*N*/ 					{	// Paste Cut: neuer Content nach Cut eingefuegt, bleibt.
/*N*/ 						// Aufsplitten der ContentChain
/*N*/ 						ScChangeActionContent *pHere, *pTmp;
/*N*/ 						pHere = (ScChangeActionContent*) p;
/*N*/ 						while ( (pTmp = pHere->GetPrevContent()) &&
/*N*/ 								pTmp->GetActionNumber() > nEndLastCut )
/*N*/ 							pHere = pTmp;
/*N*/ 						if ( pTmp )
/*N*/ 						{	// wird TopContent des Move
/*N*/ 							pTmp->SetNextContent( NULL );
/*N*/ 							pHere->SetPrevContent( NULL );
/*N*/ 						}
/*N*/ 						do
/*N*/ 						{	// Abhaengigkeit vom FromRange herstellen
/*N*/ 							AddDependentWithNotify( pActMove, pHere );
/*N*/ 						} while ( pHere = pHere->GetNextContent() );
/*N*/ 					}
/*N*/ 					else
/*N*/ 						p->UpdateReference( this, eMode, rFrom, nDx, nDy, nDz );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{	// Undo Move
/*N*/ 			BOOL bActRejected = pActMove->IsRejected();
/*N*/ 			for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
/*N*/ 			{
/*N*/ 				if ( p == pAct )
/*N*/ 					continue;	// for
/*N*/ 				if ( p->GetType() == SC_CAT_CONTENT )
/*N*/ 				{
/*N*/ 					// Inhalt in Ziel moven, wenn nicht deleted, sonst undelete
/*N*/ 					if ( p->IsDeletedIn( pActMove ) )
/*N*/ 					{
/*N*/ 						if ( ((ScChangeActionContent*)p)->IsTopContent() )
/*N*/ 						{	// erst der TopContent wird wirklich entfernt
/*N*/ 							p->RemoveDeletedIn( pActMove );
/*N*/ 							// GeneratedDelContent _nicht_ aus Liste loeschen,
/*N*/ 							// wir brauchen ihn evtl. noch fuer Reject,
/*N*/ 							// geloescht wird in DeleteCellEntries
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else
/*N*/ 						p->UpdateReference( this, eMode, rTo, nDx, nDy, nDz );
/*N*/ 					if ( bActRejected &&
/*N*/ 							((ScChangeActionContent*)p)->IsTopContent() &&
/*N*/ 							rFrom.In( p->GetBigRange() ) )
/*N*/ 					{	// Abhaengigkeit herstellen, um Content zu schreiben
/*N*/ 						ScChangeActionLinkEntry* pLink =
/*N*/ 							pActMove->AddDependent( p );
/*N*/ 						p->AddLink( pActMove, pLink );
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	// Insert / Undo Insert
/*N*/ 		switch ( GetMergeState() )
/*N*/ 		{
/*N*/ 			case SC_CTMS_NONE :
/*N*/ 			case SC_CTMS_OTHER :
/*N*/ 			{
/*N*/ 				for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
/*N*/ 				{
/*N*/ 					if ( p == pAct )
/*N*/ 						continue;	// for
/*N*/ 					p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case SC_CTMS_PREPARE :
/*N*/ 			{
/*N*/ 				// in Insert-Undo "Deleten"
/*N*/ 				const ScChangeActionLinkEntry* pLink = pAct->GetFirstDependentEntry();
/*N*/ 				while ( pLink )
/*N*/ 				{
/*N*/ 					ScChangeAction* p = (ScChangeAction*) pLink->GetAction();
/*N*/ 					if ( p )
/*N*/ 						p->SetDeletedIn( pAct );
/*N*/ 					pLink = pLink->GetNext();
/*N*/ 				}
/*N*/ 				for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
/*N*/ 				{
/*N*/ 					if ( p == pAct )
/*N*/ 						continue;	// for
/*N*/ 					if ( !p->IsDeletedIn( pAct ) )
/*N*/ 						p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case SC_CTMS_OWN :
/*N*/ 			{
/*N*/ 				for ( ScChangeAction* p = *ppFirstAction; p; p = p->GetNext() )
/*N*/ 				{
/*N*/ 					if ( p == pAct )
/*N*/ 						continue;	// for
/*N*/ 					if ( !p->IsDeletedIn( pAct ) )
/*N*/ 						p->UpdateReference( this, eMode, aRange, nDx, nDy, nDz );
/*N*/ 				}
/*N*/ 				// in Insert-Undo "Delete" rueckgaengig
/*N*/ 				const ScChangeActionLinkEntry* pLink = pAct->GetFirstDependentEntry();
/*N*/ 				while ( pLink )
/*N*/ 				{
/*N*/ 					ScChangeAction* p = (ScChangeAction*) pLink->GetAction();
/*N*/ 					if ( p )
/*N*/ 						p->RemoveDeletedIn( pAct );
/*N*/ 					pLink = pLink->GetNext();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScChangeTrack::Reject( ScChangeAction* pAct, ScChangeActionTable* pTable,
/*N*/ 		BOOL bRecursion )
/*N*/ {
/*N*/ 	if ( !pAct->IsInternalRejectable() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	BOOL bRejected = FALSE;
/*N*/ 	if ( pAct->IsInsertType() )
/*N*/ 	{
/*N*/ 		if ( pAct->HasDependent() && !bRecursion )
/*N*/ 		{
/*N*/ 			const ScBigRange& rRange = pAct->GetBigRange();
/*N*/ 			DBG_ASSERT( pTable, "ScChangeTrack::Reject: Insert ohne Table" );
/*N*/ 			for ( ScChangeAction* p = pTable->Last(); p && bOk; p = pTable->Prev() )
/*N*/ 			{
/*N*/ 				// keine Contents restoren, die eh geloescht werden wuerden
/*N*/ 				if ( p->GetType() == SC_CAT_CONTENT )
/*N*/ 					p->SetRejected();
/*N*/ 				else if ( p->IsDeleteType() )
/*N*/ 					p->Accept();		// geloeschtes ins Nirvana
/*N*/ 				else
/*N*/ 					bOk = Reject( p, NULL, TRUE );		//! rekursiv
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bOk && (bRejected = pAct->Reject( pDoc )) )
/*N*/ 		{
/*N*/ 			// pRefDoc NULL := geloeschte Zellen nicht speichern
/*N*/ 			AppendDeleteRange( pAct->GetBigRange().MakeRange(), NULL, (short) 0,
/*N*/ 				pAct->GetActionNumber() );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( pAct->IsDeleteType() )
/*N*/ 	{
/*N*/ 		DBG_ASSERT( !pTable, "ScChangeTrack::Reject: Delete mit Table" );
/*N*/ 		ScBigRange aDelRange;
/*N*/ 		ULONG nRejectAction = pAct->GetActionNumber();
/*N*/ 		BOOL bTabDel, bTabDelOk;
/*N*/ 		if ( pAct->GetType() == SC_CAT_DELETE_TABS )
/*N*/ 		{
/*N*/ 			bTabDel = TRUE;
/*N*/ 			aDelRange = pAct->GetBigRange();
/*N*/ 			bOk = bTabDelOk = pAct->Reject( pDoc );
/*N*/ 			if ( bOk )
/*N*/ 			{
/*N*/ 				pAct = pAct->GetPrev();
/*N*/ 				bOk = ( pAct && pAct->GetType() == SC_CAT_DELETE_COLS );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bTabDel = bTabDelOk = FALSE;
/*N*/ 		ScChangeActionDel* pDel = (ScChangeActionDel*) pAct;
/*N*/ 		if ( bOk )
/*N*/ 		{
/*N*/ 			aDelRange = pDel->GetOverAllRange();
/*N*/ 			bOk = aDelRange.IsValid( pDoc );
/*N*/ 		}
/*N*/ 		BOOL bOneOk = FALSE;
/*N*/ 		if ( bOk )
/*N*/ 		{
/*N*/ 			ScChangeActionType eActType = pAct->GetType();
/*N*/ 			switch ( eActType )
/*N*/ 			{
/*N*/ 				case SC_CAT_DELETE_COLS :
/*N*/ 					aDelRange.aStart.SetCol( aDelRange.aEnd.Col() );
/*N*/ 				break;
/*N*/ 				case SC_CAT_DELETE_ROWS :
/*N*/ 					aDelRange.aStart.SetRow( aDelRange.aEnd.Row() );
/*N*/ 				break;
/*N*/ 				case SC_CAT_DELETE_TABS :
/*N*/ 					aDelRange.aStart.SetTab( aDelRange.aEnd.Tab() );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			ScChangeAction* p = pAct;
/*N*/ 			BOOL bLoop = TRUE;
/*N*/ 			do
/*N*/ 			{
/*N*/ 				pDel = (ScChangeActionDel*) p;
/*N*/ 				bOk = pDel->Reject( pDoc );
/*N*/ 				if ( bOk )
/*N*/ 				{
/*N*/ 					if ( bOneOk )
/*N*/ 					{
/*N*/ 						switch ( pDel->GetType() )
/*N*/ 						{
/*N*/ 							case SC_CAT_DELETE_COLS :
/*N*/ 								aDelRange.aStart.IncCol( -1 );
/*N*/ 							break;
/*N*/ 							case SC_CAT_DELETE_ROWS :
/*N*/ 								aDelRange.aStart.IncRow( -1 );
/*N*/ 							break;
/*N*/ 							case SC_CAT_DELETE_TABS :
/*N*/ 								aDelRange.aStart.IncTab( -1 );
/*N*/ 							break;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else
/*N*/ 						bOneOk = TRUE;
/*N*/ 				}
/*N*/ 				if ( pDel->IsBaseDelete() )
/*N*/ 					bLoop = FALSE;
/*N*/ 				else
/*N*/ 					p = p->GetPrev();
/*N*/ 			} while ( bOk && bLoop && p && p->GetType() == eActType &&
/*N*/ 				!((ScChangeActionDel*)p)->IsTopDelete() );
/*N*/ 		}
/*N*/ 		bRejected = bOk;
/*N*/ 		if ( bOneOk || (bTabDel && bTabDelOk) )
/*N*/ 		{
/*N*/ 			// Delete-Reject machte UpdateReference Undo
/*N*/ 			ScChangeActionIns* pReject = new ScChangeActionIns(
/*N*/ 				aDelRange.MakeRange() );
/*N*/ 			pReject->SetRejectAction( nRejectAction );
/*N*/ 			pReject->SetState( SC_CAS_ACCEPTED );
/*N*/ 			Append( pReject );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( pAct->GetType() == SC_CAT_MOVE )
/*N*/ 	{
/*N*/ 		if ( pAct->HasDependent() && !bRecursion )
/*N*/ 		{
/*N*/ 			const ScBigRange& rRange = pAct->GetBigRange();
/*N*/ 			DBG_ASSERT( pTable, "ScChangeTrack::Reject: Move ohne Table" );
/*N*/ 			for ( ScChangeAction* p = pTable->Last(); p && bOk; p = pTable->Prev() )
/*N*/ 			{
/*N*/ 				bOk = Reject( p, NULL, TRUE );		//! rekursiv
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bOk && (bRejected = pAct->Reject( pDoc )) )
/*N*/ 		{
/*N*/ 			ScChangeActionMove* pReject = new ScChangeActionMove(
/*N*/ 				pAct->GetBigRange().MakeRange(),
/*N*/ 				((ScChangeActionMove*)pAct)->GetFromRange().MakeRange(), this );
/*N*/ 			pReject->SetRejectAction( pAct->GetActionNumber() );
/*N*/ 			pReject->SetState( SC_CAS_ACCEPTED );
/*N*/ 			Append( pReject );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( pAct->GetType() == SC_CAT_CONTENT )
/*N*/ 	{
/*N*/ 		ScRange aRange;
/*N*/ 		ScChangeActionContent* pReject;
/*N*/ 		if ( bRecursion )
/*N*/ 			pReject = NULL;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			aRange = pAct->GetBigRange().aStart.MakeAddress();
/*N*/ 			pReject = new ScChangeActionContent( aRange );
/*N*/ 			pReject->SetOldValue( pDoc->GetCell( aRange.aStart ), pDoc, pDoc );
/*N*/ 		}
/*N*/ 		if ( (bRejected = pAct->Reject( pDoc )) && !bRecursion )
/*N*/ 		{
/*N*/ 			pReject->SetNewValue( pDoc->GetCell( aRange.aStart ), pDoc );
/*N*/ 			pReject->SetRejectAction( pAct->GetActionNumber() );
/*N*/ 			pReject->SetState( SC_CAS_ACCEPTED );
/*N*/ 			Append( pReject );
/*N*/ 		}
/*N*/ 		else if ( pReject )
/*N*/ 			delete pReject;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR( "ScChangeTrack::Reject: say what?" );
/*N*/ 
/*N*/ 	return bRejected;
/*N*/ }


ULONG ScChangeTrack::AddLoadedGenerated(ScBaseCell* pNewCell, const ScBigRange& aBigRange )
{
/*N*/  	ScChangeActionContent* pAct = new ScChangeActionContent( --nGeneratedMin, pNewCell, aBigRange, pDoc );
/*N*/ 	if ( pAct )
/*N*/ 	{
/*N*/ 		if ( pFirstGeneratedDelContent )
/*N*/ 			pFirstGeneratedDelContent->pPrev = pAct;
/*N*/ 		pAct->pNext = pFirstGeneratedDelContent;
/*N*/ 		pFirstGeneratedDelContent = pAct;
/*N*/ 		aGeneratedTable.Insert( pAct->GetActionNumber(), pAct );
/*N*/ 		return pAct->GetActionNumber();
/*N*/ 	}
/*N*/  	return 0;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
