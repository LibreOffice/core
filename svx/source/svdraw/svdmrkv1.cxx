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
#include "svx/svdglob.hxx"
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include "svddrgm1.hxx"
#include <svx/svdlegacy.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrMarkView::HasMarkablePoints() const
{
    bool bRet(false);

    if(!ImpIsFrameHandles() && areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if(aSelection.size() <= FRAME_HANDLES_LIMIT)
        {
            for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size() && !bRet; nMarkNum++)
            {
                bRet = aSelection[nMarkNum]->IsPolygonObject();
            }
        }
    }

    return bRet;
}

sal_uInt32 SdrMarkView::GetMarkablePointCount() const
{
    sal_uInt32 nAnz(0);

    if(!ImpIsFrameHandles() && areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if(aSelection.size() <= FRAME_HANDLES_LIMIT)
        {
            for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size(); nMarkNum++)
            {
                const SdrObject* pObj = aSelection[nMarkNum];

                if(pObj->IsPolygonObject())
                {
                    nAnz += pObj->GetObjectPointCount();
                }
            }
        }
    }

    return nAnz;
}

bool SdrMarkView::HasMarkedPoints() const
{
    return arePointsSelected();
}

sal_uInt32 SdrMarkView::GetMarkedPointCount() const
{
    sal_uInt32 nAnz(0);

    if(!ImpIsFrameHandles() && areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if(aSelection.size() <= FRAME_HANDLES_LIMIT)
        {
            for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size(); nMarkNum++)
            {
                const sdr::selection::Indices aMarkedPoints(getSelectedPointsForSelectedSdrObject(*aSelection[nMarkNum]));

                nAnz += aMarkedPoints.size();
            }
        }
    }

    return nAnz;
}

bool SdrMarkView::IsPointMarkable(const SdrHdl& rHdl) const
{
    return !ImpIsFrameHandles()
        && (&rHdl)
        && !rHdl.IsPlusHdl()
        && HDL_GLUE != rHdl.GetKind()
        && HDL_SMARTTAG != rHdl.GetKind()
        && rHdl.GetObj()
        && rHdl.GetObj()->IsPolygonObject();
}

bool SdrMarkView::MarkPointHelper(SdrHdl* pHdl, bool bUnmark)
{
    SdrHdlContainerType aHdl;

    if(pHdl)
    {
        aHdl.push_back(pHdl);
    }

    return ImpMarkPoints(aHdl, bUnmark);
}

bool SdrMarkView::ImpMarkPoints(const SdrHdlContainerType& rHdls, bool bUnmark)
{
    sdr::selection::Indices aMarkedPoints;
    bool bRetval(true);
    bool bChanged(false);
    const SdrObject* pLastObj = 0;

    for(sal_uInt32 a(0); a < rHdls.size(); a++)
    {
        const SdrHdl* pCandidate = rHdls[a];

        if(!pCandidate)
        {
            OSL_ENSURE(false, "OOps, null-ptr in SdhHdl list (!)");
            continue;
        }

        if(!pCandidate || pCandidate->IsPlusHdl() || HDL_GLUE == pCandidate->GetKind())
        {
            continue;
        }

        if(pCandidate->IsSelected() != bUnmark)
        {
            continue;
        }

        const SdrObject* pObj = pCandidate->GetObj();

        if(!pObj || !pObj->IsPolygonObject())
        {
            continue;
        }

        if(!pLastObj || pLastObj != pObj)
        {
            if(pLastObj && bChanged)
            {
                setSelectedPointsForSelectedSdrObject(*pLastObj, aMarkedPoints);
            }

            bChanged = false;
            pLastObj = pObj;
            aMarkedPoints = getSelectedPointsForSelectedSdrObject(*pObj);
        }

        const sal_uInt32 nHdlNum(pCandidate->GetObjHdlNum());

        if (!bUnmark)
        {
            aMarkedPoints.insert(nHdlNum);
            bChanged = true;
        }
        else
        {
            if(aMarkedPoints.erase(nHdlNum))
            {
                bChanged = true;
            }
            else
            {
                bRetval = false;
            }
        }

        // TTTT: should not be needed, setSelectedPointsForSelectedSdrObject already triggers selection change and starts recreation
        // pCandidate->SetSelected(!bUnmark);
    }

    if(pLastObj && bChanged)
    {
        setSelectedPointsForSelectedSdrObject(*pLastObj, aMarkedPoints);
    }

    return bRetval;
}


