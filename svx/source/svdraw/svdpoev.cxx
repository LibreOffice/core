/*************************************************************************
 *
 *  $RCSfile: svdpoev.cxx,v $
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

#include "svdpoev.hxx"
#include <math.h>
#include "svdpagv.hxx"
#include "svdpage.hxx"
#include "svdopath.hxx"
#include "svdundo.hxx"
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache
#include "svdtrans.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPolyEditView::ImpResetPolyPossibilityFlags()
{
    eMarkedPointsSmooth=SDRPATHSMOOTH_DONTCARE;
    eMarkedSegmentsKind=SDRPATHSEGMENT_DONTCARE;
    bSetMarkedPointsSmoothPossible=FALSE;
    bSetMarkedSegmentsKindPossible=FALSE;
}

void SdrPolyEditView::ImpClearVars()
{
    ImpResetPolyPossibilityFlags();
}

SdrPolyEditView::SdrPolyEditView(SdrModel* pModel1, OutputDevice* pOut):
    SdrEditView(pModel1,pOut)
{
    ImpClearVars();
}

SdrPolyEditView::SdrPolyEditView(SdrModel* pModel1, ExtOutputDevice* pXOut):
    SdrEditView(pModel1,pXOut)
{
    ImpClearVars();
}

SdrPolyEditView::~SdrPolyEditView()
{
}

void SdrPolyEditView::ImpCheckPolyPossibilities()
{
    ImpResetPolyPossibilityFlags();
    ULONG nMarkAnz=aMark.GetMarkCount();
    if (nMarkAnz>0 && !ImpIsFrameHandles()) {
        BOOL bReadOnly=FALSE;
        BOOL b1stSmooth=TRUE;
        BOOL b1stSegm=TRUE;
        BOOL bCurve=FALSE;
        BOOL bSmoothFuz=FALSE;
        BOOL bSegmFuz=FALSE;
        XPolyFlags eSmooth=XPOLY_NORMAL;

        for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
            SdrMark* pM=aMark.GetMark(nMarkNum);
            SdrObject* pObj=pM->GetObj();
            SdrUShortCont* pPts=pM->GetMarkedPoints();
            SdrPathObj* pPath=PTR_CAST(SdrPathObj,pObj);
            if (pPath!=NULL && pPts!=NULL) {
                ULONG nMarkedPntAnz=pPts->GetCount();
                if (nMarkedPntAnz!=0) {
                    BOOL bClosed=pPath->IsClosed();
                    bSetMarkedPointsSmoothPossible=TRUE;
                    if (bClosed) bSetMarkedSegmentsKindPossible=TRUE;
                    const XPolyPolygon& rXPP=pPath->GetPathPoly();
                    for (USHORT nMarkedPntNum=0; nMarkedPntNum<(USHORT)nMarkedPntAnz; nMarkedPntNum++) {
                        USHORT nNum=pPts->GetObject(nMarkedPntNum);
                        USHORT nPolyNum=0,nPntNum=0;
                        if (pPath->FindPolyPnt(nNum,nPolyNum,nPntNum,FALSE)) {
                            const XPolygon& rXP=rXPP[nPolyNum];
                            BOOL bCanSegment=bClosed || nPntNum<rXP.GetPointCount()-1;

                            if (!bSetMarkedSegmentsKindPossible && bCanSegment) {
                                bSetMarkedSegmentsKindPossible=TRUE;
                            }
                            if (!bSmoothFuz) {
                                if (b1stSmooth) {
                                    b1stSmooth=FALSE;
                                    eSmooth=rXP.GetFlags(nPntNum);
                                } else {
                                    bSmoothFuz=eSmooth!=rXP.GetFlags(nPntNum);
                                }
                            }
                            if (!bSegmFuz) {
                                if (bCanSegment) {
                                    BOOL bCrv=rXP.IsControl(nPntNum+1);
                                    if (b1stSegm) {
                                        b1stSegm=FALSE;
                                        bCurve=bCrv;
                                    } else {
                                        bSegmFuz=bCrv!=bCurve;
                                    }
                                }
                            }
                        }
                    }
                    if (!b1stSmooth && !bSmoothFuz) {
                        if (eSmooth==XPOLY_NORMAL) eMarkedPointsSmooth=SDRPATHSMOOTH_ANGULAR;
                        if (eSmooth==XPOLY_SMOOTH) eMarkedPointsSmooth=SDRPATHSMOOTH_ASYMMETRIC;
                        if (eSmooth==XPOLY_SYMMTR) eMarkedPointsSmooth=SDRPATHSMOOTH_SYMMETRIC;
                    }
                    if (!b1stSegm && !bSegmFuz) {
                        eMarkedSegmentsKind= bCurve ? SDRPATHSEGMENT_CURVE : SDRPATHSEGMENT_LINE;
                    }
                }
            }
        }
        if (bReadOnly) {
            bSetMarkedPointsSmoothPossible=FALSE;
            bSetMarkedSegmentsKindPossible=FALSE;
        }
    }
}

void SdrPolyEditView::SetMarkedPointsSmooth(SdrPathSmoothKind eKind)
{
    XPolyFlags eFlags;
    if (eKind==SDRPATHSMOOTH_ANGULAR) eFlags=XPOLY_NORMAL;
    else if (eKind==SDRPATHSMOOTH_ASYMMETRIC) eFlags=XPOLY_SMOOTH;
    else if (eKind==SDRPATHSMOOTH_SYMMETRIC) eFlags=XPOLY_SYMMTR;
    else return;
    if (HasMarkedPoints()) {
        aMark.ForceSort();
        BegUndo(ImpGetResStr(STR_EditSetPointsSmooth),aMark.GetPointMarkDescription()/*,SDRREPFUNC_OBJ_*/);
        ULONG nMarkAnz=aMark.GetMarkCount();
        for (ULONG nMarkNum=nMarkAnz; nMarkNum>0;) {
            nMarkNum--;
            SdrMark* pM=aMark.GetMark(nMarkNum);
            SdrUShortCont* pPts=pM->GetMarkedPoints();
            SdrObject* pObj=pM->GetObj();
            SdrPathObj* pPath=PTR_CAST(SdrPathObj,pObj);
            if (pPts!=NULL && pPath!=NULL) {
                pPts->ForceSort();
                AddUndo(new SdrUndoGeoObj(*pPath));
                Rectangle aBoundRect0; if (pPath->pUserCall!=NULL) aBoundRect0=pPath->GetBoundRect();
                pPath->SendRepaintBroadcast();
                for (ULONG nNum=pPts->GetCount(); nNum>0;) {
                    nNum--;
                    USHORT nPtNum=pPts->GetObject(nNum);
                    USHORT nPolyNum,nPntNum;
                    if (pPath->FindPolyPnt(nPtNum,nPolyNum,nPntNum,FALSE)) {
                        pPath->ImpSetSmoothFlag(nPolyNum,nPntNum,eFlags);
                    }
                }
                // die folgenden 5 Statements sowie das obige SendRepaintBroadcast,
                // da ich das wesentlich schnellere ImpSetSmoothFlag verwende.
                pPath->ImpForceKind(); // ebenso impl. an der SdrPolyEditView
                pPath->SetRectsDirty();
                pPath->SetChanged();
                pPath->SendRepaintBroadcast();
                pPath->SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
            }
        }
        EndUndo();
    }
}

