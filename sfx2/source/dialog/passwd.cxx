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


// Include ---------------------------------------------------------------
#include <vcl/msgbox.hxx>

#include <sfx2/passwd.hxx>
#include "sfxtypes.hxx"
#include "sfx2/sfxresid.hxx"

#include "dialog.hrc"
#include "passwd.hrc"

#include "vcl/arrange.hxx"

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxPasswordDialog, EditModifyHdl, Edit *, pEdit )
{
    if( mbAsciiOnly && (pEdit == &maPasswordED || pEdit == &maPassword2ED) )
    {
        rtl::OUString aTest( pEdit->GetText() );
        const sal_Unicode* pTest = aTest.getStr();
        sal_Int32 nLen = aTest.getLength();
        rtl::OUStringBuffer aFilter( nLen );
        bool bReset = false;
        for( sal_Int32 i = 0; i < nLen; i++ )
        {
            if( *pTest > 0x007f )
                bReset = true;
            else
                aFilter.append( *pTest );
            pTest++;
        }
        if( bReset )
        {
            pEdit->SetSelection( Selection( 0, nLen ) );
            pEdit->ReplaceSelected( aFilter.makeStringAndClear() );
        }

    }
    bool bEnable = maPasswordED.GetText().Len() >= mnMinLen;
    if( maPassword2ED.IsVisible() )
        bEnable = (bEnable && (maPassword2ED.GetText().Len() >= mnMinLen));
    maOKBtn.Enable( bEnable );
    return 0;
}
IMPL_LINK_INLINE_END(SfxPasswordDialog, EditModifyHdl, Edit *, pEdit)

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SfxPasswordDialog, OKHdl)
{
    bool bConfirmFailed = ( ( mnExtras & SHOWEXTRAS_CONFIRM ) == SHOWEXTRAS_CONFIRM ) &&
                            ( GetConfirm() != GetPassword() );
    if( ( mnExtras & SHOWEXTRAS_CONFIRM2 ) == SHOWEXTRAS_CONFIRM2 && ( GetConfirm2() != GetPassword2() ) )
        bConfirmFailed = true;
    if ( bConfirmFailed )
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

    maPasswordBox   ( this, SfxResId( GB_PASSWD_PASSWORD ) ),
    maUserFT        ( this, SfxResId( FT_PASSWD_USER ) ),
    maUserED        ( this, SfxResId( ED_PASSWD_USER ) ),
    maPasswordFT    ( this, SfxResId( FT_PASSWD_PASSWORD ) ),
    maPasswordED    ( this, SfxResId( ED_PASSWD_PASSWORD ) ),
    maConfirmFT     ( this, SfxResId( FT_PASSWD_CONFIRM ) ),
    maConfirmED     ( this, SfxResId( ED_PASSWD_CONFIRM ) ),

    maMinLengthFT   ( this, SfxResId( FT_PASSWD_MINLEN ) ),
    maPassword2Box  ( this, 0 ),
    maPassword2FT   ( this, SfxResId( FT_PASSWD_PASSWORD2 ) ),
    maPassword2ED   ( this, SfxResId( ED_PASSWD_PASSWORD2 ) ),
    maConfirm2FT    ( this, SfxResId( FT_PASSWD_CONFIRM2 ) ),
    maConfirm2ED    ( this, SfxResId( ED_PASSWD_CONFIRM2 ) ),
    maOKBtn         ( this, SfxResId( BTN_PASSWD_OK ) ),
    maCancelBtn     ( this, SfxResId( BTN_PASSWD_CANCEL ) ),
    maHelpBtn       ( this, SfxResId( BTN_PASSWD_HELP ) ),

    maMinLenPwdStr  ( SfxResId( STR_PASSWD_MIN_LEN ).toString() ),
    maEmptyPwdStr   ( SfxResId( STR_PASSWD_EMPTY ).toString() ),
    maMainPwdStr    ( ),
    mnMinLen        ( 5 ),
    mnExtras        ( 0 ),
    mbAsciiOnly     ( false )

{
    maPasswordED.SetAccessibleName(SfxResId(TEXT_PASSWD).toString());
    FreeResource();

    // setup layout
    boost::shared_ptr<vcl::RowOrColumn> xLayout =
        boost::dynamic_pointer_cast<vcl::RowOrColumn>( getLayout() );
    xLayout->setOuterBorder( 0 );

    // get edit size, should be used as minimum
    Size aEditSize( maUserED.GetSizePixel() );

    // add labelcolumn for the labeled edit fields
    boost::shared_ptr<vcl::LabelColumn> xEdits( new vcl::LabelColumn( xLayout.get() ) );
    size_t nChildIndex = xLayout->addChild( xEdits );
    xLayout->setBorders( nChildIndex, -2, -2, -2, 0 );

    // add group box
    xEdits->addWindow( &maPasswordBox );

    // add user line
    xEdits->addRow( &maUserFT, &maUserED, -2, aEditSize );

    // add password line
    xEdits->addRow( &maPasswordFT, &maPasswordED, -2, aEditSize );

    // add confirm line
    xEdits->addRow( &maConfirmFT, &maConfirmED, -2, aEditSize );

    // add second group box
    xEdits->addWindow( &maPassword2Box );

    // add second password line
    xEdits->addRow( &maPassword2FT, &maPassword2ED, -2, aEditSize );

    // add second confirm line
    xEdits->addRow( &maConfirm2FT, &maConfirm2ED, -2, aEditSize );

    // add password length warning line
    xEdits->addWindow( &maMinLengthFT );

    // add a FixedLine
    FixedLine* pLine = new FixedLine( this, 0 );
    pLine->Show();
    addWindow( pLine, true );
    xLayout->addWindow( pLine );

    // add button column
    Size aBtnSize( maCancelBtn.GetSizePixel() );
    boost::shared_ptr<vcl::RowOrColumn> xButtons( new vcl::RowOrColumn( xLayout.get(), false ) );
    nChildIndex = xLayout->addChild( xButtons );
    xLayout->setBorders( nChildIndex, -2, 0, -2, -2 );

    xButtons->addWindow( &maHelpBtn, 0, aBtnSize );
    xButtons->addChild( new vcl::Spacer( xButtons.get() ) );
    xButtons->addWindow( &maOKBtn, 0, aBtnSize );
    xButtons->addWindow( &maCancelBtn, 0, aBtnSize );

    Link aLink = LINK( this, SfxPasswordDialog, EditModifyHdl );
    maPasswordED.SetModifyHdl( aLink );
    maPassword2ED.SetModifyHdl( aLink );
    aLink = LINK( this, SfxPasswordDialog, OKHdl );
    maOKBtn.SetClickHdl( aLink );

    if ( pGroupText )
          maPasswordBox.SetText( *pGroupText );

//set the text to the pasword length
    SetPasswdText();
}

// -----------------------------------------------------------------------

void SfxPasswordDialog::SetPasswdText( )
{
//set the new string to the minimum password length
    if( mnMinLen == 0 )
        maMinLengthFT.SetText( maEmptyPwdStr );
    else
    {
        maMainPwdStr = maMinLenPwdStr;
        maMainPwdStr.SearchAndReplace( rtl::OUString("$(MINLEN)"), String::CreateFromInt32((sal_Int32) mnMinLen ), 0);
        maMinLengthFT.SetText( maMainPwdStr );
        maMinLengthFT.Show();
    }
}

// -----------------------------------------------------------------------

void SfxPasswordDialog::SetMinLen( sal_uInt16 nLen )
{
    mnMinLen = nLen;
    SetPasswdText();
    EditModifyHdl( NULL );
}

// -----------------------------------------------------------------------

short SfxPasswordDialog::Execute()
{
    maUserFT.Hide();
    maUserED.Hide();
    maConfirmFT.Hide();
    maConfirmED.Hide();
    maPasswordFT.Hide();
    maPassword2Box.Hide();
    maPassword2FT.Hide();
    maPassword2ED.Hide();
    maPassword2FT.Hide();
    maConfirm2FT.Hide();
    maConfirm2ED.Hide();

    if( mnExtras != SHOWEXTRAS_NONE )
        maPasswordFT.Show();
    if( (mnExtras & SHOWEXTRAS_USER ) )
    {
        //TODO: Inevitably this layout logic will be wrong post merge until we can see the dialog to test it.
        Size a3Size = LogicToPixel( Size( 3, 3 ), MAP_APPFONT );
        Size a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
        long nMinHeight = maHelpBtn.GetPosPixel().Y() +
                          maHelpBtn.GetSizePixel().Height() + a6Size.Height();
        sal_uInt16 nRowHided = 1;

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

            aPos = maConfirmFT.GetPosPixel();
            maConfirmFT.SetPosPixel( aPwdPos1 );
            maConfirmED.SetPosPixel( aPwdPos2 );
            maMinLengthFT.SetPosPixel(aPos);
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

        maUserFT.Show();
        maUserED.Show();
    }
    if( (mnExtras & SHOWEXTRAS_CONFIRM ) )
    {
        maConfirmFT.Show();
        maConfirmED.Show();
    }
    if( (mnExtras & SHOWEXTRAS_PASSWORD2) )
    {
        maPassword2Box.Show();
        maPassword2FT.Show();
        maPassword2ED.Show();
    }
    if( (mnExtras & SHOWEXTRAS_CONFIRM2 ) )
    {
        maConfirm2FT.Show();
        maConfirm2ED.Show();
    }

    boost::shared_ptr<vcl::RowOrColumn> xLayout =
        boost::dynamic_pointer_cast<vcl::RowOrColumn>( getLayout() );
    SetSizePixel( xLayout->getOptimalSize( WINDOWSIZE_PREFERRED ) );

    return ModalDialog::Execute();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
