/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <scitems.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outlobj.hxx>
#include <formula/errorcodes.hxx>
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

#include <detfunc.hxx>
#include <document.hxx>
#include <dociter.hxx>
#include <drwlayer.hxx>
#include <userdat.hxx>
#include <validat.hxx>
#include <formulacell.hxx>
#include <docpool.hxx>
#include <patattr.hxx>
#include <attrib.hxx>
#include <scmod.hxx>
#include <postit.hxx>
#include <rangelst.hxx>
#include <reftokenhelper.hxx>
#include <formulaiter.hxx>
#include <cellvalue.hxx>

#include <vector>
#include <memory>

using ::std::vector;
using namespace com::sun::star;

// line ends are now created with an empty name.
// The checkForUniqueItem method then finds a unique name for the item's value.
#define SC_LINEEND_NAME     EMPTY_OUSTRING

enum DetInsertResult {              // return-values for inserting in one level
            DET_INS_CONTINUE,
            DET_INS_INSERTED,
            DET_INS_EMPTY,
            DET_INS_CIRCULAR };

class ScDetectiveData
{
private:
    SfxItemSet  aBoxSet;
    SfxItemSet  aArrowSet;
    SfxItemSet  aToTabSet;
    SfxItemSet  aFromTabSet;
    SfxItemSet  aCircleSet;         //TODO: individually ?
    sal_uInt16  nMaxLevel;

public:
    explicit ScDetectiveData( SdrModel* pModel );

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

Color ScDetectiveFunc::nArrowColor = Color(0);
Color ScDetectiveFunc::nErrorColor = Color(0);
Color ScDetectiveFunc::nCommentColor = Color(0);
bool ScDetectiveFunc::bColorsInitialized = false;

static bool lcl_HasThickLine( const SdrObject& rObj )
{
    // thin lines get width 0 -> everything greater 0 is a thick line

    return rObj.GetMergedItem(XATTR_LINEWIDTH).GetValue() > 0;
}

ScDetectiveData::ScDetectiveData( SdrModel* pModel ) :
    aBoxSet( pModel->GetItemPool(), svl::Items<SDRATTR_START, SDRATTR_END>{} ),
    aArrowSet( pModel->GetItemPool(), svl::Items<SDRATTR_START, SDRATTR_END>{} ),
    aToTabSet( pModel->GetItemPool(), svl::Items<SDRATTR_START, SDRATTR_END>{} ),
    aFromTabSet( pModel->GetItemPool(), svl::Items<SDRATTR_START, SDRATTR_END>{} ),
    aCircleSet( pModel->GetItemPool(), svl::Items<SDRATTR_START, SDRATTR_END>{} )
{
    nMaxLevel = 0;

    aBoxSet.Put( XLineColorItem( EMPTY_OUSTRING, ScDetectiveFunc::GetArrowColor() ) );
    aBoxSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );

    //  create default line endings (like XLineEndList::Create)
    //  to be independent from the configured line endings

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

    basegfx::B2DPolygon aCircle(basegfx::utils::createPolygonFromEllipse(basegfx::B2DPoint(0.0, 0.0), 100.0, 100.0));
    aCircle.setClosed(true);

    OUString aName = SC_LINEEND_NAME;

    aArrowSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aCircle) ) );
    aArrowSet.Put( XLineStartWidthItem( 200 ) );
    aArrowSet.Put( XLineStartCenterItem( true ) );
    aArrowSet.Put( XLineEndItem( aName, basegfx::B2DPolyPolygon(aTriangle) ) );
    aArrowSet.Put( XLineEndWidthItem( 200 ) );
    aArrowSet.Put( XLineEndCenterItem( false ) );

    aToTabSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aCircle) ) );
    aToTabSet.Put( XLineStartWidthItem( 200 ) );
    aToTabSet.Put( XLineStartCenterItem( true ) );
    aToTabSet.Put( XLineEndItem( aName, basegfx::B2DPolyPolygon(aSquare) ) );
    aToTabSet.Put( XLineEndWidthItem( 300 ) );
    aToTabSet.Put( XLineEndCenterItem( false ) );

    aFromTabSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aSquare) ) );
    aFromTabSet.Put( XLineStartWidthItem( 300 ) );
    aFromTabSet.Put( XLineStartCenterItem( true ) );
    aFromTabSet.Put( XLineEndItem( aName, basegfx::B2DPolyPolygon(aTriangle) ) );
    aFromTabSet.Put( XLineEndWidthItem( 200 ) );
    aFromTabSet.Put( XLineEndCenterItem( false ) );

    aCircleSet.Put( XLineColorItem( OUString(), ScDetectiveFunc::GetErrorColor() ) );
    aCircleSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );
    aCircleSet.Put( XLineWidthItem( 55 ) ); // 54 = 1 Pixel
}

ScCommentData::ScCommentData( ScDocument& rDoc, SdrModel* pModel ) :
    aCaptionSet( pModel->GetItemPool(), svl::Items<SDRATTR_START, SDRATTR_END, EE_ITEMS_START, EE_ITEMS_END>{} )
{
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(10.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(0.0, 30.0));
    aTriangle.append(basegfx::B2DPoint(20.0, 30.0));
    aTriangle.setClosed(true);

    OUString aName = SC_LINEEND_NAME;

    aCaptionSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aTriangle) ) );
    aCaptionSet.Put( XLineStartWidthItem( 200 ) );
    aCaptionSet.Put( XLineStartCenterItem( false ) );
    aCaptionSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
    Color aYellow( ScDetectiveFunc::GetCommentColor() );
    aCaptionSet.Put( XFillColorItem( OUString(), aYellow ) );

    //  shadow
    //  SdrShadowItem has sal_False, instead the shadow is set for the rectangle
    //  only with SetSpecialTextBoxShadow when the object is created
    //  (item must be set to adjust objects from older files)
    aCaptionSet.Put( makeSdrShadowItem( false ) );
    aCaptionSet.Put( makeSdrShadowXDistItem( 100 ) );
    aCaptionSet.Put( makeSdrShadowYDistItem( 100 ) );

    //  text attributes
    aCaptionSet.Put( makeSdrTextLeftDistItem( 100 ) );
    aCaptionSet.Put( makeSdrTextRightDistItem( 100 ) );
    aCaptionSet.Put( makeSdrTextUpperDistItem( 100 ) );
    aCaptionSet.Put( makeSdrTextLowerDistItem( 100 ) );

    aCaptionSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
    aCaptionSet.Put( makeSdrTextAutoGrowHeightItem( true ) );

    //  do use the default cell style, so the user has a chance to
    //  modify the font for the annotations
    rDoc.GetPool()->GetDefaultItem(ATTR_PATTERN).FillEditItemSet( &aCaptionSet );

    // support the best position for the tail connector now that
    // that notes can be resized and repositioned.
    aCaptionSet.Put( SdrCaptionEscDirItem( SdrCaptionEscDir::BestFit) );
}

