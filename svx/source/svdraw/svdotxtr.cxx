/*************************************************************************
 *
 *  $RCSfile: svdotxtr.cxx,v $
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

#include "svdotext.hxx"
#include "xoutx.hxx"     // fuer XOutCreatePolygon
#include "svditext.hxx"
#include "svdtrans.hxx"
#include "svdogrp.hxx"
#include "svdopath.hxx"
#include "svdoutl.hxx"
#include "svdtxhdl.hxx"  // DrawTextToPath fuer Convert
#include "svdpage.hxx"   // fuer Convert
#include "svdmodel.hxx"  // fuer Convert

#ifndef _OUTLINER_HXX //autogen
#include "outliner.hxx"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@@ @@@@@ @@   @@ @@@@@@  @@@@  @@@@@  @@@@@@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@     @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@@@    @@@     @@   @@  @@ @@@@@      @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@ @@  @@
//    @@   @@@@@ @@   @@   @@    @@@@  @@@@@   @@@@
//
//  Transformationen
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrTextObj::NbcSetSnapRect(const Rectangle& rRect)
{
    if (aGeo.nDrehWink!=0 || aGeo.nShearWink!=0) {
        Rectangle aSR0(GetSnapRect());
        long nWdt0=aSR0.Right()-aSR0.Left();
        long nHgt0=aSR0.Bottom()-aSR0.Top();
        long nWdt1=rRect.Right()-rRect.Left();
        long nHgt1=rRect.Bottom()-rRect.Top();
        SdrTextObj::NbcResize(maSnapRect.TopLeft(),Fraction(nWdt1,nWdt0),Fraction(nHgt1,nHgt0));
        SdrTextObj::NbcMove(Size(rRect.Left()-aSR0.Left(),rRect.Top()-aSR0.Top()));
    } else {
        long nHDist=GetTextLeftDistance()+GetTextRightDistance();
        long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
        long nTWdt0=aRect.GetWidth ()-1-nHDist; if (nTWdt0<0) nTWdt0=0;
        long nTHgt0=aRect.GetHeight()-1-nVDist; if (nTHgt0<0) nTHgt0=0;
        long nTWdt1=rRect.GetWidth ()-1-nHDist; if (nTWdt1<0) nTWdt1=0;
        long nTHgt1=rRect.GetHeight()-1-nVDist; if (nTHgt1<0) nTHgt1=0;
        aRect=rRect;
        ImpJustifyRect(aRect);
        if (bTextFrame && (pModel==NULL || !pModel->IsPasteResize())) { // #51139#
            if (nTWdt0!=nTWdt1 && IsAutoGrowWidth() ) NbcSetMinTextFrameWidth(nTWdt1);
            if (nTHgt0!=nTHgt1 && IsAutoGrowHeight()) NbcSetMinTextFrameHeight(nTHgt1);
            if (GetFitToSize()==SDRTEXTFIT_RESIZEATTR) {
                NbcResizeTextAttributes(Fraction(nTWdt1,nTWdt0),Fraction(nTHgt1,nTHgt0));
            }
            NbcAdjustTextFrameWidthAndHeight();
        }
        ImpCheckShear();
        SetRectsDirty();
    }
}

const Rectangle& SdrTextObj::GetLogicRect() const
{
    return aRect;
}

void SdrTextObj::NbcSetLogicRect(const Rectangle& rRect)
{
    long nHDist=GetTextLeftDistance()+GetTextRightDistance();
    long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
    long nTWdt0=aRect.GetWidth ()-1-nHDist; if (nTWdt0<0) nTWdt0=0;
    long nTHgt0=aRect.GetHeight()-1-nVDist; if (nTHgt0<0) nTHgt0=0;
    long nTWdt1=rRect.GetWidth ()-1-nHDist; if (nTWdt1<0) nTWdt1=0;
    long nTHgt1=rRect.GetHeight()-1-nVDist; if (nTHgt1<0) nTHgt1=0;
    aRect=rRect;
    ImpJustifyRect(aRect);
    if (bTextFrame) {
        if (nTWdt0!=nTWdt1 && IsAutoGrowWidth() ) NbcSetMinTextFrameWidth(nTWdt1);
        if (nTHgt0!=nTHgt1 && IsAutoGrowHeight()) NbcSetMinTextFrameHeight(nTHgt1);
        if (GetFitToSize()==SDRTEXTFIT_RESIZEATTR) {
            NbcResizeTextAttributes(Fraction(nTWdt1,nTWdt0),Fraction(nTHgt1,nTHgt0));
        }
        NbcAdjustTextFrameWidthAndHeight();
    }
    SetRectsDirty();
}

long SdrTextObj::GetRotateAngle() const
{
    return aGeo.nDrehWink;
}

long SdrTextObj::GetShearAngle(FASTBOOL bVertical) const
{
    return (aGeo.nDrehWink==0 || aGeo.nDrehWink==18000) ? aGeo.nShearWink : 0;
}

void SdrTextObj::NbcMove(const Size& rSiz)
{
    MoveRect(aRect,rSiz);
    MoveRect(aOutRect,rSiz);
    MoveRect(maSnapRect,rSiz);
    SetRectsDirty(TRUE);
}

void SdrTextObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    FASTBOOL bNoShearMerk=aGeo.nShearWink==0;
    FASTBOOL bRota90Merk=bNoShearMerk && aGeo.nDrehWink % 9000 ==0;
    long nHDist=GetTextLeftDistance()+GetTextRightDistance();
    long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
    long nTWdt0=aRect.GetWidth ()-1-nHDist; if (nTWdt0<0) nTWdt0=0;
    long nTHgt0=aRect.GetHeight()-1-nVDist; if (nTHgt0<0) nTHgt0=0;
    FASTBOOL bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
    FASTBOOL bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
    if (bXMirr || bYMirr) {
        Point aRef1(GetSnapRect().Center());
        if (bXMirr) {
            Point aRef2(aRef1);
            aRef2.Y()++;
            NbcMirrorGluePoints(aRef1,aRef2);
        }
        if (bYMirr) {
            Point aRef2(aRef1);
            aRef2.X()++;
            NbcMirrorGluePoints(aRef1,aRef2);
        }
    }

    if (aGeo.nDrehWink==0 && aGeo.nShearWink==0) {
        ResizeRect(aRect,rRef,xFact,yFact);
        if (bYMirr) {
            aRect.Justify();
            aRect.Move(aRect.Right()-aRect.Left(),aRect.Bottom()-aRect.Top());
            aGeo.nDrehWink=18000;
            aGeo.RecalcSinCos();
        }
    } else {
        Polygon aPol(Rect2Poly(aRect,aGeo));
        for (USHORT i=0; i<aPol.GetSize(); i++) {
             ResizePoint(aPol[i],rRef,xFact,yFact);
        }
        if (bXMirr!=bYMirr) {
            // Polygon wenden und etwas schieben
            Polygon aPol0(aPol);
            aPol[0]=aPol0[1];
            aPol[1]=aPol0[0];
            aPol[2]=aPol0[3];
            aPol[3]=aPol0[2];
            aPol[4]=aPol0[1];
        }
        Poly2Rect(aPol,aRect,aGeo);
    }

    if (bRota90Merk) {
        FASTBOOL bRota90=aGeo.nDrehWink % 9000 ==0;
        if (!bRota90) { // Scheinbar Rundungsfehler: Korregieren
            long a=NormAngle360(aGeo.nDrehWink);
            if (a<4500) a=0;
            else if (a<13500) a=9000;
            else if (a<22500) a=18000;
            else if (a<31500) a=27000;
            else a=0;
            aGeo.nDrehWink=a;
            aGeo.RecalcSinCos();
        }
        if (bNoShearMerk!=(aGeo.nShearWink==0)) { // Shear ggf. korregieren wg. Rundungsfehler
            aGeo.nShearWink=0;
            aGeo.RecalcTan();
        }
    }

    ImpJustifyRect(aRect);
    long nTWdt1=aRect.GetWidth ()-1-nHDist; if (nTWdt1<0) nTWdt1=0;
    long nTHgt1=aRect.GetHeight()-1-nVDist; if (nTHgt1<0) nTHgt1=0;
    if (bTextFrame && (pModel==NULL || !pModel->IsPasteResize())) { // #51139#
        if (nTWdt0!=nTWdt1 && IsAutoGrowWidth() ) NbcSetMinTextFrameWidth(nTWdt1);
        if (nTHgt0!=nTHgt1 && IsAutoGrowHeight()) NbcSetMinTextFrameHeight(nTHgt1);
        if (GetFitToSize()==SDRTEXTFIT_RESIZEATTR) {
            NbcResizeTextAttributes(Fraction(nTWdt1,nTWdt0),Fraction(nTHgt1,nTHgt0));
        }
        NbcAdjustTextFrameWidthAndHeight();
    }
    ImpCheckShear();
    SetRectsDirty();
}

void SdrTextObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    SetGlueReallyAbsolute(TRUE);
    long dx=aRect.Right()-aRect.Left();
    long dy=aRect.Bottom()-aRect.Top();
    Point aP(aRect.TopLeft());
    RotatePoint(aP,rRef,sn,cs);
    aRect.Left()=aP.X();
    aRect.Top()=aP.Y();
    aRect.Right()=aRect.Left()+dx;
    aRect.Bottom()=aRect.Top()+dy;
    if (aGeo.nDrehWink==0) {
        aGeo.nDrehWink=NormAngle360(nWink);
        aGeo.nSin=sn;
        aGeo.nCos=cs;
    } else {
        aGeo.nDrehWink=NormAngle360(aGeo.nDrehWink+nWink);
        aGeo.RecalcSinCos();
    }
    SetRectsDirty();
    NbcRotateGluePoints(rRef,nWink,sn,cs);
    SetGlueReallyAbsolute(FALSE);
}

void SdrTextObj::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    SetGlueReallyAbsolute(TRUE);

    // #75889# when this is a SdrPathObj aRect maybe not initialized
    Polygon aPol(Rect2Poly(aRect.IsEmpty() ? GetSnapRect() : aRect, aGeo));

    USHORT nPointCount=aPol.GetSize();
    for (USHORT i=0; i<nPointCount; i++) {
         ShearPoint(aPol[i],rRef,tn,bVShear);
    }
    Poly2Rect(aPol,aRect,aGeo);
    ImpJustifyRect(aRect);
    if (bTextFrame) {
        NbcAdjustTextFrameWidthAndHeight();
    }
    ImpCheckShear();
    SetRectsDirty();
    NbcShearGluePoints(rRef,nWink,tn,bVShear);
    SetGlueReallyAbsolute(FALSE);
}

void SdrTextObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(TRUE);
    FASTBOOL bNoShearMerk=aGeo.nShearWink==0;
    FASTBOOL bRota90Merk=FALSE;
    if (bNoShearMerk &&
        (rRef1.X()==rRef2.X() || rRef1.Y()==rRef2.Y() ||
         Abs(rRef1.X()-rRef2.X())==Abs(rRef1.Y()-rRef2.Y()))) {
        bRota90Merk=aGeo.nDrehWink % 9000 ==0;
    }
    Polygon aPol(Rect2Poly(aRect,aGeo));
    USHORT i;
    USHORT nPntAnz=aPol.GetSize();
    for (i=0; i<nPntAnz; i++) {
         MirrorPoint(aPol[i],rRef1,rRef2);
    }
    // Polygon wenden und etwas schieben
    Polygon aPol0(aPol);
    aPol[0]=aPol0[1];
    aPol[1]=aPol0[0];
    aPol[2]=aPol0[3];
    aPol[3]=aPol0[2];
    aPol[4]=aPol0[1];
    Poly2Rect(aPol,aRect,aGeo);

    if (bRota90Merk) {
        FASTBOOL bRota90=aGeo.nDrehWink % 9000 ==0;
        if (bRota90Merk && !bRota90) { // Scheinbar Rundungsfehler: Korregieren
            long a=NormAngle360(aGeo.nDrehWink);
            if (a<4500) a=0;
            else if (a<13500) a=9000;
            else if (a<22500) a=18000;
            else if (a<31500) a=27000;
            else a=0;
            aGeo.nDrehWink=a;
            aGeo.RecalcSinCos();
        }
    }
    if (bNoShearMerk!=(aGeo.nShearWink==0)) { // Shear ggf. korregieren wg. Rundungsfehler
        aGeo.nShearWink=0;
        aGeo.RecalcTan();
    }

    ImpJustifyRect(aRect);
    if (bTextFrame) {
        NbcAdjustTextFrameWidthAndHeight();
    }
    ImpCheckShear();
    SetRectsDirty();
    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(FALSE);
}

SdrObject* SdrTextObj::ImpConvertObj(FASTBOOL bToPoly) const
{
    if (!ImpCanConvTextToCurve()) return NULL;
    SdrObjGroup* pGroup=new SdrObjGroup();
    SdrOutliner& rOutl=ImpGetDrawOutliner();
    rOutl.SetUpdateMode(TRUE);
    ImpTextPortionHandler aConverter(rOutl,*this);

    aConverter.ConvertToPathObj(*pGroup,bToPoly);

    // Nachsehen, ob ueberhaupt was drin ist:
    SdrObjList* pOL=pGroup->GetSubList();

    if (pOL->GetObjCount()==0) {
        delete pGroup;
        return NULL;
    }
    // Ein einzelnes Objekt muss nicht gruppiert werden:
    if (pOL->GetObjCount()==1) {
        SdrObject* pObj=pOL->RemoveObject(0);
        delete pGroup;
        return pObj;
    }
    // Ansonsten die Gruppe zurueckgeben
    return pGroup;
}

SdrObject* SdrTextObj::DoConvertToPolyObj(BOOL bBezier) const
{
    return ImpConvertObj(!bBezier);
}

void SdrTextObj::ImpConvertSetAttrAndLayer(SdrObject* pObj, FASTBOOL bNoSetAttr) const
{
    if (pObj!=NULL) {
        pObj->ImpSetAnchorPos(aAnchor);
        pObj->NbcSetLayer(SdrLayerID(nLayerId));
        if (pModel!=NULL) {
            pObj->SetModel(pModel);
            if (!bNoSetAttr) {
//-/                SfxItemSet aItemSet((SfxItemPool&)(*GetItemPool()));
//-/                TakeAttributes(aItemSet,FALSE,TRUE);
//-/                pObj->SetAttributes(aItemSet,TRUE);
                SdrBroadcastItemChange aItemChange(*pObj);
                pObj->ClearItem();
                pObj->SetItemSet(GetItemSet());
                pObj->BroadcastItemChange(aItemChange);

                pObj->NbcSetStyleSheet(GetStyleSheet(),TRUE);
            }
        }
    }
}

SdrObject* SdrTextObj::ImpConvertMakeObj(const XPolyPolygon& rXPP, FASTBOOL bClosed, FASTBOOL bBezier, FASTBOOL bNoSetAttr) const
{
    SdrObjKind ePathKind=bClosed?OBJ_PATHFILL:OBJ_PATHLINE;
    XPolyPolygon aXPP(rXPP);
    if (bClosed) {
        // Alle XPolygone des XPolyPolygons schliessen, sonst kommt das PathObj durcheinander!
        for (USHORT i=0; i<aXPP.Count(); i++) {
            const XPolygon& rXP=aXPP[i];
            USHORT nAnz=rXP.GetPointCount();
            if (nAnz>0) {
                USHORT nMax=USHORT(nAnz-1);
                Point aPnt(rXP[0]);
                if (aPnt!=rXP[nMax]) {
                    aXPP[i].SetPointCount(nAnz+1);
                    aXPP[i][nAnz]=aPnt;
                }
            }
        }
    }
    if (!bBezier && pModel!=NULL) {
        // Polygon aus Bezierkurve interpolieren
        VirtualDevice   aVDev;
        XPolyPolygon    aXPolyPoly;

        MapMode aMap = aVDev.GetMapMode();
        aMap.SetMapUnit(pModel->GetScaleUnit());
        aMap.SetScaleX(pModel->GetScaleFraction());
        aMap.SetScaleY(pModel->GetScaleFraction());
        aVDev.SetMapMode(aMap);

        for (USHORT i=0; i<aXPP.Count(); i++)
            aXPolyPoly.Insert(XOutCreatePolygon(aXPP[i],&aVDev));
        aXPP=aXPolyPoly;
        ePathKind=bClosed?OBJ_POLY:OBJ_PLIN;
    }
    SdrPathObj* pPathObj=new SdrPathObj(ePathKind,aXPP);
    if (bBezier) {
        pPathObj->ConvertAllSegments(SDRPATH_CURVE);
    }
    ImpConvertSetAttrAndLayer(pPathObj,bNoSetAttr);
    return pPathObj;
}

SdrObject* SdrTextObj::ImpConvertAddText(SdrObject* pObj, FASTBOOL bBezier) const
{
    if (!ImpCanConvTextToCurve()) return pObj;
    SdrObject* pText=ImpConvertObj(!bBezier);
    if (pText==NULL) return pObj;
    if (pObj==NULL) return pText;
    if (pText->IsGroupObject()) {
        SdrObjList* pOL=pText->GetSubList();
        pOL->InsertObject(pObj,0);
        return pText;
    } else {
        SdrObjGroup* pGrp=new SdrObjGroup;
        SdrObjList* pOL=pGrp->GetSubList();
        pOL->InsertObject(pObj);
        pOL->InsertObject(pText);
        return pGrp;
    }
}

