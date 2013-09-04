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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <editeng/eeitem.hxx>
#include <svl/style.hxx>
#include <tools/bigint.hxx>
#include <tools/helpers.hxx>

#include "svdconv.hxx"
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // the object's name

#include <svx/sdr/contact/viewcontactofsdrcaptionobj.hxx>
#include <svx/sdr/properties/captionproperties.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdview.hxx>   // for RectSnap
#include <svx/sxcaitm.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/sxcgitm.hxx>
#include <svx/sxcllitm.hxx>
#include <svx/sxctitm.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xpoly.hxx>
#include <svx/xpool.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

enum EscDir {LKS,RTS,OBN,UNT};

class ImpCaptParams
{
public:
    SdrCaptionType              eType;
    long                        nAngle;
    long                        nGap;
    long                        nEscRel;
    long                        nEscAbs;
    long                        nLineLen;
    SdrCaptionEscDir            eEscDir;
    bool                        bFitLineLen;
    bool                        bEscRel;
    bool                        bFixedAngle;

public:
    ImpCaptParams()
    {
        eType      =SDRCAPT_TYPE3;
        bFixedAngle=sal_False;
        nAngle     =4500;
        nGap       =0;
        eEscDir    =SDRCAPT_ESCHORIZONTAL;
        bEscRel    =sal_True;
        nEscRel    =5000;
        nEscAbs    =0;
        nLineLen   =0;
        bFitLineLen=sal_True;
    }
    void CalcEscPos(const Point& rTail, const Rectangle& rRect, Point& rPt, EscDir& rDir) const;
};

void ImpCaptParams::CalcEscPos(const Point& rTailPt, const Rectangle& rRect, Point& rPt, EscDir& rDir) const
{
    Point aTl(rTailPt); // copy locally for performance reasons
    long nX,nY;
    if (bEscRel) {
        nX=rRect.Right()-rRect.Left();
        nX=BigMulDiv(nX,nEscRel,10000);
        nY=rRect.Bottom()-rRect.Top();
        nY=BigMulDiv(nY,nEscRel,10000);
    } else {
        nX=nEscAbs;
        nY=nEscAbs;
    }
    nX+=rRect.Left();
    nY+=rRect.Top();
    Point  aBestPt;
    EscDir eBestDir=LKS;
    bool bTryH=eEscDir==SDRCAPT_ESCBESTFIT;
    if (!bTryH) {
        if (eType!=SDRCAPT_TYPE1) {
            bTryH=eEscDir==SDRCAPT_ESCHORIZONTAL;
        } else {
            bTryH=eEscDir==SDRCAPT_ESCVERTICAL;
        }
    }
    bool bTryV=eEscDir==SDRCAPT_ESCBESTFIT;
    if (!bTryV) {
        if (eType!=SDRCAPT_TYPE1) {
            bTryV=eEscDir==SDRCAPT_ESCVERTICAL;
        } else {
            bTryV=eEscDir==SDRCAPT_ESCHORIZONTAL;
        }
    }

    if (bTryH) {
        Point aLft(rRect.Left()-nGap,nY);
        Point aRgt(rRect.Right()+nGap,nY);
        bool bLft=(aTl.X()-aLft.X()<aRgt.X()-aTl.X());
        if (bLft) {
            eBestDir=LKS;
            aBestPt=aLft;
        } else {
            eBestDir=RTS;
            aBestPt=aRgt;
        }
    }
    if (bTryV) {
        Point aTop(nX,rRect.Top()-nGap);
        Point aBtm(nX,rRect.Bottom()+nGap);
        bool bTop=(aTl.Y()-aTop.Y()<aBtm.Y()-aTl.Y());
        Point aBest2;
        EscDir eBest2;
        if (bTop) {
            eBest2=OBN;
            aBest2=aTop;
        } else {
            eBest2=UNT;
            aBest2=aBtm;
        }
        bool bTakeIt=eEscDir!=SDRCAPT_ESCBESTFIT;
        if (!bTakeIt) {
            BigInt aHorX(aBestPt.X()-aTl.X()); aHorX*=aHorX;
            BigInt aHorY(aBestPt.Y()-aTl.Y()); aHorY*=aHorY;
            BigInt aVerX(aBest2.X()-aTl.X());  aVerX*=aVerX;
            BigInt aVerY(aBest2.Y()-aTl.Y());  aVerY*=aVerY;
            if (eType!=SDRCAPT_TYPE1) {
                bTakeIt=aVerX+aVerY<aHorX+aHorY;
            } else {
                bTakeIt=aVerX+aVerY>=aHorX+aHorY;
            }
        }
        if (bTakeIt) {
            aBestPt=aBest2;
            eBestDir=eBest2;
        }
    }
    rPt=aBestPt;
    rDir=eBestDir;
}

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrCaptionObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::CaptionProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrCaptionObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrCaptionObj(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrCaptionObj,SdrRectObj);

