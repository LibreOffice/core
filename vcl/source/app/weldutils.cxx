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
                         const OUString& rUIXMLDescription, const OString& rID, bool bIsMobile)
    : m_pDialogController(pController)
    , m_xBuilder(Application::CreateBuilder(pParent, rUIXMLDescription, bIsMobile))
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

FormattedEntry::FormattedEntry(weld::Entry& rEntry)
    : m_rEntry(rEntry)
    , m_eOptions(Application::GetSettings().GetStyleSettings().GetSelectionOptions())
{
    m_rEntry.connect_changed(LINK(this, FormattedEntry, ModifyHdl));
    m_rEntry.connect_focus_out(LINK(this, FormattedEntry, FocusOutHdl));
}

Selection FormattedEntry::GetEntrySelection() const
{
    int nStartPos, nEndPos;
    m_rEntry.get_selection_bounds(nStartPos, nEndPos);
    return Selection(nStartPos, nEndPos);
}

OUString FormattedEntry::GetEntryText() const { return m_rEntry.get_text(); }

void FormattedEntry::SetEntryText(const OUString& rText, const Selection& rSel)
{
    m_rEntry.set_text(rText);
    auto nMin = rSel.Min();
    auto nMax = rSel.Max();
    m_rEntry.select_region(nMin < 0 ? 0 : nMin, nMax == SELECTION_MAX ? -1 : nMax);
}

void FormattedEntry::SetEntryTextColor(const Color* pColor)
{
    m_rEntry.set_font_color(pColor ? *pColor : COL_AUTO);
}

SelectionOptions FormattedEntry::GetEntrySelectionOptions() const { return m_eOptions; }

void FormattedEntry::FieldModified() { m_aModifyHdl.Call(m_rEntry); }

IMPL_LINK_NOARG(FormattedEntry, ModifyHdl, weld::Entry&, void) { Modify(); }

IMPL_LINK_NOARG(FormattedEntry, FocusOutHdl, weld::Widget&, void) { EntryLostFocus(); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
