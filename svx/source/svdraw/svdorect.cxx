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
#include <svx/svdocapt.hxx> // for Import of SdrFileVersion 2
#include <svx/svdpagv.hxx> // for
#include <svx/svdview.hxx> // the
#include <svx/svdundo.hxx> // macro example
#include <svx/svdopath.hxx>
#include "svx/svdglob.hxx"  // Stringcache
#include "svx/svdstr.hrc"   // the object's name
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/sdr/properties/rectangleproperties.hxx>
#include <svx/sdr/contact/viewcontactofsdrrectobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrRectObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::RectangleProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrRectObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrRectObj(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrRectObj,SdrTextObj);

SdrRectObj::SdrRectObj()
:   mpXPoly(0L)
{
    bClosedObj=sal_True;
}

SdrRectObj::SdrRectObj(const Rectangle& rRect)
:   SdrTextObj(rRect),
    mpXPoly(NULL)
{
    bClosedObj=sal_True;
}

SdrRectObj::SdrRectObj(SdrObjKind eNewTextKind)
:   SdrTextObj(eNewTextKind),
    mpXPoly(NULL)
{
    DBG_ASSERT(eTextKind==OBJ_TEXT || eTextKind==OBJ_TEXTEXT ||
               eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
               "SdrRectObj::SdrRectObj(SdrObjKind) can only be applied to text frames.");
    bClosedObj=sal_True;
}

SdrRectObj::SdrRectObj(SdrObjKind eNewTextKind, const Rectangle& rRect)
:   SdrTextObj(eNewTextKind,rRect),
    mpXPoly(NULL)
{
    DBG_ASSERT(eTextKind==OBJ_TEXT || eTextKind==OBJ_TEXTEXT ||
               eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
               "SdrRectObj::SdrRectObj(SdrObjKind,...) can only be applied to text frames.");
    bClosedObj=sal_True;
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
        mpXPoly = NULL;

    return *this;
}

