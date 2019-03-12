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


#include <svx/svdmrkv.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include "svddrgm1.hxx"


// Point Selection


bool SdrMarkView::HasMarkablePoints() const
{
    ForceUndirtyMrkPnt();
    bool bRet=false;
    if (!ImpIsFrameHandles()) {
        const size_t nMarkCount=GetMarkedObjectCount();
        if (nMarkCount<=static_cast<size_t>(mnFrameHandlesLimit)) {
            for (size_t nMarkNum=0; nMarkNum<nMarkCount && !bRet; ++nMarkNum) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrObject* pObj=pM->GetMarkedSdrObj();
                bRet=pObj->IsPolyObj();
            }
        }
    }
    return bRet;
}

sal_Int32 SdrMarkView::GetMarkablePointCount() const
{
    ForceUndirtyMrkPnt();
    sal_Int32 nCount=0;
    if (!ImpIsFrameHandles()) {
        const size_t nMarkCount=GetMarkedObjectCount();
        if (nMarkCount<=static_cast<size_t>(mnFrameHandlesLimit)) {
            for (size_t nMarkNum=0; nMarkNum<nMarkCount; ++nMarkNum) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrObject* pObj=pM->GetMarkedSdrObj();
                if (pObj->IsPolyObj()) {
                    nCount+=pObj->GetPointCount();
                }
            }
        }
    }
    return nCount;
}

bool SdrMarkView::HasMarkedPoints() const
{
    ForceUndirtyMrkPnt();
    bool bRet=false;
    if (!ImpIsFrameHandles()) {
        const size_t nMarkCount=GetMarkedObjectCount();
        if (nMarkCount<=static_cast<size_t>(mnFrameHandlesLimit)) {
            for (size_t nMarkNum=0; nMarkNum<nMarkCount && !bRet; ++nMarkNum) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrUShortCont& rPts = pM->GetMarkedPoints();
                bRet = !rPts.empty();
            }
        }
    }
    return bRet;
}

bool SdrMarkView::IsPointMarkable(const SdrHdl& rHdl) const
{
    return !ImpIsFrameHandles() && !rHdl.IsPlusHdl() && rHdl.GetKind()!=SdrHdlKind::Glue && rHdl.GetKind()!=SdrHdlKind::SmartTag && rHdl.GetObj()!=nullptr && rHdl.GetObj()->IsPolyObj();
}

bool SdrMarkView::MarkPointHelper(SdrHdl* pHdl, SdrMark* pMark, bool bUnmark)
{
    return ImpMarkPoint( pHdl, pMark, bUnmark );
}

bool SdrMarkView::ImpMarkPoint(SdrHdl* pHdl, SdrMark* pMark, bool bUnmark)
{
    if (pHdl==nullptr || pHdl->IsPlusHdl() || pHdl->GetKind()==SdrHdlKind::Glue)
        return false;

    if (pHdl->IsSelected() != bUnmark)
        return false;

    SdrObject* pObj=pHdl->GetObj();
    if (pObj==nullptr || !pObj->IsPolyObj())
        return false;

    if (pMark==nullptr)
    {
        const size_t nMarkNum=TryToFindMarkedObject(pObj);
        if (nMarkNum==SAL_MAX_SIZE)
            return false;
        pMark=GetSdrMarkByIndex(nMarkNum);
    }
    const sal_uInt32 nHdlNum(pHdl->GetObjHdlNum());
    SdrUShortCont& rPts=pMark->GetMarkedPoints();
    if (!bUnmark)
    {
        rPts.insert(static_cast<sal_uInt16>(nHdlNum));
    }
    else
    {
        SdrUShortCont::const_iterator it = rPts.find( static_cast<sal_uInt16>(nHdlNum) );
        if (it != rPts.end())
        {
            rPts.erase(it);
        }
        else
        {
            return false; // error case!
        }
    }

    pHdl->SetSelected(!bUnmark);
    if (!mbPlusHdlAlways)
    {
        if (!bUnmark)
        {
            SdrHdlList plusList(nullptr);
            pObj->AddToPlusHdlList(plusList, *pHdl);
            sal_uInt32 nCount(plusList.GetHdlCount());
            for (sal_uInt32 i=0; i<nCount; i++)
            {
                SdrHdl* pPlusHdl=plusList.GetHdl(i);
                pPlusHdl->SetObj(pObj);
                pPlusHdl->SetPageView(pMark->GetPageView());
                pPlusHdl->SetPlusHdl(true);
            }
            plusList.MoveTo(maHdlList);
        }
        else
        {
            for (size_t i = maHdlList.GetHdlCount(); i>0;)
            {
                --i;
                SdrHdl* pPlusHdl=maHdlList.GetHdl(i);
                if (pPlusHdl->IsPlusHdl() && pPlusHdl->GetSourceHdlNum()==nHdlNum)
                {
                    maHdlList.RemoveHdl(i);
                }
            }
        }
    }

    maHdlList.Sort();

    return true;
}


bool SdrMarkView::MarkPoint(SdrHdl& rHdl, bool bUnmark)
{
    ForceUndirtyMrkPnt();
    bool bRet=false;
    const SdrObject* pObj=rHdl.GetObj();
    if (IsPointMarkable(rHdl) && rHdl.IsSelected()==bUnmark) {
        const size_t nMarkNum=TryToFindMarkedObject(pObj);
        if (nMarkNum!=SAL_MAX_SIZE) {
            SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            if (ImpMarkPoint(&rHdl,pM,bUnmark)) {
                MarkListHasChanged();
                bRet=true;
            }
        }
    }

    return bRet;
}

bool SdrMarkView::MarkPoints(const tools::Rectangle* pRect, bool bUnmark)
{
    ForceUndirtyMrkPnt();
    bool bChgd=false;
    SortMarkedObjects();
    const SdrObject* pObj0=nullptr;
    const SdrPageView* pPV0=nullptr;
    SdrMark* pM=nullptr;
    maHdlList.Sort();
    const size_t nHdlCnt=maHdlList.GetHdlCount();
    for (size_t nHdlNum=nHdlCnt; nHdlNum>0;) {
        --nHdlNum;
        SdrHdl* pHdl=maHdlList.GetHdl(nHdlNum);
        if (IsPointMarkable(*pHdl) && pHdl->IsSelected()==bUnmark) {
            const SdrObject* pObj=pHdl->GetObj();
            const SdrPageView* pPV=pHdl->GetPageView();
            if (pObj!=pObj0 || pPV!=pPV0 || pM==nullptr) { // This section is for optimization,
                const size_t nMarkNum=TryToFindMarkedObject(pObj);  // so ImpMarkPoint() doesn't always
                if (nMarkNum!=SAL_MAX_SIZE) { // have to search the object in the MarkList.
                    pM=GetSdrMarkByIndex(nMarkNum);
                    pObj0=pObj;
                    pPV0=pPV;
                } else {
#ifdef DBG_UTIL
                    if (pObj->IsInserted()) {
                        OSL_FAIL("SdrMarkView::MarkPoints(const Rectangle* pRect): Selected object not found.");
                    }
#endif
                    pM=nullptr;
                }
            }
            Point aPos(pHdl->GetPos());
            if (pM!=nullptr && (pRect==nullptr || pRect->IsInside(aPos))) {
                if (ImpMarkPoint(pHdl,pM,bUnmark)) bChgd=true;
            }
        }
    }
    if (bChgd) {
        MarkListHasChanged();
    }

    return bChgd;
}

void SdrMarkView::MarkNextPoint()
{
    ForceUndirtyMrkPnt();
    SortMarkedObjects();
}

const tools::Rectangle& SdrMarkView::GetMarkedPointsRect() const
{
    ForceUndirtyMrkPnt();
    if (mbMarkedPointsRectsDirty) ImpSetPointsRects();
    return maMarkedPointsRect;
}

void SdrMarkView::SetPlusHandlesAlwaysVisible(bool bOn)
{ // TODO: Optimize HandlePaint!
    ForceUndirtyMrkPnt();
    if (bOn!=mbPlusHdlAlways) {
        mbPlusHdlAlways=bOn;
        SetMarkHandles(nullptr);
        MarkListHasChanged();
    }
}


// ImpSetPointsRects() is for PolyPoints and GluePoints!


void SdrMarkView::ImpSetPointsRects() const
{
    tools::Rectangle aPnts;
    tools::Rectangle aGlue;
    const size_t nHdlCnt=maHdlList.GetHdlCount();
    for (size_t nHdlNum=0; nHdlNum<nHdlCnt; ++nHdlNum) {
        const SdrHdl* pHdl=maHdlList.GetHdl(nHdlNum);
        SdrHdlKind eKind=pHdl->GetKind();
        if ((eKind==SdrHdlKind::Poly && pHdl->IsSelected()) || eKind==SdrHdlKind::Glue) {
            Point aPt(pHdl->GetPos());
            tools::Rectangle& rR=eKind==SdrHdlKind::Glue ? aGlue : aPnts;
            if (rR.IsEmpty()) {
                rR=tools::Rectangle(aPt,aPt);
            } else {
                if (aPt.X()<rR.Left  ()) rR.SetLeft(aPt.X() );
                if (aPt.X()>rR.Right ()) rR.SetRight(aPt.X() );
                if (aPt.Y()<rR.Top   ()) rR.SetTop(aPt.Y() );
                if (aPt.Y()>rR.Bottom()) rR.SetBottom(aPt.Y() );
            }
        }
    }
    const_cast<SdrMarkView*>(this)->maMarkedPointsRect=aPnts;
    const_cast<SdrMarkView*>(this)->maMarkedGluePointsRect=aGlue;
    const_cast<SdrMarkView*>(this)->mbMarkedPointsRectsDirty=false;
}


