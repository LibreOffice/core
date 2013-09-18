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

#include <swmodule.hxx>
#include <swtypes.hxx>
#include <mailconfigpage.hxx>
#include <sfx2/imgmgr.hxx>
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
#include <config.hrc>
#include <helpid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::mail;
using namespace ::com::sun::star::beans;

class SwTestAccountSettingsDialog : public SfxModalDialog
{
    VclMultiLineEdit*   m_pErrorsED;

    PushButton*         m_pStopPB;

    FixedText*          m_pEstablish;
    FixedText*          m_pFind;
    FixedText*          m_pResult1;
    FixedText*          m_pResult2;
    FixedImage*         m_pImage1;
    FixedImage*         m_pImage2;

    Image               m_aCompletedImg;
    Image               m_aFailedImg;
    OUString            m_sCompleted;
    OUString            m_sFailed;
    OUString            m_sErrorServer;

    SwMailConfigPage*   m_pParent;

    bool                m_bStop;

    void                Test();
    DECL_LINK(StopHdl, void *);
    DECL_STATIC_LINK(SwTestAccountSettingsDialog, TestHdl, void*);
public:
    SwTestAccountSettingsDialog(SwMailConfigPage* pParent);
};

class SwAuthenticationSettingsDialog : public SfxModalDialog
{
    CheckBox*        m_pAuthenticationCB;

    RadioButton*     m_pSeparateAuthenticationRB;
    RadioButton*     m_pSMTPAfterPOPRB;

    FixedText*       m_pOutgoingServerFT;
    FixedText*       m_pUserNameFT;
    Edit*            m_pUserNameED;
    FixedText*       m_pOutPasswordFT;
    Edit*            m_pOutPasswordED;

    FixedText*       m_pIncomingServerFT;
    FixedText*       m_pServerFT;
    Edit*            m_pServerED;
    FixedText*       m_pPortFT;
    NumericField*    m_pPortNF;
    FixedText*       m_pProtocolFT;
    RadioButton*     m_pPOP3RB;
    RadioButton*     m_pIMAPRB;
    FixedText*       m_pInUsernameFT;
    Edit*            m_pInUsernameED;
    FixedText*       m_pInPasswordFT;
    Edit*            m_pInPasswordED;

    OKButton*        m_pOKPB;

    SwMailMergeConfigItem& rConfigItem;

    DECL_LINK(OKHdl_Impl, void *);
    DECL_LINK( CheckBoxHdl_Impl, CheckBox*);
    DECL_LINK(RadioButtonHdl_Impl, void *);


public:
    SwAuthenticationSettingsDialog(SwMailConfigPage* pParent, SwMailMergeConfigItem& rItem);
    ~SwAuthenticationSettingsDialog();
};

SwMailConfigPage::SwMailConfigPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, "MailConfigPage" , "modules/swriter/ui/mailconfigpage.ui", rSet),

    m_pConfigItem( new SwMailMergeConfigItem )
{
    get(m_pDisplayNameED,"displayname");
    get(m_pAddressED,"address");
    get(m_pReplyToCB,"replytocb");
    get(m_pReplyToFT,"replyto_label");
    get(m_pReplyToED,"replyto");
    get(m_pServerED,"server");
    get(m_pPortNF,"port");
    get(m_pSecureCB,"secure");
    get(m_pServerAuthenticationPB,"serverauthentication");
    get(m_pTestPB,"test");

    m_pReplyToCB->SetClickHdl(LINK(this, SwMailConfigPage, ReplyToHdl));
    m_pServerAuthenticationPB->SetClickHdl(LINK(this, SwMailConfigPage, AuthenticationHdl));
    m_pTestPB->SetClickHdl(LINK(this, SwMailConfigPage, TestHdl));
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
    if(m_pDisplayNameED->GetText() != m_pDisplayNameED->GetSavedValue())
        m_pConfigItem->SetMailDisplayName(m_pDisplayNameED->GetText());
    if(m_pAddressED->GetText() != m_pAddressED->GetSavedValue())
        m_pConfigItem->SetMailAddress(m_pAddressED->GetText());
    if( m_pReplyToCB->GetSavedValue() != m_pReplyToCB->IsChecked())
        m_pConfigItem->SetMailReplyTo(m_pReplyToCB->IsChecked());
    if(m_pReplyToED->GetText() != m_pReplyToED->GetSavedValue())
        m_pConfigItem->SetMailReplyTo(m_pReplyToED->GetText());
    if(m_pServerED->GetText() != m_pServerED->GetSavedValue())
        m_pConfigItem->SetMailServer(m_pServerED->GetText());

    if(m_pPortNF->IsModified())
        m_pConfigItem->SetMailPort((sal_Int16)m_pPortNF->GetValue());

    m_pConfigItem->SetSecureConnection(m_pSecureCB->IsChecked());

    m_pConfigItem->Commit();
    return sal_True;
}

