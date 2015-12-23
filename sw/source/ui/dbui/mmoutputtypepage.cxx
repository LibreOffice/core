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

#include <mmoutputtypepage.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <vcl/msgbox.hxx>
#include <dbui.hrc>
#include <swtypes.hxx>

#include <rtl/ref.hxx>
#include <com/sun/star/mail/XSmtpService.hpp>
#include <vcl/svapp.hxx>
#include <vcl/idle.hxx>

#include <helpid.h>
#include <cmdid.h>
#include <swunohelper.hxx>
#include <mmresultdialogs.hxx>
#include <maildispatcher.hxx>
#include <imaildsplistener.hxx>

using namespace ::com::sun::star;

SwMailMergeOutputTypePage::SwMailMergeOutputTypePage(SwMailMergeWizard* pParent)
    : svt::OWizardPage(pParent, "MMOutputTypePage",
        "modules/swriter/ui/mmoutputtypepage.ui")
    , m_pWizard(pParent)
{
    get(m_pLetterRB, "letter");
    get(m_pMailRB, "email");
    get(m_pLetterHint, "letterft");
    get(m_pMailHint, "emailft");

    Link<Button*,void> aLink = LINK(this, SwMailMergeOutputTypePage, TypeHdl_Impl);
    m_pLetterRB->SetClickHdl(aLink);
    m_pMailRB->SetClickHdl(aLink);

    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if(rConfigItem.IsOutputToLetter())
        m_pLetterRB->Check();
    else
        m_pMailRB->Check();
    TypeHdl_Impl(m_pLetterRB);

}

SwMailMergeOutputTypePage::~SwMailMergeOutputTypePage()
{
    disposeOnce();
}

void SwMailMergeOutputTypePage::dispose()
{
    m_pLetterRB.clear();
    m_pMailRB.clear();
    m_pLetterHint.clear();
    m_pMailHint.clear();
    m_pWizard.clear();
    svt::OWizardPage::dispose();
}


IMPL_LINK_NOARG_TYPED(SwMailMergeOutputTypePage, TypeHdl_Impl, Button*, void)
{
    bool bLetter = m_pLetterRB->IsChecked();
    m_pLetterHint->Show(bLetter);
    m_pMailHint->Show(!bLetter);
    m_pWizard->GetConfigItem().SetOutputToLetter(bLetter);
    m_pWizard->updateRoadmapItemLabel( MM_ADDRESSBLOCKPAGE );
    m_pWizard->UpdateRoadmap();
}

struct SwSendMailDialog_Impl
{
    friend class SwSendMailDialog;
    ::osl::Mutex                                aDescriptorMutex;

    ::std::vector< SwMailDescriptor >           aDescriptors;
    sal_uInt32                                  nCurrentDescriptor;
    sal_uInt32                                  nDocumentCount;
    ::rtl::Reference< MailDispatcher >          xMailDispatcher;
    ::rtl::Reference< IMailDispatcherListener>  xMailListener;
    uno::Reference< mail::XMailService >        xConnectedMailService;
    uno::Reference< mail::XMailService >        xConnectedInMailService;
    Idle                                        aRemoveIdle;

    SwSendMailDialog_Impl() :
        nCurrentDescriptor(0),
        nDocumentCount(0)
             {
                aRemoveIdle.SetPriority(SchedulerPriority::LOWEST);
             }

    ~SwSendMailDialog_Impl()
    {
        // Shutdown must be called when the last reference to the
        // mail dispatcher will be released in order to force a
        // shutdown of the mail dispatcher thread.
        // 'join' with the mail dispatcher thread leads to a
        // deadlock (SolarMutex).
        if( xMailDispatcher.is() && !xMailDispatcher->isShutdownRequested() )
            xMailDispatcher->shutdown();
    }
    const SwMailDescriptor* GetNextDescriptor();
};

