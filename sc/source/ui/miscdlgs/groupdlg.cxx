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

#include "groupdlg.hxx"
#include "scresid.hxx"

ScGroupDlg::ScGroupDlg(vcl::Window* pParent, bool bUngroup, bool bRows)
    : ModalDialog(pParent,
        bUngroup ?
            OUString("UngroupDialog") :
            OUString("GroupDialog"),
        bUngroup ?
            OUString("modules/scalc/ui/ungroupdialog.ui") :
            OUString("modules/scalc/ui/groupdialog.ui"))
{
    get(m_pBtnRows, "rows");
    get(m_pBtnCols, "cols");

    if ( bRows )
        m_pBtnRows->Check();
    else
        m_pBtnCols->Check();

    m_pBtnRows->GrabFocus();
}

ScGroupDlg::~ScGroupDlg()
{
    disposeOnce();
}

void ScGroupDlg::dispose()
{
    m_pBtnRows.clear();
    m_pBtnCols.clear();
    ModalDialog::dispose();
}

bool ScGroupDlg::GetColsChecked() const
{
    return m_pBtnCols->IsChecked();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
