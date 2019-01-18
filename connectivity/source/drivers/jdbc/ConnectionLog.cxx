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


#include <java/sql/ConnectionLog.hxx>

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <stdio.h>


namespace connectivity { namespace java { namespace sql {


    using ::com::sun::star::uno::XComponentContext;


    namespace
    {
        sal_Int32 lcl_getFreeID( ConnectionLog::ObjectType _eType )
        {
            static oslInterlockedCount s_nCounts[ ConnectionLog::ObjectTypeCount ] = { 0, 0 };
            return osl_atomic_increment( s_nCounts + _eType );
        }
    }

    ConnectionLog::ConnectionLog( const ::comphelper::EventLogger& _rDriverLog )
        :ConnectionLog_Base( _rDriverLog )
        ,m_nObjectID( lcl_getFreeID( CONNECTION ) )
    {
    }


    ConnectionLog::ConnectionLog( const ConnectionLog& _rSourceLog )
        :ConnectionLog_Base( _rSourceLog )
        ,m_nObjectID( _rSourceLog.m_nObjectID )
    {
    }


    ConnectionLog::ConnectionLog( const ConnectionLog& _rSourceLog, ConnectionLog::ObjectType _eType )
        :ConnectionLog_Base( _rSourceLog )
        ,m_nObjectID( lcl_getFreeID( _eType ) )
    {
    }


} } } // namespace connectivity::java::sql


namespace comphelper { namespace log { namespace convert
{


    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::util::Date;
    using ::com::sun::star::util::Time;
    using ::com::sun::star::util::DateTime;


    OUString convertLogArgToString( const Date& _rDate )
    {
        char buffer[ 30 ];
        const size_t buffer_size = sizeof( buffer );
        snprintf( buffer, buffer_size, "%04i-%02i-%02i",
            static_cast<int>(_rDate.Year), static_cast<int>(_rDate.Month), static_cast<int>(_rDate.Day) );
        return OUString::createFromAscii( buffer );
    }


    OUString convertLogArgToString( const css::util::Time& _rTime )
    {
        char buffer[ 30 ];
        const size_t buffer_size = sizeof( buffer );
        snprintf( buffer, buffer_size, "%02i:%02i:%02i.%09i",
            static_cast<int>(_rTime.Hours), static_cast<int>(_rTime.Minutes), static_cast<int>(_rTime.Seconds), static_cast<int>(_rTime.NanoSeconds) );
        return OUString::createFromAscii( buffer );
    }


    OUString convertLogArgToString( const DateTime& _rDateTime )
    {
        char buffer[ sizeof("-32768-65535-65535 65535:65535:65535.4294967295") ];
            // reserve enough space for hypothetical max length
        const size_t buffer_size = sizeof( buffer );
        snprintf( buffer, buffer_size, "%04" SAL_PRIdINT32 "-%02" SAL_PRIuUINT32 "-%02" SAL_PRIuUINT32 " %02" SAL_PRIuUINT32 ":%02" SAL_PRIuUINT32 ":%02" SAL_PRIuUINT32 ".%09" SAL_PRIuUINT32,
            static_cast<sal_Int32>(_rDateTime.Year), static_cast<sal_uInt32>(_rDateTime.Month), static_cast<sal_uInt32>(_rDateTime.Day),
            static_cast<sal_uInt32>(_rDateTime.Hours), static_cast<sal_uInt32>(_rDateTime.Minutes), static_cast<sal_uInt32>(_rDateTime.Seconds), _rDateTime.NanoSeconds );
        return OUString::createFromAscii( buffer );
    }


} } }   // comphelper::log::convert


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
