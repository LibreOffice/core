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
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <editeng/eeitem.hxx>
#include <math.h>
#include <svl/style.hxx>

#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

#include <sdr/contact/viewcontactofsdrcircobj.hxx>
#include <sdr/properties/circleproperties.hxx>
#include <svx/svdattr.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdview.hxx>
#include <svx/sxciaitm.hxx>
#include <sxcikitm.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xpool.hxx>
#include <vcl/ptrstyle.hxx>

using namespace com::sun::star;

static Point GetAnglePnt(const tools::Rectangle& rR, long nAngle)
{
    Point aCenter(rR.Center());
    long nWdt=rR.Right()-rR.Left();
    long nHgt=rR.Bottom()-rR.Top();
    long nMaxRad=(std::max(nWdt,nHgt)+1) /2;
    double a;
    a = nAngle * F_PI18000;
    Point aRetval(FRound(cos(a)*nMaxRad),-FRound(sin(a)*nMaxRad));
    if (nWdt==0) aRetval.setX(0 );
    if (nHgt==0) aRetval.setY(0 );
    if (nWdt!=nHgt) {
        if (nWdt>nHgt) {
            if (nWdt!=0) {
                // stop possible overruns for very large objects
                if (std::abs(nHgt)>32767 || std::abs(aRetval.Y())>32767) {
                    aRetval.setY(BigMulDiv(aRetval.Y(),nHgt,nWdt) );
                } else {
                    aRetval.setY(aRetval.Y()*nHgt/nWdt );
                }
            }
        } else {
            if (nHgt!=0) {
                // stop possible overruns for very large objects
                if (std::abs(nWdt)>32767 || std::abs(aRetval.X())>32767) {
                    aRetval.setX(BigMulDiv(aRetval.X(),nWdt,nHgt) );
                } else {
                    aRetval.setX(aRetval.X()*nWdt/nHgt );
                }
            }
        }
    }
    aRetval+=aCenter;
    return aRetval;
}


// BaseProperties section

std::unique_ptr<sdr::properties::BaseProperties> SdrCircObj::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::CircleProperties>(*this);
}


// DrawContact section

std::unique_ptr<sdr::contact::ViewContact> SdrCircObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfSdrCircObj>(*this);
}

SdrCircObj::SdrCircObj(
    SdrModel& rSdrModel,
    SdrObjKind eNewKind)
:   SdrRectObj(rSdrModel)
{
    nStartAngle=0;
    nEndAngle=36000;
    meCircleKind=eNewKind;
    bClosedObj=eNewKind!=OBJ_CARC;
}

SdrCircObj::SdrCircObj(
    SdrModel& rSdrModel,
    SdrObjKind eNewKind,
    const tools::Rectangle& rRect)
:   SdrRectObj(rSdrModel, rRect)
{
    nStartAngle=0;
    nEndAngle=36000;
    meCircleKind=eNewKind;
    bClosedObj=eNewKind!=OBJ_CARC;
}

SdrCircObj::SdrCircObj(
    SdrModel& rSdrModel,
    SdrObjKind eNewKind,
    const tools::Rectangle& rRect,
    long nNewStartWink,
    long nNewEndWink)
:   SdrRectObj(rSdrModel, rRect)
{
    long nAngleDif=nNewEndWink-nNewStartWink;
    nStartAngle=NormAngle36000(nNewStartWink);
    nEndAngle=NormAngle36000(nNewEndWink);
    if (nAngleDif==36000) nEndAngle+=nAngleDif; // full circle
    meCircleKind=eNewKind;
    bClosedObj=eNewKind!=OBJ_CARC;
}

SdrCircObj::~SdrCircObj()
{
}

void SdrCircObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bCanConv=!HasText() || ImpCanConvTextToCurve();
    rInfo.bEdgeRadiusAllowed    = false;
    rInfo.bCanConvToPath=bCanConv;
    rInfo.bCanConvToPoly=bCanConv;
    rInfo.bCanConvToContour = !IsFontwork() && (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrCircObj::GetObjIdentifier() const
{
    return sal_uInt16(meCircleKind);
}

bool SdrCircObj::PaintNeedsXPolyCirc() const
{
    // XPoly is necessary for all rotated ellipse objects, circle and
    // ellipse segments.
    // If not WIN, then (for now) also for circle/ellipse segments and circle/
    // ellipse arcs (for precision)
    bool bNeed=aGeo.nRotationAngle!=0 || aGeo.nShearAngle!=0 || meCircleKind==OBJ_CCUT;
    // If not WIN, then for everything except full circle (for now!)
    if (meCircleKind!=OBJ_CIRC) bNeed = true;

    const SfxItemSet& rSet = GetObjectItemSet();
    if(!bNeed)
    {
        // XPoly is necessary for everything that isn't LineSolid or LineNone
        drawing::LineStyle eLine = rSet.Get(XATTR_LINESTYLE).GetValue();
        bNeed = eLine != drawing::LineStyle_NONE && eLine != drawing::LineStyle_SOLID;

        // XPoly is necessary for thick lines
        if(!bNeed && eLine != drawing::LineStyle_NONE)
            bNeed = rSet.Get(XATTR_LINEWIDTH).GetValue() != 0;

        // XPoly is necessary for circle arcs with line ends
        if(!bNeed && meCircleKind == OBJ_CARC)
        {
            // start of the line is here if StartPolygon, StartWidth!=0
            bNeed=rSet.Get(XATTR_LINESTART).GetLineStartValue().count() != 0 &&
                  rSet.Get(XATTR_LINESTARTWIDTH).GetValue() != 0;

            if(!bNeed)
            {
                // end of the line is here if EndPolygon, EndWidth!=0
                bNeed = rSet.Get(XATTR_LINEEND).GetLineEndValue().count() != 0 &&
                        rSet.Get(XATTR_LINEENDWIDTH).GetValue() != 0;
            }
        }
    }

    // XPoly is necessary if Fill !=None and !=Solid
    if(!bNeed && meCircleKind != OBJ_CARC)
    {
        drawing::FillStyle eFill=rSet.Get(XATTR_FILLSTYLE).GetValue();
        bNeed = eFill != drawing::FillStyle_NONE && eFill != drawing::FillStyle_SOLID;
    }

    if(!bNeed && meCircleKind != OBJ_CIRC && nStartAngle == nEndAngle)
        bNeed = true; // otherwise we're drawing a full circle

    return bNeed;
}

basegfx::B2DPolygon SdrCircObj::ImpCalcXPolyCirc(const SdrObjKind eCicrleKind, const tools::Rectangle& rRect1, long nStart, long nEnd) const
{
    const basegfx::B2DRange aRange(rRect1.Left(), rRect1.Top(), rRect1.Right(), rRect1.Bottom());
    basegfx::B2DPolygon aCircPolygon;

    if(OBJ_CIRC == eCicrleKind)
    {
        // create full circle. Do not use createPolygonFromEllipse; it's necessary
        // to get the start point to the bottom of the circle to keep compatible to
        // old geometry creation
        aCircPolygon = basegfx::utils::createPolygonFromUnitCircle(1);

        // needs own scaling and translation from unit circle to target size (same as
        // would be in createPolygonFromEllipse)
        const basegfx::B2DPoint aCenter(aRange.getCenter());
        const basegfx::B2DHomMatrix aMatrix(basegfx::utils::createScaleTranslateB2DHomMatrix(
            aRange.getWidth() / 2.0, aRange.getHeight() / 2.0,
            aCenter.getX(), aCenter.getY()));
        aCircPolygon.transform(aMatrix);
    }
    else
    {
        // mirror start, end for geometry creation since model coordinate system is mirrored in Y
        // #i111715# increase numerical correctness by first dividing and not using F_PI1800
        const double fStart((((36000 - nEnd) % 36000) / 18000.0) * F_PI);
        const double fEnd((((36000 - nStart) % 36000) / 18000.0) * F_PI);

        // create circle segment. This is not closed by default
        aCircPolygon = basegfx::utils::createPolygonFromEllipseSegment(
            aRange.getCenter(), aRange.getWidth() / 2.0, aRange.getHeight() / 2.0,
            fStart, fEnd);

        // check closing states
        const bool bCloseSegment(OBJ_CARC != eCicrleKind);
        const bool bCloseUsingCenter(OBJ_SECT == eCicrleKind);

        if(bCloseSegment)
        {
            if(bCloseUsingCenter)
            {
                // add center point at start (for historical reasons)
                basegfx::B2DPolygon aSector;
                aSector.append(aRange.getCenter());
                aSector.append(aCircPolygon);
                aCircPolygon = aSector;
            }

            // close
            aCircPolygon.setClosed(true);
        }
    }

    // #i76950#
    if(aGeo.nShearAngle || aGeo.nRotationAngle)
    {
        // translate top left to (0,0)
        const basegfx::B2DPoint aTopLeft(aRange.getMinimum());
        basegfx::B2DHomMatrix aMatrix(basegfx::utils::createTranslateB2DHomMatrix(
            -aTopLeft.getX(), -aTopLeft.getY()));

        // shear, rotate and back to top left (if needed)
        aMatrix = basegfx::utils::createShearXRotateTranslateB2DHomMatrix(
            aGeo.nShearAngle ? tan((36000 - aGeo.nShearAngle) * F_PI18000) : 0.0,
            aGeo.nRotationAngle ? (36000 - aGeo.nRotationAngle) * F_PI18000 : 0.0,
            aTopLeft) * aMatrix;

        // apply transformation
        aCircPolygon.transform(aMatrix);
    }

    return aCircPolygon;
}

void SdrCircObj::RecalcXPoly()
{
    const basegfx::B2DPolygon aPolyCirc(ImpCalcXPolyCirc(meCircleKind, maRect, nStartAngle, nEndAngle));
    mpXPoly.reset( new XPolygon(aPolyCirc) );
}

OUString SdrCircObj::TakeObjNameSingul() const
{
    const char* pID=STR_ObjNameSingulCIRC;
    if (maRect.GetWidth() == maRect.GetHeight() && aGeo.nShearAngle==0)
    {
        switch (meCircleKind) {
            case OBJ_CIRC: pID=STR_ObjNameSingulCIRC; break;
            case OBJ_SECT: pID=STR_ObjNameSingulSECT; break;
            case OBJ_CARC: pID=STR_ObjNameSingulCARC; break;
            case OBJ_CCUT: pID=STR_ObjNameSingulCCUT; break;
            default: break;
        }
    } else {
        switch (meCircleKind) {
            case OBJ_CIRC: pID=STR_ObjNameSingulCIRCE; break;
            case OBJ_SECT: pID=STR_ObjNameSingulSECTE; break;
            case OBJ_CARC: pID=STR_ObjNameSingulCARCE; break;
            case OBJ_CCUT: pID=STR_ObjNameSingulCCUTE; break;
            default: break;
        }
    }
    OUStringBuffer sName(SvxResId(pID));

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

OUString SdrCircObj::TakeObjNamePlural() const
{
    const char* pID=STR_ObjNamePluralCIRC;
    if (maRect.GetWidth() == maRect.GetHeight() && aGeo.nShearAngle==0)
    {
        switch (meCircleKind) {
            case OBJ_CIRC: pID=STR_ObjNamePluralCIRC; break;
            case OBJ_SECT: pID=STR_ObjNamePluralSECT; break;
            case OBJ_CARC: pID=STR_ObjNamePluralCARC; break;
            case OBJ_CCUT: pID=STR_ObjNamePluralCCUT; break;
            default: break;
        }
    } else {
        switch (meCircleKind) {
            case OBJ_CIRC: pID=STR_ObjNamePluralCIRCE; break;
            case OBJ_SECT: pID=STR_ObjNamePluralSECTE; break;
            case OBJ_CARC: pID=STR_ObjNamePluralCARCE; break;
            case OBJ_CCUT: pID=STR_ObjNamePluralCCUTE; break;
            default: break;
        }
    }
    return SvxResId(pID);
}

SdrCircObj* SdrCircObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< SdrCircObj >(rTargetModel);
}

SdrCircObj& SdrCircObj::operator=(const SdrCircObj& rObj)
{
    if( this == &rObj )
        return *this;
    SdrRectObj::operator=(rObj);

    meCircleKind = rObj.meCircleKind;
    nStartAngle = rObj.nStartAngle;
    nEndAngle = rObj.nEndAngle;

    return *this;
}

basegfx::B2DPolyPolygon SdrCircObj::TakeXorPoly() const
{
    const basegfx::B2DPolygon aCircPolygon(ImpCalcXPolyCirc(meCircleKind, maRect, nStartAngle, nEndAngle));
    return basegfx::B2DPolyPolygon(aCircPolygon);
}

struct ImpCircUser : public SdrDragStatUserData
{
    tools::Rectangle                   aR;
    Point                       aCenter;
    Point                       aP1;
    long                        nHgt;
    long                        nWdt;
    long                        nStart;
    long                        nEnd;

public:
    ImpCircUser()
    :   nHgt(0),
        nWdt(0),
        nStart(0),
        nEnd(0)
    {}
    void SetCreateParams(SdrDragStat const & rStat);
};

sal_uInt32 SdrCircObj::GetHdlCount() const
{
    if(OBJ_CIRC != meCircleKind)
    {
        return 10L;
    }
    else
    {
        return 8L;
    }
}

void SdrCircObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    for (sal_uInt32 nHdlNum=(OBJ_CIRC==meCircleKind)?2:0; nHdlNum<=9; ++nHdlNum)
    {
        Point aPnt;
        SdrHdlKind eLocalKind(SdrHdlKind::Move);
        sal_uInt32 nPNum(0);

        switch (nHdlNum)
        {
            case 0:
                aPnt = GetAnglePnt(maRect,nStartAngle);
                eLocalKind = SdrHdlKind::Circle;
                nPNum = 1;
                break;
            case 1:
                aPnt = GetAnglePnt(maRect,nEndAngle);
                eLocalKind = SdrHdlKind::Circle;
                nPNum = 2;
                break;
            case 2:
                aPnt = maRect.TopLeft();
                eLocalKind = SdrHdlKind::UpperLeft;
                break;
            case 3:
                aPnt = maRect.TopCenter();
                eLocalKind = SdrHdlKind::Upper;
                break;
            case 4:
                aPnt = maRect.TopRight();
                eLocalKind = SdrHdlKind::UpperRight;
                break;
            case 5:
                aPnt = maRect.LeftCenter();
                eLocalKind = SdrHdlKind::Left;
                break;
            case 6:
                aPnt = maRect.RightCenter();
                eLocalKind = SdrHdlKind::Right;
                break;
            case 7:
                aPnt = maRect.BottomLeft();
                eLocalKind = SdrHdlKind::LowerLeft;
                break;
            case 8:
                aPnt = maRect.BottomCenter();
                eLocalKind = SdrHdlKind::Lower;
                break;
            case 9:
                aPnt = maRect.BottomRight();
                eLocalKind = SdrHdlKind::LowerRight;
                break;
        }

        if (aGeo.nShearAngle)
        {
            ShearPoint(aPnt,maRect.TopLeft(),aGeo.nTan);
        }

        if (aGeo.nRotationAngle)
        {
            RotatePoint(aPnt,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
        }

        std::unique_ptr<SdrHdl> pH(new SdrHdl(aPnt,eLocalKind));
        pH->SetPointNum(nPNum);
        pH->SetObj(const_cast<SdrCircObj*>(this));
        pH->SetRotationAngle(aGeo.nRotationAngle);
        rHdlList.AddHdl(std::move(pH));
    }
}


bool SdrCircObj::hasSpecialDrag() const
{
    return true;
}

bool SdrCircObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const bool bAngle(rDrag.GetHdl() && SdrHdlKind::Circle == rDrag.GetHdl()->GetKind());

    if(bAngle)
    {
        if(1 == rDrag.GetHdl()->GetPointNum() || 2 == rDrag.GetHdl()->GetPointNum())
        {
            rDrag.SetNoSnap();
        }

        return true;
    }

    return SdrTextObj::beginSpecialDrag(rDrag);
}

