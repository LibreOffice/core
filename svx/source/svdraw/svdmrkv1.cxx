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
#include "svx/svdglob.hxx"
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

sal_Bool SdrMarkView::HasMarkablePoints() const
{
    ForceUndirtyMrkPnt();
    bool bRet=false;
    if (!ImpIsFrameHandles()) {
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
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
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
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

sal_Bool SdrMarkView::HasMarkedPoints() const
{
    ForceUndirtyMrkPnt();
    sal_Bool bRet=sal_False;
    if (!ImpIsFrameHandles()) {
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrUShortCont* pPts=pM->GetMarkedPoints();
                bRet=pPts!=NULL && pPts->GetCount()!=0;
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
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrUShortCont* pPts=pM->GetMarkedPoints();
                if (pPts!=NULL) nAnz+=pPts->GetCount();
            }
        }
    }
    return nAnz;
}

sal_Bool SdrMarkView::IsPointMarkable(const SdrHdl& rHdl) const
{
    return !ImpIsFrameHandles() && &rHdl!=NULL && !rHdl.IsPlusHdl() && rHdl.GetKind()!=HDL_GLUE && rHdl.GetKind()!=HDL_SMARTTAG && rHdl.GetObj()!=NULL && rHdl.GetObj()->IsPolyObj();
}

sal_Bool SdrMarkView::MarkPointHelper(SdrHdl* pHdl, SdrMark* pMark, sal_Bool bUnmark)
{
    return ImpMarkPoint( pHdl, pMark, bUnmark );
}

sal_Bool SdrMarkView::ImpMarkPoint(SdrHdl* pHdl, SdrMark* pMark, sal_Bool bUnmark)
{
    if (pHdl==NULL || pHdl->IsPlusHdl() || pHdl->GetKind()==HDL_GLUE)
        return sal_False;

    if (pHdl->IsSelected() != bUnmark)
        return sal_False;

    SdrObject* pObj=pHdl->GetObj();
    if (pObj==NULL || !pObj->IsPolyObj())
        return sal_False;

    if (pMark==NULL)
    {
        sal_uIntPtr nMarkNum=TryToFindMarkedObject(pObj);
        if (nMarkNum==CONTAINER_ENTRY_NOTFOUND)
            return sal_False;
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
        sal_uIntPtr nBla=pPts->GetPos((sal_uInt16)nHdlNum);
        if (nBla!=CONTAINER_ENTRY_NOTFOUND)
        {
            pPts->Remove(nBla);
        }
        else
        {
            return sal_False; // Fehlerfall!
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
                    pPlusHdl->SetPlusHdl(sal_True);
                    aHdl.AddHdl(pPlusHdl);
                }
            }
        }
        else
        {
            for (sal_uIntPtr i = aHdl.GetHdlCount(); i>0;)
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

    return sal_True;
}


sal_Bool SdrMarkView::MarkPoint(SdrHdl& rHdl, sal_Bool bUnmark)
{
    if (&rHdl==NULL) return sal_False;
    ForceUndirtyMrkPnt();
    sal_Bool bRet=sal_False;
    const SdrObject* pObj=rHdl.GetObj();
    if (IsPointMarkable(rHdl) && rHdl.IsSelected()==bUnmark) {
        sal_uIntPtr nMarkNum=TryToFindMarkedObject(pObj);
        if (nMarkNum!=CONTAINER_ENTRY_NOTFOUND) {
            SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            SdrUShortCont* pPts=pM->ForceMarkedPoints();
            pPts->ForceSort();
            if (ImpMarkPoint(&rHdl,pM,bUnmark)) {
                pPts->ForceSort();
                MarkListHasChanged();
                bRet=sal_True;
            }
        }
    }

    return bRet;
}

