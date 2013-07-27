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


#include <string>

#include <osl/mutex.hxx>

#include <svtools/toolbarmenu.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svl/eitem.hxx>
#include <vcl/settings.hxx>
#include <svl/intitem.hxx>
#include <editeng/colritem.hxx>

#include <svx/dialogs.hrc>
#include <svx/svdtrans.hxx>
#include <svx/sdasitm.hxx>
#include <svx/dialmgr.hxx>
#include "svx/extrusioncolorcontrol.hxx"

#include "helpid.hrc"
#include "extrusioncontrols.hxx"
#include "extrusioncontrols.hrc"
#include "colorwindow.hxx"
#include "extrusiondepthdialog.hxx"

////////////

//using ::svtools::ToolbarMenu;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::graphic;

namespace svx
{

/*************************************************************************
|*
|* ExtrusionDirectionWindow
|*
\************************************************************************/

static sal_Int32 gSkewList[] = { 135, 90, 45, 180, 0, -360, -135, -90, -45 };

ExtrusionDirectionWindow::ExtrusionDirectionWindow(
    svt::ToolboxController& rController,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParentWindow
)
:   ToolbarMenu( rFrame, pParentWindow, SVX_RES( RID_SVXFLOAT_EXTRUSION_DIRECTION )) ,
    mrController( rController ) ,
    maImgPerspective( SVX_RES( IMG_PERSPECTIVE ) ) ,
    maImgParallel(    SVX_RES( IMG_PARALLEL    ) ) ,
    msExtrusionDirection(  ".uno:ExtrusionDirection"  ) ,
    msExtrusionProjection( ".uno:ExtrusionProjection" )
{
    SetHelpId( HID_MENU_EXTRUSION_DIRECTION );

    sal_uInt16 i;
    for( i = DIRECTION_NW; i <= DIRECTION_SE; i++ )
    {
        maImgDirection[i] = Image( SVX_RES( IMG_DIRECTION + i ) );
    }

    SetSelectHdl( LINK( this, ExtrusionDirectionWindow, SelectHdl ) );
    mpDirectionSet = createEmptyValueSetControl();
    mpDirectionSet->SetHelpId( HID_VALUESET_EXTRUSION_DIRECTION );

    mpDirectionSet->SetSelectHdl( LINK( this, ExtrusionDirectionWindow, SelectHdl ) );
    mpDirectionSet->SetColCount( 3 );
    mpDirectionSet->EnableFullItemMode( sal_False );

    for( i = DIRECTION_NW; i <= DIRECTION_SE; i++ )
    {
        OUString aText( SVX_RESSTR( STR_DIRECTION + i ) );
        mpDirectionSet->InsertItem( i+1, maImgDirection[ i ], aText );
    }

    mpDirectionSet->SetOutputSizePixel( Size( 72, 72 ) );

    appendEntry( 2, mpDirectionSet );
    appendSeparator();
    appendEntry( 0, SVX_RESSTR( STR_PERSPECTIVE ), maImgPerspective );
    appendEntry( 1, SVX_RESSTR( STR_PARALLEL    ), maImgParallel    );

    SetOutputSizePixel( getMenuSize() );

    FreeResource();

    AddStatusListener( msExtrusionDirection );
    AddStatusListener( msExtrusionProjection );
}

void ExtrusionDirectionWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolbarMenu::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        for( sal_uInt16 i = DIRECTION_NW; i <= DIRECTION_SE; i++ )
        {
            mpDirectionSet->SetItemImage( i+1, maImgDirection[ i ] );
        }

        setEntryImage( 0, maImgPerspective );
        setEntryImage( 1, maImgParallel );
    }
}

// -----------------------------------------------------------------------

void ExtrusionDirectionWindow::implSetDirection( sal_Int32 nSkew, bool bEnabled )
{
    if( mpDirectionSet )
    {
        sal_uInt16 nItemId;
        for( nItemId = DIRECTION_NW; nItemId <= DIRECTION_SE; nItemId++ )
        {
            if( gSkewList[nItemId] == nSkew )
                break;
        }

        if( nItemId <= DIRECTION_SE )
        {
            mpDirectionSet->SelectItem( nItemId+1 );
        }
        else
        {
            mpDirectionSet->SetNoSelection();
        }
    }
    enableEntry( 2, bEnabled );
}

