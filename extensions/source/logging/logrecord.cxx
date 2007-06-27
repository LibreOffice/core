/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: logrecord.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 14:59:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef LOGRECORD_HXX
#include "logrecord.hxx"
#endif

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

