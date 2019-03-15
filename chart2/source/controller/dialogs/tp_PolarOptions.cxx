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

namespace chart
{

PolarOptionsTabPage::PolarOptionsTabPage(TabPageParent pWindow, const SfxItemSet& rInAttrs)
    : SfxTabPage(pWindow, "modules/schart/ui/tp_PolarOptions.ui", "tp_PolarOptions", &rInAttrs)
    , m_xCB_Clockwise(m_xBuilder->weld_check_button("CB_CLOCKWISE"))
    , m_xFL_StartingAngle(m_xBuilder->weld_frame("frameANGLE"))
    , m_xNF_StartingAngle(m_xBuilder->weld_spin_button("NF_STARTING_ANGLE"))
    , m_xFL_PlotOptions(m_xBuilder->weld_frame("framePLOT_OPTIONS"))
    , m_xCB_IncludeHiddenCells(m_xBuilder->weld_check_button("CB_INCLUDE_HIDDEN_CELLS_POLAR"))
    , m_xAngleDial(new weld::CustomWeld(*m_xBuilder, "CT_ANGLE_DIAL", m_aAngleDial))
{
    m_aAngleDial.SetLinkedField(m_xNF_StartingAngle.get());
}

PolarOptionsTabPage::~PolarOptionsTabPage()
{
    disposeOnce();
}

void PolarOptionsTabPage::dispose()
{
    m_xAngleDial.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> PolarOptionsTabPage::Create(TabPageParent pParent, const SfxItemSet* rOutAttrs)
{
    return VclPtr<PolarOptionsTabPage>::Create(pParent, *rOutAttrs);
}

bool PolarOptionsTabPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    if (m_xAngleDial->get_visible())
    {
        rOutAttrs->Put(SfxInt32Item(SCHATTR_STARTING_ANGLE,
            static_cast< sal_Int32 >(m_aAngleDial.GetRotation()/100)));
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
        long nTmp = static_cast<long>(static_cast<const SfxInt32Item*>(pPoolItem)->GetValue());
        m_aAngleDial.SetRotation( nTmp*100 );
    }
    else
    {
        m_xFL_StartingAngle->hide();
    }
    if (rInAttrs->GetItemState(SCHATTR_CLOCKWISE, true, &pPoolItem) == SfxItemState::SET)
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
