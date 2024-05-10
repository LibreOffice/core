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
#include <sfx2/passwd.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <svl/PasswordHelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

IMPL_LINK_NOARG(SfxPasswordDialog, EditModifyHdl, weld::Entry&, void)
{
    ModifyHdl();
}

void SfxPasswordDialog::ModifyHdl()
{
    OUString aPassword1Text = m_xPassword1ED->get_text();
    bool bEnable = aPassword1Text.getLength() >= mnMinLen;
    if (m_xPassword2ED->get_visible())
        bEnable = (bEnable && (m_xPassword2ED->get_text().getLength() >= mnMinLen));
    m_xOKBtn->set_sensitive(bEnable);

    // if there's a confirm entry, the dialog is being used for setting a password
    if (m_xConfirm1ED->get_visible())
    {
        m_xPassword1StrengthBar->set_percentage(
            SvPasswordHelper::GetPasswordStrengthPercentage(aPassword1Text));
        bool bPasswordMeetsPolicy = SvPasswordHelper::PasswordMeetsPolicy(
            aPassword1Text, moPasswordPolicy);
        m_xPassword1ED->set_message_type(bPasswordMeetsPolicy ? weld::EntryMessageType::Normal
                                                              : weld::EntryMessageType::Error);
        m_xPassword1PolicyLabel->set_visible(!bPasswordMeetsPolicy);
    }

    // if there's a confirm entry, the dialog is being used for setting a password
    if (m_xConfirm2ED->get_visible())
    {
        OUString aPassword2Text = m_xPassword2ED->get_text();

        m_xPassword2StrengthBar->set_percentage(
            SvPasswordHelper::GetPasswordStrengthPercentage(m_xPassword2ED->get_text()));

        // second password is optional, ignore policy if it is empty
        bool bPasswordMeetsPolicy
            = aPassword2Text.isEmpty()
                  ? true
                  : SvPasswordHelper::PasswordMeetsPolicy(
                      aPassword2Text, moPasswordPolicy);
        m_xPassword2ED->set_message_type(bPasswordMeetsPolicy ? weld::EntryMessageType::Normal
                                                              : weld::EntryMessageType::Error);
        m_xPassword2PolicyLabel->set_visible(!bPasswordMeetsPolicy);
    }
}

IMPL_LINK(SfxPasswordDialog, InsertTextHdl, OUString&, rTest, bool)
{
    if (!mbAsciiOnly)
        return true;

    const sal_Unicode* pTest = rTest.getStr();
    sal_Int32 nLen = rTest.getLength();
    OUStringBuffer aFilter(nLen);
    bool bReset = false;
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        if( *pTest > 0x007f )
            bReset = true;
        else
            aFilter.append(*pTest);
        ++pTest;
    }

    if (bReset)
    {
        rTest = aFilter.makeStringAndClear();
        // upgrade from "Normal" to "Warning" if a invalid letter was
        // discarded
        m_xOnlyAsciiFT->set_label_type(weld::LabelType::Warning);
    }

    return true;
}

IMPL_LINK_NOARG(SfxPasswordDialog, OKHdl, weld::Button&, void)
{
    if (m_xConfirm1ED->get_visible()
        && !SvPasswordHelper::PasswordMeetsPolicy(GetPassword(), moPasswordPolicy))
    {
        m_xPassword1ED->grab_focus();
        return;
    }
    if (m_xConfirm2ED->get_visible() && !GetPassword2().isEmpty()
        && !SvPasswordHelper::PasswordMeetsPolicy(GetPassword2(), moPasswordPolicy))
    {
        m_xPassword2ED->grab_focus();
        return;
    }

    bool bConfirmFailed = bool( mnExtras & SfxShowExtras::CONFIRM ) &&
                          ( GetConfirm() != GetPassword() );
    if( ( mnExtras & SfxShowExtras::CONFIRM2 ) && ( m_xConfirm2ED->get_text() != GetPassword2() ) )
        bConfirmFailed = true;
    if ( bConfirmFailed )
    {
        if (m_xConfirmFailedDialog)
            m_xConfirmFailedDialog->response(RET_CANCEL);

        m_xConfirmFailedDialog =
            std::shared_ptr<weld::MessageDialog>(Application::CreateMessageDialog(m_xDialog.get(),
                                                    VclMessageType::Warning, VclButtonsType::Ok,
                                                    SfxResId(STR_ERROR_WRONG_CONFIRM)));
        m_xConfirmFailedDialog->runAsync(m_xConfirmFailedDialog, [this](sal_uInt32 response){
            m_xConfirm1ED->set_text(OUString());
            m_xConfirm1ED->grab_focus();
            m_xConfirmFailedDialog->response(response);
         });
    }
    else
        m_xDialog->response(RET_OK);
}

// CTOR / DTOR -----------------------------------------------------------

