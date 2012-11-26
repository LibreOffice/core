/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

SdrPolyEditView::SdrPolyEditView(SdrModel& rModel1, OutputDevice* pOut)
:   SdrEditView(rModel1, pOut),
    mbSetMarkedPointsSmoothPossible(false),
    mbSetMarkedSegmentsKindPossible(false),
    meMarkedPointsSmooth(SDRPATHSMOOTH_DONTCARE),
    meMarkedSegmentsKind(SDRPATHSEGMENT_DONTCARE)
{
}

SdrPolyEditView::~SdrPolyEditView()
{
}

void SdrPolyEditView::ImpCheckPolyPossibilities()
{
    mbSetMarkedPointsSmoothPossible = false;
    mbSetMarkedSegmentsKindPossible = false;
    meMarkedPointsSmooth = SDRPATHSMOOTH_DONTCARE;
    meMarkedSegmentsKind = SDRPATHSEGMENT_DONTCARE;

    if(areSdrObjectsSelected() && !ImpIsFrameHandles())
    {
        bool b1stSmooth(true);
        bool b1stSegm(true);
        bool bCurve(false);
        bool bSmoothFuz(false);
        bool bSegmFuz(false);
        basegfx::B2VectorContinuity eSmooth = basegfx::CONTINUITY_NONE;
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size(); nMarkNum++)
        {
            const SdrPathObj* pPath = dynamic_cast< const SdrPathObj* >(aSelection[nMarkNum]);

            if(pPath)
            {
                const sdr::selection::Indices aMarkedPoints(getSelectedPointsForSelectedSdrObject(*pPath));

                if(aMarkedPoints.size())
                {
                    CheckPolyPossibilitiesHelper(
                        *pPath,
                        aMarkedPoints,
                        b1stSmooth, b1stSegm, bCurve, bSmoothFuz, bSegmFuz, eSmooth);
                }
            }
        }
    }
}

void SdrPolyEditView::CheckPolyPossibilitiesHelper(
    const SdrPathObj& rMarkedSdrPathObj,
    const sdr::selection::Indices& rMarkedPoints,
    bool& b1stSmooth, bool& b1stSegm, bool& bCurve,
    bool& bSmoothFuz, bool& bSegmFuz, basegfx::B2VectorContinuity& eSmooth)
{
    const sal_uInt32 nMarkedPntAnz(rMarkedPoints.size());

    if(nMarkedPntAnz)
    {
        const bool bClosed(rMarkedSdrPathObj.isClosed());
        mbSetMarkedPointsSmoothPossible = true;

        if(bClosed)
        {
            mbSetMarkedSegmentsKindPossible = true;
        }

        const basegfx::B2DPolyPolygon aMarkedPolyPolygon(rMarkedSdrPathObj.getB2DPolyPolygonInObjectCoordinates());

        for(sdr::selection::Indices::iterator aCurrent(rMarkedPoints.begin());
            aCurrent != rMarkedPoints.end(); aCurrent++)
        {
            sal_uInt32 nNum(*aCurrent);
            sal_uInt32 nPolyNum, nPntNum;

            if(PolyPolygonEditor::GetRelativePolyPoint(aMarkedPolyPolygon, nNum, nPolyNum, nPntNum))
            {
                const basegfx::B2DPolygon aLocalPolygon(aMarkedPolyPolygon.getB2DPolygon(nPolyNum));
                bool bCanSegment(bClosed || nPntNum < aLocalPolygon.count() - 1L);

                if(!mbSetMarkedSegmentsKindPossible && bCanSegment)
                {
                    mbSetMarkedSegmentsKindPossible = true;
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
                meMarkedPointsSmooth = SDRPATHSMOOTH_ANGULAR;
            }

            if(basegfx::CONTINUITY_C1 == eSmooth)
            {
                meMarkedPointsSmooth = SDRPATHSMOOTH_ASYMMETRIC;
            }

            if(basegfx::CONTINUITY_C2 == eSmooth)
            {
                meMarkedPointsSmooth = SDRPATHSMOOTH_SYMMETRIC;
            }
        }

        if(!b1stSegm && !bSegmFuz)
        {
            meMarkedSegmentsKind = (bCurve) ? SDRPATHSEGMENT_CURVE : SDRPATHSEGMENT_LINE;
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
        const bool bUndo(IsUndoEnabled());
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if( bUndo )
        {
            BegUndo(ImpGetResStr(STR_EditSetPointsSmooth), getSelectedPointsDescription());
        }

        for(sal_uInt32 nMarkNum(aSelection.size()); nMarkNum > 0;)
        {
            nMarkNum--;
            const sdr::selection::Indices aMarkedPoints(getSelectedPointsForSelectedSdrObject(*aSelection[nMarkNum]));
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >(aSelection[nMarkNum]);

            if(aMarkedPoints.size() && pPath)
            {
                basegfx::B2DPolyPolygon aPolyPolygon(pPath->getB2DPolyPolygonInObjectCoordinates());

                if(sdr::PolyPolygonEditor::SetPointsSmooth(aPolyPolygon, eFlags, aMarkedPoints))
                {
                    if( bUndo )
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pPath));
                    }

                    pPath->setB2DPolyPolygonInObjectCoordinates(aPolyPolygon);
                }
            }
        }

        if( bUndo )
        {
            EndUndo();
        }
    }
}

