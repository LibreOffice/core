/*************************************************************************
 *
 *  $RCSfile: EventThread.cxx,v $
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
#include "EventThread.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

OComponentEventThread::OComponentEventThread( ::cppu::OComponentHelper* pCompImpl ) :
    m_pCompImpl( pCompImpl )
{
    increment(m_refCount);

    // Eine Referenz des Controls halten
    {
        InterfaceRef xIFace(static_cast<staruno::XWeak*>(pCompImpl));
        query_interface(xIFace, m_xComp);
    }

    // und uns an dem Control anmelden
    {
        staruno::Reference<starlang::XEventListener> xEvtLstnr = static_cast<starlang::XEventListener*>(this);
        m_xComp->addEventListener( xEvtLstnr );
    }

    decrement(m_refCount);
}

OComponentEventThread::~OComponentEventThread()
{
    DBG_ASSERT( m_aEvents.size() == 0,
        "OComponentEventThread::~OComponentEventThread: Kein dispose gerufen?" );
    while (m_aEvents.size())
        delete *m_aEvents.erase(m_aEvents.begin());
}

staruno::Any SAL_CALL OComponentEventThread::queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
{
    staruno::Any aReturn;

    aReturn = OWeakObject::queryInterface(_rType);

    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,
            static_cast<starlang::XEventListener*>(this)
        );

    return aReturn;
}

void OComponentEventThread::disposing( const starlang::EventObject& evt )
{
    if( evt.Source == m_xComp )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // Event-Listener abmelden
        staruno::Reference<starlang::XEventListener>  xEvtLstnr = static_cast<starlang::XEventListener*>(this);
        m_xComp->removeEventListener( xEvtLstnr );

        // Event-Queue loeschen
        while (m_aEvents.size())
            delete *m_aEvents.erase(m_aEvents.begin());
        m_aControls.erase(m_aControls.begin(), m_aControls.end());
        m_aFlags.erase(m_aFlags.begin(), m_aFlags.end());

        // Das Control loslassen und pCompImpl auf 0 setzen, damit der
        // Thread weiss, dass er sich beenden soll.
        m_xComp = 0;
        m_pCompImpl = 0;

        // Den Thread aufwecken und beenden.
        m_aCond.set();
        terminate();
    }
}

void OComponentEventThread::addEvent( const starlang::EventObject* _pEvt, sal_Bool bFlag )
{
    staruno::Reference<starawt::XControl>  xTmp;
    addEvent( _pEvt, xTmp, bFlag );
}

void OComponentEventThread::addEvent( const starlang::EventObject* _pEvt,
                                   const staruno::Reference<starawt::XControl>& rControl,
                                   sal_Bool bFlag )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // Daten in die Queue stellen
    m_aEvents.push_back( cloneEvent( _pEvt ) );

    staruno::Reference<staruno::XWeak>      xWeakControl(rControl, staruno::UNO_QUERY);
    staruno::Reference<staruno::XAdapter>   xControlAdapter = xWeakControl.is() ? xWeakControl->queryAdapter() : staruno::Reference<staruno::XAdapter>();
    m_aControls.push_back( xControlAdapter );

    m_aFlags.push_back( bFlag );

    // Thread aufwecken
    m_aCond.set();
}

void OComponentEventThread::run()
{
    // uns selbst festhalten, damit wir nicht geloescht werden,
    // wenn zwischendrinne mal ein dispose gerufen wird.
    InterfaceRef xThis(static_cast<staruno::XWeak*>(this));

    do
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        while( m_aEvents.size() > 0 )
        {
            // Das Control holen und festhalten, damit es waehrend des
            // actionPerformed nicht geloescht werden kann.
            staruno::Reference<starlang::XComponent>  xComp = m_xComp;
            ::cppu::OComponentHelper *pCompImpl = m_pCompImpl;

            starlang::EventObject* pEvt = *m_aEvents.erase( m_aEvents.begin() );
            staruno::Reference<staruno::XAdapter> xControlAdapter = *m_aControls.erase( m_aControls.begin() );
            sal_Bool bFlag = *m_aFlags.erase( m_aFlags.begin() );

            {
                MutexRelease aReleaseOnce(m_aMutex);
                // Weil ein queryHardRef eine staruno::Exception schmeissen kann sollte
                // es nicht bei gelocktem Mutex aufgerufen werden.
                staruno::Reference<starawt::XControl>  xControl;
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

