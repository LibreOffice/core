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
#include <masterpasscrtdlg.hrc>
#include <masterpasscrtdlg.hxx>

// MasterPasswordCreateDialog---------------------------------------------------

// -----------------------------------------------------------------------

IMPL_LINK( MasterPasswordCreateDialog, EditHdl_Impl, Edit *, EMPTYARG )
{
    aOKBtn.Enable( aEDMasterPasswordCrt.GetText().Len() >= nMinLen );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( MasterPasswordCreateDialog, OKHdl_Impl, OKButton *, EMPTYARG )
{
    // compare both passwords and show message box if there are not equal!!
    if( aEDMasterPasswordCrt.GetText() == aEDMasterPasswordRepeat.GetText() )
        EndDialog( RET_OK );
    else
    {
        String aErrorMsg( ResId( STR_ERROR_PASSWORDS_NOT_IDENTICAL, *pResourceMgr ));
        ErrorBox aErrorBox( this, WB_OK, aErrorMsg );
        aErrorBox.Execute();
        aEDMasterPasswordCrt.SetText( String() );
        aEDMasterPasswordRepeat.SetText( String() );
        aEDMasterPasswordCrt.GrabFocus();
    }
    return 1;
}

// -----------------------------------------------------------------------

MasterPasswordCreateDialog::MasterPasswordCreateDialog
(
    Window*                                     pParent,
    ResMgr*                                     pResMgr
) :

    ModalDialog( pParent, ResId( DLG_UUI_MASTERPASSWORD_CRT, *pResMgr ) ),

    aFTInfoText             ( this, ResId( FT_INFOTEXT, *pResMgr ) ),
    aFLInfoText             ( this, ResId( FL_INFOTEXT, *pResMgr ) ),

    aFTMasterPasswordCrt    ( this, ResId( FT_MASTERPASSWORD_CRT, *pResMgr ) ),
    aEDMasterPasswordCrt    ( this, ResId( ED_MASTERPASSWORD_CRT, *pResMgr ) ),
    aFTMasterPasswordRepeat ( this, ResId( FT_MASTERPASSWORD_REPEAT, *pResMgr ) ),
    aEDMasterPasswordRepeat ( this, ResId( ED_MASTERPASSWORD_REPEAT, *pResMgr ) ),

    aFTCautionText          ( this, ResId( FT_CAUTIONTEXT, *pResMgr ) ),
    aFLCautionText          ( this, ResId( FL_CAUTIONTEXT, *pResMgr ) ),

    aFTMasterPasswordWarning ( this, ResId( FT_MASTERPASSWORD_WARNING, *pResMgr ) ),
    aFL ( this, ResId( FL_FIXED_LINE, *pResMgr ) ),
    aOKBtn                  ( this, ResId( BTN_MASTERPASSCRT_OK, *pResMgr ) ),
    aCancelBtn              ( this, ResId( BTN_MASTERPASSCRT_CANCEL, *pResMgr ) ),
    aHelpBtn                ( this, ResId( BTN_MASTERPASSCRT_HELP, *pResMgr ) ),

    pResourceMgr            ( pResMgr ),
    nMinLen                 ( 1 )
{
    FreeResource();

    aOKBtn.Enable( sal_False );
    aOKBtn.SetClickHdl( LINK( this, MasterPasswordCreateDialog, OKHdl_Impl ) );
    aEDMasterPasswordCrt.SetModifyHdl( LINK( this, MasterPasswordCreateDialog, EditHdl_Impl ) );

    CalculateTextHeight();

    long nLableWidth = aFTMasterPasswordWarning.GetSizePixel().Width();
    long nLabelHeight = aFTMasterPasswordWarning.GetSizePixel().Height();
    long nTextWidth = aFTMasterPasswordWarning.GetCtrlTextWidth( aFTMasterPasswordWarning.GetText() );
    long nTextHeight = aFTMasterPasswordWarning.GetTextHeight();

    Rectangle aLabelRect( aFTMasterPasswordWarning.GetPosPixel(), aFTMasterPasswordWarning.GetSizePixel() );
    Rectangle aRect = aFTMasterPasswordWarning.GetTextRect( aLabelRect, aFTMasterPasswordWarning.GetText() );

    long nNewLabelHeight = 0;
    if ( nTextWidth > 0 )
    {
        for( nNewLabelHeight = ( nTextWidth / nLableWidth + 1 ) * nTextHeight;
            nNewLabelHeight < aRect.GetHeight();
        nNewLabelHeight += nTextHeight ) {};
    }

    long nDelta = nNewLabelHeight - nLabelHeight;
    Size aNewDlgSize = GetSizePixel();
    aNewDlgSize.Height() += nDelta;
    SetSizePixel( aNewDlgSize );

    Size aNewWarningSize = aFTMasterPasswordWarning.GetSizePixel();
    aNewWarningSize.Height() = nNewLabelHeight;
    aFTMasterPasswordWarning.SetPosSizePixel( aFTMasterPasswordWarning.GetPosPixel(), aNewWarningSize );

    Window* pControls[] = { &aFL, &aOKBtn, &aCancelBtn, &aHelpBtn };
    const sal_Int32 nCCount = SAL_N_ELEMENTS( pControls );
    for ( int i = 0; i < nCCount; ++i )
    {
        Point aNewPos =(*pControls[i]).GetPosPixel();
        aNewPos.Y() += nDelta;
        pControls[i]->SetPosSizePixel( aNewPos, pControls[i]->GetSizePixel() );
    }
};

void MasterPasswordCreateDialog::CalculateTextHeight()
{
    Size aSize = aFTInfoText.GetSizePixel();
    Size aMinSize = aFTInfoText.CalcMinimumSize( aSize.Width() );
    long nDelta = 0;

    if ( aSize.Height() > aMinSize.Height() )
    {
        aFTInfoText.SetSizePixel( aMinSize );
        nDelta = aSize.Height() - aMinSize.Height();
        Window* pWins[] = { &aFLInfoText, &aFTMasterPasswordCrt, &aEDMasterPasswordCrt,
                            &aFTMasterPasswordRepeat, &aEDMasterPasswordRepeat, &aFTCautionText,
                            &aFLCautionText, &aOKBtn, &aCancelBtn, &aHelpBtn };
        Window** pWindow = pWins;
        const sal_Int32 nCount = SAL_N_ELEMENTS( pWins );
        for ( sal_Int32 i = 0; i < nCount; ++i, ++pWindow )
        {
            Point aNewPos = (*pWindow)->GetPosPixel();
            aNewPos.Y() -= nDelta;
            (*pWindow)->SetPosPixel( aNewPos );
        }
    }

    aSize = aFTCautionText.GetSizePixel();
    aMinSize = aFTCautionText.CalcMinimumSize( aSize.Width() );

    if ( aSize.Height() > aMinSize.Height() )
    {
        aFTCautionText.SetSizePixel( aMinSize );
        long nTemp = aSize.Height() - aMinSize.Height();
        nDelta += nTemp;
        Window* pWins[] = { &aFLCautionText, &aOKBtn, &aCancelBtn, &aHelpBtn };
        Window** pWindow = pWins;
        const sal_Int32 nCount = SAL_N_ELEMENTS( pWins );
        for ( sal_Int32 i = 0; i < nCount; ++i, ++pWindow )
        {
            Point aNewPos = (*pWindow)->GetPosPixel();
            aNewPos.Y() -= nTemp;
            (*pWindow)->SetPosPixel( aNewPos );
        }
    }

    if ( nDelta > 0 )
    {
        Size aDlgSize = GetOutputSizePixel();
        aDlgSize.Height() -= nDelta;
        SetSizePixel( aDlgSize );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
