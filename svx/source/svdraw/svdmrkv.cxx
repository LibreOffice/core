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

#include <svx/svdmrkv.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoedge.hxx>
#include <editeng/editdata.hxx>
#include <svx/svdglob.hxx>
#include <svx/svditext.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include "svddrgm1.hxx"
#ifdef DBG_UTIL
#include <svdibrow.hxx>
#endif
#include <svx/svdoole2.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include "gradtrns.hxx"
#include <svx/xflftrit.hxx>
#include <svx/dialmgr.hxx>
#include "svx/svdstr.hrc"
#include <svx/svdundo.hxx>
#include <svx/svdopath.hxx>
#include <svx/scene3d.hxx>
#include <svx/sdr/overlay/overlayrollingrectangle.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdouno.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// selection visualisation of Objects, Points and GluePoints

class ImplMarkingOverlay
{
    // The OverlayObjects
    ::sdr::overlay::OverlayObjectList               maObjects;

    // The remembered second position in logical coodinates
    basegfx::B2DPoint                               maSecondPosition;

    // bitfield
    // A flag to remember if the action is for unmarking.
    bool                                            mbUnmarking : 1;

public:
    ImplMarkingOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos, bool bUnmarking = false);
    ~ImplMarkingOverlay();

    void SetSecondPosition(const basegfx::B2DPoint& rNewPosition);
    bool IsUnmarking() const { return mbUnmarking; }
};

ImplMarkingOverlay::ImplMarkingOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos, bool bUnmarking)
:   maSecondPosition(rStartPos),
    mbUnmarking(bUnmarking)
{
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        ::sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

        if(pTargetOverlay)
        {
            ::sdr::overlay::OverlayRollingRectangleStriped* pNew = new ::sdr::overlay::OverlayRollingRectangleStriped(
                rStartPos, rStartPos, false);
            pTargetOverlay->add(*pNew);
            maObjects.append(*pNew);
        }
    }
}

ImplMarkingOverlay::~ImplMarkingOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
}

