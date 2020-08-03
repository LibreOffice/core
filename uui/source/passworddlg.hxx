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

#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <vcl/weld.hxx>

class PasswordDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Label> m_xFTPassword;
    std::unique_ptr<weld::Entry> m_xEDPassword;
    std::unique_ptr<weld::Label> m_xFTConfirmPassword;
    std::unique_ptr<weld::Entry> m_xEDConfirmPassword;
    std::unique_ptr<weld::Button> m_xOKBtn;
    sal_uInt16      nMinLen;
    OUString        aPasswdMismatch;


    DECL_LINK(OKHdl_Impl, weld::Button&, void);

public:
    PasswordDialog(weld::Window* pParent, css::task::PasswordRequestMode nDlgMode, const std::locale& rLocale, const OUString& aDocURL,
                   bool bOpenToModify, bool bIsSimplePasswordRequest);

    void            SetMinLen( sal_uInt16 nMin ) { nMinLen = nMin; }
    OUString        GetPassword() const { return m_xEDPassword->get_text(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
