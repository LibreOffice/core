/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "crashreportdlg.hxx"


#include <config_folders.h>

#include <i18nlangtag/languagetag.hxx>
#include <rtl/bootstrap.hxx>
#include <desktop/crashreport.hxx>
#include <sfx2/safemode.hxx>
#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>
#include <unotools/configmgr.hxx>

#include <com/sun/star/configuration/ReadOnlyAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

CrashReportDialog::CrashReportDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "svx/ui/crashreportdlg.ui",
                            "CrashReportDialog")
    , mxBtnSend(m_xBuilder->weld_button("btn_send"))
    , mxBtnCancel(m_xBuilder->weld_button("btn_cancel"))
    , mxBtnClose(m_xBuilder->weld_button("btn_close"))
    , mxEditPreUpload(m_xBuilder->weld_label("ed_pre"))
    , mxEditPostUpload(m_xBuilder->weld_text_view("ed_post"))
    , mxCBSafeMode(m_xBuilder->weld_check_button("check_safemode"))
{
    auto const config = css::configuration::ReadOnlyAccess::create(
        comphelper::getProcessComponentContext(),
        LanguageTag(
            css::uno::Reference<css::lang::XLocalizable>(
                css::configuration::theDefaultProvider::get(
                    comphelper::getProcessComponentContext()),
                css::uno::UNO_QUERY_THROW)->
            getLocale()).getBcp47());

    auto const preText = config->getByHierarchicalName(
        "/org.openoffice.Office.Common/Misc/CrashReportPreSendNotification");
    if (OUString text; preText >>= text) {
        mxEditPreUpload->set_label(
            text.replaceAll("%PRODUCTNAME", utl::ConfigManager::getProductName()));
    }

    auto const postText = config->getByHierarchicalName(
        "/org.openoffice.Office.Common/Misc/CrashReportPostSendNotification");
    if (OUString text; postText >>= text) {
        OUString url;
        rtl::Bootstrap::get("CrashDumpUrl", url);
        maSuccessMsg = text.replaceAll("%CrashDumpUrl%", url);
    } else {
        maSuccessMsg = mxEditPostUpload->get_text();
    }

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
}

CrashReportDialog::~CrashReportDialog()
{
}

short CrashReportDialog::run()
{
    short nRet = GenericDialogController::run();

    // Check whether to go to safe mode
    if (mxCBSafeMode->get_active())
    {
        sfx2::SafeMode::putFlag();
        css::task::OfficeRestartManager::get(comphelper::getProcessComponentContext())->requestRestart(
            css::uno::Reference< css::task::XInteractionHandler >());
    }
    return nRet;
}

IMPL_LINK(CrashReportDialog, BtnHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == mxBtnSend.get())
    {
        std::string response;
        bool bSuccess = CrashReporter::readSendConfig(response);

        OUString aCrashID = OUString::createFromAscii(response.c_str());

        if (bSuccess)
        {
            OUString aProcessedMessage = maSuccessMsg.replaceAll("%CRASHID", aCrashID.replaceAll("Crash-ID=",""));

            // vclbuilder seems to replace _ with ~ even in text
            mxEditPostUpload->set_text(aProcessedMessage.replaceAll("~", "_"));
        }
        else
        {
            mxEditPostUpload->set_text(aCrashID);
        }

        mxBtnClose->show();
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