bool SdrCircObj::applySpecialDrag(SdrDragStat& rDrag)
{
    const bool bAngle(rDrag.GetHdl() && SdrHdlKind::Circle == rDrag.GetHdl()->GetKind());

    if(bAngle)
    {
        Point aPt(rDrag.GetNow());

        if (aGeo.nRotationAngle!=0)
            RotatePoint(aPt,maRect.TopLeft(),-aGeo.nSin,aGeo.nCos);

        if (aGeo.nShearAngle!=0)
            ShearPoint(aPt,maRect.TopLeft(),-aGeo.nTan);

        aPt -= maRect.Center();

        long nWdt = maRect.Right() - maRect.Left();
        long nHgt = maRect.Bottom() - maRect.Top();

        if(nWdt>=nHgt)
        {
            aPt.setY(BigMulDiv(aPt.Y(),nWdt,nHgt) );
        }
        else
        {
            aPt.setX(BigMulDiv(aPt.X(),nHgt,nWdt) );
        }

        long nAngle=NormAngle36000(GetAngle(aPt));

        if (rDrag.GetView() && rDrag.GetView()->IsAngleSnapEnabled())
        {
            long nSA=rDrag.GetView()->GetSnapAngle();

            if (nSA!=0)
            {
                nAngle+=nSA/2;
                nAngle/=nSA;
                nAngle*=nSA;
                nAngle=NormAngle36000(nAngle);
            }
        }

        if(1 == rDrag.GetHdl()->GetPointNum())
        {
            nStartAngle = nAngle;
        }
        else if(2 == rDrag.GetHdl()->GetPointNum())
        {
            nEndAngle = nAngle;
        }

        SetRectsDirty();
        SetXPolyDirty();
        ImpSetCircInfoToAttr();
        SetChanged();

        return true;
    }
    else
    {
        return SdrTextObj::applySpecialDrag(rDrag);
    }
}

