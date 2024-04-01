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

#include <swtypes.hxx>
#include <mailconfigpage.hxx>
#include <mmconfigitem.hxx>
#include <mailmergehelper.hxx>
#include <officecfg/Office/Writer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/mail/MailServiceType.hpp>
#include <com/sun/star/mail/XMailService.hpp>
#include <com/sun/star/mail/MailServiceProvider.hpp>
#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::mail;

class SwTestAccountSettingsDialog : public SfxDialogController
{
    ImplSVEvent*        m_pPostedEvent;
    OUString            m_sCompleted;
    OUString            m_sFailed;
    OUString            m_sErrorServer;
    bool                m_bStop;

    SwMailConfigPage*   m_pParent;

    std::unique_ptr<weld::Button> m_xStopPB;
    std::unique_ptr<weld::TextView> m_xErrorsED;
    std::unique_ptr<weld::Label> m_xResult1;
    std::unique_ptr<weld::Label> m_xResult2;
    std::unique_ptr<weld::Image> m_xImage1;
    std::unique_ptr<weld::Image> m_xImage2;
    std::unique_ptr<weld::Image> m_xImage3;
    std::unique_ptr<weld::Image> m_xImage4;

    void                Test();
    DECL_LINK(StopHdl, weld::Button&, void);
    DECL_LINK(TestHdl, void*, void);
public:
    explicit SwTestAccountSettingsDialog(SwMailConfigPage* pParent);
    virtual ~SwTestAccountSettingsDialog() override;
};

namespace {

class SwAuthenticationSettingsDialog : public SfxDialogController
{
    SwMailMergeConfigItem& m_rConfigItem;

    std::unique_ptr<weld::CheckButton> m_xAuthenticationCB;
    std::unique_ptr<weld::Widget>      m_xAuthenticationImg;
    std::unique_ptr<weld::RadioButton> m_xSeparateAuthenticationRB;
    std::unique_ptr<weld::Widget>      m_xSeparateAuthenticationImg;
    std::unique_ptr<weld::RadioButton> m_xSMTPAfterPOPRB;
    std::unique_ptr<weld::Widget>      m_xSMTPAfterPOPImg;
    std::unique_ptr<weld::Label>       m_xOutgoingServerFT;
    std::unique_ptr<weld::Label>       m_xUserNameFT;
    std::unique_ptr<weld::Entry>       m_xUserNameED;
    std::unique_ptr<weld::Widget>      m_xUserNameImg;
    std::unique_ptr<weld::Label>       m_xOutPasswordFT;
    std::unique_ptr<weld::Entry>       m_xOutPasswordED;
    std::unique_ptr<weld::Label>       m_xIncomingServerFT;
    std::unique_ptr<weld::Label>       m_xServerFT;
    std::unique_ptr<weld::Entry>       m_xServerED;
    std::unique_ptr<weld::Widget>      m_xServerImg;
    std::unique_ptr<weld::Label>       m_xPortFT;
    std::unique_ptr<weld::SpinButton>  m_xPortNF;
    std::unique_ptr<weld::Widget>      m_xPortImg;
    std::unique_ptr<weld::Label>       m_xProtocolFT;
    std::unique_ptr<weld::RadioButton> m_xPOP3RB;
    std::unique_ptr<weld::Widget>      m_xPOP3Img;
    std::unique_ptr<weld::RadioButton> m_xIMAPRB;
    std::unique_ptr<weld::Label>       m_xInUsernameFT;
    std::unique_ptr<weld::Entry>       m_xInUsernameED;
    std::unique_ptr<weld::Widget>      m_xInUsernameImg;
    std::unique_ptr<weld::Label>       m_xInPasswordFT;
    std::unique_ptr<weld::Entry>       m_xInPasswordED;
    std::unique_ptr<weld::Button>      m_xOKPB;

