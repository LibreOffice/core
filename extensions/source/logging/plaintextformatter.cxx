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

#include <stdio.h>

#include <com/sun/star/logging/XLogFormatter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <rtl/ustrbuf.hxx>

#include <osl/thread.h>


namespace logging
{


    using ::com::sun::star::logging::XLogFormatter;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::logging::LogRecord;
    using ::com::sun::star::uno::XInterface;


    //= PlainTextFormatter - declaration

    typedef ::cppu::WeakImplHelper2 <   XLogFormatter
                                    ,   XServiceInfo
                                    >   PlainTextFormatter_Base;
    class PlainTextFormatter : public PlainTextFormatter_Base
    {
    protected:
        PlainTextFormatter();
        virtual ~PlainTextFormatter();

        // XLogFormatter
        virtual OUString SAL_CALL getHead(  ) throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL format( const LogRecord& Record ) throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getTail(  ) throw (RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception) SAL_OVERRIDE;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) SAL_OVERRIDE;

    public:
        // XServiceInfo - static version
        static OUString SAL_CALL getImplementationName_static();
        static Sequence< OUString > SAL_CALL getSupportedServiceNames_static();
        static Reference< XInterface > Create( const Reference< XComponentContext >& _rxContext );
    };


    //= PlainTextFormatter - implementation


    PlainTextFormatter::PlainTextFormatter()
    {
    }


    PlainTextFormatter::~PlainTextFormatter()
    {
    }


    OUString SAL_CALL PlainTextFormatter::getHead(  ) throw (RuntimeException, std::exception)
    {
        OUStringBuffer aHeader;
        aHeader.appendAscii( "  event no" );                 // column 1: the event number
        aHeader.appendAscii( " " );
        aHeader.appendAscii( "thread  " );                   // column 2: the thread ID
        aHeader.appendAscii( " " );
        aHeader.appendAscii( "date      " );                 // column 3: date
        aHeader.appendAscii( " " );
        aHeader.appendAscii( "time       " );         // column 4: time
        aHeader.appendAscii( " " );
        aHeader.appendAscii( "(class/method:) message" );    // column 5: class/method/message
        aHeader.appendAscii( "\n" );
        return aHeader.makeStringAndClear();
    }


    OUString SAL_CALL PlainTextFormatter::format( const LogRecord& _rRecord ) throw (RuntimeException, std::exception)
    {
        char buffer[ 30 ];
        const int buffer_size = sizeof( buffer );
        int used = snprintf( buffer, buffer_size, "%10i", (int)_rRecord.SequenceNumber );
        if ( used >= buffer_size || used < 0 )
            buffer[ buffer_size - 1 ] = 0;

        OUStringBuffer aLogEntry;
        aLogEntry.appendAscii( buffer );
        aLogEntry.appendAscii( " " );

        OString sThreadID( OUStringToOString( _rRecord.ThreadID, osl_getThreadTextEncoding() ) );
        snprintf( buffer, buffer_size, "%8s", sThreadID.getStr() );
        aLogEntry.appendAscii( buffer );
        aLogEntry.appendAscii( " " );

        snprintf( buffer, buffer_size, "%04i-%02i-%02i %02i:%02i:%02i.%09i",
            (int)_rRecord.LogTime.Year, (int)_rRecord.LogTime.Month, (int)_rRecord.LogTime.Day,
            (int)_rRecord.LogTime.Hours, (int)_rRecord.LogTime.Minutes, (int)_rRecord.LogTime.Seconds, (int)_rRecord.LogTime.NanoSeconds );
        aLogEntry.appendAscii( buffer );
        aLogEntry.appendAscii( " " );

        if ( !(_rRecord.SourceClassName.isEmpty() || _rRecord.SourceMethodName.isEmpty()) )
        {
            aLogEntry.append( _rRecord.SourceClassName );
            aLogEntry.appendAscii( "::" );
            aLogEntry.append( _rRecord.SourceMethodName );
            aLogEntry.appendAscii( ": " );
        }

        aLogEntry.append( _rRecord.Message );
        aLogEntry.appendAscii( "\n" );

        return aLogEntry.makeStringAndClear();
    }


    OUString SAL_CALL PlainTextFormatter::getTail(  ) throw (RuntimeException, std::exception)
    {
        // no tail
        return OUString();
    }

    sal_Bool SAL_CALL PlainTextFormatter::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
    {
        return cppu::supportsService(this, _rServiceName);
    }


    OUString SAL_CALL PlainTextFormatter::getImplementationName() throw(RuntimeException, std::exception)
    {
        return getImplementationName_static();
    }


    Sequence< OUString > SAL_CALL PlainTextFormatter::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        return getSupportedServiceNames_static();
    }


    OUString SAL_CALL PlainTextFormatter::getImplementationName_static()
    {
        return OUString( "com.sun.star.comp.extensions.PlainTextFormatter" );
    }


    Sequence< OUString > SAL_CALL PlainTextFormatter::getSupportedServiceNames_static()
    {
        Sequence< OUString > aServiceNames(1);
        aServiceNames[0] = "com.sun.star.logging.PlainTextFormatter";
        return aServiceNames;
    }


    Reference< XInterface > PlainTextFormatter::Create( const Reference< XComponentContext >& )
    {
        return *( new PlainTextFormatter );
    }


    void createRegistryInfo_PlainTextFormatter()
    {
        static OAutoRegistration< PlainTextFormatter > aAutoRegistration;
    }


} // namespace logging


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
