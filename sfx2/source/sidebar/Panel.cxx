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

#ifdef DEBUG
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/sidebar/Deck.hxx>
#endif

#include <tools/svborder.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>

#include <boost/bind.hpp>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

Panel::Panel(const PanelDescriptor& rPanelDescriptor,
             vcl::Window* pParentWindow,
             const bool bIsInitiallyExpanded,
             const boost::function<void()>& rDeckLayoutTrigger,
             const boost::function<Context()>& rContextAccess)
    : Window(pParentWindow),
      msPanelId(rPanelDescriptor.msId),
      mpTitleBar(VclPtr<PanelTitleBar>::Create(rPanelDescriptor.msTitle, pParentWindow, this)),
      mbIsTitleBarOptional(rPanelDescriptor.mbIsTitleBarOptional),
      mxElement(),
      mxPanelComponent(),
      mbIsExpanded(bIsInitiallyExpanded),
      maDeckLayoutTrigger(rDeckLayoutTrigger),
      maContextAccess(rContextAccess)
{
    SetBackground(Theme::GetPaint(Theme::Paint_PanelBackground).GetWallpaper());

#ifdef DEBUG
    SetText(OUString("Panel"));
#endif
}

Panel::~Panel()
{
    disposeOnce();
}

void Panel::dispose()
{
    mxPanelComponent = NULL;

    {
        Reference<lang::XComponent> xComponent (mxElement, UNO_QUERY);
        mxElement = NULL;
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

PanelTitleBar* Panel::GetTitleBar() const
{
    return mpTitleBar.get();
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
    if (mbIsExpanded != bIsExpanded)
    {
        mbIsExpanded = bIsExpanded;
        maDeckLayoutTrigger();

        if (maContextAccess)
            ResourceManager::Instance().StorePanelExpansionState(
                msPanelId,
                bIsExpanded,
                maContextAccess());
    }
}

bool Panel::HasIdPredicate (const ::rtl::OUString& rsId) const
{
    return msPanelId.equals(rsId);
}

void Panel::Paint (vcl::RenderContext& rRenderContext, const Rectangle& rUpdateArea)
{
    Window::Paint(rRenderContext, rUpdateArea);
}

void Panel::Resize()
{
    Window::Resize();

    // Forward new size to window of XUIElement.
    Reference<awt::XWindow> xElementWindow (GetElementWindow());
    if(xElementWindow.is())
    {
        const Size aSize(GetSizePixel());
        xElementWindow->setPosSize(
            0, 0, aSize.Width(), aSize.Height(), awt::PosSize::POSSIZE);
    }
}

void Panel::Activate()
{
    Window::Activate();
}

void Panel::DataChanged (const DataChangedEvent& rEvent)
{
    (void)rEvent;
    SetBackground(Theme::GetPaint(Theme::Paint_PanelBackground).GetWallpaper());
}

Reference<awt::XWindow> Panel::GetElementWindow()
{
    if (mxElement.is())
    {
        Reference<ui::XToolPanel> xToolPanel(mxElement->getRealInterface(), UNO_QUERY);
        if (xToolPanel.is())
            return xToolPanel->getWindow();
    }

    return NULL;
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
