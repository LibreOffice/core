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

#include <rtl/bootstrap.hxx>
#include <desktop/crashreport.hxx>
#include <sfx2/safemode.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>

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
    , mxBugReportMailto(m_xBuilder->weld_link_button("bugreport_mailto"))
    , mxCBSafeMode(m_xBuilder->weld_check_button("check_safemode"))
{
    maSuccessMsg = mxEditPostUpload->get_text();

    auto nWidth = mxEditPreUpload->get_preferred_size().Width();
    nWidth = std::max(nWidth, mxCBSafeMode->get_size_request().Width());
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
        mxBugReportMailto->set_uri(mxBugReportMailto->get_uri().replaceAll("%CRASHID", aCrashID.replaceAll("Crash-ID=","")));
        mxBugReportMailto->show();
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
