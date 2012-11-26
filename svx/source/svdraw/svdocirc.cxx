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
#include <svl/style.hxx>
#include <tools/bigint.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/svdocirc.hxx>
#include <math.h>
#include <svx/xpool.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdopath.hxx> // fuer die Objektkonvertierung
#include <svx/svdview.hxx>  // Zum Draggen (Ortho)
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // Objektname
#include <editeng/eeitem.hxx>
#include <svx/sdr/contact/viewcontactofsdrcircobj.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/svdlegacy.hxx>

//////////////////////////////////////////////////////////////////////////////
// nameless helpers

namespace
{
    double impSnapAngle(double fAngle, const SdrView& rView)
    {
        if(rView.IsAngleSnapEnabled())
        {
            // angle snap
            const double fSnapAngle(((rView.GetSnapAngle() % 36000) * F_PI) / 18000.0);

            fAngle = basegfx::snapToNearestMultiple(fAngle, fSnapAngle);

            if(basegfx::fTools::equalZero(fAngle))
            {
                fAngle = 0.0;
            }
            else if(basegfx::fTools::equal(fAngle, F_2PI))
            {
                fAngle = F_2PI;
            }
        }

        return fAngle;
    }

    void impCalcNewStartEnd(SdrDragStat& rStat, SdrCircObj& rCircObj, double& o_rfNewStart, double& o_rfNewEnd)
    {
        const sal_uInt32 nPntAnz(rStat.GetPointAnz());

        if(nPntAnz > 2)
        {
            basegfx::B2DHomMatrix aInvObj(rCircObj.getSdrObjectTransformation());
            aInvObj.invert();

            // calc vector in normalized object coordinates
            const bool bPoint3(3 == nPntAnz);
            const basegfx::B2DPoint aPoint(bPoint3 ? rStat.GetPoint(2) : rStat.GetPoint(3));
            const basegfx::B2DPoint aVec(aInvObj * aPoint);

            // calc angle in normalized object coordinates
            double fNewAngle(atan2(aVec.getY() - 0.5, aVec.getX() - 0.5));

            if(fNewAngle < 0.0)
            {
                // move from atan2's [-F_PI .. F_PI] to [0.0 .. F_2PI] range
                fNewAngle += F_2PI;
            }

            // angle snap
            fNewAngle = impSnapAngle(fNewAngle, rStat.GetSdrViewFromSdrDragStat());

            if(bPoint3)
            {
                o_rfNewStart = fNewAngle;
            }

            o_rfNewEnd = fNewAngle;
        }
    }

    basegfx::B2DPolygon ImpCalcXPolyCirc(
        const SdrCircleObjType eSdrCircleObjType,
        const basegfx::B2DHomMatrix& rTransform,
        double fStart,
        double fEnd)
    {
        basegfx::B2DPolygon aCircPolygon;

        if(CircleType_Circle == eSdrCircleObjType || basegfx::fTools::equal(fStart, fEnd))
        {
            // create full circle. Do not use createPolygonFromEllipse; it's necessary
            // to get the start point to the bottom of the circle to keep compatible to
            // old geometry creation
            aCircPolygon = basegfx::tools::createPolygonFromUnitCircle(1);
        }
        else
        {
            // create circle segment. This is not closed by default.
            // Exchange start and end since the historical definitions spawns
            // the visible part of the circle segment in mathematically negative
            // direction
            aCircPolygon = basegfx::tools::createPolygonFromUnitEllipseSegment(fEnd, fStart);

            // check closing states
            const bool bCloseSegment(CircleType_Arc != eSdrCircleObjType);
            const bool bCloseUsingCenter(CircleType_Sector == eSdrCircleObjType);

            if(bCloseSegment)
            {
                if(bCloseUsingCenter)
                {
                    // add center point at start (for historical reasons)
                    basegfx::B2DPolygon aSector;
                    aSector.append(basegfx::B2DPoint(0.0, 0.0));
                    aSector.append(aCircPolygon);
                    aCircPolygon = aSector;
                }

                // close
                aCircPolygon.setClosed(true);
            }
        }

        // scale and move UnitEllipse to UnitObject (-1,-1 1,1) -> (0,0 1,1), then
        // apply object transformation
        const basegfx::B2DHomMatrix aUnitToWorld(
            rTransform *
            basegfx::tools::createScaleTranslateB2DHomMatrix(0.5, 0.5, 0.5, 0.5));

        // apply
        aCircPolygon.transform(aUnitToWorld);

        return aCircPolygon;
    }
}

