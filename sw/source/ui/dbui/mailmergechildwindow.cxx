/*************************************************************************
 *
 *  $RCSfile: mailmergechildwindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-09-20 13:12:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

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
//#ifndef _COM_SUN_STAR_MAIL_XMAILSERVER_HPP_
//#include "com/sun/star/mail/XMailServer.hpp"
//#endif
#ifndef _COM_SUN_STAR_MAIL_XSMTPSERVICE_HPP_
#include "com/sun/star/mail/XSmtpService.hpp"
#endif
#ifndef _COM_SUN_STAR_MAIL_MAILSERVICETYPE_HPP_
#include "com/sun/star/mail/MailServiceType.hpp"
#endif
#ifndef _COM_SUN_STAR_MAIL_MAILSERVICEPROVIDER_HPP_
#include "com/sun/star/mail/MailServiceProvider.hpp"
#endif
#ifndef _COM_SUN_STAR_MAIL_XMAILSERVICEPROVIDER_HPP_
#include "com/sun/star/mail/XMailServiceProvider.hpp"
#endif
//#ifndef _COM_SUN_STAR_MAIL_MAILSERVER_HPP_
//#include "com/sun/star/mail/MailServer.hpp"
//#endif
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

SwMailMergeChildWindow::SwMailMergeChildWindow( Window* pParent,
                                USHORT nId,
                                SfxBindings* pBindings,
                                SfxChildWinInfo* pInfo ) :
                                SfxChildWindow( pParent, nId )
{
    pWindow = new SwMailMergeChildWin( pBindings, this, pParent);

    if (!pInfo->aSize.Width() || !pInfo->aSize.Height())
    {
        SwView* pActiveView = ::GetActiveView();
        if(pActiveView)
        {
            const SwEditWin &rEditWin = pActiveView->GetEditWin();
            pWindow->SetPosPixel(rEditWin.OutputToScreenPixel(Point(0, 0)));
        }
        else
            pWindow->SetPosPixel(pParent->OutputToScreenPixel(Point(0, 0)));
        pInfo->aPos = pWindow->GetPosPixel();
        pInfo->aSize = pWindow->GetSizePixel();
    }

    ((SwMailMergeChildWin *)pWindow)->Initialize(pInfo);
    pWindow->Show();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwMailMergeChildWin::SwMailMergeChildWin( SfxBindings* pBindings,
                            SfxChildWindow* pChild,
                            Window *pParent) :
    SfxFloatingWindow(pBindings, pChild, pParent, SW_RES(DLG_MAILMERGECHILD)),
    m_aBackTB(this, SW_RES( TB_BACK ))
{
    m_aBackTB.SetSelectHdl(LINK(this, SwMailMergeChildWin, BackHdl));
    sal_uInt16 nIResId =  GetSettings().GetStyleSettings().GetWindowColor().IsDark() ?
        ILIST_TBX_HC : ILIST_TBX;
    ResId aResId( nIResId );
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
/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

/*SwSendMailChildWindow::SwSendMailChildWindow( Window* pParent,
                                USHORT nId,
                                SfxBindings* pBindings,
                                SfxChildWinInfo* pInfo ) :
                                SfxChildWindow( pParent, nId )
{
    pWindow = new SwSendMailDialog( pBindings, this, pParent);
    static_cast<SwSendMailDialog *>(pWindow)->Initialize(pInfo);
    pWindow->Show();
}
/*-- 01.07.2004 08:21:17---------------------------------------------------

  -----------------------------------------------------------------------*/
/*SwSendMailDialog*   SwSendMailChildWindow::GetMailDialog()
{
    return (SwSendMailDialog*)GetWindow();
}

/*-- 26.08.2004 14:40:00---------------------------------------------------

  -----------------------------------------------------------------------*/
/*sal_Bool    SwSendMailChildWindow::QueryClose()
{
    return static_cast<SwSendMailDialog*>(GetWindow())->QueryTermination() &&
            SfxChildWindow::QueryClose();
}
/*-- 21.05.2004 14:07:37---------------------------------------------------

  -----------------------------------------------------------------------*/
struct SwSendMailDialog_Impl
{
    friend class SwSendMailDialog;
    ::std::vector< SwMailDescriptor >           aDescriptors;
    ::rtl::Reference< MailDispatcher >          xMailDispatcher;
    ::rtl::Reference< IMailDispatcherListener>  xMailListener;
    uno::Reference< mail::XMailService >        xConnectedMailService;
    uno::Reference< mail::XMailService >        xConnectedInMailService;

    SwSendMailDialog_Impl() {}

