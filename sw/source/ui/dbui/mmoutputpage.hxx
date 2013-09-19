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
#ifndef _MAILMERGEOUTPUTPAGE_HXX
#define _MAILMERGEOUTPUTPAGE_HXX
#include <svtools/wizardmachine.hxx>
#include <vcl/button.hxx>
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
    SwBoldFixedInfo m_aHeaderFI;
    FixedInfo       m_aOptionsFI;
    RadioButton     m_aSaveStartDocRB;
    RadioButton     m_aSaveMergedDocRB;
    RadioButton     m_aPrintRB;
    RadioButton     m_aSendMailRB;

    FixedLine       m_aSeparatorFL;

    PushButton      m_aSaveStartDocPB;

    RadioButton     m_aSaveAsOneRB;
    RadioButton     m_aSaveIndividualRB;
    RadioButton     m_aPrintAllRB; //has to be here for tab control reasons
    RadioButton     m_aSendAllRB;  //has to be here for tab control reasons
    //this group is used in save and print
    RadioButton     m_aFromRB;
    NumericField    m_aFromNF;
    FixedText       m_aToFT;
    NumericField    m_aToNF;
    PushButton      m_aSaveNowPB;

    FixedText       m_aPrinterFT;
    ListBox         m_aPrinterLB;
    PushButton      m_aPrinterSettingsPB;
    PushButton      m_aPrintNowPB;

    FixedText       m_aMailToFT;
    ListBox         m_aMailToLB;
    PushButton      m_aCopyToPB;
    FixedText       m_aSubjectFT;
    Edit            m_aSubjectED;
    FixedText       m_aSendAsFT;
    ListBox         m_aSendAsLB;
    FixedText       m_aAttachmentFT;
    Edit            m_aAttachmentED;
    PushButton      m_aSendAsPB;
    PushButton      m_aSendDocumentsPB;

    //some FixedLine labels
    String          m_sSaveStartST;
    String          m_sSaveMergedST;
    String          m_sPrintST;
    String          m_sSendMailST;

    //misc strings
    String          m_sDefaultAttachmentST;
    String          m_sNoSubjectST;
    String          m_sConfigureMail;

    String          m_sBody;

    long            m_nFromToRBPos;
    long            m_nFromToFTPos;
    long            m_nFromToNFPos;
    long            m_nRBOffset;

    bool            m_bCancelSaving;

    SwMailMergeWizard*  m_pWizard;

    //some dialog data
    Printer*        m_pTempPrinter;
    String          m_sCC;
    String          m_sBCC;


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
        virtual bool    canAdvance() const;
        virtual void    ActivatePage();
public:
        SwMailMergeOutputPage( SwMailMergeWizard* _pParent);
        ~SwMailMergeOutputPage();

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

    String                  m_sMore;
    String                  m_sLess;
    String                  m_sContinue;
    String                  m_sStop;
    String                  m_sSend;
    String                  m_sTransferStatus;
    String                  m_sErrorStatus;
    String                  m_sSendingTo;
    String                  m_sCompleted;
    String                  m_sFailed;
    String                  m_sTerminateQuery;

    bool                    m_bCancel;
    bool                    m_bDesctructionEnabled;

    ImageList               m_aImageList;

    SwSendMailDialog_Impl*  m_pImpl;
    SwMailMergeConfigItem*  m_pConfigItem;
    sal_Int32               m_nStatusHeight;
    sal_Int32               m_nSendCount;
    sal_Int32               m_nErrorCount;

    SW_DLLPRIVATE DECL_LINK( DetailsHdl_Impl, void* );
    SW_DLLPRIVATE DECL_LINK( StopHdl_Impl, PushButton* );
    SW_DLLPRIVATE DECL_LINK( CloseHdl_Impl, void* );
    SW_DLLPRIVATE DECL_STATIC_LINK( SwSendMailDialog, StartSendMails, SwSendMailDialog* );
    SW_DLLPRIVATE DECL_STATIC_LINK( SwSendMailDialog, StopSendMails, SwSendMailDialog* );
    SW_DLLPRIVATE DECL_STATIC_LINK( SwSendMailDialog, RemoveThis, Timer* );

    SW_DLLPRIVATE void        IterateMails();
    SW_DLLPRIVATE void        SendMails();
    SW_DLLPRIVATE void        UpdateTransferStatus();

    virtual void        StateChanged( StateChangedType nStateChange );

public:
    SwSendMailDialog( Window* pParent, SwMailMergeConfigItem& );
    ~SwSendMailDialog();

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
