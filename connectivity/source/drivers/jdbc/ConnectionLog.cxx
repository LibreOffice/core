/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConnectionLog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 14:35:30 $
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
#include "precompiled_connectivity.hxx"

#include "java/sql/ConnectionLog.hxx"

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
/** === end UNO includes === **/

#include <stdio.h>

//........................................................................
namespace connectivity { namespace java { namespace sql {
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    /** === end UNO using === **/

    //--------------------------------------------------------------------
    namespace
    {
        sal_Int32 lcl_getFreeID( ConnectionLog::ObjectType _eType )
        {
            static oslInterlockedCount s_nCounts[ ConnectionLog::ObjectTypeCount ] = { 0, 0 };
            return osl_incrementInterlockedCount( s_nCounts + _eType );
        }
    }

    //====================================================================
    //= ConnectionLog
    //====================================================================
    //--------------------------------------------------------------------
    ConnectionLog::ConnectionLog( const ::comphelper::ResourceBasedEventLogger& _rDriverLog )
        :ConnectionLog_Base( _rDriverLog )
        ,m_nObjectID( lcl_getFreeID( CONNECTION ) )
    {
    }

    //--------------------------------------------------------------------
    ConnectionLog::ConnectionLog( const ConnectionLog& _rSourceLog )
        :ConnectionLog_Base( _rSourceLog )
        ,m_nObjectID( _rSourceLog.m_nObjectID )
    {
    }

    //--------------------------------------------------------------------
    ConnectionLog::ConnectionLog( const ConnectionLog& _rSourceLog, ConnectionLog::ObjectType _eType )
        :ConnectionLog_Base( _rSourceLog )
        ,m_nObjectID( lcl_getFreeID( _eType ) )
    {
    }

//........................................................................
} } } // namespace connectivity::java::sql
//........................................................................

//........................................................................
namespace comphelper { namespace log { namespace convert
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::util::Date;
    using ::com::sun::star::util::Time;
    using ::com::sun::star::util::DateTime;
    /** === end UNO using === **/

    //--------------------------------------------------------------------
    ::rtl::OUString convertLogArgToString( const Date& _rDate )
    {
        char buffer[ 30 ];
        const size_t buffer_size = sizeof( buffer );
        snprintf( buffer, buffer_size, "%04i-%02i-%02i",
            (int)_rDate.Year, (int)_rDate.Month, (int)_rDate.Day );
        return ::rtl::OUString::createFromAscii( buffer );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString convertLogArgToString( const Time& _rTime )
    {
        char buffer[ 30 ];
        const size_t buffer_size = sizeof( buffer );
        snprintf( buffer, buffer_size, "%02i:%02i:%02i.%02i",
            (int)_rTime.Hours, (int)_rTime.Minutes, (int)_rTime.Seconds, (int)_rTime.HundredthSeconds );
        return ::rtl::OUString::createFromAscii( buffer );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString convertLogArgToString( const DateTime& _rDateTime )
    {
        char buffer[ 30 ];
        const size_t buffer_size = sizeof( buffer );
        snprintf( buffer, buffer_size, "%04i-%02i-%02i %02i:%02i:%02i.%02i",
            (int)_rDateTime.Year, (int)_rDateTime.Month, (int)_rDateTime.Day,
            (int)_rDateTime.Hours, (int)_rDateTime.Minutes, (int)_rDateTime.Seconds, (int)_rDateTime.HundredthSeconds );
        return ::rtl::OUString::createFromAscii( buffer );
    }

//........................................................................
} } }   // comphelper::log::convert
//........................................................................
