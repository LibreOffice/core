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

#include <sal/config.h>

#include <comphelper/propertyvalue.hxx>
#include <svtools/toolbarmenu.hxx>
#include <vcl/toolbox.hxx>

#include <svx/strings.hrc>
#include <svx/svdtrans.hxx>
#include <svx/dialmgr.hxx>

#include <helpids.h>
#include "extrusioncontrols.hxx"
#include <extrusiondepthdialog.hxx>

#include <bitmaps.hlst>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::graphic;

namespace svx
{

const sal_Int32 gSkewList[] = { 135, 90, 45, 180, 0, -360, -135, -90, -45 };
constexpr OUString g_sExtrusionDirection = u".uno:ExtrusionDirection"_ustr;
constexpr OUString g_sExtrusionProjection = u".uno:ExtrusionProjection"_ustr;
constexpr OUString EMPTY = u""_ustr;

constexpr OUString aLightOffBmps[] =
{
    RID_SVXBMP_LIGHT_OFF_FROM_TOP_LEFT,
    RID_SVXBMP_LIGHT_OFF_FROM_TOP,
    RID_SVXBMP_LIGHT_OFF_FROM_TOP_RIGHT,
    RID_SVXBMP_LIGHT_OFF_FROM_LEFT,
    EMPTY,
    RID_SVXBMP_LIGHT_OFF_FROM_RIGHT,
    RID_SVXBMP_LIGHT_OFF_FROM_BOTTOM_LEFT,
    RID_SVXBMP_LIGHT_OFF_FROM_BOTTOM,
    RID_SVXBMP_LIGHT_OFF_FROM_BOTTOM_RIGHT
};

constexpr OUString aLightOnBmps[] =
{
    RID_SVXBMP_LIGHT_ON_FROM_TOP_LEFT,
    RID_SVXBMP_LIGHT_ON_FROM_TOP,
    RID_SVXBMP_LIGHT_ON_FROM_TOP_RIGHT,
    RID_SVXBMP_LIGHT_ON_FROM_LEFT,
    EMPTY,
    RID_SVXBMP_LIGHT_ON_FROM_RIGHT,
    RID_SVXBMP_LIGHT_ON_FROM_BOTTOM_LEFT,
    RID_SVXBMP_LIGHT_ON_FROM_BOTTOM,
    RID_SVXBMP_LIGHT_ON_FROM_BOTTOM_RIGHT
};

constexpr OUString aLightPreviewBmps[] =
{
    RID_SVXBMP_LIGHT_PREVIEW_FROM_TOP_LEFT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_TOP,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_TOP_RIGHT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_LEFT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_FRONT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_RIGHT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_BOTTOM_LEFT,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_BOTTOM,
    RID_SVXBMP_LIGHT_PREVIEW_FROM_BOTTOM_RIGHT
};

constexpr OUString aDirectionBmps[] =
{
    RID_SVXBMP_DIRECTION_DIRECTION_NW,
    RID_SVXBMP_DIRECTION_DIRECTION_N,
    RID_SVXBMP_DIRECTION_DIRECTION_NE,
    RID_SVXBMP_DIRECTION_DIRECTION_W,
    RID_SVXBMP_DIRECTION_DIRECTION_NONE,
    RID_SVXBMP_DIRECTION_DIRECTION_E,
    RID_SVXBMP_DIRECTION_DIRECTION_SW,
    RID_SVXBMP_DIRECTION_DIRECTION_S,
    RID_SVXBMP_DIRECTION_DIRECTION_SE
};

constexpr TranslateId aDirectionStrs[] =
{
    RID_SVXSTR_DIRECTION_NW,
    RID_SVXSTR_DIRECTION_N,
    RID_SVXSTR_DIRECTION_NE,
    RID_SVXSTR_DIRECTION_W,
    RID_SVXSTR_DIRECTION_NONE,
    RID_SVXSTR_DIRECTION_E,
    RID_SVXSTR_DIRECTION_SW,
    RID_SVXSTR_DIRECTION_S,
    RID_SVXSTR_DIRECTION_SE
};

ExtrusionDirectionWindow::ExtrusionDirectionWindow(
    svt::PopupWindowController* pControl,
    weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"svx/ui/directionwindow.ui"_ustr, u"DirectionWindow"_ustr)
    , mxControl(pControl)
    , mxDirectionSet(new ValueSet(nullptr))
    , mxDirectionSetWin(new weld::CustomWeld(*m_xBuilder, u"valueset"_ustr, *mxDirectionSet))
    , mxPerspective(m_xBuilder->weld_radio_button(u"perspective"_ustr))
    , mxParallel(m_xBuilder->weld_radio_button(u"parallel"_ustr))
{
    mxDirectionSet->SetStyle(WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT);

    for (sal_uInt16 i = DIRECTION_NW; i <= DIRECTION_SE; ++i)
    {
        maImgDirection[i] = Image(StockImage::Yes, aDirectionBmps[i]);
    }

    mxDirectionSet->SetSelectHdl( LINK( this, ExtrusionDirectionWindow, SelectValueSetHdl ) );
    mxDirectionSet->SetColCount( 3 );
    mxDirectionSet->EnableFullItemMode( false );

    for (sal_uInt16 i = DIRECTION_NW; i <= DIRECTION_SE; ++i)
    {
        mxDirectionSet->InsertItem(i + 1, maImgDirection[i], SvxResId(aDirectionStrs[i]));
    }

    Size aSize(72, 72);
    mxDirectionSet->GetDrawingArea()->set_size_request(aSize.Width(), aSize.Height());
    mxDirectionSet->SetOutputSizePixel(aSize);

    mxPerspective->connect_toggled(LINK(this, ExtrusionDirectionWindow, SelectToolbarMenuHdl));

    AddStatusListener( g_sExtrusionDirection );
    AddStatusListener( g_sExtrusionProjection );
}

void ExtrusionDirectionWindow::GrabFocus()
{
    mxDirectionSet->GrabFocus();
}

ExtrusionDirectionWindow::~ExtrusionDirectionWindow()
{
}

void ExtrusionDirectionWindow::implSetDirection( sal_Int32 nSkew, bool bEnabled )
{
    sal_uInt16 nItemId;
    for( nItemId = DIRECTION_NW; nItemId <= DIRECTION_SE; nItemId++ )
    {
        if( gSkewList[nItemId] == nSkew )
            break;
    }

    if( nItemId <= DIRECTION_SE )
    {
        mxDirectionSet->SelectItem( nItemId+1 );
    }
    else
    {
        mxDirectionSet->SetNoSelection();
    }

    if (bEnabled)
        mxDirectionSet->Enable();
    else
        mxDirectionSet->Disable();
}

void ExtrusionDirectionWindow::implSetProjection( sal_Int32 nProjection, bool bEnabled )
{
    mxPerspective->set_active(nProjection == 0 && bEnabled);
    mxParallel->set_active(nProjection == 1 && bEnabled);
    mxPerspective->set_sensitive(bEnabled);
    mxParallel->set_sensitive(bEnabled);
}

void ExtrusionDirectionWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)
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

