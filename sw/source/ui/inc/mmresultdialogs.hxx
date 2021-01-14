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

#include <vcl/weld.hxx>
#include <vcl/print.hxx>
#include <sfx2/basedlgs.hxx>
#include <mailmergehelper.hxx>

class SwMailMergeWizard;
class SfxPrinter;
class SwSendMailDialog;
namespace com::sun::star::mail { class XMailMessage; }


/// Dialog implementing the saving as of the result document.
class SwMMResultSaveDialog : public SfxDialogController
{
    bool                   m_bCancelSaving;

    std::unique_ptr<weld::RadioButton> m_xSaveAsOneRB;
    std::unique_ptr<weld::RadioButton> m_xSaveIndividualRB;
    std::unique_ptr<weld::RadioButton> m_xFromRB;
    std::unique_ptr<weld::SpinButton> m_xFromNF;
    std::unique_ptr<weld::Label> m_xToFT;
    std::unique_ptr<weld::SpinButton> m_xToNF;
    std::unique_ptr<weld::Button> m_xOKButton;

    DECL_LINK(SaveOutputHdl_Impl, weld::Button& , void);
    DECL_LINK(DocumentSelectionHdl_Impl, weld::ToggleButton&, void);

public:
    SwMMResultSaveDialog(weld::Window* pParent);
    virtual ~SwMMResultSaveDialog() override;
};

/// Dialog implementing the printing of the result document.
class SwMMResultPrintDialog : public SfxDialogController
{
    VclPtr<Printer>        m_pTempPrinter;

    std::unique_ptr<weld::Label>        m_xPrinterFT;
    std::unique_ptr<weld::ComboBox> m_xPrinterLB;
    std::unique_ptr<weld::Button>       m_xPrinterSettingsPB;
    std::unique_ptr<weld::RadioButton>  m_xPrintAllRB;
    std::unique_ptr<weld::RadioButton>  m_xFromRB;
    std::unique_ptr<weld::SpinButton>   m_xFromNF;
    std::unique_ptr<weld::Label>        m_xToFT;
    std::unique_ptr<weld::SpinButton>   m_xToNF;
    std::unique_ptr<weld::Button>       m_xOKButton;

    DECL_LINK(PrinterChangeHdl_Impl, weld::ComboBox&, void );
    DECL_LINK(PrintHdl_Impl, weld::Button&, void);
    DECL_LINK(PrinterSetupHdl_Impl, weld::Button&, void );
    DECL_LINK(DocumentSelectionHdl_Impl, weld::ToggleButton&, void);

    void FillInPrinterSettings();

public:
    SwMMResultPrintDialog(weld::Window* pParent);
    virtual ~SwMMResultPrintDialog() override;
};

/// Dialog implementing the sending as email of the result document.
class SwMMResultEmailDialog : public SfxDialogController
{
    OUString        m_sConfigureMail;
    OUString        m_sCC;
    OUString        m_sBCC;
    OUString        m_sBody;

    std::unique_ptr<weld::Label> m_xMailToFT;
    std::unique_ptr<weld::ComboBox> m_xMailToLB;
    std::unique_ptr<weld::Button> m_xCopyToPB;
    std::unique_ptr<weld::Label> m_xSubjectFT;
    std::unique_ptr<weld::Entry> m_xSubjectED;
    std::unique_ptr<weld::Label> m_xSendAsFT;
    std::unique_ptr<weld::ComboBox> m_xSendAsLB;
    std::unique_ptr<weld::Button> m_xSendAsPB;
    std::unique_ptr<weld::Widget> m_xAttachmentGroup;
    std::unique_ptr<weld::Entry> m_xAttachmentED;
    std::unique_ptr<weld::Label> m_xPasswordFT;
    std::unique_ptr<weld::ComboBox> m_xPasswordLB;
    std::unique_ptr<weld::CheckButton> m_xPasswordCB;
    std::unique_ptr<weld::RadioButton> m_xSendAllRB;
    std::unique_ptr<weld::RadioButton> m_xFromRB;
    std::unique_ptr<weld::SpinButton> m_xFromNF;
    std::unique_ptr<weld::Label> m_xToFT;
    std::unique_ptr<weld::SpinButton> m_xToNF;
    std::unique_ptr<weld::Button> m_xOKButton;

    DECL_LINK(CopyToHdl_Impl, weld::Button&, void);
    DECL_LINK(SendTypeHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(SendAsHdl_Impl, weld::Button&, void);
    DECL_LINK(SendDocumentsHdl_Impl, weld::Button&, void);
    DECL_LINK(DocumentSelectionHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(CheckHdl, weld::ToggleButton&, void );

    void FillInEmailSettings();

public:
    SwMMResultEmailDialog(weld::Window *pParent);
    virtual ~SwMMResultEmailDialog() override;
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
class SwSendMailDialog : public weld::GenericDialogController
{
    OUString                m_sContinue;
    OUString                m_sStop;
    OUString                m_sTransferStatus;
    OUString                m_sErrorStatus;
    OUString                m_sSendingTo;
    OUString                m_sCompleted;
    OUString                m_sFailed;

    bool                    m_bCancel;
    bool                    m_bDestructionEnabled;

    std::unique_ptr<SwSendMailDialog_Impl> m_pImpl;
    SwMailMergeConfigItem*  m_pConfigItem;
    sal_Int32               m_nExpectedCount;
    sal_Int32               m_nSendCount;
    sal_Int32               m_nErrorCount;

    std::unique_ptr<weld::Label> m_xTransferStatus;
    std::unique_ptr<weld::Label> m_xPaused;
    std::unique_ptr<weld::ProgressBar> m_xProgressBar;
    std::unique_ptr<weld::Label> m_xErrorStatus;
    std::unique_ptr<weld::TreeView> m_xStatus;
    std::unique_ptr<weld::Button> m_xStop;
    std::unique_ptr<weld::Button> m_xCancel;
    std::unique_ptr<weld::Expander> m_xExpander;

    DECL_LINK( StopHdl_Impl, weld::Button&, void );
    DECL_LINK( CancelHdl_Impl, weld::Button& , void);
    DECL_STATIC_LINK( SwSendMailDialog, StartSendMails, void*, void );
    DECL_STATIC_LINK( SwSendMailDialog, StopSendMails, void*, void );
    DECL_LINK( RemoveThis, Timer*, void );

    void        IterateMails();
    void        SendMails();
    void        UpdateTransferStatus();

public:
    SwSendMailDialog( weld::Window* pParent, SwMailMergeConfigItem& );
    virtual ~SwSendMailDialog() override;

    void                AddDocument( SwMailDescriptor const & rDesc );
    void                EnableDestruction() {m_bDestructionEnabled = true;}
    void                StartSend(sal_Int32 nExpectedCount);

    void                DocumentSent( css::uno::Reference< css::mail::XMailMessage> const & xMessage,
                                        bool bResult,
                                        const OUString* pError );
    void                AllMailsSent();

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
