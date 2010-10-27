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

#ifndef SC_UI_RETYPEPASS_DLG_HXX
#define SC_UI_RETYPEPASS_DLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
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

    virtual short Execute();

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

    String          maTextNotProtected;
    String          maTextNotPassProtected;
    String          maTextHashBad;
    String          maTextHashGood;
    String          maTextHashRegen;

    DECL_LINK( OKHdl, OKButton* );
    DECL_LINK( RetypeBtnHdl, PushButton* );
    DECL_LINK( ScrollHdl, ScrollBar* );

    struct TableItem
    {
        String              maName;
        TabProtectionPtr    mpProtect;
    };
    ::std::vector<TableItem> maTableItems;

    DocProtectionPtr    mpDocItem;
    size_t              mnCurScrollPos;
    ScPasswordHash      meDesiredHash;
};

// ============================================================================

class ScRetypePassInputDlg : public ModalDialog
{
public:
    explicit ScRetypePassInputDlg(Window* pParent, ScPassHashProtectable* pProtected);
    virtual ~ScRetypePassInputDlg();

    virtual short Execute();

    bool IsRemovePassword() const;
    String GetNewPassword() const;

private:
    ScRetypePassInputDlg(); // disabled

    void Init();
    void CheckPasswordInput();

private:
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;
    HelpButton      maBtnHelp;

    RadioButton     maBtnRetypePassword;

    FixedText       maPassword1Text;
    Edit            maPassword1Edit;
    FixedText       maPassword2Text;
    Edit            maPassword2Edit;

    CheckBox        maBtnMatchOldPass;

    RadioButton     maBtnRemovePassword;

    DECL_LINK( OKHdl, OKButton* );
    DECL_LINK( RadioBtnHdl, RadioButton* );
    DECL_LINK( CheckBoxHdl, CheckBox* );
    DECL_LINK( PasswordModifyHdl, Edit* );

    ScPassHashProtectable* mpProtected;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
