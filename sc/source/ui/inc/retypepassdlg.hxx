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

#ifndef INCLUDED_SC_SOURCE_UI_INC_RETYPEPASSDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_RETYPEPASSDLG_HXX

#include <vcl/weld.hxx>
#include <tabprotection.hxx>
#include <memory>

class ScDocument;

struct PassFragment
{
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xSheetsBox;
    std::unique_ptr<weld::Label> m_xName;
    std::unique_ptr<weld::Label> m_xStatus;
    std::unique_ptr<weld::Button> m_xButton;

    PassFragment(weld::Widget* pParent);
};

class ScRetypePassDlg : public weld::GenericDialogController
{
public:
    typedef std::shared_ptr<ScDocProtection>    DocProtectionPtr;
    typedef std::shared_ptr<ScTableProtection>  TabProtectionPtr;

    explicit ScRetypePassDlg(weld::Window* pParent);
    virtual ~ScRetypePassDlg() override;

    virtual short run() override;

    void SetDataFromDocument(const ScDocument& rDoc);
    void SetDesiredHash(ScPasswordHash eHash);

    /** Write the new set of password data to the document instance to
        overwrite the current ones. */
    void WriteNewDataToDocument(ScDocument& rDoc) const;

private:
    void Init();
    void PopulateDialog();
    void SetDocData();
    void SetTableData(size_t nRowPos, SCTAB nTab);

    /** Check the status of all hash values to see if it's okay to enable
        the OK button. */
    void CheckHashStatus();

    void DeleteSheets();

private:
    OUString const        maTextNotProtected;
    OUString const        maTextNotPassProtected;
    OUString const        maTextHashBad;
    OUString const        maTextHashGood;

    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(RetypeBtnHdl, weld::Button&, void);

    struct TableItem
    {
        OUString       maName;
        TabProtectionPtr    mpProtect;
    };
    ::std::vector<TableItem> maTableItems;

    DocProtectionPtr    mpDocItem;
    ScPasswordHash      meDesiredHash;

    std::unique_ptr<weld::Button> mxBtnOk;
    std::unique_ptr<weld::Label> mxTextDocStatus;
    std::unique_ptr<weld::Button> mxBtnRetypeDoc;
    std::unique_ptr<weld::ScrolledWindow> mxScrolledWindow;
    std::unique_ptr<weld::Container> mxSheetsBox;
    std::vector<std::unique_ptr<PassFragment>> maSheets;
};

class ScRetypePassInputDlg : public weld::GenericDialogController
{
public:
    ScRetypePassInputDlg() = delete;
    explicit ScRetypePassInputDlg(weld::Window* pParent, ScPassHashProtectable* pProtected);
    virtual ~ScRetypePassInputDlg() override;

    bool IsRemovePassword() const;
    OUString GetNewPassword() const;

private:
    void Init();
    void CheckPasswordInput();

private:
    ScPassHashProtectable* const m_pProtected;

    std::unique_ptr<weld::Button> m_xBtnOk;

    std::unique_ptr<weld::RadioButton> m_xBtnRetypePassword;

    std::unique_ptr<weld::Widget> m_xPasswordGrid;
    std::unique_ptr<weld::Entry> m_xPassword1Edit;
    std::unique_ptr<weld::Entry> m_xPassword2Edit;

    std::unique_ptr<weld::CheckButton> m_xBtnMatchOldPass;

    std::unique_ptr<weld::RadioButton> m_xBtnRemovePassword;

    DECL_LINK( OKHdl, weld::Button&, void );
    DECL_LINK( RadioBtnHdl, weld::ToggleButton&, void );
    DECL_LINK( CheckBoxHdl, weld::ToggleButton&, void );
    DECL_LINK( PasswordModifyHdl, weld::Entry&, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
