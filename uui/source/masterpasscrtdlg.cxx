/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: masterpasscrtdlg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 08:19:27 $
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
#ifndef UUI_MASTERPASSCRTDLG_HRC
#include <masterpasscrtdlg.hrc>
#endif
#ifndef UUI_MASTERPASSCRTDLG_HXX
#include <masterpasscrtdlg.hxx>
#endif

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

    aFTMasterPasswordCrt        ( this, ResId( FT_MASTERPASSWORD_CRT, *pResMgr ) ),
    aEDMasterPasswordCrt        ( this, ResId( ED_MASTERPASSWORD_CRT, *pResMgr ) ),
    aFTMasterPasswordRepeat ( this, ResId( FT_MASTERPASSWORD_REPEAT, *pResMgr ) ),
    aEDMasterPasswordRepeat ( this, ResId( ED_MASTERPASSWORD_REPEAT, *pResMgr ) ),
    aOKBtn                  ( this, ResId( BTN_MASTERPASSCRT_OK, *pResMgr ) ),
    aCancelBtn              ( this, ResId( BTN_MASTERPASSCRT_CANCEL, *pResMgr ) ),
    aHelpBtn                ( this, ResId( BTN_MASTERPASSCRT_HELP, *pResMgr ) ),
    pResourceMgr            ( pResMgr ),
    nMinLen(5)
{
    FreeResource();

    aOKBtn.Enable( sal_False );
    aOKBtn.SetClickHdl( LINK( this, MasterPasswordCreateDialog, OKHdl_Impl ) );
    aEDMasterPasswordCrt.SetModifyHdl( LINK( this, MasterPasswordCreateDialog, EditHdl_Impl ) );
};

