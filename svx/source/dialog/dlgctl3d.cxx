/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlgctl3d.cxx,v $
 * $Revision: 1.20 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/dialogs.hrc>
#include <svx/xflclit.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svx/view3d.hxx>
#include <svx/polysc3d.hxx>
#include <svx/obj3d.hxx>
#include <svx/camera3d.hxx>
#include <svx/volume3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/cube3d.hxx>
#include <vcl/event.hxx>
#include <svtools/itempool.hxx>
#include <svtools/style.hxx>

#include <svx/dlgctl3d.hxx>
#include <goodies/base3d.hxx>
#include <tools/link.hxx>

// #i58240#
#ifndef _SVX_HELPID_HRC
#include "helpid.hrc"
#endif

/*************************************************************************
|*  3D Preview Control
|*  Ctor
\************************************************************************/
Svx3DPreviewControl::Svx3DPreviewControl( Window* pParent, const ResId& rResId ) :
            Control     ( pParent, rResId ),
            pModel      ( NULL ),
            pFmPage     ( NULL ),
            p3DView     ( NULL ),
            pScene      ( NULL ),
            p3DObj      ( NULL ),
            nObjectType ( PREVIEW_OBJECTTYPE_SPHERE )
{
    Construct();
}

/*************************************************************************
|* Ctor
\************************************************************************/
Svx3DPreviewControl::Svx3DPreviewControl( Window* pParent, WinBits nStyle ) :
            Control     ( pParent, nStyle ),
            pModel      ( NULL ),
            pFmPage     ( NULL ),
            p3DView     ( NULL ),
            pScene      ( NULL ),
            p3DObj      ( NULL ),
            nObjectType ( PREVIEW_OBJECTTYPE_SPHERE )
{
    Construct();
}

/*************************************************************************
|* Dtor
\************************************************************************/
Svx3DPreviewControl::~Svx3DPreviewControl()
{
    delete p3DView;
    delete pModel;
}

/*************************************************************************
|* Svx3DPreviewControl::Construct
\************************************************************************/
void Svx3DPreviewControl::Construct()
{
    // Do never mirror the preview window.  This explicitly includes right
    // to left writing environments.
    EnableRTL (FALSE);

    SetMapMode( MAP_100TH_MM );

    // Model
    pModel = new FmFormModel();
    pModel->GetItemPool().FreezeIdRanges();

    // Page
    pFmPage = new FmFormPage( *pModel, NULL );
    pModel->InsertPage( pFmPage, 0 );

    // 3D View
    p3DView = new E3dView( pModel, this );

    // 3D Scene
    pScene = new E3dPolyScene(p3DView->Get3DDefaultAttributes());

    // Objekt
    SetObjectType(PREVIEW_OBJECTTYPE_SPHERE);

    // Kameraeinstellungen, Perspektive ...
    Camera3D& rCamera  = (Camera3D&) pScene->GetCamera();
    const Volume3D& rVolume = pScene->GetBoundVolume();
    double fW = rVolume.getWidth();
    double fH = rVolume.getHeight();
    double fCamZ = rVolume.getMaxZ() + ((fW + fH) / 2.0);

    rCamera.SetAutoAdjustProjection(FALSE);
    rCamera.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
    basegfx::B3DPoint aLookAt;
    double fDefaultCamPosZ = p3DView->GetDefaultCamPosZ();
    basegfx::B3DPoint aCamPos(0.0, 0.0, fCamZ < fDefaultCamPosZ ? fDefaultCamPosZ : fCamZ);
    rCamera.SetPosAndLookAt(aCamPos, aLookAt);
    double fDefaultCamFocal = p3DView->GetDefaultCamFocal();
    rCamera.SetFocalLength(fDefaultCamFocal);
    rCamera.SetDefaults(basegfx::B3DPoint(0.0, 0.0, fDefaultCamPosZ), aLookAt, fDefaultCamFocal);

    pScene->SetCamera( rCamera );
    pFmPage->InsertObject( pScene );

    pScene->RotateX( DEG2RAD( 25 ) );
    pScene->RotateY( DEG2RAD( 40 ) ); // Weil es auch ein Wuerfel sein kann

    // SnapRects der Objekte ungueltig
    pScene->SetRectsDirty();

    // Transformationen initialisieren, damit bei RecalcSnapRect()
    // richtig gerechnet wird
    pScene->InitTransformationSet();

    SfxItemSet aSet( pModel->GetItemPool(),
        XATTR_LINESTYLE, XATTR_LINESTYLE,
        XATTR_FILL_FIRST, XATTR_FILLBITMAP,
        0, 0 );
    aSet.Put( XLineStyleItem( XLINE_NONE ) );
    aSet.Put( XFillStyleItem( XFILL_SOLID ) );
    aSet.Put( XFillColorItem( String(), Color( COL_WHITE ) ) );

//-/    pScene->NbcSetAttributes( aSet, FALSE );
    pScene->SetMergedItemSet(aSet);

    // Default-Attribute holen (ohne markiertes Objekt)
//  SfxItemSet aDefaultSet = p3DView->Get3DAttributes();

    // PageView
    SdrPageView* pPageView = p3DView->ShowSdrPage( pFmPage );
//  SdrPageView* pPageView = p3DView->ShowPage( pFmPage, Point() );
    p3DView->hideMarkHandles();

    // Szene markieren
    p3DView->MarkObj( pScene, pPageView );

    // Initiale Groesse
    pScene->FitSnapRectToBoundVol();
//  Set3DAttributes(aDefaultSet);
}

/*************************************************************************
|* Svx3DPreviewControl::Resize
\************************************************************************/
void Svx3DPreviewControl::Resize()
{
    // Seite der Page
    Size aSize( GetSizePixel() );
    aSize = PixelToLogic( aSize );
    pFmPage->SetSize( aSize );

    // Groesse setzen
    Size aObjSize( aSize.Width()*5/6, aSize.Height()*5/6 );
    Point aObjPoint( (aSize.Width() - aObjSize.Width()) / 2,
        (aSize.Height() - aObjSize.Height()) / 2);
    Rectangle aRect( aObjPoint, aObjSize);
    pScene->SetSnapRect( aRect );
}

