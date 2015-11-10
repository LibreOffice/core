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

#include "tp_AxisLabel.hxx"

#include "ResId.hxx"
#include "ResourceIds.hrc"
#include "chartview/ChartSfxItemIds.hxx"

#include <svx/chrtitem.hxx>
#include <svl/intitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

namespace chart
{

SchAxisLabelTabPage::SchAxisLabelTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs ) :
        SfxTabPage( pParent, "AxisLabelTabPage","modules/schart/ui/tp_axisLabel.ui", &rInAttrs ),

        m_bShowStaggeringControls( true ),

        m_nInitialDegrees( 0 ),
        m_bHasInitialDegrees( true ),
        m_bInitialStacking( false ),
        m_bHasInitialStacking( true ),
        m_bComplexCategories( false )
{
    get(m_pCbShowDescription, "showlabelsCB");
    get(m_pFlOrder, "orderL");
    get(m_pRbSideBySide, "tile");
    get(m_pRbUpDown, "odd");
    get(m_pRbDownUp, "even");
    get(m_pRbAuto, "auto");
    get(m_pFlTextFlow, "textflowL");
    get(m_pCbTextOverlap, "overlapCB");
    get(m_pCbTextBreak, "breakCB");
    get(m_pFlOrient, "labelTextOrient");
    get(m_pCtrlDial,"dialCtrl");
    get(m_pFtRotate,"degreeL");
    get(m_pNfRotate,"OrientDegree");
    get(m_pCbStacked,"stackedCB");
    get(m_pFtTextDirection,"textdirL");
    get(m_pLbTextDirection,"textdirLB");
    get(m_pFtABCD,"labelABCD");
    m_pCtrlDial->SetText(m_pFtABCD->GetText());
    m_pOrientHlp = new svx::OrientationHelper(*m_pCtrlDial, *m_pNfRotate, *m_pCbStacked);
    m_pOrientHlp->Enable();

    m_pCbStacked->EnableTriState( false );
    m_pOrientHlp->AddDependentWindow( *m_pFlOrient );
    m_pOrientHlp->AddDependentWindow( *m_pFtRotate, TRISTATE_TRUE );

    m_pCbShowDescription->SetClickHdl( LINK( this, SchAxisLabelTabPage, ToggleShowLabel ) );
}

SchAxisLabelTabPage::~SchAxisLabelTabPage()
{
    disposeOnce();
}

void SchAxisLabelTabPage::dispose()
{
    delete m_pOrientHlp;
    m_pOrientHlp = nullptr;
    m_pCbShowDescription.clear();
    m_pFlOrder.clear();
    m_pRbSideBySide.clear();
    m_pRbUpDown.clear();
    m_pRbDownUp.clear();
    m_pRbAuto.clear();
    m_pFlTextFlow.clear();
    m_pCbTextOverlap.clear();
    m_pCbTextBreak.clear();
    m_pFtABCD.clear();
    m_pFlOrient.clear();
    m_pCtrlDial.clear();
    m_pFtRotate.clear();
    m_pNfRotate.clear();
    m_pCbStacked.clear();
    m_pFtTextDirection.clear();
    m_pLbTextDirection.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SchAxisLabelTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrs )
{
    return VclPtr<SchAxisLabelTabPage>::Create( pParent, *rAttrs );
}

