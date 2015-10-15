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
    VclPtr<FixedText> m_pOldFL;
    VclPtr<FixedText> m_pOldPasswdFT;
    VclPtr<Edit>      m_pOldPasswdED;
    VclPtr<Edit>      m_pNewPasswdED;
    VclPtr<Edit>      m_pRepeatPasswdED;
    VclPtr<OKButton>  m_pOKBtn;

    OUString          aOldPasswdErrStr;
    OUString          aRepeatPasswdErrStr;

    Link<SvxPasswordDialog*,bool> aCheckPasswordHdl;

    bool              bEmpty;

    DECL_LINK_TYPED(ButtonHdl, Button*, void);
    DECL_LINK_TYPED(EditModifyHdl, Edit&, void);

public:
                    SvxPasswordDialog( vcl::Window* pParent, bool bAllowEmptyPasswords = false, bool bDisableOldPassword = false );
    virtual         ~SvxPasswordDialog();
    virtual void    dispose() override;

    OUString        GetOldPassword() const { return m_pOldPasswdED->GetText(); }
    OUString        GetNewPassword() const { return m_pNewPasswdED->GetText(); }

    void            SetCheckPasswordHdl( const Link<SvxPasswordDialog*,bool>& rLink ) { aCheckPasswordHdl = rLink; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
