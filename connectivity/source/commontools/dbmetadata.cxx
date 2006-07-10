/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbmetadata.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:20:08 $
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

#ifndef CONNECTIVITY_INC_CONNECTIVITY_DBMETADATA_HXX
#include <connectivity/dbmetadata.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
/** === end UNO includes === **/

#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

//........................................................................
namespace dbtools
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::sdbc::XDatabaseMetaData;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::Exception;
    /** === end UNO using === **/

    //====================================================================
    //= DatabaseMetaData_Impl
    //====================================================================
    struct DatabaseMetaData_Impl
    {
        Reference< XConnection >        xConnection;
        Reference< XDatabaseMetaData >  xConnectionMetaData;
    };

    //--------------------------------------------------------------------
    namespace
    {
        static void lcl_construct( DatabaseMetaData_Impl& _metaDataImpl, const Reference< XConnection >& _connection )
        {
            _metaDataImpl.xConnection = _connection;
            if ( !_metaDataImpl.xConnection.is() )
                return;

            _metaDataImpl.xConnectionMetaData = _connection->getMetaData();
            if ( !_metaDataImpl.xConnectionMetaData.is() )
                throw IllegalArgumentException();
        }

        static void lcl_checkConnected( DatabaseMetaData_Impl& _metaDataImpl )
        {
            if ( !_metaDataImpl.xConnection.is() )
                throwSQLException( "not connected", SQL_CONNECTION_DOES_NOT_EXIST, NULL );
        }
    }

    //====================================================================
    //= DatabaseMetaData
    //====================================================================
    //--------------------------------------------------------------------
    DatabaseMetaData::DatabaseMetaData()
        :m_pImpl( new DatabaseMetaData_Impl )
    {
    }

    //--------------------------------------------------------------------
    DatabaseMetaData::DatabaseMetaData( const Reference< XConnection >& _connection )
        :m_pImpl( new DatabaseMetaData_Impl )
    {
        lcl_construct( *m_pImpl, _connection );
    }

    //--------------------------------------------------------------------
    DatabaseMetaData::DatabaseMetaData( const DatabaseMetaData& _copyFrom )
        :m_pImpl( new DatabaseMetaData_Impl( *_copyFrom.m_pImpl ) )
    {
    }

    //--------------------------------------------------------------------
    DatabaseMetaData& DatabaseMetaData::operator=( const DatabaseMetaData& _copyFrom )
    {
        if ( this == &_copyFrom )
            return *this;

        m_pImpl.reset( new DatabaseMetaData_Impl( *_copyFrom.m_pImpl ) );
        return *this;
    }

    //--------------------------------------------------------------------
    DatabaseMetaData::~DatabaseMetaData()
    {
    }

    //--------------------------------------------------------------------
    bool DatabaseMetaData::isConnected() const
    {
        return m_pImpl->xConnection.is();
    }

    //--------------------------------------------------------------------
    bool SAL_CALL DatabaseMetaData::supportsSubqueriesInFrom() const
    {
        lcl_checkConnected( *m_pImpl );

        bool supportsSubQueries = false;
        try
        {
            sal_Int32 maxTablesInselect = m_pImpl->xConnectionMetaData->getMaxTablesInSelect();
            supportsSubQueries = ( maxTablesInselect > 1 ) || ( maxTablesInselect == 0 );
            // TODO: is there a better way to determine this? The above is not really true. More precise,
            // it's a *very* generous heuristics ...
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return supportsSubQueries;
    }

//........................................................................
} // namespace dbtools
//........................................................................

