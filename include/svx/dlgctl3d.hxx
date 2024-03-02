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

#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <svl/itemset.hxx>
#include <svx/svxdllapi.h>
#include <svx/obj3d.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <memory>

class FmFormModel;
class FmFormPage;
class E3dView;
class E3dObject;
class E3dScene;

enum class SvxPreviewObjectType { SPHERE, CUBE };

class SAL_WARN_UNUSED SVX_DLLPUBLIC Svx3DPreviewControl : public weld::CustomWidgetController
{
protected:
    std::unique_ptr<FmFormModel> mpModel;
    rtl::Reference<FmFormPage> mxFmPage;
    std::unique_ptr<E3dView> mp3DView;
    rtl::Reference<E3dScene>  mpScene;
    rtl::Reference<E3dObject>  mp3DObj;
    SvxPreviewObjectType    mnObjectType;

    void Construct();

public:
    Svx3DPreviewControl();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~Svx3DPreviewControl() override;

    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void Resize() override;

    virtual void SetObjectType(SvxPreviewObjectType nType);
    SvxPreviewObjectType GetObjectType() const { return mnObjectType; }
    SfxItemSet const & Get3DAttributes() const;
    virtual void Set3DAttributes(const SfxItemSet& rAttr);
    void ClearPageView();
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC Svx3DLightControl final : public Svx3DPreviewControl
{
    // Callback for interactive changes
    Link<Svx3DLightControl*,void>  maChangeCallback;
    Link<Svx3DLightControl*,void>  maSelectionChangeCallback;

    // lights
    sal_uInt32                  maSelectedLight;

    // extra objects for light control
    rtl::Reference<E3dObject>   mpExpansionObject;
    rtl::Reference<E3dObject>   mpLampBottomObject;
    rtl::Reference<E3dObject>   mpLampShaftObject;
    std::vector< E3dObject* >   maLightObjects;

    // 3d rotations of object
    double                      mfRotateX;
    double                      mfRotateY;
    double                      mfRotateZ;

    // interaction parameters
    Point                       maActionStartPoint;
    double                      mfSaveActionStartHor;
    double                      mfSaveActionStartVer;
    double                      mfSaveActionStartRotZ;

    bool                        mbMouseMoved : 1;
    bool                        mbMouseCaptured : 1;
    bool                        mbGeometrySelected : 1;

    void Construct2();
    void ConstructLightObjects();
    void AdaptToSelectedLight();
    void TrySelection(Point aPosPixel);

public:
    Svx3DLightControl();

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual tools::Rectangle GetFocusRect() override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool MouseMove( const MouseEvent& rMEvt ) override;
    virtual bool MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void Resize() override;

    virtual void SetObjectType(SvxPreviewObjectType nType) override;

    // register user callback
    void SetChangeCallback(Link<Svx3DLightControl*,void> aNew) { maChangeCallback = aNew; }
    void SetSelectionChangeCallback(Link<Svx3DLightControl*,void> aNew) { maSelectionChangeCallback = aNew; }

    // selection checks
    bool IsSelectionValid();
    bool IsGeometrySelected() const { return mbGeometrySelected; }

    // get/set position of selected lamp in polar coordinates, Hor:[0..360.0[ and Ver:[-90..90] degrees
    void GetPosition(double& rHor, double& rVer);
    void SetPosition(double fHor, double fVer);

    // get/set rotation of 3D object
    void SetRotation(double fRotX, double fRotY, double fRotZ);
    void GetRotation(double& rRotX, double& rRotY, double& rRotZ);

    void SelectLight(sal_uInt32 nLightNumber);
    virtual void Set3DAttributes(const SfxItemSet& rAttr) override;
    sal_uInt32 GetSelectedLight() const { return maSelectedLight; }

    // light data access
    bool GetLightOnOff(sal_uInt32 nNum) const;
    Color GetLightColor(sal_uInt32 nNum) const;
    basegfx::B3DVector GetLightDirection(sal_uInt32 nNum) const;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxLightCtl3D
{
    // local controls
    Svx3DLightControl& mrLightControl;
    weld::Scale& mrHorScroller;
    weld::Scale& mrVerScroller;
    weld::Button& mrSwitcher;

    // callback for interactive changes
    Link<SvxLightCtl3D*,void>  maUserInteractiveChangeCallback;
    Link<SvxLightCtl3D*,void>  maUserSelectionChangeCallback;

public:
    SvxLightCtl3D(Svx3DLightControl& rLightControl, weld::Scale& rHori,
               weld::Scale& rVert, weld::Button& rButton);

    // check the selection for validity
    void CheckSelection();

    // bring further settings to the outside world
    Svx3DLightControl& GetSvx3DLightControl() { return mrLightControl; }

    // register user callback
    void SetUserInteractiveChangeCallback(Link<SvxLightCtl3D*,void> aNew) { maUserInteractiveChangeCallback = aNew; }
    void SetUserSelectionChangeCallback(Link<SvxLightCtl3D*,void> aNew) { maUserSelectionChangeCallback = aNew; }

private:

    DECL_DLLPRIVATE_LINK(InternalInteractiveChange, Svx3DLightControl*, void);
    DECL_DLLPRIVATE_LINK(InternalSelectionChange, Svx3DLightControl*, void);
    DECL_DLLPRIVATE_LINK(ScrollBarMove, weld::Scale&, void);
    DECL_DLLPRIVATE_LINK(ButtonPress, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(KeyInput, const KeyEvent&, bool);
    DECL_DLLPRIVATE_LINK(FocusIn, weld::Widget&, void);

    // initialize local parameters
    void Init();

    void move( double fDeltaHor, double fDeltaVer );
};


#endif // _SCH_DLGCTL3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
