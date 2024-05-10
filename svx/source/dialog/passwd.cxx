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

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svx/passwd.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

IMPL_LINK_NOARG(SvxPasswordDialog, ButtonHdl, weld::Button&, void)
{
    bool bOK = true;

    if (m_xNewPasswdED->get_text() != m_xRepeatPasswdED->get_text())
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                 VclMessageType::Warning, VclButtonsType::Ok,
                                                                 m_aRepeatPasswdErrStr));
        xBox->run();
        m_xNewPasswdED->set_text(u""_ustr);
        m_xRepeatPasswdED->set_text(u""_ustr);
        m_xNewPasswdED->grab_focus();
        bOK = false;
    }

    if (bOK && m_aCheckPasswordHdl.IsSet() && !m_aCheckPasswordHdl.Call(this))
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                 VclMessageType::Warning, VclButtonsType::Ok,
                                                                 m_aOldPasswdErrStr));
        xBox->run();
        m_xOldPasswdED->set_text(u""_ustr);
        m_xOldPasswdED->grab_focus();
        bOK = false;
    }

    if (bOK)
        m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SvxPasswordDialog, EditModifyHdl, weld::Entry&, void)
{
    if (!m_xOKBtn->get_sensitive())
        m_xOKBtn->set_sensitive(true);
}

SvxPasswordDialog::SvxPasswordDialog(weld::Window* pParent, bool bDisableOldPassword)
    : SfxDialogController(pParent, u"svx/ui/passwd.ui"_ustr, u"PasswordDialog"_ustr)
    , m_aOldPasswdErrStr(SvxResId(RID_SVXSTR_ERR_OLD_PASSWD))
    , m_aRepeatPasswdErrStr(SvxResId(RID_SVXSTR_ERR_REPEAT_PASSWD ))
    , m_xOldFL(m_xBuilder->weld_label(u"oldpass"_ustr))
    , m_xOldPasswdFT(m_xBuilder->weld_label(u"oldpassL"_ustr))
    , m_xOldPasswdED(m_xBuilder->weld_entry(u"oldpassEntry"_ustr))
    , m_xNewPasswdED(m_xBuilder->weld_entry(u"newpassEntry"_ustr))
    , m_xRepeatPasswdED(m_xBuilder->weld_entry(u"confirmpassEntry"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xOKBtn->connect_clicked(LINK(this, SvxPasswordDialog, ButtonHdl));
    m_xRepeatPasswdED->connect_changed(LINK(this, SvxPasswordDialog, EditModifyHdl));
    EditModifyHdl(*m_xRepeatPasswdED);

    if (bDisableOldPassword)
    {
        m_xOldFL->set_sensitive(false);
        m_xOldPasswdFT->set_sensitive(false);
        m_xOldPasswdED->set_sensitive(false);
        m_xNewPasswdED->grab_focus();
    }
}

SvxPasswordDialog::~SvxPasswordDialog()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