SfxPasswordDialog::SfxPasswordDialog(weld::Widget* pParent, const OUString* pGroupText)
    : GenericDialogController(pParent, u"sfx/ui/password.ui"_ustr, u"PasswordDialog"_ustr)
    , m_xPassword1Box(m_xBuilder->weld_frame(u"password1frame"_ustr))
    , m_xUserFT(m_xBuilder->weld_label(u"userft"_ustr))
    , m_xUserED(m_xBuilder->weld_entry(u"usered"_ustr))
    , m_xPassword1FT(m_xBuilder->weld_label(u"pass1ft"_ustr))
    , m_xPassword1ED(m_xBuilder->weld_entry(u"pass1ed"_ustr))
    , m_xPassword1StrengthBar(m_xBuilder->weld_level_bar(u"pass1bar"_ustr))
    , m_xPassword1PolicyLabel(m_xBuilder->weld_label(u"pass1policylabel"_ustr))
    , m_xConfirm1FT(m_xBuilder->weld_label(u"confirm1ft"_ustr))
    , m_xConfirm1ED(m_xBuilder->weld_entry(u"confirm1ed"_ustr))
    , m_xPassword2Box(m_xBuilder->weld_frame(u"password2frame"_ustr))
    , m_xPassword2FT(m_xBuilder->weld_label(u"pass2ft"_ustr))
    , m_xPassword2ED(m_xBuilder->weld_entry(u"pass2ed"_ustr))
    , m_xPassword2StrengthBar(m_xBuilder->weld_level_bar(u"pass2bar"_ustr))
    , m_xPassword2PolicyLabel(m_xBuilder->weld_label(u"pass2policylabel"_ustr))
    , m_xConfirm2FT(m_xBuilder->weld_label(u"confirm2ft"_ustr))
    , m_xConfirm2ED(m_xBuilder->weld_entry(u"confirm2ed"_ustr))
    , m_xMinLengthFT(m_xBuilder->weld_label(u"minlenft"_ustr))
    , m_xOnlyAsciiFT(m_xBuilder->weld_label(u"onlyascii"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , maMinLenPwdStr(SfxResId(STR_PASSWD_MIN_LEN))
    , maMinLenPwdStr1(SfxResId(STR_PASSWD_MIN_LEN1))
    , maEmptyPwdStr(SfxResId(STR_PASSWD_EMPTY))
    , mnMinLen(5)
    , mnExtras(SfxShowExtras::NONE)
    , moPasswordPolicy(officecfg::Office::Common:: Security::Scripting::PasswordPolicy::get())
    , mbAsciiOnly(false)
{
    Link<weld::Entry&,void> aLink = LINK(this, SfxPasswordDialog, EditModifyHdl);
    m_xPassword1ED->connect_changed(aLink);
    m_xPassword2ED->connect_changed(aLink);
    Link<OUString&,bool> aLink2 = LINK(this, SfxPasswordDialog, InsertTextHdl);
    m_xPassword1ED->connect_insert_text(aLink2);
    m_xPassword2ED->connect_insert_text(aLink2);
    m_xConfirm1ED->connect_insert_text(aLink2);
    m_xConfirm2ED->connect_insert_text(aLink2);
    m_xOKBtn->connect_clicked(LINK(this, SfxPasswordDialog, OKHdl));

    if(moPasswordPolicy)
    {
        m_xPassword1PolicyLabel->set_label(
            officecfg::Office::Common::Security::Scripting::PasswordPolicyErrorMessage::get());
        m_xPassword2PolicyLabel->set_label(
            officecfg::Office::Common::Security::Scripting::PasswordPolicyErrorMessage::get());
    }

    if (pGroupText)
        m_xPassword1Box->set_label(*pGroupText);

    //set the text to the password length
    SetPasswdText();
}

void SfxPasswordDialog::SetPasswdText( )
{
    //set the new string to the minimum password length
    if (mnMinLen == 0)
        m_xMinLengthFT->set_label(maEmptyPwdStr);
    else
    {
        if( mnMinLen == 1 )
            m_xMinLengthFT->set_label(maMinLenPwdStr1);
        else
        {
            maMainPwdStr = maMinLenPwdStr;
            maMainPwdStr = maMainPwdStr.replaceAll( "$(MINLEN)", OUString::number(static_cast<sal_Int32>(mnMinLen) ) );
            m_xMinLengthFT->set_label(maMainPwdStr);
        }
    }
}


void SfxPasswordDialog::SetMinLen( sal_uInt16 nLen )
{
    mnMinLen = nLen;
    SetPasswdText();
    ModifyHdl();
}

void SfxPasswordDialog::ShowMinLengthText(bool bShow)
{
    m_xMinLengthFT->set_visible(bShow);
}

void SfxPasswordDialog::AllowAsciiOnly()
{
    mbAsciiOnly = true;
    m_xOnlyAsciiFT->show();
}

void SfxPasswordDialog::PreRun()
{
    m_xUserFT->hide();
    m_xUserED->hide();
    m_xConfirm1FT->hide();
    m_xConfirm1ED->hide();
    m_xPassword1StrengthBar->hide();
    m_xPassword1FT->hide();
    m_xPassword2Box->hide();
    m_xPassword2FT->hide();
    m_xPassword2ED->hide();
    m_xPassword2FT->hide();
    m_xConfirm2FT->hide();
    m_xConfirm2ED->hide();
    m_xPassword2StrengthBar->hide();

    if (mnExtras != SfxShowExtras::NONE)
        m_xPassword1FT->show();
    if (mnExtras & SfxShowExtras::USER)
    {
        m_xUserFT->show();
        m_xUserED->show();
    }
    if (mnExtras & SfxShowExtras::CONFIRM)
    {
        m_xConfirm1FT->show();
        m_xConfirm1ED->show();
        m_xPassword1StrengthBar->show();
    }
    if (mnExtras & SfxShowExtras::PASSWORD2)
    {
        m_xPassword2Box->show();
        m_xPassword2FT->show();
        m_xPassword2ED->show();
    }
    if (mnExtras & SfxShowExtras::CONFIRM2)
    {
        m_xConfirm2FT->show();
        m_xConfirm2ED->show();
        m_xPassword2StrengthBar->show();
    }
}

short SfxPasswordDialog::run()
{
    PreRun();

    return GenericDialogController::run();
}

SfxPasswordDialog::~SfxPasswordDialog()
{
    if (m_xConfirmFailedDialog)
        m_xConfirmFailedDialog->response(RET_CANCEL);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
