/*************************************************************************
 *
 *  $RCSfile: logindlg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sb $ $Date: 2000-11-10 10:57:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVT_FILEDLG_HXX
#include <svtools/filedlg.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef UUI_IDS_HRC
#include <ids.hrc>
#endif
#ifndef UUI_LOGINDLG_HRC
#include <logindlg.hrc>
#endif
#ifndef UUI_LOGINDLG_HXX
#include <logindlg.hxx>
#endif

#ifdef UNX
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif

// LoginDialog -------------------------------------------------------

void LoginDialog::HideControls_Impl( USHORT nFlags )
{
    FASTBOOL bPathHide = FALSE;
    FASTBOOL bErrorHide = FALSE;
    FASTBOOL bAccountHide = FALSE;

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
        Size aNewSz = aLoginGB.GetSizePixel();
        aNewSz.Height() -= nOffset;
        aLoginGB.SetSizePixel( aNewSz );
        aNewSz = GetSizePixel();
        aNewSz.Height() -= nOffset;
        SetSizePixel( aNewSz );
    }

    if ( bAccountHide )
    {
        long nOffset = aAccountED.GetPosPixel().Y() - aPasswordED.GetPosPixel().Y();

        Point aTmpPnt = aSavePasswdBtn.GetPosPixel();
        aTmpPnt.Y() -= nOffset;
        aSavePasswdBtn.SetPosPixel( aTmpPnt );
        Size aNewSz = aLoginGB.GetSizePixel();
        aNewSz.Height() -= nOffset;
        aLoginGB.SetSizePixel( aNewSz );
        aNewSz = GetSizePixel();
        aNewSz.Height() -= nOffset;
        SetSizePixel( aNewSz );
    }
};

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

LoginDialog::LoginDialog
(
    Window* pParent,
    USHORT nFlags,
    const String& rServer,
    const String& rRealm,
    ResMgr* pResMgr
) :

    ModalDialog( pParent, ResId( DLG_UUI_LOGIN, pResMgr ) ),

    aErrorInfo      ( this, ResId( INFO_LOGIN_ERROR ) ),
    aErrorGB        ( this, ResId( GB_LOGIN_ERROR ) ),
    aRequestInfo    ( this, ResId( INFO_LOGIN_REQUEST ) ),
    aPathFT         ( this, ResId( FT_LOGIN_PATH ) ),
    aPathED         ( this, ResId( ED_LOGIN_PATH ) ),
    aPathInfo       ( this, ResId( INFO_LOGIN_PATH ) ),
    aPathBtn        ( this, ResId( BTN_LOGIN_PATH ) ),
    aNameFT         ( this, ResId( FT_LOGIN_USERNAME ) ),
    aNameED         ( this, ResId( ED_LOGIN_USERNAME ) ),
    aNameInfo       ( this, ResId( INFO_LOGIN_USERNAME ) ),
    aPasswordFT     ( this, ResId( FT_LOGIN_PASSWORD ) ),
    aPasswordED     ( this, ResId( ED_LOGIN_PASSWORD ) ),
    aAccountFT      ( this, ResId( FT_LOGIN_ACCOUNT ) ),
    aAccountED      ( this, ResId( ED_LOGIN_ACCOUNT ) ),
    aSavePasswdBtn  ( this, ResId( CB_LOGIN_SAVEPASSWORD ) ),
    aLoginGB        ( this, ResId( GB_LOGIN_LOGIN ) ),
    aOKBtn          ( this, ResId( BTN_LOGIN_OK ) ),
    aCancelBtn      ( this, ResId( BTN_LOGIN_CANCEL ) ),
    aHelpBtn        ( this, ResId( BTN_LOGIN_HELP ) )

{
    // Einlog-Ort eintragen
    String aServer;

    if ( ( ( nFlags & LF_NO_ACCOUNT ) == LF_NO_ACCOUNT ) && rRealm.Len() )
    {
        aServer = rRealm;
        ( ( aServer += ' ' ) += String( ResId( STR_LOGIN_AT ) ) ) += ' ';
    }
    aServer += rServer;
    String aTxt = aRequestInfo.GetText();
    aTxt.SearchAndReplaceAscii( "%1", aServer );
    aRequestInfo.SetText( aTxt );

    FreeResource();

    aPathED.SetMaxTextLen( _MAX_PATH );
    aNameED.SetMaxTextLen( _MAX_PATH );

    aOKBtn.SetClickHdl( LINK( this, LoginDialog, OKHdl_Impl ) );
    aPathBtn.SetClickHdl( LINK( this, LoginDialog, PathHdl_Impl ) );

    HideControls_Impl( nFlags );
};

// -----------------------------------------------------------------------

void LoginDialog::SetName( const String& rNewName )
{
    aNameED.SetText( rNewName );
    aNameInfo.SetText( rNewName );
}

// -----------------------------------------------------------------------

void LoginDialog::ClearPassword()
{
    aPasswordED.SetText( String() );

    if ( aNameED.GetText().Len() == 0 )
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
