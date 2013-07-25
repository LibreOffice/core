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
#include <svx/svdview.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include "svx/svditer.hxx"
#include "svx/globl3d.hxx"
#include <svx/camera3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/polysc3d.hxx>
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
#include <svx/sdr/properties/e3dproperties.hxx>
#include <svx/sdr/properties/e3dcompoundproperties.hxx>
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

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;


// List for 3D-Objects

TYPEINIT1(E3dObjList, SdrObjList);

E3dObjList::E3dObjList(SdrModel* pNewModel, SdrPage* pNewPage, E3dObjList* pNewUpList)
:   SdrObjList(pNewModel, pNewPage, pNewUpList)
{
}

E3dObjList::E3dObjList(const E3dObjList& rSrcList)
:   SdrObjList(rSrcList)
{
}

E3dObjList::~E3dObjList()
{
}

void E3dObjList::NbcInsertObject(SdrObject* pObj, sal_uIntPtr nPos, const SdrInsertReason* pReason)
{
    // Get owner
    DBG_ASSERT(GetOwnerObj()->ISA(E3dObject), "Insert 3D object in parent != 3DObject");

    // Is it even a 3D object?
    if(pObj && pObj->ISA(E3dObject))
    {
        // Normal 3D object, insert means
        // call parent
        SdrObjList::NbcInsertObject(pObj, nPos, pReason);
    }
    else
    {
        // No 3D object, inserted a page in place in a scene ...
        GetOwnerObj()->GetPage()->InsertObject(pObj, nPos);
    }
}

void E3dObjList::InsertObject(SdrObject* pObj, sal_uIntPtr nPos, const SdrInsertReason* pReason)
{
    OSL_ENSURE(GetOwnerObj()->ISA(E3dObject), "Insert 3D object in non-3D Parent");

    // call parent
    SdrObjList::InsertObject(pObj, nPos, pReason);

    E3dScene* pScene = ((E3dObject*)GetOwnerObj())->GetScene();
    if(pScene)
    {
        pScene->Cleanup3DDepthMapper();
    }
}

SdrObject* E3dObjList::NbcRemoveObject(sal_uIntPtr nObjNum)
{
    DBG_ASSERT(GetOwnerObj()->ISA(E3dObject), "Remove 3D object from Parent != 3DObject");

    // call parent
    SdrObject* pRetval = SdrObjList::NbcRemoveObject(nObjNum);

    E3dScene* pScene = ((E3dObject*)GetOwnerObj())->GetScene();
    if(pScene)
    {
        pScene->Cleanup3DDepthMapper();
    }

    return pRetval;
}

SdrObject* E3dObjList::RemoveObject(sal_uIntPtr nObjNum)
{
    OSL_ENSURE(GetOwnerObj()->ISA(E3dObject), "3D object is removed from non-3D Parent");

    // call parent
    SdrObject* pRetval = SdrObjList::RemoveObject(nObjNum);

    E3dScene* pScene = ((E3dObject*)GetOwnerObj())->GetScene();
    if(pScene)
    {
        pScene->Cleanup3DDepthMapper();
    }

    return pRetval;
}

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dObject::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dObject, SdrAttrObj);

E3dObject::E3dObject()
:   maSubList(),
    maLocalBoundVol(),
    maTransformation(),
    maFullTransform(),
    mbTfHasChanged(true),
    mbIsSelected(false)
{
    bIs3DObj = true;
    maSubList.SetOwnerObj(this);
    maSubList.SetListKind(SDROBJLIST_GROUPOBJ);
    bClosedObj = true;
}

E3dObject::~E3dObject()
{
}

void E3dObject::SetSelected(bool bNew)
{
    if((bool)mbIsSelected != bNew)
    {
        mbIsSelected = bNew;
    }

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetSelected(bNew);
        }
    }
}

// Break, default implementations

bool E3dObject::IsBreakObjPossible()
{
    return false;
}

SdrAttrObj* E3dObject::GetBreakObj()
{
    return 0L;
}

// SetRectsDirty must be done through the local SdrSubList

void E3dObject::SetRectsDirty(sal_Bool bNotMyself)
{
    // call parent
    SdrAttrObj::SetRectsDirty(bNotMyself);

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetRectsDirty(bNotMyself);
        }
    }
}

sal_uInt32 E3dObject::GetObjInventor() const
{
    return E3dInventor;
}

