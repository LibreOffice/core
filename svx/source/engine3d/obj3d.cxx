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

#include <o3tl/numeric.hxx>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/svdview.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svditer.hxx>
#include <svx/globl3d.hxx>
#include <svx/camera3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/obj3d.hxx>
#include <svx/xtable.hxx>
#include <svx/xflclit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svx/xlnclit.hxx>
#include <svl/metitem.hxx>
#include <svx/xfillit.hxx>
#include <svx/xlnwtit.hxx>
#include <vcl/virdev.hxx>
#include <tools/poly.hxx>
#include <tools/b3dtrans.hxx>
#include <svx/svxids.hrc>
#include <editeng/colritem.hxx>
#include <svx/e3ditem.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/gradient.hxx>
#include <vcl/metaact.hxx>
#include <svx/svx3ditems.hxx>
#include <svl/whiter.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/eeitem.hxx>
#include <svx/xgrscit.hxx>
#include <sdr/properties/e3dproperties.hxx>
#include <sdr/properties/e3dcompoundproperties.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <svx/xlndsit.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <svx/helperhittest3d.hxx>
#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <svx/e3dsceneupdater.hxx>

using namespace com::sun::star;

std::unique_ptr<sdr::properties::BaseProperties> E3dObject::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::E3dProperties>(*this);
}

E3dObject::E3dObject(SdrModel& rSdrModel)
:   SdrAttrObj(rSdrModel),
    maLocalBoundVol(),
    maTransformation(),
    maFullTransform(),
    mbTfHasChanged(true),
    mbIsSelected(false)
{
    bIs3DObj = true;
    bClosedObj = true;
}

E3dObject::~E3dObject()
{
}

void E3dObject::SetSelected(bool bNew)
{
    if(mbIsSelected != bNew)
    {
        mbIsSelected = bNew;
    }
}

// Break, default implementations
bool E3dObject::IsBreakObjPossible()
{
    return false;
}

std::unique_ptr<SdrAttrObj,SdrObjectFreeOp> E3dObject::GetBreakObj()
{
    return nullptr;
}

SdrInventor E3dObject::GetObjInventor() const
{
    return SdrInventor::E3d;
}

sal_uInt16 E3dObject::GetObjIdentifier() const
{
    return E3D_OBJECT_ID;
}

// Determine the capabilities of the object
void E3dObject::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bResizeFreeAllowed    = true;
    rInfo.bResizePropAllowed    = true;
    rInfo.bRotateFreeAllowed    = true;
    rInfo.bRotate90Allowed      = true;
    rInfo.bMirrorFreeAllowed    = false;
    rInfo.bMirror45Allowed      = false;
    rInfo.bMirror90Allowed      = false;
    rInfo.bShearAllowed         = false;
    rInfo.bEdgeRadiusAllowed    = false;
    rInfo.bCanConvToPath        = false;

    // no transparence for 3d objects
    rInfo.bTransparenceAllowed = false;

    // Convert 3D objects in a group of polygons:
    // At first not only possible, because the creation of a group of
    // 2D polygons would be required which need to be sorted by depth,
    // ie at intersections be cut relative to each other. Also the texture
    // coordinates were an unsolved problem.
    rInfo.bCanConvToPoly = false;
    rInfo.bCanConvToContour = false;
    rInfo.bCanConvToPathLineToArea = false;
    rInfo.bCanConvToPolyLineToArea = false;
}

