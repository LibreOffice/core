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


#include <svx/svdpoev.hxx>
#include <math.h>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdundo.hxx>
#include "svx/svdstr.hrc"
#include "svdglob.hxx"
#include <svx/svdtrans.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <tools/helpers.hxx>

#include <svx/polypolygoneditor.hxx>

using namespace sdr;



void SdrPolyEditView::ImpResetPolyPossibilityFlags()
{
    eMarkedPointsSmooth=SDRPATHSMOOTH_DONTCARE;
    eMarkedSegmentsKind=SDRPATHSEGMENT_DONTCARE;
    bSetMarkedPointsSmoothPossible=false;
    bSetMarkedSegmentsKindPossible=false;
}

void SdrPolyEditView::ImpClearVars()
{
    ImpResetPolyPossibilityFlags();
}

SdrPolyEditView::SdrPolyEditView(SdrModel* pModel1, OutputDevice* pOut):
    SdrEditView(pModel1,pOut)
{
    ImpClearVars();
}

SdrPolyEditView::~SdrPolyEditView()
{
}

void SdrPolyEditView::ImpCheckPolyPossibilities()
{
    ImpResetPolyPossibilityFlags();
    const size_t nMarkCount(GetMarkedObjectCount());

    if(nMarkCount && !ImpIsFrameHandles())
    {
        bool b1stSmooth(true);
        bool b1stSegm(true);
        bool bCurve(false);
        bool bSmoothFuz(false);
        bool bSegmFuz(false);
        basegfx::B2VectorContinuity eSmooth = basegfx::B2VectorContinuity::NONE;

        for(size_t nMarkNum = 0; nMarkNum < nMarkCount; ++nMarkNum)
        {
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            CheckPolyPossibilitiesHelper( pM, b1stSmooth, b1stSegm, bCurve, bSmoothFuz, bSegmFuz, eSmooth );
        }
    }
}

void SdrPolyEditView::CheckPolyPossibilitiesHelper( SdrMark* pM, bool& b1stSmooth, bool& b1stSegm, bool& bCurve, bool& bSmoothFuz, bool& bSegmFuz, basegfx::B2VectorContinuity& eSmooth )
{
    SdrObject* pObj = pM->GetMarkedSdrObj();
    SdrUShortCont* pPts = pM->GetMarkedPoints();
    SdrPathObj* pPath = dynamic_cast<SdrPathObj*>( pObj );

    if(pPath && pPts)
    {
        const sal_uInt32 nMarkedPntAnz(pPts->size());

        if(nMarkedPntAnz)
        {
            bool bClosed(pPath->IsClosed());
            bSetMarkedPointsSmoothPossible = true;

            if(bClosed)
            {
                bSetMarkedSegmentsKindPossible = true;
            }

            for(SdrUShortCont::const_iterator it = pPts->begin(); it != pPts->end(); ++it)
            {
                sal_uInt32 nNum(*it);
                sal_uInt32 nPolyNum, nPntNum;

                if(PolyPolygonEditor::GetRelativePolyPoint(pPath->GetPathPoly(), nNum, nPolyNum, nPntNum))
                {
                    const basegfx::B2DPolygon aLocalPolygon(pPath->GetPathPoly().getB2DPolygon(nPolyNum));
                    bool bCanSegment(bClosed || nPntNum < aLocalPolygon.count() - 1L);

                    if(!bSetMarkedSegmentsKindPossible && bCanSegment)
                    {
                        bSetMarkedSegmentsKindPossible = true;
                    }

                    if(!bSmoothFuz)
                    {
                        if (b1stSmooth)
                        {
                            b1stSmooth = false;
                            eSmooth = basegfx::tools::getContinuityInPoint(aLocalPolygon, nPntNum);
                        }
                        else
                        {
                            bSmoothFuz = (eSmooth != basegfx::tools::getContinuityInPoint(aLocalPolygon, nPntNum));
                        }
                    }

                    if(!bSegmFuz)
                    {
                        if(bCanSegment)
                        {
                            bool bCrv(aLocalPolygon.isNextControlPointUsed(nPntNum));

                            if(b1stSegm)
                            {
                                b1stSegm = false;
                                bCurve = bCrv;
                            }
                            else
                            {
                                bSegmFuz = (bCrv != bCurve);
                            }
                        }
                    }
                }
            }

            if(!b1stSmooth && !bSmoothFuz)
            {
                if(basegfx::B2VectorContinuity::NONE == eSmooth)
                {
                    eMarkedPointsSmooth = SDRPATHSMOOTH_ANGULAR;
                }

                if(basegfx::B2VectorContinuity::C1 == eSmooth)
                {
                    eMarkedPointsSmooth = SDRPATHSMOOTH_ASYMMETRIC;
                }

                if(basegfx::B2VectorContinuity::C2 == eSmooth)
                {
                    eMarkedPointsSmooth = SDRPATHSMOOTH_SYMMETRIC;
                }
            }

            if(!b1stSegm && !bSegmFuz)
            {
                eMarkedSegmentsKind = (bCurve) ? SDRPATHSEGMENT_CURVE : SDRPATHSEGMENT_LINE;
            }
        }
    }
}

