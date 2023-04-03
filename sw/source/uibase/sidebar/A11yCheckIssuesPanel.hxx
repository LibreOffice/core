/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include <sfx2/AccessibilityIssue.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <svl/poolitem.hxx>
#include <tools/link.hxx>
#include <vcl/weld.hxx>

#include <doc.hxx>

namespace sw::sidebar
{
class AccessibilityCheckEntry final
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::Button> m_xGotoButton;
    std::unique_ptr<weld::Button> m_xFixButton;

    std::shared_ptr<sfx::AccessibilityIssue> const& m_pAccessibilityIssue;

public:
    AccessibilityCheckEntry(weld::Container* pParent,
                            std::shared_ptr<sfx::AccessibilityIssue> const& pAccessibilityIssue);

    weld::Widget* get_widget() const { return m_xContainer.get(); }

    DECL_LINK(GotoButtonClicked, weld::Button&, void);
    DECL_LINK(FixButtonClicked, weld::Button&, void);
};

class A11yCheckIssuesPanel : public PanelLayout,
                             public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static std::unique_ptr<PanelLayout> Create(weld::Widget* pParent, SfxBindings* pBindings);

    virtual void NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                                  const SfxPoolItem* pState) override;

    virtual void GetControlState(const sal_uInt16 /*nSId*/,
                                 boost::property_tree::ptree& /*rState*/) override{};

    A11yCheckIssuesPanel(weld::Widget* pParent, SfxBindings* pBindings);
    virtual ~A11yCheckIssuesPanel() override;

private:
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aAccessibilityCheckEntries;
    std::unique_ptr<weld::Box> m_xAccessibilityCheckBox;
    std::unique_ptr<weld::ScrolledWindow> m_xScrolledWindow;
    sfx::AccessibilityIssueCollection m_aIssueCollection;
    std::function<sfx::AccessibilityIssueCollection()> m_getIssueCollection;
    void populateIssues();

    SwDoc* mpDoc;
    ::sfx2::sidebar::ControllerItem maA11yCheckController;
    sal_Int32 mnIssueCount;
};

} //end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
