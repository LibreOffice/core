/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dlg_InsertDataTable.hxx>

namespace chart
{
InsertDataTableDialog::InsertDataTableDialog(weld::Window* pWindow)
    : GenericDialogController(pWindow, u"modules/schart/ui/dlg_InsertDataTable.ui"_ustr,
                              u"InsertDataTableDialog"_ustr)
    , m_aDataTablePropertiesResources(*m_xBuilder)
    , m_xCbShowDataTable(m_xBuilder->weld_check_button(u"showDataTable"_ustr))
{
    m_xCbShowDataTable->connect_toggled(LINK(this, InsertDataTableDialog, ShowDataTableToggle));
    init(m_aData);
}

IMPL_LINK_NOARG(InsertDataTableDialog, ShowDataTableToggle, weld::Toggleable&, void)
{
    changeEnabled();
}

void InsertDataTableDialog::changeEnabled()
{
    bool bEnable = m_xCbShowDataTable->get_active();
    m_aDataTablePropertiesResources.setChecksSensitive(bEnable);
    m_aData.mbShow = bEnable;
}

void InsertDataTableDialog::init(DataTableDialogData const& rData)
{
    m_aData = rData;
    m_aDataTablePropertiesResources.setHorizontalBorder(m_aData.mbHorizontalBorders);
    m_aDataTablePropertiesResources.setVerticalBorder(m_aData.mbVerticalBorders);
    m_aDataTablePropertiesResources.setOutline(m_aData.mbOutline);
    m_aDataTablePropertiesResources.setKeys(m_aData.mbKeys);
    m_xCbShowDataTable->set_active(m_aData.mbShow);
    changeEnabled();
}

DataTableDialogData& InsertDataTableDialog::getDataTableDialogData()
{
    m_aData.mbShow = m_xCbShowDataTable->get_active();

    m_aData.mbHorizontalBorders = m_aDataTablePropertiesResources.getHorizontalBorder();
    m_aData.mbVerticalBorders = m_aDataTablePropertiesResources.getVerticalBorder();
    m_aData.mbOutline = m_aDataTablePropertiesResources.getOutline();
    m_aData.mbKeys = m_aDataTablePropertiesResources.getKeys();

    return m_aData;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
