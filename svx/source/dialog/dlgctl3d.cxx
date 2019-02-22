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


#include <svx/dlgctl3d.hxx>
#include <svx/strings.hrc>
#include <svx/view3d.hxx>
#include <svx/fmmodel.hxx>
#include <svl/itempool.hxx>
#include <svx/fmpage.hxx>
#include <svx/sphere3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/scene3d.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>
#include <svx/helperhittest3d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/polygn3d.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <helpids.h>
#include <algorithm>
#include <svx/dialmgr.hxx>
#include <tools/helpers.hxx>
#include <vcl/settings.hxx>

using namespace com::sun::star;

Svx3DPreviewControl::Svx3DPreviewControl(vcl::Window* pParent, WinBits nStyle)
:   Control(pParent, nStyle),
    mpFmPage(nullptr),
    mpScene(nullptr),
    mp3DObj(nullptr),
    mnObjectType(SvxPreviewObjectType::SPHERE)
{
    Construct();

    // do not paint background self, DrawingLayer paints this buffered and as page
    SetControlBackground();
    SetBackground();
}

Size Svx3DPreviewControl::GetOptimalSize() const
{
    return LogicToPixel(Size(80, 100), MapMode(MapUnit::MapAppFont));
}

VCL_BUILDER_FACTORY(Svx3DPreviewControl)

Svx3DPreviewControl::~Svx3DPreviewControl()
{
    disposeOnce();
}

void Svx3DPreviewControl::dispose()
{
    mp3DView.reset();
    mpModel.reset();
    Control::dispose();
}

void Svx3DPreviewControl::Construct()
{
    // Do never mirror the preview window.  This explicitly includes right
    // to left writing environments.
    EnableRTL (false);
    SetMapMode(MapMode(MapUnit::Map100thMM));

    // Model
    mpModel.reset(new FmFormModel());
    mpModel->GetItemPool().FreezeIdRanges();

    // Page
    mpFmPage = new FmFormPage( *mpModel );
    mpModel->InsertPage( mpFmPage, 0 );

    // 3D View
    mp3DView.reset(new E3dView(*mpModel, this ));
    mp3DView->SetBufferedOutputAllowed(true);
    mp3DView->SetBufferedOverlayAllowed(true);

    // 3D Scene
    mpScene = new E3dScene(*mpModel);

    // initially create object
    SetObjectType(SvxPreviewObjectType::SPHERE);

    // camera and perspective
    Camera3D rCamera  = mpScene->GetCamera();
    const basegfx::B3DRange& rVolume = mpScene->GetBoundVolume();
    double fW = rVolume.getWidth();
    double fH = rVolume.getHeight();
    double fCamZ = rVolume.getMaxZ() + ((fW + fH) / 2.0);

    rCamera.SetAutoAdjustProjection(false);
    rCamera.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
    basegfx::B3DPoint aLookAt;
    double fDefaultCamPosZ = mp3DView->GetDefaultCamPosZ();
    basegfx::B3DPoint aCamPos(0.0, 0.0, fCamZ < fDefaultCamPosZ ? fDefaultCamPosZ : fCamZ);
    rCamera.SetPosAndLookAt(aCamPos, aLookAt);
    double fDefaultCamFocal = mp3DView->GetDefaultCamFocal();
    rCamera.SetFocalLength(fDefaultCamFocal);

    mpScene->SetCamera( rCamera );
    mpFmPage->InsertObject( mpScene );

    basegfx::B3DHomMatrix aRotation;
    aRotation.rotate(DEG2RAD( 25 ), 0.0, 0.0);
    aRotation.rotate(0.0, DEG2RAD( 40 ), 0.0);
    mpScene->SetTransform(aRotation * mpScene->GetTransform());

    // invalidate SnapRects of objects
    mpScene->SetRectsDirty();

    SfxItemSet aSet( mpModel->GetItemPool(),
        svl::Items<XATTR_LINESTYLE, XATTR_LINESTYLE,
        XATTR_FILL_FIRST, XATTR_FILLBITMAP>{} );
    aSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
    aSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
    aSet.Put( XFillColorItem( "", COL_WHITE ) );

    mpScene->SetMergedItemSet(aSet);

    // PageView
    SdrPageView* pPageView = mp3DView->ShowSdrPage( mpFmPage );
    mp3DView->hideMarkHandles();

    // mark scene
    mp3DView->MarkObj( mpScene, pPageView );
}

void Svx3DPreviewControl::Resize()
{
    // size of page
    Size aSize( GetSizePixel() );
    aSize = PixelToLogic( aSize );
    mpFmPage->SetSize( aSize );

    // set size
    Size aObjSize( aSize.Width()*5/6, aSize.Height()*5/6 );
    Point aObjPoint( (aSize.Width() - aObjSize.Width()) / 2,
        (aSize.Height() - aObjSize.Height()) / 2);
    tools::Rectangle aRect( aObjPoint, aObjSize);
    mpScene->SetSnapRect( aRect );
}

void Svx3DPreviewControl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    mp3DView->CompleteRedraw(&rRenderContext, vcl::Region(rRect));
}

