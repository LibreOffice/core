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
#ifndef INCLUDED_SW_SOURCE_UI_DBUI_MMOUTPUTPAGE_HXX
#define INCLUDED_SW_SOURCE_UI_DBUI_MMOUTPUTPAGE_HXX
#include <svtools/wizardmachine.hxx>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/combobox.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/prgsbar.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/basedlgs.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
#include "mailmergehelper.hxx"

class SwMailMergeWizard;
class SfxPrinter;
class SwSendMailDialog;

namespace com{ namespace sun{ namespace star{
    namespace mail{
        class XMailMessage;
    }
}}}

/// Dialog implementing the saving as of the result document.
class SwMMResultSaveDialog : public SfxModalDialog
{
    VclPtr<RadioButton>    m_pSaveAsOneRB;
    VclPtr<RadioButton>    m_pSaveIndividualRB;
    VclPtr<RadioButton>    m_pFromRB;
    VclPtr<NumericField>   m_pFromNF;
    VclPtr<FixedText>      m_pToFT;
    VclPtr<NumericField>   m_pToNF;

    VclPtr<Button>         m_pOKButton;

    bool                   m_bCancelSaving;

    DECL_LINK_TYPED(SaveOutputHdl_Impl, Button* , void);
    DECL_LINK_TYPED(SaveCancelHdl_Impl, Button*, void);
    DECL_LINK_TYPED(DocumentSelectionHdl_Impl, Button*, void);

public:
    SwMMResultSaveDialog(vcl::Window* pParent = nullptr);
    virtual ~SwMMResultSaveDialog();

    virtual void dispose() override;
};

/// Dialog implementing the printing of the result document.
class SwMMResultPrintDialog : public SfxModalDialog
{
    VclPtr<FixedText>      m_pPrinterFT;
    VclPtr<ListBox>        m_pPrinterLB;
    VclPtr<PushButton>     m_pPrinterSettingsPB;

    VclPtr<RadioButton>    m_pPrintAllRB;

    VclPtr<RadioButton>    m_pFromRB;
    VclPtr<NumericField>   m_pFromNF;
    VclPtr<FixedText>      m_pToFT;
    VclPtr<NumericField>   m_pToNF;

    VclPtr<Button>         m_pOKButton;

    VclPtr<Printer>        m_pTempPrinter;

    DECL_LINK_TYPED(PrinterChangeHdl_Impl, ListBox&,void );
    DECL_LINK_TYPED(PrintHdl_Impl, Button*, void);
    DECL_LINK_TYPED(PrinterSetupHdl_Impl, Button*, void );
    DECL_LINK_TYPED(DocumentSelectionHdl_Impl, Button*, void);

    void FillInPrinterSettings();

public:
    SwMMResultPrintDialog(vcl::Window* pParent = nullptr);
    virtual ~SwMMResultPrintDialog();

    virtual void dispose() override;
};

/// Dialog implementing the sending as email of the result document.
class SwMMResultEmailDialog : public SfxModalDialog
{
    VclPtr<FixedText>      m_pMailToFT;
    VclPtr<ListBox>        m_pMailToLB;
    VclPtr<PushButton>     m_pCopyToPB;

    VclPtr<FixedText>      m_pSubjectFT;
    VclPtr<Edit>           m_pSubjectED;

    VclPtr<FixedText>      m_pSendAsFT;
    VclPtr<ListBox>        m_pSendAsLB;
    VclPtr<PushButton>     m_pSendAsPB;

    VclPtr<VclContainer>   m_pAttachmentGroup;
    VclPtr<Edit>           m_pAttachmentED;

    VclPtr<RadioButton>    m_pSendAllRB;

    VclPtr<RadioButton>    m_pFromRB;
    VclPtr<NumericField>   m_pFromNF;
    VclPtr<FixedText>      m_pToFT;
    VclPtr<NumericField>   m_pToNF;

    VclPtr<Button>         m_pOKButton;

