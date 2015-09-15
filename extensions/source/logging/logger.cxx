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


#include "log_module.hxx"
#include "log_services.hxx"
#include "logrecord.hxx"
#include "loggerconfig.hxx"

#include <com/sun/star/logging/XLogger.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/logging/XLoggerPool.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weakref.hxx>
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
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::WeakReference;
    using ::com::sun::star::logging::XLogHandler;
    using ::com::sun::star::logging::XLoggerPool;
    using ::com::sun::star::logging::LogRecord;

    namespace LogLevel = ::com::sun::star::logging::LogLevel;

    typedef ::cppu::WeakImplHelper <   XLogger
                                    ,   XServiceInfo
                                    >   EventLogger_Base;
    class EventLogger   :public ::cppu::BaseMutex
                        ,public EventLogger_Base
    {
    private:
        ::cppu::OInterfaceContainerHelper   m_aHandlers;
        oslInterlockedCount                 m_nEventNumber;

        // <attributes>
        sal_Int32       m_nLogLevel;
        OUString m_sName;
        // </attributes>

    public:
        EventLogger( const Reference< XComponentContext >& _rxContext, const OUString& _rName );

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) SAL_OVERRIDE;

        // XLogger
        virtual OUString SAL_CALL getName() throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::sal_Int32 SAL_CALL getLevel() throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setLevel( ::sal_Int32 _level ) throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL addLogHandler( const Reference< XLogHandler >& LogHandler ) throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeLogHandler( const Reference< XLogHandler >& LogHandler ) throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL isLoggable( ::sal_Int32 _nLevel ) throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL log( ::sal_Int32 Level, const OUString& Message ) throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL logp( ::sal_Int32 Level, const OUString& SourceClass, const OUString& SourceMethod, const OUString& Message ) throw (RuntimeException, std::exception) SAL_OVERRIDE;

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

    typedef ::cppu::WeakImplHelper <   XLoggerPool
                                    ,   XServiceInfo
                                    >   LoggerPool_Base;
    /** administrates a pool of XLogger instances, where a logger is keyed by its name,
        and subsequent requests for a logger with the same name return the same instance.
    */
    class LoggerPool : public LoggerPool_Base
    {
    private:
        typedef ::std::map< OUString, WeakReference< XLogger > > ImplPool;

    private:
        ::osl::Mutex                    m_aMutex;
        Reference<XComponentContext>    m_xContext;
        ImplPool                        m_aImpl;

    public:
        explicit LoggerPool( const Reference< XComponentContext >& _rxContext );

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) SAL_OVERRIDE;

        // helper for factories
        static Sequence< OUString > getSupportedServiceNames_static();
        static OUString  getImplementationName_static();
        static OUString  getSingletonName_static();
        static Reference< XInterface > Create( const Reference< XComponentContext >& _rxContext );

        // XLoggerPool
        virtual Reference< XLogger > SAL_CALL getNamedLogger( const OUString& Name ) throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual Reference< XLogger > SAL_CALL getDefaultLogger(  ) throw (RuntimeException, std::exception) SAL_OVERRIDE;
    };

    EventLogger::EventLogger( const Reference< XComponentContext >& _rxContext, const OUString& _rName )
        :m_aHandlers( m_aMutex )
        ,m_nEventNumber( 0 )
        ,m_nLogLevel( LogLevel::OFF )
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

    OUString SAL_CALL EventLogger::getImplementationName() throw(RuntimeException, std::exception)
    {
        return OUString( "com.sun.star.comp.extensions.EventLogger" );
    }

    sal_Bool EventLogger::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
    {
        return cppu::supportsService(this, _rServiceName);
    }

    Sequence< OUString > SAL_CALL EventLogger::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        Sequence< OUString > aServiceNames(1);
        aServiceNames[0] = "com.sun.star.logging.Logger";
        return aServiceNames;
    }

    LoggerPool::LoggerPool( const Reference< XComponentContext >& _rxContext )
        :m_xContext( _rxContext )
    {
    }

    OUString SAL_CALL LoggerPool::getImplementationName() throw(RuntimeException, std::exception)
    {
        return getImplementationName_static();
    }

    sal_Bool SAL_CALL LoggerPool::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
    {
        return cppu::supportsService(this, _rServiceName);
    }

    Sequence< OUString > SAL_CALL LoggerPool::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        return getSupportedServiceNames_static();
    }

    OUString SAL_CALL LoggerPool::getImplementationName_static()
    {
        return OUString( "com.sun.star.comp.extensions.LoggerPool" );
    }

    Sequence< OUString > SAL_CALL LoggerPool::getSupportedServiceNames_static()
    {
        Sequence< OUString > aServiceNames(1);
        aServiceNames[0] = getSingletonName_static();
        return aServiceNames;
    }

    OUString LoggerPool::getSingletonName_static()
    {
        return OUString( "com.sun.star.logging.LoggerPool" );
    }

    Reference< XInterface > SAL_CALL LoggerPool::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new LoggerPool( _rxContext ) );
    }

    Reference< XLogger > SAL_CALL LoggerPool::getNamedLogger( const OUString& _rName ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        WeakReference< XLogger >& rLogger( m_aImpl[ _rName ] );
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
        return getNamedLogger( OUString( "org.openoffice.logging.DefaultLogger" ) );
    }

    void createRegistryInfo_LoggerPool()
    {
        static OSingletonRegistration< LoggerPool > aAutoRegistration;
    }

} // namespace logging

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
