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
#include "adabas/BKeyColumns.hxx"
#ifndef _CONNECTIVITY_ADABAS_KEYCOLUMN_HXX_
#include "adabas/BKeyColumn.hxx"
#endif
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include "adabas/BTable.hxx"

using namespace connectivity::adabas;
// -------------------------------------------------------------------------
sdbcx::ObjectType OKeyColumns::createObject(const ::rtl::OUString& _rName)
{

    Reference< starsdbc::XResultSet >
        xResult = m_pTable->getConnection()->getMetaData()->getImportedKeys(Any(),
                    m_pTable->getSchema(),m_pTable->getName());

    ::rtl::OUString aRefColumnName;
    if(xResult.is())
    {
        Reference< starsdbc::XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
        {
            if(xRow->getString(8) == _rName)
            {
                aRefColumnName = xRow->getString(4);
                break;
            }
        }
    }

     xResult = m_pTable->getConnection()->getMetaData()->getColumns(Any(),
        m_pTable->getSchema(),m_pTable->getName(),_rName);

    sdbcx::ObjectType xRet = NULL;
    if(xResult.is())
    {
        Reference< starsdbc::XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next())
        {
            if(xRow->getString(4) == _rName)
            {
                OKeyColumn* pRet = new OKeyColumn(aRefColumnName,
                                                    _rName,
                                                    xRow->getString(6),
                                                    xRow->getString(13),
                                                    xRow->getInt(11),
                                                    xRow->getInt(7),
                                                    xRow->getInt(9),
                                                    xRow->getInt(5),
                                                    sal_False);
                xRet = pRet;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeyColumns::createDescriptor()
{
    OKeyColumn* pNew = new OKeyColumn();
    return pNew;
}
// -------------------------------------------------------------------------

