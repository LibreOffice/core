/*************************************************************************
 *
 *  $RCSfile: detfunc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 11:19:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#include <svx/sdshitm.hxx>
#include <svx/sdsxyitm.hxx>
#include <svx/sdtditm.hxx>
#include <svx/svditer.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnedcit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xtable.hxx>

#include "detfunc.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "validat.hxx"
#include "cell.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "attrib.hxx"

//------------------------------------------------------------------------

// #68927# created line ends must have a name
#define SC_LINEEND_NAME     '*'

//------------------------------------------------------------------------

enum DetInsertResult {              // Return-Werte beim Einfuegen in einen Level
            DET_INS_CONTINUE,
            DET_INS_INSERTED,
            DET_INS_EMPTY,
            DET_INS_CIRCULAR };

//  maximale Textlaenge (Zeichen), die noch in "kleines" Objekt passt
#define SC_NOTE_SMALLTEXT   100

//------------------------------------------------------------------------

//-/class ScPublicAttrObj : public SdrAttrObj
//-/{
//-/private:
//-/    ScPublicAttrObj() {}                        // wird nicht angelegt
//-/public:
//-/    const XLineAttrSetItem* GetLineAttr()       { return pLineAttr; }
//-/};

//------------------------------------------------------------------------

class ScDetectiveData
{
private:
    SfxItemSet  aBoxSet;
    SfxItemSet  aArrowSet;
    SfxItemSet  aToTabSet;
    SfxItemSet  aFromTabSet;
    SfxItemSet  aCircleSet;         //! einzeln ?
    USHORT      nMaxLevel;

public:
                ScDetectiveData( SdrModel* pModel );

    SfxItemSet& GetBoxSet()     { return aBoxSet; }
    SfxItemSet& GetArrowSet()   { return aArrowSet; }
    SfxItemSet& GetToTabSet()   { return aToTabSet; }
    SfxItemSet& GetFromTabSet() { return aFromTabSet; }
    SfxItemSet& GetCircleSet()  { return aCircleSet; }

    void        SetMaxLevel( USHORT nVal )      { nMaxLevel = nVal; }
    USHORT      GetMaxLevel() const             { return nMaxLevel; }
};

class ScCommentData
{
private:
    SfxItemSet  aCaptionSet;

public:
                ScCommentData( ScDocument* pDoc, SdrModel* pModel );

    SfxItemSet& GetCaptionSet() { return aCaptionSet; }
};

//------------------------------------------------------------------------

ScDetectiveData::ScDetectiveData( SdrModel* pModel ) :
    aBoxSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aArrowSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aToTabSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aFromTabSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aCircleSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END )
{
    nMaxLevel = 0;

    aBoxSet.Put( XLineColorItem( EMPTY_STRING, Color( COL_LIGHTBLUE ) ) );
    aBoxSet.Put( XFillStyleItem( XFILL_NONE ) );

    //  #66479# Standard-Linienenden (wie aus XLineEndList::Create) selber zusammenbasteln,
    //  um von den konfigurierten Linienenden unabhaengig zu sein

    XPolygon aTriangle(3);
    aTriangle[0].X()=10; aTriangle[0].Y()= 0;
    aTriangle[1].X()= 0; aTriangle[1].Y()=30;
    aTriangle[2].X()=20; aTriangle[2].Y()=30;

    XPolygon aSquare(4);
    aSquare[0].X()= 0; aSquare[0].Y()= 0;
    aSquare[1].X()=10; aSquare[1].Y()= 0;
    aSquare[2].X()=10; aSquare[2].Y()=10;
    aSquare[3].X()= 0; aSquare[3].Y()=10;

    XPolygon aCircle(Point(0,0),100,100);

    String aName = SC_LINEEND_NAME;

    aArrowSet.Put( XLineStartItem( aName, aCircle ) );
    aArrowSet.Put( XLineStartWidthItem( 200 ) );
    aArrowSet.Put( XLineStartCenterItem( TRUE ) );
    aArrowSet.Put( XLineEndItem( aName, aTriangle ) );
    aArrowSet.Put( XLineEndWidthItem( 200 ) );
    aArrowSet.Put( XLineEndCenterItem( FALSE ) );

    aToTabSet.Put( XLineStartItem( aName, aCircle ) );
    aToTabSet.Put( XLineStartWidthItem( 200 ) );
    aToTabSet.Put( XLineStartCenterItem( TRUE ) );
    aToTabSet.Put( XLineEndItem( aName, aSquare ) );
    aToTabSet.Put( XLineEndWidthItem( 300 ) );
    aToTabSet.Put( XLineEndCenterItem( FALSE ) );

    aFromTabSet.Put( XLineStartItem( aName, aSquare ) );
    aFromTabSet.Put( XLineStartWidthItem( 300 ) );
    aFromTabSet.Put( XLineStartCenterItem( TRUE ) );
    aFromTabSet.Put( XLineEndItem( aName, aTriangle ) );
    aFromTabSet.Put( XLineEndWidthItem( 200 ) );
    aFromTabSet.Put( XLineEndCenterItem( FALSE ) );

    aCircleSet.Put( XLineColorItem( String(), Color( COL_LIGHTRED ) ) );
    aCircleSet.Put( XFillStyleItem( XFILL_NONE ) );
    USHORT nWidth = 55;     // 54 = 1 Pixel
    aCircleSet.Put( XLineWidthItem( nWidth ) );
}

ScCommentData::ScCommentData( ScDocument* pDoc, SdrModel* pModel ) :
    aCaptionSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END,
                                        EE_CHAR_START, EE_CHAR_END, 0 )
{
    XPolygon aTriangle(3);
    aTriangle[0].X()=10; aTriangle[0].Y()= 0;
    aTriangle[1].X()= 0; aTriangle[1].Y()=30;
    aTriangle[2].X()=20; aTriangle[2].Y()=30;

    String aName = SC_LINEEND_NAME;

    aCaptionSet.Put( XLineStartItem( aName, aTriangle ) );
    aCaptionSet.Put( XLineStartWidthItem( 200 ) );
    aCaptionSet.Put( XLineStartCenterItem( FALSE ) );
    aCaptionSet.Put( XFillStyleItem( XFILL_SOLID ) );
#ifdef VCL
    Color aYellow( 255,255,192 );           // hellgelb
#else
    Color aYellow( 65535,65535,49152 );     // hellgelb
#endif
    aCaptionSet.Put( XFillColorItem( String(), aYellow ) );

    //  shadow
    //  SdrShadowItem has FALSE, instead the shadow is set for the rectangle
    //  only with SetSpecialTextBoxShadow when the object is created
    //  (item must be set to adjust objects from older files)
    aCaptionSet.Put( SdrShadowItem( FALSE ) );
    aCaptionSet.Put( SdrShadowXDistItem( 100 ) );
    aCaptionSet.Put( SdrShadowYDistItem( 100 ) );

    //  text attributes
    aCaptionSet.Put( SdrTextLeftDistItem( 100 ) );
    aCaptionSet.Put( SdrTextRightDistItem( 100 ) );
    aCaptionSet.Put( SdrTextUpperDistItem( 100 ) );
    aCaptionSet.Put( SdrTextLowerDistItem( 100 ) );

    //  #78943# do use the default cell style, so the user has a chance to
    //  modify the font for the annotations
    ((const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).
        FillEditItemSet( &aCaptionSet );
}

//------------------------------------------------------------------------

inline BOOL Intersect( USHORT nStartCol1, USHORT nStartRow1, USHORT nEndCol1, USHORT nEndRow1,
                        USHORT nStartCol2, USHORT nStartRow2, USHORT nEndCol2, USHORT nEndRow2 )
{
    return nEndCol1 >= nStartCol2 && nEndCol2 >= nStartCol1 &&
            nEndRow1 >= nStartRow2 && nEndRow2 >= nStartRow1;
}

BOOL ScDetectiveFunc::HasError( const ScTripel& rStart, const ScTripel& rEnd, ScTripel& rErrPos )
{
    rErrPos = rStart;
    USHORT nError = 0;

    ScCellIterator aCellIter( pDoc, rStart.GetCol(), rStart.GetRow(), rStart.GetTab(),
                                    rEnd.GetCol(), rEnd.GetRow(), rEnd.GetTab() );
    ScBaseCell* pCell = aCellIter.GetFirst();
    while (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            nError = ((ScFormulaCell*)pCell)->GetErrCode();
            if (nError)
                rErrPos.Put( aCellIter.GetCol(), aCellIter.GetRow(), aCellIter.GetTab() );
        }
        pCell = aCellIter.GetNext();
    }

    return (nError != 0);
}

Point ScDetectiveFunc::GetDrawPos( USHORT nCol, USHORT nRow, BOOL bArrow )
{
    //  MAXCOL/ROW+1 ist erlaubt fuer Ende von Rahmen
    if (nCol > MAXCOL+1)
    {
        DBG_ERROR("falsche Col in ScDetectiveFunc::GetDrawPos");
        nCol = MAXCOL+1;
    }
    if (nRow > MAXROW+1)
    {
        DBG_ERROR("falsche Row in ScDetectiveFunc::GetDrawPos");
        nRow = MAXROW+1;
    }

    Point aPos;
    USHORT i;
    USHORT nLocalTab = nTab;        // nicht ueber this

    for (i=0; i<nCol; i++)
        aPos.X() += pDoc->GetColWidth( i,nLocalTab );
    for (i=0; i<nRow; i++)
        aPos.Y() += pDoc->FastGetRowHeight( i,nLocalTab );

    if (bArrow)
    {
        if (nCol<=MAXCOL)
            aPos.X() += pDoc->GetColWidth( nCol, nLocalTab ) / 4;
        if (nCol<=MAXROW)
            aPos.Y() += pDoc->GetRowHeight( nRow, nLocalTab ) / 2;
    }

    aPos.X() = (long) ( aPos.X() * HMM_PER_TWIPS );
    aPos.Y() = (long) ( aPos.Y() * HMM_PER_TWIPS );

    return aPos;
}

BOOL ScDetectiveFunc::HasArrow( USHORT nStartCol, USHORT nStartRow, USHORT nStartTab,
                                    USHORT nEndCol, USHORT nEndRow, USHORT nEndTab )
{
    BOOL bStartAlien = ( nStartTab != nTab );
    BOOL bEndAlien   = ( nEndTab != nTab );

    if (bStartAlien && bEndAlien)
    {
        DBG_ERROR("bStartAlien && bEndAlien");
        return TRUE;
    }

    Rectangle aStartRect;
    Rectangle aEndRect;
    if (!bStartAlien)
    {
        Point aStartPos = GetDrawPos( nStartCol, nStartRow, FALSE );
        Size aStartSize = Size(
                            (long) ( pDoc->GetColWidth( nStartCol, nTab) * HMM_PER_TWIPS ),
                            (long) ( pDoc->GetRowHeight( nStartRow, nTab) * HMM_PER_TWIPS ) );
        aStartRect = Rectangle( aStartPos, aStartSize );
    }
    if (!bEndAlien)
    {
        Point aEndPos = GetDrawPos( nEndCol, nEndRow, FALSE );
        Size aEndSize = Size(
                            (long) ( pDoc->GetColWidth( nEndCol, nTab) * HMM_PER_TWIPS ),
                            (long) ( pDoc->GetRowHeight( nEndRow, nTab) * HMM_PER_TWIPS ) );
        aEndRect = Rectangle( aEndPos, aEndSize );
    }

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");

    BOOL bFound = FALSE;
    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetLayer()==SC_LAYER_INTERN &&
                pObject->IsPolyObj() && pObject->GetPointCount()==2 )
        {
            BOOL bObjStartAlien =
                (4 == ((const XLineStartItem&)pObject->GetItem(XATTR_LINESTART)).GetValue().GetPointCount());
            BOOL bObjEndAlien =
                (4 == ((const XLineEndItem&)pObject->GetItem(XATTR_LINEEND)).GetValue().GetPointCount());

//-/            BOOL bObjStartAlien = FALSE;
//-/            BOOL bObjEndAlien = FALSE;
//-/            const XLineAttrSetItem* pLineAttrs =
//-/                ((ScPublicAttrObj*)(SdrAttrObj*)pObject)->GetLineAttr();
//-/            if (pLineAttrs)
//-/            {
//-/                const SfxItemSet& rSet = pLineAttrs->GetItemSet();
//-/                bObjStartAlien = (((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).
//-/                                        GetValue().GetPointCount() == 4 );
//-/                bObjEndAlien   = (((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).
//-/                                        GetValue().GetPointCount() == 4 );
//-/            }

            BOOL bStartHit = bStartAlien ? bObjStartAlien :
                                ( !bObjStartAlien && aStartRect.IsInside(pObject->GetPoint(0)) );
            BOOL bEndHit = bEndAlien ? bObjEndAlien :
                                ( !bObjEndAlien && aEndRect.IsInside(pObject->GetPoint(1)) );

            if ( bStartHit && bEndHit )
                bFound = TRUE;
        }
        pObject = aIter.Next();
    }

    return bFound;
}

BOOL ScDetectiveFunc::IsNonAlienArrow( SdrObject* pObject )         // static
{
    if ( pObject->GetLayer()==SC_LAYER_INTERN &&
            pObject->IsPolyObj() && pObject->GetPointCount()==2 )
    {
        BOOL bObjStartAlien =
            (4 == ((const XLineStartItem&)pObject->GetItem(XATTR_LINESTART)).GetValue().GetPointCount());
        BOOL bObjEndAlien =
            (4 == ((const XLineEndItem&)pObject->GetItem(XATTR_LINEEND)).GetValue().GetPointCount());

//-/        BOOL bObjStartAlien = FALSE;
//-/        BOOL bObjEndAlien = FALSE;
//-/        const XLineAttrSetItem* pLineAttrs =
//-/                ((ScPublicAttrObj*)(SdrAttrObj*)pObject)->GetLineAttr();
//-/        if (pLineAttrs)
//-/        {
//-/            const SfxItemSet& rSet = pLineAttrs->GetItemSet();
//-/            bObjStartAlien = (((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).
//-/                                    GetValue().GetPointCount() == 4 );
//-/            bObjEndAlien   = (((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).
//-/                                    GetValue().GetPointCount() == 4 );
//-/        }
        return !bObjStartAlien && !bObjEndAlien;
    }

    return FALSE;
}

//  DrawEntry:      Formel auf dieser Tabelle,
//                  Referenz auf dieser oder anderer
//  DrawAlienEntry: Formel auf anderer Tabelle,
//                  Referenz auf dieser

//      return FALSE: da war schon ein Pfeil

BOOL ScDetectiveFunc::DrawEntry( USHORT nCol, USHORT nRow,
                                    const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                    ScDetectiveData& rData )
{
    if ( HasArrow( rRefStart.GetCol(), rRefStart.GetRow(), rRefStart.GetTab(),
                    nCol, nRow, nTab ) )
        return FALSE;

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(nTab);

    BOOL bArea = (rRefStart != rRefEnd);
    BOOL bAlien = ( rRefEnd.GetTab() < nTab || rRefStart.GetTab() > nTab );
    if (bArea && !bAlien)
    {
        Point aStartCorner = GetDrawPos( rRefStart.GetCol(), rRefStart.GetRow(), FALSE );
        Point aEndCorner = GetDrawPos( rRefEnd.GetCol()+1, rRefEnd.GetRow()+1, FALSE );

        SdrRectObj* pBox = new SdrRectObj(Rectangle(aStartCorner,aEndCorner));

//-/        pBox->SetAttributes( rData.GetBoxSet(), FALSE );
        pBox->SetItemSetAndBroadcast(rData.GetBoxSet());

        ScDrawLayer::SetAnchor( pBox, SCA_CELL );
        pBox->SetLayer( SC_LAYER_INTERN );
        pPage->InsertObject( pBox );
        pModel->AddCalcUndo( new SdrUndoInsertObj( *pBox ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pBox, TRUE );
        pData->aStt = rRefStart;
        pData->aEnd = rRefEnd;
        pData->bValidStart = TRUE;
        pData->bValidEnd = TRUE;
    }

    Point aStartPos = GetDrawPos( rRefStart.GetCol(), rRefStart.GetRow(), TRUE );
    Point aEndPos   = GetDrawPos( nCol, nRow, TRUE );

    if (bAlien)
    {
        aStartPos = Point( aEndPos.X() - 1000, aEndPos.Y() - 1000 );
        if (aStartPos.X() < 0)
            aStartPos.X() += 2000;
        if (aStartPos.Y() < 0)
            aStartPos.Y() += 2000;
    }

    SfxItemSet& rAttrSet = bAlien ? rData.GetFromTabSet() : rData.GetArrowSet();

    if (bArea && !bAlien)
        rAttrSet.Put( XLineWidthItem( 50 ) );               // Bereich
    else
        rAttrSet.Put( XLineWidthItem( 0 ) );                // einzelne Referenz

    ColorData nColorData;
    ScTripel aErrorPos;
    if (HasError( rRefStart, rRefEnd, aErrorPos ))
        nColorData = COL_LIGHTRED;
    else
        nColorData = COL_LIGHTBLUE;

    rAttrSet.Put( XLineColorItem( String(), Color( nColorData ) ) );
    Point aPointArr[2] = {aStartPos, aEndPos};
    SdrPathObj* pArrow = new SdrPathObj(OBJ_LINE,
                XPolyPolygon(XPolygon(Polygon(2, aPointArr))));

    pArrow->NbcSetLogicRect(Rectangle(aStartPos,aEndPos));  //! noetig ???

//-/    pArrow->SetAttributes( rAttrSet, FALSE );
    pArrow->SetItemSetAndBroadcast(rAttrSet);

    ScDrawLayer::SetAnchor( pArrow, SCA_CELL );
    pArrow->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pArrow );
    pModel->AddCalcUndo( new SdrUndoInsertObj( *pArrow ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pArrow, TRUE );
    if (bAlien)
        pData->bValidStart = FALSE;
    else
    {
        pData->aStt.nCol = rRefStart.GetCol();
        pData->aStt.nRow = rRefStart.GetRow();
        pData->aStt.nTab = rRefStart.GetTab();
        pData->bValidStart = TRUE;
    }

    pData->aEnd.nCol = nCol;
    pData->aEnd.nRow = nRow;
    pData->aEnd.nTab = nTab;
    pData->bValidEnd = TRUE;

    return TRUE;
}

BOOL ScDetectiveFunc::DrawAlienEntry( const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                        ScDetectiveData& rData )
{
    if ( HasArrow( rRefStart.GetCol(), rRefStart.GetRow(), rRefStart.GetTab(),
                    0, 0, nTab+1 ) )
        return FALSE;

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(nTab);

    BOOL bArea = (rRefStart != rRefEnd);
    if (bArea)
    {
        Point aStartCorner = GetDrawPos( rRefStart.GetCol(), rRefStart.GetRow(), FALSE );
        Point aEndCorner = GetDrawPos( rRefEnd.GetCol()+1, rRefEnd.GetRow()+1, FALSE );

        SdrRectObj* pBox = new SdrRectObj(Rectangle(aStartCorner,aEndCorner));

//-/        pBox->SetAttributes( rData.GetBoxSet(), FALSE );
        pBox->SetItemSetAndBroadcast(rData.GetBoxSet());

        ScDrawLayer::SetAnchor( pBox, SCA_CELL );
        pBox->SetLayer( SC_LAYER_INTERN );
        pPage->InsertObject( pBox );
        pModel->AddCalcUndo( new SdrUndoInsertObj( *pBox ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pBox, TRUE );
        pData->aStt = rRefStart;
        pData->aEnd = rRefEnd;
        pData->bValidStart = TRUE;
        pData->bValidEnd = TRUE;
    }

    Point aStartPos = GetDrawPos( rRefStart.GetCol(), rRefStart.GetRow(), TRUE );
    Point aEndPos   = Point( aStartPos.X() + 1000, aStartPos.Y() - 1000 );
    if (aEndPos.Y() < 0)
        aEndPos.Y() += 2000;

    SfxItemSet& rAttrSet = rData.GetToTabSet();
    if (bArea)
        rAttrSet.Put( XLineWidthItem( 50 ) );               // Bereich
    else
        rAttrSet.Put( XLineWidthItem( 0 ) );                // einzelne Referenz

    ColorData nColorData;
    ScTripel aErrorPos;
    if (HasError( rRefStart, rRefEnd, aErrorPos ))
        nColorData = COL_LIGHTRED;
    else
        nColorData = COL_LIGHTBLUE;
    rAttrSet.Put( XLineColorItem( String(), Color( nColorData ) ) );
    Point aPointArr[2] = {aStartPos, aEndPos};
    SdrPathObj* pArrow = new SdrPathObj(OBJ_LINE,
                XPolyPolygon(XPolygon(Polygon(2, aPointArr))));

    pArrow->NbcSetLogicRect(Rectangle(aStartPos,aEndPos));  //! noetig ???

//-/    pArrow->SetAttributes( rAttrSet, FALSE );
    pArrow->SetItemSetAndBroadcast(rAttrSet);

    ScDrawLayer::SetAnchor( pArrow, SCA_CELL );
    pArrow->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pArrow );
    pModel->AddCalcUndo( new SdrUndoInsertObj( *pArrow ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pArrow, TRUE );
    pData->aStt.nCol = rRefStart.GetCol();
    pData->aStt.nRow = rRefStart.GetRow();
    pData->aStt.nTab = rRefStart.GetTab();
    pData->bValidStart = TRUE;
    pData->bValidEnd = FALSE;

    return TRUE;
}

void ScDetectiveFunc::DrawCircle( USHORT nCol, USHORT nRow, ScDetectiveData& rData )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(nTab);

    Point aStartPos = GetDrawPos( nCol, nRow, FALSE );
    Size aSize( (long) ( pDoc->GetColWidth(nCol, nTab) * HMM_PER_TWIPS ),
                (long) ( pDoc->GetRowHeight(nRow, nTab) * HMM_PER_TWIPS ) );
    Rectangle aRect( aStartPos, aSize );
    aRect.Left()    -= 250;
    aRect.Right()   += 250;
    aRect.Top()     -= 70;
    aRect.Bottom()  += 70;

    SdrCircObj* pCircle = new SdrCircObj( OBJ_CIRC, aRect );
    SfxItemSet& rAttrSet = rData.GetCircleSet();

//-/    pCircle->SetAttributes( rAttrSet, FALSE );
    pCircle->SetItemSetAndBroadcast(rAttrSet);

    ScDrawLayer::SetAnchor( pCircle, SCA_CELL );
    pCircle->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pCircle );
    pModel->AddCalcUndo( new SdrUndoInsertObj( *pCircle ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pCircle, TRUE );
    pData->aStt.nCol = nCol;
    pData->aStt.nRow = nRow;
    pData->aStt.nTab = nTab;
    pData->bValidStart = TRUE;
    pData->bValidEnd = FALSE;
}

SdrObject* ScDetectiveFunc::DrawCaption( USHORT nCol, USHORT nRow, const String& rText,
                                            ScCommentData& rData, SdrPage* pDestPage,
                                            BOOL bHasUserText, BOOL bLeft,
                                            const Rectangle& rVisible )
{
    ScDrawLayer* pModel = NULL;     // muss ScDrawLayer* sein wegen AddCalcUndo !!!
    SdrPage* pPage = pDestPage;
    if (!pPage)                     // keine angegeben?
    {
        pModel = pDoc->GetDrawLayer();
        pPage = pModel->GetPage(nTab);
    }

    USHORT nNextCol = nCol+1;
    const ScMergeAttr* pMerge = (const ScMergeAttr*) pDoc->GetAttr( nCol,nRow,nTab, ATTR_MERGE );
    if ( pMerge->GetColMerge() > 1 )
        nNextCol = nCol + pMerge->GetColMerge();

    Point aTailPos = GetDrawPos( nNextCol, nRow, FALSE );
    Point aRectPos = aTailPos;
    if ( bLeft )
    {
        aTailPos = GetDrawPos( nCol, nRow, FALSE );
        aTailPos.X() += 10;             // links knapp innerhalb der Zelle
    }
    else
        aTailPos.X() -= 10;             // knapp vor die naechste Zelle zeigen

    //  arrow head should be visible (if visible rectangle is set)
    if ( aTailPos.X() > rVisible.Right() && rVisible.Right() )
        aTailPos.X() = rVisible.Right();

    aRectPos.X() += 600;
    aRectPos.Y() -= 1500;
    if ( aRectPos.Y() < rVisible.Top() ) aRectPos.Y() = rVisible.Top();

    //  links wird spaeter getestet

    //  bei Textlaenge > SC_NOTE_SMALLTEXT wird die Breite verdoppelt...
    long nDefWidth = ( rText.Len() > SC_NOTE_SMALLTEXT ) ? 5800 : 2900;
    Size aRectSize( nDefWidth, 1800 );      // Hoehe wird hinterher angepasst

    long nMaxWidth = 10000;             //! oder wie?
    if ( !bHasUserText )
        nMaxWidth = aRectSize.Width();  // Notiz nicht zu gross

    if ( rVisible.Right() )
    {
        nMaxWidth = rVisible.Right() - aRectPos.X() - 100;
        if (nMaxWidth < nDefWidth)
        {
            aRectPos.X() -= nDefWidth - nMaxWidth;
            nMaxWidth = nDefWidth;
        }
    }
    if ( aRectPos.X() < rVisible.Left() )
        aRectPos.X() = rVisible.Left();

    SdrCaptionObj* pCaption = new SdrCaptionObj( Rectangle( aRectPos,aRectSize ), aTailPos );
    SfxItemSet& rAttrSet = rData.GetCaptionSet();
    if (bHasUserText)
    {
        rAttrSet.Put(SdrTextAutoGrowWidthItem(TRUE));
        rAttrSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT));
        rAttrSet.Put(SdrTextMaxFrameWidthItem(nMaxWidth));
    }

    ScDrawLayer::SetAnchor( pCaption, SCA_CELL );
    pCaption->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pCaption );

    // #78611# for SetText, the object must already be inserted
    pCaption->SetText( rText );

    //  SetAttributes must be after SetText, because the font attributes
    //  are applied to the text.
//-/    pCaption->SetAttributes( rAttrSet, FALSE );
    pCaption->SetItemSetAndBroadcast(rAttrSet);

    pCaption->SetSpecialTextBoxShadow();

    Rectangle aLogic = pCaption->GetLogicRect();
    Rectangle aOld = aLogic;
    if (bHasUserText)
        pCaption->AdjustTextFrameWidthAndHeight( aLogic, TRUE, TRUE );
    else
        pCaption->AdjustTextFrameWidthAndHeight( aLogic, TRUE, FALSE );
    if (rVisible.Bottom())
    {
        //  unterer Rand kann erst nach dem AdjustTextFrameWidthAndHeight getestet werden
        if ( aLogic.Bottom() > rVisible.Bottom() )
        {
            long nDif = aLogic.Bottom() - rVisible.Bottom();
            aLogic.Bottom() = rVisible.Bottom();
            aLogic.Top() = Max( rVisible.Top(), (long)(aLogic.Top() - nDif) );
        }
    }
    if (aLogic != aOld)
        pCaption->SetLogicRect(aLogic);

    //  Undo und UserData nur, wenn's im Dokument ist, also keine Page angegeben war
    if ( !pDestPage )
    {
        pModel->AddCalcUndo( new SdrUndoInsertObj( *pCaption ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pCaption, TRUE );
        pData->aStt.nCol = nCol;
        pData->aStt.nRow = nRow;
        pData->aStt.nTab = nTab;
        pData->bValidStart = TRUE;
        pData->bValidEnd = FALSE;
    }

    return pCaption;
}

void ScDetectiveFunc::DeleteArrowsAt( USHORT nCol, USHORT nRow, BOOL bDestPnt )
{
    Point aPos = GetDrawPos( nCol, nRow, FALSE );
    Size aSize = Size(  (long) ( pDoc->GetColWidth( nCol, nTab) * HMM_PER_TWIPS ),
                        (long) ( pDoc->GetRowHeight( nRow, nTab) * HMM_PER_TWIPS ) );
    Rectangle aRect( aPos, aSize );

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");

    pPage->RecalcObjOrdNums();

    long    nDelCount = 0;
    ULONG   nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        SdrObject** ppObj = new SdrObject*[nObjCount];

        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetLayer()==SC_LAYER_INTERN &&
                    pObject->IsPolyObj() && pObject->GetPointCount()==2 )
            {
                if (aRect.IsInside(pObject->GetPoint(bDestPnt)))            // Start/Zielpunkt
                    ppObj[nDelCount++] = pObject;
            }

            pObject = aIter.Next();
        }

        long i;
        for (i=1; i<=nDelCount; i++)
            pModel->AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );

        for (i=1; i<=nDelCount; i++)
            pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );

        delete[] ppObj;
    }
}

        //      Box um Referenz loeschen

#define SC_DET_TOLERANCE    50

inline BOOL RectIsPoints( const Rectangle& rRect, const Point& rStart, const Point& rEnd )
{
    return rRect.Left()   >= rStart.X() - SC_DET_TOLERANCE
        && rRect.Left()   <= rStart.X() + SC_DET_TOLERANCE
        && rRect.Right()  >= rEnd.X()   - SC_DET_TOLERANCE
        && rRect.Right()  <= rEnd.X()   + SC_DET_TOLERANCE
        && rRect.Top()    >= rStart.Y() - SC_DET_TOLERANCE
        && rRect.Top()    <= rStart.Y() + SC_DET_TOLERANCE
        && rRect.Bottom() >= rEnd.Y()   - SC_DET_TOLERANCE
        && rRect.Bottom() <= rEnd.Y()   + SC_DET_TOLERANCE;
}

#undef SC_DET_TOLERANCE

void ScDetectiveFunc::DeleteBox( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 )
{
/*  String aStr;
    aStr += nCol1;
    aStr += '/';
    aStr += nRow1;
    aStr += '/';
    aStr += nCol2;
    aStr += '/';
    aStr += nRow2;
    InfoBox(0,aStr).Execute();
*/

    Point aStartCorner = GetDrawPos( nCol1, nRow1, FALSE );
    Point aEndCorner = GetDrawPos( nCol2+1, nRow2+1, FALSE );
    Rectangle aObjRect;

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");

    pPage->RecalcObjOrdNums();

    long    nDelCount = 0;
    ULONG   nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        SdrObject** ppObj = new SdrObject*[nObjCount];

        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetLayer() == SC_LAYER_INTERN &&
                    pObject->Type() == TYPE(SdrRectObj) )
            {
                aObjRect = ((SdrRectObj*)pObject)->GetLogicRect();
                if ( RectIsPoints( aObjRect, aStartCorner, aEndCorner ) )
                    ppObj[nDelCount++] = pObject;
            }

            pObject = aIter.Next();
        }

        long i;
        for (i=1; i<=nDelCount; i++)
            pModel->AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );

        for (i=1; i<=nDelCount; i++)
            pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );

        delete[] ppObj;
    }
}

