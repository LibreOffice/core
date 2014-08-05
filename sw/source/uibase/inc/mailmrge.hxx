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

    Window*         m_pBeamerWin;

    RadioButton*     m_pAllRB;
    RadioButton*     m_pMarkedRB;
    RadioButton*     m_pFromRB;
    NumericField*    m_pFromNF;
    NumericField*    m_pToNF;

    RadioButton*     m_pPrinterRB;
    RadioButton*     m_pMailingRB;
    RadioButton*     m_pFileRB;

    CheckBox*        m_pSingleJobsCB;

    FixedText*       m_pSaveMergedDocumentFT;
    RadioButton*     m_pSaveSingleDocRB;
    RadioButton*     m_pSaveIndividualRB;

    CheckBox*        m_pGenerateFromDataBaseCB;

    FixedText*       m_pColumnFT;
    ListBox*         m_pColumnLB;
    FixedText*       m_pPathFT;
    Edit*            m_pPathED;
    PushButton*      m_pPathPB;
    FixedText*       m_pFilterFT;
    ListBox*         m_pFilterLB;

    ListBox*         m_pAddressFldLB;
    FixedText*       m_pSubjectFT;
    Edit*            m_pSubjectED;
    FixedText*       m_pFormatFT;
    FixedText*       m_pAttachFT;
    Edit*            m_pAttachED;
    PushButton*      m_pAttachPB;
    CheckBox*        m_pFormatHtmlCB;
    CheckBox*        m_pFormatRtfCB;
    CheckBox*        m_pFormatSwCB;

    OKButton*        m_pOkBTN;

    SwMailMergeDlg_Impl* pImpl;

    SwWrtShell&     rSh;
    SwModuleOptions* pModOpt;

    sal_uInt16          nMergeType;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >       m_aSelection;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame2 > m_xFrame;

    Size            m_aDialogSize;
    OUString m_sSaveFilter;

    DECL_LINK( ButtonHdl, Button* pBtn );
    DECL_LINK(InsertPathHdl, void *);
    DECL_LINK( OutputTypeHdl, RadioButton* pBtn );
    DECL_LINK( FilenameHdl, CheckBox* pBtn );
    DECL_LINK(ModifyHdl, void *);
    DECL_LINK( SaveTypeHdl, RadioButton* pBtn );

    virtual void    Apply() SAL_OVERRIDE;
    bool            ExecQryShell();

public:
     SwMailMergeDlg(Window* pParent, SwWrtShell& rSh,
         const OUString& rSourceName,
        const OUString& rTblName,
        sal_Int32 nCommandType,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& xConnection,
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >* pSelection = 0);
    virtual ~SwMailMergeDlg();

    inline sal_uInt16   GetMergeType() { return nMergeType; }

    bool IsSaveIndividualDocs() const { return m_pSaveIndividualRB->IsChecked(); }
    bool IsGenerateFromDataBase() const { return m_pGenerateFromDataBaseCB->IsChecked(); }
    OUString GetColumnName() const { return m_pColumnLB->GetSelectEntry();}
    OUString GetPath() const { return m_pPathED->GetText();}

    const OUString& GetSaveFilter() const {return m_sSaveFilter;}
    inline const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > GetSelection() const { return m_aSelection; }
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> GetResultSet() const;

};

class SwMailMergeCreateFromDlg : public ModalDialog
{
    RadioButton* m_pThisDocRB;
public:
    SwMailMergeCreateFromDlg(Window* pParent);
    bool IsThisDocument() const
    {
        return m_pThisDocRB->IsChecked();
    }
};

class SwMailMergeFieldConnectionsDlg : public ModalDialog
{
    RadioButton* m_pUseExistingRB;
public:
    SwMailMergeFieldConnectionsDlg(Window* pParent);
    bool IsUseExistingConnections() const
    {
        return m_pUseExistingRB->IsChecked();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
