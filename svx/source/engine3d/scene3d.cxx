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
#include <svx/svdmodel.hxx>

//////////////////////////////////////////////////////////////////////////////

class ImpRemap3DDepth
{
    sal_uInt32                  mnOrdNum;
    double                      mfMinimalDepth;

    
    bool                        mbIsScene : 1;

public:
    ImpRemap3DDepth(sal_uInt32 nOrdNum, double fMinimalDepth);
    ImpRemap3DDepth(sal_uInt32 nOrdNum);
    ~ImpRemap3DDepth();

    
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
    
    SdrObjList* pList = rScene.GetSubList();
    const sal_uInt32 nObjCount(pList->GetObjCount());

    for(sal_uInt32 a(0L); a < nObjCount; a++)
    {
        SdrObject* pCandidate = pList->GetObj(a);

        if(pCandidate)
        {
            if(pCandidate->ISA(E3dCompoundObject))
            {
                
                const double fMinimalDepth(getMinimalDepthInViewCoordinates(static_cast< const E3dCompoundObject& >(*pCandidate)));
                ImpRemap3DDepth aEntry(a, fMinimalDepth);
                maVector.push_back(aEntry);
            }
            else
            {
                
                ImpRemap3DDepth aEntry(a);
                maVector.push_back(aEntry);
            }
        }
    }

    
    
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


sdr::properties::BaseProperties* E3dScene::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dSceneProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////


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
    
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

E3dScene::E3dScene(E3dDefaultAttributes& rDefault)
:   E3dObject(),
    aCamera(basegfx::B3DPoint(0.0, 0.0, 4.0), basegfx::B3DPoint()),
    mp3DDepthRemapper(0L),
    bDrawOnlySelected(false)
{
    
    SetDefaultAttributes(rDefault);
}

void E3dScene::SetDefaultAttributes(E3dDefaultAttributes& /*rDefault*/)
{
    
#if defined(WNT)
    _control87( _MCW_EM, _MCW_EM );
#endif

    
    aCamera.SetViewWindow(-2, -2, 4, 4);
    aCameraSet.SetDeviceRectangle(-2, 2, -2, 2);
    aCamera.SetDeviceWindow(Rectangle(0, 0, 10, 10));
    Rectangle aRect(0, 0, 10, 10);
    aCameraSet.SetViewportRectangle(aRect);

    
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
        
        
    }
    else
    {
        
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




void E3dScene::SetCamera(const Camera3D& rNewCamera)
{
    
    aCamera = rNewCamera;
    ((sdr::properties::E3dSceneProperties&)GetProperties()).SetSceneItemsFromCamera();

    SetRectsDirty();

    
    Camera3D& rCam = (Camera3D&)GetCamera();

    
    if(rCam.GetAspectMapping() == AS_NO_MAPPING)
        GetCameraSet().SetRatio(0.0);

    
    basegfx::B3DPoint aVRP(rCam.GetViewPoint());
    basegfx::B3DVector aVPN(aVRP - rCam.GetVRP());
    basegfx::B3DVector aVUV(rCam.GetVUV());

    
    
    GetCameraSet().SetViewportValues(aVRP, aVPN, aVUV);

    
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



void E3dScene::StructureChanged()
{
    E3dObject::StructureChanged();

    if(!GetModel() || !GetModel()->isLocked())
    {
        
        
        
        SetRectsDirty();
    }

    ImpCleanup3DDepthMapper();
}



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

                
                pScene->removeAllNonSelectedObjects();

                
                const sal_uInt32 nObjCount(pScene->GetSubList() ? pScene->GetSubList()->GetObjCount() : 0);

                if(!nObjCount)
                {
                    
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

    
    
    
    
    
    
    
    
    
    
    
    GetViewContact().ActionChanged();
    return *this;
}



void E3dScene::RebuildLists()
{
    
    SdrLayerID nCurrLayerID = GetLayer();

    SdrObjListIter a3DIterator(maSubList, IM_FLAT);

    
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
    
    
    E3dObject::RestGeoData (rGeo);
    SetCamera (((E3DSceneGeoData &) rGeo).aCamera);
}



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

        
        
        
        
        
    Center.X() = (UpperLeft.X() + dxOutRectHalf) - rRef.X();
    Center.Y() = -((UpperLeft.Y() + dyOutRectHalf) - rRef.Y());
                  
    if (sn==1.0 && cs==0.0) { 
        NewCenter.X() = -Center.Y();
        NewCenter.Y() = -Center.X();
    } else if (sn==0.0 && cs==-1.0) { 
        NewCenter.X() = -Center.X();
        NewCenter.Y() = -Center.Y();
    } else if (sn==-1.0 && cs==0.0) { 
        NewCenter.X() =  Center.Y();
        NewCenter.Y() = -Center.X();
    }
    else          
                  
    {             
                  
                  
                  
        NewCenter.X() = (long) (Center.X() * cs - Center.Y() * sn);
        NewCenter.Y() = (long) (Center.X() * sn + Center.Y() * cs);
    }

    Size Differenz;
    Point DiffPoint = (NewCenter - Center);
    Differenz.Width() = DiffPoint.X();
    Differenz.Height() = -DiffPoint.Y();  
    NbcMove (Differenz);  
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





void E3dScene::NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(maTransformation != rMatrix)
    {
        
        E3dObject::NbcSetTransform(rMatrix);
    }
}

void E3dScene::SetTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(rMatrix != maTransformation)
    {
        
        E3dObject::SetTransform(rMatrix);
    }
}

void E3dScene::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    
    
    
    

    
    
    
    
    

    RotateScene (rRef, nWink, sn, cs);  
    double fWinkelInRad = nWink/100 * F_PI180;

    basegfx::B3DHomMatrix aRotation;
    aRotation.rotate(0.0, 0.0, fWinkelInRad);
    NbcSetTransform(aRotation * GetTransform());

    SetRectsDirty();    
    NbcRotateGluePoints(rRef,nWink,sn,cs);  
                                            
                                            
    SetGlueReallyAbsolute(false);  
    SetRectsDirty();
}

void E3dScene::RecalcSnapRect()
{
    E3dScene* pScene = GetScene();

    if(pScene == this)
    {
        
        
        Camera3D& rCam = (Camera3D&)pScene->GetCamera();
        maSnapRect = rCam.GetDeviceWindow();
    }
    else
    {
        
        
        E3dObject::RecalcSnapRect();
    }
}

bool E3dScene::IsBreakObjPossible()
{
    
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
    bSnapRectDirty=true;
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
