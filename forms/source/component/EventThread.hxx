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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_EVENTTHREAD_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_EVENTTHREAD_HXX

#include <sal/config.h>

#include <vector>

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <osl/thread.hxx>


#include <osl/conditn.hxx>
#include <cppuhelper/component.hxx>
#include <comphelper/uno3.hxx>
using namespace comphelper;


namespace frm
{


// ***************************************************************************************************
// ***************************************************************************************************

typedef ::osl::Thread   OComponentEventThread_TBASE;
class OComponentEventThread
            :public OComponentEventThread_TBASE
            ,public ::com::sun::star::lang::XEventListener
            ,public ::cppu::OWeakObject
{
    typedef std::vector<css::lang::EventObject*> ThreadEvents;
    typedef std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAdapter> > ThreadObjects;
    typedef std::vector<sal_Bool> ThreadBools;

    ::osl::Mutex                    m_aMutex;
    ::osl::Condition                m_aCond;            // Queue filled?
    ThreadEvents                    m_aEvents;          // EventQueue
    ThreadObjects                   m_aControls;        // Control for Submit
    ThreadBools                     m_aFlags;           // Flags for Submit/Reset

    ::cppu::OComponentHelper*                   m_pCompImpl;    // Implementation of the Control
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>   m_xComp; // ::com::sun::star::lang::XComponent of the Control

protected:

    // XThread
    virtual void SAL_CALL run() SAL_OVERRIDE;

    virtual void SAL_CALL kill();
    virtual void SAL_CALL onTerminated() SAL_OVERRIDE;

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
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    OComponentEventThread(::cppu::OComponentHelper* pCompImpl);
    virtual ~OComponentEventThread();

    void addEvent( const ::com::sun::star::lang::EventObject* _pEvt, sal_Bool bFlag = sal_False );
    void addEvent( const ::com::sun::star::lang::EventObject* _pEvt, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& rControl,
                   sal_Bool bFlag = sal_False );

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

/* resolve ambiguity : both OWeakObject and OObject have these memory operators */
    void * SAL_CALL operator new( size_t size ) throw() { return osl::Thread::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { osl::Thread::operator delete(p); }

private:
    void    implStarted( );
    void    implTerminated( );

    void    impl_clearEventQueue();
};


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_EVENTTHREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
