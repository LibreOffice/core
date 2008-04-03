/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewTabBar.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:00:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_VIEW_TAB_BAR_HXX
#define SD_VIEW_TAB_BAR_HXX

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XPANE_HPP_
#include <com/sun/star/drawing/framework/XPane.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_TABBARBUTTON_HPP_
#include <com/sun/star/drawing/framework/TabBarButton.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XTABBAR_HPP_
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XTOOLBAR_HPP_
#include <com/sun/star/drawing/framework/XToolBar.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCONTROLLER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCHANGELISTENER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _VCL_TABCTRL_HXX_
#include <vcl/tabctrl.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif
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

