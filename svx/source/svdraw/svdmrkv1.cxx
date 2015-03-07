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
#include "svdglob.hxx"
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include "svddrgm1.hxx"


// Point Selection


bool SdrMarkView::HasMarkablePoints() const
{
    ForceUndirtyMrkPnt();
    bool bRet=false;
    if (!ImpIsFrameHandles()) {
        const size_t nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=static_cast<size_t>(nFrameHandlesLimit)) {
            for (size_t nMarkNum=0; nMarkNum<nMarkAnz && !bRet; ++nMarkNum) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrObject* pObj=pM->GetMarkedSdrObj();
                bRet=pObj->IsPolyObj();
            }
        }
    }
    return bRet;
}

sal_uIntPtr SdrMarkView::GetMarkablePointCount() const
{
    ForceUndirtyMrkPnt();
    sal_uIntPtr nAnz=0;
    if (!ImpIsFrameHandles()) {
        const size_t nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=static_cast<size_t>(nFrameHandlesLimit)) {
            for (size_t nMarkNum=0; nMarkNum<nMarkAnz; ++nMarkNum) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrObject* pObj=pM->GetMarkedSdrObj();
                if (pObj->IsPolyObj()) {
                    nAnz+=pObj->GetPointCount();
                }
            }
        }
    }
    return nAnz;
}

bool SdrMarkView::HasMarkedPoints() const
{
    ForceUndirtyMrkPnt();
    bool bRet=false;
    if (!ImpIsFrameHandles()) {
        const size_t nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=static_cast<size_t>(nFrameHandlesLimit)) {
            for (size_t nMarkNum=0; nMarkNum<nMarkAnz && !bRet; ++nMarkNum) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrUShortCont* pPts=pM->GetMarkedPoints();
                bRet=pPts!=NULL && !pPts->empty();
            }
        }
    }
    return bRet;
}

sal_uIntPtr SdrMarkView::GetMarkedPointCount() const
{
    ForceUndirtyMrkPnt();
    sal_uIntPtr nAnz=0;
    if (!ImpIsFrameHandles()) {
        size_t nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=static_cast<size_t>(nFrameHandlesLimit)) {
            for (size_t nMarkNum=0; nMarkNum<nMarkAnz; ++nMarkNum) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrUShortCont* pPts=pM->GetMarkedPoints();
                if (pPts!=NULL) nAnz+=pPts->size();
            }
        }
    }
    return nAnz;
}

bool SdrMarkView::IsPointMarkable(const SdrHdl& rHdl) const
{
    return !ImpIsFrameHandles() && !rHdl.IsPlusHdl() && rHdl.GetKind()!=HDL_GLUE && rHdl.GetKind()!=HDL_SMARTTAG && rHdl.GetObj()!=NULL && rHdl.GetObj()->IsPolyObj();
}

bool SdrMarkView::MarkPointHelper(SdrHdl* pHdl, SdrMark* pMark, bool bUnmark)
{
    return ImpMarkPoint( pHdl, pMark, bUnmark );
}

bool SdrMarkView::ImpMarkPoint(SdrHdl* pHdl, SdrMark* pMark, bool bUnmark)
{
    if (pHdl==NULL || pHdl->IsPlusHdl() || pHdl->GetKind()==HDL_GLUE)
        return false;

    if (pHdl->IsSelected() != bUnmark)
        return false;

    SdrObject* pObj=pHdl->GetObj();
    if (pObj==NULL || !pObj->IsPolyObj())
        return false;

    if (pMark==NULL)
    {
        const size_t nMarkNum=TryToFindMarkedObject(pObj);
        if (nMarkNum==SAL_MAX_SIZE)
            return false;
        pMark=GetSdrMarkByIndex(nMarkNum);
    }
    const sal_uInt32 nHdlNum(pHdl->GetObjHdlNum());
    SdrUShortCont* pPts=pMark->ForceMarkedPoints();
    if (!bUnmark)
    {
        pPts->insert((sal_uInt16)nHdlNum);
    }
    else
    {
        SdrUShortCont::const_iterator it = pPts->find( (sal_uInt16)nHdlNum );
        if (it != pPts->end())
        {
            pPts->erase(it);
        }
        else
        {
            return false; // error case!
        }
    }

    pHdl->SetSelected(!bUnmark);
    if (!bPlusHdlAlways)
    {
        if (!bUnmark)
        {
            sal_uInt32 nAnz(pObj->GetPlusHdlCount(*pHdl));
            for (sal_uInt32 i=0; i<nAnz; i++)
            {
                SdrHdl* pPlusHdl=pObj->GetPlusHdl(*pHdl,i);
                if (pPlusHdl!=NULL)
                {
                    pPlusHdl->SetObj(pObj);
                    pPlusHdl->SetPageView(pMark->GetPageView());
                    pPlusHdl->SetPlusHdl(true);
                    maHdlList.AddHdl(pPlusHdl);
                }
            }
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
                    delete pPlusHdl;
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
            pM->ForceMarkedPoints();
            if (ImpMarkPoint(&rHdl,pM,bUnmark)) {
                MarkListHasChanged();
                bRet=true;
            }
        }
    }

    return bRet;
}