void SdrPolyEditView::SetMarkedSegmentsKind(SdrPathSegmentKind eKind)
{
    SdrPathType eType;
    if (eKind==SDRPATHSEGMENT_LINE) eType=SDRPATH_LINE;
    else if (eKind==SDRPATHSEGMENT_CURVE) eType=SDRPATH_CURVE;
    else if (eKind==SDRPATHSEGMENT_TOGGLE) eType=SDRPATH_NONE;
    else return;
    if (HasMarkedPoints()) {
        aMark.ForceSort();
        BegUndo(ImpGetResStr(STR_EditSetSegmentsKind),aMark.GetPointMarkDescription()/*,SDRREPFUNC_OBJ_*/);
        ULONG nMarkAnz=aMark.GetMarkCount();
        for (ULONG nMarkNum=nMarkAnz; nMarkNum>0;) {
            nMarkNum--;
            SdrMark* pM=aMark.GetMark(nMarkNum);
            SdrUShortCont* pPts=pM->GetMarkedPoints();
            SdrObject* pObj=pM->GetObj();
            SdrPathObj* pPath=PTR_CAST(SdrPathObj,pObj);
            if (pPts!=NULL && pPath!=NULL) {
                pPts->ForceSort();
                AddUndo(new SdrUndoGeoObj(*pPath));
                Rectangle aBoundRect0; if (pPath->pUserCall!=NULL) aBoundRect0=pPath->GetBoundRect();
                pPath->SendRepaintBroadcast();
                for (ULONG nNum=pPts->GetCount(); nNum>0;) {   // hier muss ich mir noch den 1. und letzten Punkt
                    nNum--;                         // eines jeden Polygons merken (fehlende Impl.) !!!
                    USHORT nPtNum=pPts->GetObject(nNum);
                    USHORT nPolyNum,nPntNum;
                    if (pPath->FindPolyPnt(nPtNum,nPolyNum,nPntNum,FALSE)) {
                        pPath->ImpConvertSegment(nPolyNum,nPntNum,eType,FALSE);
                    }
                }
                // die folgenden 5 Statements sowie das obige SendRepaintBroadcast,
                // da ich das wesentlich schnellere ImpSetSmoothFlag verwende.
                pPath->ImpForceKind(); // ebenso impl. an der SdrPolyEditView
                pPath->SetRectsDirty();
                pPath->SetChanged();
                pPath->SendRepaintBroadcast();
                pPath->SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
            }
        }
        EndUndo();
    }
}

