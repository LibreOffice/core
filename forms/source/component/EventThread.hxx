/*************************************************************************
 *
 *  $RCSfile: EventThread.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:05 $
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

#ifndef _FRM_EVENT_THREAD_HXX_
#define _FRM_EVENT_THREAD_HXX_

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <vos/thread.hxx>


#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif

#ifndef _UTL_STLTYPES_HXX_
#include <unotools/stl_types.hxx>
#endif
#ifndef _UNOTOOLS_GUARDING_HXX_
#include <unotools/guarding.hxx>
#endif
#ifndef _UTL_UNO3_HXX_
#include <unotools/uno3.hxx>
#endif
using namespace utl;

//.........................................................................
namespace frm
{
//.........................................................................

    namespace starlang  = ::com::sun::star::lang;
    namespace staruno   = ::com::sun::star::uno;

// ***************************************************************************************************
// ***************************************************************************************************

class OComponentEventThread
            :public ::vos::OThread
            ,public starlang::XEventListener
            ,public ::cppu::OWeakObject
{
    DECLARE_STL_VECTOR(starlang::EventObject*, ThreadEvents);
    DECLARE_STL_VECTOR(staruno::Reference<staruno::XAdapter> , ThreadObjects);
    DECLARE_STL_VECTOR(sal_Bool,    ThreadBools);

    OCountedMutex                   m_aMutex;
    ::osl::Condition                m_aCond;            // Queue gefuellt?
    ThreadEvents                    m_aEvents;          // Event-Queue
    ThreadObjects                   m_aControls;        // Control fuer Submit
    ThreadBools                     m_aFlags;           // Flags fuer Submit/Reset

    ::cppu::OComponentHelper*                   m_pCompImpl;    // Implementierung des Controls
    staruno::Reference<starlang::XComponent>    m_xComp;        // starlang::XComponent des Controls

protected:

    // XThread
    virtual void SAL_CALL run();

    // Die folgende Methode wird gerufen um das Event unter Beruecksichtigung
    // seines Typs zu duplizieren.
    virtual starlang::EventObject* cloneEvent(const starlang::EventObject* _pEvt) const = 0;

    // Ein Event bearbeiten. Der Mutex ist dabei nicht gelockt, pCompImpl
    // bleibt aber in jedem Fall gueltig. Bei pEvt kann es sich auch um
    // einen abgeleiteten Typ handeln, naemlich den, den cloneEvent
    // zurueckgibt. rControl ist nur gesetzt, wenn beim addEvent ein
    // Control uebergeben wurde. Da das Control nur als WeakRef gehalten
    // wird kann es auch zwischenzeitlich verschwinden.
    virtual void processEvent( ::cppu::OComponentHelper* _pCompImpl,
                               const starlang::EventObject* _pEvt,
                               const staruno::Reference<starawt::XControl>& _rControl,
                               sal_Bool _bFlag) = 0;

public:

    // UNO Anbindung
    DECLARE_UNO3_DEFAULTS(OComponentEventThread, OWeakObject);
    virtual staruno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

    OComponentEventThread(::cppu::OComponentHelper* pCompImpl);
    virtual ~OComponentEventThread();

    void addEvent( const starlang::EventObject* _pEvt, sal_Bool bFlag = sal_False );
    void addEvent( const starlang::EventObject* _pEvt, const staruno::Reference<starawt::XControl>& rControl,
                   sal_Bool bFlag = sal_False );

    // starlang::XEventListener
    virtual void SAL_CALL disposing(const starlang::EventObject& _rSource );

/* resolve ambiguity : both OWeakObject and OObject have these memory operators */
    void * SAL_CALL operator new( size_t size ) throw() { return OThread::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OThread::operator delete(p); }
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_EVENT_THREAD_HXX_

