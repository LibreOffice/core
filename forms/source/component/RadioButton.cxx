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

#include "RadioButton.hxx"
#include "GroupManager.hxx"
#include <property.hxx>
#include <services.hxx>
#include <comphelper/basicio.hxx>
#include <comphelper/property.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/form/FormComponentType.hpp>

namespace frm
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;


css::uno::Sequence<OUString> SAL_CALL ORadioButtonControl::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_CONTROL_RADIOBUTTON;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_RADIOBUTTON;
    return aSupported;
}


ORadioButtonControl::ORadioButtonControl(const Reference<XComponentContext>& _rxFactory)
                      :OBoundControl(_rxFactory, VCL_CONTROL_RADIOBUTTON)
{
}


ORadioButtonModel::ORadioButtonModel(const Reference<XComponentContext>& _rxFactory)
    :OReferenceValueComponent( _rxFactory, VCL_CONTROLMODEL_RADIOBUTTON, FRM_SUN_CONTROL_RADIOBUTTON )
                    // use the old control name for compytibility reasons
{

    m_nClassId = FormComponentType::RADIOBUTTON;
    m_aLabelServiceName = FRM_SUN_COMPONENT_GROUPBOX;
    initValueProperty( PROPERTY_STATE, PROPERTY_ID_STATE );
    startAggregatePropertyListening( PROPERTY_GROUP_NAME );
}


ORadioButtonModel::ORadioButtonModel( const ORadioButtonModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OReferenceValueComponent( _pOriginal, _rxFactory )
{
}


ORadioButtonModel::~ORadioButtonModel()
{
}

// XCloneable

css::uno::Reference< css::util::XCloneable > SAL_CALL ORadioButtonModel::createClone()
{
    rtl::Reference<ORadioButtonModel> pClone = new ORadioButtonModel(this, getContext());
    pClone->clonedFrom(this);
    return pClone;
}

// XServiceInfo

css::uno::Sequence<OUString> SAL_CALL ORadioButtonModel::getSupportedServiceNames()
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

    *pStoreTo++ = FRM_SUN_COMPONENT_RADIOBUTTON;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_RADIOBUTTON;
    *pStoreTo++ = BINDABLE_DATABASE_RADIO_BUTTON;

    *pStoreTo++ = FRM_COMPONENT_RADIOBUTTON;

    return aSupported;
}


void ORadioButtonModel::SetSiblingPropsTo(const OUString& rPropName, const Any& rValue)
{
    // my name
    OUString sMyGroup;
    if (hasProperty(PROPERTY_GROUP_NAME, this))
        getPropertyValue(PROPERTY_GROUP_NAME) >>= sMyGroup;
    if (sMyGroup.isEmpty())
        sMyGroup = m_aName;

    // Iterate over my siblings
    Reference<XIndexAccess> xIndexAccess(getParent(), UNO_QUERY);
    if (!xIndexAccess.is())
        return;

    Reference<XPropertySet> xMyProps = this;
    OUString sCurrentGroup;
    sal_Int32 nNumSiblings = xIndexAccess->getCount();
    for (sal_Int32 i=0; i<nNumSiblings; ++i)
    {
        Reference<XPropertySet> xSiblingProperties(xIndexAccess->getByIndex(i), UNO_QUERY);
        if (!xSiblingProperties.is())
            continue;
        if (xMyProps == xSiblingProperties)
            continue;   // do not set myself

        // Only if it's a RadioButton
        if (!hasProperty(PROPERTY_CLASSID, xSiblingProperties))
            continue;
        sal_Int16 nType = 0;
        xSiblingProperties->getPropertyValue(PROPERTY_CLASSID) >>= nType;
        if (nType != FormComponentType::RADIOBUTTON)
            continue;

        // The group association is attached to the name
        sCurrentGroup = OGroupManager::GetGroupName( xSiblingProperties );
        if (sCurrentGroup == sMyGroup)
            xSiblingProperties->setPropertyValue(rPropName, rValue);
    }
}


void ORadioButtonModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue)
{
    OReferenceValueComponent::setFastPropertyValue_NoBroadcast( nHandle, rValue );

    // if the label control changed ...
    if (nHandle == PROPERTY_ID_CONTROLLABEL)
    {   // ... forward this to our siblings
        SetSiblingPropsTo(PROPERTY_CONTROLLABEL, rValue);
    }

    // If the ControlSource property has changed ...
    if (nHandle == PROPERTY_ID_CONTROLSOURCE)
    {   // ... I have to pass the new ControlSource to my siblings, which are in the same RadioButton group as I am
        SetSiblingPropsTo(PROPERTY_CONTROLSOURCE, rValue);
    }

    // The other way: if my name changes ...
    if (nHandle == PROPERTY_ID_NAME)
    {
        setControlSource();
    }

    if (nHandle != PROPERTY_ID_DEFAULT_STATE)
        return;

    sal_Int16 nValue;
    rValue >>= nValue;
    if (1 == nValue)
    {   // Reset the 'default checked' for all Radios of the same group.
        // Because (as the Highlander already knew): "There can be only one"
        Any aZero;
        nValue = 0;
        aZero <<= nValue;
        SetSiblingPropsTo(PROPERTY_DEFAULT_STATE, aZero);
    }
}

void ORadioButtonModel::setControlSource()
{
    Reference<XIndexAccess> xIndexAccess(getParent(), UNO_QUERY);
    if (!xIndexAccess.is())
        return;

    OUString sName, sGroupName;

    if (hasProperty(PROPERTY_GROUP_NAME, this))
        getPropertyValue(PROPERTY_GROUP_NAME) >>= sGroupName;
    getPropertyValue(PROPERTY_NAME) >>= sName;

    Reference<XPropertySet> xMyProps = this;
    for (sal_Int32 i=0; i<xIndexAccess->getCount(); ++i)
    {
        Reference<XPropertySet> xSiblingProperties(xIndexAccess->getByIndex(i), UNO_QUERY);
        if (!xSiblingProperties.is())
            continue;

        if (xMyProps == xSiblingProperties)
            // Only if I didn't find myself
            continue;

        sal_Int16 nType = 0;
        xSiblingProperties->getPropertyValue(PROPERTY_CLASSID) >>= nType;
        if (nType != FormComponentType::RADIOBUTTON)
            // Only RadioButtons
            continue;

        OUString sSiblingName, sSiblingGroupName;
        if (hasProperty(PROPERTY_GROUP_NAME, xSiblingProperties))
            xSiblingProperties->getPropertyValue(PROPERTY_GROUP_NAME) >>= sSiblingGroupName;
        xSiblingProperties->getPropertyValue(PROPERTY_NAME) >>= sSiblingName;

        if ((sGroupName.isEmpty() && sSiblingGroupName.isEmpty() &&                 // (no group name
             sName == sSiblingName) ||                                              //  names match) or
            (!sGroupName.isEmpty() && !sSiblingGroupName.isEmpty() &&               // (have group name
             sGroupName == sSiblingGroupName))                                      //  they match)
        {
            setPropertyValue(PROPERTY_CONTROLSOURCE, xSiblingProperties->getPropertyValue(PROPERTY_CONTROLSOURCE));
            break;
        }
    }
}


void ORadioButtonModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    OReferenceValueComponent::describeFixedProperties( _rProps );
    sal_Int32 nOldCount = _rProps.getLength();
    _rProps.realloc( nOldCount + 1);
    css::beans::Property* pProperties = _rProps.getArray() + nOldCount;
    *pProperties++ = css::beans::Property(PROPERTY_TABINDEX, PROPERTY_ID_TABINDEX, cppu::UnoType<sal_Int16>::get(), css::beans::PropertyAttribute::BOUND);
    DBG_ASSERT( pProperties == _rProps.getArray() + _rProps.getLength(), "<...>::describeFixedProperties/getInfoHelper: forgot to adjust the count ?");
}


OUString SAL_CALL ORadioButtonModel::getServiceName()
{
    return FRM_COMPONENT_RADIOBUTTON;   // old (non-sun) name for compatibility !
}


