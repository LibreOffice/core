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

#include <sfx2/objsh.hxx>
#include <svl/PasswordHelper.hxx>
#include <vcl/svapp.hxx>
#include <officecfg/Office/Common.hxx>
#include <passwdomdlg.hxx>
#include <strings.hrc>
#include <dialmgr.hxx>

IMPL_LINK_NOARG(PasswordToOpenModifyDialog, OkBtnClickHdl, weld::Button&, void)
{
    bool bInvalidState = !m_xOpenReadonlyCB->get_active() && !m_bAllowEmpty &&
            m_xPasswdToOpenED->get_text().isEmpty() &&
            m_xPasswdToModifyED->get_text().isEmpty();
    if (bInvalidState)
    {
        m_xErrorBox.reset(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Warning, VclButtonsType::Ok,
                                                       m_bIsPasswordToModify? m_aInvalidStateForOkButton : m_aInvalidStateForOkButton_v2));
        m_xErrorBox->runAsync(m_xErrorBox, [](sal_Int32 /*nResult*/) {});
    }
    else // check for mismatched passwords and password policy
    {
        if (m_oPasswordPolicy)
        {
            if (!SvPasswordHelper::PasswordMeetsPolicy(m_xPasswdToOpenED->get_text(),
                                                       m_oPasswordPolicy))
            {
                m_xPasswdToOpenED->grab_focus();
                return;
            }

            if (m_xOpenReadonlyCB->get_active()
                && !SvPasswordHelper::PasswordMeetsPolicy(m_xPasswdToModifyED->get_text(),
                                                          m_oPasswordPolicy))
            {
                m_xPasswdToModifyED->grab_focus();
                return;
            }
        }

        const bool bToOpenMatch     = m_xPasswdToOpenED->get_text()   == m_xReenterPasswdToOpenED->get_text();
        const bool bToModifyMatch   = m_xPasswdToModifyED->get_text() == m_xReenterPasswdToModifyED->get_text();
        const int nMismatch = (bToOpenMatch? 0 : 1) + (bToModifyMatch? 0 : 1);
        if (nMismatch > 0)
        {
            m_xErrorBox.reset(Application::CreateMessageDialog(m_xDialog.get(),
                                                           VclMessageType::Warning, VclButtonsType::Ok,
                                                           nMismatch == 1 ? m_aOneMismatch : m_aTwoMismatch));
            m_xErrorBox->runAsync(m_xErrorBox, [this, bToOpenMatch, nMismatch](sal_Int32 /*nResult*/)
            {
                weld::Entry* pEdit = !bToOpenMatch ? m_xPasswdToOpenED.get() : m_xPasswdToModifyED.get();
                weld::Entry* pRepeatEdit = !bToOpenMatch? m_xReenterPasswdToOpenED.get() : m_xReenterPasswdToModifyED.get();
                if (nMismatch == 1)
                {
                    pEdit->set_text( "" );
                    pRepeatEdit->set_text( "" );
                }
                else if (nMismatch == 2)
                {
                    m_xPasswdToOpenED->set_text( "" );
                    m_xReenterPasswdToOpenED->set_text( "" );
                    m_xPasswdToModifyED->set_text( "" );
                    m_xReenterPasswdToModifyED->set_text( "" );
                }
                pEdit->grab_focus();
            });
        }
        else
        {
            m_xDialog->response(RET_OK);
        }
    }
}

