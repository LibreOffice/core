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

#include <sal/config.h>

#include <cassert>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <editeng/eeitem.hxx>
#include <svl/style.hxx>
#include <tools/bigint.hxx>
#include <tools/helpers.hxx>

#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

#include <sdr/contact/viewcontactofsdrcaptionobj.hxx>
#include <sdr/properties/captionproperties.hxx>
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
#include <svx/svdview.hxx>
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
#include <o3tl/make_unique.hxx>


enum EscDir {LKS,RTS,OBN,UNT};

class ImpCaptParams
{
public:
    SdrCaptionType              eType;
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
        eType      =SdrCaptionType::Type3;
        bFixedAngle=false;
        nGap       =0;
        eEscDir    =SdrCaptionEscDir::Horizontal;
        bEscRel    =true;
        nEscRel    =5000;
        nEscAbs    =0;
        nLineLen   =0;
        bFitLineLen=true;
    }
    void CalcEscPos(const Point& rTail, const tools::Rectangle& rRect, Point& rPt, EscDir& rDir) const;
};

void ImpCaptParams::CalcEscPos(const Point& rTailPt, const tools::Rectangle& rRect, Point& rPt, EscDir& rDir) const
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
    bool bTryH=eEscDir==SdrCaptionEscDir::BestFit;
    if (!bTryH) {
        if (eType!=SdrCaptionType::Type1) {
            bTryH=eEscDir==SdrCaptionEscDir::Horizontal;
        } else {
            bTryH=eEscDir==SdrCaptionEscDir::Vertical;
        }
    }
    bool bTryV=eEscDir==SdrCaptionEscDir::BestFit;
    if (!bTryV) {
        if (eType!=SdrCaptionType::Type1) {
            bTryV=eEscDir==SdrCaptionEscDir::Vertical;
        } else {
            bTryV=eEscDir==SdrCaptionEscDir::Horizontal;
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
        bool bTakeIt=eEscDir!=SdrCaptionEscDir::BestFit;
        if (!bTakeIt) {
            BigInt aHorX(aBestPt.X()-aTl.X()); aHorX*=aHorX;
            BigInt aHorY(aBestPt.Y()-aTl.Y()); aHorY*=aHorY;
            BigInt aVerX(aBest2.X()-aTl.X());  aVerX*=aVerX;
            BigInt aVerY(aBest2.Y()-aTl.Y());  aVerY*=aVerY;
            if (eType!=SdrCaptionType::Type1) {
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


// BaseProperties section

std::unique_ptr<sdr::properties::BaseProperties> SdrCaptionObj::CreateObjectSpecificProperties()
{
    return o3tl::make_unique<sdr::properties::CaptionProperties>(*this);
}


// DrawContact section

std::unique_ptr<sdr::contact::ViewContact> SdrCaptionObj::CreateObjectSpecificViewContact()
{
    return o3tl::make_unique<sdr::contact::ViewContactOfSdrCaptionObj>(*this);
}


SdrCaptionObj::SdrCaptionObj(SdrModel& rSdrModel)
:   SdrRectObj(rSdrModel, OBJ_TEXT),
    aTailPoly(3),  // default size: 3 points = 2 lines
    mbSpecialTextBoxShadow(false),
    mbFixedTail(false),
    mbSuppressGetBitmap(false),
    maFixedTailPos()
{
}

SdrCaptionObj::SdrCaptionObj(
    SdrModel& rSdrModel,
    const tools::Rectangle& rRect,
    const Point& rTail)
:   SdrRectObj(rSdrModel, OBJ_TEXT,rRect),
    aTailPoly(3),  // default size: 3 points = 2 lines
    mbSpecialTextBoxShadow(false),
    mbFixedTail(false),
    mbSuppressGetBitmap(false),
    maFixedTailPos()
{
    aTailPoly[0]=maFixedTailPos=rTail;
}

SdrCaptionObj::~SdrCaptionObj()
{
}

void SdrCaptionObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=false;
    rInfo.bRotate90Allowed  =false;
    rInfo.bMirrorFreeAllowed=false;
    rInfo.bMirror45Allowed  =false;
    rInfo.bMirror90Allowed  =false;
    rInfo.bTransparenceAllowed = false;
    rInfo.bShearAllowed     =false;
    rInfo.bEdgeRadiusAllowed=false;
    rInfo.bCanConvToPath    =true;
    rInfo.bCanConvToPoly    =true;
    rInfo.bCanConvToPathLineToArea=false;
    rInfo.bCanConvToPolyLineToArea=false;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrCaptionObj::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_CAPTION);
}

SdrCaptionObj* SdrCaptionObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< SdrCaptionObj >(rTargetModel);
}

SdrCaptionObj& SdrCaptionObj::operator=(const SdrCaptionObj& rObj)
{
    if( this == &rObj )
        return *this;
    SdrRectObj::operator=(rObj);

    aTailPoly = rObj.aTailPoly;
    mbSpecialTextBoxShadow = rObj.mbSpecialTextBoxShadow;
    mbFixedTail = rObj.mbFixedTail;
    maFixedTailPos = rObj.maFixedTailPos;

    return *this;
}

OUString SdrCaptionObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(SvxResId(STR_ObjNameSingulCAPTION));

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
    return SvxResId(STR_ObjNamePluralCAPTION);
}