const SwMailDescriptor* SwSendMailDialog_Impl::GetNextDescriptor()
{
    ::osl::MutexGuard aGuard(aDescriptorMutex);
    if(nCurrentDescriptor < aDescriptors.size())
    {
        ++nCurrentDescriptor;
        return &aDescriptors[nCurrentDescriptor - 1];
    }
    return nullptr;
}

using namespace ::com::sun::star;
class SwMailDispatcherListener_Impl : public IMailDispatcherListener
{
    VclPtr<SwSendMailDialog> m_pSendMailDialog;

public:
    explicit SwMailDispatcherListener_Impl(SwSendMailDialog& rParentDlg);
    virtual ~SwMailDispatcherListener_Impl();

    virtual void started(::rtl::Reference<MailDispatcher> xMailDispatcher) override;
    virtual void stopped(::rtl::Reference<MailDispatcher> xMailDispatcher) override;
    virtual void idle(::rtl::Reference<MailDispatcher> xMailDispatcher) override;
    virtual void mailDelivered(::rtl::Reference<MailDispatcher> xMailDispatcher,
                uno::Reference< mail::XMailMessage> xMailMessage) override;
    virtual void mailDeliveryError(::rtl::Reference<MailDispatcher> xMailDispatcher,
                uno::Reference< mail::XMailMessage> xMailMessage, const OUString& sErrorMessage) override;

    static void DeleteAttachments( uno::Reference< mail::XMailMessage >& xMessage );
};

SwMailDispatcherListener_Impl::SwMailDispatcherListener_Impl(SwSendMailDialog& rParentDlg) :
    m_pSendMailDialog(&rParentDlg)
{
}

SwMailDispatcherListener_Impl::~SwMailDispatcherListener_Impl()
{
}

void SwMailDispatcherListener_Impl::started(::rtl::Reference<MailDispatcher> /*xMailDispatcher*/)
{
}

void SwMailDispatcherListener_Impl::stopped(
                        ::rtl::Reference<MailDispatcher> /*xMailDispatcher*/)
{
}

void SwMailDispatcherListener_Impl::idle(::rtl::Reference<MailDispatcher> /*xMailDispatcher*/)
{
    SolarMutexGuard aGuard;
    m_pSendMailDialog->AllMailsSent();
}

void SwMailDispatcherListener_Impl::mailDelivered(
                        ::rtl::Reference<MailDispatcher> /*xMailDispatcher*/,
                        uno::Reference< mail::XMailMessage> xMailMessage)
{
    SolarMutexGuard aGuard;
    m_pSendMailDialog->DocumentSent( xMailMessage, true, nullptr );
    DeleteAttachments( xMailMessage );
}

void SwMailDispatcherListener_Impl::mailDeliveryError(
                ::rtl::Reference<MailDispatcher> /*xMailDispatcher*/,
                uno::Reference< mail::XMailMessage> xMailMessage,
                const OUString& sErrorMessage)
{
    SolarMutexGuard aGuard;
    m_pSendMailDialog->DocumentSent( xMailMessage, false, &sErrorMessage );
    DeleteAttachments( xMailMessage );
}

void SwMailDispatcherListener_Impl::DeleteAttachments( uno::Reference< mail::XMailMessage >& xMessage )
{
    uno::Sequence< mail::MailAttachment > aAttachments = xMessage->getAttachments();

    for(sal_Int32 nFile = 0; nFile < aAttachments.getLength(); ++nFile)
    {
        try
        {
            uno::Reference< beans::XPropertySet > xTransferableProperties( aAttachments[nFile].Data, uno::UNO_QUERY_THROW);
            if( xTransferableProperties.is() )
            {
                OUString sURL;
                xTransferableProperties->getPropertyValue("URL") >>= sURL;
                if(!sURL.isEmpty())
                    SWUnoHelper::UCB_DeleteFile( sURL );
            }
        }
        catch (const uno::Exception&)
        {
        }
    }
}

