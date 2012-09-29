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

#include <swtypes.hxx>
#include <mailconfigpage.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
#include <mmconfigitem.hxx>
#include <mailmergehelper.hxx>
#include <cmdid.h>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include "com/sun/star/mail/MailServiceType.hpp"
#include "com/sun/star/mail/XMailService.hpp"
#include "com/sun/star/mail/MailServiceProvider.hpp"
#include <vcl/msgbox.hxx>
#include <globals.hrc>
#include <mailconfigpage.hrc>
#include <config.hrc>
#include <helpid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::mail;
using namespace ::com::sun::star::beans;
using ::rtl::OUString;

class SwTestAccountSettingsDialog : public SfxModalDialog
{
    FixedInfo           m_aInfoFI;

    HeaderBar           m_aStatusHB;
    SvTabListBox        m_aStatusLB;

    FixedInfo           m_aErrorFI;
    MultiLineEdit       m_eErrorsED;

    FixedLine           m_aSeparatorFL;
    PushButton          m_aStopPB;
    CancelButton        m_aCancelPB;
    HelpButton          m_aHelpPB;

    ImageList           m_aImageList;

    String              m_sTask        ;
    String              m_sStatus      ;
    String              m_sEstablish   ;
    String              m_sFindServer  ;
    String              m_sCompleted   ;
    String              m_sFailed      ;
    String              m_sErrorNetwork;
    String              m_sErrorServer ;

    SwMailConfigPage*   m_pParent;

    bool                m_bStop;

    void                Test();
    DECL_LINK(StopHdl, void *);
    DECL_STATIC_LINK(SwTestAccountSettingsDialog, TestHdl, void*);
public:
    SwTestAccountSettingsDialog(SwMailConfigPage* pParent);
    ~SwTestAccountSettingsDialog();
};

class SwAuthenticationSettingsDialog : public SfxModalDialog
{
    CheckBox        m_aAuthenticationCB;

    RadioButton     m_aSeparateAuthenticationRB;
    RadioButton     m_aSMTPAfterPOPRB;

    FixedInfo       m_aOutgoingServerFT;
    FixedText       m_aUserNameFT;
    Edit            m_aUserNameED;
    FixedText       m_aOutPasswordFT;
    Edit            m_aOutPasswordED;

    FixedInfo       m_aIncomingServerFT;
    FixedText       m_aServerFT;
    Edit            m_aServerED;
    FixedText       m_aPortFT;
    NumericField    m_aPortNF;
    FixedText       m_aProtocolFT;
    RadioButton     m_aPOP3RB;
    RadioButton     m_aIMAPRB;
    FixedText       m_aInUsernameFT;
    Edit            m_aInUsernameED;
    FixedText       m_aInPasswordFT;
    Edit            m_aInPasswordED;

    FixedLine       m_aSeparatorFL;

    OKButton        m_aOKPB;
    CancelButton    m_aCancelPB;
    HelpButton      m_aHelpPB;

    SwMailMergeConfigItem& rConfigItem;

    DECL_LINK(OKHdl_Impl, void *);
    DECL_LINK( CheckBoxHdl_Impl, CheckBox*);
    DECL_LINK(RadioButtonHdl_Impl, void *);


public:
    SwAuthenticationSettingsDialog(SwMailConfigPage* pParent, SwMailMergeConfigItem& rItem);
    ~SwAuthenticationSettingsDialog();
};