sal_uInt16 E3dObject::GetObjIdentifier() const
{
    return E3D_OBJECT_ID;
}

// Determine the capabilities of the object

void E3dObject::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bResizeFreeAllowed    = sal_True;
    rInfo.bResizePropAllowed    = sal_True;
    rInfo.bRotateFreeAllowed    = sal_True;
    rInfo.bRotate90Allowed      = sal_True;
    rInfo.bMirrorFreeAllowed    = sal_False;
    rInfo.bMirror45Allowed      = sal_False;
    rInfo.bMirror90Allowed      = sal_False;
    rInfo.bShearAllowed         = sal_False;
    rInfo.bEdgeRadiusAllowed    = sal_False;
    rInfo.bCanConvToPath        = sal_False;

    // no transparence for 3d objects
    rInfo.bTransparenceAllowed = sal_False;

    // gradient depends on fillstyle
    // BM *** check if SetItem is NULL ***
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetMergedItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.bGradientAllowed = (eFillStyle == XFILL_GRADIENT);

    // Convert 3D objects in a group of polygons:
    // At first not only possible, because the creation of a group of
    // 2D polygons would be required which need to be sorted by depth,
    // ie at intersections be cut relative to each other. Also the texture
    // coorinates were an unsolved problem.
    rInfo.bCanConvToPoly = sal_False;
    rInfo.bCanConvToContour = sal_False;
    rInfo.bCanConvToPathLineToArea = sal_False;
    rInfo.bCanConvToPolyLineToArea = sal_False;
}

void E3dObject::NbcSetLayer(SdrLayerID nLayer)
{
    SdrAttrObj::NbcSetLayer(nLayer);

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            pCandidate->NbcSetLayer(nLayer);
        }
    }
}

// Set ObjList also on SubList

void E3dObject::SetObjList(SdrObjList* pNewObjList)
{
    SdrObject::SetObjList(pNewObjList);
    maSubList.SetUpList(pNewObjList);
}

void E3dObject::SetPage(SdrPage* pNewPage)
{
    SdrAttrObj::SetPage(pNewPage);
    maSubList.SetPage(pNewPage);
}

void E3dObject::SetModel(SdrModel* pNewModel)
{
    SdrAttrObj::SetModel(pNewModel);
    maSubList.SetModel(pNewModel);
}

// resize object, used from old 2d interfaces, e.g. in Move/Scale dialog (F4)

void E3dObject::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    // Movement in X, Y in the eye coordinate system
    E3dScene* pScene = GetScene();

    if(pScene)
    {
        // transform pos from 2D world to 3D eye
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
        const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());
        basegfx::B2DPoint aScaleCenter2D((double)rRef.X(), (double)rRef.Y());
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
        basegfx::B3DHomMatrix mFullTransform(GetFullTransform());
        basegfx::B3DHomMatrix mTrans(mFullTransform);

        mTrans *= aViewInfo3D.getOrientation();
        mTrans.translate(-aScaleCenter3D.getX(), -aScaleCenter3D.getY(), -aScaleCenter3D.getZ());
        mTrans.scale(fScaleX, fScaleY, 1.0);
        mTrans.translate(aScaleCenter3D.getX(), aScaleCenter3D.getY(), aScaleCenter3D.getZ());
        mTrans *= aInverseOrientation;
        mFullTransform.invert();
        mTrans *= mFullTransform;

        // Apply
        basegfx::B3DHomMatrix mObjTrans(GetTransform());
        mObjTrans *= mTrans;

        E3DModifySceneSnapRectUpdater aUpdater(this);
        SetTransform(mObjTrans);
    }
}


// Move object in 2D is needed when using cursor keys

void E3dObject::NbcMove(const Size& rSize)
{
    // Movement in X, Y in the eye coordinate system
    E3dScene* pScene = GetScene();

    if(pScene)
    {
        //Dimensions of the scene in 3D and 2D for comparison
        Rectangle aRect = pScene->GetSnapRect();

        basegfx::B3DHomMatrix mInvDispTransform;
        if(GetParentObj())
        {
            mInvDispTransform = GetParentObj()->GetFullTransform();
            mInvDispTransform.invert();
        }

        // BoundVolume from 3d world to 3d eye
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
        const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());
        basegfx::B3DRange aEyeVol(pScene->GetBoundVolume());
        aEyeVol.transform(aViewInfo3D.getOrientation());

        // build relative movement vector in eye coordinates
        basegfx::B3DPoint aMove(
            (double)rSize.Width() * aEyeVol.getWidth() / (double)aRect.GetWidth(),
            (double)-rSize.Height() * aEyeVol.getHeight() / (double)aRect.GetHeight(),
            0.0);
        basegfx::B3DPoint aPos(0.0, 0.0, 0.0);

        // movement vector to local coordinates of objects' parent
        basegfx::B3DHomMatrix aInverseOrientation(aViewInfo3D.getOrientation());
        aInverseOrientation.invert();
        basegfx::B3DHomMatrix aCompleteTrans(mInvDispTransform * aInverseOrientation);

        aMove = aCompleteTrans * aMove;
        aPos = aCompleteTrans * aPos;

        // build transformation and apply
        basegfx::B3DHomMatrix aTranslate;
        aTranslate.translate(aMove.getX() - aPos.getX(), aMove.getY() - aPos.getY(), aMove.getZ() - aPos.getZ());

        E3DModifySceneSnapRectUpdater aUpdater(pScene);
        SetTransform(aTranslate * GetTransform());
    }
}

// Return the sublist, but only if it contains objects!

SdrObjList* E3dObject::GetSubList() const
{
    return &(const_cast< E3dObjList& >(maSubList));
}

void E3dObject::RecalcSnapRect()
{
    maSnapRect = Rectangle();

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            maSnapRect.Union(pCandidate->GetSnapRect());
        }
    }
}

// Inform the parent about insertion of a 3D object, so that the parent is able
// treat the particualar objects in a special way (eg Light / Label in E3dScene)

void E3dObject::NewObjectInserted(const E3dObject* p3DObj)
{
    if(GetParentObj())
        GetParentObj()->NewObjectInserted(p3DObj);
}

// Inform parent of changes in the structure (eg by transformation), in this
// process the object in which the change has occurred is returned.

void E3dObject::StructureChanged()
{
    if ( GetParentObj() )
    {
        GetParentObj()->InvalidateBoundVolume();
        GetParentObj()->StructureChanged();
    }
}

void E3dObject::Insert3DObj(E3dObject* p3DObj)
{
    DBG_ASSERT(p3DObj, "Insert3DObj with NULL-pointer!");
    SdrPage* pPg = pPage;
    maSubList.InsertObject(p3DObj);
    pPage = pPg;
    InvalidateBoundVolume();
    NewObjectInserted(p3DObj);
    StructureChanged();
}

void E3dObject::Remove3DObj(E3dObject* p3DObj)
{
    DBG_ASSERT(p3DObj, "Remove3DObj with NULL-pointer!");

    if(p3DObj->GetParentObj() == this)
    {
        SdrPage* pPg = pPage;
        maSubList.RemoveObject(p3DObj->GetOrdNum());
        pPage = pPg;

        InvalidateBoundVolume();
        StructureChanged();
    }
}

E3dObject* E3dObject::GetParentObj() const
{
    E3dObject* pRetval = NULL;

    if(GetObjList()
        && GetObjList()->GetOwnerObj()
        && GetObjList()->GetOwnerObj()->ISA(E3dObject))
        pRetval = ((E3dObject*)GetObjList()->GetOwnerObj());
    return pRetval;
}

// Determine the top-level scene object

E3dScene* E3dObject::GetScene() const
{
    if(GetParentObj())
        return GetParentObj()->GetScene();
    return NULL;
}

// Calculate enclosed volume, including all child objects

basegfx::B3DRange E3dObject::RecalcBoundVolume() const
{
    basegfx::B3DRange aRetval;
    const sal_uInt32 nObjCnt(maSubList.GetObjCount());

    if(nObjCnt)
    {
        for(sal_uInt32 a(0); a < nObjCnt; a++)
        {
            const E3dObject* p3DObject = dynamic_cast< const E3dObject* >(maSubList.GetObj(a));

            if(p3DObject)
            {
                basegfx::B3DRange aLocalRange(p3DObject->GetBoundVolume());
                aLocalRange.transform(p3DObject->GetTransform());
                aRetval.expand(aLocalRange);
            }
        }
    }
    else
    {
        // single 3D object
        const sdr::contact::ViewContactOfE3d* pVCOfE3D = dynamic_cast< const sdr::contact::ViewContactOfE3d* >(&GetViewContact());

        if(pVCOfE3D)
        {
            // BoundVolume is without 3D object transformation, use correct sequence
            const drawinglayer::primitive3d::Primitive3DSequence xLocalSequence(pVCOfE3D->getVIP3DSWithoutObjectTransform());

            if(xLocalSequence.hasElements())
            {
                const uno::Sequence< beans::PropertyValue > aEmptyParameters;
                const drawinglayer::geometry::ViewInformation3D aLocalViewInformation3D(aEmptyParameters);

                aRetval = drawinglayer::primitive3d::getB3DRangeFromPrimitive3DSequence(
                    xLocalSequence, aLocalViewInformation3D);
            }
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

// Pass on the changes of the BoundVolumes to all child objects

void E3dObject::SetBoundVolInvalid()
{
    InvalidateBoundVolume();

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetBoundVolInvalid();
        }
    }
}

// Pass on the changes in transformation to all child objects

void E3dObject::SetTransformChanged()
{
    InvalidateBoundVolume();
    mbTfHasChanged = true;

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetTransformChanged();
        }
    }
}

// Define the hierarchical transformation over all Parents, store in
// maFullTransform and return them

const basegfx::B3DHomMatrix& E3dObject::GetFullTransform() const
{
    if(mbTfHasChanged)
    {
        basegfx::B3DHomMatrix aNewFullTransformation(maTransformation);

        if ( GetParentObj() )
        {
            aNewFullTransformation = GetParentObj()->GetFullTransform() * aNewFullTransformation;
        }

        const_cast< E3dObject* >(this)->maFullTransform = aNewFullTransformation;
        const_cast< E3dObject* >(this)->mbTfHasChanged = false;
    }

    return maFullTransform;
}

const basegfx::B3DHomMatrix& E3dObject::GetTransform() const
{
    return maTransformation;
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
        if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
    }
}

basegfx::B3DPolyPolygon E3dObject::CreateWireframe() const
{
    const basegfx::B3DRange aBoundVolume(GetBoundVolume());
    return basegfx::tools::createCubePolyPolygonFromB3DRange(aBoundVolume);
}

// Get the name of the object (singular)

OUString E3dObject::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulObj3d));

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

void E3dObject::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralObj3d);
}

E3dObject* E3dObject::Clone() const
{
    return CloneHelper< E3dObject >();
}

E3dObject& E3dObject::operator=(const E3dObject& rObj)
{
    if( this == &rObj )
        return *this;
    SdrObject::operator=(rObj);

    const E3dObject& r3DObj = (const E3dObject&) rObj;
    if (r3DObj.GetSubList())
    {
        maSubList.CopyObjects(*r3DObj.GetSubList());
    }

    // BoundVol can be copied since also the children are copied
    maLocalBoundVol  = r3DObj.maLocalBoundVol;
    maTransformation = r3DObj.maTransformation;

    // Because the parent may have changed, definitely redefine the total
    // transformation next time
    SetTransformChanged();

    // Copy selection status
    mbIsSelected = r3DObj.mbIsSelected;
    return *this;
}

SdrObjGeoData *E3dObject::NewGeoData() const
{
    return new E3DObjGeoData;
}

void E3dObject::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrAttrObj::SaveGeoData (rGeo);

    ((E3DObjGeoData &) rGeo).maLocalBoundVol  = maLocalBoundVol;
    ((E3DObjGeoData &) rGeo).maTransformation = maTransformation;
}

void E3dObject::RestGeoData(const SdrObjGeoData& rGeo)
{
    maLocalBoundVol = ((E3DObjGeoData &) rGeo).maLocalBoundVol;
    E3DModifySceneSnapRectUpdater aUpdater(this);
    NbcSetTransform(((E3DObjGeoData &) rGeo).maTransformation);
    SdrAttrObj::RestGeoData (rGeo);
}

// 2D-rotation of a 3D-body, normally this is done by the scene itself.
// This is however a correct implementation, because everything that has
// happened is a rotation around the axis perpendicular to the screen and that
// is regardless of how the scene has been rotated up until now.

