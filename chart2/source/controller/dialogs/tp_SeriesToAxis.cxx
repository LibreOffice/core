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

#include "tp_SeriesToAxis.hxx"

#include <chartview/ChartSfxItemIds.hxx>

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/ilstitem.hxx>

#include <com/sun/star/chart/MissingValueTreatment.hpp>

namespace chart
{

SchOptionTabPage::SchOptionTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"modules/schart/ui/tp_SeriesToAxis.ui"_ustr, u"TP_OPTIONS"_ustr, &rInAttrs)
    , m_nAllSeriesAxisIndex(0)
    , m_bProvidesSecondaryYAxis(true)
    , m_bProvidesOverlapAndGapWidth(false)
    , m_bProvidesBarConnectors(false)
    , m_xGrpAxis(m_xBuilder->weld_widget(u"frameGrpAxis"_ustr))
    , m_xRbtAxis1(m_xBuilder->weld_radio_button(u"RBT_OPT_AXIS_1"_ustr))
    , m_xRbtAxis2(m_xBuilder->weld_radio_button(u"RBT_OPT_AXIS_2"_ustr))
    , m_xGrpBar(m_xBuilder->weld_widget(u"frameSettings"_ustr))
    , m_xMTGap(m_xBuilder->weld_metric_spin_button(u"MT_GAP"_ustr, FieldUnit::PERCENT))
    , m_xMTOverlap(m_xBuilder->weld_metric_spin_button(u"MT_OVERLAP"_ustr, FieldUnit::PERCENT))
    , m_xCBConnect(m_xBuilder->weld_check_button(u"CB_CONNECTOR"_ustr))
    , m_xCBAxisSideBySide(m_xBuilder->weld_check_button(u"CB_BARS_SIDE_BY_SIDE"_ustr))
    , m_xGrpPlotOptions(m_xBuilder->weld_widget(u"frameFL_PLOT_OPTIONS"_ustr))
    , m_xGridPlotOptions(m_xBuilder->weld_widget(u"gridPLOT_OPTIONS"_ustr))
    , m_xRB_DontPaint(m_xBuilder->weld_radio_button(u"RB_DONT_PAINT"_ustr))
    , m_xRB_AssumeZero(m_xBuilder->weld_radio_button(u"RB_ASSUME_ZERO"_ustr))
    , m_xRB_ContinueLine(m_xBuilder->weld_radio_button(u"RB_CONTINUE_LINE"_ustr))
    , m_xCBIncludeHiddenCells(m_xBuilder->weld_check_button(u"CB_INCLUDE_HIDDEN_CELLS"_ustr))
    , m_xCBHideLegendEntry(m_xBuilder->weld_check_button(u"CB_LEGEND_ENTRY_HIDDEN"_ustr))
{
    m_xRbtAxis1->connect_toggled(LINK(this, SchOptionTabPage, EnableHdl));
    m_xRbtAxis2->connect_toggled(LINK(this, SchOptionTabPage, EnableHdl));
}

SchOptionTabPage::~SchOptionTabPage()
{
}

IMPL_LINK_NOARG(SchOptionTabPage, EnableHdl, weld::Toggleable&, void)
{
    if( m_nAllSeriesAxisIndex == 0 )
        m_xCBAxisSideBySide->set_sensitive( m_xRbtAxis2->get_active());
    else if( m_nAllSeriesAxisIndex == 1 )
        m_xCBAxisSideBySide->set_sensitive( m_xRbtAxis1->get_active());
}

std::unique_ptr<SfxTabPage> SchOptionTabPage::Create(weld::Container* pPage, weld::DialogController* pController,
                                            const SfxItemSet* rOutAttrs)
{
    return std::make_unique<SchOptionTabPage>(pPage, pController, *rOutAttrs);
}

bool SchOptionTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    if(m_xRbtAxis2->get_active())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_SECONDARY_Y));
    else
        rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_PRIMARY_Y));

    if(m_xMTGap->get_visible())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_BAR_GAPWIDTH,static_cast< sal_Int32 >( m_xMTGap->get_value(FieldUnit::PERCENT))));

    if(m_xMTOverlap->get_visible())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_BAR_OVERLAP,static_cast< sal_Int32 >( m_xMTOverlap->get_value(FieldUnit::PERCENT))));

    if(m_xCBConnect->get_visible())
        rOutAttrs->Put(SfxBoolItem(SCHATTR_BAR_CONNECT,m_xCBConnect->get_active()));

    // model property is "group bars per axis", UI feature is the other way
    // round: "show bars side by side"
    if(m_xCBAxisSideBySide->get_visible())
        rOutAttrs->Put(SfxBoolItem(SCHATTR_GROUP_BARS_PER_AXIS, ! m_xCBAxisSideBySide->get_active()));

    if(m_xRB_DontPaint->get_active())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT,css::chart::MissingValueTreatment::LEAVE_GAP));
    else if(m_xRB_AssumeZero->get_active())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT,css::chart::MissingValueTreatment::USE_ZERO));
    else if(m_xRB_ContinueLine->get_active())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT,css::chart::MissingValueTreatment::CONTINUE));

    if (m_xCBIncludeHiddenCells->get_visible())
        rOutAttrs->Put(SfxBoolItem(SCHATTR_INCLUDE_HIDDEN_CELLS, m_xCBIncludeHiddenCells->get_active()));

    if(m_xCBHideLegendEntry->get_visible())
        rOutAttrs->Put(SfxBoolItem(SCHATTR_HIDE_LEGEND_ENTRY, m_xCBHideLegendEntry->get_active()));

    return true;
}

