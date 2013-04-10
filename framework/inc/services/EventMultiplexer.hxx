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



#ifndef __FRAMEWORK_SERVICES_EVENT_MULTIPLEXER_HXX_
#define __FRAMEWORK_SERVICES_EVENT_MULTIPLEXER_HXX_

#include <com/sun/star/util/XEventMultiplexer.hpp>

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/basemutex.hxx>

#include "macros/xserviceinfo.hxx"

namespace
{
    typedef ::cppu::WeakComponentImplHelper3 <
        css::util::XEventMultiplexer,
        css::lang::XSingleComponentFactory,
        css::lang::XServiceInfo
        > EventMultiplexerInterfaceBase;
}


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;
namespace cssl = ::com::sun::star::lang;

namespace framework {

class EventMultiplexer
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public EventMultiplexerInterfaceBase
{
public:
    EventMultiplexer(const cssu::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~EventMultiplexer (void);

    // XEventMultiplexer
    virtual void SAL_CALL addEventListener (
        const cssu::Reference<css::util::XEventListener>& rxListener,
        const cssu::Reference<cssu::XInterface>& rxEventFocus)
        throw(cssu::RuntimeException, cssl::IllegalArgumentException);
    virtual void SAL_CALL removeEventListener (
        const cssu::Reference<css::util::XEventListener>& rxListener,
        const cssu::Reference<cssu::XInterface>& rxEventFocus)
        throw(cssu::RuntimeException, cssl::IllegalArgumentException);
    virtual void SAL_CALL removeAllEventListeners (
        const cssu::Reference<css::util::XEventListener>& rxListener)
        throw(cssu::RuntimeException, cssl::IllegalArgumentException);
    virtual void SAL_CALL broadcastEvent (
        const cssl::EventObject& rEventObject,
        const cssu::Reference<cssu::XInterface>& rxEventFocus)
        throw(cssu::RuntimeException);

    // XSingleComponentFactory
    virtual cssu::Reference<cssu::XInterface> SAL_CALL createInstanceWithContext (
        const cssu::Reference<cssu::XComponentContext>& rxContext)
        throw (cssu::Exception, cssu::RuntimeException);
    virtual cssu::Reference<cssu::XInterface > SAL_CALL createInstanceWithArgumentsAndContext (
        const cssu::Sequence<cssu::Any>& rArguments,
        const cssu::Reference<cssu::XComponentContext>& rxContext)
        throw (cssu::Exception, cssu::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (cssu::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService  (
        const ::rtl::OUString& rsServiceName)
        throw (cssu::RuntimeException);
    virtual cssu::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames (void)
        throw (cssu::RuntimeException);

    static ::rtl::OUString SAL_CALL impl_getStaticImplementationName (void);
    static cssu::Reference<cssu::XInterface> SAL_CALL impl_createFactory (
        const cssu::Reference<cssl::XMultiServiceFactory>& xServiceManager);

private:
    typedef ::std::vector<cssu::Reference<css::util::XEventListener> > ListenerContainer;
    typedef ::std::map<cssu::Reference<cssu::XInterface>, ListenerContainer> ListenerMap;
    ListenerMap maListeners;

    /** Notify all listeners in the container that is associated with
        the given event focus.

        Typically called twice from broadcastEvent(), once for the
        given event focus and onece for NULL.
    */
    void BroadcastEventToSingleContainer (
        const cssl::EventObject& rEventObject,
        const cssu::Reference<cssu::XInterface>& rxEventFocus);

    static cssu::Sequence< ::rtl::OUString > SAL_CALL static_GetSupportedServiceNames (void);
    static cssu::Reference<cssu::XInterface> SAL_CALL static_CreateInstance (
        const cssu::Reference<cssu::XComponentContext>& rxComponentContext)
        throw (cssu::Exception);
};

} // end of namespace framework

#endif