void Svx3DPreviewControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    Control::MouseButtonDown(rMEvt);

    if( rMEvt.IsShift() && rMEvt.IsMod1() )
    {
        if(SvxPreviewObjectType::SPHERE == GetObjectType())
        {
            SetObjectType(SvxPreviewObjectType::CUBE);
        }
        else
        {
            SetObjectType(SvxPreviewObjectType::SPHERE);
        }
    }
}

void Svx3DPreviewControl::SetObjectType(SvxPreviewObjectType nType)
{
    if( mnObjectType != nType || !mp3DObj)
    {
        SfxItemSet aSet(mpModel->GetItemPool(), svl::Items<SDRATTR_START, SDRATTR_END>{});
        mnObjectType = nType;

        if( mp3DObj )
        {
            aSet.Put(mp3DObj->GetMergedItemSet());
            mpScene->RemoveObject( mp3DObj->GetOrdNum() );
            // always use SdrObject::Free(...) for SdrObjects (!)
            SdrObject* pTemp(mp3DObj);
            SdrObject::Free(pTemp);
        }

        switch( nType )
        {
            case SvxPreviewObjectType::SPHERE:
            {
                mp3DObj = new E3dSphereObj(
                    *mpModel,
                    mp3DView->Get3DDefaultAttributes(),
                    basegfx::B3DPoint( 0, 0, 0 ),
                    basegfx::B3DVector( 5000, 5000, 5000 ));
            }
            break;

            case SvxPreviewObjectType::CUBE:
            {
                mp3DObj = new E3dCubeObj(
                    *mpModel,
                    mp3DView->Get3DDefaultAttributes(),
                    basegfx::B3DPoint( -2500, -2500, -2500 ),
                    basegfx::B3DVector( 5000, 5000, 5000 ));
            }
            break;
        }

        if (mp3DObj)
        {
            mpScene->InsertObject( mp3DObj );
            mp3DObj->SetMergedItemSet(aSet);
        }

        Resize();
    }
}

SfxItemSet const & Svx3DPreviewControl::Get3DAttributes() const
{
    return mp3DObj->GetMergedItemSet();
}

void Svx3DPreviewControl::Set3DAttributes( const SfxItemSet& rAttr )
{
    mp3DObj->SetMergedItemSet(rAttr, true);
    Resize();
}


#define RADIUS_LAMP_PREVIEW_SIZE    (4500.0)
#define RADIUS_LAMP_SMALL           (600.0)
#define RADIUS_LAMP_BIG             (1000.0)
#define NO_LIGHT_SELECTED           (0xffffffff)
#define MAX_NUMBER_LIGHTS              (8)

static const sal_Int32 g_nInteractionStartDistance = 5 * 5 * 2;

Svx3DLightControl::Svx3DLightControl(vcl::Window* pParent, WinBits nStyle)
:   Svx3DPreviewControl(pParent, nStyle),
    maChangeCallback(),
    maSelectionChangeCallback(),
    maSelectedLight(NO_LIGHT_SELECTED),
    mpExpansionObject(nullptr),
    mpLampBottomObject(nullptr),
    mpLampShaftObject(nullptr),
    maLightObjects(MAX_NUMBER_LIGHTS, nullptr),
    mfRotateX(-20.0),
    mfRotateY(45.0),
    mfRotateZ(0.0),
    maActionStartPoint(),
    mfSaveActionStartHor(0.0),
    mfSaveActionStartVer(0.0),
    mfSaveActionStartRotZ(0.0),
    mbMouseMoved(false),
    mbGeometrySelected(false)
{
    Construct2();
}

