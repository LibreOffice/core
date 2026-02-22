/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/svapp.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/DialogController.hxx>
#include <vcl/weld/Dialog.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <vcl/weld/weld.hxx>

namespace weld
{
short DialogController::run() { return getDialog()->run(); }

bool DialogController::runAsync(const std::shared_ptr<DialogController>& rController,
                                const std::function<void(sal_Int32)>& func)
{
    return rController->getDialog()->runAsync(rController, func);
}

void DialogController::set_title(const OUString& rTitle) { getDialog()->set_title(rTitle); }

OUString DialogController::get_title() const { return getConstDialog()->get_title(); }

void DialogController::set_help_id(const OUString& rHelpId) { getDialog()->set_help_id(rHelpId); }

OUString DialogController::get_help_id() const { return getConstDialog()->get_help_id(); }

void DialogController::response(int nResponse) { getDialog()->response(nResponse); }

DialogController::~DialogController() {}

Dialog* GenericDialogController::getDialog() { return m_xDialog.get(); }

GenericDialogController::GenericDialogController(weld::Widget* pParent, const OUString& rUIFile,
                                                 const OUString& rDialogId, bool bMobile)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIFile, bMobile))
    , m_xDialog(m_xBuilder->weld_dialog(rDialogId))
{
}

GenericDialogController::~GenericDialogController() {}

Dialog* MessageDialogController::getDialog() { return m_xDialog.get(); }

MessageDialogController::MessageDialogController(weld::Widget* pParent, const OUString& rUIFile,
                                                 const OUString& rDialogId,
                                                 const OUString& rRelocateId)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIFile))
    , m_xDialog(m_xBuilder->weld_message_dialog(rDialogId))
    , m_xContentArea(m_xDialog->weld_message_area())
{
    if (!rRelocateId.isEmpty())
    {
        m_xRelocate = m_xBuilder->weld_widget(rRelocateId);
        m_xOrigParent = m_xRelocate->weld_parent();
        //fdo#75121, a bit tricky because the widgets we want to align with
        //don't actually exist in the ui description, they're implied
        m_xOrigParent->move(m_xRelocate.get(), m_xContentArea.get());
    }
}

void MessageDialogController::set_primary_text(const OUString& rText)
{
    m_xDialog->set_primary_text(rText);
}

OUString MessageDialogController::get_primary_text() const { return m_xDialog->get_primary_text(); }

void MessageDialogController::set_secondary_text(const OUString& rText)
{
    m_xDialog->set_secondary_text(rText);
}

OUString MessageDialogController::get_secondary_text() const
{
    return m_xDialog->get_secondary_text();
}

void MessageDialogController::set_default_response(int nResponse)
{
    m_xDialog->set_default_response(nResponse);
}

MessageDialogController::~MessageDialogController()
{
    if (m_xRelocate)
    {
        m_xContentArea->move(m_xRelocate.get(), m_xOrigParent.get());
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
