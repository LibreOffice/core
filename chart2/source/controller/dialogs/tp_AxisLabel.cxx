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
#include <svx/sdangitm.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

namespace chart
{

SchAxisLabelTabPage::SchAxisLabelTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"modules/schart/ui/tp_axisLabel.ui"_ustr, u"AxisLabelTabPage"_ustr, &rInAttrs)
    , m_bShowStaggeringControls( true )
    , m_nInitialDegrees( 0 )
    , m_bHasInitialDegrees( true )
    , m_bInitialStacking( false )
    , m_bHasInitialStacking( true )
    , m_bComplexCategories( false )
    , m_xCbShowDescription(m_xBuilder->weld_check_button(u"showlabelsCB"_ustr))
    , m_xFlOrder(m_xBuilder->weld_label(u"orderL"_ustr))
    , m_xRbSideBySide(m_xBuilder->weld_radio_button(u"tile"_ustr))
    , m_xRbUpDown(m_xBuilder->weld_radio_button(u"odd"_ustr))
    , m_xRbDownUp(m_xBuilder->weld_radio_button(u"even"_ustr))
    , m_xRbAuto(m_xBuilder->weld_radio_button(u"auto"_ustr))
    , m_xFlTextFlow(m_xBuilder->weld_label(u"textflowL"_ustr))
    , m_xCbTextOverlap(m_xBuilder->weld_check_button(u"overlapCB"_ustr))
    , m_xCbTextBreak(m_xBuilder->weld_check_button(u"breakCB"_ustr))
    , m_xFtABCD(m_xBuilder->weld_label(u"labelABCD"_ustr))
    , m_xFtRotate(m_xBuilder->weld_label(u"degreeL"_ustr))
    , m_xNfRotate(m_xBuilder->weld_metric_spin_button(u"OrientDegree"_ustr, FieldUnit::DEGREE))
    , m_xCbStacked(m_xBuilder->weld_check_button(u"stackedCB"_ustr))
    , m_xFtTextDirection(m_xBuilder->weld_label(u"textdirL"_ustr))
    , m_aLbTextDirection(m_xBuilder->weld_combo_box(u"textdirLB"_ustr))
    , m_xCtrlDial(new svx::DialControl)
    , m_xCtrlDialWin(new weld::CustomWeld(*m_xBuilder, u"dialCtrl"_ustr, *m_xCtrlDial))
{
    m_xCtrlDial->SetText(m_xFtABCD->get_label());
    m_xCtrlDial->SetLinkedField(m_xNfRotate.get());
    m_xCtrlDialWin->set_sensitive(true);
    m_xNfRotate->set_sensitive(true);
    m_xCbStacked->set_sensitive(true);
    m_xFtRotate->set_sensitive(true);

    m_xCbStacked->connect_toggled(LINK(this, SchAxisLabelTabPage, StackedToggleHdl));
    m_xCbShowDescription->connect_toggled(LINK(this, SchAxisLabelTabPage, ToggleShowLabel));
}

SchAxisLabelTabPage::~SchAxisLabelTabPage()
{
    m_xCtrlDialWin.reset();
    m_xCtrlDial.reset();
}

std::unique_ptr<SfxTabPage> SchAxisLabelTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrs)
{
    return std::make_unique<SchAxisLabelTabPage>(pPage, pController, *rAttrs);
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

    if( m_xCtrlDial->HasRotation() )
    {
        Degree100 nDegrees = bStacked ? 0_deg100 : m_xCtrlDial->GetRotation();
        if( !m_bHasInitialDegrees || (nDegrees != m_nInitialDegrees) )
            rOutAttrs->Put( SdrAngleItem( SCHATTR_TEXT_DEGREES, nDegrees ) );
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

    if (m_aLbTextDirection.get_active() != -1)
        rOutAttrs->Put( SvxFrameDirectionItem( m_aLbTextDirection.get_active_id(), EE_PARA_WRITINGDIR ) );

    return true;
}

void SchAxisLabelTabPage::Reset( const SfxItemSet* rInAttrs )
{
    const SfxPoolItem* pPoolItem = nullptr;

    // show description
    SfxItemState aState = rInAttrs->GetItemState( SCHATTR_AXIS_SHOWDESCR, false, &pPoolItem );
    if( aState == SfxItemState::INVALID )
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
    m_nInitialDegrees = 0_deg100;
    aState = rInAttrs->GetItemState( SCHATTR_TEXT_DEGREES, false, &pPoolItem );
    if( aState == SfxItemState::SET )
        m_nInitialDegrees = static_cast< const SdrAngleItem * >( pPoolItem )->GetValue();

    m_bHasInitialDegrees = aState != SfxItemState::INVALID;
    if( m_bHasInitialDegrees )
        m_xCtrlDial->SetRotation( m_nInitialDegrees );
    else
        m_xCtrlDial->SetNoRotation();

    // check stacked item
    m_bInitialStacking = false;
    aState = rInAttrs->GetItemState( SCHATTR_TEXT_STACKED, false, &pPoolItem );
    if( aState == SfxItemState::SET )
        m_bInitialStacking = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();

    m_bHasInitialStacking = aState != SfxItemState::INVALID;
    if( m_bHasInitialDegrees )
        m_xCbStacked->set_state(m_bInitialStacking ? TRISTATE_TRUE : TRISTATE_FALSE);
    else
        m_xCbStacked->set_state(TRISTATE_INDET);
    StackedToggleHdl(*m_xCbStacked);

    if( const SvxFrameDirectionItem* pDirectionItem = rInAttrs->GetItemIfSet( EE_PARA_WRITINGDIR ) )
        m_aLbTextDirection.set_active_id( pDirectionItem->GetValue() );

    // Text overlap ----------
    aState = rInAttrs->GetItemState( SCHATTR_AXIS_LABEL_OVERLAP, false, &pPoolItem );
    if( aState == SfxItemState::INVALID )
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
    if( aState == SfxItemState::INVALID )
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
        if( const SvxChartTextOrderItem* pOrderItem = rInAttrs->GetItemIfSet( SCHATTR_AXIS_LABEL_ORDER, false ) )
        {
            SvxChartTextOrder eOrder = pOrderItem->GetValue();

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

IMPL_LINK_NOARG(SchAxisLabelTabPage, StackedToggleHdl, weld::Toggleable&, void)
{
    bool bActive = m_xCbStacked->get_active() && m_xCbStacked->get_sensitive();
    m_xNfRotate->set_sensitive(!bActive);
    m_xCtrlDialWin->set_sensitive(!bActive);
    m_xCtrlDial->StyleUpdated();
    m_xFtRotate->set_sensitive(!bActive);
}

IMPL_LINK_NOARG(SchAxisLabelTabPage, ToggleShowLabel, weld::Toggleable&, void)
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
    m_aLbTextDirection.set_sensitive( bEnable );
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
