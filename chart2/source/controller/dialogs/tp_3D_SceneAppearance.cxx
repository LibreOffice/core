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
#include "ChartModelHelper.hxx"
#include "ThreeDHelper.hxx"
#include "macros.hxx"
#include <rtl/math.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

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

lcl_ModelProperties lcl_getPropertiesFromModel( uno::Reference< frame::XModel > & xModel )
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
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return aProps;
}

void lcl_setShadeModeAtModel( uno::Reference< frame::XModel > & xModel, drawing::ShadeMode aShadeMode )
{
    try
    {
        uno::Reference< beans::XPropertySet > xDiaProp(
            ::chart::ChartModelHelper::findDiagram( xModel ), uno::UNO_QUERY_THROW );
        xDiaProp->setPropertyValue( "D3DSceneShadeMode" , uno::makeAny( aShadeMode ));
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

} // anonymous namespace

namespace chart
{

#define POS_3DSCHEME_SIMPLE    0
#define POS_3DSCHEME_REALISTIC 1
#define POS_3DSCHEME_CUSTOM 2

ThreeD_SceneAppearance_TabPage::ThreeD_SceneAppearance_TabPage(
      Window* pWindow
    , const uno::Reference< frame::XModel > & xChartModel
    , ControllerLockHelper & rControllerLockHelper )
                : TabPage ( pWindow
                           , "tp_3D_SceneAppearance"
                           , "modules/schart/ui/tp_3D_SceneAppearance.ui")
                , m_xChartModel     ( xChartModel )
                , m_bUpdateOtherControls( true )
                , m_bCommitToModel( true )
                , m_rControllerLockHelper( rControllerLockHelper )
{

    get(m_pLB_Scheme, "LB_SCHEME");
    get(m_pCB_Shading, "CB_SHADING");
    get(m_pCB_ObjectLines, "CB_OBJECTLINES");
    get(m_pCB_RoundedEdge, "CB_ROUNDEDEDGE");

    m_aCustom = m_pLB_Scheme->GetEntry(POS_3DSCHEME_CUSTOM);
    m_pLB_Scheme->RemoveEntry(POS_3DSCHEME_CUSTOM);
    m_pLB_Scheme->SetDropDownLineCount(2);

    m_pLB_Scheme->SetSelectHdl( LINK( this, ThreeD_SceneAppearance_TabPage, SelectSchemeHdl ) );

    m_pCB_RoundedEdge->SetToggleHdl( LINK( this, ThreeD_SceneAppearance_TabPage, SelectRoundedEdgeOrObjectLines ) );
    m_pCB_Shading->SetToggleHdl( LINK( this, ThreeD_SceneAppearance_TabPage, SelectShading ) );
    m_pCB_ObjectLines->SetToggleHdl( LINK( this, ThreeD_SceneAppearance_TabPage, SelectRoundedEdgeOrObjectLines ) );

    m_pCB_RoundedEdge->EnableTriState( true );
    m_pCB_Shading->EnableTriState( true );
    m_pCB_ObjectLines->EnableTriState( true );

    initControlsFromModel();
}

ThreeD_SceneAppearance_TabPage::~ThreeD_SceneAppearance_TabPage()
{}

void ThreeD_SceneAppearance_TabPage::ActivatePage()
{
    updateScheme();
}

void ThreeD_SceneAppearance_TabPage::commitPendingChanges()
{
}

void ThreeD_SceneAppearance_TabPage::applyRoundedEdgeAndObjectLinesToModel()
{
    if(!m_bCommitToModel)
        return;

    sal_Int32 nObjectLines = -1;

    switch( m_pCB_ObjectLines->GetState())
    {
        case STATE_NOCHECK:
            nObjectLines = 0;
            break;
        case STATE_CHECK:
            nObjectLines = 1;
            break;
        case STATE_DONTKNOW:
            nObjectLines = -1;
            break;
    }

    sal_Int32 nCurrentRoundedEdges = -1;
    switch( m_pCB_RoundedEdge->GetState() )
    {
        case STATE_NOCHECK:
            nCurrentRoundedEdges = 0;
            break;
        case STATE_CHECK:
            nCurrentRoundedEdges = 5;
            break;
        case STATE_DONTKNOW:
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

    switch( m_pCB_Shading->GetState())
    {
        case STATE_NOCHECK:
            aShadeMode = drawing::ShadeMode_FLAT;
            break;
        case STATE_CHECK:
            aShadeMode = drawing::ShadeMode_SMOOTH;
            break;
        case STATE_DONTKNOW:
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
        m_pCB_Shading->EnableTriState( false );
        m_pCB_Shading->Check(false);
    }
    else if(aProps.m_aShadeMode == drawing::ShadeMode_SMOOTH)
    {
        m_pCB_Shading->EnableTriState( false );
        m_pCB_Shading->Check(true);
    }
    else
    {
        m_pCB_Shading->EnableTriState( true );
        m_pCB_Shading->SetState( STATE_DONTKNOW );
    }

    if(aProps.m_nObjectLines == 0)
    {
        m_pCB_ObjectLines->EnableTriState( false );
        m_pCB_ObjectLines->Check(false);
    }
    else if(aProps.m_nObjectLines==1)
    {
        m_pCB_ObjectLines->EnableTriState( false );
        m_pCB_ObjectLines->Check(true);
    }
    else
    {
        m_pCB_ObjectLines->EnableTriState( true );
        m_pCB_ObjectLines->SetState( STATE_DONTKNOW );
    }

    if(aProps.m_nRoundedEdges >= 5)
    {
        m_pCB_RoundedEdge->EnableTriState( false );
        m_pCB_RoundedEdge->Check(true);
    }
    else if(aProps.m_nRoundedEdges<0)
    {
        m_pCB_RoundedEdge->EnableTriState( false );
        m_pCB_RoundedEdge->SetState( STATE_DONTKNOW );
    }
    else
    {
        m_pCB_RoundedEdge->EnableTriState( true );
        m_pCB_RoundedEdge->Check(false);
    }
    m_pCB_RoundedEdge->Enable( !m_pCB_ObjectLines->IsChecked() );

    updateScheme();

    m_bCommitToModel = true;
    m_bUpdateOtherControls = true;
}

void ThreeD_SceneAppearance_TabPage::updateScheme()
{
    lcl_ModelProperties aProps( lcl_getPropertiesFromModel( m_xChartModel ));

    if( m_pLB_Scheme->GetEntryCount() == (POS_3DSCHEME_CUSTOM+1) )
    {
        m_pLB_Scheme->RemoveEntry(POS_3DSCHEME_CUSTOM);
        m_pLB_Scheme->SetDropDownLineCount(2);
    }
    switch( aProps.m_eScheme )
    {
        case ThreeDLookScheme_Simple:
            m_pLB_Scheme->SelectEntryPos( POS_3DSCHEME_SIMPLE );
            break;
        case ThreeDLookScheme_Realistic:
            m_pLB_Scheme->SelectEntryPos( POS_3DSCHEME_REALISTIC );
            break;
        case ThreeDLookScheme_Unknown:
            {
                m_pLB_Scheme->InsertEntry(m_aCustom,POS_3DSCHEME_CUSTOM);
                m_pLB_Scheme->SelectEntryPos( POS_3DSCHEME_CUSTOM );
                m_pLB_Scheme->SetDropDownLineCount(3);
            }
            break;
    }
}

IMPL_LINK_NOARG(ThreeD_SceneAppearance_TabPage, SelectSchemeHdl)
{
    if( !m_bUpdateOtherControls )
        return 0;

    {
        // locked controllers
        ControllerLockHelperGuard aGuard( m_rControllerLockHelper );

        uno::Reference< chart2::XDiagram > xDiagram( ::chart::ChartModelHelper::findDiagram( m_xChartModel ) );

        if( m_pLB_Scheme->GetSelectEntryPos() == POS_3DSCHEME_REALISTIC )
            ThreeDHelper::setScheme( xDiagram, ThreeDLookScheme_Realistic );
        else if( m_pLB_Scheme->GetSelectEntryPos() == POS_3DSCHEME_SIMPLE )
            ThreeDHelper::setScheme( xDiagram, ThreeDLookScheme_Simple );
        else
        {
            OSL_FAIL( "Invalid Entry selected" );
        }
    }

    // update other controls
    initControlsFromModel();
    return 0;
}

IMPL_LINK_NOARG(ThreeD_SceneAppearance_TabPage, SelectShading)
{
    if( !m_bUpdateOtherControls )
        return 0;

    m_pCB_Shading->EnableTriState( false );
    applyShadeModeToModel();
    updateScheme();
    return 0;
}
IMPL_LINK( ThreeD_SceneAppearance_TabPage, SelectRoundedEdgeOrObjectLines, CheckBox*, pCheckBox )
{
    if( !m_bUpdateOtherControls )
        return 0;

    if( pCheckBox == m_pCB_ObjectLines )
    {
        m_pCB_ObjectLines->EnableTriState( false );
        m_bUpdateOtherControls = false;
        m_pCB_RoundedEdge->Enable( !m_pCB_ObjectLines->IsChecked() );
        if(!m_pCB_RoundedEdge->IsEnabled())
            m_pCB_RoundedEdge->Check(false);
        m_bUpdateOtherControls = true;
    }
    else
        m_pCB_RoundedEdge->EnableTriState( false );
    applyRoundedEdgeAndObjectLinesToModel();
    updateScheme();
    return 0;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