void SdrPolyEditView::SetMarkedPointsSmooth(SdrPathSmoothKind eKind)
{
    basegfx::B2VectorContinuity eFlags;

    if(SDRPATHSMOOTH_ANGULAR == eKind)
    {
        eFlags = basegfx::B2VectorContinuity::NONE;
    }
    else if(SDRPATHSMOOTH_ASYMMETRIC == eKind)
    {
        eFlags = basegfx::B2VectorContinuity::C1;
    }
    else if(SDRPATHSMOOTH_SYMMETRIC == eKind)
    {
        eFlags = basegfx::B2VectorContinuity::C2;
    }
    else
    {
        return;
    }

    if(HasMarkedPoints())
    {
        SortMarkedObjects();

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditSetPointsSmooth), GetDescriptionOfMarkedPoints());
        const size_t nMarkCount(GetMarkedObjectCount());

        for(size_t nMarkNum(nMarkCount); nMarkNum > 0;)
        {
            --nMarkNum;
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts = pM->GetMarkedPoints();
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >( pM->GetMarkedSdrObj() );

            if(pPts && pPath)
            {
                PolyPolygonEditor aEditor( pPath->GetPathPoly(), pPath->IsClosed() );
                if(aEditor.SetPointsSmooth( eFlags, *pPts ) )
                {
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pPath));
                    pPath->SetPathPoly(aEditor.GetPolyPolygon());
                }
            }
        }

        if( bUndo )
            EndUndo();
    }
}

void SdrPolyEditView::SetMarkedSegmentsKind(SdrPathSegmentKind eKind)
{
    if(HasMarkedPoints())
    {
        SortMarkedObjects();

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditSetSegmentsKind), GetDescriptionOfMarkedPoints());
        const size_t nMarkCount(GetMarkedObjectCount());

        for(size_t nMarkNum=nMarkCount; nMarkNum > 0;)
        {
            --nMarkNum;
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts = pM->GetMarkedPoints();
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >( pM->GetMarkedSdrObj() );

            if(pPts && pPath)
            {
                PolyPolygonEditor aEditor( pPath->GetPathPoly(), pPath->IsClosed() );
                if(aEditor.SetSegmentsKind( eKind, *pPts ) )
                {
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pPath));
                    pPath->SetPathPoly(aEditor.GetPolyPolygon());
                }
            }
        }

        if( bUndo )
            EndUndo();
    }
}

bool SdrPolyEditView::IsSetMarkedPointsSmoothPossible() const
{
    ForcePossibilities();
    return bSetMarkedPointsSmoothPossible;
}

SdrPathSmoothKind SdrPolyEditView::GetMarkedPointsSmooth() const
{
    ForcePossibilities();
    return eMarkedPointsSmooth;
}

bool SdrPolyEditView::IsSetMarkedSegmentsKindPossible() const
{
    ForcePossibilities();
    return bSetMarkedSegmentsKindPossible;
}

SdrPathSegmentKind SdrPolyEditView::GetMarkedSegmentsKind() const
{
    ForcePossibilities();
    return eMarkedSegmentsKind;
}

bool SdrPolyEditView::IsDeleteMarkedPointsPossible() const
{
    return HasMarkedPoints();
}

