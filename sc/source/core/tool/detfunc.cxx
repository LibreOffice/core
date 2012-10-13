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

#include "scitems.hxx"
#include <svtools/colorcfg.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outlobj.hxx>
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
#include <editeng/outliner.hxx>
#include <editeng/editobj.hxx>
#include <svx/sxcecitm.hxx>
#include <svl/whiter.hxx>
#include <editeng/writingmodeitem.hxx>

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
#include "rangelst.hxx"
#include "reftokenhelper.hxx"

#include <vector>

using ::std::vector;

//------------------------------------------------------------------------

// line ends are now created with an empty name.
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
    sal_uInt16      nMaxLevel;

public:
                ScDetectiveData( SdrModel* pModel );

    SfxItemSet& GetBoxSet()     { return aBoxSet; }
    SfxItemSet& GetArrowSet()   { return aArrowSet; }
    SfxItemSet& GetToTabSet()   { return aToTabSet; }
    SfxItemSet& GetFromTabSet() { return aFromTabSet; }
    SfxItemSet& GetCircleSet()  { return aCircleSet; }

    void        SetMaxLevel( sal_uInt16 nVal )      { nMaxLevel = nVal; }
    sal_uInt16      GetMaxLevel() const             { return nMaxLevel; }
};

class ScCommentData
{
public:
                        ScCommentData( ScDocument& rDoc, SdrModel* pModel );

    SfxItemSet&         GetCaptionSet() { return aCaptionSet; }
    void                UpdateCaptionSet( const SfxItemSet& rItemSet );

private:
    SfxItemSet          aCaptionSet;
};

//------------------------------------------------------------------------

ColorData ScDetectiveFunc::nArrowColor = 0;
ColorData ScDetectiveFunc::nErrorColor = 0;
ColorData ScDetectiveFunc::nCommentColor = 0;
sal_Bool ScDetectiveFunc::bColorsInitialized = false;

//------------------------------------------------------------------------

static sal_Bool lcl_HasThickLine( SdrObject& rObj )
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

    //  Standard-Linienenden (wie aus XLineEndList::Create) selber zusammenbasteln,
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
    aArrowSet.Put( XLineStartCenterItem( sal_True ) );
    aArrowSet.Put( XLineEndItem( aName, basegfx::B2DPolyPolygon(aTriangle) ) );
    aArrowSet.Put( XLineEndWidthItem( 200 ) );
    aArrowSet.Put( XLineEndCenterItem( false ) );

    aToTabSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aCircle) ) );
    aToTabSet.Put( XLineStartWidthItem( 200 ) );
    aToTabSet.Put( XLineStartCenterItem( sal_True ) );
    aToTabSet.Put( XLineEndItem( aName, basegfx::B2DPolyPolygon(aSquare) ) );
    aToTabSet.Put( XLineEndWidthItem( 300 ) );
    aToTabSet.Put( XLineEndCenterItem( false ) );

    aFromTabSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aSquare) ) );
    aFromTabSet.Put( XLineStartWidthItem( 300 ) );
    aFromTabSet.Put( XLineStartCenterItem( sal_True ) );
    aFromTabSet.Put( XLineEndItem( aName, basegfx::B2DPolyPolygon(aTriangle) ) );
    aFromTabSet.Put( XLineEndWidthItem( 200 ) );
    aFromTabSet.Put( XLineEndCenterItem( false ) );

    aCircleSet.Put( XLineColorItem( String(), Color( ScDetectiveFunc::GetErrorColor() ) ) );
    aCircleSet.Put( XFillStyleItem( XFILL_NONE ) );
    sal_uInt16 nWidth = 55;     // 54 = 1 Pixel
    aCircleSet.Put( XLineWidthItem( nWidth ) );
}

ScCommentData::ScCommentData( ScDocument& rDoc, SdrModel* pModel ) :
    aCaptionSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END, EE_ITEMS_START, EE_ITEMS_END, 0, 0 )
{
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(10.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(0.0, 30.0));
    aTriangle.append(basegfx::B2DPoint(20.0, 30.0));
    aTriangle.setClosed(true);

    String aName = SC_LINEEND_NAME;

    aCaptionSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aTriangle) ) );
    aCaptionSet.Put( XLineStartWidthItem( 200 ) );
    aCaptionSet.Put( XLineStartCenterItem( false ) );
    aCaptionSet.Put( XFillStyleItem( XFILL_SOLID ) );
    Color aYellow( ScDetectiveFunc::GetCommentColor() );
    aCaptionSet.Put( XFillColorItem( String(), aYellow ) );

    //  shadow
    //  SdrShadowItem has sal_False, instead the shadow is set for the rectangle
    //  only with SetSpecialTextBoxShadow when the object is created
    //  (item must be set to adjust objects from older files)
    aCaptionSet.Put( SdrShadowItem( false ) );
    aCaptionSet.Put( SdrShadowXDistItem( 100 ) );
    aCaptionSet.Put( SdrShadowYDistItem( 100 ) );

    //  text attributes
    aCaptionSet.Put( SdrTextLeftDistItem( 100 ) );
    aCaptionSet.Put( SdrTextRightDistItem( 100 ) );
    aCaptionSet.Put( SdrTextUpperDistItem( 100 ) );
    aCaptionSet.Put( SdrTextLowerDistItem( 100 ) );

    aCaptionSet.Put( SdrTextAutoGrowWidthItem( false ) );
    aCaptionSet.Put( SdrTextAutoGrowHeightItem( sal_True ) );

    //  do use the default cell style, so the user has a chance to
    //  modify the font for the annotations
    ((const ScPatternAttr&)rDoc.GetPool()->GetDefaultItem(ATTR_PATTERN)).
        FillEditItemSet( &aCaptionSet );

    // support the best position for the tail connector now that
    // that notes can be resized and repositioned.
    aCaptionSet.Put( SdrCaptionEscDirItem( SDRCAPT_ESCBESTFIT) );
}

