/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: logindlg.cxx,v $
 * $Revision: 1.8 $
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

void LoginDialog::HideControls_Impl( USHORT nFlags )
{
    FASTBOOL bPathHide = FALSE;
    FASTBOOL bErrorHide = FALSE;
    FASTBOOL bAccountHide = FALSE;
    FASTBOOL bUseSysCredsHide = FALSE;

    if ( ( nFlags & LF_NO_PATH ) == LF_NO_PATH )
    {
        aPathFT.Hide();
        aPathED.Hide();
        aPathBtn.Hide();
        bPathHide = TRUE;
    }
    else if ( ( nFlags & LF_PATH_READONLY ) == LF_PATH_READONLY )
    {
        aPathED.Hide();
        aPathInfo.Show();
        aPathBtn.Hide();
    }

    if ( ( nFlags & LF_NO_USERNAME ) == LF_NO_USERNAME )
    {
        aNameFT.Hide();
        aNameED.Hide();
    }
    else if ( ( nFlags & LF_USERNAME_READONLY ) == LF_USERNAME_READONLY )
    {
        aNameED.Hide();
        aNameInfo.Show();
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
        aErrorGB.Hide();
        bErrorHide = TRUE;
    }

    if ( ( nFlags & LF_NO_ACCOUNT ) == LF_NO_ACCOUNT )
    {
        aAccountFT.Hide();
        aAccountED.Hide();
        bAccountHide = TRUE;
    }

    if ( ( nFlags & LF_NO_USESYSCREDS ) == LF_NO_USESYSCREDS )
    {
        aUseSysCredsCB.Hide();
        bUseSysCredsHide = TRUE;
    }

    if ( bErrorHide )
    {
        long nOffset = aLoginGB.GetPosPixel().Y() -
                       aErrorGB.GetPosPixel().Y();
        Point aNewPnt = aRequestInfo.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aRequestInfo.SetPosPixel( aNewPnt );
        aNewPnt = aPathFT.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aPathFT.SetPosPixel( aNewPnt );
        aNewPnt = aPathED.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aPathED.SetPosPixel( aNewPnt );
        aNewPnt = aPathInfo.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aPathInfo.SetPosPixel( aNewPnt );
        aNewPnt = aPathBtn.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aPathBtn.SetPosPixel( aNewPnt );
        aNewPnt = aNameFT.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aNameFT.SetPosPixel( aNewPnt );
        aNewPnt = aNameED.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aNameED.SetPosPixel( aNewPnt );
        aNewPnt = aNameInfo.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aNameInfo.SetPosPixel( aNewPnt );
        aNewPnt = aPasswordFT.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aPasswordFT.SetPosPixel( aNewPnt );
        aNewPnt = aPasswordED.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aPasswordED.SetPosPixel( aNewPnt );
        aNewPnt = aAccountFT.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aAccountFT.SetPosPixel( aNewPnt );
        aNewPnt = aAccountED.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aAccountED.SetPosPixel( aNewPnt );
        aNewPnt = aSavePasswdBtn.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aSavePasswdBtn.SetPosPixel( aNewPnt );
        aNewPnt = aUseSysCredsCB.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aUseSysCredsCB.SetPosPixel( aNewPnt );
        aNewPnt = aLoginGB.GetPosPixel();
        aNewPnt.Y() -= nOffset;
        aLoginGB.SetPosPixel( aNewPnt );
        Size aNewSiz = GetSizePixel();
        aNewSiz.Height() -= nOffset;
        SetSizePixel( aNewSiz );
    }

    if ( bPathHide )
    {
        long nOffset = aNameED.GetPosPixel().Y() -
                       aPathED.GetPosPixel().Y();

        Point aTmpPnt1 = aNameFT.GetPosPixel();
        Point aTmpPnt2 = aPasswordFT.GetPosPixel();
        aNameFT.SetPosPixel( aPathFT.GetPosPixel() );
        aPasswordFT.SetPosPixel( aTmpPnt1 );
        aAccountFT.SetPosPixel( aTmpPnt2 );
        aTmpPnt1 = aNameED.GetPosPixel();
        aTmpPnt2 = aPasswordED.GetPosPixel();
        aNameED.SetPosPixel( aPathED.GetPosPixel() );
        aPasswordED.SetPosPixel( aTmpPnt1 );
        aAccountED.SetPosPixel( aTmpPnt2 );
        aNameInfo.SetPosPixel( aPathInfo.GetPosPixel() );
        aTmpPnt1 = aSavePasswdBtn.GetPosPixel();
        aTmpPnt1.Y() -= nOffset;
        aSavePasswdBtn.SetPosPixel( aTmpPnt1 );
        aTmpPnt1 = aUseSysCredsCB.GetPosPixel();
        aTmpPnt1.Y() -= nOffset;
        aUseSysCredsCB.SetPosPixel( aTmpPnt1 );
        Size aNewSz = GetSizePixel();
        aNewSz.Height() -= nOffset;
        SetSizePixel( aNewSz );
    }

    if ( bAccountHide )
    {
        long nOffset = aAccountED.GetPosPixel().Y() - aPasswordED.GetPosPixel().Y();

        Point aTmpPnt = aSavePasswdBtn.GetPosPixel();
        aTmpPnt.Y() -= nOffset;
        aSavePasswdBtn.SetPosPixel( aTmpPnt );
        aTmpPnt = aUseSysCredsCB.GetPosPixel();
        aTmpPnt.Y() -= nOffset;
        aUseSysCredsCB.SetPosPixel( aTmpPnt );
        Size aNewSz = GetSizePixel();
        aNewSz.Height() -= nOffset;
        SetSizePixel( aNewSz );
    }

    if ( bUseSysCredsHide )
    {
        long nOffset = aUseSysCredsCB.GetPosPixel().Y() -
                       aSavePasswdBtn.GetPosPixel().Y();

        Size aNewSz = GetSizePixel();
        aNewSz.Height() -= nOffset;
        SetSizePixel( aNewSz );
    }
};

