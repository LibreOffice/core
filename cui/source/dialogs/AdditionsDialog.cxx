/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <AdditionsDialog.hxx>
#include <rtl/ustrbuf.hxx>
#include <unordered_set>

using namespace css;

AdditionsDialog::AdditionsDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/additionsdialog.ui", "AdditionsDialog")
    , m_xContentWindow(m_xBuilder->weld_scrolled_window("contentWindow"))
    , m_xContentGrid(m_xBuilder->weld_container("contentGrid"))
{
    fillGrid();
}

AdditionsDialog::~AdditionsDialog() {}

void AdditionsDialog::fillGrid()
{
    for (sal_Int32 i = 0; i < 20; i++)
    {
        m_aAdditionsItems.emplace_back(m_xContentGrid.get());
        AdditionsItem& aCurrentItem = m_aAdditionsItems.back();

        sal_Int32 nGridPositionX = 0;
        sal_Int32 nGridPositionY = i;
        aCurrentItem.m_xContainer->set_grid_left_attach(nGridPositionX);
        aCurrentItem.m_xContainer->set_grid_top_attach(nGridPositionY);
        /*
        Link<weld::Button&, void> aButtonHandler
            = LINK(this, AdditionsDialog, ComboBoxSelectedHdl);
        Link<weld::ToggleButton&, void> aCheckBoxToggleHandler
            = LINK(this, FontFeaturesDialog, CheckBoxToggledHdl);
        */
        aCurrentItem.m_xLabelName->set_label("name");
        aCurrentItem.m_xLabelName->show();
        aCurrentItem.m_xLabelDescription->set_label("idesc");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
