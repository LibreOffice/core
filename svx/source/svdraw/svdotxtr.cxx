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
#include <sdr/properties/itemsettools.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svl/itemset.hxx>
#include <svx/svditer.hxx>
#include <drawinglayer/processor2d/textaspolygonextractor2d.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>

using namespace com::sun::star;

void SdrTextObj::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    if (aGeo.nRotationAngle!=0 || aGeo.nShearAngle!=0)
    {
        // Either the rotation or shear angle exists.
        tools::Rectangle aSR0(GetSnapRect());
        long nWdt0=aSR0.Right()-aSR0.Left();
        long nHgt0=aSR0.Bottom()-aSR0.Top();
        long nWdt1=rRect.Right()-rRect.Left();
        long nHgt1=rRect.Bottom()-rRect.Top();
        SdrTextObj::NbcResize(maSnapRect.TopLeft(),Fraction(nWdt1,nWdt0),Fraction(nHgt1,nHgt0));
        SdrTextObj::NbcMove(Size(rRect.Left()-aSR0.Left(),rRect.Top()-aSR0.Top()));
    }
    else
    {
        // No rotation or shear.

        maRect = rRect;
        ImpJustifyRect(maRect);

        AdaptTextMinSize();

        ImpCheckShear();
        SetRectsDirty();
    }
}

const tools::Rectangle& SdrTextObj::GetLogicRect() const
{
    return maRect;
}

void SdrTextObj::NbcSetLogicRect(const tools::Rectangle& rRect)
{
    maRect = rRect;
    ImpJustifyRect(maRect);

    AdaptTextMinSize();

    SetRectsDirty();
}

long SdrTextObj::GetRotateAngle() const
{
    return aGeo.nRotationAngle;
}

long SdrTextObj::GetShearAngle(bool /*bVertical*/) const
{
    return aGeo.nShearAngle;
}

void SdrTextObj::NbcMove(const Size& rSiz)
{
    MoveRect(maRect,rSiz);
    MoveRect(aOutRect,rSiz);
    MoveRect(maSnapRect,rSiz);
    SetRectsDirty(true);
}

void SdrTextObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    bool bNoShearMerk=aGeo.nShearAngle==0;
    bool bRota90Merk=bNoShearMerk && aGeo.nRotationAngle % 9000 ==0;
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

    if (aGeo.nRotationAngle==0 && aGeo.nShearAngle==0) {
        ResizeRect(maRect,rRef,xFact,yFact);
        if (bYMirr) {
            maRect.Justify();
            maRect.Move(maRect.Right()-maRect.Left(),maRect.Bottom()-maRect.Top());
            aGeo.nRotationAngle=18000;
            aGeo.RecalcSinCos();
        }
    }
    else
    {
        tools::Polygon aPol(Rect2Poly(maRect,aGeo));

        for(sal_uInt16 a(0); a < aPol.GetSize(); a++)
        {
             ResizePoint(aPol[a], rRef, xFact, yFact);
        }

        if(bXMirr != bYMirr)
        {
            // turn polygon and move it a little
            tools::Polygon aPol0(aPol);

            aPol[0] = aPol0[1];
            aPol[1] = aPol0[0];
            aPol[2] = aPol0[3];
            aPol[3] = aPol0[2];
            aPol[4] = aPol0[1];
        }

        Poly2Rect(aPol, maRect, aGeo);
    }

    if (bRota90Merk) {
        bool bRota90=aGeo.nRotationAngle % 9000 ==0;
        if (!bRota90) { // there's seems to be a rounding error occurring: correct it
            long a=NormAngle360(aGeo.nRotationAngle);
            if (a<4500) a=0;
            else if (a<13500) a=9000;
            else if (a<22500) a=18000;
            else if (a<31500) a=27000;
            else a=0;
            aGeo.nRotationAngle=a;
            aGeo.RecalcSinCos();
        }
        if (bNoShearMerk!=(aGeo.nShearAngle==0)) { // correct a rounding error occurring with Shear
            aGeo.nShearAngle=0;
            aGeo.RecalcTan();
        }
    }

    ImpJustifyRect(maRect);

    AdaptTextMinSize();

    if(bTextFrame && (!pModel || !pModel->IsPasteResize()))
    {
        NbcAdjustTextFrameWidthAndHeight();
    }

    ImpCheckShear();
    SetRectsDirty();
}

void SdrTextObj::NbcRotate(const Point& rRef, long nAngle, double sn, double cs)
{
    SetGlueReallyAbsolute(true);
    long dx=maRect.Right()-maRect.Left();
    long dy=maRect.Bottom()-maRect.Top();
    Point aP(maRect.TopLeft());
    RotatePoint(aP,rRef,sn,cs);
    maRect.Left()=aP.X();
    maRect.Top()=aP.Y();
    maRect.Right()=maRect.Left()+dx;
    maRect.Bottom()=maRect.Top()+dy;
    if (aGeo.nRotationAngle==0) {
        aGeo.nRotationAngle=NormAngle360(nAngle);
        aGeo.nSin=sn;
        aGeo.nCos=cs;
    } else {
        aGeo.nRotationAngle=NormAngle360(aGeo.nRotationAngle+nAngle);
        aGeo.RecalcSinCos();
    }
    SetRectsDirty();
    NbcRotateGluePoints(rRef,nAngle,sn,cs);
    SetGlueReallyAbsolute(false);
}

