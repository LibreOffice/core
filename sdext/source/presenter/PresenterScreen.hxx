/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterScreen.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:03:26 $
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

#ifndef SDEXT_PRESENTER_PRESENTER_SCREEN_HXX
#define SDEXT_PRESENTER_PRESENTER_SCREEN_HXX

#include "PresenterConfigurationAccess.hxx"
#include "PresenterPaneContainer.hxx"
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <rtl/ref.hxx>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

class PresenterWindowManager;
class PresenterController;

namespace {
    typedef ::cppu::WeakComponentImplHelper2 <
        css::task::XJob,
        css::document::XEventListener    > PresenterScreenInterfaceBase;
}


/** This is the bootstrap class of the presenter screen.  It is registered
    as drawing framework startup service.  That means that every drawing
    framework instance creates an instance of this class.

    <p>A PresenterScreen object registers itself as listener for drawing
    framework configuration changes.  It waits for the full screen marker (a
    top level resource) to appear in the current configuration.  When that
    happens the actual presenter screen is initialized.  A new
    PresenterController is created and takes over the task of controlling
    the presenter screen.</p>
*/
class PresenterScreen
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterScreenInterfaceBase
{
public:
    void SAL_CALL disposing (void);

    static ::rtl::OUString getImplementationName_static (void);
    static css::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static (void);
    static css::uno::Reference<css::uno::XInterface> Create(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext)
        SAL_THROW((css::uno::Exception));


    // XJob

    virtual css::uno::Any SAL_CALL execute(
        const css::uno::Sequence<css::beans::NamedValue >& Arguments)
        throw (css::lang::IllegalArgumentException,
            css::uno::Exception,
            css::uno::RuntimeException);

    // document::XEventListener

    virtual void SAL_CALL notifyEvent( const css::document::EventObject& Event ) throw (css::uno::RuntimeException);

    // XEventListener

    virtual void SAL_CALL disposing ( const css::lang::EventObject& rEvent) throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::frame::XModel2 > mxModel;
    css::uno::Reference<css::frame::XController> mxController;
    css::uno::WeakReference<css::drawing::framework::XConfigurationController>
        mxConfigurationControllerWeak;
    css::uno::WeakReference<css::uno::XComponentContext> mxContextWeak;
    css::uno::WeakReference<css::presentation::XSlideShowController> mxSlideShowControllerWeak;
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::drawing::framework::XResourceId> mxSlideShowViewId;
    css::uno::Reference<css::drawing::framework::XConfiguration> mxSavedConfiguration;
    ::rtl::Reference<PresenterPaneContainer> mpPaneContainer;
    sal_Int32 mnComponentIndex;
    css::uno::Reference<css::drawing::framework::XResourceFactory> mxPaneFactory;
    css::uno::Reference<css::drawing::framework::XResourceFactory> mxViewFactory;

    PresenterScreen (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterScreen (void);

    /** Make the presenter screen visible.
    */
    void InitializePresenterScreen (void);

    /** Deactivate the currently active panes to make room for the full
        screen pane and the presenter panes.
    */
    void DeactivatePanes (
        const css::uno::Reference<css::drawing::framework::XConfigurationController>& rxCC);

    void ShutdownPresenterScreen (void);

    /** Create and initialize the factory for presenter view specific panes.
    */
    void SetupPaneFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    /** Create and initialize the factory for presenter view specific views.
    */
    void SetupViewFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    /** Read the current layout from the configuration and call
        ProcessLayout to bring it on to the screen.
    */
    void SetupConfiguration (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId);

    /** Read one layout from the configuration and make resource activation
        requests to bring it on to the screen.  When one layout references a
        parent layout then this method calls itself recursively.
    */
    void ProcessLayout (
        PresenterConfigurationAccess& rConfiguration,
        const ::rtl::OUString& rsLayoutName,
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId);

    /** Called by ProcessLayout for a single entry of a Layouts
        configuration list.
    */
    void ProcessComponent (
        const ::rtl::OUString& rsKey,
        const ::std::vector<css::uno::Any>& rValues,
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId);

    css::uno::Reference<css::drawing::framework::XResourceId> SetupView (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId,
        const ::rtl::OUString& rsPaneURL,
        const ::rtl::OUString& rsViewURL,
        const ::rtl::OUString& rsTitle,
        const PresenterPaneContainer::ViewInitializationFunction& rViewInitialization,
        const double nLeft,
        const double nTop,
        const double nRight,
        const double nBottom);

    void ThrowIfDisposed (void) const throw (::com::sun::star::lang::DisposedException);
};

} }

#endif