void ScCommentData::UpdateCaptionSet( const SfxItemSet& rItemSet )
{
    SfxWhichIter aWhichIter( rItemSet );
    const SfxPoolItem* pPoolItem = nullptr;

    for( sal_uInt16 nWhich = aWhichIter.FirstWhich(); nWhich > 0; nWhich = aWhichIter.NextWhich() )
    {
        if(rItemSet.GetItemState(nWhich, false, &pPoolItem) == SfxItemState::SET)
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

void ScDetectiveFunc::Modified()
{
    pDoc->SetStreamValid(nTab, false);
}

static bool Intersect( SCCOL nStartCol1, SCROW nStartRow1, SCCOL nEndCol1, SCROW nEndRow1,
                        SCCOL nStartCol2, SCROW nStartRow2, SCCOL nEndCol2, SCROW nEndRow2 )
{
    return nEndCol1 >= nStartCol2 && nEndCol2 >= nStartCol1 &&
            nEndRow1 >= nStartRow2 && nEndRow2 >= nStartRow1;
}

bool ScDetectiveFunc::HasError( const ScRange& rRange, ScAddress& rErrPos )
{
    rErrPos = rRange.aStart;
    FormulaError nError = FormulaError::NONE;

    ScCellIterator aIter( pDoc, rRange);
    for (bool bHasCell = aIter.first(); bHasCell; bHasCell = aIter.next())
    {
        if (aIter.getType() != CELLTYPE_FORMULA)
            continue;

        nError = aIter.getFormulaCell()->GetErrCode();
        if (nError != FormulaError::NONE)
            rErrPos = aIter.GetPos();
    }

    return (nError != FormulaError::NONE);
}

Point ScDetectiveFunc::GetDrawPos( SCCOL nCol, SCROW nRow, DrawPosMode eMode ) const
{
    OSL_ENSURE( ValidColRow( nCol, nRow ), "ScDetectiveFunc::GetDrawPos - invalid cell address" );
    nCol = SanitizeCol( nCol );
    nRow = SanitizeRow( nRow );

    Point aPos;

    switch( eMode )
    {
        case DrawPosMode::TopLeft:
        break;
        case DrawPosMode::BottomRight:
            ++nCol;
            ++nRow;
        break;
        case DrawPosMode::DetectiveArrow:
            aPos.AdjustX(pDoc->GetColWidth( nCol, nTab ) / 4 );
            aPos.AdjustY(pDoc->GetRowHeight( nRow, nTab ) / 2 );
        break;
    }

    for ( SCCOL i = 0; i < nCol; ++i )
        aPos.AdjustX(pDoc->GetColWidth( i, nTab ) );
    aPos.AdjustY(pDoc->GetRowHeight( 0, nRow - 1, nTab ) );

    aPos.setX( static_cast< long >( aPos.X() * HMM_PER_TWIPS ) );
    aPos.setY( static_cast< long >( aPos.Y() * HMM_PER_TWIPS ) );

    if ( pDoc->IsNegativePage( nTab ) )
        aPos.setX( aPos.X() * -1 );

    return aPos;
}

tools::Rectangle ScDetectiveFunc::GetDrawRect( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const
{
    tools::Rectangle aRect(
        GetDrawPos( ::std::min( nCol1, nCol2 ), ::std::min( nRow1, nRow2 ), DrawPosMode::TopLeft ),
        GetDrawPos( ::std::max( nCol1, nCol2 ), ::std::max( nRow1, nRow2 ), DrawPosMode::BottomRight ) );
    aRect.Justify();    // reorder left/right in RTL sheets
    return aRect;
}

tools::Rectangle ScDetectiveFunc::GetDrawRect( SCCOL nCol, SCROW nRow ) const
{
    return GetDrawRect( nCol, nRow, nCol, nRow );
}

static bool lcl_IsOtherTab( const basegfx::B2DPolyPolygon& rPolyPolygon )
{
    //  test if rPolygon is the line end for "other table" (rectangle)
    if(1 == rPolyPolygon.count())
    {
        const basegfx::B2DPolygon& aSubPoly(rPolyPolygon.getB2DPolygon(0));

        // #i73305# circle consists of 4 segments, too, distinguishable from square by
        // the use of control points
        if(4 == aSubPoly.count() && aSubPoly.isClosed() && !aSubPoly.areControlPointsUsed())
        {
            return true;
        }
    }

    return false;
}

bool ScDetectiveFunc::HasArrow( const ScAddress& rStart,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab )
{
    bool bStartAlien = ( rStart.Tab() != nTab );
    bool bEndAlien   = ( nEndTab != nTab );

    if (bStartAlien && bEndAlien)
    {
        OSL_FAIL("bStartAlien && bEndAlien");
        return true;
    }

    tools::Rectangle aStartRect;
    tools::Rectangle aEndRect;
    if (!bStartAlien)
        aStartRect = GetDrawRect( rStart.Col(), rStart.Row() );
    if (!bEndAlien)
        aEndRect = GetDrawRect( nEndCol, nEndRow );

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");

    bool bFound = false;
    SdrObjListIter aIter( pPage, SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetLayer()==SC_LAYER_INTERN &&
                pObject->IsPolyObj() && pObject->GetPointCount()==2 )
        {
            const SfxItemSet& rSet = pObject->GetMergedItemSet();

            bool bObjStartAlien =
                lcl_IsOtherTab( rSet.Get(XATTR_LINESTART).GetLineStartValue() );
            bool bObjEndAlien =
                lcl_IsOtherTab( rSet.Get(XATTR_LINEEND).GetLineEndValue() );

            bool bStartHit = bStartAlien ? bObjStartAlien :
                                ( !bObjStartAlien && aStartRect.IsInside(pObject->GetPoint(0)) );
            bool bEndHit = bEndAlien ? bObjEndAlien :
                                ( !bObjEndAlien && aEndRect.IsInside(pObject->GetPoint(1)) );

            if ( bStartHit && bEndHit )
                bFound = true;
        }
        pObject = aIter.Next();
    }

    return bFound;
}

bool ScDetectiveFunc::IsNonAlienArrow( const SdrObject* pObject )
{
    if ( pObject->GetLayer()==SC_LAYER_INTERN &&
            pObject->IsPolyObj() && pObject->GetPointCount()==2 )
    {
        const SfxItemSet& rSet = pObject->GetMergedItemSet();

        bool bObjStartAlien =
            lcl_IsOtherTab( rSet.Get(XATTR_LINESTART).GetLineStartValue() );
        bool bObjEndAlien =
            lcl_IsOtherTab( rSet.Get(XATTR_LINEEND).GetLineEndValue() );

        return !bObjStartAlien && !bObjEndAlien;
    }

    return false;
}

//  InsertXXX: called from DrawEntry/DrawAlienEntry and InsertObject

void ScDetectiveFunc::InsertArrow( SCCOL nCol, SCROW nRow,
                                SCCOL nRefStartCol, SCROW nRefStartRow,
                                SCCOL nRefEndCol, SCROW nRefEndRow,
                                bool bFromOtherTab, bool bRed,
                                ScDetectiveData& rData )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));

    bool bArea = ( nRefStartCol != nRefEndCol || nRefStartRow != nRefEndRow );
    if (bArea && !bFromOtherTab)
    {
        // insert the rectangle before the arrow - this is relied on in FindFrameForObject

        tools::Rectangle aRect = GetDrawRect( nRefStartCol, nRefStartRow, nRefEndCol, nRefEndRow );
        SdrRectObj* pBox = new SdrRectObj(
            *pModel,
            aRect);

        pBox->SetMergedItemSetAndBroadcast(rData.GetBoxSet());

        pBox->SetLayer( SC_LAYER_INTERN );
        pPage->InsertObject( pBox );
        pModel->AddCalcUndo( std::make_unique<SdrUndoInsertObj>( *pBox ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pBox, true );
        pData->maStart.Set( nRefStartCol, nRefStartRow, nTab);
        pData->maEnd.Set( nRefEndCol, nRefEndRow, nTab);
    }

    Point aStartPos = GetDrawPos( nRefStartCol, nRefStartRow, DrawPosMode::DetectiveArrow );
    Point aEndPos = GetDrawPos( nCol, nRow, DrawPosMode::DetectiveArrow );

    if (bFromOtherTab)
    {
        bool bNegativePage = pDoc->IsNegativePage( nTab );
        long nPageSign = bNegativePage ? -1 : 1;

        aStartPos = Point( aEndPos.X() - 1000 * nPageSign, aEndPos.Y() - 1000 );
        if (aStartPos.X() * nPageSign < 0)
            aStartPos.AdjustX(2000 * nPageSign );
        if (aStartPos.Y() < 0)
            aStartPos.AdjustY(2000 );
    }

    SfxItemSet& rAttrSet = bFromOtherTab ? rData.GetFromTabSet() : rData.GetArrowSet();

    if (bArea && !bFromOtherTab)
        rAttrSet.Put( XLineWidthItem( 50 ) );               // range
    else
        rAttrSet.Put( XLineWidthItem( 0 ) );                // single reference

    Color nColor = ( bRed ? GetErrorColor() : GetArrowColor() );
    rAttrSet.Put( XLineColorItem( OUString(), nColor ) );

    basegfx::B2DPolygon aTempPoly;
    aTempPoly.append(basegfx::B2DPoint(aStartPos.X(), aStartPos.Y()));
    aTempPoly.append(basegfx::B2DPoint(aEndPos.X(), aEndPos.Y()));
    SdrPathObj* pArrow = new SdrPathObj(
        *pModel,
        OBJ_LINE,
        basegfx::B2DPolyPolygon(aTempPoly));
    pArrow->NbcSetLogicRect(tools::Rectangle(aStartPos,aEndPos));  //TODO: needed ???
    pArrow->SetMergedItemSetAndBroadcast(rAttrSet);

    pArrow->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pArrow );
    pModel->AddCalcUndo( std::make_unique<SdrUndoInsertObj>( *pArrow ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData(pArrow, true);
    if (bFromOtherTab)
        pData->maStart.SetInvalid();
    else
        pData->maStart.Set( nRefStartCol, nRefStartRow, nTab);

    pData->maEnd.Set( nCol, nRow, nTab);
    pData->meType = ScDrawObjData::DetectiveArrow;

    Modified();
}

void ScDetectiveFunc::InsertToOtherTab( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, bool bRed,
                                ScDetectiveData& rData )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));

    bool bArea = ( nStartCol != nEndCol || nStartRow != nEndRow );
    if (bArea)
    {
        tools::Rectangle aRect = GetDrawRect( nStartCol, nStartRow, nEndCol, nEndRow );
        SdrRectObj* pBox = new SdrRectObj(
            *pModel,
            aRect);

        pBox->SetMergedItemSetAndBroadcast(rData.GetBoxSet());

        pBox->SetLayer( SC_LAYER_INTERN );
        pPage->InsertObject( pBox );
        pModel->AddCalcUndo( std::make_unique<SdrUndoInsertObj>( *pBox ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pBox, true );
        pData->maStart.Set( nStartCol, nStartRow, nTab);
        pData->maEnd.Set( nEndCol, nEndRow, nTab);
    }

    bool bNegativePage = pDoc->IsNegativePage( nTab );
    long nPageSign = bNegativePage ? -1 : 1;

    Point aStartPos = GetDrawPos( nStartCol, nStartRow, DrawPosMode::DetectiveArrow );
    Point aEndPos   = Point( aStartPos.X() + 1000 * nPageSign, aStartPos.Y() - 1000 );
    if (aEndPos.Y() < 0)
        aEndPos.AdjustY(2000 );

    SfxItemSet& rAttrSet = rData.GetToTabSet();
    if (bArea)
        rAttrSet.Put( XLineWidthItem( 50 ) );               // range
    else
        rAttrSet.Put( XLineWidthItem( 0 ) );                // single reference

    Color nColor = ( bRed ? GetErrorColor() : GetArrowColor() );
    rAttrSet.Put( XLineColorItem( OUString(), nColor ) );

    basegfx::B2DPolygon aTempPoly;
    aTempPoly.append(basegfx::B2DPoint(aStartPos.X(), aStartPos.Y()));
    aTempPoly.append(basegfx::B2DPoint(aEndPos.X(), aEndPos.Y()));
    SdrPathObj* pArrow = new SdrPathObj(
        *pModel,
        OBJ_LINE,
        basegfx::B2DPolyPolygon(aTempPoly));
    pArrow->NbcSetLogicRect(tools::Rectangle(aStartPos,aEndPos));  //TODO: needed ???

    pArrow->SetMergedItemSetAndBroadcast(rAttrSet);

    pArrow->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pArrow );
    pModel->AddCalcUndo( std::make_unique<SdrUndoInsertObj>( *pArrow ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pArrow, true );
    pData->maStart.Set( nStartCol, nStartRow, nTab);
    pData->maEnd.SetInvalid();

    Modified();
}

