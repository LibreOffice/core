/*************************************************************************
 *
 *  $RCSfile: svdedtv1.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 11:11:36 $
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

#include "svdedtv.hxx"
#include <math.h>

#ifndef _MATH_H
#define _MATH_H
#endif

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SVX_RECTENUM_HXX //autogen
#include <rectenum.hxx>
#endif

#include "svxids.hrc"   // fuer SID_ATTR_TRANSFORM_...
#include "svdattr.hxx"  // fuer Get/SetGeoAttr
#include "svditext.hxx"
#include "svditer.hxx"
#include "svdtrans.hxx"
#include "svdundo.hxx"
#include "svdpage.hxx"
#include "svdpagv.hxx"
#include "svdlayer.hxx" // fuer MergeNotPersistAttr
#include "svdattrx.hxx" // fuer MergeNotPersistAttr
#include "svdetc.hxx"   // fuer SearchOutlinerItems
#include "svdopath.hxx"  // fuer Crook
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache

#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@ @@@@@  @@ @@@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@   @@
//  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@ @ @@
//  @@@@  @@  @@ @@   @@    @@@@@ @@ @@@@  @@@@@@@
//  @@    @@  @@ @@   @@     @@@  @@ @@    @@@@@@@
//  @@    @@  @@ @@   @@     @@@  @@ @@    @@@ @@@
//  @@@@@ @@@@@  @@   @@      @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::SetMarkedObjRect(const Rectangle& rRect, BOOL bCopy)
{
    DBG_ASSERT(!rRect.IsEmpty(),"SetMarkedObjRect() mit leerem Rect mach keinen Sinn");
    if (rRect.IsEmpty()) return;
    ULONG nAnz=aMark.GetMarkCount();
    if (nAnz==0) return;
    Rectangle aR0(GetMarkedObjRect());
    DBG_ASSERT(!aR0.IsEmpty(),"SetMarkedObjRect(): GetMarkedObjRect() ist leer");
    if (aR0.IsEmpty()) return;
    long x0=aR0.Left();
    long y0=aR0.Top();
    long w0=aR0.Right()-x0;
    long h0=aR0.Bottom()-y0;
    long x1=rRect.Left();
    long y1=rRect.Top();
    long w1=rRect.Right()-x1;
    long h1=rRect.Bottom()-y1;
    XubString aStr;
    ImpTakeDescriptionStr(STR_EditPosSize,aStr);
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr);
    if (bCopy) CopyMarkedObj();
    for (ULONG nm=0; nm<nAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        AddUndo(new SdrUndoGeoObj(*pO));
        Rectangle aR1(pO->GetSnapRect());
        if (!aR1.IsEmpty()) {
            if (aR1==aR0) aR1=rRect;
            else { // aR1 von aR0 nach rRect transformieren
                aR1.Move(-x0,-y0);
                BigInt l(aR1.Left());
                BigInt r(aR1.Right());
                BigInt t(aR1.Top());
                BigInt b(aR1.Bottom());
                if (w0!=0) {
                    l*=w1; l/=w0;
                    r*=w1; r/=w0;
                } else {
                    l=0; r=w1;
                }
                if (h0!=0) {
                    t*=h1; t/=h0;
                    b*=h1; b/=h0;
                } else {
                    t=0; b=h1;
                }
                aR1.Left  ()=long(l);
                aR1.Right ()=long(r);
                aR1.Top   ()=long(t);
                aR1.Bottom()=long(b);
                aR1.Move(x1,y1);
            }
            pO->SetSnapRect(aR1);
        } else {
            DBG_ERROR("SetMarkedObjRect(): pObj->GetSnapRect() liefert leeres Rect");
        }
    }
    EndUndo();
}

void SdrEditView::MoveMarkedObj(const Size& rSiz, BOOL bCopy)
{
    XubString aStr(ImpGetResStr(STR_EditMove));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    // benoetigt eigene UndoGroup wegen Parameter
    BegUndo(aStr,aMark.GetMarkDescription(),SDRREPFUNC_OBJ_MOVE);
    if (bCopy) CopyMarkedObj();
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        AddUndo(new SdrUndoMoveObj(*pO,rSiz));
        pO->Move(rSiz);
    }
    EndUndo();
}

void SdrEditView::ResizeMarkedObj(const Point& rRef, const Fraction& xFact, const Fraction& yFact, BOOL bCopy)
{
    XubString aStr;
    ImpTakeDescriptionStr(STR_EditResize,aStr);
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr);
    if (bCopy) CopyMarkedObj();
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        AddUndo(new SdrUndoGeoObj(*pO));
        Point aRef(rRef-pM->GetPageView()->GetOffset());
        pO->Resize(aRef,xFact,yFact);
    }
    EndUndo();
}

long SdrEditView::GetMarkedObjRotate() const
{
    BOOL b1st=TRUE;
    BOOL bOk=TRUE;
    long nWink=0;
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz && bOk; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        long nWink2=pO->GetRotateAngle();
        if (b1st) nWink=nWink2;
        else if (nWink2!=nWink) bOk=FALSE;
        b1st=FALSE;
    }
    if (!bOk) nWink=0;
    return nWink;
}

void SdrEditView::RotateMarkedObj(const Point& rRef, long nWink, BOOL bCopy)
{
    XubString aStr;
    ImpTakeDescriptionStr(STR_EditRotate,aStr);
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr);
    if (bCopy) CopyMarkedObj();
    double nSin=sin(nWink*nPi180);
    double nCos=cos(nWink*nPi180);
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        AddUndo(new SdrUndoGeoObj(*pO));
        Point aRef(rRef-pM->GetPageView()->GetOffset());
        pO->Rotate(aRef,nWink,nSin,nCos);
    }
    EndUndo();
}

void SdrEditView::MirrorMarkedObj(const Point& rRef1, const Point& rRef2, BOOL bCopy)
{
    XubString aStr;
    Point aDif(rRef2-rRef1);
    if (aDif.X()==0) ImpTakeDescriptionStr(STR_EditMirrorHori,aStr);
    else if (aDif.Y()==0) ImpTakeDescriptionStr(STR_EditMirrorVert,aStr);
    else if (Abs(aDif.X())==Abs(aDif.Y())) ImpTakeDescriptionStr(STR_EditMirrorDiag,aStr);
    else ImpTakeDescriptionStr(STR_EditMirrorFree,aStr);
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr);
    if (bCopy) CopyMarkedObj();
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        AddUndo(new SdrUndoGeoObj(*pO));
        Point aRef1(rRef1-pM->GetPageView()->GetOffset());
        Point aRef2(rRef2-pM->GetPageView()->GetOffset());
        pO->Mirror(aRef1,aRef2);
    }
    EndUndo();
}

void SdrEditView::MirrorMarkedObjHorizontal(BOOL bCopy)
{
    Point aCenter(GetMarkedObjRect().Center());
    Point aPt2(aCenter);
    aPt2.Y()++;
    MirrorMarkedObj(aCenter,aPt2,bCopy);
}

void SdrEditView::MirrorMarkedObjVertical(BOOL bCopy)
{
    Point aCenter(GetMarkedObjRect().Center());
    Point aPt2(aCenter);
    aPt2.X()++;
    MirrorMarkedObj(aCenter,aPt2,bCopy);
}

long SdrEditView::GetMarkedObjShear() const
{
    BOOL b1st=TRUE;
    BOOL bOk=TRUE;
    long nWink=0;
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz && bOk; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        long nWink2=pO->GetShearAngle();
        if (b1st) nWink=nWink2;
        else if (nWink2!=nWink) bOk=FALSE;
        b1st=FALSE;
    }
    if (nWink>SDRMAXSHEAR) nWink=SDRMAXSHEAR;
    if (nWink<-SDRMAXSHEAR) nWink=-SDRMAXSHEAR;
    if (!bOk) nWink=0;
    return nWink;
}

void SdrEditView::ShearMarkedObj(const Point& rRef, long nWink, BOOL bVShear, BOOL bCopy)
{
    XubString aStr;
    ImpTakeDescriptionStr(STR_EditShear,aStr);
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr);
    if (bCopy) CopyMarkedObj();
    double nTan=tan(nWink*nPi180);
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        AddUndo(new SdrUndoGeoObj(*pO));
        Point aRef(rRef);
        Point aOfs(pM->GetPageView()->GetOffset());
        aRef-=aOfs;
        pO->Shear(aRef,nWink,nTan,bVShear);
    }
    EndUndo();
}

void SdrEditView::ImpCrookObj(SdrObject* pO, const Point& rRef, const Point& rRad,
    SdrCrookMode eMode, BOOL bVertical, BOOL bNoContortion, BOOL bRotate, const Rectangle& rMarkRect)
{
    SdrPathObj* pPath=PTR_CAST(SdrPathObj,pO);
    BOOL bDone = FALSE;

    if(pPath!=NULL && !bNoContortion)
    {
        XPolyPolygon aXPP(pPath->GetPathPoly());
        switch (eMode) {
            case SDRCROOK_ROTATE : CrookRotatePoly (aXPP,rRef,rRad,bVertical);           break;
            case SDRCROOK_SLANT  : CrookSlantPoly  (aXPP,rRef,rRad,bVertical);           break;
            case SDRCROOK_STRETCH: CrookStretchPoly(aXPP,rRef,rRad,bVertical,rMarkRect); break;
        } // switch
        pPath->SetPathPoly(aXPP);
        bDone = TRUE;
    }

    if(!bDone && !pPath && pO->IsPolyObj() && pO->GetPointCount() != 0)
    {
        // FuerPolyObj's, aber NICHT fuer SdrPathObj's, z.B. fuer's Bemassungsobjekt
        USHORT nPtAnz=pO->GetPointCount();
        XPolygon aXP(nPtAnz);
        USHORT nPtNum;
        for (nPtNum=0; nPtNum<nPtAnz; nPtNum++) {
            Point aPt(pO->GetPoint(nPtNum));
            aXP[nPtNum]=aPt;
        }
        switch (eMode) {
            case SDRCROOK_ROTATE : CrookRotatePoly (aXP,rRef,rRad,bVertical);           break;
            case SDRCROOK_SLANT  : CrookSlantPoly  (aXP,rRef,rRad,bVertical);           break;
            case SDRCROOK_STRETCH: CrookStretchPoly(aXP,rRef,rRad,bVertical,rMarkRect); break;
        } // switch
        for (nPtNum=0; nPtNum<nPtAnz; nPtNum++) {
            // hier koennte man vieleicht auch mal das Broadcasting optimieren
            // ist aber z.Zt. bei den 2 Punkten des Bemassungsobjekts noch nicht so tragisch
            pO->SetPoint(aXP[nPtNum],nPtNum);
        }
        bDone = TRUE;
    }

    if(!bDone)
    {
        // Fuer alle anderen oder wenn bNoContortion
        Point aCtr0(pO->GetSnapRect().Center());
        Point aCtr1(aCtr0);
        BOOL bRotOk=FALSE;
        double nSin,nCos;
        double nWink=0;
        if (rRad.X()!=0 && rRad.Y()!=0) {
            bRotOk=bRotate;
            switch (eMode) {
                case SDRCROOK_ROTATE : nWink=CrookRotateXPoint (aCtr1,NULL,NULL,rRef,rRad,nSin,nCos,bVertical); bRotOk=bRotate; break;
                case SDRCROOK_SLANT  : nWink=CrookSlantXPoint  (aCtr1,NULL,NULL,rRef,rRad,nSin,nCos,bVertical);           break;
                case SDRCROOK_STRETCH: nWink=CrookStretchXPoint(aCtr1,NULL,NULL,rRef,rRad,nSin,nCos,bVertical,rMarkRect); break;
            } // switch
        }
        aCtr1-=aCtr0;
        if (bRotOk) pO->Rotate(aCtr0,Round(nWink/nPi180),nSin,nCos);
        pO->Move(Size(aCtr1.X(),aCtr1.Y()));
    }
}

void SdrEditView::CrookMarkedObj(const Point& rRef, const Point& rRad, SdrCrookMode eMode,
    BOOL bVertical, BOOL bNoContortion, BOOL bCopy)
{
    Rectangle aMarkRect(GetMarkedObjRect());
    XubString aStr;
    BOOL bRotate=bNoContortion && eMode==SDRCROOK_ROTATE && IsRotateAllowed(FALSE);
    ImpTakeDescriptionStr(bNoContortion?STR_EditCrook:STR_EditCrookContortion,aStr);
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr);
    if (bCopy) CopyMarkedObj();
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        AddUndo(new SdrUndoGeoObj(*pO));
        Point aRef(rRef);
        Point aOfs(pM->GetPageView()->GetOffset());
        aRef-=aOfs;
        const SdrObjList* pOL=pO->GetSubList();
        if (bNoContortion || pOL==NULL) {
            ImpCrookObj(pO,aRef,rRad,eMode,bVertical,bNoContortion,bRotate,aMarkRect);
        } else {
            SdrObjListIter aIter(*pOL,IM_DEEPNOGROUPS);
            while (aIter.IsMore()) {
                SdrObject* pO1=aIter.Next();
                ImpCrookObj(pO1,aRef,rRad,eMode,bVertical,bNoContortion,bRotate,aMarkRect);
            }
        }
    }
    EndUndo();
}

void SdrEditView::ImpDistortObj(SdrObject* pO, const Rectangle& rRef, const XPolygon& rDistortedRect, BOOL bNoContortion)
{
    SdrPathObj* pPath=PTR_CAST(SdrPathObj,pO);
    if (!bNoContortion && pPath!=NULL) {
        XPolyPolygon aXPP(pPath->GetPathPoly());
        aXPP.Distort(rRef,rDistortedRect);
        pPath->SetPathPoly(aXPP);
    } else if (pO->IsPolyObj()) {
        // z.B. fuer's Bemassungsobjekt
        USHORT nPtAnz=pO->GetPointCount();
        XPolygon aXP(nPtAnz);
        USHORT nPtNum;
        for (nPtNum=0; nPtNum<nPtAnz; nPtNum++) {
            Point aPt(pO->GetPoint(nPtNum));
            aXP[nPtNum]=aPt;
        }
        aXP.Distort(rRef,rDistortedRect);
        for (nPtNum=0; nPtNum<nPtAnz; nPtNum++) {
            // hier koennte man vieleicht auch mal das Broadcasting optimieren
            // ist aber z.Zt. bei den 2 Punkten des Bemassungsobjekts noch nicht so tragisch
            pO->SetPoint(aXP[nPtNum],nPtNum);
        }
    }
}

void SdrEditView::DistortMarkedObj(const Rectangle& rRef, const XPolygon& rDistortedRect, BOOL bNoContortion, BOOL bCopy)
{
    XubString aStr;
    ImpTakeDescriptionStr(STR_EditDistort,aStr);
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr);
    if (bCopy) CopyMarkedObj();
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pO=pM->GetObj();
        AddUndo(new SdrUndoGeoObj(*pO));
        Rectangle aRefRect(rRef);
        XPolygon  aRefPoly(rDistortedRect);
        Point aOfs(pM->GetPageView()->GetOffset());
        if (aOfs.X()!=0 || aOfs.Y()!=0) {
            aRefRect.Move(-aOfs.X(),-aOfs.Y());
            aRefPoly.Move(-aOfs.X(),-aOfs.Y());
        }
        const SdrObjList* pOL=pO->GetSubList();
        if (bNoContortion || pOL==NULL) {
            ImpDistortObj(pO,aRefRect,aRefPoly,bNoContortion);
        } else {
            SdrObjListIter aIter(*pOL,IM_DEEPNOGROUPS);
            while (aIter.IsMore()) {
                SdrObject* pO1=aIter.Next();
                ImpDistortObj(pO1,aRefRect,aRefPoly,bNoContortion);
            }
        }
    }
    EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::SetNotPersistAttrToMarked(const SfxItemSet& rAttr, BOOL bReplaceAll)
{
    // bReplaceAll hat hier keinerlei Wirkung
    Rectangle aAllSnapRect(GetMarkedObjRect());
    const SfxPoolItem *pPoolItem=NULL;
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1X,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrTransformRef1XItem*)pPoolItem)->GetValue();
        SetRef1(Point(n,GetRef1().Y()));
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1Y,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrTransformRef1YItem*)pPoolItem)->GetValue();
        SetRef1(Point(GetRef1().X(),n));
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2X,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrTransformRef2XItem*)pPoolItem)->GetValue();
        SetRef2(Point(n,GetRef2().Y()));
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2Y,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrTransformRef2YItem*)pPoolItem)->GetValue();
        SetRef2(Point(GetRef2().X(),n));
    }
    long nAllPosX=0; BOOL bAllPosX=FALSE;
    long nAllPosY=0; BOOL bAllPosY=FALSE;
    long nAllWdt=0;  BOOL bAllWdt=FALSE;
    long nAllHgt=0;  BOOL bAllHgt=FALSE;
    BOOL bDoIt=FALSE;
    if (rAttr.GetItemState(SDRATTR_ALLPOSITIONX,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        nAllPosX=((const SdrAllPositionXItem*)pPoolItem)->GetValue();
        bAllPosX=TRUE; bDoIt=TRUE;
    }
    if (rAttr.GetItemState(SDRATTR_ALLPOSITIONY,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        nAllPosY=((const SdrAllPositionYItem*)pPoolItem)->GetValue();
        bAllPosY=TRUE; bDoIt=TRUE;
    }
    if (rAttr.GetItemState(SDRATTR_ALLSIZEWIDTH,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        nAllWdt=((const SdrAllSizeWidthItem*)pPoolItem)->GetValue();
        bAllWdt=TRUE; bDoIt=TRUE;
    }
    if (rAttr.GetItemState(SDRATTR_ALLSIZEHEIGHT,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        nAllHgt=((const SdrAllSizeHeightItem*)pPoolItem)->GetValue();
        bAllHgt=TRUE; bDoIt=TRUE;
    }
    if (bDoIt) {
        Rectangle aRect(aAllSnapRect); // !!! fuer PolyPt's und GluePt's aber bitte noch aendern !!!
        if (bAllPosX) aRect.Move(nAllPosX-aRect.Left(),0);
        if (bAllPosY) aRect.Move(0,nAllPosY-aRect.Top());
        if (bAllWdt)  aRect.Right()=aAllSnapRect.Left()+nAllWdt;
        if (bAllHgt)  aRect.Bottom()=aAllSnapRect.Top()+nAllHgt;
        SetMarkedObjRect(aRect);
    }
    if (rAttr.GetItemState(SDRATTR_RESIZEXALL,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        Fraction aXFact=((const SdrResizeXAllItem*)pPoolItem)->GetValue();
        ResizeMarkedObj(aAllSnapRect.TopLeft(),aXFact,Fraction(1,1));
    }
    if (rAttr.GetItemState(SDRATTR_RESIZEYALL,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        Fraction aYFact=((const SdrResizeYAllItem*)pPoolItem)->GetValue();
        ResizeMarkedObj(aAllSnapRect.TopLeft(),Fraction(1,1),aYFact);
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEALL,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long nAngle=((const SdrRotateAllItem*)pPoolItem)->GetValue();
        RotateMarkedObj(aAllSnapRect.Center(),nAngle);
    }
    if (rAttr.GetItemState(SDRATTR_HORZSHEARALL,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long nAngle=((const SdrHorzShearAllItem*)pPoolItem)->GetValue();
        ShearMarkedObj(aAllSnapRect.Center(),nAngle,FALSE);
    }
    if (rAttr.GetItemState(SDRATTR_VERTSHEARALL,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long nAngle=((const SdrVertShearAllItem*)pPoolItem)->GetValue();
        ShearMarkedObj(aAllSnapRect.Center(),nAngle,TRUE);
    }
    // Todo: WhichRange nach Notwendigkeit ueberpruefen.
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        const SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pObj=pM->GetObj();
        const SdrPageView* pPV=pM->GetPageView();
        AddUndo(new SdrUndoGeoObj(*pObj));
        pObj->ApplyNotPersistAttr(rAttr);
    }
}

void SdrEditView::MergeNotPersistAttrFromMarked(SfxItemSet& rAttr, BOOL bOnlyHardAttr) const
{
    // bOnlyHardAttr hat hier keinerlei Wirkung
    // Hier muss ausserdem noch der Nullpunkt und
    // die PvPos berueksichtigt werden.
    Rectangle aAllSnapRect(GetMarkedObjRect()); // !!! fuer PolyPt's und GluePt's aber bitte noch aendern !!!
    long nAllSnapPosX=aAllSnapRect.Left();
    long nAllSnapPosY=aAllSnapRect.Top();
    long nAllSnapWdt=aAllSnapRect.GetWidth()-1;
    long nAllSnapHgt=aAllSnapRect.GetHeight()-1;
    // koennte mal zu CheckPossibilities mit rein
    BOOL bMovProtect=FALSE,bMovProtectDC=FALSE;
    BOOL bSizProtect=FALSE,bSizProtectDC=FALSE;
    BOOL bPrintable =TRUE ,bPrintableDC=FALSE;
    SdrLayerID nLayerId=0; BOOL bLayerDC=FALSE;
    XubString aObjName;     BOOL bObjNameDC=FALSE,bObjNameSet=FALSE;
    long nSnapPosX=0;      BOOL bSnapPosXDC=FALSE;
    long nSnapPosY=0;      BOOL bSnapPosYDC=FALSE;
    long nSnapWdt=0;       BOOL bSnapWdtDC=FALSE;
    long nSnapHgt=0;       BOOL bSnapHgtDC=FALSE;
    long nLogicWdt=0;      BOOL bLogicWdtDC=FALSE,bLogicWdtDiff=FALSE;
    long nLogicHgt=0;      BOOL bLogicHgtDC=FALSE,bLogicHgtDiff=FALSE;
    long nRotAngle=0;      BOOL bRotAngleDC=FALSE;
    long nShrAngle=0;      BOOL bShrAngleDC=FALSE;
    Rectangle aSnapRect;
    Rectangle aLogicRect;
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        const SdrMark* pM=aMark.GetMark(nm);
        const SdrObject* pObj=pM->GetObj();
        if (nm==0) {
            nLayerId=pObj->GetLayer();
            bMovProtect=pObj->IsMoveProtect();
            bSizProtect=pObj->IsResizeProtect();
            bPrintable =pObj->IsPrintable();
            Rectangle aSnapRect(pObj->GetSnapRect());
            Rectangle aLogicRect(pObj->GetLogicRect());
            nSnapPosX=aSnapRect.Left();
            nSnapPosY=aSnapRect.Top();
            nSnapWdt=aSnapRect.GetWidth()-1;
            nSnapHgt=aSnapRect.GetHeight()-1;
            nLogicWdt=aLogicRect.GetWidth()-1;
            nLogicHgt=aLogicRect.GetHeight()-1;
            bLogicWdtDiff=nLogicWdt!=nSnapWdt;
            bLogicHgtDiff=nLogicHgt!=nSnapHgt;
            nRotAngle=pObj->GetRotateAngle();
            nShrAngle=pObj->GetShearAngle();
        } else {
            if (!bLayerDC      && nLayerId   !=pObj->GetLayer())        bLayerDC=TRUE;
            if (!bMovProtectDC && bMovProtect!=pObj->IsMoveProtect())   bMovProtectDC=TRUE;
            if (!bSizProtectDC && bSizProtect!=pObj->IsResizeProtect()) bSizProtectDC=TRUE;
            if (!bPrintableDC  && bPrintable !=pObj->IsPrintable())     bPrintableDC=TRUE;
            if (!bRotAngleDC   && nRotAngle  !=pObj->GetRotateAngle())  bRotAngleDC=TRUE;
            if (!bShrAngleDC   && nShrAngle  !=pObj->GetShearAngle())   bShrAngleDC=TRUE;
            if (!bSnapWdtDC || !bSnapHgtDC || !bSnapPosXDC || !bSnapPosYDC || !bLogicWdtDiff || !bLogicHgtDiff) {
                aSnapRect=pObj->GetSnapRect();
                if (nSnapPosX!=aSnapRect.Left()) bSnapPosXDC=TRUE;
                if (nSnapPosY!=aSnapRect.Top()) bSnapPosYDC=TRUE;
                if (nSnapWdt!=aSnapRect.GetWidth()-1) bSnapWdtDC=TRUE;
                if (nSnapHgt!=aSnapRect.GetHeight()-1) bSnapHgtDC=TRUE;
            }
            if (!bLogicWdtDC || !bLogicHgtDC || !bLogicWdtDiff || !bLogicHgtDiff) {
                aLogicRect=pObj->GetLogicRect();
                if (nLogicWdt!=aLogicRect.GetWidth()-1) bLogicWdtDC=TRUE;
                if (nLogicHgt!=aLogicRect.GetHeight()-1) bLogicHgtDC=TRUE;
                if (!bLogicWdtDiff && aSnapRect.GetWidth()!=aLogicRect.GetWidth()) bLogicWdtDiff=TRUE;
                if (!bLogicHgtDiff && aSnapRect.GetHeight()!=aLogicRect.GetHeight()) bLogicHgtDiff=TRUE;
            }
        }
        if (!bObjNameDC && pObj->HasSetName()) {
            if (!bObjNameSet) {
                aObjName=pObj->GetName();
            } else {
                if (aObjName!=pObj->GetName()) bObjNameDC=TRUE;
            }
        }
    }

    if (bSnapPosXDC || nAllSnapPosX!=nSnapPosX) rAttr.Put(SdrAllPositionXItem(nAllSnapPosX));
    if (bSnapPosYDC || nAllSnapPosY!=nSnapPosY) rAttr.Put(SdrAllPositionYItem(nAllSnapPosY));
    if (bSnapWdtDC  || nAllSnapWdt !=nSnapWdt ) rAttr.Put(SdrAllSizeWidthItem(nAllSnapWdt));
    if (bSnapHgtDC  || nAllSnapHgt !=nSnapHgt ) rAttr.Put(SdrAllSizeHeightItem(nAllSnapHgt));

    // Items fuer reine Transformationen
    rAttr.Put(SdrMoveXItem());
    rAttr.Put(SdrMoveYItem());
    rAttr.Put(SdrResizeXOneItem());
    rAttr.Put(SdrResizeYOneItem());
    rAttr.Put(SdrRotateOneItem());
    rAttr.Put(SdrHorzShearOneItem());
    rAttr.Put(SdrVertShearOneItem());
    if (nMarkAnz>1) {
        rAttr.Put(SdrResizeXAllItem());
        rAttr.Put(SdrResizeYAllItem());
        rAttr.Put(SdrRotateAllItem());
        rAttr.Put(SdrHorzShearAllItem());
        rAttr.Put(SdrVertShearAllItem());
    }

    if(eDragMode == SDRDRAG_ROTATE || eDragMode == SDRDRAG_MIRROR)
    {
        rAttr.Put(SdrTransformRef1XItem(GetRef1().X()));
        rAttr.Put(SdrTransformRef1YItem(GetRef1().Y()));
    }

    if(eDragMode == SDRDRAG_MIRROR)
    {
        rAttr.Put(SdrTransformRef2XItem(GetRef2().X()));
        rAttr.Put(SdrTransformRef2YItem(GetRef2().Y()));
    }
}

SfxItemSet SdrEditView::GetAttrFromMarked(BOOL bOnlyHardAttr) const
{
    SfxItemSet aSet(pMod->GetItemPool());
    MergeAttrFromMarked(aSet,bOnlyHardAttr);
    return aSet;
}

void SdrEditView::MergeAttrFromMarked(SfxItemSet& rAttr, BOOL bOnlyHardAttr) const
{
    sal_uInt32 nMarkAnz(aMark.GetMarkCount());

    for(sal_uInt32 a(0); a < nMarkAnz; a++)
    {
        const SfxItemSet& rSet = aMark.GetMark(a)->GetObj()->GetItemSet();
//-/        rAttr.MergeValues(rSet, TRUE);
        SfxWhichIter aIter(rSet);
        sal_uInt16 nWhich(aIter.FirstWhich());

        while(nWhich)
        {
            const SfxPoolItem* pItem = NULL;
            rSet.GetItemState(nWhich, TRUE, &pItem);

            if(pItem)
            {
                if(pItem == (SfxPoolItem *)-1)
                    rAttr.InvalidateItem(nWhich);
                else
                    rAttr.MergeValue(*pItem, TRUE);
            }
            nWhich = aIter.NextWhich();
        }
    }



//-/    ULONG nMarkAnz=aMark.GetMarkCount();
//-/    for (ULONG nm=0; nm<nMarkAnz; nm++) {
//-/        SdrMark* pM=aMark.GetMark(nm);
//-///-/        pM->GetObj()->TakeAttributes(rAttr,TRUE,bOnlyHardAttr);
//-///-/        rAttr.Put(pM->GetObj()->GetItemSet());
//-/        const SfxItemSet& rSet = pM->GetObj()->GetItemSet();
//-/
//-/
//-/
//-/
//-/
//-/        static BOOL bTestOwnLoop(FALSE);
//-/        if(bTestOwnLoop)
//-/        {
//-/
//-/            SfxWhichIter aIter(rSet);
//-/            sal_uInt16 nWhich(aIter.FirstWhich());
//-/
//-/            while(nWhich)
//-/            {
//-/                rAttr.MergeValue(rSet.Get(nWhich), TRUE);
//-/                nWhich = aIter.NextWhich();
//-/            }
//-/
//-/        }
//-/        else
//-/        {
//-/
//-/            rAttr.MergeValues(rSet, TRUE);
//-/
//-/        }

//-/            SfxWhichIter aIter(rIS);
//-/            USHORT nWhich=aIter.FirstWhich();
//-/            while (nWhich!=0) {
//-/                if ((bHasLEnd || nWhich<XATTR_LINESTART || nWhich>XATTR_LINEENDCENTER) &&
//-/                    (bHasEckRad || nWhich!=SDRATTR_ECKENRADIUS) &&
//-/                    (bHasText || nWhich<SDRATTR_TEXT_MINFRAMEHEIGHT || nWhich>SDRATTR_TEXT_CONTOURFRAME) &&
//-/                    (bTextFrame || (nWhich!=SDRATTR_TEXT_AUTOGROWHEIGHT && nWhich!=SDRATTR_TEXT_MINFRAMEHEIGHT && nWhich!=SDRATTR_TEXT_MAXFRAMEHEIGHT &&
//-/                                    nWhich!=SDRATTR_TEXT_AUTOGROWWIDTH && nWhich!=SDRATTR_TEXT_MINFRAMEWIDTH && nWhich!=SDRATTR_TEXT_MAXFRAMEWIDTH)) &&
//-/                    (bCanContourFrame || nWhich!=SDRATTR_TEXT_CONTOURFRAME) &&
//-/                    (!bIsContourFrame || nWhich<SDRATTR_TEXT_MINFRAMEHEIGHT || nWhich>SDRATTR_TEXT_HORZADJUST) &&
//-/                    (nWhich<EE_FEATURE_START || nWhich>EE_FEATURE_END) )
//-/                {
//-/                    if (!bOnlyHardAttr || rIS.GetItemState(nWhich,FALSE)==SFX_ITEM_SET) {
//-/                        const SfxPoolItem& rItem=rIS.Get(nWhich);
//-/                        if (bMerge) rAttr.MergeValue(rItem,TRUE);
//-/                        else rAttr.Put(rItem);
//-/                    }
//-/                }
//-/                nWhich=aIter.NextWhich();
//-/            }



//-/        SfxItemIter aIter(rSet);
//-/        const SfxPoolItem* pItem = aIter.FirstItem();
//-/
//-/        // merge items to destination rAttr
//-/        while(pItem)
//-/        {
//-/            sal_uInt16 nWhich = pItem->Which();
//-/            SfxItemState eState = rSet.GetItemState(nWhich);
//-/
//-/            if(SFX_ITEM_SET == eState)
//-/            {
//-/                // Ist gesetzt
//-/                if(*rAttr.GetItem(nWhich) != *pItem)
//-/                {
//-/                    // SfxPoolItem muss invalidiert werden
//-/                    rAttr.InvalidateItem(nWhich);
//-/                }
//-/            }
//-/            else
//-/            {
//-/                if(SFX_ITEM_DONTCARE != eState)
//-/                {
//-/                    // Item gab es noch nicht, setze es
//-/                    rAttr.Put(*pItem);
//-/                }
//-/            }
//-/
//-/            pItem = aIter.NextItem();
//-/        }
//-/    }
}

void SdrEditView::SetAttrToMarked(const SfxItemSet& rAttr, BOOL bReplaceAll)
{
    if (HasMarkedObj()) {
#ifdef DBG_UTIL
        {
            BOOL bHasEEFeatureItems=FALSE;
            SfxItemIter aIter(rAttr);
            const SfxPoolItem* pItem=aIter.FirstItem();
            while (!bHasEEFeatureItems && pItem!=NULL) {
                if (!IsInvalidItem(pItem)) {
                    USHORT nW=pItem->Which();
                    if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END) bHasEEFeatureItems=TRUE;
                }
                pItem=aIter.NextItem();
            }
            if(bHasEEFeatureItems)
            {
                String aMessage;
                aMessage.AppendAscii("SdrEditView::SetAttrToMarked(): Das setzen von EE_FEATURE-Items an der SdrView macht keinen Sinn! Es fuehrt nur zu Overhead und nicht mehr lesbaren Dokumenten.");
                InfoBox(NULL, aMessage).Execute();
            }
        }
#endif
        // Joe, 2.7.98: Damit Undo nach Format.Standard auch die Textattribute korrekt restauriert
        BOOL bHasEEItems=SearchOutlinerItems(rAttr,bReplaceAll);
        XubString aStr;
        ImpTakeDescriptionStr(STR_EditSetAttributes,aStr);

        // AW 030100: save additional geom info when para or char attributes
        // are changed and the geom form of the text object might be changed
        BOOL bPossibleGeomChange(FALSE);
        SfxWhichIter aIter(rAttr);
        UINT16 nWhich = aIter.FirstWhich();
        while(!bPossibleGeomChange && nWhich)
        {
            SfxItemState eState = rAttr.GetItemState(nWhich);
            if(eState == SFX_ITEM_SET)
            {
                if(nWhich >= SDRATTR_TEXT_MINFRAMEHEIGHT && nWhich <= SDRATTR_TEXT_CONTOURFRAME)
                    bPossibleGeomChange = TRUE;
            }
            nWhich = aIter.NextWhich();
        }

        BegUndo(aStr);
        ULONG nMarkAnz=aMark.GetMarkCount();
        for (ULONG nm=0; nm<nMarkAnz; nm++) {
            SdrMark* pM=aMark.GetMark(nm);

            // new geometry undo
            if(bPossibleGeomChange)
            {
                // save position and size of obect, too
                AddUndo(new SdrUndoGeoObj(*pM->GetObj()));
            }

            // add attribute undo
            AddUndo(new SdrUndoAttrObj(*pM->GetObj(),FALSE,bHasEEItems || bPossibleGeomChange));

//-/            pM->GetObj()->SetAttributes(rAttr,bReplaceAll);
            SdrBroadcastItemChange aItemChange(*pM->GetObj());
            if(bReplaceAll)
                pM->GetObj()->ClearItem();
            pM->GetObj()->SetItemSet(rAttr);
            pM->GetObj()->BroadcastItemChange(aItemChange);
        }
        // besser vorher checken, was gemacht werden soll:
        // pObj->SetAttr() oder SetNotPersistAttr()
        // !!! fehlende Implementation !!!
        SetNotPersistAttrToMarked(rAttr,bReplaceAll);
        EndUndo();
    }
}

SfxStyleSheet* SdrEditView::GetStyleSheetFromMarked() const
{
    SfxStyleSheet* pRet=NULL;
    BOOL b1st=TRUE;
    ULONG nMarkAnz=aMark.GetMarkCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SfxStyleSheet* pSS=pM->GetObj()->GetStyleSheet();
        if (b1st) pRet=pSS;
        else if (pRet!=pSS) return NULL; // verschiedene StyleSheets
        b1st=FALSE;
    }
    return pRet;
}

void SdrEditView::SetStyleSheetToMarked(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
{
    if (HasMarkedObj()) {
        XubString aStr;
        if (pStyleSheet!=NULL) ImpTakeDescriptionStr(STR_EditSetStylesheet,aStr);
        else ImpTakeDescriptionStr(STR_EditDelStylesheet,aStr);
        BegUndo(aStr);
        ULONG nMarkAnz=aMark.GetMarkCount();
        for (ULONG nm=0; nm<nMarkAnz; nm++) {
            SdrMark* pM=aMark.GetMark(nm);
            AddUndo(new SdrUndoGeoObj(*pM->GetObj()));
            AddUndo(new SdrUndoAttrObj(*pM->GetObj(),TRUE,TRUE));
            pM->GetObj()->SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
        }
        EndUndo();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/* new interface src537 */
