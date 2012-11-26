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
    return ImpMarkPoint(pHdl, bUnmark);
}

bool SdrMarkView::ImpMarkPoint(SdrHdl* pHdl, bool bUnmark)
{
    if(!pHdl || pHdl->IsPlusHdl() || HDL_GLUE == pHdl->GetKind())
    {
        return false;
    }

    if(pHdl->IsSelected() != bUnmark)
    {
        return false;
    }

    const SdrObject* pObj = pHdl->GetObj();

    if(!pObj || !pObj->IsPolygonObject())
    {
        return false;
    }

    sdr::selection::Indices aMarkedPoints(getSelectedPointsForSelectedSdrObject(*pObj));
    const sal_uInt32 nHdlNum(pHdl->GetObjHdlNum());

    if (!bUnmark)
    {
        aMarkedPoints.insert(nHdlNum);
    }
    else
    {
        if(!aMarkedPoints.erase(nHdlNum))
        {
            return false;
        }
    }

    setSelectedPointsForSelectedSdrObject(*pObj, aMarkedPoints);
    pHdl->SetSelected(!bUnmark);

    return true;
}


bool SdrMarkView::MarkPoint(SdrHdl& rHdl, bool bUnmark)
{
    if(!(&rHdl))
    {
        return false;
    }

    bool bRet(false);

    if(IsPointMarkable(rHdl) && rHdl.IsSelected() == bUnmark)
    {
    const SdrObject* pObj=rHdl.GetObj();

        if(isSdrObjectSelected(*pObj))
        {
            if(ImpMarkPoint(&rHdl, bUnmark))
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

    maViewHandleList.Sort();
    const sal_uInt32 nHdlAnz(maViewHandleList.GetHdlCount());

    for(sal_uInt32 nHdlNum(nHdlAnz); nHdlNum > 0;)
    {
        nHdlNum--;
        SdrHdl* pHdl = maViewHandleList.GetHdlByIndex(nHdlNum);

        if(IsPointMarkable(*pHdl) && pHdl->IsSelected() == bUnmark)
        {
            if(!pRange || pRange->isInside(pHdl->getPosition()))
            {
                ImpMarkPoint(pHdl, bUnmark);
            }
        }
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
        SetMarkHandles();
    }
}

void SdrMarkView::impCreatePointRanges() const
{
    basegfx::B2DRange aNewMarkedPointRange;
    basegfx::B2DRange aNewMarkedGluePointRange;
    const sal_uInt32 nCount(maViewHandleList.GetHdlCount());

    for(sal_uInt32 a(0); a < nCount; a++)
                {
        const SdrHdl* pHdl = maViewHandleList.GetHdlByIndex(a);
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
    bool bRet(false);

    if(IsGluePointEditMode() && areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size() && !bRet; nMarkNum++)
        {
            const SdrObject* pObj = aSelection[nMarkNum];
            const SdrGluePointList* pGPL=pObj->GetGluePointList();

            if(pGPL && pGPL->GetCount())
            {
                for(sal_uInt32 a(0); !bRet && a < pGPL->GetCount(); a++)
                {
                    if((*pGPL)[a].IsUserDefined())
                    {
                        bRet = true;
                    }
                }
            }
        }
    }

    return bRet;
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
            const SdrGluePointList* pGPL=pObj->GetGluePointList();

            if(pGPL && pGPL->GetCount())
            {
                for(sal_uInt32 a(0); a < pGPL->GetCount(); a++)
                {
                    if((*pGPL)[a].IsUserDefined())
                    {
                        nAnz++;
                    }
                }
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
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
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
            if(pGPL && (!aMarkedGluePoints.empty() || !bUnmark))
            {
                const sal_uInt32 nGPAnz(pGPL->GetCount());
                const basegfx::B2DRange aObjSnapRange(nGPAnz ? sdr::legacy::GetSnapRange(*pObj) : basegfx::B2DRange());

                for(sal_uInt32 nGPNum(0); nGPNum < nGPAnz; nGPNum++)
                {
                    const SdrGluePoint& rGP=(*pGPL)[nGPNum];

                    if(rGP.IsUserDefined())
                    {
                        if(!pRange || pRange->isInside(rGP.GetAbsolutePos(aObjSnapRange)))
                        {
                            sdr::selection::Indices::iterator aFound(aMarkedGluePoints.find(rGP.GetId()));

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
                                    aMarkedGluePoints.insert(rGP.GetId());
                                    bGluePointsChanged = true;
                                }
                            }
                        }
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

bool SdrMarkView::PickGluePoint(const basegfx::B2DPoint& rPnt, SdrObject*& rpObj, sal_uInt32& rnId, sal_uInt32 nOptions) const
{
    rpObj = 0;
    rnId = 0;

    if(!IsGluePointEditMode())
    {
        return false;
    }

    SdrObject* pObj0=rpObj;
    sal_uInt32 nId0 = rnId;
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    sal_uInt32 nMarkNum(aSelection.size());

    while(nMarkNum > 0)
    {
        nMarkNum--;
        SdrObject* pObj = aSelection[nMarkNum];
        const SdrGluePointList* pGPL=pObj->GetGluePointList();

        if(pGPL)
        {
            const sal_uInt32 nNum(pGPL->GPLHitTest(rPnt, getHitTolLog(), sdr::legacy::GetSnapRange(*pObj), false, nId0));

            if(SDRGLUEPOINT_NOTFOUND != nNum)
            {
                // #i38892#
                const SdrGluePoint& rCandidate = (*pGPL)[nNum];

                if(rCandidate.IsUserDefined())
                {
                    rpObj=pObj;
                    rnId=(*pGPL)[nNum].GetId();

                    return true;
                }
            }
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
    const sal_uInt32 nHdlAnz(maViewHandleList.GetHdlCount());

    for(sal_uInt32 nHdlNum(0); nHdlNum < nHdlAnz; nHdlNum++)
    {
        SdrHdl* pHdl = maViewHandleList.GetHdlByIndex(nHdlNum);

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