basegfx::B2DPolyPolygon SdrCaptionObj::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aPolyPoly(SdrRectObj::TakeXorPoly());
    aPolyPoly.append(aTailPoly.getB2DPolygon());

    return aPolyPoly;
}

sal_uInt32 SdrCaptionObj::GetHdlCount() const
{
    sal_uInt32 nCount1(SdrRectObj::GetHdlCount());
    // Currently only dragging the tail's end is implemented.
    return nCount1 + 1;
}

SdrHdl* SdrCaptionObj::GetHdl(sal_uInt32 nHdlNum) const
{
    const sal_uInt32 nRectHdlCnt(SdrRectObj::GetHdlCount());

    if(nHdlNum < nRectHdlCnt)
    {
        return SdrRectObj::GetHdl(nHdlNum);
    }
    else
    {
        sal_uInt32 nPntNum(nHdlNum);
        nPntNum -= nRectHdlCnt;

        if(nPntNum < aTailPoly.GetSize())
        {
            SdrHdl* pHdl = new SdrHdl(aTailPoly.GetPoint(static_cast<sal_uInt16>(nPntNum)), SdrHdlKind::Poly);
            pHdl->SetPolyNum(1);
            pHdl->SetPointNum(nPntNum);
            return pHdl;
        }
        else
        {
            return nullptr;
        }
    }
}


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
        rDrag.SetOrtho8Possible();

        if(!pHdl)
        {
            if (bMovProt)
                return false;

            rDrag.SetNoSnap();
            rDrag.SetActionRect(maRect);

            Point aHit(rDrag.GetStart());

            if(rDrag.GetPageView() && SdrObjectPrimitiveHit(*this, aHit, 0, *rDrag.GetPageView(), nullptr, false))
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
            maRect.Move(aDelt.X(),aDelt.Y());
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


void SdrCaptionObj::ImpGetCaptParams(ImpCaptParams& rPara) const
{
    const SfxItemSet& rSet = GetObjectItemSet();
    rPara.eType      =rSet.Get(SDRATTR_CAPTIONTYPE      ).GetValue();
    rPara.bFixedAngle=rSet.Get(SDRATTR_CAPTIONFIXEDANGLE).GetValue();
    rPara.nGap       =static_cast<const SdrCaptionGapItem&>       (rSet.Get(SDRATTR_CAPTIONGAP       )).GetValue();
    rPara.eEscDir    =rSet.Get(SDRATTR_CAPTIONESCDIR    ).GetValue();
    rPara.bEscRel    =rSet.Get(SDRATTR_CAPTIONESCISREL  ).GetValue();
    rPara.nEscRel    =rSet.Get(SDRATTR_CAPTIONESCREL    ).GetValue();
    rPara.nEscAbs    =rSet.Get(SDRATTR_CAPTIONESCABS    ).GetValue();
    rPara.nLineLen   =rSet.Get(SDRATTR_CAPTIONLINELEN   ).GetValue();
    rPara.bFitLineLen=rSet.Get(SDRATTR_CAPTIONFITLINELEN).GetValue();
}

void SdrCaptionObj::ImpRecalcTail()
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    ImpCalcTail(aPara, aTailPoly, maRect);
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
// tail end pos for SdrCaptionType::Type1. This sure was the simplest method
// to achieve this, at the cost of making a whole group of const methods
// of this object implicitly change the object's position.
void SdrCaptionObj::ImpCalcTail1(const ImpCaptParams& rPara, tools::Polygon& rPoly, tools::Rectangle const & rRect)
{
    tools::Polygon aPol(2);
    Point aTl(rPoly[0]);

    aPol[0] = aTl;
    aPol[1] = aTl;

    EscDir eEscDir;
    Point aEscPos;

    rPara.CalcEscPos(aTl, rRect, aEscPos, eEscDir);
    aPol[1] = aEscPos;

    if(eEscDir==LKS || eEscDir==RTS)
    {
        aPol[0].setX( aEscPos.X() );
    }
    else
    {
        aPol[0].setY( aEscPos.Y() );
    }

    rPoly = aPol;
}

void SdrCaptionObj::ImpCalcTail2(const ImpCaptParams& rPara, tools::Polygon& rPoly, tools::Rectangle const & rRect)
{ // Gap/EscDir/EscPos/Angle
    tools::Polygon aPol(2);
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

void SdrCaptionObj::ImpCalcTail3(const ImpCaptParams& rPara, tools::Polygon& rPoly, tools::Rectangle const & rRect)
{ // Gap/EscDir/EscPos/Angle/LineLen
    tools::Polygon aPol(3);
    Point aTl(rPoly[0]);
    aPol[0]=aTl;

    EscDir eEscDir;
    Point aEscPos;
    rPara.CalcEscPos(aTl,rRect,aEscPos,eEscDir);
    aPol[1]=aEscPos;
    aPol[2]=aEscPos;

    if (eEscDir==LKS || eEscDir==RTS) {
        if (rPara.bFitLineLen) {
            aPol[1].setX((aTl.X()+aEscPos.X())/2 );
        } else {
            if (eEscDir==LKS) aPol[1].AdjustX( -(rPara.nLineLen) );
            else aPol[1].AdjustX(rPara.nLineLen );
        }
    } else {
        if (rPara.bFitLineLen) {
            aPol[1].setY((aTl.Y()+aEscPos.Y())/2 );
        } else {
            if (eEscDir==OBN) aPol[1].AdjustY( -(rPara.nLineLen) );
            else aPol[1].AdjustY(rPara.nLineLen );
        }
    }
    if (!rPara.bFixedAngle) {
        // TODO: Implementation missing.
    }
    rPoly=aPol;
}

void SdrCaptionObj::ImpCalcTail(const ImpCaptParams& rPara, tools::Polygon& rPoly, tools::Rectangle const & rRect)
{
    switch (rPara.eType) {
        case SdrCaptionType::Type1: ImpCalcTail1(rPara,rPoly,rRect); break;
        case SdrCaptionType::Type2: ImpCalcTail2(rPara,rPoly,rRect); break;
        case SdrCaptionType::Type3: ImpCalcTail3(rPara,rPoly,rRect); break;
        case SdrCaptionType::Type4: ImpCalcTail3(rPara,rPoly,rRect); break;
    }
}

bool SdrCaptionObj::BegCreate(SdrDragStat& rStat)
{
    if (maRect.IsEmpty()) return false; // Create currently only works with the given Rect

    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    maRect.SetPos(rStat.GetNow());
    aTailPoly[0]=rStat.GetStart();
    ImpCalcTail(aPara,aTailPoly,maRect);
    rStat.SetActionRect(maRect);
    return true;
}

bool SdrCaptionObj::MovCreate(SdrDragStat& rStat)
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    maRect.SetPos(rStat.GetNow());
    ImpCalcTail(aPara,aTailPoly,maRect);
    rStat.SetActionRect(maRect);
    SetBoundRectDirty();
    bSnapRectDirty=true;
    return true;
}

bool SdrCaptionObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    maRect.SetPos(rStat.GetNow());
    ImpCalcTail(aPara,aTailPoly,maRect);
    SetRectsDirty();
    return (eCmd==SdrCreateCmd::ForceEnd || rStat.GetPointCount()>=2);
}

