/*************************************************************************
 *
 *  $RCSfile: svdmrkv.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 11:11:37 $
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
#include "svditext.hxx"
#include "svdview.hxx"
#include "svdpagv.hxx"
#include "svdpage.hxx"
#include "svddrgm1.hxx"
#include "svdio.hxx"
#include "svdibrow.hxx"
#include "svdoole2.hxx"
#include "xgrad.hxx"
#include "xflgrit.hxx"

#include "gradtrns.hxx"
#include "xflftrit.hxx"

#include "dialmgr.hxx"
#include "svdstr.hrc"
#include "svdundo.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@   @@  @@@@  @@@@@  @@  @@  @@ @@ @@ @@@@@ @@   @@
//  @@@ @@@ @@  @@ @@  @@ @@  @@  @@ @@ @@ @@    @@   @@
//  @@@@@@@ @@  @@ @@  @@ @@ @@   @@ @@ @@ @@    @@ @ @@
//  @@@@@@@ @@@@@@ @@@@@  @@@@    @@@@@ @@ @@@@  @@@@@@@
//  @@ @ @@ @@  @@ @@  @@ @@ @@    @@@  @@ @@    @@@@@@@
//  @@   @@ @@  @@ @@  @@ @@  @@   @@@  @@ @@    @@@ @@@
//  @@   @@ @@  @@ @@  @@ @@  @@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::ImpClearVars()
{
    eDragMode=SDRDRAG_MOVE;
    bHdlShown=FALSE;
    bRefHdlShownOnly=FALSE;
    eEditMode=SDREDITMODE_EDIT;
    eEditMode0=SDREDITMODE_EDIT;
    bDesignMode=FALSE;
    bMarking=FALSE;
    bMarkingPoints=FALSE;
    bMarkingGluePoints=FALSE;
    bUnmarking=FALSE;
    pMarkedObj=NULL;
    pMarkedPV=NULL;
    bForceFrameHandles=FALSE;
    bPlusHdlAlways=FALSE;
    nFrameHandlesLimit=50;
    nSpecialCnt=0;
    bInsPolyPoint=FALSE;
    nInsPointNum=0;
    bEdgesOfMarkedNodesDirty=FALSE;
    bMarkedObjRectDirty=FALSE;
    bMarkedPointsRectsDirty=FALSE;
    bHdlHidden=FALSE;
    bMrkPntDirty=FALSE;
    bMarkHdlWhenTextEdit=FALSE;
//    bSolidHdlBackgroundInvalid=FALSE;
    bMarkableObjCountDirty=FALSE; // noch nicht implementiert
    nMarkableObjCount=0;          // noch nicht implementiert
}

SdrMarkView::SdrMarkView(SdrModel* pModel1, OutputDevice* pOut):
    SdrSnapView(pModel1,pOut),
    aHdl(this)
{
    ImpClearVars();
    StartListening(*pModel1);
}

SdrMarkView::SdrMarkView(SdrModel* pModel1, ExtOutputDevice* pXOut):
    SdrSnapView(pModel1,pXOut),
    aHdl(this)
{
    ImpClearVars();
    StartListening(*pModel1);
}

void __EXPORT SdrMarkView::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
    if (pSdrHint!=NULL) {
        SdrHintKind eKind=pSdrHint->GetKind();
        if (eKind==HINT_OBJLISTCLEARED) {
            USHORT nAnz=GetPageViewCount();
            BOOL bMLChgd=FALSE;
            for (USHORT nv=0; nv<nAnz; nv++) {
                SdrPageView* pPV=GetPageViewPvNum(nv);
                if (pPV->GetObjList()==pSdrHint->GetObjList()) {
                    aMark.DeletePageView(*pPV);
                    bMLChgd=TRUE;
                }
            }
            if (bMLChgd) MarkListHasChanged();
        }
        if (eKind==HINT_OBJCHG || eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED) {
            if (bHdlShown) HideMarkHdl(NULL);
            bMarkedObjRectDirty=TRUE;
            bMarkedPointsRectsDirty=TRUE;
        }
    }
    SdrSnapView::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
}

void SdrMarkView::ModelHasChanged()
{
    SdrPaintView::ModelHasChanged();
    aMark.SetNameDirty();
    bMarkedObjRectDirty=TRUE;
    bMarkedPointsRectsDirty=TRUE;
    // Es sind beispielsweise Obj markiert und aMark ist Sorted.
    // In einer anderen View 2 wird die ObjOrder veraendert
    // (z.B. MovToTop()). Dann ist Neusortieren der MarkList erforderlich.
    aMark.SetUnsorted();
    aMark.ForceSort();
    bMrkPntDirty=TRUE;
    UndirtyMrkPnt();
    SdrView* pV=(SdrView*)this;
    if (pV!=NULL && !pV->IsDragObj() && !pV->IsInsObjPoint()) { // an dieser Stelle habe ich ein ziemliches Problem !!!
        AdjustMarkHdl();
//        if (!IsSolidMarkHdl()) {
//            if (!bHdlShown) {
//                // Ein wenig unsauber ...
//                if ((bMarkHdlWhenTextEdit || !pV->IsTextEdit()) &&  // evtl. keine Handles bei TextEdit
//                    !(pV->IsDragHdlHide() && pV->IsDragObj() && // Ggf. keine Handles beim Draggen
//                      aDragStat.IsMinMoved() && !IS_TYPE(SdrDragMovHdl,pV->GetDragMethod()))) {
//                    ShowMarkHdl(NULL);
//                }
//            }
//        }
    }
}

void SdrMarkView::AfterInitRedraw(USHORT nWinNum)
{
//    if (IsShownXorVisibleWinNum(nWinNum)) { // Durch Invalidate zerstoerte Handles wiederherstellen
//        OutputDevice* pOut=GetWin(nWinNum);
//        if (pOut!=NULL && pOut->GetOutDevType()==OUTDEV_WINDOW) {
//            if (bHdlShown && IsSolidMarkHdl()) {
//              if( aWinList[nWinNum].pVDev == NULL )
//                  aWinList[nWinNum].pVDev = new VirtualDevice;
//
//                aHdl.SaveBackground(*pOut,*aWinList[nWinNum].pVDev);
//            }
//        }
//    }
}

void SdrMarkView::ImpAfterPaint()
{
//    if (IsSolidMarkHdl()) {
        SdrView* pV=(SdrView*)this;
        if (pV!=NULL && !pV->IsDragObj() && !pV->IsInsObjPoint()) { // an dieser Stelle habe ich ein ziemliches Problem !!!
            if (!bHdlShown ) {
                // Ein wenig unsauber ...
                if ((bMarkHdlWhenTextEdit || !pV->IsTextEdit()) &&  // evtl. keine Handles bei TextEdit
                    !(pV->IsDragHdlHide() && pV->IsDragObj() && // Ggf. keine Handles beim Draggen
                      aDragStat.IsMinMoved() && !IS_TYPE(SdrDragMovHdl,pV->GetDragMethod()))) {
                    ShowMarkHdl(NULL);
                }
            }
        }
//    }
//  // refresh with Paint-functionality
//  RefreshAllIAOManagers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrMarkView::IsAction() const
{
    return SdrSnapView::IsAction() || bMarking || bMarkingPoints || bMarkingGluePoints;
}

void SdrMarkView::MovAction(const Point& rPnt)
{
    SdrSnapView::MovAction(rPnt);
    if (bMarking) {
        MovMarkObj(rPnt);
    }
    if (bMarkingPoints) {
        MovMarkPoints(rPnt);
    }
    if (bMarkingGluePoints) {
        MovMarkGluePoints(rPnt);
    }
}

void SdrMarkView::EndAction()
{
    if (bMarking) EndMarkObj();
    if (bMarkingPoints) EndMarkPoints();
    if (bMarkingGluePoints) EndMarkGluePoints();
    SdrSnapView::EndAction();
}

void SdrMarkView::BckAction()
{
    SdrSnapView::BckAction();
    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
}

void SdrMarkView::BrkAction()
{
    SdrSnapView::BrkAction();
    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
}

void SdrMarkView::TakeActionRect(Rectangle& rRect) const
{
    if (bMarking || bMarkingPoints || bMarkingGluePoints) {
        rRect=Rectangle(aDragStat.GetStart(),aDragStat.GetNow());
    } else {
        SdrSnapView::TakeActionRect(rRect);
    }
}

void SdrMarkView::ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const
{
    SdrSnapView::ToggleShownXor(pOut,pRegion);
    if ((bMarking || bMarkingPoints || bMarkingGluePoints) && aDragStat.IsShown()) {
        DrawMarkObjOrPoints(pOut);
    }
//    if (bHdlShown) {
//        DrawMarkHdl(pOut,FALSE);
//    } else if (bRefHdlShownOnly) {
//        DrawMarkHdl(pOut,BOOL(2)); HACK(nur die Ref-Hdls painten)
//    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::ClearPageViews()
{
    UnmarkAllObj();
    SdrSnapView::ClearPageViews();
}

void SdrMarkView::HidePage(SdrPageView* pPV)
{
    if(pPV)
    {
        // break all creation actions when hiding page (#75081#)
        BrkAction();

        BOOL bVis(IsMarkHdlShown());

        if(bVis)
            HideMarkHdl(NULL);

        // Alle Markierungen dieser Seite verwerfen
        BOOL bMrkChg(aMark.DeletePageView(*pPV));
        SdrSnapView::HidePage(pPV);

        if(bMrkChg)
        {
            MarkListHasChanged();
            AdjustMarkHdl();
        }

        if(bVis)
            ShowMarkHdl(NULL);
    }
}

void SdrMarkView::SetPagePos(SdrPageView* pPV, const Point& rOffs)
{
    if (pPV!=NULL && rOffs!=pPV->GetOffset()) {
        BOOL bVis=IsMarkHdlShown();
        if (bVis) HideMarkHdl(NULL);
        SdrSnapView::SetPagePos(pPV,rOffs);
        bMarkedObjRectDirty=TRUE;
        bMarkedPointsRectsDirty=TRUE;
        AdjustMarkHdl();
        if (bVis) ShowMarkHdl(NULL);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrMarkView::BegMarkObj(const Point& rPnt, OutputDevice* pOut)
{
    return BegMarkObj(rPnt,(BOOL)FALSE);
}

BOOL SdrMarkView::BegMarkObj(const Point& rPnt, BOOL bUnmark)
{
    BrkAction();
    Point aPt(rPnt);
    aDragStat.Reset(aPt);
    aDragStat.NextPoint();
    aDragStat.SetMinMove(nMinMovLog);
    aAni.Reset();
    pDragWin=NULL;
    bMarking=TRUE;
    bUnmarking=bUnmark;
    nSpecialCnt=0;
    return TRUE;
}

void SdrMarkView::MovMarkObj(const Point& rPnt)
{
    if (bMarking && aDragStat.CheckMinMoved(rPnt)) {
        Point aPt(rPnt);
        HideMarkObjOrPoints(pDragWin);
        aDragStat.NextMove(aPt);
        ShowMarkObjOrPoints(pDragWin);
        long dx=aDragStat.GetNow().X()-aDragStat.GetStart().X();
        long dy=aDragStat.GetNow().Y()-aDragStat.GetStart().Y();
        OutputDevice* pOut=pDragWin;
        if (pOut==NULL) pOut=GetWin(0);
        if (pOut!=NULL) {
            Size aSiz(pOut->LogicToPixel(Size(dx,dy)));
            dx=aSiz.Width();
            dy=aSiz.Height();
        }
        if (nSpecialCnt==0 && dx>=20 && dy>=20) nSpecialCnt++;
        if (nSpecialCnt>0 && nSpecialCnt!=0xFFFF) {
            BOOL bOdd=(nSpecialCnt & 1) ==1;
            if (bOdd && dx>=20 && dy<=2) nSpecialCnt++;
            if (!bOdd && dy>=20 && dx<=2) nSpecialCnt++;
        }
        if (nSpecialCnt>=3) nSpecialCnt=0xFFFF;
    }
}

BOOL SdrMarkView::EndMarkObj()
{
    if (!aDragStat.IsMinMoved()) BrkMarkObj();
    if (bMarking) {
        HideMarkObjOrPoints(pDragWin);
        bMarking=FALSE;
        Rectangle aRect(aDragStat.GetStart(),aDragStat.GetNow());
        aRect.Justify();
        MarkObj(aRect,bUnmarking);
        bUnmarking=FALSE;
        return TRUE;
    }
    return FALSE;
}

void SdrMarkView::BrkMarkObj()
{
    if (bMarking) {
        HideMarkObjOrPoints(pDragWin);
        bMarking=FALSE;
        bUnmarking=FALSE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrMarkView::BegMarkPoints(const Point& rPnt, OutputDevice* pOut)
{
    return BegMarkPoints(rPnt,(BOOL)FALSE);
}

BOOL SdrMarkView::BegMarkPoints(const Point& rPnt, BOOL bUnmark)
{
    if (HasMarkablePoints()) {
        BrkAction();
        Point aPt(rPnt);
        aDragStat.Reset(aPt);
        aDragStat.NextPoint();
        aDragStat.SetMinMove(nMinMovLog);
        aAni.Reset();
        pDragWin=NULL;
        bMarkingPoints=TRUE;
        bUnmarking=bUnmark;
        return TRUE;
    } else return FALSE;
}

void SdrMarkView::MovMarkPoints(const Point& rPnt)
{
    if (bMarkingPoints && aDragStat.CheckMinMoved(rPnt)) {
        HideMarkObjOrPoints(pDragWin);
        aDragStat.NextMove(rPnt);
        ShowMarkObjOrPoints(pDragWin);
    }
}

BOOL SdrMarkView::EndMarkPoints()
{
    if (!HasMarkablePoints() || // Falls waerend des markierens ein Unmark oder so kam
        !aDragStat.IsMinMoved())
    {
        BrkMarkPoints();
    }
    if (bMarkingPoints) {
        HideMarkObjOrPoints(pDragWin);
        bMarkingPoints=FALSE;
        Rectangle aRect(aDragStat.GetStart(),aDragStat.GetNow());
        aRect.Justify();
        MarkPoints(aRect,bUnmarking);
        bUnmarking=FALSE;
        return TRUE;
    }
    return FALSE;
}

void SdrMarkView::BrkMarkPoints()
{
    if (bMarkingPoints) {
        HideMarkObjOrPoints(pDragWin);
        bMarkingPoints=FALSE;
        bUnmarking=FALSE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrMarkView::BegMarkGluePoints(const Point& rPnt, OutputDevice* pOut)
{
    return BegMarkGluePoints(rPnt,(BOOL)FALSE);
}

BOOL SdrMarkView::BegMarkGluePoints(const Point& rPnt, BOOL bUnmark)
{
    if (HasMarkableGluePoints()) {
        BrkAction();
        Point aPt(rPnt);
        aDragStat.Reset(aPt);
        aDragStat.NextPoint();
        aDragStat.SetMinMove(nMinMovLog);
        aAni.Reset();
        pDragWin=NULL;
        bMarkingGluePoints=TRUE;
        bUnmarking=bUnmark;
        return TRUE;
    } else return FALSE;
}

void SdrMarkView::MovMarkGluePoints(const Point& rPnt)
{
    if (bMarkingGluePoints && aDragStat.CheckMinMoved(rPnt)) {
        HideMarkObjOrPoints(pDragWin);
        aDragStat.NextMove(rPnt);
        ShowMarkObjOrPoints(pDragWin);
    }
}

BOOL SdrMarkView::EndMarkGluePoints()
{
    if (!HasMarkableGluePoints() || // Falls waerend des markierens ein Unmark oder so kam
        !aDragStat.IsMinMoved())
    {
        BrkMarkGluePoints();
    }
    if (bMarkingGluePoints) {
        HideMarkObjOrPoints(pDragWin);
        bMarkingGluePoints=FALSE;
        Rectangle aRect(aDragStat.GetStart(),aDragStat.GetNow());
        aRect.Justify();
        MarkGluePoints(&aRect,bUnmarking);
        bUnmarking=FALSE;
        return TRUE;
    }
    return FALSE;
}

void SdrMarkView::BrkMarkGluePoints()
{
    if (bMarkingGluePoints) {
        HideMarkObjOrPoints(pDragWin);
        bMarkingGluePoints=FALSE;
        bUnmarking=FALSE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::DrawMarkObjOrPoints(OutputDevice* pOut) const
{
    if (bMarking || bMarkingPoints || bMarkingGluePoints) {
        Point aPt1(aDragStat.GetStart());
        Point aPt2(aDragStat.GetNow());
        aAni.SetP1(aPt1);
        aAni.SetP2(aPt2);
        aAni.Invert(pOut);
    }
}

void SdrMarkView::ShowMarkObjOrPoints(OutputDevice* pOut)
{
    if ((bMarking || bMarkingPoints || bMarkingGluePoints) && !aDragStat.IsShown()) {
        DrawMarkObjOrPoints(pOut);
        aDragStat.SetShown(TRUE);
        aAni.Start();
    }
}

void SdrMarkView::HideMarkObjOrPoints(OutputDevice* pOut)
{
    if ((bMarking || bMarkingPoints || bMarkingGluePoints) && aDragStat.IsShown()) {
        aAni.Stop();
        DrawMarkObjOrPoints(pOut);
        aDragStat.SetShown(FALSE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrMarkView::HasMarkableObj() const
{
    ULONG nCount=0;
    ULONG nObjCnt=0;
    USHORT nPvCnt=GetPageViewCount();
    for (USHORT nPvNum=0; nPvNum<nPvCnt && nCount==0; nPvNum++) {
        SdrPageView* pPV=GetPageViewPvNum(nPvNum);
        SdrObjList* pOL=pPV->GetObjList();
        ULONG nObjAnz=pOL->GetObjCount();
        for (ULONG nObjNum=0; nObjNum<nObjAnz && nCount==0; nObjNum++) {
            SdrObject* pObj=pOL->GetObj(nObjNum);
            if (IsObjMarkable(pObj,pPV)) {
                nCount++;
            }
        }
    }
    return nCount!=0;
}

ULONG SdrMarkView::GetMarkableObjCount() const
{
    ULONG nCount=0;
    ULONG nObjCnt=0;
    USHORT nPvCnt=GetPageViewCount();
    for (USHORT nPvNum=0; nPvNum<nPvCnt; nPvNum++) {
        SdrPageView* pPV=GetPageViewPvNum(nPvNum);
        SdrObjList* pOL=pPV->GetObjList();
        ULONG nObjAnz=pOL->GetObjCount();
        for (ULONG nObjNum=0; nObjNum<nObjAnz; nObjNum++) {
            SdrObject* pObj=pOL->GetObj(nObjNum);
            if (IsObjMarkable(pObj,pPV)) {
                nCount++;
            }
        }
    }
    return nCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

//void SdrMarkView::DrawMarkHdl(OutputDevice* pOut, BOOL bNoRefHdl) const
//{
//    if (bHdlHidden) return;
////    if (IsSolidMarkHdl())
//      bNoRefHdl=FALSE; // geht leider erstmal nicht anders
//    BOOL bOnlyRefs=USHORT(bNoRefHdl)==2; HACK(nur die Ref-Hdls painten)
//    USHORT nWinAnz=GetWinCount();
//    USHORT nWinNum=0;
//    do {
//        OutputDevice* pO=pOut;
//        if (pO==NULL) {
//            pO=GetWin(nWinNum);
//            nWinNum++;
//        }
//        if (pO!=NULL) {
//            if (!bInsPolyPoint && !bNoRefHdl && !bOnlyRefs) {
//                aHdl.DrawAll(*pO);
//            } else {
//                ULONG nHdlAnz=aHdl.GetHdlCount();
//                for (ULONG nHdlNum=0; nHdlNum<nHdlAnz; nHdlNum++) {
//                    SdrHdl* pHdl=aHdl.GetHdl(nHdlNum);
//                    SdrHdlKind eKind=pHdl->GetKind();
//                    USHORT nPtNum=pHdl->GetObjHdlNum();
//                    const SdrObject* pObj=pHdl->GetObj();
//                    if ((!bInsPolyPoint || nPtNum!=nInsPointNum || pObj==NULL || pObj!=pMarkedObj) &&
//                        (!bNoRefHdl || (eKind!=HDL_REF1 && eKind!=HDL_REF2 && eKind!=HDL_MIRX))!=bOnlyRefs) {
//                        pHdl->Draw(*pO);
//                    }
//                }
//            }
//        }
//    } while (pOut==NULL && nWinNum<nWinAnz);
//}

void SdrMarkView::ImpShowMarkHdl(OutputDevice* pOut, const Region* pRegion, BOOL bNoRefHdl)
{
//    if (IsSolidMarkHdl())
        bNoRefHdl=FALSE; // geht leider erstmal nicht anders
    if (!bHdlShown) {
//        if (aHdl.IsSolidHdl()) {
//            USHORT nAnz=pOut==NULL ? aWinList.GetCount() : 1;
//            for (USHORT i=0; i<nAnz; i++) {
//                USHORT nWinNum=pOut==NULL ? i : aWinList.Find(pOut);
//                if (nWinNum!=SDRVIEWWIN_NOTFOUND) {
//                    if (aWinList[nWinNum].pVDev==NULL) {
//                        aWinList[nWinNum].pVDev=new VirtualDevice;
//                    }
//                    aHdl.SaveBackground(*aWinList[nWinNum].pWin,*aWinList[nWinNum].pVDev,pRegion);
//                    bSolidHdlBackgroundInvalid=FALSE;
//                }
//            }
//        }
//        DrawMarkHdl(pOut,bRefHdlShownOnly);
        bRefHdlShownOnly=FALSE;
        bHdlShown=TRUE;

        // refresh IAOs
//      RefreshAllIAOManagers();
    }
}

void SdrMarkView::ShowMarkHdl(OutputDevice* pOut, BOOL bNoRefHdl)
{
//    if (IsSolidMarkHdl())
        bNoRefHdl=FALSE; // geht leider erstmal nicht anders
    ImpShowMarkHdl(pOut,NULL,bNoRefHdl);

    // refresh IAOs
    RefreshAllIAOManagers();
}


void SdrMarkView::HideMarkHdl(OutputDevice* pOut, BOOL bNoRefHdl)
{
//    if (IsSolidMarkHdl())
        bNoRefHdl=FALSE; // geht leider erstmal nicht anders
    if (bHdlShown) {
        if (!bHdlHidden) { // #37331#
            // Optimierung geht nicht, weil diverse Handles trotz SolidHdl doch noch XOR gapainted werden
//          DrawMarkHdl(pOut,bNoRefHdl);
//            if (aHdl.IsSolidHdl()) {
//                BOOL bInvalidate=IsMarkHdlBackgroundInvalid();
//                USHORT nAnz=pOut==NULL ? aWinList.GetCount() : 1;
//                for (USHORT i=0; i<nAnz; i++) {
//                    USHORT nWinNum=pOut==NULL ? i : aWinList.Find(pOut);
//                    if (nWinNum!=SDRVIEWWIN_NOTFOUND) {
//                        SdrViewWinRec& rWRec=GetWinRec(nWinNum);
//                        if (rWRec.pVDev!=NULL) {
//                            OutputDevice* pOut=rWRec.pWin;
//                            if (bInvalidate) { // fuer den Writer in einigen Faellen Invalidieren
//                                if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
//                                    aHdl.Invalidate(*(Window*)pOut);
//                                }
//                            } else {
//                                if (bNoRefHdl) {
//                                    ULONG nHdlAnz=aHdl.GetHdlCount();
//                                    for (ULONG nHdlNum=0; nHdlNum<nHdlAnz; nHdlNum++) {
//                                        SdrHdl* pHdl=aHdl.GetHdl(nHdlNum);
//                                        SdrHdlKind eKind=pHdl->GetKind();
//                                        USHORT nPtNum=pHdl->GetObjHdlNum();
//                                        if (eKind!=HDL_REF1 && eKind!=HDL_REF2 && eKind!=HDL_MIRX) {
//                                            aHdl.RestoreBackground(*pOut,*rWRec.pVDev,(USHORT)nHdlNum);
//                                        }
//                                    }
//                                } else {
//                                    aHdl.RestoreBackground(*pOut,*rWRec.pVDev);
//                                }
//                            }
//                        }
//                    }
//                }
//            }
        }
        bRefHdlShownOnly=bNoRefHdl;
        bHdlShown=FALSE;
    }
//    bSolidHdlBackgroundInvalid=FALSE;

    // refresh IAOs
//  RefreshAllIAOManagers();
}

void SdrMarkView::SetMarkHdlHidden(BOOL bOn)
{
    if(bOn != bHdlHidden)
    {
        // one hide in which the old flag value is still set
        HideMarkHdl(NULL);

        // remember new value
        bHdlHidden = bOn;

        // create all IAOHandles new (depends on IsMarkHdlHidden())
        AdjustMarkHdl(TRUE);

        // show new state
        RefreshAllIAOManagers();

//      BOOL bHdlShownMerk=bHdlShown;
//      bHdlHidden=FALSE;
//      if (bOn) { // Modus anschalten, also Handles ggf aus.
//          if (bHdlShown) HideMarkHdl(NULL);
//      } else { // Ansonsten Modus ausschalten, also Handles ggf. einblenden
//          if (bHdlShown) { bHdlShown=FALSE; ShowMarkHdl(NULL); }
//      }
//      bHdlShown=bHdlShownMerk;
//      bHdlHidden=bOn;
    }
}

BOOL SdrMarkView::ImpIsFrameHandles() const
{
    ULONG nMarkAnz=aMark.GetMarkCount();
    BOOL bFrmHdl=nMarkAnz>nFrameHandlesLimit || bForceFrameHandles;
    BOOL bStdDrag=eDragMode==SDRDRAG_MOVE;
    if (nMarkAnz==1 && bStdDrag && bFrmHdl) {
        const SdrObject* pObj=aMark.GetMark(0)->GetObj();
        if (pObj->GetObjInventor()==SdrInventor) {
            UINT16 nIdent=pObj->GetObjIdentifier();
            if (nIdent==OBJ_LINE || nIdent==OBJ_EDGE || nIdent==OBJ_CAPTION || nIdent==OBJ_MEASURE) {
                bFrmHdl=FALSE;
            }
        }
    }
    if (!bStdDrag && !bFrmHdl) {
        // Grundsaetzlich erstmal alle anderen Dragmodi nur mit FrameHandles
        bFrmHdl=TRUE;
        if (eDragMode==SDRDRAG_ROTATE) {
            // bei Rotate ObjOwn-Drag, wenn mind. 1 PolyObj
            for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && bFrmHdl; nMarkNum++) {
                const SdrMark* pM=aMark.GetMark(nMarkNum);
                const SdrObject* pObj=pM->GetObj();
                bFrmHdl=!pObj->IsPolyObj();
            }
        }
    }
    if (!bFrmHdl) {
        // FrameHandles, wenn wenigstens 1 Obj kein SpecialDrag kann
        for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bFrmHdl; nMarkNum++) {
            const SdrMark* pM=aMark.GetMark(nMarkNum);
            const SdrObject* pObj=pM->GetObj();
            bFrmHdl=!pObj->HasSpecialDrag();
        }
    }
    return bFrmHdl;
}

void SdrMarkView::SetMarkHandles()
{
    aHdl.Clear();
    aHdl.SetRotateShear(eDragMode==SDRDRAG_ROTATE);
    aHdl.SetDistortShear(eDragMode==SDRDRAG_SHEAR);
    pMarkedObj=NULL;
    pMarkedPV=NULL;
    ULONG nMarkAnz=aMark.GetMarkCount();
    BOOL bStdDrag=eDragMode==SDRDRAG_MOVE;
    if (nMarkAnz==1) {
        pMarkedObj=aMark.GetMark(0)->GetObj();
    }
    BOOL bFrmHdl=ImpIsFrameHandles();
    if (nMarkAnz>0) {
        pMarkedPV=aMark.GetMark(0)->GetPageView();
        for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && (pMarkedPV!=NULL || !bFrmHdl); nMarkNum++) {
            const SdrMark* pM=aMark.GetMark(nMarkNum);
            if (pMarkedPV!=pM->GetPageView()) {
                pMarkedPV=NULL;
            }
        }
    }
    if (bFrmHdl) {
        Rectangle aRect(GetMarkedObjRect());
        if (!aRect.IsEmpty()) { // sonst nix gefunden
            BOOL bWdt0=aRect.Left()==aRect.Right();
            BOOL bHgt0=aRect.Top()==aRect.Bottom();
            if (bWdt0 && bHgt0) {
                aHdl.AddHdl(new SdrHdl(aRect.TopLeft(),HDL_UPLFT));
            } else if (!bStdDrag && (bWdt0 || bHgt0)) {
                aHdl.AddHdl(new SdrHdl(aRect.TopLeft()    ,HDL_UPLFT));
                aHdl.AddHdl(new SdrHdl(aRect.BottomRight(),HDL_LWRGT));
            } else {
                if (!bWdt0 && !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.TopLeft()     ,HDL_UPLFT));
                if (          !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.TopCenter()   ,HDL_UPPER));
                if (!bWdt0 && !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.TopRight()    ,HDL_UPRGT));
                if (!bWdt0          ) aHdl.AddHdl(new SdrHdl(aRect.LeftCenter()  ,HDL_LEFT ));
                if (!bWdt0          ) aHdl.AddHdl(new SdrHdl(aRect.RightCenter() ,HDL_RIGHT));
                if (!bWdt0 && !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.BottomLeft()  ,HDL_LWLFT));
                if (          !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.BottomCenter(),HDL_LOWER));
                if (!bWdt0 && !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.BottomRight() ,HDL_LWRGT));
            }
        }
    } else {
        for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
            const SdrMark* pM=aMark.GetMark(nMarkNum);
            SdrObject* pObj=pM->GetObj();
            SdrPageView* pPV=pM->GetPageView();
            ULONG nSiz0=aHdl.GetHdlCount();
            pObj->AddToHdlList(aHdl);
            ULONG nSiz1=aHdl.GetHdlCount();
            BOOL bPoly=pObj->IsPolyObj();
            const SdrUShortCont* pMrkPnts=pM->GetMarkedPoints();
            for (ULONG i=nSiz0; i<nSiz1; i++) {
                SdrHdl* pHdl=aHdl.GetHdl(i);
                pHdl->SetObj(pObj);
                pHdl->SetPageView(pPV);
                pHdl->SetObjHdlNum(USHORT(i-nSiz0));
                if (bPoly) {
                    BOOL bSelected=pMrkPnts!=NULL && pMrkPnts->Exist(USHORT(i-nSiz0));
                    pHdl->SetSelected(bSelected);
                    BOOL bPlus=bPlusHdlAlways;
                    if (bPlusHdlAlways || bSelected) {
                        USHORT nPlusAnz=pObj->GetPlusHdlCount(*pHdl);
                        for (USHORT nPlusNum=0; nPlusNum<nPlusAnz; nPlusNum++) {
                            SdrHdl* pPlusHdl=pObj->GetPlusHdl(*pHdl,nPlusNum);
                            if (pPlusHdl!=NULL) {
                                pPlusHdl->SetObj(pObj);
                                pPlusHdl->SetPageView(pPV);
                                pPlusHdl->SetPlusHdl(TRUE);
                                aHdl.AddHdl(pPlusHdl);
                            }
                        }
                    }
                }
            }
        } // for nMarkNum
    } // if bFrmHdl else
    // GluePoint-Handles
    for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
        const SdrMark* pM=aMark.GetMark(nMarkNum);
        SdrObject* pObj=pM->GetObj();
        SdrPageView* pPV=pM->GetPageView();
        const SdrUShortCont* pMrkGlue=pM->GetMarkedGluePoints();
        if (pMrkGlue!=NULL) {
            const SdrGluePointList* pGPL=pObj->GetGluePointList();
            if (pGPL!=NULL) {
                USHORT nGlueAnz=pGPL->GetCount();
                USHORT nAnz=(USHORT)pMrkGlue->GetCount();
                for (USHORT nNum=0; nNum<nAnz; nNum++) {
                    USHORT nId=pMrkGlue->GetObject(nNum);
                    USHORT nNum=pGPL->FindGluePoint(nId);
                    if (nNum!=SDRGLUEPOINT_NOTFOUND) {
                        const SdrGluePoint& rGP=(*pGPL)[nNum];
                        Point aPos(rGP.GetAbsolutePos(*pObj));
                        SdrHdl* pGlueHdl=new SdrHdl(aPos,HDL_GLUE);
                        pGlueHdl->SetObj(pObj);
                        pGlueHdl->SetPageView(pPV);
                        pGlueHdl->SetObjHdlNum(nId);
                        aHdl.AddHdl(pGlueHdl);
                    }
                }
            }
        }
    }

    // Drehpunkt/Spiegelachse
    AddDragModeHdl(eDragMode);

    // add custom handles (used by other apps, e.g. AnchorPos)
    AddCustomHdl();

    // sort handles
    aHdl.Sort();
}

void SdrMarkView::AddCustomHdl()
{
    // add custom handles (used by other apps, e.g. AnchorPos)
}

void SdrMarkView::SetDragMode(SdrDragMode eMode)
{
    SdrDragMode eMode0=eDragMode;
    eDragMode=eMode;
    if (eDragMode==SDRDRAG_RESIZE) eDragMode=SDRDRAG_MOVE;
    if (eDragMode!=eMode0) {
        BOOL bVis=IsMarkHdlShown();
        if (bVis) HideMarkHdl(NULL);
        ForceRefToMarked();
        SetMarkHandles();
        if (bVis) ShowMarkHdl(NULL);
        {
            if (HasMarkedObj()) MarkListHasChanged();
        }
    }
}

void SdrMarkView::AddDragModeHdl(SdrDragMode eMode)
{
    switch(eMode)
    {
        case SDRDRAG_ROTATE:
        {
            // add rotation center
            SdrHdl* pHdl = new SdrHdl(aRef1, HDL_REF1);

            aHdl.AddHdl(pHdl);

            break;
        }
        case SDRDRAG_MIRROR:
        {
            // add mirror axis
            SdrHdl* pHdl3 = new SdrHdl(aRef2, HDL_REF2);
            SdrHdl* pHdl2 = new SdrHdl(aRef1, HDL_REF1);
            SdrHdl* pHdl1 = new SdrHdlLine(*pHdl2, *pHdl3, HDL_MIRX);

            pHdl1->SetObjHdlNum(1); // fuer Sortierung
            pHdl2->SetObjHdlNum(2); // fuer Sortierung
            pHdl3->SetObjHdlNum(3); // fuer Sortierung

            aHdl.AddHdl(pHdl1); // Linie als erstes, damit als letztes im HitTest
            aHdl.AddHdl(pHdl2);
            aHdl.AddHdl(pHdl3);

            break;
        }
        case SDRDRAG_TRANSPARENCE:
        {
            // add interactive transparence handle
            UINT32 nMarkAnz = aMark.GetMarkCount();
            if(nMarkAnz == 1)
            {
                SdrObject* pObj = aMark.GetMark(0)->GetObj();
                SdrModel* pModel = GetModel();
//-/                SfxItemSet aSet(pModel->GetItemPool(), XATTR_FILLFLOATTRANSPARENCE, XATTR_FILLFLOATTRANSPARENCE, 0, 0);
//-/                pObj->TakeAttributes(aSet, TRUE, FALSE);
                const SfxItemSet& rSet = pObj->GetItemSet();
                const SfxPoolItem* pPoolItem;
                SfxItemState eState = rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, FALSE, &pPoolItem);

                if(SFX_ITEM_SET == eState)
                {
                    // activate if not yet done
                    if(!((XFillFloatTransparenceItem*)pPoolItem)->IsEnabled())
                    {
                        XFillFloatTransparenceItem aNewItem(*((XFillFloatTransparenceItem*)pPoolItem));
                        XGradient aGrad = aNewItem.GetValue();

                        aNewItem.SetEnabled(TRUE);
                        aGrad.SetStartIntens(100);
                        aGrad.SetEndIntens(100);
                        aNewItem.SetValue(aGrad);
//-/                        aSet.Put(aNewItem);

                        // add undo to allow user to take back this step
                        pModel->BegUndo(SVX_RESSTR(SIP_XA_FILLTRANSPARENCE));
                        pModel->AddUndo(new SdrUndoAttrObj(*pObj));
                        pModel->EndUndo();

                        pObj->SetItem(aNewItem);
//-/                        pObj->SetAttributes(aSet, FALSE);
                    }

                    // set values and transform to vector set
                    GradTransformer aGradTransformer;
                    GradTransVector aGradTransVector;
                    GradTransGradient aGradTransGradient;

                    aGradTransGradient.aGradient = ((XFillFloatTransparenceItem&)rSet.Get(XATTR_FILLFLOATTRANSPARENCE)).GetValue();
                    aGradTransformer.GradToVec(aGradTransGradient, aGradTransVector, pObj);

                    // build handles
                    SdrHdlColor* pColHdl1 = new SdrHdlColor(aGradTransVector.aPos1, aGradTransVector.aCol1, SDR_HANDLE_COLOR_SIZE_NORMAL, TRUE);
                    SdrHdlColor* pColHdl2 = new SdrHdlColor(aGradTransVector.aPos2, aGradTransVector.aCol2, SDR_HANDLE_COLOR_SIZE_NORMAL, TRUE);
                    SdrHdlGradient* pGradHdl = new SdrHdlGradient(aGradTransVector.aPos1, aGradTransVector.aPos2, FALSE);
                    DBG_ASSERT(pColHdl1 && pColHdl2 && pGradHdl, "Got not all necessary handles!!");

                    // link them
                    pGradHdl->SetColorHandles(pColHdl1, pColHdl2);
                    pGradHdl->SetObj(pObj);
                    pColHdl1->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));
                    pColHdl2->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));

                    // insert them
                    aHdl.AddHdl(pColHdl1);
                    aHdl.AddHdl(pColHdl2);
                    aHdl.AddHdl(pGradHdl);
                }
            }
            break;
        }
        case SDRDRAG_GRADIENT:
        {
            // add interactive gradient handle
            UINT32 nMarkAnz = aMark.GetMarkCount();
            if(nMarkAnz == 1)
            {
                SdrObject* pObj = aMark.GetMark(0)->GetObj();
//-/                SfxItemSet aSet(GetModel()->GetItemPool());
//-/                pObj->TakeAttributes(aSet, FALSE, FALSE);
                const SfxItemSet& rSet = pObj->GetItemSet();
                XFillStyle eFillStyle = ((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();

                if(eFillStyle == XFILL_GRADIENT)
                {
                    // set values and transform to vector set
                    GradTransformer aGradTransformer;
                    GradTransVector aGradTransVector;
                    GradTransGradient aGradTransGradient;
                    Size aHdlSize(15, 15);

                    aGradTransGradient.aGradient = ((XFillGradientItem&)rSet.Get(XATTR_FILLGRADIENT)).GetValue();
                    aGradTransformer.GradToVec(aGradTransGradient, aGradTransVector, pObj);

                    // build handles
                    SdrHdlColor* pColHdl1 = new SdrHdlColor(aGradTransVector.aPos1, aGradTransVector.aCol1, aHdlSize, FALSE);
                    SdrHdlColor* pColHdl2 = new SdrHdlColor(aGradTransVector.aPos2, aGradTransVector.aCol2, aHdlSize, FALSE);
                    SdrHdlGradient* pGradHdl = new SdrHdlGradient(aGradTransVector.aPos1, aGradTransVector.aPos2, TRUE);
                    DBG_ASSERT(pColHdl1 && pColHdl2 && pGradHdl, "Got not all necessary handles!!");

                    // link them
                    pGradHdl->SetColorHandles(pColHdl1, pColHdl2);
                    pGradHdl->SetObj(pObj);
                    pColHdl1->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));
                    pColHdl2->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));

                    // insert them
                    aHdl.AddHdl(pColHdl1);
                    aHdl.AddHdl(pColHdl2);
                    aHdl.AddHdl(pGradHdl);
                }
            }
            break;
        }
    }
}

void SdrMarkView::ForceRefToMarked()
{
    switch(eDragMode)
    {
        case SDRDRAG_ROTATE:
        {
            Rectangle aR(GetMarkedObjRect());
            aRef1 = aR.Center();

            break;
        }

        case SDRDRAG_MIRROR:
        {
            // Erstmal die laenge der Spiegelachsenlinie berechnen
            long nOutMin=0;
            long nOutMax=0;
            long nMinLen=0;
            long nObjDst=0;
            long nOutHgt=0;
            OutputDevice* pOut=GetWin(0);
            if (pOut!=NULL) {
                // Mindestlaenge 50 Pixel
                nMinLen=pOut->PixelToLogic(Size(0,50)).Height();
                // 20 Pixel fuer RefPt-Abstand vom Obj
                nObjDst=pOut->PixelToLogic(Size(0,20)).Height();
                // MinY/MaxY
                // Abstand zum Rand = Mindestlaenge = 10 Pixel
                long nDst=pOut->PixelToLogic(Size(0,10)).Height();
                nOutMin=-pOut->GetMapMode().GetOrigin().Y();
                nOutMax=pOut->GetOutputSize().Height()-1+nOutMin;
                nOutMin+=nDst;
                nOutMax-=nDst;
                // Absolute Mindestlaenge jedoch 10 Pixel
                if (nOutMax-nOutMin<nDst) {
                    nOutMin+=nOutMax+1;
                    nOutMin/=2;
                    nOutMin-=(nDst+1)/2;
                    nOutMax=nOutMin+nDst;
                }
                nOutHgt=nOutMax-nOutMin;
                // Sonst Mindestlaenge = 1/4 OutHgt
                long nTemp=nOutHgt/4;
                if (nTemp>nMinLen) nMinLen=nTemp;
            }

            Rectangle aR(GetMarkedObjBoundRect());
            Point aCenter(aR.Center());
            long nMarkHgt=aR.GetHeight()-1;
            long nHgt=nMarkHgt+nObjDst*2;       // 20 Pixel obej und unten ueberstehend
            if (nHgt<nMinLen) nHgt=nMinLen;     // Mindestlaenge 50 Pixel bzw. 1/4 OutHgt

            long nY1=aCenter.Y()-(nHgt+1)/2;
            long nY2=nY1+nHgt;

            if (pOut!=NULL && nMinLen>nOutHgt) nMinLen=nOutHgt; // evtl. noch etwas verkuerzen

            if (pOut!=NULL) { // nun vollstaendig in den sichtbaren Bereich schieben
                if (nY1<nOutMin) {
                    nY1=nOutMin;
                    if (nY2<nY1+nMinLen) nY2=nY1+nMinLen;
                }
                if (nY2>nOutMax) {
                    nY2=nOutMax;
                    if (nY1>nY2-nMinLen) nY1=nY2-nMinLen;
                }
            }

            aRef1.X()=aCenter.X();
            aRef1.Y()=nY1;
            aRef2.X()=aCenter.X();
            aRef2.Y()=nY2;

            break;
        }

        case SDRDRAG_TRANSPARENCE:
        case SDRDRAG_GRADIENT:
        {
            Rectangle aRect(GetMarkedObjBoundRect());
            aRef1 = aRect.TopLeft();
            aRef2 = aRect.BottomRight();
            break;
        }
    }
}

void SdrMarkView::SetRef1(const Point& rPt)
{
    if(eDragMode == SDRDRAG_ROTATE || eDragMode == SDRDRAG_MIRROR)
    {
        aRef1 = rPt;
        SdrHdl* pH = aHdl.GetHdl(HDL_REF1);
        if(pH)
            pH->SetPos(rPt);
        ShowMarkHdl(NULL);
    }
}

void SdrMarkView::SetRef2(const Point& rPt)
{
    if(eDragMode == SDRDRAG_MIRROR)
    {
        aRef2 = rPt;
        SdrHdl* pH = aHdl.GetHdl(HDL_REF2);
        if(pH)
            pH->SetPos(rPt);
        ShowMarkHdl(NULL);
    }
}

void SdrMarkView::CheckMarked()
{
    for (ULONG nm=aMark.GetMarkCount(); nm>0;) {
        nm--;
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pObj=pM->GetObj();
        SdrPageView* pPV=pM->GetPageView();
        SdrLayerID nLay=pObj->GetLayer();
        BOOL bRaus=!pObj->IsInserted(); // Obj geloescht?
        if (!pObj->Is3DObj()) {
            bRaus=bRaus || pObj->GetPage()!=pPV->GetPage();   // Obj ploetzlich in anderer Page oder Group
        }
        bRaus=bRaus || pPV->GetLockedLayers().IsSet(nLay) ||  // Layer gesperrt?
                       !pPV->GetVisibleLayers().IsSet(nLay);  // Layer nicht sichtbar?
        if (!bRaus) {
            // Joe am 9.3.1997: Gruppierte Objekten koennen nun auch
            // markiert werden. Nach EnterGroup muessen aber die Objekte
            // der hoeheren Ebene deselektiert werden.
            const SdrObjList* pOOL=pObj->GetObjList();
            const SdrObjList* pVOL=pPV->GetObjList();
            while (pOOL!=NULL && pOOL!=pVOL) {
                pOOL=pOOL->GetUpList();
            }
            bRaus=pOOL!=pVOL;
        }

        if (bRaus) {
            aMark.DeleteMarkNum(nm);
        } else {
            if (!IsGluePointEditMode()) { // Markierte GluePoints nur im GlueEditMode
                SdrUShortCont* pPts=pM->GetMarkedGluePoints();
                if (pPts!=NULL && pPts->GetCount()!=0) {
                    pPts->Clear();
                }
            }
        }
    }

    // #67670# When this leads to a change, MarkListHasChanged()
    // had been called before. Calling MarkListHasChanged() again
    // could lead to problems in sfx, see BUG description.
    //  if(bChg)
    //      MarkListHasChanged();
}

// for SW and their rearranged painting; this method forces the
// actual IAO-Handles to throw away saved contents
void SdrMarkView::ForceInvalidateMarkHandles()
{
    // at the moment, do this using SetMarkHandles()
    SetMarkHandles();
}

void SdrMarkView::SetMarkRects()
{
    for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
        SdrPageView* pPV=GetPageViewPvNum(nv);
        pPV->SetHasMarkedObj(aMark.TakeSnapRect(pPV,pPV->MarkSnap()));
        aMark.TakeBoundRect(pPV,pPV->MarkBound());
    }
}

void SdrMarkView::SetFrameHandles(BOOL bOn)
{
    if (bOn!=bForceFrameHandles) {
        BOOL bOld=ImpIsFrameHandles();
        bForceFrameHandles=bOn;
        BOOL bNew=ImpIsFrameHandles();
        if (bNew!=bOld) {
            AdjustMarkHdl(TRUE);
            MarkListHasChanged();
        }
    }
}

void SdrMarkView::SetEditMode(SdrViewEditMode eMode)
{
    if (eMode!=eEditMode) {
        BOOL bGlue0=eEditMode==SDREDITMODE_GLUEPOINTEDIT;
        BOOL bEdge0=((SdrCreateView*)this)->IsEdgeTool();
        eEditMode0=eEditMode;
        eEditMode=eMode;
        BOOL bGlue1=eEditMode==SDREDITMODE_GLUEPOINTEDIT;
        BOOL bEdge1=((SdrCreateView*)this)->IsEdgeTool();
        // etwas Aufwand um Flackern zu verhindern beim Umschalten
        // zwischen GlueEdit und EdgeTool
        if (bGlue1 && !bGlue0) ImpSetGlueVisible2(bGlue1);
        if (bEdge1!=bEdge0) ImpSetGlueVisible3(bEdge1);
        if (!bGlue1 && bGlue0) ImpSetGlueVisible2(bGlue1);
        if (bGlue0 && !bGlue1) UnmarkAllGluePoints();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrMarkView::IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const
{
    if (pObj)
    {
        if (pObj->IsMarkProtect() ||
            (!bDesignMode && pObj->IsUnoObj()))
        {
            // Objekt nicht selektierbar oder
            // SdrUnoObj nicht im DesignMode
            return FALSE;
        }
    }
    return pPV!=NULL ? pPV->IsObjMarkable(pObj) : TRUE;
}

BOOL SdrMarkView::IsMarkedObjHit(const Point& rPnt, short nTol) const
{
    BOOL bRet=FALSE;
    nTol=ImpGetHitTolLogic(nTol,NULL);
    Point aPt(rPnt);
    for (ULONG nm=0; nm<aMark.GetMarkCount() && !bRet; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        bRet=ImpIsObjHit(aPt,USHORT(nTol),pM->GetObj(),pM->GetPageView(),0);
    }
    return bRet;
}

SdrHdl* SdrMarkView::PickHandle(const Point& rPnt, const OutputDevice& rOut, ULONG nOptions, SdrHdl* pHdl0) const
{
    if (&rOut==NULL) return FALSE;
    if (bSomeObjChgdFlag) { // ggf. Handles neu berechnen lassen!
        FlushComeBackTimer();
    }
    BOOL bBack=(nOptions & SDRSEARCH_BACKWARD) !=0;
    BOOL bNext=(nOptions & SDRSEARCH_NEXT) !=0;
    Point aPt(rPnt);
    return aHdl.HitTest(aPt,rOut,bBack,bNext,pHdl0);
}

BOOL SdrMarkView::MarkObj(const Point& rPnt, short nTol, BOOL bToggle, BOOL bDeep)
{
    SdrObject* pObj;
    SdrPageView* pPV;
    nTol=ImpGetHitTolLogic(nTol,NULL);
    ULONG nOptions=SDRSEARCH_PICKMARKABLE;
    if (bDeep) nOptions=nOptions|SDRSEARCH_DEEP;
    BOOL bRet=PickObj(rPnt,(USHORT)nTol,pObj,pPV,nOptions);
    if (bRet) {
        BOOL bUnmark=bToggle && IsObjMarked(pObj);
        MarkObj(pObj,pPV,bUnmark);
    }
    return bRet;
}

BOOL SdrMarkView::MarkNextObj(BOOL bPrev)
{
    USHORT nPvAnz=GetPageViewCount();
    if (nPvAnz==0) return FALSE;
    USHORT nMaxPV=USHORT(nPvAnz-1);
    aMark.ForceSort();
    ULONG  nMarkAnz=aMark.GetMarkCount();
    ULONG  nChgMarkNum=0xFFFFFFFF; // Nummer des zu ersetzenden MarkEntries
    USHORT nSearchPvNum=bPrev ? 0 : nMaxPV;
    ULONG  nSearchObjNum=bPrev ? 0 : 0xFFFFFFFF;
    if (nMarkAnz!=0) {
        nChgMarkNum=bPrev ? 0 : ULONG(nMarkAnz-1);
        SdrMark* pM=aMark.GetMark(nChgMarkNum);
        nSearchObjNum=pM->GetObj()->GetOrdNum();
        nSearchPvNum=GetPageViewNum(pM->GetPageView());
    }
    SdrObject* pMarkObj=NULL;
    SdrPageView* pMarkPV=NULL;
    while (pMarkObj==NULL) {
        SdrPageView* pSearchPV=GetPageViewPvNum(nSearchPvNum);
        SdrObjList* pSearchObjList=pSearchPV->GetObjList();
        ULONG nObjAnz=pSearchObjList->GetObjCount();
        if (nObjAnz!=0) {
            if (nSearchObjNum>nObjAnz) nSearchObjNum=nObjAnz;
            while (pMarkObj==NULL && ((!bPrev && nSearchObjNum>0) || (bPrev && nSearchObjNum<nObjAnz))) {
                if (!bPrev) nSearchObjNum--;
                SdrObject* pSearchObj=pSearchObjList->GetObj(nSearchObjNum);
                if (IsObjMarkable(pSearchObj,pSearchPV)) {
                    if (aMark.FindObject(pSearchObj)==CONTAINER_ENTRY_NOTFOUND) {
                        pMarkObj=pSearchObj;
                        pMarkPV=pSearchPV;
                    }
                }
                if (bPrev) nSearchObjNum++;
            }
        }
        if (pMarkObj==NULL) {
            if (bPrev) {
                if (nSearchPvNum>=nMaxPV) return FALSE;
                nSearchPvNum++;
                nSearchObjNum=0;
            } else {
                if (nSearchPvNum==0) return FALSE;
                nSearchPvNum--;
                nSearchObjNum=0xFFFFFFFF;
            }
        }
    }
    if (nChgMarkNum!=0xFFFFFFFF) aMark.DeleteMark(nChgMarkNum);
    MarkObj(pMarkObj,pMarkPV); // ruft auch MarkListHasChanged(), AdjustMarkHdl()
    return TRUE;
}

BOOL SdrMarkView::MarkNextObj(const Point& rPnt, short nTol, BOOL bPrev)
{
    aMark.ForceSort();
    nTol=ImpGetHitTolLogic(nTol,NULL);
    Point aPt(rPnt);
    SdrMark* pTopMarkHit=NULL;
    SdrMark* pBtmMarkHit=NULL;
    ULONG nTopMarkHit=0;
    ULONG nBtmMarkHit=0;
    // oberstes der markierten Objekte suchen, das von rPnt getroffen wird
    ULONG nMarkAnz=aMark.GetMarkCount();
    ULONG nm=0;
    for (nm=nMarkAnz; nm>0 && pTopMarkHit==NULL;) {
        nm--;
        SdrMark* pM=aMark.GetMark(nm);
        if (ImpIsObjHit(aPt,USHORT(nTol),pM->GetObj(),pM->GetPageView(),0)) {
            pTopMarkHit=pM;
            nTopMarkHit=nm;
        }
    }
    // Nichts gefunden, dann ganz normal ein Obj markieren.
    if (pTopMarkHit==NULL) return MarkObj(rPnt,USHORT(nTol),FALSE);

    SdrObject* pTopObjHit=pTopMarkHit->GetObj();
    SdrObjList* pObjList=pTopObjHit->GetObjList();
    SdrPageView* pPV=pTopMarkHit->GetPageView();
    // unterstes der markierten Objekte suchen, das von rPnt getroffen wird
    // und auf der gleichen PageView liegt wie pTopMarkHit
    for (nm=0; nm<nMarkAnz && pBtmMarkHit==NULL; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrPageView* pPV2=pM->GetPageView();
        if (pPV2==pPV && ImpIsObjHit(aPt,USHORT(nTol),pM->GetObj(),pPV2,0)) {
            pBtmMarkHit=pM;
            nBtmMarkHit=nm;
        }
    }
    if (pBtmMarkHit==NULL) { pBtmMarkHit=pTopMarkHit; nBtmMarkHit=nTopMarkHit; }
    SdrObject* pBtmObjHit=pBtmMarkHit->GetObj();

    ULONG nObjAnz=pObjList->GetObjCount();
    ULONG nSearchBeg=bPrev ? pBtmObjHit->GetOrdNum()+1 : pTopObjHit->GetOrdNum();
    ULONG no=nSearchBeg;
    SdrObject* pFndObj=NULL;
    SdrObject* pAktObj=NULL;
    while (pFndObj==NULL && ((!bPrev && no>0) || (bPrev && no<nObjAnz))) {
        if (!bPrev) no--;
        SdrObject* pObj=pObjList->GetObj(no);
        if (ImpIsObjHit(aPt,USHORT(nTol),pObj,pPV,SDRSEARCH_TESTMARKABLE)) {
            if (aMark.FindObject(pObj)==CONTAINER_ENTRY_NOTFOUND) {
                pFndObj=pObj;
            } else {
                // hier wg. Performance ggf. noch no auf Top bzw. auf Btm stellen
            }
        }
        if (bPrev) no++;
    }
    if (pFndObj!=NULL) {
        aMark.DeleteMark(bPrev?nBtmMarkHit:nTopMarkHit);
        aMark.InsertEntry(SdrMark(pFndObj,pPV));
        MarkListHasChanged();
        AdjustMarkHdl(TRUE);
    }
    return pFndObj!=NULL;
}

BOOL SdrMarkView::MarkObj(const Rectangle& rRect, BOOL bUnmark)
{
    BOOL bFnd=FALSE;
    Rectangle aR(rRect);
    SdrObject* pObj;
    SdrObjList* pObjList;
    BrkAction();
    for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
        SdrPageView* pPV=GetPageViewPvNum(nv);
        pObjList=pPV->GetObjList();
        Rectangle aFrm1(aR);
        aFrm1-=pPV->GetOffset(); // Selektionsrahmen auf die PageView transformieren
        ULONG nObjAnz=pObjList->GetObjCount();
        for (ULONG nO=0; nO<nObjAnz; nO++) {
            pObj=pObjList->GetObj(nO);
            Rectangle aRect(pObj->GetBoundRect());
            if (aFrm1.IsInside(aRect)) {
                if (!bUnmark) {
                    if (IsObjMarkable(pObj,pPV)) {
                        aMark.InsertEntry(SdrMark(pObj,pPV));
                        bFnd=TRUE;
                    }
                } else {
                    ULONG nPos=aMark.FindObject(pObj);
                    if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
                        aMark.DeleteMark(nPos);
                        bFnd=TRUE;
                    }
                }
            }
        }
    }
    if (bFnd) {
        aMark.ForceSort();
        MarkListHasChanged();
        AdjustMarkHdl(TRUE);
        ShowMarkHdl(NULL);
    }
    return bFnd;
}

void SdrMarkView::MarkObj(SdrObject* pObj, SdrPageView* pPV, BOOL bUnmark, BOOL bImpNoSetMarkHdl)
{
    if (pObj!=NULL && pPV!=NULL && IsObjMarkable(pObj, pPV)) {
        BrkAction();
        if (!bUnmark) {
            aMark.InsertEntry(SdrMark(pObj,pPV));
        } else {
            ULONG nPos=aMark.FindObject(pObj);
            if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
                aMark.DeleteMark(nPos);
            }
        }
        if (!bImpNoSetMarkHdl) {
            MarkListHasChanged();
            AdjustMarkHdl(TRUE);
//            if (!IsSolidMarkHdl() || !bSomeObjChgdFlag) {
            if (!bSomeObjChgdFlag) {
                // ShowMarkHdl kommt sonst mit dem AfterPaintTimer
                ShowMarkHdl(NULL);
            }
        }
    }
}

BOOL SdrMarkView::IsObjMarked(SdrObject* pObj) const
{
    // nicht so ganz die feine Art: Da FindObject() nicht const ist
    // muss ich mich hier auf non-const casten.
    ULONG nPos=((SdrMarkView*)this)->aMark.FindObject(pObj);
    return nPos!=CONTAINER_ENTRY_NOTFOUND;
}

USHORT SdrMarkView::GetMarkHdlSizePixel() const
{
    return aHdl.GetHdlSize()*2+1;
}

void SdrMarkView::SetSolidMarkHdl(BOOL bOn)
{
    if (bOn!=aHdl.IsFineHdl()) {
        BOOL bMerk=IsMarkHdlShown();
        if (bMerk) HideMarkHdl(NULL);
        aHdl.SetFineHdl(bOn);
        if (bMerk) ShowMarkHdl(NULL);
    }
}

void SdrMarkView::SetMarkHdlSizePixel(USHORT nSiz)
{
    if (nSiz<3) nSiz=3;
    nSiz/=2;
    if (nSiz!=aHdl.GetHdlSize()) {
        BOOL bMerk=IsMarkHdlShown();
        if (bMerk) HideMarkHdl(NULL);
        aHdl.SetHdlSize(nSiz);
        if (bMerk) ShowMarkHdl(NULL);
    }
}

#define SDRSEARCH_IMPISMASTER 0x80000000 /* MasterPage wird gerade durchsucht */
SdrObject* SdrMarkView::ImpCheckObjHit(const Point& rPnt, USHORT nTol, SdrObject* pObj, SdrPageView* pPV, ULONG nOptions, const SetOfByte* pMVisLay) const
{
    if ((nOptions & SDRSEARCH_IMPISMASTER) !=0 && pObj->IsNotVisibleAsMaster()) {
        return NULL;
    }
    BOOL bCheckIfMarkable=(nOptions & SDRSEARCH_TESTMARKABLE)!=0;
    BOOL bBack=(nOptions & SDRSEARCH_BACKWARD)!=0;
    BOOL bDeep=(nOptions & SDRSEARCH_DEEP)!=0;
    BOOL bOLE=pObj->ISA(SdrOle2Obj);
    SdrObject* pRet=NULL;
    Point aPnt1(rPnt-pPV->GetOffset()); // rPnt auf PageView transformieren
    Rectangle aRect(pObj->GetBoundRect());
    USHORT nTol2=nTol;
    // Doppelte Tolezanz fuer ein an dieser View im TextEdit befindliches Objekt
    if (bOLE || pObj==((SdrObjEditView*)this)->GetTextEditObject()) nTol2*=2;
    aRect.Left  ()-=nTol2; // Einmal Toleranz drauf fuer alle Objekte
    aRect.Top   ()-=nTol2;
    aRect.Right ()+=nTol2;
    aRect.Bottom()+=nTol2;
    if (aRect.IsInside(aPnt1)) {
        if ((!bCheckIfMarkable || IsObjMarkable(pObj,pPV))) {
            SdrObjList* pOL=pObj->GetSubList();
            if (pOL!=NULL && pOL->GetObjCount()!=0) {
                SdrObject* pTmpObj;
                pRet=ImpCheckObjHit(rPnt,nTol,pOL,pPV,nOptions,pMVisLay,pTmpObj);
            } else {
                SdrLayerID nLay=pObj->GetLayer();
                if (pPV->GetVisibleLayers().IsSet(nLay) &&
                    (pMVisLay==NULL || pMVisLay->IsSet(nLay)))
                {
                    pRet=pObj->CheckHit(aPnt1,nTol2,&pPV->GetVisibleLayers());
                }
            }
        }
    }
    if (!bDeep && pRet!=NULL) pRet=pObj;
    return pRet;
}

