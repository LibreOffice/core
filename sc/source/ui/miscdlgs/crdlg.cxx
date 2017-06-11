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

#include "crdlg.hxx"
#include "scresid.hxx"
#include "strings.hrc"

ScColOrRowDlg::ScColOrRowDlg(vcl::Window* pParent, const OUString& rStrTitle,
    const OUString& rStrLabel)
    : ModalDialog(pParent, "ColOrRowDialog",
        "modules/scalc/ui/colorrowdialog.ui")
{
    get(m_pBtnOk, "ok");
    get(m_pBtnCols, "columns");
    get(m_pBtnRows, "rows");
    get(m_pFrame, "frame");

    SetText(rStrTitle);
    m_pFrame->set_label(rStrLabel);

    m_pBtnCols->Check();

    m_pBtnOk->SetClickHdl( LINK( this, ScColOrRowDlg, OkHdl ) );
}

ScColOrRowDlg::~ScColOrRowDlg()
{
    disposeOnce();
}

void ScColOrRowDlg::dispose()
{
    m_pFrame.clear();
    m_pBtnRows.clear();
    m_pBtnCols.clear();
    m_pBtnOk.clear();
    ModalDialog::dispose();
}


IMPL_LINK_NOARG(ScColOrRowDlg, OkHdl, Button*, void)
{
    EndDialog( m_pBtnCols->IsChecked() ? SCRET_COLS : SCRET_ROWS );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
