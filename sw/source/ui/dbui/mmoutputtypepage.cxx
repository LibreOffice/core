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

#include "mmoutputtypepage.hxx"
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <swtypes.hxx>

#include <rtl/ref.hxx>
#include <com/sun/star/mail/XSmtpService.hpp>
#include <vcl/idle.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <swunohelper.hxx>
#include <mmresultdialogs.hxx>
#include <maildispatcher.hxx>
#include <imaildsplistener.hxx>

using namespace ::com::sun::star;

SwMailMergeOutputTypePage::SwMailMergeOutputTypePage(weld::Container* pPage, SwMailMergeWizard* pWizard)
    : vcl::OWizardPage(pPage, pWizard, "modules/swriter/ui/mmoutputtypepage.ui", "MMOutputTypePage")
    , m_pWizard(pWizard)
    , m_xLetterRB(m_xBuilder->weld_radio_button("letter"))
    , m_xMailRB(m_xBuilder->weld_radio_button("email"))
    , m_xLetterHint(m_xBuilder->weld_label("letterft"))
    , m_xMailHint(m_xBuilder->weld_label("emailft"))
{
    Link<weld::ToggleButton&,void> aLink = LINK(this, SwMailMergeOutputTypePage, TypeHdl_Impl);
    m_xLetterRB->connect_toggled(aLink);
    m_xMailRB->connect_toggled(aLink);

    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if(rConfigItem.IsOutputToLetter())
        m_xLetterRB->set_active(true);
    else
        m_xMailRB->set_active(true);
    TypeHdl_Impl(*m_xLetterRB);
}

SwMailMergeOutputTypePage::~SwMailMergeOutputTypePage()
{
}

IMPL_LINK_NOARG(SwMailMergeOutputTypePage, TypeHdl_Impl, weld::ToggleButton&, void)
{
    bool bLetter = m_xLetterRB->get_active();
    m_xLetterHint->set_visible(bLetter);
    m_xMailHint->set_visible(!bLetter);
    m_pWizard->GetConfigItem().SetOutputToLetter(bLetter);
    m_pWizard->UpdateRoadmap();
}

struct SwSendMailDialog_Impl
{
    friend class SwSendMailDialog;
    ::osl::Mutex                                aDescriptorMutex;

    std::vector< SwMailDescriptor >             aDescriptors;
    sal_uInt32                                  nCurrentDescriptor;
    ::rtl::Reference< MailDispatcher >          xMailDispatcher;
    ::rtl::Reference< IMailDispatcherListener>  xMailListener;
    uno::Reference< mail::XMailService >        xConnectedInMailService;
    Idle                                        aRemoveIdle;

