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

#include "strindlg.hxx"
#include "scresid.hxx"

ScStringInputDlg::ScStringInputDlg( vcl::Window*         pParent,
                                    const OUString&   rTitle,
                                    const OUString&   rEditTitle,
                                    const OUString&   rDefault,
                                    const OString& sHelpId, const OString& sEditHelpId )
    : ModalDialog(pParent, "InputStringDialog", "modules/scalc/ui/inputstringdialog.ui")
{
    SetHelpId( sHelpId );
    SetText( rTitle );
    get(m_pFtEditTitle, "description_label");
    m_pFtEditTitle->SetText(rEditTitle);
    get(m_pEdInput, "name_entry");
    m_pEdInput->SetText( rDefault );
    m_pEdInput->SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
    m_pEdInput->SetHelpId( sEditHelpId );
}

ScStringInputDlg::~ScStringInputDlg()
{
    disposeOnce();
}

void ScStringInputDlg::dispose()
{
    m_pFtEditTitle.clear();
    m_pEdInput.clear();
    ModalDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