//------------------------------------------------------------------------

USHORT ScDetectiveFunc::InsertPredLevelArea( const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                        ScDetectiveData& rData, USHORT nLevel )
{
    USHORT nResult = DET_INS_EMPTY;

    ScCellIterator aCellIter( pDoc, rRefStart.GetCol(), rRefStart.GetRow(), rRefStart.GetTab(),
                                    rRefEnd.GetCol(), rRefEnd.GetRow(), rRefEnd.GetTab() );
    ScBaseCell* pCell = aCellIter.GetFirst();
    while (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
            switch( InsertPredLevel( aCellIter.GetCol(), aCellIter.GetRow(), rData, nLevel ) )
            {
                case DET_INS_INSERTED:
                    nResult = DET_INS_INSERTED;
                    break;
                case DET_INS_CONTINUE:
                    if (nResult != DET_INS_INSERTED)
                        nResult = DET_INS_CONTINUE;
                    break;
                case DET_INS_CIRCULAR:
                    if (nResult == DET_INS_EMPTY)
                        nResult = DET_INS_CIRCULAR;
                    break;
            }

        pCell = aCellIter.GetNext();
    }

    return nResult;
}

USHORT ScDetectiveFunc::InsertPredLevel( USHORT nCol, USHORT nRow, ScDetectiveData& rData,
                                            USHORT nLevel )
{
    ScBaseCell* pCell;
    pDoc->GetCell( nCol, nRow, nTab, pCell );
    if (!pCell)
        return DET_INS_EMPTY;
    if (pCell->GetCellType() != CELLTYPE_FORMULA)
        return DET_INS_EMPTY;

    ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
    if (pFCell->IsRunning())
        return DET_INS_CIRCULAR;

    if (pFCell->GetDirty())
        pFCell->Interpret();                // nach SetRunning geht's nicht mehr!
    pFCell->SetRunning(TRUE);

    USHORT nResult = DET_INS_EMPTY;

    ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
    ScTripel aRefStart;
    ScTripel aRefEnd;
    while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
    {
        if (DrawEntry( nCol, nRow, aRefStart, aRefEnd, rData ))
        {
            nResult = DET_INS_INSERTED;         //  neuer Pfeil eingetragen
        }
        else
        {
            //  weiterverfolgen

            if ( nLevel < rData.GetMaxLevel() )
            {
                USHORT nSubResult;
                BOOL bArea = (aRefStart != aRefEnd);
                if (bArea)
                    nSubResult = InsertPredLevelArea( aRefStart, aRefEnd, rData, nLevel+1 );
                else
                    nSubResult = InsertPredLevel( aRefStart.GetCol(), aRefStart.GetRow(),
                                                    rData, nLevel+1 );

                switch (nSubResult)
                {
                    case DET_INS_INSERTED:
                        nResult = DET_INS_INSERTED;
                        break;
                    case DET_INS_CONTINUE:
                        if (nResult != DET_INS_INSERTED)
                            nResult = DET_INS_CONTINUE;
                        break;
                    case DET_INS_CIRCULAR:
                        if (nResult == DET_INS_EMPTY)
                            nResult = DET_INS_CIRCULAR;
                        break;
                    // DET_INS_EMPTY: unveraendert lassen
                }
            }
            else                                    //  nMaxLevel erreicht
                if (nResult != DET_INS_INSERTED)
                    nResult = DET_INS_CONTINUE;
        }
    }

    pFCell->SetRunning(FALSE);

    return nResult;
}