void SdrPolyEditView::DeleteMarkedPoints()
{
    if (HasMarkedPoints()) {
        BrkAction();
        aMark.ForceSort();
        ULONG nMarkAnz=aMark.GetMarkCount();
        // Description
        BegUndo(ImpGetResStr(STR_EditDelete),aMark.GetPointMarkDescription(),SDRREPFUNC_OBJ_DELETE);
        // und nun das eigentliche loeschen
        ULONG nObjDelCount=0;
        for (ULONG nMarkNum=nMarkAnz; nMarkNum>0;) {
            nMarkNum--;
            SdrMark* pM=aMark.GetMark(nMarkNum);
            SdrUShortCont* pPts=pM->GetMarkedPoints();
            SdrObject* pObj=pM->GetObj();
            BOOL bDel=FALSE;
            if (pPts!=NULL) {
                pPts->ForceSort();
                ULONG nMarkPtsAnz=pPts->GetCount();
                if (nMarkPtsAnz!=0) {
                    AddUndo(new SdrUndoGeoObj(*pObj));

                    if(nMarkPtsAnz > 5)
                    {
                        Rectangle aBoundRect0(pObj->GetBoundRect());
                        pObj->SendRepaintBroadcast();
                        for (ULONG i=nMarkPtsAnz; i>0 && !bDel;)
                        {
                            i--;
                            bDel=!pObj->NbcDelPoint(pPts->GetObject(i));
                        }
                        if(!bDel)
                        {
                            pObj->SetChanged();
                            pObj->SendRepaintBroadcast();
                            pObj->SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
                        }
                    }
                    else
                    {
                        for (ULONG i=nMarkPtsAnz; i>0 && !bDel;)
                        {
                            i--;
                            bDel=!pObj->DelPoint(pPts->GetObject(i));
                        }
                    }
                }
            }
            if (bDel) {
                AddUndo(new SdrUndoDelObj(*pObj));
                pM->GetPageView()->GetObjList()->RemoveObject(pObj->GetOrdNum());
                nObjDelCount++;
            }
        }
        EndUndo();
        UnmarkAllPoints();
        MarkListHasChanged();
    }
}

