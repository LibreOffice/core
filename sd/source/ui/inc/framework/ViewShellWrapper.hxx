/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewShellWrapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:04:58 $
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

#ifndef SD_FRAMEWORK_VIEW_SHELL_WRAPPER_HXX
#define SD_FRAMEWORK_VIEW_SHELL_WRAPPER_HXX

#include "MutexOwner.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XVIEW_HPP_
#include <com/sun/star/drawing/framework/XView.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XRELOCATABLERESOURCE_HPP_
#include <com/sun/star/drawing/framework/XRelocatableResource.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif

#include <boost/shared_ptr.hpp>

namespace {

typedef ::cppu::WeakComponentImplHelper4 <
      ::com::sun::star::drawing::framework::XView,
      ::com::sun::star::lang::XUnoTunnel,
      ::com::sun::star::awt::XWindowListener,
      ::com::sun::star::drawing::framework::XRelocatableResource
    > ViewShellWrapperInterfaceBase;

} // end of anonymous namespace.

namespace sd { class ViewShell; }

namespace sd { namespace framework {

/** This class wraps ViewShell objects and makes them look like an XView.
    Most importantly it provides a tunnel to the ViewShell implementation.
    Then it forwards size changes of the pane window to the view shell.
*/
class ViewShellWrapper
    : private sd::MutexOwner,
      public ViewShellWrapperInterfaceBase
{
public:
    /** Create a new ViewShellWrapper object that wraps the given ViewShell
        object.
        @param pViewShell
            The ViewShell object to wrap.
        @param rsViewURL
            URL of the view type of the wrapped view shell.
        @param rxWindow
            This window reference is optional.  When a valid reference is
            given then size changes of the referenced window are forwarded
            to the ViewShell object.
    */
    ViewShellWrapper (
        ::boost::shared_ptr<ViewShell> pViewShell,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XResourceId>& rxViewId,
        const ::com::sun::star::uno::Reference<com::sun::star::awt::XWindow>& rxWindow);
    virtual ~ViewShellWrapper (void);

    virtual void SAL_CALL disposing (void);

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId (void);

    /** This method is typically used together with the XUnoTunnel interface
        to obtain a pointer to the wrapped ViewShell object for a given
        XView object.
    */
    ::boost::shared_ptr<ViewShell> GetViewShell (void);

    /** Returns whether there is exactly one reference to the called
        ViewShellWrapper object (the number of references to the wrapped
        ViewShell object is not taken into account).  This method is
        typically used by the owner of a ViewShellWrapper object to verify
        that, at the end of the ViewShellWrapper object's lifetime, the
        owner holds the last reference and by releasing it will destroy the
        object.
    */
    bool IsUnique (void);


    // XUnoTunnel

    virtual sal_Int64 SAL_CALL getSomething (const com::sun::star::uno::Sequence<sal_Int8>& rId)
        throw (com::sun::star::uno::RuntimeException);


    // XResource

    virtual ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>
        SAL_CALL getResourceId (void)
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAnchorOnly (void)
        throw (com::sun::star::uno::RuntimeException);


    // XRelocatableResource

    virtual sal_Bool SAL_CALL relocateToAnchor (
        const ::com::sun::star::uno::Reference<
            com::sun::star::drawing::framework::XResource>& xResource)
        throw (com::sun::star::uno::RuntimeException);


    // XWindowListener

    virtual void SAL_CALL windowResized(
        const ::com::sun::star::awt::WindowEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL windowMoved(
        const ::com::sun::star::awt::WindowEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL windowShown(
        const ::com::sun::star::lang::EventObject& rEvent)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL windowHidden(
        const ::com::sun::star::lang::EventObject& rEvent)
        throw (::com::sun::star::uno::RuntimeException);


    // XEventListener

    virtual void SAL_CALL disposing(
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException);

private:
    ::boost::shared_ptr<ViewShell> mpViewShell;
    const ::com::sun::star::uno::Reference<
        com::sun::star::drawing::framework::XResourceId> mxViewId;
    ::com::sun::star::uno::Reference<com::sun::star::awt::XWindow> mxWindow;
};

} } // end of namespace sd::framework

#endif