USHORT ScDetectiveFunc::FindPredLevelArea( const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                                USHORT nLevel, USHORT nDeleteLevel )
{
    USHORT nResult = nLevel;

    ScCellIterator aCellIter( pDoc, rRefStart.GetCol(), rRefStart.GetRow(), rRefStart.GetTab(),
                                    rRefEnd.GetCol(), rRefEnd.GetRow(), rRefEnd.GetTab() );
    ScBaseCell* pCell = aCellIter.GetFirst();
    while (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            USHORT nTemp = FindPredLevel( aCellIter.GetCol(), aCellIter.GetRow(), nLevel, nDeleteLevel );
            if (nTemp > nResult)
                nResult = nTemp;
        }
        pCell = aCellIter.GetNext();
    }

    return nResult;
}

                                            //  nDeleteLevel != 0   -> loeschen

USHORT ScDetectiveFunc::FindPredLevel( USHORT nCol, USHORT nRow, USHORT nLevel, USHORT nDeleteLevel )
{
    DBG_ASSERT( nLevel<1000, "Level" );

    ScBaseCell* pCell;
    pDoc->GetCell( nCol, nRow, nTab, pCell );
    if (!pCell)
        return nLevel;
    if (pCell->GetCellType() != CELLTYPE_FORMULA)
        return nLevel;

    ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
    if (pFCell->IsRunning())
        return nLevel;

    if (pFCell->GetDirty())
        pFCell->Interpret();                // nach SetRunning geht's nicht mehr!
    pFCell->SetRunning(TRUE);

    USHORT nResult = nLevel;
    BOOL bDelete = ( nDeleteLevel && nLevel == nDeleteLevel-1 );

    if ( bDelete )
    {
        DeleteArrowsAt( nCol, nRow, TRUE );                 // Pfeile, die hierher zeigen
    }

    ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
    ScTripel aRefStart;
    ScTripel aRefEnd;
    while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
    {
        BOOL bArea = ( aRefStart != aRefEnd );

        if ( bDelete )                  // Rahmen loeschen ?
        {
            if (bArea)
            {
                DeleteBox( aRefStart.GetCol(), aRefStart.GetRow(), aRefEnd.GetCol(), aRefEnd.GetRow() );
            }
        }
        else                            // weitersuchen
        {
            if ( HasArrow( aRefStart.GetCol(),aRefStart.GetRow(),aRefStart.GetTab(),
                            nCol,nRow,nTab ) )
            {
                USHORT nTemp;
                if (bArea)
                    nTemp = FindPredLevelArea( aRefStart, aRefEnd, nLevel+1, nDeleteLevel );
                else
                    nTemp = FindPredLevel( aRefStart.GetCol(),aRefStart.GetRow(),
                                                        nLevel+1, nDeleteLevel );
                if (nTemp > nResult)
                    nResult = nTemp;
            }
        }
    }

    pFCell->SetRunning(FALSE);

    return nResult;
}

