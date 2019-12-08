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
#include <svx/AccessibilityIssue.hxx>
#include <vcl/svapp.hxx>

namespace svx
{
AccessibilityCheckEntry::AccessibilityCheckEntry(weld::Container* pParent,
                                                 AccessibilityIssue const& rAccessibilityIssue)
    : m_xBuilder(Application::CreateBuilder(pParent, "svx/ui/accessibilitycheckentry.ui"))
    , m_xContainer(m_xBuilder->weld_container("accessibilityCheckEntryBox"))
    , m_xLabel(m_xBuilder->weld_label("accessibilityCheckEntryLabel"))
    , m_rAccessibilityIssue(rAccessibilityIssue)
{
    m_xLabel->set_label(m_rAccessibilityIssue.m_aIssueText);
    m_xContainer->show();
}

AccessibilityCheckDialog::AccessibilityCheckDialog(
    weld::Window* pParent,
    std::vector<svx::AccessibilityIssue> const& rAccessibilityIssueCollection)
    : GenericDialogController(pParent, "svx/ui/accessibilitycheckdialog.ui",
                              "AccessibilityCheckDialog")
    , m_rAccessibilityIssueCollection(rAccessibilityIssueCollection)
    , m_xAccessibilityCheckBox(m_xBuilder->weld_box("accessibilityCheckBox"))
{
}

AccessibilityCheckDialog::~AccessibilityCheckDialog() {}

short AccessibilityCheckDialog::run()
{
    sal_Int32 i = 0;

    for (svx::AccessibilityIssue const& rResult : m_rAccessibilityIssueCollection)
    {
        auto xEntry
            = std::make_unique<AccessibilityCheckEntry>(m_xAccessibilityCheckBox.get(), rResult);
        m_xAccessibilityCheckBox->reorder_child(xEntry->get_widget(), i++);
        m_aAccessibilityCheckEntries.push_back(std::move(xEntry));
    }
    return GenericDialogController::run();
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
