/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: passwd.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:34:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#define _SVX_PASSWD_CXX

#include "passwd.hxx"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "passwd.hrc"

// class SvxPasswordDialog -----------------------------------------------

IMPL_LINK( SvxPasswordDialog, ButtonHdl, OKButton *, EMPTYARG )
{
    BOOL bOK = TRUE;
    short nRet = RET_OK;
    String aEmpty;

    if ( aNewPasswdED.GetText() != aRepeatPasswdED.GetText() )
    {
        ErrorBox( this, WB_OK, aRepeatPasswdErrStr ).Execute();
        aNewPasswdED.SetText( aEmpty );
        aRepeatPasswdED.SetText( aEmpty );
        aNewPasswdED.GrabFocus();
        bOK = FALSE;
    }

    if ( bOK && aCheckPasswordHdl.IsSet() && !aCheckPasswordHdl.Call( this ) )
    {
        ErrorBox( this, WB_OK, aOldPasswdErrStr ).Execute();
        aOldPasswdED.SetText( aEmpty );
        aOldPasswdED.GrabFocus();
        bOK = FALSE;
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

SvxPasswordDialog::SvxPasswordDialog( Window* pParent, BOOL bAllowEmptyPasswords, BOOL bDisableOldPassword ) :
    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_PASSWORD ) ),
    aOldFL          ( this, ResId( FL_OLD_PASSWD ) ),
    aOldPasswdFT    ( this, ResId( FT_OLD_PASSWD ) ),
    aOldPasswdED    ( this, ResId( ED_OLD_PASSWD ) ),
    aNewFL          ( this, ResId( FL_NEW_PASSWD ) ),
    aNewPasswdFT    ( this, ResId( FT_NEW_PASSWD ) ),
    aNewPasswdED    ( this, ResId( ED_NEW_PASSWD ) ),
    aRepeatPasswdFT ( this, ResId( FT_REPEAT_PASSWD ) ),
    aRepeatPasswdED ( this, ResId( ED_REPEAT_PASSWD ) ),
    aOKBtn          ( this, ResId( BTN_PASSWD_OK ) ),
    aEscBtn         ( this, ResId( BTN_PASSWD_ESC ) ),
    aHelpBtn        ( this, ResId( BTN_PASSWD_HELP ) ),
    aOldPasswdErrStr    ( ResId( STR_ERR_OLD_PASSWD ) ),
    aRepeatPasswdErrStr ( ResId( STR_ERR_REPEAT_PASSWD ) ),
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


