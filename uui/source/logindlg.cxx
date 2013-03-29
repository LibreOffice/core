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
#include <vcl/msgbox.hxx>
#include "logindlg.hxx"

#include "logindlg.hrc"
#include "ids.hrc"
#include <tools/resid.hxx>
#include <osl/file.hxx>

#ifdef UNX
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <comphelper/processfactory.hxx>

using namespace com::sun::star;

static void lcl_Move( Window &rWin, long nOffset )
{
    Point aTmp( rWin.GetPosPixel() );
    aTmp.Y() -= nOffset;
    rWin.SetPosPixel( aTmp );
}


void LoginDialog::HideControls_Impl( sal_uInt16 nFlags )
{
    bool bPathHide = sal_False;
    bool bErrorHide = sal_False;
    bool bAccountHide = sal_False;
    bool bUseSysCredsHide = sal_False;

    if ( ( nFlags & LF_NO_PATH ) == LF_NO_PATH )
    {
        aPathFT.Hide();
        aPathED.Hide();
        aPathBtn.Hide();
        bPathHide = sal_True;
    }
    else if ( ( nFlags & LF_PATH_READONLY ) == LF_PATH_READONLY )
    {
        aPathED.Enable( sal_False );
        aPathBtn.Enable( sal_False );
    }

    if ( ( nFlags & LF_NO_USERNAME ) == LF_NO_USERNAME )
    {
        aNameFT.Hide();
        aNameED.Hide();
    }
    else if ( ( nFlags & LF_USERNAME_READONLY ) == LF_USERNAME_READONLY )
    {
        aNameED.Enable( sal_False );
    }

    if ( ( nFlags & LF_NO_PASSWORD ) == LF_NO_PASSWORD )
    {
        aPasswordFT.Hide();
        aPasswordED.Hide();
    }

    if ( ( nFlags & LF_NO_SAVEPASSWORD ) == LF_NO_SAVEPASSWORD )
        aSavePasswdBtn.Hide();

    if ( ( nFlags & LF_NO_ERRORTEXT ) == LF_NO_ERRORTEXT )
    {
        aErrorInfo.Hide();
        aErrorFT.Hide();
        aLogin1FL.Hide();
        bErrorHide = sal_True;
    }

    if ( ( nFlags & LF_NO_ACCOUNT ) == LF_NO_ACCOUNT )
    {
        aAccountFT.Hide();
        aAccountED.Hide();
        bAccountHide = sal_True;
    }

    if ( ( nFlags & LF_NO_USESYSCREDS ) == LF_NO_USESYSCREDS )
    {
        aUseSysCredsCB.Hide();
        bUseSysCredsHide = sal_True;
    }

    if ( bErrorHide )
    {
        long nOffset = aRequestInfo.GetPosPixel().Y() -
                       aErrorFT.GetPosPixel().Y();
        lcl_Move( aRequestInfo, nOffset );
        lcl_Move( aLogin2FL, nOffset );
        lcl_Move( aPathFT, nOffset );
        lcl_Move( aPathED, nOffset );
        lcl_Move( aPathBtn, nOffset );
        lcl_Move( aNameFT, nOffset );
        lcl_Move( aNameED, nOffset );
        lcl_Move( aPasswordFT, nOffset );
        lcl_Move( aPasswordED, nOffset );
        lcl_Move( aAccountFT, nOffset );
        lcl_Move( aAccountED, nOffset );
        lcl_Move( aSavePasswdBtn, nOffset );
        lcl_Move( aUseSysCredsCB, nOffset );
        lcl_Move( aButtonsFL, nOffset );
        lcl_Move( aOKBtn, nOffset );
        lcl_Move( aCancelBtn, nOffset );
        lcl_Move( aHelpBtn, nOffset );

        Size aNewSiz = GetSizePixel();
        aNewSiz.Height() -= nOffset;
        SetSizePixel( aNewSiz );
    }

    if ( bPathHide )
    {
        long nOffset = aNameED.GetPosPixel().Y() -
                       aPathED.GetPosPixel().Y();
        lcl_Move( aNameFT, nOffset );
        lcl_Move( aNameED, nOffset );
        lcl_Move( aPasswordFT, nOffset );
        lcl_Move( aPasswordED, nOffset );
        lcl_Move( aAccountFT, nOffset );
        lcl_Move( aAccountED, nOffset );
        lcl_Move( aSavePasswdBtn, nOffset );
        lcl_Move( aUseSysCredsCB, nOffset );
        lcl_Move( aButtonsFL, nOffset );
        lcl_Move( aOKBtn, nOffset );
        lcl_Move( aCancelBtn, nOffset );
        lcl_Move( aHelpBtn, nOffset );

        Size aNewSz = GetSizePixel();
        aNewSz.Height() -= nOffset;
        SetSizePixel( aNewSz );
    }

    if ( bAccountHide )
    {
        long nOffset = aAccountED.GetPosPixel().Y() -
                       aPasswordED.GetPosPixel().Y();
        lcl_Move( aSavePasswdBtn, nOffset );
        lcl_Move( aUseSysCredsCB, nOffset );
        lcl_Move( aButtonsFL, nOffset );
        lcl_Move( aOKBtn, nOffset );
        lcl_Move( aCancelBtn, nOffset );
        lcl_Move( aHelpBtn, nOffset );

        Size aNewSz = GetSizePixel();
        aNewSz.Height() -= nOffset;
        SetSizePixel( aNewSz );
    }

    if ( bUseSysCredsHide )
    {
        long nOffset = aUseSysCredsCB.GetPosPixel().Y() -
                       aSavePasswdBtn.GetPosPixel().Y();
        lcl_Move( aButtonsFL, nOffset );
        lcl_Move( aOKBtn, nOffset );
        lcl_Move( aCancelBtn, nOffset );
        lcl_Move( aHelpBtn, nOffset );

        Size aNewSz = GetSizePixel();
        aNewSz.Height() -= nOffset;
        SetSizePixel( aNewSz );
    }
};

