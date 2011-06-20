/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_UI_PROTECTION_DLG_HXX
#define SC_UI_PROTECTION_DLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <svx/checklbx.hxx>

class Window;
class ScTableProtection;

class ScTableProtectionDlg : public ModalDialog
{
public:
    explicit ScTableProtectionDlg(Window* pParent);
    virtual ~ScTableProtectionDlg();

    virtual short Execute();

    void SetDialogData(const ScTableProtection& rData);

    void WriteData(ScTableProtection& rData) const;

private:
    ScTableProtectionDlg(); // disabled

    void Init();

    void EnableOptionalWidgets(bool bEnable = true);

    CheckBox        maBtnProtect;

    FixedText       maPassword1Text;
    Edit            maPassword1Edit;
    FixedText       maPassword2Text;
    Edit            maPassword2Edit;

    FixedLine       maOptionsLine;
    FixedText       maOptionsText;
    SvxCheckListBox maOptionsListBox;

    OKButton        maBtnOk;
    CancelButton    maBtnCancel;
    HelpButton      maBtnHelp;

    String          maSelectLockedCells;
    String          maSelectUnlockedCells;

    DECL_LINK( OKHdl, OKButton* );
    DECL_LINK( CheckBoxHdl, CheckBox* );
    DECL_LINK( PasswordModifyHdl, Edit* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