IMPL_LINK_NOARG(ExtrusionDirectionWindow, SelectValueSetHdl, ValueSet*, void)
{
    Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(
        g_sExtrusionDirection.copy(5),
        gSkewList[mxDirectionSet->GetSelectedItemId()-1]) };

    mxControl->dispatchCommand( g_sExtrusionDirection, aArgs );

    mxControl->EndPopupMode();
}

IMPL_LINK_NOARG(ExtrusionDirectionWindow, SelectToolbarMenuHdl, weld::Toggleable&, void)
{
    int nProjection = mxPerspective->get_active() ? 0 : 1;

    Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(
        g_sExtrusionProjection.copy(5), static_cast<sal_Int32>(nProjection)) };

    mxControl->dispatchCommand( g_sExtrusionProjection, aArgs );
    implSetProjection( nProjection, true );

    mxControl->EndPopupMode();
}

ExtrusionDirectionControl::ExtrusionDirectionControl(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController(
        rxContext,
        Reference< css::frame::XFrame >(),
        u".uno:ExtrusionDirectionFloater"_ustr
    )
{
}

std::unique_ptr<WeldToolbarPopup> ExtrusionDirectionControl::weldPopupWindow()
{
    return std::make_unique<ExtrusionDirectionWindow>(this, m_pToolbar);
}

VclPtr<vcl::Window> ExtrusionDirectionControl::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<ExtrusionDirectionWindow>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

// XInitialization
void SAL_CALL ExtrusionDirectionControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString ExtrusionDirectionControl::getImplementationName()
{
    return u"com.sun.star.comp.svx.ExtrusionDirectionController"_ustr;
}


Sequence< OUString > ExtrusionDirectionControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_ExtrusionDirectionControl_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ExtrusionDirectionControl(xContext));
}