/*************************************************************************
|* Svx3DPreviewControl::Paint
\************************************************************************/
void Svx3DPreviewControl::Paint( const Rectangle& rRect )
{
    p3DView->CompleteRedraw( this, Region( rRect ) );
}

/*************************************************************************
|* Svx3DPreviewControl::MouseButtonDown
\************************************************************************/
void Svx3DPreviewControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    Control::MouseButtonDown( rMEvt );
    if( rMEvt.IsShift() && rMEvt.IsMod1() )
    {
        SetObjectType( (nObjectType+1) % 2 );
    }
}

/*************************************************************************
|* Svx3DPreviewControl::SetObjectType
\************************************************************************/
void Svx3DPreviewControl::SetObjectType( UINT16 nType )
{
    if( nObjectType != nType || !p3DObj)
    {
        SfxItemSet aSet(
            pModel->GetItemPool(),
            SDRATTR_START,  SDRATTR_END,
            0, 0);

        nObjectType = nType;

        if( p3DObj )
        {
//-/            p3DObj->TakeAttributes( aSet, FALSE, FALSE );
            aSet.Put(p3DObj->GetMergedItemSet());

            pScene->Remove3DObj( p3DObj );
            delete p3DObj;
            p3DObj = NULL;
        }

        switch( nType )
        {
            case PREVIEW_OBJECTTYPE_SPHERE:
            {
                // Kugel erzeugen
                p3DObj = new E3dSphereObj(
                    p3DView->Get3DDefaultAttributes(),
                    basegfx::B3DPoint( 0, 0, 0 ),
                    basegfx::B3DVector( 5000, 5000, 5000 ));
            }
            break;

            case PREVIEW_OBJECTTYPE_CUBE:
            {
                // Wuerfel erzeugen
                p3DObj = new E3dCubeObj(
                    p3DView->Get3DDefaultAttributes(),
                    basegfx::B3DPoint( -2500, -2500, -2500 ),
                    basegfx::B3DVector( 5000, 5000, 5000 ));
            }
            break;
        }

        // Rein in die Szene
        pScene->Insert3DObj( p3DObj );

//-/        p3DObj->NbcSetAttributes( aSet, FALSE );
        p3DObj->SetMergedItemSet(aSet);

        // Refresh
        Resize();
    }
}

/*************************************************************************
|* Svx3DPreviewControl::Get3DAttributes
\************************************************************************/
SfxItemSet Svx3DPreviewControl::Get3DAttributes() const
{
    return( p3DView->Get3DAttributes( pScene ) );
}
/*************************************************************************
|* Svx3DPreviewControl::Set3DAttributes
\************************************************************************/
void Svx3DPreviewControl::Set3DAttributes( const SfxItemSet& rAttr )
{
    p3DView->Set3DAttributes( rAttr, pScene, TRUE );
    Resize();
}

/*************************************************************************
|* Svx3DPreviewControl::SetObjectType
\************************************************************************/
void Svx3DPreviewControl::Set3DObject( const E3dObject* pObj )
{
    if( pObj->ISA( E3dCompoundObject ) )
    {
        pScene->Remove3DObj( p3DObj );
        delete p3DObj;
        p3DObj = (E3dCompoundObject*)pObj->Clone();
        pScene->Insert3DObj( p3DObj );
        Resize();
    }
    else if( pObj->ISA( E3dPolyScene ) )
    {
        SdrObject* pObject = pFmPage->RemoveObject( pScene->GetOrdNum() );
        SdrObject::Free( pObject );
        p3DObj = NULL;
        pScene = (E3dPolyScene*)pObj->Clone();
        pFmPage->InsertObject( pScene );
        Resize();
    }
}

/*************************************************************************
|*
|*  3D Preview Control
|*
\************************************************************************/

SvxPreviewCtl3D::SvxPreviewCtl3D( Window* pParent, const ResId& rResId)
:   Control( pParent, rResId )
{
    // Members initialisieren
    Init();
}

SvxPreviewCtl3D::SvxPreviewCtl3D( Window* pParent, WinBits nStyle)
:   Control( pParent, nStyle)
{
    // Members initialisieren
    Init();
}

void SvxPreviewCtl3D::Init()
{
    // Members mit Defaults fuellen
    bGeometryCube=FALSE;
    fRotateX=-20.0;
    fRotateY=45.0;
    fRotateZ=0.0;
    fDistance=10.0;
    fDeviceSize=1.5;

    // MapMode waehlen
    SetMapMode( MAP_100TH_MM );

    // Hintergrund in einem schoenen neutralen Grau
//  SetBackground( Wallpaper( Color( COL_GRAY ) ) );

    // Segmente
    nHorSegs = 24;
    nVerSegs = 12;

    // Normalenmodus
    nNormalMode = PREVIEW_NORMAL_MODE_OBJECT;

    // ShadeMode
    nShadeMode = PREVIEW_SHADEMODE_GOURAUD;

    // Geometrie erzeugen
    CreateGeometry();

    // Material initialisieren
    Color aColWhite(COL_WHITE);
    Color aColBlack(COL_BLACK);

    aObjectMaterial.SetMaterial(aColWhite, Base3DMaterialAmbient);
    aObjectMaterial.SetMaterial(aColWhite, Base3DMaterialDiffuse);
    aObjectMaterial.SetMaterial(aColWhite, Base3DMaterialSpecular);
    aObjectMaterial.SetMaterial(aColBlack, Base3DMaterialEmission);
    aObjectMaterial.SetShininess(32);
}

SvxPreviewCtl3D::~SvxPreviewCtl3D()
{
}

