/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <mergecellsdialog.hxx>

ScMergeCellsDialog::ScMergeCellsDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"modules/scalc/ui/mergecellsdialog.ui"_ustr,
                              u"MergeCellsDialog"_ustr)
    , m_xRBMoveContent(m_xBuilder->weld_radio_button(u"move-cells-radio"_ustr))
    , m_xRBKeepContent(m_xBuilder->weld_radio_button(u"keep-content-radio"_ustr))
    , m_xRBEmptyContent(m_xBuilder->weld_radio_button(u"empty-cells-radio"_ustr))
{
    m_xRBKeepContent->set_active(true);
}

ScMergeCellsDialog::~ScMergeCellsDialog() {}

ScMergeCellsOption ScMergeCellsDialog::GetMergeCellsOption() const
{
    if (m_xRBMoveContent->get_active())
        return MoveContentHiddenCells;
    if (m_xRBKeepContent->get_active())
        return KeepContentHiddenCells;
    if (m_xRBEmptyContent->get_active())
        return EmptyContentHiddenCells;
    assert(!"Unknown selection for merge cells.");
    return KeepContentHiddenCells; // default value
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
