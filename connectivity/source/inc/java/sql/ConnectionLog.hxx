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

#ifndef CONNECTIVITY_CONNECTIONLOG_HXX
#define CONNECTIVITY_CONNECTIONLOG_HXX

/** === begin UNO includes === **/
#include <com/sun/star/logging/LogLevel.hpp>
/** === end UNO includes === **/

#include <rtl/ustring.hxx>

// Strange enough, GCC requires the following forward declarations of the various
// convertLogArgToString flavors to be *before* the inclusion of comphelper/logging.hxx

namespace com { namespace sun { namespace star { namespace util
{
    struct Date;
    struct Time;
    struct DateTime;
} } } }

//........................................................................
namespace comphelper { namespace log { namespace convert
{
//........................................................................

    // helpers for logging more data types than are defined in comphelper/logging.hxx
    ::rtl::OUString convertLogArgToString( const ::com::sun::star::util::Date& _rDate );
    ::rtl::OUString convertLogArgToString( const ::com::sun::star::util::Time& _rTime );
    ::rtl::OUString convertLogArgToString( const ::com::sun::star::util::DateTime& _rDateTime );

//........................................................................
} } }
//........................................................................

#include <comphelper/logging.hxx>

namespace connectivity
{
    namespace LogLevel = ::com::sun::star::logging::LogLevel;
}

//........................................................................
namespace connectivity { namespace java { namespace sql {
//........................................................................

    //====================================================================
    //= ConnectionLog
    //====================================================================
    typedef ::comphelper::ResourceBasedEventLogger  ConnectionLog_Base;
    class ConnectionLog : public ConnectionLog_Base
    {
    public:
        enum ObjectType
        {
            CONNECTION = 0,
            STATEMENT,
            RESULTSET,

            ObjectTypeCount = RESULTSET + 1
        };

    private:
        const   sal_Int32   m_nObjectID;

    public:
        /// will construct an instance of ObjectType CONNECTION
        ConnectionLog( const ::comphelper::ResourceBasedEventLogger& _rDriverLog );
        /// will create an instance with the same object ID / ObjectType as a given source instance
        ConnectionLog( const ConnectionLog& _rSourceLog );
        /// will create an instance of arbitrary ObjectType
        ConnectionLog( const ConnectionLog& _rSourceLog, ObjectType _eType );

        sal_Int32   getObjectID() const { return m_nObjectID; }

        /// logs a given message, without any arguments, or source class/method names
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID )
        {
            return ConnectionLog_Base::log( _nLogLevel, _nMessageResID, m_nObjectID );
        }

        template< typename ARGTYPE1 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1 ) const
        {
            return ConnectionLog_Base::log( _nLogLevel, _nMessageResID, m_nObjectID, _argument1 );
        }

        template< typename ARGTYPE1, typename ARGTYPE2 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            return ConnectionLog_Base::log( _nLogLevel, _nMessageResID, m_nObjectID, _argument1, _argument2 );
        }

        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
        {
            return ConnectionLog_Base::log( _nLogLevel, _nMessageResID, m_nObjectID, _argument1, _argument2, _argument3 );
        }

        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4 ) const
        {
            return ConnectionLog_Base::log( _nLogLevel, _nMessageResID, m_nObjectID, _argument1, _argument2, _argument3, _argument4 );
        }

        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5 >
        bool        log( const sal_Int32 _nLogLevel, const sal_Int32 _nMessageResID, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5 ) const
        {
            return ConnectionLog_Base::log( _nLogLevel, _nMessageResID, m_nObjectID, _argument1, _argument2, _argument3, _argument4, _argument5 );
        }
    };

//........................................................................
} } } // namespace connectivity::java::sql
//........................................................................

#endif // CONNECTIVITY_CONNECTIONLOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
