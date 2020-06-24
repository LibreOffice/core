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
#include <passwdomdlg.hxx>
#include <strings.hrc>
#include <dialmgr.hxx>

IMPL_LINK_NOARG(PasswordToOpenModifyDialog, OkBtnClickHdl, weld::Button&, void)
{
    bool bInvalidState = !m_xOpenReadonlyCB->get_active() &&
            m_xPasswdToOpenED->get_text().isEmpty() &&
            m_xPasswdToModifyED->get_text().isEmpty();
    if (bInvalidState)
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Warning, VclButtonsType::Ok,
                                                       m_bIsPasswordToModify? m_aInvalidStateForOkButton : m_aInvalidStateForOkButton_v2));
        xErrorBox->run();
    }
    else // check for mismatched passwords...
    {
        const bool bToOpenMatch     = m_xPasswdToOpenED->get_text()   == m_xReenterPasswdToOpenED->get_text();
        const bool bToModifyMatch   = m_xPasswdToModifyED->get_text() == m_xReenterPasswdToModifyED->get_text();
        const int nMismatch = (bToOpenMatch? 0 : 1) + (bToModifyMatch? 0 : 1);
        if (nMismatch > 0)
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                           VclMessageType::Warning, VclButtonsType::Ok,
                                                           nMismatch == 1 ? m_aOneMismatch : m_aTwoMismatch));
            xErrorBox->run();

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
        }
        else
        {
            m_xDialog->response(RET_OK);
        }
    }
}

IMPL_LINK(PasswordToOpenModifyDialog, ChangeHdl, weld::Entry&, rEntry, void)
{
    weld::Label* pIndicator = nullptr;
    int nLength = rEntry.get_text().getLength();
    if (&rEntry == m_xPasswdToOpenED.get())
        pIndicator = m_xPasswdToOpenInd.get();
    else if (&rEntry == m_xReenterPasswdToOpenED.get())
        pIndicator = m_xReenterPasswdToOpenInd.get();
    else if (&rEntry == m_xPasswdToModifyED.get())
        pIndicator = m_xPasswdToModifyInd.get();
    else if (&rEntry == m_xReenterPasswdToModifyED.get())
        pIndicator = m_xReenterPasswdToModifyInd.get();
    assert(pIndicator);
    pIndicator->set_visible(nLength >= m_nMaxPasswdLen);
}

PasswordToOpenModifyDialog::PasswordToOpenModifyDialog(weld::Window * pParent, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify)
    : SfxDialogController(pParent, "cui/ui/password.ui", "PasswordDialog")
    , m_xPasswdToOpenED(m_xBuilder->weld_entry("newpassEntry"))
    , m_xPasswdToOpenInd(m_xBuilder->weld_label("newpassIndicator"))
    , m_xReenterPasswdToOpenED(m_xBuilder->weld_entry("confirmpassEntry"))
    , m_xReenterPasswdToOpenInd(m_xBuilder->weld_label("confirmpassIndicator"))
    , m_xOptionsExpander(m_xBuilder->weld_expander("expander"))
    , m_xOk(m_xBuilder->weld_button("ok"))
    , m_xOpenReadonlyCB(m_xBuilder->weld_check_button("readonly"))
    , m_xPasswdToModifyFT(m_xBuilder->weld_label("label7"))
    , m_xPasswdToModifyED(m_xBuilder->weld_entry("newpassroEntry"))
    , m_xPasswdToModifyInd(m_xBuilder->weld_label("newpassroIndicator"))
    , m_xReenterPasswdToModifyFT(m_xBuilder->weld_label("label8"))
    , m_xReenterPasswdToModifyED(m_xBuilder->weld_entry("confirmropassEntry"))
    , m_xReenterPasswdToModifyInd(m_xBuilder->weld_label("confirmropassIndicator"))
    , m_aOneMismatch( CuiResId( RID_SVXSTR_ONE_PASSWORD_MISMATCH ) )
    , m_aTwoMismatch( CuiResId( RID_SVXSTR_TWO_PASSWORDS_MISMATCH ) )
    , m_aInvalidStateForOkButton( CuiResId( RID_SVXSTR_INVALID_STATE_FOR_OK_BUTTON ) )
    , m_aInvalidStateForOkButton_v2( CuiResId( RID_SVXSTR_INVALID_STATE_FOR_OK_BUTTON_V2 ) )
    , m_nMaxPasswdLen(nMaxPasswdLen)
    , m_bIsPasswordToModify( bIsPasswordToModify )
{
    m_xOk->connect_clicked(LINK(this, PasswordToOpenModifyDialog, OkBtnClickHdl));

    if (nMaxPasswdLen)
    {
        OUString aIndicatorTemplate(CuiResId(RID_SVXSTR_PASSWORD_LEN_INDICATOR).replaceFirst("%1", OUString::number(nMaxPasswdLen)));
        m_xPasswdToOpenED->set_max_length( nMaxPasswdLen );
        m_xPasswdToOpenED->connect_changed(LINK(this, PasswordToOpenModifyDialog, ChangeHdl));
        m_xPasswdToOpenInd->set_label(aIndicatorTemplate);
        m_xReenterPasswdToOpenED->set_max_length( nMaxPasswdLen );
        m_xReenterPasswdToOpenED->connect_changed(LINK(this, PasswordToOpenModifyDialog, ChangeHdl));
        m_xReenterPasswdToOpenInd->set_label(aIndicatorTemplate);
        m_xPasswdToModifyED->set_max_length( nMaxPasswdLen );
        m_xPasswdToModifyED->connect_changed(LINK(this, PasswordToOpenModifyDialog, ChangeHdl));
        m_xPasswdToModifyInd->set_label(aIndicatorTemplate);
        m_xReenterPasswdToModifyED->set_max_length( nMaxPasswdLen );
        m_xReenterPasswdToModifyED->connect_changed(LINK(this, PasswordToOpenModifyDialog, ChangeHdl));
        m_xReenterPasswdToModifyInd->set_label(aIndicatorTemplate);
    }

    m_xPasswdToOpenED->grab_focus();

    m_xOptionsExpander->set_sensitive(bIsPasswordToModify);
    if (!bIsPasswordToModify)
        m_xOptionsExpander->hide();

    m_xOpenReadonlyCB->connect_clicked(LINK(this, PasswordToOpenModifyDialog, ReadonlyOnOffHdl));
    ReadonlyOnOffHdl(*m_xOpenReadonlyCB);
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

IMPL_LINK_NOARG(PasswordToOpenModifyDialog, ReadonlyOnOffHdl, weld::Button&, void)
{
    bool bEnable = m_xOpenReadonlyCB->get_active();
    m_xPasswdToModifyED->set_sensitive(bEnable);
    m_xPasswdToModifyFT->set_sensitive(bEnable);
    m_xReenterPasswdToModifyED->set_sensitive(bEnable);
    m_xReenterPasswdToModifyFT->set_sensitive(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
