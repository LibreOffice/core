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

CrashReportDialog::CrashReportDialog(vcl::Window* pParent):
    Dialog(pParent, "CrashReportDialog",
            "svx/ui/crashreportdlg.ui")
{
    get(mpBtnSend, "btn_send");
    get(mpBtnCancel, "btn_cancel");

    mpBtnSend->SetClickHdl(LINK(this, CrashReportDialog, BtnHdl));
    mpBtnCancel->SetClickHdl(LINK(this, CrashReportDialog, BtnHdl));
}

CrashReportDialog::~CrashReportDialog()
{
    disposeOnce();
}

void CrashReportDialog::dispose()
{
    mpBtnSend.clear();
    mpBtnCancel.clear();

    Dialog::dispose();
}

namespace {

OString getLibDir()
{
    OUString aOriginal = "$BRAND_BASE_DIR/" LIBO_LIBEXEC_FOLDER;
    rtl::Bootstrap::expandMacros(aOriginal);

    return rtl::OUStringToOString(aOriginal, RTL_TEXTENCODING_UTF8);
}

}

IMPL_LINK_TYPED(CrashReportDialog, BtnHdl, Button*, pBtn, void)
{
    if (pBtn == mpBtnSend.get())
    {
        std::string ini_path = CrashReporter::getIniFileName();
        OString aCommand = getLibDir().copy(7) + "/minidump_upload " + ini_path.c_str();
        int retVal = std::system(aCommand.getStr());
        SAL_WARN_IF(retVal != 0, "crashreport", "Failed to upload minidump. Error Code: " << retVal);
        Close();
    }
    else if (pBtn == mpBtnCancel.get())
    {
        Close();
    }
    else
    {
        assert(false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
