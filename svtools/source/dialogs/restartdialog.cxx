/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>

#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <svtools/restartdialog.hxx>
#include <tools/link.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/window.hxx>

namespace {

class RestartDialog: public ModalDialog {
public:
    RestartDialog(vcl::Window * parent, svtools::RestartReason reason):
        ModalDialog(parent, "RestartDialog", "svt/ui/restartdialog.ui")
    {
        get(btnYes_, "yes");
        get(btnNo_, "no");
        switch (reason) {
        case svtools::RESTART_REASON_JAVA:
            get(reason_, "reason_java");
            break;
        case svtools::RESTART_REASON_PDF_AS_STANDARD_JOB_FORMAT:
            get(reason_, "reason_pdf");
            break;
        case svtools::RESTART_REASON_BIBLIOGRAPHY_INSTALL:
            get(reason_, "reason_bibliography_install");
            break;
        case svtools::RESTART_REASON_MAILMERGE_INSTALL:
            get(reason_, "reason_mailmerge_install");
            break;
        case svtools::RESTART_REASON_LANGUAGE_CHANGE:
            get(reason_, "reason_language_change");
            break;
        case svtools::RESTART_REASON_ADDING_PATH:
            get(reason_, "reason_adding_path");
            break;
        case svtools::RESTART_REASON_ASSIGNING_JAVAPARAMETERS:
            get(reason_, "reason_assigning_javaparameters");
            break;
        case svtools::RESTART_REASON_ASSIGNING_FOLDERS:
            get(reason_, "reason_assigning_folders");
            break;
        case svtools::RESTART_REASON_EXP_FEATURES:
            get(reason_,"reason_exp_features");
            break;
        case svtools::RESTART_REASON_EXTENSION_INSTALL:
            get(reason_, "reason_extension_install");
            break;
        default:
            assert(false); // this cannot happen
        }
        reason_->Show();
        btnYes_->SetClickHdl(LINK(this, RestartDialog, hdlYes));
        btnNo_->SetClickHdl(LINK(this, RestartDialog, hdlNo));
    }
    virtual ~RestartDialog() { disposeOnce(); }
    virtual void dispose() override
    {
        reason_.clear();
        btnYes_.clear();
        btnNo_.clear();
        ModalDialog::dispose();
    }
private:
    DECL_LINK_TYPED(hdlYes, Button*, void);
    DECL_LINK_TYPED(hdlNo, Button*, void);

    VclPtr<vcl::Window> reason_;
    VclPtr<PushButton> btnYes_;
    VclPtr<PushButton> btnNo_;
};

IMPL_LINK_NOARG_TYPED(RestartDialog, hdlYes, Button*, void)
{
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG_TYPED(RestartDialog, hdlNo, Button*, void)
{
    EndDialog();
}

}

void svtools::executeRestartDialog(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    vcl::Window * parent, RestartReason reason)
{
    if (ScopedVclPtrInstance<RestartDialog>(parent, reason)->Execute()) {
        css::task::OfficeRestartManager::get(context)->requestRestart(
            css::uno::Reference< css::task::XInteractionHandler >());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