class SwSendWarningBox_Impl : public MessageDialog
{
    VclPtr<VclMultiLineEdit> m_pDetailED;
public:
    SwSendWarningBox_Impl(vcl::Window* pParent, const OUString& rDetails);
    virtual ~SwSendWarningBox_Impl() { disposeOnce(); }
    virtual void dispose() override
    {
        m_pDetailED.clear();
        MessageDialog::dispose();
    }
};

SwSendWarningBox_Impl::SwSendWarningBox_Impl(vcl::Window* pParent, const OUString& rDetails)
    : MessageDialog(pParent, "WarnEmailDialog", "modules/swriter/ui/warnemaildialog.ui")
{
    get(m_pDetailED, "errors");
    m_pDetailED->SetMaxTextWidth(80 * m_pDetailED->approximate_char_width());
    m_pDetailED->set_width_request(80 * m_pDetailED->approximate_char_width());
    m_pDetailED->set_height_request(8 * m_pDetailED->GetTextHeight());
    m_pDetailED->SetText(rDetails);
}

#define ITEMID_TASK     1
#define ITEMID_STATUS   2

SwSendMailDialog::SwSendMailDialog(vcl::Window *pParent, SwMailMergeConfigItem& rConfigItem) :
    ModelessDialog /*SfxModalDialog*/(pParent, "SendMailsDialog", "modules/swriter/ui/mmsendmails.ui"),
    m_pTransferStatus(get<FixedText>("transferstatus")),
    m_pPaused(get<FixedText>("paused")),
    m_pProgressBar(get<ProgressBar>("progress")),
    m_pErrorStatus(get<FixedText>("errorstatus")),
    m_pContainer(get<SvSimpleTableContainer>("container")),
    m_pStop(get<PushButton>("stop")),
    m_pClose(get<PushButton>("close")),
    m_sContinue(SW_RES( ST_CONTINUE )),
    m_sStop(m_pStop->GetText()),
    m_sTransferStatus(m_pTransferStatus->GetText()),
    m_sErrorStatus(   m_pErrorStatus->GetText()),
    m_sSendingTo(   SW_RES(ST_SENDINGTO )),
    m_sCompleted(   SW_RES(ST_COMPLETED )),
    m_sFailed(      SW_RES(ST_FAILED     )),
    m_bCancel(false),
    m_bDesctructionEnabled(false),
    m_aImageList( SW_RES( ILIST ) ),
    m_pImpl(new SwSendMailDialog_Impl),
    m_pConfigItem(&rConfigItem),
    m_nSendCount(0),
    m_nErrorCount(0)
{
    Size aSize = m_pContainer->LogicToPixel(Size(226, 80), MAP_APPFONT);
    m_pContainer->set_width_request(aSize.Width());
    m_pContainer->set_height_request(aSize.Height());
    m_pStatus = VclPtr<SvSimpleTable>::Create(*m_pContainer);
    m_pStatusHB = &(m_pStatus->GetTheHeaderBar());

    m_nStatusHeight = m_pContainer->get_height_request();
    OUString sTask(SW_RES(ST_TASK));
    OUString sStatus(SW_RES(ST_STATUS));

    m_pStop->SetClickHdl(LINK( this, SwSendMailDialog, StopHdl_Impl));
    m_pClose->SetClickHdl(LINK( this, SwSendMailDialog, CloseHdl_Impl));

    long nPos1 = aSize.Width()/3 * 2;
    long nPos2 = aSize.Width()/3;
    m_pStatusHB->InsertItem( ITEMID_TASK, sTask,
                            nPos1,
                            HeaderBarItemBits::LEFT | HeaderBarItemBits::VCENTER );
    m_pStatusHB->InsertItem( ITEMID_STATUS, sStatus,
                            nPos2,
                            HeaderBarItemBits::LEFT | HeaderBarItemBits::VCENTER );

    static long nTabs[] = {2, 0, nPos1};
    m_pStatus->SetStyle( m_pStatus->GetStyle() | WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP );
    m_pStatus->SetSelectionMode( SINGLE_SELECTION );
    m_pStatus->SetTabs(&nTabs[0], MAP_PIXEL);
    m_pStatus->SetSpaceBetweenEntries(3);

    UpdateTransferStatus();
}