void SAL_CALL ORadioButtonModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
{
    OReferenceValueComponent::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0003);

    // Properties
    _rxOutStream << getReferenceValue();
    _rxOutStream << static_cast<sal_Int16>(getDefaultChecked());
    writeHelpTextCompatibly(_rxOutStream);

    // from version 0x0003 : common properties
    writeCommonProperties(_rxOutStream);
}


void SAL_CALL ORadioButtonModel::read(const Reference<XObjectInputStream>& _rxInStream)
{
    OReferenceValueComponent::read(_rxInStream);
    ::osl::MutexGuard aGuard(m_aMutex);

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();

    OUString sReferenceValue;
    sal_Int16 nDefaultChecked( 0 );
    switch (nVersion)
    {
        case 0x0001 :
            _rxInStream >> sReferenceValue;
            _rxInStream >> nDefaultChecked;
            break;
        case 0x0002 :
            _rxInStream >> sReferenceValue;
            _rxInStream >> nDefaultChecked;
            readHelpTextCompatibly(_rxInStream);
            break;
        case 0x0003 :
            _rxInStream >> sReferenceValue;
            _rxInStream >> nDefaultChecked;
            readHelpTextCompatibly(_rxInStream);
            readCommonProperties(_rxInStream);
            break;
        default :
            OSL_FAIL("ORadioButtonModel::read : unknown version !");
            defaultCommonProperties();
            break;
    }

    setReferenceValue( sReferenceValue );
    setDefaultChecked( static_cast<ToggleState>(nDefaultChecked) );

    // Display default values after read
    if ( !getControlSource().isEmpty() )
        // (not if we don't have a control source - the "State" property acts like it is persistent, then
        resetNoBroadcast();
}


void ORadioButtonModel::_propertyChanged(const PropertyChangeEvent& _rEvent)
{
    if ( _rEvent.PropertyName == PROPERTY_STATE )
    {
        if ( _rEvent.NewValue == sal_Int16(1) )
        {
            // If my status has changed to 'checked', I have to reset all my siblings, which are in the same group as I am
            Any aZero;
            aZero <<= sal_Int16(0);
            SetSiblingPropsTo( PROPERTY_STATE, aZero );
        }
    }
    else if ( _rEvent.PropertyName == PROPERTY_GROUP_NAME )
    {
        setControlSource();
        // Can't call OReferenceValueComponent::_propertyChanged(), as it
        // doesn't know what to do with the GroupName property.
        return;
    }

    OReferenceValueComponent::_propertyChanged( _rEvent );
}


Any ORadioButtonModel::translateDbColumnToControlValue()
{
    return Any( static_cast<sal_Int16>( ( m_xColumn->getString() == getReferenceValue() ) ? TRISTATE_TRUE : TRISTATE_FALSE )
    );
}


Any ORadioButtonModel::translateExternalValueToControlValue( const Any& _rExternalValue ) const
{
    Any aControlValue = OReferenceValueComponent::translateExternalValueToControlValue( _rExternalValue );
    sal_Int16 nState = TRISTATE_FALSE;
    if ( ( aControlValue >>= nState ) && ( nState == TRISTATE_INDET ) )
        // radio buttons do not have the DONTKNOW state
        aControlValue <<= sal_Int16(TRISTATE_FALSE);
    return aControlValue;
}


bool ORadioButtonModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    Reference< XPropertySet > xField( getField() );
    OSL_PRECOND( xField.is(), "ORadioButtonModel::commitControlValueToDbColumn: not bound!" );
    if ( xField.is() )
    {
        try
        {
            sal_Int16 nValue = 0;
            m_xAggregateSet->getPropertyValue( PROPERTY_STATE ) >>= nValue;
            if ( nValue == 1 )
                xField->setPropertyValue( PROPERTY_VALUE, Any( getReferenceValue() ) );
        }
        catch(const Exception&)
        {
            OSL_FAIL("ORadioButtonModel::commitControlValueToDbColumn: could not commit !");
        }
    }
    return true;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_ORadioButtonModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::ORadioButtonModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_ORadioButtonControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::ORadioButtonControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
