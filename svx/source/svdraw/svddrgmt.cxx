/*************************************************************************
 *
 *  $RCSfile: svddrgmt.cxx,v $
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

#include "svddrgm1.hxx"
#include <math.h>

#ifndef _MATH_H
#define _MATH_H
#endif

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif

#include "xattr.hxx"
#include "xpoly.hxx"
#include "svdxout.hxx"
#include "svdetc.hxx"
#include "svdtrans.hxx"
#include "svdundo.hxx"
#include "svdmark.hxx"
#include "svdocapt.hxx"
#include "svdpagv.hxx"
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache
#include "svddrgv.hxx"
#include "svdundo.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT0(SdrDragMethod);

void SdrDragMethod::ImpTakeDescriptionStr(USHORT nStrCacheID, XubString& rStr, USHORT nVal) const
{
    USHORT nOpt=0;
    if (IsDraggingPoints()) {
        nOpt=IMPSDR_POINTSDESCRIPTION;
    } else if (IsDraggingGluePoints()) {
        nOpt=IMPSDR_GLUEPOINTSDESCRIPTION;
    }
    rView.ImpTakeDescriptionStr(nStrCacheID,rStr,nVal,nOpt);
}

SdrObject* SdrDragMethod::GetDragObj() const
{
    SdrObject* pObj=NULL;
    if (rView.pDragHdl!=NULL) pObj=rView.pDragHdl->GetObj();
    if (pObj==NULL) pObj=rView.pMarkedObj;
    return pObj;
}

SdrPageView* SdrDragMethod::GetDragPV() const
{
    SdrPageView* pPV=NULL;
    if (rView.pDragHdl!=NULL) pPV=rView.pDragHdl->GetPageView();
    if (pPV==NULL) pPV=rView.pMarkedPV;
    return pPV;
}

void SdrDragMethod::Draw() const
{
    rView.DrawDragObj(rView.pDragWin,TRUE);
}

void SdrDragMethod::Show()
{
    rView.ShowDragObj(rView.pDragWin);
}

void SdrDragMethod::Hide()
{
    rView.HideDragObj(rView.pDragWin);
}

void SdrDragMethod::MovAllPoints()
{
    USHORT nPvAnz=rView.GetPageViewCount();
    for (USHORT nv=0; nv<nPvAnz; nv++) {
        SdrPageView* pPV=rView.GetPageViewPvNum(nv);
        if (pPV->HasMarkedObj()) {
            pPV->DragPoly()=pPV->DragPoly0();
            USHORT i,j;
            USHORT nPolyAnz=pPV->DragPoly().Count();
            for (j=0; j<nPolyAnz; j++) {
                XPolygon& aPol=pPV->DragPoly()[j];
                Point aOfs(pPV->GetOffset());
                USHORT nPtAnz=aPol.GetPointCount();
                for (i=0; i<nPtAnz; i++) {
                    MovPoint(aPol[i],aOfs);
                }
            }
        }
    }
}

void SdrDragMethod::MovPoint(Point& rPnt, const Point& rPvOfs)
{
}

void SdrDragMethod::Brk()
{
    Hide();
}

FASTBOOL SdrDragMethod::IsMoveOnly() const
{
    return FALSE;
}

void SdrDragMethod::DrawXor(ExtOutputDevice& rXOut, FASTBOOL bFull) const
{
    FASTBOOL bGlue=IsDraggingGluePoints();
    FASTBOOL bPoints=IsDraggingPoints() || bGlue;
    OutputDevice* pOut=rXOut.GetOutDev();
    long x=0,y=0;
    if (bPoints) {
        long nSiz=bGlue ? 3 : rView.aHdl.GetHdlSize();
        Size aSiz(pOut->PixelToLogic(Size(nSiz,nSiz)));
        x=aSiz.Width();
        y=aSiz.Height();
        pOut->SetLineColor( Color( COL_BLACK ) );
    }
    USHORT nPvAnz=rView.GetPageViewCount();
    for (USHORT nv=0; nv<nPvAnz; nv++)
    {
        SdrPageView* pPV=rView.GetPageViewPvNum(nv);
        if (pPV->HasMarkedObj())
        {
            rXOut.SetOffset(pPV->GetOffset());
            const XPolyPolygon& rXPP=pPV->DragPoly();
            USHORT nPolyAnz=rXPP.Count();
            for (USHORT nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++)
            {
                const XPolygon& rXP=rXPP[nPolyNum];
                USHORT nPtAnz=rXP.GetPointCount();
                if (bPoints)
                {
                    for (USHORT nPtNum=0; nPtNum<nPtAnz; nPtNum++)
                    {
                        Point aPt(rXP[nPtNum]);
                        long x1=aPt.X()-x;
                        long x2=aPt.X()+x;
                        long y1=aPt.Y()-y;
                        long y2=aPt.Y()+y;
                        if (bGlue)
                        { // ein Kreuzlein an jedem Punkt
                            pOut->DrawLine(Point(x1,y1),Point(x2,y2));
                            pOut->DrawLine(Point(x1,y2),Point(x2,y1));
                        }
                        else
                        { // oder ein Kaestlein
                            pOut->DrawLine(Point(x1,y1),Point(x2,y1));
                            pOut->DrawLine(Point(x1,y2),Point(x2,y2));
                            pOut->DrawLine(Point(x1,y1),Point(x1,y2));
                            pOut->DrawLine(Point(x2,y1),Point(x2,y2));
                        }
                    }
                }
                else
                {
                    if (rXOut.GetOutDev()->GetOutDevType() == OUTDEV_WINDOW)
                    {
                        Rectangle aRect;
                        Window* pWin = (Window*) rXOut.GetOutDev();

                        if ((nPtAnz == 5 || nPtAnz == 6) &&
                            rXP.GetFlags(1) != XPOLY_CONTROL)
                        {
                            /**************************************************
                            * Ist es ein Rechteck? (keine Bezier-Kurve)
                            **************************************************/
                            if (nPtAnz == 6              &&
                                rXP[0].Y() == rXP[1].Y() &&
                                rXP[1].X() == rXP[2].X() &&
                                rXP[2].Y() == rXP[3].Y() &&
                                rXP[3].X() == rXP[4].X() &&
                                rXP[4].Y() == rXP[5].Y() &&
                                rXP[5]     == rXP[0])
                            {
                                // Spezielles Dragging-Rechteck (Drehsinn rechts)
                                aRect = Rectangle(rXP[2], rXP[4]);
                            }
                            else if (nPtAnz == 5              &&
                                     rXP[0].Y() == rXP[1].Y() &&
                                     rXP[1].X() == rXP[2].X() &&
                                     rXP[2].Y() == rXP[3].Y() &&
                                     rXP[3].X() == rXP[4].X() &&
                                     rXP[4]     == rXP[0])
                            {
                                // Rechteck (Drehsinn links)
                                aRect = rXP.GetBoundRect(pWin);
                            }
                            else if (nPtAnz == 5              &&
                                     rXP[0].X() == rXP[1].X() &&
                                     rXP[1].Y() == rXP[2].Y() &&
                                     rXP[2].X() == rXP[3].X() &&
                                     rXP[3].Y() == rXP[4].Y() &&
                                     rXP[4]     == rXP[0])
                            {
                                // Rechteck (Drehsinn rechts)
                                aRect = rXP.GetBoundRect(pWin);
                            }
                        }

                        if (!aRect.IsEmpty())
                        {
                            pWin->InvertTracking(aRect, SHOWTRACK_OBJECT | SHOWTRACK_WINDOW);
                        }
                        else
                        {
                            const Polygon aPoly( XOutCreatePolygon(rXP, pWin) );
                            pWin->InvertTracking(aPoly, SHOWTRACK_WINDOW);
                        }
                    }
                    else
                    {
                        rXOut.DrawXPolyLine(rXP);
                    }
                }
            }
        }
    }
    rView.ImpDrawEdgeXor(rXOut,bFull);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragMovHdl,SdrDragMethod);

void SdrDragMovHdl::TakeComment(XubString& rStr) const
{
    rStr=ImpGetResStr(STR_DragMethMovHdl);
    if (rView.IsDragWithCopy()) rStr+=ImpGetResStr(STR_EditWithCopy);
}

FASTBOOL SdrDragMovHdl::Beg()
{
    DragStat().Ref1()=GetDragHdl()->GetPos();
    DragStat().SetShown(!DragStat().IsShown());
    SdrHdlKind eKind=GetDragHdl()->GetKind();
    SdrHdl* pH1=GetHdlList().GetHdl(HDL_REF1);
    SdrHdl* pH2=GetHdlList().GetHdl(HDL_REF2);
    if (eKind==HDL_MIRX) {
        if (pH1==NULL || pH2==NULL) {
            DBG_ERROR("SdrDragMovHdl::Beg(): Verschieben der Spiegelachse: Referenzhandles nicht gefunden");
            return FALSE;
        }
        DragStat().SetActionRect(Rectangle(pH1->GetPos(),pH2->GetPos()));
    } else {
        Point aPt(GetDragHdl()->GetPos());
        DragStat().SetActionRect(Rectangle(aPt,aPt));
    }
    return TRUE;
}