SwMailConfigPage::SwMailConfigPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, SW_RES(TP_MAILCONFIG), rSet),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aIdentityFL( this, SW_RES(       FL_IDENTITY)),
    m_aDisplayNameFT( this, SW_RES(    FT_DISPLAYNAME)),
    m_aDisplayNameED( this, SW_RES(    ED_DISPLAYNAME)),
    m_aAddressFT( this, SW_RES(        FT_ADDRESS)),
    m_aAddressED( this, SW_RES(        ED_ADDRESS)),
    m_aReplyToCB( this, SW_RES(        CB_REPLYTO)),
    m_aReplyToFT( this, SW_RES(        FT_REPLYTO)),
    m_aReplyToED( this, SW_RES(        ED_REPLYTO)),
    m_aSMTPFL( this, SW_RES(           FL_SMTP)),
    m_aServerFT( this, SW_RES(         FT_SERVER)),
    m_aServerED( this, SW_RES(         ED_SERVER)),
    m_aPortFT( this, SW_RES(           FT_PORT)),
    m_aPortNF( this, SW_RES(           NF_PORT)),
    m_aSecureCB( this, SW_RES(         CB_SECURE)),
    m_aServerAuthenticationPB( this, SW_RES( PB_AUTHENTICATION )),
    m_aSeparatorFL( this,            SW_RES( FL_SEPARATOR      )),
    m_aTestPB( this, SW_RES(           PB_TEST)),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_pConfigItem( new SwMailMergeConfigItem )
{
    FreeResource();
    m_aReplyToCB.SetClickHdl(LINK(this, SwMailConfigPage, ReplyToHdl));
    m_aServerAuthenticationPB.SetClickHdl(LINK(this, SwMailConfigPage, AuthenticationHdl));
    m_aTestPB.SetClickHdl(LINK(this, SwMailConfigPage, TestHdl));
}

SwMailConfigPage::~SwMailConfigPage()
{
    delete m_pConfigItem;
}

SfxTabPage*  SwMailConfigPage::Create( Window* pParent, const SfxItemSet& rAttrSet)
{
    return new SwMailConfigPage(pParent, rAttrSet);
}

sal_Bool SwMailConfigPage::FillItemSet( SfxItemSet& /*rSet*/ )
{
    if(m_aDisplayNameED.GetText() != m_aDisplayNameED.GetSavedValue())
        m_pConfigItem->SetMailDisplayName(m_aDisplayNameED.GetText());
    if(m_aAddressED.GetText() != m_aAddressED.GetSavedValue())
        m_pConfigItem->SetMailAddress(m_aAddressED.GetText());
    String sReplyTo;
    if( m_aReplyToCB.GetSavedValue() != m_aReplyToCB.IsChecked())
        m_pConfigItem->SetMailReplyTo(m_aReplyToCB.IsChecked());
    if(m_aReplyToED.GetText() != m_aReplyToED.GetSavedValue())
        m_pConfigItem->SetMailReplyTo(m_aReplyToED.GetText());
    if(m_aServerED.GetText() != m_aServerED.GetSavedValue())
        m_pConfigItem->SetMailServer(m_aServerED.GetText());

    if(m_aPortNF.IsModified())
        m_pConfigItem->SetMailPort((sal_Int16)m_aPortNF.GetValue());

    m_pConfigItem->SetSecureConnection(m_aSecureCB.IsChecked());

    m_pConfigItem->Commit();
    return sal_True;
}

void SwMailConfigPage::Reset( const SfxItemSet& /*rSet*/ )
{
    m_aDisplayNameED.SetText(m_pConfigItem->GetMailDisplayName());
    m_aAddressED.SetText(m_pConfigItem->GetMailAddress());

    m_aReplyToED.SetText(m_pConfigItem->GetMailReplyTo()) ;
    m_aReplyToCB.Check(m_pConfigItem->IsMailReplyTo());
    m_aReplyToCB.GetClickHdl().Call(&m_aReplyToCB);

    m_aServerED.SetText(m_pConfigItem->GetMailServer());
    m_aPortNF.SetValue(m_pConfigItem->GetMailPort());

    m_aSecureCB.Check(m_pConfigItem->IsSecureConnection());

    m_aDisplayNameED.SaveValue();
    m_aAddressED    .SaveValue();
    m_aReplyToCB    .SaveValue();
    m_aReplyToED    .SaveValue();
    m_aServerED     .SaveValue();
    m_aPortNF       .SaveValue();
    m_aSecureCB     .SaveValue();
}

