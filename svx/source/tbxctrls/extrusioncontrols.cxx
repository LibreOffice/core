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

#include "coreservices.hxx"
#include "helpid.hrc"
#include "extrusioncontrols.hxx"
#include "extrusioncontrols.hrc"
#include "colorwindow.hxx"
#include "extrusiondepthdialog.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::graphic;

namespace svx
{

static const sal_Int32 gSkewList[] = { 135, 90, 45, 180, 0, -360, -135, -90, -45 };
static const char g_sExtrusionDirection[] = ".uno:ExtrusionDirection";
static const char g_sExtrusionProjection[] = ".uno:ExtrusionProjection";

ExtrusionDirectionWindow::ExtrusionDirectionWindow(
    svt::ToolboxController& rController,
    const css::uno::Reference< css::frame::XFrame >& rFrame,
    vcl::Window* pParentWindow
)
    : ToolbarMenu(rFrame, pParentWindow,
                  WB_MOVEABLE|WB_CLOSEABLE|WB_HIDE|WB_3DLOOK)
    , mrController(rController)
    , maImgPerspective(SVX_RES(RID_SVXIMG_PERSPECTIVE))
    , maImgParallel(SVX_RES(RID_SVXIMG_PARALLEL))
{
    for(sal_uInt16 i = DIRECTION_NW; i <= DIRECTION_SE; ++i)
    {
        maImgDirection[i] = Image( SVX_RES( RID_SVXIMG_DIRECTION + i ) );
    }

    SetSelectHdl( LINK( this, ExtrusionDirectionWindow, SelectToolbarMenuHdl ) );
    mpDirectionSet = createEmptyValueSetControl();

    mpDirectionSet->SetSelectHdl( LINK( this, ExtrusionDirectionWindow, SelectValueSetHdl ) );
    mpDirectionSet->SetColCount( 3 );
    mpDirectionSet->EnableFullItemMode( false );

    for (sal_uInt16 i = DIRECTION_NW; i <= DIRECTION_SE; ++i)
    {
        OUString aText(SVX_RESSTR(RID_SVXSTR_DIRECTION + i));
        mpDirectionSet->InsertItem(i + 1, maImgDirection[i], aText);
    }

    mpDirectionSet->SetOutputSizePixel(Size(72, 72));

    appendEntry(2, mpDirectionSet );
    appendSeparator();
    appendEntry(0, SVX_RESSTR(RID_SVXSTR_PERSPECTIVE), maImgPerspective);
    appendEntry(1, SVX_RESSTR(RID_SVXSTR_PARALLEL), maImgParallel);

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( g_sExtrusionDirection );
    AddStatusListener( g_sExtrusionProjection );
}

ExtrusionDirectionWindow::~ExtrusionDirectionWindow()
{
    disposeOnce();
}

void ExtrusionDirectionWindow::dispose()
{
    mpDirectionSet.clear();
    ToolbarMenu::dispose();
}

void ExtrusionDirectionWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolbarMenu::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
    {
        for( sal_uInt16 i = DIRECTION_NW; i <= DIRECTION_SE; i++ )
        {
            mpDirectionSet->SetItemImage( i+1, maImgDirection[ i ] );
        }

        setEntryImage( 0, maImgPerspective );
        setEntryImage( 1, maImgParallel );
    }
}


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


void ExtrusionDirectionWindow::implSetProjection( sal_Int32 nProjection, bool bEnabled )
{
    checkEntry( 0, (nProjection == 0) && bEnabled );
    checkEntry( 1, (nProjection == 1 ) && bEnabled );
    enableEntry( 0, bEnabled );
    enableEntry( 1, bEnabled );
}


void ExtrusionDirectionWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)   throw ( css::uno::RuntimeException )
{
    if( Event.FeatureURL.Main == g_sExtrusionDirection )
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
    else if( Event.FeatureURL.Main == g_sExtrusionProjection )
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


IMPL_LINK_TYPED( ExtrusionDirectionWindow, SelectValueSetHdl, ValueSet*, pControl, void )
{
    SelectHdl(pControl);
}
IMPL_LINK_TYPED( ExtrusionDirectionWindow, SelectToolbarMenuHdl, ToolbarMenu*, pControl, void )
{
    SelectHdl(pControl);
}
void ExtrusionDirectionWindow::SelectHdl(void* pControl)
{
    if ( IsInPopupMode() )
        EndPopupMode();

    if( pControl == mpDirectionSet )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString(g_sExtrusionDirection).copy(5);
        aArgs[0].Value <<= (sal_Int32)gSkewList[mpDirectionSet->GetSelectItemId()-1];

        mrController.dispatchCommand( g_sExtrusionDirection, aArgs );
    }
    else
    {
        int nProjection = getSelectedEntryId();
        if( (nProjection >= 0) && (nProjection < 2 ) )
        {
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = OUString(g_sExtrusionProjection).copy(5);
            aArgs[0].Value <<= (sal_Int32)nProjection;

            mrController.dispatchCommand( g_sExtrusionProjection, aArgs );
            implSetProjection( nProjection, true );
        }
    }
}

ExtrusionDirectionControl::ExtrusionDirectionControl(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController(
        rxContext,
        Reference< css::frame::XFrame >(),
        OUString( ".uno:ExtrusionDirectionFloater" )
    )
{
}


VclPtr<vcl::Window> ExtrusionDirectionControl::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<ExtrusionDirectionWindow>::Create( *this, m_xFrame, pParent );
}

// XInitialization
void SAL_CALL ExtrusionDirectionControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
    throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString SAL_CALL ExtrusionDirectionControl_getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionDirectionController" );
}


Sequence< OUString > SAL_CALL ExtrusionDirectionControl_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


Reference< XInterface > SAL_CALL SAL_CALL ExtrusionDirectionControl_createInstance(
    const Reference< XMultiServiceFactory >& rSMgr
)   throw( RuntimeException )
{
    return *new ExtrusionDirectionControl( comphelper::getComponentContext(rSMgr) );
}


OUString SAL_CALL ExtrusionDirectionControl::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return ExtrusionDirectionControl_getImplementationName();
}


Sequence< OUString > SAL_CALL ExtrusionDirectionControl::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    return ExtrusionDirectionControl_getSupportedServiceNames();
}

ExtrusionDepthDialog::ExtrusionDepthDialog( vcl::Window* pParent, double fDepth, FieldUnit eDefaultUnit )
    : ModalDialog( pParent, "ExtrustionDepthDialog", "svx/ui/extrustiondepthdialog.ui" )
{
    get(m_pMtrDepth, "depth");
    m_pMtrDepth->SetUnit( eDefaultUnit );
    m_pMtrDepth->SetValue( (int) fDepth * 100, FUNIT_100TH_MM );
}

ExtrusionDepthDialog::~ExtrusionDepthDialog()
{
    disposeOnce();
}

void ExtrusionDepthDialog::dispose()
{
    m_pMtrDepth.clear();
    ModalDialog::dispose();
}

double ExtrusionDepthDialog::getDepth() const
{
    return (double)( m_pMtrDepth->GetValue( FUNIT_100TH_MM ) ) / 100.0;
}

double aDepthListInch[] = { 0, 1270,2540,5080,10160 };
double aDepthListMM[] = { 0, 1000, 2500, 5000, 10000 };

ExtrusionDepthWindow::ExtrusionDepthWindow(
    svt::ToolboxController& rController,
    const css::uno::Reference< css::frame::XFrame >& rFrame,
    vcl::Window* pParentWindow
)   : ToolbarMenu( rFrame, pParentWindow, WB_MOVEABLE|WB_CLOSEABLE|WB_HIDE|WB_3DLOOK)
    , mrController( rController )
    , maImgDepth0(SVX_RES(RID_SVXIMG_DEPTH_0))
    , maImgDepth1(SVX_RES(RID_SVXIMG_DEPTH_1))
    , maImgDepth2(SVX_RES(RID_SVXIMG_DEPTH_2))
    , maImgDepth3(SVX_RES(RID_SVXIMG_DEPTH_3))
    , maImgDepth4(SVX_RES(RID_SVXIMG_DEPTH_4))
    , maImgDepthInfinity(SVX_RES(RID_SVXIMG_DEPTH_INFINITY))
    , meUnit(FUNIT_NONE)
    , mfDepth( -1.0 )
    , msExtrusionDepth( ".uno:ExtrusionDepth" )
    , msMetricUnit(     ".uno:MetricUnit"     )
{
    SetSelectHdl( LINK( this, ExtrusionDepthWindow, SelectHdl ) );

    appendEntry(0, "", maImgDepth0);
    appendEntry(1, "", maImgDepth1);
    appendEntry(2, "", maImgDepth2);
    appendEntry(3, "", maImgDepth3);
    appendEntry(4, "", maImgDepth4);
    appendEntry(5, SVX_RESSTR(RID_SVXSTR_INFINITY), maImgDepthInfinity);
    appendEntry(6, SVX_RESSTR(RID_SVXSTR_CUSTOM));

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( msExtrusionDepth );
    AddStatusListener( msMetricUnit );
}

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