SdrObject* SdrMarkView::ImpCheckObjHit(const Point& rPnt, USHORT nTol, SdrObjList* pOL, SdrPageView* pPV, ULONG nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj) const
{
    BOOL bBack=(nOptions & SDRSEARCH_BACKWARD)!=0;
    SdrObject* pRet=NULL;
    rpRootObj=NULL;
    if (pOL!=NULL) {
        ULONG nObjAnz=pOL->GetObjCount();
        ULONG nObjNum=bBack ? 0 : nObjAnz;
        while (pRet==NULL && (bBack ? nObjNum<nObjAnz : nObjNum>0)) {
            if (!bBack) nObjNum--;
            SdrObject* pObj=pOL->GetObj(nObjNum);
            pRet=ImpCheckObjHit(rPnt,nTol,pObj,pPV,nOptions,pMVisLay);
            if (pRet!=NULL) rpRootObj=pObj;
            if (bBack) nObjNum++;
        }
    }
    return pRet;
}

BOOL SdrMarkView::PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, ULONG nOptions) const
{
    return PickObj(rPnt,nTol,rpObj,rpPV,nOptions,NULL,NULL,NULL);
}

BOOL SdrMarkView::PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, ULONG nOptions, SdrObject** ppRootObj, ULONG* pnMarkNum, USHORT* pnPassNum) const
{ // Fehlt noch Pass2,Pass3
    ((SdrMarkView*)this)->aMark.ForceSort();
    if (ppRootObj!=NULL) *ppRootObj=NULL;
    if (pnMarkNum!=NULL) *pnMarkNum=CONTAINER_ENTRY_NOTFOUND;
    if (pnPassNum!=NULL) *pnPassNum=0;
    rpObj=NULL;
    rpPV=NULL;
    BOOL bWholePage=(nOptions & SDRSEARCH_WHOLEPAGE) !=0;
    BOOL bMarked=(nOptions & SDRSEARCH_MARKED) !=0;
    BOOL bMasters=!bMarked && (nOptions & SDRSEARCH_ALSOONMASTER) !=0;
    BOOL bBack=(nOptions & SDRSEARCH_BACKWARD) !=0;
    BOOL bNext=(nOptions & SDRSEARCH_NEXT) !=0; // n.i.
    BOOL bBoundCheckOn2ndPass=(nOptions & SDRSEARCH_PASS2BOUND) !=0; // n.i.
    BOOL bCheckNearestOn3rdPass=(nOptions & SDRSEARCH_PASS3NEAREST) !=0; // n.i.
    if (nTol<0) nTol=ImpGetHitTolLogic(nTol,NULL);
    Point aPt(rPnt);
    SdrObject* pObj=NULL;
    SdrObject* pHitObj=NULL;
    SdrPageView* pPV=NULL;
    if (!bBack && ((SdrObjEditView*)this)->IsTextEditFrameHit(rPnt)) {
        pObj=((SdrObjEditView*)this)->GetTextEditObject();
        pHitObj=pObj;
        pPV=((SdrObjEditView*)this)->GetTextEditPageView();
    }
    if (bMarked) {
        ULONG nMrkAnz=aMark.GetMarkCount();
        ULONG nMrkNum=bBack ? 0 : nMrkAnz;
        while (pHitObj==NULL && (bBack ? nMrkNum<nMrkAnz : nMrkNum>0)) {
            if (!bBack) nMrkNum--;
            SdrMark* pM=aMark.GetMark(nMrkNum);
            pObj=pM->GetObj();
            pPV=pM->GetPageView();
            pHitObj=ImpCheckObjHit(aPt,nTol,pObj,pPV,nOptions,NULL);
            if (bBack) nMrkNum++;
        }
    } else {
        USHORT nPvAnz=GetPageViewCount();
        USHORT nPvNum=bBack ? 0 : nPvAnz;
        while (pHitObj==NULL && (bBack ? nPvNum<nPvAnz : nPvNum>0)) {
            if (!bBack) nPvNum--;
            pPV=GetPageViewPvNum(nPvNum);
            SdrPage* pPage=pPV->GetPage();
            USHORT nPgAnz=1; if (bMasters) nPgAnz+=pPage->GetMasterPageCount();
            BOOL bExtraPassForWholePage=bWholePage && pPage!=pPV->GetObjList();
            if (bExtraPassForWholePage) nPgAnz++; // Suche erst in AktObjList, dann auf der gesamten Page
            USHORT nPgNum=bBack ? 0 : nPgAnz;
            while (pHitObj==NULL && (bBack ? nPgNum<nPgAnz : nPgNum>0)) {
                ULONG nTmpOptions=nOptions;
                if (!bBack) nPgNum--;
                const SetOfByte* pMVisLay=NULL;
                SdrObjList* pObjList=NULL;
                if (pnPassNum!=NULL) *pnPassNum&=~(SDRSEARCHPASS_MASTERPAGE|SDRSEARCHPASS_INACTIVELIST);
                if (nPgNum>=nPgAnz-1 || (bExtraPassForWholePage && nPgNum>=nPgAnz-2)) {
                    pObjList=pPV->GetObjList();
                    if (bExtraPassForWholePage && nPgNum==nPgAnz-2) {
                        pObjList=pPage;
                        if (pnPassNum!=NULL) *pnPassNum|=SDRSEARCHPASS_INACTIVELIST;
                    }
                } else { // sonst MasterPage
                    const SdrMasterPageDescriptor& rMPD=pPage->GetMasterPageDescriptor(nPgNum);
                    USHORT nNum=rMPD.GetPageNum();
                    pMVisLay=&rMPD.GetVisibleLayers();
                    if (nNum<pMod->GetMasterPageCount()) { // sonst ungueltiger MasterPageDescriptor
                        pObjList=pMod->GetMasterPage(nNum);
                    }
                    if (pnPassNum!=NULL) *pnPassNum|=SDRSEARCHPASS_MASTERPAGE;
                    nTmpOptions=nTmpOptions | SDRSEARCH_IMPISMASTER;
                }
                pHitObj=ImpCheckObjHit(aPt,nTol,pObjList,pPV,nTmpOptions,pMVisLay,pObj);
                if (bBack) nPgNum++;
            }
            if (bBack) nPvNum++;
        }

    }
    if (pHitObj!=NULL) {
        if (ppRootObj!=NULL) *ppRootObj=pObj;
        if ((nOptions & SDRSEARCH_DEEP) !=0) pObj=pHitObj;
        if ((nOptions & SDRSEARCH_TESTTEXTEDIT) !=0) {
            if (!pObj->HasTextEdit() || pPV->GetLockedLayers().IsSet(pObj->GetLayer())) {
                pObj=NULL;
            }
        }
        if (pObj!=NULL && (nOptions & SDRSEARCH_TESTMACRO) !=0) {
            Point aP(aPt); aP-=pPV->GetOffset();
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=aPt;
            aHitRec.aDownPos=aPt;
            aHitRec.nTol=nTol;
            aHitRec.pVisiLayer=&pPV->GetVisibleLayers();
            aHitRec.pPageView=pPV;
            if (!pObj->HasMacro() || !pObj->IsMacroHit(aHitRec)) pObj=NULL;
        }
        if (pObj!=NULL && (nOptions & SDRSEARCH_WITHTEXT) !=0 && pObj->GetOutlinerParaObject()==NULL) pObj=NULL;
        if (pObj!=NULL && (nOptions & SDRSEARCH_TESTTEXTAREA) !=0) {
            Point aP(aPt); aP-=pPV->GetOffset();
            if (!pObj->IsTextEditHit(aPt,0/*nTol*/,NULL)) pObj=NULL;
        }
        if (pObj!=NULL) {
            rpObj=pObj;
            rpPV=pPV;
            if (pnPassNum!=NULL) *pnPassNum|=SDRSEARCHPASS_DIRECT;
        }
    }
    return rpObj!=NULL;
}