// UndirtyMrkPnt() is for PolyPoints and GluePoints!


void SdrMarkView::UndirtyMrkPnt() const
{
    bool bChg=false;
    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nMarkNum=0; nMarkNum<nMarkCount; ++nMarkNum) {
        SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrObject* pObj=pM->GetMarkedSdrObj();
        // PolyPoints
        {
            SdrUShortCont& rPts = pM->GetMarkedPoints();
            if (pObj->IsPolyObj()) {
                // Remove invalid selected points, that is, all
                // entries above the number of points in the object.
                sal_uInt32 nMax(pObj->GetPointCount());

                SdrUShortCont::const_iterator it = rPts.lower_bound(nMax);
                if( it != rPts.end() )
                {
                    rPts.erase(it, rPts.end());
                    bChg = true;
                }
            }
            else
            {
                OSL_FAIL("SdrMarkView::UndirtyMrkPnt(): Selected points on an object that is not a PolyObj!");
                if (!rPts.empty())
                {
                    rPts.clear();
                    bChg = true;
                }
            }
        }

        // GluePoints
        {
            SdrUShortCont& rPts = pM->GetMarkedGluePoints();
            const SdrGluePointList* pGPL=pObj->GetGluePointList();
            if (pGPL!=nullptr) {
                // Remove invalid selected glue points, that is, all entries
                // (IDs) that aren't contained in the GluePointList of the
                // object
                for(SdrUShortCont::const_iterator it = rPts.begin(); it != rPts.end(); )
                {
                    sal_uInt16 nId=*it;
                    if (pGPL->FindGluePoint(nId)==SDRGLUEPOINT_NOTFOUND) {
                        it = rPts.erase(it);
                        bChg=true;
                    }
                    else
                        ++it;
                }
            } else {
                if (!rPts.empty()) {
                    rPts.clear(); // object doesn't have any glue points (any more)
                    bChg=true;
                }
            }
        }
    }
    if (bChg) const_cast<SdrMarkView*>(this)->mbMarkedPointsRectsDirty=true;
    const_cast<SdrMarkView*>(this)->mbMrkPntDirty=false;
}