void LoginDialog::EnableUseSysCredsControls_Impl( sal_Bool bUseSysCredsEnabled )
{
    aErrorInfo.Enable( !bUseSysCredsEnabled );
    aErrorFT.Enable( !bUseSysCredsEnabled );
    aRequestInfo.Enable( !bUseSysCredsEnabled );
    aPathFT.Enable( !bUseSysCredsEnabled );
    aPathED.Enable( !bUseSysCredsEnabled );
    aPathBtn.Enable( !bUseSysCredsEnabled );
    aNameFT.Enable( !bUseSysCredsEnabled );
    aNameED.Enable( !bUseSysCredsEnabled );
    aPasswordFT.Enable( !bUseSysCredsEnabled );
    aPasswordED.Enable( !bUseSysCredsEnabled );
    aAccountFT.Enable( !bUseSysCredsEnabled );
    aAccountED.Enable( !bUseSysCredsEnabled );
}

IMPL_LINK_NOARG(LoginDialog, OKHdl_Impl)
{
    // trim the strings
    aNameED.SetText(comphelper::string::strip(aNameED.GetText(), ' '));
    aPasswordED.SetText(comphelper::string::strip(aPasswordED.GetText(), ' '));
    EndDialog( RET_OK );
    return 1;
}

IMPL_LINK_NOARG(LoginDialog, PathHdl_Impl)
{
    try
    {
        uno::Reference<ui::dialogs::XFolderPicker2> xFolderPicker = ui::dialogs::FolderPicker::create(comphelper::getProcessComponentContext());

        OUString aPath( aPathED.GetText() );
        osl::FileBase::getFileURLFromSystemPath( aPath, aPath );
        xFolderPicker->setDisplayDirectory( aPath );

        if (xFolderPicker->execute() == ui::dialogs::ExecutableDialogResults::OK)
        {
            osl::FileBase::getSystemPathFromFileURL( xFolderPicker->getDirectory(), aPath );
            aPathED.SetText( aPath );
        }
    }
    catch (uno::Exception & e)
    {
        SAL_WARN("uui", "LoginDialog::PathHdl_Impl: caught UNO exception: " << e.Message);
    }

    return 1;
}

IMPL_LINK_NOARG(LoginDialog, UseSysCredsHdl_Impl)
{
    EnableUseSysCredsControls_Impl( aUseSysCredsCB.IsChecked() );
    return 1;
}

