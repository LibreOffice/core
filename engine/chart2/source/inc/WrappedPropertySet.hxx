/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include "WrappedProperty.hxx"
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <cppuhelper/implbase.hxx>

#include <memory>
#include <mutex>
#include <vector>

namespace cppu { class IPropertyArrayHelper; }
namespace cppu { class OPropertyArrayHelper; }

namespace chart
{

class WrappedPropertySet :
                         public ::cppu::WeakImplHelper
                         < css::beans::XPropertySet
                         , css::beans::XMultiPropertySet
                         , css::beans::XPropertyState
                         , css::beans::XMultiPropertyStates
                         >
{
public:
    WrappedPropertySet();
    virtual ~WrappedPropertySet() override;

    // rGuard must own m_aMutex on entry.
    void clearWrappedPropertySet(std::unique_lock<std::mutex>& rGuard);

public:
    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    void setPropertyValue( std::unique_lock<std::mutex>& rGuard, const OUString& aPropertyName, const cpo::uno::Any& aValue );
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    cpo::uno::Any getPropertyValue( std::unique_lock<std::mutex>& rGuard, const OUString& PropertyName );

    virtual void addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    void addPropertyChangeListener( std::unique_lock<std::mutex>& rGuard, const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener );
    virtual void removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    void removePropertyChangeListener( std::unique_lock<std::mutex>& rGuard, const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener );
    virtual void addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XMultiPropertySet
    //getPropertySetInfo() already declared in XPropertySet
    virtual void setPropertyValues( const cpo::uno::Sequence< OUString >& aPropertyNames, const cpo::uno::Sequence< cpo::uno::Any >& aValues ) override;
    virtual cpo::uno::Sequence< cpo::uno::Any > getPropertyValues( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void addPropertiesChangeListener( const cpo::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void firePropertiesChangeEvent( const cpo::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    //XPropertyState
    virtual css::beans::PropertyState getPropertyState( const OUString& PropertyName ) override;
    css::beans::PropertyState getPropertyState( std::unique_lock<std::mutex>& rGuard, const OUString& PropertyName );
    virtual cpo::uno::Sequence< css::beans::PropertyState > getPropertyStates( const cpo::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void setPropertyToDefault( const OUString& PropertyName ) override;
    virtual cpo::uno::Any getPropertyDefault( const OUString& aPropertyName ) override;

    //XMultiPropertyStates
    //getPropertyStates() already declared in XPropertyState
    virtual void setAllPropertiesToDefault(  ) override;
    virtual void setPropertiesToDefault( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual cpo::uno::Sequence< cpo::uno::Any > getPropertyDefaults( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;

protected: //methods
    /** give all the properties that should be visible to the outer side
    */
    virtual const cpo::uno::Sequence< css::beans::Property >& getPropertySequence()=0;
    /** give a list of all properties that need a special treatment;
    properties that are not in this list will be wrapped identical.
    The base class 'WrappedPropertySet' will take ownership on the contained pointer.
    It is not allowed to have duplicate entries in this list.
    */
    virtual std::vector< std::unique_ptr<WrappedProperty> > createWrappedProperties()=0;

    virtual css::uno::Reference< css::beans::XPropertySet > getInnerPropertySet() = 0;
    css::uno::Reference< css::beans::XPropertyState > getInnerPropertyState();

    ::cppu::IPropertyArrayHelper&   getInfoHelper(std::unique_lock<std::mutex>& rGuard);
    tWrappedPropertyMap&            getWrappedPropertyMap(std::unique_lock<std::mutex>& rGuard);

    const WrappedProperty*          getWrappedProperty( std::unique_lock<std::mutex>& rGuard, const OUString& rOuterName );
    const WrappedProperty*          getWrappedProperty( std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle );

    std::mutex m_aMutex;

private:
    css::uno::Reference< css::beans::XPropertySetInfo >     m_xInfo;//outer PropertySetInfo

    std::unique_ptr<::cppu::OPropertyArrayHelper>       m_pPropertyArrayHelper;//holds all possible outer properties

    std::unique_ptr<tWrappedPropertyMap>                m_pWrappedPropertyMap;//holds all wrapped properties (containing the special mapping from inner to outer properties)

    //Container for the XPropertyChangedListener. The listeners are inserted by handle.
    //OMultiTypeInterfaceContainerHelperInt32             m_aBoundListenerContainer;

    //Container for the XPropertyVetoableListener. The listeners are inserted by handle.
    //OMultiTypeInterfaceContainerHelperInt32             m_aVetoableListenerContainer;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
