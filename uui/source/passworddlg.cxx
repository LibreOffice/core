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

#include <comphelper/string.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/configmgr.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Dialog.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <vcl/weld/weld.hxx>
#include <bitmaps.hlst>

using namespace ::com::sun::star;

PasswordDialog::PasswordDialog(weld::Window* pParent,
    task::PasswordRequestMode nDialogMode, const std::locale& rResLocale,
    const OUString& aDocURL, bool bIsPasswordToModify, bool bIsSimplePasswordRequest)
    : GenericDialogController(pParent, u"uui/ui/password.ui"_ustr, u"PasswordDialog"_ustr)
    , m_xFTPassword(m_xBuilder->weld_label(u"newpassFT"_ustr))
    , m_xEDPassword(m_xBuilder->weld_entry(u"newpassEntry"_ustr))
    , m_xFTConfirmPassword(m_xBuilder->weld_label(u"confirmpassFT"_ustr))
    , m_xEDConfirmPassword(m_xBuilder->weld_entry(u"confirmpassEntry"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xBtn1(m_xBuilder->weld_toggle_button(u"togglebt1"_ustr))
    , m_xBtn2(m_xBuilder->weld_toggle_button(u"togglebt2"_ustr))
    , nMinLen(1)
    , aPasswdMismatch(Translate::get(STR_PASSWORD_MISMATCH, rResLocale))
{
    // Disable the OK button until something has been entered in the password field
    m_xOKBtn->set_sensitive(false);
    // tdf#115964 we can be launched before the parent has resized to its final size
    m_xDialog->set_centered_on_parent(true);

    Link<weld::Toggleable&, void> aToggleLink = LINK(this, PasswordDialog, ShowHdl);
    m_xBtn1->connect_toggled(aToggleLink);
    m_xBtn2->connect_toggled(aToggleLink);

    // default settings for enter password or reenter passwd...
    m_xFTConfirmPassword->hide();
    m_xEDConfirmPassword->hide();
    m_xBtn2->hide();
    m_xFTConfirmPassword->set_sensitive(false);
    m_xEDConfirmPassword->set_sensitive(false);
    m_xBtn2->set_sensitive(false);

    // settings for create password
    if (nDialogMode == task::PasswordRequestMode_PASSWORD_CREATE)
    {
        m_xDialog->set_title(Translate::get(STR_TITLE_CREATE_PASSWORD, rResLocale));
        m_xFTConfirmPassword->set_label(Translate::get(STR_CONFIRM_SIMPLE_PASSWORD, rResLocale));

        m_xFTConfirmPassword->show();
        m_xEDConfirmPassword->show();
        m_xFTConfirmPassword->set_sensitive(true);
        m_xEDConfirmPassword->set_sensitive(true);
        m_xBtn2->show();
    }

    auto xOpenLabel = m_xBuilder->weld_label(u"openPasswordLabel"_ustr);
    auto xEditLabel = m_xBuilder->weld_label(u"editPasswordLabel"_ustr);

    xOpenLabel->set_visible(!bIsPasswordToModify);
    xEditLabel->set_visible(bIsPasswordToModify);

    OUString aMessage(Translate::get(STR_ENTER_PASSWORD_TO_OPEN, rResLocale));

    INetURLObject url(aDocURL);

    // Append filename and product name to the title for password manager compatibility
    OUString sFileName = url.getName(INetURLObject::LAST_SEGMENT, true,
                                     INetURLObject::DecodeMechanism::Unambiguous);
    if (!sFileName.isEmpty())
    {
        const OUString sFinalTitle = Translate::get(STR_TITLE_FULL_FORMAT, rResLocale)
            .replaceAll(u"%TITLE", m_xDialog->get_title())
            .replaceAll(u"%FILENAME", sFileName)
            .replaceAll(u"%PRODUCTNAME", utl::ConfigManager::getProductName());

        m_xDialog->set_title(sFinalTitle);
    }

    OUString sTooltipTextToShow = url.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);

    if (!url.HasError() && !sTooltipTextToShow.isEmpty())
    {
        OUStringBuffer sFileNameBuffer(sFileName);
        OUString sFileNameShort = comphelper::string::truncateToLength(sFileNameBuffer, 50).makeStringAndClear();

        m_xFTPassword->set_label(aMessage.replaceAll("%s", sFileNameShort));
        m_xFTPassword->set_tooltip_text(sTooltipTextToShow);
    }
    else
    {
        m_xFTPassword->set_label(aMessage.replaceAll("%s", aDocURL));
        m_xFTPassword->set_tooltip_text(aDocURL);
    }
    m_xEDPassword->grab_focus();

    if (bIsSimplePasswordRequest)
    {
        DBG_ASSERT( aDocURL.isEmpty(), "A simple password request should not have a document URL! Use document password request instead." );
        m_xFTPassword->set_label(Translate::get(STR_ENTER_SIMPLE_PASSWORD, rResLocale));
        m_xFTPassword->set_tooltip_text({});
    }

    // This signal handler enables/disables the OK button based on whether the password entry is empty
    m_xEDPassword->connect_changed(LINK(this, PasswordDialog, EnableOKBtn_Impl));

    EnableOKBtn_Impl(*m_xEDPassword);

    m_xOKBtn->connect_clicked(LINK(this, PasswordDialog, OKHdl_Impl));
}

IMPL_LINK_NOARG(PasswordDialog, EnableOKBtn_Impl, weld::Entry&, void)
{
    bool bFirstPasswordHasText = !m_xEDPassword->get_text().isEmpty();
    m_xOKBtn->set_sensitive(bFirstPasswordHasText);
}

IMPL_LINK_NOARG(PasswordDialog, OKHdl_Impl, weld::Button&, void)
{
    bool bEDPasswdValid = m_xEDPassword->get_text().getLength() >= nMinLen;
    bool bPasswdMismatch = m_xEDConfirmPassword->get_visible() && (m_xEDConfirmPassword->get_text() != m_xEDPassword->get_text());

    bool bValid = (!m_xEDConfirmPassword->get_visible() && bEDPasswdValid) ||
            (m_xEDConfirmPassword->get_visible() && bEDPasswdValid && !bPasswdMismatch);

    if (m_xEDConfirmPassword->get_visible() && bPasswdMismatch)
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  aPasswdMismatch));
        xBox->run();

        m_xEDPassword->set_text({});
        m_xEDConfirmPassword->set_text({});
        m_xEDPassword->grab_focus();
    }
    else if (bValid)
        m_xDialog->response(RET_OK);
}

IMPL_LINK(PasswordDialog, ShowHdl, weld::Toggleable& ,rToggleable, void)
{
    bool bChecked = rToggleable.get_active();
    if (&rToggleable == m_xBtn1.get())
    {
        if (bChecked)
        {
            m_xBtn1->set_from_icon_name(RID_SVXBMP_SHOWPASS);
            m_xEDPassword->set_visibility(true);
            m_xEDPassword->grab_focus();
        }
        else
        {
            m_xBtn1->set_from_icon_name(RID_SVXBMP_HIDEPASS);
            m_xEDPassword->set_visibility(false);
            m_xEDPassword->grab_focus();
        }
    }
    else if (&rToggleable == m_xBtn2.get())
    {
        if (bChecked)
        {
            m_xBtn2->set_from_icon_name(RID_SVXBMP_SHOWPASS);
            m_xEDConfirmPassword->set_visibility(true);
            m_xEDConfirmPassword->grab_focus();
        }
        else
        {
            m_xBtn2->set_from_icon_name(RID_SVXBMP_HIDEPASS);
            m_xEDConfirmPassword->set_visibility(false);
            m_xEDConfirmPassword->grab_focus();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

