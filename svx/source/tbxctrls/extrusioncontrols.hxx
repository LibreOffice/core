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
    ExtrusionDirectionWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, vcl::Window* pParentWindow );
    virtual ~ExtrusionDirectionWindow();
    virtual void dispose() override;

    virtual void statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException ) override;
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

private:
    svt::ToolboxController& mrController;
    VclPtr<ValueSet>        mpDirectionSet;

    Image       maImgDirection[9];
    Image       maImgPerspective;
    Image       maImgParallel;

    const OUString msExtrusionDirection;
    const OUString msExtrusionProjection;

    DECL_LINK_TYPED( SelectToolbarMenuHdl, ToolbarMenu*, void );
    DECL_LINK_TYPED( SelectValueSetHdl, ValueSet*, void );
    void SelectHdl(void*);

    void implSetDirection( sal_Int32 nSkew, bool bEnabled = true );
    void implSetProjection( sal_Int32 nProjection, bool bEnabled = true );

};



class ExtrusionDirectionControl : public svt::PopupWindowController
{
public:
    explicit ExtrusionDirectionControl( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext );

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

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
    ExtrusionDepthWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, vcl::Window* pParentWindow );

    virtual void statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
};

class ExtrusionDepthController : public svt::PopupWindowController
{
public:
    explicit ExtrusionDepthController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext );

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

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

    const OUString msExtrusionLightingDirection;
    const OUString msExtrusionLightingIntensity;

    void    implSetIntensity( int nLevel, bool bEnabled );
    void    implSetDirection( int nDirection, bool bEnabled );

    DECL_LINK_TYPED( SelectToolbarMenuHdl, ToolbarMenu*, void );
    DECL_LINK_TYPED( SelectValueSetHdl, ValueSet*, void );
    void SelectHdl(void*);
public:
    ExtrusionLightingWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, vcl::Window* pParentWindow );
    virtual ~ExtrusionLightingWindow();
    virtual void dispose() override;

    virtual void statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException ) override;
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
};



class ExtrusionLightingControl : public svt::PopupWindowController
{
public:
    explicit ExtrusionLightingControl( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext );

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

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

    const OUString msExtrusionSurface;

    DECL_LINK_TYPED( SelectHdl, ToolbarMenu*, void );

    void    implSetSurface( int nSurface, bool bEnabled );

public:
    ExtrusionSurfaceWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, vcl::Window* pParentWindow );

    virtual void statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException ) override;
};



class ExtrusionSurfaceControl : public svt::PopupWindowController
{
public:
    explicit ExtrusionSurfaceControl( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext );

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    using svt::PopupWindowController::createPopupWindow;
};



}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