void ExtrusionDepthWindow::implFillStrings( FieldUnit eUnit )
{
    meUnit = eUnit;
    sal_uInt16 nResource = IsMetric( eUnit ) ? RID_SVXSTR_DEPTH_0 : RID_SVXSTR_DEPTH_0_INCH;

    for( int i = 0; i < 5; i++ )
    {
        OUString aStr( SVX_RESSTR( nResource + i ) );
        setEntryText( i, aStr );
    }
}


void ExtrusionDepthWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)   throw ( css::uno::RuntimeException, std::exception )
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

IMPL_LINK_NOARG_TYPED(ExtrusionDepthWindow, SelectHdl, ToolbarMenu*, void)
{
    int nSelected = getSelectedEntryId();
    if( nSelected != -1 )
    {
        if( nSelected == 6 )
        {
            if ( IsInPopupMode() )
                EndPopupMode();

            const OUString aCommand( ".uno:ExtrusionDepthDialog" );

            Sequence< PropertyValue > aArgs( 2 );
            aArgs[0].Name = "Depth";
            aArgs[0].Value <<= mfDepth;
            aArgs[1].Name = "Metric";
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
}


// ExtrusionDirectionControl


ExtrusionDepthController::ExtrusionDepthController(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController(
        rxContext,
        Reference< css::frame::XFrame >(),
        OUString( ".uno:ExtrusionDepthFloater" )
    )
{
}


VclPtr<vcl::Window> ExtrusionDepthController::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<ExtrusionDepthWindow>::Create( *this, m_xFrame, pParent );
}

// XInitialization
void SAL_CALL ExtrusionDepthController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
    throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString SAL_CALL ExtrusionDepthController_getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionDepthController" );
}


Sequence< OUString > SAL_CALL ExtrusionDepthController_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


Reference< XInterface > SAL_CALL SAL_CALL ExtrusionDepthController_createInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( RuntimeException )
{
    return *new ExtrusionDepthController( comphelper::getComponentContext(rSMgr) );
}

OUString SAL_CALL ExtrusionDepthController::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return ExtrusionDepthController_getImplementationName();
}

Sequence< OUString > SAL_CALL ExtrusionDepthController::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    return ExtrusionDepthController_getSupportedServiceNames();
}

static const char g_sExtrusionLightingDirection[] = ".uno:ExtrusionLightingDirection";
static const char g_sExtrusionLightingIntensity[] = ".uno:ExtrusionLightingIntensity";