//////////////////////////////////////////////////////////////////////////////
// SdrCircObjGeoData

class SdrCircObjGeoData : public SdrObjGeoData
{
public:
    SdrCircleObjType            meSdrCircleObjType;
    double                      mfStartAngle;   // [-F_PI .. F_PI]
    double                      mfEndAngle;     // [-F_PI .. F_PI]
};

//////////////////////////////////////////////////////////////////////////////
// SdrCircObj

sdr::contact::ViewContact* SdrCircObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrCircObj(*this);
}

SdrCircObj::SdrCircObj(
    SdrModel& rSdrModel,
    SdrCircleObjType eSdrCircleObjType,
    const basegfx::B2DHomMatrix& rTransform,
    double fNewStartWink,
    double fNewEndWink)
:   SdrRectObj(
        rSdrModel,
        rTransform),
    meSdrCircleObjType(eSdrCircleObjType),
    mfStartAngle(fNewStartWink),
    mfEndAngle(fNewEndWink)
{
    // snap angles to [0.0 .. F_2PI] range
    if(0.0 != mfStartAngle)
    {
        mfStartAngle = basegfx::snapToZeroRange(mfStartAngle, F_2PI);
    }

    if(F_2PI != mfEndAngle)
    {
        mfEndAngle = basegfx::snapToZeroRange(mfEndAngle, F_2PI);
    }
}

SdrCircObj::~SdrCircObj()
{
}

void SdrCircObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrCircObj* pSource = dynamic_cast< const SdrCircObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrRectObj::copyDataFromSdrObject(rSource);

            // copy local data
            meSdrCircleObjType = pSource->meSdrCircleObjType;
            mfStartAngle = pSource->mfStartAngle;
            mfEndAngle = pSource->mfEndAngle;
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SdrCircObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrCircObj* pClone = new SdrCircObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

bool SdrCircObj::IsClosedObj() const
{
    return (CircleType_Arc != meSdrCircleObjType);
}

double SdrCircObj::GetStartAngle() const
{
    return mfStartAngle;
}

double SdrCircObj::GetEndAngle() const
{
    return mfEndAngle;
}

SdrCircleObjType SdrCircObj::GetSdrCircleObjType() const
{
    return meSdrCircleObjType;
}

void SdrCircObj::SetSdrCircleObjType(SdrCircleObjType eNew)
{
    if(eNew != meSdrCircleObjType)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

        meSdrCircleObjType = eNew;
        SetChanged();
    }
}

void SdrCircObj::SetStartAngle(double fNew)
{
    fNew = basegfx::snapToZeroRange(fNew, F_2PI);

    if(!basegfx::fTools::equal(fNew, mfStartAngle))
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

        mfStartAngle = fNew;
        SetChanged();
    }
}

void SdrCircObj::SetEndAngle(double fNew)
{
    fNew = basegfx::snapToZeroRange(fNew, F_2PI);

    if(!basegfx::fTools::equal(fNew, mfEndAngle))
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

        mfEndAngle = fNew;
        SetChanged();
    }
}

void SdrCircObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    const bool bCanConv(!HasText() || ImpCanConvTextToCurve());

    rInfo.mbEdgeRadiusAllowed = false;
    rInfo.mbCanConvToPath = bCanConv;
    rInfo.mbCanConvToPoly = bCanConv;
    rInfo.mbCanConvToContour = !IsFontwork() && (rInfo.mbCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrCircObj::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_CIRC);
}

