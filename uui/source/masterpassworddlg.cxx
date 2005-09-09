/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: masterpassworddlg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:24:05 $
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

#ifndef _SVT_FILEDLG_HXX
#include <svtools/filedlg.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef UUI_IDS_HRC
#include <ids.hrc>
#endif
#ifndef UUI_MASTERPASSWORDDLG_HRC
#include <masterpassworddlg.hrc>
#endif
#ifndef UUI_MASTERPASSWORDDLG_HXX
#include <masterpassworddlg.hxx>
#endif

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

    ModalDialog( pParent, ResId( DLG_UUI_MASTERPASSWORD, pResMgr ) ),

    aFTMasterPassword       ( this, ResId( FT_MASTERPASSWORD ) ),
    aEDMasterPassword       ( this, ResId( ED_MASTERPASSWORD ) ),
    aOKBtn                  ( this, ResId( BTN_MASTERPASSWORD_OK ) ),
    aCancelBtn              ( this, ResId( BTN_MASTERPASSWORD_CANCEL ) ),
    aHelpBtn                ( this, ResId( BTN_MASTERPASSWORD_HELP ) ),
    nDialogMode             ( aDialogMode ),
    pResourceMgr            ( pResMgr )
{
    if( nDialogMode == ::com::sun::star::task::PasswordRequestMode_PASSWORD_REENTER )
    {
        String aErrorMsg( ResId( STR_ERROR_MASTERPASSWORD_WRONG, pResourceMgr ));
        ErrorBox aErrorBox( pParent, WB_OK, aErrorMsg );
        aErrorBox.Execute();
    }

    FreeResource();

    aOKBtn.SetClickHdl( LINK( this, MasterPasswordDialog, OKHdl_Impl ) );
};

