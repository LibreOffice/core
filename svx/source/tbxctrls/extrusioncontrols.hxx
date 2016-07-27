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

#include "svx/svxdllapi.h"

#include <svtools/treelistbox.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

#include <svtools/toolbarmenu.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/popupmenucontrollerbase.hxx>

class ValueSet;


namespace svx
{
class ExtrusionDirectionWindow : public svtools::ToolbarMenu
{
public:
    ExtrusionDirectionWindow( svt::ToolboxController& rController, const css::uno::Reference< css::frame::XFrame >& rFrame, vcl::Window* pParentWindow );
    virtual ~ExtrusionDirectionWindow();
    virtual void dispose() override;

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException ) override;
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

private:
    svt::ToolboxController& mrController;
    VclPtr<ValueSet>        mpDirectionSet;

    Image       maImgDirection[9];
    Image       maImgPerspective;
    Image       maImgParallel;

    DECL_LINK_TYPED( SelectToolbarMenuHdl, ToolbarMenu*, void );
    DECL_LINK_TYPED( SelectValueSetHdl, ValueSet*, void );
    void SelectHdl(void*);

    void implSetDirection( sal_Int32 nSkew, bool bEnabled );
    void implSetProjection( sal_Int32 nProjection, bool bEnabled );

};


class ExtrusionDirectionControl : public svt::PopupWindowController
{
public:
    explicit ExtrusionDirectionControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    using svt::PopupWindowController::createPopupWindow;
};

class ExtrusionDepthWindow : public svtools::ToolbarMenu
{
private:
    svt::ToolboxController& mrController;

    Image maImgDepth0;
    Image maImgDepth1;
    Image maImgDepth2;
    Image maImgDepth3;
    Image maImgDepth4;
    Image maImgDepthInfinity;

    FieldUnit   meUnit;
    double      mfDepth;

    const OUString msExtrusionDepth;
    const OUString msMetricUnit;

    DECL_LINK_TYPED( SelectHdl, ToolbarMenu*, void );

    void    implFillStrings( FieldUnit eUnit );
    void    implSetDepth( double fDepth );

public:
    ExtrusionDepthWindow( svt::ToolboxController& rController, const css::uno::Reference< css::frame::XFrame >& rFrame, vcl::Window* pParentWindow );

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
};

class ExtrusionDepthController : public svt::PopupWindowController
{
public:
    explicit ExtrusionDepthController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    using svt::PopupWindowController::createPopupWindow;
};


class ExtrusionLightingWindow : public svtools::ToolbarMenu
{
private:
    svt::ToolboxController& mrController;
    VclPtr<ValueSet>        mpLightingSet;

    Image maImgLightingOff[9];
    Image maImgLightingOn[9];
    Image maImgLightingPreview[9];

    Image maImgBright;
    Image maImgNormal;
    Image maImgDim;

    int     mnLevel;
    bool    mbLevelEnabled;
    int     mnDirection;
    bool    mbDirectionEnabled;

    void    implSetIntensity( int nLevel, bool bEnabled );
    void    implSetDirection( int nDirection, bool bEnabled );

    DECL_LINK_TYPED( SelectToolbarMenuHdl, ToolbarMenu*, void );
    DECL_LINK_TYPED( SelectValueSetHdl, ValueSet*, void );
    void SelectHdl(void*);
public:
    ExtrusionLightingWindow( svt::ToolboxController& rController, const css::uno::Reference< css::frame::XFrame >& rFrame, vcl::Window* pParentWindow );
    virtual ~ExtrusionLightingWindow();
    virtual void dispose() override;

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException ) override;
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
};


class ExtrusionLightingControl : public svt::PopupWindowController
{
public:
    explicit ExtrusionLightingControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    using svt::PopupWindowController::createPopupWindow;
};


class ExtrusionSurfaceWindow : public svtools::ToolbarMenu
{
private:
    svt::ToolboxController& mrController;

    Image maImgSurface1;
    Image maImgSurface2;
    Image maImgSurface3;
    Image maImgSurface4;

    DECL_LINK_TYPED( SelectHdl, ToolbarMenu*, void );

    void    implSetSurface( int nSurface, bool bEnabled );

public:
    ExtrusionSurfaceWindow( svt::ToolboxController& rController, const css::uno::Reference< css::frame::XFrame >& rFrame, vcl::Window* pParentWindow );

    virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException ) override;
};


class ExtrusionSurfaceControl : public svt::PopupWindowController
{
public:
    explicit ExtrusionSurfaceControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    using svt::PopupWindowController::createPopupWindow;
};


}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
