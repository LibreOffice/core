/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
#include "formulacell.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "scmod.hxx"
#include "postit.hxx"
#include "rangelst.hxx"
#include "reftokenhelper.hxx"
#include "formulaiter.hxx"
#include "cellvalue.hxx"

#include <vector>

using ::std::vector;



#define SC_LINEEND_NAME     EMPTY_OUSTRING

enum DetInsertResult {              
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
    SfxItemSet  aCircleSet;         
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

ColorData ScDetectiveFunc::nArrowColor = 0;
ColorData ScDetectiveFunc::nErrorColor = 0;
ColorData ScDetectiveFunc::nCommentColor = 0;
bool ScDetectiveFunc::bColorsInitialized = false;

static bool lcl_HasThickLine( SdrObject& rObj )
{
    

    return ( ((const XLineWidthItem&)rObj.GetMergedItem(XATTR_LINEWIDTH)).GetValue() > 0 );
}

ScDetectiveData::ScDetectiveData( SdrModel* pModel ) :
    aBoxSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aArrowSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aToTabSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aFromTabSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END ),
    aCircleSet( pModel->GetItemPool(), SDRATTR_START, SDRATTR_END )
{
    nMaxLevel = 0;

    aBoxSet.Put( XLineColorItem( EMPTY_OUSTRING, Color( ScDetectiveFunc::GetArrowColor() ) ) );
    aBoxSet.Put( XFillStyleItem( XFILL_NONE ) );

    
    

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

    OUString aName = SC_LINEEND_NAME;

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

    aCircleSet.Put( XLineColorItem( OUString(), Color( ScDetectiveFunc::GetErrorColor() ) ) );
    aCircleSet.Put( XFillStyleItem( XFILL_NONE ) );
    sal_uInt16 nWidth = 55;     
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

    OUString aName = SC_LINEEND_NAME;

    aCaptionSet.Put( XLineStartItem( aName, basegfx::B2DPolyPolygon(aTriangle) ) );
    aCaptionSet.Put( XLineStartWidthItem( 200 ) );
    aCaptionSet.Put( XLineStartCenterItem( false ) );
    aCaptionSet.Put( XFillStyleItem( XFILL_SOLID ) );
    Color aYellow( ScDetectiveFunc::GetCommentColor() );
    aCaptionSet.Put( XFillColorItem( OUString(), aYellow ) );

    
    
    
    
    aCaptionSet.Put( SdrShadowItem( false ) );
    aCaptionSet.Put( SdrShadowXDistItem( 100 ) );
    aCaptionSet.Put( SdrShadowYDistItem( 100 ) );

    
    aCaptionSet.Put( SdrTextLeftDistItem( 100 ) );
    aCaptionSet.Put( SdrTextRightDistItem( 100 ) );
    aCaptionSet.Put( SdrTextUpperDistItem( 100 ) );
    aCaptionSet.Put( SdrTextLowerDistItem( 100 ) );

    aCaptionSet.Put( SdrTextAutoGrowWidthItem( false ) );
    aCaptionSet.Put( SdrTextAutoGrowHeightItem( sal_True ) );

    
    
    ((const ScPatternAttr&)rDoc.GetPool()->GetDefaultItem(ATTR_PATTERN)).
        FillEditItemSet( &aCaptionSet );

    
    
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
                    
                    
                    
                break;
                case SDRATTR_SHADOWXDIST:
                    
                    
                break;
                case SDRATTR_SHADOWYDIST:
                    
                    
                break;

                default:
                    aCaptionSet.Put(*pPoolItem);
           }
        }
    }
}

void ScDetectiveFunc::Modified()
{
    if (pDoc->IsStreamValid(nTab))
        pDoc->SetStreamValid(nTab, false);
}

inline bool Intersect( SCCOL nStartCol1, SCROW nStartRow1, SCCOL nEndCol1, SCROW nEndRow1,
                        SCCOL nStartCol2, SCROW nStartRow2, SCCOL nEndCol2, SCROW nEndRow2 )
{
    return nEndCol1 >= nStartCol2 && nEndCol2 >= nStartCol1 &&
            nEndRow1 >= nStartRow2 && nEndRow2 >= nStartRow1;
}

