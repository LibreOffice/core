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
#include <vcl/layout.hxx>

#include <sfx2/passwd.hxx>
#include "sfxtypes.hxx"
#include <sfx2/sfxresid.hxx>

#include "sfx2/strings.hrc"


IMPL_LINK( SfxPasswordDialog, EditModifyHdl, Edit&, rEdit, void )
{
    ModifyHdl(&rEdit);
}

void SfxPasswordDialog::ModifyHdl(Edit* pEdit)
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
}

IMPL_LINK_NOARG(SfxPasswordDialog, OKHdl, Button*, void)
{
    bool bConfirmFailed = bool( mnExtras & SfxShowExtras::CONFIRM ) &&
                          ( GetConfirm() != GetPassword() );
    if( ( mnExtras & SfxShowExtras::CONFIRM2 ) && ( mpConfirm2ED->GetText() != GetPassword2() ) )
        bConfirmFailed = true;
    if ( bConfirmFailed )
    {
        ScopedVclPtrInstance< MessageDialog > aBox(this, SfxResId(STR_ERROR_WRONG_CONFIRM));
        aBox->Execute();
        mpConfirm1ED->SetText( OUString() );
        mpConfirm1ED->GrabFocus();
    }
    else
        EndDialog( RET_OK );
}

// CTOR / DTOR -----------------------------------------------------------

SfxPasswordDialog::SfxPasswordDialog(vcl::Window* pParent, const OUString* pGroupText)
    : ModalDialog(pParent, "PasswordDialog", "sfx/ui/password.ui")
    , maMinLenPwdStr(SfxResId(STR_PASSWD_MIN_LEN))
    , maMinLenPwdStr1(SfxResId(STR_PASSWD_MIN_LEN1))
    , maEmptyPwdStr(SfxResId(STR_PASSWD_EMPTY))
    , mnMinLen(5)
    , mnExtras(SfxShowExtras::NONE)
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

    Link<Edit&,void> aLink = LINK( this, SfxPasswordDialog, EditModifyHdl );
    mpPassword1ED->SetModifyHdl( aLink );
    mpPassword2ED->SetModifyHdl( aLink );
    mpOKBtn->SetClickHdl( LINK( this, SfxPasswordDialog, OKHdl ) );

    if (pGroupText)
        mpPassword1Box->set_label(*pGroupText);

    //set the text to the password length
    SetPasswdText();
}

SfxPasswordDialog::~SfxPasswordDialog()
{
    disposeOnce();
}

void SfxPasswordDialog::dispose()
{
    mpPassword1Box.clear();
    mpUserFT.clear();
    mpUserED.clear();
    mpPassword1FT.clear();
    mpPassword1ED.clear();
    mpConfirm1FT.clear();
    mpConfirm1ED.clear();
    mpPassword2Box.clear();
    mpPassword2FT.clear();
    mpPassword2ED.clear();
    mpConfirm2FT.clear();
    mpConfirm2ED.clear();
    mpMinLengthFT.clear();
    mpOKBtn.clear();
    ModalDialog::dispose();
}

void SfxPasswordDialog::SetPasswdText( )
{
//set the new string to the minimum password length
    if( mnMinLen == 0 )
        mpMinLengthFT->SetText(maEmptyPwdStr);
    else
    {
        if( mnMinLen == 1 )
            mpMinLengthFT->SetText(maMinLenPwdStr1);
        else
        {
            maMainPwdStr = maMinLenPwdStr;
            maMainPwdStr = maMainPwdStr.replaceAll( "$(MINLEN)", OUString::number((sal_Int32) mnMinLen ) );
            mpMinLengthFT->SetText(maMainPwdStr);
        }
    }
}


void SfxPasswordDialog::SetMinLen( sal_uInt16 nLen )
{
    mnMinLen = nLen;
    SetPasswdText();
    ModifyHdl( nullptr );
}

void SfxPasswordDialog::ShowMinLengthText(bool bShow)
{
    mpMinLengthFT->Show(bShow);
}


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

    if (mnExtras != SfxShowExtras::NONE)
        mpPassword1FT->Show();
    if (mnExtras & SfxShowExtras::USER)
    {
        mpUserFT->Show();
        mpUserED->Show();
    }
    if (mnExtras & SfxShowExtras::CONFIRM)
    {
        mpConfirm1FT->Show();
        mpConfirm1ED->Show();
    }
    if (mnExtras & SfxShowExtras::PASSWORD2)
    {
        mpPassword2Box->Show();
        mpPassword2FT->Show();
        mpPassword2ED->Show();
    }
    if (mnExtras & SfxShowExtras::CONFIRM2)
    {
        mpConfirm2FT->Show();
        mpConfirm2ED->Show();
    }

    return ModalDialog::Execute();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
