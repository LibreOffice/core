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
#include <bf_svtools/colorcfg.hxx>
#include <bf_svx/outlobj.hxx>

#include <bf_svx/xdef.hxx>

#include <bf_svx/svditer.hxx>
#include <bf_svx/svdocapt.hxx>
#include <bf_svx/svdocirc.hxx>
#include <bf_svx/svdopath.hxx>
#include <bf_svx/svdpage.hxx>
#include <bf_svx/svdundo.hxx>
#include <bf_svx/xflclit.hxx>
#include <bf_svx/xlnclit.hxx>
#include <bf_svx/xlnedcit.hxx>
#include <bf_svx/xlnedit.hxx>
#include <bf_svx/xlnedwit.hxx>
#include <bf_svx/xlnstcit.hxx>
#include <bf_svx/xlnstit.hxx>
#include <bf_svx/xlnstwit.hxx>
#include <bf_svx/xlnwtit.hxx>

#include "detfunc.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "validat.hxx"
#include "cell.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "scmod.hxx"
namespace binfilter {

//------------------------------------------------------------------------

// #99319# line ends are now created with an empty name.
// The checkForUniqueItem method then finds a unique name for the item's value.
#define SC_LINEEND_NAME		EMPTY_STRING

//------------------------------------------------------------------------

enum DetInsertResult {				// Return-Werte beim Einfuegen in einen Level
            DET_INS_CONTINUE,
            DET_INS_INSERTED,
            DET_INS_EMPTY,
            DET_INS_CIRCULAR };

//	maximale Textlaenge (Zeichen), die noch in "kleines" Objekt passt
#define SC_NOTE_SMALLTEXT	100

//------------------------------------------------------------------------

//-/class ScPublicAttrObj : public SdrAttrObj
//-/{
//-/private:
//-/	ScPublicAttrObj() {}						// wird nicht angelegt
//-/public:
//-/	const XLineAttrSetItem* GetLineAttr()		{ return pLineAttr; }
//-/};

//------------------------------------------------------------------------

/*N*/ class ScDetectiveData
/*N*/ {
/*N*/ private:
/*N*/ 	SfxItemSet	aBoxSet;
/*N*/ 	SfxItemSet	aArrowSet;
/*N*/ 	SfxItemSet	aToTabSet;
/*N*/ 	SfxItemSet	aFromTabSet;
/*N*/ 	SfxItemSet	aCircleSet;			//! einzeln ?
/*N*/ 	USHORT		nMaxLevel;
/*N*/ 
/*N*/ public:
/*N*/ 				ScDetectiveData( SdrModel* pModel );
/*N*/ 
/*N*/ 	SfxItemSet&	GetBoxSet()		{ return aBoxSet; }
/*N*/ 	SfxItemSet&	GetArrowSet()	{ return aArrowSet; }
/*N*/ 	SfxItemSet&	GetToTabSet()	{ return aToTabSet; }
/*N*/ 	SfxItemSet&	GetFromTabSet()	{ return aFromTabSet; }
/*N*/ 	SfxItemSet&	GetCircleSet()	{ return aCircleSet; }
/*N*/ 
/*N*/ 	void		SetMaxLevel( USHORT nVal )		{ nMaxLevel = nVal; }
/*N*/ 	USHORT		GetMaxLevel() const				{ return nMaxLevel; }
/*N*/ };

/*N*/ class ScCommentData
/*N*/ {
/*N*/ private:
/*N*/ 	SfxItemSet	aCaptionSet;
/*N*/ 
/*N*/ public:
/*N*/ 				ScCommentData( ScDocument* pDoc, SdrModel* pModel );
/*N*/ 
/*N*/ 	SfxItemSet&	GetCaptionSet()	{ return aCaptionSet; }
/*N*/ };

//------------------------------------------------------------------------

/*N*/ ColorData ScDetectiveFunc::nArrowColor = 0;
/*N*/ ColorData ScDetectiveFunc::nErrorColor = 0;
/*N*/ ColorData ScDetectiveFunc::nCommentColor = 0;
/*N*/ BOOL ScDetectiveFunc::bColorsInitialized = FALSE;

//------------------------------------------------------------------------

/*N*/ BOOL lcl_HasThickLine( SdrObject& rObj ) // detective.sdc
/*N*/ {
/*N*/ 	// thin lines get width 0 -> everything greater 0 is a thick line
/*N*/ 
/*N*/ 	return ( ((const XLineWidthItem&)rObj.GetItem(XATTR_LINEWIDTH)).GetValue() > 0 );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScDetectiveData::ScDetectiveData( SdrModel* pModel ) :
/*N*/ 	aBoxSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
/*N*/ 	aArrowSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
/*N*/ 	aToTabSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
/*N*/ 	aFromTabSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
/*N*/ 	aCircleSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END )
/*N*/ {
/*N*/ 	nMaxLevel = 0;
/*N*/ 
/*N*/ 	aBoxSet.Put( XLineColorItem( EMPTY_STRING, Color( ScDetectiveFunc::GetArrowColor() ) ) );
/*N*/ 	aBoxSet.Put( XFillStyleItem( XFILL_NONE ) );
/*N*/ 
/*N*/ 	//	#66479# Standard-Linienenden (wie aus XLineEndList::Create) selber zusammenbasteln,
/*N*/ 	//	um von den konfigurierten Linienenden unabhaengig zu sein
/*N*/ 
/*N*/ 	XPolygon aTriangle(4);
/*N*/ 	aTriangle[0].X()=10; aTriangle[0].Y()= 0;
/*N*/ 	aTriangle[1].X()= 0; aTriangle[1].Y()=30;
/*N*/ 	aTriangle[2].X()=20; aTriangle[2].Y()=30;
/*N*/ 	aTriangle[3].X()=10; aTriangle[3].Y()= 0;	// #99319# line end polygon must be closed
/*N*/ 
/*N*/ 	XPolygon aSquare(5);
/*N*/ 	aSquare[0].X()= 0; aSquare[0].Y()= 0;
/*N*/ 	aSquare[1].X()=10; aSquare[1].Y()= 0;
/*N*/ 	aSquare[2].X()=10; aSquare[2].Y()=10;
/*N*/ 	aSquare[3].X()= 0; aSquare[3].Y()=10;
/*N*/ 	aSquare[4].X()= 0; aSquare[4].Y()= 0;		// #99319# line end polygon must be closed
/*N*/ 
/*N*/ 	XPolygon aCircle(Point(0,0),100,100);
/*N*/ 
/*N*/ 	String aName = SC_LINEEND_NAME;
/*N*/ 
/*N*/ 	aArrowSet.Put( XLineStartItem( aName, aCircle ) );
/*N*/ 	aArrowSet.Put( XLineStartWidthItem( 200 ) );
/*N*/ 	aArrowSet.Put( XLineStartCenterItem( TRUE ) );
/*N*/ 	aArrowSet.Put( XLineEndItem( aName, aTriangle ) );
/*N*/ 	aArrowSet.Put( XLineEndWidthItem( 200 ) );
/*N*/ 	aArrowSet.Put( XLineEndCenterItem( FALSE ) );
/*N*/ 
/*N*/ 	aToTabSet.Put( XLineStartItem( aName, aCircle ) );
/*N*/ 	aToTabSet.Put( XLineStartWidthItem( 200 ) );
/*N*/ 	aToTabSet.Put( XLineStartCenterItem( TRUE ) );
/*N*/ 	aToTabSet.Put( XLineEndItem( aName, aSquare ) );
/*N*/ 	aToTabSet.Put( XLineEndWidthItem( 300 ) );
/*N*/ 	aToTabSet.Put( XLineEndCenterItem( FALSE ) );
/*N*/ 
/*N*/ 	aFromTabSet.Put( XLineStartItem( aName, aSquare ) );
/*N*/ 	aFromTabSet.Put( XLineStartWidthItem( 300 ) );
/*N*/ 	aFromTabSet.Put( XLineStartCenterItem( TRUE ) );
/*N*/ 	aFromTabSet.Put( XLineEndItem( aName, aTriangle ) );
/*N*/ 	aFromTabSet.Put( XLineEndWidthItem( 200 ) );
/*N*/ 	aFromTabSet.Put( XLineEndCenterItem( FALSE ) );
/*N*/ 
/*N*/ 	aCircleSet.Put( XLineColorItem( String(), Color( ScDetectiveFunc::GetErrorColor() ) ) );
/*N*/ 	aCircleSet.Put( XFillStyleItem( XFILL_NONE ) );
/*N*/ 	USHORT nWidth = 55;		// 54 = 1 Pixel
/*N*/ 	aCircleSet.Put( XLineWidthItem( nWidth ) );
/*N*/ }

/*N*/ ScCommentData::ScCommentData( ScDocument* pDoc, SdrModel* pModel ) :
/*N*/ 	aCaptionSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END,
/*N*/ 										EE_CHAR_START, EE_CHAR_END, 0 )
/*N*/ {
/*N*/ 	XPolygon aTriangle(4);
/*N*/ 	aTriangle[0].X()=10; aTriangle[0].Y()= 0;
/*N*/ 	aTriangle[1].X()= 0; aTriangle[1].Y()=30;
/*N*/ 	aTriangle[2].X()=20; aTriangle[2].Y()=30;
/*N*/ 	aTriangle[3].X()=10; aTriangle[3].Y()= 0;	// #99319# line end polygon must be closed
/*N*/ 
/*N*/ 	String aName = SC_LINEEND_NAME;
/*N*/ 
/*N*/ 	aCaptionSet.Put( XLineStartItem( aName, aTriangle ) );
/*N*/ 	aCaptionSet.Put( XLineStartWidthItem( 200 ) );
/*N*/ 	aCaptionSet.Put( XLineStartCenterItem( FALSE ) );
/*N*/ 	aCaptionSet.Put( XFillStyleItem( XFILL_SOLID ) );
/*N*/ 	Color aYellow( ScDetectiveFunc::GetCommentColor() );
/*N*/ 	aCaptionSet.Put( XFillColorItem( String(), aYellow ) );
/*N*/ 
/*N*/ 	//	shadow
/*N*/ 	//	SdrShadowItem has FALSE, instead the shadow is set for the rectangle
/*N*/ 	//	only with SetSpecialTextBoxShadow when the object is created
/*N*/ 	//	(item must be set to adjust objects from older files)
/*N*/ 	aCaptionSet.Put( SdrShadowItem( FALSE ) );
/*N*/ 	aCaptionSet.Put( SdrShadowXDistItem( 100 ) );
/*N*/ 	aCaptionSet.Put( SdrShadowYDistItem( 100 ) );
/*N*/ 
/*N*/ 	//	text attributes
/*N*/ 	aCaptionSet.Put( SdrTextLeftDistItem( 100 ) );
/*N*/ 	aCaptionSet.Put( SdrTextRightDistItem( 100 ) );
/*N*/ 	aCaptionSet.Put( SdrTextUpperDistItem( 100 ) );
/*N*/ 	aCaptionSet.Put( SdrTextLowerDistItem( 100 ) );
/*N*/ 
/*N*/ 	//	#78943# do use the default cell style, so the user has a chance to
/*N*/ 	//	modify the font for the annotations
/*N*/ 	((const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).
/*N*/ 		FillEditItemSet( &aCaptionSet );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ inline BOOL Intersect( USHORT nStartCol1, USHORT nStartRow1, USHORT nEndCol1, USHORT nEndRow1,
/*N*/ 						USHORT nStartCol2, USHORT nStartRow2, USHORT nEndCol2, USHORT nEndRow2 )
/*N*/ {
/*N*/ 	return nEndCol1 >= nStartCol2 && nEndCol2 >= nStartCol1 &&
/*N*/ 			nEndRow1 >= nStartRow2 && nEndRow2 >= nStartRow1;
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::HasError( const ScTripel& rStart, const ScTripel& rEnd, ScTripel& rErrPos )
/*N*/ {
/*N*/ 	rErrPos = rStart;
/*N*/ 	USHORT nError = 0;
/*N*/ 
/*N*/ 	ScCellIterator aCellIter( pDoc, rStart.GetCol(), rStart.GetRow(), rStart.GetTab(),
/*N*/ 									rEnd.GetCol(), rEnd.GetRow(), rEnd.GetTab() );
/*N*/ 	ScBaseCell* pCell = aCellIter.GetFirst();
/*N*/ 	while (pCell)
/*N*/ 	{
/*N*/ 		if (pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 		{
/*N*/ 			nError = ((ScFormulaCell*)pCell)->GetErrCode();
/*N*/ 			if (nError)
/*N*/ 				rErrPos.Put( aCellIter.GetCol(), aCellIter.GetRow(), aCellIter.GetTab() );
/*N*/ 		}
/*N*/ 		pCell = aCellIter.GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	return (nError != 0);
/*N*/ }

/*N*/ Point ScDetectiveFunc::GetDrawPos( USHORT nCol, USHORT nRow, BOOL bArrow )
/*N*/ {
/*N*/ 	//	MAXCOL/ROW+1 ist erlaubt fuer Ende von Rahmen
/*N*/ 	if (nCol > MAXCOL+1)
/*N*/ 	{
/*N*/ 		DBG_ERROR("falsche Col in ScDetectiveFunc::GetDrawPos");
/*N*/ 		nCol = MAXCOL+1;
/*N*/ 	}
/*N*/ 	if (nRow > MAXROW+1)
/*N*/ 	{
/*N*/ 		DBG_ERROR("falsche Row in ScDetectiveFunc::GetDrawPos");
/*N*/ 		nRow = MAXROW+1;
/*N*/ 	}
/*N*/ 
/*N*/ 	Point aPos;
/*N*/ 	USHORT i;
/*N*/ 	USHORT nLocalTab = nTab;		// nicht ueber this
/*N*/ 
/*N*/ 	for (i=0; i<nCol; i++)
/*N*/ 		aPos.X() += pDoc->GetColWidth( i,nLocalTab );
/*N*/ 	for (i=0; i<nRow; i++)
/*N*/ 		aPos.Y() += pDoc->FastGetRowHeight( i,nLocalTab );
/*N*/ 
/*N*/ 	if (bArrow)
/*N*/ 	{
/*N*/ 		if (nCol<=MAXCOL)
/*N*/ 			aPos.X() += pDoc->GetColWidth( nCol, nLocalTab ) / 4;
/*N*/ 		if (nCol<=MAXROW)
/*N*/ 			aPos.Y() += pDoc->GetRowHeight( nRow, nLocalTab ) / 2;
/*N*/ 	}
/*N*/ 
/*N*/ 	aPos.X() = (long) ( aPos.X() * HMM_PER_TWIPS );
/*N*/ 	aPos.Y() = (long) ( aPos.Y() * HMM_PER_TWIPS );
/*N*/ 
/*N*/ 	return aPos;
/*N*/ }

/*N*/ BOOL lcl_IsOtherTab( const XPolygon& rPolygon )
/*N*/ {
/*N*/ 	//	test if rPolygon is the line end for "other table" (rectangle)
/*N*/ 
/*N*/ 	USHORT nCount = rPolygon.GetPointCount();
/*N*/ 	if ( nCount == 4 )
/*N*/ 	{
/*N*/ 		//	4 points -> it is a rectangle (not closed) only if the first and last point are different
/*N*/ 
/*N*/ 		return rPolygon[0] != rPolygon[3];
/*N*/ 	}
/*N*/ 	else if ( nCount == 5 )
/*N*/ 	{
/*N*/ 		//	5 points -> it is a rectangle (closed) only if the first and last point are equal
/*N*/ 
/*N*/ 		return rPolygon[0] == rPolygon[4];
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::HasArrow( USHORT nStartCol, USHORT nStartRow, USHORT nStartTab,
/*N*/ 									USHORT nEndCol, USHORT nEndRow, USHORT nEndTab )
/*N*/ {
/*N*/ 	BOOL bStartAlien = ( nStartTab != nTab );
/*N*/ 	BOOL bEndAlien   = ( nEndTab != nTab );
/*N*/ 
/*N*/ 	if (bStartAlien && bEndAlien)
/*N*/ 	{
/*N*/ 		DBG_ERROR("bStartAlien && bEndAlien");
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	Rectangle aStartRect;
/*N*/ 	Rectangle aEndRect;
/*N*/ 	if (!bStartAlien)
/*N*/ 	{
/*N*/ 		Point aStartPos = GetDrawPos( nStartCol, nStartRow, FALSE );
/*N*/ 		Size aStartSize = Size(
/*N*/ 							(long) ( pDoc->GetColWidth( nStartCol, nTab) * HMM_PER_TWIPS ),
/*N*/ 							(long) ( pDoc->GetRowHeight( nStartRow, nTab) * HMM_PER_TWIPS ) );
/*N*/ 		aStartRect = Rectangle( aStartPos, aStartSize );
/*N*/ 	}
/*N*/ 	if (!bEndAlien)
/*N*/ 	{
/*N*/ 		Point aEndPos = GetDrawPos( nEndCol, nEndRow, FALSE );
/*N*/ 		Size aEndSize = Size(
/*N*/ 							(long) ( pDoc->GetColWidth( nEndCol, nTab) * HMM_PER_TWIPS ),
/*N*/ 							(long) ( pDoc->GetRowHeight( nEndRow, nTab) * HMM_PER_TWIPS ) );
/*N*/ 		aEndRect = Rectangle( aEndPos, aEndSize );
/*N*/ 	}
/*N*/ 
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	SdrObjListIter aIter( *pPage, IM_FLAT );
/*N*/ 	SdrObject* pObject = aIter.Next();
/*N*/ 	while (pObject && !bFound)
/*N*/ 	{
/*N*/ 		if ( pObject->GetLayer()==SC_LAYER_INTERN &&
/*N*/ 				pObject->IsPolyObj() && pObject->GetPointCount()==2 )
/*N*/ 		{
/*N*/ 			BOOL bObjStartAlien =
/*N*/ 				lcl_IsOtherTab( ((const XLineStartItem&)pObject->GetItem(XATTR_LINESTART)).GetValue() );
/*N*/ 			BOOL bObjEndAlien =
/*N*/ 				lcl_IsOtherTab( ((const XLineEndItem&)pObject->GetItem(XATTR_LINEEND)).GetValue() );
/*N*/ 			
/*N*/ //-/			BOOL bObjStartAlien = FALSE;
/*N*/ //-/			BOOL bObjEndAlien = FALSE;
/*N*/ //-/			const XLineAttrSetItem* pLineAttrs =
/*N*/ //-/				((ScPublicAttrObj*)(SdrAttrObj*)pObject)->GetLineAttr();
/*N*/ //-/			if (pLineAttrs)
/*N*/ //-/			{
/*N*/ //-/				const SfxItemSet& rSet = pLineAttrs->GetItemSet();
/*N*/ //-/				bObjStartAlien = (((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).
/*N*/ //-/										GetValue().GetPointCount() == 4 );
/*N*/ //-/				bObjEndAlien   = (((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).
/*N*/ //-/										GetValue().GetPointCount() == 4 );
/*N*/ //-/			}
/*N*/ 
/*N*/ 			BOOL bStartHit = bStartAlien ? bObjStartAlien :
/*N*/ 								( !bObjStartAlien && aStartRect.IsInside(pObject->GetPoint(0)) );
/*N*/ 			BOOL bEndHit = bEndAlien ? bObjEndAlien :
/*N*/ 								( !bObjEndAlien && aEndRect.IsInside(pObject->GetPoint(1)) );
/*N*/ 
/*N*/ 			if ( bStartHit && bEndHit )
/*N*/ 				bFound = TRUE;
/*N*/ 		}
/*N*/ 		pObject = aIter.Next();
/*N*/ 	}
/*N*/ 
/*N*/ 	return bFound;
/*N*/ }


//------------------------------------------------------------------------

//	InsertXXX: called from DrawEntry/DrawAlienEntry and InsertObject

/*N*/ BOOL ScDetectiveFunc::InsertArrow( USHORT nCol, USHORT nRow,
/*N*/ 								USHORT nRefStartCol, USHORT nRefStartRow, 
/*N*/ 								USHORT nRefEndCol, USHORT nRefEndRow,
/*N*/ 								BOOL bFromOtherTab, BOOL bRed,
/*N*/ 								ScDetectiveData& rData )
/*N*/ {
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 
/*N*/ 	BOOL bArea = ( nRefStartCol != nRefEndCol || nRefStartRow != nRefEndRow );
/*N*/ 	if (bArea && !bFromOtherTab)
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 
/*N*/ 	Point aStartPos	= GetDrawPos( nRefStartCol, nRefStartRow, TRUE );
/*N*/ 	Point aEndPos   = GetDrawPos( nCol, nRow, TRUE );
/*N*/ 
/*N*/ 	if (bFromOtherTab)
/*N*/ 	{
/*N*/ 		aStartPos = Point( aEndPos.X() - 1000, aEndPos.Y() - 1000 );
/*N*/ 		if (aStartPos.X() < 0)
/*N*/ 			aStartPos.X() += 2000;
/*N*/ 		if (aStartPos.Y() < 0)
/*N*/ 			aStartPos.Y() += 2000;
/*N*/ 	}
/*N*/ 
/*N*/ 	SfxItemSet& rAttrSet = bFromOtherTab ? rData.GetFromTabSet() : rData.GetArrowSet();
/*N*/ 
/*N*/ 	if (bArea && !bFromOtherTab)
/*N*/ 		rAttrSet.Put( XLineWidthItem( 50 ) );				// Bereich
/*N*/ 	else
/*N*/ 		rAttrSet.Put( XLineWidthItem( 0 ) );				// einzelne Referenz
/*N*/ 
/*N*/ 	ColorData nColorData = ( bRed ? GetErrorColor() : GetArrowColor() );
/*N*/ 	rAttrSet.Put( XLineColorItem( String(), Color( nColorData ) ) );
/*N*/ 	Point aPointArr[2] = {aStartPos, aEndPos};
/*N*/ 	SdrPathObj* pArrow = new SdrPathObj(OBJ_LINE,
/*N*/ 				XPolyPolygon(XPolygon(Polygon(2, aPointArr))));
/*N*/ 
/*N*/ 	pArrow->NbcSetLogicRect(Rectangle(aStartPos,aEndPos));	//! noetig ???
/*N*/ 
/*N*/ //-/	pArrow->SetAttributes( rAttrSet, FALSE );
/*N*/ 	pArrow->SetItemSetAndBroadcast(rAttrSet);
/*N*/ 
/*N*/ 	ScDrawLayer::SetAnchor( pArrow, SCA_CELL );
/*N*/ 	pArrow->SetLayer( SC_LAYER_INTERN );
/*N*/ 	pPage->InsertObject( pArrow );
/*N*/ 	pModel->AddCalcUndo( new SdrUndoInsertObj( *pArrow ) );
/*N*/ 
/*N*/ 	ScDrawObjData* pData = ScDrawLayer::GetObjData( pArrow, TRUE );
/*N*/ 	if (bFromOtherTab)
/*N*/ 		pData->bValidStart = FALSE;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pData->aStt.nCol = nRefStartCol;
/*N*/ 		pData->aStt.nRow = nRefStartRow;
/*N*/ 		pData->aStt.nTab = nTab;
/*N*/ 		pData->bValidStart = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	pData->aEnd.nCol = nCol;
/*N*/ 	pData->aEnd.nRow = nRow;
/*N*/ 	pData->aEnd.nTab = nTab;
/*N*/ 	pData->bValidEnd = TRUE;
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::InsertToOtherTab( USHORT nStartCol, USHORT nStartRow,
/*N*/ 								USHORT nEndCol, USHORT nEndRow, BOOL bRed,
/*N*/ 								ScDetectiveData& rData )
/*N*/ {
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 
/*N*/ 	BOOL bArea = ( nStartCol != nEndCol || nStartRow != nEndRow );
/*N*/ 	if (bArea)
/*N*/ 	{
/*N*/ 		Point aStartCorner = GetDrawPos( nStartCol, nStartRow, FALSE );
/*N*/ 		Point aEndCorner = GetDrawPos( nEndCol+1, nEndRow+1, FALSE );
/*N*/ 
/*N*/ 		SdrRectObj* pBox = new SdrRectObj(Rectangle(aStartCorner,aEndCorner));
/*N*/ 		
/*N*/ //-/		pBox->SetAttributes( rData.GetBoxSet(), FALSE );
/*N*/ 		pBox->SetItemSetAndBroadcast(rData.GetBoxSet());
/*N*/ 
/*N*/ 		ScDrawLayer::SetAnchor( pBox, SCA_CELL );
/*N*/ 		pBox->SetLayer( SC_LAYER_INTERN );
/*N*/ 		pPage->InsertObject( pBox );
/*N*/ 		pModel->AddCalcUndo( new SdrUndoInsertObj( *pBox ) );
/*N*/ 
/*N*/ 		ScDrawObjData* pData = ScDrawLayer::GetObjData( pBox, TRUE );
/*N*/ 		pData->aStt.nCol = nStartCol;
/*N*/ 		pData->aStt.nRow = nStartRow;
/*N*/ 		pData->aStt.nTab = nTab;
/*N*/ 		pData->aEnd.nCol = nEndCol;
/*N*/ 		pData->aEnd.nRow = nEndRow;
/*N*/ 		pData->aEnd.nTab = nTab;
/*N*/ 		pData->bValidStart = TRUE;
/*N*/ 		pData->bValidEnd = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	Point aStartPos	= GetDrawPos( nStartCol, nStartRow, TRUE );
/*N*/ 	Point aEndPos   = Point( aStartPos.X() + 1000, aStartPos.Y() - 1000 );
/*N*/ 	if (aEndPos.Y() < 0)
/*N*/ 		aEndPos.Y() += 2000;
/*N*/ 
/*N*/ 	SfxItemSet& rAttrSet = rData.GetToTabSet();
/*N*/ 	if (bArea)
/*N*/ 		rAttrSet.Put( XLineWidthItem( 50 ) );				// Bereich
/*N*/ 	else
/*N*/ 		rAttrSet.Put( XLineWidthItem( 0 ) );				// einzelne Referenz
/*N*/ 
/*N*/ 	ColorData nColorData = ( bRed ? GetErrorColor() : GetArrowColor() );
/*N*/ 	rAttrSet.Put( XLineColorItem( String(), Color( nColorData ) ) );
/*N*/ 	Point aPointArr[2] = {aStartPos, aEndPos};
/*N*/ 	SdrPathObj* pArrow = new SdrPathObj(OBJ_LINE,
/*N*/ 				XPolyPolygon(XPolygon(Polygon(2, aPointArr))));
/*N*/ 
/*N*/ 	pArrow->NbcSetLogicRect(Rectangle(aStartPos,aEndPos));	//! noetig ???
/*N*/ 
/*N*/ //-/	pArrow->SetAttributes( rAttrSet, FALSE );
/*N*/ 	pArrow->SetItemSetAndBroadcast(rAttrSet);
/*N*/ 
/*N*/ 	ScDrawLayer::SetAnchor( pArrow, SCA_CELL );
/*N*/ 	pArrow->SetLayer( SC_LAYER_INTERN );
/*N*/ 	pPage->InsertObject( pArrow );
/*N*/ 	pModel->AddCalcUndo( new SdrUndoInsertObj( *pArrow ) );
/*N*/ 
/*N*/ 	ScDrawObjData* pData = ScDrawLayer::GetObjData( pArrow, TRUE );
/*N*/ 	pData->aStt.nCol = nStartCol;
/*N*/ 	pData->aStt.nRow = nStartRow;
/*N*/ 	pData->aStt.nTab = nTab;
/*N*/ 	pData->bValidStart = TRUE;
/*N*/ 	pData->bValidEnd = FALSE;
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

//------------------------------------------------------------------------

//	DrawEntry:		Formel auf dieser Tabelle,
//					Referenz auf dieser oder anderer
//	DrawAlienEntry:	Formel auf anderer Tabelle,
//					Referenz auf dieser

//		return FALSE: da war schon ein Pfeil

/*N*/ BOOL ScDetectiveFunc::DrawEntry( USHORT nCol, USHORT nRow,
/*N*/ 									const ScTripel& rRefStart, const ScTripel& rRefEnd,
/*N*/ 									ScDetectiveData& rData )
/*N*/ {
/*N*/ 	if ( HasArrow( rRefStart.GetCol(), rRefStart.GetRow(), rRefStart.GetTab(),
/*N*/ 					nCol, nRow, nTab ) )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ScTripel aErrorPos;
/*N*/ 	BOOL bError = HasError( rRefStart, rRefEnd, aErrorPos );
/*N*/ 	BOOL bAlien = ( rRefEnd.GetTab() < nTab || rRefStart.GetTab() > nTab );
/*N*/ 
/*N*/ 	return InsertArrow( nCol, nRow,
/*N*/ 						rRefStart.GetCol(), rRefStart.GetRow(),
/*N*/ 						rRefEnd.GetCol(), rRefEnd.GetRow(),
/*N*/ 						bAlien, bError, rData );
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::DrawAlienEntry( const ScTripel& rRefStart, const ScTripel& rRefEnd,
/*N*/ 										ScDetectiveData& rData )
/*N*/ {
/*N*/ 	if ( HasArrow( rRefStart.GetCol(), rRefStart.GetRow(), rRefStart.GetTab(),
/*N*/ 					0, 0, nTab+1 ) )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ScTripel aErrorPos;
/*N*/ 	BOOL bError = HasError( rRefStart, rRefEnd, aErrorPos );
/*N*/ 
/*N*/ 	return InsertToOtherTab( rRefStart.GetCol(), rRefStart.GetRow(),
/*N*/ 								rRefEnd.GetCol(), rRefEnd.GetRow(),
/*N*/ 								bError, rData );
/*N*/ }

/*N*/ void ScDetectiveFunc::DrawCircle( USHORT nCol, USHORT nRow, ScDetectiveData& rData )
/*N*/ {
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 
/*N*/ 	Point aStartPos	= GetDrawPos( nCol, nRow, FALSE );
/*N*/ 	Size aSize( (long) ( pDoc->GetColWidth(nCol, nTab) * HMM_PER_TWIPS ),
/*N*/ 				(long) ( pDoc->GetRowHeight(nRow, nTab) * HMM_PER_TWIPS ) );
/*N*/ 	Rectangle aRect( aStartPos, aSize );
/*N*/ 	aRect.Left()	-= 250;
/*N*/ 	aRect.Right()	+= 250;
/*N*/ 	aRect.Top()		-= 70;
/*N*/ 	aRect.Bottom()	+= 70;
/*N*/ 
/*N*/ 	SdrCircObj* pCircle = new SdrCircObj( OBJ_CIRC, aRect );
/*N*/ 	SfxItemSet& rAttrSet = rData.GetCircleSet();
/*N*/ 
/*N*/ //-/	pCircle->SetAttributes( rAttrSet, FALSE );
/*N*/ 	pCircle->SetItemSetAndBroadcast(rAttrSet);
/*N*/ 
/*N*/ 	ScDrawLayer::SetAnchor( pCircle, SCA_CELL );
/*N*/ 	pCircle->SetLayer( SC_LAYER_INTERN );
/*N*/ 	pPage->InsertObject( pCircle );
/*N*/ 	pModel->AddCalcUndo( new SdrUndoInsertObj( *pCircle ) );
/*N*/ 
/*N*/ 	ScDrawObjData* pData = ScDrawLayer::GetObjData( pCircle, TRUE );
/*N*/ 	pData->aStt.nCol = nCol;
/*N*/ 	pData->aStt.nRow = nRow;
/*N*/ 	pData->aStt.nTab = nTab;
/*N*/ 	pData->bValidStart = TRUE;
/*N*/ 	pData->bValidEnd = FALSE;
/*N*/ }


/*N*/ void ScDetectiveFunc::DeleteArrowsAt( USHORT nCol, USHORT nRow, BOOL bDestPnt )
/*N*/ {
/*N*/ 	Point aPos = GetDrawPos( nCol, nRow, FALSE );
/*N*/ 	Size aSize = Size(	(long) ( pDoc->GetColWidth( nCol, nTab) * HMM_PER_TWIPS ),
/*N*/ 						(long) ( pDoc->GetRowHeight( nRow, nTab) * HMM_PER_TWIPS ) );
/*N*/ 	Rectangle aRect( aPos, aSize );
/*N*/ 
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 	pPage->RecalcObjOrdNums();
/*N*/ 
/*N*/ 	long	nDelCount = 0;
/*N*/ 	ULONG	nObjCount = pPage->GetObjCount();
/*N*/ 	if (nObjCount)
/*N*/ 	{
/*N*/ 		SdrObject** ppObj = new SdrObject*[nObjCount];
/*N*/ 
/*N*/ 		SdrObjListIter aIter( *pPage, IM_FLAT );
/*N*/ 		SdrObject* pObject = aIter.Next();
/*N*/ 		while (pObject)
/*N*/ 		{
/*N*/ 			if ( pObject->GetLayer()==SC_LAYER_INTERN &&
/*N*/ 					pObject->IsPolyObj() && pObject->GetPointCount()==2 )
/*N*/ 			{
/*N*/ 				if (aRect.IsInside(pObject->GetPoint(bDestPnt)))			// Start/Zielpunkt
/*N*/ 					ppObj[nDelCount++] = pObject;
/*N*/ 			}
/*N*/ 
/*N*/ 			pObject = aIter.Next();
/*N*/ 		}
/*N*/ 
/*N*/ 		long i;
/*N*/ 		for (i=1; i<=nDelCount; i++)
/*N*/ 			pModel->AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );
/*N*/ 
/*N*/ 		for (i=1; i<=nDelCount; i++)
/*N*/ 			pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );
/*N*/ 
/*N*/ 		delete[] ppObj;
/*N*/ 	}
/*N*/ }

        //		Box um Referenz loeschen

#define SC_DET_TOLERANCE	50

/*N*/ inline BOOL RectIsPoints( const Rectangle& rRect, const Point& rStart, const Point& rEnd )
/*N*/ {
/*N*/ 	return rRect.Left()   >= rStart.X() - SC_DET_TOLERANCE
/*N*/ 		&& rRect.Left()   <= rStart.X() + SC_DET_TOLERANCE
/*N*/ 		&& rRect.Right()  >= rEnd.X()   - SC_DET_TOLERANCE
/*N*/ 		&& rRect.Right()  <= rEnd.X()   + SC_DET_TOLERANCE
/*N*/ 		&& rRect.Top()    >= rStart.Y() - SC_DET_TOLERANCE
/*N*/ 		&& rRect.Top()    <= rStart.Y() + SC_DET_TOLERANCE
/*N*/ 		&& rRect.Bottom() >= rEnd.Y()   - SC_DET_TOLERANCE
/*N*/ 		&& rRect.Bottom() <= rEnd.Y()   + SC_DET_TOLERANCE;
/*N*/ }

#undef SC_DET_TOLERANCE

/*N*/ void ScDetectiveFunc::DeleteBox( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 )
/*N*/ {
/*	String aStr;
    aStr += nCol1;
    aStr += '/';
    aStr += nRow1;
    aStr += '/';
    aStr += nCol2;
    aStr += '/';
    aStr += nRow2;
    InfoBox(0,aStr).Execute();
*/
/*N*/ 
/*N*/ 	Point aStartCorner = GetDrawPos( nCol1, nRow1, FALSE );
/*N*/ 	Point aEndCorner = GetDrawPos( nCol2+1, nRow2+1, FALSE );
/*N*/ 	Rectangle aObjRect;
/*N*/ 
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 	pPage->RecalcObjOrdNums();
/*N*/ 
/*N*/ 	long	nDelCount = 0;
/*N*/ 	ULONG	nObjCount = pPage->GetObjCount();
/*N*/ 	if (nObjCount)
/*N*/ 	{
/*N*/ 		SdrObject** ppObj = new SdrObject*[nObjCount];
/*N*/ 
/*N*/ 		SdrObjListIter aIter( *pPage, IM_FLAT );
/*N*/ 		SdrObject* pObject = aIter.Next();
/*N*/ 		while (pObject)
/*N*/ 		{
/*N*/ 			if ( pObject->GetLayer() == SC_LAYER_INTERN &&
/*N*/ 					pObject->Type() == TYPE(SdrRectObj) )
/*N*/ 			{
/*N*/ 				aObjRect = ((SdrRectObj*)pObject)->GetLogicRect();
/*N*/ 				if ( RectIsPoints( aObjRect, aStartCorner, aEndCorner ) )
/*N*/ 					ppObj[nDelCount++] = pObject;
/*N*/ 			}
/*N*/ 
/*N*/ 			pObject = aIter.Next();
/*N*/ 		}
/*N*/ 
/*N*/ 		long i;
/*N*/ 		for (i=1; i<=nDelCount; i++)
/*N*/ 			pModel->AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );
/*N*/ 
/*N*/ 		for (i=1; i<=nDelCount; i++)
/*N*/ 			pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );
/*N*/ 
/*N*/ 		delete[] ppObj;
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

/*N*/ USHORT ScDetectiveFunc::InsertPredLevelArea( const ScTripel& rRefStart, const ScTripel& rRefEnd,
/*N*/ 										ScDetectiveData& rData, USHORT nLevel )
/*N*/ {
/*N*/ 	USHORT nResult = DET_INS_EMPTY;
/*N*/ 
/*N*/ 	ScCellIterator aCellIter( pDoc, rRefStart.GetCol(), rRefStart.GetRow(), rRefStart.GetTab(),
/*N*/ 									rRefEnd.GetCol(), rRefEnd.GetRow(), rRefEnd.GetTab() );
/*N*/ 	ScBaseCell* pCell = aCellIter.GetFirst();
/*N*/ 	while (pCell)
/*N*/ 	{
/*N*/ 		if (pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 			switch( InsertPredLevel( aCellIter.GetCol(), aCellIter.GetRow(), rData, nLevel ) )
/*N*/ 			{
/*N*/ 				case DET_INS_INSERTED:
/*N*/ 					nResult = DET_INS_INSERTED;
/*N*/ 					break;
/*N*/ 				case DET_INS_CONTINUE:
/*N*/ 					if (nResult != DET_INS_INSERTED)
/*N*/ 						nResult = DET_INS_CONTINUE;
/*N*/ 					break;
/*N*/ 				case DET_INS_CIRCULAR:
/*N*/ 					if (nResult == DET_INS_EMPTY)
/*N*/ 						nResult = DET_INS_CIRCULAR;
/*N*/ 					break;
/*N*/ 			}
/*N*/ 
/*N*/ 		pCell = aCellIter.GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	return nResult;
/*N*/ }

/*N*/ USHORT ScDetectiveFunc::InsertPredLevel( USHORT nCol, USHORT nRow, ScDetectiveData& rData,
/*N*/ 											USHORT nLevel )
/*N*/ {
/*N*/ 	ScBaseCell* pCell;
/*N*/ 	pDoc->GetCell( nCol, nRow, nTab, pCell );
/*N*/ 	if (!pCell)
/*N*/ 		return DET_INS_EMPTY;
/*N*/ 	if (pCell->GetCellType() != CELLTYPE_FORMULA)
/*N*/ 		return DET_INS_EMPTY;
/*N*/ 
/*N*/ 	ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
/*N*/ 	if (pFCell->IsRunning())
/*N*/ 		return DET_INS_CIRCULAR;
/*N*/ 
/*N*/ 	if (pFCell->GetDirty())
/*N*/ 		pFCell->Interpret();				// nach SetRunning geht's nicht mehr!
/*N*/ 	pFCell->SetRunning(TRUE);
/*N*/ 
/*N*/ 	USHORT nResult = DET_INS_EMPTY;
/*N*/ 
/*N*/ 	ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
/*N*/ 	ScTripel aRefStart;
/*N*/ 	ScTripel aRefEnd;
/*N*/ 	while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
/*N*/ 	{
/*N*/ 		if (DrawEntry( nCol, nRow, aRefStart, aRefEnd, rData ))
/*N*/ 		{
/*N*/ 			nResult = DET_INS_INSERTED;			//	neuer Pfeil eingetragen
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			//	weiterverfolgen
/*N*/ 
/*N*/ 			if ( nLevel < rData.GetMaxLevel() )
/*N*/ 			{
/*N*/ 				USHORT nSubResult;
/*N*/ 				BOOL bArea = (aRefStart != aRefEnd);
/*N*/ 				if (bArea)
/*N*/ 					nSubResult = InsertPredLevelArea( aRefStart, aRefEnd, rData, nLevel+1 );
/*N*/ 				else
/*N*/ 					nSubResult = InsertPredLevel( aRefStart.GetCol(), aRefStart.GetRow(),
/*N*/ 													rData, nLevel+1 );
/*N*/ 
/*N*/ 				switch (nSubResult)
/*N*/ 				{
/*N*/ 					case DET_INS_INSERTED:
/*N*/ 						nResult = DET_INS_INSERTED;
/*N*/ 						break;
/*N*/ 					case DET_INS_CONTINUE:
/*N*/ 						if (nResult != DET_INS_INSERTED)
/*N*/ 							nResult = DET_INS_CONTINUE;
/*N*/ 						break;
/*N*/ 					case DET_INS_CIRCULAR:
/*N*/ 						if (nResult == DET_INS_EMPTY)
/*N*/ 							nResult = DET_INS_CIRCULAR;
/*N*/ 						break;
/*N*/ 					// DET_INS_EMPTY: unveraendert lassen
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else									//	nMaxLevel erreicht
/*N*/ 				if (nResult != DET_INS_INSERTED)
/*N*/ 					nResult = DET_INS_CONTINUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pFCell->SetRunning(FALSE);
/*N*/ 
/*N*/ 	return nResult;
/*N*/ }

/*N*/ USHORT ScDetectiveFunc::FindPredLevelArea( const ScTripel& rRefStart, const ScTripel& rRefEnd,
/*N*/ 												USHORT nLevel, USHORT nDeleteLevel )
/*N*/ {
/*N*/ 	USHORT nResult = nLevel;
/*N*/ 
/*N*/ 	ScCellIterator aCellIter( pDoc, rRefStart.GetCol(), rRefStart.GetRow(), rRefStart.GetTab(),
/*N*/ 									rRefEnd.GetCol(), rRefEnd.GetRow(), rRefEnd.GetTab() );
/*N*/ 	ScBaseCell* pCell = aCellIter.GetFirst();
/*N*/ 	while (pCell)
/*N*/ 	{
/*N*/ 		if (pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 		{
/*N*/ 			USHORT nTemp = FindPredLevel( aCellIter.GetCol(), aCellIter.GetRow(), nLevel, nDeleteLevel );
/*N*/ 			if (nTemp > nResult)
/*N*/ 				nResult = nTemp;
/*N*/ 		}
/*N*/ 		pCell = aCellIter.GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	return nResult;
/*N*/ }

                                            //	nDeleteLevel != 0	-> loeschen

/*N*/ USHORT ScDetectiveFunc::FindPredLevel( USHORT nCol, USHORT nRow, USHORT nLevel, USHORT nDeleteLevel )
/*N*/ {
/*N*/ 	DBG_ASSERT( nLevel<1000, "Level" );
/*N*/ 
/*N*/ 	ScBaseCell* pCell;
/*N*/ 	pDoc->GetCell( nCol, nRow, nTab, pCell );
/*N*/ 	if (!pCell)
/*N*/ 		return nLevel;
/*N*/ 	if (pCell->GetCellType() != CELLTYPE_FORMULA)
/*N*/ 		return nLevel;
/*N*/ 
/*N*/ 	ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
/*N*/ 	if (pFCell->IsRunning())
/*N*/ 		return nLevel;
/*N*/ 
/*N*/ 	if (pFCell->GetDirty())
/*N*/ 		pFCell->Interpret();				// nach SetRunning geht's nicht mehr!
/*N*/ 	pFCell->SetRunning(TRUE);
/*N*/ 
/*N*/ 	USHORT nResult = nLevel;
/*N*/ 	BOOL bDelete = ( nDeleteLevel && nLevel == nDeleteLevel-1 );
/*N*/ 
/*N*/ 	if ( bDelete )
/*N*/ 	{
/*N*/ 		DeleteArrowsAt( nCol, nRow, TRUE );					// Pfeile, die hierher zeigen
/*N*/ 	}
/*N*/ 
/*N*/ 	ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
/*N*/ 	ScTripel aRefStart;
/*N*/ 	ScTripel aRefEnd;
/*N*/ 	while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
/*N*/ 	{
/*N*/ 		BOOL bArea = ( aRefStart != aRefEnd );
/*N*/ 
/*N*/ 		if ( bDelete )					// Rahmen loeschen ?
/*N*/ 		{
/*N*/ 			if (bArea)
/*N*/ 			{
/*N*/ 				DeleteBox( aRefStart.GetCol(), aRefStart.GetRow(), aRefEnd.GetCol(), aRefEnd.GetRow() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else							// weitersuchen
/*N*/ 		{
/*N*/ 			if ( HasArrow( aRefStart.GetCol(),aRefStart.GetRow(),aRefStart.GetTab(),
/*N*/ 							nCol,nRow,nTab ) )
/*N*/ 			{
/*N*/ 				USHORT nTemp;
/*N*/ 				if (bArea)
/*N*/ 					nTemp = FindPredLevelArea( aRefStart, aRefEnd, nLevel+1, nDeleteLevel );
/*N*/ 				else
/*N*/ 					nTemp = FindPredLevel( aRefStart.GetCol(),aRefStart.GetRow(),
/*N*/ 														nLevel+1, nDeleteLevel );
/*N*/ 				if (nTemp > nResult)
/*N*/ 					nResult = nTemp;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pFCell->SetRunning(FALSE);
/*N*/ 
/*N*/ 	return nResult;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ USHORT ScDetectiveFunc::InsertErrorLevel( USHORT nCol, USHORT nRow, ScDetectiveData& rData,
/*N*/ 											USHORT nLevel )
/*N*/ {
/*N*/ 	ScBaseCell* pCell;
/*N*/ 	pDoc->GetCell( nCol, nRow, nTab, pCell );
/*N*/ 	if (!pCell)
/*N*/ 		return DET_INS_EMPTY;
/*N*/ 	if (pCell->GetCellType() != CELLTYPE_FORMULA)
/*N*/ 		return DET_INS_EMPTY;
/*N*/ 
/*N*/ 	ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
/*N*/ 	if (pFCell->IsRunning())
/*N*/ 		return DET_INS_CIRCULAR;
/*N*/ 
/*N*/ 	if (pFCell->GetDirty())
/*N*/ 		pFCell->Interpret();				// nach SetRunning geht's nicht mehr!
/*N*/ 	pFCell->SetRunning(TRUE);
/*N*/ 
/*N*/ 	USHORT nResult = DET_INS_EMPTY;
/*N*/ 
/*N*/ 	ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
/*N*/ 	ScTripel aRefStart;
/*N*/ 	ScTripel aRefEnd;
/*N*/ 	ScTripel aErrorPos;
/*N*/ 	BOOL bHasError = FALSE;
/*N*/ 	while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
/*N*/ 	{
/*N*/ 		if (HasError( aRefStart, aRefEnd, aErrorPos ))
/*N*/ 		{
/*N*/ 			bHasError = TRUE;
/*N*/ 			if (DrawEntry( nCol, nRow, aErrorPos, aErrorPos, rData ))
/*N*/ 				nResult = DET_INS_INSERTED;
/*N*/ 
/*N*/ 			//	und weiterverfolgen
/*N*/ 
/*N*/ 			if ( nLevel < rData.GetMaxLevel() )			// praktisch immer
/*N*/ 			{
/*N*/ 				if (InsertErrorLevel( aErrorPos.GetCol(), aErrorPos.GetRow(),
/*N*/ 														rData, nLevel+1 ) == DET_INS_INSERTED)
/*N*/ 					nResult = DET_INS_INSERTED;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pFCell->SetRunning(FALSE);
/*N*/ 
/*N*/ 													// Blaetter ?
/*N*/ 	if (!bHasError)
/*N*/ 		if (InsertPredLevel( nCol, nRow, rData, rData.GetMaxLevel() ) == DET_INS_INSERTED)
/*N*/ 			nResult = DET_INS_INSERTED;
/*N*/ 
/*N*/ 	return nResult;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ USHORT ScDetectiveFunc::InsertSuccLevel( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
/*N*/ 										ScDetectiveData& rData, USHORT nLevel )
/*N*/ {
/*N*/ 	//	ueber ganzes Dokument
/*N*/ 
/*N*/ 	USHORT nResult = DET_INS_EMPTY;
/*N*/ //	ScCellIterator aCellIter( pDoc, 0,0, nTab, MAXCOL,MAXROW, nTab );
/*N*/ 	ScCellIterator aCellIter( pDoc, 0,0,0, MAXCOL,MAXROW,MAXTAB );			// alle Tabellen
/*N*/ 	ScBaseCell* pCell = aCellIter.GetFirst();
/*N*/ 	while (pCell)
/*N*/ 	{
/*N*/ 		if (pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 		{
/*N*/ 			ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
/*N*/ 			BOOL bRunning = pFCell->IsRunning();
/*N*/ 
/*N*/ 			if (pFCell->GetDirty())
/*N*/ 				pFCell->Interpret();				// nach SetRunning geht's nicht mehr!
/*N*/ 			pFCell->SetRunning(TRUE);
/*N*/ 
/*N*/ 			ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
/*N*/ 			ScTripel aRefStart;
/*N*/ 			ScTripel aRefEnd;
/*N*/ 			while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
/*N*/ 			{
/*N*/ 				if (aRefStart.GetTab() <= nTab && aRefEnd.GetTab() >= nTab)
/*N*/ 				{
/*N*/ 					if (Intersect( nCol1,nRow1,nCol2,nRow2,
/*N*/ 							aRefStart.GetCol(),aRefStart.GetRow(),
/*N*/ 							aRefEnd.GetCol(),aRefEnd.GetRow() ))
/*N*/ 					{
/*N*/ 						BOOL bAlien = ( aCellIter.GetTab() != nTab );
/*N*/ 						BOOL bDrawRet;
/*N*/ 						if (bAlien)
/*N*/ 							bDrawRet = DrawAlienEntry( aRefStart, aRefEnd, rData );
/*N*/ 						else
/*N*/ 							bDrawRet = DrawEntry( aCellIter.GetCol(), aCellIter.GetRow(),
/*N*/ 													aRefStart, aRefEnd, rData );
/*N*/ 						if (bDrawRet)
/*N*/ 						{
/*N*/ 							nResult = DET_INS_INSERTED;			//	neuer Pfeil eingetragen
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							if (bRunning)
/*N*/ 							{
/*N*/ 								if (nResult == DET_INS_EMPTY)
/*N*/ 									nResult = DET_INS_CIRCULAR;
/*N*/ 							}
/*N*/ 							else
/*N*/ 							{
/*N*/ 										//	weiterverfolgen
/*N*/ 
/*N*/ 								if ( nLevel < rData.GetMaxLevel() )
/*N*/ 								{
/*N*/ 									USHORT nSubResult = InsertSuccLevel(
/*N*/ 															aCellIter.GetCol(), aCellIter.GetRow(),
/*N*/ 															aCellIter.GetCol(), aCellIter.GetRow(),
/*N*/ 															rData, nLevel+1 );
/*N*/ 									switch (nSubResult)
/*N*/ 									{
/*N*/ 										case DET_INS_INSERTED:
/*N*/ 											nResult = DET_INS_INSERTED;
/*N*/ 											break;
/*N*/ 										case DET_INS_CONTINUE:
/*N*/ 											if (nResult != DET_INS_INSERTED)
/*N*/ 												nResult = DET_INS_CONTINUE;
/*N*/ 											break;
/*N*/ 										case DET_INS_CIRCULAR:
/*N*/ 											if (nResult == DET_INS_EMPTY)
/*N*/ 												nResult = DET_INS_CIRCULAR;
/*N*/ 											break;
/*N*/ 										// DET_INS_EMPTY: unveraendert lassen
/*N*/ 									}
/*N*/ 								}
/*N*/ 								else									//	nMaxLevel erreicht
/*N*/ 									if (nResult != DET_INS_INSERTED)
/*N*/ 										nResult = DET_INS_CONTINUE;
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			pFCell->SetRunning(bRunning);
/*N*/ 		}
/*N*/ 		pCell = aCellIter.GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	return nResult;
/*N*/ }

/*N*/ USHORT ScDetectiveFunc::FindSuccLevel( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
/*N*/ 										USHORT nLevel, USHORT nDeleteLevel )
/*N*/ {
/*N*/ 	DBG_ASSERT( nLevel<1000, "Level" );
/*N*/ 
/*N*/ 	USHORT nResult = nLevel;
/*N*/ 	BOOL bDelete = ( nDeleteLevel && nLevel == nDeleteLevel-1 );
/*N*/ 
/*N*/ 	ScCellIterator aCellIter( pDoc, 0,0, nTab, MAXCOL,MAXROW, nTab );
/*N*/ 	ScBaseCell* pCell = aCellIter.GetFirst();
/*N*/ 	while (pCell)
/*N*/ 	{
/*N*/ 		if (pCell->GetCellType() == CELLTYPE_FORMULA)
/*N*/ 		{
/*N*/ 			ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
/*N*/ 			BOOL bRunning = pFCell->IsRunning();
/*N*/ 
/*N*/ 			if (pFCell->GetDirty())
/*N*/ 				pFCell->Interpret();				// nach SetRunning geht's nicht mehr!
/*N*/ 			pFCell->SetRunning(TRUE);
/*N*/ 
/*N*/ 			ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
/*N*/ 			ScTripel aRefStart;
/*N*/ 			ScTripel aRefEnd;
/*N*/ 			while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
/*N*/ 			{
/*N*/ 				if (aRefStart.GetTab() <= nTab && aRefEnd.GetTab() >= nTab)
/*N*/ 				{
/*N*/ 					if (Intersect( nCol1,nRow1,nCol2,nRow2,
/*N*/ 							aRefStart.GetCol(),aRefStart.GetRow(),
/*N*/ 							aRefEnd.GetCol(),aRefEnd.GetRow() ))
/*N*/ 					{
/*N*/ 						if ( bDelete )							// Pfeile, die hier anfangen
/*N*/ 						{
/*N*/ 							if (aRefStart != aRefEnd)
/*N*/ 							{
/*N*/ 								DeleteBox( aRefStart.GetCol(), aRefStart.GetRow(),
/*N*/ 												aRefEnd.GetCol(), aRefEnd.GetRow() );
/*N*/ 							}
/*N*/ 							DeleteArrowsAt( aRefStart.GetCol(), aRefStart.GetRow(), FALSE );
/*N*/ 						}
/*N*/ 						else if ( !bRunning &&
/*N*/ 								HasArrow( aRefStart.GetCol(),aRefStart.GetRow(),aRefStart.GetTab(),
/*N*/ 											aCellIter.GetCol(),aCellIter.GetRow(),aCellIter.GetTab() ) )
/*N*/ 						{
/*N*/ 							USHORT nTemp = FindSuccLevel( aCellIter.GetCol(), aCellIter.GetRow(),
/*N*/ 															aCellIter.GetCol(), aCellIter.GetRow(),
/*N*/ 															nLevel+1, nDeleteLevel );
/*N*/ 							if (nTemp > nResult)
/*N*/ 								nResult = nTemp;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			pFCell->SetRunning(bRunning);
/*N*/ 		}
/*N*/ 		pCell = aCellIter.GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	return nResult;
/*N*/ }


//
//	--------------------------------------------------------------------------------
//

/*N*/ BOOL ScDetectiveFunc::ShowPred( USHORT nCol, USHORT nRow )
/*N*/ {
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 	ScDetectiveData aData( pModel );
/*N*/ 
/*N*/ 	USHORT nMaxLevel = 0;
/*N*/ 	USHORT nResult = DET_INS_CONTINUE;
/*N*/ 	while (nResult == DET_INS_CONTINUE && nMaxLevel < 1000)
/*N*/ 	{
/*N*/ 		aData.SetMaxLevel( nMaxLevel );
/*N*/ 		nResult = InsertPredLevel( nCol, nRow, aData, 0 );
/*N*/ 		++nMaxLevel;
/*N*/ 	}
/*N*/ 
/*N*/ 	return ( nResult == DET_INS_INSERTED );
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::ShowSucc( USHORT nCol, USHORT nRow )
/*N*/ {
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 	ScDetectiveData aData( pModel );
/*N*/ 
/*N*/ 	USHORT nMaxLevel = 0;
/*N*/ 	USHORT nResult = DET_INS_CONTINUE;
/*N*/ 	while (nResult == DET_INS_CONTINUE && nMaxLevel < 1000)
/*N*/ 	{
/*N*/ 		aData.SetMaxLevel( nMaxLevel );
/*N*/ 		nResult = InsertSuccLevel( nCol, nRow, nCol, nRow, aData, 0 );
/*N*/ 		++nMaxLevel;
/*N*/ 	}
/*N*/ 
/*N*/ 	return ( nResult == DET_INS_INSERTED );
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::ShowError( USHORT nCol, USHORT nRow )
/*N*/ {
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 	ScTripel aPos( nCol, nRow, nTab );
/*N*/ 	ScTripel aErrPos;
/*N*/ 	if ( !HasError( aPos,aPos,aErrPos ) )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	ScDetectiveData aData( pModel );
/*N*/ 
/*N*/ 	aData.SetMaxLevel( 1000 );
/*N*/ 	USHORT nResult = InsertErrorLevel( nCol, nRow, aData, 0 );
/*N*/ 
/*N*/ 	return ( nResult == DET_INS_INSERTED );
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::DeleteSucc( USHORT nCol, USHORT nRow )
/*N*/ {
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 	USHORT nLevelCount = FindSuccLevel( nCol, nRow, nCol, nRow, 0, 0 );
/*N*/ 	if ( nLevelCount )
/*N*/ 		FindSuccLevel( nCol, nRow, nCol, nRow, 0, nLevelCount );			// loeschen
/*N*/ 
/*N*/ 	return ( nLevelCount != 0 );
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::DeletePred( USHORT nCol, USHORT nRow )
/*N*/ {
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 	USHORT nLevelCount = FindPredLevel( nCol, nRow, 0, 0 );
/*N*/ 	if ( nLevelCount )
/*N*/ 		FindPredLevel( nCol, nRow, 0, nLevelCount );			// loeschen
/*N*/ 
/*N*/ 	return ( nLevelCount != 0 );
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::DeleteAll( ScDetectiveDelete eWhat )
/*N*/ {
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 	pPage->RecalcObjOrdNums();
/*N*/ 
/*N*/ 	long	nDelCount = 0;
/*N*/ 	ULONG	nObjCount = pPage->GetObjCount();
/*N*/ 	if (nObjCount)
/*N*/ 	{
/*N*/ 		SdrObject** ppObj = new SdrObject*[nObjCount];
/*N*/ 
/*N*/ 		SdrObjListIter aIter( *pPage, IM_FLAT );
/*N*/ 		SdrObject* pObject = aIter.Next();
/*N*/ 		while (pObject)
/*N*/ 		{
/*N*/ 			if ( pObject->GetLayer() == SC_LAYER_INTERN )
/*N*/ 			{
/*N*/ 				BOOL bDoThis = TRUE;
/*N*/ 				if ( eWhat != SC_DET_ALL )
/*N*/ 				{
/*N*/ 					BOOL bCircle = ( pObject->ISA(SdrCircObj) );
/*N*/ 					BOOL bCaption = ( pObject->ISA(SdrCaptionObj) );
/*N*/ 					if ( eWhat == SC_DET_DETECTIVE )		// Detektiv, aus Menue
/*N*/ 						bDoThis = !bCaption;				// auch Kreise
/*N*/ 					else if ( eWhat == SC_DET_CIRCLES )		// Kreise, wenn neue erzeugt werden
/*N*/ 						bDoThis = bCircle;
/*N*/ 					else if ( eWhat == SC_DET_COMMENTS )
/*N*/ 						bDoThis = bCaption;
/*N*/ 					else if ( eWhat == SC_DET_ARROWS )		// DetectiveRefresh
/*N*/ 						bDoThis = !bCaption && !bCircle;	// don't include circles
/*N*/ 					else
/*N*/ 						DBG_ERROR("wat?");
/*N*/ 				}
/*N*/ 				if ( bDoThis )
/*N*/ 					ppObj[nDelCount++] = pObject;
/*N*/ 			}
/*N*/ 
/*N*/ 			pObject = aIter.Next();
/*N*/ 		}
/*N*/ 
/*N*/ 		long i;
/*N*/ 		for (i=1; i<=nDelCount; i++)
/*N*/ 			pModel->AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );
/*N*/ 
/*N*/ 		for (i=1; i<=nDelCount; i++)
/*N*/ 			pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );
/*N*/ 
/*N*/ 		delete[] ppObj;
/*N*/ 	}
/*N*/ 
/*N*/ 	return ( nDelCount != 0 );
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::MarkInvalid(BOOL& rOverflow)
/*N*/ {
/*N*/ 	rOverflow = FALSE;
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	BOOL bDeleted = DeleteAll( SC_DET_CIRCLES );		// nur die Kreise
/*N*/ 
/*N*/ 	ScDetectiveData aData( pModel );
/*N*/ 	long nInsCount = 0;
/*N*/ 
/*N*/ 	//	Stellen suchen, wo Gueltigkeit definiert ist
/*N*/ 
/*N*/ 	ScDocAttrIterator aAttrIter( pDoc, nTab, 0,0,MAXCOL,MAXROW );
/*N*/ 	USHORT nCol, nRow1, nRow2;
/*N*/ 	const ScPatternAttr* pPattern = aAttrIter.GetNext( nCol, nRow1, nRow2 );
/*N*/ 	while ( pPattern && nInsCount < SC_DET_MAXCIRCLE )
/*N*/ 	{
/*N*/ 		ULONG nIndex = ((const SfxUInt32Item&)pPattern->GetItem(ATTR_VALIDDATA)).GetValue();
/*N*/ 		if (nIndex)
/*N*/ 		{
/*N*/ 			const ScValidationData*	pData = pDoc->GetValidationEntry( nIndex );
/*N*/ 			if ( pData )
/*N*/ 			{
/*N*/ 				//	Zellen in dem Bereich durchgehen
/*N*/ 
/*N*/ 				BOOL bMarkEmpty = !pData->IsIgnoreBlank();
/*N*/ 				USHORT nNextRow = nRow1;
/*N*/ 				USHORT nRow;
/*N*/ 				ScCellIterator aCellIter( pDoc, nCol,nRow1,nTab, nCol,nRow2,nTab );
/*N*/ 				ScBaseCell* pCell = aCellIter.GetFirst();
/*N*/ 				while ( pCell && nInsCount < SC_DET_MAXCIRCLE )
/*N*/ 				{
/*N*/ 					USHORT nCellRow = aCellIter.GetRow();
/*N*/ 					if ( bMarkEmpty )
/*N*/ 						for ( nRow = nNextRow; nRow < nCellRow && nInsCount < SC_DET_MAXCIRCLE; nRow++ )
/*N*/ 						{
/*N*/ 							DrawCircle( nCol, nRow, aData );
/*N*/ 							++nInsCount;
/*N*/ 						}
/*N*/ 					if ( !pData->IsDataValid( pCell, ScAddress( nCol, nCellRow, nTab ) ) )
/*N*/ 					{
/*N*/ 						DrawCircle( nCol, nCellRow, aData );
/*N*/ 						++nInsCount;
/*N*/ 					}
/*N*/ 					nNextRow = nCellRow + 1;
/*N*/ 					pCell = aCellIter.GetNext();
/*N*/ 				}
/*N*/ 				if ( bMarkEmpty )
/*N*/ 					for ( nRow = nNextRow; nRow <= nRow2 && nInsCount < SC_DET_MAXCIRCLE; nRow++ )
/*N*/ 					{
/*N*/ 						DrawCircle( nCol, nRow, aData );
/*N*/ 						++nInsCount;
/*N*/ 					}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		pPattern = aAttrIter.GetNext( nCol, nRow1, nRow2 );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( nInsCount >= SC_DET_MAXCIRCLE )
/*N*/ 		rOverflow = TRUE;
/*N*/ 
/*N*/ 	return ( bDeleted || nInsCount != 0 );
/*N*/ }

/*N*/ SdrObject* ScDetectiveFunc::ShowCommentUser( USHORT nCol, USHORT nRow, const String& rUserText,
/*N*/ 											const Rectangle& rVisible, BOOL bLeft, BOOL bForce,
/*N*/ 											SdrPage* pDestPage )
/*N*/ {
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel && !pDestPage)
/*N*/ 		return NULL;
/*N*/ 
/*N*/ 	SdrObject* pObject = NULL;
/*N*/ 	ScPostIt aNote;
/*N*/ 	BOOL bFound = pDoc->GetNote( nCol, nRow, nTab, aNote );
/*N*/ 	if ( bFound || bForce || rUserText.Len() )
/*N*/ 	{
        DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SdrModel* pDestModel = pModel;
/*N*/ 	}
/*N*/ 
/*N*/ 	return pObject;
/*N*/ }

/*N*/ SdrObject* ScDetectiveFunc::ShowComment( USHORT nCol, USHORT nRow, BOOL bForce, SdrPage* pDestPage )
/*N*/ {
/*N*/ 	return ShowCommentUser( nCol, nRow, String(), Rectangle(0,0,0,0), FALSE, bForce, pDestPage );
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::HideComment( USHORT nCol, USHORT nRow )
/*N*/ {
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return FALSE;
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 
/*N*/ 	pPage->RecalcObjOrdNums();
/*N*/ 	BOOL bDone = FALSE;
/*N*/ 
/*N*/ 	SdrObjListIter aIter( *pPage, IM_FLAT );
/*N*/ 	SdrObject* pObject = aIter.Next();
/*N*/ 	while (pObject && !bDone)
/*N*/ 	{
/*N*/ 		if ( pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA( SdrCaptionObj ) )
/*N*/ 		{
/*N*/ 			ScDrawObjData* pData = ScDrawLayer::GetObjData( pObject );
/*N*/ 			if ( pData && nCol == pData->aStt.nCol && nRow == pData->aStt.nRow )
/*N*/ 			{
/*N*/ 				pModel->AddCalcUndo( new SdrUndoRemoveObj( *pObject ) );
/*N*/ 				pPage->RemoveObject( pObject->GetOrdNum() );
/*N*/ 				bDone = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		pObject = aIter.Next();
/*N*/ 	}
/*N*/ 
/*N*/ 	return bDone;
/*N*/ }

/*N*/ void ScDetectiveFunc::UpdateAllComments()
/*N*/ {
/*N*/ 	//	for all caption objects, update attributes and SpecialTextBoxShadow flag
/*N*/ 	//	(on all tables - nTab is ignored!)
/*N*/ 
/*N*/ 	//	no undo actions, this is refreshed after undo
/*N*/ 
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return;
/*N*/ 
/*N*/ 	ScCommentData aData( pDoc, pModel );
/*N*/ 
/*N*/ 	USHORT nTabCount = pDoc->GetTableCount();
/*N*/ 	for (USHORT nObjTab=0; nObjTab<nTabCount; nObjTab++)
/*N*/ 	{
/*N*/ 		SdrPage* pPage = pModel->GetPage(nObjTab);
/*N*/ 		DBG_ASSERT(pPage,"Page ?");
/*N*/ 		if (pPage)
/*N*/ 		{
/*N*/ 			SdrObjListIter aIter( *pPage, IM_FLAT );
/*N*/ 			SdrObject* pObject = aIter.Next();
/*N*/ 			while (pObject)
/*N*/ 			{
/*?*/ 				if ( pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA( SdrCaptionObj ) )
/*?*/ 				{
/*?*/ 					SdrCaptionObj* pCaption = (SdrCaptionObj*)pObject;
/*?*/ 
/*?*/ 					SfxItemSet& rAttrSet = aData.GetCaptionSet();
/*?*/ 
/*?*/ //-/					pCaption->SetAttributes( rAttrSet, FALSE );
/*?*/ 					pCaption->SetItemSetAndBroadcast(rAttrSet);
/*?*/ 
/*?*/ 					pCaption->SetSpecialTextBoxShadow();
/*?*/ 				}
/*?*/ 
/*?*/ 				pObject = aIter.Next();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScDetectiveFunc::UpdateAllArrowColors() // detective.sdc
/*N*/ {
/*N*/ 	//	no undo actions necessary
/*N*/ 
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel)
/*N*/ 		return;
/*N*/ 
/*N*/ 	USHORT nTabCount = pDoc->GetTableCount();
/*N*/ 	for (USHORT nObjTab=0; nObjTab<nTabCount; nObjTab++)
/*N*/ 	{
/*N*/ 		SdrPage* pPage = pModel->GetPage(nObjTab);
/*N*/ 		DBG_ASSERT(pPage,"Page ?");
/*N*/ 		if (pPage)
/*N*/ 		{
/*N*/ 			SdrObjListIter aIter( *pPage, IM_FLAT );
/*N*/ 			SdrObject* pObject = aIter.Next();
/*N*/ 			while (pObject)
/*N*/ 			{
/*N*/ 				if ( pObject->GetLayer() == SC_LAYER_INTERN )
/*N*/ 				{
/*N*/ 					BOOL bArrow = FALSE;
/*N*/ 					BOOL bError = FALSE;
/*N*/ 
/*N*/ 					ScAddress aPos;
/*N*/ 					ScRange aSource;
/*N*/ 					BOOL bDummy;
/*N*/ 					ScDetectiveObjType eType = GetDetectiveObjectType( pObject, aPos, aSource, bDummy );
/*N*/ 					if ( eType == SC_DETOBJ_ARROW || eType == SC_DETOBJ_TOOTHERTAB )
/*N*/ 					{
/*N*/ 						//	source is valid, determine error flag from source range
/*N*/ 
/*N*/ 						ScTripel aStart( aSource.aStart.Col(), aSource.aStart.Row(), aSource.aStart.Tab() );
/*N*/ 						ScTripel aEnd( aSource.aEnd.Col(), aSource.aEnd.Row(), aSource.aEnd.Tab() );
/*N*/ 						ScTripel aErrPos;
/*N*/ 						if ( HasError( aStart, aEnd, aErrPos ) )
/*N*/ 							bError = TRUE;
/*N*/ 						else
/*N*/ 							bArrow = TRUE;
/*N*/ 					}
/*N*/ 					else if ( eType == SC_DETOBJ_FROMOTHERTAB )
/*N*/ 					{
/*N*/ 						//	source range is no longer known, take error flag from formula itself
/*N*/ 						//	(this means, if the formula has an error, all references to other tables
/*N*/ 						//	are marked red)
/*N*/ 
/*N*/ 						ScTripel aFormulaPos( aPos.Col(), aPos.Row(), aPos.Tab() );
/*N*/ 						ScTripel aErrPos;
/*N*/ 						if ( HasError( aFormulaPos, aFormulaPos, aErrPos ) )
/*N*/ 							bError = TRUE;
/*N*/ 						else
/*N*/ 							bArrow = TRUE;
/*N*/ 					}
/*N*/ 					else if ( eType == SC_DETOBJ_CIRCLE )
/*N*/ 					{
/*N*/ 						//	circles (error marks) are always red
/*N*/ 
/*N*/ 						bError = TRUE;
/*N*/ 					}
/*N*/ 					else if ( eType == SC_DETOBJ_NONE )
/*N*/ 					{
/*N*/ 						//	frame for area reference has no ObjType, always gets arrow color
/*N*/ 
/*N*/ 						if ( pObject->ISA( SdrRectObj ) && !pObject->ISA( SdrCaptionObj ) )
/*N*/ 						{
/*N*/ 							bArrow = TRUE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 
/*N*/ 					if ( bArrow || bError )
/*N*/ 					{
/*N*/ 						ColorData nColorData = ( bError ? GetErrorColor() : GetArrowColor() );
/*N*/ 						pObject->SetItem( XLineColorItem( String(), Color( nColorData ) ) );
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				pObject = aIter.Next();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScDetectiveFunc::FindFrameForObject( SdrObject* pObject, ScRange& rRange ) // detective.sdc
/*N*/ {
/*N*/ 	//	find the rectangle for an arrow (always the object directly before the arrow)
/*N*/ 	//	rRange must be initialized to the source cell of the arrow (start of area)
/*N*/ 
/*N*/ 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ 	if (!pModel) return FALSE;
/*N*/ 
/*N*/ 	SdrPage* pPage = pModel->GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page ?");
/*N*/ 	if (!pPage) return FALSE;
/*N*/ 
/*N*/ 	ULONG nPos = pPage->GetContainer().GetPos( pObject );
/*N*/ 	if ( nPos != CONTAINER_ENTRY_NOTFOUND && nPos > 0 )
/*N*/ 	{
/*N*/ 		SdrObject* pPrevObj = pPage->GetObj( nPos - 1 );
/*N*/ 		if ( pPrevObj && pPrevObj->GetLayer() == SC_LAYER_INTERN && pPrevObj->ISA(SdrRectObj) )
/*N*/ 		{
/*N*/ 			ScDrawObjData* pPrevData = ScDrawLayer::GetObjData( pPrevObj );
/*N*/ 			if ( pPrevData && pPrevData->bValidStart && pPrevData->bValidEnd )
/*N*/ 			{
/*N*/ 				if ( pPrevData->aStt.nCol == rRange.aStart.Col() &&
/*N*/ 					 pPrevData->aStt.nRow == rRange.aStart.Row() &&
/*N*/ 					 pPrevData->aStt.nTab == rRange.aStart.Tab() )
/*N*/ 				{
/*N*/ 					rRange.aEnd.Set( pPrevData->aEnd.nCol,
/*N*/ 									 pPrevData->aEnd.nRow,
/*N*/ 									 pPrevData->aEnd.nTab );
/*N*/ 					return TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ ScDetectiveObjType ScDetectiveFunc::GetDetectiveObjectType( SdrObject* pObject,
/*N*/ 								ScAddress& rPosition, ScRange& rSource, BOOL& rRedLine )
/*N*/ {
/*N*/ 	rRedLine = FALSE; // detective.sdc
/*N*/ 	ScDetectiveObjType eType = SC_DETOBJ_NONE;
/*N*/ 
/*N*/ 	if ( pObject && pObject->GetLayer() == SC_LAYER_INTERN )
/*N*/ 	{
/*N*/ 		ScDrawObjData* pData = ScDrawLayer::GetObjData( pObject );
/*N*/ 		if ( pObject->IsPolyObj() && pObject->GetPointCount() == 2 )
/*N*/ 		{
/*N*/ 			// line object -> arrow
/*N*/ 
/*N*/ 			if ( pData->bValidStart )
/*N*/ 				eType = ( pData->bValidEnd ) ? SC_DETOBJ_ARROW : SC_DETOBJ_TOOTHERTAB;
/*N*/ 			else if ( pData->bValidEnd )
/*N*/ 				eType = SC_DETOBJ_FROMOTHERTAB;
/*N*/ 
/*N*/ 			if ( pData->bValidStart )
/*N*/ 				rSource = ScRange( pData->aStt.nCol, pData->aStt.nRow, pData->aStt.nTab );
/*N*/ 			if ( pData->bValidEnd )
/*N*/ 				rPosition = ScAddress( pData->aEnd.nCol, pData->aEnd.nRow, pData->aEnd.nTab );
/*N*/ 
/*N*/ 			if ( pData->bValidStart && lcl_HasThickLine( *pObject ) )
/*N*/ 			{
/*N*/ 				// thick line -> look for frame before this object
/*N*/ 
/*N*/ 				FindFrameForObject( pObject, rSource );		// modifies rSource
/*N*/ 			}
/*N*/ 
/*N*/ 			ColorData nObjColor = ((const XLineColorItem&)pObject->GetItem(XATTR_LINECOLOR)).GetValue().GetColor();
/*N*/ 			if ( nObjColor == GetErrorColor() && nObjColor != GetArrowColor() )
/*N*/ 				rRedLine = TRUE;
/*N*/ 		}
/*N*/ 		else if ( pObject->ISA(SdrCircObj) )
/*N*/ 		{
/*N*/ 			if ( pData->bValidStart )
/*N*/ 			{
/*N*/ 				// cell position is returned in rPosition
/*N*/ 
/*N*/ 				rPosition = ScAddress( pData->aStt.nCol, pData->aStt.nRow, pData->aStt.nTab );
/*N*/ 				eType = SC_DETOBJ_CIRCLE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return eType;
/*N*/ }

/*N*/ void ScDetectiveFunc::InsertObject( ScDetectiveObjType eType,
/*N*/ 							const ScAddress& rPosition, const ScRange& rSource,
/*N*/ 							BOOL bRedLine )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	ScDrawLayer* pModel = pDoc->GetDrawLayer();
/*N*/ }

// static
/*N*/ ColorData ScDetectiveFunc::GetArrowColor()
/*N*/ {
/*N*/ 	if (!bColorsInitialized)
/*N*/ 		InitializeColors();
/*N*/ 	return nArrowColor;
/*N*/ }

// static
/*N*/ ColorData ScDetectiveFunc::GetErrorColor()
/*N*/ {
/*N*/ 	if (!bColorsInitialized)
/*N*/ 		InitializeColors();
/*N*/ 	return nErrorColor;
/*N*/ }

// static
/*N*/ ColorData ScDetectiveFunc::GetCommentColor()
/*N*/ {
/*N*/ 	if (!bColorsInitialized)
/*N*/ 		InitializeColors();
/*N*/ 	return nCommentColor;
/*N*/ }

// static
/*N*/ void ScDetectiveFunc::InitializeColors()
/*N*/ {
/*N*/ 	// may be called several times to update colors from configuration
/*N*/ 
/*N*/     const ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
/*N*/     nArrowColor   = rColorCfg.GetColorValue(CALCDETECTIVE).nColor;
/*N*/     nErrorColor   = rColorCfg.GetColorValue(CALCDETECTIVEERROR).nColor;
/*N*/     nCommentColor = rColorCfg.GetColorValue(CALCNOTESBACKGROUND).nColor;
/*N*/ 
/*N*/ 	bColorsInitialized = TRUE;
/*N*/ }

// static

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
