/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_connectivity.hxx"

#include "java/sql/ConnectionLog.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
