/*************************************************************************
 *
 *  $RCSfile: fucon3d.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2002-02-15 16:51:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif

#include <math.h>
#include <svx/globl3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/camera3d.hxx>
#include <svx/xpoly.hxx>
#include <svx/xoutx.hxx>

#include "app.hrc"
#include "res_bmp.hrc"

#include "sdview.hxx"
#include "sdwindow.hxx"
#include "viewshel.hxx"
#include "fucon3d.hxx"
#include "drawdoc.hxx"

#ifndef _SVX3DITEMS_HXX
#include <svx/svx3ditems.hxx>
#endif

// #97016#
#ifndef _E3D_POLYSC3D_HXX
#include <svx/polysc3d.hxx>
#endif

class SfxRequest;
class SdDrawDocument;

TYPEINIT1( FuConst3dObj, FuConstruct );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConst3dObj::FuConst3dObj(SdViewShell*     pViewSh,
                           SdWindow*        pWin,
                           SdView*          pView,
                           SdDrawDocument*  pDoc,
                           SfxRequest&      rReq) :
      FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
{
    pViewShell->SwitchObjectBar(RID_DRAW_OBJ_TOOLBOX);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConst3dObj::~FuConst3dObj()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

// #97016#
E3dCompoundObject* FuConst3dObj::ImpCreateBasic3DShape()
{
    E3dCompoundObject* p3DObj = NULL;

    switch (nSlotId)
    {
        default:
        case SID_3D_CUBE:
        {
            p3DObj = new E3dCubeObj(
                pView->Get3DDefaultAttributes(),
                Vector3D(-2500, -2500, -2500),
                Vector3D(5000, 5000, 5000));
            break;
        }

        case SID_3D_SPHERE:
        {
            p3DObj = new E3dSphereObj(
                pView->Get3DDefaultAttributes(),
                Vector3D(0, 0, 0),
                Vector3D(5000, 5000, 5000));
            break;
        }

        case SID_3D_SHELL:
        {
            XPolygon aXPoly(Point (0, 1250), 2500, 2500, 0, 900, FALSE);
            aXPoly.Scale(5.0, 5.0);

            p3DObj = new E3dLatheObj(
                pView->Get3DDefaultAttributes(),
                (XPolygon)XOutCreatePolygon (aXPoly, pWindow));

            // Dies ist ein offenes Objekt, muss daher defaultmaessig
            // doppelseitig behandelt werden
            p3DObj->SetItem(Svx3DDoubleSidedItem(TRUE));
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
            p3DObj = new E3dLatheObj(
                pView->Get3DDefaultAttributes(),
                (XPolygon)XOutCreatePolygon (aXPoly, pWindow));
            break;
        }

        case SID_3D_TORUS:
        {
            p3DObj = new E3dLatheObj(
                pView->Get3DDefaultAttributes(),
                (XPolygon)XOutCreatePolygon(XPolygon (Point (1000, 0), 500, 500, 0, 3600), pWindow));
            break;
        }

        case SID_3D_CYLINDER:
        {
            XPolygon aXPoly(16);
            aXPoly[0] = Point(0, 1000*5);
            aXPoly[1] = Point(50*5, 1000*5);
            aXPoly[2] = Point(100*5, 1000*5);
            aXPoly[3] = Point(200*5, 1000*5);
            aXPoly[4] = Point(300*5, 1000*5);
            aXPoly[5] = Point(400*5, 1000*5);
            aXPoly[6] = Point(450*5, 1000*5);
            aXPoly[7] = Point(500*5, 1000*5);
            aXPoly[8] = Point(500*5, -1000*5);
            aXPoly[9] = Point(450*5, -1000*5);
            aXPoly[10] = Point(400*5, -1000*5);
            aXPoly[11] = Point(300*5, -1000*5);
            aXPoly[12] = Point(200*5, -1000*5);
            aXPoly[13] = Point(100*5, -1000*5);
            aXPoly[14] = Point(50*5, -1000*5);
            aXPoly[15] = Point(0*5, -1000*5);

            p3DObj = new E3dLatheObj(
                pView->Get3DDefaultAttributes(),
                aXPoly);
            break;
        }

        case SID_3D_CONE:
        {
            XPolygon aXPoly(14);
            aXPoly[0] = Point(0, -1000*5);
            aXPoly[1] = Point(25*5, -900*5);
            aXPoly[2] = Point(50*5, -800*5);
            aXPoly[3] = Point(100*5, -600*5);
            aXPoly[4] = Point(200*5, -200*5);
            aXPoly[5] = Point(300*5, 200*5);
            aXPoly[6] = Point(400*5, 600*5);
            aXPoly[7] = Point(500*5, 1000*5);
            aXPoly[8] = Point(400*5, 1000*5);
            aXPoly[9] = Point(300*5, 1000*5);
            aXPoly[10] = Point(200*5, 1000*5);
            aXPoly[11] = Point(100*5, 1000*5);
            aXPoly[12] = Point(50*5, 1000*5);
            aXPoly[13] = Point(0*5, 1000*5);

            p3DObj = new E3dLatheObj(
                pView->Get3DDefaultAttributes(),
                aXPoly);
            break;
        }

        case SID_3D_PYRAMID:
        {
            XPolygon aXPoly(14);
            aXPoly[0] = Point(0, -1000*5);
            aXPoly[1] = Point(25*5, -900*5);
            aXPoly[2] = Point(50*5, -800*5);
            aXPoly[3] = Point(100*5, -600*5);
            aXPoly[4] = Point(200*5, -200*5);
            aXPoly[5] = Point(300*5, 200*5);
            aXPoly[6] = Point(400*5, 600*5);
            aXPoly[7] = Point(500*5, 1000*5);
            aXPoly[8] = Point(400*5, 1000*5);
            aXPoly[9] = Point(300*5, 1000*5);
            aXPoly[10] = Point(200*5, 1000*5);
            aXPoly[11] = Point(100*5, 1000*5);
            aXPoly[12] = Point(50*5, 1000*5);
            aXPoly[13] = Point(0, 1000*5);

            p3DObj = new E3dLatheObj(
                pView->Get3DDefaultAttributes(),
                aXPoly);
            p3DObj->SetItem(Svx3DHorizontalSegmentsItem(4));
            break;
        }
    }

    return p3DObj;
}

// #97016#
void FuConst3dObj::ImpPrepareBasic3DShape(E3dCompoundObject* p3DObj, E3dScene *pScene)
{
    Camera3D &aCamera  = (Camera3D&) pScene->GetCamera ();

    // get transformed BoundVolume of the new object
    Volume3D aBoundVol;
    const Volume3D& rObjVol = p3DObj->GetBoundVolume();
    const Matrix4D& rObjTrans  = p3DObj->GetTransform();
    aBoundVol.Union(rObjVol.GetTransformVolume(rObjTrans));

    Vector3D aMinVec (aBoundVol.MinVec ());
    Vector3D aMaxVec (aBoundVol.MaxVec ());
    double fDeepth = fabs (aMaxVec.Z () - aMinVec.Z ());

    aCamera.SetPRP(Vector3D(0, 0, 1000));
    aCamera.SetPosition(Vector3D(0.0, 0.0, pView->GetDefaultCamPosZ() + fDeepth / 2));
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

    if (nSlotId == SID_3D_SPHERE)
    {
        // Keine Sortierung noetig
        pScene->SetSortingMode(E3D_SORT_NO_SORTING);
    }
    else if (nSlotId == SID_3D_CYLINDER)
    {
        // Das muss auch ohne aufwendige Sortierung gehen!
        // Ersteinaml jedoch: Sortierung ueber Lookupfield
        pScene->SetSortingMode(E3D_SORT_LOOKUP_FIELD |
                               E3D_SORT_IN_PARENTS   |
                               E3D_SORT_TEST_LENGTH);
    }
    else
    {
        // Einfache Sortierung
        pScene->SetSortingMode(E3D_SORT_FAST_SORTING |
                               E3D_SORT_IN_PARENTS   |
                               E3D_SORT_TEST_LENGTH);
    }

    pScene->FitSnapRectToBoundVol();

    SfxItemSet aAttr (pViewShell->GetPool());
    pScene->SetItemSetAndBroadcast(aAttr);
}

BOOL FuConst3dObj::MouseButtonDown(const MouseEvent& rMEvt)
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

        pView->SetCurrent3DObj(p3DObj);
        E3dScene *pScene   = (E3dScene*) pView->GetCurrentLibObj();

        // #97016#
        ImpPrepareBasic3DShape(p3DObj, pScene);

        bReturn = pView->BegCreateObj(aPnt, (OutputDevice*) NULL, nDrgLog);

        SdrObject* pObj = pView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(pDoc->GetPool());
            SetStyleSheet(aAttr, pObj);

            // LineStyle rausnehmen
            aAttr.Put(XLineStyleItem (XLINE_NONE));

            pObj->SetItemSet(aAttr);
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuConst3dObj::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuConst3dObj::MouseButtonUp(const MouseEvent& rMEvt)
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

BOOL FuConst3dObj::KeyInput(const KeyEvent& rKEvt)
{
    return( FuConstruct::KeyInput(rKEvt) );
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConst3dObj::Activate()
{
    pView->SetCurrentObj(OBJ_NONE);

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConst3dObj::Deactivate()
{
    FuConstruct::Deactivate();
}

// #97016#
SdrObject* FuConst3dObj::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
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
    const Matrix4D& rObjTrans = p3DObj->GetTransform();
    Volume3D aVolume;
    aVolume.Union(rObjVol.GetTransformVolume(rObjTrans));
    double fW(aVolume.GetWidth());
    double fH(aVolume.GetHeight());
    Rectangle aRect(0, 0, (long)fW, (long)fH);
    E3dScene* pScene = new E3dPolyScene(pView->Get3DDefaultAttributes());

    // pView->InitScene(pScene, fW, fH, aVolume.MaxVec().Z() + ((fW + fH) / 4.0));
    // copied code from E3dView::InitScene
    double fCamZ(aVolume.MaxVec().Z() + ((fW + fH) / 4.0));
    Camera3D aCam(pScene->GetCamera());
    aCam.SetAutoAdjustProjection(FALSE);
    aCam.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
    Vector3D aLookAt;
    double fDefaultCamPosZ = pView->GetDefaultCamPosZ();
    Vector3D aCamPos(0.0, 0.0, fCamZ < fDefaultCamPosZ ? fDefaultCamPosZ : fCamZ);
    aCam.SetPosAndLookAt(aCamPos, aLookAt);
    aCam.SetFocalLength(pView->GetDefaultCamFocal());
    aCam.SetDefaults(Vector3D(0.0, 0.0, fDefaultCamPosZ), aLookAt, pView->GetDefaultCamFocal());
    pScene->SetCamera(aCam);

    pScene->Insert3DObj(p3DObj);
    pScene->NbcSetSnapRect(aRect);
    // SetCurrentLibObj(pScene);
    pScene->SetModel(pDoc);

    ImpPrepareBasic3DShape(p3DObj, pScene);

    SfxItemSet aAttr(pDoc->GetPool());
    SetStyleSheet(aAttr, p3DObj);
    aAttr.Put(XLineStyleItem (XLINE_NONE));
    p3DObj->SetItemSet(aAttr);

    // make object interactive at once
    pScene->SetRectsDirty();
    pScene->InitTransformationSet();
    pScene->SetLogicRect(rRectangle);

    return pScene;
}
