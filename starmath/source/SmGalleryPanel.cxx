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

#include "SmGalleryPanel.hxx"
#include <starmath.hrc>
#include <smmod.hxx>
#include <strings.hrc>
#include <view.hxx>

namespace sm::sidebar
{
// static
std::unique_ptr<PanelLayout> SmGalleryPanel::Create(weld::Widget& rParent,
                                                    const SfxBindings& rBindings)
{
    return std::make_unique<SmGalleryPanel>(rParent, rBindings);
}

SmGalleryPanel::SmGalleryPanel(weld::Widget& rParent, const SfxBindings& rBindings)
    : PanelLayout(&rParent, "GalleryMathPanel", "modules/smath/ui/sidebargallery_math.ui")
    , mrBindings(rBindings)
    , mxCategoryList(m_xBuilder->weld_tree_view("categorylist"))
    , mxElementsControl(std::make_unique<SmElementsControl>(
          m_xBuilder->weld_scrolled_window("scrolledwindow", true)))
    , mxElementsControlWin(
          std::make_unique<weld::CustomWeld>(*m_xBuilder, "element_selector", *mxElementsControl))
{
    for (size_t i = 0; i < SmElementsControl::categoriesSize(); ++i)
        mxCategoryList->append_text(SmResId(std::get<0>(SmElementsControl::categories()[i])));

    mxCategoryList->set_size_request(-1, mxCategoryList->get_height_rows(6));

    mxCategoryList->connect_changed(LINK(this, SmGalleryPanel, CategorySelectedHandle));
    mxCategoryList->select_text(SmResId(RID_CATEGORY_UNARY_BINARY_OPERATORS));

    mxElementsControl->setVerticalMode(false);
    mxElementsControl->setElementSetId(RID_CATEGORY_UNARY_BINARY_OPERATORS);
    mxElementsControl->SetSelectHdl(LINK(this, SmGalleryPanel, ElementClickHandler));
}

SmGalleryPanel::~SmGalleryPanel()
{
    mxElementsControlWin.reset();
    mxElementsControl.reset();
    mxCategoryList.reset();
}

IMPL_LINK(SmGalleryPanel, CategorySelectedHandle, weld::TreeView&, rList, void)
{
    const OUString sSelected = rList.get_selected_text();
    for (size_t i = 0; i < SmElementsControl::categoriesSize(); ++i)
    {
        const TranslateId& rCategoryId = std::get<0>(SmElementsControl::categories()[i]);
        OUString aCurrentCategoryString = SmResId(rCategoryId);
        if (aCurrentCategoryString == sSelected)
        {
            mxElementsControl->setElementSetId(rCategoryId);
            mxElementsControl->setSmSyntaxVersion(GetView()->GetDoc()->GetSmSyntaxVersion());
            return;
        }
    }
}

IMPL_LINK(SmGalleryPanel, ElementClickHandler, SmElement&, rElement, void)
{
    SmViewShell* pViewSh = GetView();

    if (pViewSh)
    {
        std::unique_ptr<SfxStringItem> pInsertCommand
            = std::make_unique<SfxStringItem>(SID_INSERTCOMMANDTEXT, rElement.getText());
        pViewSh->GetViewFrame()->GetDispatcher()->ExecuteList(
            SID_INSERTCOMMANDTEXT, SfxCallMode::RECORD, { pInsertCommand.get() });
    }
}

SmViewShell* SmGalleryPanel::GetView() const
{
    SfxViewShell* pView = mrBindings.GetDispatcher()->GetFrame()->GetViewShell();
    return dynamic_cast<SmViewShell*>(pView);
}

} // end of namespace sm::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