// -----------------------------------------------------------------------

void ExtrusionDirectionWindow::implSetProjection( sal_Int32 nProjection, bool bEnabled )
{
    checkEntry( 0, (nProjection == 0) && bEnabled );
    checkEntry( 1, (nProjection == 1 ) && bEnabled );
    enableEntry( 0, bEnabled );
    enableEntry( 1, bEnabled );
}

// -----------------------------------------------------------------------

void SAL_CALL ExtrusionDirectionWindow::statusChanged(
    const ::com::sun::star::frame::FeatureStateEvent& Event
)   throw ( ::com::sun::star::uno::RuntimeException )
{
    if( Event.FeatureURL.Main.equals( msExtrusionDirection ) )
    {
        if( !Event.IsEnabled )
        {
            implSetDirection( -1, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetDirection( nValue, true );
        }
    }
    else if( Event.FeatureURL.Main.equals( msExtrusionProjection ) )
    {
        if( !Event.IsEnabled )
        {
            implSetProjection( -1, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetProjection( nValue, true );
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( ExtrusionDirectionWindow, SelectHdl, void *, pControl )
{
    if ( IsInPopupMode() )
        EndPopupMode();

    if( pControl == mpDirectionSet )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = msExtrusionDirection.copy(5);
        aArgs[0].Value <<= (sal_Int32)gSkewList[mpDirectionSet->GetSelectItemId()-1];

        mrController.dispatchCommand( msExtrusionDirection, aArgs );
    }
    else
    {
        int nProjection = getSelectedEntryId();
        if( (nProjection >= 0) && (nProjection < 2 ) )
        {
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = msExtrusionProjection.copy(5);
            aArgs[0].Value <<= (sal_Int32)nProjection;

            mrController.dispatchCommand( msExtrusionProjection, aArgs );
            implSetProjection( nProjection, true );
        }
    }

    return 0;
}

// =======================================================================
// ExtrusionDirectionControl
// =======================================================================

ExtrusionDirectionControl::ExtrusionDirectionControl(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController(
        rxContext,
        Reference< frame::XFrame >(),
        OUString( ".uno:ExtrusionDirectionFloater" )
    )
{
}

// -----------------------------------------------------------------------

::Window* ExtrusionDirectionControl::createPopupWindow( ::Window* pParent )
{
    return new ExtrusionDirectionWindow( *this, m_xFrame, pParent );
}

// -----------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------

OUString SAL_CALL ExtrusionDirectionControl_getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionDirectionController" );
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionDirectionControl_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( "com.sun.star.frame.ToolbarController" );
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL SAL_CALL ExtrusionDirectionControl_createInstance(
    const Reference< XMultiServiceFactory >& rSMgr
)   throw( RuntimeException )
{
    return *new ExtrusionDirectionControl( comphelper::getComponentContext(rSMgr) );
}

// --------------------------------------------------------------------

OUString SAL_CALL ExtrusionDirectionControl::getImplementationName(  ) throw (RuntimeException)
{
    return ExtrusionDirectionControl_getImplementationName();
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionDirectionControl::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return ExtrusionDirectionControl_getSupportedServiceNames();
}

// ####################################################################

ExtrusionDepthDialog::ExtrusionDepthDialog( Window* pParent, double fDepth, FieldUnit eDefaultUnit )
:   ModalDialog( pParent, SVX_RES( RID_SVX_MDLG_EXTRUSION_DEPTH ) ),
    maFLDepth( this, SVX_RES( FL_DEPTH ) ),
    maMtrDepth( this, SVX_RES( MTR_DEPTH ) ),
    maOKButton( this, SVX_RES( BTN_OK ) ),
    maCancelButton( this, SVX_RES( BTN_CANCEL ) ),
    maHelpButton( this, SVX_RES( BTN_HELP ) )
{
    bool bMetric = IsMetric( eDefaultUnit );
    maMtrDepth.SetUnit( bMetric ? FUNIT_CM : FUNIT_INCH );
    maMtrDepth.SetValue( (int) fDepth * 100, FUNIT_100TH_MM );

    FreeResource();
}

ExtrusionDepthDialog::~ExtrusionDepthDialog()
{
}

double ExtrusionDepthDialog::getDepth() const
{
    return (double)( maMtrDepth.GetValue( FUNIT_100TH_MM ) ) / 100.0;
}

// ####################################################################

double aDepthListInch[] = { 0, 1270,2540,5080,10160 };
double aDepthListMM[] = { 0, 1000, 2500, 5000, 10000 };

ExtrusionDepthWindow::ExtrusionDepthWindow(
    svt::ToolboxController& rController,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParentWindow
)   : ToolbarMenu( rFrame, pParentWindow, SVX_RES( RID_SVXFLOAT_EXTRUSION_DEPTH ))
    , mrController( rController )
    , maImgDepth0( SVX_RES( IMG_DEPTH_0 ) )
    , maImgDepth1( SVX_RES( IMG_DEPTH_1 ) )
    , maImgDepth2( SVX_RES( IMG_DEPTH_2 ) )
    , maImgDepth3( SVX_RES( IMG_DEPTH_3 ) )
    , maImgDepth4( SVX_RES( IMG_DEPTH_4 ) )
    , maImgDepthInfinity( SVX_RES( IMG_DEPTH_INFINITY ) )
    , mfDepth( -1.0 )
    , msExtrusionDepth( ".uno:ExtrusionDepth" )
    , msMetricUnit(     ".uno:MetricUnit"     )
{
    SetHelpId( HID_MENU_EXTRUSION_DEPTH );

    SetSelectHdl( LINK( this, ExtrusionDepthWindow, SelectHdl ) );

    String aEmpty;
    appendEntry( 0, aEmpty, maImgDepth0 );
    appendEntry( 1, aEmpty, maImgDepth1 );
    appendEntry( 2, aEmpty, maImgDepth2 );
    appendEntry( 3, aEmpty, maImgDepth3 );
    appendEntry( 4, aEmpty, maImgDepth4 );
    appendEntry( 5, SVX_RESSTR( STR_INFINITY ), maImgDepthInfinity );
    appendEntry( 6, SVX_RESSTR( STR_CUSTOM ) );

    SetOutputSizePixel( getMenuSize() );

    FreeResource();

    AddStatusListener( msExtrusionDepth );
    AddStatusListener( msMetricUnit );
}

// -----------------------------------------------------------------------

void ExtrusionDepthWindow::implSetDepth( double fDepth )
{
    mfDepth = fDepth;
    int i;
    for( i = 0; i < 7; i++ )
    {
        if( i == 5 )
        {
            checkEntry( i, fDepth >= 338666 );
        }
        else if( i != 6 )
        {
            checkEntry( i, (fDepth == (IsMetric( meUnit ) ? aDepthListMM[i] : aDepthListInch[i]) ) );
        }
    }
}

// -----------------------------------------------------------------------

void ExtrusionDepthWindow::implFillStrings( FieldUnit eUnit )
{
    meUnit = eUnit;
    sal_uInt16 nResource = IsMetric( eUnit ) ? RID_SVXSTR_DEPTH_0 : RID_SVXSTR_DEPTH_0_INCH;

    for( int i = 0; i < 5; i++ )
    {
        OUString aStr( SVX_RESSTR( nResource + i ) );
        setEntryText( i, aStr );
    };
}

// -----------------------------------------------------------------------

void SAL_CALL ExtrusionDepthWindow::statusChanged(
    const ::com::sun::star::frame::FeatureStateEvent& Event
)   throw ( ::com::sun::star::uno::RuntimeException )
{
    if( Event.FeatureURL.Main.equals( msExtrusionDepth ) )
    {
        if( !Event.IsEnabled )
        {
            implSetDepth( 0 );
        }
        else
        {
            double fValue = 0.0;
            if( Event.State >>= fValue )
                implSetDepth( fValue );
        }
    }
    else if( Event.FeatureURL.Main.equals( msMetricUnit ) )
    {
        if( Event.IsEnabled )
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
            {
                implFillStrings( static_cast<FieldUnit>(nValue) );
                if( mfDepth >= 0.0 )
                    implSetDepth( mfDepth );
            }
        }
    }
}

// -----------------------------------------------------------------------

void ExtrusionDepthWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolbarMenu::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        setEntryImage( 0, maImgDepth0 );
        setEntryImage( 1, maImgDepth1 );
        setEntryImage( 2, maImgDepth2 );
        setEntryImage( 3, maImgDepth3 );
        setEntryImage( 4, maImgDepth4 );
        setEntryImage( 5, maImgDepthInfinity );
    }
}


// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ExtrusionDepthWindow, SelectHdl)
{
    int nSelected = getSelectedEntryId();
    if( nSelected != -1 )
    {
        if( nSelected == 6 )
        {
            if ( IsInPopupMode() )
                EndPopupMode();

            const OUString aCommand( ".uno:ExtrusionDepthDialog" );

            Any a;
            Sequence< PropertyValue > aArgs( 2 );
            aArgs[0].Name = OUString( "Depth" );
            aArgs[0].Value <<= mfDepth;
            aArgs[1].Name = OUString( "Metric" );
            aArgs[1].Value <<= static_cast<sal_Int32>( meUnit );

            mrController.dispatchCommand( aCommand, aArgs );
        }
        else
        {
            double fDepth;

            if( nSelected == 5 )
            {
                fDepth = 338666.6;
            }
            else
            {
                fDepth = IsMetric( meUnit ) ? aDepthListMM[nSelected] : aDepthListInch[nSelected];
            }

            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = msExtrusionDepth.copy(5);
            aArgs[0].Value <<= fDepth;

            mrController.dispatchCommand( msExtrusionDepth,  aArgs );
            implSetDepth( fDepth );

            if ( IsInPopupMode() )
                EndPopupMode();
        }
    }
    return 0;
}

// =======================================================================
// ExtrusionDirectionControl
// =======================================================================

ExtrusionDepthController::ExtrusionDepthController(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController(
        rxContext,
        Reference< frame::XFrame >(),
        OUString( ".uno:ExtrusionDepthFloater" )
    )
{
}

// -----------------------------------------------------------------------

::Window* ExtrusionDepthController::createPopupWindow( ::Window* pParent )
{
    return new ExtrusionDepthWindow( *this, m_xFrame, pParent );
}


// -----------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------

OUString SAL_CALL ExtrusionDepthController_getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionDepthController" );
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionDepthController_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( "com.sun.star.frame.ToolbarController" );
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL SAL_CALL ExtrusionDepthController_createInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( RuntimeException )
{
    return *new ExtrusionDepthController( comphelper::getComponentContext(rSMgr) );
}

// --------------------------------------------------------------------

OUString SAL_CALL ExtrusionDepthController::getImplementationName(  ) throw (RuntimeException)
{
    return ExtrusionDepthController_getImplementationName();
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionDepthController::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return ExtrusionDepthController_getSupportedServiceNames();
}


// ####################################################################

// -------------------------------------------------------------------------

ExtrusionLightingWindow::ExtrusionLightingWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, Window* pParentWindow )
: ToolbarMenu( rFrame, pParentWindow, SVX_RES( RID_SVXFLOAT_EXTRUSION_LIGHTING ))
, mrController( rController )
, maImgBright( SVX_RES( IMG_LIGHTING_BRIGHT ) )
, maImgNormal( SVX_RES( IMG_LIGHTING_NORMAL ) )
, maImgDim(    SVX_RES( IMG_LIGHTING_DIM    ) )
, mnLevel( 0 )
, mbLevelEnabled( false )
, mnDirection( FROM_FRONT )
, mbDirectionEnabled( false )
, msExtrusionLightingDirection( ".uno:ExtrusionLightingDirection" )
, msExtrusionLightingIntensity( ".uno:ExtrusionLightingIntensity" )
{
    sal_uInt16 i;
    for( i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; i++ )
    {
        if( i != FROM_FRONT )
        {
            maImgLightingOff[  i ] = Image( SVX_RES( IMG_LIGHT_OFF   + i ) );
            maImgLightingOn[   i ] = Image( SVX_RES( IMG_LIGHT_ON    + i ) );
        }
        maImgLightingPreview[i] = Image( SVX_RES( IMG_LIGHT_PREVIEW + i ) );
    }

    SetHelpId( HID_MENU_EXTRUSION_LIGHTING );
    SetSelectHdl( LINK( this, ExtrusionLightingWindow, SelectHdl ) );

    mpLightingSet = createEmptyValueSetControl();
    mpLightingSet->SetHelpId( HID_VALUESET_EXTRUSION_LIGHTING );

    mpLightingSet->SetSelectHdl( LINK( this, ExtrusionLightingWindow, SelectHdl ) );
    mpLightingSet->SetColCount( 3 );
    mpLightingSet->EnableFullItemMode( sal_False );

    for( i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; i++ )
    {
        if( i != FROM_FRONT )
        {
            mpLightingSet->InsertItem( i+1, maImgLightingOff[i] );
        }
        else
        {
            mpLightingSet->InsertItem( 5, maImgLightingPreview[FROM_FRONT] );
        }
    }
    mpLightingSet->SetOutputSizePixel( Size( 72, 72 ) );

    appendEntry( 3, mpLightingSet );
    appendSeparator();
    appendEntry( 0, SVX_RESSTR( STR_BRIGHT ), maImgBright );
    appendEntry( 1, SVX_RESSTR( STR_NORMAL ), maImgNormal );
    appendEntry( 2, SVX_RESSTR( STR_DIM    ), maImgDim    );

    SetOutputSizePixel( getMenuSize() );

    FreeResource();

    AddStatusListener( msExtrusionLightingDirection );
    AddStatusListener( msExtrusionLightingIntensity );
}

// -----------------------------------------------------------------------

void ExtrusionLightingWindow::implSetIntensity( int nLevel, bool bEnabled )
{
    mnLevel = nLevel;
    mbLevelEnabled = bEnabled;
    int i = 0;
    for( i = 0; i < 3; i++ )
    {
        checkEntry( i, (i == nLevel) && bEnabled );
        enableEntry( i, bEnabled );
    }
}

// -----------------------------------------------------------------------

void ExtrusionLightingWindow::implSetDirection( int nDirection, bool bEnabled )
{
    mnDirection = nDirection;
    mbDirectionEnabled = bEnabled;

    if( !bEnabled )
        nDirection = FROM_FRONT;

    sal_uInt16 nItemId;
    for( nItemId = FROM_TOP_LEFT; nItemId <= FROM_BOTTOM_RIGHT; nItemId++ )
    {
        if( nItemId == FROM_FRONT )
        {
            mpLightingSet->SetItemImage( nItemId + 1, maImgLightingPreview[ nDirection ] );
        }
        else
        {
            mpLightingSet->SetItemImage(
                nItemId + 1,
                (sal_uInt16)nDirection == nItemId ? maImgLightingOn[nItemId] : maImgLightingOff[nItemId]
            );
        }
    }

    enableEntry( 3, bEnabled );
}

// -----------------------------------------------------------------------

void SAL_CALL ExtrusionLightingWindow::statusChanged(
    const ::com::sun::star::frame::FeatureStateEvent& Event
)   throw ( ::com::sun::star::uno::RuntimeException )
{
    if( Event.FeatureURL.Main.equals( msExtrusionLightingIntensity ) )
    {
        if( !Event.IsEnabled )
        {
            implSetIntensity( 0, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetIntensity( nValue, true );
        }
    }
    else if( Event.FeatureURL.Main.equals( msExtrusionLightingDirection ) )
    {
        if( !Event.IsEnabled )
        {
            implSetDirection( 0, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetDirection( nValue, true );
        }
    }
}

// -----------------------------------------------------------------------

void ExtrusionLightingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolbarMenu::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        implSetDirection( mnDirection, mbDirectionEnabled );
        setEntryImage( 0, maImgBright );
        setEntryImage( 1, maImgNormal );
        setEntryImage( 2, maImgDim    );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( ExtrusionLightingWindow, SelectHdl, void *, pControl )
{
    if ( IsInPopupMode() )
        EndPopupMode();

    if( pControl == this )
    {
        int nLevel = getSelectedEntryId();
        if( nLevel >= 0 )
        {
            if( nLevel != 3 )
            {
                Sequence< PropertyValue > aArgs( 1 );
                aArgs[0].Name = msExtrusionLightingIntensity.copy(5);
                aArgs[0].Value <<= (sal_Int32)nLevel;

                mrController.dispatchCommand( msExtrusionLightingIntensity, aArgs );

                implSetIntensity( nLevel, true );
            }
        }
    }
    else
    {
        sal_Int32 nDirection = mpLightingSet->GetSelectItemId();

        if( (nDirection > 0) && (nDirection < 10) )
        {
            nDirection--;

            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = msExtrusionLightingDirection.copy(5);
            aArgs[0].Value <<= (sal_Int32)nDirection;

            mrController.dispatchCommand( msExtrusionLightingDirection, aArgs );

            implSetDirection( nDirection, true );
        }

    }

    return 0;
}

// ========================================================================

ExtrusionLightingControl::ExtrusionLightingControl(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController( rxContext,
                Reference< frame::XFrame >(),
                OUString( ".uno:ExtrusionDirectionFloater" )
    )
{
}

// -----------------------------------------------------------------------

::Window* ExtrusionLightingControl::createPopupWindow( ::Window* pParent )
{
    return new ExtrusionLightingWindow( *this, m_xFrame, pParent );
}

// -----------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------

OUString SAL_CALL ExtrusionLightingControl_getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionLightingController" );
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionLightingControl_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( "com.sun.star.frame.ToolbarController" );
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL SAL_CALL ExtrusionLightingControl_createInstance(
    const Reference< XMultiServiceFactory >& rSMgr
)   throw( RuntimeException )
{
    return *new ExtrusionLightingControl( comphelper::getComponentContext(rSMgr) );
}

// --------------------------------------------------------------------

OUString SAL_CALL ExtrusionLightingControl::getImplementationName(  ) throw (RuntimeException)
{
    return ExtrusionLightingControl_getImplementationName();
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionLightingControl::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return ExtrusionLightingControl_getSupportedServiceNames();
}

// ####################################################################

ExtrusionSurfaceWindow::ExtrusionSurfaceWindow(
    svt::ToolboxController& rController,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParentWindow
)   : ToolbarMenu( rFrame, pParentWindow, SVX_RES( RID_SVXFLOAT_EXTRUSION_SURFACE ) )
    , mrController( rController )
    , maImgSurface1( SVX_RES( IMG_WIRE_FRAME ) )
    , maImgSurface2( SVX_RES( IMG_MATTE ) )
    , maImgSurface3( SVX_RES( IMG_PLASTIC ) )
    , maImgSurface4( SVX_RES( IMG_METAL ) )
    , msExtrusionSurface( ".uno:ExtrusionSurface" )
{
    SetHelpId( HID_MENU_EXTRUSION_SURFACE );
    SetSelectHdl( LINK( this, ExtrusionSurfaceWindow, SelectHdl ) );

    appendEntry( 0, SVX_RESSTR( STR_WIREFRAME ), maImgSurface1 );
    appendEntry( 1, SVX_RESSTR( STR_MATTE     ), maImgSurface2 );
    appendEntry( 2, SVX_RESSTR( STR_PLASTIC   ), maImgSurface3 );
    appendEntry( 3, SVX_RESSTR( STR_METAL     ), maImgSurface4 );

    SetOutputSizePixel( getMenuSize() );

    FreeResource();

    AddStatusListener( msExtrusionSurface );
}

// -----------------------------------------------------------------------

void ExtrusionSurfaceWindow::implSetSurface( int nSurface, bool bEnabled )
{
    int i;
    for( i = 0; i < 4; i++ )
    {
        checkEntry( i, (i == nSurface) && bEnabled );
        enableEntry( i, bEnabled );
    }
}

// -----------------------------------------------------------------------

void SAL_CALL ExtrusionSurfaceWindow::statusChanged(
    const ::com::sun::star::frame::FeatureStateEvent& Event
)   throw ( ::com::sun::star::uno::RuntimeException )
{
    if( Event.FeatureURL.Main.equals( msExtrusionSurface ) )
    {
        if( !Event.IsEnabled )
        {
            implSetSurface( 0, false );
        }
        else
        {
            sal_Int32 nValue = 0;
            if( Event.State >>= nValue )
                implSetSurface( nValue, true );
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ExtrusionSurfaceWindow, SelectHdl)
{
    if ( IsInPopupMode() )
        EndPopupMode();

    sal_Int32 nSurface = getSelectedEntryId();
    if( nSurface >= 0 )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = msExtrusionSurface.copy(5);
        aArgs[0].Value <<= (sal_Int32)nSurface;

        mrController.dispatchCommand( msExtrusionSurface, aArgs );

        implSetSurface( nSurface, true );
    }

    return 0;
}

// ========================================================================

ExtrusionSurfaceControl::ExtrusionSurfaceControl(
    const Reference< XComponentContext >& rxContext
)
:   svt::PopupWindowController(
        rxContext,
        Reference< frame::XFrame >(),
        OUString( ".uno:ExtrusionSurfaceFloater" )
    )
{
}

// -----------------------------------------------------------------------

::Window* ExtrusionSurfaceControl::createPopupWindow( ::Window* pParent )
{
    return new ExtrusionSurfaceWindow( *this, m_xFrame, pParent );
}

// -----------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------

OUString SAL_CALL ExtrusionSurfaceControl_getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionSurfaceController" );
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionSurfaceControl_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( "com.sun.star.frame.ToolbarController" );
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL SAL_CALL ExtrusionSurfaceControl_createInstance(
    const Reference< XMultiServiceFactory >& rSMgr
)   throw( RuntimeException )
{
    return *new ExtrusionSurfaceControl( comphelper::getComponentContext(rSMgr) );
}

// --------------------------------------------------------------------

OUString SAL_CALL ExtrusionSurfaceControl::getImplementationName(  ) throw (RuntimeException)
{
    return ExtrusionSurfaceControl_getImplementationName();
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionSurfaceControl::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return ExtrusionSurfaceControl_getSupportedServiceNames();
}

//========================================================================

SFX_IMPL_TOOLBOX_CONTROL( ExtrusionColorControl, SvxColorItem );

ExtrusionColorControl::ExtrusionColorControl(
    sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
: SfxToolBoxControl ( nSlotId, nId, rTbx ),
  mLastColor( COL_AUTO )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
    mpBtnUpdater = new ToolboxButtonColorUpdater( nSlotId, nId, &GetToolBox() );
}

// -----------------------------------------------------------------------

ExtrusionColorControl::~ExtrusionColorControl()
{
    delete mpBtnUpdater;
}

// -----------------------------------------------------------------------

SfxPopupWindowType ExtrusionColorControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* ExtrusionColorControl::CreatePopupWindow()
{
    SvxColorWindow_Impl* pColorWin = new SvxColorWindow_Impl(
        OUString( ".uno:Extrusion3DColor" ),
        SID_EXTRUSION_3D_COLOR,
        m_xFrame,
        SVX_RESSTR( RID_SVXSTR_EXTRUSION_COLOR ),
        &GetToolBox(),
        mLastColor );
    pColorWin->StartPopupMode( &GetToolBox(), FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_ALLOWTEAROFF );
    pColorWin->StartSelection();
    SetPopupWindow( pColorWin );
    return pColorWin;
}

// -----------------------------------------------------------------------

void ExtrusionColorControl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();

    if( nSID == SID_EXTRUSION_3D_COLOR )
    {
        const SvxColorItem* pItem = 0;

        if( SFX_ITEM_DONTCARE != eState )
            pItem = PTR_CAST( SvxColorItem, pState );

        if ( pItem )
        {
            mpBtnUpdater->Update( pItem->GetValue());
            mLastColor = pItem->GetValue();
        }
    }

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
