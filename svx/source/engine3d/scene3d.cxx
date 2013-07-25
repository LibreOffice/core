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


#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include "svx/svditer.hxx"

#include <stdlib.h>
#include <svx/globl3d.hxx>
#include <svx/svdpage.hxx>
#include <svl/style.hxx>
#include <svx/scene3d.hxx>
#include <svx/e3dundo.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svxids.hrc>
#include <editeng/colritem.hxx>
#include <svx/e3ditem.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/svx3ditems.hxx>
#include <svl/whiter.hxx>
#include <svx/xflftrit.hxx>
#include <svx/sdr/properties/e3dsceneproperties.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <svx/svddrag.hxx>
#include <helperminimaldepth3d.hxx>
#include <algorithm>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/e3dsceneupdater.hxx>

//////////////////////////////////////////////////////////////////////////////

class ImpRemap3DDepth
{
    sal_uInt32                  mnOrdNum;
    double                      mfMinimalDepth;

    // bit field
    bool                        mbIsScene : 1;

public:
    ImpRemap3DDepth(sal_uInt32 nOrdNum, double fMinimalDepth);
    ImpRemap3DDepth(sal_uInt32 nOrdNum);
    ~ImpRemap3DDepth();

    // for ::std::sort
    bool operator<(const ImpRemap3DDepth& rComp) const;

    sal_uInt32 GetOrdNum() const { return mnOrdNum; }
    bool IsScene() const { return mbIsScene; }
};

ImpRemap3DDepth::ImpRemap3DDepth(sal_uInt32 nOrdNum, double fMinimalDepth)
:   mnOrdNum(nOrdNum),
    mfMinimalDepth(fMinimalDepth),
    mbIsScene(false)
{
}

ImpRemap3DDepth::ImpRemap3DDepth(sal_uInt32 nOrdNum)
:   mnOrdNum(nOrdNum),
    mfMinimalDepth(0.0),
    mbIsScene(true)
{
}

ImpRemap3DDepth::~ImpRemap3DDepth()
{
}

bool ImpRemap3DDepth::operator<(const ImpRemap3DDepth& rComp) const
{
    if(IsScene())
    {
        return false;
    }
    else
    {
        if(rComp.IsScene())
        {
            return true;
        }
        else
        {
            return mfMinimalDepth < rComp.mfMinimalDepth;
        }
    }
}

// typedefs for a vector of ImpRemap3DDepths
typedef ::std::vector< ImpRemap3DDepth > ImpRemap3DDepthVector;

//////////////////////////////////////////////////////////////////////////////

class Imp3DDepthRemapper
{
    ImpRemap3DDepthVector       maVector;

public:
    Imp3DDepthRemapper(E3dScene& rScene);
    ~Imp3DDepthRemapper();

    sal_uInt32 RemapOrdNum(sal_uInt32 nOrdNum) const;
};

Imp3DDepthRemapper::Imp3DDepthRemapper(E3dScene& rScene)
{
    // only called when rScene.GetSubList() and nObjCount > 1L
    SdrObjList* pList = rScene.GetSubList();
    const sal_uInt32 nObjCount(pList->GetObjCount());

    for(sal_uInt32 a(0L); a < nObjCount; a++)
    {
        SdrObject* pCandidate = pList->GetObj(a);

        if(pCandidate)
        {
            if(pCandidate->ISA(E3dCompoundObject))
            {
                // single 3d object, calc depth
                const double fMinimalDepth(getMinimalDepthInViewCoordinates(static_cast< const E3dCompoundObject& >(*pCandidate)));
                ImpRemap3DDepth aEntry(a, fMinimalDepth);
                maVector.push_back(aEntry);
            }
            else
            {
                // scene, use standard entry for scene
                ImpRemap3DDepth aEntry(a);
                maVector.push_back(aEntry);
            }
        }
    }

    // now, we need to sort the maVector by it's members minimal depth. The
    // smaller, the nearer to the viewer.
    ::std::sort(maVector.begin(), maVector.end());
}

Imp3DDepthRemapper::~Imp3DDepthRemapper()
{
}

