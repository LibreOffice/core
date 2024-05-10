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

#include <o3tl/safeint.hxx>
#include <sal/config.h>

#include <svx/dialmgr.hxx>

#include <svx/sidebar/InspectorTextPanel.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <inspectorvalues.hrc>

using namespace css;

const int MinimumPanelWidth = 250;

namespace svx::sidebar
{
std::unique_ptr<PanelLayout> InspectorTextPanel::Create(weld::Widget* pParent)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            u"no parent Window given to InspectorTextPanel::Create"_ustr, nullptr, 0);
    return std::make_unique<InspectorTextPanel>(pParent);
}

InspectorTextPanel::InspectorTextPanel(weld::Widget* pParent)
    : PanelLayout(pParent, u"InspectorTextPanel"_ustr, u"svx/ui/inspectortextpanel.ui"_ustr)
    , mpListBoxStyles(m_xBuilder->weld_tree_view(u"listbox_fonts"_ustr))
{
    mpListBoxStyles->set_size_request(MinimumPanelWidth, -1);
    float fWidth = mpListBoxStyles->get_approximate_digit_width();
    std::vector<int> aWidths{ o3tl::narrowing<int>(fWidth * 29) };
    // 2nd column will fill remaining space
    mpListBoxStyles->set_column_fixed_widths(aWidths);
}

static bool GetPropertyValues(std::u16string_view rPropName, const uno::Any& rAny,
                              OUString& rString)
{
    if (bool bValue; rAny >>= bValue)
    {
        rString = SvxResId(bValue ? RID_TRUE : RID_FALSE); // tdf#139136
    }
    else if (OUString aValue; (rAny >>= aValue) && !(aValue.isEmpty()))
    {
        rString = aValue;
    }
    else if (awt::FontSlant eValue; rAny >>= eValue)
    {
        rString = SvxResId(eValue == awt::FontSlant_ITALIC ? RID_ITALIC : RID_NORMAL);
    }
    else if (tools::Long nValueLong; rAny >>= nValueLong)
    {
        if (rPropName.find(u"Color") != std::u16string_view::npos)
            rString = "0x" + OUString::number(nValueLong, 16);
        else
            rString = OUString::number(nValueLong);
    }
    else if (double fValue; rAny >>= fValue)
    {
        if (rPropName.find(u"Weight") != std::u16string_view::npos)
            rString = SvxResId(fValue > 100 ? RID_BOLD : RID_NORMAL);
        else
            rString = OUString::number((round(fValue * 100)) / 100.00);
    }
    else if (short nValueShort; rAny >>= nValueShort)
    {
        rString = OUString::number(nValueShort);
    }
    else
        return false;

    return true;
}

static void FillBox_Impl(weld::TreeView& rListBoxStyles, const TreeNode& rCurrent,
                         const weld::TreeIter* pParent)
{
    std::unique_ptr<weld::TreeIter> pResult = rListBoxStyles.make_iterator();
    const OUString& rName = rCurrent.sNodeName;
    OUString sPairValue;

    if (!(rCurrent.NodeType != TreeNode::SimpleProperty
          || GetPropertyValues(rName, rCurrent.aValue, sPairValue)))
        return;

    rListBoxStyles.insert(pParent, -1, &rName, nullptr, nullptr, nullptr, false, pResult.get());
    rListBoxStyles.set_sensitive(*pResult, !rCurrent.isGrey, 0);
    rListBoxStyles.set_text_emphasis(*pResult, rCurrent.NodeType == TreeNode::Category, 0);

    if (rCurrent.NodeType == TreeNode::SimpleProperty)
    {
        rListBoxStyles.set_text(*pResult, sPairValue, 1);
        rListBoxStyles.set_sensitive(*pResult, !rCurrent.isGrey, 1);
        rListBoxStyles.set_text_emphasis(*pResult, false, 1);
    }
    else
    {
        // Necessary, without this the selection line will be truncated.
        rListBoxStyles.set_text(*pResult, u""_ustr, 1);
    }

    for (const TreeNode& rChildNode : rCurrent.children)
        FillBox_Impl(rListBoxStyles, rChildNode, pResult.get());
}

void InspectorTextPanel::updateEntries(const std::vector<TreeNode>& rStore, const sal_Int32 nParIdx)
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

    // Collapse "Default Paragraph Style"

    std::unique_ptr<weld::TreeIter> pEntry = mpListBoxStyles->make_iterator();
    if (!mpListBoxStyles->get_iter_first(*pEntry))
        return;
    // skip the optional metadata items before "Default Paragraph Style"
    for (sal_Int32 i = 0; i < nParIdx; ++i)
    {
        if (!mpListBoxStyles->iter_next_sibling(*pEntry))
            return;
    }
    if (!mpListBoxStyles->iter_next(*pEntry))
        return;

    mpListBoxStyles->collapse_row(*pEntry);
}

InspectorTextPanel::~InspectorTextPanel() {}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