    DECL_LINK(OKHdl_Impl, weld::Button&, void);
    DECL_LINK(CheckBoxHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(RadioButtonHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(InServerHdl_Impl, weld::Toggleable&, void);

public:
    SwAuthenticationSettingsDialog(weld::Window* pParent, SwMailMergeConfigItem& rItem);
};

}

SwMailConfigPage::SwMailConfigPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/mailconfigpage.ui", "MailConfigPage", &rSet)
    , m_pConfigItem(new SwMailMergeConfigItem)
    , m_xDisplayNameED(m_xBuilder->weld_entry("displayname"))
    , m_xDisplayNameImg(m_xBuilder->weld_widget("lockdisplayname"))
    , m_xAddressED(m_xBuilder->weld_entry("address"))
    , m_xAddressImg(m_xBuilder->weld_widget("lockaddress"))
    , m_xReplyToCB(m_xBuilder->weld_check_button("replytocb"))
    , m_xReplyToCBImg(m_xBuilder->weld_widget("lockreplytocb"))
    , m_xReplyToFT(m_xBuilder->weld_label("replyto_label"))
    , m_xReplyToED(m_xBuilder->weld_entry("replyto"))
    , m_xReplyToImg(m_xBuilder->weld_widget("lockreplyto"))
    , m_xServerED(m_xBuilder->weld_entry("server"))
    , m_xServerImg(m_xBuilder->weld_widget("lockserver"))
    , m_xPortNF(m_xBuilder->weld_spin_button("port"))
    , m_xPortImg(m_xBuilder->weld_widget("lockport"))
    , m_xSecureCB(m_xBuilder->weld_check_button("secure"))
    , m_xSecureImg(m_xBuilder->weld_widget("locksecure"))
    , m_xServerAuthenticationPB(m_xBuilder->weld_button("serverauthentication"))
    , m_xTestPB(m_xBuilder->weld_button("test"))
{
    m_xReplyToCB->connect_toggled(LINK(this, SwMailConfigPage, ReplyToHdl));
    m_xServerAuthenticationPB->connect_clicked(LINK(this, SwMailConfigPage, AuthenticationHdl));
    m_xTestPB->connect_clicked(LINK(this, SwMailConfigPage, TestHdl));
    m_xSecureCB->connect_toggled(LINK(this, SwMailConfigPage, SecureHdl));
}

SwMailConfigPage::~SwMailConfigPage()
{
    m_pConfigItem.reset();
}

std::unique_ptr<SfxTabPage> SwMailConfigPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwMailConfigPage>(pPage, pController, *rAttrSet);
}

OUString SwMailConfigPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { "label1", "displayname_label", "address_label", "replyto_label",
                          "label2", "server_label",      "port_label" };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { "replytocb", "secure" };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    OUString buttons[] = { "serverauthentication", "test" };

