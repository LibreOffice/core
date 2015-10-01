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

#include "CheckBox.hxx"
#include "property.hxx"
#include "property.hrc"
#include "services.hxx"
#include <comphelper/basicio.hxx>
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
using namespace ::com::sun::star::form::binding;

OCheckBoxControl::OCheckBoxControl(const Reference<XComponentContext>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_CHECKBOX)
{
}


css::uno::Sequence<OUString> SAL_CALL OCheckBoxControl::getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence<OUString> aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_CONTROL_CHECKBOX;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_CHECKBOX;
    return aSupported;
}


//= OCheckBoxModel

OCheckBoxModel::OCheckBoxModel(const Reference<XComponentContext>& _rxFactory)
    :OReferenceValueComponent( _rxFactory, VCL_CONTROLMODEL_CHECKBOX, FRM_SUN_CONTROL_CHECKBOX, true )
    // use the old control name for compytibility reasons
{

    m_nClassId = FormComponentType::CHECKBOX;
    initValueProperty( PROPERTY_STATE, PROPERTY_ID_STATE );
}


OCheckBoxModel::OCheckBoxModel( const OCheckBoxModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OReferenceValueComponent( _pOriginal, _rxFactory )
{
}


OCheckBoxModel::~OCheckBoxModel()
{
}


IMPLEMENT_DEFAULT_CLONING( OCheckBoxModel )

// XServiceInfo

css::uno::Sequence<OUString> SAL_CALL OCheckBoxModel::getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence<OUString> aSupported = OReferenceValueComponent::getSupportedServiceNames();

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 9 );
    OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_CHECKBOX;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_CHECKBOX;
    *pStoreTo++ = BINDABLE_DATABASE_CHECK_BOX;

    *pStoreTo++ = FRM_COMPONENT_CHECKBOX;

    return aSupported;
}


void OCheckBoxModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 1, OReferenceValueComponent )
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);
    END_DESCRIBE_PROPERTIES();
}


OUString SAL_CALL OCheckBoxModel::getServiceName() throw(RuntimeException, std::exception)
{
    return OUString(FRM_COMPONENT_CHECKBOX);  // old (non-sun) name for compatibility !
}


void SAL_CALL OCheckBoxModel::write(const Reference<css::io::XObjectOutputStream>& _rxOutStream)
    throw(css::io::IOException, RuntimeException, std::exception)
{
    OReferenceValueComponent::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0003);
    // Properties
    _rxOutStream << getReferenceValue();
    _rxOutStream << (sal_Int16)getDefaultChecked();
    writeHelpTextCompatibly(_rxOutStream);
    // from version 0x0003 : common properties
    writeCommonProperties(_rxOutStream);
}


void SAL_CALL OCheckBoxModel::read(const Reference<css::io::XObjectInputStream>& _rxInStream) throw(css::io::IOException, RuntimeException, std::exception)
{
    OReferenceValueComponent::read(_rxInStream);
    osl::MutexGuard aGuard(m_aMutex);

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();

    OUString sReferenceValue;
    sal_Int16       nDefaultChecked( 0 );
    switch ( nVersion )
    {
        case 0x0001:
            _rxInStream >> sReferenceValue;
            nDefaultChecked = _rxInStream->readShort();
            break;
        case 0x0002:
            _rxInStream >> sReferenceValue;
            _rxInStream >> nDefaultChecked;
            readHelpTextCompatibly( _rxInStream );
            break;
        case 0x0003:
            _rxInStream >> sReferenceValue;
            _rxInStream >> nDefaultChecked;
            readHelpTextCompatibly(_rxInStream);
            readCommonProperties(_rxInStream);
            break;
        default:
            OSL_FAIL("OCheckBoxModel::read : unknown version !");
            defaultCommonProperties();
            break;
    }
    setReferenceValue( sReferenceValue );
    setDefaultChecked( static_cast< ToggleState >( nDefaultChecked ) );

    // After reading in, display the default values
    if ( !getControlSource().isEmpty() )
        // (not if we don't have a control source - the "State" property acts like it is persistent, then
        resetNoBroadcast();
}

bool OCheckBoxModel::DbUseBool()
{
    if ( ! (getReferenceValue().isEmpty() && getNoCheckReferenceValue().isEmpty()) )
        return false;
    return true;
}


Any OCheckBoxModel::translateDbColumnToControlValue()
{
    Any aValue;


    // Set value in ControlModel
    bool bValue = bool(); // avoid warning
    if(DbUseBool())
    {
        bValue = m_xColumn->getBoolean();
    }
    else
    {
        const OUString sVal(m_xColumn->getString());
        if (sVal == getReferenceValue())
            bValue = true;
        else if (sVal == getNoCheckReferenceValue())
            bValue = false;
        else
            aValue <<= static_cast<sal_Int16>(getDefaultChecked());
    }
    if ( m_xColumn->wasNull() )
    {
        bool bTriState = true;
        if ( m_xAggregateSet.is() )
            m_xAggregateSet->getPropertyValue( PROPERTY_TRISTATE ) >>= bTriState;
        aValue <<= (sal_Int16)( bTriState ? TRISTATE_INDET : getDefaultChecked() );
    }
    else if ( !aValue.hasValue() )
    {
        // Since above either bValue is initialised, either aValue.hasValue(),
        // bValue cannot be used uninitialised here.
        // But GCC does not see/understand that, which breaks -Werror builds,
        // so we explicitly default-initialise it.
        aValue <<= (sal_Int16)( bValue ? TRISTATE_TRUE : TRISTATE_FALSE );
    }

    return aValue;
}


bool OCheckBoxModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    OSL_PRECOND( m_xColumnUpdate.is(), "OCheckBoxModel::commitControlValueToDbColumn: not bound!" );
    if ( m_xColumnUpdate.is() )
    {
        Any aControlValue( m_xAggregateSet->getPropertyValue( PROPERTY_STATE ) );
        try
        {
            sal_Int16 nValue = TRISTATE_INDET;
            aControlValue >>= nValue;
            switch (nValue)
            {
                case TRISTATE_INDET:
                    m_xColumnUpdate->updateNull();
                    break;
                case TRISTATE_TRUE:
                    if (DbUseBool())
                        m_xColumnUpdate->updateBoolean( sal_True );
                    else
                        m_xColumnUpdate->updateString( getReferenceValue() );
                    break;
                case TRISTATE_FALSE:
                    if (DbUseBool())
                        m_xColumnUpdate->updateBoolean( sal_False );
                    else
                        m_xColumnUpdate->updateString( getNoCheckReferenceValue() );
                    break;
                default:
                    OSL_FAIL("OCheckBoxModel::commitControlValueToDbColumn: invalid value !");
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("OCheckBoxModel::commitControlValueToDbColumn: could not commit !");
        }
    }
    return true;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_OCheckBoxModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OCheckBoxModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_OCheckBoxControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OCheckBoxControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
