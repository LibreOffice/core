/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <migrwarndlg.hxx>

namespace dbaccess
{
MigrationWarnDialog::MigrationWarnDialog(weld::Window* pParent)
    : MessageDialogController(pParent, u"dbaccess/ui/migrwarndlg.ui"_ustr,
                              u"MigrationWarnDialog"_ustr)
    , m_xLater(m_xBuilder->weld_button(u"no"_ustr))
{
    m_xLater->grab_focus();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
