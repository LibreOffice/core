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

#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace sd {
    class ViewShellBase;
}

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        ::com::sun::star::drawing::framework::XToolBar,
        ::com::sun::star::drawing::framework::XTabBar,
        ::com::sun::star::drawing::framework::XConfigurationChangeListener,
        ::com::sun::star::lang::XUnoTunnel
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
        const ::com::sun::star::uno::Reference<
            com::sun::star::drawing::framework::XResourceId>& rxViewTabBarId,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XController>& rxController);
    virtual ~ViewTabBar();

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    VclPtr< ::TabControl> GetTabControl() const { return mpTabControl;}

    bool ActivatePage();

    //----- drawing::framework::XConfigurationChangeListener ------------------

    virtual void SAL_CALL
        notifyConfigurationChange (
            const ::com::sun::star::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //----- XEventListener ----------------------------------------------------

    virtual void SAL_CALL disposing(
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //----- XTabBar -----------------------------------------------------------

    virtual void
        SAL_CALL addTabBarButtonAfter (
            const ::com::sun::star::drawing::framework::TabBarButton& rButton,
            const ::com::sun::star::drawing::framework::TabBarButton& rAnchor)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void
        SAL_CALL appendTabBarButton (
            const ::com::sun::star::drawing::framework::TabBarButton& rButton)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void
        SAL_CALL removeTabBarButton (
            const ::com::sun::star::drawing::framework::TabBarButton& rButton)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool
        SAL_CALL hasTabBarButton (
            const ::com::sun::star::drawing::framework::TabBarButton& rButton)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence<com::sun::star::drawing::framework::TabBarButton>
        SAL_CALL getTabBarButtons()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //----- XResource ---------------------------------------------------------

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XResourceId> SAL_CALL getResourceId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isAnchorOnly()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //----- XUnoTunnel --------------------------------------------------------

    virtual sal_Int64 SAL_CALL getSomething (const com::sun::star::uno::Sequence<sal_Int8>& rId)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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
        const ::com::sun::star::drawing::framework::TabBarButton& rButton,
        const ::com::sun::star::drawing::framework::TabBarButton& rAnchor);
    void AddTabBarButton (
        const ::com::sun::star::drawing::framework::TabBarButton& rButton);
    void RemoveTabBarButton (
        const ::com::sun::star::drawing::framework::TabBarButton& rButton);
    bool HasTabBarButton (
        const ::com::sun::star::drawing::framework::TabBarButton& rButton);
    ::com::sun::star::uno::Sequence<com::sun::star::drawing::framework::TabBarButton>
        GetTabBarButtons();

private:
    VclPtr< ::TabControl> mpTabControl;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XController> mxController;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfigurationController> mxConfigurationController;
    typedef ::std::vector<com::sun::star::drawing::framework::TabBarButton> TabBarButtonList;
    TabBarButtonList maTabBarButtons;
    VclPtr<TabPage> mpTabPage;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XResourceId> mxViewTabBarId;
    ViewShellBase* mpViewShellBase;

    void UpdateActiveButton();
    void AddTabBarButton (
        const ::com::sun::star::drawing::framework::TabBarButton& rButton,
        sal_Int32 nPosition);
    void UpdateTabBarButtons();

    /** This method is called from the constructor to get the window for an
        anchor ResourceId and pass it to our base class.  It has to be
        static because it must not access any of the, not yet initialized
        members.
    */
    static vcl::Window* GetAnchorWindow(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XResourceId>& rxViewTabBarId,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XController>& rxController);
    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
