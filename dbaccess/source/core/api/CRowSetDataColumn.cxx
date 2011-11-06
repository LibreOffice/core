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

#ifndef _DBACORE_DATACOLUMN_HXX_
#include "CRowSetDataColumn.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace dbaccess;
using namespace comphelper;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace cppu;
using namespace osl;

DBG_NAME(ORowSetDataColumn)
// -------------------------------------------------------------------------
ORowSetDataColumn::ORowSetDataColumn(   const Reference < XResultSetMetaData >& _xMetaData,
                                      const Reference < XRow >& _xRow,
                                      const Reference < XRowUpdate >& _xRowUpdate,
                                      sal_Int32 _nPos,
                                      const Reference< XDatabaseMetaData >& _rxDBMeta,
                                      const ::rtl::OUString& _rDescription,
                                      const ::rtl::OUString& i_sLabel,
                                      const ORowSetCacheIterator& _rColumnValue)
    :ODataColumn(_xMetaData,_xRow,_xRowUpdate,_nPos,_rxDBMeta)
    ,m_aColumnValue(_rColumnValue)
    ,m_sLabel(i_sLabel)
    ,m_aDescription(_rDescription)
{
    DBG_CTOR(ORowSetDataColumn,NULL);
    OColumnSettings::registerProperties( *this );
    registerProperty( PROPERTY_DESCRIPTION, PROPERTY_ID_DESCRIPTION, PropertyAttribute::READONLY, &m_aDescription, ::getCppuType( &m_aDescription ) );
}
// -------------------------------------------------------------------------
ORowSetDataColumn::~ORowSetDataColumn()
{
    DBG_DTOR(ORowSetDataColumn,NULL);
}
// -------------------------------------------------------------------------
// comphelper::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ORowSetDataColumn::createArrayHelper( ) const
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
    DECL_PROP1( VALUE,                      Any,                BOUND );
    OSL_ENSURE( nPos == nDerivedProperties, "ORowSetDataColumn::createArrayHelper: inconsistency!" );

    Sequence< Property > aRegisteredProperties;
    describeProperties( aRegisteredProperties );

    return new ::cppu::OPropertyArrayHelper( ::comphelper::concatSequences( aDerivedProperties, aRegisteredProperties ), sal_False );
}

