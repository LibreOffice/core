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
#include "svdglob.hxx"
#include "svx/svdstr.hrc"
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <sdr/properties/rectangleproperties.hxx>
#include <svx/sdr/contact/viewcontactofsdrrectobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

using namespace com::sun::star;

// BaseProperties section

sdr::properties::BaseProperties* SdrRectObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::RectangleProperties(*this);
}


// DrawContact section

sdr::contact::ViewContact* SdrRectObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrRectObj(*this);
}



TYPEINIT1(SdrRectObj,SdrTextObj);

SdrRectObj::SdrRectObj()
:   mpXPoly(nullptr)
{
    bClosedObj=true;
}

SdrRectObj::SdrRectObj(const Rectangle& rRect)
:   SdrTextObj(rRect),
    mpXPoly(nullptr)
{
    bClosedObj=true;
}

SdrRectObj::SdrRectObj(SdrObjKind eNewTextKind)
:   SdrTextObj(eNewTextKind),
    mpXPoly(nullptr)
{
    DBG_ASSERT(eTextKind==OBJ_TEXT || eTextKind==OBJ_TEXTEXT ||
               eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
               "SdrRectObj::SdrRectObj(SdrObjKind) can only be applied to text frames.");
    bClosedObj=true;
}

SdrRectObj::SdrRectObj(SdrObjKind eNewTextKind, const Rectangle& rRect)
:   SdrTextObj(eNewTextKind,rRect),
    mpXPoly(nullptr)
{
    DBG_ASSERT(eTextKind==OBJ_TEXT || eTextKind==OBJ_TEXTEXT ||
               eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
               "SdrRectObj::SdrRectObj(SdrObjKind,...) can only be applied to text frames.");
    bClosedObj=true;
}

SdrRectObj::~SdrRectObj()
{
    delete mpXPoly;
}

SdrRectObj& SdrRectObj::operator=(const SdrRectObj& rCopy)
{
    if ( this == &rCopy )
        return *this;

    SdrTextObj::operator=( rCopy );

    delete mpXPoly;

    if ( rCopy.mpXPoly )
        mpXPoly = new XPolygon( *rCopy.mpXPoly );
    else
        mpXPoly = nullptr;

    return *this;
}

void SdrRectObj::SetXPolyDirty()
{
    delete mpXPoly;
    mpXPoly = nullptr;
}

