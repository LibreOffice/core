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

#include <svx/svdotext.hxx>
#include "svx/svditext.hxx"
#include <svx/svdtrans.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>   // fuer Convert
#include <svx/svdmodel.hxx>  // fuer Convert
#include <editeng/outliner.hxx>
#include <svx/sdr/properties/itemsettools.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svl/itemset.hxx>
#include <svx/svditer.hxx>
#include <drawinglayer/processor2d/textaspolygonextractor2d.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>

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
    SetGlueReallyAbsolute(sal_True);
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
    SetGlueReallyAbsolute(sal_False);
}

void SdrTextObj::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    SetGlueReallyAbsolute(sal_True);

    // #75889# when this is a SdrPathObj aRect maybe not initialized
    Polygon aPol(Rect2Poly(aRect.IsEmpty() ? GetSnapRect() : aRect, aGeo));

    sal_uInt16 nPointCount=aPol.GetSize();
    for (sal_uInt16 i=0; i<nPointCount; i++) {
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
    SetGlueReallyAbsolute(sal_False);
}

void SdrTextObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(sal_True);
    FASTBOOL bNoShearMerk=aGeo.nShearWink==0;
    FASTBOOL bRota90Merk=sal_False;
    if (bNoShearMerk &&
        (rRef1.X()==rRef2.X() || rRef1.Y()==rRef2.Y() ||
         Abs(rRef1.X()-rRef2.X())==Abs(rRef1.Y()-rRef2.Y()))) {
        bRota90Merk=aGeo.nDrehWink % 9000 ==0;
    }
    Polygon aPol(Rect2Poly(aRect,aGeo));
    sal_uInt16 i;
    sal_uInt16 nPntAnz=aPol.GetSize();
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
    SetGlueReallyAbsolute(sal_False);
}

//////////////////////////////////////////////////////////////////////////////

SdrObject* SdrTextObj::ImpConvertContainedTextToSdrPathObjs(bool bToPoly) const
{
    SdrObject* pRetval = 0;

    if(!ImpCanConvTextToCurve())
    {
        // suppress HelpTexts from PresObj's
        return 0;
    }

    // get primitives
    const drawinglayer::primitive2d::Primitive2DSequence xSequence(GetViewContact().getViewIndependentPrimitive2DSequence());

    if(xSequence.hasElements())
    {
        // create an extractor with neutral ViewInformation
        const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
        drawinglayer::processor2d::TextAsPolygonExtractor2D aExtractor(aViewInformation2D);

        // extract text as polygons
        aExtractor.process(xSequence);

        // get results
        const drawinglayer::processor2d::TextAsPolygonDataNodeVector& rResult = aExtractor.getTarget();
        const sal_uInt32 nResultCount(rResult.size());

        if(nResultCount)
        {
            // prepare own target
            SdrObjGroup* pGroup = new SdrObjGroup();
            SdrObjList* pObjectList = pGroup->GetSubList();

            // process results
            for(sal_uInt32 a(0); a < nResultCount; a++)
            {
                const drawinglayer::processor2d::TextAsPolygonDataNode& rCandidate = rResult[a];
                basegfx::B2DPolyPolygon aPolyPolygon(rCandidate.getB2DPolyPolygon());

                if(aPolyPolygon.count())
                {
                    // take care of wanted polygon type
                    if(bToPoly)
                    {
                        if(aPolyPolygon.areControlPointsUsed())
                        {
                            aPolyPolygon = basegfx::tools::adaptiveSubdivideByAngle(aPolyPolygon);
                        }
                    }
                    else
                    {
                        if(!aPolyPolygon.areControlPointsUsed())
                        {
                            aPolyPolygon = basegfx::tools::expandToCurve(aPolyPolygon);
                        }
                    }

                    // create ItemSet with object attributes
                    SfxItemSet aAttributeSet(GetObjectItemSet());
                    SdrPathObj* pPathObj = 0;

                    // always clear objectshadow; this is included in the extraction
                    aAttributeSet.Put(SdrShadowItem(false));

                    if(rCandidate.getIsFilled())
                    {
                        // set needed items
                        aAttributeSet.Put(XFillColorItem(String(), Color(rCandidate.getBColor())));
                        aAttributeSet.Put(XLineStyleItem(XLINE_NONE));
                        aAttributeSet.Put(XFillStyleItem(XFILL_SOLID));

                        // create filled SdrPathObj
                        pPathObj = new SdrPathObj(OBJ_PATHFILL, aPolyPolygon);
                    }
                    else
                    {
                        // set needed items
                        aAttributeSet.Put(XLineColorItem(String(), Color(rCandidate.getBColor())));
                        aAttributeSet.Put(XLineStyleItem(XLINE_SOLID));
                        aAttributeSet.Put(XLineWidthItem(0));
                        aAttributeSet.Put(XFillStyleItem(XFILL_NONE));

                        // create line SdrPathObj
                        pPathObj = new SdrPathObj(OBJ_PATHLINE, aPolyPolygon);
                    }

                    // copy basic information from original
                    pPathObj->ImpSetAnchorPos(GetAnchorPos());
                    pPathObj->NbcSetLayer(GetLayer());

                    if(GetModel())
                    {
                        pPathObj->SetModel(GetModel());
                        pPathObj->NbcSetStyleSheet(GetStyleSheet(), true);
                    }

                    // apply prepared ItemSet and add to target
                    pPathObj->SetMergedItemSet(aAttributeSet);
                    pObjectList->InsertObject(pPathObj);
                }
            }

            // postprocess; if no result and/or only one object, simplify
            if(!pObjectList->GetObjCount())
            {
                delete pGroup;
            }
            else if(1 == pObjectList->GetObjCount())
            {
                pRetval = pObjectList->RemoveObject(0);
                delete pGroup;
            }
            else
            {
                pRetval = pGroup;
            }
        }
    }

    return pRetval;
}

//////////////////////////////////////////////////////////////////////////////

SdrObject* SdrTextObj::DoConvertToPolyObj(sal_Bool bBezier) const
{
    return ImpConvertContainedTextToSdrPathObjs(!bBezier);
}

bool SdrTextObj::ImpCanConvTextToCurve() const
{
    return !IsOutlText();
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

    return pPathObj;
}

SdrObject* SdrTextObj::ImpConvertAddText(SdrObject* pObj, FASTBOOL bBezier) const
{
    if(!ImpCanConvTextToCurve())
    {
        return pObj;
    }

    SdrObject* pText = ImpConvertContainedTextToSdrPathObjs(!bBezier);

    if(!pText)
    {
        return pObj;
    }

    if(!pObj)
    {
        return pText;
    }

    if(pText->IsGroupObject())
    {
        // is already group object, add partial shape in front
        SdrObjList* pOL=pText->GetSubList();
        pOL->InsertObject(pObj,0);

        return pText;
    }
    else
    {
        // not yet a group, create one and add partial and new shapes
        SdrObjGroup* pGrp=new SdrObjGroup;
        SdrObjList* pOL=pGrp->GetSubList();
        pOL->InsertObject(pObj);
        pOL->InsertObject(pText);

        return pGrp;
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof
