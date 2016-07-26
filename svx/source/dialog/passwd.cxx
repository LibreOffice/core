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

#include <comphelper/string.hxx>
#include <vcl/layout.hxx>
#include "svx/passwd.hxx"
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

IMPL_LINK_NOARG_TYPED(SvxPasswordDialog, ButtonHdl, Button*, void)
{
    bool bOK = true;
    short nRet = RET_OK;

    if ( m_pNewPasswdED->GetText() != m_pRepeatPasswdED->GetText() )
    {
        ScopedVclPtrInstance<MessageDialog>(this, aRepeatPasswdErrStr)->Execute();
        m_pNewPasswdED->SetText( "" );
        m_pRepeatPasswdED->SetText( "" );
        m_pNewPasswdED->GrabFocus();
        bOK = false;
    }

    if ( bOK && aCheckPasswordHdl.IsSet() && !aCheckPasswordHdl.Call( this ) )
    {
        ScopedVclPtrInstance<MessageDialog>(this, aOldPasswdErrStr)->Execute();
        m_pOldPasswdED->SetText( "" );
        m_pOldPasswdED->GrabFocus();
        bOK = false;
    }

    if ( bOK )
        EndDialog( nRet );
}


IMPL_LINK_NOARG_TYPED(SvxPasswordDialog, EditModifyHdl, Edit&, void)
{
    if ( !bEmpty )
    {
        OUString aPasswd = comphelper::string::strip(m_pRepeatPasswdED->GetText(), ' ');
        if ( aPasswd.isEmpty() && m_pOKBtn->IsEnabled() )
            m_pOKBtn->Disable();
        else if ( !aPasswd.isEmpty() && !m_pOKBtn->IsEnabled() )
            m_pOKBtn->Enable();
    }
    else if ( !m_pOKBtn->IsEnabled() )
        m_pOKBtn->Enable();
}


SvxPasswordDialog::SvxPasswordDialog(vcl::Window* pParent, bool bAllowEmptyPasswords, bool bDisableOldPassword)
    : SfxModalDialog(pParent, "PasswordDialog", "svx/ui/passwd.ui")
    , aOldPasswdErrStr(SVX_RESSTR(RID_SVXSTR_ERR_OLD_PASSWD))
    , aRepeatPasswdErrStr(SVX_RESSTR(RID_SVXSTR_ERR_REPEAT_PASSWD ))
    , bEmpty(bAllowEmptyPasswords)
{
    get(m_pOldFL, "oldpass");
    get(m_pOldPasswdFT, "oldpassL");
    get(m_pOldPasswdED, "oldpassEntry");
    get(m_pNewPasswdED, "newpassEntry");
    get(m_pRepeatPasswdED, "confirmpassEntry");
    get(m_pOKBtn, "ok");

    m_pOKBtn->SetClickHdl( LINK( this, SvxPasswordDialog, ButtonHdl ) );
    m_pRepeatPasswdED->SetModifyHdl( LINK( this, SvxPasswordDialog, EditModifyHdl ) );
    EditModifyHdl( *m_pRepeatPasswdED );

    if ( bDisableOldPassword )
    {
        m_pOldFL->Disable();
         m_pOldPasswdFT->Disable();
        m_pOldPasswdED->Disable();
        m_pNewPasswdED->GrabFocus();
    }
}

SvxPasswordDialog::~SvxPasswordDialog()
{
    disposeOnce();
}

void SvxPasswordDialog::dispose()
{
    m_pOldFL.clear();
    m_pOldPasswdFT.clear();
    m_pOldPasswdED.clear();
    m_pNewPasswdED.clear();
    m_pRepeatPasswdED.clear();
    m_pOKBtn.clear();
    SfxModalDialog::dispose();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
