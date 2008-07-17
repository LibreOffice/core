/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: dispatchlogging.cxx,v $
*
* $Revision: 1.2 $
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/

#include <dispatch/dispatchlogging.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <comphelper/uieventslogger.hxx>
#include <macros/debug/assertion.hxx>
#include <macros/xinterface.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

namespace {
    /**
    * @short a decorator around a XDispatch object that logs when dispatch() gets called
    * @see http://en.wikipedia.org/wiki/Decorator_pattern
    */
    class LoggingDispatcher : //public XDispatch,
        public XSynchronousDispatch,
        public XNotifyingDispatch,
        public ::cppu::OWeakObject
    {
        public:
            LoggingDispatcher(const Reference<XDispatch> slave);
            FWK_DECLARE_XINTERFACE
            virtual void SAL_CALL dispatch(
                const util::URL& url,
                const Sequence<PropertyValue>& args)
                    throw(RuntimeException);
            virtual void SAL_CALL addStatusListener(
                const Reference<XStatusListener>& control,
                const util::URL& url)
                    throw(RuntimeException);
            virtual void SAL_CALL removeStatusListener(
                const Reference<XStatusListener>& control,
                const util::URL& url)
                    throw(RuntimeException);
            virtual void SAL_CALL dispatchWithNotification(
                const util::URL& url,
                const Sequence<PropertyValue>& args,
                const Reference<XDispatchResultListener>& listener)
                    throw(RuntimeException);
            virtual Any SAL_CALL dispatchWithReturnValue(
                const util::URL& url,
                const Sequence<PropertyValue>& args)
                    throw(RuntimeException);
        private:
           Reference<XDispatch> m_Slave;
    };

    /**
    * @short a decorator around a XDispatchProvider object that logs when queried and
    * returns XDispatch objects decorated with LoggingDispatcher
    * (we are missing an explicit Decorator class, but the idea is the same)
    * @see http://en.wikipedia.org/wiki/Decorator_pattern
    */
    class DispatchLogging_Impl : public ::framework::DispatchLogging
    {
        public:
            DispatchLogging_Impl(const Reference<XDispatchProvider>& slave);
            FWK_DECLARE_XINTERFACE
            virtual Reference<XDispatch> SAL_CALL queryDispatch(
                const util::URL& url,
                const ::rtl::OUString& target_frame_name,
                sal_Int32 search_flags)
                    throw(RuntimeException);
            virtual Sequence<Reference<XDispatch> > SAL_CALL queryDispatches(
                const Sequence<DispatchDescriptor>& descriptor)
                    throw(RuntimeException);
            virtual void SAL_CALL registerDispatchProviderInterceptor(
                const Reference<XDispatchProviderInterceptor>& xInterceptor)
                    throw(RuntimeException);
            virtual void SAL_CALL releaseDispatchProviderInterceptor(
                const Reference<XDispatchProviderInterceptor>& xInterceptor)
                    throw(RuntimeException);
            virtual void SAL_CALL disposing(
                const lang::EventObject& event)
                    throw(RuntimeException);

        private:
            const Reference<XDispatchProvider> m_Slave;
            void checkSlave();
    };

    // LoggingDispatcher implementation
    LoggingDispatcher::LoggingDispatcher(
        const Reference<XDispatch> slave)
        : m_Slave(slave)
    {}

    DEFINE_XINTERFACE_3(LoggingDispatcher,
        ::cppu::OWeakObject,
        DERIVED_INTERFACE(XDispatch, XNotifyingDispatch),
        DIRECT_INTERFACE(XSynchronousDispatch),
        DIRECT_INTERFACE(XNotifyingDispatch))

    void LoggingDispatcher::dispatch(
        const util::URL& url,
        const Sequence<PropertyValue>& args)
            throw(RuntimeException)
    {
        ::comphelper::UiEventsLogger::logDispatch(url, args);
        m_Slave->dispatch(url, ::comphelper::UiEventsLogger::purgeDispatchOrigin(args));
    }

    void SAL_CALL LoggingDispatcher::dispatchWithNotification(
        const util::URL& url,
        const Sequence<PropertyValue>& args,
        const Reference<XDispatchResultListener>& listener)
            throw(RuntimeException)
    {
        ::comphelper::UiEventsLogger::logDispatch(url, args);
        Reference<XNotifyingDispatch> ref(m_Slave, UNO_QUERY);
        if(ref.is())
            ref->dispatchWithNotification(url, args, listener);
        else
            if(listener.is())
            {
                DispatchResultEvent event;
                event.State = DispatchResultState::FAILURE;
                 listener->dispatchFinished(event);
            }
    }

