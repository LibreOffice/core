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

#pragma once

#include <vcl/weld.hxx>
#include <o3tl/typed_flags_set.hxx>


enum class LoginFlags {
    NONE                = 0x0000,
    NoSavePassword      = 0x0008,  // hide "save password"
    NoErrorText         = 0x0010,  // hide message
    UsernameReadonly    = 0x0040,  // "name" readonly
    NoAccount           = 0x0080,  // hide "account"
    NoUseSysCreds       = 0x0100,  // hide "use system credentials"
};
namespace o3tl {
    template<> struct typed_flags<LoginFlags> : is_typed_flags<LoginFlags, 0x01d8> {};
}


class LoginDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Label> m_xErrorFT;
    std::unique_ptr<weld::Label> m_xErrorInfo;
    std::unique_ptr<weld::Label> m_xRequestInfo;
    std::unique_ptr<weld::Label> m_xNameFT;
    std::unique_ptr<weld::Entry> m_xNameED;
    std::unique_ptr<weld::Label> m_xPasswordFT;
    std::unique_ptr<weld::Entry> m_xPasswordED;
    std::unique_ptr<weld::Label> m_xAccountFT;
    std::unique_ptr<weld::Entry> m_xAccountED;
    std::unique_ptr<weld::CheckButton> m_xSavePasswdBtn;
    std::unique_ptr<weld::CheckButton> m_xUseSysCredsCB;
    std::unique_ptr<weld::Button> m_xOKBtn;
    OUString m_server;
    OUString m_realm;

    void            HideControls_Impl( LoginFlags nFlags );
    void            EnableUseSysCredsControls_Impl( bool bUseSysCredsEnabled );
    void            SetRequest();

    DECL_LINK(OKHdl_Impl, weld::Button&, void);
    DECL_LINK(UseSysCredsHdl_Impl, weld::Button&, void);

public:
    LoginDialog(weld::Window* pParent, LoginFlags nFlags,
        const OUString& rServer, const OUString &rRealm);
    virtual ~LoginDialog() override;

    OUString        GetName() const                             { return m_xNameED->get_text(); }
    void            SetName( const OUString& rNewName )           { m_xNameED->set_text( rNewName ); }
    OUString        GetPassword() const                         { return m_xPasswordED->get_text(); }
    void            SetPassword( const OUString& rNew );
    OUString        GetAccount() const                          { return m_xAccountED->get_text(); }
    bool            IsSavePassword() const                      { return m_xSavePasswdBtn->get_active(); }
    void            SetSavePassword( bool bSave )               { m_xSavePasswdBtn->set_active( bSave ); }
    void            SetSavePasswordText( const OUString& rTxt )   { m_xSavePasswdBtn->set_label( rTxt ); }
    bool            IsUseSystemCredentials() const              { return m_xUseSysCredsCB->get_active(); }
    void            SetUseSystemCredentials( bool bUse );
    void            SetErrorText( const OUString& rTxt )          { m_xErrorInfo->set_label( rTxt ); }
    void            ClearPassword();
    void            ClearAccount();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
