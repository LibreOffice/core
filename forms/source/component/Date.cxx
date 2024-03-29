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
#include <property.hxx>
#include <services.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <connectivity/dbconversion.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/form/FormComponentType.hpp>

using namespace dbtools;

namespace frm
{


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::form;


ODateControl::ODateControl(const Reference<XComponentContext>& _rxFactory)
               :OBoundControl(_rxFactory, VCL_CONTROL_DATEFIELD)
{
}


Sequence<Type> ODateControl::_getTypes()
{
    return OBoundControl::_getTypes();
}

css::uno::Sequence<OUString> SAL_CALL ODateControl::getSupportedServiceNames()
{
    const css::uno::Sequence<OUString> vals { FRM_SUN_CONTROL_DATEFIELD, STARDIV_ONE_FORM_CONTROL_DATEFIELD };
    return comphelper::concatSequences(OBoundControl::getSupportedServiceNames(), vals);
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
            m_xAggregateSet->setPropertyValue( PROPERTY_DATEMIN, Any(util::Date(1, 1, 1800)) );
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "forms.component", "ODateModel::ODateModel" );
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

css::uno::Reference< css::util::XCloneable > SAL_CALL ODateModel::createClone()
{
    rtl::Reference<ODateModel> pClone = new ODateModel(this, getContext());
    pClone->clonedFrom(this);
    return pClone;
}

// XServiceInfo

css::uno::Sequence<OUString> SAL_CALL ODateModel::getSupportedServiceNames()
{
    const css::uno::Sequence<OUString> vals {
       BINDABLE_CONTROL_MODEL,
       DATA_AWARE_CONTROL_MODEL,
       VALIDATABLE_CONTROL_MODEL,
       BINDABLE_DATA_AWARE_CONTROL_MODEL,
       VALIDATABLE_BINDABLE_CONTROL_MODEL,
       FRM_SUN_COMPONENT_DATEFIELD,
       FRM_SUN_COMPONENT_DATABASE_DATEFIELD,
       BINDABLE_DATABASE_DATE_FIELD,
       FRM_COMPONENT_DATEFIELD
    };

    return comphelper::concatSequences(OBoundControlModel::getSupportedServiceNames(), vals);
}


OUString SAL_CALL ODateModel::getServiceName()
{
    return FRM_COMPONENT_DATEFIELD; // old (non-sun) name for compatibility !
}

// XPropertySet

void ODateModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    OEditBaseModel::describeFixedProperties( _rProps );
    sal_Int32 nOldCount = _rProps.getLength();
    _rProps.realloc( nOldCount + 4);
    css::beans::Property* pProperties = _rProps.getArray() + nOldCount;
    *pProperties++ = css::beans::Property(PROPERTY_DEFAULT_DATE, PROPERTY_ID_DEFAULT_DATE, cppu::UnoType<util::Date>::get(), css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEDEFAULT | css::beans::PropertyAttribute::MAYBEVOID);
    *pProperties++ = css::beans::Property(PROPERTY_TABINDEX, PROPERTY_ID_TABINDEX, cppu::UnoType<sal_Int16>::get(), css::beans::PropertyAttribute::BOUND);
    *pProperties++ = css::beans::Property(PROPERTY_FORMATKEY, PROPERTY_ID_FORMATKEY, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::TRANSIENT);
    *pProperties++ = css::beans::Property(PROPERTY_FORMATSSUPPLIER, PROPERTY_ID_FORMATSSUPPLIER, cppu::UnoType<XNumberFormatsSupplier>::get(),
                                              css::beans::PropertyAttribute::READONLY | css::beans::PropertyAttribute::TRANSIENT);
    DBG_ASSERT( pProperties == _rProps.getArray() + _rProps.getLength(), "<...>::describeFixedProperties/getInfoHelper: forgot to adjust the count ?");
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
        sal_Int32 _nHandle, const Any& _rValue )
{
    if (PROPERTY_ID_FORMATKEY == _nHandle)
        return convertFormatKeyPropertyValue(_rConvertedValue, _rOldValue, _rValue);
    else
        return OEditBaseModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue );
}


void SAL_CALL ODateModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
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
    if (!xField.is())
        return;

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


bool ODateModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    Any aControlValue( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) );
    if ( aControlValue == m_aSaveValue )
        return true;

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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_ODateModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::ODateModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_ODateControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::ODateControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
