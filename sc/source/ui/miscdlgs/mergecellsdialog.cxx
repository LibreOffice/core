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
    : GenericDialogController(pParent, "modules/scalc/ui/mergecellsdialog.ui", "MergeCellsDialog")
    , m_xRBMoveContent(m_xBuilder->weld_radio_button("move-cells-radio"))
    , m_xRBKeepContent(m_xBuilder->weld_radio_button("keep-content-radio"))
    , m_xRBEmptyContent(m_xBuilder->weld_radio_button("empty-cells-radio"))
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