BOOL SdrMarkView::PickMarkedObj(const Point& rPnt, SdrObject*& rpObj, SdrPageView*& rpPV, ULONG* pnMarkNum, ULONG nOptions) const
{
    ((SdrMarkView*)this)->aMark.ForceSort();
    BOOL bBoundCheckOn2ndPass=(nOptions & SDRSEARCH_PASS2BOUND) !=0;
    BOOL bCheckNearestOn3rdPass=(nOptions & SDRSEARCH_PASS3NEAREST) !=0;
    rpObj=NULL;
    rpPV=NULL;
    if (pnMarkNum!=NULL) *pnMarkNum=CONTAINER_ENTRY_NOTFOUND;
    Point aPt(rPnt);
    USHORT nTol=(USHORT)nHitTolLog;
    BOOL bFnd=FALSE;
    ULONG nMarkAnz=aMark.GetMarkCount();
    ULONG nMarkNum;
    for (nMarkNum=nMarkAnz; nMarkNum>0 && !bFnd;) {
        nMarkNum--;
        SdrMark* pM=aMark.GetMark(nMarkNum);
        SdrPageView* pPV=pM->GetPageView();
        SdrObject* pObj=pM->GetObj();
        bFnd=ImpIsObjHit(aPt,nTol,pObj,pPV,SDRSEARCH_TESTMARKABLE);
        if (bFnd) {
            rpObj=pObj;
            rpPV=pPV;
            if (pnMarkNum!=NULL) *pnMarkNum=nMarkNum;
        }
    }
    if ((bBoundCheckOn2ndPass || bCheckNearestOn3rdPass) && !bFnd) {
        SdrObject* pBestObj=NULL;
        SdrPageView* pBestPV=NULL;
        ULONG nBestMarkNum=0;
        ULONG nBestDist=0xFFFFFFFF;
        for (nMarkNum=nMarkAnz; nMarkNum>0 && !bFnd;) {
            nMarkNum--;
            SdrMark* pM=aMark.GetMark(nMarkNum);
            SdrPageView* pPV=pM->GetPageView();
            SdrObject* pObj=pM->GetObj();
            Point aPt1(aPt);
            aPt1-=pPV->GetOffset();
            Rectangle aRect(pObj->GetBoundRect());
            aRect.Left  ()-=nTol;
            aRect.Top   ()-=nTol;
            aRect.Right ()+=nTol;
            aRect.Bottom()+=nTol;
            if (aRect.IsInside(aPt1)) {
                bFnd=TRUE;
                rpObj=pObj;
                rpPV=pPV;
                if (pnMarkNum!=NULL) *pnMarkNum=nMarkNum;
            } else if (bCheckNearestOn3rdPass) {
                ULONG nDist=0;
                if (aPt1.X()<aRect.Left())   nDist+=aRect.Left()-aPt1.X();
                if (aPt1.X()>aRect.Right())  nDist+=aPt1.X()-aRect.Right();
                if (aPt1.Y()<aRect.Top())    nDist+=aRect.Top()-aPt1.Y();
                if (aPt1.Y()>aRect.Bottom()) nDist+=aPt1.Y()-aRect.Bottom();
                if (nDist<nBestDist) {
                    pBestObj=pObj;
                    pBestPV=pPV;
                    nBestMarkNum=nMarkNum;
                }
            }
        }
        if (bCheckNearestOn3rdPass && !bFnd) {
            rpObj=pBestObj;
            rpPV=pBestPV;
            if (pnMarkNum!=NULL) *pnMarkNum=nBestMarkNum;
            bFnd=pBestObj!=NULL;
        }
    }
    return bFnd;
}

