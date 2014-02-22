/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <svx/dlgctl3d.hxx>
#include <svx/dialogs.hrc>
#include <svx/view3d.hxx>
#include <svx/fmmodel.hxx>
#include <svl/itempool.hxx>
#include <svx/fmpage.hxx>
#include <svx/polysc3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/cube3d.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builder.hxx>
#include <svx/helperhittest3d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/polygn3d.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include "helpid.hrc"
#include <algorithm>
#include <svx/dialmgr.hxx>
#include <vcl/settings.hxx>



Svx3DPreviewControl::Svx3DPreviewControl(Window* pParent, const ResId& rResId)
:   Control(pParent, rResId),
    mpModel(0),
    mpFmPage(0),
    mp3DView(0),
    mpScene(0),
    mp3DObj(0),
    mnObjectType(PREVIEW_OBJECTTYPE_SPHERE)
{
    Construct();

    
    SetControlBackground();
    SetBackground();
}

Svx3DPreviewControl::Svx3DPreviewControl(Window* pParent, WinBits nStyle)
:   Control(pParent, nStyle),
    mpModel(0),
    mpFmPage(0),
    mp3DView(0),
    mpScene(0),
    mp3DObj(0),
    mnObjectType(PREVIEW_OBJECTTYPE_SPHERE)
{
    Construct();

    
    SetControlBackground();
    SetBackground();
}

Svx3DPreviewControl::~Svx3DPreviewControl()
{
    delete mp3DView;
    delete mpModel;
}

void Svx3DPreviewControl::Construct()
{
    
    
    EnableRTL (false);
    SetMapMode( MAP_100TH_MM );

    
    mpModel = new FmFormModel();
    mpModel->GetItemPool().FreezeIdRanges();

    
    mpFmPage = new FmFormPage( *mpModel );
    mpModel->InsertPage( mpFmPage, 0 );

    
    mp3DView = new E3dView( mpModel, this );
    mp3DView->SetBufferedOutputAllowed(true);
    mp3DView->SetBufferedOverlayAllowed(true);

    
    mpScene = new E3dPolyScene(mp3DView->Get3DDefaultAttributes());

    
    SetObjectType(PREVIEW_OBJECTTYPE_SPHERE);

    
    Camera3D& rCamera  = (Camera3D&) mpScene->GetCamera();
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
    rCamera.SetDefaults(basegfx::B3DPoint(0.0, 0.0, fDefaultCamPosZ), aLookAt, fDefaultCamFocal);

    mpScene->SetCamera( rCamera );
    mpFmPage->InsertObject( mpScene );

    basegfx::B3DHomMatrix aRotation;
    aRotation.rotate(DEG2RAD( 25 ), 0.0, 0.0);
    aRotation.rotate(0.0, DEG2RAD( 40 ), 0.0);
    mpScene->SetTransform(aRotation * mpScene->GetTransform());

    
    mpScene->SetRectsDirty();

    SfxItemSet aSet( mpModel->GetItemPool(),
        XATTR_LINESTYLE, XATTR_LINESTYLE,
        XATTR_FILL_FIRST, XATTR_FILLBITMAP,
        0, 0 );
    aSet.Put( XLineStyleItem( XLINE_NONE ) );
    aSet.Put( XFillStyleItem( XFILL_SOLID ) );
    aSet.Put( XFillColorItem( "", Color( COL_WHITE ) ) );

    mpScene->SetMergedItemSet(aSet);

    
    SdrPageView* pPageView = mp3DView->ShowSdrPage( mpFmPage );
    mp3DView->hideMarkHandles();

    
    mp3DView->MarkObj( mpScene, pPageView );
}

void Svx3DPreviewControl::Resize()
{
    
    Size aSize( GetSizePixel() );
    aSize = PixelToLogic( aSize );
    mpFmPage->SetSize( aSize );

    
    Size aObjSize( aSize.Width()*5/6, aSize.Height()*5/6 );
    Point aObjPoint( (aSize.Width() - aObjSize.Width()) / 2,
        (aSize.Height() - aObjSize.Height()) / 2);
    Rectangle aRect( aObjPoint, aObjSize);
    mpScene->SetSnapRect( aRect );
}

void Svx3DPreviewControl::Paint(const Rectangle& rRect)
{
    mp3DView->CompleteRedraw(this, Region(rRect));
}

