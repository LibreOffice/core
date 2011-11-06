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

#ifndef _CONNECTIVITY_ADABAS_BDATABASEMETADATA_HXX_
#include "adabas/BDatabaseMetaData.hxx"
#endif
#include "FDatabaseMetaDataResultSet.hxx"

using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

::rtl::OUString SAL_CALL OAdabasDatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue(RTL_CONSTASCII_USTRINGPARAM("sdbc:adabas:"));
    aValue += OAdabasDatabaseMetaData_BASE::getURLImpl();
    return aValue;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OAdabasDatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -----------------------------------------------------------------------------
Reference< XResultSet > OAdabasDatabaseMetaData::impl_getTypeInfo_throw(  )
{
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);
    Reference< XResultSet > xNewRes = pResult;
    static ::connectivity::ODatabaseMetaDataResultSet::ORows aRows;
    if(aRows.empty())
    {
        aRows.reserve(19);
        Reference< XResultSet > xRes = OAdabasDatabaseMetaData_BASE::impl_getTypeInfo_throw();

        if(xRes.is())
        {
            ::connectivity::ODatabaseMetaDataResultSet::ORow aRow(19);

            Reference< XRow> xRow(xRes,UNO_QUERY);
            while(xRes->next())
            {
                sal_Int32 nPos = 1;
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getString    (1));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (2));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (3));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getString    (4));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getString    (5));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getString    (6));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (7));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getBoolean   (8));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getShort     (9));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getBoolean   (10));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (11));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getBoolean   (12));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getString    (13));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getShort     (14));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getShort     (15));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (16));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (17));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator((sal_Int16)xRow->getInt(18));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();

                // we have to fix some incorrect entries
                if(!aRow[2]->getValue().isNull())
                {
                    switch((sal_Int32)aRow[2]->getValue())
                    {
                        case DataType::FLOAT:
//                          aRow[3] = sal_Int32(15);
//                          break;
                        case DataType::REAL:
//                          aRow[3] = sal_Int32(16);
//                          break;
                        case DataType::DOUBLE:
                            aRow[2]->setValue(DataType::DOUBLE);
                            aRow[3]->setValue(sal_Int32(18));
                            break;
                        case DataType::TIMESTAMP:
                            aRow[3]->setValue(sal_Int32(27));
                            break;
                        default:
                            break;
                    }
                }
                aRows.push_back(aRow);
            }
        }
    }
    pResult->setRows(aRows);
    return xNewRes;
}
// -----------------------------------------------------------------------------