void ScCommentData::UpdateCaptionSet( const SfxItemSet& rItemSet )
{
    SfxWhichIter aWhichIter( rItemSet );
    const SfxPoolItem* pPoolItem = 0;

    for( sal_uInt16 nWhich = aWhichIter.FirstWhich(); nWhich > 0; nWhich = aWhichIter.NextWhich() )
    {
        if(rItemSet.GetItemState(nWhich, false, &pPoolItem) == SFX_ITEM_SET)
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
           }
        }
    }
}

//------------------------------------------------------------------------

void ScDetectiveFunc::Modified()
{
    if (pDoc->IsStreamValid(nTab))
        pDoc->SetStreamValid(nTab, false);
}

inline sal_Bool Intersect( SCCOL nStartCol1, SCROW nStartRow1, SCCOL nEndCol1, SCROW nEndRow1,
                        SCCOL nStartCol2, SCROW nStartRow2, SCCOL nEndCol2, SCROW nEndRow2 )
{
    return nEndCol1 >= nStartCol2 && nEndCol2 >= nStartCol1 &&
            nEndRow1 >= nStartRow2 && nEndRow2 >= nStartRow1;
}

sal_Bool ScDetectiveFunc::HasError( const ScRange& rRange, ScAddress& rErrPos )
{
    rErrPos = rRange.aStart;
    sal_uInt16 nError = 0;

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

Point ScDetectiveFunc::GetDrawPos( SCCOL nCol, SCROW nRow, DrawPosMode eMode ) const
{
    OSL_ENSURE( ValidColRow( nCol, nRow ), "ScDetectiveFunc::GetDrawPos - invalid cell address" );
    SanitizeCol( nCol );
    SanitizeRow( nRow );

    Point aPos;

    switch( eMode )
    {
        case DRAWPOS_TOPLEFT:
        break;
        case DRAWPOS_BOTTOMRIGHT:
            ++nCol;
            ++nRow;
        break;
        case DRAWPOS_DETARROW:
            aPos.X() += pDoc->GetColWidth( nCol, nTab ) / 4;
            aPos.Y() += pDoc->GetRowHeight( nRow, nTab ) / 2;
        break;
        case DRAWPOS_CAPTIONLEFT:
            aPos.X() += 6;
        break;
        case DRAWPOS_CAPTIONRIGHT:
        {
            // find right end of passed cell position
            const ScMergeAttr* pMerge = static_cast< const ScMergeAttr* >( pDoc->GetAttr( nCol, nRow, nTab, ATTR_MERGE ) );
            if ( pMerge->GetColMerge() > 1 )
                nCol = nCol + pMerge->GetColMerge();
            else
                ++nCol;
            aPos.X() -= 6;
        }
        break;
    }

    for ( SCCOL i = 0; i < nCol; ++i )
        aPos.X() += pDoc->GetColWidth( i, nTab );
    aPos.Y() += pDoc->GetRowHeight( 0, nRow - 1, nTab );

    aPos.X() = static_cast< long >( aPos.X() * HMM_PER_TWIPS );
    aPos.Y() = static_cast< long >( aPos.Y() * HMM_PER_TWIPS );

    if ( pDoc->IsNegativePage( nTab ) )
        aPos.X() *= -1;

    return aPos;
}

Rectangle ScDetectiveFunc::GetDrawRect( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const
{
    Rectangle aRect(
        GetDrawPos( ::std::min( nCol1, nCol2 ), ::std::min( nRow1, nRow2 ), DRAWPOS_TOPLEFT ),
        GetDrawPos( ::std::max( nCol1, nCol2 ), ::std::max( nRow1, nRow2 ), DRAWPOS_BOTTOMRIGHT ) );
    aRect.Justify();    // reorder left/right in RTL sheets
    return aRect;
}

Rectangle ScDetectiveFunc::GetDrawRect( SCCOL nCol, SCROW nRow ) const
{
    return GetDrawRect( nCol, nRow, nCol, nRow );
}

static sal_Bool lcl_IsOtherTab( const basegfx::B2DPolyPolygon& rPolyPolygon )
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

sal_Bool ScDetectiveFunc::HasArrow( const ScAddress& rStart,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab )
{
    sal_Bool bStartAlien = ( rStart.Tab() != nTab );
    sal_Bool bEndAlien   = ( nEndTab != nTab );

    if (bStartAlien && bEndAlien)
    {
        OSL_FAIL("bStartAlien && bEndAlien");
        return true;
    }

    Rectangle aStartRect;
    Rectangle aEndRect;
    if (!bStartAlien)
        aStartRect = GetDrawRect( rStart.Col(), rStart.Row() );
    if (!bEndAlien)
        aEndRect = GetDrawRect( nEndCol, nEndRow );

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");

    sal_Bool bFound = false;
    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetLayer()==SC_LAYER_INTERN &&
                pObject->IsPolyObj() && pObject->GetPointCount()==2 )
        {
            const SfxItemSet& rSet = pObject->GetMergedItemSet();

            sal_Bool bObjStartAlien =
                lcl_IsOtherTab( ((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).GetLineStartValue() );
            sal_Bool bObjEndAlien =
                lcl_IsOtherTab( ((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).GetLineEndValue() );

            sal_Bool bStartHit = bStartAlien ? bObjStartAlien :
                                ( !bObjStartAlien && aStartRect.IsInside(pObject->GetPoint(0)) );
            sal_Bool bEndHit = bEndAlien ? bObjEndAlien :
                                ( !bObjEndAlien && aEndRect.IsInside(pObject->GetPoint(1)) );

            if ( bStartHit && bEndHit )
                bFound = sal_True;
        }
        pObject = aIter.Next();
    }

    return bFound;
}

sal_Bool ScDetectiveFunc::IsNonAlienArrow( SdrObject* pObject )
{
    if ( pObject->GetLayer()==SC_LAYER_INTERN &&
            pObject->IsPolyObj() && pObject->GetPointCount()==2 )
    {
        const SfxItemSet& rSet = pObject->GetMergedItemSet();

        sal_Bool bObjStartAlien =
            lcl_IsOtherTab( ((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).GetLineStartValue() );
        sal_Bool bObjEndAlien =
            lcl_IsOtherTab( ((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).GetLineEndValue() );

        return !bObjStartAlien && !bObjEndAlien;
    }

    return false;
}

//------------------------------------------------------------------------

//  InsertXXX: called from DrawEntry/DrawAlienEntry and InsertObject

sal_Bool ScDetectiveFunc::InsertArrow( SCCOL nCol, SCROW nRow,
                                SCCOL nRefStartCol, SCROW nRefStartRow,
                                SCCOL nRefEndCol, SCROW nRefEndRow,
                                sal_Bool bFromOtherTab, sal_Bool bRed,
                                ScDetectiveData& rData )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));

    sal_Bool bArea = ( nRefStartCol != nRefEndCol || nRefStartRow != nRefEndRow );
    if (bArea && !bFromOtherTab)
    {
        // insert the rectangle before the arrow - this is relied on in FindFrameForObject

        Rectangle aRect = GetDrawRect( nRefStartCol, nRefStartRow, nRefEndCol, nRefEndRow );
        SdrRectObj* pBox = new SdrRectObj( aRect );

        pBox->SetMergedItemSetAndBroadcast(rData.GetBoxSet());

        pBox->SetLayer( SC_LAYER_INTERN );
        pPage->InsertObject( pBox );
        pModel->AddCalcUndo( new SdrUndoInsertObj( *pBox ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pBox, sal_True );
        pData->maStart.Set( nRefStartCol, nRefStartRow, nTab);
        pData->maEnd.Set( nRefEndCol, nRefEndRow, nTab);
    }

    Point aStartPos = GetDrawPos( nRefStartCol, nRefStartRow, DRAWPOS_DETARROW );
    Point aEndPos = GetDrawPos( nCol, nRow, DRAWPOS_DETARROW );

    if (bFromOtherTab)
    {
        sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );
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

    pArrow->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pArrow );
    pModel->AddCalcUndo( new SdrUndoInsertObj( *pArrow ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData(pArrow, true);
    if (bFromOtherTab)
        pData->maStart.SetInvalid();
    else
        pData->maStart.Set( nRefStartCol, nRefStartRow, nTab);

    pData->maEnd.Set( nCol, nRow, nTab);
    pData->meType = ScDrawObjData::DetectiveArrow;

    Modified();
    return true;
}

sal_Bool ScDetectiveFunc::InsertToOtherTab( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, sal_Bool bRed,
                                ScDetectiveData& rData )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));

    sal_Bool bArea = ( nStartCol != nEndCol || nStartRow != nEndRow );
    if (bArea)
    {
        Rectangle aRect = GetDrawRect( nStartCol, nStartRow, nEndCol, nEndRow );
        SdrRectObj* pBox = new SdrRectObj( aRect );

        pBox->SetMergedItemSetAndBroadcast(rData.GetBoxSet());

        pBox->SetLayer( SC_LAYER_INTERN );
        pPage->InsertObject( pBox );
        pModel->AddCalcUndo( new SdrUndoInsertObj( *pBox ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pBox, sal_True );
        pData->maStart.Set( nStartCol, nStartRow, nTab);
        pData->maEnd.Set( nEndCol, nEndRow, nTab);
    }

    sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );
    long nPageSign = bNegativePage ? -1 : 1;

    Point aStartPos = GetDrawPos( nStartCol, nStartRow, DRAWPOS_DETARROW );
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

    pArrow->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pArrow );
    pModel->AddCalcUndo( new SdrUndoInsertObj( *pArrow ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pArrow, sal_True );
    pData->maStart.Set( nStartCol, nStartRow, nTab);
    pData->maEnd.SetInvalid();

    Modified();
    return sal_True;
}