IMPL_LINK(SwMailConfigPage, ReplyToHdl, CheckBox*, pBox)
{
    sal_Bool bEnable = pBox->IsChecked();
    m_aReplyToFT.Enable(bEnable);
    m_aReplyToED.Enable(bEnable);
    return 0;
}

IMPL_LINK_NOARG(SwMailConfigPage, AuthenticationHdl)
{
    SwAuthenticationSettingsDialog aDlg(this, *m_pConfigItem);
    aDlg.Execute();
    return 0;
}

IMPL_LINK_NOARG(SwMailConfigPage, TestHdl)
{
    SwTestAccountSettingsDialog(this).Execute();
    return 0;
}

SwTestAccountSettingsDialog::SwTestAccountSettingsDialog(SwMailConfigPage* pParent) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_TESTACCOUNTSETTINGS)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aInfoFI( this, SW_RES(         FI_INFO )),
    m_aStatusHB( this, WB_BUTTONSTYLE | WB_BOTTOMBORDER),
    m_aStatusLB( this, SW_RES(       LB_STATUS )),
    m_aErrorFI( this, SW_RES(        FI_ERROR  )),
    m_eErrorsED( this, SW_RES(       ED_ERROR  )),
    m_aSeparatorFL( this, SW_RES(    FL_SEPAPARATOR )),
    m_aStopPB( this, SW_RES(         PB_STOP   )),
    m_aCancelPB( this, SW_RES(       PB_CANCEL )),
    m_aHelpPB( this, SW_RES(         PB_HELP   )),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_aImageList( SW_RES(ILIST) ),
    m_sTask( SW_RES(        ST_TASK          )),
    m_sStatus( SW_RES(      ST_STATUS        )),
    m_sEstablish( SW_RES(   ST_ESTABLISH     )),
    m_sFindServer( SW_RES(  ST_FINDSERVER    )),
    m_sCompleted( SW_RES(   ST_COMPLETED     )),
    m_sFailed( SW_RES(      ST_FAILED        )),
    m_sErrorServer( SW_RES( ST_ERROR_SERVER )),
    m_pParent(pParent),
    m_bStop(false)
{
    FreeResource();
    m_aStopPB.SetClickHdl(LINK(this, SwTestAccountSettingsDialog, StopHdl));

    Size aLBSize(m_aStatusLB.GetOutputSizePixel());
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
    m_aStatusHB.InsertItem( 1, m_sTask,
                            aSz.Width()/2,
                            HIB_LEFT | HIB_VCENTER );
    m_aStatusHB.InsertItem( 2, m_sStatus,
                            aSz.Width()/2,
                            HIB_LEFT | HIB_VCENTER );

    m_aStatusHB.SetHelpId(HID_MM_TESTACCOUNTSETTINGS_HB  );
    m_aStatusHB.Show();

    m_aStatusLB.SetHelpId(HID_MM_TESTACCOUNTSETTINGS_TLB);
    static long nTabs[] = {2, 0, aSz.Width()/2 };
    m_aStatusLB.SetStyle( m_aStatusLB.GetStyle() | WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP );
    m_aStatusLB.SetSelectionMode( SINGLE_SELECTION );
    m_aStatusLB.SetTabs(&nTabs[0], MAP_PIXEL);
    short nEntryHeight = m_aStatusLB.GetEntryHeight();
    m_aStatusLB.SetEntryHeight( nEntryHeight * 15 / 10 );

    Application::PostUserEvent( STATIC_LINK( this, SwTestAccountSettingsDialog, TestHdl ), this );
}

SwTestAccountSettingsDialog::~SwTestAccountSettingsDialog()
{
}

IMPL_LINK_NOARG(SwTestAccountSettingsDialog, StopHdl)
{
    m_bStop = true;
    return 0;
}

IMPL_STATIC_LINK(SwTestAccountSettingsDialog, TestHdl, void*, EMPTYARG)
{
    pThis->EnterWait();
    pThis->Test();
    pThis->LeaveWait();
    return 0;
}

