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

#include "Numeric.hxx"
#include <services.hxx>
#include <property.hxx>
#include <comphelper/types.hxx>
#include <tools/debug.hxx>
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

ONumericControl::ONumericControl(const Reference<XComponentContext>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_NUMERICFIELD)
{
}


css::uno::Sequence<OUString> ONumericControl::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_CONTROL_NUMERICFIELD;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_NUMERICFIELD;
    return aSupported;
}

// ONumericModel

ONumericModel::ONumericModel(const Reference<XComponentContext>& _rxFactory)
                :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_NUMERICFIELD, FRM_SUN_CONTROL_NUMERICFIELD, true, true )
                                    // use the old control name for compatibility reasons
{

    m_nClassId = FormComponentType::NUMERICFIELD;
    initValueProperty( PROPERTY_VALUE, PROPERTY_ID_VALUE );
}


ONumericModel::ONumericModel( const ONumericModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
{
}


ONumericModel::~ONumericModel()
{
}

// XCloneable

css::uno::Reference< css::util::XCloneable > SAL_CALL ONumericModel::createClone()
{
    rtl::Reference<ONumericModel> pClone = new ONumericModel(this, getContext());
    pClone->clonedFrom(this);
    return pClone;
}

// XServiceInfo

css::uno::Sequence<OUString> ONumericModel::getSupportedServiceNames()
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

    *pStoreTo++ = FRM_SUN_COMPONENT_NUMERICFIELD;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_NUMERICFIELD;
    *pStoreTo++ = BINDABLE_DATABASE_NUMERIC_FIELD;

    *pStoreTo++ = FRM_COMPONENT_NUMERICFIELD;

    return aSupported;
}


void ONumericModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    OEditBaseModel::describeFixedProperties( _rProps );
    sal_Int32 nOldCount = _rProps.getLength();
    _rProps.realloc( nOldCount + 2);
    css::beans::Property* pProperties = _rProps.getArray() + nOldCount;
    *pProperties++ = css::beans::Property(PROPERTY_DEFAULT_VALUE, PROPERTY_ID_DEFAULT_VALUE, cppu::UnoType<double>::get(), css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEDEFAULT | css::beans::PropertyAttribute::MAYBEVOID);
    *pProperties++ = css::beans::Property(PROPERTY_TABINDEX, PROPERTY_ID_TABINDEX, cppu::UnoType<sal_Int16>::get(), css::beans::PropertyAttribute::BOUND);
    DBG_ASSERT( pProperties == _rProps.getArray() + _rProps.getLength(), "<...>::describeFixedProperties/getInfoHelper: forgot to adjust the count ?");
}


OUString SAL_CALL ONumericModel::getServiceName()
{
    return FRM_COMPONENT_NUMERICFIELD;  // old (non-sun) name for compatibility !
}


bool ONumericModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
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


Any ONumericModel::translateDbColumnToControlValue()
{
    m_aSaveValue <<= m_xColumn->getDouble();
    if ( m_xColumn->wasNull() )
        m_aSaveValue.clear();

    return m_aSaveValue;
}


Any ONumericModel::getDefaultForReset() const
{
    Any aValue;
    if (m_aDefault.getValueType().getTypeClass() == TypeClass_DOUBLE)
        aValue = m_aDefault;

    return aValue;
}


void ONumericModel::resetNoBroadcast()
{
    OEditBaseModel::resetNoBroadcast();
    m_aSaveValue.clear();
}

}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_ONumericModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::ONumericModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_ONumericControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::ONumericControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