//------------------------------------------------------------------------

//  DrawEntry:      Formel auf dieser Tabelle,
//                  Referenz auf dieser oder anderer
//  DrawAlienEntry: Formel auf anderer Tabelle,
//                  Referenz auf dieser

//      return FALSE: da war schon ein Pfeil

sal_Bool ScDetectiveFunc::DrawEntry( SCCOL nCol, SCROW nRow,
                                    const ScRange& rRef,
                                    ScDetectiveData& rData )
{
    if ( HasArrow( rRef.aStart, nCol, nRow, nTab ) )
        return false;

    ScAddress aErrorPos;
    sal_Bool bError = HasError( rRef, aErrorPos );
    sal_Bool bAlien = ( rRef.aEnd.Tab() < nTab || rRef.aStart.Tab() > nTab );

    return InsertArrow( nCol, nRow,
                        rRef.aStart.Col(), rRef.aStart.Row(),
                        rRef.aEnd.Col(), rRef.aEnd.Row(),
                        bAlien, bError, rData );
}

sal_Bool ScDetectiveFunc::DrawAlienEntry( const ScRange& rRef,
                                        ScDetectiveData& rData )
{
    if ( HasArrow( rRef.aStart, 0, 0, nTab+1 ) )
        return false;

    ScAddress aErrorPos;
    sal_Bool bError = HasError( rRef, aErrorPos );

    return InsertToOtherTab( rRef.aStart.Col(), rRef.aStart.Row(),
                                rRef.aEnd.Col(), rRef.aEnd.Row(),
                                bError, rData );
}

