/*************************************************************************
 *
 *  $RCSfile: sbamultiplex.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2000-10-31 12:50:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SBA_MULTIPLEX_HXX
#define _SBA_MULTIPLEX_HXX

//#ifndef _SDB_TOOLS_HXX
//#include <sdb/tools.hxx>
//#endif

#ifndef _COM_SUN_STAR_BEANS_XVETOABLECHANGELISTENER_HPP_
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XUPDATELISTENER_HPP_
#include <com/sun/star/form/XUpdateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XERRORLISTENER_HPP_
#include <com/sun/star/form/XErrorListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESTORELISTENER_HPP_
#include <com/sun/star/form/XRestoreListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XINSERTLISTENER_HPP_
#include <com/sun/star/form/XInsertListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XDELETELISTENER_HPP_
#include <com/sun/star/form/XDeleteListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XPOSITIONINGLISTENER_HPP_
#include <com/sun/star/form/XPositioningListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XDATABASEPARAMETERLISTENER_HPP_
#include <com/sun/star/form/XDatabaseParameterListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADLISTENER_HPP_
#include <com/sun/star/form/XLoadListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATECHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XSUBMITLISTENER_HPP_
#include <com/sun/star/form/XSubmitListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESETLISTENER_HPP_
#include <com/sun/star/form/XResetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHLISTENER_HPP_
#include <com/sun/star/util/XRefreshListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XSQLERRORLISTENER_HPP_
#include <com/sun/star/sdb/XSQLErrorListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XROWSETAPPROVELISTENER_HPP_
#include <com/sun/star/sdb/XRowSetApproveListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSETLISTENER_HPP_
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

namespace dbaui
{
    //------------------------------------------------------------------
    // TODO : replace this class if MM provides an WeakSubObject in cppu
    class OSbaWeakSubObject : public ::cppu::OWeakObject
    {
    protected:
        ::cppu::OWeakObject&    m_rParent;

    public:
        OSbaWeakSubObject(::cppu::OWeakObject& rParent) : m_rParent(rParent) { }

        virtual void SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException) { m_rParent.acquire(); }
        virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException) { m_rParent.release(); }
    };

    //------------------------------------------------------------------
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
        virtual void SAL_CALL methodname(const eventtype& e);                               \


    #define DECLARE_MULTIPLEXER_BOOL_METHOD(methodname, eventtype)                          \
        virtual sal_Bool SAL_CALL methodname(const eventtype& e);                           \


    #define END_DECLARE_LISTENER_MULTIPLEXER()                                              \
    /* resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators */    \
        void * SAL_CALL operator new( size_t size ) throw() { return OSbaWeakSubObject::operator new(size); }   \
        void SAL_CALL operator delete( void * p ) throw() { OSbaWeakSubObject::operator delete(p); }    \
    };                                                                                      \

    //------------------------------------------------------------------
    // implementation of a listener multiplexer class

    #define IMPLEMENT_LISTENER_MULTIPLEXER_CORE(classname, listenerclass)                   \
                                                                                            \
    /*................................................................*/                    \
    classname::classname(::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex)                \
            :OSbaWeakSubObject(rSource)                                                     \
            ,OInterfaceContainerHelper(rMutex)                                              \
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
    /*................................................................*/                    \
    void SAL_CALL classname::disposing(const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException)\
    {                                                                                       \
    }                                                                                       \


    #define IMPLEMENT_LISTENER_MULTIPLEXER_VOID_METHOD(classname, listenerclass, methodname, eventtype) \
    void SAL_CALL classname::methodname(const eventtype& e)                                         \
    {                                                                                       \
        eventtype aMulti(e);                                                                \
        aMulti.Source = &m_rParent;                                                         \
        ::cppu::OInterfaceIteratorHelper aIt(*this);                                        \
        while (aIt.hasMoreElements())                                                       \
            reinterpret_cast< listenerclass*>(aIt.next())->methodname(aMulti);              \
    }                                                                                       \

    #define IMPLEMENT_LISTENER_MULTIPLEXER_BOOL_METHOD(classname, listenerclass, methodname, eventtype) \
    /*................................................................*/                    \
    sal_Bool SAL_CALL classname::methodname(const eventtype& e)                                     \
    {                                                                                       \
        eventtype aMulti(e);                                                                \
        aMulti.Source = &m_rParent;                                                         \
        ::cppu::OInterfaceIteratorHelper aIt(*this);                                        \
        sal_Bool bResult = sal_True;                                                        \
        while (bResult && aIt.hasMoreElements())                                            \
            bResult = reinterpret_cast< listenerclass*>(aIt.next())->methodname(aMulti);        \
        return bResult;                                                                     \
    }                                                                                       \

    //------------------------------------------------------------------
    // helper for classes which do event multiplexing
    #define IMPLEMENT_LISTENER_ADMINISTRATION(classname, listenernamespace, listenerdesc, multiplexer, braodcasterclass, broadcaster) \
    /*................................................................*/                    \
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
    /*................................................................*/                    \
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



    //------------------------------------------------------------------
    // declaration of property listener multiplexers
    // (with support for specialized and unspecialized property listeners)

    #define DECLARE_PROPERTY_MULTIPLEXER(classname, listenerclass, methodname, eventtype)   \
    class classname                                                                         \
            :public OSbaWeakSubObject                                                           \
            ,public listenerclass                                                           \
    {                                                                                       \
        typedef ::cppu::OMultiTypeInterfaceContainerHelperVar<                              \
                ::rtl::OUString, ::comphelper::UStringHash, ::comphelper::UStringEqual >    ListenerContainerMap;   \
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
        virtual void SAL_CALL methodname(const eventtype& e);                               \
                                                                                            \
    public:                                                                                 \
        void addInterface(const ::rtl::OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rListener);    \
        void removeInterface(const ::rtl::OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rListener); \
                                                                                            \
        void disposeAndClear();                                                             \
                                                                                            \
        sal_Int32 getOverallLen() const;                                                    \
                                                                                            \
        ::cppu::OInterfaceContainerHelper* getContainer(const ::rtl::OUString& rName)       \
            { return m_aListeners.getContainer(rName); }                                    \
                                                                                            \
    protected:                                                                              \
        void Notify(::cppu::OInterfaceContainerHelper& rListeners, const eventtype& e);     \
    };                                                                                      \


    //------------------------------------------------------------------
    // implementation of property listener multiplexers
    #define IMPLEMENT_PROPERTY_MULTIPLEXER(classname, listenerclass, methodname, eventtype) \
    /*................................................................*/                    \
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
    /*................................................................*/                    \
    void SAL_CALL classname::disposing(const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException)\
    {                                                                                       \
    }                                                                                       \
                                                                                            \
    void SAL_CALL classname::methodname(const eventtype& e)                                         \
    {                                                                                       \
        ::cppu::OInterfaceContainerHelper* pListeners = m_aListeners.getContainer(e.PropertyName);  \
        if (pListeners)                                                                     \
            Notify(*pListeners, e);                                                         \
                                                                                            \
        /* do the notification for the unspecialized listeners, too */                      \
        pListeners = m_aListeners.getContainer(::rtl::OUString());                          \
        if (pListeners)                                                                     \
            Notify(*pListeners, e);                                                         \
    }                                                                                       \
                                                                                            \
    void classname::addInterface(const ::rtl::OUString& rName,                              \
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rListener)    \
    {                                                                                       \
        m_aListeners.addInterface(rName, rListener);                                        \
    }                                                                                       \
                                                                                            \
    void classname::removeInterface(const ::rtl::OUString& rName,                           \
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
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aContained = m_aListeners.getContainedTypes();   \
        const ::rtl::OUString* pContained = aContained.getConstArray();                     \
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
            reinterpret_cast< listenerclass*>(aIt.next())->methodname(aMulti);              \
    }                                                                                       \

    //------------------------------------------------------------------
    // helper for classes which do property event multiplexing
    #define IMPLEMENT_PROPERTY_LISTENER_ADMINISTRATION(classname, listenerdesc, multiplexer, braodcasterclass, broadcaster) \
    /*................................................................*/                    \
    void SAL_CALL classname::add##listenerdesc(const ::rtl::OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::X##listenerdesc >& l ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)\
    {                                                                                       \
        multiplexer.addInterface(rName, l);                                                 \
        if (multiplexer.getOverallLen() == 1)                                               \
        {                                                                                   \
            ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
            if (xBroadcaster.is())                                                          \
                xBroadcaster->add##listenerdesc(::rtl::OUString(), &multiplexer);                           \
        }                                                                                   \
    }                                                                                       \
    /*................................................................*/                    \
    void SAL_CALL classname::remove##listenerdesc(const ::rtl::OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::X##listenerdesc >& l ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)\
    {                                                                                       \
        if (multiplexer.getOverallLen() == 1)                                               \
        {                                                                                   \
            ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
            if (xBroadcaster.is())                                                          \
                xBroadcaster->remove##listenerdesc(::rtl::OUString(), &multiplexer);                        \
        }                                                                                   \
        multiplexer.removeInterface(rName, l);                                              \
    }                                                                                       \

    #define STOP_PROPERTY_MULTIPLEXER_LISTENING(listenerdesc, multiplexer, braodcasterclass, broadcaster) \
    if (multiplexer.getOverallLen())                                                        \
    {                                                                                       \
        ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
        if (xBroadcaster.is())                                                              \
            xBroadcaster->remove##listenerdesc(::rtl::OUString(), &multiplexer);                            \
    }                                                                                       \

    #define START_PROPERTY_MULTIPLEXER_LISTENING(listenerdesc, multiplexer, braodcasterclass, broadcaster) \
    if (multiplexer.getOverallLen())                                                        \
    {                                                                                       \
        ::com::sun::star::uno::Reference< braodcasterclass > xBroadcaster(broadcaster, ::com::sun::star::uno::UNO_QUERY);   \
        if (xBroadcaster.is())                                                              \
            xBroadcaster->add##listenerdesc(::rtl::OUString(), &multiplexer);                               \
    }                                                                                       \



    //==================================================================
    // some listener multiplexers
    //==================================================================
    // ::com::sun::star::frame::XStatusListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXStatusMultiplexer, ::com::sun::star::frame::XStatusListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(statusChanged, ::com::sun::star::frame::FeatureStateEvent)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::form::XLoadListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXLoadMultiplexer, ::com::sun::star::form::XLoadListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(loaded, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(unloaded, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(unloading, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(reloading, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(reloaded, ::com::sun::star::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::form::XPositioningListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXPositioningMultiplexer, ::com::sun::star::form::XPositioningListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(positioned, ::com::sun::star::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::form::XInsertListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXInsertMultiplexer, ::com::sun::star::form::XInsertListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(inserting, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(inserted, ::com::sun::star::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::form::XRestoreListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXRestoreMultiplexer, ::com::sun::star::form::XRestoreListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(restored, ::com::sun::star::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::form::XDeleteListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXDeleteMultiplexer, ::com::sun::star::form::XDeleteListener)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveDelete, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(deleted, ::com::sun::star::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::form::XUpdateListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXUpdateMultiplexer, ::com::sun::star::form::XUpdateListener)
        DECLARE_MULTIPLEXER_BOOL_METHOD(approveUpdate, ::com::sun::star::lang::EventObject)
        DECLARE_MULTIPLEXER_VOID_METHOD(updated, ::com::sun::star::lang::EventObject)
    END_DECLARE_LISTENER_MULTIPLEXER()

    // ::com::sun::star::form::XErrorListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXErrorMultiplexer, ::com::sun::star::form::XErrorListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(errorOccured, ::com::sun::star::form::ErrorEvent)
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

    // ::com::sun::star::util::XRefreshListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXRefreshMultiplexer, ::com::sun::star::util::XRefreshListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(refreshed, ::com::sun::star::lang::EventObject)
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
    DECLARE_PROPERTY_MULTIPLEXER(SbaXPropertyChangeMultiplexer, ::com::sun::star::beans::XPropertyChangeListener, propertyChange, ::com::sun::star::beans::PropertyChangeEvent)

    // ::com::sun::star::beans::XVetoableChangeListener
    DECLARE_PROPERTY_MULTIPLEXER(SbaXVetoableChangeMultiplexer, ::com::sun::star::beans::XVetoableChangeListener, vetoableChange, ::com::sun::star::beans::PropertyChangeEvent)

    // ::com::sun::star::beans::XPropertyStateChangeListener
    DECLARE_PROPERTY_MULTIPLEXER(SbaXPropertyStateChangeMultiplexer, ::com::sun::star::beans::XPropertyStateChangeListener, propertyStateChange, ::com::sun::star::beans::PropertyStateChangeEvent)

    // ::com::sun::star::beans::XPropertiesChangeListener
    BEGIN_DECLARE_LISTENER_MULTIPLEXER(SbaXPropertiesChangeMultiplexer, ::com::sun::star::beans::XPropertiesChangeListener)
        DECLARE_MULTIPLEXER_VOID_METHOD(propertiesChange, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >)
    END_DECLARE_LISTENER_MULTIPLEXER()
    // the SbaXPropertiesChangeMultiplexer doesn't care about the property names a listener logs on for, it simply
    // forwards _all_ changes to _all_ listeners
}

#endif // _SBA_MULTIPLEXER_HXX

