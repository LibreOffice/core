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

#include <svx/svdorect.hxx>
#include <math.h>
#include <stdlib.h>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdopath.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <sdr/properties/rectangleproperties.hxx>
#include <svx/sdr/contact/viewcontactofsdrrectobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <vcl/ptrstyle.hxx>

using namespace com::sun::star;

// BaseProperties section

std::unique_ptr<sdr::properties::BaseProperties> SdrRectObj::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::RectangleProperties>(*this);
}


// DrawContact section

std::unique_ptr<sdr::contact::ViewContact> SdrRectObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfSdrRectObj>(*this);
}


SdrRectObj::SdrRectObj(SdrModel& rSdrModel)
:   SdrTextObj(rSdrModel)
{
    bClosedObj=true;
}

SdrRectObj::SdrRectObj(
    SdrModel& rSdrModel,
    const tools::Rectangle& rRect)
:   SdrTextObj(rSdrModel, rRect)
{
    bClosedObj=true;
}

SdrRectObj::SdrRectObj(
    SdrModel& rSdrModel,
    SdrObjKind eNewTextKind)
:   SdrTextObj(rSdrModel, eNewTextKind)
{
    DBG_ASSERT(eTextKind==OBJ_TEXT ||
               eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
               "SdrRectObj::SdrRectObj(SdrObjKind) can only be applied to text frames.");
    bClosedObj=true;
}

SdrRectObj::SdrRectObj(
    SdrModel& rSdrModel,
    SdrObjKind eNewTextKind,
    const tools::Rectangle& rRect)
:   SdrTextObj(rSdrModel, eNewTextKind, rRect)
{
    DBG_ASSERT(eTextKind==OBJ_TEXT ||
               eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
               "SdrRectObj::SdrRectObj(SdrObjKind,...) can only be applied to text frames.");
    bClosedObj=true;
}

SdrRectObj::~SdrRectObj()
{
}

void SdrRectObj::SetXPolyDirty()
{
    mpXPoly.reset();
}

XPolygon SdrRectObj::ImpCalcXPoly(const tools::Rectangle& rRect1, long nRad1) const
{
    XPolygon aXPoly(rRect1,nRad1,nRad1);
    const sal_uInt16 nPointCnt(aXPoly.GetPointCount());
    XPolygon aNewPoly(nPointCnt+1);
    sal_uInt16 nShift=nPointCnt-2;
    if (nRad1!=0) nShift=nPointCnt-5;
    sal_uInt16 j=nShift;
    for (sal_uInt16 i=1; i<nPointCnt; i++) {
        aNewPoly[i]=aXPoly[j];
        aNewPoly.SetFlags(i,aXPoly.GetFlags(j));
        j++;
        if (j>=nPointCnt) j=1;
    }
    aNewPoly[0]=rRect1.BottomCenter();
    aNewPoly[nPointCnt]=aNewPoly[0];
    aXPoly=aNewPoly;

    // these angles always relate to the top left corner of aRect
    if (aGeo.nShearAngle!=0) ShearXPoly(aXPoly,maRect.TopLeft(),aGeo.nTan);
    if (aGeo.nRotationAngle!=0) RotateXPoly(aXPoly,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    return aXPoly;
}

void SdrRectObj::RecalcXPoly()
{
    mpXPoly.reset( new XPolygon(ImpCalcXPoly(maRect,GetEckenradius())) );
}

const XPolygon& SdrRectObj::GetXPoly() const
{
    if(!mpXPoly)
    {
        const_cast<SdrRectObj*>(this)->RecalcXPoly();
    }

    return *mpXPoly;
}

void SdrRectObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bNoTextFrame=!IsTextFrame();
    rInfo.bResizeFreeAllowed=bNoTextFrame || aGeo.nRotationAngle%9000==0;
    rInfo.bResizePropAllowed=true;
    rInfo.bRotateFreeAllowed=true;
    rInfo.bRotate90Allowed  =true;
    rInfo.bMirrorFreeAllowed=bNoTextFrame;
    rInfo.bMirror45Allowed  =bNoTextFrame;
    rInfo.bMirror90Allowed  =bNoTextFrame;

    // allow transparency
    rInfo.bTransparenceAllowed = true;

    rInfo.bShearAllowed     =bNoTextFrame;
    rInfo.bEdgeRadiusAllowed=true;

    bool bCanConv=!HasText() || ImpCanConvTextToCurve();
    if (bCanConv && !bNoTextFrame && !HasText()) {
        bCanConv=HasFill() || HasLine();
    }
    rInfo.bCanConvToPath    =bCanConv;
    rInfo.bCanConvToPoly    =bCanConv;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrRectObj::GetObjIdentifier() const
{
    if (IsTextFrame()) return sal_uInt16(eTextKind);
    else return sal_uInt16(OBJ_RECT);
}

void SdrRectObj::TakeUnrotatedSnapRect(tools::Rectangle& rRect) const
{
    rRect = maRect;
    if (aGeo.nShearAngle!=0)
    {
        long nDst=FRound((maRect.Bottom()-maRect.Top())*aGeo.nTan);
        if (aGeo.nShearAngle>0)
        {
            Point aRef(rRect.TopLeft());
            rRect.AdjustLeft( -nDst );
            Point aTmpPt(rRect.TopLeft());
            RotatePoint(aTmpPt,aRef,aGeo.nSin,aGeo.nCos);
            aTmpPt-=rRect.TopLeft();
            rRect.Move(aTmpPt.X(),aTmpPt.Y());
        }
        else
        {
            rRect.AdjustRight( -nDst );
        }
    }
}

OUString SdrRectObj::TakeObjNameSingul() const
{
    if (IsTextFrame())
    {
        return SdrTextObj::TakeObjNameSingul();
    }

    OUStringBuffer sName;

    bool bRounded = GetEckenradius() != 0; // rounded down
    const char* pResId = bRounded ? STR_ObjNameSingulRECTRND : STR_ObjNameSingulRECT;
    if (aGeo.nShearAngle!=0)
    {
        pResId = bRounded ? STR_ObjNameSingulPARALRND : STR_ObjNameSingulPARAL;  // parallelogram or, maybe, rhombus
    }
    else if (maRect.GetWidth() == maRect.GetHeight())
    {
        pResId = bRounded ? STR_ObjNameSingulQUADRND : STR_ObjNameSingulQUAD; // square
    }
    sName.append(SvxResId(pResId));

    OUString aName(GetName());
    if (!aName.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aName);
        sName.append('\'');
    }

    return sName.makeStringAndClear();
}

