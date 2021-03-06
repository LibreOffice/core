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

#include <types.hxx>
#include <vcl/weld.hxx>

class ScDocument;

class ScMoveTableDlg : public weld::GenericDialogController
{
public:
    ScMoveTableDlg(weld::Window* pParent, const OUString& rDefault);
    virtual ~ScMoveTableDlg() override;

    sal_uInt16  GetSelectedDocument     () const { return nDocument; }
    SCTAB   GetSelectedTable        () const { return nTable; }
    bool    GetCopyTable            () const { return bCopyTable; }
    bool    GetRenameTable          () const { return bRenameTable; }
    void    GetTabNameString( OUString& rString ) const;
    void    SetForceCopyTable       ();
    void    EnableRenameTable       (bool bFlag);

private:
    void ResetRenameInput();
    void CheckNewTabName();
    ScDocument* GetSelectedDoc();

private:
    OUString   msCurrentDoc;
    OUString   msNewDoc;

    OUString   msStrTabNameUsed;
    OUString   msStrTabNameEmpty;
    OUString   msStrTabNameInvalid;

    const OUString maDefaultName;

    sal_uInt16      mnCurrentDocPos;
    sal_uInt16      nDocument;
    SCTAB           nTable;
    bool            bCopyTable:1;
    bool            bRenameTable:1;
    bool            mbEverEdited:1;

    std::unique_ptr<weld::RadioButton> m_xBtnMove;
    std::unique_ptr<weld::RadioButton> m_xBtnCopy;
    std::unique_ptr<weld::Label> m_xFtDoc;
    std::unique_ptr<weld::ComboBox> m_xLbDoc;
    std::unique_ptr<weld::TreeView> m_xLbTable;
    std::unique_ptr<weld::Entry> m_xEdTabName;
    std::unique_ptr<weld::Label> m_xFtWarn;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<weld::Label> m_xUnusedLabel;
    std::unique_ptr<weld::Label> m_xEmptyLabel;
    std::unique_ptr<weld::Label> m_xInvalidLabel;

    void    Init            ();
    void    InitDocListBox  ();
    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(SelHdl, weld::ComboBox&, void);
    DECL_LINK(CheckBtnHdl, weld::ToggleButton&, void);
    DECL_LINK(CheckNameHdl, weld::Entry&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
