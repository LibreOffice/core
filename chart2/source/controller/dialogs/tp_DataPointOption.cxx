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

#include "tp_DataPointOption.hxx"

#include <chartview/ChartSfxItemIds.hxx>
#include <svl/eitem.hxx>

namespace chart
{
DataPointOptionTabPage::DataPointOptionTabPage(weld::Container* pPage,
                                               weld::DialogController* pController,
                                               const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"modules/schart/ui/tp_DataPointOption.ui"_ustr,
                 u"tp_DataPointOption"_ustr, &rInAttrs)
    , m_xCBHideLegendEntry(m_xBuilder->weld_check_button(u"CB_LEGEND_ENTRY_HIDDEN"_ustr))
{
}

DataPointOptionTabPage::~DataPointOptionTabPage() {}

std::unique_ptr<SfxTabPage> DataPointOptionTabPage::Create(weld::Container* pPage,
                                                           weld::DialogController* pController,
                                                           const SfxItemSet* rOutAttrs)
{
    return std::make_unique<DataPointOptionTabPage>(pPage, pController, *rOutAttrs);
}

bool DataPointOptionTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    if (m_xCBHideLegendEntry->get_visible())
        rOutAttrs->Put(
            SfxBoolItem(SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY, m_xCBHideLegendEntry->get_active()));

    return true;
}

void DataPointOptionTabPage::Reset(const SfxItemSet* rInAttrs)
{
    if (const SfxBoolItem* pEntryItem
        = rInAttrs->GetItemIfSet(SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY))
    {
        bool bVal = pEntryItem->GetValue();
        m_xCBHideLegendEntry->set_active(bVal);
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
