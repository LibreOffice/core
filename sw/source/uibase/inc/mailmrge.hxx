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

#include <svx/stddlg.hxx>

#include <vcl/button.hxx>

#include <vcl/field.hxx>

#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <svtools/stdctrl.hxx>

#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>

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

class SwMailMergeDlg : public SvxStandardDialog
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

    SwMailMergeDlg_Impl* pImpl;

    SwWrtShell&     rSh;
    SwModuleOptions* pModOpt;

    DBManagerOptions nMergeType;
    css::uno::Sequence< css::uno::Any >       m_aSelection;
    css::uno::Reference< css::frame::XFrame2 > m_xFrame;

    Size            m_aDialogSize;
    OUString m_sSaveFilter;

    DECL_LINK_TYPED( ButtonHdl, Button*, void );
    DECL_LINK_TYPED( InsertPathHdl, Button*, void );
    DECL_LINK_TYPED( OutputTypeHdl, Button*, void );
    DECL_LINK_TYPED( FilenameHdl, Button*, void );
    DECL_LINK_TYPED( ModifyHdl, Edit&, void );
    DECL_LINK_TYPED( SaveTypeHdl, Button*, void );

    virtual void    Apply() override;
    bool            ExecQryShell();

public:
    SwMailMergeDlg(vcl::Window* pParent, SwWrtShell& rSh,
        const OUString& rSourceName,
        const OUString& rTableName,
        sal_Int32 nCommandType,
        const css::uno::Reference< css::sdbc::XConnection>& xConnection,
        css::uno::Sequence< css::uno::Any >* pSelection = 0);
    virtual ~SwMailMergeDlg();
    virtual void dispose() override;

    inline DBManagerOptions GetMergeType() { return nMergeType; }

    bool IsSaveSingleDoc() const { return m_pSaveSingleDocRB->IsChecked(); }
    bool IsGenerateFromDataBase() const { return m_pGenerateFromDataBaseCB->IsChecked(); }
    OUString GetColumnName() const { return m_pColumnLB->GetSelectEntry();}
    OUString GetPath() const { return m_pPathED->GetText();}

    const OUString& GetSaveFilter() const {return m_sSaveFilter;}
    inline const css::uno::Sequence< css::uno::Any > GetSelection() const { return m_aSelection; }
    css::uno::Reference< css::sdbc::XResultSet> GetResultSet() const;

};

class SwMailMergeCreateFromDlg : public ModalDialog
{
    VclPtr<RadioButton> m_pThisDocRB;
public:
    SwMailMergeCreateFromDlg(vcl::Window* pParent);
    virtual ~SwMailMergeCreateFromDlg();
    virtual void dispose() override;
    bool IsThisDocument() const
    {
        return m_pThisDocRB->IsChecked();
    }
};

class SwMailMergeFieldConnectionsDlg : public ModalDialog
{
    VclPtr<RadioButton> m_pUseExistingRB;
public:
    SwMailMergeFieldConnectionsDlg(vcl::Window* pParent);
    virtual ~SwMailMergeFieldConnectionsDlg();
    virtual void dispose() override;

    bool IsUseExistingConnections() const
    {
        return m_pUseExistingRB->IsChecked();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
