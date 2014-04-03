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
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
#include "swdllapi.h"
#include "mailmergehelper.hxx"

class SwMailMergeWizard;
class SfxPrinter;
class SwSendMailDialog;

namespace com{ namespace sun{ namespace star{
    namespace mail{
        class XMailMessage;
    }
}}}

class SwMailMergeOutputPage : public svt::OWizardPage
{
    RadioButton*    m_pSaveStartDocRB;
    RadioButton*    m_pSaveMergedDocRB;
    RadioButton*    m_pPrintRB;
    RadioButton*    m_pSendMailRB;

    VclFrame*       m_pSeparator;

    PushButton*     m_pSaveStartDocPB;

    RadioButton*    m_pSaveAsOneRB;
    RadioButton*    m_pSaveIndividualRB;
    RadioButton*    m_pPrintAllRB; //has to be here for tab control reasons
    RadioButton*    m_pSendAllRB;  //has to be here for tab control reasons
    //this group is used in save and print
    RadioButton*    m_pFromRB;
    NumericField*   m_pFromNF;
    FixedText*      m_pToFT;
    NumericField*   m_pToNF;
    PushButton*     m_pSaveNowPB;

    FixedText*      m_pPrinterFT;
    ListBox*        m_pPrinterLB;
    PushButton*     m_pPrinterSettingsPB;
    PushButton*     m_pPrintNowPB;

    FixedText*      m_pMailToFT;
    ListBox*        m_pMailToLB;
    PushButton*     m_pCopyToPB;
    FixedText*      m_pSubjectFT;
    Edit*           m_pSubjectED;
    FixedText*      m_pSendAsFT;
    ListBox*        m_pSendAsLB;
    VclContainer*   m_pAttachmentGroup;
    Edit*           m_pAttachmentED;
    PushButton*     m_pSendAsPB;
    PushButton*     m_pSendDocumentsPB;

    //some FixedLine labels
    OUString        m_sSaveStartST;
    OUString        m_sSaveMergedST;
    OUString        m_sPrintST;
    OUString        m_sSendMailST;

    //misc strings
    OUString        m_sDefaultAttachmentST;
    OUString        m_sNoSubjectST;
    OUString        m_sConfigureMail;

    OUString        m_sBody;

    bool            m_bCancelSaving;

    SwMailMergeWizard*  m_pWizard;

    //some dialog data
    Printer*        m_pTempPrinter;
    OUString        m_sCC;
    OUString        m_sBCC;

    DECL_LINK(OutputTypeHdl_Impl, RadioButton*);
    DECL_LINK(CopyToHdl_Impl, PushButton*);
    DECL_LINK(SaveStartHdl_Impl, PushButton* );
    DECL_LINK(SaveOutputHdl_Impl, PushButton* );
    DECL_LINK(PrinterChangeHdl_Impl, ListBox* );
    DECL_LINK(PrintHdl_Impl, void *);
    DECL_LINK(PrinterSetupHdl_Impl, PushButton* );
    DECL_LINK(SendTypeHdl_Impl, ListBox*);
    DECL_LINK(SendAsHdl_Impl, PushButton*);
    DECL_LINK(SendDocumentsHdl_Impl, PushButton*);
    DECL_LINK(DocumentSelectionHdl_Impl, RadioButton*);
    DECL_LINK(SaveCancelHdl_Impl, void *);
protected:
        virtual bool    canAdvance() const SAL_OVERRIDE;
        virtual void    ActivatePage() SAL_OVERRIDE;
public:
        SwMailMergeOutputPage( SwMailMergeWizard* _pParent);
        virtual ~SwMailMergeOutputPage();

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
class SW_DLLPUBLIC SwSendMailDialog : public ModelessDialog //SfxModalDialog
{
    FixedLine               m_aStatusFL;
    FixedText               m_aStatusFT;

    FixedLine               m_aTransferStatusFL;
    FixedText               m_aTransferStatusFT;
    FixedInfo               m_PausedFI;
    ProgressBar             m_aProgressBar;
    FixedText               m_aErrorStatusFT;

    PushButton              m_aDetailsPB;
    HeaderBar               m_aStatusHB;
    SvTabListBox            m_aStatusLB;

    FixedLine               m_aSeparatorFL;

    PushButton              m_aStopPB;
    PushButton              m_aClosePB;

    OUString                m_sMore;
    OUString                m_sLess;
    OUString                m_sContinue;
    OUString                m_sStop;
    OUString                m_sSend;
    OUString                m_sTransferStatus;
    OUString                m_sErrorStatus;
    OUString                m_sSendingTo;
    OUString                m_sCompleted;
    OUString                m_sFailed;
    OUString                m_sTerminateQuery;

    bool                    m_bCancel;
    bool                    m_bDesctructionEnabled;

    ImageList               m_aImageList;

    SwSendMailDialog_Impl*  m_pImpl;
    SwMailMergeConfigItem*  m_pConfigItem;
    sal_Int32               m_nStatusHeight;
    sal_Int32               m_nSendCount;
    sal_Int32               m_nErrorCount;

    SAL_DLLPRIVATE DECL_LINK( DetailsHdl_Impl, void* );
    SAL_DLLPRIVATE DECL_LINK( StopHdl_Impl, PushButton* );
    SAL_DLLPRIVATE DECL_LINK( CloseHdl_Impl, void* );
    SAL_DLLPRIVATE DECL_STATIC_LINK( SwSendMailDialog, StartSendMails, SwSendMailDialog* );
    SAL_DLLPRIVATE DECL_STATIC_LINK( SwSendMailDialog, StopSendMails, SwSendMailDialog* );
    SAL_DLLPRIVATE DECL_STATIC_LINK( SwSendMailDialog, RemoveThis, Timer* );

    SAL_DLLPRIVATE void        IterateMails();
    SAL_DLLPRIVATE void        SendMails();
    SAL_DLLPRIVATE void        UpdateTransferStatus();

    virtual void        StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;

public:
    SwSendMailDialog( Window* pParent, SwMailMergeConfigItem& );
    virtual ~SwSendMailDialog();

    void                AddDocument( SwMailDescriptor& rDesc );
    void                SetDocumentCount( sal_Int32 nAllDocuments );
    void                EnableDesctruction() {m_bDesctructionEnabled = true;}
    void                ShowDialog();

    void                DocumentSent( ::com::sun::star::uno::Reference< ::com::sun::star::mail::XMailMessage>,
                                        bool bResult,
                                        const OUString* pError );
    void                AllMailsSent();

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
