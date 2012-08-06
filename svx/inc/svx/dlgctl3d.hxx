/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
class E3dPolyScene;
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
    E3dPolyScene*           mpScene;
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
    unsigned                    mbMouseMoved : 1;
    unsigned                    mbGeometrySelected : 1;

    void Construct2();
    void ConstructLightObjects();
    void AdaptToSelectedLight();
    void TrySelection(Point aPosPixel);

public:
    Svx3DLightControl(Window* pParent, WinBits nStyle = 0);
    ~Svx3DLightControl();

    virtual void Paint(const Rectangle& rRect);
    virtual void MouseButtonDown(const MouseEvent& rMEvt);
    virtual void Tracking( const TrackingEvent& rTEvt );
    virtual void Resize();

    virtual void SetObjectType(sal_uInt16 nType);

    // register user callback
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

    // callback for interactive changes
    Link                    maUserInteractiveChangeCallback;
    Link                    maUserSelectionChangeCallback;

public:
    SvxLightCtl3D( Window* pParent, const ResId& rResId);
    ~SvxLightCtl3D();

    // react to size changes
    virtual void Resize();
    void NewLayout();

    // check the selection for validity
    void CheckSelection();

    // bring further settings to the outside world
    Svx3DLightControl& GetSvx3DLightControl() { return maLightControl; }

    // register user callback
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

    // initialize local parameters
    void Init();

    void move( double fDeltaHor, double fDeltaVer );
};

#endif // _SCH_DLGCTL3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
