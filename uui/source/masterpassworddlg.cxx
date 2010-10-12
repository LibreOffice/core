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

#include <svtools/filedlg.hxx>
#include <vcl/msgbox.hxx>

#include <ids.hrc>
#include <masterpassworddlg.hrc>
#include <masterpassworddlg.hxx>

// MasterPasswordDialog---------------------------------------------------

// -----------------------------------------------------------------------

IMPL_LINK( MasterPasswordDialog, OKHdl_Impl, OKButton *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 1;
}

// -----------------------------------------------------------------------

MasterPasswordDialog::MasterPasswordDialog
(
    Window*                                     pParent,
    ::com::sun::star::task::PasswordRequestMode aDialogMode,
    ResMgr*                                     pResMgr
) :

    ModalDialog( pParent, ResId( DLG_UUI_MASTERPASSWORD, *pResMgr ) ),

    aFTMasterPassword       ( this, ResId( FT_MASTERPASSWORD, *pResMgr ) ),
    aEDMasterPassword       ( this, ResId( ED_MASTERPASSWORD, *pResMgr ) ),
    aFL ( this, ResId( FL_FIXED_LINE, *pResMgr ) ),
    aOKBtn                  ( this, ResId( BTN_MASTERPASSWORD_OK, *pResMgr ) ),
    aCancelBtn              ( this, ResId( BTN_MASTERPASSWORD_CANCEL, *pResMgr ) ),
    aHelpBtn                ( this, ResId( BTN_MASTERPASSWORD_HELP, *pResMgr ) ),
    nDialogMode             ( aDialogMode ),
    pResourceMgr            ( pResMgr )
{
    if( nDialogMode == ::com::sun::star::task::PasswordRequestMode_PASSWORD_REENTER )
    {
        String aErrorMsg( ResId( STR_ERROR_MASTERPASSWORD_WRONG, *pResourceMgr ));
        ErrorBox aErrorBox( pParent, WB_OK, aErrorMsg );
        aErrorBox.Execute();
    }

    FreeResource();

    aOKBtn.SetClickHdl( LINK( this, MasterPasswordDialog, OKHdl_Impl ) );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