SdrCaptionObj::SdrCaptionObj():
    SdrRectObj(OBJ_TEXT),
    aTailPoly(3),  // default size: 3 points = 2 lines
    mbSpecialTextBoxShadow(sal_False),
    mbFixedTail(sal_False)
{
}

SdrCaptionObj::SdrCaptionObj(const Rectangle& rRect, const Point& rTail):
    SdrRectObj(OBJ_TEXT,rRect),
    aTailPoly(3),  // default size: 3 points = 2 lines
    mbSpecialTextBoxShadow(sal_False),
    mbFixedTail(sal_False)
{
    aTailPoly[0]=maFixedTailPos=rTail;
}

SdrCaptionObj::~SdrCaptionObj()
{
}

void SdrCaptionObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=sal_False;
    rInfo.bRotate90Allowed  =sal_False;
    rInfo.bMirrorFreeAllowed=sal_False;
    rInfo.bMirror45Allowed  =sal_False;
    rInfo.bMirror90Allowed  =sal_False;
    rInfo.bTransparenceAllowed = sal_False;
    rInfo.bGradientAllowed = sal_False;
    rInfo.bShearAllowed     =sal_False;
    rInfo.bEdgeRadiusAllowed=sal_False;
    rInfo.bCanConvToPath    =sal_True;
    rInfo.bCanConvToPoly    =sal_True;
    rInfo.bCanConvToPathLineToArea=sal_False;
    rInfo.bCanConvToPolyLineToArea=sal_False;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrCaptionObj::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_CAPTION);
}

SdrCaptionObj* SdrCaptionObj::Clone() const
{
    return CloneHelper< SdrCaptionObj >();
}

OUString SdrCaptionObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulCAPTION));

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

OUString SdrCaptionObj::TakeObjNamePlural() const
{
    return ImpGetResStr(STR_ObjNamePluralCAPTION);
}

basegfx::B2DPolyPolygon SdrCaptionObj::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aPolyPoly(SdrRectObj::TakeXorPoly());
    aPolyPoly.append(aTailPoly.getB2DPolygon());

    return aPolyPoly;
}

sal_uInt32 SdrCaptionObj::GetHdlCount() const
{
    sal_uInt32 nAnz1(SdrRectObj::GetHdlCount());
    // Currently only dragging the tail's end is implemented.
    return nAnz1 + 1L;
}