ExtrusionDepthDialog::ExtrusionDepthDialog(weld::Window* pParent, double fDepth, FieldUnit eDefaultUnit)
    : GenericDialogController(pParent, u"svx/ui/extrustiondepthdialog.ui"_ustr, u"ExtrustionDepthDialog"_ustr)
    , m_xMtrDepth(m_xBuilder->weld_metric_spin_button(u"depth"_ustr, eDefaultUnit))
{
    m_xMtrDepth->set_value(static_cast<int>(fDepth) * 100, FieldUnit::MM_100TH);
}

ExtrusionDepthDialog::~ExtrusionDepthDialog()
{
}

double ExtrusionDepthDialog::getDepth() const
{
    return static_cast<double>(m_xMtrDepth->get_value(FieldUnit::MM_100TH)) / 100.0;
}

double const aDepthListInch[] = { 0, 1270,2540,5080,10160 };
double const aDepthListMM[] = { 0, 1000, 2500, 5000, 10000 };

constexpr OUString gsExtrusionDepth( u".uno:ExtrusionDepth"_ustr );
constexpr OUString gsMetricUnit(     u".uno:MetricUnit"_ustr     );

ExtrusionDepthWindow::ExtrusionDepthWindow(svt::PopupWindowController* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"svx/ui/depthwindow.ui"_ustr, u"DepthWindow"_ustr)
    , mxControl(pControl)
    , mxDepth0(m_xBuilder->weld_radio_button(u"depth0"_ustr))
    , mxDepth1(m_xBuilder->weld_radio_button(u"depth1"_ustr))
    , mxDepth2(m_xBuilder->weld_radio_button(u"depth2"_ustr))
    , mxDepth3(m_xBuilder->weld_radio_button(u"depth3"_ustr))
    , mxDepth4(m_xBuilder->weld_radio_button(u"depth4"_ustr))
    , mxInfinity(m_xBuilder->weld_radio_button(u"infinity"_ustr))
    , mxCustom(m_xBuilder->weld_radio_button(u"custom"_ustr))
    , meUnit(FieldUnit::NONE)
    , mfDepth( -1.0 )
    , mbSettingValue(false)
    , mbCommandDispatched(false)
{
    mxDepth0->connect_toggled(LINK(this, ExtrusionDepthWindow, SelectHdl));
    mxDepth1->connect_toggled(LINK(this, ExtrusionDepthWindow, SelectHdl));
    mxDepth2->connect_toggled(LINK(this, ExtrusionDepthWindow, SelectHdl));
    mxDepth3->connect_toggled(LINK(this, ExtrusionDepthWindow, SelectHdl));
    mxDepth4->connect_toggled(LINK(this, ExtrusionDepthWindow, SelectHdl));
    mxInfinity->connect_toggled(LINK(this, ExtrusionDepthWindow, SelectHdl));
    mxCustom->connect_toggled(LINK(this, ExtrusionDepthWindow, SelectHdl));
    mxCustom->connect_mouse_release(LINK(this, ExtrusionDepthWindow, MouseReleaseHdl));

    AddStatusListener( gsExtrusionDepth );
    AddStatusListener( gsMetricUnit );
}

void ExtrusionDepthWindow::GrabFocus()
{
    mxDepth0->grab_focus();
}

