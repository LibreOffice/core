/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <res_DataTableProperties.hxx>

#include <chartview/ChartSfxItemIds.hxx>
#include <svl/eitem.hxx>

using namespace css;

namespace chart
{
DataTablePropertiesResources::DataTablePropertiesResources(weld::Builder& rBuilder)
    : m_xCbHorizontalBorder(rBuilder.weld_check_button("horizontalBorderCB"))
    , m_xCbVerticalBorder(rBuilder.weld_check_button("verticalBorderCB"))
    , m_xCbOutilne(rBuilder.weld_check_button("outlineCB"))
    , m_xCbKeys(rBuilder.weld_check_button("keysCB"))
{
}

void DataTablePropertiesResources::setChecksSensitive(bool bSensitive)
{
    m_xCbHorizontalBorder->set_sensitive(bSensitive);
    m_xCbVerticalBorder->set_sensitive(bSensitive);
    m_xCbOutilne->set_sensitive(bSensitive);
    m_xCbKeys->set_sensitive(bSensitive);
}

void DataTablePropertiesResources::initFromItemSet(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem* pPoolItem = nullptr;
    SfxItemState aState;

    aState = rInAttrs.GetItemState(SCHATTR_DATA_TABLE_HORIZONTAL_BORDER, false, &pPoolItem);
    if (aState == SfxItemState::INVALID)
    {
        m_xCbHorizontalBorder->set_state(TRISTATE_INDET);
    }
    else
    {
        if (aState == SfxItemState::SET)
            m_xCbHorizontalBorder->set_active(
                static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    }

    aState = rInAttrs.GetItemState(SCHATTR_DATA_TABLE_VERTICAL_BORDER, false, &pPoolItem);
    if (aState == SfxItemState::INVALID)
    {
        m_xCbVerticalBorder->set_state(TRISTATE_INDET);
    }
    else
    {
        if (aState == SfxItemState::SET)
            m_xCbVerticalBorder->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    }

    aState = rInAttrs.GetItemState(SCHATTR_DATA_TABLE_OUTLINE, false, &pPoolItem);
    if (aState == SfxItemState::INVALID)
    {
        m_xCbOutilne->set_state(TRISTATE_INDET);
    }
    else
    {
        if (aState == SfxItemState::SET)
            m_xCbOutilne->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    }

    aState = rInAttrs.GetItemState(SCHATTR_DATA_TABLE_KEYS, false, &pPoolItem);
    if (aState == SfxItemState::INVALID)
    {
        m_xCbKeys->set_state(TRISTATE_INDET);
    }
    else
    {
        if (aState == SfxItemState::SET)
            m_xCbKeys->set_active(static_cast<const SfxBoolItem*>(pPoolItem)->GetValue());
    }
}

bool DataTablePropertiesResources::writeToItemSet(SfxItemSet& rOutAttrs) const
{
    if (m_xCbHorizontalBorder->get_state() != TRISTATE_INDET)
    {
        rOutAttrs.Put(
            SfxBoolItem(SCHATTR_DATA_TABLE_HORIZONTAL_BORDER, m_xCbHorizontalBorder->get_active()));
    }
    if (m_xCbVerticalBorder->get_state() != TRISTATE_INDET)
    {
        rOutAttrs.Put(
            SfxBoolItem(SCHATTR_DATA_TABLE_VERTICAL_BORDER, m_xCbVerticalBorder->get_active()));
    }
    if (m_xCbOutilne->get_state() != TRISTATE_INDET)
    {
        rOutAttrs.Put(SfxBoolItem(SCHATTR_DATA_TABLE_OUTLINE, m_xCbOutilne->get_active()));
    }
    if (m_xCbKeys->get_state() != TRISTATE_INDET)
    {
        rOutAttrs.Put(SfxBoolItem(SCHATTR_DATA_TABLE_KEYS, m_xCbKeys->get_active()));
    }
    return true;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
