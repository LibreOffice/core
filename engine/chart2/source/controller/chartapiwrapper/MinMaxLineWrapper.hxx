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

#include <WrappedIgnoreProperty.hxx>
#include <comphelper/interfacecontainer4.hxx>

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <memory>

namespace chart::wrapper
{

class Chart2ModelContact;

class MinMaxLineWrapper : public ::cppu::WeakImplHelper
                        < css::lang::XComponent
                        , css::lang::XServiceInfo
                        , css::beans::XPropertySet
                        , css::beans::XMultiPropertySet
                        , css::beans::XPropertyState
                        , css::beans::XMultiPropertyStates
                        >
{
public:
    explicit MinMaxLineWrapper(std::shared_ptr<Chart2ModelContact> spChart2ModelContact);
    virtual ~MinMaxLineWrapper() override;

    /// XServiceInfo declarations
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // ____ XComponent ____
    virtual void dispose() override;
    virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;

    virtual void addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
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
    virtual cpo::uno::Sequence< css::beans::PropertyState > getPropertyStates( const cpo::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void setPropertyToDefault( const OUString& PropertyName ) override;
    virtual cpo::uno::Any getPropertyDefault( const OUString& aPropertyName ) override;

    //XMultiPropertyStates
    //getPropertyStates() already declared in XPropertyState
    virtual void setAllPropertiesToDefault(  ) override;
    virtual void setPropertiesToDefault( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual cpo::uno::Sequence< cpo::uno::Any > getPropertyDefaults( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;

private: //member
    std::mutex m_aMutex;
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_aEventListenerContainer;

    WrappedIgnoreProperty               m_aWrappedLineJointProperty;
};

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
