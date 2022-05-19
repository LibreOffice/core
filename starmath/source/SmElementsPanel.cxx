/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>

#include "SmElementsPanel.hxx"
#include <starmath.hrc>
#include <smmod.hxx>
#include <strings.hrc>
#include <view.hxx>

namespace sm::sidebar
{
// static
std::unique_ptr<PanelLayout> SmElementsPanel::Create(weld::Widget& rParent,
                                                     const SfxBindings& rBindings)
{
    return std::make_unique<SmElementsPanel>(rParent, rBindings);
}

SmElementsPanel::SmElementsPanel(weld::Widget& rParent, const SfxBindings& rBindings)
    : PanelLayout(&rParent, "MathElementsPanel", "modules/smath/ui/sidebarelements_math.ui")
    , mrBindings(rBindings)
    , mxCategoryList(m_xBuilder->weld_tree_view("categorylist"))
    , mxElementsControl(std::make_unique<SmElementsControl>(
          m_xBuilder->weld_scrolled_window("scrolledwindow", true)))
    , mxElementsControlWin(
          std::make_unique<weld::CustomWeld>(*m_xBuilder, "element_selector", *mxElementsControl))
{
    for (const auto& rCategoryId : SmElementsControl::categories())
        mxCategoryList->append_text(SmResId(rCategoryId));

    mxCategoryList->set_size_request(-1, mxCategoryList->get_height_rows(6));

    mxCategoryList->connect_changed(LINK(this, SmElementsPanel, CategorySelectedHandle));
    mxCategoryList->select_text(SmResId(RID_CATEGORY_UNARY_BINARY_OPERATORS));

    mxElementsControl->setVerticalMode(false); // Sidebar currently can only dock to a side
    mxElementsControl->setElementSetId(RID_CATEGORY_UNARY_BINARY_OPERATORS);
    mxElementsControl->SetSelectHdl(LINK(this, SmElementsPanel, ElementClickHandler));
}

SmElementsPanel::~SmElementsPanel()
{
    mxElementsControlWin.reset();
    mxElementsControl.reset();
    mxCategoryList.reset();
}

IMPL_LINK(SmElementsPanel, CategorySelectedHandle, weld::TreeView&, rList, void)
{
    const OUString sSelected = rList.get_selected_text();
    for (const auto& rCategoryId : SmElementsControl::categories())
    {
        OUString aCurrentCategoryString = SmResId(rCategoryId);
        if (aCurrentCategoryString == sSelected)
        {
            mxElementsControl->setElementSetId(rCategoryId);
            if (SmViewShell* pViewSh = GetView())
                mxElementsControl->setSmSyntaxVersion(pViewSh->GetDoc()->GetSmSyntaxVersion());
            return;
        }
    }
}

IMPL_LINK(SmElementsPanel, ElementClickHandler, SmElement&, rElement, void)
{
    if (SmViewShell* pViewSh = GetView())
    {
        std::unique_ptr<SfxStringItem> pInsertCommand
            = std::make_unique<SfxStringItem>(SID_INSERTCOMMANDTEXT, rElement.getText());
        pViewSh->GetViewFrame()->GetDispatcher()->ExecuteList(
            SID_INSERTCOMMANDTEXT, SfxCallMode::RECORD, { pInsertCommand.get() });
    }
}

SmViewShell* SmElementsPanel::GetView() const
{
    SfxViewShell* pView = mrBindings.GetDispatcher()->GetFrame()->GetViewShell();
    return dynamic_cast<SmViewShell*>(pView);
}

} // end of namespace sm::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