void ExtrusionDepthWindow::implSetDepth( double fDepth )
{
    mfDepth = fDepth;

    bool bSettingValue = mbSettingValue;
    mbSettingValue = true;

    mxCustom->set_active(true);
    bool bIsMetric = IsMetric(meUnit);
    mxDepth0->set_active(fDepth == (bIsMetric ? aDepthListMM[0] : aDepthListInch[0]));
    mxDepth1->set_active(fDepth == (bIsMetric ? aDepthListMM[1] : aDepthListInch[1]));
    mxDepth2->set_active(fDepth == (bIsMetric ? aDepthListMM[2] : aDepthListInch[2]));
    mxDepth3->set_active(fDepth == (bIsMetric ? aDepthListMM[3] : aDepthListInch[3]));
    mxDepth4->set_active(fDepth == (bIsMetric ? aDepthListMM[4] : aDepthListInch[4]));
    mxInfinity->set_active(fDepth >= 338666);

    mbSettingValue = bSettingValue;
}

void ExtrusionDepthWindow::implFillStrings( FieldUnit eUnit )
{
    meUnit = eUnit;

    const TranslateId aDepths[] =
    {
        RID_SVXSTR_DEPTH_0,
        RID_SVXSTR_DEPTH_1,
        RID_SVXSTR_DEPTH_2,
        RID_SVXSTR_DEPTH_3,
        RID_SVXSTR_DEPTH_4
    };

    const TranslateId aDepthsInch[] =
    {
        RID_SVXSTR_DEPTH_0_INCH,
        RID_SVXSTR_DEPTH_1_INCH,
        RID_SVXSTR_DEPTH_2_INCH,
        RID_SVXSTR_DEPTH_3_INCH,
        RID_SVXSTR_DEPTH_4_INCH
    };

    static_assert(SAL_N_ELEMENTS(aDepths) == SAL_N_ELEMENTS(aDepthsInch));

    const TranslateId* pResource = IsMetric(eUnit) ? aDepths : aDepthsInch;

    mxDepth0->set_label(SvxResId(pResource[0]));
    mxDepth1->set_label(SvxResId(pResource[1]));
    mxDepth2->set_label(SvxResId(pResource[2]));
    mxDepth3->set_label(SvxResId(pResource[3]));
    mxDepth4->set_label(SvxResId(pResource[4]));
}

void ExtrusionDepthWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)
{
    if( Event.FeatureURL.Main == gsExtrusionDepth )
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
    else if( Event.FeatureURL.Main == gsMetricUnit )
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

void ExtrusionDepthWindow::DispatchDepthDialog()
{
    Sequence< PropertyValue > aArgs{
        comphelper::makePropertyValue(u"Depth"_ustr, mfDepth),
        comphelper::makePropertyValue(u"Metric"_ustr, static_cast<sal_Int32>( meUnit ))
    };

    rtl::Reference<svt::PopupWindowController> xControl(mxControl);
    xControl->EndPopupMode();
    xControl->dispatchCommand(u".uno:ExtrusionDepthDialog"_ustr, aArgs);
    mbCommandDispatched = true;
}

IMPL_LINK(ExtrusionDepthWindow, SelectHdl, weld::Toggleable&, rButton, void)
{
    if (mbSettingValue || !rButton.get_active())
        return;

    // see MouseReleaseHdl for mbCommandDispatched check, there's no guarantee
    // this toggle will happen before that mouse release though it does in
    // practice for vcl and gtk
    if (mbCommandDispatched)
        return;

    if (mxCustom->get_active())
        DispatchDepthDialog();
    else
    {
        double fDepth;

        if (mxInfinity->get_active())
        {
            fDepth = 338666.6;
        }
        else
        {
            int nSelected;
            if (mxDepth0->get_active())
                nSelected = 0;
            else if (mxDepth1->get_active())
                nSelected = 1;
            else if (mxDepth2->get_active())
                nSelected = 2;
            else if (mxDepth3->get_active())
                nSelected = 3;
            else
                nSelected = 4;

            fDepth = IsMetric( meUnit ) ? aDepthListMM[nSelected] : aDepthListInch[nSelected];
        }

        Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(
            gsExtrusionDepth.copy(5), fDepth) };

        mxControl->dispatchCommand( gsExtrusionDepth,  aArgs );
        mbCommandDispatched = true;
        implSetDepth( fDepth );

        mxControl->EndPopupMode();
    }
}