void Svx3DLightControl::Construct2()
{
    {
        // hide all page stuff, use control background (normally gray)
        const Color aDialogColor(Application::GetSettings().GetStyleSettings().GetDialogColor());
        mp3DView->SetPageVisible(false);
        mp3DView->SetApplicationBackgroundColor(aDialogColor);
        mp3DView->SetApplicationDocumentColor(aDialogColor);
    }

    {
        // create invisible expansion object
        const double fMaxExpansion(RADIUS_LAMP_BIG + RADIUS_LAMP_PREVIEW_SIZE);
        mpExpansionObject = new E3dCubeObj(
            *mpModel,
            mp3DView->Get3DDefaultAttributes(),
            basegfx::B3DPoint(-fMaxExpansion, -fMaxExpansion, -fMaxExpansion),
            basegfx::B3DVector(2.0 * fMaxExpansion, 2.0 * fMaxExpansion, 2.0 * fMaxExpansion));
        mpScene->InsertObject( mpExpansionObject );
        SfxItemSet aSet(mpModel->GetItemPool());
        aSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
        aSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );
        mpExpansionObject->SetMergedItemSet(aSet);
    }

    {
        // create lamp control object (Yellow lined object)
        // base circle
        const basegfx::B2DPolygon a2DCircle(basegfx::utils::createPolygonFromCircle(basegfx::B2DPoint(0.0, 0.0), RADIUS_LAMP_PREVIEW_SIZE));
        basegfx::B3DPolygon a3DCircle(basegfx::utils::createB3DPolygonFromB2DPolygon(a2DCircle));
        basegfx::B3DHomMatrix aTransform;

        aTransform.rotate(F_PI2, 0.0, 0.0);
        aTransform.translate(0.0, -RADIUS_LAMP_PREVIEW_SIZE, 0.0);
        a3DCircle.transform(aTransform);

        // create object for it
        mpLampBottomObject = new E3dPolygonObj(
            *mpModel,
            basegfx::B3DPolyPolygon(a3DCircle));
        mpScene->InsertObject( mpLampBottomObject );

        // half circle with stand
        basegfx::B2DPolygon a2DHalfCircle;
        a2DHalfCircle.append(basegfx::B2DPoint(RADIUS_LAMP_PREVIEW_SIZE, 0.0));
        a2DHalfCircle.append(basegfx::B2DPoint(RADIUS_LAMP_PREVIEW_SIZE, -RADIUS_LAMP_PREVIEW_SIZE));
        a2DHalfCircle.append(basegfx::utils::createPolygonFromEllipseSegment(
            basegfx::B2DPoint(0.0, 0.0), RADIUS_LAMP_PREVIEW_SIZE, RADIUS_LAMP_PREVIEW_SIZE, F_2PI - F_PI2, F_PI2));
        basegfx::B3DPolygon a3DHalfCircle(basegfx::utils::createB3DPolygonFromB2DPolygon(a2DHalfCircle));

        // create object for it
        mpLampShaftObject = new E3dPolygonObj(
            *mpModel,
            basegfx::B3DPolyPolygon(a3DHalfCircle));
        mpScene->InsertObject( mpLampShaftObject );

        // initially invisible
        SfxItemSet aSet(mpModel->GetItemPool());
        aSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
        aSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );

        mpLampBottomObject->SetMergedItemSet(aSet);
        mpLampShaftObject->SetMergedItemSet(aSet);
    }

    {
        // change camera settings
        Camera3D rCamera  = mpScene->GetCamera();
        const basegfx::B3DRange& rVolume = mpScene->GetBoundVolume();
        double fW = rVolume.getWidth();
        double fH = rVolume.getHeight();
        double fCamZ = rVolume.getMaxZ() + ((fW + fH) / 2.0);

        rCamera.SetAutoAdjustProjection(false);
        rCamera.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
        basegfx::B3DPoint aLookAt;
        double fDefaultCamPosZ = mp3DView->GetDefaultCamPosZ();
        basegfx::B3DPoint aCamPos(0.0, 0.0, fCamZ < fDefaultCamPosZ ? fDefaultCamPosZ : fCamZ);
        rCamera.SetPosAndLookAt(aCamPos, aLookAt);
        double fDefaultCamFocal = mp3DView->GetDefaultCamFocal();
        rCamera.SetFocalLength(fDefaultCamFocal);

        mpScene->SetCamera( rCamera );

        basegfx::B3DHomMatrix aNeutral;
        mpScene->SetTransform(aNeutral);
    }

    // invalidate SnapRects of objects
    mpScene->SetRectsDirty();
}

void Svx3DLightControl::ConstructLightObjects()
{
    for(sal_uInt32 a(0); a < MAX_NUMBER_LIGHTS; a++)
    {
        // get rid of possible existing light object
        if(maLightObjects[a])
        {
            mpScene->RemoveObject(maLightObjects[a]->GetOrdNum());
            // always use SdrObject::Free(...) for SdrObjects (!)
            SdrObject* pTemp(maLightObjects[a]);
            SdrObject::Free(pTemp);
            maLightObjects[a] = nullptr;
        }

        if(GetLightOnOff(a))
        {
            const bool bIsSelectedLight(a == maSelectedLight);
            basegfx::B3DVector aDirection(GetLightDirection(a));
            aDirection.normalize();
            aDirection *= RADIUS_LAMP_PREVIEW_SIZE;

            const double fLampSize(bIsSelectedLight ? RADIUS_LAMP_BIG : RADIUS_LAMP_SMALL);
            E3dObject* pNewLight = new E3dSphereObj(
                *mpModel,
                mp3DView->Get3DDefaultAttributes(),
                basegfx::B3DPoint( 0, 0, 0 ),
                basegfx::B3DVector( fLampSize, fLampSize, fLampSize));
            mpScene->InsertObject(pNewLight);

            basegfx::B3DHomMatrix aTransform;
            aTransform.translate(aDirection.getX(), aDirection.getY(), aDirection.getZ());
            pNewLight->SetTransform(aTransform);

            SfxItemSet aSet(mpModel->GetItemPool());
            aSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
            aSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
            aSet.Put( XFillColorItem(OUString(), GetLightColor(a)));
            pNewLight->SetMergedItemSet(aSet);

            maLightObjects[a] = pNewLight;
        }
    }
}