//  DrawEntry:      formula from this spreadsheet,
//                  reference on this or other
//  DrawAlienEntry: formula from other spreadsheet,
//                  reference on this

//      return FALSE: there was already an arrow

bool ScDetectiveFunc::DrawEntry( SCCOL nCol, SCROW nRow,
                                    const ScRange& rRef,
                                    ScDetectiveData& rData )
{
    if ( HasArrow( rRef.aStart, nCol, nRow, nTab ) )
        return false;

    ScAddress aErrorPos;
    bool bError = HasError( rRef, aErrorPos );
    bool bAlien = ( rRef.aEnd.Tab() < nTab || rRef.aStart.Tab() > nTab );

    InsertArrow( nCol, nRow,
                 rRef.aStart.Col(), rRef.aStart.Row(),
                 rRef.aEnd.Col(), rRef.aEnd.Row(),
                 bAlien, bError, rData );
    return true;
}

bool ScDetectiveFunc::DrawAlienEntry( const ScRange& rRef,
                                        ScDetectiveData& rData )
{
    if ( HasArrow( rRef.aStart, 0, 0, nTab+1 ) )
        return false;

    ScAddress aErrorPos;
    bool bError = HasError( rRef, aErrorPos );

    InsertToOtherTab( rRef.aStart.Col(), rRef.aStart.Row(),
                      rRef.aEnd.Col(), rRef.aEnd.Row(),
                      bError, rData );
    return true;
}