void ImplMarkingOverlay::SetSecondPosition(const basegfx::B2DPoint& rNewPosition)
{
    if(rNewPosition != maSecondPosition)
    {
        // apply to OverlayObjects
        for(sal_uInt32 a(0L); a < maObjects.count(); a++)
        {
            ::sdr::overlay::OverlayRollingRectangleStriped& rCandidate = (::sdr::overlay::OverlayRollingRectangleStriped&)maObjects.getOverlayObject(a);
            rCandidate.setSecondPosition(rNewPosition);
        }

        // remember new position
        maSecondPosition = rNewPosition;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// global helpers for selection

const String getSelectionDescription(const SdrObjectVector& rSdrObjectVector)
{
    String aRetval;

    if(rSdrObjectVector.empty())
    {
        aRetval = ImpGetResStr(STR_ObjNameNoObj);
    }
    else
    {
        SdrObject* pMarkedObject = rSdrObjectVector[0];

        if(1 == rSdrObjectVector.size())
        {
            pMarkedObject->TakeObjNameSingul(aRetval);
        }
        else
        {
            pMarkedObject->TakeObjNamePlural(aRetval);
            String aCompare;
            bool bEq(true);

            for(SdrObjectVector::const_iterator aCandidate(rSdrObjectVector.begin() + 1);
                bEq && aCandidate != rSdrObjectVector.end(); aCandidate++)
            {
                (*aCandidate)->TakeObjNamePlural(aCompare);
                bEq = aRetval.Equals(aCompare);
            }

            if(!bEq)
            {
                aRetval = ImpGetResStr(STR_ObjNamePlural);
            }

            aRetval.Insert(sal_Unicode(' '), 0);
            aRetval.Insert(UniString::CreateFromInt32(rSdrObjectVector.size()), 0);
        }
    }

    return aRetval;
}

void sortSdrObjectSelection(SdrObjectVector& rSdrObjectVector)
{
    ::std::sort(rSdrObjectVector.begin(), rSdrObjectVector.end(), sdr::selection::SelectionComparator());
}

::std::set< SdrEdgeObj*, sdr::selection::SelectionComparator > getAllConnectedEdges(const SdrObjectVector& rSdrObjectVector)
{
    ::std::set< SdrEdgeObj*, sdr::selection::SelectionComparator > aRetval;

    if(!rSdrObjectVector.empty())
    {
        for(SdrObjectVector::const_iterator aCandidate(rSdrObjectVector.begin());
            aCandidate != rSdrObjectVector.end(); aCandidate++)
        {
            const ::std::vector< SdrEdgeObj* > aConnectedEdges((*aCandidate)->getAllConnectedSdrEdgeObj());

            if(!aConnectedEdges.empty())
            {
                for(::std::vector< SdrEdgeObj* >::const_iterator aEdge(aConnectedEdges.begin());
                    aEdge != aConnectedEdges.end(); aEdge++)
                {
                    if((*aEdge)->IsObjectInserted())
                    {
                        aRetval.insert(*aEdge);
                    }
                }
            }
        }
    }

    return aRetval;
}

::std::vector< SdrEdgeObj* > getAllSdrEdgeObjConnectedToSdrObjectVector(const SdrObjectVector& rSdrObjects, bool bContained)
{
    ::std::vector< SdrEdgeObj* > aRetval;

    if(!rSdrObjects.empty())
    {
        const ::std::set< SdrEdgeObj*, sdr::selection::SelectionComparator > aSetOfConnected(getAllConnectedEdges(rSdrObjects));

        if(!aSetOfConnected.empty())
        {
            ::std::set< SdrEdgeObj*, sdr::selection::SelectionComparator >::const_iterator aIter(aSetOfConnected.begin());
            const ::std::set< SdrObject*, sdr::selection::IndicesComparator > aSelectionSet(rSdrObjects.begin(), rSdrObjects.end());

            for(;aIter != aSetOfConnected.end(); aIter++)
            {
                const bool bIsContained(0 != aSelectionSet.count(*aIter));

                if(bContained == bIsContained)
                {
                    aRetval.push_back(*aIter);
                }
            }
        }
    }

    return aRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrMarkView::SdrMarkView(SdrModel& rModel1, OutputDevice* pOut)
:   SdrSnapView(rModel1, pOut),
    Timer(),
    mpMarkObjOverlay(0),
    mpMarkPointsOverlay(0),
    mpMarkGluePointsOverlay(0),
    maRef1(),
    maRef2(),
    maLastCrookCenter(),
    maViewSdrHandleList(*this),
    maSelection(*getAsSdrView()),
    mnSavePolyNum(0),
    mnSavePointNum(0),
    meSaveKind(HDL_MOVE),
    mpSaveObj(0),
    maMarkedPointRange(),
    maMarkedGluePointRange(),
    mnInsPointNum(0),
    meViewDragMode(SDRDRAG_MOVE),
    meViewEditMode(SDREDITMODE_EDIT),
    mbDesignMode(false),
    mbForceFrameHandles(false),
    mbPlusHdlAlways(false),
    mbInsPolyPoint(false),
    mbSaveOldFocus(false)
{
    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();

    StartListening(rModel1);
    SetTimeout(1);
}

SdrMarkView::~SdrMarkView()
{
    EndListening(getSdrModelFromSdrView());

    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrMarkView::IsAction() const
{
    return SdrSnapView::IsAction() || IsMarkObj() || IsMarkPoints() || IsMarkGluePoints();
}

void SdrMarkView::MovAction(const basegfx::B2DPoint& rPnt)
{
    SdrSnapView::MovAction(rPnt);

    if(IsMarkObj())
    {
        MovMarkObj(rPnt);
    }
    else if(IsMarkPoints())
    {
        MovMarkPoints(rPnt);
    }
    else if(IsMarkGluePoints())
    {
        MovMarkGluePoints(rPnt);
    }
}

void SdrMarkView::EndAction()
{
    if(IsMarkObj())
    {
        EndMarkObj();
    }
    else if(IsMarkPoints())
    {
        EndMarkPoints();
    }
    else if(IsMarkGluePoints())
    {
        EndMarkGluePoints();
    }

    SdrSnapView::EndAction();
}

void SdrMarkView::BckAction()
{
    SdrSnapView::BckAction();

    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
}

void SdrMarkView::BrkAction()
{
    SdrSnapView::BrkAction();

    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
}

basegfx::B2DRange SdrMarkView::TakeActionRange() const
{
    if(IsMarkObj() || IsMarkPoints() || IsMarkGluePoints())
    {
        return basegfx::B2DRange(GetDragStat().GetStart(), GetDragStat().GetNow());
    }
    else
    {
        return SdrSnapView::TakeActionRange();
    }
}

void SdrMarkView::HideSdrPage()
{
    UnmarkAllObj();

    SdrSnapView::HideSdrPage();
}

bool SdrMarkView::BegMarkObj(const basegfx::B2DPoint& rPnt, bool bUnmark)
{
    BrkAction();

    DBG_ASSERT(0L == mpMarkObjOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkObjOverlay (!)");
    mpMarkObjOverlay = new ImplMarkingOverlay(*this, rPnt, bUnmark);

    GetDragStat().Reset(rPnt);
    GetDragStat().NextPoint();
    GetDragStat().SetMinMove(getMinMovLog());

    return true;
}

void SdrMarkView::MovMarkObj(const basegfx::B2DPoint& rPnt)
{
    if(IsMarkObj() && GetDragStat().CheckMinMoved(rPnt))
    {
        GetDragStat().NextMove(rPnt);
        DBG_ASSERT(mpMarkObjOverlay, "SdrSnapView::MovMarkObj: no ImplPageOriginOverlay (!)");
        mpMarkObjOverlay->SetSecondPosition(rPnt);
    }
}

bool SdrMarkView::EndMarkObj()
{
    bool bRetval(false);

    if(IsMarkObj())
    {
        if(GetDragStat().IsMinMoved())
        {
            const basegfx::B2DRange aRange(GetDragStat().GetStart(), GetDragStat().GetNow());

            MarkObj(aRange, mpMarkObjOverlay->IsUnmarking());
            bRetval = true;
        }

        // cleanup
        BrkMarkObj();
    }

    return bRetval;
}

void SdrMarkView::BrkMarkObj()
{
    if(IsMarkObj())
    {
        DBG_ASSERT(mpMarkObjOverlay, "SdrSnapView::BrkMarkObj: no ImplPageOriginOverlay (!)");
        delete mpMarkObjOverlay;
        mpMarkObjOverlay = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrMarkView::BegMarkPoints(const basegfx::B2DPoint& rPnt, bool bUnmark)
{
    if(HasMarkablePoints())
    {
        BrkAction();

        DBG_ASSERT(0L == mpMarkPointsOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkPointsOverlay (!)");
        mpMarkPointsOverlay = new ImplMarkingOverlay(*this, rPnt, bUnmark);

        GetDragStat().Reset(rPnt);
        GetDragStat().NextPoint();
        GetDragStat().SetMinMove(getMinMovLog());

        return true;
    }

    return false;
}

void SdrMarkView::MovMarkPoints(const basegfx::B2DPoint& rPnt)
{
    if(IsMarkPoints() && GetDragStat().CheckMinMoved(rPnt))
    {
        GetDragStat().NextMove(rPnt);
        DBG_ASSERT(mpMarkPointsOverlay, "SdrSnapView::MovMarkPoints: no ImplPageOriginOverlay (!)");
        mpMarkPointsOverlay->SetSecondPosition(rPnt);
    }
}

bool SdrMarkView::EndMarkPoints()
{
    bool bRetval(false);

    if(IsMarkPoints())
    {
        if(GetDragStat().IsMinMoved())
        {
            const basegfx::B2DRange aRange(GetDragStat().GetStart(), GetDragStat().GetNow());
            MarkPoints(&aRange, mpMarkPointsOverlay->IsUnmarking());

            bRetval = true;
        }

        // cleanup
        BrkMarkPoints();
    }

    return bRetval;
}

void SdrMarkView::BrkMarkPoints()
{
    if(IsMarkPoints())
    {
        DBG_ASSERT(mpMarkPointsOverlay, "SdrSnapView::BrkMarkPoints: no ImplPageOriginOverlay (!)");
        delete mpMarkPointsOverlay;
        mpMarkPointsOverlay = 0L;
    }
}

bool SdrMarkView::BegMarkGluePoints(const basegfx::B2DPoint& rPnt, bool bUnmark)
{
    if(HasMarkableGluePoints())
    {
        BrkAction();

        DBG_ASSERT(0L == mpMarkGluePointsOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkGluePointsOverlay (!)");
        mpMarkGluePointsOverlay = new ImplMarkingOverlay(*this, rPnt, bUnmark);

        GetDragStat().Reset(rPnt);
        GetDragStat().NextPoint();
        GetDragStat().SetMinMove(getMinMovLog());

        return true;
    }

    return false;
}

void SdrMarkView::MovMarkGluePoints(const basegfx::B2DPoint& rPnt)
{
    if(IsMarkGluePoints() && GetDragStat().CheckMinMoved(rPnt))
    {
        GetDragStat().NextMove(rPnt);
        DBG_ASSERT(mpMarkGluePointsOverlay, "SdrSnapView::MovMarkGluePoints: no ImplPageOriginOverlay (!)");
        mpMarkGluePointsOverlay->SetSecondPosition(rPnt);
    }
}

bool SdrMarkView::EndMarkGluePoints()
{
    bool bRetval(false);

    if(IsMarkGluePoints())
    {
        if(GetDragStat().IsMinMoved())
        {
            const basegfx::B2DRange aRange(GetDragStat().GetStart(),GetDragStat().GetNow());
            MarkGluePoints(&aRange, mpMarkGluePointsOverlay->IsUnmarking());

            bRetval = true;
        }

        // cleanup
        BrkMarkGluePoints();
    }

    return bRetval;
}

void SdrMarkView::BrkMarkGluePoints()
{
    if(IsMarkGluePoints())
    {
        DBG_ASSERT(mpMarkGluePointsOverlay, "SdrSnapView::BrkMarkGluePoints: no ImplPageOriginOverlay (!)");
        delete mpMarkGluePointsOverlay;
        mpMarkGluePointsOverlay = 0L;
    }
}

bool SdrMarkView::HasMarkableObj() const
{
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        SdrObjList* pOL = pPV->GetCurrentObjectList();

        if(pOL)
        {
            SdrObjectVector aObjects(pOL->getSdrObjectVector());

            for(sal_uInt32 a(0); a < aObjects.size(); a++)
            {
                SdrObject* pCandidate = aObjects[a];

                if(pCandidate)
                {
                    if(IsObjMarkable(*pCandidate))
                    {
                        return true;
                    }
                }
                else
                {
                    OSL_ENSURE(false, "SdrObjectVector with empty entries (!)");
                }
            }
        }
        else
        {
            OSL_ENSURE(false, "Unexpected missing SdrObjList (!)");
        }
    }

    return false;
}

sal_uInt32 SdrMarkView::GetMarkableObjCount() const
{
    sal_uInt32 nCount(0);
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        SdrObjList* pOL = pPV->GetCurrentObjectList();

        if(pOL)
        {
            SdrObjectVector aObjects(pOL->getSdrObjectVector());

            for(sal_uInt32 a(0); a < aObjects.size(); a++)
            {
                SdrObject* pCandidate = aObjects[a];

                if(pCandidate)
                {
                    if(IsObjMarkable(*pCandidate))
                    {
                        nCount++;
                    }
                }
                else
                {
                    OSL_ENSURE(false, "SdrObjectVector with empty entries (!)");
                }
            }
        }
        else
        {
            OSL_ENSURE(false, "Unexpected missing SdrObjList (!)");
        }
    }

    return nCount;
}

bool SdrMarkView::ImpIsFrameHandles() const
{
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    bool bFrmHdl(aSelection.size() > FRAME_HANDLES_LIMIT || IsFrameHandles());
    const bool bStdDrag(SDRDRAG_MOVE == GetDragMode());

    if(1 == aSelection.size() && bStdDrag && bFrmHdl)
    {
        const SdrObject* pObj = aSelection[0];

        if(SdrInventor == pObj->GetObjInventor())
        {
            const sal_uInt16 nIdent(pObj->GetObjIdentifier());

            if(OBJ_EDGE == nIdent
                || OBJ_CAPTION == nIdent
                || OBJ_MEASURE == nIdent
                || OBJ_CUSTOMSHAPE == nIdent
                || OBJ_TABLE == nIdent)
            {
                bFrmHdl = false;
            }

            if(bFrmHdl && OBJ_POLY == nIdent)
            {
                const SdrPathObj* pPath = dynamic_cast< const SdrPathObj* >(pObj);

                if(pPath && pPath->isLine())
                {
                    bFrmHdl = false;
                }
            }
        }
    }

    if(!bStdDrag && !bFrmHdl)
    {
        // Grundsaetzlich erstmal alle anderen Dragmodi nur mit FrameHandles
        bFrmHdl = true;

        if(SDRDRAG_ROTATE == GetDragMode())
        {
            // bei Rotate ObjOwn-Drag, wenn mind. 1 PolyObj
            for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size() && bFrmHdl; nMarkNum++)
            {
                bFrmHdl = !aSelection[nMarkNum]->IsPolygonObject();
            }
        }
    }

    if(!bFrmHdl)
    {
        // FrameHandles, wenn wenigstens 1 Obj kein SpecialDrag kann
        for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size() && !bFrmHdl; nMarkNum++)
        {
            bFrmHdl = !aSelection[nMarkNum]->hasSpecialDrag();
        }
    }

    // no FrameHdl for crop
    if(bFrmHdl && SDRDRAG_CROP == GetDragMode())
    {
        bFrmHdl = false;
    }

    return bFrmHdl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_uInt32 SdrMarkView::GetHdlNum(SdrHdl* pHdl) const
{
    return maViewSdrHandleList.GetHdlNum(pHdl);
}

SdrHdl* SdrMarkView::GetHdlByIndex(sal_uInt32 nHdlNum)  const
{
    if(nHdlNum < maViewSdrHandleList.GetHdlCount())
    {
        return maViewSdrHandleList.GetHdlByIndex(nHdlNum);
    }

    return 0;
}

const SdrHdlList& SdrMarkView::GetHdlList() const
{
    if(IsActive())
    {
        // if a pending refresh of SdrHdl is scheduled, execute it now directly
        const_cast< SdrMarkView* >(this)->Timeout();
    }

    return maViewSdrHandleList;
}

void SdrMarkView::Timeout()
{
    // stop timer
    Stop();

    // reset old focus data to invalid (always)
    mbSaveOldFocus = false;

    if(maViewSdrHandleList.GetHdlCount())
    {
        // save focus if it exists (for SdrPathObj point selections)
        SaveMarkHandleFocus(maViewSdrHandleList);

        // clear all handles (delete them and their associated data)
        maViewSdrHandleList.Clear();
    }

    if(!maViewSdrHandleList.GetHdlCount())
    {
        // re-create all handles
        CreateMarkHandles(maViewSdrHandleList);

        // try to restore focus data (for SdrPathObj point selections)
        RestoreMarkHandleFocus(maViewSdrHandleList);

        if(maViewSdrHandleList.GetHdlCount())
        {
            // create visualization (all overlay objects based on handles)
            maViewSdrHandleList.CreateVisualizations();
        }
    }

    if(!maViewSdrHandleList.GetHdlCount())
    {
        // reset old focus data when no handles were created for new situation
        maViewSdrHandleList.ResetFocusHdl();
    }
}

void SdrMarkView::RecreateAllMarkHandles()
{
    // schedule a reset of all SdrHdl; do not do this directly every time, but
    // setup the timer to ensure this is done when there will be time. When this would
    // be done always directly, too many create/delete cycles of SdrHdl would happen
    // in some loops/actions the views perform. The recreation is also triggered
    // when someone accesses the SdrHdl list using GetHdlList() (see there)
    if(!IsActive())
    {
        SetTimeout(1);
        Start();
    }
}

void SdrMarkView::SaveMarkHandleFocus(const SdrHdlList& rTarget)
{
    if(rTarget.GetHdlCount())
    {
        // #105722# remember old focus handle values to search for it again
        const SdrHdl* pSaveOldFocusHdl = rTarget.GetFocusHdl();

        if(pSaveOldFocusHdl
            && dynamic_cast< const SdrPathObj* >(pSaveOldFocusHdl->GetObj())
            && (HDL_POLY == pSaveOldFocusHdl->GetKind()  || HDL_BWGT == pSaveOldFocusHdl->GetKind()))
        {
            mbSaveOldFocus = true;
            mnSavePolyNum = pSaveOldFocusHdl->GetPolyNum();
            mnSavePointNum = pSaveOldFocusHdl->GetPointNum();
            mpSaveObj = pSaveOldFocusHdl->GetObj();
            meSaveKind = pSaveOldFocusHdl->GetKind();
        }
    }
}

void SdrMarkView::RestoreMarkHandleFocus(SdrHdlList& rTarget)
{
    // #105722# try to restore focus handle index from remembered values
    if(mbSaveOldFocus)
    {
        for(sal_uInt32 a(0); a < rTarget.GetHdlCount(); a++)
        {
            SdrHdl* pCandidate = rTarget.GetHdlByIndex(a);

            if(pCandidate->GetObj()
                && pCandidate->GetObj() == mpSaveObj
                && pCandidate->GetKind() == meSaveKind
                && pCandidate->GetPolyNum() == mnSavePolyNum
                && pCandidate->GetPointNum() == mnSavePointNum)
            {
                rTarget.SetFocusHdl(pCandidate);
                break;
            }
        }
    }
}

void SdrMarkView::AdaptFocusHandleOnMove(const basegfx::B2DVector& rDistance)
{
    if(!rDistance.equalZero())
    {
        SdrHdl* pFocusHdl = GetHdlList().GetFocusHdl();

        if(pFocusHdl)
        {
            if(pFocusHdl->IsFocusHdl() && (pFocusHdl->IsCornerHdl() || pFocusHdl->IsVertexHdl()))
            {
                const sal_uInt32 aCount(GetHdlList().GetHdlCount());

                if(aCount)
                {
                    basegfx::B2DRange aCurrent;

                    for(sal_uInt32 a(0); a < aCount; a++)
                    {
                        SdrHdl* pCandidate = GetHdlList().GetHdlByIndex(a);

                        if(pCandidate->IsCornerHdl() || pCandidate->IsVertexHdl())
                        {
                            aCurrent.expand(pCandidate->getPosition());
                        }
                    }

                    if(!aCurrent.isEmpty())
                    {
                        SdrHdlKind aNewKind(pFocusHdl->GetKind());
                        const basegfx::B2DPoint aEndPoint(pFocusHdl->getPosition() + rDistance);
                        const bool bStartLeft(HDL_UPLFT == aNewKind || HDL_LEFT == aNewKind || HDL_LWLFT == aNewKind);
                        const bool bStartRight(HDL_UPRGT == aNewKind || HDL_RIGHT == aNewKind || HDL_LWRGT == aNewKind);
                        const bool bMirrorLTR(bStartLeft && basegfx::fTools::more(aEndPoint.getX(), aCurrent.getMaxX()));
                        const bool bMirrorRTL(bStartRight && basegfx::fTools::less(aEndPoint.getX(), aCurrent.getMinX()));

                        if(bMirrorLTR)
                        {
                            if(HDL_UPLFT == aNewKind)
                                aNewKind = HDL_UPRGT;
                            else if(HDL_LEFT == aNewKind)
                                aNewKind = HDL_RIGHT;
                            else
                                aNewKind = HDL_LWRGT;
                        }
                        else if(bMirrorRTL)
                        {
                            if(HDL_UPRGT == aNewKind)
                                aNewKind = HDL_UPLFT;
                            else if(HDL_RIGHT == aNewKind)
                                aNewKind = HDL_LEFT;
                            else
                                aNewKind = HDL_LWLFT;
                        }

                        const bool bStartTop(HDL_UPLFT == aNewKind || HDL_UPPER == aNewKind || HDL_UPRGT == aNewKind);
                        const bool bStartBottom(HDL_LWLFT == aNewKind || HDL_LOWER == aNewKind || HDL_LWRGT == aNewKind);
                        const bool bMirrorTTB(bStartTop && basegfx::fTools::more(aEndPoint.getY(), aCurrent.getMaxY()));
                        const bool bMirrorBTT(bStartBottom && basegfx::fTools::less(aEndPoint.getY(), aCurrent.getMinY()));

                        if(bMirrorTTB)
                        {
                            if(HDL_UPLFT == aNewKind)
                                aNewKind = HDL_LWLFT;
                            else if(HDL_UPPER == aNewKind)
                                aNewKind = HDL_LOWER;
                            else
                                aNewKind = HDL_LWRGT;
                        }
                        else if(bMirrorBTT)
                        {
                            if(HDL_LWLFT == aNewKind)
                                aNewKind = HDL_UPLFT;
                            else if(HDL_LOWER == aNewKind)
                                aNewKind = HDL_UPPER;
                            else
                                aNewKind = HDL_UPRGT;
                        }

                        if(aNewKind != pFocusHdl->GetKind())
                        {
                            SdrHdl* pNewFocus = GetHdlList().GetHdlByKind(aNewKind);

                            if(pNewFocus)
                            {
                                maViewSdrHandleList.SetFocusHdl(pNewFocus);
                            }
                        }
                    }
                }
            }
        }
    }
}

void SdrMarkView::CreateMarkHandles(SdrHdlList& rTarget)
{
    if(rTarget.GetHdlCount())
    {
        OSL_ENSURE(false, "CreateMarkHandles should only be called when there are no SdrHdl yet, delete them before recreation (!)");
        return;
    }

    rTarget.SetRotateShear(SDRDRAG_ROTATE == GetDragMode());
    rTarget.SetDistortShear(SDRDRAG_SHEAR == GetDragMode());
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    const bool bStdDrag(SDRDRAG_MOVE == GetDragMode());
    const bool bFrmHdl(ImpIsFrameHandles());
    const SdrObject* pSingleSelected = getSelectedIfSingle();
    const SdrTextObj* pSingleTextObj = dynamic_cast< const SdrTextObj* >(pSingleSelected);
    const bool bSingleTextObjMark(pSingleTextObj && pSingleTextObj->IsTextFrame());

    // check if text edit or ole is active and handles need to be suppressed. This may be the case
    // when a single object is selected
    // Using a strict return statement is okay here; no handles means *no* handles.
    if(pSingleSelected)
    {
        // formally #i33755#: If TextEdit is active the EditEngine will directly paint
        // to the window, so suppress Overlay and handles completely; a text frame for
        // the active text edit will be painted by the repaitnt mechanism in
        // SdrObjEditView::ImpPaintOutlinerView in this case. This needs to be reworked
        // in the future
        // Also formally #122142#: Pretty much the same for SdrCaptionObj's in calc.
        if(((SdrView*)this)->IsTextEdit())
        {
            if(pSingleTextObj && pSingleTextObj->IsInEditMode())
            {
                return;
            }
        }

        // formally #i118524#: if inplace activated OLE is selected, suppress handles
        const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pSingleSelected);

        if(pSdrOle2Obj && (pSdrOle2Obj->isInplaceActive() || pSdrOle2Obj->isUiActive()))
        {
            return;
        }
    }

    if(bFrmHdl)
    {
        const basegfx::B2DRange& rSnapRange(getMarkedObjectSnapRange());
        const bool bHideHandlesWhenInTextEdit(getAsSdrView()->IsTextEdit() && pSingleTextObj && pSingleTextObj->IsInEditMode());

        // #i118524# if inplace activated OLE is selected,
        // suppress handles
        bool bHideHandlesWhenOleActive(false);
        const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pSingleSelected);

        if(pSdrOle2Obj && (pSdrOle2Obj->isInplaceActive() || pSdrOle2Obj->isUiActive()))
        {
            bHideHandlesWhenOleActive = true;
        }

        if(!rSnapRange.isEmpty() && !bHideHandlesWhenInTextEdit && !bHideHandlesWhenOleActive)
        {
            if(bSingleTextObjMark)
            {
                const sal_uInt32 nSiz0(rTarget.GetHdlCount());
                pSingleTextObj->AddToHdlList(rTarget);
                const sal_uInt32 nSiz1(rTarget.GetHdlCount());

                for(sal_uInt32 i(nSiz0); i < nSiz1; i++)
                {
                    SdrHdl* pHdl = rTarget.GetHdlByIndex(i);
                    pHdl->SetObjHdlNum(i - nSiz0);
                }
            }
            else
            {
                // TTTT: See below, not needed
                //const bool bNoWidth(basegfx::fTools::equalZero(rSnapRange.getWidth()));
                //const bool bNoHeight(basegfx::fTools::equalZero(rSnapRange.getHeight()));
                //
                //if (bNoWidth && bNoHeight)
                //{
                //    new SdrHdl(rTarget, pSingleSelected, HDL_UPLFT, rSnapRange.getMinimum(), true);
                //}
                //else if (!bStdDrag && (bNoWidth || bNoHeight))
                //{
                //    new SdrHdl(rTarget, pSingleSelected, HDL_UPLFT, rSnapRange.getMinimum(), true);
                //    new SdrHdl(rTarget, pSingleSelected, HDL_LWRGT, rSnapRange.getMaximum(), true);
                //}
                //else
                //{
                //    const basegfx::B2DPoint aCenter(rSnapRange.getCenter());
                //
                //    if(!bNoWidth && !bNoHeight)
                //    {
                //        new SdrHdl(rTarget, pSingleSelected, HDL_UPLFT, rSnapRange.getMinimum(), true);
                //    }
                //
                //    if(!bNoHeight)
                //    {
                //        new SdrHdl(rTarget, pSingleSelected, HDL_UPPER, basegfx::B2DTuple(aCenter.getX(), rSnapRange.getMinY()), true);
                //    }
                //
                //    if(!bNoWidth && !bNoHeight)
                //    {
                //        new SdrHdl(rTarget, pSingleSelected, HDL_UPRGT, basegfx::B2DTuple(rSnapRange.getMaxX(), rSnapRange.getMinY()), true);
                //    }
                //
                //    if(!bNoWidth)
                //    {
                //        new SdrHdl(rTarget, pSingleSelected, HDL_LEFT, basegfx::B2DTuple(rSnapRange.getMinX(), aCenter.getY()), true);
                //    }
                //
                //    if(!bNoWidth)
                //    {
                //        new SdrHdl(rTarget, pSingleSelected, HDL_RIGHT, basegfx::B2DTuple(rSnapRange.getMaxX(), aCenter.getY()), true);
                //    }
                //
                //    if(!bNoWidth && !bNoHeight)
                //    {
                //        new SdrHdl(rTarget, pSingleSelected, HDL_LWLFT, basegfx::B2DTuple(rSnapRange.getMinX(), rSnapRange.getMaxY()), true);
                //    }
                //
                //    if(!bNoHeight)
                //    {
                //        new SdrHdl(rTarget, pSingleSelected, HDL_LOWER, basegfx::B2DTuple(aCenter.getX(), rSnapRange.getMaxY()), true);
                //    }
                //
                //    if(!bNoWidth && !bNoHeight)
                //    {
                //        new SdrHdl(rTarget, pSingleSelected, HDL_LWRGT, rSnapRange.getMaximum(), true);
                //    }
                //}

                // always create all handles to allow focus restauration over keyboard moves; prefer
                // bottom-right ones to make mouse hits preferred to bottom-right assuming the next
                // resize will be dragged to bottom-right
                const basegfx::B2DPoint aCenter(rSnapRange.getCenter());

                new SdrHdl(rTarget, pSingleSelected, HDL_UPLFT, rSnapRange.getMinimum(), true);
                new SdrHdl(rTarget, pSingleSelected, HDL_UPPER, basegfx::B2DTuple(aCenter.getX(), rSnapRange.getMinY()), true);
                new SdrHdl(rTarget, pSingleSelected, HDL_UPRGT, basegfx::B2DTuple(rSnapRange.getMaxX(), rSnapRange.getMinY()), true);
                new SdrHdl(rTarget, pSingleSelected, HDL_LEFT, basegfx::B2DTuple(rSnapRange.getMinX(), aCenter.getY()), true);
                new SdrHdl(rTarget, pSingleSelected, HDL_RIGHT, basegfx::B2DTuple(rSnapRange.getMaxX(), aCenter.getY()), true);
                new SdrHdl(rTarget, pSingleSelected, HDL_LWLFT, basegfx::B2DTuple(rSnapRange.getMinX(), rSnapRange.getMaxY()), true);
                new SdrHdl(rTarget, pSingleSelected, HDL_LOWER, basegfx::B2DTuple(aCenter.getX(), rSnapRange.getMaxY()), true);
                new SdrHdl(rTarget, pSingleSelected, HDL_LWRGT, rSnapRange.getMaximum(), true);
            }
        }
    }
    else
    {
        bool bDone(false);

        // moved crop handling to non-frame part and the handle creation to SdrGrafObj
        if(pSingleSelected && SDRDRAG_CROP == GetDragMode())
        {
            const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pSingleSelected);

            if(pSdrGrafObj)
            {
                pSdrGrafObj->addCropHandles(rTarget);
                bDone = true;
            }
        }

        if(!bDone)
        {
            for (sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size(); nMarkNum++)
            {
                SdrObject* pObj = aSelection[nMarkNum];
                const sal_uInt32 nSiz0(rTarget.GetHdlCount());
                pObj->AddToHdlList(rTarget);
                const sal_uInt32 nSiz1(rTarget.GetHdlCount());
                const bool bPoly(pObj->IsPolygonObject());
                const sdr::selection::Indices aMarkedPoints(bPoly ? getSelectedPointsForSelectedSdrObject(*pObj) : sdr::selection::Indices());

                for(sal_uInt32 i(nSiz0); i < nSiz1; i++)
                {
                    SdrHdl* pHdl = rTarget.GetHdlByIndex(i);
                    pHdl->SetObjHdlNum(i - nSiz0);

                    if (bPoly)
                    {
                        const bool bSelected(0 != aMarkedPoints.count(i - nSiz0));

                        pHdl->SetSelected(bSelected);

                        if(IsPlusHandlesAlwaysVisible() || bSelected)
                        {
                            const sal_uInt32 nPlusAnz(pObj->GetPlusHdlCount(*pHdl));

                            for(sal_uInt32 nPlusNum(0); nPlusNum < nPlusAnz; nPlusNum++)
                            {
                                pObj->GetPlusHdl(rTarget, *pObj, *pHdl, nPlusNum);
                            }
                        }
                    }
                }
            }
        }
    }

    // GluePoint-Handles
    for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size(); nMarkNum++)
    {
        SdrObject* pObj = aSelection[nMarkNum];
        const sdr::selection::Indices aMarkedGluePoints(getSelectedGluesForSelectedSdrObject(*pObj));

        if(!aMarkedGluePoints.empty())
        {
            const sdr::glue::GluePointProvider& rProvider = pObj->GetGluePointProvider();

            if(rProvider.hasUserGluePoints())
            {
                for(sdr::selection::Indices::const_iterator aCurrent(aMarkedGluePoints.begin());
                    aCurrent != aMarkedGluePoints.end(); aCurrent++)
                {
                    const sal_uInt32 nId(*aCurrent);
                    const sdr::glue::GluePoint* pCandidate = rProvider.findUserGluePointByID(nId);

                    if(pCandidate)
                    {
                        const basegfx::B2DPoint aPos(pObj->getSdrObjectTransformation() * pCandidate->getUnitPosition());
                        SdrHdl* pGlueHdl = new SdrHdl(rTarget, pObj, HDL_GLUE, aPos);

                        pGlueHdl->SetObjHdlNum(nId);
                    }
                }
            }
        }
    }

    // Drehpunkt/Spiegelachse
    AddDragModeHdl(rTarget, GetDragMode());

    // add custom handles (used by other apps, e.g. AnchorPos)
    AddCustomHdl(rTarget);

    // sort handles
    rTarget.Sort();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::AddCustomHdl(SdrHdlList& /*rTarget*/)
{
    // add custom handles (used by other apps, e.g. AnchorPos)
}

void SdrMarkView::SetDragMode(SdrDragMode eMode)
{
    if(SDRDRAG_RESIZE == eMode)
    {
        eMode = SDRDRAG_MOVE;
    }

    if(GetDragMode() != eMode)
    {
        meViewDragMode = eMode;

        ForceRefToMarked();
    }
}

void SdrMarkView::AddDragModeHdl(SdrHdlList& rTarget, SdrDragMode eMode)
{
    switch(eMode)
    {
        case SDRDRAG_ROTATE:
        {
            // add rotation center
            new SdrHdl(rTarget, 0, HDL_REF1, GetRef1());
            break;
        }
        case SDRDRAG_MIRROR:
        {
            // add mirror axis
            SdrHdlLine* pHdl1 = new SdrHdlLine(rTarget, HDL_MIRX);
            SdrHdl* pHdl2 = new SdrHdl(rTarget, 0, HDL_REF1, GetRef1());
            SdrHdl* pHdl3 = new SdrHdl(rTarget, 0, HDL_REF2, GetRef2());

            pHdl1->SetHandles(pHdl2, pHdl3);
            pHdl1->SetObjHdlNum(1);
            pHdl2->SetObjHdlNum(2);
            pHdl3->SetObjHdlNum(3);

            break;
        }
        case SDRDRAG_TRANSPARENCE:
        {
            // add interactive transparence handle
            SdrObject* pSingleSelected = getSelectedIfSingle();

            if(pSingleSelected)
            {
                const SfxItemSet& rSet = pSingleSelected->GetMergedItemSet();

                if(SFX_ITEM_SET != rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, false))
                {
                    // add this item, it's not yet there
                    XFillFloatTransparenceItem aNewItem(
                        (const XFillFloatTransparenceItem&)rSet.Get(XATTR_FILLFLOATTRANSPARENCE));
                    XGradient aGrad = aNewItem.GetGradientValue();

                    aNewItem.SetEnabled(true);
                    aGrad.SetStartIntens(100);
                    aGrad.SetEndIntens(100);
                    aNewItem.SetGradientValue(aGrad);

                    // add undo to allow user to take back this step
                    if( getSdrModelFromSdrView().IsUndoEnabled() )
                    {
                        getSdrModelFromSdrView().BegUndo(SVX_RESSTR(SIP_XA_FILLTRANSPARENCE));
                        getSdrModelFromSdrView().AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoAttrObject(*pSingleSelected));
                        getSdrModelFromSdrView().EndUndo();
                    }

                    SfxItemSet aNewSet(pSingleSelected->GetObjectItemPool());
                    aNewSet.Put(aNewItem);
                    pSingleSelected->SetMergedItemSetAndBroadcast(aNewSet);
                }

                // set values and transform to vector set
                GradTransformer aGradTransformer;
                GradTransVector aGradTransVector;
                GradTransGradient aGradTransGradient;

                aGradTransGradient.aGradient = ((XFillFloatTransparenceItem&)rSet.Get(XATTR_FILLFLOATTRANSPARENCE)).GetGradientValue();
                aGradTransformer.GradToVec(aGradTransGradient, aGradTransVector, pSingleSelected);

                // build handles
                SdrHdlColor* pColHdl1 = new SdrHdlColor(rTarget, *pSingleSelected, aGradTransVector.maPositionA, aGradTransVector.aCol1, SDR_HANDLE_COLOR_SIZE_NORMAL, true);
                SdrHdlColor* pColHdl2 = new SdrHdlColor(rTarget, *pSingleSelected, aGradTransVector.maPositionB, aGradTransVector.aCol2, SDR_HANDLE_COLOR_SIZE_NORMAL, true);
                SdrHdlGradient* pGradHdl = new SdrHdlGradient(rTarget, *pSingleSelected, *pColHdl1, *pColHdl2, false);

                // link them
                pColHdl1->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));
                pColHdl2->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));
            }
            break;
        }
        case SDRDRAG_GRADIENT:
        {
            // add interactive gradient handle
            SdrObject* pSingleSelected = getSelectedIfSingle();

            if(pSingleSelected)
            {
                const SfxItemSet& rSet = pSingleSelected->GetMergedItemSet();
                XFillStyle eFillStyle = ((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();

                if(eFillStyle == XFILL_GRADIENT)
                {
                    // set values and transform to vector set
                    GradTransformer aGradTransformer;
                    GradTransVector aGradTransVector;
                    GradTransGradient aGradTransGradient;
                    Size aHdlSize(15, 15);

                    aGradTransGradient.aGradient = ((XFillGradientItem&)rSet.Get(XATTR_FILLGRADIENT)).GetGradientValue();
                    aGradTransformer.GradToVec(aGradTransGradient, aGradTransVector, pSingleSelected);

                    // build handles
                    SdrHdlColor* pColHdl1 = new SdrHdlColor(rTarget, *pSingleSelected, aGradTransVector.maPositionA, aGradTransVector.aCol1, aHdlSize, false);
                    SdrHdlColor* pColHdl2 = new SdrHdlColor(rTarget, *pSingleSelected, aGradTransVector.maPositionB, aGradTransVector.aCol2, aHdlSize, false);
                    SdrHdlGradient* pGradHdl = new SdrHdlGradient(rTarget, *pSingleSelected, *pColHdl1, *pColHdl2, true);

                    // link them
                    pColHdl1->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));
                    pColHdl2->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));
                }
            }
            break;
        }
        case SDRDRAG_CROP:
        {
            // todo
            break;
        }
        default: break;
    }
}

