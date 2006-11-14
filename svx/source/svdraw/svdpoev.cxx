/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdpoev.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:49:18 $
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
#include "precompiled_svx.hxx"

#include "svdpoev.hxx"
#include <math.h>
#include "svdpagv.hxx"
#include "svdpage.hxx"
#include "svdopath.hxx"
#include "svdundo.hxx"
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache
#include "svdtrans.hxx"

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPolyEditView::ImpResetPolyPossibilityFlags()
{
    eMarkedPointsSmooth=SDRPATHSMOOTH_DONTCARE;
    eMarkedSegmentsKind=SDRPATHSEGMENT_DONTCARE;
    bSetMarkedPointsSmoothPossible=FALSE;
    bSetMarkedSegmentsKindPossible=FALSE;
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
    const sal_uInt32 nMarkAnz(GetMarkedObjectCount());

    if(nMarkAnz && !ImpIsFrameHandles())
    {
        bool bLocalReadOnly(false);
        bool b1stSmooth(true);
        bool b1stSegm(true);
        bool bCurve(false);
        bool bSmoothFuz(false);
        bool bSegmFuz(false);
        basegfx::B2VectorContinuity eSmooth = basegfx::CONTINUITY_NONE;

        for(sal_uInt32 nMarkNum(0L); nMarkNum < nMarkAnz; nMarkNum++)
        {
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
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

                        if(SdrPathObj::ImpFindPolyPnt(pPath->GetPathPoly(), nNum, nPolyNum, nPntNum))
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
                                    bool bCrv(!aLocalPolygon.getControlVectorA(nPntNum).equalZero());

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

        if(bLocalReadOnly)
        {
            bSetMarkedPointsSmoothPossible = false;
            bSetMarkedSegmentsKindPossible = false;
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
        BegUndo(ImpGetResStr(STR_EditSetPointsSmooth), GetDescriptionOfMarkedPoints());
        sal_uInt32 nMarkAnz(GetMarkedObjectCount());

        for(sal_uInt32 nMarkNum(nMarkAnz); nMarkNum > 0L;)
        {
            nMarkNum--;
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts = pM->GetMarkedPoints();
            SdrObject* pObj = pM->GetMarkedSdrObj();
            SdrPathObj* pPath = PTR_CAST(SdrPathObj, pObj);

            if(pPts && pPath)
            {
                pPts->ForceSort();
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pPath));

                // new method will do the work outside of SdrObject
                basegfx::B2DPolyPolygon aNewPolyPolygon(pPath->GetPathPoly());
                bool bPolyPolygonChanged(false);

                for(sal_uInt32 nNum(pPts->GetCount()); nNum > 0L;)
                {
                    nNum--;
                    sal_uInt32 nPtNum(pPts->GetObject(nNum));
                    sal_uInt32 nPolyNum, nPntNum;

                    if(SdrPathObj::ImpFindPolyPnt(aNewPolyPolygon, nPtNum, nPolyNum, nPntNum))
                    {
                        // do change at aNewPolyPolygon...
                        basegfx::B2DPolygon aCandidate(aNewPolyPolygon.getB2DPolygon(nPolyNum));

                        // set continuity in point, make sure there is a curve
                        bool bPolygonChanged(false);
                        bPolygonChanged = basegfx::tools::expandToCurveInPoint(aCandidate, nPntNum);
                        bPolygonChanged |= basegfx::tools::setContinuityInPoint(aCandidate, nPntNum, eFlags);

                        if(bPolygonChanged)
                        {
                            aNewPolyPolygon.setB2DPolygon(nPolyNum, aCandidate);
                            bPolyPolygonChanged = true;
                        }
                    }
                }

                // set changed polygon
                if(bPolyPolygonChanged)
                {
                    pPath->SetPathPoly(aNewPolyPolygon);
                }
            }
        }

        EndUndo();
    }
}

