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

#include "scitems.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "stlpool.hxx"
#include "globstr.hrc"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

#define GET_SCALEVALUE(set,id) 	((const SfxUInt16Item&)(set.Get( id ))).GetValue()


/*N*/ void ScTable::UpdatePageBreaks( const ScRange* pUserArea )
/*N*/ {
/*N*/ 	if ( pDocument->IsImportingXML() )
/*N*/ 		return;
/*N*/ 	if ( !pUserArea && !bPageSizeValid )
/*N*/ 		return;
/*N*/ 
/*N*/ 	SfxStyleSheetBase* pStyle = pDocument->GetStyleSheetPool()->
/*N*/ 									Find( aPageStyle, SFX_STYLE_FAMILY_PAGE );
/*N*/ 	if ( !pStyle )
/*N*/ 	{
/*N*/ 		DBG_ERROR("UpdatePageBreaks: Style nicht gefunden");
/*N*/ 		return;
/*N*/ 	}
/*N*/ 	SfxItemSet* pStyleSet = &pStyle->GetItemSet();
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	USHORT nX;
/*N*/ 	USHORT nY;
/*N*/ 	USHORT nStartCol = 0;
/*N*/ 	USHORT nStartRow = 0;
/*N*/ 	USHORT nEndCol = MAXCOL;
/*N*/ 	USHORT nEndRow = MAXROW;
/*N*/ 	if (pUserArea)
/*N*/ 	{
/*N*/ 		nStartCol = pUserArea->aStart.Col();
/*N*/ 		nStartRow = pUserArea->aStart.Row();
/*N*/ 		nEndCol = pUserArea->aEnd.Col();
/*N*/ 		nEndRow = pUserArea->aEnd.Row();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nAreaCount = GetPrintRangeCount();
/*N*/ 		if ( nAreaCount > 1 )
/*N*/ 		{
/*N*/ 			//	bei mehreren Bereichen nichts anzeigen:
/*N*/ 
/*N*/ 			for (nX=0; nX<MAXCOL; nX++)
/*N*/ 				pColFlags[nX] &= ~CR_PAGEBREAK;
/*N*/ 			for (nY=0; nY<MAXROW; nY++)
/*N*/ 				pRowFlags[nY] &= ~CR_PAGEBREAK;
/*N*/ 
/*N*/ 			return;
/*N*/ 		}
/*N*/ 		else if ( nAreaCount == 1 )
/*N*/ 		{
/*N*/ 			const ScRange* pArea = GetPrintRange( 0 );
/*N*/ 			if (pArea)
/*N*/ 			{
/*N*/ 				nStartCol = pArea->aStart.Col();
/*N*/ 				nStartRow = pArea->aStart.Row();
/*N*/ 				nEndCol = pArea->aEnd.Col();
/*N*/ 				nEndRow = pArea->aEnd.Row();
/*N*/ 			}
/*N*/ 		}			// sonst alles
/*N*/ 	}
/*N*/ 
/*N*/ 	// bSkipBreaks holen:
/*N*/ 
/*N*/ 	BOOL bSkipBreaks = FALSE;
/*N*/ 
/*N*/ 	if ( pStyleSet->GetItemState( ATTR_PAGE_SCALETOPAGES, FALSE, &pItem ) == SFX_ITEM_SET )
/*N*/ 	{
/*N*/ 		DBG_ASSERT( pItem->ISA(SfxUInt16Item), "falsches Item" );
/*N*/ 		bSkipBreaks = ( ((const SfxUInt16Item*)pItem)->GetValue() > 0 );
/*N*/ 	}
/*N*/ 
/*N*/ 	//--------------------------------------------------------------------------
/*N*/ 
/*N*/ 	long nPageSizeX = aPageSizeTwips.Width();
/*N*/ 	long nPageSizeY = aPageSizeTwips.Height();
/*N*/ 
/*N*/ 		//	Anfang: Breaks loeschen
/*N*/ 
/*N*/ 	for (nX=0; nX<nStartCol; nX++)
/*N*/ 		pColFlags[nX] &= ~CR_PAGEBREAK;
/*N*/ 	for (nY=0; nY<nStartRow; nY++)
/*N*/ 		pRowFlags[nY] &= ~CR_PAGEBREAK;
/*N*/ 
/*N*/ 	if (nStartCol)
/*N*/ 		pColFlags[nStartCol] |= CR_PAGEBREAK;			//! AREABREAK
/*N*/ 	if (nStartRow)
/*N*/ 		pRowFlags[nStartRow] |= CR_PAGEBREAK;			//! AREABREAK
/*N*/ 
/*N*/ 		//	Mittelteil: Breaks verteilen
/*N*/ 
/*N*/ 	BOOL bRepeatCol = ( nRepeatStartX != REPEAT_NONE );
/*N*/ 	BOOL bColFound = FALSE;
/*N*/ 	long nSizeX = 0;
/*N*/ 	for (nX=nStartCol; nX<=nEndCol; nX++)
/*N*/ 	{
/*N*/ 		BOOL bStartOfPage = FALSE;
/*N*/ 		long nThisX = ( pColFlags[nX] & CR_HIDDEN ) ? 0 : pColWidth[nX];
/*N*/ 		if ( (nSizeX+nThisX > nPageSizeX) || ((pColFlags[nX] & CR_MANUALBREAK) && !bSkipBreaks) )
/*N*/ 		{
/*N*/ 			pColFlags[nX] |= CR_PAGEBREAK;
/*N*/ 			nSizeX = 0;
/*N*/ 			bStartOfPage = TRUE;
/*N*/ 		}
/*N*/ 		else if (nX != nStartCol)
/*N*/ 			pColFlags[nX] &= ~CR_PAGEBREAK;
/*N*/ 		else
/*N*/ 			bStartOfPage = TRUE;
/*N*/ 
/*N*/ 		if ( bStartOfPage && bRepeatCol && nX>nRepeatStartX && !bColFound )
/*N*/ 		{
/*N*/ 			// subtract size of repeat columns from page size
/*N*/ 			for (i=nRepeatStartX; i<=nRepeatEndX; i++)
/*N*/ 				nPageSizeX -= ( pColFlags[i] & CR_HIDDEN ) ? 0 : pColWidth[i];
/*N*/ 			while (nX<=nRepeatEndX)
/*N*/ 				pColFlags[++nX] &= ~CR_PAGEBREAK;
/*N*/ 			bColFound = TRUE;
/*N*/ 		}
/*N*/ 
/*N*/ 		nSizeX += nThisX;
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bRepeatRow = ( nRepeatStartY != REPEAT_NONE );
/*N*/ 	BOOL bRowFound = FALSE;
/*N*/ 	long nSizeY = 0;
/*N*/ 	for (nY=nStartRow; nY<=nEndRow; nY++)
/*N*/ 	{
/*N*/ 		BOOL bStartOfPage = FALSE;
/*N*/ 		long nThisY = ( pRowFlags[nY] & CR_HIDDEN ) ? 0 : pRowHeight[nY];
/*N*/ 		if ( (nSizeY+nThisY > nPageSizeY) || ((pRowFlags[nY] & CR_MANUALBREAK) && !bSkipBreaks) )
/*N*/ 		{
/*N*/ 			pRowFlags[nY] |= CR_PAGEBREAK;
/*N*/ 			nSizeY = 0;
/*N*/ 			bStartOfPage = TRUE;
/*N*/ 		}
/*N*/ 		else if (nY != nStartRow)
/*N*/ 			pRowFlags[nY] &= ~CR_PAGEBREAK;
/*N*/ 		else
/*N*/ 			bStartOfPage = TRUE;
/*N*/ 
/*N*/ 		if ( bStartOfPage && bRepeatRow && nY>nRepeatStartY && !bRowFound )
/*N*/ 		{
/*?*/ 			// subtract size of repeat rows from page size
/*?*/ 			for (i=nRepeatStartY; i<=nRepeatEndY; i++)
/*?*/ 				nPageSizeY -= ( pRowFlags[i] & CR_HIDDEN ) ? 0 : pRowHeight[i];
/*?*/ 			while (nY<=nRepeatEndY)
/*?*/ 				pRowFlags[++nY] &= ~CR_PAGEBREAK;
/*?*/ 			bRowFound = TRUE;
/*N*/ 		}
/*N*/ 
/*N*/ 		nSizeY += nThisY;
/*N*/ 	}
/*N*/ 
/*N*/ 		//	Ende: Breaks loeschen
/*N*/ 
/*N*/ 	if (nEndCol < MAXCOL)
/*N*/ 	{
/*N*/ 		pColFlags[nEndCol+1] |= CR_PAGEBREAK;			//! AREABREAK
/*N*/ 		for (nX=nEndCol+2; nX<=MAXCOL; nX++)
/*N*/ 			pColFlags[nX] &= ~CR_PAGEBREAK;
/*N*/ 	}
/*N*/ 	if (nEndRow < MAXROW)
/*N*/ 	{
/*N*/ 		pRowFlags[nEndRow+1] |= CR_PAGEBREAK;			//! AREABREAK
/*N*/ 		for (nY=nEndRow+2; nY<=MAXROW; nY++)
/*N*/ 			pRowFlags[nY] &= ~CR_PAGEBREAK;
/*N*/ 	}
/*N*/ }

/*N*/ void ScTable::RemoveManualBreaks()
/*N*/ {
/*N*/ 	if (pColFlags)
/*N*/ 		for (USHORT nCol = 0; nCol <= MAXCOL; nCol++)
/*N*/ 			pColFlags[nCol] &= ~CR_MANUALBREAK;
/*N*/ 
/*N*/ 	if (pRowFlags)
/*N*/ 		for (USHORT nRow = 0; nRow <= MAXROW; nRow++)
/*N*/ 			pRowFlags[nRow] &= ~CR_MANUALBREAK;
/*N*/ }


/*N*/ void ScTable::SetPageSize( const Size& rSize )
/*N*/ {
/*N*/ 	if ( rSize.Width() != 0 && rSize.Height() != 0 )
/*N*/ 	{
/*N*/ 		bPageSizeValid = TRUE;
/*N*/ 		aPageSizeTwips = rSize;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bPageSizeValid = FALSE;
/*N*/ }

/*N*/ Size ScTable::GetPageSize() const
/*N*/ {
/*N*/ 	if ( bPageSizeValid )
/*N*/ 		return aPageSizeTwips;
/*N*/ 	else
/*N*/ 		return Size();	// leer
/*N*/ }

/*N*/ void ScTable::SetRepeatArea( USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nEndRow )
/*N*/ {
/*N*/ 	nRepeatStartX = nStartCol;
/*N*/ 	nRepeatEndX   = nEndCol;
/*N*/ 	nRepeatStartY = nStartRow;
/*N*/ 	nRepeatEndY   = nEndRow;
/*N*/ }

/*N*/ void ScTable::StartListening( const ScAddress& rAddress, SfxListener* pListener )
/*N*/ {
/*N*/ 	aCol[rAddress.Col()].StartListening( *pListener, rAddress.Row() );
/*N*/ }

/*N*/ void ScTable::EndListening( const ScAddress& rAddress, SfxListener* pListener )
/*N*/ {
/*N*/ 	aCol[rAddress.Col()].EndListening( *pListener, rAddress.Row() );
/*N*/ }

/*N*/ void ScTable::SetPageStyle( const String& rName )
/*N*/ {
/*N*/ 	if ( aPageStyle != rName )
/*N*/ 	{
/*N*/ 		String					aStrNew    = rName;
/*N*/ 		SfxStyleSheetBasePool*	pStylePool = pDocument->GetStyleSheetPool();
/*N*/ 		SfxStyleSheetBase*		pNewStyle  = pStylePool->Find( aStrNew, SFX_STYLE_FAMILY_PAGE );
/*N*/ 
/*N*/ 		if ( !pNewStyle )
/*N*/ 		{
/*?*/ 			aStrNew = ScGlobal::GetRscString(STR_STYLENAME_STANDARD);
/*?*/ 			pNewStyle = pStylePool->Find( aStrNew, SFX_STYLE_FAMILY_PAGE );
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( aPageStyle != aStrNew )
/*N*/ 		{
/*N*/ 			SfxStyleSheetBase* pOldStyle = pStylePool->Find( aPageStyle, SFX_STYLE_FAMILY_PAGE );
/*N*/ 
/*N*/ 			if ( pOldStyle && pNewStyle )
/*N*/ 			{
/*N*/ 				SfxItemSet&  rOldSet		  = pOldStyle->GetItemSet();
/*N*/ 				SfxItemSet&  rNewSet 		  = pNewStyle->GetItemSet();
/*N*/ 				const USHORT nOldScale		  = GET_SCALEVALUE(rOldSet,ATTR_PAGE_SCALE);
/*N*/ 				const USHORT nOldScaleToPages = GET_SCALEVALUE(rOldSet,ATTR_PAGE_SCALETOPAGES);
/*N*/ 				const USHORT nNewScale		  = GET_SCALEVALUE(rNewSet,ATTR_PAGE_SCALE);
/*N*/ 				const USHORT nNewScaleToPages = GET_SCALEVALUE(rNewSet,ATTR_PAGE_SCALETOPAGES);
/*N*/ 
/*N*/ 				if ( (nOldScale != nNewScale) || (nOldScaleToPages != nNewScaleToPages) )
/*N*/ 					InvalidateTextWidth();
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( pNewStyle )			// auch ohne den alten (fuer UpdateStdNames)
/*N*/ 				aPageStyle = aStrNew;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScTable::InvalidateTextWidth( const ScAddress* pAdrFrom,
/*N*/ 								   const ScAddress* pAdrTo,
/*N*/ 								   BOOL bBroadcast )
/*N*/ {
/*N*/ 	if ( pAdrFrom && !pAdrTo )
/*N*/ 	{
/*?*/ 		ScBaseCell* pCell = aCol[pAdrFrom->Col()].GetCell( pAdrFrom->Row() );
/*?*/ 		if ( pCell )
/*?*/ 		{
/*?*/ 			pCell->SetTextWidth( TEXTWIDTH_DIRTY );
/*?*/ 			pCell->SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
/*?*/ 			if ( bBroadcast )
/*?*/ 			{	// nur bei CalcAsShown
/*?*/ 				switch ( pCell->GetCellType() )
/*?*/ 				{
/*?*/ 					case CELLTYPE_VALUE :
/*?*/ 						pDocument->Broadcast( SC_HINT_DATACHANGED,
/*?*/ 							ScAddress( pAdrFrom->Col(), pAdrFrom->Row(), nTab ),
/*?*/ 							pCell );
/*?*/ 						break;
/*?*/ 					case CELLTYPE_FORMULA :
/*?*/ 						((ScFormulaCell*)pCell)->SetDirty();
/*?*/ 						break;
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const USHORT nColStart = pAdrFrom ? pAdrFrom->Col() : 0;
/*N*/ 		const USHORT nRowStart = pAdrFrom ? pAdrFrom->Row() : 0;
/*N*/ 		const USHORT nColEnd   = pAdrTo   ? pAdrTo->Col()   : MAXCOL;
/*N*/ 		const USHORT nRowEnd   = pAdrTo   ? pAdrTo->Row()   : MAXROW;
/*N*/ 
/*N*/ 		for ( USHORT nCol=nColStart; nCol<=nColEnd; nCol++ )
/*N*/ 		{
/*N*/ 			ScColumnIterator aIter( &aCol[nCol], nRowStart, nRowEnd );
/*N*/ 			ScBaseCell*		 pCell = NULL;
/*N*/ 			USHORT			 nRow  = nRowStart;
/*N*/ 
/*N*/ 			while ( aIter.Next( nRow, pCell ) )
/*N*/ 			{
/*N*/ 				pCell->SetTextWidth( TEXTWIDTH_DIRTY );
/*N*/ 				pCell->SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
/*N*/ 				if ( bBroadcast )
/*N*/ 				{	// nur bei CalcAsShown
/*?*/ 					switch ( pCell->GetCellType() )
/*?*/ 					{
/*?*/ 						case CELLTYPE_VALUE :
/*?*/ 							pDocument->Broadcast( SC_HINT_DATACHANGED,
/*?*/ 								ScAddress( nCol, nRow, nTab ), pCell );
/*?*/ 							break;
/*?*/ 						case CELLTYPE_FORMULA :
/*?*/ 							((ScFormulaCell*)pCell)->SetDirty();
/*?*/ 							break;
/*?*/ 					}
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }





}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