bool SdrMarkView::MouseMove(const MouseEvent& rMEvt, Window* pWin)
{
    const SdrHdlList& rHdlList = GetHdlList();

    if(rHdlList.GetHdlCount())
    {
        SdrHdl* pMouseOverHdl(0);

        if( !rMEvt.IsLeaveWindow() && pWin )
        {
            const basegfx::B2DPoint aPosLogic(pWin->GetInverseViewTransformation() *
                basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y()));

            pMouseOverHdl = PickHandle(aPosLogic);
        }

        // notify last mouse over handle that he lost the mouse
        const sal_uInt32 nHdlCount(rHdlList.GetHdlCount());

        for(sal_uInt32 nHdl(0); nHdl < nHdlCount; nHdl++)
        {
            SdrHdl* pCurrentHdl = GetHdlByIndex(nHdl);

            if(pCurrentHdl->isMouseOver())
            {
                if( pCurrentHdl != pMouseOverHdl )
                {
                    pCurrentHdl->setMouseOver(false);
                    pCurrentHdl->onMouseLeave();
                }

                break;
            }
        }

        // notify current mouse over handle
        if(pMouseOverHdl)
        {
            pMouseOverHdl->setMouseOver(true);
            pMouseOverHdl->onMouseEnter(rMEvt);
        }
    }

    return SdrSnapView::MouseMove(rMEvt, pWin);
}

