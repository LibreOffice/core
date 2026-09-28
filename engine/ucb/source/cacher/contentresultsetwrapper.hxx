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

#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/multiinterfacecontainer4.hxx>
#include <memory>


class ContentResultSetWrapperListener;
class ContentResultSetWrapper
                : public cppu::OWeakObject
                , public css::lang::XComponent
                , public css::sdbc::XCloseable
                , public css::sdbc::XResultSetMetaDataSupplier
                , public css::beans::XPropertySet
                , public css::ucb::XContentAccess
                , public css::sdbc::XResultSet
                , public css::sdbc::XRow
{
protected:
    typedef comphelper::OMultiTypeInterfaceContainerHelperVar4<OUString, css::beans::XPropertyChangeListener>
        PropertyChangeListenerContainer_Impl;
    typedef comphelper::OMultiTypeInterfaceContainerHelperVar4<OUString, css::beans::XVetoableChangeListener>
        VetoableChangeListenerContainer_Impl;

    //members

    //my Mutex
    std::mutex              m_aMutex;

    //different Interfaces from Origin:
    cpo::uno::Reference< css::sdbc::XResultSet >
                            m_xResultSetOrigin;
    cpo::uno::Reference< css::sdbc::XRow >
                            m_xRowOrigin; //XRow-interface from m_xOrigin
                            //!! call impl_init_xRowOrigin() bevor you access this member
    cpo::uno::Reference< css::ucb::XContentAccess >
                            m_xContentAccessOrigin; //XContentAccess-interface from m_xOrigin
                            //!! call impl_init_xContentAccessOrigin() bevor you access this member
    cpo::uno::Reference< css::beans::XPropertySet >
                            m_xPropertySetOrigin; //XPropertySet-interface from m_xOrigin
                            //!! call impl_init_xPropertySetOrigin() bevor you access this member

    cpo::uno::Reference< css::beans::XPropertySetInfo >
                            m_xPropertySetInfo;
                            //call impl_initPropertySetInfo() bevor you access this member

    sal_Int32               m_nForwardOnly;

private:
    rtl::Reference<ContentResultSetWrapperListener>
                            m_xMyListenerImpl;

    cpo::uno::Reference< css::sdbc::XResultSetMetaData >
                            m_xMetaDataFromOrigin; //XResultSetMetaData from m_xOrigin

    //management of listeners
    bool                m_bDisposed; ///Dispose call ready.
    bool                m_bInDispose;///In dispose call
    comphelper::OInterfaceContainerHelper4<css::lang::XEventListener>
                            m_aDisposeEventListeners;
    PropertyChangeListenerContainer_Impl
                            m_aPropertyChangeListeners;
    VetoableChangeListenerContainer_Impl
                            m_aVetoableChangeListeners;


    //methods:
private:
    void verifyGet();

protected:


    ContentResultSetWrapper( cpo::uno::Reference< css::sdbc::XResultSet > const & xOrigin );

    virtual ~ContentResultSetWrapper() override;

    void impl_init();
    void impl_deinit();

    //--

    void impl_init_xRowOrigin(std::unique_lock<std::mutex>&);
    void impl_init_xContentAccessOrigin(std::unique_lock<std::mutex>&);
    void impl_init_xPropertySetOrigin(std::unique_lock<std::mutex>&);

    //--

    virtual void impl_initPropertySetInfo(std::unique_lock<std::mutex>& rGuard); //helping XPropertySet

    /// @throws css::lang::DisposedException
    /// @throws cpo::uno::RuntimeException
    void
    impl_EnsureNotDisposed(std::unique_lock<std::mutex>& rGuard);

    void
    impl_notifyPropertyChangeListeners(
            std::unique_lock<std::mutex>& rGuard,
            const css::beans::PropertyChangeEvent& rEvt );

    /// @throws css::beans::PropertyVetoException
    /// @throws cpo::uno::RuntimeException
    void
    impl_notifyVetoableChangeListeners(
            std::unique_lock<std::mutex>& rGuard,
            const css::beans::PropertyChangeEvent& rEvt );

    bool impl_isForwardOnly(std::unique_lock<std::mutex>& rGuard);

public:


    // XInterface

    virtual cpo::uno::Any
    queryInterface( const cpo::uno::Type & rType ) override;


    // XComponent

    virtual void
    dispose() override final;

    virtual void
    addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& Listener ) override;

    virtual void
    removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& Listener ) override;


    //XCloseable

    virtual void
    close() override;


    //XResultSetMetaDataSupplier

    virtual cpo::uno::Reference< css::sdbc::XResultSetMetaData >
    getMetaData() override;


    // XPropertySet

    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
    getPropertySetInfo() override final;
    const cpo::uno::Reference< css::beans::XPropertySetInfo > &
    getPropertySetInfoImpl(std::unique_lock<std::mutex>& rGuard);

    virtual void
    setPropertyValue( const OUString& aPropertyName,
                      const cpo::uno::Any& aValue ) final override;
    virtual void
    setPropertyValueImpl( std::unique_lock<std::mutex>& rGuard, const OUString& aPropertyName,
                      const cpo::uno::Any& aValue );

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


    // own methods

    /// @throws cpo::uno::RuntimeException
    virtual void
        impl_disposing( const css::lang::EventObject& Source );

    /// @throws cpo::uno::RuntimeException
    virtual void
    impl_propertyChange( const css::beans::PropertyChangeEvent& evt );

    /// @throws css::beans::PropertyVetoException
    /// @throws cpo::uno::RuntimeException
    virtual void
    impl_vetoableChange( const css::beans::PropertyChangeEvent& aEvent );


    // XContentAccess

    virtual OUString
    queryContentIdentifierString() override final;
    virtual OUString
    queryContentIdentifierStringImpl(std::unique_lock<std::mutex>& rGuard);

    virtual cpo::uno::Reference< css::ucb::XContentIdentifier >
    queryContentIdentifier() override final;
    virtual cpo::uno::Reference< css::ucb::XContentIdentifier >
    queryContentIdentifierImpl(std::unique_lock<std::mutex>& rGuard);

    virtual cpo::uno::Reference< css::ucb::XContent >
    queryContent() override final;
    virtual cpo::uno::Reference<css::ucb::XContent>
    queryContentImpl(std::unique_lock<std::mutex>& rGuard);


    // XResultSet

    virtual bool
    next() override;
    virtual bool
    isBeforeFirst() override;
    virtual bool
    isAfterLast() override;
    virtual bool
    isFirst() override;
    virtual bool
    isLast() override;
    virtual void
    beforeFirst() override;
    virtual void
    afterLast() override;
    virtual bool
    first() override;
    virtual bool
    last() override;
    virtual sal_Int32
    getRow() override;
    virtual bool
    absolute( sal_Int32 row ) override;
    virtual bool
    relative( sal_Int32 rows ) override;
    virtual bool
    previous() override;
    virtual void
    refreshRow() override;
    virtual bool
    rowUpdated() override;
    virtual bool
    rowInserted() override;
    virtual bool
    rowDeleted() override;
    virtual cpo::uno::Reference<
                cpo::uno::XInterface >
    getStatement() override;


    // XRow

    virtual bool
    wasNull() override;

    virtual OUString
    getString( sal_Int32 columnIndex ) override;

    virtual bool
    getBoolean( sal_Int32 columnIndex ) override;

    virtual sal_Int8
    getByte( sal_Int32 columnIndex ) override;

    virtual sal_Int16
    getShort( sal_Int32 columnIndex ) override;

    virtual sal_Int32
    getInt( sal_Int32 columnIndex ) override;

    virtual sal_Int64
    getLong( sal_Int32 columnIndex ) override;

    virtual float
    getFloat( sal_Int32 columnIndex ) override;

    virtual double
    getDouble( sal_Int32 columnIndex ) override;

    virtual cpo::uno::Sequence< sal_Int8 >
    getBytes( sal_Int32 columnIndex ) override;

    virtual css::util::Date
    getDate( sal_Int32 columnIndex ) override;

    virtual css::util::Time
    getTime( sal_Int32 columnIndex ) override;

    virtual css::util::DateTime
    getTimestamp( sal_Int32 columnIndex ) override;

    virtual cpo::uno::Reference< css::io::XInputStream >
    getBinaryStream( sal_Int32 columnIndex ) override;

    virtual cpo::uno::Reference< css::io::XInputStream >
    getCharacterStream( sal_Int32 columnIndex ) override;

    virtual cpo::uno::Any
    getObject( sal_Int32 columnIndex,
               const cpo::uno::Reference< css::container::XNameAccess >& typeMap ) override;

    virtual cpo::uno::Reference< css::sdbc::XRef >
    getRef( sal_Int32 columnIndex ) override;

    virtual cpo::uno::Reference< css::sdbc::XBlob >
    getBlob( sal_Int32 columnIndex ) override;

    virtual cpo::uno::Reference< css::sdbc::XClob >
    getClob( sal_Int32 columnIndex ) override;

    virtual cpo::uno::Reference< css::sdbc::XArray >
    getArray( sal_Int32 columnIndex ) override;
};


class ContentResultSetWrapperListener
        : public cppu::OWeakObject
        , public css::beans::XPropertyChangeListener
        , public css::beans::XVetoableChangeListener
{
    ContentResultSetWrapper*    m_pOwner;

public:
    ContentResultSetWrapperListener( ContentResultSetWrapper* pOwner );

    virtual ~ContentResultSetWrapperListener() override;


    // XInterface
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
    virtual void acquire()
        noexcept override;
    virtual void release()
        noexcept override;

    //XEventListener

    virtual void
        disposing( const css::lang::EventObject& Source ) override;


    //XPropertyChangeListener

    virtual void
    propertyChange( const css::beans::PropertyChangeEvent& evt ) override;


    //XVetoableChangeListener

    virtual void
    vetoableChange( const css::beans::PropertyChangeEvent& aEvent ) override;


    // own methods:
    void impl_OwnerDies();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
