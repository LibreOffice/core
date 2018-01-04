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

#ifndef INCLUDED_UUI_SOURCE_LOGINDLG_HXX
#define INCLUDED_UUI_SOURCE_LOGINDLG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <o3tl/typed_flags_set.hxx>


enum class LoginFlags {
    NONE                = 0x0000,
    NoUsername          = 0x0002,  // hide "name"
    NoPassword          = 0x0004,  // hide "password"
    NoSavePassword      = 0x0008,  // hide "save password"
    NoErrorText         = 0x0010,  // hide message
    PathReadonly        = 0x0020,  // "path" readonly
    UsernameReadonly    = 0x0040,  // "name" readonly
    NoAccount           = 0x0080,  // hide "account"
    NoUseSysCreds       = 0x0100,  // hide "use system credentials"
};
namespace o3tl {
    template<> struct typed_flags<LoginFlags> : is_typed_flags<LoginFlags, 0x01fe> {};
}


class LoginDialog : public ModalDialog
{
    VclPtr<FixedText>      m_pErrorFT;
    VclPtr<FixedText>      m_pErrorInfo;
    VclPtr<FixedText>      m_pRequestInfo;
    VclPtr<FixedText>      m_pNameFT;
    VclPtr<Edit>           m_pNameED;
    VclPtr<FixedText>      m_pPasswordFT;
    VclPtr<Edit>           m_pPasswordED;
    VclPtr<FixedText>      m_pAccountFT;
    VclPtr<Edit>           m_pAccountED;
    VclPtr<CheckBox>       m_pSavePasswdBtn;
    VclPtr<CheckBox>       m_pUseSysCredsCB;
    VclPtr<OKButton>       m_pOKBtn;
    OUString m_server;
    OUString m_realm;

    void            HideControls_Impl( LoginFlags nFlags );
    void            EnableUseSysCredsControls_Impl( bool bUseSysCredsEnabled );
    void            SetRequest();

    DECL_LINK(OKHdl_Impl, Button*, void);
    DECL_LINK(UseSysCredsHdl_Impl, Button*, void);

public:
    LoginDialog(vcl::Window* pParent, LoginFlags nFlags,
        const OUString& rServer, const OUString &rRealm);
    virtual ~LoginDialog() override;
    virtual void    dispose() override;

    OUString        GetName() const                             { return m_pNameED->GetText(); }
    void            SetName( const OUString& rNewName )           { m_pNameED->SetText( rNewName ); }
    OUString        GetPassword() const                         { return m_pPasswordED->GetText(); }
    void            SetPassword( const OUString& rNew );
    OUString        GetAccount() const                          { return m_pAccountED->GetText(); }
    bool            IsSavePassword() const                      { return m_pSavePasswdBtn->IsChecked(); }
    void            SetSavePassword( bool bSave )               { m_pSavePasswdBtn->Check( bSave ); }
    void            SetSavePasswordText( const OUString& rTxt )   { m_pSavePasswdBtn->SetText( rTxt ); }
    bool            IsUseSystemCredentials() const              { return m_pUseSysCredsCB->IsChecked(); }
    void            SetUseSystemCredentials( bool bUse );
    void            SetErrorText( const OUString& rTxt )          { m_pErrorInfo->SetText( rTxt ); }
    void            ClearPassword();
    void            ClearAccount();
};


#endif // INCLUDED_UUI_SOURCE_LOGINDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