void SdrCircObj::TakeObjNameSingul(XubString& rName) const
{
    sal_uInt16 nID(STR_ObjNameSingulCIRC);
    const basegfx::B2DVector& rScale(getSdrObjectScale());
    const bool bOrthoScaled(basegfx::fTools::equal(fabs(rScale.getX()), fabs(rScale.getY())));

    if(bOrthoScaled && !isSheared())
    {
        switch (meSdrCircleObjType)
        {
            case CircleType_Circle:
            {
                nID = STR_ObjNameSingulCIRC;
                break;
            }
            case CircleType_Sector:
            {
                nID = STR_ObjNameSingulSECT;
                break;
            }
            case CircleType_Arc:
            {
                nID = STR_ObjNameSingulCARC;
                break;
            }
            case CircleType_Segment:
            {
                nID = STR_ObjNameSingulCCUT;
                break;
            }
            default: break;
        }
    }
    else
    {
        switch (meSdrCircleObjType)
        {
            case CircleType_Circle:
            {
                nID = STR_ObjNameSingulCIRCE;
                break;
            }
            case CircleType_Sector:
            {
                nID = STR_ObjNameSingulSECTE;
                break;
            }
            case CircleType_Arc:
            {
                nID = STR_ObjNameSingulCARCE;
                break;
            }
            case CircleType_Segment:
            {
                nID = STR_ObjNameSingulCCUTE;
                break;
            }
            default: break;
        }
    }

    rName = ImpGetResStr(nID);

    String aName( GetName() );

    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SdrCircObj::TakeObjNamePlural(XubString& rName) const
{
    sal_uInt16 nID(STR_ObjNamePluralCIRC);
    const basegfx::B2DVector& rScale(getSdrObjectScale());
    const bool bOrthoScaled(basegfx::fTools::equal(fabs(rScale.getX()), fabs(rScale.getY())));

    if(bOrthoScaled && !isSheared())
    {
        switch (meSdrCircleObjType)
        {
            case CircleType_Circle:
            {
                nID = STR_ObjNamePluralCIRC;
                break;
            }
            case CircleType_Sector:
            {
                nID = STR_ObjNamePluralSECT;
                break;
            }
            case CircleType_Arc:
            {
                nID = STR_ObjNamePluralCARC;
                break;
            }
            case CircleType_Segment:
            {
                nID = STR_ObjNamePluralCCUT;
                break;
            }
            default: break;
        }
    }
    else
    {
        switch (meSdrCircleObjType)
        {
            case CircleType_Circle:
            {
                nID = STR_ObjNamePluralCIRCE;
                break;
            }
            case CircleType_Sector:
            {
                nID = STR_ObjNamePluralSECTE;
                break;
            }
            case CircleType_Arc:
            {
                nID = STR_ObjNamePluralCARCE;
                break;
            }
            case CircleType_Segment:
            {
                nID = STR_ObjNamePluralCCUTE;
                break;
            }
            default: break;
        }
    }

    rName = ImpGetResStr(nID);
}

basegfx::B2DPolyPolygon SdrCircObj::TakeXorPoly() const
{
    const basegfx::B2DPolygon aCircPolygon(
        ImpCalcXPolyCirc(
            GetSdrCircleObjType(),
            getSdrObjectTransformation(),
            GetStartAngle(),
            GetEndAngle()));

    return basegfx::B2DPolyPolygon(aCircPolygon);
}

void SdrCircObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    if(CircleType_Circle != meSdrCircleObjType)
    {
        // start angle handle
        basegfx::B2DPoint aStart(0.5 + (cos(GetStartAngle()) * 0.5), 0.5 + (sin(GetStartAngle()) * 0.5));
        aStart = getSdrObjectTransformation() * aStart;
        SdrHdl* pStartHdl = new SdrHdl(rHdlList, this, HDL_CIRC, aStart);
        pStartHdl->SetPointNum(1);

        // end angle handle
        basegfx::B2DPoint aEnd(0.5 + (cos(GetEndAngle()) * 0.5), 0.5 + (sin(GetEndAngle()) * 0.5));
        aEnd = getSdrObjectTransformation() * aEnd;
        SdrHdl* pEndHdl = new SdrHdl(rHdlList, this, HDL_CIRC, aEnd);
        pEndHdl->SetPointNum(2);
    }

    // add parent's handles. Use SdrTextObj to not add the thext frame handle,
    // but the regular object ones (eight, not nine)
    SdrTextObj::AddToHdlList(rHdlList);
}

bool SdrCircObj::hasSpecialDrag() const
{
    return true;
}

bool SdrCircObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const bool bWink(rDrag.GetActiveHdl() && HDL_CIRC == rDrag.GetActiveHdl()->GetKind());

    if(bWink)
    {
        if(1 == rDrag.GetActiveHdl()->GetPointNum() || 2 == rDrag.GetActiveHdl()->GetPointNum())
        {
            rDrag.SetNoSnap(true);
        }

        return true;
    }

    return SdrTextObj::beginSpecialDrag(rDrag);
}

