/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>

#include <cmdid.h>
#include <swmodule.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <vcl/svapp.hxx>
#include <mailmergechildwindow.hxx>
#include <mmoutputpage.hxx>
#include <mmconfigitem.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include "com/sun/star/mail/XSmtpService.hpp"
#include "com/sun/star/mail/MailServiceType.hpp"
#include <rtl/ref.hxx>
#include <maildispatcher.hxx>
#include <imaildsplistener.hxx>
#include <swunohelper.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/image.hxx>
#include <mailmergechildwindow.hrc>
#include <dbui.hrc>
#include <helpid.h>

using namespace ::com::sun::star;

SFX_IMPL_FLOATINGWINDOW( SwMailMergeChildWindow, FN_MAILMERGE_CHILDWINDOW )

SwMailMergeChildWindow::SwMailMergeChildWindow( Window* _pParent,
                                sal_uInt16 nId,
                                SfxBindings* pBindings,
                                SfxChildWinInfo* pInfo ) :
                                SfxChildWindow( _pParent, nId )
{
    pWindow = new SwMailMergeChildWin( pBindings, this, _pParent);

    if (!pInfo->aSize.Width() || !pInfo->aSize.Height())
    {
        SwView* pActiveView = ::GetActiveView();
        if(pActiveView)
        {
            const SwEditWin &rEditWin = pActiveView->GetEditWin();
            pWindow->SetPosPixel(rEditWin.OutputToScreenPixel(Point(0, 0)));
        }
        else
            pWindow->SetPosPixel(_pParent->OutputToScreenPixel(Point(0, 0)));
        pInfo->aPos = pWindow->GetPosPixel();
        pInfo->aSize = pWindow->GetSizePixel();
    }

    ((SwMailMergeChildWin *)pWindow)->Initialize(pInfo);
    pWindow->Show();
}

SwMailMergeChildWin::SwMailMergeChildWin( SfxBindings* _pBindings,
                            SfxChildWindow* pChild,
                            Window *pParent) :
    SfxFloatingWindow(_pBindings, pChild, pParent, SW_RES(DLG_MAILMERGECHILD)),
    m_aBackTB(this, SW_RES( TB_BACK ))
{
    m_aBackTB.SetSelectHdl(LINK(this, SwMailMergeChildWin, BackHdl));
    sal_uInt16 nIResId = ILIST_TBX;
    ResId aResId( nIResId, *pSwResMgr );
    ImageList aIList(aResId);
    FreeResource();

    m_aBackTB.SetItemImage( 1, aIList.GetImage(FN_SHOW_ROOT) );
    m_aBackTB.SetButtonType( BUTTON_SYMBOLTEXT );
    Size aSz = m_aBackTB.CalcWindowSizePixel(1);
    m_aBackTB.SetPosSizePixel( Point(), aSz );
    SetOutputSizePixel(aSz);
    m_aBackTB.Show();
}

SwMailMergeChildWin::~SwMailMergeChildWin()
{
}

IMPL_LINK_NOARG(SwMailMergeChildWin, BackHdl)
{
    GetBindings().GetDispatcher()->Execute(FN_MAILMERGE_WIZARD, SFX_CALLMODE_ASYNCHRON);
    return 0;
}

void SwMailMergeChildWin::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxFloatingWindow::FillInfo(rInfo);
    rInfo.aWinState = rtl::OString();
    rInfo.bVisible = sal_False;
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
    Timer                                       aRemoveTimer;


    SwSendMailDialog_Impl() :
        nCurrentDescriptor(0),
        nDocumentCount(0)
             {
                aRemoveTimer.SetTimeout(500);
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
    return 0;
}

class SwMailDispatcherListener_Impl : public IMailDispatcherListener
{
    SwSendMailDialog* m_pSendMailDialog;

public:
    SwMailDispatcherListener_Impl(SwSendMailDialog& rParentDlg);
    ~SwMailDispatcherListener_Impl();

