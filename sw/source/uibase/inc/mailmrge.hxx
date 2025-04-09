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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_MAILMRGE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_MAILMRGE_HXX

#include <memory>
#include <sfx2/basedlgs.hxx>

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>

#include <dbmgr.hxx>

class SwWrtShell;
class SwModuleOptions;
struct SwMailMergeDlg_Impl;
namespace com::sun::star{
    namespace frame{
        class XFrame2;
    }
    namespace sdbc{
        class XResultSet;
        class XConnection;
    }
}

class SwMailMergeDlg final : public SfxDialogController
{
    friend class SwXSelChgLstnr_Impl;

    std::unique_ptr<SwMailMergeDlg_Impl> m_pImpl;

    SwWrtShell&     m_rSh;
    SwModuleOptions* m_pModOpt;

    DBManagerOptions m_nMergeType;
    css::uno::Sequence< css::uno::Any >        m_aSelection;
    css::uno::Reference< css::frame::XFrame2 > m_xFrame;

    OUString m_sSaveFilter;
    OUString m_sFilename;

    std::unique_ptr<weld::Container> m_xBeamerWin;

    std::unique_ptr<weld::RadioButton> m_xAllRB;
    std::unique_ptr<weld::RadioButton> m_xMarkedRB;
    std::unique_ptr<weld::RadioButton> m_xFromRB;
    std::unique_ptr<weld::SpinButton> m_xFromNF;
    std::unique_ptr<weld::SpinButton> m_xToNF;

    std::unique_ptr<weld::RadioButton> m_xPrinterRB;
    std::unique_ptr<weld::RadioButton> m_xMailingRB;
    std::unique_ptr<weld::RadioButton> m_xFileRB;

    std::unique_ptr<weld::CheckButton> m_xPasswordCB;

    std::unique_ptr<weld::Label> m_xSaveMergedDocumentFT;
    std::unique_ptr<weld::RadioButton> m_xSaveSingleDocRB;
    std::unique_ptr<weld::RadioButton> m_xSaveIndividualRB;

    std::unique_ptr<weld::CheckButton> m_xGenerateFromDataBaseCB;

    std::unique_ptr<weld::Label> m_xColumnFT;
    std::unique_ptr<weld::ComboBox> m_xColumnLB;
    std::unique_ptr<weld::Label> m_xPasswordFT;
    std::unique_ptr<weld::ComboBox> m_xPasswordLB;
    std::unique_ptr<weld::Label> m_xPathFT;
    std::unique_ptr<weld::Entry> m_xPathED;
    std::unique_ptr<weld::Button> m_xPathPB;
    std::unique_ptr<weld::Label> m_xFilterFT;
    std::unique_ptr<weld::ComboBox> m_xFilterLB;

    std::unique_ptr<weld::ComboBox> m_xAddressFieldLB;
    std::unique_ptr<weld::Label> m_xSubjectFT;
    std::unique_ptr<weld::Entry> m_xSubjectED;
    std::unique_ptr<weld::Label> m_xFormatFT;
    std::unique_ptr<weld::Label> m_xAttachFT;
    std::unique_ptr<weld::Entry> m_xAttachED;
    std::unique_ptr<weld::Button> m_xAttachPB;
    std::unique_ptr<weld::CheckButton> m_xFormatHtmlCB;
    std::unique_ptr<weld::CheckButton> m_xFormatRtfCB;
    std::unique_ptr<weld::CheckButton> m_xFormatSwCB;

    std::unique_ptr<weld::Button> m_xOkBTN;

    DECL_LINK( ButtonHdl, weld::Button&, void );
    DECL_LINK( InsertPathHdl, weld::Button&, void );
    DECL_LINK( OutputTypeHdl, weld::Toggleable&, void );
    DECL_LINK( FilenameHdl, weld::Toggleable&, void );
    DECL_LINK( ModifyHdl, weld::SpinButton&, void );
    DECL_LINK( SaveTypeHdl, weld::Toggleable&, void );
    DECL_LINK( FileFormatHdl, weld::ComboBox&, void );

    bool            ExecQryShell();
    bool            AskUserFilename() const;
    OUString        GetURLfromPath() const;

public:
    SwMailMergeDlg(weld::Window* pParent, SwWrtShell& rSh,
        const OUString& rSourceName,
        const OUString& rTableName,
        sal_Int32 nCommandType,
        const css::uno::Reference< css::sdbc::XConnection>& xConnection,
        css::uno::Sequence< css::uno::Any > const * pSelection);
    virtual ~SwMailMergeDlg() override;

    DBManagerOptions GetMergeType() const { return m_nMergeType; }

    bool IsSaveSingleDoc() const { return m_xSaveSingleDocRB->get_active(); }
    bool IsGenerateFromDataBase() const { return m_xGenerateFromDataBaseCB->get_active(); }
    bool IsFileEncryptedFromDataBase() const { return m_xPasswordCB->get_active(); }
    OUString GetColumnName() const { return m_xColumnLB->get_active_text(); }
    OUString GetPasswordColumnName() const { return m_xPasswordLB->get_active_text(); }
    OUString GetTargetURL() const;

    const OUString& GetSaveFilter() const {return m_sSaveFilter;}
    const css::uno::Sequence< css::uno::Any >& GetSelection() const { return m_aSelection; }
    css::uno::Reference< css::sdbc::XResultSet> GetResultSet() const;

};

class SwMailMergeCreateFromDlg final : public weld::GenericDialogController
{
    std::unique_ptr<weld::RadioButton> m_xThisDocRB;
public:
    SwMailMergeCreateFromDlg(weld::Window* pParent);
    virtual ~SwMailMergeCreateFromDlg() override;
    bool IsThisDocument() const
    {
        return m_xThisDocRB->get_active();
    }
};

class SwMailMergeFieldConnectionsDlg final : public weld::GenericDialogController
{
    std::unique_ptr<weld::RadioButton> m_xUseExistingRB;
public:
    SwMailMergeFieldConnectionsDlg(weld::Window* pParent);
    virtual ~SwMailMergeFieldConnectionsDlg() override;

    bool IsUseExistingConnections() const
    {
        return m_xUseExistingRB->get_active();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
