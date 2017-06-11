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

#ifndef INCLUDED_UUI_SOURCE_MASTERPASSWORDDLG_HXX
#define INCLUDED_UUI_SOURCE_MASTERPASSWORDDLG_HXX

#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>


class MasterPasswordDialog : public ModalDialog
{
    VclPtr<Edit>     m_pEDMasterPassword;
    VclPtr<OKButton> m_pOKBtn;

    DECL_LINK(OKHdl_Impl, Button*, void);

public:
    MasterPasswordDialog(vcl::Window* pParent, css::task::PasswordRequestMode nDlgMode, const std::locale& rLocale);
    virtual ~MasterPasswordDialog() override;
    virtual void dispose() override;

    OUString        GetMasterPassword() const { return m_pEDMasterPassword->GetText(); }

private:
    css::task::PasswordRequestMode     nDialogMode;
    const std::locale&                 rResLocale;
};

#endif // INCLUDED_UUI_SOURCE_MASTERPASSWORDDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
