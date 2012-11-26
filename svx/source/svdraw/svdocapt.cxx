/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <tools/bigint.hxx>
#include <svx/xlnwtit.hxx>
#include <svl/style.hxx>
#include <svx/svdocapt.hxx>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>   // fuer RectSnap
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // Objektname
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfltrit.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdr/properties/captionproperties.hxx>
#include <vcl/salbtype.hxx>     // FRound
#include <svx/sdr/contact/viewcontactofsdrcaptionobj.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/svdlegacy.hxx>

// #i32599#
inline double ImplTwipsToMM(double fVal) { return (fVal * (127.0 / 72.0)); }
inline double ImplMMToTwips(double fVal) { return (fVal * (72.0 / 127.0)); }

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
    bool                    bFitLineLen;
    bool                    bEscRel;
    bool                    bFixedAngle;

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
    Point aTl(rTailPt); // lokal kopieren wg. Performance
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

//SdrCaptionObj::SdrCaptionObj(const basegfx::B2DHomMatrix& rTransform)
//: SdrRectObj(OBJ_TEXT, rTransform),
//  aTailPoly(3),  // Default Groesse: 3 Punkte = 2 Linien
//  mbSpecialTextBoxShadow(FALSE),
//  mbFixedTail(FALSE)
//{
//}

SdrCaptionObj::SdrCaptionObj(
    SdrModel& rSdrModel,
    const basegfx::B2DHomMatrix& rTransform,
    const basegfx::B2DPoint* pTail)
:   SdrRectObj(
        rSdrModel,
        rTransform,
        OBJ_TEXT,
        true),
    aTailPoly(3),  // Default Groesse: 3 Punkte = 2 Linien
    mbSpecialTextBoxShadow(sal_False),
    mbFixedTail(sal_False)
{
    if(pTail)
    {
        aTailPoly[0] = Point(basegfx::fround(pTail->getX()), basegfx::fround(pTail->getY()));
        maFixedTailPos = *pTail;
    }
}

SdrCaptionObj::~SdrCaptionObj()
{
}

void SdrCaptionObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrCaptionObj* pSource = dynamic_cast< const SdrCaptionObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrRectObj::copyDataFromSdrObject(rSource);

            // copy local data
            aTailPoly = pSource->aTailPoly;
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SdrCaptionObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrCaptionObj* pClone = new SdrCaptionObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

void SdrCaptionObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.mbRotateFreeAllowed = false;
    rInfo.mbRotate90Allowed = false;
    rInfo.mbMirrorFreeAllowed = false;
    rInfo.mbMirror45Allowed = false;
    rInfo.mbMirror90Allowed = false;
    rInfo.mbTransparenceAllowed = false;
    rInfo.mbGradientAllowed = false;
    rInfo.mbShearAllowed = false;
    rInfo.mbEdgeRadiusAllowed = false;
    rInfo.mbCanConvToPath = true;
    rInfo.mbCanConvToPoly = true;
    rInfo.mbCanConvToPathLineToArea = false;
    rInfo.mbCanConvToPolyLineToArea = false;
    rInfo.mbCanConvToContour = (rInfo.mbCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrCaptionObj::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_CAPTION);
}

void SdrCaptionObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulCAPTION);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SdrCaptionObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralCAPTION);
}

basegfx::B2DPolyPolygon SdrCaptionObj::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aPolyPoly(SdrRectObj::TakeXorPoly());
    aPolyPoly.append(aTailPoly.getB2DPolygon());

    return aPolyPoly;
}

void SdrCaptionObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    // add parent handles
    SdrRectObj::AddToHdlList(rHdlList);

    // add own handles
    if(aTailPoly.GetSize())
        {
        const Point aOldPoint(aTailPoly.GetPoint(0));
        SdrHdl* pHdl = new SdrHdl(rHdlList, this, HDL_POLY, basegfx::B2DPoint(aOldPoint.X(), aOldPoint.Y()));
        pHdl->SetPolyNum(1);
        pHdl->SetPointNum(0);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrCaptionObj::hasSpecialDrag() const
{
    return true;
}

bool SdrCaptionObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetActiveHdl();
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
            if(mbMoveProtect)
            {
                return false;
            }

            rDrag.SetNoSnap(true);
            rDrag.SetActionRange(getObjectRange(&rDrag.GetSdrViewFromSdrDragStat()));

            const basegfx::B2DPoint aHit(rDrag.GetStart());

            if(SdrObjectPrimitiveHit(*this, aHit, 0.0, rDrag.GetSdrViewFromSdrDragStat(), false, 0))
            {
                return true;
            }
        }
        else
        {
            if((1 == pHdl->GetPolyNum()) && (0 == pHdl->GetPointNum()))
            {
                return true;
            }
        }
    }

    return false;
}

