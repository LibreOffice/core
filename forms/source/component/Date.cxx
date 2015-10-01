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

#include "Date.hxx"
#include "services.hxx"
#include <tools/date.hxx>
#include <connectivity/dbconversion.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <comphelper/processfactory.hxx>

using namespace dbtools;

namespace frm
{


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;


ODateControl::ODateControl(const Reference<XComponentContext>& _rxFactory)
               :OBoundControl(_rxFactory, VCL_CONTROL_DATEFIELD)
{
}


Sequence<Type> ODateControl::_getTypes()
{
    return OBoundControl::_getTypes();
}


css::uno::Sequence<OUString> SAL_CALL ODateControl::getSupportedServiceNames() throw(std::exception)
{
    css::uno::Sequence<OUString> aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_DATEFIELD;
    pArray[aSupported.getLength()-2] = STARDIV_ONE_FORM_CONTROL_DATEFIELD;
    return aSupported;
}


Sequence<Type> ODateModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}


ODateModel::ODateModel(const Reference<XComponentContext>& _rxFactory)
    : OEditBaseModel(_rxFactory, VCL_CONTROLMODEL_DATEFIELD,
        FRM_SUN_CONTROL_DATEFIELD, true, true)
    // use the old control name for compytibility reasons
    , OLimitedFormats(_rxFactory, FormComponentType::DATEFIELD)
    , m_bDateTimeField(false)
{
    m_nClassId = FormComponentType::DATEFIELD;
    initValueProperty( PROPERTY_DATE, PROPERTY_ID_DATE );

    setAggregateSet(m_xAggregateFastSet, getOriginalHandle(PROPERTY_ID_DATEFORMAT));

    osl_atomic_increment( &m_refCount );
    try
    {
        if ( m_xAggregateSet.is() )
            m_xAggregateSet->setPropertyValue( PROPERTY_DATEMIN, makeAny(util::Date(1, 1, 1800)) );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "ODateModel::ODateModel: caught an exception!" );
    }
    osl_atomic_decrement( &m_refCount );
}


ODateModel::ODateModel( const ODateModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    : OEditBaseModel(_pOriginal, _rxFactory)
    , OLimitedFormats(_rxFactory, FormComponentType::DATEFIELD)
    , m_bDateTimeField(false)
{
    setAggregateSet( m_xAggregateFastSet, getOriginalHandle( PROPERTY_ID_DATEFORMAT ) );
}


ODateModel::~ODateModel( )
{
    setAggregateSet(Reference< XFastPropertySet >(), -1);
}

// XCloneable

IMPLEMENT_DEFAULT_CLONING( ODateModel )

// XServiceInfo

css::uno::Sequence<OUString> SAL_CALL ODateModel::getSupportedServiceNames() throw(std::exception)
{
    css::uno::Sequence<OUString> aSupported = OBoundControlModel::getSupportedServiceNames();

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 9 );
    OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_DATEFIELD;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_DATEFIELD;
    *pStoreTo++ = BINDABLE_DATABASE_DATE_FIELD;

    *pStoreTo++ = FRM_COMPONENT_DATEFIELD;

    return aSupported;
}


OUString SAL_CALL ODateModel::getServiceName() throw ( css::uno::RuntimeException, std::exception)
{
    return OUString(FRM_COMPONENT_DATEFIELD); // old (non-sun) name for compatibility !
}

// XPropertySet

void ODateModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 4, OEditBaseModel )
        DECL_PROP3(DEFAULT_DATE,            util::Date,             BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(TABINDEX,                sal_Int16,              BOUND);
        DECL_PROP1(FORMATKEY,               sal_Int32,              TRANSIENT);
        DECL_IFACE_PROP2(FORMATSSUPPLIER,   XNumberFormatsSupplier, READONLY, TRANSIENT);
    END_DESCRIBE_PROPERTIES();
}


void SAL_CALL ODateModel::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle ) const
{
    switch (_nHandle)
    {
        case PROPERTY_ID_FORMATKEY:
            getFormatKeyPropertyValue(_rValue);
            break;
        case PROPERTY_ID_FORMATSSUPPLIER:
            _rValue <<= getFormatsSupplier();
            break;
        default:
            OEditBaseModel::getFastPropertyValue(_rValue, _nHandle);
            break;
    }
}


sal_Bool SAL_CALL ODateModel::convertFastPropertyValue(Any& _rConvertedValue, Any& _rOldValue,
        sal_Int32 _nHandle, const Any& _rValue ) throw(IllegalArgumentException)
{
    if (PROPERTY_ID_FORMATKEY == _nHandle)
        return convertFormatKeyPropertyValue(_rConvertedValue, _rOldValue, _rValue);
    else
        return OEditBaseModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue );
}


void SAL_CALL ODateModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw ( css::uno::Exception, std::exception)
{
    if (PROPERTY_ID_FORMATKEY == _nHandle)
        setFormatKeyPropertyValue(_rValue);
    else
        OEditBaseModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
}

// XLoadListener

void ODateModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    OBoundControlModel::onConnectedDbColumn( _rxForm );
    Reference<XPropertySet> xField = getField();
    if (xField.is())
    {
        m_bDateTimeField = false;
        try
        {
            sal_Int32 nFieldType = 0;
            xField->getPropertyValue(PROPERTY_FIELDTYPE) >>= nFieldType;
            m_bDateTimeField = (nFieldType == DataType::TIMESTAMP);
        }
        catch(const Exception&)
        {
        }
    }
}


bool ODateModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    Any aControlValue( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) );
    if ( aControlValue != m_aSaveValue )
    {
        if ( !aControlValue.hasValue() )
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                util::Date aDate;
                if ( !( aControlValue >>= aDate ) )
                {
                    sal_Int32 nAsInt(0);
                    aControlValue >>= nAsInt;
                    aDate = DBTypeConversion::toDate(nAsInt);
                }

                if ( !m_bDateTimeField )
                    m_xColumnUpdate->updateDate( aDate );
                else
                {
                    util::DateTime aDateTime = m_xColumn->getTimestamp();
                    aDateTime.Day = aDate.Day;
                    aDateTime.Month = aDate.Month;
                    aDateTime.Year = aDate.Year;
                    m_xColumnUpdate->updateTimestamp( aDateTime );
                }
            }
            catch(const Exception&)
            {
                return false;
            }
        }
        m_aSaveValue = aControlValue;
    }
    return true;
}


Any ODateModel::translateControlValueToExternalValue( ) const
{
    return getControlValue();
}


Any ODateModel::translateExternalValueToControlValue( const Any& _rExternalValue ) const
{
    return _rExternalValue;
}


Any ODateModel::translateControlValueToValidatableValue( ) const
{
    return getControlValue();
}


Any ODateModel::translateDbColumnToControlValue()
{
    util::Date aDate = m_xColumn->getDate();
    if (m_xColumn->wasNull())
        m_aSaveValue.clear();
    else
        m_aSaveValue <<= aDate;

    return m_aSaveValue;
}


Any ODateModel::getDefaultForReset() const
{
    return m_aDefault;
}


void ODateModel::resetNoBroadcast()
{
    OEditBaseModel::resetNoBroadcast();
    m_aSaveValue.clear();
}


Sequence< Type > ODateModel::getSupportedBindingTypes()
{
    return Sequence< Type >( & cppu::UnoType<util::Date>::get(), 1 );
}

}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_ODateModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::ODateModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_ODateControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::ODateControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
