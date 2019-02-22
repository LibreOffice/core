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

#include <fucon3d.hxx>
#include <vcl/waitobj.hxx>

#include <svx/svxids.hrc>
#include <svl/aeitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <tools/poly.hxx>

#include <math.h>
#include <svx/globl3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/camera3d.hxx>

#include <app.hrc>

#include <View.hxx>
#include <Window.hxx>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <ViewShellBase.hxx>
#include <ToolBarManager.hxx>
#include <svx/svx3ditems.hxx>

#include <basegfx/polygon/b2dpolygontools.hxx>

using namespace com::sun::star;

namespace sd {


FuConstruct3dObject::FuConstruct3dObject (
    ViewShell*  pViewSh,
    ::sd::Window*       pWin,
    ::sd::View*         pView,
    SdDrawDocument* pDoc,
    SfxRequest&     rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuConstruct3dObject::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuConstruct3dObject* pFunc;
    rtl::Reference<FuPoor> xFunc( pFunc = new FuConstruct3dObject( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent(bPermanent);
    return xFunc;
}

void FuConstruct3dObject::DoExecute( SfxRequest& rReq )
{
    FuConstruct::DoExecute( rReq );
    mpViewShell->GetViewShellBase().GetToolBarManager()->SetToolBar(
        ToolBarManager::ToolBarGroup::Function,
        ToolBarManager::msDrawingObjectToolBar);
}

E3dCompoundObject* FuConstruct3dObject::ImpCreateBasic3DShape()
{
    E3dCompoundObject* p3DObj = nullptr;

    switch (nSlotId)
    {
        default:
        case SID_3D_CUBE:
        {
            p3DObj = new E3dCubeObj(
                mpView->getSdrModelFromSdrView(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B3DPoint(-2500, -2500, -2500),
                ::basegfx::B3DVector(5000, 5000, 5000));
            break;
        }

        case SID_3D_SPHERE:
        {
            p3DObj = new E3dSphereObj(
                mpView->getSdrModelFromSdrView(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B3DPoint(0, 0, 0),
                ::basegfx::B3DVector(5000, 5000, 5000));
            break;
        }

        case SID_3D_SHELL:
        {
            XPolygon aXPoly(Point (0, 1250), 2500, 2500, 0, 900, false);
            aXPoly.Scale(5.0, 5.0);

            ::basegfx::B2DPolygon aB2DPolygon(aXPoly.getB2DPolygon());
            if(aB2DPolygon.areControlPointsUsed())
            {
                aB2DPolygon = ::basegfx::utils::adaptiveSubdivideByAngle(aB2DPolygon);
            }
            p3DObj = new E3dLatheObj(
                mpView->getSdrModelFromSdrView(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B2DPolyPolygon(aB2DPolygon));

            /* this is an open object, therefore it has to be handled double-
               sided by default */
            p3DObj->SetMergedItem(makeSvx3DDoubleSidedItem(true));
            break;
        }

        case SID_3D_HALF_SPHERE:
        {
            XPolygon aXPoly(Point (0, 1250), 2500, 2500, 0, 900, false);
            aXPoly.Scale(5.0, 5.0);

            aXPoly.Insert(0, Point (2400*5, 1250*5), PolyFlags::Normal);
            aXPoly.Insert(0, Point (2000*5, 1250*5), PolyFlags::Normal);
            aXPoly.Insert(0, Point (1500*5, 1250*5), PolyFlags::Normal);
            aXPoly.Insert(0, Point (1000*5, 1250*5), PolyFlags::Normal);
            aXPoly.Insert(0, Point (500*5, 1250*5), PolyFlags::Normal);
            aXPoly.Insert(0, Point (250*5, 1250*5), PolyFlags::Normal);
            aXPoly.Insert(0, Point (50*5, 1250*5), PolyFlags::Normal);
            aXPoly.Insert(0, Point (0,    1250*5), PolyFlags::Normal);

            ::basegfx::B2DPolygon aB2DPolygon(aXPoly.getB2DPolygon());
            if(aB2DPolygon.areControlPointsUsed())
            {
                aB2DPolygon = ::basegfx::utils::adaptiveSubdivideByAngle(aB2DPolygon);
            }
            p3DObj = new E3dLatheObj(
                mpView->getSdrModelFromSdrView(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B2DPolyPolygon(aB2DPolygon));
            break;
        }

        case SID_3D_TORUS:
        {
            ::basegfx::B2DPolygon aB2DPolygon(::basegfx::utils::createPolygonFromCircle(::basegfx::B2DPoint(1000.0, 0.0), 500.0));
            if(aB2DPolygon.areControlPointsUsed())
            {
                aB2DPolygon = ::basegfx::utils::adaptiveSubdivideByAngle(aB2DPolygon);
            }
            p3DObj = new E3dLatheObj(
                mpView->getSdrModelFromSdrView(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B2DPolyPolygon(aB2DPolygon));
            break;
        }

        case SID_3D_CYLINDER:
        {
            ::basegfx::B2DPolygon aInnerPoly;

            aInnerPoly.append(::basegfx::B2DPoint(0, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(50*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(100*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(200*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(300*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(400*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(450*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(500*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(500*5, -1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(450*5, -1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(400*5, -1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(300*5, -1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(200*5, -1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(100*5, -1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(50*5, -1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(0,    -1000*5));
            aInnerPoly.setClosed(true);

            p3DObj = new E3dLatheObj(
                mpView->getSdrModelFromSdrView(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B2DPolyPolygon(aInnerPoly));
            break;
        }

        case SID_3D_CONE:
        {
            ::basegfx::B2DPolygon aInnerPoly;

            aInnerPoly.append(::basegfx::B2DPoint(0, -1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(25*5, -900*5));
            aInnerPoly.append(::basegfx::B2DPoint(50*5, -800*5));
            aInnerPoly.append(::basegfx::B2DPoint(100*5, -600*5));
            aInnerPoly.append(::basegfx::B2DPoint(200*5, -200*5));
            aInnerPoly.append(::basegfx::B2DPoint(300*5, 200*5));
            aInnerPoly.append(::basegfx::B2DPoint(400*5, 600*5));
            aInnerPoly.append(::basegfx::B2DPoint(500*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(400*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(300*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(200*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(100*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(50*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(0,    1000*5));
            aInnerPoly.setClosed(true);

            p3DObj = new E3dLatheObj(
                mpView->getSdrModelFromSdrView(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B2DPolyPolygon(aInnerPoly));
            break;
        }

        case SID_3D_PYRAMID:
        {
            ::basegfx::B2DPolygon aInnerPoly;

            aInnerPoly.append(::basegfx::B2DPoint(0, -1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(25*5, -900*5));
            aInnerPoly.append(::basegfx::B2DPoint(50*5, -800*5));
            aInnerPoly.append(::basegfx::B2DPoint(100*5, -600*5));
            aInnerPoly.append(::basegfx::B2DPoint(200*5, -200*5));
            aInnerPoly.append(::basegfx::B2DPoint(300*5, 200*5));
            aInnerPoly.append(::basegfx::B2DPoint(400*5, 600*5));
            aInnerPoly.append(::basegfx::B2DPoint(500*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(400*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(300*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(200*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(100*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(50*5, 1000*5));
            aInnerPoly.append(::basegfx::B2DPoint(0, 1000*5));
            aInnerPoly.setClosed(true);

            p3DObj = new E3dLatheObj(
                mpView->getSdrModelFromSdrView(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B2DPolyPolygon(aInnerPoly));
            p3DObj->SetMergedItem(makeSvx3DHorizontalSegmentsItem(4));
            break;
        }
    }

    return p3DObj;
}

void FuConstruct3dObject::ImpPrepareBasic3DShape(E3dCompoundObject const * p3DObj, E3dScene *pScene)
{
    Camera3D aCamera  = pScene->GetCamera ();

    // get transformed BoundVolume of the new object
    basegfx::B3DRange aBoundVol;
    basegfx::B3DRange aObjVol(p3DObj->GetBoundVolume());
    aObjVol.transform(p3DObj->GetTransform());
    aBoundVol.expand(aObjVol);
    double fDeepth(aBoundVol.getDepth());

    aCamera.SetPRP(::basegfx::B3DPoint(0.0, 0.0, 1000.0));
    aCamera.SetPosition(::basegfx::B3DPoint(0.0, 0.0, mpView->GetDefaultCamPosZ() + fDeepth / 2));
    aCamera.SetFocalLength(mpView->GetDefaultCamFocal());
    pScene->SetCamera(aCamera);
    basegfx::B3DHomMatrix aTransformation;

    switch (nSlotId)
    {
        case SID_3D_CUBE:
        {
            aTransformation.rotate(DEG2RAD(20), 0.0, 0.0);
        }
        break;

        case SID_3D_SPHERE:
        {
        }
        break;

        case SID_3D_SHELL:
        case SID_3D_HALF_SPHERE:
        {
            aTransformation.rotate(DEG2RAD(200), 0.0, 0.0);
        }
        break;

        case SID_3D_CYLINDER:
        case SID_3D_CONE:
        case SID_3D_PYRAMID:
        {
        }
        break;

        case SID_3D_TORUS:
        {
            aTransformation.rotate(DEG2RAD(90), 0.0, 0.0);
        }
        break;

        default:
        {
        }
        break;
    }

    pScene->SetTransform(aTransformation * pScene->GetTransform());

    SfxItemSet aAttr (mpViewShell->GetPool());
    pScene->SetMergedItemSetAndBroadcast(aAttr);
}

bool FuConstruct3dObject::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

        mpWindow->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        WaitObject aWait( static_cast<vcl::Window*>(mpViewShell->GetActiveWindow()) );

        E3dCompoundObject* p3DObj = ImpCreateBasic3DShape();
        E3dScene* pScene = mpView->SetCurrent3DObj(p3DObj);

        ImpPrepareBasic3DShape(p3DObj, pScene);
        bReturn = mpView->BegCreatePreparedObject(aPnt, nDrgLog, pScene);

        SdrObject* pObj = mpView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(mpDoc->GetPool());
            SetStyleSheet(aAttr, pObj);

            // extract LineStyle
            aAttr.Put(XLineStyleItem (drawing::LineStyle_NONE));

            pObj->SetMergedItemSet(aAttr);
        }
    }

    return bReturn;
}

bool FuConstruct3dObject::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = false;

    if ( mpView->IsCreateObj() && rMEvt.IsLeft() )
    {
        mpView->EndCreateObj(SdrCreateCmd::ForceEnd);
        bReturn = true;
    }

    bReturn = FuConstruct::MouseButtonUp(rMEvt) || bReturn;

    if (!bPermanent)
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);

    return bReturn;
}

void FuConstruct3dObject::Activate()
{
    mpView->SetCurrentObj(OBJ_NONE);

    FuConstruct::Activate();
}

SdrObjectUniquePtr FuConstruct3dObject::CreateDefaultObject(const sal_uInt16 nID, const ::tools::Rectangle& rRectangle)
{

    E3dCompoundObject* p3DObj = ImpCreateBasic3DShape();

    // E3dView::SetCurrent3DObj part
    // get transformed BoundVolume of the object
    basegfx::B3DRange aObjVol(p3DObj->GetBoundVolume());
    aObjVol.transform(p3DObj->GetTransform());
    basegfx::B3DRange aVolume(aObjVol);
    double fW(aVolume.getWidth());
    double fH(aVolume.getHeight());
    ::tools::Rectangle a3DRect(0, 0, static_cast<long>(fW), static_cast<long>(fH));
    std::unique_ptr< E3dScene, SdrObjectFreeOp > pScene(new E3dScene(*mpDoc));

    // copied code from E3dView::InitScene
    double fCamZ(aVolume.getMaxZ() + ((fW + fH) / 4.0));
    Camera3D aCam(pScene->GetCamera());
    aCam.SetAutoAdjustProjection(false);
    aCam.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
    ::basegfx::B3DPoint aLookAt;
    double fDefaultCamPosZ = mpView->GetDefaultCamPosZ();
    ::basegfx::B3DPoint aCamPos(0.0, 0.0, fCamZ < fDefaultCamPosZ ? fDefaultCamPosZ : fCamZ);
    aCam.SetPosAndLookAt(aCamPos, aLookAt);
    aCam.SetFocalLength(mpView->GetDefaultCamFocal());
    pScene->SetCamera(aCam);
    pScene->InsertObject(p3DObj);
    pScene->NbcSetSnapRect(a3DRect);
    ImpPrepareBasic3DShape(p3DObj, pScene.get());
    SfxItemSet aAttr(mpDoc->GetPool());
    SetStyleSheet(aAttr, p3DObj);
    aAttr.Put(XLineStyleItem (drawing::LineStyle_NONE));
    p3DObj->SetMergedItemSet(aAttr);

    // make object interactive at once
    pScene->SetRectsDirty();

    // Take care of restrictions for the rectangle
    ::tools::Rectangle aRect(rRectangle);

    switch(nID)
    {
        case SID_3D_CUBE:
        case SID_3D_SPHERE:
        case SID_3D_TORUS:
        {
            // force quadratic
            ImpForceQuadratic(aRect);
            break;
        }

        case SID_3D_SHELL:
        case SID_3D_HALF_SPHERE:
        {
            // force horizontal layout
            break;
        }

        case SID_3D_CYLINDER:
        case SID_3D_CONE:
        case SID_3D_PYRAMID:
        {
            // force vertical layout
            break;
        }
    }

    // use changed rectangle, not original one
    pScene->SetLogicRect(aRect);

    return SdrObjectUniquePtr(pScene.release());
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