void Svx3DPreviewControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    Control::MouseButtonDown(rMEvt);

    if( rMEvt.IsShift() && rMEvt.IsMod1() )
    {
        if(PREVIEW_OBJECTTYPE_SPHERE == GetObjectType())
        {
            SetObjectType(PREVIEW_OBJECTTYPE_CUBE);
        }
        else
        {
            SetObjectType(PREVIEW_OBJECTTYPE_SPHERE);
        }
    }
}

void Svx3DPreviewControl::SetObjectType(sal_uInt16 nType)
{
    if( mnObjectType != nType || !mp3DObj)
    {
        SfxItemSet aSet(mpModel->GetItemPool(), SDRATTR_START, SDRATTR_END, 0, 0);
        mnObjectType = nType;

        if( mp3DObj )
        {
            aSet.Put(mp3DObj->GetMergedItemSet());
            mpScene->Remove3DObj( mp3DObj );
            delete mp3DObj;
            mp3DObj = NULL;
        }

        switch( nType )
        {
            case PREVIEW_OBJECTTYPE_SPHERE:
            {
                mp3DObj = new E3dSphereObj(
                    mp3DView->Get3DDefaultAttributes(),
                    basegfx::B3DPoint( 0, 0, 0 ),
                    basegfx::B3DVector( 5000, 5000, 5000 ));
            }
            break;

            case PREVIEW_OBJECTTYPE_CUBE:
            {
                mp3DObj = new E3dCubeObj(
                    mp3DView->Get3DDefaultAttributes(),
                    basegfx::B3DPoint( -2500, -2500, -2500 ),
                    basegfx::B3DVector( 5000, 5000, 5000 ));
            }
            break;
        }

        mpScene->Insert3DObj( mp3DObj );
        mp3DObj->SetMergedItemSet(aSet);

        Resize();
    }
}

SfxItemSet Svx3DPreviewControl::Get3DAttributes() const
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

Svx3DLightControl::Svx3DLightControl(Window* pParent, WinBits nStyle)
:   Svx3DPreviewControl(pParent, nStyle),
    maUserInteractiveChangeCallback(),
    maUserSelectionChangeCallback(),
    maChangeCallback(),
    maSelectionChangeCallback(),
    maSelectedLight(NO_LIGHT_SELECTED),
    mpExpansionObject(0),
    mpLampBottomObject(0),
    mpLampShaftObject(0),
    maLightObjects(MAX_NUMBER_LIGHTS, (E3dObject*)0),
    mfRotateX(-20.0),
    mfRotateY(45.0),
    mfRotateZ(0.0),
    maActionStartPoint(),
    mnInteractionStartDistance(5 * 5 * 2),
    mfSaveActionStartHor(0.0),
    mfSaveActionStartVer(0.0),
    mfSaveActionStartRotZ(0.0),
    mbMouseMoved(false),
    mbGeometrySelected(false)
{
    Construct2();
}

Svx3DLightControl::~Svx3DLightControl()
{
    
    
}