void SdrDragMovHdl::Mov(const Point& rNoSnapPnt)
{
    Point aPnt(rNoSnapPnt);
    if (DragStat().CheckMinMoved(rNoSnapPnt)) {
        if (GetDragHdl()->GetKind()==HDL_MIRX) {
            SdrHdl* pH1=GetHdlList().GetHdl(HDL_REF1);
            SdrHdl* pH2=GetHdlList().GetHdl(HDL_REF2);
            if (pH1==NULL || pH2==NULL) return;
            if (!DragStat().IsNoSnap()) {
                long nBestXSnap=0;
                long nBestYSnap=0;
                BOOL bXSnapped=FALSE;
                BOOL bYSnapped=FALSE;
                Point aDif(aPnt-DragStat().GetStart());
                rView.CheckSnap(Ref1()+aDif,NULL,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
                rView.CheckSnap(Ref2()+aDif,NULL,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
                aPnt.X()+=nBestXSnap;
                aPnt.Y()+=nBestYSnap;
            }
            if (aPnt!=DragStat().GetNow()) {
                Hide();
                DragStat().NextMove(aPnt);
                Point aDif(DragStat().GetNow()-DragStat().GetStart());
                pH1->SetPos(Ref1()+aDif);
                pH2->SetPos(Ref2()+aDif);

                SdrHdl* pHM = GetHdlList().GetHdl(HDL_MIRX);
                if(pHM)
                    pHM->Touch();

                Show();
                DragStat().SetActionRect(Rectangle(pH1->GetPos(),pH2->GetPos()));
            }
        } else {
            if (!DragStat().IsNoSnap()) SnapPos(aPnt);
            long nSA=0;
            if (rView.IsAngleSnapEnabled()) nSA=rView.GetSnapAngle();
            if (rView.IsMirrorAllowed(TRUE,TRUE)) { // eingeschraenkt
                if (!rView.IsMirrorAllowed(FALSE,FALSE)) nSA=4500;
                if (!rView.IsMirrorAllowed(TRUE,FALSE)) nSA=9000;
            }
            if (rView.IsOrtho() && nSA!=9000) nSA=4500;
            if (nSA!=0) { // Winkelfang
                SdrHdlKind eRef=HDL_REF1;
                if (GetDragHdl()->GetKind()==HDL_REF1) eRef=HDL_REF2;
                SdrHdl* pH=GetHdlList().GetHdl(eRef);
                if (pH!=NULL) {
                    Point aRef(pH->GetPos());
                    long nWink=NormAngle360(GetAngle(aPnt-aRef));
                    long nNeuWink=nWink;
                    nNeuWink+=nSA/2;
                    nNeuWink/=nSA;
                    nNeuWink*=nSA;
                    nNeuWink=NormAngle360(nNeuWink);
                    double a=(nNeuWink-nWink)*nPi180;
                    double nSin=sin(a);
                    double nCos=cos(a);
                    RotatePoint(aPnt,aRef,nSin,nCos);
                    // Bei bestimmten Werten Rundungsfehler ausschliessen:
                    if (nSA==9000) {
                        if (nNeuWink==0    || nNeuWink==18000) aPnt.Y()=aRef.Y();
                        if (nNeuWink==9000 || nNeuWink==27000) aPnt.X()=aRef.X();
                    }
                    if (nSA==4500) OrthoDistance8(aRef,aPnt,TRUE);
                }
            }
            if (aPnt!=DragStat().GetNow()) {
                Hide();
                DragStat().NextMove(aPnt);
                GetDragHdl()->SetPos(DragStat().GetNow());
                SdrHdl* pHM = GetHdlList().GetHdl(HDL_MIRX);
                if(pHM)
                    pHM->Touch();
                Show();
                DragStat().SetActionRect(Rectangle(aPnt,aPnt));
            }
        }
    }
}

FASTBOOL SdrDragMovHdl::End(FASTBOOL bCopy)
{
    switch (GetDragHdl()->GetKind()) {
        case HDL_REF1: Ref1()=DragStat().GetNow(); break;
        case HDL_REF2: Ref2()=DragStat().GetNow(); break;
        case HDL_MIRX: {
            Ref1()+=DragStat().GetNow()-DragStat().GetStart();
            Ref2()+=DragStat().GetNow()-DragStat().GetStart();
        } break;
    }
    return TRUE;
}

void SdrDragMovHdl::Brk()
{
    Hide();
    GetDragHdl()->SetPos(DragStat().GetRef1());
    SdrHdl* pHM = GetHdlList().GetHdl(HDL_MIRX);
    if(pHM)
        pHM->Touch();
    Draw();
}

void SdrDragMovHdl::Show()
{
    SdrHdl* pDragHdl=GetDragHdl();
    SdrHdlKind eDragHdl=pDragHdl->GetKind();
    FASTBOOL bMirX=eDragHdl==HDL_MIRX;
    FASTBOOL bShown=DragStat().IsShown();
    if (rView.IsSolidMarkHdl()) {
        if (!bShown) {
            const SdrHdlList& rHL=GetHdlList();
            SdrHdlKind eHdl1=eDragHdl;
            SdrHdlKind eHdl2=eDragHdl;
            if (bMirX) { eHdl1=HDL_REF1; eHdl2=HDL_REF2; }
            SdrHdl* pHdl1=rHL.GetHdl(eHdl1);
            SdrHdl* pHdl2=rHL.GetHdl(eHdl2);
            ULONG nHdlNum1=rHL.GetHdlNum(pHdl1);
            ULONG nHdlNum2=rHL.GetHdlNum(pHdl2);
            if (nHdlNum1==CONTAINER_ENTRY_NOTFOUND || pHdl1==NULL) {
                DBG_ERROR("SdrDragMovHdl::Show(): Handle nicht in der HandleList gefunden!");
                return;
            }
            if (bMirX && (nHdlNum2==CONTAINER_ENTRY_NOTFOUND || pHdl2==NULL)) {
                DBG_ERROR("SdrDragMovHdl::Show(): Handle nicht in der HandleList gefunden!");
                return;
            }
        }
    }
    SdrDragMethod::Show();
}

void SdrDragMovHdl::Hide()
{
    SdrHdl* pDragHdl=GetDragHdl();
    SdrHdlKind eDragHdl=pDragHdl->GetKind();
    FASTBOOL bMirX=eDragHdl==HDL_MIRX;
    FASTBOOL bShown=DragStat().IsShown();
    SdrDragMethod::Hide();
    if (rView.IsSolidMarkHdl()) {
        if (bShown) {
            const SdrHdlList& rHL=GetHdlList();
            SdrHdlKind eHdl1=eDragHdl;
            SdrHdlKind eHdl2=eDragHdl;
            if (bMirX) { eHdl1=HDL_REF1; eHdl2=HDL_REF2; }
            SdrHdl* pHdl1=rHL.GetHdl(eHdl1);
            SdrHdl* pHdl2=rHL.GetHdl(eHdl2);
            ULONG nHdlNum1=rHL.GetHdlNum(pHdl1);
            ULONG nHdlNum2=rHL.GetHdlNum(pHdl2);
            if (nHdlNum1==CONTAINER_ENTRY_NOTFOUND || pHdl1==NULL) {
                DBG_ERROR("SdrDragMovHdl::Hide(): Handle nicht in der HandleList gefunden!");
                return;
            }
            if (bMirX && (nHdlNum2==CONTAINER_ENTRY_NOTFOUND || pHdl2==NULL)) {
                DBG_ERROR("SdrDragMovHdl::Hide(): Handle nicht in der HandleList gefunden!");
                return;
            }
        }
    }
}

Pointer SdrDragMovHdl::GetPointer() const
{
    const SdrHdl* pHdl=GetDragHdl();
    if (pHdl!=NULL) {
        return pHdl->GetPointer();
    }
    return Pointer(POINTER_REFHAND);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragObjOwn,SdrDragMethod);

void SdrDragObjOwn::TakeComment(XubString& rStr) const
{
    const SdrObject* pObj=GetDragObj();
    if (pObj!=NULL) {
        rStr=pObj->GetDragComment(DragStat(),FALSE,FALSE);
        // Kein Copy bei ObjOwn
    }
}

FASTBOOL SdrDragObjOwn::Beg()
{
    SdrObject* pObj=GetDragObj();
    return pObj!=NULL ? pObj->BegDrag(DragStat()) : FALSE;
}

void SdrDragObjOwn::Mov(const Point& rNoSnapPnt)
{
    Point aPnt(rNoSnapPnt);
    SdrPageView* pPV=GetDragPV();
    if (pPV!=NULL) {
        if (!DragStat().IsNoSnap()) SnapPos(aPnt);
        FASTBOOL bOrtho=rView.IsOrtho();
        if (rView.IsOrtho()) {
            if (DragStat().IsOrtho8Possible()) OrthoDistance8(DragStat().GetStart(),aPnt,rView.IsBigOrtho());
            else if (DragStat().IsOrtho4Possible()) OrthoDistance4(DragStat().GetStart(),aPnt,rView.IsBigOrtho());
        }
        const SdrHdl* pHdl=DragStat().GetHdl();
        if (pHdl!=NULL) {
            aPnt-=pPV->GetOffset();
        }
        SdrObject* pObj=GetDragObj();
        if (pObj!=NULL && DragStat().CheckMinMoved(/*aPnt*/rNoSnapPnt)) {
            if (aPnt!=DragStat().GetNow()) {
                Hide();
                DragStat().NextMove(aPnt);
                pObj->MovDrag(DragStat());
                pObj->TakeDragPoly(DragStat(),pPV->DragPoly());
                Show();
            }
        }
    }
}

FASTBOOL SdrDragObjOwn::End(FASTBOOL bCopy)
{
    Hide();
    SdrUndoAction* pUndo=NULL;
    SdrUndoAction* pUndo2=NULL;
    FASTBOOL bRet=FALSE;
    SdrObject* pObj=GetDragObj();
    if (pObj!=NULL) {
        if (!rView.IsInsObjPoint()) {
            if (DragStat().IsEndDragChangesAttributes()) {
                pUndo=new SdrUndoAttrObj(*pObj);
                if (DragStat().IsEndDragChangesGeoAndAttributes()) {
                    pUndo2=new SdrUndoGeoObj(*pObj);
                }
            } else {
                pUndo=new SdrUndoGeoObj(*pObj);
            }
        }
        bRet=pObj->EndDrag(DragStat());
        if (bRet) {
            if (pUndo!=NULL) {
                if (pUndo2!=NULL) {
                    rView.BegUndo();
                }
                rView.AddUndo(pUndo);
                if (pUndo2!=NULL) {
                    rView.AddUndo(pUndo2);
                    rView.EndUndo();
                }
            }
        } else {
            delete pUndo;
        }
    }
    return bRet;
}

void SdrDragObjOwn::DrawXor(ExtOutputDevice& rXOut, FASTBOOL bFull) const
{
    SdrPageView* pPV=GetDragPV();
    if (pPV!=NULL) {
        rXOut.SetOffset(pPV->GetOffset());
        const XPolyPolygon& rXPP=pPV->DragPoly();
        OutputDevice* pOut = rXOut.GetOutDev();
        USHORT nAnz=rXPP.Count();
        for (USHORT i=0; i<nAnz; i++) {
            if (pOut->GetOutDevType() == OUTDEV_WINDOW)
            {
                const Polygon aPolygon( XOutCreatePolygon(rXPP[i], pOut) );
                ((Window*) pOut)->InvertTracking(aPolygon, SHOWTRACK_WINDOW);
            }
            else
            {
                rXOut.DrawXPolyLine(rXPP[i]);
            }
        }
    }
}

Pointer SdrDragObjOwn::GetPointer() const
{
    const SdrHdl* pHdl=GetDragHdl();
    if (pHdl!=NULL) {
        return pHdl->GetPointer();
    }
    return Pointer(POINTER_MOVE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragMove,SdrDragMethod);

void SdrDragMove::TakeComment(XubString& rStr) const
{
    XubString aStr;

    ImpTakeDescriptionStr(STR_DragMethMove, rStr);
    rStr.AppendAscii(" (x=");
    rView.GetModel()->TakeMetricStr(DragStat().GetDX(), aStr);
    rStr += aStr;
    rStr.AppendAscii(" y=");
    rView.GetModel()->TakeMetricStr(DragStat().GetDY(), aStr);
    rStr += aStr;
    rStr += sal_Unicode(')');

    if(rView.IsDragWithCopy())
    {
        if(!rView.IsInsObjPoint() && !rView.IsInsGluePoint())
        {
            rStr += ImpGetResStr(STR_EditWithCopy);
        }
    }
}

FASTBOOL SdrDragMove::Beg()
{
    SetDragPolys();
    SetStripes(TRUE);
    DragStat().SetActionRect(GetMarkedRect());
    Show();
    return TRUE;
}

void SdrDragMove::MovAllPoints()
{
    USHORT nPvAnz=rView.GetPageViewCount();
    for (USHORT nv=0; nv<nPvAnz; nv++) {
        SdrPageView* pPV=rView.GetPageViewPvNum(nv);
        if (pPV->HasMarkedObj()) {
            pPV->DragPoly()=pPV->DragPoly0();
            pPV->DragPoly().Move(DragStat().GetDX(),DragStat().GetDY());
        }
    }
}

void SdrDragMove::MovPoint(Point& rPnt, const Point& rPvOfs)
{
    rPnt.X()+=DragStat().GetDX();
    rPnt.Y()+=DragStat().GetDY();
}

void SdrDragMove::ImpCheckSnap(const Point& rPt)
{
    Point aPt(rPt);
    USHORT nRet=SnapPos(aPt);
    aPt-=rPt;
    if ((nRet & SDRSNAP_XSNAPPED) !=0) {
        if (bXSnapped) {
            if (Abs(aPt.X())<Abs(nBestXSnap)) {
                nBestXSnap=aPt.X();
            }
        } else {
            nBestXSnap=aPt.X();
            bXSnapped=TRUE;
        }
    }
    if ((nRet & SDRSNAP_YSNAPPED) !=0) {
        if (bYSnapped) {
            if (Abs(aPt.Y())<Abs(nBestYSnap)) {
                nBestYSnap=aPt.Y();
            }
        } else {
            nBestYSnap=aPt.Y();
            bYSnapped=TRUE;
        }
    }
}

void SdrDragMove::Mov(const Point& rNoSnapPnt_)
{
    nBestXSnap=0;
    nBestYSnap=0;
    bXSnapped=FALSE;
    bYSnapped=FALSE;
    Point aNoSnapPnt(rNoSnapPnt_);
    const SdrHdl* pHdl=DragStat().GetHdl();
    if (pHdl!=NULL && pHdl->GetPageView()!=NULL) {
        aNoSnapPnt-=pHdl->GetPageView()->GetOffset();
    }
    const Rectangle& aSR=GetMarkedRect();
    long nMovedx=aNoSnapPnt.X()-DragStat().GetStart().X();
    long nMovedy=aNoSnapPnt.Y()-DragStat().GetStart().Y();
    Point aLO(aSR.TopLeft());      aLO.X()+=nMovedx; aLO.Y()+=nMovedy;
    Point aRU(aSR.BottomRight());  aRU.X()+=nMovedx; aRU.Y()+=nMovedy;
    Point aLU(aLO.X(),aRU.Y());
    Point aRO(aRU.X(),aLO.Y());
    ImpCheckSnap(aLO);
    if (!rView.IsMoveSnapOnlyTopLeft()) {
        ImpCheckSnap(aRO);
        ImpCheckSnap(aLU);
        ImpCheckSnap(aRU);
    }
    Point aPnt(aNoSnapPnt.X()+nBestXSnap,aNoSnapPnt.Y()+nBestYSnap);
    FASTBOOL bOrtho=rView.IsOrtho();
    if (bOrtho) OrthoDistance8(DragStat().GetStart(),aPnt,rView.IsBigOrtho());
    if (DragStat().CheckMinMoved(aNoSnapPnt)) {
        Point aPt1(aPnt);
        Rectangle aLR(rView.GetWorkArea());
        FASTBOOL bWorkArea=!aLR.IsEmpty();
        FASTBOOL bDragLimit=IsDragLimit();
        if (bDragLimit || bWorkArea) {
            Rectangle aSR(GetMarkedRect());
            Point aD(aPt1-DragStat().GetStart());
            if (bDragLimit) {
                Rectangle aR2(GetDragLimitRect());
                if (bWorkArea) aLR.Intersection(aR2);
                else aLR=aR2;
            }
            if (aSR.Left()>aLR.Left() || aSR.Right()<aLR.Right()) { // ist ueberhaupt Platz zum verschieben?
                aSR.Move(aD.X(),0);
                if (aSR.Left()<aLR.Left()) {
                    aPt1.X()-=aSR.Left()-aLR.Left();
                } else if (aSR.Right()>aLR.Right()) {
                    aPt1.X()-=aSR.Right()-aLR.Right();
                }
            } else aPt1.X()=DragStat().GetStart().X(); // kein Platz zum verschieben
            if (aSR.Top()>aLR.Top() || aSR.Bottom()<aLR.Bottom()) { // ist ueberhaupt Platz zum verschieben?
                aSR.Move(0,aD.Y());
                if (aSR.Top()<aLR.Top()) {
                    aPt1.Y()-=aSR.Top()-aLR.Top();
                } else if (aSR.Bottom()>aLR.Bottom()) {
                    aPt1.Y()-=aSR.Bottom()-aLR.Bottom();
                }
            } else aPt1.Y()=DragStat().GetStart().Y(); // kein Platz zum verschieben
        }
        if (rView.IsDraggingGluePoints()) { // Klebepunkte aufs BoundRect des Obj limitieren
            aPt1-=DragStat().GetStart();
            const SdrMarkList& rML=GetMarkList();
            ULONG nMarkAnz=rML.GetMarkCount();
            for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
                const SdrMark* pM=rML.GetMark(nMarkNum);
                const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
                ULONG nPtAnz=pPts==NULL ? 0 : pPts->GetCount();
                if (nPtAnz!=0) {
                    const SdrObject* pObj=pM->GetObj();
                    const SdrPageView* pPV=pM->GetPageView();
                    const SdrGluePointList* pGPL=pObj->GetGluePointList();
                    Rectangle aBound(pObj->GetBoundRect());
                    for (ULONG nPtNum=0; nPtNum<nPtAnz; nPtNum++) {
                        USHORT nId=pPts->GetObject(nPtNum);
                        USHORT nGlueNum=pGPL->FindGluePoint(nId);
                        if (nGlueNum!=SDRGLUEPOINT_NOTFOUND) {
                            Point aPt((*pGPL)[nGlueNum].GetAbsolutePos(*pObj));
                            aPt+=aPt1; // soviel soll verschoben werden
                            if (aPt.X()<aBound.Left()  ) aPt1.X()-=aPt.X()-aBound.Left()  ;
                            if (aPt.X()>aBound.Right() ) aPt1.X()-=aPt.X()-aBound.Right() ;
                            if (aPt.Y()<aBound.Top()   ) aPt1.Y()-=aPt.Y()-aBound.Top()   ;
                            if (aPt.Y()>aBound.Bottom()) aPt1.Y()-=aPt.Y()-aBound.Bottom();
                        }
                    }
                }
            }
            aPt1+=DragStat().GetStart();
        }
        if (bOrtho) OrthoDistance8(DragStat().GetStart(),aPt1,FALSE);
        if (aPt1!=DragStat().GetNow()) {
            Hide();
            DragStat().NextMove(aPt1);
            MovAllPoints();
            Rectangle aAction(GetMarkedRect());
            aAction.Move(DragStat().GetDX(),DragStat().GetDY());
            DragStat().SetActionRect(aAction);
            Show();
        }
    }
}

FASTBOOL SdrDragMove::End(FASTBOOL bCopy)
{
    Hide();
    if (rView.IsInsObjPoint() || rView.IsInsGluePoint()) bCopy=FALSE;
    if (IsDraggingPoints()) {
        rView.MoveMarkedPoints(Size(DragStat().GetDX(),DragStat().GetDY()),bCopy);
    } else if (IsDraggingGluePoints()) {
        rView.MoveMarkedGluePoints(Size(DragStat().GetDX(),DragStat().GetDY()),bCopy);
    } else {
        rView.MoveMarkedObj(Size(DragStat().GetDX(),DragStat().GetDY()),bCopy);
    }
    return TRUE;
}

FASTBOOL SdrDragMove::IsMoveOnly() const
{
    return TRUE;
}

Pointer SdrDragMove::GetPointer() const
{
    if (IsDraggingPoints() || IsDraggingGluePoints()) {
        return Pointer(POINTER_MOVEPOINT);
    } else {
        return Pointer(POINTER_MOVE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragResize,SdrDragMethod);

void SdrDragResize::TakeComment(XubString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethResize, rStr);
    BOOL bEqual(aXFact == aYFact);
    Fraction aFact1(1,1);
    Point aStart(DragStat().GetStart());
    Point aRef(DragStat().GetRef1());
    INT32 nXDiv(aStart.X() - aRef.X());

    if(!nXDiv)
        nXDiv = 1;

    INT32 nYDiv(aStart.Y() - aRef.Y());

    if(!nYDiv)
        nYDiv = 1;

    BOOL bX(aXFact != aFact1 && Abs(nXDiv) > 1);
    BOOL bY(aYFact != aFact1 && Abs(nYDiv) > 1);

    if(bX || bY)
    {
        XubString aStr;

        rStr.AppendAscii(" (");

        if(bX)
        {
            if(!bEqual)
                rStr.AppendAscii("x=");

            rView.GetModel()->TakePercentStr(aXFact, aStr);
            rStr += aStr;
        }

        if(bY && !bEqual)
        {
            if(bX)
                rStr += sal_Unicode(' ');

            rStr.AppendAscii("y=");
            rView.GetModel()->TakePercentStr(aYFact, aStr);
            rStr += aStr;
        }

        rStr += sal_Unicode(')');
    }

    if(rView.IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

FASTBOOL SdrDragResize::Beg()
{
    SdrHdlKind eRefHdl=HDL_MOVE;
    SdrHdl* pRefHdl=NULL;
    switch (GetDragHdlKind()) {
        case HDL_UPLFT: eRefHdl=HDL_LWRGT; break;
        case HDL_UPPER: eRefHdl=HDL_LOWER; DragStat().SetHorFixed(TRUE); break;
        case HDL_UPRGT: eRefHdl=HDL_LWLFT; break;
        case HDL_LEFT : eRefHdl=HDL_RIGHT; DragStat().SetVerFixed(TRUE); break;
        case HDL_RIGHT: eRefHdl=HDL_LEFT ; DragStat().SetVerFixed(TRUE); break;
        case HDL_LWLFT: eRefHdl=HDL_UPRGT; break;
        case HDL_LOWER: eRefHdl=HDL_UPPER; DragStat().SetHorFixed(TRUE); break;
        case HDL_LWRGT: eRefHdl=HDL_UPLFT; break;
    }
    if (eRefHdl!=HDL_MOVE) pRefHdl=GetHdlList().GetHdl(eRefHdl);
    if (pRefHdl!=NULL && !rView.IsResizeAtCenter()) {
        DragStat().Ref1()=pRefHdl->GetPos();
    } else {
        SdrHdl* pRef1=GetHdlList().GetHdl(HDL_UPLFT);
        SdrHdl* pRef2=GetHdlList().GetHdl(HDL_LWRGT);
        if (pRef1!=NULL && pRef2!=NULL) {
            DragStat().Ref1()=Rectangle(pRef1->GetPos(),pRef2->GetPos()).Center();
        } else {
            DragStat().Ref1()=GetMarkedRect().Center();
        }
    }
    SetDragPolys();
    Show();
    return TRUE;
}

void SdrDragResize::MovPoint(Point& rPnt, const Point& rPvOfs)
{
    Point aRef(DragStat().Ref1()-rPvOfs);
    ResizePoint(rPnt,aRef,aXFact,aYFact);
}

void SdrDragResize::Mov(const Point& rNoSnapPnt)
{
    Point aPnt(GetSnapPos(rNoSnapPnt));
    Point aStart(DragStat().GetStart());
    Point aRef(DragStat().GetRef1());
    Fraction aMaxFact(0x7FFFFFFF,1);
    Rectangle aLR(rView.GetWorkArea());
    FASTBOOL bWorkArea=!aLR.IsEmpty();
    FASTBOOL bDragLimit=IsDragLimit();
    if (bDragLimit || bWorkArea) {
        Rectangle aSR(GetMarkedRect());
        if (bDragLimit) {
            Rectangle aR2(GetDragLimitRect());
            if (bWorkArea) aLR.Intersection(aR2);
            else aLR=aR2;
        }
        if (aPnt.X()<aLR.Left()) aPnt.X()=aLR.Left();
        else if (aPnt.X()>aLR.Right()) aPnt.X()=aLR.Right();
        if (aPnt.Y()<aLR.Top()) aPnt.Y()=aLR.Top();
        else if (aPnt.Y()>aLR.Bottom()) aPnt.Y()=aLR.Bottom();
        if (aRef.X()>aSR.Left()) {
            Fraction aMax(aRef.X()-aLR.Left(),aRef.X()-aSR.Left());
            if (aMax<aMaxFact) aMaxFact=aMax;
        }
        if (aRef.X()<aSR.Right()) {
            Fraction aMax(aLR.Right()-aRef.X(),aSR.Right()-aRef.X());
            if (aMax<aMaxFact) aMaxFact=aMax;
        }
        if (aRef.Y()>aSR.Top()) {
            Fraction aMax(aRef.Y()-aLR.Top(),aRef.Y()-aSR.Top());
            if (aMax<aMaxFact) aMaxFact=aMax;
        }
        if (aRef.Y()<aSR.Bottom()) {
            Fraction aMax(aLR.Bottom()-aRef.Y(),aSR.Bottom()-aRef.Y());
            if (aMax<aMaxFact) aMaxFact=aMax;
        }
    }
    long nXDiv=aStart.X()-aRef.X(); if (nXDiv==0) nXDiv=1;
    long nYDiv=aStart.Y()-aRef.Y(); if (nYDiv==0) nYDiv=1;
    long nXMul=aPnt.X()-aRef.X();
    long nYMul=aPnt.Y()-aRef.Y();
    if (nXDiv<0) { nXDiv=-nXDiv; nXMul=-nXMul; }
    if (nYDiv<0) { nYDiv=-nYDiv; nYMul=-nYMul; }
    FASTBOOL bXNeg=nXMul<0; if (bXNeg) nXMul=-nXMul;
    FASTBOOL bYNeg=nYMul<0; if (bYNeg) nYMul=-nYMul;
    FASTBOOL bOrtho=rView.IsOrtho() || !rView.IsResizeAllowed(FALSE);
    if (!DragStat().IsHorFixed() && !DragStat().IsVerFixed()) {
        if (Abs(nXDiv)<=1 || Abs(nYDiv)<=1) bOrtho=FALSE;
        if (bOrtho) {
            if ((Fraction(nXMul,nXDiv)>Fraction(nYMul,nYDiv)) !=rView.IsBigOrtho()) {
                nXMul=nYMul;
                nXDiv=nYDiv;
            } else {
                nYMul=nXMul;
                nYDiv=nXDiv;
            }
        }
    } else {
        if (bOrtho) {
            if (DragStat().IsHorFixed()) { bXNeg=FALSE; nXMul=nYMul; nXDiv=nYDiv; }
            if (DragStat().IsVerFixed()) { bYNeg=FALSE; nYMul=nXMul; nYDiv=nXDiv; }
        } else {
            if (DragStat().IsHorFixed()) { bXNeg=FALSE; nXMul=1; nXDiv=1; }
            if (DragStat().IsVerFixed()) { bYNeg=FALSE; nYMul=1; nYDiv=1; }
        }
    }
    Fraction aNeuXFact(nXMul,nXDiv);
    Fraction aNeuYFact(nYMul,nYDiv);
    if (bOrtho) {
        if (aNeuXFact>aMaxFact) {
            aNeuXFact=aMaxFact;
            aNeuYFact=aMaxFact;
        }
        if (aNeuYFact>aMaxFact) {
            aNeuXFact=aMaxFact;
            aNeuYFact=aMaxFact;
        }
    }
    if (bXNeg) aNeuXFact=Fraction(-aNeuXFact.GetNumerator(),aNeuXFact.GetDenominator());
    if (bYNeg) aNeuYFact=Fraction(-aNeuYFact.GetNumerator(),aNeuYFact.GetDenominator());
    if (DragStat().CheckMinMoved(aPnt)) {
        if ((!DragStat().IsHorFixed() && aPnt.X()!=DragStat().GetNow().X()) ||
            (!DragStat().IsVerFixed() && aPnt.Y()!=DragStat().GetNow().Y())) {
            Hide();
            DragStat().NextMove(aPnt);
            aXFact=aNeuXFact;
            aYFact=aNeuYFact;
            MovAllPoints();
            Show();
        }
    }
}

FASTBOOL SdrDragResize::End(FASTBOOL bCopy)
{
    Hide();
    if (IsDraggingPoints()) {
        rView.ResizeMarkedPoints(DragStat().Ref1(),aXFact,aYFact,bCopy);
    } else if (IsDraggingGluePoints()) {
        rView.ResizeMarkedGluePoints(DragStat().Ref1(),aXFact,aYFact,bCopy);
    } else {
        rView.ResizeMarkedObj(DragStat().Ref1(),aXFact,aYFact,bCopy);
    }
    return TRUE;
}

Pointer SdrDragResize::GetPointer() const
{
    const SdrHdl* pHdl=GetDragHdl();
    if (pHdl!=NULL) {
        return pHdl->GetPointer();
    }
    return Pointer(POINTER_MOVE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragRotate,SdrDragMethod);

void SdrDragRotate::TakeComment(XubString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethRotate, rStr);
    rStr.AppendAscii(" (");
    XubString aStr;
    INT32 nTmpWink(NormAngle360(nWink));

    if(bRight && nWink)
    {
        nTmpWink -= 36000;
    }

    rView.GetModel()->TakeWinkStr(nTmpWink, aStr);
    rStr += aStr;
    rStr += sal_Unicode(')');

    if(rView.IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

FASTBOOL SdrDragRotate::Beg()
{
    SdrHdl* pH=GetHdlList().GetHdl(HDL_REF1);
    if (pH!=NULL) {
        SetDragPolys();
        Show();
        DragStat().Ref1()=pH->GetPos();
        nWink0=GetAngle(DragStat().GetStart()-DragStat().GetRef1());
        return TRUE;
    } else {
        DBG_ERROR("SdrDragRotate::Beg(): Kein Referenzpunkt-Handle gefunden");
        return FALSE;
    }
}

void SdrDragRotate::MovPoint(Point& rPnt, const Point& rPvOfs)
{
    RotatePoint(rPnt,DragStat().GetRef1()-rPvOfs,nSin,nCos);
}

void SdrDragRotate::Mov(const Point& rPnt_)
{
    Point aPnt(rPnt_);
    const SdrHdl* pHdl=DragStat().GetHdl();
    if (pHdl!=NULL && pHdl->GetPageView()!=NULL) {
        aPnt-=pHdl->GetPageView()->GetOffset();
    }
    if (DragStat().CheckMinMoved(aPnt)) {
        long nNeuWink=NormAngle360(GetAngle(aPnt-DragStat().GetRef1())-nWink0);
        long nSA=0;
        if (rView.IsAngleSnapEnabled()) nSA=rView.GetSnapAngle();
        if (!rView.IsRotateAllowed(FALSE)) nSA=9000;
        if (nSA!=0) { // Winkelfang
            nNeuWink+=nSA/2;
            nNeuWink/=nSA;
            nNeuWink*=nSA;
        }
        nNeuWink=NormAngle180(nNeuWink);
        if (nWink!=nNeuWink) {
            USHORT nSekt0=GetAngleSector(nWink);
            USHORT nSekt1=GetAngleSector(nNeuWink);
            if (nSekt0==0 && nSekt1==3) bRight=TRUE;
            if (nSekt0==3 && nSekt1==0) bRight=FALSE;
            nWink=nNeuWink;
            double a=nWink*nPi180;
            double nSin1=sin(a); // schonmal berechnen, damit mgl. wenig Zeit
            double nCos1=cos(a); // zwischen Hide() und Show() vergeht
            Hide();
            nSin=nSin1;
            nCos=nCos1;
            DragStat().NextMove(aPnt);
            MovAllPoints();
            Show();
        }
    }
}

FASTBOOL SdrDragRotate::End(FASTBOOL bCopy)
{
    Hide();
    if (nWink!=0) {
        if (IsDraggingPoints()) {
            rView.RotateMarkedPoints(DragStat().GetRef1(),nWink,bCopy);
        } else if (IsDraggingGluePoints()) {
            rView.RotateMarkedGluePoints(DragStat().GetRef1(),nWink,bCopy);
        } else {
            rView.RotateMarkedObj(DragStat().GetRef1(),nWink,bCopy);
        }
    }
    return TRUE;
}

Pointer SdrDragRotate::GetPointer() const
{
    return Pointer(POINTER_ROTATE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragShear,SdrDragMethod);

void SdrDragShear::TakeComment(XubString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethShear, rStr);
    rStr.AppendAscii(" (");

    INT32 nTmpWink(nWink);

    if(bUpSideDown)
        nTmpWink += 18000;

    nTmpWink = NormAngle180(nTmpWink);

    XubString aStr;

    rView.GetModel()->TakeWinkStr(nTmpWink, aStr);
    rStr += aStr;
    rStr += sal_Unicode(')');

    if(rView.IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

FASTBOOL SdrDragShear::Beg()
{
    SdrHdlKind eRefHdl=HDL_MOVE;
    SdrHdl* pRefHdl=NULL;
    switch (GetDragHdlKind()) {
        case HDL_UPPER: eRefHdl=HDL_LOWER; break;
        case HDL_LOWER: eRefHdl=HDL_UPPER; break;
        case HDL_LEFT : eRefHdl=HDL_RIGHT; bVertical=TRUE; break;
        case HDL_RIGHT: eRefHdl=HDL_LEFT ; bVertical=TRUE; break;
    }
    if (eRefHdl!=HDL_MOVE) pRefHdl=GetHdlList().GetHdl(eRefHdl);
    if (pRefHdl!=NULL) {
        DragStat().Ref1()=pRefHdl->GetPos();
        nWink0=GetAngle(DragStat().GetStart()-DragStat().GetRef1());
    } else {
        DBG_ERROR("SdrDragShear::Beg(): Kein Referenzpunkt-Handle fuer Shear gefunden");
        return FALSE;
    }
    SetDragPolys();
    Show();
    return TRUE;
}

void SdrDragShear::MovPoint(Point& rPnt, const Point& rPvOfs)
{
    Point aRef(DragStat().GetRef1()-rPvOfs);
    if (bResize) {
        if (bVertical) {
            ResizePoint(rPnt,aRef,aFact,Fraction(1,1));
        } else {
            ResizePoint(rPnt,aRef,Fraction(1,1),aFact);
        }
    }
    ShearPoint(rPnt,aRef,nTan,bVertical);
}

void SdrDragShear::Mov(const Point& rPnt)
{
    if (DragStat().CheckMinMoved(rPnt)) {
        bResize=!rView.IsOrtho();
        long nSA=0;
        if (rView.IsAngleSnapEnabled()) nSA=rView.GetSnapAngle();
        Point aP0(DragStat().GetStart());
        Point aPnt(rPnt);
        Fraction aNeuFact(1,1);
        // Wenn kein Winkelfang, dann ggf. Rasterfang (ausser bei Slant)
        if (nSA==0 && !bSlant) aPnt=GetSnapPos(aPnt);
        if (!bSlant && !bResize) { // Shear ohne Resize
            if (bVertical) aPnt.X()=aP0.X(); else aPnt.Y()=aP0.Y();
        }
        Point aRef(DragStat().GetRef1());
        Point aDif(aPnt-aRef);

        long nNeuWink=0;
        if (bSlant) {
            nNeuWink=NormAngle180(-(GetAngle(aDif)-nWink0));
            if (bVertical) nNeuWink=NormAngle180(-nNeuWink);
        } else {
            if (bVertical) nNeuWink=NormAngle180(GetAngle(aDif));
            else nNeuWink=NormAngle180(-(GetAngle(aDif)-9000));
            if (nNeuWink<-9000 || nNeuWink>9000) nNeuWink=NormAngle180(nNeuWink+18000);
            if (bResize) {
                Point aPt2(aPnt);
                if (nSA!=0) aPt2=GetSnapPos(aPnt); // den also in jedem Falle fangen
                if (bVertical) {
                    aNeuFact=Fraction(aPt2.X()-aRef.X(),aP0.X()-aRef.X());
                } else {
                    aNeuFact=Fraction(aPt2.Y()-aRef.Y(),aP0.Y()-aRef.Y());
                }
            }
        }
        FASTBOOL bNeg=nNeuWink<0;
        if (bNeg) nNeuWink=-nNeuWink;
        if (nSA!=0) { // Winkelfang
            nNeuWink+=nSA/2;
            nNeuWink/=nSA;
            nNeuWink*=nSA;
        }
        nNeuWink=NormAngle360(nNeuWink);
        bUpSideDown=nNeuWink>9000 && nNeuWink<27000;
        if (bSlant) { // Resize fuer Slant berechnen
            // Mit Winkelfang jedoch ohne 89deg Begrenzung
            long nTmpWink=nNeuWink;
            if (bUpSideDown) nNeuWink-=18000;
            if (bNeg) nTmpWink=-nTmpWink;
            bResize=TRUE;
            double nCos=cos(nTmpWink*nPi180);
            aNeuFact=nCos;
            Kuerzen(aFact,10); // 3 Dezimalstellen sollten reichen
        }
        if (nNeuWink>8900) nNeuWink=8900;
        if (bNeg) nNeuWink=-nNeuWink;
        if (nWink!=nNeuWink || aFact!=aNeuFact) {
            nWink=nNeuWink;
            aFact=aNeuFact;
            double a=nWink*nPi180;
            double nTan1=0.0;
            nTan1=tan(a); // schonmal berechnen, damit mgl. wenig Zeit zwischen Hide() und Show() vergeht
            Hide();
            nTan=nTan1;
            DragStat().NextMove(rPnt);
            MovAllPoints();
            Show();
        }
    }
}

FASTBOOL SdrDragShear::End(FASTBOOL bCopy)
{
    Hide();
    if (bResize && aFact==Fraction(1,1)) bResize=FALSE;
    if (nWink!=0 || bResize) {
        if (nWink!=0 && bResize) {
            XubString aStr;
            ImpTakeDescriptionStr(STR_EditShear,aStr);
            if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
            rView.BegUndo(aStr);
        }
        if (bResize) {
            if (bVertical) {
                rView.ResizeMarkedObj(DragStat().GetRef1(),aFact,Fraction(1,1),bCopy);
            } else {
                rView.ResizeMarkedObj(DragStat().GetRef1(),Fraction(1,1),aFact,bCopy);
            }
            bCopy=FALSE;
        }
        if (nWink!=0) {
            rView.ShearMarkedObj(DragStat().GetRef1(),nWink,bVertical,bCopy);
        }
        if (nWink!=0 && bResize) rView.EndUndo();
        return TRUE;
    }
    return FALSE;
}

Pointer SdrDragShear::GetPointer() const
{
    if (bVertical) return Pointer(POINTER_VSHEAR);
    else return Pointer(POINTER_HSHEAR);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragMirror,SdrDragMethod);

FASTBOOL SdrDragMirror::ImpCheckSide(const Point& rPnt) const
{
    long nWink1=GetAngle(rPnt-DragStat().GetRef1());
    nWink1-=nWink;
    nWink1=NormAngle360(nWink1);
    return nWink1<18000;
}

void SdrDragMirror::TakeComment(XubString& rStr) const
{
    if (aDif.X()==0) ImpTakeDescriptionStr(STR_DragMethMirrorHori,rStr);
    else if (aDif.Y()==0) ImpTakeDescriptionStr(STR_DragMethMirrorVert,rStr);
    else if (Abs(aDif.X())==Abs(aDif.Y())) ImpTakeDescriptionStr(STR_DragMethMirrorDiag,rStr);
    else ImpTakeDescriptionStr(STR_DragMethMirrorFree,rStr);
    if (rView.IsDragWithCopy()) rStr+=ImpGetResStr(STR_EditWithCopy);
}

FASTBOOL SdrDragMirror::Beg()
{
    SdrHdl* pH1=GetHdlList().GetHdl(HDL_REF1);
    SdrHdl* pH2=GetHdlList().GetHdl(HDL_REF2);
    if (pH1!=NULL && pH2!=NULL) {
        DragStat().Ref1()=pH1->GetPos();
        DragStat().Ref2()=pH2->GetPos();
        Ref1()=pH1->GetPos();
        Ref2()=pH2->GetPos();
        aDif=pH2->GetPos()-pH1->GetPos();
        FASTBOOL b90=(aDif.X()==0) || aDif.Y()==0;
        FASTBOOL b45=b90 || (Abs(aDif.X())==Abs(aDif.Y()));
        nWink=NormAngle360(GetAngle(aDif));
        if (!rView.IsMirrorAllowed(FALSE,FALSE) && !b45) return FALSE; // freier Achsenwinkel nicht erlaubt
        if (!rView.IsMirrorAllowed(TRUE,FALSE) && !b90) return FALSE;  // 45deg auch nicht erlaubt
        bSide0=ImpCheckSide(DragStat().GetStart());
        SetDragPolys();
        Show();
        return TRUE;
    } else {
        DBG_ERROR("SdrDragMirror::Beg(): Spiegelachse nicht gefunden");
        return FALSE;
    }
}

void SdrDragMirror::MovPoint(Point& rPnt, const Point& rPvOfs)
{
    if (bMirrored) {
        Point aRef1(DragStat().GetRef1()-rPvOfs);
        Point aRef2(DragStat().GetRef2()-rPvOfs);
        MirrorPoint(rPnt,aRef1,aRef2);
    }
}

void SdrDragMirror::Mov(const Point& rPnt)
{
    if (DragStat().CheckMinMoved(rPnt)) {
        FASTBOOL bNeuSide=ImpCheckSide(rPnt);
        FASTBOOL bNeuMirr=bSide0!=bNeuSide;
        if (bMirrored!=bNeuMirr) {
            Hide();
            bMirrored=bNeuMirr;
            DragStat().NextMove(rPnt);
            MovAllPoints();
            Show();
        }
    }
}

FASTBOOL SdrDragMirror::End(FASTBOOL bCopy)
{
    Hide();
    if (bMirrored) {
        rView.MirrorMarkedObj(DragStat().GetRef1(),DragStat().GetRef2(),bCopy);
    }
    return TRUE;
}

Pointer SdrDragMirror::GetPointer() const
{
    return Pointer(POINTER_MIRROR);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragGradient, SdrDragMethod);

SdrDragGradient::SdrDragGradient(SdrDragView& rNewView, BOOL bGrad)
:   SdrDragMethod(rNewView),
    pIAOHandle(NULL),
    bIsGradient(bGrad)
{
}

void SdrDragGradient::TakeComment(XubString& rStr) const
{
    if(IsGradient())
        ImpTakeDescriptionStr(STR_DragMethGradient, rStr);
    else
        ImpTakeDescriptionStr(STR_DragMethTransparence, rStr);
}

FASTBOOL SdrDragGradient::Beg()
{
    FASTBOOL bRetval(FALSE);

    pIAOHandle = (SdrHdlGradient*)GetHdlList().GetHdl(IsGradient() ? HDL_GRAD : HDL_TRNS);
    if(pIAOHandle)
    {
        // save old values
        DragStat().Ref1() = pIAOHandle->GetPos();
        DragStat().Ref2() = pIAOHandle->Get2ndPos();

        // what was hit?
        BOOL bHit(FALSE);
        SdrHdlColor* pColHdl = pIAOHandle->GetColorHdl1();

        // init handling flags
        pIAOHandle->SetMoveSingleHandle(FALSE);
        pIAOHandle->SetMoveFirstHandle(FALSE);

        // test first color handle
        if(pColHdl)
        {
            const B2dIAOGroup& rIAOGroup = pColHdl->GetIAOGroup();
            if(rIAOGroup.GetIAOCount())
            {
                Point aPixelPos(rIAOGroup.GetIAObject(0)->GetManager()->GetWindow()->LogicToPixel(DragStat().GetStart()));
                if(rIAOGroup.IsHit(aPixelPos))
                {
                    bHit = TRUE;
                    pIAOHandle->SetMoveSingleHandle(TRUE);
                    pIAOHandle->SetMoveFirstHandle(TRUE);
                }
            }
        }

        // test second color handle
        pColHdl = pIAOHandle->GetColorHdl2();
        if(!bHit && pColHdl)
        {
            const B2dIAOGroup& rIAOGroup = pColHdl->GetIAOGroup();
            if(rIAOGroup.GetIAOCount())
            {
                Point aPixelPos(rIAOGroup.GetIAObject(0)->GetManager()->GetWindow()->LogicToPixel(DragStat().GetStart()));
                if(rIAOGroup.IsHit(aPixelPos))
                {
                    bHit = TRUE;
                    pIAOHandle->SetMoveSingleHandle(TRUE);
                }
            }
        }

        // test gradient handle itself
        if(!bHit)
        {
            const B2dIAOGroup& rIAOGroup = pIAOHandle->GetIAOGroup();
            if(rIAOGroup.GetIAOCount())
            {
                Point aPixelPos(rIAOGroup.GetIAObject(0)->GetManager()->GetWindow()->LogicToPixel(DragStat().GetStart()));
                if(rIAOGroup.IsHit(aPixelPos))
                {
                    bHit = TRUE;
                }
            }
        }

        // everything up and running :o}
        bRetval = bHit;
    }
    else
        DBG_ERROR("SdrDragGradient::Beg(): IAOGradient nicht gefunden");

    return bRetval;
}

void SdrDragGradient::Mov(const Point& rPnt)
{
    if(pIAOHandle && DragStat().CheckMinMoved(rPnt))
    {
        DragStat().NextMove(rPnt);

        // Do the Move here!!! DragStat().GetStart()
        Point aMoveDiff = rPnt - DragStat().GetStart();

        if(pIAOHandle->IsMoveSingleHandle())
        {
            if(pIAOHandle->IsMoveFirstHandle())
            {
                pIAOHandle->SetPos(DragStat().Ref1() + aMoveDiff);
                if(pIAOHandle->GetColorHdl1())
                    pIAOHandle->GetColorHdl1()->SetPos(DragStat().Ref1() + aMoveDiff);
            }
            else
            {
                pIAOHandle->Set2ndPos(DragStat().Ref2() + aMoveDiff);
                if(pIAOHandle->GetColorHdl2())
                    pIAOHandle->GetColorHdl2()->SetPos(DragStat().Ref2() + aMoveDiff);
            }
        }
        else
        {
            pIAOHandle->SetPos(DragStat().Ref1() + aMoveDiff);
            pIAOHandle->Set2ndPos(DragStat().Ref2() + aMoveDiff);
            if(pIAOHandle->GetColorHdl1())
                pIAOHandle->GetColorHdl1()->SetPos(DragStat().Ref1() + aMoveDiff);
            if(pIAOHandle->GetColorHdl2())
                pIAOHandle->GetColorHdl2()->SetPos(DragStat().Ref2() + aMoveDiff);
        }

        // new state
        pIAOHandle->FromIAOToItem(rView.GetMarkList().GetMark(0)->GetObj(), FALSE, FALSE);
    }
}

FASTBOOL SdrDragGradient::End(FASTBOOL bCopy)
{
    // here the result is clear, do something with the values
    Ref1() = pIAOHandle->GetPos();
    Ref2() = pIAOHandle->Get2ndPos();

    // new state
    pIAOHandle->FromIAOToItem(rView.GetMarkList().GetMark(0)->GetObj(), TRUE, TRUE);

    return TRUE;
}

void SdrDragGradient::Brk()
{
    // restore old values
    pIAOHandle->SetPos(DragStat().Ref1());
    pIAOHandle->Set2ndPos(DragStat().Ref2());
    if(pIAOHandle->GetColorHdl1())
        pIAOHandle->GetColorHdl1()->SetPos(DragStat().Ref1());
    if(pIAOHandle->GetColorHdl2())
        pIAOHandle->GetColorHdl2()->SetPos(DragStat().Ref2());

    // new state
    pIAOHandle->FromIAOToItem(rView.GetMarkList().GetMark(0)->GetObj(), TRUE, FALSE);
}

Pointer SdrDragGradient::GetPointer() const
{
    return Pointer(POINTER_REFHAND);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragCrook,SdrDragMethod);

void SdrDragCrook::TakeComment(XubString& rStr) const
{
    ImpTakeDescriptionStr(!bContortion ? STR_DragMethCrook : STR_DragMethCrookContortion, rStr);

    if(bValid)
    {
        rStr.AppendAscii(" (");

        XubString aStr;
        INT32 nVal(nWink);

        if(bAtCenter)
            nVal *= 2;

        nVal = Abs(nVal);
        rView.GetModel()->TakeWinkStr(nVal, aStr);
        rStr += aStr;
        rStr += sal_Unicode(')');
    }

    if(rView.IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

FASTBOOL SdrDragCrook::Beg()
{
    bContortionAllowed=rView.IsCrookAllowed(FALSE);
    bNoContortionAllowed=rView.IsCrookAllowed(TRUE);
    bResizeAllowed=rView.IsResizeAllowed(FALSE);
    bRotateAllowed=rView.IsRotateAllowed(FALSE);
    if (bContortionAllowed || bNoContortionAllowed) {
        bVertical=(GetDragHdlKind()==HDL_LOWER || GetDragHdlKind()==HDL_UPPER);
        aMarkRect=GetMarkedRect();
        aMarkCenter=aMarkRect.Center();
        nMarkSize=bVertical ? (aMarkRect.GetHeight()-1) : (aMarkRect.GetWidth()-1);
        aCenter=aMarkCenter;
        aStart=DragStat().GetStart();
        SetDragPolysSeparated();
        Show();
        return TRUE;
    } else {
        return FALSE;
    }
}

void SdrDragCrook::MovAllPoints()
{
    USHORT nPvAnz=rView.GetPageViewCount();
    for (USHORT nv=0; nv<nPvAnz; nv++) {
        SdrPageView* pPV=rView.GetPageViewPvNum(nv);
        pPV->DragPoly()=pPV->DragPoly0();
        if (pPV->HasMarkedObj()) {
            Point aPvOfs(pPV->GetOffset());
            USHORT nPolyAnz=pPV->DragPoly().Count();
            if (!bContortion && !rView.IsNoDragXorPolys()) {
                USHORT n1st=0,nLast=0;
                Point aC(aCenter);
                aC-=aPvOfs;
                while (n1st<nPolyAnz) {
                    nLast=n1st;
                    while (nLast<nPolyAnz && pPV->DragPoly()[nLast].GetPointCount()!=0) nLast++;
                    Rectangle aBound(pPV->DragPoly()[n1st].GetBoundRect());
                    USHORT i;
                    for (i=n1st+1; i<nLast; i++) {
                        aBound.Union(pPV->DragPoly()[n1st].GetBoundRect());
                    }
                    Point aCtr0(aBound.Center());
                    Point aCtr1(aCtr0);
                    if (bResize) {
                        Fraction aFact1(1,1);
                        if (bVertical) {
                            ResizePoint(aCtr1,aC,aFact1,aFact);
                        } else {
                            ResizePoint(aCtr1,aC,aFact,aFact1);
                        }
                    }
                    FASTBOOL bRotOk=FALSE;
                    double nSin,nCos;
                    if (aRad.X()!=0 && aRad.Y()!=0) {
                        bRotOk=bRotate;
                        switch (eMode) {
                            case SDRCROOK_ROTATE : CrookRotateXPoint (aCtr1,NULL,NULL,aC,aRad,nSin,nCos,bVertical);           break;
                            case SDRCROOK_SLANT  : CrookSlantXPoint  (aCtr1,NULL,NULL,aC,aRad,nSin,nCos,bVertical);           break;
                            case SDRCROOK_STRETCH: CrookStretchXPoint(aCtr1,NULL,NULL,aC,aRad,nSin,nCos,bVertical,aMarkRect); break;
                        } // switch
                    }
                    aCtr1-=aCtr0;
                    for (i=n1st; i<nLast; i++) {
                        if (bRotOk) {
                            RotateXPoly(pPV->DragPoly()[i],aCtr0,nSin,nCos);
                        }
                        pPV->DragPoly()[i].Move(aCtr1.X(),aCtr1.Y());
                    }
                    n1st=nLast+1;
                }
            } else {
                USHORT i,j;
                for (j=0; j<nPolyAnz; j++) {
                    XPolygon& aPol=pPV->DragPoly()[j];
                    USHORT nPtAnz=aPol.GetPointCount();
                    i=0;
                    while (i<nPtAnz) {
                        Point* pPnt=&aPol[i];
                        Point* pC1=NULL;
                        Point* pC2=NULL;
                        if (i+1<nPtAnz && aPol.IsControl(i)) { // Kontrollpunkt links
                            pC1=pPnt;
                            i++;
                            pPnt=&aPol[i];
                        }
                        i++;
                        if (i<nPtAnz && aPol.IsControl(i)) { // Kontrollpunkt rechts
                            pC2=&aPol[i];
                            i++;
                        }
                        MovPoint(*pPnt,aPvOfs,pC1,pC2);
                    }
                }
            }
        }
    }
}

void SdrDragCrook::MovPoint(Point& rPnt, const Point& rPvOfs, Point* pC1, Point* pC2)
{
    FASTBOOL bSlant=eMode==SDRCROOK_SLANT;
    FASTBOOL bStretch=eMode==SDRCROOK_STRETCH;
    FASTBOOL bVert=bVertical;
    FASTBOOL bC1=pC1!=NULL;
    FASTBOOL bC2=pC2!=NULL;
    Point aC(aCenter);
    aC-=rPvOfs;
    if (bResize) {
        Fraction aFact1(1,1);
        if (bVert) {
            ResizePoint(rPnt,aC,aFact1,aFact);
            if (bC1) ResizePoint(*pC1,aC,aFact1,aFact);
            if (bC2) ResizePoint(*pC2,aC,aFact1,aFact);
        } else {
            ResizePoint(rPnt,aC,aFact,aFact1);
            if (bC1) ResizePoint(*pC1,aC,aFact,aFact1);
            if (bC2) ResizePoint(*pC2,aC,aFact,aFact1);
        }
    }
    if (aRad.X()!=0 && aRad.Y()!=0) {
        double nSin,nCos;
        switch (eMode) {
            case SDRCROOK_ROTATE : CrookRotateXPoint (rPnt,pC1,pC2,aC,aRad,nSin,nCos,bVert);           break;
            case SDRCROOK_SLANT  : CrookSlantXPoint  (rPnt,pC1,pC2,aC,aRad,nSin,nCos,bVert);           break;
            case SDRCROOK_STRETCH: CrookStretchXPoint(rPnt,pC1,pC2,aC,aRad,nSin,nCos,bVert,aMarkRect); break;
        } // switch
    }
}

void SdrDragCrook::Mov(const Point& rPnt)
{
    if (DragStat().CheckMinMoved(rPnt)) {
        Point aPnt(rPnt);
        FASTBOOL bNeuMoveOnly=rView.IsMoveOnlyDragging();
        bAtCenter=FALSE;
        SdrCrookMode eNeuMode=rView.GetCrookMode();
        FASTBOOL bNeuContortion=!bNeuMoveOnly && ((bContortionAllowed && !rView.IsCrookNoContortion()) || !bNoContortionAllowed);
        bResize=!rView.IsOrtho() && bResizeAllowed && !bNeuMoveOnly;
        FASTBOOL bNeuRotate=bRotateAllowed && !bNeuContortion && !bNeuMoveOnly && eNeuMode==SDRCROOK_ROTATE;
        long nSA=0;
        if (nSA==0) aPnt=GetSnapPos(aPnt);
        Point aNeuCenter(aMarkCenter.X(),aStart.Y());
        if (bVertical) { aNeuCenter.X()=aStart.X(); aNeuCenter.Y()=aMarkCenter.Y(); }
        if (!rView.IsCrookAtCenter()) {
            switch (GetDragHdlKind()) {
                case HDL_UPLFT: aNeuCenter.X()=aMarkRect.Right();  bLft=TRUE; break;
                case HDL_UPPER: aNeuCenter.Y()=aMarkRect.Bottom(); bUpr=TRUE; break;
                case HDL_UPRGT: aNeuCenter.X()=aMarkRect.Left();   bRgt=TRUE; break;
                case HDL_LEFT : aNeuCenter.X()=aMarkRect.Right();  bLft=TRUE; break;
                case HDL_RIGHT: aNeuCenter.X()=aMarkRect.Left();   bRgt=TRUE; break;
                case HDL_LWLFT: aNeuCenter.X()=aMarkRect.Right();  bLft=TRUE; break;
                case HDL_LOWER: aNeuCenter.Y()=aMarkRect.Top();    bLwr=TRUE; break;
                case HDL_LWRGT: aNeuCenter.X()=aMarkRect.Left();   bRgt=TRUE; break;
                default: bAtCenter=TRUE;
            }
        } else bAtCenter=TRUE;
        Fraction aNeuFact(1,1);
        long dx1=aPnt.X()-aNeuCenter.X();
        long dy1=aPnt.Y()-aNeuCenter.Y();
        bValid=bVertical ? dx1!=0 : dy1!=0;
        if (bValid) {
            if (bVertical) bValid=Abs(dx1)*100>Abs(dy1);
            else bValid=Abs(dy1)*100>Abs(dx1);
        }
        long nNeuRad=0;
        nWink=0;
        if (bValid) {
            double a=0; // Steigung des Radius
            long nPntWink=0;
            if (bVertical) {
                a=((double)dy1)/((double)dx1); // Steigung des Radius
                nNeuRad=((long)(dy1*a)+dx1) /2;
                aNeuCenter.X()+=nNeuRad;
                nPntWink=GetAngle(aPnt-aNeuCenter);
            } else {
                a=((double)dx1)/((double)dy1); // Steigung des Radius
                nNeuRad=((long)(dx1*a)+dy1) /2;
                aNeuCenter.Y()+=nNeuRad;
                nPntWink=GetAngle(aPnt-aNeuCenter)-9000;
            }
            if (!bAtCenter) {
                if (nNeuRad<0) {
                    if (bRgt) nPntWink+=18000;
                    if (bLft) nPntWink=18000-nPntWink;
                    if (bLwr) nPntWink=-nPntWink;
                } else {
                    if (bRgt) nPntWink=-nPntWink;
                    if (bUpr) nPntWink=18000-nPntWink;
                    if (bLwr) nPntWink+=18000;
                }
                nPntWink=NormAngle360(nPntWink);
            } else {
                if (nNeuRad<0) nPntWink+=18000;
                if (bVertical) nPntWink=18000-nPntWink;
                nPntWink=NormAngle180(nPntWink);
                nPntWink=Abs(nPntWink);
            }
            double nUmfang=2*Abs(nNeuRad)*nPi;
            if (bResize) {
                if (nSA!=0) { // Winkelfang
                    long nWink0=nPntWink;
                    nPntWink+=nSA/2;
                    nPntWink/=nSA;
                    nPntWink*=nSA;
                    BigInt a(nNeuRad);
                    a*=BigInt(nWink);
                    a/=BigInt(nWink0);
                    nNeuRad=long(a);
                    if (bVertical) aNeuCenter.X()=aStart.X()+nNeuRad;
                    else aNeuCenter.Y()=aStart.Y()+nNeuRad;
                }
                long nMul=(long)(nUmfang*NormAngle360(nPntWink)/36000);
                if (bAtCenter) nMul*=2;
                aNeuFact=Fraction(nMul,nMarkSize);
                nWink=nPntWink;
            } else {
                nWink=(long)((nMarkSize*360/nUmfang)*100)/2;
                if (nWink==0) bValid=FALSE;
                if (bValid && nSA!=0) { // Winkelfang
                    long nWink0=nWink;
                    nWink+=nSA/2;
                    nWink/=nSA;
                    nWink*=nSA;
                    BigInt a(nNeuRad);
                    a*=BigInt(nWink);
                    a/=BigInt(nWink0);
                    nNeuRad=long(a);
                    if (bVertical) aNeuCenter.X()=aStart.X()+nNeuRad;
                    else aNeuCenter.Y()=aStart.Y()+nNeuRad;
                }
            }
        }
        if (nWink==0 || nNeuRad==0) bValid=FALSE;
        if (!bValid) nNeuRad=0;
        if (!bValid && bResize) {
            long nMul=bVertical ? dy1 : dx1;
            if (bLft || bUpr) nMul=-nMul;
            long nDiv=nMarkSize;
            if (bAtCenter) { nMul*=2; nMul=Abs(nMul); }
            aNeuFact=Fraction(nMul,nDiv);
        }
        if (aNeuCenter!=aCenter || bNeuContortion!=bContortion || aNeuFact!=aFact ||
            bNeuMoveOnly!=bMoveOnly || bNeuRotate!=bRotate || eNeuMode!=eMode)
        {
            Hide();
            bMoveOnly=bNeuMoveOnly;
            bRotate=bNeuRotate;
            eMode=eNeuMode;
            bContortion=bNeuContortion;
            aCenter=aNeuCenter;
            aFact=aNeuFact;
            aRad=Point(nNeuRad,nNeuRad);
            bResize=aFact!=Fraction(1,1) && aFact.GetDenominator()!=0 && aFact.IsValid();
            DragStat().NextMove(aPnt);
            MovAllPoints();
            Show();
        }
    }
}

FASTBOOL SdrDragCrook::End(FASTBOOL bCopy)
{
    Hide();
    if (bResize && aFact==Fraction(1,1)) bResize=FALSE;
    FASTBOOL bDoCrook=aCenter!=aMarkCenter && aRad.X()!=0 && aRad.Y()!=0;
    if (bDoCrook || bResize) {
        if (bResize) {
            XubString aStr;
            ImpTakeDescriptionStr(!bContortion?STR_EditCrook:STR_EditCrookContortion,aStr);
            if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
            rView.BegUndo(aStr);
        }
        if (bResize) {
            Fraction aFact1(1,1);
            if (bContortion) {
                if (bVertical) rView.ResizeMarkedObj(aCenter,aFact1,aFact,bCopy);
                else rView.ResizeMarkedObj(aCenter,aFact,aFact1,bCopy);
            } else {
                if (bCopy) rView.CopyMarkedObj();
                ULONG nMarkAnz=rView.GetMarkList().GetMarkCount();
                for (ULONG nm=0; nm<nMarkAnz; nm++) {
                    SdrMark* pM=rView.GetMarkList().GetMark(nm);
                    SdrObject* pO=pM->GetObj();
                    Point aCtr0(pO->GetSnapRect().Center());
                    Point aCtr1(aCtr0);
                    if (bVertical) ResizePoint(aCtr1,aCenter-pM->GetPageView()->GetOffset(),aFact1,aFact);
                    else ResizePoint(aCtr1,aCenter-pM->GetPageView()->GetOffset(),aFact,aFact1);
                    Size aSiz(aCtr1.X()-aCtr0.X(),aCtr1.Y()-aCtr0.Y());
                    AddUndo(new SdrUndoMoveObj(*pO,aSiz));
                    pO->Move(aSiz);
                }
            }
            bCopy=FALSE;
        }
        if (bDoCrook) {
            rView.CrookMarkedObj(aCenter,aRad,eMode,bVertical,!bContortion,bCopy);
            rView.SetLastCrookCenter(aCenter);
        }
        if (bResize) rView.EndUndo();
        return TRUE;
    }
    return FALSE;
}

void SdrDragCrook::DrawXor(ExtOutputDevice& rXOut, FASTBOOL bFull) const
{
    SdrDragMethod::DrawXor(rXOut,bFull);
    if (FALSE) {
        Rectangle aRect(aCenter.X()-aRad.X(),aCenter.Y()-aRad.Y(),aCenter.X()+aRad.X(),aCenter.Y()+aRad.Y());

        Color aLineColorMerk( rXOut.GetOutDev()->GetLineColor() );
        Color aBlackColor( COL_BLACK );
        rXOut.GetOutDev()->SetLineColor( aBlackColor );
        rXOut.GetOutDev()->DrawEllipse(aRect);
        rXOut.GetOutDev()->SetLineColor( aLineColorMerk );
    }
}

Pointer SdrDragCrook::GetPointer() const
{
    return Pointer(POINTER_CROOK);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragDistort,SdrDragMethod);

void SdrDragDistort::TakeComment(XubString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethDistort, rStr);

    XubString aStr;

    rStr.AppendAscii(" (x=");
    rView.GetModel()->TakeMetricStr(DragStat().GetDX(), aStr);
    rStr += aStr;
    rStr.AppendAscii(" y=");
    rView.GetModel()->TakeMetricStr(DragStat().GetDY(), aStr);
    rStr += aStr;
    rStr += sal_Unicode(')');

    if(rView.IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

FASTBOOL SdrDragDistort::Beg()
{
    bContortionAllowed=rView.IsDistortAllowed(FALSE);
    bNoContortionAllowed=rView.IsDistortAllowed(TRUE);
    if (bContortionAllowed || bNoContortionAllowed) {
        SdrHdlKind eKind=GetDragHdlKind();
        nPolyPt=0xFFFF;
        if (eKind==HDL_UPLFT) nPolyPt=0;
        if (eKind==HDL_UPRGT) nPolyPt=1;
        if (eKind==HDL_LWRGT) nPolyPt=2;
        if (eKind==HDL_LWLFT) nPolyPt=3;
        if (nPolyPt>3) return FALSE;
        aMarkRect=GetMarkedRect();
        aDistortedRect=XPolygon(aMarkRect);
        SetDragPolys();
        Show();
        return TRUE;
    } else {
        return FALSE;
    }
}

void SdrDragDistort::MovAllPoints()
{
    if (bContortion) {
        USHORT nPvAnz=rView.GetPageViewCount();
        for (USHORT nv=0; nv<nPvAnz; nv++) {
            SdrPageView* pPV=rView.GetPageViewPvNum(nv);
            if (pPV->HasMarkedObj()) {
                Point aOfs(pPV->GetOffset());
                pPV->DragPoly()=pPV->DragPoly0();
                if (aOfs.X()==0 && aOfs.Y()==0) {
                    pPV->DragPoly().Distort(aMarkRect,aDistortedRect);
                } else {
                    Rectangle aR(aMarkRect);
                    XPolygon aXP(aDistortedRect);
                    aR.Move(-aOfs.X(),-aOfs.Y());
                    aXP.Move(-aOfs.X(),-aOfs.Y());
                    pPV->DragPoly().Distort(aR,aXP);
                }
            }
        }
    }
}

void SdrDragDistort::Mov(const Point& rPnt)
{
    if (DragStat().CheckMinMoved(rPnt)) {
        Point aPnt(GetSnapPos(rPnt));
        if (rView.IsOrtho()) OrthoDistance8(DragStat().GetStart(),aPnt,rView.IsBigOrtho());
        FASTBOOL bNeuContortion=(bContortionAllowed && !rView.IsCrookNoContortion()) || !bNoContortionAllowed;
        if (bNeuContortion!=bContortion || aDistortedRect[nPolyPt]!=aPnt) {
            Hide();
            aDistortedRect[nPolyPt]=aPnt;
            bContortion=bNeuContortion;
            DragStat().NextMove(aPnt);
            MovAllPoints();
            Show();
        }
    }
}

FASTBOOL SdrDragDistort::End(FASTBOOL bCopy)
{
    Hide();
    FASTBOOL bDoDistort=DragStat().GetDX()!=0 || DragStat().GetDY()!=0;
    if (bDoDistort) {
        rView.DistortMarkedObj(aMarkRect,aDistortedRect,!bContortion,bCopy);
        return TRUE;
    }
    return FALSE;
}

void SdrDragDistort::DrawXor(ExtOutputDevice& rXOut, FASTBOOL bFull) const
{
    SdrDragMethod::DrawXor(rXOut,bFull);
}

Pointer SdrDragDistort::GetPointer() const
{
    return Pointer(POINTER_REFHAND);
}

