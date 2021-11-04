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

#include <config_options.h>
#include <sfx2/basedlgs.hxx>
#include <svx/svxdllapi.h>

class UNLESS_MERGELIBS(SVX_DLLPUBLIC) SvxPasswordDialog final : public SfxDialogController
{
private:
    OUString m_aOldPasswdErrStr;
    OUString m_aRepeatPasswdErrStr;
    Link<SvxPasswordDialog*,bool> m_aCheckPasswordHdl;

    std::unique_ptr<weld::Label> m_xOldFL;
    std::unique_ptr<weld::Label> m_xOldPasswdFT;
    std::unique_ptr<weld::Entry> m_xOldPasswdED;
    std::unique_ptr<weld::Entry> m_xNewPasswdED;
    std::unique_ptr<weld::Entry> m_xRepeatPasswdED;
    std::unique_ptr<weld::Button> m_xOKBtn;

    DECL_DLLPRIVATE_LINK(ButtonHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(EditModifyHdl, weld::Entry&, void);

public:
    SvxPasswordDialog(weld::Window* pParent, bool bDisableOldPassword);
    virtual ~SvxPasswordDialog() override;

    OUString        GetOldPassword() const { return m_xOldPasswdED->get_text(); }
    OUString        GetNewPassword() const { return m_xNewPasswdED->get_text(); }

    void            SetCheckPasswordHdl( const Link<SvxPasswordDialog*,bool>& rLink ) { m_aCheckPasswordHdl = rLink; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
