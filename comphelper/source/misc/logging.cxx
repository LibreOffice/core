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

    class EventLogger_Impl
    {
    private:
        Reference< XComponentContext >  m_aContext;
        Reference< XLogger >            m_xLogger;

    public:
        EventLogger_Impl( const Reference< XComponentContext >& _rxContext, const OUString& _rLoggerName );

        bool isValid() const { return m_xLogger.is(); }
        const Reference< XLogger >& getLogger() const { return m_xLogger; }
    };

    EventLogger_Impl::EventLogger_Impl( const Reference< XComponentContext >& _rxContext, const OUString& _rLoggerName )
        :m_aContext( _rxContext )
    {
        try
        {
            Reference< XLoggerPool > xPool( LoggerPool::get( m_aContext ) );
            if ( !_rLoggerName.isEmpty() )
                m_xLogger = xPool->getNamedLogger( _rLoggerName );
            else
                m_xLogger = xPool->getDefaultLogger();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "EventLogger_Impl::impl_createLogger_nothrow: caught an exception!" );
        }
    }

    EventLogger::EventLogger( const Reference< XComponentContext >& _rxContext, const sal_Char* _pAsciiLoggerName )
        :m_pImpl( new EventLogger_Impl( _rxContext, OUString::createFromAscii( _pAsciiLoggerName ) ) )
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
        catch( const Exception& )
        {
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
        catch( const Exception& )
        {
            OSL_FAIL( "EventLogger::impl_log: caught an exception!" );
        }

        return false;
    }
} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
