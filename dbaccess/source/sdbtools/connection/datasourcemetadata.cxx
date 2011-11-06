/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

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
    DataSourceMetaData::DataSourceMetaData( const ::comphelper::ComponentContext& _rContext, const Reference< XConnection >& _rxConnection )
        :ConnectionDependentComponent( _rContext )
        ,m_pImpl( new DataSourceMetaData_Impl )
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