BOOL SdrEditView::GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr) const
{
    if(aMark.GetMarkCount())
    {
        rTargetSet.Put(GetAttrFromMarked(bOnlyHardAttr), FALSE);
        return TRUE;
    }
    else
    {
        return SdrMarkView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

BOOL SdrEditView::SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll)
{
    if (aMark.GetMarkCount()!=0) {
        SetAttrToMarked(rSet,bReplaceAll);
        return TRUE;
    } else {
        return SdrMarkView::SetAttributes(rSet,bReplaceAll);
    }
}

SfxStyleSheet* SdrEditView::GetStyleSheet(BOOL& rOk) const
{
    if (aMark.GetMarkCount()!=0) {
        rOk=TRUE;
        return GetStyleSheetFromMarked();
    } else {
        return SdrMarkView::GetStyleSheet(rOk);
    }
}

BOOL SdrEditView::SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
{
    if (aMark.GetMarkCount()!=0) {
        SetStyleSheetToMarked(pStyleSheet,bDontRemoveHardAttr);
        return TRUE;
    } else {
        return SdrMarkView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SfxItemSet SdrEditView::GetGeoAttrFromMarked() const
{
    SfxItemSet aRetSet(pMod->GetItemPool(),   // SID_ATTR_TRANSFORM_... aus s:svxids.hrc
                       SID_ATTR_TRANSFORM_POS_X,SID_ATTR_TRANSFORM_ANGLE,
                       SID_ATTR_TRANSFORM_PROTECT_POS,SID_ATTR_TRANSFORM_AUTOHEIGHT,
                       SDRATTR_ECKENRADIUS,SDRATTR_ECKENRADIUS,
                       0);
    if (HasMarkedObj()) {
        SfxItemSet aMarkAttr(GetAttrFromMarked(FALSE)); // wg. AutoGrowHeight und Eckenradius
        Rectangle aRect(GetMarkedObjRect());
        if (GetPageViewCount()==1) { // hier mal etwas verbessern !!!
            GetPageViewPvNum(0)->LogicToPagePos(aRect);
        }

        // Position
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_POS_X,aRect.Left()));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_POS_Y,aRect.Top()));

        // Groesse
        long nResizeRefX=aRect.Left();
        long nResizeRefY=aRect.Top();
        if (eDragMode==SDRDRAG_ROTATE) { // Drehachse auch als Referenz fuer Resize
            nResizeRefX=aRef1.X();
            nResizeRefY=aRef1.Y();
        }
        aRetSet.Put(SfxUInt32Item(SID_ATTR_TRANSFORM_WIDTH,aRect.Right()-aRect.Left()));
        aRetSet.Put(SfxUInt32Item(SID_ATTR_TRANSFORM_HEIGHT,aRect.Bottom()-aRect.Top()));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_RESIZE_REF_X,nResizeRefX));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_RESIZE_REF_Y,nResizeRefY));

        Point aRotateAxe(aRef1);
        if (GetPageViewCount()==1) { // hier mal etwas verbessern !!!
            GetPageViewPvNum(0)->LogicToPagePos(aRotateAxe);
        }

        // Drehung
        long nRotateRefX=aRect.Center().X();
        long nRotateRefY=aRect.Center().Y();
        if (eDragMode==SDRDRAG_ROTATE) {
            nRotateRefX=aRotateAxe.X();
            nRotateRefY=aRotateAxe.Y();
        }
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_ANGLE,GetMarkedObjRotate()));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_ROT_X,nRotateRefX));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_ROT_Y,nRotateRefY));

        // Shear
        long nShearRefX=aRect.Left();
        long nShearRefY=aRect.Bottom();
        if (eDragMode==SDRDRAG_ROTATE) { // Drehachse auch als Referenz fuer Shear
            nShearRefX=aRotateAxe.X();
            nShearRefY=aRotateAxe.Y();
        }
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR,GetMarkedObjShear()));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_X,nShearRefX));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_Y,nShearRefY));

        // Pruefen der einzelnen Objekte, ob Objekte geschuetzt sind
        const SdrMarkList& rMarkList=GetMarkList();
        ULONG nMarkCount=rMarkList.GetMarkCount();
        SdrObject* pObj=rMarkList.GetMark(0)->GetObj();
        BOOL bPosProt=pObj->IsMoveProtect();
        BOOL bSizProt=pObj->IsResizeProtect();
        BOOL bPosProtDontCare=FALSE;
        BOOL bSizProtDontCare=FALSE;
        for (ULONG i=1; i<nMarkCount && (!bPosProtDontCare || !bSizProtDontCare); i++) {
            pObj=rMarkList.GetMark(i)->GetObj();
            if (bPosProt!=pObj->IsMoveProtect()) bPosProtDontCare=TRUE;
            if (bSizProt!=pObj->IsResizeProtect()) bSizProtDontCare=TRUE;
        }

        // InvalidateItem setzt das Item auf DONT_CARE
        if (bPosProtDontCare) {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_PROTECT_POS);
        } else {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_PROTECT_POS,bPosProt));
        }
        if (bSizProtDontCare) {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_PROTECT_SIZE);
        } else {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_PROTECT_SIZE,bSizProt));
        }

        SfxItemState eState=aMarkAttr.GetItemState(SDRATTR_TEXT_AUTOGROWWIDTH);
        BOOL bAutoGrow=((SdrTextAutoGrowWidthItem&)(aMarkAttr.Get(SDRATTR_TEXT_AUTOGROWWIDTH))).GetValue();
        if (eState==SFX_ITEM_DONTCARE) {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_AUTOWIDTH);
        } else if (eState==SFX_ITEM_SET) {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_AUTOWIDTH,bAutoGrow));
        }

        eState=aMarkAttr.GetItemState(SDRATTR_TEXT_AUTOGROWHEIGHT);
        bAutoGrow=((SdrTextAutoGrowHeightItem&)(aMarkAttr.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();
        if (eState==SFX_ITEM_DONTCARE) {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_AUTOHEIGHT);
        } else if (eState==SFX_ITEM_SET) {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_AUTOHEIGHT,bAutoGrow));
        }

        eState=aMarkAttr.GetItemState(SDRATTR_ECKENRADIUS);
        long nRadius=((SdrEckenradiusItem&)(aMarkAttr.Get(SDRATTR_ECKENRADIUS))).GetValue();
        if (eState==SFX_ITEM_DONTCARE) {
            aRetSet.InvalidateItem(SDRATTR_ECKENRADIUS);
        } else if (eState==SFX_ITEM_SET) {
            aRetSet.Put(SdrEckenradiusItem(nRadius));
        }

    }
    return aRetSet;
}

