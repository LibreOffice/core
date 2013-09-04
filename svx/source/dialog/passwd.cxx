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

#include <comphelper/string.hxx>
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include "svx/passwd.hxx"
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include "passwd.hrc"

// class SvxPasswordDialog -----------------------------------------------

IMPL_LINK_NOARG(SvxPasswordDialog, ButtonHdl)
{
    bool bOK = true;
    short nRet = RET_OK;
    OUString aEmpty;

    if ( aNewPasswdED.GetText() != aRepeatPasswdED.GetText() )
    {
        ErrorBox( this, WB_OK, aRepeatPasswdErrStr ).Execute();
        aNewPasswdED.SetText( aEmpty );
        aRepeatPasswdED.SetText( aEmpty );
        aNewPasswdED.GrabFocus();
        bOK = false;
    }

    if ( bOK && aCheckPasswordHdl.IsSet() && !aCheckPasswordHdl.Call( this ) )
    {
        ErrorBox( this, WB_OK, aOldPasswdErrStr ).Execute();
        aOldPasswdED.SetText( aEmpty );
        aOldPasswdED.GrabFocus();
        bOK = false;
    }

    if ( bOK )
        EndDialog( nRet );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPasswordDialog, EditModifyHdl)
{
    if ( !bEmpty )
    {
        OUString aPasswd = comphelper::string::strip(aRepeatPasswdED.GetText(), ' ');
        if ( aPasswd.isEmpty() && aOKBtn.IsEnabled() )
            aOKBtn.Disable();
        else if ( !aPasswd.isEmpty() && !aOKBtn.IsEnabled() )
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
    aOldPasswdErrStr    ( SVX_RESSTR( STR_ERR_OLD_PASSWD ) ),
    aRepeatPasswdErrStr ( SVX_RESSTR( STR_ERR_REPEAT_PASSWD ) ),
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