void Svx3DLightControl::Construct2()
{
    {
        
        const Color aDialogColor(Application::GetSettings().GetStyleSettings().GetDialogColor());
        mp3DView->SetPageVisible(false);
        mp3DView->SetApplicationBackgroundColor(aDialogColor);
        mp3DView->SetApplicationDocumentColor(aDialogColor);
    }

    {
        
        const double fMaxExpansion(RADIUS_LAMP_BIG + RADIUS_LAMP_PREVIEW_SIZE);
        mpExpansionObject = new E3dCubeObj(
            mp3DView->Get3DDefaultAttributes(),
            basegfx::B3DPoint(-fMaxExpansion, -fMaxExpansion, -fMaxExpansion),
            basegfx::B3DVector(2.0 * fMaxExpansion, 2.0 * fMaxExpansion, 2.0 * fMaxExpansion));
        mpScene->Insert3DObj( mpExpansionObject );
        SfxItemSet aSet(mpModel->GetItemPool());
        aSet.Put( XLineStyleItem( XLINE_NONE ) );
        aSet.Put( XFillStyleItem( XFILL_NONE ) );
        mpExpansionObject->SetMergedItemSet(aSet);
    }

    {
        
        
        const basegfx::B2DPolygon a2DCircle(basegfx::tools::createPolygonFromCircle(basegfx::B2DPoint(0.0, 0.0), RADIUS_LAMP_PREVIEW_SIZE));
        basegfx::B3DPolygon a3DCircle(basegfx::tools::createB3DPolygonFromB2DPolygon(a2DCircle));
        basegfx::B3DHomMatrix aTransform;

        aTransform.rotate(F_PI2, 0.0, 0.0);
        aTransform.translate(0.0, -RADIUS_LAMP_PREVIEW_SIZE, 0.0);
        a3DCircle.transform(aTransform);

        
        mpLampBottomObject = new E3dPolygonObj(
            mp3DView->Get3DDefaultAttributes(),
            basegfx::B3DPolyPolygon(a3DCircle),
            true);
        mpScene->Insert3DObj( mpLampBottomObject );

        
        basegfx::B2DPolygon a2DHalfCircle;
        a2DHalfCircle.append(basegfx::B2DPoint(RADIUS_LAMP_PREVIEW_SIZE, 0.0));
        a2DHalfCircle.append(basegfx::B2DPoint(RADIUS_LAMP_PREVIEW_SIZE, -RADIUS_LAMP_PREVIEW_SIZE));
        a2DHalfCircle.append(basegfx::tools::createPolygonFromEllipseSegment(
            basegfx::B2DPoint(0.0, 0.0), RADIUS_LAMP_PREVIEW_SIZE, RADIUS_LAMP_PREVIEW_SIZE, F_2PI - F_PI2, F_PI2));
        basegfx::B3DPolygon a3DHalfCircle(basegfx::tools::createB3DPolygonFromB2DPolygon(a2DHalfCircle));

        
        mpLampShaftObject = new E3dPolygonObj(
            mp3DView->Get3DDefaultAttributes(),
            basegfx::B3DPolyPolygon(a3DHalfCircle),
            true);
        mpScene->Insert3DObj( mpLampShaftObject );

        
        SfxItemSet aSet(mpModel->GetItemPool());
        aSet.Put( XLineStyleItem( XLINE_NONE ) );
        aSet.Put( XFillStyleItem( XFILL_NONE ) );

        mpLampBottomObject->SetMergedItemSet(aSet);
        mpLampShaftObject->SetMergedItemSet(aSet);
    }

    {
        
        Camera3D& rCamera  = (Camera3D&) mpScene->GetCamera();
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
        rCamera.SetDefaults(basegfx::B3DPoint(0.0, 0.0, fDefaultCamPosZ), aLookAt, fDefaultCamFocal);

        mpScene->SetCamera( rCamera );

        basegfx::B3DHomMatrix aNeutral;
        mpScene->SetTransform(aNeutral);
    }

    
    mpScene->SetRectsDirty();
}