void SdrPolyEditView::DeleteMarkedPoints()
{
    if (HasMarkedPoints())
    {
        BrkAction();
        SortMarkedObjects();
        const size_t nMarkCount=GetMarkedObjectCount();

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
        {
            // Description
            BegUndo(ImpGetResStr(STR_EditDelete),GetDescriptionOfMarkedPoints(),SDRREPFUNC_OBJ_DELETE);
        }

        for (size_t nMarkNum=nMarkCount; nMarkNum>0;)
        {
            --nMarkNum;
            SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts=pM->GetMarkedPoints();
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >( pM->GetMarkedSdrObj() );

            if( pPath && pPts )
            {
                PolyPolygonEditor aEditor( pPath->GetPathPoly(), pPath->IsClosed() );
                if( aEditor.DeletePoints( *pPts ) )
                {
                    if( aEditor.GetPolyPolygon().count() )
                    {
                        if( bUndo )
                            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pPath ));
                        pPath->SetPathPoly( aEditor.GetPolyPolygon() );
                    }
                    else
                    {
                        if( bUndo )
                            AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pPath ) );
                        pM->GetPageView()->GetObjList()->RemoveObject(pPath->GetOrdNum());
                        if( !bUndo )
                        {
                            SdrObject* pObj = pPath;
                            SdrObject::Free(pObj);
                        }
                    }
                }
            }
        }

        if( bUndo )
            EndUndo();
        UnmarkAllPoints();
        MarkListHasChanged();
    }
}

void SdrPolyEditView::RipUpAtMarkedPoints()
{
    if(HasMarkedPoints())
    {
        SortMarkedObjects();
        const size_t nMarkCount(GetMarkedObjectCount());

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditRipUp), GetDescriptionOfMarkedPoints());

        for(size_t nMarkNum = nMarkCount; nMarkNum > 0;)
        {
            --nMarkNum;
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts = pM->GetMarkedPoints();
            SdrPathObj* pObj = dynamic_cast<SdrPathObj*>( pM->GetMarkedSdrObj() );

            if(pPts && pObj)
            {
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                bool bKorregFlag(false);
                sal_uInt32 nMax(pObj->GetHdlCount());

                for(SdrUShortCont::const_reverse_iterator it = pPts->rbegin(); it != pPts->rend(); ++it)
                {
                    sal_uInt32 nNewPt0Idx(0L);
                    SdrObject* pNeuObj = pObj->RipPoint(*it, nNewPt0Idx);

                    if(pNeuObj)
                    {
                        SdrInsertReason aReason(SDRREASON_VIEWCALL);
                        pM->GetPageView()->GetObjList()->InsertObject(pNeuObj, pObj->GetOrdNum() + 1, &aReason);
                        if( bUndo )
                            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pNeuObj));
                        MarkObj(pNeuObj, pM->GetPageView(), false, true);
                    }

                    if(nNewPt0Idx)
                    {
                        // correction necessary?
                        DBG_ASSERT(!bKorregFlag,"Multiple index corrections at SdrPolyEditView::RipUp().");
                        if(!bKorregFlag)
                        {
                            bKorregFlag = true;

                            SdrUShortCont aReplaceSet;
                            for(SdrUShortCont::const_iterator it2 = pPts->begin(); it2 != pPts->end(); ++it2)
                            {
                                sal_uInt32 nPntNum(*it2);
                                nPntNum += nNewPt0Idx;

                                if(nPntNum >= nMax)
                                {
                                    nPntNum -= nMax;
                                }

                                aReplaceSet.insert( (sal_uInt16)nPntNum );
                            }
                            pPts->swap(aReplaceSet);

                            it = pPts->rbegin();
                        }
                    }
                }
            }
        }

        UnmarkAllPoints();
        if( bUndo )
            EndUndo();
        MarkListHasChanged();
    }
}

bool SdrPolyEditView::IsRipUpAtMarkedPointsPossible() const
{
    bool bRetval(false);
    const size_t nMarkCount(GetMarkedObjectCount());

    for(size_t a = 0; a < nMarkCount; ++a)
    {
        const SdrMark* pMark = GetSdrMarkByIndex(a);
        const SdrPathObj* pMarkedPathObject = dynamic_cast< const SdrPathObj* >(pMark->GetMarkedSdrObj());

        if(pMarkedPathObject)
        {
            const SdrUShortCont* pSelectedPoints = pMark->GetMarkedPoints();

            if(pSelectedPoints && !pSelectedPoints->empty())
            {
                const basegfx::B2DPolyPolygon& rPathPolyPolygon = pMarkedPathObject->GetPathPoly();

                if(1 == rPathPolyPolygon.count())
                {
                    // #i76617# Do not yet use basegfx::B2DPolygon since curve definitions
                    // are different and methods need to be changed thoroughly with interaction rework
                    const tools::Polygon aPathPolygon(rPathPolyPolygon.getB2DPolygon(0));
                    const sal_uInt16 nPointCount(aPathPolygon.GetSize());

                    if(nPointCount >= 3)
                    {
                        bRetval = pMarkedPathObject->IsClosedObj(); // #i76617#

                        for(SdrUShortCont::const_iterator it = pSelectedPoints->begin();
                            !bRetval && it != pSelectedPoints->end(); ++it)
                        {
                            const sal_uInt16 nMarkedPointNum(*it);

                            bRetval = (nMarkedPointNum > 0 && nMarkedPointNum < nPointCount - 1);
                        }
                    }
                }
            }
        }
    }

    return bRetval;
}

