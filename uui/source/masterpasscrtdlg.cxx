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
#include "masterpasscrtdlg.hxx"

// MasterPasswordCreateDialog---------------------------------------------------


IMPL_LINK_NOARG(MasterPasswordCreateDialog, EditHdl_Impl, weld::Entry&, void)
{
    m_xOKBtn->set_sensitive(m_xEDMasterPasswordCrt->get_text().getLength() >= 1);
}

IMPL_LINK_NOARG(MasterPasswordCreateDialog, OKHdl_Impl, weld::Button&, void)
{
    // compare both passwords and show message box if there are not equal!!
    if (m_xEDMasterPasswordCrt->get_text() == m_xEDMasterPasswordRepeat->get_text())
        m_xDialog->response(RET_OK);
    else
    {
        OUString aErrorMsg(Translate::get(STR_ERROR_PASSWORDS_NOT_IDENTICAL, rResLocale));
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  aErrorMsg));
        xErrorBox->run();
        m_xEDMasterPasswordCrt->set_text( OUString() );
        m_xEDMasterPasswordRepeat->set_text( OUString() );
        m_xEDMasterPasswordCrt->grab_focus();
    }
}

MasterPasswordCreateDialog::MasterPasswordCreateDialog(weld::Window* pParent, const std::locale& rLocale)
    : GenericDialogController(pParent, "uui/ui/setmasterpassworddlg.ui", "SetMasterPasswordDialog")
    , rResLocale(rLocale)
    , m_xEDMasterPasswordCrt(m_xBuilder->weld_entry("password1"))
    , m_xEDMasterPasswordRepeat(m_xBuilder->weld_entry("password2"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
{
    m_xOKBtn->set_sensitive(false);
    m_xOKBtn->connect_clicked( LINK( this, MasterPasswordCreateDialog, OKHdl_Impl ) );
    m_xEDMasterPasswordCrt->connect_changed( LINK( this, MasterPasswordCreateDialog, EditHdl_Impl ) );
}

MasterPasswordCreateDialog::~MasterPasswordCreateDialog()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
