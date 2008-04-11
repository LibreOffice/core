/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: logrecord.cxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

