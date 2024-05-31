/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#undef SC_DLLIMPLEMENTATION

#include <crdlg.hxx>
#include <scui_def.hxx>

ScColOrRowDlg::ScColOrRowDlg(weld::Window* pParent, const OUString& rStrTitle,
                             const OUString& rStrLabel)
    : GenericDialogController(pParent, u"modules/scalc/ui/colorrowdialog.ui"_ustr,
                              u"ColOrRowDialog"_ustr)
    , m_xFrame(m_xBuilder->weld_frame(u"frame"_ustr))
    , m_xBtnCols(m_xBuilder->weld_radio_button(u"columns"_ustr))
    , m_xBtnOk(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xDialog->set_title(rStrTitle);
    m_xFrame->set_label(rStrLabel);
    m_xBtnOk->connect_clicked(LINK(this, ScColOrRowDlg, OkHdl));
}

ScColOrRowDlg::~ScColOrRowDlg() {}

IMPL_LINK_NOARG(ScColOrRowDlg, OkHdl, weld::Button&, void)
{
    m_xDialog->response(m_xBtnCols->get_active() ? SCRET_COLS : SCRET_ROWS);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
