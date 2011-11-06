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

#include "KColumns.hxx"
#include "KTable.hxx"
#include "KTables.hxx"
#include "KCatalog.hxx"
#include "connectivity/sdbcx/VColumn.hxx"

using namespace connectivity::kab;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
sdbcx::ObjectType KabColumns::createObject(const ::rtl::OUString& _rName)
{
    Reference< XResultSet > xResult = m_pTable->getConnection()->getMetaData()->getColumns(
        Any(),
        m_pTable->getSchema(),
        m_pTable->getTableName(),
        _rName);

    sdbcx::ObjectType xRet = NULL;
    if (xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);

        while (xResult->next())
        {
            if (xRow->getString(4) == _rName)
            {
                OColumn* pRet = new OColumn(
                        _rName,
                        xRow->getString(6),
                        xRow->getString(13),
                        xRow->getString(12),
                        xRow->getInt(11),
                        xRow->getInt(7),
                        xRow->getInt(9),
                        xRow->getInt(5),
                        sal_False,
                        sal_False,
                        sal_False,
                        sal_True);
                xRet = pRet;
                break;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
void KabColumns::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshColumns();
}
// -------------------------------------------------------------------------
KabColumns::KabColumns( KabTable* _pTable,
                        ::osl::Mutex& _rMutex,
                        const TStringVector &_rVector)
    : sdbcx::OCollection(*_pTable, sal_True, _rMutex, _rVector),
      m_pTable(_pTable)
{
}