//------------------------------------------------------------------------

USHORT ScDetectiveFunc::InsertErrorLevel( USHORT nCol, USHORT nRow, ScDetectiveData& rData,
                                            USHORT nLevel )
{
    ScBaseCell* pCell;
    pDoc->GetCell( nCol, nRow, nTab, pCell );
    if (!pCell)
        return DET_INS_EMPTY;
    if (pCell->GetCellType() != CELLTYPE_FORMULA)
        return DET_INS_EMPTY;

    ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
    if (pFCell->IsRunning())
        return DET_INS_CIRCULAR;

    if (pFCell->GetDirty())
        pFCell->Interpret();                // nach SetRunning geht's nicht mehr!
    pFCell->SetRunning(TRUE);

    USHORT nResult = DET_INS_EMPTY;

    ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
    ScTripel aRefStart;
    ScTripel aRefEnd;
    ScTripel aErrorPos;
    BOOL bHasError = FALSE;
    while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
    {
        if (HasError( aRefStart, aRefEnd, aErrorPos ))
        {
            bHasError = TRUE;
            if (DrawEntry( nCol, nRow, aErrorPos, aErrorPos, rData ))
                nResult = DET_INS_INSERTED;

            //  und weiterverfolgen

            if ( nLevel < rData.GetMaxLevel() )         // praktisch immer
            {
                if (InsertErrorLevel( aErrorPos.GetCol(), aErrorPos.GetRow(),
                                                        rData, nLevel+1 ) == DET_INS_INSERTED)
                    nResult = DET_INS_INSERTED;
            }
        }
    }

    pFCell->SetRunning(FALSE);

                                                    // Blaetter ?
    if (!bHasError)
        if (InsertPredLevel( nCol, nRow, rData, rData.GetMaxLevel() ) == DET_INS_INSERTED)
            nResult = DET_INS_INSERTED;

    return nResult;
}