void Svx3DLightControl::AdaptToSelectedLight()
{
    if(NO_LIGHT_SELECTED == maSelectedLight)
    {
        // make mpLampBottomObject/mpLampShaftObject invisible
        SfxItemSet aSet(mpModel->GetItemPool());
        aSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
        aSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );
        mpLampBottomObject->SetMergedItemSet(aSet);
        mpLampShaftObject->SetMergedItemSet(aSet);
    }
    else
    {
        basegfx::B3DVector aDirection(GetLightDirection(maSelectedLight));
        aDirection.normalize();

        // make mpLampBottomObject/mpLampShaftObject visible (yellow hairline)
        SfxItemSet aSet(mpModel->GetItemPool());
        aSet.Put( XLineStyleItem( drawing::LineStyle_SOLID ) );
        aSet.Put( XLineColorItem(OUString(), COL_YELLOW));
        aSet.Put( XLineWidthItem(0));
        aSet.Put( XFillStyleItem( drawing::FillStyle_NONE ) );
        mpLampBottomObject->SetMergedItemSet(aSet);
        mpLampShaftObject->SetMergedItemSet(aSet);

        // adapt transformation of mpLampShaftObject
        basegfx::B3DHomMatrix aTransform;
        double fRotateY(0.0);

        if(!basegfx::fTools::equalZero(aDirection.getZ()) || !basegfx::fTools::equalZero(aDirection.getX()))
        {
            fRotateY = atan2(-aDirection.getZ(), aDirection.getX());
        }

        aTransform.rotate(0.0, fRotateY, 0.0);
        mpLampShaftObject->SetTransform(aTransform);

        // adapt transformation of selected light
        E3dObject* pSelectedLight = maLightObjects[sal_Int32(maSelectedLight)];

        if(pSelectedLight)
        {
            aTransform.identity();
            aTransform.translate(
                aDirection.getX() * RADIUS_LAMP_PREVIEW_SIZE,
                aDirection.getY() * RADIUS_LAMP_PREVIEW_SIZE,
                aDirection.getZ() * RADIUS_LAMP_PREVIEW_SIZE);
            pSelectedLight->SetTransform(aTransform);
        }
    }
}

void Svx3DLightControl::TrySelection(Point aPosPixel)
{
    if(mpScene)
    {
        const Point aPosLogic(PixelToLogic(aPosPixel));
        const basegfx::B2DPoint aPoint(aPosLogic.X(), aPosLogic.Y());
        std::vector< const E3dCompoundObject* > aResult;
        getAllHit3DObjectsSortedFrontToBack(aPoint, *mpScene, aResult);

        if(!aResult.empty())
        {
            // exclude expansion object which will be part of
            // the hits. It's invisible, but for HitTest, it's included
            const E3dCompoundObject* pResult = nullptr;

            for(auto const & b: aResult)
            {
                if(b && b != mpExpansionObject)
                {
                    pResult = b;
                    break;
                }
            }

            if(pResult == mp3DObj)
            {
                if(!mbGeometrySelected)
                {
                    mbGeometrySelected = true;
                    maSelectedLight = NO_LIGHT_SELECTED;
                    ConstructLightObjects();
                    AdaptToSelectedLight();
                    Invalidate();

                    if(maSelectionChangeCallback.IsSet())
                    {
                        maSelectionChangeCallback.Call(this);
                    }
                }
            }
            else
            {
                sal_uInt32 aNewSelectedLight(NO_LIGHT_SELECTED);

                for(sal_uInt32 a(0); a < MAX_NUMBER_LIGHTS; a++)
                {
                    if(maLightObjects[a] && maLightObjects[a] == pResult)
                    {
                        aNewSelectedLight = a;
                    }
                }

                if(aNewSelectedLight != maSelectedLight)
                {
                    SelectLight(aNewSelectedLight);

                    if(maSelectionChangeCallback.IsSet())
                    {
                        maSelectionChangeCallback.Call(this);
                    }
                }
            }
        }
    }
}

void Svx3DLightControl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    Svx3DPreviewControl::Paint(rRenderContext, rRect);
}

void Svx3DLightControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bCallParent(true);

    // switch state
    if(rMEvt.IsLeft())
    {
        if(IsSelectionValid() || mbGeometrySelected)
        {
            mbMouseMoved = false;
            bCallParent = false;
            maActionStartPoint = rMEvt.GetPosPixel();
            StartTracking();
        }
        else
        {
            // Single click without moving much trying to do a selection
            TrySelection(rMEvt.GetPosPixel());
            bCallParent = false;
        }
    }

    // call parent
    if(bCallParent)
    {
        Svx3DPreviewControl::MouseButtonDown(rMEvt);
    }
}

