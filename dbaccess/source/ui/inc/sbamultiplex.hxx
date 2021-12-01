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
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/multiinterfacecontainer2.hxx>
#include <comphelper/multiinterfacecontainer3.hxx>
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

    // some listener multiplexers
    // css::frame::XStatusListener
    class SbaXStatusMultiplexer
            :public OSbaWeakSubObject
            ,public css::frame::XStatusListener
            ,public ::comphelper::OInterfaceContainerHelper3<css::frame::XStatusListener>
    {
    public:
        SbaXStatusMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
        DECLARE_UNO3_DEFAULTS(SbaXStatusMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

        virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& e) override;

    private:
        css::frame::FeatureStateEvent  m_aLastKnownStatus;
    public:
        const css::frame::FeatureStateEvent& getLastEvent( ) const { return m_aLastKnownStatus; }
        /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators */
        using OSbaWeakSubObject::operator new;
        using OSbaWeakSubObject::operator delete;
    };

    // css::form::XLoadListener
    class SbaXLoadMultiplexer
            :public OSbaWeakSubObject
            ,public css::form::XLoadListener
            ,public ::comphelper::OInterfaceContainerHelper3<css::form::XLoadListener>
    {
    public:
        SbaXLoadMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
        DECLARE_UNO3_DEFAULTS(SbaXLoadMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

        virtual void SAL_CALL loaded(const css::lang::EventObject& e) override;
        virtual void SAL_CALL unloaded(const css::lang::EventObject& e) override;
        virtual void SAL_CALL unloading(const css::lang::EventObject& e) override;
        virtual void SAL_CALL reloading(const css::lang::EventObject& e) override;
        virtual void SAL_CALL reloaded(const css::lang::EventObject& e) override;
        /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators */
        using OSbaWeakSubObject::operator new;
        using OSbaWeakSubObject::operator delete;
    };

    // css::form::XDatabaseParameterListener
    class SbaXParameterMultiplexer
            :public OSbaWeakSubObject
            ,public css::form::XDatabaseParameterListener
            ,public ::comphelper::OInterfaceContainerHelper3<css::form::XDatabaseParameterListener>
    {
    public:
        SbaXParameterMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
        DECLARE_UNO3_DEFAULTS(SbaXParameterMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;
        virtual sal_Bool SAL_CALL approveParameter(const css::form::DatabaseParameterEvent& e) override;
        /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators */
        using OSbaWeakSubObject::operator new;
        using OSbaWeakSubObject::operator delete;
    };

    // css::form::XSubmitListener
    class SbaXSubmitMultiplexer
            :public OSbaWeakSubObject
            ,public css::form::XSubmitListener
            ,public ::comphelper::OInterfaceContainerHelper3<css::form::XSubmitListener>
    {
    public:
        SbaXSubmitMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
        DECLARE_UNO3_DEFAULTS(SbaXSubmitMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;
        virtual sal_Bool SAL_CALL approveSubmit(const css::lang::EventObject& e) override;
        /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators */
        using OSbaWeakSubObject::operator new;
        using OSbaWeakSubObject::operator delete;
    };

    // css::form::XResetListener
    class SbaXResetMultiplexer
            :public OSbaWeakSubObject
            ,public css::form::XResetListener
            ,public ::comphelper::OInterfaceContainerHelper3<css::form::XResetListener>
    {
    public:
        SbaXResetMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
        DECLARE_UNO3_DEFAULTS(SbaXResetMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;
        virtual sal_Bool SAL_CALL approveReset(const css::lang::EventObject& e) override;
        virtual void SAL_CALL resetted(const css::lang::EventObject& e) override;
        /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators */
        using OSbaWeakSubObject::operator new;
        using OSbaWeakSubObject::operator delete;
    };

    // css::sdbc::XRowSetListener
    class SbaXRowSetMultiplexer
            :public OSbaWeakSubObject
            ,public css::sdbc::XRowSetListener
            ,public ::comphelper::OInterfaceContainerHelper3<css::sdbc::XRowSetListener>
    {
    public:
        SbaXRowSetMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
        DECLARE_UNO3_DEFAULTS(SbaXRowSetMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;
        virtual void SAL_CALL cursorMoved(const css::lang::EventObject& e) override;
        virtual void SAL_CALL rowChanged(const css::lang::EventObject& e) override;
        virtual void SAL_CALL rowSetChanged(const css::lang::EventObject& e) override;
        /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators */
        using OSbaWeakSubObject::operator new;
        using OSbaWeakSubObject::operator delete;
    };

    // css::sdb::XRowSetApproveListener
    class SbaXRowSetApproveMultiplexer
            :public OSbaWeakSubObject
            ,public css::sdb::XRowSetApproveListener
            ,public ::comphelper::OInterfaceContainerHelper3<css::sdb::XRowSetApproveListener>
    {
    public:
        SbaXRowSetApproveMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
        DECLARE_UNO3_DEFAULTS(SbaXRowSetApproveMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;
        virtual sal_Bool SAL_CALL approveCursorMove(const css::lang::EventObject& e) override;
        virtual sal_Bool SAL_CALL approveRowChange(const css::sdb::RowChangeEvent& e) override;
        virtual sal_Bool SAL_CALL approveRowSetChange(const css::lang::EventObject& e) override;
        /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators */
        using OSbaWeakSubObject::operator new;
        using OSbaWeakSubObject::operator delete;
    };

    // css::sdb::XSQLErrorListener
    class SbaXSQLErrorMultiplexer
            :public OSbaWeakSubObject
            ,public css::sdb::XSQLErrorListener
            ,public ::comphelper::OInterfaceContainerHelper3<css::sdb::XSQLErrorListener>
    {
    public:
        SbaXSQLErrorMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
        DECLARE_UNO3_DEFAULTS(SbaXSQLErrorMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;
        virtual void SAL_CALL errorOccured(const css::sdb::SQLErrorEvent& e) override;
        /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators */
        using OSbaWeakSubObject::operator new;
        using OSbaWeakSubObject::operator delete;
    };

    // css::beans::XPropertyChangeListener
    class SbaXPropertyChangeMultiplexer final
            :public OSbaWeakSubObject
            ,public css::beans::XPropertyChangeListener
    {
        typedef ::comphelper::OMultiTypeInterfaceContainerHelperVar3<css::beans::XPropertyChangeListener, OUString>  ListenerContainerMap;
        ListenerContainerMap    m_aListeners;

    public:
        SbaXPropertyChangeMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
        DECLARE_UNO3_DEFAULTS(SbaXPropertyChangeMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& e) override;

        void addInterface(const OUString& rName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rListener);
        void removeInterface(const OUString& rName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rListener);

        void disposeAndClear();

        sal_Int32 getOverallLen() const;

        ::comphelper::OInterfaceContainerHelper3<css::beans::XPropertyChangeListener>* getContainer(const OUString& rName)
            { return m_aListeners.getContainer(rName); }

    private:
        void Notify(::comphelper::OInterfaceContainerHelper3<css::beans::XPropertyChangeListener>& rListeners, const css::beans::PropertyChangeEvent& e);
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
    class SbaXPropertiesChangeMultiplexer
            :public OSbaWeakSubObject
            ,public css::beans::XPropertiesChangeListener
            ,public ::comphelper::OInterfaceContainerHelper3<css::beans::XPropertiesChangeListener>
    {
    public:
        SbaXPropertiesChangeMultiplexer(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
        DECLARE_UNO3_DEFAULTS(SbaXPropertiesChangeMultiplexer, OSbaWeakSubObject)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        /* css::lang::XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;
        virtual void SAL_CALL propertiesChange(const css::uno::Sequence< css::beans::PropertyChangeEvent >& e) override;
        /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators */
        using OSbaWeakSubObject::operator new;
        using OSbaWeakSubObject::operator delete;
    };
    // the SbaXPropertiesChangeMultiplexer doesn't care about the property names a listener logs on for, it simply
    // forwards _all_ changes to _all_ listeners
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
