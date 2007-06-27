/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: logrecord.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 14:59:46 $
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

#ifndef LOGRECORD_HXX
#define LOGRECORD_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LOGGING_LOGRECORD_HPP_
#include <com/sun/star/logging/LogRecord.hpp>
#endif
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