bool ScDetectiveFunc::HasError( const ScRange& rRange, ScAddress& rErrPos )
{
    rErrPos = rRange.aStart;
    sal_uInt16 nError = 0;

    ScCellIterator aIter( pDoc, rRange);
    for (bool bHasCell = aIter.first(); bHasCell; bHasCell = aIter.next())
    {
        if (aIter.getType() != CELLTYPE_FORMULA)
            continue;

        nError = aIter.getFormulaCell()->GetErrCode();
        if (nError)
            rErrPos = aIter.GetPos();
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
    aRect.Justify();    
    return aRect;
}

Rectangle ScDetectiveFunc::GetDrawRect( SCCOL nCol, SCROW nRow ) const
{
    return GetDrawRect( nCol, nRow, nCol, nRow );
}

static bool lcl_IsOtherTab( const basegfx::B2DPolyPolygon& rPolyPolygon )
{
    
    if(1L == rPolyPolygon.count())
    {
        const basegfx::B2DPolygon aSubPoly(rPolyPolygon.getB2DPolygon(0L));

        
        
        if(4L == aSubPoly.count() && aSubPoly.isClosed() && !aSubPoly.areControlPointsUsed())
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

    Rectangle aStartRect;
    Rectangle aEndRect;
    if (!bStartAlien)
        aStartRect = GetDrawRect( rStart.Col(), rStart.Row() );
    if (!bEndAlien)
        aEndRect = GetDrawRect( nEndCol, nEndRow );

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");

    bool bFound = false;
    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetLayer()==SC_LAYER_INTERN &&
                pObject->IsPolyObj() && pObject->GetPointCount()==2 )
        {
            const SfxItemSet& rSet = pObject->GetMergedItemSet();

            bool bObjStartAlien =
                lcl_IsOtherTab( ((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).GetLineStartValue() );
            bool bObjEndAlien =
                lcl_IsOtherTab( ((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).GetLineEndValue() );

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

bool ScDetectiveFunc::IsNonAlienArrow( SdrObject* pObject )
{
    if ( pObject->GetLayer()==SC_LAYER_INTERN &&
            pObject->IsPolyObj() && pObject->GetPointCount()==2 )
    {
        const SfxItemSet& rSet = pObject->GetMergedItemSet();

        bool bObjStartAlien =
            lcl_IsOtherTab( ((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).GetLineStartValue() );
        bool bObjEndAlien =
            lcl_IsOtherTab( ((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).GetLineEndValue() );

        return !bObjStartAlien && !bObjEndAlien;
    }

    return false;
}



bool ScDetectiveFunc::InsertArrow( SCCOL nCol, SCROW nRow,
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
        

        Rectangle aRect = GetDrawRect( nRefStartCol, nRefStartRow, nRefEndCol, nRefEndRow );
        SdrRectObj* pBox = new SdrRectObj( aRect );

        pBox->SetMergedItemSetAndBroadcast(rData.GetBoxSet());

        pBox->SetLayer( SC_LAYER_INTERN );
        pPage->InsertObject( pBox );
        pModel->AddCalcUndo( new SdrUndoInsertObj( *pBox ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pBox, true );
        pData->maStart.Set( nRefStartCol, nRefStartRow, nTab);
        pData->maEnd.Set( nRefEndCol, nRefEndRow, nTab);
    }

    Point aStartPos = GetDrawPos( nRefStartCol, nRefStartRow, DRAWPOS_DETARROW );
    Point aEndPos = GetDrawPos( nCol, nRow, DRAWPOS_DETARROW );

    if (bFromOtherTab)
    {
        bool bNegativePage = pDoc->IsNegativePage( nTab );
        long nPageSign = bNegativePage ? -1 : 1;

        aStartPos = Point( aEndPos.X() - 1000 * nPageSign, aEndPos.Y() - 1000 );
        if (aStartPos.X() * nPageSign < 0)
            aStartPos.X() += 2000 * nPageSign;
        if (aStartPos.Y() < 0)
            aStartPos.Y() += 2000;
    }

    SfxItemSet& rAttrSet = bFromOtherTab ? rData.GetFromTabSet() : rData.GetArrowSet();

    if (bArea && !bFromOtherTab)
        rAttrSet.Put( XLineWidthItem( 50 ) );               
    else
        rAttrSet.Put( XLineWidthItem( 0 ) );                

    ColorData nColorData = ( bRed ? GetErrorColor() : GetArrowColor() );
    rAttrSet.Put( XLineColorItem( OUString(), Color( nColorData ) ) );

    basegfx::B2DPolygon aTempPoly;
    aTempPoly.append(basegfx::B2DPoint(aStartPos.X(), aStartPos.Y()));
    aTempPoly.append(basegfx::B2DPoint(aEndPos.X(), aEndPos.Y()));
    SdrPathObj* pArrow = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aTempPoly));
    pArrow->NbcSetLogicRect(Rectangle(aStartPos,aEndPos));  
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

bool ScDetectiveFunc::InsertToOtherTab( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, bool bRed,
                                ScDetectiveData& rData )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));

    bool bArea = ( nStartCol != nEndCol || nStartRow != nEndRow );
    if (bArea)
    {
        Rectangle aRect = GetDrawRect( nStartCol, nStartRow, nEndCol, nEndRow );
        SdrRectObj* pBox = new SdrRectObj( aRect );

        pBox->SetMergedItemSetAndBroadcast(rData.GetBoxSet());

        pBox->SetLayer( SC_LAYER_INTERN );
        pPage->InsertObject( pBox );
        pModel->AddCalcUndo( new SdrUndoInsertObj( *pBox ) );

        ScDrawObjData* pData = ScDrawLayer::GetObjData( pBox, true );
        pData->maStart.Set( nStartCol, nStartRow, nTab);
        pData->maEnd.Set( nEndCol, nEndRow, nTab);
    }

    bool bNegativePage = pDoc->IsNegativePage( nTab );
    long nPageSign = bNegativePage ? -1 : 1;

    Point aStartPos = GetDrawPos( nStartCol, nStartRow, DRAWPOS_DETARROW );
    Point aEndPos   = Point( aStartPos.X() + 1000 * nPageSign, aStartPos.Y() - 1000 );
    if (aEndPos.Y() < 0)
        aEndPos.Y() += 2000;

    SfxItemSet& rAttrSet = rData.GetToTabSet();
    if (bArea)
        rAttrSet.Put( XLineWidthItem( 50 ) );               
    else
        rAttrSet.Put( XLineWidthItem( 0 ) );                

    ColorData nColorData = ( bRed ? GetErrorColor() : GetArrowColor() );
    rAttrSet.Put( XLineColorItem( OUString(), Color( nColorData ) ) );

    basegfx::B2DPolygon aTempPoly;
    aTempPoly.append(basegfx::B2DPoint(aStartPos.X(), aStartPos.Y()));
    aTempPoly.append(basegfx::B2DPoint(aEndPos.X(), aEndPos.Y()));
    SdrPathObj* pArrow = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aTempPoly));
    pArrow->NbcSetLogicRect(Rectangle(aStartPos,aEndPos));  

    pArrow->SetMergedItemSetAndBroadcast(rAttrSet);

    pArrow->SetLayer( SC_LAYER_INTERN );
    pPage->InsertObject( pArrow );
    pModel->AddCalcUndo( new SdrUndoInsertObj( *pArrow ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pArrow, true );
    pData->maStart.Set( nStartCol, nStartRow, nTab);
    pData->maEnd.SetInvalid();

    Modified();
    return true;
}








bool ScDetectiveFunc::DrawEntry( SCCOL nCol, SCROW nRow,
                                    const ScRange& rRef,
                                    ScDetectiveData& rData )
{
    if ( HasArrow( rRef.aStart, nCol, nRow, nTab ) )
        return false;

    ScAddress aErrorPos;
    bool bError = HasError( rRef, aErrorPos );
    bool bAlien = ( rRef.aEnd.Tab() < nTab || rRef.aStart.Tab() > nTab );

    return InsertArrow( nCol, nRow,
                        rRef.aStart.Col(), rRef.aStart.Row(),
                        rRef.aEnd.Col(), rRef.aEnd.Row(),
                        bAlien, bError, rData );
}

bool ScDetectiveFunc::DrawAlienEntry( const ScRange& rRef,
                                        ScDetectiveData& rData )
{
    if ( HasArrow( rRef.aStart, 0, 0, nTab+1 ) )
        return false;

    ScAddress aErrorPos;
    bool bError = HasError( rRef, aErrorPos );

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

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pCircle, true );
    pData->maStart.Set( nCol, nRow, nTab);
    pData->maEnd.SetInvalid();
    pData->meType = ScDrawObjData::ValidationCircle;

    Modified();
}

void ScDetectiveFunc::DeleteArrowsAt( SCCOL nCol, SCROW nRow, bool bDestPnt )
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
                if (aRect.IsInside(pObject->GetPoint(bDestPnt ? 1 : 0))) 
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

        

#define SC_DET_TOLERANCE    50

inline bool RectIsPoints( const Rectangle& rRect, const Point& rStart, const Point& rEnd )
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
    ScRefCellValue aCell;
    aCell.assign(*pDoc, ScAddress(nCol, nRow, nTab));
    if (aCell.meType != CELLTYPE_FORMULA)
        return DET_INS_EMPTY;

    ScFormulaCell* pFCell = aCell.mpFormula;
    if (pFCell->IsRunning())
        return DET_INS_CIRCULAR;

    if (pFCell->GetDirty())
        pFCell->Interpret();                
    pFCell->SetRunning(true);

    sal_uInt16 nResult = DET_INS_EMPTY;

    ScDetectiveRefIter aIter(pFCell);
    ScRange aRef;
    while ( aIter.GetNextRef( aRef ) )
    {
        if (DrawEntry( nCol, nRow, aRef, rData ))
        {
            nResult = DET_INS_INSERTED;         
        }
        else
        {
            

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
                    
                }
            }
            else                                    
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

                                            

sal_uInt16 ScDetectiveFunc::FindPredLevel( SCCOL nCol, SCROW nRow, sal_uInt16 nLevel, sal_uInt16 nDeleteLevel )
{
    OSL_ENSURE( nLevel<1000, "Level" );

    ScRefCellValue aCell;
    aCell.assign(*pDoc, ScAddress(nCol, nRow, nTab));
    if (aCell.meType != CELLTYPE_FORMULA)
        return nLevel;

    ScFormulaCell* pFCell = aCell.mpFormula;
    if (pFCell->IsRunning())
        return nLevel;

    if (pFCell->GetDirty())
        pFCell->Interpret();                
    pFCell->SetRunning(true);

    sal_uInt16 nResult = nLevel;
    bool bDelete = ( nDeleteLevel && nLevel == nDeleteLevel-1 );

    if ( bDelete )
    {
        DeleteArrowsAt( nCol, nRow, true );                 
    }

    ScDetectiveRefIter aIter(pFCell);
    ScRange aRef;
    while ( aIter.GetNextRef( aRef) )
    {
        bool bArea = ( aRef.aStart != aRef.aEnd );

        if ( bDelete )                  
        {
            if (bArea)
            {
                DeleteBox( aRef.aStart.Col(), aRef.aStart.Row(), aRef.aEnd.Col(), aRef.aEnd.Row() );
            }
        }
        else                            
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
    ScRefCellValue aCell;
    aCell.assign(*pDoc, ScAddress(nCol, nRow, nTab));
    if (aCell.meType != CELLTYPE_FORMULA)
        return DET_INS_EMPTY;

    ScFormulaCell* pFCell = aCell.mpFormula;
    if (pFCell->IsRunning())
        return DET_INS_CIRCULAR;

    if (pFCell->GetDirty())
        pFCell->Interpret();                
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

            

            if ( nLevel < rData.GetMaxLevel() )         
            {
                if (InsertErrorLevel( aErrorPos.Col(), aErrorPos.Row(),
                                                        rData, nLevel+1 ) == DET_INS_INSERTED)
                    nResult = DET_INS_INSERTED;
            }
        }
    }

    pFCell->SetRunning(false);

                                                    
    if (!bHasError)
        if (InsertPredLevel( nCol, nRow, rData, rData.GetMaxLevel() ) == DET_INS_INSERTED)
            nResult = DET_INS_INSERTED;

    return nResult;
}

