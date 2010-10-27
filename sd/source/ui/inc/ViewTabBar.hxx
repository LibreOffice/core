/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_VIEW_TAB_BAR_HXX
#define SD_VIEW_TAB_BAR_HXX

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/TabBarButton.hpp>
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#include <com/sun/star/drawing/framework/XToolBar.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <vcl/tabctrl.hxx>
#include <cppuhelper/compbase4.hxx>
#include "MutexOwner.hxx"

#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace sd { namespace tools {
    class EventMultiplexerEvent;
} }

namespace sd {
    class ViewShellBase;
    class PaneManagerEvent;
}

namespace {
    typedef ::cppu::WeakComponentImplHelper4 <
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
    virtual ~ViewTabBar (void);

    virtual void SAL_CALL disposing (void);

    ::boost::shared_ptr< ::TabControl> GetTabControl (void) const;

    bool ActivatePage (void);

    //----- drawing::framework::XConfigurationChangeListener ------------------

    virtual void SAL_CALL
        notifyConfigurationChange (
            const ::com::sun::star::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException);


    //----- XEventListener ----------------------------------------------------

    virtual void SAL_CALL disposing(
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException);


    //----- XTabBar -----------------------------------------------------------

    virtual void
        SAL_CALL addTabBarButtonAfter (
            const ::com::sun::star::drawing::framework::TabBarButton& rButton,
            const ::com::sun::star::drawing::framework::TabBarButton& rAnchor)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void
        SAL_CALL appendTabBarButton (
            const ::com::sun::star::drawing::framework::TabBarButton& rButton)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void
        SAL_CALL removeTabBarButton (
            const ::com::sun::star::drawing::framework::TabBarButton& rButton)
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool
        SAL_CALL hasTabBarButton (
            const ::com::sun::star::drawing::framework::TabBarButton& rButton)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence<com::sun::star::drawing::framework::TabBarButton>
        SAL_CALL getTabBarButtons (void)
        throw (::com::sun::star::uno::RuntimeException);


    //----- XResource ---------------------------------------------------------

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XResourceId> SAL_CALL getResourceId (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAnchorOnly (void)
        throw (com::sun::star::uno::RuntimeException);


    //----- XUnoTunnel --------------------------------------------------------

    virtual sal_Int64 SAL_CALL getSomething (const com::sun::star::uno::Sequence<sal_Int8>& rId)
        throw (com::sun::star::uno::RuntimeException);

    // ------------------------------------------------------------------------

    /** The returned value is calculated as the difference between the
        total height of the control and the heigh of its first tab page.
        This can be considered a hack.
        This procedure works only when the control is visible.  Calling this
        method when the control is not visible results in returning a
        default value.
        To be on the safe side wait for this control to become visible and
        the call this method again.
    */
    int GetHeight (void);

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
        GetTabBarButtons (void);

private:
    ::boost::shared_ptr< ::TabControl> mpTabControl;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XController> mxController;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfigurationController> mxConfigurationController;
    typedef ::std::vector<com::sun::star::drawing::framework::TabBarButton> TabBarButtonList;
    TabBarButtonList maTabBarButtons;
    ::boost::scoped_ptr<TabPage> mpTabPage;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XResourceId> mxViewTabBarId;
    ViewShellBase* mpViewShellBase;

    void UpdateActiveButton (void);
    void AddTabBarButton (
        const ::com::sun::star::drawing::framework::TabBarButton& rButton,
        sal_Int32 nPosition);
    void UpdateTabBarButtons (void);

    /** This method is called from the constructor to get the window for an
        anchor ResourceId and pass it to our base class.  It has to be
        static because it must not access any of the, not yet initialized
        members.
    */
    static ::Window* GetAnchorWindow(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XResourceId>& rxViewTabBarId,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XController>& rxController);
    const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId (void);
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