SdrHitKind SdrMarkView::PickSomething(const Point& rPnt, short nTol) const
{
    nTol=ImpGetHitTolLogic(nTol,NULL);
    SdrHitKind eRet=SDRHIT_NONE;
    Point aPt(rPnt);
    SdrObject* pObj=NULL;
    SdrPageView* pPV=NULL;
    if (eRet==SDRHIT_NONE && PickObj(rPnt,USHORT(nTol),pObj,pPV,SDRSEARCH_PICKMARKABLE)) {
        Rectangle aRct1(aPt-Point(nTol,nTol),aPt+Point(nTol,nTol)); // HitRect fuer Toleranz
        Rectangle aBR(pObj->GetBoundRect());
        if      (aRct1.IsInside(aBR.TopLeft()))      eRet=SDRHIT_BOUNDTL;
        else if (aRct1.IsInside(aBR.TopCenter()))    eRet=SDRHIT_BOUNDTC;
        else if (aRct1.IsInside(aBR.TopRight()))     eRet=SDRHIT_BOUNDTR;
        else if (aRct1.IsInside(aBR.LeftCenter()))   eRet=SDRHIT_BOUNDCL;
        else if (aRct1.IsInside(aBR.RightCenter()))  eRet=SDRHIT_BOUNDCR;
        else if (aRct1.IsInside(aBR.BottomLeft()))   eRet=SDRHIT_BOUNDBL;
        else if (aRct1.IsInside(aBR.BottomCenter())) eRet=SDRHIT_BOUNDBC;
        else if (aRct1.IsInside(aBR.BottomRight()))  eRet=SDRHIT_BOUNDBR;
        else eRet=SDRHIT_OBJECT;
    }
    return eRet;
}

