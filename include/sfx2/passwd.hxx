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
#pragma once

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <vcl/weld.hxx>
#include <o3tl/typed_flags_set.hxx>

// defines ---------------------------------------------------------------

enum class SfxShowExtras
{
    NONE      = 0x0000,
    USER      = 0x0001,
    CONFIRM   = 0x0002,
    PASSWORD2 = 0x0004,
    CONFIRM2  = 0x0008,
    ALL       = USER | CONFIRM
};
namespace o3tl
{
    template<> struct typed_flags<SfxShowExtras> : is_typed_flags<SfxShowExtras, 0x0f> {};
}

// class SfxPasswordDialog -----------------------------------------------

class SFX2_DLLPUBLIC SfxPasswordDialog final : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Frame> m_xPassword1Box;
    std::unique_ptr<weld::Label> m_xUserFT;
    std::unique_ptr<weld::Entry> m_xUserED;
    std::unique_ptr<weld::Label> m_xPassword1FT;
    std::unique_ptr<weld::Entry> m_xPassword1ED;
    std::unique_ptr<weld::LevelBar> m_xPassword1StrengthBar;
    std::unique_ptr<weld::Label> m_xPassword1PolicyLabel;
    std::unique_ptr<weld::Label> m_xConfirm1FT;
    std::unique_ptr<weld::Entry> m_xConfirm1ED;

    std::unique_ptr<weld::Frame> m_xPassword2Box;
    std::unique_ptr<weld::Label> m_xPassword2FT;
    std::unique_ptr<weld::Entry> m_xPassword2ED;
    std::unique_ptr<weld::LevelBar> m_xPassword2StrengthBar;
    std::unique_ptr<weld::Label> m_xPassword2PolicyLabel;
    std::unique_ptr<weld::Label> m_xConfirm2FT;
    std::unique_ptr<weld::Entry> m_xConfirm2ED;

    std::unique_ptr<weld::Label> m_xMinLengthFT;
    std::unique_ptr<weld::Label> m_xOnlyAsciiFT;

    std::unique_ptr<weld::Button> m_xOKBtn;

    std::array<std::unique_ptr<weld::ToggleButton>, 4> m_xPass;

    std::shared_ptr<weld::MessageDialog> m_xConfirmFailedDialog;

    OUString        maMinLenPwdStr;
    OUString        maMinLenPwdStr1;
    OUString        maEmptyPwdStr;
    OUString        maMainPwdStr;
    sal_uInt16      mnMinLen;
    SfxShowExtras  mnExtras;
    std::optional<OUString> moPasswordPolicy;

    bool            mbAsciiOnly;
    DECL_DLLPRIVATE_LINK(OKHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(ShowHdl, weld::Toggleable&, void);
    DECL_DLLPRIVATE_LINK(InsertTextHdl, OUString&, bool);
    DECL_DLLPRIVATE_LINK(EditModifyHdl, weld::Entry&, void);
    void            ModifyHdl();

    void            SetPasswdText();

public:
    SfxPasswordDialog(weld::Widget* pParent, const OUString* pGroupText = nullptr);

    OUString GetUser() const
    {
        return m_xUserED->get_text();
    }
    OUString GetPassword() const
    {
        return m_xPassword1ED->get_text();
    }
    OUString GetConfirm() const
    {
        return m_xConfirm1ED->get_text();
    }
    OUString GetPassword2() const
    {
        return m_xPassword2ED->get_text();
    }
    void SetGroup2Text(const OUString& i_rText)
    {
        m_xPassword2Box->set_label(i_rText);
    }
    void SetMinLen(sal_uInt16 Len);
    void SetEditHelpId(const OUString& rId)
    {
        m_xPassword1ED->set_help_id(rId);
    }
    /* tdf#60874 we need a custom help ID for the Confirm
       field of the Protect Document window */
    void SetConfirmHelpId(const OUString& rId)
    {
        m_xConfirm1ED->set_help_id(rId);
    }

    void ShowExtras(SfxShowExtras nExtras)
    {
        mnExtras = nExtras;
    }

    void AllowAsciiOnly();
    void ShowMinLengthText(bool bShow);

    void PreRun();

    virtual short run() override;

    ~SfxPasswordDialog();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
