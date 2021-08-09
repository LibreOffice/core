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
#ifndef INCLUDED_SFX2_SIDEBAR_SIDEBARPANELBASE_HXX
#define INCLUDED_SFX2_SIDEBAR_SIDEBARPANELBASE_HXX

#include <sal/config.h>

#include <memory>

#include <sfx2/dllapi.h>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/ui/XContextChangeEventListener.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>
#include <com/sun/star/ui/XSidebarPanel.hpp>
#include <com/sun/star/ui/XUpdateModel.hpp>

#include <vcl/vclptr.hxx>


namespace vcl { class Window; }

namespace sfx2 { namespace sidebar {

typedef cppu::WeakComponentImplHelper<css::ui::XContextChangeEventListener,
                                       css::ui::XUIElement,
                                       css::ui::XToolPanel,
                                       css::ui::XSidebarPanel,
                                       css::ui::XUpdateModel>
            SidebarPanelBaseInterfaceBase;

/** Base class for sidebar panels that provides some convenience
    functionality.
*/
class SFX2_DLLPUBLIC SidebarPanelBase final : private ::cppu::BaseMutex,
                                        public SidebarPanelBaseInterfaceBase
{
public:
    static css::uno::Reference<css::ui::XUIElement> Create(const OUString& rsResourceURL,
                                                           const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                                           vcl::Window* pControl,
                                                           const css::ui::LayoutSize& rLayoutSize);

    // XContextChangeEventListener
    virtual void SAL_CALL notifyContextChangeEvent (const css::ui::ContextChangeEventObject& rEvent) override;

    // XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent) override;

    // XUIElement
    virtual css::uno::Reference<css::frame::XFrame> SAL_CALL getFrame() override;
    virtual OUString SAL_CALL getResourceURL() override;
    virtual sal_Int16 SAL_CALL getType() override;
    virtual css::uno::Reference<css::uno::XInterface> SAL_CALL getRealInterface() override;

    // XToolPanel
    virtual css::uno::Reference<css::accessibility::XAccessible> SAL_CALL createAccessible(
                const css::uno::Reference<css::accessibility::XAccessible>& rxParentAccessible) override;
    virtual css::uno::Reference<css::awt::XWindow> SAL_CALL getWindow() override;

    // XSidebarPanel
    virtual css::ui::LayoutSize SAL_CALL getHeightForWidth(sal_Int32 nWidth) override;
    virtual sal_Int32 SAL_CALL getMinimalWidth() override;

    // XUpdateModel
    virtual void SAL_CALL updateModel(const css::uno::Reference<css::frame::XModel>& xModel) override;

private:
    SidebarPanelBase(const OUString& rsResourceURL, const css::uno::Reference<css::frame::XFrame>& rxFrame,
                     vcl::Window* pWindow, const css::ui::LayoutSize& rLayoutSize);
    virtual ~SidebarPanelBase() override;
    SidebarPanelBase(const SidebarPanelBase&) = delete;
    SidebarPanelBase& operator=( const SidebarPanelBase& ) = delete;

    virtual void SAL_CALL disposing() override;

    css::uno::Reference<css::frame::XFrame> mxFrame;
    VclPtr<vcl::Window> mpControl;
    const OUString msResourceURL;
    const css::ui::LayoutSize maLayoutSize;
};

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
