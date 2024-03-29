/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "CRowSetDataColumn.hxx"
#include <stringconstants.hxx>
#include <strings.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <utility>

using namespace dbaccess;
using namespace comphelper;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace cppu;
using namespace osl;


ORowSetDataColumn::ORowSetDataColumn( const Reference < XResultSetMetaData >& _xMetaData,
                                      const Reference < XRow >& _xRow,
                                      const Reference < XRowUpdate >& _xRowUpdate,
                                      sal_Int32 _nPos,
                                      const Reference< XDatabaseMetaData >& _rxDBMeta,
                                      OUString i_sDescription,
                                      OUString i_sLabel,
                                      std::function<const ORowSetValue& (sal_Int32)> _getValue)
    :ODataColumn(_xMetaData,_xRow,_xRowUpdate,_nPos,_rxDBMeta)
    ,m_pGetValue(std::move(_getValue))
    ,m_sLabel(std::move(i_sLabel))
    ,m_aDescription(std::move(i_sDescription))
{
    OColumnSettings::registerProperties( *this );
    registerProperty( PROPERTY_DESCRIPTION, PROPERTY_ID_DESCRIPTION, PropertyAttribute::READONLY, &m_aDescription, cppu::UnoType<decltype(m_aDescription)>::get() );
}

ORowSetDataColumn::~ORowSetDataColumn()
{
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* ORowSetDataColumn::createArrayHelper( ) const
{
    css::uno::Sequence< css::beans::Property> aDescriptor
    {
        { PROPERTY_CATALOGNAME, PROPERTY_ID_CATALOGNAME, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_DISPLAYSIZE, PROPERTY_ID_DISPLAYSIZE, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_ISAUTOINCREMENT, PROPERTY_ID_ISAUTOINCREMENT, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_ISCASESENSITIVE, PROPERTY_ID_ISCASESENSITIVE, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_ISCURRENCY, PROPERTY_ID_ISCURRENCY, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_ISDEFINITELYWRITABLE, PROPERTY_ID_ISDEFINITELYWRITABLE, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_ISNULLABLE, PROPERTY_ID_ISNULLABLE, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_ISREADONLY, PROPERTY_ID_ISREADONLY, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::BOUND },
        { PROPERTY_ISROWVERSION, PROPERTY_ID_ISROWVERSION, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_ISSEARCHABLE, PROPERTY_ID_ISSEARCHABLE, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_ISSIGNED, PROPERTY_ID_ISSIGNED, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_ISWRITABLE, PROPERTY_ID_ISWRITABLE, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_LABEL, PROPERTY_ID_LABEL, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_PRECISION, PROPERTY_ID_PRECISION, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_SCALE, PROPERTY_ID_SCALE, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_SCHEMANAME, PROPERTY_ID_SCHEMANAME, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_SERVICENAME, PROPERTY_ID_SERVICENAME, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_TABLENAME, PROPERTY_ID_TABLENAME, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_TYPE, PROPERTY_ID_TYPE, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_TYPENAME, PROPERTY_ID_TYPENAME, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY },
        { PROPERTY_VALUE, PROPERTY_ID_VALUE, cppu::UnoType<Any>::get(), css::beans::PropertyAttribute::BOUND }
    };

    Sequence< Property > aRegisteredProperties;
    describeProperties( aRegisteredProperties );

    return new ::cppu::OPropertyArrayHelper( ::comphelper::concatSequences( aDescriptor, aRegisteredProperties ), false );
}

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& ORowSetDataColumn::getInfoHelper()
{
    return *static_cast< ::comphelper::OPropertyArrayUsageHelper< ORowSetDataColumn >* >(this)->getArrayHelper();
}

void SAL_CALL ORowSetDataColumn::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    if ( PROPERTY_ID_VALUE == nHandle )
    {
        try
        {
            rValue = m_pGetValue(m_nPos).makeAny();
        }
        catch(const SQLException &e)
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw WrappedTargetRuntimeException("Could not retrieve column value: " + e.Message,
                                                *const_cast<ORowSetDataColumn*>(this),
                                                anyEx);
        }
    }
    else if ( PROPERTY_ID_LABEL == nHandle && !m_sLabel.isEmpty() )
        rValue <<= m_sLabel;
    else
        ODataColumn::getFastPropertyValue( rValue, nHandle );
}

void SAL_CALL ORowSetDataColumn::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue )
{
    switch( nHandle )
    {
        case PROPERTY_ID_VALUE:
            updateObject(rValue);
            break;
        case PROPERTY_ID_ISREADONLY:
            {
                bool bVal = false;
                rValue >>= bVal;
                m_isReadOnly = bVal;
            }
            break;
        default:
            ODataColumn::setFastPropertyValue_NoBroadcast( nHandle,rValue );
            break;
    }
}

sal_Bool SAL_CALL ORowSetDataColumn::convertFastPropertyValue( Any & rConvertedValue,
                                                            Any & rOldValue,
                                                            sal_Int32 nHandle,
                                                            const Any& rValue )
{
    bool bModified = false;
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

Sequence< sal_Int8 > ORowSetDataColumn::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void ORowSetDataColumn::fireValueChange(const ORowSetValue& _rOldValue)
{
    const ORowSetValue &value(m_pGetValue(m_nPos));
    if ( value != _rOldValue)
    {
        sal_Int32 nHandle(PROPERTY_ID_VALUE);
        m_aOldValue = _rOldValue.makeAny();
        Any aNew = value.makeAny();

        fire(&nHandle, &aNew, &m_aOldValue, 1, false );
    }
}

ORowSetDataColumns::ORowSetDataColumns(
                bool _bCase,
                ::rtl::Reference< ::connectivity::OSQLColumns> _xColumns,
                ::cppu::OWeakObject& _rParent,
                ::osl::Mutex& _rMutex,
                const std::vector< OUString> &_rVector
                ) : connectivity::sdbcx::OCollection(_rParent,_bCase,_rMutex,_rVector)
                ,m_aColumns(std::move(_xColumns))
{
}

ORowSetDataColumns::~ORowSetDataColumns()
{
}

sdbcx::ObjectType ORowSetDataColumns::createObject(const OUString& _rName)
{
    connectivity::sdbcx::ObjectType xNamed;

    ::comphelper::UStringMixEqual aCase(isCaseSensitive());
    ::connectivity::OSQLColumns::Vector::const_iterator first =  ::connectivity::find(m_aColumns->begin(),m_aColumns->end(),_rName,aCase);
    if(first != m_aColumns->end())
        xNamed = *first;

    return xNamed;
}

void ORowSetDataColumns::disposing()
{
    ORowSetDataColumns_BASE::disposing();
    m_aColumns = nullptr;
}

void ORowSetDataColumns::assign(const ::rtl::Reference< ::connectivity::OSQLColumns>& _rColumns,const std::vector< OUString> &_rVector)
{
    m_aColumns = _rColumns;
    reFill(_rVector);
}

void ORowSetDataColumns::impl_refresh()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