//------------------------------------------------------------------------

USHORT ScDetectiveFunc::InsertSuccLevel( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                        ScDetectiveData& rData, USHORT nLevel )
{
    //  ueber ganzes Dokument

    USHORT nResult = DET_INS_EMPTY;
//  ScCellIterator aCellIter( pDoc, 0,0, nTab, MAXCOL,MAXROW, nTab );
    ScCellIterator aCellIter( pDoc, 0,0,0, MAXCOL,MAXROW,MAXTAB );          // alle Tabellen
    ScBaseCell* pCell = aCellIter.GetFirst();
    while (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
            BOOL bRunning = pFCell->IsRunning();

            if (pFCell->GetDirty())
                pFCell->Interpret();                // nach SetRunning geht's nicht mehr!
            pFCell->SetRunning(TRUE);

            ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
            ScTripel aRefStart;
            ScTripel aRefEnd;
            while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
            {
                if (aRefStart.GetTab() <= nTab && aRefEnd.GetTab() >= nTab)
                {
                    if (Intersect( nCol1,nRow1,nCol2,nRow2,
                            aRefStart.GetCol(),aRefStart.GetRow(),
                            aRefEnd.GetCol(),aRefEnd.GetRow() ))
                    {
                        BOOL bAlien = ( aCellIter.GetTab() != nTab );
                        BOOL bDrawRet;
                        if (bAlien)
                            bDrawRet = DrawAlienEntry( aRefStart, aRefEnd, rData );
                        else
                            bDrawRet = DrawEntry( aCellIter.GetCol(), aCellIter.GetRow(),
                                                    aRefStart, aRefEnd, rData );
                        if (bDrawRet)
                        {
                            nResult = DET_INS_INSERTED;         //  neuer Pfeil eingetragen
                        }
                        else
                        {
                            if (bRunning)
                            {
                                if (nResult == DET_INS_EMPTY)
                                    nResult = DET_INS_CIRCULAR;
                            }
                            else
                            {
                                        //  weiterverfolgen

                                if ( nLevel < rData.GetMaxLevel() )
                                {
                                    USHORT nSubResult = InsertSuccLevel(
                                                            aCellIter.GetCol(), aCellIter.GetRow(),
                                                            aCellIter.GetCol(), aCellIter.GetRow(),
                                                            rData, nLevel+1 );
                                    switch (nSubResult)
                                    {
                                        case DET_INS_INSERTED:
                                            nResult = DET_INS_INSERTED;
                                            break;
                                        case DET_INS_CONTINUE:
                                            if (nResult != DET_INS_INSERTED)
                                                nResult = DET_INS_CONTINUE;
                                            break;
                                        case DET_INS_CIRCULAR:
                                            if (nResult == DET_INS_EMPTY)
                                                nResult = DET_INS_CIRCULAR;
                                            break;
                                        // DET_INS_EMPTY: unveraendert lassen
                                    }
                                }
                                else                                    //  nMaxLevel erreicht
                                    if (nResult != DET_INS_INSERTED)
                                        nResult = DET_INS_CONTINUE;
                            }
                        }
                    }
                }
            }
            pFCell->SetRunning(bRunning);
        }
        pCell = aCellIter.GetNext();
    }

    return nResult;
}

