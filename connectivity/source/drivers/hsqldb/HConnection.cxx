/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HConnection.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 08:21:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "hsqldb/HConnection.hxx"

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX
#include <comphelper/listenernotification.hxx>
#endif

using namespace connectivity::hsqldb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

namespace connectivity
{
    namespace hsqldb
    {
        // =============================================================================
        // = FlushListeners
        // =============================================================================
        typedef ::comphelper::OListenerContainerBase< XFlushListener, EventObject > FlushListeners_Base;
        class FlushListeners : public FlushListeners_Base
        {
        public:
            FlushListeners( ::osl::Mutex& _rMutex ) :FlushListeners_Base( _rMutex ) { }

        protected:
            virtual bool    implNotify(
                                const Reference< XFlushListener >& _rxListener,
                                const EventObject& _rEvent
                            )   SAL_THROW( ( Exception ) );
        };

        // -----------------------------------------------------------------------------
        bool FlushListeners::implNotify( const Reference< XFlushListener >& _rxListener, const EventObject& _rEvent ) SAL_THROW( ( Exception ) )
        {
            _rxListener->flushed( _rEvent );
            return true;    // continue notifying the other listeners, if any
        }
    }
}

// =============================================================================
// = OConnectionWeakWrapper
// =============================================================================
// -----------------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::disposing(void)
{
    m_pFlushListeners->disposing( EventObject( *this ) );
    OConnectionWeakWrapper_BASE::disposing();
    OConnectionWrapper::disposing();
}
// -----------------------------------------------------------------------------
OConnectionWeakWrapper::OConnectionWeakWrapper(
    const Reference< XConnection >& _xConnection ,const Reference< XMultiServiceFactory>& _xORB )
    :OConnectionWeakWrapper_BASE( m_aMutex )
    ,m_pFlushListeners( new FlushListeners( m_aMutex ) )
{
    setDelegation(_xConnection,_xORB,m_refCount);
}
// -----------------------------------------------------------------------------
OConnectionWeakWrapper::~OConnectionWeakWrapper()
{
    if ( !OConnectionWeakWrapper_BASE::rBHelper.bDisposed )
    {
        osl_incrementInterlockedCount( &m_refCount );
        dispose();
    }
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(OConnectionWeakWrapper,OConnectionWeakWrapper_BASE,OConnectionWrapper)
IMPLEMENT_SERVICE_INFO(OConnectionWeakWrapper, "com.sun.star.sdbc.drivers.hsqldb.OConnectionWeakWrapper", "com.sun.star.sdbc.Connection")
IMPLEMENT_FORWARD_XTYPEPROVIDER2(OConnectionWeakWrapper,OConnectionWeakWrapper_BASE,OConnectionWrapper)
// XFlushable
//--------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::flush(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    try
    {
        if ( m_xConnection.is() )
        {
//          Reference< XStatement> xStmt( m_xConnection->createStatement(), UNO_QUERY_THROW );
//            xStmt->execute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SET WRITE_DELAY 0" ) ) );
//
//            sal_Bool bPreviousAutoCommit = m_xConnection->getAutoCommit();
//            m_xConnection->setAutoCommit( sal_False );
//            m_xConnection->commit();
//            m_xConnection->setAutoCommit( bPreviousAutoCommit );
//
//            if ( xStmt.is() )
//              xStmt->execute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SET WRITE_DELAY 60" ) ) );
            Reference< XStatement > xStmt( m_xConnection->createStatement(), UNO_QUERY_THROW );
            xStmt->execute( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CHECKPOINT" ) ) );
        }
        m_pFlushListeners->notify( EventObject( *this ) );
    }
    catch(::com::sun::star::uno::Exception&)
    {
        OSL_ENSURE( false, "OConnectionWeakWrapper::flush: caught an exception!" );
    }
}

//--------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::addFlushListener( const Reference< XFlushListener >& l ) throw (RuntimeException)
{
    m_pFlushListeners->addListener( l );
}

//--------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::removeFlushListener( const Reference< XFlushListener >& l ) throw (RuntimeException)
{
    m_pFlushListeners->removeListener( l );
}

// -----------------------------------------------------------------------------