IMPL_LINK_NOARG(ExtrusionDepthWindow, MouseReleaseHdl, const MouseEvent&, bool)
{
    /*
     tdf#145296 if the "custom" radiobutton was presented preselected as
     toggled on and the user clicked on it then there's no toggled signal sent
     because the item was already toggled on and didn't change state.

     So if that happens launch the custom spacing dialog explicitly here on
     mouse release.
    */
    if (mxCustom->get_active() && !mbCommandDispatched)
    {
        DispatchDepthDialog();
        return true;
    }
    return false;
}

// ExtrusionDirectionControl
ExtrusionDepthController::ExtrusionDepthController(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController(
        rxContext,
        Reference< css::frame::XFrame >(),
        u".uno:ExtrusionDepthFloater"_ustr
    )
{
}

std::unique_ptr<WeldToolbarPopup> ExtrusionDepthController::weldPopupWindow()
{
    return std::make_unique<ExtrusionDepthWindow>(this, m_pToolbar);
}

VclPtr<vcl::Window> ExtrusionDepthController::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<ExtrusionDepthWindow>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

// XInitialization
void SAL_CALL ExtrusionDepthController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString ExtrusionDepthController::getImplementationName()
{
    return u"com.sun.star.comp.svx.ExtrusionDepthController"_ustr;
}


Sequence< OUString > ExtrusionDepthController::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_ExtrusionDepthController_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ExtrusionDepthController(xContext));
}


constexpr OUString g_sExtrusionLightingDirection = u".uno:ExtrusionLightingDirection"_ustr;
constexpr OUString g_sExtrusionLightingIntensity = u".uno:ExtrusionLightingIntensity"_ustr;

ExtrusionLightingWindow::ExtrusionLightingWindow(svt::PopupWindowController* pControl,
                                                 weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"svx/ui/lightingwindow.ui"_ustr, u"LightingWindow"_ustr)
    , mxControl(pControl)
    , mxLightingSet(new ValueSet(nullptr))
    , mxLightingSetWin(new weld::CustomWeld(*m_xBuilder, u"valueset"_ustr, *mxLightingSet))
    , mxBright(m_xBuilder->weld_radio_button(u"bright"_ustr))
    , mxNormal(m_xBuilder->weld_radio_button(u"normal"_ustr))
    , mxDim(m_xBuilder->weld_radio_button(u"dim"_ustr))
{
    mxLightingSet->SetStyle(WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT);

    for (sal_uInt16 i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; ++i)
    {
        if( i != FROM_FRONT )
        {
            maImgLightingOff[i] = Image(StockImage::Yes, aLightOffBmps[i]);
            maImgLightingOn[i] = Image(StockImage::Yes, aLightOnBmps[i]);
        }
        maImgLightingPreview[i] = Image(StockImage::Yes, aLightPreviewBmps[i]);
    }

    mxLightingSet->SetHelpId( HID_VALUESET_EXTRUSION_LIGHTING );

    mxLightingSet->SetSelectHdl( LINK( this, ExtrusionLightingWindow, SelectValueSetHdl ) );
    mxLightingSet->SetColCount( 3 );
    mxLightingSet->EnableFullItemMode( false );

    for (sal_uInt16 i = FROM_TOP_LEFT; i <= FROM_BOTTOM_RIGHT; ++i)
    {
        if( i != FROM_FRONT )
        {
            mxLightingSet->InsertItem( i+1, maImgLightingOff[i] );
        }
        else
        {
            mxLightingSet->InsertItem( 5, maImgLightingPreview[FROM_FRONT] );
        }
    }
    Size aSize(72, 72);
    mxLightingSet->GetDrawingArea()->set_size_request(aSize.Width(), aSize.Height());
    mxLightingSet->SetOutputSizePixel(aSize);

    mxBright->connect_toggled(LINK(this, ExtrusionLightingWindow, SelectToolbarMenuHdl));
    mxNormal->connect_toggled(LINK(this, ExtrusionLightingWindow, SelectToolbarMenuHdl));
    mxDim->connect_toggled(LINK(this, ExtrusionLightingWindow, SelectToolbarMenuHdl));

    AddStatusListener( g_sExtrusionLightingDirection );
    AddStatusListener( g_sExtrusionLightingIntensity );
}

