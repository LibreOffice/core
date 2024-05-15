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

#include <unotools/resmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <strings.hrc>
#include "masterpassworddlg.hxx"

// MasterPasswordDialog---------------------------------------------------

IMPL_LINK_NOARG(MasterPasswordDialog, OKHdl_Impl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

MasterPasswordDialog::MasterPasswordDialog
(
    weld::Window*                               pParent,
    css::task::PasswordRequestMode              nDialogMode,
    const std::locale&                          rLocale
)
    : GenericDialogController(pParent, u"uui/ui/masterpassworddlg.ui"_ustr, u"MasterPasswordDialog"_ustr)
    , m_xEDMasterPassword(m_xBuilder->weld_entry(u"password"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
{
    if( nDialogMode == css::task::PasswordRequestMode_PASSWORD_REENTER )
    {
        OUString aErrorMsg(Translate::get(STR_ERROR_MASTERPASSWORD_WRONG, rLocale));
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(pParent,
                                                       VclMessageType::Warning, VclButtonsType::Ok, aErrorMsg));
        xErrorBox->run();
    }

    m_xOKBtn->connect_clicked( LINK( this, MasterPasswordDialog, OKHdl_Impl ) );
};

MasterPasswordDialog::~MasterPasswordDialog()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