    OUString        m_sDefaultAttachmentST;
    OUString        m_sNoSubjectST;
    OUString        m_sConfigureMail;

    OUString        m_sCC;
    OUString        m_sBCC;

    OUString        m_sBody;

    DECL_LINK_TYPED(CopyToHdl_Impl, Button*, void);
    DECL_LINK_TYPED(SendTypeHdl_Impl, ListBox&, void);
    DECL_LINK_TYPED(SendAsHdl_Impl, Button*, void);
    DECL_LINK_TYPED(SendDocumentsHdl_Impl, Button*, void);
    DECL_LINK_TYPED(DocumentSelectionHdl_Impl, Button*, void);

    void FillInEmailSettings();

public:
    SwMMResultEmailDialog(vcl::Window* pParent = nullptr);
    virtual ~SwMMResultEmailDialog();

    virtual void dispose() override;
};

struct SwMailDescriptor
{
    OUString                             sEMail;
    OUString                             sAttachmentURL;
    OUString                             sAttachmentName;
    OUString                             sMimeType;
    OUString                             sSubject;
    OUString                             sBodyMimeType;
    OUString                             sBodyContent;

    OUString                             sCC;
    OUString                             sBCC;
};
struct SwSendMailDialog_Impl;
class SwMailMergeConfigItem;
class SwSendMailDialog : public ModelessDialog //SfxModalDialog
{
    VclPtr<FixedText>               m_pTransferStatus;
    VclPtr<FixedText>               m_pPaused;
    VclPtr<ProgressBar>             m_pProgressBar;
    VclPtr<FixedText>               m_pErrorStatus;

    VclPtr<SvSimpleTableContainer>  m_pContainer;
    VclPtr<HeaderBar>               m_pStatusHB;
    VclPtr<SvSimpleTable>           m_pStatus;

    VclPtr<PushButton>              m_pStop;
    VclPtr<PushButton>              m_pClose;

    OUString                m_sContinue;
    OUString                m_sStop;
    OUString                m_sTransferStatus;
    OUString                m_sErrorStatus;
    OUString                m_sSendingTo;
    OUString                m_sCompleted;
    OUString                m_sFailed;

    bool                    m_bCancel;
    bool                    m_bDesctructionEnabled;

    ImageList               m_aImageList;

    SwSendMailDialog_Impl*  m_pImpl;
    SwMailMergeConfigItem*  m_pConfigItem;
    sal_Int32               m_nStatusHeight;
    sal_Int32               m_nSendCount;
    sal_Int32               m_nErrorCount;

    DECL_DLLPRIVATE_LINK_TYPED( StopHdl_Impl, Button*, void );
    DECL_DLLPRIVATE_LINK_TYPED( CloseHdl_Impl, Button* , void);
    DECL_DLLPRIVATE_STATIC_LINK_TYPED( SwSendMailDialog, StartSendMails, void*, void );
    DECL_DLLPRIVATE_STATIC_LINK_TYPED( SwSendMailDialog, StopSendMails, void*, void );
    DECL_DLLPRIVATE_LINK_TYPED( RemoveThis, Idle*, void );

    SAL_DLLPRIVATE void        IterateMails();
    SAL_DLLPRIVATE void        SendMails();
    SAL_DLLPRIVATE void        UpdateTransferStatus();

    virtual void        StateChanged( StateChangedType nStateChange ) override;

public:
    SwSendMailDialog( vcl::Window* pParent, SwMailMergeConfigItem& );
    virtual ~SwSendMailDialog();
    virtual void        dispose() override;

    void                AddDocument( SwMailDescriptor& rDesc );
    void                SetDocumentCount( sal_Int32 nAllDocuments );
    void                EnableDesctruction() {m_bDesctructionEnabled = true;}
    void                ShowDialog();

    void                DocumentSent( css::uno::Reference< css::mail::XMailMessage>,
                                        bool bResult,
                                        const OUString* pError );
    void                AllMailsSent();

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
