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

#include <sal/config.h>

#include <osl/time.h>
#include <svx/svdmark.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdorect.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>


#include <svx/obj3d.hxx>
#include <svx/scene3d.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svx/svdoedge.hxx>

#include <cassert>

void SdrMark::setTime()
{
    TimeValue aNow;
    osl_getSystemTime(&aNow);
    mnTimeStamp = sal_Int64(aNow.Seconds) * 1000000000L + aNow.Nanosec;
}

SdrMark::SdrMark(SdrObject* pNewObj, SdrPageView* pNewPageView)
:   mpSelectedSdrObject(pNewObj),
    mpPageView(pNewPageView),
    mbCon1(false),
    mbCon2(false),
    mnUser(0)
{
    if(mpSelectedSdrObject)
    {
        mpSelectedSdrObject->AddObjectUser( *this );
    }
    setTime();
}

SdrMark::SdrMark(const SdrMark& rMark)
:   ObjectUser(),
    mnTimeStamp(0),
    mpSelectedSdrObject(nullptr),
    mpPageView(nullptr),
    mbCon1(false),
    mbCon2(false),
    mnUser(0)
{
    *this = rMark;
}

SdrMark::~SdrMark()
{
    if (mpSelectedSdrObject)
    {
        mpSelectedSdrObject->RemoveObjectUser( *this );
    }
}

void SdrMark::ObjectInDestruction(const SdrObject& rObject)
{
    (void) rObject; // avoid warnings
    OSL_ENSURE(mpSelectedSdrObject && mpSelectedSdrObject == &rObject, "SdrMark::ObjectInDestruction: called form object different from hosted one (!)");
    OSL_ENSURE(mpSelectedSdrObject, "SdrMark::ObjectInDestruction: still selected SdrObject is deleted, deselect first (!)");
    mpSelectedSdrObject = nullptr;
}

void SdrMark::SetMarkedSdrObj(SdrObject* pNewObj)
{
    if(mpSelectedSdrObject)
    {
        mpSelectedSdrObject->RemoveObjectUser( *this );
    }

    mpSelectedSdrObject = pNewObj;

    if(mpSelectedSdrObject)
    {
        mpSelectedSdrObject->AddObjectUser( *this );
    }
}

SdrMark& SdrMark::operator=(const SdrMark& rMark)
{
    SetMarkedSdrObj(rMark.mpSelectedSdrObject);

    mnTimeStamp = rMark.mnTimeStamp;
    mpPageView = rMark.mpPageView;
    mbCon1 = rMark.mbCon1;
    mbCon2 = rMark.mbCon2;
    mnUser = rMark.mnUser;
    maPoints = rMark.maPoints;
    maGluePoints = rMark.maGluePoints;

    return *this;
}

static bool ImpSdrMarkListSorter(std::unique_ptr<SdrMark> const& lhs, std::unique_ptr<SdrMark> const& rhs)
{
    SdrObject* pObj1 = lhs->GetMarkedSdrObj();
    SdrObject* pObj2 = rhs->GetMarkedSdrObj();
    SdrObjList* pOL1 = pObj1 ? pObj1->getParentSdrObjListFromSdrObject() : nullptr;
    SdrObjList* pOL2 = pObj2 ? pObj2->getParentSdrObjListFromSdrObject() : nullptr;

    if (pOL1 == pOL2)
    {
        // AF: Note that I reverted a change from sal_uInt32 to sal_uLong (made
        // for 64bit compliance, #i78198#) because internally in SdrObject
        // both nOrdNum and mnNavigationPosition are stored as sal_uInt32.
        sal_uInt32 nObjOrd1(pObj1 ? pObj1->GetNavigationPosition() : 0);
        sal_uInt32 nObjOrd2(pObj2 ? pObj2->GetNavigationPosition() : 0);

        return nObjOrd1 < nObjOrd2;
    }
    else
    {
        return pOL1 < pOL2;
    }
}


void SdrMarkList::ForceSort() const
{
    if(!mbSorted)
    {
        const_cast<SdrMarkList*>(this)->ImpForceSort();
    }
}

