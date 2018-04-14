/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <warndlg.hxx>

namespace dbaccess
{
MigrationWarnDialog::MigrationWarnDialog(weld::Window* pParent)
    : MessageDialogController(pParent, "TODOUIfile", "MigrationWarnDialog", "ask")
    , m_xOkBtn(m_xBuilder->weld_button("yes"))
    , m_xLaterBtn(m_xBuilder->weld_button("later"))
{
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
