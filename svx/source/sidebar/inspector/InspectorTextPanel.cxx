/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/sidebar/InspectorTextPanel.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace css;

namespace svx::sidebar
{
VclPtr<vcl::Window>
InspectorTextPanel::Create(vcl::Window* pParent,
                           const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to InspectorTextPanel::Create",
                                             nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to InspectorTextPanel::Create",
                                             nullptr, 1);

    return VclPtr<InspectorTextPanel>::Create(pParent, rxFrame);
}

InspectorTextPanel::InspectorTextPanel(vcl::Window* pParent,
                                       const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, "InspectorTextPanel", "svx/ui/inspectortextpanel.ui", rxFrame)
    , mpListBoxStyles(m_xBuilder->weld_tree_view("listbox_fonts"))
{
    mpListBoxStyles->set_size_request(-1, mpListBoxStyles->get_height_rows(25));
}

static void FillBox_Impl(weld::TreeView& rListBoxStyles, const TreeNode& rCurrent,
                         weld::TreeIter* pParent)
{
    std::unique_ptr<weld::TreeIter> pResult = rListBoxStyles.make_iterator();
    const OUString& rName = rCurrent.sNodeName;
    rListBoxStyles.insert(pParent, -1, &rName, nullptr, nullptr, nullptr, false, pResult.get());

    for (const TreeNode& rChildNode : rCurrent.children)
        FillBox_Impl(rListBoxStyles, rChildNode, pResult.get());
}

void InspectorTextPanel::updateEntries(const std::vector<TreeNode>& rStore)
{
    mpListBoxStyles->freeze();
    mpListBoxStyles->clear();
    for (const TreeNode& rChildNode : rStore)
    {
        FillBox_Impl(*mpListBoxStyles, rChildNode, nullptr);
    }

    mpListBoxStyles->thaw();

    weld::TreeView* pTreeDiagram = mpListBoxStyles.get();
    pTreeDiagram->all_foreach([pTreeDiagram](weld::TreeIter& rEntry) {
        pTreeDiagram->expand_row(rEntry);
        return false;
    });

    std::unique_ptr<weld::TreeIter> pEntry = mpListBoxStyles->make_iterator();
    mpListBoxStyles->get_iter_first(*pEntry);
    mpListBoxStyles->iter_next(*pEntry);
    mpListBoxStyles->collapse_row(*pEntry); // Collapse "Default Paragraph Style"
}

InspectorTextPanel::~InspectorTextPanel() { disposeOnce(); }

void InspectorTextPanel::dispose()
{
    mpListBoxStyles.reset();

    PanelLayout::dispose();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
