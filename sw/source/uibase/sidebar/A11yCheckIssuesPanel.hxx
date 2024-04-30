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
    std::unique_ptr<weld::LinkButton> m_xGotoButton;
    std::unique_ptr<weld::Button> m_xFixButton;

    std::shared_ptr<sfx::AccessibilityIssue> m_pAccessibilityIssue;

public:
    AccessibilityCheckEntry(weld::Container* pParent,
                            std::shared_ptr<sfx::AccessibilityIssue> const& pAccessibilityIssue);

    weld::Widget* get_widget() const { return m_xContainer.get(); }

    DECL_LINK(GotoButtonClicked, weld::LinkButton&, bool);
    DECL_LINK(FixButtonClicked, weld::Button&, void);
};

enum class AccessibilityCheckGroups : size_t
{
    Document = 0,
    Styles = 1,
    Linked = 2,
    NoAlt = 3,
    Table = 4,
    Formatting = 5,
    Hyperlink = 6,
    Fakes = 7,
    Numbering = 8,
    Other = 9,
    LAST = Other
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
    void ImplDestroy();
    virtual ~A11yCheckIssuesPanel() override;

private:
    std::array<std::vector<std::unique_ptr<AccessibilityCheckEntry>>, 10> m_aEntries;
    std::array<std::unique_ptr<weld::Expander>, 10> m_xExpanders;
    std::array<std::unique_ptr<weld::Box>, 10> m_xBoxes;

    sfx::AccessibilityIssueCollection m_aIssueCollection;
    void removeAllEntries();
    void populateIssues();

    void addEntryForGroup(AccessibilityCheckGroups eGroup, std::vector<sal_Int32>& rIndices,
                          std::shared_ptr<sfx::AccessibilityIssue> const& pIssue);

    SfxBindings* mpBindings;
    SwDoc* mpDoc;
    ::sfx2::sidebar::ControllerItem maA11yCheckController;
    sal_Int32 mnIssueCount;
    bool mbAutomaticCheckEnabled;
};

} //end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
