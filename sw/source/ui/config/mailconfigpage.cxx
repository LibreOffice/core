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
#include <svtools/svmedit.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
#include <mmconfigitem.hxx>
#include <mailmergehelper.hxx>
#include <cmdid.h>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/mail/MailServiceType.hpp>
#include <com/sun/star/mail/XMailService.hpp>
#include <com/sun/star/mail/MailServiceProvider.hpp>
#include <globals.hrc>
#include <dbui.hrc>
#include <bitmaps.hlst>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::mail;
using namespace ::com::sun::star::beans;

class SwTestAccountSettingsDialog : public SfxModalDialog
{
    VclPtr<VclMultiLineEdit>   m_pErrorsED;

    VclPtr<PushButton>         m_pStopPB;

    VclPtr<FixedText>          m_pEstablish;
    VclPtr<FixedText>          m_pFind;
    VclPtr<FixedText>          m_pResult1;
    VclPtr<FixedText>          m_pResult2;
    VclPtr<FixedImage>         m_pImage1;
    VclPtr<FixedImage>         m_pImage2;

    Image               m_aCompletedImg;
    Image               m_aFailedImg;
    OUString            m_sCompleted;
    OUString            m_sFailed;
    OUString            m_sErrorServer;

    VclPtr<SwMailConfigPage>   m_pParent;

    bool                m_bStop;

    void                Test();
    DECL_LINK(StopHdl, Button*, void);
    DECL_LINK(TestHdl, void*, void);
public:
    explicit SwTestAccountSettingsDialog(SwMailConfigPage* pParent);
    virtual ~SwTestAccountSettingsDialog() override;
    virtual void dispose() override;
};

class SwAuthenticationSettingsDialog : public weld::GenericDialogController
{
    SwMailMergeConfigItem& m_rConfigItem;

    std::unique_ptr<weld::CheckButton> m_xAuthenticationCB;
    std::unique_ptr<weld::RadioButton> m_xSeparateAuthenticationRB;
    std::unique_ptr<weld::RadioButton> m_xSMTPAfterPOPRB;
    std::unique_ptr<weld::Label>       m_xOutgoingServerFT;
    std::unique_ptr<weld::Label>       m_xUserNameFT;
    std::unique_ptr<weld::Entry>       m_xUserNameED;
    std::unique_ptr<weld::Label>       m_xOutPasswordFT;
    std::unique_ptr<weld::Entry>       m_xOutPasswordED;
    std::unique_ptr<weld::Label>       m_xIncomingServerFT;
    std::unique_ptr<weld::Label>       m_xServerFT;
    std::unique_ptr<weld::Entry>       m_xServerED;
    std::unique_ptr<weld::Label>       m_xPortFT;
    std::unique_ptr<weld::SpinButton>  m_xPortNF;
    std::unique_ptr<weld::Label>       m_xProtocolFT;
    std::unique_ptr<weld::RadioButton> m_xPOP3RB;
    std::unique_ptr<weld::RadioButton> m_xIMAPRB;
    std::unique_ptr<weld::Label>       m_xInUsernameFT;
    std::unique_ptr<weld::Entry>       m_xInUsernameED;
    std::unique_ptr<weld::Label>       m_xInPasswordFT;
    std::unique_ptr<weld::Entry>       m_xInPasswordED;
    std::unique_ptr<weld::Button>      m_xOKPB;

