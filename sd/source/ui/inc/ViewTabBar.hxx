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

#ifndef INCLUDED_SD_SOURCE_UI_INC_VIEWTABBAR_HXX
#define INCLUDED_SD_SOURCE_UI_INC_VIEWTABBAR_HXX

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/TabBarButton.hpp>
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#include <com/sun/star/drawing/framework/XToolBar.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <vcl/tabctrl.hxx>
#include <cppuhelper/compbase.hxx>
#include "MutexOwner.hxx"

#include <memory>
#include <vector>

namespace sd {
    class ViewShellBase;
}

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::drawing::framework::XToolBar,
        css::drawing::framework::XTabBar,
        css::drawing::framework::XConfigurationChangeListener,
        css::lang::XUnoTunnel
        > ViewTabBarInterfaceBase;
}

namespace sd {

/** Tab control for switching between views in the center pane.
*/
class ViewTabBar
    : private sd::MutexOwner,
      public ViewTabBarInterfaceBase
{
public:
    ViewTabBar (
        const css::uno::Reference< css::drawing::framework::XResourceId>& rxViewTabBarId,
        const css::uno::Reference< css::frame::XController>& rxController);
    virtual ~ViewTabBar();

    virtual void SAL_CALL disposing() override;

    VclPtr< ::TabControl> GetTabControl() const { return mpTabControl;}

    bool ActivatePage();

    //----- drawing::framework::XConfigurationChangeListener ------------------

    virtual void SAL_CALL
        notifyConfigurationChange (
            const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    //----- XEventListener ----------------------------------------------------

    virtual void SAL_CALL disposing(
        const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    //----- XTabBar -----------------------------------------------------------

    virtual void
        SAL_CALL addTabBarButtonAfter (
            const css::drawing::framework::TabBarButton& rButton,
            const css::drawing::framework::TabBarButton& rAnchor)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void
        SAL_CALL appendTabBarButton (
            const css::drawing::framework::TabBarButton& rButton)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void
        SAL_CALL removeTabBarButton (
            const css::drawing::framework::TabBarButton& rButton)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool
        SAL_CALL hasTabBarButton (
            const css::drawing::framework::TabBarButton& rButton)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<css::drawing::framework::TabBarButton>
        SAL_CALL getTabBarButtons()
        throw (css::uno::RuntimeException, std::exception) override;

    //----- XResource ---------------------------------------------------------

    virtual css::uno::Reference<
        css::drawing::framework::XResourceId> SAL_CALL getResourceId()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isAnchorOnly()
        throw (css::uno::RuntimeException, std::exception) override;

    //----- XUnoTunnel --------------------------------------------------------

    virtual sal_Int64 SAL_CALL getSomething (const css::uno::Sequence<sal_Int8>& rId)
        throw (css::uno::RuntimeException, std::exception) override;

    /** The returned value is calculated as the difference between the
        total height of the control and the heigh of its first tab page.
        This can be considered a hack.
        This procedure works only when the control is visible.  Calling this
        method when the control is not visible results in returning a
        default value.
        To be on the safe side wait for this control to become visible and
        the call this method again.
    */
    int GetHeight();

    void AddTabBarButton (
        const css::drawing::framework::TabBarButton& rButton,
        const css::drawing::framework::TabBarButton& rAnchor);
    void AddTabBarButton (
        const css::drawing::framework::TabBarButton& rButton);
    void RemoveTabBarButton (
        const css::drawing::framework::TabBarButton& rButton);
    bool HasTabBarButton (
        const css::drawing::framework::TabBarButton& rButton);
    css::uno::Sequence<css::drawing::framework::TabBarButton>
        GetTabBarButtons();

private:
    VclPtr< ::TabControl> mpTabControl;
    css::uno::Reference<css::frame::XController> mxController;
    css::uno::Reference<css::drawing::framework::XConfigurationController> mxConfigurationController;
    typedef ::std::vector<css::drawing::framework::TabBarButton> TabBarButtonList;
    TabBarButtonList maTabBarButtons;
    VclPtr<TabPage> mpTabPage;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewTabBarId;
    ViewShellBase* mpViewShellBase;

    void UpdateActiveButton();
    void AddTabBarButton (
        const css::drawing::framework::TabBarButton& rButton,
        sal_Int32 nPosition);
    void UpdateTabBarButtons();

    /** This method is called from the constructor to get the window for an
        anchor ResourceId and pass it to our base class.  It has to be
        static because it must not access any of the, not yet initialized
        members.
    */
    static vcl::Window* GetAnchorWindow(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewTabBarId,
        const css::uno::Reference<css::frame::XController>& rxController);
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
