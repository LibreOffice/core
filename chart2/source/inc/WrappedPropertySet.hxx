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
#ifndef INCLUDED_CHART2_SOURCE_INC_WRAPPEDPROPERTYSET_HXX
#define INCLUDED_CHART2_SOURCE_INC_WRAPPEDPROPERTYSET_HXX

#include "WrappedProperty.hxx"
#include "MutexContainer.hxx"
#include "charttoolsdllapi.hxx"
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/propshlp.hxx>

#include <memory>
#include <vector>

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS WrappedPropertySet :
                           public MutexContainer
                         , public ::cppu::WeakImplHelper
                         < css::beans::XPropertySet
                         , css::beans::XMultiPropertySet
                         , css::beans::XPropertyState
                         , css::beans::XMultiPropertyStates
                         >
{
public:
    WrappedPropertySet();
    virtual ~WrappedPropertySet() override;

    void clearWrappedPropertySet();

public:
    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;

    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XMultiPropertySet
    //getPropertySetInfo() already declared in XPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    //XMultiPropertyStates
    //getPropertyStates() already declared in XPropertyState
    virtual void SAL_CALL setAllPropertiesToDefault(  ) override;
    virtual void SAL_CALL setPropertiesToDefault( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyDefaults( const css::uno::Sequence< OUString >& aPropertyNames ) override;

protected: //methods
    /** give all the properties that should be visible to the outer side
    */
    virtual const css::uno::Sequence< css::beans::Property >& getPropertySequence()=0;
    /** give a list of all properties that need a special treatment;
    properties that are not in this list will be wrapped identical.
    The base class 'WrappedPropertySet' will take ownership on the contained pointer.
    It is not allowed to have duplicate entries in this list.
    */
    virtual const std::vector< std::unique_ptr<WrappedProperty> > createWrappedProperties()=0;

    virtual css::uno::Reference< css::beans::XPropertySet > getInnerPropertySet() = 0;
    SAL_DLLPRIVATE css::uno::Reference< css::beans::XPropertyState > getInnerPropertyState();

    ::cppu::IPropertyArrayHelper&   getInfoHelper();
    SAL_DLLPRIVATE tWrappedPropertyMap&            getWrappedPropertyMap();

    const WrappedProperty*          getWrappedProperty( const OUString& rOuterName );
    const WrappedProperty*          getWrappedProperty( sal_Int32 nHandle );

protected: //member
    css::uno::Reference< css::beans::XPropertySetInfo >     m_xInfo;//outer PropertySetInfo

    std::unique_ptr<::cppu::OPropertyArrayHelper>       m_pPropertyArrayHelper;//holds all possible outer properties

    std::unique_ptr<tWrappedPropertyMap>                m_pWrappedPropertyMap;//holds all wrapped properties (containing the special mapping from inner to outer properties)

    //Container for the XProperyChangedListener. The listeners are inserted by handle.
    //OMultiTypeInterfaceContainerHelperInt32             m_aBoundListenerContainer;

    //Container for the XPropertyVetoableListener. The listeners are inserted by handle.
    //OMultiTypeInterfaceContainerHelperInt32             m_aVetoableListenerContainer;
};

} //namespace chart

// INCLUDED_CHART2_SOURCE_INC_WRAPPEDPROPERTYSET_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
