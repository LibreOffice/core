/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: detfunc.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:22:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svtools/colorcfg.hxx>
#include <svx/eeitem.hxx>
#include <svx/outlobj.hxx>
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
#include <svx/outliner.hxx>
#include <svx/editobj.hxx>
#include <svx/sxcecitm.hxx>
#include <svtools/whiter.hxx>
#include <svx/writingmodeitem.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

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
#include "scmod.hxx"
#include "postit.hxx"

//------------------------------------------------------------------------

// #99319# line ends are now created with an empty name.
// The checkForUniqueItem method then finds a unique name for the item's value.
#define SC_LINEEND_NAME     EMPTY_STRING

//------------------------------------------------------------------------

enum DetInsertResult {              // Return-Werte beim Einfuegen in einen Level
            DET_INS_CONTINUE,
            DET_INS_INSERTED,
            DET_INS_EMPTY,
            DET_INS_CIRCULAR };


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
    void    UpdateCaptionSet(const SfxItemSet& UpdateSet);
};

//------------------------------------------------------------------------

ColorData ScDetectiveFunc::nArrowColor = 0;
ColorData ScDetectiveFunc::nErrorColor = 0;
ColorData ScDetectiveFunc::nCommentColor = 0;
BOOL ScDetectiveFunc::bColorsInitialized = FALSE;

//------------------------------------------------------------------------

BOOL lcl_HasThickLine( SdrObject& rObj )
{
    // thin lines get width 0 -> everything greater 0 is a thick line

    return ( ((const XLineWidthItem&)rObj.GetMergedItem(XATTR_LINEWIDTH)).GetValue() > 0 );
}

//------------------------------------------------------------------------

ScDetectiveData::ScDetectiveData( SdrModel* pModel ) :
    aBoxSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aArrowSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aToTabSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aFromTabSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aCircleSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END )
{
    nMaxLevel = 0;

    aBoxSet.Put( XLineColorItem( EMPTY_STRING, Color( ScDetectiveFunc::GetArrowColor() ) ) );
    aBoxSet.Put( XFillStyleItem( XFILL_NONE ) );

    //  #66479# Standard-Linienenden (wie aus XLineEndList::Create) selber zusammenbasteln,
    //  um von den konfigurierten Linienenden unabhaengig zu sein

    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(10.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(0.0, 30.0));
    aTriangle.append(basegfx::B2DPoint(20.0, 30.0));
    aTriangle.setClosed(true);

    basegfx::B2DPolygon aSquare;
    aSquare.append(basegfx::B2DPoint(0.0, 0.0));
    aSquare.append(basegfx::B2DPoint(10.0, 0.0));
    aSquare.append(basegfx::B2DPoint(10.0, 10.0));
    aSquare.append(basegfx::B2DPoint(0.0, 10.0));
    aSquare.setClosed(true);

    basegfx::B2DPolygon aCircle(basegfx::tools::createPolygonFromEllipse(basegfx::B2DPoint(0.0, 0.0), 100.0, 100.0));
    aCircle.setClosed(true);

    String aName = SC_LINEEND_NAME;

    aArrowSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aCircle) ) );
    aArrowSet.Put( XLineStartWidthItem( 200 ) );
    aArrowSet.Put( XLineStartCenterItem( TRUE ) );
    aArrowSet.Put( XLineEndItem( aName, basegfx::B2DPolyPolygon(aTriangle) ) );
    aArrowSet.Put( XLineEndWidthItem( 200 ) );
    aArrowSet.Put( XLineEndCenterItem( FALSE ) );

    aToTabSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aCircle) ) );
    aToTabSet.Put( XLineStartWidthItem( 200 ) );
    aToTabSet.Put( XLineStartCenterItem( TRUE ) );
    aToTabSet.Put( XLineEndItem( aName, basegfx::B2DPolyPolygon(aSquare) ) );
    aToTabSet.Put( XLineEndWidthItem( 300 ) );
    aToTabSet.Put( XLineEndCenterItem( FALSE ) );

    aFromTabSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aSquare) ) );
    aFromTabSet.Put( XLineStartWidthItem( 300 ) );
    aFromTabSet.Put( XLineStartCenterItem( TRUE ) );
    aFromTabSet.Put( XLineEndItem( aName, basegfx::B2DPolyPolygon(aTriangle) ) );
    aFromTabSet.Put( XLineEndWidthItem( 200 ) );
    aFromTabSet.Put( XLineEndCenterItem( FALSE ) );

    aCircleSet.Put( XLineColorItem( String(), Color( ScDetectiveFunc::GetErrorColor() ) ) );
    aCircleSet.Put( XFillStyleItem( XFILL_NONE ) );
    USHORT nWidth = 55;     // 54 = 1 Pixel
    aCircleSet.Put( XLineWidthItem( nWidth ) );
}

ScCommentData::ScCommentData( ScDocument* pDoc, SdrModel* pModel ) :
    aCaptionSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END,
                                        EE_ITEMS_START, EE_ITEMS_END, 0,0 )
{
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(10.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(0.0, 30.0));
    aTriangle.append(basegfx::B2DPoint(20.0, 30.0));
    aTriangle.setClosed(true);

    String aName = SC_LINEEND_NAME;

    aCaptionSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aTriangle) ) );
    aCaptionSet.Put( XLineStartWidthItem( 200 ) );
    aCaptionSet.Put( XLineStartCenterItem( FALSE ) );
    aCaptionSet.Put( XFillStyleItem( XFILL_SOLID ) );
    Color aYellow( ScDetectiveFunc::GetCommentColor() );
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

    // support the best position for the tail connector now that
    // that notes can be resized and repositioned.
    aCaptionSet.Put( SdrCaptionEscDirItem( SDRCAPT_ESCBESTFIT) );
}

