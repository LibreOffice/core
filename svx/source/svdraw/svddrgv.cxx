/*************************************************************************
 *
 *  $RCSfile: svddrgv.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
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

#include "svddrgv.hxx"
#include "xattr.hxx"
#include "xpoly.hxx"
#include "svdxout.hxx"
#include "svdetc.hxx"
#include "svdtrans.hxx"
#include "svdundo.hxx"
#include "svdocapt.hxx"
#include "svdio.hxx"
#include "svdpagv.hxx"
#include "svdopath.hxx" // wg. Sonderbehandlung in SetDragPolys()
#include "svdoedge.hxx" // wg. Sonderbehandlung in IsInsGluePossible()
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache
#include "svddrgm1.hxx"
#include "obj3d.hxx"

#define XOR_DRAG_PEN   PEN_DOT

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@  @@@@@   @@@@   @@@@   @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@  @@ @@  @@  @@ @@ @@ @@    @@   @@
//  @@  @@ @@  @@ @@  @@ @@      @@ @@ @@ @@    @@ @ @@
//  @@  @@ @@@@@  @@@@@@ @@ @@@  @@@@@ @@ @@@@  @@@@@@@
//  @@  @@ @@  @@ @@  @@ @@  @@   @@@  @@ @@    @@@@@@@
//  @@  @@ @@  @@ @@  @@ @@  @@   @@@  @@ @@    @@@ @@@
//  @@@@@  @@  @@ @@  @@  @@@@@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::ImpClearVars()
{
    bFramDrag=FALSE;
    eDragMode=SDRDRAG_MOVE;
    bDragLimit=FALSE;
    bMarkedHitMovesAlways=FALSE;
    eDragHdl=HDL_MOVE;
    pDragHdl=NULL;
    bDragHdl=FALSE;
    bDragSpecial=FALSE;
    pDragBla=NULL;
    bDragStripes=FALSE;
    bNoDragHdl=TRUE;
    bMirrRefDragObj=TRUE;
    bSolidDragging=FALSE;
    bSolidDrgNow=FALSE;
    bSolidDrgChk=FALSE;
    bDragWithCopy=FALSE;
    pInsPointUndo=NULL;
    bInsAfter=FALSE;
    bInsGluePoint=FALSE;
    bInsObjPointMode=FALSE;
    bInsGluePointMode=FALSE;
    nDragXorPolyLimit=100;
    nDragXorPointLimit=500;
    bNoDragXorPolys=FALSE;
    bAutoVertexCon=TRUE;
    bAutoCornerCon=FALSE;
    bRubberEdgeDragging=TRUE;
    nRubberEdgeDraggingLimit=100;
    bDetailedEdgeDragging=TRUE;
    nDetailedEdgeDraggingLimit=10;
    bResizeAtCenter=FALSE;
    bCrookAtCenter=FALSE;
    bMouseHideWhileDraggingPoints=FALSE;
}

void SdrDragView::ImpMakeDragAttr()
{
    ImpDelDragAttr();
}

SdrDragView::SdrDragView(SdrModel* pModel1, OutputDevice* pOut):
    SdrExchangeView(pModel1,pOut)
{
    ImpClearVars();
    ImpMakeDragAttr();
}

SdrDragView::SdrDragView(SdrModel* pModel1, ExtOutputDevice* pXOut):
    SdrExchangeView(pModel1,pXOut)
{
    ImpClearVars();
    ImpMakeDragAttr();
}

SdrDragView::~SdrDragView()
{
    ImpDelDragAttr();
}

void SdrDragView::ImpDelDragAttr()
{
}

BOOL SdrDragView::IsAction() const
{
    return SdrExchangeView::IsAction() || pDragBla!=NULL;
}

void SdrDragView::MovAction(const Point& rPnt)
{
    SdrExchangeView::MovAction(rPnt);
    if (pDragBla!=NULL) {
        MovDragObj(rPnt);
        RefreshAllIAOManagers();
    }
}

void SdrDragView::EndAction()
{
    if (pDragBla!=NULL)
    {
        EndDragObj(FALSE);
        RefreshAllIAOManagers();
    }
    SdrExchangeView::EndAction();
}

void SdrDragView::BckAction()
{
    SdrExchangeView::BckAction();
    BrkDragObj();
    RefreshAllIAOManagers();
}

void SdrDragView::BrkAction()
{
    SdrExchangeView::BrkAction();
    BrkDragObj();
    RefreshAllIAOManagers();
}

void SdrDragView::TakeActionRect(Rectangle& rRect) const
{
    if (pDragBla!=NULL) {
        rRect=aDragStat.GetActionRect();
        if (rRect.IsEmpty()) {
            BOOL b1st=TRUE;
            for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
                SdrPageView* pPV=GetPageViewPvNum(nv);
                if (pPV->HasMarkedObj()) {
                    Rectangle aR(pPV->DragPoly().GetBoundRect(GetWin(0)));
                    aR+=pPV->GetOffset();
                    if (b1st) {
                        b1st=FALSE;
                        rRect=aR;
                    } else {
                        rRect.Union(aR);
                    }
                }
            }
        }
        if (rRect.IsEmpty()) {
            rRect=Rectangle(aDragStat.GetNow(),aDragStat.GetNow());
        }
    } else {
        SdrExchangeView::TakeActionRect(rRect);
    }
}

void SdrDragView::ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const
{
    SdrExchangeView::ToggleShownXor(pOut,pRegion);
    if (pDragBla!=NULL && aDragStat.IsShown() &&
        !IS_TYPE(SdrDragMovHdl,pDragBla)) { // das ist ein Hack !!!!!!!!!!
        DrawDragObj(pOut,TRUE);
    }
}

void SdrDragView::SetDragPolys(BOOL bReset, BOOL bSeparate)
{
    USHORT nPvAnz=GetPageViewCount();
    ULONG nMarkAnz=aMark.GetMarkCount();
    if (!bReset && (IsDraggingPoints() || IsDraggingGluePoints())) {
        BOOL bGlue=IsDraggingGluePoints();
        for (USHORT nv=0; nv<nPvAnz; nv++) {
            SdrPageView* pPV=GetPageViewPvNum(nv);
            pPV->DragPoly0().Clear();
            if (pPV->HasMarkedObj()) {
                for (ULONG nm=0; nm<nMarkAnz; nm++) {
                    SdrMark* pM=aMark.GetMark(nm);
                    if (pM->GetPageView()==pPV) {
                        const SdrUShortCont* pPts=bGlue ? pM->GetMarkedGluePoints() : pM->GetMarkedPoints();
                        if (pPts!=NULL && pPts->GetCount()!=0) {
                            const SdrObject* pObj=pM->GetObj();
                            const SdrPathObj* pPath=bGlue ? NULL : PTR_CAST(SdrPathObj,pObj);
                            const XPolyPolygon* pPathXPP=pPath!=NULL ? &pPath->GetPathPoly() : NULL;
                            const SdrGluePointList* pGPL=bGlue ? pObj->GetGluePointList() : NULL;
                            ULONG nPtAnz=pPts->GetCount();
                            XPolygon aXP((USHORT)nPtAnz);
                            for (ULONG nPtNum=0; nPtNum<nPtAnz; nPtNum++) {
                                USHORT nObjPt=pPts->GetObject(nPtNum);
                                if (bGlue) {
                                    if (pGPL!=NULL) {
                                        USHORT nGlueNum=pGPL->FindGluePoint(nObjPt);
                                        if (nGlueNum!=SDRGLUEPOINT_NOTFOUND) {
                                            aXP[(USHORT)nPtNum]=(*pGPL)[nGlueNum].GetAbsolutePos(*pObj);
                                        }
                                    }
                                } else {
                                    if (pPath!=NULL && pPathXPP!=NULL) {
                                        USHORT nPolyNum,nPointNum;
                                        if (pPath->TakePolyIdxForHdlNum(nObjPt,nPolyNum,nPointNum)) {
                                            aXP[(USHORT)nPtNum]=(*pPathXPP)[nPolyNum][nPointNum];
                                        }
                                    }
                                }
                            }
                            pPV->DragPoly0().Insert(aXP);
                        }
                    }
                }
            }
            pPV->DragPoly()=pPV->DragPoly0();
        }
        return;
    }
    Rectangle aRect;
    XPolygon aEmptyPoly(0); // Lerres XPoly fuer Separate
    ULONG nMaxObj=nDragXorPolyLimit;
    ULONG nMaxPnt=nDragXorPointLimit;
    BOOL bNoPoly = IsNoDragXorPolys() || aMark.GetMarkCount()>nMaxObj;
    BOOL bBrk=FALSE;
    ULONG nPolyCnt=0; // Max nDragXorPolyLimit Polys
    ULONG nPntCnt=0;  // Max 5*nDragXorPolyLimit Punkte
    if (!bNoPoly && !bReset) {
        for (USHORT nv=0; nv<nPvAnz && !bBrk; nv++) {
            SdrPageView* pPV=GetPageViewPvNum(nv);
            if (pPV->HasMarkedObj()) {
                pPV->DragPoly0().Clear();
                BOOL b1st=TRUE;
                XPolyPolygon aDazuXPP;
                for (ULONG nm=0; nm<nMarkAnz && !bBrk; nm++) {
                    SdrMark* pM=aMark.GetMark(nm);
                    if (pM->GetPageView()==pPV) {
                        pM->GetObj()->TakeXorPoly(aDazuXPP,FALSE);
                        USHORT nDazuPolyAnz=aDazuXPP.Count();
                        nPolyCnt+=nDazuPolyAnz;
                        for (USHORT i=0; i<nDazuPolyAnz; i++) nPntCnt+=aDazuXPP[i].GetPointCount();
                        if (nPolyCnt>nMaxObj || nPntCnt>nMaxPnt) {
                            bBrk=TRUE;
                            bNoPoly=TRUE;
                        }
                        if (!bBrk) {
                            for (USHORT i=0; i<nDazuPolyAnz; i++) {
                                // 2-Punkt-Polygone zu einfachen Linien machen,
                                // damit nicht Xor+Xor=Nix
                                const XPolygon& rP=aDazuXPP[i];
                                if (rP.GetPointCount()==3 && rP[0]==rP[2]) {
                                    aDazuXPP[i].Remove(2,1);
                                }
                            }
                            if (b1st) {
                                pPV->DragPoly0()=aDazuXPP;
                                b1st=FALSE;
                            } else {
                                if (bSeparate) {
                                    // erstmal ein leeres Polygon als Trennung zwischen den Objekten
                                    pPV->DragPoly0().Insert(aEmptyPoly);
                                }
                                pPV->DragPoly0().Insert(aDazuXPP);
                            }
                        }
                    }
                }
                pPV->DragPoly()=pPV->DragPoly0();
            }
        }
    }

    if (bNoPoly || bReset) {
        for (USHORT nv=0; nv<nPvAnz; nv++) {
            SdrPageView* pPV=GetPageViewPvNum(nv);
            if (!bReset) {
                if (pPV->HasMarkedObj()) {
                    Rectangle aR(pPV->MarkSnap());
                    if (TRUE) {
                        BOOL bMorePoints=TRUE;
                        // Faktor fuer Kontrollpunkte der Bezierkurven:
                        // 8/3 * (sin(45g) - 0.5) * 2/Pi
                        double a=0.3515953911 /2; // /2, weil halbe Strecke
                        if (bMorePoints) a/=2;
                        long dx=(long)(aR.GetWidth()*a);
                        long dy=(long)(aR.GetHeight()*a);
                        XPolygon aXP(25);
                        aXP[ 0]=aR.TopLeft();
                        aXP[ 1]=aR.TopLeft();      aXP[ 1].X()+=dx; aXP.SetFlags( 1,XPOLY_CONTROL);
                        aXP[ 2]=aR.TopCenter();    aXP[ 2].X()-=dx; aXP.SetFlags( 2,XPOLY_CONTROL);
                        aXP[ 3]=aR.TopCenter();
                        aXP[ 4]=aR.TopCenter();    aXP[ 4].X()+=dx; aXP.SetFlags( 4,XPOLY_CONTROL);
                        aXP[ 5]=aR.TopRight();     aXP[ 5].X()-=dx; aXP.SetFlags( 5,XPOLY_CONTROL);
                        aXP[ 6]=aR.TopRight();
                        aXP[ 7]=aR.TopRight();     aXP[ 7].Y()+=dy; aXP.SetFlags( 7,XPOLY_CONTROL);
                        aXP[ 8]=aR.RightCenter();  aXP[ 8].Y()-=dy; aXP.SetFlags( 8,XPOLY_CONTROL);
                        aXP[ 9]=aR.RightCenter();
                        aXP[10]=aR.RightCenter();  aXP[10].Y()+=dy; aXP.SetFlags(10,XPOLY_CONTROL);
                        aXP[11]=aR.BottomRight();  aXP[11].Y()-=dy; aXP.SetFlags(11,XPOLY_CONTROL);
                        aXP[12]=aR.BottomRight();
                        aXP[13]=aR.BottomRight();  aXP[13].X()-=dx; aXP.SetFlags(13,XPOLY_CONTROL);
                        aXP[14]=aR.BottomCenter(); aXP[14].X()+=dx; aXP.SetFlags(14,XPOLY_CONTROL);
                        aXP[15]=aR.BottomCenter();
                        aXP[16]=aR.BottomCenter(); aXP[16].X()-=dx; aXP.SetFlags(16,XPOLY_CONTROL);
                        aXP[17]=aR.BottomLeft();   aXP[17].X()+=dx; aXP.SetFlags(17,XPOLY_CONTROL);
                        aXP[18]=aR.BottomLeft();
                        aXP[19]=aR.BottomLeft();   aXP[19].Y()-=dy; aXP.SetFlags(19,XPOLY_CONTROL);
                        aXP[20]=aR.LeftCenter();   aXP[20].Y()+=dy; aXP.SetFlags(20,XPOLY_CONTROL);
                        aXP[21]=aR.LeftCenter();
                        aXP[22]=aR.LeftCenter();   aXP[22].Y()-=dy; aXP.SetFlags(22,XPOLY_CONTROL);
                        aXP[23]=aR.TopLeft();      aXP[23].Y()+=dy; aXP.SetFlags(23,XPOLY_CONTROL);
                        aXP[24]=aR.TopLeft();
                        if (bMorePoints) {
                            dx=-dx; dy=-dy;
                            for (USHORT i=aXP.GetPointCount(); i>1;) {
                                i--;
                                Point aPnt(aXP[i]);
                                aPnt+=aXP[i-3];
                                aPnt.X()/=2;
                                aPnt.Y()/=2;
                                USHORT nc1=USHORT(i-1);
                                USHORT nc2=USHORT(i+1);
                                BOOL bHor=aXP[i].Y()==aXP[i-3].Y();
                                aXP.Insert(nc1,aPnt,XPOLY_CONTROL); if (bHor) aXP[nc1].X()-=dx; else aXP[nc1].Y()-=dy;
                                aXP.Insert(i  ,aPnt,XPOLY_NORMAL);
                                aXP.Insert(nc2,aPnt,XPOLY_CONTROL); if (bHor) aXP[nc2].X()+=dx; else aXP[nc2].Y()+=dy;
                                if (i==15) { dx=-dx; dy=-dy; }
                                i-=2;
                            }
                        }
                        pPV->DragPoly0()=XPolyPolygon(aXP);
                    } else {
                        XPolygon aXP(aR);
                        pPV->DragPoly0()=XPolyPolygon(aXP);
                    }
                    pPV->DragPoly()=pPV->DragPoly0();
                }
            } else {
                pPV->DragPoly().Clear();
            }
        }
    }
}

BOOL SdrDragView::TakeDragObjAnchorPos(Point& rPos) const
{
    Rectangle aR;
    TakeActionRect(aR);
    rPos=aR.TopLeft();
    if (aMark.GetMarkCount()==1 && IsDragObj() && // nur bei Einzelselektion
        !IsDraggingPoints() && !IsDraggingGluePoints() && // nicht beim Punkteschieben
        !pDragBla->ISA(SdrDragMovHdl)) // nicht beim Handlesschieben
    {
        SdrObject* pObj=aMark.GetMark(0)->GetObj();
        if (pObj->ISA(SdrCaptionObj)) {
            Point aPt(((SdrCaptionObj*)pObj)->GetTailPos());
            BOOL bTail=eDragHdl==HDL_POLY; // Schwanz wird gedraggt (nicht so ganz feine Abfrage hier)
            BOOL bMove=pDragBla->ISA(SdrDragMove);  // Move des gesamten Obj
            BOOL bOwn=pDragBla->ISA(SdrDragObjOwn); // Objektspeziefisch
            if (!bTail) { // bei bTail liefert TakeActionRect schon das richtige
                if (bOwn) { // bOwn kann sein MoveTextFrame, ResizeTextFrame aber eben nicht mehr DragTail
                    rPos=aPt;
                } else {
                    // hier nun dragging des gesamten Objekts (Move, Resize, ...)
                    pDragBla->MovPoint(aPt,aMark.GetMark(0)->GetPageView()->GetOffset());
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrDragView::TakeDragLimit(SdrDragMode eMode, Rectangle& rRect) const
{
    return FALSE;
}

BOOL SdrDragView::BegDragObj(const Point& rPnt, OutputDevice* pOut, SdrHdl* pHdl, short nMinMov, SdrDragMethod* pForcedMeth)
{
    BrkAction();
    BOOL bRet=FALSE;
    {
        SetDragWithCopy(FALSE);
        ForceEdgesOfMarkedNodes();
        aAni.Reset();
        pDragBla=NULL;
        bDragSpecial=FALSE;
        bDragLimit=FALSE;
        SdrDragMode eTmpMode=eDragMode;
        if (eTmpMode==SDRDRAG_MOVE && pHdl!=NULL && pHdl->GetKind()!=HDL_MOVE) {
            eTmpMode=SDRDRAG_RESIZE;
        }
        bDragLimit=TakeDragLimit(eTmpMode,aDragLimit);
        bFramDrag=ImpIsFrameHandles();
        if (!bFramDrag &&
            (pMarkedObj==NULL || !pMarkedObj->HasSpecialDrag()) &&
            (pHdl==NULL || pHdl->GetObj()==NULL)) {
            bFramDrag=TRUE;
        }

        Point aPnt(rPnt);
        if(pHdl == NULL
            || pHdl->GetKind() == HDL_MIRX
            || pHdl->GetKind() == HDL_TRNS
            || pHdl->GetKind() == HDL_GRAD)
        {
            aDragStat.Reset(aPnt);
        }
        else
        {
            aDragStat.Reset(pHdl->GetPos());
        }

        aDragStat.SetView((SdrView*)this);
        aDragStat.SetPageView(pMarkedPV);  // <<-- hier muss die DragPV rein!!!
        aDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
        aDragStat.SetHdl(pHdl);
        aDragStat.NextPoint();
        pDragWin=pOut;
        pDragHdl=pHdl;
        eDragHdl= pHdl==NULL ? HDL_MOVE : pHdl->GetKind();
        bDragHdl=eDragHdl==HDL_REF1 || eDragHdl==HDL_REF2 || eDragHdl==HDL_MIRX;
        BOOL bNotDraggable = (eDragHdl == HDL_ANCHOR);

        if(bDragHdl)
        {
            pDragBla = new SdrDragMovHdl(*this);
        }
        else if(!bNotDraggable)
        {
            switch (eDragMode) {
                case SDRDRAG_ROTATE: case SDRDRAG_SHEAR: case SDRDRAG_DISTORT: {
                    switch (eDragHdl) {
                        case HDL_LEFT:  case HDL_RIGHT:
                        case HDL_UPPER: case HDL_LOWER: {
                            // Sind 3D-Objekte selektiert?
                            BOOL b3DObjSelected = FALSE;
                            for(UINT32 a=0;!b3DObjSelected && a<aMark.GetMarkCount();a++)
                            {
                                SdrObject* pObj = aMark.GetMark(a)->GetObj();
                                if(pObj && pObj->ISA(E3dObject))
                                    b3DObjSelected = TRUE;
                            }
                            // Falls ja, Shear auch bei !IsShearAllowed zulassen,
                            // da es sich bei 3D-Objekten um eingeschraenkte
                            // Rotationen handelt
                            if (!b3DObjSelected && !IsShearAllowed())
                                return FALSE;
                            pDragBla=new SdrDragShear(*this,eDragMode==SDRDRAG_ROTATE);
                        } break;
                        case HDL_UPLFT: case HDL_UPRGT:
                        case HDL_LWLFT: case HDL_LWRGT: {
                            if (eDragMode==SDRDRAG_SHEAR || eDragMode==SDRDRAG_DISTORT) {
                                if (!IsDistortAllowed(TRUE) && !IsDistortAllowed(FALSE)) return FALSE;
                                pDragBla=new SdrDragDistort(*this);
                            } else {
                                if (!IsRotateAllowed(TRUE)) return FALSE;
                                pDragBla=new SdrDragRotate(*this);
                            }
                        } break;
                        default: {
                            if (IsMarkedHitMovesAlways() && eDragHdl==HDL_MOVE) { // HDL_MOVE ist auch wenn Obj direkt getroffen
                                if (!IsMoveAllowed()) return FALSE;
                                pDragBla=new SdrDragMove(*this);
                            } else {
                                if (!IsRotateAllowed(TRUE)) return FALSE;
                                pDragBla=new SdrDragRotate(*this);
                            }
                        }
                    }
                } break;
                case SDRDRAG_MIRROR: {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways()) {
                        if (!IsMoveAllowed()) return FALSE;
                        pDragBla=new SdrDragMove(*this);
                    } else {
                        if (!IsMirrorAllowed(TRUE,TRUE)) return FALSE;
                        pDragBla=new SdrDragMirror(*this);
                    }
                } break;

                case SDRDRAG_TRANSPARENCE:
                {
                    if(eDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if(!IsMoveAllowed())
                            return FALSE;
                        pDragBla = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(!IsTransparenceAllowed())
                            return FALSE;

                        pDragBla = new SdrDragGradient(*this, FALSE);
                    }
                    break;
                }
                case SDRDRAG_GRADIENT:
                {
                    if(eDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if(!IsMoveAllowed())
                            return FALSE;
                        pDragBla = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(!IsGradientAllowed())
                            return FALSE;

                        pDragBla = new SdrDragGradient(*this);
                    }
                    break;
                }

                case SDRDRAG_CROOK : {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways()) {
                        if (!IsMoveAllowed()) return FALSE;
                        pDragBla=new SdrDragMove(*this);
                    } else {
                        if (!IsCrookAllowed(TRUE) && !IsCrookAllowed(FALSE)) return FALSE;
                        pDragBla=new SdrDragCrook(*this);
                    }
                } break;

                default: { // SDRDRAG_MOVE
                    if (eDragHdl==HDL_GLUE) {
                        pDragBla=new SdrDragMove(*this);
                    } else {
                        if (bFramDrag==TRUE) {
                            if (eDragHdl==HDL_MOVE) {
                                if (!IsMoveAllowed()) return FALSE;
                                pDragBla=new SdrDragMove(*this);
                            } else {
                                if (!IsResizeAllowed(TRUE)) return FALSE;
                                pDragBla=new SdrDragResize(*this);
                            }
                        } else {
                            if (eDragHdl==HDL_MOVE && !IsMoveAllowed()) return FALSE;
                            {
                                bDragSpecial=TRUE;
                                pDragBla=new SdrDragObjOwn(*this);
                            }
                        }
                    }
                }
            }
        }
        if (pForcedMeth!=NULL) {
            delete pDragBla;
            pDragBla=pForcedMeth;
        }
        aDragStat.SetDragMethod(pDragBla);
        if (pDragBla!=NULL) {
            bRet=pDragBla->Beg();
            if (!bRet) {
                if (pHdl==NULL && IS_TYPE(SdrDragObjOwn,pDragBla)) {
                    // Aha, Obj kann nicht Move SpecialDrag, also MoveFrameDrag versuchen
                    delete pDragBla;
                    pDragBla=NULL;
                    bDragSpecial=FALSE;
                    if (!IsMoveAllowed()) return FALSE;
                    bFramDrag=TRUE;
                    pDragBla=new SdrDragMove(*this);
                    aDragStat.SetDragMethod(pDragBla);
                    bRet=pDragBla->Beg();
                }
            }
            if (!bRet) {
                delete pDragBla;
                pDragBla=NULL;
                aDragStat.SetDragMethod(pDragBla);
            }
        }
    }

    // refresh IAOs
//--/   RefreshAllIAOManagers();

    return bRet;
}

BOOL SdrDragView::IsInsObjPointPossible() const
{
    return pMarkedObj!=NULL && pMarkedObj->IsPolyObj();
}

BOOL SdrDragView::BegInsObjPoint(BOOL bIdxZwang, USHORT nIdx, const Point& rPnt, BOOL bNewObj, OutputDevice* pOut, short nMinMov)
{
    BOOL bRet=FALSE;
    nMinMov=0;
    if (pMarkedObj!=NULL && pMarkedObj->IsPolyObj()) {
        BrkAction();
        pInsPointUndo=new SdrUndoGeoObj(*pMarkedObj);
        XubString aStr(ImpGetResStr(STR_DragInsertPoint));
        XubString aName; pMarkedObj->TakeObjNameSingul(aName);

        xub_StrLen nPos(aStr.SearchAscii("%O"));

        if(nPos != STRING_NOTFOUND)
        {
            aStr.Erase(nPos, 2);
            aStr.Insert(aName, nPos);
        }

        aInsPointUndoStr=aStr;
        Point aPt(rPnt-pMarkedPV->GetOffset());
        if (bNewObj) aPt=GetSnapPos(aPt,pMarkedPV);
        BOOL bClosed0=pMarkedObj->IsClosedObj();
        if (bIdxZwang) {
            nInsPointNum=pMarkedObj->NbcInsPoint(nIdx,aPt,IsInsertAfter(),bNewObj,TRUE);
        } else {
            FASTBOOL bTmpInsAfter = IsInsertAfter();
            nInsPointNum=pMarkedObj->NbcInsPoint(aPt,bNewObj,TRUE,bTmpInsAfter);
        }
        if (bClosed0!=pMarkedObj->IsClosedObj()) { // Obj wurde implizit geschlossen
            pMarkedObj->SendRepaintBroadcast();
        }
        if (nInsPointNum!=0xFFFF) {
            BOOL bVis=IsMarkHdlShown();
            if (bVis) HideMarkHdl(NULL);
            bInsPolyPoint=TRUE;
            UnmarkAllPoints();
            AdjustMarkHdl();
            if (bVis) ShowMarkHdl(NULL);
            bRet=BegDragObj(rPnt,pOut,aHdl.GetHdl(nInsPointNum),0);
            if (bRet) {
                if (nMinMov==0) { // ggf. nicht auf MouseMove warten
                    aDragStat.SetMinMoved();
                    MovDragObj(rPnt);
                }
            }
        } else {
            delete pInsPointUndo;
            pInsPointUndo=NULL;
        }
    }

    // refresh IAOs
//--/   RefreshAllIAOManagers();

    return bRet;
}

BOOL SdrDragView::EndInsObjPoint(SdrCreateCmd eCmd)
{
    if (IsInsObjPoint()) {
        USHORT nNextPnt=nInsPointNum;
        Point aPnt(aDragStat.GetNow());
        USHORT nMinMov=aDragStat.GetMinMove();
        BOOL bOk=EndDragObj(FALSE);
        if (bOk==TRUE && eCmd!=SDRCREATE_FORCEEND) {
            // Ret=True bedeutet: Action ist vorbei.
            bOk=!(BegInsObjPoint(TRUE,nNextPnt,aPnt,eCmd==SDRCREATE_NEXTOBJECT,pDragWin,nMinMov));
        }

        // refresh IAOs
//--/       RefreshAllIAOManagers();

        return bOk;
    } else return FALSE;
}

BOOL SdrDragView::IsInsGluePointPossible() const
{
    BOOL bRet=FALSE;
    if (IsInsGluePointMode() && HasMarkedObj()) {
        if (aMark.GetMarkCount()==1) {
            // FALSE liefern, wenn 1 Objekt und dieses ein Verbinder ist.
            const SdrObject* pObj=aMark.GetMark(0)->GetObj();
            if (!HAS_BASE(SdrEdgeObj,pObj)) {
               bRet=TRUE;
            }
        } else {
            bRet=TRUE;
        }
    }
    return bRet;
}

BOOL SdrDragView::BegInsGluePoint(const Point& rPnt)
{
    BOOL bRet=FALSE;
    SdrObject* pObj;
    SdrPageView* pPV;
    ULONG nMarkNum;
    if (PickMarkedObj(rPnt,pObj,pPV,&nMarkNum,SDRSEARCH_PASS2BOUND)) {
        BrkAction();
        UnmarkAllGluePoints();
        SdrMark* pM=aMark.GetMark(nMarkNum);
        pInsPointUndo=new SdrUndoGeoObj(*pObj);
        XubString aStr(ImpGetResStr(STR_DragInsertGluePoint));
        XubString aName; pObj->TakeObjNameSingul(aName);

        aStr.SearchAndReplaceAscii("%O", aName);

        aInsPointUndoStr=aStr;
        SdrGluePointList* pGPL=pObj->ForceGluePointList();
        if (pGPL!=NULL) {
            USHORT nGlueIdx=pGPL->Insert(SdrGluePoint());
            SdrGluePoint& rGP=(*pGPL)[nGlueIdx];
            USHORT nGlueId=rGP.GetId();
            rGP.SetAbsolutePos(rPnt-pPV->GetOffset(),*pObj);

            SdrHdl* pHdl=NULL;
            if (MarkGluePoint(pObj,nGlueId,pPV)) {
                pHdl=GetGluePointHdl(pObj,nGlueId);
            }
            if (pHdl!=NULL && pHdl->GetKind()==HDL_GLUE && pHdl->GetObj()==pObj && pHdl->GetObjHdlNum()==nGlueId) {
                SetInsertGluePoint(TRUE);
                bRet=BegDragObj(rPnt,NULL,pHdl,0);
                if (bRet) {
                    aDragStat.SetMinMoved();
                    MovDragObj(rPnt);
                } else {
                    SetInsertGluePoint(FALSE);
                    delete pInsPointUndo;
                    pInsPointUndo=NULL;
                }
            } else {
                DBG_ERROR("BegInsGluePoint(): GluePoint-Handle nicht gefunden");
            }
        } else {
            // Keine Klebepunkte moeglich bei diesem Objekt (z.B. Edge)
            SetInsertGluePoint(FALSE);
            delete pInsPointUndo;
            pInsPointUndo=NULL;
        }
    }

    // refresh IAOs
//--/   RefreshAllIAOManagers();

    return bRet;
}

void SdrDragView::MovDragObj(const Point& rPnt)
{
    if (pDragBla!=NULL) {
        Point aPnt(rPnt);
        ImpLimitToWorkArea(aPnt);
        pDragBla->Mov(aPnt);
        if (IsDragHdlHide() && aDragStat.IsMinMoved() && !bDragHdl && IsMarkHdlShown()) {
            BOOL bLeaveRefs=IS_TYPE(SdrDragMirror,pDragBla) || IS_TYPE(SdrDragRotate,pDragBla);
            BOOL bFlag=IsSolidMarkHdl() && aDragStat.IsShown();
            if (bFlag) HideDragObj(pDragWin);
            HideMarkHdl(pDragWin,bLeaveRefs);
            if (bFlag) ShowDragObj(pDragWin);
        }

        // refresh IAOs
//--/       RefreshAllIAOManagers();

    }
}

BOOL SdrDragView::EndDragObj(BOOL bCopy)
{
    BOOL bRet=FALSE;
    if (pDragBla!=NULL && aDragStat.IsMinMoved() && aDragStat.GetNow()!=aDragStat.GetPrev()) {
        ULONG nHdlAnzMerk=0;
        if (bEliminatePolyPoints) { // IBM Special
            nHdlAnzMerk=GetMarkablePointCount();
        }
        if (IsInsertGluePoint()) {
            BegUndo(aInsPointUndoStr);
            AddUndo(pInsPointUndo);
        }
        bRet=pDragBla->End(bCopy);
        if (IsInsertGluePoint()) EndUndo();
        delete pDragBla;
        if (bEliminatePolyPoints) { // IBM Special
            if (nHdlAnzMerk!=GetMarkablePointCount()) {
                UnmarkAllPoints();
            }
        }
        pDragBla=NULL;
        if (bInsPolyPoint) {
            BOOL bVis=IsMarkHdlShown();
            if (bVis) HideMarkHdl(NULL);
            SetMarkHandles();
            bInsPolyPoint=FALSE;
            if (bVis) ShowMarkHdl(NULL);
            BegUndo(aInsPointUndoStr);
            AddUndo(pInsPointUndo);
            EndUndo();
        }
        if (!bSomeObjChgdFlag) { // Aha, Obj hat nicht gebroadcastet (z.B. Writer FlyFrames)
            if (IsDragHdlHide() && !bDragHdl &&
                !IS_TYPE(SdrDragMirror,pDragBla) && !IS_TYPE(SdrDragRotate,pDragBla))
            {
                AdjustMarkHdl();
                ShowMarkHdl(pDragWin);
            }
        }
        eDragHdl=HDL_MOVE;
        pDragHdl=NULL;
        SetDragPolys(TRUE);
//--/       RefreshAllIAOManagers();
    } else {
        BrkDragObj();
    }
    bInsPolyPoint=FALSE;
    SetInsertGluePoint(FALSE);

    // refresh IAOs
//--/   RefreshAllIAOManagers();

    return bRet;
}

void SdrDragView::BrkDragObj()
{
    if (pDragBla!=NULL) {
        pDragBla->Brk();
        delete pDragBla;
        pDragBla=NULL;
        if (bInsPolyPoint) {
            BOOL bVis=IsMarkHdlShown();
            if (bVis) HideMarkHdl(NULL);
            pInsPointUndo->Undo(); // Den eingefuegten Punkt wieder raus
            delete pInsPointUndo;
            pInsPointUndo=NULL;
            SetMarkHandles();
            bInsPolyPoint=FALSE;
            if (bVis) ShowMarkHdl(NULL);
        }
        if (IsInsertGluePoint()) {
            pInsPointUndo->Undo(); // Den eingefuegten Klebepunkt wieder raus
            delete pInsPointUndo;
            pInsPointUndo=NULL;
            SetInsertGluePoint(FALSE);
        }
        if (IsDragHdlHide() && !bDragHdl &&
            !IS_TYPE(SdrDragMirror,pDragBla) && !IS_TYPE(SdrDragRotate,pDragBla))
        {
            ShowMarkHdl(pDragWin);
        }
        eDragHdl=HDL_MOVE;
        pDragHdl=NULL;
        SetDragPolys(TRUE);

        // refresh IAOs
//--/       RefreshAllIAOManagers();

    }
}

void SdrDragView::DrawDragObj(OutputDevice* pOut, BOOL bFull) const
{
    if (pDragBla!=NULL) {
        USHORT i=0;
        do {
            OutputDevice* pO=pOut;
            if (pO==NULL) {
                pO=GetWin(i);
                i++;
            }
            if (pO!=NULL) {
                ImpSdrHdcMerk aHDCMerk(*pO,SDRHDC_SAVEPENANDBRUSH,bRestoreColors);
                RasterOp eRop0=pO->GetRasterOp();
                pO->SetRasterOp(ROP_INVERT);
                pXOut->SetOutDev(pO);
                Color aBlackColor( COL_BLACK );
                Color aTranspColor( COL_TRANSPARENT );
                pXOut->OverrideLineColor( aBlackColor );
                pXOut->OverrideFillColor( aTranspColor );
                pDragBla->DrawXor(*pXOut,bFull);
                pXOut->SetOffset(Point(0,0));
                pO->SetRasterOp(eRop0);
                aHDCMerk.Restore(*pO);
            }
        } while (pOut==NULL && i<GetWinCount());
        if (aAni.IsStripes() && IsDragStripes()) {
            Rectangle aR;
            TakeActionRect(aR);
            aAni.SetP1(aR.TopLeft());
            aAni.SetP2(aR.BottomRight());
            aAni.Invert(pOut);
        }
    }
}

BOOL SdrDragView::IsMoveOnlyDragObj(BOOL bAskRTTI) const
{
    BOOL bRet=FALSE;
    if (pDragBla!=NULL && !IsDraggingPoints() && !IsDraggingGluePoints()) {
        if (bAskRTTI) {
            bRet=IS_TYPE(SdrDragMove,pDragBla);
        } else {
            bRet=pDragBla->IsMoveOnly();
        }
    }
    return bRet;
}

void SdrDragView::ImpDrawEdgeXor(ExtOutputDevice& rXOut, BOOL bFull) const
{
    ULONG nEdgeAnz=aEdgesOfMarkedNodes.GetMarkCount();
    BOOL bNo=(!IsRubberEdgeDragging() && !IsDetailedEdgeDragging()) || nEdgeAnz==0 ||
                 IsDraggingPoints() || IsDraggingGluePoints();
    if (!pDragBla->IsMoveOnly() &&
        !(IS_TYPE(SdrDragMove,pDragBla) || IS_TYPE(SdrDragResize,pDragBla) ||
          IS_TYPE(SdrDragRotate,pDragBla) || IS_TYPE(SdrDragMirror,pDragBla))) bNo=TRUE;
    if (!bNo) {
        BOOL bDetail=IsDetailedEdgeDragging() && pDragBla->IsMoveOnly() &&
                         nEdgeAnz<=nDetailedEdgeDraggingLimit;
        if (!bDetail && !(IsRubberEdgeDragging() ||
                          nEdgeAnz>nRubberEdgeDraggingLimit)) bNo=TRUE;
        if (!bNo) {
            for (USHORT i=0; i<nEdgeAnz; i++) {
                SdrMark* pEM=aEdgesOfMarkedNodes.GetMark(i);
                SdrObject* pEdge=pEM->GetObj();
                SdrPageView* pEPV=pEM->GetPageView();
                pXOut->SetOffset(pEPV->GetOffset());
                pEdge->NspToggleEdgeXor(aDragStat,rXOut,pEM->IsCon1(),pEM->IsCon2(),bDetail);
            }
        }
    }
}

void SdrDragView::ShowDragObj(OutputDevice* pOut)
{
    if (pDragBla!=NULL && !aDragStat.IsShown()) {
        DrawDragObj(pOut,FALSE);
        aDragStat.SetShown(TRUE);
        if (aAni.IsStripes() && IsDragStripes()) {
            aAni.Start();
        }
    }
}

void SdrDragView::HideDragObj(OutputDevice* pOut)
{
    if (pDragBla!=NULL && aDragStat.IsShown()) {
        if (aAni.IsStripes() && IsDragStripes()) aAni.Stop();
        DrawDragObj(pOut,FALSE);
        aDragStat.SetShown(FALSE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::SetNoDragXorPolys(BOOL bOn)
{
    if (IsNoDragXorPolys()!=bOn) {
        BOOL bDragging=pDragBla!=NULL;
        BOOL bShown=bDragging && aDragStat.IsShown();
        if (bShown) HideDragObj(pDragWin);
        bNoDragXorPolys=bOn;
        if (bDragging) {
            SetDragPolys(FALSE,IS_TYPE(SdrDragCrook,pDragBla));
            pDragBla->MovAllPoints(); // die gedraggten Polys neu berechnen
        }
        if (bShown) ShowDragObj(pDragWin);
    }
}

void SdrDragView::SetDragStripes(BOOL bOn)
{
    if (pDragBla!=NULL && aDragStat.IsShown()) {
        HideDragObj(pDragWin);
        bDragStripes=bOn;
        ShowDragObj(pDragWin);
    } else {
        bDragStripes=bOn;
    }
}

void SdrDragView::SetDragHdlHide(BOOL bOn)
{
    bNoDragHdl=bOn;
    if (pDragBla!=NULL && !bDragHdl && !IS_TYPE(SdrDragMirror,pDragBla) && !IS_TYPE(SdrDragRotate,pDragBla))
    {
        if (bOn) HideMarkHdl(pDragWin);
        else ShowMarkHdl(pDragWin);
    }
}

BOOL SdrDragView::IsOrthoDesired() const
{
    if (pDragBla!=NULL && (IS_TYPE(SdrDragObjOwn,pDragBla) || IS_TYPE(SdrDragResize,pDragBla))) {
        return bOrthoDesiredOnMarked;
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::SetRubberEdgeDragging(BOOL bOn)
{
    if (bOn!=IsRubberEdgeDragging()) {
        ULONG nAnz=aEdgesOfMarkedNodes.GetMarkCount();
        BOOL bShowHide=nAnz!=0 && IsDragObj() &&
                 (nRubberEdgeDraggingLimit>=nAnz);
        if (bShowHide) HideDragObj(NULL);
        bRubberEdgeDragging=bOn;
        if (bShowHide) ShowDragObj(NULL);
    }
}

void SdrDragView::SetRubberEdgeDraggingLimit(USHORT nEdgeObjAnz)
{
    if (nEdgeObjAnz!=nRubberEdgeDraggingLimit) {
        ULONG nAnz=aEdgesOfMarkedNodes.GetMarkCount();
        BOOL bShowHide=IsRubberEdgeDragging() && nAnz!=0 && IsDragObj() &&
                 (nEdgeObjAnz>=nAnz)!=(nRubberEdgeDraggingLimit>=nAnz);
        if (bShowHide) HideDragObj(NULL);
        nRubberEdgeDraggingLimit=nEdgeObjAnz;
        if (bShowHide) ShowDragObj(NULL);
    }
}

void SdrDragView::SetDetailedEdgeDragging(BOOL bOn)
{
    if (bOn!=IsDetailedEdgeDragging()) {
        ULONG nAnz=aEdgesOfMarkedNodes.GetMarkCount();
        BOOL bShowHide=nAnz!=0 && IsDragObj() &&
                 (nDetailedEdgeDraggingLimit>=nAnz);
        if (bShowHide) HideDragObj(NULL);
        bDetailedEdgeDragging=bOn;
        if (bShowHide) ShowDragObj(NULL);
    }
}

void SdrDragView::SetDetailedEdgeDraggingLimit(USHORT nEdgeObjAnz)
{
    if (nEdgeObjAnz!=nDetailedEdgeDraggingLimit) {
        ULONG nAnz=aEdgesOfMarkedNodes.GetMarkCount();
        BOOL bShowHide=IsDetailedEdgeDragging() && nAnz!=0 && IsDragObj() &&
                 (nEdgeObjAnz>=nAnz)!=(nDetailedEdgeDraggingLimit>=nAnz);
        if (bShowHide) HideDragObj(NULL);
        nDetailedEdgeDraggingLimit=nEdgeObjAnz;
        if (bShowHide) ShowDragObj(NULL);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::WriteRecords(SvStream& rOut) const
{
    SdrExchangeView::WriteRecords(rOut);
    {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWDRAGSTRIPES);
        rOut<<(BOOL)bDragStripes;
    } {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWDRAGHIDEHDL);
        rOut<<(BOOL)bNoDragHdl;
    } {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWOBJHITMOVES);
        rOut<<(BOOL)bMarkedHitMovesAlways;
    } {
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWMIRRDRAGOBJ);
        rOut<<(BOOL)bMirrRefDragObj;
    }
}

BOOL SdrDragView::ReadRecord(const SdrIOHeader& rViewHead,
    const SdrNamedSubRecord& rSubHead,
    SvStream& rIn)
{
    BOOL bRet=FALSE;
    if (rSubHead.GetInventor()==SdrInventor) {
        bRet=TRUE;
        switch (rSubHead.GetIdentifier()) {
            case SDRIORECNAME_VIEWDRAGSTRIPES: {
                BOOL bZwi; rIn >> bZwi; bDragStripes = bZwi;
            } break;
            case SDRIORECNAME_VIEWDRAGHIDEHDL: {
                BOOL bZwi; rIn >> bZwi; bNoDragHdl = bZwi;
            } break;
            case SDRIORECNAME_VIEWOBJHITMOVES: {
                BOOL bZwi; rIn >> bZwi; bMarkedHitMovesAlways = bZwi;
            } break;
            case SDRIORECNAME_VIEWMIRRDRAGOBJ: {
                BOOL bZwi; rIn >> bZwi; bMirrRefDragObj = bZwi;
            } break;
            default: bRet=FALSE;
        }
    }
    if (!bRet) bRet=SdrExchangeView::ReadRecord(rViewHead,rSubHead,rIn);
    return bRet;
}