void Svx3DLightControl::ConstructLightObjects()
{
    for(sal_uInt32 a(0); a < MAX_NUMBER_LIGHTS; a++)
    {
        
        if(maLightObjects[a])
        {
            mpScene->Remove3DObj(maLightObjects[a]);
            delete maLightObjects[a];
            maLightObjects[a] = 0;
        }

        if(GetLightOnOff(a))
        {
            const bool bIsSelectedLight(a == maSelectedLight);
            basegfx::B3DVector aDirection(GetLightDirection(a));
            aDirection.normalize();
            aDirection *= RADIUS_LAMP_PREVIEW_SIZE;

            const double fLampSize(bIsSelectedLight ? RADIUS_LAMP_BIG : RADIUS_LAMP_SMALL);
            E3dObject* pNewLight = new E3dSphereObj(
                mp3DView->Get3DDefaultAttributes(),
                basegfx::B3DPoint( 0, 0, 0 ),
                basegfx::B3DVector( fLampSize, fLampSize, fLampSize));
            mpScene->Insert3DObj(pNewLight);

            basegfx::B3DHomMatrix aTransform;
            aTransform.translate(aDirection.getX(), aDirection.getY(), aDirection.getZ());
            pNewLight->SetTransform(aTransform);

            SfxItemSet aSet(mpModel->GetItemPool());
            aSet.Put( XLineStyleItem( XLINE_NONE ) );
            aSet.Put( XFillStyleItem( XFILL_SOLID ) );
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
        
        SfxItemSet aSet(mpModel->GetItemPool());
        aSet.Put( XLineStyleItem( XLINE_NONE ) );
        aSet.Put( XFillStyleItem( XFILL_NONE ) );
        mpLampBottomObject->SetMergedItemSet(aSet);
        mpLampShaftObject->SetMergedItemSet(aSet);
    }
    else
    {
        basegfx::B3DVector aDirection(GetLightDirection(maSelectedLight));
        aDirection.normalize();

        
        SfxItemSet aSet(mpModel->GetItemPool());
        aSet.Put( XLineStyleItem( XLINE_SOLID ) );
        aSet.Put( XLineColorItem(OUString(), COL_YELLOW));
        aSet.Put( XLineWidthItem(0));
        aSet.Put( XFillStyleItem( XFILL_NONE ) );
        mpLampBottomObject->SetMergedItemSet(aSet);
        mpLampShaftObject->SetMergedItemSet(aSet);

        
        basegfx::B3DHomMatrix aTransform;
        double fRotateY(0.0);

        if(!basegfx::fTools::equalZero(aDirection.getZ()) || !basegfx::fTools::equalZero(aDirection.getX()))
        {
            fRotateY = atan2(-aDirection.getZ(), aDirection.getX());
        }

        aTransform.rotate(0.0, fRotateY, 0.0);
        mpLampShaftObject->SetTransform(aTransform);

        
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
            
            
            const E3dCompoundObject* pResult = 0;

            for(sal_uInt32 b(0); !pResult && b < aResult.size(); b++)
            {
                if(aResult[b] && aResult[b] != mpExpansionObject)
                {
                    pResult = aResult[b];
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

void Svx3DLightControl::Paint(const Rectangle& rRect)
{
    Svx3DPreviewControl::Paint(rRect);
}

void Svx3DLightControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bCallParent(true);

    
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
            
            TrySelection(rMEvt.GetPosPixel());
            bCallParent = false;
        }
    }

    
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
                
            }
            else
            {
                
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
            if(sal_Int32(aDeltaPos.X() * aDeltaPos.X() + aDeltaPos.Y() * aDeltaPos.Y()) > mnInteractionStartDistance)
            {
                if(mbGeometrySelected)
                {
                    GetRotation(mfSaveActionStartVer, mfSaveActionStartHor, mfSaveActionStartRotZ);
                }
                else
                {
                    
                    GetPosition(mfSaveActionStartHor, mfSaveActionStartVer);
                }

                mbMouseMoved = true;
            }
        }

        if(mbMouseMoved)
        {
            if(mbGeometrySelected)
            {
                double fNewRotX = mfSaveActionStartVer - ((double)aDeltaPos.Y() * F_PI180);
                double fNewRotY = mfSaveActionStartHor + ((double)aDeltaPos.X() * F_PI180);

                
                while(fNewRotY < 0.0)
                {
                    fNewRotY += F_2PI;
                }

                while(fNewRotY >= F_2PI)
                {
                    fNewRotY -= F_2PI;
                }

                
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
                
                double fNewPosHor = mfSaveActionStartHor + ((double)aDeltaPos.X());
                double fNewPosVer = mfSaveActionStartVer - ((double)aDeltaPos.Y());

                
                while(fNewPosHor < 0.0)
                {
                    fNewPosHor += 360.0;
                }

                while(fNewPosHor >= 360.0)
                {
                    fNewPosHor -= 360.0;
                }

                
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
    
    const Size aSize(PixelToLogic(GetSizePixel()));
    mpFmPage->SetSize(aSize);

    
    mpScene->SetSnapRect(Rectangle(Point(0, 0), aSize));
}

void Svx3DLightControl::SetObjectType(sal_uInt16 nType)
{
    
    Svx3DPreviewControl::SetObjectType(nType);

    
    if(mp3DObj)
    {
        basegfx::B3DHomMatrix aObjectRotation;
        aObjectRotation.rotate(mfRotateX, mfRotateY, mfRotateZ);
        mp3DObj->SetTransform(aObjectRotation);
    }
}

bool Svx3DLightControl::IsSelectionValid()
{
    if((NO_LIGHT_SELECTED != maSelectedLight) && (GetLightOnOff(maSelectedLight)))
    {
        return true;
    }

    return false;
}

void Svx3DLightControl::GetPosition(double& rHor, double& rVer)
{
    if(IsSelectionValid())
    {
        basegfx::B3DVector aDirection(GetLightDirection(maSelectedLight));
        aDirection.normalize();
        rHor = atan2(-aDirection.getX(), -aDirection.getZ()) + F_PI; 
        rVer = atan2(aDirection.getY(), aDirection.getXZLength()); 
        rHor /= F_PI180; 
        rVer /= F_PI180; 
    }
    if(IsGeometrySelected())
    {
        rHor = mfRotateY / F_PI180; 
        rVer = mfRotateX / F_PI180; 
    }
}

void Svx3DLightControl::SetPosition(double fHor, double fVer)
{
    if(IsSelectionValid())
    {
        
        fHor = (fHor * F_PI180) - F_PI; 
        fVer *= F_PI180; 
        basegfx::B3DVector aDirection(cos(fVer) * -sin(fHor), sin(fVer), cos(fVer) * -cos(fHor));
        aDirection.normalize();

        if(!aDirection.equal(GetLightDirection(maSelectedLight)))
        {
            
            SfxItemSet aSet(mpModel->GetItemPool());

            switch(maSelectedLight)
            {
                case 0: aSet.Put(Svx3DLightDirection1Item(aDirection)); break;
                case 1: aSet.Put(Svx3DLightDirection2Item(aDirection)); break;
                case 2: aSet.Put(Svx3DLightDirection3Item(aDirection)); break;
                case 3: aSet.Put(Svx3DLightDirection4Item(aDirection)); break;
                case 4: aSet.Put(Svx3DLightDirection5Item(aDirection)); break;
                case 5: aSet.Put(Svx3DLightDirection6Item(aDirection)); break;
                case 6: aSet.Put(Svx3DLightDirection7Item(aDirection)); break;
                default:
                case 7: aSet.Put(Svx3DLightDirection8Item(aDirection)); break;
            }

            mpScene->SetMergedItemSet(aSet);

            
            AdaptToSelectedLight();
            Invalidate();
        }
    }
    if(IsGeometrySelected())
    {
        if(mfRotateX != fVer || mfRotateY != fHor)
        {
            mfRotateX = fVer * F_PI180;
            mfRotateY = fHor * F_PI180;

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
    
    Svx3DPreviewControl::Set3DAttributes(rAttr);

    if(maSelectedLight != NO_LIGHT_SELECTED && !GetLightOnOff(maSelectedLight))
    {
        
        maSelectedLight = NO_LIGHT_SELECTED;
    }

    
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
            case 0 : return ((const Svx3DLightOnOff1Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_1)).GetValue();
            case 1 : return ((const Svx3DLightOnOff2Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_2)).GetValue();
            case 2 : return ((const Svx3DLightOnOff3Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_3)).GetValue();
            case 3 : return ((const Svx3DLightOnOff4Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_4)).GetValue();
            case 4 : return ((const Svx3DLightOnOff5Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_5)).GetValue();
            case 5 : return ((const Svx3DLightOnOff6Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_6)).GetValue();
            case 6 : return ((const Svx3DLightOnOff7Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_7)).GetValue();
            case 7 : return ((const Svx3DLightOnOff8Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTON_8)).GetValue();
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
            case 0 : return ((const Svx3DLightcolor1Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_1)).GetValue();
            case 1 : return ((const Svx3DLightcolor2Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_2)).GetValue();
            case 2 : return ((const Svx3DLightcolor3Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_3)).GetValue();
            case 3 : return ((const Svx3DLightcolor4Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_4)).GetValue();
            case 4 : return ((const Svx3DLightcolor5Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_5)).GetValue();
            case 5 : return ((const Svx3DLightcolor6Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_6)).GetValue();
            case 6 : return ((const Svx3DLightcolor7Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_7)).GetValue();
            case 7 : return ((const Svx3DLightcolor8Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_8)).GetValue();
        }
    }

    return Color(COL_BLACK);
}

basegfx::B3DVector Svx3DLightControl::GetLightDirection(sal_uInt32 nNum) const
{
    if(nNum <= 7)
    {
        const SfxItemSet aLightItemSet(Get3DAttributes());

        switch(nNum)
        {
            case 0 : return ((const Svx3DLightDirection1Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1)).GetValue();
            case 1 : return ((const Svx3DLightDirection2Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2)).GetValue();
            case 2 : return ((const Svx3DLightDirection3Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3)).GetValue();
            case 3 : return ((const Svx3DLightDirection4Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4)).GetValue();
            case 4 : return ((const Svx3DLightDirection5Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5)).GetValue();
            case 5 : return ((const Svx3DLightDirection6Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6)).GetValue();
            case 6 : return ((const Svx3DLightDirection7Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7)).GetValue();
            case 7 : return ((const Svx3DLightDirection8Item&)aLightItemSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8)).GetValue();
        }
    }

    return basegfx::B3DVector();
}



SvxLightCtl3D::SvxLightCtl3D( Window* pParent, const ResId& rResId)
:   Control(pParent, rResId),
    maLightControl(this, 0),
    maHorScroller(this, WB_HORZ | WB_DRAG),
    maVerScroller(this, WB_VERT | WB_DRAG),
    maSwitcher(this, 0)
{
    
    Init();
}

SvxLightCtl3D::SvxLightCtl3D( Window* pParent)
:   Control(pParent, WB_BORDER | WB_TABSTOP),
    maLightControl(this, 0),
    maHorScroller(this, WB_HORZ | WB_DRAG),
    maVerScroller(this, WB_VERT | WB_DRAG),
    maSwitcher(this, 0)
{
    
    Init();
}

Size SvxLightCtl3D::GetOptimalSize() const
{
    return LogicToPixel(Size(80, 100), MAP_APPFONT);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxLightCtl3D(Window *pParent, VclBuilder::stringmap &)
{
    return new SvxLightCtl3D(pParent);
}


void SvxLightCtl3D::Init()
{
    
    maHorScroller.SetHelpId(HID_CTRL3D_HSCROLL);
    maVerScroller.SetHelpId(HID_CTRL3D_VSCROLL);
    maSwitcher.SetHelpId(HID_CTRL3D_SWITCHER);
    maSwitcher.SetAccessibleName(SVX_RESSTR(STR_SWITCH));

    
    maLightControl.Show();
    maLightControl.SetChangeCallback( LINK(this, SvxLightCtl3D, InternalInteractiveChange) );
    maLightControl.SetSelectionChangeCallback( LINK(this, SvxLightCtl3D, InternalSelectionChange) );

    
    maHorScroller.Show();
    maHorScroller.SetRange(Range(0, 36000));
    maHorScroller.SetLineSize(100);
    maHorScroller.SetPageSize(1000);
    maHorScroller.SetScrollHdl( LINK(this, SvxLightCtl3D, ScrollBarMove) );

    
    maVerScroller.Show();
    maVerScroller.SetRange(Range(0, 18000));
    maVerScroller.SetLineSize(100);
    maVerScroller.SetPageSize(1000);
    maVerScroller.SetScrollHdl( LINK(this, SvxLightCtl3D, ScrollBarMove) );

    
    maSwitcher.Show();
    maSwitcher.SetClickHdl( LINK(this, SvxLightCtl3D, ButtonPress) );

    
    CheckSelection();

    
    NewLayout();
}

SvxLightCtl3D::~SvxLightCtl3D()
{
}

void SvxLightCtl3D::Resize()
{
    
    Control::Resize();

    
    NewLayout();
}

void SvxLightCtl3D::NewLayout()
{
    
    const Size aSize(GetOutputSizePixel());
    const sal_Int32 nScrollSize(maHorScroller.GetSizePixel().Height());

    
    Point aPoint(0, 0);
    Size aDestSize(aSize.Width() - nScrollSize, aSize.Height() - nScrollSize);
    maLightControl.SetPosSizePixel(aPoint, aDestSize);

    
    aPoint.Y() = aSize.Height() - nScrollSize;
    aDestSize.Height() = nScrollSize;
    maHorScroller.SetPosSizePixel(aPoint, aDestSize);

    
    aPoint.X() = aSize.Width() - nScrollSize;
    aPoint.Y() = 0;
    aDestSize.Width() = nScrollSize;
    aDestSize.Height() = aSize.Height() - nScrollSize;
    maVerScroller.SetPosSizePixel(aPoint, aDestSize);

    
    aPoint.Y() = aSize.Height() - nScrollSize;
    aDestSize.Height() = nScrollSize;
    maSwitcher.SetPosSizePixel(aPoint, aDestSize);
}

void SvxLightCtl3D::CheckSelection()
{
    const bool bSelectionValid(maLightControl.IsSelectionValid() || maLightControl.IsGeometrySelected());
    maHorScroller.Enable(bSelectionValid);
    maVerScroller.Enable(bSelectionValid);

    if(bSelectionValid)
    {
        double fHor, fVer;
        maLightControl.GetPosition(fHor, fVer);
        maHorScroller.SetThumbPos( sal_Int32(fHor * 100.0) );
        maVerScroller.SetThumbPos( 18000 - sal_Int32((fVer + 90.0) * 100.0) );
    }
}

void SvxLightCtl3D::move( double fDeltaHor, double fDeltaVer )
{
    double fHor, fVer;

    maLightControl.GetPosition(fHor, fVer);
    fHor += fDeltaHor;
    fVer += fDeltaVer;

    if( fVer > 90.0 )
        return;

    if ( fVer < -90.0 )
        return;

    maLightControl.SetPosition(fHor, fVer);
    maHorScroller.SetThumbPos( sal_Int32(fHor * 100.0) );
    maVerScroller.SetThumbPos( 18000 - sal_Int32((fVer + 90.0) * 100.0) );

    if(maUserInteractiveChangeCallback.IsSet())
    {
        maUserInteractiveChangeCallback.Call(this);
    }
}

void SvxLightCtl3D::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode aCode(rKEvt.GetKeyCode());

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
            move(  -4.0,  0.0 ); 
            break;
        }
        case KEY_RIGHT:
        {
            move( 4.0,  0.0 ); 
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
            sal_Int32 nLight(maLightControl.GetSelectedLight() - 1);

            while((nLight >= 0) && !maLightControl.GetLightOnOff(nLight))
            {
                nLight--;
            }

            if(nLight < 0)
            {
                nLight = 7;

                while((nLight >= 0) && !maLightControl.GetLightOnOff(nLight))
                {
                    nLight--;
                }
            }

            if(nLight >= 0)
            {
                maLightControl.SelectLight(nLight);
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
            sal_Int32 nLight(maLightControl.GetSelectedLight() - 1);

            while(nLight <= 7 && !maLightControl.GetLightOnOff(nLight))
            {
                nLight++;
            }

            if(nLight > 7)
            {
                nLight = 0;

                while(nLight <= 7 && !maLightControl.GetLightOnOff(nLight))
                {
                    nLight++;
                }
            }

            if(nLight <= 7)
            {
                maLightControl.SelectLight(nLight);
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

        Size aFocusSize = maLightControl.GetOutputSizePixel();

        aFocusSize.Width() -= 4;
        aFocusSize.Height() -= 4;

        Rectangle aFocusRect( Point( 2, 2 ), aFocusSize );

        aFocusRect = maLightControl.PixelToLogic( aFocusRect );

        maLightControl.ShowFocus( aFocusRect );
    }
}

void SvxLightCtl3D::LoseFocus()
{
    Control::LoseFocus();

    maLightControl.HideFocus();
}

IMPL_LINK_NOARG(SvxLightCtl3D, ScrollBarMove)
{
    const sal_Int32 nHor(maHorScroller.GetThumbPos());
    const sal_Int32 nVer(maVerScroller.GetThumbPos());

    maLightControl.SetPosition(
        ((double)nHor) / 100.0,
        ((double)((18000 - nVer) - 9000)) / 100.0);

    if(maUserInteractiveChangeCallback.IsSet())
    {
        maUserInteractiveChangeCallback.Call(this);
    }

    return 0;
}

IMPL_LINK_NOARG(SvxLightCtl3D, ButtonPress)
{
    if(PREVIEW_OBJECTTYPE_SPHERE == GetSvx3DLightControl().GetObjectType())
    {
        GetSvx3DLightControl().SetObjectType(PREVIEW_OBJECTTYPE_CUBE);
    }
    else
    {
        GetSvx3DLightControl().SetObjectType(PREVIEW_OBJECTTYPE_SPHERE);
    }

    return 0;
}

IMPL_LINK_NOARG(SvxLightCtl3D, InternalInteractiveChange)
{
    double fHor, fVer;

    maLightControl.GetPosition(fHor, fVer);
    maHorScroller.SetThumbPos( sal_Int32(fHor * 100.0) );
    maVerScroller.SetThumbPos( 18000 - sal_Int32((fVer + 90.0) * 100.0) );

    if(maUserInteractiveChangeCallback.IsSet())
    {
        maUserInteractiveChangeCallback.Call(this);
    }

    return 0;
}

IMPL_LINK_NOARG(SvxLightCtl3D, InternalSelectionChange)
{
    CheckSelection();

    if(maUserSelectionChangeCallback.IsSet())
    {
        maUserSelectionChangeCallback.Call(this);
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