    ~SwSendMailDialog_Impl()
    {
        // Shutdown must be called when the last reference to the
        // mail dispatcher will be released in order to force a
        // shutdown of the mail dispatcher thread.
        // 'join' with the mail dispatcher thread leads to a
        // deadlock (SolarMutex).
        xMailDispatcher->shutdown();
    }
};
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
void SwMailDispatcherListener_Impl::started(::rtl::Reference<MailDispatcher> xMailDispatcher)
{
}
/*-- 23.06.2004 10:04:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailDispatcherListener_Impl::stopped(
                        ::rtl::Reference<MailDispatcher> xMailDispatcher)
{
}
/*-- 23.06.2004 10:04:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailDispatcherListener_Impl::idle(::rtl::Reference<MailDispatcher> xMailDispatcher)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    m_pSendMailDialog->AllMailsSent();
}
/*-- 23.06.2004 10:04:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailDispatcherListener_Impl::mailDelivered(
                        ::rtl::Reference<MailDispatcher> xMailDispatcher,
                        uno::Reference< mail::XMailMessage> xMailMessage)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    m_pSendMailDialog->DocumentSent( xMailMessage, true, 0 );
    DeleteAttachments( xMailMessage );
}
/*-- 23.06.2004 10:04:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailDispatcherListener_Impl::mailDeliveryError(
                ::rtl::Reference<MailDispatcher> xMailDispatcher,
                uno::Reference< mail::XMailMessage> xMailMessage,
                const rtl::OUString& sErrorMessage)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    m_pSendMailDialog->DocumentSent( xMailMessage, false, &sErrorMessage );
    DeleteAttachments( xMailMessage );
}
/*-- 23.06.2004 12:30:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailDispatcherListener_Impl::DeleteAttachments( uno::Reference< mail::XMailMessage >& xMessage )
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
    aWarningImageIM(this, ResId( IM_WARNING         )),
    aWarningFI(     this, ResId( FI_WARNING         )),
    aDetailFT(      this, ResId( FT_DETAILS         )),
    aDetailED(      this, ResId( ED_DETAILS         )),
    aSeparatorFL(   this, ResId( FL_SEPARATOR       )),
    aOKPB(          this, ResId(PB_OK))
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

//SwSendMailDialog::SwSendMailDialog(SfxBindings* pBindings,
//                                SfxChildWindow* pChild, Window *pParent) :
//    SfxFloatingWindow(pBindings, pChild, pParent, SW_RES(DLG_MM_SENDMAILS)),
SwSendMailDialog::SwSendMailDialog(Window *pParent, SwMailMergeConfigItem& rConfigItem) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_SENDMAILS)),
#pragma warning (disable : 4355)
    m_aStatusFL( this, ResId(             FL_STATUS             )),
    m_aStatusFT( this, ResId(             FT_STATUS             )),
    m_PausedFI(this, ResId(               FI_PAUSED             )),
    m_aTransferStatusFL( this, ResId(     FL_TRANSFERSTATUS     )),
    m_aTransferStatusFT( this, ResId(     FT_TRANSFERSTATUS     )),
    m_aProgressBar( this,      ResId(     PB_PROGRESS           )),
    m_aErrorStatusFT( this, ResId(        FT_ERRORSTATUS        )),
    m_aDetailsPB( this, ResId(            PB_DETAILS            )),
    m_aStatusHB( this, WB_BUTTONSTYLE | WB_BOTTOMBORDER         ),
    m_aStatusLB( this, ResId(             LB_STATUS             )),
    m_aSeparatorFL( this, ResId(          FL_SEPARATOR          )),
    m_aStopPB( this, ResId(               PB_STOP               )),
    m_aClosePB( this, ResId(              PB_CLOSE              )),
#pragma warning (default : 4355)
    m_sMore(m_aDetailsPB.GetText()),
    m_sLess(ResId(ST_LESS)),
    m_sContinue(ResId( ST_CONTINUE )),
    m_sStop(m_aStopPB.GetText()),
    m_sSend(ResId(ST_SEND)),
    m_sTransferStatus(m_aTransferStatusFT.GetText()),
    m_sErrorStatus(   m_aErrorStatusFT.GetText()),
    m_sSendingTo(   ResId(ST_SENDINGTO )),
    m_sCompleted(   ResId(ST_COMPLETED )),
    m_sFailed(      ResId(ST_FAILED     )),
    m_sTerminateQuery( ResId( ST_TERMINATEQUERY )),
    m_aImageList( ResId( ILIST ) ),
    m_aImageListHC( ResId( ILIST_HC ) ),
    m_pImpl(new SwSendMailDialog_Impl),
    m_pConfigItem(&rConfigItem),
    m_bCancel(false),
    m_nSendCount(0),
    m_nErrorCount(0)
{
    m_nStatusHeight =  m_aSeparatorFL.GetPosPixel().Y() - m_aStatusLB.GetPosPixel().Y();
    String sTask(ResId(ST_TASK));
    String sStatus(ResId(ST_STATUS));
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
    m_pImpl->aDescriptors.push_back(rDesc);
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
IMPL_LINK( SwSendMailDialog, CloseHdl_Impl, PushButton*, pButton )
{
    // prevent a query for termination
    m_nSendCount = m_pImpl->aDescriptors.size();
    EndDialog( RET_OK );
//    GetBindings().GetDispatcher()->Execute(FN_MAILMERGE_SENDMAIL_CHILDWINDOW,
//        SFX_CALLMODE_ASYNCHRON);

    return 0;
}
/*-- 14.06.2004 09:48:30---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_STATIC_LINK( SwSendMailDialog, StartSendMails, SwSendMailDialog*, pDialog )
{
    pDialog->SendMails();
    return 0;
}
/*-- 07.07.2004 14:34:05---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_STATIC_LINK( SwSendMailDialog, StopSendMails, SwSendMailDialog*, pDialog )
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
    long nSent = 0;
    String sErrorMessage;
    //get a mail server connection
    uno::Reference< lang::XMultiServiceFactory> rMgr = ::comphelper::getProcessServiceFactory();
    bool bIsLoggedIn = false;
    bool bIsServer = false;
    if (rMgr.is())
    {
        EnterWait();
        uno::Reference< mail::XSmtpService > xSmtpServer;
        try
        {
            uno::Reference< mail::XMailServiceProvider > xMailServiceProvider =
                    mail::MailServiceProvider::create(getCurrentCmpCtx(rMgr));
            xSmtpServer = uno::Reference< mail::XSmtpService > (
                            xMailServiceProvider->create(
                            mail::MailServiceType_SMTP
                            ), uno::UNO_QUERY);

            uno::Reference< mail::XConnectionListener> xConnectionListener(new SwConnectionListener());

            if(m_pConfigItem->IsAuthentication() && m_pConfigItem->IsSMTPAfterPOP())
            {
                uno::Reference< mail::XMailService > xInMailService =
                        xMailServiceProvider->create(
                        m_pConfigItem->IsInServerPOP() ?
                            mail::MailServiceType_POP3 : mail::MailServiceType_IMAP);
                //authenticate at the POP or IMAP server first
                uno::Reference<mail::XAuthenticator> xAuthenticator =
                    new SwAuthenticator(
                        m_pConfigItem->GetInServerUserName(),
                        m_pConfigItem->GetInServerPassword(),
                        this);

                xInMailService->addConnectionListener(xConnectionListener);
                //check connection
                uno::Reference< uno::XCurrentContext> xConnectionContext =
                        new SwConnectionContext(
                            m_pConfigItem->GetInServerName(),
                            m_pConfigItem->GetInServerPort(),
                            ::rtl::OUString::createFromAscii( "Insecure" ));
                xInMailService->connect(xConnectionContext, xAuthenticator);
                m_pImpl->xConnectedInMailService = xInMailService;
            }
            uno::Reference< mail::XAuthenticator> xAuthenticator;
            if(m_pConfigItem->IsAuthentication() &&
                    !m_pConfigItem->IsSMTPAfterPOP() &&
                    m_pConfigItem->GetMailUserName().getLength())
                xAuthenticator =
                    new SwAuthenticator(m_pConfigItem->GetMailUserName(),
                                        m_pConfigItem->GetMailPassword(),
                                        this);
            else
                xAuthenticator =  new SwAuthenticator();
            //just to check if the server exists
            xSmtpServer->getSupportedConnectionTypes();
            bIsServer = true;
            //check connection

            uno::Reference< uno::XCurrentContext> xConnectionContext =
                    new SwConnectionContext(
                        m_pConfigItem->GetMailServer(),
                        m_pConfigItem->GetMailPort(),
                        ::rtl::OUString::createFromAscii( m_pConfigItem->IsSecureConnection() ? "Ssl" : "Insecure"));
            xSmtpServer->connect(xConnectionContext, xAuthenticator);
            m_pImpl->xConnectedMailService = uno::Reference< mail::XMailService >( xSmtpServer, uno::UNO_QUERY );

            bIsLoggedIn = xSmtpServer->isConnected();
        }
        catch(uno::Exception& rEx)
        {
            sErrorMessage = rEx.Message;
            DBG_ERROR("exception caught")
        }
        LeaveWait();
        if(!bIsLoggedIn)
        {
            //error message??
            //sErrorMessage
            DBG_ERROR("create error message")
            return;
        }
        m_pImpl->xMailDispatcher.set( new MailDispatcher(xSmtpServer));
        ::std::vector< SwMailDescriptor >::iterator aCurrentMailDescriptor;
        for( aCurrentMailDescriptor = m_pImpl->aDescriptors.begin();
                aCurrentMailDescriptor != m_pImpl->aDescriptors.end();
                        ++aCurrentMailDescriptor)
        {
            if(!SwMailMergeHelper::CheckMailAddress( aCurrentMailDescriptor->sEMail ))
            {
                ImageList& rImgLst = GetSettings().GetStyleSettings().GetWindowColor().IsDark() ?
                                            m_aImageListHC : m_aImageList;
                Image aInsertImg =   rImgLst.GetImage( FN_FORMULA_CANCEL );

                String sMessage = m_sSendingTo;
                String sTmp(aCurrentMailDescriptor->sEMail);
                sTmp += '\t';
                sTmp += m_sFailed;
                sMessage.SearchAndReplaceAscii("%1", sTmp);
                m_aStatusLB.InsertEntry( sMessage, aInsertImg, aInsertImg);
                ++m_nSendCount;
                ++m_nErrorCount;
                UpdateTransferStatus( );
                continue;
            }
            SwMailMessage* pMessage = 0;
            uno::Reference< mail::XMailMessage > xMessage = pMessage = new SwMailMessage;
            if(m_pConfigItem->IsMailReplyTo())
                pMessage->setReplyToAddress(m_pConfigItem->GetMailReplyTo());
            pMessage->addRecipient( aCurrentMailDescriptor->sEMail );
            pMessage->SetSenderAddress( m_pConfigItem->GetMailAddress() );
            if(aCurrentMailDescriptor->sAttachmentURL.getLength())
            {
                mail::MailAttachment aAttach;
                aAttach.Data =
                        new SwMailTransferable(
                            aCurrentMailDescriptor->sAttachmentURL,
                            aCurrentMailDescriptor->sAttachmentName,
                            aCurrentMailDescriptor->sMimeType );
                aAttach.ReadableName = aCurrentMailDescriptor->sAttachmentName;
                pMessage->addAttachment( aAttach );
            }
            pMessage->setSubject( aCurrentMailDescriptor->sSubject );
            uno::Reference< datatransfer::XTransferable> xBody =
                        new SwMailTransferable(
                            aCurrentMailDescriptor->sBodyContent,
                            aCurrentMailDescriptor->sBodyMimeType);
            pMessage->setBody( xBody );

            //CC and BCC are tokenized by ';'
            if(aCurrentMailDescriptor->sCC.getLength())
            {
                String sTokens( aCurrentMailDescriptor->sCC );
                sal_uInt16 nTokens = sTokens.GetTokenCount( ';' );
                xub_StrLen nPos = 0;
                for( sal_uInt16 nToken = 0; nToken < nTokens; ++nToken)
                {
                    String sTmp = sTokens.GetToken( 0, ';', nPos);
                    if( sTmp.Len() )
                        pMessage->addCcRecipient( sTmp );
                }
            }
            if(aCurrentMailDescriptor->sBCC.getLength())
            {
                String sTokens( aCurrentMailDescriptor->sBCC );
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
        }
        m_pImpl->aDescriptors.clear();
        UpdateTransferStatus();
        m_pImpl->xMailListener = new SwMailDispatcherListener_Impl(*this);
        m_pImpl->xMailDispatcher->addListener(m_pImpl->xMailListener);
        if(!m_bCancel)
        {
            m_pImpl->xMailDispatcher->start();
        }
    }
}
/*-- 27.08.2004 10:50:17---------------------------------------------------

  -----------------------------------------------------------------------*/