void SchOptionTabPage::Reset(const SfxItemSet* rInAttrs)
{
    m_xRbtAxis1->set_active(true);
    m_xRbtAxis2->set_active(false);
    if (const SfxInt32Item* pAxisItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS))
    {
        tools::Long nVal = pAxisItem->GetValue();
        if(nVal==CHART_AXIS_SECONDARY_Y)
        {
            m_xRbtAxis2->set_active(true);
            m_xRbtAxis1->set_active(false);
        }
    }

    tools::Long nTmp;
    if (const SfxInt32Item* pGapWidthItem = rInAttrs->GetItemIfSet(SCHATTR_BAR_GAPWIDTH))
    {
        nTmp = static_cast<tools::Long>(pGapWidthItem->GetValue());
        m_xMTGap->set_value(nTmp, FieldUnit::PERCENT);
    }

    if (const SfxInt32Item* pOverlapItem = rInAttrs->GetItemIfSet(SCHATTR_BAR_OVERLAP))
    {
        nTmp = static_cast<tools::Long>(pOverlapItem->GetValue());
        m_xMTOverlap->set_value(nTmp, FieldUnit::PERCENT);
    }

    if (const SfxBoolItem* pConnectItem = rInAttrs->GetItemIfSet(SCHATTR_BAR_CONNECT))
    {
        bool bCheck = pConnectItem->GetValue();
        m_xCBConnect->set_active(bCheck);
    }

    if (const SfxInt32Item* pAllSeriesItem = rInAttrs->GetItemIfSet(SCHATTR_AXIS_FOR_ALL_SERIES))
    {
        m_nAllSeriesAxisIndex = pAllSeriesItem->GetValue();
        m_xCBAxisSideBySide->set_sensitive(false);
    }
    if (const SfxBoolItem* pPerAxisItem = rInAttrs->GetItemIfSet(SCHATTR_GROUP_BARS_PER_AXIS))
    {
        // model property is "group bars per axis", UI feature is the other way
        // round: "show bars side by side"
        bool bCheck = ! pPerAxisItem->GetValue();
        m_xCBAxisSideBySide->set_active( bCheck );
    }
    else
    {
        m_xCBAxisSideBySide->hide();
    }

    //missing value treatment
    {
        std::vector< sal_Int32 > aMissingValueTreatments;
        if (const SfxIntegerListItem* pValueTreatmentsItem = rInAttrs->GetItemIfSet(SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS))
            aMissingValueTreatments = pValueTreatmentsItem->GetList();

        const SfxInt32Item* pMissingValueTreatmentItem;
        if ( aMissingValueTreatments.size()>1 &&
            (pMissingValueTreatmentItem = rInAttrs->GetItemIfSet(SCHATTR_MISSING_VALUE_TREATMENT)) )
        {
            m_xRB_DontPaint->set_sensitive(false);
            m_xRB_AssumeZero->set_sensitive(false);
            m_xRB_ContinueLine->set_sensitive(false);

            for(int nVal : aMissingValueTreatments)
            {
                if(nVal==css::chart::MissingValueTreatment::LEAVE_GAP)
                    m_xRB_DontPaint->set_sensitive(true);
                else if(nVal==css::chart::MissingValueTreatment::USE_ZERO)
                    m_xRB_AssumeZero->set_sensitive(true);
                else if(nVal==css::chart::MissingValueTreatment::CONTINUE)
                    m_xRB_ContinueLine->set_sensitive(true);
            }

            tools::Long nVal=pMissingValueTreatmentItem->GetValue();
            if(nVal==css::chart::MissingValueTreatment::LEAVE_GAP)
                m_xRB_DontPaint->set_active(true);
            else if(nVal==css::chart::MissingValueTreatment::USE_ZERO)
                m_xRB_AssumeZero->set_active(true);
            else if(nVal==css::chart::MissingValueTreatment::CONTINUE)
                m_xRB_ContinueLine->set_active(true);
        }
        else
        {
            m_xGridPlotOptions->hide();
        }
    }

    // Include hidden cells
    if (const SfxBoolItem* pHiddenCellsItem = rInAttrs->GetItemIfSet(SCHATTR_INCLUDE_HIDDEN_CELLS))
    {
        bool bVal = pHiddenCellsItem->GetValue();
        m_xCBIncludeHiddenCells->set_active(bVal);
    }
    else
    {
        m_xCBIncludeHiddenCells->hide();
        // check if the radiobutton guys above
        // are visible. If they aren't, we can
        // as well hide the whole frame
        if(!m_xGridPlotOptions->get_visible())
            m_xGrpPlotOptions->hide();
    }

    if (const SfxBoolItem* pEntryItem = rInAttrs->GetItemIfSet(SCHATTR_HIDE_LEGEND_ENTRY))
    {
        bool bVal = pEntryItem->GetValue();
        m_xCBHideLegendEntry->set_active(bVal);
    }

    AdaptControlPositionsAndVisibility();
}

void SchOptionTabPage::Init( bool bProvidesSecondaryYAxis, bool bProvidesOverlapAndGapWidth, bool bProvidesBarConnectors )
{
    m_bProvidesSecondaryYAxis = bProvidesSecondaryYAxis;
    m_bProvidesOverlapAndGapWidth = bProvidesOverlapAndGapWidth;
    m_bProvidesBarConnectors = bProvidesBarConnectors;

    AdaptControlPositionsAndVisibility();
}

void SchOptionTabPage::AdaptControlPositionsAndVisibility()
{
    m_xGrpAxis->set_visible(m_bProvidesSecondaryYAxis);
    m_xGrpBar->set_visible(m_bProvidesOverlapAndGapWidth);
    m_xCBConnect->set_visible(m_bProvidesBarConnectors);

    if (!m_xMTGap->get_visible() && !m_xMTOverlap->get_visible())
        m_xGrpBar->hide();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