    virtual void started(::rtl::Reference<MailDispatcher> xMailDispatcher);
    virtual void stopped(::rtl::Reference<MailDispatcher> xMailDispatcher);
    virtual void idle(::rtl::Reference<MailDispatcher> xMailDispatcher);
    virtual void mailDelivered(::rtl::Reference<MailDispatcher> xMailDispatcher,
                uno::Reference< mail::XMailMessage> xMailMessage);
    virtual void mailDeliveryError(::rtl::Reference<MailDispatcher> xMailDispatcher,
                uno::Reference< mail::XMailMessage> xMailMessage, const rtl::OUString& sErrorMessage);

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
    m_pSendMailDialog->DocumentSent( xMailMessage, true, 0 );
    DeleteAttachments( xMailMessage );
}

void SwMailDispatcherListener_Impl::mailDeliveryError(
                ::rtl::Reference<MailDispatcher> /*xMailDispatcher*/,
                uno::Reference< mail::XMailMessage> xMailMessage,
                const rtl::OUString& sErrorMessage)
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
                ::rtl::OUString sURL;
                xTransferableProperties->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL")) ) >>= sURL;
                if(!sURL.isEmpty())
                    SWUnoHelper::UCB_DeleteFile( sURL );
            }
        }
        catch (const uno::Exception&)
        {
        }
    }
}

class SwSendWarningBox_Impl : public ModalDialog
{
    FixedImage      aWarningImageIM;
    FixedInfo       aWarningFI;
    FixedText       aDetailFT;
    MultiLineEdit   aDetailED;
    FixedLine       aSeparatorFL;
    OKButton        aOKPB;

public:
    SwSendWarningBox_Impl(Window* pParent, const String& rDetails);
    ~SwSendWarningBox_Impl();
};

SwSendWarningBox_Impl::SwSendWarningBox_Impl(Window* pParent, const String& rDetails) :
    ModalDialog(pParent, SW_RES( DLG_MM_SENDWARNING )),
    aWarningImageIM(this, SW_RES( IM_WARNING         )),
    aWarningFI(     this, SW_RES( FI_WARNING         )),
    aDetailFT(      this, SW_RES( FT_DETAILS         )),
    aDetailED(      this, SW_RES( ED_DETAILS         )),
    aSeparatorFL(   this, SW_RES( FL_SEPARATOR       )),
    aOKPB(          this, SW_RES(PB_OK))
{
    FreeResource();
    aWarningImageIM.SetImage(WarningBox::GetStandardImage());
    aDetailED.SetText(rDetails);
}

SwSendWarningBox_Impl::~SwSendWarningBox_Impl()
{
}

#define ITEMID_TASK     1
#define ITEMID_STATUS   2