bool SdrCircObj::applySpecialDrag(SdrDragStat& rDrag)
{
    const bool bWink(rDrag.GetActiveHdl() && HDL_CIRC == rDrag.GetActiveHdl()->GetKind());

    if(bWink)
    {
        basegfx::B2DHomMatrix aInverse(getSdrObjectTransformation());
        aInverse.invert();
        const basegfx::B2DPoint aObjectCoor(aInverse * rDrag.GetNow());

        // calc angle in normalized object coordinates
        double fNewAngle(atan2(aObjectCoor.getY() - 0.5, aObjectCoor.getX() - 0.5));

        if(fNewAngle < 0.0)
        {
            // move from atan2's [-F_PI .. F_PI] to [0.0 .. F_2PI] range
            fNewAngle += F_2PI;
        }

        // angle snap
        fNewAngle = impSnapAngle(fNewAngle, rDrag.GetSdrViewFromSdrDragStat());

        if(1 == rDrag.GetActiveHdl()->GetPointNum())
        {
            SetStartAngle(fNewAngle);
        }
        else // (2 == rDrag.GetActiveHdl()->GetPointNum())
        {
            SetEndAngle(fNewAngle);
        }

        return true;
    }
    else
    {
        return SdrTextObj::applySpecialDrag(rDrag);
    }
}

String SdrCircObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    const bool bCreateComment(this == rDrag.GetSdrViewFromSdrDragStat().GetCreateObj());

    if(bCreateComment)
    {
        XubString aStr;
        TakeMarkedDescriptionString(STR_ViewCreateObj, aStr);
        const sal_uInt32 nPntAnz(rDrag.GetPointAnz());

        if(CircleType_Circle != meSdrCircleObjType && nPntAnz > 2)
        {
            double fWink(0.0);

            aStr.AppendAscii(" (");

            if(3 == nPntAnz)
            {
                fWink = GetStartAngle();
            }
            else
            {
                fWink = GetEndAngle();
            }

            const sal_Int32 nWink(basegfx::fround(((F_2PI - fWink) * 18000.0) / F_PI) % 36000);
            aStr += GetWinkStr(nWink, false);
            aStr += sal_Unicode(')');
        }

        return aStr;
    }
    else
    {
        const bool bWink(rDrag.GetActiveHdl() && HDL_CIRC == rDrag.GetActiveHdl()->GetKind());

        if(bWink)
        {
            XubString aStr;
            const double fWink(1 == rDrag.GetActiveHdl()->GetPointNum() ? GetStartAngle() : GetEndAngle());
            const sal_Int32 nWink(basegfx::fround(((F_2PI - fWink) * 18000.0) / F_PI) % 36000);

            TakeMarkedDescriptionString(STR_DragCircAngle, aStr);
            aStr.AppendAscii(" (");
            aStr += GetWinkStr(nWink, false);
            aStr += sal_Unicode(')');

            return aStr;
        }
        else
        {
            return SdrTextObj::getSpecialDragComment(rDrag);
        }
    }
}

bool SdrCircObj::MovCreate(SdrDragStat& rStat)
{
    const sal_uInt32 nPntAnz(rStat.GetPointAnz());

    if(nPntAnz < 3)
    {
        return SdrRectObj::MovCreate(rStat);
    }
    else
    {
        double fNewStart(GetStartAngle());
        double fNewEnd(GetEndAngle());

        impCalcNewStartEnd(rStat, *this, fNewStart, fNewEnd);
        SetStartAngle(fNewStart);
        SetEndAngle(fNewEnd);
    }

    return true;
}

bool SdrCircObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    bool bRet(false);
    const sal_uInt32 nPntAnz(rStat.GetPointAnz());

    if(SDRCREATE_FORCEEND == eCmd && nPntAnz < 4)
    {
        meSdrCircleObjType = CircleType_Circle;
    }

    if(CircleType_Circle == meSdrCircleObjType)
    {
        bRet = (nPntAnz >= 2);

        if(bRet)
        {
            SdrRectObj::EndCreate(rStat, eCmd);
        }
    }
    else
    {
        rStat.SetNoSnap(nPntAnz >= 2);
        rStat.SetOrtho4Possible(nPntAnz < 2);
        bRet = (nPntAnz >= 4);

        if(bRet)
        {
            double fNewStart(GetStartAngle());
            double fNewEnd(GetEndAngle());

            impCalcNewStartEnd(rStat, *this, fNewStart, fNewEnd);
            SetStartAngle(fNewStart);
            SetEndAngle(fNewEnd);
        }
    }

    return bRet;
}

bool SdrCircObj::BckCreate(SdrDragStat& rStat)
{
    const sal_uInt32 nPntAnz(rStat.GetPointAnz());

    rStat.SetNoSnap(nPntAnz >= 3);
    rStat.SetOrtho4Possible(nPntAnz < 3);

    return (CircleType_Circle != meSdrCircleObjType);
}

