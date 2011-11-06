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
#include "precompiled_comphelper.hxx"
#include <comphelper/weakeventlistener.hxx>
#include <osl/diagnose.h>


//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    //=====================================================================
    //= OWeakListenerAdapter
    //=====================================================================
    //---------------------------------------------------------------------
    OWeakListenerAdapterBase::~OWeakListenerAdapterBase()
    {
    }

    //=====================================================================
    //= OWeakEventListenerAdapter
    //=====================================================================
    //---------------------------------------------------------------------
    OWeakEventListenerAdapter::OWeakEventListenerAdapter( Reference< XWeak > _rxListener, Reference< XComponent > _rxBroadcaster )
        :OWeakEventListenerAdapter_Base( _rxListener, _rxBroadcaster )
    {
        // add ourself as listener to the broadcaster
        OSL_ENSURE( _rxBroadcaster.is(), "OWeakEventListenerAdapter::OWeakEventListenerAdapter: invalid broadcaster!" );
        if ( _rxBroadcaster.is() )
        {
            osl_incrementInterlockedCount( &m_refCount );
            {
                _rxBroadcaster->addEventListener( this );
            }
            osl_decrementInterlockedCount( &m_refCount );
            OSL_ENSURE( m_refCount > 0, "OWeakEventListenerAdapter::OWeakEventListenerAdapter: oops - not to be used with implementations which hold their listeners weak!" );
                // the one and only reason for this adapter class (A) is to add as listener to a component (C) which
                // holds its listeners hard, and forward all calls then to another listener (L) which is
                // held weak by A.
                // Now if C holds listeners weak, then we do not need A, we can add L directly to C.
        }

        OSL_ENSURE( getListener().is(), "OWeakEventListenerAdapter::OWeakEventListenerAdapter: invalid listener (does not support the XEventListener interface)!" );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OWeakEventListenerAdapter::disposing( )
    {
        Reference< XComponent > xBroadcaster( getBroadcaster( ), UNO_QUERY );
        OSL_ENSURE( xBroadcaster.is(), "OWeakEventListenerAdapter::disposing: broadcaster is invalid in the meantime! How this?" );
        if ( xBroadcaster.is() )
        {
            xBroadcaster->removeEventListener( this );
        }

        resetListener();
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................


