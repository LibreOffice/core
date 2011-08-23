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

#include "errorbroadcaster.hxx"
#include <connectivity/dbtools.hxx>
#include <com/sun/star/sdb/SQLContext.hpp>
namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::dbtools;

    //=====================================================================
    //= OErrorBroadcaster
    //=====================================================================
    //---------------------------------------------------------------------
    OErrorBroadcaster::OErrorBroadcaster( ::cppu::OBroadcastHelper& _rBHelper )
        :m_rBHelper( _rBHelper )
        ,m_aErrorListeners( _rBHelper.rMutex )
    {
    }

    //---------------------------------------------------------------------
    OErrorBroadcaster::~OErrorBroadcaster( )
    {
        OSL_ENSURE( m_rBHelper.bDisposed || m_rBHelper.bInDispose,
            "OErrorBroadcaster::~OErrorBroadcaster: not disposed!" );
        // herein, we don't have a chance to do the dispose ourself ....

        OSL_ENSURE( 0 == m_aErrorListeners.getLength(),
            "OErrorBroadcaster::~OErrorBroadcaster: still have listeners!" );
        // either we're not disposed, or the derived class did not call our dispose from within their dispose
    }

    //---------------------------------------------------------------------
    void SAL_CALL OErrorBroadcaster::disposing()
    {
        EventObject aDisposeEvent( static_cast< XSQLErrorBroadcaster* >( this ) );
        m_aErrorListeners.disposeAndClear( aDisposeEvent );
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OErrorBroadcaster::onError( const SQLException& _rException, const ::rtl::OUString& _rContextDescription )
    {
        OSL_ENSURE( false, "OErrorBroadcaster::onError: dead code!" );
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OErrorBroadcaster::onError( const ::com::sun::star::sdb::SQLErrorEvent& _rError )
    {
        if ( m_aErrorListeners.getLength() )
        {

            ::cppu::OInterfaceIteratorHelper aIter( m_aErrorListeners );
            while ( aIter.hasMoreElements() )
                static_cast< XSQLErrorListener* >( aIter.next() )->errorOccured( _rError );
        }
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OErrorBroadcaster::addSQLErrorListener( const Reference< XSQLErrorListener >& _rxListener ) throw( RuntimeException )
    {
        m_aErrorListeners.addInterface( _rxListener );
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OErrorBroadcaster::removeSQLErrorListener( const Reference< XSQLErrorListener >& _rxListener ) throw( RuntimeException )
    {
        m_aErrorListeners.removeInterface( _rxListener );
    }

//.........................................................................
}	// namespace frm
//.........................................................................

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