bool SdrMarkView::MarkPoints(const Rectangle* pRect, bool bUnmark)
{
    ForceUndirtyMrkPnt();
    bool bChgd=false;
    SortMarkedObjects();
    const SdrObject* pObj0=NULL;
    const SdrPageView* pPV0=NULL;
    SdrMark* pM=NULL;
    maHdlList.Sort();
    const size_t nHdlAnz=maHdlList.GetHdlCount();
    for (size_t nHdlNum=nHdlAnz; nHdlNum>0;) {
        --nHdlNum;
        SdrHdl* pHdl=maHdlList.GetHdl(nHdlNum);
        if (IsPointMarkable(*pHdl) && pHdl->IsSelected()==bUnmark) {
            const SdrObject* pObj=pHdl->GetObj();
            const SdrPageView* pPV=pHdl->GetPageView();
            if (pObj!=pObj0 || pPV!=pPV0 || pM==NULL) { // This section is for optimization,
                const size_t nMarkNum=TryToFindMarkedObject(pObj);  // so ImpMarkPoint() doesn't always
                if (nMarkNum!=SAL_MAX_SIZE) { // have to search the object in the MarkList.
                    pM=GetSdrMarkByIndex(nMarkNum);
                    pObj0=pObj;
                    pPV0=pPV;
                    pM->ForceMarkedPoints();
                } else {
#ifdef DBG_UTIL
                    if (pObj->IsInserted()) {
                        OSL_FAIL("SdrMarkView::MarkPoints(const Rectangle* pRect): Selected object not found.");
                    }
#endif
                    pM=NULL;
                }
            }
            Point aPos(pHdl->GetPos());
            if (pM!=NULL && (pRect==NULL || pRect->IsInside(aPos))) {
                if (ImpMarkPoint(pHdl,pM,bUnmark)) bChgd=true;
            }
        }
    }
    if (bChgd) {
        MarkListHasChanged();
    }

    return bChgd;
}

bool SdrMarkView::MarkNextPoint(const Point& /*rPnt*/, bool /*bPrev*/)
{
    ForceUndirtyMrkPnt();
    SortMarkedObjects();
    return false;
}

const Rectangle& SdrMarkView::GetMarkedPointsRect() const
{
    ForceUndirtyMrkPnt();
    if (bMarkedPointsRectsDirty) ImpSetPointsRects();
    return aMarkedPointsRect;
}

void SdrMarkView::SetPlusHandlesAlwaysVisible(bool bOn)
{ // TODO: Optimize HandlePaint!
    ForceUndirtyMrkPnt();
    if (bOn!=bPlusHdlAlways) {
        bPlusHdlAlways=bOn;
        SetMarkHandles();
        MarkListHasChanged();
    }
}


// ImpSetPointsRects() is for PolyPoints and GluePoints!


void SdrMarkView::ImpSetPointsRects() const
{
    Rectangle aPnts;
    Rectangle aGlue;
    const size_t nHdlAnz=maHdlList.GetHdlCount();
    for (size_t nHdlNum=0; nHdlNum<nHdlAnz; ++nHdlNum) {
        const SdrHdl* pHdl=maHdlList.GetHdl(nHdlNum);
        SdrHdlKind eKind=pHdl->GetKind();
        if ((eKind==HDL_POLY && pHdl->IsSelected()) || eKind==HDL_GLUE) {
            Point aPt(pHdl->GetPos());
            Rectangle& rR=eKind==HDL_GLUE ? aGlue : aPnts;
            if (rR.IsEmpty()) {
                rR=Rectangle(aPt,aPt);
            } else {
                if (aPt.X()<rR.Left  ()) rR.Left  ()=aPt.X();
                if (aPt.X()>rR.Right ()) rR.Right ()=aPt.X();
                if (aPt.Y()<rR.Top   ()) rR.Top   ()=aPt.Y();
                if (aPt.Y()>rR.Bottom()) rR.Bottom()=aPt.Y();
            }
        }
    }
    ((SdrMarkView*)this)->aMarkedPointsRect=aPnts;
    ((SdrMarkView*)this)->aMarkedGluePointsRect=aGlue;
    ((SdrMarkView*)this)->bMarkedPointsRectsDirty=false;
}


