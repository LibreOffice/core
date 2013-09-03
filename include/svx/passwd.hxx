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
#ifndef _SVX_PASSWD_HXX
#define _SVX_PASSWD_HXX

#include <sfx2/basedlgs.hxx>

#include <vcl/fixed.hxx>

#include <vcl/edit.hxx>

#include <vcl/button.hxx>
#include "svx/svxdllapi.h"

// class SvxPasswordDialog -----------------------------------------------

class SVX_DLLPUBLIC SvxPasswordDialog : public SfxModalDialog
{
private:
    FixedLine       aOldFL;
    FixedText       aOldPasswdFT;
    Edit            aOldPasswdED;
    FixedLine       aNewFL;
    FixedText       aNewPasswdFT;
    Edit            aNewPasswdED;
    FixedText       aRepeatPasswdFT;
    Edit            aRepeatPasswdED;
    OKButton        aOKBtn;
    CancelButton    aEscBtn;
    HelpButton      aHelpBtn;

    OUString        aOldPasswdErrStr;
    OUString        aRepeatPasswdErrStr;

    Link            aCheckPasswordHdl;

    sal_Bool            bEmpty;

    DECL_LINK(ButtonHdl, void *);
    DECL_LINK(EditModifyHdl, void *);

public:
                    SvxPasswordDialog( Window* pParent, sal_Bool bAllowEmptyPasswords = sal_False, sal_Bool bDisableOldPassword = sal_False );
                    ~SvxPasswordDialog();

    OUString        GetOldPassword() const { return aOldPasswdED.GetText(); }
    OUString        GetNewPassword() const { return aNewPasswdED.GetText(); }

    void            SetCheckPasswordHdl( const Link& rLink ) { aCheckPasswordHdl = rLink; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
