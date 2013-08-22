/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
    bool bEnable = mpPassword1ED->GetText().getLength() >= mnMinLen;
    if( mpPassword2ED->IsVisible() )
        bEnable = (bEnable && (mpPassword2ED->GetText().getLength() >= mnMinLen));
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
        mpConfirm1ED->SetText( OUString() );
        mpConfirm1ED->GrabFocus();
    }
    else
        EndDialog( RET_OK );
    return 0;
}

// CTOR / DTOR -----------------------------------------------------------

SfxPasswordDialog::SfxPasswordDialog(Window* pParent, const OUString* pGroupText)
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
        maMainPwdStr = maMainPwdStr.replaceAll( "$(MINLEN)", OUString::number((sal_Int32) mnMinLen ) );
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

void SfxPasswordDialog::ShowMinLengthText(bool bShow)
{
    mpMinLengthFT->Show(bShow);
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
