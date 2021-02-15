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

#include <sidebar/DeckTitleBar.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>

#include <vcl/customweld.hxx>
#include <vcl/ptrstyle.hxx>

#ifdef DEBUG
#include <sfx2/sidebar/Tools.hxx>
#endif

namespace sfx2::sidebar {

class GripWidget : public weld::CustomWidgetController
{
private:
    BitmapEx maGrip;
public:
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override
    {
        weld::CustomWidgetController::SetDrawingArea(pDrawingArea);
        StyleUpdated();
    }

    virtual void StyleUpdated() override
    {
        maGrip = BitmapEx("sfx2/res/grip.png");
        Size aGripSize(maGrip.GetSizePixel());
        set_size_request(aGripSize.Width(), aGripSize.Height());
        weld::CustomWidgetController::StyleUpdated();
    }

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/) override
    {
        rRenderContext.SetBackground(Theme::GetColor(Theme::Color_DeckTitleBarBackground));
        rRenderContext.DrawBitmapEx(Point(0, 0), maGrip);
    }
};

DeckTitleBar::DeckTitleBar (const OUString& rsTitle,
                            vcl::Window* pParentWindow,
                            const std::function<void()>& rCloserAction)
    : TitleBar(pParentWindow, "sfx/ui/decktitlebar.ui", "DeckTitleBar",
               Theme::Color_DeckTitleBarBackground)
    , mxGripWidget(new GripWidget)
    , mxGripWeld(new weld::CustomWeld(*m_xBuilder, "grip", *mxGripWidget))
    , mxLabel(m_xBuilder->weld_label("label"))
    , maCloserAction(rCloserAction)
    , mbIsCloserVisible(false)
{
    mxLabel->set_label(rsTitle);
    mxGripWidget->SetPointer(PointerStyle::Move);

    OSL_ASSERT(pParentWindow != nullptr);

    if (maCloserAction)
        SetCloserVisible(true);

#ifdef DEBUG
    SetText(OUString("DeckTitleBar"));
#endif
}

DeckTitleBar::~DeckTitleBar()
{
    disposeOnce();
}

void DeckTitleBar::dispose()
{
    mxLabel.reset();
    mxGripWeld.reset();
    mxGripWidget.reset();
    TitleBar::dispose();
}

tools::Rectangle DeckTitleBar::GetDragArea()
{
    int x, y, width, height;
    if (mxGripWidget->GetDrawingArea()->get_extents_relative_to(*m_xContainer, x, y, width, height))
        return tools::Rectangle(Point(x, y), Size(width, height));
    return tools::Rectangle();
}

void DeckTitleBar::SetTitle(const OUString& rsTitle)
{
    mxLabel->set_label(rsTitle);
}

OUString DeckTitleBar::GetTitle() const
{
    return mxLabel->get_label();
}

void DeckTitleBar::SetCloserVisible (const bool bIsCloserVisible)
{
    if (mbIsCloserVisible == bIsCloserVisible)
        return;

    mbIsCloserVisible = bIsCloserVisible;

    mxToolBox->set_item_visible("button", mbIsCloserVisible);
}

void DeckTitleBar::HandleToolBoxItemClick()
{
    if (maCloserAction)
        maCloserAction();
}

void DeckTitleBar::DataChanged (const DataChangedEvent& rEvent)
{
    mxToolBox->set_item_icon_name("button", "sfx2/res/closedoc.png");
    TitleBar::DataChanged(rEvent);
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
