/*************************************************************************
 *
 *  $RCSfile: loadlisteneradapter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:03:02 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_BIB_LOADLISTENERADAPTER_HXX
#include "loadlisteneradapter.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

//.........................................................................
namespace bib
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;

    //=====================================================================
    //= OComponentListener
    //=====================================================================
    //---------------------------------------------------------------------
    OComponentListener::~OComponentListener()
    {
        {
            ::osl::MutexGuard aGuard( m_rMutex );
            if ( m_pAdapter )
                m_pAdapter->dispose();
        }
    }

    //---------------------------------------------------------------------
    void OComponentListener::_disposing( const EventObject& _rSource ) throw( RuntimeException)
    {
        // nothing to do here, overrride if you're interested in
    }

    //---------------------------------------------------------------------
    void OComponentListener::setAdapter( OComponentAdapterBase* pAdapter )
    {
        {
            ::osl::MutexGuard aGuard( m_rMutex );
            if ( m_pAdapter )
            {
                m_pAdapter->release();
                m_pAdapter = NULL;
            }
        }

        if ( pAdapter )
        {
            ::osl::MutexGuard aGuard( m_rMutex );
            m_pAdapter = pAdapter;
            m_pAdapter->acquire();
        }
    }

    //=====================================================================
    //= OComponentAdapterBase
    //=====================================================================
    //---------------------------------------------------------------------
    OComponentAdapterBase::OComponentAdapterBase( const Reference< XComponent >& _rxComp, sal_Bool _bAutoRelease )
        :m_xComponent( _rxComp )
        ,m_pListener( NULL )
        ,m_nLockCount( 0 )
        ,m_bListening( sal_False )
        ,m_bAutoRelease( _bAutoRelease )
    {
        OSL_ENSURE( m_xComponent.is(), "OComponentAdapterBase::OComponentAdapterBase: invalid component!" );
    }

    //---------------------------------------------------------------------
    void OComponentAdapterBase::Init( OComponentListener* _pListener )
    {
        OSL_ENSURE( !m_pListener, "OComponentAdapterBase::Init: already initialized!" );
        OSL_ENSURE( _pListener, "OComponentAdapterBase::Init: invalid listener!" );

        m_pListener = _pListener;
        if ( m_pListener )
            m_pListener->setAdapter( this );

        startComponentListening( );
        m_bListening = sal_True;
    }

    //---------------------------------------------------------------------
    OComponentAdapterBase::~OComponentAdapterBase()
    {
    }

    //---------------------------------------------------------------------
    void OComponentAdapterBase::lock()
    {
        ++m_nLockCount;
    }

    //---------------------------------------------------------------------
    void OComponentAdapterBase::unlock()
    {
        --m_nLockCount;
    }

    //---------------------------------------------------------------------
    void OComponentAdapterBase::dispose()
    {
        if ( m_bListening )
        {
            ::vos::ORef< OComponentAdapterBase > xPreventDelete(this);

            disposing();

            m_pListener->setAdapter(NULL);

            m_pListener = NULL;
            m_bListening = sal_False;

            if (m_bAutoRelease)
                m_xComponent = NULL;
        }
    }

    // XEventListener
    //---------------------------------------------------------------------
    void OComponentAdapterBase::disposing()
    {
        // nothing to do here
    }

    //---------------------------------------------------------------------
    void SAL_CALL OComponentAdapterBase::disposing( const EventObject& _rSource ) throw( RuntimeException )
    {
        if ( m_pListener )
        {
             // tell the listener
            if ( !locked() )
                m_pListener->_disposing( _rSource );

            // disconnect the listener
            if ( m_pListener )  // may have been reset whilest calling into _disposing
                m_pListener->setAdapter( NULL );
        }

        m_pListener = NULL;
        m_bListening = sal_False;

        if ( m_bAutoRelease )
            m_xComponent = NULL;
    }

    //=====================================================================
    //= OLoadListenerAdapter
    //=====================================================================
    //---------------------------------------------------------------------
    OLoadListenerAdapter::OLoadListenerAdapter( const Reference< XLoadable >& _rxLoadable, sal_Bool _bAutoRelease )
        :OComponentAdapterBase( Reference< XComponent >( _rxLoadable, UNO_QUERY ), _bAutoRelease )
    {
    }

    //---------------------------------------------------------------------
    void OLoadListenerAdapter::startComponentListening()
    {
        Reference< XLoadable > xLoadable( getComponent(), UNO_QUERY );
        OSL_ENSURE( xLoadable.is(), "OLoadListenerAdapter::OLoadListenerAdapter: invalid object!" );
        if ( xLoadable.is() )
            xLoadable->addLoadListener( this );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OLoadListenerAdapter::acquire(  ) throw ()
    {
        OLoadListenerAdapter_Base::acquire();
    }

    //---------------------------------------------------------------------
    void SAL_CALL OLoadListenerAdapter::release(  ) throw ()
    {
        OLoadListenerAdapter_Base::release();
    }

    //---------------------------------------------------------------------
    void SAL_CALL OLoadListenerAdapter::disposing( const  EventObject& _rSource ) throw( RuntimeException)
    {
        OComponentAdapterBase::disposing( _rSource );
    }

    //---------------------------------------------------------------------
    void OLoadListenerAdapter::disposing()
    {
        Reference< XLoadable > xLoadable( getComponent(), UNO_QUERY );
        if ( xLoadable.is() )
            xLoadable->removeLoadListener( this );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OLoadListenerAdapter::loaded( const EventObject& _rEvent ) throw (RuntimeException)
    {
        if ( !locked() && getLoadListener( ) )
            getLoadListener( )->_loaded( _rEvent );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OLoadListenerAdapter::unloading( const EventObject& _rEvent ) throw (RuntimeException)
    {
        if ( !locked() && getLoadListener( ) )
            getLoadListener( )->_unloading( _rEvent );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OLoadListenerAdapter::unloaded( const EventObject& _rEvent ) throw (RuntimeException)
    {
        if ( !locked() && getLoadListener( ) )
            getLoadListener( )->_unloaded( _rEvent );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OLoadListenerAdapter::reloading( const EventObject& _rEvent ) throw (RuntimeException)
    {
        if ( !locked() && getLoadListener( ) )
            getLoadListener( )->_reloading( _rEvent );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OLoadListenerAdapter::reloaded( const EventObject& _rEvent ) throw (RuntimeException)
    {
        if ( !locked() && getLoadListener( ) )
            getLoadListener( )->_reloaded( _rEvent );
    }

//.........................................................................
}   // namespace bib
//.........................................................................

