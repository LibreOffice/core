/*************************************************************************
 *
 *  $RCSfile: svdmrkv1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svdmrkv.hxx"
#include "svdetc.hxx"
#include "svdoedge.hxx"
#include "svdglob.hxx"
#include "svdpagv.hxx"
#include "svdpage.hxx"
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
    BOOL bRet=FALSE;
    if (!ImpIsFrameHandles()) {
        ULONG nMarkAnz=aMark.GetMarkCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
                const SdrMark* pM=aMark.GetMark(nMarkNum);
                const SdrObject* pObj=pM->GetObj();
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
        ULONG nMarkAnz=aMark.GetMarkCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
                const SdrMark* pM=aMark.GetMark(nMarkNum);
                const SdrObject* pObj=pM->GetObj();
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
        ULONG nMarkAnz=aMark.GetMarkCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
                const SdrMark* pM=aMark.GetMark(nMarkNum);
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
        ULONG nMarkAnz=aMark.GetMarkCount();
        if (nMarkAnz<=nFrameHandlesLimit) {
            for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
                const SdrMark* pM=aMark.GetMark(nMarkNum);
                const SdrUShortCont* pPts=pM->GetMarkedPoints();
                if (pPts!=NULL) nAnz+=pPts->GetCount();
            }
        }
    }
    return nAnz;
}

BOOL SdrMarkView::IsPointMarkable(const SdrHdl& rHdl) const
{
    return !ImpIsFrameHandles() && &rHdl!=NULL && !rHdl.IsPlusHdl() && rHdl.GetKind()!=HDL_GLUE && rHdl.GetObj()!=NULL && rHdl.GetObj()->IsPolyObj();
}

BOOL SdrMarkView::ImpMarkPoint(SdrHdl* pHdl, SdrMark* pMark, BOOL bUnmark)
{
    if (pHdl==NULL || pHdl->IsPlusHdl() || pHdl->GetKind()==HDL_GLUE) return FALSE;
    if (pHdl->IsSelected() != bUnmark) return FALSE;
    SdrObject* pObj=pHdl->GetObj();
    if (pObj==NULL || !pObj->IsPolyObj()) return FALSE;
    if (pMark==NULL) {
        ULONG nMarkNum=aMark.FindObject(pObj);
        if (nMarkNum==CONTAINER_ENTRY_NOTFOUND) return FALSE;
        pMark=aMark.GetMark(nMarkNum);
    }
    USHORT nHdlNum=pHdl->GetObjHdlNum();
    SdrUShortCont* pPts=pMark->ForceMarkedPoints();
    if (!bUnmark) {
        pPts->Insert(nHdlNum);
    } else {
        ULONG nBla=pPts->GetPos(nHdlNum);
        if (nBla!=CONTAINER_ENTRY_NOTFOUND) {
            pPts->Remove(nBla);
        } else return FALSE; // Fehlerfall!
    }
    BOOL bVis=IsMarkHdlShown();
    USHORT nWinAnz=GetWinCount();
//    if (bVis) for (nw=0; nw<nWinAnz; nw++) pHdl->Draw(*GetWin(nw));
    pHdl->SetSelected(!bUnmark);
//    if (bVis) for (nw=0; nw<nWinAnz; nw++) pHdl->Draw(*GetWin(nw));
    if (!bPlusHdlAlways) {
        BOOL bSolid=IsSolidMarkHdl();
        if (!bUnmark) {
            USHORT nAnz=pObj->GetPlusHdlCount(*pHdl);
            if (nAnz!=0 && bSolid && bVis) HideMarkHdl(NULL);
            for (USHORT i=0; i<nAnz; i++) {
                SdrHdl* pPlusHdl=pObj->GetPlusHdl(*pHdl,i);
                if (pPlusHdl!=NULL) {
                    pPlusHdl->SetObj(pObj);
                    pPlusHdl->SetPageView(pMark->GetPageView());
                    pPlusHdl->SetPlusHdl(TRUE);
                    aHdl.AddHdl(pPlusHdl);
//                    if (bVis && !bSolid) for (nw=0; nw<nWinAnz; nw++) pPlusHdl->Draw(*GetWin(nw));
                }
            }
            if (nAnz!=0 && bSolid && bVis) ShowMarkHdl(NULL);
        } else {
            ULONG nAnz=aHdl.GetHdlCount();
            for (ULONG i=nAnz; i>0;) {
                i--;
                SdrHdl* pPlusHdl=aHdl.GetHdl(i);
                BOOL bFlag=FALSE;
                if (pPlusHdl->IsPlusHdl() && pPlusHdl->GetSourceHdlNum()==nHdlNum) {
                    if (bVis) {
                        if (bSolid) {
                            bFlag=TRUE;
                            HideMarkHdl(NULL); // SolidMarkHdl und Bezier ist noch nicht fertig!
                        }
//                      else {
//                            for (nw=0; nw<nWinAnz; nw++) pPlusHdl->Draw(*GetWin(nw));
//                        }
                    }
                    aHdl.RemoveHdl(i);
                    delete pPlusHdl;
                }
                if (bFlag) ShowMarkHdl(NULL);
            }
        }
    }

    // refresh IAOs
//  RefreshAllIAOManagers();

    return TRUE;
}

BOOL SdrMarkView::MarkPoint(SdrHdl& rHdl, BOOL bUnmark)
{
    if (&rHdl==NULL) return FALSE;
    ForceUndirtyMrkPnt();
    BOOL bRet=FALSE;
    const SdrObject* pObj=rHdl.GetObj();
    if (IsPointMarkable(rHdl) && rHdl.IsSelected()==bUnmark) {
        ULONG nMarkNum=aMark.FindObject(pObj);
        if (nMarkNum!=CONTAINER_ENTRY_NOTFOUND) {
            SdrMark* pM=aMark.GetMark(nMarkNum);
            SdrUShortCont* pPts=pM->ForceMarkedPoints();
            pPts->ForceSort();
            if (ImpMarkPoint(&rHdl,pM,bUnmark)) {
                pPts->ForceSort();
                MarkListHasChanged();
                bRet=TRUE;
            }
        }
    }

    // refresh IAOs
//--/   RefreshAllIAOManagers();

    return bRet;
}

BOOL SdrMarkView::MarkPoints(const Rectangle* pRect, BOOL bUnmark)
{
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    aMark.ForceSort();
    const SdrObject* pObj0=NULL;
    const SdrPageView* pPV0=NULL;
    SdrMark* pM=NULL;
    aHdl.Sort();
    BOOL bHideHdl=IsMarkHdlShown() && IsSolidMarkHdl() && !bPlusHdlAlways;
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
                ULONG nMarkNum=aMark.FindObject(pObj);  // damit ImpMarkPoint() nicht staendig das
                if (nMarkNum!=CONTAINER_ENTRY_NOTFOUND) { // Objekt in der MarkList suchen muss.
                    pM=aMark.GetMark(nMarkNum);
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
            aPos+=pPV->GetOffset();
            if (pM!=NULL && (pRect==NULL || pRect->IsInside(aPos))) {
                if (bHideHdl && IsMarkHdlShown() && pHdl->GetObj()!=NULL) {
                    USHORT nAnz=pHdl->GetObj()->GetPlusHdlCount(*pHdl);
                    if (nAnz!=0) HideMarkHdl(NULL); // #36987#
                }
                if (ImpMarkPoint(pHdl,pM,bUnmark)) bChgd=TRUE;
            }
        }
    }
    if (pM!=NULL) { // Den zuletzt geaenderten MarkEntry ggf. noch aufraeumen
        SdrUShortCont* pPts=pM->GetMarkedPoints();
        if (pPts!=NULL) pPts->ForceSort();
    }
    if (bHideHdl) ShowMarkHdl(NULL); // #36987#
    if (bChgd) {
        MarkListHasChanged();
    }

    // refresh IAOs
//--/   RefreshAllIAOManagers();

    return bChgd;
}

BOOL SdrMarkView::MarkNextPoint(BOOL bPrev)
{
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    aMark.ForceSort();
    // ...
    if (bChgd) {
        MarkListHasChanged();
    }
    return bChgd;
}

BOOL SdrMarkView::MarkNextPoint(const Point& rPnt, BOOL bPrev)
{
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    aMark.ForceSort();
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
        BOOL bVis=IsMarkHdlShown();
        if (bVis) HideMarkHdl(NULL);
        bPlusHdlAlways=bOn;
        SetMarkHandles();
        if (bVis) ShowMarkHdl(NULL);
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
            const SdrPageView* pPV=pHdl->GetPageView();
            if (pPV!=NULL) aPt+=pPV->GetOffset();
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
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
        SdrMark* pM=aMark.GetMark(nMarkNum);
        const SdrObject* pObj=pM->GetObj();
        // PolyPoints
        SdrUShortCont* pPts=pM->GetMarkedPoints();
        if (pPts!=NULL) {
            if (pObj->IsPolyObj()) {
                // Ungueltig markierte Punkte entfernen, also alle
                // Eintraege die groesser sind als die Punktanzahl des Objekts
                USHORT nMax=pObj->GetPointCount();
                USHORT nPtNum=0xFFFF;
                pPts->ForceSort();
                for (ULONG nIndex=pPts->GetCount(); nIndex>0 && nPtNum>=nMax;) {
                    nIndex--;
                    nPtNum=pPts->GetObject(nIndex);
                    if (nPtNum>=nMax) {
                        pPts->Remove(nIndex);
                        bChg=TRUE;
                    }
                }
            } else {
                DBG_ERROR("SdrMarkView::UndirtyMrkPnt(): Markierte Punkte an einem Objekt, dass kein PolyObj ist!");
                if (pPts!=NULL && pPts->GetCount()!=0) {
                    pPts->Clear();
                    bChg=TRUE;
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
        ULONG nMarkAnz=aMark.GetMarkCount();
        for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
            const SdrMark* pM=aMark.GetMark(nMarkNum);
            const SdrObject* pObj=pM->GetObj();
            const SdrGluePointList* pGPL=pObj->GetGluePointList();
            bRet=pGPL!=NULL && pGPL->GetCount()!=0;
        }
    }
    return bRet;
}

ULONG SdrMarkView::GetMarkableGluePointCount() const
{
    ULONG nAnz=0;
    if (IsGluePointEditMode()) {
        ForceUndirtyMrkPnt();
        ULONG nMarkAnz=aMark.GetMarkCount();
        for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
            const SdrMark* pM=aMark.GetMark(nMarkNum);
            const SdrObject* pObj=pM->GetObj();
            const SdrGluePointList* pGPL=pObj->GetGluePointList();
            if (pGPL!=NULL) {
                nAnz+=pGPL->GetCount();
            }
        }
    }
    return nAnz;
}

BOOL SdrMarkView::HasMarkedGluePoints() const
{
    ForceUndirtyMrkPnt();
    BOOL bRet=FALSE;
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
        const SdrMark* pM=aMark.GetMark(nMarkNum);
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        bRet=pPts!=NULL && pPts->GetCount()!=0;
    }
    return bRet;
}

ULONG SdrMarkView::GetMarkedGluePointCount() const
{
    ForceUndirtyMrkPnt();
    ULONG nAnz=0;
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
        const SdrMark* pM=aMark.GetMark(nMarkNum);
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
    aMark.ForceSort();
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
        SdrMark* pM=aMark.GetMark(nMarkNum);
        const SdrObject* pObj=pM->GetObj();
        const SdrPageView* pPV=pM->GetPageView();
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
                    Point aPos(rGP.GetAbsolutePos(*pObj));
                    aPos+=pPV->GetOffset();
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
    if (bChgd) {
        AdjustMarkHdl();
        MarkListHasChanged();
    }
    return bChgd;
}

BOOL SdrMarkView::PickGluePoint(const Point& rPnt, SdrObject*& rpObj, USHORT& rnId, SdrPageView*& rpPV, ULONG nOptions) const
{
    SdrObject* pObj0=rpObj;
    SdrPageView* pPV0=rpPV;
    USHORT nId0=rnId;
    rpObj=NULL; rpPV=NULL; rnId=0;
    if (!IsGluePointEditMode()) return FALSE;
    BOOL bBack=(nOptions & SDRSEARCH_BACKWARD) !=0;
    BOOL bNext=(nOptions & SDRSEARCH_NEXT) !=0;
    const OutputDevice* pOut=(const OutputDevice*)pActualOutDev;
    if (pOut==NULL) const OutputDevice* pOut=GetWin(0);
    if (pOut==NULL) return FALSE;
    ((SdrMarkView*)this)->aMark.ForceSort();
    ULONG nMarkAnz=aMark.GetMarkCount();
    ULONG nMarkNum=bBack ? 0 : nMarkAnz;
    if (bNext) {
        nMarkNum=((SdrMarkView*)this)->aMark.FindObject(pObj0);
        if (nMarkNum==CONTAINER_ENTRY_NOTFOUND) return FALSE;
        if (!bBack) nMarkNum++;
    }
    while (bBack ? nMarkNum<nMarkAnz : nMarkNum>0) {
        if (!bBack) nMarkNum--;
        const SdrMark* pM=aMark.GetMark(nMarkNum);
        SdrObject* pObj=pM->GetObj();
        SdrPageView* pPV=pM->GetPageView();
        const SdrGluePointList* pGPL=pObj->GetGluePointList();
        if (pGPL!=NULL) {
            Point aPnt(rPnt);
            aPnt-=pPV->GetOffset();
            USHORT nNum=pGPL->HitTest(aPnt,*pOut,pObj,bBack,bNext,nId0);
            if (nNum!=SDRGLUEPOINT_NOTFOUND) {
                rpObj=pObj;
                rnId=(*pGPL)[nNum].GetId();
                rpPV=pPV;
                return TRUE;
            }
        }
        bNext=FALSE; // HitNextGluePoint nur beim ersten Obj
        if (bBack) nMarkNum++;
    }
    return FALSE;
}

BOOL SdrMarkView::MarkGluePoint(const SdrObject* pObj, USHORT nId, const SdrPageView* pPV, BOOL bUnmark)
{
    if (!IsGluePointEditMode()) return FALSE;
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    if (pObj!=NULL) {
        ULONG nMarkPos=aMark.FindObject(pObj);
        if (nMarkPos!=CONTAINER_ENTRY_NOTFOUND) {
            SdrMark* pM=aMark.GetMark(nMarkPos);
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
    ULONG nPos=((SdrMarkView*)this)->aMark.FindObject(pObj); // casting auf NonConst
    if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
        const SdrMark* pM=aMark.GetMark(nPos);
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
        return MarkGluePoint(rHdl.GetObj(),rHdl.GetObjHdlNum(),rHdl.GetPageView(),TRUE);
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

BOOL SdrMarkView::MarkNextGluePoint(BOOL bPrev)
{
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    aMark.ForceSort();
    // ...
    if (bChgd) {
        MarkListHasChanged();
    }
    return bChgd;
}

BOOL SdrMarkView::MarkNextGluePoint(const Point& rPnt, BOOL bPrev)
{
    ForceUndirtyMrkPnt();
    BOOL bChgd=FALSE;
    aMark.ForceSort();
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