bool SdrCaptionObj::BckCreate(SdrDragStat& /*rStat*/)
{
    return false;
}

void SdrCaptionObj::BrkCreate(SdrDragStat& /*rStat*/)
{
}

basegfx::B2DPolyPolygon SdrCaptionObj::TakeCreatePoly(const SdrDragStat& /*rDrag*/) const
{
    basegfx::B2DPolyPolygon aRetval;
    const basegfx::B2DRange aRange(maRect.Left(), maRect.Top(), maRect.Right(), maRect.Bottom());
    aRetval.append(basegfx::utils::createPolygonFromRect(aRange));
    aRetval.append(aTailPoly.getB2DPolygon());
    return aRetval;
}

Pointer SdrCaptionObj::GetCreatePointer() const
{
    return Pointer(PointerStyle::DrawCaption);
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

const tools::Rectangle& SdrCaptionObj::GetLogicRect() const
{
    return maRect;
}

void SdrCaptionObj::NbcSetLogicRect(const tools::Rectangle& rRect)
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
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcSetTailPos(rPos);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
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
    SdrCaptObjGeoData& rCGeo=static_cast<SdrCaptObjGeoData&>(rGeo);
    rCGeo.aTailPoly=aTailPoly;
}

void SdrCaptionObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::RestGeoData(rGeo);
    const SdrCaptObjGeoData& rCGeo=static_cast<const SdrCaptObjGeoData&>(rGeo);
    aTailPoly=rCGeo.aTailPoly;
}