void SwTestAccountSettingsDialog::Test()
{
    uno::Reference<XMultiServiceFactory> rMgr = ::comphelper::getProcessServiceFactory();

    rtl::OUString sException;

    bool bIsLoggedIn = false;
    bool bIsServer = false;
    if (rMgr.is())
    {
        try
        {
            uno::Reference< mail::XMailService > xInMailService;
            uno::Reference< mail::XMailServiceProvider > xMailServiceProvider(
                mail::MailServiceProvider::create(
                    comphelper::getComponentContext(rMgr)));
            uno::Reference< mail::XMailService > xMailService =
                    xMailServiceProvider->create(
                    mail::MailServiceType_SMTP);
            if(m_bStop)
                return;
            uno::Reference<XConnectionListener> xConnectionListener(new SwConnectionListener());

            if(m_pParent->m_pConfigItem->IsAuthentication() &&
                    m_pParent->m_pConfigItem->IsSMTPAfterPOP())
            {
                xInMailService = xMailServiceProvider->create(
                        m_pParent->m_pConfigItem->IsInServerPOP() ?
                            mail::MailServiceType_POP3 : mail::MailServiceType_IMAP);
                if(m_bStop)
                    return;
                //authenticate at the POP or IMAP server first
                uno::Reference<XAuthenticator> xAuthenticator =
                    new SwAuthenticator(
                        m_pParent->m_pConfigItem->GetInServerUserName(),
                        m_pParent->m_pConfigItem->GetInServerPassword(),
                        this);

                xInMailService->addConnectionListener(xConnectionListener);
                //check connection
                uno::Reference< uno::XCurrentContext> xConnectionContext =
                        new SwConnectionContext(
                            m_pParent->m_pConfigItem->GetInServerName(),
                            m_pParent->m_pConfigItem->GetInServerPort(),
                            ::rtl::OUString("Insecure"));
                xInMailService->connect(xConnectionContext, xAuthenticator);
            }
            if(m_bStop)
                return;
            uno::Reference<XAuthenticator> xAuthenticator;
            if(m_pParent->m_pConfigItem->IsAuthentication() &&
                    !m_pParent->m_pConfigItem->IsSMTPAfterPOP() &&
                    !m_pParent->m_pConfigItem->GetMailUserName().isEmpty())
                xAuthenticator =
                    new SwAuthenticator(
                        m_pParent->m_pConfigItem->GetMailUserName(),
                        m_pParent->m_pConfigItem->GetMailPassword(),
                        this);
            else
                xAuthenticator =  new SwAuthenticator();

            xMailService->addConnectionListener(xConnectionListener);
            if(m_bStop)
                return;
            //just to check if the server exists
            xMailService->getSupportedConnectionTypes();
            if(m_bStop)
                return;
            bIsServer = true;
            //check connection
            uno::Reference< uno::XCurrentContext> xConnectionContext =
                    new SwConnectionContext(
                        m_pParent->m_aServerED.GetText(),
                        sal::static_int_cast< sal_Int16, sal_Int64 >(m_pParent->m_aPortNF.GetValue()),
                        m_pParent->m_aSecureCB.IsChecked() ? OUString("Ssl") : OUString("Insecure"));
            xMailService->connect(xConnectionContext, xAuthenticator);
            bIsLoggedIn = xMailService->isConnected();
            if( xInMailService.is() )
                xInMailService->disconnect();
            if( xMailService->isConnected())
                xMailService->disconnect();
        }
        catch (const uno::Exception& e)
        {
            sException = e.Message;
        }
    }

    Image aFailedImg =   m_aImageList.GetImage( FN_FORMULA_CANCEL );
    Image aCompletedImg = m_aImageList.GetImage( FN_FORMULA_APPLY );

    String sTmp(m_sEstablish);
    sTmp += '\t';
    sTmp += bIsServer ? m_sCompleted : m_sFailed;
    m_aStatusLB.InsertEntry(sTmp,
            bIsServer ? aCompletedImg : aFailedImg,
            bIsServer ? aCompletedImg : aFailedImg);

    sTmp = m_sFindServer;
    sTmp += '\t';
    sTmp += bIsLoggedIn ? m_sCompleted : m_sFailed;
    m_aStatusLB.InsertEntry(sTmp,
            bIsLoggedIn ? aCompletedImg : aFailedImg,
            bIsLoggedIn ? aCompletedImg : aFailedImg);

    if(!bIsServer || !bIsLoggedIn )
    {
        rtl::OUStringBuffer aErrorMessage(m_sErrorServer);
        if (!sException.isEmpty())
            aErrorMessage.appendAscii(RTL_CONSTASCII_STRINGPARAM("\n--\n")).append(sException);
        m_eErrorsED.SetText(aErrorMessage.makeStringAndClear());
    }
}