bool SdrMarkView::HasMarkableGluePoints() const
{
    bool bRet=false;
    if (IsGluePointEditMode()) {
        ForceUndirtyMrkPnt();
        const size_t nMarkCount=GetMarkedObjectCount();
        for (size_t nMarkNum=0; nMarkNum<nMarkCount && !bRet; ++nMarkNum) {
            const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            const SdrObject* pObj=pM->GetMarkedSdrObj();
            const SdrGluePointList* pGPL=pObj->GetGluePointList();

            // #i38892#
            if(pGPL && pGPL->GetCount())
            {
                for(sal_uInt16 a(0); !bRet && a < pGPL->GetCount(); a++)
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

bool SdrMarkView::HasMarkedGluePoints() const
{
    ForceUndirtyMrkPnt();
    bool bRet=false;
    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nMarkNum=0; nMarkNum<nMarkCount && !bRet; ++nMarkNum) {
        const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrUShortCont& rPts = pM->GetMarkedGluePoints();
        bRet = !rPts.empty();
    }
    return bRet;
}

bool SdrMarkView::MarkGluePoints(const tools::Rectangle* pRect, bool bUnmark)
{
    if (!IsGluePointEditMode() && !bUnmark) return false;
    ForceUndirtyMrkPnt();
    bool bChgd=false;
    SortMarkedObjects();
    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nMarkNum=0; nMarkNum<nMarkCount; ++nMarkNum) {
        SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
        SdrUShortCont& rPts = pM->GetMarkedGluePoints();
        if (bUnmark && pRect==nullptr) { // UnmarkAll
            if (!rPts.empty()) {
                rPts.clear();
                bChgd=true;
            }
        } else {
            if (pGPL!=nullptr) {
                sal_uInt16 nGluePointCnt=pGPL->GetCount();
                for (sal_uInt16 nGPNum=0; nGPNum<nGluePointCnt; nGPNum++) {
                    const SdrGluePoint& rGP=(*pGPL)[nGPNum];

                    // #i38892#
                    if(rGP.IsUserDefined())
                    {
                        Point aPos(rGP.GetAbsolutePos(*pObj));
                        if (pRect==nullptr || pRect->IsInside(aPos)) {
                            bool bContains = rPts.find( rGP.GetId() ) != rPts.end();
                            if (!bUnmark && !bContains) {
                                bChgd=true;
                                rPts.insert(rGP.GetId());
                            }
                            if (bUnmark && bContains) {
                                bChgd=true;
                                rPts.erase(rGP.GetId());
                            }
                        }
                    }
                }
            }
        }
    }
    if (bChgd) {
        AdjustMarkHdl();
        MarkListHasChanged();
    }
    return bChgd;
}

bool SdrMarkView::PickGluePoint(const Point& rPnt, SdrObject*& rpObj, sal_uInt16& rnId, SdrPageView*& rpPV) const
{
    rpObj=nullptr; rpPV=nullptr; rnId=0;
    if (!IsGluePointEditMode()) return false;
    OutputDevice* pOut=mpActualOutDev.get();
    if (pOut==nullptr) pOut=GetFirstOutputDevice();
    if (pOut==nullptr) return false;
    SortMarkedObjects();
    const size_t nMarkCount=GetMarkedObjectCount();
    size_t nMarkNum=nMarkCount;
    while (nMarkNum>0) {
        nMarkNum--;
        const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrPageView* pPV=pM->GetPageView();
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
        if (pGPL!=nullptr) {
            sal_uInt16 nNum=pGPL->HitTest(rPnt,*pOut,pObj);
            if (nNum!=SDRGLUEPOINT_NOTFOUND)
            {
                // #i38892#
                const SdrGluePoint& rCandidate = (*pGPL)[nNum];

                if(rCandidate.IsUserDefined())
                {
                    rpObj=pObj;
                    rnId=(*pGPL)[nNum].GetId();
                    rpPV=pPV;
                    return true;
                }
            }
        }
    }
    return false;
}

bool SdrMarkView::MarkGluePoint(const SdrObject* pObj, sal_uInt16 nId, bool bUnmark)
{
    if (!IsGluePointEditMode()) return false;
    ForceUndirtyMrkPnt();
    bool bChgd=false;
    if (pObj!=nullptr) {
        const size_t nMarkPos=TryToFindMarkedObject(pObj);
        if (nMarkPos!=SAL_MAX_SIZE) {
            SdrMark* pM=GetSdrMarkByIndex(nMarkPos);
            SdrUShortCont& rPts = pM->GetMarkedGluePoints();
            bool bContains = rPts.find( nId ) != rPts.end();
            if (!bUnmark && !bContains) {
                bChgd=true;
                rPts.insert(nId);
            }
            if (bUnmark && bContains) {
                bChgd=true;
                rPts.erase(nId);
            }
        } else {
            // TODO: implement implicit selection of objects
        }
    }
    if (bChgd) {
        AdjustMarkHdl();
        MarkListHasChanged();
    }
    return bChgd;
}

bool SdrMarkView::IsGluePointMarked(const SdrObject* pObj, sal_uInt16 nId) const
{
    ForceUndirtyMrkPnt();
    bool bRet=false;
    const size_t nPos=TryToFindMarkedObject(pObj); // casting to NonConst
    if (nPos!=SAL_MAX_SIZE) {
        const SdrMark* pM=GetSdrMarkByIndex(nPos);
        const SdrUShortCont& rPts = pM->GetMarkedGluePoints();
        bRet = rPts.find( nId ) != rPts.end();
    }
    return bRet;
}

SdrHdl* SdrMarkView::GetGluePointHdl(const SdrObject* pObj, sal_uInt16 nId) const
{
    ForceUndirtyMrkPnt();
    const size_t nHdlCnt=maHdlList.GetHdlCount();
    for (size_t nHdlNum=0; nHdlNum<nHdlCnt; ++nHdlNum) {
        SdrHdl* pHdl=maHdlList.GetHdl(nHdlNum);
        if (pHdl->GetObj()==pObj &&
            pHdl->GetKind()==SdrHdlKind::Glue &&
            pHdl->GetObjHdlNum()==nId ) return pHdl;
    }
    return nullptr;
}

void SdrMarkView::MarkNextGluePoint()
{
    ForceUndirtyMrkPnt();
    SortMarkedObjects();
}

const tools::Rectangle& SdrMarkView::GetMarkedGluePointsRect() const
{
    ForceUndirtyMrkPnt();
    if (mbMarkedPointsRectsDirty) ImpSetPointsRects();
    return maMarkedGluePointsRect;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