// resize object, used from old 2d interfaces, e.g. in Move/Scale dialog (F4)
void E3dObject::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    // Movement in X, Y in the eye coordinate system
    E3dScene* pScene(getRootE3dSceneFromE3dObject());

    if(nullptr == pScene)
    {
        return;
    }

    // transform pos from 2D world to 3D eye
    const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
    const drawinglayer::geometry::ViewInformation3D& aViewInfo3D(rVCScene.getViewInformation3D());
    basegfx::B2DPoint aScaleCenter2D(static_cast<double>(rRef.X()), static_cast<double>(rRef.Y()));
    basegfx::B2DHomMatrix aInverseSceneTransform(rVCScene.getObjectTransformation());

    aInverseSceneTransform.invert();
    aScaleCenter2D = aInverseSceneTransform * aScaleCenter2D;

    basegfx::B3DPoint aScaleCenter3D(aScaleCenter2D.getX(), aScaleCenter2D.getY(), 0.5);
    basegfx::B3DHomMatrix aInverseViewToEye(aViewInfo3D.getDeviceToView() * aViewInfo3D.getProjection());

    aInverseViewToEye.invert();
    aScaleCenter3D = aInverseViewToEye * aScaleCenter3D;

    // Get scale factors
    double fScaleX(xFact);
    double fScaleY(yFact);

    // build transform
    basegfx::B3DHomMatrix aInverseOrientation(aViewInfo3D.getOrientation());
    aInverseOrientation.invert();
    basegfx::B3DHomMatrix aFullTransform(GetFullTransform());
    basegfx::B3DHomMatrix aTrans(aFullTransform);

    aTrans *= aViewInfo3D.getOrientation();
    aTrans.translate(-aScaleCenter3D.getX(), -aScaleCenter3D.getY(), -aScaleCenter3D.getZ());
    aTrans.scale(fScaleX, fScaleY, 1.0);
    aTrans.translate(aScaleCenter3D.getX(), aScaleCenter3D.getY(), aScaleCenter3D.getZ());
    aTrans *= aInverseOrientation;
    aFullTransform.invert();
    aTrans *= aFullTransform;

    // Apply
    basegfx::B3DHomMatrix aObjTrans(GetTransform());
    aObjTrans *= aTrans;

    E3DModifySceneSnapRectUpdater aUpdater(this);
    SetTransform(aObjTrans);
}

// Move object in 2D is needed when using cursor keys
void E3dObject::NbcMove(const Size& rSize)
{
    // Movement in X, Y in the eye coordinate system
    E3dScene* pScene(getRootE3dSceneFromE3dObject());

    if(nullptr == pScene)
    {
        return;
    }

    //Dimensions of the scene in 3D and 2D for comparison
    tools::Rectangle aRect = pScene->GetSnapRect();
    basegfx::B3DHomMatrix aInvDispTransform;
    E3dScene* pParent(getParentE3dSceneFromE3dObject());

    if(nullptr != pParent)
    {
        aInvDispTransform = pParent->GetFullTransform();
        aInvDispTransform.invert();
    }

    // BoundVolume from 3d world to 3d eye
    const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
    const drawinglayer::geometry::ViewInformation3D& aViewInfo3D(rVCScene.getViewInformation3D());
    basegfx::B3DRange aEyeVol(pScene->GetBoundVolume());
    aEyeVol.transform(aViewInfo3D.getOrientation());

    if ((aRect.GetWidth() == 0) || (aRect.GetHeight() == 0))
        throw o3tl::divide_by_zero();

    // build relative movement vector in eye coordinates
    basegfx::B3DPoint aMove(
        static_cast<double>(rSize.Width()) * aEyeVol.getWidth() / static_cast<double>(aRect.GetWidth()),
        static_cast<double>(-rSize.Height()) * aEyeVol.getHeight() / static_cast<double>(aRect.GetHeight()),
        0.0);
    basegfx::B3DPoint aPos(0.0, 0.0, 0.0);

    // movement vector to local coordinates of objects' parent
    basegfx::B3DHomMatrix aInverseOrientation(aViewInfo3D.getOrientation());
    aInverseOrientation.invert();
    basegfx::B3DHomMatrix aCompleteTrans(aInvDispTransform * aInverseOrientation);

    aMove = aCompleteTrans * aMove;
    aPos = aCompleteTrans * aPos;

    // build transformation and apply
    basegfx::B3DHomMatrix aTranslate;
    aTranslate.translate(aMove.getX() - aPos.getX(), aMove.getY() - aPos.getY(), aMove.getZ() - aPos.getZ());

    E3DModifySceneSnapRectUpdater aUpdater(pScene);
    SetTransform(aTranslate * GetTransform());
}

void E3dObject::RecalcSnapRect()
{
    maSnapRect = tools::Rectangle();
}

// Inform parent of changes in the structure (eg by transformation), in this
// process the object in which the change has occurred is returned.
void E3dObject::StructureChanged()
{
    E3dScene* pParent(getParentE3dSceneFromE3dObject());

    if(nullptr != pParent)
    {
        pParent->InvalidateBoundVolume();
        pParent->StructureChanged();
    }
}

E3dScene* E3dObject::getParentE3dSceneFromE3dObject() const
{
    return dynamic_cast< E3dScene* >(getParentSdrObjectFromSdrObject());
}

// Determine the top-level scene object
E3dScene* E3dObject::getRootE3dSceneFromE3dObject() const
{
    E3dScene* pParent(getParentE3dSceneFromE3dObject());

    if(nullptr != pParent)
    {
        return pParent->getRootE3dSceneFromE3dObject();
    }

    return nullptr;
}

