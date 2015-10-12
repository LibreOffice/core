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

#ifndef INCLUDED_SVX_DLGCTL3D_HXX
#define INCLUDED_SVX_DLGCTL3D_HXX

#include <vcl/ctrl.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/button.hxx>
#include <svl/itemset.hxx>
#include <svx/svxdllapi.h>
#include <basegfx/vector/b3dvector.hxx>

class FmFormModel;
class FmFormPage;
class E3dView;
class E3dPolyScene;
class E3dObject;

enum class SvxPreviewObjectType { SPHERE, CUBE };

class SVX_DLLPUBLIC SAL_WARN_UNUSED Svx3DPreviewControl : public Control
{
protected:
    FmFormModel*            mpModel;
    FmFormPage*             mpFmPage;
    E3dView*                mp3DView;
    E3dPolyScene*           mpScene;
    E3dObject*              mp3DObj;
    SvxPreviewObjectType    mnObjectType;

    void Construct();

public:
    Svx3DPreviewControl(vcl::Window* pParent, WinBits nStyle = 0);
    virtual ~Svx3DPreviewControl();
    virtual void dispose() override;

    virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;

    virtual void SetObjectType(SvxPreviewObjectType nType);
    SvxPreviewObjectType GetObjectType() const { return mnObjectType; }
    SfxItemSet Get3DAttributes() const;
    virtual void Set3DAttributes(const SfxItemSet& rAttr);
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED Svx3DLightControl : public Svx3DPreviewControl
{
    // Callback for interactive changes
    Link<Svx3DLightControl*,void>  maChangeCallback;
    Link<Svx3DLightControl*,void>  maSelectionChangeCallback;

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
    Svx3DLightControl(vcl::Window* pParent, WinBits nStyle = 0);

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void Tracking( const TrackingEvent& rTEvt ) override;
    virtual void Resize() override;

    virtual void SetObjectType(SvxPreviewObjectType nType) override;

    // register user callback
    void SetChangeCallback(Link<Svx3DLightControl*,void> aNew) { maChangeCallback = aNew; }
    void SetSelectionChangeCallback(Link<Svx3DLightControl*,void> aNew) { maSelectionChangeCallback = aNew; }

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
    virtual void Set3DAttributes(const SfxItemSet& rAttr) override;
    sal_uInt32 GetSelectedLight() { return maSelectedLight; }

    // light data access
    bool GetLightOnOff(sal_uInt32 nNum) const;
    Color GetLightColor(sal_uInt32 nNum) const;
    basegfx::B3DVector GetLightDirection(sal_uInt32 nNum) const;
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxLightCtl3D : public Control
{
private:
    // local controls
    VclPtr<Svx3DLightControl>  maLightControl;
    VclPtr<ScrollBar>          maHorScroller;
    VclPtr<ScrollBar>          maVerScroller;
    VclPtr<PushButton>         maSwitcher;

    // callback for interactive changes
    Link<SvxLightCtl3D*,void>  maUserInteractiveChangeCallback;
    Link<SvxLightCtl3D*,void>  maUserSelectionChangeCallback;

public:
    SvxLightCtl3D(vcl::Window* pParent);
    virtual ~SvxLightCtl3D();
    virtual void dispose() override;

    // react to size changes
    virtual void Resize() override;
    void NewLayout();

    // check the selection for validity
    void CheckSelection();

    // bring further settings to the outside world
    Svx3DLightControl& GetSvx3DLightControl() { return *maLightControl.get(); }

    // register user callback
    void SetUserInteractiveChangeCallback(Link<SvxLightCtl3D*,void> aNew) { maUserInteractiveChangeCallback = aNew; }
    void SetUserSelectionChangeCallback(Link<SvxLightCtl3D*,void> aNew) { maUserSelectionChangeCallback = aNew; }

    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;

    virtual Size GetOptimalSize() const override;

protected:

    DECL_LINK_TYPED( InternalInteractiveChange, Svx3DLightControl*, void);
    DECL_LINK_TYPED( InternalSelectionChange, Svx3DLightControl*, void);
    DECL_LINK_TYPED( ScrollBarMove, ScrollBar*, void);
    DECL_LINK_TYPED( ButtonPress, Button*, void);

    // initialize local parameters
    void Init();

    void move( double fDeltaHor, double fDeltaVer );
};

#endif // _SCH_DLGCTL3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