void SdrMarkView::ForceRefToMarked()
{
    switch(GetDragMode())
    {
        case SDRDRAG_ROTATE:
        {
            const basegfx::B2DRange aSnapRange(getMarkedObjectSnapRange());
            maRef1 = aSnapRange.getCenter();
            break;
        }

        case SDRDRAG_MIRROR:
        {
            // Erstmal die laenge der Spiegelachsenlinie berechnen
            sal_Int32 nOutMin(0);
            sal_Int32 nOutMax(0);
            sal_Int32 nMinLen(0);
            sal_Int32 nObjDst(0);
            sal_Int32 nOutHgt(0);
            OutputDevice* pOut=GetFirstOutputDevice();

            if(pOut)
            {
                // Mindestlaenge 50 Pixel
                nMinLen=pOut->PixelToLogic(Size(0,50)).Height();

                // 20 Pixel fuer RefPt-Abstand vom Obj
                nObjDst=pOut->PixelToLogic(Size(0,20)).Height();

                // MinY/MaxY
                // Abstand zum Rand = Mindestlaenge = 10 Pixel
                const sal_Int32 nDst(pOut->PixelToLogic(Size(0,10)).Height());

                nOutMin=-pOut->GetMapMode().GetOrigin().Y();
                nOutMax=pOut->GetOutputSize().Height()-1+nOutMin;
                nOutMin+=nDst;
                nOutMax-=nDst;

                // Absolute Mindestlaenge jedoch 10 Pixel
                if(nOutMax - nOutMin < nDst)
                {
                    nOutMin+=nOutMax+1;
                    nOutMin/=2;
                    nOutMin-=(nDst+1)/2;
                    nOutMax=nOutMin+nDst;
                }

                nOutHgt=nOutMax-nOutMin;

                // Sonst Mindestlaenge = 1/4 OutHgt
                const sal_Int32 nTemp(nOutHgt / 4);

                if(nTemp > nMinLen)
                {
                    nMinLen = nTemp;
                }
            }

            const Rectangle aR(sdr::legacy::GetAllObjBoundRect(getSelectedSdrObjectVectorFromSdrMarkView()));
            const Point aCenter(aR.Center());
            const sal_Int32 nMarkHgt(aR.GetHeight() - 1);
            sal_Int32 nHgt(nMarkHgt + nObjDst * 2);       // 20 Pixel obej und unten ueberstehend

            if(nHgt < nMinLen)
            {
                nHgt = nMinLen;     // Mindestlaenge 50 Pixel bzw. 1/4 OutHgt
            }

            sal_Int32 nY1(aCenter.Y() - (nHgt + 1) / 2);
            sal_Int32 nY2(nY1 + nHgt);

            if(pOut && nMinLen > nOutHgt)
            {
                nMinLen = nOutHgt; // evtl. noch etwas verkuerzen
            }

            if(pOut)
            {
                // nun vollstaendig in den sichtbaren Bereich schieben
                if(nY1 < nOutMin)
                {
                    nY1=nOutMin;

                    if(nY2 < nY1 + nMinLen)
                    {
                        nY2 = nY1 + nMinLen;
                    }
                }

                if(nY2 > nOutMax)
                {
                    nY2=nOutMax;

                    if(nY1 > nY2 - nMinLen)
                    {
                        nY1 = nY2 - nMinLen;
                    }
                }
            }

            maRef1 = basegfx::B2DPoint(aCenter.X(), nY1);
            maRef2 = basegfx::B2DPoint(aCenter.X(), nY2);

            break;
        }

        case SDRDRAG_TRANSPARENCE:
        case SDRDRAG_GRADIENT:
        case SDRDRAG_CROP:
        {
            const basegfx::B2DRange aBoundRange(sdr::legacy::GetAllObjBoundRange(getSelectedSdrObjectVectorFromSdrMarkView()));
            maRef1 = aBoundRange.getMinimum();
            maRef2 = aBoundRange.getMaximum();
            break;
        }
        default: break;
    }

    // force recreation of SdrHdls TTTT needed?
    RecreateAllMarkHandles();
}