OUString SdrCircObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    const bool bCreateComment(rDrag.GetView() && this == rDrag.GetView()->GetCreateObj());

    if(bCreateComment)
    {
        OUString aStr;
        ImpTakeDescriptionStr(STR_ViewCreateObj, aStr);
        OUStringBuffer aBuf(aStr);
        const sal_uInt32 nPointCount(rDrag.GetPointCount());

        if(OBJ_CIRC != meCircleKind && nPointCount > 2)
        {
            const ImpCircUser* pU = static_cast<const ImpCircUser*>(rDrag.GetUser());
            sal_Int32 nAngle;

            aBuf.append(" (");

            if(3 == nPointCount)
            {
                nAngle = pU->nStart;
            }
            else
            {
                nAngle = pU->nEnd;
            }

            aBuf.append(SdrModel::GetAngleString(nAngle));
            aBuf.append(')');
        }

        return aBuf.makeStringAndClear();
    }
    else
    {
        const bool bAngle(rDrag.GetHdl() && SdrHdlKind::Circle == rDrag.GetHdl()->GetKind());

        if(bAngle)
        {
            const sal_Int32 nAngle(1 == rDrag.GetHdl()->GetPointNum() ? nStartAngle : nEndAngle);

            OUString aStr;
            ImpTakeDescriptionStr(STR_DragCircAngle, aStr);
            OUStringBuffer aBuf(aStr);
            aBuf.append(" (");
            aBuf.append(SdrModel::GetAngleString(nAngle));
            aBuf.append(')');

            return aBuf.makeStringAndClear();
        }
        else
        {
            return SdrTextObj::getSpecialDragComment(rDrag);
        }
    }
}