void SdrPolyEditView::RipUpAtMarkedPoints()
{
    if (HasMarkedPoints()) {
        aMark.ForceSort();
        ULONG nMarkAnz=aMark.GetMarkCount();
        BegUndo(ImpGetResStr(STR_EditRipUp),aMark.GetPointMarkDescription()/*,SDRREPFUNC_OBJ_MOVE*/);
        for (ULONG nMarkNum=nMarkAnz; nMarkNum>0;) {
            nMarkNum--;
            SdrMark* pM=aMark.GetMark(nMarkNum);
            SdrUShortCont* pPts=pM->GetMarkedPoints();
            SdrObject* pObj=pM->GetObj();
            if (pPts!=NULL) {
                pPts->ForceSort();
                AddUndo(new SdrUndoGeoObj(*pObj));
                BOOL bKorregFlag=FALSE;
                BOOL bInsAny=FALSE;
                ULONG nMarkPtsAnz=pPts->GetCount();
                USHORT nMax=pObj->GetHdlCount();
                for (ULONG i=nMarkPtsAnz; i>0;) {
                    i--;
                    USHORT nNewPt0Idx=0;
                    SdrObject* pNeuObj=pObj->RipPoint(pPts->GetObject(i),nNewPt0Idx);
                    if (pNeuObj!=NULL) {
                        bInsAny=TRUE;
                        SdrInsertReason aReason(SDRREASON_VIEWCALL,pObj);
                        pM->GetPageView()->GetObjList()->InsertObject(pNeuObj,pObj->GetOrdNum()+1,&aReason);
                        AddUndo(new SdrUndoNewObj(*pNeuObj));
                        MarkObj(pNeuObj,pM->GetPageView(),FALSE,TRUE);
                    }
                    if (nNewPt0Idx!=0) { // Korrektur notwendig?
                        DBG_ASSERT(bKorregFlag==FALSE,"Mehrfache Indexkorrektur bei SdrPolyEditView::RipUp()");
                        if (!bKorregFlag) {
                            bKorregFlag=TRUE;
                            for (ULONG nBla=0; nBla<nMarkPtsAnz; nBla++) {
                                USHORT nPntNum=pPts->GetObject(nBla);
                                nPntNum+=nNewPt0Idx;
                                if (nPntNum>=nMax) nPntNum-=nMax;
                                pPts->Replace(nPntNum,nBla);
                            }
                            i=nMarkPtsAnz; // ... und nochmal von vorn
                        }
                    }
                }
            }
        }
        UnmarkAllPoints();
        EndUndo();
        MarkListHasChanged();
    }
}

BOOL SdrPolyEditView::IsRipUpAtMarkedPointsPossible() const
{
    BOOL bRet=FALSE;
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
        SdrMark* pM=aMark.GetMark(nMarkNum);
        SdrObject* pObj=pM->GetObj();
        SdrUShortCont* pPts=pM->GetMarkedPoints();
        ULONG nMarkPntAnz=pPts!=NULL ? pPts->GetCount() : 0;
        if (pPts!=NULL && nMarkPntAnz!=0 && pObj!=NULL && pObj->ISA(SdrPathObj)) {
            const XPolyPolygon& rXPP=((SdrPathObj*)pObj)->GetPathPoly();
            if (rXPP.Count()==1) { // es muss genau 1 Polygon drin sein!
                const XPolygon& rXP=rXPP[0];
                USHORT nPntAnz=rXP.GetPointCount();
                if (nPntAnz>=3) {
                    bRet=pObj->IsClosedObj();
                    if (!bRet) {
                        for (ULONG nMarkPntNum=0; nMarkPntNum<nMarkPntAnz && !bRet; nMarkPntNum++) {
                            USHORT nMarkPt=pPts->GetObject(nMarkPntNum);
                            bRet=nMarkPt>0 && nMarkPt<nPntAnz-1;
                        }
                    }
                }
            }
        }
    }
    return bRet;
}

