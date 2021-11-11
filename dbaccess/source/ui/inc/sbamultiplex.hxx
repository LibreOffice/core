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

#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/form/XDatabaseParameterListener.hpp>
#include <com/sun/star/form/XLoadListener.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/form/XSubmitListener.hpp>
#include <com/sun/star/form/XResetListener.hpp>
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
#include <com/sun/star/sdb/XRowSetApproveListener.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <comphelper/uno3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/multiinterfacecontainer2.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>

namespace dbaui
{
    // TODO : replace this class if MM provides a WeakSubObject in cppu
    class OSbaWeakSubObject : public ::cppu::OWeakObject
    {
    protected:
        ::cppu::OWeakObject&    m_rParent;

    public:
        OSbaWeakSubObject(::cppu::OWeakObject& rParent) : m_rParent(rParent) { }

        virtual void SAL_CALL acquire() noexcept override { m_rParent.acquire(); }
        virtual void SAL_CALL release() noexcept override { m_rParent.release(); }
    };

    // declaration of a listener multiplexer class
    #define BEGIN_DECLARE_LISTENER_MULTIPLEXER(classname, listenerclass)                    \
    class classname                                                                         \
            :public OSbaWeakSubObject                                                           \
            ,public listenerclass                                                           \
            ,public ::comphelper::OInterfaceContainerHelper2                                       \
    {                                                                                       \
    public:                                                                                 \
        classname( ::cppu::OWeakObject& rSource,                                            \
            ::osl::Mutex& rMutex);                                                          \
        DECLARE_UNO3_DEFAULTS(classname, OSbaWeakSubObject)                                     \
        virtual css::uno::Any  SAL_CALL queryInterface(                        \
            const css::uno::Type& _rType) override; \
                                                                                            \
        /* css::lang::XEventListener */                                        \
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;  \

    #define DECLARE_MULTIPLEXER_VOID_METHOD(methodname, eventtype)                          \
        virtual void SAL_CALL methodname(const eventtype& e) override; \

    #define DECLARE_MULTIPLEXER_BOOL_METHOD(methodname, eventtype)                          \
        virtual sal_Bool SAL_CALL methodname(const eventtype& e) override;   \

    #define END_DECLARE_LISTENER_MULTIPLEXER()                                               \
        /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators */    \
        using OSbaWeakSubObject::operator new;   \
        using OSbaWeakSubObject::operator delete;    \
    };                                                                                      \

    // implementation of a listener multiplexer class

    #define IMPLEMENT_LISTENER_MULTIPLEXER_CORE(classname, listenerclass)                   \
                                                                                            \
    classname::classname(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)               \
            :OSbaWeakSubObject(rSource)                                                     \
            ,OInterfaceContainerHelper2(_rMutex)                                             \
    {                                                                                       \
    }                                                                                       \
                                                                                            \
    css::uno::Any  SAL_CALL classname::queryInterface(                         \
        const css::uno::Type& _rType) \
    {                                                                                       \
        css::uno::Any aReturn =                                                \
            OSbaWeakSubObject::queryInterface(_rType);                                          \
        if (!aReturn.hasValue())                                                            \
            aReturn = ::cppu::queryInterface(_rType,                                        \
                static_cast< listenerclass* >(this),                                        \
                static_cast< css::lang::XEventListener* >(static_cast< listenerclass* >(this)) \
            );                                                                              \
                                                                                            \
        return aReturn;                                                                     \
    }                                                                                       \
    void SAL_CALL classname::disposing(const css::lang::EventObject& )\
    {                                                                                       \
    }                                                                                       \


    #define STOP_MULTIPLEXER_LISTENING(listenerdesc, multiplexer, broadcasterclass, broadcaster) \
    if (multiplexer.getLength())                                                            \
    {                                                                                   \
        css::uno::Reference< broadcasterclass > xBroadcaster(broadcaster, css::uno::UNO_QUERY);   \
        if (xBroadcaster.is())                                                          \
            xBroadcaster->remove##listenerdesc(&multiplexer);                           \
    }                                                                                   \

    #define START_MULTIPLEXER_LISTENING(listenerdesc, multiplexer, broadcasterclass, broadcaster) \
    if (multiplexer.getLength())                                                        \
    {                                                                                   \
        css::uno::Reference< broadcasterclass > xBroadcaster(broadcaster, css::uno::UNO_QUERY);   \
        if (xBroadcaster.is())                                                          \
            xBroadcaster->add##listenerdesc(&multiplexer);                              \
    }                                                                                   \

    #define STOP_PROPERTY_MULTIPLEXER_LISTENING(listenerdesc, multiplexer, broadcasterclass, broadcaster) \
    if (multiplexer.getOverallLen())                                                        \
    {                                                                                       \
        css::uno::Reference< broadcasterclass > xBroadcaster(broadcaster, css::uno::UNO_QUERY);   \
        if (xBroadcaster.is())                                                              \
            xBroadcaster->remove##listenerdesc(OUString(), &multiplexer);                            \
    }                                                                                       \