void ImpCircUser::SetCreateParams(SdrDragStat const & rStat)
{
    rStat.TakeCreateRect(aR);
    aR.Justify();
    aCenter=aR.Center();
    nWdt=aR.Right()-aR.Left();
    nHgt=aR.Bottom()-aR.Top();
    nStart=0;
    nEnd=36000;
    if (rStat.GetPointCount()>2) {
        Point aP(rStat.GetPoint(2)-aCenter);
        if (nWdt==0) aP.setX(0 );
        if (nHgt==0) aP.setY(0 );
        if (nWdt>=nHgt) {
            if (nHgt!=0) aP.setY(aP.Y()*nWdt/nHgt );
        } else {
            if (nWdt!=0) aP.setX(aP.X()*nHgt/nWdt );
        }
        nStart=NormAngle36000(GetAngle(aP));
        if (rStat.GetView()!=nullptr && rStat.GetView()->IsAngleSnapEnabled()) {
            long nSA=rStat.GetView()->GetSnapAngle();
            if (nSA!=0) { // angle snapping
                nStart+=nSA/2;
                nStart/=nSA;
                nStart*=nSA;
                nStart=NormAngle36000(nStart);
            }
        }
        aP1 = GetAnglePnt(aR,nStart);
        nEnd=nStart;
    } else aP1=aCenter;
    if (rStat.GetPointCount()>3) {
        Point aP(rStat.GetPoint(3)-aCenter);
        if (nWdt>=nHgt) {
            aP.setY(BigMulDiv(aP.Y(),nWdt,nHgt) );
        } else {
            aP.setX(BigMulDiv(aP.X(),nHgt,nWdt) );
        }
        nEnd=NormAngle36000(GetAngle(aP));
        if (rStat.GetView()!=nullptr && rStat.GetView()->IsAngleSnapEnabled()) {
            long nSA=rStat.GetView()->GetSnapAngle();
            if (nSA!=0) { // angle snapping
                nEnd+=nSA/2;
                nEnd/=nSA;
                nEnd*=nSA;
                nEnd=NormAngle36000(nEnd);
            }
        }
    }
}

void SdrCircObj::ImpSetCreateParams(SdrDragStat& rStat)
{
    ImpCircUser* pU=static_cast<ImpCircUser*>(rStat.GetUser());
    if (pU==nullptr) {
        pU=new ImpCircUser;
        rStat.SetUser(std::unique_ptr<ImpCircUser>(pU));
    }
    pU->SetCreateParams(rStat);
}

bool SdrCircObj::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    tools::Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    maRect = aRect1;
    ImpSetCreateParams(rStat);
    return true;
}

bool SdrCircObj::MovCreate(SdrDragStat& rStat)
{
    ImpSetCreateParams(rStat);
    ImpCircUser* pU=static_cast<ImpCircUser*>(rStat.GetUser());
    rStat.SetActionRect(pU->aR);
    maRect = pU->aR; // for ObjName
    ImpJustifyRect(maRect);
    nStartAngle=pU->nStart;
    nEndAngle=pU->nEnd;
    SetBoundRectDirty();
    bSnapRectDirty=true;
    SetXPolyDirty();

    // #i103058# push current angle settings to ItemSet to
    // allow FullDrag visualisation
    if(rStat.GetPointCount() >= 4)
    {
        ImpSetCircInfoToAttr();
    }

    return true;
}

bool SdrCircObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    ImpSetCreateParams(rStat);
    ImpCircUser* pU=static_cast<ImpCircUser*>(rStat.GetUser());
    bool bRet = false;
    if (eCmd==SdrCreateCmd::ForceEnd && rStat.GetPointCount()<4) meCircleKind=OBJ_CIRC;
    if (meCircleKind==OBJ_CIRC) {
        bRet=rStat.GetPointCount()>=2;
        if (bRet) {
            maRect = pU->aR;
            ImpJustifyRect(maRect);
        }
    } else {
        rStat.SetNoSnap(rStat.GetPointCount()>=2);
        rStat.SetOrtho4Possible(rStat.GetPointCount()<2);
        bRet=rStat.GetPointCount()>=4;
        if (bRet) {
            maRect = pU->aR;
            ImpJustifyRect(maRect);
            nStartAngle=pU->nStart;
            nEndAngle=pU->nEnd;
        }
    }
    bClosedObj=meCircleKind!=OBJ_CARC;
    SetRectsDirty();
    SetXPolyDirty();
    ImpSetCircInfoToAttr();
    if (bRet)
        rStat.SetUser(nullptr);
    return bRet;
}

void SdrCircObj::BrkCreate(SdrDragStat& rStat)
{
    rStat.SetUser(nullptr);
}

bool SdrCircObj::BckCreate(SdrDragStat& rStat)
{
    rStat.SetNoSnap(rStat.GetPointCount()>=3);
    rStat.SetOrtho4Possible(rStat.GetPointCount()<3);
    return meCircleKind!=OBJ_CIRC;
}

basegfx::B2DPolyPolygon SdrCircObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    const ImpCircUser* pU = static_cast<const ImpCircUser*>(rDrag.GetUser());

    if(rDrag.GetPointCount() < 4)
    {
        // force to OBJ_CIRC to get full visualisation
        basegfx::B2DPolyPolygon aRetval(ImpCalcXPolyCirc(OBJ_CIRC, pU->aR, pU->nStart, pU->nEnd));

        if(3 == rDrag.GetPointCount())
        {
            // add edge to first point on ellipse
            basegfx::B2DPolygon aNew;

            aNew.append(basegfx::B2DPoint(pU->aCenter.X(), pU->aCenter.Y()));
            aNew.append(basegfx::B2DPoint(pU->aP1.X(), pU->aP1.Y()));
            aRetval.append(aNew);
        }

        return aRetval;
    }
    else
    {
        return basegfx::B2DPolyPolygon(ImpCalcXPolyCirc(meCircleKind, pU->aR, pU->nStart, pU->nEnd));
    }
}

