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

#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>


#define LF_NO_PATH              0x0001  // hide "path"
#define LF_NO_USERNAME          0x0002  // hide "name"
#define LF_NO_PASSWORD          0x0004  // hide "password"
#define LF_NO_SAVEPASSWORD      0x0008  // hide "save password"
#define LF_NO_ERRORTEXT         0x0010  // hide message
#define LF_PATH_READONLY        0x0020  // "path" readonly
#define LF_USERNAME_READONLY    0x0040  // "name" readonly
#define LF_NO_ACCOUNT           0x0080  // hide "account"
#define LF_NO_USESYSCREDS       0x0100  // hide "use system cedentials"





class LoginDialog : public ModalDialog
{
    FixedText*      m_pErrorFT;
    FixedText*      m_pErrorInfo;
    FixedText*      m_pRequestInfo;
    FixedText*      m_pPathFT;
    Edit*           m_pPathED;
    PushButton*     m_pPathBtn;
    FixedText*      m_pNameFT;
    Edit*           m_pNameED;
    FixedText*      m_pPasswordFT;
    Edit*           m_pPasswordED;
    FixedText*      m_pAccountFT;
    Edit*           m_pAccountED;
    CheckBox*       m_pSavePasswdBtn;
    CheckBox*       m_pUseSysCredsCB;
    OKButton*       m_pOKBtn;

    void            HideControls_Impl( sal_uInt16 nFlags );
    void            EnableUseSysCredsControls_Impl( bool bUseSysCredsEnabled );

    DECL_LINK(OKHdl_Impl, void *);
    DECL_LINK(PathHdl_Impl, void *);
    DECL_LINK(UseSysCredsHdl_Impl, void *);

public:
    LoginDialog(vcl::Window* pParent, sal_uInt16 nFlags,
        const OUString& rServer, const OUString &rRealm);

    OUString        GetPath() const                             { return m_pPathED->GetText(); }
    void            SetPath( const OUString& rNewPath )           { m_pPathED->SetText( rNewPath ); }
    OUString        GetName() const                             { return m_pNameED->GetText(); }
    void            SetName( const OUString& rNewName )           { m_pNameED->SetText( rNewName ); }
    OUString        GetPassword() const                         { return m_pPasswordED->GetText(); }
    void            SetPassword( const OUString& rNew )           { m_pPasswordED->SetText( rNew ); }
    OUString        GetAccount() const                          { return m_pAccountED->GetText(); }
    void            SetAccount( const OUString& rNew )            { m_pAccountED->SetText( rNew ); }
    bool            IsSavePassword() const                      { return m_pSavePasswdBtn->IsChecked(); }
    void            SetSavePassword( bool bSave )               { m_pSavePasswdBtn->Check( bSave ); }
    void            SetSavePasswordText( const OUString& rTxt )   { m_pSavePasswdBtn->SetText( rTxt ); }
    bool        IsUseSystemCredentials() const              { return m_pUseSysCredsCB->IsChecked(); }
    void            SetUseSystemCredentials( bool bUse );
    void            SetErrorText( const OUString& rTxt )          { m_pErrorInfo->SetText( rTxt ); }
    void            SetLoginRequestText( const OUString& rTxt )   { m_pRequestInfo->SetText( rTxt ); }
    void            ClearPassword();
    void            ClearAccount();
};







#endif // INCLUDED_UUI_SOURCE_LOGINDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
