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

#ifndef SC_UI_RETYPEPASS_DLG_HXX
#define SC_UI_RETYPEPASS_DLG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/scrbar.hxx>
#include <svx/checklbx.hxx>
#include <svtools/stdctrl.hxx>

#include "tabprotection.hxx"

#include <boost/shared_ptr.hpp>

class Window;
class ScDocProtection;
class ScTableProtection;
class ScDocument;

class ScRetypePassDlg : public ModalDialog
{
public:
    typedef ::boost::shared_ptr<ScDocProtection>    DocProtectionPtr;
    typedef ::boost::shared_ptr<ScTableProtection>  TabProtectionPtr;

    explicit ScRetypePassDlg(Window* pParent);
    virtual ~ScRetypePassDlg();

    virtual short Execute() SAL_OVERRIDE;

    void SetDataFromDocument(const ScDocument& rDoc);
    void SetDesiredHash(ScPasswordHash eHash);

    /** Write the new set of password data to the document instance to
        overwrite the current ones. */
    void WriteNewDataToDocument(ScDocument& rDoc) const;

private:
    ScRetypePassDlg(); // disabled

    void Init();
    void PopulateDialog();
    void SetDocData();
    void SetTableData(size_t nRowPos, SCTAB nTab);
    void ResetTableRows();

    /** Check the status of all hash values to see if it's okay to enable
        the OK button. */
    void CheckHashStatus();

private:
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;
    HelpButton      maBtnHelp;

    FixedInfo       maTextDescription;

    FixedLine       maLineDocument;
    FixedText       maTextDocStatus;
    PushButton      maBtnRetypeDoc;

    FixedLine       maLineSheet;
    FixedText       maTextSheetName1;
    FixedText       maTextSheetStatus1;
    PushButton      maBtnRetypeSheet1;

    FixedText       maTextSheetName2;
    FixedText       maTextSheetStatus2;
    PushButton      maBtnRetypeSheet2;

    FixedText       maTextSheetName3;
    FixedText       maTextSheetStatus3;
    PushButton      maBtnRetypeSheet3;

    FixedText       maTextSheetName4;
    FixedText       maTextSheetStatus4;
    PushButton      maBtnRetypeSheet4;

    ScrollBar       maScrollBar;

    OUString        maTextNotProtected;
    OUString        maTextNotPassProtected;
    OUString        maTextHashBad;
    OUString        maTextHashGood;
    OUString        maTextHashRegen;

    DECL_LINK( OKHdl, void* );
    DECL_LINK( RetypeBtnHdl, PushButton* );
    DECL_LINK( ScrollHdl, void* );

    struct TableItem
    {
        OUString       maName;
        TabProtectionPtr    mpProtect;
    };
    ::std::vector<TableItem> maTableItems;

    DocProtectionPtr    mpDocItem;
    size_t              mnCurScrollPos;
    ScPasswordHash      meDesiredHash;
};



class ScRetypePassInputDlg : public ModalDialog
{
public:
    explicit ScRetypePassInputDlg(Window* pParent, ScPassHashProtectable* pProtected);
    virtual ~ScRetypePassInputDlg();

    virtual short Execute() SAL_OVERRIDE;

    bool IsRemovePassword() const;
    OUString GetNewPassword() const;

private:
    ScRetypePassInputDlg(); // disabled

    void Init();
    void CheckPasswordInput();

private:
    OKButton*       m_pBtnOk;

    RadioButton*    m_pBtnRetypePassword;

    VclContainer*   m_pPasswordGrid;
    Edit*           m_pPassword1Edit;
    Edit*           m_pPassword2Edit;

    CheckBox*       m_pBtnMatchOldPass;

    RadioButton*    m_pBtnRemovePassword;

    DECL_LINK( OKHdl, void* );
    DECL_LINK( RadioBtnHdl, RadioButton* );
    DECL_LINK( CheckBoxHdl, void* );
    DECL_LINK( PasswordModifyHdl, void* );

    ScPassHashProtectable* mpProtected;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
