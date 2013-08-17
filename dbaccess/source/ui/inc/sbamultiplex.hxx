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

#ifndef _SBA_MULTIPLEX_HXX
#define _SBA_MULTIPLEX_HXX

#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/form/XUpdateListener.hpp>
#include <com/sun/star/form/XErrorListener.hpp>
#include <com/sun/star/form/XRestoreListener.hpp>
#include <com/sun/star/form/XInsertListener.hpp>
#include <com/sun/star/form/XDeleteListener.hpp>
#include <com/sun/star/form/XPositioningListener.hpp>
#include <com/sun/star/form/XDatabaseParameterListener.hpp>
#include <com/sun/star/form/XLoadListener.hpp>
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/form/XSubmitListener.hpp>
#include <com/sun/star/form/XResetListener.hpp>
#include <com/sun/star/util/XRefreshListener.hpp>
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
#include <com/sun/star/sdb/XRowSetApproveListener.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <comphelper/stl_types.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>

namespace dbaui
{
    // TODO : replace this class if MM provides an WeakSubObject in cppu
    class OSbaWeakSubObject : public ::cppu::OWeakObject
    {
    protected:
        ::cppu::OWeakObject&    m_rParent;

    public:
        OSbaWeakSubObject(::cppu::OWeakObject& rParent) : m_rParent(rParent) { }

        virtual void SAL_CALL acquire() throw() { m_rParent.acquire(); }
        virtual void SAL_CALL release() throw() { m_rParent.release(); }
    };

    // declaration of a listener multiplexer class
    #define BEGIN_DECLARE_LISTENER_MULTIPLEXER(classname, listenerclass)                    \
    class classname                                                                         \
            :public OSbaWeakSubObject                                                           \
            ,public listenerclass                                                           \
            ,public ::cppu::OInterfaceContainerHelper                                       \
    {                                                                                       \
    public:                                                                                 \
        classname( ::cppu::OWeakObject& rSource,                                            \
            ::osl::Mutex& rMutex);                                                          \
        DECLARE_UNO3_DEFAULTS(classname, OSbaWeakSubObject);                                    \
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(                        \
            const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException); \
                                                                                            \
        /* ::com::sun::star::lang::XEventListener */                                        \
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException);  \

    #define DECLARE_MULTIPLEXER_VOID_METHOD(methodname, eventtype)                          \
        virtual void SAL_CALL methodname(const eventtype& e) throw (::com::sun::star::uno::RuntimeException); \

    #define DECLARE_MULTIPLEXER_BOOL_METHOD(methodname, eventtype)                          \
        virtual sal_Bool SAL_CALL methodname(const eventtype& e) throw (::com::sun::star::uno::RuntimeException);   \