bool SdrCaptionObj::applySpecialDrag(SdrDragStat& rDrag)
{
    const SdrHdl* pHdl = rDrag.GetActiveHdl();

    if(pHdl && 0 == pHdl->GetPolyNum())
    {
        const bool bRet(SdrRectObj::applySpecialDrag(rDrag));
        ImpRecalcTail();
        ActionChanged();

        return bRet;
    }
    else
    {
        const basegfx::B2DPoint aDelta(rDrag.GetNow()-rDrag.GetStart());

        if(!pHdl)
        {
            sdr::legacy::transformSdrObject(*this, basegfx::tools::createTranslateB2DHomMatrix(aDelta));
        }
        else
        {
            const Point aOldPoint(basegfx::fround(aDelta.getX()), basegfx::fround(aDelta.getY()));
            aTailPoly[0] += aOldPoint;
        }

        ImpRecalcTail();
        ActionChanged();

        return true;
    }
}

String SdrCaptionObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    const bool bCreateComment(this == rDrag.GetSdrViewFromSdrDragStat().GetCreateObj());

    if(bCreateComment)
    {
        return String();
    }
    else
    {
        const SdrHdl* pHdl = rDrag.GetActiveHdl();

        if(pHdl && 0 == pHdl->GetPolyNum())
        {
            return SdrRectObj::getSpecialDragComment(rDrag);
        }
        else
        {
            XubString aStr;

            if(!pHdl)
            {
                TakeMarkedDescriptionString(STR_DragCaptFram, aStr);
            }
            else
            {
                TakeMarkedDescriptionString(STR_DragCaptTail, aStr);
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
    rPara.bFixedAngle=((SdrOnOffItem&)        (rSet.Get(SDRATTR_CAPTIONANGLE     ))).GetValue();
    rPara.nAngle     =((SdrAngleItem&)        (rSet.Get(SDRATTR_CAPTIONFIXEDANGLE))).GetValue();
    rPara.nGap       =((SdrMetricItem&)       (rSet.Get(SDRATTR_CAPTIONGAP       ))).GetValue();
    rPara.eEscDir    =((SdrCaptionEscDirItem&)    (rSet.Get(SDRATTR_CAPTIONESCDIR    ))).GetValue();
    rPara.bEscRel    =((SdrYesNoItem&)        (rSet.Get(SDRATTR_CAPTIONESCISREL  ))).GetValue();
    rPara.nEscRel    =((SfxInt32Item&)        (rSet.Get(SDRATTR_CAPTIONESCREL    ))).GetValue();
    rPara.nEscAbs    =((SdrMetricItem&)       (rSet.Get(SDRATTR_CAPTIONESCABS    ))).GetValue();
    rPara.nLineLen   =((SdrMetricItem&)       (rSet.Get(SDRATTR_CAPTIONLINELEN   ))).GetValue();
    rPara.bFitLineLen=((SdrYesNoItem&)        (rSet.Get(SDRATTR_CAPTIONFITLINELEN))).GetValue();
}

void SdrCaptionObj::ImpRecalcTail()
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    Rectangle aRect(sdr::legacy::GetSnapRect(*this));
    ImpCalcTail(aPara,aTailPoly,aRect);
    ActionChanged();
}

// #i35971#
// SdrCaptionObj::ImpCalcTail1 does move the object(!). What a hack.
// I really wonder why this had not triggered problems before. I am
// sure there are some places where SetTailPos() is called at least
// twice or SetSnapRect after it again just to work around this.
// Changed this method to not do that.
// Also found why this has been done: For interactive dragging of the
// tail end pos for SDRCAPT_TYPE1. This sure was the simplest method
// to achieve this, for the cost to make a whole group of const methods
// of this object implicitly chainging the object's position.
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
        // fehlende Implementation
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
        // fehlende Implementation
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
    if(getSdrObjectTransformation().isIdentity())
    {
        return false;
    }

    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);

    const basegfx::B2DRange aSnapRange(sdr::legacy::GetSnapRange(*this));
    sdr::legacy::transformSdrObject(*this, basegfx::tools::createTranslateB2DHomMatrix(rStat.GetNow() - aSnapRange.getMinimum()));

    aTailPoly[0]=Point(basegfx::fround(rStat.GetStart().getX()), basegfx::fround(rStat.GetStart().getY()));
    Rectangle aOldRect(floor(aSnapRange.getMinX()), floor(aSnapRange.getMinY()), ceil(aSnapRange.getMaxX()), ceil(aSnapRange.getMaxY()));
    ImpCalcTail(aPara, aTailPoly, aOldRect);

    rStat.SetActionRange(aSnapRange);

    return true;
}