void ExtrusionLightingWindow::GrabFocus()
{
    mxLightingSet->GrabFocus();
}

ExtrusionLightingWindow::~ExtrusionLightingWindow()
{
}

void ExtrusionLightingWindow::implSetIntensity( int nLevel, bool bEnabled )
{
    mxBright->set_sensitive(bEnabled);
    mxBright->set_active(nLevel == 0 && bEnabled);
    mxNormal->set_sensitive(bEnabled);
    mxNormal->set_active(nLevel == 1 && bEnabled);
    mxDim->set_sensitive(bEnabled);
    mxDim->set_active(nLevel == 2 && bEnabled);
}

void ExtrusionLightingWindow::implSetDirection( int nDirection, bool bEnabled )
{
    if( !bEnabled )
        nDirection = FROM_FRONT;

    sal_uInt16 nItemId;
    for( nItemId = FROM_TOP_LEFT; nItemId <= FROM_BOTTOM_RIGHT; nItemId++ )
    {
        if( nItemId == FROM_FRONT )
        {
            mxLightingSet->SetItemImage( nItemId + 1, maImgLightingPreview[ nDirection ] );
        }
        else
        {
            mxLightingSet->SetItemImage(
                nItemId + 1,
                static_cast<sal_uInt16>(nDirection) == nItemId ? maImgLightingOn[nItemId] : maImgLightingOff[nItemId]
            );
        }
    }

    if (bEnabled)
        mxLightingSet->Enable();
    else
        mxLightingSet->Disable();
}

void ExtrusionLightingWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)
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

IMPL_LINK_NOARG(ExtrusionLightingWindow, SelectValueSetHdl, ValueSet*, void)
{
    sal_Int32 nDirection = mxLightingSet->GetSelectedItemId();

    if( (nDirection > 0) && (nDirection < 10) )
    {
        nDirection--;

        Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(
            g_sExtrusionLightingDirection.copy(5), nDirection) };

        mxControl->dispatchCommand( g_sExtrusionLightingDirection, aArgs );

        implSetDirection( nDirection, true );
    }

    mxControl->EndPopupMode();
}

IMPL_LINK(ExtrusionLightingWindow, SelectToolbarMenuHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;

    int nLevel;
    if (mxBright->get_active())
        nLevel = 0;
    else if (mxNormal->get_active())
        nLevel = 1;
    else
        nLevel = 2;

    Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(
        g_sExtrusionLightingIntensity.copy(5), static_cast<sal_Int32>(nLevel)) };

    mxControl->dispatchCommand( g_sExtrusionLightingIntensity, aArgs );

    implSetIntensity( nLevel, true );

    mxControl->EndPopupMode();
}

ExtrusionLightingControl::ExtrusionLightingControl(
    const Reference< XComponentContext >& rxContext
)   : svt::PopupWindowController( rxContext,
                Reference< css::frame::XFrame >(),
                u".uno:ExtrusionDirectionFloater"_ustr
    )
{
}

std::unique_ptr<WeldToolbarPopup> ExtrusionLightingControl::weldPopupWindow()
{
    return std::make_unique<ExtrusionLightingWindow>(this, m_pToolbar);
}

VclPtr<vcl::Window> ExtrusionLightingControl::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<ExtrusionLightingWindow>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

// XInitialization
void SAL_CALL ExtrusionLightingControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString ExtrusionLightingControl::getImplementationName()
{
    return u"com.sun.star.comp.svx.ExtrusionLightingController"_ustr;
}


Sequence< OUString > ExtrusionLightingControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_ExtrusionLightingControl_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ExtrusionLightingControl(xContext));
}


constexpr OUString g_sExtrusionSurface = u".uno:ExtrusionSurface"_ustr;

