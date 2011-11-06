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
#include "precompiled_dbaccess.hxx"
#ifndef DBACCESS_INDEXES_HXX_
#include "CIndexes.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_INDEXTYPE_HPP_
#include <com/sun/star/sdbc/IndexType.hpp>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbaccess;
using namespace cppu;


ObjectType OIndexes::createObject(const ::rtl::OUString& _rName)
{
    ObjectType xRet;
    if ( m_xIndexes.is() && m_xIndexes->hasByName(_rName) )
        xRet.set(m_xIndexes->getByName(_rName),UNO_QUERY);
    else
        xRet = OIndexesHelper::createObject(_rName);

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexes::createDescriptor()
{
    Reference<XDataDescriptorFactory> xData( m_xIndexes,UNO_QUERY);
    if(xData.is())
        return xData->createDataDescriptor();
    else
        return OIndexesHelper::createDescriptor();
}
// -------------------------------------------------------------------------
// XAppend
ObjectType OIndexes::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    Reference<XAppend> xData( m_xIndexes,UNO_QUERY);
    if ( !xData.is() )
        return OIndexesHelper::appendObject( _rForName, descriptor );

    xData->appendByDescriptor(descriptor);
    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OIndexes::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    if ( m_xIndexes.is() )
    {
        Reference<XDrop> xData( m_xIndexes,UNO_QUERY);
        if ( xData.is() )
            xData->dropByName(_sElementName);
    }
    else
        OIndexesHelper::dropObject(_nPos,_sElementName);
}
// -------------------------------------------------------------------------
void SAL_CALL OIndexes::disposing(void)
{
    if ( m_xIndexes.is() )
        clear_NoDispose();
    else
        OIndexesHelper::disposing();
}
// -----------------------------------------------------------------------------