Point ImpGetPoint(Rectangle aRect, RECT_POINT eRP)
{
    switch(eRP) {
        case RP_LT: return aRect.TopLeft();
        case RP_MT: return aRect.TopCenter();
        case RP_RT: return aRect.TopRight();
        case RP_LM: return aRect.LeftCenter();
        case RP_MM: return aRect.Center();
        case RP_RM: return aRect.RightCenter();
        case RP_LB: return aRect.BottomLeft();
        case RP_MB: return aRect.BottomCenter();
        case RP_RB: return aRect.BottomRight();
    }
    return Point(); // Sollte nicht vorkommen !
}

void SdrEditView::SetGeoAttrToMarked(const SfxItemSet& rAttr)
{
    Rectangle aRect(GetMarkedObjRect());
    if (GetPageViewCount()==1) {
        GetPageViewPvNum(0)->LogicToPagePos(aRect);
    }
    long nOldRotateAngle=GetMarkedObjRotate();
    long nOldShearAngle=GetMarkedObjShear();
    const SdrMarkList& rMarkList=GetMarkList();
    ULONG nMarkCount=rMarkList.GetMarkCount();
    SdrObject* pObj=NULL;

    RECT_POINT eSizePoint=RP_MM;
    long nPosDX=0;
    long nPosDY=0;
    long nSizX=0;
    long nSizY=0;
    long nRotateAngle=0;
    long nRotateX=0;
    long nRotateY=0;
    long nNewShearAngle=0;
    long nShearAngle=0;
    long nShearX=0;
    long nShearY=0;
    BOOL bShearVert=FALSE;

    BOOL bChgPos=FALSE;
    BOOL bChgSiz=FALSE;
    BOOL bChgHgt=FALSE;
    BOOL bRotate=FALSE;
    BOOL bShear =FALSE;

    BOOL bSetAttr=FALSE;
    SfxItemSet aSetAttr(pMod->GetItemPool());

    const SfxPoolItem* pPoolItem=NULL;

    // Position
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_POS_X,TRUE,&pPoolItem)) {
        nPosDX=((const SfxInt32Item*)pPoolItem)->GetValue()-aRect.Left();
        bChgPos=TRUE;
    }
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_POS_Y,TRUE,&pPoolItem)){
        nPosDY=((const SfxInt32Item*)pPoolItem)->GetValue()-aRect.Top();
        bChgPos=TRUE;
    }
    // Groesse
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_WIDTH,TRUE,&pPoolItem)) {
        nSizX=((const SfxUInt32Item*)pPoolItem)->GetValue();
        bChgSiz=TRUE;
    }
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_HEIGHT,TRUE,&pPoolItem)) {
        nSizY=((const SfxUInt32Item*)pPoolItem)->GetValue();
        bChgSiz=TRUE;
        bChgHgt=TRUE;
    }
    if (bChgSiz) {
        eSizePoint=(RECT_POINT)((const SfxAllEnumItem&)rAttr.Get(SID_ATTR_TRANSFORM_SIZE_POINT)).GetValue();
    }

    // Rotation
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_ANGLE,TRUE,&pPoolItem)) {
        nRotateAngle=((const SfxInt32Item*)pPoolItem)->GetValue()-nOldRotateAngle;
        bRotate=nRotateAngle!=0;
        if (bRotate) {
            nRotateX=((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_ROT_X)).GetValue();
            nRotateY=((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_ROT_Y)).GetValue();
        }
    }

    // Shear
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_SHEAR,TRUE,&pPoolItem)) {
        nNewShearAngle=((const SfxInt32Item*)pPoolItem)->GetValue();
        if (nNewShearAngle>SDRMAXSHEAR) nNewShearAngle=SDRMAXSHEAR;
        if (nNewShearAngle<-SDRMAXSHEAR) nNewShearAngle=-SDRMAXSHEAR;
        if (nNewShearAngle!=nOldShearAngle) {
            bShearVert=((const SfxBoolItem&)rAttr.Get(SID_ATTR_TRANSFORM_SHEAR_VERTICAL)).GetValue();
            if (bShearVert) {
                nShearAngle=nNewShearAngle;
            } else {
                if (nNewShearAngle!=0 && nOldShearAngle!=0) {
                    // Bugfix #25714#.
                    double nOld=tan((double)nOldShearAngle*nPi180);
                    double nNew=tan((double)nNewShearAngle*nPi180);
                    nNew-=nOld;
                    nNew=atan(nNew)/nPi180;
                    nShearAngle=Round(nNew);
                } else {
                    nShearAngle=nNewShearAngle-nOldShearAngle;
                }
            }
            bShear=nShearAngle!=0;
            if (bShear) {
                nShearX=((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_SHEAR_X)).GetValue();
                nShearY=((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_SHEAR_Y)).GetValue();
            }
        }
    }

    // AutoGrow
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_AUTOWIDTH,TRUE,&pPoolItem)) {
        BOOL bAutoGrow=((const SfxBoolItem*)pPoolItem)->GetValue();
        aSetAttr.Put(SdrTextAutoGrowWidthItem(bAutoGrow));
        bSetAttr=TRUE;
    }

    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_AUTOHEIGHT,TRUE,&pPoolItem)) {
        BOOL bAutoGrow=((const SfxBoolItem*)pPoolItem)->GetValue();
        aSetAttr.Put(SdrTextAutoGrowHeightItem(bAutoGrow));
        bSetAttr=TRUE;
    }

    // Eckenradius
    if (SFX_ITEM_SET==rAttr.GetItemState(SDRATTR_ECKENRADIUS,TRUE,&pPoolItem)) {
        long nRadius=((SdrEckenradiusItem*)pPoolItem)->GetValue();
        aSetAttr.Put(SdrEckenradiusItem(nRadius));
        bSetAttr=TRUE;
    }

    ForcePossibilities();

    BegUndo(ImpGetResStr(STR_EditTransform),aMark.GetMarkDescription());

    if (bSetAttr) {
        SetAttrToMarked(aSetAttr,FALSE);
    }

    // Groesse und Hoehe aendern
    if (bChgSiz && (bResizeFreeAllowed || bResizePropAllowed)) {
        Fraction aWdt(nSizX,aRect.Right()-aRect.Left());
        Fraction aHgt(nSizY,aRect.Bottom()-aRect.Top());
        Point aRef(ImpGetPoint(aRect,eSizePoint));
        if (GetPageViewCount()==1) {
            GetPageViewPvNum(0)->PagePosToLogic(aRef);
        }
        ResizeMarkedObj(aRef,aWdt,aHgt);
    }

    // Rotieren
    if (bRotate && (bRotateFreeAllowed || bRotate90Allowed)) {
        Point aRef(nRotateX,nRotateY);
        if (GetPageViewCount()==1) {
            GetPageViewPvNum(0)->PagePosToLogic(aRef);
        }
        RotateMarkedObj(aRef,nRotateAngle);
    }

    // Shear
    if (bShear && bShearAllowed) {
        Point aRef(nShearX,nShearY);
        if (GetPageViewCount()==1) {
            GetPageViewPvNum(0)->PagePosToLogic(aRef);
        }
        ShearMarkedObj(aRef,nShearAngle,bShearVert);
        long nTempAngle=GetMarkedObjShear();
        if (nTempAngle!=0 && nTempAngle!=nNewShearAngle && !bShearVert) {
            // noch eine 2. Iteration zur Kompensation der Rundungsfehler
            double nOld=tan((double)nTempAngle*nPi180);
            double nNew=tan((double)nNewShearAngle*nPi180);
            nNew-=nOld;
            nNew=atan(nNew)/nPi180;
            nTempAngle=Round(nNew);
            if (nTempAngle!=0) {
                ShearMarkedObj(aRef,nTempAngle,bShearVert);
            }
        }
    }

    // Position aendern
    if (bChgPos && bMoveAllowed) {
        MoveMarkedObj(Size(nPosDX,nPosDY));
    }

    // Position geschuetzt
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_PROTECT_POS,TRUE,&pPoolItem)) {
        BOOL bProtPos=((const SfxBoolItem*)pPoolItem)->GetValue();
        for (ULONG i=0; i<nMarkCount; i++) {
            pObj=rMarkList.GetMark(i)->GetObj();
            pObj->SetMoveProtect(bProtPos);
        }
        // BugFix 13897: hier muesste besser ein Broadcast her!
        // Ausserdem fehlt Undo
        // oder -> bProtPos/Size als Item (Interface)
        bMoveProtect=bProtPos;
    }

    // Groesse geschuetzt
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_PROTECT_SIZE,TRUE,&pPoolItem)) {
        BOOL bProtSize=((const SfxBoolItem*)pPoolItem)->GetValue();
        for (ULONG i=0; i<nMarkCount; i++) {
            pObj=rMarkList.GetMark(i)->GetObj();
            pObj->SetResizeProtect(bProtSize);
        }
        // BugFix 13897: hier muesste besser ein Broadcast her!
        // Ausserdem fehlt Undo
        // oder -> bProtPos/Size als Item (Interface)
        bResizeProtect=bProtSize;
    }
    EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrEditView::IsAlignPossible() const
{  // Mindestens 2 markierte Objekte, davon mind. 1 beweglich
    ForcePossibilities();
    ULONG nAnz=aMark.GetMarkCount();
    if (nAnz==0) return FALSE;         // Nix markiert!
    if (nAnz==1) return bMoveAllowed;  // einzelnes Obj an der Seite ausrichten
    return bOneOrMoreMovable;          // ansonsten ist MarkCount>=2
}

