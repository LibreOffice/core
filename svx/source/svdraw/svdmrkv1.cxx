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

#include <svx/svdmrkv.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoedge.hxx>
#include "svdglob.hxx"
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include "svddrgm1.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@  @@  @@ @@  @@ @@  @@ @@@@@@ @@   @@  @@@@  @@@@@  @@  @@ @@ @@@@@ @@@@@  @@  @@ @@  @@  @@@@
//  @@  @@ @@  @@ @@@ @@ @@  @@   @@   @@@ @@@ @@  @@ @@  @@ @@  @@ @@ @@    @@  @@ @@  @@ @@@ @@ @@  @@
//  @@  @@ @@  @@ @@@@@@ @@ @@    @@   @@@@@@@ @@  @@ @@  @@ @@ @@  @@ @@    @@  @@ @@  @@ @@@@@@ @@
//  @@@@@  @@  @@ @@@@@@ @@@@     @@   @@@@@@@ @@@@@@ @@@@@  @@@@   @@ @@@@  @@@@@  @@  @@ @@@@@@ @@ @@@
//  @@     @@  @@ @@ @@@ @@ @@    @@   @@ @ @@ @@  @@ @@  @@ @@ @@  @@ @@    @@  @@ @@  @@ @@ @@@ @@  @@
//  @@     @@  @@ @@  @@ @@  @@   @@   @@   @@ @@  @@ @@  @@ @@  @@ @@ @@    @@  @@ @@  @@ @@  @@ @@  @@
//  @@      @@@@  @@  @@ @@  @@   @@   @@   @@ @@  @@ @@  @@ @@  @@ @@ @@@@@ @@  @@  @@@@  @@  @@  @@@@@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrMarkView::HasMarkablePoints() const
{
    ForceUndirtyMrkPnt();
    bool bRet=false;
    if (!ImpIsFrameHandles()) {
        ULONG nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrObject* pObj=pM->GetMarkedSdrObj();
                bRet=pObj->IsPolyObj();
            }
        }
    }
    return bRet;
}

ULONG SdrMarkView::GetMarkablePointCount() const
{
    ForceUndirtyMrkPnt();
    ULONG nAnz=0;
    if (!ImpIsFrameHandles()) {
        ULONG nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
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

BOOL SdrMarkView::HasMarkedPoints() const
{
    ForceUndirtyMrkPnt();
    BOOL bRet=FALSE;
    if (!ImpIsFrameHandles()) {
        ULONG nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrUShortCont* pPts=pM->GetMarkedPoints();
                bRet=pPts!=NULL && pPts->GetCount()!=0;
            }
        }
    }
    return bRet;
}

ULONG SdrMarkView::GetMarkedPointCount() const
{
    ForceUndirtyMrkPnt();
    ULONG nAnz=0;
    if (!ImpIsFrameHandles()) {
        ULONG nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrUShortCont* pPts=pM->GetMarkedPoints();
                if (pPts!=NULL) nAnz+=pPts->GetCount();
            }
        }
    }
    return nAnz;
}

BOOL SdrMarkView::IsPointMarkable(const SdrHdl& rHdl) const
{
    return !ImpIsFrameHandles() && &rHdl!=NULL && !rHdl.IsPlusHdl() && rHdl.GetKind()!=HDL_GLUE && rHdl.GetKind()!=HDL_SMARTTAG && rHdl.GetObj()!=NULL && rHdl.GetObj()->IsPolyObj();
}

BOOL SdrMarkView::MarkPointHelper(SdrHdl* pHdl, SdrMark* pMark, BOOL bUnmark)
{
    return ImpMarkPoint( pHdl, pMark, bUnmark );
}