IMPL_LINK(PasswordToOpenModifyDialog, ChangeHdl, weld::Entry&, rEntry, void)
{
    auto aPasswordText = rEntry.get_text();

    weld::Label* pIndicator = nullptr;
    weld::LevelBar* pLevelBar = nullptr;
    if (&rEntry == m_xPasswdToOpenED.get())
    {
        pIndicator = m_xPasswdToOpenInd.get();
        pLevelBar = m_xPasswdToOpenBar.get();
    }
    else if (&rEntry == m_xReenterPasswdToOpenED.get())
    {
        pIndicator = m_xReenterPasswdToOpenInd.get();
    }
    else if (&rEntry == m_xPasswdToModifyED.get())
    {
        pIndicator = m_xPasswdToModifyInd.get();
        pLevelBar = m_xPasswdToModifyBar.get();
    }
    else if (&rEntry == m_xReenterPasswdToModifyED.get())
    {
        pIndicator = m_xReenterPasswdToModifyInd.get();
    }
    assert(pIndicator);

    bool bPasswordMeetsPolicy
        = SvPasswordHelper::PasswordMeetsPolicy(aPasswordText, m_oPasswordPolicy);
    if (pLevelBar)
    {
        rEntry.set_message_type(bPasswordMeetsPolicy ? weld::EntryMessageType::Normal
                                                     : weld::EntryMessageType::Error);
        pIndicator->set_visible(!bPasswordMeetsPolicy);
    }

    // if password doesn't meet policy cap the percentage at 70%
    if (pLevelBar)
        pLevelBar->set_percentage(
            std::min(SvPasswordHelper::GetPasswordStrengthPercentage(aPasswordText),
                     bPasswordMeetsPolicy ? std::numeric_limits<double>::max() : 70.0));

    if (m_nMaxPasswdLen)
    {
        int nLength = aPasswordText.getLength();
        pIndicator->set_visible(nLength >= m_nMaxPasswdLen);
    }
}

PasswordToOpenModifyDialog::PasswordToOpenModifyDialog(weld::Window * pParent, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify)
    : SfxDialogController(pParent, "cui/ui/password.ui", "PasswordDialog")
    , m_xPasswdToOpenED(m_xBuilder->weld_entry("newpassEntry"))
    , m_xPasswdToOpenInd(m_xBuilder->weld_label("newpassIndicator"))
    , m_xPasswdToOpenBar(m_xBuilder->weld_level_bar("passlevelbar"))
    , m_xReenterPasswdToOpenED(m_xBuilder->weld_entry("confirmpassEntry"))
    , m_xReenterPasswdToOpenInd(m_xBuilder->weld_label("confirmpassIndicator"))
    , m_xOptionsExpander(m_xBuilder->weld_expander("expander"))
    , m_xOk(m_xBuilder->weld_button("ok"))
    , m_xOpenReadonlyCB(m_xBuilder->weld_check_button("readonly"))
    , m_xPasswdToModifyFT(m_xBuilder->weld_label("label7"))
    , m_xPasswdToModifyED(m_xBuilder->weld_entry("newpassroEntry"))
    , m_xPasswdToModifyInd(m_xBuilder->weld_label("newpassroIndicator"))
    , m_xPasswdToModifyBar(m_xBuilder->weld_level_bar("ropasslevelbar"))
    , m_xReenterPasswdToModifyFT(m_xBuilder->weld_label("label8"))
    , m_xReenterPasswdToModifyED(m_xBuilder->weld_entry("confirmropassEntry"))
    , m_xReenterPasswdToModifyInd(m_xBuilder->weld_label("confirmropassIndicator"))
    , m_aOneMismatch( CuiResId( RID_CUISTR_ONE_PASSWORD_MISMATCH ) )
    , m_aTwoMismatch( CuiResId( RID_CUISTR_TWO_PASSWORDS_MISMATCH ) )
    , m_aInvalidStateForOkButton( CuiResId( RID_CUISTR_INVALID_STATE_FOR_OK_BUTTON ) )
    , m_aInvalidStateForOkButton_v2( CuiResId( RID_CUISTR_INVALID_STATE_FOR_OK_BUTTON_V2 ) )
    , m_oPasswordPolicy(officecfg::Office::Common::Security::Scripting::PasswordPolicy::get())
    , m_nMaxPasswdLen(nMaxPasswdLen)
    , m_bIsPasswordToModify( bIsPasswordToModify )
    , m_bAllowEmpty( false )
{
    m_xOk->connect_clicked(LINK(this, PasswordToOpenModifyDialog, OkBtnClickHdl));
    m_xPasswdToOpenED->connect_changed(LINK(this, PasswordToOpenModifyDialog, ChangeHdl));
    m_xPasswdToModifyED->connect_changed(LINK(this, PasswordToOpenModifyDialog, ChangeHdl));
    if(m_oPasswordPolicy || nMaxPasswdLen)
    {
        m_xReenterPasswdToOpenED->connect_changed(LINK(this, PasswordToOpenModifyDialog, ChangeHdl));
        m_xReenterPasswdToModifyED->connect_changed(LINK(this, PasswordToOpenModifyDialog, ChangeHdl));

        OUString aIndicatorText{};
        OUString aMaxPassLengthIndicator{ CuiResId(RID_CUISTR_PASSWORD_LEN_INDICATOR)
                                              .replaceFirst("%1",
                                                            OUString::number(nMaxPasswdLen)) };
        if (m_oPasswordPolicy && nMaxPasswdLen)
        {
            aIndicatorText
                = officecfg::Office::Common::Security::Scripting::PasswordPolicyErrorMessage::get()
                  + "\n" + aMaxPassLengthIndicator;
        }
        else if (m_oPasswordPolicy)
        {
            aIndicatorText
                = officecfg::Office::Common::Security::Scripting::PasswordPolicyErrorMessage::get();
        }
        else if (nMaxPasswdLen)
        {
            aIndicatorText = aMaxPassLengthIndicator;
        }

        m_xPasswdToOpenInd->set_label(aIndicatorText);
        m_xReenterPasswdToOpenInd->set_label(aMaxPassLengthIndicator);
        m_xPasswdToModifyInd->set_label(aIndicatorText);
        m_xReenterPasswdToModifyInd->set_label(aMaxPassLengthIndicator);

        if (nMaxPasswdLen)
        {
            m_xPasswdToOpenED->set_max_length(nMaxPasswdLen);
            m_xReenterPasswdToOpenED->set_max_length(nMaxPasswdLen);
            m_xPasswdToModifyED->set_max_length(nMaxPasswdLen);
            m_xReenterPasswdToModifyED->set_max_length(nMaxPasswdLen);
        }
    }

    m_xPasswdToOpenED->grab_focus();

    m_xOptionsExpander->set_sensitive(bIsPasswordToModify);
    if (!bIsPasswordToModify)
        m_xOptionsExpander->hide();
    else if (SfxObjectShell* pSh = SfxObjectShell::Current())
    {
        if (pSh->IsLoadReadonly())
        {
            m_xOpenReadonlyCB->set_active(true);
            m_xOptionsExpander->set_expanded(true);
        }
    }

    m_xOpenReadonlyCB->connect_toggled(LINK(this, PasswordToOpenModifyDialog, ReadonlyOnOffHdl));
    ReadonlyOnOffHdl(*m_xOpenReadonlyCB);
}