ExtrusionLightingWindow::ExtrusionLightingWindow(svt::ToolboxController& rController,
                                                 const css::uno::Reference<css::frame::XFrame >& rFrame,
                                                 vcl::Window* pParentWindow)
    : ToolbarMenu(rFrame, pParentWindow, WB_MOVEABLE|WB_CLOSEABLE|WB_HIDE|WB_3DLOOK)
    , mrController(rController)
    , maImgBright(SVX_RES(RID_SVXIMG_LIGHTING_BRIGHT))
    , maImgNormal(SVX_RES(RID_SVXIMG_LIGHTING_NORMAL))
    , maImgDim(SVX_RES(RID_SVXIMG_LIGHTING_DIM))
    , mnLevel(0)
    , mbLevelEnabled(false)
    , mnDirection(FROM_FRONT)
    , mbDirectionEnabled(false)
{
    for (sal_uInt16 i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; ++i)
    {
        if( i != FROM_FRONT )
        {
            maImgLightingOff[i] = Image(SVX_RES(RID_SVXIMG_LIGHT_OFF + i));
            maImgLightingOn[i] = Image(SVX_RES(RID_SVXIMG_LIGHT_ON + i));
        }
        maImgLightingPreview[i] = Image(SVX_RES(RID_SVXIMG_LIGHT_PREVIEW + i));
    }

    SetSelectHdl( LINK( this, ExtrusionLightingWindow, SelectToolbarMenuHdl ) );

    mpLightingSet = createEmptyValueSetControl();
    mpLightingSet->SetHelpId( HID_VALUESET_EXTRUSION_LIGHTING );

    mpLightingSet->SetSelectHdl( LINK( this, ExtrusionLightingWindow, SelectValueSetHdl ) );
    mpLightingSet->SetColCount( 3 );
    mpLightingSet->EnableFullItemMode( false );

    for (sal_uInt16 i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; ++i)
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

    appendEntry(3, mpLightingSet);
    appendSeparator();
    appendEntry(0, SVX_RESSTR(RID_SVXSTR_BRIGHT), maImgBright);
    appendEntry(1, SVX_RESSTR(RID_SVXSTR_NORMAL), maImgNormal);
    appendEntry(2, SVX_RESSTR(RID_SVXSTR_DIM), maImgDim);

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( g_sExtrusionLightingDirection );
    AddStatusListener( g_sExtrusionLightingIntensity );
}

ExtrusionLightingWindow::~ExtrusionLightingWindow()
{
    disposeOnce();
}

void ExtrusionLightingWindow::dispose()
{
    mpLightingSet.clear();
    ToolbarMenu::dispose();
}

void ExtrusionLightingWindow::implSetIntensity( int nLevel, bool bEnabled )
{
    mnLevel = nLevel;
    mbLevelEnabled = bEnabled;
    for (int i = 0; i < 3; ++i)
    {
        checkEntry( i, (i == nLevel) && bEnabled );
        enableEntry( i, bEnabled );
    }
}

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


void ExtrusionLightingWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)   throw ( css::uno::RuntimeException )
{
    if( Event.FeatureURL.Main == g_sExtrusionLightingIntensity )
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
    else if( Event.FeatureURL.Main == g_sExtrusionLightingDirection )
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


void ExtrusionLightingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolbarMenu::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
    {
        implSetDirection( mnDirection, mbDirectionEnabled );
        setEntryImage( 0, maImgBright );
        setEntryImage( 1, maImgNormal );
        setEntryImage( 2, maImgDim    );
    }
}


IMPL_LINK_TYPED( ExtrusionLightingWindow, SelectValueSetHdl, ValueSet*, pControl, void )
{
    SelectHdl(pControl);
}
IMPL_LINK_TYPED( ExtrusionLightingWindow, SelectToolbarMenuHdl, ToolbarMenu*, pControl, void )
{
    SelectHdl(pControl);
}
void ExtrusionLightingWindow::SelectHdl(void* pControl)
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
                aArgs[0].Name = OUString(g_sExtrusionLightingIntensity).copy(5);
                aArgs[0].Value <<= (sal_Int32)nLevel;

                mrController.dispatchCommand( g_sExtrusionLightingIntensity, aArgs );

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
            aArgs[0].Name = OUString(g_sExtrusionLightingDirection).copy(5);
            aArgs[0].Value <<= (sal_Int32)nDirection;

            mrController.dispatchCommand( g_sExtrusionLightingDirection, aArgs );

            implSetDirection( nDirection, true );
        }

    }
}


ExtrusionLightingControl::ExtrusionLightingControl(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController( rxContext,
                Reference< css::frame::XFrame >(),
                OUString( ".uno:ExtrusionDirectionFloater" )
    )
{
}


VclPtr<vcl::Window> ExtrusionLightingControl::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<ExtrusionLightingWindow>::Create( *this, m_xFrame, pParent );
}

// XInitialization
void SAL_CALL ExtrusionLightingControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
    throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString SAL_CALL ExtrusionLightingControl_getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionLightingController" );
}


Sequence< OUString > SAL_CALL ExtrusionLightingControl_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


