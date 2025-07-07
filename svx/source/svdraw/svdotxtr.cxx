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
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <sdr/properties/itemsettools.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svl/itemset.hxx>
#include <drawinglayer/processor2d/textaspolygonextractor2d.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/sdshitm.hxx>
#include <comphelper/configuration.hxx>

using namespace com::sun::star;

void SdrTextObj::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    if (maGeo.m_nRotationAngle || maGeo.m_nShearAngle)
    {
        // Either the rotation or shear angle exists.
        tools::Rectangle aSR0(GetSnapRect());
        tools::Long nWdt0=aSR0.Right()-aSR0.Left();
        tools::Long nHgt0=aSR0.Bottom()-aSR0.Top();
        tools::Long nWdt1=rRect.Right()-rRect.Left();
        tools::Long nHgt1=rRect.Bottom()-rRect.Top();
        SdrTextObj::NbcResize(maSnapRect.TopLeft(),Fraction(nWdt1,nWdt0),Fraction(nHgt1,nHgt0));
        SdrTextObj::NbcMove(Size(rRect.Left()-aSR0.Left(),rRect.Top()-aSR0.Top()));
    }
    else
    {
        // No rotation or shear.
        tools::Rectangle aRectangle(rRect);
        ImpJustifyRect(aRectangle);
        setRectangle(aRectangle);

        AdaptTextMinSize();

        ImpCheckShear();
        SetBoundAndSnapRectsDirty();
    }
}

const tools::Rectangle& SdrTextObj::GetLogicRect() const
{
    return getRectangle();
}

void SdrTextObj::NbcSetLogicRect(const tools::Rectangle& rRect, bool bAdaptTextMinSize)
{
    tools::Rectangle aRectangle(rRect);
    ImpJustifyRect(aRectangle);
    setRectangle(aRectangle);

    if (bAdaptTextMinSize)
        AdaptTextMinSize();

    SetBoundAndSnapRectsDirty();
}

Degree100 SdrTextObj::GetRotateAngle() const
{
    return maGeo.m_nRotationAngle;
}

Degree100 SdrTextObj::GetShearAngle(bool /*bVertical*/) const
{
    return maGeo.m_nShearAngle;
}

void SdrTextObj::NbcMove(const Size& rSize)
{
    moveRectangle(rSize.Width(), rSize.Height());
    moveOutRectangle(rSize.Width(), rSize.Height());
    maSnapRect.Move(rSize);
    SetBoundAndSnapRectsDirty(true);
}

void SdrTextObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    bool bNotSheared=maGeo.m_nShearAngle==0_deg100;
    bool bRotate90=bNotSheared && maGeo.m_nRotationAngle.get() % 9000 ==0;
    bool bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
    bool bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
    if (bXMirr || bYMirr) {
        Point aRef1(GetSnapRect().Center());
        if (bXMirr) {
            Point aRef2(aRef1);
            aRef2.AdjustY( 1 );
            NbcMirrorGluePoints(aRef1,aRef2);
        }
        if (bYMirr) {
            Point aRef2(aRef1);
            aRef2.AdjustX( 1 );
            NbcMirrorGluePoints(aRef1,aRef2);
        }
    }

    if (maGeo.m_nRotationAngle==0_deg100 && maGeo.m_nShearAngle==0_deg100) {
        auto aRectangle = getRectangle();
        ResizeRect(aRectangle, rRef, xFact, yFact);
        setRectangle(aRectangle);
        if (bYMirr)
        {
            moveRectangle(aRectangle.Right() - aRectangle.Left(), aRectangle.Bottom() - aRectangle.Top());
            maGeo.m_nRotationAngle=18000_deg100;
            maGeo.RecalcSinCos();
        }
    }
    else
    {
        tools::Polygon aPol(Rect2Poly(getRectangle(), maGeo));

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
        tools::Rectangle aRectangle = svx::polygonToRectangle(aPol, maGeo);
        setRectangle(aRectangle);
    }

    if (bRotate90) {
        bool bRota90=maGeo.m_nRotationAngle.get() % 9000 ==0;
        if (!bRota90) { // there's seems to be a rounding error occurring: correct it
            Degree100 a=NormAngle36000(maGeo.m_nRotationAngle);
            if (a<4500_deg100) a=0_deg100;
            else if (a<13500_deg100) a=9000_deg100;
            else if (a<22500_deg100) a=18000_deg100;
            else if (a<31500_deg100) a=27000_deg100;
            else a=0_deg100;
            maGeo.m_nRotationAngle=a;
            maGeo.RecalcSinCos();
        }
        if (bNotSheared!=(maGeo.m_nShearAngle==0_deg100)) { // correct a rounding error occurring with Shear
            maGeo.m_nShearAngle=0_deg100;
            maGeo.RecalcTan();
        }
    }

    AdaptTextMinSize();

    if(mbTextFrame && !getSdrModelFromSdrObject().IsPasteResize())
    {
        NbcAdjustTextFrameWidthAndHeight();
    }

    ImpCheckShear();
    SetBoundAndSnapRectsDirty();
}

