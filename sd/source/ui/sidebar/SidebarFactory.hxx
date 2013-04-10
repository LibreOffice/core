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

#ifndef SD_SIDEBAR_FACTORY_HXX
#define SD_SIDEBAR_FACTORY_HXX

#include <cppuhelper/compbase4.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ref.hxx>
#include "framework/Pane.hxx"

#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;


namespace sd {
    class ViewShellBase;
}

namespace sd { namespace sidebar {

namespace
{
    typedef ::cppu::WeakComponentImplHelper3 <
        css::lang::XInitialization,
        css::ui::XUIElementFactory,
        css::lang::XEventListener
        > SidebarFactoryInterfaceBase;
}


/** This factory creates both XUIElements (for sidebar panels) and
    a drawing framework pane.

    The drawing framework pane is a container for the SidebarViewShell
    which is necessary to run the legacy implementations of the task
    pane panels.

    Control and information flow is like this:

    When one of the old task panels is requested to be displayed in
    the sidebar this factory is called for
    XUIElementFactory::createUIElement().
    One of the arguments, the window, is then exported into the
    drawing framework as pane.  After this the drawing framework is
    used to create the SidebarViewShell (once known as
    TaskPaneViewShell or ToolPanelViewShell) and the requested panel.
*/
class SidebarFactory
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public SidebarFactoryInterfaceBase
{
public:
    static ::rtl::OUString SAL_CALL getImplementationName (void);
    static cssu::Reference<cssu::XInterface> SAL_CALL createInstance (
        const cssu::Reference<css::lang::XMultiServiceFactory>& rxFactory);
    static cssu::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames (void);

    SidebarFactory (const cssu::Reference<cssu::XComponentContext>& rxContext);
    virtual ~SidebarFactory (void);

    virtual void SAL_CALL disposing (void);


    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


    // XUIElementFactory

    cssu::Reference<css::ui::XUIElement> SAL_CALL createUIElement (
        const ::rtl::OUString& rsResourceURL,
        const ::cssu::Sequence<css::beans::PropertyValue>& rArguments)
        throw(
            css::container::NoSuchElementException,
            css::lang::IllegalArgumentException,
            cssu::RuntimeException);


    // XEventListener

    virtual void SAL_CALL disposing (const ::css::lang::EventObject& rEvent)
        throw(cssu::RuntimeException);
};


} } // end of namespace sd::sidebar

#endif
