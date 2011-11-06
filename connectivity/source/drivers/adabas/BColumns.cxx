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
#include "adabas/BColumns.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include "adabas/BTable.hxx"
#include "adabas/BTables.hxx"
#include "adabas/BCatalog.hxx"
#include <comphelper/types.hxx>
#include "connectivity/dbtools.hxx"
#include <comphelper/property.hxx>

using namespace ::comphelper;


using namespace connectivity::adabas;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OColumns::createObject(const ::rtl::OUString& _rName)
{
    Reference< XResultSet > xResult = m_pTable->getMetaData()->getColumns(Any(),
                                                            m_pTable->getSchema(),m_pTable->getTableName(),_rName);

    sdbcx::ObjectType xRet = NULL;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
        {
            if(xRow->getString(4) == _rName)
            {
                sal_Int32 nType             = xRow->getInt(5);
                ::rtl::OUString sTypeName   = xRow->getString(6);
                sal_Int32 nPrec             = xRow->getInt(7);
                OAdabasCatalog::correctColumnProperties(nPrec,nType,sTypeName);
                sal_Bool bAutoIncrement = sal_False;
                if ( !_rName.equalsAscii("DEFAULT") && !m_pTable->getSchema().equalsAscii("DOMAIN") && !m_pTable->getTableName().equalsAscii("COLUMNS") )
                {
                    Reference< XStatement > xStmt = m_pTable->getMetaData()->getConnection()->createStatement(  );
                    ::rtl::OUString sQuery(RTL_CONSTASCII_USTRINGPARAM("SELECT \"DEFAULT\" FROM DOMAIN.COLUMNS WHERE OWNER = '"));
                    sQuery += m_pTable->getSchema();
                    sQuery += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("' AND TABLENAME = '"));
                    sQuery += m_pTable->getTableName() + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("' AND COLUMNNAME = '"));
                    sQuery += _rName + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("'"));
                    try
                    {
                        Reference< XResultSet > xResult2 = xStmt->executeQuery(sQuery);
                        Reference< XRow > xRow2(xResult2,UNO_QUERY);
                        if ( xRow2.is() && xResult2->next() )
                            bAutoIncrement = xRow2->getString(1) == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DEFAULT STAMP"));
                    }
                    catch(const Exception&)
                    {
                    }
                }

                xRet = new OColumn(_rName,
                                            sTypeName,
                                            xRow->getString(13),
                                            xRow->getString(12),
                                            xRow->getInt(11),
                                            nPrec,
                                            xRow->getInt(9),
                                            nType,
                                            bAutoIncrement,sal_False,sal_False,sal_True);
                break;
            }
        }
        ::comphelper::disposeComponent(xResult);
    }

    return xRet;
}

// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshColumns();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumns::createDescriptor()
{
    return new OColumn(sal_True);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OColumns::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if ( m_pTable->isNew() )
        return cloneDescriptor( descriptor );

    ::rtl::OUString aSql(RTL_CONSTASCII_USTRINGPARAM("ALTER TABLE "));
    ::rtl::OUString sQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

    m_pTable->beginTransAction();
    try
    {
        aSql += ::dbtools::quoteName(sQuote,m_pTable->getSchema()) + sDot + ::dbtools::quoteName(sQuote,m_pTable->getTableName());
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ADD ("));
        aSql += ::dbtools::quoteName(sQuote,_rForName);
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));
        aSql += OTables::getColumnSqlType(descriptor);
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" )"));

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement();
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);

        m_pTable->alterNotNullValue(getINT32(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))),_rForName);
    }
    catch(const Exception&)
    {
        m_pTable->rollbackTransAction();
        throw;
    }
    m_pTable->endTransAction();

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OColumns::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    OSL_ENSURE(m_pTable,"OColumns::dropByName: Table is null!");
    if(!m_pTable->isNew())
    {
        ::rtl::OUString aSql(RTL_CONSTASCII_USTRINGPARAM("ALTER TABLE "));
        ::rtl::OUString sQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

        aSql += ::dbtools::quoteName(sQuote,m_pTable->getSchema()) + sDot + ::dbtools::quoteName(sQuote,m_pTable->getTableName());
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DROP "));
        aSql += ::dbtools::quoteName(sQuote,_sElementName);

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------



