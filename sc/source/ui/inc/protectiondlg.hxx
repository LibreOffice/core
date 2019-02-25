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

#ifndef INCLUDED_SC_SOURCE_UI_INC_PROTECTIONDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PROTECTIONDLG_HXX

#include <vcl/weld.hxx>
#include <scdllapi.h>

class ScTableProtection;

class ScTableProtectionDlg : public weld::GenericDialogController
{
public:
    ScTableProtectionDlg() = delete;
    explicit SC_DLLPUBLIC ScTableProtectionDlg(weld::Window* pParent);
    virtual ~ScTableProtectionDlg() override;

    void SetDialogData(const ScTableProtection& rData);

    void WriteData(ScTableProtection& rData) const;

private:
    void Init();

    void EnableOptionalWidgets(bool bEnable);

    OUString         m_aSelectLockedCells;
    OUString         m_aSelectUnlockedCells;
    OUString         m_aInsertColumns;
    OUString         m_aInsertRows;
    OUString         m_aDeleteColumns;
    OUString         m_aDeleteRows;

    std::unique_ptr<weld::CheckButton> m_xBtnProtect;
    std::unique_ptr<weld::Container> m_xPasswords;
    std::unique_ptr<weld::Container> m_xOptions;
    std::unique_ptr<weld::Entry> m_xPassword1Edit;
    std::unique_ptr<weld::Entry> m_xPassword2Edit;
    std::unique_ptr<weld::TreeView> m_xOptionsListBox;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Label> m_xProtected;
    std::unique_ptr<weld::Label> m_xUnprotected;
    std::unique_ptr<weld::Label> m_xInsertColumns;
    std::unique_ptr<weld::Label> m_xInsertRows;
    std::unique_ptr<weld::Label> m_xDeleteColumns;
    std::unique_ptr<weld::Label> m_xDeleteRows;

    void InsertEntry(const OUString& rTxt);

    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(CheckBoxHdl, weld::ToggleButton&, void);
    DECL_LINK(PasswordModifyHdl, weld::Entry&, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