basegfx::B2DPolyPolygon SdrCircObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    const sal_uInt32 nPntAnz(rDrag.GetPointAnz());

    if(nPntAnz < 4)
    {
        // force to CircleType_Circle to get full visualisation
        basegfx::B2DPolyPolygon aRetval(
            ImpCalcXPolyCirc(
                CircleType_Circle,
                getSdrObjectTransformation(),
                0.0,
                0.0));

        if(3 == nPntAnz)
        {
            // add edge to first point on ellipse
            basegfx::B2DPolygon aNew;
            const basegfx::B2DPoint aStart(0.5 + (cos(GetStartAngle()) * 0.5), 0.5 + (sin(GetStartAngle()) * 0.5));

            // add center and point at start angle as line
            aNew.append(getSdrObjectTransformation() * basegfx::B2DPoint(0.5, 0.5));
            aNew.append(getSdrObjectTransformation() * aStart);
            aRetval.append(aNew);
        }

        return aRetval;
    }
    else
    {
        return basegfx::B2DPolyPolygon(
            ImpCalcXPolyCirc(
                GetSdrCircleObjType(),
                getSdrObjectTransformation(),
                GetStartAngle(),
                GetEndAngle()));
    }
}

Pointer SdrCircObj::GetCreatePointer(const SdrView& /*rSdrView*/) const
{
    switch (meSdrCircleObjType)
    {
        case CircleType_Circle: return Pointer(POINTER_DRAW_ELLIPSE);
        case CircleType_Sector: return Pointer(POINTER_DRAW_PIE);
        case CircleType_Arc: return Pointer(POINTER_DRAW_ARC);
        case CircleType_Segment: return Pointer(POINTER_DRAW_CIRCLECUT);
        default: break;
    }

    return Pointer(POINTER_CROSS);
}

SdrObjGeoData* SdrCircObj::NewGeoData() const
{
    return new SdrCircObjGeoData;
}

void SdrCircObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrRectObj::SaveGeoData(rGeo);
    SdrCircObjGeoData* pCGeo = dynamic_cast< SdrCircObjGeoData* >(&rGeo);

    if(pCGeo)
    {
        pCGeo->meSdrCircleObjType = GetSdrCircleObjType();
        pCGeo->mfStartAngle = GetStartAngle();
        pCGeo->mfEndAngle = GetEndAngle();
    }
}

void SdrCircObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::RestGeoData(rGeo);
    const SdrCircObjGeoData* pCGeo = dynamic_cast< const SdrCircObjGeoData* >(&rGeo);

    if(pCGeo)
    {
        SetSdrCircleObjType(pCGeo->meSdrCircleObjType);
        SetStartAngle(pCGeo->mfStartAngle);
        SetEndAngle(pCGeo->mfEndAngle);
    }
}

sal_uInt32 SdrCircObj::GetSnapPointCount() const
{
    if(CircleType_Circle == meSdrCircleObjType)
    {
        return 1;
    }
    else
    {
        return 3;
    }
}

basegfx::B2DPoint SdrCircObj::GetSnapPoint(sal_uInt32 i) const
{
    basegfx::B2DPoint aNormalized(0.5, 0.5);

    switch(i)
    {
        case 1:
        {
            aNormalized = basegfx::B2DPoint(0.5 + (cos(GetStartAngle()) * 0.5), 0.5 + (sin(GetStartAngle()) * 0.5));
            break;
        }
        case 2:
        {
            aNormalized = basegfx::B2DPoint(0.5 + (cos(GetEndAngle()) * 0.5), 0.5 + (sin(GetEndAngle()) * 0.5));
            break;
        }
        default:
        {
            // center requested; already in aNormalized
            break;
        }
    }

    return getSdrObjectTransformation() * aNormalized;
}

SdrObject* SdrCircObj::DoConvertToPolygonObject(bool bBezier, bool bAddText) const
{
    const bool bCanBeFilled(CircleType_Arc != meSdrCircleObjType);
    const basegfx::B2DPolygon aCircPolygon(
        ImpCalcXPolyCirc(
            GetSdrCircleObjType(),
            getSdrObjectTransformation(),
            GetStartAngle(),
            GetEndAngle()));
    SdrObject* pRet = ImpConvertMakeObj(basegfx::B2DPolyPolygon(aCircPolygon), bCanBeFilled, bBezier);

    if(bAddText)
    {
        pRet = ImpConvertAddText(pRet, bBezier);
    }

    return pRet;
}

//////////////////////////////////////////////////////////////////////////////
// eof
