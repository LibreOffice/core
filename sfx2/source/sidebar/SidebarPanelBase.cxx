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
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ILayoutableWindow.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include <vcl/layout.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

Reference<ui::XUIElement> SidebarPanelBase::Create (
    const OUString& rsResourceURL,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    PanelLayout* pWindow,
    const css::ui::LayoutSize& rLayoutSize)
{
    Reference<ui::XUIElement> xUIElement (
        new SidebarPanelBase(
            rsResourceURL,
            rxFrame,
            pWindow,
            rLayoutSize));
    return xUIElement;
}

SidebarPanelBase::SidebarPanelBase (
    const OUString& rsResourceURL,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    PanelLayout* pWindow,
    const css::ui::LayoutSize& rLayoutSize)
    : SidebarPanelBaseInterfaceBase(m_aMutex),
      mxFrame(rxFrame),
      mpControl(pWindow),
      msResourceURL(rsResourceURL),
      maLayoutSize(rLayoutSize)
{
    if (mxFrame.is())
    {
        css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
            css::ui::ContextChangeEventMultiplexer::get(
                ::comphelper::getProcessComponentContext()));
        xMultiplexer->addContextChangeEventListener(this, mxFrame->getController());
    }
    if (mpControl != nullptr)
    {
        mpControl->SetBackground(Theme::GetColor(Theme::Color_PanelBackground));
        mpControl->Show();
    }
}

SidebarPanelBase::~SidebarPanelBase()
{
}

void SAL_CALL SidebarPanelBase::disposing()
{
    SolarMutexGuard aGuard;

    mpControl.disposeAndClear();

    if (mxFrame.is())
    {
        css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
            css::ui::ContextChangeEventMultiplexer::get(
                ::comphelper::getProcessComponentContext()));
        xMultiplexer->removeAllContextChangeEventListeners(this);
        mxFrame = nullptr;
    }
}

// XContextChangeEventListener
void SAL_CALL SidebarPanelBase::notifyContextChangeEvent (
    const ui::ContextChangeEventObject& rEvent)
{
    SolarMutexGuard aGuard;

    IContextChangeReceiver* pContextChangeReceiver
        = dynamic_cast<IContextChangeReceiver*>(mpControl.get());
    if (pContextChangeReceiver != nullptr)
    {
        const vcl::EnumContext aContext(
            vcl::EnumContext::GetApplicationEnum(rEvent.ApplicationName),
            vcl::EnumContext::GetContextEnum(rEvent.ContextName));
        pContextChangeReceiver->HandleContextChange(aContext);
    }
}

void SAL_CALL SidebarPanelBase::disposing (
    const css::lang::EventObject&)
{
    SolarMutexGuard aGuard;

    mxFrame = nullptr;
    mpControl = nullptr;
}

css::uno::Reference<css::frame::XFrame> SAL_CALL SidebarPanelBase::getFrame()
{
    return mxFrame;
}

OUString SAL_CALL SidebarPanelBase::getResourceURL()
{
    return msResourceURL;
}

sal_Int16 SAL_CALL SidebarPanelBase::getType()
{
    return ui::UIElementType::TOOLPANEL;
}

Reference<XInterface> SAL_CALL SidebarPanelBase::getRealInterface()
{
    return Reference<XInterface>(static_cast<XWeak*>(this));
}

Reference<accessibility::XAccessible> SAL_CALL SidebarPanelBase::createAccessible (
    const Reference<accessibility::XAccessible>&)
{
    // Not yet implemented.
    return nullptr;
}

Reference<awt::XWindow> SAL_CALL SidebarPanelBase::getWindow()
{
    SolarMutexGuard aGuard;

    if (mpControl != nullptr)
        return Reference<awt::XWindow>(
            mpControl->GetComponentInterface(),
            UNO_QUERY);
    else
        return nullptr;
}

ui::LayoutSize SAL_CALL SidebarPanelBase::getHeightForWidth (const sal_Int32 nWidth)
{
    SolarMutexGuard aGuard;

    if (maLayoutSize.Minimum >= 0)
        return maLayoutSize;
    else
    {
        ILayoutableWindow* pLayoutableWindow = dynamic_cast<ILayoutableWindow*>(mpControl.get());
        if (pLayoutableWindow)
            return pLayoutableWindow->GetHeightForWidth(nWidth);
        else if (isLayoutEnabled(mpControl))
        {
            // widget layout-based sidebar
            Size aSize(mpControl->get_preferred_size());
            return ui::LayoutSize(aSize.Height(), aSize.Height(), aSize.Height());
        }
        else if (mpControl != nullptr)
        {
            const sal_Int32 nHeight (mpControl->GetSizePixel().Height());
            return ui::LayoutSize(nHeight,nHeight,nHeight);
        }
    }

    return ui::LayoutSize(0,0,0);
}

sal_Int32 SAL_CALL SidebarPanelBase::getMinimalWidth ()
{
    SolarMutexGuard aGuard;

    if (isLayoutEnabled(mpControl))
    {
        // widget layout-based sidebar
        Size aSize(mpControl->get_preferred_size());
        return aSize.Width();
    }
    return 0;
}

void SAL_CALL SidebarPanelBase::updateModel(const css::uno::Reference<css::frame::XModel>& xModel)
{
    SolarMutexGuard aGuard;

    SidebarModelUpdate* pModelUpdate = dynamic_cast<SidebarModelUpdate*>(mpControl.get());
    if (!pModelUpdate)
        return;

    pModelUpdate->updateModel(xModel);
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