XPolygon SdrRectObj::ImpCalcXPoly(const Rectangle& rRect1, long nRad1) const
{
    XPolygon aXPoly(rRect1,nRad1,nRad1);
    const sal_uInt16 nPointAnz(aXPoly.GetPointCount());
    XPolygon aNeuPoly(nPointAnz+1);
    sal_uInt16 nShift=nPointAnz-2;
    if (nRad1!=0) nShift=nPointAnz-5;
    sal_uInt16 j=nShift;
    for (sal_uInt16 i=1; i<nPointAnz; i++) {
        aNeuPoly[i]=aXPoly[j];
        aNeuPoly.SetFlags(i,aXPoly.GetFlags(j));
        j++;
        if (j>=nPointAnz) j=1;
    }
    aNeuPoly[0]=rRect1.BottomCenter();
    aNeuPoly[nPointAnz]=aNeuPoly[0];
    aXPoly=aNeuPoly;

    // these angles always relate to the top left corner of aRect
    if (aGeo.nShearAngle!=0) ShearXPoly(aXPoly,maRect.TopLeft(),aGeo.nTan);
    if (aGeo.nRotationAngle!=0) RotateXPoly(aXPoly,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    return aXPoly;
}

void SdrRectObj::RecalcXPoly()
{
    delete mpXPoly;
    mpXPoly = new XPolygon(ImpCalcXPoly(maRect,GetEckenradius()));
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

    // gradient depends on fillstyle
    drawing::FillStyle eFillStyle = static_cast<const XFillStyleItem&>(GetObjectItem(XATTR_FILLSTYLE)).GetValue();
    rInfo.bGradientAllowed = (eFillStyle == drawing::FillStyle_GRADIENT);

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

void SdrRectObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
{
    rRect = maRect;
    if (aGeo.nShearAngle!=0)
    {
        long nDst=Round((maRect.Bottom()-maRect.Top())*aGeo.nTan);
        if (aGeo.nShearAngle>0)
        {
            Point aRef(rRect.TopLeft());
            rRect.Left()-=nDst;
            Point aTmpPt(rRect.TopLeft());
            RotatePoint(aTmpPt,aRef,aGeo.nSin,aGeo.nCos);
            aTmpPt-=rRect.TopLeft();
            rRect.Move(aTmpPt.X(),aTmpPt.Y());
        }
        else
        {
            rRect.Right()-=nDst;
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

    sal_uInt16 nResId=STR_ObjNameSingulRECT;
    if (aGeo.nShearAngle!=0) {
        nResId+=4;  // parallelogram or, maybe, rhombus
    } else {
        if (maRect.GetWidth() == maRect.GetHeight()) nResId+=2; // square
    }
    if (GetEckenradius()!=0) nResId+=8; // rounded down
    sName.append(ImpGetResStr(nResId));

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

    sal_uInt16 nResId=STR_ObjNamePluralRECT;

    if (aGeo.nShearAngle!=0)
    {
        nResId+=4;  // parallelogram or rhombus
    }
    else
    {
        if (maRect.GetWidth() == maRect.GetHeight())
            nResId+=2; // square
    }

    if (GetEckenradius()!=0)
        nResId+=8; // rounded down

    return ImpGetResStr(nResId);
}

SdrRectObj* SdrRectObj::Clone() const
{
    return CloneHelper< SdrRectObj >();
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

void SdrRectObj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrTextObj::NbcSetSnapRect(rRect);
    SetXPolyDirty();
}

void SdrRectObj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrTextObj::NbcSetLogicRect(rRect);
    SetXPolyDirty();
}

sal_uInt32 SdrRectObj::GetHdlCount() const
{
    return IsTextFrame() ? 10 : 9;
}

SdrHdl* SdrRectObj::GetHdl(sal_uInt32 nHdlNum) const
{
    SdrHdl* pH = nullptr;
    Point aPnt;
    SdrHdlKind eKind = HDL_MOVE;

    if(!IsTextFrame())
    {
        nHdlNum++;
    }

    switch(nHdlNum)
    {
        case 0:
        {
            OSL_ENSURE(!IsTextEditActive(), "Do not use a ImpTextframeHdl for hilighting text in active text edit, this will collide with EditEngine paints (!)");
            // hack for calc grid sync to ensure the hatched area
            // for a textbox is displayed at correct position
            pH = new ImpTextframeHdl(maRect + GetGridOffset() );
            pH->SetObj(const_cast<SdrRectObj*>(this));
            pH->SetRotationAngle(aGeo.nRotationAngle);
            break;
        }
        case 1:
        {
            long a = GetEckenradius();
            long b = std::max(maRect.GetWidth(),maRect.GetHeight())/2; // rounded up, because GetWidth() adds 1
            if (a>b) a=b;
            if (a<0) a=0;
            aPnt=maRect.TopLeft();
            aPnt.X()+=a;
            eKind = HDL_CIRC;
            break;
        }
        case 2: aPnt=maRect.TopLeft();      eKind = HDL_UPLFT; break;
        case 3: aPnt=maRect.TopCenter();    eKind = HDL_UPPER; break;
        case 4: aPnt=maRect.TopRight();     eKind = HDL_UPRGT; break;
        case 5: aPnt=maRect.LeftCenter();   eKind = HDL_LEFT ; break;
        case 6: aPnt=maRect.RightCenter();  eKind = HDL_RIGHT; break;
        case 7: aPnt=maRect.BottomLeft();   eKind = HDL_LWLFT; break;
        case 8: aPnt=maRect.BottomCenter(); eKind = HDL_LOWER; break;
        case 9: aPnt=maRect.BottomRight();  eKind = HDL_LWRGT; break;
    }

    if(!pH)
    {
        if(aGeo.nShearAngle)
        {
            ShearPoint(aPnt,maRect.TopLeft(),aGeo.nTan);
        }

        if(aGeo.nRotationAngle)
        {
            RotatePoint(aPnt,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
        }

        pH = new SdrHdl(aPnt,eKind);
        pH->SetObj(const_cast<SdrRectObj*>(this));
        pH->SetRotationAngle(aGeo.nRotationAngle);
    }

    return pH;
}



bool SdrRectObj::hasSpecialDrag() const
{
    return true;
}

bool SdrRectObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const bool bRad(rDrag.GetHdl() && HDL_CIRC == rDrag.GetHdl()->GetKind());

    if(bRad)
    {
        rDrag.SetEndDragChangesAttributes(true);

        return true;
    }

    return SdrTextObj::beginSpecialDrag(rDrag);
}

bool SdrRectObj::applySpecialDrag(SdrDragStat& rDrag)
{
    const bool bRad(rDrag.GetHdl() && HDL_CIRC == rDrag.GetHdl()->GetKind());

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
        const bool bRad(rDrag.GetHdl() && HDL_CIRC == rDrag.GetHdl()->GetKind());

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
    Rectangle aRect1;
    rDrag.TakeCreateRect(aRect1);
    aRect1.Justify();

    basegfx::B2DPolyPolygon aRetval;
    aRetval.append(ImpCalcXPoly(aRect1,GetEckenradius()).getB2DPolygon());
    return aRetval;
}

Pointer SdrRectObj::GetCreatePointer() const
{
    if (IsTextFrame()) return Pointer(PointerStyle::DrawText);
    return Pointer(PointerStyle::DrawRect);
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

bool SdrRectObj::DoMacro(const SdrObjMacroHitRec& rRec)
{
    return SdrTextObj::DoMacro(rRec);
}

OUString SdrRectObj::GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const
{
    return SdrTextObj::GetMacroPopupComment(rRec);
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
        case 0: aPt=maRect.TopCenter();    aPt.Y()-=nWdt; break;
        case 1: aPt=maRect.RightCenter();  aPt.X()+=nWdt; break;
        case 2: aPt=maRect.BottomCenter(); aPt.Y()+=nWdt; break;
        case 3: aPt=maRect.LeftCenter();   aPt.X()-=nWdt; break;
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
        case 0: aPt=maRect.TopLeft();     aPt.X()-=nWdt; aPt.Y()-=nWdt; break;
        case 1: aPt=maRect.TopRight();    aPt.X()+=nWdt; aPt.Y()-=nWdt; break;
        case 2: aPt=maRect.BottomRight(); aPt.X()+=nWdt; aPt.Y()+=nWdt; break;
        case 3: aPt=maRect.BottomLeft();  aPt.X()-=nWdt; aPt.Y()+=nWdt; break;
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
