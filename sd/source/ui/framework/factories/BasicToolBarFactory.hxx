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



#ifndef SD_FRAMEWORK_BASIC_TOOL_BAR_FACTORY_HXX
#define SD_FRAMEWORK_BASIC_TOOL_BAR_FACTORY_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/basemutex.hxx>


namespace css = ::com::sun::star;

namespace {

typedef ::cppu::WeakComponentImplHelper3 <
    css::drawing::framework::XResourceFactory,
    css::lang::XInitialization,
    css::lang::XEventListener
    > BasicToolBarFactoryInterfaceBase;

} // end of anonymous namespace.

namespace sd {
class ViewShellBase;
}

namespace sd { namespace framework {

/** This factory provides some of the frequently used tool bars:
        private:resource/toolbar/ViewTabBar
*/
class BasicToolBarFactory
    : protected ::cppu::BaseMutex,
      public BasicToolBarFactoryInterfaceBase
{
public:
    BasicToolBarFactory (
        const css::uno::Reference<com::sun::star::uno::XComponentContext>& rxContext);
    virtual ~BasicToolBarFactory (void);

    virtual void SAL_CALL disposing (void);


    // ToolBarFactory

    virtual css::uno::Reference<com::sun::star::drawing::framework::XResource> SAL_CALL
        createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxToolBarId)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException);

    virtual void SAL_CALL
        releaseResource (
            const css::uno::Reference<com::sun::star::drawing::framework::XResource>&
                rxToolBar)
        throw (css::uno::RuntimeException);


    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<com::sun::star::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


    // lang::XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEventObject)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::drawing::framework::XConfigurationController> mxConfigurationController;
    css::uno::Reference<css::frame::XController> mxController;
    ViewShellBase* mpViewShellBase;

    void Shutdown (void);

    void ThrowIfDisposed (void) const
        throw (css::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif
