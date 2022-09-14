/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>
#include "res_DataTableProperties.hxx"

namespace chart
{
/** The data table properties (data) used by the dialog */
struct DataTableDialogData
{
    bool mbShow = true;
    bool mbHorizontalBorders = false;
    bool mbVerticalBorders = false;
    bool mbOutline = false;
    bool mbKeys = false;
};

/** The dialog to change the data table specific properties */
class InsertDataTableDialog final : public weld::GenericDialogController
{
private:
    DataTablePropertiesResources m_aDataTablePropertiesResources;
    std::unique_ptr<weld::CheckButton> m_xCbShowDataTable;

    DataTableDialogData m_aData;

    DECL_LINK(ShowDataTableToggle, weld::Toggleable&, void);

    void changeEnabled();

public:
    InsertDataTableDialog(weld::Window* pParent);

    /** Set the initial state of the data table properties */
    void init(DataTableDialogData const& rData);

    /** Get the state of the data table properties from the dialog */
    DataTableDialogData& getDataTableDialogData();
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