sal_uInt32 Imp3DDepthRemapper::RemapOrdNum(sal_uInt32 nOrdNum) const
{
    if(nOrdNum < maVector.size())
    {
        nOrdNum = maVector[(maVector.size() - 1) - nOrdNum].GetOrdNum();
    }

    return nOrdNum;
}

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* E3dScene::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dSceneProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* E3dScene::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfE3dScene(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dScene, E3dObject);

E3dScene::E3dScene()
:   E3dObject(),
    aCamera(basegfx::B3DPoint(0.0, 0.0, 4.0), basegfx::B3DPoint()),
    mp3DDepthRemapper(0L),
    bDrawOnlySelected(false)
{
    // Set defaults
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

E3dScene::E3dScene(E3dDefaultAttributes& rDefault)
:   E3dObject(),
    aCamera(basegfx::B3DPoint(0.0, 0.0, 4.0), basegfx::B3DPoint()),
    mp3DDepthRemapper(0L),
    bDrawOnlySelected(false)
{
    // Set defaults
    SetDefaultAttributes(rDefault);
}

void E3dScene::SetDefaultAttributes(E3dDefaultAttributes& /*rDefault*/)
{
    // For WIN95/NT turn off the FP-Exceptions
#if defined(WNT)
    _control87( _MCW_EM, _MCW_EM );
#endif

    // Set defaults
    aCamera.SetViewWindow(-2, -2, 4, 4);
    aCameraSet.SetDeviceRectangle(-2, 2, -2, 2);
    aCamera.SetDeviceWindow(Rectangle(0, 0, 10, 10));
    Rectangle aRect(0, 0, 10, 10);
    aCameraSet.SetViewportRectangle(aRect);

    // set defaults for Camera from ItemPool
    aCamera.SetProjection(GetPerspective());
    basegfx::B3DPoint aActualPosition(aCamera.GetPosition());
    double fNew = GetDistance();

    if(fabs(fNew - aActualPosition.getZ()) > 1.0)
    {
        aCamera.SetPosition( basegfx::B3DPoint( aActualPosition.getX(), aActualPosition.getY(), fNew) );
    }

    fNew = GetFocalLength() / 100.0;
    aCamera.SetFocalLength(fNew);
}

E3dScene::~E3dScene()
{
    ImpCleanup3DDepthMapper();
}

basegfx::B2DPolyPolygon E3dScene::TakeXorPoly() const
{
    const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(GetViewContact());
    const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());
    const basegfx::B3DPolyPolygon aCubePolyPolygon(CreateWireframe());

    basegfx::B2DPolyPolygon aRetval(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(aCubePolyPolygon,
        aViewInfo3D.getObjectToView()));
    aRetval.transform(rVCScene.getObjectTransformation());

    return aRetval;
}

void E3dScene::ImpCleanup3DDepthMapper()
{
    if(mp3DDepthRemapper)
    {
        delete mp3DDepthRemapper;
        mp3DDepthRemapper = 0L;
    }
}

sal_uInt32 E3dScene::RemapOrdNum(sal_uInt32 nNewOrdNum) const
{
    if(!mp3DDepthRemapper)
    {
        const sal_uInt32 nObjCount(GetSubList() ? GetSubList()->GetObjCount() : 0L);

        if(nObjCount > 1L)
        {
            ((E3dScene*)this)->mp3DDepthRemapper = new Imp3DDepthRemapper((E3dScene&)(*this));
        }
    }

    if(mp3DDepthRemapper)
    {
        return mp3DDepthRemapper->RemapOrdNum(nNewOrdNum);
    }

    return nNewOrdNum;
}

sal_uInt16 E3dScene::GetObjIdentifier() const
{
    return E3D_SCENE_ID;
}

void E3dScene::SetBoundRectDirty()
{
    E3dScene* pScene = GetScene();

    if(pScene == this)
    {
        // avoid resetting aOutRect which in case of a 3D scene used as 2d object
        // is model data,not re-creatable view data
    }
    else
    {
        // if not the outmost scene it is used as group in 3d, call parent
        E3dObject::SetBoundRectDirty();
    }
}

void E3dScene::NbcSetSnapRect(const Rectangle& rRect)
{
    SetRectsDirty();
    E3dObject::NbcSetSnapRect(rRect);
    aCamera.SetDeviceWindow(rRect);
    aCameraSet.SetViewportRectangle((Rectangle&)rRect);

    ImpCleanup3DDepthMapper();
}

