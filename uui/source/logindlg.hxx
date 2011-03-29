/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef UUI_LOGINDLG_HXX
#define UUI_LOGINDLG_HXX

#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

//============================================================================
#define LF_NO_PATH              0x0001  // hide "path"
#define LF_NO_USERNAME          0x0002  // hide "name"
#define LF_NO_PASSWORD          0x0004  // hide "password"
#define LF_NO_SAVEPASSWORD      0x0008  // hide "save password"
#define LF_NO_ERRORTEXT         0x0010  // hide message
#define LF_PATH_READONLY        0x0020  // "path" readonly
#define LF_USERNAME_READONLY    0x0040  // "name" readonly
#define LF_NO_ACCOUNT           0x0080  // hide "account"
#define LF_NO_USESYSCREDS       0x0100  // hide "use system cedentials"

//............................................................................
//............................................................................

//============================================================================
class LoginDialog : public ModalDialog
{
    FixedText       aErrorFT;
    FixedInfo       aErrorInfo;
    FixedLine       aLogin1FL;
    FixedInfo       aRequestInfo;
    FixedLine       aLogin2FL;
    FixedText       aPathFT;
    Edit            aPathED;
    PushButton      aPathBtn;
    FixedText       aNameFT;
    Edit            aNameED;
    FixedText       aPasswordFT;
    Edit            aPasswordED;
    FixedText       aAccountFT;
    Edit            aAccountED;
    CheckBox        aSavePasswdBtn;
    CheckBox        aUseSysCredsCB;
    FixedLine       aButtonsFL;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;

    void            HideControls_Impl( sal_uInt16 nFlags );
    void            EnableUseSysCredsControls_Impl( sal_Bool bUseSysCredsEnabled );

    DECL_LINK( OKHdl_Impl, OKButton * );
    DECL_LINK( PathHdl_Impl, PushButton * );
    DECL_LINK( UseSysCredsHdl_Impl, CheckBox * );

public:
    LoginDialog( Window* pParent, sal_uInt16 nFlags,
                 const String& rServer, const String* pRealm,
                 ResMgr * pResMgr );
    virtual ~LoginDialog();

    String          GetPath() const                             { return aPathED.GetText(); }
    void            SetPath( const String& rNewPath )           { aPathED.SetText( rNewPath ); }
    String          GetName() const                             { return aNameED.GetText(); }
    void            SetName( const String& rNewName )           { aNameED.SetText( rNewName ); }
    String          GetPassword() const                         { return aPasswordED.GetText(); }
    void            SetPassword( const String& rNew )           { aPasswordED.SetText( rNew ); }
    String          GetAccount() const                          { return aAccountED.GetText(); }
    void            SetAccount( const String& rNew )            { aAccountED.SetText( rNew ); }
    sal_Bool            IsSavePassword() const                      { return aSavePasswdBtn.IsChecked(); }
    void            SetSavePassword( sal_Bool bSave )               { aSavePasswdBtn.Check( bSave ); }
    void            SetSavePasswordText( const String& rTxt )   { aSavePasswdBtn.SetText( rTxt ); }
    sal_Bool            IsUseSystemCredentials() const              { return aUseSysCredsCB.IsChecked(); }
    void            SetUseSystemCredentials( sal_Bool bUse );
    void            SetErrorText( const String& rTxt )          { aErrorInfo.SetText( rTxt ); }
    void            SetLoginRequestText( const String& rTxt )   { aRequestInfo.SetText( rTxt ); }
    void            ClearPassword();
    void            ClearAccount();
};

// -----------------------------------------------------------------------


//............................................................................
//............................................................................

#endif // UUI_LOGINDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
