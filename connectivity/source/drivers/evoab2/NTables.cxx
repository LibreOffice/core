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
#include "NTables.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <connectivity/sdbcx/VTable.hxx>
#include "NCatalog.hxx"
#ifndef _CONNECTIVITY_EVOAB_BCONNECTION_HXX_
#include "NConnection.hxx"
#endif
#include <comphelper/extract.hxx>
#include "connectivity/dbtools.hxx"
#include "connectivity/dbexception.hxx"
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/types.hxx>
#include "NDebug.hxx"
#include "NTable.hxx"
using namespace ::comphelper;

using namespace ::cppu;
using namespace connectivity::evoab;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

ObjectType OEvoabTables::createObject(const ::rtl::OUString& aName)
{
    ::rtl::OUString aSchema = ::rtl::OUString::createFromAscii("%");

    Sequence< ::rtl::OUString > aTypes(1);
    aTypes[0] = ::rtl::OUString::createFromAscii("TABLE");
    ::rtl::OUString sEmpty;

    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),aSchema,aName,aTypes);

    ObjectType xRet = NULL;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next()) // there can be only one table with this name
        {
            OEvoabTable* pRet = new OEvoabTable(
                    this,
                    (OEvoabConnection *)static_cast<OEvoabCatalog&>(m_rParent).getConnection(),
                    aName,
                    xRow->getString(4),
                    xRow->getString(5),
                    sEmpty);
                    xRet = pRet;
        }
    }

    ::comphelper::disposeComponent(xResult);

    return xRet;
}
// -------------------------------------------------------------------------
void OEvoabTables::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<OEvoabCatalog&>(m_rParent).refreshTables();
}
// -------------------------------------------------------------------------
void OEvoabTables::disposing(void)
{
m_xMetaData.clear();
    OCollection::disposing();
}
// -----------------------------------------------------------------------------