void Svx3DLightControl::Tracking( const TrackingEvent& rTEvt )
{
    if(rTEvt.IsTrackingEnded())
    {
        if(rTEvt.IsTrackingCanceled())
        {
            if(mbMouseMoved)
            {
                // interrupt tracking
                mbMouseMoved = false;

                if(mbGeometrySelected)
                {
                    SetRotation(mfSaveActionStartVer, mfSaveActionStartHor, mfSaveActionStartRotZ);
                }
                else
                {
                    SetPosition(mfSaveActionStartHor, mfSaveActionStartVer);
                }

                if(maChangeCallback.IsSet())
                {
                    maChangeCallback.Call(this);
                }
            }
        }
        else
        {
            const MouseEvent& rMEvt = rTEvt.GetMouseEvent();

            if(mbMouseMoved)
            {
                // was change interactively
            }
            else
            {
                // simple click without much movement, try selection
                TrySelection(rMEvt.GetPosPixel());
            }
        }
    }
    else
    {
        const MouseEvent& rMEvt = rTEvt.GetMouseEvent();
        Point aDeltaPos = rMEvt.GetPosPixel() - maActionStartPoint;

        if(!mbMouseMoved)
        {
            if(sal_Int32(aDeltaPos.X() * aDeltaPos.X() + aDeltaPos.Y() * aDeltaPos.Y()) > g_nInteractionStartDistance)
            {
                if(mbGeometrySelected)
                {
                    GetRotation(mfSaveActionStartVer, mfSaveActionStartHor, mfSaveActionStartRotZ);
                }
                else
                {
                    // interaction start, save values
                    GetPosition(mfSaveActionStartHor, mfSaveActionStartVer);
                }

                mbMouseMoved = true;
            }
        }

        if(mbMouseMoved)
        {
            if(mbGeometrySelected)
            {
                double fNewRotX = mfSaveActionStartVer - basegfx::deg2rad(aDeltaPos.Y());
                double fNewRotY = mfSaveActionStartHor + basegfx::deg2rad(aDeltaPos.X());

                // cut horizontal
                while(fNewRotY < 0.0)
                {
                    fNewRotY += F_2PI;
                }

                while(fNewRotY >= F_2PI)
                {
                    fNewRotY -= F_2PI;
                }

                // cut vertical
                if(fNewRotX < -F_PI2)
                {
                    fNewRotX = -F_PI2;
                }

                if(fNewRotX > F_PI2)
                {
                    fNewRotX = F_PI2;
                }

                SetRotation(fNewRotX, fNewRotY, mfSaveActionStartRotZ);

                if(maChangeCallback.IsSet())
                {
                    maChangeCallback.Call(this);
                }
            }
            else
            {
                // interaction in progress
                double fNewPosHor = mfSaveActionStartHor + static_cast<double>(aDeltaPos.X());
                double fNewPosVer = mfSaveActionStartVer - static_cast<double>(aDeltaPos.Y());

                // cut horizontal
                fNewPosHor = NormAngle360(fNewPosHor);

                // cut vertical
                if(fNewPosVer < -90.0)
                {
                    fNewPosVer = -90.0;
                }

                if(fNewPosVer > 90.0)
                {
                    fNewPosVer = 90.0;
                }

                SetPosition(fNewPosHor, fNewPosVer);

                if(maChangeCallback.IsSet())
                {
                    maChangeCallback.Call(this);
                }
            }
        }
    }
}

void Svx3DLightControl::Resize()
{
    // set size of page
    const Size aSize(PixelToLogic(GetSizePixel()));
    mpFmPage->SetSize(aSize);

    // set position and size of scene
    mpScene->SetSnapRect(tools::Rectangle(Point(0, 0), aSize));
}

void Svx3DLightControl::SetObjectType(SvxPreviewObjectType nType)
{
    // call parent
    Svx3DPreviewControl::SetObjectType(nType);

    // apply object rotation
    if(mp3DObj)
    {
        basegfx::B3DHomMatrix aObjectRotation;
        aObjectRotation.rotate(mfRotateX, mfRotateY, mfRotateZ);
        mp3DObj->SetTransform(aObjectRotation);
    }
}

bool Svx3DLightControl::IsSelectionValid()
{
    return (NO_LIGHT_SELECTED != maSelectedLight) && GetLightOnOff(maSelectedLight);
}

void Svx3DLightControl::GetPosition(double& rHor, double& rVer)
{
    if(IsSelectionValid())
    {
        basegfx::B3DVector aDirection(GetLightDirection(maSelectedLight));
        aDirection.normalize();
        rHor = basegfx::rad2deg(atan2(-aDirection.getX(), -aDirection.getZ()) + F_PI); // 0..360.0
        rVer = basegfx::rad2deg(atan2(aDirection.getY(), aDirection.getXZLength())); // -90.0..90.0
    }
    if(IsGeometrySelected())
    {
        rHor = basegfx::rad2deg(mfRotateY); // 0..360.0
        rVer = basegfx::rad2deg(mfRotateX); // -90.0..90.0
    }
}