SwSendMailDialog::SwSendMailDialog(Window *pParent, SwMailMergeConfigItem& rConfigItem) :
    ModelessDialog /*SfxModalDialog*/(pParent, SW_RES(DLG_MM_SENDMAILS)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aStatusFL( this, SW_RES(             FL_STATUS             )),
    m_aStatusFT( this, SW_RES(             FT_STATUS1            )),
    m_aTransferStatusFL( this, SW_RES(     FL_TRANSFERSTATUS     )),
    m_aTransferStatusFT( this, SW_RES(     FT_TRANSFERSTATUS     )),
    m_PausedFI(this, SW_RES(               FI_PAUSED             )),
    m_aProgressBar( this,      SW_RES(     PB_PROGRESS           )),
    m_aErrorStatusFT( this, SW_RES(        FT_ERRORSTATUS        )),
    m_aDetailsPB( this, SW_RES(            PB_DETAILS            )),
    m_aStatusHB( this, WB_BUTTONSTYLE | WB_BOTTOMBORDER         ),
    m_aStatusLB( this, SW_RES(             LB_STATUS             )),
    m_aSeparatorFL( this, SW_RES(          FL_SEPARATOR          )),
    m_aStopPB( this, SW_RES(               PB_STOP               )),
    m_aClosePB( this, SW_RES(              PB_CLOSE              )),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_sMore(m_aDetailsPB.GetText()),
    m_sLess(SW_RES(ST_LESS)),
    m_sContinue(SW_RES( ST_CONTINUE )),
    m_sStop(m_aStopPB.GetText()),
    m_sSend(SW_RES(ST_SEND)),
    m_sTransferStatus(m_aTransferStatusFT.GetText()),
    m_sErrorStatus(   m_aErrorStatusFT.GetText()),
    m_sSendingTo(   SW_RES(ST_SENDINGTO )),
    m_sCompleted(   SW_RES(ST_COMPLETED )),
    m_sFailed(      SW_RES(ST_FAILED     )),
    m_sTerminateQuery( SW_RES( ST_TERMINATEQUERY )),
    m_bCancel(false),
    m_bDesctructionEnabled(false),
    m_aImageList( SW_RES( ILIST ) ),
    m_pImpl(new SwSendMailDialog_Impl),
    m_pConfigItem(&rConfigItem),
    m_nSendCount(0),
    m_nErrorCount(0)
{
    m_nStatusHeight =  m_aSeparatorFL.GetPosPixel().Y() - m_aStatusLB.GetPosPixel().Y();
    String sTask(SW_RES(ST_TASK));
    String sStatus(SW_RES(ST_STATUS));
    m_aStatusLB.SetHelpId(HID_MM_SENDMAILS_STATUSLB);

    FreeResource();

    m_aDetailsPB.SetClickHdl(LINK( this, SwSendMailDialog, DetailsHdl_Impl));
    m_aStopPB.SetClickHdl(LINK( this, SwSendMailDialog, StopHdl_Impl));
    m_aClosePB.SetClickHdl(LINK( this, SwSendMailDialog, CloseHdl_Impl));


    Size aLBSize(m_aStatusLB.GetSizePixel());
    m_aStatusHB.SetSizePixel(aLBSize);
    Size aHeadSize(m_aStatusHB.CalcWindowSizePixel());
    aHeadSize.Width() = aLBSize.Width();
    m_aStatusHB.SetSizePixel(aHeadSize);
    Point aLBPos(m_aStatusLB.GetPosPixel());
    m_aStatusHB.SetPosPixel(aLBPos);
    aLBPos.Y() += aHeadSize.Height();
    aLBSize.Height() -= aHeadSize.Height();
    m_aStatusLB.SetPosSizePixel(aLBPos, aLBSize);

    Size aSz(m_aStatusHB.GetOutputSizePixel());
    long nPos1 = aSz.Width()/3 * 2;
    long nPos2 = aSz.Width()/3;
    m_aStatusHB.InsertItem( ITEMID_TASK, sTask,
                            nPos1,
                            HIB_LEFT | HIB_VCENTER );
    m_aStatusHB.InsertItem( ITEMID_STATUS, sStatus,
                            nPos2,
                            HIB_LEFT | HIB_VCENTER );
    m_aStatusHB.SetHelpId(HID_MM_ADDRESSLIST_HB );
    m_aStatusHB.Show();

    m_aStatusLB.SetHelpId(HID_MM_MAILSTATUS_TLB);
    static long nTabs[] = {3, 0, nPos1, aSz.Width() };
    m_aStatusLB.SetStyle( m_aStatusLB.GetStyle() | WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP );
    m_aStatusLB.SetSelectionMode( SINGLE_SELECTION );
    m_aStatusLB.SetTabs(&nTabs[0], MAP_PIXEL);
    m_aStatusLB.SetSpaceBetweenEntries(3);

    DetailsHdl_Impl( &m_aDetailsPB );
    UpdateTransferStatus();
}

SwSendMailDialog::~SwSendMailDialog()
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

static void lcl_Move(Control& rCtrl, long nYOffset)
{
    Point aPos(rCtrl.GetPosPixel());
    aPos.Y() += nYOffset;
    rCtrl.SetPosPixel(aPos);
}

IMPL_LINK_NOARG(SwSendMailDialog, DetailsHdl_Impl)
{
    long nMove = 0;
    if(m_aStatusLB.IsVisible())
    {
        m_aStatusLB.Hide();
        m_aStatusHB.Hide();
        nMove = - m_nStatusHeight;
        m_aDetailsPB.SetText(m_sMore);
    }
    else
    {
        m_aStatusLB.Show();
        m_aStatusHB.Show();
        nMove = m_nStatusHeight;
        m_aDetailsPB.SetText(m_sLess);
    }
    lcl_Move(m_aSeparatorFL, nMove);
    lcl_Move(m_aStopPB, nMove);
    lcl_Move(m_aClosePB, nMove);
    Size aDlgSize = GetSizePixel(); aDlgSize.Height() += nMove; SetSizePixel(aDlgSize);

    return 0;
}

IMPL_LINK( SwSendMailDialog, StopHdl_Impl, PushButton*, pButton )
{
    m_bCancel = true;
    if(m_pImpl->xMailDispatcher.is())
    {
        if(m_pImpl->xMailDispatcher->isStarted())
        {
            m_pImpl->xMailDispatcher->stop();
            pButton->SetText(m_sContinue);
            m_PausedFI.Show();
        }
        else
        {
            m_pImpl->xMailDispatcher->start();
            pButton->SetText(m_sStop);
            m_PausedFI.Show(sal_False);
        }
    }
    return 0;
}

IMPL_LINK_NOARG(SwSendMailDialog, CloseHdl_Impl)
{
    ModelessDialog::Show( sal_False );
    return 0;
}

IMPL_STATIC_LINK_NOINSTANCE( SwSendMailDialog, StartSendMails, SwSendMailDialog*, pDialog )
{
    pDialog->SendMails();
    return 0;
}

IMPL_STATIC_LINK( SwSendMailDialog, RemoveThis, Timer*, pTimer )
{
    if( pThis->m_pImpl->xMailDispatcher.is() )
    {
        if(pThis->m_pImpl->xMailDispatcher->isStarted())
            pThis->m_pImpl->xMailDispatcher->stop();
        if(!pThis->m_pImpl->xMailDispatcher->isShutdownRequested())
            pThis->m_pImpl->xMailDispatcher->shutdown();
    }

    if( pThis->m_bDesctructionEnabled &&
            (!pThis->m_pImpl->xMailDispatcher.is() ||
                    !pThis->m_pImpl->xMailDispatcher->isRunning()))
    {
        delete pThis;
    }
    else
    {
        pTimer->Start();
    }
    return 0;
}

IMPL_STATIC_LINK_NOINSTANCE( SwSendMailDialog, StopSendMails, SwSendMailDialog*, pDialog )
{
    if(pDialog->m_pImpl->xMailDispatcher.is() &&
        pDialog->m_pImpl->xMailDispatcher->isStarted())
    {
        pDialog->m_pImpl->xMailDispatcher->stop();
        pDialog->m_aStopPB.SetText(pDialog->m_sContinue);
        pDialog->m_PausedFI.Show();
    }
    return 0;
}

void  SwSendMailDialog::SendMails()
{
    if(!m_pConfigItem)
    {
        OSL_FAIL("config item not set");
        return;
    }
    String sErrorMessage;
    bool bIsLoggedIn = false;
    EnterWait();
    //get a mail server connection
    uno::Reference< mail::XSmtpService > xSmtpServer =
                SwMailMergeHelper::ConnectToSmtpServer( *m_pConfigItem,
                                            m_pImpl->xConnectedInMailService,
                                            aEmptyStr, aEmptyStr, this );
    bIsLoggedIn = xSmtpServer.is() && xSmtpServer->isConnected();
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

            String sMessage = m_sSendingTo;
            String sTmp(pCurrentMailDescriptor->sEMail);
            sTmp += '\t';
            sTmp += m_sFailed;
            sMessage.SearchAndReplaceAscii("%1", sTmp);
            m_aStatusLB.InsertEntry( sMessage, aInsertImg, aInsertImg);
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
            String sTokens( pCurrentMailDescriptor->sCC );
            sal_uInt16 nTokens = comphelper::string::getTokenCount(sTokens, ';');
            xub_StrLen nPos = 0;
            for( sal_uInt16 nToken = 0; nToken < nTokens; ++nToken)
            {
                String sTmp = sTokens.GetToken( 0, ';', nPos);
                if( sTmp.Len() )
                    pMessage->addCcRecipient( sTmp );
            }
        }
        if(!pCurrentMailDescriptor->sBCC.isEmpty())
        {
            String sTokens( pCurrentMailDescriptor->sBCC );
            sal_uInt16 nTokens = comphelper::string::getTokenCount(sTokens, ';');
            xub_StrLen nPos = 0;
            for( sal_uInt16 nToken = 0; nToken < nTokens; ++nToken)
            {
                String sTmp = sTokens.GetToken( 0, ';', nPos);
                if( sTmp.Len() )
                    pMessage->addBccRecipient( sTmp );
            }
        }
        m_pImpl->xMailDispatcher->enqueueMailMessage( xMessage );
        pCurrentMailDescriptor = m_pImpl->GetNextDescriptor();
    }
    UpdateTransferStatus();
}

