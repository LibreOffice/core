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
#ifndef INCLUDED_SVX_PASSWD_HXX
#define INCLUDED_SVX_PASSWD_HXX

#include <sfx2/basedlgs.hxx>

#include <vcl/fixed.hxx>

#include <vcl/edit.hxx>

#include <vcl/button.hxx>
#include <svx/svxdllapi.h>

// class SvxPasswordDialog -----------------------------------------------

class SVX_DLLPUBLIC SvxPasswordDialog : public SfxModalDialog
{
private:
    FixedText* m_pOldFL;
    FixedText* m_pOldPasswdFT;
    Edit* m_pOldPasswdED;
    Edit* m_pNewPasswdED;
    Edit* m_pRepeatPasswdED;
    OKButton* m_pOKBtn;

    OUString        aOldPasswdErrStr;
    OUString        aRepeatPasswdErrStr;

    Link            aCheckPasswordHdl;

    bool            bEmpty;

    DECL_LINK(ButtonHdl, void *);
    DECL_LINK(EditModifyHdl, void *);

public:
                    SvxPasswordDialog( Window* pParent, bool bAllowEmptyPasswords = false, bool bDisableOldPassword = false );
                    virtual ~SvxPasswordDialog();

    OUString        GetOldPassword() const { return m_pOldPasswdED->GetText(); }
    OUString        GetNewPassword() const { return m_pNewPasswdED->GetText(); }

    void            SetCheckPasswordHdl( const Link& rLink ) { aCheckPasswordHdl = rLink; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