void SdrMarkList::ImpForceSort()
{
    if(!mbSorted)
    {
        mbSorted = true;
        size_t nCount = maList.size();

        // remove invalid
        if(nCount > 0 )
        {
            maList.erase(std::remove_if(maList.begin(), maList.end(),
                [](std::unique_ptr<SdrMark>& rItem) { return rItem.get()->GetMarkedSdrObj() == nullptr; }),
                maList.end());
            nCount = maList.size();
        }

        if(nCount > 1)
        {
            std::sort(maList.begin(), maList.end(), ImpSdrMarkListSorter);

            // remove duplicates
            if(maList.size() > 1)
            {
                SdrMark* pCurrent = maList.back().get();
                for (size_t count = maList.size() - 1; count; --count)
                {
                    size_t i = count - 1;
                    SdrMark* pCmp = maList[i].get();
                    assert(pCurrent->GetMarkedSdrObj());
                    if(pCurrent->GetMarkedSdrObj() == pCmp->GetMarkedSdrObj())
                    {
                        // Con1/Con2 Merging
                        if(pCmp->IsCon1())
                            pCurrent->SetCon1(true);

                        if(pCmp->IsCon2())
                            pCurrent->SetCon2(true);

                        // delete pCmp
                        maList.erase(maList.begin() + i);
                    }
                    else
                    {
                        pCurrent = pCmp;
                    }
                }
            }
        }
    }
}

void SdrMarkList::Clear()
{
    maList.clear();
    mbSorted = true; //we're empty, so can be considered sorted
    SetNameDirty();
}

SdrMarkList& SdrMarkList::operator=(const SdrMarkList& rLst)
{
    if (this != &rLst)
    {
        Clear();

        for(size_t i = 0; i < rLst.GetMarkCount(); ++i)
        {
            SdrMark* pMark = rLst.GetMark(i);
            maList.emplace_back(new SdrMark(*pMark));
        }

        maMarkName = rLst.maMarkName;
        mbNameOk = rLst.mbNameOk;
        maPointName = rLst.maPointName;
        mbPointNameOk = rLst.mbPointNameOk;
        maGluePointName = rLst.maGluePointName;
        mbSorted = rLst.mbSorted;
    }
    return *this;
}

SdrMark* SdrMarkList::GetMark(size_t nNum) const
{
    return (nNum < maList.size()) ? maList[nNum].get() : nullptr;
}

size_t SdrMarkList::FindObject(const SdrObject* pObj) const
{
    // Since relying on OrdNums is not allowed for the selection because objects in the
    // selection may not be inserted in a list if they are e.g. modified ATM, i changed
    // this loop to just look if the object pointer is in the selection.

    // Problem is that GetOrdNum() which is const, internally casts to non-const and
    // hardly sets the OrdNum member of the object (nOrdNum) to 0 (ZERO) if the object
    // is not inserted in a object list.
    // Since this may be by purpose and necessary somewhere else i decided that it is
    // less dangerous to change this method then changing SdrObject::GetOrdNum().
    if(pObj)
    {
        for(size_t a = 0; a < maList.size(); ++a)
        {
            if(maList[a]->GetMarkedSdrObj() == pObj)
            {
                return a;
            }
        }
    }

    return SAL_MAX_SIZE;
}

void SdrMarkList::InsertEntry(const SdrMark& rMark, bool bChkSort)
{
    SetNameDirty();
    const size_t nCount(maList.size());

    if(!bChkSort || !mbSorted || nCount == 0)
    {
        if(!bChkSort)
            mbSorted = false;

        maList.emplace_back(new SdrMark(rMark));
    }
    else
    {
        SdrMark* pLast = GetMark(nCount - 1);
        const SdrObject* pLastObj = pLast->GetMarkedSdrObj();
        const SdrObject* pNewObj = rMark.GetMarkedSdrObj();

        if(pLastObj == pNewObj)
        {
            // This one already exists.
            // Con1/Con2 Merging
            if(rMark.IsCon1())
                pLast->SetCon1(true);

            if(rMark.IsCon2())
                pLast->SetCon2(true);
        }
        else
        {
            maList.emplace_back(new SdrMark(rMark));

            // now check if the sort is ok
            const SdrObjList* pLastOL = pLastObj!=nullptr ? pLastObj->getParentSdrObjListFromSdrObject() : nullptr;
            const SdrObjList* pNewOL = pNewObj !=nullptr ? pNewObj->getParentSdrObjListFromSdrObject() : nullptr;

            if(pLastOL == pNewOL)
            {
                const sal_uLong nLastNum(pLastObj!=nullptr ? pLastObj->GetOrdNum() : 0);
                const sal_uLong nNewNum(pNewObj !=nullptr ? pNewObj ->GetOrdNum() : 0);

                if(nNewNum < nLastNum)
                {
                    // at some point, we have to sort
                    mbSorted = false;
                }
            }
            else
            {
                // at some point, we have to sort
                mbSorted = false;
            }
        }
    }
}