SwSendMailDialog::~SwSendMailDialog()
{
    disposeOnce();
}

void SwSendMailDialog::dispose()
{
    if(m_pImpl->xMailDispatcher.is())
    {
        try
        {
            if(m_pImpl->xMailDispatcher->isStarted())
                m_pImpl->xMailDispatcher->stop();
            if(m_pImpl->xConnectedMailService.is() && m_pImpl->xConnectedMailService->isConnected())
                m_pImpl->xConnectedMailService->disconnect();
            if(m_pImpl->xConnectedInMailService.is() && m_pImpl->xConnectedInMailService->isConnected())
                m_pImpl->xConnectedInMailService->disconnect();

            uno::Reference<mail::XMailMessage> xMessage =
                    m_pImpl->xMailDispatcher->dequeueMailMessage();
            while(xMessage.is())
            {
                SwMailDispatcherListener_Impl::DeleteAttachments( xMessage );
                xMessage = m_pImpl->xMailDispatcher->dequeueMailMessage();
            }
        }
        catch (const uno::Exception&)
        {
        }
    }
    delete m_pImpl;
    m_pStatus.disposeAndClear();
    m_pTransferStatus.clear();
    m_pPaused.clear();
    m_pProgressBar.clear();
    m_pErrorStatus.clear();
    m_pContainer.clear();
    m_pStatusHB.clear();
    m_pStop.clear();
    m_pClose.clear();
    ModelessDialog::dispose();
}

void SwSendMailDialog::AddDocument( SwMailDescriptor& rDesc )
{
    ::osl::MutexGuard aGuard(m_pImpl->aDescriptorMutex);
    m_pImpl->aDescriptors.push_back(rDesc);
    // if the dialog is already running then continue sending of documents
    if(m_pImpl->xMailDispatcher.is())
    {
        IterateMails();
    }

}

void SwSendMailDialog::SetDocumentCount( sal_Int32 nAllDocuments )
{
    m_pImpl->nDocumentCount = nAllDocuments;
    UpdateTransferStatus();
}

IMPL_LINK_TYPED( SwSendMailDialog, StopHdl_Impl, Button*, pButton, void )
{
    m_bCancel = true;
    if(m_pImpl->xMailDispatcher.is())
    {
        if(m_pImpl->xMailDispatcher->isStarted())
        {
            m_pImpl->xMailDispatcher->stop();
            pButton->SetText(m_sContinue);
            m_pPaused->Show();
        }
        else
        {
            m_pImpl->xMailDispatcher->start();
            pButton->SetText(m_sStop);
            m_pPaused->Show(false);
        }
    }
}

IMPL_LINK_NOARG_TYPED(SwSendMailDialog, CloseHdl_Impl, Button*, void)
{
    ModelessDialog::Show( false );
}

IMPL_STATIC_LINK_TYPED( SwSendMailDialog, StartSendMails, void*, pDialog, void )
{
    static_cast<SwSendMailDialog*>(pDialog)->SendMails();
}

IMPL_LINK_TYPED( SwSendMailDialog, RemoveThis, Idle*, pTimer, void )
{
    if( m_pImpl->xMailDispatcher.is() )
    {
        if(m_pImpl->xMailDispatcher->isStarted())
            m_pImpl->xMailDispatcher->stop();
        if(!m_pImpl->xMailDispatcher->isShutdownRequested())
            m_pImpl->xMailDispatcher->shutdown();
    }

    if( m_bDesctructionEnabled &&
            (!m_pImpl->xMailDispatcher.is() ||
                    !m_pImpl->xMailDispatcher->isRunning()))
    {
        disposeOnce();
    }
    else
    {
        pTimer->Start();
    }
}