void SdrPolyEditView::SetMarkedSegmentsKind(SdrPathSegmentKind eKind)
{
    if(HasMarkedPoints())
    {
        SortMarkedObjects();
        BegUndo(ImpGetResStr(STR_EditSetSegmentsKind), GetDescriptionOfMarkedPoints());
        sal_uInt32 nMarkAnz(GetMarkedObjectCount());

        for(sal_uInt32 nMarkNum(nMarkAnz); nMarkNum > 0L;)
        {
            nMarkNum--;
            SdrMark* pM = GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts = pM->GetMarkedPoints();
            SdrObject* pObj = pM->GetMarkedSdrObj();
            SdrPathObj* pPath = PTR_CAST(SdrPathObj,pObj);

            if(pPts && pPath)
            {
                pPts->ForceSort();
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pPath));

                // new method will do the work outside of SdrObject
                basegfx::B2DPolyPolygon aNewPolyPolygon(pPath->GetPathPoly());
                bool bPolyPolyChanged(false);

                for(sal_uInt32 nNum(pPts->GetCount()); nNum > 0L;)
                {
                    nNum--;
                    sal_uInt32 nPtNum(pPts->GetObject(nNum));
                    sal_uInt32 nPolyNum, nPntNum;

                    if(SdrPathObj::ImpFindPolyPnt(aNewPolyPolygon, nPtNum, nPolyNum, nPntNum))
                    {
                        // do change at aNewPolyPolygon. Take a look at edge.
                        basegfx::B2DPolygon aCandidate(aNewPolyPolygon.getB2DPolygon(nPolyNum));
                        bool bCandidateChanged(false);
                        const sal_uInt32 nCount(aCandidate.count());

                        if(nCount && (nPntNum < nCount || aCandidate.isClosed()))
                        {
                            const bool bContolUsed(aCandidate.areControlVectorsUsed() &&
                                !aCandidate.getControlVectorA(nPntNum).equalZero() &&
                                !aCandidate.getControlVectorB(nPntNum).equalZero());

                            if(bContolUsed)
                            {
                                if(SDRPATHSEGMENT_TOGGLE == eKind || SDRPATHSEGMENT_LINE == eKind)
                                {
                                    // remove control
                                    const basegfx::B2DVector aEmptyVector;
                                    aCandidate.setControlVectorA(nPntNum, aEmptyVector);
                                    aCandidate.setControlVectorB(nPntNum, aEmptyVector);
                                    bCandidateChanged = true;
                                }
                            }
                            else
                            {
                                if(SDRPATHSEGMENT_TOGGLE == eKind || SDRPATHSEGMENT_CURVE == eKind)
                                {
                                    // add control
                                    const sal_uInt32 nNext(basegfx::tools::getIndexOfSuccessor(nPntNum, aCandidate));
                                    const basegfx::B2DPoint aStart(aCandidate.getB2DPoint(nPntNum));
                                    const basegfx::B2DPoint aEnd(aCandidate.getB2DPoint(nNext));
                                    aCandidate.setControlPointA(nPntNum, interpolate(aStart, aEnd, (1.0 / 3.0)));
                                    aCandidate.setControlPointB(nPntNum, interpolate(aStart, aEnd, (2.0 / 3.0)));
                                    bCandidateChanged = true;
                                }
                            }

                            if(bCandidateChanged)
                            {
                                aNewPolyPolygon.setB2DPolygon(nPolyNum, aCandidate);
                                bPolyPolyChanged = true;
                            }
                        }
                    }
                }

                // set changed polygon
                if(bPolyPolyChanged)
                {
                    pPath->SetPathPoly(aNewPolyPolygon);
                }
            }
        }

        EndUndo();
    }
}