Reference< XInterface > SAL_CALL SAL_CALL ExtrusionLightingControl_createInstance(
    const Reference< XMultiServiceFactory >& rSMgr
)   throw( RuntimeException )
{
    return *new ExtrusionLightingControl( comphelper::getComponentContext(rSMgr) );
}


OUString SAL_CALL ExtrusionLightingControl::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return ExtrusionLightingControl_getImplementationName();
}


Sequence< OUString > SAL_CALL ExtrusionLightingControl::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    return ExtrusionLightingControl_getSupportedServiceNames();
}

static const char g_sExtrusionSurface[] = ".uno:ExtrusionSurface";

ExtrusionSurfaceWindow::ExtrusionSurfaceWindow(
    svt::ToolboxController& rController,
    const css::uno::Reference< css::frame::XFrame >& rFrame,
    vcl::Window* pParentWindow)
    : ToolbarMenu(rFrame, pParentWindow, WB_MOVEABLE|WB_CLOSEABLE|WB_HIDE|WB_3DLOOK)
    , mrController(rController)
    , maImgSurface1(SVX_RES(RID_SVXIMG_WIRE_FRAME))
    , maImgSurface2(SVX_RES(RID_SVXIMG_MATTE))
    , maImgSurface3(SVX_RES(RID_SVXIMG_PLASTIC))
    , maImgSurface4(SVX_RES(RID_SVXIMG_METAL))
{
    SetSelectHdl( LINK( this, ExtrusionSurfaceWindow, SelectHdl ) );

    appendEntry(0, SVX_RESSTR(RID_SVXSTR_WIREFRAME), maImgSurface1);
    appendEntry(1, SVX_RESSTR(RID_SVXSTR_MATTE), maImgSurface2);
    appendEntry(2, SVX_RESSTR(RID_SVXSTR_PLASTIC), maImgSurface3);
    appendEntry(3, SVX_RESSTR(RID_SVXSTR_METAL), maImgSurface4);

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( g_sExtrusionSurface );
}

void ExtrusionSurfaceWindow::implSetSurface( int nSurface, bool bEnabled )
{
    for(int i = 0; i < 4; ++i)
    {
        checkEntry( i, (i == nSurface) && bEnabled );
        enableEntry( i, bEnabled );
    }
}

void ExtrusionSurfaceWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)   throw ( css::uno::RuntimeException )
{
    if( Event.FeatureURL.Main == g_sExtrusionSurface )
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


IMPL_LINK_NOARG_TYPED(ExtrusionSurfaceWindow, SelectHdl, ToolbarMenu*, void)
{
    if ( IsInPopupMode() )
        EndPopupMode();

    sal_Int32 nSurface = getSelectedEntryId();
    if( nSurface >= 0 )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString(g_sExtrusionSurface).copy(5);
        aArgs[0].Value <<= (sal_Int32)nSurface;

        mrController.dispatchCommand( g_sExtrusionSurface, aArgs );

        implSetSurface( nSurface, true );
    }
}


ExtrusionSurfaceControl::ExtrusionSurfaceControl(
    const Reference< XComponentContext >& rxContext
)
:   svt::PopupWindowController(
        rxContext,
        Reference< css::frame::XFrame >(),
        OUString( ".uno:ExtrusionSurfaceFloater" )
    )
{
}


VclPtr<vcl::Window> ExtrusionSurfaceControl::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<ExtrusionSurfaceWindow>::Create( *this, m_xFrame, pParent );
}

// XInitialization
void SAL_CALL ExtrusionSurfaceControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
    throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString SAL_CALL ExtrusionSurfaceControl_getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ExtrusionSurfaceController" );
}


Sequence< OUString > SAL_CALL ExtrusionSurfaceControl_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}


Reference< XInterface > SAL_CALL SAL_CALL ExtrusionSurfaceControl_createInstance(
    const Reference< XMultiServiceFactory >& rSMgr
)   throw( RuntimeException )
{
    return *new ExtrusionSurfaceControl( comphelper::getComponentContext(rSMgr) );
}


OUString SAL_CALL ExtrusionSurfaceControl::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return ExtrusionSurfaceControl_getImplementationName();
}


Sequence< OUString > SAL_CALL ExtrusionSurfaceControl::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    return ExtrusionSurfaceControl_getSupportedServiceNames();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
