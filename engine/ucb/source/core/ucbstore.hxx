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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ucb/XPropertySetRegistryFactory.hpp>
#include <com/sun/star/ucb/XPropertySetRegistry.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <cpo/uno/XComponentContext.hpp>
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
    cpo::uno::Reference< cpo::uno::XComponentContext >    m_xContext;
    cpo::uno::Sequence< cpo::uno::Any >                   m_aInitArgs;
    rtl::Reference< PropertySetRegistry >                 m_xTheRegistry;

public:
    explicit UcbStore( const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext );
    virtual ~UcbStore() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XPropertySetRegistryFactory
    virtual cpo::uno::Reference< css::ucb::XPropertySetRegistry >
    createPropertySetRegistry( const OUString& URL ) override;

    // XInitialization
    virtual void
    initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;
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

    cpo::uno::Reference< cpo::uno::XComponentContext > m_xContext;
    const cpo::uno::Sequence< cpo::uno::Any >             m_aInitArgs;
    PropertySetMap_Impl               m_aPropSets;
    cpo::uno::Reference< css::lang::XMultiServiceFactory > m_xConfigProvider;
    cpo::uno::Reference< cpo::uno::XInterface >           m_xRootReadAccess;
    cpo::uno::Reference< cpo::uno::XInterface >           m_xRootWriteAccess;
    std::mutex                        m_aMutex;
    bool                              m_bTriedToGetRootReadAccess;
    bool                              m_bTriedToGetRootWriteAccess;

private:
    const cpo::uno::Reference< css::lang::XMultiServiceFactory > &
    getConfigProvider(std::unique_lock<std::mutex>& l);

    void add   ( std::unique_lock<std::mutex>& rCreatorGuard, PersistentPropertySet* pSet );
    void remove( PersistentPropertySet* pSet );

    void renamePropertySet( const OUString& rOldKey,
                            const OUString& rNewKey );

public:
    PropertySetRegistry(
        const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext,
        const cpo::uno::Sequence< cpo::uno::Any >& rInitArgs);
    virtual ~PropertySetRegistry() override;


    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XPropertySetRegistry
    virtual cpo::uno::Reference< css::ucb::XPersistentPropertySet >
    openPropertySet( const OUString& key, bool create ) override;
    virtual void
    removePropertySet( const OUString& key ) override;

    // XElementAccess ( XNameAccess is derived from it )
    virtual cpo::uno::Type
    getElementType() override;
    virtual bool
    hasElements() override;

    // XNameAccess
    virtual cpo::uno::Any
    getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString >
    getElementNames() override;
    virtual bool
    hasByName( const OUString& aName ) override;

    // Non-interface methods
    cpo::uno::Reference< cpo::uno::XInterface >
    getRootConfigReadAccess();
    cpo::uno::Reference< cpo::uno::XInterface >
    getConfigWriteAccess( const OUString& rPath );
private:
    cpo::uno::Reference< cpo::uno::XInterface >
    getRootConfigReadAccessImpl(std::unique_lock<std::mutex>& l);
    cpo::uno::Reference< cpo::uno::XInterface >
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
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XComponent
    virtual void
    dispose() override;
    virtual void
    addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& Listener ) override;
    virtual void
    removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& Listener ) override;

    // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
    getPropertySetInfo() override;
    virtual void
    setPropertyValue( const OUString& aPropertyName,
                      const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any
    getPropertyValue( const OUString& PropertyName ) override;
    virtual void
    addPropertyChangeListener( const OUString& aPropertyName,
                               const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void
    removePropertyChangeListener( const OUString& aPropertyName,
                                  const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void
    addVetoableChangeListener( const OUString& PropertyName,
                               const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void
    removeVetoableChangeListener( const OUString& PropertyName,
                                  const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XPersistentPropertySet
    virtual cpo::uno::Reference< css::ucb::XPropertySetRegistry >
    getRegistry() override;
    virtual OUString
    getKey() override;

    // XNamed
    virtual OUString
    getName() override;
    virtual void
    setName( const OUString& aName ) override;

    // XPropertyContainer
    virtual void
    addProperty( const OUString& Name,
                 sal_Int16 Attributes,
                 const cpo::uno::Any& DefaultValue ) override;
    virtual void
    removeProperty( const OUString& Name ) override;

    // XPropertySetInfoChangeNotifier
    virtual void
    addPropertySetInfoChangeListener( const cpo::uno::Reference< css::beans::XPropertySetInfoChangeListener >& Listener ) override;
    virtual void
    removePropertySetInfoChangeListener( const cpo::uno::Reference< css::beans::XPropertySetInfoChangeListener >& Listener ) override;

    // XPropertyAccess
    virtual cpo::uno::Sequence< css::beans::PropertyValue >
    getPropertyValues() override;
    virtual void
    setPropertyValues( const cpo::uno::Sequence< css::beans::PropertyValue >& aProps ) override;

    // Non-interface methods.
    PropertySetRegistry& getPropertySetRegistry();
    OUString getFullKey();
private:
    const OUString& getFullKeyImpl(std::unique_lock<std::mutex>&);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
