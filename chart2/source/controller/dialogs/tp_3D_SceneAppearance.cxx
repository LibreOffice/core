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

#include "tp_3D_SceneAppearance.hxx"
#include <ChartModelHelper.hxx>
#include <ThreeDHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

namespace
{

struct lcl_ModelProperties
{
    drawing::ShadeMode        m_aShadeMode;
    sal_Int32                 m_nRoundedEdges;
    sal_Int32                 m_nObjectLines;
    ::chart::ThreeDLookScheme m_eScheme;

    lcl_ModelProperties()
        : m_aShadeMode(drawing::ShadeMode_FLAT)
        , m_nRoundedEdges(-1)
        , m_nObjectLines(-1)
        , m_eScheme(::chart::ThreeDLookScheme_Unknown)
    {}
};

lcl_ModelProperties lcl_getPropertiesFromModel( uno::Reference< frame::XModel > const & xModel )
{
    lcl_ModelProperties aProps;
    try
    {
        uno::Reference< chart2::XDiagram > xDiagram( ::chart::ChartModelHelper::findDiagram( xModel ) );
        uno::Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY_THROW );
        xDiaProp->getPropertyValue( "D3DSceneShadeMode" ) >>= aProps.m_aShadeMode;
        ::chart::ThreeDHelper::getRoundedEdgesAndObjectLines( xDiagram, aProps.m_nRoundedEdges, aProps.m_nObjectLines );
        aProps.m_eScheme = ::chart::ThreeDHelper::detectScheme( xDiagram );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return aProps;
}

void lcl_setShadeModeAtModel( uno::Reference< frame::XModel > const & xModel, drawing::ShadeMode aShadeMode )
{
    try
    {
        uno::Reference< beans::XPropertySet > xDiaProp(
            ::chart::ChartModelHelper::findDiagram( xModel ), uno::UNO_QUERY_THROW );
        xDiaProp->setPropertyValue( "D3DSceneShadeMode" , uno::Any( aShadeMode ));
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

} // anonymous namespace

namespace chart
{

#define POS_3DSCHEME_SIMPLE    0
#define POS_3DSCHEME_REALISTIC 1
#define POS_3DSCHEME_CUSTOM 2

ThreeD_SceneAppearance_TabPage::ThreeD_SceneAppearance_TabPage(weld::Container* pParent,
        const uno::Reference<frame::XModel>& xChartModel,
        ControllerLockHelper& rControllerLockHelper)
    : m_xChartModel(xChartModel)
    , m_bUpdateOtherControls(true)
    , m_bCommitToModel(true)
    , m_rControllerLockHelper(rControllerLockHelper)
    , m_xBuilder(Application::CreateBuilder(pParent, "modules/schart/ui/tp_3D_SceneAppearance.ui"))
    , m_xContainer(m_xBuilder->weld_container("tp_3D_SceneAppearance"))
    , m_xLB_Scheme(m_xBuilder->weld_combo_box("LB_SCHEME"))
    , m_xCB_Shading(m_xBuilder->weld_check_button("CB_SHADING"))
    , m_xCB_ObjectLines(m_xBuilder->weld_check_button("CB_OBJECTLINES"))
    , m_xCB_RoundedEdge(m_xBuilder->weld_check_button("CB_ROUNDEDEDGE"))
{
    m_aCustom = m_xLB_Scheme->get_text(POS_3DSCHEME_CUSTOM);
    m_xLB_Scheme->remove(POS_3DSCHEME_CUSTOM);

    m_xLB_Scheme->connect_changed( LINK( this, ThreeD_SceneAppearance_TabPage, SelectSchemeHdl ) );

    m_xCB_RoundedEdge->connect_toggled( LINK( this, ThreeD_SceneAppearance_TabPage, SelectRoundedEdgeOrObjectLines ) );
    m_xCB_Shading->connect_toggled( LINK( this, ThreeD_SceneAppearance_TabPage, SelectShading ) );
    m_xCB_ObjectLines->connect_toggled( LINK( this, ThreeD_SceneAppearance_TabPage, SelectRoundedEdgeOrObjectLines ) );

    initControlsFromModel();
}

ThreeD_SceneAppearance_TabPage::~ThreeD_SceneAppearance_TabPage()
{
}

void ThreeD_SceneAppearance_TabPage::ActivatePage()
{
    updateScheme();
}

void ThreeD_SceneAppearance_TabPage::applyRoundedEdgeAndObjectLinesToModel()
{
    if(!m_bCommitToModel)
        return;

    sal_Int32 nObjectLines = -1;

    switch( m_xCB_ObjectLines->get_state())
    {
        case TRISTATE_FALSE:
            nObjectLines = 0;
            break;
        case TRISTATE_TRUE:
            nObjectLines = 1;
            break;
        case TRISTATE_INDET:
            nObjectLines = -1;
            break;
    }

    sal_Int32 nCurrentRoundedEdges = -1;
    switch( m_xCB_RoundedEdge->get_state() )
    {
        case TRISTATE_FALSE:
            nCurrentRoundedEdges = 0;
            break;
        case TRISTATE_TRUE:
            nCurrentRoundedEdges = 5;
            break;
        case TRISTATE_INDET:
            nCurrentRoundedEdges = -1;
            break;
    }

    // locked controllers
    ControllerLockHelperGuard aGuard( m_rControllerLockHelper );
    ThreeDHelper::setRoundedEdgesAndObjectLines(
        ::chart::ChartModelHelper::findDiagram( m_xChartModel ), nCurrentRoundedEdges, nObjectLines );
}

void ThreeD_SceneAppearance_TabPage::applyShadeModeToModel()
{
    if(!m_bCommitToModel)
        return;

    drawing::ShadeMode aShadeMode = drawing::ShadeMode_PHONG;

    switch( m_xCB_Shading->get_state())
    {
        case TRISTATE_FALSE:
            aShadeMode = drawing::ShadeMode_FLAT;
            break;
        case TRISTATE_TRUE:
            aShadeMode = drawing::ShadeMode_SMOOTH;
            break;
        case TRISTATE_INDET:
            // nothing
            break;
    }

    lcl_setShadeModeAtModel( m_xChartModel, aShadeMode );
}

void ThreeD_SceneAppearance_TabPage::initControlsFromModel()
{
    m_bCommitToModel = false;
    m_bUpdateOtherControls = false;

    lcl_ModelProperties aProps( lcl_getPropertiesFromModel( m_xChartModel ));

    if(aProps.m_aShadeMode == drawing::ShadeMode_FLAT)
    {
        m_xCB_Shading->set_active(false);
    }
    else if(aProps.m_aShadeMode == drawing::ShadeMode_SMOOTH)
    {
        m_xCB_Shading->set_active(true);
    }
    else
    {
        m_xCB_Shading->set_state(TRISTATE_INDET);
    }

    if(aProps.m_nObjectLines == 0)
    {
        m_xCB_ObjectLines->set_active(false);
    }
    else if(aProps.m_nObjectLines==1)
    {
        m_xCB_ObjectLines->set_active(true);
    }
    else
    {
        m_xCB_ObjectLines->set_state(TRISTATE_INDET);
    }

    if(aProps.m_nRoundedEdges >= 5)
    {
        m_xCB_RoundedEdge->set_active(true);
    }
    else if(aProps.m_nRoundedEdges<0)
    {
        m_xCB_RoundedEdge->set_state(TRISTATE_INDET);
    }
    else
    {
        m_xCB_RoundedEdge->set_active(false);
    }
    m_xCB_RoundedEdge->set_sensitive( !m_xCB_ObjectLines->get_active() );

    updateScheme();

    m_bCommitToModel = true;
    m_bUpdateOtherControls = true;
}

void ThreeD_SceneAppearance_TabPage::updateScheme()
{
    lcl_ModelProperties aProps( lcl_getPropertiesFromModel( m_xChartModel ));

    if (m_xLB_Scheme->get_count() == (POS_3DSCHEME_CUSTOM+1))
        m_xLB_Scheme->remove(POS_3DSCHEME_CUSTOM);
    switch( aProps.m_eScheme )
    {
        case ThreeDLookScheme_Simple:
            m_xLB_Scheme->set_active( POS_3DSCHEME_SIMPLE );
            break;
        case ThreeDLookScheme_Realistic:
            m_xLB_Scheme->set_active( POS_3DSCHEME_REALISTIC );
            break;
        case ThreeDLookScheme_Unknown:
            {
                m_xLB_Scheme->insert_text(POS_3DSCHEME_CUSTOM, m_aCustom);
                m_xLB_Scheme->set_active(POS_3DSCHEME_CUSTOM);
            }
            break;
    }
}

IMPL_LINK_NOARG(ThreeD_SceneAppearance_TabPage, SelectSchemeHdl, weld::ComboBox&, void)
{
    if( !m_bUpdateOtherControls )
        return;

    {
        // locked controllers
        ControllerLockHelperGuard aGuard( m_rControllerLockHelper );

        uno::Reference< chart2::XDiagram > xDiagram( ::chart::ChartModelHelper::findDiagram( m_xChartModel ) );

        if( m_xLB_Scheme->get_active() == POS_3DSCHEME_REALISTIC )
            ThreeDHelper::setScheme( xDiagram, ThreeDLookScheme_Realistic );
        else if( m_xLB_Scheme->get_active() == POS_3DSCHEME_SIMPLE )
            ThreeDHelper::setScheme( xDiagram, ThreeDLookScheme_Simple );
        else
        {
            OSL_FAIL( "Invalid Entry selected" );
        }
    }

    // update other controls
    initControlsFromModel();
}

IMPL_LINK_NOARG(ThreeD_SceneAppearance_TabPage, SelectShading, weld::ToggleButton&, void)
{
    if( !m_bUpdateOtherControls )
        return;

    applyShadeModeToModel();
    updateScheme();
}

IMPL_LINK(ThreeD_SceneAppearance_TabPage, SelectRoundedEdgeOrObjectLines, weld::ToggleButton&, rCheckBox, void)
{
    if( !m_bUpdateOtherControls )
        return;

    if (&rCheckBox == m_xCB_ObjectLines.get())
    {
        m_bUpdateOtherControls = false;
        m_xCB_RoundedEdge->set_sensitive( !m_xCB_ObjectLines->get_active() );
        if(!m_xCB_RoundedEdge->get_sensitive())
            m_xCB_RoundedEdge->set_active(false);
        m_bUpdateOtherControls = true;
    }

    applyRoundedEdgeAndObjectLinesToModel();
    updateScheme();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
