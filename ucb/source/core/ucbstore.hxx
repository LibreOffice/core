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

#ifndef INCLUDED_UCB_SOURCE_CORE_UCBSTORE_HXX
#define INCLUDED_UCB_SOURCE_CORE_UCBSTORE_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ucb/XPropertySetRegistryFactory.hpp>
#include <com/sun/star/ucb/XPropertySetRegistry.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertySetInfoChangeNotifier.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <cppuhelper/implbase.hxx>
#include <memory>


#define STORE_SERVICE_NAME          "com.sun.star.ucb.Store"
#define PROPSET_REG_SERVICE_NAME    "com.sun.star.ucb.PropertySetRegistry"
#define PERS_PROPSET_SERVICE_NAME   "com.sun.star.ucb.PersistentPropertySet"



struct UcbStore_Impl;

class UcbStore : public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::ucb::XPropertySetRegistryFactory,
    css::lang::XInitialization >
{
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    std::unique_ptr<UcbStore_Impl> m_pImpl;

public:
    explicit UcbStore( const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~UcbStore();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    static css::uno::Reference< css::lang::XSingleServiceFactory >
    createServiceFactory( const css::uno::Reference<
                          css::lang::XMultiServiceFactory >& rxServiceMgr );

    // XPropertySetRegistryFactory
    virtual css::uno::Reference< css::ucb::XPropertySetRegistry > SAL_CALL
    createPropertySetRegistry( const OUString& URL )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL
    initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw( css::uno::Exception,
               css::uno::RuntimeException, std::exception ) override;

    const css::uno::Sequence< css::uno::Any >&
    getInitArgs() const;
};



struct PropertySetRegistry_Impl;
class PersistentPropertySet;

class PropertySetRegistry : public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::ucb::XPropertySetRegistry,
    css::container::XNameAccess >
{
    friend class PersistentPropertySet;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    std::unique_ptr<PropertySetRegistry_Impl> m_pImpl;

private:
    css::uno::Reference< css::lang::XMultiServiceFactory >
    getConfigProvider();

    void add   ( PersistentPropertySet* pSet );
    void remove( PersistentPropertySet* pSet );

    void renamePropertySet( const OUString& rOldKey,
                            const OUString& rNewKey );

public:
    PropertySetRegistry(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Sequence< css::uno::Any >& rInitArgs);
    virtual ~PropertySetRegistry();


    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // XPropertySetRegistry
    virtual css::uno::Reference< css::ucb::XPersistentPropertySet > SAL_CALL
    openPropertySet( const OUString& key, sal_Bool create )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removePropertySet( const OUString& key )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XElementAccess ( XNameAccess is derived from it )
    virtual css::uno::Type SAL_CALL
    getElementType()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    hasElements()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL
    getByName( const OUString& aName )
        throw( css::container::NoSuchElementException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
    getElementNames()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    hasByName( const OUString& aName )
        throw( css::uno::RuntimeException, std::exception ) override;

    // Non-interface methods
    css::uno::Reference< css::uno::XInterface >
    getRootConfigReadAccess();
    css::uno::Reference< css::uno::XInterface >
    getConfigWriteAccess( const OUString& rPath );
};



struct PersistentPropertySet_Impl;

class PersistentPropertySet : public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::lang::XComponent,
    css::ucb::XPersistentPropertySet,
    css::container::XNamed,
    css::beans::XPropertyContainer,
    css::beans::XPropertySetInfoChangeNotifier,
    css::beans::XPropertyAccess >
{
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    std::unique_ptr<PersistentPropertySet_Impl> m_pImpl;

private:
    void notifyPropertyChangeEvent(
        const css::beans::PropertyChangeEvent& rEvent ) const;
    void notifyPropertySetInfoChange(
        const css::beans::PropertySetInfoChangeEvent& evt ) const;

public:
    PersistentPropertySet(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        PropertySetRegistry& rCreator,
        const OUString& rKey );
    virtual ~PersistentPropertySet();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // XComponent
    virtual void SAL_CALL
    dispose()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    setPropertyValue( const OUString& aPropertyName,
                      const css::uno::Any& aValue )
        throw( css::beans::UnknownPropertyException,
               css::beans::PropertyVetoException,
               css::lang::IllegalArgumentException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL
    getPropertyValue( const OUString& PropertyName )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    addPropertyChangeListener( const OUString& aPropertyName,
                               const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removePropertyChangeListener( const OUString& aPropertyName,
                                  const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    addVetoableChangeListener( const OUString& PropertyName,
                               const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removeVetoableChangeListener( const OUString& PropertyName,
                                  const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw( css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;

    // XPersistentPropertySet
    virtual css::uno::Reference< css::ucb::XPropertySetRegistry > SAL_CALL
    getRegistry()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL
    getKey()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XNamed
    virtual OUString SAL_CALL
    getName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    setName( const OUString& aName )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XPropertyContainer
    virtual void SAL_CALL
    addProperty( const OUString& Name,
                 sal_Int16 Attributes,
                 const css::uno::Any& DefaultValue )
        throw( css::beans::PropertyExistException,
               css::beans::IllegalTypeException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removeProperty( const OUString& Name )
        throw( css::beans::UnknownPropertyException,
               css::beans::NotRemoveableException,
               css::uno::RuntimeException, std::exception ) override;

    // XPropertySetInfoChangeNotifier
    virtual void SAL_CALL
    addPropertySetInfoChangeListener( const css::uno::Reference< css::beans::XPropertySetInfoChangeListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removePropertySetInfoChangeListener( const css::uno::Reference< css::beans::XPropertySetInfoChangeListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XPropertyAccess
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
    getPropertyValues()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& aProps )
        throw( css::beans::UnknownPropertyException,
               css::beans::PropertyVetoException,
               css::lang::IllegalArgumentException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception ) override;

    // Non-interface methods.
    PropertySetRegistry& getPropertySetRegistry();
    const OUString& getFullKey();
};

#endif // INCLUDED_UCB_SOURCE_CORE_UCBSTORE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