void ScDetectiveFunc::DrawCircle( SCCOL nCol, SCROW nRow, ScDetectiveData& rData )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));

    Rectangle aRect = GetDrawRect( nCol, nRow );
    aRect.Left()    -= 250;
    aRect.Right()   += 250;
    aRect.Top()     -= 70;
    aRect.Bottom()  += 70;

    SdrCircObj* pCircle = new SdrCircObj( OBJ_CIRC, aRect );
    SfxItemSet& rAttrSet = rData.GetCircleSet();

    pCircle->SetMergedItemSetAndBroadcast(rAttrSet);

    pCircle->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pCircle );
    pModel->AddCalcUndo( new SdrUndoInsertObj( *pCircle ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pCircle, sal_True );
    pData->maStart.Set( nCol, nRow, nTab);
    pData->maEnd.SetInvalid();
    pData->meType = ScDrawObjData::ValidationCircle;

    Modified();
}

void ScDetectiveFunc::DeleteArrowsAt( SCCOL nCol, SCROW nRow, sal_Bool bDestPnt )
{
    Rectangle aRect = GetDrawRect( nCol, nRow );

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");

    pPage->RecalcObjOrdNums();

    sal_uLong nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        long nDelCount = 0;
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

        Modified();
    }
}

        //      Box um Referenz loeschen

#define SC_DET_TOLERANCE    50

inline sal_Bool RectIsPoints( const Rectangle& rRect, const Point& rStart, const Point& rEnd )
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
    Rectangle aCornerRect = GetDrawRect( nCol1, nRow1, nCol2, nRow2 );
    Point aStartCorner = aCornerRect.TopLeft();
    Point aEndCorner = aCornerRect.BottomRight();
    Rectangle aObjRect;

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");

    pPage->RecalcObjOrdNums();

    sal_uLong nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        long nDelCount = 0;
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

        Modified();
    }
}

//------------------------------------------------------------------------

sal_uInt16 ScDetectiveFunc::InsertPredLevelArea( const ScRange& rRef,
                                        ScDetectiveData& rData, sal_uInt16 nLevel )
{
    sal_uInt16 nResult = DET_INS_EMPTY;

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

sal_uInt16 ScDetectiveFunc::InsertPredLevel( SCCOL nCol, SCROW nRow, ScDetectiveData& rData,
                                            sal_uInt16 nLevel )
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
    pFCell->SetRunning(sal_True);

    sal_uInt16 nResult = DET_INS_EMPTY;

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
                sal_uInt16 nSubResult;
                sal_Bool bArea = (aRef.aStart != aRef.aEnd);
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

    pFCell->SetRunning(false);

    return nResult;
}

