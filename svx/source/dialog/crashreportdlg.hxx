/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_SOURCE_DIALOG_CRASHREPORTDLG_HXX
#define INCLUDED_SVX_SOURCE_DIALOG_CRASHREPORTDLG_HXX

#include <vcl/weld.hxx>

class CrashReportDialog : public weld::GenericDialogController
{
public:
    explicit CrashReportDialog(weld::Window* pParent);
    virtual short run() override;
    virtual ~CrashReportDialog() override;

private:
    std::unique_ptr<weld::Button> mxBtnSend;
    std::unique_ptr<weld::Button> mxBtnCancel;
    std::unique_ptr<weld::Button> mxBtnClose;
    std::unique_ptr<weld::Label> mxEditPreUpload;
    std::unique_ptr<weld::TextView> mxEditPostUpload;
    std::unique_ptr<weld::LinkButton> mxBugReportMailto;
    std::unique_ptr<weld::CheckButton> mxCBSafeMode;

    OUString maSuccessMsg;

    DECL_LINK(BtnHdl, weld::Button&, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
