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



#ifndef SD_FRAMEWORK_PRESENTATION_FACTORY_HXX
#define SD_FRAMEWORK_PRESENTATION_FACTORY_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase2.hxx>


#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
    css::drawing::framework::XResourceFactory,
    css::drawing::framework::XConfigurationChangeListener
    > PresentationFactoryInterfaceBase;

} // end of anonymous namespace.



namespace sd { namespace framework {

/** This factory creates a marker view whose existence in a configuration
    indicates that a slideshow is running (in another but associated
    application window).
*/
class PresentationFactory
    : private sd::MutexOwner,
      public PresentationFactoryInterfaceBase
{
public:
    static const ::rtl::OUString msPresentationViewURL;

    PresentationFactory (
        const css::uno::Reference<css::frame::XController>& rxController);
    virtual ~PresentationFactory (void);

    virtual void SAL_CALL disposing (void);


    // XResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxViewId)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException);

    virtual void SAL_CALL releaseResource (
        const css::uno::Reference<css::drawing::framework::XResource>& xView)
        throw(css::uno::RuntimeException);


    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException);


    // lang::XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEventObject)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;
    css::uno::Reference<css::frame::XController> mxController;

    void ThrowIfDisposed (void) const
        throw (css::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif
