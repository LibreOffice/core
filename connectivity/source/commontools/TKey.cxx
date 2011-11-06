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


#include "connectivity/TKey.hxx"
#include "connectivity/TKeyColumns.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "TConnection.hxx"
#include "connectivity/TTableHelper.hxx"

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
OTableKeyHelper::OTableKeyHelper(OTableHelper* _pTable) : connectivity::sdbcx::OKey(sal_True)
    ,m_pTable(_pTable)
{
    construct();
}
// -------------------------------------------------------------------------
OTableKeyHelper::OTableKeyHelper(   OTableHelper* _pTable
            ,const ::rtl::OUString& _Name
            ,const sdbcx::TKeyProperties& _rProps
            ) : connectivity::sdbcx::OKey(_Name,_rProps,sal_True)
                ,m_pTable(_pTable)
{
    construct();
    refreshColumns();
}
// -------------------------------------------------------------------------
void OTableKeyHelper::refreshColumns()
{
    if ( !m_pTable )
        return;

    ::std::vector< ::rtl::OUString> aVector;
    if ( !isNew() )
    {
        aVector = m_aProps->m_aKeyColumnNames;
        if ( aVector.empty() )
        {
            ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
            ::rtl::OUString aSchema,aTable;
            m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
            m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;

            if ( m_Name.getLength() ) // foreign key
            {

                Reference< XResultSet > xResult = m_pTable->getMetaData()->getImportedKeys(m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
                    aSchema,aTable);

                if ( xResult.is() )
                {
                    Reference< XRow > xRow(xResult,UNO_QUERY);
                    while( xResult->next() )
                    {
                        ::rtl::OUString aForeignKeyColumn = xRow->getString(8);
                        if(xRow->getString(12) == m_Name)
                            aVector.push_back(aForeignKeyColumn);
                    }
                }
            }

            if ( aVector.empty() )
            {
                const Reference< XResultSet > xResult = m_pTable->getMetaData()->getPrimaryKeys(m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
                    aSchema,aTable);

                if ( xResult.is() )
                {
                    const Reference< XRow > xRow(xResult,UNO_QUERY);
                    while( xResult->next() )
                        aVector.push_back(xRow->getString(4));
                } // if ( xResult.is() )
            }
        }
    }


    if ( m_pColumns )
        m_pColumns ->reFill(aVector);
    else
        m_pColumns  = new OKeyColumnsHelper(this,m_aMutex,aVector);
}
// -----------------------------------------------------------------------------

