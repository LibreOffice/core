/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SVX_ACCESSIBILITYCHECKDIALOG_HXX
#define INCLUDED_SVX_ACCESSIBILITYCHECKDIALOG_HXX

#include <sal/types.h>
#include <svx/svxdllapi.h>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/weld.hxx>
#include <svx/AccessibilityIssue.hxx>

class SVX_DLLPUBLIC AccessibilityCheckEntry final
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Label> m_xLabel;

    svx::AccessibilityIssue const& m_rAccessibilityIssue;

public:
    AccessibilityCheckEntry(weld::Container* pParent,
                            svx::AccessibilityIssue const& rAccessibilityIssue);
    weld::Widget* get_widget() const { return m_xContainer.get(); }
};

class SVX_DLLPUBLIC AccessibilityCheckDialog final : public weld::GenericDialogController
{
private:
    std::vector<svx::AccessibilityIssue> m_rAccessibilityIssueCollection;
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aAccessibilityCheckEntries;

    // Controls
    std::unique_ptr<weld::Box> m_xAccessibilityCheckBox;

public:
    AccessibilityCheckDialog(
        weld::Window* pParent,
        std::vector<svx::AccessibilityIssue> const& rAccessibilityIssueCollection);
    virtual ~AccessibilityCheckDialog() override;
    virtual short run() override;
};

#endif // INCLUDED_SVX_ACCESSIBILITYCHECKDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
