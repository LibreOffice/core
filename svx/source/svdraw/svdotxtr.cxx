/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svx/svdotext.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/editdata.hxx>
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

        // #115391#
        AdaptTextMinSize();

        if (bTextFrame && (pModel==NULL || !pModel->IsPasteResize()))
        {
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

    // #115391#
    AdaptTextMinSize();

    if(bTextFrame)
    {
        NbcAdjustTextFrameWidthAndHeight();
    }

    SetRectsDirty();
}

long SdrTextObj::GetRotateAngle() const
{
    return aGeo.nDrehWink;
}

long SdrTextObj::GetShearAngle(bool /*bVertical*/) const
{
    return aGeo.nShearWink;
}

void SdrTextObj::NbcMove(const Size& rSiz)
{
    MoveRect(aRect,rSiz);
    MoveRect(aOutRect,rSiz);
    MoveRect(maSnapRect,rSiz);
    SetRectsDirty(true);
}

void SdrTextObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    bool bNoShearMerk=aGeo.nShearWink==0;
    bool bRota90Merk=bNoShearMerk && aGeo.nDrehWink % 9000 ==0;
    long nHDist=GetTextLeftDistance()+GetTextRightDistance();
    long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
    long nTWdt0=aRect.GetWidth ()-1-nHDist; if (nTWdt0<0) nTWdt0=0;
    long nTHgt0=aRect.GetHeight()-1-nVDist; if (nTHgt0<0) nTHgt0=0;
    bool bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
    bool bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
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
        Polygon aPol(Rect2Poly(aRect,aGeo));

        for(sal_uInt16 a(0); a < aPol.GetSize(); a++)
        {
             ResizePoint(aPol[a], rRef, xFact, yFact);
        }

        if(bXMirr != bYMirr)
        {
            // turn polygon and move it a little
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
        bool bRota90=aGeo.nDrehWink % 9000 ==0;
        if (!bRota90) { // there's seems to be a rounding error occurring: correct it
            long a=NormAngle360(aGeo.nDrehWink);
            if (a<4500) a=0;
            else if (a<13500) a=9000;
            else if (a<22500) a=18000;
            else if (a<31500) a=27000;
            else a=0;
            aGeo.nDrehWink=a;
            aGeo.RecalcSinCos();
        }
        if (bNoShearMerk!=(aGeo.nShearWink==0)) { // correct a rounding error occurring with Shear
            aGeo.nShearWink=0;
            aGeo.RecalcTan();
        }
    }

    ImpJustifyRect(aRect);

    long nTWdt1=aRect.GetWidth ()-1-nHDist; if (nTWdt1<0) nTWdt1=0;
    long nTHgt1=aRect.GetHeight()-1-nVDist; if (nTHgt1<0) nTHgt1=0;

    // #115391#
    AdaptTextMinSize();

    if(bTextFrame && (!pModel || !pModel->IsPasteResize()))
    {
        NbcAdjustTextFrameWidthAndHeight();
    }

    ImpCheckShear();
    SetRectsDirty();
}

void SdrTextObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    SetGlueReallyAbsolute(true);
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
    SetGlueReallyAbsolute(false);
}

void SdrTextObj::NbcShear(const Point& rRef, long nWink, double tn, bool bVShear)
{
    SetGlueReallyAbsolute(true);

    // when this is a SdrPathObj, aRect may be uninitialized
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
    SetGlueReallyAbsolute(false);
}

void SdrTextObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(true);
    bool bNoShearMerk=aGeo.nShearWink==0;
    bool bRota90Merk = false;
    if (bNoShearMerk &&
        (rRef1.X()==rRef2.X() || rRef1.Y()==rRef2.Y() ||
         std::abs(rRef1.X()-rRef2.X())==std::abs(rRef1.Y()-rRef2.Y()))) {
        bRota90Merk=aGeo.nDrehWink % 9000 ==0;
    }
    Polygon aPol(Rect2Poly(aRect,aGeo));
    sal_uInt16 i;
    sal_uInt16 nPntAnz=aPol.GetSize();
    for (i=0; i<nPntAnz; i++) {
         MirrorPoint(aPol[i],rRef1,rRef2);
    }
    // turn polygon and move it a little
    Polygon aPol0(aPol);
    aPol[0]=aPol0[1];
    aPol[1]=aPol0[0];
    aPol[2]=aPol0[3];
    aPol[3]=aPol0[2];
    aPol[4]=aPol0[1];
    Poly2Rect(aPol,aRect,aGeo);

    if (bRota90Merk) {
        bool bRota90=aGeo.nDrehWink % 9000 ==0;
        if (bRota90Merk && !bRota90) { // there's seems to be a rounding error occurring: correct it
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
    if (bNoShearMerk!=(aGeo.nShearWink==0)) { // correct a rounding error occurring with Shear
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
    SetGlueReallyAbsolute(false);
}



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
                        aAttributeSet.Put(XFillColorItem(OUString(), Color(rCandidate.getBColor())));
                        aAttributeSet.Put(XLineStyleItem(XLINE_NONE));
                        aAttributeSet.Put(XFillStyleItem(XFILL_SOLID));

                        // create filled SdrPathObj
                        pPathObj = new SdrPathObj(OBJ_PATHFILL, aPolyPolygon);
                    }
                    else
                    {
                        // set needed items
                        aAttributeSet.Put(XLineColorItem(OUString(), Color(rCandidate.getBColor())));
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



SdrObject* SdrTextObj::DoConvertToPolyObj(bool bBezier, bool bAddText) const
{
    if(bAddText)
    {
        return ImpConvertContainedTextToSdrPathObjs(!bBezier);
    }

    return 0;
}

bool SdrTextObj::ImpCanConvTextToCurve() const
{
    return !IsOutlText();
}

SdrObject* SdrTextObj::ImpConvertMakeObj(const basegfx::B2DPolyPolygon& rPolyPolygon, bool bClosed, bool bBezier, bool bNoSetAttr) const
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
                pPathObj->NbcSetStyleSheet(GetStyleSheet(), true);
            }
        }
    }

    return pPathObj;
}

SdrObject* SdrTextObj::ImpConvertAddText(SdrObject* pObj, bool bBezier) const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
