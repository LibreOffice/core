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

#include <sfx2/basedlgs.hxx>
#include <memory>

class PasswordToOpenModifyDialog : public SfxDialogController
{
    std::unique_ptr<weld::Entry> m_xPasswdToOpenED;
    std::unique_ptr<weld::Label> m_xPasswdToOpenInd;
    std::unique_ptr<weld::Entry> m_xReenterPasswdToOpenED;
    std::unique_ptr<weld::Label> m_xReenterPasswdToOpenInd;
    std::unique_ptr<weld::Expander> m_xOptionsExpander;
    std::unique_ptr<weld::Button> m_xOk;
    std::unique_ptr<weld::CheckButton> m_xOpenReadonlyCB;
    std::unique_ptr<weld::Label> m_xPasswdToModifyFT;
    std::unique_ptr<weld::Entry> m_xPasswdToModifyED;
    std::unique_ptr<weld::Label> m_xPasswdToModifyInd;
    std::unique_ptr<weld::Label> m_xReenterPasswdToModifyFT;
    std::unique_ptr<weld::Entry> m_xReenterPasswdToModifyED;
    std::unique_ptr<weld::Label> m_xReenterPasswdToModifyInd;

    OUString                    m_aOneMismatch;
    OUString                    m_aTwoMismatch;
    OUString                    m_aInvalidStateForOkButton;
    OUString                    m_aInvalidStateForOkButton_v2;

    int                         m_nMaxPasswdLen;
    bool                        m_bIsPasswordToModify;


    DECL_LINK(OkBtnClickHdl, weld::Button&, void);
    DECL_LINK(ReadonlyOnOffHdl, weld::Button&, void);
    DECL_LINK(ChangeHdl, weld::Entry&, void);

    PasswordToOpenModifyDialog( const PasswordToOpenModifyDialog & ) = delete;
    PasswordToOpenModifyDialog & operator = ( const PasswordToOpenModifyDialog & ) = delete;

public:
    PasswordToOpenModifyDialog(weld::Window* pParent,
            sal_uInt16 nMaxPasswdLen /* 0 -> no max len enforced */,
            bool bIsPasswordToModify );

    // AbstractPasswordToOpenModifyDialog
    OUString  GetPasswordToOpen() const;
    OUString  GetPasswordToModify() const;
    bool    IsRecommendToOpenReadonly() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
