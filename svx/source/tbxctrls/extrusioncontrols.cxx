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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/awt/XPopupMenuExtended.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <vos/mutex.hxx>

#include <svtools/toolbarmenu.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svl/eitem.hxx>
#include <vcl/settings.hxx>
#include <svl/intitem.hxx>
#include <editeng/colritem.hxx>
#include <tools/urlobj.hxx>

#include <svx/dialogs.hrc>
#include <svx/svdtrans.hxx>
#include <svx/sdasitm.hxx>
#include <svx/dialmgr.hxx>
#include "svx/extrusioncolorcontrol.hxx"

//#include "chrtitem.hxx"
#include "helpid.hrc"
#include "extrusioncontrols.hxx"
#include "extrusioncontrols.hrc"
#include "colorwindow.hxx"
#include "extrusiondepthdialog.hxx"

////////////

using ::rtl::OUString;
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

ExtrusionDirectionWindow::ExtrusionDirectionWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, Window* pParentWindow )
: ToolbarMenu( rFrame, pParentWindow, SVX_RES( RID_SVXFLOAT_EXTRUSION_DIRECTION ))
, mrController( rController )
, maImgPerspective( SVX_RES( IMG_PERSPECTIVE ) )
, maImgPerspectiveH( SVX_RES( IMG_PERSPECTIVE_H ) )
, maImgParallel( SVX_RES( IMG_PARALLEL ) )
, maImgParallelH( SVX_RES( IMG_PARALLEL_H ) )
, msExtrusionDirection( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionDirection" ) )
, msExtrusionProjection( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionProjection" ) )
{
    SetHelpId( HID_MENU_EXTRUSION_DIRECTION );

    sal_uInt16 i;
    for( i = DIRECTION_NW; i <= DIRECTION_SE; i++ )
    {
        maImgDirection[i] = Image( SVX_RES( IMG_DIRECTION + i ) );
        maImgDirectionH[i] = Image( SVX_RES( IMG_DIRECTION_H + i ) );
    }

    SetSelectHdl( LINK( this, ExtrusionDirectionWindow, SelectHdl ) );
    mpDirectionSet = createEmptyValueSetControl();
    mpDirectionSet->SetHelpId( HID_VALUESET_EXTRUSION_DIRECTION );

    mpDirectionSet->SetSelectHdl( LINK( this, ExtrusionDirectionWindow, SelectHdl ) );
    mpDirectionSet->SetColCount( 3 );
    mpDirectionSet->EnableFullItemMode( sal_False );

    bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

    for( i = DIRECTION_NW; i <= DIRECTION_SE; i++ )
    {
        String aText( SVX_RES( STR_DIRECTION + i ) );
        mpDirectionSet->InsertItem( i+1, bHighContrast ? maImgDirectionH[ i ] : maImgDirection[ i ], aText );
    }

    mpDirectionSet->SetOutputSizePixel( Size( 72, 72 ) );

    appendEntry( 2, mpDirectionSet );
    appendSeparator();
    appendEntry( 0, String( SVX_RES( STR_PERSPECTIVE ) ), bHighContrast ? maImgPerspectiveH : maImgPerspective );
    appendEntry( 1, String( SVX_RES( STR_PARALLEL ) ), bHighContrast ? maImgParallelH : maImgParallel );

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
        bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

        for( sal_uInt16 i = DIRECTION_NW; i <= DIRECTION_SE; i++ )
        {
            mpDirectionSet->SetItemImage( i+1, bHighContrast ? maImgDirectionH[ i ] : maImgDirection[ i ] );
        }

        setEntryImage( 0, bHighContrast ? maImgPerspectiveH : maImgPerspective );
        setEntryImage( 1, bHighContrast ? maImgParallelH : maImgParallel );
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

void SAL_CALL ExtrusionDirectionWindow::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException )
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

ExtrusionDirectionControl::ExtrusionDirectionControl( const Reference< lang::XMultiServiceFactory >& rServiceManager )
: svt::PopupWindowController( rServiceManager, Reference< frame::XFrame >(), OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionDirectionFloater" ) ) )
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
    return OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svx.ExtrusionDirectionController" ));
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionDirectionControl_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ));
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL SAL_CALL ExtrusionDirectionControl_createInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( RuntimeException )
{
    return *new ExtrusionDirectionControl( rSMgr );
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
//  bool bMetric = IsMetric( meDefaultUnit );
    return (double)( maMtrDepth.GetValue( FUNIT_100TH_MM ) ) / 100.0;
}

// ####################################################################

double aDepthListInch[] = { 0, 1270,2540,5080,10160 };
double aDepthListMM[] = { 0, 1000, 2500, 5000, 10000 };

ExtrusionDepthWindow::ExtrusionDepthWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, Window* pParentWindow )
: ToolbarMenu( rFrame, pParentWindow, SVX_RES( RID_SVXFLOAT_EXTRUSION_DEPTH ))
, mrController( rController )
, maImgDepth0( SVX_RES( IMG_DEPTH_0 ) )
, maImgDepth1( SVX_RES( IMG_DEPTH_1 ) )
, maImgDepth2( SVX_RES( IMG_DEPTH_2 ) )
, maImgDepth3( SVX_RES( IMG_DEPTH_3 ) )
, maImgDepth4( SVX_RES( IMG_DEPTH_4 ) )
, maImgDepthInfinity( SVX_RES( IMG_DEPTH_INFINITY ) )
, maImgDepth0h( SVX_RES( IMG_DEPTH_0_H ) )
, maImgDepth1h( SVX_RES( IMG_DEPTH_1_H ) )
, maImgDepth2h( SVX_RES( IMG_DEPTH_2_H ) )
, maImgDepth3h( SVX_RES( IMG_DEPTH_3_H ) )
, maImgDepth4h( SVX_RES( IMG_DEPTH_4_H ) )
, maImgDepthInfinityh( SVX_RES( IMG_DEPTH_INFINITY_H ) )
, mfDepth( -1.0 )
, msExtrusionDepth( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionDepth" ) )
, msMetricUnit( RTL_CONSTASCII_USTRINGPARAM( ".uno:MetricUnit" ) )
{
    SetHelpId( HID_MENU_EXTRUSION_DEPTH );

    SetSelectHdl( LINK( this, ExtrusionDepthWindow, SelectHdl ) );

    bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

    String aEmpty;
    appendEntry( 0, aEmpty, bHighContrast ? maImgDepth0h : maImgDepth0 );
    appendEntry( 1, aEmpty, bHighContrast ? maImgDepth1h : maImgDepth1 );
    appendEntry( 2, aEmpty, bHighContrast ? maImgDepth2h : maImgDepth2 );
    appendEntry( 3, aEmpty, bHighContrast ? maImgDepth3h : maImgDepth3 );
    appendEntry( 4, aEmpty, bHighContrast ? maImgDepth4h : maImgDepth4 );
    appendEntry( 5, String( SVX_RES( STR_INFINITY ) ), bHighContrast ? maImgDepthInfinityh : maImgDepthInfinity );
    appendEntry( 6, String( SVX_RES( STR_CUSTOM ) ) );

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
        String aStr( SVX_RES( nResource + i ) );
        setEntryText( i, aStr );
    };
}