bool SdrCaptionObj::MovCreate(SdrDragStat& rStat)
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);

    const basegfx::B2DRange aSnapRange(sdr::legacy::GetSnapRange(*this));
    sdr::legacy::transformSdrObject(*this, basegfx::tools::createTranslateB2DHomMatrix(rStat.GetNow() - aSnapRange.getMinimum()));

    Rectangle aOldRect(floor(aSnapRange.getMinX()), floor(aSnapRange.getMinY()), ceil(aSnapRange.getMaxX()), ceil(aSnapRange.getMaxY()));
    ImpCalcTail(aPara, aTailPoly, aOldRect);

    rStat.SetActionRange(aSnapRange);

    return true;
}

bool SdrCaptionObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);

    const basegfx::B2DRange aSnapRange(sdr::legacy::GetSnapRange(*this));
    sdr::legacy::transformSdrObject(*this, basegfx::tools::createTranslateB2DHomMatrix(rStat.GetNow() - aSnapRange.getMinimum()));

    Rectangle aOldRect(floor(aSnapRange.getMinX()), floor(aSnapRange.getMinY()), ceil(aSnapRange.getMaxX()), ceil(aSnapRange.getMaxY()));
    ImpCalcTail(aPara, aTailPoly, aOldRect);

    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
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
    basegfx::B2DPolyPolygon aRetval(basegfx::tools::createUnitPolygon());

    aRetval.transform(getSdrObjectTransformation());
    aRetval.append(aTailPoly.getB2DPolygon());

    return aRetval;
}

Pointer SdrCaptionObj::GetCreatePointer(const SdrView& /*rSdrView*/) const
{
    return Pointer(POINTER_DRAW_CAPTION);
}

void SdrCaptionObj::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    SdrRectObj::setSdrObjectTransformation(rTransformation);

    if(mbFixedTail)
    {
        SetTailPos(GetFixedTailPos());
    }
}

const basegfx::B2DPoint SdrCaptionObj::GetTailPos() const
{
    return basegfx::B2DPoint(aTailPoly[0].X(), aTailPoly[0].Y());
}

void SdrCaptionObj::SetTailPos(const basegfx::B2DPoint& rPos)
{
    const Point aOldPoint(basegfx::fround(rPos.getX()), basegfx::fround(rPos.getY()));

    if(!aTailPoly.GetSize() || aTailPoly[0] != aOldPoint)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

        aTailPoly[0] = aOldPoint;
        ImpRecalcTail();
        SetChanged();
    }
}

sal_uInt32 SdrCaptionObj::GetSnapPointCount() const
{
    // !!!!! fehlende Impl.
    return 0L;
}

basegfx::B2DPoint SdrCaptionObj::GetSnapPoint(sal_uInt32 /*i*/) const
{
    // !!!!! fehlende Impl.
    return basegfx::B2DPoint(0.0, 0.0);
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

SdrObject* SdrCaptionObj::DoConvertToPolygonObject(bool bBezier, bool bAddText) const
{
    SdrObject* pRect = SdrRectObj::DoConvertToPolygonObject(bBezier, bAddText);
    SdrObject* pTail = ImpConvertMakeObj(basegfx::B2DPolyPolygon(aTailPoly.getB2DPolygon()), false, bBezier);
    SdrObject* pRet= (pTail) ? pTail : pRect;

    if(pTail && pRect)
    {
        bool bInsRect(true);
        bool bInsTail(true);
        SdrObjList* pOL = pTail->getChildrenOfSdrObject();

        if(pOL)
        {
            pRet = pRect;
            bInsTail = false;
        }

        if(!pOL)
        {
            pOL = pRect->getChildrenOfSdrObject();
        }

        if(pOL)
        {
            pRet = pRect;
            bInsRect = false;
        }

        if(!pOL)
        {
            SdrObjGroup* pGrp = new SdrObjGroup(getSdrModelFromSdrObject());
            pOL = pGrp->getChildrenOfSdrObject();
            pRet = pGrp;
        }

        if(bInsRect)
        {
            pOL->InsertObjectToSdrObjList(*pRect);
        }

        if(bInsTail)
        {
            pOL->InsertObjectToSdrObjList(*pTail, 0);
        }
    }

    return pRet;
}

// geometry access
basegfx::B2DPolygon SdrCaptionObj::getTailPolygon() const
{
    return aTailPoly.getB2DPolygon();
}

// eof
