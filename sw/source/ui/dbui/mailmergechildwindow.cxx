/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mailmergechildwindow.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-06 11:04:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif

#include <cmdid.h>
#include <swmodule.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <vcl/svapp.hxx>
#ifndef _SWMAILMERGECHILDWINDOW_HXX
#include <mailmergechildwindow.hxx>
#endif
#ifndef _MAILMERGEOUTPUTPAGE_HXX
#include <mmoutputpage.hxx>
#endif
#ifndef _MMCONFIGITEM_HXX
#include <mmconfigitem.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_MAIL_XSMTPSERVICE_HPP_
#include "com/sun/star/mail/XSmtpService.hpp"
#endif
#ifndef _COM_SUN_STAR_MAIL_MAILSERVICETYPE_HPP_
#include "com/sun/star/mail/MailServiceType.hpp"
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef INCLUDED_MAILDISPATCHER_HXX
#include <maildispatcher.hxx>
#endif
#ifndef INCLUDED_IMAILDSPLISTENER_HXX
#include <imaildsplistener.hxx>
#endif
#ifndef _SWUNOHELPER_HXX
#include <swunohelper.hxx>
#endif
#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#include <vcl/image.hxx>
#include <mailmergechildwindow.hrc>
#include <dbui.hrc>
#include <helpid.h>

using namespace ::com::sun::star;

SFX_IMPL_FLOATINGWINDOW( SwMailMergeChildWindow, FN_MAILMERGE_CHILDWINDOW )
//SFX_IMPL_FLOATINGWINDOW( SwSendMailChildWindow, FN_MAILMERGE_SENDMAIL_CHILDWINDOW )

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwMailMergeChildWindow::SwMailMergeChildWindow( Window* _pParent,
                                USHORT nId,
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

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwMailMergeChildWin::SwMailMergeChildWin( SfxBindings* _pBindings,
                            SfxChildWindow* pChild,
                            Window *pParent) :
    SfxFloatingWindow(_pBindings, pChild, pParent, SW_RES(DLG_MAILMERGECHILD)),
    m_aBackTB(this, SW_RES( TB_BACK ))
{
    m_aBackTB.SetSelectHdl(LINK(this, SwMailMergeChildWin, BackHdl));
    sal_uInt16 nIResId =  GetSettings().GetStyleSettings().GetWindowColor().IsDark() ?
        ILIST_TBX_HC : ILIST_TBX;
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

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwMailMergeChildWin::~SwMailMergeChildWin()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwMailMergeChildWin, BackHdl, ToolBox *, EMPTYARG )
{
    GetBindings().GetDispatcher()->Execute(FN_MAILMERGE_WIZARD, SFX_CALLMODE_ASYNCHRON);
    return 0;
}

/*-- 26.05.2004 10:12:44---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwMailMergeChildWin::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxFloatingWindow::FillInfo(rInfo);
    rInfo.aWinState.Erase();
    rInfo.bVisible = sal_False;
}
/*-- 21.05.2004 14:07:37---------------------------------------------------

  -----------------------------------------------------------------------*/
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

/*-- 23.06.2004 10:19:55---------------------------------------------------

  -----------------------------------------------------------------------*/
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

/*-- 23.06.2004 10:04:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailDispatcherListener_Impl::SwMailDispatcherListener_Impl(SwSendMailDialog& rParentDlg) :
    m_pSendMailDialog(&rParentDlg)
{
}
/*-- 23.06.2004 10:04:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailDispatcherListener_Impl::~SwMailDispatcherListener_Impl()
{
}
/*-- 23.06.2004 10:04:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailDispatcherListener_Impl::started(::rtl::Reference<MailDispatcher> /*xMailDispatcher*/)
{
}
/*-- 23.06.2004 10:04:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailDispatcherListener_Impl::stopped(
                        ::rtl::Reference<MailDispatcher> /*xMailDispatcher*/)
{
}
/*-- 23.06.2004 10:04:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailDispatcherListener_Impl::idle(::rtl::Reference<MailDispatcher> /*xMailDispatcher*/)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    m_pSendMailDialog->AllMailsSent();
}
/*-- 23.06.2004 10:04:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailDispatcherListener_Impl::mailDelivered(
                        ::rtl::Reference<MailDispatcher> /*xMailDispatcher*/,
                        uno::Reference< mail::XMailMessage> xMailMessage)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    m_pSendMailDialog->DocumentSent( xMailMessage, true, 0 );
    DeleteAttachments( xMailMessage );
}
/*-- 23.06.2004 10:04:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailDispatcherListener_Impl::mailDeliveryError(
                ::rtl::Reference<MailDispatcher> /*xMailDispatcher*/,
                uno::Reference< mail::XMailMessage> xMailMessage,
                const rtl::OUString& sErrorMessage)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    m_pSendMailDialog->DocumentSent( xMailMessage, false, &sErrorMessage );
    DeleteAttachments( xMailMessage );
}
/*-- 23.06.2004 12:30:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailDispatcherListener_Impl::DeleteAttachments( uno::Reference< mail::XMailMessage >& /*xMessage*/ )
{
//DBG_ERROR("SwMailDispatcherListener_Impl::DeleteAttachments not implemented")
//    uno::Sequence< mail::MailAttachmentDescriptor > aAttachments = xMessage->getAttachments();
//    for(sal_Int32 nFile = 0; nFile < aAttachments.getLength(); ++nFile)
//        if(aAttachments[nFile].FileUrl.getLength())
//            SWUnoHelper::UCB_DeleteFile( aAttachments[nFile].FileUrl );
}
/*-- 07.07.2004 13:45:51---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 07.07.2004 13:52:41---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 07.07.2004 13:52:41---------------------------------------------------

  -----------------------------------------------------------------------*/
