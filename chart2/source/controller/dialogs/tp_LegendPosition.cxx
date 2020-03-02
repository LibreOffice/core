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

#include "tp_LegendPosition.hxx"
#include <res_LegendPosition.hxx>
#include <TextDirectionListBox.hxx>
#include <chartview/ChartSfxItemIds.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

namespace chart
{

SchLegendPosTabPage::SchLegendPosTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "modules/schart/ui/tp_LegendPosition.ui", "tp_LegendPosition", &rInAttrs)
    , m_aLegendPositionResources(*m_xBuilder)
    , m_xLbTextDirection(new TextDirectionListBox(m_xBuilder->weld_combo_box("LB_LEGEND_TEXTDIR")))
    , m_xCBLegendNoOverlay(m_xBuilder->weld_check_button("CB_NO_OVERLAY"))
{
}

SchLegendPosTabPage::~SchLegendPosTabPage()
{
    m_xLbTextDirection.reset();
}

std::unique_ptr<SfxTabPage> SchLegendPosTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rOutAttrs)
{
    return std::make_unique<SchLegendPosTabPage>(pPage, pController, *rOutAttrs);
}

bool SchLegendPosTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    m_aLegendPositionResources.writeToItemSet(*rOutAttrs);

    if (m_xLbTextDirection->get_active() != -1)
        rOutAttrs->Put(SvxFrameDirectionItem(m_xLbTextDirection->get_active_id(), EE_PARA_WRITINGDIR));

    if (m_xCBLegendNoOverlay->get_visible())
        rOutAttrs->Put(SfxBoolItem(SCHATTR_LEGEND_NO_OVERLAY, m_xCBLegendNoOverlay->get_active()));

    return true;
}

void SchLegendPosTabPage::Reset(const SfxItemSet* rInAttrs)
{
    m_aLegendPositionResources.initFromItemSet(*rInAttrs);

    const SfxPoolItem* pPoolItem = nullptr;
    if( rInAttrs->GetItemState( EE_PARA_WRITINGDIR, true, &pPoolItem ) == SfxItemState::SET )
        m_xLbTextDirection->set_active_id( static_cast<const SvxFrameDirectionItem*>(pPoolItem)->GetValue() );

    if (rInAttrs->GetItemState(SCHATTR_LEGEND_NO_OVERLAY, true, &pPoolItem) == SfxItemState::SET)
    {
        bool bVal = static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();
        m_xCBLegendNoOverlay->set_active(bVal);
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
