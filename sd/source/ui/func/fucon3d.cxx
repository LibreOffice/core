/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fucon3d.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:27:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "fucon3d.hxx"

#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

#include <svx/svxids.hrc>
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif

#include <math.h>
#include <svx/globl3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/camera3d.hxx>
#include <svx/xoutx.hxx>

#include "app.hrc"
#include "res_bmp.hrc"

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_TOOL_BAR_MANAGER_HXX
#include "ToolBarManager.hxx"
#endif

#ifndef _SVX3DITEMS_HXX
#include <svx/svx3ditems.hxx>
#endif

// #97016#
#ifndef _E3D_POLYSC3D_HXX
#include <svx/polysc3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

namespace sd {

TYPEINIT1( FuConstruct3dObject, FuConstruct );

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
    pViewShell->GetViewShellBase().GetToolBarManager().SetToolBar(
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
                pView->Get3DDefaultAttributes(),
                ::basegfx::B3DPoint(-2500, -2500, -2500),
                ::basegfx::B3DVector(5000, 5000, 5000));
            break;
        }

        case SID_3D_SPHERE:
        {
            p3DObj = new E3dSphereObj(
                pView->Get3DDefaultAttributes(),
                ::basegfx::B3DPoint(0, 0, 0),
                ::basegfx::B3DVector(5000, 5000, 5000));
            break;
        }

        case SID_3D_SHELL:
        {
            XPolygon aXPoly(Point (0, 1250), 2500, 2500, 0, 900, FALSE);
            aXPoly.Scale(5.0, 5.0);

            ::basegfx::B2DPolygon aB2DPolygon(aXPoly.getB2DPolygon());
            if(aB2DPolygon.areControlVectorsUsed())
            {
                aB2DPolygon = ::basegfx::tools::adaptiveSubdivideByAngle(aB2DPolygon);
            }
            p3DObj = new E3dLatheObj(pView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aB2DPolygon));

            // Dies ist ein offenes Objekt, muss daher defaultmaessig
            // doppelseitig behandelt werden
            p3DObj->SetMergedItem(Svx3DDoubleSidedItem(TRUE));
            break;
        }

        case SID_3D_HALF_SPHERE:
        {
            XPolygon aXPoly(Point (0, 1250), 2500, 2500, 0, 900, FALSE);
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
            if(aB2DPolygon.areControlVectorsUsed())
            {
                aB2DPolygon = ::basegfx::tools::adaptiveSubdivideByAngle(aB2DPolygon);
            }
            p3DObj = new E3dLatheObj(pView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aB2DPolygon));
            break;
        }

        case SID_3D_TORUS:
        {
            ::basegfx::B2DPolygon aB2DPolygon(::basegfx::tools::createPolygonFromCircle(::basegfx::B2DPoint(1000.0, 0.0), 500.0));
            if(aB2DPolygon.areControlVectorsUsed())
            {
                aB2DPolygon = ::basegfx::tools::adaptiveSubdivideByAngle(aB2DPolygon);
            }
            p3DObj = new E3dLatheObj(pView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aB2DPolygon));
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

            p3DObj = new E3dLatheObj(pView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aInnerPoly));
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

            p3DObj = new E3dLatheObj(pView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aInnerPoly));
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

            p3DObj = new E3dLatheObj(pView->Get3DDefaultAttributes(), ::basegfx::B2DPolyPolygon(aInnerPoly));
            p3DObj->SetMergedItem(Svx3DHorizontalSegmentsItem(4));
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
    Volume3D aBoundVol;
    const Volume3D& rObjVol = p3DObj->GetBoundVolume();
    const ::basegfx::B3DHomMatrix& rObjTrans  = p3DObj->GetTransform();
    aBoundVol.expand(rObjVol.GetTransformVolume(rObjTrans));
    double fDeepth(aBoundVol.getDepth());

    aCamera.SetPRP(::basegfx::B3DPoint(0.0, 0.0, 1000.0));
    aCamera.SetPosition(::basegfx::B3DPoint(0.0, 0.0, pView->GetDefaultCamPosZ() + fDeepth / 2));
    aCamera.SetFocalLength(pView->GetDefaultCamFocal());
    pScene->SetCamera(aCamera);

    switch (nSlotId)
    {
        case SID_3D_CUBE:
        {
            pScene->RotateX(DEG2RAD(20));
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
            pScene->RotateX(DEG2RAD(200));
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
            pScene->RotateX(DEG2RAD(90));
        }
        break;

        default:
        {
        }
        break;
    }

    pScene->FitSnapRectToBoundVol();

    SfxItemSet aAttr (pViewShell->GetPool());
    pScene->SetMergedItemSetAndBroadcast(aAttr);
}

