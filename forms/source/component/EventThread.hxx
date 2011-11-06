/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _FRM_EVENT_THREAD_HXX_
#define _FRM_EVENT_THREAD_HXX_

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <vos/thread.hxx>


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

typedef ::vos::OThread  OComponentEventThread_TBASE;
class OComponentEventThread
            :public OComponentEventThread_TBASE
            ,public ::com::sun::star::lang::XEventListener
            ,public ::cppu::OWeakObject
{
    DECLARE_STL_VECTOR(::com::sun::star::lang::EventObject*, ThreadEvents);
    DECLARE_STL_VECTOR(::com::sun::star::uno::Reference< ::com::sun::star::uno::XAdapter> , ThreadObjects);
    DECLARE_STL_VECTOR(sal_Bool,    ThreadBools);

    ::osl::Mutex                    m_aMutex;
    ::osl::Condition                m_aCond;            // Queue gefuellt?
    ThreadEvents                    m_aEvents;          // Event-Queue
    ThreadObjects                   m_aControls;        // Control fuer Submit
    ThreadBools                     m_aFlags;           // Flags fuer Submit/Reset

    ::cppu::OComponentHelper*                   m_pCompImpl;    // Implementierung des Controls
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>   m_xComp;        // ::com::sun::star::lang::XComponent des Controls

protected:

    // XThread
    virtual void SAL_CALL run();

    virtual void SAL_CALL kill();
    virtual void SAL_CALL onTerminated();

    // Die folgende Methode wird gerufen um das Event unter Beruecksichtigung
    // seines Typs zu duplizieren.
    virtual ::com::sun::star::lang::EventObject* cloneEvent(const ::com::sun::star::lang::EventObject* _pEvt) const = 0;

    // Ein Event bearbeiten. Der Mutex ist dabei nicht gelockt, pCompImpl
    // bleibt aber in jedem Fall gueltig. Bei pEvt kann es sich auch um
    // einen abgeleiteten Typ handeln, naemlich den, den cloneEvent
    // zurueckgibt. rControl ist nur gesetzt, wenn beim addEvent ein
    // Control uebergeben wurde. Da das Control nur als WeakRef gehalten
    // wird kann es auch zwischenzeitlich verschwinden.
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
    void * SAL_CALL operator new( size_t size ) throw() { return OThread::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OThread::operator delete(p); }

private:
    void    implStarted( );
    void    implTerminated( );

    void    impl_clearEventQueue();
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_EVENT_THREAD_HXX_

