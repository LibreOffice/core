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


#include <comphelper/logging.hxx>

#include <com/sun/star/logging/LoggerPool.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/resource/OfficeResourceLoader.hpp>
#include <com/sun/star/resource/XResourceBundle.hpp>
#include <com/sun/star/resource/XResourceBundleLoader.hpp>

#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>


namespace comphelper
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::logging::XLoggerPool;
    using ::com::sun::star::logging::LoggerPool;
    using ::com::sun::star::logging::XLogger;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::logging::XLogHandler;
    using ::com::sun::star::resource::XResourceBundle;
    using ::com::sun::star::resource::XResourceBundleLoader;

    namespace LogLevel = ::com::sun::star::logging::LogLevel;

    class EventLogger_Impl
    {
    private:
        Reference< XComponentContext >  m_aContext;
        OUString                 m_sLoggerName;
        Reference< XLogger >            m_xLogger;

    public:
        EventLogger_Impl( const Reference< XComponentContext >& _rxContext, const OUString& _rLoggerName )
            :m_aContext( _rxContext )
            ,m_sLoggerName( _rLoggerName )
        {
            impl_createLogger_nothrow();
        }

        inline bool isValid() const { return m_xLogger.is(); }
        inline const Reference< XLogger >& getLogger() const { return m_xLogger; }
        inline Reference< XComponentContext > getContext() const { return m_aContext; }

    private:
        void    impl_createLogger_nothrow();
    };

    void EventLogger_Impl::impl_createLogger_nothrow()
    {
        try
        {
            Reference< XLoggerPool > xPool( LoggerPool::get( m_aContext ) );
            if ( !m_sLoggerName.isEmpty() )
                m_xLogger = xPool->getNamedLogger( m_sLoggerName );
            else
                m_xLogger = xPool->getDefaultLogger();
        }
        catch( const Exception& e )
        {
            (void)e;
            OSL_FAIL( "EventLogger_Impl::impl_createLogger_nothrow: caught an exception!" );
        }
    }

    EventLogger::EventLogger( const Reference< XComponentContext >& _rxContext, const sal_Char* _pAsciiLoggerName )
        :m_pImpl( new EventLogger_Impl( _rxContext, OUString::createFromAscii( _pAsciiLoggerName ) ) )
    {
    }


    EventLogger::~EventLogger()
    {
    }


    bool EventLogger::isLoggable( const sal_Int32 _nLogLevel ) const
    {
        if ( !m_pImpl->isValid() )
            return false;

        try
        {
            return m_pImpl->getLogger()->isLoggable( _nLogLevel );
        }
        catch( const Exception& e )
        {
            (void)e;
            OSL_FAIL( "EventLogger::isLoggable: caught an exception!" );
        }

        return false;
    }


    namespace
    {
        void lcl_replaceParameter( OUString& _inout_Message, const char* _rPlaceHolder, const OUString& _rReplacement )
        {
            sal_Int32 nPlaceholderPosition = _inout_Message.indexOfAsciiL( _rPlaceHolder, strlen(_rPlaceHolder) );
            OSL_ENSURE( nPlaceholderPosition >= 0, "lcl_replaceParameter: placeholder not found!" );
            if ( nPlaceholderPosition < 0 )
                return;

            _inout_Message = _inout_Message.replaceAt( nPlaceholderPosition, strlen(_rPlaceHolder), _rReplacement );
        }
    }


    bool EventLogger::impl_log( const sal_Int32 _nLogLevel,
        const sal_Char* _pSourceClass, const sal_Char* _pSourceMethod, const OUString& _rMessage,
        const OptionalString& _rArgument1, const OptionalString& _rArgument2,
        const OptionalString& _rArgument3, const OptionalString& _rArgument4,
        const OptionalString& _rArgument5, const OptionalString& _rArgument6 ) const
    {
        OUString sMessage( _rMessage );
        if ( !!_rArgument1 )
            lcl_replaceParameter( sMessage, "$1$", *_rArgument1 );

        if ( !!_rArgument2 )
            lcl_replaceParameter( sMessage, "$2$", *_rArgument2 );

        if ( !!_rArgument3 )
            lcl_replaceParameter( sMessage, "$3$", *_rArgument3 );

        if ( !!_rArgument4 )
            lcl_replaceParameter( sMessage, "$4$", *_rArgument4 );

        if ( !!_rArgument5 )
            lcl_replaceParameter( sMessage, "$5$", *_rArgument5 );

        if ( !!_rArgument6 )
            lcl_replaceParameter( sMessage, "$6$", *_rArgument6 );

        try
        {
            Reference< XLogger > xLogger( m_pImpl->getLogger() );
            OSL_PRECOND( xLogger.is(), "EventLogger::impl_log: should never be called without a logger!" );
            if ( _pSourceClass && _pSourceMethod )
            {
                xLogger->logp(
                    _nLogLevel,
                    OUString::createFromAscii( _pSourceClass ),
                    OUString::createFromAscii( _pSourceMethod ),
                    sMessage
                );
            }
            else
            {
                xLogger->log( _nLogLevel, sMessage );
            }
        }
        catch( const Exception& e )
        {
            (void)e;
            OSL_FAIL( "EventLogger::impl_log: caught an exception!" );
        }

        return false;
    }

    struct ResourceBasedEventLogger_Data
    {
        /// the base name of the resource bundle
        OUString                 sBundleBaseName;
        /// did we already attempt to load the bundle?
        bool                            bBundleLoaded;
        /// the lazily loaded bundle
        Reference< XResourceBundle >    xBundle;

        ResourceBasedEventLogger_Data()
            :sBundleBaseName()
            ,bBundleLoaded( false )
            ,xBundle()
        {
        }
    };


    bool    lcl_loadBundle_nothrow( Reference< XComponentContext > const & _rContext, ResourceBasedEventLogger_Data& _rLoggerData )
    {
        if ( _rLoggerData.bBundleLoaded )
            return _rLoggerData.xBundle.is();

        // no matter what happens below, don't attempt creation ever again
        _rLoggerData.bBundleLoaded = true;

        try
        {
            Reference< XResourceBundleLoader > xLoader(
                css::resource::OfficeResourceLoader::get(
                    _rContext ) );
            _rLoggerData.xBundle.set( xLoader->loadBundle_Default( _rLoggerData.sBundleBaseName ), UNO_QUERY_THROW );
        }
        catch( const Exception& e )
        {
            (void)e;
            OSL_FAIL( "lcl_loadBundle_nothrow: caught an exception!" );
        }

        return _rLoggerData.xBundle.is();
    }


    OUString lcl_loadString_nothrow( const Reference< XResourceBundle >& _rxBundle, const sal_Int32 _nMessageResID )
    {
        OSL_PRECOND( _rxBundle.is(), "lcl_loadString_nothrow: this will crash!" );
        OUString sMessage;
        try
        {
            OUStringBuffer aBuffer;
            aBuffer.append( "string:" );
            aBuffer.append( _nMessageResID );
            OSL_VERIFY( _rxBundle->getDirectElement( aBuffer.makeStringAndClear() ) >>= sMessage );
        }
        catch( const Exception& e )
        {
            (void)e;
            OSL_FAIL( "lcl_loadString_nothrow: caught an exception!" );
        }
        return sMessage;
    }

    ResourceBasedEventLogger::ResourceBasedEventLogger( const Reference< XComponentContext >& _rxContext, const sal_Char* _pResourceBundleBaseName,
        const sal_Char* _pAsciiLoggerName )
        :EventLogger( _rxContext, _pAsciiLoggerName )
        ,m_pData( new ResourceBasedEventLogger_Data )
    {
        m_pData->sBundleBaseName = OUString::createFromAscii( _pResourceBundleBaseName );
    }


    OUString ResourceBasedEventLogger::impl_loadStringMessage_nothrow( const sal_Int32 _nMessageResID ) const
    {
        OUString sMessage;
        if ( lcl_loadBundle_nothrow( m_pImpl->getContext(), *m_pData ) )
            sMessage = lcl_loadString_nothrow( m_pData->xBundle, _nMessageResID );
        if ( sMessage.isEmpty() )
        {
            OUStringBuffer aBuffer;
            aBuffer.append( "<invalid event resource: '" );
            aBuffer.append( m_pData->sBundleBaseName );
            aBuffer.append( ":" );
            aBuffer.append( _nMessageResID );
            aBuffer.append( "'>" );
            sMessage = aBuffer.makeStringAndClear();
        }
        return sMessage;
    }


} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