OUString SdrRectObj::TakeObjNamePlural() const
{
    if (IsTextFrame())
    {
        return SdrTextObj::TakeObjNamePlural();
    }

    bool bRounded = GetEckenradius() != 0; // rounded down
    const char* pResId = bRounded ? STR_ObjNamePluralRECTRND : STR_ObjNamePluralRECT;
    if (aGeo.nShearAngle!=0)
    {
        pResId = bRounded ? STR_ObjNamePluralPARALRND : STR_ObjNamePluralPARAL;  // parallelogram or rhombus
    }
    else if (maRect.GetWidth() == maRect.GetHeight())
    {
        pResId = bRounded ? STR_ObjNamePluralQUADRND : STR_ObjNamePluralQUAD; // square
    }

    return SvxResId(pResId);
}

SdrRectObj* SdrRectObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< SdrRectObj >(rTargetModel);
}

SdrRectObj& SdrRectObj::operator=(const SdrRectObj& rCopy)
{
    if ( this == &rCopy )
        return *this;

    SdrTextObj::operator=( rCopy );

    if ( rCopy.mpXPoly )
        mpXPoly.reset( new XPolygon( *rCopy.mpXPoly ) );
    else
        mpXPoly.reset();

    return *this;
}

basegfx::B2DPolyPolygon SdrRectObj::TakeXorPoly() const
{
    XPolyPolygon aXPP;
    aXPP.Insert(ImpCalcXPoly(maRect,GetEckenradius()));
    return aXPP.getB2DPolyPolygon();
}

void SdrRectObj::RecalcSnapRect()
{
    long nEckRad=GetEckenradius();
    if ((aGeo.nRotationAngle!=0 || aGeo.nShearAngle!=0) && nEckRad!=0) {
        maSnapRect=GetXPoly().GetBoundRect();
    } else {
        SdrTextObj::RecalcSnapRect();
    }
}

void SdrRectObj::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    SdrTextObj::NbcSetSnapRect(rRect);
    SetXPolyDirty();
}

void SdrRectObj::NbcSetLogicRect(const tools::Rectangle& rRect)
{
    SdrTextObj::NbcSetLogicRect(rRect);
    SetXPolyDirty();
}

sal_uInt32 SdrRectObj::GetHdlCount() const
{
    return IsTextFrame() ? 10 : 9;
}

void SdrRectObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    // A text box has an additional (pseudo-)handle for the blinking frame.
    if(IsTextFrame())
    {
        OSL_ENSURE(!IsTextEditActive(), "Do not use a ImpTextframeHdl for highlighting text in active text edit, this will collide with EditEngine paints (!)");
        std::unique_ptr<SdrHdl> pH(new ImpTextframeHdl(maRect));
        pH->SetObj(const_cast<SdrRectObj*>(this));
        pH->SetRotationAngle(aGeo.nRotationAngle);
        rHdlList.AddHdl(std::move(pH));
    }

    for(sal_Int32 nHdlNum = 1; nHdlNum <= 9; ++nHdlNum)
    {
        Point aPnt;
        SdrHdlKind eKind = SdrHdlKind::Move;

        switch(nHdlNum)
        {
            case 1: // Handle for changing the corner radius
            {
                long a = GetEckenradius();
                long b = std::max(maRect.GetWidth(),maRect.GetHeight())/2; // rounded up, because GetWidth() adds 1
                if (a>b) a=b;
                if (a<0) a=0;
                aPnt=maRect.TopLeft();
                aPnt.AdjustX(a );
                eKind = SdrHdlKind::Circle;
                break;
            }
            case 2: aPnt=maRect.TopLeft();      eKind = SdrHdlKind::UpperLeft; break;
            case 3: aPnt=maRect.TopCenter();    eKind = SdrHdlKind::Upper; break;
            case 4: aPnt=maRect.TopRight();     eKind = SdrHdlKind::UpperRight; break;
            case 5: aPnt=maRect.LeftCenter();   eKind = SdrHdlKind::Left ; break;
            case 6: aPnt=maRect.RightCenter();  eKind = SdrHdlKind::Right; break;
            case 7: aPnt=maRect.BottomLeft();   eKind = SdrHdlKind::LowerLeft; break;
            case 8: aPnt=maRect.BottomCenter(); eKind = SdrHdlKind::Lower; break;
            case 9: aPnt=maRect.BottomRight();  eKind = SdrHdlKind::LowerRight; break;
        }

        if(aGeo.nShearAngle)
        {
            ShearPoint(aPnt,maRect.TopLeft(),aGeo.nTan);
        }
        if(aGeo.nRotationAngle)
        {
            RotatePoint(aPnt,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
        }

        std::unique_ptr<SdrHdl> pH(new SdrHdl(aPnt,eKind));
        pH->SetObj(const_cast<SdrRectObj*>(this));
        pH->SetRotationAngle(aGeo.nRotationAngle);
        rHdlList.AddHdl(std::move(pH));
    }
}

bool SdrRectObj::hasSpecialDrag() const
{
    return true;
}

bool SdrRectObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const bool bRad(rDrag.GetHdl() && SdrHdlKind::Circle == rDrag.GetHdl()->GetKind());

    if(bRad)
    {
        rDrag.SetEndDragChangesAttributes(true);

        return true;
    }

    return SdrTextObj::beginSpecialDrag(rDrag);
}

bool SdrRectObj::applySpecialDrag(SdrDragStat& rDrag)
{
    const bool bRad(rDrag.GetHdl() && SdrHdlKind::Circle == rDrag.GetHdl()->GetKind());

    if (bRad)
    {
        Point aPt(rDrag.GetNow());

        if(aGeo.nRotationAngle)
            RotatePoint(aPt,maRect.TopLeft(),-aGeo.nSin,aGeo.nCos);

        sal_Int32 nRad(aPt.X() - maRect.Left());

        if (nRad < 0)
            nRad = 0;

        if(nRad != GetEckenradius())
        {
            NbcSetEckenradius(nRad);
        }

        return true;
    }
    else
    {
        return SdrTextObj::applySpecialDrag(rDrag);
    }
}

OUString SdrRectObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    const bool bCreateComment(rDrag.GetView() && this == rDrag.GetView()->GetCreateObj());

    if(bCreateComment)
    {
        return OUString();
    }
    else
    {
        const bool bRad(rDrag.GetHdl() && SdrHdlKind::Circle == rDrag.GetHdl()->GetKind());

        if(bRad)
        {
            Point aPt(rDrag.GetNow());

            // -sin for reversal
            if(aGeo.nRotationAngle)
                RotatePoint(aPt, maRect.TopLeft(), -aGeo.nSin, aGeo.nCos);

            sal_Int32 nRad(aPt.X() - maRect.Left());

            if(nRad < 0)
                nRad = 0;

            OUString aStr;
            ImpTakeDescriptionStr(STR_DragRectEckRad, aStr);
            OUStringBuffer aBuf(aStr);
            aBuf.append(" (");
            aBuf.append(GetMetrStr(nRad));
            aBuf.append(')');

            return aBuf.makeStringAndClear();
        }
        else
        {
            return SdrTextObj::getSpecialDragComment(rDrag);
        }
    }
}


