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
#include "property.hxx"
#include "property.hrc"
#include "services.hxx"
#include <tools/debug.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/basicio.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>

//.........................................................................
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

//==================================================================

InterfaceRef SAL_CALL ORadioButtonControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new ORadioButtonControl( comphelper::getComponentContext(_rxFactory) ));
}


StringSequence SAL_CALL ORadioButtonControl::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_RADIOBUTTON;
    return aSupported;
}



ORadioButtonControl::ORadioButtonControl(const Reference<XComponentContext>& _rxFactory)
                      :OBoundControl(_rxFactory, VCL_CONTROL_RADIOBUTTON)
{
}


void SAL_CALL ORadioButtonControl::createPeer(const Reference<starawt::XToolkit>& _rxToolkit, const Reference<starawt::XWindowPeer>& _rxParent) throw (RuntimeException)
{
    OBoundControl::createPeer(_rxToolkit, _rxParent);

    // switch off the auto-toggle, we do this ourself ....
    // (formerly this switch-off was done in the toolkit - but the correct place is here ...)
//  Reference< XVclWindowPeer >  xVclWindowPeer( getPeer(), UNO_QUERY );
//  if (xVclWindowPeer.is())
//      xVclWindowPeer->setProperty(OUString("AutoToggle"), ::cppu::bool2any(sal_False));
    // new order: do _not_ switch off the auto toggle because:
    // * today, it is not necessary anymore to handle the toggling ourself (everything works fine without it)
    // * without auto toggle, the AccessibleEvents as fired by the radio buttons are
    //     a. newly checked button: "unchecked"->"checked"
    //     b. previously checked button: "checked"->"unchecked"
    //   This is deadly for AT-tools, which then get the "unchecked" event _immediately_ after the "checked" event,
    //   and only read the latter. This makes radio buttons pretty unusable in form documents.
    //   So we switched AutoToggle _on_, again, because then VCL can handle the notifications, and will send
    //   them in the proper order.
}

//==================================================================
InterfaceRef SAL_CALL ORadioButtonModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new ORadioButtonModel( comphelper::getComponentContext(_rxFactory) ));
}



ORadioButtonModel::ORadioButtonModel(const Reference<XComponentContext>& _rxFactory)
    :OReferenceValueComponent( _rxFactory, VCL_CONTROLMODEL_RADIOBUTTON, FRM_SUN_CONTROL_RADIOBUTTON,sal_True )
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

IMPLEMENT_DEFAULT_CLONING( ORadioButtonModel )

// XServiceInfo

StringSequence SAL_CALL ORadioButtonModel::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OReferenceValueComponent::getSupportedServiceNames();

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 8 );
    OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_RADIOBUTTON;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_RADIOBUTTON;
    *pStoreTo++ = BINDABLE_DATABASE_RADIO_BUTTON;

    return aSupported;
}


void ORadioButtonModel::SetSiblingPropsTo(const OUString& rPropName, const Any& rValue)
{
    // my name
    OUString sMyGroup;
    if (hasProperty(PROPERTY_GROUP_NAME, this))
        this->getPropertyValue(PROPERTY_GROUP_NAME) >>= sMyGroup;
    if (sMyGroup.isEmpty())
        sMyGroup = m_aName;

    // Iterate over my siblings
    Reference<XIndexAccess> xIndexAccess(getParent(), UNO_QUERY);
    if (xIndexAccess.is())
    {
        Reference<XPropertySet> xMyProps;
        query_interface(static_cast<XWeak*>(this), xMyProps);
        OUString sCurrentGroup;
        sal_Int32 nNumSiblings = xIndexAccess->getCount();
        for (sal_Int32 i=0; i<nNumSiblings; ++i)
        {
            Reference<XPropertySet> xSiblingProperties(*(InterfaceRef*)xIndexAccess->getByIndex(i).getValue(), UNO_QUERY);
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
}


void ORadioButtonModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) throw (Exception)
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

    if (nHandle == PROPERTY_ID_DEFAULT_STATE)
    {
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
}

void ORadioButtonModel::setControlSource()
{
    Reference<XIndexAccess> xIndexAccess(getParent(), UNO_QUERY);
    if (xIndexAccess.is())
    {
        OUString sName, sGroupName;

        if (hasProperty(PROPERTY_GROUP_NAME, this))
            this->getPropertyValue(PROPERTY_GROUP_NAME) >>= sGroupName;
        this->getPropertyValue(PROPERTY_NAME) >>= sName;

        Reference<XPropertySet> xMyProps;
        query_interface(static_cast<XWeak*>(this), xMyProps);
        for (sal_Int32 i=0; i<xIndexAccess->getCount(); ++i)
        {
            Reference<XPropertySet> xSiblingProperties(*(InterfaceRef*)xIndexAccess->getByIndex(i).getValue(), UNO_QUERY);
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
}


void ORadioButtonModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 1, OReferenceValueComponent )
        DECL_PROP1(TABINDEX,            sal_Int16,                  BOUND);
    END_DESCRIBE_PROPERTIES();
}


OUString SAL_CALL ORadioButtonModel::getServiceName() throw(RuntimeException)
{
    return OUString(FRM_COMPONENT_RADIOBUTTON);   // old (non-sun) name for compatibility !
}


void SAL_CALL ORadioButtonModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
    throw(IOException, RuntimeException)
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


void SAL_CALL ORadioButtonModel::read(const Reference<XObjectInputStream>& _rxInStream) throw(IOException, RuntimeException)
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
    setDefaultChecked( (ToggleState)nDefaultChecked );

    // Display default values after read
    if ( !getControlSource().isEmpty() )
        // (not if we don't have a control source - the "State" property acts like it is persistent, then
        resetNoBroadcast();
}


void ORadioButtonModel::_propertyChanged(const PropertyChangeEvent& _rEvent) throw(RuntimeException)
{
    if ( _rEvent.PropertyName.equals( PROPERTY_STATE ) )
    {
        if ( _rEvent.NewValue == (sal_Int16)1 )
        {
            // If my status has changed to 'checked', I have to reset all my siblings, which are in the same group as I am
            Any aZero;
            aZero <<= (sal_Int16)0;
            SetSiblingPropsTo( PROPERTY_STATE, aZero );
        }
    }
    else if ( _rEvent.PropertyName.equals( PROPERTY_GROUP_NAME ) )
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
    return makeAny( (sal_Int16)
        ( ( m_xColumn->getString() == getReferenceValue() ) ? STATE_CHECK : STATE_NOCHECK )
    );
}


Any ORadioButtonModel::translateExternalValueToControlValue( const Any& _rExternalValue ) const
{
    Any aControlValue = OReferenceValueComponent::translateExternalValueToControlValue( _rExternalValue );
    sal_Int16 nState = STATE_NOCHECK;
    if ( ( aControlValue >>= nState ) && ( nState == STATE_DONTKNOW ) )
        // radio buttons do not have the DONTKNOW state
        aControlValue <<= (sal_Int16)STATE_NOCHECK;
    return aControlValue;
}


sal_Bool ORadioButtonModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
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
                xField->setPropertyValue( PROPERTY_VALUE, makeAny( getReferenceValue() ) );
        }
        catch(const Exception&)
        {
            OSL_FAIL("ORadioButtonModel::commitControlValueToDbColumn: could not commit !");
        }
    }
    return sal_True;
}

//.........................................................................
}
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