short SwSendMailDialog::Execute()
{
    Application::PostUserEvent( STATIC_LINK( this, SwSendMailDialog,
                                                StartSendMails ), this );
    return SfxModalDialog::Execute();
}

/*-- 14.06.2004 09:50:18---------------------------------------------------

  -----------------------------------------------------------------------*/
/*void SwSendMailDialog::Start()
{
    Application::PostUserEvent( STATIC_LINK( this, SwSendMailDialog,
                                                StartSendMails ), this );
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
    sStatus.SearchAndReplaceAscii("%2", String::CreateFromInt32(m_pImpl->aDescriptors.size()));
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
/*-- 01.07.2004 08:16:26---------------------------------------------------

  -----------------------------------------------------------------------*/
/*void    SwSendMailDialog::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxFloatingWindow::FillInfo(rInfo);
    rInfo.bVisible = sal_False;
}
/*-- 18.08.2004 12:54:30---------------------------------------------------

  -----------------------------------------------------------------------*/
/*sal_Bool SwSendMailDialog::QueryTermination()
{
    bool bRet = sal_True;
    if(m_nSendCount < m_pImpl->aDescriptors.size())
    {
        QueryBox aQuery(this, WB_YES_NO, m_sTerminateQuery);
        USHORT nRet = aQuery.Execute();
        bRet = nRet == RET_YES;
    }
    return bRet;
}*/


