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
#include <vcl/vclenum.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Button.hxx>
#include <vcl/weld/Dialog.hxx>
#include <vcl/weld/Label.hxx>
#include <vcl/weld/DialogController.hxx>
#include <svtools/svtresid.hxx>
#include <unotools/resmgr.hxx>

#include <restart.hrc>

namespace {

class RestartDialog : public weld::GenericDialogController{
public:
    RestartDialog(weld::Window* parent, svtools::RestartReason reason)
        : GenericDialogController(parent, u"svt/ui/restartdialog.ui"_ustr, u"RestartDialog"_ustr)
        , btnYes_(m_xBuilder->weld_button(u"yes"_ustr))
        , btnNo_(m_xBuilder->weld_button(u"no"_ustr))
    {
        assert(reason < svtools::RESTART_REASON_NONE);
        reason_ = m_xBuilder->weld_label(u"reason_template"_ustr);
        reason_->set_label(SvtResId(RID_RESTART_REASONS[reason]));
        btnYes_->connect_clicked(LINK(this, RestartDialog, hdlYes));
        btnNo_->connect_clicked(LINK(this, RestartDialog, hdlNo));
    }
private:
    DECL_LINK(hdlYes, weld::Button&, void);
    DECL_LINK(hdlNo, weld::Button&, void);

    std::unique_ptr<weld::Label> reason_;
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