bool SchAxisLabelTabPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    bool bStacked = false;
    if( m_pOrientHlp->GetStackedState() != TRISTATE_INDET )
    {
        bStacked = m_pOrientHlp->GetStackedState() == TRISTATE_TRUE;
        if( !m_bHasInitialStacking || (bStacked != m_bInitialStacking) )
            rOutAttrs->Put( SfxBoolItem( SCHATTR_TEXT_STACKED, bStacked ) );
    }

    if( m_pCtrlDial->HasRotation() )
    {
        sal_Int32 nDegrees = bStacked ? 0 : m_pCtrlDial->GetRotation();
        if( !m_bHasInitialDegrees || (nDegrees != m_nInitialDegrees) )
            rOutAttrs->Put( SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );
    }

    if( m_bShowStaggeringControls )
    {
        SvxChartTextOrder eOrder = CHTXTORDER_SIDEBYSIDE;
        bool bRadioButtonChecked = true;

        if( m_pRbUpDown->IsChecked())
            eOrder = CHTXTORDER_UPDOWN;
        else if( m_pRbDownUp->IsChecked())
            eOrder = CHTXTORDER_DOWNUP;
        else if( m_pRbAuto->IsChecked())
            eOrder = CHTXTORDER_AUTO;
        else if( m_pRbSideBySide->IsChecked())
            eOrder = CHTXTORDER_SIDEBYSIDE;
        else
            bRadioButtonChecked = false;

        if( bRadioButtonChecked )
            rOutAttrs->Put( SvxChartTextOrderItem( eOrder, SCHATTR_AXIS_LABEL_ORDER ));
    }

    if( m_pCbTextOverlap->GetState() != TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_AXIS_LABEL_OVERLAP, m_pCbTextOverlap->IsChecked() ) );
    if( m_pCbTextBreak->GetState() != TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_AXIS_LABEL_BREAK, m_pCbTextBreak->IsChecked() ) );
    if( m_pCbShowDescription->GetState() != TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_AXIS_SHOWDESCR, m_pCbShowDescription->IsChecked() ) );

    if( m_pLbTextDirection->GetSelectEntryCount() > 0 )
        rOutAttrs->Put( SvxFrameDirectionItem( m_pLbTextDirection->GetSelectEntryValue(), EE_PARA_WRITINGDIR ) );

    return true;
}

