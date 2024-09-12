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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ucb/XPropertySetRegistryFactory.hpp>
#include <com/sun/star/ucb/XPropertySetRegistry.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertySetInfoChangeNotifier.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/multiinterfacecontainer4.hxx>
#include <comphelper/compbase.hxx>
#include <rtl/ref.hxx>
#include <unordered_map>

class PropertySetRegistry;

using UcbStore_Base = comphelper::WeakComponentImplHelper <
                        css::lang::XServiceInfo,
                        css::ucb::XPropertySetRegistryFactory,
                        css::lang::XInitialization >;

class UcbStore : public UcbStore_Base
{
    css::uno::Reference< css::uno::XComponentContext >    m_xContext;
    css::uno::Sequence< css::uno::Any >                   m_aInitArgs;
    rtl::Reference< PropertySetRegistry >                 m_xTheRegistry;

public:
    explicit UcbStore( const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~UcbStore() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XPropertySetRegistryFactory
    virtual css::uno::Reference< css::ucb::XPropertySetRegistry > SAL_CALL
    createPropertySetRegistry( const OUString& URL ) override;

    // XInitialization
    virtual void SAL_CALL
    initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;
};


class PersistentPropertySet;

// PropertySetMap_Impl.
typedef std::unordered_map< OUString, PersistentPropertySet*> PropertySetMap_Impl;

class PropertySetRegistry : public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::ucb::XPropertySetRegistry,
    css::container::XNameAccess >
{
    friend class PersistentPropertySet;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    const css::uno::Sequence< css::uno::Any >             m_aInitArgs;
    PropertySetMap_Impl               m_aPropSets;
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xConfigProvider;
    css::uno::Reference< css::uno::XInterface >           m_xRootReadAccess;
    css::uno::Reference< css::uno::XInterface >           m_xRootWriteAccess;
    std::mutex                        m_aMutex;
    bool                              m_bTriedToGetRootReadAccess;
    bool                              m_bTriedToGetRootWriteAccess;

private:
    css::uno::Reference< css::lang::XMultiServiceFactory >
    getConfigProvider(std::unique_lock<std::mutex>& l);

    void add   ( std::unique_lock<std::mutex>& rCreatorGuard, PersistentPropertySet* pSet );
    void remove( PersistentPropertySet* pSet );

    void renamePropertySet( const OUString& rOldKey,
                            const OUString& rNewKey );

public:
    PropertySetRegistry(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Sequence< css::uno::Any >& rInitArgs);
    virtual ~PropertySetRegistry() override;


    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XPropertySetRegistry
    virtual css::uno::Reference< css::ucb::XPersistentPropertySet > SAL_CALL
    openPropertySet( const OUString& key, sal_Bool create ) override;
    virtual void SAL_CALL
    removePropertySet( const OUString& key ) override;

    // XElementAccess ( XNameAccess is derived from it )
    virtual css::uno::Type SAL_CALL
    getElementType() override;
    virtual sal_Bool SAL_CALL
    hasElements() override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL
    getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
    getElementNames() override;
    virtual sal_Bool SAL_CALL
    hasByName( const OUString& aName ) override;

    // Non-interface methods
    css::uno::Reference< css::uno::XInterface >
    getRootConfigReadAccess();
    css::uno::Reference< css::uno::XInterface >
    getConfigWriteAccess( const OUString& rPath );
private:
    css::uno::Reference< css::uno::XInterface >
    getRootConfigReadAccessImpl(std::unique_lock<std::mutex>& l);
    css::uno::Reference< css::uno::XInterface >
    getConfigWriteAccessImpl( std::unique_lock<std::mutex>& l, const OUString& rPath );
};


class PropertySetInfo_Impl;
typedef comphelper::OMultiTypeInterfaceContainerHelperVar4<OUString, css::beans::XPropertyChangeListener> PropertyListeners_Impl;

class PersistentPropertySet : public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::lang::XComponent,
    css::ucb::XPersistentPropertySet,
    css::container::XNamed,
    css::beans::XPropertyContainer,
    css::beans::XPropertySetInfoChangeNotifier,
    css::beans::XPropertyAccess >
{
    rtl::Reference<PropertySetRegistry>  m_pCreator;
    rtl::Reference<PropertySetInfo_Impl> m_pInfo;
    OUString                    m_aKey;
    OUString                    m_aFullKey;
    mutable std::mutex          m_aMutex;
    comphelper::OInterfaceContainerHelper4<css::lang::XEventListener>  m_aDisposeEventListeners;
    comphelper::OInterfaceContainerHelper4<css::beans::XPropertySetInfoChangeListener>  m_aPropSetChangeListeners;
    PropertyListeners_Impl      m_aPropertyChangeListeners;

private:
    void notifyPropertyChangeEvent(
        std::unique_lock<std::mutex>& rGuard,
        const css::beans::PropertyChangeEvent& rEvent ) const;
    void notifyPropertySetInfoChange(
        std::unique_lock<std::mutex>& rGuard,
        const css::beans::PropertySetInfoChangeEvent& evt ) const;

public:
    PersistentPropertySet(
        std::unique_lock<std::mutex>& rCreatorGuard,
        PropertySetRegistry& rCreator,
        OUString aKey );
    virtual ~PersistentPropertySet() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL
    dispose() override;
    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener ) override;
    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo() override;
    virtual void SAL_CALL
    setPropertyValue( const OUString& aPropertyName,
                      const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL
    getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL
    addPropertyChangeListener( const OUString& aPropertyName,
                               const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL
    removePropertyChangeListener( const OUString& aPropertyName,
                                  const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL
    addVetoableChangeListener( const OUString& PropertyName,
                               const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL
    removeVetoableChangeListener( const OUString& PropertyName,
                                  const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XPersistentPropertySet
    virtual css::uno::Reference< css::ucb::XPropertySetRegistry > SAL_CALL
    getRegistry() override;
    virtual OUString SAL_CALL
    getKey() override;

    // XNamed
    virtual OUString SAL_CALL
    getName() override;
    virtual void SAL_CALL
    setName( const OUString& aName ) override;

    // XPropertyContainer
    virtual void SAL_CALL
    addProperty( const OUString& Name,
                 sal_Int16 Attributes,
                 const css::uno::Any& DefaultValue ) override;
    virtual void SAL_CALL
    removeProperty( const OUString& Name ) override;

    // XPropertySetInfoChangeNotifier
    virtual void SAL_CALL
    addPropertySetInfoChangeListener( const css::uno::Reference< css::beans::XPropertySetInfoChangeListener >& Listener ) override;
    virtual void SAL_CALL
    removePropertySetInfoChangeListener( const css::uno::Reference< css::beans::XPropertySetInfoChangeListener >& Listener ) override;

    // XPropertyAccess
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
    getPropertyValues() override;
    virtual void SAL_CALL
    setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& aProps ) override;

    // Non-interface methods.
    PropertySetRegistry& getPropertySetRegistry();
    OUString getFullKey();
private:
    const OUString& getFullKeyImpl(std::unique_lock<std::mutex>&);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
