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

/**************************************************************************
                                TODO
 **************************************************************************

 - This implementation is not a dynamic result set!!! It only implements
   the necessary interfaces, but never recognizes/notifies changes!!!

 *************************************************************************/

#ifndef DBA_DATASUPPLIER_HXX
#include "myucp_datasupplier.hxx"
#endif
#ifndef DBA_UCPRESULTSET_HXX
#include "myucp_resultset.hxx"
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;

using namespace dbaccess;

//=========================================================================
//=========================================================================
//
// DynamicResultSet Implementation.
//
//=========================================================================
//=========================================================================

DynamicResultSet::DynamicResultSet(
                      const Reference< XMultiServiceFactory >& rxSMgr,
                      const rtl::Reference< ODocumentContainer >& rxContent,
                      const OpenCommandArgument2& rCommand,
                      const Reference< XCommandEnvironment >& rxEnv )
    :ResultSetImplHelper( rxSMgr, rCommand )
    ,m_xContent(rxContent)
    ,m_xEnv( rxEnv )
{
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

void DynamicResultSet::initStatic()
{
    m_xResultSet1
        = new ::ucbhelper::ResultSet( m_xSMgr,
                                      m_aCommand.Properties,
                                      new DataSupplier( m_xSMgr,
                                                        m_xContent,
                                                        m_aCommand.Mode ),
                                      m_xEnv );
}

//=========================================================================
void DynamicResultSet::initDynamic()
{
    m_xResultSet1
        = new ::ucbhelper::ResultSet( m_xSMgr,
                                      m_aCommand.Properties,
                                      new DataSupplier( m_xSMgr,
                                                        m_xContent,
                                                        m_aCommand.Mode ),
                                      m_xEnv );
    m_xResultSet2 = m_xResultSet1;
}