void SvxPreviewCtl3D::Paint( const Rectangle& )
{
    // Base3D anfordern
    Base3D* pBase3D = Base3D::Create(this, nShadeMode == PREVIEW_SHADEMODE_DRAFT);

    Rectangle aVisible(Point(0,0), GetOutputSizePixel());
    aVisible = PixelToLogic(aVisible);

    // Orientierung
    basegfx::B3DHomMatrix mOrient;
    aCameraSet.SetObjectTrans(mOrient);
    aCameraSet.SetOrientation(
        basegfx::B3DPoint(0.0, 0.0, fDistance),
        basegfx::B3DVector(0.0, 0.0, 1.0),
        basegfx::B3DVector(0.0, 1.0, 0.0));
//  aCameraSet.SetOrientation(mOrient);

    // Matritzen setzen
    pBase3D->SetTransformationSet(&aCameraSet);

    // Licht setzen
    pBase3D->SetLightGroup(&aLights);

    // ShadeMode setzen
    if(nShadeMode == PREVIEW_SHADEMODE_FLAT || nShadeMode == PREVIEW_SHADEMODE_DRAFT)
        pBase3D->SetShadeModel(Base3DFlat);
    else if(nShadeMode == PREVIEW_SHADEMODE_GOURAUD)
        pBase3D->SetShadeModel(Base3DSmooth);
    else
        pBase3D->SetShadeModel(Base3DPhong);

    // Ausgaberechteck setzen
    aCameraSet.SetDeviceRectangle(-fDeviceSize, fDeviceSize, -fDeviceSize, fDeviceSize, FALSE);
    aCameraSet.SetFrontClippingPlane(fDistance - fDeviceSize);
    aCameraSet.SetBackClippingPlane(fDistance + fDeviceSize);
    aCameraSet.SetViewportRectangle(aVisible);

    // Matritzen setzen
    pBase3D->SetTransformationSet(&aCameraSet);

    // Werte fuer Objekt setzen
    pBase3D->SetActiveTexture();
    pBase3D->SetMaterial(aObjectMaterial.GetMaterial(Base3DMaterialAmbient), Base3DMaterialAmbient);
    pBase3D->SetMaterial(aObjectMaterial.GetMaterial(Base3DMaterialDiffuse), Base3DMaterialDiffuse);
    pBase3D->SetMaterial(aObjectMaterial.GetMaterial(Base3DMaterialSpecular), Base3DMaterialSpecular);
    pBase3D->SetMaterial(aObjectMaterial.GetMaterial(Base3DMaterialEmission), Base3DMaterialEmission);
    pBase3D->SetShininess(aObjectMaterial.GetShininess());

    pBase3D->SetRenderMode(Base3DRenderFill);
    pBase3D->SetCullMode(Base3DCullBack);

    // ScissorRegion defaultmaessig disablen
    pBase3D->ActivateScissorRegion(FALSE);

    // Nicht flach
    pBase3D->SetForceFlat(FALSE);

    // Geometrie ausgeben
    DrawGeometryClip(pBase3D);
}

void SvxPreviewCtl3D::DrawGeometryClip(Base3D *pBase3D)
{
    // spezielles Clipping fuer OpenGL, um keine floating windows ueberzumalen
    if(pBase3D->GetBase3DType() == BASE3D_TYPE_OPENGL
        && GetOutDevType() == OUTDEV_WINDOW
        && pBase3D->GetTransformationSet())
    {
        Window* pWin = (Window*)this;
        Region aClipRegion = pWin->GetActiveClipRegion();

        // ClipRegion ist gesetzt, benutze diese
        RegionHandle aRegionHandle = aClipRegion.BeginEnumRects();
        Rectangle aClipRect;

        while(aClipRegion.GetEnumRects(aRegionHandle, aClipRect))
        {
            if(aClipRect.IsOver(pBase3D->GetTransformationSet()->GetLogicalViewportBounds()))
            {
                // Viewport setzen
                pBase3D->SetScissorRegion(aClipRect, TRUE);

                // Zeichne alle Objekte
                pBase3D->StartScene();
                DrawGeometry(pBase3D);
                pBase3D->EndScene();
            }
        }
        aClipRegion.EndEnumRects(aRegionHandle);
    }
    else
    {
        // Zeichne alle Objekte
        pBase3D->StartScene();
        DrawGeometry(pBase3D);
        pBase3D->EndScene();
    }
}

void SvxPreviewCtl3D::DrawGeometry(Base3D *pBase3D)
{
    pBase3D->DrawPolygonGeometry(aGeometry);
}

void SvxPreviewCtl3D::SetGeometry(BOOL bGeomCube)
{
    if(bGeometryCube != bGeomCube)
    {
        bGeometryCube = bGeomCube;
        CreateGeometry();
    }
    Invalidate();
}

void SvxPreviewCtl3D::SetRotation(double fRotX, double fRotY, double fRotZ)
{
    if(fRotX != fRotateX || fRotY != fRotateY || fRotZ != fRotateZ)
    {
        fRotateX = fRotX;
        fRotateY = fRotY;
        fRotateZ = fRotZ;
        CreateGeometry();
    }
    Invalidate();
}

void SvxPreviewCtl3D::GetRotation(double& rRotX, double& rRotY, double& rRotZ)
{
    rRotX = fRotateX;
    rRotY = fRotateY;
    rRotZ = fRotateZ;
}

// Zugriffsfunktionen Materialien
void SvxPreviewCtl3D::SetMaterial(Color rNew, Base3DMaterialValue eVal)
{
    if(aObjectMaterial.GetMaterial(eVal) != rNew)
    {
        aObjectMaterial.SetMaterial(rNew, eVal);
        Invalidate();
    }
}

Color SvxPreviewCtl3D::GetMaterial(Base3DMaterialValue eVal)
{
    return aObjectMaterial.GetMaterial(eVal);
}

void SvxPreviewCtl3D::SetShininess(UINT16 nNew)
{
    if(aObjectMaterial.GetShininess() != nNew)
    {
        aObjectMaterial.SetShininess(nNew);
        Invalidate();
    }
}

UINT16 SvxPreviewCtl3D::GetShininess()
{
    return aObjectMaterial.GetShininess();
}

// Lichtquellen setzen
void SvxPreviewCtl3D::SetLightGroup(B3dLightGroup* pNew)
{
    if(pNew)
    {
        aLights = *pNew;
        Invalidate();
    }
}

// View-Einstellungen
void SvxPreviewCtl3D::SetUserDistance(double fNew)
{
    if(fNew != fDistance)
    {
        fDistance = fNew;
        Invalidate();
    }
}