    #define START_PROPERTY_MULTIPLEXER_LISTENING(listenerdesc, multiplexer, broadcasterclass, broadcaster) \
    if (multiplexer.getOverallLen())                                                        \
    {                                                                                       \
        css::uno::Reference< broadcasterclass > xBroadcaster(broadcaster, css::uno::UNO_QUERY);   \
        if (xBroadcaster.is())                                                              \
            xBroadcaster->add##listenerdesc(OUString(), &multiplexer);                               \
    }                                                                                       \

    // some listener multiplexers
    // css::frame::XStatusListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXStatusMultiplexer, css::frame::XStatusListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(statusChanged, css::frame::FeatureStateEvent)

    private:
        css::frame::FeatureStateEvent  m_aLastKnownStatus;
    public:                                                                                 \
        const css::frame::FeatureStateEvent& getLastEvent( ) const { return m_aLastKnownStatus; }
    END_DECLARE_LISTENER_MULTIPLEXER()

    // css::form::XLoadListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXLoadMultiplexer, css::form::XLoadListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(loaded, css::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(unloaded, css::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(unloading, css::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(reloading, css::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(reloaded, css::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // css::form::XDatabaseParameterListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXParameterMultiplexer, css::form::XDatabaseParameterListener)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveParameter, css::form::DatabaseParameterEvent)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // css::form::XSubmitListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXSubmitMultiplexer, css::form::XSubmitListener)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveSubmit, css::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // css::form::XResetListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXResetMultiplexer, css::form::XResetListener)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveReset, css::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(resetted, css::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // css::sdbc::XRowSetListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXRowSetMultiplexer, css::sdbc::XRowSetListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(cursorMoved, css::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(rowChanged, css::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(rowSetChanged, css::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // css::sdb::XRowSetApproveListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXRowSetApproveMultiplexer, css::sdb::XRowSetApproveListener)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveCursorMove, css::lang::EventObject)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveRowChange, css::sdb::RowChangeEvent)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveRowSetChange, css::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // css::sdb::XSQLErrorListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXSQLErrorMultiplexer, css::sdb::XSQLErrorListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(errorOccured, css::sdb::SQLErrorEvent)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // css::beans::XPropertyChangeListener
    class SbaXPropertyChangeMultiplexer final
            :public OSbaWeakSubObject
            ,public css::beans::XPropertyChangeListener
    {
        typedef ::comphelper::OMultiTypeInterfaceContainerHelperVar2<OUString >  ListenerContainerMap;
        ListenerContainerMap    m_aListeners;

    public:
        SbaXPropertyChangeMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
        DECLARE_UNO3_DEFAULTS(SbaXPropertyChangeMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& e) override;

        void addInterface(const OUString& rName, const css::uno::Reference< css::uno::XInterface >& rListener);
        void removeInterface(const OUString& rName, const css::uno::Reference< css::uno::XInterface >& rListener);

        void disposeAndClear();

        sal_Int32 getOverallLen() const;

        ::comphelper::OInterfaceContainerHelper2* getContainer(const OUString& rName)
            { return m_aListeners.getContainer(rName); }

    private:
        void Notify(::comphelper::OInterfaceContainerHelper2& rListeners, const css::beans::PropertyChangeEvent& e);
    };

    // css::beans::XVetoableChangeListener
    class SbaXVetoableChangeMultiplexer final
            :public OSbaWeakSubObject
            ,public css::beans::XVetoableChangeListener
    {
        typedef ::comphelper::OMultiTypeInterfaceContainerHelperVar2<OUString >  ListenerContainerMap;
        ListenerContainerMap    m_aListeners;

    public:
        SbaXVetoableChangeMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
        DECLARE_UNO3_DEFAULTS(SbaXVetoableChangeMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

        virtual void SAL_CALL vetoableChange(const css::beans::PropertyChangeEvent& e) override;

        void addInterface(const OUString& rName, const css::uno::Reference< css::uno::XInterface >& rListener);
        void removeInterface(const OUString& rName, const css::uno::Reference< css::uno::XInterface >& rListener);

        void disposeAndClear();

        sal_Int32 getOverallLen() const;

        ::comphelper::OInterfaceContainerHelper2* getContainer(const OUString& rName)
            { return m_aListeners.getContainer(rName); }

    private:
        void Notify(::comphelper::OInterfaceContainerHelper2& rListeners, const css::beans::PropertyChangeEvent& e);
    };

    // css::beans::XPropertiesChangeListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXPropertiesChangeMultiplexer, css::beans::XPropertiesChangeListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(propertiesChange, css::uno::Sequence< css::beans::PropertyChangeEvent >)
    END_DECLARE_LISTENER_MULTIPLEXER()
    // the SbaXPropertiesChangeMultiplexer doesn't care about the property names a listener logs on for, it simply
    // forwards _all_ changes to _all_ listeners
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
