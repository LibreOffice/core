/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdotxtr.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:55:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdotext.hxx>
#include "svditext.hxx"
#include <svx/svdtrans.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdoutl.hxx>
#include "svdtxhdl.hxx"  // DrawTextToPath fuer Convert
#include <svx/svdpage.hxx>   // fuer Convert
#include <svx/svdmodel.hxx>  // fuer Convert

#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif

#ifndef _SDR_PROPERTIES_ITEMSETTOOLS_HXX
#include <svx/sdr/properties/itemsettools.hxx>
#endif

#ifndef _SDR_PROPERTIES_PROPERTIES_HXX
#include <svx/sdr/properties/properties.hxx>
#endif

// #i37011#
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
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

long SdrTextObj::GetShearAngle(FASTBOOL /*bVertical*/) const
{
    return aGeo.nShearWink;
}

void SdrTextObj::NbcMove(const Size& rSiz)
{
    MoveRect(aRect,rSiz);
    MoveRect(aOutRect,rSiz);
    MoveRect(maSnapRect,rSiz);
    SetRectsDirty(sal_True);
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
    }
    else
    {
        // #100663# aRect is NOT initialized for lines (polgon objects with two
        // exceptionally handled points). Thus, after this call the text rotaion is
        // gone. This error must be present since day one of this old drawing layer.
        // It's astonishing that noone discovered it earlier.
        // Polygon aPol(Rect2Poly(aRect,aGeo));
        // Polygon aPol(Rect2Poly(GetSnapRect(), aGeo));

        // #101412# go back to old method, side effects are impossible
        // to calculate.
        Polygon aPol(Rect2Poly(aRect,aGeo));

        for(sal_uInt16 a(0); a < aPol.GetSize(); a++)
        {
             ResizePoint(aPol[a], rRef, xFact, yFact);
        }

        if(bXMirr != bYMirr)
        {
            // Polygon wenden und etwas schieben
            Polygon aPol0(aPol);

            aPol[0] = aPol0[1];
            aPol[1] = aPol0[0];
            aPol[2] = aPol0[3];
            aPol[3] = aPol0[2];
            aPol[4] = aPol0[1];
        }

        Poly2Rect(aPol, aRect, aGeo);
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

    boost::shared_ptr< SdrOutliner > xOutl( const_cast< SdrTextObj* >(this)->CreateDrawOutliner() );
    xOutl->SetUpdateMode(TRUE);
    ImpTextPortionHandler aConverter(*(xOutl.get()),*this);

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

SdrObject* SdrTextObj::ImpConvertMakeObj(const basegfx::B2DPolyPolygon& rPolyPolygon, sal_Bool bClosed, sal_Bool bBezier, sal_Bool bNoSetAttr) const
{
    SdrObjKind ePathKind = bClosed ? OBJ_PATHFILL : OBJ_PATHLINE;
    basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPolygon);

    // #i37011#
    if(!bBezier)
    {
        aB2DPolyPolygon = basegfx::tools::adaptiveSubdivideByAngle(aB2DPolyPolygon);
        ePathKind = bClosed ? OBJ_POLY : OBJ_PLIN;
    }

    SdrPathObj* pPathObj = new SdrPathObj(ePathKind, aB2DPolyPolygon);

    if(bBezier)
    {
        // create bezier curves
        pPathObj->SetPathPoly(basegfx::tools::expandToCurve(pPathObj->GetPathPoly()));
    }

    {
        if(pPathObj)
        {
            pPathObj->ImpSetAnchorPos(aAnchor);
            pPathObj->NbcSetLayer(SdrLayerID(GetLayer()));

            if(pModel)
            {
                pPathObj->SetModel(pModel);

                if(!bNoSetAttr)
                {
                    sdr::properties::ItemChangeBroadcaster aC(*pPathObj);

                    pPathObj->ClearMergedItem();
                    pPathObj->SetMergedItemSet(GetObjectItemSet());
                    pPathObj->GetProperties().BroadcastItemChange(aC);
                    pPathObj->NbcSetStyleSheet(GetStyleSheet(), sal_True);
                }
            }
        }
    }

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

