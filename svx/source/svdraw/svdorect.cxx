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
#include <svx/svdocapt.hxx> // fuer Import von SdrFileVersion 2
#include <svx/svdpagv.hxx> // fuer
#include <svx/svdview.hxx> // das
#include <svx/svdundo.hxx> // Macro-Beispiel
#include <svx/svdopath.hxx>
#include "svx/svdglob.hxx"  // Stringcache
#include "svx/svdstr.hrc"   // Objektname
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/sdr/properties/rectangleproperties.hxx>
#include <svx/sdr/contact/viewcontactofsdrrectobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>

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

SdrRectObj::SdrRectObj(
    SdrModel& rSdrModel,
    const basegfx::B2DHomMatrix& rTransform,
    SdrObjKind eNewTextKind,
    bool bIsTextFrame)
:   SdrTextObj(rSdrModel, rTransform, eNewTextKind, bIsTextFrame)
{
    DBG_ASSERT(eTextKind==OBJ_TEXT || eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
               "SdrRectObj::SdrRectObj(SdrObjKind,...) ist nur fuer Textrahmen gedacht");
}

SdrRectObj::~SdrRectObj()
{
}

void SdrRectObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrRectObj* pSource = dynamic_cast< const SdrRectObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrTextObj::copyDataFromSdrObject(rSource);

            // no local data to copy
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SdrRectObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrRectObj* pClone = new SdrRectObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

bool SdrRectObj::IsClosedObj() const
{
    return true;
}

void SdrRectObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    bool bNoTextFrame = !IsTextFrame();
    const long aOldRotation(sdr::legacy::GetRotateAngle(*this));
    rInfo.mbResizeFreeAllowed = bNoTextFrame || aOldRotation%9000==0;
    rInfo.mbResizePropAllowed = true;
    rInfo.mbRotateFreeAllowed = true;
    rInfo.mbRotate90Allowed = true;
    rInfo.mbMirrorFreeAllowed=bNoTextFrame;
    rInfo.mbMirror45Allowed  =bNoTextFrame;
    rInfo.mbMirror90Allowed = bNoTextFrame;

    // allow transparence
    rInfo.mbTransparenceAllowed = true;

    // gradient depends on fillstyle
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetObjectItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.mbGradientAllowed = (XFILL_GRADIENT == eFillStyle);

    rInfo.mbShearAllowed = bNoTextFrame;
    rInfo.mbEdgeRadiusAllowed = true;

    bool bCanConv = !HasText() || ImpCanConvTextToCurve();

    if (bCanConv && !bNoTextFrame && !HasText())
    {
        bCanConv=HasFill() || HasLine();
    }

    rInfo.mbCanConvToPath = bCanConv;
    rInfo.mbCanConvToPoly = bCanConv;
    rInfo.mbCanConvToContour = (rInfo.mbCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrRectObj::GetObjIdentifier() const
{
    if(IsTextFrame())
        return sal_uInt16(eTextKind);
    else
        return sal_uInt16(OBJ_RECT);
}

void SdrRectObj::TakeObjNameSingul(XubString& rName) const
{
    if (IsTextFrame())
    {
        SdrTextObj::TakeObjNameSingul(rName);
    }
    else
    {
        sal_uInt16 nResId(STR_ObjNameSingulRECT);

        if(!basegfx::fTools::equalZero(getSdrObjectShearX()))
        {
            nResId+=4;  // Parallelogramm oder Raute
            // Raute ist nicht, weil Shear die vertikalen Kanten verlaengert!
            // Wenn Zeit ist, werde ich das mal berechnen.
        }
        else
        {
            const basegfx::B2DVector aObjectScale(absolute(getSdrObjectScale()));

            if(basegfx::fTools::equal(aObjectScale.getX(), aObjectScale.getY()))
            {
                nResId += 2; // Quadrat
            }
        }
        if(GetEdgeRadius())
        {
            nResId += 8; // abgerundet
        }

        rName=ImpGetResStr(nResId);

        String aName( GetName() );

        if(aName.Len())
        {
            rName += sal_Unicode(' ');
            rName += sal_Unicode('\'');
            rName += aName;
            rName += sal_Unicode('\'');
        }
    }
}

void SdrRectObj::TakeObjNamePlural(XubString& rName) const
{
    if (IsTextFrame())
    {
        SdrTextObj::TakeObjNamePlural(rName);
    }
    else
    {
        sal_uInt16 nResId(STR_ObjNamePluralRECT);

        if(!basegfx::fTools::equalZero(getSdrObjectShearX()))
        {
            nResId += 4;  // Parallelogramm oder Raute
        }
        else
        {
            const basegfx::B2DVector aObjectScale(absolute(getSdrObjectScale()));

            if(basegfx::fTools::equal(aObjectScale.getX(), aObjectScale.getY()))
            {
                nResId += 2; // Quadrat
            }
        }

        if(GetEdgeRadius())
        {
            nResId += 8; // abgerundet
        }

        rName = ImpGetResStr(nResId);
    }
}

basegfx::B2DPolyPolygon SdrRectObj::TakeXorPoly() const
{
    double fCornerRadiusX(0.0);
    double fCornerRadiusY(0.0);

    if(GetEdgeRadius())
    {
        const basegfx::B2DVector aObjectScale(absolute(getSdrObjectScale()));

        drawinglayer::primitive2d::calculateRelativeCornerRadius(
            GetEdgeRadius(),
            aObjectScale.getX(),
            aObjectScale.getY(),
            fCornerRadiusX,
            fCornerRadiusY);
    }

    basegfx::B2DPolygon aUnitOutline(
        basegfx::tools::createPolygonFromRect(
            basegfx::B2DRange::getUnitB2DRange(),
            fCornerRadiusX,
            fCornerRadiusY));

    aUnitOutline.transform(getSdrObjectTransformation());

    return basegfx::B2DPolyPolygon(aUnitOutline);
}

void SdrRectObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    if(IsTextFrame())
    {
        // add TextFrame handle
        new ImpTextframeHdl(rHdlList, *this, getSdrObjectTransformation());
    }

    {
        // add edge radius control
        const basegfx::B2DVector aAbsScale(basegfx::absolute(getSdrObjectScale()));
        double fRadius(std::max(0.0, (double)GetEdgeRadius()));
        basegfx::B2DPoint aPos;

        if(fRadius < 0.0)
        {
            fRadius = 0.0;
        }

        if(aAbsScale.getX() > aAbsScale.getY())
        {
            fRadius /= basegfx::fTools::equalZero(aAbsScale.getX()) ? 1.0 : aAbsScale.getX();
            fRadius = std::min(0.5, fRadius);
            aPos = getSdrObjectTransformation() * basegfx::B2DPoint(fRadius, 0.0);
        }
        else
        {
            fRadius /= basegfx::fTools::equalZero(aAbsScale.getY()) ? 1.0 : aAbsScale.getY();
            fRadius = std::min(0.5, fRadius);
            aPos = getSdrObjectTransformation() * basegfx::B2DPoint(0.0, fRadius);
        }

        new SdrHdl(rHdlList, this, HDL_CIRC, aPos);
    }

    // use default eight object handles from parent
    SdrTextObj::AddToHdlList(rHdlList);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrRectObj::hasSpecialDrag() const
{
    return true;
}

bool SdrRectObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const bool bRad(rDrag.GetActiveHdl() && HDL_CIRC == rDrag.GetActiveHdl()->GetKind());

    if(bRad)
    {
        rDrag.SetEndDragChangesAttributes(true);

        return true;
    }

    return SdrTextObj::beginSpecialDrag(rDrag);
}