    for (const auto& btn : buttons)
    {
        if (const auto& pString = m_xBuilder->weld_button(btn))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SwMailConfigPage::FillItemSet( SfxItemSet* /*rSet*/ )
{
    if (m_xDisplayNameED->get_value_changed_from_saved())
        m_pConfigItem->SetMailDisplayName(m_xDisplayNameED->get_text());
    if (m_xAddressED->get_value_changed_from_saved())
        m_pConfigItem->SetMailAddress(m_xAddressED->get_text());
    if (m_xReplyToCB->get_state_changed_from_saved() )
        m_pConfigItem->SetMailReplyTo(m_xReplyToCB->get_active());
    if (m_xReplyToED->get_value_changed_from_saved())
        m_pConfigItem->SetMailReplyTo(m_xReplyToED->get_text());
    if (m_xServerED->get_value_changed_from_saved())
        m_pConfigItem->SetMailServer(m_xServerED->get_text());

    m_pConfigItem->SetMailPort(m_xPortNF->get_value());
    m_pConfigItem->SetSecureConnection(m_xSecureCB->get_active());

    m_pConfigItem->Commit();
    return true;
}

void SwMailConfigPage::Reset( const SfxItemSet* /*rSet*/ )
{
    m_xDisplayNameED->set_text(m_pConfigItem->GetMailDisplayName());
    m_xDisplayNameED->set_sensitive(!officecfg::Office::Writer::MailMergeWizard::MailDisplayName::isReadOnly());
    m_xDisplayNameImg->set_visible(officecfg::Office::Writer::MailMergeWizard::MailDisplayName::isReadOnly());

    m_xAddressED->set_text(m_pConfigItem->GetMailAddress());
    m_xAddressED->set_sensitive(!officecfg::Office::Writer::MailMergeWizard::MailAddress::isReadOnly());
    m_xAddressImg->set_visible(officecfg::Office::Writer::MailMergeWizard::MailAddress::isReadOnly());

    m_xReplyToED->set_text(m_pConfigItem->GetMailReplyTo());
    m_xReplyToED->set_sensitive(!officecfg::Office::Writer::MailMergeWizard::MailReplyTo::isReadOnly());
    m_xReplyToImg->set_visible(officecfg::Office::Writer::MailMergeWizard::MailReplyTo::isReadOnly());

    m_xReplyToCB->set_active(m_pConfigItem->IsMailReplyTo());
    m_xReplyToCB->set_sensitive(!officecfg::Office::Writer::MailMergeWizard::IsMailReplyTo::isReadOnly());
    m_xReplyToCBImg->set_visible(officecfg::Office::Writer::MailMergeWizard::IsMailReplyTo::isReadOnly());

    ReplyToHdl(*m_xReplyToCB);

    m_xServerED->set_text(m_pConfigItem->GetMailServer());
    m_xServerED->set_sensitive(!officecfg::Office::Writer::MailMergeWizard::MailServer::isReadOnly());
    m_xServerImg->set_visible(officecfg::Office::Writer::MailMergeWizard::MailServer::isReadOnly());

    m_xPortNF->set_value(m_pConfigItem->GetMailPort());
    m_xPortNF->set_sensitive(!officecfg::Office::Writer::MailMergeWizard::MailPort::isReadOnly());
    m_xPortImg->set_visible(officecfg::Office::Writer::MailMergeWizard::MailPort::isReadOnly());

    m_xSecureCB->set_active(m_pConfigItem->IsSecureConnection());
    m_xSecureCB->set_sensitive(!officecfg::Office::Writer::MailMergeWizard::IsSecureConnection::isReadOnly());
    m_xSecureImg->set_visible(officecfg::Office::Writer::MailMergeWizard::IsSecureConnection::isReadOnly());

    m_xDisplayNameED->save_value();
    m_xAddressED->save_value();
    m_xReplyToCB->save_state();
    m_xReplyToED->save_value();
    m_xServerED->save_value();
    m_xPortNF->save_value();
    m_xSecureCB->save_state();
}

IMPL_LINK(SwMailConfigPage, ReplyToHdl, weld::Toggleable&, rBox, void)
{
    bool bEnable = rBox.get_active() && !m_xReplyToImg->get_visible();
    m_xReplyToFT->set_sensitive(bEnable);
    m_xReplyToED->set_sensitive(bEnable);
}

IMPL_LINK_NOARG(SwMailConfigPage, AuthenticationHdl, weld::Button&, void)
{
    m_pConfigItem->SetMailAddress(m_xAddressED->get_text());

    SwAuthenticationSettingsDialog aDlg(GetFrameWeld(), *m_pConfigItem);
    aDlg.run();
}

IMPL_LINK_NOARG(SwMailConfigPage, TestHdl, weld::Button&, void)
{
    SwTestAccountSettingsDialog aDlg(this);
    aDlg.run();
}

IMPL_LINK(SwMailConfigPage, SecureHdl, weld::Toggleable&, rBox, void)
{
    bool bEnable = rBox.get_active();
    m_pConfigItem->SetSecureConnection(bEnable);
    m_pConfigItem->SetMailPort(m_xPortNF->get_value());
    m_xPortNF->set_value(m_pConfigItem->GetMailPort());
}

SwTestAccountSettingsDialog::SwTestAccountSettingsDialog(SwMailConfigPage* pParent)
    : SfxDialogController(pParent->GetFrameWeld(), "modules/swriter/ui/testmailsettings.ui", "TestMailSettings")
    , m_bStop(false)
    , m_pParent(pParent)
    , m_xStopPB(m_xBuilder->weld_button("stop"))
    , m_xErrorsED(m_xBuilder->weld_text_view("errors"))
    , m_xResult1(m_xBuilder->weld_label("result1"))
    , m_xResult2(m_xBuilder->weld_label("result2"))
    , m_xImage1(m_xBuilder->weld_image("image1"))
    , m_xImage2(m_xBuilder->weld_image("image2"))
    , m_xImage3(m_xBuilder->weld_image("image3"))
    , m_xImage4(m_xBuilder->weld_image("image4"))
{
    m_xErrorsED->set_size_request(m_xErrorsED->get_approximate_digit_width() * 72,
                                  m_xErrorsED->get_height_rows(8));
    m_sErrorServer = m_xErrorsED->get_text();
    m_xErrorsED->set_text("");
    m_sCompleted = m_xResult1->get_label();
    m_sFailed = m_xResult2->get_label();

    m_xStopPB->connect_clicked(LINK(this, SwTestAccountSettingsDialog, StopHdl));

    m_pPostedEvent = Application::PostUserEvent(LINK(this, SwTestAccountSettingsDialog, TestHdl));
}

SwTestAccountSettingsDialog::~SwTestAccountSettingsDialog()
{
    if (m_pPostedEvent)
    {
        Application::RemoveUserEvent(m_pPostedEvent);
    }
}

IMPL_LINK_NOARG(SwTestAccountSettingsDialog, StopHdl, weld::Button&, void)
{
    m_bStop = true;
}

IMPL_LINK_NOARG(SwTestAccountSettingsDialog, TestHdl, void*, void)
{
    m_pPostedEvent = nullptr;
    weld::WaitObject aWait(m_xDialog.get());
    Test();
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
        uno::Reference<XConnectionListener> xConnectionListener(new SwConnectionListener);

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
                    m_xDialog.get());

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
                    m_xDialog.get());
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
                    m_pParent->m_xServerED->get_text(),
                    m_pParent->m_xPortNF->get_value(),
                    m_pParent->m_xSecureCB->get_active() ? OUString("Ssl") : OUString("Insecure"));
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

    m_xResult1->set_label(bIsServer ? m_sCompleted : m_sFailed);
    m_xImage1->set_visible(!bIsServer);
    m_xImage3->set_visible(bIsServer);

    m_xResult2->set_label(bIsLoggedIn ? m_sCompleted : m_sFailed);
    m_xImage2->set_visible(!bIsLoggedIn);
    m_xImage4->set_visible(bIsLoggedIn);

    if (!bIsServer || !bIsLoggedIn)
    {
        OUString aErrorMessage(m_sErrorServer);
        if (!sException.isEmpty())
            aErrorMessage += "\n--\n" + sException;
        m_xErrorsED->set_text(aErrorMessage);
    }
}

