/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlgctl3d.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SVX_DLGCTL3D_HXX
#define _SVX_DLGCTL3D_HXX

// includes --------------------------------------------------------------

#include <tools/poly.hxx>
#include <vcl/ctrl.hxx>
#include <goodies/b3dgeom.hxx>
#include <goodies/b3dtrans.hxx>
#include <goodies/matril3d.hxx>
#include <goodies/b3dlight.hxx>
#include <vcl/scrbar.hxx>

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <svtools/itemset.hxx>

class FmFormModel;
class FmFormPage;
class E3dView;
class E3dPolyScene;
class E3dObject;
class Base3D;

/*************************************************************************
|*
|*  Control zur Darstellung einer 3D-Scene
|*
\************************************************************************/

#define PREVIEW_OBJECTTYPE_SPHERE           0x0000
#define PREVIEW_OBJECTTYPE_CUBE             0x0001

class Svx3DPreviewControl : public Control
{
protected:
    FmFormModel*        pModel;
    FmFormPage*         pFmPage;
    E3dView*            p3DView;
    E3dPolyScene*       pScene;
    E3dObject*          p3DObj;
    UINT16              nObjectType;

    void                Construct();

public:
                    Svx3DPreviewControl( Window* pParent, const ResId& rResId );
                    Svx3DPreviewControl( Window* pParent, WinBits nStyle = 0 );
                    ~Svx3DPreviewControl();

    virtual void    Paint( const Rectangle& rRect );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Resize();

    void            Reset();

    void            SetObjectType( UINT16 nType );
    UINT16          GetObjectType() const { return( nObjectType ); }

    SfxItemSet      Get3DAttributes() const;
    void            Set3DAttributes( const SfxItemSet& rAttr );

    void            Set3DObject( const E3dObject* pObj );
};

/*************************************************************************
|*
|*  3D Preview Control
|*
\************************************************************************/

// Defines fuer NormalMode
#define PREVIEW_NORMAL_MODE_OBJECT          0x0000
#define PREVIEW_NORMAL_MODE_FLAT            0x0001
#define PREVIEW_NORMAL_MODE_SPHERE          0x0002

// Defines fuer ShadeMode
#define PREVIEW_SHADEMODE_FLAT              0x0000
#define PREVIEW_SHADEMODE_PHONG             0x0001
#define PREVIEW_SHADEMODE_GOURAUD           0x0002
#define PREVIEW_SHADEMODE_DRAFT             0x0003

class SvxPreviewCtl3D : public Control
{
protected:
    // Geometrie des Objektes
    B3dGeometry             aGeometry;

    // Kameraset
    B3dCamera               aCameraSet;
    double                  fDistance;
    double                  fDeviceSize;

    // Rotation der Geometrie (bei Cube)
    double                  fRotateX;
    double                  fRotateY;
    double                  fRotateZ;

    // Farben des Objektes
    B3dMaterial             aObjectMaterial;

    // Lichtquellen
    B3dLightGroup           aLights;

    // Segmentierung, wird bei Kugel verwendet
    UINT16                  nHorSegs;
    UINT16                  nVerSegs;

    // Modus fuer Normalen
    UINT16                  nNormalMode;

    // Zeichenmodus
    UINT16                  nShadeMode;

    // Art der Geometrie, Cube oder Sphere
    BOOL                    bGeometryCube;

public:
    SvxPreviewCtl3D( Window* pParent, const ResId& rResId);
    SvxPreviewCtl3D( Window* pParent, WinBits nStyle = 0);
    ~SvxPreviewCtl3D();

    // Zeichenmethode
    virtual void Paint( const Rectangle& rRect );
    void DrawGeometryClip(Base3D* pBase3D);
    virtual void DrawGeometry(Base3D* pBase3D);

    // Art der Geometrie setzen
    void SetGeometry(BOOL bGeomCube);

    // Rotation setzen
    void SetRotation(double fRotX, double fRotY, double fRotZ);
    void GetRotation(double& rRotX, double& rRotY, double& rRotZ);

    // Zugriffsfunktionen Materialien
    void SetMaterial(Color rNew,
        Base3DMaterialValue=Base3DMaterialAmbient);
    Color GetMaterial(Base3DMaterialValue=Base3DMaterialAmbient);
    void SetShininess(UINT16 nNew);
    UINT16 GetShininess();

    // Lichtquellen setzen
    void SetLightGroup(B3dLightGroup* pNew=0L);
    B3dLightGroup* GetLightGroup() { return &aLights; }

    // View-Einstellungen
    void SetUserDistance(double fNew);
    double GetUserDistance() { return fDistance; }
    void SetDeviceSize(double fNew);
    double GetDeviceSize() { return fDeviceSize; }

    // Zugriffsfunktionen Segmentierung
    UINT16 GetHorizontalSegments() { return nHorSegs; }
    UINT16 GetVerticalSegments() { return nVerSegs; }
    void SetHorizontalSegments(UINT16 nNew);
    void SetVerticalSegments(UINT16 nNew);
    void SetSegments(UINT16 nNewHor, UINT16 nNewVer);

    // Zugriff Normalenmodus
    UINT16 GetNormalMode() { return nNormalMode; }
    void SetNormalMode(UINT16 nNew);