void ScCommentData::UpdateCaptionSet( const SfxItemSet& rSet)
{
    SfxWhichIter aWhichIter(rSet);
    sal_uInt16 nWhich(aWhichIter.FirstWhich());
    const SfxPoolItem* pPoolItem = NULL;

    while(nWhich)
    {
        if(rSet.GetItemState(nWhich, FALSE, &pPoolItem) == SFX_ITEM_SET)
        {
            switch(nWhich)
            {
                case SDRATTR_SHADOW:
                    // use existing Caption default - appears that setting this
                    // to true screws up the tail appearance. See also comment
                    // for default setting above.
                    break;
                case SDRATTR_SHADOWXDIST:
                    // use existing Caption default - svx sets a value of 35
                    // but default 100 gives a better appearance.
                    break;
                case SDRATTR_SHADOWYDIST:
                    // use existing Caption default - svx sets a value of 35
                    // but default 100 gives a better appearance.
                    break;

                default:
            aCaptionSet.Put(*pPoolItem);
                    break;
           }
        }
        nWhich = aWhichIter.NextWhich();
    }
}

//------------------------------------------------------------------------

inline BOOL Intersect( SCCOL nStartCol1, SCROW nStartRow1, SCCOL nEndCol1, SCROW nEndRow1,
                        SCCOL nStartCol2, SCROW nStartRow2, SCCOL nEndCol2, SCROW nEndRow2 )
{
    return nEndCol1 >= nStartCol2 && nEndCol2 >= nStartCol1 &&
            nEndRow1 >= nStartRow2 && nEndRow2 >= nStartRow1;
}

BOOL ScDetectiveFunc::HasError( const ScRange& rRange, ScAddress& rErrPos )
{
    rErrPos = rRange.aStart;
    USHORT nError = 0;

    ScCellIterator aCellIter( pDoc, rRange);
    ScBaseCell* pCell = aCellIter.GetFirst();
    while (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            nError = ((ScFormulaCell*)pCell)->GetErrCode();
            if (nError)
                rErrPos.Set( aCellIter.GetCol(), aCellIter.GetRow(), aCellIter.GetTab() );
        }
        pCell = aCellIter.GetNext();
    }

    return (nError != 0);
}

Point ScDetectiveFunc::GetDrawPos( SCCOL nCol, SCROW nRow, BOOL bArrow )
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
    SCTAB nLocalTab = nTab;     // nicht ueber this

    for (SCCOL i=0; i<nCol; i++)
        aPos.X() += pDoc->GetColWidth( i,nLocalTab );
    aPos.Y() += pDoc->FastGetRowHeight( 0, nRow-1, nLocalTab );

    if (bArrow)
    {
        if (ValidCol(nCol))
            aPos.X() += pDoc->GetColWidth( nCol, nLocalTab ) / 4;
        if (ValidRow(nRow))
            aPos.Y() += pDoc->GetRowHeight( nRow, nLocalTab ) / 2;
    }

    aPos.X() = (long) ( aPos.X() * HMM_PER_TWIPS );
    aPos.Y() = (long) ( aPos.Y() * HMM_PER_TWIPS );

    BOOL bNegativePage = pDoc->IsNegativePage( nTab );
    if ( bNegativePage )
        aPos.X() = -aPos.X();

    return aPos;
}

BOOL lcl_IsOtherTab( const basegfx::B2DPolyPolygon& rPolyPolygon )
{
    //  test if rPolygon is the line end for "other table" (rectangle)
    if(1L == rPolyPolygon.count())
    {
        const basegfx::B2DPolygon aSubPoly(rPolyPolygon.getB2DPolygon(0L));

        // #i73305# circle consists of 4 segments, too, distinguishable from square by
        // the use of control points
        if(4L == aSubPoly.count() && aSubPoly.isClosed() && !aSubPoly.areControlPointsUsed())
        {
            return true;
        }
    }

    return false;
}

