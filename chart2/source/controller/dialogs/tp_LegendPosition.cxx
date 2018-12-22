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
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

namespace chart
{

SchLegendPosTabPage::SchLegendPosTabPage(TabPageParent pWindow, const SfxItemSet& rInAttrs)
    : SfxTabPage(pWindow, "modules/schart/ui/tp_LegendPosition.ui", "tp_LegendPosition", &rInAttrs)
    , m_aLegendPositionResources(*m_xBuilder)
    , m_xLbTextDirection(new TextDirectionListBox(m_xBuilder->weld_combo_box("LB_LEGEND_TEXTDIR")))
{
}

SchLegendPosTabPage::~SchLegendPosTabPage()
{
    disposeOnce();
}

void SchLegendPosTabPage::dispose()
{
    m_xLbTextDirection.reset();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> SchLegendPosTabPage::Create(TabPageParent pParent, const SfxItemSet* rOutAttrs)
{
    return VclPtr<SchLegendPosTabPage>::Create(pParent, *rOutAttrs);
}

bool SchLegendPosTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    m_aLegendPositionResources.writeToItemSet(*rOutAttrs);

    if (m_xLbTextDirection->get_active() != -1)
        rOutAttrs->Put(SvxFrameDirectionItem(m_xLbTextDirection->get_active_id(), EE_PARA_WRITINGDIR));

    return true;
}

void SchLegendPosTabPage::Reset(const SfxItemSet* rInAttrs)
{
    m_aLegendPositionResources.initFromItemSet(*rInAttrs);

    const SfxPoolItem* pPoolItem = nullptr;
    if( rInAttrs->GetItemState( EE_PARA_WRITINGDIR, true, &pPoolItem ) == SfxItemState::SET )
        m_xLbTextDirection->set_active_id( static_cast<const SvxFrameDirectionItem*>(pPoolItem)->GetValue() );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
