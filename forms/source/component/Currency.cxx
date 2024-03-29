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

#include "Currency.hxx"
#include <property.hxx>
#include <services.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <comphelper/types.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/form/FormComponentType.hpp>


namespace frm
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::util;

OCurrencyControl::OCurrencyControl(const Reference<XComponentContext>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_CURRENCYFIELD)
{
}

css::uno::Sequence<OUString> SAL_CALL OCurrencyControl::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_CONTROL_CURRENCYFIELD;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_CURRENCYFIELD;
    return aSupported;
}


// OCurrencyModel

void OCurrencyModel::implConstruct()
{
    if (!m_xAggregateSet.is())
        return;

    try
    {
        // get the system international information
        const SvtSysLocale aSysLocale;
        const LocaleDataWrapper& aLocaleInfo = aSysLocale.GetLocaleData();

        OUString sCurrencySymbol;
        bool bPrependCurrencySymbol = false;
        switch ( aLocaleInfo.getCurrPositiveFormat() )
        {
            case 0: // $1
                sCurrencySymbol = aLocaleInfo.getCurrSymbol();
                bPrependCurrencySymbol = true;
                break;
            case 1: // 1$
                sCurrencySymbol = aLocaleInfo.getCurrSymbol();
                bPrependCurrencySymbol = false;
                break;
            case 2: // $ 1
                sCurrencySymbol = aLocaleInfo.getCurrSymbol() + " ";
                bPrependCurrencySymbol = true;
                break;
            case 3: // 1 $
                sCurrencySymbol = " " + aLocaleInfo.getCurrSymbol();
                bPrependCurrencySymbol = false;
                break;
        }
        if (!sCurrencySymbol.isEmpty())
        {
            m_xAggregateSet->setPropertyValue(PROPERTY_CURRENCYSYMBOL, Any(sCurrencySymbol));
            m_xAggregateSet->setPropertyValue(PROPERTY_CURRSYM_POSITION, Any(bPrependCurrencySymbol));
        }
    }
    catch(const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "forms.component", "OCurrencyModel::implConstruct: caught an exception while initializing the aggregate!" );
    }
}


OCurrencyModel::OCurrencyModel(const Reference<XComponentContext>& _rxFactory)
    :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_CURRENCYFIELD, FRM_SUN_CONTROL_CURRENCYFIELD, false, true )
    // use the old control name for compatibility reasons
{

    m_nClassId = FormComponentType::CURRENCYFIELD;
    initValueProperty( PROPERTY_VALUE, PROPERTY_ID_VALUE );

    implConstruct();
}


OCurrencyModel::OCurrencyModel( const OCurrencyModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
{
    implConstruct();
}


OCurrencyModel::~OCurrencyModel()
{
}

// XCloneable

css::uno::Reference< css::util::XCloneable > SAL_CALL OCurrencyModel::createClone()
{
    rtl::Reference<OCurrencyModel> pClone = new OCurrencyModel(this, getContext());
    pClone->clonedFrom(this);
    return pClone;
}


// XServiceInfo

css::uno::Sequence<OUString> SAL_CALL OCurrencyModel::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OBoundControlModel::getSupportedServiceNames();

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 5 );
    OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_CURRENCYFIELD;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_CURRENCYFIELD;

    *pStoreTo++ = FRM_COMPONENT_CURRENCYFIELD;

    return aSupported;
}


void OCurrencyModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    OEditBaseModel::describeFixedProperties( _rProps );
    sal_Int32 nOldCount = _rProps.getLength();
    _rProps.realloc( nOldCount + 2);
    css::beans::Property* pProperties = _rProps.getArray() + nOldCount;
    // Set Value to transient
    // ModifyPropertyAttributes(_rAggregateProps, PROPERTY_VALUE, PropertyAttribute::TRANSIENT, 0);

    *pProperties++ = css::beans::Property(PROPERTY_DEFAULT_VALUE, PROPERTY_ID_DEFAULT_VALUE, cppu::UnoType<double>::get(), css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEDEFAULT | css::beans::PropertyAttribute::MAYBEVOID);
    *pProperties++ = css::beans::Property(PROPERTY_TABINDEX, PROPERTY_ID_TABINDEX, cppu::UnoType<sal_Int16>::get(), css::beans::PropertyAttribute::BOUND);
    DBG_ASSERT( pProperties == _rProps.getArray() + _rProps.getLength(), "<...>::describeFixedProperties/getInfoHelper: forgot to adjust the count ?");
}


OUString SAL_CALL OCurrencyModel::getServiceName()
{
    return FRM_COMPONENT_CURRENCYFIELD; // old (non-sun) name for compatibility !
}


bool OCurrencyModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    Any aControlValue( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) );
    if ( aControlValue != m_aSaveValue )
    {
        if ( aControlValue.getValueType().getTypeClass() == TypeClass_VOID )
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                m_xColumnUpdate->updateDouble( getDouble( aControlValue ) );
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


Any OCurrencyModel::translateDbColumnToControlValue()
{
    m_aSaveValue <<= m_xColumn->getDouble();
    if ( m_xColumn->wasNull() )
        m_aSaveValue.clear();
    return m_aSaveValue;
}

// XReset

Any OCurrencyModel::getDefaultForReset() const
{
    Any aValue;
    if ( m_aDefault.getValueType().getTypeClass() == TypeClass_DOUBLE )
        aValue = m_aDefault;

    return aValue;
}


void OCurrencyModel::resetNoBroadcast()
{
    OEditBaseModel::resetNoBroadcast();
    m_aSaveValue.clear();
}


}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OCurrencyModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OCurrencyModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OCurrencyControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OCurrencyControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
