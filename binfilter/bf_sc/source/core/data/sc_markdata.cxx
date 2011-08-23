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

// INCLUDE ---------------------------------------------------------------

#include <tools/debug.hxx>

#include "markdata.hxx"
#include "markarr.hxx"
#include "rangelst.hxx"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

//------------------------------------------------------------------------

/*N*/ ScMarkData::ScMarkData() :
/*N*/ 	pMultiSel( NULL )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 		bTabMarked[i] = FALSE;
/*N*/ 
/*N*/ 	ResetMark();
/*N*/ }

/*N*/ ScMarkData::ScMarkData(const ScMarkData& rData) :
/*N*/ 	pMultiSel( NULL ),
/*N*/ 	aMarkRange( rData.aMarkRange ),
/*N*/ 	aMultiRange( rData.aMultiRange )
/*N*/ {
/*N*/ 	bMarked		 = rData.bMarked;
/*N*/ 	bMultiMarked = rData.bMultiMarked;
/*N*/ 	bMarking	 = rData.bMarking;
/*N*/ 	bMarkIsNeg	 = rData.bMarkIsNeg;
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	for (i=0; i<=MAXTAB; i++)
/*N*/ 		bTabMarked[i] = rData.bTabMarked[i];
/*N*/ 
/*N*/ 	if (rData.pMultiSel)
/*N*/ 	{
/*N*/ 		pMultiSel = new ScMarkArray[MAXCOL+1];
/*N*/ 		for (i=0; i<=MAXCOL; i++)
/*N*/ 			rData.pMultiSel[i].CopyMarksTo( pMultiSel[i] );
/*N*/ 	}
/*N*/ }


/*N*/ ScMarkData::~ScMarkData()
/*N*/ {
/*N*/ 	delete[] pMultiSel;
/*N*/ }

/*N*/ void ScMarkData::ResetMark()
/*N*/ {
/*N*/ 	delete[] pMultiSel;
/*N*/ 	pMultiSel = NULL;
/*N*/ 
/*N*/ 	bMarked = bMultiMarked = FALSE;
/*N*/ 	bMarking = bMarkIsNeg = FALSE;
/*N*/ }

/*N*/ void ScMarkData::SetMarkArea( const ScRange& rRange )
/*N*/ {
/*N*/ 	aMarkRange = rRange;
/*N*/ 	aMarkRange.Justify();
/*N*/ 	if ( !bMarked )
/*N*/ 	{
/*N*/ 		// #77987# Upon creation of a document ScFormatShell GetTextAttrState
/*N*/ 		// may query (default) attributes although no sheet is marked yet.
/*N*/ 		// => mark that one.
/*N*/ 		if ( !GetSelectCount() )
/*N*/ 			bTabMarked[ aMarkRange.aStart.Tab() ] = TRUE;
/*N*/ 		bMarked = TRUE;
/*N*/ 	}
/*N*/ }

/*N*/ void ScMarkData::GetMarkArea( ScRange& rRange ) const
/*N*/ {
/*N*/ 	rRange = aMarkRange;		//! inline ?
/*N*/ }

/*N*/ void ScMarkData::GetMultiMarkArea( ScRange& rRange ) const
/*N*/ {
/*N*/ 	rRange = aMultiRange;
/*N*/ }

