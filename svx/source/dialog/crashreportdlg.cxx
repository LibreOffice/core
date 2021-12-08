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

CrashReportDialog::CrashReportDialog(vcl::Window* pParent):
    Dialog(pParent, "CrashReportDialog",
            "svx/ui/crashreportdlg.ui")
{
    get(mpBtnSend, "btn_send");
    get(mpBtnCancel, "btn_cancel");
    get(mpBtnClose, "btn_close");
    get(mpEditPreUpload, "ed_pre");
    get(mpEditPostUpload, "ed_post");
    get(mpCBSafeMode, "check_safemode");

    maSuccessMsg = mpEditPostUpload->GetText();

    mpBtnSend->SetClickHdl(LINK(this, CrashReportDialog, BtnHdl));
    mpBtnCancel->SetClickHdl(LINK(this, CrashReportDialog, BtnHdl));
    mpBtnClose->SetClickHdl(LINK(this, CrashReportDialog, BtnHdl));
    mpEditPostUpload->SetReadOnly();
}

CrashReportDialog::~CrashReportDialog()
{
    disposeOnce();
}

void CrashReportDialog::dispose()
{
    mpBtnSend.clear();
    mpBtnCancel.clear();
    mpBtnClose.clear();
    mpEditPreUpload.clear();
    mpEditPostUpload.clear();
    mpCBSafeMode.clear();

    Dialog::dispose();
}

bool CrashReportDialog::Close()
{
    // Check whether to go to safe mode
    if (mpCBSafeMode->IsChecked())
    {
        sfx2::SafeMode::putFlag();
        css::task::OfficeRestartManager::get(comphelper::getProcessComponentContext())->requestRestart(
            css::uno::Reference< css::task::XInteractionHandler >());
    }

    return Dialog::Close();
}

IMPL_LINK(CrashReportDialog, BtnHdl, Button*, pBtn, void)
{
    if (pBtn == mpBtnSend.get())
    {
        std::string response;
        bool bSuccess = CrashReporter::readSendConfig(response);

        OUString aCrashID = OUString::createFromAscii(response.c_str());

        if (bSuccess)
        {
            OUString aProcessedMessage = maSuccessMsg.replaceAll("%CRASHID", aCrashID.replaceAll("Crash-ID=",""));

            // vclbuilder seems to replace _ with ~ even in text
            mpEditPostUpload->SetText(aProcessedMessage.replaceAll("~", "_"));
        }
        else
        {
            mpEditPostUpload->SetText(aCrashID);
        }

        mpBtnClose->Show();
        mpEditPreUpload->Hide();
        mpEditPostUpload->Show();
        mpBtnSend->Hide();
        mpBtnSend->Disable();
        mpBtnCancel->Hide();
        mpBtnCancel->Disable();
        mpBtnClose->GrabFocus();

        setOptimalLayoutSize();
    }
    else if (pBtn == mpBtnCancel.get())
    {
        Close();
    }
    else if (pBtn == mpBtnClose.get())
    {
        Close();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
