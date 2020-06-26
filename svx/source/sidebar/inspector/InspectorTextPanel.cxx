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
    , mxListBoxStyles(m_xBuilder->weld_tree_view("listbox_fonts"))
{
    mxListBoxStyles->set_size_request(-1, mxListBoxStyles->get_height_rows(27));
}

static void FillBox_Impl(weld::TreeView& rListBoxStyles, const TreeNode& current,
                         weld::TreeIter* pParent)
{
    std::unique_ptr<weld::TreeIter> pResult = rListBoxStyles.make_iterator();
    const OUString& rName = current.sNodeName;
    rListBoxStyles.insert(pParent, -1, &rName, nullptr, nullptr, nullptr, false, pResult.get());

    for (const TreeNode& ChildNode : current.children)
        FillBox_Impl(rListBoxStyles, ChildNode, pResult.get());
}

void InspectorTextPanel::updateEntries(const std::vector<TreeNode>& rStore)
{
    mxListBoxStyles->freeze();
    mxListBoxStyles->clear();
    for (const TreeNode& ChildNode : rStore)
    {
        FillBox_Impl(*mxListBoxStyles, ChildNode, nullptr);
    }

    mxListBoxStyles->thaw();

    weld::TreeView* pTreeDiagram = mxListBoxStyles.get();
    pTreeDiagram->all_foreach([pTreeDiagram](weld::TreeIter& rEntry) {
        pTreeDiagram->expand_row(rEntry);
        return false;
    });
}

InspectorTextPanel::~InspectorTextPanel() { disposeOnce(); }

void InspectorTextPanel::dispose()
{
    mxListBoxStyles.reset();

    PanelLayout::dispose();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
