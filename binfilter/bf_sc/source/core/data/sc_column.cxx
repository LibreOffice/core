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

#include <bf_svtools/poolcach.hxx>
#include <bf_svx/scripttypeitem.hxx>
#include <string.h>

#include "scitems.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "markdata.hxx"
namespace binfilter {

//#pragma optimize ( "", off )
//	nur Search ohne Optimierung!

// STATIC DATA -----------------------------------------------------------


/*N*/ inline BOOL CellVisible( const ScBaseCell* pCell )		//! an Zelle verschieben
/*N*/ {
/*N*/ 	return ( pCell->GetCellType() != CELLTYPE_NOTE || pCell->GetNotePtr() );
/*N*/ }

/*N*/ inline BOOL IsAmbiguousScriptNonZero( BYTE nScript )
/*N*/ {
/*N*/ 	//!	move to a header file
/*N*/ 	return ( nScript != SCRIPTTYPE_LATIN &&
/*N*/ 			 nScript != SCRIPTTYPE_ASIAN &&
/*N*/ 			 nScript != SCRIPTTYPE_COMPLEX &&
/*N*/ 			 nScript != 0 );
/*N*/ }

// -----------------------------------------------------------------------------------------


/*N*/ ScColumn::ScColumn() :
/*N*/ 	nCol( 0 ),
/*N*/ 	nCount( 0 ),
/*N*/ 	nLimit( 0 ),
/*N*/ 	pItems( NULL ),
/*N*/ 	pDocument( NULL ),
/*N*/ 	pAttrArray( NULL )
/*N*/ {
/*N*/ }


/*N*/ ScColumn::~ScColumn()
/*N*/ {
/*N*/ 	FreeAll();
/*N*/ 	if (pAttrArray) delete pAttrArray;
/*N*/ }


/*N*/ void ScColumn::Init(USHORT nNewCol, USHORT nNewTab, ScDocument* pDoc)
/*N*/ {
/*N*/ 	nCol = nNewCol;
/*N*/ 	nTab = nNewTab;
/*N*/ 	pDocument = pDoc;
/*N*/ 	pAttrArray = new ScAttrArray( nCol, nTab, pDocument );
/*N*/ }




/*N*/ USHORT ScColumn::GetBlockMatrixEdges( USHORT nRow1, USHORT nRow2, USHORT nMask ) const
/*N*/ {
/*N*/ 	// nix:0, mitte:1, unten:2, links:4, oben:8, rechts:16, offen:32
/*N*/ 	if ( !pItems )
/*N*/ 		return 0;
/*N*/ 	if ( nRow1 == nRow2 )
/*N*/ 	{
/*N*/ 		USHORT nIndex;
/*N*/ 		if ( Search( nRow1, nIndex ) )
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 			if ( pCell->GetCellType() == CELLTYPE_FORMULA
/*N*/ 				&& ((ScFormulaCell*)pCell)->GetMatrixFlag() )
/*N*/ 			{
/*N*/ 				ScAddress aOrg( (UINT32)0xFFFFFFFF );
/*N*/ 				return ((ScFormulaCell*)pCell)->GetMatrixEdge( aOrg );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		return 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ScAddress aOrg( (UINT32)0xFFFFFFFF );
/*N*/ 		BOOL bOpen = FALSE;
/*N*/ 		USHORT nEdges = 0;
/*N*/ 		USHORT nIndex;
/*N*/ 		Search( nRow1, nIndex );
/*N*/ 		while ( nIndex < nCount && pItems[nIndex].nRow <= nRow2 )
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 			if ( pCell->GetCellType() == CELLTYPE_FORMULA
/*N*/ 				&& ((ScFormulaCell*)pCell)->GetMatrixFlag() )
/*N*/ 			{
/*N*/ 				nEdges = ((ScFormulaCell*)pCell)->GetMatrixEdge( aOrg );
/*N*/ 				if ( nEdges )
/*N*/ 				{
/*N*/ 					if ( nEdges & 8 )
/*N*/ 						bOpen = TRUE;	// obere Kante oeffnet, weitersehen
/*N*/ 					else if ( !bOpen )
/*N*/ 						return nEdges | 32;	// es gibt was, was nicht geoeffnet wurde
/*N*/ 					else if ( nEdges & 1 )
/*N*/ 						return nEdges;	// mittendrin
/*N*/ 					// (nMask & 16 und  (4 und nicht 16)) oder
/*N*/ 					// (nMask & 4  und (16 und nicht 4))
/*N*/ 					if ( ((nMask & 16) && (nEdges & 4)  && !(nEdges & 16))
/*N*/ 						|| ((nMask & 4)  && (nEdges & 16) && !(nEdges & 4)) )
/*N*/ 						return nEdges;	// nur linke/rechte Kante
/*N*/ 					if ( nEdges & 2 )
/*N*/ 						bOpen = FALSE;	// untere Kante schliesst
/*N*/ 				}
/*N*/ 			}
/*N*/ 			nIndex++;
/*N*/ 		}
/*N*/ 		if ( bOpen )
/*N*/ 			nEdges |= 32;			// es geht noch weiter
/*N*/ 		return nEdges;
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScColumn::HasSelectionMatrixFragment(const ScMarkData& rMark) const
/*N*/ {
/*N*/ 	if ( rMark.IsMultiMarked() )
/*N*/ 	{
/*N*/ 		BOOL bFound = FALSE;
/*N*/ 
/*N*/ 		ScAddress aOrg( (UINT32)0xFFFFFFFF );
/*N*/ 		ScAddress aCurOrg( (UINT32)0xFFFFFFFF );
/*N*/ 		USHORT nTop, nBottom;
/*N*/ 		ScMarkArrayIter aMarkIter( rMark.GetArray()+nCol );
/*N*/ 		while ( !bFound && aMarkIter.Next( nTop, nBottom ) )
/*N*/ 		{
/*N*/ 			BOOL bOpen = FALSE;
/*N*/ 			USHORT nEdges;
/*N*/ 			USHORT nIndex;
/*N*/ 			Search( nTop, nIndex );
/*N*/ 			while ( !bFound && nIndex < nCount && pItems[nIndex].nRow <= nBottom )
/*N*/ 			{
/*N*/ 				ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 				if ( pCell->GetCellType() == CELLTYPE_FORMULA
/*N*/ 					&& ((ScFormulaCell*)pCell)->GetMatrixFlag() )
/*N*/ 				{
/*N*/ 					nEdges = ((ScFormulaCell*)pCell)->GetMatrixEdge( aOrg );
/*N*/ 					if ( nEdges )
/*N*/ 					{
/*N*/ 						if ( nEdges & 8 )
/*N*/ 							bOpen = TRUE;	// obere Kante oeffnet, weitersehen
/*N*/ 						else if ( !bOpen )
/*N*/ 							return TRUE;	// es gibt was, was nicht geoeffnet wurde
/*N*/ 						else if ( nEdges & 1 )
/*N*/ 							bFound = TRUE;	// mittendrin, alles selektiert?
/*N*/ 						// (4 und nicht 16) oder (16 und nicht 4)
/*N*/ 						if ( (((nEdges & 4) | 16) ^ ((nEdges & 16) | 4)) )
/*N*/ 							bFound = TRUE;	// nur linke/rechte Kante, alles selektiert?
/*N*/ 						if ( nEdges & 2 )
/*N*/ 							bOpen = FALSE;	// untere Kante schliesst
/*N*/ 
/*N*/ 						if ( bFound )
/*N*/ 						{	// alles selektiert?
/*?*/ 							if ( aCurOrg != aOrg )
/*?*/ 							{	// neue Matrix zu pruefen?
/*?*/ 								aCurOrg = aOrg;
/*?*/ 								ScFormulaCell* pFCell;
/*?*/ 								if ( ((ScFormulaCell*)pCell)->GetMatrixFlag()
/*?*/ 										== MM_REFERENCE )
/*?*/ 									pFCell = (ScFormulaCell*) pDocument->GetCell( aOrg );
/*?*/ 								else
/*?*/ 									pFCell = (ScFormulaCell*)pCell;
/*?*/ 								USHORT nC, nR;
/*?*/ 								pFCell->GetMatColsRows( nC, nR );
/*?*/ 								ScRange aRange( aOrg, ScAddress(
/*?*/ 									aOrg.Col() + nC - 1, aOrg.Row() + nR - 1,
/*?*/ 									aOrg.Tab() ) );
/*?*/ 								if ( rMark.IsAllMarked( aRange ) )
/*?*/ 									bFound = FALSE;
/*?*/ 							}
/*?*/ 							else
/*?*/ 								bFound = FALSE;		// war schon
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				nIndex++;
/*N*/ 			}
/*N*/ 			if ( bOpen )
/*N*/ 				return TRUE;
/*N*/ 		}
/*N*/ 		return bFound;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }




/*N*/ BOOL ScColumn::HasAttrib( USHORT nRow1, USHORT nRow2, USHORT nMask ) const
/*N*/ {
/*N*/ 	return pAttrArray->HasAttrib( nRow1, nRow2, nMask );
/*N*/ }




/*N*/ BOOL ScColumn::ExtendMerge( USHORT nThisCol, USHORT nStartRow, USHORT nEndRow,
/*N*/ 							USHORT& rPaintCol, USHORT& rPaintRow,
/*N*/ 							BOOL bRefresh, BOOL bAttrs )
/*N*/ {
/*N*/ 	return pAttrArray->ExtendMerge( nThisCol, nStartRow, nEndRow, rPaintCol, rPaintRow, bRefresh, bAttrs );
/*N*/ }


/*N*/ void ScColumn::MergeSelectionPattern( SfxItemSet** ppSet, const ScMarkData& rMark, BOOL bDeep ) const
/*N*/ {
/*N*/ 	USHORT nTop;
/*N*/ 	USHORT nBottom;
/*N*/ 
/*N*/ 	if ( rMark.IsMultiMarked() )
/*N*/ 	{
/*N*/ 		ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
/*N*/ 		while (aMarkIter.Next( nTop, nBottom ))
/*N*/ 			pAttrArray->MergePatternArea( nTop, nBottom, ppSet, bDeep );
/*N*/ 	}
/*N*/ }


/*N*/ void ScColumn::MergePatternArea( SfxItemSet** ppSet, USHORT nRow1, USHORT nRow2, BOOL bDeep ) const
/*N*/ {
/*N*/ 	pAttrArray->MergePatternArea( nRow1, nRow2, ppSet, bDeep );
/*N*/ }






/*N*/ const ScPatternAttr* ScColumn::GetPattern( USHORT nRow ) const
/*N*/ {
/*N*/ 	return pAttrArray->GetPattern( nRow );
/*N*/ }


/*N*/ const SfxPoolItem* ScColumn::GetAttr( USHORT nRow, USHORT nWhich ) const
/*N*/ {
/*N*/ 	return &pAttrArray->GetPattern( nRow )->GetItemSet().Get(nWhich);
/*N*/ }


/*N*/ ULONG ScColumn::GetNumberFormat( USHORT nRow ) const
/*N*/ {
/*N*/ 	return pAttrArray->GetPattern( nRow )->GetNumberFormat( pDocument->GetFormatTable() );
/*N*/ }


/*N*/ short ScColumn::ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark )
/*N*/ {
/*N*/ 	USHORT nTop;
/*N*/ 	USHORT nBottom;
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 
/*N*/ 	if ( rMark.IsMultiMarked() )
/*N*/ 	{
/*N*/ 		ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
/*N*/ 		while (aMarkIter.Next( nTop, nBottom ))
/*N*/ 		{
/*N*/ 			pAttrArray->ApplyCacheArea( nTop, nBottom, pCache );
/*N*/ 			bFound = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!bFound)
/*N*/ 		return -1;
/*N*/ 	else if (nTop==0 && nBottom==MAXROW)
/*N*/ 		return 0;
/*N*/ 	else
/*N*/ 		return (short) nBottom;
/*N*/ }










/*N*/ void ScColumn::ApplyPatternArea( USHORT nStartRow, USHORT nEndRow, const ScPatternAttr& rPatAttr )
/*N*/ {
/*N*/ 	const SfxItemSet* pSet = &rPatAttr.GetItemSet();
/*N*/ 	SfxItemPoolCache aCache( pDocument->GetPool(), pSet );
/*N*/ 	pAttrArray->ApplyCacheArea( nStartRow, nEndRow, &aCache );
/*N*/ }






/*N*/ void ScColumn::ApplyStyleArea( USHORT nStartRow, USHORT nEndRow, const ScStyleSheet& rStyle )
/*N*/ {
/*N*/ 	pAttrArray->ApplyStyleArea(nStartRow, nEndRow, (ScStyleSheet*)&rStyle);
/*N*/ }


/*N*/ void ScColumn::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
/*N*/ {
/*N*/ 	USHORT nTop;
/*N*/ 	USHORT nBottom;
/*N*/ 
/*N*/ 	if ( rMark.IsMultiMarked() )
/*N*/ 	{
/*N*/ 		ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
/*N*/ 		while (aMarkIter.Next( nTop, nBottom ))
/*N*/ 			pAttrArray->ApplyStyleArea(nTop, nBottom, (ScStyleSheet*)&rStyle);
/*N*/ 	}
/*N*/ }

/*N*/ const ScStyleSheet* ScColumn::GetSelectionStyle( const ScMarkData& rMark, BOOL& rFound ) const
/*N*/ {
/*N*/ 	rFound = FALSE;
/*N*/ 	if (!rMark.IsMultiMarked())
/*N*/ 	{
/*N*/ 		DBG_ERROR("ScColumn::GetSelectionStyle ohne Selektion");
/*N*/ 		return NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bEqual = TRUE;
/*N*/ 
/*N*/ 	const ScStyleSheet* pStyle = NULL;
/*N*/ 	const ScStyleSheet* pNewStyle;
/*N*/ 
/*N*/ 	ScMarkArrayIter aMarkIter( rMark.GetArray() + nCol );
/*N*/ 	USHORT nTop;
/*N*/ 	USHORT nBottom;
/*N*/ 	while (bEqual && aMarkIter.Next( nTop, nBottom ))
/*N*/ 	{
/*N*/ 		ScAttrIterator aAttrIter( pAttrArray, nTop, nBottom );
/*N*/ 		USHORT nRow;
/*N*/ 		USHORT nDummy;
/*N*/ 		const ScPatternAttr* pPattern;
/*N*/ 		while (bEqual && ( pPattern = aAttrIter.Next( nRow, nDummy ) ))
/*N*/ 		{
/*N*/ 			pNewStyle = pPattern->GetStyleSheet();
/*N*/ 			rFound = TRUE;
/*N*/ 			if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
/*N*/ 				bEqual = FALSE;												// unterschiedliche
/*N*/ 			pStyle = pNewStyle;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bEqual ? pStyle : NULL;
/*N*/ }


/*N*/ const ScStyleSheet*	ScColumn::GetAreaStyle( BOOL& rFound, USHORT nRow1, USHORT nRow2 ) const
/*N*/ {
/*N*/ 	rFound = FALSE;
/*N*/ 
/*N*/ 	BOOL bEqual = TRUE;
/*N*/ 
/*N*/ 	const ScStyleSheet* pStyle = NULL;
/*N*/ 	const ScStyleSheet* pNewStyle;
/*N*/ 
/*N*/ 	ScAttrIterator aAttrIter( pAttrArray, nRow1, nRow2 );
/*N*/ 	USHORT nRow;
/*N*/ 	USHORT nDummy;
/*N*/ 	const ScPatternAttr* pPattern;
/*N*/ 	while (bEqual && ( pPattern = aAttrIter.Next( nRow, nDummy ) ))
/*N*/ 	{
/*N*/ 		pNewStyle = pPattern->GetStyleSheet();
/*N*/ 		rFound = TRUE;
/*N*/ 		if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
/*N*/ 			bEqual = FALSE;												// unterschiedliche
/*N*/ 		pStyle = pNewStyle;
/*N*/ 	}
/*N*/ 
/*N*/ 	return bEqual ? pStyle : NULL;
/*N*/ }


/*N*/ void ScColumn::FindStyleSheet( const SfxStyleSheetBase* pStyleSheet, BOOL* pUsed, BOOL bReset )
/*N*/ {
/*N*/ 	pAttrArray->FindStyleSheet( pStyleSheet, pUsed, bReset );
/*N*/ }


/*N*/ BOOL ScColumn::ApplyFlags( USHORT nStartRow, USHORT nEndRow, INT16 nFlags )
/*N*/ {
/*N*/ 	return pAttrArray->ApplyFlags( nStartRow, nEndRow, nFlags );
/*N*/ }


/*N*/ BOOL ScColumn::RemoveFlags( USHORT nStartRow, USHORT nEndRow, INT16 nFlags )
/*N*/ {
/*N*/ 	return pAttrArray->RemoveFlags( nStartRow, nEndRow, nFlags );
/*N*/ }


/*N*/ void ScColumn::ClearItems( USHORT nStartRow, USHORT nEndRow, const USHORT* pWhich )
/*N*/ {
/*N*/ 	pAttrArray->ClearItems( nStartRow, nEndRow, pWhich );
/*N*/ }


/*N*/ void ScColumn::ApplyAttr( USHORT nRow, const SfxPoolItem& rAttr )
/*N*/ {
/*N*/ 	//	um nur ein neues SetItem zu erzeugen, brauchen wir keinen SfxItemPoolCache.
/*N*/ 	//!	Achtung: der SfxItemPoolCache scheint zuviele Refs fuer das neue SetItem zu erzeugen ??
/*N*/ 
/*N*/ 	ScDocumentPool* pDocPool = pDocument->GetPool();
/*N*/ 
/*N*/ 	const ScPatternAttr* pOldPattern = pAttrArray->GetPattern( nRow );
/*N*/ 	ScPatternAttr* pTemp = new ScPatternAttr(*pOldPattern);
/*N*/ 	pTemp->GetItemSet().Put(rAttr);
/*N*/ 	const ScPatternAttr* pNewPattern = (const ScPatternAttr*) &pDocPool->Put( *pTemp );
/*N*/ 
/*N*/ 	if ( pNewPattern != pOldPattern )
/*N*/ 		pAttrArray->SetPattern( nRow, pNewPattern );
/*N*/ 	else
/*?*/ 		pDocPool->Remove( *pNewPattern );		// ausser Spesen nichts gewesen
/*N*/ 
/*N*/ 	delete pTemp;
/*N*/ 
/*N*/ 		// alte Version mit SfxItemPoolCache:
/*N*/ #if 0
/*?*/ 	SfxItemPoolCache aCache( pDocument->GetPool(), &rAttr );
/*?*/ 
/*?*/ 	const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );
/*?*/ 
/*?*/ 	//	TRUE = alten Eintrag behalten
/*?*/ 
/*?*/ 	ScPatternAttr* pNewPattern = (ScPatternAttr*) &aCache.ApplyTo( *pPattern, TRUE );
/*?*/ 	ScDocumentPool::CheckRef( *pPattern );
/*?*/ 	ScDocumentPool::CheckRef( *pNewPattern );
/*?*/ 
/*?*/ 	if (pNewPattern != pPattern)
/*?*/ 	  pAttrArray->SetPattern( nRow, pNewPattern );
/*N*/ #endif
/*N*/ }

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif


/*N*/ BOOL ScColumn::Search( USHORT nRow, USHORT& nIndex ) const
/*N*/ {
/*N*/ 	if ( !pItems || !nCount )
/*N*/ 	{
/*N*/ 		nIndex = 0;
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 	USHORT nMinRow = pItems[0].nRow;
/*N*/ 	if ( nRow <= nMinRow )
/*N*/ 	{
/*N*/ 		nIndex = 0;
/*N*/ 		return nRow == nMinRow;
/*N*/ 	}
/*N*/ 	USHORT nMaxRow = pItems[nCount-1].nRow;
/*N*/ 	if ( nRow >= nMaxRow )
/*N*/ 	{
/*N*/ 		if ( nRow == nMaxRow )
/*N*/ 		{
/*N*/ 			nIndex = nCount - 1;
/*N*/ 			return TRUE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nIndex = nCount;
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	short nOldLo, nOldHi;
/*N*/ 	short	nLo 	= nOldLo = 0;
/*N*/ 	short	nHi 	= nOldHi = Min( (short)(nCount-1), (short)nRow );
/*N*/ 	short	i		= 0;
/*N*/ 	BOOL	bFound	= FALSE;
/*N*/ 	// einigermassen durchgaengige Belegung? => interpolierende Suche
/*N*/ 	BOOL	bInterpol = (nMaxRow - nMinRow < nCount * 2);
/*N*/ 	USHORT	nR;
/*N*/ 
/*N*/ 	while ( !bFound && nLo <= nHi )
/*N*/ 	{
/*N*/ 		if ( !bInterpol || nHi - nLo < 3 )
/*N*/ 			i = (nLo+nHi) / 2;			// kein Aufwand, kein division by zero
/*N*/ 		else
/*N*/ 		{	// interpolierende Suche
/*N*/ 			long nLoRow = pItems[nLo].nRow;		// kein USHORT Unterlauf bei Subtraktion
/*N*/ 			i = nLo + (short)((long)(nRow - nLoRow) * (nHi - nLo)
/*N*/ 				/ (pItems[nHi].nRow - nLoRow));
/*N*/ 			if ( i < 0 || i >= nCount )
/*N*/ 			{	// oops ...
/*N*/ 				i = (nLo+nHi) / 2;
/*N*/ 				bInterpol = FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		nR = pItems[i].nRow;
/*N*/ 		if ( nR < nRow )
/*N*/ 		{
/*N*/ 			nLo = i+1;
/*N*/ 			if ( bInterpol )
/*N*/ 			{
/*N*/ 				if ( nLo <= nOldLo )
/*N*/ 					bInterpol = FALSE;
/*N*/ 				else
/*N*/ 					nOldLo = nLo;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if ( nR > nRow )
/*N*/ 			{
/*N*/ 				nHi = i-1;
/*N*/ 				if ( bInterpol )
/*N*/ 				{
/*N*/ 					if ( nHi >= nOldHi )
/*N*/ 						bInterpol = FALSE;
/*N*/ 					else
/*N*/ 						nOldHi = nHi;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bFound = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (bFound)
/*N*/ 		nIndex = i;
/*N*/ 	else
/*N*/ 		nIndex = nLo; // hinterer Index
/*N*/ 	return bFound;
/*N*/ }

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif


/*N*/ ScBaseCell* ScColumn::GetCell( USHORT nRow ) const
/*N*/ {
/*N*/ 	USHORT nIndex;
/*N*/ 	if (Search(nRow, nIndex))
/*N*/ 		return pItems[nIndex].pCell;
/*N*/ 	return NULL;
/*N*/ }


/*N*/ void ScColumn::Resize( USHORT nSize )
/*N*/ {
/*N*/ 	if (nSize > MAXROW+1)
/*N*/ 		nSize = MAXROW+1;
/*N*/ 	if (nSize < nCount)
/*N*/ 		nSize = nCount;
/*N*/ 
/*N*/ 	ColEntry* pNewItems;
/*N*/ 	if (nSize)
/*N*/ 	{
/*N*/ 		USHORT nNewSize = nSize + COLUMN_DELTA - 1;
/*N*/ 		nNewSize -= nNewSize % COLUMN_DELTA;
/*N*/ 		nLimit = nNewSize;
/*N*/ 		pNewItems = new ColEntry[nLimit];
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nLimit = 0;
/*N*/ 		pNewItems = NULL;
/*N*/ 	}
/*N*/ 	if (pItems)
/*N*/ 	{
/*N*/ 		if (pNewItems)
/*N*/ 			memmove( pNewItems, pItems, nCount * sizeof(ColEntry) );
/*N*/ 		delete[] pItems;
/*N*/ 	}
/*N*/ 	pItems = pNewItems;
/*N*/ }

//	SetNewRow gehoert zu SwapRow


//	SwapRow zum Sortieren





/*N*/ BOOL ScColumn::TestInsertCol( USHORT nStartRow, USHORT nEndRow) const
/*N*/ {
/*N*/ 	if (!IsEmpty())
/*N*/ 	{
/*N*/ 		BOOL bTest = TRUE;
/*N*/ 		if (pItems)
/*N*/ 			for (USHORT i=0; (i<nCount) && bTest; i++)
/*N*/ 				bTest = ((pItems[i].nRow < nStartRow) && (pItems[i].nRow > nEndRow))
/*N*/ 						|| !CellVisible(pItems[i].pCell);
/*N*/ 
/*N*/ 		//	AttrArray testet nur zusammengefasste
/*N*/ 
/*N*/ 		if ((bTest) && (pAttrArray))
/*N*/ 			bTest = pAttrArray->TestInsertCol(nStartRow, nEndRow);
/*N*/ 
/*N*/ 		//!		rausgeschobene Attribute bei Undo beruecksichtigen
/*N*/ 
/*N*/ 		return bTest;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return TRUE;
/*N*/ }


/*N*/ BOOL ScColumn::TestInsertRow( USHORT nSize ) const
/*N*/ {
/*N*/ 	//	AttrArray testet nur zusammengefasste
/*N*/ 
/*N*/ 	if ( pItems && nCount )
/*N*/ 		return ( pItems[nCount-1].nRow <= MAXROW-nSize && pAttrArray->TestInsertRow( nSize ) );
/*N*/ 	else
/*N*/ 		return pAttrArray->TestInsertRow( nSize );
/*N*/ 
/*N*/ 	//!		rausgeschobene Attribute bei Undo beruecksichtigen
/*N*/ 
/*N*/ 	if ( nSize > MAXROW )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	USHORT nVis = nCount;
/*N*/ 	while ( nVis && !CellVisible(pItems[nVis-1].pCell) )
/*N*/ 		--nVis;
/*N*/ 
/*N*/ 	if ( nVis )
/*N*/ 		return ( pItems[nVis-1].nRow <= MAXROW-nSize );
/*N*/ 	else
/*N*/ 		return TRUE;
/*N*/ }


/*N*/ void ScColumn::InsertRow( USHORT nStartRow, USHORT nSize )
/*N*/ {
/*N*/ 	pAttrArray->InsertRow( nStartRow, nSize );
/*N*/ 
/*N*/ 	//!	Search
/*N*/ 
/*N*/ 	if ( !pItems || !nCount )
/*N*/ 		return;
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	Search( nStartRow, i );
/*N*/ 	if ( i >= nCount )
/*N*/ 		return ;
/*N*/ 
/*N*/ 	BOOL bOldAutoCalc = pDocument->GetAutoCalc();
/*N*/ 	pDocument->SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 
/*N*/ 	USHORT nNewCount = nCount;
/*N*/ 	BOOL bCountChanged = FALSE;
/*N*/ 	ScAddress aAdr( nCol, 0, nTab );
/*N*/     ScHint aHint( SC_HINT_DATACHANGED, aAdr, NULL );    // only areas (ScBaseCell* == NULL)
/*N*/     ScAddress& rAddress = aHint.GetAddress();
/*N*/     // for sparse occupation use single broadcasts, not ranges
/*N*/     BOOL bSingleBroadcasts = (((pItems[nCount-1].nRow - pItems[i].nRow) /
/*N*/                 (nCount - i)) > 1);
/*N*/     if ( bSingleBroadcasts )
/*N*/     {
/*N*/         USHORT nLastBroadcast = MAXROW+1;
/*N*/         for ( ; i < nCount; i++)
/*N*/         {
/*N*/             USHORT nOldRow = pItems[i].nRow;
/*N*/             // #43940# Aenderung Quelle broadcasten
/*N*/             if ( nLastBroadcast != nOldRow )
/*N*/             {   // direkt aufeinanderfolgende nicht doppelt broadcasten
/*N*/                 rAddress.SetRow( nOldRow );
/*N*/                 pDocument->AreaBroadcast( aHint );
/*N*/             }
/*N*/             USHORT nNewRow = (pItems[i].nRow += nSize);
/*N*/             // #43940# Aenderung Ziel broadcasten
/*N*/             rAddress.SetRow( nNewRow );
/*N*/             pDocument->AreaBroadcast( aHint );
/*N*/             nLastBroadcast = nNewRow;
/*N*/             ScBaseCell* pCell = pItems[i].pCell;
/*N*/             if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/                 ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
/*N*/             if ( nNewRow > MAXROW && !bCountChanged )
/*N*/             {
/*N*/                 nNewCount = i;
/*N*/                 bCountChanged = TRUE;
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         rAddress.SetRow( pItems[i].nRow );
/*N*/         ScRange aRange( rAddress );
/*N*/         for ( ; i < nCount; i++)
/*N*/         {
/*N*/             USHORT nNewRow = (pItems[i].nRow += nSize);
/*N*/             ScBaseCell* pCell = pItems[i].pCell;
/*N*/             if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/                 ((ScFormulaCell*)pCell)->aPos.SetRow( nNewRow );
/*N*/             if ( nNewRow > MAXROW && !bCountChanged )
/*N*/             {
/*N*/                 nNewCount = i;
/*N*/                 bCountChanged = TRUE;
/*N*/                 aRange.aEnd.SetRow( MAXROW );
/*N*/             }
/*N*/         }
/*N*/         if ( !bCountChanged )
/*N*/             aRange.aEnd.SetRow( pItems[nCount-1].nRow );
/*N*/         pDocument->AreaBroadcastInRange( aRange, aHint );
/*N*/     }
/*N*/ 
/*N*/ 	if (bCountChanged)
/*N*/ 	{
/*N*/ 		USHORT nDelCount = nCount - nNewCount;
/*N*/ 		ScBaseCell** ppDelCells = new ScBaseCell*[nDelCount];
/*N*/ 		USHORT* pDelRows = new USHORT[nDelCount];
/*N*/ 		for (i = 0; i < nDelCount; i++)
/*N*/ 		{
/*N*/ 			ppDelCells[i] = pItems[nNewCount+i].pCell;
/*N*/ 			pDelRows[i] = pItems[nNewCount+i].nRow;
/*N*/ 		}
/*N*/ 		nCount = nNewCount;
/*N*/ 
/*N*/ 		for (i = 0; i < nDelCount; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = ppDelCells[i];
/*N*/ 			DBG_ASSERT( !CellVisible(pCell), "sichtbare Zelle weggeschoben" );
/*N*/ 			ScBroadcasterList* pBC = pCell->GetBroadcaster();
/*N*/ 			if (pBC)
/*N*/ 			{
/*N*/ 				MoveListeners( *pBC, pDelRows[i] - nSize );
/*N*/ 				pCell->SetBroadcaster(NULL);
/*N*/ 				pCell->Delete();
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		delete[] pDelRows;
/*N*/ 		delete[] ppDelCells;
/*N*/ 	}
/*N*/ 
/*N*/ 	pDocument->SetAutoCalc( bOldAutoCalc );
/*N*/ }




/*N*/ void ScColumn::CopyToColumn(USHORT nRow1, USHORT nRow2, USHORT nFlags, BOOL bMarked,
/*N*/ 								ScColumn& rColumn, const ScMarkData* pMarkData, BOOL bAsLink )
/*N*/ {
/*N*/ 	if (bMarked)
/*N*/ 	{
/*N*/ 		USHORT nStart, nEnd;
/*N*/ 		if (pMarkData && pMarkData->IsMultiMarked())
/*N*/ 		{
/*N*/ 			ScMarkArrayIter aIter( pMarkData->GetArray()+nCol );
/*N*/ 
/*N*/ 			while ( aIter.Next( nStart, nEnd ) && nStart <= nRow2 )
/*N*/ 			{
/*N*/ 				if ( nEnd >= nRow1 )
/*N*/ 					CopyToColumn( Max(nRow1,nStart), Min(nRow2,nEnd),
/*N*/ 									nFlags, FALSE, rColumn, pMarkData, bAsLink );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			DBG_ERROR("CopyToColumn: bMarked, aber keine Markierung");
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( (nFlags & IDF_ATTRIB) != 0 )
/*N*/ 	{
/*N*/ 		if ( (nFlags & IDF_STYLES) != IDF_STYLES )
/*N*/ 		{	// StyleSheets im Zieldokument bleiben erhalten
/*?*/ 			// z.B. DIF und RTF Clipboard-Import
/*?*/ 			for ( USHORT nRow = nRow1; nRow <= nRow2; nRow++ )
/*?*/ 			{
/*?*/ 				const ScStyleSheet* pStyle =
/*?*/ 					rColumn.pAttrArray->GetPattern( nRow )->GetStyleSheet();
/*?*/ 				const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );
/*?*/ 				ScPatternAttr* pNewPattern = new ScPatternAttr( *pPattern );
/*?*/ 				pNewPattern->SetStyleSheet( (ScStyleSheet*)pStyle );
/*?*/ 				rColumn.pAttrArray->SetPattern( nRow, pNewPattern, TRUE );
/*?*/ 				delete pNewPattern;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pAttrArray->CopyArea( nRow1, nRow2, 0, *rColumn.pAttrArray);
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	if ((nFlags & IDF_CONTENTS) != 0)
/*N*/ 	{
/*N*/ 		USHORT i;
/*N*/ 		USHORT nBlockCount = 0;
/*N*/ 		USHORT nStartIndex, nEndIndex;
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 			if ((pItems[i].nRow >= nRow1) && (pItems[i].nRow <= nRow2))
/*N*/ 			{
/*N*/ 				if (!nBlockCount)
/*N*/ 					nStartIndex = i;
/*N*/ 				nEndIndex = i;
/*N*/ 				++nBlockCount;
/*N*/ 			}
/*N*/ 
/*N*/ 		if (nBlockCount)
/*N*/ 		{
/*N*/ 			rColumn.Resize( rColumn.GetCellCount() + nBlockCount );
/*N*/ 			ScAddress aAdr( rColumn.nCol, 0, rColumn.nTab );
/*N*/ 			for (i = nStartIndex; i <= nEndIndex; i++)
/*N*/ 			{
/*N*/ 				ScBaseCell* pOld = pItems[i].pCell;
/*N*/ 				aAdr.SetRow( pItems[i].nRow );
/*N*/ 				ScBaseCell* pNew = NULL;
/*N*/ 				if (bAsLink)
/*N*/ 				{
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 					pNew = CreateRefCell( rColumn.pDocument, aAdr, i, nFlags );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					pNew = CloneCell( i, nFlags, rColumn.pDocument, aAdr );
/*N*/ 
/*N*/ 					if ( pNew && pNew->GetNotePtr() && (nFlags & IDF_NOTE) == 0 )
/*?*/ 						pNew->DeleteNote();
/*N*/ 				}
/*N*/ 
/*N*/ 				if (pNew)
/*N*/ 					rColumn.Insert(pItems[i].nRow, pNew);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }












/*N*/ void ScColumn::MarkScenarioIn( ScMarkData& rDestMark ) const
/*N*/ {
/*N*/ 	ScRange aRange( nCol, 0, nTab );
/*N*/ 
/*N*/ 	ScAttrIterator aAttrIter( pAttrArray, 0, MAXROW );
/*N*/ 	USHORT nStart, nEnd;
/*N*/ 	const ScPatternAttr* pPattern = aAttrIter.Next( nStart, nEnd );
/*N*/ 	while (pPattern)
/*N*/ 	{
/*N*/ 		if ( ((ScMergeFlagAttr&)pPattern->GetItem( ATTR_MERGE_FLAG )).IsScenario() )
/*N*/ 		{
/*N*/ 			aRange.aStart.SetRow( nStart );
/*N*/ 			aRange.aEnd.SetRow( nEnd );
/*N*/ 			rDestMark.SetMultiMarkArea( aRange, TRUE );
/*N*/ 		}
/*N*/ 
/*N*/ 		pPattern = aAttrIter.Next( nStart, nEnd );
/*N*/ 	}
/*N*/ }


/*N*/ void ScColumn::SwapCol(ScColumn& rCol)
/*N*/ {
/*N*/ 	USHORT nTemp;
/*N*/ 
/*N*/ 	nTemp = rCol.nCount;
/*N*/ 	rCol.nCount  = nCount;
/*N*/ 	nCount = nTemp;
/*N*/ 
/*N*/ 	nTemp = rCol.nLimit;
/*N*/ 	rCol.nLimit = nLimit;
/*N*/ 	nLimit = nTemp;
/*N*/ 
/*N*/ 	ColEntry* pTempItems = rCol.pItems;
/*N*/ 	rCol.pItems = pItems;
/*N*/ 	pItems = pTempItems;
/*N*/ 
/*N*/ 	ScAttrArray* pTempAttr = rCol.pAttrArray;
/*N*/ 	rCol.pAttrArray = pAttrArray;
/*N*/ 	pAttrArray = pTempAttr;
/*N*/ 
/*N*/ 	// #38415# AttrArray muss richtige Spaltennummer haben
/*N*/ 	pAttrArray->SetCol(nCol);
/*N*/ 	rCol.pAttrArray->SetCol(rCol.nCol);
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	if (pItems)
/*N*/ 		for (i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScFormulaCell* pCell = (ScFormulaCell*) pItems[i].pCell;
/*N*/ 			if( pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 				pCell->aPos.SetCol(nCol);
/*N*/ 		}
/*N*/ 	if (rCol.pItems)
/*N*/ 		for (i = 0; i < rCol.nCount; i++)
/*N*/ 		{
/*N*/ 			ScFormulaCell* pCell = (ScFormulaCell*) rCol.pItems[i].pCell;
/*N*/ 			if( pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 				pCell->aPos.SetCol(rCol.nCol);
/*N*/ 		}
/*N*/ }


/*N*/ void ScColumn::MoveTo(USHORT nStartRow, USHORT nEndRow, ScColumn& rCol)
/*N*/ {
/*N*/ 	pAttrArray->MoveTo(nStartRow, nEndRow, *rCol.pAttrArray);
/*N*/ 
/*N*/ 	if (pItems)
/*N*/ 	{
/*N*/ 		USHORT nStartPos;
/*N*/ 		USHORT nMoveCount=0;
/*N*/ 		USHORT i;
/*N*/ 		for (i=0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			if ((pItems[i].nRow >= nStartRow) && (pItems[i].nRow <= nEndRow))
/*N*/ 			{
/*N*/ 				if (nMoveCount==0)
/*N*/ 					nStartPos=i;
/*N*/ 				++nMoveCount;
/*N*/ 
/*N*/ 				rCol.Insert(pItems[i].nRow, pItems[i].pCell);
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if (nMoveCount > 0)
/*N*/ 		{
/*N*/ 			//	Formeln benachrichtigen, dass sich etwas aendert
/*N*/ 
/*N*/ 			ScNoteCell* pNoteCell = new ScNoteCell;		// Dummy wie in DeleteRange
/*N*/ 			USHORT nEndPos = nStartPos+nMoveCount-1;
/*N*/ 			for (i=nStartPos; i<=nEndPos; i++)
/*N*/ 				pItems[i].pCell = pNoteCell;			// nicht auf die verschobenen zugreifen
/*N*/             ScAddress aAdr( nCol, 0, nTab );
/*N*/             ScHint aHint( SC_HINT_DYING, aAdr, NULL );  // areas only
/*N*/             ScAddress& rAddress = aHint.GetAddress();
/*N*/ 			for (i=nStartPos; i<=nEndPos; i++)
/*N*/             {
/*N*/                 rAddress.SetRow( pItems[i].nRow );
/*N*/ 				pDocument->AreaBroadcast( aHint );
/*N*/             }
/*N*/ 			delete pNoteCell;
/*N*/ 
/*N*/ 			nCount -= nMoveCount;
/*N*/ 			memmove( &pItems[nStartPos], &pItems[nStartPos+nMoveCount],
/*N*/ 						(nCount - nStartPos) * sizeof(ColEntry) );
/*N*/ 			pItems[nCount].nRow = 0;
/*N*/ 			pItems[nCount].pCell = NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScColumn::UpdateReference( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
/*N*/ 			 USHORT nCol2, USHORT nRow2, USHORT nTab2, short nDx, short nDy, short nDz,
/*N*/ 			 ScDocument* pUndoDoc )
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 	{
/*N*/ 		ScRange aRange( ScAddress( nCol1, nRow1, nTab1 ),
/*N*/ 						ScAddress( nCol2, nRow2, nTab2 ) );
/*N*/ 		if ( eUpdateRefMode == URM_COPY && nRow1 == nRow2 )
/*N*/ 		{	// z.B. eine einzelne Zelle aus dem Clipboard eingefuegt
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			USHORT nIndex;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/             // #90279# For performance reasons two loop bodies instead of
/*?*/             // testing for update mode in each iteration.
/*?*/             // Anyways, this is still a bottleneck on large arrays with few
/*?*/             // formulas cells.
/*?*/             if ( eUpdateRefMode == URM_COPY )
/*?*/             {
/*?*/                 USHORT i;
/*?*/                 Search( nRow1, i );
/*?*/                 for ( ; i < nCount; i++ )
/*?*/                 {
/*?*/                     USHORT nRow = pItems[i].nRow;
/*?*/                     if ( nRow > nRow2 )
/*?*/                         break;
/*?*/                     ScBaseCell* pCell = pItems[i].pCell;
/*?*/                     if( pCell->GetCellType() == CELLTYPE_FORMULA)
/*?*/                     {
/*?*/                         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ((ScFormulaCell*)pCell)->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz, pUndoDoc );
/*?*/                     }
/*?*/                 }
/*?*/             }
/*?*/             else
/*?*/             {
/*?*/                 USHORT i = 0;
/*?*/                 for ( ; i < nCount; i++ )
/*?*/                 {
/*?*/                     ScBaseCell* pCell = pItems[i].pCell;
/*?*/                     if( pCell->GetCellType() == CELLTYPE_FORMULA)
/*?*/                     {
/*?*/                         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 USHORT nRow = pItems[i].nRow;
/*?*/                     }
/*?*/                 }
/*?*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ }






/*N*/ void ScColumn::UpdateInsertTab( USHORT nTable)
/*N*/ {
/*N*/ 	if (nTab >= nTable) nTab++;
/*N*/ 	if( pItems )
/*N*/ 		UpdateInsertTabOnlyCells( nTable );
/*N*/ }


/*N*/ void ScColumn::UpdateInsertTabOnlyCells( USHORT nTable)
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScFormulaCell* pCell = (ScFormulaCell*) pItems[i].pCell;
/*N*/ 			if( pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 			{
/*N*/ 				USHORT nRow = pItems[i].nRow;
/*N*/ 				pCell->UpdateInsertTab(nTable);
/*N*/ 				if ( nRow != pItems[i].nRow )
/*N*/ 					Search( nRow, i );		// Listener geloescht/eingefuegt?
/*N*/ 			}
/*N*/ 		}
/*N*/ }




/*N*/  void ScColumn::UpdateDeleteTab( USHORT nTable, BOOL bIsMove, ScColumn* pRefUndo )
/*N*/  {
/*N*/  	if (nTab > nTable)
/*N*/  		pAttrArray->SetTab(--nTab);
/*N*/  
/*N*/  	if (pItems)
/*N*/  		for (USHORT i = 0; i < nCount; i++)
/*N*/  			if ( pItems[i].pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/  			{
/*N*/  				USHORT nRow = pItems[i].nRow;
/*N*/  				ScFormulaCell* pOld = (ScFormulaCell*)pItems[i].pCell;
/*N*/  
/*N*/  				ScFormulaCell* pSave = NULL;
/*N*/  				if (pRefUndo)
/*N*/  					pSave = (ScFormulaCell*)pOld->Clone( pDocument,
/*N*/  						ScAddress( nCol, nRow, nTab ), TRUE );
/*N*/  
/*N*/  				BOOL bChanged = pOld->UpdateDeleteTab(nTable, bIsMove);
/*N*/  				if ( nRow != pItems[i].nRow )
/*N*/  					Search( nRow, i );		// Listener geloescht/eingefuegt?
/*N*/  
/*N*/  				if (pRefUndo)
/*N*/  				{
/*N*/  					if (bChanged)
/*N*/  						pRefUndo->Insert( nRow, pSave );
/*N*/  					else
/*N*/  						delete pSave;
/*N*/  				}
/*N*/  			}
/*N*/  }


/*N*/ void ScColumn::UpdateCompile( BOOL bForceIfNameInUse )
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
/*N*/ 			if( p->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 			{
/*N*/ 				USHORT nRow = pItems[i].nRow;
/*N*/ 				p->UpdateCompile( bForceIfNameInUse );
/*N*/ 				if ( nRow != pItems[i].nRow )
/*?*/ 					Search( nRow, i );		// Listener geloescht/eingefuegt?
/*N*/ 			}
/*N*/ 		}
/*N*/ }


/*N*/ void ScColumn::SetTabNo(USHORT nNewTab)
/*N*/ {
/*N*/ 	nTab = nNewTab;
/*N*/ 	pAttrArray->SetTab( nNewTab );
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
/*N*/ 			if( p->GetCellType() == CELLTYPE_FORMULA )
/*?*/ 				p->aPos.SetTab( nNewTab );
/*N*/ 		}
/*N*/ }


/*N*/ BOOL ScColumn::IsRangeNameInUse(USHORT nRow1, USHORT nRow2, USHORT nIndex) const
/*N*/ {
/*N*/ 	BOOL bInUse = FALSE;
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; !bInUse && (i < nCount); i++)
/*N*/ 			if ((pItems[i].nRow >= nRow1) &&
/*N*/ 				(pItems[i].nRow <= nRow2) &&
/*N*/ 				(pItems[i].pCell->GetCellType() == CELLTYPE_FORMULA))
/*N*/ 					bInUse = ((ScFormulaCell*)pItems[i].pCell)->IsRangeNameInUse(nIndex);
/*N*/ 	return bInUse;
/*N*/ }




/*N*/ void ScColumn::SetDirtyVar()
/*N*/ {
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
/*N*/ 		if( p->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 			p->SetDirtyVar();
/*N*/ 	}
/*N*/ }


/*N*/ void ScColumn::SetDirty()
/*N*/ {
/*N*/ 	// wird nur dokumentweit verwendet, kein FormulaTrack
/*N*/ 	BOOL bOldAutoCalc = pDocument->GetAutoCalc();
/*N*/ 	pDocument->SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
/*N*/ 		if( p->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 		{
/*N*/ 			p->SetDirtyVar();
/*N*/ 			if ( !pDocument->IsInFormulaTree( p ) )
/*N*/ 				pDocument->PutInFormulaTree( p );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	pDocument->SetAutoCalc( bOldAutoCalc );
/*N*/ }




/*N*/  void ScColumn::SetTableOpDirty( const ScRange& rRange )
/*N*/  {
/*N*/  	if ( !pItems || !nCount )
/*N*/  		return ;
/*N*/  	BOOL bOldAutoCalc = pDocument->GetAutoCalc();
/*N*/  	pDocument->SetAutoCalc( FALSE );	// no multiple recalculation
/*N*/  	USHORT nRow2 = rRange.aEnd.Row();
/*N*/  	ScAddress aPos( nCol, 0, nTab );
/*N*/      ScHint aHint( SC_HINT_TABLEOPDIRTY, aPos, NULL );
/*N*/  	USHORT nRow, nIndex;
/*N*/  	Search( rRange.aStart.Row(), nIndex );
/*N*/  	while ( nIndex < nCount && (nRow = pItems[nIndex].nRow) <= nRow2 )
/*N*/  	{
/*N*/  		ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/  		if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/  			((ScFormulaCell*)pCell)->SetTableOpDirty();
/*N*/  		else
/*N*/  		{
/*N*/  			aHint.GetAddress().SetRow( nRow );
/*N*/              aHint.SetCell( pCell );
/*N*/  			pDocument->Broadcast( aHint );
/*N*/  		}
/*N*/  		nIndex++;
/*N*/  	}
/*N*/  	pDocument->SetAutoCalc( bOldAutoCalc );
/*N*/  }


/*N*/ void ScColumn::SetDirtyAfterLoad()
/*N*/ {
/*N*/ 	BOOL bOldAutoCalc = pDocument->GetAutoCalc();
/*N*/ 	pDocument->SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
/*N*/ 		if ( p->GetCellType() == CELLTYPE_FORMULA && p->GetDirty() )
/*N*/ 			p->SetDirty();
/*N*/ 			// wenn die Zelle durch CalcAfterLoad schon bDirty war, muss
/*N*/ 			// jetzt noch FormulaTracking stattfinden
/*N*/ 	}
/*N*/ 	pDocument->SetAutoCalc( bOldAutoCalc );
/*N*/ }


/*N*/ void ScColumn::SetRelNameDirty()
/*N*/ {
/*N*/ 	BOOL bOldAutoCalc = pDocument->GetAutoCalc();
/*N*/ 	pDocument->SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScFormulaCell* p = (ScFormulaCell*) pItems[i].pCell;
/*N*/ 		if( p->GetCellType() == CELLTYPE_FORMULA && p->HasRelNameReference() )
/*N*/ 			p->SetDirty();
/*N*/ 	}
/*N*/ 	pDocument->SetAutoCalc( bOldAutoCalc );
/*N*/ }


/*N*/ void ScColumn::CalcAll()
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i=0; i<nCount; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if (pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 			{
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 				// nach F9 ctrl-F9: ueberprueft die Berechnung per FormulaTree
/*N*/ 				ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
/*N*/ 				double nOldVal, nNewVal;
/*N*/ 				nOldVal = pFCell->GetValue();
/*N*/ #endif
/*N*/ 				((ScFormulaCell*)pCell)->Interpret();
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 				if ( pFCell->GetCode()->IsRecalcModeNormal() )
/*N*/ 					nNewVal = pFCell->GetValue();
/*N*/ 				else
/*N*/ 					nNewVal = nOldVal;	// random(), jetzt() etc.
/*N*/ 				DBG_ASSERT( nOldVal==nNewVal, "CalcAll: nOldVal != nNewVal" );
/*N*/ #endif
/*N*/ 			}
/*N*/ 		}
/*N*/ }


/*N*/ void ScColumn::CompileAll()
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 			{
/*?*/ 				USHORT nRow = pItems[i].nRow;
/*?*/ 				// fuer unbedingtes kompilieren
/*?*/ 				// bCompile=TRUE und pCode->nError=0
/*?*/ 				((ScFormulaCell*)pCell)->GetCode()->SetError( 0 );
/*?*/ 				((ScFormulaCell*)pCell)->SetCompile( TRUE );
/*?*/ 				((ScFormulaCell*)pCell)->CompileTokenArray();
/*?*/ 				if ( nRow != pItems[i].nRow )
/*?*/ 					Search( nRow, i );		// Listener geloescht/eingefuegt?
/*N*/ 			}
/*N*/ 		}
/*N*/ }


/*N*/ void ScColumn::CompileXML( ScProgress& rProgress )
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 			{
/*N*/ 				USHORT nRow = pItems[i].nRow;
/*N*/ 				((ScFormulaCell*)pCell)->CompileXML( rProgress );
/*N*/ 				if ( nRow != pItems[i].nRow )
/*N*/ 					Search( nRow, i );		// Listener geloescht/eingefuegt?
/*N*/ 			}
/*N*/ 		}
/*N*/ }


/*N*/ void ScColumn::CalcAfterLoad()
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[i].pCell;
/*N*/ 			if ( pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 				((ScFormulaCell*)pCell)->CalcAfterLoad();
/*N*/ 		}
/*N*/ }


/*N*/ void ScColumn::ResetChanged( USHORT nStartRow, USHORT nEndRow )
/*N*/ {
/*N*/ 	if (pItems)
/*N*/ 	{
/*N*/ 		USHORT nIndex;
/*N*/ 		Search(nStartRow,nIndex);
/*N*/ 		while (nIndex<nCount && pItems[nIndex].nRow <= nEndRow)
/*N*/ 		{
/*N*/ 			ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 			if (pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 				((ScFormulaCell*)pCell)->ResetChanged();
/*N*/ 			++nIndex;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ BOOL ScColumn::HasEditCells(USHORT nStartRow, USHORT nEndRow, USHORT& rFirst) const
/*N*/ {
/*N*/ 	//	used in GetOptimalHeight - ambiguous script type counts as edit cell
/*N*/ 
/*N*/ 	USHORT nRow;
/*N*/ 	USHORT nIndex;
/*N*/ 	Search(nStartRow,nIndex);
/*N*/ 	while ( (nIndex < nCount) ? ((nRow=pItems[nIndex].nRow) <= nEndRow) : FALSE )
/*N*/ 	{
/*N*/ 		ScBaseCell* pCell = pItems[nIndex].pCell;
/*N*/ 		if ( pCell->GetCellType() == CELLTYPE_EDIT ||
/*N*/ 			 IsAmbiguousScriptNonZero( pDocument->GetScriptType(nCol, nRow, nTab, pCell) ) )
/*N*/ 		{
/*N*/ 			rFirst = nRow;
/*N*/ 			return TRUE;
/*N*/ 		}
/*N*/ 		++nIndex;
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }






}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