// Calculate enclosed volume, including all child objects
basegfx::B3DRange E3dObject::RecalcBoundVolume() const
{
    basegfx::B3DRange aRetval;
    const sdr::contact::ViewContactOfE3d* pVCOfE3D = dynamic_cast< const sdr::contact::ViewContactOfE3d* >(&GetViewContact());

    if(pVCOfE3D)
    {
        // BoundVolume is without 3D object transformation, use correct sequence
        const drawinglayer::primitive3d::Primitive3DContainer& xLocalSequence(pVCOfE3D->getVIP3DSWithoutObjectTransform());

        if(!xLocalSequence.empty())
        {
            const uno::Sequence< beans::PropertyValue > aEmptyParameters;
            const drawinglayer::geometry::ViewInformation3D aLocalViewInformation3D(aEmptyParameters);

            aRetval = xLocalSequence.getB3DRange(aLocalViewInformation3D);
        }
    }

    return aRetval;
}

// Get enclosed volume and possibly recalculate it
const basegfx::B3DRange& E3dObject::GetBoundVolume() const
{
    if(maLocalBoundVol.isEmpty())
    {
        const_cast< E3dObject* >(this)->maLocalBoundVol = RecalcBoundVolume();
    }

    return maLocalBoundVol;
}

void E3dObject::InvalidateBoundVolume()
{
    maLocalBoundVol.reset();
}

// Pass on the changes in transformation to all child objects
void E3dObject::SetTransformChanged()
{
    InvalidateBoundVolume();
    mbTfHasChanged = true;
}

// Define the hierarchical transformation over all Parents, store in
// maFullTransform and return them
const basegfx::B3DHomMatrix& E3dObject::GetFullTransform() const
{
    if(mbTfHasChanged)
    {
        basegfx::B3DHomMatrix aNewFullTransformation(maTransformation);
        E3dScene* pParent(getParentE3dSceneFromE3dObject());

        if(nullptr != pParent)
        {
            aNewFullTransformation = pParent->GetFullTransform() * aNewFullTransformation;
        }

        const_cast< E3dObject* >(this)->maFullTransform = aNewFullTransformation;
        const_cast< E3dObject* >(this)->mbTfHasChanged = false;
    }

    return maFullTransform;
}

void E3dObject::NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(maTransformation != rMatrix)
    {
        maTransformation = rMatrix;
        SetTransformChanged();
        StructureChanged();
    }
}

// Set transformation matrix with repaint broadcast
void E3dObject::SetTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(rMatrix != maTransformation)
    {
        NbcSetTransform(rMatrix);
        SetChanged();
        BroadcastObjectChange();
        if (pUserCall != nullptr) pUserCall->Changed(*this, SdrUserCallType::Resize, tools::Rectangle());
    }
}

basegfx::B3DPolyPolygon E3dObject::CreateWireframe() const
{
    const basegfx::B3DRange aBoundVolume(GetBoundVolume());
    return basegfx::utils::createCubePolyPolygonFromB3DRange(aBoundVolume);
}

// Get the name of the object (singular)
OUString E3dObject::TakeObjNameSingul() const
{
    OUStringBuffer sName(SvxResId(STR_ObjNameSingulObj3d));

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

// Get the name of the object (plural)
OUString E3dObject::TakeObjNamePlural() const
{
    return SvxResId(STR_ObjNamePluralObj3d);
}

E3dObject* E3dObject::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< E3dObject >(rTargetModel);
}

E3dObject& E3dObject::operator=(const E3dObject& rSource)
{
    if(this != &rSource)
    {
        // call parent
        SdrAttrObj::operator=(rSource);

        // BoundVol can be copied since also the children are copied
        maLocalBoundVol  = rSource.maLocalBoundVol;
        maTransformation = rSource.maTransformation;

        // Because the parent may have changed, definitely redefine the total
        // transformation next time
        SetTransformChanged();

        // Copy selection status
        mbIsSelected = rSource.mbIsSelected;
    }

    return *this;
}

SdrObjGeoData *E3dObject::NewGeoData() const
{
    return new E3DObjGeoData;
}

void E3dObject::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrAttrObj::SaveGeoData (rGeo);

    static_cast<E3DObjGeoData &>(rGeo).maLocalBoundVol  = maLocalBoundVol;
    static_cast<E3DObjGeoData &>(rGeo).maTransformation = maTransformation;
}

