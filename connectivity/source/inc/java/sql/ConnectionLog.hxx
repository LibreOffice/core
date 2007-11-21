/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConnectionLog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:07:56 $
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

#ifndef CONNECTIVITY_CONNECTIONLOG_HXX
#define CONNECTIVITY_CONNECTIONLOG_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LOGGING_LOGLEVEL_HPP_
#include <com/sun/star/logging/LogLevel.hpp>
#endif
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
