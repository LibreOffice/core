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



#ifndef LOGRECORD_HXX
#define LOGRECORD_HXX

/** === begin UNO includes === **/
#include <com/sun/star/logging/LogRecord.hpp>
/** === end UNO includes === **/

#include <osl/interlck.h>

//........................................................................
namespace logging
{
//........................................................................

    //====================================================================
    //= helper
    //====================================================================
    ::com::sun::star::logging::LogRecord createLogRecord(
        const ::rtl::OUString& _rLoggerName,
        const ::rtl::OUString& _rClassName,
        const ::rtl::OUString& _rMethodName,
        const ::rtl::OUString& _rMessage,
        sal_Int32 _nLogLevel,
        oslInterlockedCount _nEventNumber
    );

    inline ::com::sun::star::logging::LogRecord createLogRecord(
        const ::rtl::OUString& _rLoggerName,
        const ::rtl::OUString& _rMessage,
        sal_Int32 _nLogLevel,
        oslInterlockedCount _nEventNumber
    )
    {
        return createLogRecord( _rLoggerName, ::rtl::OUString(), ::rtl::OUString(), _rMessage, _nLogLevel, _nEventNumber );
    }

//........................................................................
} // namespace logging
//........................................................................

#endif // LOGRECORD_HXX