void E3dScene::NbcMove(const Size& rSize)
{
    Rectangle aNewSnapRect = GetSnapRect();
    MoveRect(aNewSnapRect, rSize);
    NbcSetSnapRect(aNewSnapRect);
}

void E3dScene::NbcResize(const Point& rRef, const Fraction& rXFact,
                                            const Fraction& rYFact)
{
    Rectangle aNewSnapRect = GetSnapRect();
    ResizeRect(aNewSnapRect, rRef, rXFact, rYFact);
    NbcSetSnapRect(aNewSnapRect);
}

// Set new camera, and thus mark the scene and if possible the bound volume
// as changed

void E3dScene::SetCamera(const Camera3D& rNewCamera)
{
    // Set old camera
    aCamera = rNewCamera;
    ((sdr::properties::E3dSceneProperties&)GetProperties()).SetSceneItemsFromCamera();

    SetRectsDirty();

    // Fill new camera from old
    Camera3D& rCam = (Camera3D&)GetCamera();

    // Turn off ratio
    if(rCam.GetAspectMapping() == AS_NO_MAPPING)
        GetCameraSet().SetRatio(0.0);

    // Set Imaging geometry
    basegfx::B3DPoint aVRP(rCam.GetViewPoint());
    basegfx::B3DVector aVPN(aVRP - rCam.GetVRP());
    basegfx::B3DVector aVUV(rCam.GetVUV());

    // use SetViewportValues() to set VRP, VPN and VUV as vectors, too.
    // Else these values would not be exported/imported correctly.
    GetCameraSet().SetViewportValues(aVRP, aVPN, aVUV);

    // Set perspective
    GetCameraSet().SetPerspective(rCam.GetProjection() == PR_PERSPECTIVE);
    GetCameraSet().SetViewportRectangle((Rectangle&)rCam.GetDeviceWindow());

    ImpCleanup3DDepthMapper();
}

void E3dScene::NewObjectInserted(const E3dObject* p3DObj)
{
    E3dObject::NewObjectInserted(p3DObj);

    if ( p3DObj == this )
        return;

    ImpCleanup3DDepthMapper();
}

// Inform parent of changes of a child

void E3dScene::StructureChanged()
{
    E3dObject::StructureChanged();
    SetRectsDirty();

    ImpCleanup3DDepthMapper();
}

// Determine the overall scene object

E3dScene* E3dScene::GetScene() const
{
    if(GetParentObj())
        return GetParentObj()->GetScene();
    else
        return (E3dScene*)this;
}

void E3dScene::removeAllNonSelectedObjects()
{
    E3DModifySceneSnapRectUpdater aUpdater(this);

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        SdrObject* pObj = maSubList.GetObj(a);

        if(pObj)
        {
            bool bRemoveObject(false);

            if(pObj->ISA(E3dScene))
            {
                E3dScene* pScene = (E3dScene*)pObj;

                // iterate over this sub-scene
                pScene->removeAllNonSelectedObjects();

                // check object count. Empty scenes can be deleted
                const sal_uInt32 nObjCount(pScene->GetSubList() ? pScene->GetSubList()->GetObjCount() : 0);

                if(!nObjCount)
                {
                    // all objects removed, scene can be removed, too
                    bRemoveObject = true;
                }
            }
            else if(pObj->ISA(E3dCompoundObject))
            {
                E3dCompoundObject* pCompound = (E3dCompoundObject*)pObj;

                if(!pCompound->GetSelected())
                {
                    bRemoveObject = true;
                }
            }

            if(bRemoveObject)
            {
                maSubList.NbcRemoveObject(pObj->GetOrdNum());
                a--;
                SdrObject::Free(pObj);
            }
        }
    }
}

E3dScene* E3dScene::Clone() const
{
    return CloneHelper< E3dScene >();
}

E3dScene& E3dScene::operator=(const E3dScene& rObj)
{
    if( this == &rObj )
        return *this;
    E3dObject::operator=(rObj);

    const E3dScene& r3DObj = (const E3dScene&) rObj;
    aCamera          = r3DObj.aCamera;

    aCameraSet = r3DObj.aCameraSet;
    ((sdr::properties::E3dSceneProperties&)GetProperties()).SetSceneItemsFromCamera();

    InvalidateBoundVolume();
    RebuildLists();
    SetRectsDirty();

    ImpCleanup3DDepthMapper();

    // #i101941#
    // After a Scene as model object is cloned, the used
    // ViewContactOfE3dScene is created and partially used
    // to calculate Bound/SnapRects, but - since quite some
    // values are buffered at the VC - not really well
    // initialized. It would be possible to always watch for
    // preconditions of buffered data, but this would be expensive
    // and would create a lot of short living data structures.
    // It is currently better to flush that data, e.g. by using
    // ActionChanged at the VC which will for this class
    // flush that cached data and initalize it's valid reconstruction
    GetViewContact().ActionChanged();
    return *this;
}

// Rebuild Light- and label- object lists rebuild (after loading, allocation)

void E3dScene::RebuildLists()
{
    // first delete
    SdrLayerID nCurrLayerID = GetLayer();

    SdrObjListIter a3DIterator(maSubList, IM_FLAT);

    // then examine all the objects in the scene
    while ( a3DIterator.IsMore() )
    {
        E3dObject* p3DObj = (E3dObject*) a3DIterator.Next();
        p3DObj->NbcSetLayer(nCurrLayerID);
        NewObjectInserted(p3DObj);
    }
}

SdrObjGeoData *E3dScene::NewGeoData() const
{
    return new E3DSceneGeoData;
}

void E3dScene::SaveGeoData(SdrObjGeoData& rGeo) const
{
    E3dObject::SaveGeoData (rGeo);

    ((E3DSceneGeoData &) rGeo).aCamera = aCamera;
}

void E3dScene::RestGeoData(const SdrObjGeoData& rGeo)
{
    // #i94832# removed E3DModifySceneSnapRectUpdater here.
    // It should not be needed, is already part of E3dObject::RestGeoData
    E3dObject::RestGeoData (rGeo);
    SetCamera (((E3DSceneGeoData &) rGeo).aCamera);
}

// Something was changed in the style sheet, so change scene

void E3dScene::Notify(SfxBroadcaster &rBC, const SfxHint  &rHint)
{
    SetRectsDirty();
    E3dObject::Notify(rBC, rHint);
}

void E3dScene::RotateScene (const Point& rRef, long /*nWink*/, double sn, double cs)
{
    Point UpperLeft, LowerRight, Center, NewCenter;

    UpperLeft = aOutRect.TopLeft();
    LowerRight = aOutRect.BottomRight();

    long dxOutRectHalf = labs(UpperLeft.X() - LowerRight.X());
    dxOutRectHalf /= 2;
    long dyOutRectHalf = labs(UpperLeft.Y() - LowerRight.Y());
    dyOutRectHalf /= 2;

        // Only the center is moved. The corners are moved by NbcMove. For the
        // rotation a cartesian coordinate system is used in which the pivot
        // point is the origin, and the y-axis increases upward, the X-axis to
        // the right. This must be especially noted for the Y-values.
        // (When considering a flat piece of paper the Y-axis pointing downwards
    Center.X() = (UpperLeft.X() + dxOutRectHalf) - rRef.X();
    Center.Y() = -((UpperLeft.Y() + dyOutRectHalf) - rRef.Y());
                  // A few special cases has to be dealt with first (n * 90 degrees n integer)
    if (sn==1.0 && cs==0.0) { // 90deg
        NewCenter.X() = -Center.Y();
        NewCenter.Y() = -Center.X();
    } else if (sn==0.0 && cs==-1.0) { // 180deg
        NewCenter.X() = -Center.X();
        NewCenter.Y() = -Center.Y();
    } else if (sn==-1.0 && cs==0.0) { // 270deg
        NewCenter.X() =  Center.Y();
        NewCenter.Y() = -Center.X();
    }
    else          // Here it is rotated to any angle in the mathematically
                  // positive direction!
    {             // xnew = x * cos(alpha) - y * sin(alpha)
                  // ynew = x * sin(alpha) + y * cos(alpha)
                  // Bottom Right is not rotated: the pages of aOutRect must
                  // remain parallel to the coordinate axes.
        NewCenter.X() = (long) (Center.X() * cs - Center.Y() * sn);
        NewCenter.Y() = (long) (Center.X() * sn + Center.Y() * cs);
    }

    Size Differenz;
    Point DiffPoint = (NewCenter - Center);
    Differenz.Width() = DiffPoint.X();
    Differenz.Height() = -DiffPoint.Y();  // Note that the Y-axis is counted ad positive downward.
    NbcMove (Differenz);  // Actually executes the coordinate transformation.
}