BOOL SdrMarkView::ImpMarkPoint(SdrHdl* pHdl, SdrMark* pMark, BOOL bUnmark)
{
    if (pHdl==NULL || pHdl->IsPlusHdl() || pHdl->GetKind()==HDL_GLUE)
        return FALSE;

    if (pHdl->IsSelected() != bUnmark)
        return FALSE;

    SdrObject* pObj=pHdl->GetObj();
    if (pObj==NULL || !pObj->IsPolyObj())
        return FALSE;

    if (pMark==NULL)
    {
        ULONG nMarkNum=TryToFindMarkedObject(pObj);
        if (nMarkNum==CONTAINER_ENTRY_NOTFOUND)
            return FALSE;
        pMark=GetSdrMarkByIndex(nMarkNum);
    }
    const sal_uInt32 nHdlNum(pHdl->GetObjHdlNum());
    SdrUShortCont* pPts=pMark->ForceMarkedPoints();
    if (!bUnmark)
    {
        pPts->Insert((sal_uInt16)nHdlNum);
    }
    else
    {
        ULONG nBla=pPts->GetPos((sal_uInt16)nHdlNum);
        if (nBla!=CONTAINER_ENTRY_NOTFOUND)
        {
            pPts->Remove(nBla);
        }
        else
        {
            return FALSE; // Fehlerfall!
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
                    pPlusHdl->SetPlusHdl(TRUE);
                    aHdl.AddHdl(pPlusHdl);
                }
            }
        }
        else
        {
            for (ULONG i = aHdl.GetHdlCount(); i>0;)
            {
                i--;
                SdrHdl* pPlusHdl=aHdl.GetHdl(i);
                if (pPlusHdl->IsPlusHdl() && pPlusHdl->GetSourceHdlNum()==nHdlNum)
                {
                    aHdl.RemoveHdl(i);
                    delete pPlusHdl;
                }
            }
        }
    }

    // #97016# II: Sort handles. This was missing in ImpMarkPoint all the time.
    aHdl.Sort();

    return TRUE;
}


BOOL SdrMarkView::MarkPoint(SdrHdl& rHdl, BOOL bUnmark)
{
    if (&rHdl==NULL) return FALSE;
    ForceUndirtyMrkPnt();
    BOOL bRet=FALSE;
    const SdrObject* pObj=rHdl.GetObj();
    if (IsPointMarkable(rHdl) && rHdl.IsSelected()==bUnmark) {
        ULONG nMarkNum=TryToFindMarkedObject(pObj);
        if (nMarkNum!=CONTAINER_ENTRY_NOTFOUND) {
            SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts=pM->ForceMarkedPoints();
            pPts->ForceSort();
            if (ImpMarkPoint(&rHdl,pM,bUnmark)) {
                pPts->ForceSort();
                MarkListHasChanged();
                bRet=TRUE;
            }
        }
    }

    return bRet;
}

BOOL SdrMarkView::MarkPoints(const Rectangle* pRect, BOOL bUnmark)
{
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    SortMarkedObjects();
    const SdrObject* pObj0=NULL;
    const SdrPageView* pPV0=NULL;
    SdrMark* pM=NULL;
    aHdl.Sort();
    //HMHBOOL bHideHdl=IsMarkHdlShown() && IsSolidMarkHdl() && !bPlusHdlAlways;
    ULONG nHdlAnz=aHdl.GetHdlCount();
    for (ULONG nHdlNum=nHdlAnz; nHdlNum>0;) {
        nHdlNum--;
        SdrHdl* pHdl=aHdl.GetHdl(nHdlNum);
        if (IsPointMarkable(*pHdl) && pHdl->IsSelected()==bUnmark) {
            const SdrObject* pObj=pHdl->GetObj();
            const SdrPageView* pPV=pHdl->GetPageView();
            if (pObj!=pObj0 || pPV!=pPV0 || pM==NULL) { // Dieser Abschnitt dient zur Optimierung,
                if (pM!=NULL) {
                    SdrUShortCont* pPts=pM->GetMarkedPoints();
                    if (pPts!=NULL) pPts->ForceSort();
                }
                ULONG nMarkNum=TryToFindMarkedObject(pObj);  // damit ImpMarkPoint() nicht staendig das
                if (nMarkNum!=CONTAINER_ENTRY_NOTFOUND) { // Objekt in der MarkList suchen muss.
                    pM=GetSdrMarkByIndex(nMarkNum);
                    pObj0=pObj;
                    pPV0=pPV;
                    SdrUShortCont* pPts=pM->ForceMarkedPoints();
                    pPts->ForceSort();
                } else {
#ifdef DBG_UTIL
                    if (pObj->IsInserted()) {
                        OSL_FAIL("SdrMarkView::MarkPoints(const Rectangle* pRect): Markiertes Objekt nicht gefunden");
                    }
#endif
                    pM=NULL;
                }
            }
            Point aPos(pHdl->GetPos());
            if (pM!=NULL && (pRect==NULL || pRect->IsInside(aPos))) {
                //HMHif (bHideHdl && IsMarkHdlShown() && pHdl->GetObj()!=NULL) {
                    //HMHsal_uInt32 nAnz=pHdl->GetObj()->GetPlusHdlCount(*pHdl);
                    //HMHif (nAnz!=0L) HideMarkHdl(); // #36987#
                //HMH}
                if (ImpMarkPoint(pHdl,pM,bUnmark)) bChgd=TRUE;
            }
        }
    }
    if (pM!=NULL) { // Den zuletzt geaenderten MarkEntry ggf. noch aufraeumen
        SdrUShortCont* pPts=pM->GetMarkedPoints();
        if (pPts!=NULL) pPts->ForceSort();
    }
    //HMHif (bHideHdl) ShowMarkHdl(); // #36987#
    if (bChgd) {
        MarkListHasChanged();
    }

    return bChgd;
}