void ScDetectiveFunc::DrawCircle( SCCOL nCol, SCROW nRow, ScDetectiveData& rData )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));

    tools::Rectangle aRect = GetDrawRect( nCol, nRow );
    aRect.AdjustLeft( -250 );
    aRect.AdjustRight(250 );
    aRect.AdjustTop( -70 );
    aRect.AdjustBottom(70 );

    SdrCircObj* pCircle = new SdrCircObj(
        *pModel,
        OBJ_CIRC,
        aRect);
    SfxItemSet& rAttrSet = rData.GetCircleSet();

    pCircle->SetMergedItemSetAndBroadcast(rAttrSet);

    pCircle->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pCircle );
    pModel->AddCalcUndo( std::make_unique<SdrUndoInsertObj>( *pCircle ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pCircle, true );
    pData->maStart.Set( nCol, nRow, nTab);
    pData->maEnd.SetInvalid();
    pData->meType = ScDrawObjData::ValidationCircle;

    Modified();
}

void ScDetectiveFunc::DeleteArrowsAt( SCCOL nCol, SCROW nRow, bool bDestPnt )
{
    tools::Rectangle aRect = GetDrawRect( nCol, nRow );

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");

    pPage->RecalcObjOrdNums();

    const size_t nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        size_t nDelCount = 0;
        std::unique_ptr<SdrObject*[]> ppObj(new SdrObject*[nObjCount]);

        SdrObjListIter aIter( pPage, SdrIterMode::Flat );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetLayer()==SC_LAYER_INTERN &&
                    pObject->IsPolyObj() && pObject->GetPointCount()==2 )
            {
                if (aRect.IsInside(pObject->GetPoint(bDestPnt ? 1 : 0))) // start/destinationpoint
                    ppObj[nDelCount++] = pObject;
            }

            pObject = aIter.Next();
        }

        const bool bRecording = pModel->IsRecording();

        if (bRecording)
        {
            for (size_t i=1; i<=nDelCount; ++i)
                pModel->AddCalcUndo(std::make_unique<SdrUndoDelObj>(*ppObj[nDelCount-i]));
        }

        for (size_t i=1; i<=nDelCount; ++i)
        {
            // remove the object from the drawing page, delete if undo is disabled
            SdrObject* pObj = pPage->RemoveObject(ppObj[nDelCount-i]->GetOrdNum());
            if( !bRecording )
                SdrObject::Free( pObj );
        }

        ppObj.reset();

        Modified();
    }
}

        //      delete box around reference