LoginDialog::LoginDialog
(
    Window* pParent,
    sal_uInt16 nFlags,
    const rtl::OUString& rServer,
    const rtl::OUString& rRealm,
    ResMgr* pResMgr
) :

    ModalDialog( pParent, ResId( DLG_UUI_LOGIN, *pResMgr ) ),

    aErrorFT        ( this, ResId( FT_LOGIN_ERROR, *pResMgr ) ),
    aErrorInfo      ( this, ResId( FT_INFO_LOGIN_ERROR, *pResMgr ) ),
    aLogin1FL       ( this, ResId( FL_LOGIN_1, *pResMgr ) ),
    aRequestInfo    ( this, ResId( FT_INFO_LOGIN_REQUEST, *pResMgr ) ),
    aLogin2FL       ( this, ResId( FL_LOGIN_2, *pResMgr ) ),
    aPathFT         ( this, ResId( FT_LOGIN_PATH, *pResMgr ) ),
    aPathED         ( this, ResId( ED_LOGIN_PATH, *pResMgr ) ),
    aPathBtn        ( this, ResId( BTN_LOGIN_PATH, *pResMgr ) ),
    aNameFT         ( this, ResId( FT_LOGIN_USERNAME, *pResMgr ) ),
    aNameED         ( this, ResId( ED_LOGIN_USERNAME, *pResMgr ) ),
    aPasswordFT     ( this, ResId( FT_LOGIN_PASSWORD, *pResMgr ) ),
    aPasswordED     ( this, ResId( ED_LOGIN_PASSWORD, *pResMgr ) ),
    aAccountFT      ( this, ResId( FT_LOGIN_ACCOUNT, *pResMgr ) ),
    aAccountED      ( this, ResId( ED_LOGIN_ACCOUNT, *pResMgr ) ),
    aSavePasswdBtn  ( this, ResId( CB_LOGIN_SAVEPASSWORD, *pResMgr ) ),
    aUseSysCredsCB  ( this, ResId( CB_LOGIN_USESYSCREDS, *pResMgr ) ),
    aButtonsFL      ( this, ResId( FL_BUTTONS, *pResMgr ) ),
    aOKBtn          ( this, ResId( BTN_LOGIN_OK, *pResMgr ) ),
    aCancelBtn      ( this, ResId( BTN_LOGIN_CANCEL, *pResMgr ) ),
    aHelpBtn        ( this, ResId( BTN_LOGIN_HELP, *pResMgr ) )

{
    OUString aRequest;
    if ((nFlags & LF_NO_ACCOUNT) != 0 && !rRealm.isEmpty())
    {
        aRequest = OUString(ResId(STR_LOGIN_REALM, *pResMgr));
        aRequest = aRequest.replaceAll("%2", rRealm);
    }
    else
        aRequest = aRequestInfo.GetText();

    if ( !( ( nFlags & LF_NO_USESYSCREDS ) == LF_NO_USESYSCREDS ) )
      EnableUseSysCredsControls_Impl( aUseSysCredsCB.IsChecked() );

    aRequest = aRequest.replaceAll("%1", rServer);
    aRequestInfo.SetText(aRequest);

    FreeResource();

    aPathED.SetMaxTextLen( _MAX_PATH );
    aNameED.SetMaxTextLen( _MAX_PATH );

    aOKBtn.SetClickHdl( LINK( this, LoginDialog, OKHdl_Impl ) );
    aPathBtn.SetClickHdl( LINK( this, LoginDialog, PathHdl_Impl ) );
    aUseSysCredsCB.SetClickHdl( LINK( this, LoginDialog, UseSysCredsHdl_Impl ) );

    HideControls_Impl( nFlags );
};

LoginDialog::~LoginDialog()
{
}

void LoginDialog::SetUseSystemCredentials( sal_Bool bUse )
{
    if ( aUseSysCredsCB.IsVisible() )
    {
        aUseSysCredsCB.Check( bUse );
        EnableUseSysCredsControls_Impl( bUse );
    }
}

void LoginDialog::ClearPassword()
{
    aPasswordED.SetText( String() );

    if ( aNameED.GetText().isEmpty() )
        aNameED.GrabFocus();
    else
        aPasswordED.GrabFocus();
};

void LoginDialog::ClearAccount()
{
    aAccountED.SetText( String() );
    aAccountED.GrabFocus();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