    SwSendMailDialog_Impl() :
        nCurrentDescriptor(0)
    {
        aRemoveIdle.SetPriority(TaskPriority::LOWEST);
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

namespace {

class SwMailDispatcherListener_Impl : public IMailDispatcherListener
{
    SwSendMailDialog& m_rSendMailDialog;

public:
    explicit SwMailDispatcherListener_Impl(SwSendMailDialog& rParentDlg);

    virtual void idle() override;
    virtual void mailDelivered(uno::Reference< mail::XMailMessage> xMailMessage) override;
    virtual void mailDeliveryError(::rtl::Reference<MailDispatcher> xMailDispatcher,
                uno::Reference< mail::XMailMessage> xMailMessage, const OUString& sErrorMessage) override;

    static void DeleteAttachments( uno::Reference< mail::XMailMessage > const & xMessage );
};

}

SwMailDispatcherListener_Impl::SwMailDispatcherListener_Impl(SwSendMailDialog& rParentDlg)
    : m_rSendMailDialog(rParentDlg)
{
}

void SwMailDispatcherListener_Impl::idle()
{
    SolarMutexGuard aGuard;
    m_rSendMailDialog.AllMailsSent();
}

void SwMailDispatcherListener_Impl::mailDelivered(
                        uno::Reference< mail::XMailMessage> xMailMessage)
{
    SolarMutexGuard aGuard;
    m_rSendMailDialog.DocumentSent( xMailMessage, true, nullptr );
    DeleteAttachments( xMailMessage );
}

void SwMailDispatcherListener_Impl::mailDeliveryError(
                ::rtl::Reference<MailDispatcher> /*xMailDispatcher*/,
                uno::Reference< mail::XMailMessage> xMailMessage,
                const OUString& sErrorMessage)
{
    SolarMutexGuard aGuard;
    m_rSendMailDialog.DocumentSent( xMailMessage, false, &sErrorMessage );
    DeleteAttachments( xMailMessage );
}

void SwMailDispatcherListener_Impl::DeleteAttachments( uno::Reference< mail::XMailMessage > const & xMessage )
{
    const uno::Sequence< mail::MailAttachment > aAttachments = xMessage->getAttachments();

    for(const auto& rAttachment : aAttachments)
    {
        try
        {
            uno::Reference< beans::XPropertySet > xTransferableProperties( rAttachment.Data, uno::UNO_QUERY_THROW);
            OUString sURL;
            xTransferableProperties->getPropertyValue("URL") >>= sURL;
            if(!sURL.isEmpty())
                SWUnoHelper::UCB_DeleteFile( sURL );
        }
        catch (const uno::Exception&)
        {
        }
    }
}

namespace {

class SwSendWarningBox_Impl : public weld::MessageDialogController
{
    std::unique_ptr<weld::TextView> m_xDetailED;
public:
    SwSendWarningBox_Impl(weld::Window* pParent, const OUString& rDetails)
        : MessageDialogController(pParent, "modules/swriter/ui/warnemaildialog.ui", "WarnEmailDialog", "grid")
        , m_xDetailED(m_xBuilder->weld_text_view("errors"))
    {
        m_xDetailED->set_size_request(80 * m_xDetailED->get_approximate_digit_width(),
                                      8 * m_xDetailED->get_text_height());
        m_xDetailED->set_text(rDetails);
    }
};

}

SwSendMailDialog::SwSendMailDialog(weld::Window *pParent, SwMailMergeConfigItem& rConfigItem)
    : GenericDialogController(pParent, "modules/swriter/ui/mmsendmails.ui", "SendMailsDialog")
    , m_sContinue(SwResId( ST_CONTINUE ))
    , m_sSendingTo(   SwResId(ST_SENDINGTO ))
    , m_sCompleted(   SwResId(ST_COMPLETED ))
    , m_sFailed(      SwResId(ST_FAILED     ))
    , m_bCancel(false)
    , m_bDestructionEnabled(false)
    , m_pImpl(new SwSendMailDialog_Impl)
    , m_pConfigItem(&rConfigItem)
    , m_nExpectedCount(0)
    , m_nSendCount(0)
    , m_nErrorCount(0)
    , m_xTransferStatus(m_xBuilder->weld_label("transferstatus"))
    , m_xPaused(m_xBuilder->weld_label("paused"))
    , m_xProgressBar(m_xBuilder->weld_progress_bar("progress"))
    , m_xErrorStatus(m_xBuilder->weld_label("errorstatus"))
    , m_xStatus(m_xBuilder->weld_tree_view("container"))
    , m_xStop(m_xBuilder->weld_button("stop"))
    , m_xClose(m_xBuilder->weld_button("cancel"))
    , m_xExpander(m_xBuilder->weld_expander("details"))
{
    m_sStop = m_xStop->get_label();
    m_sTransferStatus = m_xTransferStatus->get_label();
    m_sErrorStatus = m_xErrorStatus->get_label();

    Size aSize(m_xStatus->get_approximate_digit_width() * 28,
               m_xStatus->get_height_rows(20));
    m_xStatus->set_size_request(aSize.Width(), aSize.Height());

    m_xStop->connect_clicked(LINK( this, SwSendMailDialog, StopHdl_Impl));
    m_xClose->connect_clicked(LINK( this, SwSendMailDialog, CloseHdl_Impl));

    std::vector<int> aWidths;
    aWidths.push_back(m_xStatus->get_checkbox_column_width());
    aWidths.push_back(aSize.Width()/3 * 2);
    m_xStatus->set_column_fixed_widths(aWidths);

    m_xPaused->set_visible(false);
    UpdateTransferStatus();
}

SwSendMailDialog::~SwSendMailDialog()
{
    if(!m_pImpl->xMailDispatcher.is())
        return;

    try
    {
        if(m_pImpl->xMailDispatcher->isStarted())
            m_pImpl->xMailDispatcher->stop();
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

void SwSendMailDialog::AddDocument( SwMailDescriptor const & rDesc )
{
    ::osl::MutexGuard aGuard(m_pImpl->aDescriptorMutex);
    m_pImpl->aDescriptors.push_back(rDesc);
    // if the dialog is already running then continue sending of documents
    if(m_pImpl->xMailDispatcher.is())
    {
        IterateMails();
    }
}

IMPL_LINK( SwSendMailDialog, StopHdl_Impl, weld::Button&, rButton, void )
{
    m_bCancel = true;
    if(!m_pImpl->xMailDispatcher.is())
        return;

    if(m_pImpl->xMailDispatcher->isStarted())
    {
        m_pImpl->xMailDispatcher->stop();
        rButton.set_label(m_sContinue);
        m_xPaused->show();
    }
    else
    {
        m_pImpl->xMailDispatcher->start();
        rButton.set_label(m_sStop);
        m_xPaused->hide();
    }
}

IMPL_LINK_NOARG(SwSendMailDialog, CloseHdl_Impl, weld::Button&, void)
{
    m_xDialog->hide();

    if (m_bDestructionEnabled)
        m_xDialog->response(RET_CANCEL);
    else
    {
        m_pImpl->aRemoveIdle.SetInvokeHandler( LINK( this, SwSendMailDialog, RemoveThis ) );
        m_pImpl->aRemoveIdle.Start();
    }
}

IMPL_STATIC_LINK( SwSendMailDialog, StartSendMails, void*, pDialog, void )
{
    static_cast<SwSendMailDialog*>(pDialog)->SendMails();
}

IMPL_LINK( SwSendMailDialog, RemoveThis, Timer*, pTimer, void )
{
    if( m_pImpl->xMailDispatcher.is() )
    {
        if(m_pImpl->xMailDispatcher->isStarted())
            m_pImpl->xMailDispatcher->stop();
        if(!m_pImpl->xMailDispatcher->isShutdownRequested())
            m_pImpl->xMailDispatcher->shutdown();
    }

    if( m_bDestructionEnabled &&
            (!m_pImpl->xMailDispatcher.is() ||
                    !m_pImpl->xMailDispatcher->isRunning()))
    {
        m_xDialog->response(RET_CANCEL);
    }
    else
    {
        pTimer->Start();
    }
}

IMPL_STATIC_LINK( SwSendMailDialog, StopSendMails, void*, p, void )
{
    SwSendMailDialog* pDialog = static_cast<SwSendMailDialog*>(p);
    if(pDialog->m_pImpl->xMailDispatcher.is() &&
        pDialog->m_pImpl->xMailDispatcher->isStarted())
    {
        pDialog->m_pImpl->xMailDispatcher->stop();
        pDialog->m_xStop->set_label(pDialog->m_sContinue);
        pDialog->m_xPaused->show();
    }
}

void SwSendMailDialog::SendMails()
{
    if(!m_pConfigItem)
    {
        OSL_FAIL("config item not set");
        return;
    }
    auto xWait(std::make_unique<weld::WaitObject>(m_xDialog.get()));
    //get a mail server connection
    uno::Reference< mail::XSmtpService > xSmtpServer =
                SwMailMergeHelper::ConnectToSmtpServer( *m_pConfigItem,
                                            m_pImpl->xConnectedInMailService,
                                            OUString(), OUString(), m_xDialog.get());
    bool bIsLoggedIn = xSmtpServer.is() && xSmtpServer->isConnected();
    xWait.reset();
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
        if (!SwMailMergeHelper::CheckMailAddress( pCurrentMailDescriptor->sEMail))
        {
            OUString sMessage = m_sSendingTo;
            m_xStatus->append();
            m_xStatus->set_image(m_nSendCount, RID_BMP_FORMULA_CANCEL, 0);
            m_xStatus->set_text(m_nSendCount, sMessage.replaceFirst("%1", pCurrentMailDescriptor->sEMail), 1);
            m_xStatus->set_text(m_nSendCount, m_sFailed, 1);
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

void SwSendMailDialog::StartSend(sal_Int32 nExpectedCount)
{
    Application::PostUserEvent( LINK( this, SwSendMailDialog,
                                      StartSendMails ), this );
    m_nExpectedCount = nExpectedCount > 0 ? nExpectedCount : 1;
}

void SwSendMailDialog::DocumentSent( uno::Reference< mail::XMailMessage> const & xMessage,
                                        bool bResult,
                                        const OUString* pError )
{
    //sending should stop on send errors
    if(pError &&
        m_pImpl->xMailDispatcher.is() && m_pImpl->xMailDispatcher->isStarted())
    {
        Application::PostUserEvent( LINK( this, SwSendMailDialog,
                                          StopSendMails ), this );
    }
    OUString sInsertImg(bResult ? OUString(RID_BMP_FORMULA_APPLY) : OUString(RID_BMP_FORMULA_CANCEL));

    OUString sMessage = m_sSendingTo;
    m_xStatus->append();
    m_xStatus->set_image(m_nSendCount, sInsertImg, 0);
    m_xStatus->set_text(m_nSendCount, sMessage.replaceFirst("%1", xMessage->getRecipients()[0]), 1);
    m_xStatus->set_text(m_nSendCount, bResult ? m_sCompleted : m_sFailed, 1);
    ++m_nSendCount;
    if(!bResult)
        ++m_nErrorCount;

    UpdateTransferStatus( );

    if (pError)
    {
        SwSendWarningBox_Impl aDlg(m_xDialog.get(), *pError);
        aDlg.run();
    }
}

void SwSendMailDialog::UpdateTransferStatus()
{
    OUString sStatus( m_sTransferStatus );
    sStatus = sStatus.replaceFirst("%1", OUString::number(m_nSendCount) );
    sStatus = sStatus.replaceFirst("%2", OUString::number(m_nExpectedCount));
    m_xTransferStatus->set_label(sStatus);

    sStatus = m_sErrorStatus.replaceFirst("%1", OUString::number(m_nErrorCount) );
    m_xErrorStatus->set_label(sStatus);

    if (!m_pImpl->aDescriptors.empty())
    {
        assert(m_nExpectedCount && "div-by-zero");
        m_xProgressBar->set_percentage(m_nSendCount * 100 / m_nExpectedCount);
    }
    else
        m_xProgressBar->set_percentage(0);
}

void SwSendMailDialog::AllMailsSent()
{
    // Leave open if some kind of error occurred
    if (m_nSendCount == m_nExpectedCount)
    {
        m_xStop->set_sensitive(false);
        m_xDialog->hide();
        m_xDialog->response(RET_CANCEL);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