void SdrMarkView::UnmarkAllObj(SdrPageView* pPV)
{
    if (aMark.GetMarkCount()!=0) {
        BrkAction();
        BOOL bVis=bHdlShown;
        if (bVis) HideMarkHdl(NULL);
        if (pPV!=NULL) {
            aMark.DeletePageView(*pPV);
        } else {
            aMark.Clear();
        }
        pMarkedObj=NULL;
        pMarkedPV=NULL;
        MarkListHasChanged();
        AdjustMarkHdl(TRUE);
        if (bVis) ShowMarkHdl(NULL); // ggf. fuer die RefPoints
    }
}

void SdrMarkView::MarkAllObj(SdrPageView* pPV)
{
    BOOL bMarkChg=FALSE;
    BrkAction();
    HideMarkHdl(NULL);
    if (pPV!=NULL) {
        bMarkChg=aMark.InsertPageView(*pPV);
    } else {
        for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
            SdrPageView* pPV=GetPageViewPvNum(nv);
            if (aMark.InsertPageView(*pPV)) bMarkChg=TRUE;
        }
    }
    if (bMarkChg) MarkListHasChanged();
    if (aMark.GetMarkCount()!=0) {
        AdjustMarkHdl(TRUE);
        ShowMarkHdl(NULL);
    } else {
    }
}

