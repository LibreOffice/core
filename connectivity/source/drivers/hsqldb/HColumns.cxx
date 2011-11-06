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
#include "hsqldb/HColumns.hxx"
#include "TConnection.hxx"


using namespace ::comphelper;
using namespace connectivity::hsqldb;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OHSQLColumns::OHSQLColumns( ::cppu::OWeakObject& _rParent
                                ,sal_Bool _bCase
                                ,::osl::Mutex& _rMutex
                                ,const TStringVector &_rVector
                                ,sal_Bool _bUseHardRef
            ) : OColumnsHelper(_rParent,_bCase,_rMutex,_rVector,_bUseHardRef)
{
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OHSQLColumns::createDescriptor()
{
    return new OHSQLColumn(sal_True);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
OHSQLColumn::OHSQLColumn(   sal_Bool    _bCase)
    : connectivity::sdbcx::OColumn( _bCase )
{
    construct();
}
// -------------------------------------------------------------------------
void OHSQLColumn::construct()
{
    m_sAutoIncrement = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IDENTITY"));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_AUTOINCREMENTCREATION),PROPERTY_ID_AUTOINCREMENTCREATION,0,&m_sAutoIncrement, ::getCppuType(&m_sAutoIncrement));
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OHSQLColumn::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & SAL_CALL OHSQLColumn::getInfoHelper()
{
    return *OHSQLColumn_PROP::getArrayHelper(isNew() ? 1 : 0);
}
// -----------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OHSQLColumn::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.Column");

    return aSupported;
}
// -----------------------------------------------------------------------------
