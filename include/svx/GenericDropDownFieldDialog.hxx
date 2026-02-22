/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/svxdllapi.h>
#include <vcl/weld/ComboBox.hxx>
#include <vcl/weld/DialogController.hxx>
#include <vcl/weld/weld.hxx>

class SVX_DLLPUBLIC GenericDropDownFieldDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::ComboBox> m_xComboBox;

public:
    GenericDropDownFieldDialog(weld::Widget* pParent, const OUString& rTitle);
    GenericDropDownFieldDialog(weld::Widget* pParent, const OUString& rTitle,
                               const std::vector<OUString>& rItems);

    void InsertItem(OUString aText);
    void SetActiveItem(sal_uInt32 nPos) { m_xComboBox->set_active(nPos); }
    OUString GetSelectedItem() { return m_xComboBox->get_active_text(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