SwSendWarningBox_Impl::~SwSendWarningBox_Impl()
{
}

/*-- 21.05.2004 14:10:39---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    m_aImageListHC( SW_RES( ILIST_HC ) ),
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
    m_aStatusLB.SetWindowBits( WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP );
    m_aStatusLB.SetSelectionMode( SINGLE_SELECTION );
    m_aStatusLB.SetTabs(&nTabs[0], MAP_PIXEL);
    m_aStatusLB.SetSpaceBetweenEntries(3);

    DetailsHdl_Impl( &m_aDetailsPB );
    UpdateTransferStatus();
}
/*-- 21.05.2004 14:10:39---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        catch(const uno::Exception&)
        {
        }
    }
    delete m_pImpl;
}
/*-- 21.05.2004 14:10:40---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 31.01.2005 08:59:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwSendMailDialog::SetDocumentCount( sal_Int32 nAllDocuments )
{
    m_pImpl->nDocumentCount = nAllDocuments;
    UpdateTransferStatus();
}
/*-- 21.05.2004 14:10:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_Move(Control& rCtrl, long nYOffset)
{
    Point aPos(rCtrl.GetPosPixel());
    aPos.Y() += nYOffset;
    rCtrl.SetPosPixel(aPos);
}
/*-- 21.05.2004 14:10:40---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwSendMailDialog, DetailsHdl_Impl, PushButton*, EMPTYARG )
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
/*-- 21.05.2004 14:10:40---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 21.05.2004 14:10:40---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwSendMailDialog, CloseHdl_Impl, PushButton*, EMPTYARG )
{
    ModelessDialog::Show( sal_False );
    return 0;
}
/*-- 14.06.2004 09:48:30---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_STATIC_LINK_NOINSTANCE( SwSendMailDialog, StartSendMails, SwSendMailDialog*, pDialog )
{
    pDialog->SendMails();
    return 0;
}
/*-- 14.06.2004 09:48:30---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 07.07.2004 14:34:05---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 14.06.2004 10:38:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwSendMailDialog::SendMails()
{
    if(!m_pConfigItem)
    {
        DBG_ERROR("config item not set")
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
        DBG_ERROR("create error message")
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
/* -----------------28.01.2005 10:17-----------------

 --------------------------------------------------*/
void  SwSendMailDialog::IterateMails()
{
    const SwMailDescriptor* pCurrentMailDescriptor = m_pImpl->GetNextDescriptor();
    while( pCurrentMailDescriptor )
    {
        if(!SwMailMergeHelper::CheckMailAddress( pCurrentMailDescriptor->sEMail ))
        {
            ImageList& rImgLst = GetSettings().GetStyleSettings().GetWindowColor().IsDark() ?
                                        m_aImageListHC : m_aImageList;
            Image aInsertImg =   rImgLst.GetImage( FN_FORMULA_CANCEL );

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
        SwMailMessage* pMessage = 0;
        uno::Reference< mail::XMailMessage > xMessage = pMessage = new SwMailMessage;
        if(m_pConfigItem->IsMailReplyTo())
            pMessage->setReplyToAddress(m_pConfigItem->GetMailReplyTo());
        pMessage->addRecipient( pCurrentMailDescriptor->sEMail );
        pMessage->SetSenderName( m_pConfigItem->GetMailDisplayName() );
        pMessage->SetSenderAddress( m_pConfigItem->GetMailAddress() );
        if(pCurrentMailDescriptor->sAttachmentURL.getLength())
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
        if(pCurrentMailDescriptor->sCC.getLength())
        {
            String sTokens( pCurrentMailDescriptor->sCC );
            sal_uInt16 nTokens = sTokens.GetTokenCount( ';' );
            xub_StrLen nPos = 0;
            for( sal_uInt16 nToken = 0; nToken < nTokens; ++nToken)
            {
                String sTmp = sTokens.GetToken( 0, ';', nPos);
                if( sTmp.Len() )
                    pMessage->addCcRecipient( sTmp );
            }
        }
        if(pCurrentMailDescriptor->sBCC.getLength())
        {
            String sTokens( pCurrentMailDescriptor->sBCC );
            sal_uInt16 nTokens = sTokens.GetTokenCount( ';' );
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
/*-- 27.08.2004 10:50:17---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwSendMailDialog::ShowDialog()
{
    Application::PostUserEvent( STATIC_LINK( this, SwSendMailDialog,
                                                StartSendMails ), this );
    ModelessDialog::Show();
}
/*-- 27.08.2004 10:50:17---------------------------------------------------

  -----------------------------------------------------------------------*/
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

/*-- 23.06.2004 10:14:22---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    ImageList& rImgLst = GetSettings().GetStyleSettings().GetWindowColor().IsDark() ?
                                m_aImageListHC : m_aImageList;
    Image aInsertImg =   rImgLst.GetImage( bResult ? FN_FORMULA_APPLY : FN_FORMULA_CANCEL );

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
/*-- 23.06.2004 11:25:31---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        m_aProgressBar.SetValue((USHORT)(m_nSendCount * 100 / m_pImpl->aDescriptors.size()));
    else
        m_aProgressBar.SetValue(0);
}
/*-- 23.06.2004 11:18:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwSendMailDialog::AllMailsSent()
{
    m_aStopPB.Enable(sal_False);
}


