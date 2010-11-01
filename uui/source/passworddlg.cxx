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

#include <sal/macros.h>
#include <svtools/filedlg.hxx>
#include <vcl/msgbox.hxx>

#include <ids.hrc>
#include <passworddlg.hrc>
#include <passworddlg.hxx>

// PasswordDialog---------------------------------------------------------

// -----------------------------------------------------------------------

IMPL_LINK( PasswordDialog, OKHdl_Impl, OKButton *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 1;
}

// -----------------------------------------------------------------------

PasswordDialog::PasswordDialog(
    Window* _pParent,
    ::com::sun::star::task::PasswordRequestMode nDlgMode,
    ResMgr * pResMgr,
    rtl::OUString& aDocURL,
    bool bOpenToModify )

    :ModalDialog( _pParent, ResId( DLG_UUI_PASSWORD, *pResMgr ) )
    ,aFTPassword( this, ResId( FT_PASSWORD, *pResMgr ))
    ,aEDPassword( this, ResId( ED_PASSWORD, *pResMgr ))
    ,aOKBtn ( this, ResId( BTN_PASSWORD_OK, *pResMgr ))
    ,aCancelBtn ( this, ResId( BTN_PASSWORD_CANCEL, *pResMgr ))
    ,aHelpBtn ( this, ResId( BTN_PASSWORD_HELP, *pResMgr ))
    ,aFixedLine1( this, ResId( FL_FIXED_LINE_1, *pResMgr ))
    ,nDialogMode( nDlgMode )
    ,pResourceMgr ( pResMgr )
{
    if( nDialogMode == ::com::sun::star::task::PasswordRequestMode_PASSWORD_REENTER )
    {
        USHORT nErrStrId = bOpenToModify ? STR_ERROR_PASSWORD_TO_MODIFY_WRONG : STR_ERROR_PASSWORD_TO_OPEN_WRONG;
        String aErrorMsg( ResId( nErrStrId, *pResourceMgr ));
        ErrorBox aErrorBox( _pParent, WB_OK, aErrorMsg );
        aErrorBox.Execute();
    }

    USHORT nStrId = bOpenToModify ? STR_ENTER_PASSWORD_TO_MODIFY : STR_ENTER_PASSWORD_TO_OPEN;
    aFTPassword.SetText( String( ResId( nStrId, *pResourceMgr ) ) );

    FreeResource();

    aFTPassword.SetText( aFTPassword.GetText() + aDocURL );

    aOKBtn.SetClickHdl( LINK( this, PasswordDialog, OKHdl_Impl ) );

    long nLabelWidth = aFTPassword.GetSizePixel().Width();
    long nLabelHeight = aFTPassword.GetSizePixel().Height();
    long nTextWidth = aFTPassword.GetCtrlTextWidth( aFTPassword.GetText() );
    long nTextHeight = aFTPassword.GetTextHeight();

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

    Window* pControls[] = { &aEDPassword, &aFixedLine1, &aOKBtn, &aCancelBtn, &aHelpBtn };
    const sal_Int32 nCCount = SAL_N_ELEMENTS( pControls );
    for ( int i = 0; i < nCCount; ++i )
    {
        Point aNewPos =(*pControls[i]).GetPosPixel();
        aNewPos.Y() += nDelta;
        pControls[i]->SetPosSizePixel( aNewPos, pControls[i]->GetSizePixel() );
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
