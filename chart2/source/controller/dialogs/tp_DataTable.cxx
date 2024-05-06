/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tp_DataTable.hxx"

namespace chart
{
DataTableTabPage::DataTableTabPage(weld::Container* pPage, weld::DialogController* pController,
                                   const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"modules/schart/ui/tp_DataTable.ui"_ustr,
                 u"DataTableTabPage"_ustr, &rInAttrs)
    , m_aDataTablePropertiesResources(*m_xBuilder)
{
}

DataTableTabPage::~DataTableTabPage() = default;

std::unique_ptr<SfxTabPage> DataTableTabPage::Create(weld::Container* pPage,
                                                     weld::DialogController* pController,
                                                     const SfxItemSet* rAttrs)
{
    return std::make_unique<DataTableTabPage>(pPage, pController, *rAttrs);
}

bool DataTableTabPage::FillItemSet(SfxItemSet* pOutAttrs)
{
    return m_aDataTablePropertiesResources.writeToItemSet(*pOutAttrs);
}

void DataTableTabPage::Reset(const SfxItemSet* pInAttrs)
{
    m_aDataTablePropertiesResources.initFromItemSet(*pInAttrs);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