sal_Bool SdrMarkView::MarkPoints(const Rectangle* pRect, sal_Bool bUnmark)
{
    ForceUndirtyMrkPnt();
    sal_Bool bChgd=sal_False;
    SortMarkedObjects();
    const SdrObject* pObj0=NULL;
    const SdrPageView* pPV0=NULL;
    SdrMark* pM=NULL;
    aHdl.Sort();
    //HMHBOOL bHideHdl=IsMarkHdlShown() && IsSolidMarkHdl() && !bPlusHdlAlways;
    sal_uIntPtr nHdlAnz=aHdl.GetHdlCount();
    for (sal_uIntPtr nHdlNum=nHdlAnz; nHdlNum>0;) {
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
                sal_uIntPtr nMarkNum=TryToFindMarkedObject(pObj);  // damit ImpMarkPoint() nicht staendig das
                if (nMarkNum!=CONTAINER_ENTRY_NOTFOUND) { // Objekt in der MarkList suchen muss.
                    pM=GetSdrMarkByIndex(nMarkNum);
                    pObj0=pObj;
                    pPV0=pPV;
                    SdrUShortCont* pPts=pM->ForceMarkedPoints();
                    pPts->ForceSort();
                } else {
#ifdef DBG_UTIL
                    if (pObj->IsInserted()) {
                        DBG_ERROR("SdrMarkView::MarkPoints(const Rectangle* pRect): Markiertes Objekt nicht gefunden");
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
                if (ImpMarkPoint(pHdl,pM,bUnmark)) bChgd=sal_True;
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

sal_Bool SdrMarkView::MarkNextPoint(sal_Bool /*bPrev*/)
{
    ForceUndirtyMrkPnt();
    sal_Bool bChgd=sal_False;
    SortMarkedObjects();
    // ...
    if (bChgd) {
        MarkListHasChanged();
    }
    return bChgd;
}

sal_Bool SdrMarkView::MarkNextPoint(const Point& /*rPnt*/, sal_Bool /*bPrev*/)
{
    ForceUndirtyMrkPnt();
    sal_Bool bChgd=sal_False;
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

void SdrMarkView::SetPlusHandlesAlwaysVisible(sal_Bool bOn)
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
    sal_uIntPtr nHdlAnz=aHdl.GetHdlCount();
    for (sal_uIntPtr nHdlNum=0; nHdlNum<nHdlAnz; nHdlNum++) {
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
    ((SdrMarkView*)this)->bMarkedPointsRectsDirty=sal_False;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// UndirtyMrkPnt() ist fuer PolyPoints und GluePoints!
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::UndirtyMrkPnt() const
{
    sal_Bool bChg=sal_False;
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
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
                        bChg = sal_True;
                    }
                }
            }
            else
            {
                DBG_ERROR("SdrMarkView::UndirtyMrkPnt(): Markierte Punkte an einem Objekt, dass kein PolyObj ist!");
                if(pPts && pPts->GetCount())
                {
                    pPts->Clear();
                    bChg = sal_True;
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
                for (sal_uIntPtr nIndex=pPts->GetCount(); nIndex>0;) {
                    nIndex--;
                    sal_uInt16 nId=pPts->GetObject(nIndex);
                    if (pGPL->FindGluePoint(nId)==SDRGLUEPOINT_NOTFOUND) {
                        pPts->Remove(nIndex);
                        bChg=sal_True;
                    }
                }
            } else {
                if (pPts!=NULL && pPts->GetCount()!=0) {
                    pPts->Clear(); // Objekt hat keine Klebepunkte (mehr)
                    bChg=sal_True;
                }
            }
        }
    }
    if (bChg) ((SdrMarkView*)this)->bMarkedPointsRectsDirty=sal_True;
    ((SdrMarkView*)this)->bMrkPntDirty=sal_False;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrMarkView::HasMarkableGluePoints() const
{
    sal_Bool bRet=sal_False;
    if (IsGluePointEditMode()) {
        ForceUndirtyMrkPnt();
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
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
                        bRet = sal_True;
                    }
                }
            }
        }
    }
    return bRet;
}

sal_uIntPtr SdrMarkView::GetMarkableGluePointCount() const
{
    sal_uIntPtr nAnz=0;
    if (IsGluePointEditMode()) {
        ForceUndirtyMrkPnt();
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
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

sal_Bool SdrMarkView::HasMarkedGluePoints() const
{
    ForceUndirtyMrkPnt();
    sal_Bool bRet=sal_False;
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
        const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        bRet=pPts!=NULL && pPts->GetCount()!=0;
    }
    return bRet;
}

sal_uIntPtr SdrMarkView::GetMarkedGluePointCount() const
{
    ForceUndirtyMrkPnt();
    sal_uIntPtr nAnz=0;
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
        const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        if (pPts!=NULL) nAnz+=pPts->GetCount();
    }
    return nAnz;
}

