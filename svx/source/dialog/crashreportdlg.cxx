/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "crashreportdlg.hxx"

#include <desktop/crashreport.hxx>
#include <sfx2/safemode.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/configmgr.hxx>
#include <officecfg/Office/Common.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

IMPL_STATIC_LINK_NOARG(CrashReportDialog, InstallLOKNotifierHdl, void*,
                       vcl::ILibreOfficeKitNotifier*)
{
    return GetpApp();
}

CrashReportDialog::CrashReportDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "svx/ui/crashreportdlg.ui", "CrashReportDialog")
    , mxBtnSend(m_xBuilder->weld_button("btn_send"))
    , mxBtnCancel(m_xBuilder->weld_button("btn_cancel"))
    , mxBtnClose(m_xBuilder->weld_button("btn_close"))
    , mxEditPreUpload(m_xBuilder->weld_label("ed_pre"))
    , mxEditPostUpload(m_xBuilder->weld_label("ed_post"))
    , mxLinkButton(m_xBuilder->weld_link_button("linkbutton"))
    , mxFtBugReport(m_xBuilder->weld_label("ed_bugreport"))
    , mxCBSafeMode(m_xBuilder->weld_check_button("check_safemode"))
    , mxPrivacyPolicyButton(m_xBuilder->weld_link_button("btnPrivacyPolicy"))
{
    maLinkTemplate = mxLinkButton->get_uri();

    auto const offerSafeMode = officecfg::Office::Common::Misc::OfferSafeMode::get();
    mxCBSafeMode->set_visible(offerSafeMode);

    auto nWidth = mxEditPreUpload->get_preferred_size().Width();
    if (offerSafeMode)
    {
        nWidth = std::max(nWidth, mxCBSafeMode->get_size_request().Width());
    }
    mxEditPreUpload->set_size_request(nWidth, -1);
    mxCBSafeMode->set_size_request(nWidth, -1);

    mxBtnSend->connect_clicked(LINK(this, CrashReportDialog, BtnHdl));
    mxBtnCancel->connect_clicked(LINK(this, CrashReportDialog, BtnHdl));
    mxBtnClose->connect_clicked(LINK(this, CrashReportDialog, BtnHdl));

    mxPrivacyPolicyButton->set_uri(
        officecfg::Office::Common::Menus::PrivacyPolicyURL::get()
        + "?type=crashreport&LOvers=" + utl::ConfigManager::getProductVersion()
        + "&LOlocale=" + LanguageTag(utl::ConfigManager::getUILocale()).getBcp47());

    m_xDialog->SetInstallLOKNotifierHdl(LINK(this, CrashReportDialog, InstallLOKNotifierHdl));
}

CrashReportDialog::~CrashReportDialog() {}

short CrashReportDialog::run()
{
    short nRet = GenericDialogController::run();

    // Check whether to go to safe mode
    if (mxCBSafeMode->get_active())
    {
        sfx2::SafeMode::putFlag();
        css::task::OfficeRestartManager::get(comphelper::getProcessComponentContext())
            ->requestRestart(css::uno::Reference<css::task::XInteractionHandler>());
    }
    return nRet;
}

IMPL_LINK(CrashReportDialog, BtnHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == mxBtnSend.get())
    {
        std::string response;
        bool bSuccess = CrashReporter::readSendConfig(response);

        OUString aCrashID = OUString::createFromAscii(response);

        if (bSuccess)
        {
            OUString aProcessedLink
                = maLinkTemplate.replaceAll("%CRASHID", aCrashID.replaceAll("Crash-ID=", ""));

            // vclbuilder seems to replace _ with ~ even in text
            mxLinkButton->set_label(aProcessedLink.replaceAll("~", "_"));
            mxLinkButton->set_uri(aProcessedLink);
        }
        else
        {
            mxEditPostUpload->set_label(aCrashID);
        }

        mxLinkButton->set_visible(bSuccess);

        mxBtnClose->show();
        mxFtBugReport->show();
        mxEditPostUpload->show();
        mxBtnSend->set_sensitive(false);
        mxBtnCancel->set_sensitive(false);
        mxBtnClose->grab_focus();

        mxEditPreUpload->hide();
        mxBtnSend->hide();
        mxBtnCancel->hide();

        m_xDialog->resize_to_request();
    }
    else if (&rBtn == mxBtnCancel.get())
    {
        m_xDialog->response(RET_CLOSE);
    }
    else if (&rBtn == mxBtnClose.get())
    {
        m_xDialog->response(RET_CLOSE);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
