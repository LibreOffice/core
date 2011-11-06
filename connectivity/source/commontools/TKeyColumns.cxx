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
#include "connectivity/TKeyColumns.hxx"
#include "connectivity/sdbcx/VKeyColumn.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/property.hxx>
#include "TConnection.hxx"
#include "connectivity/TTableHelper.hxx"

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
OKeyColumnsHelper::OKeyColumnsHelper(   OTableKeyHelper* _pKey,
                ::osl::Mutex& _rMutex,
                const ::std::vector< ::rtl::OUString> &_rVector)
            : connectivity::sdbcx::OCollection(*_pKey,sal_True,_rMutex,_rVector)
            ,m_pKey(_pKey)
{
}
// -------------------------------------------------------------------------
sdbcx::ObjectType OKeyColumnsHelper::createObject(const ::rtl::OUString& _rName)
{
    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    ::rtl::OUString aSchema,aTable;
    m_pKey->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME))   >>= aSchema;
    m_pKey->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))         >>= aTable;

    // frist get the related column to _rName
    Reference< XResultSet > xResult = m_pKey->getTable()->getMetaData()->getImportedKeys(
            m_pKey->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),aSchema,aTable);

    ::rtl::OUString aRefColumnName;
    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aTemp;
        while(xResult->next())
        {
            aTemp = xRow->getString(4);
            if(xRow->getString(8) == _rName && m_pKey->getName() == xRow->getString(12))
            {
                aRefColumnName = aTemp;
                break;
            }
        }
    }

    sdbcx::ObjectType xRet;

    // now describe the column _rName and set his related column
    xResult = m_pKey->getTable()->getMetaData()->getColumns(
                m_pKey->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),aSchema,aTable,_rName);

    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if ( xResult->next() )
        {
            if ( xRow->getString(4) == _rName )
            {
                sal_Int32 nDataType = xRow->getInt(5);
                ::rtl::OUString aTypeName(xRow->getString(6));
                sal_Int32 nSize = xRow->getInt(7);
                sal_Int32 nDec  = xRow->getInt(9);
                sal_Int32 nNull = xRow->getInt(11);
                ::rtl::OUString sColumnDef;
                try
                {
                    sColumnDef = xRow->getString(13);
                }
                catch(const SQLException&)
                {
                    // somethimes we get an error when asking for this param
                }

                OKeyColumn* pRet = new OKeyColumn(aRefColumnName,
                                                    _rName,
                                                    aTypeName,
                                                    sColumnDef,
                                                    nNull,
                                                    nSize,
                                                    nDec,
                                                    nDataType,
                                                    sal_False,
                                                    sal_False,
                                                    sal_False,
                                                    isCaseSensitive());
                xRet = pRet;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeyColumnsHelper::createDescriptor()
{
    return new OKeyColumn(isCaseSensitive());
}
// -------------------------------------------------------------------------
void OKeyColumnsHelper::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
    m_pKey->refreshColumns();
}
// -----------------------------------------------------------------------------