    // Zugriff auf ShadeMode
    UINT16 GetShadeMode() { return nShadeMode; }
    void SetShadeMode(UINT16 nNew);

protected:

    // Geometrieerzeugung
    void CreateGeometry();

    // Lokale Parameter Initialisieren
    void Init();
};

/*************************************************************************
|*
|*  3D Light Preview Control
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxLightPrevievCtl3D : public SvxPreviewCtl3D
{
private:
    // Geometrie eines Lichtobjektes
    B3dGeometry         aLightGeometry;

    Base3DLightNumber   eSelectedLight;

    // Werte fuer Rendering
    double              fObjectRadius;
    double              fDistanceToObject;
    double              fScaleSizeSelected;
    double              fLampSize;

    // Callback bei interaktiven Aenderungen
    Link                aChangeCallback;
    Link                aSelectionChangeCallback;

    // Sichern der Interaktion
    double              fSaveActionStartHor;
    double              fSaveActionStartVer;
    double              fSaveActionStartRotZ;
    Point               aActionStartPoint;

    // Mindestentfernung fuer Interaktionsstart
    INT32               nInteractionStartDistance;

    // Maus-Status
    unsigned            bMouseMoved                     : 1;
    unsigned            bGeometrySelected               : 1;

public:
    SvxLightPrevievCtl3D( Window* pParent, const ResId& rResId);
    SvxLightPrevievCtl3D( Window* pParent, WinBits nStyle = 0);
    ~SvxLightPrevievCtl3D();

    void SelectLight(Base3DLightNumber=Base3DLightNone);
    Base3DLightNumber GetSelectedLight() { return eSelectedLight; }

    void SelectGeometry();
    BOOL IsGeometrySelected() { return bGeometrySelected; }

    void SetObjectRadius(double fNew);
    double GetObjectRadius() { return fObjectRadius; }

    void SetDistanceToObject(double fNew);
    double GetDistanceToObject() { return fDistanceToObject; }

    void SetScaleSizeSelected(double fNew);
    double GetScaleSizeSelected() { return fScaleSizeSelected; }

    void SetLampSize(double fNew);
    double GetLampSize() { return fLampSize; }

    // Zeichenmethode
    virtual void DrawGeometry(Base3D* pBase3D);
    void DrawLightGeometry(Base3DLightNumber eLightNum, Base3D* pBase3D);

    // Selektion gueltig
    BOOL IsSelectionValid();

    // Selektierte Lampe Position in Polarkoordinaten holen/setzen
    // dabei geht Hor:[0..360.0[ und Ver:[-90..90] Grad
    void GetPosition(double& rHor, double& rVer);
    void SetPosition(double fHor, double fVer);

    // Callback eintragen
    void SetChangeCallback(Link aNew) { aChangeCallback = aNew; }
    void SetSelectionChangeCallback(Link aNew) { aSelectionChangeCallback = aNew; }

    // Interaktion
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        Tracking( const TrackingEvent& rTEvt );

protected:

    // Geometrieerzeugung Lampe
    void CreateLightGeometry();

    // Selektion einer Lampe
    void TrySelection(Point aPosPixel);

    // Lokale Parameter Initialisieren
    void Init();
};

/*************************************************************************
|*
|*  3D Light Control
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxLightCtl3D : public Control
{
private:
    // Lokale Controls
    SvxLightPrevievCtl3D    aLightControl;
    ScrollBar               aHorScroller;
    ScrollBar               aVerScroller;
    PushButton              aSwitcher;
    basegfx::B3DVector  aVector;

// Callback bei interaktiven Aenderungen
    Link                    aUserInteractiveChangeCallback;
    Link                    aUserSelectionChangeCallback;

    // Flags
    unsigned                bVectorValid                    : 1;
    unsigned                bSphereUsed                     : 1;

public:
    SvxLightCtl3D( Window* pParent, const ResId& rResId);
    SvxLightCtl3D( Window* pParent, WinBits nStyle = 0);
    ~SvxLightCtl3D();

    // Altes Interface
    void SetVector(const basegfx::B3DVector& rNew);
    const basegfx::B3DVector& GetVector();
    BOOL GetVectorValid() { return bVectorValid; }

    // Reagiere auf Groessenaenderungen
    virtual void        Resize();
    void NewLayout();

    // Selektion auf Gueltigkeit pruefen
    void CheckSelection();

    // Um weitere Einstellungen nach Aussen zu bringen...
    SvxLightPrevievCtl3D& GetPreviewControl() { return aLightControl; }

    // User Callback eintragen
    void SetUserInteractiveChangeCallback(Link aNew)
        { aUserInteractiveChangeCallback = aNew; }
    void SetUserSelectionChangeCallback(Link aNew)
        { aUserSelectionChangeCallback = aNew; }

    virtual void KeyInput( const KeyEvent& rKEvt );
    virtual void GetFocus();
    virtual void LoseFocus();

protected:

    DECL_LINK( InternalInteractiveChange, void*);
    DECL_LINK( InternalSelectionChange, void*);
    DECL_LINK( ScrollBarMove, void*);
    DECL_LINK( ButtonPress, void*);

    // Lokale Parameter Initialisieren
    void Init();

    void move( double fDeltaHor, double fDeltaVer );
};


#endif // _SCH_DLGCTL3D_HXX