namespace
{

gfx::Tuple2DL createTupleFromPoint(Point const& rPoint, gfx::LengthUnit eUnit = gfx::LengthUnit::hmm)
{
    auto x = gfx::Length::from(eUnit, rPoint.X());
    auto y = gfx::Length::from(eUnit, rPoint.Y());
    return gfx::Tuple2DL(x, y);
}

} // end anonymous


void SdrTextObj::NbcRotate(const Point& rRef, Degree100 nAngle, double sn, double cs)
{
    SetGlueReallyAbsolute(true);
    gfx::Tuple2DL aReference = createTupleFromPoint(rRef, getSdrModelFromSdrObject().getUnit());

    gfx::Length aWidth = maRectangleRange.getWidth();
    gfx::Length aHeight = maRectangleRange.getHeight();

    gfx::Tuple2DL aPoint(maRectangleRange.getMinX(), maRectangleRange.getMinY());
    gfx::Tuple2DL aRotated = svx::rotatePoint(aPoint, aReference, sn, cs);

    maRectangleRange = gfx::Range2DLWrap(
        aRotated.getX(),
        aRotated.getY(),
        aRotated.getX() + aWidth,
        aRotated.getY() + aHeight);

    if (maGeo.m_nRotationAngle == 0_deg100)
    {
        maGeo.m_nRotationAngle = NormAngle36000(nAngle);
        maGeo.mfSinRotationAngle = sn;
        maGeo.mfCosRotationAngle = cs;
    }
    else
    {
        maGeo.m_nRotationAngle = NormAngle36000(maGeo.m_nRotationAngle + nAngle);
        maGeo.RecalcSinCos();
    }

    SetBoundAndSnapRectsDirty();
    NbcRotateGluePoints(rRef, nAngle, sn, cs);
    SetGlueReallyAbsolute(false);
}

void SdrTextObj::NbcShear(const Point& rRef, Degree100 /*nAngle*/, double tn, bool bVShear)
{
    SetGlueReallyAbsolute(true);

    auto aRectangle = getRectangle();
    // when this is a SdrPathObj, aRect may be uninitialized
    tools::Polygon aPol(Rect2Poly(aRectangle.IsEmpty() ? GetSnapRect() : aRectangle, maGeo));

    sal_uInt16 nPointCount=aPol.GetSize();
    for (sal_uInt16 i=0; i<nPointCount; i++) {
         ShearPoint(aPol[i],rRef,tn,bVShear);
    }
    aRectangle = svx::polygonToRectangle(aPol, maGeo);
    ImpJustifyRect(aRectangle);
    setRectangle(aRectangle);

    if (mbTextFrame) {
        NbcAdjustTextFrameWidthAndHeight();
    }
    ImpCheckShear();
    SetBoundAndSnapRectsDirty();
    NbcShearGluePoints(rRef,tn,bVShear);
    SetGlueReallyAbsolute(false);
}

void SdrTextObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(true);
    bool bNotSheared=maGeo.m_nShearAngle==0_deg100;
    bool bRotate90 = false;
    if (bNotSheared &&
        (rRef1.X()==rRef2.X() || rRef1.Y()==rRef2.Y() ||
         std::abs(rRef1.X()-rRef2.X())==std::abs(rRef1.Y()-rRef2.Y()))) {
        bRotate90=maGeo.m_nRotationAngle.get() % 9000 ==0;
    }
    tools::Polygon aPol(Rect2Poly(getRectangle(), maGeo));
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
    tools::Rectangle aRectangle = svx::polygonToRectangle(aPol, maGeo);
    setRectangle(aRectangle);

    if (bRotate90)
    {
        bool bRota90=maGeo.m_nRotationAngle.get() % 9000 ==0;
        if (!bRota90) { // there's seems to be a rounding error occurring: correct it
            Degree100 a=NormAngle36000(maGeo.m_nRotationAngle);
            if (a<4500_deg100) a=0_deg100;
            else if (a<13500_deg100) a=9000_deg100;
            else if (a<22500_deg100) a=18000_deg100;
            else if (a<31500_deg100) a=27000_deg100;
            else a=0_deg100;
            maGeo.m_nRotationAngle=a;
            maGeo.RecalcSinCos();
        }
    }
    if (bNotSheared!=(maGeo.m_nShearAngle==0_deg100)) { // correct a rounding error occurring with Shear
        maGeo.m_nShearAngle=0_deg100;
        maGeo.RecalcTan();
    }

    if (mbTextFrame) {
        NbcAdjustTextFrameWidthAndHeight();
    }
    ImpCheckShear();
    SetBoundAndSnapRectsDirty();
    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(false);
}


