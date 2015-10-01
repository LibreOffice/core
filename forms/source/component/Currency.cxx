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
#include "services.hxx"
#include <unotools/localedatawrapper.hxx>
#include <vcl/svapp.hxx>
#include <unotools/syslocale.hxx>
#include <comphelper/processfactory.hxx>


namespace frm
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

OCurrencyControl::OCurrencyControl(const Reference<XComponentContext>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_CURRENCYFIELD)
{
}

Sequence<Type> OCurrencyControl::_getTypes()
{
    return OBoundControl::_getTypes();
}


css::uno::Sequence<OUString> SAL_CALL OCurrencyControl::getSupportedServiceNames() throw(std::exception)
{
    css::uno::Sequence<OUString> aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_CONTROL_CURRENCYFIELD;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_CURRENCYFIELD;
    return aSupported;
}


// OCurrencyModel

Sequence<Type> OCurrencyModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}


void OCurrencyModel::implConstruct()
{
    if (m_xAggregateSet.is())
    {
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
                m_xAggregateSet->setPropertyValue(PROPERTY_CURRENCYSYMBOL, makeAny(sCurrencySymbol));
                m_xAggregateSet->setPropertyValue(PROPERTY_CURRSYM_POSITION, makeAny(bPrependCurrencySymbol));
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL( "OCurrencyModel::implConstruct: caught an exception while initializing the aggregate!" );
        }
    }
}



OCurrencyModel::OCurrencyModel(const Reference<XComponentContext>& _rxFactory)
    :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_CURRENCYFIELD, FRM_SUN_CONTROL_CURRENCYFIELD, false, true )
    // use the old control name for compaytibility reasons
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

IMPLEMENT_DEFAULT_CLONING( OCurrencyModel )

// XServiceInfo

css::uno::Sequence<OUString> SAL_CALL OCurrencyModel::getSupportedServiceNames() throw(std::exception)
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
    BEGIN_DESCRIBE_PROPERTIES( 2, OEditBaseModel )
    // Set Value to transient
    // ModifyPropertyAttributes(_rAggregateProps, PROPERTY_VALUE, PropertyAttribute::TRANSIENT, 0);

        DECL_PROP3(DEFAULT_VALUE,       double,             BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(TABINDEX,        sal_Int16,              BOUND);
    END_DESCRIBE_PROPERTIES();
}


OUString SAL_CALL OCurrencyModel::getServiceName() throw ( css::uno::RuntimeException, std::exception)
{
    return OUString(FRM_COMPONENT_CURRENCYFIELD); // old (non-sun) name for compatibility !
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_OCurrencyModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OCurrencyModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_OCurrencyControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OCurrencyControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
