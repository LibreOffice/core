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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifdef WTC
#define private public
#endif

#include "hintids.hxx"

#ifndef _SVX_BOXITEM_HXX //autogen
#include <bf_svx/boxitem.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _CELLFRM_HXX //autogen
#include <cellfrm.hxx>
#endif
#ifndef _TABFRM_HXX //autogen
#include <tabfrm.hxx>
#endif
#ifndef _TXTFRM_HXX //autogen
#include <txtfrm.hxx>
#endif

#include "doc.hxx"
#include "viscrs.hxx"
#include "swtblfmt.hxx"
namespace binfilter {


extern void ClearFEShellTabCols();

//siehe auch swtable.cxx
#define COLFUZZY 20L







/*N*/ void lcl_GetStartEndCell( const SwCursor& rCrsr,
/*N*/ 						SwLayoutFrm *&prStart, SwLayoutFrm *&prEnd )
/*N*/ {
/*N*/ 	ASSERT( rCrsr.GetCntntNode() && rCrsr.GetCntntNode( FALSE ),
/*N*/ 			"Tabselection nicht auf Cnt." );
/*N*/ 
/*N*/ 	Point aPtPos, aMkPos;
/*N*/ 	const SwShellCrsr* pShCrsr = rCrsr;
/*N*/ 	if( pShCrsr )
/*N*/ 	{
/*N*/ 		aPtPos = pShCrsr->GetPtPos();
/*N*/ 		aMkPos = pShCrsr->GetMkPos();
/*N*/ 	}
/*N*/ 
/*N*/ 	prStart = rCrsr.GetCntntNode()->GetFrm( &aPtPos )->GetUpper();
/*N*/ 	prEnd	= rCrsr.GetCntntNode(FALSE)->GetFrm( &aMkPos )->GetUpper();
/*N*/ }

/***********************************************************************
#*	Class	   :  SwDoc
#*	Methoden   :  SetRowHeight(), GetRowHeight()
#*	Datum	   :  MA 17. May. 93
#*	Update	   :  JP 28.04.98
#***********************************************************************/
//Die Zeilenhoehe wird ausgehend von der Selektion ermittelt/gesetzt.
//Ausgehend von jeder Zelle innerhalb der Selektion werden nach oben alle
//Zeilen abgeklappert, die oberste Zeile erhaelt den gewuenschten Wert alle
//tieferliegenden Zeilen einen entsprechenden Wert der sich aus der
//Relation der alten und neuen Groesse der obersten Zeile und ihrer
//eigenen Groesse ergiebt.
//Alle veraenderten Zeilen erhalten ggf. ein eigenes FrmFmt.
//Natuerlich darf jede Zeile nur einmal angefasst werden.

/*N*/ inline void InsertLine( SvPtrarr& rLineArr, SwTableLine* pLine )
/*N*/ {
/*N*/ 	if( USHRT_MAX == rLineArr.GetPos( (void*&)pLine ) )
/*N*/ 		rLineArr.Insert( (void*&)pLine, rLineArr.Count() );
/*N*/ }

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

/*N*/ struct LinesAndTable
/*N*/ {
/*N*/ 		  SvPtrarr &rLines;
/*N*/ 	const SwTable  &rTable;
/*N*/ 		  BOOL		bInsertLines;
/*N*/ 
/*N*/ 	LinesAndTable( SvPtrarr &rL, const SwTable &rTbl ) :
/*N*/ 		  rLines( rL ), rTable( rTbl ), bInsertLines( TRUE ) {}
/*N*/ };


/*N*/ BOOL _FindLine( const _FndLine*& rpLine, void* pPara );

/*N*/ BOOL _FindBox( const _FndBox*& rpBox, void* pPara )
/*N*/ {
/*N*/ 	if ( rpBox->GetLines().Count() )
/*N*/ 	{
/*N*/ 		((LinesAndTable*)pPara)->bInsertLines = TRUE;
/*N*/ 		((_FndBox*)rpBox)->GetLines().ForEach( _FindLine, pPara );
/*N*/ 		if ( ((LinesAndTable*)pPara)->bInsertLines )
/*N*/ 		{
/*N*/ 			const SwTableLines &rLines = rpBox->GetBox()
/*N*/ 									? rpBox->GetBox()->GetTabLines()
/*N*/ 									: ((LinesAndTable*)pPara)->rTable.GetTabLines();
/*N*/ 			if ( rpBox->GetLines().Count() == rLines.Count() )
/*N*/ 			{
/*N*/ 				for ( USHORT i = 0; i < rLines.Count(); ++i )
/*N*/ 					::binfilter::InsertLine( ((LinesAndTable*)pPara)->rLines,
/*N*/ 								  (SwTableLine*)rLines[i] );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				((LinesAndTable*)pPara)->bInsertLines = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( rpBox->GetBox() )
/*N*/ 		::binfilter::InsertLine( ((LinesAndTable*)pPara)->rLines,
/*N*/ 					  (SwTableLine*)rpBox->GetBox()->GetUpper() );
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL _FindLine( const _FndLine*& rpLine, void* pPara )
/*N*/ {
/*N*/ 	((_FndLine*)rpLine)->GetBoxes().ForEach( _FindBox, pPara );
/*N*/ 	return TRUE;
/*N*/ }


/***********************************************************************
#*	Class	   :  SwDoc
#*	Methoden   :  SetTabBorders(), GetTabBorders()
#*	Datum	   :  MA 18. May. 93
#*	Update	   :  JP 29.04.98
#***********************************************************************/
/*N*/ inline void InsertCell( SvPtrarr& rCellArr, SwCellFrm* pCellFrm )
/*N*/ {
/*N*/ 	if( USHRT_MAX == rCellArr.GetPos( (void*&)pCellFrm ) )
/*N*/ 		rCellArr.Insert( (void*&)pCellFrm, rCellArr.Count() );
/*N*/ }

//-----------------------------------------------------------------------------
/*N*/ void lcl_CollectCells( SvPtrarr &rArr, const SwRect &rUnion,
/*N*/ 						  SwTabFrm *pTab )
/*N*/ {
/*N*/ 	SwLayoutFrm *pCell = pTab->FirstCell();
/*N*/ 	do
/*N*/ 	{
/*N*/ 		// Wenn in der Zelle ein spaltiger Bereich sitzt, muessen wir
/*N*/ 		// uns erst wieder zur Zelle hochhangeln
/*N*/ 		while ( !pCell->IsCellFrm() )
/*?*/ 			pCell = pCell->GetUpper();
/*N*/ 		ASSERT( pCell, "Frame ist keine Zelle." );
/*N*/ 		if ( rUnion.IsOver( pCell->Frm() ) )
/*N*/ 			::binfilter::InsertCell( rArr, (SwCellFrm*)pCell );
/*N*/ 		//Dafuer sorgen, dass die Zelle auch verlassen wird (Bereiche)
/*N*/ 		SwLayoutFrm *pTmp = pCell;
/*N*/ 		do
/*N*/ 		{	pTmp = pTmp->GetNextLayoutLeaf();
/*N*/ 		} while ( pCell->IsAnLower( pTmp ) );
/*N*/ 		pCell = pTmp;
/*N*/ 	} while( pCell && pTab->IsAnLower( pCell ) );
/*N*/ }




/*N*/ void SwDoc::GetTabBorders( const SwCursor& rCursor, SfxItemSet& rSet ) const
/*N*/ {
/*N*/ 	SwTableNode* pTblNd = rCursor.GetPoint()->nNode.GetNode().FindTableNode();
/*N*/ 	if( !pTblNd )
/*?*/ 		return ;
/*N*/ 
/*N*/ 	SwLayoutFrm *pStart, *pEnd;
/*N*/ 	::binfilter::lcl_GetStartEndCell( rCursor, pStart, pEnd );
/*N*/ 
/*N*/ 	SwSelUnions aUnions;
/*N*/ 	::binfilter::MakeSelUnions( aUnions, pStart, pEnd );
/*N*/ 
/*N*/ 	if( aUnions.Count() )
/*N*/ 	{
/*N*/ 		SvxBoxItem	   aSetBox	  ((const SvxBoxItem	&) rSet.Get(RES_BOX    ));
/*N*/ 		SvxBoxInfoItem aSetBoxInfo((const SvxBoxInfoItem&) rSet.Get(SID_ATTR_BORDER_INNER));
/*N*/ 
/*N*/ 		BOOL bTopSet	  =	FALSE,
/*N*/ 			 bBottomSet   =	FALSE,
/*N*/ 			 bLeftSet	  =	FALSE,
/*N*/ 			 bRightSet	  =	FALSE,
/*N*/ 			 bHoriSet	  = FALSE,
/*N*/ 			 bVertSet	  = FALSE,
/*N*/ 			 bDistanceSet = FALSE;
/*N*/ 
/*N*/ 		aSetBoxInfo.ResetFlags();
/*N*/ 
/*N*/ 		for ( USHORT i = 0; i < aUnions.Count(); ++i )
/*N*/ 		{
/*N*/ 			SwSelUnion *pUnion = aUnions[i];
/*N*/ 			const SwTabFrm *pTab = pUnion->GetTable();
/*N*/ 			const SwRect &rUnion = pUnion->GetUnion();
/*N*/ 			const BOOL bFirst = i == 0 ? TRUE : FALSE;
/*N*/ 			const BOOL bLast  = i == aUnions.Count() - 1 ? TRUE : FALSE;
/*N*/ 
/*N*/ 			SvPtrarr aCellArr( 255, 255 );
/*N*/ 			::binfilter::lcl_CollectCells( aCellArr, rUnion, (SwTabFrm*)pTab );
/*N*/ 
/*N*/ 			for ( USHORT j = 0; j < aCellArr.Count(); ++j )
/*N*/ 			{
/*N*/ 				const SwCellFrm *pCell = (const SwCellFrm*)aCellArr[j];
/*N*/                 const sal_Bool bVert = pCell->IsVertical();
/*N*/                 const sal_Bool bRTL = pCell->IsRightToLeft();
/*N*/                 sal_Bool bTopOver, bLeftOver, bRightOver, bBottomOver;
/*N*/                 if ( bVert )
/*N*/                 {
/*N*/                     bTopOver = pCell->Frm().Right() >= rUnion.Right();
/*N*/                     bLeftOver = pCell->Frm().Top() <= rUnion.Top();
/*N*/                     bRightOver = pCell->Frm().Bottom() >= rUnion.Bottom();
/*N*/                     bBottomOver = pCell->Frm().Left() <= rUnion.Left();
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     bTopOver = pCell->Frm().Top() <= rUnion.Top();
/*N*/                     bLeftOver = pCell->Frm().Left() <= rUnion.Left();
/*N*/                     bRightOver = pCell->Frm().Right() >= rUnion.Right();
/*N*/                     bBottomOver = pCell->Frm().Bottom() >= rUnion.Bottom();
/*N*/                 }
/*N*/ 
/*N*/                 if ( bRTL )
/*N*/                 {
/*N*/                     sal_Bool bTmp = bRightOver;
/*N*/                     bRightOver = bLeftOver;
/*N*/                     bLeftOver = bTmp;
/*N*/                 }
/*N*/ 
/*N*/ 				const SwFrmFmt	*pFmt  = pCell->GetFmt();
/*N*/ 				const SvxBoxItem  &rBox  = pFmt->GetBox();
/*N*/ 
/*N*/ 				//Obere Kante
/*N*/ 			    if ( bFirst && bTopOver )
/*N*/ 				{
/*N*/ 					if (aSetBoxInfo.IsValid(VALID_TOP))
/*N*/ 					{
/*N*/ 						if ( !bTopSet )
/*N*/ 						{	bTopSet = TRUE;
/*N*/ 							aSetBox.SetLine( rBox.GetTop(), BOX_LINE_TOP );
/*N*/ 						}
/*?*/ 						else if ((aSetBox.GetTop() && rBox.GetTop() &&
/*?*/ 								 !(*aSetBox.GetTop() == *rBox.GetTop())) ||
/*?*/ 								 ((!aSetBox.GetTop()) ^ (!rBox.GetTop()))) // XOR-Ausdruck ist TRUE, wenn genau einer der beiden Pointer 0 ist
/*?*/ 						{
/*?*/ 							aSetBoxInfo.SetValid(VALID_TOP, FALSE );
/*?*/ 							aSetBox.SetLine( 0, BOX_LINE_TOP );
/*?*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				//Linke Kante
/*N*/ 				if ( bLeftOver )
/*N*/ 				{
/*N*/ 					if (aSetBoxInfo.IsValid(VALID_LEFT))
/*N*/ 					{
/*N*/ 						if ( !bLeftSet )
/*N*/ 						{	bLeftSet = TRUE;
/*N*/ 							aSetBox.SetLine( rBox.GetLeft(), BOX_LINE_LEFT );
/*N*/ 						}
/*N*/ 						else if ((aSetBox.GetLeft() && rBox.GetLeft() &&
/*?*/ 								 !(*aSetBox.GetLeft() == *rBox.GetLeft())) ||
/*?*/ 								 ((!aSetBox.GetLeft()) ^ (!rBox.GetLeft())))
/*?*/ 						{
/*?*/ 							aSetBoxInfo.SetValid(VALID_LEFT, FALSE );
/*?*/ 							aSetBox.SetLine( 0, BOX_LINE_LEFT );
/*?*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*?*/ 					if (aSetBoxInfo.IsValid(VALID_VERT))
/*?*/ 					{
/*?*/ 						if ( !bVertSet )
/*?*/ 						{	bVertSet = TRUE;
/*?*/ 							aSetBoxInfo.SetLine( rBox.GetLeft(), BOXINFO_LINE_VERT );
/*?*/ 						}
/*?*/ 						else if ((aSetBoxInfo.GetVert() && rBox.GetLeft() &&
/*?*/ 								 !(*aSetBoxInfo.GetVert() == *rBox.GetLeft())) ||
/*?*/ 								 ((!aSetBoxInfo.GetVert()) ^ (!rBox.GetLeft())))
/*?*/ 						{	aSetBoxInfo.SetValid( VALID_VERT, FALSE );
/*?*/ 							aSetBoxInfo.SetLine( 0, BOXINFO_LINE_VERT );
/*?*/ 						}
/*?*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				//Rechte Kante
/*N*/ 				if ( aSetBoxInfo.IsValid(VALID_RIGHT) && bRightOver )
/*N*/ 				{
/*N*/ 					if ( !bRightSet )
/*N*/ 					{	bRightSet = TRUE;
/*N*/ 						aSetBox.SetLine( rBox.GetRight(), BOX_LINE_RIGHT );
/*N*/ 					}
/*N*/ 					else if ((aSetBox.GetRight() && rBox.GetRight() &&
/*?*/ 							 !(*aSetBox.GetRight() == *rBox.GetRight())) ||
/*?*/ 							 (!aSetBox.GetRight() ^ !rBox.GetRight()))
/*?*/ 					{	aSetBoxInfo.SetValid( VALID_RIGHT, FALSE );
/*?*/ 						aSetBox.SetLine( 0, BOX_LINE_RIGHT );
/*?*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				//Untere Kante
/*N*/ 				if ( bLast && bBottomOver )
/*N*/ 				{
/*N*/ 					if ( aSetBoxInfo.IsValid(VALID_BOTTOM) )
/*N*/ 					{
/*N*/ 						if ( !bBottomSet )
/*N*/ 						{	bBottomSet = TRUE;
/*N*/ 							aSetBox.SetLine( rBox.GetBottom(), BOX_LINE_BOTTOM );
/*N*/ 						}
/*N*/ 						else if ((aSetBox.GetBottom() && rBox.GetBottom() &&
/*?*/ 								 !(*aSetBox.GetBottom() == *rBox.GetBottom())) ||
/*?*/ 								 (!aSetBox.GetBottom() ^ !rBox.GetBottom()))
/*?*/ 						{	aSetBoxInfo.SetValid( VALID_BOTTOM, FALSE );
/*?*/ 							aSetBox.SetLine( 0, BOX_LINE_BOTTOM );
/*?*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				//in allen Zeilen ausser der letzten werden die
/*N*/ 				// horiz. Linien aus der Bottom-Linie entnommen
/*N*/ 				else
/*N*/ 				{
/*?*/ 					if (aSetBoxInfo.IsValid(VALID_HORI))
/*?*/ 					{
/*?*/ 						if ( !bHoriSet )
/*?*/ 						{	bHoriSet = TRUE;
/*?*/ 							aSetBoxInfo.SetLine( rBox.GetBottom(), BOXINFO_LINE_HORI );
/*?*/ 						}
/*?*/ 						else if ((aSetBoxInfo.GetHori() && rBox.GetBottom() &&
/*?*/ 								 !(*aSetBoxInfo.GetHori() == *rBox.GetBottom())) ||
/*?*/ 								 ((!aSetBoxInfo.GetHori()) ^ (!rBox.GetBottom())))
/*?*/ 						{
/*?*/ 							aSetBoxInfo.SetValid( VALID_HORI, FALSE );
/*?*/ 							aSetBoxInfo.SetLine( 0, BOXINFO_LINE_HORI );
/*?*/ 						}
/*?*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				// Abstand zum Text
/*N*/ 				if (aSetBoxInfo.IsValid(VALID_DISTANCE))
/*N*/ 				{
/*N*/ 					static USHORT __READONLY_DATA aBorders[] = {
/*N*/ 						BOX_LINE_BOTTOM, BOX_LINE_TOP,
/*N*/ 						BOX_LINE_RIGHT, BOX_LINE_LEFT };
/*N*/ 					const USHORT* pBrd = aBorders;
/*N*/ 
/*N*/ 					if( !bDistanceSet )		// bei 1. Durchlauf erstmal setzen
/*N*/ 					{
/*N*/ 						bDistanceSet = TRUE;
/*N*/ 						for( int i = 0; i < 4; ++i, ++pBrd )
/*N*/ 							aSetBox.SetDistance( rBox.GetDistance( *pBrd ),
/*N*/ 												*pBrd );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*?*/ 						for( int i = 0; i < 4; ++i, ++pBrd )
/*?*/ 							if( aSetBox.GetDistance( *pBrd ) !=
/*?*/ 								rBox.GetDistance( *pBrd ) )
/*?*/ 							{
/*?*/ 								aSetBoxInfo.SetValid( VALID_DISTANCE, FALSE );
/*?*/ 								aSetBox.SetDistance( (USHORT) 0 );
/*?*/ 								break;
/*?*/ 							}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		rSet.Put( aSetBox );
/*N*/ 		rSet.Put( aSetBoxInfo );
/*N*/ 	}
/*N*/ }

}
