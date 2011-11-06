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
#include "dbase/DColumns.hxx"
#include "dbase/DTable.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/property.hxx>

using namespace connectivity::dbase;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

typedef file::OColumns ODbaseColumns_BASE;
sdbcx::ObjectType ODbaseColumns::createObject(const ::rtl::OUString& _rName)
{

    ODbaseTable* pTable = (ODbaseTable*)m_pTable;

    //  Reference< XFastPropertySet> xCol(pTable->getColumns()[_rName],UNO_QUERY);
    ::vos::ORef<OSQLColumns> aCols = pTable->getTableColumns();
    OSQLColumns::Vector::const_iterator aIter = find(aCols->get().begin(),aCols->get().end(),_rName,::comphelper::UStringMixEqual(isCaseSensitive()));

    sdbcx::ObjectType xRet;
    if(aIter != aCols->get().end())
        xRet = sdbcx::ObjectType(*aIter,UNO_QUERY);
    return xRet;
}

// -------------------------------------------------------------------------
void ODbaseColumns::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshColumns();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > ODbaseColumns::createDescriptor()
{
    return new sdbcx::OColumn(isCaseSensitive());
}
// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType ODbaseColumns::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    if ( m_pTable->isNew() )
        return cloneDescriptor( descriptor );

    m_pTable->addColumn( descriptor );
    return createObject( _rForName );
}
// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------
// XDrop
void ODbaseColumns::dropObject(sal_Int32 _nPos,const ::rtl::OUString /*_sElementName*/)
{
    if(!m_pTable->isNew())
        m_pTable->dropColumn(_nPos);
}
// -----------------------------------------------------------------------------