void SdrTextObj::NbcShear(const Point& rRef, long /*nAngle*/, double tn, bool bVShear)
{
    SetGlueReallyAbsolute(true);

    // when this is a SdrPathObj, aRect may be uninitialized
    tools::Polygon aPol(Rect2Poly(maRect.IsEmpty() ? GetSnapRect() : maRect, aGeo));

    sal_uInt16 nPointCount=aPol.GetSize();
    for (sal_uInt16 i=0; i<nPointCount; i++) {
         ShearPoint(aPol[i],rRef,tn,bVShear);
    }
    Poly2Rect(aPol,maRect,aGeo);
    ImpJustifyRect(maRect);
    if (bTextFrame) {
        NbcAdjustTextFrameWidthAndHeight();
    }
    ImpCheckShear();
    SetRectsDirty();
    NbcShearGluePoints(rRef,tn,bVShear);
    SetGlueReallyAbsolute(false);
}

void SdrTextObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(true);
    bool bNoShearMerk=aGeo.nShearAngle==0;
    bool bRota90Merk = false;
    if (bNoShearMerk &&
        (rRef1.X()==rRef2.X() || rRef1.Y()==rRef2.Y() ||
         std::abs(rRef1.X()-rRef2.X())==std::abs(rRef1.Y()-rRef2.Y()))) {
        bRota90Merk=aGeo.nRotationAngle % 9000 ==0;
    }
    tools::Polygon aPol(Rect2Poly(maRect,aGeo));
    sal_uInt16 i;
    sal_uInt16 nPointCount=aPol.GetSize();
    for (i=0; i<nPointCount; i++) {
         MirrorPoint(aPol[i],rRef1,rRef2);
    }
    // turn polygon and move it a little
    tools::Polygon aPol0(aPol);
    aPol[0]=aPol0[1];
    aPol[1]=aPol0[0];
    aPol[2]=aPol0[3];
    aPol[3]=aPol0[2];
    aPol[4]=aPol0[1];
    Poly2Rect(aPol,maRect,aGeo);

    if (bRota90Merk) {
        bool bRota90=aGeo.nRotationAngle % 9000 ==0;
        if (bRota90Merk && !bRota90) { // there's seems to be a rounding error occurring: correct it
            long a=NormAngle360(aGeo.nRotationAngle);
            if (a<4500) a=0;
            else if (a<13500) a=9000;
            else if (a<22500) a=18000;
            else if (a<31500) a=27000;
            else a=0;
            aGeo.nRotationAngle=a;
            aGeo.RecalcSinCos();
        }
    }
    if (bNoShearMerk!=(aGeo.nShearAngle==0)) { // correct a rounding error occurring with Shear
        aGeo.nShearAngle=0;
        aGeo.RecalcTan();
    }

    ImpJustifyRect(maRect);
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
    SdrObject* pRetval = nullptr;

    if(!ImpCanConvTextToCurve())
    {
        // suppress HelpTexts from PresObj's
        return nullptr;
    }

    // get primitives
    const drawinglayer::primitive2d::Primitive2DContainer xSequence(GetViewContact().getViewIndependentPrimitive2DContainer());

    if(!xSequence.empty())
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
                            aPolyPolygon = basegfx::utils::adaptiveSubdivideByAngle(aPolyPolygon);
                        }
                    }
                    else
                    {
                        if(!aPolyPolygon.areControlPointsUsed())
                        {
                            aPolyPolygon = basegfx::utils::expandToCurve(aPolyPolygon);
                        }
                    }

                    // create ItemSet with object attributes
                    SfxItemSet aAttributeSet(GetObjectItemSet());
                    SdrPathObj* pPathObj = nullptr;

                    // always clear objectshadow; this is included in the extraction
                    aAttributeSet.Put(makeSdrShadowItem(false));

                    if(rCandidate.getIsFilled())
                    {
                        // set needed items
                        aAttributeSet.Put(XFillColorItem(OUString(), Color(rCandidate.getBColor())));
                        aAttributeSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
                        aAttributeSet.Put(XFillStyleItem(drawing::FillStyle_SOLID));

                        // create filled SdrPathObj
                        pPathObj = new SdrPathObj(OBJ_PATHFILL, aPolyPolygon);
                    }
                    else
                    {
                        // set needed items
                        aAttributeSet.Put(XLineColorItem(OUString(), Color(rCandidate.getBColor())));
                        aAttributeSet.Put(XLineStyleItem(drawing::LineStyle_SOLID));
                        aAttributeSet.Put(XLineWidthItem(0));
                        aAttributeSet.Put(XFillStyleItem(drawing::FillStyle_NONE));

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

    return nullptr;
}

bool SdrTextObj::ImpCanConvTextToCurve() const
{
    return !IsOutlText();
}

SdrObject* SdrTextObj::ImpConvertMakeObj(const basegfx::B2DPolyPolygon& rPolyPolygon, bool bClosed, bool bBezier) const
{
    SdrObjKind ePathKind = bClosed ? OBJ_PATHFILL : OBJ_PATHLINE;
    basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPolygon);

    // #i37011#
    if(!bBezier)
    {
        aB2DPolyPolygon = basegfx::utils::adaptiveSubdivideByAngle(aB2DPolyPolygon);
        ePathKind = bClosed ? OBJ_POLY : OBJ_PLIN;
    }

    SdrPathObj* pPathObj = new SdrPathObj(ePathKind, aB2DPolyPolygon);

    if(bBezier)
    {
        // create bezier curves
        pPathObj->SetPathPoly(basegfx::utils::expandToCurve(pPathObj->GetPathPoly()));
    }

    pPathObj->ImpSetAnchorPos(aAnchor);
    pPathObj->NbcSetLayer(GetLayer());

    if(pModel)
    {
        pPathObj->SetModel(pModel);

        sdr::properties::ItemChangeBroadcaster aC(*pPathObj);

        pPathObj->ClearMergedItem();
        pPathObj->SetMergedItemSet(GetObjectItemSet());
        pPathObj->GetProperties().BroadcastItemChange(aC);
        pPathObj->NbcSetStyleSheet(GetStyleSheet(), true);
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