void SdrMarkList::DeleteMark(size_t nNum)
{
    SdrMark* pMark = GetMark(nNum);
    DBG_ASSERT(pMark!=nullptr,"DeleteMark: MarkEntry not found.");

    if(pMark)
    {
        maList.erase(maList.begin() + nNum);
        if (maList.empty())
            mbSorted = true; //we're empty, so can be considered sorted
        SetNameDirty();
    }
}

void SdrMarkList::ReplaceMark(const SdrMark& rNewMark, size_t nNum)
{
    SdrMark* pMark = GetMark(nNum);
    DBG_ASSERT(pMark!=nullptr,"ReplaceMark: MarkEntry not found.");

    if(pMark)
    {
        SetNameDirty();
        maList[nNum].reset(new SdrMark(rNewMark));
        mbSorted = false;
    }
}

void SdrMarkList::Merge(const SdrMarkList& rSrcList, bool bReverse)
{
    const size_t nCount(rSrcList.maList.size());

    if(rSrcList.mbSorted)
    {
        // merge without forcing a Sort in rSrcList
        bReverse = false;
    }

    if(!bReverse)
    {
        for(size_t i = 0; i < nCount; ++i)
        {
            SdrMark* pM = rSrcList.maList[i].get();
            InsertEntry(*pM);
        }
    }
    else
    {
        for(size_t i = nCount; i > 0;)
        {
            --i;
            SdrMark* pM = rSrcList.maList[i].get();
            InsertEntry(*pM);
        }
    }
}

bool SdrMarkList::DeletePageView(const SdrPageView& rPV)
{
    bool bChgd(false);

    for(auto it = maList.begin(); it != maList.end(); )
    {
        SdrMark* pMark = it->get();

        if(pMark->GetPageView()==&rPV)
        {
            it = maList.erase(it);
            SetNameDirty();
            bChgd = true;
        }
        else
            ++it;
    }

    return bChgd;
}

bool SdrMarkList::InsertPageView(const SdrPageView& rPV)
{
    bool bChgd(false);
    DeletePageView(rPV); // delete all of them, then append the entire page
    const SdrObjList* pOL = rPV.GetObjList();
    const size_t nObjCount(pOL->GetObjCount());

    for(size_t nO = 0; nO < nObjCount; ++nO)
    {
        SdrObject* pObj = pOL->GetObj(nO);
        bool bDoIt(rPV.IsObjMarkable(pObj));

        if(bDoIt)
        {
            maList.emplace_back(new SdrMark(pObj, const_cast<SdrPageView*>(&rPV)));
            SetNameDirty();
            bChgd = true;
        }
    }

    return bChgd;
}

const OUString& SdrMarkList::GetMarkDescription() const
{
    const size_t nCount(GetMarkCount());

    if(mbNameOk && 1 == nCount)
    {
        // if it's a single selection, cache only text frame
        const SdrObject* pObj = GetMark(0)->GetMarkedSdrObj();
        const SdrTextObj* pTextObj = dynamic_cast<const SdrTextObj*>( pObj );

        if(!pTextObj || !pTextObj->IsTextFrame())
        {
            const_cast<SdrMarkList*>(this)->mbNameOk = false;
        }
    }

    if(!mbNameOk)
    {
        SdrMark* pMark = GetMark(0);
        OUString aNam;

        if(!nCount)
        {
            const_cast<SdrMarkList*>(this)->maMarkName = SvxResId(STR_ObjNameNoObj);
        }
        else if(1 == nCount)
        {
            if(pMark->GetMarkedSdrObj())
            {
                aNam = pMark->GetMarkedSdrObj()->TakeObjNameSingul();
            }
        }
        else
        {
            if(pMark->GetMarkedSdrObj())
            {
                aNam = pMark->GetMarkedSdrObj()->TakeObjNamePlural();
                bool bEq(true);

                for(size_t i = 1; i < GetMarkCount() && bEq; ++i)
                {
                    SdrMark* pMark2 = GetMark(i);
                    OUString aStr1(pMark2->GetMarkedSdrObj()->TakeObjNamePlural());
                    bEq = aNam == aStr1;
                }

                if(!bEq)
                {
                    aNam = SvxResId(STR_ObjNamePlural);
                }
            }

            aNam = OUString::number( nCount ) + " " + aNam;
        }

        const_cast<SdrMarkList*>(this)->maMarkName = aNam;
        const_cast<SdrMarkList*>(this)->mbNameOk = true;
    }

    return maMarkName;
}