void Svx3DLightControl::SetPosition(double fHor, double fVer)
{
    if(IsSelectionValid())
    {
        // set selected light's direction
        fHor = basegfx::deg2rad(fHor) - F_PI; // -PI..PI
        fVer = basegfx::deg2rad(fVer); // -PI2..PI2
        basegfx::B3DVector aDirection(cos(fVer) * -sin(fHor), sin(fVer), cos(fVer) * -cos(fHor));
        aDirection.normalize();

        if(!aDirection.equal(GetLightDirection(maSelectedLight)))
        {
            // set changed light direction at SdrScene
            SfxItemSet aSet(mpModel->GetItemPool());

            switch(maSelectedLight)
            {
                case 0: aSet.Put(makeSvx3DLightDirection1Item(aDirection)); break;
                case 1: aSet.Put(makeSvx3DLightDirection2Item(aDirection)); break;
                case 2: aSet.Put(makeSvx3DLightDirection3Item(aDirection)); break;
                case 3: aSet.Put(makeSvx3DLightDirection4Item(aDirection)); break;
                case 4: aSet.Put(makeSvx3DLightDirection5Item(aDirection)); break;
                case 5: aSet.Put(makeSvx3DLightDirection6Item(aDirection)); break;
                case 6: aSet.Put(makeSvx3DLightDirection7Item(aDirection)); break;
                default:
                case 7: aSet.Put(makeSvx3DLightDirection8Item(aDirection)); break;
            }

            mpScene->SetMergedItemSet(aSet);

            // correct 3D light's and LampFrame's geometries
            AdaptToSelectedLight();
            Invalidate();
        }
    }
    if(IsGeometrySelected())
    {
        if(mfRotateX != fVer || mfRotateY != fHor)
        {
            mfRotateX = basegfx::deg2rad(fVer);
            mfRotateY = basegfx::deg2rad(fHor);

            if(mp3DObj)
            {
                basegfx::B3DHomMatrix aObjectRotation;
                aObjectRotation.rotate(mfRotateX, mfRotateY, mfRotateZ);
                mp3DObj->SetTransform(aObjectRotation);

                Invalidate();
            }
        }
    }
}

void Svx3DLightControl::SetRotation(double fRotX, double fRotY, double fRotZ)
{
    if(IsGeometrySelected())
    {
        if(fRotX != mfRotateX || fRotY != mfRotateY || fRotZ != mfRotateZ)
        {
            mfRotateX = fRotX;
            mfRotateY = fRotY;
            mfRotateZ = fRotZ;

            if(mp3DObj)
            {
                basegfx::B3DHomMatrix aObjectRotation;
                aObjectRotation.rotate(mfRotateX, mfRotateY, mfRotateZ);
                mp3DObj->SetTransform(aObjectRotation);

                Invalidate();
            }
        }
    }
}

void Svx3DLightControl::GetRotation(double& rRotX, double& rRotY, double& rRotZ)
{
    rRotX = mfRotateX;
    rRotY = mfRotateY;
    rRotZ = mfRotateZ;
}

void Svx3DLightControl::Set3DAttributes( const SfxItemSet& rAttr )
{
    // call parent
    Svx3DPreviewControl::Set3DAttributes(rAttr);

    if(maSelectedLight != NO_LIGHT_SELECTED && !GetLightOnOff(maSelectedLight))
    {
        // selected light is no more active, select new one
        maSelectedLight = NO_LIGHT_SELECTED;
    }

    // local updates
    ConstructLightObjects();
    AdaptToSelectedLight();
    Invalidate();
}

void Svx3DLightControl::SelectLight(sal_uInt32 nLightNumber)
{
    if(nLightNumber > 7)
    {
        nLightNumber = NO_LIGHT_SELECTED;
    }

    if(NO_LIGHT_SELECTED != nLightNumber)
    {
        if(!GetLightOnOff(nLightNumber))
        {
            nLightNumber = NO_LIGHT_SELECTED;
        }
    }

    if(nLightNumber != maSelectedLight)
    {
        maSelectedLight = nLightNumber;
        mbGeometrySelected = false;
        ConstructLightObjects();
        AdaptToSelectedLight();
        Invalidate();
    }
}

bool Svx3DLightControl::GetLightOnOff(sal_uInt32 nNum) const
{
    if(nNum <= 7)
    {
        const SfxItemSet aLightItemSet(Get3DAttributes());

        switch(nNum)
        {
            case 0 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_1).GetValue();
            case 1 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_2).GetValue();
            case 2 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_3).GetValue();
            case 3 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_4).GetValue();
            case 4 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_5).GetValue();
            case 5 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_6).GetValue();
            case 6 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_7).GetValue();
            case 7 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_8).GetValue();
        }
    }

    return false;
}

Color Svx3DLightControl::GetLightColor(sal_uInt32 nNum) const
{
    if(nNum <= 7)
    {
        const SfxItemSet aLightItemSet(Get3DAttributes());

        switch(nNum)
        {
            case 0 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_1).GetValue();
            case 1 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_2).GetValue();
            case 2 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_3).GetValue();
            case 3 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_4).GetValue();
            case 4 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_5).GetValue();
            case 5 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_6).GetValue();
            case 6 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_7).GetValue();
            case 7 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_8).GetValue();
        }
    }

    return COL_BLACK;
}

basegfx::B3DVector Svx3DLightControl::GetLightDirection(sal_uInt32 nNum) const
{
    if(nNum <= 7)
    {
        const SfxItemSet aLightItemSet(Get3DAttributes());

        switch(nNum)
        {
            case 0 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1).GetValue();
            case 1 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2).GetValue();
            case 2 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3).GetValue();
            case 3 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4).GetValue();
            case 4 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5).GetValue();
            case 5 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6).GetValue();
            case 6 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7).GetValue();
            case 7 : return aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8).GetValue();
        }
    }

    return basegfx::B3DVector();
}

