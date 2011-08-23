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


#include "refupdat.hxx"
#include "compiler.hxx"
#include "chgtrack.hxx"
namespace binfilter {

//------------------------------------------------------------------------

/*N*/ BOOL lcl_MoveStart( short& rRef, short nStart, short nDelta, short nMask )
/*N*/ {
/*N*/ 	BOOL bCut = FALSE;
/*N*/ 	if ( rRef >= nStart )
/*N*/ 		rRef += nDelta;
/*N*/ 	else if ( nDelta < 0 && rRef >= nStart + nDelta )
/*N*/ 		rRef = nStart + nDelta;				//! begrenzen ???
/*N*/ 	if ( rRef < 0 )
/*N*/ 	{
/*N*/ 		rRef = 0;
/*N*/ 		bCut = TRUE;
/*N*/ 	}
/*N*/ 	else if ( rRef > nMask )
/*N*/ 	{
/*N*/ 		rRef = nMask;
/*N*/ 		bCut = TRUE;
/*N*/ 	}
/*N*/ 	return bCut;
/*N*/ }

/*N*/ BOOL lcl_MoveEnd( short& rRef, short nStart, short nDelta, short nMask )
/*N*/ {
/*N*/ 	BOOL bCut = FALSE;
/*N*/ 	if ( rRef >= nStart )
/*N*/ 		rRef += nDelta;
/*N*/ 	else if ( nDelta < 0 && rRef >= nStart + nDelta )
/*N*/ 		rRef = nStart + nDelta - 1;			//! begrenzen ???
/*N*/ 	if ( rRef < 0 )
/*N*/ 	{
/*N*/ 		rRef = 0;
/*N*/ 		bCut = TRUE;
/*N*/ 	}
/*N*/ 	else if ( rRef > nMask )
/*N*/ 	{
/*N*/ 		rRef = nMask;
/*N*/ 		bCut = TRUE;
/*N*/ 	}
/*N*/ 	return bCut;
/*N*/ }


/*N*/  BOOL lcl_MoveItCut( short& rRef, short nDelta, short nMask )
/*N*/  {
/*N*/  	BOOL bCut = FALSE;
/*N*/  	rRef += nDelta;
/*N*/  	if ( rRef < 0 )
/*N*/  	{
/*N*/  		rRef = 0;
/*N*/  		bCut = TRUE;
/*N*/  	}
/*N*/  	else if ( rRef > nMask )
/*N*/  	{
/*N*/  		rRef = nMask;
/*N*/  		bCut = TRUE;
/*N*/  	}
/*N*/  	return bCut;
/*N*/  }

/*N*/ void lcl_MoveItWrap( short& rRef, short nDelta, short nMask )
/*N*/ {
/*N*/ 	rRef += nDelta;
/*N*/ 	if ( rRef < 0 )
/*N*/ 		rRef += nMask+1;
/*N*/ 	else if ( rRef > nMask )
/*N*/ 		rRef -= nMask+1;
/*N*/ }

/*N*/  BOOL lcl_MoveRefPart( short& rRef1Val, BOOL& rRef1Del,
/*N*/  					  short& rRef2Val, BOOL& rRef2Del,
/*N*/  					  short nStart, short nEnd, short nDelta, short nMask )
/*N*/  {
/*N*/  	if ( nDelta )
/*N*/  	{
/*N*/  		BOOL bDel, bCut1, bCut2;
/*N*/  		bDel = bCut1 = bCut2 = FALSE;
/*N*/  		short n;
/*N*/  		if ( nDelta < 0 )
/*N*/  		{
/*N*/  			n = nStart + nDelta;
/*N*/  			if ( n <= rRef1Val && rRef1Val < nStart
/*N*/  			  && n <= rRef2Val && rRef2Val < nStart )
/*N*/  				bDel = TRUE;
/*N*/  		}
/*N*/  		else
/*N*/  		{
/*N*/  			n = nEnd + nDelta;
/*N*/  			if ( nEnd < rRef1Val && rRef1Val <= n
/*N*/  			  && nEnd < rRef2Val && rRef2Val <= n )
/*N*/  				bDel = TRUE;
/*N*/  		}
/*N*/  		if ( bDel )
/*N*/  		{	// geloeschte mitverschieben
/*N*/  			rRef1Val += nDelta;
/*N*/  			rRef2Val += nDelta;
/*N*/  		}
/*N*/  		else
/*N*/  		{
/*N*/  			if ( rRef1Del )
/*N*/  				rRef1Val += nDelta;
/*N*/  			else
/*N*/  				bCut1 = lcl_MoveStart( rRef1Val, nStart, nDelta, nMask );
/*N*/  			if ( rRef2Del )
/*N*/  				rRef2Val += nDelta;
/*N*/  			else
/*N*/  				bCut2 = lcl_MoveEnd( rRef2Val, nStart, nDelta, nMask );
/*N*/  		}
/*N*/  		if ( bDel || (bCut1 && bCut2) )
/*N*/  			rRef1Del = rRef2Del = TRUE;
/*N*/  		return bDel || bCut1 || bCut2 || rRef1Del || rRef2Del;
/*N*/  	}
/*N*/  	else
/*N*/  		return FALSE;
/*N*/  }

/*N*/ #if OSL_DEBUG_LEVEL < 2
/*N*/ inline
/*N*/ #endif
/*N*/ BOOL IsExpand( short n1, short n2, short nStart, short nD )
/*N*/ {	//! vor normalem Move...
/*N*/ 	return
/*N*/ 		nD > 0 			// Insert
/*N*/ 	 && n1 < n2			// mindestens zwei Cols/Rows/Tabs in Ref
/*N*/ 	 && (
/*N*/ 		(nStart <= n1 && n1 < nStart + nD)		// n1 innerhalb des Insert
/*N*/ 		|| (n2 + 1 == nStart)					// n2 direkt vor Insert
/*N*/ 		);		// n1 < nStart <= n2 wird sowieso expanded!
/*N*/ }


/*N*/ #if OSL_DEBUG_LEVEL < 2
/*N*/ inline
/*N*/ #endif
/*N*/ void Expand( short& n1, short& n2, short nStart, short nD )
/*N*/ {	//! nach normalem Move..., nur wenn IsExpand vorher TRUE war!
/*N*/ 	//! erst das Ende
/*N*/ 	if ( n2 + 1 == nStart )
/*N*/ 	{	// am Ende
/*N*/ 		n2 += nD;
/*N*/ 		return;
/*N*/ 	}
/*N*/ 	// am Anfang
/*N*/ 	n1 -= nD;
/*N*/ }


/*N*/  BOOL lcl_IsWrapBig( INT32 nRef, INT32 nDelta )
/*N*/  {
/*N*/  	if ( nRef > 0 && nDelta > 0 )
/*N*/  		return nRef + nDelta <= 0;
/*N*/  	else if ( nRef < 0 && nDelta < 0 )
/*N*/  		return nRef + nDelta >= 0;
/*N*/  	return FALSE;
/*N*/  }


/*N*/  BOOL lcl_MoveBig( INT32& rRef, INT32 nStart, INT32 nDelta )
/*N*/  {
/*N*/  	BOOL bCut = FALSE;
/*N*/  	if ( rRef >= nStart )
/*N*/  	{
/*N*/  		if ( nDelta > 0 )
/*N*/  			bCut = lcl_IsWrapBig( rRef, nDelta );
/*N*/  		if ( bCut )
/*N*/  			rRef = nInt32Max;
/*N*/  		else
/*N*/  			rRef += nDelta;
/*N*/  	}
/*N*/  	return bCut;
/*N*/  }

/*N*/  BOOL lcl_MoveItCutBig( INT32& rRef, INT32 nDelta )
/*N*/  {
/*N*/  	BOOL bCut = lcl_IsWrapBig( rRef, nDelta );
/*N*/  	rRef += nDelta;
/*N*/  	return bCut;
/*N*/  }


/*N*/ ScRefUpdateRes ScRefUpdate::Update( ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
/*N*/ 										USHORT nCol1, USHORT nRow1, USHORT nTab1,
/*N*/ 										USHORT nCol2, USHORT nRow2, USHORT nTab2,
/*N*/ 										short nDx, short nDy, short nDz,
/*N*/ 										USHORT& theCol1, USHORT& theRow1, USHORT& theTab1,
/*N*/ 										USHORT& theCol2, USHORT& theRow2, USHORT& theTab2 )
/*N*/ {
/*N*/ 	ScRefUpdateRes eRet = UR_NOTHING;
/*N*/ 
/*N*/ 	USHORT oldCol1 = theCol1;
/*N*/ 	USHORT oldRow1 = theRow1;
/*N*/ 	USHORT oldTab1 = theTab1;
/*N*/ 	USHORT oldCol2 = theCol2;
/*N*/ 	USHORT oldRow2 = theRow2;
/*N*/ 	USHORT oldTab2 = theTab2;
/*N*/ 
/*N*/ 	BOOL bCut1, bCut2;
/*N*/ 
/*N*/ 	if (eUpdateRefMode == URM_INSDEL)
/*N*/ 	{
/*N*/ 		BOOL bExpand = pDoc->IsExpandRefs();
/*N*/ 		if ( nDx && (theRow1 >= nRow1) && (theRow2 <= nRow2) &&
/*N*/ 					(theTab1 >= nTab1) && (theTab2 <= nTab2) )
/*N*/ 		{
/*N*/ 			BOOL bExp = (bExpand && IsExpand( theCol1, theCol2, nCol1, nDx ));
/*N*/ 			bCut1 = lcl_MoveStart( (short&) theCol1, nCol1, nDx, MAXCOL );
/*N*/ 			bCut2 = lcl_MoveEnd( (short&) theCol2, nCol1, nDx, MAXCOL );
/*N*/ 			if ( theCol2 < theCol1 )
/*N*/ 			{
/*N*/ 				eRet = UR_INVALID;
/*N*/ 				theCol2 = theCol1;
/*N*/ 			}
/*N*/ 			else if ( bCut1 || bCut2 )
/*N*/ 				eRet = UR_UPDATED;
/*N*/ 			if ( bExp )
/*N*/ 			{
/*N*/ 				Expand( (short&) theCol1, (short&) theCol2, nCol1, nDx );
/*N*/ 				eRet = UR_UPDATED;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( nDy && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
/*N*/ 					(theTab1 >= nTab1) && (theTab2 <= nTab2) )
/*N*/ 		{
/*?*/ 			BOOL bExp = (bExpand && IsExpand( theRow1, theRow2, nRow1, nDy ));
/*?*/ 			bCut1 = lcl_MoveStart( (short&) theRow1, nRow1, nDy, MAXROW );
/*?*/ 			bCut2 = lcl_MoveEnd( (short&) theRow2, nRow1, nDy, MAXROW );
/*?*/ 			if ( theRow2 < theRow1 )
/*?*/ 			{
/*?*/ 				eRet = UR_INVALID;
/*?*/ 				theRow2 = theRow1;
/*?*/ 			}
/*?*/ 			else if ( bCut1 || bCut2 )
/*?*/ 				eRet = UR_UPDATED;
/*?*/ 			if ( bExp )
/*?*/ 			{
/*?*/ 				Expand( (short&) theRow1, (short&) theRow2, nRow1, nDy );
/*?*/ 				eRet = UR_UPDATED;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		if ( nDz && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
/*N*/ 					(theRow1 >= nRow1) && (theRow2 <= nRow2) )
/*N*/ 		{
/*N*/ 			short nMaxTab = pDoc->GetTableCount() - 1;
/*N*/ 			nMaxTab += nDz;		// auf die neue Anzahl anpassen
/*N*/ 			BOOL bExp = (bExpand && IsExpand( theTab1, theTab2, nTab1, nDz ));
/*N*/ 			bCut1 = lcl_MoveStart( (short&) theTab1, nTab1, nDz, nMaxTab );
/*N*/ 			bCut2 = lcl_MoveEnd( (short&) theTab2, nTab1, nDz, nMaxTab );
/*N*/ 			if ( theTab2 < theTab1 )
/*N*/ 			{
/*N*/ 				eRet = UR_INVALID;
/*N*/ 				theTab2 = theTab1;
/*N*/ 			}
/*N*/ 			else if ( bCut1 || bCut2 )
/*N*/ 				eRet = UR_UPDATED;
/*N*/ 			if ( bExp )
/*N*/ 			{
/*?*/ 				Expand( (short&) theTab1, (short&) theTab2, nTab1, nDz );
/*?*/ 				eRet = UR_UPDATED;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if (eUpdateRefMode == URM_MOVE)
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if ((theCol1 >= nCol1-nDx) && (theRow1 >= nRow1-nDy) && (theTab1 >= nTab1-nDz) &&
/*N*/ 	}
/*N*/ 	else if (eUpdateRefMode == URM_REORDER)
/*N*/ 	{
/*?*/ 		//	bisher nur fuer nDz (MoveTab)
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 DBG_ASSERT ( !nDx && !nDy, "URM_REORDER fuer x und y noch nicht implementiert" );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( eRet == UR_NOTHING )
/*N*/ 	{
/*N*/ 		if (oldCol1 != theCol1
/*N*/ 		 ||	oldRow1 != theRow1
/*N*/ 		 ||	oldTab1 != theTab1
/*N*/ 		 ||	oldCol2 != theCol2
/*N*/ 		 ||	oldRow2 != theRow2
/*N*/ 		 ||	oldTab2 != theTab2
/*N*/ 			)
/*N*/ 			eRet = UR_UPDATED;
/*N*/ 	}
/*N*/ 	return eRet;
/*N*/ }


// simples UpdateReference fuer ScBigRange (ScChangeAction/ScChangeTrack)
// Referenzen koennen auch ausserhalb des Dokuments liegen!
// Ganze Spalten/Zeilen (nInt32Min..nInt32Max) bleiben immer solche!
/*N*/  ScRefUpdateRes ScRefUpdate::Update( UpdateRefMode eUpdateRefMode,
/*N*/  		const ScBigRange& rWhere, INT32 nDx, INT32 nDy, INT32 nDz,
/*N*/  		ScBigRange& rWhat )
/*N*/  {
/*N*/  	ScRefUpdateRes eRet = UR_NOTHING;
/*N*/  	const ScBigRange aOldRange( rWhat );
/*N*/  
/*N*/  	INT32 nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*N*/  	INT32 theCol1, theRow1, theTab1, theCol2, theRow2, theTab2;
/*N*/  	rWhere.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
/*N*/  	rWhat.GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 );
/*N*/  
/*N*/  	BOOL bCut1, bCut2;
/*N*/  
/*N*/  	if (eUpdateRefMode == URM_INSDEL)
/*N*/  	{
/*N*/  		if ( nDx && (theRow1 >= nRow1) && (theRow2 <= nRow2) &&
/*N*/  					(theTab1 >= nTab1) && (theTab2 <= nTab2) &&
/*N*/  					!(theCol1 == nInt32Min && theCol2 == nInt32Max) )
/*N*/  		{
/*N*/  			bCut1 = lcl_MoveBig( theCol1, nCol1, nDx );
/*N*/  			bCut2 = lcl_MoveBig( theCol2, nCol1, nDx );
/*N*/  			if ( bCut1 || bCut2 )
/*N*/  				eRet = UR_UPDATED;
/*N*/  			rWhat.aStart.SetCol( theCol1 );
/*N*/  			rWhat.aEnd.SetCol( theCol2 );
/*N*/  		}
/*N*/  		if ( nDy && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
/*N*/  					(theTab1 >= nTab1) && (theTab2 <= nTab2) &&
/*N*/  					!(theRow1 == nInt32Min && theRow2 == nInt32Max) )
/*N*/  		{
/*N*/  			bCut1 = lcl_MoveBig( theRow1, nRow1, nDy );
/*N*/  			bCut2 = lcl_MoveBig( theRow2, nRow1, nDy );
/*N*/  			if ( bCut1 || bCut2 )
/*N*/  				eRet = UR_UPDATED;
/*N*/  			rWhat.aStart.SetRow( theRow1 );
/*N*/  			rWhat.aEnd.SetRow( theRow2 );
/*N*/  		}
/*N*/  		if ( nDz && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
/*N*/  					(theRow1 >= nRow1) && (theRow2 <= nRow2) &&
/*N*/  					!(theTab1 == nInt32Min && theTab2 == nInt32Max) )
/*N*/  		{
/*N*/  			bCut1 = lcl_MoveBig( theTab1, nTab1, nDz );
/*N*/  			bCut2 = lcl_MoveBig( theTab2, nTab1, nDz );
/*N*/  			if ( bCut1 || bCut2 )
/*N*/  				eRet = UR_UPDATED;
/*N*/  			rWhat.aStart.SetTab( theTab1 );
/*N*/  			rWhat.aEnd.SetTab( theTab2 );
/*N*/  		}
/*N*/  	}
/*N*/  	else if (eUpdateRefMode == URM_MOVE)
/*N*/  	{
/*N*/  		if ( rWhere.In( rWhat ) )
/*N*/  		{
/*N*/  			if ( nDx && !(theCol1 == nInt32Min && theCol2 == nInt32Max) )
/*N*/  			{
/*N*/  				bCut1 = lcl_MoveItCutBig( theCol1, nDx );
/*N*/  				bCut2 = lcl_MoveItCutBig( theCol2, nDx );
/*N*/  				if ( bCut1 || bCut2 )
/*N*/  					eRet = UR_UPDATED;
/*N*/  				rWhat.aStart.SetCol( theCol1 );
/*N*/  				rWhat.aEnd.SetCol( theCol2 );
/*N*/  			}
/*N*/  			if ( nDy && !(theRow1 == nInt32Min && theRow2 == nInt32Max) )
/*N*/  			{
/*N*/  				bCut1 = lcl_MoveItCutBig( theRow1, nDy );
/*N*/  				bCut2 = lcl_MoveItCutBig( theRow2, nDy );
/*N*/  				if ( bCut1 || bCut2 )
/*N*/  					eRet = UR_UPDATED;
/*N*/  				rWhat.aStart.SetRow( theRow1 );
/*N*/  				rWhat.aEnd.SetRow( theRow2 );
/*N*/  			}
/*N*/  			if ( nDz && !(theTab1 == nInt32Min && theTab2 == nInt32Max) )
/*N*/  			{
/*N*/  				bCut1 = lcl_MoveItCutBig( theTab1, nDz );
/*N*/  				bCut2 = lcl_MoveItCutBig( theTab2, nDz );
/*N*/  				if ( bCut1 || bCut2 )
/*N*/  					eRet = UR_UPDATED;
/*N*/  				rWhat.aStart.SetTab( theTab1 );
/*N*/  				rWhat.aEnd.SetTab( theTab2 );
/*N*/  			}
/*N*/  		}
/*N*/  	}
/*N*/  
/*N*/  	if ( eRet == UR_NOTHING && rWhat != aOldRange )
/*N*/  		eRet = UR_UPDATED;
/*N*/  
/*N*/  	return eRet;
/*N*/  }


// vor dem Aufruf muessen die Abs-Refs aktualisiert werden!
/*N*/  ScRefUpdateRes ScRefUpdate::Update( ScDocument* pDoc, UpdateRefMode eMode,
/*N*/  									const ScAddress& rPos, const ScRange& r,
/*N*/  									short nDx, short nDy, short nDz,
/*N*/  									ComplRefData& rRef )
/*N*/  {
/*N*/  	ScRefUpdateRes eRet = UR_NOTHING;
/*N*/  
/*N*/  	short nCol1 = r.aStart.Col();
/*N*/  	short nRow1 = r.aStart.Row();
/*N*/  	short nTab1 = r.aStart.Tab();
/*N*/  	short nCol2 = r.aEnd.Col();
/*N*/  	short nRow2 = r.aEnd.Row();
/*N*/  	short nTab2 = r.aEnd.Tab();
/*N*/  
/*N*/  	if( eMode == URM_INSDEL )
/*N*/  	{
/*N*/  		BOOL bExpand = pDoc->IsExpandRefs();
/*N*/  
/*N*/  		const ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
/*N*/  		BOOL bInDeleteUndo =
/*N*/  			( pChangeTrack ? pChangeTrack->IsInDeleteUndo() : FALSE );
/*N*/  
/*N*/  		USHORT oldCol1 = rRef.Ref1.nCol;
/*N*/  		USHORT oldRow1 = rRef.Ref1.nRow;
/*N*/  		USHORT oldTab1 = rRef.Ref1.nTab;
/*N*/  		USHORT oldCol2 = rRef.Ref2.nCol;
/*N*/  		USHORT oldRow2 = rRef.Ref2.nRow;
/*N*/  		USHORT oldTab2 = rRef.Ref2.nTab;
/*N*/  
/*N*/  		BOOL bRef1ColDel = rRef.Ref1.IsColDeleted();
/*N*/  		BOOL bRef2ColDel = rRef.Ref2.IsColDeleted();
/*N*/  		BOOL bRef1RowDel = rRef.Ref1.IsRowDeleted();
/*N*/  		BOOL bRef2RowDel = rRef.Ref2.IsRowDeleted();
/*N*/  		BOOL bRef1TabDel = rRef.Ref1.IsTabDeleted();
/*N*/  		BOOL bRef2TabDel = rRef.Ref2.IsTabDeleted();
/*N*/  
/*N*/  		if( nDx &&
/*N*/  			((rRef.Ref1.nRow >= nRow1
/*N*/  		   && rRef.Ref2.nRow <= nRow2) || (bRef1RowDel || bRef2RowDel))
/*N*/  			&&
/*N*/  			((rRef.Ref1.nTab >= nTab1
/*N*/  		   && rRef.Ref2.nTab <= nTab2) || (bRef1TabDel || bRef2TabDel))
/*N*/  		   )
/*N*/  		{
/*N*/  			BOOL bExp = (bExpand && !bInDeleteUndo && IsExpand( rRef.Ref1.nCol,
/*N*/  				rRef.Ref2.nCol, nCol1, nDx ));
/*N*/  			if ( lcl_MoveRefPart( rRef.Ref1.nCol, bRef1ColDel,
/*N*/  								  rRef.Ref2.nCol, bRef2ColDel,
/*N*/  								  nCol1, nCol2, nDx, MAXCOL ) )
/*N*/  			{
/*N*/  				eRet = UR_UPDATED;
/*N*/  				if ( bInDeleteUndo && (bRef1ColDel || bRef2ColDel) )
/*N*/  				{
/*N*/  					if ( bRef1ColDel && nCol1 <= rRef.Ref1.nCol &&
/*N*/  							rRef.Ref1.nCol <= nCol1 + nDx )
/*N*/  						rRef.Ref1.SetColDeleted( FALSE );
/*N*/  					if ( bRef2ColDel && nCol1 <= rRef.Ref2.nCol &&
/*N*/  							rRef.Ref2.nCol <= nCol1 + nDx )
/*N*/  						rRef.Ref2.SetColDeleted( FALSE );
/*N*/  				}
/*N*/  				else
/*N*/  				{
/*N*/  					if ( bRef1ColDel )
/*N*/  						rRef.Ref1.SetColDeleted( TRUE );
/*N*/  					if ( bRef2ColDel )
/*N*/  						rRef.Ref2.SetColDeleted( TRUE );
/*N*/  				}
/*N*/  			}
/*N*/  			if ( bExp )
/*N*/  			{
/*N*/  				Expand( rRef.Ref1.nCol, rRef.Ref2.nCol, nCol1, nDx );
/*N*/  				eRet = UR_UPDATED;
/*N*/  			}
/*N*/  		}
/*N*/  		if( nDy &&
/*N*/  			((rRef.Ref1.nCol >= nCol1
/*N*/  		   && rRef.Ref2.nCol <= nCol2) || (bRef1ColDel || bRef2ColDel))
/*N*/  			&&
/*N*/  			((rRef.Ref1.nTab >= nTab1
/*N*/  		   && rRef.Ref2.nTab <= nTab2) || (bRef1TabDel || bRef2TabDel))
/*N*/  		   )
/*N*/  		{
/*N*/  			BOOL bExp = (bExpand && !bInDeleteUndo && IsExpand( rRef.Ref1.nRow,
/*N*/  				rRef.Ref2.nRow, nRow1, nDy ));
/*N*/  			if ( lcl_MoveRefPart( rRef.Ref1.nRow, bRef1RowDel,
/*N*/  								rRef.Ref2.nRow, bRef2RowDel,
/*N*/  								nRow1, nRow2, nDy, MAXROW ) )
/*N*/  			{
/*N*/  				eRet = UR_UPDATED;
/*N*/  				if ( bInDeleteUndo && (bRef1RowDel || bRef2RowDel) )
/*N*/  				{
/*N*/  					if ( bRef1RowDel && nRow1 <= rRef.Ref1.nRow &&
/*N*/  							rRef.Ref1.nRow <= nRow1 + nDy )
/*N*/  						rRef.Ref1.SetRowDeleted( FALSE );
/*N*/  					if ( bRef2RowDel && nRow1 <= rRef.Ref2.nRow &&
/*N*/  							rRef.Ref2.nRow <= nRow1 + nDy )
/*N*/  						rRef.Ref2.SetRowDeleted( FALSE );
/*N*/  				}
/*N*/  				else
/*N*/  				{
/*N*/  					if ( bRef1RowDel )
/*N*/  						rRef.Ref1.SetRowDeleted( TRUE );
/*N*/  					if ( bRef2RowDel )
/*N*/  						rRef.Ref2.SetRowDeleted( TRUE );
/*N*/  				}
/*N*/  			}
/*N*/  			if ( bExp )
/*N*/  			{
/*N*/  				Expand( rRef.Ref1.nRow, rRef.Ref2.nRow, nRow1, nDy );
/*N*/  				eRet = UR_UPDATED;
/*N*/  			}
/*N*/  		}
/*N*/  		if( nDz &&
/*N*/  			((rRef.Ref1.nCol >= nCol1
/*N*/  		   && rRef.Ref2.nCol <= nCol2) || (bRef1ColDel || bRef2ColDel))
/*N*/  			&&
/*N*/  			((rRef.Ref1.nRow >= nRow1
/*N*/  		   && rRef.Ref2.nRow <= nRow2) || (bRef1RowDel || bRef2RowDel))
/*N*/  		   )
/*N*/  		{
/*N*/  			BOOL bExp = (bExpand && !bInDeleteUndo && IsExpand( rRef.Ref1.nTab,
/*N*/  				rRef.Ref2.nTab, nTab1, nDz ));
/*N*/  			short nMaxTab = (short) pDoc->GetTableCount() - 1;
/*N*/  			if ( lcl_MoveRefPart( rRef.Ref1.nTab, bRef1TabDel,
/*N*/  								  rRef.Ref2.nTab, bRef2TabDel,
/*N*/  								  nTab1, nTab2, nDz, nMaxTab ) )
/*N*/  			{
/*N*/  				eRet = UR_UPDATED;
/*N*/  				if ( bInDeleteUndo && (bRef1TabDel || bRef2TabDel) )
/*N*/  				{
/*N*/  					if ( bRef1TabDel && nTab1 <= rRef.Ref1.nTab &&
/*N*/  							rRef.Ref1.nTab <= nTab1 + nDz )
/*N*/  						rRef.Ref1.SetTabDeleted( FALSE );
/*N*/  					if ( bRef2TabDel && nTab1 <= rRef.Ref2.nTab &&
/*N*/  							rRef.Ref2.nTab <= nTab1 + nDz )
/*N*/  						rRef.Ref2.SetTabDeleted( FALSE );
/*N*/  				}
/*N*/  				else
/*N*/  				{
/*N*/  					if ( bRef1TabDel )
/*N*/  						rRef.Ref1.SetTabDeleted( TRUE );
/*N*/  					if ( bRef2TabDel )
/*N*/  						rRef.Ref2.SetTabDeleted( TRUE );
/*N*/  				}
/*N*/  			}
/*N*/  			if ( bExp )
/*N*/  			{
/*N*/  				Expand( rRef.Ref1.nTab, rRef.Ref2.nTab, nTab1, nDz );
/*N*/  				eRet = UR_UPDATED;
/*N*/  			}
/*N*/  		}
/*N*/  		if ( eRet == UR_NOTHING )
/*N*/  		{
/*N*/  			if (oldCol1 != rRef.Ref1.nCol
/*N*/  			 ||	oldRow1 != rRef.Ref1.nRow
/*N*/  			 ||	oldTab1 != rRef.Ref1.nTab
/*N*/  			 ||	oldCol2 != rRef.Ref2.nCol
/*N*/  			 ||	oldRow2 != rRef.Ref2.nRow
/*N*/  			 ||	oldTab2 != rRef.Ref2.nTab
/*N*/  				)
/*N*/  				eRet = UR_UPDATED;
/*N*/  		}
/*N*/  		rRef.CalcRelFromAbs( rPos );
/*N*/  	}
/*N*/  	else
/*N*/  	{
/*N*/  		if( eMode == URM_MOVE )
/*N*/  		{
/*N*/  			if ( rRef.Ref1.nCol >= nCol1-nDx
/*N*/  			  && rRef.Ref1.nRow >= nRow1-nDy
/*N*/  			  && rRef.Ref1.nTab >= nTab1-nDz
/*N*/  			  && rRef.Ref2.nCol <= nCol2-nDx
/*N*/  			  && rRef.Ref2.nRow <= nRow2-nDy
/*N*/  			  && rRef.Ref2.nTab <= nTab2-nDz )
/*N*/  			{
/*N*/  				eRet = Move( pDoc, rPos, nDx, nDy, nDz, rRef, FALSE, TRUE );		// immer verschieben
/*N*/  			}
/*N*/  			else if ( nDz && r.In( rPos ) )
/*N*/  			{
/*N*/  				rRef.Ref1.SetFlag3D( TRUE );
/*N*/  				rRef.Ref2.SetFlag3D( TRUE );
/*N*/  				eRet = UR_UPDATED;
/*N*/  				rRef.CalcRelFromAbs( rPos );
/*N*/  			}
/*N*/  			else
/*N*/  				rRef.CalcRelFromAbs( rPos );
/*N*/  		}
/*N*/  		else if( eMode == URM_COPY && r.In( rPos ) )
/*N*/  			eRet = Move( pDoc, rPos, nDx, nDy, nDz, rRef, FALSE, FALSE );		// nur relative
/*N*/  			// sollte nicht mehr verwendet werden muessen
/*N*/  		else
/*N*/  			rRef.CalcRelFromAbs( rPos );
/*N*/  	}
/*N*/  	return eRet;
/*N*/  }

// vor dem Aufruf muessen die Abs-Refs aktualisiert werden!
/*N*/  ScRefUpdateRes ScRefUpdate::Move( ScDocument* pDoc, const ScAddress& rPos,
/*N*/  								  short nDx, short nDy, short nDz,
/*N*/  								  ComplRefData& rRef, BOOL bWrap, BOOL bAbsolute )
/*N*/  {
/*N*/  	ScRefUpdateRes eRet = UR_NOTHING;
/*N*/  
/*N*/  	USHORT oldCol1 = rRef.Ref1.nCol;
/*N*/  	USHORT oldRow1 = rRef.Ref1.nRow;
/*N*/  	USHORT oldTab1 = rRef.Ref1.nTab;
/*N*/  	USHORT oldCol2 = rRef.Ref2.nCol;
/*N*/  	USHORT oldRow2 = rRef.Ref2.nRow;
/*N*/  	USHORT oldTab2 = rRef.Ref2.nTab;
/*N*/  
/*N*/  	BOOL bCut1, bCut2;
/*N*/  	if ( nDx )
/*N*/  	{
/*N*/  		bCut1 = bCut2 = FALSE;
/*N*/  		if( bAbsolute || rRef.Ref1.IsColRel() )
/*N*/  		{
/*N*/  			if( bWrap )
/*N*/  				lcl_MoveItWrap( (short&) rRef.Ref1.nCol, nDx, MAXCOL );
/*N*/  			else
/*N*/  				bCut1 = lcl_MoveItCut( (short&) rRef.Ref1.nCol, nDx, MAXCOL );
/*N*/  		}
/*N*/  		if( bAbsolute || rRef.Ref2.IsColRel() )
/*N*/  		{
/*N*/  			if( bWrap )
/*N*/  				lcl_MoveItWrap( (short&) rRef.Ref2.nCol, nDx, MAXCOL );
/*N*/  			else
/*N*/  				bCut2 = lcl_MoveItCut( (short&) rRef.Ref2.nCol, nDx, MAXCOL );
/*N*/  		}
/*N*/  		if ( bCut1 || bCut2 )
/*N*/  			eRet = UR_UPDATED;
/*N*/  		if ( bCut1 && bCut2 )
/*N*/  		{
/*N*/  			rRef.Ref1.SetColDeleted( TRUE );
/*N*/  			rRef.Ref2.SetColDeleted( TRUE );
/*N*/  		}
/*N*/  	}
/*N*/  	if ( nDy )
/*N*/  	{
/*N*/  		bCut1 = bCut2 = FALSE;
/*N*/  		if( bAbsolute || rRef.Ref1.IsRowRel() )
/*N*/  		{
/*N*/  			if( bWrap )
/*N*/  				lcl_MoveItWrap( (short&) rRef.Ref1.nRow, nDy, MAXROW );
/*N*/  			else
/*N*/  				bCut1 = lcl_MoveItCut( (short&) rRef.Ref1.nRow, nDy, MAXROW );
/*N*/  		}
/*N*/  		if( bAbsolute || rRef.Ref2.IsRowRel() )
/*N*/  		{
/*N*/  			if( bWrap )
/*N*/  				lcl_MoveItWrap( (short&) rRef.Ref2.nRow, nDy, MAXROW );
/*N*/  			else
/*N*/  				bCut2 = lcl_MoveItCut( (short&) rRef.Ref2.nRow, nDy, MAXROW );
/*N*/  		}
/*N*/  		if ( bCut1 || bCut2 )
/*N*/  			eRet = UR_UPDATED;
/*N*/  		if ( bCut1 && bCut2 )
/*N*/  		{
/*N*/  			rRef.Ref1.SetRowDeleted( TRUE );
/*N*/  			rRef.Ref2.SetRowDeleted( TRUE );
/*N*/  		}
/*N*/  	}
/*N*/  	if ( nDz )
/*N*/  	{
/*N*/  		bCut1 = bCut2 = FALSE;
/*N*/  		short nMaxTab = (short) pDoc->GetTableCount() - 1;
/*N*/  		if( bAbsolute || rRef.Ref1.IsTabRel() )
/*N*/  		{
/*N*/  			if( bWrap )
/*N*/  				lcl_MoveItWrap( (short&) rRef.Ref1.nTab, nDz, nMaxTab );
/*N*/  			else
/*N*/  				bCut1 = lcl_MoveItCut( (short&) rRef.Ref1.nTab, nDz, nMaxTab );
/*N*/  			rRef.Ref1.SetFlag3D( rPos.Tab() != rRef.Ref1.nTab );
/*N*/  		}
/*N*/  		if( bAbsolute || rRef.Ref2.IsTabRel() )
/*N*/  		{
/*N*/  			if( bWrap )
/*N*/  				lcl_MoveItWrap( (short&) rRef.Ref2.nTab, nDz, nMaxTab );
/*N*/  			else
/*N*/  				bCut2 = lcl_MoveItCut( (short&) rRef.Ref2.nTab, nDz, nMaxTab );
/*N*/  			rRef.Ref2.SetFlag3D( rPos.Tab() != rRef.Ref2.nTab );
/*N*/  		}
/*N*/  		if ( bCut1 || bCut2 )
/*N*/  			eRet = UR_UPDATED;
/*N*/  		if ( bCut1 && bCut2 )
/*N*/  		{
/*N*/  			rRef.Ref1.SetTabDeleted( TRUE );
/*N*/  			rRef.Ref2.SetTabDeleted( TRUE );
/*N*/  		}
/*N*/  	}
/*N*/  
/*N*/  	if ( eRet == UR_NOTHING )
/*N*/  	{
/*N*/  		if (oldCol1 != rRef.Ref1.nCol
/*N*/  		 ||	oldRow1 != rRef.Ref1.nRow
/*N*/  		 ||	oldTab1 != rRef.Ref1.nTab
/*N*/  		 ||	oldCol2 != rRef.Ref2.nCol
/*N*/  		 ||	oldRow2 != rRef.Ref2.nRow
/*N*/  		 ||	oldTab2 != rRef.Ref2.nTab
/*N*/  			)
/*N*/  			eRet = UR_UPDATED;
/*N*/  	}
/*N*/  	if ( bWrap && eRet != UR_NOTHING )
/*N*/  		rRef.PutInOrder();
/*N*/  	rRef.CalcRelFromAbs( rPos );
/*N*/  	return eRet;
/*N*/  }

/*N*/ void ScRefUpdate::MoveRelWrap( ScDocument* pDoc, const ScAddress& rPos,
/*N*/ 								  ComplRefData& rRef )
/*N*/ {
/*N*/ 	if( rRef.Ref1.IsColRel() )
/*N*/ 	{
/*N*/ 		 rRef.Ref1.nCol = rRef.Ref1.nRelCol + rPos.Col();
/*N*/ 		lcl_MoveItWrap( (short&) rRef.Ref1.nCol, 0, MAXCOL );
/*N*/ 	}
/*N*/ 	if( rRef.Ref2.IsColRel() )
/*N*/ 	{
/*N*/		rRef.Ref2.nCol = rRef.Ref2.nRelCol + rPos.Col();
/*N*/ 		lcl_MoveItWrap( (short&) rRef.Ref2.nCol, 0, MAXCOL );
/*N*/ 	}
/*N*/ 	if( rRef.Ref1.IsRowRel() )
/*N*/ 	{
/*N*/ 		rRef.Ref1.nRow = rRef.Ref1.nRelRow + rPos.Row();
/*N*/ 		lcl_MoveItWrap( (short&) rRef.Ref1.nRow, 0, MAXROW );
/*N*/ 	}
/*N*/ 	if( rRef.Ref2.IsRowRel() )
/*N*/ 	{
/*N*/ 		rRef.Ref2.nRow = rRef.Ref2.nRelRow + rPos.Row();
/*N*/ 		lcl_MoveItWrap( (short&) rRef.Ref2.nRow, 0, MAXROW );
/*N*/ 	}
/*?*/ 	short nMaxTab = (short) pDoc->GetTableCount() - 1;
/*?*/ 	if( rRef.Ref1.IsTabRel() )
/*N*/ 	{
/*N*/ 		 rRef.Ref1.nTab = rRef.Ref1.nRelTab + rPos.Tab();
/*N*/ 		lcl_MoveItWrap( (short&) rRef.Ref1.nTab, 0, nMaxTab );
/*N*/ 	}
/*N*/ 	if( rRef.Ref2.IsTabRel() )
/*N*/ 	{
/*N*/ 		 rRef.Ref2.nTab = rRef.Ref2.nRelTab + rPos.Tab();
/*N*/ 		lcl_MoveItWrap( (short&) rRef.Ref2.nTab, 0, nMaxTab );
/*N*/ 	}
/*N*/ 	rRef.PutInOrder();
/*N*/ 	rRef.CalcRelFromAbs( rPos );
/*N*/ }
}
