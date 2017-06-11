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

#include "passwdomdlg.hxx"

#include "cuires.hrc"
#include "strings.hrc"
#include "dialmgr.hxx"

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>

struct PasswordToOpenModifyDialog_Impl
{
    VclPtr<PasswordToOpenModifyDialog>    m_pParent;

    VclPtr<Edit>                       m_pPasswdToOpenED;
    VclPtr<Edit>                       m_pReenterPasswdToOpenED;
    VclPtr<VclExpander>                m_pOptionsExpander;
    VclPtr<OKButton>                   m_pOk;
    VclPtr<CheckBox>                   m_pOpenReadonlyCB;
    VclPtr<Edit>                       m_pPasswdToModifyED;
    VclPtr<Edit>                       m_pReenterPasswdToModifyED;

    OUString                    m_aOneMismatch;
    OUString                    m_aTwoMismatch;
    OUString                    m_aInvalidStateForOkButton;
    OUString                    m_aInvalidStateForOkButton_v2;

    bool                        m_bIsPasswordToModify;


    DECL_LINK( OkBtnClickHdl, Button*, void );

    PasswordToOpenModifyDialog_Impl( PasswordToOpenModifyDialog * pParent,
            sal_uInt16 nMinPasswdLen, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify );
};

PasswordToOpenModifyDialog_Impl::PasswordToOpenModifyDialog_Impl(
        PasswordToOpenModifyDialog * pParent,
        sal_uInt16 nMinPasswdLen,
        sal_uInt16 nMaxPasswdLen,
        bool bIsPasswordToModify )
    : m_pParent( pParent )
    , m_aOneMismatch( CuiResId( RID_SVXSTR_ONE_PASSWORD_MISMATCH ) )
    , m_aTwoMismatch( CuiResId( RID_SVXSTR_TWO_PASSWORDS_MISMATCH ) )
    , m_aInvalidStateForOkButton( CuiResId( RID_SVXSTR_INVALID_STATE_FOR_OK_BUTTON ) )
    , m_aInvalidStateForOkButton_v2( CuiResId( RID_SVXSTR_INVALID_STATE_FOR_OK_BUTTON_V2 ) )
    , m_bIsPasswordToModify( bIsPasswordToModify )
{
    pParent->get(m_pPasswdToOpenED, "newpassEntry");
    pParent->get(m_pReenterPasswdToOpenED, "confirmpassEntry");
    pParent->get(m_pOk, "ok");
    pParent->get(m_pOpenReadonlyCB, "readonly");
    pParent->get(m_pPasswdToModifyED, "newpassroEntry");
    pParent->get(m_pReenterPasswdToModifyED, "confirmropassEntry");
    pParent->get(m_pOptionsExpander, "expander");

    m_pOk->SetClickHdl( LINK( this, PasswordToOpenModifyDialog_Impl, OkBtnClickHdl ) );

    if (nMaxPasswdLen)
    {
        m_pPasswdToOpenED->SetMaxTextLen( nMaxPasswdLen );
        m_pReenterPasswdToOpenED->SetMaxTextLen( nMaxPasswdLen );
        m_pPasswdToModifyED->SetMaxTextLen( nMaxPasswdLen );
        m_pReenterPasswdToModifyED->SetMaxTextLen( nMaxPasswdLen );
    }

    (void) nMinPasswdLen;   // currently not supported

    m_pPasswdToOpenED->GrabFocus();

    m_pOptionsExpander->Enable(bIsPasswordToModify);
    if (!bIsPasswordToModify)
        m_pOptionsExpander->Hide();
}

IMPL_LINK_NOARG( PasswordToOpenModifyDialog_Impl, OkBtnClickHdl, Button *, void )
{
    bool bInvalidState = !m_pOpenReadonlyCB->IsChecked() &&
            m_pPasswdToOpenED->GetText().isEmpty() &&
            m_pPasswdToModifyED->GetText().isEmpty();
    if (bInvalidState)
    {
        ScopedVclPtrInstance<MessageDialog> aErrorBox(m_pParent,
            m_bIsPasswordToModify? m_aInvalidStateForOkButton : m_aInvalidStateForOkButton_v2);
        aErrorBox->Execute();
    }
    else // check for mismatched passwords...
    {
        const bool bToOpenMatch     = m_pPasswdToOpenED->GetText()   == m_pReenterPasswdToOpenED->GetText();
        const bool bToModifyMatch   = m_pPasswdToModifyED->GetText() == m_pReenterPasswdToModifyED->GetText();
        const int nMismatch = (bToOpenMatch? 0 : 1) + (bToModifyMatch? 0 : 1);
        if (nMismatch > 0)
        {
            ScopedVclPtrInstance< MessageDialog > aErrorBox(m_pParent, nMismatch == 1 ? m_aOneMismatch : m_aTwoMismatch);
            aErrorBox->Execute();

            Edit* pEdit = !bToOpenMatch ? m_pPasswdToOpenED.get() : m_pPasswdToModifyED.get();
            Edit* pRepeatEdit = !bToOpenMatch? m_pReenterPasswdToOpenED.get() : m_pReenterPasswdToModifyED.get();
            if (nMismatch == 1)
            {
                pEdit->SetText( "" );
                pRepeatEdit->SetText( "" );
            }
            else if (nMismatch == 2)
            {
                m_pPasswdToOpenED->SetText( "" );
                m_pReenterPasswdToOpenED->SetText( "" );
                m_pPasswdToModifyED->SetText( "" );
                m_pReenterPasswdToModifyED->SetText( "" );
            }
            pEdit->GrabFocus();
        }
        else
        {
            m_pParent->EndDialog( RET_OK );
        }
    }
}

PasswordToOpenModifyDialog::PasswordToOpenModifyDialog(
    vcl::Window * pParent, sal_uInt16 nMinPasswdLen,
    sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify)
    : SfxModalDialog( pParent, "PasswordDialog", "cui/ui/password.ui" )
{
    m_pImpl.reset(new PasswordToOpenModifyDialog_Impl(this,
        nMinPasswdLen, nMaxPasswdLen, bIsPasswordToModify ) );
}


PasswordToOpenModifyDialog::~PasswordToOpenModifyDialog()
{
    disposeOnce();
}

OUString PasswordToOpenModifyDialog::GetPasswordToOpen() const
{
    const bool bPasswdOk =
            !m_pImpl->m_pPasswdToOpenED->GetText().isEmpty() &&
            m_pImpl->m_pPasswdToOpenED->GetText() == m_pImpl->m_pReenterPasswdToOpenED->GetText();
    return bPasswdOk ? m_pImpl->m_pPasswdToOpenED->GetText() : OUString();
}


OUString PasswordToOpenModifyDialog::GetPasswordToModify() const
{
    const bool bPasswdOk =
            !m_pImpl->m_pPasswdToModifyED->GetText().isEmpty() &&
            m_pImpl->m_pPasswdToModifyED->GetText() == m_pImpl->m_pReenterPasswdToModifyED->GetText();
    return bPasswdOk ? m_pImpl->m_pPasswdToModifyED->GetText() : OUString();
}


bool PasswordToOpenModifyDialog::IsRecommendToOpenReadonly() const
{
    return m_pImpl->m_pOpenReadonlyCB->IsChecked();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