    DECL_LINK(OKHdl_Impl, weld::Button&, void);
    DECL_LINK(CheckBoxHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(RadioButtonHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(InServerHdl_Impl, weld::Button&, void);

public:
    SwAuthenticationSettingsDialog(weld::Window* pParent, SwMailMergeConfigItem& rItem);
};

SwMailConfigPage::SwMailConfigPage( vcl::Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, "MailConfigPage" , "modules/swriter/ui/mailconfigpage.ui", &rSet),

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
    m_pSecureCB->SetClickHdl(LINK(this, SwMailConfigPage, SecureHdl));
}

SwMailConfigPage::~SwMailConfigPage()
{
    disposeOnce();
}

void SwMailConfigPage::dispose()
{
    m_pConfigItem.reset();
    m_pDisplayNameED.clear();
    m_pAddressED.clear();
    m_pReplyToCB.clear();
    m_pReplyToFT.clear();
    m_pReplyToED.clear();
    m_pServerED.clear();
    m_pPortNF.clear();
    m_pSecureCB.clear();
    m_pServerAuthenticationPB.clear();
    m_pTestPB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwMailConfigPage::Create( TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<SwMailConfigPage>::Create(pParent.pParent, *rAttrSet);
}

bool SwMailConfigPage::FillItemSet( SfxItemSet* /*rSet*/ )
{
    if(m_pDisplayNameED->IsValueChangedFromSaved())
        m_pConfigItem->SetMailDisplayName(m_pDisplayNameED->GetText());
    if(m_pAddressED->IsValueChangedFromSaved())
        m_pConfigItem->SetMailAddress(m_pAddressED->GetText());
    if( m_pReplyToCB->IsValueChangedFromSaved() )
        m_pConfigItem->SetMailReplyTo(m_pReplyToCB->IsChecked());
    if(m_pReplyToED->IsValueChangedFromSaved())
        m_pConfigItem->SetMailReplyTo(m_pReplyToED->GetText());
    if(m_pServerED->IsValueChangedFromSaved())
        m_pConfigItem->SetMailServer(m_pServerED->GetText());

    m_pConfigItem->SetMailPort(static_cast<sal_Int16>(m_pPortNF->GetValue()));
    m_pConfigItem->SetSecureConnection(m_pSecureCB->IsChecked());

    m_pConfigItem->Commit();
    return true;
}

void SwMailConfigPage::Reset( const SfxItemSet* /*rSet*/ )
{
    m_pDisplayNameED->SetText(m_pConfigItem->GetMailDisplayName());
    m_pAddressED->SetText(m_pConfigItem->GetMailAddress());

    m_pReplyToED->SetText(m_pConfigItem->GetMailReplyTo()) ;
    m_pReplyToCB->Check(m_pConfigItem->IsMailReplyTo());
    m_pReplyToCB->GetClickHdl().Call(m_pReplyToCB.get());

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

IMPL_LINK(SwMailConfigPage, ReplyToHdl, Button*, pBox, void)
{
    bool bEnable = static_cast<CheckBox*>(pBox)->IsChecked();
    m_pReplyToFT->Enable(bEnable);
    m_pReplyToED->Enable(bEnable);
}

IMPL_LINK_NOARG(SwMailConfigPage, AuthenticationHdl, Button*, void)
{
    m_pConfigItem->SetMailAddress(m_pAddressED->GetText());

    SwAuthenticationSettingsDialog aDlg(GetFrameWeld(), *m_pConfigItem);
    aDlg.run();
}

IMPL_LINK_NOARG(SwMailConfigPage, TestHdl, Button*, void)
{
    ScopedVclPtrInstance<SwTestAccountSettingsDialog>(this)->Execute();
}

IMPL_LINK(SwMailConfigPage, SecureHdl, Button*, pBox, void)
{
    bool bEnable = static_cast<CheckBox*>(pBox)->IsChecked();
    m_pConfigItem->SetSecureConnection(bEnable);
    m_pConfigItem->SetMailPort(static_cast<sal_Int16>(m_pPortNF->GetValue()));
    m_pPortNF->SetValue(m_pConfigItem->GetMailPort());
}

SwTestAccountSettingsDialog::SwTestAccountSettingsDialog(SwMailConfigPage* pParent)
    : SfxModalDialog(pParent, "TestMailSettings", "modules/swriter/ui/testmailsettings.ui")
    , m_aCompletedImg(BitmapEx(RID_BMP_FORMULA_APPLY))
    , m_aFailedImg(BitmapEx(RID_BMP_FORMULA_CANCEL))
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

    m_pStopPB->SetClickHdl(LINK(this, SwTestAccountSettingsDialog, StopHdl));

    Application::PostUserEvent( LINK( this, SwTestAccountSettingsDialog, TestHdl ), this, true );
}

SwTestAccountSettingsDialog::~SwTestAccountSettingsDialog()
{
    disposeOnce();
}

void SwTestAccountSettingsDialog::dispose()
{
    m_pErrorsED.clear();
    m_pStopPB.clear();
    m_pEstablish.clear();
    m_pFind.clear();
    m_pResult1.clear();
    m_pResult2.clear();
    m_pImage1.clear();
    m_pImage2.clear();
    m_pParent.clear();
    SfxModalDialog::dispose();
}

IMPL_LINK_NOARG(SwTestAccountSettingsDialog, StopHdl, Button*, void)
{
    m_bStop = true;
}

IMPL_LINK_NOARG(SwTestAccountSettingsDialog, TestHdl, void*, void)
{
    EnterWait();
    Test();
    LeaveWait();
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
                        "Insecure");
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
        OUString aErrorMessage(m_sErrorServer);
        if (!sException.isEmpty())
            aErrorMessage += "\n--\n" + sException;
        m_pErrorsED->SetText(aErrorMessage);
    }
}

