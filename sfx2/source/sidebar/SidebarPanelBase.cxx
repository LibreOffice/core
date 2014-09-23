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
#include <sfx2/imagemgr.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/layout.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

using namespace css;
using namespace css::uno;


namespace sfx2 { namespace sidebar {

Reference<ui::XUIElement> SidebarPanelBase::Create (
    const ::rtl::OUString& rsResourceURL,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    vcl::Window* pWindow,
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
    const ::rtl::OUString& rsResourceURL,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    vcl::Window* pWindow,
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
        if (xMultiplexer.is())
            xMultiplexer->addContextChangeEventListener(this, mxFrame->getController());
    }
    if (mpControl != NULL)
    {
        mpControl->SetBackground(Theme::GetWallpaper(Theme::Paint_PanelBackground));
        mpControl->Show();
    }
}




SidebarPanelBase::~SidebarPanelBase (void)
{
}




void SAL_CALL SidebarPanelBase::disposing (void)
    throw (css::uno::RuntimeException)
{
    if (mpControl != NULL)
    {
        delete mpControl;
        mpControl = NULL;
    }

    if (mxFrame.is())
    {
        css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
            css::ui::ContextChangeEventMultiplexer::get(
                ::comphelper::getProcessComponentContext()));
        if (xMultiplexer.is())
            xMultiplexer->removeAllContextChangeEventListeners(this);
        mxFrame = NULL;
    }
}




// XContextChangeEventListener
void SAL_CALL SidebarPanelBase::notifyContextChangeEvent (
    const ui::ContextChangeEventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    IContextChangeReceiver* pContextChangeReceiver
        = dynamic_cast<IContextChangeReceiver*>(mpControl);
    if (pContextChangeReceiver != NULL)
    {
        const EnumContext aContext(
            EnumContext::GetApplicationEnum(rEvent.ApplicationName),
            EnumContext::GetContextEnum(rEvent.ContextName));
        pContextChangeReceiver->HandleContextChange(aContext);
    }
}




void SAL_CALL SidebarPanelBase::disposing (
    const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;

    mxFrame = NULL;
    mpControl = NULL;
}




css::uno::Reference<css::frame::XFrame> SAL_CALL SidebarPanelBase::getFrame (void)
    throw(css::uno::RuntimeException, std::exception)
{
    return mxFrame;
}




::rtl::OUString SAL_CALL SidebarPanelBase::getResourceURL (void)
    throw(css::uno::RuntimeException, std::exception)
{
    return msResourceURL;
}




sal_Int16 SAL_CALL SidebarPanelBase::getType (void)
    throw(css::uno::RuntimeException, std::exception)
{
    return ui::UIElementType::TOOLPANEL;
}




Reference<XInterface> SAL_CALL SidebarPanelBase::getRealInterface (void)
    throw(css::uno::RuntimeException, std::exception)
{
    return Reference<XInterface>(static_cast<XWeak*>(this));
}




Reference<accessibility::XAccessible> SAL_CALL SidebarPanelBase::createAccessible (
    const Reference<accessibility::XAccessible>& rxParentAccessible)
    throw(css::uno::RuntimeException, std::exception)
{
    (void)rxParentAccessible;

    // Not yet implemented.
    return NULL;
}




Reference<awt::XWindow> SAL_CALL SidebarPanelBase::getWindow (void)
    throw(css::uno::RuntimeException, std::exception)
{
    if (mpControl != NULL)
        return Reference<awt::XWindow>(
            mpControl->GetComponentInterface(),
            UNO_QUERY);
    else
        return NULL;
}




ui::LayoutSize SAL_CALL SidebarPanelBase::getHeightForWidth (const sal_Int32 nWidth)
    throw(css::uno::RuntimeException, std::exception)
{
    if (maLayoutSize.Minimum >= 0)
        return maLayoutSize;
    else
    {
        ILayoutableWindow* pLayoutableWindow = dynamic_cast<ILayoutableWindow*>(mpControl);

        if (isLayoutEnabled(mpControl))
        {
            // widget layout-based sidebar
            Size aSize(mpControl->get_preferred_size());
            return ui::LayoutSize(aSize.Height(), aSize.Height(), aSize.Height());
        }
        else if (pLayoutableWindow != NULL)
            return pLayoutableWindow->GetHeightForWidth(nWidth);
        else if (mpControl != NULL)
        {
            const sal_Int32 nHeight (mpControl->GetSizePixel().Height());
            return ui::LayoutSize(nHeight,nHeight,nHeight);
        }
    }

    return ui::LayoutSize(0,0,0);
}

sal_Int32 SAL_CALL SidebarPanelBase::getMinimalWidth () throw(css::uno::RuntimeException, std::exception)
{
    if (isLayoutEnabled(mpControl))
    {
        // widget layout-based sidebar
        Size aSize(mpControl->get_preferred_size());
        return aSize.Width();
    }
    return 0;
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