// -----------------------------------------------------------------------

void SAL_CALL ExtrusionDepthWindow::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException )
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
        bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

        setEntryImage( 0, bHighContrast ? maImgDepth0h : maImgDepth0 );
        setEntryImage( 1, bHighContrast ? maImgDepth1h : maImgDepth1 );
        setEntryImage( 2, bHighContrast ? maImgDepth2h : maImgDepth2 );
        setEntryImage( 3, bHighContrast ? maImgDepth3h : maImgDepth3 );
        setEntryImage( 4, bHighContrast ? maImgDepth4h : maImgDepth4 );
        setEntryImage( 5, bHighContrast ? maImgDepthInfinityh : maImgDepthInfinity );
    }
}


// -----------------------------------------------------------------------

IMPL_LINK( ExtrusionDepthWindow, SelectHdl, void *, EMPTYARG )
{
    int nSelected = getSelectedEntryId();
    if( nSelected != -1 )
    {
        if( nSelected == 6 )
        {
            if ( IsInPopupMode() )
                EndPopupMode();

            const rtl::OUString aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionDepthDialog" ));

            Any a;
            Sequence< PropertyValue > aArgs( 2 );
            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Depth" ));
            aArgs[0].Value <<= mfDepth;
            aArgs[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Metric" ));
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

ExtrusionDepthController::ExtrusionDepthController( const Reference< lang::XMultiServiceFactory >& rServiceManager )
: svt::PopupWindowController( rServiceManager, Reference< frame::XFrame >(), OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionDepthFloater" ) ) )
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
    return OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svx.ExtrusionDepthController" ));
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionDepthController_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.PopupMenuController" ));
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL SAL_CALL ExtrusionDepthController_createInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( RuntimeException )
{
    return *new ExtrusionDepthController( rSMgr );
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
, maImgDim( SVX_RES( IMG_LIGHTING_DIM ) )
, maImgBrighth( SVX_RES( IMG_LIGHTING_BRIGHT_H ) )
, maImgNormalh( SVX_RES( IMG_LIGHTING_NORMAL_H ) )
, maImgDimh( SVX_RES( IMG_LIGHTING_DIM_H ) )
, mnLevel( 0 )
, mbLevelEnabled( false )
, mnDirection( FROM_FRONT )
, mbDirectionEnabled( false )
, msExtrusionLightingDirection( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionLightingDirection" ))
, msExtrusionLightingIntensity( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionLightingIntensity" ))
{
    sal_uInt16 i;
    for( i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; i++ )
    {
        if( i != FROM_FRONT )
        {
            maImgLightingOff[i] = Image( SVX_RES( IMG_LIGHT_OFF + i ) );
            maImgLightingOn[i] = Image( SVX_RES( IMG_LIGHT_ON + i ) );
            maImgLightingOffh[i] = Image( SVX_RES( IMG_LIGHT_OFF_H + i ) );
            maImgLightingOnh[i] = Image( SVX_RES( IMG_LIGHT_ON_H + i ) );
        }
        maImgLightingPreview[i] = Image( SVX_RES( IMG_LIGHT_PREVIEW + i ) );
        maImgLightingPreviewh[i] = Image( SVX_RES( IMG_LIGHT_PREVIEW_H + i ) );
    }

    SetHelpId( HID_MENU_EXTRUSION_LIGHTING );
    SetSelectHdl( LINK( this, ExtrusionLightingWindow, SelectHdl ) );

    mpLightingSet = createEmptyValueSetControl();
    mpLightingSet->SetHelpId( HID_VALUESET_EXTRUSION_LIGHTING );

    mpLightingSet->SetSelectHdl( LINK( this, ExtrusionLightingWindow, SelectHdl ) );
    mpLightingSet->SetColCount( 3 );
    mpLightingSet->EnableFullItemMode( sal_False );

    bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

    for( i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; i++ )
    {
        if( i != FROM_FRONT )
        {
            mpLightingSet->InsertItem( i+1, bHighContrast ? maImgLightingOffh[i] : maImgLightingOff[i] );
        }
        else
        {
            mpLightingSet->InsertItem( 5, bHighContrast ? maImgLightingPreviewh[FROM_FRONT] : maImgLightingPreview[FROM_FRONT] );
        }
    }
    mpLightingSet->SetOutputSizePixel( Size( 72, 72 ) );

    appendEntry( 3, mpLightingSet );
    appendSeparator();
    appendEntry( 0, String( SVX_RES( STR_BRIGHT ) ), bHighContrast ? maImgBrighth : maImgBright );
    appendEntry( 1, String( SVX_RES( STR_NORMAL ) ), bHighContrast ? maImgNormalh : maImgNormal );
    appendEntry( 2, String( SVX_RES( STR_DIM ) ), bHighContrast ? maImgDimh : maImgDim );

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

    bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

    if( !bEnabled )
        nDirection = FROM_FRONT;

    sal_uInt16 nItemId;
    for( nItemId = FROM_TOP_LEFT; nItemId <= FROM_BOTTOM_RIGHT; nItemId++ )
    {
        if( nItemId == FROM_FRONT )
        {
            mpLightingSet->SetItemImage( nItemId + 1, bHighContrast ? maImgLightingPreviewh[ nDirection ] : maImgLightingPreview[ nDirection ] );
        }
        else
        {
            if( bHighContrast )
            {
                mpLightingSet->SetItemImage( nItemId + 1, (sal_uInt16)nDirection == nItemId ? maImgLightingOnh[nItemId] : maImgLightingOffh[nItemId] );
            }
            else
            {
                mpLightingSet->SetItemImage( nItemId + 1, (sal_uInt16)nDirection == nItemId ? maImgLightingOn[nItemId] : maImgLightingOff[nItemId] );
            }
        }
    }

    enableEntry( 3, bEnabled );
}

// -----------------------------------------------------------------------

void SAL_CALL ExtrusionLightingWindow::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException )
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
        bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

        implSetDirection( mnDirection, mbDirectionEnabled );
        setEntryImage( 0, bHighContrast ? maImgBrighth : maImgBright );
        setEntryImage( 1, bHighContrast ? maImgNormalh : maImgNormal );
        setEntryImage( 2, bHighContrast ? maImgDimh : maImgDim );
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

ExtrusionLightingControl::ExtrusionLightingControl( const Reference< lang::XMultiServiceFactory >& rServiceManager )
: svt::PopupWindowController( rServiceManager, Reference< frame::XFrame >(), OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionDirectionFloater" ) ) )
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
    return OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svx.ExtrusionLightingController" ));
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionLightingControl_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ));
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL SAL_CALL ExtrusionLightingControl_createInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( RuntimeException )
{
    return *new ExtrusionLightingControl( rSMgr );
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

ExtrusionSurfaceWindow::ExtrusionSurfaceWindow( svt::ToolboxController& rController, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, Window* pParentWindow )
: ToolbarMenu( rFrame, pParentWindow, SVX_RES( RID_SVXFLOAT_EXTRUSION_SURFACE ))
, mrController( rController )
, maImgSurface1( SVX_RES( IMG_WIRE_FRAME ) )
, maImgSurface2( SVX_RES( IMG_MATTE ) )
, maImgSurface3( SVX_RES( IMG_PLASTIC ) )
, maImgSurface4( SVX_RES( IMG_METAL ) )
, maImgSurface1h( SVX_RES( IMG_WIRE_FRAME_H ) )
, maImgSurface2h( SVX_RES( IMG_MATTE_H ) )
, maImgSurface3h( SVX_RES( IMG_PLASTIC_H ) )
, maImgSurface4h( SVX_RES( IMG_METAL_H ) )
, msExtrusionSurface( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionSurface" ))
{
    bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

    SetHelpId( HID_MENU_EXTRUSION_SURFACE );
    SetSelectHdl( LINK( this, ExtrusionSurfaceWindow, SelectHdl ) );

    appendEntry( 0, String( SVX_RES( STR_WIREFRAME ) ), bHighContrast ? maImgSurface1h : maImgSurface1 );
    appendEntry( 1, String( SVX_RES( STR_MATTE ) ), bHighContrast ? maImgSurface2h : maImgSurface2 );
    appendEntry( 2, String( SVX_RES( STR_PLASTIC ) ), bHighContrast ? maImgSurface3h : maImgSurface3 );
    appendEntry( 3, String( SVX_RES( STR_METAL ) ), bHighContrast ? maImgSurface4h : maImgSurface4 );

    SetOutputSizePixel( getMenuSize() );

    FreeResource();

    AddStatusListener( msExtrusionSurface );
}

// -----------------------------------------------------------------------

void ExtrusionSurfaceWindow::implSetSurface( int nSurface, bool bEnabled )
{
//  if( mpMenu )
    {
        int i;
        for( i = 0; i < 4; i++ )
        {
            checkEntry( i, (i == nSurface) && bEnabled );
            enableEntry( i, bEnabled );
        }
    }
}

// -----------------------------------------------------------------------

void SAL_CALL ExtrusionSurfaceWindow::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException )
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

IMPL_LINK( ExtrusionSurfaceWindow, SelectHdl, void *, EMPTYARG )
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

ExtrusionSurfaceControl::ExtrusionSurfaceControl( const Reference< lang::XMultiServiceFactory >& rServiceManager )
: svt::PopupWindowController( rServiceManager, Reference< frame::XFrame >(), OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ExtrusionSurfaceFloater" ) ) )
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
    return OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svx.ExtrusionSurfaceController" ));
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ExtrusionSurfaceControl_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ));
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL SAL_CALL ExtrusionSurfaceControl_createInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( RuntimeException )
{
    return *new ExtrusionSurfaceControl( rSMgr );
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
: SfxToolBoxControl ( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
    mpBtnUpdater = new ToolboxButtonColorUpdater( nSlotId, nId, &GetToolBox(), TBX_UPDATER_MODE_CHAR_COLOR_NEW );
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
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Extrusion3DColor" )),
        SID_EXTRUSION_3D_COLOR,
        m_xFrame,
        SVX_RESSTR( RID_SVXSTR_EXTRUSION_COLOR ),
        &GetToolBox() );
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
            mpBtnUpdater->Update( pItem->GetValue());
    }

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

}