void SdrMarkView::SetRef1(const basegfx::B2DPoint& rPt)
{
    if(SDRDRAG_ROTATE == GetDragMode() || SDRDRAG_MIRROR == GetDragMode())
    {
        maRef1 = rPt;
        SdrHdl* pH = GetHdlList().GetHdlByKind(HDL_REF1);

        if(pH)
        {
            pH->setPosition(rPt);
        }
    }
}

void SdrMarkView::SetRef2(const basegfx::B2DPoint& rPt)
{
    if(SDRDRAG_MIRROR == GetDragMode())
    {
        maRef2 = rPt;
        SdrHdl* pH = GetHdlList().GetHdlByKind(HDL_REF2);

        if(pH)
        {
            pH->setPosition(rPt);
        }
    }
}

void SdrMarkView::SetFrameHandles(bool bOn)
{
    if(bOn != IsFrameHandles())
    {
        const bool bOld(ImpIsFrameHandles());
        mbForceFrameHandles = bOn;
        const bool bNew(ImpIsFrameHandles());

        if(bNew != bOld)
        {
            // force recreation of SdrHdls
            RecreateAllMarkHandles();
        }
    }
}

void SdrMarkView::SetViewEditMode(SdrViewEditMode eMode)
{
    if(eMode != GetViewEditMode())
    {
        bool bGlue0(SDREDITMODE_GLUEPOINTEDIT == GetViewEditMode());
        bool bEdge0(((SdrCreateView*)this)->IsEdgeTool());

        meViewEditMode = eMode;

        bool bGlue1(SDREDITMODE_GLUEPOINTEDIT == GetViewEditMode());
        bool bEdge1(((SdrCreateView*)this)->IsEdgeTool());

        // etwas Aufwand um Flackern zu verhindern beim Umschalten
        // zwischen GlueEdit und EdgeTool
        if(bGlue1 && !bGlue0)
        {
            ImpSetGlueVisible2(bGlue1);
        }

        if(bEdge1 != bEdge0)
        {
            ImpSetGlueVisible3(bEdge1);
        }

        if(!bGlue1 && bGlue0)
        {
            ImpSetGlueVisible2(bGlue1);
        }

        if(bGlue0 && !bGlue1)
        {
            MarkGluePoints(0, true);
        }
    }
}

