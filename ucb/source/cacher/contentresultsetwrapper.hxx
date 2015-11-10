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

#ifndef INCLUDED_UCB_SOURCE_CACHER_CONTENTRESULTSETWRAPPER_HXX
#define INCLUDED_UCB_SOURCE_CACHER_CONTENTRESULTSETWRAPPER_HXX

#include <rtl/ustring.hxx>
#include <ucbhelper/macros.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/interfacecontainer.hxx>



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
    typedef cppu::OMultiTypeInterfaceContainerHelperVar<OUString>
        PropertyChangeListenerContainer_Impl;

    class ReacquireableGuard
    {
    protected:
        osl::Mutex* pT;
    public:

        ReacquireableGuard(osl::Mutex * t) : pT(t)
        {
            pT->acquire();
        }

        ReacquireableGuard(osl::Mutex& t) : pT(&t)
        {
            pT->acquire();
        }

        /** Releases mutex. */
        ~ReacquireableGuard()
        {
            if (pT)
                pT->release();
        }

        /** Releases mutex. */
        void clear()
        {
            if(pT)
            {
                pT->release();
                pT = nullptr;
            }
        }

        /** Reacquire mutex. */
        void reacquire()
        {
            if(pT)
            {
                pT->acquire();
            }
        }
    };


    //members

    //my Mutex
    osl::Mutex              m_aMutex;

    //different Interfaces from Origin:
    css::uno::Reference< css::sdbc::XResultSet >
                            m_xResultSetOrigin;
    css::uno::Reference< css::sdbc::XRow >
                            m_xRowOrigin; //XRow-interface from m_xOrigin
                            //!! call impl_init_xRowOrigin() bevor you access this member
    css::uno::Reference< css::ucb::XContentAccess >
                            m_xContentAccessOrigin; //XContentAccess-interface from m_xOrigin
                            //!! call impl_init_xContentAccessOrigin() bevor you access this member
    css::uno::Reference< css::beans::XPropertySet >
                            m_xPropertySetOrigin; //XPropertySet-interface from m_xOrigin
                            //!! call impl_init_xPropertySetOrigin() bevor you access this member

    css::uno::Reference< css::beans::XPropertySetInfo >
                            m_xPropertySetInfo;
                            //call impl_initPropertySetInfo() bevor you access this member

    sal_Int32               m_nForwardOnly;

private:
    css::uno::Reference< css::beans::XPropertyChangeListener >
                            m_xMyListenerImpl;
    ContentResultSetWrapperListener*
                            m_pMyListenerImpl;

    css::uno::Reference< css::sdbc::XResultSetMetaData >
                            m_xMetaDataFromOrigin; //XResultSetMetaData from m_xOrigin

    //management of listeners
    bool                m_bDisposed; ///Dispose call ready.
    bool                m_bInDispose;///In dispose call
    osl::Mutex              m_aContainerMutex;
    cppu::OInterfaceContainerHelper*
                            m_pDisposeEventListeners;
    PropertyChangeListenerContainer_Impl*
                            m_pPropertyChangeListeners;
    PropertyChangeListenerContainer_Impl*
                            m_pVetoableChangeListeners;


    //methods:
private:
    PropertyChangeListenerContainer_Impl* SAL_CALL
    impl_getPropertyChangeListenerContainer();

    PropertyChangeListenerContainer_Impl* SAL_CALL
    impl_getVetoableChangeListenerContainer();

protected:


    ContentResultSetWrapper( css::uno::Reference< css::sdbc::XResultSet > xOrigin );

    virtual ~ContentResultSetWrapper();

    void SAL_CALL impl_init();
    void SAL_CALL impl_deinit();

    //--

    void SAL_CALL impl_init_xRowOrigin();
    void SAL_CALL impl_init_xContentAccessOrigin();
    void SAL_CALL impl_init_xPropertySetOrigin();

    //--

    virtual void SAL_CALL impl_initPropertySetInfo(); //helping XPropertySet

    void SAL_CALL
    impl_EnsureNotDisposed()
        throw( css::lang::DisposedException,
               css::uno::RuntimeException );

    void SAL_CALL
    impl_notifyPropertyChangeListeners(
            const css::beans::PropertyChangeEvent& rEvt );

    void SAL_CALL
    impl_notifyVetoableChangeListeners(
            const css::beans::PropertyChangeEvent& rEvt )
            throw( css::beans::PropertyVetoException,
                   css::uno::RuntimeException );

    bool SAL_CALL impl_isForwardOnly();

public:


    // XInterface

    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;


    // XComponent

    virtual void SAL_CALL
    dispose() throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
            throw( css::uno::RuntimeException, std::exception ) override;


    //XCloseable

    virtual void SAL_CALL
    close()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;


    //XResultSetMetaDataSupplier

    virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL
    getMetaData()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;


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


    // own methods

    virtual void SAL_CALL
        impl_disposing( const css::lang::EventObject& Source )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL
    impl_propertyChange( const css::beans::PropertyChangeEvent& evt )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL
    impl_vetoableChange( const css::beans::PropertyChangeEvent& aEvent )
        throw( css::beans::PropertyVetoException,
               css::uno::RuntimeException );


    // XContentAccess

    virtual OUString SAL_CALL
    queryContentIdentifierString()
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
    queryContentIdentifier()
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    queryContent()
        throw( css::uno::RuntimeException, std::exception ) override;


    // XResultSet

    virtual sal_Bool SAL_CALL
    next()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    isBeforeFirst()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    isAfterLast()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    isFirst()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    isLast()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    beforeFirst()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    afterLast()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    first()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    last()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL
    getRow()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    absolute( sal_Int32 row )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    relative( sal_Int32 rows )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    previous()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    refreshRow()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    rowUpdated()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    rowInserted()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    rowDeleted()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::uno::XInterface > SAL_CALL
    getStatement()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;


    // XRow

    virtual sal_Bool SAL_CALL
    wasNull()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL
    getString( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
    getBoolean( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int8 SAL_CALL
    getByte( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int16 SAL_CALL
    getShort( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
    getInt( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int64 SAL_CALL
    getLong( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual float SAL_CALL
    getFloat( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual double SAL_CALL
    getDouble( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getBytes( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::util::Date SAL_CALL
    getDate( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::util::Time SAL_CALL
    getTime( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::util::DateTime SAL_CALL
    getTimestamp( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const css::uno::Reference< css::container::XNameAccess >& typeMap )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
};



class ContentResultSetWrapperListener
        : public cppu::OWeakObject
        , public css::beans::XPropertyChangeListener
        , public css::beans::XVetoableChangeListener
{
protected:
    ContentResultSetWrapper*    m_pOwner;

public:
    ContentResultSetWrapperListener( ContentResultSetWrapper* pOwner );

    virtual ~ContentResultSetWrapperListener();


    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    //XEventListener

    virtual void SAL_CALL
        disposing( const css::lang::EventObject& Source )
        throw( css::uno::RuntimeException, std::exception ) override;


    //XPropertyChangeListener

    virtual void SAL_CALL
    propertyChange( const css::beans::PropertyChangeEvent& evt )
        throw( css::uno::RuntimeException, std::exception ) override;


    //XVetoableChangeListener

    virtual void SAL_CALL
    vetoableChange( const css::beans::PropertyChangeEvent& aEvent )
        throw( css::beans::PropertyVetoException,
               css::uno::RuntimeException, std::exception ) override;


    // own methods:
    void SAL_CALL impl_OwnerDies();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
