/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

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


