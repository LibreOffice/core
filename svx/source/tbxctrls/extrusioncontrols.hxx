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
#ifndef _SVX_EXTRUSION_CONTROLS_HXX
#define _SVX_EXTRUSION_CONTROLS_HXX

#include "svx/svxdllapi.h"

#include <svtools/valueset.hxx>
#include <svtools/svtreebx.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

#include <svtools/toolbarmenu.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/popupmenucontrollerbase.hxx>

class SfxBindings;
class SfxStatusForwarder;

//========================================================================

namespace svx
{
class ToolboxButtonColorUpdater;

class ExtrusionDirectionWindow : public svtools::ToolbarMenu
{
public:
    ExtrusionDirectionWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, Window* pParentWindow );

    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );

private:
    svt::ToolboxController& mrController;
    ValueSet* mpDirectionSet;

    Image       maImgDirection[9];
    Image       maImgDirectionH[9];

    Image       maImgPerspective;
    Image       maImgPerspectiveH;
    Image       maImgParallel;
    Image       maImgParallelH;

    const rtl::OUString msExtrusionDirection;
    const rtl::OUString msExtrusionProjection;

    DECL_LINK( SelectHdl, void * );
    void            FillValueSet();

    void implSetDirection( sal_Int32 nSkew, bool bEnabled = true );
    void implSetProjection( sal_Int32 nProjection, bool bEnabled = true );

};

//========================================================================

class ExtrusionDirectionControl : public svt::PopupWindowController
{
public:
    ExtrusionDirectionControl( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager );

    virtual ::Window* createPopupWindow( ::Window* pParent );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );
};

//========================================================================

class ExtrusionDepthController : public svt::PopupMenuControllerBase
{
    using svt::PopupMenuControllerBase::disposing;
private:
    Image maImgDepth[5];
    Image maImgDepthInfinity;

    Image maImgDepth_hc[5];
    Image maImgDepthInfinityh;

    FieldUnit   meUnit;
    double      mfDepth;

    const rtl::OUString msExtrusionDepth;
    const rtl::OUString msMetricUnit;

public:
    ExtrusionDepthController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
    virtual ~ExtrusionDepthController();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XStatusListener
    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

    // XMenuListener
    virtual void SAL_CALL select( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

    // XPopupMenuController
    virtual void SAL_CALL updatePopupMenu() throw (::com::sun::star::uno::RuntimeException);
};

//========================================================================

class ExtrusionLightingWindow : public svtools::ToolbarMenu
{
private:
    svt::ToolboxController& mrController;
    ValueSet*    mpLightingSet;

    Image maImgLightingOff[9];
    Image maImgLightingOn[9];
    Image maImgLightingPreview[9];

    Image maImgLightingOffh[9];
    Image maImgLightingOnh[9];
    Image maImgLightingPreviewh[9];

    Image maImgBright;
    Image maImgNormal;
    Image maImgDim;
    Image maImgBrighth;
    Image maImgNormalh;
    Image maImgDimh;

    int     mnLevel;
    bool    mbLevelEnabled;
    int     mnDirection;
    bool    mbDirectionEnabled;

    const rtl::OUString msExtrusionLightingDirection;
    const rtl::OUString msExtrusionLightingIntensity;

    void    implSetIntensity( int nLevel, bool bEnabled );
    void    implSetDirection( int nDirection, bool bEnabled );

    DECL_LINK( SelectHdl, void * );
public:
    ExtrusionLightingWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, Window* pParentWindow );

    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

//========================================================================

class ExtrusionLightingControl : public svt::PopupWindowController
{
public:
    ExtrusionLightingControl( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager );

    virtual ::Window* createPopupWindow( ::Window* pParent );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );
};

//========================================================================

class ExtrusionSurfaceWindow : public svtools::ToolbarMenu
{
private:
    svt::ToolboxController& mrController;

    Image maImgSurface1;
    Image maImgSurface2;
    Image maImgSurface3;
    Image maImgSurface4;
    Image maImgSurface1h;
    Image maImgSurface2h;
    Image maImgSurface3h;
    Image maImgSurface4h;

    const rtl::OUString msExtrusionSurface;

    DECL_LINK( SelectHdl, void * );

    void    implSetSurface( int nSurface, bool bEnabled );

public:
    ExtrusionSurfaceWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, Window* pParentWindow );

    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );
};

//========================================================================

class ExtrusionSurfaceControl : public svt::PopupWindowController
{
public:
    ExtrusionSurfaceControl( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager );

    virtual ::Window* createPopupWindow( ::Window* pParent );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );
};

//========================================================================

}
#endif

