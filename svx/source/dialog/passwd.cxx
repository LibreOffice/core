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
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include "svx/passwd.hxx"
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

// class SvxPasswordDialog -----------------------------------------------

IMPL_LINK_NOARG(SvxPasswordDialog, ButtonHdl)
{
    bool bOK = true;
    short nRet = RET_OK;
    OUString aEmpty;

    if ( m_pNewPasswdED->GetText() != m_pRepeatPasswdED->GetText() )
    {
        ErrorBox( this, WB_OK, aRepeatPasswdErrStr ).Execute();
        m_pNewPasswdED->SetText( aEmpty );
        m_pRepeatPasswdED->SetText( aEmpty );
        m_pNewPasswdED->GrabFocus();
        bOK = false;
    }

    if ( bOK && aCheckPasswordHdl.IsSet() && !aCheckPasswordHdl.Call( this ) )
    {
        ErrorBox( this, WB_OK, aOldPasswdErrStr ).Execute();
        m_pOldPasswdED->SetText( aEmpty );
        m_pOldPasswdED->GrabFocus();
        bOK = false;
    }

    if ( bOK )
        EndDialog( nRet );

    return 0;
}



IMPL_LINK_NOARG(SvxPasswordDialog, EditModifyHdl)
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
    return 0;
}



SvxPasswordDialog::SvxPasswordDialog(Window* pParent, sal_Bool bAllowEmptyPasswords, sal_Bool bDisableOldPassword)
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
    EditModifyHdl( 0 );

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
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
