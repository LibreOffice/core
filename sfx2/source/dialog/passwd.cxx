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

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxPasswordDialog, EditModifyHdl, Edit *, pEdit )
{
    if (mbAsciiOnly && (pEdit == mpPassword1ED || pEdit == mpPassword2ED))
    {
        OUString aTest( pEdit->GetText() );
        const sal_Unicode* pTest = aTest.getStr();
        sal_Int32 nLen = aTest.getLength();
        OUStringBuffer aFilter( nLen );
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
    bool bEnable = mpPassword1ED->GetText().Len() >= mnMinLen;
    if( mpPassword2ED->IsVisible() )
        bEnable = (bEnable && (mpPassword2ED->GetText().Len() >= mnMinLen));
    mpOKBtn->Enable( bEnable );
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
        mpConfirm1ED->SetText( String() );
        mpConfirm1ED->GrabFocus();
    }
    else
        EndDialog( RET_OK );
    return 0;
}

// CTOR / DTOR -----------------------------------------------------------

SfxPasswordDialog::SfxPasswordDialog(Window* pParent, const String* pGroupText)
    : ModalDialog(pParent, "PasswordDialog", "sfx/ui/password.ui")
    , maMinLenPwdStr(SFX2_RESSTR(STR_PASSWD_MIN_LEN))
    , maEmptyPwdStr(SFX2_RESSTR(STR_PASSWD_EMPTY))
    , mnMinLen(5)
    , mnExtras(0)
    , mbAsciiOnly(false)
{
    get(mpPassword1Box, "password1frame");
    get(mpUserFT, "userft");
    get(mpUserED, "usered");
    get(mpPassword1FT, "pass1ft");
    get(mpPassword1ED, "pass1ed");
    get(mpConfirm1FT, "confirm1ft");
    get(mpConfirm1ED, "confirm1ed");

    get(mpPassword2Box, "password2frame");
    get(mpPassword2FT, "pass2ft");
    get(mpPassword2ED, "pass2ed");
    get(mpConfirm2FT, "confirm2ft");
    get(mpConfirm2ED, "confirm2ed");

    get(mpMinLengthFT, "minlenft");

    get(mpOKBtn, "ok");

    mpPassword1ED->SetAccessibleName(SFX2_RESSTR(STR_PASSWD));

    Link aLink = LINK( this, SfxPasswordDialog, EditModifyHdl );
    mpPassword1ED->SetModifyHdl( aLink );
    mpPassword2ED->SetModifyHdl( aLink );
    aLink = LINK( this, SfxPasswordDialog, OKHdl );
    mpOKBtn->SetClickHdl( aLink );

    if (pGroupText)
        mpPassword1Box->set_label(*pGroupText);

    //set the text to the pasword length
    SetPasswdText();
}

// -----------------------------------------------------------------------

void SfxPasswordDialog::SetPasswdText( )
{
//set the new string to the minimum password length
    if( mnMinLen == 0 )
        mpMinLengthFT->SetText(maEmptyPwdStr);
    else
    {
        maMainPwdStr = maMinLenPwdStr;
        maMainPwdStr.SearchAndReplace( OUString("$(MINLEN)"), String::CreateFromInt32((sal_Int32) mnMinLen ), 0);
        mpMinLengthFT->SetText(maMainPwdStr);
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
    mpUserFT->Hide();
    mpUserED->Hide();
    mpConfirm1FT->Hide();
    mpConfirm1ED->Hide();
    mpPassword1FT->Hide();
    mpPassword2Box->Hide();
    mpPassword2FT->Hide();
    mpPassword2ED->Hide();
    mpPassword2FT->Hide();
    mpConfirm2FT->Hide();
    mpConfirm2ED->Hide();

    if (mnExtras != SHOWEXTRAS_NONE)
        mpPassword1FT->Show();
    if (mnExtras & SHOWEXTRAS_USER)
    {
        mpUserFT->Show();
        mpUserED->Show();
    }
    if (mnExtras & SHOWEXTRAS_CONFIRM)
    {
        mpConfirm1FT->Show();
        mpConfirm1ED->Show();
    }
    if (mnExtras & SHOWEXTRAS_PASSWORD2)
    {
        mpPassword2Box->Show();
        mpPassword2FT->Show();
        mpPassword2ED->Show();
    }
    if (mnExtras & SHOWEXTRAS_CONFIRM2)
    {
        mpConfirm2FT->Show();
        mpConfirm2ED->Show();
    }

    return ModalDialog::Execute();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