bool SdrMarkView::IsObjMarkable(const SdrObject& rObj) const
{
    if(rObj.IsMarkProtect() || (!IsDesignMode() && dynamic_cast< const SdrUnoObj* >(&rObj)))
    {
        // Objekt nicht selektierbar oder
        // SdrUnoObj nicht im DesignMode
        return false;
    }

    return GetSdrPageView() ? GetSdrPageView()->IsObjMarkable(rObj) : true;
}

bool SdrMarkView::IsMarkedObjHit(const basegfx::B2DPoint& rPnt, double fTol) const
{
    bool bRet(false);

    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nm(0); nm < aSelection.size() && !bRet; nm++)
        {
            bRet = CheckSingleSdrObjectHit(rPnt, fTol, aSelection[nm], 0, 0);
        }
    }

    return bRet;
}

SdrHdl* SdrMarkView::PickHandle(const basegfx::B2DPoint& rPnt) const
{
    return GetHdlList().IsHdlListHit(rPnt);
}

bool SdrMarkView::MarkObj(const basegfx::B2DPoint& rPnt, double fTol, bool bToggle, bool bDeep)
{
    SdrObject* pObj;
    sal_uInt32 nOptions(SDRSEARCH_PICKMARKABLE);

    if(bDeep)
    {
        nOptions = nOptions|SDRSEARCH_DEEP;
    }

    bool bRet(PickObj(rPnt, fTol, pObj, nOptions));

    if(bRet)
    {
        bool bUnmark = bToggle && IsObjMarked(*pObj);

        MarkObj(*pObj, bUnmark);
    }

    return bRet;
}

bool SdrMarkView::MarkNextObj(bool bPrev)
{
    SdrPageView* pPageView = GetSdrPageView();

    if(pPageView)
    {
        SdrObject* pChangeSdrObject(0); // Nummer des zu ersetzenden MarkEntries
        sal_uInt32 nSearchObjNum(bPrev ? 0 : SAL_MAX_UINT32);

        if(areSdrObjectsSelected())
        {
            const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

            pChangeSdrObject = aSelection[bPrev ? 0 : aSelection.size() - 1];
            nSearchObjNum = pChangeSdrObject->GetNavigationPosition();
        }

        SdrObject* pMarkObj = 0;
        const SdrObjList* pSearchObjList = pPageView->GetCurrentObjectList();
        const sal_uInt32 nObjAnz(pSearchObjList->GetObjCount());

        if(nObjAnz)
        {
            if(nSearchObjNum > nObjAnz)
            {
                nSearchObjNum = nObjAnz;
            }

            while(!pMarkObj && ((!bPrev && nSearchObjNum > 0) || (bPrev && nSearchObjNum < nObjAnz)))
            {
                if (!bPrev)
                {
                    nSearchObjNum--;
                }

                SdrObject* pSearchObj = pSearchObjList->GetObj(nSearchObjNum);

                if(IsObjMarkable(*pSearchObj))
                {
                    if(!isSdrObjectSelected(*pSearchObj))
                    {
                        pMarkObj=pSearchObj;
                    }
                }

                if(bPrev)
                {
                    nSearchObjNum++;
                }
            }
        }

        if(!pMarkObj)
        {
            return false;
        }

        if(pChangeSdrObject)
        {
            removeSdrObjectFromSelection(*pChangeSdrObject);
        }

        MarkObj(*pMarkObj);
        return true;
    }

    return false;
}

bool SdrMarkView::MarkNextObj(const basegfx::B2DPoint& rPnt, double fTol, bool bPrev)
{
    SdrObject* pTopMarkHit = 0;
    SdrObject* pBtmMarkHit = 0;

    // oberstes der markierten Objekte suchen, das von rPnt getroffen wird
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    sal_uInt32 nm(0);

    for(nm = aSelection.size(); nm > 0 && !pTopMarkHit;)
    {
        nm--;

        if(CheckSingleSdrObjectHit(rPnt, fTol, aSelection[nm], 0, 0))
        {
            pTopMarkHit = aSelection[nm];
        }
    }

    // Nichts gefunden, dann ganz normal ein Obj markieren.
    if(!pTopMarkHit)
    {
        return MarkObj(rPnt, fTol, false);
    }

    SdrObjList* pObjList = pTopMarkHit->getParentOfSdrObject();

    // unterstes der markierten Objekte suchen, das von rPnt getroffen wird
    for(nm = 0; nm < aSelection.size() && !pBtmMarkHit; nm++)
    {
        if(CheckSingleSdrObjectHit(rPnt, fTol, aSelection[nm], 0, 0))
        {
            pBtmMarkHit = aSelection[nm];
        }
    }

    if(!pBtmMarkHit)
    {
        pBtmMarkHit = pTopMarkHit;
    }

    const sal_uInt32 nObjAnz(pObjList->GetObjCount());
    sal_uInt32 nSearchBeg;
    E3dScene* pScene = 0;
    E3dCompoundObject* pObjHit = dynamic_cast< E3dCompoundObject* >(bPrev ? pBtmMarkHit : pTopMarkHit);
    bool bRemap(pObjHit
        ? pObjHit->IsAOrdNumRemapCandidate(pScene)
        : false);

    if(bPrev)
    {
        sal_uInt32 nOrdNumBtm(pBtmMarkHit->GetNavigationPosition());

        if(bRemap)
        {
            nOrdNumBtm = pScene->RemapOrdNum(nOrdNumBtm);
        }

        nSearchBeg = nOrdNumBtm + 1;
    }
    else
    {
        sal_uInt32 nOrdNumTop(pTopMarkHit->GetNavigationPosition());

        if(bRemap)
        {
            nOrdNumTop = pScene->RemapOrdNum(nOrdNumTop);
        }

        nSearchBeg = nOrdNumTop;
    }

    sal_uInt32 no(nSearchBeg);
    SdrObject* pFndObj = 0;

    while(!pFndObj && ((!bPrev && no > 0) || (bPrev && no < nObjAnz)))
    {
        if(!bPrev)
        {
            no--;
        }

        SdrObject* pObj;

        if(bRemap)
        {
            pObj = pObjList->GetObj(pScene->RemapOrdNum(no));
        }
        else
        {
            pObj = pObjList->GetObj(no);
        }

        if(CheckSingleSdrObjectHit(rPnt, fTol, pObj, SDRSEARCH_TESTMARKABLE, 0))
        {
            if(!isSdrObjectSelected(*pObj))
            {
                pFndObj=pObj;
            }
            else
            {
                // hier wg. Performance ggf. noch no auf Top bzw. auf Btm stellen
            }
        }

        if(bPrev)
        {
            no++;
        }
    }

    if(pFndObj)
    {
        removeSdrObjectFromSelection(bPrev ? *pBtmMarkHit : *pTopMarkHit);
        addSdrObjectToSelection(*pFndObj);
    }

    return (0 != pFndObj);
}