bool SdrRectObj::applySpecialDrag(SdrDragStat& rDrag)
{
    const bool bRad(rDrag.GetActiveHdl() && HDL_CIRC == rDrag.GetActiveHdl()->GetKind());

    if (bRad)
    {
        basegfx::B2DHomMatrix aInverse(getSdrObjectTransformation());
        aInverse.invert();

        const basegfx::B2DPoint aObjectCoor(aInverse * rDrag.GetNow());
        const basegfx::B2DVector aAbsScale(basegfx::absolute(getSdrObjectScale()));
        sal_Int32 nRadius(0);

        if(aAbsScale.getX() > aAbsScale.getY())
        {
            const double fRadius(aAbsScale.getX() * (std::min(0.5, std::max(0.0, aObjectCoor.getX()))));
            nRadius = basegfx::fround(fRadius);
        }
        else
        {
            const double fRadius(aAbsScale.getY() * (std::min(0.5, std::max(0.0, aObjectCoor.getY()))));
            nRadius = basegfx::fround(fRadius);
        }

        if(nRadius != GetEdgeRadius())
        {
            SetEdgeRadius(nRadius);
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
    const bool bCreateComment(this == rDrag.GetSdrViewFromSdrDragStat().GetCreateObj());

    if(bCreateComment)
    {
        return String();
    }
    else
    {
        const bool bRad(rDrag.GetActiveHdl() && HDL_CIRC == rDrag.GetActiveHdl()->GetKind());

        if(bRad)
        {
            basegfx::B2DPoint aPoint(rDrag.GetNow());
            const basegfx::B2DPoint aObjectTopLeft(getSdrObjectTranslate());
            const double fObjectRotate(getSdrObjectRotate());
            const double fObjectShearX(getSdrObjectShearX());
            basegfx::B2DHomMatrix aBackTransform;

            if(!basegfx::fTools::equalZero(fObjectRotate) || !basegfx::fTools::equalZero(fObjectShearX))
            {
                aBackTransform.translate(-getSdrObjectTranslate());

                if(!basegfx::fTools::equalZero(fObjectRotate))
                {
                    aBackTransform.rotate(-fObjectRotate);
                }

                if(!basegfx::fTools::equalZero(fObjectShearX))
                {
                    aBackTransform.shearX(-fObjectShearX);
                }

                aBackTransform.translate(getSdrObjectTranslate());
            }

            if(!aBackTransform.isIdentity())
            {
                aPoint = aBackTransform * aPoint;
            }

            double fRad(aPoint.getX() - aObjectTopLeft.getX());

            if(fRad < 0.0)
            {
                fRad = 0.0;
            }

            XubString aStr;

            TakeMarkedDescriptionString(STR_DragRectEckRad, aStr);
            aStr.AppendAscii(" (");
            aStr += GetMetrStr(basegfx::fround(fRad));
            aStr += sal_Unicode(')');

            return aStr;
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
    const basegfx::B2DRange aRange(rDrag.TakeCreateRange());
    const basegfx::B2DVector aScale(aRange.getRange());
    double fCornerRadiusX(0.0);
    double fCornerRadiusY(0.0);

    if(GetEdgeRadius())
    {
        drawinglayer::primitive2d::calculateRelativeCornerRadius(
            GetEdgeRadius(),
            aScale.getX(),
            aScale.getY(),
            fCornerRadiusX,
            fCornerRadiusY);
    }

    basegfx::B2DPolygon aUnitOutline(
        basegfx::tools::createPolygonFromRect(
            basegfx::B2DRange::getUnitB2DRange(),
            fCornerRadiusX,
            fCornerRadiusY));

    aUnitOutline.transform(basegfx::tools::createScaleTranslateB2DHomMatrix(aScale, aRange.getMinimum()));

    return basegfx::B2DPolyPolygon(aUnitOutline);
}

Pointer SdrRectObj::GetCreatePointer(const SdrView& /*rSdrView*/) const
{
    if (IsTextFrame())
    {
        return Pointer(POINTER_DRAW_TEXT);
    }

    return Pointer(POINTER_DRAW_RECT);
}

bool SdrRectObj::DoMacro(const SdrObjMacroHitRec& rRec)
{
    return SdrTextObj::DoMacro(rRec);
}

XubString SdrRectObj::GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const
{
    return SdrTextObj::GetMacroPopupComment(rRec);
}

SdrObject* SdrRectObj::DoConvertToPolygonObject(bool bBezier, bool bAddText) const
{
    basegfx::B2DPolyPolygon aPolyPolygon(TakeXorPoly());
    aPolyPolygon.removeDoublePoints();
    SdrObject* pRet = 0;

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

//////////////////////////////////////////////////////////////////////////////
// eof
