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

#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // the object's name

#include <svx/sdr/contact/viewcontactofsdrcircobj.hxx>
#include <svx/sdr/properties/circleproperties.hxx>
#include <svx/svdattr.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx> // for the object conversion
#include <svx/svdpage.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdview.hxx>  // for dragging (Ortho)
#include <svx/sxciaitm.hxx>
#include <svx/sxcikitm.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xpool.hxx>

//////////////////////////////////////////////////////////////////////////////

Point GetWinkPnt(const Rectangle& rR, long nWink)
{
    Point aCenter(rR.Center());
    long nWdt=rR.Right()-rR.Left();
    long nHgt=rR.Bottom()-rR.Top();
    long nMaxRad=((nWdt>nHgt ? nWdt : nHgt)+1) /2;
    double a;
    a=nWink*nPi180;
    Point aRetval(Round(cos(a)*nMaxRad),-Round(sin(a)*nMaxRad));
    if (nWdt==0) aRetval.X()=0;
    if (nHgt==0) aRetval.Y()=0;
    if (nWdt!=nHgt) {
        if (nWdt>nHgt) {
            if (nWdt!=0) {
                // stop possible overruns for very large objects
                if (std::abs(nHgt)>32767 || std::abs(aRetval.Y())>32767) {
                    aRetval.Y()=BigMulDiv(aRetval.Y(),nHgt,nWdt);
                } else {
                    aRetval.Y()=aRetval.Y()*nHgt/nWdt;
                }
            }
        } else {
            if (nHgt!=0) {
                // stop possible overruns for very large objects
                if (std::abs(nWdt)>32767 || std::abs(aRetval.X())>32767) {
                    aRetval.X()=BigMulDiv(aRetval.X(),nWdt,nHgt);
                } else {
                    aRetval.X()=aRetval.X()*nWdt/nHgt;
                }
            }
        }
    }
    aRetval+=aCenter;
    return aRetval;
}

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrCircObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::CircleProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrCircObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrCircObj(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrCircObj,SdrRectObj);

SdrCircObj::SdrCircObj(SdrObjKind eNewKind)
{
    nStartWink=0;
    nEndWink=36000;
    meCircleKind=eNewKind;
    bClosedObj=eNewKind!=OBJ_CARC;
}

SdrCircObj::SdrCircObj(SdrObjKind eNewKind, const Rectangle& rRect):
    SdrRectObj(rRect)
{
    nStartWink=0;
    nEndWink=36000;
    meCircleKind=eNewKind;
    bClosedObj=eNewKind!=OBJ_CARC;
}

SdrCircObj::SdrCircObj(SdrObjKind eNewKind, const Rectangle& rRect, long nNewStartWink, long nNewEndWink):
    SdrRectObj(rRect)
{
    long nWinkDif=nNewEndWink-nNewStartWink;
    nStartWink=NormAngle360(nNewStartWink);
    nEndWink=NormAngle360(nNewEndWink);
    if (nWinkDif==36000) nEndWink+=nWinkDif; // full circle
    meCircleKind=eNewKind;
    bClosedObj=eNewKind!=OBJ_CARC;
}

SdrCircObj::~SdrCircObj()
{
}

void SdrCircObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bCanConv=!HasText() || ImpCanConvTextToCurve();
    rInfo.bEdgeRadiusAllowed    = sal_False;
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
    bool bNeed=aGeo.nDrehWink!=0 || aGeo.nShearWink!=0 || meCircleKind==OBJ_CCUT;
    // If not WIN, then for everything except full circle (for now!)
    if (meCircleKind!=OBJ_CIRC) bNeed = true;

    const SfxItemSet& rSet = GetObjectItemSet();
    if(!bNeed)
    {
        // XPoly is necessary for everything that isn't LineSolid or LineNone
        XLineStyle eLine = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
        bNeed = eLine != XLINE_NONE && eLine != XLINE_SOLID;

        // XPoly is necessary for thick lines
        if(!bNeed && eLine != XLINE_NONE)
            bNeed = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue() != 0;

        // XPoly is necessary for circle arcs with line ends
        if(!bNeed && meCircleKind == OBJ_CARC)
        {
            // start of the line is here if StartPolygon, StartWidth!=0
            bNeed=((XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetLineStartValue().count() != 0L &&
                  ((XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue() != 0;

            if(!bNeed)
            {
                // end of the line is here if EndPolygon, EndWidth!=0
                bNeed = ((XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetLineEndValue().count() != 0L &&
                        ((XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue() != 0;
            }
        }
    }

    // XPoly is necessary if Fill !=None and !=Solid
    if(!bNeed && meCircleKind != OBJ_CARC)
    {
        XFillStyle eFill=((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();
        bNeed = eFill != XFILL_NONE && eFill != XFILL_SOLID;
    }

    if(!bNeed && meCircleKind != OBJ_CIRC && nStartWink == nEndWink)
        bNeed = true; // otherwise we're drawing a full circle

    return bNeed;
}

basegfx::B2DPolygon SdrCircObj::ImpCalcXPolyCirc(const SdrObjKind eCicrleKind, const Rectangle& rRect1, long nStart, long nEnd) const
{
    const basegfx::B2DRange aRange(rRect1.Left(), rRect1.Top(), rRect1.Right(), rRect1.Bottom());
    basegfx::B2DPolygon aCircPolygon;

    if(OBJ_CIRC == eCicrleKind)
    {
        // create full circle. Do not use createPolygonFromEllipse; it's necessary
        // to get the start point to the bottom of the circle to keep compatible to
        // old geometry creation
        aCircPolygon = basegfx::tools::createPolygonFromUnitCircle(1);

        // needs own scaling and translation from unit circle to target size (same as
        // would be in createPolygonFromEllipse)
        const basegfx::B2DPoint aCenter(aRange.getCenter());
        const basegfx::B2DHomMatrix aMatrix(basegfx::tools::createScaleTranslateB2DHomMatrix(
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
        aCircPolygon = basegfx::tools::createPolygonFromEllipseSegment(
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
    if(aGeo.nShearWink || aGeo.nDrehWink)
    {
        // translate top left to (0,0)
        const basegfx::B2DPoint aTopLeft(aRange.getMinimum());
        basegfx::B2DHomMatrix aMatrix(basegfx::tools::createTranslateB2DHomMatrix(
            -aTopLeft.getX(), -aTopLeft.getY()));

        // shear, rotate and back to top left (if needed)
        aMatrix = basegfx::tools::createShearXRotateTranslateB2DHomMatrix(
            aGeo.nShearWink ? tan((36000 - aGeo.nShearWink) * F_PI18000) : 0.0,
            aGeo.nDrehWink ? (36000 - aGeo.nDrehWink) * F_PI18000 : 0.0,
            aTopLeft) * aMatrix;

        // apply transformation
        aCircPolygon.transform(aMatrix);
    }

    return aCircPolygon;
}

void SdrCircObj::RecalcXPoly()
{
    const basegfx::B2DPolygon aPolyCirc(ImpCalcXPolyCirc(meCircleKind, aRect, nStartWink, nEndWink));
    mpXPoly = new XPolygon(aPolyCirc);
}

OUString SdrCircObj::TakeObjNameSingul() const
{
    sal_uInt16 nID=STR_ObjNameSingulCIRC;
    if (aRect.GetWidth()==aRect.GetHeight() && aGeo.nShearWink==0) {
        switch (meCircleKind) {
            case OBJ_CIRC: nID=STR_ObjNameSingulCIRC; break;
            case OBJ_SECT: nID=STR_ObjNameSingulSECT; break;
            case OBJ_CARC: nID=STR_ObjNameSingulCARC; break;
            case OBJ_CCUT: nID=STR_ObjNameSingulCCUT; break;
            default: break;
        }
    } else {
        switch (meCircleKind) {
            case OBJ_CIRC: nID=STR_ObjNameSingulCIRCE; break;
            case OBJ_SECT: nID=STR_ObjNameSingulSECTE; break;
            case OBJ_CARC: nID=STR_ObjNameSingulCARCE; break;
            case OBJ_CCUT: nID=STR_ObjNameSingulCCUTE; break;
            default: break;
        }
    }
    OUStringBuffer sName(ImpGetResStr(nID));

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
    sal_uInt16 nID=STR_ObjNamePluralCIRC;
    if (aRect.GetWidth()==aRect.GetHeight() && aGeo.nShearWink==0) {
        switch (meCircleKind) {
            case OBJ_CIRC: nID=STR_ObjNamePluralCIRC; break;
            case OBJ_SECT: nID=STR_ObjNamePluralSECT; break;
            case OBJ_CARC: nID=STR_ObjNamePluralCARC; break;
            case OBJ_CCUT: nID=STR_ObjNamePluralCCUT; break;
            default: break;
        }
    } else {
        switch (meCircleKind) {
            case OBJ_CIRC: nID=STR_ObjNamePluralCIRCE; break;
            case OBJ_SECT: nID=STR_ObjNamePluralSECTE; break;
            case OBJ_CARC: nID=STR_ObjNamePluralCARCE; break;
            case OBJ_CCUT: nID=STR_ObjNamePluralCCUTE; break;
            default: break;
        }
    }
    return ImpGetResStr(nID);
}

SdrCircObj* SdrCircObj::Clone() const
{
    return CloneHelper< SdrCircObj >();
}

basegfx::B2DPolyPolygon SdrCircObj::TakeXorPoly() const
{
    const basegfx::B2DPolygon aCircPolygon(ImpCalcXPolyCirc(meCircleKind, aRect, nStartWink, nEndWink));
    return basegfx::B2DPolyPolygon(aCircPolygon);
}

struct ImpCircUser : public SdrDragStatUserData
{
    Rectangle                   aR;
    Point                       aCenter;
    Point                       aRadius;
    Point                       aP1;
    Point                       aP2;
    long                        nMaxRad;
    long                        nHgt;
    long                        nWdt;
    long                        nStart;
    long                        nEnd;
    long                        nWink;
    bool                        bRight; // not yet implemented

public:
    ImpCircUser()
    :   nMaxRad(0),
        nHgt(0),
        nWdt(0),
        nStart(0),
        nEnd(0),
        nWink(0),
        bRight(sal_False)
    {}
    void SetCreateParams(SdrDragStat& rStat);
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

SdrHdl* SdrCircObj::GetHdl(sal_uInt32 nHdlNum) const
{
    if (meCircleKind==OBJ_CIRC)
    {
        nHdlNum += 2L;
    }

    SdrHdl* pH = NULL;
    Point aPnt;
    SdrHdlKind eLocalKind(HDL_MOVE);
    sal_uInt32 nPNum(0);

    switch (nHdlNum)
    {
        case 0:
            aPnt = GetWinkPnt(aRect,nStartWink);
            eLocalKind = HDL_CIRC;
            nPNum = 1;
            break;
        case 1:
            aPnt = GetWinkPnt(aRect,nEndWink);
            eLocalKind = HDL_CIRC;
            nPNum = 2L;
            break;
        case 2:
            aPnt = aRect.TopLeft();
            eLocalKind = HDL_UPLFT;
            break;
        case 3:
            aPnt = aRect.TopCenter();
            eLocalKind = HDL_UPPER;
            break;
        case 4:
            aPnt = aRect.TopRight();
            eLocalKind = HDL_UPRGT;
            break;
        case 5:
            aPnt = aRect.LeftCenter();
            eLocalKind = HDL_LEFT;
            break;
        case 6:
            aPnt = aRect.RightCenter();
            eLocalKind = HDL_RIGHT;
            break;
        case 7:
            aPnt = aRect.BottomLeft();
            eLocalKind = HDL_LWLFT;
            break;
        case 8:
            aPnt = aRect.BottomCenter();
            eLocalKind = HDL_LOWER;
            break;
        case 9:
            aPnt = aRect.BottomRight();
            eLocalKind = HDL_LWRGT;
            break;
    }

    if (aGeo.nShearWink)
    {
        ShearPoint(aPnt,aRect.TopLeft(),aGeo.nTan);
    }

    if (aGeo.nDrehWink)
    {
        RotatePoint(aPnt,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    }

    if (eLocalKind != HDL_MOVE)
    {
        pH = new SdrHdl(aPnt,eLocalKind);
        pH->SetPointNum(nPNum);
        pH->SetObj((SdrObject*)this);
        pH->SetDrehWink(aGeo.nDrehWink);
    }

    return pH;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrCircObj::hasSpecialDrag() const
{
    return true;
}

bool SdrCircObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const bool bWink(rDrag.GetHdl() && HDL_CIRC == rDrag.GetHdl()->GetKind());

    if(bWink)
    {
        if(1 == rDrag.GetHdl()->GetPointNum() || 2 == rDrag.GetHdl()->GetPointNum())
        {
            rDrag.SetNoSnap(true);
        }

        return true;
    }

    return SdrTextObj::beginSpecialDrag(rDrag);
}

bool SdrCircObj::applySpecialDrag(SdrDragStat& rDrag)
{
    const bool bWink(rDrag.GetHdl() && HDL_CIRC == rDrag.GetHdl()->GetKind());

    if(bWink)
    {
        Point aPt(rDrag.GetNow());

        if (aGeo.nDrehWink!=0)
            RotatePoint(aPt,aRect.TopLeft(),-aGeo.nSin,aGeo.nCos);

        if (aGeo.nShearWink!=0)
            ShearPoint(aPt,aRect.TopLeft(),-aGeo.nTan);

        aPt-=aRect.Center();

        long nWdt=aRect.Right()-aRect.Left();
        long nHgt=aRect.Bottom()-aRect.Top();

        if(nWdt>=nHgt)
        {
            aPt.Y()=BigMulDiv(aPt.Y(),nWdt,nHgt);
        }
        else
        {
            aPt.X()=BigMulDiv(aPt.X(),nHgt,nWdt);
        }

        long nWink=NormAngle360(GetAngle(aPt));

        if (rDrag.GetView() && rDrag.GetView()->IsAngleSnapEnabled())
        {
            long nSA=rDrag.GetView()->GetSnapAngle();

            if (nSA!=0)
            {
                nWink+=nSA/2;
                nWink/=nSA;
                nWink*=nSA;
                nWink=NormAngle360(nWink);
            }
        }

        if(1 == rDrag.GetHdl()->GetPointNum())
        {
            nStartWink = nWink;
        }
        else if(2 == rDrag.GetHdl()->GetPointNum())
        {
            nEndWink = nWink;
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
        const sal_uInt32 nPntAnz(rDrag.GetPointAnz());

        if(OBJ_CIRC != meCircleKind && nPntAnz > 2)
        {
            ImpCircUser* pU = (ImpCircUser*)rDrag.GetUser();
            sal_Int32 nWink;

            aBuf.appendAscii(" (");

            if(3 == nPntAnz)
            {
                nWink = pU->nStart;
            }
            else
            {
                nWink = pU->nEnd;
            }

            aBuf.append(GetWinkStr(nWink,false));
            aBuf.append(sal_Unicode(')'));
        }

        return aBuf.makeStringAndClear();
    }
    else
    {
        const bool bWink(rDrag.GetHdl() && HDL_CIRC == rDrag.GetHdl()->GetKind());

        if(bWink)
        {
            const sal_Int32 nWink(1 == rDrag.GetHdl()->GetPointNum() ? nStartWink : nEndWink);

            OUString aStr;
            ImpTakeDescriptionStr(STR_DragCircAngle, aStr);
            OUStringBuffer aBuf(aStr);
            aBuf.appendAscii(" (");
            aBuf.append(GetWinkStr(nWink,false));
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

void ImpCircUser::SetCreateParams(SdrDragStat& rStat)
{
    rStat.TakeCreateRect(aR);
    aR.Justify();
    aCenter=aR.Center();
    nWdt=aR.Right()-aR.Left();
    nHgt=aR.Bottom()-aR.Top();
    nMaxRad=((nWdt>nHgt ? nWdt : nHgt)+1) /2;
    nStart=0;
    nEnd=36000;
    if (rStat.GetPointAnz()>2) {
        Point aP(rStat.GetPoint(2)-aCenter);
        if (nWdt==0) aP.X()=0;
        if (nHgt==0) aP.Y()=0;
        if (nWdt>=nHgt) {
            if (nHgt!=0) aP.Y()=aP.Y()*nWdt/nHgt;
        } else {
            if (nWdt!=0) aP.X()=aP.X()*nHgt/nWdt;
        }
        nStart=NormAngle360(GetAngle(aP));
        if (rStat.GetView()!=NULL && rStat.GetView()->IsAngleSnapEnabled()) {
            long nSA=rStat.GetView()->GetSnapAngle();
            if (nSA!=0) { // angle snapping
                nStart+=nSA/2;
                nStart/=nSA;
                nStart*=nSA;
                nStart=NormAngle360(nStart);
            }
        }
        aP1 = GetWinkPnt(aR,nStart);
        nEnd=nStart;
        aP2=aP1;
    } else aP1=aCenter;
    if (rStat.GetPointAnz()>3) {
        Point aP(rStat.GetPoint(3)-aCenter);
        if (nWdt>=nHgt) {
            aP.Y()=BigMulDiv(aP.Y(),nWdt,nHgt);
        } else {
            aP.X()=BigMulDiv(aP.X(),nHgt,nWdt);
        }
        nEnd=NormAngle360(GetAngle(aP));
        if (rStat.GetView()!=NULL && rStat.GetView()->IsAngleSnapEnabled()) {
            long nSA=rStat.GetView()->GetSnapAngle();
            if (nSA!=0) { // angle snapping
                nEnd+=nSA/2;
                nEnd/=nSA;
                nEnd*=nSA;
                nEnd=NormAngle360(nEnd);
            }
        }
        aP2 = GetWinkPnt(aR,nEnd);
    } else aP2=aCenter;
}

void SdrCircObj::ImpSetCreateParams(SdrDragStat& rStat) const
{
    ImpCircUser* pU=(ImpCircUser*)rStat.GetUser();
    if (pU==NULL) {
        pU=new ImpCircUser;
        rStat.SetUser(pU);
    }
    pU->SetCreateParams(rStat);
}

bool SdrCircObj::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    aRect = aRect1;
    ImpSetCreateParams(rStat);
    return sal_True;
}

bool SdrCircObj::MovCreate(SdrDragStat& rStat)
{
    ImpSetCreateParams(rStat);
    ImpCircUser* pU=(ImpCircUser*)rStat.GetUser();
    rStat.SetActionRect(pU->aR);
    aRect=pU->aR; // for ObjName
    ImpJustifyRect(aRect);
    nStartWink=pU->nStart;
    nEndWink=pU->nEnd;
    SetBoundRectDirty();
    bSnapRectDirty=sal_True;
    SetXPolyDirty();

    // #i103058# push current angle settings to ItemSet to
    // allow FullDrag visualisation
    if(rStat.GetPointAnz() >= 4)
    {
        ImpSetCircInfoToAttr();
    }

    return sal_True;
}

bool SdrCircObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    ImpSetCreateParams(rStat);
    ImpCircUser* pU=(ImpCircUser*)rStat.GetUser();
    bool bRet = false;
    if (eCmd==SDRCREATE_FORCEEND && rStat.GetPointAnz()<4) meCircleKind=OBJ_CIRC;
    if (meCircleKind==OBJ_CIRC) {
        bRet=rStat.GetPointAnz()>=2;
        if (bRet) {
            aRect=pU->aR;
            ImpJustifyRect(aRect);
        }
    } else {
        rStat.SetNoSnap(rStat.GetPointAnz()>=2);
        rStat.SetOrtho4Possible(rStat.GetPointAnz()<2);
        bRet=rStat.GetPointAnz()>=4;
        if (bRet) {
            aRect=pU->aR;
            ImpJustifyRect(aRect);
            nStartWink=pU->nStart;
            nEndWink=pU->nEnd;
        }
    }
    bClosedObj=meCircleKind!=OBJ_CARC;
    SetRectsDirty();
    SetXPolyDirty();
    ImpSetCircInfoToAttr();
    if (bRet) {
        delete pU;
        rStat.SetUser(NULL);
    }
    return bRet;
}

void SdrCircObj::BrkCreate(SdrDragStat& rStat)
{
    ImpCircUser* pU=(ImpCircUser*)rStat.GetUser();
    delete pU;
    rStat.SetUser(NULL);
}

bool SdrCircObj::BckCreate(SdrDragStat& rStat)
{
    rStat.SetNoSnap(rStat.GetPointAnz()>=3);
    rStat.SetOrtho4Possible(rStat.GetPointAnz()<3);
    return meCircleKind!=OBJ_CIRC;
}

basegfx::B2DPolyPolygon SdrCircObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    ImpCircUser* pU = (ImpCircUser*)rDrag.GetUser();

    if(rDrag.GetPointAnz() < 4L)
    {
        // force to OBJ_CIRC to get full visualisation
        basegfx::B2DPolyPolygon aRetval(ImpCalcXPolyCirc(OBJ_CIRC, pU->aR, pU->nStart, pU->nEnd));

        if(3L == rDrag.GetPointAnz())
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

Pointer SdrCircObj::GetCreatePointer() const
{
    switch (meCircleKind) {
        case OBJ_CIRC: return Pointer(POINTER_DRAW_ELLIPSE);
        case OBJ_SECT: return Pointer(POINTER_DRAW_PIE);
        case OBJ_CARC: return Pointer(POINTER_DRAW_ARC);
        case OBJ_CCUT: return Pointer(POINTER_DRAW_CIRCLECUT);
        default: break;
    } // switch
    return Pointer(POINTER_CROSS);
}

void SdrCircObj::NbcMove(const Size& aSiz)
{
    MoveRect(aRect,aSiz);
    MoveRect(aOutRect,aSiz);
    MoveRect(maSnapRect,aSiz);
    SetXPolyDirty();
    SetRectsDirty(sal_True);
}

void SdrCircObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    long nWink0=aGeo.nDrehWink;
    bool bNoShearRota=(aGeo.nDrehWink==0 && aGeo.nShearWink==0);
    SdrTextObj::NbcResize(rRef,xFact,yFact);
    bNoShearRota|=(aGeo.nDrehWink==0 && aGeo.nShearWink==0);
    if (meCircleKind!=OBJ_CIRC) {
        bool bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
        bool bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
        if (bXMirr || bYMirr) {
            // At bXMirr!=bYMirr we should actually swap both line ends.
            // That, however, is pretty bad (because of forced "hard" formatting).
            // Alternatively, we could implement a bMirrored flag (maybe even
            // a more general one, e. g. for mirrored text, ...).
            long nS0=nStartWink;
            long nE0=nEndWink;
            if (bNoShearRota) {
                // the RectObj already mirrors at VMirror because of a 180deg rotation
                if (! (bXMirr && bYMirr)) {
                    long nTmp=nS0;
                    nS0=18000-nE0;
                    nE0=18000-nTmp;
                }
            } else { // mirror contorted ellipses
                if (bXMirr!=bYMirr) {
                    nS0+=nWink0;
                    nE0+=nWink0;
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
                    nS0-=aGeo.nDrehWink;
                    nE0-=aGeo.nDrehWink;
                }
            }
            long nWinkDif=nE0-nS0;
            nStartWink=NormAngle360(nS0);
            nEndWink  =NormAngle360(nE0);
            if (nWinkDif==36000) nEndWink+=nWinkDif; // full circle
        }
    }
    SetXPolyDirty();
    ImpSetCircInfoToAttr();
}

void SdrCircObj::NbcShear(const Point& rRef, long nWink, double tn, bool bVShear)
{
    SdrTextObj::NbcShear(rRef,nWink,tn,bVShear);
    SetXPolyDirty();
    ImpSetCircInfoToAttr();
}

void SdrCircObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    bool bFreeMirr=meCircleKind!=OBJ_CIRC;
    Point aTmpPt1;
    Point aTmpPt2;
    if (bFreeMirr) { // some preparations for using an arbitrary axis of reflection
        Point aCenter(aRect.Center());
        long nWdt=aRect.GetWidth()-1;
        long nHgt=aRect.GetHeight()-1;
        long nMaxRad=((nWdt>nHgt ? nWdt : nHgt)+1) /2;
        double a;
        // starting point
        a=nStartWink*nPi180;
        aTmpPt1=Point(Round(cos(a)*nMaxRad),-Round(sin(a)*nMaxRad));
        if (nWdt==0) aTmpPt1.X()=0;
        if (nHgt==0) aTmpPt1.Y()=0;
        aTmpPt1+=aCenter;
        // finishing point
        a=nEndWink*nPi180;
        aTmpPt2=Point(Round(cos(a)*nMaxRad),-Round(sin(a)*nMaxRad));
        if (nWdt==0) aTmpPt2.X()=0;
        if (nHgt==0) aTmpPt2.Y()=0;
        aTmpPt2+=aCenter;
        if (aGeo.nDrehWink!=0) {
            RotatePoint(aTmpPt1,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
            RotatePoint(aTmpPt2,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
        }
        if (aGeo.nShearWink!=0) {
            ShearPoint(aTmpPt1,aRect.TopLeft(),aGeo.nTan);
            ShearPoint(aTmpPt2,aRect.TopLeft(),aGeo.nTan);
        }
    }
    SdrTextObj::NbcMirror(rRef1,rRef2);
    if (meCircleKind!=OBJ_CIRC) { // adapt starting and finishing angle
        MirrorPoint(aTmpPt1,rRef1,rRef2);
        MirrorPoint(aTmpPt2,rRef1,rRef2);
        // unrotate:
        if (aGeo.nDrehWink!=0) {
            RotatePoint(aTmpPt1,aRect.TopLeft(),-aGeo.nSin,aGeo.nCos); // -sin for reversion
            RotatePoint(aTmpPt2,aRect.TopLeft(),-aGeo.nSin,aGeo.nCos); // -sin for reversion
        }
        // unshear:
        if (aGeo.nShearWink!=0) {
            ShearPoint(aTmpPt1,aRect.TopLeft(),-aGeo.nTan); // -tan for reversion
            ShearPoint(aTmpPt2,aRect.TopLeft(),-aGeo.nTan); // -tan for reversion
        }
        Point aCenter(aRect.Center());
        aTmpPt1-=aCenter;
        aTmpPt2-=aCenter;
        // because it's mirrored, the angles are swapped, too
        nStartWink=GetAngle(aTmpPt2);
        nEndWink  =GetAngle(aTmpPt1);
        long nWinkDif=nEndWink-nStartWink;
        nStartWink=NormAngle360(nStartWink);
        nEndWink  =NormAngle360(nEndWink);
        if (nWinkDif==36000) nEndWink+=nWinkDif; // full circle
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
    SdrCircObjGeoData& rCGeo=(SdrCircObjGeoData&)rGeo;
    rCGeo.nStartWink=nStartWink;
    rCGeo.nEndWink  =nEndWink;
}

void SdrCircObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::RestGeoData(rGeo);
    SdrCircObjGeoData& rCGeo=(SdrCircObjGeoData&)rGeo;
    nStartWink=rCGeo.nStartWink;
    nEndWink  =rCGeo.nEndWink;
    SetXPolyDirty();
    ImpSetCircInfoToAttr();
}

void Union(Rectangle& rR, const Point& rP)
{
    if (rP.X()<rR.Left  ()) rR.Left  ()=rP.X();
    if (rP.X()>rR.Right ()) rR.Right ()=rP.X();
    if (rP.Y()<rR.Top   ()) rR.Top   ()=rP.Y();
    if (rP.Y()>rR.Bottom()) rR.Bottom()=rP.Y();
}

void SdrCircObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
{
    rRect=aRect;
    if (meCircleKind!=OBJ_CIRC) {
        const Point aPntStart(GetWinkPnt(aRect,nStartWink));
        const Point aPntEnd(GetWinkPnt(aRect,nEndWink));
        long a=nStartWink;
        long e=nEndWink;
        rRect.Left  ()=aRect.Right();
        rRect.Right ()=aRect.Left();
        rRect.Top   ()=aRect.Bottom();
        rRect.Bottom()=aRect.Top();
        Union(rRect,aPntStart);
        Union(rRect,aPntEnd);
        if ((a<=18000 && e>=18000) || (a>e && (a<=18000 || e>=18000))) {
            Union(rRect,aRect.LeftCenter());
        }
        if ((a<=27000 && e>=27000) || (a>e && (a<=27000 || e>=27000))) {
            Union(rRect,aRect.BottomCenter());
        }
        if (a>e) {
            Union(rRect,aRect.RightCenter());
        }
        if ((a<=9000 && e>=9000) || (a>e && (a<=9000 || e>=9000))) {
            Union(rRect,aRect.TopCenter());
        }
        if (meCircleKind==OBJ_SECT) {
            Union(rRect,aRect.Center());
        }
        if (aGeo.nDrehWink!=0) {
            Point aDst(rRect.TopLeft());
            aDst-=aRect.TopLeft();
            Point aDst0(aDst);
            RotatePoint(aDst,Point(),aGeo.nSin,aGeo.nCos);
            aDst-=aDst0;
            rRect.Move(aDst.X(),aDst.Y());
        }
    }
    if (aGeo.nShearWink!=0) {
        long nDst=Round((rRect.Bottom()-rRect.Top())*aGeo.nTan);
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

void SdrCircObj::RecalcSnapRect()
{
    if (PaintNeedsXPolyCirc()) {
        maSnapRect=GetXPoly().GetBoundRect();
    } else {
        TakeUnrotatedSnapRect(maSnapRect);
    }
}

void SdrCircObj::NbcSetSnapRect(const Rectangle& rRect)
{
    if (aGeo.nDrehWink!=0 || aGeo.nShearWink!=0 || meCircleKind!=OBJ_CIRC) {
        Rectangle aSR0(GetSnapRect());
        long nWdt0=aSR0.Right()-aSR0.Left();
        long nHgt0=aSR0.Bottom()-aSR0.Top();
        long nWdt1=rRect.Right()-rRect.Left();
        long nHgt1=rRect.Bottom()-rRect.Top();
        NbcResize(maSnapRect.TopLeft(),Fraction(nWdt1,nWdt0),Fraction(nHgt1,nHgt0));
        NbcMove(Size(rRect.Left()-aSR0.Left(),rRect.Top()-aSR0.Top()));
    } else {
        aRect=rRect;
        ImpJustifyRect(aRect);
    }
    SetRectsDirty();
    SetXPolyDirty();
    ImpSetCircInfoToAttr();
}

sal_uInt32 SdrCircObj::GetSnapPointCount() const
{
    if (meCircleKind==OBJ_CIRC) {
        return 1L;
    } else {
        return 3L;
    }
}

Point SdrCircObj::GetSnapPoint(sal_uInt32 i) const
{
    switch (i) {
        case 1 : return GetWinkPnt(aRect,nStartWink);
        case 2 : return GetWinkPnt(aRect,nEndWink);
        default: return aRect.Center();
    }
}

void SdrCircObj::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SetXPolyDirty();
    SdrRectObj::Notify(rBC,rHint);
    ImpSetAttrToCircInfo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrCircObj::ImpSetAttrToCircInfo()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    SdrCircKind eNewKindA = ((SdrCircKindItem&)rSet.Get(SDRATTR_CIRCKIND)).GetValue();
    SdrObjKind eNewKind = meCircleKind;

    if(eNewKindA == SDRCIRC_FULL)
        eNewKind = OBJ_CIRC;
    else if(eNewKindA == SDRCIRC_SECT)
        eNewKind = OBJ_SECT;
    else if(eNewKindA == SDRCIRC_ARC)
        eNewKind = OBJ_CARC;
    else if(eNewKindA == SDRCIRC_CUT)
        eNewKind = OBJ_CCUT;

    sal_Int32 nNewStart = ((SdrCircStartAngleItem&)rSet.Get(SDRATTR_CIRCSTARTANGLE)).GetValue();
    sal_Int32 nNewEnd = ((SdrCircEndAngleItem&)rSet.Get(SDRATTR_CIRCENDANGLE)).GetValue();

    bool bKindChg = meCircleKind != eNewKind;
    bool bWinkChg = nNewStart != nStartWink || nNewEnd != nEndWink;

    if(bKindChg || bWinkChg)
    {
        meCircleKind = eNewKind;
        nStartWink = nNewStart;
        nEndWink = nNewEnd;

        if(bKindChg || (meCircleKind != OBJ_CIRC && bWinkChg))
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

    SdrCircKind eOldKindA = ((SdrCircKindItem&)rSet.Get(SDRATTR_CIRCKIND)).GetValue();
    sal_Int32 nOldStartWink = ((SdrCircStartAngleItem&)rSet.Get(SDRATTR_CIRCSTARTANGLE)).GetValue();
    sal_Int32 nOldEndWink = ((SdrCircEndAngleItem&)rSet.Get(SDRATTR_CIRCENDANGLE)).GetValue();

    if(eNewKindA != eOldKindA || nStartWink != nOldStartWink || nEndWink != nOldEndWink)
    {
        // since SetItem() implicitly calls ImpSetAttrToCircInfo()
        // setting the item directly is necessary here.
        if(eNewKindA != eOldKindA)
        {
            GetProperties().SetObjectItemDirect(SdrCircKindItem(eNewKindA));
        }

        if(nStartWink != nOldStartWink)
        {
            GetProperties().SetObjectItemDirect(SdrCircStartAngleItem(nStartWink));
        }

        if(nEndWink != nOldEndWink)
        {
            GetProperties().SetObjectItemDirect(SdrCircEndAngleItem(nEndWink));
        }

        SetXPolyDirty();
        ImpSetAttrToCircInfo();
    }
}

SdrObject* SdrCircObj::DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const
{
    const sal_Bool bFill(OBJ_CARC == meCircleKind ? sal_False : sal_True);
    const basegfx::B2DPolygon aCircPolygon(ImpCalcXPolyCirc(meCircleKind, aRect, nStartWink, nEndWink));
    SdrObject* pRet = ImpConvertMakeObj(basegfx::B2DPolyPolygon(aCircPolygon), bFill, bBezier);

    if(bAddText)
    {
        pRet = ImpConvertAddText(pRet, bBezier);
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