BOOL ScDetectiveFunc::HasArrow( const ScAddress& rStart,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab )
{
    BOOL bStartAlien = ( rStart.Tab() != nTab );
    BOOL bEndAlien   = ( nEndTab != nTab );

    if (bStartAlien && bEndAlien)
    {
        DBG_ERROR("bStartAlien && bEndAlien");
        return TRUE;
    }

    BOOL bNegativePage = pDoc->IsNegativePage( nTab );

    Rectangle aStartRect;
    Rectangle aEndRect;
    if (!bStartAlien)
    {
        Point aStartPos = GetDrawPos( rStart.Col(), rStart.Row(), FALSE );
        Size aStartSize = Size(
                            (long) ( pDoc->GetColWidth( rStart.Col(), nTab) * HMM_PER_TWIPS ),
                            (long) ( pDoc->GetRowHeight( rStart.Row(), nTab) * HMM_PER_TWIPS ) );
        if ( bNegativePage )
            aStartPos.X() -= aStartSize.Width();
        aStartRect = Rectangle( aStartPos, aStartSize );
    }
    if (!bEndAlien)
    {
        Point aEndPos = GetDrawPos( nEndCol, nEndRow, FALSE );
        Size aEndSize = Size(
                            (long) ( pDoc->GetColWidth( nEndCol, nTab) * HMM_PER_TWIPS ),
                            (long) ( pDoc->GetRowHeight( nEndRow, nTab) * HMM_PER_TWIPS ) );
        if ( bNegativePage )
            aEndPos.X() -= aEndSize.Width();
        aEndRect = Rectangle( aEndPos, aEndSize );
    }

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page ?");

    BOOL bFound = FALSE;
    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetLayer()==SC_LAYER_INTERN &&
                pObject->IsPolyObj() && pObject->GetPointCount()==2 )
        {
            const SfxItemSet& rSet = pObject->GetMergedItemSet();

            BOOL bObjStartAlien =
                lcl_IsOtherTab( ((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).GetLineStartValue() );
            BOOL bObjEndAlien =
                lcl_IsOtherTab( ((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).GetLineEndValue() );

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
        const SfxItemSet& rSet = pObject->GetMergedItemSet();

        BOOL bObjStartAlien =
            lcl_IsOtherTab( ((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).GetLineStartValue() );
        BOOL bObjEndAlien =
            lcl_IsOtherTab( ((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).GetLineEndValue() );

        return !bObjStartAlien && !bObjEndAlien;
    }

    return FALSE;
}

//------------------------------------------------------------------------

//  InsertXXX: called from DrawEntry/DrawAlienEntry and InsertObject

BOOL ScDetectiveFunc::InsertArrow( SCCOL nCol, SCROW nRow,
                                SCCOL nRefStartCol, SCROW nRefStartRow,
                                SCCOL nRefEndCol, SCROW nRefEndRow,
                                BOOL bFromOtherTab, BOOL bRed,
                                ScDetectiveData& rData )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));

    BOOL bArea = ( nRefStartCol != nRefEndCol || nRefStartRow != nRefEndRow );
    if (bArea && !bFromOtherTab)
    {
        // insert the rectangle before the arrow - this is relied on in FindFrameForObject

        Point aStartCorner = GetDrawPos( nRefStartCol, nRefStartRow, FALSE );
        Point aEndCorner = GetDrawPos( nRefEndCol+1, nRefEndRow+1, FALSE );

        SdrRectObj* pBox = new SdrRectObj(Rectangle(aStartCorner,aEndCorner));

        pBox->SetMergedItemSetAndBroadcast(rData.GetBoxSet());

        ScDrawLayer::SetAnchor( pBox, SCA_CELL );
        pBox->SetLayer( SC_LAYER_INTERN );
        pPage->InsertObject( pBox );
        pModel->AddCalcUndo( new SdrUndoInsertObj( *pBox ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pBox, TRUE );
        pData->aStt.Set( nRefStartCol, nRefStartRow, nTab);
        pData->aEnd.Set( nRefEndCol, nRefEndRow, nTab);
        pData->bValidStart = TRUE;
        pData->bValidEnd = TRUE;
    }

    Point aStartPos = GetDrawPos( nRefStartCol, nRefStartRow, TRUE );
    Point aEndPos   = GetDrawPos( nCol, nRow, TRUE );

    if (bFromOtherTab)
    {
        BOOL bNegativePage = pDoc->IsNegativePage( nTab );
        long nPageSign = bNegativePage ? -1 : 1;

        aStartPos = Point( aEndPos.X() - 1000 * nPageSign, aEndPos.Y() - 1000 );
        if (aStartPos.X() * nPageSign < 0)
            aStartPos.X() += 2000 * nPageSign;
        if (aStartPos.Y() < 0)
            aStartPos.Y() += 2000;
    }

    SfxItemSet& rAttrSet = bFromOtherTab ? rData.GetFromTabSet() : rData.GetArrowSet();

    if (bArea && !bFromOtherTab)
        rAttrSet.Put( XLineWidthItem( 50 ) );               // Bereich
    else
        rAttrSet.Put( XLineWidthItem( 0 ) );                // einzelne Referenz

    ColorData nColorData = ( bRed ? GetErrorColor() : GetArrowColor() );
    rAttrSet.Put( XLineColorItem( String(), Color( nColorData ) ) );

    basegfx::B2DPolygon aTempPoly;
    aTempPoly.append(basegfx::B2DPoint(aStartPos.X(), aStartPos.Y()));
    aTempPoly.append(basegfx::B2DPoint(aEndPos.X(), aEndPos.Y()));
    SdrPathObj* pArrow = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aTempPoly));
    pArrow->NbcSetLogicRect(Rectangle(aStartPos,aEndPos));  //! noetig ???
    pArrow->SetMergedItemSetAndBroadcast(rAttrSet);

    ScDrawLayer::SetAnchor( pArrow, SCA_CELL );
    pArrow->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pArrow );
    pModel->AddCalcUndo( new SdrUndoInsertObj( *pArrow ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pArrow, TRUE );
    if (bFromOtherTab)
        pData->bValidStart = FALSE;
    else
    {
        pData->aStt.Set( nRefStartCol, nRefStartRow, nTab);
        pData->bValidStart = TRUE;
    }

    pData->aEnd.Set( nCol, nRow, nTab);
    pData->bValidEnd = TRUE;

    return TRUE;
}

BOOL ScDetectiveFunc::InsertToOtherTab( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, BOOL bRed,
                                ScDetectiveData& rData )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));

    BOOL bArea = ( nStartCol != nEndCol || nStartRow != nEndRow );
    if (bArea)
    {
        Point aStartCorner = GetDrawPos( nStartCol, nStartRow, FALSE );
        Point aEndCorner = GetDrawPos( nEndCol+1, nEndRow+1, FALSE );

        SdrRectObj* pBox = new SdrRectObj(Rectangle(aStartCorner,aEndCorner));

        pBox->SetMergedItemSetAndBroadcast(rData.GetBoxSet());

        ScDrawLayer::SetAnchor( pBox, SCA_CELL );
        pBox->SetLayer( SC_LAYER_INTERN );
        pPage->InsertObject( pBox );
        pModel->AddCalcUndo( new SdrUndoInsertObj( *pBox ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pBox, TRUE );
        pData->aStt.Set( nStartCol, nStartRow, nTab);
        pData->aEnd.Set( nEndCol, nEndRow, nTab);
        pData->bValidStart = TRUE;
        pData->bValidEnd = TRUE;
    }

    BOOL bNegativePage = pDoc->IsNegativePage( nTab );
    long nPageSign = bNegativePage ? -1 : 1;

    Point aStartPos = GetDrawPos( nStartCol, nStartRow, TRUE );
    Point aEndPos   = Point( aStartPos.X() + 1000 * nPageSign, aStartPos.Y() - 1000 );
    if (aEndPos.Y() < 0)
        aEndPos.Y() += 2000;

    SfxItemSet& rAttrSet = rData.GetToTabSet();
    if (bArea)
        rAttrSet.Put( XLineWidthItem( 50 ) );               // Bereich
    else
        rAttrSet.Put( XLineWidthItem( 0 ) );                // einzelne Referenz

    ColorData nColorData = ( bRed ? GetErrorColor() : GetArrowColor() );
    rAttrSet.Put( XLineColorItem( String(), Color( nColorData ) ) );

    basegfx::B2DPolygon aTempPoly;
    aTempPoly.append(basegfx::B2DPoint(aStartPos.X(), aStartPos.Y()));
    aTempPoly.append(basegfx::B2DPoint(aEndPos.X(), aEndPos.Y()));
    SdrPathObj* pArrow = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aTempPoly));
    pArrow->NbcSetLogicRect(Rectangle(aStartPos,aEndPos));  //! noetig ???

    pArrow->SetMergedItemSetAndBroadcast(rAttrSet);

    ScDrawLayer::SetAnchor( pArrow, SCA_CELL );
    pArrow->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pArrow );
    pModel->AddCalcUndo( new SdrUndoInsertObj( *pArrow ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pArrow, TRUE );
    pData->aStt.Set( nStartCol, nStartRow, nTab);
    pData->bValidStart = TRUE;
    pData->bValidEnd = FALSE;

    return TRUE;
}

