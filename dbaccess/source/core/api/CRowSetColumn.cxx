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

#include "dbastrings.hrc"
#include "apitools.hxx"
#include "CRowSetColumn.hxx"

#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <comphelper/types.hxx>
#include <cppuhelper/typeprovider.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;

//..............................................................................
namespace dbaccess
{
//..............................................................................

//------------------------------------------------------------------------------
ORowSetColumn::ORowSetColumn(   const Reference < XResultSetMetaData >& _xMetaData, const Reference < XRow >& _xRow, sal_Int32 _nPos,
                const Reference< XDatabaseMetaData >& _rxDBMeta, const ::rtl::OUString& _rDescription, const ::rtl::OUString& i_sLabel,ORowSetCacheIterator& _rColumnValue )
    :ORowSetDataColumn( _xMetaData, _xRow, NULL, _nPos, _rxDBMeta, _rDescription, i_sLabel,_rColumnValue )
{
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ORowSetColumn::createArrayHelper( ) const
{
    const sal_Int32 nDerivedProperties = 21;
    Sequence< Property> aDerivedProperties( nDerivedProperties );
    Property* pDesc = aDerivedProperties.getArray();
    sal_Int32 nPos = 0;

    DECL_PROP1( CATALOGNAME,                ::rtl::OUString,    READONLY );
    DECL_PROP1( DISPLAYSIZE,                sal_Int32,          READONLY );
    DECL_PROP1_BOOL( ISAUTOINCREMENT,                           READONLY );
    DECL_PROP1_BOOL( ISCASESENSITIVE,                           READONLY );
    DECL_PROP1_BOOL( ISCURRENCY,                                READONLY );
    DECL_PROP1_BOOL( ISDEFINITELYWRITABLE,                      READONLY );
    DECL_PROP1( ISNULLABLE,                 sal_Int32,          READONLY );
    DECL_PROP1_BOOL( ISREADONLY,                                BOUND );
    DECL_PROP1_BOOL( ISROWVERSION,                              READONLY );
    DECL_PROP1_BOOL( ISSEARCHABLE,                              READONLY );
    DECL_PROP1_BOOL( ISSIGNED,                                  READONLY );
    DECL_PROP1_BOOL( ISWRITABLE,                                READONLY );
    DECL_PROP1( LABEL,                      ::rtl::OUString,    READONLY );
    DECL_PROP1( PRECISION,                  sal_Int32,          READONLY );
    DECL_PROP1( SCALE,                      sal_Int32,          READONLY );
    DECL_PROP1( SCHEMANAME,                 ::rtl::OUString,    READONLY );
    DECL_PROP1( SERVICENAME,                ::rtl::OUString,    READONLY );
    DECL_PROP1( TABLENAME,                  ::rtl::OUString,    READONLY );
    DECL_PROP1( TYPE,                       sal_Int32,          READONLY );
    DECL_PROP1( TYPENAME,                   ::rtl::OUString,    READONLY );
    DECL_PROP2( VALUE,                      Any,                READONLY, BOUND );
    OSL_ENSURE( nPos == nDerivedProperties, "ORowSetColumn::createArrayHelper: inconsistency!" );

    Sequence< Property > aRegisteredProperties;
    describeProperties( aRegisteredProperties );

    return new ::cppu::OPropertyArrayHelper( ::comphelper::concatSequences( aDerivedProperties, aRegisteredProperties ), sal_False );
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ORowSetColumn::getInfoHelper()
{
    return *static_cast< ::comphelper::OPropertyArrayUsageHelper< ORowSetColumn >* >(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void SAL_CALL ORowSetColumn::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue )throw (Exception)
{
    OSL_ENSURE( nHandle != PROPERTY_ID_VALUE, "ORowSetColumn::setFastPropertyValue_NoBroadcast: hmm? This property is marked as READONLY!" );
    if ( nHandle != PROPERTY_ID_VALUE )
        ORowSetDataColumn::setFastPropertyValue_NoBroadcast( nHandle, rValue );
}

//..............................................................................
}   // namespace dbaccess
//..............................................................................
