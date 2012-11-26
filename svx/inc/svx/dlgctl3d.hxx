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



#ifndef _SVX_DLGCTL3D_HXX
#define _SVX_DLGCTL3D_HXX

#include <vcl/ctrl.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/button.hxx>
#include <svl/itemset.hxx>
#include "svx/svxdllapi.h"
#include <basegfx/vector/b3dvector.hxx>

//////////////////////////////////////////////////////////////////////////////

class FmFormModel;
class FmFormPage;
class E3dView;
class E3dScene;
class E3dObject;

//////////////////////////////////////////////////////////////////////////////

#define PREVIEW_OBJECTTYPE_SPHERE           0x0000
#define PREVIEW_OBJECTTYPE_CUBE             0x0001

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC Svx3DPreviewControl : public Control
{
protected:
    FmFormModel*            mpModel;
    FmFormPage*             mpFmPage;
    E3dView*                mp3DView;
    E3dScene*               mpScene;
    E3dObject*              mp3DObj;
    sal_uInt16              mnObjectType;

    void Construct();

public:
    Svx3DPreviewControl(Window* pParent, const ResId& rResId);
    Svx3DPreviewControl(Window* pParent, WinBits nStyle = 0);
    ~Svx3DPreviewControl();

    virtual void Paint( const Rectangle& rRect );
    virtual void MouseButtonDown( const MouseEvent& rMEvt );
    virtual void Resize();

    void Reset();
    virtual void SetObjectType(sal_uInt16 nType);
    sal_uInt16 GetObjectType() const { return( mnObjectType ); }
    SfxItemSet Get3DAttributes() const;
    virtual void Set3DAttributes(const SfxItemSet& rAttr);
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC Svx3DLightControl : public Svx3DPreviewControl
{
    // Callback for interactive changes
    Link                        maUserInteractiveChangeCallback;
    Link                        maUserSelectionChangeCallback;
    Link                        maChangeCallback;
    Link                        maSelectionChangeCallback;

    // lights
    sal_uInt32                  maSelectedLight;

    // extra objects for light control
    E3dObject*                  mpExpansionObject;
    E3dObject*                  mpLampBottomObject;
    E3dObject*                  mpLampShaftObject;
    std::vector< E3dObject* >   maLightObjects;

    // 3d rotations of object
    double                      mfRotateX;
    double                      mfRotateY;
    double                      mfRotateZ;

    // interaction parameters
    Point                       maActionStartPoint;
    sal_Int32                   mnInteractionStartDistance;
    double                      mfSaveActionStartHor;
    double                      mfSaveActionStartVer;
    double                      mfSaveActionStartRotZ;

    // bitfield
    bool                        mbMouseMoved : 1;
    bool                        mbGeometrySelected : 1;

    void Construct2();
    void ConstructLightObjects();
    void AdaptToSelectedLight();
    void TrySelection(Point aPosPixel);

public:
    Svx3DLightControl(Window* pParent, const ResId& rResId);
    Svx3DLightControl(Window* pParent, WinBits nStyle = 0);
    ~Svx3DLightControl();

    virtual void Paint(const Rectangle& rRect);
    virtual void MouseButtonDown(const MouseEvent& rMEvt);
    virtual void Tracking( const TrackingEvent& rTEvt );
    virtual void Resize();

    virtual void SetObjectType(sal_uInt16 nType);

    // User Callback eintragen
    void SetUserInteractiveChangeCallback(Link aNew) { maUserInteractiveChangeCallback = aNew; }
    void SetUserSelectionChangeCallback(Link aNew) { maUserSelectionChangeCallback = aNew; }
    void SetChangeCallback(Link aNew) { maChangeCallback = aNew; }
    void SetSelectionChangeCallback(Link aNew) { maSelectionChangeCallback = aNew; }

    // selection checks
    bool IsSelectionValid();
    bool IsGeometrySelected() { return mbGeometrySelected; }

    // get/set position of selected lamp in polar coordinates, Hor:[0..360.0[ and Ver:[-90..90] degrees
    void GetPosition(double& rHor, double& rVer);
    void SetPosition(double fHor, double fVer);

    // get/set rotation of 3D object
    void SetRotation(double fRotX, double fRotY, double fRotZ);
    void GetRotation(double& rRotX, double& rRotY, double& rRotZ);

    void SelectLight(sal_uInt32 nLightNumber);
    virtual void Set3DAttributes(const SfxItemSet& rAttr);
    sal_uInt32 GetSelectedLight() { return maSelectedLight; }

    // light data access
    bool GetLightOnOff(sal_uInt32 nNum) const;
    Color GetLightColor(sal_uInt32 nNum) const;
    basegfx::B3DVector GetLightDirection(sal_uInt32 nNum) const;
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SvxLightCtl3D : public Control
{
private:
    // local controls
    Svx3DLightControl       maLightControl;
    ScrollBar               maHorScroller;
    ScrollBar               maVerScroller;
    PushButton              maSwitcher;

    // Callback bei interaktiven Aenderungen
    Link                    maUserInteractiveChangeCallback;
    Link                    maUserSelectionChangeCallback;

public:
    SvxLightCtl3D( Window* pParent, const ResId& rResId);
    SvxLightCtl3D( Window* pParent, WinBits nStyle = 0);
    ~SvxLightCtl3D();

    // Reagiere auf Groessenaenderungen
    virtual void Resize();
    void NewLayout();

    // Selektion auf Gueltigkeit pruefen
    void CheckSelection();

    // Um weitere Einstellungen nach Aussen zu bringen...
    Svx3DLightControl& GetSvx3DLightControl() { return maLightControl; }

    // User Callback eintragen
    void SetUserInteractiveChangeCallback(Link aNew) { maUserInteractiveChangeCallback = aNew; }
    void SetUserSelectionChangeCallback(Link aNew) { maUserSelectionChangeCallback = aNew; }

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

//////////////////////////////////////////////////////////////////////////////
// eof