SwMailConfigDlg::SwMailConfigDlg(vcl::Window* pParent, SfxItemSet& rSet)
    : SfxSingleTabDialog(pParent, rSet)
{
    // create TabPage
    SetTabPage(SwMailConfigPage::Create(get_content_area(), &rSet));
}

SwAuthenticationSettingsDialog::SwAuthenticationSettingsDialog(
    weld::Window* pParent, SwMailMergeConfigItem& rItem)
    : GenericDialogController(pParent, "modules/swriter/ui/authenticationsettingsdialog.ui", "AuthenticationSettingsDialog")
    , m_rConfigItem(rItem)
    , m_xAuthenticationCB(m_xBuilder->weld_check_button("authentication"))
    , m_xSeparateAuthenticationRB(m_xBuilder->weld_radio_button("separateauthentication"))
    , m_xSMTPAfterPOPRB(m_xBuilder->weld_radio_button("smtpafterpop"))
    , m_xOutgoingServerFT(m_xBuilder->weld_label("label1"))
    , m_xUserNameFT(m_xBuilder->weld_label("username_label"))
    , m_xUserNameED(m_xBuilder->weld_entry("username"))
    , m_xOutPasswordFT(m_xBuilder->weld_label("outpassword_label"))
    , m_xOutPasswordED(m_xBuilder->weld_entry("outpassword"))
    , m_xIncomingServerFT(m_xBuilder->weld_label("label2"))
    , m_xServerFT(m_xBuilder->weld_label("server_label"))
    , m_xServerED(m_xBuilder->weld_entry("server"))
    , m_xPortFT(m_xBuilder->weld_label("port_label"))
    , m_xPortNF(m_xBuilder->weld_spin_button("port"))
    , m_xProtocolFT(m_xBuilder->weld_label("label3"))
    , m_xPOP3RB(m_xBuilder->weld_radio_button("pop3"))
    , m_xIMAPRB(m_xBuilder->weld_radio_button("imap"))
    , m_xInUsernameFT(m_xBuilder->weld_label("inusername_label"))
    , m_xInUsernameED(m_xBuilder->weld_entry("inusername"))
    , m_xInPasswordFT(m_xBuilder->weld_label("inpassword_label"))
    , m_xInPasswordED(m_xBuilder->weld_entry("inpassword"))
    , m_xOKPB(m_xBuilder->weld_button("ok"))
{
    m_xAuthenticationCB->connect_toggled( LINK( this, SwAuthenticationSettingsDialog, CheckBoxHdl_Impl));
    Link<weld::ToggleButton&,void> aRBLink = LINK( this, SwAuthenticationSettingsDialog, RadioButtonHdl_Impl );
    m_xSeparateAuthenticationRB->connect_toggled( aRBLink );
    m_xSMTPAfterPOPRB->connect_toggled( aRBLink );
    m_xOKPB->connect_clicked( LINK( this, SwAuthenticationSettingsDialog, OKHdl_Impl));
    Link<weld::Button&,void> aInServerLink = LINK( this, SwAuthenticationSettingsDialog, InServerHdl_Impl );
    m_xPOP3RB->connect_clicked( aInServerLink );
    m_xIMAPRB->connect_clicked( aInServerLink );

    m_xAuthenticationCB->set_active(m_rConfigItem.IsAuthentication());
    if (m_rConfigItem.IsSMTPAfterPOP())
        m_xSMTPAfterPOPRB->set_active(true);
    else
        m_xSeparateAuthenticationRB->set_active(true);
    m_xUserNameED->set_text(m_rConfigItem.GetMailUserName());
    m_xOutPasswordED->set_text(m_rConfigItem.GetMailPassword());

    m_xServerED->set_text(m_rConfigItem.GetInServerName());
    m_xPortNF->set_value(m_rConfigItem.GetInServerPort());
    if (m_rConfigItem.IsInServerPOP())
        m_xPOP3RB->set_active(true);
    else
        m_xIMAPRB->set_active(true);
    m_xInUsernameED->set_text(m_rConfigItem.GetInServerUserName());
    m_xInPasswordED->set_text(m_rConfigItem.GetInServerPassword());

    CheckBoxHdl_Impl(*m_xAuthenticationCB);
}

