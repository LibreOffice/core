/*************************************************************************
 *
 *  $RCSfile: passwd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:31 $
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

// Include ---------------------------------------------------------------

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#pragma hdrstop

#include "passwd.hxx"
#include "sfxtypes.hxx"
#include "sfxresid.hxx"

#include "dialog.hrc"
#include "passwd.hrc"

#define MAX_PASSWORD_LEN    ((USHORT)16)

// -----------------------------------------------------------------------

#if (SUPD < 555)
void SfxPasswordDialog::SetConfirm()
{
    SetText( maConfirmStr );
    maPasswordED.SetText( String() );
    maPasswordED.GrabFocus();
}
#endif

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxPasswordDialog, EditModifyHdl, Edit *, EMPTYARG )
{
    maOKBtn.Enable( maPasswordED.GetText().Len() >= mnMinLen );
    return 0;
}
IMPL_LINK_INLINE_END( SfxPasswordDialog, EditModifyHdl, Edit *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK( SfxPasswordDialog, OKHdl, OKButton *, EMPTYARG )
{
    if ( ( ( mnExtras & SHOWEXTRAS_CONFIRM ) == SHOWEXTRAS_CONFIRM ) &&
         ( GetConfirm() != GetPassword() ) )
    {
        ErrorBox aBox( this, SfxResId( MSG_ERROR_WRONG_CONFIRM ) );
        aBox.Execute();
        maConfirmED.SetText( String() );
        maConfirmED.GrabFocus();
    }
    else
        EndDialog( RET_OK );
    return 0;
}

// CTOR / DTOR -----------------------------------------------------------

SfxPasswordDialog::SfxPasswordDialog( Window* pParent, const String* pGroupText ) :

    ModalDialog( pParent, SfxResId ( DLG_PASSWD ) ),

    maUserFT        ( this, ResId( FT_PASSWD_USER ) ),
    maUserED        ( this, ResId( ED_PASSWD_USER ) ),
    maPasswordFT    ( this, ResId( FT_PASSWD_PASSWORD ) ),
    maPasswordED    ( this, ResId( ED_PASSWD_PASSWORD ) ),
    maConfirmFT     ( this, ResId( FT_PASSWD_CONFIRM ) ),
    maConfirmED     ( this, ResId( ED_PASSWD_CONFIRM ) ),
    maPasswordBox   ( this, ResId( GB_PASSWD_PASSWORD ) ),
    maOKBtn         ( this, ResId( BTN_PASSWD_OK ) ),
    maCancelBtn     ( this, ResId( BTN_PASSWD_CANCEL ) ),
    maHelpBtn       ( this, ResId( BTN_PASSWD_HELP ) ),
    maConfirmStr    (       ResId( STR_PASSWD_CONFIRM ) ),

    mnMinLen        ( 5 ),
    mnExtras        ( 0 )

{
    FreeResource();

    Link aLink = LINK( this, SfxPasswordDialog, EditModifyHdl );
    maPasswordED.SetModifyHdl( aLink );
    aLink = LINK( this, SfxPasswordDialog, OKHdl );
    maOKBtn.SetClickHdl( aLink );

    maPasswordED.SetMaxTextLen( MAX_PASSWORD_LEN );
    maConfirmED.SetMaxTextLen( MAX_PASSWORD_LEN );

    if ( pGroupText )
          maPasswordBox.SetText( *pGroupText );
}

// -----------------------------------------------------------------------

void SfxPasswordDialog::SetMinLen( USHORT nLen )
{
    mnMinLen = Min( nLen, MAX_PASSWORD_LEN );
    EditModifyHdl( NULL );
}

// -----------------------------------------------------------------------

short SfxPasswordDialog::Execute()
{
    if ( mnExtras < SHOWEXTRAS_ALL )
    {
        Size a3Size = LogicToPixel( Size( 3, 3 ), MAP_APPFONT );
        Size a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
        long nMinHeight = maHelpBtn.GetPosPixel().Y() +
                          maHelpBtn.GetSizePixel().Height() + a6Size.Height();
        USHORT nRowHided = 1;

        if ( SHOWEXTRAS_NONE == mnExtras )
        {
            maUserFT.Hide();
            maUserED.Hide();
            maConfirmFT.Hide();
            maConfirmED.Hide();
            maPasswordFT.Hide();

            Point aPos = maUserFT.GetPosPixel();
            long nEnd = maUserED.GetPosPixel().X() + maUserED.GetSizePixel().Width();
            maPasswordED.SetPosPixel( aPos );
            Size aSize = maPasswordED.GetSizePixel();
            aSize.Width() = nEnd - aPos.X();
            maPasswordED.SetSizePixel( aSize );

            nRowHided = 2;
        }
        else if ( SHOWEXTRAS_USER == mnExtras )
        {
            maConfirmFT.Hide();
            maConfirmED.Hide();
        }
        else if ( SHOWEXTRAS_CONFIRM == mnExtras )
        {
            maUserFT.Hide();
            maUserED.Hide();

            Point aPwdPos1 = maPasswordFT.GetPosPixel();
            Point aPwdPos2 = maPasswordED.GetPosPixel();

            Point aPos = maUserFT.GetPosPixel();
            maPasswordFT.SetPosPixel( aPos );
            aPos = maUserED.GetPosPixel();
            maPasswordED.SetPosPixel( aPos );

            maConfirmFT.SetPosPixel( aPwdPos1 );
            maConfirmED.SetPosPixel( aPwdPos2 );
        }

        Size aBoxSize = maPasswordBox.GetSizePixel();
        aBoxSize.Height() -= ( nRowHided * maUserED.GetSizePixel().Height() );
        aBoxSize.Height() -= ( nRowHided * a3Size.Height() );
        maPasswordBox.SetSizePixel( aBoxSize );

        long nDlgHeight = maPasswordBox.GetPosPixel().Y() + aBoxSize.Height() + a6Size.Height();
        if ( nDlgHeight < nMinHeight )
            nDlgHeight = nMinHeight;
        Size aDlgSize = GetOutputSizePixel();
        aDlgSize.Height() = nDlgHeight;
        SetOutputSizePixel( aDlgSize );
    }

    return ModalDialog::Execute();
}


