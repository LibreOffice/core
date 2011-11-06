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
#include "precompiled_connectivity.hxx"
#include "adabas/BPreparedStatement.hxx"
#include "adabas/BResultSet.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/dbexception.hxx>

using namespace connectivity;
using namespace connectivity::adabas;
using namespace connectivity::odbc;
//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

OAdabasPreparedStatement::OAdabasPreparedStatement( OAdabasConnection* _pConnection,const ::rtl::OUString& sql)
: ::connectivity::odbc::OPreparedStatement( _pConnection,sql)
{
    m_aSelectColumns = _pConnection->createSelectColumns(sql);
}
// -----------------------------------------------------------------------------
OResultSet* OAdabasPreparedStatement::createResulSet()
{
    return new OAdabasResultSet(m_aStatementHandle,this,m_aSelectColumns);
}
// -----------------------------------------------------------------------------
void OAdabasPreparedStatement::setUsingBookmarks(sal_Bool /*_bUseBookmark*/)
{
    ::dbtools::throwFeatureNotImplementedException( "bookmarks", *this );
    // adabas doesn't support bookmarks
}
// -----------------------------------------------------------------------------
void OAdabasPreparedStatement::setResultSetConcurrency(sal_Int32 /*_par0*/)
{
    ::dbtools::throwFeatureNotImplementedException( "PreparedStatement:ResultSetConcurrency", *this );
}
// -----------------------------------------------------------------------------
void OAdabasPreparedStatement::setResultSetType(sal_Int32 /*_par0*/)
{
    ::dbtools::throwFeatureNotImplementedException( "PreparedStatement:ResultSetType", *this );
}
// -----------------------------------------------------------------------------