void SwSendMailDialog::ShowDialog()
{
    Application::PostUserEvent( STATIC_LINK( this, SwSendMailDialog,
                                                StartSendMails ), this );
    ModelessDialog::Show();
}

void  SwSendMailDialog::StateChanged( StateChangedType nStateChange )
{
    ModelessDialog::StateChanged( nStateChange );
    if(STATE_CHANGE_VISIBLE == nStateChange && !IsVisible())
    {
        m_pImpl->aRemoveTimer.SetTimeoutHdl( STATIC_LINK( this, SwSendMailDialog,
                                                    RemoveThis ) );
        m_pImpl->aRemoveTimer.Start();
    }
}

void SwSendMailDialog::DocumentSent( uno::Reference< mail::XMailMessage> xMessage,
                                        bool bResult,
                                        const ::rtl::OUString* pError )
{
    //sending should stop on send errors
    if(pError &&
        m_pImpl->xMailDispatcher.is() && m_pImpl->xMailDispatcher->isStarted())
    {
        Application::PostUserEvent( STATIC_LINK( this, SwSendMailDialog,
                                                    StopSendMails ), this );
    }
    Image aInsertImg = m_aImageList.GetImage( bResult ? FN_FORMULA_APPLY : FN_FORMULA_CANCEL );

    String sMessage = m_sSendingTo;
    String sTmp(xMessage->getRecipients()[0]);
    sTmp += '\t';
    sTmp += bResult ? m_sCompleted : m_sFailed;
    sMessage.SearchAndReplaceAscii("%1", sTmp);
    m_aStatusLB.InsertEntry( sMessage, aInsertImg, aInsertImg);
    ++m_nSendCount;
    if(!bResult)
        ++m_nErrorCount;

    UpdateTransferStatus( );
    if(pError)
    {
        SwSendWarningBox_Impl* pDlg = new SwSendWarningBox_Impl(0, *pError);
        pDlg->Execute();
        delete pDlg;
    }
}

void SwSendMailDialog::UpdateTransferStatus()
{
    String sStatus( m_sTransferStatus );
    sStatus.SearchAndReplaceAscii("%1", String::CreateFromInt32(m_nSendCount) );
    sStatus.SearchAndReplaceAscii("%2", String::CreateFromInt32(m_pImpl->nDocumentCount));
    m_aTransferStatusFT.SetText(sStatus);

    sStatus = m_sErrorStatus;
    sStatus.SearchAndReplaceAscii("%1", String::CreateFromInt32(m_nErrorCount) );
    m_aErrorStatusFT.SetText(sStatus);

    if(m_pImpl->aDescriptors.size())
        m_aProgressBar.SetValue((sal_uInt16)(m_nSendCount * 100 / m_pImpl->aDescriptors.size()));
    else
        m_aProgressBar.SetValue(0);
}

void SwSendMailDialog::AllMailsSent()
{
    m_aStopPB.Enable(sal_False);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