void E3dObject::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    // So currently the glue points are defined relative to the scene aOutRect.
    // Before turning the glue points are defined relative to the page. They
    // take no part in the rotation of the scene. To ensure this, there is the
    // SetGlueReallyAbsolute(sal_True);

    double fWinkelInRad = nWink/100 * F_PI180;

    basegfx::B3DHomMatrix aRotateZ;
    aRotateZ.rotate(0.0, 0.0, fWinkelInRad);
    NbcSetTransform(aRotateZ * GetTransform());

    SetRectsDirty();        // This forces a recalculation of all BoundRects
    NbcRotateGluePoints(rRef,nWink,sn,cs);  // Rotate the glue points (who still
                                            // have coordinates relative to the
                                            // original page)
    SetGlueReallyAbsolute(sal_False);       // from now they are again relative to BoundRect (that is defined as aOutRect)
}

/*************************************************************************/

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dCompoundObject::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dCompoundProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dCompoundObject, E3dObject);

E3dCompoundObject::E3dCompoundObject()
:   E3dObject(),
    aMaterialAmbientColor(),
    bCreateNormals(false),
    bCreateTexture(false)
{
    // Set defaults
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

E3dCompoundObject::E3dCompoundObject(E3dDefaultAttributes& rDefault)
:   E3dObject(),
    aMaterialAmbientColor(),
    bCreateNormals(false),
    bCreateTexture(false)
{
    // Set defaults
    SetDefaultAttributes(rDefault);
}

void E3dCompoundObject::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    // Set defaults
    aMaterialAmbientColor = rDefault.GetDefaultAmbientColor();

    bCreateNormals = rDefault.GetDefaultCreateNormals();
    bCreateTexture = rDefault.GetDefaultCreateTexture();
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
        aRetval = basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(aCubePolyPolygon,
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

                rHdlList.AddHdl(new SdrHdl(Point(basegfx::fround(aPos2D.getX()), basegfx::fround(aPos2D.getY())), HDL_BWGT));
            }
        }
    }

    const basegfx::B2DPolyPolygon aPolyPolygon(TakeXorPoly());

    if(aPolyPolygon.count())
    {
        E3dVolumeMarker* pVolMarker = new E3dVolumeMarker(aPolyPolygon);
        rHdlList.AddHdl(pVolMarker);
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
    maSnapRect = Rectangle();

    if(pRootScene)
    {
        // get VC of 3D candidate
        const sdr::contact::ViewContactOfE3d* pVCOfE3D = dynamic_cast< const sdr::contact::ViewContactOfE3d* >(&GetViewContact());

        if(pVCOfE3D)
        {
            // get 3D primitive sequence
            const drawinglayer::primitive3d::Primitive3DSequence xLocalSequence(pVCOfE3D->getViewIndependentPrimitive3DSequence());

            if(xLocalSequence.hasElements())
            {
                // get BoundVolume
                basegfx::B3DRange aBoundVolume(drawinglayer::primitive3d::getB3DRangeFromPrimitive3DSequence(
                    xLocalSequence, aViewInfo3D));

                // transform bound volume to relative scene coordinates
                aBoundVolume.transform(aViewInfo3D.getObjectToView());

                // build 2d relative scene range
                basegfx::B2DRange aSnapRange(
                    aBoundVolume.getMinX(), aBoundVolume.getMinY(),
                    aBoundVolume.getMaxX(), aBoundVolume.getMaxY());

                // transform to 2D world coordiantes
                const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
                aSnapRange.transform(rVCScene.getObjectTransformation());

                // snap to integer
                maSnapRect = Rectangle(
                    sal_Int32(floor(aSnapRange.getMinX())), sal_Int32(floor(aSnapRange.getMinY())),
                    sal_Int32(ceil(aSnapRange.getMaxX())), sal_Int32(ceil(aSnapRange.getMaxY())));
            }
        }
    }
}

E3dCompoundObject* E3dCompoundObject::Clone() const
{
    return CloneHelper< E3dCompoundObject >();
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
        aRetval = basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(rCandidate,
            aViewInfo3D.getObjectToView() * GetTransform());
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
        aRetval.transform(rVCScene.getObjectTransformation());
    }

    return aRetval;
}

bool E3dCompoundObject::IsAOrdNumRemapCandidate(E3dScene*& prScene) const
{
    if(GetObjList()
        && GetObjList()->GetOwnerObj()
        && GetObjList()->GetOwnerObj()->ISA(E3dScene))
    {
        prScene = (E3dScene*)GetObjList()->GetOwnerObj();
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
