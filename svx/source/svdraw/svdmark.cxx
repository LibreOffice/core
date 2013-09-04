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


////////////////////////////////////////////////////////////////////////////////////////////////////

#include <svx/svdmark.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include "svx/svditer.hxx"
#include <svx/svdpagv.hxx>
#include <svx/svdopath.hxx> // To turn off
#include <svx/svdogrp.hxx>  // the cache at
#include <svx/svdorect.hxx> // GetMarkDescription.
#include "svx/svdstr.hrc"   // names taken from the resource
#include "svx/svdglob.hxx"  // StringCache

////////////////////////////////////////////////////////////////////////////////////////////////////
#include <svx/obj3d.hxx>
#include <svx/scene3d.hxx>
#include <svl/brdcst.hxx>
#include <svx/svdoedge.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

SdrMark::SdrMark(SdrObject* pNewObj, SdrPageView* pNewPageView)
:   mpSelectedSdrObject(pNewObj),
    mpPageView(pNewPageView),
    mpPoints(0L),
    mpLines(0L),
    mpGluePoints(0L),
    mbCon1(sal_False),
    mbCon2(sal_False),
    mnUser(0)
{
    if(mpSelectedSdrObject)
    {
        mpSelectedSdrObject->AddObjectUser( *this );
    }
}

SdrMark::SdrMark(const SdrMark& rMark)
:   ObjectUser(),
    mpSelectedSdrObject(0L),
    mpPageView(0L),
    mpPoints(0L),
    mpLines(0L),
    mpGluePoints(0L),
    mbCon1(sal_False),
    mbCon2(sal_False),
    mnUser(0)
{
    *this = rMark;
}

SdrMark::~SdrMark()
{
    if(mpSelectedSdrObject)
    {
        mpSelectedSdrObject->RemoveObjectUser( *this );
    }

    if(mpPoints)
    {
        delete mpPoints;
    }

    if(mpLines)
    {
        delete mpLines;
    }

    if(mpGluePoints)
    {
        delete mpGluePoints;
    }
}

void SdrMark::ObjectInDestruction(const SdrObject& rObject)
{
    (void) rObject; // avoid warnings
    OSL_ENSURE(mpSelectedSdrObject && mpSelectedSdrObject == &rObject, "SdrMark::ObjectInDestruction: called form object different from hosted one (!)");
    OSL_ENSURE(mpSelectedSdrObject, "SdrMark::ObjectInDestruction: still selected SdrObject is deleted, deselect first (!)");
    mpSelectedSdrObject = 0L;
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

SdrObject* SdrMark::GetMarkedSdrObj() const
{
    return mpSelectedSdrObject;
}

SdrMark& SdrMark::operator=(const SdrMark& rMark)
{
    SetMarkedSdrObj(rMark.mpSelectedSdrObject);
    mpPageView = rMark.mpPageView;
    mbCon1 = rMark.mbCon1;
    mbCon2 = rMark.mbCon2;
    mnUser = rMark.mnUser;

    if(!rMark.mpPoints)
    {
        if(mpPoints)
        {
            delete mpPoints;
            mpPoints = 0L;
        }
    }
    else
    {
        if(!mpPoints)
        {
            mpPoints = new SdrUShortCont(*rMark.mpPoints);
        }
        else
        {
            *mpPoints = *rMark.mpPoints;
        }
    }

    if(!rMark.mpLines)
    {
        if(mpLines)
        {
            delete mpLines;
            mpLines = 0L;
        }
    }
    else
    {
        if(!mpLines)
        {
            mpLines = new SdrUShortCont(*rMark.mpLines);
        }
        else
        {
            *mpLines = *rMark.mpLines;
        }
    }

    if(!rMark.mpGluePoints)
    {
        if(mpGluePoints)
        {
            delete mpGluePoints;
            mpGluePoints = 0L;
        }
    }
    else
    {
        if(!mpGluePoints)
        {
            mpGluePoints = new SdrUShortCont(*rMark.mpGluePoints);
        }
        else
        {
            *mpGluePoints = *rMark.mpGluePoints;
        }
    }

    return *this;
}

sal_Bool SdrMark::operator==(const SdrMark& rMark) const
{
    sal_Bool bRet(mpSelectedSdrObject == rMark.mpSelectedSdrObject && mpPageView == rMark.mpPageView && mbCon1 == rMark.mbCon1 && mbCon2 == rMark.mbCon2 && mnUser == rMark.mnUser);

    if((mpPoints != 0L) != (rMark.mpPoints != 0L))
        bRet = sal_False;

    if((mpLines != 0L) != (rMark.mpLines != 0L))
        bRet = sal_False;

    if((mpGluePoints != 0L) != (rMark.mpGluePoints != 0L))
        bRet = sal_False;

    if(bRet && mpPoints && *mpPoints != *rMark.mpPoints)
        bRet = sal_False;

    if(bRet && mpLines && *mpLines != *rMark.mpLines)
        bRet = sal_False;

    if(bRet && mpGluePoints && *mpGluePoints != *rMark.mpGluePoints)
        bRet = sal_False;

    return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static bool ImpSdrMarkListSorter(SdrMark* const& lhs, SdrMark* const& rhs)
{
    SdrObject* pObj1 = lhs->GetMarkedSdrObj();
    SdrObject* pObj2 = rhs->GetMarkedSdrObj();
    SdrObjList* pOL1 = (pObj1) ? pObj1->GetObjList() : 0L;
    SdrObjList* pOL2 = (pObj2) ? pObj2->GetObjList() : 0L;

    if (pOL1 == pOL2)
    {
        // AF: Note that I reverted a change from sal_uInt32 to sal_uLong (made
        // for 64bit compliance, #i78198#) because internally in SdrObject
        // both nOrdNum and mnNavigationPosition are stored as sal_uInt32.
        sal_uInt32 nObjOrd1((pObj1) ? pObj1->GetNavigationPosition() : 0);
        sal_uInt32 nObjOrd2((pObj2) ? pObj2->GetNavigationPosition() : 0);

        return nObjOrd1 < nObjOrd2;
    }
    else
    {
        return (sal_IntPtr)pOL1 < (sal_IntPtr)pOL2;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkList::ForceSort() const
{
    if(!mbSorted)
    {
        ((SdrMarkList*)this)->ImpForceSort();
    }
}

void SdrMarkList::ImpForceSort()
{
    if(!mbSorted)
    {
        mbSorted = sal_True;
        sal_uLong nAnz = maList.size();

        // remove invalid
        if(nAnz > 0 )
        {
            for(std::vector<SdrMark*>::iterator it = maList.begin(); it != maList.end(); )
            {
                SdrMark* pAkt = *it;
                if(pAkt->GetMarkedSdrObj() == 0)
                {
                    it = maList.erase( it );
                    delete pAkt;
                }
                else
                    ++it;
            }
            nAnz = maList.size();
        }

        if(nAnz > 1)
        {
            std::sort(maList.begin(), maList.end(), ImpSdrMarkListSorter);

            // remove duplicates
            if(maList.size() > 1)
            {
                SdrMark* pAkt = maList.back();
                int i = maList.size() - 2;
                while(i)
                {
                    SdrMark* pCmp = maList[i];
                    if(pAkt->GetMarkedSdrObj() == pCmp->GetMarkedSdrObj() && pAkt->GetMarkedSdrObj())
                    {
                        // Con1/Con2 Merging
                        if(pCmp->IsCon1())
                            pAkt->SetCon1(sal_True);

                        if(pCmp->IsCon2())
                            pAkt->SetCon2(sal_True);

                        // delete pCmp
                        maList.erase(maList.begin() + i);

                        delete pCmp;
                    }
                    else
                    {
                        pAkt = pCmp;
                    }

                    --i;
                }
            }
        }
    }
}

void SdrMarkList::Clear()
{
    for(sal_uLong i(0L); i < GetMarkCount(); i++)
    {
        SdrMark* pMark = GetMark(i);
        delete pMark;
    }

    maList.clear();
    SetNameDirty();
}

void SdrMarkList::operator=(const SdrMarkList& rLst)
{
    Clear();

    for(sal_uLong i(0L); i < rLst.GetMarkCount(); i++)
    {
        SdrMark* pMark = rLst.GetMark(i);
        SdrMark* pNeuMark = new SdrMark(*pMark);
        maList.push_back(pNeuMark);
    }

    maMarkName = rLst.maMarkName;
    mbNameOk = rLst.mbNameOk;
    maPointName = rLst.maPointName;
    mbPointNameOk = rLst.mbPointNameOk;
    maGluePointName = rLst.maGluePointName;
    mbGluePointNameOk = rLst.mbGluePointNameOk;
    mbSorted = rLst.mbSorted;
}

SdrMark* SdrMarkList::GetMark(sal_uLong nNum) const
{
    return (nNum < maList.size()) ? maList[nNum] : NULL;
}

sal_uLong SdrMarkList::FindObject(const SdrObject* pObj) const
{
    // Since relying on OrdNums is not allowed for the selection because objects in the
    // selection may not be inserted in a list if they are e.g. modified ATM, i changed
    // this loop to just look if the object pointer is in the selection.
    //
    // Problem is that GetOrdNum() which is const, internally casts to non-const and
    // hardly sets the OrdNum member of the object (nOrdNum) to 0 (ZERO) if the object
    // is not inserted in a object list.
    // Since this may be by purpose and necessary somewhere else i decided that it is
    // less dangerous to change this method then changing SdrObject::GetOrdNum().
    if(pObj && !maList.empty())
    {
        for(sal_uLong a(0L); a < maList.size(); a++)
        {
            if(maList[a]->GetMarkedSdrObj() == pObj)
            {
                return a;
            }
        }
    }

    return CONTAINER_ENTRY_NOTFOUND;
}

void SdrMarkList::InsertEntry(const SdrMark& rMark, sal_Bool bChkSort)
{
    SetNameDirty();
    sal_uLong nAnz(maList.size());

    if(!bChkSort || !mbSorted || nAnz == 0)
    {
        if(!bChkSort)
            mbSorted = sal_False;

        maList.push_back(new SdrMark(rMark));
    }
    else
    {
        SdrMark* pLast = GetMark(sal_uLong(nAnz - 1));
        const SdrObject* pLastObj = pLast->GetMarkedSdrObj();
        const SdrObject* pNeuObj = rMark.GetMarkedSdrObj();

        if(pLastObj == pNeuObj)
        {
            // This one already exists.
            // Con1/Con2 Merging
            if(rMark.IsCon1())
                pLast->SetCon1(sal_True);

            if(rMark.IsCon2())
                pLast->SetCon2(sal_True);
        }
        else
        {
            SdrMark* pKopie = new SdrMark(rMark);
            maList.push_back(pKopie);

            // now check if the sort is ok
            const SdrObjList* pLastOL = pLastObj!=0L ? pLastObj->GetObjList() : 0L;
            const SdrObjList* pNeuOL = pNeuObj !=0L ? pNeuObj ->GetObjList() : 0L;

            if(pLastOL == pNeuOL)
            {
                const sal_uLong nLastNum(pLastObj!=0L ? pLastObj->GetOrdNum() : 0);
                const sal_uLong nNeuNum(pNeuObj !=0L ? pNeuObj ->GetOrdNum() : 0);

                if(nNeuNum < nLastNum)
                {
                    // at some point, we have to sort
                    mbSorted = sal_False;
                }
            }
            else
            {
                // at some point, we have to sort
                mbSorted = sal_False;
            }
        }
    }

    return;
}

void SdrMarkList::DeleteMark(sal_uLong nNum)
{
    SdrMark* pMark = GetMark(nNum);
    DBG_ASSERT(pMark!=0L,"DeleteMark: MarkEntry not found.");

    if(pMark)
    {
        maList.erase(maList.begin() + nNum);
        delete pMark;
        SetNameDirty();
    }
}

void SdrMarkList::ReplaceMark(const SdrMark& rNewMark, sal_uLong nNum)
{
    SdrMark* pMark = GetMark(nNum);
    DBG_ASSERT(pMark!=0L,"ReplaceMark: MarkEntry not found.");

    if(pMark)
    {
        delete pMark;
        SetNameDirty();
        SdrMark* pKopie = new SdrMark(rNewMark);
        maList[nNum] = pKopie;
        mbSorted = sal_False;
    }
}

void SdrMarkList::Merge(const SdrMarkList& rSrcList, sal_Bool bReverse)
{
    sal_uLong nAnz(rSrcList.maList.size());

    if(rSrcList.mbSorted)
    {
        // merge without forcing a Sort in rSrcList
        bReverse = sal_False;
    }

    if(!bReverse)
    {
        for(sal_uLong i(0L); i < nAnz; i++)
        {
            SdrMark* pM = rSrcList.maList[i];
            InsertEntry(*pM);
        }
    }
    else
    {
        for(sal_uLong i(nAnz); i > 0;)
        {
            i--;
            SdrMark* pM = rSrcList.maList[i];
            InsertEntry(*pM);
        }
    }
}

sal_Bool SdrMarkList::DeletePageView(const SdrPageView& rPV)
{
    sal_Bool bChgd(sal_False);

    for(std::vector<SdrMark*>::iterator it = maList.begin(); it != maList.end(); )
    {
        SdrMark* pMark = *it;

        if(pMark->GetPageView()==&rPV)
        {
            it = maList.erase(it);
            delete pMark;
            SetNameDirty();
            bChgd = sal_True;
        }
        else
            ++it;
    }

    return bChgd;
}

sal_Bool SdrMarkList::InsertPageView(const SdrPageView& rPV)
{
    sal_Bool bChgd(sal_False);
    DeletePageView(rPV); // delete all of them, then append the entire page
    SdrObject* pObj;
    const SdrObjList* pOL = rPV.GetObjList();
    sal_uLong nObjAnz(pOL->GetObjCount());

    for(sal_uLong nO(0L); nO < nObjAnz; nO++)
    {
        pObj = pOL->GetObj(nO);
        sal_Bool bDoIt(rPV.IsObjMarkable(pObj));

        if(bDoIt)
        {
            SdrMark* pM = new SdrMark(pObj, (SdrPageView*)&rPV);
            maList.push_back(pM);
            SetNameDirty();
            bChgd = sal_True;
        }
    }

    return bChgd;
}

const OUString& SdrMarkList::GetMarkDescription() const
{
    sal_uLong nAnz(GetMarkCount());

    if(mbNameOk && 1L == nAnz)
    {
        // if it's a single selection, cache only text frame
        const SdrObject* pObj = GetMark(0)->GetMarkedSdrObj();
        const SdrTextObj* pTextObj = PTR_CAST(SdrTextObj, pObj);

        if(!pTextObj || !pTextObj->IsTextFrame())
        {
            ((SdrMarkList*)(this))->mbNameOk = sal_False;
        }
    }

    if(!mbNameOk)
    {
        SdrMark* pMark = GetMark(0);
        OUString aNam;

        if(!nAnz)
        {
            const_cast<SdrMarkList*>(this)->maMarkName = ImpGetResStr(STR_ObjNameNoObj);
        }
        else if(1L == nAnz)
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

                for(sal_uLong i = 1; i < GetMarkCount() && bEq; i++)
                {
                    SdrMark* pMark2 = GetMark(i);
                    OUString aStr1(pMark2->GetMarkedSdrObj()->TakeObjNamePlural());
                    bEq = aNam == aStr1;
                }

                if(!bEq)
                {
                    aNam = ImpGetResStr(STR_ObjNamePlural);
                }
            }

            aNam = OUString::number( nAnz ) + " " + aNam;
        }

        const_cast<SdrMarkList*>(this)->maMarkName = aNam;
        const_cast<SdrMarkList*>(this)->mbNameOk = sal_True;
    }

    return maMarkName;
}

const OUString& SdrMarkList::GetPointMarkDescription(sal_Bool bGlue) const
{
    sal_Bool& rNameOk = (sal_Bool&)(bGlue ? mbGluePointNameOk : mbPointNameOk);
    OUString& rName = const_cast<OUString&>(bGlue ? maGluePointName : maPointName);
    sal_uLong nMarkAnz(GetMarkCount());
    sal_uLong nMarkPtAnz(0L);
    sal_uLong nMarkPtObjAnz(0L);
    sal_uLong n1stMarkNum(ULONG_MAX);

    for(sal_uLong nMarkNum(0L); nMarkNum < nMarkAnz; nMarkNum++)
    {
        const SdrMark* pMark = GetMark(nMarkNum);
        const SdrUShortCont* pPts = bGlue ? pMark->GetMarkedGluePoints() : pMark->GetMarkedPoints();
        sal_uLong nAnz(pPts ? pPts->size() : 0);

        if(nAnz)
        {
            if(n1stMarkNum == ULONG_MAX)
            {
                n1stMarkNum = nMarkNum;
            }

            nMarkPtAnz += nAnz;
            nMarkPtObjAnz++;
        }

        if(nMarkPtObjAnz > 1 && rNameOk)
        {
            // preliminary decision
            return rName;
        }
    }

    if(rNameOk && 1L == nMarkPtObjAnz)
    {
        // if it's a single selection, cache only text frame
        const SdrObject* pObj = GetMark(0)->GetMarkedSdrObj();
        const SdrTextObj* pTextObj = PTR_CAST(SdrTextObj,pObj);

        if(!pTextObj || !pTextObj->IsTextFrame())
        {
            rNameOk = sal_False;
        }
    }

    if(!nMarkPtObjAnz)
    {
        rName = OUString();
        rNameOk = sal_True;
    }
    else if(!rNameOk)
    {
        const SdrMark* pMark = GetMark(n1stMarkNum);
        OUString aNam;

        if(1L == nMarkPtObjAnz)
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

            for(sal_uLong i(n1stMarkNum + 1L); i < GetMarkCount() && bEq; i++)
            {
                const SdrMark* pMark2 = GetMark(i);
                const SdrUShortCont* pPts = bGlue ? pMark2->GetMarkedGluePoints() : pMark2->GetMarkedPoints();

                if(pPts && !pPts->empty() && pMark2->GetMarkedSdrObj())
                {
                    OUString aStr1(pMark2->GetMarkedSdrObj()->TakeObjNamePlural());
                    bEq = aNam == aStr1;
                }
            }

            if(!bEq)
            {
                aNam = ImpGetResStr(STR_ObjNamePlural);
            }

            aNam = OUString::number( nMarkPtObjAnz ) + " " + aNam;
        }

        OUString aStr1;

        if(1L == nMarkPtAnz)
        {
            aStr1 = (ImpGetResStr(bGlue ? STR_ViewMarkedGluePoint : STR_ViewMarkedPoint));
        }
        else
        {
            aStr1 = (ImpGetResStr(bGlue ? STR_ViewMarkedGluePoints : STR_ViewMarkedPoints));
            aStr1 = aStr1.replaceFirst("%2", OUString::number( nMarkPtAnz ));
        }

        aStr1 = aStr1.replaceFirst("%1", aNam);
        rName = aStr1;
        rNameOk = sal_True;
    }

    return rName;
}

sal_Bool SdrMarkList::TakeBoundRect(SdrPageView* pPV, Rectangle& rRect) const
{
    sal_Bool bFnd(sal_False);
    Rectangle aR;

    for(sal_uLong i(0L); i < GetMarkCount(); i++)
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
                    bFnd = sal_True;
                }
            }
        }
    }

    return bFnd;
}