void SchAxisLabelTabPage::Reset( const SfxItemSet* rInAttrs )
{
   const SfxPoolItem* pPoolItem = nullptr;

    // show description
    SfxItemState aState = rInAttrs->GetItemState( SCHATTR_AXIS_SHOWDESCR, false, &pPoolItem );
    if( aState == SfxItemState::DONTCARE )
    {
        m_pCbShowDescription->EnableTriState();
        m_pCbShowDescription->SetState( TRISTATE_INDET );
    }
    else
    {
        m_pCbShowDescription->EnableTriState( false );
        bool bCheck = false;
        if( aState == SfxItemState::SET )
            bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_pCbShowDescription->Check( bCheck );

        if( aState != SfxItemState::DEFAULT && aState != SfxItemState::SET )
            m_pCbShowDescription->Hide();
    }

    // Rotation as orient item or in degrees ----------

    // check new degree item
    m_nInitialDegrees = 0;
    aState = rInAttrs->GetItemState( SCHATTR_TEXT_DEGREES, false, &pPoolItem );
    if( aState == SfxItemState::SET )
        m_nInitialDegrees = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();

    m_bHasInitialDegrees = aState != SfxItemState::DONTCARE;
    if( m_bHasInitialDegrees )
        m_pCtrlDial->SetRotation( m_nInitialDegrees );
    else
        m_pCtrlDial->SetNoRotation();

    // check stacked item
    m_bInitialStacking = false;
    aState = rInAttrs->GetItemState( SCHATTR_TEXT_STACKED, false, &pPoolItem );
    if( aState == SfxItemState::SET )
        m_bInitialStacking = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();

    m_bHasInitialStacking = aState != SfxItemState::DONTCARE;
    if( m_bHasInitialDegrees )
        m_pOrientHlp->SetStackedState( m_bInitialStacking ? TRISTATE_TRUE : TRISTATE_FALSE );
    else
        m_pOrientHlp->SetStackedState( TRISTATE_INDET );

    if( rInAttrs->GetItemState( EE_PARA_WRITINGDIR, true, &pPoolItem ) == SfxItemState::SET )
        m_pLbTextDirection->SelectEntryValue( SvxFrameDirection(static_cast<const SvxFrameDirectionItem*>(pPoolItem)->GetValue()) );

    // Text overlap ----------
    aState = rInAttrs->GetItemState( SCHATTR_AXIS_LABEL_OVERLAP, false, &pPoolItem );
    if( aState == SfxItemState::DONTCARE )
    {
        m_pCbTextOverlap->EnableTriState();
        m_pCbTextOverlap->SetState( TRISTATE_INDET );
    }
    else
    {
        m_pCbTextOverlap->EnableTriState( false );
        bool bCheck = false;
        if( aState == SfxItemState::SET )
            bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_pCbTextOverlap->Check( bCheck );

        if( aState != SfxItemState::DEFAULT && aState != SfxItemState::SET )
            m_pCbTextOverlap->Hide();
    }

    // text break ----------
    aState = rInAttrs->GetItemState( SCHATTR_AXIS_LABEL_BREAK, false, &pPoolItem );
    if( aState == SfxItemState::DONTCARE )
    {
        m_pCbTextBreak->EnableTriState();
        m_pCbTextBreak->SetState( TRISTATE_INDET );
    }
    else
    {
        m_pCbTextBreak->EnableTriState( false );
        bool bCheck = false;
        if( aState == SfxItemState::SET )
            bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_pCbTextBreak->Check( bCheck );

        if( aState != SfxItemState::DEFAULT && aState != SfxItemState::SET )
        {
            m_pCbTextBreak->Hide();
            if( ! m_pCbTextOverlap->IsVisible() )
                m_pFlTextFlow->Hide();
        }
    }

    // text order ----------
    if( m_bShowStaggeringControls )
    {
        aState = rInAttrs->GetItemState( SCHATTR_AXIS_LABEL_ORDER, false, &pPoolItem );
        if( aState == SfxItemState::SET )
        {
            SvxChartTextOrder eOrder = static_cast< const SvxChartTextOrderItem * >( pPoolItem )->GetValue();

            switch( eOrder )
            {
                case CHTXTORDER_SIDEBYSIDE:
                    m_pRbSideBySide->Check();
                    break;
                case CHTXTORDER_UPDOWN:
                    m_pRbUpDown->Check();
                    break;
                case CHTXTORDER_DOWNUP:
                    m_pRbDownUp->Check();
                    break;
                case CHTXTORDER_AUTO:
                    m_pRbAuto->Check();
                    break;
            }
        }
    }

    ToggleShowLabel( nullptr );
}

void SchAxisLabelTabPage::ShowStaggeringControls( bool bShowStaggeringControls )
{
    m_bShowStaggeringControls = bShowStaggeringControls;

    if( !m_bShowStaggeringControls )
    {
        m_pRbSideBySide->Hide();
        m_pRbUpDown->Hide();
        m_pRbDownUp->Hide();
        m_pRbAuto->Hide();
        m_pFlOrder->Hide();
    }
}

void SchAxisLabelTabPage::SetComplexCategories( bool bComplexCategories )
{
    m_bComplexCategories = bComplexCategories;
}

// event handling routines

IMPL_LINK_NOARG_TYPED(SchAxisLabelTabPage, ToggleShowLabel, Button*, void)
{
    bool bEnable = ( m_pCbShowDescription->GetState() != TRISTATE_FALSE );

    m_pOrientHlp->Enable( bEnable );
    m_pFlOrder->Enable( bEnable );
    m_pRbSideBySide->Enable( bEnable );
    m_pRbUpDown->Enable( bEnable );
    m_pRbDownUp->Enable( bEnable );
    m_pRbAuto->Enable( bEnable );

    m_pFlTextFlow->Enable( bEnable );
    m_pCbTextOverlap->Enable( bEnable && !m_bComplexCategories );
    m_pCbTextBreak->Enable( bEnable );

    m_pFtTextDirection->Enable( bEnable );
    m_pLbTextDirection->Enable( bEnable );
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
