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

#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/PanelTitleBar.hxx>
#include <sfx2/sidebar/PanelDescriptor.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/Paint.hxx>
#include <sfx2/sidebar/ResourceManager.hxx>

#include <sfx2/sidebar/SidebarController.hxx>


#ifdef DEBUG
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/sidebar/Deck.hxx>
#endif

#include <tools/svborder.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

Panel::Panel(const PanelDescriptor& rPanelDescriptor,
             vcl::Window* pParentWindow,
             const bool bIsInitiallyExpanded,
             const std::function<void()>& rDeckLayoutTrigger,
             const std::function<Context()>& rContextAccess,
             const css::uno::Reference<css::frame::XFrame>& rxFrame
            )
    : Window(pParentWindow)
    , msPanelId(rPanelDescriptor.msId)
    , mpTitleBar(VclPtr<PanelTitleBar>::Create(rPanelDescriptor.msTitle, pParentWindow, this))
    , mbIsTitleBarOptional(rPanelDescriptor.mbIsTitleBarOptional)
    , mxElement()
    , mxPanelComponent()
    , mbIsExpanded(bIsInitiallyExpanded)
    , maDeckLayoutTrigger(rDeckLayoutTrigger)
    , maContextAccess(rContextAccess)
    , mxFrame(rxFrame)
{
#ifdef DEBUG
    SetText(OUString("Panel"));
#endif
}

Panel::~Panel()
{
    disposeOnce();
    assert(!mpTitleBar);
}

void Panel::ApplySettings(vcl::RenderContext& rRenderContext)
{
    rRenderContext.SetBackground(Theme::GetPaint(Theme::Paint_PanelBackground).GetWallpaper());
}

void Panel::dispose()
{
    mxPanelComponent = nullptr;

    {
        Reference<lang::XComponent> xComponent (mxElement, UNO_QUERY);
        mxElement = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }

    {
        Reference<lang::XComponent> xComponent (GetElementWindow(), UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }

    mpTitleBar.disposeAndClear();

    vcl::Window::dispose();
}

VclPtr<PanelTitleBar> Panel::GetTitleBar() const
{
    return mpTitleBar;
}

void Panel::SetUIElement (const Reference<ui::XUIElement>& rxElement)
{
    mxElement = rxElement;
    if (mxElement.is())
    {
        mxPanelComponent.set(mxElement->getRealInterface(), UNO_QUERY);
    }
}

void Panel::SetExpanded (const bool bIsExpanded)
{
    SidebarController* pSidebarController = SidebarController::GetSidebarControllerForFrame(mxFrame);

    if (mbIsExpanded != bIsExpanded)
    {
        mbIsExpanded = bIsExpanded;
        maDeckLayoutTrigger();

        if (maContextAccess && pSidebarController)
        {
            pSidebarController->GetResourceManager()->StorePanelExpansionState(
                msPanelId,
                bIsExpanded,
                maContextAccess());
        }
    }
}

bool Panel::HasIdPredicate (const OUString& rsId) const
{
    return msPanelId.equals(rsId);
}

void Panel::Resize()
{
    Window::Resize();

    // Forward new size to window of XUIElement.
    Reference<awt::XWindow> xElementWindow (GetElementWindow());
    if(xElementWindow.is())
    {
        const Size aSize(GetSizePixel());
        xElementWindow->setPosSize(0, 0, aSize.Width(), aSize.Height(),
                                   awt::PosSize::POSSIZE);
    }
}

void Panel::DataChanged (const DataChangedEvent& rEvent)
{
    (void)rEvent;
    Invalidate();
}

Reference<awt::XWindow> Panel::GetElementWindow()
{
    if (mxElement.is())
    {
        Reference<ui::XToolPanel> xToolPanel(mxElement->getRealInterface(), UNO_QUERY);
        if (xToolPanel.is())
            return xToolPanel->getWindow();
    }

    return nullptr;
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