void E3dObject::RestGeoData(const SdrObjGeoData& rGeo)
{
    maLocalBoundVol = static_cast<const E3DObjGeoData &>(rGeo).maLocalBoundVol;
    E3DModifySceneSnapRectUpdater aUpdater(this);
    NbcSetTransform(static_cast<const E3DObjGeoData &>(rGeo).maTransformation);
    SdrAttrObj::RestGeoData (rGeo);
}

// 2D-rotation of a 3D-body, normally this is done by the scene itself.
// This is however a correct implementation, because everything that has
// happened is a rotation around the axis perpendicular to the screen and that
// is regardless of how the scene has been rotated up until now.
void E3dObject::NbcRotate(const Point& rRef, long nAngle, double sn, double cs)
{
    // So currently the glue points are defined relative to the scene aOutRect.
    // Before turning the glue points are defined relative to the page. They
    // take no part in the rotation of the scene. To ensure this, there is the
    // SetGlueReallyAbsolute(sal_True);
    double fAngleInRad = basegfx::deg2rad(nAngle/100.0);

    basegfx::B3DHomMatrix aRotateZ;
    aRotateZ.rotate(0.0, 0.0, fAngleInRad);
    NbcSetTransform(aRotateZ * GetTransform());

    SetRectsDirty();        // This forces a recalculation of all BoundRects
    NbcRotateGluePoints(rRef,nAngle,sn,cs);  // Rotate the glue points (who still
                                            // have coordinates relative to the
                                            // original page)
    SetGlueReallyAbsolute(false);       // from now they are again relative to BoundRect (that is defined as aOutRect)
}

std::unique_ptr<sdr::properties::BaseProperties> E3dCompoundObject::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::E3dCompoundProperties>(*this);
}

E3dCompoundObject::E3dCompoundObject(SdrModel& rSdrModel)
:   E3dObject(rSdrModel)
{
}

E3dCompoundObject::~E3dCompoundObject ()
{
}

basegfx::B2DPolyPolygon E3dCompoundObject::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aRetval;
    const uno::Sequence< beans::PropertyValue > aEmptyParameters;
    drawinglayer::geometry::ViewInformation3D aViewInfo3D(aEmptyParameters);
    E3dScene* pRootScene = fillViewInformation3DForCompoundObject(aViewInfo3D, *this);

    if(pRootScene)
    {
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
        const basegfx::B3DPolyPolygon aCubePolyPolygon(CreateWireframe());
        aRetval = basegfx::utils::createB2DPolyPolygonFromB3DPolyPolygon(aCubePolyPolygon,
            aViewInfo3D.getObjectToView() * GetTransform());
        aRetval.transform(rVCScene.getObjectTransformation());
    }

    return aRetval;
}

sal_uInt32 E3dCompoundObject::GetHdlCount() const
{
    // 8 corners + 1 E3dVolumeMarker (= Wireframe representation)
    return 9L;
}

void E3dCompoundObject::AddToHdlList(SdrHdlList& rHdlList) const
{
    const uno::Sequence< beans::PropertyValue > aEmptyParameters;
    drawinglayer::geometry::ViewInformation3D aViewInfo3D(aEmptyParameters);
    E3dScene* pRootScene = fillViewInformation3DForCompoundObject(aViewInfo3D, *this);

    if(pRootScene)
    {
        const basegfx::B3DRange aBoundVolume(GetBoundVolume());

        if(!aBoundVolume.isEmpty())
        {
            const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());

            for(sal_uInt32 a(0); a < 8; a++)
            {
                basegfx::B3DPoint aPos3D;

                switch(a)
                {
                    case 0 : aPos3D.setX(aBoundVolume.getMinX()); aPos3D.setY(aBoundVolume.getMinY()); aPos3D.setZ(aBoundVolume.getMinZ()); break;
                    case 1 : aPos3D.setX(aBoundVolume.getMinX()); aPos3D.setY(aBoundVolume.getMinY()); aPos3D.setZ(aBoundVolume.getMaxZ()); break;
                    case 2 : aPos3D.setX(aBoundVolume.getMinX()); aPos3D.setY(aBoundVolume.getMaxY()); aPos3D.setZ(aBoundVolume.getMinZ()); break;
                    case 3 : aPos3D.setX(aBoundVolume.getMinX()); aPos3D.setY(aBoundVolume.getMaxY()); aPos3D.setZ(aBoundVolume.getMaxZ()); break;
                    case 4 : aPos3D.setX(aBoundVolume.getMaxX()); aPos3D.setY(aBoundVolume.getMinY()); aPos3D.setZ(aBoundVolume.getMinZ()); break;
                    case 5 : aPos3D.setX(aBoundVolume.getMaxX()); aPos3D.setY(aBoundVolume.getMinY()); aPos3D.setZ(aBoundVolume.getMaxZ()); break;
                    case 6 : aPos3D.setX(aBoundVolume.getMaxX()); aPos3D.setY(aBoundVolume.getMaxY()); aPos3D.setZ(aBoundVolume.getMinZ()); break;
                    case 7 : aPos3D.setX(aBoundVolume.getMaxX()); aPos3D.setY(aBoundVolume.getMaxY()); aPos3D.setZ(aBoundVolume.getMaxZ()); break;
                }

                // to 3d view coor
                aPos3D *= aViewInfo3D.getObjectToView() * GetTransform();

                // create 2d relative scene
                basegfx::B2DPoint aPos2D(aPos3D.getX(), aPos3D.getY());

                // to 2d world coor
                aPos2D *= rVCScene.getObjectTransformation();

                rHdlList.AddHdl(std::make_unique<SdrHdl>(Point(basegfx::fround(aPos2D.getX()), basegfx::fround(aPos2D.getY())), SdrHdlKind::BezierWeight));
            }
        }
    }

    const basegfx::B2DPolyPolygon aPolyPolygon(TakeXorPoly());

    if(aPolyPolygon.count())
    {
        rHdlList.AddHdl(std::make_unique<E3dVolumeMarker>(aPolyPolygon));
    }
}