SwMailConfigDlg::SwMailConfigDlg(Window* pParent, SfxItemSet& rSet ) :
    SfxSingleTabDialog(pParent, rSet, 0)
{
    // create TabPage
    SetTabPage(SwMailConfigPage::Create( this, rSet ));
}

SwMailConfigDlg::~SwMailConfigDlg()
{
}

SwAuthenticationSettingsDialog::SwAuthenticationSettingsDialog(
        SwMailConfigPage* pParent, SwMailMergeConfigItem& rItem) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_SERVERAUTHENTICATION)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
        m_aAuthenticationCB( this,          SW_RES( CB_AUTHENTICATION        )),
        m_aSeparateAuthenticationRB( this,  SW_RES( RB_SEP_AUTHENTICATION   )),
        m_aSMTPAfterPOPRB( this,            SW_RES( RB_SMPTAFTERPOP         )),
        m_aOutgoingServerFT( this,          SW_RES( FT_OUTGOINGSERVER        )),
        m_aUserNameFT( this,                SW_RES( FT_USERNAME             )),
        m_aUserNameED( this,                SW_RES( ED_USERNAME             )),
        m_aOutPasswordFT( this,             SW_RES( FT_OUTPASSWORD          )),
        m_aOutPasswordED( this,             SW_RES( ED_OUTPASSWORD          )),
        m_aIncomingServerFT( this,          SW_RES( FT_INCOMINGSERVER       )),
        m_aServerFT( this,                  SW_RES( FT_SERVER               )),
        m_aServerED( this,                  SW_RES( ED_SERVER               )),
        m_aPortFT( this,                    SW_RES( FT_PORT                 )),
        m_aPortNF( this,                    SW_RES( NF_PORT                 )),
        m_aProtocolFT( this,                SW_RES( FT_PROTOCOL      )),
        m_aPOP3RB( this,                    SW_RES( RB_POP3          )),
        m_aIMAPRB( this,                    SW_RES( RB_IMAP          )),
        m_aInUsernameFT( this,              SW_RES( FT_INUSERNAME            )),
        m_aInUsernameED( this,              SW_RES( ED_INUSERNAME           )),
        m_aInPasswordFT( this,              SW_RES( FT_INPASSWORD           )),
        m_aInPasswordED( this,              SW_RES( ED_INPASSWORD           )),
        m_aSeparatorFL( this,               SW_RES( FL_SEPARATOR            )),
        m_aOKPB( this,                      SW_RES( PB_OK                   )),
        m_aCancelPB( this,                  SW_RES( PB_CANCEL               )),
        m_aHelpPB( this,                    SW_RES( PB_HELP                 )),