ExtrusionSurfaceWindow::ExtrusionSurfaceWindow(svt::PopupWindowController* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"svx/ui/surfacewindow.ui"_ustr, u"SurfaceWindow"_ustr)
    , mxControl(pControl)
    , mxWireFrame(m_xBuilder->weld_radio_button(u"wireframe"_ustr))
    , mxMatt(m_xBuilder->weld_radio_button(u"matt"_ustr))
    , mxPlastic(m_xBuilder->weld_radio_button(u"plastic"_ustr))
    , mxMetal(m_xBuilder->weld_radio_button(u"metal"_ustr))
    , mxMetalMSO(m_xBuilder->weld_radio_button(u"metalMSO"_ustr))
{
    mxWireFrame->connect_toggled(LINK(this, ExtrusionSurfaceWindow, SelectHdl));
    mxMatt->connect_toggled(LINK(this, ExtrusionSurfaceWindow, SelectHdl));
    mxPlastic->connect_toggled(LINK(this, ExtrusionSurfaceWindow, SelectHdl));
    mxMetal->connect_toggled(LINK(this, ExtrusionSurfaceWindow, SelectHdl));
    mxMetalMSO->connect_toggled(LINK(this, ExtrusionSurfaceWindow, SelectHdl));

    AddStatusListener( g_sExtrusionSurface );
}

void ExtrusionSurfaceWindow::GrabFocus()
{
    mxWireFrame->grab_focus();
}

void ExtrusionSurfaceWindow::implSetSurface( int nSurface, bool bEnabled )
{
    mxWireFrame->set_active(nSurface == 0 && bEnabled);
    mxWireFrame->set_sensitive(bEnabled);
    mxMatt->set_active(nSurface == 1 && bEnabled);
    mxMatt->set_sensitive(bEnabled);
    mxPlastic->set_active(nSurface == 2 && bEnabled);
    mxPlastic->set_sensitive(bEnabled);
    mxMetal->set_active(nSurface == 3 && bEnabled);
    mxMetal->set_sensitive(bEnabled);
    mxMetalMSO->set_active(nSurface == 4 && bEnabled);
    mxMetalMSO->set_sensitive(bEnabled);
}

void ExtrusionSurfaceWindow::statusChanged(
    const css::frame::FeatureStateEvent& Event
)
{
    if( Event.FeatureURL.Main != g_sExtrusionSurface )
        return;

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

IMPL_LINK(ExtrusionSurfaceWindow, SelectHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;

    sal_Int32 nSurface;
    if (mxWireFrame->get_active())
        nSurface = 0;
    else if (mxMatt->get_active())
        nSurface = 1;
    else if (mxPlastic->get_active())
        nSurface = 2;
    else if (mxMetal->get_active())
        nSurface = 3;
    else
        nSurface = 4;

    Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(
        g_sExtrusionSurface.copy(5), nSurface) };

    mxControl->dispatchCommand( g_sExtrusionSurface, aArgs );

    implSetSurface( nSurface, true );

    mxControl->EndPopupMode();
}

ExtrusionSurfaceControl::ExtrusionSurfaceControl(
    const Reference< XComponentContext >& rxContext
)
:   svt::PopupWindowController(
        rxContext,
        Reference< css::frame::XFrame >(),
        u".uno:ExtrusionSurfaceFloater"_ustr
    )
{
}

std::unique_ptr<WeldToolbarPopup> ExtrusionSurfaceControl::weldPopupWindow()
{
    return std::make_unique<ExtrusionSurfaceWindow>(this, m_pToolbar);
}

VclPtr<vcl::Window> ExtrusionSurfaceControl::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<ExtrusionSurfaceWindow>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

// XInitialization
void SAL_CALL ExtrusionSurfaceControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

// XServiceInfo


OUString ExtrusionSurfaceControl::getImplementationName()
{
    return u"com.sun.star.comp.svx.ExtrusionSurfaceController"_ustr;
}


Sequence< OUString > ExtrusionSurfaceControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_ExtrusionSurfaceControl_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ExtrusionSurfaceControl(xContext));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
