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

#include <bf_sfx2/objsh.hxx>
#include <bf_svtools/lstner.hxx>

// INCLUDE ---------------------------------------------------------------

#include "document.hxx"
#include "bcaslot.hxx"
#include "scerrors.hxx"
namespace binfilter {

// Anzahl der Slots je Dimension
// muessen ganzzahlige Teiler von MAXCOL+1 bzw. MAXROW+1 sein
#define BCA_SLOTS_COL 16
#define BCA_SLOTS_ROW 256
#define BCA_SLOT_COLS ((MAXCOL+1) / BCA_SLOTS_COL)
#define BCA_SLOT_ROWS ((MAXROW+1) / BCA_SLOTS_ROW)
// vielfaches?
#if (BCA_SLOT_COLS * BCA_SLOTS_COL) != (MAXCOL+1)
#error bad BCA_SLOTS_COL value!
#endif
#if (BCA_SLOT_ROWS * BCA_SLOTS_ROW) != (MAXROW+1)
#error bad BCA_SLOTS_ROW value!
#endif
// Groesse des Slot-Arrays
#define BCA_SLOTS (BCA_SLOTS_COL * BCA_SLOTS_ROW)
#if BCA_SLOTS > 16350
#error BCA_SLOTS DOOMed!
#endif

DECLARE_LIST( ScBroadcastAreaList, ScBroadcastArea* )//STRIP008 ;

// STATIC DATA -----------------------------------------------------------

#ifdef erDEBUG
ULONG erCountBCAInserts = 0;
ULONG erCountBCAFinds = 0;
#endif

/*N*/ SV_IMPL_OP_PTRARR_SORT( ScBroadcastAreas, ScBroadcastAreaPtr );
/*N*/ TYPEINIT1( ScHint, SfxSimpleHint );
TYPEINIT1( ScAreaChangedHint, SfxHint );


/*N*/ ScBroadcastAreaSlot::ScBroadcastAreaSlot( ScDocument* pDocument,
/*N*/ 		ScBroadcastAreaSlotMachine* pBASMa ) :
/*N*/ 	pDoc( pDocument ),
/*N*/ 	pBASM( pBASMa )
/*N*/ {
/*N*/ 	pBroadcastAreaTbl = new ScBroadcastAreas( BCA_INITGROWSIZE, BCA_INITGROWSIZE );
/*N*/ 	pTmpSeekBroadcastArea = new ScBroadcastArea( ScRange() );
/*N*/ }


/*N*/ ScBroadcastAreaSlot::~ScBroadcastAreaSlot()
/*N*/ {
/*N*/ 	USHORT nPos = pBroadcastAreaTbl->Count();
/*N*/ 	if ( nPos )
/*N*/ 	{
/*N*/ 		ScBroadcastArea** ppArea =
/*N*/ 			((ScBroadcastArea**) pBroadcastAreaTbl->GetData()) + nPos - 1;
/*N*/ 		for ( ; nPos-- >0; ppArea-- )
/*N*/ 		{
/*N*/ 			if ( !(*ppArea)->DecRef() )
/*N*/ 				delete *ppArea;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	delete pBroadcastAreaTbl;
/*N*/ 	delete pTmpSeekBroadcastArea;
/*N*/ }


// nur hier werden neue BroadcastAreas angelegt, wodurch keine doppelten entstehen.
// Ist rpArea != NULL werden keine Listener gestartet sondern nur die Area
// eingetragen und der RefCount erhoeht
/*N*/ void ScBroadcastAreaSlot::StartListeningArea( const ScRange& rRange,
/*N*/ 		SfxListener* pListener, ScBroadcastArea*& rpArea
/*N*/ 	)
/*N*/ {
/*N*/ 	DBG_ASSERT(pListener, "StartListeningArea: pListener Null");
/*N*/ 	if ( pDoc->GetHardRecalcState() )
/*N*/ 		return;
/*N*/ 	if ( (long)( (pBroadcastAreaTbl->Count() + 1 + BCA_INITGROWSIZE)
/*N*/ 			* sizeof(ScBroadcastArea*) ) >= USHRT_MAX
/*N*/ 		)
/*N*/ 	{
/*?*/ 		if ( !pDoc->GetHardRecalcState() )
/*?*/ 		{
/*?*/ 			pDoc->SetHardRecalcState( 1 );
/*?*/ 
/*?*/ 			SfxObjectShell* pShell = pDoc->GetDocumentShell();
/*?*/ 			DBG_ASSERT( pShell, "Missing DocShell :-/" );
/*?*/ 
/*?*/ 			if ( pShell )
/*?*/ 				pShell->SetError( SCWARN_CORE_HARD_RECALC );
/*?*/ 
/*?*/ 			pDoc->SetAutoCalc( FALSE );
/*?*/ 			pDoc->SetHardRecalcState( 2 );
/*?*/ 		}
/*?*/ 		return;
/*N*/ 	}
/*N*/ 	if ( !rpArea )
/*N*/ 	{
/*N*/ 		rpArea = new ScBroadcastArea( rRange );
/*N*/ 		// meistens existiert die Area noch nicht, der Versuch sofort zu inserten
/*N*/ 		// erspart in diesen Faellen ein doppeltes Seek_Entry
/*N*/ 		if ( pBroadcastAreaTbl->Insert( rpArea ) )
/*N*/ 			rpArea->IncRef();
/*N*/ 		else
/*N*/ 		{
/*N*/ 			delete rpArea;
/*N*/ 			rpArea = GetBroadcastArea( rRange );
/*N*/ 		}
/*N*/ 		pListener->StartListening( *rpArea, TRUE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( pBroadcastAreaTbl->Insert( rpArea ) )
/*N*/ 			rpArea->IncRef();
/*N*/ 	}
/*N*/ }


/*N*/ // Ist rpArea != NULL werden keine Listener gestopt sondern nur die Area
/*N*/ // ausgetragen und der RefCount vermindert
/*N*/ void ScBroadcastAreaSlot::EndListeningArea( const ScRange& rRange,
/*N*/ 		SfxListener* pListener, ScBroadcastArea*& rpArea
/*N*/ 	)
/*N*/ {
/*N*/ 	DBG_ASSERT(pListener, "EndListeningArea: pListener Null");
/*N*/ 	if ( !rpArea )
/*N*/ 	{
/*N*/ 		USHORT nPos;
/*N*/ 		if ( (nPos = FindBroadcastArea( rRange )) == USHRT_MAX )
/*N*/ 			return;
/*N*/ 		rpArea = (*pBroadcastAreaTbl)[ nPos ];
/*N*/ 		pListener->EndListening( *rpArea );
/*N*/ 		if ( !rpArea->HasListeners() )
/*N*/ 		{	// wenn keiner mehr zuhoert ist die Area ueberfluessig
/*N*/ 			pBroadcastAreaTbl->Remove( nPos );
/*N*/ 			if ( !rpArea->DecRef() )
/*N*/ 			{
/*N*/ 				delete rpArea;
/*N*/ 				rpArea = NULL;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( !rpArea->HasListeners() )
/*N*/ 		{
/*N*/ 			USHORT nPos;
/*N*/ 			if ( (nPos = FindBroadcastArea( rRange )) == USHRT_MAX )
/*N*/ 				return;
/*N*/ 			pBroadcastAreaTbl->Remove( nPos );
/*N*/ 			if ( !rpArea->DecRef() )
/*N*/ 			{
/*N*/ 				delete rpArea;
/*N*/ 				rpArea = NULL;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ 
/*N*/ USHORT ScBroadcastAreaSlot::FindBroadcastArea( const ScRange& rRange ) const
/*N*/ {
/*N*/ 	USHORT nPos;
/*N*/ 	pTmpSeekBroadcastArea->UpdateRange( rRange );
/*N*/ 	if ( pBroadcastAreaTbl->Seek_Entry( pTmpSeekBroadcastArea, &nPos ) )
/*N*/ 		return nPos;
/*N*/ 	return USHRT_MAX;
/*N*/ }


/*N*/ ScBroadcastArea* ScBroadcastAreaSlot::GetBroadcastArea(
/*N*/ 		const ScRange& rRange ) const
/*N*/ {
/*N*/ 	USHORT nPos;
/*N*/ 	if ( (nPos = FindBroadcastArea( rRange )) != USHRT_MAX )
/*N*/ 		return (*pBroadcastAreaTbl)[ nPos ];
/*N*/ 	return 0;
/*N*/ }


/*N*/ BOOL ScBroadcastAreaSlot::AreaBroadcast( const ScHint& rHint) const
/*N*/ {
/*N*/ 	USHORT nCount = pBroadcastAreaTbl->Count();
/*N*/ 	if ( nCount == 0 )
/*N*/ 		return FALSE;
/*N*/ 	const ScBroadcastArea** ppArea =
/*N*/ 		(const ScBroadcastArea**) pBroadcastAreaTbl->GetData();
/*N*/ 	BOOL bIsBroadcasted = FALSE;
/*N*/ 	// leider laesst sich nicht nach dem erstmoeglichen suchen
/*N*/ 	USHORT nPos = 0;
/*N*/ 	// den letztmoeglichen suchen, Seek_Entry liefert naechst groesseren
/*N*/ 	// oder freie Position wenn nicht gefunden
/*N*/ 	USHORT nPosEnd;
/*N*/     const ScAddress& rAddress = rHint.GetAddress();
/*N*/ 	pTmpSeekBroadcastArea->UpdateRange( ScRange( rAddress,
/*N*/ 		ScAddress( MAXCOL, MAXROW, MAXTAB ) ) );
/*N*/ 	if ( !pBroadcastAreaTbl->Seek_Entry( pTmpSeekBroadcastArea, &nPosEnd )
/*N*/ 	  && nPosEnd > 0 )
/*N*/ 		--nPosEnd;
/*N*/ 	for ( ; nPos <= nPosEnd; ++nPos, ppArea++ )
/*N*/ 	{
/*N*/ 		if ( ((ScBroadcastArea*)*ppArea)->In( rAddress ) )
/*N*/ 		{
/*N*/ 			((ScBroadcastArea*)*ppArea)->Broadcast( rHint );
/*N*/ 			bIsBroadcasted = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bIsBroadcasted;
/*N*/ }


/*N*/ BOOL ScBroadcastAreaSlot::AreaBroadcastInRange( const ScRange& rRange,
/*N*/         const ScHint& rHint) const
/*N*/ {
/*N*/ 	USHORT nCount = pBroadcastAreaTbl->Count();
/*N*/ 	if ( nCount == 0 )
/*N*/ 		return FALSE;
/*N*/ 	const ScBroadcastArea** ppArea =
/*N*/ 		(const ScBroadcastArea**) pBroadcastAreaTbl->GetData();
/*N*/ 	BOOL bIsBroadcasted = FALSE;
/*N*/ 	// unfortunately we can't search for the first matching entry
/*N*/ 	USHORT nPos = 0;
/*N*/     // search the last matching entry, Seek_Entry returns the next being
/*N*/     // greater, or a free position if not found
/*N*/ 	USHORT nPosEnd;
/*N*/ 	pTmpSeekBroadcastArea->UpdateRange( rRange );
/*N*/     if ( !pBroadcastAreaTbl->Seek_Entry( pTmpSeekBroadcastArea, &nPosEnd ) &&
/*N*/             nPosEnd > 0 )
/*N*/         --nPosEnd;
/*N*/ 	for ( ; nPos <= nPosEnd; ++nPos, ppArea++ )
/*N*/ 	{
/*N*/ 		if ( ((ScBroadcastArea*)*ppArea)->Intersects( rRange ) )
/*N*/ 		{
/*N*/ 			((ScBroadcastArea*)*ppArea)->Broadcast( rHint );
/*N*/ 			bIsBroadcasted = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bIsBroadcasted;
/*N*/ }


//	DelBroadcastAreasInRange wird unter Windows (16 Bit) kaputtoptimiert

#ifdef WIN
#pragma optimize("",off)
#endif

/*N*/ void ScBroadcastAreaSlot::DelBroadcastAreasInRange( const ScRange& rRange )
/*N*/ {
/*N*/ 	ScBroadcastArea* pArea;
/*N*/ 	ScAddress aStart( rRange.aStart );
/*N*/ 	USHORT nPos = pBroadcastAreaTbl->Count();
/*N*/ 	const ScBroadcastArea** ppArea =
/*N*/ 		(const ScBroadcastArea**) pBroadcastAreaTbl->GetData() + nPos - 1;
/*N*/ 	for ( ; nPos-- >0; ppArea-- )
/*N*/ 	{	// rueckwaerts wg. Pointer-Aufrueckerei im Array
/*N*/ 		pArea = (ScBroadcastArea*)*ppArea;
/*N*/ 		if ( pArea->aStart < aStart )
/*N*/ 			return;		// davor nur noch niedrigere
/*N*/ 			// gesuchte muessen komplett innerhalb von rRange liegen
/*N*/ 		if ( rRange.In( pArea->aStart ) && rRange.In( pArea->aEnd ) )
/*N*/ 		{
/*N*/ 			pBroadcastAreaTbl->Remove( nPos );
/*N*/ 			ppArea = (const ScBroadcastArea**) pBroadcastAreaTbl->GetData()
/*N*/ 				+ nPos;
/*N*/ 			if ( !pArea->DecRef() )
/*N*/ 				delete pArea;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

#ifdef WIN
#pragma optimize("",on)
#endif





// --- ScBroadcastAreaSlotMachine -------------------------------------

/*N*/ ScBroadcastAreaSlotMachine::ScBroadcastAreaSlotMachine(
/*N*/ 		ScDocument* pDocument ) :
/*N*/ 	pDoc( pDocument ),
/*N*/ 	pBCAlwaysList( new ScBroadcastAreaList ),
/*N*/ 	pUpdateChain( NULL ),
/*N*/ 	pEOUpdateChain( NULL )
/*N*/ {
/*N*/ 	ppSlots = new ScBroadcastAreaSlot* [ BCA_SLOTS ];
/*N*/ 	memset( ppSlots, 0 , sizeof( ScBroadcastAreaSlot* ) * BCA_SLOTS );
/*N*/ }


/*N*/ ScBroadcastAreaSlotMachine::~ScBroadcastAreaSlotMachine()
/*N*/ {
/*N*/ 
/*N*/ 	ScBroadcastAreaSlot** pp = ppSlots;
/*N*/ 	for ( USHORT j=0; j < BCA_SLOTS; ++j, ++pp )
/*N*/ 	{
/*N*/ 		if ( *pp )
/*N*/ 			delete *pp;
/*N*/ 	}
/*N*/ 	delete[] ppSlots;
/*N*/ 
/*N*/ 	for ( ScBroadcastArea* pBCA = pBCAlwaysList->First(); pBCA; pBCA = pBCAlwaysList->Next() )
/*N*/ 	{
/*N*/ 		delete pBCA;
/*N*/ 	}
/*N*/ 	delete pBCAlwaysList;
/*N*/ }


/*N*/ inline USHORT ScBroadcastAreaSlotMachine::ComputeSlotOffset(
/*N*/ 		const ScAddress& rAddress ) const
/*N*/ {
/*N*/  USHORT nRow = rAddress.Row();
/*N*/  USHORT nCol = rAddress.Col();
/*N*/  if ( nRow > MAXROW || nCol > MAXCOL )
/*N*/  {
/*N*/  	DBG_ASSERT( FALSE, "Row/Col ungueltig!" );
/*N*/  	return 0;
/*N*/  }
/*N*/  else
/*N*/  	return
/*N*/  		nRow / BCA_SLOT_ROWS +
/*N*/  		nCol / BCA_SLOT_COLS * BCA_SLOTS_ROW;
/*N*/ }


/*N*/ void ScBroadcastAreaSlotMachine::ComputeAreaPoints( const ScRange& rRange,
/*N*/ 		USHORT& rStart, USHORT& rEnd, USHORT& rRowBreak
/*N*/ 	) const
/*N*/ {
/*N*/ 	rStart = ComputeSlotOffset( rRange.aStart );
/*N*/ 	rEnd = ComputeSlotOffset( rRange.aEnd );
/*N*/ 	// Anzahl Zeilen-Slots pro Spalte minus eins
/*N*/ 	rRowBreak = ComputeSlotOffset(
/*N*/ 		ScAddress( rRange.aStart.Col(), rRange.aEnd.Row(), 0 ) ) - rStart;
/*N*/ }


/*N*/ void ScBroadcastAreaSlotMachine::StartListeningArea( const ScRange& rRange,
/*N*/ 		SfxListener* pListener
/*N*/ 	)
/*N*/ {
/*N*/ 	if ( rRange == BCA_LISTEN_ALWAYS  )
/*N*/ 	{
/*N*/ 		ScBroadcastArea* pBCA;
/*N*/ 		if ( !pBCAlwaysList->Count() )
/*N*/ 		{
/*N*/ 			pBCA = new ScBroadcastArea( rRange );
/*N*/ 			pListener->StartListening( *pBCA, FALSE );	// kein PreventDupes
/*N*/ 			pBCAlwaysList->Insert( pBCA, LIST_APPEND );
/*N*/ 			return ;
/*N*/ 		}
/*N*/ 		ScBroadcastArea* pLast;
/*N*/ 		for ( pBCA = pBCAlwaysList->First(); pBCA; pBCA = pBCAlwaysList->Next() )
/*N*/ 		{
/*N*/ 			if ( pListener->IsListening( *pBCA ) )
/*N*/ 				return ;		// keine Dupes
/*N*/ 			pLast = pBCA;
/*N*/ 		}
/*?*/ 		pBCA = pLast;
/*?*/ 		//! ListenerArrays don't shrink!
/*?*/ 		if ( pBCA->GetListenerCount() > ((USHRT_MAX / 2) / sizeof(SfxBroadcaster*)) )
/*?*/ 		{	// Arrays nicht zu gross werden lassen
/*?*/ 			pBCA = new ScBroadcastArea( rRange );
/*?*/ 			pBCAlwaysList->Insert( pBCA, LIST_APPEND );
/*?*/ 		}
/*?*/ 		pListener->StartListening( *pBCA, FALSE );	// kein PreventDupes
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nStart, nEnd, nRowBreak;
/*N*/ 		ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
/*N*/ 		USHORT nOff = nStart;
/*N*/ 		USHORT nBreak = nOff + nRowBreak;
/*N*/ 		ScBroadcastAreaSlot** pp = ppSlots + nOff;
/*N*/ 		ScBroadcastArea* pArea = NULL;
/*N*/ 		while ( nOff <= nEnd )
/*N*/ 		{
/*N*/ 			if ( !*pp )
/*N*/ 				*pp = new ScBroadcastAreaSlot( pDoc, this );
/*N*/ 			// der erste erzeugt ggbf. die BroadcastArea
/*N*/ 			(*pp)->StartListeningArea( rRange, pListener, pArea );
/*N*/ 			if ( nOff < nBreak )
/*N*/ 			{
/*N*/ 				++nOff;
/*N*/ 				++pp;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				nStart += BCA_SLOTS_ROW;
/*N*/ 				nOff = nStart;
/*N*/ 				pp = ppSlots + nOff;
/*N*/ 				nBreak = nOff + nRowBreak;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScBroadcastAreaSlotMachine::EndListeningArea( const ScRange& rRange,
/*N*/ 		SfxListener* pListener
/*N*/ 	)
/*N*/ {
/*N*/ 	if ( rRange == BCA_LISTEN_ALWAYS  )
/*N*/ 	{
/*?*/ 		if ( pBCAlwaysList->Count() )
/*?*/ 		{
/*?*/ 			for ( ScBroadcastArea* pBCA = pBCAlwaysList->First(); pBCA; pBCA = pBCAlwaysList->Next() )
/*?*/ 			{
/*?*/ 				// EndListening liefert FALSE wenn !IsListening, keine Dupes
/*?*/ 				if ( pListener->EndListening( *pBCA, FALSE ) )
/*?*/ 				{
/*?*/ 					if ( !pBCA->HasListeners() )
/*?*/ 					{
/*?*/ 						pBCAlwaysList->Remove();
/*?*/ 						delete pBCA;
/*?*/ 					}
/*?*/ 					return ;
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nStart, nEnd, nRowBreak;
/*N*/ 		ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
/*N*/ 		USHORT nOff = nStart;
/*N*/ 		USHORT nBreak = nOff + nRowBreak;
/*N*/ 		ScBroadcastAreaSlot** pp = ppSlots + nOff;
/*N*/ 		ScBroadcastArea* pArea = NULL;
/*N*/ 		while ( nOff <= nEnd )
/*N*/ 		{
/*N*/ 			if ( *pp )
/*N*/ 				(*pp)->EndListeningArea( rRange, pListener, pArea );
/*N*/ 			if ( nOff < nBreak )
/*N*/ 			{
/*N*/ 				++nOff;
/*N*/ 				++pp;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				nStart += BCA_SLOTS_ROW;
/*N*/ 				nOff = nStart;
/*N*/ 				pp = ppSlots + nOff;
/*N*/ 				nBreak = nOff + nRowBreak;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ 
/*N*/ BOOL ScBroadcastAreaSlotMachine::AreaBroadcast( const ScHint& rHint ) const
/*N*/ {
/*N*/     const ScAddress& rAddress = rHint.GetAddress();
/*N*/ 	if ( rAddress == BCA_BRDCST_ALWAYS )
/*N*/ 	{
/*N*/ 		if ( pBCAlwaysList->Count() )
/*N*/ 		{
/*N*/ 			for ( ScBroadcastArea* pBCA = pBCAlwaysList->First(); pBCA; pBCA = pBCAlwaysList->Next() )
/*N*/ 			{
/*N*/ 				pBCA->Broadcast( rHint );
/*N*/ 			}
/*N*/ 			return TRUE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ScBroadcastAreaSlot* pSlot = ppSlots[ ComputeSlotOffset( rAddress ) ];
/*N*/ 		if ( pSlot )
/*N*/ 			return pSlot->AreaBroadcast( rHint );
/*N*/ 		else
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScBroadcastAreaSlotMachine::AreaBroadcastInRange( const ScRange& rRange,
/*N*/         const ScHint& rHint ) const
/*N*/ {
/*N*/     BOOL bBroadcasted = FALSE;
/*N*/ 	USHORT nStart, nEnd, nRowBreak;
/*N*/ 	ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
/*N*/ 	USHORT nOff = nStart;
/*N*/ 	USHORT nBreak = nOff + nRowBreak;
/*N*/ 	ScBroadcastAreaSlot** pp = ppSlots + nOff;
/*N*/ 	while ( nOff <= nEnd )
/*N*/ 	{
/*N*/ 		if ( *pp )
/*N*/ 			bBroadcasted |= (*pp)->AreaBroadcastInRange( rRange, rHint );
/*N*/ 		if ( nOff < nBreak )
/*N*/ 		{
/*N*/ 			++nOff;
/*N*/ 			++pp;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nStart += BCA_SLOTS_ROW;
/*N*/ 			nOff = nStart;
/*N*/ 			pp = ppSlots + nOff;
/*N*/ 			nBreak = nOff + nRowBreak;
/*N*/ 		}
/*N*/ 	}
/*N*/     return bBroadcasted;
/*N*/ }


/*N*/ void ScBroadcastAreaSlotMachine::DelBroadcastAreasInRange(
/*N*/ 		const ScRange& rRange
/*N*/ 	)
/*N*/ {
/*N*/ 	USHORT nStart, nEnd, nRowBreak;
/*N*/ 	ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
/*N*/ 	USHORT nOff = nStart;
/*N*/ 	USHORT nBreak = nOff + nRowBreak;
/*N*/ 	ScBroadcastAreaSlot** pp = ppSlots + nOff;
/*N*/ 	while ( nOff <= nEnd )
/*N*/ 	{
/*N*/ 		if ( *pp )
/*N*/ 			(*pp)->DelBroadcastAreasInRange( rRange );
/*N*/ 		if ( nOff < nBreak )
/*N*/ 		{
/*N*/ 			++nOff;
/*N*/ 			++pp;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nStart += BCA_SLOTS_ROW;
/*N*/ 			nOff = nStart;
/*N*/ 			pp = ppSlots + nOff;
/*N*/ 			nBreak = nOff + nRowBreak;
/*N*/ 		}
/*N*/ 	}
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