BOOL FuConstruct3dObject::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

        pWindow->CaptureMouse();
        USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

        E3dCompoundObject* p3DObj = NULL;

        WaitObject aWait( (Window*)pViewShell->GetActiveWindow() );

        // #97016#
        p3DObj = ImpCreateBasic3DShape();
        E3dScene* pScene = pView->SetCurrent3DObj(p3DObj);

        // #97016#
        ImpPrepareBasic3DShape(p3DObj, pScene);
        bReturn = pView->BegCreatePreparedObject(aPnt, nDrgLog, pScene);

        SdrObject* pObj = pView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(pDoc->GetPool());
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

BOOL FuConstruct3dObject::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuConstruct3dObject::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pView->EndCreateObj(SDRCREATE_FORCEEND);
        bReturn = TRUE;
    }

    bReturn = FuConstruct::MouseButtonUp(rMEvt) || bReturn;

    if (!bPermanent)
        pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuConstruct3dObject::KeyInput(const KeyEvent& rKEvt)
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
    pView->SetCurrentObj(OBJ_NONE);

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
SdrObject* FuConstruct3dObject::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
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
    const Volume3D& rObjVol = p3DObj->GetBoundVolume();
    const ::basegfx::B3DHomMatrix& rObjTrans = p3DObj->GetTransform();
    Volume3D aVolume(rObjVol.GetTransformVolume(rObjTrans));
    double fW(aVolume.getWidth());
    double fH(aVolume.getHeight());
    Rectangle a3DRect(0, 0, (long)fW, (long)fH);
    E3dScene* pScene = new E3dPolyScene(pView->Get3DDefaultAttributes());

    // pView->InitScene(pScene, fW, fH, aVolume.MaxVec().Z() + ((fW + fH) / 4.0));
    // copied code from E3dView::InitScene
    double fCamZ(aVolume.getMaxZ() + ((fW + fH) / 4.0));
    Camera3D aCam(pScene->GetCamera());
    aCam.SetAutoAdjustProjection(FALSE);
    aCam.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
    ::basegfx::B3DPoint aLookAt;
    double fDefaultCamPosZ = pView->GetDefaultCamPosZ();
    ::basegfx::B3DPoint aCamPos(0.0, 0.0, fCamZ < fDefaultCamPosZ ? fDefaultCamPosZ : fCamZ);
    aCam.SetPosAndLookAt(aCamPos, aLookAt);
    aCam.SetFocalLength(pView->GetDefaultCamFocal());
    aCam.SetDefaults(::basegfx::B3DPoint(0.0, 0.0, fDefaultCamPosZ), aLookAt, pView->GetDefaultCamFocal());
    pScene->SetCamera(aCam);

    pScene->Insert3DObj(p3DObj);
    pScene->NbcSetSnapRect(a3DRect);
    pScene->SetModel(pDoc);

    ImpPrepareBasic3DShape(p3DObj, pScene);

    SfxItemSet aAttr(pDoc->GetPool());
    SetStyleSheet(aAttr, p3DObj);
    aAttr.Put(XLineStyleItem (XLINE_NONE));
    p3DObj->SetMergedItemSet(aAttr);

    // make object interactive at once
    pScene->SetRectsDirty();
    pScene->InitTransformationSet();

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

    // #97016#, #98245# use changed rectangle, not original one
    pScene->SetLogicRect(aRect);

    return pScene;
}

} // end of namespace sd