void SdrPolyEditView::SetMarkedSegmentsKind(SdrPathSegmentKind eKind)
{
    if(HasMarkedPoints())
    {
        const bool bUndo(IsUndoEnabled());
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if( bUndo )
        {
            BegUndo(ImpGetResStr(STR_EditSetSegmentsKind), getSelectedPointsDescription());
        }

        for(sal_uInt32 nMarkNum(aSelection.size()); nMarkNum > 0;)
        {
            nMarkNum--;
            const sdr::selection::Indices aMarkedPoints(getSelectedPointsForSelectedSdrObject(*aSelection[nMarkNum]));
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >(aSelection[nMarkNum]);

            if(aMarkedPoints.size() && pPath)
            {
                basegfx::B2DPolyPolygon aPolyPolygon(pPath->getB2DPolyPolygonInObjectCoordinates());

                if(sdr::PolyPolygonEditor::SetSegmentsKind(aPolyPolygon, eKind, aMarkedPoints))
                {
                    if( bUndo )
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pPath));
                    }

                    pPath->setB2DPolyPolygonInObjectCoordinates(aPolyPolygon);
                }
            }
        }

        if( bUndo )
        {
            EndUndo();
        }
    }
}

bool SdrPolyEditView::IsSetMarkedPointsSmoothPossible() const
{
    ForcePossibilities();

    return mbSetMarkedPointsSmoothPossible;
}

SdrPathSmoothKind SdrPolyEditView::GetMarkedPointsSmooth() const
{
    ForcePossibilities();

    return meMarkedPointsSmooth;
}

bool SdrPolyEditView::IsSetMarkedSegmentsKindPossible() const
{
    ForcePossibilities();

    return mbSetMarkedSegmentsKindPossible;
}

SdrPathSegmentKind SdrPolyEditView::GetMarkedSegmentsKind() const
{
    ForcePossibilities();

    return meMarkedSegmentsKind;
}

bool SdrPolyEditView::IsDeleteMarkedPointsPossible() const
{
    return HasMarkedPoints();
}

void SdrPolyEditView::DeleteMarkedPoints()
{
    if(HasMarkedPoints() && GetSdrPageView())
    {
        BrkAction();

        const bool bUndo(IsUndoEnabled());
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if( bUndo )
        {
            // Description
            BegUndo(ImpGetResStr(STR_EditDelete), getSelectedPointsDescription(), SDRREPFUNC_OBJ_DELETE);
        }

        for(sal_uInt32 nMarkNum(aSelection.size()); nMarkNum > 0;)
        {
            nMarkNum--;
            const sdr::selection::Indices aMarkedPoints(getSelectedPointsForSelectedSdrObject(*aSelection[nMarkNum]));
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >(aSelection[nMarkNum]);

            if(pPath && aMarkedPoints.size())
            {
                basegfx::B2DPolyPolygon aPolyPolygon(pPath->getB2DPolyPolygonInObjectCoordinates());

                if(sdr::PolyPolygonEditor::DeletePoints(aPolyPolygon, aMarkedPoints))
                {
                    if(aPolyPolygon.count())
                    {
                        if( bUndo )
                        {
                            AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pPath));
                        }

                        pPath->setB2DPolyPolygonInObjectCoordinates(aPolyPolygon);
                    }
                    else
                    {
                        if( bUndo )
                        {
                            AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoDeleteObject(*pPath));
                        }

                        GetSdrPageView()->GetCurrentObjectList()->RemoveObjectFromSdrObjList(pPath->GetNavigationPosition());

                        if( !bUndo )
                        {
                            SdrObject* pObj = pPath;
                            deleteSdrObjectSafeAndClearPointer(pObj);
                        }
                    }
                }
            }
        }

        if( bUndo )
        {
            EndUndo();
        }

        MarkPoints(0, true); // unmarkall
    }
}

