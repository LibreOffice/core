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

#include "KCatalog.hxx"
#include "KConnection.hxx"
#include "KTables.hxx"

using namespace connectivity::kab;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::cppu;

// -------------------------------------------------------------------------
KabCatalog::KabCatalog(KabConnection* _pCon)
        : connectivity::sdbcx::OCatalog(_pCon),
          m_pConnection(_pCon),
          m_xMetaData(m_pConnection->getMetaData())
{
}
// -------------------------------------------------------------------------
void KabCatalog::refreshTables()
{
    TStringVector aVector;
    Sequence< ::rtl::OUString > aTypes(1);
    aTypes[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%"));
    Reference< XResultSet > xResult = m_xMetaData->getTables(
        Any(),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%")),
        aTypes);

    if (xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aName;
        // const ::rtl::OUString& sDot = KabCatalog::getDot();

        while (xResult->next())
        {
            // aName = xRow->getString(2);
            // aName += sDot;
            aName = xRow->getString(3);
            aVector.push_back(aName);
        }
    }
    if (m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = new KabTables(m_xMetaData,*this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void KabCatalog::refreshViews()
{
}
// -------------------------------------------------------------------------
void KabCatalog::refreshGroups()
{
}
// -------------------------------------------------------------------------
void KabCatalog::refreshUsers()
{
}
// -------------------------------------------------------------------------
const ::rtl::OUString& KabCatalog::getDot()
{
    static const ::rtl::OUString sDot = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("."));
    return sDot;
}
// -----------------------------------------------------------------------------

// XTablesSupplier
Reference< XNameAccess > SAL_CALL KabCatalog::getTables(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    try
    {
        if (!m_pTables)
            refreshTables();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return m_pTables;
}
