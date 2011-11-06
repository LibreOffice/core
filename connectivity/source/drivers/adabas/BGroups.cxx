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
#include "adabas/BGroups.hxx"
#include "adabas/BGroup.hxx"
#include "adabas/BTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <comphelper/types.hxx>

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
// -------------------------------------------------------------------------
sdbcx::ObjectType OGroups::createObject(const ::rtl::OUString& _rName)
{
    return new OAdabasGroup(m_pConnection,_rName);
}
// -------------------------------------------------------------------------
void OGroups::impl_refresh() throw(RuntimeException)
{
    m_pParent->refreshGroups();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OGroups::createDescriptor()
{
    //  OAdabasGroup* pNew =
    return new OAdabasGroup(m_pConnection);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OGroups::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& /*descriptor*/ )
{
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE USERGROUP ");
    ::rtl::OUString aQuote  = m_pConnection->getMetaData()->getIdentifierQuoteString(  );

    aSql = aSql + aQuote + _rForName + aQuote;

    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OGroups::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("DROP USERGROUP ");
    ::rtl::OUString aQuote  = m_pConnection->getMetaData()->getIdentifierQuoteString(  );

    aSql = aSql + aQuote + _sElementName + aQuote;

    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);
}
// -------------------------------------------------------------------------