// -----------------------------------------------------------------------
void LoginDialog::EnableUseSysCredsControls_Impl( BOOL bUseSysCredsEnabled )
{
    aErrorInfo.Enable( !bUseSysCredsEnabled );
    aErrorGB.Enable( !bUseSysCredsEnabled );
    aRequestInfo.Enable( !bUseSysCredsEnabled );
    aPathFT.Enable( !bUseSysCredsEnabled );
    aPathED.Enable( !bUseSysCredsEnabled );
    aPathInfo.Enable( !bUseSysCredsEnabled );
    aPathBtn.Enable( !bUseSysCredsEnabled );
    aNameFT.Enable( !bUseSysCredsEnabled );
    aNameED.Enable( !bUseSysCredsEnabled );
    aNameInfo.Enable( !bUseSysCredsEnabled );
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
    USHORT nFlags,
    const String& rServer,
    const String* pRealm,
    ResMgr* pResMgr
) :

    ModalDialog( pParent, ResId( DLG_UUI_LOGIN, *pResMgr ) ),

    aErrorInfo      ( this, ResId( INFO_LOGIN_ERROR, *pResMgr ) ),
    aErrorGB        ( this, ResId( GB_LOGIN_ERROR, *pResMgr ) ),
    aRequestInfo    ( this, ResId( INFO_LOGIN_REQUEST, *pResMgr ) ),
    aPathFT         ( this, ResId( FT_LOGIN_PATH, *pResMgr ) ),
    aPathED         ( this, ResId( ED_LOGIN_PATH, *pResMgr ) ),
    aPathInfo       ( this, ResId( INFO_LOGIN_PATH, *pResMgr ) ),
    aPathBtn        ( this, ResId( BTN_LOGIN_PATH, *pResMgr ) ),
    aNameFT         ( this, ResId( FT_LOGIN_USERNAME, *pResMgr ) ),
    aNameED         ( this, ResId( ED_LOGIN_USERNAME, *pResMgr ) ),
    aNameInfo       ( this, ResId( INFO_LOGIN_USERNAME, *pResMgr ) ),
    aPasswordFT     ( this, ResId( FT_LOGIN_PASSWORD, *pResMgr ) ),
    aPasswordED     ( this, ResId( ED_LOGIN_PASSWORD, *pResMgr ) ),
    aAccountFT      ( this, ResId( FT_LOGIN_ACCOUNT, *pResMgr ) ),
    aAccountED      ( this, ResId( ED_LOGIN_ACCOUNT, *pResMgr ) ),
    aSavePasswdBtn  ( this, ResId( CB_LOGIN_SAVEPASSWORD, *pResMgr ) ),
    aUseSysCredsCB  ( this, ResId( CB_LOGIN_USESYSCREDS, *pResMgr ) ),
    aLoginGB        ( this, ResId( GB_LOGIN_LOGIN, *pResMgr ) ),
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

void LoginDialog::SetName( const String& rNewName )
{
    aNameED.SetText( rNewName );
    aNameInfo.SetText( rNewName );
}

// -----------------------------------------------------------------------

void LoginDialog::SetUseSystemCredentials( BOOL bUse )
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