bool SdrMarkView::MarkObj(const basegfx::B2DRange& rRange, bool bUnmark)
{
    bool bFnd(false);
    const basegfx::B2DRange aR(rRange);
    SdrObject* pObj;
    SdrObjList* pObjList;
    BrkAction();
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        pObjList = pPV->GetCurrentObjectList();
        basegfx::B2DRange aFrm1(aR);
        const sal_uInt32 nObjAnz(pObjList->GetObjCount());

        for(sal_uInt32 nO(0); nO < nObjAnz; nO++)
        {
            pObj=pObjList->GetObj(nO);
            const basegfx::B2DRange& rObjectBound(pObj->getObjectRange(getAsSdrView()));

            if(aFrm1.isInside(rObjectBound))
            {
                if(!bUnmark)
                {
                    if(IsObjMarkable(*pObj))
                    {
                        addSdrObjectToSelection(*pObj);
                        bFnd = true;
                    }
                }
                else
                {
                    if(isSdrObjectSelected(*pObj))
                    {
                        removeSdrObjectFromSelection(*pObj);
                        bFnd = true;
                    }
                }
            }
        }
    }

    return bFnd; // TTTT: needed?
}

void SdrMarkView::MarkObj(SdrObject& rObj, bool bUnmark)
{
    if (IsObjMarkable(rObj))
    {
        BrkAction();

        if (!bUnmark)
        {
            addSdrObjectToSelection(rObj);
        }
        else
        {
            removeSdrObjectFromSelection(rObj);
        }
    }
}

bool SdrMarkView::IsObjMarked(const SdrObject& rObj) const
{
    // nicht so ganz die feine Art: Da FindObject() nicht const ist
    // muss ich mich hier auf non-const casten.
    return isSdrObjectSelected(rObj);
}

sal_uInt16 SdrMarkView::GetMarkHdlSizePixel() const
{
    return maViewSdrHandleList.GetHdlSize()*2+1;
}

bool SdrMarkView::IsSolidMarkHdl() const
{
    return maViewSdrHandleList.IsFineHdl();
}

void SdrMarkView::SetSolidMarkHdl(bool bOn)
{
    if(bOn != maViewSdrHandleList.IsFineHdl())
    {
        maViewSdrHandleList.SetFineHdl(bOn);
        RecreateAllMarkHandles();
    }
}

void SdrMarkView::SetMarkHdlSizePixel(sal_uInt16 nSiz)
{
    if (nSiz<3) nSiz=3;
    nSiz/=2;
    if (nSiz!=maViewSdrHandleList.GetHdlSize())
    {
        maViewSdrHandleList.SetHdlSize(nSiz);
        RecreateAllMarkHandles();
    }
}

#define SDRSEARCH_IMPISMASTER 0x80000000 /* MasterPage wird gerade durchsucht */

SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const basegfx::B2DPoint& rPnt, double fTol, SdrObject* pObj, sal_uInt32 nOptions, const SetOfByte* pMVisLay) const
{
    if(((nOptions & SDRSEARCH_IMPISMASTER) && pObj->IsNotVisibleAsMaster()) || (!pObj->IsVisible()))
    {
        return 0;
    }

    const bool bCheckIfMarkable(nOptions & SDRSEARCH_TESTMARKABLE);
    const bool bDeep(nOptions & SDRSEARCH_DEEP);
    const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >(pObj);
    const SdrTextObj* pSdrTextObj = dynamic_cast< SdrTextObj* >(pObj);
    SdrObject* pRet = 0;
    basegfx::B2DRange aBoundRange(pObj->getObjectRange(getAsSdrView()));
    double fTol2(fTol);

    // double tolerance for OLE, text frames and objects in
    // active text edit
    if(pSdrOle2Obj || (pSdrTextObj && pSdrTextObj->IsTextFrame()) || pObj==((SdrObjEditView*)this)->GetTextEditObject())
    {
        fTol2 *= 2.0;
    }

    if(!basegfx::fTools::equalZero(fTol2))
    {
        aBoundRange.grow(fabs(fTol2));
    }

    if(aBoundRange.isInside(rPnt))
    {
        if((!bCheckIfMarkable || IsObjMarkable(*pObj)))
        {
            SdrObjList* pOL = pObj->getChildrenOfSdrObject();

            if(pOL && pOL->GetObjCount())
            {
                SdrObject* pTmpObj;

                pRet = CheckSingleSdrObjectHit(rPnt, fTol, pOL, nOptions, pMVisLay, pTmpObj);
            }
            else
            {
                if(!pMVisLay || pMVisLay->IsSet(pObj->GetLayer()))
                {
                    pRet = SdrObjectPrimitiveHit(*pObj, rPnt, fTol2, *getAsSdrView(), false, 0);
                }
            }
        }
    }

    if(!bDeep && pRet)
    {
        pRet=pObj;
    }

    return pRet;
}

SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const basegfx::B2DPoint& rPnt, double fTol, SdrObjList* pOL, sal_uInt32 nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj) const
{
    SdrObject* pRet = 0;
    rpRootObj = 0;

    if(pOL)
    {
        E3dScene* pRemapScene = dynamic_cast< E3dScene* >(pOL->getSdrObjectFromSdrObjList());
        const sal_uInt32 nObjAnz(pOL->GetObjCount());
        sal_uInt32 nObjNum(nObjAnz);

        while(!pRet && nObjNum > 0)
        {
            nObjNum--;
            SdrObject* pObj;

            if(pRemapScene)
            {
                pObj = pOL->GetObj(pRemapScene->RemapOrdNum(nObjNum));
            }
            else
            {
                pObj = pOL->GetObj(nObjNum);
            }

            pRet = CheckSingleSdrObjectHit(rPnt, fTol, pObj, nOptions, pMVisLay);

            if(pRet)
            {
                rpRootObj = pObj;
            }
        }
    }

    return pRet;
}

bool SdrMarkView::PickObj(const basegfx::B2DPoint& rPnt, double fTol, SdrObject*& rpObj, sal_uInt32 nOptions) const
{
    return PickObj(rPnt, fTol, rpObj, nOptions, 0, 0, 0);
}

bool SdrMarkView::PickObj(const basegfx::B2DPoint& rPnt, double fTol, SdrObject*& rpObj, sal_uInt32 nOptions, SdrObject** ppRootObj, sal_uInt32* pnMarkNum, sal_uInt16* pnPassNum) const
{
    if(ppRootObj)
    {
        *ppRootObj = 0;
    }

    if(pnMarkNum)
    {
        *pnMarkNum = CONTAINER_ENTRY_NOTFOUND;
    }

    if(pnPassNum)
    {
        *pnPassNum = 0;
    }

    rpObj = 0;

    const bool bWholePage(nOptions & SDRSEARCH_WHOLEPAGE);
    const bool bMarked(nOptions & SDRSEARCH_MARKED);
    const bool bMasters(!bMarked && (nOptions & SDRSEARCH_ALSOONMASTER));

    SdrObject* pObj = 0;
    SdrObject* pHitObj = 0;

    if(static_cast< const SdrView* >(this)->IsTextEditFrameHit(rPnt))
    {
        pObj = static_cast< const SdrView* >(this)->GetTextEditObject();
        pHitObj=pObj;
    }

    if(bMarked)
    {
        if(areSdrObjectsSelected())
        {
            const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
            sal_uInt32 nMrkNum(aSelection.size());

            while(!pHitObj && nMrkNum > 0)
            {
                nMrkNum--;
                pObj = aSelection[nMrkNum];
                pHitObj = CheckSingleSdrObjectHit(rPnt, fTol, pObj, nOptions, 0);
            }
        }
    }
    else
    {
        if(GetSdrPageView())
        {
            SdrPage& rPage = GetSdrPageView()->getSdrPageFromSdrPageView();
            sal_uInt16 nPgAnz(1);

            if(bMasters && rPage.TRG_HasMasterPage())
            {
                nPgAnz++;
            }

            const bool bExtraPassForWholePage(bWholePage && &rPage != GetSdrPageView()->GetCurrentObjectList());

            if(bExtraPassForWholePage)
            {
                nPgAnz++; // Suche erst in AktObjList, dann auf der gesamten Page
            }

            sal_uInt16 nPgNum(nPgAnz);

            while(!pHitObj && nPgNum > 0)
            {
                sal_uInt32 nTmpOptions(nOptions);
                nPgNum--;
                const SetOfByte* pMVisLay = 0;
                SdrObjList* pObjList = 0;

                if(pnPassNum)
                {
                    *pnPassNum &= ~(SDRSEARCHPASS_MASTERPAGE|SDRSEARCHPASS_INACTIVELIST);
                }

                if (nPgNum>=nPgAnz-1 || (bExtraPassForWholePage && nPgNum>=nPgAnz-2))
                {
                    pObjList = GetSdrPageView()->GetCurrentObjectList();

                    if(bExtraPassForWholePage && nPgNum == nPgAnz - 2)
                    {
                        pObjList = &rPage;

                        if(pnPassNum)
                        {
                            *pnPassNum |= SDRSEARCHPASS_INACTIVELIST;
                        }
                    }
                }
                else
                {
                    // sonst MasterPage
                    SdrPage& rMasterPage = rPage.TRG_GetMasterPage();
                    pMVisLay = &rPage.TRG_GetMasterPageVisibleLayers();
                    pObjList = &rMasterPage;

                    if(pnPassNum)
                    {
                        *pnPassNum |= SDRSEARCHPASS_MASTERPAGE;
                    }

                    nTmpOptions=nTmpOptions | SDRSEARCH_IMPISMASTER;
                }

                pHitObj = CheckSingleSdrObjectHit(rPnt, fTol, pObjList, nTmpOptions, pMVisLay, pObj);
            }
        }
    }

    if(pHitObj)
    {
        if(ppRootObj)
        {
            *ppRootObj = pObj;
        }

        if(nOptions & SDRSEARCH_DEEP)
        {
            pObj = pHitObj;
        }

        if(nOptions & SDRSEARCH_TESTTEXTEDIT)
        {
            if(!pObj->HasTextEdit() || (GetSdrPageView() && GetSdrPageView()->GetLockedLayers().IsSet(pObj->GetLayer())))
            {
                pObj = 0;
            }
        }

        if(pObj && (nOptions & SDRSEARCH_TESTMACRO))
        {
            SdrObjMacroHitRec aHitRec;

            aHitRec.maPos = rPnt;
            aHitRec.maDownPos = rPnt;
            aHitRec.mfTol = fTol;
            aHitRec.mpSdrView = getAsSdrView();

            if(!pObj->HasMacro() || !pObj->IsMacroHit(aHitRec))
            {
                pObj = 0;
            }
        }

//      if(pObj && (nOptions & SDRSEARCH_WITHTEXT) && !pObj->GetOutlinerParaObject())
//      {
//          pObj = 0;
//      }

        if(pObj)
        {
            rpObj=pObj;

            if(pnPassNum)
            {
                *pnPassNum |= SDRSEARCHPASS_DIRECT;
            }
        }
    }

    return (0 != rpObj);
}

