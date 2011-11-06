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
#include "ado/ACatalog.hxx"
#ifndef _CONNECTIVITY_ADO_BCONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif
#include "ado/AGroups.hxx"
#include "ado/AUsers.hxx"
#include "ado/ATables.hxx"
#include "ado/AViews.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>


// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::ado;
// -------------------------------------------------------------------------
OCatalog::OCatalog(_ADOCatalog* _pCatalog,OConnection* _pCon) : connectivity::sdbcx::OCatalog(_pCon)
                ,m_pConnection(_pCon)
                ,m_aCatalog(_pCatalog)
{
}
// -----------------------------------------------------------------------------
OCatalog::~OCatalog()
{
    if(m_aCatalog.IsValid())
        m_aCatalog.putref_ActiveConnection(NULL);
    m_aCatalog.clear();
}
// -----------------------------------------------------------------------------
void OCatalog::refreshTables()
{
    TStringVector aVector;

    WpADOTables aTables(m_aCatalog.get_Tables());
  if ( aTables.IsValid() )
  {
    aTables.Refresh();
    sal_Int32 nCount = aTables.GetItemCount();
    aVector.reserve(nCount);
    for(sal_Int32 i=0;i< nCount;++i)
    {
        WpADOTable aElement = aTables.GetItem(i);
          if ( aElement.IsValid() )
          {
              ::rtl::OUString sTypeName = aElement.get_Type();
                  if ( !sTypeName.equalsIgnoreAsciiCaseAscii("SYSTEM TABLE") && !sTypeName.equalsIgnoreAsciiCaseAscii("ACCESS TABLE") )
                     aVector.push_back(aElement.get_Name());
               }
         }
     }

    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = new OTables(this,m_aMutex,aVector,aTables,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------
void OCatalog::refreshViews()
{
    TStringVector aVector;

    WpADOViews aViews = m_aCatalog.get_Views();
    aViews.fillElementNames(aVector);

    if(m_pViews)
        m_pViews->reFill(aVector);
    else
        m_pViews = new OViews(this,m_aMutex,aVector,aViews,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------
void OCatalog::refreshGroups()
{
    TStringVector aVector;

    WpADOGroups aGroups = m_aCatalog.get_Groups();
    aGroups.fillElementNames(aVector);

    if(m_pGroups)
        m_pGroups->reFill(aVector);
    else
        m_pGroups = new OGroups(this,m_aMutex,aVector,aGroups,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------
void OCatalog::refreshUsers()
{
    TStringVector aVector;

    WpADOUsers aUsers = m_aCatalog.get_Users();
    aUsers.fillElementNames(aVector);

    if(m_pUsers)
        m_pUsers->reFill(aVector);
    else
        m_pUsers = new OUsers(this,m_aMutex,aVector,aUsers,m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------


