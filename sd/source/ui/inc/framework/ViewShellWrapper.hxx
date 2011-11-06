/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SD_FRAMEWORK_VIEW_SHELL_WRAPPER_HXX
#define SD_FRAMEWORK_VIEW_SHELL_WRAPPER_HXX

#include "MutexOwner.hxx"
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/drawing/framework/XRelocatableResource.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase4.hxx>
#include <cppuhelper/implbase1.hxx>

#include <boost/shared_ptr.hpp>

namespace {

typedef ::cppu::WeakComponentImplHelper4    <   ::com::sun::star::lang::XUnoTunnel
                                            ,   ::com::sun::star::awt::XWindowListener
                                            ,   ::com::sun::star::drawing::framework::XRelocatableResource
                                            ,   ::com::sun::star::drawing::framework::XView
                                            >   ViewShellWrapperInterfaceBase;

} // end of anonymous namespace.

namespace sd { class ViewShell; }

namespace sd { namespace framework {

/** This class wraps ViewShell objects and makes them look like an XView.
    Most importantly it provides a tunnel to the ViewShell implementation.
    Then it forwards size changes of the pane window to the view shell.
*/
class ViewShellWrapper  :private sd::MutexOwner
                        ,public ViewShellWrapperInterfaceBase
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
    ::boost::shared_ptr< ViewShell >                                                            mpViewShell;
    const ::com::sun::star::uno::Reference< com::sun::star::drawing::framework::XResourceId >   mxViewId;
    ::com::sun::star::uno::Reference<com::sun::star::awt::XWindow >                             mxWindow;
};

} } // end of namespace sd::framework

#endif
