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
#include "errorbroadcaster.hxx"
#include <connectivity/dbtools.hxx>
#include <com/sun/star/sdb/SQLContext.hpp>

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
        Any aError;
        if ( _rContextDescription.getLength() )
            aError = makeAny( prependErrorInfo( _rException, static_cast< XSQLErrorBroadcaster* >( this ), _rContextDescription ) );
        else
            aError = makeAny( _rException );

        onError( SQLErrorEvent( static_cast< XSQLErrorBroadcaster* >( this ), aError ) );
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
}   // namespace frm
//.........................................................................