basegfx::B2DPolyPolygon SdrRectObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    tools::Rectangle aRect1;
    rDrag.TakeCreateRect(aRect1);
    aRect1.Justify();

    basegfx::B2DPolyPolygon aRetval;
    aRetval.append(ImpCalcXPoly(aRect1,GetEckenradius()).getB2DPolygon());
    return aRetval;
}

PointerStyle SdrRectObj::GetCreatePointer() const
{
    if (IsTextFrame()) return PointerStyle::DrawText;
    return PointerStyle::DrawRect;
}

void SdrRectObj::NbcMove(const Size& rSiz)
{
    SdrTextObj::NbcMove(rSiz);
    SetXPolyDirty();
}

void SdrRectObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrTextObj::NbcResize(rRef,xFact,yFact);
    SetXPolyDirty();
}

void SdrRectObj::NbcRotate(const Point& rRef, long nAngle, double sn, double cs)
{
    SdrTextObj::NbcRotate(rRef,nAngle,sn,cs);
    SetXPolyDirty();
}

void SdrRectObj::NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear)
{
    SdrTextObj::NbcShear(rRef,nAngle,tn,bVShear);
    SetXPolyDirty();
}

void SdrRectObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SdrTextObj::NbcMirror(rRef1,rRef2);
    SetXPolyDirty();
}

SdrGluePoint SdrRectObj::GetVertexGluePoint(sal_uInt16 nPosNum) const
{
    sal_Int32 nWdt = ImpGetLineWdt(); // #i25616#

    // #i25616#
    if(!LineIsOutsideGeometry())
    {
        nWdt++;
        nWdt /= 2;
    }

    Point aPt;
    switch (nPosNum) {
        case 0: aPt=maRect.TopCenter();    aPt.AdjustY( -nWdt ); break;
        case 1: aPt=maRect.RightCenter();  aPt.AdjustX(nWdt ); break;
        case 2: aPt=maRect.BottomCenter(); aPt.AdjustY(nWdt ); break;
        case 3: aPt=maRect.LeftCenter();   aPt.AdjustX( -nWdt ); break;
    }
    if (aGeo.nShearAngle!=0) ShearPoint(aPt,maRect.TopLeft(),aGeo.nTan);
    if (aGeo.nRotationAngle!=0) RotatePoint(aPt,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    aPt-=GetSnapRect().Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(false);
    return aGP;
}

SdrGluePoint SdrRectObj::GetCornerGluePoint(sal_uInt16 nPosNum) const
{
    sal_Int32 nWdt = ImpGetLineWdt(); // #i25616#

    // #i25616#
    if(!LineIsOutsideGeometry())
    {
        nWdt++;
        nWdt /= 2;
    }

    Point aPt;
    switch (nPosNum) {
        case 0: aPt=maRect.TopLeft();     aPt.AdjustX( -nWdt ); aPt.AdjustY( -nWdt ); break;
        case 1: aPt=maRect.TopRight();    aPt.AdjustX(nWdt ); aPt.AdjustY( -nWdt ); break;
        case 2: aPt=maRect.BottomRight(); aPt.AdjustX(nWdt ); aPt.AdjustY(nWdt ); break;
        case 3: aPt=maRect.BottomLeft();  aPt.AdjustX( -nWdt ); aPt.AdjustY(nWdt ); break;
    }
    if (aGeo.nShearAngle!=0) ShearPoint(aPt,maRect.TopLeft(),aGeo.nTan);
    if (aGeo.nRotationAngle!=0) RotatePoint(aPt,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    aPt-=GetSnapRect().Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(false);
    return aGP;
}

SdrObject* SdrRectObj::DoConvertToPolyObj(bool bBezier, bool bAddText) const
{
    XPolygon aXP(ImpCalcXPoly(maRect,GetEckenradius()));
    { // TODO: this is only for the moment, until we have the new TakeContour()
        aXP.Remove(0,1);
        aXP[aXP.GetPointCount()-1]=aXP[0];
    }

    basegfx::B2DPolyPolygon aPolyPolygon(aXP.getB2DPolygon());
    aPolyPolygon.removeDoublePoints();
    SdrObject* pRet = nullptr;

    // small correction: Do not create something when no fill and no line. To
    // be sure to not damage something with non-text frames, do this only
    // when used with bAddText==false from other converters
    if((bAddText && !IsTextFrame()) || HasFill() || HasLine())
    {
        pRet = ImpConvertMakeObj(aPolyPolygon, true, bBezier);
    }

    if(bAddText)
    {
        pRet = ImpConvertAddText(pRet, bBezier);
    }

    return pRet;
}

void SdrRectObj::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SdrTextObj::Notify(rBC,rHint);
    SetXPolyDirty(); // because of the corner radius
}

void SdrRectObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrTextObj::RestGeoData(rGeo);
    SetXPolyDirty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
