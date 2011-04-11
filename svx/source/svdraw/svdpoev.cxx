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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdpoev.hxx>
#include <math.h>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdundo.hxx>
#include "svx/svdstr.hrc"   // Namen aus der Resource
#include "svx/svdglob.hxx"  // StringCache
#include <svx/svdtrans.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <vcl/salbtype.hxx>     // FRound

#include <svx/polypolygoneditor.hxx>

using namespace sdr;

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPolyEditView::ImpResetPolyPossibilityFlags()
{
    eMarkedPointsSmooth=SDRPATHSMOOTH_DONTCARE;
    eMarkedSegmentsKind=SDRPATHSEGMENT_DONTCARE;
    bSetMarkedPointsSmoothPossible=sal_False;
    bSetMarkedSegmentsKindPossible=sal_False;
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
    const sal_uIntPtr nMarkAnz(GetMarkedObjectCount());

    if(nMarkAnz && !ImpIsFrameHandles())
    {
        bool b1stSmooth(true);
        bool b1stSegm(true);
        bool bCurve(false);
        bool bSmoothFuz(false);
        bool bSegmFuz(false);
        basegfx::B2VectorContinuity eSmooth = basegfx::CONTINUITY_NONE;

        for(sal_uIntPtr nMarkNum(0L); nMarkNum < nMarkAnz; nMarkNum++)
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
    SdrPathObj* pPath = PTR_CAST(SdrPathObj,pObj);

    if(pPath && pPts)
    {
        const sal_uInt32 nMarkedPntAnz(pPts->GetCount());

        if(nMarkedPntAnz)
        {
            bool bClosed(pPath->IsClosed());
            bSetMarkedPointsSmoothPossible = true;

            if(bClosed)
            {
                bSetMarkedSegmentsKindPossible = true;
            }

            for(sal_uInt32 nMarkedPntNum(0L); nMarkedPntNum < nMarkedPntAnz; nMarkedPntNum++)
            {
                sal_uInt32 nNum(pPts->GetObject(nMarkedPntNum));
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
                if(basegfx::CONTINUITY_NONE == eSmooth)
                {
                    eMarkedPointsSmooth = SDRPATHSMOOTH_ANGULAR;
                }

                if(basegfx::CONTINUITY_C1 == eSmooth)
                {
                    eMarkedPointsSmooth = SDRPATHSMOOTH_ASYMMETRIC;
                }

                if(basegfx::CONTINUITY_C2 == eSmooth)
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
        eFlags = basegfx::CONTINUITY_NONE;
    }
    else if(SDRPATHSMOOTH_ASYMMETRIC == eKind)
    {
        eFlags = basegfx::CONTINUITY_C1;
    }
    else if(SDRPATHSMOOTH_SYMMETRIC == eKind)
    {
        eFlags = basegfx::CONTINUITY_C2;
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
        sal_uIntPtr nMarkAnz(GetMarkedObjectCount());

        for(sal_uIntPtr nMarkNum(nMarkAnz); nMarkNum > 0L;)
        {
            nMarkNum--;
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts = pM->GetMarkedPoints();
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >( pM->GetMarkedSdrObj() );

            if(pPts && pPath)
            {
                PolyPolygonEditor aEditor( pPath->GetPathPoly(), pPath->IsClosed() );
                if(aEditor.SetPointsSmooth( eFlags, pPts->getContainer() ) )
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
        sal_uIntPtr nMarkAnz(GetMarkedObjectCount());

        for(sal_uIntPtr nMarkNum(nMarkAnz); nMarkNum > 0L;)
        {
            nMarkNum--;
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts = pM->GetMarkedPoints();
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >( pM->GetMarkedSdrObj() );

            if(pPts && pPath)
            {
                PolyPolygonEditor aEditor( pPath->GetPathPoly(), pPath->IsClosed() );
                if(aEditor.SetSegmentsKind( eKind, pPts->getContainer()) )
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

sal_Bool SdrPolyEditView::IsSetMarkedPointsSmoothPossible() const
{
    ForcePossibilities();
    return bSetMarkedPointsSmoothPossible;
}

SdrPathSmoothKind SdrPolyEditView::GetMarkedPointsSmooth() const
{
    ForcePossibilities();
    return eMarkedPointsSmooth;
}

sal_Bool SdrPolyEditView::IsSetMarkedSegmentsKindPossible() const
{
    ForcePossibilities();
    return bSetMarkedSegmentsKindPossible;
}

SdrPathSegmentKind SdrPolyEditView::GetMarkedSegmentsKind() const
{
    ForcePossibilities();
    return eMarkedSegmentsKind;
}

sal_Bool SdrPolyEditView::IsDeleteMarkedPointsPossible() const
{
    return HasMarkedPoints();
}

void SdrPolyEditView::DeleteMarkedPoints()
{
    if (HasMarkedPoints())
    {
        BrkAction();
        SortMarkedObjects();
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
        {
            // Description
            BegUndo(ImpGetResStr(STR_EditDelete),GetDescriptionOfMarkedPoints(),SDRREPFUNC_OBJ_DELETE);
        }

        for (sal_uIntPtr nMarkNum=nMarkAnz; nMarkNum>0;)
        {
            nMarkNum--;
            SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts=pM->GetMarkedPoints();
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >( pM->GetMarkedSdrObj() );

            if( pPath && pPts )
            {
                PolyPolygonEditor aEditor( pPath ->GetPathPoly(), pPath->IsClosed() );
                if( aEditor.DeletePoints( pPts->getContainer() ) )
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
        sal_uInt32 nMarkAnz(GetMarkedObjectCount());

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditRipUp), GetDescriptionOfMarkedPoints());

        for(sal_uInt32 nMarkNum(nMarkAnz); nMarkNum > 0L;)
        {
            nMarkNum--;
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts = pM->GetMarkedPoints();
            SdrPathObj* pObj = PTR_CAST(SdrPathObj, pM->GetMarkedSdrObj());

            if(pPts && pObj)
            {
                pPts->ForceSort();
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                sal_Bool bKorregFlag(sal_False);
                sal_uInt32 nMarkPtsAnz(pPts->GetCount());
                sal_uInt32 nMax(pObj->GetHdlCount());

                for(sal_uInt32 i(nMarkPtsAnz); i > 0L;)
                {
                    i--;
                    sal_uInt32 nNewPt0Idx(0L);
                    SdrObject* pNeuObj = pObj->RipPoint(pPts->GetObject(i), nNewPt0Idx);

                    if(pNeuObj)
                    {
                        SdrInsertReason aReason(SDRREASON_VIEWCALL, pObj);
                        pM->GetPageView()->GetObjList()->InsertObject(pNeuObj, pObj->GetOrdNum() + 1, &aReason);
                        if( bUndo )
                            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pNeuObj));
                        MarkObj(pNeuObj, pM->GetPageView(), sal_False, sal_True);
                    }

                    if(nNewPt0Idx)
                    {
                        // Korrektur notwendig?
                        DBG_ASSERT(bKorregFlag==sal_False,"Mehrfache Indexkorrektur bei SdrPolyEditView::RipUp()");
                        if(!bKorregFlag)
                        {
                            bKorregFlag = sal_True;

                            for(sal_uInt32 nBla(0L); nBla < nMarkPtsAnz; nBla++)
                            {
                                sal_uInt32 nPntNum(pPts->GetObject(nBla));
                                nPntNum += nNewPt0Idx;

                                if(nPntNum >= nMax)
                                {
                                    nPntNum -= nMax;
                                }

                                pPts->Replace((sal_uInt16)nPntNum, nBla);
                            }

                            i = nMarkPtsAnz; // ... und nochmal von vorn
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
    const sal_uInt32 nMarkCount(GetMarkedObjectCount());

    for(sal_uInt32 a(0); a < nMarkCount; a++)
    {
        const SdrMark* pMark = GetSdrMarkByIndex(a);
        const SdrPathObj* pMarkedPathObject = dynamic_cast< const SdrPathObj* >(pMark->GetMarkedSdrObj());

        if(pMarkedPathObject)
        {
            const SdrUShortCont* pSelectedPoints = pMark->GetMarkedPoints();

            if(pSelectedPoints && pSelectedPoints->GetCount())
            {
                const basegfx::B2DPolyPolygon& rPathPolyPolygon = pMarkedPathObject->GetPathPoly();

                if(1 == rPathPolyPolygon.count())
                {
                    // #i76617# Do not yet use basegfx::B2DPolygon since curve definitions
                    // are different and methods need to be changed thoroughly with interaction rework
                    const Polygon aPathPolygon(rPathPolyPolygon.getB2DPolygon(0));
                    const sal_uInt16 nPointCount(aPathPolygon.GetSize());

                    if(nPointCount >= 3)
                    {
                        bRetval = pMarkedPathObject->IsClosedObj(); // #i76617# aPathPolygon.isClosed();

                        for(sal_uInt32 b(0); !bRetval && b < pSelectedPoints->GetCount(); b++)
                        {
                            const sal_uInt16 nMarkedPointNum(pSelectedPoints->GetObject(b));

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
    const sal_uInt32 nMarkCount(GetMarkedObjectCount());

    for(sal_uInt32 a(0); a < nMarkCount; a++)
    {
        const SdrMark* pMark = GetSdrMarkByIndex(a);
        const SdrPathObj* pMarkedPathObject = dynamic_cast< const SdrPathObj* >(pMark->GetMarkedSdrObj());

        if(pMarkedPathObject)
        {
            // #i76617# Do not yet use basegfx::B2DPolygon since curve definitions
            // are different and methods need to be changed thoroughly with interaction rework
            const PolyPolygon aPathPolyPolygon(pMarkedPathObject->GetPathPoly());
            const sal_uInt16 nPolygonCount(aPathPolyPolygon.Count());

            for(sal_uInt16 b(0); !bRetval && b < nPolygonCount; b++)
            {
                const Polygon& rPathPolygon = aPathPolyPolygon[b];
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
    const sal_uInt32 nMarkCount(GetMarkedObjectCount());

    for(sal_uInt32 a(0); !(bOpen && bClosed) && a < nMarkCount; a++)
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

void SdrPolyEditView::ShutMarkedObjects()
{
    CloseMarkedObjects();
}

void SdrPolyEditView::CloseMarkedObjects(sal_Bool bToggle, sal_Bool bOpen) // , long nOpenDistance)
{
    if (AreObjectsMarked())
    {
        const bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditShut),GetDescriptionOfMarkedPoints());

        bool bChg=false;
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
        {
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pO=pM->GetMarkedSdrObj();
            sal_Bool bClosed=pO->IsClosedObj();
            if ((pO->IsPolyObj() && (bClosed==bOpen)) || bToggle)
            {
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pO));

                SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >( pO );
                if(pPathObj)
                    pPathObj->ToggleClosed();
                bChg=true;
            }
        }

        if( bUndo )
            EndUndo();

        if (bChg)
        {
            UnmarkAllPoints();
            MarkListHasChanged();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPolyEditView::ImpCopyMarkedPoints()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPolyEditView::ImpTransformMarkedPoints(PPolyTrFunc pTrFunc, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    const bool bUndo = IsUndoEnabled();

    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont* pPts=pM->GetMarkedPoints();
        sal_uIntPtr nPtAnz=pPts==NULL ? 0 : pPts->GetCount();
        SdrPathObj* pPath=PTR_CAST(SdrPathObj,pObj);
        if (nPtAnz!=0 && pPath!=NULL)
        {
            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

            basegfx::B2DPolyPolygon aXPP(pPath->GetPathPoly());

            for(sal_uInt32 nPtNum(0L); nPtNum < nPtAnz; nPtNum++)
            {
                sal_uInt32 nPt(pPts->GetObject(nPtNum));
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

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpMove(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* /*p2*/, const void* /*p3*/, const void* /*p4*/, const void* /*p5*/)
{
    MovePoint(rPt,*(const Size*)p1);
    if (pC1!=NULL) MovePoint(*pC1,*(const Size*)p1);
    if (pC2!=NULL) MovePoint(*pC2,*(const Size*)p1);
}

void SdrPolyEditView::MoveMarkedPoints(const Size& rSiz, bool bCopy)
{
    bCopy=sal_False; // noch nicht implementiert
    ForceUndirtyMrkPnt();
    XubString aStr(ImpGetResStr(STR_EditMove));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedPoints(),SDRREPFUNC_OBJ_MOVE);
    if (bCopy) ImpCopyMarkedPoints();
    ImpTransformMarkedPoints(ImpMove,&rSiz);
    EndUndo();
    AdjustMarkHdl();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpResize(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* p2, const void* p3, const void* /*p4*/, const void* /*p5*/)
{
    ResizePoint(rPt,*(const Point*)p1,*(const Fraction*)p2,*(const Fraction*)p3);
    if (pC1!=NULL) ResizePoint(*pC1,*(const Point*)p1,*(const Fraction*)p2,*(const Fraction*)p3);
    if (pC2!=NULL) ResizePoint(*pC2,*(const Point*)p1,*(const Fraction*)p2,*(const Fraction*)p3);
}

void SdrPolyEditView::ResizeMarkedPoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy)
{
    bCopy=sal_False; // noch nicht implementiert
    ForceUndirtyMrkPnt();
    XubString aStr(ImpGetResStr(STR_EditResize));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedPoints(),SDRREPFUNC_OBJ_RESIZE);
    if (bCopy) ImpCopyMarkedPoints();
    ImpTransformMarkedPoints(ImpResize,&rRef,&xFact,&yFact);
    EndUndo();
    AdjustMarkHdl();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpRotate(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* /*p2*/, const void* p3, const void* p4, const void* /*p5*/)
{
    RotatePoint(rPt,*(const Point*)p1,*(const double*)p3,*(const double*)p4);
    if (pC1!=NULL) RotatePoint(*pC1,*(const Point*)p1,*(const double*)p3,*(const double*)p4);
    if (pC2!=NULL) RotatePoint(*pC2,*(const Point*)p1,*(const double*)p3,*(const double*)p4);
}

void SdrPolyEditView::RotateMarkedPoints(const Point& rRef, long nWink, bool bCopy)
{
    bCopy=sal_False; // noch nicht implementiert
    ForceUndirtyMrkPnt();
    XubString aStr(ImpGetResStr(STR_EditResize));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedPoints(),SDRREPFUNC_OBJ_ROTATE);
    if (bCopy) ImpCopyMarkedPoints();
    double nSin=sin(nWink*nPi180);
    double nCos=cos(nWink*nPi180);
    ImpTransformMarkedPoints(ImpRotate,&rRef,&nWink,&nSin,&nCos);
    EndUndo();
    AdjustMarkHdl();
}

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