PointerStyle SdrCircObj::GetCreatePointer() const
{
    switch (meCircleKind) {
        case OBJ_CIRC: return PointerStyle::DrawEllipse;
        case OBJ_SECT: return PointerStyle::DrawPie;
        case OBJ_CARC: return PointerStyle::DrawArc;
        case OBJ_CCUT: return PointerStyle::DrawCircleCut;
        default: break;
    } // switch
    return PointerStyle::Cross;
}

void SdrCircObj::NbcMove(const Size& aSiz)
{
    maRect.Move(aSiz);
    aOutRect.Move(aSiz);
    maSnapRect.Move(aSiz);
    SetXPolyDirty();
    SetRectsDirty(true);
}

void SdrCircObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    long nAngle0=aGeo.nRotationAngle;
    bool bNoShearRota=(aGeo.nRotationAngle==0 && aGeo.nShearAngle==0);
    SdrTextObj::NbcResize(rRef,xFact,yFact);
    bNoShearRota|=(aGeo.nRotationAngle==0 && aGeo.nShearAngle==0);
    if (meCircleKind!=OBJ_CIRC) {
        bool bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
        bool bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
        if (bXMirr || bYMirr) {
            // At bXMirr!=bYMirr we should actually swap both line ends.
            // That, however, is pretty bad (because of forced "hard" formatting).
            // Alternatively, we could implement a bMirrored flag (maybe even
            // a more general one, e. g. for mirrored text, ...).
            long nS0=nStartAngle;
            long nE0=nEndAngle;
            if (bNoShearRota) {
                // the RectObj already mirrors at VMirror because of a 180deg rotation
                if (! (bXMirr && bYMirr)) {
                    long nTmp=nS0;
                    nS0=18000-nE0;
                    nE0=18000-nTmp;
                }
            } else { // mirror contorted ellipses
                if (bXMirr!=bYMirr) {
                    nS0+=nAngle0;
                    nE0+=nAngle0;
                    if (bXMirr) {
                        long nTmp=nS0;
                        nS0=18000-nE0;
                        nE0=18000-nTmp;
                    }
                    if (bYMirr) {
                        long nTmp=nS0;
                        nS0=-nE0;
                        nE0=-nTmp;
                    }
                    nS0-=aGeo.nRotationAngle;
                    nE0-=aGeo.nRotationAngle;
                }
            }
            long nAngleDif=nE0-nS0;
            nStartAngle=NormAngle36000(nS0);
            nEndAngle  =NormAngle36000(nE0);
            if (nAngleDif==36000) nEndAngle+=nAngleDif; // full circle
        }
    }
    SetXPolyDirty();
    ImpSetCircInfoToAttr();
}

void SdrCircObj::NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear)
{
    SdrTextObj::NbcShear(rRef,nAngle,tn,bVShear);
    SetXPolyDirty();
    ImpSetCircInfoToAttr();
}

void SdrCircObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    bool bFreeMirr=meCircleKind!=OBJ_CIRC;
    Point aTmpPt1;
    Point aTmpPt2;
    if (bFreeMirr) { // some preparations for using an arbitrary axis of reflection
        Point aCenter(maRect.Center());
        long nWdt=maRect.GetWidth()-1;
        long nHgt=maRect.GetHeight()-1;
        long nMaxRad=(std::max(nWdt,nHgt)+1) /2;
        double a;
        // starting point
        a = nStartAngle * F_PI18000;
        aTmpPt1=Point(FRound(cos(a)*nMaxRad),-FRound(sin(a)*nMaxRad));
        if (nWdt==0) aTmpPt1.setX(0 );
        if (nHgt==0) aTmpPt1.setY(0 );
        aTmpPt1+=aCenter;
        // finishing point
        a = nEndAngle * F_PI18000;
        aTmpPt2=Point(FRound(cos(a)*nMaxRad),-FRound(sin(a)*nMaxRad));
        if (nWdt==0) aTmpPt2.setX(0 );
        if (nHgt==0) aTmpPt2.setY(0 );
        aTmpPt2+=aCenter;
        if (aGeo.nRotationAngle!=0) {
            RotatePoint(aTmpPt1,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
            RotatePoint(aTmpPt2,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
        }
        if (aGeo.nShearAngle!=0) {
            ShearPoint(aTmpPt1,maRect.TopLeft(),aGeo.nTan);
            ShearPoint(aTmpPt2,maRect.TopLeft(),aGeo.nTan);
        }
    }
    SdrTextObj::NbcMirror(rRef1,rRef2);
    if (meCircleKind!=OBJ_CIRC) { // adapt starting and finishing angle
        MirrorPoint(aTmpPt1,rRef1,rRef2);
        MirrorPoint(aTmpPt2,rRef1,rRef2);
        // unrotate:
        if (aGeo.nRotationAngle!=0) {
            RotatePoint(aTmpPt1,maRect.TopLeft(),-aGeo.nSin,aGeo.nCos); // -sin for reversion
            RotatePoint(aTmpPt2,maRect.TopLeft(),-aGeo.nSin,aGeo.nCos); // -sin for reversion
        }
        // unshear:
        if (aGeo.nShearAngle!=0) {
            ShearPoint(aTmpPt1,maRect.TopLeft(),-aGeo.nTan); // -tan for reversion
            ShearPoint(aTmpPt2,maRect.TopLeft(),-aGeo.nTan); // -tan for reversion
        }
        Point aCenter(maRect.Center());
        aTmpPt1-=aCenter;
        aTmpPt2-=aCenter;
        // because it's mirrored, the angles are swapped, too
        nStartAngle=GetAngle(aTmpPt2);
        nEndAngle  =GetAngle(aTmpPt1);
        long nAngleDif=nEndAngle-nStartAngle;
        nStartAngle=NormAngle36000(nStartAngle);
        nEndAngle  =NormAngle36000(nEndAngle);
        if (nAngleDif==36000) nEndAngle+=nAngleDif; // full circle
    }
    SetXPolyDirty();
    ImpSetCircInfoToAttr();
}

SdrObjGeoData* SdrCircObj::NewGeoData() const
{
    return new SdrCircObjGeoData;
}

void SdrCircObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrRectObj::SaveGeoData(rGeo);
    SdrCircObjGeoData& rCGeo=static_cast<SdrCircObjGeoData&>(rGeo);
    rCGeo.nStartAngle=nStartAngle;
    rCGeo.nEndAngle  =nEndAngle;
}

void SdrCircObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::RestGeoData(rGeo);
    const SdrCircObjGeoData& rCGeo=static_cast<const SdrCircObjGeoData&>(rGeo);
    nStartAngle=rCGeo.nStartAngle;
    nEndAngle  =rCGeo.nEndAngle;
    SetXPolyDirty();
    ImpSetCircInfoToAttr();
}

static void Union(tools::Rectangle& rR, const Point& rP)
{
    if (rP.X()<rR.Left  ()) rR.SetLeft(rP.X() );
    if (rP.X()>rR.Right ()) rR.SetRight(rP.X() );
    if (rP.Y()<rR.Top   ()) rR.SetTop(rP.Y() );
    if (rP.Y()>rR.Bottom()) rR.SetBottom(rP.Y() );
}

void SdrCircObj::TakeUnrotatedSnapRect(tools::Rectangle& rRect) const
{
    rRect = maRect;
    if (meCircleKind!=OBJ_CIRC) {
        const Point aPntStart(GetAnglePnt(maRect,nStartAngle));
        const Point aPntEnd(GetAnglePnt(maRect,nEndAngle));
        long a=nStartAngle;
        long e=nEndAngle;
        rRect.SetLeft(maRect.Right() );
        rRect.SetRight(maRect.Left() );
        rRect.SetTop(maRect.Bottom() );
        rRect.SetBottom(maRect.Top() );
        Union(rRect,aPntStart);
        Union(rRect,aPntEnd);
        if ((a<=18000 && e>=18000) || (a>e && (a<=18000 || e>=18000))) {
            Union(rRect,maRect.LeftCenter());
        }
        if ((a<=27000 && e>=27000) || (a>e && (a<=27000 || e>=27000))) {
            Union(rRect,maRect.BottomCenter());
        }
        if (a>e) {
            Union(rRect,maRect.RightCenter());
        }
        if ((a<=9000 && e>=9000) || (a>e && (a<=9000 || e>=9000))) {
            Union(rRect,maRect.TopCenter());
        }
        if (meCircleKind==OBJ_SECT) {
            Union(rRect,maRect.Center());
        }
        if (aGeo.nRotationAngle!=0) {
            Point aDst(rRect.TopLeft());
            aDst-=maRect.TopLeft();
            Point aDst0(aDst);
            RotatePoint(aDst,Point(),aGeo.nSin,aGeo.nCos);
            aDst-=aDst0;
            rRect.Move(aDst.X(),aDst.Y());
        }
    }
    if (aGeo.nShearAngle!=0) {
        long nDst=FRound((rRect.Bottom()-rRect.Top())*aGeo.nTan);
        if (aGeo.nShearAngle>0) {
            Point aRef(rRect.TopLeft());
            rRect.AdjustLeft( -nDst );
            Point aTmpPt(rRect.TopLeft());
            RotatePoint(aTmpPt,aRef,aGeo.nSin,aGeo.nCos);
            aTmpPt-=rRect.TopLeft();
            rRect.Move(aTmpPt.X(),aTmpPt.Y());
        } else {
            rRect.AdjustRight( -nDst );
        }
    }
}

void SdrCircObj::RecalcSnapRect()
{
    if (PaintNeedsXPolyCirc()) {
        maSnapRect=GetXPoly().GetBoundRect();
    } else {
        TakeUnrotatedSnapRect(maSnapRect);
    }
}