void SvxPreviewCtl3D::SetDeviceSize(double fNew)
{
    if(fNew != fDeviceSize)
    {
        fDeviceSize = fNew;
        Invalidate();
    }
}

// Zugriffsfunktionen Segmentierung
void SvxPreviewCtl3D::SetHorizontalSegments(UINT16 nNew)
{
    if(nNew != nHorSegs)
    {
        nHorSegs = nNew;
        CreateGeometry();
        Invalidate();
    }
}

void SvxPreviewCtl3D::SetVerticalSegments(UINT16 nNew)
{
    if(nNew != nVerSegs)
    {
        nVerSegs = nNew;
        CreateGeometry();
        Invalidate();
    }
}

void SvxPreviewCtl3D::SetSegments(UINT16 nNewHor, UINT16 nNewVer)
{
    if(nNewHor != nHorSegs || nNewVer != nVerSegs)
    {
        nHorSegs = nNewHor;
        nVerSegs = nNewVer;
        CreateGeometry();
        Invalidate();
    }
}

// Zugriff Normalenmodus
void SvxPreviewCtl3D::SetNormalMode(UINT16 nNew)
{
    if(nNew != nNormalMode)
    {
        nNormalMode = nNew;
        CreateGeometry();
        Invalidate();
    }
}

// Zugriff auf ShadeMode
void SvxPreviewCtl3D::SetShadeMode(UINT16 nNew)
{
    if(nNew != nShadeMode)
    {
        nShadeMode = nNew;
        Invalidate();
    }
}

void SvxPreviewCtl3D::CreateGeometry()
{
    // Wuerfel erzeugen fuer Objektgroesse
    basegfx::B3DRange aVolume;
    aVolume.expand(basegfx::B3DPoint(-1.0, -1.0, -1.0));
    aVolume.expand(basegfx::B3DPoint( 1.0,  1.0,  1.0));

    if(bGeometryCube)
    {
        // Wuerfel erzeugen
        aGeometry.CreateCube(aVolume);
    }
    else
    {
        // AHCTUNG: Das PreviewControl hat bis zu dieser Stelle KEINE
        // Begrenzung in der Anzahl der Hor/Ver Segmente. Diese wird hier nun
        // explizit eingeschraenkt.
        double fHSegs = (nHorSegs > 50) ? 50.0 : (double)nHorSegs;
        double fVSegs = (nVerSegs > 50) ? 50.0 : (double)nVerSegs;

        // Kugel erzeugen
        aGeometry.CreateSphere(aVolume, fHSegs, fVSegs);
    }

    if(nNormalMode != PREVIEW_NORMAL_MODE_OBJECT)
    {
        if(!(nNormalMode == PREVIEW_NORMAL_MODE_FLAT))
        {
            aGeometry.CreateDefaultNormalsSphere();
        }
    }

    // Gesetzte Rotation ausfuehren
    if(fRotateX != 0.0 || fRotateY != 0.0 || fRotateZ != 0.0)
    {
        basegfx::B3DHomMatrix aRotMat;
        if(fRotateY != 0.0)
            aRotMat.rotate(0.0, fRotateY * F_PI180, 0.0);
        if(fRotateX != 0.0)
            aRotMat.rotate(-fRotateX * F_PI180, 0.0, 0.0);
        if(fRotateZ != 0.0)
            aRotMat.rotate(0.0, 0.0, fRotateZ * F_PI180);
        aGeometry.Transform(aRotMat);
    }
}

/*************************************************************************
|*
|*  3D Light Control
|*
\************************************************************************/

SvxLightPrevievCtl3D::SvxLightPrevievCtl3D( Window* pParent, const ResId& rResId)
:   SvxPreviewCtl3D(pParent, rResId)
{
    // Members initialisieren
    Init();
}

SvxLightPrevievCtl3D::SvxLightPrevievCtl3D( Window* pParent, WinBits nStyle )
:   SvxPreviewCtl3D(pParent, nStyle)
{
    // Members initialisieren
    Init();
}

void SvxLightPrevievCtl3D::Init()
{
    // Do never mirror the preview window.  This explicitly includes right
    // to left writing environments.
    EnableRTL (FALSE);

    // Lokale Parameter fuellen
    eSelectedLight = Base3DLightNone;
    fObjectRadius = 1.414;
    fDistanceToObject = 0.4;
    fScaleSizeSelected = 1.8;
    fLampSize = 0.1;
    nInteractionStartDistance = 5 * 5 * 2;
    bMouseMoved = FALSE;
    bGeometrySelected = FALSE;

    // Device groesser, da Lampen angezeigt werden
    SetDeviceSize(2.0);

    // Geometrie fuer Lampenobjekt erzeugen
    CreateLightGeometry();
}

SvxLightPrevievCtl3D::~SvxLightPrevievCtl3D()
{
}

void SvxLightPrevievCtl3D::SelectLight(Base3DLightNumber eNew)
{
    if(eNew != eSelectedLight)
    {
        eSelectedLight = eNew;
        bGeometrySelected = FALSE;
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::SelectGeometry()
{
    if(!bGeometrySelected)
    {
        bGeometrySelected = TRUE;
        eSelectedLight = Base3DLightNone;
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::SetObjectRadius(double fNew)
{
    if(fObjectRadius != fNew)
    {
        fObjectRadius = fNew;
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::SetDistanceToObject(double fNew)
{
    if(fDistanceToObject != fNew)
    {
        fDistanceToObject = fNew;
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::SetScaleSizeSelected(double fNew)
{
    if(fScaleSizeSelected != fNew)
    {
        fScaleSizeSelected = fNew;
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::SetLampSize(double fNew)
{
    if(fLampSize != fNew)
    {
        fLampSize = fNew;
        CreateLightGeometry();
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::DrawGeometry(Base3D *pBase3D)
{
    // call parent; zeichnet das Objekt selbst
    SvxPreviewCtl3D::DrawGeometry(pBase3D);

    // Lichter zeichnen
    for(UINT16 a=0;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
    {
        Base3DLightNumber eLightNum = (Base3DLightNumber)(Base3DLight0 + a);
        if(aLights.IsEnabled(eLightNum))
            DrawLightGeometry(eLightNum, pBase3D);
    }
}

void SvxLightPrevievCtl3D::DrawLightGeometry(Base3DLightNumber eLightNum,
    Base3D* pBase3D)
{
    // Geometrie bereitstellen
    B3dGeometry aNew;
    basegfx::B3DHomMatrix aTrans;
    double fRadius = fObjectRadius + fDistanceToObject;
    Color aLineColor(COL_YELLOW);
    aNew = aLightGeometry;

    if(eLightNum == eSelectedLight)
        aTrans.scale(fScaleSizeSelected, fScaleSizeSelected, fScaleSizeSelected);

    basegfx::B3DVector aDirection(aLights.GetDirection(eLightNum));
    aDirection.normalize();
    aDirection *= fRadius;
    aTrans.translate(aDirection.getX(), aDirection.getY(), aDirection.getZ());

    aNew.Transform(aTrans);

    // Material setzen
    Color aZwi;
    aZwi = aLights.GetIntensity(Base3DMaterialDiffuse, eLightNum);
    pBase3D->SetMaterial(aZwi, Base3DMaterialAmbient);
//  pBase3D->SetMaterial(aZwi, Base3DMaterialDiffuse);
    pBase3D->SetMaterial(aZwi, Base3DMaterialEmission);
    aZwi = aLights.GetIntensity(Base3DMaterialSpecular, eLightNum);
    pBase3D->SetMaterial(aZwi, Base3DMaterialSpecular);

    // Lampe Zeichnen
    pBase3D->SetRenderMode(Base3DRenderLine);
    pBase3D->DrawPolygonGeometry(aNew);

    if(eLightNum == eSelectedLight)
    {
        // Beleuchtung aus und Linienfarbe setzen
        BOOL bLightingWasEnabled = aLights.IsLightingEnabled();
        aLights.EnableLighting(FALSE);
        pBase3D->SetLightGroup(&aLights);
        pBase3D->SetLineWidth();

        // Kreis am Boden zeichnen
        basegfx::B3DPoint aPoint(0.0, -fRadius, fRadius);
        pBase3D->StartPrimitive(Base3DLineLoop);
        pBase3D->SetColor(aLineColor);

        double fWink;
        for(fWink=-F_PI;fWink < F_PI; fWink += F_2PI/24.0)
        {
            aPoint.setZ(-cos(fWink) * fRadius);
            aPoint.setX(-sin(fWink) * fRadius);
            pBase3D->AddVertex(aPoint);
        }
        pBase3D->EndPrimitive();

        // Kreisbogen zeichnen
        double fBodenWinkel = atan2(-aDirection.getX(), -aDirection.getZ());
        double fSinBoden = sin(fBodenWinkel) * fRadius;
        double fCosBoden = cos(fBodenWinkel) * fRadius;
        pBase3D->StartPrimitive(Base3DLineStrip);
        pBase3D->SetColor(aLineColor);

        for(fWink=-F_PI2;fWink < F_PI2; fWink += F_PI/12.0)
        {
            aPoint.setX(cos(fWink) * -fSinBoden);
            aPoint.setY(sin(fWink) * fRadius);
            aPoint.setZ(cos(fWink) * -fCosBoden);
            pBase3D->AddVertex(aPoint);
        }
        pBase3D->EndPrimitive();

        // Verbindung zeichnen
        pBase3D->StartPrimitive(Base3DLineStrip);
        pBase3D->SetColor(aLineColor);
        aPoint = basegfx::B3DPoint(0.0, -fRadius, 0.0);
        pBase3D->AddVertex(aPoint);
        aPoint.setX(-fSinBoden);
        aPoint.setZ(-fCosBoden);
        pBase3D->AddVertex(aPoint);
        aPoint.setY(0.0);
        pBase3D->AddVertex(aPoint);
        pBase3D->EndPrimitive();

        // Beleuchtung wieder eischalten
        aLights.EnableLighting(bLightingWasEnabled);
        pBase3D->SetLightGroup(&aLights);
    }
}

void SvxLightPrevievCtl3D::CreateLightGeometry()
{
    // Wuerfel erzeugen fuer Objektgroesse
    basegfx::B3DRange aVolume;
    aVolume.expand(basegfx::B3DPoint(-fLampSize, -fLampSize, -fLampSize));
    aVolume.expand(basegfx::B3DPoint( fLampSize,  fLampSize,  fLampSize));

    // Kugel erzeugen
    aLightGeometry.CreateSphere(aVolume, 4.0, 3.0);
}

// Selektion gueltig? D.h.: Lampe ist Selektiert un auch EINGESCHALTET
BOOL SvxLightPrevievCtl3D::IsSelectionValid()
{
    if((eSelectedLight != Base3DLightNone)
        && (aLights.GetLightObject(eSelectedLight).IsEnabled()))
    {
        return TRUE;
    }
    return FALSE;
}

// Selektierte Lampe Position in Polarkoordinaten holen/setzen
// dabei geht Hor:[0..360.0[ und Ver:[-90..90] Grad
void SvxLightPrevievCtl3D::GetPosition(double& rHor, double& rVer)
{
    if(IsSelectionValid())
    {
        basegfx::B3DVector aDirection(aLights.GetDirection(eSelectedLight));
        aDirection.normalize();
        rHor = atan2(-aDirection.getX(), -aDirection.getZ()) + F_PI; // 0..2PI
        rVer = atan2(aDirection.getY(), aDirection.getXZLength()); // -PI2..PI2
        rHor /= F_PI180; // 0..360.0
        rVer /= F_PI180; // -90.0..90.0
    }
    if(IsGeometrySelected())
    {
        rHor = fRotateY;
        rVer = fRotateX;
    }
}

void SvxLightPrevievCtl3D::SetPosition(double fHor, double fVer)
{
    if(IsSelectionValid())
    {
        basegfx::B3DVector aDirection;
        fHor = (fHor * F_PI180) - F_PI; // -PI..PI
        fVer *= F_PI180; // -PI2..PI2
        aDirection.setX(cos(fVer) * -sin(fHor));
        aDirection.setY(sin(fVer));
        aDirection.setZ(cos(fVer) * -cos(fHor));
        aDirection.normalize();
        aLights.SetDirection(aDirection, eSelectedLight);
        Invalidate();
    }
    if(IsGeometrySelected())
    {
        SetRotation(fVer, fHor, fRotateZ);
    }
}

// Interaktion
void SvxLightPrevievCtl3D::MouseButtonDown( const MouseEvent& rMEvt )
{
    BOOL bCallParent = TRUE;

    // Status switchen
    if(rMEvt.IsLeft())
    {
        if(IsSelectionValid() || bGeometrySelected)
        {
            bMouseMoved = FALSE;
            bCallParent = FALSE;
            aActionStartPoint = rMEvt.GetPosPixel();
            StartTracking();
        }
        else
        {
            // Einfacher Click ohne viel Bewegen, versuche eine
            // Selektion
            TrySelection(rMEvt.GetPosPixel());
            bCallParent = FALSE;
        }
    }

    // call parent
    if(bCallParent)
        SvxPreviewCtl3D::MouseButtonDown(rMEvt);
}

void SvxLightPrevievCtl3D::Tracking( const TrackingEvent& rTEvt )
{
    if(rTEvt.IsTrackingEnded())
    {
        if(rTEvt.IsTrackingCanceled())
        {
            if(bMouseMoved)
            {
                // Interaktion abbrechen
                bMouseMoved = FALSE;
                if(bGeometrySelected)
                {
                    SetRotation(fSaveActionStartVer, fSaveActionStartHor, fSaveActionStartRotZ);
                }
                else
                {
                    SetPosition(fSaveActionStartHor, fSaveActionStartVer);
                }
                if(aChangeCallback.IsSet())
                    aChangeCallback.Call(this);
            }
        }
        else
        {
            const MouseEvent& rMEvt = rTEvt.GetMouseEvent();
            if(bMouseMoved)
            {
                // Wurde interaktiv veraendert
            }
            else
            {
                // Einfacher Click ohne viel Bewegen, versuche eine
                // Selektion
                TrySelection(rMEvt.GetPosPixel());
            }
        }
    }
    else
    {
        const MouseEvent& rMEvt = rTEvt.GetMouseEvent();
        Point aDeltaPos = rMEvt.GetPosPixel() - aActionStartPoint;

        if(!bMouseMoved)
        {
            if(INT32(aDeltaPos.X() * aDeltaPos.X() + aDeltaPos.Y() * aDeltaPos.Y())
                > nInteractionStartDistance)
            {
                if(bGeometrySelected)
                {
                    GetRotation(fSaveActionStartVer, fSaveActionStartHor, fSaveActionStartRotZ);
                }
                else
                {
                    // Start der Interaktion, Werte Sichern
                    GetPosition(fSaveActionStartHor, fSaveActionStartVer);
                }
                bMouseMoved = TRUE;
            }
        }

        if(bMouseMoved)
        {
            if(bGeometrySelected)
            {
                double fNewRotX = fSaveActionStartVer - ((double)aDeltaPos.Y());
                double fNewRotY = fSaveActionStartHor + ((double)aDeltaPos.X());

                // Horizontal abgleichen
                while(fNewRotY < 0.0)
                    fNewRotY += 360.0;
                while(fNewRotY >= 360.0)
                    fNewRotY -= 360.0;

                // Vertikal cutten
                if(fNewRotX < -90.0)
                    fNewRotX = -90.0;
                if(fNewRotX > 90.0)
                    fNewRotX = 90.0;

                SetRotation(fNewRotX, fNewRotY, fSaveActionStartRotZ);

                if(aChangeCallback.IsSet())
                    aChangeCallback.Call(this);
            }
            else
            {
                // Interaktion im vollen Gange
                double fNewPosHor = fSaveActionStartHor + ((double)aDeltaPos.X());
                double fNewPosVer = fSaveActionStartVer - ((double)aDeltaPos.Y());

                // Horizontal abgleichen
                while(fNewPosHor < 0.0)
                    fNewPosHor += 360.0;
                while(fNewPosHor >= 360.0)
                    fNewPosHor -= 360.0;

                // Vertikal cutten
                if(fNewPosVer < -90.0)
                    fNewPosVer = -90.0;
                if(fNewPosVer > 90.0)
                    fNewPosVer = 90.0;

                SetPosition(fNewPosHor, fNewPosVer);

                if(aChangeCallback.IsSet())
                    aChangeCallback.Call(this);
            }
        }
    }
}

// Selektion einer Lampe
void SvxLightPrevievCtl3D::TrySelection(Point aPosPixel)
{
    BOOL bNewSelection(FALSE);
    Base3DLightNumber eNew = Base3DLightNone;

    for(UINT16 a=0;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
    {
        Base3DLightNumber eActualLight = (Base3DLightNumber)(Base3DLight0 + a);
        if(aLights.IsEnabled(eActualLight))
        {
            basegfx::B3DVector aLightPos(aLights.GetDirection(eActualLight));
            aLightPos.normalize();
            aLightPos *= GetObjectRadius() + GetDistanceToObject();
            basegfx::B3DPoint aScreenPos(aCameraSet.ObjectToViewCoor(aLightPos));
            Point aScreenPosPixel((long)(aScreenPos.getX() + 0.5), (long)(aScreenPos.getY() + 0.5));
            aScreenPosPixel = LogicToPixel(aScreenPosPixel);
            aScreenPosPixel -= aPosPixel;
            INT32 nDistance = (aScreenPosPixel.getX() * aScreenPosPixel.getX()) + (aScreenPosPixel.getY() * aScreenPosPixel.getY());

            if(nDistance < nInteractionStartDistance)
            {
                eNew = eActualLight;
                bNewSelection = TRUE;
            }
        }
    }

    if(bNewSelection && eSelectedLight != eNew)
    {
        // Auswaehlen
        SelectLight(eNew);

        // Falls tatsaechlich eine andere Lampe selektiert
        // wurde, rufe den entsprechenden Callback
        if(aSelectionChangeCallback.IsSet())
            aSelectionChangeCallback.Call(this);
    }
    else
    {
        // Punkt in logische Koordinaten umrechnen
        Point aPosLogic = PixelToLogic(aPosPixel);

        // Punkte generieren
        basegfx::B3DPoint aHitFront(aPosLogic.X(), aPosLogic.Y(), 0.0);
        basegfx::B3DPoint aHitBack(aPosLogic.X(), aPosLogic.Y(), ZBUFFER_DEPTH_RANGE);

        // Umrechnen
        aHitFront = aCameraSet.ViewToObjectCoor(aHitFront);
        aHitBack = aCameraSet.ViewToObjectCoor(aHitBack);

        // Eventuell die Geometrie des Beispielobjektes waehlen
        if(aGeometry.CheckHit(aHitFront, aHitBack, 0))
        {
            // Auswaehlen
            SelectGeometry();

            // Falls tatsaechlich eine andere Lampe selektiert
            // wurde, rufe den entsprechenden Callback
            if(aSelectionChangeCallback.IsSet())
                aSelectionChangeCallback.Call(this);
        }
    }
}

/*************************************************************************
|*
|*  3D Light Control Konstruktor
|*
\************************************************************************/

SvxLightCtl3D::SvxLightCtl3D( Window* pParent, const ResId& rResId)
:   Control(pParent, rResId),
    aLightControl(this, 0),
    aHorScroller(this, WB_HORZ | WB_DRAG),
    aVerScroller(this, WB_VERT | WB_DRAG),
    aSwitcher(this, 0)
{
    // Members initialisieren
    Init();
}

SvxLightCtl3D::SvxLightCtl3D( Window* pParent, WinBits nStyle )
:   Control(pParent, nStyle),
    aLightControl(this, 0),
    aHorScroller(this, WB_HORZ | WB_DRAG),
    aVerScroller(this, WB_VERT | WB_DRAG),
    aSwitcher(this, 0)
{
    // Members initialisieren
    Init();
}

void SvxLightCtl3D::Init()
{
    // #i58240# set HelpIDs for scrollbars and switcher
    aHorScroller.SetHelpId(HID_CTRL3D_HSCROLL);
    aVerScroller.SetHelpId(HID_CTRL3D_VSCROLL);
    aSwitcher.SetHelpId(HID_CTRL3D_SWITCHER);

    // Lokale Parameter setzen
    bSphereUsed = TRUE;
    bVectorValid = FALSE;

    // Light preview
    aLightControl.Show();
    aLightControl.SetChangeCallback( LINK(this, SvxLightCtl3D, InternalInteractiveChange) );
    aLightControl.SetSelectionChangeCallback( LINK(this, SvxLightCtl3D, InternalSelectionChange) );

    // Horiz Scrollbar
    aHorScroller.Show();
    aHorScroller.SetRange(Range(0, 36000));
    aHorScroller.SetLineSize(100);
    aHorScroller.SetPageSize(1000);
    aHorScroller.SetScrollHdl( LINK(this, SvxLightCtl3D, ScrollBarMove) );

    // Vert Scrollbar
    aVerScroller.Show();
    aVerScroller.SetRange(Range(0, 18000));
    aVerScroller.SetLineSize(100);
    aVerScroller.SetPageSize(1000);
    aVerScroller.SetScrollHdl( LINK(this, SvxLightCtl3D, ScrollBarMove) );

    // Switch Button
    aSwitcher.Show();
    aSwitcher.SetClickHdl( LINK(this, SvxLightCtl3D, ButtonPress) );

    // Selektion klaeren
    CheckSelection();

    // Neues Layout
    NewLayout();
}

SvxLightCtl3D::~SvxLightCtl3D()
{
}

void SvxLightCtl3D::SetVector(const basegfx::B3DVector& rNew)
{
    aVector = rNew;
    aVector.normalize();
    bVectorValid = TRUE;
}

const basegfx::B3DVector& SvxLightCtl3D::GetVector()
{
    // Grobe Anbindung an altes Verhalten, um eine Reaktion zu haben
    aVector = aLightControl.GetLightGroup()->GetDirection(aLightControl.GetSelectedLight());
    aVector.normalize();
    return aVector;
}

void SvxLightCtl3D::Resize()
{
    // call parent
    Control::Resize();

    // Neues Layout
    NewLayout();
}

void SvxLightCtl3D::NewLayout()
{
    // Layout members
    Size aSize = GetOutputSizePixel();
    long nScrollSize = aHorScroller.GetSizePixel().Height();

    // Preview Fenster
    Point aPoint(0, 0);
    Size aDestSize(aSize.Width() - nScrollSize, aSize.Height() - nScrollSize);
    aLightControl.SetPosSizePixel(aPoint, aDestSize);

    // Horizontaler Scrollbar
    aPoint.Y() = aSize.Height() - nScrollSize;
    aDestSize.Height() = nScrollSize;
    aHorScroller.SetPosSizePixel(aPoint, aDestSize);

    // Vertikaler Scrollbar
    aPoint.X() = aSize.Width() - nScrollSize;
    aPoint.Y() = 0;
    aDestSize.Width() = nScrollSize;
    aDestSize.Height() = aSize.Height() - nScrollSize;
    aVerScroller.SetPosSizePixel(aPoint, aDestSize);

    // Button
    aPoint.Y() = aSize.Height() - nScrollSize;
    aDestSize.Height() = nScrollSize;
    aSwitcher.SetPosSizePixel(aPoint, aDestSize);
}

// Selektion auf Gueltigkeit pruefen
void SvxLightCtl3D::CheckSelection()
{
    BOOL bSelectionValid = (aLightControl.IsSelectionValid()
        || aLightControl.IsGeometrySelected());
    aHorScroller.Enable(bSelectionValid);
    aVerScroller.Enable(bSelectionValid);

    if(bSelectionValid)
    {
        double fHor, fVer;
        aLightControl.GetPosition(fHor, fVer);
        aHorScroller.SetThumbPos( INT32(fHor * 100.0) );
        aVerScroller.SetThumbPos( 18000 - INT32((fVer + 90.0) * 100.0) );
    }
}

void SvxLightCtl3D::move( double fDeltaHor, double fDeltaVer )
{
    double fHor, fVer;

    aLightControl.GetPosition(fHor, fVer);

    fHor += fDeltaHor;
    fVer += fDeltaVer;

    if( fVer > 90.0 )
        return;

    if ( fVer < -90.0 )
        return;

    aLightControl.SetPosition(fHor, fVer);
    aHorScroller.SetThumbPos( INT32(fHor * 100.0) );
    aVerScroller.SetThumbPos( 18000 - INT32((fVer + 90.0) * 100.0) );

    if(aUserInteractiveChangeCallback.IsSet())
        aUserInteractiveChangeCallback.Call(this);

}

void SvxLightCtl3D::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode = rKEvt.GetKeyCode();

    if( aCode.GetModifier() )
    {
        Control::KeyInput( rKEvt );
        return;
    }

    switch ( aCode.GetCode() )
    {
        case KEY_SPACE:
            ;
            break;
        case KEY_LEFT:
            move(  -4.0,  0.0 ); // #i58242# changed move direction in X
            break;
        case KEY_RIGHT:
            move( 4.0,  0.0 ); // #i58242# changed move direction in X
            break;
        case KEY_UP:
            move(  0.0,  4.0 );
            break;
        case KEY_DOWN:
            move(  0.0, -4.0 );
            break;
        case KEY_PAGEUP:
            {
                B3dLightGroup* pLights = aLightControl.GetLightGroup();
                int eLight = aLightControl.GetSelectedLight() - 1;

                while( (eLight >= Base3DLight0) && !pLights->IsEnabled((Base3DLightNumber)eLight) )
                    eLight--;

                if( eLight < Base3DLight0 )
                {
                    eLight =  Base3DLight7;
                    while( (eLight >= Base3DLight0) && !pLights->IsEnabled((Base3DLightNumber)eLight) )
                        eLight--;
                }

                if( eLight >= Base3DLight0 )
                {
                    aLightControl.SelectLight((Base3DLightNumber)eLight);
                    CheckSelection();
                    if(aUserSelectionChangeCallback.IsSet())
                        aUserSelectionChangeCallback.Call(this);
                }
                break;
            }
        case KEY_PAGEDOWN:
            {
                B3dLightGroup* pLights = aLightControl.GetLightGroup();
                int eLight = aLightControl.GetSelectedLight() + 1;

                while( (eLight < Base3DLightNone) && !pLights->IsEnabled((Base3DLightNumber)eLight) )
                    eLight++;

                if( eLight == Base3DLightNone )
                {
                    eLight = Base3DLight0;
                    while( (eLight < Base3DLightNone) && !pLights->IsEnabled((Base3DLightNumber)eLight) )
                        eLight++;
                }

                if( eLight < Base3DLightNone )
                {
                    aLightControl.SelectLight((Base3DLightNumber)eLight);
                    CheckSelection();
                    if(aUserSelectionChangeCallback.IsSet())
                        aUserSelectionChangeCallback.Call(this);
                }
                break;
            }
        default:
            Control::KeyInput( rKEvt );
            break;
    }
}

void SvxLightCtl3D::GetFocus()
{
    Control::GetFocus();

    if( HasFocus() && IsEnabled() )
    {
        CheckSelection();

        Size aFocusSize = aLightControl.GetOutputSizePixel();

        aFocusSize.Width() -= 4;
        aFocusSize.Height() -= 4;

        Rectangle aFocusRect( Point( 2, 2 ), aFocusSize );

        aFocusRect = aLightControl.PixelToLogic( aFocusRect );

        aLightControl.ShowFocus( aFocusRect );
    }
}

void SvxLightCtl3D::LoseFocus()
{
    Control::LoseFocus();

    aLightControl.HideFocus();
}

IMPL_LINK( SvxLightCtl3D, ScrollBarMove, void*, EMPTYARG)
{
    INT32 nHor = aHorScroller.GetThumbPos();
    INT32 nVer = aVerScroller.GetThumbPos();

    aLightControl.SetPosition(
        ((double)nHor) / 100.0,
        ((double)((18000 - nVer) - 9000)) / 100.0);

    if(aUserInteractiveChangeCallback.IsSet())
        aUserInteractiveChangeCallback.Call(this);

    // ...um Kompatibel zu bleiben, kann spaeter wieder raus
    //InteractiveChange(NULL);

    return 0;
}

IMPL_LINK( SvxLightCtl3D, ButtonPress, void*, EMPTYARG)
{
    aLightControl.SetGeometry(bSphereUsed);
    bSphereUsed = !bSphereUsed;
    return 0;
}

IMPL_LINK( SvxLightCtl3D, InternalInteractiveChange, void*, EMPTYARG)
{
    double fHor, fVer;

    aLightControl.GetPosition(fHor, fVer);
    aHorScroller.SetThumbPos( INT32(fHor * 100.0) );
    aVerScroller.SetThumbPos( 18000 - INT32((fVer + 90.0) * 100.0) );

    if(aUserInteractiveChangeCallback.IsSet())
        aUserInteractiveChangeCallback.Call(this);

    // ...um Kompatibel zu bleiben, kann spaeter wieder raus
    //InteractiveChange(NULL);

    return 0;
}

IMPL_LINK( SvxLightCtl3D, InternalSelectionChange, void*, EMPTYARG)
{
    CheckSelection();

    if(aUserSelectionChangeCallback.IsSet())
        aUserSelectionChangeCallback.Call(this);

    // ...um Kompatibel zu bleiben, kann spaeter wieder raus
    //SelectionChange(NULL);

    return 0;
}

// ...um Kompatibel zu bleiben, kann spaeter wieder raus
/*
IMPL_LINK( SvxLightCtl3D, InteractiveChange, void*, EMPTYARG)
{
    return NULL;
} */

/*
IMPL_LINK( SvxLightCtl3D, SelectionChange, void*, EMPTYARG)
{
    return NULL;
}*/