//------------------------------------------------------------------------

//  DrawEntry:      Formel auf dieser Tabelle,
//                  Referenz auf dieser oder anderer
//  DrawAlienEntry: Formel auf anderer Tabelle,
//                  Referenz auf dieser

//      return FALSE: da war schon ein Pfeil

BOOL ScDetectiveFunc::DrawEntry( SCCOL nCol, SCROW nRow,
                                    const ScRange& rRef,
                                    ScDetectiveData& rData )
{
    if ( HasArrow( rRef.aStart, nCol, nRow, nTab ) )
        return FALSE;

    ScAddress aErrorPos;
    BOOL bError = HasError( rRef, aErrorPos );
    BOOL bAlien = ( rRef.aEnd.Tab() < nTab || rRef.aStart.Tab() > nTab );

    return InsertArrow( nCol, nRow,
                        rRef.aStart.Col(), rRef.aStart.Row(),
                        rRef.aEnd.Col(), rRef.aEnd.Row(),
                        bAlien, bError, rData );
}

BOOL ScDetectiveFunc::DrawAlienEntry( const ScRange& rRef,
                                        ScDetectiveData& rData )
{
    if ( HasArrow( rRef.aStart, 0, 0, nTab+1 ) )
        return FALSE;

    ScAddress aErrorPos;
    BOOL bError = HasError( rRef, aErrorPos );

    return InsertToOtherTab( rRef.aStart.Col(), rRef.aStart.Row(),
                                rRef.aEnd.Col(), rRef.aEnd.Row(),
                                bError, rData );
}

void ScDetectiveFunc::DrawCircle( SCCOL nCol, SCROW nRow, ScDetectiveData& rData )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));

    Point aStartPos = GetDrawPos( nCol, nRow, FALSE );
    Size aSize( (long) ( pDoc->GetColWidth(nCol, nTab) * HMM_PER_TWIPS ),
                (long) ( pDoc->GetRowHeight(nRow, nTab) * HMM_PER_TWIPS ) );

    BOOL bNegativePage = pDoc->IsNegativePage( nTab );
    if ( bNegativePage )
        aStartPos.X() -= aSize.Width();

    Rectangle aRect( aStartPos, aSize );
    aRect.Left()    -= 250;
    aRect.Right()   += 250;
    aRect.Top()     -= 70;
    aRect.Bottom()  += 70;

    SdrCircObj* pCircle = new SdrCircObj( OBJ_CIRC, aRect );
    SfxItemSet& rAttrSet = rData.GetCircleSet();

    pCircle->SetMergedItemSetAndBroadcast(rAttrSet);

    ScDrawLayer::SetAnchor( pCircle, SCA_CELL );
    pCircle->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pCircle );
    pModel->AddCalcUndo( new SdrUndoInsertObj( *pCircle ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pCircle, TRUE );
    pData->aStt.Set( nCol, nRow, nTab);
    pData->bValidStart = TRUE;
    pData->bValidEnd = FALSE;
}

BOOL lcl_MirrorCheckNoteRectangle(Rectangle& rRect, BOOL bNegativePage)
{
    BOOL bMirrorChange = false;

    if ( bNegativePage )
    {
        if(rRect.Left() >= 0 && rRect.Right() > 0)
            bMirrorChange = true;
    }
    else
    {
        if(rRect.Left() < 0 && rRect.Right() <= 0)
            bMirrorChange = true;
    }

    if(bMirrorChange)
    {
        long nTemp = rRect.Left();
        rRect.Left() = -rRect.Right();
        rRect.Right() = -nTemp;
    }
    return bMirrorChange;
}

