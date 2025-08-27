/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

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
    std::unique_ptr<weld::Label> mxEditPostUpload;
    std::unique_ptr<weld::LinkButton> mxLinkButton;
    std::unique_ptr<weld::CheckButton> mxCBSafeMode;
    std::unique_ptr<weld::LinkButton> mxPrivacyPolicyButton;

    OUString maLinkTemplate;

    DECL_LINK(BtnHdl, weld::Button&, void);
    DECL_STATIC_LINK(CrashReportDialog, InstallLOKNotifierHdl, void*,
                     vcl::ILibreOfficeKitNotifier*);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
