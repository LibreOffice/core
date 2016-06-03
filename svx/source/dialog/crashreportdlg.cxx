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
#include <desktop/minidump.hxx>
#include <osl/file.hxx>

CrashReportDialog::CrashReportDialog(vcl::Window* pParent):
    Dialog(pParent, "CrashReportDialog",
            "svx/ui/crashreportdlg.ui")
{
    get(mpBtnSend, "btn_send");
    get(mpBtnCancel, "btn_cancel");
    get(mpBtnClose, "btn_close");
    get(mpEditPreUpload, "ed_pre");
    get(mpEditPostUpload, "ed_post");

    mpBtnSend->SetClickHdl(LINK(this, CrashReportDialog, BtnHdl));
    mpBtnCancel->SetClickHdl(LINK(this, CrashReportDialog, BtnHdl));
    mpBtnClose->SetClickHdl(LINK(this, CrashReportDialog, BtnHdl));
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

    Dialog::dispose();
}

namespace {

OUString getLibDir()
{
    OUString aOriginal = "$BRAND_BASE_DIR/" LIBO_LIBEXEC_FOLDER;
    rtl::Bootstrap::expandMacros(aOriginal);

    return aOriginal;
}

}

IMPL_LINK_TYPED(CrashReportDialog, BtnHdl, Button*, pBtn, void)
{
    if (pBtn == mpBtnSend.get())
    {
        std::string ini_path = CrashReporter::getIniFileName();

        std::string response;
        bool bSuccess = readConfig(ini_path, response);

        OUString aCrashID = OUString::createFromAscii(response.c_str());

        if (bSuccess)
            mpEditPostUpload->SetText(mpEditPostUpload->GetText() + " " + aCrashID);
        else
            mpEditPostUpload->SetText(mpEditPostUpload->GetText() + " Error!");

        mpBtnClose->Show();
        mpEditPreUpload->Hide();
        mpEditPostUpload->Show();
        mpBtnSend->Hide();
        mpBtnCancel->Hide();
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