rtl::Reference<SdrObject> SdrTextObj::ImpConvertContainedTextToSdrPathObjs(bool bToPoly) const
{
    rtl::Reference<SdrObject> pRetval;

    if(!ImpCanConvTextToCurve())
    {
        // suppress HelpTexts from PresObj's
        return nullptr;
    }

    // create an extractor with neutral ViewInformation
    const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
    drawinglayer::processor2d::TextAsPolygonExtractor2D aExtractor(aViewInformation2D);

    // extract text as polygons
    GetViewContact().getViewIndependentPrimitive2DContainer(aExtractor);

    // get results
    const drawinglayer::processor2d::TextAsPolygonDataNodeVector& rResult = aExtractor.getTarget();
    const sal_uInt32 nResultCount(rResult.size());

    if(nResultCount)
    {
        // prepare own target
        rtl::Reference<SdrObjGroup> pGroup = new SdrObjGroup(getSdrModelFromSdrObject());
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
                rtl::Reference<SdrPathObj> pPathObj;

                // always clear objectshadow; this is included in the extraction
                aAttributeSet.Put(makeSdrShadowItem(false));

                if(rCandidate.getIsFilled())
                {
                    // set needed items
                    aAttributeSet.Put(XFillColorItem(OUString(), Color(rCandidate.getBColor())));
                    aAttributeSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
                    aAttributeSet.Put(XFillStyleItem(drawing::FillStyle_SOLID));

                    // create filled SdrPathObj
                    pPathObj = new SdrPathObj(
                        getSdrModelFromSdrObject(),
                        SdrObjKind::PathFill,
                        std::move(aPolyPolygon));
                }
                else
                {
                    // set needed items
                    aAttributeSet.Put(XLineColorItem(OUString(), Color(rCandidate.getBColor())));
                    aAttributeSet.Put(XLineStyleItem(drawing::LineStyle_SOLID));
                    aAttributeSet.Put(XLineWidthItem(0));
                    aAttributeSet.Put(XFillStyleItem(drawing::FillStyle_NONE));

                    // create line SdrPathObj
                    pPathObj = new SdrPathObj(
                        getSdrModelFromSdrObject(),
                        SdrObjKind::PathLine,
                        std::move(aPolyPolygon));
                }

                // copy basic information from original
                pPathObj->ImpSetAnchorPos(GetAnchorPos());
                pPathObj->NbcSetLayer(GetLayer());
                pPathObj->NbcSetStyleSheet(GetStyleSheet(), true);

                // apply prepared ItemSet and add to target
                pPathObj->SetMergedItemSet(aAttributeSet);
                pObjectList->InsertObject(pPathObj.get());
            }
        }

        // postprocess; if no result and/or only one object, simplify
        if(!pObjectList->GetObjCount())
        {
            pGroup.clear();
        }
        else if(1 == pObjectList->GetObjCount())
        {
            pRetval = pObjectList->RemoveObject(0);
            pGroup.clear();
        }
        else
        {
            pRetval = pGroup;
        }
    }

    return pRetval;
}


rtl::Reference<SdrObject> SdrTextObj::DoConvertToPolyObj(bool bBezier, bool bAddText) const
{
    if(bAddText)
    {
        return ImpConvertContainedTextToSdrPathObjs(!bBezier);
    }

    return nullptr;
}

bool SdrTextObj::ImpCanConvTextToCurve() const
{
    return !IsOutlText() && !comphelper::IsFuzzing();
}

rtl::Reference<SdrPathObj> SdrTextObj::ImpConvertMakeObj(const basegfx::B2DPolyPolygon& rPolyPolygon, bool bClosed, bool bBezier) const
{
    SdrObjKind ePathKind = bClosed ? SdrObjKind::PathFill : SdrObjKind::PathLine;
    basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPolygon);

    // #i37011#
    if(!bBezier)
    {
        aB2DPolyPolygon = basegfx::utils::adaptiveSubdivideByAngle(aB2DPolyPolygon);
        ePathKind = bClosed ? SdrObjKind::Polygon : SdrObjKind::PolyLine;
    }

    rtl::Reference<SdrPathObj> pPathObj(new SdrPathObj(
        getSdrModelFromSdrObject(),
        ePathKind,
        std::move(aB2DPolyPolygon)));

    if(bBezier)
    {
        // create bezier curves
        pPathObj->SetPathPoly(basegfx::utils::expandToCurve(pPathObj->GetPathPoly()));
    }

    pPathObj->ImpSetAnchorPos(m_aAnchor);
    pPathObj->NbcSetLayer(GetLayer());
    sdr::properties::ItemChangeBroadcaster aC(*pPathObj);
    pPathObj->ClearMergedItem();
    pPathObj->SetMergedItemSet(GetObjectItemSet());
    pPathObj->GetProperties().BroadcastItemChange(aC);
    pPathObj->NbcSetStyleSheet(GetStyleSheet(), true);

    return pPathObj;
}

rtl::Reference<SdrObject> SdrTextObj::ImpConvertAddText(const rtl::Reference<SdrObject> & pObj, bool bBezier) const
{
    if(!ImpCanConvTextToCurve())
    {
        return pObj;
    }

    rtl::Reference<SdrObject> pText = ImpConvertContainedTextToSdrPathObjs(!bBezier);

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
        pOL->InsertObject(pObj.get(),0);

        return pText;
    }
    else
    {
        // not yet a group, create one and add partial and new shapes
        rtl::Reference<SdrObjGroup> pGrp(new SdrObjGroup(getSdrModelFromSdrObject()));
        SdrObjList* pOL=pGrp->GetSubList();
        pOL->InsertObject(pObj.get());
        pOL->InsertObject(pText.get());

        return pGrp;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