const OUString& SdrMarkList::GetPointMarkDescription(bool bGlue) const
{
    bool& rNameOk = const_cast<bool&>(bGlue ? mbGluePointNameOk : mbPointNameOk);
    OUString& rName = const_cast<OUString&>(bGlue ? maGluePointName : maPointName);
    const size_t nMarkCount(GetMarkCount());
    size_t nMarkPtCnt(0);
    size_t nMarkPtObjCnt(0);
    size_t n1stMarkNum(SAL_MAX_SIZE);

    for(size_t nMarkNum = 0; nMarkNum < nMarkCount; ++nMarkNum)
    {
        const SdrMark* pMark = GetMark(nMarkNum);
        const SdrUShortCont& rPts = bGlue ? pMark->GetMarkedGluePoints() : pMark->GetMarkedPoints();

        if (!rPts.empty())
        {
            if(n1stMarkNum == SAL_MAX_SIZE)
            {
                n1stMarkNum = nMarkNum;
            }

            nMarkPtCnt += rPts.size();
            nMarkPtObjCnt++;
        }

        if(nMarkPtObjCnt > 1 && rNameOk)
        {
            // preliminary decision
            return rName;
        }
    }

    if(rNameOk && 1 == nMarkPtObjCnt)
    {
        // if it's a single selection, cache only text frame
        const SdrObject* pObj = GetMark(0)->GetMarkedSdrObj();
        const SdrTextObj* pTextObj = dynamic_cast<const SdrTextObj*>( pObj );

        if(!pTextObj || !pTextObj->IsTextFrame())
        {
            rNameOk = false;
        }
    }

    if(!nMarkPtObjCnt)
    {
        rName.clear();
        rNameOk = true;
    }
    else if(!rNameOk)
    {
        const SdrMark* pMark = GetMark(n1stMarkNum);
        OUString aNam;

        if(1 == nMarkPtObjCnt)
        {
            if(pMark->GetMarkedSdrObj())
            {
                aNam = pMark->GetMarkedSdrObj()->TakeObjNameSingul();
            }
        }
        else
        {
            if(pMark->GetMarkedSdrObj())
            {
                aNam = pMark->GetMarkedSdrObj()->TakeObjNamePlural();
            }

            bool bEq(true);

            for(size_t i = n1stMarkNum + 1; i < GetMarkCount() && bEq; ++i)
            {
                const SdrMark* pMark2 = GetMark(i);
                const SdrUShortCont& rPts = bGlue ? pMark2->GetMarkedGluePoints() : pMark2->GetMarkedPoints();

                if (!rPts.empty() && pMark2->GetMarkedSdrObj())
                {
                    OUString aStr1(pMark2->GetMarkedSdrObj()->TakeObjNamePlural());
                    bEq = aNam == aStr1;
                }
            }

            if(!bEq)
            {
                aNam = SvxResId(STR_ObjNamePlural);
            }

            aNam = OUString::number( nMarkPtObjCnt ) + " " + aNam;
        }

        OUString aStr1;

        if(1 == nMarkPtCnt)
        {
            aStr1 = SvxResId(bGlue ? STR_ViewMarkedGluePoint : STR_ViewMarkedPoint);
        }
        else
        {
            aStr1 = SvxResId(bGlue ? STR_ViewMarkedGluePoints : STR_ViewMarkedPoints);
            aStr1 = aStr1.replaceFirst("%2", OUString::number( nMarkPtCnt ));
        }

        aStr1 = aStr1.replaceFirst("%1", aNam);
        rName = aStr1;
        rNameOk = true;
    }

    return rName;
}

bool SdrMarkList::TakeBoundRect(SdrPageView const * pPV, tools::Rectangle& rRect) const
{
    bool bFnd(false);
    tools::Rectangle aR;

    for(size_t i = 0; i < GetMarkCount(); ++i)
    {
        SdrMark* pMark = GetMark(i);

        if(!pPV || pMark->GetPageView() == pPV)
        {
            if(pMark->GetMarkedSdrObj())
            {
                aR = pMark->GetMarkedSdrObj()->GetCurrentBoundRect();

                if(bFnd)
                {
                    rRect.Union(aR);
                }
                else
                {
                    rRect = aR;
                    bFnd = true;
                }
            }
        }
    }

    return bFnd;
}

bool SdrMarkList::TakeSnapRect(SdrPageView const * pPV, tools::Rectangle& rRect) const
{
    bool bFnd(false);

    for(size_t i = 0; i < GetMarkCount(); ++i)
    {
        SdrMark* pMark = GetMark(i);

        if(!pPV || pMark->GetPageView() == pPV)
        {
            if(pMark->GetMarkedSdrObj())
            {
                tools::Rectangle aR(pMark->GetMarkedSdrObj()->GetSnapRect());

                if(bFnd)
                {
                    rRect.Union(aR);
                }
                else
                {
                    rRect = aR;
                    bFnd = true;
                }
            }
        }
    }

    return bFnd;
}


