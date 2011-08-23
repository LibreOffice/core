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

#include <tools/debug.hxx>

#include "markarr.hxx"
#include "global.hxx"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

//------------------------------------------------------------------------

/*N*/ ScMarkArray::ScMarkArray()
/*N*/ {
/*N*/     nCount = nLimit = 1;
/*N*/ 	pData = new ScMarkEntry[1];
/*N*/ 	if (pData)
/*N*/ 	{
/*N*/ 		pData[0].nRow = MAXROW;
/*N*/ 		pData[0].bMarked = FALSE;
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScMarkArray::~ScMarkArray()
/*N*/ {
/*N*/ 	if (pData)
/*N*/ 		delete[] pData;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScMarkArray::Reset( BOOL bMarked )
/*N*/ {
/*N*/ 	if (pData)
/*N*/ 	{
/*N*/ 		delete[] pData;
/*N*/ 
/*N*/         nCount = nLimit = 1;
/*N*/ 		pData = new ScMarkEntry[1];
/*N*/ 		if (pData)
/*N*/ 		{
/*N*/ 			pData[0].nRow = MAXROW;
/*N*/ 			pData[0].bMarked = bMarked;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScMarkArray::Search( USHORT nRow, short& nIndex ) const
/*N*/ {
/*N*/ 	short	nLo 		= 0;
/*N*/ 	short	nHi 		= ((short) nCount) - 1;
/*N*/ 	short	nStartRow	= 0;
/*N*/ 	short	nEndRow 	= 0;
/*N*/ 	short	i			= 0;
/*N*/ 	BOOL	bFound		= (nCount == 1);
/*N*/ 	if (pData)
/*N*/ 	{
/*N*/ 		while ( !bFound && nLo <= nHi )
/*N*/ 		{
/*N*/ 			i = (nLo + nHi) / 2;
/*N*/ 			if (i > 0)
/*N*/ 				nStartRow = (short) pData[i - 1].nRow;
/*N*/ 			else
/*N*/ 				nStartRow = -1;
/*N*/ 			nEndRow = (short) pData[i].nRow;
/*N*/ 			if (nEndRow < (short) nRow)
/*N*/ 				nLo = ++i;
/*N*/ 			else
/*N*/ 				if (nStartRow >= (short) nRow)
/*N*/ 					nHi = --i;
/*N*/ 				else
/*N*/ 					bFound = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bFound = FALSE;
/*N*/ 
/*N*/ 	if (bFound)
/*N*/ 		nIndex=i;
/*N*/ 	else
/*N*/ 		nIndex=0;
/*N*/ 	return bFound;
/*N*/ }

/*N*/ BOOL ScMarkArray::GetMark( USHORT nRow ) const
/*N*/ {
/*N*/ 	short i;
/*N*/ 	if (Search( nRow, i ))
/*N*/ 		return pData[i].bMarked;
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ }

//------------------------------------------------------------------------


/*N*/ void ScMarkArray::SetMarkArea( USHORT nStartRow, USHORT nEndRow, BOOL bMarked )
/*N*/ {
/*N*/ 	if ((nStartRow >= 0 && nStartRow <= MAXROW) && (nEndRow >= 0 && nEndRow <= MAXROW))
/*N*/ 	{
/*N*/ 		if ((nStartRow == 0) && (nEndRow == MAXROW))
/*N*/ 		{
/*N*/ 			Reset(bMarked);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/             USHORT nNeeded = nCount + 2;
/*N*/             if ( nLimit < nNeeded )
/*N*/             {
/*N*/                 nLimit += SC_MARKARRAY_DELTA;
/*N*/                 if ( nLimit < nNeeded )
/*N*/                     nLimit = nNeeded;
/*N*/                 ScMarkEntry* pNewData = new ScMarkEntry[nLimit];
/*N*/                 memcpy( pNewData, pData, nCount*sizeof(ScMarkEntry) );
/*N*/ 				delete[] pData;
/*N*/ 				pData = pNewData;
/*N*/             }
/*N*/ 
/*N*/             USHORT ni;          // number of entries in beginning
/*N*/             USHORT nInsert;     // insert position (MAXROW+1 := no insert)
/*N*/             BOOL bCombined = FALSE;
/*N*/             BOOL bSplit = FALSE;
/*N*/             if ( nStartRow > 0 )
/*N*/             {
/*N*/                 // skip beginning
/*N*/                 short nIndex;
/*N*/                 Search( nStartRow, nIndex );
/*N*/                 ni = nIndex;
/*N*/ 
/*N*/                 nInsert = MAXROW+1;
/*N*/                 if ( pData[ni].bMarked != bMarked )
/*N*/                 {
/*N*/                     if ( ni == 0 || (pData[ni-1].nRow < nStartRow - 1) )
/*N*/                     {   // may be a split or a simple insert or just a shrink,
/*N*/                         // row adjustment is done further down
/*N*/                         if ( pData[ni].nRow > nEndRow )
/*N*/                             bSplit = TRUE;
/*N*/                         ni++;
/*N*/                         nInsert = ni;
/*N*/                     }
/*N*/                     else if ( ni > 0 && pData[ni-1].nRow == nStartRow - 1 )
/*N*/                         nInsert = ni;
/*N*/                 }
/*N*/                 if ( ni > 0 && pData[ni-1].bMarked == bMarked )
/*N*/                 {   // combine
/*N*/                     pData[ni-1].nRow = nEndRow;
/*N*/                     nInsert = MAXROW+1;
/*N*/                     bCombined = TRUE;
/*N*/                 }
/*N*/             }
/*N*/             else
/*N*/                 nInsert = ni = 0;
/*N*/ 
/*N*/             USHORT nj = ni;     // stop position of range to replace
/*N*/             while ( nj < nCount && pData[nj].nRow <= nEndRow )
/*N*/                 nj++;
/*N*/             if ( !bSplit )
/*N*/             {
/*N*/                 if ( nj < nCount && pData[nj].bMarked == bMarked )
/*N*/                 {   // combine
/*N*/                     if ( ni > 0 )
/*N*/                     {
/*N*/                         if ( pData[ni-1].bMarked == bMarked )
/*N*/                         {   // adjacent entries
/*N*/                             pData[ni-1].nRow = pData[nj].nRow;
/*N*/                             nj++;
/*N*/                         }
/*N*/                         else if ( ni == nInsert )
/*N*/                             pData[ni-1].nRow = nStartRow - 1;   // shrink
/*N*/                     }
/*N*/                     nInsert = MAXROW+1;
/*N*/                     bCombined = TRUE;
/*N*/                 }
/*N*/                 else if ( ni > 0 && ni == nInsert )
/*N*/                     pData[ni-1].nRow = nStartRow - 1;   // shrink
/*N*/             }
/*N*/             if ( ni < nj )
/*N*/             {   // remove middle entries
/*N*/                 if ( !bCombined )
/*N*/                 {   // replace one entry
/*N*/                     pData[ni].nRow = nEndRow;
/*N*/                     pData[ni].bMarked = bMarked;
/*N*/                     ni++;
/*N*/                     nInsert = MAXROW+1;
/*N*/                 }
/*N*/                 if ( ni < nj )
/*N*/                 {   // remove entries
/*N*/                     memmove( pData + ni, pData + nj, (nCount - nj) * sizeof(ScMarkEntry) );
/*N*/                     nCount -= nj - ni;
/*N*/                 }
/*N*/             }
/*N*/ 
/*N*/             if ( nInsert <= MAXROW )
/*N*/             {   // insert or append new entry
/*N*/                 if ( nInsert <= nCount )
/*N*/                 {
/*N*/                     if ( !bSplit )
/*N*/                         memmove( pData + nInsert + 1, pData + nInsert,
/*N*/                             (nCount - nInsert) * sizeof(ScMarkEntry) );
/*N*/                     else
/*N*/                     {
/*N*/                         memmove( pData + nInsert + 2, pData + nInsert,
/*N*/                             (nCount - nInsert) * sizeof(ScMarkEntry) );
/*N*/                         pData[nInsert+1] = pData[nInsert-1];
/*N*/                         nCount++;
/*N*/                     }
/*N*/                 }
/*N*/                 if ( nInsert )
/*N*/                     pData[nInsert-1].nRow = nStartRow - 1;
/*N*/                 pData[nInsert].nRow = nEndRow;
/*N*/                 pData[nInsert].bMarked = bMarked;
/*N*/                 nCount++;
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ //	InfoBox(0, String(nCount) + String(" Eintraege") ).Execute();
/*N*/ }


/*N*/ BOOL ScMarkArray::IsAllMarked( USHORT nStartRow, USHORT nEndRow ) const
/*N*/ {
/*N*/ 	short nStartIndex;
/*N*/ 	short nEndIndex;
/*N*/ 
/*N*/ 	if (Search( nStartRow, nStartIndex ))
/*N*/ 		if (pData[nStartIndex].bMarked)
/*N*/ 			if (Search( nEndRow, nEndIndex ))
/*N*/ 				if (nEndIndex==nStartIndex)
/*N*/ 					return TRUE;
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScMarkArray::HasOneMark( USHORT& rStartRow, USHORT& rEndRow ) const
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	if ( nCount == 1 )
/*N*/ 	{
/*N*/ 		if ( pData[0].bMarked )
/*N*/ 		{
/*N*/ 			rStartRow = 0;
/*N*/ 			rEndRow = MAXROW;
/*N*/ 			bRet = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( nCount == 2 )
/*N*/ 	{
/*N*/ 		if ( pData[0].bMarked )
/*N*/ 		{
/*N*/ 			rStartRow = 0;
/*N*/ 			rEndRow = pData[0].nRow;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			rStartRow = pData[0].nRow + 1;
/*N*/ 			rEndRow = MAXROW;
/*N*/ 		}
/*N*/ 		bRet = TRUE;
/*N*/ 	}
/*N*/ 	else if ( nCount == 3 )
/*N*/ 	{
/*N*/ 		if ( pData[1].bMarked )
/*N*/ 		{
/*N*/ 			rStartRow = pData[0].nRow + 1;
/*N*/ 			rEndRow = pData[1].nRow;
/*N*/ 			bRet = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ BOOL ScMarkArray::HasMarks() const
/*N*/ {
/*N*/ 	return ( nCount > 1 || pData[0].bMarked );
/*N*/ }



/*N*/ void ScMarkArray::CopyMarksTo( ScMarkArray& rDestMarkArray ) const
/*N*/ {
/*N*/ 	delete[] rDestMarkArray.pData;
/*N*/ 
/*N*/ 	if (pData)
/*N*/ 	{
/*N*/ 		rDestMarkArray.pData = new ScMarkEntry[nCount];
/*N*/ 		memmove( rDestMarkArray.pData, pData, nCount * sizeof(ScMarkEntry) );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rDestMarkArray.pData = NULL;
/*N*/ 
/*N*/     rDestMarkArray.nCount = rDestMarkArray.nLimit = nCount;
/*N*/ }

/*N*/ short ScMarkArray::GetNextMarked( short nRow, BOOL bUp ) const
/*N*/ {
/*N*/ 	short nRet = nRow;
/*N*/ 	if (VALIDROW(nRow))
/*N*/ 	{
/*N*/ 		short nIndex;
/*N*/ 		Search(nRow, nIndex);
/*N*/ 		if (!pData[nIndex].bMarked)
/*N*/ 		{
/*N*/ 			if (bUp)
/*N*/ 			{
/*N*/ 				if (nIndex>0)
/*N*/ 					nRet = pData[nIndex-1].nRow;
/*N*/ 				else
/*N*/ 					nRet = -1;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nRet = pData[nIndex].nRow + 1;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*N*/ USHORT ScMarkArray::GetMarkEnd( USHORT nRow, BOOL bUp ) const
/*N*/ {
/*N*/ 	USHORT nRet;
/*N*/ 	short nIndex;
/*N*/ 	Search(nRow, nIndex);
/*N*/ 	DBG_ASSERT( pData[nIndex].bMarked, "GetMarkEnd ohne bMarked" );
/*N*/ 	if (bUp)
/*N*/ 	{
/*N*/ 		if (nIndex>0)
/*N*/ 			nRet = pData[nIndex-1].nRow + 1;
/*N*/ 		else
/*N*/ 			nRet = 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nRet = pData[nIndex].nRow;
/*N*/ 
/*N*/ 	return nRet;
/*N*/ }

//
//	-------------- Iterator ----------------------------------------------
//

/*N*/ ScMarkArrayIter::ScMarkArrayIter( const ScMarkArray* pNewArray ) :
/*N*/ 	pArray( pNewArray ),
/*N*/ 	nPos( 0 )
/*N*/ {
/*N*/ }

/*N*/ ScMarkArrayIter::~ScMarkArrayIter()
/*N*/ {
/*N*/ }

/*N*/ BOOL ScMarkArrayIter::Next( USHORT& rTop, USHORT& rBottom )
/*N*/ {
/*N*/ 	if ( nPos >= pArray->nCount )
/*N*/ 		return FALSE;
/*N*/ 	while (!pArray->pData[nPos].bMarked)
/*N*/ 	{
/*N*/ 		++nPos;
/*N*/ 		if ( nPos >= pArray->nCount )
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ 	rBottom = pArray->pData[nPos].nRow;
/*N*/ 	if (nPos==0)
/*N*/ 		rTop = 0;
/*N*/ 	else
/*N*/ 		rTop = pArray->pData[nPos-1].nRow + 1;
/*N*/ 	++nPos;
/*N*/ 	return TRUE;
/*N*/ }





}
