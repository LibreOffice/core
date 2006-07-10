/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datasourcemetadata.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:19:17 $
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

#ifndef DBACCESS_DATASOURCEMETADATA_HXX
#include "datasourcemetadata.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
/** === end UNO includes === **/

#ifndef CONNECTIVITY_INC_CONNECTIVITY_DBMETADATA_HXX
#include <connectivity/dbmetadata.hxx>
#endif

//........................................................................
namespace sdbtools
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::uno::RuntimeException;
    /** === end UNO using === **/

    //====================================================================
    //= DataSourceMetaData_Impl
    //====================================================================
    struct DataSourceMetaData_Impl
    {
    };

    //====================================================================
    //= DataSourceMetaData
    //====================================================================
    //--------------------------------------------------------------------
    DataSourceMetaData::DataSourceMetaData( const Reference< XConnection >& _rxConnection )
        :m_pImpl( new DataSourceMetaData_Impl )
    {
        if ( !_rxConnection.is() )
            throw NullPointerException();
        setWeakConnection( _rxConnection );
    }

    //--------------------------------------------------------------------
    DataSourceMetaData::~DataSourceMetaData()
    {
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL DataSourceMetaData::supportsQueriesInFrom(  ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        ::dbtools::DatabaseMetaData aMeta( getConnection() );
        return aMeta.supportsSubqueriesInFrom();
    }

//........................................................................
} // namespace sdbtools
//........................................................................