PasswordToOpenModifyDialog::~PasswordToOpenModifyDialog()
{
    if (m_xErrorBox)
    {
        m_xErrorBox->response(RET_CANCEL);
    }
}

void PasswordToOpenModifyDialog::AllowEmpty()
{
    m_bAllowEmpty = true;
}

OUString PasswordToOpenModifyDialog::GetPasswordToOpen() const
{
    const bool bPasswdOk =
            !m_xPasswdToOpenED->get_text().isEmpty() &&
            m_xPasswdToOpenED->get_text() == m_xReenterPasswdToOpenED->get_text();
    return bPasswdOk ? m_xPasswdToOpenED->get_text() : OUString();
}


OUString PasswordToOpenModifyDialog::GetPasswordToModify() const
{
    const bool bPasswdOk =
            !m_xPasswdToModifyED->get_text().isEmpty() &&
            m_xPasswdToModifyED->get_text() == m_xReenterPasswdToModifyED->get_text();
    return bPasswdOk ? m_xPasswdToModifyED->get_text() : OUString();
}


bool PasswordToOpenModifyDialog::IsRecommendToOpenReadonly() const
{
    return m_xOpenReadonlyCB->get_active();
}

IMPL_LINK_NOARG(PasswordToOpenModifyDialog, ReadonlyOnOffHdl, weld::Toggleable&, void)
{
    bool bEnable = m_xOpenReadonlyCB->get_active();
    m_xPasswdToModifyED->set_sensitive(bEnable);
    m_xPasswdToModifyFT->set_sensitive(bEnable);
    m_xReenterPasswdToModifyED->set_sensitive(bEnable);
    m_xReenterPasswdToModifyFT->set_sensitive(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