    Any SAL_CALL LoggingDispatcher::dispatchWithReturnValue(
        const util::URL& url,
        const Sequence<PropertyValue>& args)
            throw(RuntimeException)
    {
        ::comphelper::UiEventsLogger::logDispatch(url, args);
        Reference<XSynchronousDispatch> ref(m_Slave, UNO_QUERY);
        if(ref.is())
            return ref->dispatchWithReturnValue(url, args);
        else
            return Any();
    }

    void LoggingDispatcher::addStatusListener(
        const Reference<XStatusListener>& control,
        const util::URL& url)
            throw(RuntimeException)
    { m_Slave->addStatusListener(control, url); }

    void LoggingDispatcher::removeStatusListener(
        const Reference<XStatusListener>& control,
        const util::URL& url)
            throw(RuntimeException)
    { m_Slave->removeStatusListener(control, url); }


    // DispatchLogging implementation
    DispatchLogging_Impl::DispatchLogging_Impl(const Reference<XDispatchProvider>& slave)
        : m_Slave(slave)
    { checkSlave(); }

    DEFINE_XINTERFACE_3(DispatchLogging_Impl,
        ::cppu::OWeakObject,
        DIRECT_INTERFACE(XDispatchProvider),
        DIRECT_INTERFACE(XDispatchProviderInterception),
        DIRECT_INTERFACE(lang::XEventListener))

    Reference<XDispatch> DispatchLogging_Impl::queryDispatch(
        const util::URL& url,
        const ::rtl::OUString& target_frame_name,
        sal_Int32 search_flags)
            throw(RuntimeException)
    {
        Reference<XDispatch> slave_result = m_Slave->queryDispatch(url, target_frame_name, search_flags);
        if(slave_result.is())
            return Reference<XDispatch>(new LoggingDispatcher(slave_result));
        return slave_result;
    }

    Sequence<Reference<XDispatch> > DispatchLogging_Impl::queryDispatches(
        const Sequence<DispatchDescriptor>& descriptors)
            throw(RuntimeException)
    { return m_Slave->queryDispatches(descriptors); }

    void DispatchLogging_Impl::registerDispatchProviderInterceptor(
        const Reference<XDispatchProviderInterceptor>& interceptor)
            throw(RuntimeException)
    {
        Reference<XDispatchProviderInterception> slave_as_inter(m_Slave, UNO_QUERY);
        slave_as_inter->registerDispatchProviderInterceptor(interceptor);
    }

    void DispatchLogging_Impl::releaseDispatchProviderInterceptor(
        const Reference<XDispatchProviderInterceptor>& interceptor)
            throw(RuntimeException)
    {
        Reference<XDispatchProviderInterception> slave_as_inter(m_Slave, UNO_QUERY);
        slave_as_inter->releaseDispatchProviderInterceptor(interceptor); }

    void DispatchLogging_Impl::disposing(
        const lang::EventObject& event)
            throw(RuntimeException)
    {
        Reference<XEventListener> slave_as_el(m_Slave, UNO_QUERY);
        slave_as_el->disposing(event);
    }

    // a cleaner way would be a new interface for the slave inheriting from
    // XDispatchProvider, XEventListener and XDispatchProviderInterceptor ...
    void DispatchLogging_Impl::checkSlave()
    {
        Reference<XDispatchProviderInterception> slave_as_inter(m_Slave, UNO_QUERY);
        Reference<XEventListener> slave_as_el(m_Slave, UNO_QUERY);
        LOG_ASSERT2(!slave_as_inter.is(),
            "DispatchLogging", "Slave does not support XDispatchProviderInterception, but should.");
        LOG_ASSERT2(!slave_as_el.is(),
            "DispatchLogging", "Slave does not support XEventListener, but should.");
    }
}

namespace framework {
    Reference<XDispatchProvider> DispatchLogging::createInstance(const Reference<XDispatchProvider>& slave)
    {
        return Reference<XDispatchProvider>(new DispatchLogging_Impl(slave));
    }
}
