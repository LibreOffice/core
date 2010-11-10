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

#include <svtools/filedlg.hxx>
#include <vcl/msgbox.hxx>
#include "logindlg.hxx"

#ifndef UUI_LOGINDLG_HRC
#include "logindlg.hrc"
#endif
#ifndef UUI_IDS_HRC
#include "ids.hrc"
#endif
#include <tools/resid.hxx>

#ifdef UNX
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif

// LoginDialog -------------------------------------------------------

//............................................................................
//............................................................................

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

// -----------------------------------------------------------------------
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

// -----------------------------------------------------------------------

IMPL_LINK( LoginDialog, OKHdl_Impl, OKButton *, EMPTYARG )
{
    // trim the strings
    aNameED.SetText( aNameED.GetText().EraseLeadingChars().
        EraseTrailingChars() );
    aPasswordED.SetText( aPasswordED.GetText().EraseLeadingChars().
        EraseTrailingChars() );
    EndDialog( RET_OK );
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( LoginDialog, PathHdl_Impl, PushButton *, EMPTYARG )
{
    PathDialog* pDlg = new PathDialog( this, WB_3DLOOK );
    pDlg->SetPath( aPathED.GetText() );

    if ( pDlg->Execute() == RET_OK )
        aPathED.SetText( pDlg->GetPath() );

    delete pDlg;
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( LoginDialog, UseSysCredsHdl_Impl, CheckBox *, EMPTYARG )
{
    EnableUseSysCredsControls_Impl( aUseSysCredsCB.IsChecked() );
    return 1;
}

// -----------------------------------------------------------------------

LoginDialog::LoginDialog
(
    Window* pParent,
    sal_uInt16 nFlags,
    const String& rServer,
    const String* pRealm,
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
    UniString aRequest;
    if ((nFlags & LF_NO_ACCOUNT) != 0 && pRealm && pRealm->Len() != 0)
    {
        aRequest = ResId(STR_LOGIN_REALM, *pResMgr);
        aRequest.SearchAndReplaceAscii("%2", *pRealm);
    }
    else
        aRequest = aRequestInfo.GetText();

    if ( !( ( nFlags & LF_NO_USESYSCREDS ) == LF_NO_USESYSCREDS ) )
      EnableUseSysCredsControls_Impl( aUseSysCredsCB.IsChecked() );

    aRequest.SearchAndReplaceAscii("%1", rServer);
    aRequestInfo.SetText(aRequest);

    FreeResource();

    aPathED.SetMaxTextLen( _MAX_PATH );
    aNameED.SetMaxTextLen( _MAX_PATH );

    aOKBtn.SetClickHdl( LINK( this, LoginDialog, OKHdl_Impl ) );
    aPathBtn.SetClickHdl( LINK( this, LoginDialog, PathHdl_Impl ) );
    aUseSysCredsCB.SetClickHdl( LINK( this, LoginDialog, UseSysCredsHdl_Impl ) );

    HideControls_Impl( nFlags );
};

// -----------------------------------------------------------------------

LoginDialog::~LoginDialog()
{
}

// -----------------------------------------------------------------------

void LoginDialog::SetUseSystemCredentials( sal_Bool bUse )
{
    if ( aUseSysCredsCB.IsVisible() )
    {
        aUseSysCredsCB.Check( bUse );
        EnableUseSysCredsControls_Impl( bUse );
    }
}

// -----------------------------------------------------------------------

void LoginDialog::ClearPassword()
{
    aPasswordED.SetText( String() );

    if ( 0 == aNameED.GetText().Len() )
        aNameED.GrabFocus();
    else
        aPasswordED.GrabFocus();
};

// -----------------------------------------------------------------------

void LoginDialog::ClearAccount()
{
    aAccountED.SetText( String() );
    aAccountED.GrabFocus();
};

//............................................................................
//............................................................................
