/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <toolbarmodedlg.hxx>

#include <dialmgr.hxx>

ToolbarmodeDialog::ToolbarmodeDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/toolbarmodedialog.ui", "User Interface")
    , m_pImage(m_xBuilder->weld_image("imImage"))
{
}

ToolbarmodeDialog::~ToolbarmodeDialog()
{
}