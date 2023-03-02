/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/AccessibilityCheckDialog.hxx>

#include <sfx2/AccessibilityIssue.hxx>
#include <utility>
#include <vcl/svapp.hxx>

namespace svx
{
AccessibilityCheckEntry::AccessibilityCheckEntry(
    weld::Container* pParent, std::shared_ptr<sfx::AccessibilityIssue> const& rAccessibilityIssue)
    : m_xBuilder(Application::CreateBuilder(pParent, "svx/ui/accessibilitycheckentry.ui"))
    , m_xContainer(m_xBuilder->weld_container("accessibilityCheckEntryBox"))
    , m_xLabel(m_xBuilder->weld_label("accessibilityCheckEntryLabel"))
    , m_xGotoButton(m_xBuilder->weld_button("accessibilityCheckEntryGotoButton"))
    , m_pAccessibilityIssue(rAccessibilityIssue)
{
    m_xLabel->set_label(m_pAccessibilityIssue->m_aIssueText);
    // lock in the height as including the button so all rows are the same height
    m_xContainer->set_size_request(-1, m_xContainer->get_preferred_size().Height());
    m_xGotoButton->set_visible(m_pAccessibilityIssue->canGotoIssue());
    m_xGotoButton->connect_clicked(LINK(this, AccessibilityCheckEntry, GotoButtonClicked));
}

IMPL_LINK_NOARG(AccessibilityCheckEntry, GotoButtonClicked, weld::Button&, void)
{
    m_pAccessibilityIssue->gotoIssue();
}

AccessibilityCheckDialog::AccessibilityCheckDialog(
    weld::Window* pParent, sfx::AccessibilityIssueCollection aIssueCollection,
    std::function<sfx::AccessibilityIssueCollection()> getIssueCollection)
    : GenericDialogController(pParent, "svx/ui/accessibilitycheckdialog.ui",
                              "AccessibilityCheckDialog")
    , m_aIssueCollection(std::move(aIssueCollection))
    , m_getIssueCollection(getIssueCollection)
    , m_xScrolledWindow(m_xBuilder->weld_scrolled_window("scrolledwindow"))
    , m_xAccessibilityCheckBox(m_xBuilder->weld_box("accessibilityCheckBox"))
    , m_xRescanBtn(m_xBuilder->weld_button("rescan"))
{
    m_xRescanBtn->connect_clicked(LINK(this, AccessibilityCheckDialog, RescanButtonClicked));

    populateIssues();
}

AccessibilityCheckDialog::~AccessibilityCheckDialog() {}

void AccessibilityCheckDialog::populateIssues()
{
    sal_Int32 i = 0;
    for (std::shared_ptr<sfx::AccessibilityIssue> const& pIssue : m_aIssueCollection.getIssues())
    {
        auto xEntry
            = std::make_unique<AccessibilityCheckEntry>(m_xAccessibilityCheckBox.get(), pIssue);
        m_xAccessibilityCheckBox->reorder_child(xEntry->get_widget(), i++);
        m_aAccessibilityCheckEntries.push_back(std::move(xEntry));
    }

    if (!m_aAccessibilityCheckEntries.empty())
    {
        auto nRowHeight
            = m_aAccessibilityCheckEntries.back()->get_widget()->get_preferred_size().Height();
        // 6 is the spacing set in the .ui
        m_xScrolledWindow->vadjustment_set_step_increment(nRowHeight + 6);
    }
}

IMPL_LINK_NOARG(AccessibilityCheckDialog, RescanButtonClicked, weld::Button&, void)
{
    // Remove old issue widgets
    for (auto const& xEntry : m_aAccessibilityCheckEntries)
        m_xAccessibilityCheckBox->move(xEntry->get_widget(), nullptr);

    m_aIssueCollection = m_getIssueCollection();
    populateIssues();
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
