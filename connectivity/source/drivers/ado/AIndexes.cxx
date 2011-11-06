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
#include "ado/AIndexes.hxx"
#include "ado/AIndex.hxx"
#include "ado/AConnection.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include "resource/ado_res.hrc"
using namespace ::comphelper;


using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

sdbcx::ObjectType OIndexes::createObject(const ::rtl::OUString& _rName)
{
    return new OAdoIndex(isCaseSensitive(),m_pConnection,m_aCollection.GetItem(_rName));
}
// -------------------------------------------------------------------------
void OIndexes::impl_refresh() throw(RuntimeException)
{
    m_aCollection.Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexes::createDescriptor()
{
    return new OAdoIndex(isCaseSensitive(),m_pConnection);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OIndexes::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    OAdoIndex* pIndex = NULL;
    if ( !getImplementation(pIndex,descriptor) || pIndex == NULL )
        m_pConnection->throwGenericSQLException( STR_INVALID_INDEX_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );

    ADOIndexes* pIndexes = m_aCollection;
    if ( FAILED( pIndexes->Append( OLEVariant( _rForName ), OLEVariant( pIndex->getImpl() ) ) ) )
    {
        ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));
        m_pConnection->throwGenericSQLException( STR_INVALID_INDEX_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );
    }

    return new OAdoIndex(isCaseSensitive(),m_pConnection,pIndex->getImpl());
}
// -------------------------------------------------------------------------
// XDrop
void OIndexes::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    m_aCollection.Delete(_sElementName);
}
// -----------------------------------------------------------------------------


