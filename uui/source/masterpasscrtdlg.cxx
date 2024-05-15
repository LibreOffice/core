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

#include <officecfg/Office/Common.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <strings.hrc>
#include <svl/PasswordHelper.hxx>
#include "masterpasscrtdlg.hxx"

// MasterPasswordCreateDialog---------------------------------------------------


IMPL_LINK_NOARG(MasterPasswordCreateDialog, EditHdl_Impl, weld::Entry&, void)
{
    OUString aPasswordText = m_xEDMasterPasswordCrt->get_text();
    m_xOKBtn->set_sensitive(aPasswordText.getLength() >= 1);
    m_xPasswdStrengthBar->set_percentage(
        SvPasswordHelper::GetPasswordStrengthPercentage(aPasswordText));

    if(m_oPasswordPolicy)
    {
        bool bPasswordMeetsPolicy
            = SvPasswordHelper::PasswordMeetsPolicy(aPasswordText, m_oPasswordPolicy);
        m_xEDMasterPasswordCrt->set_message_type(bPasswordMeetsPolicy ? weld::EntryMessageType::Normal
                                                 : weld::EntryMessageType::Error);
        m_xPasswordPolicyLabel->set_visible(!bPasswordMeetsPolicy);
    }
}

IMPL_LINK_NOARG(MasterPasswordCreateDialog, OKHdl_Impl, weld::Button&, void)
{
    if (m_oPasswordPolicy
        && !SvPasswordHelper::PasswordMeetsPolicy(m_xEDMasterPasswordCrt->get_text(),
                                                  m_oPasswordPolicy))
    {
        m_xEDMasterPasswordCrt->grab_focus();
        return;
    }

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
    : GenericDialogController(pParent, u"uui/ui/setmasterpassworddlg.ui"_ustr, u"SetMasterPasswordDialog"_ustr)
    , rResLocale(rLocale)
    , m_xEDMasterPasswordCrt(m_xBuilder->weld_entry(u"password1"_ustr))
    , m_xPasswordPolicyLabel(m_xBuilder->weld_label(u"passpolicylabel"_ustr))
    , m_xEDMasterPasswordRepeat(m_xBuilder->weld_entry(u"password2"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xPasswdStrengthBar(m_xBuilder->weld_level_bar(u"password1levelbar"_ustr))
    , m_oPasswordPolicy(officecfg::Office::Common::Security::Scripting::PasswordPolicy::get())
{
    m_xOKBtn->set_sensitive(false);
    m_xOKBtn->connect_clicked( LINK( this, MasterPasswordCreateDialog, OKHdl_Impl ) );
    m_xEDMasterPasswordCrt->connect_changed( LINK( this, MasterPasswordCreateDialog, EditHdl_Impl ) );
    if (m_oPasswordPolicy)
    {
        m_xPasswordPolicyLabel->set_label(
            officecfg::Office::Common::Security::Scripting::PasswordPolicyErrorMessage::get());
    }
}

MasterPasswordCreateDialog::~MasterPasswordCreateDialog()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