void SwMailConfigPage::Reset( const SfxItemSet& /*rSet*/ )
{
    m_pDisplayNameED->SetText(m_pConfigItem->GetMailDisplayName());
    m_pAddressED->SetText(m_pConfigItem->GetMailAddress());

    m_pReplyToED->SetText(m_pConfigItem->GetMailReplyTo()) ;
    m_pReplyToCB->Check(m_pConfigItem->IsMailReplyTo());
    m_pReplyToCB->GetClickHdl().Call(&m_pReplyToCB);

    m_pServerED->SetText(m_pConfigItem->GetMailServer());
    m_pPortNF->SetValue(m_pConfigItem->GetMailPort());

    m_pSecureCB->Check(m_pConfigItem->IsSecureConnection());

    m_pDisplayNameED->SaveValue();
    m_pAddressED    ->SaveValue();
    m_pReplyToCB    ->SaveValue();
    m_pReplyToED    ->SaveValue();
    m_pServerED     ->SaveValue();
    m_pPortNF       ->SaveValue();
    m_pSecureCB     ->SaveValue();
}

IMPL_LINK(SwMailConfigPage, ReplyToHdl, CheckBox*, pBox)
{
    sal_Bool bEnable = pBox->IsChecked();
    m_pReplyToFT->Enable(bEnable);
    m_pReplyToED->Enable(bEnable);
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

SwTestAccountSettingsDialog::SwTestAccountSettingsDialog(SwMailConfigPage* pParent)
    : SfxModalDialog(pParent, "TestMailSettings", "modules/swriter/ui/testmailsettings.ui")
    , m_pParent(pParent)
    , m_bStop(false)
{
    get(m_pStopPB, "stop");
    get(m_pErrorsED, "errors");
    m_pErrorsED->SetMaxTextWidth(80 * m_pErrorsED->approximate_char_width());
    m_pErrorsED->set_height_request(8 * m_pErrorsED->GetTextHeight());
    m_sErrorServer = m_pErrorsED->GetText();
    m_pErrorsED->SetText("");
    get(m_pEstablish, "establish");
    get(m_pFind, "find");
    get(m_pImage1, "image1");
    get(m_pResult1, "result1");
    get(m_pImage2, "image2");
    get(m_pResult2, "result2");
    m_sCompleted = m_pResult1->GetText();
    m_sFailed = m_pResult2->GetText();


    SfxImageManager* pManager = SfxImageManager::GetImageManager( SW_MOD() );
    m_aFailedImg = pManager->GetImage(FN_FORMULA_CANCEL);
    m_aCompletedImg = pManager->GetImage(FN_FORMULA_APPLY);

    m_pStopPB->SetClickHdl(LINK(this, SwTestAccountSettingsDialog, StopHdl));

    Application::PostUserEvent( STATIC_LINK( this, SwTestAccountSettingsDialog, TestHdl ), this );
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
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();

    OUString sException;

    bool bIsLoggedIn = false;
    bool bIsServer = false;
    try
    {
        uno::Reference< mail::XMailService > xInMailService;
        uno::Reference< mail::XMailServiceProvider > xMailServiceProvider(
            mail::MailServiceProvider::create(xContext) );
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
                        OUString("Insecure"));
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
                    m_pParent->m_pServerED->GetText(),
                    sal::static_int_cast< sal_Int16, sal_Int64 >(m_pParent->m_pPortNF->GetValue()),
                    m_pParent->m_pSecureCB->IsChecked() ? OUString("Ssl") : OUString("Insecure"));
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

    m_pResult1->SetText(bIsServer ? m_sCompleted : m_sFailed);
    m_pImage1->SetImage(bIsServer ? m_aCompletedImg : m_aFailedImg);

    m_pResult2->SetText(bIsLoggedIn ? m_sCompleted : m_sFailed);
    m_pImage2->SetImage(bIsLoggedIn ? m_aCompletedImg : m_aFailedImg);

    if (!bIsServer || !bIsLoggedIn)
    {
        OUStringBuffer aErrorMessage(m_sErrorServer);
        if (!sException.isEmpty())
            aErrorMessage.append("\n--\n").append(sException);
        m_pErrorsED->SetText(aErrorMessage.makeStringAndClear());
    }
}

SwMailConfigDlg::SwMailConfigDlg(Window* pParent, SfxItemSet& rSet)
    : SfxSingleTabDialog(pParent, rSet)
{
    // create TabPage
    setTabPage(SwMailConfigPage::Create(get_content_area(), rSet));
}