// UndirtyMrkPnt() is for PolyPoints and GluePoints!


void SdrMarkView::UndirtyMrkPnt() const
{
    bool bChg=false;
    const size_t nMarkAnz=GetMarkedObjectCount();
    for (size_t nMarkNum=0; nMarkNum<nMarkAnz; ++nMarkNum) {
        SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrObject* pObj=pM->GetMarkedSdrObj();
        // PolyPoints
        SdrUShortCont* pPts=pM->GetMarkedPoints();
        if (pPts!=NULL) {
            if (pObj->IsPolyObj()) {
                // Remove invalid selected points, that is, all
                // entries above the number of points in the object.
                sal_uInt32 nMax(pObj->GetPointCount());

                SdrUShortCont::const_iterator it = pPts->lower_bound(nMax);
                if( it != pPts->end() )
                {
                    pPts->erase(it, pPts->end() );
                    bChg = true;
                }
            }
            else
            {
                OSL_FAIL("SdrMarkView::UndirtyMrkPnt(): Selected points on an object that is not a PolyObj!");
                if(pPts && !pPts->empty())
                {
                    pPts->clear();
                    bChg = true;
                }
            }
        }

        // GluePoints
        pPts=pM->GetMarkedGluePoints();
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
        if (pPts!=NULL) {
            if (pGPL!=NULL) {
                // Remove invalid selected glue points, that is, all entries
                // (IDs) that aren't contained in the GluePointList of the
                // object
                for(SdrUShortCont::const_iterator it = pPts->begin(); it != pPts->end(); )
                {
                    sal_uInt16 nId=*it;
                    if (pGPL->FindGluePoint(nId)==SDRGLUEPOINT_NOTFOUND) {
                        it = pPts->erase(it);
                        bChg=true;
                    }
                    else
                        ++it;
                }
            } else {
                if (pPts!=NULL && !pPts->empty()) {
                    pPts->clear(); // object doesn't have any glue points (any more)
                    bChg=true;
                }
            }
        }
    }
    if (bChg) ((SdrMarkView*)this)->bMarkedPointsRectsDirty=true;
    ((SdrMarkView*)this)->bMrkPntDirty=false;
}