OUString E3dScene::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulScene3d));

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

OUString E3dScene::TakeObjNamePlural() const
{
    return ImpGetResStr(STR_ObjNamePluralScene3d);
}

// The NbcRotate routine overloads the one of the SdrObject. The idea is
// to be able to rotate the scene relative to the position of the scene
// and then the objects in the scene

void E3dScene::NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(maTransformation != rMatrix)
    {
        // call parent
        E3dObject::NbcSetTransform(rMatrix);
    }
}

void E3dScene::SetTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(rMatrix != maTransformation)
    {
        // call parent
        E3dObject::SetTransform(rMatrix);
    }
}

void E3dScene::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    // So currently the glue points are defined relative to the scene aOutRect.
    // Before turning the glue points are defined relative to the page. They
    // take no part in the rotation of the scene. To ensure this, there is the
    // SetGlueReallyAbsolute(sal_True);

    // So that was the scene, now the objects used in the scene
    // 3D objects, if there is only one it can still have multiple surfaces but
    // the surfaces do not hve to be connected. This allows you to access child
    // objects. So going through the entire list and rotate around the Z axis
    // through the enter of aOutRect's (Steiner's theorem), so RotateZ

    RotateScene (rRef, nWink, sn, cs);  // Rotates the scene
    double fWinkelInRad = nWink/100 * F_PI180;

    basegfx::B3DHomMatrix aRotation;
    aRotation.rotate(0.0, 0.0, fWinkelInRad);
    NbcSetTransform(aRotation * GetTransform());

    SetRectsDirty();    // This forces a recalculation of all BoundRects
    NbcRotateGluePoints(rRef,nWink,sn,cs);  // Rotate the glue points (who still
                                            // have coordinates relative to the
                                            // original page)
    SetGlueReallyAbsolute(sal_False);  // from now they are again relative to BoundRect (that is defined as aOutRect)
    SetRectsDirty();
}

void E3dScene::RecalcSnapRect()
{
    E3dScene* pScene = GetScene();

    if(pScene == this)
    {
        // The Scene is used as a 2D-Objekt, take the SnapRect from the
        // 2D Display settings
        Camera3D& rCam = (Camera3D&)pScene->GetCamera();
        maSnapRect = rCam.GetDeviceWindow();
    }
    else
    {
        // The Scene itself is a member of another scene, get the SnapRect
        // as a composite object
        E3dObject::RecalcSnapRect();
    }
}

bool E3dScene::IsBreakObjPossible()
{
    // Break scene, if all members are able to break
    SdrObjListIter a3DIterator(maSubList, IM_DEEPWITHGROUPS);

    while ( a3DIterator.IsMore() )
    {
        E3dObject* pObj = (E3dObject*) a3DIterator.Next();
        DBG_ASSERT(pObj->ISA(E3dObject), "only 3D objects are allowed in scenes!");
        if(!pObj->IsBreakObjPossible())
            return false;
    }

    return true;
}

basegfx::B2DPolyPolygon E3dScene::TakeCreatePoly(const SdrDragStat& /*rDrag*/) const
{
    return TakeXorPoly();
}

bool E3dScene::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    NbcSetSnapRect(aRect1);
    return true;
}

bool E3dScene::MovCreate(SdrDragStat& rStat)
{
    Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    NbcSetSnapRect(aRect1);
    SetBoundRectDirty();
    bSnapRectDirty=sal_True;
    return true;
}

bool E3dScene::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    aRect1.Justify();
    NbcSetSnapRect(aRect1);
    SetRectsDirty();
    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

bool E3dScene::BckCreate(SdrDragStat& /*rStat*/)
{
    return false;
}

void E3dScene::BrkCreate(SdrDragStat& /*rStat*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
