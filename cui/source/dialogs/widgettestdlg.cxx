/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <widgettestdlg.hxx>

WidgetTestDialog::WidgetTestDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/widgettestdialog.ui", "WidgetTestDialog")
{
    m_xOKButton = m_xBuilder->weld_button("ok_btn");
    m_xCancelButton = m_xBuilder->weld_button("cancel_btn");

    m_xOKButton->connect_clicked(LINK(this, WidgetTestDialog, OkHdl));
    m_xCancelButton->connect_clicked(LINK(this, WidgetTestDialog, CancelHdl));
}

WidgetTestDialog::~WidgetTestDialog() {}

IMPL_LINK_NOARG(WidgetTestDialog, OkHdl, weld::Button&, void) { m_xDialog->response(RET_OK); }

IMPL_LINK_NOARG(WidgetTestDialog, CancelHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
