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

#include <com/sun/star/logging/XCsvLogFormatter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <rtl/ustrbuf.hxx>

#include <stdio.h>
#include <string_view>

namespace logging
{
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::logging::LogRecord;

    namespace {

    // formats for csv files as defined by RFC4180
    class CsvFormatter : public cppu::WeakImplHelper<css::logging::XCsvLogFormatter, css::lang::XServiceInfo>
    {
    public:
        virtual OUString SAL_CALL formatMultiColumn(const Sequence< OUString>& column_data) override;

        CsvFormatter();

    private:
        // XCsvLogFormatter
        virtual sal_Bool SAL_CALL getLogEventNo() override;
        virtual sal_Bool SAL_CALL getLogThread() override;
        virtual sal_Bool SAL_CALL getLogTimestamp() override;
        virtual sal_Bool SAL_CALL getLogSource() override;
        virtual Sequence< OUString > SAL_CALL getColumnnames() override;

        virtual void SAL_CALL setLogEventNo( sal_Bool log_event_no ) override;
        virtual void SAL_CALL setLogThread( sal_Bool log_thread ) override;
        virtual void SAL_CALL setLogTimestamp( sal_Bool log_timestamp ) override;
        virtual void SAL_CALL setLogSource( sal_Bool log_source ) override;
        virtual void SAL_CALL setColumnnames( const Sequence< OUString>& column_names) override;

        // XLogFormatter
        virtual OUString SAL_CALL getHead(  ) override;
        virtual OUString SAL_CALL format( const LogRecord& Record ) override;
        virtual OUString SAL_CALL getTail(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& service_name ) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    private:
        bool m_LogEventNo;
        bool m_LogThread;
        bool m_LogTimestamp;
        bool m_LogSource;
        bool m_MultiColumn;
        css::uno::Sequence< OUString > m_Columnnames;
    };

    }
} // namespace logging

// private helpers
namespace
{
    const sal_Unicode quote_char = '"';
    const sal_Unicode comma_char = ',';
    const OUStringLiteral dos_newline = u"\r\n";

    bool needsQuoting(const OUString& str)
    {
        return std::u16string_view(str).find_first_of(u"\",\n\r") != std::u16string_view::npos;
    };

    void appendEncodedString(OUStringBuffer& buf, const OUString& str)
    {
        if(needsQuoting(str))
        {
            // each double-quote will get replaced by two double-quotes
            buf.append(quote_char);
            const sal_Int32 buf_offset = buf.getLength();
            const sal_Int32 str_length = str.getLength();
            buf.append(str);
            // special treatment for the last character
            if(quote_char==str[str_length-1])
                buf.append(quote_char);
            // iterating backwards because the index at which we insert won't be shifted
            // when moving that way.
            for(sal_Int32 i = str_length; i>=0; )
            {
                i=str.lastIndexOf(quote_char, --i);
                if(i!=-1)
                    buf.insert(buf_offset + i, quote_char);
            }
            buf.append(quote_char);
        }
        else
            buf.append(str);
    };
}

namespace logging
{
    CsvFormatter::CsvFormatter()
        :m_LogEventNo(true),
        m_LogThread(true),
        m_LogTimestamp(true),
        m_LogSource(false),
        m_MultiColumn(false),
        m_Columnnames({ "message" })
    { }

    sal_Bool CsvFormatter::getLogEventNo()
    {
        return m_LogEventNo;
    }

    sal_Bool CsvFormatter::getLogThread()
    {
        return m_LogThread;
    }

    sal_Bool CsvFormatter::getLogTimestamp()
    {
        return m_LogTimestamp;
    }

    sal_Bool CsvFormatter::getLogSource()
    {
        return m_LogSource;
    }

    Sequence< OUString > CsvFormatter::getColumnnames()
    {
        return m_Columnnames;
    }

    void CsvFormatter::setLogEventNo(sal_Bool log_event_no)
    {
        m_LogEventNo = log_event_no;
    }

    void CsvFormatter::setLogThread(sal_Bool log_thread)
    {
        m_LogThread = log_thread;
    }

    void CsvFormatter::setLogTimestamp(sal_Bool log_timestamp)
    {
        m_LogTimestamp = log_timestamp;
    }

    void CsvFormatter::setLogSource(sal_Bool log_source)
    {
        m_LogSource = log_source;
    }

    void CsvFormatter::setColumnnames(const Sequence< OUString >& columnnames)
    {
        m_Columnnames = columnnames;
        m_MultiColumn = (m_Columnnames.getLength()>1);
    }

    OUString SAL_CALL CsvFormatter::getHead(  )
    {
        OUStringBuffer buf;
        if(m_LogEventNo)
            buf.append("event no,");
        if(m_LogThread)
            buf.append("thread,");
        if(m_LogTimestamp)
            buf.append("timestamp,");
        if(m_LogSource)
            buf.append("class,method,");
        sal_Int32 columns = m_Columnnames.getLength();
        for(sal_Int32 i=0; i<columns; i++)
        {
            buf.append(m_Columnnames[i]);
            buf.append(comma_char);
        }
        buf.setLength(buf.getLength()-1);
        buf.append(dos_newline);
        return buf.makeStringAndClear();
    }

    OUString SAL_CALL CsvFormatter::format( const LogRecord& record )
    {
        OUStringBuffer aLogEntry;

        if(m_LogEventNo)
        {
            aLogEntry.append( record.SequenceNumber );
            aLogEntry.append(comma_char);
        }

        if(m_LogThread)
        {
            aLogEntry.append( record.ThreadID );
            aLogEntry.append(comma_char);
        }

        if(m_LogTimestamp)
        {
            if (   record.LogTime.Year < -9999 || 9999 < record.LogTime.Year
                || record.LogTime.Month < 1 || 12 < record.LogTime.Month
                || record.LogTime.Day < 1 || 31 < record.LogTime.Day
                || 24 < record.LogTime.Hours
                || 60 < record.LogTime.Minutes
                || 60 < record.LogTime.Seconds
                || 999999999 < record.LogTime.NanoSeconds)
            {
                throw css::lang::IllegalArgumentException("invalid date", static_cast<cppu::OWeakObject*>(this), 1);
            }

            // ISO 8601
            char buffer[ 31 ];
            const size_t buffer_size = sizeof( buffer );
            snprintf( buffer, buffer_size, "%04i-%02i-%02iT%02i:%02i:%02i.%09i",
                static_cast<int>(record.LogTime.Year),
                static_cast<int>(record.LogTime.Month),
                static_cast<int>(record.LogTime.Day),
                static_cast<int>(record.LogTime.Hours),
                static_cast<int>(record.LogTime.Minutes),
                static_cast<int>(record.LogTime.Seconds),
                static_cast<int>(record.LogTime.NanoSeconds) );
            aLogEntry.appendAscii( buffer );
            aLogEntry.append(comma_char);
        }

        if(m_LogSource)
        {
            appendEncodedString(aLogEntry, record.SourceClassName);
            aLogEntry.append(comma_char);

            appendEncodedString(aLogEntry, record.SourceMethodName);
            aLogEntry.append(comma_char);
        }

        // if the CsvFormatter has multiple columns set via setColumnnames(), the
        // message of the record is expected to be encoded with formatMultiColumn
        // if the CsvFormatter has only one column set, the message is expected not
        // to be encoded
        if(m_MultiColumn)
            aLogEntry.append(record.Message);
        else
            appendEncodedString(aLogEntry, record.Message);

        aLogEntry.append( dos_newline );
        return aLogEntry.makeStringAndClear();
    }

    OUString SAL_CALL CsvFormatter::getTail(  )
    {
        return OUString();
    }

    OUString SAL_CALL CsvFormatter::formatMultiColumn(const Sequence< OUString>& column_data)
    {
        sal_Int32 columns = column_data.getLength();
        OUStringBuffer buf;
        for(int i=0; i<columns; i++)
        {
            appendEncodedString(buf, column_data[i]);
            buf.append(comma_char);
        }
        buf.setLength(buf.getLength()-1);
        return buf.makeStringAndClear();
    }

    sal_Bool SAL_CALL CsvFormatter::supportsService( const OUString& service_name )
    {
        return cppu::supportsService(this, service_name);
    }

    OUString SAL_CALL CsvFormatter::getImplementationName()
    {
        return "com.sun.star.comp.extensions.CsvFormatter";
    }

    Sequence< OUString > SAL_CALL CsvFormatter::getSupportedServiceNames()
    {
        return { "com.sun.star.logging.CsvFormatter" };
    }

} // namespace logging

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_extensions_CsvFormatter(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new logging::CsvFormatter());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
