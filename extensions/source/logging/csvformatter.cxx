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
#include <string>

#include <com/sun/star/logging/XCsvLogFormatter.hpp>
#include <com/sun/star/logging/XLogFormatter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/componentcontext.hxx>

#include <cppuhelper/implbase2.hxx>

#include <rtl/ustrbuf.hxx>

#include <osl/thread.h>

namespace logging
{

    /** === begin UNO using === **/
    using ::com::sun::star::logging::XCsvLogFormatter;
    using ::com::sun::star::logging::XLogFormatter;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::logging::LogRecord;
    using ::com::sun::star::uno::XInterface;
    /** === end UNO using === **/

    //= CsvFormatter - declaration
    //= formats for csv files as defined by RFC4180
    typedef ::cppu::WeakImplHelper2 <   XCsvLogFormatter
                                    ,   XServiceInfo
                                    >   CsvFormatter_Base;
    class CsvFormatter : public CsvFormatter_Base
    {
    public:
        virtual ::rtl::OUString SAL_CALL formatMultiColumn(const Sequence< ::rtl::OUString>& column_data) throw (RuntimeException);

        // XServiceInfo - static version
        static ::rtl::OUString SAL_CALL getImplementationName_static();
        static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static();
        static Reference< XInterface > Create( const Reference< XComponentContext >& context );

    protected:
        CsvFormatter( const Reference< XComponentContext >& context );
        virtual ~CsvFormatter();

        // XCsvLogFormatter
        virtual ::sal_Bool SAL_CALL getLogEventNo() throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL getLogThread() throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL getLogTimestamp() throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL getLogSource() throw (RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getColumnnames() throw (RuntimeException);

        virtual void SAL_CALL setLogEventNo( ::sal_Bool log_event_no ) throw (RuntimeException);
        virtual void SAL_CALL setLogThread( ::sal_Bool log_thread ) throw (RuntimeException);
        virtual void SAL_CALL setLogTimestamp( ::sal_Bool log_timestamp ) throw (RuntimeException);
        virtual void SAL_CALL setLogSource( ::sal_Bool log_source ) throw (RuntimeException);
        virtual void SAL_CALL setColumnnames( const Sequence< ::rtl::OUString>& column_names) throw (RuntimeException);

        // XLogFormatter
        virtual ::rtl::OUString SAL_CALL getHead(  ) throw (RuntimeException);
        virtual ::rtl::OUString SAL_CALL format( const LogRecord& Record ) throw (RuntimeException);
        virtual ::rtl::OUString SAL_CALL getTail(  ) throw (RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& service_name ) throw(RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

    private:
        ::comphelper::ComponentContext m_aContext;
        ::sal_Bool m_LogEventNo;
        ::sal_Bool m_LogThread;
        ::sal_Bool m_LogTimestamp;
        ::sal_Bool m_LogSource;
        ::sal_Bool m_MultiColumn;
        ::com::sun::star::uno::Sequence< ::rtl::OUString > m_Columnnames;
    };
} // namespace logging

//= private helpers
namespace
{
    const sal_Unicode quote_char = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\"")).toChar();
    const sal_Unicode comma_char = ::rtl::OUString(",").toChar();
    const ::rtl::OUString dos_newline = ::rtl::OUString("\r\n");

    inline bool needsQuoting(const ::rtl::OUString& str)
    {
        static const ::rtl::OUString quote_trigger_chars = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("\",\n\r"));
        sal_Int32 len = str.getLength();
        for(sal_Int32 i=0; i<len; i++)
            if(quote_trigger_chars.indexOf(str[i])!=-1)
                return true;
        return false;
    };

    inline void appendEncodedString(::rtl::OUStringBuffer& buf, const ::rtl::OUString& str)
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
            // iterating backwards because the index at which we insert wont be shifted
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

    ::com::sun::star::uno::Sequence< ::rtl::OUString> initialColumns()
    {
        com::sun::star::uno::Sequence< ::rtl::OUString> result = ::com::sun::star::uno::Sequence< ::rtl::OUString>(1);
        result[0] = ::rtl::OUString("message");
        return result;
    };
}

//= CsvFormatter - implementation
namespace logging
{
    CsvFormatter::CsvFormatter( const Reference< XComponentContext >& context )
        :m_aContext( context ),
        m_LogEventNo(true),
        m_LogThread(true),
        m_LogTimestamp(true),
        m_LogSource(false),
        m_MultiColumn(false),
        m_Columnnames(initialColumns())
    { }

    CsvFormatter::~CsvFormatter()
    { }

    ::sal_Bool CsvFormatter::getLogEventNo() throw (RuntimeException)
    {
        return m_LogEventNo;
    }

    ::sal_Bool CsvFormatter::getLogThread() throw (RuntimeException)
    {
        return m_LogThread;
    }

    ::sal_Bool CsvFormatter::getLogTimestamp() throw (RuntimeException)
    {
        return m_LogTimestamp;
    }

    ::sal_Bool CsvFormatter::getLogSource() throw (RuntimeException)
    {
        return m_LogSource;
    }

    Sequence< ::rtl::OUString > CsvFormatter::getColumnnames() throw (RuntimeException)
    {
        return m_Columnnames;
    }

    void CsvFormatter::setLogEventNo(::sal_Bool log_event_no) throw (RuntimeException)
    {
        m_LogEventNo = log_event_no;
    }

    void CsvFormatter::setLogThread(::sal_Bool log_thread) throw (RuntimeException)
    {
        m_LogThread = log_thread;
    }

    void CsvFormatter::setLogTimestamp(::sal_Bool log_timestamp) throw (RuntimeException)
    {
        m_LogTimestamp = log_timestamp;
    }

    void CsvFormatter::setLogSource(::sal_Bool log_source) throw (RuntimeException)
    {
        m_LogSource = log_source;
    }

    void CsvFormatter::setColumnnames(const Sequence< ::rtl::OUString >& columnnames) throw (RuntimeException)
    {
        m_Columnnames = Sequence< ::rtl::OUString>(columnnames);
        m_MultiColumn = (m_Columnnames.getLength()>1);
    }

    ::rtl::OUString SAL_CALL CsvFormatter::getHead(  ) throw (RuntimeException)
    {
        ::rtl::OUStringBuffer buf;
        if(m_LogEventNo)
            buf.appendAscii("event no,");
        if(m_LogThread)
            buf.appendAscii("thread,");
        if(m_LogTimestamp)
            buf.appendAscii("timestamp,");
        if(m_LogSource)
            buf.appendAscii("class,method,");
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

    ::rtl::OUString SAL_CALL CsvFormatter::format( const LogRecord& record ) throw (RuntimeException)
    {
        ::rtl::OUStringBuffer aLogEntry;

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
            // ISO 8601
            char buffer[ 30 ];
            const size_t buffer_size = sizeof( buffer );
            snprintf( buffer, buffer_size, "%04i-%02i-%02iT%02i:%02i:%02i.%02i",
                (int)record.LogTime.Year,
                (int)record.LogTime.Month,
                (int)record.LogTime.Day,
                (int)record.LogTime.Hours,
                (int)record.LogTime.Minutes,
                (int)record.LogTime.Seconds,
                (int)record.LogTime.HundredthSeconds );
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

    ::rtl::OUString SAL_CALL CsvFormatter::getTail(  ) throw (RuntimeException)
    {
        return ::rtl::OUString();
    }

    ::rtl::OUString SAL_CALL CsvFormatter::formatMultiColumn(const Sequence< ::rtl::OUString>& column_data) throw (RuntimeException)
    {
        sal_Int32 columns = column_data.getLength();
        ::rtl::OUStringBuffer buf;
        for(int i=0; i<columns; i++)
        {
            appendEncodedString(buf, column_data[i]);
            buf.append(comma_char);
        }
        buf.setLength(buf.getLength()-1);
        return buf.makeStringAndClear();
    }

    ::sal_Bool SAL_CALL CsvFormatter::supportsService( const ::rtl::OUString& service_name ) throw(RuntimeException)
    {
        const Sequence< ::rtl::OUString > aServiceNames( getSupportedServiceNames() );
        for (   const ::rtl::OUString* pServiceNames = aServiceNames.getConstArray();
                pServiceNames != aServiceNames.getConstArray() + aServiceNames.getLength();
                ++pServiceNames
            )
            if ( service_name == *pServiceNames )
                return sal_True;
        return sal_False;
    }

    ::rtl::OUString SAL_CALL CsvFormatter::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    Sequence< ::rtl::OUString > SAL_CALL CsvFormatter::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    ::rtl::OUString SAL_CALL CsvFormatter::getImplementationName_static()
    {
        return ::rtl::OUString( "com.sun.star.comp.extensions.CsvFormatter" );
    }

    Sequence< ::rtl::OUString > SAL_CALL CsvFormatter::getSupportedServiceNames_static()
    {
        Sequence< ::rtl::OUString > aServiceNames(1);
        aServiceNames[0] = ::rtl::OUString( "com.sun.star.logging.CsvFormatter" );
        return aServiceNames;
    }

    Reference< XInterface > CsvFormatter::Create( const Reference< XComponentContext >& context )
    {
        return *( new CsvFormatter( context ) );
    }

    void createRegistryInfo_CsvFormatter()
    {
        static OAutoRegistration< CsvFormatter > aAutoRegistration;
    }
} // namespace logging

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
