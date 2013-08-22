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
#ifndef _SFX_PASSWD_HXX
#define _SFX_PASSWD_HXX

#include "sal/config.h"
#include <sfx2/dllapi.h>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <sfx2/app.hxx>

// defines ---------------------------------------------------------------

#define SHOWEXTRAS_NONE      ((sal_uInt16)0x0000)
#define SHOWEXTRAS_USER      ((sal_uInt16)0x0001)
#define SHOWEXTRAS_CONFIRM   ((sal_uInt16)0x0002)
#define SHOWEXTRAS_PASSWORD2 ((sal_uInt16)0x0004)
#define SHOWEXTRAS_CONFIRM2  ((sal_uInt16)0x0008)
#define SHOWEXTRAS_ALL       ((sal_uInt16)(SHOWEXTRAS_USER | SHOWEXTRAS_CONFIRM))

// class SfxPasswordDialog -----------------------------------------------

class SFX2_DLLPUBLIC SfxPasswordDialog : public ModalDialog
{
private:
    VclFrame*       mpPassword1Box;
    FixedText*      mpUserFT;
    Edit*           mpUserED;
    FixedText*      mpPassword1FT;
    Edit*           mpPassword1ED;
    FixedText*      mpConfirm1FT;
    Edit*           mpConfirm1ED;

    VclFrame*       mpPassword2Box;
    FixedText*      mpPassword2FT;
    Edit*           mpPassword2ED;
    FixedText*      mpConfirm2FT;
    Edit*           mpConfirm2ED;

    FixedText*      mpMinLengthFT;

    OKButton*       mpOKBtn;

    OUString        maMinLenPwdStr;
    OUString        maEmptyPwdStr;
    OUString        maMainPwdStr;
    sal_uInt16      mnMinLen;
    sal_uInt16      mnExtras;

    bool            mbAsciiOnly;
    DECL_DLLPRIVATE_LINK(EditModifyHdl, Edit*);
    DECL_DLLPRIVATE_LINK(OKHdl, void *);

    void            SetPasswdText();

public:
    SfxPasswordDialog(Window* pParent, const OUString* pGroupText = NULL);

    OUString GetUser() const
    {
        return mpUserED->GetText();
    }
    OUString GetPassword() const
    {
        return mpPassword1ED->GetText();
    }
    OUString GetConfirm() const
    {
        return mpConfirm1ED->GetText();
    }
    OUString GetPassword2() const
    {
        return mpPassword2ED->GetText();
    }
    OUString GetConfirm2() const
    {
        return mpConfirm2ED->GetText();
    }
    void SetGroup2Text(const OUString& i_rText)
    {
        mpPassword2Box->set_label(i_rText);
    }
    void SetMinLen(sal_uInt16 Len);
    void SetEditHelpId(const OString& rId)
    {
        mpPassword1ED->SetHelpId( rId );
    }
    void ShowExtras(sal_uInt16 nExtras)
    {
        mnExtras = nExtras;
    }
    void AllowAsciiOnly(bool i_bAsciiOnly = true)
    {
        mbAsciiOnly = i_bAsciiOnly;
    }

    void ShowMinLengthText(bool bShow);

    virtual short Execute();
};

#endif // #ifndef _SFX_PASSWD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