SwMailConfigDlg::SwMailConfigDlg(weld::Window* pParent, const SfxItemSet& rSet)
    : SfxSingleTabDialogController(pParent, &rSet)
{
    // create TabPage
    SetTabPage(SwMailConfigPage::Create(get_content_area(), this, &rSet));
    m_xDialog->set_title(SwResId(STR_MAILCONFIG_DLG_TITLE));
}

SwAuthenticationSettingsDialog::SwAuthenticationSettingsDialog(
    weld::Window* pParent, SwMailMergeConfigItem& rItem)
    : SfxDialogController(pParent, "modules/swriter/ui/authenticationsettingsdialog.ui", "AuthenticationSettingsDialog")
    , m_rConfigItem(rItem)
    , m_xAuthenticationCB(m_xBuilder->weld_check_button("authentication"))
    , m_xAuthenticationImg(m_xBuilder->weld_widget("lockauthentication"))
    , m_xSeparateAuthenticationRB(m_xBuilder->weld_radio_button("separateauthentication"))
    , m_xSeparateAuthenticationImg(m_xBuilder->weld_widget("lockseparaauth"))
    , m_xSMTPAfterPOPRB(m_xBuilder->weld_radio_button("smtpafterpop"))
    , m_xSMTPAfterPOPImg(m_xBuilder->weld_widget("locksmtpafterpop"))
    , m_xOutgoingServerFT(m_xBuilder->weld_label("label1"))
    , m_xUserNameFT(m_xBuilder->weld_label("username_label"))
    , m_xUserNameED(m_xBuilder->weld_entry("username"))
    , m_xUserNameImg(m_xBuilder->weld_widget("lockusername"))
    , m_xOutPasswordFT(m_xBuilder->weld_label("outpassword_label"))
    , m_xOutPasswordED(m_xBuilder->weld_entry("outpassword"))
    , m_xIncomingServerFT(m_xBuilder->weld_label("label2"))
    , m_xServerFT(m_xBuilder->weld_label("server_label"))
    , m_xServerED(m_xBuilder->weld_entry("server"))
    , m_xServerImg(m_xBuilder->weld_widget("lockserver"))
    , m_xPortFT(m_xBuilder->weld_label("port_label"))
    , m_xPortNF(m_xBuilder->weld_spin_button("port"))
    , m_xPortImg(m_xBuilder->weld_widget("lockport"))
    , m_xProtocolFT(m_xBuilder->weld_label("label3"))
    , m_xPOP3RB(m_xBuilder->weld_radio_button("pop3"))
    , m_xPOP3Img(m_xBuilder->weld_widget("lockpop3"))
    , m_xIMAPRB(m_xBuilder->weld_radio_button("imap"))
    , m_xInUsernameFT(m_xBuilder->weld_label("inusername_label"))
    , m_xInUsernameED(m_xBuilder->weld_entry("inusername"))
    , m_xInUsernameImg(m_xBuilder->weld_widget("lockinusername"))
    , m_xInPasswordFT(m_xBuilder->weld_label("inpassword_label"))
    , m_xInPasswordED(m_xBuilder->weld_entry("inpassword"))
    , m_xOKPB(m_xBuilder->weld_button("ok"))
{
    m_xAuthenticationCB->connect_toggled( LINK( this, SwAuthenticationSettingsDialog, CheckBoxHdl_Impl));
    Link<weld::Toggleable&,void> aRBLink = LINK( this, SwAuthenticationSettingsDialog, RadioButtonHdl_Impl );
    m_xSeparateAuthenticationRB->connect_toggled( aRBLink );
    m_xSMTPAfterPOPRB->connect_toggled( aRBLink );
    m_xOKPB->connect_clicked( LINK( this, SwAuthenticationSettingsDialog, OKHdl_Impl));
    m_xPOP3RB->connect_toggled(LINK(this, SwAuthenticationSettingsDialog, InServerHdl_Impl));

    m_xAuthenticationCB->set_active(m_rConfigItem.IsAuthentication());
    m_xAuthenticationCB->set_sensitive(!officecfg::Office::Writer::MailMergeWizard::IsAuthentication::isReadOnly());
    m_xAuthenticationImg->set_visible(officecfg::Office::Writer::MailMergeWizard::IsAuthentication::isReadOnly());
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

IMPL_LINK( SwAuthenticationSettingsDialog, CheckBoxHdl_Impl, weld::Toggleable&, rBox, void)
{
    bool bChecked = rBox.get_active();
    m_xSeparateAuthenticationRB->set_sensitive(bChecked);
    m_xSMTPAfterPOPRB->set_sensitive(bChecked);
    RadioButtonHdl_Impl(*m_xSeparateAuthenticationRB);
}

IMPL_LINK_NOARG(SwAuthenticationSettingsDialog, RadioButtonHdl_Impl, weld::Toggleable&, void)
{
    bool bSeparate = m_xSeparateAuthenticationRB->get_active();
    bool bIsEnabled = m_xSeparateAuthenticationRB->get_sensitive();
    bool bNotSeparate = !bSeparate && bIsEnabled;
    bSeparate &= bIsEnabled;

    bool bReadOnly = officecfg::Office::Writer::MailMergeWizard::IsSMPTAfterPOP::isReadOnly();
    if (bSeparate || bNotSeparate)
    {
        m_xSeparateAuthenticationRB->set_sensitive(!bReadOnly);
        m_xSeparateAuthenticationImg->set_visible(bReadOnly);
        m_xSMTPAfterPOPRB->set_sensitive(!bReadOnly);
        m_xSMTPAfterPOPImg->set_visible(bReadOnly);
    }
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

    bReadOnly = officecfg::Office::Writer::MailMergeWizard::MailUserName::isReadOnly();
    m_xUserNameED->set_sensitive(bSeparate && !bReadOnly);
    m_xUserNameImg->set_visible(bReadOnly);

    m_xOutPasswordFT->set_sensitive(bSeparate);
    m_xOutPasswordED->set_sensitive(bSeparate);

    m_xIncomingServerFT->set_sensitive(bNotSeparate);
    m_xServerFT->set_sensitive(bNotSeparate);

    bReadOnly = officecfg::Office::Writer::MailMergeWizard::InServerName::isReadOnly();
    m_xServerED->set_sensitive(bNotSeparate && !bReadOnly);
    m_xServerImg->set_visible(bReadOnly);

    m_xPortFT->set_sensitive(bNotSeparate);
    bReadOnly = officecfg::Office::Writer::MailMergeWizard::InServerPort::isReadOnly();
    m_xPortNF->set_sensitive(bNotSeparate && !bReadOnly);
    m_xPortImg->set_visible(bReadOnly);

    m_xInUsernameFT->set_sensitive(bNotSeparate);
    bReadOnly = officecfg::Office::Writer::MailMergeWizard::InServerUserName::isReadOnly();
    m_xInUsernameED->set_sensitive(bNotSeparate && !bReadOnly);
    m_xInUsernameImg->set_visible(bReadOnly);
    m_xProtocolFT->set_sensitive(bNotSeparate);

    bReadOnly = officecfg::Office::Writer::MailMergeWizard::InServerIsPOP::isReadOnly();
    m_xPOP3RB->set_sensitive(bNotSeparate && !bReadOnly);
    m_xIMAPRB->set_sensitive(bNotSeparate && !bReadOnly);
    m_xPOP3Img->set_visible(bReadOnly);

    m_xInPasswordFT->set_sensitive(bNotSeparate);
    m_xInPasswordED->set_sensitive(bNotSeparate);
}

IMPL_LINK_NOARG( SwAuthenticationSettingsDialog, InServerHdl_Impl, weld::Toggleable&, void)
{
    bool bPOP = m_xPOP3RB->get_active();
    m_rConfigItem.SetInServerPOP(bPOP);
    m_xPortNF->set_value(m_rConfigItem.GetInServerPort());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
