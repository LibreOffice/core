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

#include "logrecord.hxx"

#include <osl/time.h>
#include <osl/thread.h>
#include <osl/thread.hxx>
#include <osl/diagnose.h>

//........................................................................
namespace logging
{
//........................................................................

    using ::com::sun::star::logging::LogRecord;
    using ::com::sun::star::util::DateTime;

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
        OUString getCurrentThreadID()
        {
            oslThreadIdentifier nThreadID( osl::Thread::getCurrentIdentifier() );
            return OUString::valueOf( (sal_Int64)nThreadID );
        }
    }

    //--------------------------------------------------------------------
    LogRecord createLogRecord( const OUString& _rLoggerName, const OUString& _rClassName,
        const OUString& _rMethodName, const OUString& _rMessage,
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
        aTimeStamp.NanoSeconds = aDateTime.NanoSeconds;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