void SdrRectObj::SetXPolyDirty()
{
    delete mpXPoly;
    mpXPoly = 0L;
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
    if (aGeo.nShearWink!=0) ShearXPoly(aXPoly,aRect.TopLeft(),aGeo.nTan);
    if (aGeo.nDrehWink!=0) RotateXPoly(aXPoly,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    return aXPoly;
}

void SdrRectObj::RecalcXPoly()
{
    delete mpXPoly;
    mpXPoly = new XPolygon(ImpCalcXPoly(aRect,GetEckenradius()));
}

const XPolygon& SdrRectObj::GetXPoly() const
{
    if(!mpXPoly)
    {
        ((SdrRectObj*)this)->RecalcXPoly();
    }

    return *mpXPoly;
}

void SdrRectObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bNoTextFrame=!IsTextFrame();
    rInfo.bResizeFreeAllowed=bNoTextFrame || aGeo.nDrehWink%9000==0;
    rInfo.bResizePropAllowed=sal_True;
    rInfo.bRotateFreeAllowed=sal_True;
    rInfo.bRotate90Allowed  =sal_True;
    rInfo.bMirrorFreeAllowed=bNoTextFrame;
    rInfo.bMirror45Allowed  =bNoTextFrame;
    rInfo.bMirror90Allowed  =bNoTextFrame;

    // allow transparency
    rInfo.bTransparenceAllowed = sal_True;

    // gradient depends on fillstyle
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetObjectItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.bGradientAllowed = (eFillStyle == XFILL_GRADIENT);

    rInfo.bShearAllowed     =bNoTextFrame;
    rInfo.bEdgeRadiusAllowed=sal_True;

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
    rRect=aRect;
    if (aGeo.nShearWink!=0) {
        long nDst=Round((aRect.Bottom()-aRect.Top())*aGeo.nTan);
        if (aGeo.nShearWink>0) {
            Point aRef(rRect.TopLeft());
            rRect.Left()-=nDst;
            Point aTmpPt(rRect.TopLeft());
            RotatePoint(aTmpPt,aRef,aGeo.nSin,aGeo.nCos);
            aTmpPt-=rRect.TopLeft();
            rRect.Move(aTmpPt.X(),aTmpPt.Y());
        } else {
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
    if (aGeo.nShearWink!=0) {
        nResId+=4;  // parallelogram or, maybe, rhombus
    } else {
        if (aRect.GetWidth()==aRect.GetHeight()) nResId+=2; // square
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

void SdrRectObj::TakeObjNamePlural(XubString& rName) const
{
    if (IsTextFrame()) SdrTextObj::TakeObjNamePlural(rName);
    else {
        sal_uInt16 nResId=STR_ObjNamePluralRECT;
        if (aGeo.nShearWink!=0) {
            nResId+=4;  // parallelogram or rhombus
        } else {
            if (aRect.GetWidth()==aRect.GetHeight()) nResId+=2; // square
        }
        if (GetEckenradius()!=0) nResId+=8; // rounded down
        rName=ImpGetResStr(nResId);
    }
}

SdrRectObj* SdrRectObj::Clone() const
{
    return CloneHelper< SdrRectObj >();
}

basegfx::B2DPolyPolygon SdrRectObj::TakeXorPoly() const
{
    XPolyPolygon aXPP;
    aXPP.Insert(ImpCalcXPoly(aRect,GetEckenradius()));
    return aXPP.getB2DPolyPolygon();
}

void SdrRectObj::RecalcSnapRect()
{
    long nEckRad=GetEckenradius();
    if ((aGeo.nDrehWink!=0 || aGeo.nShearWink!=0) && nEckRad!=0) {
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
    SdrHdl* pH = NULL;
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
            pH = new ImpTextframeHdl(aRect + GetGridOffset() );
            pH->SetObj((SdrObject*)this);
            pH->SetDrehWink(aGeo.nDrehWink);
            break;
        }
        case 1:
        {
            long a = GetEckenradius();
            long b = std::max(aRect.GetWidth(),aRect.GetHeight())/2; // rounded up, because GetWidth() adds 1
            if (a>b) a=b;
            if (a<0) a=0;
            aPnt=aRect.TopLeft();
            aPnt.X()+=a;
            eKind = HDL_CIRC;
            break;
        }
        case 2: aPnt=aRect.TopLeft();      eKind = HDL_UPLFT; break;
        case 3: aPnt=aRect.TopCenter();    eKind = HDL_UPPER; break;
        case 4: aPnt=aRect.TopRight();     eKind = HDL_UPRGT; break;
        case 5: aPnt=aRect.LeftCenter();   eKind = HDL_LEFT ; break;
        case 6: aPnt=aRect.RightCenter();  eKind = HDL_RIGHT; break;
        case 7: aPnt=aRect.BottomLeft();   eKind = HDL_LWLFT; break;
        case 8: aPnt=aRect.BottomCenter(); eKind = HDL_LOWER; break;
        case 9: aPnt=aRect.BottomRight();  eKind = HDL_LWRGT; break;
    }

    if(!pH)
    {
        if(aGeo.nShearWink)
        {
            ShearPoint(aPnt,aRect.TopLeft(),aGeo.nTan);
        }

        if(aGeo.nDrehWink)
        {
            RotatePoint(aPnt,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
        }

        pH = new SdrHdl(aPnt,eKind);
        pH->SetObj((SdrObject*)this);
        pH->SetDrehWink(aGeo.nDrehWink);
    }

    return pH;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

        if(aGeo.nDrehWink)
            RotatePoint(aPt,aRect.TopLeft(),-aGeo.nSin,aGeo.nCos);

        sal_Int32 nRad(aPt.X() - aRect.Left());

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

String SdrRectObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    const bool bCreateComment(rDrag.GetView() && this == rDrag.GetView()->GetCreateObj());

    if(bCreateComment)
    {
        return String();
    }
    else
    {
        const bool bRad(rDrag.GetHdl() && HDL_CIRC == rDrag.GetHdl()->GetKind());

        if(bRad)
        {
            Point aPt(rDrag.GetNow());

            // -sin for reversal
            if(aGeo.nDrehWink)
                RotatePoint(aPt, aRect.TopLeft(), -aGeo.nSin, aGeo.nCos);

            sal_Int32 nRad(aPt.X() - aRect.Left());

            if(nRad < 0)
                nRad = 0;

            OUString aStr;
            ImpTakeDescriptionStr(STR_DragRectEckRad, aStr);
            OUStringBuffer aBuf(aStr);
            aBuf.appendAscii(" (");
            aBuf.append(GetMetrStr(nRad));
            aBuf.append(sal_Unicode(')'));

            return aBuf.makeStringAndClear();
        }
        else
        {
            return SdrTextObj::getSpecialDragComment(rDrag);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
    if (IsTextFrame()) return Pointer(POINTER_DRAW_TEXT);
    return Pointer(POINTER_DRAW_RECT);
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

void SdrRectObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    SdrTextObj::NbcRotate(rRef,nWink,sn,cs);
    SetXPolyDirty();
}

void SdrRectObj::NbcShear(const Point& rRef, long nWink, double tn, bool bVShear)
{
    SdrTextObj::NbcShear(rRef,nWink,tn,bVShear);
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
        case 0: aPt=aRect.TopCenter();    aPt.Y()-=nWdt; break;
        case 1: aPt=aRect.RightCenter();  aPt.X()+=nWdt; break;
        case 2: aPt=aRect.BottomCenter(); aPt.Y()+=nWdt; break;
        case 3: aPt=aRect.LeftCenter();   aPt.X()-=nWdt; break;
    }
    if (aGeo.nShearWink!=0) ShearPoint(aPt,aRect.TopLeft(),aGeo.nTan);
    if (aGeo.nDrehWink!=0) RotatePoint(aPt,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    aPt-=GetSnapRect().Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(sal_False);
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
        case 0: aPt=aRect.TopLeft();     aPt.X()-=nWdt; aPt.Y()-=nWdt; break;
        case 1: aPt=aRect.TopRight();    aPt.X()+=nWdt; aPt.Y()-=nWdt; break;
        case 2: aPt=aRect.BottomRight(); aPt.X()+=nWdt; aPt.Y()+=nWdt; break;
        case 3: aPt=aRect.BottomLeft();  aPt.X()-=nWdt; aPt.Y()+=nWdt; break;
    }
    if (aGeo.nShearWink!=0) ShearPoint(aPt,aRect.TopLeft(),aGeo.nTan);
    if (aGeo.nDrehWink!=0) RotatePoint(aPt,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    aPt-=GetSnapRect().Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(sal_False);
    return aGP;
}

SdrObject* SdrRectObj::DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const
{
    XPolygon aXP(ImpCalcXPoly(aRect,GetEckenradius()));
    { // TODO: this is only for the moment, until we have the new TakeContour()
        aXP.Remove(0,1);
        aXP[aXP.GetPointCount()-1]=aXP[0];
    }

    basegfx::B2DPolyPolygon aPolyPolygon(aXP.getB2DPolygon());
    aPolyPolygon.removeDoublePoints();
    SdrObject* pRet = 0L;

    // small correction: Do not create something when no fill and no line. To
    // be sure to not damage something with non-text frames, do this only
    // when used with bAddText==false from other converters
    if((bAddText && !IsTextFrame()) || HasFill() || HasLine())
    {
        pRet = ImpConvertMakeObj(aPolyPolygon, sal_True, bBezier);
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
