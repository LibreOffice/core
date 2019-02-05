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
#include <vcl/weld.hxx>

namespace {

class RestartDialog : public weld::GenericDialogController{
public:
    RestartDialog(weld::Window* parent, svtools::RestartReason reason)
        : GenericDialogController(parent, "svt/ui/restartdialog.ui", "RestartDialog")
        , btnYes_(m_xBuilder->weld_button("yes"))
        , btnNo_(m_xBuilder->weld_button("no"))
    {
        switch (reason) {
        case svtools::RESTART_REASON_JAVA:
            reason_ = m_xBuilder->weld_widget("reason_java");
            break;
        case svtools::RESTART_REASON_PDF_AS_STANDARD_JOB_FORMAT:
            reason_ = m_xBuilder->weld_widget("reason_pdf");
            break;
        case svtools::RESTART_REASON_BIBLIOGRAPHY_INSTALL:
            reason_ = m_xBuilder->weld_widget("reason_bibliography_install");
            break;
        case svtools::RESTART_REASON_MAILMERGE_INSTALL:
            reason_ = m_xBuilder->weld_widget("reason_mailmerge_install");
            break;
        case svtools::RESTART_REASON_LANGUAGE_CHANGE:
            reason_ = m_xBuilder->weld_widget("reason_language_change");
            break;
        case svtools::RESTART_REASON_ADDING_PATH:
            reason_ = m_xBuilder->weld_widget("reason_adding_path");
            break;
        case svtools::RESTART_REASON_ASSIGNING_JAVAPARAMETERS:
            reason_ = m_xBuilder->weld_widget("reason_assigning_javaparameters");
            break;
        case svtools::RESTART_REASON_ASSIGNING_FOLDERS:
            reason_ = m_xBuilder->weld_widget("reason_assigning_folders");
            break;
        case svtools::RESTART_REASON_EXP_FEATURES:
            reason_ = m_xBuilder->weld_widget("reason_exp_features");
            break;
        case svtools::RESTART_REASON_EXTENSION_INSTALL:
            reason_ = m_xBuilder->weld_widget("reason_extension_install");
            break;
        case svtools::RESTART_REASON_OPENGL:
            reason_ = m_xBuilder->weld_widget("reason_opengl");
            break;
        case svtools::RESTART_REASON_OPENCL:
            reason_ = m_xBuilder->weld_widget("reason_opencl");
            break;
        case svtools::RESTART_REASON_MSCOMPATIBLE_FORMS_MENU:
            reason_ = m_xBuilder->weld_widget("reason_mscompatible_formsmenu");
            break;
        default:
            assert(false); // this cannot happen
        }
        reason_->show();
        btnYes_->connect_clicked(LINK(this, RestartDialog, hdlYes));
        btnNo_->connect_clicked(LINK(this, RestartDialog, hdlNo));
    }
private:
    DECL_LINK(hdlYes, weld::Button&, void);
    DECL_LINK(hdlNo, weld::Button&, void);

    std::unique_ptr<weld::Widget> reason_;
    std::unique_ptr<weld::Button> btnYes_;
    std::unique_ptr<weld::Button> btnNo_;
};

IMPL_LINK_NOARG(RestartDialog, hdlYes, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(RestartDialog, hdlNo, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

}

bool svtools::executeRestartDialog(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    weld::Window* parent, RestartReason reason)
{
    auto xRestartManager = css::task::OfficeRestartManager::get(context);
    if (xRestartManager->isRestartRequested(false))
        return true; // don't try to show another dialog when restart is already in progress
    RestartDialog aDlg(parent, reason);
    if (aDlg.run()) {
        xRestartManager->requestRestart(
            css::uno::Reference< css::task::XInteractionHandler >());
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
