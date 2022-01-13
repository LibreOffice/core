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
#ifndef INCLUDED_SVX_SOURCE_TBXCTRLS_EXTRUSIONCONTROLS_HXX
#define INCLUDED_SVX_SOURCE_TBXCTRLS_EXTRUSIONCONTROLS_HXX

#include <svtools/toolbarmenu.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/valueset.hxx>
#include <vcl/customweld.hxx>

// enum to index light images
#define FROM_TOP_LEFT       0
#define FROM_TOP            1
#define FROM_TOP_RIGHT      2
#define FROM_LEFT           3
#define FROM_FRONT          4
#define FROM_RIGHT          5
#define FROM_BOTTOM_LEFT    6
#define FROM_BOTTOM         7
#define FROM_BOTTOM_RIGHT   8

#define DIRECTION_NW        0
#define DIRECTION_N         1
#define DIRECTION_NE        2
#define DIRECTION_W         3
#define DIRECTION_NONE      4
#define DIRECTION_E         5
#define DIRECTION_SW        6
#define DIRECTION_S         7
#define DIRECTION_SE        8

namespace svx
{
class ExtrusionDirectionWindow final : public WeldToolbarPopup
{
public:
    ExtrusionDirectionWindow(svt::PopupWindowController* pControl, weld::Widget* pParentWindow);
    virtual void GrabFocus() override;
    virtual ~ExtrusionDirectionWindow() override;

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override;

private:
    rtl::Reference<svt::PopupWindowController> mxControl;
    std::unique_ptr<ValueSet> mxDirectionSet;
    std::unique_ptr<weld::CustomWeld> mxDirectionSetWin;
    std::unique_ptr<weld::RadioButton> mxPerspective;
    std::unique_ptr<weld::RadioButton> mxParallel;

    Image       maImgDirection[9];

    DECL_LINK( SelectToolbarMenuHdl, weld::Toggleable&, void );
    DECL_LINK( SelectValueSetHdl, ValueSet*, void );

    void implSetDirection( sal_Int32 nSkew, bool bEnabled );
    void implSetProjection( sal_Int32 nProjection, bool bEnabled );

};

class ExtrusionDirectionControl : public svt::PopupWindowController
{
public:
    explicit ExtrusionDirectionControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ExtrusionDepthWindow final : public WeldToolbarPopup
{
private:
    rtl::Reference<svt::PopupWindowController> mxControl;
    std::unique_ptr<weld::RadioButton> mxDepth0;
    std::unique_ptr<weld::RadioButton> mxDepth1;
    std::unique_ptr<weld::RadioButton> mxDepth2;
    std::unique_ptr<weld::RadioButton> mxDepth3;
    std::unique_ptr<weld::RadioButton> mxDepth4;
    std::unique_ptr<weld::RadioButton> mxInfinity;
    std::unique_ptr<weld::RadioButton> mxCustom;

    FieldUnit   meUnit;
    double      mfDepth;
    bool        mbSettingValue;
    bool        mbCommandDispatched;

    DECL_LINK( SelectHdl, weld::Toggleable&, void );
    DECL_LINK( MouseReleaseHdl, const MouseEvent&, bool );

    void    implFillStrings( FieldUnit eUnit );
    void    implSetDepth( double fDepth );

    void    DispatchDepthDialog();

public:
    ExtrusionDepthWindow(svt::PopupWindowController* pControl, weld::Widget* pParentWindow);
    virtual void GrabFocus() override;

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override;
};

class ExtrusionDepthController : public svt::PopupWindowController
{
public:
    explicit ExtrusionDepthController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ExtrusionLightingWindow final : public WeldToolbarPopup
{
private:
    rtl::Reference<svt::PopupWindowController> mxControl;
    std::unique_ptr<ValueSet> mxLightingSet;
    std::unique_ptr<weld::CustomWeld> mxLightingSetWin;
    std::unique_ptr<weld::RadioButton> mxBright;
    std::unique_ptr<weld::RadioButton> mxNormal;
    std::unique_ptr<weld::RadioButton> mxDim;

    Image maImgLightingOff[9];
    Image maImgLightingOn[9];
    Image maImgLightingPreview[9];

    void    implSetIntensity( int nLevel, bool bEnabled );
    void    implSetDirection( int nDirection, bool bEnabled );

    DECL_LINK( SelectToolbarMenuHdl, weld::Toggleable&, void );
    DECL_LINK( SelectValueSetHdl, ValueSet*, void );
public:
    ExtrusionLightingWindow(svt::PopupWindowController* pControl, weld::Widget* pParentWindow);
    virtual void GrabFocus() override;
    virtual ~ExtrusionLightingWindow() override;

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override;
};

class ExtrusionLightingControl : public svt::PopupWindowController
{
public:
    explicit ExtrusionLightingControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ExtrusionSurfaceWindow final : public WeldToolbarPopup
{
private:
    rtl::Reference<svt::PopupWindowController> mxControl;
    std::unique_ptr<weld::RadioButton> mxWireFrame;
    std::unique_ptr<weld::RadioButton> mxMatt;
    std::unique_ptr<weld::RadioButton> mxPlastic;
    std::unique_ptr<weld::RadioButton> mxMetal;
    std::unique_ptr<weld::RadioButton> mxMetalMSO;

    DECL_LINK( SelectHdl, weld::Toggleable&, void );

    void    implSetSurface( int nSurface, bool bEnabled );

public:
    ExtrusionSurfaceWindow(svt::PopupWindowController* pControl, weld::Widget* pParentWindow);
    virtual void GrabFocus() override;

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override;
};


class ExtrusionSurfaceControl : public svt::PopupWindowController
{
public:
    explicit ExtrusionSurfaceControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
