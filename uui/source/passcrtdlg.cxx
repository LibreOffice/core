/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: passcrtdlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 08:20:04 $
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
#ifndef UUI_PASSCRTDLG_HRC
#include <passcrtdlg.hrc>
#endif
#ifndef UUI_PASSCRTDLG_HXX
#include <passcrtdlg.hxx>
#endif

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

PasswordCreateDialog::PasswordCreateDialog
(
    Window*                                     pParent,
    ResMgr*                                     pResMgr
) :

    ModalDialog( pParent, ResId( DLG_UUI_PASSWORD_CRT, *pResMgr ) ),

    aFTPasswordCrt      ( this, ResId( FT_PASSWORD_CRT, *pResMgr ) ),
    aEDPasswordCrt      ( this, ResId( ED_PASSWORD_CRT, *pResMgr ) ),
    aFTPasswordRepeat   ( this, ResId( FT_PASSWORD_REPEAT, *pResMgr ) ),
    aEDPasswordRepeat   ( this, ResId( ED_PASSWORD_REPEAT, *pResMgr ) ),
    aOKBtn                  ( this, ResId( BTN_PASSCRT_OK, *pResMgr ) ),
    aCancelBtn              ( this, ResId( BTN_PASSCRT_CANCEL, *pResMgr ) ),
    aHelpBtn                ( this, ResId( BTN_PASSCRT_HELP, *pResMgr ) ),
    pResourceMgr            ( pResMgr ),
    nMinLen(5)
{
    FreeResource();

    aOKBtn.SetClickHdl( LINK( this, PasswordCreateDialog, OKHdl_Impl ) );
    aEDPasswordCrt.SetModifyHdl( LINK( this, PasswordCreateDialog, EditHdl_Impl ) );

    aOKBtn.Enable( sal_False );
};

