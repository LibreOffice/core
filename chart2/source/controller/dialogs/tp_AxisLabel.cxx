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

#include <chartview/ChartSfxItemIds.hxx>
#include <TextDirectionListBox.hxx>

#include <svx/chrtitem.hxx>
#include <svl/intitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

namespace chart
{

SchAxisLabelTabPage::SchAxisLabelTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "modules/schart/ui/tp_axisLabel.ui", "AxisLabelTabPage", &rInAttrs)
    , m_bShowStaggeringControls( true )
    , m_nInitialDegrees( 0 )
    , m_bHasInitialDegrees( true )
    , m_bInitialStacking( false )
    , m_bHasInitialStacking( true )
    , m_bComplexCategories( false )
    , m_xCbShowDescription(m_xBuilder->weld_check_button("showlabelsCB"))
    , m_xFlOrder(m_xBuilder->weld_label("orderL"))
    , m_xRbSideBySide(m_xBuilder->weld_radio_button("tile"))
    , m_xRbUpDown(m_xBuilder->weld_radio_button("odd"))
    , m_xRbDownUp(m_xBuilder->weld_radio_button("even"))
    , m_xRbAuto(m_xBuilder->weld_radio_button("auto"))
    , m_xFlTextFlow(m_xBuilder->weld_label("textflowL"))
    , m_xCbTextOverlap(m_xBuilder->weld_check_button("overlapCB"))
    , m_xCbTextBreak(m_xBuilder->weld_check_button("breakCB"))
    , m_xFtABCD(m_xBuilder->weld_label("labelABCD"))
    , m_xFlOrient(m_xBuilder->weld_label("labelTextOrient"))
    , m_xFtRotate(m_xBuilder->weld_label("degreeL"))
    , m_xNfRotate(m_xBuilder->weld_spin_button("OrientDegree"))
    , m_xCbStacked(m_xBuilder->weld_check_button("stackedCB"))
    , m_xFtTextDirection(m_xBuilder->weld_label("textdirL"))
    , m_xLbTextDirection(new TextDirectionListBox(m_xBuilder->weld_combo_box("textdirLB")))
    , m_xCtrlDial(new weld::CustomWeld(*m_xBuilder, "dialCtrl", m_aCtrlDial))
{
    m_aCtrlDial.SetText(m_xFtABCD->get_label());
    m_aCtrlDial.SetLinkedField(m_xNfRotate.get());
    m_xCtrlDial->set_sensitive(true);
    m_xNfRotate->set_sensitive(true);
    m_xCbStacked->set_sensitive(true);
    m_xFtRotate->set_sensitive(true);

    m_xCbStacked->connect_toggled(LINK(this, SchAxisLabelTabPage, StackedToggleHdl));
    m_xCbShowDescription->connect_toggled(LINK(this, SchAxisLabelTabPage, ToggleShowLabel));
}

SchAxisLabelTabPage::~SchAxisLabelTabPage()
{
    disposeOnce();
}

void SchAxisLabelTabPage::dispose()
{
    m_xCtrlDial.reset();
    m_xLbTextDirection.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SchAxisLabelTabPage::Create(TabPageParent pParent, const SfxItemSet* rAttrs)
{
    return VclPtr<SchAxisLabelTabPage>::Create(pParent, *rAttrs);
}

bool SchAxisLabelTabPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    bool bStacked = false;
    if (m_xCbStacked->get_state() != TRISTATE_INDET )
    {
        bStacked = m_xCbStacked->get_state() == TRISTATE_TRUE;
        if( !m_bHasInitialStacking || (bStacked != m_bInitialStacking) )
            rOutAttrs->Put( SfxBoolItem( SCHATTR_TEXT_STACKED, bStacked ) );
    }

    if( m_aCtrlDial.HasRotation() )
    {
        sal_Int32 nDegrees = bStacked ? 0 : m_aCtrlDial.GetRotation();
        if( !m_bHasInitialDegrees || (nDegrees != m_nInitialDegrees) )
            rOutAttrs->Put( SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );
    }

    if( m_bShowStaggeringControls )
    {
        SvxChartTextOrder eOrder = SvxChartTextOrder::SideBySide;
        bool bRadioButtonChecked = true;

        if( m_xRbUpDown->get_active())
            eOrder = SvxChartTextOrder::UpDown;
        else if( m_xRbDownUp->get_active())
            eOrder = SvxChartTextOrder::DownUp;
        else if( m_xRbAuto->get_active())
            eOrder = SvxChartTextOrder::Auto;
        else if( m_xRbSideBySide->get_active())
            eOrder = SvxChartTextOrder::SideBySide;
        else
            bRadioButtonChecked = false;

        if( bRadioButtonChecked )
            rOutAttrs->Put( SvxChartTextOrderItem( eOrder, SCHATTR_AXIS_LABEL_ORDER ));
    }

    if( m_xCbTextOverlap->get_state() != TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_AXIS_LABEL_OVERLAP, m_xCbTextOverlap->get_active() ) );
    if( m_xCbTextBreak->get_state() != TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_AXIS_LABEL_BREAK, m_xCbTextBreak->get_active() ) );
    if( m_xCbShowDescription->get_state() != TRISTATE_INDET )
        rOutAttrs->Put( SfxBoolItem( SCHATTR_AXIS_SHOWDESCR, m_xCbShowDescription->get_active() ) );

    if (m_xLbTextDirection->get_active() != -1)
        rOutAttrs->Put( SvxFrameDirectionItem( m_xLbTextDirection->get_active_id(), EE_PARA_WRITINGDIR ) );

    return true;
}