void SdrPolyEditView::DeleteMarkedPoints()
{
    if (HasMarkedPoints()) {
        BrkAction();
        SortMarkedObjects();
        ULONG nMarkAnz=GetMarkedObjectCount();
        // Description
        BegUndo(ImpGetResStr(STR_EditDelete),GetDescriptionOfMarkedPoints(),SDRREPFUNC_OBJ_DELETE);
        // und nun das eigentliche loeschen
        ULONG nObjDelCount=0;
        for (ULONG nMarkNum=nMarkAnz; nMarkNum>0;) {
            nMarkNum--;
            SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts=pM->GetMarkedPoints();
            SdrPathObj* pObj = PTR_CAST(SdrPathObj, pM->GetMarkedSdrObj());
            BOOL bDel=FALSE;
            if (pPts!=NULL)
            {
                pPts->ForceSort();
                ULONG nMarkPtsAnz=pPts->GetCount();

                if(nMarkPtsAnz && pObj)
                {
                    basegfx::B2DPolyPolygon aPathPolygon(pObj->GetPathPoly());
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

                    for (ULONG i=nMarkPtsAnz; i>0 && !bDel;)
                    {
                        i--;

                        sal_uInt32 nPoly, nPnt;
                        if(SdrPathObj::ImpFindPolyPnt(aPathPolygon, pPts->GetObject(i), nPoly, nPnt))
                        {
                            basegfx::B2DPolygon aCandidate(aPathPolygon.getB2DPolygon(nPoly));

                            if(aCandidate.areControlVectorsUsed()
                                && aCandidate.count() > 1L
                                && (aCandidate.isClosed() || nPnt)
                                && !aCandidate.getControlVectorB(nPnt).equalZero())
                            {
                                // copy control vector to predecessor to rescue it
                                const sal_uInt32 nPredecessor(basegfx::tools::getIndexOfPredecessor(nPnt, aCandidate));
                                aCandidate.setControlPointB(nPredecessor, aCandidate.getControlPointB(nPnt));
                            }

                            // remove point
                            aCandidate.remove(nPnt);

                            if((pObj->IsClosed() && aCandidate.count() < 3L) || aCandidate.count() < 2L)
                            {
                                aPathPolygon.remove(nPoly);
                            }
                            else
                            {
                                aPathPolygon.setB2DPolygon(nPoly, aCandidate);
                            }
                        }
                    }

                    if(aPathPolygon.count())
                    {
                        pObj->SetPathPoly(aPathPolygon);
                    }
                    else
                    {
                        bDel = true;
                    }
                }
            }

            if(bDel)
            {
                AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj) );
                pM->GetPageView()->GetObjList()->RemoveObject(pObj->GetOrdNum());
                nObjDelCount++;
            }
        }

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
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                sal_Bool bKorregFlag(sal_False);
                sal_Bool bInsAny(sal_False);
                sal_uInt32 nMarkPtsAnz(pPts->GetCount());
                sal_uInt32 nMax(pObj->GetHdlCount());

                for(sal_uInt32 i(nMarkPtsAnz); i > 0L;)
                {
                    i--;
                    sal_uInt32 nNewPt0Idx(0L);
                    SdrObject* pNeuObj = pObj->RipPoint(pPts->GetObject(i), nNewPt0Idx);

                    if(pNeuObj)
                    {
                        bInsAny = sal_True;
                        SdrInsertReason aReason(SDRREASON_VIEWCALL, pObj);
                        pM->GetPageView()->GetObjList()->InsertObject(pNeuObj, pObj->GetOrdNum() + 1, &aReason);
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pNeuObj));
                        MarkObj(pNeuObj, pM->GetPageView(), FALSE, TRUE);
                    }

                    if(nNewPt0Idx)
                    {
                        // Korrektur notwendig?
                        DBG_ASSERT(bKorregFlag==FALSE,"Mehrfache Indexkorrektur bei SdrPolyEditView::RipUp()");
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
        EndUndo();
        MarkListHasChanged();
    }
}

BOOL SdrPolyEditView::IsRipUpAtMarkedPointsPossible() const
{
    BOOL bRet=FALSE;
    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
        SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrUShortCont* pPts=pM->GetMarkedPoints();
        ULONG nMarkPntAnz=pPts!=NULL ? pPts->GetCount() : 0;
        if (pPts!=NULL && nMarkPntAnz!=0 && pObj!=NULL && pObj->ISA(SdrPathObj)) {
            const XPolyPolygon& rXPP=((SdrPathObj*)pObj)->GetPathPoly();
            if (rXPP.Count()==1) { // es muss genau 1 Polygon drin sein!
                const XPolygon& rXP=rXPP[0];
                USHORT nPntAnz=rXP.GetPointCount();
                if (nPntAnz>=3) {
                    bRet=pObj->IsClosedObj();
                    if (!bRet) {
                        for (ULONG nMarkPntNum=0; nMarkPntNum<nMarkPntAnz && !bRet; nMarkPntNum++) {
                            USHORT nMarkPt=pPts->GetObject(nMarkPntNum);
                            bRet=nMarkPt>0 && nMarkPt<nPntAnz-1;
                        }
                    }
                }
            }
        }
    }
    return bRet;
}

BOOL SdrPolyEditView::IsOpenCloseMarkedObjectsPossible() const
{
    BOOL bRet=FALSE;
    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nm=0; nm<nMarkAnz && !bRet; nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        if (pO->ISA(SdrPathObj)) {
            const XPolyPolygon& rXPP=((SdrPathObj*)pO)->GetPathPoly();
            USHORT nPolyAnz=rXPP.Count();
            for (USHORT nPolyNum=0; nPolyNum<nPolyAnz && !bRet; nPolyNum++) {
                const XPolygon& rXP=rXPP[nPolyNum];
                USHORT nPntAnz=rXP.GetPointCount();
                bRet=nPntAnz>=3;
            }
        }
        else
            return FALSE;
    }
    return bRet;
}

