/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: logrecord.hxx,v $
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