#define SC_DET_TOLERANCE    50

static bool RectIsPoints( const tools::Rectangle& rRect, const Point& rStart, const Point& rEnd )
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
    tools::Rectangle aCornerRect = GetDrawRect( nCol1, nRow1, nCol2, nRow2 );
    Point aStartCorner = aCornerRect.TopLeft();
    Point aEndCorner = aCornerRect.BottomRight();
    tools::Rectangle aObjRect;

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");

    pPage->RecalcObjOrdNums();

    const size_t nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        size_t nDelCount = 0;
        std::unique_ptr<SdrObject*[]> ppObj(new SdrObject*[nObjCount]);

        SdrObjListIter aIter( pPage, SdrIterMode::Flat );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetLayer() == SC_LAYER_INTERN &&
                    dynamic_cast< const SdrRectObj* >(pObject) != nullptr )
            {
                aObjRect = static_cast<SdrRectObj*>(pObject)->GetLogicRect();
                aObjRect.Justify();
                if ( RectIsPoints( aObjRect, aStartCorner, aEndCorner ) )
                    ppObj[nDelCount++] = pObject;
            }

            pObject = aIter.Next();
        }

        for (size_t i=1; i<=nDelCount; ++i)
            pModel->AddCalcUndo( std::make_unique<SdrUndoRemoveObj>( *ppObj[nDelCount-i] ) );

        for (size_t i=1; i<=nDelCount; ++i)
            pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );

        ppObj.reset();

        Modified();
    }
}

sal_uInt16 ScDetectiveFunc::InsertPredLevelArea( const ScRange& rRef,
                                        ScDetectiveData& rData, sal_uInt16 nLevel )
{
    sal_uInt16 nResult = DET_INS_EMPTY;

    ScCellIterator aIter( pDoc, rRef);
    for (bool bHasCell = aIter.first(); bHasCell; bHasCell = aIter.next())
    {
        if (aIter.getType() != CELLTYPE_FORMULA)
            continue;

        const ScAddress& rPos = aIter.GetPos();
        switch (InsertPredLevel(rPos.Col(), rPos.Row(), rData, nLevel))
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
            default:
                ;
        }
    }

    return nResult;
}

