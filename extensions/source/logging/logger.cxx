/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "logrecord.hxx"
#include "loggerconfig.hxx"

#include <com/sun/star/logging/XLogger.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/logging/XLoggerPool.hpp>

#include <cppuhelper/basemutex.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ref.hxx>
#include <map>


namespace logging
{

    using ::com::sun::star::logging::XLogger;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::WeakReference;
    using ::com::sun::star::logging::XLogHandler;
    using ::com::sun::star::logging::LogRecord;

    class EventLogger : public cppu::BaseMutex,
                        public cppu::WeakImplHelper<css::logging::XLogger>
    {
    private:
        comphelper::OInterfaceContainerHelper2     m_aHandlers;
        oslInterlockedCount                 m_nEventNumber;

        // <attributes>
        sal_Int32       m_nLogLevel;
        OUString m_sName;
        // </attributes>

    public:
        EventLogger( const Reference< XComponentContext >& _rxContext, const OUString& _rName );

        // XLogger
        virtual OUString SAL_CALL getName() throw (RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getLevel() throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL setLevel( ::sal_Int32 _level ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL addLogHandler( const Reference< XLogHandler >& LogHandler ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL removeLogHandler( const Reference< XLogHandler >& LogHandler ) throw (RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isLoggable( ::sal_Int32 _nLevel ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL log( ::sal_Int32 Level, const OUString& Message ) throw (RuntimeException, std::exception) override;
        virtual void SAL_CALL logp( ::sal_Int32 Level, const OUString& SourceClass, const OUString& SourceMethod, const OUString& Message ) throw (RuntimeException, std::exception) override;

    protected:
        virtual ~EventLogger();

    private:
        /** logs the given log record
        */
        void    impl_ts_logEvent_nothrow( const LogRecord& _rRecord );

        /** non-threadsafe impl-version of isLoggable
        */
        bool    impl_nts_isLoggable_nothrow( ::sal_Int32 _nLevel );
    };

    /** administrates a pool of XLogger instances, where a logger is keyed by its name,
        and subsequent requests for a logger with the same name return the same instance.
    */
    class LoggerPool : public cppu::WeakImplHelper<css::logging::XLoggerPool, XServiceInfo>
    {
    private:
        ::osl::Mutex                    m_aMutex;
        Reference<XComponentContext>    m_xContext;
        std::map< OUString, WeakReference<XLogger> > m_aLoggerMap;

    public:
        explicit LoggerPool( const Reference< XComponentContext >& _rxContext );

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) override;

        // XLoggerPool
        virtual Reference< XLogger > SAL_CALL getNamedLogger( const OUString& Name ) throw (RuntimeException, std::exception) override;
        virtual Reference< XLogger > SAL_CALL getDefaultLogger(  ) throw (RuntimeException, std::exception) override;
    };

    EventLogger::EventLogger( const Reference< XComponentContext >& _rxContext, const OUString& _rName )
        :m_aHandlers( m_aMutex )
        ,m_nEventNumber( 0 )
        ,m_nLogLevel( css::logging::LogLevel::OFF )
        ,m_sName( _rName )
    {
        osl_atomic_increment( &m_refCount );
        {
            initializeLoggerFromConfiguration( _rxContext, this );
        }
        osl_atomic_decrement( &m_refCount );
    }

    EventLogger::~EventLogger()
    {
    }

    bool EventLogger::impl_nts_isLoggable_nothrow( ::sal_Int32 _nLevel )
    {
        if ( _nLevel < m_nLogLevel )
            return false;

        if ( !m_aHandlers.getLength() )
            return false;

        return true;
    }

    void EventLogger::impl_ts_logEvent_nothrow( const LogRecord& _rRecord )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !impl_nts_isLoggable_nothrow( _rRecord.Level ) )
            return;

        m_aHandlers.forEach< XLogHandler >(
            [&_rRecord] (Reference<XLogHandler> const& rxListener) { rxListener->publish(_rRecord); } );
        m_aHandlers.forEach< XLogHandler >(
            [] (Reference<XLogHandler> const& rxListener) { rxListener->flush(); } );
    }

    OUString SAL_CALL EventLogger::getName() throw (RuntimeException, std::exception)
    {
        return m_sName;
    }

    ::sal_Int32 SAL_CALL EventLogger::getLevel() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_nLogLevel;
    }

    void SAL_CALL EventLogger::setLevel( ::sal_Int32 _level ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_nLogLevel = _level;
    }

    void SAL_CALL EventLogger::addLogHandler( const Reference< XLogHandler >& _rxLogHandler ) throw (RuntimeException, std::exception)
    {
        if ( _rxLogHandler.is() )
            m_aHandlers.addInterface( _rxLogHandler );
    }

    void SAL_CALL EventLogger::removeLogHandler( const Reference< XLogHandler >& _rxLogHandler ) throw (RuntimeException, std::exception)
    {
        if ( _rxLogHandler.is() )
            m_aHandlers.removeInterface( _rxLogHandler );
    }

    sal_Bool SAL_CALL EventLogger::isLoggable( ::sal_Int32 _nLevel ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return impl_nts_isLoggable_nothrow( _nLevel );
    }

    void SAL_CALL EventLogger::log( ::sal_Int32 _nLevel, const OUString& _rMessage ) throw (RuntimeException, std::exception)
    {
        impl_ts_logEvent_nothrow( createLogRecord(
            m_sName,
            _rMessage,
            _nLevel,
            osl_atomic_increment( &m_nEventNumber )
        ) );
    }

    void SAL_CALL EventLogger::logp( ::sal_Int32 _nLevel, const OUString& _rSourceClass, const OUString& _rSourceMethod, const OUString& _rMessage ) throw (RuntimeException, std::exception)
    {
        impl_ts_logEvent_nothrow( createLogRecord(
            m_sName,
            _rSourceClass,
            _rSourceMethod,
            _rMessage,
            _nLevel,
            osl_atomic_increment( &m_nEventNumber )
        ) );
    }

    LoggerPool::LoggerPool( const Reference< XComponentContext >& _rxContext )
        :m_xContext( _rxContext )
    {
    }

    OUString SAL_CALL LoggerPool::getImplementationName() throw(RuntimeException, std::exception)
    {
        return OUString("com.sun.star.comp.extensions.LoggerPool");
    }

    sal_Bool SAL_CALL LoggerPool::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
    {
        return cppu::supportsService(this, _rServiceName);
    }

    Sequence< OUString > SAL_CALL LoggerPool::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        return { "com.sun.star.logging.LoggerPool" };
    }

    Reference< XLogger > SAL_CALL LoggerPool::getNamedLogger( const OUString& _rName ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        WeakReference< XLogger >& rLogger( m_aLoggerMap[ _rName ] );
        Reference< XLogger > xLogger( rLogger );
        if ( !xLogger.is() )
        {
            // never requested before, or already dead
            xLogger = new EventLogger( m_xContext, _rName );
            rLogger = xLogger;
        }

        return xLogger;
    }

    Reference< XLogger > SAL_CALL LoggerPool::getDefaultLogger(  ) throw (RuntimeException, std::exception)
    {
        return getNamedLogger( "org.openoffice.logging.DefaultLogger" );
    }

} // namespace logging

namespace {

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(new logging::LoggerPool(context)))
    {}

    rtl::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_extensions_LoggerPool(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