bool SdrMarkView::MarkPoint(SdrHdl& rHdl, bool bUnmark)
{
    if(!(&rHdl))
    {
        OSL_ENSURE(false, "OOps, a null-pointer to a SdrHdl as reference was handed over (!)");
        return false;
    }

    bool bRet(false);

    if(IsPointMarkable(rHdl) && rHdl.IsSelected() == bUnmark)
    {
        const SdrObject* pObj = rHdl.GetObj();

        if(isSdrObjectSelected(*pObj))
        {
            SdrHdlContainerType aHdl;

            aHdl.push_back(&rHdl);

            if(ImpMarkPoints(aHdl, bUnmark))
            {
                bRet = true;
            }
        }
    }

    return bRet;
}

void SdrMarkView::MarkPoints(const basegfx::B2DRange* pRange, bool bUnmark)
{
    const SdrObject* pObj0 = 0;
    const SdrHdlList& rHdlList = GetHdlList();
    const sal_uInt32 nHdlAnz(rHdlList.GetHdlCount());
    SdrHdlContainerType aHdl;

    for(sal_uInt32 nHdlNum(nHdlAnz); nHdlNum > 0;)
    {
        nHdlNum--;
        SdrHdl* pHdl = rHdlList.GetHdlByIndex(nHdlNum);

        if(pHdl && IsPointMarkable(*pHdl) && pHdl->IsSelected() == bUnmark)
        {
            if(!pRange || pRange->isInside(pHdl->getPosition()))
            {
                aHdl.push_back(pHdl);
            }
        }
    }

    if(aHdl.size())
    {
        ImpMarkPoints(aHdl, bUnmark);
    }
}

const basegfx::B2DRange& SdrMarkView::getMarkedPointRange() const
{
    if(maMarkedPointRange.isEmpty())
    {
        impCreatePointRanges();
    }

    return maMarkedPointRange;
}

void SdrMarkView::SetPlusHandlesAlwaysVisible(bool bOn)
{
    if(bOn != IsPlusHandlesAlwaysVisible())
    {
        mbPlusHdlAlways = bOn;
        RecreateAllMarkHandles();
    }
}

void SdrMarkView::impCreatePointRanges() const
{
    basegfx::B2DRange aNewMarkedPointRange;
    basegfx::B2DRange aNewMarkedGluePointRange;
    const SdrHdlList& rHdlList = GetHdlList();
    const sal_uInt32 nCount(rHdlList.GetHdlCount());

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        const SdrHdl* pHdl = rHdlList.GetHdlByIndex(a);
        const SdrHdlKind eKind(pHdl->GetKind());

        if(HDL_POLY == eKind && pHdl->IsSelected())
        {
            aNewMarkedPointRange.expand(pHdl->getPosition());
        }
        else if(HDL_GLUE == eKind)
        {
            aNewMarkedGluePointRange.expand(pHdl->getPosition());
        }
    }

    const_cast< SdrMarkView* >(this)->maMarkedPointRange = aNewMarkedPointRange;
    const_cast< SdrMarkView* >(this)->maMarkedGluePointRange = aNewMarkedGluePointRange;
}

bool SdrMarkView::HasMarkableGluePoints() const
{
    if(IsGluePointEditMode() && areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size(); nMarkNum++)
        {
            const SdrObject* pObj = aSelection[nMarkNum];

            if(pObj)
            {
                const sdr::glue::GluePointProvider& rProvider = pObj->GetGluePointProvider();

                if(rProvider.hasUserGluePoints())
                {
                    const sdr::glue::GluePointVector aCandidates(rProvider.getUserGluePointVector());

                    for(sal_uInt32 a(0); a < aCandidates.size(); a++)
                    {
                        const sdr::glue::GluePoint* pCandidate = aCandidates[a];

                        if(pCandidate)
                        {
                            if(aCandidates[a]->getUserDefined())
                            {
                                return true;
                            }
                        }
                        else
                        {
                            OSL_ENSURE(false, "Got a sdr::glue::PointVector with empty spots (!)");
                        }
                    }
                }
            }
            else
            {
                OSL_ENSURE(false, "Got empty entry in selection list (!)");
            }
        }
    }

    return false;
}

sal_uInt32 SdrMarkView::GetMarkableGluePointCount() const
{
    sal_uInt32 nAnz(0);

    if(IsGluePointEditMode() && areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size(); nMarkNum++)
        {
            const SdrObject* pObj = aSelection[nMarkNum];

            if(pObj)
            {
                const sdr::glue::GluePointProvider& rProvider = pObj->GetGluePointProvider();

                if(rProvider.hasUserGluePoints())
                {
                    const sdr::glue::GluePointVector aCandidates(rProvider.getUserGluePointVector());

                    for(sal_uInt32 a(0); a < aCandidates.size(); a++)
                    {
                        const sdr::glue::GluePoint* pCandidate = aCandidates[a];

                        if(pCandidate)
                        {
                            if(aCandidates[a]->getUserDefined())
                            {
                                nAnz++;
                            }
                        }
                        else
                        {
                            OSL_ENSURE(false, "Got a sdr::glue::PointVector with empty spots (!)");
                        }
                    }
                }
            }
            else
            {
                OSL_ENSURE(false, "Got object selection with empty slots (!)");
            }
        }
    }

    return nAnz;
}

sal_uInt32 SdrMarkView::GetMarkedGluePointCount() const
{
    sal_uInt32 nAnz(0);

    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size(); nMarkNum++)
        {
            const sdr::selection::Indices aMarkedGluePoints(getSelectedGluesForSelectedSdrObject(*aSelection[nMarkNum]));

            nAnz += aMarkedGluePoints.size();
        }
    }

    return nAnz;
}