sal_uInt16 ScDetectiveFunc::InsertSuccLevel( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        ScDetectiveData& rData, sal_uInt16 nLevel )
{
    

    sal_uInt16 nResult = DET_INS_EMPTY;
    ScCellIterator aCellIter(pDoc, ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB));  
    for (bool bHas = aCellIter.first(); bHas; bHas = aCellIter.next())
    {
        if (aCellIter.getType() != CELLTYPE_FORMULA)
            continue;

        ScFormulaCell* pFCell = aCellIter.getFormulaCell();
        bool bRunning = pFCell->IsRunning();

        if (pFCell->GetDirty())
            pFCell->Interpret();                
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
                        nResult = DET_INS_INSERTED;         
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
                                    
                                }
                            }
                            else                                    
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
            pFCell->Interpret();                
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
                    if ( bDelete )                          
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

//

//

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
        FindSuccLevel( nCol, nRow, nCol, nRow, 0, nLevelCount );            

    return ( nLevelCount != 0 );
}

bool ScDetectiveFunc::DeletePred( SCCOL nCol, SCROW nRow )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    sal_uInt16 nLevelCount = FindPredLevel( nCol, nRow, 0, 0 );
    if ( nLevelCount )
        FindPredLevel( nCol, nRow, 0, nLevelCount );            

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
                bool bDoThis = true;
                if ( eWhat != SC_DET_ALL )
                {
                    bool bCircle = ( pObject->ISA(SdrCircObj) );
                    bool bCaption = ScDrawLayer::IsNoteCaption( pObject );
                    if ( eWhat == SC_DET_DETECTIVE )        
                        bDoThis = !bCaption;                
                    else if ( eWhat == SC_DET_CIRCLES )     
                        bDoThis = bCircle;
                    else if ( eWhat == SC_DET_ARROWS )      
                        bDoThis = !bCaption && !bCircle;    
                    else
                    {
                        OSL_FAIL("what?");
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

bool ScDetectiveFunc::MarkInvalid(bool& rOverflow)
{
    rOverflow = false;
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return false;

    bool bDeleted = DeleteAll( SC_DET_CIRCLES );        

    ScDetectiveData aData( pModel );
    long nInsCount = 0;

    

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
        for (ScToken* p = aRefIter.GetNextRefToken(); p; p = aRefIter.GetNextRefToken())
        {
            ScTokenRef pRef(static_cast<ScToken*>(p->Clone()));
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
        for (ScToken* p = aRefIter.GetNextRefToken(); p; p = aRefIter.GetNextRefToken())
        {
            const ScAddress& aPos = aIter.GetPos();
            ScTokenRef pRef(static_cast<ScToken*>(p->Clone()));
            if (ScRefTokenHelper::intersects(aSrcRange, pRef, aPos))
            {
                
                pRef = ScRefTokenHelper::createRefToken(aPos);
                ScRefTokenHelper::join(rRefTokens, pRef, ScAddress());
            }
        }
    }
}

void ScDetectiveFunc::UpdateAllComments( ScDocument& rDoc )
{
    
    

    

    ScDrawLayer* pModel = rDoc.GetDrawLayer();
    if (!pModel)
        return;

    for( SCTAB nObjTab = 0, nTabCount = rDoc.GetTableCount(); nObjTab < nTabCount; ++nObjTab )
    {
        SdrPage* pPage = pModel->GetPage( static_cast< sal_uInt16 >( nObjTab ) );
        OSL_ENSURE( pPage, "Page ?" );
        if( pPage )
        {
            SdrObjListIter aIter( *pPage, IM_FLAT );
            for( SdrObject* pObject = aIter.Next(); pObject; pObject = aIter.Next() )
            {
                if ( ScDrawObjData* pData = ScDrawLayer::GetNoteCaptionData( pObject, nObjTab ) )
                {
                    ScPostIt* pNote = rDoc.GetNote( pData->maStart );
                    
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
                    bool bArrow = false;
                    bool bError = false;

                    ScAddress aPos;
                    ScRange aSource;
                    bool bDummy;
                    ScDetectiveObjType eType = GetDetectiveObjectType( pObject, nObjTab, aPos, aSource, bDummy );
                    if ( eType == SC_DETOBJ_ARROW || eType == SC_DETOBJ_TOOTHERTAB )
                    {
                        

                        ScAddress aErrPos;
                        if ( HasError( aSource, aErrPos ) )
                            bError = true;
                        else
                            bArrow = true;
                    }
                    else if ( eType == SC_DETOBJ_FROMOTHERTAB )
                    {
                        
                        
                        

                        ScAddress aErrPos;
                        if ( HasError( ScRange( aPos), aErrPos ) )
                            bError = true;
                        else
                            bArrow = true;
                    }
                    else if ( eType == SC_DETOBJ_CIRCLE )
                    {
                        

                        bError = true;
                    }
                    else if ( eType == SC_DETOBJ_NONE )
                    {
                        

                        if ( pObject->ISA( SdrRectObj ) && !pObject->ISA( SdrCaptionObj ) )
                        {
                            bArrow = true;
                        }
                    }

                    if ( bArrow || bError )
                    {
                        ColorData nColorData = ( bError ? GetErrorColor() : GetArrowColor() );
                        pObject->SetMergedItem( XLineColorItem( OUString(), Color( nColorData ) ) );

                        
                        pObject->ActionChanged();
                    }
                }
            }
        }
    }
}

bool ScDetectiveFunc::FindFrameForObject( SdrObject* pObject, ScRange& rRange )
{
    
    

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel) return false;

    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");
    if (!pPage) return false;

    
    if( pObject && pObject->GetPage() && (pObject->GetPage() == pObject->GetObjList()) )
    {
        
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
                    return true;
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
                    

                    FindFrameForObject( pObject, rSource );     
                }

                ColorData nObjColor = ((const XLineColorItem&)pObject->GetMergedItem(XATTR_LINECOLOR)).GetColorValue().GetColor();
                if ( nObjColor == GetErrorColor() && nObjColor != GetArrowColor() )
                    rRedLine = true;
            }
            else if ( pObject->ISA(SdrCircObj) )
            {
                if ( bValidStart )
                {
                    

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
