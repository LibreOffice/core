/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BasicPaneFactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:31:53 $
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

#ifndef SD_FRAMEWORK_BASIC_PANE_FACTORY_HXX
#define SD_FRAMEWORK_BASIC_PANE_FACTORY_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include "UpdateLockManager.hxx"


#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;


namespace {

typedef ::cppu::WeakComponentImplHelper3 <
    css::lang::XInitialization,
    css::drawing::framework::XResourceFactory,
    css::drawing::framework::XConfigurationChangeListener
    > BasicPaneFactoryInterfaceBase;

} // end of anonymous namespace.


namespace sd {

class ViewShellBase;
}

namespace sd { namespace framework {

/** This factory provides the frequently used standard panes
        private:resource/pane/CenterPane
        private:resource/pane/FullScreenPane
        private:resource/pane/LeftImpressPane
        private:resource/pane/LeftDrawPane
        private:resource/pane/RightPane
    There are two left panes because this is (seems to be) the only way to
    show different titles for the left pane in Draw and Impress.
*/
class BasicPaneFactory
    : private ::cppu::BaseMutex,
      public BasicPaneFactoryInterfaceBase
{
public:
    BasicPaneFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~BasicPaneFactory (void);

    virtual void SAL_CALL disposing (void);


    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


    // XResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL
        releaseResource (
            const css::uno::Reference<css::drawing::framework::XResource>& rxPane)
        throw (css::uno::RuntimeException);


    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException);


    // lang::XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEventObject)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::WeakReference<css::drawing::framework::XConfigurationController>
        mxConfigurationControllerWeak;
    css::uno::WeakReference<css::frame::XController> mxControllerWeak;
    ViewShellBase* mpViewShellBase;
    class PaneDescriptor;
    class PaneContainer;
    ::boost::scoped_ptr<PaneContainer> mpPaneContainer;
    bool mbFirstUpdateSeen;
    ::boost::shared_ptr<UpdateLockManager> mpUpdateLockManager;

    /** Create a new instance of FrameWindowPane.
        @param rPaneId
            There is only one frame window so this id is just checked to
            have the correct value.
    */
    css::uno::Reference<css::drawing::framework::XResource>
        CreateFrameWindowPane (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId);

    /** Create a new pane that represents the center pane in full screen
        mode.
    */
    css::uno::Reference<css::drawing::framework::XResource>
        CreateFullScreenPane (
            const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId);

    /** Create a new instance of ChildWindowPane.
        @param rPaneId
            The ResourceURL member defines which side pane to create.
    */
    css::uno::Reference<css::drawing::framework::XResource>
        CreateChildWindowPane (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxPaneId,
            const PaneDescriptor& rDescriptor);

    /** Return <TRUE/> when the specified resource is bound to one of the
        child windows.
    */
    bool IsBoundToChildWindow (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId) const;

    void ThrowIfDisposed (void) const
        throw (css::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif
