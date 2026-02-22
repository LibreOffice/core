/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/GenericDropDownFieldDialog.hxx>
#include <vcl/weld/Builder.hxx>

GenericDropDownFieldDialog::GenericDropDownFieldDialog(weld::Widget* pParent,
                                                       const OUString& rTitle)
    : GenericDialogController(pParent, u"svx/ui/dropdownfielddialog.ui"_ustr,
                              u"DropDownDialog"_ustr)
    , m_xComboBox(m_xBuilder->weld_combo_box(u"combo_box"_ustr))
{
    set_title(rTitle);
}

GenericDropDownFieldDialog::GenericDropDownFieldDialog(weld::Widget* pParent,
                                                       const OUString& rTitle,
                                                       const std::vector<OUString>& rItems)
    : GenericDropDownFieldDialog(pParent, rTitle)
{
    for (const OUString& rItem : rItems)
    {
        InsertItem(rItem);
    }
    SetActiveItem(0);
}

void GenericDropDownFieldDialog::InsertItem(OUString aText)
{
    m_xComboBox->append_text(std::move(aText));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
