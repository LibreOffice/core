/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FRM_EVENT_THREAD_HXX_
#define _FRM_EVENT_THREAD_HXX_

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <osl/thread.hxx>


#include <osl/conditn.hxx>
#include <cppuhelper/component.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/uno3.hxx>
using namespace comphelper;

//.........................................................................
namespace frm
{
//.........................................................................

// ***************************************************************************************************
// ***************************************************************************************************

typedef ::osl::Thread   OComponentEventThread_TBASE;
class OComponentEventThread
            :public OComponentEventThread_TBASE
            ,public ::com::sun::star::lang::XEventListener
            ,public ::cppu::OWeakObject
{
    DECLARE_STL_VECTOR(::com::sun::star::lang::EventObject*, ThreadEvents);
    DECLARE_STL_VECTOR(::com::sun::star::uno::Reference< ::com::sun::star::uno::XAdapter> , ThreadObjects);
    DECLARE_STL_VECTOR(sal_Bool,    ThreadBools);

    ::osl::Mutex                    m_aMutex;
    ::osl::Condition                m_aCond;            // Queue filled?
    ThreadEvents                    m_aEvents;          // EventQueue
    ThreadObjects                   m_aControls;        // Control for Submit
    ThreadBools                     m_aFlags;           // Flags for Submit/Reset

    ::cppu::OComponentHelper*                   m_pCompImpl;    // Implementation of the Control
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>   m_xComp; // ::com::sun::star::lang::XComponent of the Control

protected:

    // XThread
    virtual void SAL_CALL run();

    virtual void SAL_CALL kill();
    virtual void SAL_CALL onTerminated();

    // The following method is called to duplicate the Event while respecting it's type.
    virtual ::com::sun::star::lang::EventObject* cloneEvent(const ::com::sun::star::lang::EventObject* _pEvt) const = 0;

    // Edit an Event:
    // The mutex is not locked, but pCompImpl stays valid in any case.
    // pEvt can be a derrived type, namely the one that cloneEvent returns.
    // rControl is only set, if a Control has been passed in addEvent.
    // Because the Control is only held as a WeakRef, it can disappear in the meantime.
    virtual void processEvent( ::cppu::OComponentHelper* _pCompImpl,
                               const ::com::sun::star::lang::EventObject* _pEvt,
                               const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& _rControl,
                               sal_Bool _bFlag) = 0;

public:

    // UNO Anbindung
    DECLARE_UNO3_DEFAULTS(OComponentEventThread, OWeakObject);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

    OComponentEventThread(::cppu::OComponentHelper* pCompImpl);
    virtual ~OComponentEventThread();

    void addEvent( const ::com::sun::star::lang::EventObject* _pEvt, sal_Bool bFlag = sal_False );
    void addEvent( const ::com::sun::star::lang::EventObject* _pEvt, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& rControl,
                   sal_Bool bFlag = sal_False );

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

/* resolve ambiguity : both OWeakObject and OObject have these memory operators */
    void * SAL_CALL operator new( size_t size ) throw() { return osl::Thread::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { osl::Thread::operator delete(p); }

private:
    void    implStarted( );
    void    implTerminated( );

    void    impl_clearEventQueue();
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_EVENT_THREAD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