bool SdrPolyEditView::IsOpenCloseMarkedObjectsPossible() const
{
    bool bRetval(false);
    const size_t nMarkCount(GetMarkedObjectCount());

    for(size_t a = 0; a < nMarkCount; ++a)
    {
        const SdrMark* pMark = GetSdrMarkByIndex(a);
        const SdrPathObj* pMarkedPathObject = dynamic_cast< const SdrPathObj* >(pMark->GetMarkedSdrObj());

        if(pMarkedPathObject)
        {
            // #i76617# Do not yet use basegfx::B2DPolygon since curve definitions
            // are different and methods need to be changed thoroughly with interaction rework
            const tools::PolyPolygon aPathPolyPolygon(pMarkedPathObject->GetPathPoly());
            const sal_uInt16 nPolygonCount(aPathPolyPolygon.Count());

            for(sal_uInt16 b(0); !bRetval && b < nPolygonCount; b++)
            {
                const tools::Polygon& rPathPolygon = aPathPolyPolygon[b];
                const sal_uInt16 nPointCount(rPathPolygon.GetSize());

                bRetval = (nPointCount >= 3);
            }
        }
    }

    return bRetval;
}

SdrObjClosedKind SdrPolyEditView::GetMarkedObjectsClosedState() const
{
    bool bOpen(false);
    bool bClosed(false);
    const size_t nMarkCount(GetMarkedObjectCount());

    for(size_t a = 0; !(bOpen && bClosed) && a < nMarkCount; ++a)
    {
        const SdrMark* pMark = GetSdrMarkByIndex(a);
        const SdrPathObj* pMarkedPathObject = dynamic_cast< const SdrPathObj* >(pMark->GetMarkedSdrObj());

        if(pMarkedPathObject)
        {
            if(pMarkedPathObject->IsClosedObj())
            {
                bClosed = true;
            }
            else
            {
                bOpen = true;
            }
        }
    }

    if(bOpen && bClosed)
    {
        return SDROBJCLOSED_DONTCARE;
    }
    else if(bOpen)
    {
        return SDROBJCLOSED_OPEN;
    }
    else
    {
        return SDROBJCLOSED_CLOSED;
    }
}

void SdrPolyEditView::ImpTransformMarkedPoints(PPolyTrFunc pTrFunc, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    const bool bUndo = IsUndoEnabled();

    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nm=0; nm<nMarkCount; ++nm)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont* pPts=pM->GetMarkedPoints();
        sal_uIntPtr nPointCount=pPts==NULL ? 0 : pPts->size();
        SdrPathObj* pPath=dynamic_cast<SdrPathObj*>( pObj );
        if (nPointCount!=0 && pPath!=NULL)
        {
            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

            basegfx::B2DPolyPolygon aXPP(pPath->GetPathPoly());

            for(SdrUShortCont::const_iterator it = pPts->begin(); it != pPts->end(); ++it)
            {
                sal_uInt32 nPt = *it;
                sal_uInt32 nPolyNum, nPointNum;

                if(PolyPolygonEditor::GetRelativePolyPoint(aXPP, nPt, nPolyNum, nPointNum))
                {
                    //#i83671# used nLocalPointNum (which was the polygon point count)
                    // instead of the point index (nPointNum). This of course leaded
                    // to a wrong point access to the B2DPolygon.
                    basegfx::B2DPolygon aNewXP(aXPP.getB2DPolygon(nPolyNum));
                    Point aPos, aC1, aC2;
                    bool bC1(false);
                    bool bC2(false);

                    const basegfx::B2DPoint aB2DPos(aNewXP.getB2DPoint(nPointNum));
                    aPos = Point(FRound(aB2DPos.getX()), FRound(aB2DPos.getY()));

                    if(aNewXP.isPrevControlPointUsed(nPointNum))
                    {
                        const basegfx::B2DPoint aB2DC1(aNewXP.getPrevControlPoint(nPointNum));
                        aC1 = Point(FRound(aB2DC1.getX()), FRound(aB2DC1.getY()));
                        bC1 = true;
                    }

                    if(aNewXP.isNextControlPointUsed(nPointNum))
                    {
                        const basegfx::B2DPoint aB2DC2(aNewXP.getNextControlPoint(nPointNum));
                        aC2 = Point(FRound(aB2DC2.getX()), FRound(aB2DC2.getY()));
                        bC2 = true;
                    }

                    (*pTrFunc)(aPos,&aC1,&aC2,p1,p2,p3,p4,p5);
                    aNewXP.setB2DPoint(nPointNum, basegfx::B2DPoint(aPos.X(), aPos.Y()));

                    if (bC1)
                    {
                        aNewXP.setPrevControlPoint(nPointNum, basegfx::B2DPoint(aC1.X(), aC1.Y()));
                    }

                    if (bC2)
                    {
                        aNewXP.setNextControlPoint(nPointNum, basegfx::B2DPoint(aC2.X(), aC2.Y()));
                    }

                    aXPP.setB2DPolygon(nPolyNum, aNewXP);
                }
            }

            pPath->SetPathPoly(aXPP);
        }
    }
}



