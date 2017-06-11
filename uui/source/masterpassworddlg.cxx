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

#include <tools/simplerm.hxx>
#include <vcl/layout.hxx>
#include "strings.hrc"
#include "masterpassworddlg.hxx"

// MasterPasswordDialog---------------------------------------------------


IMPL_LINK_NOARG(MasterPasswordDialog, OKHdl_Impl, Button*, void)
{
    EndDialog( RET_OK );
}


MasterPasswordDialog::MasterPasswordDialog
(
    vcl::Window*                                pParent,
    css::task::PasswordRequestMode              aDialogMode,
    const std::locale&                          rLocale
)
    : ModalDialog(pParent, "MasterPasswordDialog", "uui/ui/masterpassworddlg.ui")
    , nDialogMode(aDialogMode)
    , rResLocale(rLocale)
{
    get(m_pEDMasterPassword, "password");
    get(m_pOKBtn, "ok");
    if( nDialogMode == css::task::PasswordRequestMode_PASSWORD_REENTER )
    {
        OUString aErrorMsg(Translate::get(STR_ERROR_MASTERPASSWORD_WRONG, rResLocale));
        ScopedVclPtrInstance< MessageDialog > aErrorBox(pParent, aErrorMsg);
        aErrorBox->Execute();
    }

    m_pOKBtn->SetClickHdl( LINK( this, MasterPasswordDialog, OKHdl_Impl ) );
};

MasterPasswordDialog::~MasterPasswordDialog()
{
    disposeOnce();
}

void MasterPasswordDialog::dispose()
{
    m_pEDMasterPassword.clear();
    m_pOKBtn.clear();
    ModalDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