#ifdef MSC
#pragma warning (default : 4355)
#endif
        rConfigItem( rItem )
{
    FreeResource();

    m_aAuthenticationCB.SetClickHdl( LINK( this, SwAuthenticationSettingsDialog, CheckBoxHdl_Impl));
    Link aRBLink = LINK( this, SwAuthenticationSettingsDialog, RadioButtonHdl_Impl );
    m_aSeparateAuthenticationRB.SetClickHdl( aRBLink );
    m_aSMTPAfterPOPRB.SetClickHdl( aRBLink );
    m_aOKPB.SetClickHdl( LINK( this, SwAuthenticationSettingsDialog, OKHdl_Impl));

    m_aAuthenticationCB.Check( rConfigItem.IsAuthentication() );
    if(rConfigItem.IsSMTPAfterPOP())
        m_aSMTPAfterPOPRB.Check();
    else
        m_aSeparateAuthenticationRB.Check();
    m_aUserNameED.SetText( rConfigItem.GetMailUserName() );
    m_aOutPasswordED.SetText( rConfigItem.GetMailPassword() );

    m_aServerED.SetText( rConfigItem.GetInServerName() );
    m_aPortNF.SetValue( rConfigItem.GetInServerPort() );
    if(rConfigItem.IsInServerPOP())
        m_aPOP3RB.Check();
    else
        m_aIMAPRB.Check();
    m_aInUsernameED.SetText( rConfigItem.GetInServerUserName());
    m_aInPasswordED.SetText( rConfigItem.GetInServerPassword() );

    CheckBoxHdl_Impl( &m_aAuthenticationCB );
}

SwAuthenticationSettingsDialog::~SwAuthenticationSettingsDialog()
{
}

IMPL_LINK_NOARG(SwAuthenticationSettingsDialog, OKHdl_Impl)
{
    rConfigItem.SetAuthentication( m_aAuthenticationCB.IsChecked() );
    rConfigItem.SetSMTPAfterPOP(m_aSMTPAfterPOPRB.IsChecked());
    rConfigItem.SetMailUserName(m_aUserNameED.GetText());
    rConfigItem.SetMailPassword(m_aOutPasswordED.GetText());
    rConfigItem.SetInServerName(m_aServerED.GetText());
    rConfigItem.SetInServerPort(sal::static_int_cast< sal_Int16, sal_Int64 >(m_aPortNF.GetValue( ) ));
    rConfigItem.SetInServerPOP(m_aPOP3RB.IsChecked());
    rConfigItem.SetInServerUserName(m_aInUsernameED.GetText());

    rConfigItem.SetInServerPassword(m_aInPasswordED.GetText());
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK( SwAuthenticationSettingsDialog, CheckBoxHdl_Impl, CheckBox*, pBox)
{
    sal_Bool bChecked = pBox->IsChecked();
    m_aSeparateAuthenticationRB.Enable(bChecked);
    m_aSMTPAfterPOPRB.Enable(bChecked);
    RadioButtonHdl_Impl( 0 );

    return 0;
}

IMPL_LINK_NOARG(SwAuthenticationSettingsDialog, RadioButtonHdl_Impl)
{
    sal_Bool bSeparate = m_aSeparateAuthenticationRB.IsChecked();
    sal_Bool bIsEnabled = m_aSeparateAuthenticationRB.IsEnabled();
    sal_Bool bNotSeparate = !bSeparate & bIsEnabled;
    bSeparate &= bIsEnabled;

    m_aOutgoingServerFT.Enable(bSeparate);
    m_aUserNameFT.Enable(bSeparate);
    m_aUserNameED.Enable(bSeparate);
    m_aOutPasswordFT.Enable(bSeparate);
    m_aOutPasswordED.Enable(bSeparate);

    m_aIncomingServerFT.Enable(bNotSeparate);
    m_aServerFT.Enable(bNotSeparate);
    m_aServerED.Enable(bNotSeparate);
    m_aPortFT.Enable(bNotSeparate);
    m_aPortNF.Enable(bNotSeparate);
    m_aInUsernameFT.Enable(bNotSeparate);
    m_aInUsernameED.Enable(bNotSeparate);
    m_aProtocolFT.Enable(bNotSeparate);
    m_aPOP3RB.Enable(bNotSeparate);
    m_aIMAPRB.Enable(bNotSeparate);
    m_aInPasswordFT.Enable(bNotSeparate);
    m_aInPasswordED.Enable(bNotSeparate);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
