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

#include "passworddlg.hxx"
#include "passworddlg.hrc"
#include "ids.hrc"

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
    OUString& aDocURL,
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
        OUString aErrorMsg(ResId(nErrStrId, *pResourceMgr).toString());
        ErrorBox aErrorBox( GetParent(), WB_OK, aErrorMsg );
        aErrorBox.Execute();
    }

    // default settings for enter password or reenter passwd...
    OUString aTitle(ResId(STR_TITLE_ENTER_PASSWORD, *pResourceMgr).toString());
    aFTConfirmPassword.Hide();
    aEDConfirmPassword.Hide();
    aFTConfirmPassword.Enable( sal_False );
    aEDConfirmPassword.Enable( sal_False );

    // settings for create password
    if (nDialogMode == task::PasswordRequestMode_PASSWORD_CREATE)
    {
        aTitle = ResId(STR_TITLE_CREATE_PASSWORD, *pResourceMgr).toString();

        aFTConfirmPassword.SetText(ResId(STR_CONFIRM_SIMPLE_PASSWORD, *pResourceMgr).toString());

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
    aFTPassword.SetText(ResId(nStrId, *pResourceMgr).toString());
    aFTPassword.SetText( aFTPassword.GetText() + aDocURL );
    if (bIsSimplePasswordRequest)
    {
        DBG_ASSERT( aDocURL.isEmpty(), "A simple password request should not have a document URL! Use document password request instead." );
        aFTPassword.SetText(ResId(STR_ENTER_SIMPLE_PASSWORD, *pResourceMgr).toString());
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


IMPL_LINK_NOARG(PasswordDialog, OKHdl_Impl)
{
    bool bEDPasswdValid = aEDPassword.GetText().getLength() >= nMinLen;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

