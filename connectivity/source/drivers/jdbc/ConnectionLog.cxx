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


#include "java/sql/ConnectionLog.hxx"

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>

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
            return osl_atomic_increment( s_nCounts + _eType );
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