SwAuthenticationSettingsDialog::SwAuthenticationSettingsDialog(
    SwMailConfigPage* pParent, SwMailMergeConfigItem& rItem)
    : SfxModalDialog(pParent, "AuthenticationSettingsDialog", "modules/swriter/ui/authenticationsettingsdialog.ui")
    , rConfigItem( rItem )
{
    get(m_pAuthenticationCB,"authentication");
    get(m_pSeparateAuthenticationRB,"seperateauthentication");
    get(m_pSMTPAfterPOPRB,"smtpafterpop");
    get(m_pOutgoingServerFT,"label1");
    get(m_pUserNameFT,"username_label");
    get(m_pUserNameED,"username");
    get(m_pOutPasswordFT,"outpassword_label");
    get(m_pOutPasswordED,"outpassword");
    get(m_pIncomingServerFT,"label2");
    get(m_pServerFT,"server_label");
    get(m_pServerED,"server");
    get(m_pPortFT,"port_label");
    get(m_pPortNF,"port");
    get(m_pProtocolFT,"label3");
    get(m_pPOP3RB,"pop3");
    get(m_pIMAPRB,"imap");
    get(m_pInUsernameFT,"inusername_label");
    get(m_pInUsernameED,"inusername");
    get(m_pInPasswordFT,"inpassword_label");
    get(m_pInPasswordED,"inpassword");

    get(m_pOKPB,"ok");

    m_pAuthenticationCB->SetClickHdl( LINK( this, SwAuthenticationSettingsDialog, CheckBoxHdl_Impl));
    Link aRBLink = LINK( this, SwAuthenticationSettingsDialog, RadioButtonHdl_Impl );
    m_pSeparateAuthenticationRB->SetClickHdl( aRBLink );
    m_pSMTPAfterPOPRB->SetClickHdl( aRBLink );
    m_pOKPB->SetClickHdl( LINK( this, SwAuthenticationSettingsDialog, OKHdl_Impl));

    m_pAuthenticationCB->Check( rConfigItem.IsAuthentication() );
    if(rConfigItem.IsSMTPAfterPOP())
        m_pSMTPAfterPOPRB->Check();
    else
        m_pSeparateAuthenticationRB->Check();
    m_pUserNameED->SetText( rConfigItem.GetMailUserName() );
    m_pOutPasswordED->SetText( rConfigItem.GetMailPassword() );

    m_pServerED->SetText( rConfigItem.GetInServerName() );
    m_pPortNF->SetValue( rConfigItem.GetInServerPort() );
    if(rConfigItem.IsInServerPOP())
        m_pPOP3RB->Check();
    else
        m_pIMAPRB->Check();
    m_pInUsernameED->SetText( rConfigItem.GetInServerUserName());
    m_pInPasswordED->SetText( rConfigItem.GetInServerPassword() );

    CheckBoxHdl_Impl( m_pAuthenticationCB );
}

SwAuthenticationSettingsDialog::~SwAuthenticationSettingsDialog()
{
}

IMPL_LINK_NOARG(SwAuthenticationSettingsDialog, OKHdl_Impl)
{
    rConfigItem.SetAuthentication( m_pAuthenticationCB->IsChecked() );
    rConfigItem.SetSMTPAfterPOP(m_pSMTPAfterPOPRB->IsChecked());
    rConfigItem.SetMailUserName(m_pUserNameED->GetText());
    rConfigItem.SetMailPassword(m_pOutPasswordED->GetText());
    rConfigItem.SetInServerName(m_pServerED->GetText());
    rConfigItem.SetInServerPort(sal::static_int_cast< sal_Int16, sal_Int64 >(m_pPortNF->GetValue( ) ));
    rConfigItem.SetInServerPOP(m_pPOP3RB->IsChecked());
    rConfigItem.SetInServerUserName(m_pInUsernameED->GetText());

    rConfigItem.SetInServerPassword(m_pInPasswordED->GetText());
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK( SwAuthenticationSettingsDialog, CheckBoxHdl_Impl, CheckBox*, pBox)
{
    sal_Bool bChecked = pBox->IsChecked();
    m_pSeparateAuthenticationRB->Enable(bChecked);
    m_pSMTPAfterPOPRB->Enable(bChecked);
    RadioButtonHdl_Impl( 0 );

    return 0;
}

IMPL_LINK_NOARG(SwAuthenticationSettingsDialog, RadioButtonHdl_Impl)
{
    sal_Bool bSeparate = m_pSeparateAuthenticationRB->IsChecked();
    sal_Bool bIsEnabled = m_pSeparateAuthenticationRB->IsEnabled();
    sal_Bool bNotSeparate = !bSeparate && bIsEnabled;
    bSeparate &= bIsEnabled;

    m_pOutgoingServerFT->Enable(bSeparate);
    m_pUserNameFT->Enable(bSeparate);
    m_pUserNameED->Enable(bSeparate);
    m_pOutPasswordFT->Enable(bSeparate);
    m_pOutPasswordED->Enable(bSeparate);

    m_pIncomingServerFT->Enable(bNotSeparate);
    m_pServerFT->Enable(bNotSeparate);
    m_pServerED->Enable(bNotSeparate);
    m_pPortFT->Enable(bNotSeparate);
    m_pPortNF->Enable(bNotSeparate);
    m_pInUsernameFT->Enable(bNotSeparate);
    m_pInUsernameED->Enable(bNotSeparate);
    m_pProtocolFT->Enable(bNotSeparate);
    m_pPOP3RB->Enable(bNotSeparate);
    m_pIMAPRB->Enable(bNotSeparate);
    m_pInPasswordFT->Enable(bNotSeparate);
    m_pInPasswordED->Enable(bNotSeparate);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