USHORT ScDetectiveFunc::FindSuccLevel( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                        USHORT nLevel, USHORT nDeleteLevel )
{
    DBG_ASSERT( nLevel<1000, "Level" );

    USHORT nResult = nLevel;
    BOOL bDelete = ( nDeleteLevel && nLevel == nDeleteLevel-1 );

    ScCellIterator aCellIter( pDoc, 0,0, nTab, MAXCOL,MAXROW, nTab );
    ScBaseCell* pCell = aCellIter.GetFirst();
    while (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
            BOOL bRunning = pFCell->IsRunning();

            if (pFCell->GetDirty())
                pFCell->Interpret();                // nach SetRunning geht's nicht mehr!
            pFCell->SetRunning(TRUE);

            ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
            ScTripel aRefStart;
            ScTripel aRefEnd;
            while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
            {
                if (aRefStart.GetTab() <= nTab && aRefEnd.GetTab() >= nTab)
                {
                    if (Intersect( nCol1,nRow1,nCol2,nRow2,
                            aRefStart.GetCol(),aRefStart.GetRow(),
                            aRefEnd.GetCol(),aRefEnd.GetRow() ))
                    {
                        if ( bDelete )                          // Pfeile, die hier anfangen
                        {
                            if (aRefStart != aRefEnd)
                            {
                                DeleteBox( aRefStart.GetCol(), aRefStart.GetRow(),
                                                aRefEnd.GetCol(), aRefEnd.GetRow() );
                            }
                            DeleteArrowsAt( aRefStart.GetCol(), aRefStart.GetRow(), FALSE );
                        }
                        else if ( !bRunning &&
                                HasArrow( aRefStart.GetCol(),aRefStart.GetRow(),aRefStart.GetTab(),
                                            aCellIter.GetCol(),aCellIter.GetRow(),aCellIter.GetTab() ) )
                        {
                            USHORT nTemp = FindSuccLevel( aCellIter.GetCol(), aCellIter.GetRow(),
                                                            aCellIter.GetCol(), aCellIter.GetRow(),
                                                            nLevel+1, nDeleteLevel );
                            if (nTemp > nResult)
                                nResult = nTemp;
                        }
                    }
                }
            }

            pFCell->SetRunning(bRunning);
        }
        pCell = aCellIter.GetNext();
    }

    return nResult;
}