static void ImpMove(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* /*p2*/, const void* /*p3*/, const void* /*p4*/, const void* /*p5*/)
{
    MovePoint(rPt,*static_cast<const Size*>(p1));
    if (pC1!=NULL) MovePoint(*pC1,*static_cast<const Size*>(p1));
    if (pC2!=NULL) MovePoint(*pC2,*static_cast<const Size*>(p1));
}

void SdrPolyEditView::MoveMarkedPoints(const Size& rSiz)
{
    ForceUndirtyMrkPnt();
    OUString aStr(ImpGetResStr(STR_EditMove));
    BegUndo(aStr,GetDescriptionOfMarkedPoints(),SDRREPFUNC_OBJ_MOVE);
    ImpTransformMarkedPoints(ImpMove,&rSiz);
    EndUndo();
    AdjustMarkHdl();
}

static void ImpResize(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* p2, const void* p3, const void* /*p4*/, const void* /*p5*/)
{
    ResizePoint(rPt,*static_cast<const Point*>(p1),*static_cast<const Fraction*>(p2),*static_cast<const Fraction*>(p3));
    if (pC1!=NULL) ResizePoint(*pC1,*static_cast<const Point*>(p1),*static_cast<const Fraction*>(p2),*static_cast<const Fraction*>(p3));
    if (pC2!=NULL) ResizePoint(*pC2,*static_cast<const Point*>(p1),*static_cast<const Fraction*>(p2),*static_cast<const Fraction*>(p3));
}

void SdrPolyEditView::ResizeMarkedPoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    ForceUndirtyMrkPnt();
    OUString aStr(ImpGetResStr(STR_EditResize));
    BegUndo(aStr,GetDescriptionOfMarkedPoints(),SDRREPFUNC_OBJ_RESIZE);
    ImpTransformMarkedPoints(ImpResize,&rRef,&xFact,&yFact);
    EndUndo();
    AdjustMarkHdl();
}

static void ImpRotate(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* /*p2*/, const void* p3, const void* p4, const void* /*p5*/)
{
    RotatePoint(rPt,*static_cast<const Point*>(p1),*static_cast<const double*>(p3),*static_cast<const double*>(p4));
    if (pC1!=NULL) RotatePoint(*pC1,*static_cast<const Point*>(p1),*static_cast<const double*>(p3),*static_cast<const double*>(p4));
    if (pC2!=NULL) RotatePoint(*pC2,*static_cast<const Point*>(p1),*static_cast<const double*>(p3),*static_cast<const double*>(p4));
}

void SdrPolyEditView::RotateMarkedPoints(const Point& rRef, long nAngle)
{
    ForceUndirtyMrkPnt();
    OUString aStr(ImpGetResStr(STR_EditResize));
    BegUndo(aStr,GetDescriptionOfMarkedPoints(),SDRREPFUNC_OBJ_ROTATE);
    double nSin=sin(nAngle*nPi180);
    double nCos=cos(nAngle*nPi180);
    ImpTransformMarkedPoints(ImpRotate,&rRef,&nAngle,&nSin,&nCos);
    EndUndo();
    AdjustMarkHdl();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
