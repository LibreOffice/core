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
#pragma once

#include <MutexContainer.hxx>
#include <comphelper/interfacecontainer2.hxx>

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

class UpDownBarWrapper : public MutexContainer
                       , public ::cppu::WeakImplHelper
                        < css::lang::XComponent
                        , css::lang::XServiceInfo
                        , css::beans::XPropertySet
                        , css::beans::XMultiPropertySet
                        , css::beans::XPropertyState
                        , css::beans::XMultiPropertyStates
                        >
{
public:
    UpDownBarWrapper(bool bUp, const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);
    virtual ~UpDownBarWrapper() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // ____ XComponent ____
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference<
                                            css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference<
                                               css::lang::XEventListener >& aListener ) override;

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

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper2      m_aEventListenerContainer;

    OUString       m_aPropertySetName;
};

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
