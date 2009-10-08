/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CRowSetDataColumn.cxx,v $
 * $Revision: 1.35 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
                                      const ORowSetCacheIterator& _rColumnValue)
    : ODataColumn(_xMetaData,_xRow,_xRowUpdate,_nPos,_rxDBMeta)
    ,m_aColumnValue(_rColumnValue)
    ,m_aDescription(_rDescription)
{
    DBG_CTOR(ORowSetDataColumn,NULL);
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
    BEGIN_PROPERTY_HELPER(31)
        DECL_PROP2(ALIGN,                   sal_Int32,          BOUND,MAYBEVOID);
        DECL_PROP1(CATALOGNAME,             ::rtl::OUString,    READONLY);
        DECL_PROP2(CONTROLDEFAULT,          ::rtl::OUString,    BOUND,MAYBEVOID);
        DECL_PROP1_IFACE(CONTROLMODEL,      XPropertySet,       BOUND       );
        DECL_PROP1(DESCRIPTION,             ::rtl::OUString,    READONLY);
        DECL_PROP1(DISPLAYSIZE,             sal_Int32,          READONLY);
        DECL_PROP2(NUMBERFORMAT,            sal_Int32,          BOUND,MAYBEVOID);
        DECL_PROP2(HELPTEXT,            ::rtl::OUString,    BOUND,MAYBEVOID);
        DECL_PROP1_BOOL(HIDDEN,                             BOUND);
        DECL_PROP1_BOOL(ISAUTOINCREMENT,                        READONLY);
        DECL_PROP1_BOOL(ISCASESENSITIVE,                        READONLY);
        DECL_PROP1_BOOL(ISCURRENCY,                             READONLY);
        DECL_PROP1_BOOL(ISDEFINITELYWRITABLE,                   READONLY);
        DECL_PROP1(ISNULLABLE,              sal_Int32,          READONLY);
        DECL_PROP1_BOOL(ISREADONLY,                             READONLY);
        DECL_PROP1_BOOL(ISROWVERSION,                           READONLY);
        DECL_PROP1_BOOL(ISSEARCHABLE,                           READONLY);
        DECL_PROP1_BOOL(ISSIGNED,                               READONLY);
        DECL_PROP1_BOOL(ISWRITABLE,                             READONLY);
        DECL_PROP1(LABEL,                   ::rtl::OUString,    READONLY);
        DECL_PROP1(NAME,                    ::rtl::OUString,    READONLY);
        DECL_PROP1(PRECISION,               sal_Int32,          READONLY);
        DECL_PROP2(RELATIVEPOSITION,    sal_Int32,          BOUND, MAYBEVOID);
        DECL_PROP1(SCALE,                   sal_Int32,          READONLY);
        DECL_PROP1(SCHEMANAME,              ::rtl::OUString,    READONLY);
        DECL_PROP1(SERVICENAME,             ::rtl::OUString,    READONLY);
        DECL_PROP1(TABLENAME,               ::rtl::OUString,    READONLY);
        DECL_PROP1(TYPE,                    sal_Int32,          READONLY);
        DECL_PROP1(TYPENAME,                ::rtl::OUString,    READONLY);
        DECL_PROP1(VALUE,                   Any,                BOUND);
        DECL_PROP1(WIDTH,                   sal_Int32,          MAYBEVOID);
    END_PROPERTY_HELPER();
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
    switch(nHandle)
    {
        case PROPERTY_ID_DESCRIPTION:
            rValue <<= m_aDescription;
            break;
        case PROPERTY_ID_ALIGN:
        case PROPERTY_ID_NUMBERFORMAT:
        case PROPERTY_ID_RELATIVEPOSITION:
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_HIDDEN:
        case PROPERTY_ID_CONTROLMODEL:
        case PROPERTY_ID_HELPTEXT:
        case PROPERTY_ID_CONTROLDEFAULT:
            OColumnSettings::getFastPropertyValue( rValue, nHandle );
            break;
        case PROPERTY_ID_VALUE:
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
            break;
        default:
            ODataColumn::getFastPropertyValue(rValue,nHandle);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetDataColumn::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue )throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ALIGN:
        case PROPERTY_ID_NUMBERFORMAT:
        case PROPERTY_ID_RELATIVEPOSITION:
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_HIDDEN:
        case PROPERTY_ID_CONTROLMODEL:
        case PROPERTY_ID_HELPTEXT:
        case PROPERTY_ID_CONTROLDEFAULT:
            OColumnSettings::setFastPropertyValue_NoBroadcast( nHandle, rValue );
            break;
        case PROPERTY_ID_VALUE:
            updateObject(rValue);
            break;
        default:
            ODataColumn::setFastPropertyValue_NoBroadcast(nHandle,rValue );
    }
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetDataColumn::convertFastPropertyValue( Any & rConvertedValue,
                                                            Any & rOldValue,
                                                            sal_Int32 nHandle,
                                                            const Any& rValue ) throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch(nHandle)
    {
        case PROPERTY_ID_ALIGN:
        case PROPERTY_ID_NUMBERFORMAT:
        case PROPERTY_ID_RELATIVEPOSITION:
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_HIDDEN:
        case PROPERTY_ID_CONTROLMODEL:
        case PROPERTY_ID_HELPTEXT:
        case PROPERTY_ID_CONTROLDEFAULT:
            bModified = OColumnSettings::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
            break;
        case PROPERTY_ID_VALUE:
            rConvertedValue = rValue;
            getFastPropertyValue(rOldValue, PROPERTY_ID_VALUE);
            bModified = !::comphelper::compare(rConvertedValue, rOldValue);
            break;
        default:
            bModified = ODataColumn::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);

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