sal_Bool SdrMarkList::TakeSnapRect(SdrPageView* pPV, Rectangle& rRect) const
{
    sal_Bool bFnd(sal_False);

    for(sal_uLong i(0L); i < GetMarkCount(); i++)
    {
        SdrMark* pMark = GetMark(i);

        if(!pPV || pMark->GetPageView() == pPV)
        {
            if(pMark->GetMarkedSdrObj())
            {
                Rectangle aR(pMark->GetMarkedSdrObj()->GetSnapRect());

                if(bFnd)
                {
                    rRect.Union(aR);
                }
                else
                {
                    rRect = aR;
                    bFnd = sal_True;
                }
            }
        }
    }

    return bFnd;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    ViewSelection::ViewSelection()
    :   mbEdgesOfMarkedNodesDirty(sal_False)
    {
    }

    void ViewSelection::SetEdgesOfMarkedNodesDirty()
    {
        if(!mbEdgesOfMarkedNodesDirty)
        {
            mbEdgesOfMarkedNodesDirty = sal_True;
            maEdgesOfMarkedNodes.Clear();
            maMarkedEdgesOfMarkedNodes.Clear();
            maAllMarkedObjects.clear();
        }
    }

    const SdrMarkList& ViewSelection::GetEdgesOfMarkedNodes() const
    {
        if(mbEdgesOfMarkedNodesDirty)
        {
            ((ViewSelection*)this)->ImpForceEdgesOfMarkedNodes();
        }

        return maEdgesOfMarkedNodes;
    }

    const SdrMarkList& ViewSelection::GetMarkedEdgesOfMarkedNodes() const
    {
        if(mbEdgesOfMarkedNodesDirty)
        {
            ((ViewSelection*)this)->ImpForceEdgesOfMarkedNodes();
        }

        return maMarkedEdgesOfMarkedNodes;
    }

    const std::vector<SdrObject*>& ViewSelection::GetAllMarkedObjects() const
    {
        if(mbEdgesOfMarkedNodesDirty)
            ((ViewSelection*)this)->ImpForceEdgesOfMarkedNodes();

        return maAllMarkedObjects;
    }

    void ViewSelection::ImplCollectCompleteSelection(SdrObject* pObj)
    {
        if(pObj)
        {
            sal_Bool bIsGroup(pObj->IsGroupObject());

            if(bIsGroup && pObj->ISA(E3dObject) && !pObj->ISA(E3dScene))
            {
                bIsGroup = sal_False;
            }

            if(bIsGroup)
            {
                SdrObjList* pList = pObj->GetSubList();

                for(sal_uLong a(0L); a < pList->GetObjCount(); a++)
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
            mbEdgesOfMarkedNodesDirty = sal_False;
            maMarkedObjectList.ForceSort();
            maEdgesOfMarkedNodes.Clear();
            maMarkedEdgesOfMarkedNodes.Clear();
            maAllMarkedObjects.clear();

            // GetMarkCount after ForceSort
            const sal_uLong nMarkAnz(maMarkedObjectList.GetMarkCount());

            for(sal_uLong a(0L); a < nMarkAnz; a++)
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
                            sal_uInt16 nLstAnz(pBC->GetListenerCount());

                            for(sal_uInt16 nl(0); nl < nLstAnz; nl++)
                            {
                                SfxListener* pLst = pBC->GetListener(nl);
                                SdrEdgeObj* pEdge = PTR_CAST(SdrEdgeObj, pLst);

                                if(pEdge && pEdge->IsInserted() && pEdge->GetPage() == pCandidate->GetPage())
                                {
                                    SdrMark aM(pEdge, maMarkedObjectList.GetMark(a)->GetPageView());

                                    if(pEdge->GetConnectedNode(sal_True) == pCandidate)
                                    {
                                        aM.SetCon1(sal_True);
                                    }

                                    if(pEdge->GetConnectedNode(sal_False) == pCandidate)
                                    {
                                        aM.SetCon2(sal_True);
                                    }

                                    if(CONTAINER_ENTRY_NOTFOUND == maMarkedObjectList.FindObject(pEdge))
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