namespace sdr
{
    ViewSelection::ViewSelection()
    :   mbEdgesOfMarkedNodesDirty(false)
    {
    }

    void ViewSelection::SetEdgesOfMarkedNodesDirty()
    {
        if(!mbEdgesOfMarkedNodesDirty)
        {
            mbEdgesOfMarkedNodesDirty = true;
            maEdgesOfMarkedNodes.Clear();
            maMarkedEdgesOfMarkedNodes.Clear();
            maAllMarkedObjects.clear();
        }
    }

    const SdrMarkList& ViewSelection::GetEdgesOfMarkedNodes() const
    {
        if(mbEdgesOfMarkedNodesDirty)
        {
            const_cast<ViewSelection*>(this)->ImpForceEdgesOfMarkedNodes();
        }

        return maEdgesOfMarkedNodes;
    }

    const SdrMarkList& ViewSelection::GetMarkedEdgesOfMarkedNodes() const
    {
        if(mbEdgesOfMarkedNodesDirty)
        {
            const_cast<ViewSelection*>(this)->ImpForceEdgesOfMarkedNodes();
        }

        return maMarkedEdgesOfMarkedNodes;
    }

    const std::vector<SdrObject*>& ViewSelection::GetAllMarkedObjects() const
    {
        if(mbEdgesOfMarkedNodesDirty)
            const_cast<ViewSelection*>(this)->ImpForceEdgesOfMarkedNodes();

        return maAllMarkedObjects;
    }

    void ViewSelection::ImplCollectCompleteSelection(SdrObject* pObj)
    {
        if(pObj)
        {
            bool bIsGroup(pObj->IsGroupObject());

            if(bIsGroup && dynamic_cast< const E3dObject* >(pObj) != nullptr && dynamic_cast< const E3dScene* >(pObj) == nullptr)
            {
                bIsGroup = false;
            }

            if(bIsGroup)
            {
                SdrObjList* pList = pObj->GetSubList();

                for(size_t a = 0; a < pList->GetObjCount(); ++a)
                {
                    SdrObject* pObj2 = pList->GetObj(a);
                    ImplCollectCompleteSelection(pObj2);
                }
            }

            maAllMarkedObjects.push_back(pObj);
        }
    }

    void ViewSelection::ImpForceEdgesOfMarkedNodes()
    {
        if(mbEdgesOfMarkedNodesDirty)
        {
            mbEdgesOfMarkedNodesDirty = false;
            maMarkedObjectList.ForceSort();
            maEdgesOfMarkedNodes.Clear();
            maMarkedEdgesOfMarkedNodes.Clear();
            maAllMarkedObjects.clear();

            // GetMarkCount after ForceSort
            const size_t nMarkCount(maMarkedObjectList.GetMarkCount());

            for(size_t a = 0; a < nMarkCount; ++a)
            {
                SdrObject* pCandidate = maMarkedObjectList.GetMark(a)->GetMarkedSdrObj();

                if(pCandidate)
                {
                    // build transitive hull
                    ImplCollectCompleteSelection(pCandidate);

                    if(pCandidate->IsNode())
                    {
                        // travel over broadcaster/listener to access edges connected to the selected object
                        const SfxBroadcaster* pBC = pCandidate->GetBroadcaster();

                        if(pBC)
                        {
                            const size_t nLstCnt(pBC->GetSizeOfVector());

                            for(size_t nl=0; nl < nLstCnt; ++nl)
                            {
                                SfxListener* pLst = pBC->GetListener(nl);
                                SdrEdgeObj* pEdge = dynamic_cast<SdrEdgeObj*>( pLst );

                                if(pEdge && pEdge->IsInserted() && pEdge->getSdrPageFromSdrObject() == pCandidate->getSdrPageFromSdrObject())
                                {
                                    SdrMark aM(pEdge, maMarkedObjectList.GetMark(a)->GetPageView());

                                    if(pEdge->GetConnectedNode(true) == pCandidate)
                                    {
                                        aM.SetCon1(true);
                                    }

                                    if(pEdge->GetConnectedNode(false) == pCandidate)
                                    {
                                        aM.SetCon2(true);
                                    }

                                    if(SAL_MAX_SIZE == maMarkedObjectList.FindObject(pEdge))
                                    {
                                        // check if it itself is selected
                                        maEdgesOfMarkedNodes.InsertEntry(aM);
                                    }
                                    else
                                    {
                                        maMarkedEdgesOfMarkedNodes.InsertEntry(aM);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            maEdgesOfMarkedNodes.ForceSort();
            maMarkedEdgesOfMarkedNodes.ForceSort();
        }
    }
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