void SdrCircObj::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    if (aGeo.nRotationAngle!=0 || aGeo.nShearAngle!=0 || meCircleKind!=OBJ_CIRC) {
        tools::Rectangle aSR0(GetSnapRect());
        long nWdt0=aSR0.Right()-aSR0.Left();
        long nHgt0=aSR0.Bottom()-aSR0.Top();
        long nWdt1=rRect.Right()-rRect.Left();
        long nHgt1=rRect.Bottom()-rRect.Top();
        NbcResize(maSnapRect.TopLeft(),Fraction(nWdt1,nWdt0),Fraction(nHgt1,nHgt0));
        NbcMove(Size(rRect.Left()-aSR0.Left(),rRect.Top()-aSR0.Top()));
    } else {
        maRect=rRect;
        ImpJustifyRect(maRect);
    }
    SetRectsDirty();
    SetXPolyDirty();
    ImpSetCircInfoToAttr();
}

sal_uInt32 SdrCircObj::GetSnapPointCount() const
{
    if (meCircleKind==OBJ_CIRC) {
        return 1;
    } else {
        return 3;
    }
}

Point SdrCircObj::GetSnapPoint(sal_uInt32 i) const
{
    switch (i) {
        case 1 : return GetAnglePnt(maRect,nStartAngle);
        case 2 : return GetAnglePnt(maRect,nEndAngle);
        default: return maRect.Center();
    }
}

void SdrCircObj::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SetXPolyDirty();
    SdrRectObj::Notify(rBC,rHint);
    ImpSetAttrToCircInfo();
}


void SdrCircObj::ImpSetAttrToCircInfo()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    SdrCircKind eNewKindA = rSet.Get(SDRATTR_CIRCKIND).GetValue();
    SdrObjKind eNewKind = meCircleKind;

    if(eNewKindA == SDRCIRC_FULL)
        eNewKind = OBJ_CIRC;
    else if(eNewKindA == SDRCIRC_SECT)
        eNewKind = OBJ_SECT;
    else if(eNewKindA == SDRCIRC_ARC)
        eNewKind = OBJ_CARC;
    else if(eNewKindA == SDRCIRC_CUT)
        eNewKind = OBJ_CCUT;

    sal_Int32 nNewStart = rSet.Get(SDRATTR_CIRCSTARTANGLE).GetValue();
    sal_Int32 nNewEnd = rSet.Get(SDRATTR_CIRCENDANGLE).GetValue();

    bool bKindChg = meCircleKind != eNewKind;
    bool bAngleChg = nNewStart != nStartAngle || nNewEnd != nEndAngle;

    if(bKindChg || bAngleChg)
    {
        meCircleKind = eNewKind;
        nStartAngle = nNewStart;
        nEndAngle = nNewEnd;

        if(bKindChg || (meCircleKind != OBJ_CIRC && bAngleChg))
        {
            SetXPolyDirty();
            SetRectsDirty();
        }
    }
}

void SdrCircObj::ImpSetCircInfoToAttr()
{
    SdrCircKind eNewKindA = SDRCIRC_FULL;
    const SfxItemSet& rSet = GetObjectItemSet();

    if(meCircleKind == OBJ_SECT)
        eNewKindA = SDRCIRC_SECT;
    else if(meCircleKind == OBJ_CARC)
        eNewKindA = SDRCIRC_ARC;
    else if(meCircleKind == OBJ_CCUT)
        eNewKindA = SDRCIRC_CUT;

    SdrCircKind eOldKindA = rSet.Get(SDRATTR_CIRCKIND).GetValue();
    sal_Int32 nOldStartAngle = rSet.Get(SDRATTR_CIRCSTARTANGLE).GetValue();
    sal_Int32 nOldEndAngle = rSet.Get(SDRATTR_CIRCENDANGLE).GetValue();

    if(eNewKindA != eOldKindA || nStartAngle != nOldStartAngle || nEndAngle != nOldEndAngle)
    {
        // since SetItem() implicitly calls ImpSetAttrToCircInfo()
        // setting the item directly is necessary here.
        if(eNewKindA != eOldKindA)
        {
            GetProperties().SetObjectItemDirect(SdrCircKindItem(eNewKindA));
        }

        if(nStartAngle != nOldStartAngle)
        {
            GetProperties().SetObjectItemDirect(makeSdrCircStartAngleItem(nStartAngle));
        }

        if(nEndAngle != nOldEndAngle)
        {
            GetProperties().SetObjectItemDirect(makeSdrCircEndAngleItem(nEndAngle));
        }

        SetXPolyDirty();
        ImpSetAttrToCircInfo();
    }
}

SdrObject* SdrCircObj::DoConvertToPolyObj(bool bBezier, bool bAddText) const
{
    const bool bFill(meCircleKind != OBJ_CARC);
    const basegfx::B2DPolygon aCircPolygon(ImpCalcXPolyCirc(meCircleKind, maRect, nStartAngle, nEndAngle));
    SdrObject* pRet = ImpConvertMakeObj(basegfx::B2DPolyPolygon(aCircPolygon), bFill, bBezier);

    if(bAddText)
    {
        pRet = ImpConvertAddText(pRet, bBezier);
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