SdrObject* SdrCaptionObj::DoConvertToPolyObj(bool bBezier, bool bAddText) const
{
    SdrObject* pRect=SdrRectObj::DoConvertToPolyObj(bBezier, bAddText);
    SdrObject* pTail = ImpConvertMakeObj(basegfx::B2DPolyPolygon(aTailPoly.getB2DPolygon()), false, bBezier);
    SdrObject* pRet=(pTail!=nullptr) ? pTail : pRect;
    if (pTail!=nullptr && pRect!=nullptr) {
        bool bInsRect = true;
        bool bInsTail = true;
        SdrObjList* pOL=pTail->GetSubList();
        if (pOL!=nullptr) { pRet=pRect; bInsTail = false; }
        if (pOL==nullptr) pOL=pRect->GetSubList();
        if (pOL!=nullptr) { pRet=pRect; bInsRect = false; }
        if (pOL==nullptr)
        {
            SdrObjGroup* pGrp = new SdrObjGroup(getSdrModelFromSdrObject());
            pOL=pGrp->GetSubList();
            pRet=pGrp;
        }
        if (bInsRect) pOL->NbcInsertObject(pRect);
        if (bInsTail) pOL->NbcInsertObject(pTail,0);
    }
    return pRet;
}

namespace {

void handleNegativeScale(basegfx::B2DTuple & scale, double * rotate) {
    assert(rotate != nullptr);

    // #i75086# Old DrawingLayer (GeoStat and geometry) does not support holding negative scalings
    // in X and Y which equal a 180 degree rotation. Recognize it and react accordingly
    if(basegfx::fTools::less(scale.getX(), 0.0) && basegfx::fTools::less(scale.getY(), 0.0))
    {
        scale.setX(fabs(scale.getX()));
        scale.setY(fabs(scale.getY()));
        *rotate = fmod(*rotate + F_PI, F_2PI);
    }
}

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

    handleNegativeScale(aScale, &fRotate);

    // if anchor is used, make position relative to it
    if(getSdrModelFromSdrObject().IsWriter())
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate += basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // build BaseRect
    Point aPoint(FRound(aTranslate.getX()), FRound(aTranslate.getY()));
    tools::Rectangle aBaseRect(aPoint, Size(FRound(aScale.getX()), FRound(aScale.getY())));

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
