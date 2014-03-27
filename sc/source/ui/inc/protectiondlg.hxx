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

#ifndef SC_UI_PROTECTION_DLG_HXX
#define SC_UI_PROTECTION_DLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/layout.hxx>
#include <svx/checklbx.hxx>

class Window;
class ScTableProtection;

class ScTableProtectionDlg : public ModalDialog
{
public:
    explicit ScTableProtectionDlg(Window* pParent);
    virtual ~ScTableProtectionDlg();

    virtual short Execute() SAL_OVERRIDE;

    void SetDialogData(const ScTableProtection& rData);

    void WriteData(ScTableProtection& rData) const;

private:
    ScTableProtectionDlg(); // disabled

    void Init();

    void EnableOptionalWidgets(bool bEnable = true);

    CheckBox*        m_pBtnProtect;

    VclContainer*    m_pPasswords;
    VclContainer*    m_pOptions;
    Edit*            m_pPassword1Edit;
    Edit*            m_pPassword2Edit;

    SvxCheckListBox* m_pOptionsListBox;

    OKButton*        m_pBtnOk;

    OUString         m_aSelectLockedCells;
    OUString         m_aSelectUnlockedCells;

    DECL_LINK( OKHdl, void* );
    DECL_LINK( CheckBoxHdl, CheckBox* );
    DECL_LINK( PasswordModifyHdl, void* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