SdrObject* ScDetectiveFunc::DrawCaption( SCCOL nCol, SCROW nRow, const String& rText,
                                            ScCommentData& rData, SdrPage* pDestPage,
                                            BOOL bHasUserText, BOOL bLeft,
                                            const Rectangle& rVisible )
{
    ScDrawLayer* pModel = NULL;     // muss ScDrawLayer* sein wegen AddCalcUndo !!!
    SdrPage* pPage = pDestPage;
    if (!pPage)                     // keine angegeben?
    {
        pModel = pDoc->GetDrawLayer();
        pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    }

    BOOL bNegativePage = pDoc->IsNegativePage( nTab );
    long nPageSign = bNegativePage ? -1 : 1;

    SCCOL nNextCol = nCol+1;
    const ScMergeAttr* pMerge = (const ScMergeAttr*) pDoc->GetAttr( nCol,nRow,nTab, ATTR_MERGE );
    if ( pMerge->GetColMerge() > 1 )
        nNextCol = nCol + pMerge->GetColMerge();

    Point aTailPos = GetDrawPos( nNextCol, nRow, FALSE );
    Point aRectPos = aTailPos;
    if ( bLeft )
    {
        aTailPos = GetDrawPos( nCol, nRow, FALSE );
        aTailPos.X() += 10 * nPageSign;             // left, just inside the cell
    }
    else
        aTailPos.X() -= 10 * nPageSign;             // point just before the next cell

    //  arrow head should be visible (if visible rectangle is set)
    if ( bNegativePage )
    {
        if ( aTailPos.X() < rVisible.Left() && rVisible.Left() )
            aTailPos.X() = rVisible.Left();
    }
    else
    {
        if ( aTailPos.X() > rVisible.Right() && rVisible.Right() )
            aTailPos.X() = rVisible.Right();
    }

    aRectPos.X() += 600 * nPageSign;
    aRectPos.Y() -= 1500;
    if ( aRectPos.Y() < rVisible.Top() ) aRectPos.Y() = rVisible.Top();

    //  links wird spaeter getestet

    //  bei Textlaenge > SC_NOTE_SMALLTEXT wird die Breite verdoppelt...
    long nDefWidth = ( rText.Len() > SC_NOTE_SMALLTEXT ) ? 5800 : 2900;
    Size aRectSize( nDefWidth, 1800 );

    long nMaxWidth = 10000;             //! oder wie?
    if ( !bHasUserText )
        nMaxWidth = aRectSize.Width();  // Notiz nicht zu gross

    if ( bNegativePage )
    {
        if ( rVisible.Left() )
        {
            nMaxWidth = aRectPos.X() - rVisible.Left() - 100;
            if (nMaxWidth < nDefWidth)
            {
                aRectPos.X() += nDefWidth - nMaxWidth;
                nMaxWidth = nDefWidth;
            }
        }
        if ( aRectPos.X() > rVisible.Right() )
            aRectPos.X() = rVisible.Right();

        aRectPos.X() -= aRectSize.Width();
    }
    else
    {
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
    }

    bool bNewNote = true;
    Rectangle aTextRect;
    ScPostIt aCellNote(pDoc);
    if(pDoc->GetNote( nCol, nRow, nTab, aCellNote ))
    {
        aTextRect = aCellNote.GetRectangle();
        if(lcl_MirrorCheckNoteRectangle(aTextRect,bNegativePage))
        {
            aCellNote.SetRectangle(aTextRect);
            pDoc->SetNote( nCol, nRow, nTab, aCellNote );
        }
        bNewNote = false;
    }
    SdrCaptionObj* pCaption;
    //if no rectangle dimensions stored then default to our calculated dimensions.
    if(aTextRect.IsEmpty())
    {
        pCaption = new SdrCaptionObj( Rectangle( aRectPos,aRectSize ), aTailPos );
        aTextRect = pCaption->GetLogicRect();
        aCellNote.SetRectangle(aTextRect);
        pDoc->SetNote( nCol, nRow, nTab, aCellNote );
    }
    else
        pCaption = new SdrCaptionObj( aTextRect, aTailPos );

    if(!bNewNote)
    {
        rData.UpdateCaptionSet(aCellNote.GetItemSet());
    }
    SfxItemSet& rAttrSet = rData.GetCaptionSet();


    if (bHasUserText)
    {
        rAttrSet.Put(SdrTextAutoGrowWidthItem(TRUE));
        rAttrSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT));
        rAttrSet.Put(SdrTextMaxFrameWidthItem(nMaxWidth));
    }

    ScDrawLayer::SetAnchor( pCaption, SCA_PAGE );
    pCaption->SetLayer( SC_LAYER_INTERN );
    pCaption->SetSpecialTextBoxShadow();
    pCaption->SetFixedTail();


    if(bHasUserText)
    {
        pPage->InsertObject( pCaption );
        // #78611# for SetText, the object must already be inserted
        pCaption->SetText( rText );
        //  SetAttributes must be after SetText, because the font attributes
        //  are applied to the text.
        pCaption->SetMergedItemSetAndBroadcast(rAttrSet);
    }
    else
    {
        pPage->InsertObject( pCaption );

        // To support different paragraph alignments using the
        // ScNoteEditEngine(), it is necessary to apply the
        // ItemSet of the container before the creation of the
        // EditTextObject(). But to support Vertical text, the opposite
        // is true.
        BOOL bVertical = static_cast<const SvxWritingModeItem&> (rAttrSet.Get (SDRATTR_TEXTDIRECTION)).GetValue() == com::sun::star::text::WritingMode_TB_RL;
        if(!bVertical)
            pCaption->SetMergedItemSetAndBroadcast(rAttrSet);

        // Keep the existing rectangle size.
        if(!bNewNote)
            pCaption->SetLogicRect(aTextRect);

        ScPostIt aNote(pDoc);
        if(pDoc->GetNote( nCol, nRow, nTab, aNote ))
        {
            if(const EditTextObject* pEditText = aNote.GetEditTextObject())
            {
                OutlinerParaObject* pOPO = new OutlinerParaObject( *pEditText );
                pOPO->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );
                pCaption->NbcSetOutlinerParaObject( pOPO );
            }
        }
        if(bVertical)
            pCaption->SetMergedItemSetAndBroadcast(rAttrSet);
     }

    if (bHasUserText)
    {
        pCaption->AdjustTextFrameWidthAndHeight( aTextRect, TRUE, TRUE );
        aTextRect = pCaption->GetLogicRect();
    }

    aCellNote.SetRectangle(aTextRect);
    pDoc->SetNote( nCol, nRow, nTab, aCellNote );

    //  Undo und UserData nur, wenn's im Dokument ist, also keine Page angegeben war
    if ( !pDestPage )
    {
        pModel->AddCalcUndo( new SdrUndoInsertObj( *pCaption ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pCaption, TRUE );
        pData->aStt.Set( nCol, nRow, nTab);
        pData->bValidStart = TRUE;
        pData->bValidEnd = FALSE;
    }

    return pCaption;
}

