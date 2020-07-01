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

#include <sfx2/AccessibilityIssue.hxx>
#include <svx/svxdllapi.h>
#include <tools/link.hxx>
#include <vcl/weld.hxx>

namespace svx
{
class AccessibilityCheckEntry final
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::Button> m_xGotoButton;

    std::shared_ptr<sfx::AccessibilityIssue> const& m_pAccessibilityIssue;

public:
    AccessibilityCheckEntry(weld::Container* pParent,
                            std::shared_ptr<sfx::AccessibilityIssue> const& pAccessibilityIssue);

    weld::Widget* get_widget() const { return m_xContainer.get(); }

    DECL_LINK(GotoButtonClicked, weld::Button&, void);
};

class SVX_DLLPUBLIC AccessibilityCheckDialog final : public weld::GenericDialogController
{
private:
    sfx::AccessibilityIssueCollection m_aIssueCollection;
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aAccessibilityCheckEntries;

    // Controls
    std::unique_ptr<weld::Box> m_xAccessibilityCheckBox;

public:
    AccessibilityCheckDialog(weld::Window* pParent,
                             sfx::AccessibilityIssueCollection const& rIssueCollection);
    virtual ~AccessibilityCheckDialog() override;
    virtual short run() override;
};

} // end svx namespace

#endif // INCLUDED_SVX_ACCESSIBILITYCHECKDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