sal_uInt16 ScDetectiveFunc::InsertPredLevel( SCCOL nCol, SCROW nRow, ScDetectiveData& rData,
                                            sal_uInt16 nLevel )
{
    ScRefCellValue aCell(*pDoc, ScAddress(nCol, nRow, nTab));
    if (aCell.meType != CELLTYPE_FORMULA)
        return DET_INS_EMPTY;

    ScFormulaCell* pFCell = aCell.mpFormula;
    if (pFCell->IsRunning())
        return DET_INS_CIRCULAR;

    if (pFCell->GetDirty())
        pFCell->Interpret();                // can't be called after SetRunning
    pFCell->SetRunning(true);

    sal_uInt16 nResult = DET_INS_EMPTY;

    ScDetectiveRefIter aIter(pFCell);
    ScRange aRef;
    while ( aIter.GetNextRef( aRef ) )
    {
        if (DrawEntry( nCol, nRow, aRef, rData ))
        {
            nResult = DET_INS_INSERTED;         // insert new arrow
        }
        else
        {
            //  continue

            if ( nLevel < rData.GetMaxLevel() )
            {
                sal_uInt16 nSubResult;
                bool bArea = (aRef.aStart != aRef.aEnd);
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
                    // DET_INS_EMPTY: no change
                }
            }
            else                                    //  nMaxLevel reached
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
    for (bool bHasCell = aCellIter.first(); bHasCell; bHasCell = aCellIter.next())
    {
        if (aCellIter.getType() != CELLTYPE_FORMULA)
            continue;

        sal_uInt16 nTemp = FindPredLevel(aCellIter.GetPos().Col(), aCellIter.GetPos().Row(), nLevel, nDeleteLevel);
        if (nTemp > nResult)
            nResult = nTemp;
    }

    return nResult;
}

                                            //  nDeleteLevel != 0   -> delete

sal_uInt16 ScDetectiveFunc::FindPredLevel( SCCOL nCol, SCROW nRow, sal_uInt16 nLevel, sal_uInt16 nDeleteLevel )
{
    OSL_ENSURE( nLevel<1000, "Level" );

    ScRefCellValue aCell(*pDoc, ScAddress(nCol, nRow, nTab));
    if (aCell.meType != CELLTYPE_FORMULA)
        return nLevel;

    ScFormulaCell* pFCell = aCell.mpFormula;
    if (pFCell->IsRunning())
        return nLevel;

    if (pFCell->GetDirty())
        pFCell->Interpret();                // can't be called after SetRunning
    pFCell->SetRunning(true);

    sal_uInt16 nResult = nLevel;
    bool bDelete = ( nDeleteLevel && nLevel == nDeleteLevel-1 );

    if ( bDelete )
    {
        DeleteArrowsAt( nCol, nRow, true );                 // arrows, that are pointing here
    }

    ScDetectiveRefIter aIter(pFCell);
    ScRange aRef;
    while ( aIter.GetNextRef( aRef) )
    {
        bool bArea = ( aRef.aStart != aRef.aEnd );

        if ( bDelete )                  // delete frame ?
        {
            if (bArea)
            {
                DeleteBox( aRef.aStart.Col(), aRef.aStart.Row(), aRef.aEnd.Col(), aRef.aEnd.Row() );
            }
        }
        else                            // continue searching
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

sal_uInt16 ScDetectiveFunc::InsertErrorLevel( SCCOL nCol, SCROW nRow, ScDetectiveData& rData,
                                            sal_uInt16 nLevel )
{
    ScRefCellValue aCell(*pDoc, ScAddress(nCol, nRow, nTab));
    if (aCell.meType != CELLTYPE_FORMULA)
        return DET_INS_EMPTY;

    ScFormulaCell* pFCell = aCell.mpFormula;
    if (pFCell->IsRunning())
        return DET_INS_CIRCULAR;

    if (pFCell->GetDirty())
        pFCell->Interpret();                // can't be called after SetRunning
    pFCell->SetRunning(true);

    sal_uInt16 nResult = DET_INS_EMPTY;

    ScDetectiveRefIter aIter(pFCell);
    ScRange aRef;
    ScAddress aErrorPos;
    bool bHasError = false;
    while ( aIter.GetNextRef( aRef ) )
    {
        if (HasError( aRef, aErrorPos ))
        {
            bHasError = true;
            if (DrawEntry( nCol, nRow, ScRange( aErrorPos), rData ))
                nResult = DET_INS_INSERTED;

            if ( nLevel < rData.GetMaxLevel() )         // hits most of the time
            {
                if (InsertErrorLevel( aErrorPos.Col(), aErrorPos.Row(),
                                                        rData, nLevel+1 ) == DET_INS_INSERTED)
                    nResult = DET_INS_INSERTED;
            }
        }
    }

    pFCell->SetRunning(false);

                                                    // leaves ?
    if (!bHasError)
        if (InsertPredLevel( nCol, nRow, rData, rData.GetMaxLevel() ) == DET_INS_INSERTED)
            nResult = DET_INS_INSERTED;

    return nResult;
}

sal_uInt16 ScDetectiveFunc::InsertSuccLevel( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        ScDetectiveData& rData, sal_uInt16 nLevel )
{
    // over the entire document.

    sal_uInt16 nResult = DET_INS_EMPTY;
    ScCellIterator aCellIter(pDoc, ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB));  // all sheets
    for (bool bHas = aCellIter.first(); bHas; bHas = aCellIter.next())
    {
        if (aCellIter.getType() != CELLTYPE_FORMULA)
            continue;

        ScFormulaCell* pFCell = aCellIter.getFormulaCell();
        bool bRunning = pFCell->IsRunning();

        if (pFCell->GetDirty())
            pFCell->Interpret();                // can't be called after SetRunning
        pFCell->SetRunning(true);

        ScDetectiveRefIter aIter(pFCell);
        ScRange aRef;
        while ( aIter.GetNextRef( aRef) )
        {
            if (aRef.aStart.Tab() <= nTab && aRef.aEnd.Tab() >= nTab)
            {
                if (Intersect( nCol1,nRow1,nCol2,nRow2,
                        aRef.aStart.Col(),aRef.aStart.Row(),
                        aRef.aEnd.Col(),aRef.aEnd.Row() ))
                {
                    bool bAlien = ( aCellIter.GetPos().Tab() != nTab );
                    bool bDrawRet;
                    if (bAlien)
                        bDrawRet = DrawAlienEntry( aRef, rData );
                    else
                        bDrawRet = DrawEntry( aCellIter.GetPos().Col(), aCellIter.GetPos().Row(),
                                                aRef, rData );
                    if (bDrawRet)
                    {
                        nResult = DET_INS_INSERTED;         //  insert new arrow
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

                            if ( nLevel < rData.GetMaxLevel() )
                            {
                                sal_uInt16 nSubResult = InsertSuccLevel(
                                                        aCellIter.GetPos().Col(), aCellIter.GetPos().Row(),
                                                        aCellIter.GetPos().Col(), aCellIter.GetPos().Row(),
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
                                    // DET_INS_EMPTY: leave unchanged
                                }
                            }
                            else                                    //  nMaxLevel reached
                                if (nResult != DET_INS_INSERTED)
                                    nResult = DET_INS_CONTINUE;
                        }
                    }
                }
            }
        }
        pFCell->SetRunning(bRunning);
    }

    return nResult;
}

sal_uInt16 ScDetectiveFunc::FindSuccLevel( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        sal_uInt16 nLevel, sal_uInt16 nDeleteLevel )
{
    OSL_ENSURE( nLevel<1000, "Level" );

    sal_uInt16 nResult = nLevel;
    bool bDelete = ( nDeleteLevel && nLevel == nDeleteLevel-1 );

    ScCellIterator aCellIter( pDoc, ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab) );
    for (bool bHas = aCellIter.first(); bHas; bHas = aCellIter.next())
    {
        if (aCellIter.getType() != CELLTYPE_FORMULA)
            continue;

        ScFormulaCell* pFCell = aCellIter.getFormulaCell();
        bool bRunning = pFCell->IsRunning();

        if (pFCell->GetDirty())
            pFCell->Interpret();                // can't be called after SetRunning
        pFCell->SetRunning(true);

        ScDetectiveRefIter aIter(pFCell);
        ScRange aRef;
        while ( aIter.GetNextRef( aRef) )
        {
            if (aRef.aStart.Tab() <= nTab && aRef.aEnd.Tab() >= nTab)
            {
                if (Intersect( nCol1,nRow1,nCol2,nRow2,
                        aRef.aStart.Col(),aRef.aStart.Row(),
                        aRef.aEnd.Col(),aRef.aEnd.Row() ))
                {
                    if ( bDelete )                          // arrows, that are starting here
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
                                        aCellIter.GetPos().Col(),aCellIter.GetPos().Row(),aCellIter.GetPos().Tab() ) )
                    {
                        sal_uInt16 nTemp = FindSuccLevel( aCellIter.GetPos().Col(), aCellIter.GetPos().Row(),
                                                        aCellIter.GetPos().Col(), aCellIter.GetPos().Row(),
                                                        nLevel+1, nDeleteLevel );
                        if (nTemp > nResult)
                            nResult = nTemp;
                    }
                }
            }
        }

        pFCell->SetRunning(bRunning);
    }

    return nResult;
}

