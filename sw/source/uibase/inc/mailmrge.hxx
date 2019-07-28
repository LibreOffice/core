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

#include <vcl/button.hxx>

#include <vcl/field.hxx>

#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>

#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>

#include <dbmgr.hxx>

class SwWrtShell;
class SwModuleOptions;
class SwXSelChgLstnr_Impl;
struct SwMailMergeDlg_Impl;
namespace com{namespace sun{namespace star{
    namespace frame{
        class XFrame2;
    }
    namespace sdbc{
        class XResultSet;
        class XConnection;
    }
}}}

class SwMailMergeDlg : public SfxModalDialog
{
    friend class SwXSelChgLstnr_Impl;

    VclPtr<vcl::Window>     m_pBeamerWin;

    VclPtr<RadioButton>     m_pAllRB;
    VclPtr<RadioButton>     m_pMarkedRB;
    VclPtr<RadioButton>     m_pFromRB;
    VclPtr<NumericField>    m_pFromNF;
    VclPtr<NumericField>    m_pToNF;

    VclPtr<RadioButton>     m_pPrinterRB;
    VclPtr<RadioButton>     m_pMailingRB;
    VclPtr<RadioButton>     m_pFileRB;

    VclPtr<CheckBox>        m_pSingleJobsCB;

    VclPtr<FixedText>       m_pSaveMergedDocumentFT;
    VclPtr<RadioButton>     m_pSaveSingleDocRB;
    VclPtr<RadioButton>     m_pSaveIndividualRB;

    VclPtr<CheckBox>        m_pGenerateFromDataBaseCB;

    VclPtr<FixedText>       m_pColumnFT;
    VclPtr<ListBox>         m_pColumnLB;
    VclPtr<FixedText>       m_pPathFT;
    VclPtr<Edit>            m_pPathED;
    VclPtr<PushButton>      m_pPathPB;
    VclPtr<FixedText>       m_pFilterFT;
    VclPtr<ListBox>         m_pFilterLB;

    VclPtr<ListBox>         m_pAddressFieldLB;
    VclPtr<FixedText>       m_pSubjectFT;
    VclPtr<Edit>            m_pSubjectED;
    VclPtr<FixedText>       m_pFormatFT;
    VclPtr<FixedText>       m_pAttachFT;
    VclPtr<Edit>            m_pAttachED;
    VclPtr<PushButton>      m_pAttachPB;
    VclPtr<CheckBox>        m_pFormatHtmlCB;
    VclPtr<CheckBox>        m_pFormatRtfCB;
    VclPtr<CheckBox>        m_pFormatSwCB;

    VclPtr<OKButton>        m_pOkBTN;

    std::unique_ptr<SwMailMergeDlg_Impl> pImpl;

    SwWrtShell&     rSh;
    SwModuleOptions* pModOpt;

    DBManagerOptions nMergeType;
    css::uno::Sequence< css::uno::Any >        m_aSelection;
    css::uno::Reference< css::frame::XFrame2 > m_xFrame;

    OUString m_sSaveFilter;
    OUString m_sFilename;

    DECL_LINK( ButtonHdl, Button*, void );
    DECL_LINK( InsertPathHdl, Button*, void );
    DECL_LINK( OutputTypeHdl, Button*, void );
    DECL_LINK( FilenameHdl, Button*, void );
    DECL_LINK( ModifyHdl, Edit&, void );
    DECL_LINK( SaveTypeHdl, Button*, void );

    bool            ExecQryShell();
    bool            AskUserFilename() const;
    OUString        GetURLfromPath() const;

public:
    SwMailMergeDlg(vcl::Window* pParent, SwWrtShell& rSh,
        const OUString& rSourceName,
        const OUString& rTableName,
        sal_Int32 nCommandType,
        const css::uno::Reference< css::sdbc::XConnection>& xConnection,
        css::uno::Sequence< css::uno::Any > const * pSelection);
    virtual ~SwMailMergeDlg() override;
    virtual void dispose() override;

    DBManagerOptions GetMergeType() { return nMergeType; }

    bool IsSaveSingleDoc() const { return m_pSaveSingleDocRB->IsChecked(); }
    bool IsGenerateFromDataBase() const { return m_pGenerateFromDataBaseCB->IsChecked(); }
    OUString GetColumnName() const { return m_pColumnLB->GetSelectedEntry(); }
    OUString GetTargetURL() const;

    const OUString& GetSaveFilter() const {return m_sSaveFilter;}
    const css::uno::Sequence< css::uno::Any >& GetSelection() const { return m_aSelection; }
    css::uno::Reference< css::sdbc::XResultSet> GetResultSet() const;

};

class SwMailMergeCreateFromDlg : public weld::GenericDialogController
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

class SwMailMergeFieldConnectionsDlg : public weld::GenericDialogController
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
