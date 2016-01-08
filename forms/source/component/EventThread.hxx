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


typedef ::osl::Thread   OComponentEventThread_TBASE;
class OComponentEventThread
            :public OComponentEventThread_TBASE
            ,public css::lang::XEventListener
            ,public ::cppu::OWeakObject
{
    typedef std::vector<css::lang::EventObject*> ThreadEvents;
    typedef std::vector< css::uno::Reference< css::uno::XAdapter> > ThreadObjects;
    typedef std::vector<sal_Bool> ThreadBools;

    ::osl::Mutex                    m_aMutex;
    ::osl::Condition                m_aCond;            // Queue filled?
    ThreadEvents                    m_aEvents;          // EventQueue
    ThreadObjects                   m_aControls;        // Control for Submit
    ThreadBools                     m_aFlags;           // Flags for Submit/Reset

    ::cppu::OComponentHelper*                     m_pCompImpl;    // Implementation of the Control
    css::uno::Reference< css::lang::XComponent>   m_xComp; // css::lang::XComponent of the Control

protected:

    // XThread
    virtual void SAL_CALL run() override;

    virtual void SAL_CALL onTerminated() override;

    // The following method is called to duplicate the Event while respecting its type.
    virtual css::lang::EventObject* cloneEvent(const css::lang::EventObject* _pEvt) const = 0;

    // Edit an Event:
    // The mutex is not locked, but pCompImpl stays valid in any case.
    // pEvt can be a derrived type, namely the one that cloneEvent returns.
    // rControl is only set, if a Control has been passed in addEvent.
    // Because the Control is only held as a WeakRef, it can disappear in the meantime.
    virtual void processEvent( ::cppu::OComponentHelper* _pCompImpl,
                               const css::lang::EventObject* _pEvt,
                               const css::uno::Reference< css::awt::XControl>& _rControl,
                               bool _bFlag) = 0;

public:

    // UNO Anbindung
    DECLARE_UNO3_DEFAULTS(OComponentEventThread, OWeakObject)
    virtual css::uno::Any SAL_CALL queryInterface(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) override;

    explicit OComponentEventThread(::cppu::OComponentHelper* pCompImpl);
    virtual ~OComponentEventThread();

    void addEvent( const css::lang::EventObject* _pEvt, bool bFlag = false );
    void addEvent( const css::lang::EventObject* _pEvt, const css::uno::Reference< css::awt::XControl>& rControl,
                   bool bFlag = false );

    // css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& _rSource ) throw (css::uno::RuntimeException, std::exception) override;

    // Resolve ambiguity: both OWeakObject and OObject have these memory operators
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