BOOL SdrMarkView::MarkNextPoint(BOOL /*bPrev*/)
{
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    SortMarkedObjects();
    // ...
    if (bChgd) {
        MarkListHasChanged();
    }
    return bChgd;
}

BOOL SdrMarkView::MarkNextPoint(const Point& /*rPnt*/, BOOL /*bPrev*/)
{
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    SortMarkedObjects();
    // ...
    if (bChgd) {
        MarkListHasChanged();
    }
    return bChgd;
}

const Rectangle& SdrMarkView::GetMarkedPointsRect() const
{
    ForceUndirtyMrkPnt();
    if (bMarkedPointsRectsDirty) ImpSetPointsRects();
    return aMarkedPointsRect;
}

void SdrMarkView::SetPlusHandlesAlwaysVisible(BOOL bOn)
{ // HandlePaint optimieren !!!!!!!
    ForceUndirtyMrkPnt();
    if (bOn!=bPlusHdlAlways) {
        //HMHBOOL bVis=IsMarkHdlShown();
        //HMHif (bVis) HideMarkHdl();
        bPlusHdlAlways=bOn;
        SetMarkHandles();
        //HMHif (bVis) ShowMarkHdl();
        MarkListHasChanged();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ImpSetPointsRects() ist fuer PolyPoints und GluePoints!
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::ImpSetPointsRects() const
{
    Rectangle aPnts;
    Rectangle aGlue;
    ULONG nHdlAnz=aHdl.GetHdlCount();
    for (ULONG nHdlNum=0; nHdlNum<nHdlAnz; nHdlNum++) {
        const SdrHdl* pHdl=aHdl.GetHdl(nHdlNum);
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
    ((SdrMarkView*)this)->bMarkedPointsRectsDirty=FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// UndirtyMrkPnt() ist fuer PolyPoints und GluePoints!
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::UndirtyMrkPnt() const
{
    BOOL bChg=FALSE;
    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
        SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrObject* pObj=pM->GetMarkedSdrObj();
        // PolyPoints
        SdrUShortCont* pPts=pM->GetMarkedPoints();
        if (pPts!=NULL) {
            if (pObj->IsPolyObj()) {
                // Ungueltig markierte Punkte entfernen, also alle
                // Eintraege die groesser sind als die Punktanzahl des Objekts
                sal_uInt32 nMax(pObj->GetPointCount());
                sal_uInt32 nPtNum(0xffffffff);

                pPts->ForceSort();

                for (sal_uInt32 nIndex(pPts->GetCount()); nIndex > 0L && nPtNum >= nMax;)
                {
                    nIndex--;
                    nPtNum = pPts->GetObject(nIndex);

                    if(nPtNum >= nMax)
                    {
                        pPts->Remove(nIndex);
                        bChg = TRUE;
                    }
                }
            }
            else
            {
                OSL_FAIL("SdrMarkView::UndirtyMrkPnt(): Markierte Punkte an einem Objekt, dass kein PolyObj ist!");
                if(pPts && pPts->GetCount())
                {
                    pPts->Clear();
                    bChg = TRUE;
                }
            }
        }

        // GluePoints
        pPts=pM->GetMarkedGluePoints();
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
        if (pPts!=NULL) {
            if (pGPL!=NULL) {
                // Ungueltig markierte Klebepunkte entfernen, also alle
                // Eintraege (Id's) die nicht in der GluePointList des
                // Objekts enthalten sind
                pPts->ForceSort();
                for (ULONG nIndex=pPts->GetCount(); nIndex>0;) {
                    nIndex--;
                    USHORT nId=pPts->GetObject(nIndex);
                    if (pGPL->FindGluePoint(nId)==SDRGLUEPOINT_NOTFOUND) {
                        pPts->Remove(nIndex);
                        bChg=TRUE;
                    }
                }
            } else {
                if (pPts!=NULL && pPts->GetCount()!=0) {
                    pPts->Clear(); // Objekt hat keine Klebepunkte (mehr)
                    bChg=TRUE;
                }
            }
        }
    }
    if (bChg) ((SdrMarkView*)this)->bMarkedPointsRectsDirty=TRUE;
    ((SdrMarkView*)this)->bMrkPntDirty=FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrMarkView::HasMarkableGluePoints() const
{
    BOOL bRet=FALSE;
    if (IsGluePointEditMode()) {
        ForceUndirtyMrkPnt();
        ULONG nMarkAnz=GetMarkedObjectCount();
        for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
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
                        bRet = TRUE;
                    }
                }
            }
        }
    }
    return bRet;
}