SvxLightCtl3D::SvxLightCtl3D( vcl::Window* pParent)
:   Control(pParent, WB_BORDER | WB_TABSTOP),
    maLightControl(VclPtr<Svx3DLightControl>::Create(this, 0)),
    maHorScroller(VclPtr<ScrollBar>::Create(this, WB_HORZ | WB_DRAG)),
    maVerScroller(VclPtr<ScrollBar>::Create(this, WB_VERT | WB_DRAG)),
    maSwitcher(VclPtr<PushButton>::Create(this, 0))
{
    // init members
    Init();
}

Size SvxLightCtl3D::GetOptimalSize() const
{
    return LogicToPixel(Size(80, 100), MapMode(MapUnit::MapAppFont));
}

VCL_BUILDER_FACTORY(SvxLightCtl3D)

void SvxLightCtl3D::Init()
{
    // #i58240# set HelpIDs for scrollbars and switcher
    maHorScroller->SetHelpId(HID_CTRL3D_HSCROLL);
    maVerScroller->SetHelpId(HID_CTRL3D_VSCROLL);
    maSwitcher->SetHelpId(HID_CTRL3D_SWITCHER);
    maSwitcher->SetAccessibleName(SvxResId(STR_SWITCH));

    // Light preview
    maLightControl->Show();
    maLightControl->SetChangeCallback( LINK(this, SvxLightCtl3D, InternalInteractiveChange) );
    maLightControl->SetSelectionChangeCallback( LINK(this, SvxLightCtl3D, InternalSelectionChange) );

    // Horiz Scrollbar
    maHorScroller->Show();
    maHorScroller->SetRange(Range(0, 36000));
    maHorScroller->SetLineSize(100);
    maHorScroller->SetPageSize(1000);
    maHorScroller->SetScrollHdl( LINK(this, SvxLightCtl3D, ScrollBarMove) );

    // Vert Scrollbar
    maVerScroller->Show();
    maVerScroller->SetRange(Range(0, 18000));
    maVerScroller->SetLineSize(100);
    maVerScroller->SetPageSize(1000);
    maVerScroller->SetScrollHdl( LINK(this, SvxLightCtl3D, ScrollBarMove) );

    // Switch Button
    maSwitcher->Show();
    maSwitcher->SetClickHdl( LINK(this, SvxLightCtl3D, ButtonPress) );

    // check selection
    CheckSelection();

    // new layout
    NewLayout();
}

SvxLightCtl3D::~SvxLightCtl3D()
{
    disposeOnce();
}

void SvxLightCtl3D::dispose()
{
    maLightControl.disposeAndClear();
    maHorScroller.disposeAndClear();
    maVerScroller.disposeAndClear();
    maSwitcher.disposeAndClear();
    Control::dispose();
}

void SvxLightCtl3D::Resize()
{
    // call parent
    Control::Resize();

    // new layout
    NewLayout();
}

void SvxLightCtl3D::NewLayout()
{
    // Layout members
    const Size aSize(GetOutputSizePixel());
    const sal_Int32 nScrollSize(maHorScroller->GetSizePixel().Height());

    // Preview control
    Point aPoint(0, 0);
    Size aDestSize(aSize.Width() - nScrollSize, aSize.Height() - nScrollSize);
    maLightControl->SetPosSizePixel(aPoint, aDestSize);

    // hor scrollbar
    aPoint.setY( aSize.Height() - nScrollSize );
    aDestSize.setHeight( nScrollSize );
    maHorScroller->SetPosSizePixel(aPoint, aDestSize);

    // vert scrollbar
    aPoint.setX( aSize.Width() - nScrollSize );
    aPoint.setY( 0 );
    aDestSize.setWidth( nScrollSize );
    aDestSize.setHeight( aSize.Height() - nScrollSize );
    maVerScroller->SetPosSizePixel(aPoint, aDestSize);

    // button
    aPoint.setY( aSize.Height() - nScrollSize );
    aDestSize.setHeight( nScrollSize );
    maSwitcher->SetPosSizePixel(aPoint, aDestSize);
}

void SvxLightCtl3D::CheckSelection()
{
    const bool bSelectionValid(maLightControl->IsSelectionValid() || maLightControl->IsGeometrySelected());
    maHorScroller->Enable(bSelectionValid);
    maVerScroller->Enable(bSelectionValid);

    if(bSelectionValid)
    {
        double fHor(0.0), fVer(0.0);
        maLightControl->GetPosition(fHor, fVer);
        maHorScroller->SetThumbPos( sal_Int32(fHor * 100.0) );
        maVerScroller->SetThumbPos( 18000 - sal_Int32((fVer + 90.0) * 100.0) );
    }
}

