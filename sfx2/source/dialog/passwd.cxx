/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: passwd.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_sfx2.hxx"

// Include ---------------------------------------------------------------
#include <vcl/msgbox.hxx>
#ifndef GCC
#endif

#include <sfx2/passwd.hxx>
#include "sfxtypes.hxx"
#include "sfxresid.hxx"

#include "dialog.hrc"
#include "passwd.hrc"

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

    maUserFT        ( this, SfxResId( FT_PASSWD_USER ) ),
    maUserED        ( this, SfxResId( ED_PASSWD_USER ) ),
    maPasswordFT    ( this, SfxResId( FT_PASSWD_PASSWORD ) ),
    maPasswordED    ( this, SfxResId( ED_PASSWD_PASSWORD ) ),
    maConfirmFT     ( this, SfxResId( FT_PASSWD_CONFIRM ) ),
    maConfirmED     ( this, SfxResId( ED_PASSWD_CONFIRM ) ),
    maPasswordBox   ( this, SfxResId( GB_PASSWD_PASSWORD ) ),
    maOKBtn         ( this, SfxResId( BTN_PASSWD_OK ) ),
    maCancelBtn     ( this, SfxResId( BTN_PASSWD_CANCEL ) ),
    maHelpBtn       ( this, SfxResId( BTN_PASSWD_HELP ) ),
    maConfirmStr    (       SfxResId( STR_PASSWD_CONFIRM ) ),

    mnMinLen        ( 5 ),
    mnExtras        ( 0 )

{
    FreeResource();

    Link aLink = LINK( this, SfxPasswordDialog, EditModifyHdl );
    maPasswordED.SetModifyHdl( aLink );
    aLink = LINK( this, SfxPasswordDialog, OKHdl );
    maOKBtn.SetClickHdl( aLink );

    if ( pGroupText )
          maPasswordBox.SetText( *pGroupText );
}

// -----------------------------------------------------------------------

void SfxPasswordDialog::SetMinLen( USHORT nLen )
{
    mnMinLen = nLen;
    EditModifyHdl( NULL );
}

// -----------------------------------------------------------------------

void SfxPasswordDialog::SetMaxLen( USHORT nLen )
{
    maPasswordED.SetMaxTextLen( nLen );
    maConfirmED.SetMaxTextLen( nLen );
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