sal_Bool SdrMarkView::MarkGluePoints(const Rectangle* pRect, sal_Bool bUnmark)
{
    if (!IsGluePointEditMode() && !bUnmark) return sal_False;
    ForceUndirtyMrkPnt();
    sal_Bool bChgd=sal_False;
    SortMarkedObjects();
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
        SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        const SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
        SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        if (bUnmark && pRect==NULL) { // UnmarkAll
            if (pPts!=NULL && pPts->GetCount()!=0) {
                pPts->Clear();
                bChgd=sal_True;
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
                            if (pPts==NULL) pPts=pM->ForceMarkedGluePoints();
                            else pPts->ForceSort();
                            sal_uIntPtr nPos=pPts->GetPos(rGP.GetId());
                            if (!bUnmark && nPos==CONTAINER_ENTRY_NOTFOUND) {
                                bChgd=sal_True;
                                pPts->Insert(rGP.GetId());
                            }
                            if (bUnmark && nPos!=CONTAINER_ENTRY_NOTFOUND) {
                                bChgd=sal_True;
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

sal_Bool SdrMarkView::PickGluePoint(const Point& rPnt, SdrObject*& rpObj, sal_uInt16& rnId, SdrPageView*& rpPV, sal_uIntPtr nOptions) const
{
    SdrObject* pObj0=rpObj;
    //SdrPageView* pPV0=rpPV;
    sal_uInt16 nId0=rnId;
    rpObj=NULL; rpPV=NULL; rnId=0;
    if (!IsGluePointEditMode()) return sal_False;
    sal_Bool bBack=(nOptions & SDRSEARCH_BACKWARD) !=0;
    sal_Bool bNext=(nOptions & SDRSEARCH_NEXT) !=0;
    OutputDevice* pOut=(OutputDevice*)pActualOutDev;
    if (pOut==NULL) pOut=GetFirstOutputDevice(); //GetWin(0);
    if (pOut==NULL) return sal_False;
    SortMarkedObjects();
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    sal_uIntPtr nMarkNum=bBack ? 0 : nMarkAnz;
    if (bNext) {
        nMarkNum=((SdrMarkView*)this)->TryToFindMarkedObject(pObj0);
        if (nMarkNum==CONTAINER_ENTRY_NOTFOUND) return sal_False;
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
                    return sal_True;
                }
            }
        }
        bNext=sal_False; // HitNextGluePoint nur beim ersten Obj
        if (bBack) nMarkNum++;
    }
    return sal_False;
}

sal_Bool SdrMarkView::MarkGluePoint(const SdrObject* pObj, sal_uInt16 nId, const SdrPageView* /*pPV*/, sal_Bool bUnmark)
{
    if (!IsGluePointEditMode()) return sal_False;
    ForceUndirtyMrkPnt();
    sal_Bool bChgd=sal_False;
    if (pObj!=NULL) {
        sal_uIntPtr nMarkPos=TryToFindMarkedObject(pObj);
        if (nMarkPos!=CONTAINER_ENTRY_NOTFOUND) {
            SdrMark* pM=GetSdrMarkByIndex(nMarkPos);
            SdrUShortCont* pPts=bUnmark ? pM->GetMarkedGluePoints() : pM->ForceMarkedGluePoints();
            if (pPts!=NULL) {
                sal_uIntPtr nPointPos=pPts->GetPos(nId);
                if (!bUnmark && nPointPos==CONTAINER_ENTRY_NOTFOUND) {
                    bChgd=sal_True;
                    pPts->Insert(nId);
                }
                if (bUnmark && nPointPos!=CONTAINER_ENTRY_NOTFOUND) {
                    bChgd=sal_True;
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

sal_Bool SdrMarkView::IsGluePointMarked(const SdrObject* pObj, sal_uInt16 nId) const
{
    ForceUndirtyMrkPnt();
    sal_Bool bRet=sal_False;
    sal_uIntPtr nPos=((SdrMarkView*)this)->TryToFindMarkedObject(pObj); // casting auf NonConst
    if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
        const SdrMark* pM=GetSdrMarkByIndex(nPos);
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        if (pPts!=NULL) {
            bRet=pPts->Exist(nId);
        }
    }
    return bRet;
}

sal_Bool SdrMarkView::UnmarkGluePoint(const SdrHdl& rHdl)
{
    if (&rHdl!=NULL && rHdl.GetKind()==HDL_GLUE && rHdl.GetObj()!=NULL) {
        return MarkGluePoint(rHdl.GetObj(),(sal_uInt16)rHdl.GetObjHdlNum(),rHdl.GetPageView(),sal_True);
    } else return sal_False;
}

SdrHdl* SdrMarkView::GetGluePointHdl(const SdrObject* pObj, sal_uInt16 nId) const
{
    ForceUndirtyMrkPnt();
    sal_uIntPtr nHdlAnz=aHdl.GetHdlCount();
    for (sal_uIntPtr nHdlNum=0; nHdlNum<nHdlAnz; nHdlNum++) {
        SdrHdl* pHdl=aHdl.GetHdl(nHdlNum);
        if (pHdl->GetObj()==pObj &&
            pHdl->GetKind()==HDL_GLUE &&
            pHdl->GetObjHdlNum()==nId ) return pHdl;
    }
    return NULL;
}

sal_Bool SdrMarkView::MarkNextGluePoint(sal_Bool /*bPrev*/)
{
    ForceUndirtyMrkPnt();
    sal_Bool bChgd=sal_False;
    SortMarkedObjects();
    // ...
    if (bChgd) {
        MarkListHasChanged();
    }
    return bChgd;
}

sal_Bool SdrMarkView::MarkNextGluePoint(const Point& /*rPnt*/, sal_Bool /*bPrev*/)
{
    ForceUndirtyMrkPnt();
    sal_Bool bChgd=sal_False;
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