// cppu::OPropertySetHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ORowSetDataColumn::getInfoHelper()
{
    return *static_cast< ::comphelper::OPropertyArrayUsageHelper< ORowSetDataColumn >* >(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetDataColumn::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    if ( PROPERTY_ID_VALUE == nHandle )
    {
        if ( !m_aColumnValue.isNull() && m_aColumnValue->isValid() )
        {
            ::osl::Mutex* pMutex = m_aColumnValue.getMutex();
            ::osl::MutexGuard aGuard( *pMutex );
#if OSL_DEBUG_LEVEL > 0
            ORowSetRow aRow = *m_aColumnValue;
#endif
            OSL_ENSURE((sal_Int32)aRow->get().size() > m_nPos,"Pos is greater than size of vector");
            rValue = ((*m_aColumnValue)->get())[m_nPos].makeAny();
        }
    }
    else if ( PROPERTY_ID_LABEL == nHandle && m_sLabel.getLength() )
        rValue <<= m_sLabel;
    else
        ODataColumn::getFastPropertyValue( rValue, nHandle );
}

// -------------------------------------------------------------------------
void SAL_CALL ORowSetDataColumn::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue )throw (Exception)
{
    switch( nHandle )
    {
        case PROPERTY_ID_VALUE:
            updateObject(rValue);
            break;
        case PROPERTY_ID_ISREADONLY:
            {
                sal_Bool bVal = sal_False;
                rValue >>= bVal;
                m_isReadOnly.reset(bVal);
            }
            break;
        default:
            ODataColumn::setFastPropertyValue_NoBroadcast( nHandle,rValue );
            break;
    }
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetDataColumn::convertFastPropertyValue( Any & rConvertedValue,
                                                            Any & rOldValue,
                                                            sal_Int32 nHandle,
                                                            const Any& rValue ) throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch( nHandle )
    {
        case PROPERTY_ID_VALUE:
            {
                rConvertedValue = rValue;
                getFastPropertyValue(rOldValue, PROPERTY_ID_VALUE);
                bModified = rConvertedValue != rOldValue;
            }
            break;
        case PROPERTY_ID_ISREADONLY:
            {
                rConvertedValue = rValue;
                getFastPropertyValue(rOldValue, PROPERTY_ID_ISREADONLY);
                bModified = rConvertedValue != rOldValue;
            }
            break;
        default:
            bModified = ODataColumn::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);
            break;
    }

    return bModified;
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > ORowSetDataColumn::getImplementationId() throw (RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}
// -------------------------------------------------------------------------
void ORowSetDataColumn::fireValueChange(const ORowSetValue& _rOldValue)
{
    if ( !m_aColumnValue.isNull() && m_aColumnValue->isValid() && (!(((*m_aColumnValue)->get())[m_nPos] == _rOldValue)) )
    {
        sal_Int32 nHandle = PROPERTY_ID_VALUE;
        m_aOldValue = _rOldValue.makeAny();
        Any aNew = ((*m_aColumnValue)->get())[m_nPos].makeAny();

        fire(&nHandle, &aNew, &m_aOldValue, 1, sal_False );
    }
    else if ( !m_aColumnValue.isNull() && !_rOldValue.isNull() )
    {
        sal_Int32 nHandle = PROPERTY_ID_VALUE;
        m_aOldValue = _rOldValue.makeAny();
        Any aNew;

        fire(&nHandle, &aNew, &m_aOldValue, 1, sal_False );
    }
}
// -----------------------------------------------------------------------------
DBG_NAME(ORowSetDataColumns )
ORowSetDataColumns::ORowSetDataColumns(
                sal_Bool _bCase,
                const ::vos::ORef< ::connectivity::OSQLColumns>& _rColumns,
                ::cppu::OWeakObject& _rParent,
                ::osl::Mutex& _rMutex,
                const ::std::vector< ::rtl::OUString> &_rVector
                ) : connectivity::sdbcx::OCollection(_rParent,_bCase,_rMutex,_rVector)
                ,m_aColumns(_rColumns)
{
    DBG_CTOR(ORowSetDataColumns ,NULL);
}
// -----------------------------------------------------------------------------
ORowSetDataColumns::~ORowSetDataColumns()
{
    DBG_DTOR(ORowSetDataColumns ,NULL);
}
// -----------------------------------------------------------------------------
sdbcx::ObjectType ORowSetDataColumns::createObject(const ::rtl::OUString& _rName)
{
    connectivity::sdbcx::ObjectType xNamed;

    ::comphelper::UStringMixEqual aCase(isCaseSensitive());
    ::connectivity::OSQLColumns::Vector::const_iterator first =  ::connectivity::find(m_aColumns->get().begin(),m_aColumns->get().end(),_rName,aCase);
    if(first != m_aColumns->get().end())
        xNamed.set(*first,UNO_QUERY);

    return xNamed;
}
// -----------------------------------------------------------------------------
void SAL_CALL ORowSetDataColumns::disposing(void)
{
    //  clear_NoDispose();
    ORowSetDataColumns_BASE::disposing();
    m_aColumns = NULL;
    //  m_aColumns.clear();
}
// -----------------------------------------------------------------------------
void ORowSetDataColumns::assign(const ::vos::ORef< ::connectivity::OSQLColumns>& _rColumns,const ::std::vector< ::rtl::OUString> &_rVector)
{
    m_aColumns = _rColumns;
    reFill(_rVector);
}
// -----------------------------------------------------------------------------
void ORowSetDataColumns::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
}
// -----------------------------------------------------------------------------