IMPL_STATIC_LINK_TYPED( SwSendMailDialog, StopSendMails, void*, p, void )
{
    SwSendMailDialog* pDialog = static_cast<SwSendMailDialog*>(p);
    if(pDialog->m_pImpl->xMailDispatcher.is() &&
        pDialog->m_pImpl->xMailDispatcher->isStarted())
    {
        pDialog->m_pImpl->xMailDispatcher->stop();
        pDialog->m_pStop->SetText(pDialog->m_sContinue);
        pDialog->m_pPaused->Show();
    }
}

void  SwSendMailDialog::SendMails()
{
    if(!m_pConfigItem)
    {
        OSL_FAIL("config item not set");
        return;
    }
    EnterWait();
    //get a mail server connection
    uno::Reference< mail::XSmtpService > xSmtpServer =
                SwMailMergeHelper::ConnectToSmtpServer( *m_pConfigItem,
                                            m_pImpl->xConnectedInMailService,
                                            aEmptyOUStr, aEmptyOUStr, this );
    bool bIsLoggedIn = xSmtpServer.is() && xSmtpServer->isConnected();
    LeaveWait();
    if(!bIsLoggedIn)
    {
        OSL_FAIL("create error message");
        return;
    }
    m_pImpl->xMailDispatcher.set( new MailDispatcher(xSmtpServer));
    IterateMails();
    m_pImpl->xMailListener = new SwMailDispatcherListener_Impl(*this);
    m_pImpl->xMailDispatcher->addListener(m_pImpl->xMailListener);
    if(!m_bCancel)
    {
        m_pImpl->xMailDispatcher->start();
    }
}

void  SwSendMailDialog::IterateMails()
{
    const SwMailDescriptor* pCurrentMailDescriptor = m_pImpl->GetNextDescriptor();
    while( pCurrentMailDescriptor )
    {
        if(!SwMailMergeHelper::CheckMailAddress( pCurrentMailDescriptor->sEMail ))
        {
            Image aInsertImg = m_aImageList.GetImage( FN_FORMULA_CANCEL );

            OUString sMessage = m_sSendingTo;
            OUString sTmp(pCurrentMailDescriptor->sEMail);
            sTmp += "\t";
            sTmp += m_sFailed;
            m_pStatus->InsertEntry( sMessage.replaceFirst("%1", sTmp), aInsertImg, aInsertImg);
            ++m_nSendCount;
            ++m_nErrorCount;
            UpdateTransferStatus( );
            pCurrentMailDescriptor = m_pImpl->GetNextDescriptor();
            continue;
        }
        SwMailMessage* pMessage = new SwMailMessage;
        uno::Reference< mail::XMailMessage > xMessage = pMessage;
        if(m_pConfigItem->IsMailReplyTo())
            pMessage->setReplyToAddress(m_pConfigItem->GetMailReplyTo());
        pMessage->addRecipient( pCurrentMailDescriptor->sEMail );
        pMessage->SetSenderName( m_pConfigItem->GetMailDisplayName() );
        pMessage->SetSenderAddress( m_pConfigItem->GetMailAddress() );
        if(!pCurrentMailDescriptor->sAttachmentURL.isEmpty())
        {
            mail::MailAttachment aAttach;
            aAttach.Data =
                    new SwMailTransferable(
                        pCurrentMailDescriptor->sAttachmentURL,
                        pCurrentMailDescriptor->sAttachmentName,
                        pCurrentMailDescriptor->sMimeType );
            aAttach.ReadableName = pCurrentMailDescriptor->sAttachmentName;
            pMessage->addAttachment( aAttach );
        }
        pMessage->setSubject( pCurrentMailDescriptor->sSubject );
        uno::Reference< datatransfer::XTransferable> xBody =
                    new SwMailTransferable(
                        pCurrentMailDescriptor->sBodyContent,
                        pCurrentMailDescriptor->sBodyMimeType);
        pMessage->setBody( xBody );

        //CC and BCC are tokenized by ';'
        if(!pCurrentMailDescriptor->sCC.isEmpty())
        {
            sal_Int32 nPos = 0;
            do
            {
                OUString sTmp = pCurrentMailDescriptor->sCC.getToken( 0, ';', nPos );
                if( !sTmp.isEmpty() )
                    pMessage->addCcRecipient( sTmp );
            }
            while (nPos >= 0);
        }
        if(!pCurrentMailDescriptor->sBCC.isEmpty())
        {
            sal_Int32 nPos = 0;
            do
            {
                OUString sTmp = pCurrentMailDescriptor->sBCC.getToken( 0, ';', nPos );
                if( !sTmp.isEmpty() )
                    pMessage->addBccRecipient( sTmp );
            }
            while (nPos >= 0);
        }
        m_pImpl->xMailDispatcher->enqueueMailMessage( xMessage );
        pCurrentMailDescriptor = m_pImpl->GetNextDescriptor();
    }
    UpdateTransferStatus();
}

