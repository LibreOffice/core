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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_CONNECTIONLOG_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_CONNECTIONLOG_HXX

#include <com/sun/star/logging/LogLevel.hpp>

#include <rtl/ustring.hxx>

// Strange enough, GCC requires the following forward declarations of the various
// convertLogArgToString flavors to be *before* the inclusion of comphelper/logging.hxx

namespace com { namespace sun { namespace star { namespace util
{
    struct Date;
    struct Time;
    struct DateTime;
} } } }


namespace comphelper { namespace log { namespace convert
{


    // helpers for logging more data types than are defined in comphelper/logging.hxx
    OUString convertLogArgToString( const css::util::Date& _rDate );
    OUString convertLogArgToString( const css::util::Time& _rTime );
    OUString convertLogArgToString( const css::util::DateTime& _rDateTime );


} } }


#include <comphelper/logging.hxx>

namespace connectivity
{
    namespace LogLevel = css::logging::LogLevel;
}


namespace connectivity { namespace java { namespace sql {

    typedef ::comphelper::EventLogger ConnectionLog_Base;
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
        ConnectionLog( const ::comphelper::EventLogger & _rDriverLog );
        /// will create an instance with the same object ID / ObjectType as a given source instance
        ConnectionLog( const ConnectionLog& _rSourceLog );
        /// will create an instance of arbitrary ObjectType
        ConnectionLog( const ConnectionLog& _rSourceLog, ObjectType _eType );

        sal_Int32   getObjectID() const { return m_nObjectID; }

        /// logs a given message, without any arguments, or source class/method names
        void        log( const sal_Int32 _nLogLevel, const OUString& rMessage )
        {
            ConnectionLog_Base::log( _nLogLevel, rMessage, m_nObjectID );
        }

        template< typename ARGTYPE1 >
        void        log( const sal_Int32 _nLogLevel, const OUString& rMessage, ARGTYPE1 _argument1 ) const
        {
            ConnectionLog_Base::log( _nLogLevel, rMessage, m_nObjectID, _argument1 );
        }

        template< typename ARGTYPE1, typename ARGTYPE2 >
        void        log( const sal_Int32 _nLogLevel, const OUString& rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2 ) const
        {
            ConnectionLog_Base::log( _nLogLevel, rMessage, m_nObjectID, _argument1, _argument2 );
        }

        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3 >
        void        log( const sal_Int32 _nLogLevel, const OUString& rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3 ) const
        {
            ConnectionLog_Base::log( _nLogLevel, rMessage, m_nObjectID, _argument1, _argument2, _argument3 );
        }

        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4 >
        void        log( const sal_Int32 _nLogLevel, const OUString& rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4 ) const
        {
            ConnectionLog_Base::log( _nLogLevel, rMessage, m_nObjectID, _argument1, _argument2, _argument3, _argument4 );
        }

        template< typename ARGTYPE1, typename ARGTYPE2, typename ARGTYPE3, typename ARGTYPE4, typename ARGTYPE5 >
        void        log( const sal_Int32 _nLogLevel, const OUString& rMessage, ARGTYPE1 _argument1, ARGTYPE2 _argument2, ARGTYPE3 _argument3, ARGTYPE4 _argument4, ARGTYPE5 _argument5 ) const
        {
            ConnectionLog_Base::log( _nLogLevel, rMessage, m_nObjectID, _argument1, _argument2, _argument3, _argument4, _argument5 );
        }
    };


} } } // namespace connectivity::java::sql


#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_CONNECTIONLOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
