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

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/vclmedit.hxx>

class CrashReportDialog : public Dialog
{
public:

    explicit CrashReportDialog(vcl::Window* pParent);

    virtual ~CrashReportDialog() override;

    virtual void dispose() override;

    virtual bool Close() override;

private:

    VclPtr<Button> mpBtnSend;
    VclPtr<Button> mpBtnCancel;
    VclPtr<Button> mpBtnClose;
    VclPtr<FixedText> mpEditPreUpload;
    VclPtr<VclMultiLineEdit> mpEditPostUpload;
    VclPtr<CheckBox> mpCBSafeMode;

    OUString maSuccessMsg;

    DECL_LINK(BtnHdl, Button*, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