sal_uInt16 ScDetectiveFunc::FindPredLevelArea( const ScRange& rRef,
                                                sal_uInt16 nLevel, sal_uInt16 nDeleteLevel )
{
    sal_uInt16 nResult = nLevel;

    ScCellIterator aCellIter( pDoc, rRef);
    ScBaseCell* pCell = aCellIter.GetFirst();
    while (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            sal_uInt16 nTemp = FindPredLevel( aCellIter.GetCol(), aCellIter.GetRow(), nLevel, nDeleteLevel );
            if (nTemp > nResult)
                nResult = nTemp;
        }
        pCell = aCellIter.GetNext();
    }

    return nResult;
}

                                            //  nDeleteLevel != 0   -> loeschen

sal_uInt16 ScDetectiveFunc::FindPredLevel( SCCOL nCol, SCROW nRow, sal_uInt16 nLevel, sal_uInt16 nDeleteLevel )
{
    OSL_ENSURE( nLevel<1000, "Level" );

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
    pFCell->SetRunning(sal_True);

    sal_uInt16 nResult = nLevel;
    sal_Bool bDelete = ( nDeleteLevel && nLevel == nDeleteLevel-1 );

    if ( bDelete )
    {
        DeleteArrowsAt( nCol, nRow, sal_True );                 // Pfeile, die hierher zeigen
    }

    ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
    ScRange aRef;
    while ( aIter.GetNextRef( aRef) )
    {
        sal_Bool bArea = ( aRef.aStart != aRef.aEnd );

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
                sal_uInt16 nTemp;
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

    pFCell->SetRunning(false);

    return nResult;
}

//------------------------------------------------------------------------

sal_uInt16 ScDetectiveFunc::InsertErrorLevel( SCCOL nCol, SCROW nRow, ScDetectiveData& rData,
                                            sal_uInt16 nLevel )
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
    pFCell->SetRunning(sal_True);

    sal_uInt16 nResult = DET_INS_EMPTY;

    ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
    ScRange aRef;
    ScAddress aErrorPos;
    sal_Bool bHasError = false;
    while ( aIter.GetNextRef( aRef ) )
    {
        if (HasError( aRef, aErrorPos ))
        {
            bHasError = sal_True;
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

    pFCell->SetRunning(false);

                                                    // Blaetter ?
    if (!bHasError)
        if (InsertPredLevel( nCol, nRow, rData, rData.GetMaxLevel() ) == DET_INS_INSERTED)
            nResult = DET_INS_INSERTED;

    return nResult;
}

//------------------------------------------------------------------------

sal_uInt16 ScDetectiveFunc::InsertSuccLevel( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        ScDetectiveData& rData, sal_uInt16 nLevel )
{
    //  ueber ganzes Dokument

    sal_uInt16 nResult = DET_INS_EMPTY;
    ScCellIterator aCellIter( pDoc, 0,0,0, MAXCOL,MAXROW,MAXTAB );          // alle Tabellen
    ScBaseCell* pCell = aCellIter.GetFirst();
    while (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
            sal_Bool bRunning = pFCell->IsRunning();

            if (pFCell->GetDirty())
                pFCell->Interpret();                // nach SetRunning geht's nicht mehr!
            pFCell->SetRunning(sal_True);

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
                        sal_Bool bAlien = ( aCellIter.GetTab() != nTab );
                        sal_Bool bDrawRet;
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
                                    sal_uInt16 nSubResult = InsertSuccLevel(
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

sal_uInt16 ScDetectiveFunc::FindSuccLevel( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        sal_uInt16 nLevel, sal_uInt16 nDeleteLevel )
{
    OSL_ENSURE( nLevel<1000, "Level" );

    sal_uInt16 nResult = nLevel;
    sal_Bool bDelete = ( nDeleteLevel && nLevel == nDeleteLevel-1 );

    ScCellIterator aCellIter( pDoc, 0,0, nTab, MAXCOL,MAXROW, nTab );
    ScBaseCell* pCell = aCellIter.GetFirst();
    while (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
            sal_Bool bRunning = pFCell->IsRunning();

            if (pFCell->GetDirty())
                pFCell->Interpret();                // nach SetRunning geht's nicht mehr!
            pFCell->SetRunning(sal_True);

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
                            DeleteArrowsAt( aRef.aStart.Col(), aRef.aStart.Row(), false );
                        }
                        else if ( !bRunning &&
                                HasArrow( aRef.aStart,
                                            aCellIter.GetCol(),aCellIter.GetRow(),aCellIter.GetTab() ) )
                        {
                            sal_uInt16 nTemp = FindSuccLevel( aCellIter.GetCol(), aCellIter.GetRow(),
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

sal_Bool ScDetectiveFunc::ShowPred( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    ScDetectiveData aData( pModel );

    sal_uInt16 nMaxLevel = 0;
    sal_uInt16 nResult = DET_INS_CONTINUE;
    while (nResult == DET_INS_CONTINUE && nMaxLevel < 1000)
    {
        aData.SetMaxLevel( nMaxLevel );
        nResult = InsertPredLevel( nCol, nRow, aData, 0 );
        ++nMaxLevel;
    }

    return ( nResult == DET_INS_INSERTED );
}

sal_Bool ScDetectiveFunc::ShowSucc( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    ScDetectiveData aData( pModel );

    sal_uInt16 nMaxLevel = 0;
    sal_uInt16 nResult = DET_INS_CONTINUE;
    while (nResult == DET_INS_CONTINUE && nMaxLevel < 1000)
    {
        aData.SetMaxLevel( nMaxLevel );
        nResult = InsertSuccLevel( nCol, nRow, nCol, nRow, aData, 0 );
        ++nMaxLevel;
    }

    return ( nResult == DET_INS_INSERTED );
}

sal_Bool ScDetectiveFunc::ShowError( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    ScRange aRange( nCol, nRow, nTab );
    ScAddress aErrPos;
    if ( !HasError( aRange,aErrPos ) )
        return false;

    ScDetectiveData aData( pModel );

    aData.SetMaxLevel( 1000 );
    sal_uInt16 nResult = InsertErrorLevel( nCol, nRow, aData, 0 );

    return ( nResult == DET_INS_INSERTED );
}

sal_Bool ScDetectiveFunc::DeleteSucc( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    sal_uInt16 nLevelCount = FindSuccLevel( nCol, nRow, nCol, nRow, 0, 0 );
    if ( nLevelCount )
        FindSuccLevel( nCol, nRow, nCol, nRow, 0, nLevelCount );            // loeschen

    return ( nLevelCount != 0 );
}

sal_Bool ScDetectiveFunc::DeletePred( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    sal_uInt16 nLevelCount = FindPredLevel( nCol, nRow, 0, 0 );
    if ( nLevelCount )
        FindPredLevel( nCol, nRow, 0, nLevelCount );            // loeschen

    return ( nLevelCount != 0 );
}

sal_Bool ScDetectiveFunc::DeleteAll( ScDetectiveDelete eWhat )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");

    pPage->RecalcObjOrdNums();

    long    nDelCount = 0;
    sal_uLong   nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        SdrObject** ppObj = new SdrObject*[nObjCount];

        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetLayer() == SC_LAYER_INTERN )
            {
                sal_Bool bDoThis = sal_True;
                if ( eWhat != SC_DET_ALL )
                {
                    sal_Bool bCircle = ( pObject->ISA(SdrCircObj) );
                    sal_Bool bCaption = ScDrawLayer::IsNoteCaption( pObject );
                    if ( eWhat == SC_DET_DETECTIVE )        // Detektiv, aus Menue
                        bDoThis = !bCaption;                // auch Kreise
                    else if ( eWhat == SC_DET_CIRCLES )     // Kreise, wenn neue erzeugt werden
                        bDoThis = bCircle;
                    else if ( eWhat == SC_DET_ARROWS )      // DetectiveRefresh
                        bDoThis = !bCaption && !bCircle;    // don't include circles
                    else
                    {
                        OSL_FAIL("wat?");
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

        Modified();
    }

    return ( nDelCount != 0 );
}

sal_Bool ScDetectiveFunc::MarkInvalid(sal_Bool& rOverflow)
{
    rOverflow = false;
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    sal_Bool bDeleted = DeleteAll( SC_DET_CIRCLES );        // nur die Kreise

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
        sal_uLong nIndex = ((const SfxUInt32Item&)pPattern->GetItem(ATTR_VALIDDATA)).GetValue();
        if (nIndex)
        {
            const ScValidationData* pData = pDoc->GetValidationEntry( nIndex );
            if ( pData )
            {
                //  Zellen in dem Bereich durchgehen

                sal_Bool bMarkEmpty = !pData->IsIgnoreBlank();
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
        rOverflow = sal_True;

    return ( bDeleted || nInsCount != 0 );
}

void ScDetectiveFunc::GetAllPreds(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                  vector<ScTokenRef>& rRefTokens)
{
    ScCellIterator aCellIter(pDoc, nCol1, nRow1, nTab, nCol2, nRow2, nTab);
    for (ScBaseCell* pCell = aCellIter.GetFirst(); pCell; pCell = aCellIter.GetNext())
    {
        if (pCell->GetCellType() != CELLTYPE_FORMULA)
            continue;

        ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);
        ScDetectiveRefIter aRefIter(pFCell);
        for (ScToken* p = aRefIter.GetNextRefToken(); p; p = aRefIter.GetNextRefToken())
        {
            ScTokenRef pRef(static_cast<ScToken*>(p->Clone()));
            pRef->CalcAbsIfRel(aCellIter.GetPos());
            ScRefTokenHelper::join(rRefTokens, pRef);
        }
    }
}

void ScDetectiveFunc::GetAllSuccs(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                  vector<ScTokenRef>& rRefTokens)
{
    vector<ScTokenRef> aSrcRange;
    aSrcRange.push_back(
        ScRefTokenHelper::createRefToken(ScRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab)));

    ScCellIterator aCellIter(pDoc, 0, 0, nTab, MAXCOL, MAXROW, nTab);
    for (ScBaseCell* pCell = aCellIter.GetFirst(); pCell; pCell = aCellIter.GetNext())
    {
        if (pCell->GetCellType() != CELLTYPE_FORMULA)
            continue;

        ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);
        ScDetectiveRefIter aRefIter(pFCell);
        for (ScToken* p = aRefIter.GetNextRefToken(); p; p = aRefIter.GetNextRefToken())
        {
            ScAddress aPos = aCellIter.GetPos();
            ScTokenRef pRef(static_cast<ScToken*>(p->Clone()));
            pRef->CalcAbsIfRel(aPos);
            if (ScRefTokenHelper::intersects(aSrcRange, pRef))
            {
                // This address is absolute.
                pRef = ScRefTokenHelper::createRefToken(aPos);
                ScRefTokenHelper::join(rRefTokens, pRef);
            }
        }
    }
}

void ScDetectiveFunc::UpdateAllComments( ScDocument& rDoc )
{
    //  for all caption objects, update attributes and SpecialTextBoxShadow flag
    //  (on all tables - nTab is ignored!)

    //  no undo actions, this is refreshed after undo

    ScDrawLayer* pModel = rDoc.GetDrawLayer();
    if (!pModel)
        return;

    for( SCTAB nObjTab = 0, nTabCount = rDoc.GetTableCount(); nObjTab < nTabCount; ++nObjTab )
    {
        rDoc.InitializeNoteCaptions( nObjTab );
        SdrPage* pPage = pModel->GetPage( static_cast< sal_uInt16 >( nObjTab ) );
        OSL_ENSURE( pPage, "Page ?" );
        if( pPage )
        {
            SdrObjListIter aIter( *pPage, IM_FLAT );
            for( SdrObject* pObject = aIter.Next(); pObject; pObject = aIter.Next() )
            {
                if ( ScDrawObjData* pData = ScDrawLayer::GetNoteCaptionData( pObject, nObjTab ) )
                {
                    ScPostIt* pNote = rDoc.GetNotes( pData->maStart.Tab() )->findByAddress( pData->maStart );
                    // caption should exist, we iterate over drawing objects...
                    OSL_ENSURE( pNote && (pNote->GetCaption() == pObject), "ScDetectiveFunc::UpdateAllComments - invalid cell note" );
                    if( pNote )
                    {
                        ScCommentData aData( rDoc, pModel );
                        SfxItemSet aAttrColorSet = pObject->GetMergedItemSet();
                        aAttrColorSet.Put( XFillColorItem( String(), GetCommentColor() ) );
                        aData.UpdateCaptionSet( aAttrColorSet );
                        pObject->SetMergedItemSetAndBroadcast( aData.GetCaptionSet() );
                        if( SdrCaptionObj* pCaption = dynamic_cast< SdrCaptionObj* >( pObject ) )
                        {
                            pCaption->SetSpecialTextBoxShadow();
                            pCaption->SetFixedTail();
                        }
                    }
                }
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

    for( SCTAB nObjTab = 0, nTabCount = pDoc->GetTableCount(); nObjTab < nTabCount; ++nObjTab )
    {
        SdrPage* pPage = pModel->GetPage( static_cast< sal_uInt16 >( nObjTab ) );
        OSL_ENSURE( pPage, "Page ?" );
        if( pPage )
        {
            SdrObjListIter aIter( *pPage, IM_FLAT );
            for( SdrObject* pObject = aIter.Next(); pObject; pObject = aIter.Next() )
            {
                if ( pObject->GetLayer() == SC_LAYER_INTERN )
                {
                    sal_Bool bArrow = false;
                    sal_Bool bError = false;

                    ScAddress aPos;
                    ScRange aSource;
                    bool bDummy;
                    ScDetectiveObjType eType = GetDetectiveObjectType( pObject, nObjTab, aPos, aSource, bDummy );
                    if ( eType == SC_DETOBJ_ARROW || eType == SC_DETOBJ_TOOTHERTAB )
                    {
                        //  source is valid, determine error flag from source range

                        ScAddress aErrPos;
                        if ( HasError( aSource, aErrPos ) )
                            bError = sal_True;
                        else
                            bArrow = sal_True;
                    }
                    else if ( eType == SC_DETOBJ_FROMOTHERTAB )
                    {
                        //  source range is no longer known, take error flag from formula itself
                        //  (this means, if the formula has an error, all references to other tables
                        //  are marked red)

                        ScAddress aErrPos;
                        if ( HasError( ScRange( aPos), aErrPos ) )
                            bError = sal_True;
                        else
                            bArrow = sal_True;
                    }
                    else if ( eType == SC_DETOBJ_CIRCLE )
                    {
                        //  circles (error marks) are always red

                        bError = sal_True;
                    }
                    else if ( eType == SC_DETOBJ_NONE )
                    {
                        //  frame for area reference has no ObjType, always gets arrow color

                        if ( pObject->ISA( SdrRectObj ) && !pObject->ISA( SdrCaptionObj ) )
                        {
                            bArrow = sal_True;
                        }
                    }

                    if ( bArrow || bError )
                    {
                        ColorData nColorData = ( bError ? GetErrorColor() : GetArrowColor() );
                        pObject->SetMergedItem( XLineColorItem( String(), Color( nColorData ) ) );

                        // repaint only
                        pObject->ActionChanged();
                    }
                }
            }
        }
    }
}

sal_Bool ScDetectiveFunc::FindFrameForObject( SdrObject* pObject, ScRange& rRange )
{
    //  find the rectangle for an arrow (always the object directly before the arrow)
    //  rRange must be initialized to the source cell of the arrow (start of area)

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel) return false;

    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");
    if (!pPage) return false;

    // test if the object is a direct page member
    if( pObject && pObject->GetPage() && (pObject->GetPage() == pObject->GetObjList()) )
    {
        // Is there a previous object?
        const sal_uInt32 nOrdNum(pObject->GetOrdNum());

        if(nOrdNum > 0)
        {
            SdrObject* pPrevObj = pPage->GetObj(nOrdNum - 1);

            if ( pPrevObj && pPrevObj->GetLayer() == SC_LAYER_INTERN && pPrevObj->ISA(SdrRectObj) )
            {
                ScDrawObjData* pPrevData = ScDrawLayer::GetObjDataTab( pPrevObj, rRange.aStart.Tab() );
                if ( pPrevData && pPrevData->maStart.IsValid() && pPrevData->maEnd.IsValid() && (pPrevData->maStart == rRange.aStart) )
                {
                    rRange.aEnd = pPrevData->maEnd;
                    return sal_True;
                }
            }
        }
    }
    return false;
}

ScDetectiveObjType ScDetectiveFunc::GetDetectiveObjectType( SdrObject* pObject, SCTAB nObjTab,
                                ScAddress& rPosition, ScRange& rSource, bool& rRedLine )
{
    rRedLine = false;
    ScDetectiveObjType eType = SC_DETOBJ_NONE;

    if ( pObject && pObject->GetLayer() == SC_LAYER_INTERN )
    {
        if ( ScDrawObjData* pData = ScDrawLayer::GetObjDataTab( pObject, nObjTab ) )
        {
            bool bValidStart = pData->maStart.IsValid();
            bool bValidEnd = pData->maEnd.IsValid();

            if ( pObject->IsPolyObj() && pObject->GetPointCount() == 2 )
            {
                // line object -> arrow

                if ( bValidStart )
                    eType = bValidEnd ? SC_DETOBJ_ARROW : SC_DETOBJ_TOOTHERTAB;
                else if ( bValidEnd )
                    eType = SC_DETOBJ_FROMOTHERTAB;

                if ( bValidStart )
                    rSource = pData->maStart;
                if ( bValidEnd )
                    rPosition = pData->maEnd;

                if ( bValidStart && lcl_HasThickLine( *pObject ) )
                {
                    // thick line -> look for frame before this object

                    FindFrameForObject( pObject, rSource );     // modifies rSource
                }

                ColorData nObjColor = ((const XLineColorItem&)pObject->GetMergedItem(XATTR_LINECOLOR)).GetColorValue().GetColor();
                if ( nObjColor == GetErrorColor() && nObjColor != GetArrowColor() )
                    rRedLine = true;
            }
            else if ( pObject->ISA(SdrCircObj) )
            {
                if ( bValidStart )
                {
                    // cell position is returned in rPosition

                    rPosition = pData->maStart;
                    eType = SC_DETOBJ_CIRCLE;
                }
            }
        }
    }

    return eType;
}

void ScDetectiveFunc::InsertObject( ScDetectiveObjType eType,
                            const ScAddress& rPosition, const ScRange& rSource,
                            sal_Bool bRedLine )
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

ColorData ScDetectiveFunc::GetArrowColor()
{
    if (!bColorsInitialized)
        InitializeColors();
    return nArrowColor;
}

ColorData ScDetectiveFunc::GetErrorColor()
{
    if (!bColorsInitialized)
        InitializeColors();
    return nErrorColor;
}

ColorData ScDetectiveFunc::GetCommentColor()
{
    if (!bColorsInitialized)
        InitializeColors();
    return nCommentColor;
}

void ScDetectiveFunc::InitializeColors()
{
    // may be called several times to update colors from configuration

    const svtools::ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
    nArrowColor   = rColorCfg.GetColorValue(svtools::CALCDETECTIVE).nColor;
    nErrorColor   = rColorCfg.GetColorValue(svtools::CALCDETECTIVEERROR).nColor;
    nCommentColor = rColorCfg.GetColorValue(svtools::CALCNOTESBACKGROUND).nColor;

    bColorsInitialized = sal_True;
}

sal_Bool ScDetectiveFunc::IsColorsInitialized()
{
    return bColorsInitialized;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