sal_uInt16 E3dCompoundObject::GetObjIdentifier() const
{
    return E3D_COMPOUNDOBJ_ID;
}

void E3dCompoundObject::RecalcSnapRect()
{
    const uno::Sequence< beans::PropertyValue > aEmptyParameters;
    drawinglayer::geometry::ViewInformation3D aViewInfo3D(aEmptyParameters);
    E3dScene* pRootScene = fillViewInformation3DForCompoundObject(aViewInfo3D, *this);
    maSnapRect = tools::Rectangle();

    if(pRootScene)
    {
        // get VC of 3D candidate
        const sdr::contact::ViewContactOfE3d* pVCOfE3D = dynamic_cast< const sdr::contact::ViewContactOfE3d* >(&GetViewContact());

        if(pVCOfE3D)
        {
            // get 3D primitive sequence
            const drawinglayer::primitive3d::Primitive3DContainer xLocalSequence(pVCOfE3D->getViewIndependentPrimitive3DContainer());

            if(!xLocalSequence.empty())
            {
                // get BoundVolume
                basegfx::B3DRange aBoundVolume(xLocalSequence.getB3DRange(aViewInfo3D));

                // transform bound volume to relative scene coordinates
                aBoundVolume.transform(aViewInfo3D.getObjectToView());

                // build 2d relative scene range
                basegfx::B2DRange aSnapRange(
                    aBoundVolume.getMinX(), aBoundVolume.getMinY(),
                    aBoundVolume.getMaxX(), aBoundVolume.getMaxY());

                // transform to 2D world coordinates
                const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
                aSnapRange.transform(rVCScene.getObjectTransformation());

                // snap to integer
                maSnapRect = tools::Rectangle(
                    sal_Int32(floor(aSnapRange.getMinX())), sal_Int32(floor(aSnapRange.getMinY())),
                    sal_Int32(ceil(aSnapRange.getMaxX())), sal_Int32(ceil(aSnapRange.getMaxY())));
            }
        }
    }
}

E3dCompoundObject* E3dCompoundObject::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< E3dCompoundObject >(rTargetModel);
}

E3dCompoundObject& E3dCompoundObject::operator=(const E3dCompoundObject& rObj)
{
    if( this == &rObj )
        return *this;
    E3dObject::operator=(rObj);
    return *this;
}

// convert given basegfx::B3DPolyPolygon to screen coor
basegfx::B2DPolyPolygon E3dCompoundObject::TransformToScreenCoor(const basegfx::B3DPolyPolygon& rCandidate)
{
    const uno::Sequence< beans::PropertyValue > aEmptyParameters;
    drawinglayer::geometry::ViewInformation3D aViewInfo3D(aEmptyParameters);
    E3dScene* pRootScene = fillViewInformation3DForCompoundObject(aViewInfo3D, *this);
    basegfx::B2DPolyPolygon aRetval;

    if(pRootScene)
    {
        aRetval = basegfx::utils::createB2DPolyPolygonFromB3DPolyPolygon(rCandidate,
            aViewInfo3D.getObjectToView() * GetTransform());
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
        aRetval.transform(rVCScene.getObjectTransformation());
    }

    return aRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