bool SdrMarkView::PickMarkedObj(const basegfx::B2DPoint& rPnt, SdrObject*& rpObj, sal_uInt32* pnMarkNum, sal_uInt32 nOptions) const
{
    const bool bBoundCheckOn2ndPass(nOptions & SDRSEARCH_PASS2BOUND);
    const bool bCheckNearestOn3rdPass(nOptions & SDRSEARCH_PASS3NEAREST);
    rpObj = 0;

    if(pnMarkNum)
    {
        *pnMarkNum = CONTAINER_ENTRY_NOTFOUND;
    }

    double fTol(getHitTolLog());
    bool bFnd(false);
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    sal_uInt32 nMarkNum(0);

    for(nMarkNum = aSelection.size(); nMarkNum > 0 && !bFnd;)
    {
        nMarkNum--;
        SdrObject* pObj = aSelection[nMarkNum];
        bFnd = CheckSingleSdrObjectHit(rPnt, fTol, pObj, SDRSEARCH_TESTMARKABLE, 0);

        if(bFnd)
        {
            rpObj=pObj;

            if(pnMarkNum)
            {
                *pnMarkNum = nMarkNum;
            }
        }
    }

    if((bBoundCheckOn2ndPass || bCheckNearestOn3rdPass) && !bFnd)
    {
        SdrObject* pBestObj = 0;
        sal_uInt32 nBestMarkNum(0);
        double fBestDist(0.0);

        for(nMarkNum = aSelection.size(); nMarkNum > 0 && !bFnd;)
        {
            nMarkNum--;
            SdrObject* pObj = aSelection[nMarkNum];
            basegfx::B2DRange aObjRange(pObj->getObjectRange(getAsSdrView()));

            if(!basegfx::fTools::equalZero(fTol))
            {
                aObjRange.grow(fabs(fTol));
            }

            if(aObjRange.isInside(rPnt))
            {
                bFnd = true;
                rpObj=pObj;

                if(pnMarkNum)
                {
                    *pnMarkNum = nMarkNum;
                }
            }
            else if(bCheckNearestOn3rdPass)
            {
                double fDist(0.0);

                if(rPnt.getX() < aObjRange.getMinX())
                {
                    fDist += aObjRange.getMinX() - rPnt.getX();
                }

                if(rPnt.getX() > aObjRange.getMaxX())
                {
                    fDist += rPnt.getX() - aObjRange.getMaxX();
                }


                if(rPnt.getY() < aObjRange.getMinY())
                {
                    fDist += aObjRange.getMinY() - rPnt.getY();
                }


                if(rPnt.getY() > aObjRange.getMaxY())
                {
                    fDist += rPnt.getY() - aObjRange.getMaxY();
                }

                if(basegfx::fTools::equalZero(fBestDist) || basegfx::fTools::less(fDist, fBestDist))
                {
                    fBestDist = fDist;
                    pBestObj=pObj;
                    nBestMarkNum=nMarkNum;
                }
            }
        }

        if(bCheckNearestOn3rdPass && !bFnd)
        {
            rpObj=pBestObj;

            if(pnMarkNum)
            {
                *pnMarkNum = nBestMarkNum;
            }

            bFnd = (0 != pBestObj);
        }
    }

    return bFnd;
}

SdrHitKind SdrMarkView::PickSomething(const basegfx::B2DPoint& rPnt, double fTol) const
{
    SdrObject* pObj = 0;

    if(PickObj(rPnt, fTol, pObj, SDRSEARCH_PICKMARKABLE))
    {
        return SDRHIT_OBJECT;
    }

    return SDRHIT_NONE;
}

void SdrMarkView::UnmarkAllObj()
{
    if(areSdrObjectsSelected())
    {
        BrkAction();
        clearSdrObjectSelection();
    }
}

void SdrMarkView::MarkAllObj()
{
    BrkAction();
    const SdrPageView* pSourceSdrPageView = GetSdrPageView();

    if(pSourceSdrPageView)
    {
        const SdrObjList* pList = pSourceSdrPageView->GetCurrentObjectList();
        const sal_uInt32 nCount(pList->GetObjCount());
        SdrObjectVector aNewSelection;

        aNewSelection.reserve(nCount);

        for(sal_uInt32 a(0); a < nCount; a++)
        {
            SdrObject* pObj = pList->GetObj(a);

            if(pSourceSdrPageView->IsObjMarkable(*pObj))
            {
                aNewSelection.push_back(pObj);
            }
        }

        setSdrObjectSelection(aNewSelection);
    }
}

const basegfx::B2DRange& SdrMarkView::getMarkedObjectSnapRange() const
{
    return maSelection.getSnapRange();
}

void SdrMarkView::TakeMarkedDescriptionString(sal_uInt16 nStrCacheID, XubString& rStr, sal_uInt16 nVal, sal_uInt16 nOpt) const
{
    if(areSdrObjectsSelected())
    {
        rStr = ImpGetResStr(nStrCacheID);
        xub_StrLen nPos = rStr.SearchAscii("%1");

        if(nPos != STRING_NOTFOUND)
        {
            rStr.Erase(nPos, 2);

            if(nOpt == IMPSDR_POINTSDESCRIPTION)
            {
                rStr.Insert(getSelectedPointsDescription(), nPos);
            }
            else if(nOpt == IMPSDR_GLUEPOINTSDESCRIPTION)
            {
                rStr.Insert(getSelectedGluesDescription(), nPos);
            }
            else
            {
                const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
                rStr.Insert(getSelectionDescription(aSelection), nPos);
            }
        }

        nPos = rStr.SearchAscii("%2");

        if(nPos != STRING_NOTFOUND)
        {
            rStr.Erase(nPos, 2);
            rStr.Insert(UniString::CreateFromInt32(nVal), nPos);
        }
    }
}

bool SdrMarkView::EnterMarkedGroup()
{
    bool bRet(false);
    SdrPageView* pPV = GetSdrPageView();

    if(pPV && areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        bool bEnter(false);

        for(sal_uInt32 nm(aSelection.size()); nm > 0 && !bEnter;)
        {
            nm--;

            SdrObject* pObj = aSelection[nm];

            if(pObj->getChildrenOfSdrObject())
            {
                if(pPV->EnterGroup(pObj))
                {
                    bRet = true;
                    bEnter = true;
                }
            }
        }
    }

    return bRet;
}

void SdrMarkView::handleSelectionChange()
{
    // call parent
    SdrSnapView::handleSelectionChange();

    // local reactions
    maMarkedPointRange.reset();
    maMarkedGluePointRange.reset();

#ifdef DBG_UTIL
    // evtl. reset ItemBrowser
    if(GetItemBrowser())
    {
        mpItemBrowser->SetDirty();
    }
#endif

    // handle GlueVisible flag
    ImpSetGlueVisible4(0 != dynamic_cast< SdrEdgeObj* >(getSelectedIfSingle()));

    // visualize new selection
    RecreateAllMarkHandles();
}

void SdrMarkView::SetMoveOutside(bool bOn)
{
    maViewSdrHandleList.SetMoveOutside(bOn);
    RecreateAllMarkHandles();
}

bool SdrMarkView::IsMoveOutside() const
{
    return maViewSdrHandleList.IsMoveOutside();
}

void SdrMarkView::SetDesignMode( bool _bOn )
{
    if ( IsDesignMode() != _bOn )
    {
        mbDesignMode = _bOn;
        SdrPageView* pPageView = GetSdrPageView();
        if ( pPageView )
            pPageView->SetDesignMode( _bOn );
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof
