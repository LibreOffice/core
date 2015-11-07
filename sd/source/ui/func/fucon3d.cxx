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

#include "fucon3d.hxx"
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

#include "app.hrc"
#include "res_bmp.hrc"
#include "View.hxx"
#include "Window.hxx"
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"
#include <svx/svx3ditems.hxx>

#include <svx/polysc3d.hxx>
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
        ToolBarManager::TBG_FUNCTION,
        ToolBarManager::msDrawingObjectToolBar);
}

E3dCompoundObject* FuConstruct3dObject::ImpCreateBasic3DShape()
{
    E3dCompoundObject* p3DObj = NULL;

    switch (nSlotId)
    {
        default:
        case SID_3D_CUBE:
        {
            p3DObj = new E3dCubeObj(
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B3DPoint(-2500, -2500, -2500),
                ::basegfx::B3DVector(5000, 5000, 5000));
            break;
        }

        case SID_3D_SPHERE:
        {
            p3DObj = new E3dSphereObj(
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
                aB2DPolygon = ::basegfx::tools::adaptiveSubdivideByAngle(aB2DPolygon);
            }
            p3DObj = new E3dLatheObj(mpView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aB2DPolygon));

            /* this is an open object, therefore it has to be handled double-
               sided by default */
            p3DObj->SetMergedItem(makeSvx3DDoubleSidedItem(true));
            break;
        }

        case SID_3D_HALF_SPHERE:
        {
            XPolygon aXPoly(Point (0, 1250), 2500, 2500, 0, 900, false);
            aXPoly.Scale(5.0, 5.0);

            aXPoly.Insert(0, Point (2400*5, 1250*5), XPOLY_NORMAL);
            aXPoly.Insert(0, Point (2000*5, 1250*5), XPOLY_NORMAL);
            aXPoly.Insert(0, Point (1500*5, 1250*5), XPOLY_NORMAL);
            aXPoly.Insert(0, Point (1000*5, 1250*5), XPOLY_NORMAL);
            aXPoly.Insert(0, Point (500*5, 1250*5), XPOLY_NORMAL);
            aXPoly.Insert(0, Point (250*5, 1250*5), XPOLY_NORMAL);
            aXPoly.Insert(0, Point (50*5, 1250*5), XPOLY_NORMAL);
            aXPoly.Insert(0, Point (0*5, 1250*5), XPOLY_NORMAL);

            ::basegfx::B2DPolygon aB2DPolygon(aXPoly.getB2DPolygon());
            if(aB2DPolygon.areControlPointsUsed())
            {
                aB2DPolygon = ::basegfx::tools::adaptiveSubdivideByAngle(aB2DPolygon);
            }
            p3DObj = new E3dLatheObj(mpView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aB2DPolygon));
            break;
        }

        case SID_3D_TORUS:
        {
            ::basegfx::B2DPolygon aB2DPolygon(::basegfx::tools::createPolygonFromCircle(::basegfx::B2DPoint(1000.0, 0.0), 500.0));
            if(aB2DPolygon.areControlPointsUsed())
            {
                aB2DPolygon = ::basegfx::tools::adaptiveSubdivideByAngle(aB2DPolygon);
            }
            p3DObj = new E3dLatheObj(mpView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aB2DPolygon));
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
            aInnerPoly.append(::basegfx::B2DPoint(0*5, -1000*5));
            aInnerPoly.setClosed(true);

            p3DObj = new E3dLatheObj(mpView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aInnerPoly));
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
            aInnerPoly.append(::basegfx::B2DPoint(0*5, 1000*5));
            aInnerPoly.setClosed(true);

            p3DObj = new E3dLatheObj(mpView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aInnerPoly));
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

            p3DObj = new E3dLatheObj(mpView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aInnerPoly));
            p3DObj->SetMergedItem(makeSvx3DHorizontalSegmentsItem(4));
            break;
        }
    }

    return p3DObj;
}

void FuConstruct3dObject::ImpPrepareBasic3DShape(E3dCompoundObject* p3DObj, E3dScene *pScene)
{
    Camera3D &aCamera  = (Camera3D&) pScene->GetCamera ();

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

        E3dCompoundObject* p3DObj = NULL;

        WaitObject aWait( static_cast<vcl::Window*>(mpViewShell->GetActiveWindow()) );

        p3DObj = ImpCreateBasic3DShape();
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

bool FuConstruct3dObject::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

bool FuConstruct3dObject::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = false;

    if ( mpView->IsCreateObj() && rMEvt.IsLeft() )
    {
        mpView->EndCreateObj(SDRCREATE_FORCEEND);
        bReturn = true;
    }

    bReturn = FuConstruct::MouseButtonUp(rMEvt) || bReturn;

    if (!bPermanent)
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);

    return bReturn;
}

/**
 * Process keyboard input
 * @returns sal_True if a KeyEvent is being processed, sal_False otherwise
 */
bool FuConstruct3dObject::KeyInput(const KeyEvent& rKEvt)
{
    return FuConstruct::KeyInput(rKEvt);
}

void FuConstruct3dObject::Activate()
{
    mpView->SetCurrentObj(OBJ_NONE);

    FuConstruct::Activate();
}

void FuConstruct3dObject::Deactivate()
{
    FuConstruct::Deactivate();
}

SdrObject* FuConstruct3dObject::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{

    E3dCompoundObject* p3DObj = ImpCreateBasic3DShape();

    // E3dView::SetCurrent3DObj part
    // get transformed BoundVolume of the object
    basegfx::B3DRange aObjVol(p3DObj->GetBoundVolume());
    aObjVol.transform(p3DObj->GetTransform());
    basegfx::B3DRange aVolume(aObjVol);
    double fW(aVolume.getWidth());
    double fH(aVolume.getHeight());
    Rectangle a3DRect(0, 0, (long)fW, (long)fH);
    E3dScene* pScene = new E3dPolyScene(mpView->Get3DDefaultAttributes());

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
    aCam.SetDefaults(::basegfx::B3DPoint(0.0, 0.0, fDefaultCamPosZ), aLookAt, mpView->GetDefaultCamFocal());
    pScene->SetCamera(aCam);

    pScene->Insert3DObj(p3DObj);
    pScene->NbcSetSnapRect(a3DRect);
    pScene->SetModel(mpDoc);

    ImpPrepareBasic3DShape(p3DObj, pScene);

    SfxItemSet aAttr(mpDoc->GetPool());
    SetStyleSheet(aAttr, p3DObj);
    aAttr.Put(XLineStyleItem (drawing::LineStyle_NONE));
    p3DObj->SetMergedItemSet(aAttr);

    // make object interactive at once
    pScene->SetRectsDirty();

    // Take care of restrictions for the rectangle
    Rectangle aRect(rRectangle);

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

    return pScene;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
