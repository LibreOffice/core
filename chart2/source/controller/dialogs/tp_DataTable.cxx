/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tp_DataTable.hxx"

#include <chartview/ChartSfxItemIds.hxx>
#include <svl/eitem.hxx>

namespace chart
{
DataTableTabPage::DataTableTabPage(weld::Container* pPage, weld::DialogController* pController,
                                   const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "modules/schart/ui/tp_DataTable.ui", "DataTableTabPage",
                 &rInAttrs)
    , m_xCbHorizontalBorder(m_xBuilder->weld_check_button("horizontalBorderCB"))
    , m_xCbVerticalBorder(m_xBuilder->weld_check_button("verticalBorderCB"))
    , m_xCbOutilne(m_xBuilder->weld_check_button("outlineCB"))
    , m_xCbKeys(m_xBuilder->weld_check_button("keysCB"))
{
}

DataTableTabPage::~DataTableTabPage() = default;

std::unique_ptr<SfxTabPage> DataTableTabPage::Create(weld::Container* pPage,
                                                     weld::DialogController* pController,
                                                     const SfxItemSet* rAttrs)
{
    return std::make_unique<DataTableTabPage>(pPage, pController, *rAttrs);
}

bool DataTableTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    if (m_xCbHorizontalBorder->get_state() != TRISTATE_INDET)
    {
        rOutAttrs->Put(
            SfxBoolItem(SCHATTR_DATA_TABLE_HORIZONTAL_BORDER, m_xCbHorizontalBorder->get_active()));
    }
    if (m_xCbVerticalBorder->get_state() != TRISTATE_INDET)
    {
        rOutAttrs->Put(
            SfxBoolItem(SCHATTR_DATA_TABLE_VERTICAL_BORDER, m_xCbVerticalBorder->get_active()));
    }
    if (m_xCbOutilne->get_state() != TRISTATE_INDET)
    {
        rOutAttrs->Put(SfxBoolItem(SCHATTR_DATA_TABLE_OUTLINE, m_xCbOutilne->get_active()));
    }
    if (m_xCbKeys->get_state() != TRISTATE_INDET)
    {
        rOutAttrs->Put(SfxBoolItem(SCHATTR_DATA_TABLE_KEYS, m_xCbKeys->get_active()));
    }
    return true;
}

void DataTableTabPage::Reset(const SfxItemSet* pInAttrs)
{
    const SfxPoolItem* pPoolItem = nullptr;
    SfxItemState aState;

    aState = pInAttrs->GetItemState(SCHATTR_DATA_TABLE_HORIZONTAL_BORDER, false, &pPoolItem);
    if (aState == SfxItemState::DONTCARE)
    {
        m_xCbHorizontalBorder->set_state(TRISTATE_INDET);
    }
    else
    {
        if (aState == SfxItemState::SET)
            m_xCbHorizontalBorder->set_active(
                static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    }

    aState = pInAttrs->GetItemState(SCHATTR_DATA_TABLE_VERTICAL_BORDER, false, &pPoolItem);
    if (aState == SfxItemState::DONTCARE)
    {
        m_xCbVerticalBorder->set_state(TRISTATE_INDET);
    }
    else
    {
        if (aState == SfxItemState::SET)
            m_xCbVerticalBorder->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    }

    aState = pInAttrs->GetItemState(SCHATTR_DATA_TABLE_OUTLINE, false, &pPoolItem);
    if (aState == SfxItemState::DONTCARE)
    {
        m_xCbOutilne->set_state(TRISTATE_INDET);
    }
    else
    {
        if (aState == SfxItemState::SET)
            m_xCbOutilne->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    }

    aState = pInAttrs->GetItemState(SCHATTR_DATA_TABLE_KEYS, false, &pPoolItem);
    if (aState == SfxItemState::DONTCARE)
    {
        m_xCbKeys->set_state(TRISTATE_INDET);
    }
    else
    {
        if (aState == SfxItemState::SET)
            m_xCbKeys->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
