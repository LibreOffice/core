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

#include "Pattern.hxx"
#include "comphelper/processfactory.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::sdbc::XRowSet;
using ::com::sun::star::uno::UNO_QUERY;

namespace FormComponentType = ::com::sun::star::form::FormComponentType;

namespace frm
{

OPatternControl::OPatternControl(const Reference<XComponentContext>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_PATTERNFIELD)
{
}


Sequence<Type> OPatternControl::_getTypes()
{
    return OBoundControl::_getTypes();
}


css::uno::Sequence<OUString> OPatternControl::getSupportedServiceNames() throw(std::exception)
{
    css::uno::Sequence<OUString> aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_CONTROL_PATTERNFIELD;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_PATTERNFIELD;
    return aSupported;
}


// OPatternModel


Sequence<Type> OPatternModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}



OPatternModel::OPatternModel(const Reference<XComponentContext>& _rxFactory)
    :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_PATTERNFIELD, FRM_SUN_CONTROL_PATTERNFIELD, false, false )
                                    // use the old control name for compytibility reasons
{

    m_nClassId = FormComponentType::PATTERNFIELD;
    initValueProperty( PROPERTY_TEXT, PROPERTY_ID_TEXT );
}


OPatternModel::OPatternModel( const OPatternModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
{
}


OPatternModel::~OPatternModel()
{
}

// XCloneable

IMPLEMENT_DEFAULT_CLONING( OPatternModel )

// XServiceInfo

css::uno::Sequence<OUString> SAL_CALL OPatternModel::getSupportedServiceNames() throw(std::exception)
{
    css::uno::Sequence<OUString> aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 3);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-3] = FRM_SUN_COMPONENT_DATABASE_PATTERNFIELD;
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_PATTERNFIELD;
    pArray[aSupported.getLength()-1] = FRM_COMPONENT_PATTERNFIELD;
    return aSupported;
}



void OPatternModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 4, OEditBaseModel )
        DECL_PROP2(DEFAULT_TEXT,    OUString,    BOUND, MAYBEDEFAULT);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                  BOUND);
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);
        DECL_PROP2(FILTERPROPOSAL,  sal_Bool,           BOUND, MAYBEDEFAULT);
    END_DESCRIBE_PROPERTIES();
}


OUString SAL_CALL OPatternModel::getServiceName() throw ( css::uno::RuntimeException, std::exception)
{
    return OUString(FRM_COMPONENT_PATTERNFIELD);  // old (non-sun) name for compatibility !
}


bool OPatternModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    Any aNewValue( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) );

    if ( aNewValue != m_aLastKnownValue )
    {
        OUString sNewValue;
        aNewValue >>= sNewValue;

        if  (   !aNewValue.hasValue()
            ||  (   sNewValue.isEmpty()         // an empty string
                &&  m_bEmptyIsNull              // which should be interpreted as NULL
                )
            )
        {
            m_xColumnUpdate->updateNull();
        }
        else
        {
            OSL_ENSURE( m_pFormattedValue.get(), "OPatternModel::commitControlValueToDbColumn: no value helper!" );
            if ( !m_pFormattedValue.get() )
                return false;

            if ( !m_pFormattedValue->setFormattedValue( sNewValue ) )
                return false;
        }

        m_aLastKnownValue = aNewValue;
    }

    return true;
}


void OPatternModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    OEditBaseModel::onConnectedDbColumn( _rxForm );

    Reference< XPropertySet > xField( getField() );
    if ( !xField.is() )
        return;

    m_pFormattedValue.reset( new ::dbtools::FormattedColumnValue( getContext(), Reference< XRowSet >( _rxForm, UNO_QUERY ), xField ) );
}


void OPatternModel::onDisconnectedDbColumn()
{
    OEditBaseModel::onDisconnectedDbColumn();
    m_pFormattedValue.reset();
}

// XPropertyChangeListener

Any OPatternModel::translateDbColumnToControlValue()
{
    OSL_PRECOND( m_pFormattedValue.get(), "OPatternModel::translateDbColumnToControlValue: no value helper!" );

    if ( m_pFormattedValue.get() )
    {
        OUString sValue( m_pFormattedValue->getFormattedValue() );
        if  (   sValue.isEmpty()
            &&  m_pFormattedValue->getColumn().is()
            &&  m_pFormattedValue->getColumn()->wasNull()
            )
        {
            m_aLastKnownValue.clear();
        }
        else
        {
            m_aLastKnownValue <<= sValue;
        }
    }
    else
        m_aLastKnownValue.clear();

    return m_aLastKnownValue.hasValue() ? m_aLastKnownValue : makeAny( OUString() );
        // (m_aLastKnownValue is allowed to be VOID, the control value isn't)
}

// XReset

Any OPatternModel::getDefaultForReset() const
{
    return makeAny( m_aDefaultText );
}

void OPatternModel::resetNoBroadcast()
{
    OEditBaseModel::resetNoBroadcast();
    m_aLastKnownValue.clear();
}

}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_OPatternModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OPatternModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_OPatternControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OPatternControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