IMPL_LINK_NOARG(SwAuthenticationSettingsDialog, OKHdl_Impl, weld::Button&, void)
{
    m_rConfigItem.SetAuthentication( m_xAuthenticationCB->get_active() );
    m_rConfigItem.SetSMTPAfterPOP(m_xSMTPAfterPOPRB->get_active());
    m_rConfigItem.SetMailUserName(m_xUserNameED->get_text());
    m_rConfigItem.SetMailPassword(m_xOutPasswordED->get_text());
    m_rConfigItem.SetInServerName(m_xServerED->get_text());
    m_rConfigItem.SetInServerPort(m_xPortNF->get_value());
    m_rConfigItem.SetInServerPOP(m_xPOP3RB->get_active());
    m_rConfigItem.SetInServerUserName(m_xInUsernameED->get_text());
    m_rConfigItem.SetInServerPassword(m_xInPasswordED->get_text());
    m_xDialog->response(RET_OK);
}

IMPL_LINK( SwAuthenticationSettingsDialog, CheckBoxHdl_Impl, weld::ToggleButton&, rBox, void)
{
    bool bChecked = rBox.get_active();
    m_xSeparateAuthenticationRB->set_sensitive(bChecked);
    m_xSMTPAfterPOPRB->set_sensitive(bChecked);
    RadioButtonHdl_Impl(*m_xSeparateAuthenticationRB);
}

IMPL_LINK_NOARG(SwAuthenticationSettingsDialog, RadioButtonHdl_Impl, weld::ToggleButton&, void)
{
    bool bSeparate = m_xSeparateAuthenticationRB->get_active();
    bool bIsEnabled = m_xSeparateAuthenticationRB->get_sensitive();
    bool bNotSeparate = !bSeparate && bIsEnabled;
    bSeparate &= bIsEnabled;

    if (bSeparate && m_xUserNameED->get_text().isEmpty())
        m_xUserNameED->set_text(m_rConfigItem.GetMailAddress());
    else if (!bSeparate && m_xUserNameED->get_text() == m_rConfigItem.GetMailAddress())
        m_xUserNameED->set_text("");

    if (bNotSeparate && m_xInUsernameED->get_text().isEmpty())
        m_xInUsernameED->set_text(m_rConfigItem.GetMailAddress());
    else if (!bNotSeparate && m_xInUsernameED->get_text() == m_rConfigItem.GetMailAddress())
        m_xInUsernameED->set_text("");

    m_xOutgoingServerFT->set_sensitive(bSeparate);
    m_xUserNameFT->set_sensitive(bSeparate);
    m_xUserNameED->set_sensitive(bSeparate);
    m_xOutPasswordFT->set_sensitive(bSeparate);
    m_xOutPasswordED->set_sensitive(bSeparate);

    m_xIncomingServerFT->set_sensitive(bNotSeparate);
    m_xServerFT->set_sensitive(bNotSeparate);
    m_xServerED->set_sensitive(bNotSeparate);
    m_xPortFT->set_sensitive(bNotSeparate);
    m_xPortNF->set_sensitive(bNotSeparate);
    m_xInUsernameFT->set_sensitive(bNotSeparate);
    m_xInUsernameED->set_sensitive(bNotSeparate);
    m_xProtocolFT->set_sensitive(bNotSeparate);
    m_xPOP3RB->set_sensitive(bNotSeparate);
    m_xIMAPRB->set_sensitive(bNotSeparate);
    m_xInPasswordFT->set_sensitive(bNotSeparate);
    m_xInPasswordED->set_sensitive(bNotSeparate);
}

IMPL_LINK_NOARG( SwAuthenticationSettingsDialog, InServerHdl_Impl, weld::Button&, void)
{
    bool bPOP = m_xPOP3RB->get_active();
    m_rConfigItem.SetInServerPOP(bPOP);
    m_xPortNF->set_value(m_rConfigItem.GetInServerPort());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
