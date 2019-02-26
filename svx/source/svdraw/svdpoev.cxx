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
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/svdtrans.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <tools/helpers.hxx>

#include <svx/polypolygoneditor.hxx>

using namespace sdr;


void SdrPolyEditView::ImpResetPolyPossibilityFlags()
{
    eMarkedPointsSmooth=SdrPathSmoothKind::DontCare;
    eMarkedSegmentsKind=SdrPathSegmentKind::DontCare;
    bSetMarkedPointsSmoothPossible=false;
    bSetMarkedSegmentsKindPossible=false;
}

SdrPolyEditView::SdrPolyEditView(
    SdrModel& rSdrModel,
    OutputDevice* pOut)
:   SdrEditView(rSdrModel, pOut)
{
    ImpResetPolyPossibilityFlags();
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
    SdrPathObj* pPath = dynamic_cast<SdrPathObj*>( pObj );

    if (!pPath)
        return;

    SdrUShortCont& rPts = pM->GetMarkedPoints();
    if (rPts.empty())
        return;

    const bool bClosed(pPath->IsClosed());
    bSetMarkedPointsSmoothPossible = true;

    if (bClosed)
    {
        bSetMarkedSegmentsKindPossible = true;
    }

    for (const auto& rPt : rPts)
    {
        sal_uInt32 nNum(rPt);
        sal_uInt32 nPolyNum, nPntNum;

        if(PolyPolygonEditor::GetRelativePolyPoint(pPath->GetPathPoly(), nNum, nPolyNum, nPntNum))
        {
            const basegfx::B2DPolygon aLocalPolygon(pPath->GetPathPoly().getB2DPolygon(nPolyNum));
            bool bCanSegment(bClosed || nPntNum < aLocalPolygon.count() - 1);

            if(!bSetMarkedSegmentsKindPossible && bCanSegment)
            {
                bSetMarkedSegmentsKindPossible = true;
            }

            if(!bSmoothFuz)
            {
                if (b1stSmooth)
                {
                    b1stSmooth = false;
                    eSmooth = basegfx::utils::getContinuityInPoint(aLocalPolygon, nPntNum);
                }
                else
                {
                    bSmoothFuz = (eSmooth != basegfx::utils::getContinuityInPoint(aLocalPolygon, nPntNum));
                }
            }

            if(!bSegmFuz && bCanSegment)
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

    if(!b1stSmooth && !bSmoothFuz)
    {
        if(basegfx::B2VectorContinuity::NONE == eSmooth)
        {
            eMarkedPointsSmooth = SdrPathSmoothKind::Angular;
        }

        if(basegfx::B2VectorContinuity::C1 == eSmooth)
        {
            eMarkedPointsSmooth = SdrPathSmoothKind::Asymmetric;
        }

        if(basegfx::B2VectorContinuity::C2 == eSmooth)
        {
            eMarkedPointsSmooth = SdrPathSmoothKind::Symmetric;
        }
    }

    if(!b1stSegm && !bSegmFuz)
    {
        eMarkedSegmentsKind = bCurve ? SdrPathSegmentKind::Curve : SdrPathSegmentKind::Line;
    }
}

void SdrPolyEditView::SetMarkedPointsSmooth(SdrPathSmoothKind eKind)
{
    basegfx::B2VectorContinuity eFlags;

    if(SdrPathSmoothKind::Angular == eKind)
    {
        eFlags = basegfx::B2VectorContinuity::NONE;
    }
    else if(SdrPathSmoothKind::Asymmetric == eKind)
    {
        eFlags = basegfx::B2VectorContinuity::C1;
    }
    else if(SdrPathSmoothKind::Symmetric == eKind)
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
            BegUndo(SvxResId(STR_EditSetPointsSmooth), GetDescriptionOfMarkedPoints());
        const size_t nMarkCount(GetMarkedObjectCount());

        for(size_t nMarkNum(nMarkCount); nMarkNum > 0;)
        {
            --nMarkNum;
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >( pM->GetMarkedSdrObj() );
            if (!pPath)
                continue;

            SdrUShortCont& rPts = pM->GetMarkedPoints();
            PolyPolygonEditor aEditor(pPath->GetPathPoly());
            if (aEditor.SetPointsSmooth(eFlags, rPts))
            {
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pPath));
                pPath->SetPathPoly(aEditor.GetPolyPolygon());
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
            BegUndo(SvxResId(STR_EditSetSegmentsKind), GetDescriptionOfMarkedPoints());
        const size_t nMarkCount(GetMarkedObjectCount());

        for(size_t nMarkNum=nMarkCount; nMarkNum > 0;)
        {
            --nMarkNum;
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >( pM->GetMarkedSdrObj() );
            if (!pPath)
                continue;
            SdrUShortCont& rPts = pM->GetMarkedPoints();
            PolyPolygonEditor aEditor( pPath->GetPathPoly());
            if (aEditor.SetSegmentsKind(eKind, rPts))
            {
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pPath));
                pPath->SetPathPoly(aEditor.GetPolyPolygon());
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
            BegUndo(SvxResId(STR_EditDelete),GetDescriptionOfMarkedPoints(),SdrRepeatFunc::Delete);
        }

        for (size_t nMarkNum=nMarkCount; nMarkNum>0;)
        {
            --nMarkNum;
            SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >( pM->GetMarkedSdrObj() );
            if (!pPath)
                continue;

            SdrUShortCont& rPts = pM->GetMarkedPoints();
            PolyPolygonEditor aEditor( pPath->GetPathPoly());
            if (aEditor.DeletePoints(rPts))
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
            BegUndo(SvxResId(STR_EditRipUp), GetDescriptionOfMarkedPoints());

        for(size_t nMarkNum = nMarkCount; nMarkNum > 0;)
        {
            --nMarkNum;
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            SdrPathObj* pObj = dynamic_cast<SdrPathObj*>( pM->GetMarkedSdrObj() );
            if (!pObj)
                continue;

            SdrUShortCont& rPts = pM->GetMarkedPoints();

            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
            bool bCorrectionFlag(false);
            sal_uInt32 nMax(pObj->GetHdlCount());

            for(SdrUShortCont::const_reverse_iterator it = rPts.rbegin(); it != rPts.rend(); ++it)
            {
                sal_uInt32 nNewPt0Idx(0);
                SdrObject* pNewObj = pObj->RipPoint(*it, nNewPt0Idx);

                if(pNewObj)
                {
                    pM->GetPageView()->GetObjList()->InsertObject(pNewObj, pObj->GetOrdNum() + 1);
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pNewObj));
                    MarkObj(pNewObj, pM->GetPageView(), false, true);
                }

                if(nNewPt0Idx)
                {
                    // correction necessary?
                    DBG_ASSERT(!bCorrectionFlag,"Multiple index corrections at SdrPolyEditView::RipUp().");
                    if(!bCorrectionFlag)
                    {
                        bCorrectionFlag = true;

                        SdrUShortCont aReplaceSet;
                        for(const auto& rPt : rPts)
                        {
                            sal_uInt32 nPntNum(rPt);
                            nPntNum += nNewPt0Idx;

                            if(nPntNum >= nMax)
                            {
                                nPntNum -= nMax;
                            }

                            aReplaceSet.insert( static_cast<sal_uInt16>(nPntNum) );
                        }
                        rPts.swap(aReplaceSet);

                        it = rPts.rbegin();
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

        if (!pMarkedPathObject)
            continue;

        const SdrUShortCont& rSelectedPoints = pMark->GetMarkedPoints();
        if (rSelectedPoints.empty())
            continue;

        const basegfx::B2DPolyPolygon& rPathPolyPolygon = pMarkedPathObject->GetPathPoly();

        if(1 == rPathPolyPolygon.count())
        {
            // #i76617# Do not yet use basegfx::B2DPolygon since curve definitions
            // are different and methods need to be changed thoroughly with interaction rework
            const tools::Polygon aPathPolygon(rPathPolyPolygon.getB2DPolygon(0));
            const sal_uInt16 nPointCount(aPathPolygon.GetSize());

            if(nPointCount >= 3)
            {
                bRetval = pMarkedPathObject->IsClosedObj() // #i76617#
                    || std::any_of(rSelectedPoints.begin(), rSelectedPoints.end(),
                        [nPointCount](const sal_uInt16 nMarkedPointNum) {
                            return nMarkedPointNum > 0 && nMarkedPointNum < nPointCount - 1;
                        });
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
        return SdrObjClosedKind::DontCare;
    }
    else if(bOpen)
    {
        return SdrObjClosedKind::Open;
    }
    else
    {
        return SdrObjClosedKind::Closed;
    }
}

void SdrPolyEditView::ImpTransformMarkedPoints(PPolyTrFunc pTrFunc, const void* p1, const void* p2, const void* p3, const void* p4)
{
    const bool bUndo = IsUndoEnabled();

    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nm=0; nm<nMarkCount; ++nm)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrPathObj* pPath=dynamic_cast<SdrPathObj*>( pObj );
        if (!pPath)
            continue;

        const SdrUShortCont& rPts = pM->GetMarkedPoints();
        if (rPts.empty())
            continue;

        if( bUndo )
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

        basegfx::B2DPolyPolygon aXPP(pPath->GetPathPoly());

        for (const auto& rPt : rPts)
        {
            sal_uInt32 nPt = rPt;
            sal_uInt32 nPolyNum, nPointNum;

            if(PolyPolygonEditor::GetRelativePolyPoint(aXPP, nPt, nPolyNum, nPointNum))
            {
                //#i83671# used nLocalPointNum (which was the polygon point count)
                // instead of the point index (nPointNum). This of course led
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

                (*pTrFunc)(aPos,&aC1,&aC2,p1,p2,p3,p4);
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


static void ImpMove(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* /*p2*/, const void* /*p3*/, const void* /*p4*/)
{
    rPt.Move(*static_cast<const Size*>(p1));
    if (pC1!=nullptr) pC1->Move(*static_cast<const Size*>(p1));
    if (pC2!=nullptr) pC2->Move(*static_cast<const Size*>(p1));
}

void SdrPolyEditView::MoveMarkedPoints(const Size& rSiz)
{
    ForceUndirtyMrkPnt();
    OUString aStr(SvxResId(STR_EditMove));
    BegUndo(aStr,GetDescriptionOfMarkedPoints(),SdrRepeatFunc::Move);
    ImpTransformMarkedPoints(ImpMove,&rSiz);
    EndUndo();
    AdjustMarkHdl();
}

static void ImpResize(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* p2, const void* p3, const void* /*p4*/)
{
    ResizePoint(rPt,*static_cast<const Point*>(p1),*static_cast<const Fraction*>(p2),*static_cast<const Fraction*>(p3));
    if (pC1!=nullptr) ResizePoint(*pC1,*static_cast<const Point*>(p1),*static_cast<const Fraction*>(p2),*static_cast<const Fraction*>(p3));
    if (pC2!=nullptr) ResizePoint(*pC2,*static_cast<const Point*>(p1),*static_cast<const Fraction*>(p2),*static_cast<const Fraction*>(p3));
}

void SdrPolyEditView::ResizeMarkedPoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    ForceUndirtyMrkPnt();
    OUString aStr(SvxResId(STR_EditResize));
    BegUndo(aStr,GetDescriptionOfMarkedPoints(),SdrRepeatFunc::Resize);
    ImpTransformMarkedPoints(ImpResize,&rRef,&xFact,&yFact);
    EndUndo();
    AdjustMarkHdl();
}

static void ImpRotate(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* /*p2*/, const void* p3, const void* p4)
{
    RotatePoint(rPt,*static_cast<const Point*>(p1),*static_cast<const double*>(p3),*static_cast<const double*>(p4));
    if (pC1!=nullptr) RotatePoint(*pC1,*static_cast<const Point*>(p1),*static_cast<const double*>(p3),*static_cast<const double*>(p4));
    if (pC2!=nullptr) RotatePoint(*pC2,*static_cast<const Point*>(p1),*static_cast<const double*>(p3),*static_cast<const double*>(p4));
}

void SdrPolyEditView::RotateMarkedPoints(const Point& rRef, long nAngle)
{
    ForceUndirtyMrkPnt();
    OUString aStr(SvxResId(STR_EditResize));
    BegUndo(aStr,GetDescriptionOfMarkedPoints(),SdrRepeatFunc::Rotate);
    double nSin = sin(nAngle * F_PI18000);
    double nCos = cos(nAngle * F_PI18000);
    ImpTransformMarkedPoints(ImpRotate,&rRef,&nAngle,&nSin,&nCos);
    EndUndo();
    AdjustMarkHdl();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
