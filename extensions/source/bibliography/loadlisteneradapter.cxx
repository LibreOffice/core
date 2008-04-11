/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: loadlisteneradapter.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "loadlisteneradapter.hxx"
#include <osl/diagnose.h>
#include <vos/ref.hxx>

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
    void OComponentListener::_disposing( const EventObject& /*_rSource*/ ) throw( RuntimeException)
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

