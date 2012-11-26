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
#include "precompiled_sd.hxx"

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
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/svdlegacy.hxx>

#include "app.hrc"
#include "res_bmp.hrc"
#include "View.hxx"
#include "Window.hxx"
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"
#include <svx/svx3ditems.hxx>

namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstruct3dObject::FuConstruct3dObject (
    ViewShell*  pViewSh,
    ::sd::Window*       pWin,
    ::sd::View*         pView,
    SdDrawDocument* pDoc,
    SfxRequest&     rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuConstruct3dObject::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuConstruct3dObject* pFunc;
    FunctionReference xFunc( pFunc = new FuConstruct3dObject( pViewSh, pWin, pView, pDoc, rReq ) );
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

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

// #97016#
E3dCompoundObject* FuConstruct3dObject::ImpCreateBasic3DShape()
{
    E3dCompoundObject* p3DObj = NULL;

    switch (nSlotId)
    {
        default:
        case SID_3D_CUBE:
        {
            p3DObj = new E3dCubeObj(
                *GetDoc(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B3DPoint(-2500, -2500, -2500),
                ::basegfx::B3DVector(5000, 5000, 5000));
            break;
        }

        case SID_3D_SPHERE:
        {
            p3DObj = new E3dSphereObj(
                *GetDoc(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B3DPoint(0, 0, 0),
                ::basegfx::B3DVector(5000, 5000, 5000));
            break;
        }

        case SID_3D_SHELL:
        {
            basegfx::B2DPolygon aB2DPolygon(
                basegfx::tools::createPolygonFromEllipseSegment(
                    basegfx::B2DPoint(0.0, 1250.0 * 5.0),
                    2500.0 * 5.0,
                    2500.0 * 5.0,
                    270.0 * F_PI / 180.0,
                    0.0));

            if(aB2DPolygon.areControlPointsUsed())
            {
                aB2DPolygon = basegfx::tools::adaptiveSubdivideByAngle(aB2DPolygon);
            }

            p3DObj = new E3dLatheObj(
                *GetDoc(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B2DPolyPolygon(aB2DPolygon));

            // Dies ist ein offenes Objekt, muss daher defaultmaessig
            // doppelseitig behandelt werden
            p3DObj->SetMergedItem(SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, true));
            break;
        }

        case SID_3D_HALF_SPHERE:
        {
            basegfx::B2DPolygon aB2DPolygon;

            // add in-between points to the horizontal line to not run into problems
            // when the vertical segment count gets changed eventually later
            aB2DPolygon.append(basegfx::B2DPoint(0.0, 1250.0 * 5.0));
            aB2DPolygon.append(basegfx::B2DPoint(50.0 * 5.0, 1250.0 * 5.0));
            aB2DPolygon.append(basegfx::B2DPoint(250.0 * 5.0, 1250.0 * 5.0));
            aB2DPolygon.append(basegfx::B2DPoint(500.0 * 5.0, 1250.0 * 5.0));
            aB2DPolygon.append(basegfx::B2DPoint(1000.0 * 5.0, 1250.0 * 5.0));
            aB2DPolygon.append(basegfx::B2DPoint(1500.0 * 5.0, 1250.0 * 5.0));
            aB2DPolygon.append(basegfx::B2DPoint(2000.0 * 5.0, 1250.0 * 5.0));
            aB2DPolygon.append(basegfx::B2DPoint(2400.0 * 5.0, 1250.0 * 5.0));

            basegfx::B2DPolygon aHalfSphere(
                basegfx::tools::createPolygonFromEllipseSegment(
                    basegfx::B2DPoint(0.0, 1250.0 * 5.0),
                    2500.0 * 5.0,
                    2500.0 * 5.0,
                    270.0 * F_PI / 180.0,
                    0.0));

            aHalfSphere.flip();
            aB2DPolygon.append(aHalfSphere);

            if(aB2DPolygon.areControlPointsUsed())
            {
                aB2DPolygon = ::basegfx::tools::adaptiveSubdivideByAngle(aB2DPolygon);
            }

            p3DObj = new E3dLatheObj(
                *GetDoc(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B2DPolyPolygon(aB2DPolygon));
            break;
        }

        case SID_3D_TORUS:
        {
            ::basegfx::B2DPolygon aB2DPolygon(::basegfx::tools::createPolygonFromCircle(::basegfx::B2DPoint(1000.0, 0.0), 500.0));

            if(aB2DPolygon.areControlPointsUsed())
            {
                aB2DPolygon = ::basegfx::tools::adaptiveSubdivideByAngle(aB2DPolygon);
            }

            p3DObj = new E3dLatheObj(
                *GetDoc(),
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
            aInnerPoly.append(::basegfx::B2DPoint(0*5, -1000*5));
            aInnerPoly.setClosed(true);

            p3DObj = new E3dLatheObj(
                *GetDoc(),
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
            aInnerPoly.append(::basegfx::B2DPoint(0*5, 1000*5));
            aInnerPoly.setClosed(true);

            p3DObj = new E3dLatheObj(
                *GetDoc(),
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
                *GetDoc(),
                mpView->Get3DDefaultAttributes(),
                ::basegfx::B2DPolyPolygon(aInnerPoly));
            p3DObj->SetMergedItem(SfxUInt32Item(SDRATTR_3DOBJ_HORZ_SEGS, 4));
            break;
        }
    }

    return p3DObj;
}

// #97016#
void FuConstruct3dObject::ImpPrepareBasic3DShape(E3dCompoundObject* p3DObj, E3dScene *pScene)
{
    Camera3D &aCamera  = (Camera3D&) pScene->GetCamera ();

    // get transformed BoundVolume of the new object
    basegfx::B3DRange aBoundVol;
    basegfx::B3DRange aObjVol(p3DObj->GetBoundVolume());
    aObjVol.transform(p3DObj->GetB3DTransform());
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
//              pScene->RotateX(DEG2RAD(60));
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
//              pScene->RotateX(DEG2RAD(25));
        }
        break;

        case SID_3D_TORUS:
        {
//              pScene->RotateX(DEG2RAD(15));
            aTransformation.rotate(DEG2RAD(90), 0.0, 0.0);
        }
        break;

        default:
        {
        }
        break;
    }

    pScene->SetB3DTransform(aTransformation * pScene->GetB3DTransform());

    SfxItemSet aAttr (mpViewShell->GetPool());
    pScene->SetMergedItemSetAndBroadcast(aAttr);
}

bool FuConstruct3dObject::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPixelPos);

        mpWindow->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        E3dCompoundObject* p3DObj = NULL;

        WaitObject aWait( (Window*)mpViewShell->GetActiveWindow() );

        // #97016#
        p3DObj = ImpCreateBasic3DShape();
        E3dScene* pScene = mpView->SetCurrent3DObj(p3DObj);

        // #97016#
        ImpPrepareBasic3DShape(p3DObj, pScene);
        bReturn = mpView->BegCreatePreparedObject(aLogicPos, nDrgLog, pScene);

        SdrObject* pObj = mpView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(mpDoc->GetItemPool());
            SetStyleSheet(aAttr, pObj);

            // LineStyle rausnehmen
            aAttr.Put(XLineStyleItem (XLINE_NONE));

            pObj->SetMergedItemSet(aAttr);
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuConstruct3dObject::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuConstruct3dObject::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = false;

    if ( mpView->GetCreateObj() && rMEvt.IsLeft() )
    {
        mpView->EndCreateObj(SDRCREATE_FORCEEND);
        bReturn = true;
    }

    bReturn = FuConstruct::MouseButtonUp(rMEvt) || bReturn;

    if (!bPermanent)
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert true, andernfalls
|* false.
|*
\************************************************************************/

bool FuConstruct3dObject::KeyInput(const KeyEvent& rKEvt)
{
    return( FuConstruct::KeyInput(rKEvt) );
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstruct3dObject::Activate()
{
    mpView->setSdrObjectCreationInfo(SdrObjectCreationInfo(static_cast< sal_uInt16 >(OBJ_NONE)));

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstruct3dObject::Deactivate()
{
    FuConstruct::Deactivate();
}

// #97016#
SdrObject* FuConstruct3dObject::CreateDefaultObject(const sal_uInt16 nID, const basegfx::B2DRange& rRange)
{
    // case SID_3D_CUBE:
    // case SID_3D_SHELL:
    // case SID_3D_SPHERE:
    // case SID_3D_TORUS:
    // case SID_3D_HALF_SPHERE:
    // case SID_3D_CYLINDER:
    // case SID_3D_CONE:
    // case SID_3D_PYRAMID:

    E3dCompoundObject* p3DObj = ImpCreateBasic3DShape();

    // E3dView::SetCurrent3DObj part
    // get transformed BoundVolume of the object
    basegfx::B3DRange aObjVol(p3DObj->GetBoundVolume());
    aObjVol.transform(p3DObj->GetB3DTransform());
    basegfx::B3DRange aVolume(aObjVol);
    double fW(aVolume.getWidth());
    double fH(aVolume.getHeight());
    Rectangle a3DRect(0, 0, (long)fW, (long)fH);
    E3dScene* pScene = new E3dScene(
        *GetDoc(),
        mpView->Get3DDefaultAttributes());

    // mpView->InitScene(pScene, fW, fH, aVolume.MaxVec().Z() + ((fW + fH) / 4.0));
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

    pScene->Insert3DObj(*p3DObj);
    sdr::legacy::SetSnapRect(*pScene, a3DRect);
    // pScene->SetModel(mpDoc);

    ImpPrepareBasic3DShape(p3DObj, pScene);

    SfxItemSet aAttr(mpDoc->GetItemPool());
    SetStyleSheet(aAttr, p3DObj);
    aAttr.Put(XLineStyleItem (XLINE_NONE));
    p3DObj->SetMergedItemSet(aAttr);

    // make object interactive at once
    pScene->ActionChanged();

    // Take care of restrictions for the rectangle
    basegfx::B2DRange aRange(rRange);

    switch(nID)
    {
        case SID_3D_CUBE:
        case SID_3D_SPHERE:
        case SID_3D_TORUS:
        {
            // force quadratic
            ImpForceQuadratic(aRange);
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

    // #97016#, #98245# use changed rectangle, not original one
    sdr::legacy::SetLogicRange(*pScene, aRange);

    return pScene;
}

} // end of namespace sd
