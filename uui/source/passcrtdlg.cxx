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
#include <passcrtdlg.hrc>
#include <passcrtdlg.hxx>

// PasswordCreateDialog---------------------------------------------------

// -----------------------------------------------------------------------

IMPL_LINK( PasswordCreateDialog, EditHdl_Impl, Edit *, EMPTYARG )
{
    aOKBtn.Enable( aEDPasswordCrt.GetText().Len() >= nMinLen );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( PasswordCreateDialog, OKHdl_Impl, OKButton *, EMPTYARG )
{
    // compare both passwords and show message box if there are not equal!!
    if( aEDPasswordCrt.GetText() == aEDPasswordRepeat.GetText() )
        EndDialog( RET_OK );
    else
    {
        String aErrorMsg( ResId( STR_ERROR_PASSWORDS_NOT_IDENTICAL, *pResourceMgr ));
        ErrorBox aErrorBox( this, WB_OK, aErrorMsg );
        aErrorBox.Execute();
        aEDPasswordCrt.SetText( String() );
        aEDPasswordRepeat.SetText( String() );
        aEDPasswordCrt.GrabFocus();
    }
    return 1;
}

// -----------------------------------------------------------------------

PasswordCreateDialog::PasswordCreateDialog( Window* _pParent, ResMgr * pResMgr, bool bMSCryptoMode)
    :ModalDialog( _pParent, ResId( DLG_UUI_PASSWORD_CRT, *pResMgr ) )
    ,aFTPasswordCrt           ( this, ResId( FT_PASSWORD_CRT, *pResMgr )             )
    ,aEDPasswordCrt       ( this, ResId( ED_PASSWORD_CRT, *pResMgr )        )
    ,aFTPasswordRepeat           ( this, ResId( FT_PASSWORD_REPEAT, *pResMgr )             )
    ,aEDPasswordRepeat       ( this, ResId( ED_PASSWORD_REPEAT, *pResMgr )        )
    ,aFTWarning           ( this, ResId( bMSCryptoMode ? FT_MSPASSWORD_WARNING : FT_PASSWORD_WARNING, *pResMgr )             )
    ,aFixedLine1       ( this, ResId( FL_FIXED_LINE_1, *pResMgr )        )
    ,aOKBtn   ( this, ResId( BTN_PASSCRT_OK, *pResMgr )    )
    ,aCancelBtn   ( this, ResId( BTN_PASSCRT_CANCEL, *pResMgr )    )
    ,aHelpBtn   ( this, ResId( BTN_PASSCRT_HELP, *pResMgr )    )
    ,pResourceMgr           ( pResMgr )
    ,nMinLen(1) // if it should be changed for ODF, it must stay 1 for bMSCryptoMode
{
    FreeResource();

    aOKBtn.SetClickHdl( LINK( this, PasswordCreateDialog, OKHdl_Impl ) );
    aEDPasswordCrt.SetModifyHdl( LINK( this, PasswordCreateDialog, EditHdl_Impl ) );

    aOKBtn.Enable( sal_False );

    if ( bMSCryptoMode )
    {
        aEDPasswordCrt.SetMaxTextLen( 15 );
        aEDPasswordRepeat.SetMaxTextLen( 15 );
    }

    long nLabelWidth = aFTWarning.GetSizePixel().Width();
    long nLabelHeight = aFTWarning.GetSizePixel().Height();
    long nTextWidth = aFTWarning.GetCtrlTextWidth( aFTWarning.GetText() );
    long nTextHeight = aFTWarning.GetTextHeight();

    Rectangle aLabelRect( aFTWarning.GetPosPixel(), aFTWarning.GetSizePixel() );
    Rectangle aRect = aFTWarning.GetTextRect( aLabelRect, aFTWarning.GetText() );

    long nNewLabelHeight = 0;
    for( nNewLabelHeight = ( nTextWidth / nLabelWidth + 1 ) * nTextHeight;
        nNewLabelHeight < aRect.GetHeight();
        nNewLabelHeight += nTextHeight ) {} ;

    long nDelta = nNewLabelHeight - nLabelHeight;

    Size aNewDlgSize = GetSizePixel();
    aNewDlgSize.Height() += nDelta;
    SetSizePixel( aNewDlgSize );

    Size aNewWarningSize = aFTWarning.GetSizePixel();
    aNewWarningSize.Height() = nNewLabelHeight;
    aFTWarning.SetPosSizePixel( aFTWarning.GetPosPixel(), aNewWarningSize );

    Window* pControls[] = { &aFixedLine1, &aOKBtn, &aCancelBtn, &aHelpBtn };
    const sal_Int32 nCCount = SAL_N_ELEMENTS( pControls );
    for ( int i = 0; i < nCCount; ++i )
    {
        Point aNewPos =(*pControls[i]).GetPosPixel();
        aNewPos.Y() += nDelta;
        pControls[i]->SetPosSizePixel( aNewPos, pControls[i]->GetSizePixel() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