void SdrPolyEditView::RipUpAtMarkedPoints()
{
    if(HasMarkedPoints() && GetSdrPageView())
    {
        const bool bUndo(IsUndoEnabled());
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if( bUndo )
        {
            BegUndo(ImpGetResStr(STR_EditRipUp), getSelectedPointsDescription());
        }

        for(sal_uInt32 nMarkNum(aSelection.size()); nMarkNum > 0;)
        {
            nMarkNum--;
            sdr::selection::Indices aMarkedPoints(getSelectedPointsForSelectedSdrObject(*aSelection[nMarkNum]));
            bool bMarkedPointsChanged(false);
            SdrPathObj* pObj = dynamic_cast< SdrPathObj* >(aSelection[nMarkNum]);

            if(aMarkedPoints.size() && pObj)
            {
                if( bUndo )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                }

                bool bKorregFlag(false);
                bool bInsAny(false);
                const sal_uInt32 nMax(pObj->getB2DPolyPolygonInObjectCoordinates().allPointCount());

                for(sdr::selection::Indices::reverse_iterator aCurrent(aMarkedPoints.rbegin());
                    aCurrent != aMarkedPoints.rend(); aCurrent++)
                {
                    sal_uInt32 nNewPt0Idx(0);
                    SdrObject* pNeuObj = pObj->RipPoint(*aCurrent, nNewPt0Idx);

                    if(pNeuObj)
                    {
                        bInsAny = true;
                        GetSdrPageView()->GetCurrentObjectList()->InsertObjectToSdrObjList(
                            *pNeuObj,
                            pObj->GetNavigationPosition() + 1);

                        if( bUndo )
                        {
                            AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pNeuObj));
                        }

                        MarkObj(*pNeuObj, false);
                    }

                    if(nNewPt0Idx)
                    {
                        // Korrektur notwendig?
                        DBG_ASSERT(bKorregFlag==false,"Mehrfache Indexkorrektur bei SdrPolyEditView::RipUp()");

                        if(!bKorregFlag)
                        {
                            bKorregFlag = true;

                            for(sdr::selection::Indices::iterator aCorrect(aMarkedPoints.begin());
                                aCorrect != aMarkedPoints.end();)
                            {
                                sal_uInt32 nPntNum(*aCorrect);

                                nPntNum += nNewPt0Idx;

                                if(nPntNum >= nMax)
                                {
                                    nPntNum -= nMax;
                                }

                                sdr::selection::Indices::iterator aNext(aCorrect);
                                aNext++;
                                aMarkedPoints.erase(aCorrect);
                                aMarkedPoints.insert(nPntNum);
                                bMarkedPointsChanged = true;
                                aCorrect = aNext;
                            }

                            // start again
                            aCurrent = aMarkedPoints.rbegin();
                        }
                    }
                }
            }

            if(bMarkedPointsChanged)
            {
                setSelectedPointsForSelectedSdrObject(*aSelection[nMarkNum], aMarkedPoints);
            }
        }

        MarkPoints(0, true); // unmarkall

        if( bUndo )
        {
            EndUndo();
        }
    }
}

bool SdrPolyEditView::IsRipUpAtMarkedPointsPossible() const
{
    bool bRetval(false);

    if(HasMarkedPoints())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            const SdrPathObj* pMarkedPathObject = dynamic_cast< const SdrPathObj* >(aSelection[a]);

            if(pMarkedPathObject)
            {
                const sdr::selection::Indices aMarkedPoints(getSelectedPointsForSelectedSdrObject(*pMarkedPathObject));

                if(aMarkedPoints.size())
                {
                    const basegfx::B2DPolyPolygon aPathPolyPolygon = pMarkedPathObject->getB2DPolyPolygonInObjectCoordinates();

                    if(1 == aPathPolyPolygon.count())
                    {
                        // #i76617# Do not yet use basegfx::B2DPolygon since curve definitions
                        // are different and methods need to be changed thoroughly with interaction rework
                        const Polygon aPathPolygon(aPathPolyPolygon.getB2DPolygon(0));
                        const sal_uInt32 nPointCount(aPathPolygon.GetSize());

                        if(nPointCount >= 3)
                        {
                            bRetval = pMarkedPathObject->IsClosedObj(); // #i76617# aPathPolygon.isClosed();

                            for(sdr::selection::Indices::const_iterator aCurrent(aMarkedPoints.begin());
                                !bRetval && aCurrent != aMarkedPoints.end(); aCurrent++)
                            {
                                const sal_uInt32 nMarkedPointNum(*aCurrent);

                                bRetval = (nMarkedPointNum > 0 && nMarkedPointNum < nPointCount - 1);
                            }
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

    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            const SdrPathObj* pMarkedPathObject = dynamic_cast< const SdrPathObj* >(aSelection[a]);

            if(pMarkedPathObject)
            {
                // #i76617# Do not yet use basegfx::B2DPolygon since curve definitions
                // are different and methods need to be changed thoroughly with interaction rework
                const PolyPolygon aPathPolyPolygon(pMarkedPathObject->getB2DPolyPolygonInObjectCoordinates());
                const sal_uInt16 nPolygonCount(aPathPolyPolygon.Count());

                for(sal_uInt16 b(0); !bRetval && b < nPolygonCount; b++)
                {
                    const Polygon& rPathPolygon = aPathPolyPolygon[b];
                    const sal_uInt16 nPointCount(rPathPolygon.GetSize());

                    bRetval = (nPointCount >= 3);
                }
            }
        }
    }

    return bRetval;
}

SdrObjClosedKind SdrPolyEditView::GetMarkedObjectsClosedState() const
{
    if(areSdrObjectsSelected())
    {
        bool bOpen(false);
        bool bClosed(false);
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 a(0); !(bOpen && bClosed) && a < aSelection.size(); a++)
        {
            const SdrPathObj* pMarkedPathObject = dynamic_cast< const SdrPathObj* >(aSelection[a]);

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
    else
    {
        return SDROBJCLOSED_DONTCARE;
    }
}

void SdrPolyEditView::CloseMarkedObjects(bool bToggle, bool bOpen)
{
    if(areSdrObjectsSelected())
    {
        const bool bUndo(IsUndoEnabled());
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        bool bChg(false);

        if( bUndo )
        {
            BegUndo(ImpGetResStr(STR_EditShut), getSelectedPointsDescription());
        }

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pO = aSelection[nm];
            const bool bClosed(pO->IsClosedObj());

            if(pO->IsPolygonObject() && (bClosed == bOpen) || bToggle)
            {
                if( bUndo )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pO));
                }

                SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >( pO );

                if(pPathObj)
                {
                    pPathObj->ToggleClosed();
                }

                bChg=true;
            }
        }

        if( bUndo )
        {
            EndUndo();
        }

        if (bChg)
        {
            MarkPoints(0, true); // unmarkall
        }
    }
}