BOOL SdrPolyEditView::IsOpenCloseMarkedObjectsPossible() const
{
    BOOL bRet=FALSE;
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz && !bRet; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        if (pO->ISA(SdrPathObj)) {
            const XPolyPolygon& rXPP=((SdrPathObj*)pO)->GetPathPoly();
            USHORT nPolyAnz=rXPP.Count();
            for (USHORT nPolyNum=0; nPolyNum<nPolyAnz && !bRet; nPolyNum++) {
                const XPolygon& rXP=rXPP[nPolyNum];
                USHORT nPntAnz=rXP.GetPointCount();
                bRet=nPntAnz>=3;
            }
        }
        else
            return FALSE;
    }
    return bRet;
}

SdrObjClosedKind SdrPolyEditView::GetMarkedObjectsClosedState() const
{
    BOOL bOpen=FALSE;
    BOOL bClosed=FALSE;
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz && (!bOpen || !bClosed); nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        if (pO->ISA(SdrPathObj)) {
            const XPolyPolygon& rXPP=((SdrPathObj*)pO)->GetPathPoly();
            if (rXPP.Count()==1) { // es muss genau 1 Polygon drin sein!
                const XPolygon& rXP=rXPP[0];
                USHORT nPntAnz=rXP.GetPointCount();
                if (nPntAnz>=3) {
                    if (pO->IsClosedObj()) bClosed=TRUE; else bOpen=TRUE;
                }
            }
        }
    }
    if (bOpen && bClosed) return SDROBJCLOSED_DONTCARE;
    if (bOpen) return SDROBJCLOSED_OPEN;
    return SDROBJCLOSED_CLOSED;
}

void SdrPolyEditView::ShutMarkedObjects()
{
    CloseMarkedObjects();
}