/*N*/ void ScMarkData::SetMultiMarkArea( const ScRange& rRange, BOOL bMark )
/*N*/ {
/*N*/ 	if (!pMultiSel)
/*N*/ 	{
/*N*/ 		pMultiSel = new ScMarkArray[MAXCOL+1];
/*N*/ 
/*N*/ 		// if simple mark range is set, copy to multi marks
/*N*/ 		if ( bMarked && !bMarkIsNeg )
/*N*/ 		{
/*N*/ 			bMarked = FALSE;
/*N*/ 			SetMultiMarkArea( aMarkRange, TRUE );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/ 	USHORT nEndCol = rRange.aEnd.Col();
/*N*/ 	USHORT nEndRow = rRange.aEnd.Row();
/*N*/ 	PutInOrder( nStartRow, nEndRow );
/*N*/ 	PutInOrder( nStartCol, nEndCol );
/*N*/ 
/*N*/ 	USHORT nCol;
/*N*/ 	for (nCol=nStartCol; nCol<=nEndCol; nCol++)
/*N*/ 		pMultiSel[nCol].SetMarkArea( nStartRow, nEndRow, bMark );
/*N*/ 
/*N*/ 	if ( bMultiMarked )					// aMultiRange updaten
/*N*/ 	{
/*N*/ 		if ( nStartCol < aMultiRange.aStart.Col() )
/*N*/ 			aMultiRange.aStart.SetCol( nStartCol );
/*N*/ 		if ( nStartRow < aMultiRange.aStart.Row() )
/*N*/ 			aMultiRange.aStart.SetRow( nStartRow );
/*N*/ 		if ( nEndCol > aMultiRange.aEnd.Col() )
/*N*/ 			aMultiRange.aEnd.SetCol( nEndCol );
/*N*/ 		if ( nEndRow > aMultiRange.aEnd.Row() )
/*N*/ 			aMultiRange.aEnd.SetRow( nEndRow );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aMultiRange = rRange;			// neu
/*N*/ 		bMultiMarked = TRUE;
/*N*/ 	}
/*N*/ }


/*N*/ void ScMarkData::SelectOneTable( USHORT nTab )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 		bTabMarked[i] = ( nTab == i );
/*N*/ }

/*N*/ USHORT ScMarkData::GetSelectCount() const
/*N*/ {
/*N*/ 	USHORT nCount = 0;
/*N*/ 	for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 		if (bTabMarked[i])
/*N*/ 			++nCount;
/*N*/ 
/*N*/ 	return nCount;
/*N*/ }

/*N*/ void ScMarkData::MarkToMulti()
/*N*/ {
/*N*/ 	if ( bMarked && !bMarking )
/*N*/ 	{
/*N*/ 		SetMultiMarkArea( aMarkRange, !bMarkIsNeg );
/*N*/ 		bMarked = FALSE;
/*N*/ 
/*N*/ 		//	check if all multi mark ranges have been removed
/*N*/ 		if ( bMarkIsNeg && !HasAnyMultiMarks() )
/*?*/ 			ResetMark();
/*N*/ 	}
/*N*/ }

/*N*/ void ScMarkData::MarkToSimple()
/*N*/ {
/*N*/ 	if ( bMarking )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if ( bMultiMarked && bMarked )
/*?*/ 		MarkToMulti();					// may result in bMarked and bMultiMarked reset
/*N*/ 
/*N*/ 	if ( bMultiMarked )
/*N*/ 	{
/*N*/ 		DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");
/*N*/ 
/*N*/ 		ScRange aNew = aMultiRange;
/*N*/ 
/*N*/ 		BOOL bOk = FALSE;
/*N*/ 		USHORT nStartCol = aNew.aStart.Col();
/*N*/ 		USHORT nEndCol   = aNew.aEnd.Col();
/*N*/ 
/*N*/ 		while ( nStartCol < nEndCol && !pMultiSel[nStartCol].HasMarks() )
/*N*/ 			++nStartCol;
/*N*/ 		while ( nStartCol < nEndCol && !pMultiSel[nEndCol].HasMarks() )
/*N*/ 			--nEndCol;
/*N*/ 
/*N*/ 		//	Zeilen werden nur aus MarkArray genommen
/*N*/ 		USHORT nStartRow, nEndRow;
/*N*/ 		if ( pMultiSel[nStartCol].HasOneMark( nStartRow, nEndRow ) )
/*N*/ 		{
/*N*/ 			bOk = TRUE;
/*N*/ 			USHORT nCmpStart, nCmpEnd;
/*N*/ 			for (USHORT nCol=nStartCol+1; nCol<=nEndCol && bOk; nCol++)
/*N*/ 				if ( !pMultiSel[nCol].HasOneMark( nCmpStart, nCmpEnd )
/*N*/ 						|| nCmpStart != nStartRow || nCmpEnd != nEndRow )
/*N*/ 					bOk = FALSE;
/*N*/ 		}
/*N*/ 
/*N*/ 		if (bOk)
/*N*/ 		{
/*?*/ 			aNew.aStart.SetCol(nStartCol);
/*?*/ 			aNew.aStart.SetRow(nStartRow);
/*?*/ 			aNew.aEnd.SetCol(nEndCol);
/*?*/ 			aNew.aEnd.SetRow(nEndRow);
/*?*/ 
/*?*/ 			ResetMark();
/*?*/ 			aMarkRange = aNew;
/*?*/ 			bMarked = TRUE;
/*?*/ 			bMarkIsNeg = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScMarkData::IsCellMarked( USHORT nCol, USHORT nRow, BOOL bNoSimple ) const
/*N*/ {
/*N*/ 	if ( bMarked && !bNoSimple && !bMarkIsNeg )
/*?*/ 		if ( aMarkRange.aStart.Col() <= nCol && aMarkRange.aEnd.Col() >= nCol &&
/*?*/ 			 aMarkRange.aStart.Row() <= nRow && aMarkRange.aEnd.Row() >= nRow )
/*?*/ 			return TRUE;
/*N*/ 
/*N*/ 	if (bMultiMarked)
/*N*/ 	{
/*N*/ 		//!	hier auf negative Markierung testen ?
/*N*/ 
/*N*/ 		DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");
/*N*/ 		return pMultiSel[nCol].GetMark( nRow );
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }



/*N*/ void ScMarkData::MarkFromRangeList( const ScRangeList& rList, BOOL bReset )
/*N*/ {
/*N*/ 	if (bReset)
/*N*/ 	{
/*?*/ 		for (USHORT i=0; i<=MAXTAB; i++)
/*?*/ 			bTabMarked[i] = FALSE;				// Tabellen sind nicht in ResetMark
/*?*/ 		ResetMark();
/*N*/ 	}
/*N*/ 
/*N*/ 	ULONG nCount = rList.Count();
/*N*/ 	if ( nCount == 1 && !bMarked && !bMultiMarked )
/*N*/ 	{
/*N*/ 		ScRange aRange = *rList.GetObject(0);
/*N*/ 		SetMarkArea( aRange );
/*N*/ 		SelectTable( aRange.aStart.Tab(), TRUE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for (ULONG i=0; i<nCount; i++)
/*N*/ 		{
/*N*/ 			ScRange aRange = *rList.GetObject(i);
/*N*/ 			SetMultiMarkArea( aRange, TRUE );
/*N*/ 			SelectTable( aRange.aStart.Tab(), TRUE );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScMarkData::FillRangeListWithMarks( ScRangeList* pList, BOOL bClear ) const
/*N*/ {
/*N*/ 	if (!pList)
/*N*/ 		return;
/*N*/ 
/*N*/ 	if (bClear)
/*?*/ 		pList->RemoveAll();
/*N*/ 
/*N*/ 	//!		bei mehreren selektierten Tabellen mehrere Ranges eintragen !!!
/*N*/ 
/*N*/ 	if ( bMultiMarked )
/*N*/ 	{
/*N*/ 		DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");
/*N*/ 
/*N*/ 		USHORT nTab = aMultiRange.aStart.Tab();
/*N*/ 
/*N*/ 		USHORT nStartCol = aMultiRange.aStart.Col();
/*N*/ 		USHORT nEndCol = aMultiRange.aEnd.Col();
/*N*/ 		for (USHORT nCol=nStartCol; nCol<=nEndCol; nCol++)
/*N*/ 			if (pMultiSel[nCol].HasMarks())
/*N*/ 			{
/*N*/ 				USHORT nTop, nBottom;
/*N*/ 				ScRange aRange( nCol, 0, nTab );
/*N*/ 				ScMarkArrayIter aMarkIter( &pMultiSel[nCol] );
/*N*/ 				while ( aMarkIter.Next( nTop, nBottom ) )
/*N*/ 				{
/*N*/ 					aRange.aStart.SetRow( nTop );
/*N*/ 					aRange.aEnd.SetRow( nBottom );
/*N*/ 					pList->Join( aRange );
/*N*/ 				}
/*N*/ 			}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bMarked )
/*?*/ 		pList->Append( aMarkRange );
/*N*/ }

/*N*/ BOOL ScMarkData::IsAllMarked( const ScRange& rRange ) const
/*N*/ {
/*N*/ 	if ( !bMultiMarked )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");
/*N*/ 
/*N*/ 	USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/ 	USHORT nEndCol = rRange.aEnd.Col();
/*N*/ 	USHORT nEndRow = rRange.aEnd.Row();
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	for (USHORT nCol=nStartCol; nCol<=nEndCol && bOk; nCol++)
/*N*/ 		if ( !pMultiSel[nCol].IsAllMarked( nStartRow, nEndRow ) )
/*N*/ 			bOk = FALSE;
/*N*/ 
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScMarkData::HasMultiMarks( USHORT nCol ) const
/*N*/ {
/*N*/ 	if ( !bMultiMarked )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");
/*N*/ 
/*N*/ 	return pMultiSel[nCol].HasMarks();
/*N*/ }

/*N*/ BOOL ScMarkData::HasAnyMultiMarks() const
/*N*/ {
/*N*/ 	if ( !bMultiMarked )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");
/*N*/ 
/*N*/ 	for (USHORT nCol=0; nCol<=MAXCOL; nCol++)
/*N*/ 		if ( pMultiSel[nCol].HasMarks() )
/*N*/ 			return TRUE;
/*N*/ 
/*N*/ 	return FALSE;		// nix
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
