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

#include "namecrea.hxx"
#include "scresid.hxx"

ScNameCreateDlg::ScNameCreateDlg( vcl::Window * pParent, CreateNameFlags nFlags )
    : ModalDialog(pParent, "CreateNamesDialog", "modules/scalc/ui/createnamesdialog.ui")
{
    get(m_pTopBox, "top");
    get(m_pLeftBox, "left");
    get(m_pBottomBox, "bottom");
    get(m_pRightBox, "right");
    m_pTopBox->Check   ( bool(nFlags & CreateNameFlags::Top) );
    m_pLeftBox->Check  ( bool(nFlags & CreateNameFlags::Left) );
    m_pBottomBox->Check( bool(nFlags & CreateNameFlags::Bottom) );
    m_pRightBox->Check ( bool(nFlags & CreateNameFlags::Right) );
}

ScNameCreateDlg::~ScNameCreateDlg()
{
    disposeOnce();
}

void ScNameCreateDlg::dispose()
{
    m_pTopBox.clear();
    m_pLeftBox.clear();
    m_pBottomBox.clear();
    m_pRightBox.clear();
    ModalDialog::dispose();
}

CreateNameFlags ScNameCreateDlg::GetFlags() const
{
    CreateNameFlags nResult = CreateNameFlags::NONE;

    if (m_pTopBox->IsChecked())
        nResult |= CreateNameFlags::Top;
    if (m_pLeftBox->IsChecked())
        nResult |= CreateNameFlags::Left;
    if (m_pBottomBox->IsChecked())
        nResult |= CreateNameFlags::Bottom;
    if (m_pRightBox->IsChecked())
        nResult |= CreateNameFlags::Right;

    return nResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
