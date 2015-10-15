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

#ifndef INCLUDED_UUI_SOURCE_MASTERPASSCRTDLG_HXX
#define INCLUDED_UUI_SOURCE_MASTERPASSCRTDLG_HXX

#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>


class MasterPasswordCreateDialog : public ModalDialog
{
private:
    VclPtr<Edit>     m_pEDMasterPasswordCrt;
    VclPtr<Edit>     m_pEDMasterPasswordRepeat;
    VclPtr<OKButton> m_pOKBtn;


    DECL_LINK_TYPED(OKHdl_Impl, Button*, void);
    DECL_LINK_TYPED(EditHdl_Impl, Edit&, void);

public:
    MasterPasswordCreateDialog( vcl::Window* pParent, ResMgr * pResMgr );
    virtual ~MasterPasswordCreateDialog();
    virtual void dispose() override;

    OUString GetMasterPassword() const { return m_pEDMasterPasswordCrt->GetText(); }

private:
    ResMgr*                                         pResourceMgr;
    sal_uInt16                                      nMinLen;
};

#endif // INCLUDED_UUI_SOURCE_MASTERPASSCRTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