void SdrMarkView::MarkGluePoints(const basegfx::B2DRange* pRange, bool bUnmark)
{
    if(!IsGluePointEditMode() && !bUnmark)
    {
        return;
    }

    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size(); nMarkNum++)
    {
        const SdrObject* pObj = aSelection[nMarkNum];

        if(!pObj)
        {
            OSL_ENSURE(false, "Got SdrObjectVector from selection with empty entries (!)");
        }
        else
        {
            sdr::selection::Indices aMarkedGluePoints(getSelectedGluesForSelectedSdrObject(*pObj));
            bool bGluePointsChanged(false);

            if(bUnmark && !pRange)
            {
                // UnmarkAll
                if(aMarkedGluePoints.size())
                {
                    aMarkedGluePoints.clear();
                    bGluePointsChanged = true;
                }
            }
            else
            {
                const sdr::glue::GluePointProvider& rProvider = pObj->GetGluePointProvider();

                if(rProvider.hasUserGluePoints() && (!aMarkedGluePoints.empty() || !bUnmark))
                {
                    const sdr::glue::GluePointVector aCandidates(rProvider.getUserGluePointVector());

                    for(sal_uInt32 a(0); a < aCandidates.size(); a++)
                    {
                        const sdr::glue::GluePoint* pCandidate = aCandidates[a];

                        if(pCandidate)
                        {
                            if(pCandidate->getUserDefined())
                            {
                                if(!pRange || pRange->isInside(pObj->getSdrObjectTransformation() * pCandidate->getUnitPosition()))
                                {
                                    sdr::selection::Indices::iterator aFound(aMarkedGluePoints.find(pCandidate->getID()));

                                    if(bUnmark)
                                    {
                                        if(aFound != aMarkedGluePoints.end())
                                        {
                                            aMarkedGluePoints.erase(aFound);
                                            bGluePointsChanged = true;
                                        }
                                    }
                                    else
                                    {
                                        if(aFound == aMarkedGluePoints.end())
                                        {
                                            aMarkedGluePoints.insert(pCandidate->getID());
                                            bGluePointsChanged = true;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            OSL_ENSURE(false, "Got a sdr::glue::PointVector with empty spots (!)");
                        }
                    }
                }
            }

            if(bGluePointsChanged)
            {
                setSelectedGluesForSelectedSdrObject(*pObj, aMarkedGluePoints);
            }
        }
    }
}

bool SdrMarkView::PickGluePoint(const basegfx::B2DPoint& rPnt, SdrObject*& rpObj, sal_uInt32& rnId) const
{
    rpObj = 0;
    rnId = 0;

    if(!IsGluePointEditMode())
    {
        return false;
    }

    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    sal_uInt32 nMarkNum(aSelection.size());

    while(nMarkNum > 0)
    {
        nMarkNum--;
        SdrObject* pObj = aSelection[nMarkNum];

        if(pObj)
        {
            const sdr::glue::GluePointProvider& rProvider = pObj->GetGluePointProvider();

            if(rProvider.hasUserGluePoints())
            {
                const sdr::glue::GluePointVector aCandidates(rProvider.getUserGluePointVector());

                for(sal_uInt32 a(0); a < aCandidates.size(); a++)
                {
                    const sdr::glue::GluePoint* pCandidate = aCandidates[a];

                    if(pCandidate)
                    {
                        if(pCandidate->getUserDefined())
                        {
                            const basegfx::B2DPoint aAbsolutePos(pObj->getSdrObjectTransformation() * pCandidate->getUnitPosition());
                            const double fDist(basegfx::B2DVector(aAbsolutePos - rPnt).getLength());

                            if(basegfx::fTools::lessOrEqual(fDist, getHitTolLog()))
                            {
                                rpObj = pObj;
                                rnId = pCandidate->getID();

                                return true;
                            }
                        }
                    }
                    else
                    {
                        OSL_ENSURE(false, "Got a sdr::glue::PointVector with empty spots (!)");
                    }
                }
            }
        }
        else
        {
            OSL_ENSURE(false, "Got a selection with empty slots (!)");
        }
    }

    return false;
}

bool SdrMarkView::MarkGluePoint(const SdrObject* pObj, sal_uInt32 nId, bool bUnmark)
{
    if(!IsGluePointEditMode())
    {
        return false;
    }

    bool bChgd(false);

    if(pObj)
    {
        if(isSdrObjectSelected(*pObj))
        {
            sdr::selection::Indices aMarkedGluePoints(getSelectedGluesForSelectedSdrObject(*pObj));
            sdr::selection::Indices::iterator aFound(aMarkedGluePoints.find(nId));
            bool bGluePointsChanged(false);

            if(bUnmark)
            {
                if(aFound != aMarkedGluePoints.end())
                {
                    aMarkedGluePoints.erase(aFound);
                    bGluePointsChanged = true;
                }
            }
            else
            {
                if(aFound == aMarkedGluePoints.end())
                {
                    aMarkedGluePoints.insert(nId);
                    bGluePointsChanged = true;
                }
            }

            if(bGluePointsChanged)
            {
                bChgd = true;
                setSelectedGluesForSelectedSdrObject(*pObj, aMarkedGluePoints);
            }
        }
        else
        {
            // Objekt implizit markieren ...
            // ... fehlende Implementation
        }
    }

    return bChgd;
}

bool SdrMarkView::IsGluePointMarked(const SdrObject& rObj, sal_uInt32 nId) const
{
    return (0 != getSelectedGluesForSelectedSdrObject(rObj).count(nId));
}

bool SdrMarkView::UnmarkGluePoint(const SdrHdl& rHdl)
{
    if(&rHdl && HDL_GLUE == rHdl.GetKind() && rHdl.GetObj())
    {
        return MarkGluePoint(rHdl.GetObj(), rHdl.GetObjHdlNum(), true);
    }
    else
    {
        return false;
    }
}

SdrHdl* SdrMarkView::GetGluePointHdl(const SdrObject* pObj, sal_uInt32 nId) const
{
    const SdrHdlList& rHdlList = GetHdlList();
    const sal_uInt32 nHdlAnz(rHdlList.GetHdlCount());

    for(sal_uInt32 nHdlNum(0); nHdlNum < nHdlAnz; nHdlNum++)
    {
        SdrHdl* pHdl = rHdlList.GetHdlByIndex(nHdlNum);

        if (pHdl->GetObj()==pObj &&
            HDL_GLUE == pHdl->GetKind() &&
            pHdl->GetObjHdlNum() == nId)
        {
            return pHdl;
        }
    }

    return 0;
}

const basegfx::B2DRange& SdrMarkView::getMarkedGluePointRange() const
{
    if(maMarkedGluePointRange.isEmpty())
    {
        impCreatePointRanges();
    }

    return maMarkedGluePointRange;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