    #define END_DECLARE_LISTENER_MULTIPLEXER()                                              \
    /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators */    \
        void * SAL_CALL operator new( size_t size ) throw() { return OSbaWeakSubObject::operator new(size); }   \
        void SAL_CALL operator delete( void * p ) throw() { OSbaWeakSubObject::operator delete(p); }    \
    };                                                                                      \

    // implementation of a listener multiplexer class

    #define IMPLEMENT_LISTENER_MULTIPLEXER_CORE(classname, listenerclass)                   \
                                                                                            \
    classname::classname(::cppu::OWeakObject& rSource, ::osl::Mutex& _rMutex)               \
            :OSbaWeakSubObject(rSource)                                                     \
            ,OInterfaceContainerHelper(_rMutex)                                             \
    {                                                                                       \
    }                                                                                       \
                                                                                            \
    ::com::sun::star::uno::Any  SAL_CALL classname::queryInterface(                         \
        const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException) \
    {                                                                                       \
        ::com::sun::star::uno::Any aReturn =                                                \
            OSbaWeakSubObject::queryInterface(_rType);                                          \
        if (!aReturn.hasValue())                                                            \
            aReturn = ::cppu::queryInterface(_rType,                                        \
                static_cast< listenerclass* >(this),                                        \
                static_cast< ::com::sun::star::lang::XEventListener* >(static_cast< listenerclass* >(this)) \
            );                                                                              \
                                                                                            \
        return aReturn;                                                                     \
    }                                                                                       \
    void SAL_CALL classname::disposing(const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException)\
    {                                                                                       \
    }                                                                                       \

    #define IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(classname, listenerclass, methodname, eventtype) \
    void SAL_CALL classname::methodname(const eventtype& e) throw (::com::sun::star::uno::RuntimeException) \
    {                                                                                       \
        eventtype aMulti(e);                                                                \
        aMulti.Source = &m_rParent;                                                         \
        ::cppu::OInterfaceIteratorHelper aIt(*this);                                        \
        while (aIt.hasMoreElements())                                                       \
            static_cast< listenerclass*>(aIt.next())->methodname(aMulti);               \
    }                                                                                       \

    #define IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(classname, listenerclass, methodname, eventtype) \
    sal_Bool SAL_CALL classname::methodname(const eventtype& e) throw (::com::sun::star::uno::RuntimeException) \
    {                                                                                       \
        eventtype aMulti(e);                                                                \
        aMulti.Source = &m_rParent;                                                         \
        ::cppu::OInterfaceIteratorHelper aIt(*this);                                        \
        sal_Bool bResult = sal_True;                                                        \
        while (bResult && aIt.hasMoreElements())                                            \
            bResult = static_cast< listenerclass*>(aIt.next())->methodname(aMulti);     \
        return bResult;                                                                     \
    }                                                                                       \

    // helper for classes which do event multiplexing
    #define IMPLEMENT_LISTENER_ADMINISTRATION(classname, listenernamespace, listenerdesc, multiplexer, braodcasterclass, broadcaster) \
    void SAL_CALL classname::add##listenerdesc(const ::com::sun::star::uno::Reference< ::com::sun::star::listenernamespace::X##listenerdesc >& l) throw(::com::sun::star::uno::RuntimeException)\
    {                                                                                       \
        multiplexer.addInterface(l);                                                            \
        if (multiplexer.getLength() == 1)                                                   \
        {                                                                                   \
            ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
            if (xBroadcaster.is())                                                          \
                xBroadcaster->add##listenerdesc(&multiplexer);                              \
        }                                                                                   \
    }                                                                                       \
    void SAL_CALL classname::remove##listenerdesc(const ::com::sun::star::uno::Reference< ::com::sun::star::listenernamespace::X##listenerdesc >& l) throw(::com::sun::star::uno::RuntimeException)\
    {                                                                                       \
        if (multiplexer.getLength() == 1)                                                   \
        {                                                                                   \
            ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
            if (xBroadcaster.is())                                                          \
                xBroadcaster->remove##listenerdesc(&multiplexer);                           \
        }                                                                                   \
        multiplexer.removeInterface(l);                                                     \
    }                                                                                       \

    #define STOP_MULTIPLEXER_LISTENING(listenerdesc, multiplexer, braodcasterclass, broadcaster) \
    if (multiplexer.getLength())                                                            \
    {                                                                                   \
        ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
        if (xBroadcaster.is())                                                          \
            xBroadcaster->remove##listenerdesc(&multiplexer);                           \
    }                                                                                   \

    #define START_MULTIPLEXER_LISTENING(listenerdesc, multiplexer, braodcasterclass, broadcaster) \
    if (multiplexer.getLength())                                                        \
    {                                                                                   \
        ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
        if (xBroadcaster.is())                                                          \
            xBroadcaster->add##listenerdesc(&multiplexer);                              \
    }                                                                                   \

    // declaration of property listener multiplexers
    // (with support for specialized and unspecialized property listeners)

    #define DECLARE_PROPERTY_MULTIPLEXER(classname, listenerclass, methodname, eventtype, exceptions)   \
    class classname                                                                         \
            :public OSbaWeakSubObject                                                           \
            ,public listenerclass                                                           \
    {                                                                                       \
        typedef ::cppu::OMultiTypeInterfaceContainerHelperVar<                              \
                OUString, OUStringHash, ::comphelper::UStringEqual >  ListenerContainerMap;   \
        ListenerContainerMap    m_aListeners;                                               \
                                                                                            \
    public:                                                                                 \
        classname( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );                    \
        DECLARE_UNO3_DEFAULTS(classname, OSbaWeakSubObject);                                    \
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(                        \
            const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException); \
                                                                                            \
        /* ::com::sun::star::lang::XEventListener */                                        \
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException);  \
                                                                                            \
        virtual void SAL_CALL methodname(const eventtype& e)  throw exceptions;             \
                                                                                            \
    public:                                                                                 \
        void addInterface(const OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rListener);    \
        void removeInterface(const OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rListener); \
                                                                                            \
        void disposeAndClear();                                                             \
                                                                                            \
        sal_Int32 getOverallLen() const;                                                    \
                                                                                            \
        ::cppu::OInterfaceContainerHelper* getContainer(const OUString& rName)       \
            { return m_aListeners.getContainer(rName); }                                    \
                                                                                            \
    protected:                                                                              \
        void Notify(::cppu::OInterfaceContainerHelper& rListeners, const eventtype& e);     \
    };                                                                                      \

    // implementation of property listener multiplexers
    #define IMPLEMENT_PROPERTY_MULTIPLEXER(classname, listenerclass, methodname, eventtype, exceptions) \
    classname::classname(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex)                \
            :OSbaWeakSubObject(rSource)                                                     \
            ,m_aListeners(rMutex)                                                           \
    {                                                                                       \
    }                                                                                       \
                                                                                            \
    ::com::sun::star::uno::Any  SAL_CALL classname::queryInterface(                         \
        const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException) \
    {                                                                                       \
        ::com::sun::star::uno::Any aReturn =                                                \
            OSbaWeakSubObject::queryInterface(_rType);                                          \
        if (!aReturn.hasValue())                                                            \
            aReturn = ::cppu::queryInterface(_rType,                                        \
                static_cast< listenerclass* >(this),                                        \
                static_cast< ::com::sun::star::lang::XEventListener* >(static_cast< listenerclass* >(this)) \
            );                                                                              \
                                                                                            \
        return aReturn;                                                                     \
    }                                                                                       \
    void SAL_CALL classname::disposing(const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException)\
    {                                                                                       \
    }                                                                                       \
                                                                                            \
    void SAL_CALL classname::methodname(const eventtype& e) throw exceptions                \
    {                                                                                       \
        ::cppu::OInterfaceContainerHelper* pListeners = m_aListeners.getContainer(e.PropertyName);  \
        if (pListeners)                                                                     \
            Notify(*pListeners, e);                                                         \
                                                                                            \
        /* do the notification for the unspecialized listeners, too */                      \
        pListeners = m_aListeners.getContainer(OUString());                          \
        if (pListeners)                                                                     \
            Notify(*pListeners, e);                                                         \
    }                                                                                       \
                                                                                            \
    void classname::addInterface(const OUString& rName,                              \
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rListener)    \
    {                                                                                       \
        m_aListeners.addInterface(rName, rListener);                                        \
    }                                                                                       \
                                                                                            \
    void classname::removeInterface(const OUString& rName,                           \
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rListener)    \
    {                                                                                       \
        m_aListeners.removeInterface(rName, rListener);                                     \
    }                                                                                       \
                                                                                            \
    void classname::disposeAndClear()                                                       \
    {                                                                                       \
        ::com::sun::star::lang::EventObject aEvt(m_rParent);                                \
        m_aListeners.disposeAndClear(aEvt);                                                             \
    }                                                                                       \
                                                                                            \
    sal_Int32 classname::getOverallLen() const                                              \
    {                                                                                       \
        sal_Int32 nLen = 0;                                                                 \
        ::com::sun::star::uno::Sequence< OUString > aContained = m_aListeners.getContainedTypes();   \
        const OUString* pContained = aContained.getConstArray();                     \
        for (   sal_Int32 i=0; i<aContained.getLength(); ++i, ++pContained)                 \
            nLen += m_aListeners.getContainer(*pContained)->getLength();                    \
        return nLen;                                                                        \
    }                                                                                       \
                                                                                            \
    void classname::Notify(::cppu::OInterfaceContainerHelper& rListeners, const eventtype& e)   \
    {                                                                                       \
        eventtype aMulti(e);                                                                \
        aMulti.Source = &m_rParent;                                                         \
        ::cppu::OInterfaceIteratorHelper aIt(rListeners);                                   \
        while (aIt.hasMoreElements())                                                       \
            static_cast< listenerclass*>(aIt.next())->methodname(aMulti);               \
    }                                                                                       \

    // helper for classes which do property event multiplexing
    #define IMPLEMENT_PROPERTY_LISTENER_ADMINISTRATION(classname, listenerdesc, multiplexer, braodcasterclass, broadcaster) \
    void SAL_CALL classname::add##listenerdesc(const OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::X##listenerdesc >& l ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)\
    {                                                                                       \
        multiplexer.addInterface(rName, l);                                                 \
        if (multiplexer.getOverallLen() == 1)                                               \
        {                                                                                   \
            ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
            if (xBroadcaster.is())                                                          \
                xBroadcaster->add##listenerdesc(OUString(), &multiplexer);                           \
        }                                                                                   \
    }                                                                                       \
    void SAL_CALL classname::remove##listenerdesc(const OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::X##listenerdesc >& l ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)\
    {                                                                                       \
        if (multiplexer.getOverallLen() == 1)                                               \
        {                                                                                   \
            ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
            if (xBroadcaster.is())                                                          \
                xBroadcaster->remove##listenerdesc(OUString(), &multiplexer);                        \
        }                                                                                   \
        multiplexer.removeInterface(rName, l);                                              \
    }                                                                                       \

    #define STOP_PROPERTY_MULTIPLEXER_LISTENING(listenerdesc, multiplexer, braodcasterclass, broadcaster) \
    if (multiplexer.getOverallLen())                                                        \
    {                                                                                       \
        ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
        if (xBroadcaster.is())                                                              \
            xBroadcaster->remove##listenerdesc(OUString(), &multiplexer);                            \
    }                                                                                       \

    #define START_PROPERTY_MULTIPLEXER_LISTENING(listenerdesc, multiplexer, braodcasterclass, broadcaster) \
    if (multiplexer.getOverallLen())                                                        \
    {                                                                                       \
        ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
        if (xBroadcaster.is())                                                              \
            xBroadcaster->add##listenerdesc(OUString(), &multiplexer);                               \
    }                                                                                       \

    // some listener multiplexers
    // ::com::sun::star::frame::XStatusListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXStatusMultiplexer, ::com::sun::star::frame::XStatusListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(statusChanged, ::com::sun::star::frame::FeatureStateEvent)

    private:
        ::com::sun::star::frame::FeatureStateEvent  m_aLastKnownStatus;
    public:                                                                                 \
        inline ::com::sun::star::frame::FeatureStateEvent getLastEvent( ) const { return m_aLastKnownStatus; }
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::form::XLoadListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(loaded, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(unloaded, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(unloading, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(reloading, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(reloaded, ::com::sun::star::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::form::XDatabaseParameterListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXParameterMultiplexer, ::com::sun::star::form::XDatabaseParameterListener)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveParameter, ::com::sun::star::form::DatabaseParameterEvent)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::form::XSubmitListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXSubmitMultiplexer, ::com::sun::star::form::XSubmitListener)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveSubmit, ::com::sun::star::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::form::XResetListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXResetMultiplexer, ::com::sun::star::form::XResetListener)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveReset, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(resetted, ::com::sun::star::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::sdbc::XRowSetListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXRowSetMultiplexer, ::com::sun::star::sdbc::XRowSetListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(cursorMoved, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(rowChanged, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(rowSetChanged, ::com::sun::star::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::sdb::XRowSetApproveListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXRowSetApproveMultiplexer, ::com::sun::star::sdb::XRowSetApproveListener)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveCursorMove, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveRowChange, ::com::sun::star::sdb::RowChangeEvent)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveRowSetChange, ::com::sun::star::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::sdb::XSQLErrorListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXSQLErrorMultiplexer, ::com::sun::star::sdb::XSQLErrorListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(errorOccured, ::com::sun::star::sdb::SQLErrorEvent)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::beans::XPropertyChangeListener
    DECLARE_PROPERTY_MULTIPLEXER(SbaXPropertyChangeMultiplexer, ::com::sun::star::beans::XPropertyChangeListener, propertyChange, ::com::sun::star::beans::PropertyChangeEvent, (::com::sun::star::uno::RuntimeException))

    // ::com::sun::star::beans::XVetoableChangeListener
    DECLARE_PROPERTY_MULTIPLEXER(SbaXVetoableChangeMultiplexer, ::com::sun::star::beans::XVetoableChangeListener, vetoableChange, ::com::sun::star::beans::PropertyChangeEvent, (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException))

    // ::com::sun::star::beans::XPropertiesChangeListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXPropertiesChangeMultiplexer, ::com::sun::star::beans::XPropertiesChangeListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(propertiesChange, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >)
    END_DECLARE_LISTENER_MULTIPLEXER()
    // the SbaXPropertiesChangeMultiplexer doesn't care about the property names a listener logs on for, it simply
    // forwards _all_ changes to _all_ listeners
}

#endif // _SBA_MULTIPLEXER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