bool ScDetectiveFunc::ShowPred( SCCOL nCol, SCROW nRow )
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

bool ScDetectiveFunc::ShowSucc( SCCOL nCol, SCROW nRow )
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

bool ScDetectiveFunc::ShowError( SCCOL nCol, SCROW nRow )
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

bool ScDetectiveFunc::DeleteSucc( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    sal_uInt16 nLevelCount = FindSuccLevel( nCol, nRow, nCol, nRow, 0, 0 );
    if ( nLevelCount )
        FindSuccLevel( nCol, nRow, nCol, nRow, 0, nLevelCount );            // delete

    return ( nLevelCount != 0 );
}

bool ScDetectiveFunc::DeletePred( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    sal_uInt16 nLevelCount = FindPredLevel( nCol, nRow, 0, 0 );
    if ( nLevelCount )
        FindPredLevel( nCol, nRow, 0, nLevelCount );            // delete

    return ( nLevelCount != 0 );
}

bool ScDetectiveFunc::DeleteAll( ScDetectiveDelete eWhat )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");

    pPage->RecalcObjOrdNums();

    size_t nDelCount = 0;
    const size_t nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        std::unique_ptr<SdrObject*[]> ppObj(new SdrObject*[nObjCount]);

        SdrObjListIter aIter( pPage, SdrIterMode::Flat );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetLayer() == SC_LAYER_INTERN )
            {
                bool bDoThis = true;
                bool bCircle = ( dynamic_cast<const SdrCircObj*>( pObject) !=  nullptr );
                bool bCaption = ScDrawLayer::IsNoteCaption( pObject );
                if ( eWhat == ScDetectiveDelete::Detective )      // detective, from menu
                    bDoThis = !bCaption;                          // also circles
                else if ( eWhat == ScDetectiveDelete::Circles )   // circles, if new created
                    bDoThis = bCircle;
                else if ( eWhat == ScDetectiveDelete::Arrows )    // DetectiveRefresh
                    bDoThis = !bCaption && !bCircle;              // don't include circles
                else
                {
                    OSL_FAIL("what?");
                }
                if ( bDoThis )
                    ppObj[nDelCount++] = pObject;
            }

            pObject = aIter.Next();
        }

        for (size_t i=1; i<=nDelCount; ++i)
            pModel->AddCalcUndo( std::make_unique<SdrUndoRemoveObj>( *ppObj[nDelCount-i] ) );

        for (size_t i=1; i<=nDelCount; ++i)
            pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );

        ppObj.reset();

        Modified();
    }

    return ( nDelCount != 0 );
}

