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

#ifndef _CONNECTIVITY_ADABAS_GROUP_HXX_
#include "adabas/BGroup.hxx"
#endif
#include "adabas/BUsers.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "adabas/BConnection.hxx"
#include <comphelper/types.hxx>

using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
OAdabasGroup::OAdabasGroup( OAdabasConnection* _pConnection) : connectivity::sdbcx::OGroup(sal_True)
                    ,m_pConnection(_pConnection)
{
    construct();
    TStringVector aVector;
    m_pUsers = new OUsers(*this,m_aMutex,aVector,m_pConnection,this);
}
// -------------------------------------------------------------------------
OAdabasGroup::OAdabasGroup( OAdabasConnection* _pConnection,
                const ::rtl::OUString& _Name
              ) : connectivity::sdbcx::OGroup(_Name,sal_True)
                ,m_pConnection(_pConnection)
{
    construct();
    refreshUsers();
}
// -------------------------------------------------------------------------
void OAdabasGroup::refreshUsers()
{
    if(!m_pConnection)
        return;

    TStringVector aVector;
        Reference< XStatement > xStmt = m_pConnection->createStatement(  );

    ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("SELECT DISTINCT USERNAME FROM DOMAIN.USERS WHERE USERNAME IS NOT NULL AND USERNAME <> ' ' AND USERNAME <> 'CONTROL' AND GROUPNAME = '");
    aSql += getName( );
    aSql += ::rtl::OUString::createFromAscii("'");

    Reference< XResultSet >  xResult = xStmt->executeQuery(aSql);
    if(xResult.is())
    {
                Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(1));
        ::comphelper::disposeComponent(xResult);
    }
    ::comphelper::disposeComponent(xStmt);

    if(m_pUsers)
        m_pUsers->reFill(aVector);
    else
        m_pUsers = new OUsers(*this,m_aMutex,aVector,m_pConnection,this);
}


