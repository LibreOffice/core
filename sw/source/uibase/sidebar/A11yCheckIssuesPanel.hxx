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

    std::shared_ptr<sfx::AccessibilityIssue> const& m_pAccessibilityIssue;

public:
    AccessibilityCheckEntry(weld::Container* pParent,
                            std::shared_ptr<sfx::AccessibilityIssue> const& pAccessibilityIssue);

    weld::Widget* get_widget() const { return m_xContainer.get(); }

    DECL_LINK(GotoButtonClicked, weld::LinkButton&, bool);
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
    void ImplDestroy();
    virtual ~A11yCheckIssuesPanel() override;

private:
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aDocumentEntries;
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aStylesEntries;
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aNoAltEntries;
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aTableEntries;
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aFormattingEntries;
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aHyperlinkEntries;
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aFakesEntries;
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aNumberingEntries;
    std::vector<std::unique_ptr<AccessibilityCheckEntry>> m_aOtherEntries;
    std::unique_ptr<weld::Expander> m_xExpanderDocument;
    std::unique_ptr<weld::Expander> m_xExpanderStyles;
    std::unique_ptr<weld::Expander> m_xExpanderNoAlt;
    std::unique_ptr<weld::Expander> m_xExpanderTable;
    std::unique_ptr<weld::Expander> m_xExpanderFormatting;
    std::unique_ptr<weld::Expander> m_xExpanderHyperlink;
    std::unique_ptr<weld::Expander> m_xExpanderFakes;
    std::unique_ptr<weld::Expander> m_xExpanderNumbering;
    std::unique_ptr<weld::Expander> m_xExpanderOther;
    std::unique_ptr<weld::Box> m_xBoxDocument;
    std::unique_ptr<weld::Box> m_xBoxStyles;
    std::unique_ptr<weld::Box> m_xBoxNoAlt;
    std::unique_ptr<weld::Box> m_xBoxTable;
    std::unique_ptr<weld::Box> m_xBoxFormatting;
    std::unique_ptr<weld::Box> m_xBoxHyperlink;
    std::unique_ptr<weld::Box> m_xBoxFakes;
    std::unique_ptr<weld::Box> m_xBoxNumbering;
    std::unique_ptr<weld::Box> m_xBoxOther;
    sfx::AccessibilityIssueCollection m_aIssueCollection;
    std::function<sfx::AccessibilityIssueCollection()> m_getIssueCollection;
    void populateIssues();

    SfxBindings* mpBindings;
    SwDoc* mpDoc;
    ::sfx2::sidebar::ControllerItem maA11yCheckController;
    sal_Int32 mnIssueCount;
    bool mbAutomaticCheckEnabled;
};

} //end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
