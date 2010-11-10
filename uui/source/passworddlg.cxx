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

#include "passworddlg.hxx"
#include "passworddlg.hrc"
#include "ids.hrc"

#include <svtools/filedlg.hxx>
#include <vcl/msgbox.hxx>


using namespace ::com::sun::star;


// -----------------------------------------------------------------------

static void lcl_Move( Window &rWin, long nOffset )
{
    Point aTmp( rWin.GetPosPixel() );
    aTmp.Y() += nOffset;
    rWin.SetPosPixel( aTmp );
}

// -----------------------------------------------------------------------

PasswordDialog::PasswordDialog(
    Window* _pParent,
    task::PasswordRequestMode nDlgMode,
    ResMgr * pResMgr,
    rtl::OUString& aDocURL,
    bool bOpenToModify,
    bool bIsSimplePasswordRequest )

    :ModalDialog( _pParent, ResId( DLG_UUI_PASSWORD, *pResMgr ) )
    ,aFTPassword( this, ResId( FT_PASSWORD, *pResMgr ))
    ,aEDPassword( this, ResId( ED_PASSWORD, *pResMgr ))
    ,aFTConfirmPassword( this, ResId( FT_CONFIRM_PASSWORD, *pResMgr ))
    ,aEDConfirmPassword( this, ResId( ED_CONFIRM_PASSWORD, *pResMgr ))
    ,aOKBtn ( this, ResId( BTN_PASSWORD_OK, *pResMgr ))
    ,aCancelBtn ( this, ResId( BTN_PASSWORD_CANCEL, *pResMgr ))
    ,aHelpBtn ( this, ResId( BTN_PASSWORD_HELP, *pResMgr ))
    ,aFixedLine1( this, ResId( FL_FIXED_LINE_1, *pResMgr ))
    ,nMinLen(1)
    ,aPasswdMismatch( ResId( STR_PASSWORD_MISMATCH, *pResMgr ))
    ,nDialogMode( nDlgMode )
    ,pResourceMgr ( pResMgr )
{
    if( nDialogMode == task::PasswordRequestMode_PASSWORD_REENTER )
    {
        const sal_uInt16 nOpenToModifyErrStrId = bOpenToModify ? STR_ERROR_PASSWORD_TO_MODIFY_WRONG : STR_ERROR_PASSWORD_TO_OPEN_WRONG;
        const sal_uInt16 nErrStrId = bIsSimplePasswordRequest ? STR_ERROR_SIMPLE_PASSWORD_WRONG : nOpenToModifyErrStrId;
        String aErrorMsg( ResId( nErrStrId, *pResourceMgr ));
        ErrorBox aErrorBox( this, WB_OK, aErrorMsg );
        aErrorBox.Execute();
    }

    // default settings for enter password or reenter passwd...
    String aTitle( ResId( STR_TITLE_ENTER_PASSWORD, *pResourceMgr ) );
    aFTConfirmPassword.Hide();
    aEDConfirmPassword.Hide();
    aFTConfirmPassword.Enable( sal_False );
    aEDConfirmPassword.Enable( sal_False );

    // settings for create password
    if (nDialogMode == task::PasswordRequestMode_PASSWORD_CREATE)
    {
        aTitle = String( ResId( STR_TITLE_CREATE_PASSWORD, *pResourceMgr ) );

        aFTConfirmPassword.SetText( String( ResId( STR_CONFIRM_SIMPLE_PASSWORD, *pResourceMgr ) ) );

        aFTConfirmPassword.Show();
        aEDConfirmPassword.Show();
        aFTConfirmPassword.Enable( sal_True );
        aEDConfirmPassword.Enable( sal_True );
    }
    else
    {
        // shrink dialog by size of hidden controls and move rest up accordingly

        long nDelta = aFixedLine1.GetPosPixel().Y() - aFTConfirmPassword.GetPosPixel().Y();

        lcl_Move( aFixedLine1, -nDelta );
        lcl_Move( aOKBtn, -nDelta );
        lcl_Move( aCancelBtn, -nDelta );
        lcl_Move( aHelpBtn, -nDelta );

        Size aNewDlgSize = GetSizePixel();
        aNewDlgSize.Height() -= nDelta;
        SetSizePixel( aNewDlgSize );
    }

    SetText( aTitle );

    sal_uInt16 nStrId = bOpenToModify ? STR_ENTER_PASSWORD_TO_MODIFY : STR_ENTER_PASSWORD_TO_OPEN;
    aFTPassword.SetText( String( ResId( nStrId, *pResourceMgr ) ) );
    aFTPassword.SetText( aFTPassword.GetText() + aDocURL );
    if (bIsSimplePasswordRequest)
    {
        DBG_ASSERT( aDocURL.getLength() == 0, "A simple password request should not have a document URL! Use document password request instead." );
        aFTPassword.SetText( String( ResId( STR_ENTER_SIMPLE_PASSWORD, *pResourceMgr ) ) );
    }

    FreeResource();

    aOKBtn.SetClickHdl( LINK( this, PasswordDialog, OKHdl_Impl ) );


    //
    // move controls down by extra height needed for aFTPassword
    // (usually only needed if a URL was provided)
    //

    long nLabelWidth    = aFTPassword.GetSizePixel().Width();
    long nLabelHeight   = aFTPassword.GetSizePixel().Height();
    long nTextWidth     = aFTPassword.GetCtrlTextWidth( aFTPassword.GetText() );
    long nTextHeight    = aFTPassword.GetTextHeight();

    Rectangle aLabelRect( aFTPassword.GetPosPixel(), aFTPassword.GetSizePixel() );
    Rectangle aRect = aFTPassword.GetTextRect( aLabelRect, aFTPassword.GetText() );

    long nNewLabelHeight = 0;
    for( nNewLabelHeight = ( nTextWidth / nLabelWidth + 1 ) * nTextHeight;
        nNewLabelHeight < aRect.GetHeight();
        nNewLabelHeight += nTextHeight ) {} ;

    long nDelta = nNewLabelHeight - nLabelHeight;

    Size aNewDlgSize = GetSizePixel();
    aNewDlgSize.Height() += nDelta;
    SetSizePixel( aNewDlgSize );

    Size aNewLabelSize = aFTPassword.GetSizePixel();
    aNewLabelSize.Height() = nNewLabelHeight;
    aFTPassword.SetPosSizePixel( aFTPassword.GetPosPixel(), aNewLabelSize );

    lcl_Move( aEDPassword, nDelta );
    lcl_Move( aFTConfirmPassword, nDelta );
    lcl_Move( aEDConfirmPassword, nDelta );
    lcl_Move( aFixedLine1, nDelta );
    lcl_Move( aOKBtn, nDelta );
    lcl_Move( aCancelBtn, nDelta );
    lcl_Move( aHelpBtn, nDelta );
}


IMPL_LINK( PasswordDialog, OKHdl_Impl, OKButton *, EMPTYARG )
{
    bool bEDPasswdValid = aEDPassword.GetText().Len() >= nMinLen;
    bool bPasswdMismatch = aEDConfirmPassword.GetText() != aEDPassword.GetText();
    bool bValid = (!aEDConfirmPassword.IsVisible() && bEDPasswdValid) ||
            (aEDConfirmPassword.IsVisible() && bEDPasswdValid && !bPasswdMismatch);

    if (aEDConfirmPassword.IsVisible() && bPasswdMismatch)
    {
        ErrorBox aErrorBox( this, WB_OK, aPasswdMismatch );
        aErrorBox.Execute();
    }
    else if (bValid)
        EndDialog( RET_OK );

    return 1;
}