SdrHdl* SdrCaptionObj::GetHdl(sal_uInt32 nHdlNum) const
{
    const sal_uInt32 nRectHdlAnz(SdrRectObj::GetHdlCount());

    if(nHdlNum < nRectHdlAnz)
    {
        return SdrRectObj::GetHdl(nHdlNum);
    }
    else
    {
        sal_uInt32 nPntNum(nHdlNum);
        nPntNum -= nRectHdlAnz;

        if(nPntNum < aTailPoly.GetSize())
        {
            SdrHdl* pHdl = new SdrHdl(aTailPoly.GetPoint((sal_uInt16)nPntNum), HDL_POLY);
            pHdl->SetPolyNum(1L);
            pHdl->SetPointNum(nPntNum);
            return pHdl;
        }
        else
        {
            return 0L;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrCaptionObj::hasSpecialDrag() const
{
    return true;
}

bool SdrCaptionObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetHdl();
    rDrag.SetEndDragChangesAttributes(true);
    rDrag.SetEndDragChangesGeoAndAttributes(true);

    if(pHdl && 0 == pHdl->GetPolyNum())
    {
        return SdrRectObj::beginSpecialDrag(rDrag);
    }
    else
    {
        rDrag.SetOrtho8Possible(true);

        if(!pHdl)
        {
            if (bMovProt)
                return 0;

            rDrag.SetNoSnap(true);
            rDrag.SetActionRect(aRect);

            Point aHit(rDrag.GetStart());

            if(rDrag.GetPageView() && SdrObjectPrimitiveHit(*this, aHit, 0, *rDrag.GetPageView(), 0, false))
            {
                return true;
            }
        }
        else
        {
            if((1 == pHdl->GetPolyNum()) && (0 == pHdl->GetPointNum()))
                return true;
        }
    }

    return false;
}

bool SdrCaptionObj::applySpecialDrag(SdrDragStat& rDrag)
{
    const SdrHdl* pHdl = rDrag.GetHdl();

    if(pHdl && 0 == pHdl->GetPolyNum())
    {
        const bool bRet(SdrRectObj::applySpecialDrag(rDrag));
        ImpRecalcTail();
        ActionChanged();

        return bRet;
    }
    else
    {
        Point aDelt(rDrag.GetNow()-rDrag.GetStart());

        if(!pHdl)
        {
            aRect.Move(aDelt.X(),aDelt.Y());
        }
        else
        {
            aTailPoly[0] += aDelt;
        }

        ImpRecalcTail();
        ActionChanged();

        return true;
    }
}

OUString SdrCaptionObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    const bool bCreateComment(rDrag.GetView() && this == rDrag.GetView()->GetCreateObj());

    if(bCreateComment)
    {
        return OUString();
    }
    else
    {
        const SdrHdl* pHdl = rDrag.GetHdl();

        if(pHdl && 0 == pHdl->GetPolyNum())
        {
            return SdrRectObj::getSpecialDragComment(rDrag);
        }
        else
        {
            OUString aStr;

            if(!pHdl)
            {
                ImpTakeDescriptionStr(STR_DragCaptFram, aStr);
            }
            else
            {
                ImpTakeDescriptionStr(STR_DragCaptTail, aStr);
            }

            return aStr;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrCaptionObj::ImpGetCaptParams(ImpCaptParams& rPara) const
{
    const SfxItemSet& rSet = GetObjectItemSet();
    rPara.eType      =((SdrCaptionTypeItem&)      (rSet.Get(SDRATTR_CAPTIONTYPE      ))).GetValue();
    rPara.bFixedAngle=((SdrCaptionFixedAngleItem&)(rSet.Get(SDRATTR_CAPTIONANGLE     ))).GetValue();
    rPara.nAngle     =((SdrCaptionAngleItem&)     (rSet.Get(SDRATTR_CAPTIONFIXEDANGLE))).GetValue();
    rPara.nGap       =((SdrCaptionGapItem&)       (rSet.Get(SDRATTR_CAPTIONGAP       ))).GetValue();
    rPara.eEscDir    =((SdrCaptionEscDirItem&)    (rSet.Get(SDRATTR_CAPTIONESCDIR    ))).GetValue();
    rPara.bEscRel    =((SdrCaptionEscIsRelItem&)  (rSet.Get(SDRATTR_CAPTIONESCISREL  ))).GetValue();
    rPara.nEscRel    =((SdrCaptionEscRelItem&)    (rSet.Get(SDRATTR_CAPTIONESCREL    ))).GetValue();
    rPara.nEscAbs    =((SdrCaptionEscAbsItem&)    (rSet.Get(SDRATTR_CAPTIONESCABS    ))).GetValue();
    rPara.nLineLen   =((SdrCaptionLineLenItem&)   (rSet.Get(SDRATTR_CAPTIONLINELEN   ))).GetValue();
    rPara.bFitLineLen=((SdrCaptionFitLineLenItem&)(rSet.Get(SDRATTR_CAPTIONFITLINELEN))).GetValue();
}

void SdrCaptionObj::ImpRecalcTail()
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    ImpCalcTail(aPara,aTailPoly,aRect);
    SetRectsDirty();
    SetXPolyDirty();
}

// #i35971#
// SdrCaptionObj::ImpCalcTail1 does move the object(!). What a hack.
// I really wonder why this had not triggered problems before. I am
// sure there are some places where SetTailPos() is called at least
// twice or SetSnapRect after it again just to work around this.
// Changed this method to not do that.
// Also found why this has been done: For interactive dragging of the
// tail end pos for SDRCAPT_TYPE1. This sure was the simplest method
// to achieve this, at the cost of making a whole group of const methods
// of this object implicitly change the object's position.
void SdrCaptionObj::ImpCalcTail1(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
{
    Polygon aPol(2);
    Point aTl(rPoly[0]);

    aPol[0] = aTl;
    aPol[1] = aTl;

    EscDir eEscDir;
    Point aEscPos;

    rPara.CalcEscPos(aTl, rRect, aEscPos, eEscDir);
    aPol[1] = aEscPos;

    if(eEscDir==LKS || eEscDir==RTS)
    {
        aPol[0].X() = aEscPos.X();
    }
    else
    {
        aPol[0].Y() = aEscPos.Y();
    }

    rPoly = aPol;
}

void SdrCaptionObj::ImpCalcTail2(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
{ // Gap/EscDir/EscPos/Angle
    Polygon aPol(2);
    Point aTl(rPoly[0]);
    aPol[0]=aTl;

    EscDir eEscDir;
    Point aEscPos;
    rPara.CalcEscPos(aTl,rRect,aEscPos,eEscDir);
    aPol[1]=aEscPos;

    if (!rPara.bFixedAngle) {
        // TODO: Implementation missing.
    }
    rPoly=aPol;
}

void SdrCaptionObj::ImpCalcTail3(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
{ // Gap/EscDir/EscPos/Angle/LineLen
    Polygon aPol(3);
    Point aTl(rPoly[0]);
    aPol[0]=aTl;

    EscDir eEscDir;
    Point aEscPos;
    rPara.CalcEscPos(aTl,rRect,aEscPos,eEscDir);
    aPol[1]=aEscPos;
    aPol[2]=aEscPos;

    if (eEscDir==LKS || eEscDir==RTS) {
        if (rPara.bFitLineLen) {
            aPol[1].X()=(aTl.X()+aEscPos.X())/2;
        } else {
            if (eEscDir==LKS) aPol[1].X()-=rPara.nLineLen;
            else aPol[1].X()+=rPara.nLineLen;
        }
    } else {
        if (rPara.bFitLineLen) {
            aPol[1].Y()=(aTl.Y()+aEscPos.Y())/2;
        } else {
            if (eEscDir==OBN) aPol[1].Y()-=rPara.nLineLen;
            else aPol[1].Y()+=rPara.nLineLen;
        }
    }
    if (!rPara.bFixedAngle) {
        // TODO: Implementation missing.
    }
    rPoly=aPol;
}

void SdrCaptionObj::ImpCalcTail4(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
{
    ImpCalcTail3(rPara,rPoly,rRect);
}

void SdrCaptionObj::ImpCalcTail(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
{
    switch (rPara.eType) {
        case SDRCAPT_TYPE1: ImpCalcTail1(rPara,rPoly,rRect); break;
        case SDRCAPT_TYPE2: ImpCalcTail2(rPara,rPoly,rRect); break;
        case SDRCAPT_TYPE3: ImpCalcTail3(rPara,rPoly,rRect); break;
        case SDRCAPT_TYPE4: ImpCalcTail4(rPara,rPoly,rRect); break;
    }
}

bool SdrCaptionObj::BegCreate(SdrDragStat& rStat)
{
    if (aRect.IsEmpty()) return sal_False; // Create currently only works with the given Rect

    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    aRect.SetPos(rStat.GetNow());
    aTailPoly[0]=rStat.GetStart();
    ImpCalcTail(aPara,aTailPoly,aRect);
    rStat.SetActionRect(aRect);
    return sal_True;
}

bool SdrCaptionObj::MovCreate(SdrDragStat& rStat)
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    aRect.SetPos(rStat.GetNow());
    ImpCalcTail(aPara,aTailPoly,aRect);
    rStat.SetActionRect(aRect);
    SetBoundRectDirty();
    bSnapRectDirty=sal_True;
    return sal_True;
}

bool SdrCaptionObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    aRect.SetPos(rStat.GetNow());
    ImpCalcTail(aPara,aTailPoly,aRect);
    SetRectsDirty();
    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

bool SdrCaptionObj::BckCreate(SdrDragStat& /*rStat*/)
{
    return sal_False;
}

void SdrCaptionObj::BrkCreate(SdrDragStat& /*rStat*/)
{
}

basegfx::B2DPolyPolygon SdrCaptionObj::TakeCreatePoly(const SdrDragStat& /*rDrag*/) const
{
    basegfx::B2DPolyPolygon aRetval;
    const basegfx::B2DRange aRange(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
    aRetval.append(basegfx::tools::createPolygonFromRect(aRange));
    aRetval.append(aTailPoly.getB2DPolygon());
    return aRetval;
}

Pointer SdrCaptionObj::GetCreatePointer() const
{
    return Pointer(POINTER_DRAW_CAPTION);
}

void SdrCaptionObj::NbcMove(const Size& rSiz)
{
    SdrRectObj::NbcMove(rSiz);
    MovePoly(aTailPoly,rSiz);
    if(mbFixedTail)
        SetTailPos(GetFixedTailPos());
}

void SdrCaptionObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize(rRef,xFact,yFact);
    ResizePoly(aTailPoly,rRef,xFact,yFact);
    ImpRecalcTail();
    if(mbFixedTail)
        SetTailPos(GetFixedTailPos());
}

void SdrCaptionObj::NbcSetRelativePos(const Point& rPnt)
{
    Point aRelPos0(aTailPoly.GetPoint(0)-aAnchor);
    Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
    NbcMove(aSiz); // This also calls SetRectsDirty()
}

Point SdrCaptionObj::GetRelativePos() const
{
    return aTailPoly.GetPoint(0)-aAnchor;
}

void SdrCaptionObj::NbcSetAnchorPos(const Point& rPnt)
{
    SdrRectObj::NbcSetAnchorPos(rPnt);
    // TODO: Implementation missing.
}

const Point& SdrCaptionObj::GetAnchorPos() const
{
    // TODO: Implementation missing.
    return SdrRectObj::GetAnchorPos();
}

void SdrCaptionObj::RecalcSnapRect()
{
    SdrRectObj::RecalcSnapRect();
    // #i32599#
    // TODO: Implementation missing.
}

const Rectangle& SdrCaptionObj::GetSnapRect() const
{
    return SdrRectObj::GetSnapRect();
}

void SdrCaptionObj::NbcSetSnapRect(const Rectangle& rRect)
{
    // #i32599#
    // Move back to see the rectangle of the underlying SdrRectObj
    // as the SnapRect, without the TailPos. That simplifies SnapRect
    // handling again, if not allows it at all...
    SdrRectObj::NbcSetSnapRect(rRect);
}

const Rectangle& SdrCaptionObj::GetLogicRect() const
{
    return aRect;
}

void SdrCaptionObj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetLogicRect(rRect);
    ImpRecalcTail();
}

const Point& SdrCaptionObj::GetTailPos() const
{
    return aTailPoly[0];
}

void SdrCaptionObj::SetTailPos(const Point& rPos)
{
    if (aTailPoly.GetSize()==0 || aTailPoly[0]!=rPos) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        NbcSetTailPos(rPos);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrCaptionObj::NbcSetTailPos(const Point& rPos)
{
    aTailPoly[0]=rPos;
    ImpRecalcTail();
}

sal_uInt32 SdrCaptionObj::GetSnapPointCount() const
{
    // TODO: Implementation missing.
    return 0L;
}

Point SdrCaptionObj::GetSnapPoint(sal_uInt32 /*i*/) const
{
    // TODO: Implementation missing.
    return Point(0,0);
}

void SdrCaptionObj::SetModel(SdrModel* pNewModel)
{
    SdrRectObj::SetModel(pNewModel);
    ImpRecalcTail();
}

void SdrCaptionObj::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SdrRectObj::Notify(rBC,rHint);
    ImpRecalcTail();
}

SdrObjGeoData* SdrCaptionObj::NewGeoData() const
{
    return new SdrCaptObjGeoData;
}

void SdrCaptionObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrRectObj::SaveGeoData(rGeo);
    SdrCaptObjGeoData& rCGeo=(SdrCaptObjGeoData&)rGeo;
    rCGeo.aTailPoly=aTailPoly;
}

void SdrCaptionObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::RestGeoData(rGeo);
    SdrCaptObjGeoData& rCGeo=(SdrCaptObjGeoData&)rGeo;
    aTailPoly=rCGeo.aTailPoly;
}

SdrObject* SdrCaptionObj::DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const
{
    SdrObject* pRect=SdrRectObj::DoConvertToPolyObj(bBezier, bAddText);
    SdrObject* pTail = ImpConvertMakeObj(basegfx::B2DPolyPolygon(aTailPoly.getB2DPolygon()), sal_False, bBezier);
    SdrObject* pRet=(pTail!=NULL) ? pTail : pRect;
    if (pTail!=NULL && pRect!=NULL) {
        bool bInsRect = true;
        bool bInsTail = true;
        SdrObjList* pOL=pTail->GetSubList();
        if (pOL!=NULL) { pRet=pRect; bInsTail = false; }
        if (pOL==NULL) pOL=pRect->GetSubList();
        if (pOL!=NULL) { pRet=pRect; bInsRect = false; }
        if (pOL==NULL) {
            SdrObjGroup* pGrp=new SdrObjGroup;
            pOL=pGrp->GetSubList();
            pRet=pGrp;
        }
        if (bInsRect) pOL->NbcInsertObject(pRect);
        if (bInsTail) pOL->NbcInsertObject(pTail,0);
    }
    return pRet;
}

// #i32599#
// Add own implementation for TRSetBaseGeometry to handle TailPos over changes.
void SdrCaptionObj::TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& /*rPolyPolygon*/)
{
    // break up matrix
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate, fShearX;
    rMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

    // #i75086# Old DrawingLayer (GeoStat and geometry) does not support holding negative scalings
    // in X and Y which equal a 180 degree rotation. Recognize it and react accordingly
    if(basegfx::fTools::less(aScale.getX(), 0.0) && basegfx::fTools::less(aScale.getY(), 0.0))
    {
        aScale.setX(fabs(aScale.getX()));
        aScale.setY(fabs(aScale.getY()));
        fRotate = fmod(fRotate + F_PI, F_2PI);
    }

    // force metric to pool metric
    SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                aTranslate.setX(ImplMMToTwips(aTranslate.getX()));
                aTranslate.setY(ImplMMToTwips(aTranslate.getY()));

                // size
                aScale.setX(ImplMMToTwips(aScale.getX()));
                aScale.setY(ImplMMToTwips(aScale.getY()));

                break;
            }
            default:
            {
                OSL_FAIL("TRSetBaseGeometry: Missing unit translation to PoolMetric!");
            }
        }
    }

    // if anchor is used, make position relative to it
    if( pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate += basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // build BaseRect
    Point aPoint(FRound(aTranslate.getX()), FRound(aTranslate.getY()));
    Rectangle aBaseRect(aPoint, Size(FRound(aScale.getX()), FRound(aScale.getY())));

    // set BaseRect, but rescue TailPos over this call
    const Point aTailPoint = GetTailPos();
    SetSnapRect(aBaseRect);
    SetTailPos(aTailPoint);
    ImpRecalcTail();
}

// geometry access
basegfx::B2DPolygon SdrCaptionObj::getTailPolygon() const
{
    return aTailPoly.getB2DPolygon();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
