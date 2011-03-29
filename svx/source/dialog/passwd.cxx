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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>

#define _SVX_PASSWD_CXX

#include "svx/passwd.hxx"
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include "passwd.hrc"

// class SvxPasswordDialog -----------------------------------------------

IMPL_LINK( SvxPasswordDialog, ButtonHdl, OKButton *, EMPTYARG )
{
    sal_Bool bOK = sal_True;
    short nRet = RET_OK;
    String aEmpty;

    if ( aNewPasswdED.GetText() != aRepeatPasswdED.GetText() )
    {
        ErrorBox( this, WB_OK, aRepeatPasswdErrStr ).Execute();
        aNewPasswdED.SetText( aEmpty );
        aRepeatPasswdED.SetText( aEmpty );
        aNewPasswdED.GrabFocus();
        bOK = sal_False;
    }

    if ( bOK && aCheckPasswordHdl.IsSet() && !aCheckPasswordHdl.Call( this ) )
    {
        ErrorBox( this, WB_OK, aOldPasswdErrStr ).Execute();
        aOldPasswdED.SetText( aEmpty );
        aOldPasswdED.GrabFocus();
        bOK = sal_False;
    }

    if ( bOK )
        EndDialog( nRet );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPasswordDialog, EditModifyHdl, Edit *, EMPTYARG )
{
    if ( !bEmpty )
    {
        String aPasswd = aRepeatPasswdED.GetText();
        aPasswd.EraseLeadingChars().EraseTrailingChars();

        if ( !aPasswd.Len() && aOKBtn.IsEnabled() )
            aOKBtn.Disable();
        else if ( aPasswd.Len() && !aOKBtn.IsEnabled() )
            aOKBtn.Enable();
    }
    else if ( !aOKBtn.IsEnabled() )
        aOKBtn.Enable();
    return 0;
}

// -----------------------------------------------------------------------

SvxPasswordDialog::SvxPasswordDialog( Window* pParent, sal_Bool bAllowEmptyPasswords, sal_Bool bDisableOldPassword ) :
    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_PASSWORD ) ),
    aOldFL          ( this, SVX_RES( FL_OLD_PASSWD ) ),
    aOldPasswdFT    ( this, SVX_RES( FT_OLD_PASSWD ) ),
    aOldPasswdED    ( this, SVX_RES( ED_OLD_PASSWD ) ),
    aNewFL          ( this, SVX_RES( FL_NEW_PASSWD ) ),
    aNewPasswdFT    ( this, SVX_RES( FT_NEW_PASSWD ) ),
    aNewPasswdED    ( this, SVX_RES( ED_NEW_PASSWD ) ),
    aRepeatPasswdFT ( this, SVX_RES( FT_REPEAT_PASSWD ) ),
    aRepeatPasswdED ( this, SVX_RES( ED_REPEAT_PASSWD ) ),
    aOKBtn          ( this, SVX_RES( BTN_PASSWD_OK ) ),
    aEscBtn         ( this, SVX_RES( BTN_PASSWD_ESC ) ),
    aHelpBtn        ( this, SVX_RES( BTN_PASSWD_HELP ) ),
    aOldPasswdErrStr    ( SVX_RES( STR_ERR_OLD_PASSWD ) ),
    aRepeatPasswdErrStr ( SVX_RES( STR_ERR_REPEAT_PASSWD ) ),
    bEmpty  ( bAllowEmptyPasswords )
{
    FreeResource();

    aOKBtn.SetClickHdl( LINK( this, SvxPasswordDialog, ButtonHdl ) );
    aRepeatPasswdED.SetModifyHdl( LINK( this, SvxPasswordDialog, EditModifyHdl ) );
    EditModifyHdl( 0 );

    if ( bDisableOldPassword )
    {
        aOldFL.Disable();
         aOldPasswdFT.Disable();
        aOldPasswdED.Disable();
        aNewPasswdED.GrabFocus();
    }
}

// -----------------------------------------------------------------------

SvxPasswordDialog::~SvxPasswordDialog()
{
}

// -----------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