void SdrPolyEditView::TransformMarkedPoints(
    const basegfx::B2DHomMatrix& rTransformation,
    const SdrRepeatFunc aRepFunc,
    bool bCopy)
{
    // not yet supported
    bCopy = false;

    if(HasMarkedPoints() && !rTransformation.isIdentity())
    {
        const bool bUndo(IsUndoEnabled());
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        XubString aStr;

        switch(aRepFunc)
        {
            default: //case SDRREPFUNC_OBJ_MOVE:
                aStr = ImpGetResStr(STR_EditMove);
                break;
            case SDRREPFUNC_OBJ_RESIZE:
                aStr = ImpGetResStr(STR_EditResize);
                break;
            case SDRREPFUNC_OBJ_ROTATE:
                aStr = ImpGetResStr(STR_EditResize); // no own string for rotate ?!?
                break;
        }

        if(bCopy)
        {
            aStr += ImpGetResStr(STR_EditWithCopy);
        }

        if(bUndo)
        {
            BegUndo(aStr, getSelectedPointsDescription(), aRepFunc);
        }

        if(bCopy)
        {
            // no implementation yet
            OSL_ENSURE(false, "Missing implementation: Copy selected points (!)");
        }

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            const sdr::selection::Indices aMarkedPoints(getSelectedPointsForSelectedSdrObject(*aSelection[nm]));
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >(aSelection[nm]);

            if(aMarkedPoints.size() && pPath)
            {
                if( bUndo )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pPath));
                }

                basegfx::B2DPolyPolygon aXPP(pPath->getB2DPolyPolygonInObjectCoordinates());

                for(sdr::selection::Indices::const_iterator aCurrent(aMarkedPoints.begin());
                    aCurrent != aMarkedPoints.end(); aCurrent++)
                {
                    const sal_uInt32 nPt(*aCurrent);
                    sal_uInt32 nPolyNum, nPointNum;

                    if(PolyPolygonEditor::GetRelativePolyPoint(aXPP, nPt, nPolyNum, nPointNum))
                    {
                        //#i83671# used nLocalPointNum (which was the polygon point count)
                        // instead of the point index (nPointNum). This of course leaded
                        // to a wrong point access to the B2DPolygon.
                        basegfx::B2DPolygon aNewXP(aXPP.getB2DPolygon(nPolyNum));

                        // transform single point
                        aNewXP.setB2DPoint(nPointNum, rTransformation * aNewXP.getB2DPoint(nPointNum));

                        if(aNewXP.isPrevControlPointUsed(nPointNum))
                        {
                                // transform control point
                                aNewXP.setPrevControlPoint(nPointNum, rTransformation * aNewXP.getPrevControlPoint(nPointNum));
                        }

                        if(aNewXP.isNextControlPointUsed(nPointNum))
                        {
                                // transform control point
                                aNewXP.setNextControlPoint(nPointNum, rTransformation * aNewXP.getNextControlPoint(nPointNum));
                        }

                        aXPP.setB2DPolygon(nPolyNum, aNewXP);
                    }
                }

                pPath->setB2DPolyPolygonInObjectCoordinates(aXPP);
            }
        }

        if(bUndo)
        {
            EndUndo();
        }

        SetMarkHandles();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
