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

#include "tp_PolarOptions.hxx"
#include <chartview/ChartSfxItemIds.hxx>

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/sdangitm.hxx>
#include <officecfg/Office/Compatibility.hxx>

namespace chart
{

PolarOptionsTabPage::PolarOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "modules/schart/ui/tp_PolarOptions.ui", "tp_PolarOptions", &rInAttrs)
    , m_xCB_Clockwise(m_xBuilder->weld_check_button("CB_CLOCKWISE"))
    , m_xFL_StartingAngle(m_xBuilder->weld_frame("frameANGLE"))
    , m_xNF_StartingAngle(m_xBuilder->weld_metric_spin_button("NF_STARTING_ANGLE", FieldUnit::DEGREE))
    , m_xFL_PlotOptions(m_xBuilder->weld_frame("framePLOT_OPTIONS"))
    , m_xCB_IncludeHiddenCells(m_xBuilder->weld_check_button("CB_INCLUDE_HIDDEN_CELLS_POLAR"))
    , m_xAngleDial(new svx::DialControl)
    , m_xAngleDialWin(new weld::CustomWeld(*m_xBuilder, "CT_ANGLE_DIAL", *m_xAngleDial))
{
    m_xAngleDial->SetLinkedField(m_xNF_StartingAngle.get());
}

PolarOptionsTabPage::~PolarOptionsTabPage()
{
    m_xAngleDialWin.reset();
    m_xAngleDial.reset();
}

std::unique_ptr<SfxTabPage> PolarOptionsTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rOutAttrs)
{
    return std::make_unique<PolarOptionsTabPage>(pPage, pController, *rOutAttrs);
}

bool PolarOptionsTabPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    if (m_xAngleDialWin->get_visible())
    {
        rOutAttrs->Put(SdrAngleItem(SCHATTR_STARTING_ANGLE, m_xAngleDial->GetRotation()));
    }

    if( m_xCB_Clockwise->get_visible() )
        rOutAttrs->Put(SfxBoolItem(SCHATTR_CLOCKWISE,m_xCB_Clockwise->get_active()));

    if (m_xCB_IncludeHiddenCells->get_visible())
        rOutAttrs->Put(SfxBoolItem(SCHATTR_INCLUDE_HIDDEN_CELLS, m_xCB_IncludeHiddenCells->get_active()));

    return true;
}

void PolarOptionsTabPage::Reset(const SfxItemSet* rInAttrs)
{
    const SfxPoolItem *pPoolItem = nullptr;

    if (rInAttrs->GetItemState(SCHATTR_STARTING_ANGLE, true, &pPoolItem) == SfxItemState::SET)
    {
        sal_Int32 nTmp = static_cast<const SdrAngleItem*>(pPoolItem)->GetValue();
        m_xAngleDial->SetRotation( nTmp );
    }
    else
    {
        m_xFL_StartingAngle->hide();
    }
    // tdf#108059 Hide clockwise orientation checkbox in OOXML-heavy environments it would be useless anyways
    if (!officecfg::Office::Compatibility::View::ClockwisePieChartDirection::get() && rInAttrs->GetItemState(SCHATTR_CLOCKWISE, true, &pPoolItem) == SfxItemState::SET)
    {
        bool bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_xCB_Clockwise->set_active(bCheck);
    }
    else
    {
        m_xCB_Clockwise->hide();
    }
    if (rInAttrs->GetItemState(SCHATTR_INCLUDE_HIDDEN_CELLS, true, &pPoolItem) == SfxItemState::SET)
    {
        bool bVal = static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();
        m_xCB_IncludeHiddenCells->set_active(bVal);
    }
    else
    {
        m_xFL_PlotOptions->hide();
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