ULONG SdrMarkView::GetMarkableGluePointCount() const
{
    ULONG nAnz=0;
    if (IsGluePointEditMode()) {
        ForceUndirtyMrkPnt();
        ULONG nMarkAnz=GetMarkedObjectCount();
        for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
            const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            const SdrObject* pObj=pM->GetMarkedSdrObj();
            const SdrGluePointList* pGPL=pObj->GetGluePointList();

            // #i38892#
            if(pGPL && pGPL->GetCount())
            {
                for(sal_uInt16 a(0); a < pGPL->GetCount(); a++)
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

BOOL SdrMarkView::HasMarkedGluePoints() const
{
    ForceUndirtyMrkPnt();
    BOOL bRet=FALSE;
    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
        const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        bRet=pPts!=NULL && pPts->GetCount()!=0;
    }
    return bRet;
}

ULONG SdrMarkView::GetMarkedGluePointCount() const
{
    ForceUndirtyMrkPnt();
    ULONG nAnz=0;
    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
        const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        if (pPts!=NULL) nAnz+=pPts->GetCount();
    }
    return nAnz;
}

BOOL SdrMarkView::MarkGluePoints(const Rectangle* pRect, BOOL bUnmark)
{
    if (!IsGluePointEditMode() && !bUnmark) return FALSE;
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    SortMarkedObjects();
    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
        SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
        SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        if (bUnmark && pRect==NULL) { // UnmarkAll
            if (pPts!=NULL && pPts->GetCount()!=0) {
                pPts->Clear();
                bChgd=TRUE;
            }
        } else {
            if (pGPL!=NULL && (pPts!=NULL || !bUnmark)) {
                USHORT nGPAnz=pGPL->GetCount();
                for (USHORT nGPNum=0; nGPNum<nGPAnz; nGPNum++) {
                    const SdrGluePoint& rGP=(*pGPL)[nGPNum];

                    // #i38892#
                    if(rGP.IsUserDefined())
                    {
                        Point aPos(rGP.GetAbsolutePos(*pObj));
                        if (pRect==NULL || pRect->IsInside(aPos)) {
                            if (pPts==NULL) pPts=pM->ForceMarkedGluePoints();
                            else pPts->ForceSort();
                            ULONG nPos=pPts->GetPos(rGP.GetId());
                            if (!bUnmark && nPos==CONTAINER_ENTRY_NOTFOUND) {
                                bChgd=TRUE;
                                pPts->Insert(rGP.GetId());
                            }
                            if (bUnmark && nPos!=CONTAINER_ENTRY_NOTFOUND) {
                                bChgd=TRUE;
                                pPts->Remove(nPos);
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

BOOL SdrMarkView::PickGluePoint(const Point& rPnt, SdrObject*& rpObj, USHORT& rnId, SdrPageView*& rpPV, ULONG nOptions) const
{
    SdrObject* pObj0=rpObj;
    //SdrPageView* pPV0=rpPV;
    USHORT nId0=rnId;
    rpObj=NULL; rpPV=NULL; rnId=0;
    if (!IsGluePointEditMode()) return FALSE;
    BOOL bBack=(nOptions & SDRSEARCH_BACKWARD) !=0;
    BOOL bNext=(nOptions & SDRSEARCH_NEXT) !=0;
    OutputDevice* pOut=(OutputDevice*)pActualOutDev;
    if (pOut==NULL) pOut=GetFirstOutputDevice(); //GetWin(0);
    if (pOut==NULL) return FALSE;
    SortMarkedObjects();
    ULONG nMarkAnz=GetMarkedObjectCount();
    ULONG nMarkNum=bBack ? 0 : nMarkAnz;
    if (bNext) {
        nMarkNum=((SdrMarkView*)this)->TryToFindMarkedObject(pObj0);
        if (nMarkNum==CONTAINER_ENTRY_NOTFOUND) return FALSE;
        if (!bBack) nMarkNum++;
    }
    while (bBack ? nMarkNum<nMarkAnz : nMarkNum>0) {
        if (!bBack) nMarkNum--;
        const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrPageView* pPV=pM->GetPageView();
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
        if (pGPL!=NULL) {
            USHORT nNum=pGPL->HitTest(rPnt,*pOut,pObj,bBack,bNext,nId0);
            if (nNum!=SDRGLUEPOINT_NOTFOUND)
            {
                // #i38892#
                const SdrGluePoint& rCandidate = (*pGPL)[nNum];

                if(rCandidate.IsUserDefined())
                {
                    rpObj=pObj;
                    rnId=(*pGPL)[nNum].GetId();
                    rpPV=pPV;
                    return TRUE;
                }
            }
        }
        bNext=FALSE; // HitNextGluePoint nur beim ersten Obj
        if (bBack) nMarkNum++;
    }
    return FALSE;
}

BOOL SdrMarkView::MarkGluePoint(const SdrObject* pObj, USHORT nId, const SdrPageView* /*pPV*/, BOOL bUnmark)
{
    if (!IsGluePointEditMode()) return FALSE;
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    if (pObj!=NULL) {
        ULONG nMarkPos=TryToFindMarkedObject(pObj);
        if (nMarkPos!=CONTAINER_ENTRY_NOTFOUND) {
            SdrMark* pM=GetSdrMarkByIndex(nMarkPos);
            SdrUShortCont* pPts=bUnmark ? pM->GetMarkedGluePoints() : pM->ForceMarkedGluePoints();
            if (pPts!=NULL) {
                ULONG nPointPos=pPts->GetPos(nId);
                if (!bUnmark && nPointPos==CONTAINER_ENTRY_NOTFOUND) {
                    bChgd=TRUE;
                    pPts->Insert(nId);
                }
                if (bUnmark && nPointPos!=CONTAINER_ENTRY_NOTFOUND) {
                    bChgd=TRUE;
                    pPts->Remove(nPointPos);
                }
            }
        } else {
            // Objekt implizit markieren ...
            // ... fehlende Implementation
        }
    }
    if (bChgd) {
        AdjustMarkHdl();
        MarkListHasChanged();
    }
    return bChgd;
}

BOOL SdrMarkView::IsGluePointMarked(const SdrObject* pObj, USHORT nId) const
{
    ForceUndirtyMrkPnt();
    BOOL bRet=FALSE;
    ULONG nPos=((SdrMarkView*)this)->TryToFindMarkedObject(pObj); // casting auf NonConst
    if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
        const SdrMark* pM=GetSdrMarkByIndex(nPos);
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        if (pPts!=NULL) {
            bRet=pPts->Exist(nId);
        }
    }
    return bRet;
}

BOOL SdrMarkView::UnmarkGluePoint(const SdrHdl& rHdl)
{
    if (&rHdl!=NULL && rHdl.GetKind()==HDL_GLUE && rHdl.GetObj()!=NULL) {
        return MarkGluePoint(rHdl.GetObj(),(sal_uInt16)rHdl.GetObjHdlNum(),rHdl.GetPageView(),TRUE);
    } else return FALSE;
}

SdrHdl* SdrMarkView::GetGluePointHdl(const SdrObject* pObj, USHORT nId) const
{
    ForceUndirtyMrkPnt();
    ULONG nHdlAnz=aHdl.GetHdlCount();
    for (ULONG nHdlNum=0; nHdlNum<nHdlAnz; nHdlNum++) {
        SdrHdl* pHdl=aHdl.GetHdl(nHdlNum);
        if (pHdl->GetObj()==pObj &&
            pHdl->GetKind()==HDL_GLUE &&
            pHdl->GetObjHdlNum()==nId ) return pHdl;
    }
    return NULL;
}

BOOL SdrMarkView::MarkNextGluePoint(BOOL /*bPrev*/)
{
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    SortMarkedObjects();
    // ...
    if (bChgd) {
        MarkListHasChanged();
    }
    return bChgd;
}

BOOL SdrMarkView::MarkNextGluePoint(const Point& /*rPnt*/, BOOL /*bPrev*/)
{
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    SortMarkedObjects();
    // ...
    if (bChgd) {
        MarkListHasChanged();
    }
    return bChgd;
}

const Rectangle& SdrMarkView::GetMarkedGluePointsRect() const
{
    ForceUndirtyMrkPnt();
    if (bMarkedPointsRectsDirty) ImpSetPointsRects();
    return aMarkedGluePointsRect;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
