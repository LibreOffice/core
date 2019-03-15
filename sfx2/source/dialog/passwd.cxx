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

#include <sfx2/passwd.hxx>
#include <sfxtypes.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <vcl/weld.hxx>

IMPL_LINK_NOARG(SfxPasswordDialog, EditModifyHdl, weld::Entry&, void)
{
    ModifyHdl();
}

void SfxPasswordDialog::ModifyHdl()
{
    bool bEnable = m_xPassword1ED->get_text().getLength() >= mnMinLen;
    if (m_xPassword2ED->get_visible())
        bEnable = (bEnable && (m_xPassword2ED->get_text().getLength() >= mnMinLen));
    m_xOKBtn->set_sensitive(bEnable);
}

IMPL_LINK(SfxPasswordDialog, InsertTextHdl, OUString&, rTest, bool)
{
    if (!mbAsciiOnly)
        return true;

    const sal_Unicode* pTest = rTest.getStr();
    sal_Int32 nLen = rTest.getLength();
    OUStringBuffer aFilter(nLen);
    bool bReset = false;
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        if( *pTest > 0x007f )
            bReset = true;
        else
            aFilter.append(*pTest);
        ++pTest;
    }

    if (bReset)
        rTest = aFilter.makeStringAndClear();

    return true;
}

IMPL_LINK_NOARG(SfxPasswordDialog, OKHdl, weld::Button&, void)
{
    bool bConfirmFailed = bool( mnExtras & SfxShowExtras::CONFIRM ) &&
                          ( GetConfirm() != GetPassword() );
    if( ( mnExtras & SfxShowExtras::CONFIRM2 ) && ( m_xConfirm2ED->get_text() != GetPassword2() ) )
        bConfirmFailed = true;
    if ( bConfirmFailed )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                 VclMessageType::Warning, VclButtonsType::Ok,
                                                                 SfxResId(STR_ERROR_WRONG_CONFIRM)));
        xBox->run();
        m_xConfirm1ED->set_text(OUString());
        m_xConfirm1ED->grab_focus();
    }
    else
        m_xDialog->response(RET_OK);
}

// CTOR / DTOR -----------------------------------------------------------

SfxPasswordDialog::SfxPasswordDialog(weld::Widget* pParent, const OUString* pGroupText)
    : GenericDialogController(pParent, "sfx/ui/password.ui", "PasswordDialog")
    , m_xPassword1Box(m_xBuilder->weld_frame("password1frame"))
    , m_xUserFT(m_xBuilder->weld_label("userft"))
    , m_xUserED(m_xBuilder->weld_entry("usered"))
    , m_xPassword1FT(m_xBuilder->weld_label("pass1ft"))
    , m_xPassword1ED(m_xBuilder->weld_entry("pass1ed"))
    , m_xConfirm1FT(m_xBuilder->weld_label("confirm1ft"))
    , m_xConfirm1ED(m_xBuilder->weld_entry("confirm1ed"))
    , m_xPassword2Box(m_xBuilder->weld_frame("password2frame"))
    , m_xPassword2FT(m_xBuilder->weld_label("pass2ft"))
    , m_xPassword2ED(m_xBuilder->weld_entry("pass2ed"))
    , m_xConfirm2FT(m_xBuilder->weld_label("confirm2ft"))
    , m_xConfirm2ED(m_xBuilder->weld_entry("confirm2ed"))
    , m_xMinLengthFT(m_xBuilder->weld_label("minlenft"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
    , maMinLenPwdStr(SfxResId(STR_PASSWD_MIN_LEN))
    , maMinLenPwdStr1(SfxResId(STR_PASSWD_MIN_LEN1))
    , maEmptyPwdStr(SfxResId(STR_PASSWD_EMPTY))
    , mnMinLen(5)
    , mnExtras(SfxShowExtras::NONE)
    , mbAsciiOnly(false)
{
    Link<weld::Entry&,void> aLink = LINK(this, SfxPasswordDialog, EditModifyHdl);
    m_xPassword1ED->connect_changed(aLink);
    m_xPassword2ED->connect_changed(aLink);
    Link<OUString&,bool> aLink2 = LINK(this, SfxPasswordDialog, InsertTextHdl);
    m_xPassword1ED->connect_insert_text(aLink2);
    m_xPassword2ED->connect_insert_text(aLink2);
    m_xOKBtn->connect_clicked(LINK(this, SfxPasswordDialog, OKHdl));

    if (pGroupText)
        m_xPassword1Box->set_label(*pGroupText);

    //set the text to the password length
    SetPasswdText();
}

void SfxPasswordDialog::SetPasswdText( )
{
    //set the new string to the minimum password length
    if (mnMinLen == 0)
        m_xMinLengthFT->set_label(maEmptyPwdStr);
    else
    {
        if( mnMinLen == 1 )
            m_xMinLengthFT->set_label(maMinLenPwdStr1);
        else
        {
            maMainPwdStr = maMinLenPwdStr;
            maMainPwdStr = maMainPwdStr.replaceAll( "$(MINLEN)", OUString::number(static_cast<sal_Int32>(mnMinLen) ) );
            m_xMinLengthFT->set_label(maMainPwdStr);
        }
    }
}


void SfxPasswordDialog::SetMinLen( sal_uInt16 nLen )
{
    mnMinLen = nLen;
    SetPasswdText();
    ModifyHdl();
}

void SfxPasswordDialog::ShowMinLengthText(bool bShow)
{
    m_xMinLengthFT->set_visible(bShow);
}

short SfxPasswordDialog::run()
{
    m_xUserFT->hide();
    m_xUserED->hide();
    m_xConfirm1FT->hide();
    m_xConfirm1ED->hide();
    m_xPassword1FT->hide();
    m_xPassword2Box->hide();
    m_xPassword2FT->hide();
    m_xPassword2ED->hide();
    m_xPassword2FT->hide();
    m_xConfirm2FT->hide();
    m_xConfirm2ED->hide();

    if (mnExtras != SfxShowExtras::NONE)
        m_xPassword1FT->show();
    if (mnExtras & SfxShowExtras::USER)
    {
        m_xUserFT->show();
        m_xUserED->show();
    }
    if (mnExtras & SfxShowExtras::CONFIRM)
    {
        m_xConfirm1FT->show();
        m_xConfirm1ED->show();
    }
    if (mnExtras & SfxShowExtras::PASSWORD2)
    {
        m_xPassword2Box->show();
        m_xPassword2FT->show();
        m_xPassword2ED->show();
    }
    if (mnExtras & SfxShowExtras::CONFIRM2)
    {
        m_xConfirm2FT->show();
        m_xConfirm2ED->show();
    }

    return GenericDialogController::run();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
