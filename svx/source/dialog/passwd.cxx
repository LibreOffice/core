/*************************************************************************
 *
 *  $RCSfile: passwd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#pragma hdrstop

#define _SVX_PASSWD_CXX

#include "passwd.hxx"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "passwd.hrc"

// class SvxPasswordDialog -----------------------------------------------

IMPL_LINK( SvxPasswordDialog, OKHdl_Impl, OKButton *, EMPTYARG )
{
    FASTBOOL bOK = TRUE;
    short nRet = RET_OK;
    String aEmpty;

    if ( bVerify )
    {
        if ( aOldPasswdED.GetText() != aOldPassword )
        {
            ErrorBox aBox( this, WB_OK, aOldPasswdErrStr );
            aBox.Execute();
            aOldPasswdED.SetText( aEmpty );
            aOldPasswdED.GrabFocus();
            bOK = FALSE;
        }
    }

    if ( bOK && aNewPasswdED.GetText() != aRepeatPasswdED.GetText() )
    {
        ErrorBox( this, WB_OK, aRepeatPasswdErrStr ).Execute();
        bOK = FALSE;
        aNewPasswdED.SetText( aEmpty );
        aRepeatPasswdED.SetText( aEmpty );
        aNewPasswdED.GrabFocus();
    }

    if ( bOK )
        EndDialog( nRet );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxPasswordDialog, ModifyHdl_Impl, Edit *, EMPTYARG )
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

SvxPasswordDialog::SvxPasswordDialog( Window* pParent,
                                      FASTBOOL bAllowEmptyPasswords ) :

    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_PASSWORD ) ),

    aOldPasswdFT    ( this, ResId( FT_OLD_PASSWD ) ),
    aOldPasswdED    ( this, ResId( ED_OLD_PASSWD ) ),
    aOldGB          ( this, ResId( GB_OLD_PASSWD ) ),
    aNewPasswdFT    ( this, ResId( FT_NEW_PASSWD ) ),
    aNewPasswdED    ( this, ResId( ED_NEW_PASSWD ) ),
    aRepeatPasswdFT ( this, ResId( FT_REPEAT_PASSWD ) ),
    aRepeatPasswdED ( this, ResId( ED_REPEAT_PASSWD ) ),
    aNewGB          ( this, ResId( GB_NEW_PASSWD ) ),
    aOKBtn          ( this, ResId( BTN_PASSWD_OK ) ),
    aEscBtn         ( this, ResId( BTN_PASSWD_ESC ) ),
    aHelpBtn        ( this, ResId( BTN_PASSWD_HELP ) ),

    aOldPasswdErrStr    ( ResId( STR_ERR_OLD_PASSWD ) ),
    aRepeatPasswdErrStr ( ResId( STR_ERR_REPEAT_PASSWD ) ),

    bVerify ( FALSE ),
    bEmpty  ( bAllowEmptyPasswords )

{
    FreeResource();

    aOKBtn.SetClickHdl( LINK( this, SvxPasswordDialog, OKHdl_Impl ) );
    aRepeatPasswdED.SetModifyHdl(
        LINK( this, SvxPasswordDialog, ModifyHdl_Impl ) );
    ModifyHdl_Impl( 0 );
}

// -----------------------------------------------------------------------

SvxPasswordDialog::~SvxPasswordDialog()
{
}


// -----------------------------------------------------------------------

void SvxPasswordDialog::SetOldPassword( const String& rOld )
{
    aOldPassword = rOld;

    if ( !aOldPassword.Len() && !bEmpty )
    {
        bVerify = FALSE;
        aOldPasswdFT.Disable();
        aOldPasswdED.Disable();
        aNewPasswdED.GrabFocus();
    }
    else
        bVerify = TRUE;
}