void SdrMarkView::AdjustMarkHdl(BOOL bRestraintPaint)
{
    BOOL bVis=bHdlShown;
    if (bVis) HideMarkHdl(NULL);
    CheckMarked();
    SetMarkRects();
    SetMarkHandles();
    if(bRestraintPaint && bVis)
    {
        ShowMarkHdl(NULL);

        // refresh IAOs
//--/       RefreshAllIAOManagers();
    }
}

Rectangle SdrMarkView::GetMarkedObjBoundRect() const
{
    Rectangle aRect;
    for (ULONG nm=0; nm<aMark.GetMarkCount(); nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        Rectangle aR1(pO->GetBoundRect());
        aR1+=pM->GetPageView()->GetOffset();
        if (aRect.IsEmpty()) aRect=aR1;
        else aRect.Union(aR1);
    }
    return aRect;
}

const Rectangle& SdrMarkView::GetMarkedObjRect() const
{
    if (bMarkedObjRectDirty) {
        ((SdrMarkView*)this)->bMarkedObjRectDirty=FALSE;
        Rectangle aRect;
        for (ULONG nm=0; nm<aMark.GetMarkCount(); nm++) {
            SdrMark* pM=aMark.GetMark(nm);
            SdrObject* pO=pM->GetObj();
            Rectangle aR1(pO->GetSnapRect());
            aR1+=pM->GetPageView()->GetOffset();
            if (aRect.IsEmpty()) aRect=aR1;
            else aRect.Union(aR1);
        }
        ((SdrMarkView*)this)->aMarkedObjRect=aRect;
    }
    return aMarkedObjRect;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::ImpTakeDescriptionStr(const SdrMarkList& rML, USHORT nStrCacheID, XubString& rStr, USHORT nVal, USHORT nOpt) const
{
    rStr = ImpGetResStr(nStrCacheID);
    xub_StrLen nPos = rStr.SearchAscii("%O");

    if(nPos != STRING_NOTFOUND)
    {
        rStr.Erase(nPos, 2);

        if(nOpt == IMPSDR_POINTSDESCRIPTION)
        {
            rStr.Insert(rML.GetPointMarkDescription(), nPos);
        }
        else if(nOpt == IMPSDR_GLUEPOINTSDESCRIPTION)
        {
            rStr.Insert(rML.GetGluePointMarkDescription(), nPos);
        }
        else
        {
            rStr.Insert(rML.GetMarkDescription(), nPos);
        }
    }

    nPos = rStr.SearchAscii("%N");

    if(nPos != STRING_NOTFOUND)
    {
        rStr.Erase(nPos, 2);
        rStr.Insert(UniString::CreateFromInt32(nVal), nPos);
    }
}

void SdrMarkView::ImpTakeDescriptionStr(USHORT nStrCacheID, XubString& rStr, USHORT nVal, USHORT nOpt) const
{
    ImpTakeDescriptionStr(aMark,nStrCacheID,rStr,nVal,nOpt);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrMarkView::EnterMarkedGroup()
{
    BOOL bRet=FALSE;
    // Es wird nur die erste gefundene Gruppe (also nur in einer PageView) geentert
    // Weil PageView::EnterGroup ein AdjustMarkHdl ruft.
    // Das muss ich per Flag mal unterbinden  vvvvvvvv
    for (USHORT nv=0;   nv<GetPageViewCount() && !bRet; nv++) {
        BOOL bEnter=FALSE;
        SdrPageView* pPV=GetPageViewPvNum(nv);
        for (ULONG nm=aMark.GetMarkCount(); nm>0 && !bEnter;) {
            nm--;
            SdrMark* pM=aMark.GetMark(nm);
            if (pM->GetPageView()==pPV) {
                SdrObject* pObj=pM->GetObj();
                if (pObj->IsGroupObject()) {
                    if (pPV->EnterGroup(pObj)) {
                        bRet=TRUE;
                        bEnter=TRUE;
                    }
                }
            }
        }
    }
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::ForceEdgesOfMarkedNodes()
{
    if (bEdgesOfMarkedNodesDirty) {
        aEdgesOfMarkedNodes.Clear();
        aMarkedEdgesOfMarkedNodes.Clear();
        aMarkedEdges.Clear();
        bEdgesOfMarkedNodesDirty=FALSE;
        aMark.ForceSort();
        ULONG nMarkAnz=aMark.GetMarkCount();
        for (ULONG nm=0; nm<nMarkAnz; nm++) {
            SdrObject* pNode=aMark.GetMark(nm)->GetObj();
            const SfxBroadcaster* pBC=pNode->GetBroadcaster();
            if (pNode->IsNode()) {
                if (pBC!=NULL) {
                    USHORT nLstAnz=pBC->GetListenerCount();
                    for (USHORT nl=0; nl<nLstAnz; nl++) {
                        SfxListener* pLst=pBC->GetListener(nl);
                        SdrEdgeObj* pEdge=PTR_CAST(SdrEdgeObj,pLst);
                        if (pEdge!=NULL && pEdge->IsInserted() && pEdge->GetPage()==pNode->GetPage()) {
                            SdrMark aM(pEdge,aMark.GetMark(nm)->GetPageView());
                            if (pEdge->GetConnectedNode(TRUE)==pNode) aM.SetCon1(TRUE);
                            if (pEdge->GetConnectedNode(FALSE)==pNode) aM.SetCon2(TRUE);
                            if (!IsObjMarked(pEdge)) { // nachsehen, ob er selbst markiert ist
                                aEdgesOfMarkedNodes.InsertEntry(aM);
                            } else {
                                aMarkedEdgesOfMarkedNodes.InsertEntry(aM);
                            }
                        }
                    }
                }
            }
            if (pNode->IsEdge()) {
                aMarkedEdges.InsertEntry(*aMark.GetMark(nm)); // alle markierten Edges merken
            }
        }
        aEdgesOfMarkedNodes.ForceSort();
        aMarkedEdgesOfMarkedNodes.ForceSort();
        aMarkedEdges.ForceSort();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::MarkListHasChanged()
{
    aMark.SetNameDirty();
    bEdgesOfMarkedNodesDirty=TRUE;
    aEdgesOfMarkedNodes.Clear();
    aMarkedEdgesOfMarkedNodes.Clear();
    aMarkedEdges.Clear();
    bMarkedObjRectDirty=TRUE;
    bMarkedPointsRectsDirty=TRUE;
    if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
    BOOL bOneEdgeMarked=FALSE;
    if (aMark.GetMarkCount()==1) {
        const SdrObject* pObj=aMark.GetMark(0)->GetObj();
        if (pObj->GetObjInventor()==SdrInventor) {
            UINT16 nIdent=pObj->GetObjIdentifier();
            bOneEdgeMarked=nIdent==OBJ_EDGE;
        }
    }
    ImpSetGlueVisible4(bOneEdgeMarked);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::WriteRecords(SvStream& rOut) const
{
    SdrSnapView::WriteRecords(rOut);
    {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWDRAGMODE);
        rOut<<BOOL(eEditMode==SDREDITMODE_EDIT); // wg. Kompatibilitaet
        rOut<<USHORT(eDragMode);
        rOut<<aRef1;
        rOut<<aRef2;
        rOut<<BOOL(bForceFrameHandles);
        rOut<<BOOL(bPlusHdlAlways);
        rOut<<BOOL(eEditMode==SDREDITMODE_GLUEPOINTEDIT); // wg. Kompatibilitaet
        rOut<<USHORT(eEditMode);
        rOut<<BOOL(bMarkHdlWhenTextEdit);
    } {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWCROOKCENTER);
        rOut<<aLastCrookCenter;
    }
}

BOOL SdrMarkView::ReadRecord(const SdrIOHeader& rViewHead,
    const SdrNamedSubRecord& rSubHead,
    SvStream& rIn)
{
    BOOL bRet=FALSE;
    if (rSubHead.GetInventor()==SdrInventor) {
        bRet=TRUE;
        switch (rSubHead.GetIdentifier()) {
            case SDRIORECNAME_VIEWDRAGMODE: {
                eEditMode=SDREDITMODE_EDIT;
                BOOL bTmpBool;
                USHORT nTmpUShort;
                rIn>>bTmpBool; if (!bTmpBool) eEditMode=SDREDITMODE_CREATE; // wg. Kompatibilitaet
                USHORT nDragMode;
                rIn>>nDragMode;
                eDragMode=SdrDragMode(nDragMode);
                rIn>>aRef1;
                rIn>>aRef2;
                rIn>>bTmpBool; bForceFrameHandles=bTmpBool;
                rIn>>bTmpBool; bPlusHdlAlways=bTmpBool;
                if (rSubHead.GetBytesLeft()!=0) {
                    rIn>>bTmpBool;
                    if (bTmpBool) eEditMode=SDREDITMODE_GLUEPOINTEDIT; // wg. Kompatibilitaet
                }
                if (rSubHead.GetBytesLeft()!=0) {
                    rIn>>nTmpUShort;
                    eEditMode=(SdrViewEditMode)nTmpUShort;
                }
                bGlueVisible2=eEditMode==SDREDITMODE_GLUEPOINTEDIT;
                if (rSubHead.GetBytesLeft()!=0) {
                    rIn>>bTmpBool;
                    bMarkHdlWhenTextEdit=bTmpBool;
                }
            } break;
            case SDRIORECNAME_VIEWCROOKCENTER: {
                rIn>>aLastCrookCenter;
            } break;
            default: bRet=FALSE;
        }
    }
    if (!bRet) bRet=SdrSnapView::ReadRecord(rViewHead,rSubHead,rIn);
    return bRet;
}

void SdrMarkView::SetDesignMode(BOOL bOn)
{
    if (bDesignMode != bOn)
    {
        bDesignMode = bOn;
        // Setzen des Modes fuer alle Controls
        USHORT nAnz = GetPageViewCount();
        for (USHORT nv = 0; nv<nAnz; nv++)
        {
            SdrPageView* pPV = GetPageViewPvNum(nv);
            const SdrPageViewWinList& rWinList = pPV->GetWinList();
            for (ULONG i = 0; i < rWinList.GetCount(); i++)
            {
                const SdrPageViewWinRec& rWR = rWinList[i];
                const SdrUnoControlList& rControlList = rWR.GetControlList();

                for(UINT32 j = 0; j < rControlList.GetCount(); j++)
                {
                    rControlList[j].GetControl()->setDesignMode(bOn);
                }
            }
        }
    }
}

// MarkHandles Objektaenderung:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// - Bei Notify mit HINT_OBJCHG (oder so) werden die Handles erstmal versteckt
//   (wenn nicht schon wegen Dragging versteckt).
// - XorHdl: Bei ModelHasChanged() werden sie dann wieder angezeigt.
// - PaintEvents kommen nun durch.
//   - Die XorHandles werden z.T. wieder uebermalt.
//   - Xor:  Nach dem Painten werden die Handles im (vom PaintHandler gerufenen)
//           InitRedraw per ToggleShownXor bei gesetzter ClipRegion nochmal gemalt
//           und damit ist alles in Butter.
//   - ToggleShownXor macht bei SolidHdl nix weil bHdlShown=FALSE
//   - Der AfterPaintTimer wird gestartet.
// - SolidHdl: Im AfterPaintHandler wird ShowMarkHdl gerufen.
//   Da die Handles zu diesem Zeitpunkt nicht angezeigt sind wird:
//   - SaveBackground durchgefuehrt.
//   - DrawMarkHdl gerufen und bHdlShown gesetzt.
//
// MarkHandles bei sonstigem Invalidate:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// In diesem Fall bekomme ich kein Notify und beim Aufruf des
// PaintHandlers->InitRedraw() sind auch die SolidHandles sichtbar.