void SchAxisLabelTabPage::Reset( const SfxItemSet* rInAttrs )
{
    const SfxPoolItem* pPoolItem = nullptr;

    // show description
    SfxItemState aState = rInAttrs->GetItemState( SCHATTR_AXIS_SHOWDESCR, false, &pPoolItem );
    if( aState == SfxItemState::DONTCARE )
    {
        m_xCbShowDescription->set_state( TRISTATE_INDET );
    }
    else
    {
        bool bCheck = false;
        if( aState == SfxItemState::SET )
            bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_xCbShowDescription->set_active( bCheck );

        if( aState != SfxItemState::DEFAULT && aState != SfxItemState::SET )
            m_xCbShowDescription->hide();
    }

    // Rotation as orient item or in degrees ----------

    // check new degree item
    m_nInitialDegrees = 0;
    aState = rInAttrs->GetItemState( SCHATTR_TEXT_DEGREES, false, &pPoolItem );
    if( aState == SfxItemState::SET )
        m_nInitialDegrees = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();

    m_bHasInitialDegrees = aState != SfxItemState::DONTCARE;
    if( m_bHasInitialDegrees )
        m_aCtrlDial.SetRotation( m_nInitialDegrees );
    else
        m_aCtrlDial.SetNoRotation();

    // check stacked item
    m_bInitialStacking = false;
    aState = rInAttrs->GetItemState( SCHATTR_TEXT_STACKED, false, &pPoolItem );
    if( aState == SfxItemState::SET )
        m_bInitialStacking = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();

    m_bHasInitialStacking = aState != SfxItemState::DONTCARE;
    if( m_bHasInitialDegrees )
        m_xCbStacked->set_state(m_bInitialStacking ? TRISTATE_TRUE : TRISTATE_FALSE);
    else
        m_xCbStacked->set_state(TRISTATE_INDET);
    StackedToggleHdl(*m_xCbStacked);

    if( rInAttrs->GetItemState( EE_PARA_WRITINGDIR, true, &pPoolItem ) == SfxItemState::SET )
        m_xLbTextDirection->set_active_id( static_cast<const SvxFrameDirectionItem*>(pPoolItem)->GetValue() );

    // Text overlap ----------
    aState = rInAttrs->GetItemState( SCHATTR_AXIS_LABEL_OVERLAP, false, &pPoolItem );
    if( aState == SfxItemState::DONTCARE )
    {
        m_xCbTextOverlap->set_state( TRISTATE_INDET );
    }
    else
    {
        bool bCheck = false;
        if( aState == SfxItemState::SET )
            bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_xCbTextOverlap->set_active( bCheck );

        if( aState != SfxItemState::DEFAULT && aState != SfxItemState::SET )
            m_xCbTextOverlap->hide();
    }

    // text break ----------
    aState = rInAttrs->GetItemState( SCHATTR_AXIS_LABEL_BREAK, false, &pPoolItem );
    if( aState == SfxItemState::DONTCARE )
    {
        m_xCbTextBreak->set_state( TRISTATE_INDET );
    }
    else
    {
        bool bCheck = false;
        if( aState == SfxItemState::SET )
            bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_xCbTextBreak->set_active( bCheck );

        if( aState != SfxItemState::DEFAULT && aState != SfxItemState::SET )
        {
            m_xCbTextBreak->hide();
            if( ! m_xCbTextOverlap->get_visible() )
                m_xFlTextFlow->hide();
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
                case SvxChartTextOrder::SideBySide:
                    m_xRbSideBySide->set_active(true);
                    break;
                case SvxChartTextOrder::UpDown:
                    m_xRbUpDown->set_active(true);
                    break;
                case SvxChartTextOrder::DownUp:
                    m_xRbDownUp->set_active(true);
                    break;
                case SvxChartTextOrder::Auto:
                    m_xRbAuto->set_active(true);
                    break;
            }
        }
    }

    ToggleShowLabel(*m_xCbShowDescription);
}

void SchAxisLabelTabPage::ShowStaggeringControls( bool bShowStaggeringControls )
{
    m_bShowStaggeringControls = bShowStaggeringControls;

    if( !m_bShowStaggeringControls )
    {
        m_xRbSideBySide->hide();
        m_xRbUpDown->hide();
        m_xRbDownUp->hide();
        m_xRbAuto->hide();
        m_xFlOrder->hide();
    }
}

void SchAxisLabelTabPage::SetComplexCategories( bool bComplexCategories )
{
    m_bComplexCategories = bComplexCategories;
}

// event handling routines

IMPL_LINK_NOARG(SchAxisLabelTabPage, StackedToggleHdl, weld::ToggleButton&, void)
{
    bool bActive = m_xCbStacked->get_active() && m_xCbStacked->get_sensitive();
    m_xNfRotate->set_sensitive(!bActive);
    m_xCtrlDial->set_sensitive(!bActive);
    m_aCtrlDial.StyleUpdated();
    m_xFtRotate->set_sensitive(!bActive);
}

IMPL_LINK_NOARG(SchAxisLabelTabPage, ToggleShowLabel, weld::ToggleButton&, void)
{
    bool bEnable = ( m_xCbShowDescription->get_state() != TRISTATE_FALSE );

    m_xCbStacked->set_sensitive(bEnable);
    StackedToggleHdl(*m_xCbStacked);

    m_xFlOrder->set_sensitive( bEnable );
    m_xRbSideBySide->set_sensitive( bEnable );
    m_xRbUpDown->set_sensitive( bEnable );
    m_xRbDownUp->set_sensitive( bEnable );
    m_xRbAuto->set_sensitive( bEnable );

    m_xFlTextFlow->set_sensitive( bEnable );
    m_xCbTextOverlap->set_sensitive( bEnable && !m_bComplexCategories );
    m_xCbTextBreak->set_sensitive( bEnable );

    m_xFtTextDirection->set_sensitive( bEnable );
    m_xLbTextDirection->set_sensitive( bEnable );
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