void SwSendMailDialog::ShowDialog()
{
    Application::PostUserEvent( LINK( this, SwSendMailDialog,
                                      StartSendMails ), this, true );
    ModelessDialog::Show();
}

void  SwSendMailDialog::StateChanged( StateChangedType nStateChange )
{
    ModelessDialog::StateChanged( nStateChange );
    if(StateChangedType::Visible == nStateChange && !IsVisible())
    {
        m_pImpl->aRemoveIdle.SetIdleHdl( LINK( this, SwSendMailDialog,
                                                    RemoveThis ) );
        m_pImpl->aRemoveIdle.Start();
    }
}

void SwSendMailDialog::DocumentSent( uno::Reference< mail::XMailMessage> xMessage,
                                        bool bResult,
                                        const OUString* pError )
{
    //sending should stop on send errors
    if(pError &&
        m_pImpl->xMailDispatcher.is() && m_pImpl->xMailDispatcher->isStarted())
    {
        Application::PostUserEvent( LINK( this, SwSendMailDialog,
                                          StopSendMails ), this, true );
    }
    Image aInsertImg = m_aImageList.GetImage( bResult ? FN_FORMULA_APPLY : FN_FORMULA_CANCEL );

    OUString sMessage = m_sSendingTo;
    OUString sTmp(xMessage->getRecipients()[0]);
    sTmp += "\t";
    sTmp += bResult ? m_sCompleted : m_sFailed;
    m_pStatus->InsertEntry( sMessage.replaceFirst("%1", sTmp), aInsertImg, aInsertImg);
    ++m_nSendCount;
    if(!bResult)
        ++m_nErrorCount;

    UpdateTransferStatus( );

    if (pError)
    {
        VclPtrInstance< SwSendWarningBox_Impl > pDlg(nullptr, *pError);
        pDlg->Execute();
    }
}

void SwSendMailDialog::UpdateTransferStatus()
{
    OUString sStatus( m_sTransferStatus );
    sStatus = sStatus.replaceFirst("%1", OUString::number(m_nSendCount) );
    sStatus = sStatus.replaceFirst("%2", OUString::number(m_pImpl->aDescriptors.size()));
    m_pTransferStatus->SetText(sStatus);

    sStatus = m_sErrorStatus.replaceFirst("%1", OUString::number(m_nErrorCount) );
    m_pErrorStatus->SetText(sStatus);

    if(m_pImpl->aDescriptors.size())
        m_pProgressBar->SetValue((sal_uInt16)(m_nSendCount * 100 / m_pImpl->aDescriptors.size()));
    else
        m_pProgressBar->SetValue(0);
}

void SwSendMailDialog::AllMailsSent()
{
    m_pStop->Enable(false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
