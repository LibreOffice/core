/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "logrecord.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <osl/time.h>
#include <osl/thread.h>
#include <osl/diagnose.h>

//........................................................................
namespace logging
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::logging::LogRecord;
    using ::com::sun::star::util::DateTime;
    /** === end UNO using === **/

    //====================================================================
    //= helper
    //====================================================================
    //--------------------------------------------------------------------
    namespace
    {
        /** returns a string representation of the current thread

            @todo
                We need a way to retrieve the current UNO thread ID as string,
                which is issue #i77342#
        */
        ::rtl::OUString getCurrentThreadID()
        {
            oslThreadIdentifier nThreadID( osl_getThreadIdentifier( NULL ) );
            return ::rtl::OUString::valueOf( (sal_Int64)nThreadID );
        }
    }

    //--------------------------------------------------------------------
    LogRecord createLogRecord( const ::rtl::OUString& _rLoggerName, const ::rtl::OUString& _rClassName,
        const ::rtl::OUString& _rMethodName, const ::rtl::OUString& _rMessage,
        sal_Int32 _nLogLevel, oslInterlockedCount _nEventNumber )
    {
        TimeValue aTimeValue;
        osl_getSystemTime( &aTimeValue );

        oslDateTime aDateTime;
        OSL_VERIFY( osl_getDateTimeFromTimeValue( &aTimeValue, &aDateTime ) );

        DateTime aTimeStamp;
        aTimeStamp.Year = aDateTime.Year;
        aTimeStamp.Month = aDateTime.Month;
        aTimeStamp.Day = aDateTime.Day;
        aTimeStamp.Hours = aDateTime.Hours;
        aTimeStamp.Minutes = aDateTime.Minutes;
        aTimeStamp.Seconds = aDateTime.Seconds;
        aTimeStamp.HundredthSeconds = ::sal::static_int_cast< sal_Int16 >( aDateTime.NanoSeconds / 10000000 );

        return LogRecord(
            _rLoggerName,
            _rClassName,
            _rMethodName,
            _rMessage,
            aTimeStamp,
            _nEventNumber,
            getCurrentThreadID(),
            _nLogLevel
        );
    }

//........................................................................
} // namespace logging
//........................................................................

