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
#include "logrecord.hxx"
#include "loggerconfig.hxx"

#include <com/sun/star/logging/XLogger.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/logging/XLoggerPool.hpp>

#include <tools/diagnose_ex.h>

#include <comphelper/componentcontext.hxx>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/weakref.hxx>

#include <boost/bind.hpp>

#include <map>

//........................................................................
namespace logging
{
//........................................................................

    /** === begin UNO using === **/
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
    /** === end UNO using === **/
    namespace LogLevel = ::com::sun::star::logging::LogLevel;

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        sal_Bool lcl_supportsService_nothrow( XServiceInfo& _rSI, const ::rtl::OUString& _rServiceName )
        {
            const Sequence< ::rtl::OUString > aServiceNames( _rSI.getSupportedServiceNames() );
            for (   const ::rtl::OUString* pServiceNames = aServiceNames.getConstArray();
                    pServiceNames != aServiceNames.getConstArray() + aServiceNames.getLength();
                    ++pServiceNames
                )
                if ( _rServiceName == *pServiceNames )
                    return sal_True;
            return sal_False;
        }
    }

    //====================================================================
    //= EventLogger - declaration
    //====================================================================
    typedef ::cppu::WeakImplHelper2 <   XLogger
                                    ,   XServiceInfo
                                    >   EventLogger_Base;
    class EventLogger   :public ::cppu::BaseMutex
                        ,public EventLogger_Base
    {
    private:
        ::comphelper::ComponentContext      m_aContext;
        ::cppu::OInterfaceContainerHelper   m_aHandlers;
        oslInterlockedCount                 m_nEventNumber;

        // <attributes>
        sal_Int32       m_nLogLevel;
        ::rtl::OUString m_sName;
        // </attributes>

    public:
        EventLogger( const Reference< XComponentContext >& _rxContext, const ::rtl::OUString& _rName );

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

        // XLogger
        virtual ::rtl::OUString SAL_CALL getName() throw (RuntimeException);
        virtual ::sal_Int32 SAL_CALL getLevel() throw (RuntimeException);
        virtual void SAL_CALL setLevel( ::sal_Int32 _level ) throw (RuntimeException);
        virtual void SAL_CALL addLogHandler( const Reference< XLogHandler >& LogHandler ) throw (RuntimeException);
        virtual void SAL_CALL removeLogHandler( const Reference< XLogHandler >& LogHandler ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL isLoggable( ::sal_Int32 _nLevel ) throw (RuntimeException);
        virtual void SAL_CALL log( ::sal_Int32 Level, const ::rtl::OUString& Message ) throw (RuntimeException);
        virtual void SAL_CALL logp( ::sal_Int32 Level, const ::rtl::OUString& SourceClass, const ::rtl::OUString& SourceMethod, const ::rtl::OUString& Message ) throw (RuntimeException);

    protected:
        ~EventLogger();

    private:
        /** logs the given log record
        */
        void    impl_ts_logEvent_nothrow( const LogRecord& _rRecord );

        /** non-threadsafe impl-version of isLoggable
        */
        bool    impl_nts_isLoggable_nothrow( ::sal_Int32 _nLevel );
    };

    //====================================================================
    //= LoggerPool - declaration
    //====================================================================
    typedef ::cppu::WeakImplHelper2 <   XLoggerPool
                                    ,   XServiceInfo
                                    >   LoggerPool_Base;
    /** administrates a pool of XLogger instances, where a logger is keyed by its name,
        and subsequent requests for a logger with the same name return the same instance.
    */
    class LoggerPool : public LoggerPool_Base
    {
    private:
        typedef ::std::map< ::rtl::OUString, WeakReference< XLogger > > ImplPool;

    private:
        ::osl::Mutex                    m_aMutex;
        ::comphelper::ComponentContext  m_aContext;
        ImplPool                        m_aImpl;

    public:
        LoggerPool( const Reference< XComponentContext >& _rxContext );

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

        // helper for factories
        static Sequence< ::rtl::OUString > getSupportedServiceNames_static();
        static ::rtl::OUString  getImplementationName_static();
        static ::rtl::OUString  getSingletonName_static();
        static Reference< XInterface > Create( const Reference< XComponentContext >& _rxContext );

        // XLoggerPool
        virtual Reference< XLogger > SAL_CALL getNamedLogger( const ::rtl::OUString& Name ) throw (RuntimeException);
        virtual Reference< XLogger > SAL_CALL getDefaultLogger(  ) throw (RuntimeException);
    };

    //====================================================================
    //= EventLogger - implementation
    //====================================================================
    //--------------------------------------------------------------------
    EventLogger::EventLogger( const Reference< XComponentContext >& _rxContext, const ::rtl::OUString& _rName )
        :m_aContext( _rxContext )
        ,m_aHandlers( m_aMutex )
        ,m_nEventNumber( 0 )
        ,m_nLogLevel( LogLevel::OFF )
        ,m_sName( _rName )
    {
        osl_atomic_increment( &m_refCount );
        {
            initializeLoggerFromConfiguration( m_aContext, this );
        }
        osl_atomic_decrement( &m_refCount );
    }

    //--------------------------------------------------------------------
    EventLogger::~EventLogger()
    {
    }

    //--------------------------------------------------------------------
    bool EventLogger::impl_nts_isLoggable_nothrow( ::sal_Int32 _nLevel )
    {
        if ( _nLevel < m_nLogLevel )
            return false;

        if ( !m_aHandlers.getLength() )
            return false;

        return true;
    }

    //--------------------------------------------------------------------
    void EventLogger::impl_ts_logEvent_nothrow( const LogRecord& _rRecord )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !impl_nts_isLoggable_nothrow( _rRecord.Level ) )
            return;

        m_aHandlers.forEach< XLogHandler >(
            ::boost::bind( &XLogHandler::publish, _1, ::boost::cref( _rRecord ) ) );
        m_aHandlers.forEach< XLogHandler >(
            ::boost::bind( &XLogHandler::flush, _1 ) );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL EventLogger::getName() throw (RuntimeException)
    {
        return m_sName;
    }

    //--------------------------------------------------------------------
    ::sal_Int32 SAL_CALL EventLogger::getLevel() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_nLogLevel;
    }

    //--------------------------------------------------------------------
    void SAL_CALL EventLogger::setLevel( ::sal_Int32 _level ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_nLogLevel = _level;
    }

    //--------------------------------------------------------------------
    void SAL_CALL EventLogger::addLogHandler( const Reference< XLogHandler >& _rxLogHandler ) throw (RuntimeException)
    {
        if ( _rxLogHandler.is() )
            m_aHandlers.addInterface( _rxLogHandler );
    }

    //--------------------------------------------------------------------
    void SAL_CALL EventLogger::removeLogHandler( const Reference< XLogHandler >& _rxLogHandler ) throw (RuntimeException)
    {
        if ( _rxLogHandler.is() )
            m_aHandlers.removeInterface( _rxLogHandler );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL EventLogger::isLoggable( ::sal_Int32 _nLevel ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return impl_nts_isLoggable_nothrow( _nLevel );
    }

    //--------------------------------------------------------------------
    void SAL_CALL EventLogger::log( ::sal_Int32 _nLevel, const ::rtl::OUString& _rMessage ) throw (RuntimeException)
    {
        impl_ts_logEvent_nothrow( createLogRecord(
            m_sName,
            _rMessage,
            _nLevel,
            osl_atomic_increment( &m_nEventNumber )
        ) );
    }

    //--------------------------------------------------------------------
    void SAL_CALL EventLogger::logp( ::sal_Int32 _nLevel, const ::rtl::OUString& _rSourceClass, const ::rtl::OUString& _rSourceMethod, const ::rtl::OUString& _rMessage ) throw (RuntimeException)
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

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL EventLogger::getImplementationName() throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.extensions.EventLogger" ) );
    }

    //--------------------------------------------------------------------
    ::sal_Bool EventLogger::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
    {
        return lcl_supportsService_nothrow( *this, _rServiceName );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL EventLogger::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServiceNames(1);
        aServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.logging.Logger" ) );
        return aServiceNames;
    }

    //====================================================================
    //= LoggerPool - implementation
    //====================================================================
    //--------------------------------------------------------------------
    LoggerPool::LoggerPool( const Reference< XComponentContext >& _rxContext )
        :m_aContext( _rxContext )
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL LoggerPool::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL LoggerPool::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
    {
        return lcl_supportsService_nothrow( *this, _rServiceName );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL LoggerPool::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL LoggerPool::getImplementationName_static()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.extensions.LoggerPool" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL LoggerPool::getSupportedServiceNames_static()
    {
        Sequence< ::rtl::OUString > aServiceNames(1);
        aServiceNames[0] = getSingletonName_static();
        return aServiceNames;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString LoggerPool::getSingletonName_static()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.logging.LoggerPool" ) );
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL LoggerPool::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new LoggerPool( _rxContext ) );
    }

    //--------------------------------------------------------------------
    Reference< XLogger > SAL_CALL LoggerPool::getNamedLogger( const ::rtl::OUString& _rName ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        WeakReference< XLogger >& rLogger( m_aImpl[ _rName ] );
        Reference< XLogger > xLogger( (Reference< XLogger >)rLogger );
        if ( !xLogger.is() )
        {
            // never requested before, or already dead
            xLogger = new EventLogger( m_aContext.getUNOContext(), _rName );
            rLogger = xLogger;
        }

        return xLogger;
    }

    //--------------------------------------------------------------------
    Reference< XLogger > SAL_CALL LoggerPool::getDefaultLogger(  ) throw (RuntimeException)
    {
        return getNamedLogger( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.logging.DefaultLogger" ) ) );
    }

    //--------------------------------------------------------------------
    void createRegistryInfo_LoggerPool()
    {
        static OSingletonRegistration< LoggerPool > aAutoRegistration;
    }

//........................................................................
} // namespace logging
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