//
//  --------------------------------------------------------------------------------
//

BOOL ScDetectiveFunc::ShowPred( USHORT nCol, USHORT nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");

    ScDetectiveData aData( pModel );

    USHORT nMaxLevel = 0;
    USHORT nResult = DET_INS_CONTINUE;
    while (nResult == DET_INS_CONTINUE && nMaxLevel < 1000)
    {
        aData.SetMaxLevel( nMaxLevel );
        nResult = InsertPredLevel( nCol, nRow, aData, 0 );
        ++nMaxLevel;
    }

    return ( nResult == DET_INS_INSERTED );
}

BOOL ScDetectiveFunc::ShowSucc( USHORT nCol, USHORT nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");

    ScDetectiveData aData( pModel );

    USHORT nMaxLevel = 0;
    USHORT nResult = DET_INS_CONTINUE;
    while (nResult == DET_INS_CONTINUE && nMaxLevel < 1000)
    {
        aData.SetMaxLevel( nMaxLevel );
        nResult = InsertSuccLevel( nCol, nRow, nCol, nRow, aData, 0 );
        ++nMaxLevel;
    }

    return ( nResult == DET_INS_INSERTED );
}

BOOL ScDetectiveFunc::ShowError( USHORT nCol, USHORT nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");

    ScTripel aPos( nCol, nRow, nTab );
    ScTripel aErrPos;
    if ( !HasError( aPos,aPos,aErrPos ) )
        return FALSE;

    ScDetectiveData aData( pModel );

    aData.SetMaxLevel( 1000 );
    USHORT nResult = InsertErrorLevel( nCol, nRow, aData, 0 );

    return ( nResult == DET_INS_INSERTED );
}

BOOL ScDetectiveFunc::DeleteSucc( USHORT nCol, USHORT nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");

    USHORT nLevelCount = FindSuccLevel( nCol, nRow, nCol, nRow, 0, 0 );
    if ( nLevelCount )
        FindSuccLevel( nCol, nRow, nCol, nRow, 0, nLevelCount );            // loeschen

    return ( nLevelCount != 0 );
}

BOOL ScDetectiveFunc::DeletePred( USHORT nCol, USHORT nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");

    USHORT nLevelCount = FindPredLevel( nCol, nRow, 0, 0 );
    if ( nLevelCount )
        FindPredLevel( nCol, nRow, 0, nLevelCount );            // loeschen

    return ( nLevelCount != 0 );
}

BOOL ScDetectiveFunc::DeleteAll( ScDetectiveDelete eWhat )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");

    pPage->RecalcObjOrdNums();

    long    nDelCount = 0;
    ULONG   nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        SdrObject** ppObj = new SdrObject*[nObjCount];

        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetLayer() == SC_LAYER_INTERN )
            {
                BOOL bDoThis = TRUE;
                if ( eWhat != SC_DET_ALL )
                {
                    BOOL bCircle = ( pObject->ISA(SdrCircObj) );
                    BOOL bCaption = ( pObject->ISA(SdrCaptionObj) );
                    if ( eWhat == SC_DET_DETECTIVE )        // Detektiv, aus Menue
                        bDoThis = !bCaption;                // auch Kreise
                    else if ( eWhat == SC_DET_CIRCLES )     // Kreise, wenn neue erzeugt werden
                        bDoThis = bCircle;
                    else if ( eWhat == SC_DET_COMMENTS )
                        bDoThis = bCaption;
                    else
                        DBG_ERROR("wat?");
                }
                if ( bDoThis )
                    ppObj[nDelCount++] = pObject;
            }

            pObject = aIter.Next();
        }

        long i;
        for (i=1; i<=nDelCount; i++)
            pModel->AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );

        for (i=1; i<=nDelCount; i++)
            pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );

        delete[] ppObj;
    }

    return ( nDelCount != 0 );
}

BOOL ScDetectiveFunc::MarkInvalid(BOOL& rOverflow)
{
    rOverflow = FALSE;
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    BOOL bDeleted = DeleteAll( SC_DET_CIRCLES );        // nur die Kreise

    ScDetectiveData aData( pModel );
    long nInsCount = 0;

    //  Stellen suchen, wo Gueltigkeit definiert ist

    ScDocAttrIterator aAttrIter( pDoc, nTab, 0,0,MAXCOL,MAXROW );
    USHORT nCol, nRow1, nRow2;
    const ScPatternAttr* pPattern = aAttrIter.GetNext( nCol, nRow1, nRow2 );
    while ( pPattern && nInsCount < SC_DET_MAXCIRCLE )
    {
        ULONG nIndex = ((const SfxUInt32Item&)pPattern->GetItem(ATTR_VALIDDATA)).GetValue();
        if (nIndex)
        {
            const ScValidationData* pData = pDoc->GetValidationEntry( nIndex );
            if ( pData )
            {
                //  Zellen in dem Bereich durchgehen

                BOOL bMarkEmpty = !pData->IsIgnoreBlank();
                USHORT nNextRow = nRow1;
                USHORT nRow;
                ScCellIterator aCellIter( pDoc, nCol,nRow1,nTab, nCol,nRow2,nTab );
                ScBaseCell* pCell = aCellIter.GetFirst();
                while ( pCell && nInsCount < SC_DET_MAXCIRCLE )
                {
                    USHORT nCellRow = aCellIter.GetRow();
                    if ( bMarkEmpty )
                        for ( nRow = nNextRow; nRow < nCellRow && nInsCount < SC_DET_MAXCIRCLE; nRow++ )
                        {
                            DrawCircle( nCol, nRow, aData );
                            ++nInsCount;
                        }
                    if ( !pData->IsDataValid( pCell, ScAddress( nCol, nCellRow, nTab ) ) )
                    {
                        DrawCircle( nCol, nCellRow, aData );
                        ++nInsCount;
                    }
                    nNextRow = nCellRow + 1;
                    pCell = aCellIter.GetNext();
                }
                if ( bMarkEmpty )
                    for ( nRow = nNextRow; nRow <= nRow2 && nInsCount < SC_DET_MAXCIRCLE; nRow++ )
                    {
                        DrawCircle( nCol, nRow, aData );
                        ++nInsCount;
                    }
            }
        }

        pPattern = aAttrIter.GetNext( nCol, nRow1, nRow2 );
    }

    if ( nInsCount >= SC_DET_MAXCIRCLE )
        rOverflow = TRUE;

    return ( bDeleted || nInsCount != 0 );
}

SdrObject* ScDetectiveFunc::ShowCommentUser( USHORT nCol, USHORT nRow, const String& rUserText,
                                            const Rectangle& rVisible, BOOL bLeft, BOOL bForce,
                                            SdrPage* pDestPage )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel && !pDestPage)
        return NULL;

    SdrObject* pObject = NULL;
    ScPostIt aNote;
    BOOL bFound = pDoc->GetNote( nCol, nRow, nTab, aNote );
    if ( bFound || bForce || rUserText.Len() )
    {
        SdrModel* pDestModel = pModel;
        if ( pDestPage )
            pDestModel = pDestPage->GetModel();
        ScCommentData aData( pDoc, pDestModel );    // richtigen Pool benutzen

        String aNoteText = aNote.GetText();     //! Autor etc. von der Notiz?

        String aDisplay;
        BOOL bHasUser = ( rUserText.Len() != 0 );
        if ( bHasUser )
        {
            aDisplay += rUserText;
            if ( aNoteText.Len() )
                aDisplay.AppendAscii( RTL_CONSTASCII_STRINGPARAM("\n--------\n") );
        }
        aDisplay += aNoteText;

        pObject = DrawCaption( nCol, nRow, aDisplay, aData, pDestPage, bHasUser, bLeft, rVisible );
    }

    return pObject;
}

SdrObject* ScDetectiveFunc::ShowComment( USHORT nCol, USHORT nRow, BOOL bForce, SdrPage* pDestPage )
{
    return ShowCommentUser( nCol, nRow, String(), Rectangle(0,0,0,0), FALSE, bForce, pDestPage );
}

BOOL ScDetectiveFunc::HideComment( USHORT nCol, USHORT nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;
    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");

    pPage->RecalcObjOrdNums();
    BOOL bDone = FALSE;

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bDone)
    {
        if ( pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA( SdrCaptionObj ) )
        {
            ScDrawObjData* pData = ScDrawLayer::GetObjData( pObject );
            if ( pData && nCol == pData->aStt.nCol && nRow == pData->aStt.nRow )
            {
                pModel->AddCalcUndo( new SdrUndoRemoveObj( *pObject ) );
                pPage->RemoveObject( pObject->GetOrdNum() );
                bDone = TRUE;
            }
        }

        pObject = aIter.Next();
    }

    return bDone;
}

void ScDetectiveFunc::UpdateAllComments()
{
    //  for all caption objects, update attributes and SpecialTextBoxShadow flag
    //  (on all tables - nTab is ignored!)

    //  no undo actions, this is refreshed after undo

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return;

    ScCommentData aData( pDoc, pModel );

    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT nObjTab=0; nObjTab<nTabCount; nObjTab++)
    {
        SdrPage* pPage = pModel->GetPage(nObjTab);
        DBG_ASSERT(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_FLAT );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA( SdrCaptionObj ) )
                {
                    SdrCaptionObj* pCaption = (SdrCaptionObj*)pObject;

                    SfxItemSet& rAttrSet = aData.GetCaptionSet();

//-/                    pCaption->SetAttributes( rAttrSet, FALSE );
                    pCaption->SetItemSetAndBroadcast(rAttrSet);

                    pCaption->SetSpecialTextBoxShadow();
                }

                pObject = aIter.Next();
            }
        }
    }
}