SdrObjClosedKind SdrPolyEditView::GetMarkedObjectsClosedState() const
{
    BOOL bOpen=FALSE;
    BOOL bClosed=FALSE;
    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nm=0; nm<nMarkAnz && (!bOpen || !bClosed); nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        if (pO->ISA(SdrPathObj)) {
            const XPolyPolygon& rXPP=((SdrPathObj*)pO)->GetPathPoly();
            if (rXPP.Count()==1) { // es muss genau 1 Polygon drin sein!
                const XPolygon& rXP=rXPP[0];
                USHORT nPntAnz=rXP.GetPointCount();
                if (nPntAnz>=3) {
                    if (pO->IsClosedObj()) bClosed=TRUE; else bOpen=TRUE;
                }
            }
        }
    }
    if (bOpen && bClosed) return SDROBJCLOSED_DONTCARE;
    if (bOpen) return SDROBJCLOSED_OPEN;
    return SDROBJCLOSED_CLOSED;
}

void SdrPolyEditView::ShutMarkedObjects()
{
    CloseMarkedObjects();
}

void SdrPolyEditView::CloseMarkedObjects(BOOL bToggle, BOOL bOpen) // , long nOpenDistance)
{
    if (AreObjectsMarked()) {
        BegUndo(ImpGetResStr(STR_EditShut),GetDescriptionOfMarkedPoints());
        BOOL bChg=FALSE;
        ULONG nMarkAnz=GetMarkedObjectCount();
        for (ULONG nm=0; nm<nMarkAnz; nm++) {
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pO=pM->GetMarkedSdrObj();
            BOOL bClosed=pO->IsClosedObj();
            if (pO->IsPolyObj() && (bClosed==bOpen) || bToggle) {
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pO));
                if (pO->ISA(SdrPathObj)) {
                    ((SdrPathObj*)pO)->ToggleClosed(); // nOpenDistance);
                }
                bChg=TRUE;
            }
        }
        EndUndo();
        if (bChg) {
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
    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont* pPts=pM->GetMarkedPoints();
        ULONG nPtAnz=pPts==NULL ? 0 : pPts->GetCount();
        SdrPathObj* pPath=PTR_CAST(SdrPathObj,pObj);
        if (nPtAnz!=0 && pPath!=NULL)
        {
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
            basegfx::B2DPolyPolygon aXPP(pPath->GetPathPoly());

            for(sal_uInt32 nPtNum(0L); nPtNum < nPtAnz; nPtNum++)
            {
                sal_uInt32 nPt(pPts->GetObject(nPtNum));
                sal_uInt32 nPolyNum, nPointNum;

                if(SdrPathObj::ImpFindPolyPnt(aXPP, nPt, nPolyNum, nPointNum))
                {
                    basegfx::B2DPolygon aNewXP(aXPP.getB2DPolygon(nPolyNum));
                    const sal_uInt32 nLocalPointNum(aNewXP.count());
                    Point aPos, aC1, aC2;
                    bool bC1(false);
                    bool bC2(false);

                    const basegfx::B2DPoint aB2DPos(aNewXP.getB2DPoint(nLocalPointNum));
                    aPos = Point(FRound(aB2DPos.getX()), FRound(aB2DPos.getY()));
                    const sal_uInt32 nPrevPointNum(basegfx::tools::getIndexOfPredecessor(nLocalPointNum, aNewXP));

                    if(nPrevPointNum != nLocalPointNum)
                    {
                        if(!aNewXP.getControlVectorB(nPrevPointNum).equalZero())
                        {
                            const basegfx::B2DPoint aB2DC1(aNewXP.getControlPointB(nPrevPointNum));
                            aC1 = Point(FRound(aB2DC1.getX()), FRound(aB2DC1.getY()));
                            bC1 = true;
                        }
                    }

                    if(!aNewXP.getControlVectorA(nLocalPointNum).equalZero())
                    {
                        const basegfx::B2DPoint aB2DC2(aNewXP.getControlPointA(nLocalPointNum));
                        aC2 = Point(FRound(aB2DC2.getX()), FRound(aB2DC2.getY()));
                        bC2 = true;
                    }

                    (*pTrFunc)(aPos,&aC1,&aC2,p1,p2,p3,p4,p5);
                    aNewXP.setB2DPoint(nLocalPointNum, basegfx::B2DPoint(aPos.X(), aPos.Y()));

                    if (bC1)
                    {
                        aNewXP.setControlPointB(nPrevPointNum, basegfx::B2DPoint(aC1.X(), aC1.Y()));
                    }

                    if (bC2)
                    {
                        aNewXP.setControlPointA(nLocalPointNum, basegfx::B2DPoint(aC2.X(), aC2.Y()));
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
    bCopy=FALSE; // noch nicht implementiert
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
    bCopy=FALSE; // noch nicht implementiert
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
    bCopy=FALSE; // noch nicht implementiert
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
