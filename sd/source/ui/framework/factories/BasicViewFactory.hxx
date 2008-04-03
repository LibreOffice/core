/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BasicViewFactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:32:41 $
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

#ifndef SD_FRAMEWORK_BASIC_VIEW_FACTORY_HXX
#define SD_FRAMEWORK_BASIC_VIEW_FACTORY_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/compbase2.hxx>
#include <osl/mutex.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sd {
class ViewShell;
class ViewShellBase;
class FrameView;
}
class SfxViewFrame;
class Window;

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
    css::drawing::framework::XResourceFactory,
    css::lang::XInitialization
    > BasicViewFactoryInterfaceBase;

} // end of anonymous namespace.




namespace sd { namespace framework {

/** Factory for the frequently used standard views of the drawing framework:
        private:resource/view/
        private:resource/view/ImpressView
        private:resource/view/GraphicView
        private:resource/view/OutlineView
        private:resource/view/NotesView
        private:resource/view/HandoutView
        private:resource/view/SlideSorter
        private:resource/view/PresentationView
        private:resource/view/TaskPane
    For some views in some panes this class also acts as a cache.
*/
class BasicViewFactory
    : private sd::MutexOwner,
      public BasicViewFactoryInterfaceBase
{
public:
    BasicViewFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~BasicViewFactory (void);

    virtual void SAL_CALL disposing (void);


    // XViewFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL releaseResource (
        const css::uno::Reference<css::drawing::framework::XResource>& xView)
        throw(css::uno::RuntimeException);


    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);

private:
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;
    class ViewDescriptor;
    class ViewShellContainer;
    ::boost::scoped_ptr<ViewShellContainer> mpViewShellContainer;
    ViewShellBase* mpBase;
    FrameView* mpFrameView;

    class ViewCache;
    ::boost::shared_ptr<ViewCache> mpViewCache;

    css::uno::Reference<css::drawing::framework::XPane> mxLocalPane;

    ::boost::shared_ptr<ViewDescriptor> CreateView (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        SfxViewFrame& rFrame,
        ::Window& rWindow,
        const css::uno::Reference<css::drawing::framework::XPane>& rxPane,
        FrameView* pFrameView);

    ::boost::shared_ptr<ViewShell> CreateViewShell (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        SfxViewFrame& rFrame,
        ::Window& rWindow,
        FrameView* pFrameView);

    void ActivateCenterView (
        const ::boost::shared_ptr<ViewDescriptor>& rpDescriptor);

    void ReleaseView (
        const ::boost::shared_ptr<ViewDescriptor>& rpDescriptor,
        bool bDoNotCache = false);

    bool IsCacheable (
        const ::boost::shared_ptr<ViewDescriptor>& rpDescriptor);

    ::boost::shared_ptr<ViewDescriptor> GetViewFromCache (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::drawing::framework::XPane>& rxPane);
};

} } // end of namespace sd::framework

#endif
