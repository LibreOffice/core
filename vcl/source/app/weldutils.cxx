/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/builderpage.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>

BuilderPage::BuilderPage(weld::Widget* pParent, weld::DialogController* pController,
                         const OUString& rUIXMLDescription, const OString& rID)
    : m_pDialogController(pController)
    , m_xBuilder(Application::CreateBuilder(pParent, rUIXMLDescription))
    , m_xContainer(m_xBuilder->weld_container(rID))
{
}

void BuilderPage::Activate() {}

void BuilderPage::Deactivate() {}

BuilderPage::~BuilderPage() COVERITY_NOEXCEPT_FALSE {}

namespace weld
{
bool DialogController::runAsync(const std::shared_ptr<DialogController>& rController,
                                const std::function<void(sal_Int32)>& func)
{
    return rController->getDialog()->runAsync(rController, func);
}

DialogController::~DialogController() COVERITY_NOEXCEPT_FALSE {}

Dialog* GenericDialogController::getDialog() { return m_xDialog.get(); }

GenericDialogController::GenericDialogController(weld::Widget* pParent, const OUString& rUIFile,
                                                 const OString& rDialogId, bool bMobile)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIFile, bMobile))
    , m_xDialog(m_xBuilder->weld_dialog(rDialogId))
{
}

GenericDialogController::~GenericDialogController() COVERITY_NOEXCEPT_FALSE {}

Dialog* MessageDialogController::getDialog() { return m_xDialog.get(); }

MessageDialogController::MessageDialogController(weld::Widget* pParent, const OUString& rUIFile,
                                                 const OString& rDialogId,
                                                 const OString& rRelocateId)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIFile))
    , m_xDialog(m_xBuilder->weld_message_dialog(rDialogId))
    , m_xContentArea(m_xDialog->weld_message_area())
{
    if (!rRelocateId.isEmpty())
    {
        m_xRelocate = m_xBuilder->weld_container(rRelocateId);
        m_xOrigParent = m_xRelocate->weld_parent();
        //fdo#75121, a bit tricky because the widgets we want to align with
        //don't actually exist in the ui description, they're implied
        m_xOrigParent->move(m_xRelocate.get(), m_xContentArea.get());
    }
}

MessageDialogController::~MessageDialogController()
{
    if (m_xRelocate)
    {
        m_xContentArea->move(m_xRelocate.get(), m_xOrigParent.get());
    }
}

AssistantController::AssistantController(weld::Widget* pParent, const OUString& rUIFile,
                                         const OString& rDialogId)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIFile))
    , m_xAssistant(m_xBuilder->weld_assistant(rDialogId))
{
}

Dialog* AssistantController::getDialog() { return m_xAssistant.get(); }

AssistantController::~AssistantController() {}

void TriStateEnabled::ButtonToggled(weld::ToggleButton& rToggle)
{
    if (bTriStateEnabled)
    {
        switch (eState)
        {
            case TRISTATE_INDET:
                rToggle.set_state(TRISTATE_FALSE);
                break;
            case TRISTATE_TRUE:
                rToggle.set_state(TRISTATE_INDET);
                break;
            case TRISTATE_FALSE:
                rToggle.set_state(TRISTATE_TRUE);
                break;
        }
    }
    eState = rToggle.get_state();
}

void RemoveParentKeepChildren(weld::TreeView& rTreeView, weld::TreeIter& rParent)
{
    if (rTreeView.iter_has_child(rParent))
    {
        std::unique_ptr<weld::TreeIter> xNewParent(rTreeView.make_iterator(&rParent));
        if (!rTreeView.iter_parent(*xNewParent))
            xNewParent.reset();

        while (true)
        {
            std::unique_ptr<weld::TreeIter> xChild(rTreeView.make_iterator(&rParent));
            if (!rTreeView.iter_children(*xChild))
                break;
            rTreeView.move_subtree(*xChild, xNewParent.get(), -1);
        }
    }
    rTreeView.remove(rParent);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
