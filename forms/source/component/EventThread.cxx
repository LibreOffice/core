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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "EventThread.hxx"
#include <comphelper/guarding.hxx>
#include <tools/debug.hxx>

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;

DBG_NAME( OComponentEventThread )
OComponentEventThread::OComponentEventThread( ::cppu::OComponentHelper* pCompImpl ) :
    m_pCompImpl( pCompImpl )
{
    DBG_CTOR( OComponentEventThread, NULL );

    increment(m_refCount);

    // Eine Referenz des Controls halten
    {
        InterfaceRef xIFace(static_cast<XWeak*>(pCompImpl));
        query_interface(xIFace, m_xComp);
    }

    // und uns an dem Control anmelden
    {
        Reference<XEventListener> xEvtLstnr = static_cast<XEventListener*>(this);
        m_xComp->addEventListener( xEvtLstnr );
    }

    decrement(m_refCount);
}

OComponentEventThread::~OComponentEventThread()
{
    DBG_DTOR( OComponentEventThread, NULL );

    DBG_ASSERT( m_aEvents.size() == 0,
        "OComponentEventThread::~OComponentEventThread: Kein dispose gerufen?" );

    impl_clearEventQueue();
}

Any SAL_CALL OComponentEventThread::queryInterface(const Type& _rType) throw (RuntimeException)
{
    Any aReturn;

    aReturn = OWeakObject::queryInterface(_rType);

    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast<XEventListener*>(this)
        );

    return aReturn;
}

void OComponentEventThread::impl_clearEventQueue()
{
    while ( m_aEvents.size() )
    {
        delete *m_aEvents.begin();
        m_aEvents.erase( m_aEvents.begin() );
    }
    m_aControls.erase( m_aControls.begin(), m_aControls.end() );
    m_aFlags.erase( m_aFlags.begin(), m_aFlags.end() );
}

void OComponentEventThread::disposing( const EventObject& evt ) throw ( ::com::sun::star::uno::RuntimeException)
{
    if( evt.Source == m_xComp )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // Event-Listener abmelden
        Reference<XEventListener>  xEvtLstnr = static_cast<XEventListener*>(this);
        m_xComp->removeEventListener( xEvtLstnr );

        // Event-Queue loeschen
        impl_clearEventQueue();

        // Das Control loslassen und pCompImpl auf 0 setzen, damit der
        // Thread weiss, dass er sich beenden soll.
        m_xComp = 0;
        m_pCompImpl = 0;

        // Den Thread aufwecken und beenden.
        m_aCond.set();
        terminate();
    }
}

void OComponentEventThread::addEvent( const EventObject* _pEvt, sal_Bool bFlag )
{
    Reference<XControl>  xTmp;
    addEvent( _pEvt, xTmp, bFlag );
}

void OComponentEventThread::addEvent( const EventObject* _pEvt,
                                   const Reference<XControl>& rControl,
                                   sal_Bool bFlag )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Daten in die Queue stellen
    m_aEvents.push_back( cloneEvent( _pEvt ) );

    Reference<XWeak>        xWeakControl(rControl, UNO_QUERY);
    Reference<XAdapter> xControlAdapter = xWeakControl.is() ? xWeakControl->queryAdapter() : Reference<XAdapter>();
    m_aControls.push_back( xControlAdapter );

    m_aFlags.push_back( bFlag );

    // Thread aufwecken
    m_aCond.set();
}

//---------------------------------------------------------------------
//--- 22.08.01 15:48:15 -----------------------------------------------

void OComponentEventThread::implStarted( )
{
    acquire( );
}

//---------------------------------------------------------------------
//--- 22.08.01 15:48:16 -----------------------------------------------

void OComponentEventThread::implTerminated( )
{
    release( );
}

//---------------------------------------------------------------------
//--- 22.08.01 15:47:31 -----------------------------------------------

void SAL_CALL OComponentEventThread::kill()
{
    OComponentEventThread_TBASE::kill();

    implTerminated( );
}

//---------------------------------------------------------------------
//--- 22.08.01 15:47:33 -----------------------------------------------

void SAL_CALL OComponentEventThread::onTerminated()
{
    OComponentEventThread_TBASE::onTerminated();

    implTerminated( );
}

void OComponentEventThread::run()
{
    implStarted( );

    // uns selbst festhalten, damit wir nicht geloescht werden,
    // wenn zwischendrinne mal ein dispose gerufen wird.
    InterfaceRef xThis(static_cast<XWeak*>(this));

    do
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        while( m_aEvents.size() > 0 )
        {
            // Das Control holen und festhalten, damit es waehrend des
            // actionPerformed nicht geloescht werden kann.
            Reference<XComponent>  xComp = m_xComp;
            ::cppu::OComponentHelper *pCompImpl = m_pCompImpl;

            ThreadEvents::iterator firstEvent( m_aEvents.begin() );
            EventObject* pEvt = *firstEvent;
            m_aEvents.erase( firstEvent );

            ThreadObjects::iterator firstControl( m_aControls.begin() );
            Reference<XAdapter> xControlAdapter = *firstControl;
            m_aControls.erase( firstControl );

            ThreadBools::iterator firstFlag( m_aFlags.begin() );
            sal_Bool bFlag = *firstFlag;
            m_aFlags.erase( firstFlag );

            {
                MutexRelease aReleaseOnce(m_aMutex);
                // Weil ein queryHardRef eine Exception schmeissen kann sollte
                // es nicht bei gelocktem Mutex aufgerufen werden.
                Reference<XControl>  xControl;
                if ( xControlAdapter.is() )
                    query_interface(xControlAdapter->queryAdapted(), xControl);

                if( xComp.is() )
                    processEvent( pCompImpl, pEvt, xControl, bFlag );
            }

            delete pEvt;
        };

        // Nach einem dispose kennen wir das Control nicht mehr. Dann darf
        // auch nicht gewartet werden.
        if( !m_xComp.is() )
            return;

        // Warte-Bedingung zuruecksetzen
        m_aCond.reset();
        {
            MutexRelease aReleaseOnce(m_aMutex);
            // und warten ... falls nicht zwischenzeitlich doch noch ein
            // Event eingetroffen ist.
            m_aCond.wait();
        }
    }
    while( sal_True );
}

//.........................................................................
}   // namespace frm
//.........................................................................

