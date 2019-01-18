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

#include <sal/config.h>

#include <com/sun/star/logging/XLogFormatter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <rtl/ustrbuf.hxx>
#include <osl/thread.h>

#include <stdio.h>

namespace logging
{
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::logging::LogRecord;
    using ::com::sun::star::uno::XInterface;

    class PlainTextFormatter : public cppu::WeakImplHelper<css::logging::XLogFormatter, css::lang::XServiceInfo>
    {
    public:
        PlainTextFormatter();

    private:
        // XLogFormatter
        virtual OUString SAL_CALL getHead(  ) override;
        virtual OUString SAL_CALL format( const LogRecord& Record ) override;
        virtual OUString SAL_CALL getTail(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& _rServiceName ) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
    };

    PlainTextFormatter::PlainTextFormatter()
    {
    }

    OUString SAL_CALL PlainTextFormatter::getHead(  )
    {
        OUStringBuffer aHeader;
        aHeader.append( "  event no" );                 // column 1: the event number
        aHeader.append( " " );
        aHeader.append( "thread  " );                   // column 2: the thread ID
        aHeader.append( " " );
        aHeader.append( "date      " );                 // column 3: date
        aHeader.append( " " );
        aHeader.append( "time       " );         // column 4: time
        aHeader.append( " " );
        aHeader.append( "(class/method:) message" );    // column 5: class/method/message
        aHeader.append( "\n" );
        return aHeader.makeStringAndClear();
    }


    OUString SAL_CALL PlainTextFormatter::format( const LogRecord& _rRecord )
    {
        char buffer[ sizeof("-32768-65535-65535 65535:65535:65535.4294967295") ];
            // reserve enough space for hypothetical max length
        const int buffer_size = sizeof( buffer );
        int used = snprintf( buffer, buffer_size, "%10i", static_cast<int>(_rRecord.SequenceNumber) );
        if ( used >= buffer_size || used < 0 )
            buffer[ buffer_size - 1 ] = 0;

        OUStringBuffer aLogEntry;
        aLogEntry.appendAscii( buffer );
        aLogEntry.append( " " );

        OString sThreadID( OUStringToOString( _rRecord.ThreadID, osl_getThreadTextEncoding() ) );
        snprintf( buffer, buffer_size, "%8s", sThreadID.getStr() );
        aLogEntry.appendAscii( buffer );
        aLogEntry.append( " " );

        snprintf( buffer, buffer_size, "%04" SAL_PRIdINT32 "-%02" SAL_PRIuUINT32 "-%02" SAL_PRIuUINT32 " %02" SAL_PRIuUINT32 ":%02" SAL_PRIuUINT32 ":%02" SAL_PRIuUINT32 ".%09" SAL_PRIuUINT32,
            static_cast<sal_Int32>(_rRecord.LogTime.Year), static_cast<sal_uInt32>(_rRecord.LogTime.Month), static_cast<sal_uInt32>(_rRecord.LogTime.Day),
            static_cast<sal_uInt32>(_rRecord.LogTime.Hours), static_cast<sal_uInt32>(_rRecord.LogTime.Minutes), static_cast<sal_uInt32>(_rRecord.LogTime.Seconds), _rRecord.LogTime.NanoSeconds );
        aLogEntry.appendAscii( buffer );
        aLogEntry.append( " " );

        if ( !(_rRecord.SourceClassName.isEmpty() || _rRecord.SourceMethodName.isEmpty()) )
        {
            aLogEntry.append( _rRecord.SourceClassName );
            aLogEntry.append( "::" );
            aLogEntry.append( _rRecord.SourceMethodName );
            aLogEntry.append( ": " );
        }

        aLogEntry.append( _rRecord.Message );
        aLogEntry.append( "\n" );

        return aLogEntry.makeStringAndClear();
    }


    OUString SAL_CALL PlainTextFormatter::getTail(  )
    {
        // no tail
        return OUString();
    }

    sal_Bool SAL_CALL PlainTextFormatter::supportsService( const OUString& _rServiceName )
    {
        return cppu::supportsService(this, _rServiceName);
    }


    OUString SAL_CALL PlainTextFormatter::getImplementationName()
    {
        return OUString("com.sun.star.comp.extensions.PlainTextFormatter");
    }

    Sequence< OUString > SAL_CALL PlainTextFormatter::getSupportedServiceNames()
    {
        return { "com.sun.star.logging.PlainTextFormatter" };
    }

} // namespace logging

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_extensions_PlainTextFormatter(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new logging::PlainTextFormatter());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
