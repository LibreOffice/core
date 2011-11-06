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

    using svt::PopupWindowController::createPopupWindow;
};

//========================================================================

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

    Image maImgDepth0h;
    Image maImgDepth1h;
    Image maImgDepth2h;
    Image maImgDepth3h;
    Image maImgDepth4h;
    Image maImgDepthInfinityh;

    FieldUnit   meUnit;
    double      mfDepth;

    const rtl::OUString msExtrusionDepth;
    const rtl::OUString msMetricUnit;

    DECL_LINK( SelectHdl, void * );

    void    implFillStrings( FieldUnit eUnit );
    void    implSetDepth( double fDepth );

public:
    ExtrusionDepthWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, Window* pParentWindow );

    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

//========================================================================

class ExtrusionDepthController : public svt::PopupWindowController
{
public:
    ExtrusionDepthController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager );

    virtual ::Window* createPopupWindow( ::Window* pParent );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    using svt::PopupWindowController::createPopupWindow;
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

    using svt::PopupWindowController::createPopupWindow;
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

    using svt::PopupWindowController::createPopupWindow;
};

//========================================================================

}
#endif