void SdrEditView::AlignMarkedObjects(SdrHorAlign eHor, SdrVertAlign eVert, BOOL bBoundRects)
{
    if (eHor==SDRHALIGN_NONE && eVert==SDRVALIGN_NONE) return;
    aMark.ForceSort();
    if (aMark.GetMarkCount()<1) return;
    XubString aStr(aMark.GetMarkDescription());
    if (eHor==SDRHALIGN_NONE) {
        switch (eVert) {
            case SDRVALIGN_TOP   : ImpTakeDescriptionStr(STR_EditAlignVTop   ,aStr); break;
            case SDRVALIGN_BOTTOM: ImpTakeDescriptionStr(STR_EditAlignVBottom,aStr); break;
            case SDRVALIGN_CENTER: ImpTakeDescriptionStr(STR_EditAlignVCenter,aStr); break;
        }
    } else if (eVert==SDRVALIGN_NONE) {
        switch (eHor) {
            case SDRHALIGN_LEFT  : ImpTakeDescriptionStr(STR_EditAlignHLeft  ,aStr); break;
            case SDRHALIGN_RIGHT : ImpTakeDescriptionStr(STR_EditAlignHRight ,aStr); break;
            case SDRHALIGN_CENTER: ImpTakeDescriptionStr(STR_EditAlignHCenter,aStr); break;
        }
    } else if (eHor==SDRHALIGN_CENTER && eVert==SDRVALIGN_CENTER) {
        ImpTakeDescriptionStr(STR_EditAlignCenter,aStr);
    } else {
        ImpTakeDescriptionStr(STR_EditAlign,aStr);
    }
    BegUndo(aStr);
    Rectangle aBound;
    ULONG nMarkAnz=aMark.GetMarkCount();
    ULONG nm;
    BOOL bHasFixed=FALSE;
    for (nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pObj=pM->GetObj();
        SdrObjTransformInfoRec aInfo;
        pObj->TakeObjInfo(aInfo);
        if (!aInfo.bMoveAllowed || pObj->IsMoveProtect()) {
            Point aOfs(pM->GetPageView()->GetOffset());
            Rectangle aObjRect(bBoundRects?pObj->GetBoundRect():pObj->GetSnapRect());
            aObjRect.Move(aOfs.X(),aOfs.Y());
            aBound.Union(aObjRect);
            bHasFixed=TRUE;
        }
    }
    if (!bHasFixed) {
        if (nMarkAnz==1) { // einzelnes Obj an der Seite ausrichten
            const SdrObject* pObj=aMark.GetMark(0L)->GetObj();
            const SdrPage* pPage=pObj->GetPage();
            const SdrPageGridFrameList* pGFL=pPage->GetGridFrameList(aMark.GetMark(0L)->GetPageView(),&(pObj->GetSnapRect()));
            const SdrPageGridFrame* pFrame=NULL;
            if (pGFL!=NULL && pGFL->GetCount()!=0) { // Writer
                pFrame=&((*pGFL)[0]);
            }
            if (pFrame!=NULL) { // Writer
                aBound=pFrame->GetUserArea();
            } else {
                aBound=Rectangle(pPage->GetLftBorder(),pPage->GetUppBorder(),
                                 pPage->GetWdt()-pPage->GetRgtBorder(),
                                 pPage->GetHgt()-pPage->GetLwrBorder());
            }
        } else {
            if (bBoundRects) aBound=GetMarkedObjBoundRect();
            else aBound=GetMarkedObjRect();
        }
    }
    Point aCenter(aBound.Center());
    for (nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aMark.GetMark(nm);
        SdrObject* pObj=pM->GetObj();
        SdrObjTransformInfoRec aInfo;
        pObj->TakeObjInfo(aInfo);
        if (aInfo.bMoveAllowed && !pObj->IsMoveProtect()) {
            SdrPageView* pPV=pM->GetPageView();
            Point aOfs(pPV->GetOffset());
            long nXMov=0;
            long nYMov=0;
            Rectangle aObjRect(bBoundRects?pObj->GetBoundRect():pObj->GetSnapRect());
            switch (eVert) {
                case SDRVALIGN_TOP   : nYMov=aBound.Top()   -aObjRect.Top()       -aOfs.Y(); break;
                case SDRVALIGN_BOTTOM: nYMov=aBound.Bottom()-aObjRect.Bottom()    -aOfs.Y(); break;
                case SDRVALIGN_CENTER: nYMov=aCenter.Y()    -aObjRect.Center().Y()-aOfs.Y(); break;
            }
            switch (eHor) {
                case SDRHALIGN_LEFT  : nXMov=aBound.Left()  -aObjRect.Left()      -aOfs.X(); break;
                case SDRHALIGN_RIGHT : nXMov=aBound.Right() -aObjRect.Right()     -aOfs.X(); break;
                case SDRHALIGN_CENTER: nXMov=aCenter.X()    -aObjRect.Center().X()-aOfs.X(); break;
            }
            if (nXMov!=0 || nYMov!=0) {
                AddUndo(new SdrUndoMoveObj(*pObj,Size(nXMov,nYMov)));
                pObj->Move(Size(nXMov,nYMov));
            }
        }
    }
    EndUndo();
}