void SdrPolyEditView::CloseMarkedObjects(BOOL bToggle, BOOL bOpen, long nOpenDistance)
{
    if (HasMarkedObj()) {
        BegUndo(ImpGetResStr(STR_EditShut),aMark.GetPointMarkDescription());
        BOOL bChg=FALSE;
        ULONG nMarkAnz=aMark.GetMarkCount();
        for (ULONG nm=0; nm<nMarkAnz; nm++) {
            SdrMark* pM=aMark.GetMark(nm);
            SdrObject* pO=pM->GetObj();
            BOOL bClosed=pO->IsClosedObj();
            if (pO->IsPolyObj() && (bClosed==bOpen) || bToggle) {
                AddUndo(new SdrUndoGeoObj(*pO));
                if (pO->ISA(SdrPathObj)) {
                    ((SdrPathObj*)pO)->ToggleClosed(nOpenDistance);
                } else {
                    if (!bOpen) pO->Shut();
                }
                bChg=TRUE;
            }
        }
        EndUndo();
        if (bChg) {
            UnmarkAllPoints();
            MarkListHasChanged();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPolyEditView::ImpCopyMarkedPoints()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPolyEditView::ImpTransformMarkedPoints(PPolyTrFunc pTrFunc, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pObj=pM->GetObj();
        const SdrUShortCont* pPts=pM->GetMarkedPoints();
        Point aPvOfs(pM->GetPageView()->GetOffset());
        ULONG nPtAnz=pPts==NULL ? 0 : pPts->GetCount();
        SdrPathObj* pPath=PTR_CAST(SdrPathObj,pObj);
        if (nPtAnz!=0 && pPath!=NULL) {
            AddUndo(new SdrUndoGeoObj(*pObj));
            XPolyPolygon aXPP(pPath->GetPathPoly());
            BOOL bClosed=pPath->IsClosed();
            for (ULONG nPtNum=0; nPtNum<nPtAnz; nPtNum++) {
                USHORT nPt=pPts->GetObject(nPtNum);
                USHORT nPolyNum,nPointNum;
                if (pPath->TakePolyIdxForHdlNum(nPt,nPolyNum,nPointNum)) {
                    XPolygon& rXP=aXPP[nPolyNum];
                    USHORT nPointAnz=rXP.GetPointCount();
                    Point& rPos=rXP[nPointNum];
                    Point* pC1=NULL;
                    Point* pC2=NULL;
                    if (nPointNum>0 && rXP.IsControl(nPointNum-1)) {
                        pC1=&rXP[nPointNum-1];
                    }
                    if (nPointNum<nPointAnz-1 && rXP.IsControl(nPointNum+1)) {
                        pC2=&rXP[nPointNum+1];
                    }
                    if (bClosed && nPointNum==0 && nPointAnz>=2 && rXP.IsControl(nPointAnz-2)) {
                        pC1=&rXP[nPointAnz-2];
                    }
                    rPos+=aPvOfs;
                    if (pC1!=NULL) *pC1+=aPvOfs;
                    if (pC2!=NULL) *pC2+=aPvOfs;
                    (*pTrFunc)(rPos,pC1,pC2,p1,p2,p3,p4,p5);
                    rPos-=aPvOfs;
                    if (pC1!=NULL) *pC1-=aPvOfs;
                    if (pC2!=NULL) *pC2-=aPvOfs;
                    if (bClosed && nPointNum==0) rXP[nPointAnz-1]=rXP[0];
                }
            }
            pPath->SetPathPoly(aXPP);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpMove(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    MovePoint(rPt,*(const Size*)p1);
    if (pC1!=NULL) MovePoint(*pC1,*(const Size*)p1);
    if (pC2!=NULL) MovePoint(*pC2,*(const Size*)p1);
}

void SdrPolyEditView::MoveMarkedPoints(const Size& rSiz, BOOL bCopy)
{
    bCopy=FALSE; // noch nicht implementiert
    ForceUndirtyMrkPnt();
    XubString aStr(ImpGetResStr(STR_EditMove));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,aMark.GetPointMarkDescription(),SDRREPFUNC_OBJ_MOVE);
    if (bCopy) ImpCopyMarkedPoints();
    ImpTransformMarkedPoints(ImpMove,&rSiz);
    EndUndo();
    AdjustMarkHdl();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpResize(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    ResizePoint(rPt,*(const Point*)p1,*(const Fraction*)p2,*(const Fraction*)p3);
    if (pC1!=NULL) ResizePoint(*pC1,*(const Point*)p1,*(const Fraction*)p2,*(const Fraction*)p3);
    if (pC2!=NULL) ResizePoint(*pC2,*(const Point*)p1,*(const Fraction*)p2,*(const Fraction*)p3);
}

void SdrPolyEditView::ResizeMarkedPoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact, BOOL bCopy)
{
    bCopy=FALSE; // noch nicht implementiert
    ForceUndirtyMrkPnt();
    XubString aStr(ImpGetResStr(STR_EditResize));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,aMark.GetPointMarkDescription(),SDRREPFUNC_OBJ_RESIZE);
    if (bCopy) ImpCopyMarkedPoints();
    ImpTransformMarkedPoints(ImpResize,&rRef,&xFact,&yFact);
    EndUndo();
    AdjustMarkHdl();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpRotate(Point& rPt, Point* pC1, Point* pC2, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    RotatePoint(rPt,*(const Point*)p1,*(const double*)p3,*(const double*)p4);
    if (pC1!=NULL) RotatePoint(*pC1,*(const Point*)p1,*(const double*)p3,*(const double*)p4);
    if (pC2!=NULL) RotatePoint(*pC2,*(const Point*)p1,*(const double*)p3,*(const double*)p4);
}

void SdrPolyEditView::RotateMarkedPoints(const Point& rRef, long nWink, BOOL bCopy)
{
    bCopy=FALSE; // noch nicht implementiert
    ForceUndirtyMrkPnt();
    XubString aStr(ImpGetResStr(STR_EditResize));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,aMark.GetPointMarkDescription(),SDRREPFUNC_OBJ_ROTATE);
    if (bCopy) ImpCopyMarkedPoints();
    double nSin=sin(nWink*nPi180);
    double nCos=cos(nWink*nPi180);
    ImpTransformMarkedPoints(ImpRotate,&rRef,&nWink,&nSin,&nCos);
    EndUndo();
    AdjustMarkHdl();
}