void SvxLightCtl3D::move( double fDeltaHor, double fDeltaVer )
{
    double fHor(0.0), fVer(0.0);

    maLightControl->GetPosition(fHor, fVer);
    fHor += fDeltaHor;
    fVer += fDeltaVer;

    if( fVer > 90.0 )
        return;

    if ( fVer < -90.0 )
        return;

    maLightControl->SetPosition(fHor, fVer);
    maHorScroller->SetThumbPos( sal_Int32(fHor * 100.0) );
    maVerScroller->SetThumbPos( 18000 - sal_Int32((fVer + 90.0) * 100.0) );

    if(maUserInteractiveChangeCallback.IsSet())
    {
        maUserInteractiveChangeCallback.Call(this);
    }
}

void SvxLightCtl3D::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode aCode(rKEvt.GetKeyCode());

    if( aCode.GetModifier() )
    {
        Control::KeyInput( rKEvt );
        return;
    }

    switch ( aCode.GetCode() )
    {
        case KEY_SPACE:
        {
            break;
        }
        case KEY_LEFT:
        {
            move(  -4.0,  0.0 ); // #i58242# changed move direction in X
            break;
        }
        case KEY_RIGHT:
        {
            move( 4.0,  0.0 ); // #i58242# changed move direction in X
            break;
        }
        case KEY_UP:
        {
            move(  0.0,  4.0 );
            break;
        }
        case KEY_DOWN:
        {
            move(  0.0, -4.0 );
            break;
        }
        case KEY_PAGEUP:
        {
            sal_Int32 nLight(maLightControl->GetSelectedLight() - 1);

            while((nLight >= 0) && !maLightControl->GetLightOnOff(nLight))
            {
                nLight--;
            }

            if(nLight < 0)
            {
                nLight = 7;

                while((nLight >= 0) && !maLightControl->GetLightOnOff(nLight))
                {
                    nLight--;
                }
            }

            if(nLight >= 0)
            {
                maLightControl->SelectLight(nLight);
                CheckSelection();

                if(maUserSelectionChangeCallback.IsSet())
                {
                    maUserSelectionChangeCallback.Call(this);
                }
            }

            break;
        }
        case KEY_PAGEDOWN:
        {
            sal_Int32 nLight(maLightControl->GetSelectedLight() - 1);

            while(nLight <= 7 && !maLightControl->GetLightOnOff(nLight))
            {
                nLight++;
            }

            if(nLight > 7)
            {
                nLight = 0;

                while(nLight <= 7 && !maLightControl->GetLightOnOff(nLight))
                {
                    nLight++;
                }
            }

            if(nLight <= 7)
            {
                maLightControl->SelectLight(nLight);
                CheckSelection();

                if(maUserSelectionChangeCallback.IsSet())
                {
                    maUserSelectionChangeCallback.Call(this);
                }
            }

            break;
        }
        default:
        {
            Control::KeyInput( rKEvt );
            break;
        }
    }
}

void SvxLightCtl3D::GetFocus()
{
    Control::GetFocus();

    if(HasFocus() && IsEnabled())
    {
        CheckSelection();

        Size aFocusSize = maLightControl->GetOutputSizePixel();

        aFocusSize.AdjustWidth( -4 );
        aFocusSize.AdjustHeight( -4 );

        tools::Rectangle aFocusRect( Point( 2, 2 ), aFocusSize );

        aFocusRect = maLightControl->PixelToLogic( aFocusRect );

        maLightControl->ShowFocus( aFocusRect );
    }
}

void SvxLightCtl3D::LoseFocus()
{
    Control::LoseFocus();

    maLightControl->HideFocus();
}

IMPL_LINK_NOARG(SvxLightCtl3D, ScrollBarMove, ScrollBar*, void)
{
    const sal_Int32 nHor(maHorScroller->GetThumbPos());
    const sal_Int32 nVer(maVerScroller->GetThumbPos());

    maLightControl->SetPosition(
        static_cast<double>(nHor) / 100.0,
        static_cast<double>((18000 - nVer) - 9000) / 100.0);

    if(maUserInteractiveChangeCallback.IsSet())
    {
        maUserInteractiveChangeCallback.Call(this);
    }
}

IMPL_LINK_NOARG(SvxLightCtl3D, ButtonPress, Button*, void)
{
    if(SvxPreviewObjectType::SPHERE == GetSvx3DLightControl().GetObjectType())
    {
        GetSvx3DLightControl().SetObjectType(SvxPreviewObjectType::CUBE);
    }
    else
    {
        GetSvx3DLightControl().SetObjectType(SvxPreviewObjectType::SPHERE);
    }
}

IMPL_LINK_NOARG(SvxLightCtl3D, InternalInteractiveChange, Svx3DLightControl*, void)
{
    double fHor(0.0), fVer(0.0);

    maLightControl->GetPosition(fHor, fVer);
    maHorScroller->SetThumbPos( sal_Int32(fHor * 100.0) );
    maVerScroller->SetThumbPos( 18000 - sal_Int32((fVer + 90.0) * 100.0) );

    if(maUserInteractiveChangeCallback.IsSet())
    {
        maUserInteractiveChangeCallback.Call(this);
    }
}

IMPL_LINK_NOARG(SvxLightCtl3D, InternalSelectionChange, Svx3DLightControl*, void)
{
    CheckSelection();

    if(maUserSelectionChangeCallback.IsSet())
    {
        maUserSelectionChangeCallback.Call(this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