bool ScDetectiveFunc::MarkInvalid(bool& rOverflow)
{
    rOverflow = false;
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    bool bDeleted = DeleteAll( ScDetectiveDelete::Circles );        // just circles

    ScDetectiveData aData( pModel );
    long nInsCount = 0;

    //  search for valid places
    ScDocAttrIterator aAttrIter( pDoc, nTab, 0,0,MAXCOL,MAXROW );
    SCCOL nCol;
    SCROW nRow1;
    SCROW nRow2;
    const ScPatternAttr* pPattern = aAttrIter.GetNext( nCol, nRow1, nRow2 );
    while ( pPattern && nInsCount < SC_DET_MAXCIRCLE )
    {
        sal_uLong nIndex = pPattern->GetItem(ATTR_VALIDDATA).GetValue();
        if (nIndex)
        {
            const ScValidationData* pData = pDoc->GetValidationEntry( nIndex );
            if ( pData )
            {
                //  pass cells in this area

                bool bMarkEmpty = !pData->IsIgnoreBlank();
                SCROW nNextRow = nRow1;
                SCROW nRow;
                ScCellIterator aCellIter( pDoc, ScRange(nCol, nRow1, nTab, nCol, nRow2, nTab) );
                for (bool bHas = aCellIter.first(); bHas && nInsCount < SC_DET_MAXCIRCLE; bHas = aCellIter.next())
                {
                    SCROW nCellRow = aCellIter.GetPos().Row();
                    if ( bMarkEmpty )
                        for ( nRow = nNextRow; nRow < nCellRow && nInsCount < SC_DET_MAXCIRCLE; nRow++ )
                        {
                            DrawCircle( nCol, nRow, aData );
                            ++nInsCount;
                        }
                    ScRefCellValue aCell = aCellIter.getRefCellValue();
                    if (!pData->IsDataValid(aCell, aCellIter.GetPos()))
                    {
                        DrawCircle( nCol, nCellRow, aData );
                        ++nInsCount;
                    }
                    nNextRow = nCellRow + 1;
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
        rOverflow = true;

    return ( bDeleted || nInsCount != 0 );
}

void ScDetectiveFunc::GetAllPreds(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                  vector<ScTokenRef>& rRefTokens)
{
    ScCellIterator aIter(pDoc, ScRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab));
    for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
    {
        if (aIter.getType() != CELLTYPE_FORMULA)
            continue;

        ScFormulaCell* pFCell = aIter.getFormulaCell();
        ScDetectiveRefIter aRefIter(pFCell);
        for (formula::FormulaToken* p = aRefIter.GetNextRefToken(); p; p = aRefIter.GetNextRefToken())
        {
            ScTokenRef pRef(p->Clone());
            ScRefTokenHelper::join(rRefTokens, pRef, aIter.GetPos());
        }
    }
}

void ScDetectiveFunc::GetAllSuccs(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                  vector<ScTokenRef>& rRefTokens)
{
    vector<ScTokenRef> aSrcRange;
    aSrcRange.push_back(
        ScRefTokenHelper::createRefToken(ScRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab)));

    ScCellIterator aIter(pDoc, ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab));
    for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
    {
        if (aIter.getType() != CELLTYPE_FORMULA)
            continue;

        ScFormulaCell* pFCell = aIter.getFormulaCell();
        ScDetectiveRefIter aRefIter(pFCell);
        for (formula::FormulaToken* p = aRefIter.GetNextRefToken(); p; p = aRefIter.GetNextRefToken())
        {
            const ScAddress& aPos = aIter.GetPos();
            ScTokenRef pRef(p->Clone());
            if (ScRefTokenHelper::intersects(aSrcRange, pRef, aPos))
            {
                // This address is absolute.
                pRef = ScRefTokenHelper::createRefToken(aPos);
                ScRefTokenHelper::join(rRefTokens, pRef, ScAddress());
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
        SdrPage* pPage = pModel->GetPage( static_cast< sal_uInt16 >( nObjTab ) );
        OSL_ENSURE( pPage, "Page ?" );
        if( pPage )
        {
            SdrObjListIter aIter( pPage, SdrIterMode::Flat );
            for( SdrObject* pObject = aIter.Next(); pObject; pObject = aIter.Next() )
            {
                if ( ScDrawObjData* pData = ScDrawLayer::GetNoteCaptionData( pObject, nObjTab ) )
                {
                    ScPostIt* pNote = rDoc.GetNote( pData->maStart );
                    // caption should exist, we iterate over drawing objects...
                    OSL_ENSURE( pNote && (pNote->GetCaption() == pObject), "ScDetectiveFunc::UpdateAllComments - invalid cell note" );
                    if( pNote )
                    {
                        ScCommentData aData( rDoc, pModel );
                        SfxItemSet aAttrColorSet = pObject->GetMergedItemSet();
                        aAttrColorSet.Put( XFillColorItem( OUString(), GetCommentColor() ) );
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
            SdrObjListIter aIter( pPage, SdrIterMode::Flat );
            for( SdrObject* pObject = aIter.Next(); pObject; pObject = aIter.Next() )
            {
                if ( pObject->GetLayer() == SC_LAYER_INTERN )
                {
                    bool bArrow = false;
                    bool bError = false;

                    ScAddress aPos;
                    ScRange aSource;
                    bool bDummy;
                    ScDetectiveObjType eType = GetDetectiveObjectType( pObject, nObjTab, aPos, aSource, bDummy );
                    if ( eType == SC_DETOBJ_ARROW || eType == SC_DETOBJ_TOOTHERTAB )
                    {
                        //  source is valid, determine error flag from source range

                        ScAddress aErrPos;
                        if ( HasError( aSource, aErrPos ) )
                            bError = true;
                        else
                            bArrow = true;
                    }
                    else if ( eType == SC_DETOBJ_FROMOTHERTAB )
                    {
                        //  source range is no longer known, take error flag from formula itself
                        //  (this means, if the formula has an error, all references to other tables
                        //  are marked red)

                        ScAddress aErrPos;
                        if ( HasError( ScRange( aPos), aErrPos ) )
                            bError = true;
                        else
                            bArrow = true;
                    }
                    else if ( eType == SC_DETOBJ_CIRCLE )
                    {
                        //  circles (error marks) are always red

                        bError = true;
                    }
                    else if ( eType == SC_DETOBJ_NONE )
                    {
                        //  frame for area reference has no ObjType, always gets arrow color

                        if ( dynamic_cast<const SdrRectObj*>( pObject) != nullptr && dynamic_cast<const SdrCaptionObj*>( pObject) ==  nullptr )
                        {
                            bArrow = true;
                        }
                    }

                    if ( bArrow || bError )
                    {
                        Color nColor = ( bError ? GetErrorColor() : GetArrowColor() );
                        pObject->SetMergedItem( XLineColorItem( OUString(), nColor ) );

                        // repaint only
                        pObject->ActionChanged();
                    }
                }
            }
        }
    }
}

void ScDetectiveFunc::FindFrameForObject( const SdrObject* pObject, ScRange& rRange )
{
    //  find the rectangle for an arrow (always the object directly before the arrow)
    //  rRange must be initialized to the source cell of the arrow (start of area)

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel) return;

    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");
    if (!pPage) return;

    // test if the object is a direct page member
    if( pObject && pObject->getSdrPageFromSdrObject() && (pObject->getSdrPageFromSdrObject() == pObject->getParentSdrObjListFromSdrObject()->getSdrPageFromSdrObjList()) )
    {
        // Is there a previous object?
        const size_t nOrdNum = pObject->GetOrdNum();

        if(nOrdNum > 0)
        {
            SdrObject* pPrevObj = pPage->GetObj(nOrdNum - 1);

            if ( pPrevObj && pPrevObj->GetLayer() == SC_LAYER_INTERN && dynamic_cast<const SdrRectObj*>( pPrevObj) !=  nullptr )
            {
                ScDrawObjData* pPrevData = ScDrawLayer::GetObjDataTab( pPrevObj, rRange.aStart.Tab() );
                if ( pPrevData && pPrevData->maStart.IsValid() && pPrevData->maEnd.IsValid() && (pPrevData->maStart == rRange.aStart) )
                {
                    rRange.aEnd = pPrevData->maEnd;
                    return;
                }
            }
        }
    }
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

                Color nObjColor = pObject->GetMergedItem(XATTR_LINECOLOR).GetColorValue();
                if ( nObjColor == GetErrorColor() && nObjColor != GetArrowColor() )
                    rRedLine = true;
            }
            else if ( dynamic_cast<const SdrCircObj*>( pObject) !=  nullptr )
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
                            bool bRedLine )
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

Color ScDetectiveFunc::GetArrowColor()
{
    if (!bColorsInitialized)
        InitializeColors();
    return nArrowColor;
}

Color ScDetectiveFunc::GetErrorColor()
{
    if (!bColorsInitialized)
        InitializeColors();
    return nErrorColor;
}

Color ScDetectiveFunc::GetCommentColor()
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

    bColorsInitialized = true;
}

bool ScDetectiveFunc::IsColorsInitialized()
{
    return bColorsInitialized;
}

void ScDetectiveFunc::AppendChangTrackNoteSeparator(OUString &rDisplay)
{
    rDisplay += "\n--------\n";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