void ScDetectiveFunc::DeleteArrowsAt( SCCOL nCol, SCROW nRow, BOOL bDestPnt )
{
    BOOL bNegativePage = pDoc->IsNegativePage( nTab );

    Point aPos = GetDrawPos( nCol, nRow, FALSE );
    Size aSize = Size(  (long) ( pDoc->GetColWidth( nCol, nTab) * HMM_PER_TWIPS ),
                        (long) ( pDoc->GetRowHeight( nRow, nTab) * HMM_PER_TWIPS ) );
    if ( bNegativePage )
        aPos.X() -= aSize.Width();
    Rectangle aRect( aPos, aSize );

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
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

void ScDetectiveFunc::DeleteBox( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
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
    Rectangle aCornerRect( aStartCorner, aEndCorner );
    aCornerRect.Justify();
    aStartCorner = aCornerRect.TopLeft();
    aEndCorner = aCornerRect.BottomRight();
    Rectangle aObjRect;

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
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
                aObjRect.Justify();
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

USHORT ScDetectiveFunc::InsertPredLevelArea( const ScRange& rRef,
                                        ScDetectiveData& rData, USHORT nLevel )
{
    USHORT nResult = DET_INS_EMPTY;

    ScCellIterator aCellIter( pDoc, rRef);
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

USHORT ScDetectiveFunc::InsertPredLevel( SCCOL nCol, SCROW nRow, ScDetectiveData& rData,
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
    ScRange aRef;
    while ( aIter.GetNextRef( aRef ) )
    {
        if (DrawEntry( nCol, nRow, aRef, rData ))
        {
            nResult = DET_INS_INSERTED;         //  neuer Pfeil eingetragen
        }
        else
        {
            //  weiterverfolgen

            if ( nLevel < rData.GetMaxLevel() )
            {
                USHORT nSubResult;
                BOOL bArea = (aRef.aStart != aRef.aEnd);
                if (bArea)
                    nSubResult = InsertPredLevelArea( aRef, rData, nLevel+1 );
                else
                    nSubResult = InsertPredLevel( aRef.aStart.Col(), aRef.aStart.Row(),
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

USHORT ScDetectiveFunc::FindPredLevelArea( const ScRange& rRef,
                                                USHORT nLevel, USHORT nDeleteLevel )
{
    USHORT nResult = nLevel;

    ScCellIterator aCellIter( pDoc, rRef);
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

USHORT ScDetectiveFunc::FindPredLevel( SCCOL nCol, SCROW nRow, USHORT nLevel, USHORT nDeleteLevel )
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
    ScRange aRef;
    while ( aIter.GetNextRef( aRef) )
    {
        BOOL bArea = ( aRef.aStart != aRef.aEnd );

        if ( bDelete )                  // Rahmen loeschen ?
        {
            if (bArea)
            {
                DeleteBox( aRef.aStart.Col(), aRef.aStart.Row(), aRef.aEnd.Col(), aRef.aEnd.Row() );
            }
        }
        else                            // weitersuchen
        {
            if ( HasArrow( aRef.aStart, nCol,nRow,nTab ) )
            {
                USHORT nTemp;
                if (bArea)
                    nTemp = FindPredLevelArea( aRef, nLevel+1, nDeleteLevel );
                else
                    nTemp = FindPredLevel( aRef.aStart.Col(),aRef.aStart.Row(),
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

USHORT ScDetectiveFunc::InsertErrorLevel( SCCOL nCol, SCROW nRow, ScDetectiveData& rData,
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
    ScRange aRef;
    ScAddress aErrorPos;
    BOOL bHasError = FALSE;
    while ( aIter.GetNextRef( aRef ) )
    {
        if (HasError( aRef, aErrorPos ))
        {
            bHasError = TRUE;
            if (DrawEntry( nCol, nRow, ScRange( aErrorPos), rData ))
                nResult = DET_INS_INSERTED;

            //  und weiterverfolgen

            if ( nLevel < rData.GetMaxLevel() )         // praktisch immer
            {
                if (InsertErrorLevel( aErrorPos.Col(), aErrorPos.Row(),
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

USHORT ScDetectiveFunc::InsertSuccLevel( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
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
            ScRange aRef;
            while ( aIter.GetNextRef( aRef) )
            {
                if (aRef.aStart.Tab() <= nTab && aRef.aEnd.Tab() >= nTab)
                {
                    if (Intersect( nCol1,nRow1,nCol2,nRow2,
                            aRef.aStart.Col(),aRef.aStart.Row(),
                            aRef.aEnd.Col(),aRef.aEnd.Row() ))
                    {
                        BOOL bAlien = ( aCellIter.GetTab() != nTab );
                        BOOL bDrawRet;
                        if (bAlien)
                            bDrawRet = DrawAlienEntry( aRef, rData );
                        else
                            bDrawRet = DrawEntry( aCellIter.GetCol(), aCellIter.GetRow(),
                                                    aRef, rData );
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

USHORT ScDetectiveFunc::FindSuccLevel( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
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
            ScRange aRef;
            while ( aIter.GetNextRef( aRef) )
            {
                if (aRef.aStart.Tab() <= nTab && aRef.aEnd.Tab() >= nTab)
                {
                    if (Intersect( nCol1,nRow1,nCol2,nRow2,
                            aRef.aStart.Col(),aRef.aStart.Row(),
                            aRef.aEnd.Col(),aRef.aEnd.Row() ))
                    {
                        if ( bDelete )                          // Pfeile, die hier anfangen
                        {
                            if (aRef.aStart != aRef.aEnd)
                            {
                                DeleteBox( aRef.aStart.Col(), aRef.aStart.Row(),
                                                aRef.aEnd.Col(), aRef.aEnd.Row() );
                            }
                            DeleteArrowsAt( aRef.aStart.Col(), aRef.aStart.Row(), FALSE );
                        }
                        else if ( !bRunning &&
                                HasArrow( aRef.aStart,
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

BOOL ScDetectiveFunc::ShowPred( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

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

BOOL ScDetectiveFunc::ShowSucc( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

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

BOOL ScDetectiveFunc::ShowError( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    ScRange aRange( nCol, nRow, nTab );
    ScAddress aErrPos;
    if ( !HasError( aRange,aErrPos ) )
        return FALSE;

    ScDetectiveData aData( pModel );

    aData.SetMaxLevel( 1000 );
    USHORT nResult = InsertErrorLevel( nCol, nRow, aData, 0 );

    return ( nResult == DET_INS_INSERTED );
}

BOOL ScDetectiveFunc::DeleteSucc( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

    USHORT nLevelCount = FindSuccLevel( nCol, nRow, nCol, nRow, 0, 0 );
    if ( nLevelCount )
        FindSuccLevel( nCol, nRow, nCol, nRow, 0, nLevelCount );            // loeschen

    return ( nLevelCount != 0 );
}

BOOL ScDetectiveFunc::DeletePred( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;

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

    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
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
                    else if ( eWhat == SC_DET_ARROWS )      // DetectiveRefresh
                        bDoThis = !bCaption && !bCircle;    // don't include circles
                    else
                    {
                        DBG_ERROR("wat?");
                    }
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
    SCCOL nCol;
    SCROW nRow1;
    SCROW nRow2;
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
                SCROW nNextRow = nRow1;
                SCROW nRow;
                ScCellIterator aCellIter( pDoc, nCol,nRow1,nTab, nCol,nRow2,nTab );
                ScBaseCell* pCell = aCellIter.GetFirst();
                while ( pCell && nInsCount < SC_DET_MAXCIRCLE )
                {
                    SCROW nCellRow = aCellIter.GetRow();
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

SdrObject* ScDetectiveFunc::ShowCommentUser( SCCOL nCol, SCROW nRow, const String& rUserText,
                                            const Rectangle& rVisible, BOOL bLeft, BOOL bForce,
                                            SdrPage* pDestPage )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel && !pDestPage)
        return NULL;

    SdrObject* pObject = NULL;
    ScPostIt aNote(pDoc);
    BOOL bFound = pDoc->GetNote( nCol, nRow, nTab, aNote );
    if ( bFound || bForce || rUserText.Len() )
    {
        SdrModel* pDestModel = pModel;
        if ( pDestPage )
            pDestModel = pDestPage->GetModel();
        ScCommentData aData( pDoc, pDestModel );    // richtigen Pool benutzen

        String aNoteText = aNote.GetText();     //! Author etc. of this Note?

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

SdrObject* ScDetectiveFunc::ShowComment( SCCOL nCol, SCROW nRow, BOOL bForce, SdrPage* pDestPage )
{
    return ShowCommentUser( nCol, nRow, String(), Rectangle(0,0,0,0), FALSE, bForce, pDestPage );
}

BOOL ScDetectiveFunc::HideComment( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return FALSE;
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
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
            if ( pData && nCol == pData->aStt.Col() && nRow == pData->aStt.Row() )
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

    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nObjTab=0; nObjTab<nTabCount; nObjTab++)
    {
        SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nObjTab));
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

                    ScDrawObjData* pData = ScDrawLayer::GetObjData( pCaption, TRUE );

                    ScPostIt aCellNote(pDoc);
                    if(pDoc->GetNote( pData->aStt.Col(), pData->aStt.Row(), nObjTab, aCellNote ))
                    {
                        ScCommentData aData( pDoc, pModel );
                        SfxItemSet rAttrColorSet(aCellNote.GetItemSet());
                        Color aCommentColor( ScDetectiveFunc::GetCommentColor() );
                        rAttrColorSet.Put( XFillColorItem( String(), aCommentColor ) );
                        aData.UpdateCaptionSet(rAttrColorSet);
                        SfxItemSet& rAttrSet = aData.GetCaptionSet();
                        pCaption->SetMergedItemSetAndBroadcast(rAttrSet);
                        pCaption->SetSpecialTextBoxShadow();
                        pCaption->SetFixedTail();
                        aCellNote.SetItemSet(rAttrSet);
                        pDoc->SetNote( pData->aStt.Col(), pData->aStt.Row(), nObjTab, aCellNote );
                    }
                }

                pObject = aIter.Next();
            }
        }
    }
}

void ScDetectiveFunc::UpdateAllArrowColors()
{
    //  no undo actions necessary

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return;

    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nObjTab=0; nObjTab<nTabCount; nObjTab++)
    {
        SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nObjTab));
        DBG_ASSERT(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_FLAT );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetLayer() == SC_LAYER_INTERN )
                {
                    BOOL bArrow = FALSE;
                    BOOL bError = FALSE;

                    ScAddress aPos;
                    ScRange aSource;
                    BOOL bDummy;
                    ScDetectiveObjType eType = GetDetectiveObjectType( pObject, nObjTab, aPos, aSource, bDummy );
                    if ( eType == SC_DETOBJ_ARROW || eType == SC_DETOBJ_TOOTHERTAB )
                    {
                        //  source is valid, determine error flag from source range

                        ScAddress aErrPos;
                        if ( HasError( aSource, aErrPos ) )
                            bError = TRUE;
                        else
                            bArrow = TRUE;
                    }
                    else if ( eType == SC_DETOBJ_FROMOTHERTAB )
                    {
                        //  source range is no longer known, take error flag from formula itself
                        //  (this means, if the formula has an error, all references to other tables
                        //  are marked red)

                        ScAddress aErrPos;
                        if ( HasError( ScRange( aPos), aErrPos ) )
                            bError = TRUE;
                        else
                            bArrow = TRUE;
                    }
                    else if ( eType == SC_DETOBJ_CIRCLE )
                    {
                        //  circles (error marks) are always red

                        bError = TRUE;
                    }
                    else if ( eType == SC_DETOBJ_NONE )
                    {
                        //  frame for area reference has no ObjType, always gets arrow color

                        if ( pObject->ISA( SdrRectObj ) && !pObject->ISA( SdrCaptionObj ) )
                        {
                            bArrow = TRUE;
                        }
                    }

                    if ( bArrow || bError )
                    {
                        ColorData nColorData = ( bError ? GetErrorColor() : GetArrowColor() );
                        //pObject->SendRepaintBroadcast(pObject->GetBoundRect());
                        pObject->SetMergedItem( XLineColorItem( String(), Color( nColorData ) ) );

                        // repaint only
                        pObject->ActionChanged();
                        // pObject->SendRepaintBroadcast(pObject->GetBoundRect());
                    }
                }

                pObject = aIter.Next();
            }
        }
    }
}

BOOL ScDetectiveFunc::FindFrameForObject( SdrObject* pObject, ScRange& rRange )
{
    //  find the rectangle for an arrow (always the object directly before the arrow)
    //  rRange must be initialized to the source cell of the arrow (start of area)

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel) return FALSE;

    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage) return FALSE;

    // test if the object is a direct page member
    if(pObject
        && pObject->GetPage()
        && pObject->GetObjList()
        && pObject->GetPage() == pObject->GetObjList())
    {
        // Is there a previous object?
        const sal_uInt32 nOrdNum(pObject->GetOrdNum());

        if(nOrdNum > 0L)
        {
            SdrObject* pPrevObj = pPage->GetObj(nOrdNum - 1L);

            if ( pPrevObj && pPrevObj->GetLayer() == SC_LAYER_INTERN && pPrevObj->ISA(SdrRectObj) )
            {
                ScDrawObjData* pPrevData = ScDrawLayer::GetObjDataTab( pPrevObj, rRange.aStart.Tab() );
                if ( pPrevData && pPrevData->bValidStart && pPrevData->bValidEnd )
                {
                    if ( pPrevData->aStt == rRange.aStart )
                    {
                        rRange.aEnd = pPrevData->aEnd;
                        return TRUE;
                    }
                }
            }
        }
    }

    // GetContainer() no longer allowed, baaad style (!)
    //ULONG nPos = pPage->GetContainer().GetPos( pObject );
    //if ( nPos != CONTAINER_ENTRY_NOTFOUND && nPos > 0 )
    //{
    //  SdrObject* pPrevObj = pPage->GetObj( nPos - 1 );
    //  if ( pPrevObj && pPrevObj->GetLayer() == SC_LAYER_INTERN && pPrevObj->ISA(SdrRectObj) )
    //  {
    //      ScDrawObjData* pPrevData = ScDrawLayer::GetObjDataTab( pPrevObj, rRange.aStart.Tab() );
    //      if ( pPrevData && pPrevData->bValidStart && pPrevData->bValidEnd )
    //      {
    //          if ( pPrevData->aStt.nCol == rRange.aStart.Col() &&
    //               pPrevData->aStt.nRow == rRange.aStart.Row() &&
    //               pPrevData->aStt.nTab == rRange.aStart.Tab() )
    //          {
    //              rRange.aEnd.Set( pPrevData->aEnd.nCol,
    //                               pPrevData->aEnd.nRow,
    //                               pPrevData->aEnd.nTab );
    //              return TRUE;
    //          }
    //      }
    //  }
    //}

    return FALSE;
}

ScDetectiveObjType ScDetectiveFunc::GetDetectiveObjectType( SdrObject* pObject, SCTAB nObjTab,
                                ScAddress& rPosition, ScRange& rSource, BOOL& rRedLine )
{
    rRedLine = FALSE;
    ScDetectiveObjType eType = SC_DETOBJ_NONE;

    if ( pObject && pObject->GetLayer() == SC_LAYER_INTERN )
    {
        ScDrawObjData* pData = ScDrawLayer::GetObjDataTab( pObject, nObjTab );
        if ( pObject->IsPolyObj() && pObject->GetPointCount() == 2 )
        {
            // line object -> arrow

            if ( pData->bValidStart )
                eType = ( pData->bValidEnd ) ? SC_DETOBJ_ARROW : SC_DETOBJ_TOOTHERTAB;
            else if ( pData->bValidEnd )
                eType = SC_DETOBJ_FROMOTHERTAB;

            if ( pData->bValidStart )
                rSource = pData->aStt;
            if ( pData->bValidEnd )
                rPosition = pData->aEnd;

            if ( pData->bValidStart && lcl_HasThickLine( *pObject ) )
            {
                // thick line -> look for frame before this object

                FindFrameForObject( pObject, rSource );     // modifies rSource
            }

            ColorData nObjColor = ((const XLineColorItem&)pObject->GetMergedItem(XATTR_LINECOLOR)).GetColorValue().GetColor();
            if ( nObjColor == GetErrorColor() && nObjColor != GetArrowColor() )
                rRedLine = TRUE;
        }
        else if ( pObject->ISA(SdrCircObj) )
        {
            if ( pData->bValidStart )
            {
                // cell position is returned in rPosition

                rPosition = pData->aStt;
                eType = SC_DETOBJ_CIRCLE;
            }
        }
    }

    return eType;
}

void ScDetectiveFunc::InsertObject( ScDetectiveObjType eType,
                            const ScAddress& rPosition, const ScRange& rSource,
                            BOOL bRedLine )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel) return;
    ScDetectiveData aData( pModel );

    switch (eType)
    {
        case SC_DETOBJ_ARROW:
        case SC_DETOBJ_FROMOTHERTAB:
            InsertArrow( rPosition.Col(), rPosition.Row(),
                         rSource.aStart.Col(), rSource.aStart.Row(),
                         rSource.aEnd.Col(), rSource.aEnd.Row(),
                         (eType == SC_DETOBJ_FROMOTHERTAB), bRedLine, aData );
            break;
        case SC_DETOBJ_TOOTHERTAB:
            InsertToOtherTab( rSource.aStart.Col(), rSource.aStart.Row(),
                              rSource.aEnd.Col(), rSource.aEnd.Row(),
                              bRedLine, aData );
            break;
        case SC_DETOBJ_CIRCLE:
            DrawCircle( rPosition.Col(), rPosition.Row(), aData );
            break;
        default:
        {
            // added to avoid warnings
        }
    }
}

// static
ColorData ScDetectiveFunc::GetArrowColor()
{
    if (!bColorsInitialized)
        InitializeColors();
    return nArrowColor;
}

// static
ColorData ScDetectiveFunc::GetErrorColor()
{
    if (!bColorsInitialized)
        InitializeColors();
    return nErrorColor;
}

// static
ColorData ScDetectiveFunc::GetCommentColor()
{
    if (!bColorsInitialized)
        InitializeColors();
    return nCommentColor;
}

// static
void ScDetectiveFunc::InitializeColors()
{
    // may be called several times to update colors from configuration

    const svtools::ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
    nArrowColor   = rColorCfg.GetColorValue(svtools::CALCDETECTIVE).nColor;
    nErrorColor   = rColorCfg.GetColorValue(svtools::CALCDETECTIVEERROR).nColor;
    nCommentColor = rColorCfg.GetColorValue(svtools::CALCNOTESBACKGROUND).nColor;

    bColorsInitialized = TRUE;
}

// static
BOOL ScDetectiveFunc::IsColorsInitialized()
{
    return bColorsInitialized;
}

