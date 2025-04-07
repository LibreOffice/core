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

#include "passworddlg.hxx"
#include <strings.hrc>

#include <unotools/resmgr.hxx>
#include <unotools/configmgr.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <bitmaps.hlst>

using namespace ::com::sun::star;

PasswordDialog::PasswordDialog(weld::Window* pParent,
    task::PasswordRequestMode nDialogMode, const std::locale& rResLocale,
    const OUString& aDocURL, bool bOpenToModify, bool bIsSimplePasswordRequest)
    : GenericDialogController(pParent, u"uui/ui/password.ui"_ustr, u"PasswordDialog"_ustr)
    , m_xFTPassword(m_xBuilder->weld_label(u"newpassFT"_ustr))
    , m_xEDPassword(m_xBuilder->weld_entry(u"newpassEntry"_ustr))
    , m_xFTConfirmPassword(m_xBuilder->weld_label(u"confirmpassFT"_ustr))
    , m_xEDConfirmPassword(m_xBuilder->weld_entry(u"confirmpassEntry"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , nMinLen(1)
    , aPasswdMismatch(Translate::get(STR_PASSWORD_MISMATCH, rResLocale))
{
    // tdf#115964 we can be launched before the parent has resized to its final size
    m_xDialog->set_centered_on_parent(true);

    m_xPass[0] = m_xBuilder->weld_toggle_button(u"togglebt1"_ustr);
    m_xPass[1] = m_xBuilder->weld_toggle_button(u"togglebt2"_ustr);

    Link<weld::Toggleable&, void> aToggleLink = LINK(this, PasswordDialog, ShowHdl);

    for (auto& aPass : m_xPass)
    {
        if (aPass->get_active())
            aPass->set_from_icon_name(RID_SVXBMP_SHOWPASS);
        else
            aPass->set_from_icon_name(RID_SVXBMP_HIDEPASS);
        aPass->connect_toggled(aToggleLink);
    }

    // default settings for enter password or reenter passwd...
    OUString aTitle(Translate::get(STR_TITLE_ENTER_PASSWORD, rResLocale));
    m_xFTConfirmPassword->hide();
    m_xEDConfirmPassword->hide();
    m_xPass[1]->hide();
    m_xFTConfirmPassword->set_sensitive(false);
    m_xEDConfirmPassword->set_sensitive(false);
    m_xPass[1]->set_sensitive(false);

    // settings for create password
    if (nDialogMode == task::PasswordRequestMode_PASSWORD_CREATE)
    {
        aTitle = Translate::get(STR_TITLE_CREATE_PASSWORD, rResLocale);

        m_xFTConfirmPassword->set_label(Translate::get(STR_CONFIRM_SIMPLE_PASSWORD, rResLocale));

        m_xFTConfirmPassword->show();
        m_xEDConfirmPassword->show();
        m_xFTConfirmPassword->set_sensitive(true);
        m_xEDConfirmPassword->set_sensitive(true);
    }

    m_xDialog->set_title(aTitle);

    TranslateId pStrId = bOpenToModify ? STR_ENTER_PASSWORD_TO_MODIFY : STR_ENTER_PASSWORD_TO_OPEN;
    OUString aMessage(Translate::get(pStrId, rResLocale));
    INetURLObject url(aDocURL);

    // tdf#66553 - add file name to title bar for password managers
    OUString aFileName = url.getName(INetURLObject::LAST_SEGMENT, true,
                                     INetURLObject::DecodeMechanism::Unambiguous);
    if (!aFileName.isEmpty())
        aFileName += " - " + utl::ConfigManager::getProductName();
    m_xDialog->set_title(aTitle + " - " + aFileName);

    auto aUrl = url.HasError()
        ? aDocURL : url.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);
    aMessage += m_xFTPassword->escape_ui_str(aUrl);
    m_xFTPassword->set_label(aMessage);

    if (bIsSimplePasswordRequest)
    {
        DBG_ASSERT( aDocURL.isEmpty(), "A simple password request should not have a document URL! Use document password request instead." );
        m_xFTPassword->set_label(Translate::get(STR_ENTER_SIMPLE_PASSWORD, rResLocale));
    }

    m_xOKBtn->connect_clicked(LINK(this, PasswordDialog, OKHdl_Impl));
}

IMPL_LINK_NOARG(PasswordDialog, OKHdl_Impl, weld::Button&, void)
{
    bool bEDPasswdValid = m_xEDPassword->get_text().getLength() >= nMinLen;
    bool bPasswdMismatch = m_xEDConfirmPassword->get_text() != m_xEDPassword->get_text();
    bool bValid = (!m_xEDConfirmPassword->get_visible() && bEDPasswdValid) ||
            (m_xEDConfirmPassword->get_visible() && bEDPasswdValid && !bPasswdMismatch);

    if (m_xEDConfirmPassword->get_visible() && bPasswdMismatch)
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  aPasswdMismatch));
        xBox->run();
    }
    else if (bValid)
        m_xDialog->response(RET_OK);
}

IMPL_LINK(PasswordDialog, ShowHdl, weld::Toggleable& ,rToggleable, void)
{
    bool bChecked = rToggleable.get_active();
    if (&rToggleable == m_xPass[0].get())
    {
        if (bChecked)
        {
            m_xPass[0]->set_from_icon_name(RID_SVXBMP_SHOWPASS);
            m_xEDPassword->set_visibility(true);
            m_xEDPassword->grab_focus();
        }
        else
        {
            m_xPass[0]->set_from_icon_name(RID_SVXBMP_HIDEPASS);
            m_xEDPassword->set_visibility(false);
            m_xEDPassword->grab_focus();
        }
    }
    else if (&rToggleable == m_xPass[1].get())
    {
        if (bChecked)
        {
            m_xPass[1]->set_from_icon_name(RID_SVXBMP_SHOWPASS);
            m_xEDConfirmPassword->set_visibility(true);
            m_xEDConfirmPassword->grab_focus();
        }
        else
        {
            m_xPass[1]->set_from_icon_name(RID_SVXBMP_HIDEPASS);
            m_xEDConfirmPassword->set_visibility(false);
            m_xEDConfirmPassword->grab_focus();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