bool SdrMarkView::HasMarkableGluePoints() const
{
    bool bRet=false;
    if (IsGluePointEditMode()) {
        ForceUndirtyMrkPnt();
        const size_t nMarkAnz=GetMarkedObjectCount();
        for (size_t nMarkNum=0; nMarkNum<nMarkAnz && !bRet; ++nMarkNum) {
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
    const size_t nMarkAnz=GetMarkedObjectCount();
    for (size_t nMarkNum=0; nMarkNum<nMarkAnz && !bRet; ++nMarkNum) {
        const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        bRet=pPts!=NULL && !pPts->empty();
    }
    return bRet;
}

bool SdrMarkView::MarkGluePoints(const Rectangle* pRect, bool bUnmark)
{
    if (!IsGluePointEditMode() && !bUnmark) return false;
    ForceUndirtyMrkPnt();
    bool bChgd=false;
    SortMarkedObjects();
    const size_t nMarkAnz=GetMarkedObjectCount();
    for (size_t nMarkNum=0; nMarkNum<nMarkAnz; ++nMarkNum) {
        SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
        SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        if (bUnmark && pRect==NULL) { // UnmarkAll
            if (pPts!=NULL && !pPts->empty()) {
                pPts->clear();
                bChgd=true;
            }
        } else {
            if (pGPL!=NULL && (pPts!=NULL || !bUnmark)) {
                sal_uInt16 nGPAnz=pGPL->GetCount();
                for (sal_uInt16 nGPNum=0; nGPNum<nGPAnz; nGPNum++) {
                    const SdrGluePoint& rGP=(*pGPL)[nGPNum];

                    // #i38892#
                    if(rGP.IsUserDefined())
                    {
                        Point aPos(rGP.GetAbsolutePos(*pObj));
                        if (pRect==NULL || pRect->IsInside(aPos)) {
                            if (pPts==NULL)
                                pPts=pM->ForceMarkedGluePoints();
                            bool bContains = pPts->find( rGP.GetId() ) != pPts->end();
                            if (!bUnmark && !bContains) {
                                bChgd=true;
                                pPts->insert(rGP.GetId());
                            }
                            if (bUnmark && bContains) {
                                bChgd=true;
                                pPts->erase(rGP.GetId());
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

bool SdrMarkView::PickGluePoint(const Point& rPnt, SdrObject*& rpObj, sal_uInt16& rnId, SdrPageView*& rpPV, sal_uIntPtr nOptions) const
{
    SdrObject* pObj0=rpObj;
    sal_uInt16 nId0=rnId;
    rpObj=NULL; rpPV=NULL; rnId=0;
    if (!IsGluePointEditMode()) return false;
    bool bBack=(nOptions & SDRSEARCH_BACKWARD) !=0;
    bool bNext=(nOptions & SDRSEARCH_NEXT) !=0;
    OutputDevice* pOut=(OutputDevice*)pActualOutDev;
    if (pOut==NULL) pOut=GetFirstOutputDevice();
    if (pOut==NULL) return false;
    SortMarkedObjects();
    const size_t nMarkAnz=GetMarkedObjectCount();
    size_t nMarkNum=bBack ? 0 : nMarkAnz;
    if (bNext) {
        nMarkNum=((SdrMarkView*)this)->TryToFindMarkedObject(pObj0);
        if (nMarkNum==SAL_MAX_SIZE) return false;
        if (!bBack) nMarkNum++;
    }
    while (bBack ? nMarkNum<nMarkAnz : nMarkNum>0) {
        if (!bBack) nMarkNum--;
        const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrPageView* pPV=pM->GetPageView();
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
        if (pGPL!=NULL) {
            sal_uInt16 nNum=pGPL->HitTest(rPnt,*pOut,pObj,bBack,bNext,nId0);
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
        bNext=false; // HitNextGluePoint only for the first Obj
        if (bBack) nMarkNum++;
    }
    return false;
}

bool SdrMarkView::MarkGluePoint(const SdrObject* pObj, sal_uInt16 nId, const SdrPageView* /*pPV*/, bool bUnmark)
{
    if (!IsGluePointEditMode()) return false;
    ForceUndirtyMrkPnt();
    bool bChgd=false;
    if (pObj!=NULL) {
        const size_t nMarkPos=TryToFindMarkedObject(pObj);
        if (nMarkPos!=SAL_MAX_SIZE) {
            SdrMark* pM=GetSdrMarkByIndex(nMarkPos);
            SdrUShortCont* pPts=bUnmark ? pM->GetMarkedGluePoints() : pM->ForceMarkedGluePoints();
            if (pPts!=NULL) {
                bool bContains = pPts->find( nId ) != pPts->end();
                if (!bUnmark && !bContains) {
                    bChgd=true;
                    pPts->insert(nId);
                }
                if (bUnmark && bContains) {
                    bChgd=true;
                    pPts->erase(nId);
                }
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
    const size_t nPos=((SdrMarkView*)this)->TryToFindMarkedObject(pObj); // casting to NonConst
    if (nPos!=SAL_MAX_SIZE) {
        const SdrMark* pM=GetSdrMarkByIndex(nPos);
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        if (pPts!=NULL) {
            bRet = pPts->find( nId ) != pPts->end();
        }
    }
    return bRet;
}

SdrHdl* SdrMarkView::GetGluePointHdl(const SdrObject* pObj, sal_uInt16 nId) const
{
    ForceUndirtyMrkPnt();
    const size_t nHdlAnz=maHdlList.GetHdlCount();
    for (size_t nHdlNum=0; nHdlNum<nHdlAnz; ++nHdlNum) {
        SdrHdl* pHdl=maHdlList.GetHdl(nHdlNum);
        if (pHdl->GetObj()==pObj &&
            pHdl->GetKind()==HDL_GLUE &&
            pHdl->GetObjHdlNum()==nId ) return pHdl;
    }
    return NULL;
}

bool SdrMarkView::MarkNextGluePoint(const Point& /*rPnt*/, bool /*bPrev*/)
{
    ForceUndirtyMrkPnt();
    SortMarkedObjects();
    return false;
}

const Rectangle& SdrMarkView::GetMarkedGluePointsRect() const
{
    ForceUndirtyMrkPnt();
    if (bMarkedPointsRectsDirty) ImpSetPointsRects();
    return aMarkedGluePointsRect;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
