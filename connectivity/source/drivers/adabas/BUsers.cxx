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
#include "adabas/BUsers.hxx"
#include "adabas/BUser.hxx"
#include "adabas/BTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "connectivity/sdbcx/IRefreshable.hxx"
#include <comphelper/types.hxx>
#include "connectivity/dbexception.hxx"
#include "connectivity/dbtools.hxx"
#include "resource/adabas_res.hrc"

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OUsers::createObject(const ::rtl::OUString& _rName)
{
    return new OAdabasUser(m_pConnection,_rName);
}
// -------------------------------------------------------------------------
void OUsers::impl_refresh() throw(RuntimeException)
{
    m_pParent->refreshUsers();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OUsers::createDescriptor()
{
    OUserExtend* pNew = new OUserExtend(m_pConnection);
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OUsers::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE USER ");
    ::rtl::OUString aQuote  = m_pConnection->getMetaData()->getIdentifierQuoteString(  );

    ::rtl::OUString sUserName( _rForName );
    sUserName = sUserName.toAsciiUpperCase();
    descriptor->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME),makeAny(sUserName));
    aSql += ::dbtools::quoteName(aQuote,sUserName)
                + ::rtl::OUString::createFromAscii(" PASSWORD ")
                + getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD)));
    aSql += ::rtl::OUString::createFromAscii(" RESOURCE NOT EXCLUSIVE");

    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    if(xStmt.is())
        xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OUsers::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    {
        // first we have to check if this user is live relevaant for the database
        // which means with out these users the database will miss more than one important system table
        ::rtl::OUString sUsers = ::rtl::OUString::createFromAscii("SELECT USERMODE,USERNAME FROM DOMAIN.USERS WHERE USERNAME = '");
        sUsers += _sElementName + ::rtl::OUString::createFromAscii("'");
        Reference< XStatement > xStmt = m_pConnection->createStatement();
        if(xStmt.is())
        {
            Reference<XResultSet> xRes = xStmt->executeQuery(sUsers);
            Reference<XRow> xRow(xRes,UNO_QUERY);
            if(xRes.is() && xRow.is() && xRes->next()) // there can only be one user with this name
            {
                static const ::rtl::OUString sDbaUser = ::rtl::OUString::createFromAscii("DBA");
                if(xRow->getString(1) == sDbaUser)
                {
                    ::comphelper::disposeComponent(xStmt);
                    m_pConnection->throwGenericSQLException(STR_USER_NO_DELETE,static_cast< XDrop* >( this ));
                }
            }
            ::comphelper::disposeComponent(xStmt);
        }
    }

    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("DROP USER ");
        ::rtl::OUString aQuote  = m_pConnection->getMetaData()->getIdentifierQuoteString(  );
        aSql += ::dbtools::quoteName(aQuote,_sElementName);

        Reference< XStatement > xStmt = m_pConnection->createStatement(  );
        if(xStmt.is())
            xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}

// -------------------------------------------------------------------------
